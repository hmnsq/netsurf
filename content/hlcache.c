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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/ring.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "content/mimesniff.h"
#அந்தர்பாவ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"

வகைவரை கட்டமைப்பு hlcache_entry hlcache_entry;
வகைவரை கட்டமைப்பு hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
கட்டமைப்பு hlcache_retrieval_ctx {
	கட்டமைப்பு hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	கட்டமைப்பு hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
கட்டமைப்பு hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	வெறுமை *pw;			/**< Client data */
};

/** Entry in high-level cache */
கட்டமைப்பு hlcache_entry {
	கட்டமைப்பு content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
கட்டமைப்பு hlcache_s {
	கட்டமைப்பு hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned முழு hit_மொத்தம்;
	unsigned முழு miss_மொத்தம்;
};

/** high level cache state */
நிலையான கட்டமைப்பு hlcache_s *hlcache = வெற்று;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
நிலையான வெறுமை hlcache_clean(வெறுமை *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != வெற்று);

	க்கு (entry = hlcache->content_list; entry != வெற்று; entry = next) {
		next = entry->next;

		எனில் (entry->content == வெற்று)
			தொடர்;

		எனில் (content_மொத்தம்_users(entry->content) != 0)
			தொடர்;

		எனில் (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			எனில் (force_clean == false)
				தொடர்;
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
		எனில் (entry->prev == வெற்று)
			hlcache->content_list = entry->next;
		இல்லையெனில்
			entry->prev->next = entry->next;

		எனில் (entry->next != வெற்று)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		விடுவி(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, வெற்று);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
நிலையான bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	திரும்பு ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
நிலையான வெறுமை hlcache_content_callback(கட்டமைப்பு content *c, content_செய்தி செய்தி,
		மாறிலி ஒன்றிப்பு content_செய்தி_data *data, வெறுமை *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = செய்தி,
	};

	எனில் (data != வெற்று) {
		event.data = *data;
	}

	எனில் (handle->cb != வெற்று)
		error = handle->cb(handle, &event, handle->pw);

	எனில் (error != NSERROR_OK)
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
நிலையான nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	க்கு (entry = hlcache->content_list; entry != வெற்று; entry = entry->next) {
		hlcache_handle entry_handle = { entry, வெற்று, வெற்று };
		மாறிலி llcache_handle *entry_llcache;

		எனில் (entry->content == வெற்று)
			தொடர்;

		/* Ignore contents in the error state */
		எனில் (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			தொடர்;

		/* Ensure that content is shareable */
		எனில் (content_is_shareable(entry->content) == false)
			தொடர்;

		/* Ensure that quirks mode is acceptable */
		எனில் (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			தொடர்;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		எனில் (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			நிறுத்து;
	}

	எனில் (entry == வெற்று) {
		/* No existing entry, so need to create one */
		entry = malloc(அளவு(hlcache_entry));
		எனில் (entry == வெற்று)
			திரும்பு NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		எனில் (entry->content == வெற்று) {
			விடுவி(entry);
			திரும்பு NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = வெற்று;
		entry->next = hlcache->content_list;
		எனில் (hlcache->content_list != வெற்று)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_மொத்தம்++;
	} இல்லையெனில் {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_மொத்தம்++;
	}

	/* Associate handle with content */
	எனில் (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		திரும்பு NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	எனில் (ctx->handle->cb != வெற்று) {
		content_status status = content_get_status(ctx->handle);

		எனில் (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_செய்தி_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} இல்லையெனில் எனில் (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_செய்தி_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			எனில் (ctx->handle->cb != வெற்று) {
				event.type = CONTENT_செய்தி_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} இல்லையெனில் எனில் (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_செய்தி_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			எனில் (ctx->handle->cb != வெற்று) {
				event.type = CONTENT_செய்தி_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			எனில் (ctx->handle->cb != வெற்று) {
				event.type = CONTENT_செய்தி_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	திரும்பு error;
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
நிலையான nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	எனில் ((effective_type != வெற்று) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		எனில் (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			எனில் (ctx->handle->cb != வெற்று) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_செய்தி_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} இல்லையெனில் எனில் (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} இல்லையெனில் {
		/* Unacceptable type: report error */
		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_ERROR;
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
	விடுவி((எழுத்து *) ctx->child.charset);
	விடுவி(ctx);

	திரும்பு error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
நிலையான nserror
hlcache_llcache_callback(llcache_handle *handle,
			 மாறிலி llcache_event *event,
			 வெறுமை *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = வெற்று;
	nserror error;

	ப்ரதிஜ்ஞா(ctx->llcache == handle);

	தேர்வு (event->type) {
	நிலை LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		நிறுத்து;
	நிலை LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), வெற்று, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		எனில் (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		எனில் (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		திரும்பு error;

		நிறுத்து;
	நிலை LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		எனில் (error != NSERROR_OK) {
			ப்ரதிஜ்ஞா(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		திரும்பு error;

		நிறுத்து;
	நிலை LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				வெற்று, 0, false, false, &effective_type);
		எனில் (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			திரும்பு error;
		}

		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = வெற்று;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		நிறுத்து;
	நிலை LLCACHE_EVENT_ERROR:
		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.செய்தி;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		நிறுத்து;
	நிலை LLCACHE_EVENT_PROGRESS:
		நிறுத்து;
	நிலை LLCACHE_EVENT_REDIRECT:
		எனில் (ctx->handle->cb != வெற்று) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_செய்தி_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		நிறுத்து;
	}

	திரும்பு NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(மாறிலி கட்டமைப்பு hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, அளவு(கட்டமைப்பு hlcache_s));
	எனில் (hlcache == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	எனில் (ret != NSERROR_OK) {
		விடுவி(hlcache);
		hlcache = வெற்று;
		திரும்பு ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, வெற்று);

	திரும்பு NSERROR_OK;
}

/* See hlcache.h for documentation */
வெறுமை hlcache_stop(வெறுமை)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, வெற்று);
}

/* See hlcache.h for documentation */
வெறுமை hlcache_finalise(வெறுமை)
{
	uint32_t இலக்கம்_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	க்கு (இலக்கம்_contents = 0, entry = hlcache->content_list;
			entry != வெற்று; entry = entry->next) {
		இலக்கம்_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      இலக்கம்_contents);

	/* Drain cache */
	செய் {
		prev_contents = இலக்கம்_contents;

		hlcache_clean(வெற்று);

		க்கு (இலக்கம்_contents = 0, entry = hlcache->content_list;
				entry != வெற்று; entry = entry->next) {
			இலக்கம்_contents++;
		}
	} போது (இலக்கம்_contents > 0 && இலக்கம்_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			இலக்கம்_contents);

	/* Drain cache again, forcing the matter */
	செய் {
		prev_contents = இலக்கம்_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		க்கு (இலக்கம்_contents = 0, entry = hlcache->content_list;
				entry != வெற்று; entry = entry->next) {
			இலக்கம்_contents++;
		}
	} போது (இலக்கம்_contents > 0 && இலக்கம்_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", இலக்கம்_contents);
	க்கு (entry = hlcache->content_list; entry != வெற்று; entry = entry->next) {
		hlcache_handle entry_handle = { entry, வெற்று, வெற்று };

		எனில் (entry->content != வெற்று) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_மொத்தம்_users(entry->content));
		} இல்லையெனில் {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	எனில் (hlcache->retrieval_ctx_ring != வெற்று) {
		ctx = hlcache->retrieval_ctx_ring;

		செய் {
			next = ctx->r_next;

			எனில் (ctx->llcache != வெற்று)
				llcache_handle_release(ctx->llcache);

			எனில் (ctx->handle != வெற்று)
				விடுவி(ctx->handle);

			எனில் (ctx->child.charset != வெற்று)
				விடுவி((எழுத்து *) ctx->child.charset);

			விடுவி(ctx);

			ctx = next;
		} போது (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = வெற்று;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_மொத்தம்,
	      hlcache->miss_மொத்தம்);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, வெற்று);

	விடுவி(hlcache);
	hlcache = வெற்று;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, வெறுமை *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ப்ரதிஜ்ஞா(cb != வெற்று);

	ctx = calloc(1, அளவு(hlcache_retrieval_ctx));
	எனில் (ctx == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, அளவு(hlcache_handle));
	எனில் (ctx->handle == வெற்று) {
		விடுவி(ctx);
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (child != வெற்று) {
		எனில் (child->charset != வெற்று) {
			ctx->child.charset = strdup(child->charset);
			எனில் (ctx->child.charset == வெற்று) {
				விடுவி(ctx->handle);
				விடுவி(ctx);
				திரும்பு NSERROR_NOMEM;
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
	எனில் (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		விடுவி((எழுத்து *) ctx->child.charset);
		விடுவி(ctx->handle);
		விடுவி(ctx);
	} இல்லையெனில் {
		/* successfully started fetch so add new context to list */
		RING_செருகு(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	திரும்பு error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	எனில் (handle->entry != வெற்று) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} இல்லையெனில் {
		RING_ITERATE_START(கட்டமைப்பு hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			எனில் (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				விடுவி((எழுத்து *) ictx->child.charset);
				விடுவி(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = வெற்று;
	handle->pw = வெற்று;

	விடுவி(handle);

	திரும்பு NSERROR_OK;
}

/* See hlcache.h for documentation */
கட்டமைப்பு content *hlcache_handle_get_content(மாறிலி hlcache_handle *handle)
{
	எனில் ((handle != வெற்று) && (handle->entry != வெற்று)) {
		திரும்பு handle->entry->content;
	}

	திரும்பு வெற்று;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	கட்டமைப்பு hlcache_entry *entry = handle->entry;
	கட்டமைப்பு content *c;

	எனில் (entry == வெற்று) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(கட்டமைப்பு hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			எனில் (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				விடுவி((எழுத்து *) ictx->child.charset);
				விடுவி(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		திரும்பு NSERROR_OK;
	}

	c = entry->content;

	எனில் (content_மொத்தம்_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		கட்டமைப்பு content *clone = content_clone(c);

		எனில் (clone == வெற்று)
			திரும்பு NSERROR_NOMEM;

		entry = calloc(1, அளவு(கட்டமைப்பு hlcache_entry));

		எனில் (entry == வெற்று) {
			content_destroy(clone);
			திரும்பு NSERROR_NOMEM;
		}

		எனில் (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			விடுவி(entry);
			திரும்பு NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = வெற்று;
		entry->next = hlcache->content_list;
		எனில் (hlcache->content_list != வெற்று)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	திரும்பு content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, வெறுமை *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	திரும்பு NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = வெற்று;
	திரும்பு NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(மாறிலி hlcache_handle *handle)
{
	nsurl *result = வெற்று;

	ப்ரதிஜ்ஞா(handle != வெற்று);

	எனில் (handle->entry != வெற்று) {
		result = content_get_url(handle->entry->content);
	} இல்லையெனில் {
		RING_ITERATE_START(கட்டமைப்பு hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			எனில் (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	திரும்பு result;
}
