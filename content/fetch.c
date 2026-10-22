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

#ضمن <stdlib.h>
#ضمن <assert.h>
#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/ring.h"
#ضمن "netsurf/misc.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/fetchers.h"
#ضمن "content/fetchers/resource.h"
#ضمن "content/fetchers/about/about.h"
#ضمن "content/fetchers/curl.h"
#ضمن "content/fetchers/data.h"
#ضمن "content/fetchers/file/file.h"
#ضمن "javascript/fetcher.h"
#ضمن "content/urldb.h"

/** The maximum number of fetchers that can be added */
#وصف_ڪريو MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#وصف_ڪريو وقت_جدول_وقت 10

/** The fdset timeout in ms */
#وصف_ڪريو FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
تعريفنوع بنية scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	بنية fetcher_operation_جدول ops; /**< The fetchers operations. */
	صحيح refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ساكن scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
بنية fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	فراغ *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_ڪوڊ http_ڪوڊ;	/**< HTTP response code, or 0. */
	صحيح fetcherd;           /**< Fetcher descriptor for this fetch */
	فراغ *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_سرگرم;	/**< This fetch is active. */
	fetch_نياپو_type آخري_نياپو;/**< The last message sent for this fetch */
	بنية fetch *r_prev;	/**< Previous active fetch in ::fetch_ring. */
	بنية fetch *r_ايندڙ;	/**< Next active fetch in ::fetch_ring. */
};

ساكن بنية fetch *fetch_ring = عدم;	/**< Ring of active fetches. */
ساكن بنية fetch *قطار_ring = عدم;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ساكن مضمن فراغ fetch_ref_fetcher(صحيح fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ساكن مضمن فراغ fetch_unref_fetcher(صحيح fetcherd)
{
	fetchers[fetcherd].refcount--;
	إذا (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ساكن صحيح get_fetcher_for_scheme(lwc_string *scheme)
{
	صحيح fetcherd;
	bool ميلاپ;

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		إذا ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &ميلاپ) == lwc_error_ok) &&
		    (ميلاپ == true)) {
			أرجع fetcherd;
	       }
	}
	أرجع -1;
}

/**
 * Dispatch a single job
 */
