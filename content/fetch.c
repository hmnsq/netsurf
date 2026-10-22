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

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <libwapcaplet/libwapcaplet.h>

#include "utils/config.h"
#include "utils/corestrings.h"
#include "utils/nsoption.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/nsurl.h"
#include "utils/ring.h"
#include "netsurf/misc.h"
#include "desktop/gui_internal.h"

#include "content/fetch.h"
#include "content/fetchers.h"
#include "content/fetchers/resource.h"
#include "content/fetchers/about/about.h"
#include "content/fetchers/curl.h"
#include "content/fetchers/data.h"
#include "content/fetchers/file/file.h"
#include "javascript/fetcher.h"
#include "content/urldb.h"

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
typedef sa_nracanaa scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	sa_nracanaa fetcher_operation_table ops; /**< The fetchers operations. */
	ཧྲིལ refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

static scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
sa_nracanaa fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	void *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	ཧྲིལ fetcherd;           /**< Fetcher descriptor for this fetch */
	void *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_msg_type last_msg;/**< The last message sent for this fetch */
	sa_nracanaa fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	sa_nracanaa fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

static sa_nracanaa fetch *fetch_ring = NULL;	/**< Ring of active fetches. */
static sa_nracanaa fetch *queue_ring = NULL;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

static anta_hstha void fetch_ref_fetcher(ཧྲིལ fetcherd)
{
	fetchers[fetcherd].refcount++;
}

static anta_hstha void fetch_unref_fetcher(ཧྲིལ fetcherd)
{
	fetchers[fetcherd].refcount--;
	གལ་ཏེ (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
static ཧྲིལ get_fetcher_for_scheme(lwc_string *scheme)
{
	ཧྲིལ fetcherd;
	bool match;

	ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		གལ་ཏེ ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			ཕྱིར fetcherd;
	       }
	}
	ཕྱིར -1;
}

/**
 * Dispatch a single job
 */
