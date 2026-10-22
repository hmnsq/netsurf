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

#شامل <stdlib.h>
#شامل <assert.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/nsurl.h"
#شامل "utils/ring.h"
#شامل "netsurf/misc.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/fetchers.h"
#شامل "content/fetchers/resource.h"
#شامل "content/fetchers/about/about.h"
#شامل "content/fetchers/curl.h"
#شامل "content/fetchers/data.h"
#شامل "content/fetchers/file/file.h"
#شامل "javascript/fetcher.h"
#شامل "content/urldb.h"

/** The maximum number of fetchers that can be added */
#تعریف_کرو MAX_FETCHERS 10

/** The time in ms between polling the fetchers.
 *
 * \todo The schedule timeout should be profiled to see if there is a
 * better value or even if it needs to be dynamic.
 */
#تعریف_کرو جدول_وقت_وقت 10

/** The fdset timeout in ms */
#تعریف_کرو FDSET_TIMEOUT 1000

/**
 * Information about a fetcher for a given scheme.
 */
قسمتعریف ساخت scheme_fetcher_s {
	lwc_string *scheme; /**< The scheme. */

	ساخت fetcher_operation_جدول ops; /**< The fetchers operations. */
	صحیح refcount; /**< When zero the fetcher is no longer in use. */
} scheme_fetcher;

ساکن scheme_fetcher fetchers[MAX_FETCHERS];

/** Information for a single fetch. */
ساخت fetch {
	fetch_callback callback;/**< Callback function. */
	nsurl *url;		/**< URL. */
	nsurl *referer;		/**< Referer URL. */
	bool verifiable;	/**< Transaction is verifiable */
	خالی *p;		/**< Private data for callback. */
	lwc_string *host;	/**< Host part of URL, interned */
	http_response_رمز http_رمز;	/**< HTTP response code, or 0. */
	صحیح fetcherd;           /**< Fetcher descriptor for this fetch */
	خالی *fetcher_handle;	/**< The handle for the fetcher. */
	bool fetch_is_فعال;	/**< This fetch is active. */
	fetch_پیغام_type آخری_پیغام;/**< The last message sent for this fetch */
	ساخت fetch *r_پچھلا;	/**< Previous active fetch in ::fetch_ring. */
	ساخت fetch *r_اگلا;	/**< Next active fetch in ::fetch_ring. */
};

ساکن ساخت fetch *fetch_ring = تہی;	/**< Ring of active fetches. */
ساکن ساخت fetch *قطار_ring = تہی;	/**< Ring of queued fetches */

/******************************************************************************
 * fetch internals							      *
 ******************************************************************************/

ساکن اندرونی خالی fetch_ref_fetcher(صحیح fetcherd)
{
	fetchers[fetcherd].refcount++;
}

ساکن اندرونی خالی fetch_unref_fetcher(صحیح fetcherd)
{
	fetchers[fetcherd].refcount--;
	اگر (fetchers[fetcherd].refcount == 0) {
		fetchers[fetcherd].ops.finalise(fetchers[fetcherd].scheme);
		lwc_string_unref(fetchers[fetcherd].scheme);
	}
}

/**
 * Find a suitable fetcher for a scheme.
 */
ساکن صحیح get_fetcher_for_scheme(lwc_string *scheme)
{
	صحیح fetcherd;
	bool مطابقت;

	برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر ((fetchers[fetcherd].refcount > 0) &&
		    (lwc_string_isequal(fetchers[fetcherd].scheme,
					scheme, &مطابقت) == lwc_error_ok) &&
		    (مطابقت == true)) {
			واپس fetcherd;
	       }
	}
	واپس -1;
}

/**
 * Dispatch a single job
 */
ساکن bool fetch_dispatch_کام(ساخت fetch *fetch)
{
	RING_REMOVE(قطار_ring, fetch);
	NSLOG(fetch, DEBUG,
	      "Attempting to start fetch %p, fetcher %p, url %s", fetch,
	      fetch->fetcher_handle,
	      nsurl_access(fetch->url));

	اگر (!fetchers[fetch->fetcherd].ops.آغاز(fetch->fetcher_handle)) {
		RING_داخل_کرو(قطار_ring, fetch); /* Put it back on the end of the queue */
		واپس false;
	} ورنہ {
		RING_داخل_کرو(fetch_ring, fetch);
		fetch->fetch_is_فعال = true;
		واپس true;
	}
}

/**
 * Choose and dispatch a single job. Return false if we failed to dispatch
 * anything.
 *
 * We don't check the overall dispatch size here because we're not called unless
 * there is room in the fetch queue for us.
 */
