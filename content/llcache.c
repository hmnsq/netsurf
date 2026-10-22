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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <nsutils/time.h>
#अन्तर्भाव <nsutils/base64.h>

#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/time.h"
#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/backing_store.h"
#अन्तर्भाव "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
प्रकारनिर्देश गणना {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_शीर्षकाणि,		/**< Fetching headers */
	LLCACHE_FETCH_दत्त,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
प्रकारनिर्देश संरचना llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
संरचना llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	शून्य *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	size_t अष्टकानि;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
प्रकारनिर्देश संरचना llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_target;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	संरचना llcache_object_user *पूर्व;	/**< Previous in list */
	संरचना llcache_object_user *अग्र;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
प्रकारनिर्देश संरचना {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_दत्त *post;	/**< POST data, or NULL for GET */

	संरचना fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_गणनम्;	/**< Count of redirects followed */

	uint32_t retries_remaining;     /**< Number of times to retry on timeout */

	bool hsts_अन्तर्_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
प्रकारनिर्देश गणना {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#परिभाषय INVALID_AGE -1

/** Cache control data */
प्रकारनिर्देश संरचना {
	time_t req_time;	/**< Time of request */
	time_t res_time;	/**< Time of response */
	time_t fin_time;	/**< Time of request completion */
	time_t date;		/**< Date: response header */
	time_t expires;		/**< Expires: response header */
	पूर्णाङ्क age;		/**< Age: response header */
	पूर्णाङ्क max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	वर्ण *etag;		/**< Etag: response header */
	time_t चरमः_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
प्रकारनिर्देश संरचना {
	वर्ण *name;		/**< Header name */
	वर्ण *मानम्;		/**< Header value */
} llcache_शीर्षकम्;

/** Current status of an object's data */
प्रकारनिर्देश गणना {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
संरचना llcache_object {
	llcache_object *पूर्व;	     /**< Previous in list */
	llcache_object *अग्र;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *source_दत्त;	     /**< Source data for object */
	size_t source_len;	     /**< Byte length of source data */
	size_t source_alloc;	     /**< Allocated size of source buffer */

	संरचना cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_गणनम्;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_शीर्षकम् *शीर्षकाणि;     /**< Fetch headers */
	size_t अङ्कः_शीर्षकाणि;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	time_t चरमः_प्रयुक्तम्; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
संरचना llcache_s {
	/** Head of the low-level cached object list */
	llcache_object *cached_objects;

	/** Head of the low-level uncached object list */
	llcache_object *uncached_objects;

	/** The target upper bound for the RAM cache size */
	uint32_t सीमा;

	/** The number of fetch attempts we make when timing out */
	uint32_t fetch_attempts;

	/** Whether or not our users are caught up */
	bool all_caught_ऊर्ध्वम्;


	/* backing store elements */


	/**
	 * The minimum lifetime to consider sending objects to backing
	 * store.
	 */
	पूर्णाङ्क minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	अचिह्नित दीर्घ time_कालांशः;

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
	size_t अधिकतमम्_bandwidth;

	/**
	 * Total number of bytes written to backing store.
	 */
	uint64_t योग_written;

	/**
	 * Total number of milliseconds taken to write to backing store.
	 */
	uint64_t योग_elapsed;

};

/** low level cache state */
स्थिर संरचना llcache_s *llcache = रिक्त;

/* forward referenced callback function */
स्थिर शून्य llcache_fetch_callback(ध्रुव fetch_सन्देशः *सन्देशः, शून्य *p);

/* forward referenced catch up function */
स्थिर शून्य llcache_users_not_caught_ऊर्ध्वम्(शून्य);


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
स्थिर nserror llcache_object_user_new(llcache_handle_callback cb, शून्य *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = शून्यस्मृतिं_देहि(1, परिमाण(llcache_handle));
	यदि (h == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	u = शून्यस्मृतिं_देहि(1, परिमाण(llcache_object_user));
	यदि (u == रिक्त) {
		मुक्त(h);
		प्रत्यागम NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (शून्य *) cb, pw);

	*user = u;

	प्रत्यागम NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
स्थिर nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	प्रतिज्ञा(user->अग्र == रिक्त);
	प्रतिज्ञा(user->पूर्व == रिक्त);

	यदि (user->handle != रिक्त)
		मुक्त(user->handle);

	मुक्त(user);

	प्रत्यागम NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
स्थिर nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	प्रतिज्ञा(user != रिक्त);
	प्रतिज्ञा(object != रिक्त);
	प्रतिज्ञा(object->users != रिक्त);
	प्रतिज्ञा(user->handle == रिक्त || user->handle->object == object);
	प्रतिज्ञा((user->पूर्व != रिक्त) || (object->users == user));

	यदि (user == object->users)
		object->users = user->अग्र;
	अन्यथा
		user->पूर्व->अग्र = user->अग्र;

	यदि (user->अग्र != रिक्त)
		user->अग्र->पूर्व = user->पूर्व;

	user->अग्र = user->पूर्व = रिक्त;

	/* record the time the last user was removed from the object */
	यदि (object->users == रिक्त) {
		object->चरमः_प्रयुक्तम् = time(रिक्त);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	प्रत्यागम NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
स्थिर nserror llcache_send_event_गन्तव्यम्_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *अग्र_user;

	user = object->users;
	यावत् (user != रिक्त) {
		bool was_target = user->iterator_target;
		user->iterator_target = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		अग्र_user = user->अग्र;

		user->iterator_target = was_target;

		यदि (user->queued_for_delete && !was_target) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		यदि (error != NSERROR_OK)
			भङ्ग;

		user = अग्र_user;
	}

	प्रत्यागम error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_object_new(nsurl *url, llcache_object **परिणाम)
{
	llcache_object *obj = शून्यस्मृतिं_देहि(1, परिमाण(llcache_object));
	यदि (obj == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*परिणाम = obj;

	प्रत्यागम NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_post_दत्त_clone(ध्रुव llcache_post_दत्त *orig,
		llcache_post_दत्त **clone)
{
	llcache_post_दत्त *post_clone;

	post_clone = शून्यस्मृतिं_देहि(1, परिमाण(llcache_post_दत्त));
	यदि (post_clone == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	यदि (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->दत्त.urlenc = strdup(orig->दत्त.urlenc);
		यदि (post_clone->दत्त.urlenc == रिक्त) {
			मुक्त(post_clone);

			प्रत्यागम NSERROR_NOMEM;
		}
	} अन्यथा {
		post_clone->दत्त.multipart = fetch_multipart_दत्त_clone(
				orig->दत्त.multipart);
		यदि (post_clone->दत्त.multipart == रिक्त) {
			मुक्त(post_clone);

			प्रत्यागम NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror llcache_fetch_विभज_शीर्षकम्(ध्रुव uint8_t *दत्त, size_t len,
		वर्ण **name, वर्ण **मानम्)
{
	वर्ण *n, *v;
	ध्रुव uint8_t *colon;

	/* Strip leading whitespace from name */
	यावत् (दत्त[0] == ' ' || दत्त[0] == '\t' ||
	       दत्त[0] == '\r' || दत्त[0] == '\n') {
		दत्त++;
	}

	/* Find colon */
	colon = (ध्रुव uint8_t *) strchr((ध्रुव वर्ण *) दत्त, ':');
	यदि (colon == रिक्त) {
		/* Failed, assume a key with no value */
		colon = दत्त + वाक्यदैर्घ्यम्((ध्रुव वर्ण *)दत्त);

		/* Strip trailing whitespace from name */
		यावत् ((colon > दत्त) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((ध्रुव वर्ण *) दत्त, colon - दत्त);
		यदि (n == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		v = strdup("");
		यदि (v == रिक्त) {
			मुक्त(n);
			प्रत्यागम NSERROR_NOMEM;
		}
	} अन्यथा {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		यावत् (colon > दत्त && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((ध्रुव वर्ण *) दत्त, colon - दत्त);
		यदि (n == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		/* Find colon again */
		यावत् (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		कुरु {
			colon++;
		} यावत् (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		यावत् (len > 0 && (दत्त[len - 1] == ' ' ||
				दत्त[len - 1] == '\t' ||
				दत्त[len - 1] == '\r' ||
				दत्त[len - 1] == '\n')) {
			len--;
		}

		v = strndup((ध्रुव वर्ण *) colon, len - (colon - दत्त));
		यदि (v == रिक्त) {
			मुक्त(n);
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	*name = n;
	*मानम् = v;

	प्रत्यागम NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror
llcache_fetch_parse_cache_control(llcache_object *object, वर्ण *मानम्)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(मानम्, &cc);
	यदि (error != NSERROR_OK) {
		/* Ignore parse errors */
		प्रत्यागम NSERROR_OK;
	}

	यदि (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	यदि (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	प्रत्यागम NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror
llcache_fetch_शीर्षकम्_cache_control(llcache_object *object,
				   वर्ण *name,
				   वर्ण *मानम्)
{
	nserror res;
	size_t name_len;

	/* Parse cache headers to populate cache control data */
	name_len = वाक्यदैर्घ्यम्(name);

	विकल्प (name_len) {
	पक्ष 3:
		यदि (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			यदि ('0' <= *मानम् && *मानम् <= '9') {
				object->cache.age = atoi(मानम्);
			}

		}
		भङ्ग;

	पक्ष 4:
		यदि (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(मानम्,
					    वाक्यदैर्घ्यम्(मानम्),
					    &object->cache.date);
			यदि (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      मानम्, res);
			}
		} अन्यथा यदि (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			मुक्त(object->cache.etag);
			object->cache.etag = strdup(मानम्);
			यदि (object->cache.etag == रिक्त) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				प्रत्यागम NSERROR_NOMEM;
			}
		}
		भङ्ग;

	पक्ष 7:
		यदि (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(मानम्,
					    वाक्यदैर्घ्यम्(मानम्),
					    &object->cache.expires);
			यदि (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      मानम्, res);
				object->cache.expires = (time_t)0x7fffffff;
			}
		}
		भङ्ग;

	पक्ष 13:
		यदि (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,मानम्);
		} अन्यथा यदि (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(मानम्,
				      वाक्यदैर्घ्यम्(मानम्),
				      &object->cache.चरमः_modified);
		}
		भङ्ग;
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
स्थिर अन्तःस्थ शून्य llcache_destroy_शीर्षकाणि(llcache_object *object)
{
	यावत् (object->अङ्कः_शीर्षकाणि > 0) {
		object->अङ्कः_शीर्षकाणि--;

		मुक्त(object->शीर्षकाणि[object->अङ्कः_शीर्षकाणि].name);
		मुक्त(object->शीर्षकाणि[object->अङ्कः_शीर्षकाणि].मानम्);
	}
	मुक्त(object->शीर्षकाणि);
	object->शीर्षकाणि = रिक्त;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
स्थिर अन्तःस्थ शून्य llcache_invalidate_cache_control_दत्त(llcache_object *object)
{
	मुक्त(object->cache.etag);
	स्मृतिं_पूरय(&(object->cache), 0, परिमाण(llcache_cache_control));

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
स्थिर nserror llcache_fetch_process_शीर्षकम्(llcache_object *object,
		ध्रुव uint8_t *दत्त, size_t len)
{
	nserror res;
	वर्ण *name, *मानम्;
	llcache_शीर्षकम् *अस्थायी;

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
	यदि (वाक्ये_तुलय((ध्रुव वर्ण *) दत्त, "HTTP/", SLEN("HTTP/")) == 0) {
		time_t req_time = object->cache.req_time;

		llcache_invalidate_cache_control_दत्त(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_time = req_time;

		llcache_destroy_शीर्षकाणि(object);
	}

	/* Set fetch response time if not already set */
	यदि (object->cache.res_time == 0) {
		object->cache.res_time = time(रिक्त);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_विभज_शीर्षकम्(दत्त, len, &name, &मानम्);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	/* deal with empty header */
	यदि (name[0] == 0) {
		मुक्त(name);
		मुक्त(मानम्);
		प्रत्यागम NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_शीर्षकम्_cache_control(object, name, मानम्);
	यदि (res != NSERROR_OK) {
		मुक्त(name);
		मुक्त(मानम्);
		प्रत्यागम res;
	}

	/* Append header data to the object's headers array */
	अस्थायी = realloc(object->शीर्षकाणि,
		       (object->अङ्कः_शीर्षकाणि + 1) * परिमाण(llcache_शीर्षकम्));
	यदि (अस्थायी == रिक्त) {
		मुक्त(name);
		मुक्त(मानम्);
		प्रत्यागम NSERROR_NOMEM;
	}

	object->शीर्षकाणि = अस्थायी;

	object->शीर्षकाणि[object->अङ्कः_शीर्षकाणि].name = name;
	object->शीर्षकाणि[object->अङ्कः_शीर्षकाणि].मानम् = मानम्;

	object->अङ्कः_शीर्षकाणि++;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror get_referer_शीर्षकम्(nsurl *url, nsurl *referer, वर्ण **शीर्षकम्_बहिर्गमनम्)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool मेलनम्;
	bool match1;
	bool match2;
	वर्ण *शीर्षकम्;

	/* Determine whether to send the Referer header */
	यदि (!nsoption_bool(send_referer)) {
		प्रत्यागम NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (scheme == रिक्त) {
		प्रत्यागम NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	यदि (ref_scheme == रिक्त) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		प्रत्यागम NSERROR_INVALID;
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
	यदि (lwc_string_isequal(scheme, ref_scheme,
			       &मेलनम्) != lwc_error_ok) {
		मेलनम् = false;
	}
	यदि (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	यदि (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	यदि (मेलनम् == true || (match1 == true && match2 == true)) {
		ध्रुव size_t len = SLEN("Referer: ") +
			nsurl_दीर्घता(referer) + 1;

		शीर्षकम् = स्मृतिं_देहि(len);
		यदि (शीर्षकम् == रिक्त) {
			res = NSERROR_NOMEM;
		} अन्यथा {
			snprintf(शीर्षकम्, len, "Referer: %s",
				 nsurl_access(referer));

			*शीर्षकम्_बहिर्गमनम् = शीर्षकम्;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	प्रत्यागम res;
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
स्थिर nserror llcache_object_refetch(llcache_object *object)
{
	ध्रुव वर्ण *urlenc = रिक्त;
	संरचना fetch_multipart_दत्त *multipart = रिक्त;
	वर्ण **शीर्षकाणि = रिक्त;
	पूर्णाङ्क शीर्षकम्_idx = 0;
	nserror res;

	यदि (object->fetch.post != रिक्त) {
		यदि (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->दत्त.urlenc;
		} अन्यथा {
			multipart = object->fetch.post->दत्त.multipart;
		}
	}

	/* Generate headers */
	शीर्षकाणि = स्मृतिं_देहि(4 * परिमाण(वर्ण *));
	यदि (शीर्षकाणि == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	यदि (object->cache.etag != रिक्त) {
		ध्रुव size_t len = SLEN("If-None-Match: ") +
				वाक्यदैर्घ्यम्(object->cache.etag) + 1;

		शीर्षकाणि[शीर्षकम्_idx] = स्मृतिं_देहि(len);
		यदि (शीर्षकाणि[शीर्षकम्_idx] == रिक्त) {
			मुक्त(शीर्षकाणि);
			प्रत्यागम NSERROR_NOMEM;
		}

		snprintf(शीर्षकाणि[शीर्षकम्_idx], len, "If-None-Match: %s",
				object->cache.etag);

		शीर्षकम्_idx++;
	}

	/* cache-control header for modification time */
	यदि (object->cache.चरमः_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		ध्रुव size_t len = SLEN("If-Modified-Since: ") + 29 + 1;

		शीर्षकाणि[शीर्षकम्_idx] = स्मृतिं_देहि(len);
		यदि (शीर्षकाणि[शीर्षकम्_idx] == रिक्त) {
			यावत् (--शीर्षकम्_idx >= 0)
				मुक्त(शीर्षकाणि[शीर्षकम्_idx]);
			मुक्त(शीर्षकाणि);
			प्रत्यागम NSERROR_NOMEM;
		}

		snprintf(शीर्षकाणि[शीर्षकम्_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.चरमः_modified));

		शीर्षकम्_idx++;
	}

	/* Referer header */
	यदि (object->fetch.referer != रिक्त) {
		यदि (get_referer_शीर्षकम्(object->url,
				       object->fetch.referer,
				       &शीर्षकाणि[शीर्षकम्_idx]) == NSERROR_OK) {
			शीर्षकम्_idx++;
		}
	}
	शीर्षकाणि[शीर्षकम्_idx] = रिक्त;

	/* Reset cache control data */
	llcache_invalidate_cache_control_दत्त(object);
	object->cache.req_time = time(रिक्त);
	object->cache.fin_time = object->cache.req_time;

	/* Reset fetch state */
	object->fetch.state = LLCACHE_FETCH_INIT;

	NSLOG(llcache, DEBUG, "Re-fetching %p", object);

	/* Kick off fetch */
	res = fetch_start(object->url,
			  object->fetch.referer,
			  llcache_fetch_callback,
			  object,
			  object->fetch.flags & LLCACHE_RETRIEVE_NO_ERROR_पृष्ठानि,
			  urlenc,
			  multipart,
			  object->fetch.flags & LLCACHE_RETRIEVE_VERIFIABLE,
			  object->fetch.tried_with_tls_downgrade,
			  (ध्रुव वर्ण **)शीर्षकाणि,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	यावत् (--शीर्षकम्_idx >= 0) {
		मुक्त(शीर्षकाणि[शीर्षकम्_idx]);
	}
	मुक्त(शीर्षकाणि);

	प्रत्यागम res;
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
स्थिर nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, ध्रुव llcache_post_दत्त *post,
		uint32_t redirect_गणनम्, bool hsts_अन्तर्_use)
{
	nserror error;
	nsurl *referer_clone = रिक्त;
	llcache_post_दत्त *post_clone = रिक्त;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	यदि (post != रिक्त) {
		error = llcache_post_दत्त_clone(post, &post_clone);
		यदि (error != NSERROR_OK)
			प्रत्यागम error;
	}

	यदि (referer != रिक्त)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_गणनम् = redirect_गणनम्;
	object->fetch.retries_remaining = llcache->fetch_attempts;
	object->fetch.hsts_अन्तर्_use = hsts_अन्तर्_use;

	प्रत्यागम llcache_object_refetch(object);
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
स्थिर nserror llcache_object_destroy(llcache_object *object)
{
	size_t i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	यदि (object->source_दत्त != रिक्त) {
		यदि (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} अन्यथा {
			मुक्त(object->source_दत्त);
		}
	}

	nsurl_unref(object->url);

	यदि (object->fetch.fetch != रिक्त) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = रिक्त;
	}

	यदि (object->fetch.referer != रिक्त)
		nsurl_unref(object->fetch.referer);

	यदि (object->fetch.post != रिक्त) {
		यदि (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			मुक्त(object->fetch.post->दत्त.urlenc);
		} अन्यथा {
			fetch_multipart_दत्त_destroy(
					object->fetch.post->दत्त.multipart);
		}

		मुक्त(object->fetch.post);
	}

	मुक्त(object->cache.etag);

	प्रति (i = 0; i < object->अङ्कः_शीर्षकाणि; i++) {
		मुक्त(object->शीर्षकाणि[i].name);
		मुक्त(object->शीर्षकाणि[i].मानम्);
	}
	मुक्त(object->शीर्षकाणि);

	मुक्त(object);

	प्रत्यागम NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
स्थिर nserror llcache_object_सङ्कलय_गन्तव्यम्_list(llcache_object *object,
		llcache_object **list)
{
	object->पूर्व = रिक्त;
	object->अग्र = *list;

	यदि (*list != रिक्त)
		(*list)->पूर्व = object;
	*list = object;

	प्रत्यागम NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
स्थिर पूर्णाङ्क
llcache_object_rfc2616_remaining_lifetime(ध्रुव llcache_cache_control *cd)
{
	पूर्णाङ्क current_age, freshness_lifetime;
	time_t now = time(रिक्त);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_time - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_time - cd->req_time + now - cd->res_time;

	/* Determine freshness lifetime of this object */
	यदि (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} अन्यथा यदि (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} अन्यथा यदि (cd->चरमः_modified != 0) {
		freshness_lifetime = (now - cd->चरमः_modified) / 10;
	} अन्यथा {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	यदि ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		प्रत्यागम freshness_lifetime - current_age;
	}

	प्रत्यागम 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
स्थिर bool llcache_object_is_fresh(ध्रुव llcache_object *object)
{
	पूर्णाङ्क remaining_lifetime;
	ध्रुव llcache_cache_control *cd = &object->cache;

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
	प्रत्यागम ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
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
स्थिर nserror llcache_object_clone_cache_दत्त(llcache_object *source,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	यदि (source->cache.etag != रिक्त) {
		वर्ण *etag = source->cache.etag;

		यदि (deep) {
			/* Copy the etag */
			etag = strdup(source->cache.etag);
			यदि (etag == रिक्त)
				प्रत्यागम NSERROR_NOMEM;
		} अन्यथा {
			/* Destination takes ownership */
			source->cache.etag = रिक्त;
		}

		यदि (destination->cache.etag != रिक्त)
			मुक्त(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_time = source->cache.req_time;
	destination->cache.res_time = source->cache.res_time;
	destination->cache.fin_time = source->cache.fin_time;

	यदि (source->cache.date != 0)
		destination->cache.date = source->cache.date;

	यदि (source->cache.expires != 0)
		destination->cache.expires = source->cache.expires;

	यदि (source->cache.age != INVALID_AGE)
		destination->cache.age = source->cache.age;

	यदि (source->cache.max_age != INVALID_AGE)
		destination->cache.max_age = source->cache.max_age;

	यदि (source->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = source->cache.no_cache;

	यदि (source->cache.चरमः_modified != 0)
		destination->cache.चरमः_modified = source->cache.चरमः_modified;

	प्रत्यागम NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
स्थिर nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	यदि (object == *list)
		*list = object->अग्र;
	अन्यथा
		object->पूर्व->अग्र = object->अग्र;

	यदि (object->अग्र != रिक्त)
		object->अग्र->पूर्व = object->पूर्व;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror llcache_retrieve_persisted_दत्त(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	यदि ((object->source_दत्त != रिक्त) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		प्रत्यागम NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	प्रत्यागम guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->source_दत्त,
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
स्थिर nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **दत्त_बहिर्गमनम्,
			   size_t *datasize_बहिर्गमनम्)
{
	size_t allocsize;
	पूर्णाङ्क datasize;
	uint8_t *दत्त;
	वर्ण *op;
	अचिह्नित पूर्णाङ्क hloop;
	पूर्णाङ्क use;
	size_t cert_chain_गभीरता;

	यदि (object->chain != रिक्त) {
		cert_chain_गभीरता = object->chain->गभीरता;
	} अन्यथा {
		cert_chain_गभीरता = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	प्रति (hloop = 0 ; hloop < object->अङ्कः_शीर्षकाणि ; hloop++) {
		allocsize += वाक्यदैर्घ्यम्(object->शीर्षकाणि[hloop].name) + 1;
		allocsize += वाक्यदैर्घ्यम्(object->शीर्षकाणि[hloop].मानम्) + 1;
	}

	allocsize += nsurl_दीर्घता(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	प्रति (hloop = 0; hloop < cert_chain_गभीरता; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_दीर्घता + 2) / 3);
	}

	दत्त = स्मृतिं_देहि(allocsize);
	यदि (दत्त == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	op = (वर्ण *)दत्त;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	यदि (use < 0) {
		गच्छ operror;
	}
	use++; /* does not count the null */
	यदि (use > datasize) {
		गच्छ अतिप्रवाहः;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->source_len);
	यदि (use < 0) {
		गच्छ operror;
	}
	use++; /* does not count the null */
	यदि (use > datasize)
		गच्छ अतिप्रवाहः;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_time);
	यदि (use == 0)
		गच्छ अतिप्रवाहः;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_time);
	यदि (use == 0)
		गच्छ अतिप्रवाहः;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_time);
	यदि (use == 0)
		गच्छ अतिप्रवाहः;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->अङ्कः_शीर्षकाणि);
	यदि (use < 0) {
		गच्छ operror;
	}
	use++; /* does not count the null */
	यदि (use > datasize)
		गच्छ अतिप्रवाहः;
	op += use;
	datasize -= use;

	/*शीर्षकाणि*/
	प्रति (hloop = 0 ; hloop < object->अङ्कः_शीर्षकाणि ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->शीर्षकाणि[hloop].name,
			       object->शीर्षकाणि[hloop].मानम्);
		यदि (use < 0) {
			गच्छ operror;
		}
		use++; /* does not count the null */
		यदि (use > datasize)
			गच्छ अतिप्रवाहः;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_गभीरता);
	यदि (use < 0) {
		गच्छ operror;
	}
	use++; /* does not count the null */
	यदि (use > datasize)
		गच्छ अतिप्रवाहः;
	op += use;
	datasize -= use;

	/* SSL certificates */
	प्रति (hloop = 0; hloop < cert_chain_गभीरता; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (पूर्णाङ्क)(object->chain->certs[hloop].err));
		यदि (use < 0) {
			गच्छ operror;
		}
		use++; /* does not count the null */
		यदि (use > datasize)
			गच्छ अतिप्रवाहः;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		यदि (object->chain->certs[hloop].der != रिक्त) {
			size_t output_दीर्घता = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_दीर्घता,
				(uint8_t *)op,
				&output_दीर्घता);
			यदि (res != NSUERROR_OK) {
				गच्छ operror;
			}
			use = output_दीर्घता;
		} अन्यथा {
			use = 0;
		}
		use++; /* allow for null */
		यदि (use > datasize)
			गच्छ अतिप्रवाहः;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*दत्त_बहिर्गमनम् = दत्त;
	*datasize_बहिर्गमनम् = allocsize - datasize;

	प्रत्यागम NSERROR_OK;

अतिप्रवाहः:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	मुक्त(दत्त);
	प्रत्यागम NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	मुक्त(दत्त);
	प्रत्यागम NSERROR_INVALID;
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
स्थिर nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = रिक्त;
	size_t metadatalen = 0;
	size_t remaining = 0;
	nsurl *metadataurl;
	अचिह्नित पूर्णाङ्क पङ्क्तिः;
	वर्ण *ln;
	पूर्णाङ्क lnsize;

	size_t source_दीर्घता;
	time_t request_time;
	time_t response_time;
	time_t completion_time;
	size_t अङ्कः_शीर्षकाणि;
	size_t hloop;
	size_t ssl_cert_गणनम् = 0;
	संरचना cert_chain *chain = रिक्त;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
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
	पङ्क्तिः = 1;
	ln = (वर्ण *)metadata;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	यदि (lnsize < 7) {
		res = NSERROR_INVALID;
		गच्छ format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	यदि (res != NSERROR_OK)
		गच्छ format_error;

	यदि (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		प्रत्यागम NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	पङ्क्तिः = 2;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	यदि ((lnsize < 1) || (वाक्यात्_पठ(ln, "%" PRIsizet, &source_दीर्घता) != 1)) {
		res = NSERROR_INVALID;
		गच्छ format_error;
	}


	/* metadata line 3 is the time of request */
	पङ्क्तिः = 3;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_time);
	यदि (res != NSERROR_OK)
		गच्छ format_error;


	/* metadata line 4 is the time of response */
	पङ्क्तिः = 4;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_time);
	यदि (res != NSERROR_OK)
		गच्छ format_error;


	/* metadata line 5 is the time of request completion */
	पङ्क्तिः = 5;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_time);
	यदि (res != NSERROR_OK)
		गच्छ format_error;


	/* metadata line 6 is the number of headers */
	पङ्क्तिः = 6;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	यदि ((lnsize < 1) || (वाक्यात्_पठ(ln, "%" PRIsizet, &अङ्कः_शीर्षकाणि) != 1)) {
		res = NSERROR_INVALID;
		गच्छ format_error;
	}

	/* read headers */
	प्रति (hloop = 0 ; hloop < अङ्कः_शीर्षकाणि; hloop++) {
		पङ्क्तिः++;
		ln += lnsize + 1;
		lnsize = वाक्यदैर्घ्यम्(ln);
		remaining -= lnsize + 1;

		res = llcache_fetch_process_शीर्षकम्(object,
						   (uint8_t *)ln,
						   lnsize);
		यदि (res != NSERROR_OK)
			गच्छ format_error;
	}

	यदि (remaining == 0) {
		गच्छ skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	पङ्क्तिः++;
	ln += lnsize + 1;
	lnsize = वाक्यदैर्घ्यम्(ln);
	remaining -= lnsize + 1;

	यदि ((lnsize < 1) || (वाक्यात्_पठ(ln, "%" PRIsizet, &ssl_cert_गणनम्) != 1)) {
		res = NSERROR_INVALID;
		गच्छ format_error;
	}

	यदि (ssl_cert_गणनम् == 0) {
		गच्छ skip_ssl_certificates;
	}

	यदि (ssl_cert_गणनम् > MAX_CERT_गभीरता) {
		res = NSERROR_INVALID;
		गच्छ format_error;
	}

	res = cert_chain_alloc(ssl_cert_गणनम्, &chain);
	यदि (res != NSERROR_OK) {
		गच्छ format_error;
	}

	प्रति (hloop = 0; hloop < ssl_cert_गणनम्; hloop++) {
		पूर्णाङ्क errcode;
		nsuerror nsures;

		/* Certificate error code */
		पङ्क्तिः++;
		ln += lnsize + 1;
		lnsize = वाक्यदैर्घ्यम्(ln);
		remaining -= lnsize + 1;
		यदि ((lnsize < 1) || (वाक्यात्_पठ(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			गच्छ format_error;
		}
		यदि (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_ज्ञातम्) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} अन्यथा {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		पङ्क्तिः++;
		ln += lnsize + 1;
		lnsize = वाक्यदैर्घ्यम्(ln);
		remaining -= lnsize + 1;
		यदि (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((ध्रुव uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_दीर्घता);
			यदि (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				गच्छ format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->source_len = source_दीर्घता;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->source_alloc = metadatalen;

	object->cache.req_time = request_time;
	object->cache.res_time = response_time;
	object->cache.fin_time = completion_time;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	प्रत्यागम NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      पङ्क्तिः, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	प्रत्यागम res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
स्थिर अन्तःस्थ bool llcache__scheme_is_persistable(ध्रुव nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool मेलनम्;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_http,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	प्रत्यागम persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
स्थिर अन्तःस्थ bool llcache__scheme_is_cachable(ध्रुव nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool मेलनम्;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	यदि ((lwc_string_isequal(scheme, corestring_lwc_http,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_दत्त,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_resource,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_file,
			&मेलनम्) == lwc_error_ok &&
			(मेलनम् == true))) {
		cachable = true;
	}

	lwc_string_unref(scheme);

	प्रत्यागम cachable;
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
स्थिर nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				ध्रुव llcache_post_दत्त *post,
				uint32_t redirect_गणनम्)
{
	nserror error;
	nsurl *referer_clone = रिक्त;
	llcache_post_दत्त *post_clone = रिक्त;

	यदि (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	object->cache.req_time = time(रिक्त);
	object->cache.fin_time = object->cache.req_time;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	यदि (error != NSERROR_OK) {
		प्रत्यागम error;
	}

	/* entry came out of cache - need to setup object state */
	यदि (post != रिक्त) {
		error = llcache_post_दत्त_clone(post, &post_clone);
		यदि (error != NSERROR_OK)
			प्रत्यागम error;
	}

	यदि (referer != रिक्त) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_गणनम् = redirect_गणनम्;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = रिक्त;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   ध्रुव llcache_post_दत्त *post,
				   uint32_t redirect_गणनम्,
				   bool hsts_अन्तर्_use,
				   llcache_object **परिणाम)
{
	nserror error;
	llcache_object *obj, *newest = रिक्त;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==रिक्त?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	प्रति (obj = llcache->cached_objects; obj != रिक्त; obj = obj->अग्र) {

		यदि ((newest == रिक्त ||
		     obj->cache.req_time > newest->cache.req_time) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	यदि (newest == रिक्त) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		यदि (error != NSERROR_OK)
			प्रत्यागम error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_गणनम्);
		यदि (error == NSERROR_OK) {
			NSLOG(llcache, DEBUG, "retrieved object from persistent store");

			/* set newest object from persistent store which
			 * will cause the normal object handling to be used.
			 */
			newest = obj;

			/* Add new object to cached object list */
			llcache_object_सङ्कलय_गन्तव्यम्_list(obj, &llcache->cached_objects);

		}
		/* else no object found and irretrievable from cache,
		 * fall through with newest unset to start fetch
		 */
	}

	यदि ((newest != रिक्त) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_दत्त(newest);
		यदि (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*परिणाम = newest;

			प्रत्यागम NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		यदि (error != NSERROR_OK) {
			प्रत्यागम error;
		}
	} अन्यथा यदि (newest != रिक्त) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_दत्त(newest);
		यदि (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			यदि (error != NSERROR_OK)
				प्रत्यागम error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_दत्त(newest, obj, true);
			यदि (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				प्रत्यागम error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_गणनम्++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_गणनम्, hsts_अन्तर्_use);
			यदि (error != NSERROR_OK) {
				newest->candidate_गणनम्--;
				llcache_object_destroy(obj);
				प्रत्यागम error;
			}

			/* Add new object to cache */
			llcache_object_सङ्कलय_गन्तव्यम्_list(obj, &llcache->cached_objects);

			*परिणाम = obj;

			प्रत्यागम NSERROR_OK;
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
		यदि (error != NSERROR_OK) {
			प्रत्यागम error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_गणनम्, hsts_अन्तर्_use);
	यदि (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		प्रत्यागम error;
	}

	/* Add new object to cache */
	llcache_object_सङ्कलय_गन्तव्यम्_list(obj, &llcache->cached_objects);

	*परिणाम = obj;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ध्रुव llcache_post_दत्त *post,
			uint32_t redirect_गणनम्,
			bool hsts_अन्तर्_use,
			llcache_object **परिणाम)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==रिक्त?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	/* determine if content is cachable */
	यदि ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} अन्यथा यदि (post != रिक्त) {
		/* POST requests are never cached */
		uncachable = true;
	} अन्यथा {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	यदि (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		यदि (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			प्रत्यागम error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_गणनम्, hsts_अन्तर्_use);
		यदि (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			प्रत्यागम error;
		}

		/* Add new object to uncached list */
		llcache_object_सङ्कलय_गन्तव्यम्_list(obj, &llcache->uncached_objects);
	} अन्यथा {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_गणनम्,
				hsts_अन्तर्_use, &obj);
		यदि (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			प्रत्यागम error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*परिणाम = obj;

	nsurl_unref(defragmented_url);

	प्रत्यागम NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
स्थिर nserror llcache_object_सङ्कलय_user(llcache_object *object,
		llcache_object_user *user)
{
	प्रतिज्ञा(user->अग्र == रिक्त);
	प्रतिज्ञा(user->पूर्व == रिक्त);
	प्रतिज्ञा(user->handle != रिक्त);

	user->handle->object = object;

	user->पूर्व = रिक्त;
	user->अग्र = object->users;

	यदि (object->users != रिक्त)
		object->users->पूर्व = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	प्रत्यागम NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_hsts_transform_url(nsurl *url, nsurl **परिणाम,
		bool *hsts_अन्तर्_use)
{
	lwc_string *scheme = रिक्त;
	bool मेलनम्;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	यदि (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&मेलनम्) != lwc_error_ok || मेलनम् == false) {
		/* Non-HTTP fetch: no transform required */
		यदि (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&मेलनम्) == lwc_error_ok && मेलनम्) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_अन्तर्_use = urldb_get_hsts_enabled(url);
		} अन्यथा {
			/* Anything else: no HSTS */
			*hsts_अन्तर्_use = false;
		}
		lwc_string_unref(scheme);
		*परिणाम = nsurl_ref(url);
		प्रत्यागम error;
	}
	lwc_string_unref(scheme);

	यदि (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				परिणाम);
		*hsts_अन्तर्_use = (error == NSERROR_OK);
	} अन्यथा {
		*परिणाम = nsurl_ref(url);
		*hsts_अन्तर्_use = false;
	}

	प्रत्यागम error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_hsts_update_policy(llcache_object *object)
{
	size_t i;
	lwc_string *scheme = रिक्त;
	bool मेलनम् = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	यदि (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&मेलनम्) != lwc_error_ok || मेलनम् == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		प्रत्यागम NSERROR_OK;
	}
	lwc_string_unref(scheme);

	यदि (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		प्रत्यागम NSERROR_OK;
	}

	प्रति (i = 0; i < object->अङ्कः_शीर्षकाणि; i++) {
		यदि (strcasecmp("Strict-Transport-Security",
				object->शीर्षकाणि[i].name) == 0) {
			urldb_समुच्चयः_hsts_policy(object->url,
					object->शीर्षकाणि[i].मानम्);
			/* Only process the first one we find */
			भङ्ग;
		}
	}

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool मेलनम्;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	यदि ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&मेलनम्) == lwc_error_ok && मेलनम् == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&मेलनम्) == lwc_error_ok && मेलनम् == false)) {
		/* file, about and resource are not valid redirect targets */
		यदि ((lwc_string_isequal(scheme, corestring_lwc_file,
				&मेलनम्) == lwc_error_ok && मेलनम् == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&मेलनम्) == lwc_error_ok && मेलनम् == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&मेलनम्) == lwc_error_ok && मेलनम् == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			प्रत्यागम NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	यदि (fetch_can_fetch(dst_url) == false) {
		प्रत्यागम NSERROR_UNSAFE_REDIRECT;
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_fetch_redirect(llcache_object *object,
		ध्रुव वर्ण *target, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *अग्र;
	ध्रुव llcache_post_दत्त *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_अन्तर्_use;
	http_response_संहिता http_संहिता;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_संहिता = fetch_http_संहिता(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = रिक्त;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_दत्त(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(शून्य) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	यदि (object->fetch.redirect_गणनम् > nsoption_uint(fetch_redirect_सीमा)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		प्रत्यागम NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, target, &url);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_अन्तर्_use);
	nsurl_unref(url);
	यदि (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		प्रत्यागम res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	यदि (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		प्रत्यागम res;
	}

	/* alter behaviour based on specific redirect code */
	विकल्प (http_संहिता) {
	पक्ष HTTP_RESPONSE_MOVED_PERMANENTLY:
	पक्ष HTTP_RESPONSE_प्राप्तम्:
	पक्ष HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = रिक्त;
		भङ्ग;

	पक्ष HTTP_RESPONSE_TEMPORARY_REDIRECT:
	पक्ष HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		भङ्ग;

	स्वभाव:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_संहिता);
		nsurl_unref(hsts_url);
		प्रत्यागम NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.दत्त.redirect.from = object->url;
	event.दत्त.redirect.गन्तव्यम् = hsts_url;

	res = llcache_send_event_गन्तव्यम्_users(object, &event);
	यदि (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		प्रत्यागम res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_गणनम् + 1,
					hsts_अन्तर्_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	यदि (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		प्रति (user = object->users; user != रिक्त; user = अग्र) {
			अग्र = user->अग्र;

			llcache_object_remove_user(object, user);
			llcache_object_सङ्कलय_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	प्रत्यागम res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
स्थिर nserror llcache_object_cache_update(llcache_object *object)
{
	यदि (object->cache.date == 0)
		object->cache.date = time(रिक्त);

	प्रत्यागम NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
स्थिर nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	यदि (object->candidate != रिक्त) {
		llcache_object_user *user, *अग्र;

		/* Move user(s) to candidate content */
		प्रति (user = object->users; user != रिक्त; user = अग्र) {
			अग्र = user->अग्र;

			llcache_object_remove_user(object, user);
			llcache_object_सङ्कलय_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_गणनम्--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_दत्त(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		यदि (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = रिक्त;
	} अन्यथा {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = रिक्त;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_दत्त(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(शून्य) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	प्रत्यागम NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
स्थिर nserror
llcache_fetch_process_दत्त(llcache_object *object,
			   ध्रुव uint8_t *दत्त,
			   size_t len)
{
	यदि (object->fetch.state != LLCACHE_FETCH_दत्त) {
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
		दीर्घ http_संहिता = fetch_http_संहिता(object->fetch.fetch);

		यदि ((http_संहिता != 200 && http_संहिता != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_दत्त(object);
		}

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		object->fetch.state = LLCACHE_FETCH_दत्त;
	}

	/* Resize source buffer if it's too small */
	यदि (object->source_len + len >= object->source_alloc) {
		ध्रुव size_t new_len = object->source_len + len + 64 * 1024;
		uint8_t *अस्थायी = realloc(object->source_दत्त, new_len);
		यदि (अस्थायी == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		object->source_दत्त = अस्थायी;
		object->source_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	स्मृतिं_प्रतिलिख(object->source_दत्त + object->source_len, दत्त, len);
	object->source_len += len;

	प्रत्यागम NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
स्थिर nserror llcache_fetch_auth(llcache_object *object, ध्रुव वर्ण *realm)
{
	ध्रुव वर्ण *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = रिक्त;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_दत्त(object);

	/* Destroy headers */
	llcache_destroy_शीर्षकाणि(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	यदि (realm == रिक्त)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	यदि (auth == रिक्त || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.दत्त.error.संहिता = NSERROR_BAD_AUTH;
		event.दत्त.error.सन्देशः = realm;

		error = llcache_send_event_गन्तव्यम्_users(object, &event);
	} अन्यथा {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	प्रत्यागम error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = रिक्त;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_दत्त(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	यदि (object->fetch.hsts_अन्तर्_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.दत्त.error.संहिता = NSERROR_BAD_CERTS;
		event.दत्त.error.सन्देशः = messages_get("FetchFailed");

		error = llcache_send_event_गन्तव्यम्_users(object, &event);
	} अन्यथा {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.दत्त.error.संहिता = NSERROR_UNKNOWN;
		event.दत्त.error.सन्देशः = messages_get("FetchFailed");

		error = llcache_send_event_गन्तव्यम्_users(object, &event);
	}

	प्रत्यागम error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = रिक्त;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_दत्त(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	यदि (object->fetch.hsts_अन्तर्_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.दत्त.error.संहिता = NSERROR_UNKNOWN;
		event.दत्त.error.सन्देशः = messages_get("FetchFailed");

		error = llcache_send_event_गन्तव्यम्_users(object, &event);
	} अन्यथा {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	प्रत्यागम error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
स्थिर nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	यदि (object->fetch.retries_remaining > 1) {
		object->fetch.retries_remaining--;
		प्रत्यागम llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = रिक्त;

	/* Release candidate, if any */
	यदि (object->candidate != रिक्त) {
		object->candidate->candidate_गणनम्--;
		object->candidate = रिक्त;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_दत्त(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.दत्त.error.संहिता = NSERROR_TIMEOUT;
	event.दत्त.error.सन्देशः = रिक्त;

	प्रत्यागम llcache_send_event_गन्तव्यम्_users(object, &event);
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
स्थिर nserror
build_candidate_list(संरचना llcache_object ***lst_बहिर्गमनम्, पूर्णाङ्क *lst_len_बहिर्गमनम्)
{
	llcache_object *object, *अग्र;
	संरचना llcache_object **lst;
	पूर्णाङ्क lst_len = 0;
	पूर्णाङ्क remaining_lifetime;

#परिभाषय MAX_PERSIST_प्रत्येकम्_सततम् 128

	lst = शून्यस्मृतिं_देहि(MAX_PERSIST_प्रत्येकम्_सततम्, परिमाण(संरचना llcache_object *));
	यदि (lst == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रति (object = llcache->cached_objects; object != रिक्त; object = अग्र) {
		अग्र = object->अग्र;

		/* Only consider http(s) for the disc cache. */
		यदि (!llcache__scheme_is_persistable(object->url)) {
			अनुवर्त;
		}

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		यदि ((object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (remaining_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			यदि (lst_len == MAX_PERSIST_प्रत्येकम्_सततम्)
				भङ्ग;
		}
	}

	यदि (lst_len == 0) {
		मुक्त(lst);
		प्रत्यागम NSERROR_NOT_प्राप्तम्;
	}

	/** \todo sort list here */

	*lst_len_बहिर्गमनम् = lst_len;
	*lst_बहिर्गमनम् = lst;

#अपरिभाषय MAX_PERSIST_प्रत्येकम्_सततम्

	प्रत्यागम NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
स्थिर nserror
write_backing_store(संरचना llcache_object *object,
		    size_t *written_बहिर्गमनम्,
		    अचिह्नित दीर्घ *elapsed)
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
				   object->source_दत्त,
				   object->source_len);
	यदि (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		प्रत्यागम ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	यदि (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		प्रत्यागम ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	यदि (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		प्रत्यागम ret;
	}
	nsu_getmonotonic_ms(&endms);

	object->store_state = LLCACHE_STATE_DISC;

	*written_बहिर्गमनम् = object->source_len + metadatasize;

	/* by ignoring the overflow this assumes the writeout took
	 * less than 5 weeks.
	 */
	*elapsed = endms - startms;

	/* ensure the writeout is reported to have taken at least the
	 * minimal amount of time
	 */
	यदि (*elapsed == 0) {
		*elapsed = 1;
	}

	प्रत्यागम NSERROR_OK;
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
स्थिर शून्य llcache_persist_slowcheck(शून्य *p)
{
	uint64_t योग_bandwidth; /* total bandwidth */

	यदि (llcache->योग_written > (2 * llcache->minimum_bandwidth)) {

		योग_bandwidth = (llcache->योग_written * 1000) / llcache->योग_elapsed;

		यदि (योग_bandwidth < llcache->minimum_bandwidth) {
			NSLOG(llcache, INFO,
			      "Current bandwidth %"PRIu64" less than minimum %"PRIsizet,
			      योग_bandwidth,
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
स्थिर शून्य llcache_persist(शून्य *p)
{
	nserror ret;
	संरचना llcache_object **lst; /* candidate object list */
	पूर्णाङ्क lst_गणनम्; /* number of candidates in list */
	पूर्णाङ्क idx; /* current candidate object index in list */
	पूर्णाङ्क अग्र = -1; /* when the next run should be scheduled for */

	अचिह्नित दीर्घ write_सीमा; /* max number of bytes to write in this run*/

	size_t written; /* all bytes written for a single object */
	अचिह्नित दीर्घ elapsed; /* how long writing an object took */

	size_t योग_written = 0; /* total bytes written in this run */
	अचिह्नित दीर्घ योग_elapsed = 1; /* total ms used to write bytes */
	अचिह्नित दीर्घ योग_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_गणनम्);
	यदि (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		प्रत्यागम;
	}

	write_सीमा = (llcache->अधिकतमम्_bandwidth * llcache->time_कालांशः) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	प्रति (idx = 0; idx < lst_गणनम्; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		यदि (ret != NSERROR_OK) {
			अनुवर्त;
		}

		/* successfully wrote object to backing store */
		योग_written += written;
		योग_elapsed += elapsed;
		योग_bandwidth = (योग_written * 1000) / योग_elapsed;

		NSLOG(llcache, DEBUG,
		      "Wrote %"PRIsizet" bytes in %lums bw:%lu %s",
		      written, elapsed, (written * 1000) / elapsed,
		      nsurl_access(lst[idx]->url) );

		/* check to for the time quantum or the size
		 * (bandwidth) for this run being exceeded.
		 */
		यदि (योग_elapsed > llcache->time_कालांशः) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			यदि (योग_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->कालसारणी(
						llcache->time_कालांशः * 100,
						llcache_persist_slowcheck,
						रिक्त);
				भङ्ग;
			} अन्यथा {
				यदि (योग_bandwidth > llcache->अधिकतमम्_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					अग्र = ((योग_written * llcache->time_कालांशः) / write_सीमा) - योग_elapsed;
				} अन्यथा {
					अग्र = llcache->time_कालांशः;
				}
				भङ्ग;
			}
		} अन्यथा यदि (योग_written > write_सीमा) {
			/* The bandwidth limit has been reached. */

			यदि (योग_bandwidth > llcache->अधिकतमम्_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				अग्र = ((योग_written * llcache->time_कालांशः) / write_सीमा) - योग_elapsed;
			} अन्यथा {
				अग्र = llcache->time_कालांशः - योग_elapsed;
			}
			भङ्ग;
		}

	}
	मुक्त(lst);

	/* Completed list without running out of allowed bytes or time */
	यदि (idx == lst_गणनम्) {
		/* only reschedule if writing is making any progress at all */
		यदि (योग_written > 0) {
			अग्र = llcache->time_कालांशः - योग_elapsed;
		} अन्यथा {
			अग्र = -1;
		}
	}

	llcache->योग_written += योग_written;
	llcache->योग_elapsed += योग_elapsed;

	NSLOG(llcache, DEBUG,
	      "writeout size:%"PRIsizet" time:%lu bandwidth:%lubytes/s",
	      योग_written, योग_elapsed, योग_bandwidth);

	NSLOG(llcache, DEBUG, "Rescheduling writeout in %dms", अग्र);
	guit->misc->कालसारणी(अग्र, llcache_persist, रिक्त);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
स्थिर शून्य llcache_fetch_callback(ध्रुव fetch_सन्देशः *सन्देशः, शून्य *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	यदि (llcache == रिक्त) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		प्रतिज्ञा(false);
		/* In case assertions are off, return here */
		प्रत्यागम;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", सन्देशः->type, object);

	विकल्प (सन्देशः->type) {
	पक्ष FETCH_शीर्षकम्:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_शीर्षकाणि;

		error = llcache_fetch_process_शीर्षकम्(object,
				सन्देशः->दत्त.शीर्षकम्_or_दत्त.buf,
				सन्देशः->दत्त.शीर्षकम्_or_दत्त.len);
		भङ्ग;

	/* 3xx responses */
	पक्ष FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		error = llcache_fetch_redirect(object,
				सन्देशः->दत्त.redirect, &object);
		यदि (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.दत्त.error.संहिता = error;
			event.दत्त.error.सन्देशः = रिक्त;
			error = llcache_send_event_गन्तव्यम्_users(object, &event);
		}
		भङ्ग;

	पक्ष FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		भङ्ग;

	/* Normal 2xx state machine */
	पक्ष FETCH_दत्त:
		/* Received some data */
		error = llcache_fetch_process_दत्त(object,
				सन्देशः->दत्त.शीर्षकम्_or_दत्त.buf,
				सन्देशः->दत्त.शीर्षकम्_or_दत्त.len);
		भङ्ग;

	पक्ष FETCH_परिसमाप्तम्:
		/* Finished fetching */
	{
		uint8_t *अस्थायी;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = रिक्त;

		/* Shrink source buffer to required size */
		अस्थायी = realloc(object->source_दत्त,
				object->source_len);
		/* If source_len is 0, then temp may be NULL */
		यदि (अस्थायी != रिक्त || object->source_len == 0) {
			object->source_दत्त = अस्थायी;
			object->source_alloc = object->source_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_time = time(रिक्त);

		(शून्य) llcache_hsts_update_policy(object);

		guit->misc->कालसारणी(5000, llcache_persist, रिक्त);
	}
		भङ्ग;

	/* Out-of-band information */
	पक्ष FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		भङ्ग;

	पक्ष FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = रिक्त;

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_दत्त(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.दत्त.error.संहिता = NSERROR_UNKNOWN;
		event.दत्त.error.सन्देशः = सन्देशः->दत्त.error;

		error = llcache_send_event_गन्तव्यम्_users(object, &event);

		भङ्ग;

	पक्ष FETCH_प्रगतिः:
		/* Progress update */
		event.type = LLCACHE_EVENT_प्रगतिः;
		event.दत्त.प्रगतिः_सन्देशः = सन्देशः->दत्त.प्रगतिः;

		error = llcache_send_event_गन्तव्यम्_users(object, &event);

		भङ्ग;

	पक्ष FETCH_CERTS:
		/* Certificate information from the fetch */

		यदि (object->chain != रिक्त) {
			cert_chain_free(object->chain);
			object->chain = रिक्त;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(सन्देशः->दत्त.chain, &object->chain);
		यदि (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.दत्त.chain = सन्देशः->दत्त.chain;

		error = llcache_send_event_गन्तव्यम्_users(object, &event);

		भङ्ग;

	/* Events requiring action */
	पक्ष FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		error = llcache_fetch_auth(object, सन्देशः->दत्त.auth.realm);
		भङ्ग;

	पक्ष FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		error = llcache_fetch_cert_error(object);
		भङ्ग;

	पक्ष FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		यदि (object->candidate != रिक्त) {
			object->candidate->candidate_गणनम्--;
			object->candidate = रिक्त;
		}

		error = llcache_fetch_ssl_error(object);
		भङ्ग;
	}

	/* Deal with any errors reported by event handlers */
	यदि (error != NSERROR_OK) {
		यदि (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		यदि (object->fetch.fetch != रिक्त) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = रिक्त;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_दत्त(object);

			object->fetch.state = LLCACHE_FETCH_COMPLETE;
		}
	}

	/* There may be users which are not caught up so schedule ourselves */
	llcache_users_not_caught_ऊर्ध्वम्();
}

/**
 * Find a user of a low-level cache object
 *
 * \param handle  External cache handle to search for
 * \return Pointer to corresponding user, or NULL if not found
 */
स्थिर llcache_object_user *llcache_object_find_user(
		ध्रुव llcache_handle *handle)
{
	llcache_object_user *user;

	प्रतिज्ञा(handle->object != रिक्त);

	प्रति (user = handle->object->users; user != रिक्त; user = user->अग्र) {
		यदि (user->handle == handle)
			भङ्ग;
	}

	प्रत्यागम user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
स्थिर bool llcache_object_अन्तर्_list(ध्रुव llcache_object *object,
		ध्रुव llcache_object *list)
{
	यावत् (list != रिक्त) {
		यदि (list == object)
			भङ्ग;

		list = list->अग्र;
	}

	प्रत्यागम list != रिक्त;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror llcache_object_notify_users(llcache_object *object)
{
	nserror error;
	llcache_object_user *user, *अग्र_user;
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

	प्रति (user = object->users; user != रिक्त; user = अग्र_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		ध्रुव llcache_fetch_state objstate = object->fetch.state;

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

		यदि (handle->state != objstate) {
			यदि (emitted_notify == false) {
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
		यदि (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_शीर्षकाणि;

			/* Emit any certificate data we hold */
			यदि (object->chain != रिक्त) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.दत्त.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} अन्यथा {
				error = NSERROR_OK;
			}

			यदि (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				यदि (error != NSERROR_OK)
					प्रत्यागम error;

				अनुवर्त;
			} अन्यथा यदि (error == NSERROR_NEED_दत्त) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_शीर्षकाणि;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_ऊर्ध्वम्();
				अनुवर्त;
			} अन्यथा यदि (error != NSERROR_OK) {
				user->iterator_target = false;
				प्रत्यागम error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		यदि (handle->state == LLCACHE_FETCH_शीर्षकाणि &&
				objstate > LLCACHE_FETCH_शीर्षकाणि) {
			handle->state = LLCACHE_FETCH_दत्त;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_शीर्षकाणि;

			error = handle->cb(handle, &event, handle->pw);

			यदि (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				यदि (error != NSERROR_OK)
					प्रत्यागम error;

				अनुवर्त;
			} अन्यथा यदि (error == NSERROR_NEED_दत्त) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_शीर्षकाणि;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_ऊर्ध्वम्();
				अनुवर्त;
			} अन्यथा यदि (error != NSERROR_OK) {
				user->iterator_target = false;
				प्रत्यागम error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		यदि (handle->state == LLCACHE_FETCH_दत्त &&
				objstate >= LLCACHE_FETCH_दत्त &&
				object->source_len > handle->अष्टकानि) {
			size_t orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_दत्त;
			event.दत्त.दत्त.buf =
					object->source_दत्त + handle->अष्टकानि;
			event.दत्त.दत्त.len =
					object->source_len - handle->अष्टकानि;

			/* Update record of last byte emitted */
			यदि (object->fetch.flags &
					LLCACHE_RETRIEVE_प्रवाहः_दत्त) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->अष्टकानि = object->source_len = 0;
			} अन्यथा {
				orig_handle_read = handle->अष्टकानि;
				handle->अष्टकानि = object->source_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			यदि (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				यदि (error != NSERROR_OK)
					प्रत्यागम error;

				अनुवर्त;
			} अन्यथा यदि (error == NSERROR_NEED_दत्त) {
				/* User requested replay */
				handle->अष्टकानि = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_ऊर्ध्वम्();
				अनुवर्त;
			} अन्यथा यदि (error != NSERROR_OK) {
				user->iterator_target = false;
				प्रत्यागम error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		यदि (handle->state == LLCACHE_FETCH_दत्त &&
				objstate > LLCACHE_FETCH_दत्त) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_समाप्तम्;

			error = handle->cb(handle, &event, handle->pw);
			यदि (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				यदि (error != NSERROR_OK)
					प्रत्यागम error;

				अनुवर्त;
			} अन्यथा यदि (error == NSERROR_NEED_दत्त) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_दत्त;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_ऊर्ध्वम्();
				अनुवर्त;
			} अन्यथा यदि (error != NSERROR_OK) {
				user->iterator_target = false;
				प्रत्यागम error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_target = false;

		अग्र_user = user->अग्र;
	}

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	newobj->source_alloc = newobj->source_len = object->source_len;

	यदि (object->source_len > 0) {
		newobj->source_दत्त = स्मृतिं_देहि(newobj->source_alloc);
		यदि (newobj->source_दत्त == रिक्त) {
			llcache_object_destroy(newobj);
			प्रत्यागम NSERROR_NOMEM;
		}
		स्मृतिं_प्रतिलिख(newobj->source_दत्त, object->source_दत्त,
				newobj->source_len);
	}

	यदि (object->अङ्कः_शीर्षकाणि > 0) {
		newobj->शीर्षकाणि = शून्यस्मृतिं_देहि(object->अङ्कः_शीर्षकाणि,
				परिमाण(llcache_शीर्षकम्));
		यदि (newobj->शीर्षकाणि == रिक्त) {
			llcache_object_destroy(newobj);
			प्रत्यागम NSERROR_NOMEM;
		}
		यावत् (newobj->अङ्कः_शीर्षकाणि < object->अङ्कः_शीर्षकाणि) {
			llcache_शीर्षकम् *nh =
					&(newobj->शीर्षकाणि[newobj->अङ्कः_शीर्षकाणि]);
			llcache_शीर्षकम् *oh =
					&(object->शीर्षकाणि[newobj->अङ्कः_शीर्षकाणि]);
			newobj->अङ्कः_शीर्षकाणि += 1;
			nh->name = strdup(oh->name);
			nh->मानम् = strdup(oh->मानम्);
			यदि (nh->name == रिक्त || nh->मानम् == रिक्त) {
				llcache_object_destroy(newobj);
				प्रत्यागम NSERROR_NOMEM;
			}
		}
	}

	यदि (object->chain != रिक्त) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		यदि (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			प्रत्यागम error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	प्रत्यागम NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
स्थिर अन्तःस्थ uint32_t
योग_object_आकार(llcache_object *object)
{
	uint32_t tot;
	size_t hdrc;

	tot = परिमाण(*object);
	tot += nsurl_दीर्घता(object->url);

	यदि (object->source_दत्त != रिक्त) {
		tot += object->source_len;
	}

	tot += परिमाण(llcache_शीर्षकम्) * object->अङ्कः_शीर्षकाणि;

	प्रति (hdrc = 0; hdrc < object->अङ्कः_शीर्षकाणि; hdrc++) {
		यदि (object->शीर्षकाणि[hdrc].name != रिक्त) {
			tot += वाक्यदैर्घ्यम्(object->शीर्षकाणि[hdrc].name);
		}
		यदि (object->शीर्षकाणि[hdrc].मानम् != रिक्त) {
			tot += वाक्यदैर्घ्यम्(object->शीर्षकाणि[hdrc].मानम्);
		}
	}

	tot += cert_chain_आकार(object->chain);

	प्रत्यागम tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
स्थिर शून्य llcache_catch_ऊर्ध्वम्_all_users(शून्य *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_ऊर्ध्वम् = true;

	/* Catch new users up with state of objects */
	प्रति (object = llcache->cached_objects; object != रिक्त;
			object = object->अग्र) {
		llcache_object_notify_users(object);
	}

	प्रति (object = llcache->uncached_objects; object != रिक्त;
			object = object->अग्र) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
स्थिर शून्य llcache_users_not_caught_ऊर्ध्वम्(शून्य)
{
	यदि (llcache->all_caught_ऊर्ध्वम्) {
		llcache->all_caught_ऊर्ध्वम् = false;
		guit->misc->कालसारणी(0, llcache_catch_ऊर्ध्वम्_all_users, रिक्त);
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
शून्य llcache_clean(bool purge)
{
	llcache_object *object, *अग्र;
	uint32_t llcache_आकार = 0;
	पूर्णाङ्क remaining_lifetime;
	uint32_t सीमा;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	यदि (purge) {
		सीमा = 0;
	} अन्यथा {
		सीमा = llcache->सीमा;
	}

	/* Uncacheable objects with no users or fetches */
	प्रति (object = llcache->uncached_objects;
	     object != रिक्त;
	     object = अग्र) {
		अग्र = object->अग्र;

		/* The candidate count of uncacheable objects is always 0 */
		यदि ((object->users == रिक्त) &&
		    (object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त)) {
			NSLOG(llcache, DEBUG, "Discarding uncachable object with no users (%p) %s",
				    object, nsurl_access(object->url));

			llcache_object_remove_from_list(object,
					&llcache->uncached_objects);
			llcache_object_destroy(object);
		} अन्यथा {
			llcache_आकार += योग_object_आकार(object);
		}
	}


	/* Stale cacheable objects with no users or pending fetches */
	प्रति (object = llcache->cached_objects;
	     object != रिक्त;
	     object = अग्र) {
		अग्र = object->अग्र;

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		यदि ((object->users == रिक्त) &&
		    (object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त) &&
		    (remaining_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				यदि (object->store_state == LLCACHE_STATE_DISC) {
					guit->llcache->invalidate(object->url);
				}

				llcache_object_destroy(object);

		} अन्यथा {
			/* object has users so account for the storage */
			llcache_आकार += योग_object_आकार(object);
		}
	}

	/* if the cache limit is exceeded try to make some objects
	 * persistent so their RAM can be reclaimed in the next
	 * step
	 */
	यदि (सीमा < llcache_आकार) {
		llcache_persist(रिक्त);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	प्रति (object = llcache->cached_objects;
	     ((सीमा < llcache_आकार) && (object != रिक्त));
	     object = अग्र) {
		अग्र = object->अग्र;
		यदि ((object->users == रिक्त) &&
		    (object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->source_दत्त = रिक्त;

			llcache_आकार -=	object->source_len;

			NSLOG(llcache, DEBUG,
			      "Freeing source data for %p len:%"PRIsizet,
			      object, object->source_len);
		}
	}

	/* Fresh cacheable objects with no users, no pending fetches
	 * and pushed to persistent store while the cache exceeds
	 * the configured size. Effectively just the llcache object metadata.
	 */
	प्रति (object = llcache->cached_objects;
	     ((सीमा < llcache_आकार) && (object != रिक्त));
	     object = अग्र) {
		अग्र = object->अग्र;
		यदि ((object->users == रिक्त) &&
		    (object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->source_दत्त == रिक्त)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (दीर्घ)(time(रिक्त) - object->चरमः_प्रयुक्तम्),
			      object,
			      nsurl_access(object->url));

			llcache_आकार -=	योग_object_आकार(object);

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
	प्रति (object = llcache->cached_objects;
	     ((सीमा < llcache_आकार) && (object != रिक्त));
	     object = अग्र) {
		अग्र = object->अग्र;

		यदि ((object->users == रिक्त) &&
		    (object->candidate_गणनम् == 0) &&
		    (object->fetch.fetch == रिक्त) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (दीर्घ)(time(रिक्त) - object->चरमः_प्रयुक्तम्),
			      object,
			      nsurl_access(object->url));

			llcache_आकार -=	object->source_len + परिमाण(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_आकार, सीमा);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(ध्रुव संरचना llcache_parameters *prm)
{
	llcache = शून्यस्मृतिं_देहि(1, परिमाण(संरचना llcache_s));
	यदि (llcache == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	llcache->सीमा = prm->सीमा;
	llcache->minimum_lifetime = prm->minimum_lifetime;
	llcache->minimum_bandwidth = prm->minimum_bandwidth;
	llcache->अधिकतमम्_bandwidth = prm->अधिकतमम्_bandwidth;
	llcache->time_कालांशः = prm->time_कालांशः;
	llcache->fetch_attempts = prm->fetch_attempts;
	llcache->all_caught_ऊर्ध्वम् = true;

	NSLOG(llcache, INFO,
	      "llcache initialising with a limit of %"PRIu32" bytes",
	      llcache->सीमा);

	/* backing store initialisation */
	प्रत्यागम guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
शून्य llcache_finalise(शून्य)
{
	llcache_object *object, *अग्र;
	uint64_t योग_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(रिक्त);
	/* Now clear the persistence callback */
	guit->misc->कालसारणी(-1, llcache_persist, रिक्त);

	/* Clean uncached objects */
	प्रति (object = llcache->uncached_objects; object != रिक्त; object = अग्र) {
		llcache_object_user *user, *अग्र_user;

		अग्र = object->अग्र;

		प्रति (user = object->users; user != रिक्त; user = अग्र_user) {
			अग्र_user = user->अग्र;

			यदि (user->handle != रिक्त)
				मुक्त(user->handle);

			मुक्त(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = रिक्त;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	प्रति (object = llcache->cached_objects; object != रिक्त; object = अग्र) {
		llcache_object_user *user, *अग्र_user;

		अग्र = object->अग्र;

		प्रति (user = object->users; user != रिक्त; user = अग्र_user) {
			अग्र_user = user->अग्र;

			यदि (user->handle != रिक्त)
			       मुक्त(user->handle);

			मुक्त(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = रिक्त;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	यदि (llcache->योग_elapsed > 0) {
		योग_bandwidth = (llcache->योग_written * 1000) /
			llcache->योग_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->योग_written,
	      llcache->योग_elapsed,
	      योग_bandwidth);

	मुक्त(llcache);
	llcache = रिक्त;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			ध्रुव llcache_post_दत्त *post,
			llcache_handle_callback cb, शून्य *pw,
			llcache_handle **परिणाम)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_अन्तर्_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_अन्तर्_use);
	यदि (error != NSERROR_OK) {
		प्रत्यागम error;
	}

	/* Can we fetch this URL at all? */
	यदि (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		प्रत्यागम NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	यदि (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		प्रत्यागम error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_अन्तर्_use, &object);
	यदि (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		प्रत्यागम error;
	}

	/* Add user to object */
	llcache_object_सङ्कलय_user(object, user);

	*परिणाम = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_ऊर्ध्वम्();

	nsurl_unref(hsts_url);

	प्रत्यागम NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, शून्य *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	प्रत्यागम NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_find_user(handle);

	प्रतिज्ञा(user != रिक्त);

	यदि (user->iterator_target) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} अन्यथा {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		यदि (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	प्रत्यागम error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **परिणाम)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	यदि (error == NSERROR_OK) {
		llcache_object_सङ्कलय_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*परिणाम = newuser->handle;
	}

	प्रत्यागम error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	यदि (user->पूर्व != रिक्त)
		all_alone = false;
	यदि (user->अग्र != रिक्त)
		all_alone = false;

	यदि (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		यदि (error != NSERROR_OK)
			प्रत्यागम error;

		/* Move across to the new object */
		यदि (user->iterator_target) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					शून्यस्मृतिं_देहि(1, परिमाण(llcache_object_user));
			यदि (newuser == रिक्त) {
				llcache_object_destroy(newobject);
				प्रत्यागम NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = रिक्त;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_सङ्कलय_user(newobject, newuser);
		} अन्यथा {
			llcache_object_remove_user(object, user);
			llcache_object_सङ्कलय_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_सङ्कलय_गन्तव्यम्_list(newobject,
				&llcache->uncached_objects);
	} अन्यथा {
		/* We're the only user, so abort any fetch in progress */
		यदि (object->fetch.fetch != रिक्त) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = रिक्त;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_दत्त(object);
	}

	प्रत्यागम error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_प्रवाहः(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	यदि (user->पूर्व != रिक्त || user->अग्र != रिक्त)
		प्रत्यागम NSERROR_OK;

	/* Forcibly uncache this object */
	यदि (llcache_object_अन्तर्_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_सङ्कलय_गन्तव्यम्_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_प्रवाहः_दत्त;

	प्रत्यागम NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_दत्त(llcache_handle *handle)
{
	यदि ((handle->object != रिक्त) &&
	    (handle->object->fetch.fetch == रिक्त) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	प्रत्यागम NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(ध्रुव llcache_handle *handle)
{
	प्रत्यागम handle->object != रिक्त ? handle->object->url : रिक्त;
}

/* See llcache.h for documentation */
ध्रुव uint8_t *llcache_handle_get_source_दत्त(ध्रुव llcache_handle *handle,
		size_t *आकार)
{
	*आकार = handle->object != रिक्त ? handle->object->source_len : 0;

	प्रत्यागम handle->object != रिक्त ? handle->object->source_दत्त : रिक्त;
}

/* See llcache.h for documentation */
ध्रुव वर्ण *llcache_handle_get_शीर्षकम्(ध्रुव llcache_handle *handle,
		ध्रुव वर्ण *कुञ्चिका)
{
	ध्रुव llcache_object *object = handle->object;
	size_t i;

	यदि (object == रिक्त)
		प्रत्यागम रिक्त;

	/* About as trivial as possible */
	प्रति (i = 0; i < object->अङ्कः_शीर्षकाणि; i++) {
		यदि (strcasecmp(कुञ्चिका, object->शीर्षकाणि[i].name) == 0)
			प्रत्यागम object->शीर्षकाणि[i].मानम्;
	}

	प्रत्यागम रिक्त;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(ध्रुव llcache_handle *a,
		ध्रुव llcache_handle *b)
{
	प्रत्यागम a->object == b->object;
}
