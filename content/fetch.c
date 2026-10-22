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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/resource.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/about/about.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/curl.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/data.h"
#ಅನ್ತರ್ಭಾವ "content/fetchers/file/file.h"
#ಅನ್ತರ್ಭಾವ "javascript/fetcher.h"
#ಅನ್ತರ್ಭಾವ "content/urldb.h"

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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ಸಂರಚನಾ fetcher_operation_table ops; /**< The fetchers operations. */
	ಪೂರ್ಣಾಙ್ಕ refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ಸ್ಥಿರ scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ಸಂರಚನಾ fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	ಶೂನ್ಯ *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	ಪೂರ್ಣಾಙ್ಕ fetcherd;           /**< Fetcher descriptor for this fetch */
	ಶೂನ್ಯ *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_ಸಂದೇಶ_type last_ಸಂದೇಶ;/**< The last message sent for this fetch */
	ಸಂರಚನಾ fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	ಸಂರಚನಾ fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

ಸ್ಥಿರ ಸಂರಚನಾ fetch *fetch_ring = ರಿಕ್ತ;	/**< Ring of active fetches. */
ಸ್ಥಿರ ಸಂರಚನಾ fetch *queue_ring = ರಿಕ್ತ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ fetch_ref_fetcher(ಪೂರ್ಣಾಙ್ಕ fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ fetch_unref_fetcher(ಪೂರ್ಣಾಙ್ಕ fetcherd)
{
	fetchers[fetcherd].refcount--;
	ಯದಿ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_fetcher_for_scheme(lwc_string *scheme)
{
	ಪೂರ್ಣಾಙ್ಕ fetcherd;
	bool match;

	ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಯದಿ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			ಪ್ರತ್ಯಾಗಮ fetcherd;
	       }
	}
	ಪ್ರತ್ಯಾಗಮ -1;
}

/**
 * Dispatch a single job
 */
