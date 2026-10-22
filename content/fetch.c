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
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ವೇಳಾಪಟ್ಟಿ_ಸಮಯ 10

/** The fdset timeout in ms */
#ವ್ಯಾಖ್ಯಾನಿಸು FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ ops; /**< The fetchers operations. */
	ಪೂರ್ಣಾಂಕ refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ಸ್ಥಿರ scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ರಚನೆ fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	ಶೂನ್ಯ *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_ಸಂಕೇತ http_ಸಂಕೇತ;	/**< HTTP response code, or 0. */
	ಪೂರ್ಣಾಂಕ fetcherd;           /**< Fetcher descriptor for this fetch */
	ಶೂನ್ಯ *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_ಸಕ್ರಿಯ;	/**< This fetch is active. */
	fetch_ಸಂದೇಶ_type ಕೊನೆಯ_ಸಂದೇಶ;/**< The last message sent for this fetch */
	ರಚನೆ fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	ರಚನೆ fetch *r_ಮುಂದಿನ;	/**< Next active fetch in ::fetch_ring. */
};

ಸ್ಥಿರ ರಚನೆ fetch *fetch_ring = ಖಾಲಿ;	/**< Ring of active fetches. */
ಸ್ಥಿರ ರಚನೆ fetch *ಸರತಿ_ring = ಖಾಲಿ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ fetch_ref_fetcher(ಪೂರ್ಣಾಂಕ fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ fetch_unref_fetcher(ಪೂರ್ಣಾಂಕ fetcherd)
{
	fetchers[fetcherd].refcount--;
	ಆದರೆ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_fetcher_for_scheme(lwc_string *scheme)
{
	ಪೂರ್ಣಾಂಕ fetcherd;
	bool ಹೊಂದಿಕೆ;

	ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಆದರೆ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &ಹೊಂದಿಕೆ) == lwc_error_ok) &&
		    (ಹೊಂದಿಕೆ == true)) {
			ಹಿಂತಿರುಗಿ fetcherd;
	       }
	}
	ಹಿಂತಿರುಗಿ -1;
}

/**
 * Dispatch a single job
 */
