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

प्रकारनिर्देश संरचना hlcache_entry hlcache_entry;
प्रकारनिर्देश संरचना hlcache_retrieval_ctx hlcache_retrieval_ctx;

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
	unsigned पूर्णाङ्क hit_गणनम्;
	unsigned पूर्णाङ्क miss_गणनम्;
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

	अत्थाय (entry = hlcache->content_list; entry != रिक्त; entry = next) {
		next = entry->next;

		सचे (entry->content == रिक्त)
			अनुवर्त;

		सचे (content_गणनम्_users(entry->content) != 0)
			अनुवर्त;

		सचे (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			सचे (force_clean == false)
				अनुवर्त;
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
		सचे (entry->prev == रिक्त)
			hlcache->content_list = entry->next;
		अञ्ञथा
			entry->prev->next = entry->next;

		सचे (entry->next != रिक्त)
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

	पटिनिवत्त ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
स्थिर शून्य hlcache_content_callback(संरचना content *c, content_सन्देशः सन्देशः,
		ध्रुव सङ्घ content_सन्देशः_data *data, शून्य *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = सन्देशः,
	};

	सचे (data != रिक्त) {
		event.data = *data;
	}

	सचे (handle->cb != रिक्त)
		error = handle->cb(handle, &event, handle->pw);

	सचे (error != NSERROR_OK)
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
	अत्थाय (entry = hlcache->content_list; entry != रिक्त; entry = entry->next) {
		hlcache_handle entry_handle = { entry, रिक्त, रिक्त };
		ध्रुव llcache_handle *entry_llcache;

		सचे (entry->content == रिक्त)
			अनुवर्त;

		/* Ignore contents in the error state */
		सचे (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			अनुवर्त;

		/* Ensure that content is shareable */
		सचे (content_is_shareable(entry->content) == false)
			अनुवर्त;

		/* Ensure that quirks mode is acceptable */
		सचे (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			अनुवर्त;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		सचे (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			भङ्ग;
	}

	सचे (entry == रिक्त) {
		/* No existing entry, so need to create one */
		entry = malloc(परिमाण(hlcache_entry));
		सचे (entry == रिक्त)
			पटिनिवत्त NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		सचे (entry->content == रिक्त) {
			मुक्त(entry);
			पटिनिवत्त NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = रिक्त;
		entry->next = hlcache->content_list;
		सचे (hlcache->content_list != रिक्त)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_गणनम्++;
	} अञ्ञथा {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_गणनम्++;
	}

	/* Associate handle with content */
	सचे (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		पटिनिवत्त NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	सचे (ctx->handle->cb != रिक्त) {
		content_status status = content_get_status(ctx->handle);

		सचे (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} अञ्ञथा सचे (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			सचे (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} अञ्ञथा सचे (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			सचे (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			सचे (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	पटिनिवत्त error;
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

	सचे ((effective_type != रिक्त) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		सचे (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			सचे (ctx->handle->cb != रिक्त) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_सन्देशः_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} अञ्ञथा सचे (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} अञ्ञथा {
		/* Unacceptable type: report error */
		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
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
	मुक्त((वर्ण *) ctx->child.charset);
	मुक्त(ctx);

	पटिनिवत्त error;
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
			 ध्रुव llcache_event *event,
			 शून्य *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = रिक्त;
	nserror error;

	प्रतिज्ञा(ctx->llcache == handle);

	विकल्प (event->type) {
	पक्ष LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), रिक्त, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		सचे (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		सचे (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		पटिनिवत्त error;

		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		सचे (error != NSERROR_OK) {
			प्रतिज्ञा(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		पटिनिवत्त error;

		भङ्ग;
	पक्ष LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				रिक्त, 0, false, false, &effective_type);
		सचे (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			पटिनिवत्त error;
		}

		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = रिक्त;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_ERROR:
		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.सन्देशः;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_PROGRESS:
		भङ्ग;
	पक्ष LLCACHE_EVENT_REDIRECT:
		सचे (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	}

	पटिनिवत्त NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ध्रुव संरचना hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, परिमाण(संरचना hlcache_s));
	सचे (hlcache == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	सचे (ret != NSERROR_OK) {
		मुक्त(hlcache);
		hlcache = रिक्त;
		पटिनिवत्त ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, रिक्त);

	पटिनिवत्त NSERROR_OK;
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
	uint32_t अङ्कः_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	अत्थाय (अङ्कः_contents = 0, entry = hlcache->content_list;
			entry != रिक्त; entry = entry->next) {
		अङ्कः_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      अङ्कः_contents);

	/* Drain cache */
	करोहि {
		prev_contents = अङ्कः_contents;

		hlcache_clean(रिक्त);

		अत्थाय (अङ्कः_contents = 0, entry = hlcache->content_list;
				entry != रिक्त; entry = entry->next) {
			अङ्कः_contents++;
		}
	} याव (अङ्कः_contents > 0 && अङ्कः_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			अङ्कः_contents);

	/* Drain cache again, forcing the matter */
	करोहि {
		prev_contents = अङ्कः_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		अत्थाय (अङ्कः_contents = 0, entry = hlcache->content_list;
				entry != रिक्त; entry = entry->next) {
			अङ्कः_contents++;
		}
	} याव (अङ्कः_contents > 0 && अङ्कः_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", अङ्कः_contents);
	अत्थाय (entry = hlcache->content_list; entry != रिक्त; entry = entry->next) {
		hlcache_handle entry_handle = { entry, रिक्त, रिक्त };

		सचे (entry->content != रिक्त) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_गणनम्_users(entry->content));
		} अञ्ञथा {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	सचे (hlcache->retrieval_ctx_ring != रिक्त) {
		ctx = hlcache->retrieval_ctx_ring;

		करोहि {
			next = ctx->r_next;

			सचे (ctx->llcache != रिक्त)
				llcache_handle_release(ctx->llcache);

			सचे (ctx->handle != रिक्त)
				मुक्त(ctx->handle);

			सचे (ctx->child.charset != रिक्त)
				मुक्त((वर्ण *) ctx->child.charset);

			मुक्त(ctx);

			ctx = next;
		} याव (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = रिक्त;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_गणनम्,
	      hlcache->miss_गणनम्);

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

	ctx = calloc(1, परिमाण(hlcache_retrieval_ctx));
	सचे (ctx == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, परिमाण(hlcache_handle));
	सचे (ctx->handle == रिक्त) {
		मुक्त(ctx);
		पटिनिवत्त NSERROR_NOMEM;
	}

	सचे (child != रिक्त) {
		सचे (child->charset != रिक्त) {
			ctx->child.charset = strdup(child->charset);
			सचे (ctx->child.charset == रिक्त) {
				मुक्त(ctx->handle);
				मुक्त(ctx);
				पटिनिवत्त NSERROR_NOMEM;
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
	सचे (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		मुक्त((वर्ण *) ctx->child.charset);
		मुक्त(ctx->handle);
		मुक्त(ctx);
	} अञ्ञथा {
		/* successfully started fetch so add new context to list */
		RING_निवेशय(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	पटिनिवत्त error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	सचे (handle->entry != रिक्त) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} अञ्ञथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			सचे (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((वर्ण *) ictx->child.charset);
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

	पटिनिवत्त NSERROR_OK;
}

/* See hlcache.h for documentation */
संरचना content *hlcache_handle_get_content(ध्रुव hlcache_handle *handle)
{
	सचे ((handle != रिक्त) && (handle->entry != रिक्त)) {
		पटिनिवत्त handle->entry->content;
	}

	पटिनिवत्त रिक्त;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	संरचना hlcache_entry *entry = handle->entry;
	संरचना content *c;

	सचे (entry == रिक्त) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			सचे (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((वर्ण *) ictx->child.charset);
				मुक्त(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		पटिनिवत्त NSERROR_OK;
	}

	c = entry->content;

	सचे (content_गणनम्_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		संरचना content *clone = content_clone(c);

		सचे (clone == रिक्त)
			पटिनिवत्त NSERROR_NOMEM;

		entry = calloc(1, परिमाण(संरचना hlcache_entry));

		सचे (entry == रिक्त) {
			content_destroy(clone);
			पटिनिवत्त NSERROR_NOMEM;
		}

		सचे (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			मुक्त(entry);
			पटिनिवत्त NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = रिक्त;
		entry->next = hlcache->content_list;
		सचे (hlcache->content_list != रिक्त)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	पटिनिवत्त content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, शून्य *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	पटिनिवत्त NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = रिक्त;
	पटिनिवत्त NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ध्रुव hlcache_handle *handle)
{
	nsurl *result = रिक्त;

	प्रतिज्ञा(handle != रिक्त);

	सचे (handle->entry != रिक्त) {
		result = content_get_url(handle->entry->content);
	} अञ्ञथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			सचे (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	पटिनिवत्त result;
}
