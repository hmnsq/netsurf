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

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/fetchers.h"
#అన్తర్భావ "content/fetchers/resource.h"
#అన్తర్భావ "content/fetchers/about/about.h"
#అన్తర్భావ "content/fetchers/curl.h"
#అన్తర్భావ "content/fetchers/data.h"
#అన్తర్భావ "content/fetchers/file/file.h"
#అన్తర్భావ "javascript/fetcher.h"
#అన్తర్భావ "content/urldb.h"

/** The maximum number of fetchers that can be added */
#నిర్వచించు MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#నిర్వచించు కాలపట్టిక_సమయం 10

/** The fdset timeout in ms */
#నిర్వచించు FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
రకనిర్వచనం నిర్మాణం scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	నిర్మాణం fetcher_operation_పట్టిక ops; /**< The fetchers operations. */
	పూర్ణాంకం refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

స్థిర scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
నిర్మాణం fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	శూన్య *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_సంకేతం http_సంకేతం;	/**< HTTP response code, or 0. */
	పూర్ణాంకం fetcherd;           /**< Fetcher descriptor for this fetch */
	శూన్య *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_చురుకైన;	/**< This fetch is active. */
	fetch_సందేశం_type చివరి_సందేశం;/**< The last message sent for this fetch */
	నిర్మాణం fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	నిర్మాణం fetch *r_తదుపరి;	/**< Next active fetch in ::fetch_ring. */
};

స్థిర నిర్మాణం fetch *fetch_ring = ఖాళీ;	/**< Ring of active fetches. */
స్థిర నిర్మాణం fetch *వరుస_ring = ఖాళీ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

స్థిర అంతఃవరుస శూన్య fetch_ref_fetcher(పూర్ణాంకం fetcherd)
{
	fetchers[fetcherd].refcount++;
}

స్థిర అంతఃవరుస శూన్య fetch_unref_fetcher(పూర్ణాంకం fetcherd)
{
	fetchers[fetcherd].refcount--;
	ఒకవేళ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
స్థిర పూర్ణాంకం get_fetcher_for_scheme(lwc_string *scheme)
{
	పూర్ణాంకం fetcherd;
	bool సరిపోలిక;

	కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ఒకవేళ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &సరిపోలిక) == lwc_error_ok) &&
		    (సరిపోలిక == true)) {
			తిరిగి fetcherd;
	       }
	}
	తిరిగి -1;
}

/**
 * Dispatch a single job
 */