ساكن bool fetch_dispatch_ڪم(بنية fetch *fetch)
{
	RING_REMOVE(قطار_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	إذا (!fetchers[fetch->fetcherd].ops.شروعات(fetch->fetcher_handle)) {
		RING_وجهو(قطار_ring, fetch); /* Put it back on the end of the queue */
		أرجع false;
	} وإلا {
		RING_وجهو(fetch_ring, fetch);
		fetch->fetch_is_سرگرم = true;
		أرجع true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ساكن bool fetch_چونڊيو_and_dispatch(فراغ)
{
	bool same_host;
	بنية fetch *queueitem;
	queueitem = قطار_ring;
	افعل {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		صحيح countbyhost;
		RING_COUNTBYLWCHOST(بنية fetch, fetch_ring, countbyhost,
				    queueitem->host);
		إذا (countbyhost < nsoption_int(max_fetchers_في_host)) {
			/* We can dispatch this item in theory */
			أرجع fetch_dispatch_ڪم(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		طالما (same_host == true && queueitem->r_ايندڙ != قطار_ring) {
			إذا (lwc_string_isequal(queueitem->host,
					       queueitem->r_ايندڙ->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_ايندڙ;
			}
		}
		queueitem = queueitem->r_ايندڙ;
	} طالما (queueitem != قطار_ring);
	أرجع false;
}

ساكن فراغ dump_rings(فراغ)
{
	بنية fetch *q;
	بنية fetch *f;

	q = قطار_ring;
	إذا (q) {
		افعل {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_ايندڙ;
		} طالما (q != قطار_ring);
	}
	f = fetch_ring;
	إذا (f) {
		افعل {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_ايندڙ;
		} طالما (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ساكن bool fetch_dispatch_jobs(فراغ)
{
	صحيح all_سرگرم;
	صحيح all_queued;

	RING_GETSIZE(بنية fetch, قطار_ring, all_queued);
	RING_GETSIZE(بنية fetch, fetch_ring, all_سرگرم);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_سرگرم);
	dump_rings();

	طالما ((all_queued != 0) &&
	       (all_سرگرم < nsoption_int(max_fetchers)) &&
	       fetch_چونڊيو_and_dispatch()) {
			all_queued--;
			all_سرگرم++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_سرگرم);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_سرگرم);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	أرجع (all_سرگرم > 0);
}

ساكن فراغ fetcher_poll(فراغ *اڻ_استعمال_ٿيل)
{
	صحيح fetcherd;

	إذا (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			إذا (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->وقت_جدول(وقت_جدول_وقت, fetcher_poll, عدم);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(فراغ)
{
	nserror ret;

#جيڪڏهن_وصف_ٿيل WITH_CURL
	ret = fetch_curl_رجسٽر();
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}
#جيڪڏهن_پورو

	ret = fetch_معلومات_رجسٽر();
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	ret = fetch_file_رجسٽر();
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	ret = fetch_resource_رجسٽر();
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	ret = fetch_about_رجسٽر();
	إذا (ret != NSERROR_OK) {
		أرجع ret;
	}

	ret = fetch_javascript_رجسٽر();

	أرجع ret;
}

/* exported interface documented in content/fetchers.h */
فراغ fetcher_quit(فراغ)
{
	صحيح fetcherd; /* fetcher index */
	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		إذا (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_معلومات(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		إذا (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_جوڙيو(lwc_string *scheme, ثابت بنية fetcher_operation_جدول *ops)
{
	صحيح fetcherd;

	/* find unused fetcher descriptor */
	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		إذا (fetchers[fetcherd].refcount == 0) {
			اكسر;
		}
	}
	إذا (fetcherd == MAX_FETCHERS) {
		أرجع NSERROR_INIT_FAILED;
	}

	إذا (!ops->initialise(scheme)) {
		أرجع NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	أرجع NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_سيٽ,
		      fd_set *write_fd_سيٽ,
		      fd_set *except_fd_سيٽ,
		      صحيح *maxfd_ٻاهريون)
{
	صحيح maxfd = -1;
	صحيح fetcherd; /* fetcher index */

	إذا (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_ٻاهريون = -1;
		أرجع NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		إذا (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_سيٽ);
	FD_ZERO(write_fd_سيٽ);
	FD_ZERO(except_fd_سيٽ);

	لكل (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		إذا ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != عدم)) {
			/* fetcher present */
			صحيح fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_سيٽ,
				write_fd_سيٽ, except_fd_سيٽ);
			إذا (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	إذا (maxfd >= 0) {
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
		guit->misc->وقت_جدول(FDSET_TIMEOUT, fetcher_poll, عدم);
	}

	*maxfd_ٻاهريون = maxfd;

	أرجع NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_شروعات(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    فراغ *p,
	    bool only_2xx,
	    ثابت حرف *post_urlenc,
	    ثابت بنية fetch_multipart_معلومات *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ثابت حرف *سرا[],
	    بنية fetch **fetch_ٻاهريون)
{
	بنية fetch *fetch;
	lwc_string *scheme;

	fetch = صفر_يادگيري_ڏيو(1, حجم (*fetch));
	إذا (fetch == عدم) {
		أرجع NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	أكد(scheme != عدم);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	إذا (fetch->fetcherd == -1) {
		حرر(fetch);
		أرجع NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	إذا (referer != عدم) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.تياري(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						سرا);
	إذا (fetch->fetcher_handle == عدم) {
		lwc_string_unref(fetch->host);

		إذا (fetch->url != عدم)
			nsurl_unref(fetch->url);

		إذا (fetch->referer != عدم)
			nsurl_unref(fetch->referer);

		حرر(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		أرجع NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_وجهو(قطار_ring, fetch);

	/* Ask the queue to run. */
	إذا (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->وقت_جدول(10, fetcher_poll, عدم);
	}

	*fetch_ٻاهريون = fetch;
	أرجع NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
فراغ fetch_abort(بنية fetch *f)
{
	أكد(f);
	f->آخري_نياپو = FETCH_اندروني_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
فراغ fetch_free(بنية fetch *f)
{
	إذا (f->آخري_نياپو < FETCH_MIN_ختم_نياپو) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_نياپو نياپو;

		نياپو.type = FETCH_ERROR;
		نياپو.معلومات.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&نياپو, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.حرر(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	إذا (f->referer != عدم) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	حرر(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	صحيح fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	إذا (fetcherd == -1) {
		أرجع false;
	}

	أرجع fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
فراغ fetch_change_callback(بنية fetch *fetch,
			   fetch_callback callback,
			   فراغ *p)
{
	أكد(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_ڪوڊ fetch_http_ڪوڊ(بنية fetch *fetch)
{
	أرجع fetch->http_ڪوڊ;
}


/* exported interface documented in content/fetch.h */
بنية fetch_multipart_معلومات *
fetch_multipart_معلومات_clone(ثابت بنية fetch_multipart_معلومات *list)
{
	بنية fetch_multipart_معلومات *clone, *آخري = عدم;
	بنية fetch_multipart_معلومات *نتيجو = عدم;

	لكل (; list != عدم; list = list->ايندڙ) {
		clone = يادگيري_ڏيو(حجم(بنية fetch_multipart_معلومات));
		إذا (clone == عدم) {
			إذا (نتيجو != عدم)
				fetch_multipart_معلومات_destroy(نتيجو);

			أرجع عدم;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		إذا (clone->name == عدم) {
			حرر(clone);
			إذا (نتيجو != عدم)
				fetch_multipart_معلومات_destroy(نتيجو);

			أرجع عدم;
		}

		clone->قدر = strdup(list->قدر);
		إذا (clone->قدر == عدم) {
			حرر(clone->name);
			حرر(clone);
			إذا (نتيجو != عدم)
				fetch_multipart_معلومات_destroy(نتيجو);

			أرجع عدم;
		}

		إذا (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			إذا (clone->rawfile == عدم) {
				حرر(clone->قدر);
				حرر(clone->name);
				حرر(clone);
				إذا (نتيجو != عدم)
					fetch_multipart_معلومات_destroy(نتيجو);

				أرجع عدم;
			}
		} وإلا {
			clone->rawfile = عدم;
		}

		clone->ايندڙ = عدم;

		إذا (نتيجو == عدم)
			نتيجو = clone;
		وإلا
			آخري->ايندڙ = clone;

		آخري = clone;
	}

	أرجع نتيجو;
}


/* exported interface documented in content/fetch.h */
ثابت حرف *
fetch_multipart_معلومات_ڳوليو(ثابت بنية fetch_multipart_معلومات *list,
			  ثابت حرف *name)
{
	طالما (list != عدم) {
		إذا (strcmp(list->name, name) == 0) {
			أرجع list->قدر;
		}
		list = list->ايندڙ;
	}

	أرجع عدم;
}


/* exported interface documented in content/fetch.h */
فراغ fetch_multipart_معلومات_destroy(بنية fetch_multipart_معلومات *list)
{
	بنية fetch_multipart_معلومات *ايندڙ;

	لكل (; list != عدم; list = ايندڙ) {
		ايندڙ = list->ايندڙ;
		حرر(list->name);
		حرر(list->قدر);
		إذا (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			حرر(list->rawfile);
		}
		حرر(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_معلومات_new_kv(بنية fetch_multipart_معلومات **list,
			    ثابت حرف *name,
			    ثابت حرف *قدر)
{
	بنية fetch_multipart_معلومات *newdata;

	أكد(list);

	newdata = صفر_يادگيري_ڏيو(1, حجم(*newdata));

	إذا (newdata == عدم) {
		أرجع NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	إذا (newdata->name == عدم) {
		حرر(newdata);
		أرجع NSERROR_NOMEM;
	}

	newdata->قدر = strdup(قدر);
	إذا (newdata->قدر == عدم) {
		حرر(newdata->name);
		حرر(newdata);
		أرجع NSERROR_NOMEM;
	}

	newdata->ايندڙ = *list;
	*list = newdata;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
فراغ
fetch_send_callback(ثابت fetch_نياپو *نياپو, بنية fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	إذا (نياپو->type > fetch->آخري_نياپو)
		fetch->آخري_نياپو = نياپو->type;
	fetch->callback(نياپو, fetch->p);
}


/* exported interface documented in content/fetch.h */
فراغ fetch_remove_from_queues(بنية fetch *fetch)
{
	صحيح all_سرگرم;
	صحيح all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	إذا (fetch->fetch_is_سرگرم) {
		RING_REMOVE(fetch_ring, fetch);
	} وإلا {
		RING_REMOVE(قطار_ring, fetch);
	}


	RING_GETSIZE(بنية fetch, fetch_ring, all_سرگرم);
	RING_GETSIZE(بنية fetch, قطار_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_سرگرم);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_سيٽ_http_ڪوڊ(بنية fetch *fetch, http_response_ڪوڊ http_ڪوڊ)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_ڪوڊ);

	fetch->http_ڪوڊ = http_ڪوڊ;

	أرجع NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
فراغ fetch_سيٽ_cookie(بنية fetch *fetch, ثابت حرف *معلومات)
{
	أكد(fetch && معلومات);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	إذا (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_سيٽ_cookie(معلومات, fetch->url, عدم);
	} وإلا إذا (fetch->referer != عدم) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_سيٽ_cookie(معلومات, fetch->url, fetch->referer);
	}
}
