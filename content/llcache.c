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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>
#ಅನ್ತರ್ಭಾವ <nsutils/base64.h>

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/time.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/backing_store.h"
#ಅನ್ತರ್ಭಾವ "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು,		/**< Fetching headers */
	LLCACHE_FETCH_ದತ್ತಾಂಶ,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
ರಚನೆ llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	ಶೂನ್ಯ *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t ಬೈಟುಗಳು;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_ಗುರಿ;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	ರಚನೆ llcache_object_user *prev;	/**< Previous in list */
	ರಚನೆ llcache_object_user *ಮುಂದಿನ;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_ದತ್ತಾಂಶ *post;	/**< POST data, or NULL for GET */

	ರಚನೆ fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_ಎಣಿಕೆ;	/**< Count of redirects followed */

	uint32_t retries_ಉಳಿದ;     /**< Number of times to retry on timeout */

	bool hsts_ಒಳ_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_AGE -1

/** Cache control data */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	time_t req_ಸಮಯ;	/**< Time of request */
	time_t res_ಸಮಯ;	/**< Time of response */
	time_t fin_ಸಮಯ;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	ಪೂರ್ಣಾಂಕ age;		/**< Age: response header */
	ಪೂರ್ಣಾಂಕ max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	ಅಕ್ಷರ *etag;		/**< Etag: response header */
	time_t ಕೊನೆಯ_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ಅಕ್ಷರ *name;		/**< Header name */
	ಅಕ್ಷರ *ಮೌಲ್ಯ;		/**< Header value */
} llcache_ಶೀರ್ಷಿಕೆ;

/** Current status of an object's data */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
ರಚನೆ llcache_object {
	llcache_object *prev;	     /**< Previous in list */
	llcache_object *ಮುಂದಿನ;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *ಮೂಲ_ದತ್ತಾಂಶ;	     /**< Source data for object */
	size_t ಮೂಲ_len;	     /**< Byte length of source data */
	size_t ಮೂಲ_alloc;	     /**< Allocated size of source buffer */

	ರಚನೆ cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_ಎಣಿಕೆ;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_ಶೀರ್ಷಿಕೆ *ಶೀರ್ಷಿಕೆಗಳು;     /**< Fetch headers */
	size_t ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t ಕೊನೆಯ_ಬಳಸಿದ; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
ರಚನೆ llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t ಎಲ್ಲೆ;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_ಮೇಲೆ;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	ಪೂರ್ಣಾಂಕ minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಸಮಯ_ಕಾಲಾಂಶ;

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
	size_t ಗರಿಷ್ಠ_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t ಒಟ್ಟು_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t ಒಟ್ಟು_elapsed;

};

/** low level cache state */
ಸ್ಥಿರ ರಚನೆ llcache_s *llcache = ಖಾಲಿ;

/* forward referenced callback function */
ಸ್ಥಿರ ಶೂನ್ಯ llcache_fetch_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ಶೂನ್ಯ *p);

/* forward referenced catch up function */
ಸ್ಥಿರ ಶೂನ್ಯ llcache_users_not_caught_ಮೇಲೆ(ಶೂನ್ಯ);


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
ಸ್ಥಿರ nserror llcache_object_user_new(llcache_handle_callback cb, ಶೂನ್ಯ *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(llcache_handle));
	ಆದರೆ (h == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	u = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(llcache_object_user));
	ಆದರೆ (u == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(h);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (ಶೂನ್ಯ *) cb, pw);

	*user = u;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
