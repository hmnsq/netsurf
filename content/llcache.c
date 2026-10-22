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

#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <nsutils/time.h>
#समावेश <nsutils/base64.h>

#समावेश "netsurf/inttypes.h"
#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/utils.h"
#समावेश "utils/time.h"
#समावेश "utils/http.h"
#समावेश "utils/nsoption.h"
#समावेश "netsurf/misc.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/fetch.h"
#समावेश "content/backing_store.h"
#समावेश "content/urldb.h"

/**
 * State of a low-level cache object fetch.
 */
प्रकार क्रमागत {
	LLCACHE_FETCH_INIT,		/**< Initial state, before fetch */
	LLCACHE_FETCH_HEADERS,		/**< Fetching headers */
	LLCACHE_FETCH_तथ्य,		/**< Fetching object data */
	LLCACHE_FETCH_COMPLETE		/**< Fetch completed */
} llcache_fetch_state;

/**
 * Type of low-level cache object.
 */
प्रकार काष्ठा llcache_object llcache_object;

/**
 * Handle to low-level cache object.
 */
काष्ठा llcache_handle {
	llcache_object *object;		/**< Pointer to associated object */

	llcache_handle_callback cb;	/**< Client callback */
	व्योम *pw;			/**< Client data */

	llcache_fetch_state state;	/**< Last known state of object fetch */
	माप_प्रकार bytes;			/**< Last reported byte count */
};

/**
 * Low-level cache object user record.
 */
प्रकार काष्ठा llcache_object_user {
	llcache_handle *handle;		/**< Handle data for client */

	bool iterator_target;		/**< This is the iterator target */
	bool queued_for_delete;		/**< This user is queued for deletion */

	काष्ठा llcache_object_user *पूर्व;	/**< Previous in list */
	काष्ठा llcache_object_user *अग्र;	/**< Next in list */
} llcache_object_user;

/**
 * Low-level cache object fetch context.
 */
प्रकार काष्ठा {
	uint32_t flags;			/**< Fetch flags */
	nsurl *referer;			/**< Referring URL, or NULL if none */
	llcache_post_तथ्य *post;	/**< POST data, or NULL for GET */

	काष्ठा fetch *fetch;		/**< Fetch handle for this object */

	llcache_fetch_state state;	/**< Current state of object fetch */

	uint32_t redirect_गिनती;	/**< Count of redirects followed */

	uint32_t retries_remaining;     /**< Number of times to retry on timeout */

	bool hsts_in_use;		/**< Whether HSTS applies to this fetch */

	bool tried_with_auth;		/**< Whether we've tried with auth */

	bool tried_with_tls_downgrade;	/**< Whether we've tried TLS 1.2 */

	bool tainted_tls;		/**< Whether the TLS transport is tainted */
} llcache_fetch_ctx;

/**
 * Validation control.
 */
प्रकार क्रमागत {
	LLCACHE_VALIDATE_FRESH,		/**< Only revalidate if not fresh */
	LLCACHE_VALIDATE_ALWAYS,	/**< Always revalidate */
	LLCACHE_VALIDATE_ONCE		/**< Revalidate once only */
} llcache_validate;

/**
 * Cache control value for invalid age.
 */
#घोषणा INVALID_AGE -1

/** Cache control data */
प्रकार काष्ठा {
	समय_प्रकार req_time;	/**< Time of request */
	समय_प्रकार res_time;	/**< Time of response */
	समय_प्रकार fin_time;	/**< Time of request completion */
	समय_प्रकार date;		/**< Date: response header */
	समय_प्रकार expires;		/**< Expires: response header */
	पूर्णांक age;		/**< Age: response header */
	पूर्णांक max_age;		/**< Max-Age Cache-control parameter */
	llcache_validate no_cache;	/**< No-Cache Cache-control parameter */
	अक्षर *etag;		/**< Etag: response header */
	समय_प्रकार last_modified;	/**< Last-Modified: response header */
} llcache_cache_control;

/** Representation of a fetch header */
प्रकार काष्ठा {
	अक्षर *name;		/**< Header name */
	अक्षर *मूल्य;		/**< Header value */
} llcache_header;

/** Current status of an object's data */
प्रकार क्रमागत {
	LLCACHE_STATE_RAM = 0, /**< source data is stored in RAM only */
	LLCACHE_STATE_DISC, /**< source data is stored on disc */
} llcache_store_state;

/**
 * Low-level cache object
 *
 * \todo Consider whether a list is a sane container.
 */
काष्ठा llcache_object {
	llcache_object *पूर्व;	     /**< Previous in list */
	llcache_object *अग्र;	     /**< Next in list */

	nsurl *url;		     /**< Post-redirect URL for object */

	/** \todo We need a generic dynamic buffer object */
	uint8_t *source_तथ्य;	     /**< Source data for object */
	माप_प्रकार source_len;	     /**< Byte length of source data */
	माप_प्रकार source_alloc;	     /**< Allocated size of source buffer */

	काष्ठा cert_chain *chain;    /**< Certificate chain from the fetch */

	llcache_store_state store_state; /**< where the data for the object is stored */

	llcache_object_user *users;  /**< List of users */

	llcache_fetch_ctx fetch;     /**< Fetch context for object */

	llcache_cache_control cache; /**< Cache control data for object */
	llcache_object *candidate;   /**< Object to use, if fetch determines
				      * that it is still fresh
				      */
	uint32_t candidate_गिनती;    /**< Count of objects this is a
				      * candidate for
				      */

	llcache_header *headers;     /**< Fetch headers */
	माप_प्रकार अंक_headers;	     /**< Number of fetch headers */

	/* Instrumentation. These elements are strictly for information
	 * to improve the cache performance and to provide performance
	 * metrics. The values are non-authoritative and must not be used to
	 * determine object lifetime etc.
	 */
	समय_प्रकार last_used; /**< time the last user was removed from the object */
};

/**
 * Core llcache control context.
 */
काष्ठा llcache_s {
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
	पूर्णांक minimum_lifetime;

	/**
	 * The time over which to apply the bandwidth calculations in ms
	 */
	अचिन्हित दीर्घ time_quantum;

	/**
	 * The minimum bandwidth to allow the backing store to use in
	 * bytes/second. Below this the backing store will be
	 * disabled.
	 */
	माप_प्रकार minimum_bandwidth;

	/**
	 * The maximum bandwidth to allow the backing store to use in
	 * bytes/second
	 */
	माप_प्रकार maximum_bandwidth;

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
अटल काष्ठा llcache_s *llcache = शून्य;

/* forward referenced callback function */
अटल व्योम llcache_fetch_callback(स्थिर fetch_संदेश *संदेश, व्योम *p);

/* forward referenced catch up function */
अटल व्योम llcache_users_not_caught_up(व्योम);


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
अटल nserror llcache_object_user_new(llcache_handle_callback cb, व्योम *pw,
		llcache_object_user **user)
{
	llcache_handle *h;
	llcache_object_user *u;

	h = सुस्मृति(1, माप(llcache_handle));
	अगर (h == शून्य) {
		वापस NSERROR_NOMEM;
	}

	u = सुस्मृति(1, माप(llcache_object_user));
	अगर (u == शून्य) {
		मुक्त(h);
		वापस NSERROR_NOMEM;
	}

	h->cb = cb;
	h->pw = pw;

	u->handle = h;

	NSLOG(llcache, DEBUG,
	      "Created user %p (%p, %p, %p)", u, h, (व्योम *) cb, pw);

	*user = u;

	वापस NSERROR_OK;
}

/**
 * Destroy an object user
 *
 * \param user	User to destroy
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \pre User is not attached to an object
 */
अटल nserror llcache_object_user_destroy(llcache_object_user *user)
{
	NSLOG(llcache, DEBUG, "Destroyed user %p", user);

	निश्चित(user->अग्र == शून्य);
	निश्चित(user->पूर्व == शून्य);

	अगर (user->handle != शून्य)
		मुक्त(user->handle);

	मुक्त(user);

	वापस NSERROR_OK;
}

/**
 * Remove a user from a low-level cache object
 *
 * \param object  Object to remove user from
 * \param user	  User to remove
 * \return NSERROR_OK.
 */
अटल nserror llcache_object_remove_user(llcache_object *object,
		llcache_object_user *user)
{
	निश्चित(user != शून्य);
	निश्चित(object != शून्य);
	निश्चित(object->users != शून्य);
	निश्चित(user->handle == शून्य || user->handle->object == object);
	निश्चित((user->पूर्व != शून्य) || (object->users == user));

	अगर (user == object->users)
		object->users = user->अग्र;
	अन्यथा
		user->पूर्व->अग्र = user->अग्र;

	अगर (user->अग्र != शून्य)
		user->अग्र->पूर्व = user->पूर्व;

	user->अग्र = user->पूर्व = शून्य;

	/* record the time the last user was removed from the object */
	अगर (object->users == शून्य) {
		object->last_used = समय(शून्य);
	}

	NSLOG(llcache, DEBUG, "Removing user %p from %p", user, object);

	वापस NSERROR_OK;
}

/**
 * Iterate the users of an object, calling their callbacks.
 *
 * \param object	The object to iterate
 * \param event		The event to pass to the callback.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
अटल nserror llcache_send_event_to_users(llcache_object *object,
					   llcache_event *event)
{
	nserror error = NSERROR_OK;
	llcache_object_user *user, *अग्र_user;

	user = object->users;
	जबतक (user != शून्य) {
		bool was_target = user->iterator_target;
		user->iterator_target = true;

		error = user->handle->cb(user->handle, event,
					user->handle->pw);

		अग्र_user = user->अग्र;

		user->iterator_target = was_target;

		अगर (user->queued_for_delete && !was_target) {
			llcache_object_remove_user(object, user);
			llcache_object_user_destroy(user);
		}

		अगर (error != NSERROR_OK)
			अवरोध;

		user = अग्र_user;
	}

	वापस error;
}

/**
 * Create a new low-level cache object
 *
 * \param url	  URL of object to create
 * \param result  Pointer to location to receive result
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_object_new(nsurl *url, llcache_object **परिणाम)
{
	llcache_object *obj = सुस्मृति(1, माप(llcache_object));
	अगर (obj == शून्य)
		वापस NSERROR_NOMEM;

	NSLOG(llcache, DEBUG, "Created object %p (%s)", obj, nsurl_access(url));

	obj->url = nsurl_ref(url);

	*परिणाम = obj;

	वापस NSERROR_OK;
}

/**
 * Clone a POST data object
 *
 * \param orig	 Object to clone
 * \param clone	 Pointer to location to receive clone
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_post_तथ्य_clone(स्थिर llcache_post_तथ्य *orig,
		llcache_post_तथ्य **clone)
{
	llcache_post_तथ्य *post_clone;

	post_clone = सुस्मृति(1, माप(llcache_post_तथ्य));
	अगर (post_clone == शून्य)
		वापस NSERROR_NOMEM;

	post_clone->type = orig->type;

	/* Deep-copy the type-specific data */
	अगर (orig->type == LLCACHE_POST_URL_ENCODED) {
		post_clone->तथ्य.urlenc = strdup(orig->तथ्य.urlenc);
		अगर (post_clone->तथ्य.urlenc == शून्य) {
			मुक्त(post_clone);

			वापस NSERROR_NOMEM;
		}
	} अन्यथा {
		post_clone->तथ्य.multipart = fetch_multipart_तथ्य_clone(
				orig->तथ्य.multipart);
		अगर (post_clone->तथ्य.multipart == शून्य) {
			मुक्त(post_clone);

			वापस NSERROR_NOMEM;
		}
	}

	*clone = post_clone;

	वापस NSERROR_OK;
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
अटल nserror llcache_fetch_split_header(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		अक्षर **name, अक्षर **मूल्य)
{
	अक्षर *n, *v;
	स्थिर uint8_t *colon;

	/* Strip leading whitespace from name */
	जबतक (तथ्य[0] == ' ' || तथ्य[0] == '\t' ||
	       तथ्य[0] == '\r' || तथ्य[0] == '\n') {
		तथ्य++;
	}

	/* Find colon */
	colon = (स्थिर uint8_t *) म_अक्षर((स्थिर अक्षर *) तथ्य, ':');
	अगर (colon == शून्य) {
		/* Failed, assume a key with no value */
		colon = तथ्य + म_माप((स्थिर अक्षर *)तथ्य);

		/* Strip trailing whitespace from name */
		जबतक ((colon > तथ्य) &&
		       (colon[-1] == ' ' || colon[-1] == '\t' ||
			colon[-1] == '\r' || colon[-1] == '\n')) {
			colon--;
		}
		n = strndup((स्थिर अक्षर *) तथ्य, colon - तथ्य);
		अगर (n == शून्य)
			वापस NSERROR_NOMEM;

		v = strdup("");
		अगर (v == शून्य) {
			मुक्त(n);
			वापस NSERROR_NOMEM;
		}
	} अन्यथा {
		/* Split header into name & value */

		/* Strip trailing whitespace from name */
		जबतक (colon > तथ्य && (colon[-1] == ' ' ||
				colon[-1] == '\t' || colon[-1] == '\r' ||
				colon[-1] == '\n'))
			colon--;

		n = strndup((स्थिर अक्षर *) तथ्य, colon - तथ्य);
		अगर (n == शून्य)
			वापस NSERROR_NOMEM;

		/* Find colon again */
		जबतक (*colon != ':') {
			colon++;
		}

		/* Skip over colon and any subsequent whitespace */
		करो {
			colon++;
		} जबतक (*colon == ' ' || *colon == '\t' ||
				*colon == '\r' || *colon == '\n');

		/* Strip trailing whitespace from value */
		जबतक (len > 0 && (तथ्य[len - 1] == ' ' ||
				तथ्य[len - 1] == '\t' ||
				तथ्य[len - 1] == '\r' ||
				तथ्य[len - 1] == '\n')) {
			len--;
		}

		v = strndup((स्थिर अक्षर *) colon, len - (colon - तथ्य));
		अगर (v == शून्य) {
			मुक्त(n);
			वापस NSERROR_NOMEM;
		}
	}

	*name = n;
	*मूल्य = v;

	वापस NSERROR_OK;
}

/**
 * parse cache control header value
 *
 * \param object Object to parse header for
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror
llcache_fetch_parse_cache_control(llcache_object *object, अक्षर *मूल्य)
{
	http_cache_control *cc;
	nserror error;

	error = http_parse_cache_control(मूल्य, &cc);
	अगर (error != NSERROR_OK) {
		/* Ignore parse errors */
		वापस NSERROR_OK;
	}

	अगर (http_cache_control_no_cache(cc) ||
			http_cache_control_no_store(cc)) {
		/**
		 * \todo When we get a disk cache we should
		 *  distinguish between these two.
		 */
		object->cache.no_cache = LLCACHE_VALIDATE_ALWAYS;
	}

	अगर (http_cache_control_has_max_age(cc)) {
		object->cache.max_age = http_cache_control_max_age(cc);
	}

	http_cache_control_destroy(cc);

	वापस NSERROR_OK;
}

