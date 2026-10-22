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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/base64.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/time.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/backing_store.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ,		/**< Fetching headers */
	LLCACHE_FETCH_ਅੰਕੜੇ,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
ਬਣਤਰ llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	ਖਾਲੀ *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t ਬਾਈਟਾਂ;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_ਨਿਸ਼ਾਨਾ;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	ਬਣਤਰ llcache_object_user *prev;	/**< Previous in list */
	ਬਣਤਰ llcache_object_user *ਅਗਲਾ;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_ਅੰਕੜੇ *post;	/**< POST data, or NULL for GET */

	ਬਣਤਰ fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_ਗਿਣਤੀ;	/**< Count of redirects followed */

	uint32_t retries_ਬਚਿਆ;     /**< Number of times to retry on timeout */

	bool hsts_ਵਿੱਚ_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_AGE -1

/** Cache control data */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	time_t req_ਸਮਾਂ;	/**< Time of request */
	time_t res_ਸਮਾਂ;	/**< Time of response */
	time_t fin_ਸਮਾਂ;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	ਪੂਰਨ age;		/**< Age: response header */
	ਪੂਰਨ max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	ਅੱਖਰ *etag;		/**< Etag: response header */
	time_t ਆਖਰੀ_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	ਅੱਖਰ *name;		/**< Header name */
	ਅੱਖਰ *ਮੁੱਲ;		/**< Header value */
} llcache_ਸਿਰਲੇਖ;

/** Current status of an object's data */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
ਬਣਤਰ llcache_object {
	llcache_object *prev;	     /**< Previous in list */
	llcache_object *ਅਗਲਾ;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *ਸਰੋਤ_ਅੰਕੜੇ;	     /**< Source data for object */
	size_t ਸਰੋਤ_len;	     /**< Byte length of source data */
	size_t ਸਰੋਤ_alloc;	     /**< Allocated size of source buffer */

	ਬਣਤਰ cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_ਗਿਣਤੀ;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_ਸਿਰਲੇਖ *ਸਿਰਲੇਖਾਂ;     /**< Fetch headers */
	size_t ਅੰਕ_ਸਿਰਲੇਖਾਂ;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t ਆਖਰੀ_ਵਰਤਿਆ; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
ਬਣਤਰ llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t ਹੱਦਬੰਦੀ;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_ਉੱਪਰ;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	ਪੂਰਨ minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ;

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
	size_t ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t ਕੁੱਲ_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t ਕੁੱਲ_elapsed;

};

/** low level cache state */
ਸਥਿਰ ਬਣਤਰ llcache_s *llcache = ਨਿਰਾ;

/* forward referenced callback function */
ਸਥਿਰ ਖਾਲੀ llcache_fetch_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਖਾਲੀ *p);

/* forward referenced catch up function */
ਸਥਿਰ ਖਾਲੀ llcache_users_not_caught_ਉੱਪਰ(ਖਾਲੀ);


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
ਸਥਿਰ nserror llcache_object_user_new(llcache_handle_callback cb, ਖਾਲੀ *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(llcache_handle));
	ਜੇ (h == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	u = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(llcache_object_user));
	ਜੇ (u == ਨਿਰਾ) {
		ਮੁਕਤ(h);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (ਖਾਲੀ *) cb, pw);

	*user = u;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
