/*
 * Copyright 2006,2007 Daniel Silverstone <dsilvers@digital-scurf.org>
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * Implementation of fetching of data from a URL.
 *
 * The implementation is the fetch factory and the generic operations
 * around the fetcher specific methods.
 *
 * Active fetches are held in the circular linked list ::fetch_ring. There may
 * be at most nsoption max_fetchers_per_host active requests per Host: header.
 * There may be at most nsoption max_fetchers active requests overall. Inactive
 * fetches are stored in the ::queue_ring waiting for use.
 */

#شامل <stdlib.h>
#شامل <assert.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/nsurl.h"
#شامل "utils/ring.h"
#شامل "netsurf/misc.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "content/fetchers/resource.h"
#شامل "content/fetchers/about/about.h"
#شامل "content/fetchers/curl.h"
#شامل "content/fetchers/data.h"
#شامل "content/fetchers/file/file.h"
#شامل "javascript/fetcher.h"
#شامل "content/urldb.h"

/** The maximum number of fetchers that can be added */
#define MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#define SCHEDULE_TIME 10

/** The fdset timeout in ms */
#define FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
تعریفنوع ساختار scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ساختار fetcher_operation_table ops; /**< The fetchers operations. */
	صحیح refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ایستا scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ساختار fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	تهی *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	صحیح fetcherd;           /**< Fetcher descriptor for this fetch */
	تهی *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_پیام_type last_پیام;/**< The last message sent for this fetch */
	ساختار fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	ساختار fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