/**
 * Update cache control from appropriate header
 *
 * \param object Object to parse header for
 * \param name header name
 * \param value header value
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror
llcache_fetch_header_cache_control(llcache_object *object,
				   अक्षर *name,
				   अक्षर *मूल्य)
{
	nserror res;
	माप_प्रकार name_len;

	/* Parse cache headers to populate cache control data */
	name_len = म_माप(name);

	चयन (name_len) {
	हाल 3:
		अगर (strcasecmp(name, "Age") == 0) {
			/* extract Age header */
			अगर ('0' <= *मूल्य && *मूल्य <= '9') {
				object->cache.age = म_से_प(मूल्य);
			}

		}
		अवरोध;

	हाल 4:
		अगर (strcasecmp(name, "Date") == 0) {
			/* extract Date header */
			res = nsc_strntimet(मूल्य,
					    म_माप(मूल्य),
					    &object->cache.date);
			अगर (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Date header value \"%s\" returned %d",
				      मूल्य, res);
			}
		} अन्यथा अगर (strcasecmp(name, "ETag") == 0) {
			/* extract ETag header */
			मुक्त(object->cache.etag);
			object->cache.etag = strdup(मूल्य);
			अगर (object->cache.etag == शून्य) {
				NSLOG(llcache, INFO,
				      "No memory to duplicate ETag");
				वापस NSERROR_NOMEM;
			}
		}
		अवरोध;

	हाल 7:
		अगर (strcasecmp(name, "Expires") == 0) {
			/* process Expires header value */
			res = nsc_strntimet(मूल्य,
					    म_माप(मूल्य),
					    &object->cache.expires);
			अगर (res != NSERROR_OK) {
				NSLOG(llcache, INFO,
				      "Processing Expires header value \"%s\" returned %d",
				      मूल्य, res);
				object->cache.expires = (समय_प्रकार)0x7fffffff;
			}
		}
		अवरोध;

	हाल 13:
		अगर (strcasecmp(name, "Cache-Control") == 0) {
			/* parse Cache-Control header value */
			llcache_fetch_parse_cache_control(object,मूल्य);
		} अन्यथा अगर (strcasecmp(name, "Last-Modified") == 0) {
			/* parse Last-Modified header value */
			nsc_strntimet(मूल्य,
				      म_माप(मूल्य),
				      &object->cache.last_modified);
		}
		अवरोध;
	}

	वापस NSERROR_OK;
}

/**
 * Destroy headers.
 *
 * \param object The object to destroy headers within.
 */
अटल अंतरभूत व्योम llcache_destroy_headers(llcache_object *object)
{
	जबतक (object->अंक_headers > 0) {
		object->अंक_headers--;

		मुक्त(object->headers[object->अंक_headers].name);
		मुक्त(object->headers[object->अंक_headers].मूल्य);
	}
	मुक्त(object->headers);
	object->headers = शून्य;
}

/**
 * Invalidate cache control data.
 *
 * \param object The object to invalidate cache control for.
 */