ਸਥਿਰ nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	ਪ੍ਰਤਿਜ੍ਞਾ(user->ਅਗਲਾ == ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(user->prev == ਨਿਰਾ);

	ਜੇ (user->handle != ਨਿਰਾ)
		ਮੁਕਤ(user->handle);

	ਮੁਕਤ(user);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
ਸਥਿਰ nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(user != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(object != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(object->users != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(user->handle == ਨਿਰਾ || user->handle->object == object);
	ਪ੍ਰਤਿਜ੍ਞਾ((user->prev != ਨਿਰਾ) || (object->users == user));

	ਜੇ (user == object->users)
		object->users = user->ਅਗਲਾ;
	ਨਹੀਂਤਾਂ
		user->prev->ਅਗਲਾ = user->ਅਗਲਾ;

	ਜੇ (user->ਅਗਲਾ != ਨਿਰਾ)
		user->ਅਗਲਾ->prev = user->prev;

	user->ਅਗਲਾ = user->prev = ਨਿਰਾ;

	/* record the time the last user was removed from the object */
	ਜੇ (object->users == ਨਿਰਾ) {
		object->ਆਖਰੀ_ਵਰਤਿਆ = time(ਨਿਰਾ);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ਸਥਿਰ nserror llcache_send_event_ਮੰਜ਼ਿਲ_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *ਅਗਲਾ_user;

	user = object->users;
	ਜਦੋਂ (user != ਨਿਰਾ) {
		bool was_ਨਿਸ਼ਾਨਾ = user->iterator_ਨਿਸ਼ਾਨਾ;
		user->iterator_ਨਿਸ਼ਾਨਾ = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		ਅਗਲਾ_user = user->ਅਗਲਾ;

		user->iterator_ਨਿਸ਼ਾਨਾ = was_ਨਿਸ਼ਾਨਾ;

		ਜੇ (user->queued_for_delete && !was_ਨਿਸ਼ਾਨਾ) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		ਜੇ (error != NSERROR_OK)
			ਰੋਕੋ;

		user = ਅਗਲਾ_user;
	}

	ਵਾਪਸ error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_object_new(nsurl *url, llcache_object **ਨਤੀਜਾ)
{
	llcache_object *obj = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(llcache_object));
	ਜੇ (obj == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*ਨਤੀਜਾ = obj;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_post_ਅੰਕੜੇ_clone(ਅਚਲ llcache_post_ਅੰਕੜੇ *orig,
		llcache_post_ਅੰਕੜੇ **clone)
{
	llcache_post_ਅੰਕੜੇ *post_clone;

	post_clone = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(llcache_post_ਅੰਕੜੇ));
	ਜੇ (post_clone == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	ਜੇ (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->ਅੰਕੜੇ.urlenc = strdup(orig->ਅੰਕੜੇ.urlenc);
		ਜੇ (post_clone->ਅੰਕੜੇ.urlenc == ਨਿਰਾ) {
			ਮੁਕਤ(post_clone);

			ਵਾਪਸ NSERROR_NOMEM;
		}
	} ਨਹੀਂਤਾਂ {
		post_clone->ਅੰਕੜੇ.multipart = fetch_multipart_ਅੰਕੜੇ_clone(
				orig->ਅੰਕੜੇ.multipart);
		ਜੇ (post_clone->ਅੰਕੜੇ.multipart == ਨਿਰਾ) {
			ਮੁਕਤ(post_clone);

			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror llcache_fetch_ਵੰਡੋ_ਸਿਰਲੇਖ(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		ਅੱਖਰ **name, ਅੱਖਰ **ਮੁੱਲ)
{
	ਅੱਖਰ *n, *v;
	ਅਚਲ uint8_t *colon;

	/* Strip leading whitespace from name */
	ਜਦੋਂ (ਅੰਕੜੇ[0] == ' ' || ਅੰਕੜੇ[0] == '\t' ||
	       ਅੰਕੜੇ[0] == '\r' || ਅੰਕੜੇ[0] == '\n') {
		ਅੰਕੜੇ++;
	}

	/* Find colon */
	colon = (ਅਚਲ uint8_t *) strchr((ਅਚਲ ਅੱਖਰ *) ਅੰਕੜੇ, ':');
	ਜੇ (colon == ਨਿਰਾ) {
		/* Failed, assume a key with no value */
		colon = ਅੰਕੜੇ + ਲੜੀ_ਲੰਬਾਈ((ਅਚਲ ਅੱਖਰ *)ਅੰਕੜੇ);

		/* Strip trailing whitespace from name */
		ਜਦੋਂ ((colon > ਅੰਕੜੇ) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ਅਚਲ ਅੱਖਰ *) ਅੰਕੜੇ, colon - ਅੰਕੜੇ);
		ਜੇ (n == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		v = strdup("");
		ਜੇ (v == ਨਿਰਾ) {
			ਮੁਕਤ(n);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	} ਨਹੀਂਤਾਂ {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		ਜਦੋਂ (colon > ਅੰਕੜੇ && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ਅਚਲ ਅੱਖਰ *) ਅੰਕੜੇ, colon - ਅੰਕੜੇ);
		ਜੇ (n == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		/* Find colon again */
		ਜਦੋਂ (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		ਕਰੋ {
			colon++;
		} ਜਦੋਂ (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		ਜਦੋਂ (len > 0 && (ਅੰਕੜੇ[len - 1] == ' ' ||
				ਅੰਕੜੇ[len - 1] == '\t' ||
				ਅੰਕੜੇ[len - 1] == '\r' ||
				ਅੰਕੜੇ[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ਅਚਲ ਅੱਖਰ *) colon, len - (colon - ਅੰਕੜੇ));
		ਜੇ (v == ਨਿਰਾ) {
			ਮੁਕਤ(n);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	*name = n;
	*ਮੁੱਲ = v;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror
llcache_fetch_parse_cache_control(llcache_object *object, ਅੱਖਰ *ਮੁੱਲ)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(ਮੁੱਲ, &cc);
	ਜੇ (error != NSERROR_OK) {
		/* Ignore parse errors */
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	ਜੇ (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror
llcache_fetch_ਸਿਰਲੇਖ_cache_control(llcache_object *object,
				   ਅੱਖਰ *name,
				   ਅੱਖਰ *ਮੁੱਲ)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = ਲੜੀ_ਲੰਬਾਈ(name);

	ਚੋਣ (name_len) {
	ਹਾਲਤ 3:
		ਜੇ (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			ਜੇ ('0' <= *ਮੁੱਲ && *ਮੁੱਲ <= '9') {
				object->cache.age = atoi(ਮੁੱਲ);
			}

		}
		ਰੋਕੋ;

	ਹਾਲਤ 4:
		ਜੇ (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(ਮੁੱਲ,
					    ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ),
					    &object->cache.date);
			ਜੇ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      ਮੁੱਲ, res);
			}
		} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			ਮੁਕਤ(object->cache.etag);
			object->cache.etag = strdup(ਮੁੱਲ);
			ਜੇ (object->cache.etag == ਨਿਰਾ) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				ਵਾਪਸ NSERROR_NOMEM;
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ 7:
		ਜੇ (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(ਮੁੱਲ,
					    ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ),
					    &object->cache.expires);
			ਜੇ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      ਮੁੱਲ, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ 13:
		ਜੇ (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,ਮੁੱਲ);
		} ਨਹੀਂਤਾਂ ਜੇ (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(ਮੁੱਲ,
				      ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ),
				      &object->cache.ਆਖਰੀ_modified);
		}
		ਰੋਕੋ;
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ llcache_destroy_ਸਿਰਲੇਖਾਂ(llcache_object *object)
{
	ਜਦੋਂ (object->ਅੰਕ_ਸਿਰਲੇਖਾਂ > 0) {
		object->ਅੰਕ_ਸਿਰਲੇਖਾਂ--;

		ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ[object->ਅੰਕ_ਸਿਰਲੇਖਾਂ].name);
		ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ[object->ਅੰਕ_ਸਿਰਲੇਖਾਂ].ਮੁੱਲ);
	}
	ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ);
	object->ਸਿਰਲੇਖਾਂ = ਨਿਰਾ;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ llcache_invalidate_cache_control_ਅੰਕੜੇ(llcache_object *object)
{
	ਮੁਕਤ(object->cache.etag);
	memset(&(object->cache), 0, ਆਕਾਰ(llcache_cache_control));

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
ਸਥਿਰ nserror llcache_fetch_process_ਸਿਰਲੇਖ(llcache_object *object,
		ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len)
{
	nserror res;
	ਅੱਖਰ *name, *ਮੁੱਲ;
	llcache_ਸਿਰਲੇਖ *ਆਰਜ਼ੀ;

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
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ((ਅਚਲ ਅੱਖਰ *) ਅੰਕੜੇ, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_ਸਮਾਂ = object->cache.req_ਸਮਾਂ;

		llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_ਸਮਾਂ = req_ਸਮਾਂ;

		llcache_destroy_ਸਿਰਲੇਖਾਂ(object);
	}

	/* Set fetch response time if not already set */
	ਜੇ (object->cache.res_ਸਮਾਂ == 0) {
		object->cache.res_ਸਮਾਂ = time(ਨਿਰਾ);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_ਵੰਡੋ_ਸਿਰਲੇਖ(ਅੰਕੜੇ, len, &name, &ਮੁੱਲ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	/* deal with empty header */
	ਜੇ (name[0] == 0) {
		ਮੁਕਤ(name);
		ਮੁਕਤ(ਮੁੱਲ);
		ਵਾਪਸ NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_ਸਿਰਲੇਖ_cache_control(object, name, ਮੁੱਲ);
	ਜੇ (res != NSERROR_OK) {
		ਮੁਕਤ(name);
		ਮੁਕਤ(ਮੁੱਲ);
		ਵਾਪਸ res;
	}

	/* Append header data to the object's headers array */
	ਆਰਜ਼ੀ = realloc(object->ਸਿਰਲੇਖਾਂ,
		       (object->ਅੰਕ_ਸਿਰਲੇਖਾਂ + 1) * ਆਕਾਰ(llcache_ਸਿਰਲੇਖ));
	ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ) {
		ਮੁਕਤ(name);
		ਮੁਕਤ(ਮੁੱਲ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	object->ਸਿਰਲੇਖਾਂ = ਆਰਜ਼ੀ;

	object->ਸਿਰਲੇਖਾਂ[object->ਅੰਕ_ਸਿਰਲੇਖਾਂ].name = name;
	object->ਸਿਰਲੇਖਾਂ[object->ਅੰਕ_ਸਿਰਲੇਖਾਂ].ਮੁੱਲ = ਮੁੱਲ;

	object->ਅੰਕ_ਸਿਰਲੇਖਾਂ++;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror get_referer_ਸਿਰਲੇਖ(nsurl *url, nsurl *referer, ਅੱਖਰ **ਸਿਰਲੇਖ_ਬਾਹਰਲਾ)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool ਮੇਲ;
	bool match1;
	bool match2;
	ਅੱਖਰ *ਸਿਰਲੇਖ;

	/* Determine whether to send the Referer header */
	ਜੇ (!nsoption_bool(send_referer)) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (scheme == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	ਜੇ (ref_scheme == ਨਿਰਾ) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		ਵਾਪਸ NSERROR_INVALID;
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
	ਜੇ (lwc_string_isequal(scheme, ref_scheme,
			       &ਮੇਲ) != lwc_error_ok) {
		ਮੇਲ = false;
	}
	ਜੇ (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	ਜੇ (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	ਜੇ (ਮੇਲ == true || (match1 == true && match2 == true)) {
		ਅਚਲ size_t len = SLEN("Referer: ") +
			nsurl_ਲੰਮਾਈ(referer) + 1;

		ਸਿਰਲੇਖ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
		ਜੇ (ਸਿਰਲੇਖ == ਨਿਰਾ) {
			res = NSERROR_NOMEM;
		} ਨਹੀਂਤਾਂ {
			snprintf(ਸਿਰਲੇਖ, len, "Referer: %s",
				 nsurl_access(referer));

			*ਸਿਰਲੇਖ_ਬਾਹਰਲਾ = ਸਿਰਲੇਖ;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror llcache_object_refetch(llcache_object *object)
{
	ਅਚਲ ਅੱਖਰ *urlenc = ਨਿਰਾ;
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *multipart = ਨਿਰਾ;
	ਅੱਖਰ **ਸਿਰਲੇਖਾਂ = ਨਿਰਾ;
	ਪੂਰਨ ਸਿਰਲੇਖ_idx = 0;
	nserror res;

	ਜੇ (object->fetch.post != ਨਿਰਾ) {
		ਜੇ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->ਅੰਕੜੇ.urlenc;
		} ਨਹੀਂਤਾਂ {
			multipart = object->fetch.post->ਅੰਕੜੇ.multipart;
		}
	}

	/* Generate headers */
	ਸਿਰਲੇਖਾਂ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(4 * ਆਕਾਰ(ਅੱਖਰ *));
	ਜੇ (ਸਿਰਲੇਖਾਂ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	ਜੇ (object->cache.etag != ਨਿਰਾ) {
		ਅਚਲ size_t len = SLEN("If-None-Match: ") +
				ਲੜੀ_ਲੰਬਾਈ(object->cache.etag) + 1;

		ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx] = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
		ਜੇ (ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx] == ਨਿਰਾ) {
			ਮੁਕਤ(ਸਿਰਲੇਖਾਂ);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		snprintf(ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx], len, "If-None-Match: %s",
				object->cache.etag);

		ਸਿਰਲੇਖ_idx++;
	}

	/* cache-control header for modification time */
	ਜੇ (object->cache.ਆਖਰੀ_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ਅਚਲ size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx] = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
		ਜੇ (ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx] == ਨਿਰਾ) {
			ਜਦੋਂ (--ਸਿਰਲੇਖ_idx >= 0)
				ਮੁਕਤ(ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx]);
			ਮੁਕਤ(ਸਿਰਲੇਖਾਂ);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		snprintf(ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.ਆਖਰੀ_modified));

		ਸਿਰਲੇਖ_idx++;
	}

	/* Referer header */
	ਜੇ (object->fetch.referer != ਨਿਰਾ) {
		ਜੇ (get_referer_ਸਿਰਲੇਖ(object->url,
				       object->fetch.referer,
				       &ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx]) == NSERROR_OK) {
			ਸਿਰਲੇਖ_idx++;
		}
	}
	ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx] = ਨਿਰਾ;

	/* Reset cache control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);
	object->cache.req_ਸਮਾਂ = time(ਨਿਰਾ);
	object->cache.fin_ਸਮਾਂ = object->cache.req_ਸਮਾਂ;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_ਸ਼ੁਰੂ(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_ਪੰਨੇ,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (ਅਚਲ ਅੱਖਰ **)ਸਿਰਲੇਖਾਂ,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	ਜਦੋਂ (--ਸਿਰਲੇਖ_idx >= 0) {
		ਮੁਕਤ(ਸਿਰਲੇਖਾਂ[ਸਿਰਲੇਖ_idx]);
	}
	ਮੁਕਤ(ਸਿਰਲੇਖਾਂ);

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
		uint32_t redirect_ਗਿਣਤੀ, bool hsts_ਵਿੱਚ_use)
{
	nserror error;
	nsurl *referer_clone = ਨਿਰਾ;
	llcache_post_ਅੰਕੜੇ *post_clone = ਨਿਰਾ;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	ਜੇ (post != ਨਿਰਾ) {
		error = llcache_post_ਅੰਕੜੇ_clone(post, &post_clone);
		ਜੇ (error != NSERROR_OK)
			ਵਾਪਸ error;
	}

	ਜੇ (referer != ਨਿਰਾ)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ਗਿਣਤੀ = redirect_ਗਿਣਤੀ;
	object->fetch.retries_ਬਚਿਆ = llcache->fetch_attempts;
	object->fetch.hsts_ਵਿੱਚ_use = hsts_ਵਿੱਚ_use;

	ਵਾਪਸ llcache_object_refetch(object);
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
ਸਥਿਰ nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	ਜੇ (object->ਸਰੋਤ_ਅੰਕੜੇ != ਨਿਰਾ) {
		ਜੇ (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} ਨਹੀਂਤਾਂ {
			ਮੁਕਤ(object->ਸਰੋਤ_ਅੰਕੜੇ);
		}
	}

	nsurl_unref(object->url);

	ਜੇ (object->fetch.fetch != ਨਿਰਾ) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = ਨਿਰਾ;
	}

	ਜੇ (object->fetch.referer != ਨਿਰਾ)
		nsurl_unref(object->fetch.referer);

	ਜੇ (object->fetch.post != ਨਿਰਾ) {
		ਜੇ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			ਮੁਕਤ(object->fetch.post->ਅੰਕੜੇ.urlenc);
		} ਨਹੀਂਤਾਂ {
			fetch_multipart_ਅੰਕੜੇ_destroy(
					object->fetch.post->ਅੰਕੜੇ.multipart);
		}

		ਮੁਕਤ(object->fetch.post);
	}

	ਮੁਕਤ(object->cache.etag);

	ਲਈ (i = 0; i < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ; i++) {
		ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ[i].name);
		ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ[i].ਮੁੱਲ);
	}
	ਮੁਕਤ(object->ਸਿਰਲੇਖਾਂ);

	ਮੁਕਤ(object);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
ਸਥਿਰ nserror llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(llcache_object *object,
		llcache_object **list)
{
	object->prev = ਨਿਰਾ;
	object->ਅਗਲਾ = *list;

	ਜੇ (*list != ਨਿਰਾ)
		(*list)->prev = object;
	*list = object;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
ਸਥਿਰ ਪੂਰਨ
llcache_object_rfc2616_ਬਚਿਆ_lifetime(ਅਚਲ llcache_cache_control *cd)
{
	ਪੂਰਨ current_age, freshness_lifetime;
	time_t now = time(ਨਿਰਾ);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_ਸਮਾਂ - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_ਸਮਾਂ - cd->req_ਸਮਾਂ + now - cd->res_ਸਮਾਂ;

	/* Determine freshness lifetime of this object */
	ਜੇ (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} ਨਹੀਂਤਾਂ ਜੇ (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} ਨਹੀਂਤਾਂ ਜੇ (cd->ਆਖਰੀ_modified != 0) {
		freshness_lifetime = (now - cd->ਆਖਰੀ_modified) / 10;
	} ਨਹੀਂਤਾਂ {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	ਜੇ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		ਵਾਪਸ freshness_lifetime - current_age;
	}

	ਵਾਪਸ 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
ਸਥਿਰ bool llcache_object_is_fresh(ਅਚਲ llcache_object *object)
{
	ਪੂਰਨ ਬਚਿਆ_lifetime;
	ਅਚਲ llcache_cache_control *cd = &object->cache;

	ਬਚਿਆ_lifetime = llcache_object_rfc2616_ਬਚਿਆ_lifetime(cd);

	NSLOG(llcache, DEBUG, "%p: (%d > 0 || %d != %d)", object,
	     ਬਚਿਆ_lifetime,
	     object->fetch.state, LLCACHE_FETCH_COMPLETE);

	/* The object is fresh if:
	 * - it was not forbidden from being returned from the cache
	 *   unvalidated.
	 *
	 * - it has remaining lifetime or still being fetched.
	 */
	ਵਾਪਸ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
		((ਬਚਿਆ_lifetime > 0) ||
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
ਸਥਿਰ nserror llcache_object_clone_cache_ਅੰਕੜੇ(llcache_object *ਸਰੋਤ,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	ਜੇ (ਸਰੋਤ->cache.etag != ਨਿਰਾ) {
		ਅੱਖਰ *etag = ਸਰੋਤ->cache.etag;

		ਜੇ (deep) {
			/* Copy the etag */
			etag = strdup(ਸਰੋਤ->cache.etag);
			ਜੇ (etag == ਨਿਰਾ)
				ਵਾਪਸ NSERROR_NOMEM;
		} ਨਹੀਂਤਾਂ {
			/* Destination takes ownership */
			ਸਰੋਤ->cache.etag = ਨਿਰਾ;
		}

		ਜੇ (destination->cache.etag != ਨਿਰਾ)
			ਮੁਕਤ(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_ਸਮਾਂ = ਸਰੋਤ->cache.req_ਸਮਾਂ;
	destination->cache.res_ਸਮਾਂ = ਸਰੋਤ->cache.res_ਸਮਾਂ;
	destination->cache.fin_ਸਮਾਂ = ਸਰੋਤ->cache.fin_ਸਮਾਂ;

	ਜੇ (ਸਰੋਤ->cache.date != 0)
		destination->cache.date = ਸਰੋਤ->cache.date;

	ਜੇ (ਸਰੋਤ->cache.expires != 0)
		destination->cache.expires = ਸਰੋਤ->cache.expires;

	ਜੇ (ਸਰੋਤ->cache.age != INVALID_AGE)
		destination->cache.age = ਸਰੋਤ->cache.age;

	ਜੇ (ਸਰੋਤ->cache.max_age != INVALID_AGE)
		destination->cache.max_age = ਸਰੋਤ->cache.max_age;

	ਜੇ (ਸਰੋਤ->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = ਸਰੋਤ->cache.no_cache;

	ਜੇ (ਸਰੋਤ->cache.ਆਖਰੀ_modified != 0)
		destination->cache.ਆਖਰੀ_modified = ਸਰੋਤ->cache.ਆਖਰੀ_modified;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
ਸਥਿਰ nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	ਜੇ (object == *list)
		*list = object->ਅਗਲਾ;
	ਨਹੀਂਤਾਂ
		object->prev->ਅਗਲਾ = object->ਅਗਲਾ;

	ਜੇ (object->ਅਗਲਾ != ਨਿਰਾ)
		object->ਅਗਲਾ->prev = object->prev;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror llcache_retrieve_persisted_ਅੰਕੜੇ(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	ਜੇ ((object->ਸਰੋਤ_ਅੰਕੜੇ != ਨਿਰਾ) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		ਵਾਪਸ NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	ਵਾਪਸ guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->ਸਰੋਤ_ਅੰਕੜੇ,
				    &object->ਸਰੋਤ_len);
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
ਸਥਿਰ nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **ਅੰਕੜੇ_ਬਾਹਰਲਾ,
			   size_t *datasize_ਬਾਹਰਲਾ)
{
	size_t allocsize;
	ਪੂਰਨ datasize;
	uint8_t *ਅੰਕੜੇ;
	ਅੱਖਰ *op;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ hloop;
	ਪੂਰਨ use;
	size_t cert_chain_ਡੂੰਘਾਈ;

	ਜੇ (object->chain != ਨਿਰਾ) {
		cert_chain_ਡੂੰਘਾਈ = object->chain->ਡੂੰਘਾਈ;
	} ਨਹੀਂਤਾਂ {
		cert_chain_ਡੂੰਘਾਈ = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	ਲਈ (hloop = 0 ; hloop < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ ; hloop++) {
		allocsize += ਲੜੀ_ਲੰਬਾਈ(object->ਸਿਰਲੇਖਾਂ[hloop].name) + 1;
		allocsize += ਲੜੀ_ਲੰਬਾਈ(object->ਸਿਰਲੇਖਾਂ[hloop].ਮੁੱਲ) + 1;
	}

	allocsize += nsurl_ਲੰਮਾਈ(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	ਲਈ (hloop = 0; hloop < cert_chain_ਡੂੰਘਾਈ; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_ਲੰਮਾਈ + 2) / 3);
	}

	ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(allocsize);
	ਜੇ (ਅੰਕੜੇ == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	op = (ਅੱਖਰ *)ਅੰਕੜੇ;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	ਜੇ (use < 0) {
		ਜਾਓ operror;
	}
	use++; /* does not count the null */
	ਜੇ (use > datasize) {
		ਜਾਓ ਉਛਾਲ;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->ਸਰੋਤ_len);
	ਜੇ (use < 0) {
		ਜਾਓ operror;
	}
	use++; /* does not count the null */
	ਜੇ (use > datasize)
		ਜਾਓ ਉਛਾਲ;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_ਸਮਾਂ);
	ਜੇ (use == 0)
		ਜਾਓ ਉਛਾਲ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_ਸਮਾਂ);
	ਜੇ (use == 0)
		ਜਾਓ ਉਛਾਲ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_ਸਮਾਂ);
	ਜੇ (use == 0)
		ਜਾਓ ਉਛਾਲ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->ਅੰਕ_ਸਿਰਲੇਖਾਂ);
	ਜੇ (use < 0) {
		ਜਾਓ operror;
	}
	use++; /* does not count the null */
	ਜੇ (use > datasize)
		ਜਾਓ ਉਛਾਲ;
	op += use;
	datasize -= use;

	/*ਸਿਰਲੇਖਾਂ*/
	ਲਈ (hloop = 0 ; hloop < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->ਸਿਰਲੇਖਾਂ[hloop].name,
			       object->ਸਿਰਲੇਖਾਂ[hloop].ਮੁੱਲ);
		ਜੇ (use < 0) {
			ਜਾਓ operror;
		}
		use++; /* does not count the null */
		ਜੇ (use > datasize)
			ਜਾਓ ਉਛਾਲ;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_ਡੂੰਘਾਈ);
	ਜੇ (use < 0) {
		ਜਾਓ operror;
	}
	use++; /* does not count the null */
	ਜੇ (use > datasize)
		ਜਾਓ ਉਛਾਲ;
	op += use;
	datasize -= use;

	/* SSL certificates */
	ਲਈ (hloop = 0; hloop < cert_chain_ਡੂੰਘਾਈ; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (ਪੂਰਨ)(object->chain->certs[hloop].err));
		ਜੇ (use < 0) {
			ਜਾਓ operror;
		}
		use++; /* does not count the null */
		ਜੇ (use > datasize)
			ਜਾਓ ਉਛਾਲ;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		ਜੇ (object->chain->certs[hloop].der != ਨਿਰਾ) {
			size_t output_ਲੰਮਾਈ = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_ਲੰਮਾਈ,
				(uint8_t *)op,
				&output_ਲੰਮਾਈ);
			ਜੇ (res != NSUERROR_OK) {
				ਜਾਓ operror;
			}
			use = output_ਲੰਮਾਈ;
		} ਨਹੀਂਤਾਂ {
			use = 0;
		}
		use++; /* allow for null */
		ਜੇ (use > datasize)
			ਜਾਓ ਉਛਾਲ;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*ਅੰਕੜੇ_ਬਾਹਰਲਾ = ਅੰਕੜੇ;
	*datasize_ਬਾਹਰਲਾ = allocsize - datasize;

	ਵਾਪਸ NSERROR_OK;

ਉਛਾਲ:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	ਮੁਕਤ(ਅੰਕੜੇ);
	ਵਾਪਸ NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	ਮੁਕਤ(ਅੰਕੜੇ);
	ਵਾਪਸ NSERROR_INVALID;
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
ਸਥਿਰ nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = ਨਿਰਾ;
	size_t metadatalen = 0;
	size_t ਬਚਿਆ = 0;
	nsurl *metadataurl;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਤਰ;
	ਅੱਖਰ *ln;
	ਪੂਰਨ lnsize;

	size_t ਸਰੋਤ_ਲੰਮਾਈ;
	time_t request_ਸਮਾਂ;
	time_t response_ਸਮਾਂ;
	time_t completion_ਸਮਾਂ;
	size_t ਅੰਕ_ਸਿਰਲੇਖਾਂ;
	size_t hloop;
	size_t ssl_cert_ਗਿਣਤੀ = 0;
	ਬਣਤਰ cert_chain *chain = ਨਿਰਾ;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	NSLOG(llcache, INFO, "Processing retrieved data");

	/* metadata is stored as a sequence of NULL terminated strings
	 * which we call 'line's here.
	 */

	/* We track remaining data because as we extend this data structure
	 * we need to know if we should continue to parse
	 */
	ਬਚਿਆ = metadatalen;

	/* metadata line 1 is the url the metadata referrs to */
	ਸਤਰ = 1;
	ln = (ਅੱਖਰ *)metadata;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	ਜੇ (lnsize < 7) {
		res = NSERROR_INVALID;
		ਜਾਓ format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	ਜੇ (res != NSERROR_OK)
		ਜਾਓ format_error;

	ਜੇ (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		ਵਾਪਸ NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	ਸਤਰ = 2;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	ਜੇ ((lnsize < 1) || (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ln, "%" PRIsizet, &ਸਰੋਤ_ਲੰਮਾਈ) != 1)) {
		res = NSERROR_INVALID;
		ਜਾਓ format_error;
	}


	/* metadata line 3 is the time of request */
	ਸਤਰ = 3;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_ਸਮਾਂ);
	ਜੇ (res != NSERROR_OK)
		ਜਾਓ format_error;


	/* metadata line 4 is the time of response */
	ਸਤਰ = 4;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_ਸਮਾਂ);
	ਜੇ (res != NSERROR_OK)
		ਜਾਓ format_error;


	/* metadata line 5 is the time of request completion */
	ਸਤਰ = 5;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_ਸਮਾਂ);
	ਜੇ (res != NSERROR_OK)
		ਜਾਓ format_error;


	/* metadata line 6 is the number of headers */
	ਸਤਰ = 6;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	ਜੇ ((lnsize < 1) || (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ln, "%" PRIsizet, &ਅੰਕ_ਸਿਰਲੇਖਾਂ) != 1)) {
		res = NSERROR_INVALID;
		ਜਾਓ format_error;
	}

	/* read headers */
	ਲਈ (hloop = 0 ; hloop < ਅੰਕ_ਸਿਰਲੇਖਾਂ; hloop++) {
		ਸਤਰ++;
		ln += lnsize + 1;
		lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
		ਬਚਿਆ -= lnsize + 1;

		res = llcache_fetch_process_ਸਿਰਲੇਖ(object,
						   (uint8_t *)ln,
						   lnsize);
		ਜੇ (res != NSERROR_OK)
			ਜਾਓ format_error;
	}

	ਜੇ (ਬਚਿਆ == 0) {
		ਜਾਓ skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	ਸਤਰ++;
	ln += lnsize + 1;
	lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
	ਬਚਿਆ -= lnsize + 1;

	ਜੇ ((lnsize < 1) || (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ln, "%" PRIsizet, &ssl_cert_ਗਿਣਤੀ) != 1)) {
		res = NSERROR_INVALID;
		ਜਾਓ format_error;
	}

	ਜੇ (ssl_cert_ਗਿਣਤੀ == 0) {
		ਜਾਓ skip_ssl_certificates;
	}

	ਜੇ (ssl_cert_ਗਿਣਤੀ > MAX_CERT_ਡੂੰਘਾਈ) {
		res = NSERROR_INVALID;
		ਜਾਓ format_error;
	}

	res = cert_chain_alloc(ssl_cert_ਗਿਣਤੀ, &chain);
	ਜੇ (res != NSERROR_OK) {
		ਜਾਓ format_error;
	}

	ਲਈ (hloop = 0; hloop < ssl_cert_ਗਿਣਤੀ; hloop++) {
		ਪੂਰਨ errcode;
		nsuerror nsures;

		/* Certificate error code */
		ਸਤਰ++;
		ln += lnsize + 1;
		lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
		ਬਚਿਆ -= lnsize + 1;
		ਜੇ ((lnsize < 1) || (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			ਜਾਓ format_error;
		}
		ਜੇ (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_ਗਿਆਤ) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} ਨਹੀਂਤਾਂ {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		ਸਤਰ++;
		ln += lnsize + 1;
		lnsize = ਲੜੀ_ਲੰਬਾਈ(ln);
		ਬਚਿਆ -= lnsize + 1;
		ਜੇ (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ਅਚਲ uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_ਲੰਮਾਈ);
			ਜੇ (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				ਜਾਓ format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->ਸਰੋਤ_len = ਸਰੋਤ_ਲੰਮਾਈ;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->ਸਰੋਤ_alloc = metadatalen;

	object->cache.req_ਸਮਾਂ = request_ਸਮਾਂ;
	object->cache.res_ਸਮਾਂ = response_ਸਮਾਂ;
	object->cache.fin_ਸਮਾਂ = completion_ਸਮਾਂ;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	ਵਾਪਸ NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      ਸਤਰ, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	ਵਾਪਸ res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool llcache__scheme_is_persistable(ਅਚਲ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool ਮੇਲ;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ਜੇ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	ਵਾਪਸ persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool llcache__scheme_is_cachable(ਅਚਲ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool ਮੇਲ;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ਜੇ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ਅੰਕੜੇ,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&ਮੇਲ) == lwc_error_ok &&
			(ਮੇਲ == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	ਵਾਪਸ cachable;
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
ਸਥਿਰ nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
				uint32_t redirect_ਗਿਣਤੀ)
{
	nserror error;
	nsurl *referer_clone = ਨਿਰਾ;
	llcache_post_ਅੰਕੜੇ *post_clone = ਨਿਰਾ;

	ਜੇ (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	object->cache.req_ਸਮਾਂ = time(ਨਿਰਾ);
	object->cache.fin_ਸਮਾਂ = object->cache.req_ਸਮਾਂ;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	ਜੇ (error != NSERROR_OK) {
		ਵਾਪਸ error;
	}

	/* entry came out of cache - need to setup object state */
	ਜੇ (post != ਨਿਰਾ) {
		error = llcache_post_ਅੰਕੜੇ_clone(post, &post_clone);
		ਜੇ (error != NSERROR_OK)
			ਵਾਪਸ error;
	}

	ਜੇ (referer != ਨਿਰਾ) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ਗਿਣਤੀ = redirect_ਗਿਣਤੀ;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ਨਿਰਾ;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
				   uint32_t redirect_ਗਿਣਤੀ,
				   bool hsts_ਵਿੱਚ_use,
				   llcache_object **ਨਤੀਜਾ)
{
	nserror error;
	llcache_object *obj, *newest = ਨਿਰਾ;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==ਨਿਰਾ?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	ਲਈ (obj = llcache->cached_objects; obj != ਨਿਰਾ; obj = obj->ਅਗਲਾ) {

		ਜੇ ((newest == ਨਿਰਾ ||
		     obj->cache.req_ਸਮਾਂ > newest->cache.req_ਸਮਾਂ) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	ਜੇ (newest == ਨਿਰਾ) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		ਜੇ (error != NSERROR_OK)
			ਵਾਪਸ error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_ਗਿਣਤੀ);
		ਜੇ (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	ਜੇ ((newest != ਨਿਰਾ) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ਅੰਕੜੇ(newest);
		ਜੇ (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*ਨਤੀਜਾ = newest;

			ਵਾਪਸ NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		ਜੇ (error != NSERROR_OK) {
			ਵਾਪਸ error;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (newest != ਨਿਰਾ) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ਅੰਕੜੇ(newest);
		ਜੇ (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			ਜੇ (error != NSERROR_OK)
				ਵਾਪਸ error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_ਅੰਕੜੇ(newest, obj, true);
			ਜੇ (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				ਵਾਪਸ error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_ਗਿਣਤੀ++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_ਗਿਣਤੀ, hsts_ਵਿੱਚ_use);
			ਜੇ (error != NSERROR_OK) {
				newest->candidate_ਗਿਣਤੀ--;
				llcache_object_destroy(obj);
				ਵਾਪਸ error;
			}

			/* Add new object to cache */
			llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(obj, &llcache->cached_objects);

			*ਨਤੀਜਾ = obj;

			ਵਾਪਸ NSERROR_OK;
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
		ਜੇ (error != NSERROR_OK) {
			ਵਾਪਸ error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_ਗਿਣਤੀ, hsts_ਵਿੱਚ_use);
	ਜੇ (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		ਵਾਪਸ error;
	}

	/* Add new object to cache */
	llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(obj, &llcache->cached_objects);

	*ਨਤੀਜਾ = obj;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
			uint32_t redirect_ਗਿਣਤੀ,
			bool hsts_ਵਿੱਚ_use,
			llcache_object **ਨਤੀਜਾ)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==ਨਿਰਾ?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	/* determine if content is cachable */
	ਜੇ ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} ਨਹੀਂਤਾਂ ਜੇ (post != ਨਿਰਾ) {
		/* POST requests are never cached */
		uncachable = true;
	} ਨਹੀਂਤਾਂ {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	ਜੇ (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		ਜੇ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ਵਾਪਸ error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_ਗਿਣਤੀ, hsts_ਵਿੱਚ_use);
		ਜੇ (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			ਵਾਪਸ error;
		}

		/* Add new object to uncached list */
		llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(obj, &llcache->uncached_objects);
	} ਨਹੀਂਤਾਂ {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_ਗਿਣਤੀ,
				hsts_ਵਿੱਚ_use, &obj);
		ਜੇ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ਵਾਪਸ error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*ਨਤੀਜਾ = obj;

	nsurl_unref(defragmented_url);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
ਸਥਿਰ nserror llcache_object_ਜੋੜੋ_user(llcache_object *object,
		llcache_object_user *user)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(user->ਅਗਲਾ == ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(user->prev == ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(user->handle != ਨਿਰਾ);

	user->handle->object = object;

	user->prev = ਨਿਰਾ;
	user->ਅਗਲਾ = object->users;

	ਜੇ (object->users != ਨਿਰਾ)
		object->users->prev = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_hsts_transform_url(nsurl *url, nsurl **ਨਤੀਜਾ,
		bool *hsts_ਵਿੱਚ_use)
{
	lwc_string *scheme = ਨਿਰਾ;
	bool ਮੇਲ;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ਜੇ (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&ਮੇਲ) != lwc_error_ok || ਮੇਲ == false) {
		/* Non-HTTP fetch: no transform required */
		ਜੇ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&ਮੇਲ) == lwc_error_ok && ਮੇਲ) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_ਵਿੱਚ_use = urldb_get_hsts_enabled(url);
		} ਨਹੀਂਤਾਂ {
			/* Anything else: no HSTS */
			*hsts_ਵਿੱਚ_use = false;
		}
		lwc_string_unref(scheme);
		*ਨਤੀਜਾ = nsurl_ref(url);
		ਵਾਪਸ error;
	}
	lwc_string_unref(scheme);

	ਜੇ (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				ਨਤੀਜਾ);
		*hsts_ਵਿੱਚ_use = (error == NSERROR_OK);
	} ਨਹੀਂਤਾਂ {
		*ਨਤੀਜਾ = nsurl_ref(url);
		*hsts_ਵਿੱਚ_use = false;
	}

	ਵਾਪਸ error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = ਨਿਰਾ;
	bool ਮੇਲ = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	ਜੇ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&ਮੇਲ) != lwc_error_ok || ਮੇਲ == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		ਵਾਪਸ NSERROR_OK;
	}
	lwc_string_unref(scheme);

	ਜੇ (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		ਵਾਪਸ NSERROR_OK;
	}

	ਲਈ (i = 0; i < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ; i++) {
		ਜੇ (strcasecmp("Strict-Transport-Security",
				object->ਸਿਰਲੇਖਾਂ[i].name) == 0) {
			urldb_ਸੈੱਟ_hsts_policy(object->url,
					object->ਸਿਰਲੇਖਾਂ[i].ਮੁੱਲ);
			/* Only process the first one we find */
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool ਮੇਲ;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	ਜੇ ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&ਮੇਲ) == lwc_error_ok && ਮੇਲ == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&ਮੇਲ) == lwc_error_ok && ਮੇਲ == false)) {
		/* file, about and resource are not valid redirect targets */
		ਜੇ ((lwc_string_isequal(scheme, corestring_lwc_file,
				&ਮੇਲ) == lwc_error_ok && ਮੇਲ == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&ਮੇਲ) == lwc_error_ok && ਮੇਲ == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&ਮੇਲ) == lwc_error_ok && ਮੇਲ == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			ਵਾਪਸ NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	ਜੇ (fetch_can_fetch(dst_url) == false) {
		ਵਾਪਸ NSERROR_UNSAFE_REDIRECT;
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_fetch_redirect(llcache_object *object,
		ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *ਅਗਲਾ;
	ਅਚਲ llcache_post_ਅੰਕੜੇ *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_ਵਿੱਚ_use;
	http_response_ਕੋਡ http_ਕੋਡ;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_ਕੋਡ = fetch_http_ਕੋਡ(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ਨਿਰਾ;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ਖਾਲੀ) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	ਜੇ (object->fetch.redirect_ਗਿਣਤੀ > nsoption_uint(fetch_redirect_ਹੱਦਬੰਦੀ)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		ਵਾਪਸ NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, ਨਿਸ਼ਾਨਾ, &url);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_ਵਿੱਚ_use);
	nsurl_unref(url);
	ਜੇ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		ਵਾਪਸ res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	ਜੇ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		ਵਾਪਸ res;
	}

	/* alter behaviour based on specific redirect code */
	ਚੋਣ (http_ਕੋਡ) {
	ਹਾਲਤ HTTP_RESPONSE_MOVED_PERMANENTLY:
	ਹਾਲਤ HTTP_RESPONSE_ਮਿਲਿਆ:
	ਹਾਲਤ HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = ਨਿਰਾ;
		ਰੋਕੋ;

	ਹਾਲਤ HTTP_RESPONSE_TEMPORARY_REDIRECT:
	ਹਾਲਤ HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		ਰੋਕੋ;

	ਮੂਲ:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_ਕੋਡ);
		nsurl_unref(hsts_url);
		ਵਾਪਸ NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.ਅੰਕੜੇ.redirect.from = object->url;
	event.ਅੰਕੜੇ.redirect.ਮੰਜ਼ਿਲ = hsts_url;

	res = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
	ਜੇ (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ਵਾਪਸ res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_ਗਿਣਤੀ + 1,
					hsts_ਵਿੱਚ_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	ਜੇ (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		ਲਈ (user = object->users; user != ਨਿਰਾ; user = ਅਗਲਾ) {
			ਅਗਲਾ = user->ਅਗਲਾ;

			llcache_object_remove_user(object, user);
			llcache_object_ਜੋੜੋ_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	ਵਾਪਸ res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
ਸਥਿਰ nserror llcache_object_cache_update(llcache_object *object)
{
	ਜੇ (object->cache.date == 0)
		object->cache.date = time(ਨਿਰਾ);

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
ਸਥਿਰ nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	ਜੇ (object->candidate != ਨਿਰਾ) {
		llcache_object_user *user, *ਅਗਲਾ;

		/* Move user(s) to candidate content */
		ਲਈ (user = object->users; user != ਨਿਰਾ; user = ਅਗਲਾ) {
			ਅਗਲਾ = user->ਅਗਲਾ;

			llcache_object_remove_user(object, user);
			llcache_object_ਜੋੜੋ_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_ਗਿਣਤੀ--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_ਅੰਕੜੇ(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		ਜੇ (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ਨਿਰਾ;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ਖਾਲੀ) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ਸਥਿਰ nserror
llcache_fetch_process_ਅੰਕੜੇ(llcache_object *object,
			   ਅਚਲ uint8_t *ਅੰਕੜੇ,
			   size_t len)
{
	ਜੇ (object->fetch.state != LLCACHE_FETCH_ਅੰਕੜੇ) {
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
		ਲੰਮਾ http_ਕੋਡ = fetch_http_ਕੋਡ(object->fetch.fetch);

		ਜੇ ((http_ਕੋਡ != 200 && http_ਕੋਡ != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ਅੰਕੜੇ(object);
		}

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		object->fetch.state = LLCACHE_FETCH_ਅੰਕੜੇ;
	}

	/* Resize source buffer if it's too small */
	ਜੇ (object->ਸਰੋਤ_len + len >= object->ਸਰੋਤ_alloc) {
		ਅਚਲ size_t new_len = object->ਸਰੋਤ_len + len + 64 * 1024;
		uint8_t *ਆਰਜ਼ੀ = realloc(object->ਸਰੋਤ_ਅੰਕੜੇ, new_len);
		ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		object->ਸਰੋਤ_ਅੰਕੜੇ = ਆਰਜ਼ੀ;
		object->ਸਰੋਤ_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(object->ਸਰੋਤ_ਅੰਕੜੇ + object->ਸਰੋਤ_len, ਅੰਕੜੇ, len);
	object->ਸਰੋਤ_len += len;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ਸਥਿਰ nserror llcache_fetch_auth(llcache_object *object, ਅਚਲ ਅੱਖਰ *realm)
{
	ਅਚਲ ਅੱਖਰ *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ਨਿਰਾ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	/* Destroy headers */
	llcache_destroy_ਸਿਰਲੇਖਾਂ(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	ਜੇ (realm == ਨਿਰਾ)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	ਜੇ (auth == ਨਿਰਾ || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_BAD_AUTH;
		event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = realm;

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
	} ਨਹੀਂਤਾਂ {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	ਵਾਪਸ error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ਨਿਰਾ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	ਜੇ (object->fetch.hsts_ਵਿੱਚ_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_BAD_CERTS;
		event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = messages_get("FetchFailed");

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
	} ਨਹੀਂਤਾਂ {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_UNKNOWN;
		event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = messages_get("FetchFailed");

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
	}

	ਵਾਪਸ error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ਨਿਰਾ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	ਜੇ (object->fetch.hsts_ਵਿੱਚ_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_UNKNOWN;
		event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = messages_get("FetchFailed");

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
	} ਨਹੀਂਤਾਂ {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	ਵਾਪਸ error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
ਸਥਿਰ nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	ਜੇ (object->fetch.retries_ਬਚਿਆ > 1) {
		object->fetch.retries_ਬਚਿਆ--;
		ਵਾਪਸ llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ਨਿਰਾ;

	/* Release candidate, if any */
	ਜੇ (object->candidate != ਨਿਰਾ) {
		object->candidate->candidate_ਗਿਣਤੀ--;
		object->candidate = ਨਿਰਾ;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_TIMEOUT;
	event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = ਨਿਰਾ;

	ਵਾਪਸ llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
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
ਸਥਿਰ nserror
build_candidate_list(ਬਣਤਰ llcache_object ***lst_ਬਾਹਰਲਾ, ਪੂਰਨ *lst_len_ਬਾਹਰਲਾ)
{
	llcache_object *object, *ਅਗਲਾ;
	ਬਣਤਰ llcache_object **lst;
	ਪੂਰਨ lst_len = 0;
	ਪੂਰਨ ਬਚਿਆ_lifetime;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_PERSIST_ਪ੍ਰਤੀ_ਲੜੀਵਾਰ 128

	lst = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(MAX_PERSIST_ਪ੍ਰਤੀ_ਲੜੀਵਾਰ, ਆਕਾਰ(ਬਣਤਰ llcache_object *));
	ਜੇ (lst == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਲਈ (object = llcache->cached_objects; object != ਨਿਰਾ; object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		/* Only consider http(s) for the disc cache. */
		ਜੇ (!llcache__scheme_is_persistable(object->url)) {
			ਜਾਰੀ;
		}

		ਬਚਿਆ_lifetime = llcache_object_rfc2616_ਬਚਿਆ_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		ਜੇ ((object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (ਬਚਿਆ_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			ਜੇ (lst_len == MAX_PERSIST_ਪ੍ਰਤੀ_ਲੜੀਵਾਰ)
				ਰੋਕੋ;
		}
	}

	ਜੇ (lst_len == 0) {
		ਮੁਕਤ(lst);
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;
	}

	/** \todo sort list here */

	*lst_len_ਬਾਹਰਲਾ = lst_len;
	*lst_ਬਾਹਰਲਾ = lst;

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MAX_PERSIST_ਪ੍ਰਤੀ_ਲੜੀਵਾਰ

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
ਸਥਿਰ nserror
write_backing_store(ਬਣਤਰ llcache_object *object,
		    size_t *written_ਬਾਹਰਲਾ,
		    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *elapsed)
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
				   object->ਸਰੋਤ_ਅੰਕੜੇ,
				   object->ਸਰੋਤ_len);
	ਜੇ (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		ਵਾਪਸ ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	ਜੇ (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ਵਾਪਸ ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	ਜੇ (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ਵਾਪਸ ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_ਬਾਹਰਲਾ = object->ਸਰੋਤ_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	ਜੇ (*elapsed == 0) {
		*elapsed = 1;
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ ਖਾਲੀ llcache_persist_slowcheck(ਖਾਲੀ *p)
{
	uint64_t ਕੁੱਲ_bandwidth; /* total bandwidth */

	ਜੇ (llcache->ਕੁੱਲ_written > (2 * llcache->minimum_bandwidth)) {

		ਕੁੱਲ_bandwidth = (llcache->ਕੁੱਲ_written * 1000) / llcache->ਕੁੱਲ_elapsed;

		ਜੇ (ਕੁੱਲ_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      ਕੁੱਲ_bandwidth,
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
ਸਥਿਰ ਖਾਲੀ llcache_persist(ਖਾਲੀ *p)
{
	nserror ret;
	ਬਣਤਰ llcache_object **lst; /* candidate object list */
	ਪੂਰਨ lst_ਗਿਣਤੀ; /* number of candidates in list */
	ਪੂਰਨ idx; /* current candidate object index in list */
	ਪੂਰਨ ਅਗਲਾ = -1; /* when the next run should be scheduled for */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ write_ਹੱਦਬੰਦੀ; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ elapsed; /* how long writing an object took */

	size_t ਕੁੱਲ_written = 0; /* total bytes written in this run */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ_elapsed = 1; /* total ms used to write bytes */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_ਗਿਣਤੀ);
	ਜੇ (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		ਵਾਪਸ;
	}

	write_ਹੱਦਬੰਦੀ = (llcache->ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth * llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	ਲਈ (idx = 0; idx < lst_ਗਿਣਤੀ; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		ਜੇ (ret != NSERROR_OK) {
			ਜਾਰੀ;
		}

		/* successfully wrote object to backing store */
		ਕੁੱਲ_written += written;
		ਕੁੱਲ_elapsed += elapsed;
		ਕੁੱਲ_bandwidth = (ਕੁੱਲ_written * 1000) / ਕੁੱਲ_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		ਜੇ (ਕੁੱਲ_elapsed > llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			ਜੇ (ਕੁੱਲ_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(
						llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ * 100,
						llcache_persist_slowcheck,
						ਨਿਰਾ);
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (ਕੁੱਲ_bandwidth > llcache->ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					ਅਗਲਾ = ((ਕੁੱਲ_written * llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ) / write_ਹੱਦਬੰਦੀ) - ਕੁੱਲ_elapsed;
				} ਨਹੀਂਤਾਂ {
					ਅਗਲਾ = llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ;
				}
				ਰੋਕੋ;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (ਕੁੱਲ_written > write_ਹੱਦਬੰਦੀ) {
			/* The bandwidth limit has been reached. */

			ਜੇ (ਕੁੱਲ_bandwidth > llcache->ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				ਅਗਲਾ = ((ਕੁੱਲ_written * llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ) / write_ਹੱਦਬੰਦੀ) - ਕੁੱਲ_elapsed;
			} ਨਹੀਂਤਾਂ {
				ਅਗਲਾ = llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ - ਕੁੱਲ_elapsed;
			}
			ਰੋਕੋ;
		}

	}
	ਮੁਕਤ(lst);

	/* Completed list without running out of allowed bytes or time */
	ਜੇ (idx == lst_ਗਿਣਤੀ) {
		/* only reschedule if writing is making any progress at all */
		ਜੇ (ਕੁੱਲ_written > 0) {
			ਅਗਲਾ = llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ - ਕੁੱਲ_elapsed;
		} ਨਹੀਂਤਾਂ {
			ਅਗਲਾ = -1;
		}
	}

	llcache->ਕੁੱਲ_written += ਕੁੱਲ_written;
	llcache->ਕੁੱਲ_elapsed += ਕੁੱਲ_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      ਕੁੱਲ_written, ਕੁੱਲ_elapsed, ਕੁੱਲ_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", ਅਗਲਾ);
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(ਅਗਲਾ, llcache_persist, ਨਿਰਾ);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
ਸਥਿਰ ਖਾਲੀ llcache_fetch_callback(ਅਚਲ fetch_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ, ਖਾਲੀ *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	ਜੇ (llcache == ਨਿਰਾ) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		ਪ੍ਰਤਿਜ੍ਞਾ(false);
		/* In case assertions are off, return here */
		ਵਾਪਸ;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", ਸੁਨੇਹਾ->type, object);

	ਚੋਣ (ਸੁਨੇਹਾ->type) {
	ਹਾਲਤ FETCH_ਸਿਰਲੇਖ:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ;

		error = llcache_fetch_process_ਸਿਰਲੇਖ(object,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len);
		ਰੋਕੋ;

	/* 3xx responses */
	ਹਾਲਤ FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		error = llcache_fetch_redirect(object,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.redirect, &object);
		ਜੇ (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.ਅੰਕੜੇ.error.ਕੋਡ = error;
			event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = ਨਿਰਾ;
			error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);
		}
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		ਰੋਕੋ;

	/* Normal 2xx state machine */
	ਹਾਲਤ FETCH_ਅੰਕੜੇ:
		/* Received some data */
		error = llcache_fetch_process_ਅੰਕੜੇ(object,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.buf,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਸਿਰਲੇਖ_or_ਅੰਕੜੇ.len);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_ਮੁਕੰਮਲ:
		/* Finished fetching */
	{
		uint8_t *ਆਰਜ਼ੀ;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ਨਿਰਾ;

		/* Shrink source buffer to required size */
		ਆਰਜ਼ੀ = realloc(object->ਸਰੋਤ_ਅੰਕੜੇ,
				object->ਸਰੋਤ_len);
		/* If source_len is 0, then temp may be NULL */
		ਜੇ (ਆਰਜ਼ੀ != ਨਿਰਾ || object->ਸਰੋਤ_len == 0) {
			object->ਸਰੋਤ_ਅੰਕੜੇ = ਆਰਜ਼ੀ;
			object->ਸਰੋਤ_alloc = object->ਸਰੋਤ_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_ਸਮਾਂ = time(ਨਿਰਾ);

		(ਖਾਲੀ) llcache_hsts_update_policy(object);

		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(5000, llcache_persist, ਨਿਰਾ);
	}
		ਰੋਕੋ;

	/* Out-of-band information */
	ਹਾਲਤ FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ਨਿਰਾ;

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.ਅੰਕੜੇ.error.ਕੋਡ = NSERROR_UNKNOWN;
		event.ਅੰਕੜੇ.error.ਸੁਨੇਹਾ = ਸੁਨੇਹਾ->ਅੰਕੜੇ.error;

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);

		ਰੋਕੋ;

	ਹਾਲਤ FETCH_ਤਰੱਕੀ:
		/* Progress update */
		event.type = LLCACHE_EVENT_ਤਰੱਕੀ;
		event.ਅੰਕੜੇ.ਤਰੱਕੀ_ਸੁਨੇਹਾ = ਸੁਨੇਹਾ->ਅੰਕੜੇ.ਤਰੱਕੀ;

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);

		ਰੋਕੋ;

	ਹਾਲਤ FETCH_CERTS:
		/* Certificate information from the fetch */

		ਜੇ (object->chain != ਨਿਰਾ) {
			cert_chain_free(object->chain);
			object->chain = ਨਿਰਾ;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(ਸੁਨੇਹਾ->ਅੰਕੜੇ.chain, &object->chain);
		ਜੇ (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.ਅੰਕੜੇ.chain = ਸੁਨੇਹਾ->ਅੰਕੜੇ.chain;

		error = llcache_send_event_ਮੰਜ਼ਿਲ_users(object, &event);

		ਰੋਕੋ;

	/* Events requiring action */
	ਹਾਲਤ FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		error = llcache_fetch_auth(object, ਸੁਨੇਹਾ->ਅੰਕੜੇ.auth.realm);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		error = llcache_fetch_cert_error(object);
		ਰੋਕੋ;

	ਹਾਲਤ FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		ਜੇ (object->candidate != ਨਿਰਾ) {
			object->candidate->candidate_ਗਿਣਤੀ--;
			object->candidate = ਨਿਰਾ;
		}

		error = llcache_fetch_ssl_error(object);
		ਰੋਕੋ;
	}

	/* Deal with any errors reported by event handlers */
	ਜੇ (error != NSERROR_OK) {
		ਜੇ (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		ਜੇ (object->fetch.fetch != ਨਿਰਾ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ਨਿਰਾ;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ਅੰਕੜੇ(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_ਉੱਪਰ();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
ਸਥਿਰ llcache_object_user *llcache_object_ਲੱਭੋ_user(
		ਅਚਲ llcache_handle *handle)
{
	llcache_object_user *user;

	ਪ੍ਰਤਿਜ੍ਞਾ(handle->object != ਨਿਰਾ);

	ਲਈ (user = handle->object->users; user != ਨਿਰਾ; user = user->ਅਗਲਾ) {
		ਜੇ (user->handle == handle)
			ਰੋਕੋ;
	}

	ਵਾਪਸ user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
ਸਥਿਰ bool llcache_object_ਵਿੱਚ_list(ਅਚਲ llcache_object *object,
		ਅਚਲ llcache_object *list)
{
	ਜਦੋਂ (list != ਨਿਰਾ) {
		ਜੇ (list == object)
			ਰੋਕੋ;

		list = list->ਅਗਲਾ;
	}

	ਵਾਪਸ list != ਨਿਰਾ;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *ਅਗਲਾ_user;
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

	ਲਈ (user = object->users; user != ਨਿਰਾ; user = ਅਗਲਾ_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ਅਚਲ llcache_fetch_state objstate = object->fetch.state;

		/* Flag that this user is the current iteration target
		 * in case the client attempts to destroy it underneath us */
		user->iterator_ਨਿਸ਼ਾਨਾ = true;

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

		ਜੇ (handle->state != objstate) {
			ਜੇ (emitted_notify == false) {
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
		ਜੇ (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ;

			/* Emit any certificate data we hold */
			ਜੇ (object->chain != ਨਿਰਾ) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.ਅੰਕੜੇ.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} ਨਹੀਂਤਾਂ {
				error = NSERROR_OK;
			}

			ਜੇ (user->queued_for_delete) {
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ਜੇ (error != NSERROR_OK)
					ਵਾਪਸ error;

				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error == NSERROR_NEED_ਅੰਕੜੇ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_users_not_caught_ਉੱਪਰ();
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error != NSERROR_OK) {
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਵਾਪਸ error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		ਜੇ (handle->state == LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ &&
				objstate > LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ) {
			handle->state = LLCACHE_FETCH_ਅੰਕੜੇ;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_ਸਿਰਲੇਖਾਂ;

			error = handle->cb(handle, &event, handle->pw);

			ਜੇ (user->queued_for_delete) {
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ਜੇ (error != NSERROR_OK)
					ਵਾਪਸ error;

				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error == NSERROR_NEED_ਅੰਕੜੇ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ਸਿਰਲੇਖਾਂ;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_users_not_caught_ਉੱਪਰ();
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error != NSERROR_OK) {
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਵਾਪਸ error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		ਜੇ (handle->state == LLCACHE_FETCH_ਅੰਕੜੇ &&
				objstate >= LLCACHE_FETCH_ਅੰਕੜੇ &&
				object->ਸਰੋਤ_len > handle->ਬਾਈਟਾਂ) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_ਅੰਕੜੇ;
			event.ਅੰਕੜੇ.ਅੰਕੜੇ.buf =
					object->ਸਰੋਤ_ਅੰਕੜੇ + handle->ਬਾਈਟਾਂ;
			event.ਅੰਕੜੇ.ਅੰਕੜੇ.len =
					object->ਸਰੋਤ_len - handle->ਬਾਈਟਾਂ;

			/* Update record of last byte emitted */
			ਜੇ (object->fetch.flags &
					LLCACHE_RETRIEVE_ਧਾਰਾ_ਅੰਕੜੇ) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->ਬਾਈਟਾਂ = object->ਸਰੋਤ_len = 0;
			} ਨਹੀਂਤਾਂ {
				orig_handle_read = handle->ਬਾਈਟਾਂ;
				handle->ਬਾਈਟਾਂ = object->ਸਰੋਤ_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			ਜੇ (user->queued_for_delete) {
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ਜੇ (error != NSERROR_OK)
					ਵਾਪਸ error;

				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error == NSERROR_NEED_ਅੰਕੜੇ) {
				/* User requested replay */
				handle->ਬਾਈਟਾਂ = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_users_not_caught_ਉੱਪਰ();
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error != NSERROR_OK) {
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਵਾਪਸ error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		ਜੇ (handle->state == LLCACHE_FETCH_ਅੰਕੜੇ &&
				objstate > LLCACHE_FETCH_ਅੰਕੜੇ) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_ਪੂਰਾ;

			error = handle->cb(handle, &event, handle->pw);
			ਜੇ (user->queued_for_delete) {
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ਜੇ (error != NSERROR_OK)
					ਵਾਪਸ error;

				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error == NSERROR_NEED_ਅੰਕੜੇ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ਅੰਕੜੇ;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਅਗਲਾ_user = user->ਅਗਲਾ;
				llcache_users_not_caught_ਉੱਪਰ();
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ ਜੇ (error != NSERROR_OK) {
				user->iterator_ਨਿਸ਼ਾਨਾ = false;
				ਵਾਪਸ error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_ਨਿਸ਼ਾਨਾ = false;

		ਅਗਲਾ_user = user->ਅਗਲਾ;
	}

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	newobj->ਸਰੋਤ_alloc = newobj->ਸਰੋਤ_len = object->ਸਰੋਤ_len;

	ਜੇ (object->ਸਰੋਤ_len > 0) {
		newobj->ਸਰੋਤ_ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(newobj->ਸਰੋਤ_alloc);
		ਜੇ (newobj->ਸਰੋਤ_ਅੰਕੜੇ == ਨਿਰਾ) {
			llcache_object_destroy(newobj);
			ਵਾਪਸ NSERROR_NOMEM;
		}
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(newobj->ਸਰੋਤ_ਅੰਕੜੇ, object->ਸਰੋਤ_ਅੰਕੜੇ,
				newobj->ਸਰੋਤ_len);
	}

	ਜੇ (object->ਅੰਕ_ਸਿਰਲੇਖਾਂ > 0) {
		newobj->ਸਿਰਲੇਖਾਂ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(object->ਅੰਕ_ਸਿਰਲੇਖਾਂ,
				ਆਕਾਰ(llcache_ਸਿਰਲੇਖ));
		ਜੇ (newobj->ਸਿਰਲੇਖਾਂ == ਨਿਰਾ) {
			llcache_object_destroy(newobj);
			ਵਾਪਸ NSERROR_NOMEM;
		}
		ਜਦੋਂ (newobj->ਅੰਕ_ਸਿਰਲੇਖਾਂ < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ) {
			llcache_ਸਿਰਲੇਖ *nh =
					&(newobj->ਸਿਰਲੇਖਾਂ[newobj->ਅੰਕ_ਸਿਰਲੇਖਾਂ]);
			llcache_ਸਿਰਲੇਖ *oh =
					&(object->ਸਿਰਲੇਖਾਂ[newobj->ਅੰਕ_ਸਿਰਲੇਖਾਂ]);
			newobj->ਅੰਕ_ਸਿਰਲੇਖਾਂ += 1;
			nh->name = strdup(oh->name);
			nh->ਮੁੱਲ = strdup(oh->ਮੁੱਲ);
			ਜੇ (nh->name == ਨਿਰਾ || nh->ਮੁੱਲ == ਨਿਰਾ) {
				llcache_object_destroy(newobj);
				ਵਾਪਸ NSERROR_NOMEM;
			}
		}
	}

	ਜੇ (object->chain != ਨਿਰਾ) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		ਜੇ (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			ਵਾਪਸ error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ uint32_t
ਕੁੱਲ_object_ਮਾਪ(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = ਆਕਾਰ(*object);
	tot += nsurl_ਲੰਮਾਈ(object->url);

	ਜੇ (object->ਸਰੋਤ_ਅੰਕੜੇ != ਨਿਰਾ) {
		tot += object->ਸਰੋਤ_len;
	}

	tot += ਆਕਾਰ(llcache_ਸਿਰਲੇਖ) * object->ਅੰਕ_ਸਿਰਲੇਖਾਂ;

	ਲਈ (hdrc = 0; hdrc < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ; hdrc++) {
		ਜੇ (object->ਸਿਰਲੇਖਾਂ[hdrc].name != ਨਿਰਾ) {
			tot += ਲੜੀ_ਲੰਬਾਈ(object->ਸਿਰਲੇਖਾਂ[hdrc].name);
		}
		ਜੇ (object->ਸਿਰਲੇਖਾਂ[hdrc].ਮੁੱਲ != ਨਿਰਾ) {
			tot += ਲੜੀ_ਲੰਬਾਈ(object->ਸਿਰਲੇਖਾਂ[hdrc].ਮੁੱਲ);
		}
	}

	tot += cert_chain_ਮਾਪ(object->chain);

	ਵਾਪਸ tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
ਸਥਿਰ ਖਾਲੀ llcache_catch_ਉੱਪਰ_all_users(ਖਾਲੀ *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_ਉੱਪਰ = true;

	/* Catch new users up with state of objects */
	ਲਈ (object = llcache->cached_objects; object != ਨਿਰਾ;
			object = object->ਅਗਲਾ) {
		llcache_object_notify_users(object);
	}

	ਲਈ (object = llcache->uncached_objects; object != ਨਿਰਾ;
			object = object->ਅਗਲਾ) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
ਸਥਿਰ ਖਾਲੀ llcache_users_not_caught_ਉੱਪਰ(ਖਾਲੀ)
{
	ਜੇ (llcache->all_caught_ਉੱਪਰ) {
		llcache->all_caught_ਉੱਪਰ = false;
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(0, llcache_catch_ਉੱਪਰ_all_users, ਨਿਰਾ);
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
ਖਾਲੀ llcache_clean(bool purge)
{
	llcache_object *object, *ਅਗਲਾ;
	uint32_t llcache_ਮਾਪ = 0;
	ਪੂਰਨ ਬਚਿਆ_lifetime;
	uint32_t ਹੱਦਬੰਦੀ;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	ਜੇ (purge) {
		ਹੱਦਬੰਦੀ = 0;
	} ਨਹੀਂਤਾਂ {
		ਹੱਦਬੰਦੀ = llcache->ਹੱਦਬੰਦੀ;
	}

	/* Uncacheable objects with no users or fetches */
	ਲਈ (object = llcache->uncached_objects;
	     object != ਨਿਰਾ;
	     object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		/* The candidate count of uncacheable objects is always 0 */
		ਜੇ ((object->users == ਨਿਰਾ) &&
		    (object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} ਨਹੀਂਤਾਂ {
			llcache_ਮਾਪ += ਕੁੱਲ_object_ਮਾਪ(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	ਲਈ (object = llcache->cached_objects;
	     object != ਨਿਰਾ;
	     object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		ਬਚਿਆ_lifetime = llcache_object_rfc2616_ਬਚਿਆ_lifetime(
				&object->cache);

		ਜੇ ((object->users == ਨਿਰਾ) &&
		    (object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ) &&
		    (ਬਚਿਆ_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				ਜੇ (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} ਨਹੀਂਤਾਂ {
			/* object has users so account for the storage */
			llcache_ਮਾਪ += ਕੁੱਲ_object_ਮਾਪ(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	ਜੇ (ਹੱਦਬੰਦੀ < llcache_ਮਾਪ) {
		llcache_persist(ਨਿਰਾ);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	ਲਈ (object = llcache->cached_objects;
	     ((ਹੱਦਬੰਦੀ < llcache_ਮਾਪ) && (object != ਨਿਰਾ));
	     object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;
		ਜੇ ((object->users == ਨਿਰਾ) &&
		    (object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->ਸਰੋਤ_ਅੰਕੜੇ = ਨਿਰਾ;

			llcache_ਮਾਪ -=	object->ਸਰੋਤ_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->ਸਰੋਤ_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	ਲਈ (object = llcache->cached_objects;
	     ((ਹੱਦਬੰਦੀ < llcache_ਮਾਪ) && (object != ਨਿਰਾ));
	     object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;
		ਜੇ ((object->users == ਨਿਰਾ) &&
		    (object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->ਸਰੋਤ_ਅੰਕੜੇ == ਨਿਰਾ)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ਸਰੋਤ_len,
			      (ਲੰਮਾ)(time(ਨਿਰਾ) - object->ਆਖਰੀ_ਵਰਤਿਆ),
			      object,
			      nsurl_access(object->url));

			llcache_ਮਾਪ -=	ਕੁੱਲ_object_ਮਾਪ(object);

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
	ਲਈ (object = llcache->cached_objects;
	     ((ਹੱਦਬੰਦੀ < llcache_ਮਾਪ) && (object != ਨਿਰਾ));
	     object = ਅਗਲਾ) {
		ਅਗਲਾ = object->ਅਗਲਾ;

		ਜੇ ((object->users == ਨਿਰਾ) &&
		    (object->candidate_ਗਿਣਤੀ == 0) &&
		    (object->fetch.fetch == ਨਿਰਾ) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ਸਰੋਤ_len,
			      (ਲੰਮਾ)(time(ਨਿਰਾ) - object->ਆਖਰੀ_ਵਰਤਿਆ),
			      object,
			      nsurl_access(object->url));

			llcache_ਮਾਪ -=	object->ਸਰੋਤ_len + ਆਕਾਰ(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_ਮਾਪ, ਹੱਦਬੰਦੀ);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ਅਚਲ ਬਣਤਰ llcache_parameters *prm)
{
	llcache = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ llcache_s));
	ਜੇ (llcache == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	llcache->ਹੱਦਬੰਦੀ = prm->ਹੱਦਬੰਦੀ;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth = prm->ਵੱਧ_ਤੋਂ_ਵੱਧ_bandwidth;
	llcache->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ = prm->ਸਮਾਂ_ਸਮਾਂ_ਹਿੱਸਾ;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_ਉੱਪਰ = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->ਹੱਦਬੰਦੀ);

	/* backing store initialisation */
	ਵਾਪਸ guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
ਖਾਲੀ llcache_finalise(ਖਾਲੀ)
{
	llcache_object *object, *ਅਗਲਾ;
	uint64_t ਕੁੱਲ_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(ਨਿਰਾ);
	/* Now clear the persistence callback */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, llcache_persist, ਨਿਰਾ);

	/* Clean uncached objects */
	ਲਈ (object = llcache->uncached_objects; object != ਨਿਰਾ; object = ਅਗਲਾ) {
		llcache_object_user *user, *ਅਗਲਾ_user;

		ਅਗਲਾ = object->ਅਗਲਾ;

		ਲਈ (user = object->users; user != ਨਿਰਾ; user = ਅਗਲਾ_user) {
			ਅਗਲਾ_user = user->ਅਗਲਾ;

			ਜੇ (user->handle != ਨਿਰਾ)
				ਮੁਕਤ(user->handle);

			ਮੁਕਤ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ਨਿਰਾ;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	ਲਈ (object = llcache->cached_objects; object != ਨਿਰਾ; object = ਅਗਲਾ) {
		llcache_object_user *user, *ਅਗਲਾ_user;

		ਅਗਲਾ = object->ਅਗਲਾ;

		ਲਈ (user = object->users; user != ਨਿਰਾ; user = ਅਗਲਾ_user) {
			ਅਗਲਾ_user = user->ਅਗਲਾ;

			ਜੇ (user->handle != ਨਿਰਾ)
			       ਮੁਕਤ(user->handle);

			ਮੁਕਤ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ਨਿਰਾ;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	ਜੇ (llcache->ਕੁੱਲ_elapsed > 0) {
		ਕੁੱਲ_bandwidth = (llcache->ਕੁੱਲ_written * 1000) /
			llcache->ਕੁੱਲ_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->ਕੁੱਲ_written,
	      llcache->ਕੁੱਲ_elapsed,
	      ਕੁੱਲ_bandwidth);

	ਮੁਕਤ(llcache);
	llcache = ਨਿਰਾ;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ਅਚਲ llcache_post_ਅੰਕੜੇ *post,
			llcache_handle_callback cb, ਖਾਲੀ *pw,
			llcache_handle **ਨਤੀਜਾ)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_ਵਿੱਚ_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_ਵਿੱਚ_use);
	ਜੇ (error != NSERROR_OK) {
		ਵਾਪਸ error;
	}

	/* Can we fetch this URL at all? */
	ਜੇ (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		ਵਾਪਸ NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	ਜੇ (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ਵਾਪਸ error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_ਵਿੱਚ_use, &object);
	ਜੇ (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		ਵਾਪਸ error;
	}

	/* Add user to object */
	llcache_object_ਜੋੜੋ_user(object, user);

	*ਨਤੀਜਾ = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_ਉੱਪਰ();

	nsurl_unref(hsts_url);

	ਵਾਪਸ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ਖਾਲੀ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ਵਾਪਸ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_ਲੱਭੋ_user(handle);

	ਪ੍ਰਤਿਜ੍ਞਾ(user != ਨਿਰਾ);

	ਜੇ (user->iterator_ਨਿਸ਼ਾਨਾ) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} ਨਹੀਂਤਾਂ {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		ਜੇ (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	ਵਾਪਸ error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ਨਤੀਜਾ)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	ਜੇ (error == NSERROR_OK) {
		llcache_object_ਜੋੜੋ_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*ਨਤੀਜਾ = newuser->handle;
	}

	ਵਾਪਸ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ਲੱਭੋ_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	ਜੇ (user->prev != ਨਿਰਾ)
		all_alone = false;
	ਜੇ (user->ਅਗਲਾ != ਨਿਰਾ)
		all_alone = false;

	ਜੇ (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		ਜੇ (error != NSERROR_OK)
			ਵਾਪਸ error;

		/* Move across to the new object */
		ਜੇ (user->iterator_ਨਿਸ਼ਾਨਾ) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(llcache_object_user));
			ਜੇ (newuser == ਨਿਰਾ) {
				llcache_object_destroy(newobject);
				ਵਾਪਸ NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = ਨਿਰਾ;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_ਜੋੜੋ_user(newobject, newuser);
		} ਨਹੀਂਤਾਂ {
			llcache_object_remove_user(object, user);
			llcache_object_ਜੋੜੋ_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(newobject,
				&llcache->uncached_objects);
	} ਨਹੀਂਤਾਂ {
		/* We're the only user, so abort any fetch in progress */
		ਜੇ (object->fetch.fetch != ਨਿਰਾ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ਨਿਰਾ;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ਅੰਕੜੇ(object);
	}

	ਵਾਪਸ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_ਧਾਰਾ(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ਲੱਭੋ_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	ਜੇ (user->prev != ਨਿਰਾ || user->ਅਗਲਾ != ਨਿਰਾ)
		ਵਾਪਸ NSERROR_OK;

	/* Forcibly uncache this object */
	ਜੇ (llcache_object_ਵਿੱਚ_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_ਜੋੜੋ_ਮੰਜ਼ਿਲ_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_ਧਾਰਾ_ਅੰਕੜੇ;

	ਵਾਪਸ NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_ਅੰਕੜੇ(llcache_handle *handle)
{
	ਜੇ ((handle->object != ਨਿਰਾ) &&
	    (handle->object->fetch.fetch == ਨਿਰਾ) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	ਵਾਪਸ NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ਅਚਲ llcache_handle *handle)
{
	ਵਾਪਸ handle->object != ਨਿਰਾ ? handle->object->url : ਨਿਰਾ;
}

/* See llcache.h for documentation */
ਅਚਲ uint8_t *llcache_handle_get_ਸਰੋਤ_ਅੰਕੜੇ(ਅਚਲ llcache_handle *handle,
		size_t *ਮਾਪ)
{
	*ਮਾਪ = handle->object != ਨਿਰਾ ? handle->object->ਸਰੋਤ_len : 0;

	ਵਾਪਸ handle->object != ਨਿਰਾ ? handle->object->ਸਰੋਤ_ਅੰਕੜੇ : ਨਿਰਾ;
}

/* See llcache.h for documentation */
ਅਚਲ ਅੱਖਰ *llcache_handle_get_ਸਿਰਲੇਖ(ਅਚਲ llcache_handle *handle,
		ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ)
{
	ਅਚਲ llcache_object *object = handle->object;
	size_t i;

	ਜੇ (object == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	/* About as trivial as possible */
	ਲਈ (i = 0; i < object->ਅੰਕ_ਸਿਰਲੇਖਾਂ; i++) {
		ਜੇ (strcasecmp(ਕੁੰਜੀ, object->ਸਿਰਲੇਖਾਂ[i].name) == 0)
			ਵਾਪਸ object->ਸਿਰਲੇਖਾਂ[i].ਮੁੱਲ;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ਅਚਲ llcache_handle *a,
		ਅਚਲ llcache_handle *b)
{
	ਵਾਪਸ a->object == b->object;
}
