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

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "utils/ring.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "content/fetch.h"
#অন্তর্ভাৱ "content/fetchers.h"
#অন্তর্ভাৱ "content/fetchers/resource.h"
#অন্তর্ভাৱ "content/fetchers/about/about.h"
#অন্তর্ভাৱ "content/fetchers/curl.h"
#অন্তর্ভাৱ "content/fetchers/data.h"
#অন্তর্ভাৱ "content/fetchers/file/file.h"
#অন্তর্ভাৱ "javascript/fetcher.h"
#অন্তর্ভাৱ "content/urldb.h"

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
প্রকারনির্দেশ সংরচনা scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	সংরচনা fetcher_operation_table ops; /**< The fetchers operations. */
	পূর্ণাঙ্ক refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

স্থির scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
সংরচনা fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	শূন্য *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	পূর্ণাঙ্ক fetcherd;           /**< Fetcher descriptor for this fetch */
	শূন্য *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_বাৰ্তা_type last_বাৰ্তা;/**< The last message sent for this fetch */
	সংরচনা fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	সংরচনা fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

স্থির সংরচনা fetch *fetch_ring = রিক্ত;	/**< Ring of active fetches. */
স্থির সংরচনা fetch *queue_ring = রিক্ত;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

স্থির অন্তঃস্থ শূন্য fetch_ref_fetcher(পূর্ণাঙ্ক fetcherd)
{
	fetchers[fetcherd].refcount++;
}

স্থির অন্তঃস্থ শূন্য fetch_unref_fetcher(পূর্ণাঙ্ক fetcherd)
{
	fetchers[fetcherd].refcount--;
	যদি (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
স্থির পূর্ণাঙ্ক get_fetcher_for_scheme(lwc_string *scheme)
{
	পূর্ণাঙ্ক fetcherd;
	bool match;

	বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		যদি ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			ঘূৰাই fetcherd;
	       }
	}
	ঘূৰাই -1;
}

/**
 * Dispatch a single job
 */