अटल अंतरभूत व्योम llcache_invalidate_cache_control_तथ्य(llcache_object *object)
{
	मुक्त(object->cache.etag);
	स_रखो(&(object->cache), 0, माप(llcache_cache_control));

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
अटल nserror llcache_fetch_process_header(llcache_object *object,
		स्थिर uint8_t *तथ्य, माप_प्रकार len)
{
	nserror res;
	अक्षर *name, *मूल्य;
	llcache_header *अस्थायी;

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
	अगर (म_भेदन((स्थिर अक्षर *) तथ्य, "HTTP/", SLEN("HTTP/")) == 0) {
		समय_प्रकार req_time = object->cache.req_time;

		llcache_invalidate_cache_control_तथ्य(object);

		/* Restore request time, so we compute object's age correctly */
		object->cache.req_time = req_time;

		llcache_destroy_headers(object);
	}

	/* Set fetch response time if not already set */
	अगर (object->cache.res_time == 0) {
		object->cache.res_time = समय(शून्य);
	}

	/* Parse header into name-value pair */
	res = llcache_fetch_split_header(तथ्य, len, &name, &मूल्य);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	/* deal with empty header */
	अगर (name[0] == 0) {
		मुक्त(name);
		मुक्त(मूल्य);
		वापस NSERROR_OK;
	}

	/* update cache control data from header */
	res = llcache_fetch_header_cache_control(object, name, मूल्य);
	अगर (res != NSERROR_OK) {
		मुक्त(name);
		मुक्त(मूल्य);
		वापस res;
	}

	/* Append header data to the object's headers array */
	अस्थायी = पुनः_स्मृति(object->headers,
		       (object->अंक_headers + 1) * माप(llcache_header));
	अगर (अस्थायी == शून्य) {
		मुक्त(name);
		मुक्त(मूल्य);
		वापस NSERROR_NOMEM;
	}

	object->headers = अस्थायी;

	object->headers[object->अंक_headers].name = name;
	object->headers[object->अंक_headers].मूल्य = मूल्य;

	object->अंक_headers++;

	वापस NSERROR_OK;
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
अटल nserror get_referer_header(nsurl *url, nsurl *referer, अक्षर **header_out)
{
	nserror res = NSERROR_INVALID;
	lwc_string *ref_scheme;
	lwc_string *scheme;
	bool match;
	bool match1;
	bool match2;
	अक्षर *header;

	/* Determine whether to send the Referer header */
	अगर (!nsoption_bool(send_referer)) {
		वापस NSERROR_INVALID;
	}

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (scheme == शून्य) {
		वापस NSERROR_BAD_URL;
	}

	ref_scheme = nsurl_get_component(referer, NSURL_SCHEME);
	अगर (ref_scheme == शून्य) {
		/* referer has no scheme so no header */
		lwc_string_unref(scheme);
		वापस NSERROR_INVALID;
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
	अगर (lwc_string_isequal(scheme, ref_scheme,
			       &match) != lwc_error_ok) {
		match = false;
	}
	अगर (lwc_string_isequal(scheme, corestring_lwc_https,
			       &match1) != lwc_error_ok) {
		match1 = false;
	}
	अगर (lwc_string_isequal(ref_scheme, corestring_lwc_http,
			       &match2) != lwc_error_ok) {
		match2 = false;
	}
	अगर (match == true || (match1 == true && match2 == true)) {
		स्थिर माप_प्रकार len = SLEN("Referer: ") +
			nsurl_लंबाई(referer) + 1;

		header = दो_स्मृति(len);
		अगर (header == शून्य) {
			res = NSERROR_NOMEM;
		} अन्यथा {
			snprintf(header, len, "Referer: %s",
				 nsurl_access(referer));

			*header_out = header;
			res = NSERROR_OK;
		}
	}


	lwc_string_unref(scheme);
	lwc_string_unref(ref_scheme);

	वापस res;
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
अटल nserror llcache_object_refetch(llcache_object *object)
{
	स्थिर अक्षर *urlenc = शून्य;
	काष्ठा fetch_multipart_तथ्य *multipart = शून्य;
	अक्षर **headers = शून्य;
	पूर्णांक header_idx = 0;
	nserror res;

	अगर (object->fetch.post != शून्य) {
		अगर (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			urlenc = object->fetch.post->तथ्य.urlenc;
		} अन्यथा {
			multipart = object->fetch.post->तथ्य.multipart;
		}
	}

	/* Generate headers */
	headers = दो_स्मृति(4 * माप(अक्षर *));
	अगर (headers == शून्य) {
		वापस NSERROR_NOMEM;
	}

	/* cache-control header for etag */
	अगर (object->cache.etag != शून्य) {
		स्थिर माप_प्रकार len = SLEN("If-None-Match: ") +
				म_माप(object->cache.etag) + 1;

		headers[header_idx] = दो_स्मृति(len);
		अगर (headers[header_idx] == शून्य) {
			मुक्त(headers);
			वापस NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-None-Match: %s",
				object->cache.etag);

		header_idx++;
	}

	/* cache-control header for modification time */
	अगर (object->cache.last_modified != 0) {
		/* Maximum length of an RFC 1123 date is 29 bytes */
		स्थिर माप_प्रकार len = SLEN("If-Modified-Since: ") + 29 + 1;

		headers[header_idx] = दो_स्मृति(len);
		अगर (headers[header_idx] == शून्य) {
			जबतक (--header_idx >= 0)
				मुक्त(headers[header_idx]);
			मुक्त(headers);
			वापस NSERROR_NOMEM;
		}

		snprintf(headers[header_idx], len, "If-Modified-Since: %s",
				rfc1123_date(object->cache.last_modified));

		header_idx++;
	}

	/* Referer header */
	अगर (object->fetch.referer != शून्य) {
		अगर (get_referer_header(object->url,
				       object->fetch.referer,
				       &headers[header_idx]) == NSERROR_OK) {
			header_idx++;
		}
	}
	headers[header_idx] = शून्य;

	/* Reset cache control data */
	llcache_invalidate_cache_control_तथ्य(object);
	object->cache.req_time = समय(शून्य);
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
			  (स्थिर अक्षर **)headers,
			  &object->fetch.fetch);

	/* Clean up cache-control headers */
	जबतक (--header_idx >= 0) {
		मुक्त(headers[header_idx]);
	}
	मुक्त(headers);

	वापस res;
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
अटल nserror llcache_object_fetch(llcache_object *object, uint32_t flags,
		nsurl *referer, स्थिर llcache_post_तथ्य *post,
		uint32_t redirect_गिनती, bool hsts_in_use)
{
	nserror error;
	nsurl *referer_clone = शून्य;
	llcache_post_तथ्य *post_clone = शून्य;

	NSLOG(llcache, DEBUG, "Starting fetch for %p", object);

	अगर (post != शून्य) {
		error = llcache_post_तथ्य_clone(post, &post_clone);
		अगर (error != NSERROR_OK)
			वापस error;
	}

	अगर (referer != शून्य)
		referer_clone = nsurl_ref(referer);

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_गिनती = redirect_गिनती;
	object->fetch.retries_remaining = llcache->fetch_attempts;
	object->fetch.hsts_in_use = hsts_in_use;

	वापस llcache_object_refetch(object);
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
अटल nserror llcache_object_destroy(llcache_object *object)
{
	माप_प्रकार i;

	NSLOG(llcache, DEBUG, "Destroying object %p, %s", object,
	      nsurl_access(object->url));

	cert_chain_free(object->chain);

	अगर (object->source_तथ्य != शून्य) {
		अगर (object->store_state == LLCACHE_STATE_DISC) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);
		} अन्यथा {
			मुक्त(object->source_तथ्य);
		}
	}

	nsurl_unref(object->url);

	अगर (object->fetch.fetch != शून्य) {
		fetch_abort(object->fetch.fetch);
		object->fetch.fetch = शून्य;
	}

	अगर (object->fetch.referer != शून्य)
		nsurl_unref(object->fetch.referer);

	अगर (object->fetch.post != शून्य) {
		अगर (object->fetch.post->type == LLCACHE_POST_URL_ENCODED) {
			मुक्त(object->fetch.post->तथ्य.urlenc);
		} अन्यथा {
			fetch_multipart_तथ्य_destroy(
					object->fetch.post->तथ्य.multipart);
		}

		मुक्त(object->fetch.post);
	}

	मुक्त(object->cache.etag);

	क्रम (i = 0; i < object->अंक_headers; i++) {
		मुक्त(object->headers[i].name);
		मुक्त(object->headers[i].मूल्य);
	}
	मुक्त(object->headers);

	मुक्त(object);

	वापस NSERROR_OK;
}

/**
 * Add a low-level cache object to a cache list
 *
 * \param object  Object to add
 * \param list	  List to add to
 * \return NSERROR_OK
 */
अटल nserror llcache_object_add_to_list(llcache_object *object,
		llcache_object **list)
{
	object->पूर्व = शून्य;
	object->अग्र = *list;

	अगर (*list != शून्य)
		(*list)->पूर्व = object;
	*list = object;

	वापस NSERROR_OK;
}

/**
 * Determine the remaining lifetime of a cache object using the
 *
 * \param cd cache control data.
 * \return The length of time remaining for the object or 0 if expired.
 */
अटल पूर्णांक
llcache_object_rfc2616_remaining_lifetime(स्थिर llcache_cache_control *cd)
{
	पूर्णांक current_age, freshness_lifetime;
	समय_प्रकार now = समय(शून्य);

	/* Calculate staleness of cached object as per RFC 2616 13.2.3/13.2.4 */
	current_age = max(0, (cd->res_time - cd->date));
	current_age = max(current_age, (cd->age == INVALID_AGE) ? 0 : cd->age);
	current_age += cd->res_time - cd->req_time + now - cd->res_time;

	/* Determine freshness lifetime of this object */
	अगर (cd->max_age != INVALID_AGE) {
		freshness_lifetime = cd->max_age;
	} अन्यथा अगर (cd->expires != 0) {
		freshness_lifetime = cd->expires - cd->date;
	} अन्यथा अगर (cd->last_modified != 0) {
		freshness_lifetime = (now - cd->last_modified) / 10;
	} अन्यथा {
		freshness_lifetime = 0;
	}

	NSLOG(llcache, DEBUG, "%d:%d", freshness_lifetime, current_age);

	अगर ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
	    (freshness_lifetime > current_age)) {
		/* object was not forbidden from being returned from
		 * the cache unvalidated (i.e. the response contained
		 * a no-cache directive)
		 *
		 * The object current age is within the freshness lifetime.
		 */
		वापस freshness_lifetime - current_age;
	}

	वापस 0; /* object has no remaining lifetime */
}

/**
 * Determine if an object is still fresh
 *
 * \param object  Object to consider
 * \return True if object is still fresh, false otherwise
 */
अटल bool llcache_object_is_fresh(स्थिर llcache_object *object)
{
	पूर्णांक remaining_lifetime;
	स्थिर llcache_cache_control *cd = &object->cache;

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
	वापस ((cd->no_cache == LLCACHE_VALIDATE_FRESH) &&
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
अटल nserror llcache_object_clone_cache_तथ्य(llcache_object *source,
		llcache_object *destination, bool deep)
{
	/* ETag must be first, as it can fail when deep cloning */
	अगर (source->cache.etag != शून्य) {
		अक्षर *etag = source->cache.etag;

		अगर (deep) {
			/* Copy the etag */
			etag = strdup(source->cache.etag);
			अगर (etag == शून्य)
				वापस NSERROR_NOMEM;
		} अन्यथा {
			/* Destination takes ownership */
			source->cache.etag = शून्य;
		}

		अगर (destination->cache.etag != शून्य)
			मुक्त(destination->cache.etag);

		destination->cache.etag = etag;
	}

	destination->cache.req_time = source->cache.req_time;
	destination->cache.res_time = source->cache.res_time;
	destination->cache.fin_time = source->cache.fin_time;

	अगर (source->cache.date != 0)
		destination->cache.date = source->cache.date;

	अगर (source->cache.expires != 0)
		destination->cache.expires = source->cache.expires;

	अगर (source->cache.age != INVALID_AGE)
		destination->cache.age = source->cache.age;

	अगर (source->cache.max_age != INVALID_AGE)
		destination->cache.max_age = source->cache.max_age;

	अगर (source->cache.no_cache != LLCACHE_VALIDATE_FRESH)
		destination->cache.no_cache = source->cache.no_cache;

	अगर (source->cache.last_modified != 0)
		destination->cache.last_modified = source->cache.last_modified;

	वापस NSERROR_OK;
}

/**
 * Remove a low-level cache object from a cache list
 *
 * \param object  Object to remove
 * \param list	  List to remove from
 * \return NSERROR_OK
 */
अटल nserror
llcache_object_remove_from_list(llcache_object *object, llcache_object **list)
{
	अगर (object == *list)
		*list = object->अग्र;
	अन्यथा
		object->पूर्व->अग्र = object->अग्र;

	अगर (object->अग्र != शून्य)
		object->अग्र->पूर्व = object->पूर्व;

	वापस NSERROR_OK;
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
अटल nserror llcache_retrieve_persisted_तथ्य(llcache_object *object)
{
	/* ensure the source data is present if necessary */
	अगर ((object->source_तथ्य != शून्य) ||
	    (object->store_state != LLCACHE_STATE_DISC)) {
		/* source data does not require retrieving from
		 * persistent store.
		 */
		वापस NSERROR_OK;
	}

	/* Source data for the object may be in the persistent store */
	वापस guit->llcache->fetch(object->url,
				    BACKING_STORE_NONE,
				    &object->source_तथ्य,
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
अटल nserror
llcache_serialise_metadata(llcache_object *object,
			   uint8_t **तथ्य_out,
			   माप_प्रकार *datasize_out)
{
	माप_प्रकार allocsize;
	पूर्णांक datasize;
	uint8_t *तथ्य;
	अक्षर *op;
	अचिन्हित पूर्णांक hloop;
	पूर्णांक use;
	माप_प्रकार cert_chain_depth;

	अगर (object->chain != शून्य) {
		cert_chain_depth = object->chain->depth;
	} अन्यथा {
		cert_chain_depth = 0;
	}

	allocsize = 10 + 1; /* object length */

	allocsize += 10 + 1; /* request time */

	allocsize += 10 + 1; /* response time */

	allocsize += 10 + 1; /* completion time */

	allocsize += 10 + 1; /* space for number of header entries */

	क्रम (hloop = 0 ; hloop < object->अंक_headers ; hloop++) {
		allocsize += म_माप(object->headers[hloop].name) + 1;
		allocsize += म_माप(object->headers[hloop].मूल्य) + 1;
	}

	allocsize += nsurl_लंबाई(object->url) + 1;

	/* space for number of DER formatted certificates */
	allocsize += 10 + 1;

	क्रम (hloop = 0; hloop < cert_chain_depth; hloop++) {
		allocsize += 10 + 1; /* error status */
		allocsize += 4 * ((object->chain->certs[hloop].der_लंबाई + 2) / 3);
	}

	तथ्य = दो_स्मृति(allocsize);
	अगर (तथ्य == शून्य) {
		वापस NSERROR_NOMEM;
	}

	op = (अक्षर *)तथ्य;
	datasize = allocsize;

	/* the url, used for checking for collisions */
	use = snprintf(op, datasize, "%s", nsurl_access(object->url));
	अगर (use < 0) {
		जाओ operror;
	}
	use++; /* does not count the null */
	अगर (use > datasize) {
		जाओ overflow;
	}
	op += use;
	datasize -= use;

	/* object size */
	use = snprintf(op, datasize, "%" PRIsizet, object->source_len);
	अगर (use < 0) {
		जाओ operror;
	}
	use++; /* does not count the null */
	अगर (use > datasize)
		जाओ overflow;
	op += use;
	datasize -= use;

	/* Time of request */
	use = nsc_sntimet(op, datasize, &object->cache.req_time);
	अगर (use == 0)
		जाओ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of response */
	use = nsc_sntimet(op, datasize, &object->cache.res_time);
	अगर (use == 0)
		जाओ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* Time of completion */
	use = nsc_sntimet(op, datasize, &object->cache.fin_time);
	अगर (use == 0)
		जाओ overflow;
	use++; /* does not count the null */
	op += use;
	datasize -= use;

	/* number of headers */
	use = snprintf(op, datasize, "%" PRIsizet, object->अंक_headers);
	अगर (use < 0) {
		जाओ operror;
	}
	use++; /* does not count the null */
	अगर (use > datasize)
		जाओ overflow;
	op += use;
	datasize -= use;

	/* headers */
	क्रम (hloop = 0 ; hloop < object->अंक_headers ; hloop++) {
		use = snprintf(op, datasize,
			       "%s:%s",
			       object->headers[hloop].name,
			       object->headers[hloop].मूल्य);
		अगर (use < 0) {
			जाओ operror;
		}
		use++; /* does not count the null */
		अगर (use > datasize)
			जाओ overflow;
		op += use;
		datasize -= use;
	}

	/* number of DER formatted ssl certificates */
	use = snprintf(op, datasize, "%" PRIsizet, cert_chain_depth);
	अगर (use < 0) {
		जाओ operror;
	}
	use++; /* does not count the null */
	अगर (use > datasize)
		जाओ overflow;
	op += use;
	datasize -= use;

	/* SSL certificates */
	क्रम (hloop = 0; hloop < cert_chain_depth; hloop++) {
		nsuerror res;

		/* Certificate error code */
		use = snprintf(op, datasize, "%d",
			       (पूर्णांक)(object->chain->certs[hloop].err));
		अगर (use < 0) {
			जाओ operror;
		}
		use++; /* does not count the null */
		अगर (use > datasize)
			जाओ overflow;
		op += use;
		datasize -= use;

		/* DER certificate data in base64 encoding */
		अगर (object->chain->certs[hloop].der != शून्य) {
			माप_प्रकार output_लंबाई = datasize;
			res = nsu_base64_encode(
				object->chain->certs[hloop].der,
				object->chain->certs[hloop].der_लंबाई,
				(uint8_t *)op,
				&output_लंबाई);
			अगर (res != NSUERROR_OK) {
				जाओ operror;
			}
			use = output_लंबाई;
		} अन्यथा {
			use = 0;
		}
		use++; /* allow for null */
		अगर (use > datasize)
			जाओ overflow;
		op += use;
		*(op - 1) = 0;
		datasize -= use;
	}

	NSLOG(llcache, DEBUG, "Filled buffer with %d spare", datasize);

	*तथ्य_out = तथ्य;
	*datasize_out = allocsize - datasize;

	वापस NSERROR_OK;

overflow:
	/* somehow we overflowed the buffer - hth? */
	NSLOG(llcache, INFO, "Overflowed metadata buffer");
	मुक्त(तथ्य);
	वापस NSERROR_INVALID;

operror:
	/* output error */
	NSLOG(llcache, INFO, "Output error");
	मुक्त(तथ्य);
	वापस NSERROR_INVALID;
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
अटल nserror
llcache_process_metadata(llcache_object *object)
{
	nserror res;
	uint8_t *metadata = शून्य;
	माप_प्रकार metadatalen = 0;
	माप_प्रकार remaining = 0;
	nsurl *metadataurl;
	अचिन्हित पूर्णांक पंक्ति;
	अक्षर *ln;
	पूर्णांक lnsize;

	माप_प्रकार source_लंबाई;
	समय_प्रकार request_time;
	समय_प्रकार response_time;
	समय_प्रकार completion_time;
	माप_प्रकार अंक_headers;
	माप_प्रकार hloop;
	माप_प्रकार ssl_cert_गिनती = 0;
	काष्ठा cert_chain *chain = शून्य;

	NSLOG(llcache, INFO, "Retrieving metadata");

	/* attempt to retrieve object metadata from the backing store */
	res = guit->llcache->fetch(object->url,
				   BACKING_STORE_META,
				   &metadata,
				   &metadatalen);
	अगर (res != NSERROR_OK) {
		वापस res;
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
	पंक्ति = 1;
	ln = (अक्षर *)metadata;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	अगर (lnsize < 7) {
		res = NSERROR_INVALID;
		जाओ format_error;
	}

	res = nsurl_create(ln, &metadataurl);
	अगर (res != NSERROR_OK)
		जाओ format_error;

	अगर (nsurl_compare(object->url, metadataurl, NSURL_COMPLETE) != true) {
		/* backing store returned the wrong object for the
		 * request. This may occur if the backing store had
		 * a collision in its storage method. We cope with this
		 * by simply skipping caching of this object.
		 */

		NSLOG(llcache, INFO, "Got metadata for %s instead of %s",
		      nsurl_access(metadataurl), nsurl_access(object->url));

		nsurl_unref(metadataurl);

		guit->llcache->release(object->url, BACKING_STORE_META);

		वापस NSERROR_BAD_URL;
	}
	nsurl_unref(metadataurl);


	/* metadata line 2 is the object's length */
	पंक्ति = 2;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	अगर ((lnsize < 1) || (माला_पूछो(ln, "%" PRIsizet, &source_लंबाई) != 1)) {
		res = NSERROR_INVALID;
		जाओ format_error;
	}


	/* metadata line 3 is the time of request */
	पंक्ति = 3;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &request_time);
	अगर (res != NSERROR_OK)
		जाओ format_error;


	/* metadata line 4 is the time of response */
	पंक्ति = 4;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &response_time);
	अगर (res != NSERROR_OK)
		जाओ format_error;


	/* metadata line 5 is the time of request completion */
	पंक्ति = 5;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	res = nsc_snptimet(ln, lnsize, &completion_time);
	अगर (res != NSERROR_OK)
		जाओ format_error;


	/* metadata line 6 is the number of headers */
	पंक्ति = 6;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	अगर ((lnsize < 1) || (माला_पूछो(ln, "%" PRIsizet, &अंक_headers) != 1)) {
		res = NSERROR_INVALID;
		जाओ format_error;
	}

	/* read headers */
	क्रम (hloop = 0 ; hloop < अंक_headers; hloop++) {
		पंक्ति++;
		ln += lnsize + 1;
		lnsize = म_माप(ln);
		remaining -= lnsize + 1;

		res = llcache_fetch_process_header(object,
						   (uint8_t *)ln,
						   lnsize);
		अगर (res != NSERROR_OK)
			जाओ format_error;
	}

	अगर (remaining == 0) {
		जाओ skip_ssl_certificates;
	}

	/* Next line is the number of DER base64 encoded certificates */
	पंक्ति++;
	ln += lnsize + 1;
	lnsize = म_माप(ln);
	remaining -= lnsize + 1;

	अगर ((lnsize < 1) || (माला_पूछो(ln, "%" PRIsizet, &ssl_cert_गिनती) != 1)) {
		res = NSERROR_INVALID;
		जाओ format_error;
	}

	अगर (ssl_cert_गिनती == 0) {
		जाओ skip_ssl_certificates;
	}

	अगर (ssl_cert_गिनती > MAX_CERT_DEPTH) {
		res = NSERROR_INVALID;
		जाओ format_error;
	}

	res = cert_chain_alloc(ssl_cert_गिनती, &chain);
	अगर (res != NSERROR_OK) {
		जाओ format_error;
	}

	क्रम (hloop = 0; hloop < ssl_cert_गिनती; hloop++) {
		पूर्णांक errcode;
		nsuerror nsures;

		/* Certificate error code */
		पंक्ति++;
		ln += lnsize + 1;
		lnsize = म_माप(ln);
		remaining -= lnsize + 1;
		अगर ((lnsize < 1) || (माला_पूछो(ln, "%d", &errcode) != 1)) {
			res = NSERROR_INVALID;
			जाओ format_error;
		}
		अगर (errcode < SSL_CERT_ERR_OK ||
		    errcode > SSL_CERT_ERR_MAX_KNOWN) {
			/* Error with the cert code, assume UNKNOWN */
			chain->certs[hloop].err = SSL_CERT_ERR_UNKNOWN;
		} अन्यथा {
			chain->certs[hloop].err = (ssl_cert_err)errcode;
		}

		/* base64 encoded DER certificate data */
		पंक्ति++;
		ln += lnsize + 1;
		lnsize = म_माप(ln);
		remaining -= lnsize + 1;
		अगर (lnsize > 0) {
			nsures = nsu_base64_decode_alloc((स्थिर uint8_t *)ln,
					lnsize,
					&chain->certs[hloop].der,
					&chain->certs[hloop].der_लंबाई);
			अगर (nsures != NSUERROR_OK) {
				res = NSERROR_NOMEM;
				जाओ format_error;
			}
		}
	}

skip_ssl_certificates:
	guit->llcache->release(object->url, BACKING_STORE_META);

	/* update object on successful parse of metadata  */
	object->source_len = source_लंबाई;

	/** \todo really not sure this is right, nothing is allocated here? */
	object->source_alloc = metadatalen;

	object->cache.req_time = request_time;
	object->cache.res_time = response_time;
	object->cache.fin_time = completion_time;

	object->chain = chain;

	/* object stored in backing store */
	object->store_state = LLCACHE_STATE_DISC;

	वापस NSERROR_OK;

format_error:
	NSLOG(llcache, INFO,
	      "metadata error on line %d error code %d\n",
	      पंक्ति, res);
	guit->llcache->release(object->url, BACKING_STORE_META);

	cert_chain_free(chain);

	वापस res;
}

/**
 * Check whether a scheme is persistable.
 *
 * \param url  URL to check.
 * \return true iff url has a persistable scheme.
 */
अटल अंतरभूत bool llcache__scheme_is_persistable(स्थिर nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool persistable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	अगर ((lwc_string_isequal(scheme, corestring_lwc_http,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&match) == lwc_error_ok &&
			(match == true))) {
		persistable = true;
	}

	lwc_string_unref(scheme);

	वापस persistable;
}

/**
 * Check whether a scheme is cachable.
 *
 * \param url  URL to check.
 * \return true iff url has a cachable scheme.
 */
अटल अंतरभूत bool llcache__scheme_is_cachable(स्थिर nsurl *url)
{
	lwc_string *scheme = nsurl_get_component(url, NSURL_SCHEME);
	bool cachable = false;
	bool match;

	/* nsurl ensures lower case schemes, and corestrings are lower
	 * case, so it's safe to use case-sensitive comparison. */
	अगर ((lwc_string_isequal(scheme, corestring_lwc_http,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_https,
			&match) == lwc_error_ok &&
			(match == true)) ||
	    (lwc_string_isequal(scheme, corestring_lwc_तथ्य,
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

	वापस cachable;
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
अटल nserror
llcache_object_fetch_persistent(llcache_object *object,
				uint32_t flags,
				nsurl *referer,
				स्थिर llcache_post_तथ्य *post,
				uint32_t redirect_गिनती)
{
	nserror error;
	nsurl *referer_clone = शून्य;
	llcache_post_तथ्य *post_clone = शून्य;

	अगर (!llcache__scheme_is_persistable(object->url)) {
		/* Don't bother looking up non-http(s) stuff; we don't
		 * persist it. */
		वापस NSERROR_NOT_FOUND;
	}

	object->cache.req_time = समय(शून्य);
	object->cache.fin_time = object->cache.req_time;

	/* retrieve and process metadata */
	error = llcache_process_metadata(object);
	अगर (error != NSERROR_OK) {
		वापस error;
	}

	/* entry came out of cache - need to setup object state */
	अगर (post != शून्य) {
		error = llcache_post_तथ्य_clone(post, &post_clone);
		अगर (error != NSERROR_OK)
			वापस error;
	}

	अगर (referer != शून्य) {
		referer_clone = nsurl_ref(referer);
	}

	object->fetch.flags = flags;
	object->fetch.referer = referer_clone;
	object->fetch.post = post_clone;
	object->fetch.redirect_गिनती = redirect_गिनती;

	/* fetch is "finished" */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = शून्य;

	वापस NSERROR_OK;
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
अटल nserror
llcache_object_retrieve_from_cache(nsurl *url,
				   uint32_t flags,
				   nsurl *referer,
				   स्थिर llcache_post_तथ्य *post,
				   uint32_t redirect_गिनती,
				   bool hsts_in_use,
				   llcache_object **परिणाम)
{
	nserror error;
	llcache_object *obj, *newest = शून्य;

	NSLOG(llcache, DEBUG,
	      "Searching cache for %s flags:%"PRIx32" referer:%s post:%p",
	      nsurl_access(url), flags,
	      referer==शून्य?"":nsurl_access(referer),
	      post);

	/* Search for the most recently fetched matching object */
	क्रम (obj = llcache->cached_objects; obj != शून्य; obj = obj->अग्र) {

		अगर ((newest == शून्य ||
		     obj->cache.req_time > newest->cache.req_time) &&
		    nsurl_compare(obj->url, url,
				  NSURL_COMPLETE) == true) {
			newest = obj;
		}
	}

	/* No viable object found in cache create one and attempt to
	 * pull from persistent store.
	 */
	अगर (newest == शून्य) {
		NSLOG(llcache, DEBUG, "No viable object found in llcache");

		error = llcache_object_new(url, &obj);
		अगर (error != NSERROR_OK)
			वापस error;

		/* attempt to retrieve object from persistent store */
		error = llcache_object_fetch_persistent(obj, flags, referer, post, redirect_गिनती);
		अगर (error == NSERROR_OK) {
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

	अगर ((newest != शून्य) && (llcache_object_is_fresh(newest))) {
		/* Found a suitable object, and it's still fresh */
		NSLOG(llcache, DEBUG, "Found fresh %p", newest);

		/* The client needs to catch up with the object's state.
		 * This will occur the next time that llcache_poll is called.
		 */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_तथ्य(newest);
		अगर (error == NSERROR_OK) {
			/* source data was successfully retrieved from
			 * persistent store
			 */
			*परिणाम = newest;

			वापस NSERROR_OK;
		}

		/* retrieval of source data from persistent store
		 * failed, destroy cache object and fall though to
		 * cache miss to re-fetch
		 */
		NSLOG(llcache, DEBUG, "Persistent retrieval failed for %p", newest);

		llcache_object_remove_from_list(newest,	&llcache->cached_objects);
		llcache_object_destroy(newest);

		error = llcache_object_new(url, &obj);
		अगर (error != NSERROR_OK) {
			वापस error;
		}
	} अन्यथा अगर (newest != शून्य) {
		/* Found a candidate object but it needs freshness validation */

		/* ensure the source data is present */
		error = llcache_retrieve_persisted_तथ्य(newest);
		अगर (error == NSERROR_OK) {

			/* Create a new object */
			error = llcache_object_new(url, &obj);
			अगर (error != NSERROR_OK)
				वापस error;

			NSLOG(llcache, DEBUG, "Found candidate %p (%p)", obj, newest);

			/* Clone candidate's cache data */
			error = llcache_object_clone_cache_तथ्य(newest, obj, true);
			अगर (error != NSERROR_OK) {
				llcache_object_destroy(obj);
				वापस error;
			}

			/* Record candidate, so we can fall back if it is still fresh */
			newest->candidate_गिनती++;
			obj->candidate = newest;

			/* Attempt to kick-off fetch */
			error = llcache_object_fetch(obj, flags, referer, post,
						     redirect_गिनती, hsts_in_use);
			अगर (error != NSERROR_OK) {
				newest->candidate_गिनती--;
				llcache_object_destroy(obj);
				वापस error;
			}

			/* Add new object to cache */
			llcache_object_add_to_list(obj, &llcache->cached_objects);

			*परिणाम = obj;

			वापस NSERROR_OK;
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
		अगर (error != NSERROR_OK) {
			वापस error;
		}
	}

	/* Attempt to kick-off fetch */
	error = llcache_object_fetch(obj, flags, referer, post,
			redirect_गिनती, hsts_in_use);
	अगर (error != NSERROR_OK) {
		llcache_object_destroy(obj);
		वापस error;
	}

	/* Add new object to cache */
	llcache_object_add_to_list(obj, &llcache->cached_objects);

	*परिणाम = obj;

	वापस NSERROR_OK;
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
अटल nserror
llcache_object_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			स्थिर llcache_post_तथ्य *post,
			uint32_t redirect_गिनती,
			bool hsts_in_use,
			llcache_object **परिणाम)
{
	nserror error;
	llcache_object *obj;
	nsurl *defragmented_url;
	bool uncachable = false;

	NSLOG(llcache, DEBUG, "Retrieve %s (%"PRIx32", %s, %p)", nsurl_access(url), flags,
		     referer==शून्य?"":nsurl_access(referer), post);


	/* Get rid of any url fragment */
	error = nsurl_defragment(url, &defragmented_url);
	अगर (error != NSERROR_OK)
		वापस error;

	/* determine if content is cachable */
	अगर ((flags & LLCACHE_RETRIEVE_FORCE_FETCH) != 0) {
		/* Forced fetches are never cached */
		uncachable = true;
	} अन्यथा अगर (post != शून्य) {
		/* POST requests are never cached */
		uncachable = true;
	} अन्यथा {
		uncachable = !llcache__scheme_is_cachable(defragmented_url);
	}

	अगर (uncachable) {
		/* Create new object */
		error = llcache_object_new(defragmented_url, &obj);
		अगर (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			वापस error;
		}

		/* Attempt to kick-off fetch */
		error = llcache_object_fetch(obj, flags, referer, post,
				redirect_गिनती, hsts_in_use);
		अगर (error != NSERROR_OK) {
			llcache_object_destroy(obj);
			nsurl_unref(defragmented_url);
			वापस error;
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(obj, &llcache->uncached_objects);
	} अन्यथा {
		error = llcache_object_retrieve_from_cache(defragmented_url,
				flags, referer, post, redirect_गिनती,
				hsts_in_use, &obj);
		अगर (error != NSERROR_OK) {
			nsurl_unref(defragmented_url);
			वापस error;
		}

		/* Returned object is already in the cached list */
	}

	NSLOG(llcache, DEBUG, "Retrieved %p", obj);

	*परिणाम = obj;

	nsurl_unref(defragmented_url);

	वापस NSERROR_OK;
}


/**
 * Add a user to a low-level cache object
 *
 * \param object Object to add user to
 * \param user User to add
 * \return NSERROR_OK.
 */
अटल nserror llcache_object_add_user(llcache_object *object,
		llcache_object_user *user)
{
	निश्चित(user->अग्र == शून्य);
	निश्चित(user->पूर्व == शून्य);
	निश्चित(user->handle != शून्य);

	user->handle->object = object;

	user->पूर्व = शून्य;
	user->अग्र = object->users;

	अगर (object->users != शून्य)
		object->users->पूर्व = user;
	object->users = user;

	NSLOG(llcache, DEBUG, "Adding user %p to %p", user, object);

	वापस NSERROR_OK;
}

/**
 * Transform a request-URI based on HSTS policy
 *
 * \param url URL to transform
 * \param result Pointer to location to receive transformed URL
 * \param hsts_in_use Pointer to location to receive HSTS in-use flag
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_hsts_transform_url(nsurl *url, nsurl **परिणाम,
		bool *hsts_in_use)
{
	lwc_string *scheme = शून्य;
	bool match;
	nserror error = NSERROR_OK;

	scheme = nsurl_get_component(url, NSURL_SCHEME);
	अगर (lwc_string_caseless_isequal(scheme, corestring_lwc_http,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTP fetch: no transform required */
		अगर (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
				&match) == lwc_error_ok && match) {
			/* HTTPS: ask urldb if HSTS is enabled */
			*hsts_in_use = urldb_get_hsts_enabled(url);
		} अन्यथा {
			/* Anything else: no HSTS */
			*hsts_in_use = false;
		}
		lwc_string_unref(scheme);
		*परिणाम = nsurl_ref(url);
		वापस error;
	}
	lwc_string_unref(scheme);

	अगर (urldb_get_hsts_enabled(url)) {
		/* Only need to force HTTPS. If original port was explicitly
		 * specified as 80, nsurl_create/join will remove it (as
		 * it's redundant) */
		error = nsurl_replace_scheme(url, corestring_lwc_https,
				परिणाम);
		*hsts_in_use = (error == NSERROR_OK);
	} अन्यथा {
		*परिणाम = nsurl_ref(url);
		*hsts_in_use = false;
	}

	वापस error;
}

/**
 * Update HSTS policy for target domain.
 *
 * \param object Newly-fetched cache object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_hsts_update_policy(llcache_object *object)
{
	माप_प्रकार i;
	lwc_string *scheme = शून्य;
	bool match = false;

	scheme = nsurl_get_component(object->url, NSURL_SCHEME);
	अगर (lwc_string_caseless_isequal(scheme, corestring_lwc_https,
			&match) != lwc_error_ok || match == false) {
		/* Non-HTTPS fetch: ignore */
		lwc_string_unref(scheme);
		वापस NSERROR_OK;
	}
	lwc_string_unref(scheme);

	अगर (object->fetch.tainted_tls) {
		/* Transport is tainted: ignore */
		वापस NSERROR_OK;
	}

	क्रम (i = 0; i < object->अंक_headers; i++) {
		अगर (strcasecmp("Strict-Transport-Security",
				object->headers[i].name) == 0) {
			urldb_set_hsts_policy(object->url,
					object->headers[i].मूल्य);
			/* Only process the first one we find */
			अवरोध;
		}
	}

	वापस NSERROR_OK;
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
अटल nserror check_redirect_safety(nsurl *src_url, nsurl *dst_url)
{
	lwc_string *scheme;
	lwc_string *object_scheme;
	bool match;

	object_scheme = nsurl_get_component(src_url, NSURL_SCHEME);
	scheme = nsurl_get_component(dst_url, NSURL_SCHEME);

	/* resource: and about: are allowed to redirect anywhere */
	अगर ((lwc_string_isequal(object_scheme, corestring_lwc_resource,
			&match) == lwc_error_ok && match == false) &&
	    (lwc_string_isequal(object_scheme, corestring_lwc_about,
			&match) == lwc_error_ok && match == false)) {
		/* file, about and resource are not valid redirect targets */
		अगर ((lwc_string_isequal(scheme, corestring_lwc_file,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_about,
				&match) == lwc_error_ok && match == true) ||
		    (lwc_string_isequal(scheme, corestring_lwc_resource,
				&match) == lwc_error_ok && match == true)) {
			lwc_string_unref(object_scheme);
			lwc_string_unref(scheme);

			वापस NSERROR_UNSAFE_REDIRECT;
		}
	}

	lwc_string_unref(scheme);
	lwc_string_unref(object_scheme);

	/* Bail out if we've no way of handling this URL */
	अगर (fetch_can_fetch(dst_url) == false) {
		वापस NSERROR_UNSAFE_REDIRECT;
	}

	वापस NSERROR_OK;
}

/**
 * Handle FETCH_REDIRECT event
 *
 * \param object       Object being redirected
 * \param target       Target of redirect (may be relative)
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_fetch_redirect(llcache_object *object,
		स्थिर अक्षर *target, llcache_object **replacement)
{
	nserror res;
	llcache_object *dest;
	llcache_object_user *user, *अग्र;
	स्थिर llcache_post_तथ्य *post = object->fetch.post;
	nsurl *url, *hsts_url;
	bool hsts_in_use;
	http_response_code http_code;
	llcache_event event;

	/* Extract HTTP response code from the fetch object before it is aborted */
	http_code = fetch_http_code(object->fetch.fetch);

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = शून्य;

	/* Invalidate the cache control data */
	llcache_invalidate_cache_control_तथ्य(object);

	/* And mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(व्योम) llcache_hsts_update_policy(object);

	/* Forcibly stop redirecting if we've followed too many redirects */
	अगर (object->fetch.redirect_गिनती > nsoption_uint(fetch_redirect_limit)) {
		NSLOG(llcache, INFO, "Too many nested redirects");
		वापस NSERROR_CYCLIC_REDIRECT;
	}

	/* Make target absolute */
	res = nsurl_join(object->url, target, &url);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	/* Perform HSTS transform */
	res = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	nsurl_unref(url);
	अगर (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "hsts transform failed");
		वापस res;
	}

	/* check redirect allowed before informing users */
	res = check_redirect_safety(object->url, hsts_url);
	अगर (res != NSERROR_OK) {
		NSLOG(llcache, INFO, "unsafe redirect");
		nsurl_unref(hsts_url);
		वापस res;
	}

	/* alter behaviour based on specific redirect code */
	चयन (http_code) {
	हाल HTTP_RESPONSE_MOVED_PERMANENTLY:
	हाल HTTP_RESPONSE_FOUND:
	हाल HTTP_RESPONSE_SEE_OTHER:
		/* 301, 302, 303 redirects change method to GET */
		post = शून्य;
		अवरोध;

	हाल HTTP_RESPONSE_TEMPORARY_REDIRECT:
	हाल HTTP_RESPONSE_PERMANENT_REDIRECT:
		/* 307 and 308 keep method unchanged */
		अवरोध;

	शेष:
		/** \todo implement codes
		 * HTTP_RESPONSE_MULTIPLE_CHOICES (300),
		 * HTTP_RESPONSE_USE_PROXY (305)
		 */
		NSLOG(llcache, INFO, "unsupported redirect %d", http_code);
		nsurl_unref(hsts_url);
		वापस NSERROR_BAD_REDIRECT;
	}

	/* Inform users of redirect */
	event.type = LLCACHE_EVENT_REDIRECT;
	event.तथ्य.redirect.from = object->url;
	event.तथ्य.redirect.to = hsts_url;

	res = llcache_send_event_to_users(object, &event);
	अगर (res != NSERROR_OK) {
		nsurl_unref(hsts_url);
		वापस res;
	}

	/* Attempt to fetch target URL */
	res = llcache_object_retrieve(hsts_url,
					object->fetch.flags,
					object->fetch.referer,
					post,
					object->fetch.redirect_गिनती + 1,
					hsts_in_use,
					&dest);

	/* No longer require url */
	nsurl_unref(hsts_url);

	अगर (res == NSERROR_OK) {
		/* successfully started redirected fetch */

		/* Move user(s) to replacement object */
		क्रम (user = object->users; user != शून्य; user = अग्र) {
			अग्र = user->अग्र;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(dest, user);
		}

		/* Dest is now our object */
		*replacement = dest;
	}

	वापस res;
}

/**
 * Update an object's cache state
 *
 * \param object  Object to update cache for
 * \return NSERROR_OK.
 */
अटल nserror llcache_object_cache_update(llcache_object *object)
{
	अगर (object->cache.date == 0)
		object->cache.date = समय(शून्य);

	वापस NSERROR_OK;
}

/**
 * Handle FETCH_NOTMODIFIED event
 *
 * \param object       Object to process
 * \param replacement  Pointer to location to receive replacement object
 * \return NSERROR_OK.
 */
अटल nserror llcache_fetch_notmodified(llcache_object *object,
		llcache_object **replacement)
{
	/* There may be no candidate if the server erroneously responded
	 * to an unconditional request with a 304 Not Modified response.
	 * In this case, we simply retain the initial object, having
	 * invalidated it and marked it as complete.
	 */
	अगर (object->candidate != शून्य) {
		llcache_object_user *user, *अग्र;

		/* Move user(s) to candidate content */
		क्रम (user = object->users; user != शून्य; user = अग्र) {
			अग्र = user->अग्र;

			llcache_object_remove_user(object, user);
			llcache_object_add_user(object->candidate, user);
		}

		/* Candidate is no longer a candidate for us */
		object->candidate->candidate_गिनती--;

		/* Clone our cache control data into the candidate */
		llcache_object_clone_cache_तथ्य(object, object->candidate,
				false);
		/* Bring candidate's cache data up to date */
		llcache_object_cache_update(object->candidate);
		/* Revert no-cache to normal, if required */
		अगर (object->candidate->cache.no_cache ==
				LLCACHE_VALIDATE_ONCE) {
			object->candidate->cache.no_cache =
				LLCACHE_VALIDATE_FRESH;
		}

		/* Candidate is now our object */
		*replacement = object->candidate;
		object->candidate = शून्य;
	} अन्यथा {
		/* There was no candidate: retain object */
		*replacement = object;
	}

	/* Ensure fetch has stopped */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = शून्य;

	/* Invalidate our cache-control data */
	llcache_invalidate_cache_control_तथ्य(object);

	/* Mark it complete */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;

	(व्योम) llcache_hsts_update_policy(object);

	/* Old object will be flushed from the cache on the next poll */

	वापस NSERROR_OK;
}

/**
 * Process a chunk of fetched data
 *
 * \param object  Object being fetched
 * \param data	  Data to process
 * \param len	  Byte length of data
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
अटल nserror
llcache_fetch_process_तथ्य(llcache_object *object,
			   स्थिर uint8_t *तथ्य,
			   माप_प्रकार len)
{
	अगर (object->fetch.state != LLCACHE_FETCH_तथ्य) {
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
		दीर्घ http_code = fetch_http_code(object->fetch.fetch);

		अगर ((http_code != 200 && http_code != 203) ||
		    (nsurl_has_component(object->url, NSURL_QUERY) &&
		     (object->cache.max_age == INVALID_AGE &&
		      object->cache.expires == 0))) {
			/* Invalidate cache control data */
			llcache_invalidate_cache_control_तथ्य(object);
		}

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		object->fetch.state = LLCACHE_FETCH_तथ्य;
	}

	/* Resize source buffer if it's too small */
	अगर (object->source_len + len >= object->source_alloc) {
		स्थिर माप_प्रकार new_len = object->source_len + len + 64 * 1024;
		uint8_t *अस्थायी = पुनः_स्मृति(object->source_तथ्य, new_len);
		अगर (अस्थायी == शून्य)
			वापस NSERROR_NOMEM;

		object->source_तथ्य = अस्थायी;
		object->source_alloc = new_len;
	}

	/* Append this data chunk to source buffer */
	स_नकल(object->source_तथ्य + object->source_len, तथ्य, len);
	object->source_len += len;

	वापस NSERROR_OK;
}


/**
 * Handle an authentication request
 *
 * \param object  Object being fetched
 * \param realm	  Authentication realm
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
अटल nserror llcache_fetch_auth(llcache_object *object, स्थिर अक्षर *realm)
{
	स्थिर अक्षर *auth;
	nserror error = NSERROR_OK;

	/* Abort fetch for this object */
	fetch_abort(object->fetch.fetch);
	object->fetch.fetch = शून्य;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_तथ्य(object);

	/* Destroy headers */
	llcache_destroy_headers(object);

	/* If there was no realm, then default to the URL */
	/** \todo If there was no WWW-Authenticate header, use response body */
	अगर (realm == शून्य)
		realm = nsurl_access(object->url);

	auth = urldb_get_auth_details(object->url, realm);

	अगर (auth == शून्य || object->fetch.tried_with_auth == true) {
		llcache_event event;
		/* No authentication details, or tried what we had, so ask */
		object->fetch.tried_with_auth = false;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.तथ्य.error.code = NSERROR_BAD_AUTH;
		event.तथ्य.error.संदेश = realm;

		error = llcache_send_event_to_users(object, &event);
	} अन्यथा {
		/* Flag that we've tried to refetch with credentials, so
		 * that if the fetch fails again, we ask the user again */
		object->fetch.tried_with_auth = true;
		error = llcache_object_refetch(object);
	}

	वापस error;
}

/**
 * Handle a TLS certificate verification failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_fetch_cert_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = शून्य;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_तथ्य(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Only give the user a chance if HSTS isn't in use for this fetch */
	अगर (object->fetch.hsts_in_use == false) {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.तथ्य.error.code = NSERROR_BAD_CERTS;
		event.तथ्य.error.संदेश = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} अन्यथा {
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.तथ्य.error.code = NSERROR_UNKNOWN;
		event.तथ्य.error.संदेश = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	}

	वापस error;
}


/**
 * Handle a TLS connection setup failure
 *
 * \param object  Object being fetched
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_fetch_ssl_error(llcache_object *object)
{
	nserror error = NSERROR_OK;

	/* Fetch has been stopped, and destroyed. Invalidate object's pointer */
	object->fetch.fetch = शून्य;

	/* Invalidate cache-control data */
	llcache_invalidate_cache_control_तथ्य(object);

	/* Consider the TLS transport tainted */
	object->fetch.tainted_tls = true;

	/* Make no attempt to downgrade if HSTS is in use
	 * (i.e. assume server does TLS properly) */
	अगर (object->fetch.hsts_in_use ||
	    object->fetch.tried_with_tls_downgrade) {
		/* Have already tried to downgrade, so give up */
		llcache_event event;

		/* Mark object complete */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Inform client(s) that object fetch failed */
		event.type = LLCACHE_EVENT_ERROR;
		/** \todo More appropriate error message */
		event.तथ्य.error.code = NSERROR_UNKNOWN;
		event.तथ्य.error.संदेश = messages_get("FetchFailed");

		error = llcache_send_event_to_users(object, &event);
	} अन्यथा {
		/* Flag that we've tried to downgrade, so that if the
		 * fetch fails again, we give up */
		object->fetch.tried_with_tls_downgrade = true;
		error = llcache_object_refetch(object);
	}

	वापस error;
}


/**
 * handle time out while trying to fetch.
 *
 * \param object Object being fetched
 * \return NSERROR_OK on success otherwise error code
 */
अटल nserror llcache_fetch_timeout(llcache_object *object)
{
	llcache_event event;

	/* The fetch has already been cleaned up by the fetcher but
	 * we would like to retry if we can.
	 */
	अगर (object->fetch.retries_remaining > 1) {
		object->fetch.retries_remaining--;
		वापस llcache_object_refetch(object);
	}

	/* The fetch has has already been cleaned up by the fetcher */
	object->fetch.state = LLCACHE_FETCH_COMPLETE;
	object->fetch.fetch = शून्य;

	/* Release candidate, if any */
	अगर (object->candidate != शून्य) {
		object->candidate->candidate_गिनती--;
		object->candidate = शून्य;
	}

	/* Invalidate cache control data */
	llcache_invalidate_cache_control_तथ्य(object);

	event.type = LLCACHE_EVENT_ERROR;
	event.तथ्य.error.code = NSERROR_TIMEOUT;
	event.तथ्य.error.संदेश = शून्य;

	वापस llcache_send_event_to_users(object, &event);
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
अटल nserror
build_candidate_list(काष्ठा llcache_object ***lst_out, पूर्णांक *lst_len_out)
{
	llcache_object *object, *अग्र;
	काष्ठा llcache_object **lst;
	पूर्णांक lst_len = 0;
	पूर्णांक remaining_lifetime;

#घोषणा MAX_PERSIST_PER_RUN 128

	lst = सुस्मृति(MAX_PERSIST_PER_RUN, माप(काष्ठा llcache_object *));
	अगर (lst == शून्य) {
		वापस NSERROR_NOMEM;
	}

	क्रम (object = llcache->cached_objects; object != शून्य; object = अग्र) {
		अग्र = object->अग्र;

		/* Only consider http(s) for the disc cache. */
		अगर (!llcache__scheme_is_persistable(object->url)) {
			जारी;
		}

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		/* cacehable objects with no pending fetches, not
		 * already on disc and with sufficient lifetime to
		 * make disc cache worthwhile
		 */
		अगर ((object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य) &&
		    (object->store_state == LLCACHE_STATE_RAM) &&
		    (remaining_lifetime > llcache->minimum_lifetime)) {
			lst[lst_len] = object;
			lst_len++;
			अगर (lst_len == MAX_PERSIST_PER_RUN)
				अवरोध;
		}
	}

	अगर (lst_len == 0) {
		मुक्त(lst);
		वापस NSERROR_NOT_FOUND;
	}

	/** \todo sort list here */

	*lst_len_out = lst_len;
	*lst_out = lst;

#अघोषित MAX_PERSIST_PER_RUN

	वापस NSERROR_OK;
}

/**
 * Write an object to the backing store.
 *
 * \param object The object to put in the backing store.
 * \param written_out The amount of data written out.
 * \param elapsed The time in ms it took to complete the write to backing store.
 * \return NSERROR_OK on success or appropriate error code.
 */
अटल nserror
write_backing_store(काष्ठा llcache_object *object,
		    माप_प्रकार *written_out,
		    अचिन्हित दीर्घ *elapsed)
{
	nserror ret;
	uint8_t *metadata;
	माप_प्रकार metadatasize;
	uint64_t startms = 0;
	uint64_t endms = 1000;

	nsu_getmonotonic_ms(&startms);

	/* put object data in backing store */
	ret = guit->llcache->store(object->url,
				   BACKING_STORE_NONE,
				   object->source_तथ्य,
				   object->source_len);
	अगर (ret != NSERROR_OK) {
		/* unable to put source data in backing store */
		वापस ret;
	}

	ret = llcache_serialise_metadata(object, &metadata, &metadatasize);
	अगर (ret != NSERROR_OK) {
		/* There has been a metadata serialisation error. Ensure the
		 * already written data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		वापस ret;
	}

	ret = guit->llcache->store(object->url,
				   BACKING_STORE_META,
				   metadata,
				   metadatasize);
	guit->llcache->release(object->url, BACKING_STORE_META);
	अगर (ret != NSERROR_OK) {
		/* There has been an error putting the metadata in the
		 * backing store. Ensure the data object is invalidated.
		 */
		guit->llcache->invalidate(object->url);
		वापस ret;
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
	अगर (*elapsed == 0) {
		*elapsed = 1;
	}

	वापस NSERROR_OK;
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
अटल व्योम llcache_persist_slowcheck(व्योम *p)
{
	uint64_t योग_bandwidth; /* total bandwidth */

	अगर (llcache->योग_written > (2 * llcache->minimum_bandwidth)) {

		योग_bandwidth = (llcache->योग_written * 1000) / llcache->योग_elapsed;

		अगर (योग_bandwidth < llcache->minimum_bandwidth) {
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
अटल व्योम llcache_persist(व्योम *p)
{
	nserror ret;
	काष्ठा llcache_object **lst; /* candidate object list */
	पूर्णांक lst_गिनती; /* number of candidates in list */
	पूर्णांक idx; /* current candidate object index in list */
	पूर्णांक अग्र = -1; /* when the next run should be scheduled for */

	अचिन्हित दीर्घ write_limit; /* max number of bytes to write in this run*/

	माप_प्रकार written; /* all bytes written for a single object */
	अचिन्हित दीर्घ elapsed; /* how long writing an object took */

	माप_प्रकार योग_written = 0; /* total bytes written in this run */
	अचिन्हित दीर्घ योग_elapsed = 1; /* total ms used to write bytes */
	अचिन्हित दीर्घ योग_bandwidth = 0; /* total bandwidth */

	ret = build_candidate_list(&lst, &lst_गिनती);
	अगर (ret != NSERROR_OK) {
		NSLOG(llcache, DEBUG, "Unable to construct candidate list for persistent writeout");
		वापस;
	}

	write_limit = (llcache->maximum_bandwidth * llcache->time_quantum) / 1000;

	/* obtained a candidate list, make each object persistent in turn */
	क्रम (idx = 0; idx < lst_गिनती; idx++) {
		ret = write_backing_store(lst[idx], &written, &elapsed);
		अगर (ret != NSERROR_OK) {
			जारी;
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
		अगर (योग_elapsed > llcache->time_quantum) {
			NSLOG(llcache, INFO, "Overran timeslot");
			/* writeout has exhausted the available time.
			 * Either the writeout is slow or the last
			 * object was very large.
			 */
			अगर (योग_bandwidth < llcache->minimum_bandwidth) {
				/* Writeout was slow in this time quantum.
				 *  Schedule a check in the future to see if
				 *  overall performance is too slow to be useful.
				 */
				guit->misc->schedule(
						llcache->time_quantum * 100,
						llcache_persist_slowcheck,
						शून्य);
				अवरोध;
			} अन्यथा {
				अगर (योग_bandwidth > llcache->maximum_bandwidth) {
					/* fast writeout of large file
					 * so calculate delay as if
					 * write happened only at max
					 * limit
					 */
					अग्र = ((योग_written * llcache->time_quantum) / write_limit) - योग_elapsed;
				} अन्यथा {
					अग्र = llcache->time_quantum;
				}
				अवरोध;
			}
		} अन्यथा अगर (योग_written > write_limit) {
			/* The bandwidth limit has been reached. */

			अगर (योग_bandwidth > llcache->maximum_bandwidth) {
				/* fast writeout of large file so
				 * calculate delay as if write
				 * happened only at max limit
				 */
				अग्र = ((योग_written * llcache->time_quantum) / write_limit) - योग_elapsed;
			} अन्यथा {
				अग्र = llcache->time_quantum - योग_elapsed;
			}
			अवरोध;
		}

	}
	मुक्त(lst);

	/* Completed list without running out of allowed bytes or time */
	अगर (idx == lst_गिनती) {
		/* only reschedule if writing is making any progress at all */
		अगर (योग_written > 0) {
			अग्र = llcache->time_quantum - योग_elapsed;
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
	guit->misc->schedule(अग्र, llcache_persist, शून्य);
}


/**
 * Handler for fetch events
 *
 * \param msg  Fetch event
 * \param p    Our private data
 */
अटल व्योम llcache_fetch_callback(स्थिर fetch_संदेश *संदेश, व्योम *p)
{
	nserror error = NSERROR_OK;
	llcache_object *object = p;
	llcache_event event;

	अगर (llcache == शून्य) {
		NSLOG(llcache, CRITICAL, "Callback happened after llcache finalisation");
		निश्चित(false);
		/* In case assertions are off, return here */
		वापस;
	}

	NSLOG(llcache, DEBUG, "Fetch event %d for %p", संदेश->type, object);

	चयन (संदेश->type) {
	हाल FETCH_HEADER:
		/* Received a fetch header */
		object->fetch.state = LLCACHE_FETCH_HEADERS;

		error = llcache_fetch_process_header(object,
				संदेश->तथ्य.header_or_तथ्य.buf,
				संदेश->तथ्य.header_or_तथ्य.len);
		अवरोध;

	/* 3xx responses */
	हाल FETCH_REDIRECT:
		/* Request resulted in a redirect */

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		error = llcache_fetch_redirect(object,
				संदेश->तथ्य.redirect, &object);
		अगर (error != NSERROR_OK) {
			event.type = LLCACHE_EVENT_ERROR;
			event.तथ्य.error.code = error;
			event.तथ्य.error.संदेश = शून्य;
			error = llcache_send_event_to_users(object, &event);
		}
		अवरोध;

	हाल FETCH_NOTMODIFIED:
		/* Conditional request determined that cached object is fresh */
		error = llcache_fetch_notmodified(object, &object);
		अवरोध;

	/* Normal 2xx state machine */
	हाल FETCH_तथ्य:
		/* Received some data */
		error = llcache_fetch_process_तथ्य(object,
				संदेश->तथ्य.header_or_तथ्य.buf,
				संदेश->तथ्य.header_or_तथ्य.len);
		अवरोध;

	हाल FETCH_FINISHED:
		/* Finished fetching */
	{
		uint8_t *अस्थायी;

		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = शून्य;

		/* Shrink source buffer to required size */
		अस्थायी = पुनः_स्मृति(object->source_तथ्य,
				object->source_len);
		/* If source_len is 0, then temp may be NULL */
		अगर (अस्थायी != शून्य || object->source_len == 0) {
			object->source_तथ्य = अस्थायी;
			object->source_alloc = object->source_len;
		}

		llcache_object_cache_update(object);

		/* record when the fetch finished */
		object->cache.fin_time = समय(शून्य);

		(व्योम) llcache_hsts_update_policy(object);

		guit->misc->schedule(5000, llcache_persist, शून्य);
	}
		अवरोध;

	/* Out-of-band information */
	हाल FETCH_TIMEDOUT:
		/* Timed out while trying to fetch. */
		error = llcache_fetch_timeout(object);
		अवरोध;

	हाल FETCH_ERROR:
		/* An error occurred while fetching */
		/* The fetch has has already been cleaned up by the fetcher */
		object->fetch.state = LLCACHE_FETCH_COMPLETE;
		object->fetch.fetch = शून्य;

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_तथ्य(object);

		/** \todo Consider using errorcode for something */

		event.type = LLCACHE_EVENT_ERROR;
		event.तथ्य.error.code = NSERROR_UNKNOWN;
		event.तथ्य.error.संदेश = संदेश->तथ्य.error;

		error = llcache_send_event_to_users(object, &event);

		अवरोध;

	हाल FETCH_PROGRESS:
		/* Progress update */
		event.type = LLCACHE_EVENT_PROGRESS;
		event.तथ्य.progress_संदेश = संदेश->तथ्य.progress;

		error = llcache_send_event_to_users(object, &event);

		अवरोध;

	हाल FETCH_CERTS:
		/* Certificate information from the fetch */

		अगर (object->chain != शून्य) {
			cert_chain_free(object->chain);
			object->chain = शून्य;
		}

		/* Persist the chain onto our object */
		error = cert_chain_dup(संदेश->तथ्य.chain, &object->chain);
		अगर (error != NSERROR_OK) {
			NSLOG(llcache, ERROR,
			      "Unable to duplicate cert chain into cache: %s",
			      messages_get_errorcode(error));
		}

		/* Now pass on the event */
		event.type = LLCACHE_EVENT_GOT_CERTS;
		event.तथ्य.chain = संदेश->तथ्य.chain;

		error = llcache_send_event_to_users(object, &event);

		अवरोध;

	/* Events requiring action */
	हाल FETCH_AUTH:
		/* Need Authentication */

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		error = llcache_fetch_auth(object, संदेश->तथ्य.auth.realm);
		अवरोध;

	हाल FETCH_CERT_ERR:
		/* Something went wrong when validating TLS certificates */

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		error = llcache_fetch_cert_error(object);
		अवरोध;

	हाल FETCH_SSL_ERR:
		/* TLS connection setup failed */

		/* Release candidate, if any */
		अगर (object->candidate != शून्य) {
			object->candidate->candidate_गिनती--;
			object->candidate = शून्य;
		}

		error = llcache_fetch_ssl_error(object);
		अवरोध;
	}

	/* Deal with any errors reported by event handlers */
	अगर (error != NSERROR_OK) {
		अगर (error == NSERROR_NOMEM) {
			/* attempt to purge the cache to free some
			 * memory. will not help this fetch, but may
			 * allow the UI to report errors etc.
			 */
			llcache_clean(true);
		}

		अगर (object->fetch.fetch != शून्य) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = शून्य;

			/* Invalidate cache control data */
			llcache_invalidate_cache_control_तथ्य(object);

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
अटल llcache_object_user *llcache_object_find_user(
		स्थिर llcache_handle *handle)
{
	llcache_object_user *user;

	निश्चित(handle->object != शून्य);

	क्रम (user = handle->object->users; user != शून्य; user = user->अग्र) {
		अगर (user->handle == handle)
			अवरोध;
	}

	वापस user;
}


/**
 * Determine if a low-level cache object resides in a given list
 *
 * \param object  Object to search for
 * \param list	  List to search in
 * \return True if object resides in list, false otherwise
 */
अटल bool llcache_object_in_list(स्थिर llcache_object *object,
		स्थिर llcache_object *list)
{
	जबतक (list != शून्य) {
		अगर (list == object)
			अवरोध;

		list = list->अग्र;
	}

	वापस list != शून्य;
}

/**
 * Notify users of an object's current state
 *
 * \param object  Object to notify users about
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror llcache_object_notify_users(llcache_object *object)
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

	क्रम (user = object->users; user != शून्य; user = अग्र_user) {
		/* Emit necessary events to bring the user up-to-date */
		llcache_handle *handle = user->handle;
		स्थिर llcache_fetch_state objstate = object->fetch.state;

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

		अगर (handle->state != objstate) {
			अगर (emitted_notify == false) {
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
		अगर (handle->state == LLCACHE_FETCH_INIT &&
				objstate > LLCACHE_FETCH_INIT) {
			handle->state = LLCACHE_FETCH_HEADERS;

			/* Emit any certificate data we hold */
			अगर (object->chain != शून्य) {
				event.type = LLCACHE_EVENT_GOT_CERTS;
				event.तथ्य.chain = object->chain;
				error = handle->cb(handle, &event, handle->pw);
			} अन्यथा {
				error = NSERROR_OK;
			}

			अगर (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				अगर (error != NSERROR_OK)
					वापस error;

				जारी;
			} अन्यथा अगर (error == NSERROR_NEED_तथ्य) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_up();
				जारी;
			} अन्यथा अगर (error != NSERROR_OK) {
				user->iterator_target = false;
				वापस error;
			}
		}

		/* User: HEADERS, Obj: DATA, COMPLETE => User->DATA */
		अगर (handle->state == LLCACHE_FETCH_HEADERS &&
				objstate > LLCACHE_FETCH_HEADERS) {
			handle->state = LLCACHE_FETCH_तथ्य;

			/* Emit HAD_HEADERS event */
			event.type = LLCACHE_EVENT_HAD_HEADERS;

			error = handle->cb(handle, &event, handle->pw);

			अगर (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				अगर (error != NSERROR_OK)
					वापस error;

				जारी;
			} अन्यथा अगर (error == NSERROR_NEED_तथ्य) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_HEADERS;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_up();
				जारी;
			} अन्यथा अगर (error != NSERROR_OK) {
				user->iterator_target = false;
				वापस error;
			}
		}

		/* User: DATA, Obj: DATA, COMPLETE, more source available */
		अगर (handle->state == LLCACHE_FETCH_तथ्य &&
				objstate >= LLCACHE_FETCH_तथ्य &&
				object->source_len > handle->bytes) {
			माप_प्रकार orig_handle_read;

			/* Construct HAD_DATA event */
			event.type = LLCACHE_EVENT_HAD_तथ्य;
			event.तथ्य.तथ्य.buf =
					object->source_तथ्य + handle->bytes;
			event.तथ्य.तथ्य.len =
					object->source_len - handle->bytes;

			/* Update record of last byte emitted */
			अगर (object->fetch.flags &
					LLCACHE_RETRIEVE_STREAM_तथ्य) {
				/* Streaming, so reset to zero to
				 * minimise amount of cached source data.
				 * Additionally, we don't support replay
				 * when streaming. */
				orig_handle_read = 0;
				handle->bytes = object->source_len = 0;
			} अन्यथा {
				orig_handle_read = handle->bytes;
				handle->bytes = object->source_len;
			}

			/* Emit event */
			error = handle->cb(handle, &event, handle->pw);
			अगर (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				अगर (error != NSERROR_OK)
					वापस error;

				जारी;
			} अन्यथा अगर (error == NSERROR_NEED_तथ्य) {
				/* User requested replay */
				handle->bytes = orig_handle_read;

				/* Continue with the next user -- we'll
				 * reemit the data next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_up();
				जारी;
			} अन्यथा अगर (error != NSERROR_OK) {
				user->iterator_target = false;
				वापस error;
			}
		}

		/* User: DATA, Obj: COMPLETE => User->COMPLETE */
		अगर (handle->state == LLCACHE_FETCH_तथ्य &&
				objstate > LLCACHE_FETCH_तथ्य) {
			handle->state = LLCACHE_FETCH_COMPLETE;

			/* Emit DONE event */
			event.type = LLCACHE_EVENT_DONE;

			error = handle->cb(handle, &event, handle->pw);
			अगर (user->queued_for_delete) {
				अग्र_user = user->अग्र;
				llcache_object_remove_user(object, user);
				llcache_object_user_destroy(user);

				अगर (error != NSERROR_OK)
					वापस error;

				जारी;
			} अन्यथा अगर (error == NSERROR_NEED_तथ्य) {
				/* User requested replay */
				handle->state = LLCACHE_FETCH_तथ्य;

				/* Continue with the next user -- we'll
				 * reemit the event next time round */
				user->iterator_target = false;
				अग्र_user = user->अग्र;
				llcache_users_not_caught_up();
				जारी;
			} अन्यथा अगर (error != NSERROR_OK) {
				user->iterator_target = false;
				वापस error;
			}
		}

		/* No longer the target of an iterator */
		user->iterator_target = false;

		अग्र_user = user->अग्र;
	}

	वापस NSERROR_OK;
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
अटल nserror
llcache_object_snapshot(llcache_object *object,	llcache_object **snapshot)
{
	llcache_object *newobj;
	nserror error;

	error = llcache_object_new(object->url, &newobj);

	अगर (error != NSERROR_OK)
		वापस error;

	newobj->source_alloc = newobj->source_len = object->source_len;

	अगर (object->source_len > 0) {
		newobj->source_तथ्य = दो_स्मृति(newobj->source_alloc);
		अगर (newobj->source_तथ्य == शून्य) {
			llcache_object_destroy(newobj);
			वापस NSERROR_NOMEM;
		}
		स_नकल(newobj->source_तथ्य, object->source_तथ्य,
				newobj->source_len);
	}

	अगर (object->अंक_headers > 0) {
		newobj->headers = सुस्मृति(object->अंक_headers,
				माप(llcache_header));
		अगर (newobj->headers == शून्य) {
			llcache_object_destroy(newobj);
			वापस NSERROR_NOMEM;
		}
		जबतक (newobj->अंक_headers < object->अंक_headers) {
			llcache_header *nh =
					&(newobj->headers[newobj->अंक_headers]);
			llcache_header *oh =
					&(object->headers[newobj->अंक_headers]);
			newobj->अंक_headers += 1;
			nh->name = strdup(oh->name);
			nh->मूल्य = strdup(oh->मूल्य);
			अगर (nh->name == शून्य || nh->मूल्य == शून्य) {
				llcache_object_destroy(newobj);
				वापस NSERROR_NOMEM;
			}
		}
	}

	अगर (object->chain != शून्य) {
		error = cert_chain_dup(object->chain, &newobj->chain);
		अगर (error != NSERROR_OK) {
			llcache_object_destroy(newobj);
			वापस error;
		}
	}

	newobj->fetch.state = LLCACHE_FETCH_COMPLETE;

	*snapshot = newobj;

	वापस NSERROR_OK;
}

/**
 * total ram usage of object
 *
 * \param object The object to calculate the total RAM usage of.
 * \return The total RAM usage in bytes.
 */
अटल अंतरभूत uint32_t
योग_object_आकार(llcache_object *object)
{
	uint32_t tot;
	माप_प्रकार hdrc;

	tot = माप(*object);
	tot += nsurl_लंबाई(object->url);

	अगर (object->source_तथ्य != शून्य) {
		tot += object->source_len;
	}

	tot += माप(llcache_header) * object->अंक_headers;

	क्रम (hdrc = 0; hdrc < object->अंक_headers; hdrc++) {
		अगर (object->headers[hdrc].name != शून्य) {
			tot += म_माप(object->headers[hdrc].name);
		}
		अगर (object->headers[hdrc].मूल्य != शून्य) {
			tot += म_माप(object->headers[hdrc].मूल्य);
		}
	}

	tot += cert_chain_आकार(object->chain);

	वापस tot;
}

/**
 * Catch up the cache users with state changes from fetchers.
 *
 * \param ignored We ignore this because all our state comes from llcache.
 */
अटल व्योम llcache_catch_up_all_users(व्योम *ignored)
{
	llcache_object *object;

	/* Assume after this we'll be all caught up.  If any user of a handle
	 * defers then we'll invalidate all_caught_up and reschedule via
	 * llcache_users_not_caught_up()
	 */
	llcache->all_caught_up = true;

	/* Catch new users up with state of objects */
	क्रम (object = llcache->cached_objects; object != शून्य;
			object = object->अग्र) {
		llcache_object_notify_users(object);
	}

	क्रम (object = llcache->uncached_objects; object != शून्य;
			object = object->अग्र) {
		llcache_object_notify_users(object);
	}
}

/**
 * Ask for ::llcache_catch_up_all_users to be scheduled ASAP to pump the
 * user state machines.
 */
अटल व्योम llcache_users_not_caught_up(व्योम)
{
	अगर (llcache->all_caught_up) {
		llcache->all_caught_up = false;
		guit->misc->schedule(0, llcache_catch_up_all_users, शून्य);
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
व्योम llcache_clean(bool purge)
{
	llcache_object *object, *अग्र;
	uint32_t llcache_आकार = 0;
	पूर्णांक remaining_lifetime;
	uint32_t limit;

	NSLOG(llcache, DEBUG, "Attempting cache clean");

	/* If the cache is being purged set the size limit to zero. */
	अगर (purge) {
		limit = 0;
	} अन्यथा {
		limit = llcache->limit;
	}

	/* Uncacheable objects with no users or fetches */
	क्रम (object = llcache->uncached_objects;
	     object != शून्य;
	     object = अग्र) {
		अग्र = object->अग्र;

		/* The candidate count of uncacheable objects is always 0 */
		अगर ((object->users == शून्य) &&
		    (object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य)) {
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
	क्रम (object = llcache->cached_objects;
	     object != शून्य;
	     object = अग्र) {
		अग्र = object->अग्र;

		remaining_lifetime = llcache_object_rfc2616_remaining_lifetime(
				&object->cache);

		अगर ((object->users == शून्य) &&
		    (object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य) &&
		    (remaining_lifetime <= 0)) {
			/* object is stale */
			NSLOG(llcache, DEBUG, "discarding stale cacheable object with no "
					"users or pending fetches (%p) %s",
					object, nsurl_access(object->url));

				llcache_object_remove_from_list(object,
						&llcache->cached_objects);

				अगर (object->store_state == LLCACHE_STATE_DISC) {
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
	अगर (limit < llcache_आकार) {
		llcache_persist(शून्य);
	}

	/* Source data of fresh cacheable objects with no users, no
	 * pending fetches and pushed to persistent store while the
	 * cache exceeds the configured size.
	 */
	क्रम (object = llcache->cached_objects;
	     ((limit < llcache_आकार) && (object != शून्य));
	     object = अग्र) {
		अग्र = object->अग्र;
		अगर ((object->users == शून्य) &&
		    (object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य) &&
		    (object->store_state == LLCACHE_STATE_DISC)) {
			guit->llcache->release(object->url, BACKING_STORE_NONE);

			object->source_तथ्य = शून्य;

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
	क्रम (object = llcache->cached_objects;
	     ((limit < llcache_आकार) && (object != शून्य));
	     object = अग्र) {
		अग्र = object->अग्र;
		अगर ((object->users == शून्य) &&
		    (object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य) &&
		    (object->store_state == LLCACHE_STATE_DISC) &&
		    (object->source_तथ्य == शून्य)) {
			NSLOG(llcache, DEBUG,
			     "discarding backed object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (दीर्घ)(समय(शून्य) - object->last_used),
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
	क्रम (object = llcache->cached_objects;
	     ((limit < llcache_आकार) && (object != शून्य));
	     object = अग्र) {
		अग्र = object->अग्र;

		अगर ((object->users == शून्य) &&
		    (object->candidate_गिनती == 0) &&
		    (object->fetch.fetch == शून्य) &&
		    (object->store_state == LLCACHE_STATE_RAM)) {
			NSLOG(llcache, DEBUG,
			      "discarding fresh object len:%"PRIsizet" age:%ld (%p) %s",
			      object->source_len,
			      (दीर्घ)(समय(शून्य) - object->last_used),
			      object,
			      nsurl_access(object->url));

			llcache_आकार -=	object->source_len + माप(*object);

			llcache_object_remove_from_list(object,
						&llcache->cached_objects);
			llcache_object_destroy(object);
		}
	}

	NSLOG(llcache, DEBUG, "Size: %"PRIu32" (limit: %"PRIu32")", llcache_आकार, limit);
}

/* Exported interface documented in content/llcache.h */
nserror
llcache_initialise(स्थिर काष्ठा llcache_parameters *prm)
{
	llcache = सुस्मृति(1, माप(काष्ठा llcache_s));
	अगर (llcache == शून्य) {
		वापस NSERROR_NOMEM;
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
	वापस guit->llcache->initialise(&prm->store);
}


/* Exported interface documented in content/llcache.h */
व्योम llcache_finalise(व्योम)
{
	llcache_object *object, *अग्र;
	uint64_t योग_bandwidth = 0; /* total bandwidth */

	/* Attempt to persist anything we have left lying around */
	llcache_persist(शून्य);
	/* Now clear the persistence callback */
	guit->misc->schedule(-1, llcache_persist, शून्य);

	/* Clean uncached objects */
	क्रम (object = llcache->uncached_objects; object != शून्य; object = अग्र) {
		llcache_object_user *user, *अग्र_user;

		अग्र = object->अग्र;

		क्रम (user = object->users; user != शून्य; user = अग्र_user) {
			अग्र_user = user->अग्र;

			अगर (user->handle != शून्य)
				मुक्त(user->handle);

			मुक्त(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = शून्य;

		llcache_object_destroy(object);
	}

	/* Clean cached objects */
	क्रम (object = llcache->cached_objects; object != शून्य; object = अग्र) {
		llcache_object_user *user, *अग्र_user;

		अग्र = object->अग्र;

		क्रम (user = object->users; user != शून्य; user = अग्र_user) {
			अग्र_user = user->अग्र;

			अगर (user->handle != शून्य)
			       मुक्त(user->handle);

			मुक्त(user);
		}

		/* Fetch system has already been destroyed */
		object->fetch.fetch = शून्य;

		llcache_object_destroy(object);
	}

	/* backing store finalisation */
	guit->llcache->finalise();

	अगर (llcache->योग_elapsed > 0) {
		योग_bandwidth = (llcache->योग_written * 1000) /
			llcache->योग_elapsed;
	}

	NSLOG(llcache, INFO,
	      "Backing store wrote %"PRIu64" bytes in %"PRIu64" ms ""(average %"PRIu64" bytes/second)",
	      llcache->योग_written,
	      llcache->योग_elapsed,
	      योग_bandwidth);

	मुक्त(llcache);
	llcache = शून्य;
}



/* Exported interface documented in content/llcache.h */
nserror
llcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			स्थिर llcache_post_तथ्य *post,
			llcache_handle_callback cb, व्योम *pw,
			llcache_handle **परिणाम)
{
	nserror error;
	llcache_object_user *user;
	llcache_object *object;
	nsurl *hsts_url;
	bool hsts_in_use;

	/* Perform HSTS transform */
	error = llcache_hsts_transform_url(url, &hsts_url, &hsts_in_use);
	अगर (error != NSERROR_OK) {
		वापस error;
	}

	/* Can we fetch this URL at all? */
	अगर (fetch_can_fetch(hsts_url) == false) {
		nsurl_unref(hsts_url);
		वापस NSERROR_NO_FETCH_HANDLER;
	}

	/* Create a new object user */
	error = llcache_object_user_new(cb, pw, &user);
	अगर (error != NSERROR_OK) {
		nsurl_unref(hsts_url);
		वापस error;
	}

	/* Retrieve a suitable object from the cache,
	 * creating a new one if needed. */
	error = llcache_object_retrieve(hsts_url, flags, referer, post, 0,
			hsts_in_use, &object);
	अगर (error != NSERROR_OK) {
		llcache_object_user_destroy(user);
		nsurl_unref(hsts_url);
		वापस error;
	}

	/* Add user to object */
	llcache_object_add_user(object, user);

	*परिणाम = user->handle;

	/* Users exist which are now not caught up! */
	llcache_users_not_caught_up();

	nsurl_unref(hsts_url);

	वापस NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_change_callback(llcache_handle *handle,
		llcache_handle_callback cb, व्योम *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	वापस NSERROR_OK;
}


/* Exported interface documented in content/llcache.h */
nserror llcache_handle_release(llcache_handle *handle)
{
	nserror error = NSERROR_OK;
	llcache_object *object = handle->object;
	llcache_object_user *user = llcache_object_find_user(handle);

	निश्चित(user != शून्य);

	अगर (user->iterator_target) {
		/* Can't remove / delete user object if it's
		 * the target of an iterator */
		user->queued_for_delete = true;
	} अन्यथा {
		/* Remove the user from the object and destroy it */
		error = llcache_object_remove_user(object, user);
		अगर (error == NSERROR_OK) {
			error = llcache_object_user_destroy(user);
		}
	}

	वापस error;
}

/* Exported interface documented in content/llcache.h */
nserror llcache_handle_clone(llcache_handle *handle, llcache_handle **परिणाम)
{
	nserror error;
	llcache_object_user *newuser;

	error = llcache_object_user_new(handle->cb, handle->pw, &newuser);
	अगर (error == NSERROR_OK) {
		llcache_object_add_user(handle->object, newuser);
		newuser->handle->state = handle->state;
		*परिणाम = newuser->handle;
	}

	वापस error;
}

/* See llcache.h for documentation */
nserror llcache_handle_abort(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object, *newobject;
	nserror error = NSERROR_OK;
	bool all_alone = true;

	/* Determine if we are the only user */
	अगर (user->पूर्व != शून्य)
		all_alone = false;
	अगर (user->अग्र != शून्य)
		all_alone = false;

	अगर (all_alone == false) {
		/* We must snapshot this object */
		error = llcache_object_snapshot(object, &newobject);
		अगर (error != NSERROR_OK)
			वापस error;

		/* Move across to the new object */
		अगर (user->iterator_target) {
			/* User is current iterator target, clone it */
			llcache_object_user *newuser =
					सुस्मृति(1, माप(llcache_object_user));
			अगर (newuser == शून्य) {
				llcache_object_destroy(newobject);
				वापस NSERROR_NOMEM;
			}

			/* Move handle across to clone */
			newuser->handle = user->handle;
			user->handle = शून्य;

			/* Mark user as needing deletion */
			user->queued_for_delete = true;

			llcache_object_add_user(newobject, newuser);
		} अन्यथा {
			llcache_object_remove_user(object, user);
			llcache_object_add_user(newobject, user);
		}

		/* Add new object to uncached list */
		llcache_object_add_to_list(newobject,
				&llcache->uncached_objects);
	} अन्यथा {
		/* We're the only user, so abort any fetch in progress */
		अगर (object->fetch.fetch != शून्य) {
			fetch_abort(object->fetch.fetch);
			object->fetch.fetch = शून्य;
		}

		object->fetch.state = LLCACHE_FETCH_COMPLETE;

		/* Invalidate cache control data */
		llcache_invalidate_cache_control_तथ्य(object);
	}

	वापस error;
}

/* See llcache.h for documentation */
nserror llcache_handle_force_stream(llcache_handle *handle)
{
	llcache_object_user *user = llcache_object_find_user(handle);
	llcache_object *object = handle->object;

	/* Cannot stream if there are multiple users */
	अगर (user->पूर्व != शून्य || user->अग्र != शून्य)
		वापस NSERROR_OK;

	/* Forcibly uncache this object */
	अगर (llcache_object_in_list(object, llcache->cached_objects)) {
		llcache_object_remove_from_list(object,
				&llcache->cached_objects);
		llcache_object_add_to_list(object, &llcache->uncached_objects);
	}

	object->fetch.flags |= LLCACHE_RETRIEVE_STREAM_तथ्य;

	वापस NSERROR_OK;
}

/* See llcache.h for documentation */
nserror llcache_handle_invalidate_cache_तथ्य(llcache_handle *handle)
{
	अगर ((handle->object != शून्य) &&
	    (handle->object->fetch.fetch == शून्य) &&
	    (handle->object->cache.no_cache == LLCACHE_VALIDATE_FRESH)) {
		/* mark the cached object as requiring validation */
		handle->object->cache.no_cache = LLCACHE_VALIDATE_ONCE;
	}

	वापस NSERROR_OK;
}

/* See llcache.h for documentation */
nsurl *llcache_handle_get_url(स्थिर llcache_handle *handle)
{
	वापस handle->object != शून्य ? handle->object->url : शून्य;
}

/* See llcache.h for documentation */
स्थिर uint8_t *llcache_handle_get_source_तथ्य(स्थिर llcache_handle *handle,
		माप_प्रकार *आकार)
{
	*आकार = handle->object != शून्य ? handle->object->source_len : 0;

	वापस handle->object != शून्य ? handle->object->source_तथ्य : शून्य;
}

/* See llcache.h for documentation */
स्थिर अक्षर *llcache_handle_get_header(स्थिर llcache_handle *handle,
		स्थिर अक्षर *कुंजी)
{
	स्थिर llcache_object *object = handle->object;
	माप_प्रकार i;

	अगर (object == शून्य)
		वापस शून्य;

	/* About as trivial as possible */
	क्रम (i = 0; i < object->अंक_headers; i++) {
		अगर (strcasecmp(कुंजी, object->headers[i].name) == 0)
			वापस object->headers[i].मूल्य;
	}

	वापस शून्य;
}

/* See llcache.h for documentation */
bool llcache_handle_references_same_object(स्थिर llcache_handle *a,
		स्थिर llcache_handle *b)
{
	वापस a->object == b->object;
}