ساکن bool fetch_انتخاب_and_dispatch(خالی)
{
	bool same_host;
	ساخت fetch *queueitem;
	queueitem = قطار_ring;
	کرو {
		/* We can dispatch the selected item if there is room in the
		 * fetch ring
		 */
		صحیح countbyhost;
		RING_COUNTBYLWCHOST(ساخت fetch, fetch_ring, countbyhost,
				    queueitem->host);
		اگر (countbyhost < nsoption_int(max_fetchers_فی_host)) {
			/* We can dispatch this item in theory */
			واپس fetch_dispatch_کام(queueitem);
		}
		/* skip over other items with the same host */
		same_host = true;
		جب (same_host == true && queueitem->r_اگلا != قطار_ring) {
			اگر (lwc_string_isequal(queueitem->host,
					       queueitem->r_اگلا->host, &same_host) ==
			    lwc_error_ok && same_host == true) {
				queueitem = queueitem->r_اگلا;
			}
		}
		queueitem = queueitem->r_اگلا;
	} جب (queueitem != قطار_ring);
	واپس false;
}

ساکن خالی dump_rings(خالی)
{
	ساخت fetch *q;
	ساخت fetch *f;

	q = قطار_ring;
	اگر (q) {
		کرو {
			NSLOG(fetch, DEBUG, "queue_ring: %s",
			      nsurl_access(q->url));
			q = q->r_اگلا;
		} جب (q != قطار_ring);
	}
	f = fetch_ring;
	اگر (f) {
		کرو {
			NSLOG(fetch, DEBUG, "fetch_ring: %s",
			      nsurl_access(f->url));
			f = f->r_اگلا;
		} جب (f != fetch_ring);
	}
}

/**
 * Dispatch as many jobs as we have room to dispatch.
 *
 * @return true if there are active fetchers that require polling else false.
 */
ساکن bool fetch_dispatch_jobs(خالی)
{
	صحیح all_فعال;
	صحیح all_queued;

	RING_GETSIZE(ساخت fetch, قطار_ring, all_queued);
	RING_GETSIZE(ساخت fetch, fetch_ring, all_فعال);

	NSLOG(fetch, DEBUG,
	      "queue_ring %i, fetch_ring %i",
	      all_queued,
	      all_فعال);
	dump_rings();

	جب ((all_queued != 0) &&
	       (all_فعال < nsoption_int(max_fetchers)) &&
	       fetch_انتخاب_and_dispatch()) {
			all_queued--;
			all_فعال++;
			NSLOG(fetch, DEBUG,
			      "%d queued, %d fetching",
			      all_queued,
			      all_فعال);
	}

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_فعال);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);

	واپس (all_فعال > 0);
}

ساکن خالی fetcher_poll(خالی *غیر_مستعمل)
{
	صحیح fetcherd;

	اگر (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "Polling fetchers");
		برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
			اگر (fetchers[fetcherd].refcount > 0) {
				/* fetcher present */
				fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
			}
		}

		/* schedule active fetchers to run again in 10ms */
		guit->misc->جدول_وقت(جدول_وقت_وقت, fetcher_poll, تہی);
	}
}

/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/* exported interface documented in content/fetch.h */
nserror fetcher_init(خالی)
{
	nserror ret;

#اگر_تعریف_شدہ WITH_CURL
	ret = fetch_curl_رجسٹر();
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}
#اگر_ختم

	ret = fetch_ڈیٹا_رجسٹر();
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}

	ret = fetch_file_رجسٹر();
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}

	ret = fetch_resource_رجسٹر();
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}

	ret = fetch_about_رجسٹر();
	اگر (ret != NSERROR_OK) {
		واپس ret;
	}

	ret = fetch_javascript_رجسٹر();

	واپس ret;
}

/* exported interface documented in content/fetchers.h */
خالی fetcher_quit(خالی)
{
	صحیح fetcherd; /* fetcher index */
	برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount > 1) {
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
			      lwc_string_ڈیٹا(fetchers[fetcherd].scheme),
			      fetchers[fetcherd].refcount);

			fetchers[fetcherd].refcount = 1;
		}
		اگر (fetchers[fetcherd].refcount == 1) {

			fetch_unref_fetcher(fetcherd);
		}
	}
}

