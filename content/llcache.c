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

#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <nsutils/time.h>
#ضمن <nsutils/base64.h>

#ضمن "netsurf/inttypes.h"
#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/utils.h"
#ضمن "utils/time.h"
#ضمن "utils/http.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/misc.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/fetch.h"
#ضمن "content/backing_store.h"
#ضمن "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
تعريفنوع تعداد {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_HEADERS,		/**< Fetching headers */
	LLCACHE_FETCH_DATA,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
تعريفنوع بنية llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
بنية llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	فراغ *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t bytes;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
تعريفنوع بنية llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_target;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	بنية llcache_object_user *prev;	/**< Previous in list */
	بنية llcache_object_user *next;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
تعريفنوع بنية {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_data *post;	/**< POST data, or NULL for GET */

	بنية fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_count;	/**< Count of redirects followed */

	uint32_t retries_remaining;     /**< Number of times to retry on timeout */

	bool hsts_in_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
تعريفنوع تعداد {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#define INVALID_AGE -1

/** Cache control data */
تعريفنوع بنية {
	time_t req_time;	/**< Time of request */
	time_t res_time;	/**< Time of response */
	time_t fin_time;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	صحيح age;		/**< Age: response header */
	صحيح max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	حرف *etag;		/**< Etag: response header */
	time_t last_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
تعريفنوع بنية {
	حرف *name;		/**< Header name */
	حرف *value;		/**< Header value */
} llcache_header;

/** Current status of an object's data */
تعريفنوع تعداد {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
بنية llcache_object {
	llcache_object *prev;	     /**< Previous in list */
	llcache_object *next;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *source_data;	     /**< Source data for object */
	size_t source_len;	     /**< Byte length of source data */
	size_t source_alloc;	     /**< Allocated size of source buffer */

	بنية cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_count;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_header *headers;     /**< Fetch headers */
	size_t num_headers;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t last_used; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
بنية llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t limit;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_up;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	صحيح minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	unsigned long time_quantum;

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
	size_t maximum_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t total_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t total_elapsed;

};

/** low level cache state */
ساكن بنية llcache_s *llcache = عدم;

/* forward referenced callback function */
ساكن فراغ llcache_fetch_callback(ثابت fetch_msg *msg, فراغ *p);

/* forward referenced catch up function */
ساكن فراغ llcache_users_not_caught_up(فراغ);


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
ساكن nserror llcache_object_user_new(llcache_handle_callback cb, فراغ *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = calloc(1, حجم(llcache_handle));
	אם (h == عدم) {
		החזר NSERROR_NOMEM;
	}

	u = calloc(1, حجم(llcache_object_user));
	אם (u == عدم) {
		حرر(h);
		החזר NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (فراغ *) cb, pw);

	*user = u;

	החזר NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
ساكن nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	أكد(user->next == عدم);
	أكد(user->prev == عدم);

	אם (user->handle != عدم)
		حرر(user->handle);

	حرر(user);

	החזר NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
ساكن nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	أكد(user != عدم);
	أكد(object != عدم);
	أكد(object->users != عدم);
	أكد(user->handle == عدم || user->handle->object == object);
	أكد((user->prev != عدم) || (object->users == user));

	אם (user == object->users)
		object->users = user->next;
	אחרת
		user->prev->next = user->next;

	אם (user->next != عدم)
		user->next->prev = user->prev;

	user->next = user->prev = عدم;

	/* record the time the last user was removed from the object */
	אם (object->users == عدم) {
		object->last_used = time(عدم);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	החזר NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساكن nserror llcache_send_event_to_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *next_user;

	user = object->users;
	כלעוד (user != عدم) {
		bool was_target = user->iterator_target;
		user->iterator_target = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		next_user = user->next;

		user->iterator_target = was_target;

		אם (user->queued_for_delete && !was_target) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		אם (error != NSERROR_OK)
			اكسر;

		user = next_user;
	}

	החזר error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_object_new(nsurl *url, llcache_object **result)
{
	llcache_object *obj = calloc(1, حجم(llcache_object));
	אם (obj == عدم)
		החזר NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*result = obj;

	החזר NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_post_data_clone(ثابت llcache_post_data *orig,
		llcache_post_data **clone)
{
	llcache_post_data *post_clone;

	post_clone = calloc(1, حجم(llcache_post_data));
	אם (post_clone == عدم)
		החזר NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	אם (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->data.urlenc = strdup(orig->data.urlenc);
		אם (post_clone->data.urlenc == عدم) {
			حرر(post_clone);

			החזר NSERROR_NOMEM;
		}
	} אחרת {
		post_clone->data.multipart = fetch_multipart_data_clone(
				orig->data.multipart);
		אם (post_clone->data.multipart == عدم) {
			حرر(post_clone);

			החזר NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	החזר NSERROR_OK;
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
ساكن nserror llcache_fetch_split_header(ثابت uint8_t *data, size_t len,
		حرف **name, حرف **value)
{
	حرف *n, *v;
	ثابت uint8_t *colon;

	/* Strip leading whitespace from name */
	כלעוד (data[0] == ' ' || data[0] == '\t' ||
	       data[0] == '\r' || data[0] == '\n') {
		data++;
	}

	/* Find colon */
	colon = (ثابت uint8_t *) strchr((ثابت حرف *) data, ':');
	אם (colon == عدم) {
		/* Failed, assume a key with no value */
		colon = data + strlen((ثابت حرف *)data);

		/* Strip trailing whitespace from name */
		כלעוד ((colon > data) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ثابت حرف *) data, colon - data);
		אם (n == عدم)
			החזר NSERROR_NOMEM;

		v = strdup("");
		אם (v == عدم) {
			حرر(n);
			החזר NSERROR_NOMEM;
		}
	} אחרת {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		כלעוד (colon > data && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ثابت حرف *) data, colon - data);
		אם (n == عدم)
			החזר NSERROR_NOMEM;

		/* Find colon again */
		כלעוד (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		עשה {
			colon++;
		} כלעוד (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		כלעוד (len > 0 && (data[len - 1] == ' ' ||
				data[len - 1] == '\t' ||
				data[len - 1] == '\r' ||
				data[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ثابت حرف *) colon, len - (colon - data));
		אם (v == عدم) {
			حرر(n);
			החזר NSERROR_NOMEM;
		}
	}

	*name = n;
	*value = v;

	החזר NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror
llcache_fetch_parse_cache_control(llcache_object *object, حرف *value)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(value, &cc);
	אם (error != NSERROR_OK) {
		/* Ignore parse errors */
		החזר NSERROR_OK;
	}

	אם (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	אם (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	החזר NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror
llcache_fetch_header_cache_control(llcache_object *object,
				   حرف *name,
				   حرف *value)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = strlen(name);

	بدل (name_len) {
	حالة 3:
		אם (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			אם ('0' <= *value && *value <= '9') {
				object->cache.age = atoi(value);
			}

		}
		اكسر;

	حالة 4:
		אם (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(value,
					    strlen(value),
					    &object->cache.date);
			אם (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      value, res);
			}
		} אחרת אם (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			حرر(object->cache.etag);
			object->cache.etag = strdup(value);
			אם (object->cache.etag == عدم) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				החזר NSERROR_NOMEM;
			}
		}
		اكسر;

	حالة 7:
		אם (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(value,
					    strlen(value),
					    &object->cache.expires);
			אם (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      value, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		اكسر;

	حالة 13:
		אם (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,value);
		} אחרת אם (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(value,
				      strlen(value),
				      &object->cache.last_modified);
		}
		اكسر;
	}

	החזר NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
ساكن مضمن فراغ llcache_destroy_headers(llcache_object *object)
{
	כלעוד (object->num_headers > 0) {
		object->num_headers--;

		حرر(object->headers[object->num_headers].name);
		حرر(object->headers[object->num_headers].value);
	}
	حرر(object->headers);
	object->headers = عدم;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
ساكن مضمن فراغ llcache_invalidate_cache_control_data(llcache_object *object)
{
	حرر(object->cache.etag);
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
ساكن nserror llcache_fetch_process_header(llcache_object *object,
		ثابت uint8_t *data, size_t len)
{
	nserror res;
	حرف *name, *value;
	llcache_header *temp;

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
	אם (strncmp((ثابت حرف *) data, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_time = object->cache.req_time;

		llcache_invalidate_cache_control_data(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_time = req_time;

		llcache_destroy_headers(object);
	}

	/* Set fetch response time if not already set */
	אם (object->cache.res_time == 0) {
		object->cache.res_time = time(عدم);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_split_header(data, len, &name, &value);
	אם (res != NSERROR_OK) {
		החזר res;
	}

	/* deal with empty header */
	אם (name[0] == 0) {
		حرر(name);
		حرر(value);
		החזר NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_header_cache_control(object, name, value);
	אם (res != NSERROR_OK) {
		حرر(name);
		حرر(value);
		החזר res;
	}

	/* Append header data to the object's headers array */
	temp = realloc(object->headers,
		       (object->num_headers + 1) * حجم(llcache_header));
	אם (temp == عدم) {
		حرر(name);
		حرر(value);
		החזר NSERROR_NOMEM;
	}

	object->headers = temp;

	object->headers[object->num_headers].name = name;
	object->headers[object->num_headers].value = value;

	object->num_headers++;

	החזר NSERROR_OK;
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
ساكن nserror get_referer_header(nsurl *url, nsurl *referer, حرف **header_out)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool match;
	bool match1;
	bool match2;
	حرف *header;

	/* Determine whether to send the Referer header */
	אם (!nsoption_bool(send_referer)) {
		החזר NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	אם (scheme == عدم) {
		החזר NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	אם (ref_scheme == عدم) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		החזר NSERROR_INVALID;
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
	אם (lwc_string_isequal(scheme, ref_scheme,
			       &match) != lwc_error_ok) {
		match = false;
	}
	אם (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	אם (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	אם (match == true || (match1 == true && match2 == true)) {
		ثابت size_t len = SLEN("Referer: ") +
			nsurl_length(referer) + 1;

		header = malloc(len);
		אם (header == عدم) {
			res = NSERROR_NOMEM;
		} אחרת {
			snprintf(header, len, "Referer: %s",
				 nsurl_access(referer));

			*header_out = header;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	החזר res;
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
ساكن nserror llcache_object_refetch(llcache_object *object)
{
	ثابت حرف *urlenc = عدم;
	بنية fetch_multipart_data *multipart = عدم;
	حرف **headers = عدم;
	صحيح header_idx = 0;
	nserror res;

	אם (object->fetch.post != عدم) {
		אם (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->data.urlenc;
		} אחרת {
			multipart = object->fetch.post->data.multipart;
		}
	}

	/* Generate headers */
	headers = malloc(4 * حجم(حرف *));
	אם (headers == عدم) {
		החזר NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	אם (object->cache.etag != عدم) {
		ثابت size_t len = SLEN("If-None-Match: ") +
				strlen(object->cache.etag) + 1;

		headers[header_idx] = malloc(len);
		אם (headers[header_idx] == عدم) {
			حرر(headers);
			החזר NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-None-Match: %s",
				object->cache.etag);

		header_idx++;
	}

	/* cache-control header for modification time */
	אם (object->cache.last_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ثابت size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		headers[header_idx] = malloc(len);
		אם (headers[header_idx] == عدم) {
			כלעוד (--header_idx >= 0)
				حرر(headers[header_idx]);
			حرر(headers);
			החזר NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.last_modified));

		header_idx++;
	}

	/* Referer header */
	אם (object->fetch.referer != عدم) {
		אם (get_referer_header(object->url,
				       object->fetch.referer,
				       &headers[header_idx]) == NSERROR_OK) {
			header_idx++;
		}
	}
	headers[header_idx] = عدم;

	/* Reset cache control data */
	llcache_invalidate_cache_control_data(object);
	object->cache.req_time = time(عدم);
	object->cache.fin_time = object->cache.req_time;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_start(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_PAGES,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (ثابت حرف **)headers,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	כלעוד (--header_idx >= 0) {
		حرر(headers[header_idx]);
	}
	حرر(headers);

	החזר res;
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
ساكن nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ثابت llcache_post_data *post,
		uint32_t redirect_count, bool hsts_in_use)
{
	nserror error;
	nsurl *referer_clone = عدم;
	llcache_post_data *post_clone = عدم;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	אם (post != عدم) {
		error = llcache_post_data_clone(post, &post_clone);
		אם (error != NSERROR_OK)
			החזר error;
	}

	אם (referer != عدم)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_count = redirect_count;
	object->fetch.retries_remaining = llcache->fetch_attempts;
	object->fetch.hsts_in_use = hsts_in_use;

	החזר llcache_object_refetch(object);
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
ساكن nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	אם (object->source_data != عدم) {
		אם (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} אחרת {
			حرر(object->source_data);
		}
	}

	nsurl_unref(object->url);

	אם (object->fetch.fetch != عدم) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = عدم;
	}

	אם (object->fetch.referer != عدم)
		nsurl_unref(object->fetch.referer);

	אם (object->fetch.post != عدم) {
		אם (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			حرر(object->fetch.post->data.urlenc);
		} אחרת {
			fetch_multipart_data_destroy(
					object->fetch.post->data.multipart);
		}

		حرر(object->fetch.post);
	}

	حرر(object->cache.etag);

	עבור (i = 0; i < object->num_headers; i++) {
		حرر(object->headers[i].name);
		حرر(object->headers[i].value);
	}
	حرر(object->headers);

	حرر(object);

	החזר NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
ساكن nserror llcache_object_add_to_list(llcache_object *object,
		llcache_object **list)
{
	object->prev = عدم;
	object->next = *list;

	אם (*list != عدم)
		(*list)->prev = object;
	*list = object;

	החזר NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
ساكن صحيح
llcache_object_rfc2616_remaining_lifetime(ثابت llcache_cache_control *cd)
{
	صحيح current_age, freshness_lifetime;
	time_t now = time(عدم);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_time - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_time - cd->req_time + now - cd->res_time;

	/* Determine freshness lifetime of this object */
	אם (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} אחרת אם (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} אחרת אם (cd->last_modified != 0) {
		freshness_lifetime = (now - cd->last_modified) / 10;
	} אחרת {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	אם ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		החזר freshness_lifetime - current_age;
	}

	החזר 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
ساكن bool llcache_object_is_fresh(ثابت llcache_object *object)
{
	صحيح remaining_lifetime;
	ثابت llcache_cache_control *cd = &object->cache;

	remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(cd);

	NSLOG(llcache, DEBUG, "%p: (%d > 0 || %d != %d)", object,
	     remaining_lifetime,
	     object->fetch.state, LLCACHE_FETCH_COMPLETE);

	/* The object is fresh if:
	 * - it was not forbidden from being returned from the cache
	 *   unvalidated.
	 *
	 * - it has remaining lifetime or still being fetched.
	 */
	החזר ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
		((remaining_lifetime > 0) ||
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
ساكن nserror llcache_object_clone_cache_data(llcache_object *source,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	אם (source->cache.etag != عدم) {
		حرف *etag = source->cache.etag;

		אם (deep) {
			/* Copy the etag */
			etag = strdup(source->cache.etag);
			אם (etag == عدم)
				החזר NSERROR_NOMEM;
		} אחרת {
			/* Destination takes ownership */
			source->cache.etag = عدم;
		}

		אם (destination->cache.etag != عدم)
			حرر(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_time = source->cache.req_time;
	destination->cache.res_time = source->cache.res_time;
	destination->cache.fin_time = source->cache.fin_time;

	אם (source->cache.date != 0)
		destination->cache.date = source->cache.date;

	אם (source->cache.expires != 0)
		destination->cache.expires = source->cache.expires;

	אם (source->cache.age != INVALID_AGE)
		destination->cache.age = source->cache.age;

	אם (source->cache.max_age != INVALID_AGE)
		destination->cache.max_age = source->cache.max_age;

	אם (source->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = source->cache.no_cache;

	אם (source->cache.last_modified != 0)
		destination->cache.last_modified = source->cache.last_modified;

	החזר NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
ساكن nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	אם (object == *list)
		*list = object->next;
	אחרת
		object->prev->next = object->next;

	אם (object->next != عدم)
		object->next->prev = object->prev;

	החזר NSERROR_OK;
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
ساكن nserror llcache_retrieve_persisted_data(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	אם ((object->source_data != عدم) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		החזר NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	החזר guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->source_data,
				    &object->source_len);
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
ساكن nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **data_out,
			   size_t *datasize_out)
{
	size_t allocsize;
	صحيح datasize;
	uint8_t *data;
	حرف *op;
	unsigned صحيح hloop;
	صحيح use;
	size_t cert_chain_depth;

	אם (object->chain != عدم) {
		cert_chain_depth = object->chain->depth;
	} אחרת {
		cert_chain_depth = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	עבור (hloop = 0 ; hloop < object->num_headers ; hloop++) {
		allocsize += strlen(object->headers[hloop].name) + 1;
		allocsize += strlen(object->headers[hloop].value) + 1;
	}

	allocsize += nsurl_length(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	עבור (hloop = 0; hloop < cert_chain_depth; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_length + 2) / 3);
	}

	data = malloc(allocsize);
	אם (data == عدم) {
		החזר NSERROR_NOMEM;
	}

	op = (حرف *)data;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	אם (use < 0) {
		اذهب operror;
	}
	use++; /* does not count the null */
	אם (use > datasize) {
		اذهب overflow;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->source_len);
	אם (use < 0) {
		اذهب operror;
	}
	use++; /* does not count the null */
	אם (use > datasize)
		اذهب overflow;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_time);
	אם (use == 0)
		اذهب overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_time);
	אם (use == 0)
		اذهب overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_time);
	אם (use == 0)
		اذهب overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->num_headers);
	אם (use < 0) {
		اذهب operror;
	}
	use++; /* does not count the null */
	אם (use > datasize)
		اذهب overflow;
	op += use;
	datasize -= use;

	/* headers */
	עבור (hloop = 0 ; hloop < object->num_headers ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->headers[hloop].name,
			       object->headers[hloop].value);
		אם (use < 0) {
			اذهب operror;
		}
		use++; /* does not count the null */
		אם (use > datasize)
			اذهب overflow;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_depth);
	אם (use < 0) {
		اذهب operror;
	}
	use++; /* does not count the null */
	אם (use > datasize)
		اذهب overflow;
	op += use;
	datasize -= use;

	/* SSL certificates */
	עבור (hloop = 0; hloop < cert_chain_depth; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (صحيح)(object->chain->certs[hloop].err));
		אם (use < 0) {
			اذهب operror;
		}
		use++; /* does not count the null */
		אם (use > datasize)
			اذهب overflow;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		אם (object->chain->certs[hloop].der != عدم) {
			size_t output_length = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_length,
				(uint8_t *)op,
				&output_length);
			אם (res != NSUERROR_OK) {
				اذهب operror;
			}
			use = output_length;
		} אחרת {
			use = 0;
		}
		use++; /* allow for null */
		אם (use > datasize)
			اذهب overflow;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*data_out = data;
	*datasize_out = allocsize - datasize;

	החזר NSERROR_OK;

overflow:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	حرر(data);
	החזר NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	حرر(data);
	החזר NSERROR_INVALID;
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
ساكن nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = عدم;
	size_t metadatalen = 0;
	size_t remaining = 0;
	nsurl *metadataurl;
	unsigned صحيح line;
	حرف *ln;
	صحيح lnsize;

	size_t source_length;
	time_t request_time;
	time_t response_time;
	time_t completion_time;
	size_t num_headers;
	size_t hloop;
	size_t ssl_cert_count = 0;
	بنية cert_chain *chain = عدم;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	אם (res != NSERROR_OK) {
		החזר res;
	}

	NSLOG(llcache, INFO, "Processing retrieved data");

	/* metadata is stored as a sequence of NULL terminated strings
	 * which we call 'line's here.
	 */

	/* We track remaining data because as we extend this data structure
	 * we need to know if we should continue to parse
	 */
	remaining = metadatalen;

	/* metadata line 1 is the url the metadata referrs to */
	line = 1;
	ln = (حرف *)metadata;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	אם (lnsize < 7) {
		res = NSERROR_INVALID;
		اذهب format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	אם (res != NSERROR_OK)
		اذهب format_error;

	אם (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		החזר NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	line = 2;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	אם ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &source_length) != 1)) {
		res = NSERROR_INVALID;
		اذهب format_error;
	}


	/* metadata line 3 is the time of request */
	line = 3;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_time);
	אם (res != NSERROR_OK)
		اذهب format_error;


	/* metadata line 4 is the time of response */
	line = 4;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_time);
	אם (res != NSERROR_OK)
		اذهب format_error;


	/* metadata line 5 is the time of request completion */
	line = 5;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_time);
	אם (res != NSERROR_OK)
		اذهب format_error;


	/* metadata line 6 is the number of headers */
	line = 6;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	אם ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &num_headers) != 1)) {
		res = NSERROR_INVALID;
		اذهب format_error;
	}

	/* read headers */
	עבור (hloop = 0 ; hloop < num_headers; hloop++) {
		line++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;

		res = llcache_fetch_process_header(object,
						   (uint8_t *)ln,
						   lnsize);
		אם (res != NSERROR_OK)
			اذهب format_error;
	}

	אם (remaining == 0) {
		اذهب skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	line++;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	אם ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &ssl_cert_count) != 1)) {
		res = NSERROR_INVALID;
		اذهب format_error;
	}

	אם (ssl_cert_count == 0) {
		اذهب skip_ssl_certificates;
	}

	אם (ssl_cert_count > MAX_CERT_DEPTH) {
		res = NSERROR_INVALID;
		اذهب format_error;
	}

	res = cert_chain_alloc(ssl_cert_count, &chain);
	אם (res != NSERROR_OK) {
		اذهب format_error;
	}

	עבור (hloop = 0; hloop < ssl_cert_count; hloop++) {
		صحيح errcode;
		nsuerror nsures;

		/* Certificate error code */
		line++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;
		אם ((lnsize < 1) || (sscanf(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			اذهب format_error;
		}
		אם (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_KNOWN) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} אחרת {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		line++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;
		אם (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ثابت uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_length);
			אם (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				اذهب format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->source_len = source_length;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->source_alloc = metadatalen;

	object->cache.req_time = request_time;
	object->cache.res_time = response_time;
	object->cache.fin_time = completion_time;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	החזר NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      line, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	החזר res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
ساكن مضمن bool llcache__scheme_is_persistable(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	אם ((lwc_string_isequal(scheme, corestring_lwc_http,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&match) == lwc_error_ok &&
			(match == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	החזר persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
ساكن مضمن bool llcache__scheme_is_cachable(ثابت nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	אם ((lwc_string_isequal(scheme, corestring_lwc_http,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_data,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&match) == lwc_error_ok &&
			(match == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	החזר cachable;
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
ساكن nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ثابت llcache_post_data *post,
				uint32_t redirect_count)
{
	nserror error;
	nsurl *referer_clone = عدم;
	llcache_post_data *post_clone = عدم;

	אם (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		החזר NSERROR_NOT_FOUND;
	}

	object->cache.req_time = time(عدم);
	object->cache.fin_time = object->cache.req_time;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	אם (error != NSERROR_OK) {
		החזר error;
	}

	/* entry came out of cache - need to setup object state */
	אם (post != عدم) {
		error = llcache_post_data_clone(post, &post_clone);
		אם (error != NSERROR_OK)
			החזר error;
	}

	אם (referer != عدم) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_count = redirect_count;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = عدم;

	החזר NSERROR_OK;
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
ساكن nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ثابت llcache_post_data *post,
				   uint32_t redirect_count,
				   bool hsts_in_use,
				   llcache_object **result)
{
	nserror error;
	llcache_object *obj, *newest = عدم;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==عدم?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	עבור (obj = llcache->cached_objects; obj != عدم; obj = obj->next) {

		אם ((newest == عدم ||
		     obj->cache.req_time > newest->cache.req_time) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	אם (newest == عدم) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		אם (error != NSERROR_OK)
			החזר error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_count);
		אם (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_add_to_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	אם ((newest != عدم) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_data(newest);
		אם (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*result = newest;

			החזר NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		אם (error != NSERROR_OK) {
			החזר error;
		}
	} אחרת אם (newest != عدم) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_data(newest);
		אם (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			אם (error != NSERROR_OK)
				החזר error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_data(newest, obj, true);
			אם (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				החזר error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_count++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_count, hsts_in_use);
			אם (error != NSERROR_OK) {
				newest->candidate_count--;
				llcache_object_destroy(obj);
				החזר error;
			}

			/* Add new object to cache */
			llcache_object_add_to_list(obj, &llcache->cached_objects);

			*result = obj;

			החזר NSERROR_OK;
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
		אם (error != NSERROR_OK) {
			החזר error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_count, hsts_in_use);
	אם (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		החזר error;
	}

	/* Add new object to cache */
	llcache_object_add_to_list(obj, &llcache->cached_objects);

	*result = obj;

	החזר NSERROR_OK;
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
ساكن nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ثابت llcache_post_data *post,
			uint32_t redirect_count,
			bool hsts_in_use,
			llcache_object **result)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==عدم?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	אם (error != NSERROR_OK)
		החזר error;

	/* determine if content is cachable */
	אם ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} אחרת אם (post != عدم) {
		/* POST requests are never cached */
		uncachable = true;
	} אחרת {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	אם (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		אם (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			החזר error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_count, hsts_in_use);
		אם (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			החזר error;
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(obj, &llcache->uncached_objects);
	} אחרת {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_count,
				hsts_in_use, &obj);
		אם (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			החזר error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*result = obj;

	nsurl_unref(defragmented_url);

	החזר NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
ساكن nserror llcache_object_add_user(llcache_object *object,
		llcache_object_user *user)
{
	أكد(user->next == عدم);
	أكد(user->prev == عدم);
	أكد(user->handle != عدم);

	user->handle->object = object;

	user->prev = عدم;
	user->next = object->users;

	אם (object->users != عدم)
		object->users->prev = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	החזר NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_hsts_transform_url(nsurl *url, nsurl **result,
		bool *hsts_in_use)
{
	lwc_string *scheme = عدم;
	bool match;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	אם (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTP fetch: no transform required */
		אם (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok && match) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_in_use = urldb_get_hsts_enabled(url);
		} אחרת {
			/* Anything else: no HSTS */
			*hsts_in_use = false;
		}
		lwc_string_unref(scheme);
		*result = nsurl_ref(url);
		החזר error;
	}
	lwc_string_unref(scheme);

	אם (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				result);
		*hsts_in_use = (error == NSERROR_OK);
	} אחרת {
		*result = nsurl_ref(url);
		*hsts_in_use = false;
	}

	החזר error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = عدم;
	bool match = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	אם (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		החזר NSERROR_OK;
	}
	lwc_string_unref(scheme);

	אם (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		החזר NSERROR_OK;
	}

	עבור (i = 0; i < object->num_headers; i++) {
		אם (strcasecmp("Strict-Transport-Security",
				object->headers[i].name) == 0) {
			urldb_set_hsts_policy(object->url,
					object->headers[i].value);
			/* Only process the first one we find */
			اكسر;
		}
	}

	החזר NSERROR_OK;
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
ساكن nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool match;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	אם ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&match) == lwc_error_ok && match == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&match) == lwc_error_ok && match == false)) {
		/* file, about and resource are not valid redirect targets */
		אם ((lwc_string_isequal(scheme, corestring_lwc_file,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&match) == lwc_error_ok && match == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			החזר NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	אם (fetch_can_fetch(dst_url) == false) {
		החזר NSERROR_UNSAFE_REDIRECT;
	}

	החזר NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_fetch_redirect(llcache_object *object,
		ثابت حرف *target, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *next;
	ثابت llcache_post_data *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_in_use;
	http_response_code http_code;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_code = fetch_http_code(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = عدم;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_data(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(فراغ) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	אם (object->fetch.redirect_count > nsoption_uint(fetch_redirect_limit)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		החזר NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, target, &url);
	אם (res != NSERROR_OK) {
		החזר res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	nsurl_unref(url);
	אם (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		החזר res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	אם (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		החזר res;
	}

	/* alter behaviour based on specific redirect code */
	بدل (http_code) {
	حالة HTTP_RESPONSE_MOVED_PERMANENTLY:
	حالة HTTP_RESPONSE_FOUND:
	حالة HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = عدم;
		اكسر;

	حالة HTTP_RESPONSE_TEMPORARY_REDIRECT:
	حالة HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		اكسر;

	افتراضي:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_code);
		nsurl_unref(hsts_url);
		החזר NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.data.redirect.from = object->url;
	event.data.redirect.to = hsts_url;

	res = llcache_send_event_to_users(object, &event);
	אם (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		החזר res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_count + 1,
					hsts_in_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	אם (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		עבור (user = object->users; user != عدم; user = next) {
			next = user->next;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	החזר res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
ساكن nserror llcache_object_cache_update(llcache_object *object)
{
	אם (object->cache.date == 0)
		object->cache.date = time(عدم);

	החזר NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
ساكن nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	אם (object->candidate != عدم) {
		llcache_object_user *user, *next;

		/* Move user(s) to candidate content */
		עבור (user = object->users; user != عدم; user = next) {
			next = user->next;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_count--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_data(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		אם (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = عدم;
	} אחרת {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = عدم;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(فراغ) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	החזר NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساكن nserror
llcache_fetch_process_data(llcache_object *object,
			   ثابت uint8_t *data,
			   size_t len)
{
	אם (object->fetch.state != LLCACHE_FETCH_DATA) {
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
		long http_code = fetch_http_code(object->fetch.fetch);

		אם ((http_code != 200 && http_code != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_data(object);
		}

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		object->fetch.state = LLCACHE_FETCH_DATA;
	}

	/* Resize source buffer if it's too small */
	אם (object->source_len + len >= object->source_alloc) {
		ثابت size_t new_len = object->source_len + len + 64 * 1024;
		uint8_t *temp = realloc(object->source_data, new_len);
		אם (temp == عدم)
			החזר NSERROR_NOMEM;

		object->source_data = temp;
		object->source_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	memcpy(object->source_data + object->source_len, data, len);
	object->source_len += len;

	החזר NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ساكن nserror llcache_fetch_auth(llcache_object *object, ثابت حرف *realm)
{
	ثابت حرف *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = عدم;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Destroy headers */
	llcache_destroy_headers(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	אם (realm == عدم)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	אם (auth == عدم || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_BAD_AUTH;
		event.data.error.msg = realm;

		error = llcache_send_event_to_users(object, &event);
	} אחרת {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	החזר error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = عدم;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	אם (object->fetch.hsts_in_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_BAD_CERTS;
		event.data.error.msg = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} אחרת {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.msg = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	}

	החזר error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = عدم;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	אם (object->fetch.hsts_in_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.msg = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} אחרת {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	החזר error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
ساكن nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	אם (object->fetch.retries_remaining > 1) {
		object->fetch.retries_remaining--;
		החזר llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = عدم;

	/* Release candidate, if any */
	אם (object->candidate != عدم) {
		object->candidate->candidate_count--;
		object->candidate = عدم;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_data(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.data.error.code = NSERROR_TIMEOUT;
	event.data.error.msg = عدم;

	החזר llcache_send_event_to_users(object, &event);
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
ساكن nserror
build_candidate_list(بنية llcache_object ***lst_out, صحيح *lst_len_out)
{
	llcache_object *object, *next;
	بنية llcache_object **lst;
	صحيح lst_len = 0;
	صحيح remaining_lifetime;

#define MAX_PERSIST_PER_RUN 128

	lst = calloc(MAX_PERSIST_PER_RUN, حجم(بنية llcache_object *));
	אם (lst == عدم) {
		החזר NSERROR_NOMEM;
	}

	עבור (object = llcache->cached_objects; object != عدم; object = next) {
		next = object->next;

		/* Only consider http(s) for the disc cache. */
		אם (!llcache__scheme_is_persistable(object->url)) {
			استمر;
		}

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		אם ((object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (remaining_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			אם (lst_len == MAX_PERSIST_PER_RUN)
				اكسر;
		}
	}

	אם (lst_len == 0) {
		حرر(lst);
		החזר NSERROR_NOT_FOUND;
	}

	/** \todo sort list here */

	*lst_len_out = lst_len;
	*lst_out = lst;

#undef MAX_PERSIST_PER_RUN

	החזר NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
write_backing_store(بنية llcache_object *object,
		    size_t *written_out,
		    unsigned long *elapsed)
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
				   object->source_data,
				   object->source_len);
	אם (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		החזר ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	אם (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		החזר ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	אם (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		החזר ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_out = object->source_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	אם (*elapsed == 0) {
		*elapsed = 1;
	}

	החזר NSERROR_OK;
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
ساكن فراغ llcache_persist_slowcheck(فراغ *p)
{
	uint64_t total_bandwidth; /* total bandwidth */

	אם (llcache->total_written > (2 * llcache->minimum_bandwidth)) {

		total_bandwidth = (llcache->total_written * 1000) / llcache->total_elapsed;

		אם (total_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      total_bandwidth,
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
ساكن فراغ llcache_persist(فراغ *p)
{
	nserror ret;
	بنية llcache_object **lst; /* candidate object list */
	صحيح lst_count; /* number of candidates in list */
	صحيح idx; /* current candidate object index in list */
	صحيح next = -1; /* when the next run should be scheduled for */

	unsigned long write_limit; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	unsigned long elapsed; /* how long writing an object took */

	size_t total_written = 0; /* total bytes written in this run */
	unsigned long total_elapsed = 1; /* total ms used to write bytes */
	unsigned long total_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_count);
	אם (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		החזר;
	}

	write_limit = (llcache->maximum_bandwidth * llcache->time_quantum) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	עבור (idx = 0; idx < lst_count; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		אם (ret != NSERROR_OK) {
			استمر;
		}

		/* successfully wrote object to backing store */
		total_written += written;
		total_elapsed += elapsed;
		total_bandwidth = (total_written * 1000) / total_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		אם (total_elapsed > llcache->time_quantum) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			אם (total_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->schedule(
						llcache->time_quantum * 100,
						llcache_persist_slowcheck,
						عدم);
				اكسر;
			} אחרת {
				אם (total_bandwidth > llcache->maximum_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					next = ((total_written * llcache->time_quantum) / write_limit) - total_elapsed;
				} אחרת {
					next = llcache->time_quantum;
				}
				اكسر;
			}
		} אחרת אם (total_written > write_limit) {
			/* The bandwidth limit has been reached. */

			אם (total_bandwidth > llcache->maximum_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				next = ((total_written * llcache->time_quantum) / write_limit) - total_elapsed;
			} אחרת {
				next = llcache->time_quantum - total_elapsed;
			}
			اكسر;
		}

	}
	حرر(lst);

	/* Completed list without running out of allowed bytes or time */
	אם (idx == lst_count) {
		/* only reschedule if writing is making any progress at all */
		אם (total_written > 0) {
			next = llcache->time_quantum - total_elapsed;
		} אחרת {
			next = -1;
		}
	}

	llcache->total_written += total_written;
	llcache->total_elapsed += total_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      total_written, total_elapsed, total_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", next);
	guit->misc->schedule(next, llcache_persist, عدم);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
ساكن فراغ llcache_fetch_callback(ثابت fetch_msg *msg, فراغ *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	אם (llcache == عدم) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		أكد(false);
		/* In case assertions are off, return here */
		החזר;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", msg->type, object);

	بدل (msg->type) {
	حالة FETCH_HEADER:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_HEADERS;

		error = llcache_fetch_process_header(object,
				msg->data.header_or_data.buf,
				msg->data.header_or_data.len);
		اكسر;

	/* 3xx responses */
	حالة FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		error = llcache_fetch_redirect(object,
				msg->data.redirect, &object);
		אם (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.data.error.code = error;
			event.data.error.msg = عدم;
			error = llcache_send_event_to_users(object, &event);
		}
		اكسر;

	حالة FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		اكسر;

	/* Normal 2xx state machine */
	حالة FETCH_DATA:
		/* Received some data */
		error = llcache_fetch_process_data(object,
				msg->data.header_or_data.buf,
				msg->data.header_or_data.len);
		اكسر;

	حالة FETCH_FINISHED:
		/* Finished fetching */
	{
		uint8_t *temp;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = عدم;

		/* Shrink source buffer to required size */
		temp = realloc(object->source_data,
				object->source_len);
		/* If source_len is 0, then temp may be NULL */
		אם (temp != عدم || object->source_len == 0) {
			object->source_data = temp;
			object->source_alloc = object->source_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_time = time(عدم);

		(فراغ) llcache_hsts_update_policy(object);

		guit->misc->schedule(5000, llcache_persist, عدم);
	}
		اكسر;

	/* Out-of-band information */
	حالة FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		اكسر;

	حالة FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = عدم;

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_data(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.msg = msg->data.error;

		error = llcache_send_event_to_users(object, &event);

		اكسر;

	حالة FETCH_PROGRESS:
		/* Progress update */
		event.type = LLCACHE_EVENT_PROGRESS;
		event.data.progress_msg = msg->data.progress;

		error = llcache_send_event_to_users(object, &event);

		اكسر;

	حالة FETCH_CERTS:
		/* Certificate information from the fetch */

		אם (object->chain != عدم) {
			cert_chain_free(object->chain);
			object->chain = عدم;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(msg->data.chain, &object->chain);
		אם (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.data.chain = msg->data.chain;

		error = llcache_send_event_to_users(object, &event);

		اكسر;

	/* Events requiring action */
	حالة FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		error = llcache_fetch_auth(object, msg->data.auth.realm);
		اكسر;

	حالة FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		error = llcache_fetch_cert_error(object);
		اكسر;

	حالة FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		אם (object->candidate != عدم) {
			object->candidate->candidate_count--;
			object->candidate = عدم;
		}

		error = llcache_fetch_ssl_error(object);
		اكسر;
	}

	/* Deal with any errors reported by event handlers */
	אם (error != NSERROR_OK) {
		אם (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		אם (object->fetch.fetch != عدم) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = عدم;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_data(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_up();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
ساكن llcache_object_user *llcache_object_find_user(
		ثابت llcache_handle *handle)
{
	llcache_object_user *user;

	أكد(handle->object != عدم);

	עבור (user = handle->object->users; user != عدم; user = user->next) {
		אם (user->handle == handle)
			اكسر;
	}

	החזר user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
ساكن bool llcache_object_in_list(ثابت llcache_object *object,
		ثابت llcache_object *list)
{
	כלעוד (list != عدم) {
		אם (list == object)
			اكسر;

		list = list->next;
	}

	החזר list != عدم;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *next_user;
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

	עבור (user = object->users; user != عدم; user = next_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ثابت llcache_fetch_state objstate = object->fetch.state;

		/* Flag that this user is the current iteration target
		 * in case the client attempts to destroy it underneath us */
		user->iterator_target = true;

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

		אם (handle->state != objstate) {
			אם (emitted_notify == false) {
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
		אם (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_HEADERS;

			/* Emit any certificate data we hold */
			אם (object->chain != عدم) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.data.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} אחרת {
				error = NSERROR_OK;
			}

			אם (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				אם (error != NSERROR_OK)
					החזר error;

				استمر;
			} אחרת אם (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				استمر;
			} אחרת אם (error != NSERROR_OK) {
				user->iterator_target = false;
				החזר error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		אם (handle->state == LLCACHE_FETCH_HEADERS &&
				objstate > LLCACHE_FETCH_HEADERS) {
			handle->state = LLCACHE_FETCH_DATA;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_HEADERS;

			error = handle->cb(handle, &event, handle->pw);

			אם (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				אם (error != NSERROR_OK)
					החזר error;

				استمر;
			} אחרת אם (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				استمر;
			} אחרת אם (error != NSERROR_OK) {
				user->iterator_target = false;
				החזר error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		אם (handle->state == LLCACHE_FETCH_DATA &&
				objstate >= LLCACHE_FETCH_DATA &&
				object->source_len > handle->bytes) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_DATA;
			event.data.data.buf =
					object->source_data + handle->bytes;
			event.data.data.len =
					object->source_len - handle->bytes;

			/* Update record of last byte emitted */
			אם (object->fetch.flags &
					LLCACHE_RETRIEVE_STREAM_DATA) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->bytes = object->source_len = 0;
			} אחרת {
				orig_handle_read = handle->bytes;
				handle->bytes = object->source_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			אם (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				אם (error != NSERROR_OK)
					החזר error;

				استمر;
			} אחרת אם (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->bytes = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				استمر;
			} אחרת אם (error != NSERROR_OK) {
				user->iterator_target = false;
				החזר error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		אם (handle->state == LLCACHE_FETCH_DATA &&
				objstate > LLCACHE_FETCH_DATA) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_DONE;

			error = handle->cb(handle, &event, handle->pw);
			אם (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				אם (error != NSERROR_OK)
					החזר error;

				استمر;
			} אחרת אם (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_DATA;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				استمر;
			} אחרת אם (error != NSERROR_OK) {
				user->iterator_target = false;
				החזר error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_target = false;

		next_user = user->next;
	}

	החזר NSERROR_OK;
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
ساكن nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	אם (error != NSERROR_OK)
		החזר error;

	newobj->source_alloc = newobj->source_len = object->source_len;

	אם (object->source_len > 0) {
		newobj->source_data = malloc(newobj->source_alloc);
		אם (newobj->source_data == عدم) {
			llcache_object_destroy(newobj);
			החזר NSERROR_NOMEM;
		}
		memcpy(newobj->source_data, object->source_data,
				newobj->source_len);
	}

	אם (object->num_headers > 0) {
		newobj->headers = calloc(object->num_headers,
				حجم(llcache_header));
		אם (newobj->headers == عدم) {
			llcache_object_destroy(newobj);
			החזר NSERROR_NOMEM;
		}
		כלעוד (newobj->num_headers < object->num_headers) {
			llcache_header *nh =
					&(newobj->headers[newobj->num_headers]);
			llcache_header *oh =
					&(object->headers[newobj->num_headers]);
			newobj->num_headers += 1;
			nh->name = strdup(oh->name);
			nh->value = strdup(oh->value);
			אם (nh->name == عدم || nh->value == عدم) {
				llcache_object_destroy(newobj);
				החזר NSERROR_NOMEM;
			}
		}
	}

	אם (object->chain != عدم) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		אם (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			החזר error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	החזר NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
ساكن مضمن uint32_t
total_object_size(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = حجم(*object);
	tot += nsurl_length(object->url);

	אם (object->source_data != عدم) {
		tot += object->source_len;
	}

	tot += حجم(llcache_header) * object->num_headers;

	עבור (hdrc = 0; hdrc < object->num_headers; hdrc++) {
		אם (object->headers[hdrc].name != عدم) {
			tot += strlen(object->headers[hdrc].name);
		}
		אם (object->headers[hdrc].value != عدم) {
			tot += strlen(object->headers[hdrc].value);
		}
	}

	tot += cert_chain_size(object->chain);

	החזר tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
ساكن فراغ llcache_catch_up_all_users(فراغ *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_up = true;

	/* Catch new users up with state of objects */
	עבור (object = llcache->cached_objects; object != عدم;
			object = object->next) {
		llcache_object_notify_users(object);
	}

	עבור (object = llcache->uncached_objects; object != عدم;
			object = object->next) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
ساكن فراغ llcache_users_not_caught_up(فراغ)
{
	אם (llcache->all_caught_up) {
		llcache->all_caught_up = false;
		guit->misc->schedule(0, llcache_catch_up_all_users, عدم);
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
فراغ llcache_clean(bool purge)
{
	llcache_object *object, *next;
	uint32_t llcache_size = 0;
	صحيح remaining_lifetime;
	uint32_t limit;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	אם (purge) {
		limit = 0;
	} אחרת {
		limit = llcache->limit;
	}

	/* Uncacheable objects with no users or fetches */
	עבור (object = llcache->uncached_objects;
	     object != عدم;
	     object = next) {
		next = object->next;

		/* The candidate count of uncacheable objects is always 0 */
		אם ((object->users == عدم) &&
		    (object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} אחרת {
			llcache_size += total_object_size(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	עבור (object = llcache->cached_objects;
	     object != عدم;
	     object = next) {
		next = object->next;

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		אם ((object->users == عدم) &&
		    (object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم) &&
		    (remaining_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				אם (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} אחרת {
			/* object has users so account for the storage */
			llcache_size += total_object_size(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	אם (limit < llcache_size) {
		llcache_persist(عدم);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	עבור (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != عدم));
	     object = next) {
		next = object->next;
		אם ((object->users == عدم) &&
		    (object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->source_data = عدم;

			llcache_size -=	object->source_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->source_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	עבור (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != عدم));
	     object = next) {
		next = object->next;
		אם ((object->users == عدم) &&
		    (object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->source_data == عدم)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (long)(time(عدم) - object->last_used),
			      object,
			      nsurl_access(object->url));

			llcache_size -=	total_object_size(object);

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
	עבור (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != عدم));
	     object = next) {
		next = object->next;

		אם ((object->users == عدم) &&
		    (object->candidate_count == 0) &&
		    (object->fetch.fetch == عدم) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (long)(time(عدم) - object->last_used),
			      object,
			      nsurl_access(object->url));

			llcache_size -=	object->source_len + حجم(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_size, limit);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ثابت بنية llcache_parameters *prm)
{
	llcache = calloc(1, حجم(بنية llcache_s));
	אם (llcache == عدم) {
		החזר NSERROR_NOMEM;
	}

	llcache->limit = prm->limit;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->maximum_bandwidth = prm->maximum_bandwidth;
	llcache->time_quantum = prm->time_quantum;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_up = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->limit);

	/* backing store initialisation */
	החזר guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
فراغ llcache_finalise(فراغ)
{
	llcache_object *object, *next;
	uint64_t total_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(عدم);
	/* Now clear the persistence callback */
	guit->misc->schedule(-1, llcache_persist, عدم);

	/* Clean uncached objects */
	עבור (object = llcache->uncached_objects; object != عدم; object = next) {
		llcache_object_user *user, *next_user;

		next = object->next;

		עבור (user = object->users; user != عدم; user = next_user) {
			next_user = user->next;

			אם (user->handle != عدم)
				حرر(user->handle);

			حرر(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = عدم;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	עבור (object = llcache->cached_objects; object != عدم; object = next) {
		llcache_object_user *user, *next_user;

		next = object->next;

		עבור (user = object->users; user != عدم; user = next_user) {
			next_user = user->next;

			אם (user->handle != عدم)
			       حرر(user->handle);

			حرر(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = عدم;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	אם (llcache->total_elapsed > 0) {
		total_bandwidth = (llcache->total_written * 1000) /
			llcache->total_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->total_written,
	      llcache->total_elapsed,
	      total_bandwidth);

	حرر(llcache);
	llcache = عدم;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ثابت llcache_post_data *post,
			llcache_handle_callback cb, فراغ *pw,
			llcache_handle **result)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_in_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	אם (error != NSERROR_OK) {
		החזר error;
	}

	/* Can we fetch this URL at all? */
	אם (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		החזר NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	אם (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		החזר error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_in_use, &object);
	אם (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		החזר error;
	}

	/* Add user to object */
	llcache_object_add_user(object, user);

	*result = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_up();

	nsurl_unref(hsts_url);

	החזר NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, فراغ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	החזר NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_find_user(handle);

	أكد(user != عدم);

	אם (user->iterator_target) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} אחרת {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		אם (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	החזר error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **result)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	אם (error == NSERROR_OK) {
		llcache_object_add_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*result = newuser->handle;
	}

	החזר error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	אם (user->prev != عدم)
		all_alone = false;
	אם (user->next != عدم)
		all_alone = false;

	אם (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		אם (error != NSERROR_OK)
			החזר error;

		/* Move across to the new object */
		אם (user->iterator_target) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					calloc(1, حجم(llcache_object_user));
			אם (newuser == عدم) {
				llcache_object_destroy(newobject);
				החזר NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = عدم;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_add_user(newobject, newuser);
		} אחרת {
			llcache_object_remove_user(object, user);
			llcache_object_add_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(newobject,
				&llcache->uncached_objects);
	} אחרת {
		/* We're the only user, so abort any fetch in progress */
		אם (object->fetch.fetch != عدم) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = عدم;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_data(object);
	}

	החזר error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_stream(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	אם (user->prev != عدم || user->next != عدم)
		החזר NSERROR_OK;

	/* Forcibly uncache this object */
	אם (llcache_object_in_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_add_to_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_STREAM_DATA;

	החזר NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_data(llcache_handle *handle)
{
	אם ((handle->object != عدم) &&
	    (handle->object->fetch.fetch == عدم) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	החזר NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ثابت llcache_handle *handle)
{
	החזר handle->object != عدم ? handle->object->url : عدم;
}

/* See llcache.h for documentation */
ثابت uint8_t *llcache_handle_get_source_data(ثابت llcache_handle *handle,
		size_t *size)
{
	*size = handle->object != عدم ? handle->object->source_len : 0;

	החזר handle->object != عدم ? handle->object->source_data : عدم;
}

/* See llcache.h for documentation */
ثابت حرف *llcache_handle_get_header(ثابت llcache_handle *handle,
		ثابت حرف *key)
{
	ثابت llcache_object *object = handle->object;
	size_t i;

	אם (object == عدم)
		החזר عدم;

	/* About as trivial as possible */
	עבור (i = 0; i < object->num_headers; i++) {
		אם (strcasecmp(key, object->headers[i].name) == 0)
			החזר object->headers[i].value;
	}

	החזר عدم;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ثابت llcache_handle *a,
		ثابت llcache_handle *b)
{
	החזר a->object == b->object;
}
