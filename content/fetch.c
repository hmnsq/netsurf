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

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/fetchers.h"
#அந்தர்பாவ "content/fetchers/resource.h"
#அந்தர்பாவ "content/fetchers/about/about.h"
#அந்தர்பாவ "content/fetchers/curl.h"
#அந்தர்பாவ "content/fetchers/data.h"
#அந்தர்பாவ "content/fetchers/file/file.h"
#அந்தர்பாவ "javascript/fetcher.h"
#அந்தர்பாவ "content/urldb.h"

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
வகைவரை கட்டமைப்பு scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	கட்டமைப்பு fetcher_operation_table ops; /**< The fetchers operations. */
	முழு refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

நிலையான scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
கட்டமைப்பு fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	வெறுமை *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	முழு fetcherd;           /**< Fetcher descriptor for this fetch */
	வெறுமை *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_செய்தி_type last_செய்தி;/**< The last message sent for this fetch */
	கட்டமைப்பு fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	கட்டமைப்பு fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

நிலையான கட்டமைப்பு fetch *fetch_ring = வெற்று;	/**< Ring of active fetches. */
நிலையான கட்டமைப்பு fetch *queue_ring = வெற்று;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

நிலையான உள்வரி வெறுமை fetch_ref_fetcher(முழு fetcherd)
{
	fetchers[fetcherd].refcount++;
}

நிலையான உள்வரி வெறுமை fetch_unref_fetcher(முழு fetcherd)
{
	fetchers[fetcherd].refcount--;
	எனில் (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
நிலையான முழு get_fetcher_for_scheme(lwc_string *scheme)
{
	முழு fetcherd;
	bool match;

	க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		எனில் ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			திரும்பு fetcherd;
	       }
	}
	திரும்பு -1;
}

/**
 * Dispatch a single job
 */