/* exported interface documented in content/fetchers.h */
nserror
fetcher_جوڑو(lwc_string *scheme, ثابت ساخت fetcher_operation_جدول *ops)
{
	صحیح fetcherd;

	/* find unused fetcher descriptor */
	برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount == 0) {
			توڑو;
		}
	}
	اگر (fetcherd == MAX_FETCHERS) {
		واپس NSERROR_INIT_FAILED;
	}

	اگر (!ops->initialise(scheme)) {
		واپس NSERROR_INIT_FAILED;
	}

	fetchers[fetcherd].scheme = scheme;
	fetchers[fetcherd].ops = *ops;

	fetch_ref_fetcher(fetcherd);

	واپس NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_fdset(fd_set *read_fd_سیٹ,
		      fd_set *write_fd_سیٹ,
		      fd_set *except_fd_سیٹ,
		      صحیح *maxfd_برآمد)
{
	صحیح maxfd = -1;
	صحیح fetcherd; /* fetcher index */

	اگر (!fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "No jobs");
		*maxfd_برآمد = -1;
		واپس NSERROR_OK;
	}

	NSLOG(fetch, DEBUG, "Polling fetchers");

	برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر (fetchers[fetcherd].refcount > 0) {
			/* fetcher present */
			fetchers[fetcherd].ops.poll(fetchers[fetcherd].scheme);
		}
	}

	FD_ZERO(read_fd_سیٹ);
	FD_ZERO(write_fd_سیٹ);
	FD_ZERO(except_fd_سیٹ);

	برائے (fetcherd = 0; fetcherd < MAX_FETCHERS; fetcherd++) {
		اگر ((fetchers[fetcherd].refcount > 0) &&
		    (fetchers[fetcherd].ops.fdset != تہی)) {
			/* fetcher present */
			صحیح fetcher_maxfd;
			fetcher_maxfd = fetchers[fetcherd].ops.fdset(
				fetchers[fetcherd].scheme, read_fd_سیٹ,
				write_fd_سیٹ, except_fd_سیٹ);
			اگر (fetcher_maxfd > maxfd)
				maxfd = fetcher_maxfd;
		}
	}

	اگر (maxfd >= 0) {
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
		guit->misc->جدول_وقت(FDSET_TIMEOUT, fetcher_poll, تہی);
	}

	*maxfd_برآمد = maxfd;

	واپس NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
nserror
fetch_آغاز(nsurl *url,
	    nsurl *referer,
	    fetch_callback callback,
	    خالی *p,
	    bool only_2xx,
	    ثابت حرف *post_urlenc,
	    ثابت ساخت fetch_multipart_ڈیٹا *post_multipart,
	    bool verifiable,
	    bool downgrade_tls,
	    ثابت حرف *سرخیاں[],
	    ساخت fetch **fetch_برآمد)
{
	ساخت fetch *fetch;
	lwc_string *scheme;

	fetch = صفر_حافظہ_دو(1, حجم (*fetch));
	اگر (fetch == تہی) {
		واپس NSERROR_NOMEM;
	}

	/* The URL we're fetching must have a scheme */
	scheme = nsurl_get_component(url, NSURL_SCHEME);
	أكد(scheme != تہی);

	/* try and obtain a fetcher for this scheme */
	fetch->fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);
	اگر (fetch->fetcherd == -1) {
		آزاد(fetch);
		واپس NSERROR_NO_FETCH_HANDLER;
	}

	NSLOG(fetch, DEBUG, "fetch %p, url '%s'", fetch, nsurl_access(url));

	/* construct a new fetch structure */
	fetch->callback = callback;
	fetch->url = nsurl_ref(url);
	fetch->verifiable = verifiable;
	fetch->p = p;
	fetch->host = nsurl_get_component(url, NSURL_HOST);

	اگر (referer != تہی) {
		fetch->referer = nsurl_ref(referer);
	}

	/* try and set up the fetch */
	fetch->fetcher_handle = fetchers[fetch->fetcherd].ops.تیاری(fetch, url,
						only_2xx, downgrade_tls,
						post_urlenc, post_multipart,
						سرخیاں);
	اگر (fetch->fetcher_handle == تہی) {
		lwc_string_unref(fetch->host);

		اگر (fetch->url != تہی)
			nsurl_unref(fetch->url);

		اگر (fetch->referer != تہی)
			nsurl_unref(fetch->referer);

		آزاد(fetch);


	/** \todo The fetchers setup should return nserror and that be
	 * passed back rather than assuming a bad url
	 */
		واپس NSERROR_BAD_URL;
	}

	/* Rah, got it, so ref the fetcher. */
	fetch_ref_fetcher(fetch->fetcherd);

	/* Dump new fetch in the queue. */
	RING_داخل_کرو(قطار_ring, fetch);

	/* Ask the queue to run. */
	اگر (fetch_dispatch_jobs()) {
		NSLOG(fetch, DEBUG, "scheduling poll");
		/* schedule active fetchers to run again in 10ms */
		guit->misc->جدول_وقت(10, fetcher_poll, تہی);
	}

	*fetch_برآمد = fetch;
	واپس NSERROR_OK;
}

