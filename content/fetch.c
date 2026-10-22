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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/fetchers.h"
#അന്തര്ഭാവ "content/fetchers/resource.h"
#അന്തര്ഭാവ "content/fetchers/about/about.h"
#അന്തര്ഭാവ "content/fetchers/curl.h"
#അന്തര്ഭാവ "content/fetchers/data.h"
#അന്തര്ഭാവ "content/fetchers/file/file.h"
#അന്തര്ഭാവ "javascript/fetcher.h"
#അന്തര്ഭാവ "content/urldb.h"

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
തരനിർവചനം ഘടന scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ഘടന fetcher_operation_table ops; /**< The fetchers operations. */
	പൂർണ്ണസംഖ്യ refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

സ്ഥിതം scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ഘടന fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	ശൂന്യം *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	പൂർണ്ണസംഖ്യ fetcherd;           /**< Fetcher descriptor for this fetch */
	ശൂന്യം *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_സന്ദേശം_type last_സന്ദേശം;/**< The last message sent for this fetch */
	ഘടന fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	ഘടന fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

സ്ഥിതം ഘടന fetch *fetch_ring = ശൂന്യ;	/**< Ring of active fetches. */
സ്ഥിതം ഘടന fetch *queue_ring = ശൂന്യ;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

സ്ഥിതം വരിയിൽ ശൂന്യം fetch_ref_fetcher(പൂർണ്ണസംഖ്യ fetcherd)
{
	fetchers[fetcherd].refcount++;
}

സ്ഥിതം വരിയിൽ ശൂന്യം fetch_unref_fetcher(പൂർണ്ണസംഖ്യ fetcherd)
{
	fetchers[fetcherd].refcount--;
	എങ്കിൽ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ get_fetcher_for_scheme(lwc_string *scheme)
{
	പൂർണ്ണസംഖ്യ fetcherd;
	bool match;

	വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		എങ്കിൽ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			തിരികെ fetcherd;
	       }
	}
	തിരികെ -1;
}

/**
 * Dispatch a single job
 */
