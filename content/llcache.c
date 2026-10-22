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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/base64.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/time.h"
#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/backing_store.h"
#ଅନ୍ତର୍ଭାଵ "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_HEADERS,		/**< Fetching headers */
	LLCACHE_FETCH_DATA,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
ସଂରଚନା llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	ଶୂନ୍ୟ *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t bytes;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_target;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	ସଂରଚନା llcache_object_user *prev;	/**< Previous in list */
	ସଂରଚନା llcache_object_user *next;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_data *post;	/**< POST data, or NULL for GET */

	ସଂରଚନା fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_ଗଣନା;	/**< Count of redirects followed */

	uint32_t retries_remaining;     /**< Number of times to retry on timeout */

	bool hsts_in_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#define INVALID_AGE -1

/** Cache control data */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
	time_t req_time;	/**< Time of request */
	time_t res_time;	/**< Time of response */
	time_t fin_time;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା age;		/**< Age: response header */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	ଅକ୍ଷର *etag;		/**< Etag: response header */
	time_t last_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
	ଅକ୍ଷର *name;		/**< Header name */
	ଅକ୍ଷର *ମୂଲ୍ୟ;		/**< Header value */
} llcache_header;

/** Current status of an object's data */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
ସଂରଚନା llcache_object {
	llcache_object *prev;	     /**< Previous in list */
	llcache_object *next;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *source_data;	     /**< Source data for object */
	size_t source_len;	     /**< Byte length of source data */
	size_t source_alloc;	     /**< Allocated size of source buffer */

	ସଂରଚନା cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_ଗଣନା;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_header *headers;     /**< Fetch headers */
	size_t ଅଙ୍କ_headers;	     /**< Number of fetch headers */

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
ସଂରଚନା llcache_s {
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
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା minimum_lifetime;

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
ସ୍ଥିର ସଂରଚନା llcache_s *llcache = ଖାଲି;

/* forward referenced callback function */
ସ୍ଥିର ଶୂନ୍ୟ llcache_fetch_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ, ଶୂନ୍ୟ *p);

/* forward referenced catch up function */
ସ୍ଥିର ଶୂନ୍ୟ llcache_users_not_caught_up(ଶୂନ୍ୟ);


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
ସ୍ଥିର nserror llcache_object_user_new(llcache_handle_callback cb, ଶୂନ୍ୟ *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = calloc(1, ଆକାର(llcache_handle));
	ଯଦି (h == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	u = calloc(1, ଆକାର(llcache_object_user));
	ଯଦି (u == ଖାଲି) {
		ମୁକ୍ତ(h);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (ଶୂନ୍ୟ *) cb, pw);

	*user = u;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
ସ୍ଥିର nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	ପ୍ରତିଜ୍ଞା(user->next == ଖାଲି);
	ପ୍ରତିଜ୍ଞା(user->prev == ଖାଲି);

	ଯଦି (user->handle != ଖାଲି)
		ମୁକ୍ତ(user->handle);

	ମୁକ୍ତ(user);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
ସ୍ଥିର nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	ପ୍ରତିଜ୍ଞା(user != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(object != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(object->users != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(user->handle == ଖାଲି || user->handle->object == object);
	ପ୍ରତିଜ୍ଞା((user->prev != ଖାଲି) || (object->users == user));

	ଯଦି (user == object->users)
		object->users = user->next;
	ଅନ୍ୟଥା
		user->prev->next = user->next;

	ଯଦି (user->next != ଖାଲି)
		user->next->prev = user->prev;

	user->next = user->prev = ଖାଲି;

	/* record the time the last user was removed from the object */
	ଯଦି (object->users == ଖାଲି) {
		object->last_used = time(ଖାଲି);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ସ୍ଥିର nserror llcache_send_event_to_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *next_user;

	user = object->users;
	ଯେତେବେଳେ (user != ଖାଲି) {
		bool was_target = user->iterator_target;
		user->iterator_target = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		next_user = user->next;

		user->iterator_target = was_target;

		ଯଦି (user->queued_for_delete && !was_target) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		ଯଦି (error != NSERROR_OK)
			ଭାଙ୍ଗ;

		user = next_user;
	}

	ଫେରନ୍ତୁ error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_object_new(nsurl *url, llcache_object **result)
{
	llcache_object *obj = calloc(1, ଆକାର(llcache_object));
	ଯଦି (obj == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*result = obj;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_post_data_clone(ଅଚଳ llcache_post_data *orig,
		llcache_post_data **clone)
{
	llcache_post_data *post_clone;

	post_clone = calloc(1, ଆକାର(llcache_post_data));
	ଯଦି (post_clone == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	ଯଦି (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->data.urlenc = strdup(orig->data.urlenc);
		ଯଦି (post_clone->data.urlenc == ଖାଲି) {
			ମୁକ୍ତ(post_clone);

			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	} ଅନ୍ୟଥା {
		post_clone->data.multipart = fetch_multipart_data_clone(
				orig->data.multipart);
		ଯଦି (post_clone->data.multipart == ଖାଲି) {
			ମୁକ୍ତ(post_clone);

			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror llcache_fetch_split_header(ଅଚଳ uint8_t *data, size_t len,
		ଅକ୍ଷର **name, ଅକ୍ଷର **ମୂଲ୍ୟ)
{
	ଅକ୍ଷର *n, *v;
	ଅଚଳ uint8_t *colon;

	/* Strip leading whitespace from name */
	ଯେତେବେଳେ (data[0] == ' ' || data[0] == '\t' ||
	       data[0] == '\r' || data[0] == '\n') {
		data++;
	}

	/* Find colon */
	colon = (ଅଚଳ uint8_t *) strchr((ଅଚଳ ଅକ୍ଷର *) data, ':');
	ଯଦି (colon == ଖାଲି) {
		/* Failed, assume a key with no value */
		colon = data + strlen((ଅଚଳ ଅକ୍ଷର *)data);

		/* Strip trailing whitespace from name */
		ଯେତେବେଳେ ((colon > data) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ଅଚଳ ଅକ୍ଷର *) data, colon - data);
		ଯଦି (n == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		v = strdup("");
		ଯଦି (v == ଖାଲି) {
			ମୁକ୍ତ(n);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	} ଅନ୍ୟଥା {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		ଯେତେବେଳେ (colon > data && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ଅଚଳ ଅକ୍ଷର *) data, colon - data);
		ଯଦି (n == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		/* Find colon again */
		ଯେତେବେଳେ (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		କର {
			colon++;
		} ଯେତେବେଳେ (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		ଯେତେବେଳେ (len > 0 && (data[len - 1] == ' ' ||
				data[len - 1] == '\t' ||
				data[len - 1] == '\r' ||
				data[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ଅଚଳ ଅକ୍ଷର *) colon, len - (colon - data));
		ଯଦି (v == ଖାଲି) {
			ମୁକ୍ତ(n);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	*name = n;
	*ମୂଲ୍ୟ = v;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror
llcache_fetch_parse_cache_control(llcache_object *object, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(ମୂଲ୍ୟ, &cc);
	ଯଦି (error != NSERROR_OK) {
		/* Ignore parse errors */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	ଯଦି (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror
llcache_fetch_header_cache_control(llcache_object *object,
				   ଅକ୍ଷର *name,
				   ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = strlen(name);

	ବାଛ (name_len) {
	ସ୍ଥିତି 3:
		ଯଦି (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			ଯଦି ('0' <= *ମୂଲ୍ୟ && *ମୂଲ୍ୟ <= '9') {
				object->cache.age = atoi(ମୂଲ୍ୟ);
			}

		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି 4:
		ଯଦି (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(ମୂଲ୍ୟ,
					    strlen(ମୂଲ୍ୟ),
					    &object->cache.date);
			ଯଦି (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      ମୂଲ୍ୟ, res);
			}
		} ଅନ୍ୟଥା ଯଦି (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			ମୁକ୍ତ(object->cache.etag);
			object->cache.etag = strdup(ମୂଲ୍ୟ);
			ଯଦି (object->cache.etag == ଖାଲି) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି 7:
		ଯଦି (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(ମୂଲ୍ୟ,
					    strlen(ମୂଲ୍ୟ),
					    &object->cache.expires);
			ଯଦି (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      ମୂଲ୍ୟ, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି 13:
		ଯଦି (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,ମୂଲ୍ୟ);
		} ଅନ୍ୟଥା ଯଦି (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(ମୂଲ୍ୟ,
				      strlen(ମୂଲ୍ୟ),
				      &object->cache.last_modified);
		}
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ llcache_destroy_headers(llcache_object *object)
{
	ଯେତେବେଳେ (object->ଅଙ୍କ_headers > 0) {
		object->ଅଙ୍କ_headers--;

		ମୁକ୍ତ(object->headers[object->ଅଙ୍କ_headers].name);
		ମୁକ୍ତ(object->headers[object->ଅଙ୍କ_headers].ମୂଲ୍ୟ);
	}
	ମୁକ୍ତ(object->headers);
	object->headers = ଖାଲି;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ llcache_invalidate_cache_control_data(llcache_object *object)
{
	ମୁକ୍ତ(object->cache.etag);
	memset(&(object->cache), 0, ଆକାର(llcache_cache_control));

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
ସ୍ଥିର nserror llcache_fetch_process_header(llcache_object *object,
		ଅଚଳ uint8_t *data, size_t len)
{
	nserror res;
	ଅକ୍ଷର *name, *ମୂଲ୍ୟ;
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
	ଯଦି (strncmp((ଅଚଳ ଅକ୍ଷର *) data, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_time = object->cache.req_time;

		llcache_invalidate_cache_control_data(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_time = req_time;

		llcache_destroy_headers(object);
	}

	/* Set fetch response time if not already set */
	ଯଦି (object->cache.res_time == 0) {
		object->cache.res_time = time(ଖାଲି);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_split_header(data, len, &name, &ମୂଲ୍ୟ);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	/* deal with empty header */
	ଯଦି (name[0] == 0) {
		ମୁକ୍ତ(name);
		ମୁକ୍ତ(ମୂଲ୍ୟ);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_header_cache_control(object, name, ମୂଲ୍ୟ);
	ଯଦି (res != NSERROR_OK) {
		ମୁକ୍ତ(name);
		ମୁକ୍ତ(ମୂଲ୍ୟ);
		ଫେରନ୍ତୁ res;
	}

	/* Append header data to the object's headers array */
	temp = realloc(object->headers,
		       (object->ଅଙ୍କ_headers + 1) * ଆକାର(llcache_header));
	ଯଦି (temp == ଖାଲି) {
		ମୁକ୍ତ(name);
		ମୁକ୍ତ(ମୂଲ୍ୟ);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	object->headers = temp;

	object->headers[object->ଅଙ୍କ_headers].name = name;
	object->headers[object->ଅଙ୍କ_headers].ମୂଲ୍ୟ = ମୂଲ୍ୟ;

	object->ଅଙ୍କ_headers++;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror get_referer_header(nsurl *url, nsurl *referer, ଅକ୍ଷର **header_out)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool match;
	bool match1;
	bool match2;
	ଅକ୍ଷର *header;

	/* Determine whether to send the Referer header */
	ଯଦି (!nsoption_bool(send_referer)) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (scheme == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	ଯଦି (ref_scheme == ଖାଲି) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ NSERROR_INVALID;
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
	ଯଦି (lwc_string_isequal(scheme, ref_scheme,
			       &match) != lwc_error_ok) {
		match = false;
	}
	ଯଦି (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	ଯଦି (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	ଯଦି (match == true || (match1 == true && match2 == true)) {
		ଅଚଳ size_t len = SLEN("Referer: ") +
			nsurl_length(referer) + 1;

		header = malloc(len);
		ଯଦି (header == ଖାଲି) {
			res = NSERROR_NOMEM;
		} ଅନ୍ୟଥା {
			snprintf(header, len, "Referer: %s",
				 nsurl_access(referer));

			*header_out = header;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror llcache_object_refetch(llcache_object *object)
{
	ଅଚଳ ଅକ୍ଷର *urlenc = ଖାଲି;
	ସଂରଚନା fetch_multipart_data *multipart = ଖାଲି;
	ଅକ୍ଷର **headers = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_idx = 0;
	nserror res;

	ଯଦି (object->fetch.post != ଖାଲି) {
		ଯଦି (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->data.urlenc;
		} ଅନ୍ୟଥା {
			multipart = object->fetch.post->data.multipart;
		}
	}

	/* Generate headers */
	headers = malloc(4 * ଆକାର(ଅକ୍ଷର *));
	ଯଦି (headers == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	ଯଦି (object->cache.etag != ଖାଲି) {
		ଅଚଳ size_t len = SLEN("If-None-Match: ") +
				strlen(object->cache.etag) + 1;

		headers[header_idx] = malloc(len);
		ଯଦି (headers[header_idx] == ଖାଲି) {
			ମୁକ୍ତ(headers);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-None-Match: %s",
				object->cache.etag);

		header_idx++;
	}

	/* cache-control header for modification time */
	ଯଦି (object->cache.last_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ଅଚଳ size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		headers[header_idx] = malloc(len);
		ଯଦି (headers[header_idx] == ଖାଲି) {
			ଯେତେବେଳେ (--header_idx >= 0)
				ମୁକ୍ତ(headers[header_idx]);
			ମୁକ୍ତ(headers);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.last_modified));

		header_idx++;
	}

	/* Referer header */
	ଯଦି (object->fetch.referer != ଖାଲି) {
		ଯଦି (get_referer_header(object->url,
				       object->fetch.referer,
				       &headers[header_idx]) == NSERROR_OK) {
			header_idx++;
		}
	}
	headers[header_idx] = ଖାଲି;

	/* Reset cache control data */
	llcache_invalidate_cache_control_data(object);
	object->cache.req_time = time(ଖାଲି);
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
			  (ଅଚଳ ଅକ୍ଷର **)headers,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	ଯେତେବେଳେ (--header_idx >= 0) {
		ମୁକ୍ତ(headers[header_idx]);
	}
	ମୁକ୍ତ(headers);

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ଅଚଳ llcache_post_data *post,
		uint32_t redirect_ଗଣନା, bool hsts_in_use)
{
	nserror error;
	nsurl *referer_clone = ଖାଲି;
	llcache_post_data *post_clone = ଖାଲି;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	ଯଦି (post != ଖାଲି) {
		error = llcache_post_data_clone(post, &post_clone);
		ଯଦି (error != NSERROR_OK)
			ଫେରନ୍ତୁ error;
	}

	ଯଦି (referer != ଖାଲି)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ଗଣନା = redirect_ଗଣନା;
	object->fetch.retries_remaining = llcache->fetch_attempts;
	object->fetch.hsts_in_use = hsts_in_use;

	ଫେରନ୍ତୁ llcache_object_refetch(object);
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
ସ୍ଥିର nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	ଯଦି (object->source_data != ଖାଲି) {
		ଯଦି (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} ଅନ୍ୟଥା {
			ମୁକ୍ତ(object->source_data);
		}
	}

	nsurl_unref(object->url);

	ଯଦି (object->fetch.fetch != ଖାଲି) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = ଖାଲି;
	}

	ଯଦି (object->fetch.referer != ଖାଲି)
		nsurl_unref(object->fetch.referer);

	ଯଦି (object->fetch.post != ଖାଲି) {
		ଯଦି (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			ମୁକ୍ତ(object->fetch.post->data.urlenc);
		} ଅନ୍ୟଥା {
			fetch_multipart_data_destroy(
					object->fetch.post->data.multipart);
		}

		ମୁକ୍ତ(object->fetch.post);
	}

	ମୁକ୍ତ(object->cache.etag);

	ପାଇଁ (i = 0; i < object->ଅଙ୍କ_headers; i++) {
		ମୁକ୍ତ(object->headers[i].name);
		ମୁକ୍ତ(object->headers[i].ମୂଲ୍ୟ);
	}
	ମୁକ୍ତ(object->headers);

	ମୁକ୍ତ(object);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
ସ୍ଥିର nserror llcache_object_add_to_list(llcache_object *object,
		llcache_object **list)
{
	object->prev = ଖାଲି;
	object->next = *list;

	ଯଦି (*list != ଖାଲି)
		(*list)->prev = object;
	*list = object;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
llcache_object_rfc2616_remaining_lifetime(ଅଚଳ llcache_cache_control *cd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା current_age, freshness_lifetime;
	time_t now = time(ଖାଲି);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_time - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_time - cd->req_time + now - cd->res_time;

	/* Determine freshness lifetime of this object */
	ଯଦି (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} ଅନ୍ୟଥା ଯଦି (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} ଅନ୍ୟଥା ଯଦି (cd->last_modified != 0) {
		freshness_lifetime = (now - cd->last_modified) / 10;
	} ଅନ୍ୟଥା {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	ଯଦି ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		ଫେରନ୍ତୁ freshness_lifetime - current_age;
	}

	ଫେରନ୍ତୁ 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
ସ୍ଥିର bool llcache_object_is_fresh(ଅଚଳ llcache_object *object)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_lifetime;
	ଅଚଳ llcache_cache_control *cd = &object->cache;

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
	ଫେରନ୍ତୁ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
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
ସ୍ଥିର nserror llcache_object_clone_cache_data(llcache_object *source,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	ଯଦି (source->cache.etag != ଖାଲି) {
		ଅକ୍ଷର *etag = source->cache.etag;

		ଯଦି (deep) {
			/* Copy the etag */
			etag = strdup(source->cache.etag);
			ଯଦି (etag == ଖାଲି)
				ଫେରନ୍ତୁ NSERROR_NOMEM;
		} ଅନ୍ୟଥା {
			/* Destination takes ownership */
			source->cache.etag = ଖାଲି;
		}

		ଯଦି (destination->cache.etag != ଖାଲି)
			ମୁକ୍ତ(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_time = source->cache.req_time;
	destination->cache.res_time = source->cache.res_time;
	destination->cache.fin_time = source->cache.fin_time;

	ଯଦି (source->cache.date != 0)
		destination->cache.date = source->cache.date;

	ଯଦି (source->cache.expires != 0)
		destination->cache.expires = source->cache.expires;

	ଯଦି (source->cache.age != INVALID_AGE)
		destination->cache.age = source->cache.age;

	ଯଦି (source->cache.max_age != INVALID_AGE)
		destination->cache.max_age = source->cache.max_age;

	ଯଦି (source->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = source->cache.no_cache;

	ଯଦି (source->cache.last_modified != 0)
		destination->cache.last_modified = source->cache.last_modified;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
ସ୍ଥିର nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	ଯଦି (object == *list)
		*list = object->next;
	ଅନ୍ୟଥା
		object->prev->next = object->next;

	ଯଦି (object->next != ଖାଲି)
		object->next->prev = object->prev;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror llcache_retrieve_persisted_data(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	ଯଦି ((object->source_data != ଖାଲି) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	ଫେରନ୍ତୁ guit->llcache->fetch(object->url,
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
ସ୍ଥିର nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **data_out,
			   size_t *datasize_out)
{
	size_t allocsize;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା datasize;
	uint8_t *data;
	ଅକ୍ଷର *op;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା hloop;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା use;
	size_t cert_chain_depth;

	ଯଦି (object->chain != ଖାଲି) {
		cert_chain_depth = object->chain->depth;
	} ଅନ୍ୟଥା {
		cert_chain_depth = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	ପାଇଁ (hloop = 0 ; hloop < object->ଅଙ୍କ_headers ; hloop++) {
		allocsize += strlen(object->headers[hloop].name) + 1;
		allocsize += strlen(object->headers[hloop].ମୂଲ୍ୟ) + 1;
	}

	allocsize += nsurl_length(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	ପାଇଁ (hloop = 0; hloop < cert_chain_depth; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_length + 2) / 3);
	}

	data = malloc(allocsize);
	ଯଦି (data == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	op = (ଅକ୍ଷର *)data;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	ଯଦି (use < 0) {
		ଯାଅ operror;
	}
	use++; /* does not count the null */
	ଯଦି (use > datasize) {
		ଯାଅ overflow;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->source_len);
	ଯଦି (use < 0) {
		ଯାଅ operror;
	}
	use++; /* does not count the null */
	ଯଦି (use > datasize)
		ଯାଅ overflow;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_time);
	ଯଦି (use == 0)
		ଯାଅ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_time);
	ଯଦି (use == 0)
		ଯାଅ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_time);
	ଯଦି (use == 0)
		ଯାଅ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->ଅଙ୍କ_headers);
	ଯଦି (use < 0) {
		ଯାଅ operror;
	}
	use++; /* does not count the null */
	ଯଦି (use > datasize)
		ଯାଅ overflow;
	op += use;
	datasize -= use;

	/* headers */
	ପାଇଁ (hloop = 0 ; hloop < object->ଅଙ୍କ_headers ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->headers[hloop].name,
			       object->headers[hloop].ମୂଲ୍ୟ);
		ଯଦି (use < 0) {
			ଯାଅ operror;
		}
		use++; /* does not count the null */
		ଯଦି (use > datasize)
			ଯାଅ overflow;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_depth);
	ଯଦି (use < 0) {
		ଯାଅ operror;
	}
	use++; /* does not count the null */
	ଯଦି (use > datasize)
		ଯାଅ overflow;
	op += use;
	datasize -= use;

	/* SSL certificates */
	ପାଇଁ (hloop = 0; hloop < cert_chain_depth; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(object->chain->certs[hloop].err));
		ଯଦି (use < 0) {
			ଯାଅ operror;
		}
		use++; /* does not count the null */
		ଯଦି (use > datasize)
			ଯାଅ overflow;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		ଯଦି (object->chain->certs[hloop].der != ଖାଲି) {
			size_t output_length = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_length,
				(uint8_t *)op,
				&output_length);
			ଯଦି (res != NSUERROR_OK) {
				ଯାଅ operror;
			}
			use = output_length;
		} ଅନ୍ୟଥା {
			use = 0;
		}
		use++; /* allow for null */
		ଯଦି (use > datasize)
			ଯାଅ overflow;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*data_out = data;
	*datasize_out = allocsize - datasize;

	ଫେରନ୍ତୁ NSERROR_OK;

overflow:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	ମୁକ୍ତ(data);
	ଫେରନ୍ତୁ NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	ମୁକ୍ତ(data);
	ଫେରନ୍ତୁ NSERROR_INVALID;
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
ସ୍ଥିର nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = ଖାଲି;
	size_t metadatalen = 0;
	size_t remaining = 0;
	nsurl *metadataurl;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି;
	ଅକ୍ଷର *ln;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lnsize;

	size_t source_length;
	time_t request_time;
	time_t response_time;
	time_t completion_time;
	size_t ଅଙ୍କ_headers;
	size_t hloop;
	size_t ssl_cert_ଗଣନା = 0;
	ସଂରଚନା cert_chain *chain = ଖାଲି;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
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
	ଧାଡ଼ି = 1;
	ln = (ଅକ୍ଷର *)metadata;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	ଯଦି (lnsize < 7) {
		res = NSERROR_INVALID;
		ଯାଅ format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	ଯଦି (res != NSERROR_OK)
		ଯାଅ format_error;

	ଯଦି (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		ଫେରନ୍ତୁ NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	ଧାଡ଼ି = 2;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	ଯଦି ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &source_length) != 1)) {
		res = NSERROR_INVALID;
		ଯାଅ format_error;
	}


	/* metadata line 3 is the time of request */
	ଧାଡ଼ି = 3;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_time);
	ଯଦି (res != NSERROR_OK)
		ଯାଅ format_error;


	/* metadata line 4 is the time of response */
	ଧାଡ଼ି = 4;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_time);
	ଯଦି (res != NSERROR_OK)
		ଯାଅ format_error;


	/* metadata line 5 is the time of request completion */
	ଧାଡ଼ି = 5;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_time);
	ଯଦି (res != NSERROR_OK)
		ଯାଅ format_error;


	/* metadata line 6 is the number of headers */
	ଧାଡ଼ି = 6;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	ଯଦି ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &ଅଙ୍କ_headers) != 1)) {
		res = NSERROR_INVALID;
		ଯାଅ format_error;
	}

	/* read headers */
	ପାଇଁ (hloop = 0 ; hloop < ଅଙ୍କ_headers; hloop++) {
		ଧାଡ଼ି++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;

		res = llcache_fetch_process_header(object,
						   (uint8_t *)ln,
						   lnsize);
		ଯଦି (res != NSERROR_OK)
			ଯାଅ format_error;
	}

	ଯଦି (remaining == 0) {
		ଯାଅ skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	ଧାଡ଼ି++;
	ln += lnsize + 1;
	lnsize = strlen(ln);
	remaining -= lnsize + 1;

	ଯଦି ((lnsize < 1) || (sscanf(ln, "%" PRIsizet, &ssl_cert_ଗଣନା) != 1)) {
		res = NSERROR_INVALID;
		ଯାଅ format_error;
	}

	ଯଦି (ssl_cert_ଗଣନା == 0) {
		ଯାଅ skip_ssl_certificates;
	}

	ଯଦି (ssl_cert_ଗଣନା > MAX_CERT_DEPTH) {
		res = NSERROR_INVALID;
		ଯାଅ format_error;
	}

	res = cert_chain_alloc(ssl_cert_ଗଣନା, &chain);
	ଯଦି (res != NSERROR_OK) {
		ଯାଅ format_error;
	}

	ପାଇଁ (hloop = 0; hloop < ssl_cert_ଗଣନା; hloop++) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା errcode;
		nsuerror nsures;

		/* Certificate error code */
		ଧାଡ଼ି++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;
		ଯଦି ((lnsize < 1) || (sscanf(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			ଯାଅ format_error;
		}
		ଯଦି (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_KNOWN) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} ଅନ୍ୟଥା {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		ଧାଡ଼ି++;
		ln += lnsize + 1;
		lnsize = strlen(ln);
		remaining -= lnsize + 1;
		ଯଦି (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ଅଚଳ uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_length);
			ଯଦି (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				ଯାଅ format_error;
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

	ଫେରନ୍ତୁ NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      ଧାଡ଼ି, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	ଫେରନ୍ତୁ res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool llcache__scheme_is_persistable(ଅଚଳ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ଯଦି ((lwc_string_isequal(scheme, corestring_lwc_http,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&match) == lwc_error_ok &&
			(match == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	ଫେରନ୍ତୁ persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool llcache__scheme_is_cachable(ଅଚଳ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ଯଦି ((lwc_string_isequal(scheme, corestring_lwc_http,
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

	ଫେରନ୍ତୁ cachable;
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
ସ୍ଥିର nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ଅଚଳ llcache_post_data *post,
				uint32_t redirect_ଗଣନା)
{
	nserror error;
	nsurl *referer_clone = ଖାଲି;
	llcache_post_data *post_clone = ଖାଲି;

	ଯଦି (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	object->cache.req_time = time(ଖାଲି);
	object->cache.fin_time = object->cache.req_time;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	ଯଦି (error != NSERROR_OK) {
		ଫେରନ୍ତୁ error;
	}

	/* entry came out of cache - need to setup object state */
	ଯଦି (post != ଖାଲି) {
		error = llcache_post_data_clone(post, &post_clone);
		ଯଦି (error != NSERROR_OK)
			ଫେରନ୍ତୁ error;
	}

	ଯଦି (referer != ଖାଲି) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ଗଣନା = redirect_ଗଣନା;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ଖାଲି;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ଅଚଳ llcache_post_data *post,
				   uint32_t redirect_ଗଣନା,
				   bool hsts_in_use,
				   llcache_object **result)
{
	nserror error;
	llcache_object *obj, *newest = ଖାଲି;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==ଖାଲି?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	ପାଇଁ (obj = llcache->cached_objects; obj != ଖାଲି; obj = obj->next) {

		ଯଦି ((newest == ଖାଲି ||
		     obj->cache.req_time > newest->cache.req_time) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	ଯଦି (newest == ଖାଲି) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		ଯଦି (error != NSERROR_OK)
			ଫେରନ୍ତୁ error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_ଗଣନା);
		ଯଦି (error == NSERROR_OK) {
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

	ଯଦି ((newest != ଖାଲି) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_data(newest);
		ଯଦି (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*result = newest;

			ଫେରନ୍ତୁ NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		ଯଦି (error != NSERROR_OK) {
			ଫେରନ୍ତୁ error;
		}
	} ଅନ୍ୟଥା ଯଦି (newest != ଖାଲି) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_data(newest);
		ଯଦି (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			ଯଦି (error != NSERROR_OK)
				ଫେରନ୍ତୁ error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_data(newest, obj, true);
			ଯଦି (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				ଫେରନ୍ତୁ error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_ଗଣନା++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_ଗଣନା, hsts_in_use);
			ଯଦି (error != NSERROR_OK) {
				newest->candidate_ଗଣନା--;
				llcache_object_destroy(obj);
				ଫେରନ୍ତୁ error;
			}

			/* Add new object to cache */
			llcache_object_add_to_list(obj, &llcache->cached_objects);

			*result = obj;

			ଫେରନ୍ତୁ NSERROR_OK;
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
		ଯଦି (error != NSERROR_OK) {
			ଫେରନ୍ତୁ error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_ଗଣନା, hsts_in_use);
	ଯଦି (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		ଫେରନ୍ତୁ error;
	}

	/* Add new object to cache */
	llcache_object_add_to_list(obj, &llcache->cached_objects);

	*result = obj;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ଅଚଳ llcache_post_data *post,
			uint32_t redirect_ଗଣନା,
			bool hsts_in_use,
			llcache_object **result)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==ଖାଲି?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	/* determine if content is cachable */
	ଯଦି ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} ଅନ୍ୟଥା ଯଦି (post != ଖାଲି) {
		/* POST requests are never cached */
		uncachable = true;
	} ଅନ୍ୟଥା {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	ଯଦି (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		ଯଦି (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ଫେରନ୍ତୁ error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_ଗଣନା, hsts_in_use);
		ଯଦି (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			ଫେରନ୍ତୁ error;
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(obj, &llcache->uncached_objects);
	} ଅନ୍ୟଥା {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_ଗଣନା,
				hsts_in_use, &obj);
		ଯଦି (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ଫେରନ୍ତୁ error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*result = obj;

	nsurl_unref(defragmented_url);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
ସ୍ଥିର nserror llcache_object_add_user(llcache_object *object,
		llcache_object_user *user)
{
	ପ୍ରତିଜ୍ଞା(user->next == ଖାଲି);
	ପ୍ରତିଜ୍ଞା(user->prev == ଖାଲି);
	ପ୍ରତିଜ୍ଞା(user->handle != ଖାଲି);

	user->handle->object = object;

	user->prev = ଖାଲି;
	user->next = object->users;

	ଯଦି (object->users != ଖାଲି)
		object->users->prev = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_hsts_transform_url(nsurl *url, nsurl **result,
		bool *hsts_in_use)
{
	lwc_string *scheme = ଖାଲି;
	bool match;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ଯଦି (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTP fetch: no transform required */
		ଯଦି (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok && match) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_in_use = urldb_get_hsts_enabled(url);
		} ଅନ୍ୟଥା {
			/* Anything else: no HSTS */
			*hsts_in_use = false;
		}
		lwc_string_unref(scheme);
		*result = nsurl_ref(url);
		ଫେରନ୍ତୁ error;
	}
	lwc_string_unref(scheme);

	ଯଦି (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				result);
		*hsts_in_use = (error == NSERROR_OK);
	} ଅନ୍ୟଥା {
		*result = nsurl_ref(url);
		*hsts_in_use = false;
	}

	ଫେରନ୍ତୁ error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = ଖାଲି;
	bool match = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	ଯଦି (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		ଫେରନ୍ତୁ NSERROR_OK;
	}
	lwc_string_unref(scheme);

	ଯଦି (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ପାଇଁ (i = 0; i < object->ଅଙ୍କ_headers; i++) {
		ଯଦି (strcasecmp("Strict-Transport-Security",
				object->headers[i].name) == 0) {
			urldb_set_hsts_policy(object->url,
					object->headers[i].ମୂଲ୍ୟ);
			/* Only process the first one we find */
			ଭାଙ୍ଗ;
		}
	}

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool match;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	ଯଦି ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&match) == lwc_error_ok && match == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&match) == lwc_error_ok && match == false)) {
		/* file, about and resource are not valid redirect targets */
		ଯଦି ((lwc_string_isequal(scheme, corestring_lwc_file,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&match) == lwc_error_ok && match == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			ଫେରନ୍ତୁ NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	ଯଦି (fetch_can_fetch(dst_url) == false) {
		ଫେରନ୍ତୁ NSERROR_UNSAFE_REDIRECT;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_fetch_redirect(llcache_object *object,
		ଅଚଳ ଅକ୍ଷର *target, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *next;
	ଅଚଳ llcache_post_data *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_in_use;
	http_response_code http_code;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_code = fetch_http_code(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ଖାଲି;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_data(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ଶୂନ୍ୟ) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	ଯଦି (object->fetch.redirect_ଗଣନା > nsoption_uint(fetch_redirect_limit)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		ଫେରନ୍ତୁ NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, target, &url);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	nsurl_unref(url);
	ଯଦି (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		ଫେରନ୍ତୁ res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	ଯଦି (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ res;
	}

	/* alter behaviour based on specific redirect code */
	ବାଛ (http_code) {
	ସ୍ଥିତି HTTP_RESPONSE_MOVED_PERMANENTLY:
	ସ୍ଥିତି HTTP_RESPONSE_FOUND:
	ସ୍ଥିତି HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = ଖାଲି;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି HTTP_RESPONSE_TEMPORARY_REDIRECT:
	ସ୍ଥିତି HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_code);
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.data.redirect.from = object->url;
	event.data.redirect.to = hsts_url;

	res = llcache_send_event_to_users(object, &event);
	ଯଦି (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_ଗଣନା + 1,
					hsts_in_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	ଯଦି (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		ପାଇଁ (user = object->users; user != ଖାଲି; user = next) {
			next = user->next;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	ଫେରନ୍ତୁ res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
ସ୍ଥିର nserror llcache_object_cache_update(llcache_object *object)
{
	ଯଦି (object->cache.date == 0)
		object->cache.date = time(ଖାଲି);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
ସ୍ଥିର nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	ଯଦି (object->candidate != ଖାଲି) {
		llcache_object_user *user, *next;

		/* Move user(s) to candidate content */
		ପାଇଁ (user = object->users; user != ଖାଲି; user = next) {
			next = user->next;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_ଗଣନା--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_data(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		ଯଦି (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = ଖାଲି;
	} ଅନ୍ୟଥା {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ଖାଲି;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ଶୂନ୍ୟ) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ସ୍ଥିର nserror
llcache_fetch_process_data(llcache_object *object,
			   ଅଚଳ uint8_t *data,
			   size_t len)
{
	ଯଦି (object->fetch.state != LLCACHE_FETCH_DATA) {
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

		ଯଦି ((http_code != 200 && http_code != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_data(object);
		}

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		object->fetch.state = LLCACHE_FETCH_DATA;
	}

	/* Resize source buffer if it's too small */
	ଯଦି (object->source_len + len >= object->source_alloc) {
		ଅଚଳ size_t new_len = object->source_len + len + 64 * 1024;
		uint8_t *temp = realloc(object->source_data, new_len);
		ଯଦି (temp == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		object->source_data = temp;
		object->source_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	memcpy(object->source_data + object->source_len, data, len);
	object->source_len += len;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ସ୍ଥିର nserror llcache_fetch_auth(llcache_object *object, ଅଚଳ ଅକ୍ଷର *realm)
{
	ଅଚଳ ଅକ୍ଷର *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ଖାଲି;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Destroy headers */
	llcache_destroy_headers(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	ଯଦି (realm == ଖାଲି)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	ଯଦି (auth == ଖାଲି || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_BAD_AUTH;
		event.data.error.ସନ୍ଦେଶ = realm;

		error = llcache_send_event_to_users(object, &event);
	} ଅନ୍ୟଥା {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	ଫେରନ୍ତୁ error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ଖାଲି;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	ଯଦି (object->fetch.hsts_in_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_BAD_CERTS;
		event.data.error.ସନ୍ଦେଶ = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} ଅନ୍ୟଥା {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.ସନ୍ଦେଶ = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	}

	ଫେରନ୍ତୁ error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ଖାଲି;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_data(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	ଯଦି (object->fetch.hsts_in_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.ସନ୍ଦେଶ = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} ଅନ୍ୟଥା {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	ଫେରନ୍ତୁ error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
ସ୍ଥିର nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	ଯଦି (object->fetch.retries_remaining > 1) {
		object->fetch.retries_remaining--;
		ଫେରନ୍ତୁ llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ଖାଲି;

	/* Release candidate, if any */
	ଯଦି (object->candidate != ଖାଲି) {
		object->candidate->candidate_ଗଣନା--;
		object->candidate = ଖାଲି;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_data(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.data.error.code = NSERROR_TIMEOUT;
	event.data.error.ସନ୍ଦେଶ = ଖାଲି;

	ଫେରନ୍ତୁ llcache_send_event_to_users(object, &event);
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
ସ୍ଥିର nserror
build_candidate_list(ସଂରଚନା llcache_object ***lst_out, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *lst_len_out)
{
	llcache_object *object, *next;
	ସଂରଚନା llcache_object **lst;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lst_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_lifetime;

#define MAX_PERSIST_PER_RUN 128

	lst = calloc(MAX_PERSIST_PER_RUN, ଆକାର(ସଂରଚନା llcache_object *));
	ଯଦି (lst == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ପାଇଁ (object = llcache->cached_objects; object != ଖାଲି; object = next) {
		next = object->next;

		/* Only consider http(s) for the disc cache. */
		ଯଦି (!llcache__scheme_is_persistable(object->url)) {
			ଜାରି;
		}

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		ଯଦି ((object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (remaining_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			ଯଦି (lst_len == MAX_PERSIST_PER_RUN)
				ଭାଙ୍ଗ;
		}
	}

	ଯଦି (lst_len == 0) {
		ମୁକ୍ତ(lst);
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;
	}

	/** \todo sort list here */

	*lst_len_out = lst_len;
	*lst_out = lst;

#undef MAX_PERSIST_PER_RUN

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
ସ୍ଥିର nserror
write_backing_store(ସଂରଚନା llcache_object *object,
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
	ଯଦି (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		ଫେରନ୍ତୁ ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	ଯଦି (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ଫେରନ୍ତୁ ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	ଯଦି (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ଫେରନ୍ତୁ ret;
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
	ଯଦି (*elapsed == 0) {
		*elapsed = 1;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର ଶୂନ୍ୟ llcache_persist_slowcheck(ଶୂନ୍ୟ *p)
{
	uint64_t total_bandwidth; /* total bandwidth */

	ଯଦି (llcache->total_written > (2 * llcache->minimum_bandwidth)) {

		total_bandwidth = (llcache->total_written * 1000) / llcache->total_elapsed;

		ଯଦି (total_bandwidth < llcache->minimum_bandwidth) {
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
ସ୍ଥିର ଶୂନ୍ୟ llcache_persist(ଶୂନ୍ୟ *p)
{
	nserror ret;
	ସଂରଚନା llcache_object **lst; /* candidate object list */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lst_ଗଣନା; /* number of candidates in list */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx; /* current candidate object index in list */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା next = -1; /* when the next run should be scheduled for */

	unsigned long write_limit; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	unsigned long elapsed; /* how long writing an object took */

	size_t total_written = 0; /* total bytes written in this run */
	unsigned long total_elapsed = 1; /* total ms used to write bytes */
	unsigned long total_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_ଗଣନା);
	ଯଦି (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		ଫେରନ୍ତୁ;
	}

	write_limit = (llcache->maximum_bandwidth * llcache->time_quantum) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	ପାଇଁ (idx = 0; idx < lst_ଗଣନା; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		ଯଦି (ret != NSERROR_OK) {
			ଜାରି;
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
		ଯଦି (total_elapsed > llcache->time_quantum) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			ଯଦି (total_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->schedule(
						llcache->time_quantum * 100,
						llcache_persist_slowcheck,
						ଖାଲି);
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା {
				ଯଦି (total_bandwidth > llcache->maximum_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					next = ((total_written * llcache->time_quantum) / write_limit) - total_elapsed;
				} ଅନ୍ୟଥା {
					next = llcache->time_quantum;
				}
				ଭାଙ୍ଗ;
			}
		} ଅନ୍ୟଥା ଯଦି (total_written > write_limit) {
			/* The bandwidth limit has been reached. */

			ଯଦି (total_bandwidth > llcache->maximum_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				next = ((total_written * llcache->time_quantum) / write_limit) - total_elapsed;
			} ଅନ୍ୟଥା {
				next = llcache->time_quantum - total_elapsed;
			}
			ଭାଙ୍ଗ;
		}

	}
	ମୁକ୍ତ(lst);

	/* Completed list without running out of allowed bytes or time */
	ଯଦି (idx == lst_ଗଣନା) {
		/* only reschedule if writing is making any progress at all */
		ଯଦି (total_written > 0) {
			next = llcache->time_quantum - total_elapsed;
		} ଅନ୍ୟଥା {
			next = -1;
		}
	}

	llcache->total_written += total_written;
	llcache->total_elapsed += total_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      total_written, total_elapsed, total_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", next);
	guit->misc->schedule(next, llcache_persist, ଖାଲି);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
ସ୍ଥିର ଶୂନ୍ୟ llcache_fetch_callback(ଅଚଳ fetch_ସନ୍ଦେଶ *ସନ୍ଦେଶ, ଶୂନ୍ୟ *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	ଯଦି (llcache == ଖାଲି) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		ପ୍ରତିଜ୍ଞା(false);
		/* In case assertions are off, return here */
		ଫେରନ୍ତୁ;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", ସନ୍ଦେଶ->type, object);

	ବାଛ (ସନ୍ଦେଶ->type) {
	ସ୍ଥିତି FETCH_HEADER:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_HEADERS;

		error = llcache_fetch_process_header(object,
				ସନ୍ଦେଶ->data.header_or_data.buf,
				ସନ୍ଦେଶ->data.header_or_data.len);
		ଭାଙ୍ଗ;

	/* 3xx responses */
	ସ୍ଥିତି FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		error = llcache_fetch_redirect(object,
				ସନ୍ଦେଶ->data.redirect, &object);
		ଯଦି (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.data.error.code = error;
			event.data.error.ସନ୍ଦେଶ = ଖାଲି;
			error = llcache_send_event_to_users(object, &event);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		ଭାଙ୍ଗ;

	/* Normal 2xx state machine */
	ସ୍ଥିତି FETCH_DATA:
		/* Received some data */
		error = llcache_fetch_process_data(object,
				ସନ୍ଦେଶ->data.header_or_data.buf,
				ସନ୍ଦେଶ->data.header_or_data.len);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_FINISHED:
		/* Finished fetching */
	{
		uint8_t *temp;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ଖାଲି;

		/* Shrink source buffer to required size */
		temp = realloc(object->source_data,
				object->source_len);
		/* If source_len is 0, then temp may be NULL */
		ଯଦି (temp != ଖାଲି || object->source_len == 0) {
			object->source_data = temp;
			object->source_alloc = object->source_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_time = time(ଖାଲି);

		(ଶୂନ୍ୟ) llcache_hsts_update_policy(object);

		guit->misc->schedule(5000, llcache_persist, ଖାଲି);
	}
		ଭାଙ୍ଗ;

	/* Out-of-band information */
	ସ୍ଥିତି FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ଖାଲି;

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_data(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.data.error.code = NSERROR_UNKNOWN;
		event.data.error.ସନ୍ଦେଶ = ସନ୍ଦେଶ->data.error;

		error = llcache_send_event_to_users(object, &event);

		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_PROGRESS:
		/* Progress update */
		event.type = LLCACHE_EVENT_PROGRESS;
		event.data.progress_ସନ୍ଦେଶ = ସନ୍ଦେଶ->data.progress;

		error = llcache_send_event_to_users(object, &event);

		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_CERTS:
		/* Certificate information from the fetch */

		ଯଦି (object->chain != ଖାଲି) {
			cert_chain_free(object->chain);
			object->chain = ଖାଲି;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(ସନ୍ଦେଶ->data.chain, &object->chain);
		ଯଦି (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.data.chain = ସନ୍ଦେଶ->data.chain;

		error = llcache_send_event_to_users(object, &event);

		ଭାଙ୍ଗ;

	/* Events requiring action */
	ସ୍ଥିତି FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		error = llcache_fetch_auth(object, ସନ୍ଦେଶ->data.auth.realm);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		error = llcache_fetch_cert_error(object);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		ଯଦି (object->candidate != ଖାଲି) {
			object->candidate->candidate_ଗଣନା--;
			object->candidate = ଖାଲି;
		}

		error = llcache_fetch_ssl_error(object);
		ଭାଙ୍ଗ;
	}

	/* Deal with any errors reported by event handlers */
	ଯଦି (error != NSERROR_OK) {
		ଯଦି (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		ଯଦି (object->fetch.fetch != ଖାଲି) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ଖାଲି;

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
ସ୍ଥିର llcache_object_user *llcache_object_find_user(
		ଅଚଳ llcache_handle *handle)
{
	llcache_object_user *user;

	ପ୍ରତିଜ୍ଞା(handle->object != ଖାଲି);

	ପାଇଁ (user = handle->object->users; user != ଖାଲି; user = user->next) {
		ଯଦି (user->handle == handle)
			ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
ସ୍ଥିର bool llcache_object_in_list(ଅଚଳ llcache_object *object,
		ଅଚଳ llcache_object *list)
{
	ଯେତେବେଳେ (list != ଖାଲି) {
		ଯଦି (list == object)
			ଭାଙ୍ଗ;

		list = list->next;
	}

	ଫେରନ୍ତୁ list != ଖାଲି;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror llcache_object_notify_users(llcache_object *object)
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

	ପାଇଁ (user = object->users; user != ଖାଲି; user = next_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ଅଚଳ llcache_fetch_state objstate = object->fetch.state;

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

		ଯଦି (handle->state != objstate) {
			ଯଦି (emitted_notify == false) {
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
		ଯଦି (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_HEADERS;

			/* Emit any certificate data we hold */
			ଯଦି (object->chain != ଖାଲି) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.data.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} ଅନ୍ୟଥା {
				error = NSERROR_OK;
			}

			ଯଦି (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ଯଦି (error != NSERROR_OK)
					ଫେରନ୍ତୁ error;

				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error != NSERROR_OK) {
				user->iterator_target = false;
				ଫେରନ୍ତୁ error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		ଯଦି (handle->state == LLCACHE_FETCH_HEADERS &&
				objstate > LLCACHE_FETCH_HEADERS) {
			handle->state = LLCACHE_FETCH_DATA;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_HEADERS;

			error = handle->cb(handle, &event, handle->pw);

			ଯଦି (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ଯଦି (error != NSERROR_OK)
					ଫେରନ୍ତୁ error;

				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error != NSERROR_OK) {
				user->iterator_target = false;
				ଫେରନ୍ତୁ error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		ଯଦି (handle->state == LLCACHE_FETCH_DATA &&
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
			ଯଦି (object->fetch.flags &
					LLCACHE_RETRIEVE_STREAM_DATA) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->bytes = object->source_len = 0;
			} ଅନ୍ୟଥା {
				orig_handle_read = handle->bytes;
				handle->bytes = object->source_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			ଯଦି (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ଯଦି (error != NSERROR_OK)
					ଫେରନ୍ତୁ error;

				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->bytes = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error != NSERROR_OK) {
				user->iterator_target = false;
				ଫେରନ୍ତୁ error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		ଯଦି (handle->state == LLCACHE_FETCH_DATA &&
				objstate > LLCACHE_FETCH_DATA) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_DONE;

			error = handle->cb(handle, &event, handle->pw);
			ଯଦି (user->queued_for_delete) {
				next_user = user->next;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ଯଦି (error != NSERROR_OK)
					ଫେରନ୍ତୁ error;

				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error == NSERROR_NEED_DATA) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_DATA;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				next_user = user->next;
				llcache_users_not_caught_up();
				ଜାରି;
			} ଅନ୍ୟଥା ଯଦି (error != NSERROR_OK) {
				user->iterator_target = false;
				ଫେରନ୍ତୁ error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_target = false;

		next_user = user->next;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	newobj->source_alloc = newobj->source_len = object->source_len;

	ଯଦି (object->source_len > 0) {
		newobj->source_data = malloc(newobj->source_alloc);
		ଯଦି (newobj->source_data == ଖାଲି) {
			llcache_object_destroy(newobj);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		memcpy(newobj->source_data, object->source_data,
				newobj->source_len);
	}

	ଯଦି (object->ଅଙ୍କ_headers > 0) {
		newobj->headers = calloc(object->ଅଙ୍କ_headers,
				ଆକାର(llcache_header));
		ଯଦି (newobj->headers == ଖାଲି) {
			llcache_object_destroy(newobj);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		ଯେତେବେଳେ (newobj->ଅଙ୍କ_headers < object->ଅଙ୍କ_headers) {
			llcache_header *nh =
					&(newobj->headers[newobj->ଅଙ୍କ_headers]);
			llcache_header *oh =
					&(object->headers[newobj->ଅଙ୍କ_headers]);
			newobj->ଅଙ୍କ_headers += 1;
			nh->name = strdup(oh->name);
			nh->ମୂଲ୍ୟ = strdup(oh->ମୂଲ୍ୟ);
			ଯଦି (nh->name == ଖାଲି || nh->ମୂଲ୍ୟ == ଖାଲି) {
				llcache_object_destroy(newobj);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}
		}
	}

	ଯଦି (object->chain != ଖାଲି) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		ଯଦି (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			ଫେରନ୍ତୁ error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ uint32_t
total_object_size(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = ଆକାର(*object);
	tot += nsurl_length(object->url);

	ଯଦି (object->source_data != ଖାଲି) {
		tot += object->source_len;
	}

	tot += ଆକାର(llcache_header) * object->ଅଙ୍କ_headers;

	ପାଇଁ (hdrc = 0; hdrc < object->ଅଙ୍କ_headers; hdrc++) {
		ଯଦି (object->headers[hdrc].name != ଖାଲି) {
			tot += strlen(object->headers[hdrc].name);
		}
		ଯଦି (object->headers[hdrc].ମୂଲ୍ୟ != ଖାଲି) {
			tot += strlen(object->headers[hdrc].ମୂଲ୍ୟ);
		}
	}

	tot += cert_chain_size(object->chain);

	ଫେରନ୍ତୁ tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
ସ୍ଥିର ଶୂନ୍ୟ llcache_catch_up_all_users(ଶୂନ୍ୟ *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_up = true;

	/* Catch new users up with state of objects */
	ପାଇଁ (object = llcache->cached_objects; object != ଖାଲି;
			object = object->next) {
		llcache_object_notify_users(object);
	}

	ପାଇଁ (object = llcache->uncached_objects; object != ଖାଲି;
			object = object->next) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
ସ୍ଥିର ଶୂନ୍ୟ llcache_users_not_caught_up(ଶୂନ୍ୟ)
{
	ଯଦି (llcache->all_caught_up) {
		llcache->all_caught_up = false;
		guit->misc->schedule(0, llcache_catch_up_all_users, ଖାଲି);
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
ଶୂନ୍ୟ llcache_clean(bool purge)
{
	llcache_object *object, *next;
	uint32_t llcache_size = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା remaining_lifetime;
	uint32_t limit;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	ଯଦି (purge) {
		limit = 0;
	} ଅନ୍ୟଥା {
		limit = llcache->limit;
	}

	/* Uncacheable objects with no users or fetches */
	ପାଇଁ (object = llcache->uncached_objects;
	     object != ଖାଲି;
	     object = next) {
		next = object->next;

		/* The candidate count of uncacheable objects is always 0 */
		ଯଦି ((object->users == ଖାଲି) &&
		    (object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} ଅନ୍ୟଥା {
			llcache_size += total_object_size(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	ପାଇଁ (object = llcache->cached_objects;
	     object != ଖାଲି;
	     object = next) {
		next = object->next;

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		ଯଦି ((object->users == ଖାଲି) &&
		    (object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି) &&
		    (remaining_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				ଯଦି (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} ଅନ୍ୟଥା {
			/* object has users so account for the storage */
			llcache_size += total_object_size(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	ଯଦି (limit < llcache_size) {
		llcache_persist(ଖାଲି);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	ପାଇଁ (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != ଖାଲି));
	     object = next) {
		next = object->next;
		ଯଦି ((object->users == ଖାଲି) &&
		    (object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->source_data = ଖାଲି;

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
	ପାଇଁ (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != ଖାଲି));
	     object = next) {
		next = object->next;
		ଯଦି ((object->users == ଖାଲି) &&
		    (object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->source_data == ଖାଲି)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (long)(time(ଖାଲି) - object->last_used),
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
	ପାଇଁ (object = llcache->cached_objects;
	     ((limit < llcache_size) && (object != ଖାଲି));
	     object = next) {
		next = object->next;

		ଯଦି ((object->users == ଖାଲି) &&
		    (object->candidate_ଗଣନା == 0) &&
		    (object->fetch.fetch == ଖାଲି) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (long)(time(ଖାଲି) - object->last_used),
			      object,
			      nsurl_access(object->url));

			llcache_size -=	object->source_len + ଆକାର(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_size, limit);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ଅଚଳ ସଂରଚନା llcache_parameters *prm)
{
	llcache = calloc(1, ଆକାର(ସଂରଚନା llcache_s));
	ଯଦି (llcache == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
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
	ଫେରନ୍ତୁ guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
ଶୂନ୍ୟ llcache_finalise(ଶୂନ୍ୟ)
{
	llcache_object *object, *next;
	uint64_t total_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(ଖାଲି);
	/* Now clear the persistence callback */
	guit->misc->schedule(-1, llcache_persist, ଖାଲି);

	/* Clean uncached objects */
	ପାଇଁ (object = llcache->uncached_objects; object != ଖାଲି; object = next) {
		llcache_object_user *user, *next_user;

		next = object->next;

		ପାଇଁ (user = object->users; user != ଖାଲି; user = next_user) {
			next_user = user->next;

			ଯଦି (user->handle != ଖାଲି)
				ମୁକ୍ତ(user->handle);

			ମୁକ୍ତ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ଖାଲି;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	ପାଇଁ (object = llcache->cached_objects; object != ଖାଲି; object = next) {
		llcache_object_user *user, *next_user;

		next = object->next;

		ପାଇଁ (user = object->users; user != ଖାଲି; user = next_user) {
			next_user = user->next;

			ଯଦି (user->handle != ଖାଲି)
			       ମୁକ୍ତ(user->handle);

			ମୁକ୍ତ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ଖାଲି;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	ଯଦି (llcache->total_elapsed > 0) {
		total_bandwidth = (llcache->total_written * 1000) /
			llcache->total_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->total_written,
	      llcache->total_elapsed,
	      total_bandwidth);

	ମୁକ୍ତ(llcache);
	llcache = ଖାଲି;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ଅଚଳ llcache_post_data *post,
			llcache_handle_callback cb, ଶୂନ୍ୟ *pw,
			llcache_handle **result)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_in_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	ଯଦି (error != NSERROR_OK) {
		ଫେରନ୍ତୁ error;
	}

	/* Can we fetch this URL at all? */
	ଯଦି (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	ଯଦି (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_in_use, &object);
	ଯଦି (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		ଫେରନ୍ତୁ error;
	}

	/* Add user to object */
	llcache_object_add_user(object, user);

	*result = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_up();

	nsurl_unref(hsts_url);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ଶୂନ୍ୟ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_find_user(handle);

	ପ୍ରତିଜ୍ଞା(user != ଖାଲି);

	ଯଦି (user->iterator_target) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} ଅନ୍ୟଥା {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		ଯଦି (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	ଫେରନ୍ତୁ error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **result)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	ଯଦି (error == NSERROR_OK) {
		llcache_object_add_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*result = newuser->handle;
	}

	ଫେରନ୍ତୁ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	ଯଦି (user->prev != ଖାଲି)
		all_alone = false;
	ଯଦି (user->next != ଖାଲି)
		all_alone = false;

	ଯଦି (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		ଯଦି (error != NSERROR_OK)
			ଫେରନ୍ତୁ error;

		/* Move across to the new object */
		ଯଦି (user->iterator_target) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					calloc(1, ଆକାର(llcache_object_user));
			ଯଦି (newuser == ଖାଲି) {
				llcache_object_destroy(newobject);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = ଖାଲି;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_add_user(newobject, newuser);
		} ଅନ୍ୟଥା {
			llcache_object_remove_user(object, user);
			llcache_object_add_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(newobject,
				&llcache->uncached_objects);
	} ଅନ୍ୟଥା {
		/* We're the only user, so abort any fetch in progress */
		ଯଦି (object->fetch.fetch != ଖାଲି) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ଖାଲି;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_data(object);
	}

	ଫେରନ୍ତୁ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_stream(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	ଯଦି (user->prev != ଖାଲି || user->next != ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_OK;

	/* Forcibly uncache this object */
	ଯଦି (llcache_object_in_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_add_to_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_STREAM_DATA;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_data(llcache_handle *handle)
{
	ଯଦି ((handle->object != ଖାଲି) &&
	    (handle->object->fetch.fetch == ଖାଲି) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ଅଚଳ llcache_handle *handle)
{
	ଫେରନ୍ତୁ handle->object != ଖାଲି ? handle->object->url : ଖାଲି;
}

/* See llcache.h for documentation */
ଅଚଳ uint8_t *llcache_handle_get_source_data(ଅଚଳ llcache_handle *handle,
		size_t *size)
{
	*size = handle->object != ଖାଲି ? handle->object->source_len : 0;

	ଫେରନ୍ତୁ handle->object != ଖାଲି ? handle->object->source_data : ଖାଲି;
}

/* See llcache.h for documentation */
ଅଚଳ ଅକ୍ଷର *llcache_handle_get_header(ଅଚଳ llcache_handle *handle,
		ଅଚଳ ଅକ୍ଷର *ଚାବି)
{
	ଅଚଳ llcache_object *object = handle->object;
	size_t i;

	ଯଦି (object == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	/* About as trivial as possible */
	ପାଇଁ (i = 0; i < object->ଅଙ୍କ_headers; i++) {
		ଯଦି (strcasecmp(ଚାବି, object->headers[i].name) == 0)
			ଫେରନ୍ତୁ object->headers[i].ମୂଲ୍ୟ;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ଅଚଳ llcache_handle *a,
		ଅଚଳ llcache_handle *b)
{
	ଫେରନ୍ତୁ a->object == b->object;
}