/* exported interface documented in content/fetch.h */
خالی fetch_abort(ساخت fetch *f)
{
	أكد(f);
	f->آخری_پیغام = FETCH_اندرونی_ABORTED;
	NSLOG(fetch, DEBUG,
	      "fetch %p, fetcher %p, url '%s'", f, f->fetcher_handle,
	     nsurl_access(f->url));
	fetchers[f->fetcherd].ops.abort(f->fetcher_handle);
}

/* exported interface documented in content/fetch.h */
خالی fetch_free(ساخت fetch *f)
{
	اگر (f->آخری_پیغام < FETCH_MIN_ختم_شدہ_پیغام) {
		/* We didn't finish, so tell our user that an error occurred */
		fetch_پیغام پیغام;

		پیغام.type = FETCH_ERROR;
		پیغام.ڈیٹا.error = "FetchFailedToFinish";

		NSLOG(fetch, CRITICAL,
		      "During the fetch of %s, the fetcher did not finish.",
		      nsurl_access(f->url));

		fetch_send_callback(&پیغام, f);
	}

	NSLOG(fetch, DEBUG,
	      "Freeing fetch %p, fetcher %p",
	      f,
	      f->fetcher_handle);

	fetchers[f->fetcherd].ops.آزاد(f->fetcher_handle);

	fetch_unref_fetcher(f->fetcherd);

	nsurl_unref(f->url);
	اگر (f->referer != تہی) {
		nsurl_unref(f->referer);
	}
	lwc_string_unref(f->host);
	آزاد(f);
}



/* exported interface documented in content/fetch.h */
bool fetch_can_fetch(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	صحیح fetcherd;

	fetcherd = get_fetcher_for_scheme(scheme);
	lwc_string_unref(scheme);

	اگر (fetcherd == -1) {
		واپس false;
	}

	واپس fetchers[fetcherd].ops.acceptable(url);
}

/* exported interface documented in content/fetch.h */
خالی fetch_change_callback(ساخت fetch *fetch,
			   fetch_callback callback,
			   خالی *p)
{
	أكد(fetch);
	fetch->callback = callback;
	fetch->p = p;
}

/* exported interface documented in content/fetch.h */
http_response_رمز fetch_http_رمز(ساخت fetch *fetch)
{
	واپس fetch->http_رمز;
}


/* exported interface documented in content/fetch.h */
ساخت fetch_multipart_ڈیٹا *
fetch_multipart_ڈیٹا_clone(ثابت ساخت fetch_multipart_ڈیٹا *list)
{
	ساخت fetch_multipart_ڈیٹا *clone, *آخری = تہی;
	ساخت fetch_multipart_ڈیٹا *نتیجہ = تہی;

	برائے (; list != تہی; list = list->اگلا) {
		clone = حافظہ_دو(حجم(ساخت fetch_multipart_ڈیٹا));
		اگر (clone == تہی) {
			اگر (نتیجہ != تہی)
				fetch_multipart_ڈیٹا_destroy(نتیجہ);

			واپس تہی;
		}

		clone->file = list->file;

		clone->name = strdup(list->name);
		اگر (clone->name == تہی) {
			آزاد(clone);
			اگر (نتیجہ != تہی)
				fetch_multipart_ڈیٹا_destroy(نتیجہ);

			واپس تہی;
		}

		clone->قدر = strdup(list->قدر);
		اگر (clone->قدر == تہی) {
			آزاد(clone->name);
			آزاد(clone);
			اگر (نتیجہ != تہی)
				fetch_multipart_ڈیٹا_destroy(نتیجہ);

			واپس تہی;
		}

		اگر (clone->file) {
			clone->rawfile = strdup(list->rawfile);
			اگر (clone->rawfile == تہی) {
				آزاد(clone->قدر);
				آزاد(clone->name);
				آزاد(clone);
				اگر (نتیجہ != تہی)
					fetch_multipart_ڈیٹا_destroy(نتیجہ);

				واپس تہی;
			}
		} ورنہ {
			clone->rawfile = تہی;
		}

		clone->اگلا = تہی;

		اگر (نتیجہ == تہی)
			نتیجہ = clone;
		ورنہ
			آخری->اگلا = clone;

		آخری = clone;
	}

	واپس نتیجہ;
}


