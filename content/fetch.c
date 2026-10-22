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

#ضمن <stdlib.h>
#ضمن <assert.h>
#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/ring.h"
#ضمن "netsurf/misc.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "content/fetchers/resource.h"
#ضمن "content/fetchers/about/about.h"
#ضمن "content/fetchers/curl.h"
#ضمن "content/fetchers/data.h"
#ضمن "content/fetchers/file/file.h"
#ضمن "javascript/fetcher.h"
#ضمن "content/urldb.h"

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
تعريفنوع بنية scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	بنية fetcher_operation_table ops; /**< The fetchers operations. */
	صحيح refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ساكن scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
بنية fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	فراغ *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	صحيح fetcherd;           /**< Fetcher descriptor for this fetch */
	فراغ *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_msg_type last_msg;/**< The last message sent for this fetch */
	بنية fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	بنية fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

ساكن بنية fetch *fetch_ring = عدم;	/**< Ring of active fetches. */
ساكن بنية fetch *queue_ring = عدم;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ساكن مضمن فراغ fetch_ref_fetcher(صحيح fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ساكن مضمن فراغ fetch_unref_fetcher(صحيح fetcherd)
{
	fetchers[fetcherd].refcount--;
	ܐܢ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ساكن صحيح get_fetcher_for_scheme(lwc_string *scheme)
{
	صحيح fetcherd;
	bool match;

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ܐܢ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			ܦܢܐ fetcherd;
	       }
	}
	ܦܢܐ -1;
}

/**
 * Dispatch a single job
 */