స్థిర bool fetch_dispatch_పని(నిర్మాణం fetch *fetch)
{
	RING_REMOVE(వరుస_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ఒకవేళ (!fetchers[fetch->fetcherd].ops.ప్రారంభం(fetch->fetcher_handle)) {
		RING_చొప్పించు(వరుస_ring, fetch); /* Put it back on the end of the queue */
		తిరిగి false;
	} లేకపోతే {
		RING_చొప్పించు(fetch_ring, fetch);
		fetch->fetch_is_చురుకైన = true;
		తిరిగి true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
స్థిర bool fetch_ఎంచుకో_and_dispatch(శూన్య)
{
	bool same_host;
	నిర్మాణం fetch *queueitem;
	queueitem = వరుస_ring;
	చేయి {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		పూర్ణాంకం countbyhost;
		RING_COUNTBYLWCHOST(నిర్మాణం fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ఒకవేళ (countbyhost < nsoption_int(max_fetchers_చొప్పున_host)) {
			/* We can dispatch this item in theory */
			తిరిగి fetch_dispatch_పని(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		అయితే (same_host == true && queueitem->r_తదుపరి != వరుస_ring) {
			ఒకవేళ (lwc_string_isequal(queueitem->host,
					       queueitem->r_తదుపరి->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_తదుపరి;
			}
		}
		queueitem = queueitem->r_తదుపరి;
	} అయితే (queueitem != వరుస_ring);
	తిరిగి false;
}

స్థిర శూన్య dump_rings(శూన్య)
{
	నిర్మాణం fetch *q;
	నిర్మాణం fetch *f;

	q = వరుస_ring;
	ఒకవేళ (q) {
		చేయి {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_తదుపరి;
		} అయితే (q != వరుస_ring);
	}
	f = fetch_ring;
	ఒకవేళ (f) {
		చేయి {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_తదుపరి;
		} అయితే (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
స్థిర bool fetch_dispatch_jobs(శూన్య)
{
	పూర్ణాంకం all_చురుకైన;
	పూర్ణాంకం all_queued;

	RING_GETSIZE(నిర్మాణం fetch, వరుస_ring, all_queued);
	RING_GETSIZE(నిర్మాణం fetch, fetch_ring, all_చురుకైన);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_చురుకైన);
	dump_rings();

	అయితే ((all_queued != 0) &&
	       (all_చురుకైన < nsoption_int(max_fetchers)) &&
	       fetch_ఎంచుకో_and_dispatch()) {
			all_queued--;
			all_చురుకైన++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_చురుకైన);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_చురుకైన);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	తిరిగి (all_చురుకైన > 0);
}

స్థిర శూన్య fetcher_poll(శూన్య *వాడని)
{
	పూర్ణాంకం fetcherd;

	ఒకవేళ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ఒకవేళ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->కాలపట్టిక(కాలపట్టిక_సమయం, fetcher_poll, ఖాళీ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(శూన్య)
{
	nserror ret;

#నిర్వచితమైతే WITH_CURL
	ret = fetch_curl_రిజిస్టరు();
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}
#అయితే_ముగింపు

	ret = fetch_దత్తాంశం_రిజిస్టరు();
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	ret = fetch_file_రిజిస్టరు();
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	ret = fetch_resource_రిజిస్టరు();
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	ret = fetch_about_రిజిస్టరు();
	ఒకవేళ (ret != NSERROR_OK) {
		తిరిగి ret;
	}

	ret = fetch_javascript_రిజిస్టరు();

	తిరిగి ret;
}

/* exported interface documented in content/fetchers.h */
శూన్య fetcher_quit(శూన్య)
{
	పూర్ణాంకం fetcherd; /* fetcher index */
	కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ఒకవేళ (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_దత్తాంశం(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		ఒకవేళ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_కూడు(lwc_string *scheme, స్థిరరాశి నిర్మాణం fetcher_operation_పట్టిక *ops)
{
	పూర్ణాంకం fetcherd;

	/* find unused fetcher descriptor */
	కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ఒకవేళ (fetchers[fetcherd].refcount == 0) {
			ఆపు;
		}
	}
	ఒకవేళ (fetcherd == MAX_FETCHERS) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	ఒకవేళ (!ops->initialise(scheme)) {
		తిరిగి NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	తిరిగి NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_సమితి,
		      fd_set *write_fd_సమితి,
		      fd_set *except_fd_సమితి,
		      పూర్ణాంకం *maxfd_బహిర్గతం)
{
	పూర్ణాంకం maxfd = -1;
	పూర్ణాంకం fetcherd; /* fetcher index */

	ఒకవేళ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_బహిర్గతం = -1;
		తిరిగి NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ఒకవేళ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_సమితి);
	FD_ZERO(write_fd_సమితి);
	FD_ZERO(except_fd_సమితి);

	కొరకు (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ఒకవేళ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ఖాళీ)) {
			/* fetcher present */
			పూర్ణాంకం fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_సమితి,
				write_fd_సమితి, except_fd_సమితి);
			ఒకవేళ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ఒకవేళ (maxfd >= 0) {
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
		guit->misc->కాలపట్టిక(FDSET_TIMEOUT, fetcher_poll, ఖాళీ);
	}

	*maxfd_బహిర్గతం = maxfd;

	తిరిగి NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_ప్రారంభం(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    శూన్య *p,
	    bool only_2xx,
	    స్థిరరాశి అక్షరం *post_urlenc,
	    స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    స్థిరరాశి అక్షరం *శీర్షికలు[],
	    నిర్మాణం fetch **fetch_బహిర్గతం)
{
	నిర్మాణం fetch *fetch;
	lwc_string *scheme;

	fetch = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం (*fetch));
	ఒకవేళ (fetch == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ప్రతిజ్ఞా(scheme != ఖాళీ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ఒకవేళ (fetch->fetcherd == -1) {
		విడుదల(fetch);
		తిరిగి NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ఒకవేళ (referer != ఖాళీ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.అమరిక(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						శీర్షికలు);
	ఒకవేళ (fetch->fetcher_handle == ఖాళీ) {
		lwc_string_unref(fetch->host);

		ఒకవేళ (fetch->url != ఖాళీ)
			nsurl_unref(fetch->url);

		ఒకవేళ (fetch->referer != ఖాళీ)
			nsurl_unref(fetch->referer);

		విడుదల(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		తిరిగి NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_చొప్పించు(వరుస_ring, fetch);

	/* Ask the queue to run. */
	ఒకవేళ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->కాలపట్టిక(10, fetcher_poll, ఖాళీ);
	}

	*fetch_బహిర్గతం = fetch;
	తిరిగి NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
శూన్య fetch_abort(నిర్మాణం fetch *f)
{
	ప్రతిజ్ఞా(f);
	f->చివరి_సందేశం = FETCH_అంతర్గత_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
శూన్య fetch_free(నిర్మాణం fetch *f)
{
	ఒకవేళ (f->చివరి_సందేశం < FETCH_MIN_ముగిసిన_సందేశం) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_సందేశం సందేశం;

		సందేశం.type = FETCH_ERROR;
		సందేశం.దత్తాంశం.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&సందేశం, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.విడుదల(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	ఒకవేళ (f->referer != ఖాళీ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	విడుదల(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(స్థిరరాశి nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	పూర్ణాంకం fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ఒకవేళ (fetcherd == -1) {
		తిరిగి false;
	}

	తిరిగి fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
శూన్య fetch_change_callback(నిర్మాణం fetch *fetch,
			   fetch_callback callback,
			   శూన్య *p)
{
	ప్రతిజ్ఞా(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_సంకేతం fetch_http_సంకేతం(నిర్మాణం fetch *fetch)
{
	తిరిగి fetch->http_సంకేతం;
}


/* exported interface documented in content/fetch.h */
నిర్మాణం fetch_multipart_దత్తాంశం *
fetch_multipart_దత్తాంశం_clone(స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *list)
{
	నిర్మాణం fetch_multipart_దత్తాంశం *clone, *చివరి = ఖాళీ;
	నిర్మాణం fetch_multipart_దత్తాంశం *ఫలితం = ఖాళీ;

	కొరకు (; list != ఖాళీ; list = list->తదుపరి) {
		clone = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం fetch_multipart_దత్తాంశం));
		ఒకవేళ (clone == ఖాళీ) {
			ఒకవేళ (ఫలితం != ఖాళీ)
				fetch_multipart_దత్తాంశం_destroy(ఫలితం);

			తిరిగి ఖాళీ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ఒకవేళ (clone->name == ఖాళీ) {
			విడుదల(clone);
			ఒకవేళ (ఫలితం != ఖాళీ)
				fetch_multipart_దత్తాంశం_destroy(ఫలితం);

			తిరిగి ఖాళీ;
		}

		clone->విలువ = strdup(list->విలువ);
		ఒకవేళ (clone->విలువ == ఖాళీ) {
			విడుదల(clone->name);
			విడుదల(clone);
			ఒకవేళ (ఫలితం != ఖాళీ)
				fetch_multipart_దత్తాంశం_destroy(ఫలితం);

			తిరిగి ఖాళీ;
		}

		ఒకవేళ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ఒకవేళ (clone->rawfile == ఖాళీ) {
				విడుదల(clone->విలువ);
				విడుదల(clone->name);
				విడుదల(clone);
				ఒకవేళ (ఫలితం != ఖాళీ)
					fetch_multipart_దత్తాంశం_destroy(ఫలితం);

				తిరిగి ఖాళీ;
			}
		} లేకపోతే {
			clone->rawfile = ఖాళీ;
		}

		clone->తదుపరి = ఖాళీ;

		ఒకవేళ (ఫలితం == ఖాళీ)
			ఫలితం = clone;
		లేకపోతే
			చివరి->తదుపరి = clone;

		చివరి = clone;
	}

	తిరిగి ఫలితం;
}


/* exported interface documented in content/fetch.h */
స్థిరరాశి అక్షరం *
fetch_multipart_దత్తాంశం_కనుగొను(స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *list,
			  స్థిరరాశి అక్షరం *name)
{
	అయితే (list != ఖాళీ) {
		ఒకవేళ (strcmp(list->name, name) == 0) {
			తిరిగి list->విలువ;
		}
		list = list->తదుపరి;
	}

	తిరిగి ఖాళీ;
}


/* exported interface documented in content/fetch.h */
శూన్య fetch_multipart_దత్తాంశం_destroy(నిర్మాణం fetch_multipart_దత్తాంశం *list)
{
	నిర్మాణం fetch_multipart_దత్తాంశం *తదుపరి;

	కొరకు (; list != ఖాళీ; list = తదుపరి) {
		తదుపరి = list->తదుపరి;
		విడుదల(list->name);
		విడుదల(list->విలువ);
		ఒకవేళ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			విడుదల(list->rawfile);
		}
		విడుదల(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_దత్తాంశం_new_kv(నిర్మాణం fetch_multipart_దత్తాంశం **list,
			    స్థిరరాశి అక్షరం *name,
			    స్థిరరాశి అక్షరం *విలువ)
{
	నిర్మాణం fetch_multipart_దత్తాంశం *newdata;

	ప్రతిజ్ఞా(list);

	newdata = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*newdata));

	ఒకవేళ (newdata == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ఒకవేళ (newdata->name == ఖాళీ) {
		విడుదల(newdata);
		తిరిగి NSERROR_NOMEM;
	}

	newdata->విలువ = strdup(విలువ);
	ఒకవేళ (newdata->విలువ == ఖాళీ) {
		విడుదల(newdata->name);
		విడుదల(newdata);
		తిరిగి NSERROR_NOMEM;
	}

	newdata->తదుపరి = *list;
	*list = newdata;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
శూన్య
fetch_send_callback(స్థిరరాశి fetch_సందేశం *సందేశం, నిర్మాణం fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ఒకవేళ (సందేశం->type > fetch->చివరి_సందేశం)
		fetch->చివరి_సందేశం = సందేశం->type;
	fetch->callback(సందేశం, fetch->p);
}


/* exported interface documented in content/fetch.h */
శూన్య fetch_remove_from_queues(నిర్మాణం fetch *fetch)
{
	పూర్ణాంకం all_చురుకైన;
	పూర్ణాంకం all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ఒకవేళ (fetch->fetch_is_చురుకైన) {
		RING_REMOVE(fetch_ring, fetch);
	} లేకపోతే {
		RING_REMOVE(వరుస_ring, fetch);
	}


	RING_GETSIZE(నిర్మాణం fetch, fetch_ring, all_చురుకైన);
	RING_GETSIZE(నిర్మాణం fetch, వరుస_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_చురుకైన);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_సమితి_http_సంకేతం(నిర్మాణం fetch *fetch, http_response_సంకేతం http_సంకేతం)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_సంకేతం);

	fetch->http_సంకేతం = http_సంకేతం;

	తిరిగి NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
శూన్య fetch_సమితి_cookie(నిర్మాణం fetch *fetch, స్థిరరాశి అక్షరం *దత్తాంశం)
{
	ప్రతిజ్ఞా(fetch && దత్తాంశం);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ఒకవేళ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_సమితి_cookie(దత్తాంశం, fetch->url, ఖాళీ);
	} లేకపోతే ఒకవేళ (fetch->referer != ఖాళీ) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_సమితి_cookie(దత్తాంశం, fetch->url, fetch->referer);
	}
}
