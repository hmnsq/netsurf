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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/resource.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/about/about.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/curl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/data.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetchers/file/file.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/fetcher.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/urldb.h"

/** The maximum number of fetchers that can be added */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮਾਂ_ਸਾਰਣੀ_ਸਮਾਂ 10

/** The fdset timeout in ms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ ops; /**< The fetchers operations. */
	ਪੂਰਨ refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ਸਥਿਰ scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ਬਣਤਰ fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	ਖਾਲੀ *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_ਕੋਡ http_ਕੋਡ;	/**< HTTP response code, or 0. */
	ਪੂਰਨ fetcherd;           /**< Fetcher descriptor for this fetch */
	ਖਾਲੀ *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_ਸਰਗਰਮ;	/**< This fetch is active. */
	fetch_ਸੁਨੇਹਾ_type ਆਖਰੀ_ਸੁਨੇਹਾ;/**< The last message sent for this fetch */
	ਬਣਤਰ fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	ਬਣਤਰ fetch *r_ਅਗਲਾ;	/**< Next active fetch in ::fetch_ring. */
};

ਸਥਿਰ ਬਣਤਰ fetch *fetch_ring = ਨਿਰਾ;	/**< Ring of active fetches. */
ਸਥਿਰ ਬਣਤਰ fetch *ਕਤਾਰ_ring = ਨਿਰਾ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ fetch_ref_fetcher(ਪੂਰਨ fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ fetch_unref_fetcher(ਪੂਰਨ fetcherd)
{
	fetchers[fetcherd].refcount--;
	ਜੇ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ਸਥਿਰ ਪੂਰਨ get_fetcher_for_scheme(lwc_string *scheme)
{
	ਪੂਰਨ fetcherd;
	bool ਮੇਲ;

	ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ਜੇ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &ਮੇਲ) == lwc_error_ok) &&
		    (ਮੇਲ == true)) {
			ਵਾਪਸ fetcherd;
	       }
	}
	ਵਾਪਸ -1;
}

/**
 * Dispatch a single job
 */