ساكن bool fetch_dispatch_job(بنية fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ܐܢ (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_INSERT(queue_ring, fetch); /* Put it back on the end of the queue */
		ܦܢܐ false;
	} ܐܚܪܢܐ {
		RING_INSERT(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		ܦܢܐ true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ساكن bool fetch_choose_and_dispatch(فراغ)
{
	bool same_host;
	بنية fetch *queueitem;
	queueitem = queue_ring;
	ܥܒܕ {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		صحيح countbyhost;
		RING_COUNTBYLWCHOST(بنية fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ܐܢ (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			ܦܢܐ fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		ܟܡܐ (same_host == true && queueitem->r_next != queue_ring) {
			ܐܢ (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} ܟܡܐ (queueitem != queue_ring);
	ܦܢܐ false;
}

ساكن فراغ dump_rings(فراغ)
{
	بنية fetch *q;
	بنية fetch *f;

	q = queue_ring;
	ܐܢ (q) {
		ܥܒܕ {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} ܟܡܐ (q != queue_ring);
	}
	f = fetch_ring;
	ܐܢ (f) {
		ܥܒܕ {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} ܟܡܐ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ساكن bool fetch_dispatch_jobs(فراغ)
{
	صحيح all_active;
	صحيح all_queued;

	RING_GETSIZE(بنية fetch, queue_ring, all_queued);
	RING_GETSIZE(بنية fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	ܟܡܐ ((all_queued != 0) &&
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

	ܦܢܐ (all_active > 0);
}

ساكن فراغ fetcher_poll(فراغ *unused)
{
	صحيح fetcherd;

	ܐܢ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ܐܢ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, عدم);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(فراغ)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	ܐܢ (ret != NSERROR_OK) {
		ܦܢܐ ret;
	}
#endif

	ret = fetch_data_register();
	ܐܢ (ret != NSERROR_OK) {
		ܦܢܐ ret;
	}

	ret = fetch_file_register();
	ܐܢ (ret != NSERROR_OK) {
		ܦܢܐ ret;
	}

	ret = fetch_resource_register();
	ܐܢ (ret != NSERROR_OK) {
		ܦܢܐ ret;
	}

	ret = fetch_about_register();
	ܐܢ (ret != NSERROR_OK) {
		ܦܢܐ ret;
	}

	ret = fetch_javascript_register();

	ܦܢܐ ret;
}

/* exported interface documented in content/fetchers.h */
فراغ fetcher_quit(فراغ)
{
	صحيح fetcherd; /* fetcher index */
	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ܐܢ (fetchers[fetcherd].refcount > 1) {
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
		ܐܢ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, ثابت بنية fetcher_operation_table *ops)
{
	صحيح fetcherd;

	/* find unused fetcher descriptor */
	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ܐܢ (fetchers[fetcherd].refcount == 0) {
			اكسر;
		}
	}
	ܐܢ (fetcherd == MAX_FETCHERS) {
		ܦܢܐ NSERROR_INIT_FAILED;
	}

	ܐܢ (!ops->initialise(scheme)) {
		ܦܢܐ NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ܦܢܐ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      صحيح *maxfd_out)
{
	صحيح maxfd = -1;
	صحيح fetcherd; /* fetcher index */

	ܐܢ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		ܦܢܐ NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ܐܢ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ܐܢ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != عدم)) {
			/* fetcher present */
			صحيح fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			ܐܢ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ܐܢ (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, عدم);
	}

	*maxfd_out = maxfd;

	ܦܢܐ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    فراغ *p,
	    bool only_2xx,
	    ثابت حرف *post_urlenc,
	    ثابت بنية fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ثابت حرف *headers[],
	    بنية fetch **fetch_out)
{
	بنية fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, حجم (*fetch));
	ܐܢ (fetch == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	أكد(scheme != عدم);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ܐܢ (fetch->fetcherd == -1) {
		حرر(fetch);
		ܦܢܐ NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ܐܢ (referer != عدم) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	ܐܢ (fetch->fetcher_handle == عدم) {
		lwc_string_unref(fetch->host);

		ܐܢ (fetch->url != عدم)
			nsurl_unref(fetch->url);

		ܐܢ (fetch->referer != عدم)
			nsurl_unref(fetch->referer);

		حرر(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ܦܢܐ NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_INSERT(queue_ring, fetch);

	/* Ask the queue to run. */
	ܐܢ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, عدم);
	}

	*fetch_out = fetch;
	ܦܢܐ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
فراغ fetch_abort(بنية fetch *f)
{
	أكد(f);
	f->last_msg = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
فراغ fetch_free(بنية fetch *f)
{
	ܐܢ (f->last_msg < FETCH_MIN_FINISHED_MSG) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_msg msg;

		msg.type = FETCH_ERROR;
		msg.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&msg, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.حرر(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	ܐܢ (f->referer != عدم) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	حرر(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	صحيح fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ܐܢ (fetcherd == -1) {
		ܦܢܐ false;
	}

	ܦܢܐ fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
فراغ fetch_change_callback(بنية fetch *fetch,
			   fetch_callback callback,
			   فراغ *p)
{
	أكد(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(بنية fetch *fetch)
{
	ܦܢܐ fetch->http_code;
}


/* exported interface documented in content/fetch.h */
بنية fetch_multipart_data *
fetch_multipart_data_clone(ثابت بنية fetch_multipart_data *list)
{
	بنية fetch_multipart_data *clone, *last = عدم;
	بنية fetch_multipart_data *result = عدم;

	لكل (; list != عدم; list = list->next) {
		clone = malloc(حجم(بنية fetch_multipart_data));
		ܐܢ (clone == عدم) {
			ܐܢ (result != عدم)
				fetch_multipart_data_destroy(result);

			ܦܢܐ عدم;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ܐܢ (clone->name == عدم) {
			حرر(clone);
			ܐܢ (result != عدم)
				fetch_multipart_data_destroy(result);

			ܦܢܐ عدم;
		}

		clone->value = strdup(list->value);
		ܐܢ (clone->value == عدم) {
			حرر(clone->name);
			حرر(clone);
			ܐܢ (result != عدم)
				fetch_multipart_data_destroy(result);

			ܦܢܐ عدم;
		}

		ܐܢ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ܐܢ (clone->rawfile == عدم) {
				حرر(clone->value);
				حرر(clone->name);
				حرر(clone);
				ܐܢ (result != عدم)
					fetch_multipart_data_destroy(result);

				ܦܢܐ عدم;
			}
		} ܐܚܪܢܐ {
			clone->rawfile = عدم;
		}

		clone->next = عدم;

		ܐܢ (result == عدم)
			result = clone;
		ܐܚܪܢܐ
			last->next = clone;

		last = clone;
	}

	ܦܢܐ result;
}


/* exported interface documented in content/fetch.h */
ثابت حرف *
fetch_multipart_data_find(ثابت بنية fetch_multipart_data *list,
			  ثابت حرف *name)
{
	ܟܡܐ (list != عدم) {
		ܐܢ (strcmp(list->name, name) == 0) {
			ܦܢܐ list->value;
		}
		list = list->next;
	}

	ܦܢܐ عدم;
}


/* exported interface documented in content/fetch.h */
فراغ fetch_multipart_data_destroy(بنية fetch_multipart_data *list)
{
	بنية fetch_multipart_data *next;

	لكل (; list != عدم; list = next) {
		next = list->next;
		حرر(list->name);
		حرر(list->value);
		ܐܢ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			حرر(list->rawfile);
		}
		حرر(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(بنية fetch_multipart_data **list,
			    ثابت حرف *name,
			    ثابت حرف *value)
{
	بنية fetch_multipart_data *newdata;

	أكد(list);

	newdata = calloc(1, حجم(*newdata));

	ܐܢ (newdata == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ܐܢ (newdata->name == عدم) {
		حرر(newdata);
		ܦܢܐ NSERROR_NOMEM;
	}

	newdata->value = strdup(value);
	ܐܢ (newdata->value == عدم) {
		حرر(newdata->name);
		حرر(newdata);
		ܦܢܐ NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	ܦܢܐ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
فراغ
fetch_send_callback(ثابت fetch_msg *msg, بنية fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ܐܢ (msg->type > fetch->last_msg)
		fetch->last_msg = msg->type;
	fetch->callback(msg, fetch->p);
}


/* exported interface documented in content/fetch.h */
فراغ fetch_remove_from_queues(بنية fetch *fetch)
{
	صحيح all_active;
	صحيح all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ܐܢ (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} ܐܚܪܢܐ {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(بنية fetch, fetch_ring, all_active);
	RING_GETSIZE(بنية fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(بنية fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	ܦܢܐ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
فراغ fetch_set_cookie(بنية fetch *fetch, ثابت حرف *data)
{
	أكد(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ܐܢ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, عدم);
	} ܐܚܪܢܐ ܐܢ (fetch->referer != عدم) {
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