ಸ್ಥಿರ bool fetch_dispatch_job(ಸಂರಚನಾ fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ಯದಿ (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_ಸೇರಿಸು(queue_ring, fetch); /* Put it back on the end of the queue */
		ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ {
		RING_ಸೇರಿಸು(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		ಪ್ರತ್ಯಾಗಮ true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ಸ್ಥಿರ bool fetch_choose_and_dispatch(ಶೂನ್ಯ)
{
	bool same_host;
	ಸಂರಚನಾ fetch *queueitem;
	queueitem = queue_ring;
	ಕುರು {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		ಪೂರ್ಣಾಙ್ಕ countbyhost;
		RING_COUNTBYLWCHOST(ಸಂರಚನಾ fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ಯದಿ (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			ಪ್ರತ್ಯಾಗಮ fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		ಯಾವತ್ (same_host == true && queueitem->r_next != queue_ring) {
			ಯದಿ (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} ಯಾವತ್ (queueitem != queue_ring);
	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_rings(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ fetch *q;
	ಸಂರಚನಾ fetch *f;

	q = queue_ring;
	ಯದಿ (q) {
		ಕುರು {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} ಯಾವತ್ (q != queue_ring);
	}
	f = fetch_ring;
	ಯದಿ (f) {
		ಕುರು {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} ಯಾವತ್ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ಸ್ಥಿರ bool fetch_dispatch_jobs(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ all_active;
	ಪೂರ್ಣಾಙ್ಕ all_queued;

	RING_GETSIZE(ಸಂರಚನಾ fetch, queue_ring, all_queued);
	RING_GETSIZE(ಸಂರಚನಾ fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	ಯಾವತ್ ((all_queued != 0) &&
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

	ಪ್ರತ್ಯಾಗಮ (all_active > 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetcher_poll(ಶೂನ್ಯ *unused)
{
	ಪೂರ್ಣಾಙ್ಕ fetcherd;

	ಯದಿ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ಯದಿ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, ರಿಕ್ತ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(ಶೂನ್ಯ)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}
#endif

	ret = fetch_data_register();
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fetch_file_register();
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fetch_resource_register();
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fetch_about_register();
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = fetch_javascript_register();

	ಪ್ರತ್ಯಾಗಮ ret;
}

/* exported interface documented in content/fetchers.h */
ಶೂನ್ಯ fetcher_quit(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ fetcherd; /* fetcher index */
	ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಯದಿ (fetchers[fetcherd].refcount > 1) {
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
		ಯದಿ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, ಧ್ರುವ ಸಂರಚನಾ fetcher_operation_table *ops)
{
	ಪೂರ್ಣಾಙ್ಕ fetcherd;

	/* find unused fetcher descriptor */
	ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಯದಿ (fetchers[fetcherd].refcount == 0) {
			ಭಙ್ಗ;
		}
	}
	ಯದಿ (fetcherd == MAX_FETCHERS) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}

	ಯದಿ (!ops->initialise(scheme)) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      ಪೂರ್ಣಾಙ್ಕ *maxfd_out)
{
	ಪೂರ್ಣಾಙ್ಕ maxfd = -1;
	ಪೂರ್ಣಾಙ್ಕ fetcherd; /* fetcher index */

	ಯದಿ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಯದಿ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	ಪ್ರತಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಯದಿ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ರಿಕ್ತ)) {
			/* fetcher present */
			ಪೂರ್ಣಾಙ್ಕ fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			ಯದಿ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ಯದಿ (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, ರಿಕ್ತ);
	}

	*maxfd_out = maxfd;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    ಶೂನ್ಯ *p,
	    bool only_2xx,
	    ಧ್ರುವ ವರ್ಣ *post_urlenc,
	    ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ಧ್ರುವ ವರ್ಣ *headers[],
	    ಸಂರಚನಾ fetch **fetch_out)
{
	ಸಂರಚನಾ fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, ಪರಿಮಾಣ (*fetch));
	ಯದಿ (fetch == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಪ್ರತಿಜ್ಞಾ(scheme != ರಿಕ್ತ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ಯದಿ (fetch->fetcherd == -1) {
		ಮುಕ್ತ(fetch);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ಯದಿ (referer != ರಿಕ್ತ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	ಯದಿ (fetch->fetcher_handle == ರಿಕ್ತ) {
		lwc_string_unref(fetch->host);

		ಯದಿ (fetch->url != ರಿಕ್ತ)
			nsurl_unref(fetch->url);

		ಯದಿ (fetch->referer != ರಿಕ್ತ)
			nsurl_unref(fetch->referer);

		ಮುಕ್ತ(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_ಸೇರಿಸು(queue_ring, fetch);

	/* Ask the queue to run. */
	ಯದಿ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, ರಿಕ್ತ);
	}

	*fetch_out = fetch;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_abort(ಸಂರಚನಾ fetch *f)
{
	ಪ್ರತಿಜ್ಞಾ(f);
	f->last_ಸಂದೇಶ = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_free(ಸಂರಚನಾ fetch *f)
{
	ಯದಿ (f->last_ಸಂದೇಶ < FETCH_MIN_FINISHED_ಸಂದೇಶ) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_ಸಂದೇಶ ಸಂದೇಶ;

		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&ಸಂದೇಶ, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.ಮುಕ್ತ(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	ಯದಿ (f->referer != ರಿಕ್ತ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	ಮುಕ್ತ(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ಧ್ರುವ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಪೂರ್ಣಾಙ್ಕ fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ಯದಿ (fetcherd == -1) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_change_callback(ಸಂರಚನಾ fetch *fetch,
			   fetch_callback callback,
			   ಶೂನ್ಯ *p)
{
	ಪ್ರತಿಜ್ಞಾ(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(ಸಂರಚನಾ fetch *fetch)
{
	ಪ್ರತ್ಯಾಗಮ fetch->http_code;
}


/* exported interface documented in content/fetch.h */
ಸಂರಚನಾ fetch_multipart_data *
fetch_multipart_data_clone(ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *list)
{
	ಸಂರಚನಾ fetch_multipart_data *clone, *last = ರಿಕ್ತ;
	ಸಂರಚನಾ fetch_multipart_data *result = ರಿಕ್ತ;

	ಪ್ರತಿ (; list != ರಿಕ್ತ; list = list->next) {
		clone = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ fetch_multipart_data));
		ಯದಿ (clone == ರಿಕ್ತ) {
			ಯದಿ (result != ರಿಕ್ತ)
				fetch_multipart_data_destroy(result);

			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ಯದಿ (clone->name == ರಿಕ್ತ) {
			ಮುಕ್ತ(clone);
			ಯದಿ (result != ರಿಕ್ತ)
				fetch_multipart_data_destroy(result);

			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		clone->ಮೌಲ್ಯ = strdup(list->ಮೌಲ್ಯ);
		ಯದಿ (clone->ಮೌಲ್ಯ == ರಿಕ್ತ) {
			ಮುಕ್ತ(clone->name);
			ಮುಕ್ತ(clone);
			ಯದಿ (result != ರಿಕ್ತ)
				fetch_multipart_data_destroy(result);

			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		ಯದಿ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ಯದಿ (clone->rawfile == ರಿಕ್ತ) {
				ಮುಕ್ತ(clone->ಮೌಲ್ಯ);
				ಮುಕ್ತ(clone->name);
				ಮುಕ್ತ(clone);
				ಯದಿ (result != ರಿಕ್ತ)
					fetch_multipart_data_destroy(result);

				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}
		} ಅನ್ಯಥಾ {
			clone->rawfile = ರಿಕ್ತ;
		}

		clone->next = ರಿಕ್ತ;

		ಯದಿ (result == ರಿಕ್ತ)
			result = clone;
		ಅನ್ಯಥಾ
			last->next = clone;

		last = clone;
	}

	ಪ್ರತ್ಯಾಗಮ result;
}


/* exported interface documented in content/fetch.h */
ಧ್ರುವ ವರ್ಣ *
fetch_multipart_data_find(ಧ್ರುವ ಸಂರಚನಾ fetch_multipart_data *list,
			  ಧ್ರುವ ವರ್ಣ *name)
{
	ಯಾವತ್ (list != ರಿಕ್ತ) {
		ಯದಿ (strcmp(list->name, name) == 0) {
			ಪ್ರತ್ಯಾಗಮ list->ಮೌಲ್ಯ;
		}
		list = list->next;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_multipart_data_destroy(ಸಂರಚನಾ fetch_multipart_data *list)
{
	ಸಂರಚನಾ fetch_multipart_data *next;

	ಪ್ರತಿ (; list != ರಿಕ್ತ; list = next) {
		next = list->next;
		ಮುಕ್ತ(list->name);
		ಮುಕ್ತ(list->ಮೌಲ್ಯ);
		ಯದಿ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			ಮುಕ್ತ(list->rawfile);
		}
		ಮುಕ್ತ(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(ಸಂರಚನಾ fetch_multipart_data **list,
			    ಧ್ರುವ ವರ್ಣ *name,
			    ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ fetch_multipart_data *newdata;

	ಪ್ರತಿಜ್ಞಾ(list);

	newdata = calloc(1, ಪರಿಮಾಣ(*newdata));

	ಯದಿ (newdata == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ಯದಿ (newdata->name == ರಿಕ್ತ) {
		ಮುಕ್ತ(newdata);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	newdata->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
	ಯದಿ (newdata->ಮೌಲ್ಯ == ರಿಕ್ತ) {
		ಮುಕ್ತ(newdata->name);
		ಮುಕ್ತ(newdata);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ
fetch_send_callback(ಧ್ರುವ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ಸಂರಚನಾ fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ಯದಿ (ಸಂದೇಶ->type > fetch->last_ಸಂದೇಶ)
		fetch->last_ಸಂದೇಶ = ಸಂದೇಶ->type;
	fetch->callback(ಸಂದೇಶ, fetch->p);
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_remove_from_queues(ಸಂರಚನಾ fetch *fetch)
{
	ಪೂರ್ಣಾಙ್ಕ all_active;
	ಪೂರ್ಣಾಙ್ಕ all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ಯದಿ (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} ಅನ್ಯಥಾ {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(ಸಂರಚನಾ fetch, fetch_ring, all_active);
	RING_GETSIZE(ಸಂರಚನಾ fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(ಸಂರಚನಾ fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_set_cookie(ಸಂರಚನಾ fetch *fetch, ಧ್ರುವ ವರ್ಣ *data)
{
	ಪ್ರತಿಜ್ಞಾ(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ಯದಿ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, ರಿಕ್ತ);
	} ಅನ್ಯಥಾ ಯದಿ (fetch->referer != ರಿಕ್ತ) {
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
