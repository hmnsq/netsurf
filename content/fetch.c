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

#समावेश <stdlib.h>
#समावेश <assert.h>
#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <time.h>
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/ring.h"
#समावेश "netsurf/misc.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/fetch.h"
#समावेश "content/fetchers.h"
#समावेश "content/fetchers/resource.h"
#समावेश "content/fetchers/about/about.h"
#समावेश "content/fetchers/curl.h"
#समावेश "content/fetchers/data.h"
#समावेश "content/fetchers/file/file.h"
#समावेश "javascript/fetcher.h"
#समावेश "content/urldb.h"

/** The maximum number of fetchers that can be added */
#घोषणा MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#घोषणा SCHEDULE_TIME 10

/** The fdset timeout in ms */
#घोषणा FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
प्रकार काष्ठा scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	काष्ठा fetcher_operation_table ops; /**< The fetchers operations. */
	पूर्णांक refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

अटल scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
काष्ठा fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	व्योम *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_code http_code;	/**< HTTP response code, or 0. */
	पूर्णांक fetcherd;           /**< Fetcher descriptor for this fetch */
	व्योम *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_active;	/**< This fetch is active. */
	fetch_संदेश_type last_संदेश;/**< The last message sent for this fetch */
	काष्ठा fetch *r_पूर्व;	/**< Previous active fetch in ::fetch_ring. */
	काष्ठा fetch *r_अग्र;	/**< Next active fetch in ::fetch_ring. */
};

अटल काष्ठा fetch *fetch_ring = शून्य;	/**< Ring of active fetches. */
अटल काष्ठा fetch *queue_ring = शून्य;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

अटल अंतरभूत व्योम fetch_ref_fetcher(पूर्णांक fetcherd)
{
	fetchers[fetcherd].refcount++;
}

अटल अंतरभूत व्योम fetch_unref_fetcher(पूर्णांक fetcherd)
{
	fetchers[fetcherd].refcount--;
	अगर (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
अटल पूर्णांक get_fetcher_for_scheme(lwc_string *scheme)
{
	पूर्णांक fetcherd;
	bool match;

	क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		अगर ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &match) == lwc_error_ok) &&
		    (match == true)) {
			वापस fetcherd;
	       }
	}
	वापस -1;
}

/**
 * Dispatch a single job
 */
