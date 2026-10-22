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

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <nsutils/time.h>
#అన్తర్భావ <nsutils/base64.h>

#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/time.h"
#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/backing_store.h"
#అన్తర్భావ "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
రకనిర్వచనం గణన {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_శీర్షికలు,		/**< Fetching headers */
	LLCACHE_FETCH_దత్తాంశం,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
రకనిర్వచనం నిర్మాణం llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
నిర్మాణం llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	శూన్య *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t బైట్లు;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
రకనిర్వచనం నిర్మాణం llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_లక్ష్యం;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	నిర్మాణం llcache_object_user *prev;	/**< Previous in list */
	నిర్మాణం llcache_object_user *తదుపరి;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
రకనిర్వచనం నిర్మాణం {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_దత్తాంశం *post;	/**< POST data, or NULL for GET */

	నిర్మాణం fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_లెక్క;	/**< Count of redirects followed */

	uint32_t retries_మిగిలిన;     /**< Number of times to retry on timeout */

	bool hsts_లోపల_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
రకనిర్వచనం గణన {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#నిర్వచించు INVALID_AGE -1

/** Cache control data */
రకనిర్వచనం నిర్మాణం {
	time_t req_సమయం;	/**< Time of request */
	time_t res_సమయం;	/**< Time of response */
	time_t fin_సమయం;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	పూర్ణాంకం age;		/**< Age: response header */
	పూర్ణాంకం max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	అక్షరం *etag;		/**< Etag: response header */
	time_t చివరి_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
రకనిర్వచనం నిర్మాణం {
	అక్షరం *name;		/**< Header name */
	అక్షరం *విలువ;		/**< Header value */
} llcache_శీర్షిక;

/** Current status of an object's data */
రకనిర్వచనం గణన {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
నిర్మాణం llcache_object {
	llcache_object *prev;	     /**< Previous in list */
	llcache_object *తదుపరి;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *ఆధారం_దత్తాంశం;	     /**< Source data for object */
	size_t ఆధారం_len;	     /**< Byte length of source data */
	size_t ఆధారం_alloc;	     /**< Allocated size of source buffer */

	నిర్మాణం cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_లెక్క;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_శీర్షిక *శీర్షికలు;     /**< Fetch headers */
	size_t అంకె_శీర్షికలు;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t చివరి_వాడిన; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
నిర్మాణం llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t హద్దు;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_పైకి;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	పూర్ణాంకం minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	గుర్తులేని దీర్ఘ సమయం_కాలాంశం;

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
	size_t గరిష్ఠ_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t మొత్తం_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t మొత్తం_elapsed;

};

/** low level cache state */
స్థిర నిర్మాణం llcache_s *llcache = ఖాళీ;

/* forward referenced callback function */
స్థిర శూన్య llcache_fetch_callback(స్థిరరాశి fetch_సందేశం *సందేశం, శూన్య *p);

/* forward referenced catch up function */
స్థిర శూన్య llcache_users_not_caught_పైకి(శూన్య);


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
స్థిర nserror llcache_object_user_new(llcache_handle_callback cb, శూన్య *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(llcache_handle));
	ఒకవేళ (h == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	u = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(llcache_object_user));
	ఒకవేళ (u == ఖాళీ) {
		విడుదల(h);
		తిరిగి NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (శూన్య *) cb, pw);

	*user = u;

	తిరిగి NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
స్థిర nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	ప్రతిజ్ఞా(user->తదుపరి == ఖాళీ);
	ప్రతిజ్ఞా(user->prev == ఖాళీ);

	ఒకవేళ (user->handle != ఖాళీ)
		విడుదల(user->handle);

	విడుదల(user);

	తిరిగి NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
స్థిర nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	ప్రతిజ్ఞా(user != ఖాళీ);
	ప్రతిజ్ఞా(object != ఖాళీ);
	ప్రతిజ్ఞా(object->users != ఖాళీ);
	ప్రతిజ్ఞా(user->handle == ఖాళీ || user->handle->object == object);
	ప్రతిజ్ఞా((user->prev != ఖాళీ) || (object->users == user));

	ఒకవేళ (user == object->users)
		object->users = user->తదుపరి;
	లేకపోతే
		user->prev->తదుపరి = user->తదుపరి;

	ఒకవేళ (user->తదుపరి != ఖాళీ)
		user->తదుపరి->prev = user->prev;

	user->తదుపరి = user->prev = ఖాళీ;

	/* record the time the last user was removed from the object */
	ఒకవేళ (object->users == ఖాళీ) {
		object->చివరి_వాడిన = time(ఖాళీ);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	తిరిగి NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
స్థిర nserror llcache_send_event_గమ్యం_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *తదుపరి_user;

	user = object->users;
	అయితే (user != ఖాళీ) {
		bool was_లక్ష్యం = user->iterator_లక్ష్యం;
		user->iterator_లక్ష్యం = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		తదుపరి_user = user->తదుపరి;

		user->iterator_లక్ష్యం = was_లక్ష్యం;

		ఒకవేళ (user->queued_for_delete && !was_లక్ష్యం) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		ఒకవేళ (error != NSERROR_OK)
			ఆపు;

		user = తదుపరి_user;
	}

	తిరిగి error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_object_new(nsurl *url, llcache_object **ఫలితం)
{
	llcache_object *obj = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(llcache_object));
	ఒకవేళ (obj == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*ఫలితం = obj;

	తిరిగి NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_post_దత్తాంశం_clone(స్థిరరాశి llcache_post_దత్తాంశం *orig,
		llcache_post_దత్తాంశం **clone)
{
	llcache_post_దత్తాంశం *post_clone;

	post_clone = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(llcache_post_దత్తాంశం));
	ఒకవేళ (post_clone == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	ఒకవేళ (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->దత్తాంశం.urlenc = strdup(orig->దత్తాంశం.urlenc);
		ఒకవేళ (post_clone->దత్తాంశం.urlenc == ఖాళీ) {
			విడుదల(post_clone);

			తిరిగి NSERROR_NOMEM;
		}
	} లేకపోతే {
		post_clone->దత్తాంశం.multipart = fetch_multipart_దత్తాంశం_clone(
				orig->దత్తాంశం.multipart);
		ఒకవేళ (post_clone->దత్తాంశం.multipart == ఖాళీ) {
			విడుదల(post_clone);

			తిరిగి NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	తిరిగి NSERROR_OK;
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
స్థిర nserror llcache_fetch_విభజించు_శీర్షిక(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		అక్షరం **name, అక్షరం **విలువ)
{
	అక్షరం *n, *v;
	స్థిరరాశి uint8_t *colon;

	/* Strip leading whitespace from name */
	అయితే (దత్తాంశం[0] == ' ' || దత్తాంశం[0] == '\t' ||
	       దత్తాంశం[0] == '\r' || దత్తాంశం[0] == '\n') {
		దత్తాంశం++;
	}

	/* Find colon */
	colon = (స్థిరరాశి uint8_t *) strchr((స్థిరరాశి అక్షరం *) దత్తాంశం, ':');
	ఒకవేళ (colon == ఖాళీ) {
		/* Failed, assume a key with no value */
		colon = దత్తాంశం + అక్షరశ్రేణి_పొడవు((స్థిరరాశి అక్షరం *)దత్తాంశం);

		/* Strip trailing whitespace from name */
		అయితే ((colon > దత్తాంశం) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((స్థిరరాశి అక్షరం *) దత్తాంశం, colon - దత్తాంశం);
		ఒకవేళ (n == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		v = strdup("");
		ఒకవేళ (v == ఖాళీ) {
			విడుదల(n);
			తిరిగి NSERROR_NOMEM;
		}
	} లేకపోతే {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		అయితే (colon > దత్తాంశం && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((స్థిరరాశి అక్షరం *) దత్తాంశం, colon - దత్తాంశం);
		ఒకవేళ (n == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		/* Find colon again */
		అయితే (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		చేయి {
			colon++;
		} అయితే (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		అయితే (len > 0 && (దత్తాంశం[len - 1] == ' ' ||
				దత్తాంశం[len - 1] == '\t' ||
				దత్తాంశం[len - 1] == '\r' ||
				దత్తాంశం[len - 1] == '\n')) {
			len--;
		}

		v = strndup((స్థిరరాశి అక్షరం *) colon, len - (colon - దత్తాంశం));
		ఒకవేళ (v == ఖాళీ) {
			విడుదల(n);
			తిరిగి NSERROR_NOMEM;
		}
	}

	*name = n;
	*విలువ = v;

	తిరిగి NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror
llcache_fetch_parse_cache_control(llcache_object *object, అక్షరం *విలువ)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(విలువ, &cc);
	ఒకవేళ (error != NSERROR_OK) {
		/* Ignore parse errors */
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	ఒకవేళ (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	తిరిగి NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror
llcache_fetch_శీర్షిక_cache_control(llcache_object *object,
				   అక్షరం *name,
				   అక్షరం *విలువ)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = అక్షరశ్రేణి_పొడవు(name);

	ఎంపిక (name_len) {
	స్థితి 3:
		ఒకవేళ (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			ఒకవేళ ('0' <= *విలువ && *విలువ <= '9') {
				object->cache.age = atoi(విలువ);
			}

		}
		ఆపు;

	స్థితి 4:
		ఒకవేళ (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(విలువ,
					    అక్షరశ్రేణి_పొడవు(విలువ),
					    &object->cache.date);
			ఒకవేళ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      విలువ, res);
			}
		} లేకపోతే ఒకవేళ (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			విడుదల(object->cache.etag);
			object->cache.etag = strdup(విలువ);
			ఒకవేళ (object->cache.etag == ఖాళీ) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				తిరిగి NSERROR_NOMEM;
			}
		}
		ఆపు;

	స్థితి 7:
		ఒకవేళ (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(విలువ,
					    అక్షరశ్రేణి_పొడవు(విలువ),
					    &object->cache.expires);
			ఒకవేళ (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      విలువ, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		ఆపు;

	స్థితి 13:
		ఒకవేళ (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,విలువ);
		} లేకపోతే ఒకవేళ (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(విలువ,
				      అక్షరశ్రేణి_పొడవు(విలువ),
				      &object->cache.చివరి_modified);
		}
		ఆపు;
	}

	తిరిగి NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
స్థిర అంతఃవరుస శూన్య llcache_destroy_శీర్షికలు(llcache_object *object)
{
	అయితే (object->అంకె_శీర్షికలు > 0) {
		object->అంకె_శీర్షికలు--;

		విడుదల(object->శీర్షికలు[object->అంకె_శీర్షికలు].name);
		విడుదల(object->శీర్షికలు[object->అంకె_శీర్షికలు].విలువ);
	}
	విడుదల(object->శీర్షికలు);
	object->శీర్షికలు = ఖాళీ;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
స్థిర అంతఃవరుస శూన్య llcache_invalidate_cache_control_దత్తాంశం(llcache_object *object)
{
	విడుదల(object->cache.etag);
	memset(&(object->cache), 0, పరిమాణం(llcache_cache_control));

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
స్థిర nserror llcache_fetch_process_శీర్షిక(llcache_object *object,
		స్థిరరాశి uint8_t *దత్తాంశం, size_t len)
{
	nserror res;
	అక్షరం *name, *విలువ;
	llcache_శీర్షిక *తాత్కాలిక;

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
	ఒకవేళ (అక్షరశ్రేణి_పోలిక((స్థిరరాశి అక్షరం *) దత్తాంశం, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_సమయం = object->cache.req_సమయం;

		llcache_invalidate_cache_control_దత్తాంశం(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_సమయం = req_సమయం;

		llcache_destroy_శీర్షికలు(object);
	}

	/* Set fetch response time if not already set */
	ఒకవేళ (object->cache.res_సమయం == 0) {
		object->cache.res_సమయం = time(ఖాళీ);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_విభజించు_శీర్షిక(దత్తాంశం, len, &name, &విలువ);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	/* deal with empty header */
	ఒకవేళ (name[0] == 0) {
		విడుదల(name);
		విడుదల(విలువ);
		తిరిగి NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_శీర్షిక_cache_control(object, name, విలువ);
	ఒకవేళ (res != NSERROR_OK) {
		విడుదల(name);
		విడుదల(విలువ);
		తిరిగి res;
	}

	/* Append header data to the object's headers array */
	తాత్కాలిక = realloc(object->శీర్షికలు,
		       (object->అంకె_శీర్షికలు + 1) * పరిమాణం(llcache_శీర్షిక));
	ఒకవేళ (తాత్కాలిక == ఖాళీ) {
		విడుదల(name);
		విడుదల(విలువ);
		తిరిగి NSERROR_NOMEM;
	}

	object->శీర్షికలు = తాత్కాలిక;

	object->శీర్షికలు[object->అంకె_శీర్షికలు].name = name;
	object->శీర్షికలు[object->అంకె_శీర్షికలు].విలువ = విలువ;

	object->అంకె_శీర్షికలు++;

	తిరిగి NSERROR_OK;
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
స్థిర nserror get_referer_శీర్షిక(nsurl *url, nsurl *referer, అక్షరం **శీర్షిక_బహిర్గతం)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool సరిపోలిక;
	bool match1;
	bool match2;
	అక్షరం *శీర్షిక;

	/* Determine whether to send the Referer header */
	ఒకవేళ (!nsoption_bool(send_referer)) {
		తిరిగి NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (scheme == ఖాళీ) {
		తిరిగి NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	ఒకవేళ (ref_scheme == ఖాళీ) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		తిరిగి NSERROR_INVALID;
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
	ఒకవేళ (lwc_string_isequal(scheme, ref_scheme,
			       &సరిపోలిక) != lwc_error_ok) {
		సరిపోలిక = false;
	}
	ఒకవేళ (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	ఒకవేళ (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	ఒకవేళ (సరిపోలిక == true || (match1 == true && match2 == true)) {
		స్థిరరాశి size_t len = SLEN("Referer: ") +
			nsurl_నిడివి(referer) + 1;

		శీర్షిక = జ్ఞాపకం_ఇవ్వు(len);
		ఒకవేళ (శీర్షిక == ఖాళీ) {
			res = NSERROR_NOMEM;
		} లేకపోతే {
			snprintf(శీర్షిక, len, "Referer: %s",
				 nsurl_access(referer));

			*శీర్షిక_బహిర్గతం = శీర్షిక;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	తిరిగి res;
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
స్థిర nserror llcache_object_refetch(llcache_object *object)
{
	స్థిరరాశి అక్షరం *urlenc = ఖాళీ;
	నిర్మాణం fetch_multipart_దత్తాంశం *multipart = ఖాళీ;
	అక్షరం **శీర్షికలు = ఖాళీ;
	పూర్ణాంకం శీర్షిక_idx = 0;
	nserror res;

	ఒకవేళ (object->fetch.post != ఖాళీ) {
		ఒకవేళ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->దత్తాంశం.urlenc;
		} లేకపోతే {
			multipart = object->fetch.post->దత్తాంశం.multipart;
		}
	}

	/* Generate headers */
	శీర్షికలు = జ్ఞాపకం_ఇవ్వు(4 * పరిమాణం(అక్షరం *));
	ఒకవేళ (శీర్షికలు == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	ఒకవేళ (object->cache.etag != ఖాళీ) {
		స్థిరరాశి size_t len = SLEN("If-None-Match: ") +
				అక్షరశ్రేణి_పొడవు(object->cache.etag) + 1;

		శీర్షికలు[శీర్షిక_idx] = జ్ఞాపకం_ఇవ్వు(len);
		ఒకవేళ (శీర్షికలు[శీర్షిక_idx] == ఖాళీ) {
			విడుదల(శీర్షికలు);
			తిరిగి NSERROR_NOMEM;
		}

		snprintf(శీర్షికలు[శీర్షిక_idx], len, "If-None-Match: %s",
				object->cache.etag);

		శీర్షిక_idx++;
	}

	/* cache-control header for modification time */
	ఒకవేళ (object->cache.చివరి_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		స్థిరరాశి size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		శీర్షికలు[శీర్షిక_idx] = జ్ఞాపకం_ఇవ్వు(len);
		ఒకవేళ (శీర్షికలు[శీర్షిక_idx] == ఖాళీ) {
			అయితే (--శీర్షిక_idx >= 0)
				విడుదల(శీర్షికలు[శీర్షిక_idx]);
			విడుదల(శీర్షికలు);
			తిరిగి NSERROR_NOMEM;
		}

		snprintf(శీర్షికలు[శీర్షిక_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.చివరి_modified));

		శీర్షిక_idx++;
	}

	/* Referer header */
	ఒకవేళ (object->fetch.referer != ఖాళీ) {
		ఒకవేళ (get_referer_శీర్షిక(object->url,
				       object->fetch.referer,
				       &శీర్షికలు[శీర్షిక_idx]) == NSERROR_OK) {
			శీర్షిక_idx++;
		}
	}
	శీర్షికలు[శీర్షిక_idx] = ఖాళీ;

	/* Reset cache control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);
	object->cache.req_సమయం = time(ఖాళీ);
	object->cache.fin_సమయం = object->cache.req_సమయం;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_ప్రారంభం(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_పుటలు,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (స్థిరరాశి అక్షరం **)శీర్షికలు,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	అయితే (--శీర్షిక_idx >= 0) {
		విడుదల(శీర్షికలు[శీర్షిక_idx]);
	}
	విడుదల(శీర్షికలు);

	తిరిగి res;
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
స్థిర nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, స్థిరరాశి llcache_post_దత్తాంశం *post,
		uint32_t redirect_లెక్క, bool hsts_లోపల_use)
{
	nserror error;
	nsurl *referer_clone = ఖాళీ;
	llcache_post_దత్తాంశం *post_clone = ఖాళీ;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	ఒకవేళ (post != ఖాళీ) {
		error = llcache_post_దత్తాంశం_clone(post, &post_clone);
		ఒకవేళ (error != NSERROR_OK)
			తిరిగి error;
	}

	ఒకవేళ (referer != ఖాళీ)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_లెక్క = redirect_లెక్క;
	object->fetch.retries_మిగిలిన = llcache->fetch_attempts;
	object->fetch.hsts_లోపల_use = hsts_లోపల_use;

	తిరిగి llcache_object_refetch(object);
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
స్థిర nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	ఒకవేళ (object->ఆధారం_దత్తాంశం != ఖాళీ) {
		ఒకవేళ (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} లేకపోతే {
			విడుదల(object->ఆధారం_దత్తాంశం);
		}
	}

	nsurl_unref(object->url);

	ఒకవేళ (object->fetch.fetch != ఖాళీ) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = ఖాళీ;
	}

	ఒకవేళ (object->fetch.referer != ఖాళీ)
		nsurl_unref(object->fetch.referer);

	ఒకవేళ (object->fetch.post != ఖాళీ) {
		ఒకవేళ (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			విడుదల(object->fetch.post->దత్తాంశం.urlenc);
		} లేకపోతే {
			fetch_multipart_దత్తాంశం_destroy(
					object->fetch.post->దత్తాంశం.multipart);
		}

		విడుదల(object->fetch.post);
	}

	విడుదల(object->cache.etag);

	కొరకు (i = 0; i < object->అంకె_శీర్షికలు; i++) {
		విడుదల(object->శీర్షికలు[i].name);
		విడుదల(object->శీర్షికలు[i].విలువ);
	}
	విడుదల(object->శీర్షికలు);

	విడుదల(object);

	తిరిగి NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
స్థిర nserror llcache_object_కూడు_గమ్యం_list(llcache_object *object,
		llcache_object **list)
{
	object->prev = ఖాళీ;
	object->తదుపరి = *list;

	ఒకవేళ (*list != ఖాళీ)
		(*list)->prev = object;
	*list = object;

	తిరిగి NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
స్థిర పూర్ణాంకం
llcache_object_rfc2616_మిగిలిన_lifetime(స్థిరరాశి llcache_cache_control *cd)
{
	పూర్ణాంకం current_age, freshness_lifetime;
	time_t now = time(ఖాళీ);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_సమయం - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_సమయం - cd->req_సమయం + now - cd->res_సమయం;

	/* Determine freshness lifetime of this object */
	ఒకవేళ (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} లేకపోతే ఒకవేళ (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} లేకపోతే ఒకవేళ (cd->చివరి_modified != 0) {
		freshness_lifetime = (now - cd->చివరి_modified) / 10;
	} లేకపోతే {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	ఒకవేళ ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		తిరిగి freshness_lifetime - current_age;
	}

	తిరిగి 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
స్థిర bool llcache_object_is_fresh(స్థిరరాశి llcache_object *object)
{
	పూర్ణాంకం మిగిలిన_lifetime;
	స్థిరరాశి llcache_cache_control *cd = &object->cache;

	మిగిలిన_lifetime = llcache_object_rfc2616_మిగిలిన_lifetime(cd);

	NSLOG(llcache, DEBUG, "%p: (%d > 0 || %d != %d)", object,
	     మిగిలిన_lifetime,
	     object->fetch.state, LLCACHE_FETCH_COMPLETE);

	/* The object is fresh if:
	 * - it was not forbidden from being returned from the cache
	 *   unvalidated.
	 *
	 * - it has remaining lifetime or still being fetched.
	 */
	తిరిగి ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
		((మిగిలిన_lifetime > 0) ||
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
స్థిర nserror llcache_object_clone_cache_దత్తాంశం(llcache_object *ఆధారం,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	ఒకవేళ (ఆధారం->cache.etag != ఖాళీ) {
		అక్షరం *etag = ఆధారం->cache.etag;

		ఒకవేళ (deep) {
			/* Copy the etag */
			etag = strdup(ఆధారం->cache.etag);
			ఒకవేళ (etag == ఖాళీ)
				తిరిగి NSERROR_NOMEM;
		} లేకపోతే {
			/* Destination takes ownership */
			ఆధారం->cache.etag = ఖాళీ;
		}

		ఒకవేళ (destination->cache.etag != ఖాళీ)
			విడుదల(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_సమయం = ఆధారం->cache.req_సమయం;
	destination->cache.res_సమయం = ఆధారం->cache.res_సమయం;
	destination->cache.fin_సమయం = ఆధారం->cache.fin_సమయం;

	ఒకవేళ (ఆధారం->cache.date != 0)
		destination->cache.date = ఆధారం->cache.date;

	ఒకవేళ (ఆధారం->cache.expires != 0)
		destination->cache.expires = ఆధారం->cache.expires;

	ఒకవేళ (ఆధారం->cache.age != INVALID_AGE)
		destination->cache.age = ఆధారం->cache.age;

	ఒకవేళ (ఆధారం->cache.max_age != INVALID_AGE)
		destination->cache.max_age = ఆధారం->cache.max_age;

	ఒకవేళ (ఆధారం->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = ఆధారం->cache.no_cache;

	ఒకవేళ (ఆధారం->cache.చివరి_modified != 0)
		destination->cache.చివరి_modified = ఆధారం->cache.చివరి_modified;

	తిరిగి NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
స్థిర nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	ఒకవేళ (object == *list)
		*list = object->తదుపరి;
	లేకపోతే
		object->prev->తదుపరి = object->తదుపరి;

	ఒకవేళ (object->తదుపరి != ఖాళీ)
		object->తదుపరి->prev = object->prev;

	తిరిగి NSERROR_OK;
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
స్థిర nserror llcache_retrieve_persisted_దత్తాంశం(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	ఒకవేళ ((object->ఆధారం_దత్తాంశం != ఖాళీ) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		తిరిగి NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	తిరిగి guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->ఆధారం_దత్తాంశం,
				    &object->ఆధారం_len);
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
స్థిర nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **దత్తాంశం_బహిర్గతం,
			   size_t *datasize_బహిర్గతం)
{
	size_t allocsize;
	పూర్ణాంకం datasize;
	uint8_t *దత్తాంశం;
	అక్షరం *op;
	గుర్తులేని పూర్ణాంకం hloop;
	పూర్ణాంకం use;
	size_t cert_chain_లోతు;

	ఒకవేళ (object->chain != ఖాళీ) {
		cert_chain_లోతు = object->chain->లోతు;
	} లేకపోతే {
		cert_chain_లోతు = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	కొరకు (hloop = 0 ; hloop < object->అంకె_శీర్షికలు ; hloop++) {
		allocsize += అక్షరశ్రేణి_పొడవు(object->శీర్షికలు[hloop].name) + 1;
		allocsize += అక్షరశ్రేణి_పొడవు(object->శీర్షికలు[hloop].విలువ) + 1;
	}

	allocsize += nsurl_నిడివి(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	కొరకు (hloop = 0; hloop < cert_chain_లోతు; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_నిడివి + 2) / 3);
	}

	దత్తాంశం = జ్ఞాపకం_ఇవ్వు(allocsize);
	ఒకవేళ (దత్తాంశం == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	op = (అక్షరం *)దత్తాంశం;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	ఒకవేళ (use < 0) {
		వెళ్ళు operror;
	}
	use++; /* does not count the null */
	ఒకవేళ (use > datasize) {
		వెళ్ళు అధికప్రవాహం;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->ఆధారం_len);
	ఒకవేళ (use < 0) {
		వెళ్ళు operror;
	}
	use++; /* does not count the null */
	ఒకవేళ (use > datasize)
		వెళ్ళు అధికప్రవాహం;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_సమయం);
	ఒకవేళ (use == 0)
		వెళ్ళు అధికప్రవాహం;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_సమయం);
	ఒకవేళ (use == 0)
		వెళ్ళు అధికప్రవాహం;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_సమయం);
	ఒకవేళ (use == 0)
		వెళ్ళు అధికప్రవాహం;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->అంకె_శీర్షికలు);
	ఒకవేళ (use < 0) {
		వెళ్ళు operror;
	}
	use++; /* does not count the null */
	ఒకవేళ (use > datasize)
		వెళ్ళు అధికప్రవాహం;
	op += use;
	datasize -= use;

	/*శీర్షికలు*/
	కొరకు (hloop = 0 ; hloop < object->అంకె_శీర్షికలు ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->శీర్షికలు[hloop].name,
			       object->శీర్షికలు[hloop].విలువ);
		ఒకవేళ (use < 0) {
			వెళ్ళు operror;
		}
		use++; /* does not count the null */
		ఒకవేళ (use > datasize)
			వెళ్ళు అధికప్రవాహం;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_లోతు);
	ఒకవేళ (use < 0) {
		వెళ్ళు operror;
	}
	use++; /* does not count the null */
	ఒకవేళ (use > datasize)
		వెళ్ళు అధికప్రవాహం;
	op += use;
	datasize -= use;

	/* SSL certificates */
	కొరకు (hloop = 0; hloop < cert_chain_లోతు; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (పూర్ణాంకం)(object->chain->certs[hloop].err));
		ఒకవేళ (use < 0) {
			వెళ్ళు operror;
		}
		use++; /* does not count the null */
		ఒకవేళ (use > datasize)
			వెళ్ళు అధికప్రవాహం;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		ఒకవేళ (object->chain->certs[hloop].der != ఖాళీ) {
			size_t output_నిడివి = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_నిడివి,
				(uint8_t *)op,
				&output_నిడివి);
			ఒకవేళ (res != NSUERROR_OK) {
				వెళ్ళు operror;
			}
			use = output_నిడివి;
		} లేకపోతే {
			use = 0;
		}
		use++; /* allow for null */
		ఒకవేళ (use > datasize)
			వెళ్ళు అధికప్రవాహం;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*దత్తాంశం_బహిర్గతం = దత్తాంశం;
	*datasize_బహిర్గతం = allocsize - datasize;

	తిరిగి NSERROR_OK;

అధికప్రవాహం:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	విడుదల(దత్తాంశం);
	తిరిగి NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	విడుదల(దత్తాంశం);
	తిరిగి NSERROR_INVALID;
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
స్థిర nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = ఖాళీ;
	size_t metadatalen = 0;
	size_t మిగిలిన = 0;
	nsurl *metadataurl;
	గుర్తులేని పూర్ణాంకం పంక్తి;
	అక్షరం *ln;
	పూర్ణాంకం lnsize;

	size_t ఆధారం_నిడివి;
	time_t request_సమయం;
	time_t response_సమయం;
	time_t completion_సమయం;
	size_t అంకె_శీర్షికలు;
	size_t hloop;
	size_t ssl_cert_లెక్క = 0;
	నిర్మాణం cert_chain *chain = ఖాళీ;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	NSLOG(llcache, INFO, "Processing retrieved data");

	/* metadata is stored as a sequence of NULL terminated strings
	 * which we call 'line's here.
	 */

	/* We track remaining data because as we extend this data structure
	 * we need to know if we should continue to parse
	 */
	మిగిలిన = metadatalen;

	/* metadata line 1 is the url the metadata referrs to */
	పంక్తి = 1;
	ln = (అక్షరం *)metadata;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	ఒకవేళ (lnsize < 7) {
		res = NSERROR_INVALID;
		వెళ్ళు format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	ఒకవేళ (res != NSERROR_OK)
		వెళ్ళు format_error;

	ఒకవేళ (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		తిరిగి NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	పంక్తి = 2;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	ఒకవేళ ((lnsize < 1) || (అక్షరశ్రేణి_చదువు(ln, "%" PRIsizet, &ఆధారం_నిడివి) != 1)) {
		res = NSERROR_INVALID;
		వెళ్ళు format_error;
	}


	/* metadata line 3 is the time of request */
	పంక్తి = 3;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_సమయం);
	ఒకవేళ (res != NSERROR_OK)
		వెళ్ళు format_error;


	/* metadata line 4 is the time of response */
	పంక్తి = 4;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_సమయం);
	ఒకవేళ (res != NSERROR_OK)
		వెళ్ళు format_error;


	/* metadata line 5 is the time of request completion */
	పంక్తి = 5;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_సమయం);
	ఒకవేళ (res != NSERROR_OK)
		వెళ్ళు format_error;


	/* metadata line 6 is the number of headers */
	పంక్తి = 6;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	ఒకవేళ ((lnsize < 1) || (అక్షరశ్రేణి_చదువు(ln, "%" PRIsizet, &అంకె_శీర్షికలు) != 1)) {
		res = NSERROR_INVALID;
		వెళ్ళు format_error;
	}

	/* read headers */
	కొరకు (hloop = 0 ; hloop < అంకె_శీర్షికలు; hloop++) {
		పంక్తి++;
		ln += lnsize + 1;
		lnsize = అక్షరశ్రేణి_పొడవు(ln);
		మిగిలిన -= lnsize + 1;

		res = llcache_fetch_process_శీర్షిక(object,
						   (uint8_t *)ln,
						   lnsize);
		ఒకవేళ (res != NSERROR_OK)
			వెళ్ళు format_error;
	}

	ఒకవేళ (మిగిలిన == 0) {
		వెళ్ళు skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	పంక్తి++;
	ln += lnsize + 1;
	lnsize = అక్షరశ్రేణి_పొడవు(ln);
	మిగిలిన -= lnsize + 1;

	ఒకవేళ ((lnsize < 1) || (అక్షరశ్రేణి_చదువు(ln, "%" PRIsizet, &ssl_cert_లెక్క) != 1)) {
		res = NSERROR_INVALID;
		వెళ్ళు format_error;
	}

	ఒకవేళ (ssl_cert_లెక్క == 0) {
		వెళ్ళు skip_ssl_certificates;
	}

	ఒకవేళ (ssl_cert_లెక్క > MAX_CERT_లోతు) {
		res = NSERROR_INVALID;
		వెళ్ళు format_error;
	}

	res = cert_chain_alloc(ssl_cert_లెక్క, &chain);
	ఒకవేళ (res != NSERROR_OK) {
		వెళ్ళు format_error;
	}

	కొరకు (hloop = 0; hloop < ssl_cert_లెక్క; hloop++) {
		పూర్ణాంకం errcode;
		nsuerror nsures;

		/* Certificate error code */
		పంక్తి++;
		ln += lnsize + 1;
		lnsize = అక్షరశ్రేణి_పొడవు(ln);
		మిగిలిన -= lnsize + 1;
		ఒకవేళ ((lnsize < 1) || (అక్షరశ్రేణి_చదువు(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			వెళ్ళు format_error;
		}
		ఒకవేళ (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_తెలిసిన) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} లేకపోతే {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		పంక్తి++;
		ln += lnsize + 1;
		lnsize = అక్షరశ్రేణి_పొడవు(ln);
		మిగిలిన -= lnsize + 1;
		ఒకవేళ (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((స్థిరరాశి uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_నిడివి);
			ఒకవేళ (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				వెళ్ళు format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->ఆధారం_len = ఆధారం_నిడివి;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->ఆధారం_alloc = metadatalen;

	object->cache.req_సమయం = request_సమయం;
	object->cache.res_సమయం = response_సమయం;
	object->cache.fin_సమయం = completion_సమయం;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	తిరిగి NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      పంక్తి, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	తిరిగి res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
స్థిర అంతఃవరుస bool llcache__scheme_is_persistable(స్థిరరాశి nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool సరిపోలిక;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ఒకవేళ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	తిరిగి persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
స్థిర అంతఃవరుస bool llcache__scheme_is_cachable(స్థిరరాశి nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool సరిపోలిక;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	ఒకవేళ ((lwc_string_isequal(scheme, corestring_lwc_http,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_దత్తాంశం,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&సరిపోలిక) == lwc_error_ok &&
			(సరిపోలిక == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	తిరిగి cachable;
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
స్థిర nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				స్థిరరాశి llcache_post_దత్తాంశం *post,
				uint32_t redirect_లెక్క)
{
	nserror error;
	nsurl *referer_clone = ఖాళీ;
	llcache_post_దత్తాంశం *post_clone = ఖాళీ;

	ఒకవేళ (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	object->cache.req_సమయం = time(ఖాళీ);
	object->cache.fin_సమయం = object->cache.req_సమయం;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	ఒకవేళ (error != NSERROR_OK) {
		తిరిగి error;
	}

	/* entry came out of cache - need to setup object state */
	ఒకవేళ (post != ఖాళీ) {
		error = llcache_post_దత్తాంశం_clone(post, &post_clone);
		ఒకవేళ (error != NSERROR_OK)
			తిరిగి error;
	}

	ఒకవేళ (referer != ఖాళీ) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_లెక్క = redirect_లెక్క;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ఖాళీ;

	తిరిగి NSERROR_OK;
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
స్థిర nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   స్థిరరాశి llcache_post_దత్తాంశం *post,
				   uint32_t redirect_లెక్క,
				   bool hsts_లోపల_use,
				   llcache_object **ఫలితం)
{
	nserror error;
	llcache_object *obj, *newest = ఖాళీ;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==ఖాళీ?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	కొరకు (obj = llcache->cached_objects; obj != ఖాళీ; obj = obj->తదుపరి) {

		ఒకవేళ ((newest == ఖాళీ ||
		     obj->cache.req_సమయం > newest->cache.req_సమయం) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	ఒకవేళ (newest == ఖాళీ) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		ఒకవేళ (error != NSERROR_OK)
			తిరిగి error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_లెక్క);
		ఒకవేళ (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_కూడు_గమ్యం_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	ఒకవేళ ((newest != ఖాళీ) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_దత్తాంశం(newest);
		ఒకవేళ (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*ఫలితం = newest;

			తిరిగి NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		ఒకవేళ (error != NSERROR_OK) {
			తిరిగి error;
		}
	} లేకపోతే ఒకవేళ (newest != ఖాళీ) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_దత్తాంశం(newest);
		ఒకవేళ (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			ఒకవేళ (error != NSERROR_OK)
				తిరిగి error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_దత్తాంశం(newest, obj, true);
			ఒకవేళ (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				తిరిగి error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_లెక్క++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_లెక్క, hsts_లోపల_use);
			ఒకవేళ (error != NSERROR_OK) {
				newest->candidate_లెక్క--;
				llcache_object_destroy(obj);
				తిరిగి error;
			}

			/* Add new object to cache */
			llcache_object_కూడు_గమ్యం_list(obj, &llcache->cached_objects);

			*ఫలితం = obj;

			తిరిగి NSERROR_OK;
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
		ఒకవేళ (error != NSERROR_OK) {
			తిరిగి error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_లెక్క, hsts_లోపల_use);
	ఒకవేళ (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		తిరిగి error;
	}

	/* Add new object to cache */
	llcache_object_కూడు_గమ్యం_list(obj, &llcache->cached_objects);

	*ఫలితం = obj;

	తిరిగి NSERROR_OK;
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
స్థిర nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			స్థిరరాశి llcache_post_దత్తాంశం *post,
			uint32_t redirect_లెక్క,
			bool hsts_లోపల_use,
			llcache_object **ఫలితం)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==ఖాళీ?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	/* determine if content is cachable */
	ఒకవేళ ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} లేకపోతే ఒకవేళ (post != ఖాళీ) {
		/* POST requests are never cached */
		uncachable = true;
	} లేకపోతే {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	ఒకవేళ (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		ఒకవేళ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			తిరిగి error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_లెక్క, hsts_లోపల_use);
		ఒకవేళ (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			తిరిగి error;
		}

		/* Add new object to uncached list */
		llcache_object_కూడు_గమ్యం_list(obj, &llcache->uncached_objects);
	} లేకపోతే {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_లెక్క,
				hsts_లోపల_use, &obj);
		ఒకవేళ (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			తిరిగి error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*ఫలితం = obj;

	nsurl_unref(defragmented_url);

	తిరిగి NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
స్థిర nserror llcache_object_కూడు_user(llcache_object *object,
		llcache_object_user *user)
{
	ప్రతిజ్ఞా(user->తదుపరి == ఖాళీ);
	ప్రతిజ్ఞా(user->prev == ఖాళీ);
	ప్రతిజ్ఞా(user->handle != ఖాళీ);

	user->handle->object = object;

	user->prev = ఖాళీ;
	user->తదుపరి = object->users;

	ఒకవేళ (object->users != ఖాళీ)
		object->users->prev = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	తిరిగి NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_hsts_transform_url(nsurl *url, nsurl **ఫలితం,
		bool *hsts_లోపల_use)
{
	lwc_string *scheme = ఖాళీ;
	bool సరిపోలిక;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	ఒకవేళ (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&సరిపోలిక) != lwc_error_ok || సరిపోలిక == false) {
		/* Non-HTTP fetch: no transform required */
		ఒకవేళ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&సరిపోలిక) == lwc_error_ok && సరిపోలిక) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_లోపల_use = urldb_get_hsts_enabled(url);
		} లేకపోతే {
			/* Anything else: no HSTS */
			*hsts_లోపల_use = false;
		}
		lwc_string_unref(scheme);
		*ఫలితం = nsurl_ref(url);
		తిరిగి error;
	}
	lwc_string_unref(scheme);

	ఒకవేళ (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				ఫలితం);
		*hsts_లోపల_use = (error == NSERROR_OK);
	} లేకపోతే {
		*ఫలితం = nsurl_ref(url);
		*hsts_లోపల_use = false;
	}

	తిరిగి error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = ఖాళీ;
	bool సరిపోలిక = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	ఒకవేళ (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&సరిపోలిక) != lwc_error_ok || సరిపోలిక == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		తిరిగి NSERROR_OK;
	}
	lwc_string_unref(scheme);

	ఒకవేళ (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		తిరిగి NSERROR_OK;
	}

	కొరకు (i = 0; i < object->అంకె_శీర్షికలు; i++) {
		ఒకవేళ (strcasecmp("Strict-Transport-Security",
				object->శీర్షికలు[i].name) == 0) {
			urldb_సమితి_hsts_policy(object->url,
					object->శీర్షికలు[i].విలువ);
			/* Only process the first one we find */
			ఆపు;
		}
	}

	తిరిగి NSERROR_OK;
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
స్థిర nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool సరిపోలిక;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	ఒకవేళ ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&సరిపోలిక) == lwc_error_ok && సరిపోలిక == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&సరిపోలిక) == lwc_error_ok && సరిపోలిక == false)) {
		/* file, about and resource are not valid redirect targets */
		ఒకవేళ ((lwc_string_isequal(scheme, corestring_lwc_file,
				&సరిపోలిక) == lwc_error_ok && సరిపోలిక == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&సరిపోలిక) == lwc_error_ok && సరిపోలిక == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&సరిపోలిక) == lwc_error_ok && సరిపోలిక == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			తిరిగి NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	ఒకవేళ (fetch_can_fetch(dst_url) == false) {
		తిరిగి NSERROR_UNSAFE_REDIRECT;
	}

	తిరిగి NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_fetch_redirect(llcache_object *object,
		స్థిరరాశి అక్షరం *లక్ష్యం, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *తదుపరి;
	స్థిరరాశి llcache_post_దత్తాంశం *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_లోపల_use;
	http_response_సంకేతం http_సంకేతం;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_సంకేతం = fetch_http_సంకేతం(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ఖాళీ;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(శూన్య) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	ఒకవేళ (object->fetch.redirect_లెక్క > nsoption_uint(fetch_redirect_హద్దు)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		తిరిగి NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, లక్ష్యం, &url);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_లోపల_use);
	nsurl_unref(url);
	ఒకవేళ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		తిరిగి res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	ఒకవేళ (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		తిరిగి res;
	}

	/* alter behaviour based on specific redirect code */
	ఎంపిక (http_సంకేతం) {
	స్థితి HTTP_RESPONSE_MOVED_PERMANENTLY:
	స్థితి HTTP_RESPONSE_దొరికింది:
	స్థితి HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = ఖాళీ;
		ఆపు;

	స్థితి HTTP_RESPONSE_TEMPORARY_REDIRECT:
	స్థితి HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		ఆపు;

	అప్రమేయ:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_సంకేతం);
		nsurl_unref(hsts_url);
		తిరిగి NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.దత్తాంశం.redirect.from = object->url;
	event.దత్తాంశం.redirect.గమ్యం = hsts_url;

	res = llcache_send_event_గమ్యం_users(object, &event);
	ఒకవేళ (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		తిరిగి res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_లెక్క + 1,
					hsts_లోపల_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	ఒకవేళ (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		కొరకు (user = object->users; user != ఖాళీ; user = తదుపరి) {
			తదుపరి = user->తదుపరి;

			llcache_object_remove_user(object, user);
			llcache_object_కూడు_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	తిరిగి res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
స్థిర nserror llcache_object_cache_update(llcache_object *object)
{
	ఒకవేళ (object->cache.date == 0)
		object->cache.date = time(ఖాళీ);

	తిరిగి NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
స్థిర nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	ఒకవేళ (object->candidate != ఖాళీ) {
		llcache_object_user *user, *తదుపరి;

		/* Move user(s) to candidate content */
		కొరకు (user = object->users; user != ఖాళీ; user = తదుపరి) {
			తదుపరి = user->తదుపరి;

			llcache_object_remove_user(object, user);
			llcache_object_కూడు_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_లెక్క--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_దత్తాంశం(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		ఒకవేళ (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = ఖాళీ;
	} లేకపోతే {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ఖాళీ;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(శూన్య) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	తిరిగి NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
స్థిర nserror
llcache_fetch_process_దత్తాంశం(llcache_object *object,
			   స్థిరరాశి uint8_t *దత్తాంశం,
			   size_t len)
{
	ఒకవేళ (object->fetch.state != LLCACHE_FETCH_దత్తాంశం) {
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
		దీర్ఘ http_సంకేతం = fetch_http_సంకేతం(object->fetch.fetch);

		ఒకవేళ ((http_సంకేతం != 200 && http_సంకేతం != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_దత్తాంశం(object);
		}

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		object->fetch.state = LLCACHE_FETCH_దత్తాంశం;
	}

	/* Resize source buffer if it's too small */
	ఒకవేళ (object->ఆధారం_len + len >= object->ఆధారం_alloc) {
		స్థిరరాశి size_t new_len = object->ఆధారం_len + len + 64 * 1024;
		uint8_t *తాత్కాలిక = realloc(object->ఆధారం_దత్తాంశం, new_len);
		ఒకవేళ (తాత్కాలిక == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		object->ఆధారం_దత్తాంశం = తాత్కాలిక;
		object->ఆధారం_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	జ్ఞాపకం_నకలు(object->ఆధారం_దత్తాంశం + object->ఆధారం_len, దత్తాంశం, len);
	object->ఆధారం_len += len;

	తిరిగి NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
స్థిర nserror llcache_fetch_auth(llcache_object *object, స్థిరరాశి అక్షరం *realm)
{
	స్థిరరాశి అక్షరం *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = ఖాళీ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	/* Destroy headers */
	llcache_destroy_శీర్షికలు(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	ఒకవేళ (realm == ఖాళీ)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	ఒకవేళ (auth == ఖాళీ || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.దత్తాంశం.error.సంకేతం_2 = NSERROR_BAD_AUTH;
		event.దత్తాంశం.error.సందేశం = realm;

		error = llcache_send_event_గమ్యం_users(object, &event);
	} లేకపోతే {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	తిరిగి error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ఖాళీ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	ఒకవేళ (object->fetch.hsts_లోపల_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.దత్తాంశం.error.సంకేతం_2 = NSERROR_BAD_CERTS;
		event.దత్తాంశం.error.సందేశం = messages_get("FetchFailed");

		error = llcache_send_event_గమ్యం_users(object, &event);
	} లేకపోతే {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.దత్తాంశం.error.సంకేతం_2 = NSERROR_UNKNOWN;
		event.దత్తాంశం.error.సందేశం = messages_get("FetchFailed");

		error = llcache_send_event_గమ్యం_users(object, &event);
	}

	తిరిగి error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = ఖాళీ;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	ఒకవేళ (object->fetch.hsts_లోపల_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.దత్తాంశం.error.సంకేతం_2 = NSERROR_UNKNOWN;
		event.దత్తాంశం.error.సందేశం = messages_get("FetchFailed");

		error = llcache_send_event_గమ్యం_users(object, &event);
	} లేకపోతే {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	తిరిగి error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
స్థిర nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	ఒకవేళ (object->fetch.retries_మిగిలిన > 1) {
		object->fetch.retries_మిగిలిన--;
		తిరిగి llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = ఖాళీ;

	/* Release candidate, if any */
	ఒకవేళ (object->candidate != ఖాళీ) {
		object->candidate->candidate_లెక్క--;
		object->candidate = ఖాళీ;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_దత్తాంశం(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.దత్తాంశం.error.సంకేతం_2 = NSERROR_TIMEOUT;
	event.దత్తాంశం.error.సందేశం = ఖాళీ;

	తిరిగి llcache_send_event_గమ్యం_users(object, &event);
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
స్థిర nserror
build_candidate_list(నిర్మాణం llcache_object ***lst_బహిర్గతం, పూర్ణాంకం *lst_len_బహిర్గతం)
{
	llcache_object *object, *తదుపరి;
	నిర్మాణం llcache_object **lst;
	పూర్ణాంకం lst_len = 0;
	పూర్ణాంకం మిగిలిన_lifetime;

#నిర్వచించు MAX_PERSIST_చొప్పున_నిరంతరం 128

	lst = సున్నా_జ్ఞాపకం_ఇవ్వు(MAX_PERSIST_చొప్పున_నిరంతరం, పరిమాణం(నిర్మాణం llcache_object *));
	ఒకవేళ (lst == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	కొరకు (object = llcache->cached_objects; object != ఖాళీ; object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		/* Only consider http(s) for the disc cache. */
		ఒకవేళ (!llcache__scheme_is_persistable(object->url)) {
			కొనసాగు;
		}

		మిగిలిన_lifetime = llcache_object_rfc2616_మిగిలిన_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		ఒకవేళ ((object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (మిగిలిన_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			ఒకవేళ (lst_len == MAX_PERSIST_చొప్పున_నిరంతరం)
				ఆపు;
		}
	}

	ఒకవేళ (lst_len == 0) {
		విడుదల(lst);
		తిరిగి NSERROR_NOT_దొరికింది;
	}

	/** \todo sort list here */

	*lst_len_బహిర్గతం = lst_len;
	*lst_బహిర్గతం = lst;

#నిర్వచనం_రద్దు MAX_PERSIST_చొప్పున_నిరంతరం

	తిరిగి NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
స్థిర nserror
write_backing_store(నిర్మాణం llcache_object *object,
		    size_t *written_బహిర్గతం,
		    గుర్తులేని దీర్ఘ *elapsed)
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
				   object->ఆధారం_దత్తాంశం,
				   object->ఆధారం_len);
	ఒకవేళ (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		తిరిగి ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	ఒకవేళ (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		తిరిగి ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	ఒకవేళ (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		తిరిగి ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_బహిర్గతం = object->ఆధారం_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	ఒకవేళ (*elapsed == 0) {
		*elapsed = 1;
	}

	తిరిగి NSERROR_OK;
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
స్థిర శూన్య llcache_persist_slowcheck(శూన్య *p)
{
	uint64_t మొత్తం_bandwidth; /* total bandwidth */

	ఒకవేళ (llcache->మొత్తం_written > (2 * llcache->minimum_bandwidth)) {

		మొత్తం_bandwidth = (llcache->మొత్తం_written * 1000) / llcache->మొత్తం_elapsed;

		ఒకవేళ (మొత్తం_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      మొత్తం_bandwidth,
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
స్థిర శూన్య llcache_persist(శూన్య *p)
{
	nserror ret;
	నిర్మాణం llcache_object **lst; /* candidate object list */
	పూర్ణాంకం lst_లెక్క; /* number of candidates in list */
	పూర్ణాంకం idx; /* current candidate object index in list */
	పూర్ణాంకం తదుపరి = -1; /* when the next run should be scheduled for */

	గుర్తులేని దీర్ఘ write_హద్దు; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	గుర్తులేని దీర్ఘ elapsed; /* how long writing an object took */

	size_t మొత్తం_written = 0; /* total bytes written in this run */
	గుర్తులేని దీర్ఘ మొత్తం_elapsed = 1; /* total ms used to write bytes */
	గుర్తులేని దీర్ఘ మొత్తం_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_లెక్క);
	ఒకవేళ (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		తిరిగి;
	}

	write_హద్దు = (llcache->గరిష్ఠ_bandwidth * llcache->సమయం_కాలాంశం) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	కొరకు (idx = 0; idx < lst_లెక్క; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		ఒకవేళ (ret != NSERROR_OK) {
			కొనసాగు;
		}

		/* successfully wrote object to backing store */
		మొత్తం_written += written;
		మొత్తం_elapsed += elapsed;
		మొత్తం_bandwidth = (మొత్తం_written * 1000) / మొత్తం_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		ఒకవేళ (మొత్తం_elapsed > llcache->సమయం_కాలాంశం) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			ఒకవేళ (మొత్తం_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->కాలపట్టిక(
						llcache->సమయం_కాలాంశం * 100,
						llcache_persist_slowcheck,
						ఖాళీ);
				ఆపు;
			} లేకపోతే {
				ఒకవేళ (మొత్తం_bandwidth > llcache->గరిష్ఠ_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					తదుపరి = ((మొత్తం_written * llcache->సమయం_కాలాంశం) / write_హద్దు) - మొత్తం_elapsed;
				} లేకపోతే {
					తదుపరి = llcache->సమయం_కాలాంశం;
				}
				ఆపు;
			}
		} లేకపోతే ఒకవేళ (మొత్తం_written > write_హద్దు) {
			/* The bandwidth limit has been reached. */

			ఒకవేళ (మొత్తం_bandwidth > llcache->గరిష్ఠ_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				తదుపరి = ((మొత్తం_written * llcache->సమయం_కాలాంశం) / write_హద్దు) - మొత్తం_elapsed;
			} లేకపోతే {
				తదుపరి = llcache->సమయం_కాలాంశం - మొత్తం_elapsed;
			}
			ఆపు;
		}

	}
	విడుదల(lst);

	/* Completed list without running out of allowed bytes or time */
	ఒకవేళ (idx == lst_లెక్క) {
		/* only reschedule if writing is making any progress at all */
		ఒకవేళ (మొత్తం_written > 0) {
			తదుపరి = llcache->సమయం_కాలాంశం - మొత్తం_elapsed;
		} లేకపోతే {
			తదుపరి = -1;
		}
	}

	llcache->మొత్తం_written += మొత్తం_written;
	llcache->మొత్తం_elapsed += మొత్తం_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      మొత్తం_written, మొత్తం_elapsed, మొత్తం_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", తదుపరి);
	guit->misc->కాలపట్టిక(తదుపరి, llcache_persist, ఖాళీ);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
స్థిర శూన్య llcache_fetch_callback(స్థిరరాశి fetch_సందేశం *సందేశం, శూన్య *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	ఒకవేళ (llcache == ఖాళీ) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		ప్రతిజ్ఞా(false);
		/* In case assertions are off, return here */
		తిరిగి;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", సందేశం->type, object);

	ఎంపిక (సందేశం->type) {
	స్థితి FETCH_శీర్షిక:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_శీర్షికలు;

		error = llcache_fetch_process_శీర్షిక(object,
				సందేశం->దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf,
				సందేశం->దత్తాంశం.శీర్షిక_or_దత్తాంశం.len);
		ఆపు;

	/* 3xx responses */
	స్థితి FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		error = llcache_fetch_redirect(object,
				సందేశం->దత్తాంశం.redirect, &object);
		ఒకవేళ (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.దత్తాంశం.error.సంకేతం_2 = error;
			event.దత్తాంశం.error.సందేశం = ఖాళీ;
			error = llcache_send_event_గమ్యం_users(object, &event);
		}
		ఆపు;

	స్థితి FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		ఆపు;

	/* Normal 2xx state machine */
	స్థితి FETCH_దత్తాంశం:
		/* Received some data */
		error = llcache_fetch_process_దత్తాంశం(object,
				సందేశం->దత్తాంశం.శీర్షిక_or_దత్తాంశం.buf,
				సందేశం->దత్తాంశం.శీర్షిక_or_దత్తాంశం.len);
		ఆపు;

	స్థితి FETCH_ముగిసిన:
		/* Finished fetching */
	{
		uint8_t *తాత్కాలిక;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ఖాళీ;

		/* Shrink source buffer to required size */
		తాత్కాలిక = realloc(object->ఆధారం_దత్తాంశం,
				object->ఆధారం_len);
		/* If source_len is 0, then temp may be NULL */
		ఒకవేళ (తాత్కాలిక != ఖాళీ || object->ఆధారం_len == 0) {
			object->ఆధారం_దత్తాంశం = తాత్కాలిక;
			object->ఆధారం_alloc = object->ఆధారం_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_సమయం = time(ఖాళీ);

		(శూన్య) llcache_hsts_update_policy(object);

		guit->misc->కాలపట్టిక(5000, llcache_persist, ఖాళీ);
	}
		ఆపు;

	/* Out-of-band information */
	స్థితి FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		ఆపు;

	స్థితి FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = ఖాళీ;

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_దత్తాంశం(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.దత్తాంశం.error.సంకేతం_2 = NSERROR_UNKNOWN;
		event.దత్తాంశం.error.సందేశం = సందేశం->దత్తాంశం.error;

		error = llcache_send_event_గమ్యం_users(object, &event);

		ఆపు;

	స్థితి FETCH_పురోగతి:
		/* Progress update */
		event.type = LLCACHE_EVENT_పురోగతి;
		event.దత్తాంశం.పురోగతి_సందేశం = సందేశం->దత్తాంశం.పురోగతి;

		error = llcache_send_event_గమ్యం_users(object, &event);

		ఆపు;

	స్థితి FETCH_CERTS:
		/* Certificate information from the fetch */

		ఒకవేళ (object->chain != ఖాళీ) {
			cert_chain_free(object->chain);
			object->chain = ఖాళీ;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(సందేశం->దత్తాంశం.chain, &object->chain);
		ఒకవేళ (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.దత్తాంశం.chain = సందేశం->దత్తాంశం.chain;

		error = llcache_send_event_గమ్యం_users(object, &event);

		ఆపు;

	/* Events requiring action */
	స్థితి FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		error = llcache_fetch_auth(object, సందేశం->దత్తాంశం.auth.realm);
		ఆపు;

	స్థితి FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		error = llcache_fetch_cert_error(object);
		ఆపు;

	స్థితి FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		ఒకవేళ (object->candidate != ఖాళీ) {
			object->candidate->candidate_లెక్క--;
			object->candidate = ఖాళీ;
		}

		error = llcache_fetch_ssl_error(object);
		ఆపు;
	}

	/* Deal with any errors reported by event handlers */
	ఒకవేళ (error != NSERROR_OK) {
		ఒకవేళ (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		ఒకవేళ (object->fetch.fetch != ఖాళీ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ఖాళీ;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_దత్తాంశం(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_పైకి();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
స్థిర llcache_object_user *llcache_object_కనుగొను_user(
		స్థిరరాశి llcache_handle *handle)
{
	llcache_object_user *user;

	ప్రతిజ్ఞా(handle->object != ఖాళీ);

	కొరకు (user = handle->object->users; user != ఖాళీ; user = user->తదుపరి) {
		ఒకవేళ (user->handle == handle)
			ఆపు;
	}

	తిరిగి user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
స్థిర bool llcache_object_లోపల_list(స్థిరరాశి llcache_object *object,
		స్థిరరాశి llcache_object *list)
{
	అయితే (list != ఖాళీ) {
		ఒకవేళ (list == object)
			ఆపు;

		list = list->తదుపరి;
	}

	తిరిగి list != ఖాళీ;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *తదుపరి_user;
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

	కొరకు (user = object->users; user != ఖాళీ; user = తదుపరి_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		స్థిరరాశి llcache_fetch_state objstate = object->fetch.state;

		/* Flag that this user is the current iteration target
		 * in case the client attempts to destroy it underneath us */
		user->iterator_లక్ష్యం = true;

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

		ఒకవేళ (handle->state != objstate) {
			ఒకవేళ (emitted_notify == false) {
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
		ఒకవేళ (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_శీర్షికలు;

			/* Emit any certificate data we hold */
			ఒకవేళ (object->chain != ఖాళీ) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.దత్తాంశం.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} లేకపోతే {
				error = NSERROR_OK;
			}

			ఒకవేళ (user->queued_for_delete) {
				తదుపరి_user = user->తదుపరి;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ఒకవేళ (error != NSERROR_OK)
					తిరిగి error;

				కొనసాగు;
			} లేకపోతే ఒకవేళ (error == NSERROR_NEED_దత్తాంశం) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_శీర్షికలు;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_లక్ష్యం = false;
				తదుపరి_user = user->తదుపరి;
				llcache_users_not_caught_పైకి();
				కొనసాగు;
			} లేకపోతే ఒకవేళ (error != NSERROR_OK) {
				user->iterator_లక్ష్యం = false;
				తిరిగి error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		ఒకవేళ (handle->state == LLCACHE_FETCH_శీర్షికలు &&
				objstate > LLCACHE_FETCH_శీర్షికలు) {
			handle->state = LLCACHE_FETCH_దత్తాంశం;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_శీర్షికలు;

			error = handle->cb(handle, &event, handle->pw);

			ఒకవేళ (user->queued_for_delete) {
				తదుపరి_user = user->తదుపరి;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ఒకవేళ (error != NSERROR_OK)
					తిరిగి error;

				కొనసాగు;
			} లేకపోతే ఒకవేళ (error == NSERROR_NEED_దత్తాంశం) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_శీర్షికలు;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_లక్ష్యం = false;
				తదుపరి_user = user->తదుపరి;
				llcache_users_not_caught_పైకి();
				కొనసాగు;
			} లేకపోతే ఒకవేళ (error != NSERROR_OK) {
				user->iterator_లక్ష్యం = false;
				తిరిగి error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		ఒకవేళ (handle->state == LLCACHE_FETCH_దత్తాంశం &&
				objstate >= LLCACHE_FETCH_దత్తాంశం &&
				object->ఆధారం_len > handle->బైట్లు) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_దత్తాంశం;
			event.దత్తాంశం.దత్తాంశం.buf =
					object->ఆధారం_దత్తాంశం + handle->బైట్లు;
			event.దత్తాంశం.దత్తాంశం.len =
					object->ఆధారం_len - handle->బైట్లు;

			/* Update record of last byte emitted */
			ఒకవేళ (object->fetch.flags &
					LLCACHE_RETRIEVE_ప్రవాహం_దత్తాంశం) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->బైట్లు = object->ఆధారం_len = 0;
			} లేకపోతే {
				orig_handle_read = handle->బైట్లు;
				handle->బైట్లు = object->ఆధారం_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			ఒకవేళ (user->queued_for_delete) {
				తదుపరి_user = user->తదుపరి;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ఒకవేళ (error != NSERROR_OK)
					తిరిగి error;

				కొనసాగు;
			} లేకపోతే ఒకవేళ (error == NSERROR_NEED_దత్తాంశం) {
				/* User requested replay */
				handle->బైట్లు = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_లక్ష్యం = false;
				తదుపరి_user = user->తదుపరి;
				llcache_users_not_caught_పైకి();
				కొనసాగు;
			} లేకపోతే ఒకవేళ (error != NSERROR_OK) {
				user->iterator_లక్ష్యం = false;
				తిరిగి error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		ఒకవేళ (handle->state == LLCACHE_FETCH_దత్తాంశం &&
				objstate > LLCACHE_FETCH_దత్తాంశం) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_పూర్తి;

			error = handle->cb(handle, &event, handle->pw);
			ఒకవేళ (user->queued_for_delete) {
				తదుపరి_user = user->తదుపరి;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				ఒకవేళ (error != NSERROR_OK)
					తిరిగి error;

				కొనసాగు;
			} లేకపోతే ఒకవేళ (error == NSERROR_NEED_దత్తాంశం) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_దత్తాంశం;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_లక్ష్యం = false;
				తదుపరి_user = user->తదుపరి;
				llcache_users_not_caught_పైకి();
				కొనసాగు;
			} లేకపోతే ఒకవేళ (error != NSERROR_OK) {
				user->iterator_లక్ష్యం = false;
				తిరిగి error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_లక్ష్యం = false;

		తదుపరి_user = user->తదుపరి;
	}

	తిరిగి NSERROR_OK;
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
స్థిర nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	newobj->ఆధారం_alloc = newobj->ఆధారం_len = object->ఆధారం_len;

	ఒకవేళ (object->ఆధారం_len > 0) {
		newobj->ఆధారం_దత్తాంశం = జ్ఞాపకం_ఇవ్వు(newobj->ఆధారం_alloc);
		ఒకవేళ (newobj->ఆధారం_దత్తాంశం == ఖాళీ) {
			llcache_object_destroy(newobj);
			తిరిగి NSERROR_NOMEM;
		}
		జ్ఞాపకం_నకలు(newobj->ఆధారం_దత్తాంశం, object->ఆధారం_దత్తాంశం,
				newobj->ఆధారం_len);
	}

	ఒకవేళ (object->అంకె_శీర్షికలు > 0) {
		newobj->శీర్షికలు = సున్నా_జ్ఞాపకం_ఇవ్వు(object->అంకె_శీర్షికలు,
				పరిమాణం(llcache_శీర్షిక));
		ఒకవేళ (newobj->శీర్షికలు == ఖాళీ) {
			llcache_object_destroy(newobj);
			తిరిగి NSERROR_NOMEM;
		}
		అయితే (newobj->అంకె_శీర్షికలు < object->అంకె_శీర్షికలు) {
			llcache_శీర్షిక *nh =
					&(newobj->శీర్షికలు[newobj->అంకె_శీర్షికలు]);
			llcache_శీర్షిక *oh =
					&(object->శీర్షికలు[newobj->అంకె_శీర్షికలు]);
			newobj->అంకె_శీర్షికలు += 1;
			nh->name = strdup(oh->name);
			nh->విలువ = strdup(oh->విలువ);
			ఒకవేళ (nh->name == ఖాళీ || nh->విలువ == ఖాళీ) {
				llcache_object_destroy(newobj);
				తిరిగి NSERROR_NOMEM;
			}
		}
	}

	ఒకవేళ (object->chain != ఖాళీ) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		ఒకవేళ (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			తిరిగి error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	తిరిగి NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
స్థిర అంతఃవరుస uint32_t
మొత్తం_object_కొలత(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = పరిమాణం(*object);
	tot += nsurl_నిడివి(object->url);

	ఒకవేళ (object->ఆధారం_దత్తాంశం != ఖాళీ) {
		tot += object->ఆధారం_len;
	}

	tot += పరిమాణం(llcache_శీర్షిక) * object->అంకె_శీర్షికలు;

	కొరకు (hdrc = 0; hdrc < object->అంకె_శీర్షికలు; hdrc++) {
		ఒకవేళ (object->శీర్షికలు[hdrc].name != ఖాళీ) {
			tot += అక్షరశ్రేణి_పొడవు(object->శీర్షికలు[hdrc].name);
		}
		ఒకవేళ (object->శీర్షికలు[hdrc].విలువ != ఖాళీ) {
			tot += అక్షరశ్రేణి_పొడవు(object->శీర్షికలు[hdrc].విలువ);
		}
	}

	tot += cert_chain_కొలత(object->chain);

	తిరిగి tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
స్థిర శూన్య llcache_catch_పైకి_all_users(శూన్య *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_పైకి = true;

	/* Catch new users up with state of objects */
	కొరకు (object = llcache->cached_objects; object != ఖాళీ;
			object = object->తదుపరి) {
		llcache_object_notify_users(object);
	}

	కొరకు (object = llcache->uncached_objects; object != ఖాళీ;
			object = object->తదుపరి) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
స్థిర శూన్య llcache_users_not_caught_పైకి(శూన్య)
{
	ఒకవేళ (llcache->all_caught_పైకి) {
		llcache->all_caught_పైకి = false;
		guit->misc->కాలపట్టిక(0, llcache_catch_పైకి_all_users, ఖాళీ);
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
శూన్య llcache_clean(bool purge)
{
	llcache_object *object, *తదుపరి;
	uint32_t llcache_కొలత = 0;
	పూర్ణాంకం మిగిలిన_lifetime;
	uint32_t హద్దు;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	ఒకవేళ (purge) {
		హద్దు = 0;
	} లేకపోతే {
		హద్దు = llcache->హద్దు;
	}

	/* Uncacheable objects with no users or fetches */
	కొరకు (object = llcache->uncached_objects;
	     object != ఖాళీ;
	     object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		/* The candidate count of uncacheable objects is always 0 */
		ఒకవేళ ((object->users == ఖాళీ) &&
		    (object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} లేకపోతే {
			llcache_కొలత += మొత్తం_object_కొలత(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	కొరకు (object = llcache->cached_objects;
	     object != ఖాళీ;
	     object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		మిగిలిన_lifetime = llcache_object_rfc2616_మిగిలిన_lifetime(
				&object->cache);

		ఒకవేళ ((object->users == ఖాళీ) &&
		    (object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ) &&
		    (మిగిలిన_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				ఒకవేళ (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} లేకపోతే {
			/* object has users so account for the storage */
			llcache_కొలత += మొత్తం_object_కొలత(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	ఒకవేళ (హద్దు < llcache_కొలత) {
		llcache_persist(ఖాళీ);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	కొరకు (object = llcache->cached_objects;
	     ((హద్దు < llcache_కొలత) && (object != ఖాళీ));
	     object = తదుపరి) {
		తదుపరి = object->తదుపరి;
		ఒకవేళ ((object->users == ఖాళీ) &&
		    (object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->ఆధారం_దత్తాంశం = ఖాళీ;

			llcache_కొలత -=	object->ఆధారం_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->ఆధారం_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	కొరకు (object = llcache->cached_objects;
	     ((హద్దు < llcache_కొలత) && (object != ఖాళీ));
	     object = తదుపరి) {
		తదుపరి = object->తదుపరి;
		ఒకవేళ ((object->users == ఖాళీ) &&
		    (object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->ఆధారం_దత్తాంశం == ఖాళీ)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ఆధారం_len,
			      (దీర్ఘ)(time(ఖాళీ) - object->చివరి_వాడిన),
			      object,
			      nsurl_access(object->url));

			llcache_కొలత -=	మొత్తం_object_కొలత(object);

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
	కొరకు (object = llcache->cached_objects;
	     ((హద్దు < llcache_కొలత) && (object != ఖాళీ));
	     object = తదుపరి) {
		తదుపరి = object->తదుపరి;

		ఒకవేళ ((object->users == ఖాళీ) &&
		    (object->candidate_లెక్క == 0) &&
		    (object->fetch.fetch == ఖాళీ) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->ఆధారం_len,
			      (దీర్ఘ)(time(ఖాళీ) - object->చివరి_వాడిన),
			      object,
			      nsurl_access(object->url));

			llcache_కొలత -=	object->ఆధారం_len + పరిమాణం(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_కొలత, హద్దు);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(స్థిరరాశి నిర్మాణం llcache_parameters *prm)
{
	llcache = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం llcache_s));
	ఒకవేళ (llcache == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	llcache->హద్దు = prm->హద్దు;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->గరిష్ఠ_bandwidth = prm->గరిష్ఠ_bandwidth;
	llcache->సమయం_కాలాంశం = prm->సమయం_కాలాంశం;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_పైకి = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->హద్దు);

	/* backing store initialisation */
	తిరిగి guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
శూన్య llcache_finalise(శూన్య)
{
	llcache_object *object, *తదుపరి;
	uint64_t మొత్తం_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(ఖాళీ);
	/* Now clear the persistence callback */
	guit->misc->కాలపట్టిక(-1, llcache_persist, ఖాళీ);

	/* Clean uncached objects */
	కొరకు (object = llcache->uncached_objects; object != ఖాళీ; object = తదుపరి) {
		llcache_object_user *user, *తదుపరి_user;

		తదుపరి = object->తదుపరి;

		కొరకు (user = object->users; user != ఖాళీ; user = తదుపరి_user) {
			తదుపరి_user = user->తదుపరి;

			ఒకవేళ (user->handle != ఖాళీ)
				విడుదల(user->handle);

			విడుదల(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ఖాళీ;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	కొరకు (object = llcache->cached_objects; object != ఖాళీ; object = తదుపరి) {
		llcache_object_user *user, *తదుపరి_user;

		తదుపరి = object->తదుపరి;

		కొరకు (user = object->users; user != ఖాళీ; user = తదుపరి_user) {
			తదుపరి_user = user->తదుపరి;

			ఒకవేళ (user->handle != ఖాళీ)
			       విడుదల(user->handle);

			విడుదల(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = ఖాళీ;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	ఒకవేళ (llcache->మొత్తం_elapsed > 0) {
		మొత్తం_bandwidth = (llcache->మొత్తం_written * 1000) /
			llcache->మొత్తం_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->మొత్తం_written,
	      llcache->మొత్తం_elapsed,
	      మొత్తం_bandwidth);

	విడుదల(llcache);
	llcache = ఖాళీ;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			స్థిరరాశి llcache_post_దత్తాంశం *post,
			llcache_handle_callback cb, శూన్య *pw,
			llcache_handle **ఫలితం)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_లోపల_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_లోపల_use);
	ఒకవేళ (error != NSERROR_OK) {
		తిరిగి error;
	}

	/* Can we fetch this URL at all? */
	ఒకవేళ (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		తిరిగి NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	ఒకవేళ (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		తిరిగి error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_లోపల_use, &object);
	ఒకవేళ (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		తిరిగి error;
	}

	/* Add user to object */
	llcache_object_కూడు_user(object, user);

	*ఫలితం = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_పైకి();

	nsurl_unref(hsts_url);

	తిరిగి NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, శూన్య *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	తిరిగి NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_కనుగొను_user(handle);

	ప్రతిజ్ఞా(user != ఖాళీ);

	ఒకవేళ (user->iterator_లక్ష్యం) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} లేకపోతే {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		ఒకవేళ (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	తిరిగి error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **ఫలితం)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	ఒకవేళ (error == NSERROR_OK) {
		llcache_object_కూడు_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*ఫలితం = newuser->handle;
	}

	తిరిగి error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_కనుగొను_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	ఒకవేళ (user->prev != ఖాళీ)
		all_alone = false;
	ఒకవేళ (user->తదుపరి != ఖాళీ)
		all_alone = false;

	ఒకవేళ (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		ఒకవేళ (error != NSERROR_OK)
			తిరిగి error;

		/* Move across to the new object */
		ఒకవేళ (user->iterator_లక్ష్యం) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(llcache_object_user));
			ఒకవేళ (newuser == ఖాళీ) {
				llcache_object_destroy(newobject);
				తిరిగి NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = ఖాళీ;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_కూడు_user(newobject, newuser);
		} లేకపోతే {
			llcache_object_remove_user(object, user);
			llcache_object_కూడు_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_కూడు_గమ్యం_list(newobject,
				&llcache->uncached_objects);
	} లేకపోతే {
		/* We're the only user, so abort any fetch in progress */
		ఒకవేళ (object->fetch.fetch != ఖాళీ) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = ఖాళీ;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_దత్తాంశం(object);
	}

	తిరిగి error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_ప్రవాహం(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_కనుగొను_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	ఒకవేళ (user->prev != ఖాళీ || user->తదుపరి != ఖాళీ)
		తిరిగి NSERROR_OK;

	/* Forcibly uncache this object */
	ఒకవేళ (llcache_object_లోపల_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_కూడు_గమ్యం_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_ప్రవాహం_దత్తాంశం;

	తిరిగి NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_దత్తాంశం(llcache_handle *handle)
{
	ఒకవేళ ((handle->object != ఖాళీ) &&
	    (handle->object->fetch.fetch == ఖాళీ) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	తిరిగి NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(స్థిరరాశి llcache_handle *handle)
{
	తిరిగి handle->object != ఖాళీ ? handle->object->url : ఖాళీ;
}

/* See llcache.h for documentation */
స్థిరరాశి uint8_t *llcache_handle_get_ఆధారం_దత్తాంశం(స్థిరరాశి llcache_handle *handle,
		size_t *కొలత)
{
	*కొలత = handle->object != ఖాళీ ? handle->object->ఆధారం_len : 0;

	తిరిగి handle->object != ఖాళీ ? handle->object->ఆధారం_దత్తాంశం : ఖాళీ;
}

/* See llcache.h for documentation */
స్థిరరాశి అక్షరం *llcache_handle_get_శీర్షిక(స్థిరరాశి llcache_handle *handle,
		స్థిరరాశి అక్షరం *కీలకం)
{
	స్థిరరాశి llcache_object *object = handle->object;
	size_t i;

	ఒకవేళ (object == ఖాళీ)
		తిరిగి ఖాళీ;

	/* About as trivial as possible */
	కొరకు (i = 0; i < object->అంకె_శీర్షికలు; i++) {
		ఒకవేళ (strcasecmp(కీలకం, object->శీర్షికలు[i].name) == 0)
			తిరిగి object->శీర్షికలు[i].విలువ;
	}

	తిరిగి ఖాళీ;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(స్థిరరాశి llcache_handle *a,
		స్థిరరాశి llcache_handle *b)
{
	తిరిగి a->object == b->object;
}
