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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/fetchers.h"
#अन्तर्भाव "content/fetchers/resource.h"
#अन्तर्भाव "content/fetchers/about/about.h"
#अन्तर्भाव "content/fetchers/curl.h"
#अन्तर्भाव "content/fetchers/data.h"
#अन्तर्भाव "content/fetchers/file/file.h"
#अन्तर्भाव "javascript/fetcher.h"
#अन्तर्भाव "content/urldb.h"

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
प्रकारव्याख्या संरचना scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	संरचना fetcher_operation_table ops; /**< The fetchers operations. */
	पूर्णांक refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

स्थिर scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
संरचना fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	शून्य *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	पूर्णांक fetcherd;           /**< Fetcher descriptor for this fetch */
	शून्य *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_संदेश_type last_संदेश;/**< The last message sent for this fetch */
	संरचना fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	संरचना fetch *r_next;	/**< Next active fetch in ::fetch_ring. */
};

स्थिर संरचना fetch *fetch_ring = रिक्त;	/**< Ring of active fetches. */
स्थिर संरचना fetch *queue_ring = रिक्त;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

स्थिर ओळीत शून्य fetch_ref_fetcher(पूर्णांक fetcherd)
{
	fetchers[fetcherd].refcount++;
}

स्थिर ओळीत शून्य fetch_unref_fetcher(पूर्णांक fetcherd)
{
	fetchers[fetcherd].refcount--;
	जर (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
स्थिर पूर्णांक get_fetcher_for_scheme(lwc_string *scheme)
{
	पूर्णांक fetcherd;
	bool match;

	साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		जर ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			परत fetcherd;
	       }
	}
	परत -1;
}

/**
 * Dispatch a single job
 */
