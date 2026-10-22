/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Low-level resource cache implementation
 *
 * This is the implementation of the low level cache. This cache
 * stores source objects in memory and may use a persistent backing
 * store to extend their lifetime.
 *
 * \todo fix writeout conditions and ordering.
 *
 * \todo instrument and (auto)tune
 *
 */

#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <nsutils/time.h>
#شامل <nsutils/base64.h>

#شامل "netsurf/inttypes.h"
#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/nsurl.h"
#شامل "utils/utils.h"
#شامل "utils/time.h"
#شامل "utils/http.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/misc.h"
#شامل "desktop/gui_internal.h"

#شامل "content/fetch.h"
#شامل "content/backing_store.h"
#شامل "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
قسمتعریف شمار {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_سرخیاں,		/**< Fetching headers */
	LLCACHE_FETCH_ڈیٹا,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
قسمتعریف ساخت llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
ساخت llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	خالی *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t بائٹیں;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
قسمتعریف ساخت llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_ہدف;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	ساخت llcache_object_user *پچھلا;	/**< Previous in list */
	ساخت llcache_object_user *اگلا;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
قسمتعریف ساخت {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_ڈیٹا *post;	/**< POST data, or NULL for GET */

	ساخت fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_گنتی;	/**< Count of redirects followed */

	uint32_t retries_بقایا;     /**< Number of times to retry on timeout */

	bool hsts_اندر_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
قسمتعریف شمار {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#تعریف_کرو INVALID_AGE -1

/** Cache control data */
قسمتعریف ساخت {
	time_t req_وقت;	/**< Time of request */
	time_t res_وقت;	/**< Time of response */
	time_t fin_وقت;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	صحیح age;		/**< Age: response header */
	صحیح max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	حرف *etag;		/**< Etag: response header */
	time_t آخری_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
قسمتعریف ساخت {
	حرف *name;		/**< Header name */
	حرف *قدر;		/**< Header value */
} llcache_سرخی;

/** Current status of an object's data */
قسمتعریف شمار {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
ساخت llcache_object {
	llcache_object *پچھلا;	     /**< Previous in list */
	llcache_object *اگلا;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *ماخذ_ڈیٹا;	     /**< Source data for object */
	size_t ماخذ_len;	     /**< Byte length of source data */
	size_t ماخذ_alloc;	     /**< Allocated size of source buffer */

	ساخت cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_گنتی;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_سرخی *سرخیاں;     /**< Fetch headers */
	size_t عدد_سرخیاں;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t آخری_مستعمل; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
ساخت llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t حد_بندی;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_اوپر;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	صحیح minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	غیرعلامتی طویل وقت_وقتی_حصہ;

	/**
	 * The minimum bandwidth to allow the backing store to use in
	 * bytes/second. Below this the backing store will be
	 * disabled.
	 */
	size_t minimum_bandwidth;

	/**
	 * The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	size_t زیادہ_سے_زیادہ_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t کل_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t کل_elapsed;

};

/** low level cache state */
ساکن ساخت llcache_s *llcache = تہی;

/* forward referenced callback function */
ساکن خالی llcache_fetch_callback(ثابت fetch_پیغام *پیغام, خالی *p);

/* forward referenced catch up function */
ساکن خالی llcache_users_not_caught_اوپر(خالی);


/******************************************************************************
 * Low-level cache internals						      *
 ******************************************************************************/

/**
 * Create a new object user.
 *
 * \param cb Callback routine.
 * \param pw Private data for callback.
 * \param user Pointer to location to receive result.
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_user_new(llcache_handle_callback cb, خالی *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = صفر_حافظہ_دو(1, حجم(llcache_handle));
	اگر (h == تہی) {
		واپس NSERROR_NOMEM;
	}

	u = صفر_حافظہ_دو(1, حجم(llcache_object_user));
	اگر (u == تہی) {
		آزاد(h);
		واپس NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (خالی *) cb, pw);

	*user = u;

	واپس NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
ساکن nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	أكد(user->اگلا == تہی);
	أكد(user->پچھلا == تہی);

	اگر (user->handle != تہی)
		آزاد(user->handle);

	آزاد(user);

	واپس NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
ساکن nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	أكد(user != تہی);
	أكد(object != تہی);
	أكد(object->users != تہی);
	أكد(user->handle == تہی || user->handle->object == object);
	أكد((user->پچھلا != تہی) || (object->users == user));

	اگر (user == object->users)
		object->users = user->اگلا;
	ورنہ
		user->پچھلا->اگلا = user->اگلا;

	اگر (user->اگلا != تہی)
		user->اگلا->پچھلا = user->پچھلا;

	user->اگلا = user->پچھلا = تہی;

	/* record the time the last user was removed from the object */
	اگر (object->users == تہی) {
		object->آخری_مستعمل = time(تہی);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	واپس NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساکن nserror llcache_send_event_مقصد_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *اگلا_user;

	user = object->users;
	جب (user != تہی) {
		bool was_ہدف = user->iterator_ہدف;
		user->iterator_ہدف = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		اگلا_user = user->اگلا;

		user->iterator_ہدف = was_ہدف;

		اگر (user->queued_for_delete && !was_ہدف) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		اگر (error != NSERROR_OK)
			توڑو;

		user = اگلا_user;
	}

	واپس error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_new(nsurl *url, llcache_object **نتیجہ)
{
	llcache_object *obj = صفر_حافظہ_دو(1, حجم(llcache_object));
	اگر (obj == تہی)
		واپس NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*نتیجہ = obj;

	واپس NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_post_ڈیٹا_clone(ثابت llcache_post_ڈیٹا *orig,
		llcache_post_ڈیٹا **clone)
{
	llcache_post_ڈیٹا *post_clone;

	post_clone = صفر_حافظہ_دو(1, حجم(llcache_post_ڈیٹا));
	اگر (post_clone == تہی)
		واپس NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	اگر (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->ڈیٹا.urlenc = strdup(orig->ڈیٹا.urlenc);
		اگر (post_clone->ڈیٹا.urlenc == تہی) {
			آزاد(post_clone);

			واپس NSERROR_NOMEM;
		}
	} ورنہ {
		post_clone->ڈیٹا.multipart = fetch_multipart_ڈیٹا_clone(
				orig->ڈیٹا.multipart);
		اگر (post_clone->ڈیٹا.multipart == تہی) {
			آزاد(post_clone);

			واپس NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	واپس NSERROR_OK;
}

/**
 * Split a fetch header into name and value
 *
 * HTTP header splitting according to grammar defined in RFC7230 section 3.2
 *   https://tools.ietf.org/html/rfc7230#section-3.2
 *
 * This implementation is non conformant in that it:
 *  - includes carrige return and newline in whitespace (3.2.3)
 *  - allows whitespace before and after the field-name token (3.2.4)
 *  - does not handle obsolete line folding (3.2.4)
 *
 * \param data	 Header string
 * \param len	 Byte length of header
 * \param name	 Pointer to location to receive header name
 * \param value	 Pointer to location to receive header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_fetch_بانٹو_سرخی(ثابت uint8_t *ڈیٹا, size_t len,
		حرف **name, حرف **قدر)
{
	حرف *n, *v;
	ثابت uint8_t *colon;

	/* Strip leading whitespace from name */
	جب (ڈیٹا[0] == ' ' || ڈیٹا[0] == '\t' ||
	       ڈیٹا[0] == '\r' || ڈیٹا[0] == '\n') {
		ڈیٹا++;
	}

	/* Find colon */
	colon = (ثابت uint8_t *) strchr((ثابت حرف *) ڈیٹا, ':');
	اگر (colon == تہی) {
		/* Failed, assume a key with no value */
		colon = ڈیٹا + لڑی_لمبائی((ثابت حرف *)ڈیٹا);

		/* Strip trailing whitespace from name */
		جب ((colon > ڈیٹا) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ثابت حرف *) ڈیٹا, colon - ڈیٹا);
		اگر (n == تہی)
			واپس NSERROR_NOMEM;

		v = strdup("");
		اگر (v == تہی) {
			آزاد(n);
			واپس NSERROR_NOMEM;
		}
	} ورنہ {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		جب (colon > ڈیٹا && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ثابت حرف *) ڈیٹا, colon - ڈیٹا);
		اگر (n == تہی)
			واپس NSERROR_NOMEM;

		/* Find colon again */
		جب (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		کرو {
			colon++;
		} جب (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		جب (len > 0 && (ڈیٹا[len - 1] == ' ' ||
				ڈیٹا[len - 1] == '\t' ||
				ڈیٹا[len - 1] == '\r' ||
				ڈیٹا[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ثابت حرف *) colon, len - (colon - ڈیٹا));
		اگر (v == تہی) {
			آزاد(n);
			واپس NSERROR_NOMEM;
		}
	}

	*name = n;
	*قدر = v;

	واپس NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
llcache_fetch_parse_cache_control(llcache_object *object, حرف *قدر)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(قدر, &cc);
	اگر (error != NSERROR_OK) {
		/* Ignore parse errors */
		واپس NSERROR_OK;
	}

	اگر (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	اگر (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	واپس NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
llcache_fetch_سرخی_cache_control(llcache_object *object,
				   حرف *name,
				   حرف *قدر)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = لڑی_لمبائی(name);

	انتخاب (name_len) {
	حالت 3:
		اگر (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			اگر ('0' <= *قدر && *قدر <= '9') {
				object->cache.age = atoi(قدر);
			}

		}
		توڑو;

	حالت 4:
		اگر (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(قدر,
					    لڑی_لمبائی(قدر),
					    &object->cache.date);
			اگر (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      قدر, res);
			}
		} ورنہ اگر (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			آزاد(object->cache.etag);
			object->cache.etag = strdup(قدر);
			اگر (object->cache.etag == تہی) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				واپس NSERROR_NOMEM;
			}
		}
		توڑو;

	حالت 7:
		اگر (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(قدر,
					    لڑی_لمبائی(قدر),
					    &object->cache.expires);
			اگر (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      قدر, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		توڑو;

	حالت 13:
		اگر (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,قدر);
		} ورنہ اگر (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(قدر,
				      لڑی_لمبائی(قدر),
				      &object->cache.آخری_modified);
		}
		توڑو;
	}

	واپس NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
ساکن اندرونی خالی llcache_destroy_سرخیاں(llcache_object *object)
{
	جب (object->عدد_سرخیاں > 0) {
		object->عدد_سرخیاں--;

		آزاد(object->سرخیاں[object->عدد_سرخیاں].name);
		آزاد(object->سرخیاں[object->عدد_سرخیاں].قدر);
	}
	آزاد(object->سرخیاں);
	object->سرخیاں = تہی;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
ساکن اندرونی خالی llcache_invalidate_cache_control_ڈیٹا(llcache_object *object)
{
	آزاد(object->cache.etag);
	memset(&(object->cache), 0, حجم(llcache_cache_control));

	object->cache.age = INVALID_AGE;
	object->cache.max_age = INVALID_AGE;
}

/**
 * Process a fetch header
 *
 * \param object  Object being fetched
 * \param data	  Header string
 * \param len	  Byte length of header
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_fetch_process_سرخی(llcache_object *object,
		ثابت uint8_t *ڈیٹا, size_t len)
{
	nserror res;
	حرف *name, *قدر;
	llcache_سرخی *عارضی;

	/**
	 * \note The headers for multiple HTTP responses may be
	 * delivered to us if the fetch layer receives a 401 response
	 * for which it has authentication credentials. This will
	 * result in a silent re-request after which we'll receive the
	 * actual response headers for the object we want to fetch
	 * (assuming that the credentials were correct of course)
	 *
	 * Therefore, if the header is an HTTP response start marker, then we
	 * must discard any headers we've read so far, reset the cache data
	 * that we might have computed, and start again.
	 */
	/** \todo Properly parse the response line */
	اگر (لڑی_موازنہ((ثابت حرف *) ڈیٹا, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_وقت = object->cache.req_وقت;

		llcache_invalidate_cache_control_ڈیٹا(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_وقت = req_وقت;

		llcache_destroy_سرخیاں(object);
	}

	/* Set fetch response time if not already set */
	اگر (object->cache.res_وقت == 0) {
		object->cache.res_وقت = time(تہی);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_بانٹو_سرخی(ڈیٹا, len, &name, &قدر);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	/* deal with empty header */
	اگر (name[0] == 0) {
		آزاد(name);
		آزاد(قدر);
		واپس NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_سرخی_cache_control(object, name, قدر);
	اگر (res != NSERROR_OK) {
		آزاد(name);
		آزاد(قدر);
		واپس res;
	}

	/* Append header data to the object's headers array */
	عارضی = realloc(object->سرخیاں,
		       (object->عدد_سرخیاں + 1) * حجم(llcache_سرخی));
	اگر (عارضی == تہی) {
		آزاد(name);
		آزاد(قدر);
		واپس NSERROR_NOMEM;
	}

	object->سرخیاں = عارضی;

	object->سرخیاں[object->عدد_سرخیاں].name = name;
	object->سرخیاں[object->عدد_سرخیاں].قدر = قدر;

	object->عدد_سرخیاں++;

	واپس NSERROR_OK;
}

/**
 * construct a Referer header appropriate for the request
 *
 * \param url The url being navigated to
 * \param referer The referring url
 * \param header_out A pointer to receive the header. The buffer must
 *                    be freed by the caller.
 * \return NSERROR_OK and \a header_out updated on success else error code
 */
ساکن nserror get_referer_سرخی(nsurl *url, nsurl *referer, حرف **سرخی_برآمد)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool مطابقت;
	bool match1;
	bool match2;
	حرف *سرخی;

	/* Determine whether to send the Referer header */
	اگر (!nsoption_bool(send_referer)) {
		واپس NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (scheme == تہی) {
		واپس NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	اگر (ref_scheme == تہی) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		واپس NSERROR_INVALID;
	}

	/* User permits us to send the header
	 * Only send it if:
	 *    1) The fetch and referer schemes match
	 * or 2) The fetch is https and the referer is http
	 *
	 * This ensures that referer information is only sent
	 * across schemes in the special case of an https
	 * request from a page served over http. The inverse
	 * (https -> http) should not send the referer (15.1.3)
	 */
	اگر (lwc_string_isequal(scheme, ref_scheme,
			       &مطابقت) != lwc_error_ok) {
		مطابقت = false;
	}
	اگر (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	اگر (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	اگر (مطابقت == true || (match1 == true && match2 == true)) {
		ثابت size_t len = SLEN("Referer: ") +
			nsurl_لمبائی(referer) + 1;

		سرخی = حافظہ_دو(len);
		اگر (سرخی == تہی) {
			res = NSERROR_NOMEM;
		} ورنہ {
			snprintf(سرخی, len, "Referer: %s",
				 nsurl_access(referer));

			*سرخی_برآمد = سرخی;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	واپس res;
}

/**
 * (Re)fetch an object
 *
 * Sets up headers and attempts to start an actual fetch from the
 * fetchers system updating the llcache object with the new fetch on
 * successful start.
 *
 * \pre The fetch parameters in object->fetch must be populated
 *
 * \param object Object to refetch
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_refetch(llcache_object *object)
{
	ثابت حرف *urlenc = تہی;
	ساخت fetch_multipart_ڈیٹا *multipart = تہی;
	حرف **سرخیاں = تہی;
	صحیح سرخی_idx = 0;
	nserror res;

	اگر (object->fetch.post != تہی) {
		اگر (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->ڈیٹا.urlenc;
		} ورنہ {
			multipart = object->fetch.post->ڈیٹا.multipart;
		}
	}

	/* Generate headers */
	سرخیاں = حافظہ_دو(4 * حجم(حرف *));
	اگر (سرخیاں == تہی) {
		واپس NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	اگر (object->cache.etag != تہی) {
		ثابت size_t len = SLEN("If-None-Match: ") +
				لڑی_لمبائی(object->cache.etag) + 1;

		سرخیاں[سرخی_idx] = حافظہ_دو(len);
		اگر (سرخیاں[سرخی_idx] == تہی) {
			آزاد(سرخیاں);
			واپس NSERROR_NOMEM;
		}

		snprintf(سرخیاں[سرخی_idx], len, "If-None-Match: %s",
				object->cache.etag);

		سرخی_idx++;
	}

	/* cache-control header for modification time */
	اگر (object->cache.آخری_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ثابت size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		سرخیاں[سرخی_idx] = حافظہ_دو(len);
		اگر (سرخیاں[سرخی_idx] == تہی) {
			جب (--سرخی_idx >= 0)
				آزاد(سرخیاں[سرخی_idx]);
			آزاد(سرخیاں);
			واپس NSERROR_NOMEM;
		}

		snprintf(سرخیاں[سرخی_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.آخری_modified));

		سرخی_idx++;
	}

	/* Referer header */
	اگر (object->fetch.referer != تہی) {
		اگر (get_referer_سرخی(object->url,
				       object->fetch.referer,
				       &سرخیاں[سرخی_idx]) == NSERROR_OK) {
			سرخی_idx++;
		}
	}
	سرخیاں[سرخی_idx] = تہی;

	/* Reset cache control data */
	llcache_invalidate_cache_control_ڈیٹا(object);
	object->cache.req_وقت = time(تہی);
	object->cache.fin_وقت = object->cache.req_وقت;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_آغاز(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_صفحات,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (ثابت حرف **)سرخیاں,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	جب (--سرخی_idx >= 0) {
		آزاد(سرخیاں[سرخی_idx]);
	}
	آزاد(سرخیاں);

	واپس res;
}

/**
 * Kick-off a fetch for an object
 *
 * \pre object::url must contain the URL to fetch
 * \pre If there is a freshness validation candidate,
 *	object::candidate and object::cache must be filled in
 * \pre There must not be a fetch in progress for \a object
 *
 * \param object	  Object to fetch
 * \param flags		  Fetch flags
 * \param referer	  Referring URL, or NULL for none
 * \param post		  POST data, or NULL for GET
 * \param redirect_count  Number of redirects followed so far
 * \param hsts_in_use     Whether HSTS applies to this fetch
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ثابت llcache_post_ڈیٹا *post,
		uint32_t redirect_گنتی, bool hsts_اندر_use)
{
	nserror error;
	nsurl *referer_clone = تہی;
	llcache_post_ڈیٹا *post_clone = تہی;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	اگر (post != تہی) {
		error = llcache_post_ڈیٹا_clone(post, &post_clone);
		اگر (error != NSERROR_OK)
			واپس error;
	}

	اگر (referer != تہی)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_گنتی = redirect_گنتی;
	object->fetch.retries_بقایا = llcache->fetch_attempts;
	object->fetch.hsts_اندر_use = hsts_اندر_use;

	واپس llcache_object_refetch(object);
}

/**
 * Destroy a low-level cache object
 *
 * \pre Object is detached from cache list
 * \pre Object has no users
 * \pre Object is not a candidate (i.e. object::candidate_count == 0)
 *
 * \param object  Object to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	اگر (object->ماخذ_ڈیٹا != تہی) {
		اگر (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} ورنہ {
			آزاد(object->ماخذ_ڈیٹا);
		}
	}

	nsurl_unref(object->url);

	اگر (object->fetch.fetch != تہی) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = تہی;
	}

	اگر (object->fetch.referer != تہی)
		nsurl_unref(object->fetch.referer);

	اگر (object->fetch.post != تہی) {
		اگر (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			آزاد(object->fetch.post->ڈیٹا.urlenc);
		} ورنہ {
			fetch_multipart_ڈیٹا_destroy(
					object->fetch.post->ڈیٹا.multipart);
		}

		آزاد(object->fetch.post);
	}

	آزاد(object->cache.etag);

	برائے (i = 0; i < object->عدد_سرخیاں; i++) {
		آزاد(object->سرخیاں[i].name);
		آزاد(object->سرخیاں[i].قدر);
	}
	آزاد(object->سرخیاں);

	آزاد(object);

	واپس NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
ساکن nserror llcache_object_جوڑو_مقصد_list(llcache_object *object,
		llcache_object **list)
{
	object->پچھلا = تہی;
	object->اگلا = *list;

	اگر (*list != تہی)
		(*list)->پچھلا = object;
	*list = object;

	واپس NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
ساکن صحیح
llcache_object_rfc2616_بقایا_lifetime(ثابت llcache_cache_control *cd)
{
	صحیح current_age, freshness_lifetime;
	time_t now = time(تہی);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_وقت - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_وقت - cd->req_وقت + now - cd->res_وقت;

	/* Determine freshness lifetime of this object */
	اگر (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} ورنہ اگر (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} ورنہ اگر (cd->آخری_modified != 0) {
		freshness_lifetime = (now - cd->آخری_modified) / 10;
	} ورنہ {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	اگر ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		واپس freshness_lifetime - current_age;
	}

	واپس 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
ساکن bool llcache_object_is_fresh(ثابت llcache_object *object)
{
	صحیح بقایا_lifetime;
	ثابت llcache_cache_control *cd = &object->cache;

	بقایا_lifetime = llcache_object_rfc2616_بقایا_lifetime(cd);

	NSLOG(llcache, DEBUG, "%p: (%d > 0 || %d != %d)", object,
	     بقایا_lifetime,
	     object->fetch.state, LLCACHE_FETCH_COMPLETE);

	/* The object is fresh if:
	 * - it was not forbidden from being returned from the cache
	 *   unvalidated.
	 *
	 * - it has remaining lifetime or still being fetched.
	 */
	واپس ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
		((بقایا_lifetime > 0) ||
		 (object->fetch.state != LLCACHE_FETCH_COMPLETE)));
}

/**
 * Clone an object's cache data
 *
 * \post If \a deep is false, then any pointers in \a source will be set to NULL
 *
 * \param source       Source object containing cache data to clone
 * \param destination  Destination object to clone cache data into
 * \param deep	       Whether to deep-copy the data or not
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_clone_cache_ڈیٹا(llcache_object *ماخذ,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	اگر (ماخذ->cache.etag != تہی) {
		حرف *etag = ماخذ->cache.etag;

		اگر (deep) {
			/* Copy the etag */
			etag = strdup(ماخذ->cache.etag);
			اگر (etag == تہی)
				واپس NSERROR_NOMEM;
		} ورنہ {
			/* Destination takes ownership */
			ماخذ->cache.etag = تہی;
		}

		اگر (destination->cache.etag != تہی)
			آزاد(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_وقت = ماخذ->cache.req_وقت;
	destination->cache.res_وقت = ماخذ->cache.res_وقت;
	destination->cache.fin_وقت = ماخذ->cache.fin_وقت;

	اگر (ماخذ->cache.date != 0)
		destination->cache.date = ماخذ->cache.date;

	اگر (ماخذ->cache.expires != 0)
		destination->cache.expires = ماخذ->cache.expires;

	اگر (ماخذ->cache.age != INVALID_AGE)
		destination->cache.age = ماخذ->cache.age;

	اگر (ماخذ->cache.max_age != INVALID_AGE)
		destination->cache.max_age = ماخذ->cache.max_age;

	اگر (ماخذ->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = ماخذ->cache.no_cache;

	اگر (ماخذ->cache.آخری_modified != 0)
		destination->cache.آخری_modified = ماخذ->cache.آخری_modified;

	واپس NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
ساکن nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	اگر (object == *list)
		*list = object->اگلا;
	ورنہ
		object->پچھلا->اگلا = object->اگلا;

	اگر (object->اگلا != تہی)
		object->اگلا->پچھلا = object->پچھلا;

	واپس NSERROR_OK;
}

/**
 * Retrieve source data for an object from persistent store if necessary.
 *
 * If an object's source data has been placed in the persistent store
 * and there is no in-memory copy, then attempt to retrieve the source
 * data.
 *
 * \param object the object to operate on.
 * \return appropriate error code.
 */
ساکن nserror llcache_retrieve_persisted_ڈیٹا(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	اگر ((object->ماخذ_ڈیٹا != تہی) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		واپس NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	واپس guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->ماخذ_ڈیٹا,
				    &object->ماخذ_len);
}

/**
 * Generate a serialised version of an object's metadata
 *
 * The metadata includes object headers.
 *
 * \param object The cache object to serialise the metadata of.
 * \param data_out Where the serialised buffer will be placed.
 * \param datasize_out The size of the serialised data.
 * \return NSERROR_OK on success with \a data_out and \a datasize_out
 *         updated, NSERROR_NOMEM on memory exhaustion or
 *         NSERROR_INVALID if there was an error serialising the
 *         stream.
 */
ساکن nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **ڈیٹا_برآمد,
			   size_t *datasize_برآمد)
{
	size_t allocsize;
	صحیح datasize;
	uint8_t *ڈیٹا;
	حرف *op;
	غیرعلامتی صحیح hloop;
	صحیح use;
	size_t cert_chain_گہرائی;

	اگر (object->chain != تہی) {
		cert_chain_گہرائی = object->chain->گہرائی;
	} ورنہ {
		cert_chain_گہرائی = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	برائے (hloop = 0 ; hloop < object->عدد_سرخیاں ; hloop++) {
		allocsize += لڑی_لمبائی(object->سرخیاں[hloop].name) + 1;
		allocsize += لڑی_لمبائی(object->سرخیاں[hloop].قدر) + 1;
	}

	allocsize += nsurl_لمبائی(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	برائے (hloop = 0; hloop < cert_chain_گہرائی; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_لمبائی + 2) / 3);
	}

	ڈیٹا = حافظہ_دو(allocsize);
	اگر (ڈیٹا == تہی) {
		واپس NSERROR_NOMEM;
	}

	op = (حرف *)ڈیٹا;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	اگر (use < 0) {
		جاؤ operror;
	}
	use++; /* does not count the null */
	اگر (use > datasize) {
		جاؤ حد_سے_تجاوز;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->ماخذ_len);
	اگر (use < 0) {
		جاؤ operror;
	}
	use++; /* does not count the null */
	اگر (use > datasize)
		جاؤ حد_سے_تجاوز;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_وقت);
	اگر (use == 0)
		جاؤ حد_سے_تجاوز;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_وقت);
	اگر (use == 0)
		جاؤ حد_سے_تجاوز;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_وقت);
	اگر (use == 0)
		جاؤ حد_سے_تجاوز;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->عدد_سرخیاں);
	اگر (use < 0) {
		جاؤ operror;
	}
	use++; /* does not count the null */
	اگر (use > datasize)
		جاؤ حد_سے_تجاوز;
	op += use;
	datasize -= use;

	/*سرخیاں*/
	برائے (hloop = 0 ; hloop < object->عدد_سرخیاں ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->سرخیاں[hloop].name,
			       object->سرخیاں[hloop].قدر);
		اگر (use < 0) {
			جاؤ operror;
		}
		use++; /* does not count the null */
		اگر (use > datasize)
			جاؤ حد_سے_تجاوز;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_گہرائی);
	اگر (use < 0) {
		جاؤ operror;
	}
	use++; /* does not count the null */
	اگر (use > datasize)
		جاؤ حد_سے_تجاوز;
	op += use;
	datasize -= use;

	/* SSL certificates */
	برائے (hloop = 0; hloop < cert_chain_گہرائی; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (صحیح)(object->chain->certs[hloop].err));
		اگر (use < 0) {
			جاؤ operror;
		}
		use++; /* does not count the null */
		اگر (use > datasize)
			جاؤ حد_سے_تجاوز;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		اگر (object->chain->certs[hloop].der != تہی) {
			size_t output_لمبائی = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_لمبائی,
				(uint8_t *)op,
				&output_لمبائی);
			اگر (res != NSUERROR_OK) {
				جاؤ operror;
			}
			use = output_لمبائی;
		} ورنہ {
			use = 0;
		}
		use++; /* allow for null */
		اگر (use > datasize)
			جاؤ حد_سے_تجاوز;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*ڈیٹا_برآمد = ڈیٹا;
	*datasize_برآمد = allocsize - datasize;

	واپس NSERROR_OK;

حد_سے_تجاوز:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	آزاد(ڈیٹا);
	واپس NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	آزاد(ڈیٹا);
	واپس NSERROR_INVALID;
}

/**
 * Deserialisation of an object's metadata.
 *
 * Attempt to retrieve and deserialise the metadata for an object from
 * the backing store.
 *
 * This must only update object if it is successful otherwise difficult
 * to debug crashes happen later by using bad leftover object state.
 *
 * \param object The object to retrieve the metadata for.
 * \return NSERROR_OK if the metatdata was retrieved and deserialised
 *         or error code if URL is not in persistent storage or in
 *         event of deserialisation error.
 */
ساکن nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = تہی;
	size_t metadatalen = 0;
	size_t بقایا = 0;
	nsurl *metadataurl;
	غیرعلامتی صحیح سطر;
	حرف *ln;
	صحیح lnsize;

	size_t ماخذ_لمبائی;
	time_t request_وقت;
	time_t response_وقت;
	time_t completion_وقت;
	size_t عدد_سرخیاں;
	size_t hloop;
	size_t ssl_cert_گنتی = 0;
	ساخت cert_chain *chain = تہی;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	NSLOG(llcache, INFO, "Processing retrieved data");

	/* metadata is stored as a sequence of NULL terminated strings
	 * which we call 'line's here.
	 */

	/* We track remaining data because as we extend this data structure
	 * we need to know if we should continue to parse
	 */
	بقایا = metadatalen;

	/* metadata line 1 is the url the metadata referrs to */
	سطر = 1;
	ln = (حرف *)metadata;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	اگر (lnsize < 7) {
		res = NSERROR_INVALID;
		جاؤ format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	اگر (res != NSERROR_OK)
		جاؤ format_error;

	اگر (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		واپس NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	سطر = 2;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	اگر ((lnsize < 1) || (لڑی_سے_پڑھو(ln, "%" PRIsizet, &ماخذ_لمبائی) != 1)) {
		res = NSERROR_INVALID;
		جاؤ format_error;
	}


	/* metadata line 3 is the time of request */
	سطر = 3;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_وقت);
	اگر (res != NSERROR_OK)
		جاؤ format_error;


	/* metadata line 4 is the time of response */
	سطر = 4;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_وقت);
	اگر (res != NSERROR_OK)
		جاؤ format_error;


	/* metadata line 5 is the time of request completion */
	سطر = 5;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_وقت);
	اگر (res != NSERROR_OK)
		جاؤ format_error;


	/* metadata line 6 is the number of headers */
	سطر = 6;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	اگر ((lnsize < 1) || (لڑی_سے_پڑھو(ln, "%" PRIsizet, &عدد_سرخیاں) != 1)) {
		res = NSERROR_INVALID;
		جاؤ format_error;
	}

	/* read headers */
	برائے (hloop = 0 ; hloop < عدد_سرخیاں; hloop++) {
		سطر++;
		ln += lnsize + 1;
		lnsize = لڑی_لمبائی(ln);
		بقایا -= lnsize + 1;

		res = llcache_fetch_process_سرخی(object,
						   (uint8_t *)ln,
						   lnsize);
		اگر (res != NSERROR_OK)
			جاؤ format_error;
	}

	اگر (بقایا == 0) {
		جاؤ skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	سطر++;
	ln += lnsize + 1;
	lnsize = لڑی_لمبائی(ln);
	بقایا -= lnsize + 1;

	اگر ((lnsize < 1) || (لڑی_سے_پڑھو(ln, "%" PRIsizet, &ssl_cert_گنتی) != 1)) {
		res = NSERROR_INVALID;
		جاؤ format_error;
	}

	اگر (ssl_cert_گنتی == 0) {
		جاؤ skip_ssl_certificates;
	}

	اگر (ssl_cert_گنتی > MAX_CERT_گہرائی) {
		res = NSERROR_INVALID;
		جاؤ format_error;
	}

	res = cert_chain_alloc(ssl_cert_گنتی, &chain);
	اگر (res != NSERROR_OK) {
		جاؤ format_error;
	}

	برائے (hloop = 0; hloop < ssl_cert_گنتی; hloop++) {
		صحیح errcode;
		nsuerror nsures;

		/* Certificate error code */
		سطر++;
		ln += lnsize + 1;
		lnsize = لڑی_لمبائی(ln);
		بقایا -= lnsize + 1;
		اگر ((lnsize < 1) || (لڑی_سے_پڑھو(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			جاؤ format_error;
		}
		اگر (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_معلوم) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} ورنہ {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		سطر++;
		ln += lnsize + 1;
		lnsize = لڑی_لمبائی(ln);
		بقایا -= lnsize + 1;
		اگر (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ثابت uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_لمبائی);
			اگر (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				جاؤ format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->ماخذ_len = ماخذ_لمبائی;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->ماخذ_alloc = metadatalen;

	object->cache.req_وقت = request_وقت;
	object->cache.res_وقت = response_وقت;
	object->cache.fin_وقت = completion_وقت;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	واپس NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      سطر, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	واپس res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
ساکن اندرونی bool llcache__scheme_is_persistable(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool مطابقت;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_http,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	واپس persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
ساکن اندرونی bool llcache__scheme_is_cachable(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool مطابقت;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	اگر ((lwc_string_isequal(scheme, corestring_lwc_http,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ڈیٹا,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&مطابقت) == lwc_error_ok &&
			(مطابقت == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	واپس cachable;
}

/**
 * Attempt to retrieve an object from persistent storage.
 *
 * \param object The object to populate from persistent store.
 * \param flags Fetch flags.
 * \param referer The referring url.
 * \param post Post data for fetch.
 * \param redirect_count how many times this fetch has been redirected.
 * \return NSERROR_OK if the object was successfully retrieved from the
 *         cache else appropriate error code.
 */
ساکن nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ثابت llcache_post_ڈیٹا *post,
				uint32_t redirect_گنتی)
{
	nserror error;
	nsurl *referer_clone = تہی;
	llcache_post_ڈیٹا *post_clone = تہی;

	اگر (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		واپس NSERROR_NOT_ملا;
	}

	object->cache.req_وقت = time(تہی);
	object->cache.fin_وقت = object->cache.req_وقت;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	اگر (error != NSERROR_OK) {
		واپس error;
	}

	/* entry came out of cache - need to setup object state */
	اگر (post != تہی) {
		error = llcache_post_ڈیٹا_clone(post, &post_clone);
		اگر (error != NSERROR_OK)
			واپس error;
	}

	اگر (referer != تہی) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_گنتی = redirect_گنتی;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = تہی;

	واپس NSERROR_OK;
}

/**
 * Retrieve a potentially cached object
 *
 * \param url		  URL of object to retrieve
 * \param flags		  Fetch flags
 * \param referer	  Referring URL, or NULL if none
 * \param post		  POST data, or NULL for a GET request
 * \param redirect_count  Number of redirects followed so far
 * \param hsts_in_use     Whether HSTS applies to this fetch
 * \param result	  Pointer to location to receive retrieved object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ثابت llcache_post_ڈیٹا *post,
				   uint32_t redirect_گنتی,
				   bool hsts_اندر_use,
				   llcache_object **نتیجہ)
{
	nserror error;
	llcache_object *obj, *newest = تہی;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==تہی?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	برائے (obj = llcache->cached_objects; obj != تہی; obj = obj->اگلا) {

		اگر ((newest == تہی ||
		     obj->cache.req_وقت > newest->cache.req_وقت) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	اگر (newest == تہی) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		اگر (error != NSERROR_OK)
			واپس error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_گنتی);
		اگر (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_جوڑو_مقصد_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	اگر ((newest != تہی) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ڈیٹا(newest);
		اگر (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*نتیجہ = newest;

			واپس NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		اگر (error != NSERROR_OK) {
			واپس error;
		}
	} ورنہ اگر (newest != تہی) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ڈیٹا(newest);
		اگر (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			اگر (error != NSERROR_OK)
				واپس error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_ڈیٹا(newest, obj, true);
			اگر (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				واپس error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_گنتی++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_گنتی, hsts_اندر_use);
			اگر (error != NSERROR_OK) {
				newest->candidate_گنتی--;
				llcache_object_destroy(obj);
				واپس error;
			}

			/* Add new object to cache */
			llcache_object_جوڑو_مقصد_list(obj, &llcache->cached_objects);

			*نتیجہ = obj;

			واپس NSERROR_OK;
		}

		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-retch
		 */
		llcache_object_remove_from_list(newest,
						&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		اگر (error != NSERROR_OK) {
			واپس error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_گنتی, hsts_اندر_use);
	اگر (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		واپس error;
	}

	/* Add new object to cache */
	llcache_object_جوڑو_مقصد_list(obj, &llcache->cached_objects);

	*نتیجہ = obj;

	واپس NSERROR_OK;
}

/**
 * Retrieve an object from the cache, fetching it if necessary.
 *
 * \param url		  URL of object to retrieve
 * \param flags		  Fetch flags
 * \param referer	  Referring URL, or NULL if none
 * \param post		  POST data, or NULL for a GET request
 * \param redirect_count  Number of redirects followed so far
 * \param hsts_in_use     Whether HSTS applies to this fetch
 * \param result	  Pointer to location to receive retrieved object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ثابت llcache_post_ڈیٹا *post,
			uint32_t redirect_گنتی,
			bool hsts_اندر_use,
			llcache_object **نتیجہ)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==تہی?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	اگر (error != NSERROR_OK)
		واپس error;

	/* determine if content is cachable */
	اگر ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} ورنہ اگر (post != تہی) {
		/* POST requests are never cached */
		uncachable = true;
	} ورنہ {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	اگر (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		اگر (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			واپس error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_گنتی, hsts_اندر_use);
		اگر (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			واپس error;
		}

		/* Add new object to uncached list */
		llcache_object_جوڑو_مقصد_list(obj, &llcache->uncached_objects);
	} ورنہ {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_گنتی,
				hsts_اندر_use, &obj);
		اگر (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			واپس error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*نتیجہ = obj;

	nsurl_unref(defragmented_url);

	واپس NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
ساکن nserror llcache_object_جوڑو_user(llcache_object *object,
		llcache_object_user *user)
{
	أكد(user->اگلا == تہی);
	أكد(user->پچھلا == تہی);
	أكد(user->handle != تہی);

	user->handle->object = object;

	user->پچھلا = تہی;
	user->اگلا = object->users;

	اگر (object->users != تہی)
		object->users->پچھلا = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	واپس NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_hsts_transform_url(nsurl *url, nsurl **نتیجہ,
		bool *hsts_اندر_use)
{
	lwc_string *scheme = تہی;
	bool مطابقت;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	اگر (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&مطابقت) != lwc_error_ok || مطابقت == false) {
		/* Non-HTTP fetch: no transform required */
		اگر (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&مطابقت) == lwc_error_ok && مطابقت) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_اندر_use = urldb_get_hsts_enabled(url);
		} ورنہ {
			/* Anything else: no HSTS */
			*hsts_اندر_use = false;
		}
		lwc_string_unref(scheme);
		*نتیجہ = nsurl_ref(url);
		واپس error;
	}
	lwc_string_unref(scheme);

	اگر (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				نتیجہ);
		*hsts_اندر_use = (error == NSERROR_OK);
	} ورنہ {
		*نتیجہ = nsurl_ref(url);
		*hsts_اندر_use = false;
	}

	واپس error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = تہی;
	bool مطابقت = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	اگر (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&مطابقت) != lwc_error_ok || مطابقت == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		واپس NSERROR_OK;
	}
	lwc_string_unref(scheme);

	اگر (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		واپس NSERROR_OK;
	}

	برائے (i = 0; i < object->عدد_سرخیاں; i++) {
		اگر (strcasecmp("Strict-Transport-Security",
				object->سرخیاں[i].name) == 0) {
			urldb_سیٹ_hsts_policy(object->url,
					object->سرخیاں[i].قدر);
			/* Only process the first one we find */
			توڑو;
		}
	}

	واپس NSERROR_OK;
}

/**
 * determine if a redirect is safe
 *
 * Reject attempts to redirect from unvalidated to validated schemes
 * A "validated" scheme is one over which we have some guarantee that
 * the source is trustworthy.
 *
 * \param src_url source URL of redirect
 * \param dst_url destination URL of redirect
 * \return NSERROR_OK if safe else error code
 */
ساکن nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool مطابقت;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	اگر ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&مطابقت) == lwc_error_ok && مطابقت == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&مطابقت) == lwc_error_ok && مطابقت == false)) {
		/* file, about and resource are not valid redirect targets */
		اگر ((lwc_string_isequal(scheme, corestring_lwc_file,
				&مطابقت) == lwc_error_ok && مطابقت == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&مطابقت) == lwc_error_ok && مطابقت == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&مطابقت) == lwc_error_ok && مطابقت == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			واپس NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	اگر (fetch_can_fetch(dst_url) == false) {
		واپس NSERROR_UNSAFE_REDIRECT;
	}

	واپس NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_fetch_redirect(llcache_object *object,
		ثابت حرف *ہدف, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *اگلا;
	ثابت llcache_post_ڈیٹا *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_اندر_use;
	http_response_رمز http_رمز;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_رمز = fetch_http_رمز(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = تہی;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(خالی) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	اگر (object->fetch.redirect_گنتی > nsoption_uint(fetch_redirect_حد_بندی)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		واپس NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, ہدف, &url);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_اندر_use);
	nsurl_unref(url);
	اگر (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		واپس res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	اگر (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		واپس res;
	}

	/* alter behaviour based on specific redirect code */
	انتخاب (http_رمز) {
	حالت HTTP_RESPONSE_MOVED_PERMANENTLY:
	حالت HTTP_RESPONSE_ملا:
	حالت HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = تہی;
		توڑو;

	حالت HTTP_RESPONSE_TEMPORARY_REDIRECT:
	حالت HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		توڑو;

	طےشدہ:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_رمز);
		nsurl_unref(hsts_url);
		واپس NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.ڈیٹا.redirect.from = object->url;
	event.ڈیٹا.redirect.مقصد = hsts_url;

	res = llcache_send_event_مقصد_users(object, &event);
	اگر (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		واپس res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_گنتی + 1,
					hsts_اندر_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	اگر (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		برائے (user = object->users; user != تہی; user = اگلا) {
			اگلا = user->اگلا;

			llcache_object_remove_user(object, user);
			llcache_object_جوڑو_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	واپس res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
ساکن nserror llcache_object_cache_update(llcache_object *object)
{
	اگر (object->cache.date == 0)
		object->cache.date = time(تہی);

	واپس NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
ساکن nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	اگر (object->candidate != تہی) {
		llcache_object_user *user, *اگلا;

		/* Move user(s) to candidate content */
		برائے (user = object->users; user != تہی; user = اگلا) {
			اگلا = user->اگلا;

			llcache_object_remove_user(object, user);
			llcache_object_جوڑو_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_گنتی--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_ڈیٹا(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		اگر (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = تہی;
	} ورنہ {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = تہی;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(خالی) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	واپس NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساکن nserror
llcache_fetch_process_ڈیٹا(llcache_object *object,
			   ثابت uint8_t *ڈیٹا,
			   size_t len)
{
	اگر (object->fetch.state != LLCACHE_FETCH_ڈیٹا) {
		/**
		 * \note
		 * On entry into this state, check if we need to
		 * invalidate the cache control data. We are guaranteed
		 * to have received all response headers.
		 *
		 * There are two cases in which we want to suppress
		 * cacheing of an object:
		 *
		 * 1) The HTTP response code is not 200 or 203
		 * 2) The request URI had a query string and the
		 *    response headers did not provide an explicit
		 *    object expiration time.
		 */
		طویل http_رمز = fetch_http_رمز(object->fetch.fetch);

		اگر ((http_رمز != 200 && http_رمز != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ڈیٹا(object);
		}

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		object->fetch.state = LLCACHE_FETCH_ڈیٹا;
	}

	/* Resize source buffer if it's too small */
	اگر (object->ماخذ_len + len >= object->ماخذ_alloc) {
		ثابت size_t new_len = object->ماخذ_len + len + 64 * 1024;
		uint8_t *عارضی = realloc(object->ماخذ_ڈیٹا, new_len);
		اگر (عارضی == تہی)
			واپس NSERROR_NOMEM;

		object->ماخذ_ڈیٹا = عارضی;
		object->ماخذ_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	حافظہ_نقل(object->ماخذ_ڈیٹا + object->ماخذ_len, ڈیٹا, len);
	object->ماخذ_len += len;

	واپس NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساکن nserror llcache_fetch_auth(llcache_object *object, ثابت حرف *realm)
{
	ثابت حرف *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = تہی;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	/* Destroy headers */
	llcache_destroy_سرخیاں(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	اگر (realm == تہی)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	اگر (auth == تہی || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ڈیٹا.error.رمز = NSERROR_BAD_AUTH;
		event.ڈیٹا.error.پیغام = realm;

		error = llcache_send_event_مقصد_users(object, &event);
	} ورنہ {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	واپس error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = تہی;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	اگر (object->fetch.hsts_اندر_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ڈیٹا.error.رمز = NSERROR_BAD_CERTS;
		event.ڈیٹا.error.پیغام = messages_get("FetchFailed");

		error = llcache_send_event_مقصد_users(object, &event);
	} ورنہ {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ڈیٹا.error.رمز = NSERROR_UNKNOWN;
		event.ڈیٹا.error.پیغام = messages_get("FetchFailed");

		error = llcache_send_event_مقصد_users(object, &event);
	}

	واپس error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = تہی;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	اگر (object->fetch.hsts_اندر_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ڈیٹا.error.رمز = NSERROR_UNKNOWN;
		event.ڈیٹا.error.پیغام = messages_get("FetchFailed");

		error = llcache_send_event_مقصد_users(object, &event);
	} ورنہ {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	واپس error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
ساکن nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	اگر (object->fetch.retries_بقایا > 1) {
		object->fetch.retries_بقایا--;
		واپس llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = تہی;

	/* Release candidate, if any */
	اگر (object->candidate != تہی) {
		object->candidate->candidate_گنتی--;
		object->candidate = تہی;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_ڈیٹا(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.ڈیٹا.error.رمز = NSERROR_TIMEOUT;
	event.ڈیٹا.error.پیغام = تہی;

	واپس llcache_send_event_مقصد_users(object, &event);
}


/**
 * Construct a sorted list of objects available for writeout operation.
 *
 * The list contains fresh cacheable objects held in RAM with no
 * pending fetches. Any objects with a remaining lifetime less than
 * the configured minimum lifetime are simply not considered, they will
 * become stale before pushing to backing store is worth the cost.
 *
 * \todo calculate useful cost metrics to improve sorting.
 *
 * \param[out] lst_out list of candidate objects.
 * \param[out] lst_len_out Number of candidate objects in result.
 * \return NSERROR_OK with \a lst_out and \a lst_len_out updated or
 *         error code.
 */
ساکن nserror
build_candidate_list(ساخت llcache_object ***lst_برآمد, صحیح *lst_len_برآمد)
{
	llcache_object *object, *اگلا;
	ساخت llcache_object **lst;
	صحیح lst_len = 0;
	صحیح بقایا_lifetime;

#تعریف_کرو MAX_PERSIST_فی_تسلسل 128

	lst = صفر_حافظہ_دو(MAX_PERSIST_فی_تسلسل, حجم(ساخت llcache_object *));
	اگر (lst == تہی) {
		واپس NSERROR_NOMEM;
	}

	برائے (object = llcache->cached_objects; object != تہی; object = اگلا) {
		اگلا = object->اگلا;

		/* Only consider http(s) for the disc cache. */
		اگر (!llcache__scheme_is_persistable(object->url)) {
			جاری;
		}

		بقایا_lifetime = llcache_object_rfc2616_بقایا_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		اگر ((object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (بقایا_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			اگر (lst_len == MAX_PERSIST_فی_تسلسل)
				توڑو;
		}
	}

	اگر (lst_len == 0) {
		آزاد(lst);
		واپس NSERROR_NOT_ملا;
	}

	/** \todo sort list here */

	*lst_len_برآمد = lst_len;
	*lst_برآمد = lst;

#تعریف_منسوخ MAX_PERSIST_فی_تسلسل

	واپس NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساکن nserror
write_backing_store(ساخت llcache_object *object,
		    size_t *written_برآمد,
		    غیرعلامتی طویل *elapsed)
{
	nserror ret;
	uint8_t *metadata;
	size_t metadatasize;
	uint64_t startms = 0;
	uint64_t endms = 1000;

	nsu_getmonotonic_ms(&startms);

	/* put object data in backing store */
	ret = guit->llcache->store(object->url,
				   BACKING_STORE_NONE,
				   object->ماخذ_ڈیٹا,
				   object->ماخذ_len);
	اگر (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		واپس ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	اگر (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		واپس ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	اگر (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		واپس ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_برآمد = object->ماخذ_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	اگر (*elapsed == 0) {
		*elapsed = 1;
	}

	واپس NSERROR_OK;
}

/**
 * Check for overall write performance.
 *
 * If the overall write bandwidth has fallen below a useful level for
 * the backing store to be effective disable it.
 *
 * It is important to ensure a useful amount of data has been written
 * before calculating bandwidths otherwise tiny files taking a
 * disproportionately long time to write might trigger this erroneously.
 *
 * \param p The context pointer passed to the callback.
 */
ساکن خالی llcache_persist_slowcheck(خالی *p)
{
	uint64_t کل_bandwidth; /* total bandwidth */

	اگر (llcache->کل_written > (2 * llcache->minimum_bandwidth)) {

		کل_bandwidth = (llcache->کل_written * 1000) / llcache->کل_elapsed;

		اگر (کل_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      کل_bandwidth,
			      llcache->minimum_bandwidth);
			guit->llcache->finalise();
		}
	}
}

/**
 * Possibly write objects data to backing store.
 *
 * \param p The context pointer passed to the callback.
 */
ساکن خالی llcache_persist(خالی *p)
{
	nserror ret;
	ساخت llcache_object **lst; /* candidate object list */
	صحیح lst_گنتی; /* number of candidates in list */
	صحیح idx; /* current candidate object index in list */
	صحیح اگلا = -1; /* when the next run should be scheduled for */

	غیرعلامتی طویل write_حد_بندی; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	غیرعلامتی طویل elapsed; /* how long writing an object took */

	size_t کل_written = 0; /* total bytes written in this run */
	غیرعلامتی طویل کل_elapsed = 1; /* total ms used to write bytes */
	غیرعلامتی طویل کل_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_گنتی);
	اگر (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		واپس;
	}

	write_حد_بندی = (llcache->زیادہ_سے_زیادہ_bandwidth * llcache->وقت_وقتی_حصہ) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	برائے (idx = 0; idx < lst_گنتی; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		اگر (ret != NSERROR_OK) {
			جاری;
		}

		/* successfully wrote object to backing store */
		کل_written += written;
		کل_elapsed += elapsed;
		کل_bandwidth = (کل_written * 1000) / کل_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		اگر (کل_elapsed > llcache->وقت_وقتی_حصہ) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			اگر (کل_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->جدول_وقت(
						llcache->وقت_وقتی_حصہ * 100,
						llcache_persist_slowcheck,
						تہی);
				توڑو;
			} ورنہ {
				اگر (کل_bandwidth > llcache->زیادہ_سے_زیادہ_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					اگلا = ((کل_written * llcache->وقت_وقتی_حصہ) / write_حد_بندی) - کل_elapsed;
				} ورنہ {
					اگلا = llcache->وقت_وقتی_حصہ;
				}
				توڑو;
			}
		} ورنہ اگر (کل_written > write_حد_بندی) {
			/* The bandwidth limit has been reached. */

			اگر (کل_bandwidth > llcache->زیادہ_سے_زیادہ_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				اگلا = ((کل_written * llcache->وقت_وقتی_حصہ) / write_حد_بندی) - کل_elapsed;
			} ورنہ {
				اگلا = llcache->وقت_وقتی_حصہ - کل_elapsed;
			}
			توڑو;
		}

	}
	آزاد(lst);

	/* Completed list without running out of allowed bytes or time */
	اگر (idx == lst_گنتی) {
		/* only reschedule if writing is making any progress at all */
		اگر (کل_written > 0) {
			اگلا = llcache->وقت_وقتی_حصہ - کل_elapsed;
		} ورنہ {
			اگلا = -1;
		}
	}

	llcache->کل_written += کل_written;
	llcache->کل_elapsed += کل_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      کل_written, کل_elapsed, کل_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", اگلا);
	guit->misc->جدول_وقت(اگلا, llcache_persist, تہی);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
ساکن خالی llcache_fetch_callback(ثابت fetch_پیغام *پیغام, خالی *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	اگر (llcache == تہی) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		أكد(false);
		/* In case assertions are off, return here */
		واپس;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", پیغام->type, object);

	انتخاب (پیغام->type) {
	حالت FETCH_سرخی:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_سرخیاں;

		error = llcache_fetch_process_سرخی(object,
				پیغام->ڈیٹا.سرخی_or_ڈیٹا.buf,
				پیغام->ڈیٹا.سرخی_or_ڈیٹا.len);
		توڑو;

	/* 3xx responses */
	حالت FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		error = llcache_fetch_redirect(object,
				پیغام->ڈیٹا.redirect, &object);
		اگر (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.ڈیٹا.error.رمز = error;
			event.ڈیٹا.error.پیغام = تہی;
			error = llcache_send_event_مقصد_users(object, &event);
		}
		توڑو;

	حالت FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		توڑو;

	/* Normal 2xx state machine */
	حالت FETCH_ڈیٹا:
		/* Received some data */
		error = llcache_fetch_process_ڈیٹا(object,
				پیغام->ڈیٹا.سرخی_or_ڈیٹا.buf,
				پیغام->ڈیٹا.سرخی_or_ڈیٹا.len);
		توڑو;

	حالت FETCH_ختم_شدہ:
		/* Finished fetching */
	{
		uint8_t *عارضی;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = تہی;

		/* Shrink source buffer to required size */
		عارضی = realloc(object->ماخذ_ڈیٹا,
				object->ماخذ_len);
		/* If source_len is 0, then temp may be NULL */
		اگر (عارضی != تہی || object->ماخذ_len == 0) {
			object->ماخذ_ڈیٹا = عارضی;
			object->ماخذ_alloc = object->ماخذ_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_وقت = time(تہی);

		(خالی) llcache_hsts_update_policy(object);

		guit->misc->جدول_وقت(5000, llcache_persist, تہی);
	}
		توڑو;

	/* Out-of-band information */
	حالت FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		توڑو;

	حالت FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = تہی;

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ڈیٹا(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.ڈیٹا.error.رمز = NSERROR_UNKNOWN;
		event.ڈیٹا.error.پیغام = پیغام->ڈیٹا.error;

		error = llcache_send_event_مقصد_users(object, &event);

		توڑو;

	حالت FETCH_پیش_رفت:
		/* Progress update */
		event.type = LLCACHE_EVENT_پیش_رفت;
		event.ڈیٹا.پیش_رفت_پیغام = پیغام->ڈیٹا.پیش_رفت;

		error = llcache_send_event_مقصد_users(object, &event);

		توڑو;

	حالت FETCH_CERTS:
		/* Certificate information from the fetch */

		اگر (object->chain != تہی) {
			cert_chain_free(object->chain);
			object->chain = تہی;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(پیغام->ڈیٹا.chain, &object->chain);
		اگر (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.ڈیٹا.chain = پیغام->ڈیٹا.chain;

		error = llcache_send_event_مقصد_users(object, &event);

		توڑو;

	/* Events requiring action */
	حالت FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		error = llcache_fetch_auth(object, پیغام->ڈیٹا.auth.realm);
		توڑو;

	حالت FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		error = llcache_fetch_cert_error(object);
		توڑو;

	حالت FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		اگر (object->candidate != تہی) {
			object->candidate->candidate_گنتی--;
			object->candidate = تہی;
		}

		error = llcache_fetch_ssl_error(object);
		توڑو;
	}

	/* Deal with any errors reported by event handlers */
	اگر (error != NSERROR_OK) {
		اگر (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		اگر (object->fetch.fetch != تہی) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = تہی;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ڈیٹا(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_اوپر();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
ساکن llcache_object_user *llcache_object_ڈھونڈو_user(
		ثابت llcache_handle *handle)
{
	llcache_object_user *user;

	أكد(handle->object != تہی);

	برائے (user = handle->object->users; user != تہی; user = user->اگلا) {
		اگر (user->handle == handle)
			توڑو;
	}

	واپس user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
ساکن bool llcache_object_اندر_list(ثابت llcache_object *object,
		ثابت llcache_object *list)
{
	جب (list != تہی) {
		اگر (list == object)
			توڑو;

		list = list->اگلا;
	}

	واپس list != تہی;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *اگلا_user;
	llcache_event event;
	bool emitted_notify = false;

	/**
	 * State transitions and event emission for users.
	 * Rows: user state. Cols: object state.
	 *
	 * User - Obj	INIT	HEADERS		DATA	COMPLETE
	 * INIT		 -	   T		 T*	   T*
	 * HEADERS	 -	   -		 T	   T*
	 * DATA		 -	   -		 M	   T
	 * COMPLETE	 -	   -		 -	   -
	 *
	 * T => transition user to object state
	 * M => no transition required, but may need to emit event
	 *
	 * The transitions marked with an asterisk can be removed by moving
	 * the user context into the subsequent state and then reevaluating.
	 *
	 * Events are issued as follows:
	 *
	 * HAD_HEADERS: on transition from HEADERS -> DATA state
	 * HAD_DATA   : in DATA state, whenever there's new source data
	 * DONE	      : on transition from DATA -> COMPLETE state
	 */

	برائے (user = object->users; user != تہی; user = اگلا_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ثابت llcache_fetch_state objstate = object->fetch.state;

		/* Flag that this user is the current iteration target
		 * in case the client attempts to destroy it underneath us */
		user->iterator_ہدف = true;

		/* A note on the computation of next_user:
		 *
		 * Within this loop, we may make a number of calls to
		 * client code. Our contract with clients is that they
		 * can do whatever they like from within their callback
		 * handlers. This is so that we limit the pain of
		 * reentrancy to this module alone.
		 *
		 * One of the things a client can do from within its
		 * callback handler is to remove users from this object's
		 * user list. In the common case, the user they attempt
		 * to remove is the current iteration target, and we
		 * already protect against that causing problems here.
		 * However, no such protection exists if the client
		 * attempts to remove other users from this object's
		 * user list.
		 *
		 * Therefore, we cannot compute next_user up-front
		 * and expect it to remain valid across calls to
		 * client code (as the identity of the next user
		 * in the list may change underneath us). Instead,
		 * we must compute next_user at the point where we
		 * are about to cause another iteration of this loop
		 * (i.e. at the very end, and also at the points where
		 * continue is used)
		 */

		اگر (handle->state != objstate) {
			اگر (emitted_notify == false) {
				NSLOG(llcache, DEBUG,
				      "Notifying users of %p",
				      object);
				emitted_notify = true;
			}

			NSLOG(llcache, DEBUG,
			      "User %p state: %d Object state: %d",
			      user,
			      handle->state,
			      objstate);
		}

		/* User: INIT, Obj: HEADERS, DATA, COMPLETE => User->HEADERS */
		اگر (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_سرخیاں;

			/* Emit any certificate data we hold */
			اگر (object->chain != تہی) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.ڈیٹا.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} ورنہ {
				error = NSERROR_OK;
			}

			اگر (user->queued_for_delete) {
				اگلا_user = user->اگلا;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				اگر (error != NSERROR_OK)
					واپس error;

				جاری;
			} ورنہ اگر (error == NSERROR_NEED_ڈیٹا) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_سرخیاں;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ہدف = false;
				اگلا_user = user->اگلا;
				llcache_users_not_caught_اوپر();
				جاری;
			} ورنہ اگر (error != NSERROR_OK) {
				user->iterator_ہدف = false;
				واپس error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		اگر (handle->state == LLCACHE_FETCH_سرخیاں &&
				objstate > LLCACHE_FETCH_سرخیاں) {
			handle->state = LLCACHE_FETCH_ڈیٹا;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_سرخیاں;

			error = handle->cb(handle, &event, handle->pw);

			اگر (user->queued_for_delete) {
				اگلا_user = user->اگلا;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				اگر (error != NSERROR_OK)
					واپس error;

				جاری;
			} ورنہ اگر (error == NSERROR_NEED_ڈیٹا) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_سرخیاں;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ہدف = false;
				اگلا_user = user->اگلا;
				llcache_users_not_caught_اوپر();
				جاری;
			} ورنہ اگر (error != NSERROR_OK) {
				user->iterator_ہدف = false;
				واپس error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		اگر (handle->state == LLCACHE_FETCH_ڈیٹا &&
				objstate >= LLCACHE_FETCH_ڈیٹا &&
				object->ماخذ_len > handle->بائٹیں) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_ڈیٹا;
			event.ڈیٹا.ڈیٹا.buf =
					object->ماخذ_ڈیٹا + handle->بائٹیں;
			event.ڈیٹا.ڈیٹا.len =
					object->ماخذ_len - handle->بائٹیں;

			/* Update record of last byte emitted */
			اگر (object->fetch.flags &
					LLCACHE_RETRIEVE_دھارا_ڈیٹا) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->بائٹیں = object->ماخذ_len = 0;
			} ورنہ {
				orig_handle_read = handle->بائٹیں;
				handle->بائٹیں = object->ماخذ_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			اگر (user->queued_for_delete) {
				اگلا_user = user->اگلا;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				اگر (error != NSERROR_OK)
					واپس error;

				جاری;
			} ورنہ اگر (error == NSERROR_NEED_ڈیٹا) {
				/* User requested replay */
				handle->بائٹیں = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_ہدف = false;
				اگلا_user = user->اگلا;
				llcache_users_not_caught_اوپر();
				جاری;
			} ورنہ اگر (error != NSERROR_OK) {
				user->iterator_ہدف = false;
				واپس error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		اگر (handle->state == LLCACHE_FETCH_ڈیٹا &&
				objstate > LLCACHE_FETCH_ڈیٹا) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_مکمل;

			error = handle->cb(handle, &event, handle->pw);
			اگر (user->queued_for_delete) {
				اگلا_user = user->اگلا;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				اگر (error != NSERROR_OK)
					واپس error;

				جاری;
			} ورنہ اگر (error == NSERROR_NEED_ڈیٹا) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ڈیٹا;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ہدف = false;
				اگلا_user = user->اگلا;
				llcache_users_not_caught_اوپر();
				جاری;
			} ورنہ اگر (error != NSERROR_OK) {
				user->iterator_ہدف = false;
				واپس error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_ہدف = false;

		اگلا_user = user->اگلا;
	}

	واپس NSERROR_OK;
}

/**
 * Make a snapshot of the current state of an llcache_object.
 *
 * This has the side-effect of the new object being non-cacheable,
 * also not-fetching and not a candidate for any other object.
 *
 * Also note that this new object has no users and at least one
 * should be assigned to it before llcache_clean is entered or it
 * will be immediately cleaned up.
 *
 * \param object  The object to take a snapshot of
 * \param snapshot  Pointer to receive snapshot of \a object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	اگر (error != NSERROR_OK)
		واپس error;

	newobj->ماخذ_alloc = newobj->ماخذ_len = object->ماخذ_len;

	اگر (object->ماخذ_len > 0) {
		newobj->ماخذ_ڈیٹا = حافظہ_دو(newobj->ماخذ_alloc);
		اگر (newobj->ماخذ_ڈیٹا == تہی) {
			llcache_object_destroy(newobj);
			واپس NSERROR_NOMEM;
		}
		حافظہ_نقل(newobj->ماخذ_ڈیٹا, object->ماخذ_ڈیٹا,
				newobj->ماخذ_len);
	}

	اگر (object->عدد_سرخیاں > 0) {
		newobj->سرخیاں = صفر_حافظہ_دو(object->عدد_سرخیاں,
				حجم(llcache_سرخی));
		اگر (newobj->سرخیاں == تہی) {
			llcache_object_destroy(newobj);
			واپس NSERROR_NOMEM;
		}
		جب (newobj->عدد_سرخیاں < object->عدد_سرخیاں) {
			llcache_سرخی *nh =
					&(newobj->سرخیاں[newobj->عدد_سرخیاں]);
			llcache_سرخی *oh =
					&(object->سرخیاں[newobj->عدد_سرخیاں]);
			newobj->عدد_سرخیاں += 1;
			nh->name = strdup(oh->name);
			nh->قدر = strdup(oh->قدر);
			اگر (nh->name == تہی || nh->قدر == تہی) {
				llcache_object_destroy(newobj);
				واپس NSERROR_NOMEM;
			}
		}
	}

	اگر (object->chain != تہی) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		اگر (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			واپس error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	واپس NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
ساکن اندرونی uint32_t
کل_object_سائز(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = حجم(*object);
	tot += nsurl_لمبائی(object->url);

	اگر (object->ماخذ_ڈیٹا != تہی) {
		tot += object->ماخذ_len;
	}

	tot += حجم(llcache_سرخی) * object->عدد_سرخیاں;

	برائے (hdrc = 0; hdrc < object->عدد_سرخیاں; hdrc++) {
		اگر (object->سرخیاں[hdrc].name != تہی) {
			tot += لڑی_لمبائی(object->سرخیاں[hdrc].name);
		}
		اگر (object->سرخیاں[hdrc].قدر != تہی) {
			tot += لڑی_لمبائی(object->سرخیاں[hdrc].قدر);
		}
	}

	tot += cert_chain_سائز(object->chain);

	واپس tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
ساکن خالی llcache_catch_اوپر_all_users(خالی *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_اوپر = true;

	/* Catch new users up with state of objects */
	برائے (object = llcache->cached_objects; object != تہی;
			object = object->اگلا) {
		llcache_object_notify_users(object);
	}

	برائے (object = llcache->uncached_objects; object != تہی;
			object = object->اگلا) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
ساکن خالی llcache_users_not_caught_اوپر(خالی)
{
	اگر (llcache->all_caught_اوپر) {
		llcache->all_caught_اوپر = false;
		guit->misc->جدول_وقت(0, llcache_catch_اوپر_all_users, تہی);
	}
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/

/*
 * Attempt to clean the cache
 *
 * The memory cache cleaning discards objects in order of increasing value.
 *
 * Exported interface documented in llcache.h
 */
خالی llcache_clean(bool purge)
{
	llcache_object *object, *اگلا;
	uint32_t llcache_سائز = 0;
	صحیح بقایا_lifetime;
	uint32_t حد_بندی;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	اگر (purge) {
		حد_بندی = 0;
	} ورنہ {
		حد_بندی = llcache->حد_بندی;
	}

	/* Uncacheable objects with no users or fetches */
	برائے (object = llcache->uncached_objects;
	     object != تہی;
	     object = اگلا) {
		اگلا = object->اگلا;

		/* The candidate count of uncacheable objects is always 0 */
		اگر ((object->users == تہی) &&
		    (object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} ورنہ {
			llcache_سائز += کل_object_سائز(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	برائے (object = llcache->cached_objects;
	     object != تہی;
	     object = اگلا) {
		اگلا = object->اگلا;

		بقایا_lifetime = llcache_object_rfc2616_بقایا_lifetime(
				&object->cache);

		اگر ((object->users == تہی) &&
		    (object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی) &&
		    (بقایا_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				اگر (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} ورنہ {
			/* object has users so account for the storage */
			llcache_سائز += کل_object_سائز(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	اگر (حد_بندی < llcache_سائز) {
		llcache_persist(تہی);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	برائے (object = llcache->cached_objects;
	     ((حد_بندی < llcache_سائز) && (object != تہی));
	     object = اگلا) {
		اگلا = object->اگلا;
		اگر ((object->users == تہی) &&
		    (object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->ماخذ_ڈیٹا = تہی;

			llcache_سائز -=	object->ماخذ_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->ماخذ_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	برائے (object = llcache->cached_objects;
	     ((حد_بندی < llcache_سائز) && (object != تہی));
	     object = اگلا) {
		اگلا = object->اگلا;
		اگر ((object->users == تہی) &&
		    (object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->ماخذ_ڈیٹا == تہی)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ماخذ_len,
			      (طویل)(time(تہی) - object->آخری_مستعمل),
			      object,
			      nsurl_access(object->url));

			llcache_سائز -=	کل_object_سائز(object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);

		}
	}

	/* Fresh cacheable objects with no users or pending fetches
	 * while the cache exceeds the configured size. These are the
	 * most valuable objects as replacing them is a full network
	 * fetch
	 */
	برائے (object = llcache->cached_objects;
	     ((حد_بندی < llcache_سائز) && (object != تہی));
	     object = اگلا) {
		اگلا = object->اگلا;

		اگر ((object->users == تہی) &&
		    (object->candidate_گنتی == 0) &&
		    (object->fetch.fetch == تہی) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ماخذ_len,
			      (طویل)(time(تہی) - object->آخری_مستعمل),
			      object,
			      nsurl_access(object->url));

			llcache_سائز -=	object->ماخذ_len + حجم(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_سائز, حد_بندی);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ثابت ساخت llcache_parameters *prm)
{
	llcache = صفر_حافظہ_دو(1, حجم(ساخت llcache_s));
	اگر (llcache == تہی) {
		واپس NSERROR_NOMEM;
	}

	llcache->حد_بندی = prm->حد_بندی;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->زیادہ_سے_زیادہ_bandwidth = prm->زیادہ_سے_زیادہ_bandwidth;
	llcache->وقت_وقتی_حصہ = prm->وقت_وقتی_حصہ;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_اوپر = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->حد_بندی);

	/* backing store initialisation */
	واپس guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
خالی llcache_finalise(خالی)
{
	llcache_object *object, *اگلا;
	uint64_t کل_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(تہی);
	/* Now clear the persistence callback */
	guit->misc->جدول_وقت(-1, llcache_persist, تہی);

	/* Clean uncached objects */
	برائے (object = llcache->uncached_objects; object != تہی; object = اگلا) {
		llcache_object_user *user, *اگلا_user;

		اگلا = object->اگلا;

		برائے (user = object->users; user != تہی; user = اگلا_user) {
			اگلا_user = user->اگلا;

			اگر (user->handle != تہی)
				آزاد(user->handle);

			آزاد(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = تہی;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	برائے (object = llcache->cached_objects; object != تہی; object = اگلا) {
		llcache_object_user *user, *اگلا_user;

		اگلا = object->اگلا;

		برائے (user = object->users; user != تہی; user = اگلا_user) {
			اگلا_user = user->اگلا;

			اگر (user->handle != تہی)
			       آزاد(user->handle);

			آزاد(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = تہی;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	اگر (llcache->کل_elapsed > 0) {
		کل_bandwidth = (llcache->کل_written * 1000) /
			llcache->کل_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->کل_written,
	      llcache->کل_elapsed,
	      کل_bandwidth);

	آزاد(llcache);
	llcache = تہی;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ثابت llcache_post_ڈیٹا *post,
			llcache_handle_callback cb, خالی *pw,
			llcache_handle **نتیجہ)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_اندر_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_اندر_use);
	اگر (error != NSERROR_OK) {
		واپس error;
	}

	/* Can we fetch this URL at all? */
	اگر (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		واپس NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	اگر (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		واپس error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_اندر_use, &object);
	اگر (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		واپس error;
	}

	/* Add user to object */
	llcache_object_جوڑو_user(object, user);

	*نتیجہ = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_اوپر();

	nsurl_unref(hsts_url);

	واپس NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, خالی *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	واپس NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_ڈھونڈو_user(handle);

	أكد(user != تہی);

	اگر (user->iterator_ہدف) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} ورنہ {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		اگر (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	واپس error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **نتیجہ)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	اگر (error == NSERROR_OK) {
		llcache_object_جوڑو_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*نتیجہ = newuser->handle;
	}

	واپس error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ڈھونڈو_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	اگر (user->پچھلا != تہی)
		all_alone = false;
	اگر (user->اگلا != تہی)
		all_alone = false;

	اگر (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		اگر (error != NSERROR_OK)
			واپس error;

		/* Move across to the new object */
		اگر (user->iterator_ہدف) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					صفر_حافظہ_دو(1, حجم(llcache_object_user));
			اگر (newuser == تہی) {
				llcache_object_destroy(newobject);
				واپس NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = تہی;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_جوڑو_user(newobject, newuser);
		} ورنہ {
			llcache_object_remove_user(object, user);
			llcache_object_جوڑو_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_جوڑو_مقصد_list(newobject,
				&llcache->uncached_objects);
	} ورنہ {
		/* We're the only user, so abort any fetch in progress */
		اگر (object->fetch.fetch != تہی) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = تہی;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ڈیٹا(object);
	}

	واپس error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_دھارا(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ڈھونڈو_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	اگر (user->پچھلا != تہی || user->اگلا != تہی)
		واپس NSERROR_OK;

	/* Forcibly uncache this object */
	اگر (llcache_object_اندر_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_جوڑو_مقصد_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_دھارا_ڈیٹا;

	واپس NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_ڈیٹا(llcache_handle *handle)
{
	اگر ((handle->object != تہی) &&
	    (handle->object->fetch.fetch == تہی) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	واپس NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ثابت llcache_handle *handle)
{
	واپس handle->object != تہی ? handle->object->url : تہی;
}

/* See llcache.h for documentation */
ثابت uint8_t *llcache_handle_get_ماخذ_ڈیٹا(ثابت llcache_handle *handle,
		size_t *سائز)
{
	*سائز = handle->object != تہی ? handle->object->ماخذ_len : 0;

	واپس handle->object != تہی ? handle->object->ماخذ_ڈیٹا : تہی;
}

/* See llcache.h for documentation */
ثابت حرف *llcache_handle_get_سرخی(ثابت llcache_handle *handle,
		ثابت حرف *کلید)
{
	ثابت llcache_object *object = handle->object;
	size_t i;

	اگر (object == تہی)
		واپس تہی;

	/* About as trivial as possible */
	برائے (i = 0; i < object->عدد_سرخیاں; i++) {
		اگر (strcasecmp(کلید, object->سرخیاں[i].name) == 0)
			واپس object->سرخیاں[i].قدر;
	}

	واپس تہی;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ثابت llcache_handle *a,
		ثابت llcache_handle *b)
{
	واپس a->object == b->object;
}