static bool fetch_dispatch_job(sa_nracanaa fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	གལ་ཏེ (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_INSERT(queue_ring, fetch); /* Put it back on the end of the queue */
		ཕྱིར false;
	} དེ་མིན {
		RING_INSERT(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		ཕྱིར true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
static bool fetch_choose_and_dispatch(void)
{
	bool same_host;
	sa_nracanaa fetch *queueitem;
	queueitem = queue_ring;
	do {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		ཧྲིལ countbyhost;
		RING_COUNTBYLWCHOST(sa_nracanaa fetch, fetch_ring, countbyhost,
				    queueitem->host);
		གལ་ཏེ (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			ཕྱིར fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		སྐབས (same_host == true && queueitem->r_next != queue_ring) {
			གལ་ཏེ (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} སྐབས (queueitem != queue_ring);
	ཕྱིར false;
}

static void dump_rings(void)
{
	sa_nracanaa fetch *q;
	sa_nracanaa fetch *f;

	q = queue_ring;
	གལ་ཏེ (q) {
		do {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} སྐབས (q != queue_ring);
	}
	f = fetch_ring;
	གལ་ཏེ (f) {
		do {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} སྐབས (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
static bool fetch_dispatch_jobs(void)
{
	ཧྲིལ all_active;
	ཧྲིལ all_queued;

	RING_GETSIZE(sa_nracanaa fetch, queue_ring, all_queued);
	RING_GETSIZE(sa_nracanaa fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	སྐབས ((all_queued != 0) &&
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

	ཕྱིར (all_active > 0);
}

static void fetcher_poll(void *unused)
{
	ཧྲིལ fetcherd;

	གལ་ཏེ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			གལ་ཏེ (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, NULL);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(void)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	གལ་ཏེ (ret != NSERROR_OK) {
		ཕྱིར ret;
	}
#endif

	ret = fetch_data_register();
	གལ་ཏེ (ret != NSERROR_OK) {
		ཕྱིར ret;
	}

	ret = fetch_file_register();
	གལ་ཏེ (ret != NSERROR_OK) {
		ཕྱིར ret;
	}

	ret = fetch_resource_register();
	གལ་ཏེ (ret != NSERROR_OK) {
		ཕྱིར ret;
	}

	ret = fetch_about_register();
	གལ་ཏེ (ret != NSERROR_OK) {
		ཕྱིར ret;
	}

	ret = fetch_javascript_register();

	ཕྱིར ret;
}

/* exported interface documented in content/fetchers.h */
void fetcher_quit(void)
{
	ཧྲིལ fetcherd; /* fetcher index */
	ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		གལ་ཏེ (fetchers[fetcherd].refcount > 1) {
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
		གལ་ཏེ (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, const sa_nracanaa fetcher_operation_table *ops)
{
	ཧྲིལ fetcherd;

	/* find unused fetcher descriptor */
	ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		གལ་ཏེ (fetchers[fetcherd].refcount == 0) {
			bha_nga;
		}
	}
	གལ་ཏེ (fetcherd == MAX_FETCHERS) {
		ཕྱིར NSERROR_INIT_FAILED;
	}

	གལ་ཏེ (!ops->initialise(scheme)) {
		ཕྱིར NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	ཕྱིར NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      ཧྲིལ *maxfd_out)
{
	ཧྲིལ maxfd = -1;
	ཧྲིལ fetcherd; /* fetcher index */

	གལ་ཏེ (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		ཕྱིར NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		གལ་ཏེ (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	ཆེད་དུ (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		གལ་ཏེ ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != NULL)) {
			/* fetcher present */
			ཧྲིལ fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			གལ་ཏེ (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	གལ་ཏེ (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, NULL);
	}

	*maxfd_out = maxfd;

	ཕྱིར NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    void *p,
	    bool only_2xx,
	    const var_na *post_urlenc,
	    const sa_nracanaa fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    const var_na *headers[],
	    sa_nracanaa fetch **fetch_out)
{
	sa_nracanaa fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, parimaa_na (*fetch));
	གལ་ཏེ (fetch == NULL) {
		ཕྱིར NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	assert(scheme != NULL);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	གལ་ཏེ (fetch->fetcherd == -1) {
		free(fetch);
		ཕྱིར NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	གལ་ཏེ (referer != NULL) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	གལ་ཏེ (fetch->fetcher_handle == NULL) {
		lwc_string_unref(fetch->host);

		གལ་ཏེ (fetch->url != NULL)
			nsurl_unref(fetch->url);

		གལ་ཏེ (fetch->referer != NULL)
			nsurl_unref(fetch->referer);

		free(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		ཕྱིར NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_INSERT(queue_ring, fetch);

	/* Ask the queue to run. */
	གལ་ཏེ (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, NULL);
	}

	*fetch_out = fetch;
	ཕྱིར NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
void fetch_abort(sa_nracanaa fetch *f)
{
	assert(f);
	f->last_msg = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
void fetch_free(sa_nracanaa fetch *f)
{
	གལ་ཏེ (f->last_msg < FETCH_MIN_FINISHED_MSG) {
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

	fetchers[f->fetcherd].ops.free(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	གལ་ཏེ (f->referer != NULL) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	free(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(const nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	ཧྲིལ fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	གལ་ཏེ (fetcherd == -1) {
		ཕྱིར false;
	}

	ཕྱིར fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
void fetch_change_callback(sa_nracanaa fetch *fetch,
			   fetch_callback callback,
			   void *p)
{
	assert(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(sa_nracanaa fetch *fetch)
{
	ཕྱིར fetch->http_code;
}


/* exported interface documented in content/fetch.h */
sa_nracanaa fetch_multipart_data *
fetch_multipart_data_clone(const sa_nracanaa fetch_multipart_data *list)
{
	sa_nracanaa fetch_multipart_data *clone, *last = NULL;
	sa_nracanaa fetch_multipart_data *result = NULL;

	ཆེད་དུ (; list != NULL; list = list->next) {
		clone = malloc(parimaa_na(sa_nracanaa fetch_multipart_data));
		གལ་ཏེ (clone == NULL) {
			གལ་ཏེ (result != NULL)
				fetch_multipart_data_destroy(result);

			ཕྱིར NULL;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		གལ་ཏེ (clone->name == NULL) {
			free(clone);
			གལ་ཏེ (result != NULL)
				fetch_multipart_data_destroy(result);

			ཕྱིར NULL;
		}

		clone->value = strdup(list->value);
		གལ་ཏེ (clone->value == NULL) {
			free(clone->name);
			free(clone);
			གལ་ཏེ (result != NULL)
				fetch_multipart_data_destroy(result);

			ཕྱིར NULL;
		}

		གལ་ཏེ (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			གལ་ཏེ (clone->rawfile == NULL) {
				free(clone->value);
				free(clone->name);
				free(clone);
				གལ་ཏེ (result != NULL)
					fetch_multipart_data_destroy(result);

				ཕྱིར NULL;
			}
		} དེ་མིན {
			clone->rawfile = NULL;
		}

		clone->next = NULL;

		གལ་ཏེ (result == NULL)
			result = clone;
		དེ་མིན
			last->next = clone;

		last = clone;
	}

	ཕྱིར result;
}


/* exported interface documented in content/fetch.h */
const var_na *
fetch_multipart_data_find(const sa_nracanaa fetch_multipart_data *list,
			  const var_na *name)
{
	སྐབས (list != NULL) {
		གལ་ཏེ (strcmp(list->name, name) == 0) {
			ཕྱིར list->value;
		}
		list = list->next;
	}

	ཕྱིར NULL;
}


/* exported interface documented in content/fetch.h */
void fetch_multipart_data_destroy(sa_nracanaa fetch_multipart_data *list)
{
	sa_nracanaa fetch_multipart_data *next;

	ཆེད་དུ (; list != NULL; list = next) {
		next = list->next;
		free(list->name);
		free(list->value);
		གལ་ཏེ (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			free(list->rawfile);
		}
		free(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(sa_nracanaa fetch_multipart_data **list,
			    const var_na *name,
			    const var_na *value)
{
	sa_nracanaa fetch_multipart_data *newdata;

	assert(list);

	newdata = calloc(1, parimaa_na(*newdata));

	གལ་ཏེ (newdata == NULL) {
		ཕྱིར NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	གལ་ཏེ (newdata->name == NULL) {
		free(newdata);
		ཕྱིར NSERROR_NOMEM;
	}

	newdata->value = strdup(value);
	གལ་ཏེ (newdata->value == NULL) {
		free(newdata->name);
		free(newdata);
		ཕྱིར NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	ཕྱིར NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
void
fetch_send_callback(const fetch_msg *msg, sa_nracanaa fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	གལ་ཏེ (msg->type > fetch->last_msg)
		fetch->last_msg = msg->type;
	fetch->callback(msg, fetch->p);
}


/* exported interface documented in content/fetch.h */
void fetch_remove_from_queues(sa_nracanaa fetch *fetch)
{
	ཧྲིལ all_active;
	ཧྲིལ all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	གལ་ཏེ (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} དེ་མིན {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(sa_nracanaa fetch, fetch_ring, all_active);
	RING_GETSIZE(sa_nracanaa fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(sa_nracanaa fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	ཕྱིར NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
void fetch_set_cookie(sa_nracanaa fetch *fetch, const var_na *data)
{
	assert(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	གལ་ཏེ (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, NULL);
	} དེ་མིན གལ་ཏེ (fetch->referer != NULL) {
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
