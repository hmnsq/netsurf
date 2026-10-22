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
typedef struct scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	struct fetcher_operation_table ops; /**< The fetchers operations. */
	цяло refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

static scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
struct fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	void *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	цяло fetcherd;           /**< Fetcher descriptor for this fetch */
	void *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_msg_type last_msg;/**< The last message sent for this fetch */
	struct fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	struct fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

static struct fetch *fetch_ring = NULL;	/**< Ring of active fetches. */
static struct fetch *queue_ring = NULL;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

static inline void fetch_ref_fetcher(цяло fetcherd)
{
	fetchers[fetcherd].refcount++;
}

static inline void fetch_unref_fetcher(цяло fetcherd)
{
	fetchers[fetcherd].refcount--;
	ако (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
static цяло get_fetcher_for_scheme(lwc_string *scheme)
{
	цяло fetcherd;
	bool match;

	за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ако ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			върни fetcherd;
	       }
	}
	върни -1;
}

/**
 * Dispatch a single job
 */
static bool fetch_dispatch_job(struct fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	ако (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_INSERT(queue_ring, fetch); /* Put it back on the end of the queue */
		върни false;
	} иначе {
		RING_INSERT(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		върни true;
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
	struct fetch *queueitem;
	queueitem = queue_ring;
	do {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		цяло countbyhost;
		RING_COUNTBYLWCHOST(struct fetch, fetch_ring, countbyhost,
				    queueitem->host);
		ако (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			върни fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		докато (same_host == true && queueitem->r_next != queue_ring) {
			ако (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} докато (queueitem != queue_ring);
	върни false;
}

static void dump_rings(void)
{
	struct fetch *q;
	struct fetch *f;

	q = queue_ring;
	ако (q) {
		do {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} докато (q != queue_ring);
	}
	f = fetch_ring;
	ако (f) {
		do {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} докато (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
static bool fetch_dispatch_jobs(void)
{
	цяло all_active;
	цяло all_queued;

	RING_GETSIZE(struct fetch, queue_ring, all_queued);
	RING_GETSIZE(struct fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	докато ((all_queued != 0) &&
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

	върни (all_active > 0);
}

static void fetcher_poll(void *unused)
{
	цяло fetcherd;

	ако (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			ако (fetchers[fetcherd].refcount > 0) {
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
	ако (ret != NSERROR_OK) {
		върни ret;
	}
#endif

	ret = fetch_data_register();
	ако (ret != NSERROR_OK) {
		върни ret;
	}

	ret = fetch_file_register();
	ако (ret != NSERROR_OK) {
		върни ret;
	}

	ret = fetch_resource_register();
	ако (ret != NSERROR_OK) {
		върни ret;
	}

	ret = fetch_about_register();
	ако (ret != NSERROR_OK) {
		върни ret;
	}

	ret = fetch_javascript_register();

	върни ret;
}

/* exported interface documented in content/fetchers.h */
void fetcher_quit(void)
{
	цяло fetcherd; /* fetcher index */
	за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ако (fetchers[fetcherd].refcount > 1) {
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
		ако (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, const struct fetcher_operation_table *ops)
{
	цяло fetcherd;

	/* find unused fetcher descriptor */
	за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ако (fetchers[fetcherd].refcount == 0) {
			break;
		}
	}
	ако (fetcherd == MAX_FETCHERS) {
		върни NSERROR_INIT_FAILED;
	}

	ако (!ops->initialise(scheme)) {
		върни NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	върни NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      цяло *maxfd_out)
{
	цяло maxfd = -1;
	цяло fetcherd; /* fetcher index */

	ако (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		върни NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ако (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	за (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		ако ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != NULL)) {
			/* fetcher present */
			цяло fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			ако (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	ако (maxfd >= 0) {
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

	върни NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    void *p,
	    bool only_2xx,
	    const char *post_urlenc,
	    const struct fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    const char *headers[],
	    struct fetch **fetch_out)
{
	struct fetch *fetch;
	lwc_string *scheme;

	fetch = calloc(1, sizeof (*fetch));
	ако (fetch == NULL) {
		върни NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	assert(scheme != NULL);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	ако (fetch->fetcherd == -1) {
		free(fetch);
		върни NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	ако (referer != NULL) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	ако (fetch->fetcher_handle == NULL) {
		lwc_string_unref(fetch->host);

		ако (fetch->url != NULL)
			nsurl_unref(fetch->url);

		ако (fetch->referer != NULL)
			nsurl_unref(fetch->referer);

		free(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		върни NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_INSERT(queue_ring, fetch);

	/* Ask the queue to run. */
	ако (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, NULL);
	}

	*fetch_out = fetch;
	върни NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
void fetch_abort(struct fetch *f)
{
	assert(f);
	f->last_msg = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
void fetch_free(struct fetch *f)
{
	ако (f->last_msg < FETCH_MIN_FINISHED_MSG) {
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
	ако (f->referer != NULL) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	free(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(const nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	цяло fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	ако (fetcherd == -1) {
		върни false;
	}

	върни fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
void fetch_change_callback(struct fetch *fetch,
			   fetch_callback callback,
			   void *p)
{
	assert(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(struct fetch *fetch)
{
	върни fetch->http_code;
}


/* exported interface documented in content/fetch.h */
struct fetch_multipart_data *
fetch_multipart_data_clone(const struct fetch_multipart_data *list)
{
	struct fetch_multipart_data *clone, *last = NULL;
	struct fetch_multipart_data *result = NULL;

	за (; list != NULL; list = list->next) {
		clone = malloc(sizeof(struct fetch_multipart_data));
		ако (clone == NULL) {
			ако (result != NULL)
				fetch_multipart_data_destroy(result);

			върни NULL;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		ако (clone->name == NULL) {
			free(clone);
			ако (result != NULL)
				fetch_multipart_data_destroy(result);

			върни NULL;
		}

		clone->value = strdup(list->value);
		ако (clone->value == NULL) {
			free(clone->name);
			free(clone);
			ако (result != NULL)
				fetch_multipart_data_destroy(result);

			върни NULL;
		}

		ако (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			ако (clone->rawfile == NULL) {
				free(clone->value);
				free(clone->name);
				free(clone);
				ако (result != NULL)
					fetch_multipart_data_destroy(result);

				върни NULL;
			}
		} иначе {
			clone->rawfile = NULL;
		}

		clone->next = NULL;

		ако (result == NULL)
			result = clone;
		иначе
			last->next = clone;

		last = clone;
	}

	върни result;
}


/* exported interface documented in content/fetch.h */
const char *
fetch_multipart_data_find(const struct fetch_multipart_data *list,
			  const char *name)
{
	докато (list != NULL) {
		ако (strcmp(list->name, name) == 0) {
			върни list->value;
		}
		list = list->next;
	}

	върни NULL;
}


/* exported interface documented in content/fetch.h */
void fetch_multipart_data_destroy(struct fetch_multipart_data *list)
{
	struct fetch_multipart_data *next;

	за (; list != NULL; list = next) {
		next = list->next;
		free(list->name);
		free(list->value);
		ако (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			free(list->rawfile);
		}
		free(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(struct fetch_multipart_data **list,
			    const char *name,
			    const char *value)
{
	struct fetch_multipart_data *newdata;

	assert(list);

	newdata = calloc(1, sizeof(*newdata));

	ако (newdata == NULL) {
		върни NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	ако (newdata->name == NULL) {
		free(newdata);
		върни NSERROR_NOMEM;
	}

	newdata->value = strdup(value);
	ако (newdata->value == NULL) {
		free(newdata->name);
		free(newdata);
		върни NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	върни NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
void
fetch_send_callback(const fetch_msg *msg, struct fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	ако (msg->type > fetch->last_msg)
		fetch->last_msg = msg->type;
	fetch->callback(msg, fetch->p);
}


/* exported interface documented in content/fetch.h */
void fetch_remove_from_queues(struct fetch *fetch)
{
	цяло all_active;
	цяло all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	ако (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} иначе {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(struct fetch, fetch_ring, all_active);
	RING_GETSIZE(struct fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(struct fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	върни NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
void fetch_set_cookie(struct fetch *fetch, const char *data)
{
	assert(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	ако (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, NULL);
	} иначе ако (fetch->referer != NULL) {
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