സ്ഥിതം bool fetch_dispatch_job(ഘടന fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	എങ്കിൽ (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_തിരുകുക(queue_ring, fetch); /* Put it back on the end of the queue */
		തിരികെ false;
	} അല്ലെങ്കിൽ {
		RING_തിരുകുക(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		തിരികെ true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
സ്ഥിതം bool fetch_choose_and_dispatch(ശൂന്യം)
{
	bool same_host;
	ഘടന fetch *queueitem;
	queueitem = queue_ring;
	ചെയ്യുക {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		പൂർണ്ണസംഖ്യ countbyhost;
		RING_COUNTBYLWCHOST(ഘടന fetch, fetch_ring, countbyhost,
				    queueitem->host);
		എങ്കിൽ (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			തിരികെ fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		ആകുമ്പോൾ (same_host == true && queueitem->r_next != queue_ring) {
			എങ്കിൽ (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} ആകുമ്പോൾ (queueitem != queue_ring);
	തിരികെ false;
}

സ്ഥിതം ശൂന്യം dump_rings(ശൂന്യം)
{
	ഘടന fetch *q;
	ഘടന fetch *f;

	q = queue_ring;
	എങ്കിൽ (q) {
		ചെയ്യുക {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} ആകുമ്പോൾ (q != queue_ring);
	}
	f = fetch_ring;
	എങ്കിൽ (f) {
		ചെയ്യുക {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} ആകുമ്പോൾ (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
സ്ഥിതം bool fetch_dispatch_jobs(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ all_active;
	പൂർണ്ണസംഖ്യ all_queued;

	RING_GETSIZE(ഘടന fetch, queue_ring, all_queued);
	RING_GETSIZE(ഘടന fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	ആകുമ്പോൾ ((all_queued != 0) &&
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

	തിരികെ (all_active > 0);
}

സ്ഥിതം ശൂന്യം fetcher_poll(ശൂന്യം *unused)
{
	പൂർണ്ണസംഖ്യ fetcherd;

	എങ്കിൽ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			എങ്കിൽ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, ശൂന്യ);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(ശൂന്യം)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}
#endif

	ret = fetch_data_register();
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}

	ret = fetch_file_register();
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}

	ret = fetch_resource_register();
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}

	ret = fetch_about_register();
	എങ്കിൽ (ret != NSERROR_OK) {
		തിരികെ ret;
	}

	ret = fetch_javascript_register();

	തിരികെ ret;
}

/* exported interface documented in content/fetchers.h */
ശൂന്യം fetcher_quit(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ fetcherd; /* fetcher index */
	വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		എങ്കിൽ (fetchers[fetcherd].refcount > 1) {
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
		എങ്കിൽ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, സ്ഥിരം ഘടന fetcher_operation_table *ops)
{
	പൂർണ്ണസംഖ്യ fetcherd;

	/* find unused fetcher descriptor */
	വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		എങ്കിൽ (fetchers[fetcherd].refcount == 0) {
			നിർത്തുക;
		}
	}
	എങ്കിൽ (fetcherd == MAX_FETCHERS) {
		തിരികെ NSERROR_INIT_FAILED;
	}

	എങ്കിൽ (!ops->initialise(scheme)) {
		തിരികെ NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	തിരികെ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      പൂർണ്ണസംഖ്യ *maxfd_out)
{
	പൂർണ്ണസംഖ്യ maxfd = -1;
	പൂർണ്ണസംഖ്യ fetcherd; /* fetcher index */

	എങ്കിൽ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		തിരികെ NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		എങ്കിൽ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	വേണ്ടി (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		എങ്കിൽ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ശൂന്യ)) {
			/* fetcher present */
			പൂർണ്ണസംഖ്യ fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			എങ്കിൽ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	എങ്കിൽ (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, ശൂന്യ);
	}

	*maxfd_out = maxfd;

	തിരികെ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    ശൂന്യം *p,
	    bool only_2xx,
	    സ്ഥിരം അക്ഷരം *post_urlenc,
	    സ്ഥിരം ഘടന fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    സ്ഥിരം അക്ഷരം *headers[],
	    ഘടന fetch **fetch_out)
{
	ഘടന fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, വലുപ്പം (*fetch));
	എങ്കിൽ (fetch == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	പ്രതിജ്ഞാ(scheme != ശൂന്യ);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	എങ്കിൽ (fetch->fetcherd == -1) {
		സ്വതന്ത്രം(fetch);
		തിരികെ NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	എങ്കിൽ (referer != ശൂന്യ) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	എങ്കിൽ (fetch->fetcher_handle == ശൂന്യ) {
		lwc_string_unref(fetch->host);

		എങ്കിൽ (fetch->url != ശൂന്യ)
			nsurl_unref(fetch->url);

		എങ്കിൽ (fetch->referer != ശൂന്യ)
			nsurl_unref(fetch->referer);

		സ്വതന്ത്രം(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		തിരികെ NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_തിരുകുക(queue_ring, fetch);

	/* Ask the queue to run. */
	എങ്കിൽ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, ശൂന്യ);
	}

	*fetch_out = fetch;
	തിരികെ NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_abort(ഘടന fetch *f)
{
	പ്രതിജ്ഞാ(f);
	f->last_സന്ദേശം = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_free(ഘടന fetch *f)
{
	എങ്കിൽ (f->last_സന്ദേശം < FETCH_MIN_FINISHED_സന്ദേശം) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_സന്ദേശം സന്ദേശം;

		സന്ദേശം.type = FETCH_ERROR;
		സന്ദേശം.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&സന്ദേശം, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.സ്വതന്ത്രം(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	എങ്കിൽ (f->referer != ശൂന്യ) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	സ്വതന്ത്രം(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(സ്ഥിരം nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	പൂർണ്ണസംഖ്യ fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	എങ്കിൽ (fetcherd == -1) {
		തിരികെ false;
	}

	തിരികെ fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_change_callback(ഘടന fetch *fetch,
			   fetch_callback callback,
			   ശൂന്യം *p)
{
	പ്രതിജ്ഞാ(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(ഘടന fetch *fetch)
{
	തിരികെ fetch->http_code;
}


/* exported interface documented in content/fetch.h */
ഘടന fetch_multipart_data *
fetch_multipart_data_clone(സ്ഥിരം ഘടന fetch_multipart_data *list)
{
	ഘടന fetch_multipart_data *clone, *last = ശൂന്യ;
	ഘടന fetch_multipart_data *result = ശൂന്യ;

	വേണ്ടി (; list != ശൂന്യ; list = list->next) {
		clone = malloc(വലുപ്പം(ഘടന fetch_multipart_data));
		എങ്കിൽ (clone == ശൂന്യ) {
			എങ്കിൽ (result != ശൂന്യ)
				fetch_multipart_data_destroy(result);

			തിരികെ ശൂന്യ;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		എങ്കിൽ (clone->name == ശൂന്യ) {
			സ്വതന്ത്രം(clone);
			എങ്കിൽ (result != ശൂന്യ)
				fetch_multipart_data_destroy(result);

			തിരികെ ശൂന്യ;
		}

		clone->മൂല്യം = strdup(list->മൂല്യം);
		എങ്കിൽ (clone->മൂല്യം == ശൂന്യ) {
			സ്വതന്ത്രം(clone->name);
			സ്വതന്ത്രം(clone);
			എങ്കിൽ (result != ശൂന്യ)
				fetch_multipart_data_destroy(result);

			തിരികെ ശൂന്യ;
		}

		എങ്കിൽ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			എങ്കിൽ (clone->rawfile == ശൂന്യ) {
				സ്വതന്ത്രം(clone->മൂല്യം);
				സ്വതന്ത്രം(clone->name);
				സ്വതന്ത്രം(clone);
				എങ്കിൽ (result != ശൂന്യ)
					fetch_multipart_data_destroy(result);

				തിരികെ ശൂന്യ;
			}
		} അല്ലെങ്കിൽ {
			clone->rawfile = ശൂന്യ;
		}

		clone->next = ശൂന്യ;

		എങ്കിൽ (result == ശൂന്യ)
			result = clone;
		അല്ലെങ്കിൽ
			last->next = clone;

		last = clone;
	}

	തിരികെ result;
}


/* exported interface documented in content/fetch.h */
സ്ഥിരം അക്ഷരം *
fetch_multipart_data_find(സ്ഥിരം ഘടന fetch_multipart_data *list,
			  സ്ഥിരം അക്ഷരം *name)
{
	ആകുമ്പോൾ (list != ശൂന്യ) {
		എങ്കിൽ (strcmp(list->name, name) == 0) {
			തിരികെ list->മൂല്യം;
		}
		list = list->next;
	}

	തിരികെ ശൂന്യ;
}


/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_multipart_data_destroy(ഘടന fetch_multipart_data *list)
{
	ഘടന fetch_multipart_data *next;

	വേണ്ടി (; list != ശൂന്യ; list = next) {
		next = list->next;
		സ്വതന്ത്രം(list->name);
		സ്വതന്ത്രം(list->മൂല്യം);
		എങ്കിൽ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			സ്വതന്ത്രം(list->rawfile);
		}
		സ്വതന്ത്രം(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(ഘടന fetch_multipart_data **list,
			    സ്ഥിരം അക്ഷരം *name,
			    സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	ഘടന fetch_multipart_data *newdata;

	പ്രതിജ്ഞാ(list);

	newdata = calloc(1, വലുപ്പം(*newdata));

	എങ്കിൽ (newdata == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	എങ്കിൽ (newdata->name == ശൂന്യ) {
		സ്വതന്ത്രം(newdata);
		തിരികെ NSERROR_NOMEM;
	}

	newdata->മൂല്യം = strdup(മൂല്യം);
	എങ്കിൽ (newdata->മൂല്യം == ശൂന്യ) {
		സ്വതന്ത്രം(newdata->name);
		സ്വതന്ത്രം(newdata);
		തിരികെ NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ശൂന്യം
fetch_send_callback(സ്ഥിരം fetch_സന്ദേശം *സന്ദേശം, ഘടന fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	എങ്കിൽ (സന്ദേശം->type > fetch->last_സന്ദേശം)
		fetch->last_സന്ദേശം = സന്ദേശം->type;
	fetch->callback(സന്ദേശം, fetch->p);
}


/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_remove_from_queues(ഘടന fetch *fetch)
{
	പൂർണ്ണസംഖ്യ all_active;
	പൂർണ്ണസംഖ്യ all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	എങ്കിൽ (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} അല്ലെങ്കിൽ {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(ഘടന fetch, fetch_ring, all_active);
	RING_GETSIZE(ഘടന fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(ഘടന fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	തിരികെ NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
ശൂന്യം fetch_set_cookie(ഘടന fetch *fetch, സ്ഥിരം അക്ഷരം *data)
{
	പ്രതിജ്ഞാ(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	എങ്കിൽ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, ശൂന്യ);
	} അല്ലെങ്കിൽ എങ്കിൽ (fetch->referer != ശൂന്യ) {
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