/* exported interface documented in content/fetch.h */
ثابت حرف *
fetch_multipart_ڈیٹا_ڈھونڈو(ثابت ساخت fetch_multipart_ڈیٹا *list,
			  ثابت حرف *name)
{
	جب (list != تہی) {
		اگر (strcmp(list->name, name) == 0) {
			واپس list->قدر;
		}
		list = list->اگلا;
	}

	واپس تہی;
}


/* exported interface documented in content/fetch.h */
خالی fetch_multipart_ڈیٹا_destroy(ساخت fetch_multipart_ڈیٹا *list)
{
	ساخت fetch_multipart_ڈیٹا *اگلا;

	برائے (; list != تہی; list = اگلا) {
		اگلا = list->اگلا;
		آزاد(list->name);
		آزاد(list->قدر);
		اگر (list->file) {
			NSLOG(fetch, DEBUG,
			      "Freeing rawfile: %s", list->rawfile);
			آزاد(list->rawfile);
		}
		آزاد(list);
	}
}


/* exported interface documented in content/fetch.h */
nserror
fetch_multipart_ڈیٹا_new_kv(ساخت fetch_multipart_ڈیٹا **list,
			    ثابت حرف *name,
			    ثابت حرف *قدر)
{
	ساخت fetch_multipart_ڈیٹا *newdata;

	أكد(list);

	newdata = صفر_حافظہ_دو(1, حجم(*newdata));

	اگر (newdata == تہی) {
		واپس NSERROR_NOMEM;
	}

	newdata->name = strdup(name);
	اگر (newdata->name == تہی) {
		آزاد(newdata);
		واپس NSERROR_NOMEM;
	}

	newdata->قدر = strdup(قدر);
	اگر (newdata->قدر == تہی) {
		آزاد(newdata->name);
		آزاد(newdata);
		واپس NSERROR_NOMEM;
	}

	newdata->اگلا = *list;
	*list = newdata;

	واپس NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
خالی
fetch_send_callback(ثابت fetch_پیغام *پیغام, ساخت fetch *fetch)
{
	/* Bump the last_msg to the greatest seen msg */
	اگر (پیغام->type > fetch->آخری_پیغام)
		fetch->آخری_پیغام = پیغام->type;
	fetch->callback(پیغام, fetch->p);
}


/* exported interface documented in content/fetch.h */
خالی fetch_remove_from_queues(ساخت fetch *fetch)
{
	صحیح all_فعال;
	صحیح all_queued;

	NSLOG(fetch, DEBUG,
	      "Fetch %p, fetcher %p can be freed",
	      fetch,
	      fetch->fetcher_handle);

	/* Go ahead and free the fetch properly now */
	اگر (fetch->fetch_is_فعال) {
		RING_REMOVE(fetch_ring, fetch);
	} ورنہ {
		RING_REMOVE(قطار_ring, fetch);
	}


	RING_GETSIZE(ساخت fetch, fetch_ring, all_فعال);
	RING_GETSIZE(ساخت fetch, قطار_ring, all_queued);

	NSLOG(fetch, DEBUG, "Fetch ring is now %d elements.", all_فعال);
	NSLOG(fetch, DEBUG, "Queue ring is now %d elements.", all_queued);
}


/* exported interface documented in content/fetch.h */
nserror fetch_سیٹ_http_رمز(ساخت fetch *fetch, http_response_رمز http_رمز)
{
	NSLOG(fetch, DEBUG, "Setting HTTP code to %d", http_رمز);

	fetch->http_رمز = http_رمز;

	واپس NSERROR_OK;
}


/* exported interface documented in content/fetch.h */
خالی fetch_سیٹ_cookie(ساخت fetch *fetch, ثابت حرف *ڈیٹا)
{
	أكد(fetch && ڈیٹا);

	/* If the fetch is unverifiable err on the side of caution and
	 * do not set the cookie */

	اگر (fetch->verifiable) {
		/* If the transaction's verifiable, we don't require
		 * that the request uri and the parent domain match,
		 * so don't pass in any referer/parent in this case. */
		urldb_سیٹ_cookie(ڈیٹا, fetch->url, تہی);
	} ورنہ اگر (fetch->referer != تہی) {
		/* Permit the cookie to be set if the fetch is unverifiable
		 * and the fetch URI domain matches the referer. */
		/** \todo Long-term, this needs to be replaced with a
		 * comparison against the origin fetch URI. In the case
		 * where a nested object requests a fetch, the origin URI
		 * is the nested object's parent URI, whereas the referer
		 * for the fetch will be the nested object's URI. */
		urldb_سیٹ_cookie(ڈیٹا, fetch->url, fetch->referer);
	}
}
