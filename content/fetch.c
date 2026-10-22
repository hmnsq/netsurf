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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/fetchers.h"
#અન્તર્ભાવ "content/fetchers/resource.h"
#અન્તર્ભાવ "content/fetchers/about/about.h"
#અન્તર્ભાવ "content/fetchers/curl.h"
#અન્તર્ભાવ "content/fetchers/data.h"
#અન્તર્ભાવ "content/fetchers/file/file.h"
#અન્તર્ભાવ "javascript/fetcher.h"
#અન્તર્ભાવ "content/urldb.h"

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
પ્રકારવ્યાખ્યા સંરચના scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	સંરચના fetcher_operation_table ops; /**< The fetchers operations. */
	પૂર્ણાંક refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

સ્થિર scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
સંરચના fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	શૂન્ય *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	પૂર્ણાંક fetcherd;           /**< Fetcher descriptor for this fetch */
	શૂન્ય *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_સંદેશ_type last_સંદેશ;/**< The last message sent for this fetch */
	સંરચના fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	સંરચના fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

સ્થિર સંરચના fetch *fetch_ring = ખાલી;	/**< Ring of active fetches. */
સ્થિર સંરચના fetch *queue_ring = ખાલી;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

સ્થિર અંતઃરેખ શૂન્ય fetch_ref_fetcher(પૂર્ણાંક fetcherd)
{
	fetchers[fetcherd].refcount++;
}

સ્થિર અંતઃરેખ શૂન્ય fetch_unref_fetcher(પૂર્ણાંક fetcherd)
{
	fetchers[fetcherd].refcount--;
	જો (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
સ્થિર પૂર્ણાંક get_fetcher_for_scheme(lwc_string *scheme)
{
	પૂર્ણાંક fetcherd;
	bool match;

	માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		જો ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			પાછા fetcherd;
	       }
	}
	પાછા -1;
}

/**
 * Dispatch a single job
 */
