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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/mimesniff.h"
#अन्तर्भाव "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"

प्रकारव्याख्या संरचना hlcache_entry hlcache_entry;
प्रकारव्याख्या संरचना hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
संरचना hlcache_retrieval_ctx {
	संरचना hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	संरचना hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
संरचना hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	शून्य *pw;			/**< Client data */
};

/** Entry in high-level cache */
संरचना hlcache_entry {
	संरचना content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
संरचना hlcache_s {
	संरचना hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	अचिन्हित पूर्णांक hit_मोजणी;
	अचिन्हित पूर्णांक miss_मोजणी;
};

/** high level cache state */
स्थिर संरचना hlcache_s *hlcache = रिक्त;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
स्थिर शून्य hlcache_clean(शून्य *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != रिक्त);

	साठी (entry = hlcache->content_list; entry != रिक्त; entry = next) {
		next = entry->next;

		जर (entry->content == रिक्त)
			सुरूठेवा;

		जर (content_मोजणी_users(entry->content) != 0)
			सुरूठेवा;

		जर (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			जर (force_clean == false)
				सुरूठेवा;
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
		जर (entry->prev == रिक्त)
			hlcache->content_list = entry->next;
		अन्यथा
			entry->prev->next = entry->next;

		जर (entry->next != रिक्त)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		मुक्त(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, रिक्त);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
स्थिर bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	परत ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
स्थिर शून्य hlcache_content_callback(संरचना content *c, content_संदेश संदेश,
		स्थिरांक संघ content_संदेश_data *data, शून्य *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = संदेश,
	};

	जर (data != रिक्त) {
		event.data = *data;
	}

	जर (handle->cb != रिक्त)
		error = handle->cb(handle, &event, handle->pw);

	जर (error != NSERROR_OK)
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
स्थिर nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	साठी (entry = hlcache->content_list; entry != रिक्त; entry = entry->next) {
		hlcache_handle entry_handle = { entry, रिक्त, रिक्त };
		स्थिरांक llcache_handle *entry_llcache;

		जर (entry->content == रिक्त)
			सुरूठेवा;

		/* Ignore contents in the error state */
		जर (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			सुरूठेवा;

		/* Ensure that content is shareable */
		जर (content_is_shareable(entry->content) == false)
			सुरूठेवा;

		/* Ensure that quirks mode is acceptable */
		जर (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			सुरूठेवा;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		जर (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			थांबा;
	}

	जर (entry == रिक्त) {
		/* No existing entry, so need to create one */
		entry = दो_स्मृति(आकार(hlcache_entry));
		जर (entry == रिक्त)
			परत NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		जर (entry->content == रिक्त) {
			मुक्त(entry);
			परत NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = रिक्त;
		entry->next = hlcache->content_list;
		जर (hlcache->content_list != रिक्त)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_मोजणी++;
	} अन्यथा {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_मोजणी++;
	}

	/* Associate handle with content */
	जर (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		परत NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	जर (ctx->handle->cb != रिक्त) {
		content_status status = content_get_status(ctx->handle);

		जर (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} अन्यथा जर (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			जर (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_संदेश_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} अन्यथा जर (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_संदेश_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			जर (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_संदेश_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			जर (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_संदेश_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	परत error;
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
स्थिर nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	जर ((effective_type != रिक्त) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		जर (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			जर (ctx->handle->cb != रिक्त) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_संदेश_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} अन्यथा जर (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} अन्यथा {
		/* Unacceptable type: report error */
		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.data.errordata.errormsg = messages_get("UnacceptableType");

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

	परत error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror
hlcache_llcache_callback(llcache_handle *handle,
			 स्थिरांक llcache_event *event,
			 शून्य *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = रिक्त;
	nserror error;

	प्रतिज्ञा(ctx->llcache == handle);

	निवड (event->type) {
	स्थिती LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		थांबा;
	स्थिती LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), रिक्त, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		जर (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		जर (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		परत error;

		थांबा;
	स्थिती LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		जर (error != NSERROR_OK) {
			प्रतिज्ञा(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		परत error;

		थांबा;
	स्थिती LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				रिक्त, 0, false, false, &effective_type);
		जर (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			परत error;
		}

		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = रिक्त;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		थांबा;
	स्थिती LLCACHE_EVENT_ERROR:
		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.संदेश;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		थांबा;
	स्थिती LLCACHE_EVENT_PROGRESS:
		थांबा;
	स्थिती LLCACHE_EVENT_REDIRECT:
		जर (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_संदेश_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		थांबा;
	}

	परत NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(स्थिरांक संरचना hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = सुस्मृति(1, आकार(संरचना hlcache_s));
	जर (hlcache == रिक्त) {
		परत NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	जर (ret != NSERROR_OK) {
		मुक्त(hlcache);
		hlcache = रिक्त;
		परत ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, रिक्त);

	परत NSERROR_OK;
}

/* See hlcache.h for documentation */
शून्य hlcache_stop(शून्य)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, रिक्त);
}

/* See hlcache.h for documentation */
शून्य hlcache_finalise(शून्य)
{
	uint32_t अंक_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	साठी (अंक_contents = 0, entry = hlcache->content_list;
			entry != रिक्त; entry = entry->next) {
		अंक_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      अंक_contents);

	/* Drain cache */
	करा {
		prev_contents = अंक_contents;

		hlcache_clean(रिक्त);

		साठी (अंक_contents = 0, entry = hlcache->content_list;
				entry != रिक्त; entry = entry->next) {
			अंक_contents++;
		}
	} पर्यंत (अंक_contents > 0 && अंक_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			अंक_contents);

	/* Drain cache again, forcing the matter */
	करा {
		prev_contents = अंक_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		साठी (अंक_contents = 0, entry = hlcache->content_list;
				entry != रिक्त; entry = entry->next) {
			अंक_contents++;
		}
	} पर्यंत (अंक_contents > 0 && अंक_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", अंक_contents);
	साठी (entry = hlcache->content_list; entry != रिक्त; entry = entry->next) {
		hlcache_handle entry_handle = { entry, रिक्त, रिक्त };

		जर (entry->content != रिक्त) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_मोजणी_users(entry->content));
		} अन्यथा {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	जर (hlcache->retrieval_ctx_ring != रिक्त) {
		ctx = hlcache->retrieval_ctx_ring;

		करा {
			next = ctx->r_next;

			जर (ctx->llcache != रिक्त)
				llcache_handle_release(ctx->llcache);

			जर (ctx->handle != रिक्त)
				मुक्त(ctx->handle);

			जर (ctx->child.charset != रिक्त)
				मुक्त((अक्षर *) ctx->child.charset);

			मुक्त(ctx);

			ctx = next;
		} पर्यंत (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = रिक्त;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_मोजणी,
	      hlcache->miss_मोजणी);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, रिक्त);

	मुक्त(hlcache);
	hlcache = रिक्त;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, शून्य *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	प्रतिज्ञा(cb != रिक्त);

	ctx = सुस्मृति(1, आकार(hlcache_retrieval_ctx));
	जर (ctx == रिक्त) {
		परत NSERROR_NOMEM;
	}

	ctx->handle = सुस्मृति(1, आकार(hlcache_handle));
	जर (ctx->handle == रिक्त) {
		मुक्त(ctx);
		परत NSERROR_NOMEM;
	}

	जर (child != रिक्त) {
		जर (child->charset != रिक्त) {
			ctx->child.charset = strdup(child->charset);
			जर (ctx->child.charset == रिक्त) {
				मुक्त(ctx->handle);
				मुक्त(ctx);
				परत NSERROR_NOMEM;
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
	जर (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		मुक्त((अक्षर *) ctx->child.charset);
		मुक्त(ctx->handle);
		मुक्त(ctx);
	} अन्यथा {
		/* successfully started fetch so add new context to list */
		RING_घाला(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	परत error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	जर (handle->entry != रिक्त) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} अन्यथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			जर (ictx->handle == handle &&
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

	handle->cb = रिक्त;
	handle->pw = रिक्त;

	मुक्त(handle);

	परत NSERROR_OK;
}

/* See hlcache.h for documentation */
संरचना content *hlcache_handle_get_content(स्थिरांक hlcache_handle *handle)
{
	जर ((handle != रिक्त) && (handle->entry != रिक्त)) {
		परत handle->entry->content;
	}

	परत रिक्त;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	संरचना hlcache_entry *entry = handle->entry;
	संरचना content *c;

	जर (entry == रिक्त) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			जर (ictx->handle == handle &&
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

		परत NSERROR_OK;
	}

	c = entry->content;

	जर (content_मोजणी_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		संरचना content *clone = content_clone(c);

		जर (clone == रिक्त)
			परत NSERROR_NOMEM;

		entry = सुस्मृति(1, आकार(संरचना hlcache_entry));

		जर (entry == रिक्त) {
			content_destroy(clone);
			परत NSERROR_NOMEM;
		}

		जर (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			मुक्त(entry);
			परत NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = रिक्त;
		entry->next = hlcache->content_list;
		जर (hlcache->content_list != रिक्त)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	परत content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, शून्य *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	परत NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = रिक्त;
	परत NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(स्थिरांक hlcache_handle *handle)
{
	nsurl *result = रिक्त;

	प्रतिज्ञा(handle != रिक्त);

	जर (handle->entry != रिक्त) {
		result = content_get_url(handle->entry->content);
	} अन्यथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			जर (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	परत result;
}