நிலையான bool fetch_dispatch_job(கட்டமைப்பு fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	எனில் (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_செருகு(queue_ring, fetch); /* Put it back on the end of the queue */
		திரும்பு false;
	} இல்லையெனில் {
		RING_செருகு(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		திரும்பு true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
நிலையான bool fetch_choose_and_dispatch(வெறுமை)
{
	bool same_host;
	கட்டமைப்பு fetch *queueitem;
	queueitem = queue_ring;
	செய் {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		முழு countbyhost;
		RING_COUNTBYLWCHOST(கட்டமைப்பு fetch, fetch_ring, countbyhost,
				    queueitem->host);
		எனில் (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			திரும்பு fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		போது (same_host == true && queueitem->r_next != queue_ring) {
			எனில் (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} போது (queueitem != queue_ring);
	திரும்பு false;
}

நிலையான வெறுமை dump_rings(வெறுமை)
{
	கட்டமைப்பு fetch *q;
	கட்டமைப்பு fetch *f;

	q = queue_ring;
	எனில் (q) {
		செய் {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} போது (q != queue_ring);
	}
	f = fetch_ring;
	எனில் (f) {
		செய் {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} போது (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
நிலையான bool fetch_dispatch_jobs(வெறுமை)
{
	முழு all_active;
	முழு all_queued;

	RING_GETSIZE(கட்டமைப்பு fetch, queue_ring, all_queued);
	RING_GETSIZE(கட்டமைப்பு fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	போது ((all_queued != 0) &&
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

	திரும்பு (all_active > 0);
}

நிலையான வெறுமை fetcher_poll(வெறுமை *unused)
{
	முழு fetcherd;

	எனில் (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			எனில் (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, வெற்று);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(வெறுமை)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}
#endif

	ret = fetch_data_register();
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}

	ret = fetch_file_register();
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}

	ret = fetch_resource_register();
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}

	ret = fetch_about_register();
	எனில் (ret != NSERROR_OK) {
		திரும்பு ret;
	}

	ret = fetch_javascript_register();

	திரும்பு ret;
}

/* exported interface documented in content/fetchers.h */
வெறுமை fetcher_quit(வெறுமை)
{
	முழு fetcherd; /* fetcher index */
	க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		எனில் (fetchers[fetcherd].refcount > 1) {
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
		எனில் (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, மாறிலி கட்டமைப்பு fetcher_operation_table *ops)
{
	முழு fetcherd;

	/* find unused fetcher descriptor */
	க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		எனில் (fetchers[fetcherd].refcount == 0) {
			நிறுத்து;
		}
	}
	எனில் (fetcherd == MAX_FETCHERS) {
		திரும்பு NSERROR_INIT_FAILED;
	}

	எனில் (!ops->initialise(scheme)) {
		திரும்பு NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	திரும்பு NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      முழு *maxfd_out)
{
	முழு maxfd = -1;
	முழு fetcherd; /* fetcher index */

	எனில் (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		திரும்பு NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		எனில் (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	க்கு (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		எனில் ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != வெற்று)) {
			/* fetcher present */
			முழு fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			எனில் (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	எனில் (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, வெற்று);
	}

	*maxfd_out = maxfd;

	திரும்பு NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    வெறுமை *p,
	    bool only_2xx,
	    மாறிலி எழுத்து *post_urlenc,
	    மாறிலி கட்டமைப்பு fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    மாறிலி எழுத்து *headers[],
	    கட்டமைப்பு fetch **fetch_out)
{
	கட்டமைப்பு fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, அளவு (*fetch));
	எனில் (fetch == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ப்ரதிஜ்ஞா(scheme != வெற்று);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	எனில் (fetch->fetcherd == -1) {
		விடுவி(fetch);
		திரும்பு NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	எனில் (referer != வெற்று) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	எனில் (fetch->fetcher_handle == வெற்று) {
		lwc_string_unref(fetch->host);

		எனில் (fetch->url != வெற்று)
			nsurl_unref(fetch->url);

		எனில் (fetch->referer != வெற்று)
			nsurl_unref(fetch->referer);

		விடுவி(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		திரும்பு NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_செருகு(queue_ring, fetch);

	/* Ask the queue to run. */
	எனில் (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, வெற்று);
	}

	*fetch_out = fetch;
	திரும்பு NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
வெறுமை fetch_abort(கட்டமைப்பு fetch *f)
{
	ப்ரதிஜ்ஞா(f);
	f->last_செய்தி = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
வெறுமை fetch_free(கட்டமைப்பு fetch *f)
{
	எனில் (f->last_செய்தி < FETCH_MIN_FINISHED_செய்தி) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_செய்தி செய்தி;

		செய்தி.type = FETCH_ERROR;
		செய்தி.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&செய்தி, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.விடுவி(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	எனில் (f->referer != வெற்று) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	விடுவி(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(மாறிலி nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	முழு fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	எனில் (fetcherd == -1) {
		திரும்பு false;
	}

	திரும்பு fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
வெறுமை fetch_change_callback(கட்டமைப்பு fetch *fetch,
			   fetch_callback callback,
			   வெறுமை *p)
{
	ப்ரதிஜ்ஞா(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(கட்டமைப்பு fetch *fetch)
{
	திரும்பு fetch->http_code;
}


/* exported interface documented in content/fetch.h */
கட்டமைப்பு fetch_multipart_data *
fetch_multipart_data_clone(மாறிலி கட்டமைப்பு fetch_multipart_data *list)
{
	கட்டமைப்பு fetch_multipart_data *clone, *last = வெற்று;
	கட்டமைப்பு fetch_multipart_data *result = வெற்று;

	க்கு (; list != வெற்று; list = list->next) {
		clone = malloc(அளவு(கட்டமைப்பு fetch_multipart_data));
		எனில் (clone == வெற்று) {
			எனில் (result != வெற்று)
				fetch_multipart_data_destroy(result);

			திரும்பு வெற்று;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		எனில் (clone->name == வெற்று) {
			விடுவி(clone);
			எனில் (result != வெற்று)
				fetch_multipart_data_destroy(result);

			திரும்பு வெற்று;
		}

		clone->மதிப்பு = strdup(list->மதிப்பு);
		எனில் (clone->மதிப்பு == வெற்று) {
			விடுவி(clone->name);
			விடுவி(clone);
			எனில் (result != வெற்று)
				fetch_multipart_data_destroy(result);

			திரும்பு வெற்று;
		}

		எனில் (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			எனில் (clone->rawfile == வெற்று) {
				விடுவி(clone->மதிப்பு);
				விடுவி(clone->name);
				விடுவி(clone);
				எனில் (result != வெற்று)
					fetch_multipart_data_destroy(result);

				திரும்பு வெற்று;
			}
		} இல்லையெனில் {
			clone->rawfile = வெற்று;
		}

		clone->next = வெற்று;

		எனில் (result == வெற்று)
			result = clone;
		இல்லையெனில்
			last->next = clone;

		last = clone;
	}

	திரும்பு result;
}


/* exported interface documented in content/fetch.h */
மாறிலி எழுத்து *
fetch_multipart_data_find(மாறிலி கட்டமைப்பு fetch_multipart_data *list,
			  மாறிலி எழுத்து *name)
{
	போது (list != வெற்று) {
		எனில் (strcmp(list->name, name) == 0) {
			திரும்பு list->மதிப்பு;
		}
		list = list->next;
	}

	திரும்பு வெற்று;
}


/* exported interface documented in content/fetch.h */
வெறுமை fetch_multipart_data_destroy(கட்டமைப்பு fetch_multipart_data *list)
{
	கட்டமைப்பு fetch_multipart_data *next;

	க்கு (; list != வெற்று; list = next) {
		next = list->next;
		விடுவி(list->name);
		விடுவி(list->மதிப்பு);
		எனில் (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			விடுவி(list->rawfile);
		}
		விடுவி(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(கட்டமைப்பு fetch_multipart_data **list,
			    மாறிலி எழுத்து *name,
			    மாறிலி எழுத்து *மதிப்பு)
{
	கட்டமைப்பு fetch_multipart_data *newdata;

	ப்ரதிஜ்ஞா(list);

	newdata = calloc(1, அளவு(*newdata));

	எனில் (newdata == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	எனில் (newdata->name == வெற்று) {
		விடுவி(newdata);
		திரும்பு NSERROR_NOMEM;
	}

	newdata->மதிப்பு = strdup(மதிப்பு);
	எனில் (newdata->மதிப்பு == வெற்று) {
		விடுவி(newdata->name);
		விடுவி(newdata);
		திரும்பு NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
வெறுமை
fetch_send_callback(மாறிலி fetch_செய்தி *செய்தி, கட்டமைப்பு fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	எனில் (செய்தி->type > fetch->last_செய்தி)
		fetch->last_செய்தி = செய்தி->type;
	fetch->callback(செய்தி, fetch->p);
}


/* exported interface documented in content/fetch.h */
வெறுமை fetch_remove_from_queues(கட்டமைப்பு fetch *fetch)
{
	முழு all_active;
	முழு all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	எனில் (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} இல்லையெனில் {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(கட்டமைப்பு fetch, fetch_ring, all_active);
	RING_GETSIZE(கட்டமைப்பு fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(கட்டமைப்பு fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	திரும்பு NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
வெறுமை fetch_set_cookie(கட்டமைப்பு fetch *fetch, மாறிலி எழுத்து *data)
{
	ப்ரதிஜ்ஞா(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	எனில் (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, வெற்று);
	} இல்லையெனில் எனில் (fetch->referer != வெற்று) {
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