સ્થિર bool fetch_dispatch_job(સંરચના fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	જો (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_ઉમેરો(queue_ring, fetch); /* Put it back on the end of the queue */
		પાછા false;
	} નહીંતર {
		RING_ઉમેરો(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		પાછા true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
સ્થિર bool fetch_choose_and_dispatch(શૂન્ય)
{
	bool same_host;
	સંરચના fetch *queueitem;
	queueitem = queue_ring;
	કરો {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		પૂર્ણાંક countbyhost;
		RING_COUNTBYLWCHOST(સંરચના fetch, fetch_ring, countbyhost,
				    queueitem->host);
		જો (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			પાછા fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		જ્યારે (same_host == true && queueitem->r_next != queue_ring) {
			જો (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} જ્યારે (queueitem != queue_ring);
	પાછા false;
}

સ્થિર શૂન્ય dump_rings(શૂન્ય)
{
	સંરચના fetch *q;
	સંરચના fetch *f;

	q = queue_ring;
	જો (q) {
		કરો {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} જ્યારે (q != queue_ring);
	}
	f = fetch_ring;
	જો (f) {
		કરો {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} જ્યારે (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
સ્થિર bool fetch_dispatch_jobs(શૂન્ય)
{
	પૂર્ણાંક all_active;
	પૂર્ણાંક all_queued;

	RING_GETSIZE(સંરચના fetch, queue_ring, all_queued);
	RING_GETSIZE(સંરચના fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	જ્યારે ((all_queued != 0) &&
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

	પાછા (all_active > 0);
}

સ્થિર શૂન્ય fetcher_poll(શૂન્ય *unused)
{
	પૂર્ણાંક fetcherd;

	જો (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			જો (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, ખાલી);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(શૂન્ય)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}
#endif

	ret = fetch_data_register();
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	ret = fetch_file_register();
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	ret = fetch_resource_register();
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	ret = fetch_about_register();
	જો (ret != NSERROR_OK) {
		પાછા ret;
	}

	ret = fetch_javascript_register();

	પાછા ret;
}

/* exported interface documented in content/fetchers.h */
શૂન્ય fetcher_quit(શૂન્ય)
{
	પૂર્ણાંક fetcherd; /* fetcher index */
	માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		જો (fetchers[fetcherd].refcount > 1) {
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
		જો (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, અચલ સંરચના fetcher_operation_table *ops)
{
	પૂર્ણાંક fetcherd;

	/* find unused fetcher descriptor */
	માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		જો (fetchers[fetcherd].refcount == 0) {
			તોડો;
		}
	}
	જો (fetcherd == MAX_FETCHERS) {
		પાછા NSERROR_INIT_FAILED;
	}

	જો (!ops->initialise(scheme)) {
		પાછા NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	પાછા NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      પૂર્ણાંક *maxfd_out)
{
	પૂર્ણાંક maxfd = -1;
	પૂર્ણાંક fetcherd; /* fetcher index */

	જો (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		પાછા NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		જો (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	માટે (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		જો ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != ખાલી)) {
			/* fetcher present */
			પૂર્ણાંક fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			જો (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	જો (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, ખાલી);
	}

	*maxfd_out = maxfd;

	પાછા NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    શૂન્ય *p,
	    bool only_2xx,
	    અચલ અક્ષર *post_urlenc,
	    અચલ સંરચના fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    અચલ અક્ષર *headers[],
	    સંરચના fetch **fetch_out)
{
	સંરચના fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, કદ (*fetch));
	જો (fetch == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	પ્રતિજ્ઞા(scheme != ખાલી);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	જો (fetch->fetcherd == -1) {
		મુક્ત(fetch);
		પાછા NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	જો (referer != ખાલી) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	જો (fetch->fetcher_handle == ખાલી) {
		lwc_string_unref(fetch->host);

		જો (fetch->url != ખાલી)
			nsurl_unref(fetch->url);

		જો (fetch->referer != ખાલી)
			nsurl_unref(fetch->referer);

		મુક્ત(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		પાછા NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_ઉમેરો(queue_ring, fetch);

	/* Ask the queue to run. */
	જો (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, ખાલી);
	}

	*fetch_out = fetch;
	પાછા NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
શૂન્ય fetch_abort(સંરચના fetch *f)
{
	પ્રતિજ્ઞા(f);
	f->last_સંદેશ = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
શૂન્ય fetch_free(સંરચના fetch *f)
{
	જો (f->last_સંદેશ < FETCH_MIN_FINISHED_સંદેશ) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_સંદેશ સંદેશ;

		સંદેશ.type = FETCH_ERROR;
		સંદેશ.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&સંદેશ, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.મુક્ત(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	જો (f->referer != ખાલી) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	મુક્ત(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(અચલ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	પૂર્ણાંક fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	જો (fetcherd == -1) {
		પાછા false;
	}

	પાછા fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
શૂન્ય fetch_change_callback(સંરચના fetch *fetch,
			   fetch_callback callback,
			   શૂન્ય *p)
{
	પ્રતિજ્ઞા(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(સંરચના fetch *fetch)
{
	પાછા fetch->http_code;
}


/* exported interface documented in content/fetch.h */
સંરચના fetch_multipart_data *
fetch_multipart_data_clone(અચલ સંરચના fetch_multipart_data *list)
{
	સંરચના fetch_multipart_data *clone, *last = ખાલી;
	સંરચના fetch_multipart_data *result = ખાલી;

	માટે (; list != ખાલી; list = list->next) {
		clone = malloc(કદ(સંરચના fetch_multipart_data));
		જો (clone == ખાલી) {
			જો (result != ખાલી)
				fetch_multipart_data_destroy(result);

			પાછા ખાલી;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		જો (clone->name == ખાલી) {
			મુક્ત(clone);
			જો (result != ખાલી)
				fetch_multipart_data_destroy(result);

			પાછા ખાલી;
		}

		clone->કિંમત = strdup(list->કિંમત);
		જો (clone->કિંમત == ખાલી) {
			મુક્ત(clone->name);
			મુક્ત(clone);
			જો (result != ખાલી)
				fetch_multipart_data_destroy(result);

			પાછા ખાલી;
		}

		જો (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			જો (clone->rawfile == ખાલી) {
				મુક્ત(clone->કિંમત);
				મુક્ત(clone->name);
				મુક્ત(clone);
				જો (result != ખાલી)
					fetch_multipart_data_destroy(result);

				પાછા ખાલી;
			}
		} નહીંતર {
			clone->rawfile = ખાલી;
		}

		clone->next = ખાલી;

		જો (result == ખાલી)
			result = clone;
		નહીંતર
			last->next = clone;

		last = clone;
	}

	પાછા result;
}


/* exported interface documented in content/fetch.h */
અચલ અક્ષર *
fetch_multipart_data_find(અચલ સંરચના fetch_multipart_data *list,
			  અચલ અક્ષર *name)
{
	જ્યારે (list != ખાલી) {
		જો (strcmp(list->name, name) == 0) {
			પાછા list->કિંમત;
		}
		list = list->next;
	}

	પાછા ખાલી;
}


/* exported interface documented in content/fetch.h */
શૂન્ય fetch_multipart_data_destroy(સંરચના fetch_multipart_data *list)
{
	સંરચના fetch_multipart_data *next;

	માટે (; list != ખાલી; list = next) {
		next = list->next;
		મુક્ત(list->name);
		મુક્ત(list->કિંમત);
		જો (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			મુક્ત(list->rawfile);
		}
		મુક્ત(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(સંરચના fetch_multipart_data **list,
			    અચલ અક્ષર *name,
			    અચલ અક્ષર *કિંમત)
{
	સંરચના fetch_multipart_data *newdata;

	પ્રતિજ્ઞા(list);

	newdata = calloc(1, કદ(*newdata));

	જો (newdata == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	જો (newdata->name == ખાલી) {
		મુક્ત(newdata);
		પાછા NSERROR_NOMEM;
	}

	newdata->કિંમત = strdup(કિંમત);
	જો (newdata->કિંમત == ખાલી) {
		મુક્ત(newdata->name);
		મુક્ત(newdata);
		પાછા NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
શૂન્ય
fetch_send_callback(અચલ fetch_સંદેશ *સંદેશ, સંરચના fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	જો (સંદેશ->type > fetch->last_સંદેશ)
		fetch->last_સંદેશ = સંદેશ->type;
	fetch->callback(સંદેશ, fetch->p);
}


/* exported interface documented in content/fetch.h */
શૂન્ય fetch_remove_from_queues(સંરચના fetch *fetch)
{
	પૂર્ણાંક all_active;
	પૂર્ણાંક all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	જો (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} નહીંતર {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(સંરચના fetch, fetch_ring, all_active);
	RING_GETSIZE(સંરચના fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(સંરચના fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	પાછા NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
શૂન્ય fetch_set_cookie(સંરચના fetch *fetch, અચલ અક્ષર *data)
{
	પ્રતિજ્ઞા(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	જો (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, ખાલી);
	} નહીંતર જો (fetch->referer != ખાલી) {
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