अटल bool fetch_dispatch_job(काष्ठा fetch *fetch)
{
	RING_REMOVE(queue_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	अगर (!fetchers[fetch->fetcherd].ops.start(fetch->fetcher_handle)) {
		RING_जोड़ो(queue_ring, fetch); /* Put it back on the end of the queue */
		वापस false;
	} अन्यथा {
		RING_जोड़ो(fetch_ring, fetch);
		fetch->fetch_is_active = true;
		वापस true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
अटल bool fetch_choose_and_dispatch(व्योम)
{
	bool same_host;
	काष्ठा fetch *queueitem;
	queueitem = queue_ring;
	करो {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		पूर्णांक countbyhost;
		RING_COUNTBYLWCHOST(काष्ठा fetch, fetch_ring, countbyhost,
				    queueitem->host);
		अगर (countbyhost < nsoption_int(max_fetchers_per_host)) {
			/* We can dispatch this item in theory */
			वापस fetch_dispatch_job(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		जबतक (same_host == true && queueitem->r_अग्र != queue_ring) {
			अगर (lwc_string_isequal(queueitem->host,
					       queueitem->r_अग्र->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_अग्र;
			}
		}
		queueitem = queueitem->r_अग्र;
	} जबतक (queueitem != queue_ring);
	वापस false;
}

अटल व्योम dump_rings(व्योम)
{
	काष्ठा fetch *q;
	काष्ठा fetch *f;

	q = queue_ring;
	अगर (q) {
		करो {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_अग्र;
		} जबतक (q != queue_ring);
	}
	f = fetch_ring;
	अगर (f) {
		करो {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_अग्र;
		} जबतक (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
अटल bool fetch_dispatch_jobs(व्योम)
{
	पूर्णांक all_active;
	पूर्णांक all_queued;

	RING_GETSIZE(काष्ठा fetch, queue_ring, all_queued);
	RING_GETSIZE(काष्ठा fetch, fetch_ring, all_active);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_active);
	dump_rings();

	जबतक ((all_queued != 0) &&
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

	वापस (all_active > 0);
}

अटल व्योम fetcher_poll(व्योम *unused)
{
	पूर्णांक fetcherd;

	अगर (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			अगर (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(SCHEDULE_TIME, fetcher_poll, शून्य);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(व्योम)
{
	nserror ret;

#अगर_घोषित WITH_CURL
	ret = fetch_curl_register();
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}
#पूर्ण_अगर

	ret = fetch_तथ्य_register();
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	ret = fetch_file_register();
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	ret = fetch_resource_register();
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	ret = fetch_about_register();
	अगर (ret != NSERROR_OK) {
		वापस ret;
	}

	ret = fetch_javascript_register();

	वापस ret;
}

/* exported interface documented in content/fetchers.h */
व्योम fetcher_quit(व्योम)
{
	पूर्णांक fetcherd; /* fetcher index */
	क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		अगर (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_तथ्य(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		अगर (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_add(lwc_string *scheme, स्थिर काष्ठा fetcher_operation_table *ops)
{
	पूर्णांक fetcherd;

	/* find unused fetcher descriptor */
	क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		अगर (fetchers[fetcherd].refcount == 0) {
			अवरोध;
		}
	}
	अगर (fetcherd == MAX_FETCHERS) {
		वापस NSERROR_INIT_FAILED;
	}

	अगर (!ops->initialise(scheme)) {
		वापस NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	वापस NSERROR_OK;
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

	अगर (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_out = -1;
		वापस NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		अगर (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_set);
	FD_ZERO(write_fd_set);
	FD_ZERO(except_fd_set);

	क्रम (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		अगर ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != शून्य)) {
			/* fetcher present */
			पूर्णांक fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_set,
				write_fd_set, except_fd_set);
			अगर (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	अगर (maxfd >= 0) {
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
		guit->misc->schedule(FDSET_TIMEOUT, fetcher_poll, शून्य);
	}

	*maxfd_out = maxfd;

	वापस NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_start(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    व्योम *p,
	    bool only_2xx,
	    स्थिर अक्षर *post_urlenc,
	    स्थिर काष्ठा fetch_multipart_तथ्य *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    स्थिर अक्षर *headers[],
	    काष्ठा fetch **fetch_out)
{
	काष्ठा fetch *fetch;
	lwc_string *scheme;

	fetch = सुस्मृति(1, माप (*fetch));
	अगर (fetch == शून्य) {
		वापस NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	निश्चित(scheme != शून्य);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	अगर (fetch->fetcherd == -1) {
		मुक्त(fetch);
		वापस NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	अगर (referer != शून्य) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.setup(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						headers);
	अगर (fetch->fetcher_handle == शून्य) {
		lwc_string_unref(fetch->host);

		अगर (fetch->url != शून्य)
			nsurl_unref(fetch->url);

		अगर (fetch->referer != शून्य)
			nsurl_unref(fetch->referer);

		मुक्त(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		वापस NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_जोड़ो(queue_ring, fetch);

	/* Ask the queue to run. */
	अगर (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->schedule(10, fetcher_poll, शून्य);
	}

	*fetch_out = fetch;
	वापस NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
व्योम fetch_abort(काष्ठा fetch *f)
{
	निश्चित(f);
	f->last_संदेश = FETCH__INTERNAL_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.पात(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
व्योम fetch_free(काष्ठा fetch *f)
{
	अगर (f->last_संदेश < FETCH_MIN_FINISHED_संदेश) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_संदेश संदेश;

		संदेश.type = FETCH_ERROR;
		संदेश.तथ्य.error = "FetchFailedToFinish";

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
	अगर (f->referer != शून्य) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	मुक्त(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(स्थिर nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	पूर्णांक fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	अगर (fetcherd == -1) {
		वापस false;
	}

	वापस fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
व्योम fetch_change_callback(काष्ठा fetch *fetch,
			   fetch_callback callback,
			   व्योम *p)
{
	निश्चित(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_code fetch_http_code(काष्ठा fetch *fetch)
{
	वापस fetch->http_code;
}


/* exported interface documented in content/fetch.h */
काष्ठा fetch_multipart_तथ्य *
fetch_multipart_तथ्य_clone(स्थिर काष्ठा fetch_multipart_तथ्य *list)
{
	काष्ठा fetch_multipart_तथ्य *clone, *last = शून्य;
	काष्ठा fetch_multipart_तथ्य *परिणाम = शून्य;

	क्रम (; list != शून्य; list = list->अग्र) {
		clone = दो_स्मृति(माप(काष्ठा fetch_multipart_तथ्य));
		अगर (clone == शून्य) {
			अगर (परिणाम != शून्य)
				fetch_multipart_तथ्य_destroy(परिणाम);

			वापस शून्य;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		अगर (clone->name == शून्य) {
			मुक्त(clone);
			अगर (परिणाम != शून्य)
				fetch_multipart_तथ्य_destroy(परिणाम);

			वापस शून्य;
		}

		clone->मूल्य = strdup(list->मूल्य);
		अगर (clone->मूल्य == शून्य) {
			मुक्त(clone->name);
			मुक्त(clone);
			अगर (परिणाम != शून्य)
				fetch_multipart_तथ्य_destroy(परिणाम);

			वापस शून्य;
		}

		अगर (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			अगर (clone->rawfile == शून्य) {
				मुक्त(clone->मूल्य);
				मुक्त(clone->name);
				मुक्त(clone);
				अगर (परिणाम != शून्य)
					fetch_multipart_तथ्य_destroy(परिणाम);

				वापस शून्य;
			}
		} अन्यथा {
			clone->rawfile = शून्य;
		}

		clone->अग्र = शून्य;

		अगर (परिणाम == शून्य)
			परिणाम = clone;
		अन्यथा
			last->अग्र = clone;

		last = clone;
	}

	वापस परिणाम;
}


/* exported interface documented in content/fetch.h */
स्थिर अक्षर *
fetch_multipart_तथ्य_find(स्थिर काष्ठा fetch_multipart_तथ्य *list,
			  स्थिर अक्षर *name)
{
	जबतक (list != शून्य) {
		अगर (म_भेद(list->name, name) == 0) {
			वापस list->मूल्य;
		}
		list = list->अग्र;
	}

	वापस शून्य;
}


/* exported interface documented in content/fetch.h */
व्योम fetch_multipart_तथ्य_destroy(काष्ठा fetch_multipart_तथ्य *list)
{
	काष्ठा fetch_multipart_तथ्य *अग्र;

	क्रम (; list != शून्य; list = अग्र) {
		अग्र = list->अग्र;
		मुक्त(list->name);
		मुक्त(list->मूल्य);
		अगर (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			मुक्त(list->rawfile);
		}
		मुक्त(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_तथ्य_new_kv(काष्ठा fetch_multipart_तथ्य **list,
			    स्थिर अक्षर *name,
			    स्थिर अक्षर *मूल्य)
{
	काष्ठा fetch_multipart_तथ्य *newdata;

	निश्चित(list);

	newdata = सुस्मृति(1, माप(*newdata));

	अगर (newdata == शून्य) {
		वापस NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	अगर (newdata->name == शून्य) {
		मुक्त(newdata);
		वापस NSERROR_NOMEM;
	}

	newdata->मूल्य = strdup(मूल्य);
	अगर (newdata->मूल्य == शून्य) {
		मुक्त(newdata->name);
		मुक्त(newdata);
		वापस NSERROR_NOMEM;
	}

	newdata->अग्र = *list;
	*list = newdata;

	वापस NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
व्योम
fetch_send_callback(स्थिर fetch_संदेश *संदेश, काष्ठा fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	अगर (संदेश->type > fetch->last_संदेश)
		fetch->last_संदेश = संदेश->type;
	fetch->callback(संदेश, fetch->p);
}


/* exported interface documented in content/fetch.h */
व्योम fetch_remove_from_queues(काष्ठा fetch *fetch)
{
	पूर्णांक all_active;
	पूर्णांक all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	अगर (fetch->fetch_is_active) {
		RING_REMOVE(fetch_ring, fetch);
	} अन्यथा {
		RING_REMOVE(queue_ring, fetch);
	}


	RING_GETSIZE(काष्ठा fetch, fetch_ring, all_active);
	RING_GETSIZE(काष्ठा fetch, queue_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_active);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_set_http_code(काष्ठा fetch *fetch, http_response_code http_code)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_code);

	fetch->http_code = http_code;

	वापस NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
व्योम fetch_set_cookie(काष्ठा fetch *fetch, स्थिर अक्षर *तथ्य)
{
	निश्चित(fetch && तथ्य);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	अगर (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_set_cookie(तथ्य, fetch->url, शून्य);
	} अन्यथा अगर (fetch->referer != शून्य) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_set_cookie(तथ्य, fetch->url, fetch->referer);
	}
}