स्थिर bool fetch_dispatch_job(संरचना fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	जर (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_घाला(queue_ring, fetch); /* Put it back on the end of the queue */
		परत false;
	} अन्यथा {
		RING_घाला(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		परत true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
स्थिर bool fetch_choose_and_dispatch(शून्य)
{
	bool same_host;
	संरचना fetch *queueitem;
	queueitem = queue_ring;
	करा {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		पूर्णांक countbyhost;
		RING_COUNTBYLWCHOST(संरचना fetch, fetch_ring, countbyhost,
				    queueitem->host);
		जर (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			परत fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		पर्यंत (same_host == true && queueitem->r_next != queue_ring) {
			जर (lwc_string_isequal(queueitem->host,
					       queueitem->r_next->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_next;
			}
		}
		queueitem = queueitem->r_next;
	} पर्यंत (queueitem != queue_ring);
	परत false;
}

स्थिर शून्य dump_rings(शून्य)
{
	संरचना fetch *q;
	संरचना fetch *f;

	q = queue_ring;
	जर (q) {
		करा {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_next;
		} पर्यंत (q != queue_ring);
	}
	f = fetch_ring;
	जर (f) {
		करा {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_next;
		} पर्यंत (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
स्थिर bool fetch_dispatch_jobs(शून्य)
{
	पूर्णांक all_active;
	पूर्णांक all_queued;

	RING_GETSIZE(संरचना fetch, queue_ring, all_queued);
	RING_GETSIZE(संरचना fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	पर्यंत ((all_queued != 0) &&
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

	परत (all_active > 0);
}

स्थिर शून्य fetcher_poll(शून्य *unused)
{
	पूर्णांक fetcherd;

	जर (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			जर (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, रिक्त);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(शून्य)
{
	nserror ret;

#ifdef WITH_CURL
	ret = fetch_curl_register();
	जर (ret != NSERROR_OK) {
		परत ret;
	}
#endif

	ret = fetch_data_register();
	जर (ret != NSERROR_OK) {
		परत ret;
	}

	ret = fetch_file_register();
	जर (ret != NSERROR_OK) {
		परत ret;
	}

	ret = fetch_resource_register();
	जर (ret != NSERROR_OK) {
		परत ret;
	}

	ret = fetch_about_register();
	जर (ret != NSERROR_OK) {
		परत ret;
	}

	ret = fetch_javascript_register();

	परत ret;
}

/* exported interface documented in content/fetchers.h */
शून्य fetcher_quit(शून्य)
{
	पूर्णांक fetcherd; /* fetcher index */
	साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		जर (fetchers[fetcherd].refcount > 1) {
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
		जर (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, स्थिरांक संरचना fetcher_operation_table *ops)
{
	पूर्णांक fetcherd;

	/* find unused fetcher descriptor */
	साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		जर (fetchers[fetcherd].refcount == 0) {
			थांबा;
		}
	}
	जर (fetcherd == MAX_FETCHERS) {
		परत NSERROR_INIT_FAILED;
	}

	जर (!ops->initialise(scheme)) {
		परत NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	परत NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_set,
		      fd_set *write_fd_set,
		      fd_set *except_fd_set,
		      पूर्णांक *maxfd_out)
{
	पूर्णांक maxfd = -1;
	पूर्णांक fetcherd; /* fetcher index */

	जर (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		परत NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		जर (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	साठी (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		जर ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != रिक्त)) {
			/* fetcher present */
			पूर्णांक fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			जर (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	जर (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, रिक्त);
	}

	*maxfd_out = maxfd;

	परत NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    शून्य *p,
	    bool only_2xx,
	    स्थिरांक अक्षर *post_urlenc,
	    स्थिरांक संरचना fetch_multipart_data *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    स्थिरांक अक्षर *headers[],
	    संरचना fetch **fetch_out)
{
	संरचना fetch *fetch;
	lwc_string *scheme;

	fetch = सुस्मृति(1, आकार (*fetch));
	जर (fetch == रिक्त) {
		परत NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	प्रतिज्ञा(scheme != रिक्त);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	जर (fetch->fetcherd == -1) {
		मुक्त(fetch);
		परत NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	जर (referer != रिक्त) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	जर (fetch->fetcher_handle == रिक्त) {
		lwc_string_unref(fetch->host);

		जर (fetch->url != रिक्त)
			nsurl_unref(fetch->url);

		जर (fetch->referer != रिक्त)
			nsurl_unref(fetch->referer);

		मुक्त(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		परत NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_घाला(queue_ring, fetch);

	/* Ask the queue to run. */
	जर (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, रिक्त);
	}

	*fetch_out = fetch;
	परत NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
शून्य fetch_abort(संरचना fetch *f)
{
	प्रतिज्ञा(f);
	f->last_संदेश = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.पात(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
शून्य fetch_free(संरचना fetch *f)
{
	जर (f->last_संदेश < FETCH_MIN_FINISHED_संदेश) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_संदेश संदेश;

		संदेश.type = FETCH_ERROR;
		संदेश.data.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&संदेश, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.मुक्त(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	जर (f->referer != रिक्त) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	मुक्त(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(स्थिरांक nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	पूर्णांक fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	जर (fetcherd == -1) {
		परत false;
	}

	परत fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
शून्य fetch_change_callback(संरचना fetch *fetch,
			   fetch_callback callback,
			   शून्य *p)
{
	प्रतिज्ञा(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(संरचना fetch *fetch)
{
	परत fetch->http_code;
}


/* exported interface documented in content/fetch.h */
संरचना fetch_multipart_data *
fetch_multipart_data_clone(स्थिरांक संरचना fetch_multipart_data *list)
{
	संरचना fetch_multipart_data *clone, *last = रिक्त;
	संरचना fetch_multipart_data *result = रिक्त;

	साठी (; list != रिक्त; list = list->next) {
		clone = दो_स्मृति(आकार(संरचना fetch_multipart_data));
		जर (clone == रिक्त) {
			जर (result != रिक्त)
				fetch_multipart_data_destroy(result);

			परत रिक्त;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		जर (clone->name == रिक्त) {
			मुक्त(clone);
			जर (result != रिक्त)
				fetch_multipart_data_destroy(result);

			परत रिक्त;
		}

		clone->मूल्य = strdup(list->मूल्य);
		जर (clone->मूल्य == रिक्त) {
			मुक्त(clone->name);
			मुक्त(clone);
			जर (result != रिक्त)
				fetch_multipart_data_destroy(result);

			परत रिक्त;
		}

		जर (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			जर (clone->rawfile == रिक्त) {
				मुक्त(clone->मूल्य);
				मुक्त(clone->name);
				मुक्त(clone);
				जर (result != रिक्त)
					fetch_multipart_data_destroy(result);

				परत रिक्त;
			}
		} अन्यथा {
			clone->rawfile = रिक्त;
		}

		clone->next = रिक्त;

		जर (result == रिक्त)
			result = clone;
		अन्यथा
			last->next = clone;

		last = clone;
	}

	परत result;
}


/* exported interface documented in content/fetch.h */
स्थिरांक अक्षर *
fetch_multipart_data_find(स्थिरांक संरचना fetch_multipart_data *list,
			  स्थिरांक अक्षर *name)
{
	पर्यंत (list != रिक्त) {
		जर (म_भेद(list->name, name) == 0) {
			परत list->मूल्य;
		}
		list = list->next;
	}

	परत रिक्त;
}


/* exported interface documented in content/fetch.h */
शून्य fetch_multipart_data_destroy(संरचना fetch_multipart_data *list)
{
	संरचना fetch_multipart_data *next;

	साठी (; list != रिक्त; list = next) {
		next = list->next;
		मुक्त(list->name);
		मुक्त(list->मूल्य);
		जर (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			मुक्त(list->rawfile);
		}
		मुक्त(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_data_new_kv(संरचना fetch_multipart_data **list,
			    स्थिरांक अक्षर *name,
			    स्थिरांक अक्षर *मूल्य)
{
	संरचना fetch_multipart_data *newdata;

	प्रतिज्ञा(list);

	newdata = सुस्मृति(1, आकार(*newdata));

	जर (newdata == रिक्त) {
		परत NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	जर (newdata->name == रिक्त) {
		मुक्त(newdata);
		परत NSERROR_NOMEM;
	}

	newdata->मूल्य = strdup(मूल्य);
	जर (newdata->मूल्य == रिक्त) {
		मुक्त(newdata->name);
		मुक्त(newdata);
		परत NSERROR_NOMEM;
	}

	newdata->next = *list;
	*list = newdata;

	परत NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
शून्य
fetch_send_callback(स्थिरांक fetch_संदेश *संदेश, संरचना fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	जर (संदेश->type > fetch->last_संदेश)
		fetch->last_संदेश = संदेश->type;
	fetch->callback(संदेश, fetch->p);
}


/* exported interface documented in content/fetch.h */
शून्य fetch_remove_from_queues(संरचना fetch *fetch)
{
	पूर्णांक all_active;
	पूर्णांक all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	जर (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} अन्यथा {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(संरचना fetch, fetch_ring, all_active);
	RING_GETSIZE(संरचना fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(संरचना fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	परत NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
शून्य fetch_set_cookie(संरचना fetch *fetch, स्थिरांक अक्षर *data)
{
	प्रतिज्ञा(fetch && data);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	जर (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(data, fetch->url, रिक्त);
	} अन्यथा जर (fetch->referer != रिक्त) {
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