স্থির bool fetch_dispatch_job(সংরচনা fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	যদি (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_ঢোকাও(queue_ring, fetch); /* Put it back on the end of the queue */
		ঘূৰাই false;
	} অন্যথা {
		RING_ঢোকাও(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		ঘূৰাই true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
স্থির bool fetch_choose_and_dispatch(শূন্য)
{
	bool same_host;
	সংরচনা fetch *queueitem;
	queueitem = queue_ring;
	কৰা {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		পূর্ণাঙ্ক countbyhost;
		RING_COUNTBYLWCHOST(সংরচনা fetch, fetch_ring, countbyhost,
				    queueitem->host);
		যদি (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			ঘূৰাই fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		যেতিয়ালৈ (same_host == true && queueitem->r_next != queue_ring) {
			যদি (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} যেতিয়ালৈ (queueitem != queue_ring);
	ঘূৰাই false;
}

স্থির শূন্য dump_rings(শূন্য)
{
	সংরচনা fetch *q;
	সংরচনা fetch *f;

	q = queue_ring;
	যদি (q) {
		কৰা {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} যেতিয়ালৈ (q != queue_ring);
	}
	f = fetch_ring;
	যদি (f) {
		কৰা {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} যেতিয়ালৈ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
স্থির bool fetch_dispatch_jobs(শূন্য)
{
	পূর্ণাঙ্ক all_active;
	পূর্ণাঙ্ক all_queued;

	RING_GETSIZE(সংরচনা fetch, queue_ring, all_queued);
	RING_GETSIZE(সংরচনা fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	যেতিয়ালৈ ((all_queued != 0) &&
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

	ঘূৰাই (all_active > 0);
}

স্থির শূন্য fetcher_poll(শূন্য *unused)
{
	পূর্ণাঙ্ক fetcherd;

	যদি (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			যদি (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, রিক্ত);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(শূন্য)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}
#endif

	ret = fetch_data_register();
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}

	ret = fetch_file_register();
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}

	ret = fetch_resource_register();
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}

	ret = fetch_about_register();
	যদি (ret != NSERROR_OK) {
		ঘূৰাই ret;
	}

	ret = fetch_javascript_register();

	ঘূৰাই ret;
}

/* exported interface documented in content/fetchers.h */
শূন্য fetcher_quit(শূন্য)
{
	পূর্ণাঙ্ক fetcherd; /* fetcher index */
	বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		যদি (fetchers[fetcherd].refcount > 1) {
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
		যদি (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, ধ্রুৱ সংরচনা fetcher_operation_table *ops)
{
	পূর্ণাঙ্ক fetcherd;

	/* find unused fetcher descriptor */
	বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		যদি (fetchers[fetcherd].refcount == 0) {
			ভঙ্গ;
		}
	}
	যদি (fetcherd == MAX_FETCHERS) {
		ঘূৰাই NSERROR_INIT_FAILED;
	}

	যদি (!ops->initialise(scheme)) {
		ঘূৰাই NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      পূর্ণাঙ্ক *maxfd_out)
{
	পূর্ণাঙ্ক maxfd = -1;
	পূর্ণাঙ্ক fetcherd; /* fetcher index */

	যদি (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		ঘূৰাই NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		যদি (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	বাবে (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		যদি ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != রিক্ত)) {
			/* fetcher present */
			পূর্ণাঙ্ক fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			যদি (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	যদি (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, রিক্ত);
	}

	*maxfd_out = maxfd;

	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    শূন্য *p,
	    bool only_2xx,
	    ধ্রুৱ ৱর্ণ *post_urlenc,
	    ধ্রুৱ সংরচনা fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ধ্রুৱ ৱর্ণ *headers[],
	    সংরচনা fetch **fetch_out)
{
	সংরচনা fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, পরিমাণ (*fetch));
	যদি (fetch == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	প্রতিজ্ঞা(scheme != রিক্ত);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	যদি (fetch->fetcherd == -1) {
		মুক্ত(fetch);
		ঘূৰাই NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	যদি (referer != রিক্ত) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	যদি (fetch->fetcher_handle == রিক্ত) {
		lwc_string_unref(fetch->host);

		যদি (fetch->url != রিক্ত)
			nsurl_unref(fetch->url);

		যদি (fetch->referer != রিক্ত)
			nsurl_unref(fetch->referer);

		মুক্ত(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ঘূৰাই NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_ঢোকাও(queue_ring, fetch);

	/* Ask the queue to run. */
	যদি (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, রিক্ত);
	}

	*fetch_out = fetch;
	ঘূৰাই NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
শূন্য fetch_abort(সংরচনা fetch *f)
{
	প্রতিজ্ঞা(f);
	f->last_বাৰ্তা = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
শূন্য fetch_free(সংরচনা fetch *f)
{
	যদি (f->last_বাৰ্তা < FETCH_MIN_FINISHED_বাৰ্তা) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_বাৰ্তা বাৰ্তা;

		বাৰ্তা.type = FETCH_ERROR;
		বাৰ্তা.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&বাৰ্তা, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.মুক্ত(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	যদি (f->referer != রিক্ত) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	মুক্ত(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ধ্রুৱ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	পূর্ণাঙ্ক fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	যদি (fetcherd == -1) {
		ঘূৰাই false;
	}

	ঘূৰাই fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
শূন্য fetch_change_callback(সংরচনা fetch *fetch,
			   fetch_callback callback,
			   শূন্য *p)
{
	প্রতিজ্ঞা(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(সংরচনা fetch *fetch)
{
	ঘূৰাই fetch->http_code;
}


/* exported interface documented in content/fetch.h */
সংরচনা fetch_multipart_data *
fetch_multipart_data_clone(ধ্রুৱ সংরচনা fetch_multipart_data *list)
{
	সংরচনা fetch_multipart_data *clone, *last = রিক্ত;
	সংরচনা fetch_multipart_data *result = রিক্ত;

	বাবে (; list != রিক্ত; list = list->next) {
		clone = malloc(পরিমাণ(সংরচনা fetch_multipart_data));
		যদি (clone == রিক্ত) {
			যদি (result != রিক্ত)
				fetch_multipart_data_destroy(result);

			ঘূৰাই রিক্ত;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		যদি (clone->name == রিক্ত) {
			মুক্ত(clone);
			যদি (result != রিক্ত)
				fetch_multipart_data_destroy(result);

			ঘূৰাই রিক্ত;
		}

		clone->মান = strdup(list->মান);
		যদি (clone->মান == রিক্ত) {
			মুক্ত(clone->name);
			মুক্ত(clone);
			যদি (result != রিক্ত)
				fetch_multipart_data_destroy(result);

			ঘূৰাই রিক্ত;
		}

		যদি (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			যদি (clone->rawfile == রিক্ত) {
				মুক্ত(clone->মান);
				মুক্ত(clone->name);
				মুক্ত(clone);
				যদি (result != রিক্ত)
					fetch_multipart_data_destroy(result);

				ঘূৰাই রিক্ত;
			}
		} অন্যথা {
			clone->rawfile = রিক্ত;
		}

		clone->next = রিক্ত;

		যদি (result == রিক্ত)
			result = clone;
		অন্যথা
			last->next = clone;

		last = clone;
	}

	ঘূৰাই result;
}


/* exported interface documented in content/fetch.h */
ধ্রুৱ ৱর্ণ *
fetch_multipart_data_find(ধ্রুৱ সংরচনা fetch_multipart_data *list,
			  ধ্রুৱ ৱর্ণ *name)
{
	যেতিয়ালৈ (list != রিক্ত) {
		যদি (strcmp(list->name, name) == 0) {
			ঘূৰাই list->মান;
		}
		list = list->next;
	}

	ঘূৰাই রিক্ত;
}


/* exported interface documented in content/fetch.h */
শূন্য fetch_multipart_data_destroy(সংরচনা fetch_multipart_data *list)
{
	সংরচনা fetch_multipart_data *next;

	বাবে (; list != রিক্ত; list = next) {
		next = list->next;
		মুক্ত(list->name);
		মুক্ত(list->মান);
		যদি (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			মুক্ত(list->rawfile);
		}
		মুক্ত(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(সংরচনা fetch_multipart_data **list,
			    ধ্রুৱ ৱর্ণ *name,
			    ধ্রুৱ ৱর্ণ *মান)
{
	সংরচনা fetch_multipart_data *newdata;

	প্রতিজ্ঞা(list);

	newdata = calloc(1, পরিমাণ(*newdata));

	যদি (newdata == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	যদি (newdata->name == রিক্ত) {
		মুক্ত(newdata);
		ঘূৰাই NSERROR_NOMEM;
	}

	newdata->মান = strdup(মান);
	যদি (newdata->মান == রিক্ত) {
		মুক্ত(newdata->name);
		মুক্ত(newdata);
		ঘূৰাই NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	ঘূৰাই NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
শূন্য
fetch_send_callback(ধ্রুৱ fetch_বাৰ্তা *বাৰ্তা, সংরচনা fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	যদি (বাৰ্তা->type > fetch->last_বাৰ্তা)
		fetch->last_বাৰ্তা = বাৰ্তা->type;
	fetch->callback(বাৰ্তা, fetch->p);
}


/* exported interface documented in content/fetch.h */
শূন্য fetch_remove_from_queues(সংরচনা fetch *fetch)
{
	পূর্ণাঙ্ক all_active;
	পূর্ণাঙ্ক all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	যদি (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} অন্যথা {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(সংরচনা fetch, fetch_ring, all_active);
	RING_GETSIZE(সংরচনা fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(সংরচনা fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	ঘূৰাই NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
শূন্য fetch_set_cookie(সংরচনা fetch *fetch, ধ্রুৱ ৱর্ণ *data)
{
	প্রতিজ্ঞা(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	যদি (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, রিক্ত);
	} অন্যথা যদি (fetch->referer != রিক্ত) {
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