ಸ್ಥಿರ nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	ಪ್ರತಿಜ್ಞಾ(user->ಮುಂದಿನ == ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(user->prev == ಖಾಲಿ);

	ಆದರೆ (user->handle != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(user->handle);

	ಬಿಡುಗಡೆ(user);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
ಸ್ಥಿರ nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	ಪ್ರತಿಜ್ಞಾ(user != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(object != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(object->users != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(user->handle == ಖಾಲಿ || user->handle->object == object);
	ಪ್ರತಿಜ್ಞಾ((user->prev != ಖಾಲಿ) || (object->users == user));

	ಆದರೆ (user == object->users)
		object->users = user->ಮುಂದಿನ;
	ಇಲ್ಲದಿದ್ದರೆ
		user->prev->ಮುಂದಿನ = user->ಮುಂದಿನ;

	ಆದರೆ (user->ಮುಂದಿನ != ಖಾಲಿ)
		user->ಮುಂದಿನ->prev = user->prev;

	user->ಮುಂದಿನ = user->prev = ಖಾಲಿ;

	/* record the time the last user was removed from the object */
	ಆದರೆ (object->users == ಖಾಲಿ) {
		object->ಕೊನೆಯ_ಬಳಸಿದ = time(ಖಾಲಿ);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ಸ್ಥಿರ nserror llcache_send_event_ಗಮ್ಯ_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *ಮುಂದಿನ_user;

	user = object->users;
	ಆಗ (user != ಖಾಲಿ) {
		bool was_ಗುರಿ = user->iterator_ಗುರಿ;
		user->iterator_ಗುರಿ = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		ಮುಂದಿನ_user = user->ಮುಂದಿನ;

		user->iterator_ಗುರಿ = was_ಗುರಿ;

		ಆದರೆ (user->queued_for_delete && !was_ಗುರಿ) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		ಆದರೆ (error != NSERROR_OK)
			ನಿಲ್ಲಿಸು;

		user = ಮುಂದಿನ_user;
	}

	ಹಿಂತಿರುಗಿ error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_object_new(nsurl *url, llcache_object **ಫಲಿತಾಂಶ)
{
	llcache_object *obj = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(llcache_object));
	ಆದರೆ (obj == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*ಫಲಿತಾಂಶ = obj;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_post_ದತ್ತಾಂಶ_clone(ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *orig,
		llcache_post_ದತ್ತಾಂಶ **clone)
{
	llcache_post_ದತ್ತಾಂಶ *post_clone;

	post_clone = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(llcache_post_ದತ್ತಾಂಶ));
	ಆದರೆ (post_clone == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	ಆದರೆ (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->ದತ್ತಾಂಶ.urlenc = strdup(orig->ದತ್ತಾಂಶ.urlenc);
		ಆದರೆ (post_clone->ದತ್ತಾಂಶ.urlenc == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(post_clone);

			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		post_clone->ದತ್ತಾಂಶ.multipart = fetch_multipart_ದತ್ತಾಂಶ_clone(
				orig->ದತ್ತಾಂಶ.multipart);
		ಆದರೆ (post_clone->ದತ್ತಾಂಶ.multipart == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(post_clone);

			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror llcache_fetch_ವಿಭಜಿಸು_ಶೀರ್ಷಿಕೆ(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		ಅಕ್ಷರ **name, ಅಕ್ಷರ **ಮೌಲ್ಯ)
{
	ಅಕ್ಷರ *n, *v;
	ಸ್ಥಿರಾಂಕ uint8_t *colon;

	/* Strip leading whitespace from name */
	ಆಗ (ದತ್ತಾಂಶ[0] == ' ' || ದತ್ತಾಂಶ[0] == '\t' ||
	       ದತ್ತಾಂಶ[0] == '\r' || ದತ್ತಾಂಶ[0] == '\n') {
		ದತ್ತಾಂಶ++;
	}

	/* Find colon */
	colon = (ಸ್ಥಿರಾಂಕ uint8_t *) strchr((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) ದತ್ತಾಂಶ, ':');
	ಆದರೆ (colon == ಖಾಲಿ) {
		/* Failed, assume a key with no value */
		colon = ದತ್ತಾಂಶ + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ದತ್ತಾಂಶ);

		/* Strip trailing whitespace from name */
		ಆಗ ((colon > ದತ್ತಾಂಶ) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) ದತ್ತಾಂಶ, colon - ದತ್ತಾಂಶ);
		ಆದರೆ (n == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		v = strdup("");
		ಆದರೆ (v == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(n);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		ಆಗ (colon > ದತ್ತಾಂಶ && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) ದತ್ತಾಂಶ, colon - ದತ್ತಾಂಶ);
		ಆದರೆ (n == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		/* Find colon again */
		ಆಗ (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		ಮಾಡು {
			colon++;
		} ಆಗ (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		ಆಗ (len > 0 && (ದತ್ತಾಂಶ[len - 1] == ' ' ||
				ದತ್ತಾಂಶ[len - 1] == '\t' ||
				ದತ್ತಾಂಶ[len - 1] == '\r' ||
				ದತ್ತಾಂಶ[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) colon, len - (colon - ದತ್ತಾಂಶ));
		ಆದರೆ (v == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(n);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	*name = n;
	*ಮೌಲ್ಯ = v;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror
llcache_fetch_parse_cache_control(llcache_object *object, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(ಮೌಲ್ಯ, &cc);
	ಆದರೆ (error != NSERROR_OK) {
		/* Ignore parse errors */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	ಆದರೆ (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror
llcache_fetch_ಶೀರ್ಷಿಕೆ_cache_control(llcache_object *object,
				   ಅಕ್ಷರ *name,
				   ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);

	ಆಯ್ಕೆ (name_len) {
	ಸ್ಥಿತಿ 3:
		ಆದರೆ (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			ಆದರೆ ('0' <= *ಮೌಲ್ಯ && *ಮೌಲ್ಯ <= '9') {
				object->cache.age = atoi(ಮೌಲ್ಯ);
			}

		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ 4:
		ಆದರೆ (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(ಮೌಲ್ಯ,
					    ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ),
					    &object->cache.date);
			ಆದರೆ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      ಮೌಲ್ಯ, res);
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			ಬಿಡುಗಡೆ(object->cache.etag);
			object->cache.etag = strdup(ಮೌಲ್ಯ);
			ಆದರೆ (object->cache.etag == ಖಾಲಿ) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ 7:
		ಆದರೆ (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(ಮೌಲ್ಯ,
					    ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ),
					    &object->cache.expires);
			ಆದರೆ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      ಮೌಲ್ಯ, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ 13:
		ಆದರೆ (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,ಮೌಲ್ಯ);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(ಮೌಲ್ಯ,
				      ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ),
				      &object->cache.ಕೊನೆಯ_modified);
		}
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ llcache_destroy_ಶೀರ್ಷಿಕೆಗಳು(llcache_object *object)
{
	ಆಗ (object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು > 0) {
		object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು--;

		ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು[object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು].name);
		ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು[object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು].ಮೌಲ್ಯ);
	}
	ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು);
	object->ಶೀರ್ಷಿಕೆಗಳು = ಖಾಲಿ;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ llcache_invalidate_cache_control_ದತ್ತಾಂಶ(llcache_object *object)
{
	ಬಿಡುಗಡೆ(object->cache.etag);
	memset(&(object->cache), 0, ಗಾತ್ರ(llcache_cache_control));

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
ಸ್ಥಿರ nserror llcache_fetch_process_ಶೀರ್ಷಿಕೆ(llcache_object *object,
		ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len)
{
	nserror res;
	ಅಕ್ಷರ *name, *ಮೌಲ್ಯ;
	llcache_ಶೀರ್ಷಿಕೆ *ತಾತ್ಕಾಲಿಕ;

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
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *) ದತ್ತಾಂಶ, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_ಸಮಯ = object->cache.req_ಸಮಯ;

		llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_ಸಮಯ = req_ಸಮಯ;

		llcache_destroy_ಶೀರ್ಷಿಕೆಗಳು(object);
	}

	/* Set fetch response time if not already set */
	ಆದರೆ (object->cache.res_ಸಮಯ == 0) {
		object->cache.res_ಸಮಯ = time(ಖಾಲಿ);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_ವಿಭಜಿಸು_ಶೀರ್ಷಿಕೆ(ದತ್ತಾಂಶ, len, &name, &ಮೌಲ್ಯ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* deal with empty header */
	ಆದರೆ (name[0] == 0) {
		ಬಿಡುಗಡೆ(name);
		ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_ಶೀರ್ಷಿಕೆ_cache_control(object, name, ಮೌಲ್ಯ);
	ಆದರೆ (res != NSERROR_OK) {
		ಬಿಡುಗಡೆ(name);
		ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
		ಹಿಂತಿರುಗಿ res;
	}

	/* Append header data to the object's headers array */
	ತಾತ್ಕಾಲಿಕ = realloc(object->ಶೀರ್ಷಿಕೆಗಳು,
		       (object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು + 1) * ಗಾತ್ರ(llcache_ಶೀರ್ಷಿಕೆ));
	ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(name);
		ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	object->ಶೀರ್ಷಿಕೆಗಳು = ತಾತ್ಕಾಲಿಕ;

	object->ಶೀರ್ಷಿಕೆಗಳು[object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು].name = name;
	object->ಶೀರ್ಷಿಕೆಗಳು[object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು].ಮೌಲ್ಯ = ಮೌಲ್ಯ;

	object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು++;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror get_referer_ಶೀರ್ಷಿಕೆ(nsurl *url, nsurl *referer, ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆ_ಹೊರಹೋಗು)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool ಹೊಂದಿಕೆ;
	bool match1;
	bool match2;
	ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆ;

	/* Determine whether to send the Referer header */
	ಆದರೆ (!nsoption_bool(send_referer)) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಆದರೆ (scheme == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	ಆದರೆ (ref_scheme == ಖಾಲಿ) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
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
	ಆದರೆ (lwc_string_isequal(scheme, ref_scheme,
			       &ಹೊಂದಿಕೆ) != lwc_error_ok) {
		ಹೊಂದಿಕೆ = false;
	}
	ಆದರೆ (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	ಆದರೆ (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	ಆದರೆ (ಹೊಂದಿಕೆ == true || (match1 == true && match2 == true)) {
		ಸ್ಥಿರಾಂಕ size_t len = SLEN("Referer: ") +
			nsurl_ಉದ್ದಳತೆ(referer) + 1;

		ಶೀರ್ಷಿಕೆ = ಸ್ಮರಣೆ_ಕೊಡು(len);
		ಆದರೆ (ಶೀರ್ಷಿಕೆ == ಖಾಲಿ) {
			res = NSERROR_NOMEM;
		} ಇಲ್ಲದಿದ್ದರೆ {
			snprintf(ಶೀರ್ಷಿಕೆ, len, "Referer: %s",
				 nsurl_access(referer));

			*ಶೀರ್ಷಿಕೆ_ಹೊರಹೋಗು = ಶೀರ್ಷಿಕೆ;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ nserror llcache_object_refetch(llcache_object *object)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *urlenc = ಖಾಲಿ;
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *multipart = ಖಾಲಿ;
	ಅಕ್ಷರ **ಶೀರ್ಷಿಕೆಗಳು = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_idx = 0;
	nserror res;

	ಆದರೆ (object->fetch.post != ಖಾಲಿ) {
		ಆದರೆ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->ದತ್ತಾಂಶ.urlenc;
		} ಇಲ್ಲದಿದ್ದರೆ {
			multipart = object->fetch.post->ದತ್ತಾಂಶ.multipart;
		}
	}

	/* Generate headers */
	ಶೀರ್ಷಿಕೆಗಳು = ಸ್ಮರಣೆ_ಕೊಡು(4 * ಗಾತ್ರ(ಅಕ್ಷರ *));
	ಆದರೆ (ಶೀರ್ಷಿಕೆಗಳು == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	ಆದರೆ (object->cache.etag != ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ size_t len = SLEN("If-None-Match: ") +
				ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(object->cache.etag) + 1;

		ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx] = ಸ್ಮರಣೆ_ಕೊಡು(len);
		ಆದರೆ (ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx] == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಶೀರ್ಷಿಕೆಗಳು);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		snprintf(ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx], len, "If-None-Match: %s",
				object->cache.etag);

		ಶೀರ್ಷಿಕೆ_idx++;
	}

	/* cache-control header for modification time */
	ಆದರೆ (object->cache.ಕೊನೆಯ_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ಸ್ಥಿರಾಂಕ size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx] = ಸ್ಮರಣೆ_ಕೊಡು(len);
		ಆದರೆ (ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx] == ಖಾಲಿ) {
			ಆಗ (--ಶೀರ್ಷಿಕೆ_idx >= 0)
				ಬಿಡುಗಡೆ(ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx]);
			ಬಿಡುಗಡೆ(ಶೀರ್ಷಿಕೆಗಳು);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		snprintf(ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.ಕೊನೆಯ_modified));

		ಶೀರ್ಷಿಕೆ_idx++;
	}

	/* Referer header */
	ಆದರೆ (object->fetch.referer != ಖಾಲಿ) {
		ಆದರೆ (get_referer_ಶೀರ್ಷಿಕೆ(object->url,
				       object->fetch.referer,
				       &ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx]) == NSERROR_OK) {
			ಶೀರ್ಷಿಕೆ_idx++;
		}
	}
	ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx] = ಖಾಲಿ;

	/* Reset cache control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);
	object->cache.req_ಸಮಯ = time(ಖಾಲಿ);
	object->cache.fin_ಸಮಯ = object->cache.req_ಸಮಯ;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_ಆರಂಭ(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_ಪುಟಗಳು,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **)ಶೀರ್ಷಿಕೆಗಳು,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	ಆಗ (--ಶೀರ್ಷಿಕೆ_idx >= 0) {
		ಬಿಡುಗಡೆ(ಶೀರ್ಷಿಕೆಗಳು[ಶೀರ್ಷಿಕೆ_idx]);
	}
	ಬಿಡುಗಡೆ(ಶೀರ್ಷಿಕೆಗಳು);

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
		uint32_t redirect_ಎಣಿಕೆ, bool hsts_ಒಳ_use)
{
	nserror error;
	nsurl *referer_clone = ಖಾಲಿ;
	llcache_post_ದತ್ತಾಂಶ *post_clone = ಖಾಲಿ;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	ಆದರೆ (post != ಖಾಲಿ) {
		error = llcache_post_ದತ್ತಾಂಶ_clone(post, &post_clone);
		ಆದರೆ (error != NSERROR_OK)
			ಹಿಂತಿರುಗಿ error;
	}

	ಆದರೆ (referer != ಖಾಲಿ)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ಎಣಿಕೆ = redirect_ಎಣಿಕೆ;
	object->fetch.retries_ಉಳಿದ = llcache->fetch_attempts;
	object->fetch.hsts_ಒಳ_use = hsts_ಒಳ_use;

	ಹಿಂತಿರುಗಿ llcache_object_refetch(object);
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
ಸ್ಥಿರ nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	ಆದರೆ (object->ಮೂಲ_ದತ್ತಾಂಶ != ಖಾಲಿ) {
		ಆದರೆ (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬಿಡುಗಡೆ(object->ಮೂಲ_ದತ್ತಾಂಶ);
		}
	}

	nsurl_unref(object->url);

	ಆದರೆ (object->fetch.fetch != ಖಾಲಿ) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = ಖಾಲಿ;
	}

	ಆದರೆ (object->fetch.referer != ಖಾಲಿ)
		nsurl_unref(object->fetch.referer);

	ಆದರೆ (object->fetch.post != ಖಾಲಿ) {
		ಆದರೆ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			ಬಿಡುಗಡೆ(object->fetch.post->ದತ್ತಾಂಶ.urlenc);
		} ಇಲ್ಲದಿದ್ದರೆ {
			fetch_multipart_ದತ್ತಾಂಶ_destroy(
					object->fetch.post->ದತ್ತಾಂಶ.multipart);
		}

		ಬಿಡುಗಡೆ(object->fetch.post);
	}

	ಬಿಡುಗಡೆ(object->cache.etag);

	ಗಾಗಿ (i = 0; i < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು; i++) {
		ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು[i].name);
		ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು[i].ಮೌಲ್ಯ);
	}
	ಬಿಡುಗಡೆ(object->ಶೀರ್ಷಿಕೆಗಳು);

	ಬಿಡುಗಡೆ(object);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
ಸ್ಥಿರ nserror llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(llcache_object *object,
		llcache_object **list)
{
	object->prev = ಖಾಲಿ;
	object->ಮುಂದಿನ = *list;

	ಆದರೆ (*list != ಖಾಲಿ)
		(*list)->prev = object;
	*list = object;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
llcache_object_rfc2616_ಉಳಿದ_lifetime(ಸ್ಥಿರಾಂಕ llcache_cache_control *cd)
{
	ಪೂರ್ಣಾಂಕ current_age, freshness_lifetime;
	time_t now = time(ಖಾಲಿ);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_ಸಮಯ - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_ಸಮಯ - cd->req_ಸಮಯ + now - cd->res_ಸಮಯ;

	/* Determine freshness lifetime of this object */
	ಆದರೆ (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cd->ಕೊನೆಯ_modified != 0) {
		freshness_lifetime = (now - cd->ಕೊನೆಯ_modified) / 10;
	} ಇಲ್ಲದಿದ್ದರೆ {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	ಆದರೆ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		ಹಿಂತಿರುಗಿ freshness_lifetime - current_age;
	}

	ಹಿಂತಿರುಗಿ 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
ಸ್ಥಿರ bool llcache_object_is_fresh(ಸ್ಥಿರಾಂಕ llcache_object *object)
{
	ಪೂರ್ಣಾಂಕ ಉಳಿದ_lifetime;
	ಸ್ಥಿರಾಂಕ llcache_cache_control *cd = &object->cache;

	ಉಳಿದ_lifetime = llcache_object_rfc2616_ಉಳಿದ_lifetime(cd);

	NSLOG(llcache, DEBUG, "%p: (%d > 0 || %d != %d)", object,
	     ಉಳಿದ_lifetime,
	     object->fetch.state, LLCACHE_FETCH_COMPLETE);

	/* The object is fresh if:
	 * - it was not forbidden from being returned from the cache
	 *   unvalidated.
	 *
	 * - it has remaining lifetime or still being fetched.
	 */
	ಹಿಂತಿರುಗಿ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
		((ಉಳಿದ_lifetime > 0) ||
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
ಸ್ಥಿರ nserror llcache_object_clone_cache_ದತ್ತಾಂಶ(llcache_object *ಮೂಲ,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	ಆದರೆ (ಮೂಲ->cache.etag != ಖಾಲಿ) {
		ಅಕ್ಷರ *etag = ಮೂಲ->cache.etag;

		ಆದರೆ (deep) {
			/* Copy the etag */
			etag = strdup(ಮೂಲ->cache.etag);
			ಆದರೆ (etag == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Destination takes ownership */
			ಮೂಲ->cache.etag = ಖಾಲಿ;
		}

		ಆದರೆ (destination->cache.etag != ಖಾಲಿ)
			ಬಿಡುಗಡೆ(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_ಸಮಯ = ಮೂಲ->cache.req_ಸಮಯ;
	destination->cache.res_ಸಮಯ = ಮೂಲ->cache.res_ಸಮಯ;
	destination->cache.fin_ಸಮಯ = ಮೂಲ->cache.fin_ಸಮಯ;

	ಆದರೆ (ಮೂಲ->cache.date != 0)
		destination->cache.date = ಮೂಲ->cache.date;

	ಆದರೆ (ಮೂಲ->cache.expires != 0)
		destination->cache.expires = ಮೂಲ->cache.expires;

	ಆದರೆ (ಮೂಲ->cache.age != INVALID_AGE)
		destination->cache.age = ಮೂಲ->cache.age;

	ಆದರೆ (ಮೂಲ->cache.max_age != INVALID_AGE)
		destination->cache.max_age = ಮೂಲ->cache.max_age;

	ಆದರೆ (ಮೂಲ->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = ಮೂಲ->cache.no_cache;

	ಆದರೆ (ಮೂಲ->cache.ಕೊನೆಯ_modified != 0)
		destination->cache.ಕೊನೆಯ_modified = ಮೂಲ->cache.ಕೊನೆಯ_modified;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
ಸ್ಥಿರ nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	ಆದರೆ (object == *list)
		*list = object->ಮುಂದಿನ;
	ಇಲ್ಲದಿದ್ದರೆ
		object->prev->ಮುಂದಿನ = object->ಮುಂದಿನ;

	ಆದರೆ (object->ಮುಂದಿನ != ಖಾಲಿ)
		object->ಮುಂದಿನ->prev = object->prev;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror llcache_retrieve_persisted_ದತ್ತಾಂಶ(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	ಆದರೆ ((object->ಮೂಲ_ದತ್ತಾಂಶ != ಖಾಲಿ) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	ಹಿಂತಿರುಗಿ guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->ಮೂಲ_ದತ್ತಾಂಶ,
				    &object->ಮೂಲ_len);
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
ಸ್ಥಿರ nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **ದತ್ತಾಂಶ_ಹೊರಹೋಗು,
			   size_t *datasize_ಹೊರಹೋಗು)
{
	size_t allocsize;
	ಪೂರ್ಣಾಂಕ datasize;
	uint8_t *ದತ್ತಾಂಶ;
	ಅಕ್ಷರ *op;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ hloop;
	ಪೂರ್ಣಾಂಕ use;
	size_t cert_chain_ಆಳ;

	ಆದರೆ (object->chain != ಖಾಲಿ) {
		cert_chain_ಆಳ = object->chain->ಆಳ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		cert_chain_ಆಳ = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	ಗಾಗಿ (hloop = 0 ; hloop < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು ; hloop++) {
		allocsize += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(object->ಶೀರ್ಷಿಕೆಗಳು[hloop].name) + 1;
		allocsize += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(object->ಶೀರ್ಷಿಕೆಗಳು[hloop].ಮೌಲ್ಯ) + 1;
	}

	allocsize += nsurl_ಉದ್ದಳತೆ(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	ಗಾಗಿ (hloop = 0; hloop < cert_chain_ಆಳ; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_ಉದ್ದಳತೆ + 2) / 3);
	}

	ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(allocsize);
	ಆದರೆ (ದತ್ತಾಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	op = (ಅಕ್ಷರ *)ದತ್ತಾಂಶ;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	ಆದರೆ (use < 0) {
		ಹೋಗು operror;
	}
	use++; /* does not count the null */
	ಆದರೆ (use > datasize) {
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->ಮೂಲ_len);
	ಆದರೆ (use < 0) {
		ಹೋಗು operror;
	}
	use++; /* does not count the null */
	ಆದರೆ (use > datasize)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_ಸಮಯ);
	ಆದರೆ (use == 0)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_ಸಮಯ);
	ಆದರೆ (use == 0)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_ಸಮಯ);
	ಆದರೆ (use == 0)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು);
	ಆದರೆ (use < 0) {
		ಹೋಗು operror;
	}
	use++; /* does not count the null */
	ಆದರೆ (use > datasize)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	op += use;
	datasize -= use;

	/*ಶೀರ್ಷಿಕೆಗಳು*/
	ಗಾಗಿ (hloop = 0 ; hloop < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->ಶೀರ್ಷಿಕೆಗಳು[hloop].name,
			       object->ಶೀರ್ಷಿಕೆಗಳು[hloop].ಮೌಲ್ಯ);
		ಆದರೆ (use < 0) {
			ಹೋಗು operror;
		}
		use++; /* does not count the null */
		ಆದರೆ (use > datasize)
			ಹೋಗು ಅತಿಪ್ರವಾಹ;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_ಆಳ);
	ಆದರೆ (use < 0) {
		ಹೋಗು operror;
	}
	use++; /* does not count the null */
	ಆದರೆ (use > datasize)
		ಹೋಗು ಅತಿಪ್ರವಾಹ;
	op += use;
	datasize -= use;

	/* SSL certificates */
	ಗಾಗಿ (hloop = 0; hloop < cert_chain_ಆಳ; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (ಪೂರ್ಣಾಂಕ)(object->chain->certs[hloop].err));
		ಆದರೆ (use < 0) {
			ಹೋಗು operror;
		}
		use++; /* does not count the null */
		ಆದರೆ (use > datasize)
			ಹೋಗು ಅತಿಪ್ರವಾಹ;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		ಆದರೆ (object->chain->certs[hloop].der != ಖಾಲಿ) {
			size_t output_ಉದ್ದಳತೆ = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_ಉದ್ದಳತೆ,
				(uint8_t *)op,
				&output_ಉದ್ದಳತೆ);
			ಆದರೆ (res != NSUERROR_OK) {
				ಹೋಗು operror;
			}
			use = output_ಉದ್ದಳತೆ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			use = 0;
		}
		use++; /* allow for null */
		ಆದರೆ (use > datasize)
			ಹೋಗು ಅತಿಪ್ರವಾಹ;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*ದತ್ತಾಂಶ_ಹೊರಹೋಗು = ದತ್ತಾಂಶ;
	*datasize_ಹೊರಹೋಗು = allocsize - datasize;

	ಹಿಂತಿರುಗಿ NSERROR_OK;

ಅತಿಪ್ರವಾಹ:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
	ಹಿಂತಿರುಗಿ NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
	ಹಿಂತಿರುಗಿ NSERROR_INVALID;
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
ಸ್ಥಿರ nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = ಖಾಲಿ;
	size_t metadatalen = 0;
	size_t ಉಳಿದ = 0;
	nsurl *metadataurl;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸಾಲು;
	ಅಕ್ಷರ *ln;
	ಪೂರ್ಣಾಂಕ lnsize;

	size_t ಮೂಲ_ಉದ್ದಳತೆ;
	time_t request_ಸಮಯ;
	time_t response_ಸಮಯ;
	time_t completion_ಸಮಯ;
	size_t ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು;
	size_t hloop;
	size_t ssl_cert_ಎಣಿಕೆ = 0;
	ರಚನೆ cert_chain *chain = ಖಾಲಿ;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	NSLOG(llcache, INFO, "Processing retrieved data");

	/* metadata is stored as a sequence of NULL terminated strings
	 * which we call 'line's here.
	 */

	/* We track remaining data because as we extend this data structure
	 * we need to know if we should continue to parse
	 */
	ಉಳಿದ = metadatalen;

	/* metadata line 1 is the url the metadata referrs to */
	ಸಾಲು = 1;
	ln = (ಅಕ್ಷರ *)metadata;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	ಆದರೆ (lnsize < 7) {
		res = NSERROR_INVALID;
		ಹೋಗು format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	ಆದರೆ (res != NSERROR_OK)
		ಹೋಗು format_error;

	ಆದರೆ (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		ಹಿಂತಿರುಗಿ NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	ಸಾಲು = 2;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	ಆದರೆ ((lnsize < 1) || (ಅಕ್ಷರಸರಣಿ_ಓದು(ln, "%" PRIsizet, &ಮೂಲ_ಉದ್ದಳತೆ) != 1)) {
		res = NSERROR_INVALID;
		ಹೋಗು format_error;
	}


	/* metadata line 3 is the time of request */
	ಸಾಲು = 3;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_ಸಮಯ);
	ಆದರೆ (res != NSERROR_OK)
		ಹೋಗು format_error;


	/* metadata line 4 is the time of response */
	ಸಾಲು = 4;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_ಸಮಯ);
	ಆದರೆ (res != NSERROR_OK)
		ಹೋಗು format_error;


	/* metadata line 5 is the time of request completion */
	ಸಾಲು = 5;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_ಸಮಯ);
	ಆದರೆ (res != NSERROR_OK)
		ಹೋಗು format_error;


	/* metadata line 6 is the number of headers */
	ಸಾಲು = 6;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	ಆದರೆ ((lnsize < 1) || (ಅಕ್ಷರಸರಣಿ_ಓದು(ln, "%" PRIsizet, &ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು) != 1)) {
		res = NSERROR_INVALID;
		ಹೋಗು format_error;
	}

	/* read headers */
	ಗಾಗಿ (hloop = 0 ; hloop < ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು; hloop++) {
		ಸಾಲು++;
		ln += lnsize + 1;
		lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
		ಉಳಿದ -= lnsize + 1;

		res = llcache_fetch_process_ಶೀರ್ಷಿಕೆ(object,
						   (uint8_t *)ln,
						   lnsize);
		ಆದರೆ (res != NSERROR_OK)
			ಹೋಗು format_error;
	}

	ಆದರೆ (ಉಳಿದ == 0) {
		ಹೋಗು skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	ಸಾಲು++;
	ln += lnsize + 1;
	lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
	ಉಳಿದ -= lnsize + 1;

	ಆದರೆ ((lnsize < 1) || (ಅಕ್ಷರಸರಣಿ_ಓದು(ln, "%" PRIsizet, &ssl_cert_ಎಣಿಕೆ) != 1)) {
		res = NSERROR_INVALID;
		ಹೋಗು format_error;
	}

	ಆದರೆ (ssl_cert_ಎಣಿಕೆ == 0) {
		ಹೋಗು skip_ssl_certificates;
	}

	ಆದರೆ (ssl_cert_ಎಣಿಕೆ > MAX_CERT_ಆಳ) {
		res = NSERROR_INVALID;
		ಹೋಗು format_error;
	}

	res = cert_chain_alloc(ssl_cert_ಎಣಿಕೆ, &chain);
	ಆದರೆ (res != NSERROR_OK) {
		ಹೋಗು format_error;
	}

	ಗಾಗಿ (hloop = 0; hloop < ssl_cert_ಎಣಿಕೆ; hloop++) {
		ಪೂರ್ಣಾಂಕ errcode;
		nsuerror nsures;

		/* Certificate error code */
		ಸಾಲು++;
		ln += lnsize + 1;
		lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
		ಉಳಿದ -= lnsize + 1;
		ಆದರೆ ((lnsize < 1) || (ಅಕ್ಷರಸರಣಿ_ಓದು(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			ಹೋಗು format_error;
		}
		ಆದರೆ (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_ತಿಳಿದ) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} ಇಲ್ಲದಿದ್ದರೆ {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		ಸಾಲು++;
		ln += lnsize + 1;
		lnsize = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ln);
		ಉಳಿದ -= lnsize + 1;
		ಆದರೆ (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ಸ್ಥಿರಾಂಕ uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_ಉದ್ದಳತೆ);
			ಆದರೆ (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				ಹೋಗು format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->ಮೂಲ_len = ಮೂಲ_ಉದ್ದಳತೆ;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->ಮೂಲ_alloc = metadatalen;

	object->cache.req_ಸಮಯ = request_ಸಮಯ;
	object->cache.res_ಸಮಯ = response_ಸಮಯ;
	object->cache.fin_ಸಮಯ = completion_ಸಮಯ;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	ಹಿಂತಿರುಗಿ NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      ಸಾಲು, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	ಹಿಂತಿರುಗಿ res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool llcache__scheme_is_persistable(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool ಹೊಂದಿಕೆ;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ಆದರೆ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	ಹಿಂತಿರುಗಿ persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool llcache__scheme_is_cachable(ಸ್ಥಿರಾಂಕ nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool ಹೊಂದಿಕೆ;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ಆದರೆ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_ದತ್ತಾಂಶ,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&ಹೊಂದಿಕೆ) == lwc_error_ok &&
			(ಹೊಂದಿಕೆ == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	ಹಿಂತಿರುಗಿ cachable;
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
ಸ್ಥಿರ nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
				uint32_t redirect_ಎಣಿಕೆ)
{
	nserror error;
	nsurl *referer_clone = ಖಾಲಿ;
	llcache_post_ದತ್ತಾಂಶ *post_clone = ಖಾಲಿ;

	ಆದರೆ (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	object->cache.req_ಸಮಯ = time(ಖಾಲಿ);
	object->cache.fin_ಸಮಯ = object->cache.req_ಸಮಯ;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	ಆದರೆ (error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	/* entry came out of cache - need to setup object state */
	ಆದರೆ (post != ಖಾಲಿ) {
		error = llcache_post_ದತ್ತಾಂಶ_clone(post, &post_clone);
		ಆದರೆ (error != NSERROR_OK)
			ಹಿಂತಿರುಗಿ error;
	}

	ಆದರೆ (referer != ಖಾಲಿ) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_ಎಣಿಕೆ = redirect_ಎಣಿಕೆ;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
				   uint32_t redirect_ಎಣಿಕೆ,
				   bool hsts_ಒಳ_use,
				   llcache_object **ಫಲಿತಾಂಶ)
{
	nserror error;
	llcache_object *obj, *newest = ಖಾಲಿ;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==ಖಾಲಿ?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	ಗಾಗಿ (obj = llcache->cached_objects; obj != ಖಾಲಿ; obj = obj->ಮುಂದಿನ) {

		ಆದರೆ ((newest == ಖಾಲಿ ||
		     obj->cache.req_ಸಮಯ > newest->cache.req_ಸಮಯ) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	ಆದರೆ (newest == ಖಾಲಿ) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		ಆದರೆ (error != NSERROR_OK)
			ಹಿಂತಿರುಗಿ error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_ಎಣಿಕೆ);
		ಆದರೆ (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	ಆದರೆ ((newest != ಖಾಲಿ) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ದತ್ತಾಂಶ(newest);
		ಆದರೆ (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*ಫಲಿತಾಂಶ = newest;

			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		ಆದರೆ (error != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ error;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (newest != ಖಾಲಿ) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_ದತ್ತಾಂಶ(newest);
		ಆದರೆ (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			ಆದರೆ (error != NSERROR_OK)
				ಹಿಂತಿರುಗಿ error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_ದತ್ತಾಂಶ(newest, obj, true);
			ಆದರೆ (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				ಹಿಂತಿರುಗಿ error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_ಎಣಿಕೆ++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_ಎಣಿಕೆ, hsts_ಒಳ_use);
			ಆದರೆ (error != NSERROR_OK) {
				newest->candidate_ಎಣಿಕೆ--;
				llcache_object_destroy(obj);
				ಹಿಂತಿರುಗಿ error;
			}

			/* Add new object to cache */
			llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(obj, &llcache->cached_objects);

			*ಫಲಿತಾಂಶ = obj;

			ಹಿಂತಿರುಗಿ NSERROR_OK;
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
		ಆದರೆ (error != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_ಎಣಿಕೆ, hsts_ಒಳ_use);
	ಆದರೆ (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Add new object to cache */
	llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(obj, &llcache->cached_objects);

	*ಫಲಿತಾಂಶ = obj;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
			uint32_t redirect_ಎಣಿಕೆ,
			bool hsts_ಒಳ_use,
			llcache_object **ಫಲಿತಾಂಶ)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==ಖಾಲಿ?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	/* determine if content is cachable */
	ಆದರೆ ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (post != ಖಾಲಿ) {
		/* POST requests are never cached */
		uncachable = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	ಆದರೆ (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		ಆದರೆ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ಹಿಂತಿರುಗಿ error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_ಎಣಿಕೆ, hsts_ಒಳ_use);
		ಆದರೆ (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			ಹಿಂತಿರುಗಿ error;
		}

		/* Add new object to uncached list */
		llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(obj, &llcache->uncached_objects);
	} ಇಲ್ಲದಿದ್ದರೆ {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_ಎಣಿಕೆ,
				hsts_ಒಳ_use, &obj);
		ಆದರೆ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			ಹಿಂತಿರುಗಿ error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*ಫಲಿತಾಂಶ = obj;

	nsurl_unref(defragmented_url);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
ಸ್ಥಿರ nserror llcache_object_ಕೂಡಿಸು_user(llcache_object *object,
		llcache_object_user *user)
{
	ಪ್ರತಿಜ್ಞಾ(user->ಮುಂದಿನ == ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(user->prev == ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(user->handle != ಖಾಲಿ);

	user->handle->object = object;

	user->prev = ಖಾಲಿ;
	user->ಮುಂದಿನ = object->users;

	ಆದರೆ (object->users != ಖಾಲಿ)
		object->users->prev = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_hsts_transform_url(nsurl *url, nsurl **ಫಲಿತಾಂಶ,
		bool *hsts_ಒಳ_use)
{
	lwc_string *scheme = ಖಾಲಿ;
	bool ಹೊಂದಿಕೆ;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ಆದರೆ (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&ಹೊಂದಿಕೆ) != lwc_error_ok || ಹೊಂದಿಕೆ == false) {
		/* Non-HTTP fetch: no transform required */
		ಆದರೆ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_ಒಳ_use = urldb_get_hsts_enabled(url);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Anything else: no HSTS */
			*hsts_ಒಳ_use = false;
		}
		lwc_string_unref(scheme);
		*ಫಲಿತಾಂಶ = nsurl_ref(url);
		ಹಿಂತಿರುಗಿ error;
	}
	lwc_string_unref(scheme);

	ಆದರೆ (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				ಫಲಿತಾಂಶ);
		*hsts_ಒಳ_use = (error == NSERROR_OK);
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಫಲಿತಾಂಶ = nsurl_ref(url);
		*hsts_ಒಳ_use = false;
	}

	ಹಿಂತಿರುಗಿ error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = ಖಾಲಿ;
	bool ಹೊಂದಿಕೆ = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	ಆದರೆ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&ಹೊಂದಿಕೆ) != lwc_error_ok || ಹೊಂದಿಕೆ == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}
	lwc_string_unref(scheme);

	ಆದರೆ (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಗಾಗಿ (i = 0; i < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು; i++) {
		ಆದರೆ (strcasecmp("Strict-Transport-Security",
				object->ಶೀರ್ಷಿಕೆಗಳು[i].name) == 0) {
			urldb_ಗಣ_hsts_policy(object->url,
					object->ಶೀರ್ಷಿಕೆಗಳು[i].ಮೌಲ್ಯ);
			/* Only process the first one we find */
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool ಹೊಂದಿಕೆ;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	ಆದರೆ ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ == false)) {
		/* file, about and resource are not valid redirect targets */
		ಆದರೆ ((lwc_string_isequal(scheme, corestring_lwc_file,
				&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			ಹಿಂತಿರುಗಿ NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	ಆದರೆ (fetch_can_fetch(dst_url) == false) {
		ಹಿಂತಿರುಗಿ NSERROR_UNSAFE_REDIRECT;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_fetch_redirect(llcache_object *object,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *ಮುಂದಿನ;
	ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_ಒಳ_use;
	http_response_ಸಂಕೇತ http_ಸಂಕೇತ;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_ಸಂಕೇತ = fetch_http_ಸಂಕೇತ(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ಖಾಲಿ;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ಶೂನ್ಯ) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	ಆದರೆ (object->fetch.redirect_ಎಣಿಕೆ > nsoption_uint(fetch_redirect_ಎಲ್ಲೆ)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		ಹಿಂತಿರುಗಿ NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, ಗುರಿ, &url);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_ಒಳ_use);
	nsurl_unref(url);
	ಆದರೆ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		ಹಿಂತಿರುಗಿ res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	ಆದರೆ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ res;
	}

	/* alter behaviour based on specific redirect code */
	ಆಯ್ಕೆ (http_ಸಂಕೇತ) {
	ಸ್ಥಿತಿ HTTP_RESPONSE_MOVED_PERMANENTLY:
	ಸ್ಥಿತಿ HTTP_RESPONSE_ಸಿಕ್ಕಿತು:
	ಸ್ಥಿತಿ HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = ಖಾಲಿ;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ HTTP_RESPONSE_TEMPORARY_REDIRECT:
	ಸ್ಥಿತಿ HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_ಸಂಕೇತ);
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.ದತ್ತಾಂಶ.redirect.from = object->url;
	event.ದತ್ತಾಂಶ.redirect.ಗಮ್ಯ = hsts_url;

	res = llcache_send_event_ಗಮ್ಯ_users(object, &event);
	ಆದರೆ (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_ಎಣಿಕೆ + 1,
					hsts_ಒಳ_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	ಆದರೆ (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		ಗಾಗಿ (user = object->users; user != ಖಾಲಿ; user = ಮುಂದಿನ) {
			ಮುಂದಿನ = user->ಮುಂದಿನ;

			llcache_object_remove_user(object, user);
			llcache_object_ಕೂಡಿಸು_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	ಹಿಂತಿರುಗಿ res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
ಸ್ಥಿರ nserror llcache_object_cache_update(llcache_object *object)
{
	ಆದರೆ (object->cache.date == 0)
		object->cache.date = time(ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
ಸ್ಥಿರ nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	ಆದರೆ (object->candidate != ಖಾಲಿ) {
		llcache_object_user *user, *ಮುಂದಿನ;

		/* Move user(s) to candidate content */
		ಗಾಗಿ (user = object->users; user != ಖಾಲಿ; user = ಮುಂದಿನ) {
			ಮುಂದಿನ = user->ಮುಂದಿನ;

			llcache_object_remove_user(object, user);
			llcache_object_ಕೂಡಿಸು_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_ಎಣಿಕೆ--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_ದತ್ತಾಂಶ(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		ಆದರೆ (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ಖಾಲಿ;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(ಶೂನ್ಯ) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ಸ್ಥಿರ nserror
llcache_fetch_process_ದತ್ತಾಂಶ(llcache_object *object,
			   ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ,
			   size_t len)
{
	ಆದರೆ (object->fetch.state != LLCACHE_FETCH_ದತ್ತಾಂಶ) {
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
		ದೀರ್ಘ http_ಸಂಕೇತ = fetch_http_ಸಂಕೇತ(object->fetch.fetch);

		ಆದರೆ ((http_ಸಂಕೇತ != 200 && http_ಸಂಕೇತ != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);
		}

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		object->fetch.state = LLCACHE_FETCH_ದತ್ತಾಂಶ;
	}

	/* Resize source buffer if it's too small */
	ಆದರೆ (object->ಮೂಲ_len + len >= object->ಮೂಲ_alloc) {
		ಸ್ಥಿರಾಂಕ size_t new_len = object->ಮೂಲ_len + len + 64 * 1024;
		uint8_t *ತಾತ್ಕಾಲಿಕ = realloc(object->ಮೂಲ_ದತ್ತಾಂಶ, new_len);
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		object->ಮೂಲ_ದತ್ತಾಂಶ = ತಾತ್ಕಾಲಿಕ;
		object->ಮೂಲ_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	ಸ್ಮರಣೆ_ನಕಲು(object->ಮೂಲ_ದತ್ತಾಂಶ + object->ಮೂಲ_len, ದತ್ತಾಂಶ, len);
	object->ಮೂಲ_len += len;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
ಸ್ಥಿರ nserror llcache_fetch_auth(llcache_object *object, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ಖಾಲಿ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	/* Destroy headers */
	llcache_destroy_ಶೀರ್ಷಿಕೆಗಳು(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	ಆದರೆ (realm == ಖಾಲಿ)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	ಆದರೆ (auth == ಖಾಲಿ || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_BAD_AUTH;
		event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = realm;

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	ಹಿಂತಿರುಗಿ error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ಖಾಲಿ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	ಆದರೆ (object->fetch.hsts_ಒಳ_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_BAD_CERTS;
		event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = messages_get("FetchFailed");

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);
	} ಇಲ್ಲದಿದ್ದರೆ {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_UNKNOWN;
		event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = messages_get("FetchFailed");

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);
	}

	ಹಿಂತಿರುಗಿ error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ಖಾಲಿ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	ಆದರೆ (object->fetch.hsts_ಒಳ_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_UNKNOWN;
		event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = messages_get("FetchFailed");

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	ಹಿಂತಿರುಗಿ error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
ಸ್ಥಿರ nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	ಆದರೆ (object->fetch.retries_ಉಳಿದ > 1) {
		object->fetch.retries_ಉಳಿದ--;
		ಹಿಂತಿರುಗಿ llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ಖಾಲಿ;

	/* Release candidate, if any */
	ಆದರೆ (object->candidate != ಖಾಲಿ) {
		object->candidate->candidate_ಎಣಿಕೆ--;
		object->candidate = ಖಾಲಿ;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_TIMEOUT;
	event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ llcache_send_event_ಗಮ್ಯ_users(object, &event);
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
ಸ್ಥಿರ nserror
build_candidate_list(ರಚನೆ llcache_object ***lst_ಹೊರಹೋಗು, ಪೂರ್ಣಾಂಕ *lst_len_ಹೊರಹೋಗು)
{
	llcache_object *object, *ಮುಂದಿನ;
	ರಚನೆ llcache_object **lst;
	ಪೂರ್ಣಾಂಕ lst_len = 0;
	ಪೂರ್ಣಾಂಕ ಉಳಿದ_lifetime;

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_PERSIST_ಪ್ರತಿ_ಸತತ 128

	lst = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(MAX_PERSIST_ಪ್ರತಿ_ಸತತ, ಗಾತ್ರ(ರಚನೆ llcache_object *));
	ಆದರೆ (lst == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಗಾಗಿ (object = llcache->cached_objects; object != ಖಾಲಿ; object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		/* Only consider http(s) for the disc cache. */
		ಆದರೆ (!llcache__scheme_is_persistable(object->url)) {
			ಮುಂದುವರಿ;
		}

		ಉಳಿದ_lifetime = llcache_object_rfc2616_ಉಳಿದ_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		ಆದರೆ ((object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (ಉಳಿದ_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			ಆದರೆ (lst_len == MAX_PERSIST_ಪ್ರತಿ_ಸತತ)
				ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (lst_len == 0) {
		ಬಿಡುಗಡೆ(lst);
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;
	}

	/** \todo sort list here */

	*lst_len_ಹೊರಹೋಗು = lst_len;
	*lst_ಹೊರಹೋಗು = lst;

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು MAX_PERSIST_ಪ್ರತಿ_ಸತತ

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
ಸ್ಥಿರ nserror
write_backing_store(ರಚನೆ llcache_object *object,
		    size_t *written_ಹೊರಹೋಗು,
		    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *elapsed)
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
				   object->ಮೂಲ_ದತ್ತಾಂಶ,
				   object->ಮೂಲ_len);
	ಆದರೆ (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	ಆದರೆ (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	ಆದರೆ (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		ಹಿಂತಿರುಗಿ ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_ಹೊರಹೋಗು = object->ಮೂಲ_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	ಆದರೆ (*elapsed == 0) {
		*elapsed = 1;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ ಶೂನ್ಯ llcache_persist_slowcheck(ಶೂನ್ಯ *p)
{
	uint64_t ಒಟ್ಟು_bandwidth; /* total bandwidth */

	ಆದರೆ (llcache->ಒಟ್ಟು_written > (2 * llcache->minimum_bandwidth)) {

		ಒಟ್ಟು_bandwidth = (llcache->ಒಟ್ಟು_written * 1000) / llcache->ಒಟ್ಟು_elapsed;

		ಆದರೆ (ಒಟ್ಟು_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      ಒಟ್ಟು_bandwidth,
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
ಸ್ಥಿರ ಶೂನ್ಯ llcache_persist(ಶೂನ್ಯ *p)
{
	nserror ret;
	ರಚನೆ llcache_object **lst; /* candidate object list */
	ಪೂರ್ಣಾಂಕ lst_ಎಣಿಕೆ; /* number of candidates in list */
	ಪೂರ್ಣಾಂಕ idx; /* current candidate object index in list */
	ಪೂರ್ಣಾಂಕ ಮುಂದಿನ = -1; /* when the next run should be scheduled for */

	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ write_ಎಲ್ಲೆ; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ elapsed; /* how long writing an object took */

	size_t ಒಟ್ಟು_written = 0; /* total bytes written in this run */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು_elapsed = 1; /* total ms used to write bytes */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_ಎಣಿಕೆ);
	ಆದರೆ (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		ಹಿಂತಿರುಗಿ;
	}

	write_ಎಲ್ಲೆ = (llcache->ಗರಿಷ್ಠ_bandwidth * llcache->ಸಮಯ_ಕಾಲಾಂಶ) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	ಗಾಗಿ (idx = 0; idx < lst_ಎಣಿಕೆ; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		ಆದರೆ (ret != NSERROR_OK) {
			ಮುಂದುವರಿ;
		}

		/* successfully wrote object to backing store */
		ಒಟ್ಟು_written += written;
		ಒಟ್ಟು_elapsed += elapsed;
		ಒಟ್ಟು_bandwidth = (ಒಟ್ಟು_written * 1000) / ಒಟ್ಟು_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		ಆದರೆ (ಒಟ್ಟು_elapsed > llcache->ಸಮಯ_ಕಾಲಾಂಶ) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			ಆದರೆ (ಒಟ್ಟು_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->ವೇಳಾಪಟ್ಟಿ(
						llcache->ಸಮಯ_ಕಾಲಾಂಶ * 100,
						llcache_persist_slowcheck,
						ಖಾಲಿ);
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (ಒಟ್ಟು_bandwidth > llcache->ಗರಿಷ್ಠ_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					ಮುಂದಿನ = ((ಒಟ್ಟು_written * llcache->ಸಮಯ_ಕಾಲಾಂಶ) / write_ಎಲ್ಲೆ) - ಒಟ್ಟು_elapsed;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಮುಂದಿನ = llcache->ಸಮಯ_ಕಾಲಾಂಶ;
				}
				ನಿಲ್ಲಿಸು;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಟ್ಟು_written > write_ಎಲ್ಲೆ) {
			/* The bandwidth limit has been reached. */

			ಆದರೆ (ಒಟ್ಟು_bandwidth > llcache->ಗರಿಷ್ಠ_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				ಮುಂದಿನ = ((ಒಟ್ಟು_written * llcache->ಸಮಯ_ಕಾಲಾಂಶ) / write_ಎಲ್ಲೆ) - ಒಟ್ಟು_elapsed;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಮುಂದಿನ = llcache->ಸಮಯ_ಕಾಲಾಂಶ - ಒಟ್ಟು_elapsed;
			}
			ನಿಲ್ಲಿಸು;
		}

	}
	ಬಿಡುಗಡೆ(lst);

	/* Completed list without running out of allowed bytes or time */
	ಆದರೆ (idx == lst_ಎಣಿಕೆ) {
		/* only reschedule if writing is making any progress at all */
		ಆದರೆ (ಒಟ್ಟು_written > 0) {
			ಮುಂದಿನ = llcache->ಸಮಯ_ಕಾಲಾಂಶ - ಒಟ್ಟು_elapsed;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಮುಂದಿನ = -1;
		}
	}

	llcache->ಒಟ್ಟು_written += ಒಟ್ಟು_written;
	llcache->ಒಟ್ಟು_elapsed += ಒಟ್ಟು_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      ಒಟ್ಟು_written, ಒಟ್ಟು_elapsed, ಒಟ್ಟು_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", ಮುಂದಿನ);
	guit->misc->ವೇಳಾಪಟ್ಟಿ(ಮುಂದಿನ, llcache_persist, ಖಾಲಿ);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
ಸ್ಥಿರ ಶೂನ್ಯ llcache_fetch_callback(ಸ್ಥಿರಾಂಕ fetch_ಸಂದೇಶ *ಸಂದೇಶ, ಶೂನ್ಯ *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	ಆದರೆ (llcache == ಖಾಲಿ) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		ಪ್ರತಿಜ್ಞಾ(false);
		/* In case assertions are off, return here */
		ಹಿಂತಿರುಗಿ;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", ಸಂದೇಶ->type, object);

	ಆಯ್ಕೆ (ಸಂದೇಶ->type) {
	ಸ್ಥಿತಿ FETCH_ಶೀರ್ಷಿಕೆ:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು;

		error = llcache_fetch_process_ಶೀರ್ಷಿಕೆ(object,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len);
		ನಿಲ್ಲಿಸು;

	/* 3xx responses */
	ಸ್ಥಿತಿ FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		error = llcache_fetch_redirect(object,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.redirect, &object);
		ಆದರೆ (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = error;
			event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = ಖಾಲಿ;
			error = llcache_send_event_ಗಮ್ಯ_users(object, &event);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		ನಿಲ್ಲಿಸು;

	/* Normal 2xx state machine */
	ಸ್ಥಿತಿ FETCH_ದತ್ತಾಂಶ:
		/* Received some data */
		error = llcache_fetch_process_ದತ್ತಾಂಶ(object,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.buf,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.ಶೀರ್ಷಿಕೆ_or_ದತ್ತಾಂಶ.len);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_ಪೂರ್ಣಗೊಂಡ:
		/* Finished fetching */
	{
		uint8_t *ತಾತ್ಕಾಲಿಕ;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ಖಾಲಿ;

		/* Shrink source buffer to required size */
		ತಾತ್ಕಾಲಿಕ = realloc(object->ಮೂಲ_ದತ್ತಾಂಶ,
				object->ಮೂಲ_len);
		/* If source_len is 0, then temp may be NULL */
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ != ಖಾಲಿ || object->ಮೂಲ_len == 0) {
			object->ಮೂಲ_ದತ್ತಾಂಶ = ತಾತ್ಕಾಲಿಕ;
			object->ಮೂಲ_alloc = object->ಮೂಲ_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_ಸಮಯ = time(ಖಾಲಿ);

		(ಶೂನ್ಯ) llcache_hsts_update_policy(object);

		guit->misc->ವೇಳಾಪಟ್ಟಿ(5000, llcache_persist, ಖಾಲಿ);
	}
		ನಿಲ್ಲಿಸು;

	/* Out-of-band information */
	ಸ್ಥಿತಿ FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ಖಾಲಿ;

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2 = NSERROR_UNKNOWN;
		event.ದತ್ತಾಂಶ.error.ಸಂದೇಶ = ಸಂದೇಶ->ದತ್ತಾಂಶ.error;

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);

		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_ಪ್ರಗತಿ:
		/* Progress update */
		event.type = LLCACHE_EVENT_ಪ್ರಗತಿ;
		event.ದತ್ತಾಂಶ.ಪ್ರಗತಿ_ಸಂದೇಶ = ಸಂದೇಶ->ದತ್ತಾಂಶ.ಪ್ರಗತಿ;

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);

		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_CERTS:
		/* Certificate information from the fetch */

		ಆದರೆ (object->chain != ಖಾಲಿ) {
			cert_chain_free(object->chain);
			object->chain = ಖಾಲಿ;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(ಸಂದೇಶ->ದತ್ತಾಂಶ.chain, &object->chain);
		ಆದರೆ (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.ದತ್ತಾಂಶ.chain = ಸಂದೇಶ->ದತ್ತಾಂಶ.chain;

		error = llcache_send_event_ಗಮ್ಯ_users(object, &event);

		ನಿಲ್ಲಿಸು;

	/* Events requiring action */
	ಸ್ಥಿತಿ FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		error = llcache_fetch_auth(object, ಸಂದೇಶ->ದತ್ತಾಂಶ.auth.realm);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		error = llcache_fetch_cert_error(object);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		ಆದರೆ (object->candidate != ಖಾಲಿ) {
			object->candidate->candidate_ಎಣಿಕೆ--;
			object->candidate = ಖಾಲಿ;
		}

		error = llcache_fetch_ssl_error(object);
		ನಿಲ್ಲಿಸು;
	}

	/* Deal with any errors reported by event handlers */
	ಆದರೆ (error != NSERROR_OK) {
		ಆದರೆ (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		ಆದರೆ (object->fetch.fetch != ಖಾಲಿ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ಖಾಲಿ;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_ಮೇಲೆ();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
ಸ್ಥಿರ llcache_object_user *llcache_object_ಕಂಡುಹಿಡಿ_user(
		ಸ್ಥಿರಾಂಕ llcache_handle *handle)
{
	llcache_object_user *user;

	ಪ್ರತಿಜ್ಞಾ(handle->object != ಖಾಲಿ);

	ಗಾಗಿ (user = handle->object->users; user != ಖಾಲಿ; user = user->ಮುಂದಿನ) {
		ಆದರೆ (user->handle == handle)
			ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
ಸ್ಥಿರ bool llcache_object_ಒಳ_list(ಸ್ಥಿರಾಂಕ llcache_object *object,
		ಸ್ಥಿರಾಂಕ llcache_object *list)
{
	ಆಗ (list != ಖಾಲಿ) {
		ಆದರೆ (list == object)
			ನಿಲ್ಲಿಸು;

		list = list->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ list != ಖಾಲಿ;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *ಮುಂದಿನ_user;
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

	ಗಾಗಿ (user = object->users; user != ಖಾಲಿ; user = ಮುಂದಿನ_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ಸ್ಥಿರಾಂಕ llcache_fetch_state objstate = object->fetch.state;

		/* Flag that this user is the current iteration target
		 * in case the client attempts to destroy it underneath us */
		user->iterator_ಗುರಿ = true;

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

		ಆದರೆ (handle->state != objstate) {
			ಆದರೆ (emitted_notify == false) {
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
		ಆದರೆ (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು;

			/* Emit any certificate data we hold */
			ಆದರೆ (object->chain != ಖಾಲಿ) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.ದತ್ತಾಂಶ.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} ಇಲ್ಲದಿದ್ದರೆ {
				error = NSERROR_OK;
			}

			ಆದರೆ (user->queued_for_delete) {
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ಆದರೆ (error != NSERROR_OK)
					ಹಿಂತಿರುಗಿ error;

				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error == NSERROR_NEED_ದತ್ತಾಂಶ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ಗುರಿ = false;
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_users_not_caught_ಮೇಲೆ();
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error != NSERROR_OK) {
				user->iterator_ಗುರಿ = false;
				ಹಿಂತಿರುಗಿ error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		ಆದರೆ (handle->state == LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು &&
				objstate > LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು) {
			handle->state = LLCACHE_FETCH_ದತ್ತಾಂಶ;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_ಶೀರ್ಷಿಕೆಗಳು;

			error = handle->cb(handle, &event, handle->pw);

			ಆದರೆ (user->queued_for_delete) {
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ಆದರೆ (error != NSERROR_OK)
					ಹಿಂತಿರುಗಿ error;

				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error == NSERROR_NEED_ದತ್ತಾಂಶ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ಶೀರ್ಷಿಕೆಗಳು;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ಗುರಿ = false;
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_users_not_caught_ಮೇಲೆ();
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error != NSERROR_OK) {
				user->iterator_ಗುರಿ = false;
				ಹಿಂತಿರುಗಿ error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		ಆದರೆ (handle->state == LLCACHE_FETCH_ದತ್ತಾಂಶ &&
				objstate >= LLCACHE_FETCH_ದತ್ತಾಂಶ &&
				object->ಮೂಲ_len > handle->ಬೈಟುಗಳು) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_ದತ್ತಾಂಶ;
			event.ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.buf =
					object->ಮೂಲ_ದತ್ತಾಂಶ + handle->ಬೈಟುಗಳು;
			event.ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.len =
					object->ಮೂಲ_len - handle->ಬೈಟುಗಳು;

			/* Update record of last byte emitted */
			ಆದರೆ (object->fetch.flags &
					LLCACHE_RETRIEVE_ಹರಿವು_ದತ್ತಾಂಶ) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->ಬೈಟುಗಳು = object->ಮೂಲ_len = 0;
			} ಇಲ್ಲದಿದ್ದರೆ {
				orig_handle_read = handle->ಬೈಟುಗಳು;
				handle->ಬೈಟುಗಳು = object->ಮೂಲ_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			ಆದರೆ (user->queued_for_delete) {
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ಆದರೆ (error != NSERROR_OK)
					ಹಿಂತಿರುಗಿ error;

				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error == NSERROR_NEED_ದತ್ತಾಂಶ) {
				/* User requested replay */
				handle->ಬೈಟುಗಳು = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_ಗುರಿ = false;
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_users_not_caught_ಮೇಲೆ();
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error != NSERROR_OK) {
				user->iterator_ಗುರಿ = false;
				ಹಿಂತಿರುಗಿ error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		ಆದರೆ (handle->state == LLCACHE_FETCH_ದತ್ತಾಂಶ &&
				objstate > LLCACHE_FETCH_ದತ್ತಾಂಶ) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_ಮುಗಿದ;

			error = handle->cb(handle, &event, handle->pw);
			ಆದರೆ (user->queued_for_delete) {
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ಆದರೆ (error != NSERROR_OK)
					ಹಿಂತಿರುಗಿ error;

				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error == NSERROR_NEED_ದತ್ತಾಂಶ) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_ದತ್ತಾಂಶ;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_ಗುರಿ = false;
				ಮುಂದಿನ_user = user->ಮುಂದಿನ;
				llcache_users_not_caught_ಮೇಲೆ();
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error != NSERROR_OK) {
				user->iterator_ಗುರಿ = false;
				ಹಿಂತಿರುಗಿ error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_ಗುರಿ = false;

		ಮುಂದಿನ_user = user->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	newobj->ಮೂಲ_alloc = newobj->ಮೂಲ_len = object->ಮೂಲ_len;

	ಆದರೆ (object->ಮೂಲ_len > 0) {
		newobj->ಮೂಲ_ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(newobj->ಮೂಲ_alloc);
		ಆದರೆ (newobj->ಮೂಲ_ದತ್ತಾಂಶ == ಖಾಲಿ) {
			llcache_object_destroy(newobj);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		ಸ್ಮರಣೆ_ನಕಲು(newobj->ಮೂಲ_ದತ್ತಾಂಶ, object->ಮೂಲ_ದತ್ತಾಂಶ,
				newobj->ಮೂಲ_len);
	}

	ಆದರೆ (object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು > 0) {
		newobj->ಶೀರ್ಷಿಕೆಗಳು = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು,
				ಗಾತ್ರ(llcache_ಶೀರ್ಷಿಕೆ));
		ಆದರೆ (newobj->ಶೀರ್ಷಿಕೆಗಳು == ಖಾಲಿ) {
			llcache_object_destroy(newobj);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		ಆಗ (newobj->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು) {
			llcache_ಶೀರ್ಷಿಕೆ *nh =
					&(newobj->ಶೀರ್ಷಿಕೆಗಳು[newobj->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು]);
			llcache_ಶೀರ್ಷಿಕೆ *oh =
					&(object->ಶೀರ್ಷಿಕೆಗಳು[newobj->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು]);
			newobj->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು += 1;
			nh->name = strdup(oh->name);
			nh->ಮೌಲ್ಯ = strdup(oh->ಮೌಲ್ಯ);
			ಆದರೆ (nh->name == ಖಾಲಿ || nh->ಮೌಲ್ಯ == ಖಾಲಿ) {
				llcache_object_destroy(newobj);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
		}
	}

	ಆದರೆ (object->chain != ಖಾಲಿ) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		ಆದರೆ (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			ಹಿಂತಿರುಗಿ error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ uint32_t
ಒಟ್ಟು_object_ಗಾತ್ರ(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = ಗಾತ್ರ(*object);
	tot += nsurl_ಉದ್ದಳತೆ(object->url);

	ಆದರೆ (object->ಮೂಲ_ದತ್ತಾಂಶ != ಖಾಲಿ) {
		tot += object->ಮೂಲ_len;
	}

	tot += ಗಾತ್ರ(llcache_ಶೀರ್ಷಿಕೆ) * object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು;

	ಗಾಗಿ (hdrc = 0; hdrc < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು; hdrc++) {
		ಆದರೆ (object->ಶೀರ್ಷಿಕೆಗಳು[hdrc].name != ಖಾಲಿ) {
			tot += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(object->ಶೀರ್ಷಿಕೆಗಳು[hdrc].name);
		}
		ಆದರೆ (object->ಶೀರ್ಷಿಕೆಗಳು[hdrc].ಮೌಲ್ಯ != ಖಾಲಿ) {
			tot += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(object->ಶೀರ್ಷಿಕೆಗಳು[hdrc].ಮೌಲ್ಯ);
		}
	}

	tot += cert_chain_ಗಾತ್ರ(object->chain);

	ಹಿಂತಿರುಗಿ tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
ಸ್ಥಿರ ಶೂನ್ಯ llcache_catch_ಮೇಲೆ_all_users(ಶೂನ್ಯ *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_ಮೇಲೆ = true;

	/* Catch new users up with state of objects */
	ಗಾಗಿ (object = llcache->cached_objects; object != ಖಾಲಿ;
			object = object->ಮುಂದಿನ) {
		llcache_object_notify_users(object);
	}

	ಗಾಗಿ (object = llcache->uncached_objects; object != ಖಾಲಿ;
			object = object->ಮುಂದಿನ) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
ಸ್ಥಿರ ಶೂನ್ಯ llcache_users_not_caught_ಮೇಲೆ(ಶೂನ್ಯ)
{
	ಆದರೆ (llcache->all_caught_ಮೇಲೆ) {
		llcache->all_caught_ಮೇಲೆ = false;
		guit->misc->ವೇಳಾಪಟ್ಟಿ(0, llcache_catch_ಮೇಲೆ_all_users, ಖಾಲಿ);
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
ಶೂನ್ಯ llcache_clean(bool purge)
{
	llcache_object *object, *ಮುಂದಿನ;
	uint32_t llcache_ಗಾತ್ರ = 0;
	ಪೂರ್ಣಾಂಕ ಉಳಿದ_lifetime;
	uint32_t ಎಲ್ಲೆ;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	ಆದರೆ (purge) {
		ಎಲ್ಲೆ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಎಲ್ಲೆ = llcache->ಎಲ್ಲೆ;
	}

	/* Uncacheable objects with no users or fetches */
	ಗಾಗಿ (object = llcache->uncached_objects;
	     object != ಖಾಲಿ;
	     object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		/* The candidate count of uncacheable objects is always 0 */
		ಆದರೆ ((object->users == ಖಾಲಿ) &&
		    (object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} ಇಲ್ಲದಿದ್ದರೆ {
			llcache_ಗಾತ್ರ += ಒಟ್ಟು_object_ಗಾತ್ರ(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	ಗಾಗಿ (object = llcache->cached_objects;
	     object != ಖಾಲಿ;
	     object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಉಳಿದ_lifetime = llcache_object_rfc2616_ಉಳಿದ_lifetime(
				&object->cache);

		ಆದರೆ ((object->users == ಖಾಲಿ) &&
		    (object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ) &&
		    (ಉಳಿದ_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				ಆದರೆ (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* object has users so account for the storage */
			llcache_ಗಾತ್ರ += ಒಟ್ಟು_object_ಗಾತ್ರ(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	ಆದರೆ (ಎಲ್ಲೆ < llcache_ಗಾತ್ರ) {
		llcache_persist(ಖಾಲಿ);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	ಗಾಗಿ (object = llcache->cached_objects;
	     ((ಎಲ್ಲೆ < llcache_ಗಾತ್ರ) && (object != ಖಾಲಿ));
	     object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;
		ಆದರೆ ((object->users == ಖಾಲಿ) &&
		    (object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->ಮೂಲ_ದತ್ತಾಂಶ = ಖಾಲಿ;

			llcache_ಗಾತ್ರ -=	object->ಮೂಲ_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->ಮೂಲ_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	ಗಾಗಿ (object = llcache->cached_objects;
	     ((ಎಲ್ಲೆ < llcache_ಗಾತ್ರ) && (object != ಖಾಲಿ));
	     object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;
		ಆದರೆ ((object->users == ಖಾಲಿ) &&
		    (object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->ಮೂಲ_ದತ್ತಾಂಶ == ಖಾಲಿ)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ಮೂಲ_len,
			      (ದೀರ್ಘ)(time(ಖಾಲಿ) - object->ಕೊನೆಯ_ಬಳಸಿದ),
			      object,
			      nsurl_access(object->url));

			llcache_ಗಾತ್ರ -=	ಒಟ್ಟು_object_ಗಾತ್ರ(object);

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
	ಗಾಗಿ (object = llcache->cached_objects;
	     ((ಎಲ್ಲೆ < llcache_ಗಾತ್ರ) && (object != ಖಾಲಿ));
	     object = ಮುಂದಿನ) {
		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಆದರೆ ((object->users == ಖಾಲಿ) &&
		    (object->candidate_ಎಣಿಕೆ == 0) &&
		    (object->fetch.fetch == ಖಾಲಿ) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ಮೂಲ_len,
			      (ದೀರ್ಘ)(time(ಖಾಲಿ) - object->ಕೊನೆಯ_ಬಳಸಿದ),
			      object,
			      nsurl_access(object->url));

			llcache_ಗಾತ್ರ -=	object->ಮೂಲ_len + ಗಾತ್ರ(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_ಗಾತ್ರ, ಎಲ್ಲೆ);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ಸ್ಥಿರಾಂಕ ರಚನೆ llcache_parameters *prm)
{
	llcache = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ llcache_s));
	ಆದರೆ (llcache == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	llcache->ಎಲ್ಲೆ = prm->ಎಲ್ಲೆ;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->ಗರಿಷ್ಠ_bandwidth = prm->ಗರಿಷ್ಠ_bandwidth;
	llcache->ಸಮಯ_ಕಾಲಾಂಶ = prm->ಸಮಯ_ಕಾಲಾಂಶ;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_ಮೇಲೆ = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->ಎಲ್ಲೆ);

	/* backing store initialisation */
	ಹಿಂತಿರುಗಿ guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
ಶೂನ್ಯ llcache_finalise(ಶೂನ್ಯ)
{
	llcache_object *object, *ಮುಂದಿನ;
	uint64_t ಒಟ್ಟು_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(ಖಾಲಿ);
	/* Now clear the persistence callback */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, llcache_persist, ಖಾಲಿ);

	/* Clean uncached objects */
	ಗಾಗಿ (object = llcache->uncached_objects; object != ಖಾಲಿ; object = ಮುಂದಿನ) {
		llcache_object_user *user, *ಮುಂದಿನ_user;

		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಗಾಗಿ (user = object->users; user != ಖಾಲಿ; user = ಮುಂದಿನ_user) {
			ಮುಂದಿನ_user = user->ಮುಂದಿನ;

			ಆದರೆ (user->handle != ಖಾಲಿ)
				ಬಿಡುಗಡೆ(user->handle);

			ಬಿಡುಗಡೆ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ಖಾಲಿ;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	ಗಾಗಿ (object = llcache->cached_objects; object != ಖಾಲಿ; object = ಮುಂದಿನ) {
		llcache_object_user *user, *ಮುಂದಿನ_user;

		ಮುಂದಿನ = object->ಮುಂದಿನ;

		ಗಾಗಿ (user = object->users; user != ಖಾಲಿ; user = ಮುಂದಿನ_user) {
			ಮುಂದಿನ_user = user->ಮುಂದಿನ;

			ಆದರೆ (user->handle != ಖಾಲಿ)
			       ಬಿಡುಗಡೆ(user->handle);

			ಬಿಡುಗಡೆ(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ಖಾಲಿ;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	ಆದರೆ (llcache->ಒಟ್ಟು_elapsed > 0) {
		ಒಟ್ಟು_bandwidth = (llcache->ಒಟ್ಟು_written * 1000) /
			llcache->ಒಟ್ಟು_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->ಒಟ್ಟು_written,
	      llcache->ಒಟ್ಟು_elapsed,
	      ಒಟ್ಟು_bandwidth);

	ಬಿಡುಗಡೆ(llcache);
	llcache = ಖಾಲಿ;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ಸ್ಥಿರಾಂಕ llcache_post_ದತ್ತಾಂಶ *post,
			llcache_handle_callback cb, ಶೂನ್ಯ *pw,
			llcache_handle **ಫಲಿತಾಂಶ)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_ಒಳ_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_ಒಳ_use);
	ಆದರೆ (error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	/* Can we fetch this URL at all? */
	ಆದರೆ (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	ಆದರೆ (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_ಒಳ_use, &object);
	ಆದರೆ (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Add user to object */
	llcache_object_ಕೂಡಿಸು_user(object, user);

	*ಫಲಿತಾಂಶ = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_ಮೇಲೆ();

	nsurl_unref(hsts_url);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, ಶೂನ್ಯ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_ಕಂಡುಹಿಡಿ_user(handle);

	ಪ್ರತಿಜ್ಞಾ(user != ಖಾಲಿ);

	ಆದರೆ (user->iterator_ಗುರಿ) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		ಆದರೆ (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	ಹಿಂತಿರುಗಿ error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ಫಲಿತಾಂಶ)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	ಆದರೆ (error == NSERROR_OK) {
		llcache_object_ಕೂಡಿಸು_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*ಫಲಿತಾಂಶ = newuser->handle;
	}

	ಹಿಂತಿರುಗಿ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ಕಂಡುಹಿಡಿ_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	ಆದರೆ (user->prev != ಖಾಲಿ)
		all_alone = false;
	ಆದರೆ (user->ಮುಂದಿನ != ಖಾಲಿ)
		all_alone = false;

	ಆದರೆ (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		ಆದರೆ (error != NSERROR_OK)
			ಹಿಂತಿರುಗಿ error;

		/* Move across to the new object */
		ಆದರೆ (user->iterator_ಗುರಿ) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(llcache_object_user));
			ಆದರೆ (newuser == ಖಾಲಿ) {
				llcache_object_destroy(newobject);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = ಖಾಲಿ;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_ಕೂಡಿಸು_user(newobject, newuser);
		} ಇಲ್ಲದಿದ್ದರೆ {
			llcache_object_remove_user(object, user);
			llcache_object_ಕೂಡಿಸು_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(newobject,
				&llcache->uncached_objects);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* We're the only user, so abort any fetch in progress */
		ಆದರೆ (object->fetch.fetch != ಖಾಲಿ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ಖಾಲಿ;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_ದತ್ತಾಂಶ(object);
	}

	ಹಿಂತಿರುಗಿ error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_ಹರಿವು(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_ಕಂಡುಹಿಡಿ_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	ಆದರೆ (user->prev != ಖಾಲಿ || user->ಮುಂದಿನ != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	/* Forcibly uncache this object */
	ಆದರೆ (llcache_object_ಒಳ_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_ಕೂಡಿಸು_ಗಮ್ಯ_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_ಹರಿವು_ದತ್ತಾಂಶ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_ದತ್ತಾಂಶ(llcache_handle *handle)
{
	ಆದರೆ ((handle->object != ಖಾಲಿ) &&
	    (handle->object->fetch.fetch == ಖಾಲಿ) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ಸ್ಥಿರಾಂಕ llcache_handle *handle)
{
	ಹಿಂತಿರುಗಿ handle->object != ಖಾಲಿ ? handle->object->url : ಖಾಲಿ;
}

/* See llcache.h for documentation */
ಸ್ಥಿರಾಂಕ uint8_t *llcache_handle_get_ಮೂಲ_ದತ್ತಾಂಶ(ಸ್ಥಿರಾಂಕ llcache_handle *handle,
		size_t *ಗಾತ್ರ_2)
{
	*ಗಾತ್ರ_2 = handle->object != ಖಾಲಿ ? handle->object->ಮೂಲ_len : 0;

	ಹಿಂತಿರುಗಿ handle->object != ಖಾಲಿ ? handle->object->ಮೂಲ_ದತ್ತಾಂಶ : ಖಾಲಿ;
}

/* See llcache.h for documentation */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *llcache_handle_get_ಶೀರ್ಷಿಕೆ(ಸ್ಥಿರಾಂಕ llcache_handle *handle,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ)
{
	ಸ್ಥಿರಾಂಕ llcache_object *object = handle->object;
	size_t i;

	ಆದರೆ (object == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* About as trivial as possible */
	ಗಾಗಿ (i = 0; i < object->ಅಂಕೆ_ಶೀರ್ಷಿಕೆಗಳು; i++) {
		ಆದರೆ (strcasecmp(ಕೀಲಿ, object->ಶೀರ್ಷಿಕೆಗಳು[i].name) == 0)
			ಹಿಂತಿರುಗಿ object->ಶೀರ್ಷಿಕೆಗಳು[i].ಮೌಲ್ಯ;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ಸ್ಥಿರಾಂಕ llcache_handle *a,
		ಸ್ಥಿರಾಂಕ llcache_handle *b)
{
	ಹಿಂತಿರುಗಿ a->object == b->object;
}
