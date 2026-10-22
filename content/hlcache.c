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
 * High-level resource cache implementation.
 */

#समावेश <assert.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "utils/http.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/ring.h"
#समावेश "utils/utils.h"
#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/misc.h"
#समावेश "netsurf/content.h"
#समावेश "desktop/gui_internal.h"

#समावेश "content/mimesniff.h"
#समावेश "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"

प्रकार काष्ठा hlcache_entry hlcache_entry;
प्रकार काष्ठा hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
काष्ठा hlcache_retrieval_ctx {
	काष्ठा hlcache_retrieval_ctx *r_पूर्व; /**< Previous retrieval context in the ring */
	काष्ठा hlcache_retrieval_ctx *r_अग्र; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
काष्ठा hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	व्योम *pw;			/**< Client data */
};

/** Entry in high-level cache */
काष्ठा hlcache_entry {
	काष्ठा content *content;	/**< Pointer to associated content */

	hlcache_entry *अग्र;		/**< Next sibling */
	hlcache_entry *पूर्व;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
काष्ठा hlcache_s {
	काष्ठा hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	अचिन्हित पूर्णांक hit_गिनती;
	अचिन्हित पूर्णांक miss_गिनती;
};

/** high level cache state */
अटल काष्ठा hlcache_s *hlcache = शून्य;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
अटल व्योम hlcache_clean(व्योम *force_clean_flag)
{
	hlcache_entry *entry, *अग्र;
	bool force_clean = (force_clean_flag != शून्य);

	क्रम (entry = hlcache->content_list; entry != शून्य; entry = अग्र) {
		अग्र = entry->अग्र;

		अगर (entry->content == शून्य)
			जारी;

		अगर (content_गिनती_users(entry->content) != 0)
			जारी;

		अगर (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			अगर (force_clean == false)
				जारी;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(entry->content);
			content_set_error(entry->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		अगर (entry->पूर्व == शून्य)
			hlcache->content_list = entry->अग्र;
		अन्यथा
			entry->पूर्व->अग्र = entry->अग्र;

		अगर (entry->अग्र != शून्य)
			entry->अग्र->पूर्व = entry->पूर्व;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		मुक्त(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, शून्य);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
अटल bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	वापस ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
अटल व्योम hlcache_content_callback(काष्ठा content *c, content_संदेश संदेश,
		स्थिर जोड़ content_संदेश_तथ्य *तथ्य, व्योम *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = संदेश,
	};

	अगर (तथ्य != शून्य) {
		event.तथ्य = *तथ्य;
	}

	अगर (handle->cb != शून्य)
		error = handle->cb(handle, &event, handle->pw);

	अगर (error != NSERROR_OK)
		NSLOG(netsurf, INFO, "Error in callback: %d", error);
}

/**
 * Find a content for the high-level cache handle
 *
 * \param ctx             High-level cache retrieval context
 * \param effective_type  Effective MIME type of content
 * \return NSERROR_OK on success,
 *         NSERROR_NEED_DATA on success where data is needed,
 *         appropriate error otherwise
 *
 * \pre handle::state == HLCACHE_HANDLE_NEW
 * \pre Headers must have been received for associated low-level handle
 * \post Low-level handle is either released, or associated with new content
 * \post High-level handle is registered with content
 */
अटल nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	क्रम (entry = hlcache->content_list; entry != शून्य; entry = entry->अग्र) {
		hlcache_handle entry_handle = { entry, शून्य, शून्य };
		स्थिर llcache_handle *entry_llcache;

		अगर (entry->content == शून्य)
			जारी;

		/* Ignore contents in the error state */
		अगर (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			जारी;

		/* Ensure that content is shareable */
		अगर (content_is_shareable(entry->content) == false)
			जारी;

		/* Ensure that quirks mode is acceptable */
		अगर (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			जारी;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		अगर (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			अवरोध;
	}

	अगर (entry == शून्य) {
		/* No existing entry, so need to create one */
		entry = दो_स्मृति(माप(hlcache_entry));
		अगर (entry == शून्य)
			वापस NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		अगर (entry->content == शून्य) {
			मुक्त(entry);
			वापस NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->पूर्व = शून्य;
		entry->अग्र = hlcache->content_list;
		अगर (hlcache->content_list != शून्य)
			hlcache->content_list->पूर्व = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_तथ्य;

		hlcache->miss_गिनती++;
	} अन्यथा {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_गिनती++;
	}

	/* Associate handle with content */
	अगर (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		वापस NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	अगर (ctx->handle->cb != शून्य) {
		content_status status = content_get_status(ctx->handle);

		अगर (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} अन्यथा अगर (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			अगर (ctx->handle->cb != शून्य) {
				event.type = CONTENT_संदेश_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} अन्यथा अगर (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			अगर (ctx->handle->cb != शून्य) {
				event.type = CONTENT_संदेश_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			अगर (ctx->handle->cb != शून्य) {
				event.type = CONTENT_संदेश_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	वापस error;
}

/**
 * Migrate a retrieval context into its final destination content
 *
 * \param ctx             Context to migrate
 * \param effective_type  The effective MIME type of the content, or NULL
 * \return NSERROR_OK on success,
 *         NSERROR_NEED_DATA on success where data is needed,
 *         appropriate error otherwise
 */
अटल nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	अगर ((effective_type != शून्य) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		अगर (error != NSERROR_OK && error != NSERROR_NEED_तथ्य) {
			अगर (ctx->handle->cb != शून्य) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_संदेश_ERROR;
				hlevent.तथ्य.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.तथ्य.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} अन्यथा अगर (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_DOWNLOAD;
			hlevent.तथ्य.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_तथ्य;
	} अन्यथा {
		/* Unacceptable type: report error */
		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.तथ्य.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.तथ्य.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_target = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	मुक्त((अक्षर *) ctx->child.charset);
	मुक्त(ctx);

	वापस error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
अटल nserror
hlcache_llcache_callback(llcache_handle *handle,
			 स्थिर llcache_event *event,
			 व्योम *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = शून्य;
	nserror error;

	निश्चित(ctx->llcache == handle);

	चयन (event->type) {
	हाल LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_SSL_CERTS;
			hlevent.तथ्य.chain = event->तथ्य.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		अवरोध;
	हाल LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), शून्य, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		अगर (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		अगर (error == NSERROR_NEED_तथ्य)
			error = NSERROR_OK;

		वापस error;

		अवरोध;
	हाल LLCACHE_EVENT_HAD_तथ्य:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->तथ्य.तथ्य.buf, event->तथ्य.तथ्य.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		अगर (error != NSERROR_OK) {
			निश्चित(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		वापस error;

		अवरोध;
	हाल LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				शून्य, 0, false, false, &effective_type);
		अगर (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			वापस error;
		}

		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.तथ्य.errordata.errorcode = error;
			hlevent.तथ्य.errordata.errormsg = शून्य;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		अवरोध;
	हाल LLCACHE_EVENT_ERROR:
		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.तथ्य.errordata.errorcode = event->तथ्य.error.code;
			hlevent.तथ्य.errordata.errormsg = event->तथ्य.error.संदेश;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		अवरोध;
	हाल LLCACHE_EVENT_PROGRESS:
		अवरोध;
	हाल LLCACHE_EVENT_REDIRECT:
		अगर (ctx->handle->cb != शून्य) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_REDIRECT;
			hlevent.तथ्य.redirect.from = event->तथ्य.redirect.from;
			hlevent.तथ्य.redirect.to = event->तथ्य.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		अवरोध;
	}

	वापस NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(स्थिर काष्ठा hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = सुस्मृति(1, माप(काष्ठा hlcache_s));
	अगर (hlcache == शून्य) {
		वापस NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	अगर (ret != NSERROR_OK) {
		मुक्त(hlcache);
		hlcache = शून्य;
		वापस ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, शून्य);

	वापस NSERROR_OK;
}

/* See hlcache.h for documentation */
व्योम hlcache_stop(व्योम)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, शून्य);
}

/* See hlcache.h for documentation */
व्योम hlcache_finalise(व्योम)
{
	uint32_t अंक_contents, पूर्व_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *अग्र;

	/* Obtain initial count of contents remaining */
	क्रम (अंक_contents = 0, entry = hlcache->content_list;
			entry != शून्य; entry = entry->अग्र) {
		अंक_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      अंक_contents);

	/* Drain cache */
	करो {
		पूर्व_contents = अंक_contents;

		hlcache_clean(शून्य);

		क्रम (अंक_contents = 0, entry = hlcache->content_list;
				entry != शून्य; entry = entry->अग्र) {
			अंक_contents++;
		}
	} जबतक (अंक_contents > 0 && अंक_contents != पूर्व_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			अंक_contents);

	/* Drain cache again, forcing the matter */
	करो {
		पूर्व_contents = अंक_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		क्रम (अंक_contents = 0, entry = hlcache->content_list;
				entry != शून्य; entry = entry->अग्र) {
			अंक_contents++;
		}
	} जबतक (अंक_contents > 0 && अंक_contents != पूर्व_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", अंक_contents);
	क्रम (entry = hlcache->content_list; entry != शून्य; entry = entry->अग्र) {
		hlcache_handle entry_handle = { entry, शून्य, शून्य };

		अगर (entry->content != शून्य) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_गिनती_users(entry->content));
		} अन्यथा {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	अगर (hlcache->retrieval_ctx_ring != शून्य) {
		ctx = hlcache->retrieval_ctx_ring;

		करो {
			अग्र = ctx->r_अग्र;

			अगर (ctx->llcache != शून्य)
				llcache_handle_release(ctx->llcache);

			अगर (ctx->handle != शून्य)
				मुक्त(ctx->handle);

			अगर (ctx->child.charset != शून्य)
				मुक्त((अक्षर *) ctx->child.charset);

			मुक्त(ctx);

			ctx = अग्र;
		} जबतक (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = शून्य;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_गिनती,
	      hlcache->miss_गिनती);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, शून्य);

	मुक्त(hlcache);
	hlcache = शून्य;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_तथ्य *post,
			hlcache_handle_callback cb, व्योम *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **परिणाम)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	निश्चित(cb != शून्य);

	ctx = सुस्मृति(1, माप(hlcache_retrieval_ctx));
	अगर (ctx == शून्य) {
		वापस NSERROR_NOMEM;
	}

	ctx->handle = सुस्मृति(1, माप(hlcache_handle));
	अगर (ctx->handle == शून्य) {
		मुक्त(ctx);
		वापस NSERROR_NOMEM;
	}

	अगर (child != शून्य) {
		अगर (child->charset != शून्य) {
			ctx->child.charset = strdup(child->charset);
			अगर (ctx->child.charset == शून्य) {
				मुक्त(ctx->handle);
				मुक्त(ctx);
				वापस NSERROR_NOMEM;
			}
		}
		ctx->child.quirks = child->quirks;
	}

	ctx->flags = flags;
	ctx->accepted_types = accepted_types;

	ctx->handle->cb = cb;
	ctx->handle->pw = pw;

	error = llcache_handle_retrieve(url, flags, referer, post,
			hlcache_llcache_callback, ctx,
			&ctx->llcache);
	अगर (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		मुक्त((अक्षर *) ctx->child.charset);
		मुक्त(ctx->handle);
		मुक्त(ctx);
	} अन्यथा {
		/* successfully started fetch so add new context to list */
		RING_जोड़ो(hlcache->retrieval_ctx_ring, ctx);

		*परिणाम = ctx->handle;
	}
	वापस error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	अगर (handle->entry != शून्य) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} अन्यथा {
		RING_ITERATE_START(काष्ठा hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			अगर (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((अक्षर *) ictx->child.charset);
				मुक्त(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = शून्य;
	handle->pw = शून्य;

	मुक्त(handle);

	वापस NSERROR_OK;
}

/* See hlcache.h for documentation */
काष्ठा content *hlcache_handle_get_content(स्थिर hlcache_handle *handle)
{
	अगर ((handle != शून्य) && (handle->entry != शून्य)) {
		वापस handle->entry->content;
	}

	वापस शून्य;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	काष्ठा hlcache_entry *entry = handle->entry;
	काष्ठा content *c;

	अगर (entry == शून्य) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(काष्ठा hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			अगर (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((अक्षर *) ictx->child.charset);
				मुक्त(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		वापस NSERROR_OK;
	}

	c = entry->content;

	अगर (content_गिनती_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		काष्ठा content *clone = content_clone(c);

		अगर (clone == शून्य)
			वापस NSERROR_NOMEM;

		entry = सुस्मृति(1, माप(काष्ठा hlcache_entry));

		अगर (entry == शून्य) {
			content_destroy(clone);
			वापस NSERROR_NOMEM;
		}

		अगर (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			मुक्त(entry);
			वापस NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->पूर्व = शून्य;
		entry->अग्र = hlcache->content_list;
		अगर (hlcache->content_list != शून्य)
			hlcache->content_list->पूर्व = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	वापस content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, व्योम *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	वापस NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **परिणाम)
{
	*परिणाम = शून्य;
	वापस NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(स्थिर hlcache_handle *handle)
{
	nsurl *परिणाम = शून्य;

	निश्चित(handle != शून्य);

	अगर (handle->entry != शून्य) {
		परिणाम = content_get_url(handle->entry->content);
	} अन्यथा {
		RING_ITERATE_START(काष्ठा hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			अगर (ictx->handle == handle) {
				/* This is the nascent context for us */
				परिणाम = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	वापस परिणाम;
}