ایستا ساختار fetch *fetch_ring = پوچ;	/**< Ring of active fetches. */
ایستا ساختار fetch *queue_ring = پوچ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ایستا درونخطی تهی fetch_ref_fetcher(صحیح fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ایستا درونخطی تهی fetch_unref_fetcher(صحیح fetcherd)
{
	fetchers[fetcherd].refcount--;
	اگر (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ایستا صحیح get_fetcher_for_scheme(lwc_string *scheme)
{
	صحیح fetcherd;
	bool match;

	برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			بازگشت fetcherd;
	       }
	}
	بازگشت -1;
}

/**
 * Dispatch a single job
 */
ایستا bool fetch_dispatch_job(ساختار fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	اگر (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_درج(queue_ring, fetch); /* Put it back on the end of the queue */
		بازگشت false;
	} وگرنه {
		RING_درج(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		بازگشت true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ایستا bool fetch_choose_and_dispatch(تهی)
{
	bool same_host;
	ساختار fetch *queueitem;
	queueitem = queue_ring;
	انجامبده {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		صحیح countbyhost;
		RING_COUNTBYLWCHOST(ساختار fetch, fetch_ring, countbyhost,
				    queueitem->host);
		اگر (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			بازگشت fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		وقتی (same_host == true && queueitem->r_next != queue_ring) {
			اگر (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} وقتی (queueitem != queue_ring);
	بازگشت false;
}

ایستا تهی dump_rings(تهی)
{
	ساختار fetch *q;
	ساختار fetch *f;

	q = queue_ring;
	اگر (q) {
		انجامبده {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} وقتی (q != queue_ring);
	}
	f = fetch_ring;
	اگر (f) {
		انجامبده {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} وقتی (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ایستا bool fetch_dispatch_jobs(تهی)
{
	صحیح all_active;
	صحیح all_queued;

	RING_GETSIZE(ساختار fetch, queue_ring, all_queued);
	RING_GETSIZE(ساختار fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	وقتی ((all_queued != 0) &&
	       (all_active < nsoption_int(max_fetchers)) &&
	       fetch_choose_and_dispatch()) {
			all_queued--;
			all_active++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_active);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	بازگشت (all_active > 0);
}

ایستا تهی fetcher_poll(تهی *unused)
{
	صحیح fetcherd;

	اگر (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			اگر (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, پوچ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(تهی)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}
#endif

	ret = fetch_data_register();
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}

	ret = fetch_file_register();
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}

	ret = fetch_resource_register();
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}

	ret = fetch_about_register();
	اگر (ret != NSERROR_OK) {
		بازگشت ret;
	}

	ret = fetch_javascript_register();

	بازگشت ret;
}

/* exported interface documented in content/fetchers.h */
تهی fetcher_quit(تهی)
{
	صحیح fetcherd; /* fetcher index */
	برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount > 1) {
			/* fetcher still has reference at quit. This
			 * should not happen as the fetch should have
			 * been aborted in llcache shutdown.
			 *
			 * This appears to be normal behaviour if a
			 * curl operation is still in progress at exit
			 * as the abort waits for curl to complete.
			 *
			 * We could make the user wait for curl to
			 * complete but we are exiting anyway so thats
			 * unhelpful. Instead we just log it and force
			 * the reference count to allow the fetcher to
			 * be stopped.
			 */
			NSLOG(fetch, INFO,
			      "Fetcher for scheme %s still has %d active users at quit.",
			      lwc_string_data(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		اگر (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, ثابت ساختار fetcher_operation_table *ops)
{
	صحیح fetcherd;

	/* find unused fetcher descriptor */
	برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount == 0) {
			بشکن;
		}
	}
	اگر (fetcherd == MAX_FETCHERS) {
		بازگشت NSERROR_INIT_FAILED;
	}

	اگر (!ops->initialise(scheme)) {
		بازگشت NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	بازگشت NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      صحیح *maxfd_out)
{
	صحیح maxfd = -1;
	صحیح fetcherd; /* fetcher index */

	اگر (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		بازگشت NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	برای (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != پوچ)) {
			/* fetcher present */
			صحیح fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			اگر (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	اگر (maxfd >= 0) {
		/* change the scheduled poll to happen is a 1000ms as
		 * we assume fetching an fdset means the fetchers will
		 * be run by the client waking up on data available on
		 * the fd and re-calling fetcher_fdset() if this does
		 * not happen the fetch polling will continue as
		 * usual.
		 */
		/** @note adjusting the schedule time is only done for
		 * curl currently. This is because as it is assumed to
		 * be the only fetcher that can possibly have fd to
		 * select on. All the other fetchers continue to need
		 * polling frequently.
		 */
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, پوچ);
	}

	*maxfd_out = maxfd;

	بازگشت NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    تهی *p,
	    bool only_2xx,
	    ثابت نویسه *post_urlenc,
	    ثابت ساختار fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ثابت نویسه *headers[],
	    ساختار fetch **fetch_out)
{
	ساختار fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, اندازه (*fetch));
	اگر (fetch == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	أكد(scheme != پوچ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	اگر (fetch->fetcherd == -1) {
		آزاد(fetch);
		بازگشت NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	اگر (referer != پوچ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	اگر (fetch->fetcher_handle == پوچ) {
		lwc_string_unref(fetch->host);

		اگر (fetch->url != پوچ)
			nsurl_unref(fetch->url);

		اگر (fetch->referer != پوچ)
			nsurl_unref(fetch->referer);

		آزاد(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		بازگشت NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_درج(queue_ring, fetch);

	/* Ask the queue to run. */
	اگر (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, پوچ);
	}

	*fetch_out = fetch;
	بازگشت NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
تهی fetch_abort(ساختار fetch *f)
{
	أكد(f);
	f->last_پیام = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
تهی fetch_free(ساختار fetch *f)
{
	اگر (f->last_پیام < FETCH_MIN_FINISHED_پیام) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_پیام پیام;

		پیام.type = FETCH_ERROR;
		پیام.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&پیام, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.آزاد(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	اگر (f->referer != پوچ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	آزاد(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	صحیح fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	اگر (fetcherd == -1) {
		بازگشت false;
	}

	بازگشت fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
تهی fetch_change_callback(ساختار fetch *fetch,
			   fetch_callback callback,
			   تهی *p)
{
	أكد(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(ساختار fetch *fetch)
{
	بازگشت fetch->http_code;
}


/* exported interface documented in content/fetch.h */
ساختار fetch_multipart_data *
fetch_multipart_data_clone(ثابت ساختار fetch_multipart_data *list)
{
	ساختار fetch_multipart_data *clone, *last = پوچ;
	ساختار fetch_multipart_data *result = پوچ;

	برای (; list != پوچ; list = list->next) {
		clone = malloc(اندازه(ساختار fetch_multipart_data));
		اگر (clone == پوچ) {
			اگر (result != پوچ)
				fetch_multipart_data_destroy(result);

			بازگشت پوچ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		اگر (clone->name == پوچ) {
			آزاد(clone);
			اگر (result != پوچ)
				fetch_multipart_data_destroy(result);

			بازگشت پوچ;
		}

		clone->مقدار = strdup(list->مقدار);
		اگر (clone->مقدار == پوچ) {
			آزاد(clone->name);
			آزاد(clone);
			اگر (result != پوچ)
				fetch_multipart_data_destroy(result);

			بازگشت پوچ;
		}

		اگر (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			اگر (clone->rawfile == پوچ) {
				آزاد(clone->مقدار);
				آزاد(clone->name);
				آزاد(clone);
				اگر (result != پوچ)
					fetch_multipart_data_destroy(result);

				بازگشت پوچ;
			}
		} وگرنه {
			clone->rawfile = پوچ;
		}

		clone->next = پوچ;

		اگر (result == پوچ)
			result = clone;
		وگرنه
			last->next = clone;

		last = clone;
	}

	بازگشت result;
}


/* exported interface documented in content/fetch.h */
ثابت نویسه *
fetch_multipart_data_find(ثابت ساختار fetch_multipart_data *list,
			  ثابت نویسه *name)
{
	وقتی (list != پوچ) {
		اگر (strcmp(list->name, name) == 0) {
			بازگشت list->مقدار;
		}
		list = list->next;
	}

	بازگشت پوچ;
}


/* exported interface documented in content/fetch.h */
تهی fetch_multipart_data_destroy(ساختار fetch_multipart_data *list)
{
	ساختار fetch_multipart_data *next;

	برای (; list != پوچ; list = next) {
		next = list->next;
		آزاد(list->name);
		آزاد(list->مقدار);
		اگر (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			آزاد(list->rawfile);
		}
		آزاد(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(ساختار fetch_multipart_data **list,
			    ثابت نویسه *name,
			    ثابت نویسه *مقدار)
{
	ساختار fetch_multipart_data *newdata;

	أكد(list);

	newdata = calloc(1, اندازه(*newdata));

	اگر (newdata == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	اگر (newdata->name == پوچ) {
		آزاد(newdata);
		بازگشت NSERROR_NOMEM;
	}

	newdata->مقدار = strdup(مقدار);
	اگر (newdata->مقدار == پوچ) {
		آزاد(newdata->name);
		آزاد(newdata);
		بازگشت NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
تهی
fetch_send_callback(ثابت fetch_پیام *پیام, ساختار fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	اگر (پیام->type > fetch->last_پیام)
		fetch->last_پیام = پیام->type;
	fetch->callback(پیام, fetch->p);
}


/* exported interface documented in content/fetch.h */
تهی fetch_remove_from_queues(ساختار fetch *fetch)
{
	صحیح all_active;
	صحیح all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	اگر (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} وگرنه {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(ساختار fetch, fetch_ring, all_active);
	RING_GETSIZE(ساختار fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(ساختار fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	بازگشت NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
تهی fetch_set_cookie(ساختار fetch *fetch, ثابت نویسه *data)
{
	أكد(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	اگر (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, پوچ);
	} وگرنه اگر (fetch->referer != پوچ) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_set_cookie(data, fetch->url, fetch->referer);
	}
}