ಸ್ಥಿರ bool fetch_dispatch_ಕೆಲಸ(ರಚನೆ fetch *fetch)
{
	RING_REMOVE(ಸರತಿ_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ಆದರೆ (!fetchers[fetch->fetcherd].ops.ಆರಂಭ(fetch->fetcher_handle)) {
		RING_ಸೇರಿಸು(ಸರತಿ_ring, fetch); /* Put it back on the end of the queue */
		ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		RING_ಸೇರಿಸು(fetch_ring, fetch);
		fetch->fetch_is_ಸಕ್ರಿಯ = true;
		ಹಿಂತಿರುಗಿ true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ಸ್ಥಿರ bool fetch_ಆರಿಸು_and_dispatch(ಶೂನ್ಯ)
{
	bool same_host;
	ರಚನೆ fetch *queueitem;
	queueitem = ಸರತಿ_ring;
	ಮಾಡು {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		ಪೂರ್ಣಾಂಕ countbyhost;
		RING_COUNTBYLWCHOST(ರಚನೆ fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ಆದರೆ (countbyhost < nsoption_int(max_fetchers_ಪ್ರತಿ_host)) {
			/* We can dispatch this item in theory */
			ಹಿಂತಿರುಗಿ fetch_dispatch_ಕೆಲಸ(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		ಆಗ (same_host == true && queueitem->r_ಮುಂದಿನ != ಸರತಿ_ring) {
			ಆದರೆ (lwc_string_isequal(queueitem->host,
					       queueitem->r_ಮುಂದಿನ->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_ಮುಂದಿನ;
			}
		}
		queueitem = queueitem->r_ಮುಂದಿನ;
	} ಆಗ (queueitem != ಸರತಿ_ring);
	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_rings(ಶೂನ್ಯ)
{
	ರಚನೆ fetch *q;
	ರಚನೆ fetch *f;

	q = ಸರತಿ_ring;
	ಆದರೆ (q) {
		ಮಾಡು {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_ಮುಂದಿನ;
		} ಆಗ (q != ಸರತಿ_ring);
	}
	f = fetch_ring;
	ಆದರೆ (f) {
		ಮಾಡು {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_ಮುಂದಿನ;
		} ಆಗ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ಸ್ಥಿರ bool fetch_dispatch_jobs(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ all_ಸಕ್ರಿಯ;
	ಪೂರ್ಣಾಂಕ all_queued;

	RING_GETSIZE(ರಚನೆ fetch, ಸರತಿ_ring, all_queued);
	RING_GETSIZE(ರಚನೆ fetch, fetch_ring, all_ಸಕ್ರಿಯ);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_ಸಕ್ರಿಯ);
	dump_rings();

	ಆಗ ((all_queued != 0) &&
	       (all_ಸಕ್ರಿಯ < nsoption_int(max_fetchers)) &&
	       fetch_ಆರಿಸು_and_dispatch()) {
			all_queued--;
			all_ಸಕ್ರಿಯ++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_ಸಕ್ರಿಯ);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_ಸಕ್ರಿಯ);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	ಹಿಂತಿರುಗಿ (all_ಸಕ್ರಿಯ > 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ fetcher_poll(ಶೂನ್ಯ *ಬಳಸದ)
{
	ಪೂರ್ಣಾಂಕ fetcherd;

	ಆದರೆ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ಆದರೆ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->ವೇಳಾಪಟ್ಟಿ(ವೇಳಾಪಟ್ಟಿ_ಸಮಯ, fetcher_poll, ಖಾಲಿ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(ಶೂನ್ಯ)
{
	nserror ret;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WITH_CURL
	ret = fetch_curl_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ret = fetch_ದತ್ತಾಂಶ_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fetch_file_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fetch_resource_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fetch_about_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (ret != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = fetch_javascript_ರಿಜಿಸ್ಟರ್();

	ಹಿಂತಿರುಗಿ ret;
}

/* exported interface documented in content/fetchers.h */
ಶೂನ್ಯ fetcher_quit(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ fetcherd; /* fetcher index */
	ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಆದರೆ (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_ದತ್ತಾಂಶ(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		ಆದರೆ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_ಕೂಡಿಸು(lwc_string *scheme, ಸ್ಥಿರಾಂಕ ರಚನೆ fetcher_operation_ಕೋಷ್ಟಕ *ops)
{
	ಪೂರ್ಣಾಂಕ fetcherd;

	/* find unused fetcher descriptor */
	ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಆದರೆ (fetchers[fetcherd].refcount == 0) {
			ನಿಲ್ಲಿಸು;
		}
	}
	ಆದರೆ (fetcherd == MAX_FETCHERS) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	ಆದರೆ (!ops->initialise(scheme)) {
		ಹಿಂತಿರುಗಿ NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_ಗಣ,
		      fd_set *write_fd_ಗಣ,
		      fd_set *except_fd_ಗಣ,
		      ಪೂರ್ಣಾಂಕ *maxfd_ಹೊರಹೋಗು)
{
	ಪೂರ್ಣಾಂಕ maxfd = -1;
	ಪೂರ್ಣಾಂಕ fetcherd; /* fetcher index */

	ಆದರೆ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_ಹೊರಹೋಗು = -1;
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಆದರೆ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_ಗಣ);
	FD_ZERO(write_fd_ಗಣ);
	FD_ZERO(except_fd_ಗಣ);

	ಗಾಗಿ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ಆದರೆ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ಖಾಲಿ)) {
			/* fetcher present */
			ಪೂರ್ಣಾಂಕ fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_ಗಣ,
				write_fd_ಗಣ, except_fd_ಗಣ);
			ಆದರೆ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ಆದರೆ (maxfd >= 0) {
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
		guit->misc->ವೇಳಾಪಟ್ಟಿ(FDSET_TIMEOUT, fetcher_poll, ಖಾಲಿ);
	}

	*maxfd_ಹೊರಹೋಗು = maxfd;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_ಆರಂಭ(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    ಶೂನ್ಯ *p,
	    bool only_2xx,
	    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *post_urlenc,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆಗಳು[],
	    ರಚನೆ fetch **fetch_ಹೊರಹೋಗು)
{
	ರಚನೆ fetch *fetch;
	lwc_string *scheme;

	fetch = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ (*fetch));
	ಆದರೆ (fetch == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಪ್ರತಿಜ್ಞಾ(scheme != ಖಾಲಿ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ಆದರೆ (fetch->fetcherd == -1) {
		ಬಿಡುಗಡೆ(fetch);
		ಹಿಂತಿರುಗಿ NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ಆದರೆ (referer != ಖಾಲಿ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.ಸಿದ್ಧತೆ(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						ಶೀರ್ಷಿಕೆಗಳು);
	ಆದರೆ (fetch->fetcher_handle == ಖಾಲಿ) {
		lwc_string_unref(fetch->host);

		ಆದರೆ (fetch->url != ಖಾಲಿ)
			nsurl_unref(fetch->url);

		ಆದರೆ (fetch->referer != ಖಾಲಿ)
			nsurl_unref(fetch->referer);

		ಬಿಡುಗಡೆ(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ಹಿಂತಿರುಗಿ NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_ಸೇರಿಸು(ಸರತಿ_ring, fetch);

	/* Ask the queue to run. */
	ಆದರೆ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->ವೇಳಾಪಟ್ಟಿ(10, fetcher_poll, ಖಾಲಿ);
	}

	*fetch_ಹೊರಹೋಗು = fetch;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_abort(ರಚನೆ fetch *f)
{
	ಪ್ರತಿಜ್ಞಾ(f);
	f->ಕೊನೆಯ_ಸಂದೇಶ = FETCH_ಆಂತರಿಕ_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_free(ರಚನೆ fetch *f)
{
	ಆದರೆ (f->ಕೊನೆಯ_ಸಂದೇಶ < FETCH_MIN_ಪೂರ್ಣಗೊಂಡ_ಸಂದೇಶ) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_ಸಂದೇಶ ಸಂದೇಶ;

		ಸಂದೇಶ.type = FETCH_ERROR;
		ಸಂದೇಶ.ದತ್ತಾಂಶ.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&ಸಂದೇಶ, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.ಬಿಡುಗಡೆ(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	ಆದರೆ (f->referer != ಖಾಲಿ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	ಬಿಡುಗಡೆ(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಪೂರ್ಣಾಂಕ fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ಆದರೆ (fetcherd == -1) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_change_callback(ರಚನೆ fetch *fetch,
			   fetch_callback callback,
			   ಶೂನ್ಯ *p)
{
	ಪ್ರತಿಜ್ಞಾ(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_ಸಂಕೇತ fetch_http_ಸಂಕೇತ(ರಚನೆ fetch *fetch)
{
	ಹಿಂತಿರುಗಿ fetch->http_ಸಂಕೇತ;
}


/* exported interface documented in content/fetch.h */
ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *
fetch_multipart_ದತ್ತಾಂಶ_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *list)
{
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *clone, *ಕೊನೆಯ = ಖಾಲಿ;
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *ಫಲಿತಾಂಶ = ಖಾಲಿ;

	ಗಾಗಿ (; list != ಖಾಲಿ; list = list->ಮುಂದಿನ) {
		clone = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ));
		ಆದರೆ (clone == ಖಾಲಿ) {
			ಆದರೆ (ಫಲಿತಾಂಶ != ಖಾಲಿ)
				fetch_multipart_ದತ್ತಾಂಶ_destroy(ಫಲಿತಾಂಶ);

			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ಆದರೆ (clone->name == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(clone);
			ಆದರೆ (ಫಲಿತಾಂಶ != ಖಾಲಿ)
				fetch_multipart_ದತ್ತಾಂಶ_destroy(ಫಲಿತಾಂಶ);

			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		clone->ಮೌಲ್ಯ = strdup(list->ಮೌಲ್ಯ);
		ಆದರೆ (clone->ಮೌಲ್ಯ == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(clone->name);
			ಬಿಡುಗಡೆ(clone);
			ಆದರೆ (ಫಲಿತಾಂಶ != ಖಾಲಿ)
				fetch_multipart_ದತ್ತಾಂಶ_destroy(ಫಲಿತಾಂಶ);

			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		ಆದರೆ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ಆದರೆ (clone->rawfile == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(clone->ಮೌಲ್ಯ);
				ಬಿಡುಗಡೆ(clone->name);
				ಬಿಡುಗಡೆ(clone);
				ಆದರೆ (ಫಲಿತಾಂಶ != ಖಾಲಿ)
					fetch_multipart_ದತ್ತಾಂಶ_destroy(ಫಲಿತಾಂಶ);

				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			clone->rawfile = ಖಾಲಿ;
		}

		clone->ಮುಂದಿನ = ಖಾಲಿ;

		ಆದರೆ (ಫಲಿತಾಂಶ == ಖಾಲಿ)
			ಫಲಿತಾಂಶ = clone;
		ಇಲ್ಲದಿದ್ದರೆ
			ಕೊನೆಯ->ಮುಂದಿನ = clone;

		ಕೊನೆಯ = clone;
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}


/* exported interface documented in content/fetch.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
fetch_multipart_ದತ್ತಾಂಶ_ಕಂಡುಹಿಡಿ(ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *list,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಆಗ (list != ಖಾಲಿ) {
		ಆದರೆ (strcmp(list->name, name) == 0) {
			ಹಿಂತಿರುಗಿ list->ಮೌಲ್ಯ;
		}
		list = list->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_multipart_ದತ್ತಾಂಶ_destroy(ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *list)
{
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *ಮುಂದಿನ;

	ಗಾಗಿ (; list != ಖಾಲಿ; list = ಮುಂದಿನ) {
		ಮುಂದಿನ = list->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(list->name);
		ಬಿಡುಗಡೆ(list->ಮೌಲ್ಯ);
		ಆದರೆ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			ಬಿಡುಗಡೆ(list->rawfile);
		}
		ಬಿಡುಗಡೆ(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_ದತ್ತಾಂಶ_new_kv(ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ **list,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *newdata;

	ಪ್ರತಿಜ್ಞಾ(list);

	newdata = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*newdata));

	ಆದರೆ (newdata == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ಆದರೆ (newdata->name == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(newdata);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	newdata->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
	ಆದರೆ (newdata->ಮೌಲ್ಯ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(newdata->name);
		ಬಿಡುಗಡೆ(newdata);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	newdata->ಮುಂದಿನ = *list;
	*list = newdata;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ
fetch_send_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ರಚನೆ fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ಆದರೆ (ಸಂದೇಶ->type > fetch->ಕೊನೆಯ_ಸಂದೇಶ)
		fetch->ಕೊನೆಯ_ಸಂದೇಶ = ಸಂದೇಶ->type;
	fetch->callback(ಸಂದೇಶ, fetch->p);
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_remove_from_queues(ರಚನೆ fetch *fetch)
{
	ಪೂರ್ಣಾಂಕ all_ಸಕ್ರಿಯ;
	ಪೂರ್ಣಾಂಕ all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ಆದರೆ (fetch->fetch_is_ಸಕ್ರಿಯ) {
		RING_REMOVE(fetch_ring, fetch);
	} ಇಲ್ಲದಿದ್ದರೆ {
		RING_REMOVE(ಸರತಿ_ring, fetch);
	}


	RING_GETSIZE(ರಚನೆ fetch, fetch_ring, all_ಸಕ್ರಿಯ);
	RING_GETSIZE(ರಚನೆ fetch, ಸರತಿ_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_ಸಕ್ರಿಯ);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_ಗಣ_http_ಸಂಕೇತ(ರಚನೆ fetch *fetch, http_response_ಸಂಕೇತ http_ಸಂಕೇತ)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_ಸಂಕೇತ);

	fetch->http_ಸಂಕೇತ = http_ಸಂಕೇತ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ಶೂನ್ಯ fetch_ಗಣ_cookie(ರಚನೆ fetch *fetch, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ)
{
	ಪ್ರತಿಜ್ಞಾ(fetch && ದತ್ತಾಂಶ);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ಆದರೆ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_ಗಣ_cookie(ದತ್ತಾಂಶ, fetch->url, ಖಾಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fetch->referer != ಖಾಲಿ) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_ಗಣ_cookie(ದತ್ತಾಂಶ, fetch->url, fetch->referer);
	}
}