ਸਥਿਰ bool fetch_dispatch_ਕੰਮ(ਬਣਤਰ fetch *fetch)
{
	RING_REMOVE(ਕਤਾਰ_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ਜੇ (!fetchers[fetch->fetcherd].ops.ਸ਼ੁਰੂ(fetch->fetcher_handle)) {
		RING_ਪਾਓ(ਕਤਾਰ_ring, fetch); /* Put it back on the end of the queue */
		ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		RING_ਪਾਓ(fetch_ring, fetch);
		fetch->fetch_is_ਸਰਗਰਮ = true;
		ਵਾਪਸ true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ਸਥਿਰ bool fetch_ਚੁਣੋ_and_dispatch(ਖਾਲੀ)
{
	bool same_host;
	ਬਣਤਰ fetch *queueitem;
	queueitem = ਕਤਾਰ_ring;
	ਕਰੋ {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		ਪੂਰਨ countbyhost;
		RING_COUNTBYLWCHOST(ਬਣਤਰ fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ਜੇ (countbyhost < nsoption_int(max_fetchers_ਪ੍ਰਤੀ_host)) {
			/* We can dispatch this item in theory */
			ਵਾਪਸ fetch_dispatch_ਕੰਮ(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		ਜਦੋਂ (same_host == true && queueitem->r_ਅਗਲਾ != ਕਤਾਰ_ring) {
			ਜੇ (lwc_string_isequal(queueitem->host,
					       queueitem->r_ਅਗਲਾ->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_ਅਗਲਾ;
			}
		}
		queueitem = queueitem->r_ਅਗਲਾ;
	} ਜਦੋਂ (queueitem != ਕਤਾਰ_ring);
	ਵਾਪਸ false;
}

ਸਥਿਰ ਖਾਲੀ dump_rings(ਖਾਲੀ)
{
	ਬਣਤਰ fetch *q;
	ਬਣਤਰ fetch *f;

	q = ਕਤਾਰ_ring;
	ਜੇ (q) {
		ਕਰੋ {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_ਅਗਲਾ;
		} ਜਦੋਂ (q != ਕਤਾਰ_ring);
	}
	f = fetch_ring;
	ਜੇ (f) {
		ਕਰੋ {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_ਅਗਲਾ;
		} ਜਦੋਂ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ਸਥਿਰ bool fetch_dispatch_jobs(ਖਾਲੀ)
{
	ਪੂਰਨ all_ਸਰਗਰਮ;
	ਪੂਰਨ all_queued;

	RING_GETSIZE(ਬਣਤਰ fetch, ਕਤਾਰ_ring, all_queued);
	RING_GETSIZE(ਬਣਤਰ fetch, fetch_ring, all_ਸਰਗਰਮ);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_ਸਰਗਰਮ);
	dump_rings();

	ਜਦੋਂ ((all_queued != 0) &&
	       (all_ਸਰਗਰਮ < nsoption_int(max_fetchers)) &&
	       fetch_ਚੁਣੋ_and_dispatch()) {
			all_queued--;
			all_ਸਰਗਰਮ++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_ਸਰਗਰਮ);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_ਸਰਗਰਮ);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	ਵਾਪਸ (all_ਸਰਗਰਮ > 0);
}

ਸਥਿਰ ਖਾਲੀ fetcher_poll(ਖਾਲੀ *ਅਣਵਰਤਿਆ)
{
	ਪੂਰਨ fetcherd;

	ਜੇ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ਜੇ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(ਸਮਾਂ_ਸਾਰਣੀ_ਸਮਾਂ, fetcher_poll, ਨਿਰਾ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(ਖਾਲੀ)
{
	nserror ret;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WITH_CURL
	ret = fetch_curl_ਰਜਿਸਟਰ();
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}
#ਜੇ_ਅੰਤ

	ret = fetch_ਅੰਕੜੇ_ਰਜਿਸਟਰ();
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ret = fetch_file_ਰਜਿਸਟਰ();
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ret = fetch_resource_ਰਜਿਸਟਰ();
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ret = fetch_about_ਰਜਿਸਟਰ();
	ਜੇ (ret != NSERROR_OK) {
		ਵਾਪਸ ret;
	}

	ret = fetch_javascript_ਰਜਿਸਟਰ();

	ਵਾਪਸ ret;
}

/* exported interface documented in content/fetchers.h */
ਖਾਲੀ fetcher_quit(ਖਾਲੀ)
{
	ਪੂਰਨ fetcherd; /* fetcher index */
	ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ਜੇ (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_ਅੰਕੜੇ(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		ਜੇ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_ਜੋੜੋ(lwc_string *scheme, ਅਚਲ ਬਣਤਰ fetcher_operation_ਸਾਰਣੀ *ops)
{
	ਪੂਰਨ fetcherd;

	/* find unused fetcher descriptor */
	ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ਜੇ (fetchers[fetcherd].refcount == 0) {
			ਰੋਕੋ;
		}
	}
	ਜੇ (fetcherd == MAX_FETCHERS) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	ਜੇ (!ops->initialise(scheme)) {
		ਵਾਪਸ NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_ਸੈੱਟ,
		      fd_set *write_fd_ਸੈੱਟ,
		      fd_set *except_fd_ਸੈੱਟ,
		      ਪੂਰਨ *maxfd_ਬਾਹਰਲਾ)
{
	ਪੂਰਨ maxfd = -1;
	ਪੂਰਨ fetcherd; /* fetcher index */

	ਜੇ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_ਬਾਹਰਲਾ = -1;
		ਵਾਪਸ NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ਜੇ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_ਸੈੱਟ);
	FD_ZERO(write_fd_ਸੈੱਟ);
	FD_ZERO(except_fd_ਸੈੱਟ);

	ਲਈ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ਜੇ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ਨਿਰਾ)) {
			/* fetcher present */
			ਪੂਰਨ fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_ਸੈੱਟ,
				write_fd_ਸੈੱਟ, except_fd_ਸੈੱਟ);
			ਜੇ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ਜੇ (maxfd >= 0) {
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
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(FDSET_TIMEOUT, fetcher_poll, ਨਿਰਾ);
	}

	*maxfd_ਬਾਹਰਲਾ = maxfd;

	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_ਸ਼ੁਰੂ(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    ਖਾਲੀ *p,
	    bool only_2xx,
	    ਅਚਲ ਅੱਖਰ *post_urlenc,
	    ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ਅਚਲ ਅੱਖਰ *ਸਿਰਲੇਖਾਂ[],
	    ਬਣਤਰ fetch **fetch_ਬਾਹਰਲਾ)
{
	ਬਣਤਰ fetch *fetch;
	lwc_string *scheme;

	fetch = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ (*fetch));
	ਜੇ (fetch == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਪ੍ਰਤਿਜ੍ਞਾ(scheme != ਨਿਰਾ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ਜੇ (fetch->fetcherd == -1) {
		ਮੁਕਤ(fetch);
		ਵਾਪਸ NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ਜੇ (referer != ਨਿਰਾ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.ਤਿਆਰੀ(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						ਸਿਰਲੇਖਾਂ);
	ਜੇ (fetch->fetcher_handle == ਨਿਰਾ) {
		lwc_string_unref(fetch->host);

		ਜੇ (fetch->url != ਨਿਰਾ)
			nsurl_unref(fetch->url);

		ਜੇ (fetch->referer != ਨਿਰਾ)
			nsurl_unref(fetch->referer);

		ਮੁਕਤ(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ਵਾਪਸ NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_ਪਾਓ(ਕਤਾਰ_ring, fetch);

	/* Ask the queue to run. */
	ਜੇ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(10, fetcher_poll, ਨਿਰਾ);
	}

	*fetch_ਬਾਹਰਲਾ = fetch;
	ਵਾਪਸ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_abort(ਬਣਤਰ fetch *f)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(f);
	f->ਆਖਰੀ_ਸੁਨੇਹਾ = FETCH_ਅੰਦਰੂਨੀ_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_free(ਬਣਤਰ fetch *f)
{
	ਜੇ (f->ਆਖਰੀ_ਸੁਨੇਹਾ < FETCH_MIN_ਮੁਕੰਮਲ_ਸੁਨੇਹਾ) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_ਸੁਨੇਹਾ ਸੁਨੇਹਾ;

		ਸੁਨੇਹਾ.type = FETCH_ERROR;
		ਸੁਨੇਹਾ.ਅੰਕੜੇ.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&ਸੁਨੇਹਾ, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.ਮੁਕਤ(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	ਜੇ (f->referer != ਨਿਰਾ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	ਮੁਕਤ(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ਅਚਲ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਪੂਰਨ fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ਜੇ (fetcherd == -1) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_change_callback(ਬਣਤਰ fetch *fetch,
			   fetch_callback callback,
			   ਖਾਲੀ *p)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_ਕੋਡ fetch_http_ਕੋਡ(ਬਣਤਰ fetch *fetch)
{
	ਵਾਪਸ fetch->http_ਕੋਡ;
}


/* exported interface documented in content/fetch.h */
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *
fetch_multipart_ਅੰਕੜੇ_clone(ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list)
{
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *clone, *ਆਖਰੀ = ਨਿਰਾ;
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *ਨਤੀਜਾ = ਨਿਰਾ;

	ਲਈ (; list != ਨਿਰਾ; list = list->ਅਗਲਾ) {
		clone = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ));
		ਜੇ (clone == ਨਿਰਾ) {
			ਜੇ (ਨਤੀਜਾ != ਨਿਰਾ)
				fetch_multipart_ਅੰਕੜੇ_destroy(ਨਤੀਜਾ);

			ਵਾਪਸ ਨਿਰਾ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ਜੇ (clone->name == ਨਿਰਾ) {
			ਮੁਕਤ(clone);
			ਜੇ (ਨਤੀਜਾ != ਨਿਰਾ)
				fetch_multipart_ਅੰਕੜੇ_destroy(ਨਤੀਜਾ);

			ਵਾਪਸ ਨਿਰਾ;
		}

		clone->ਮੁੱਲ = strdup(list->ਮੁੱਲ);
		ਜੇ (clone->ਮੁੱਲ == ਨਿਰਾ) {
			ਮੁਕਤ(clone->name);
			ਮੁਕਤ(clone);
			ਜੇ (ਨਤੀਜਾ != ਨਿਰਾ)
				fetch_multipart_ਅੰਕੜੇ_destroy(ਨਤੀਜਾ);

			ਵਾਪਸ ਨਿਰਾ;
		}

		ਜੇ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ਜੇ (clone->rawfile == ਨਿਰਾ) {
				ਮੁਕਤ(clone->ਮੁੱਲ);
				ਮੁਕਤ(clone->name);
				ਮੁਕਤ(clone);
				ਜੇ (ਨਤੀਜਾ != ਨਿਰਾ)
					fetch_multipart_ਅੰਕੜੇ_destroy(ਨਤੀਜਾ);

				ਵਾਪਸ ਨਿਰਾ;
			}
		} ਨਹੀਂਤਾਂ {
			clone->rawfile = ਨਿਰਾ;
		}

		clone->ਅਗਲਾ = ਨਿਰਾ;

		ਜੇ (ਨਤੀਜਾ == ਨਿਰਾ)
			ਨਤੀਜਾ = clone;
		ਨਹੀਂਤਾਂ
			ਆਖਰੀ->ਅਗਲਾ = clone;

		ਆਖਰੀ = clone;
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}


/* exported interface documented in content/fetch.h */
ਅਚਲ ਅੱਖਰ *
fetch_multipart_ਅੰਕੜੇ_ਲੱਭੋ(ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list,
			  ਅਚਲ ਅੱਖਰ *name)
{
	ਜਦੋਂ (list != ਨਿਰਾ) {
		ਜੇ (strcmp(list->name, name) == 0) {
			ਵਾਪਸ list->ਮੁੱਲ;
		}
		list = list->ਅਗਲਾ;
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_multipart_ਅੰਕੜੇ_destroy(ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *list)
{
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *ਅਗਲਾ;

	ਲਈ (; list != ਨਿਰਾ; list = ਅਗਲਾ) {
		ਅਗਲਾ = list->ਅਗਲਾ;
		ਮੁਕਤ(list->name);
		ਮੁਕਤ(list->ਮੁੱਲ);
		ਜੇ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			ਮੁਕਤ(list->rawfile);
		}
		ਮੁਕਤ(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_ਅੰਕੜੇ_new_kv(ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ **list,
			    ਅਚਲ ਅੱਖਰ *name,
			    ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *newdata;

	ਪ੍ਰਤਿਜ੍ਞਾ(list);

	newdata = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*newdata));

	ਜੇ (newdata == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ਜੇ (newdata->name == ਨਿਰਾ) {
		ਮੁਕਤ(newdata);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	newdata->ਮੁੱਲ = strdup(ਮੁੱਲ);
	ਜੇ (newdata->ਮੁੱਲ == ਨਿਰਾ) {
		ਮੁਕਤ(newdata->name);
		ਮੁਕਤ(newdata);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	newdata->ਅਗਲਾ = *list;
	*list = newdata;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ਖਾਲੀ
fetch_send_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਬਣਤਰ fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ਜੇ (ਸੁਨੇਹਾ->type > fetch->ਆਖਰੀ_ਸੁਨੇਹਾ)
		fetch->ਆਖਰੀ_ਸੁਨੇਹਾ = ਸੁਨੇਹਾ->type;
	fetch->callback(ਸੁਨੇਹਾ, fetch->p);
}


/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_remove_from_queues(ਬਣਤਰ fetch *fetch)
{
	ਪੂਰਨ all_ਸਰਗਰਮ;
	ਪੂਰਨ all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ਜੇ (fetch->fetch_is_ਸਰਗਰਮ) {
		RING_REMOVE(fetch_ring, fetch);
	} ਨਹੀਂਤਾਂ {
		RING_REMOVE(ਕਤਾਰ_ring, fetch);
	}


	RING_GETSIZE(ਬਣਤਰ fetch, fetch_ring, all_ਸਰਗਰਮ);
	RING_GETSIZE(ਬਣਤਰ fetch, ਕਤਾਰ_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_ਸਰਗਰਮ);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_ਸੈੱਟ_http_ਕੋਡ(ਬਣਤਰ fetch *fetch, http_response_ਕੋਡ http_ਕੋਡ)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_ਕੋਡ);

	fetch->http_ਕੋਡ = http_ਕੋਡ;

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ਖਾਲੀ fetch_ਸੈੱਟ_cookie(ਬਣਤਰ fetch *fetch, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(fetch && ਅੰਕੜੇ);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ਜੇ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_ਸੈੱਟ_cookie(ਅੰਕੜੇ, fetch->url, ਨਿਰਾ);
	} ਨਹੀਂਤਾਂ ਜੇ (fetch->referer != ਨਿਰਾ) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_ਸੈੱਟ_cookie(ਅੰਕੜੇ, fetch->url, fetch->referer);
	}
}
