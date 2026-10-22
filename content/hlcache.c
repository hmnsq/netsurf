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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/ring.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "content/mimesniff.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ hlcache_entry hlcache_entry;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ಸಂರಚನಾ hlcache_retrieval_ctx {
	ಸಂರಚನಾ hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ಸಂರಚನಾ hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ಸಂರಚನಾ hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	ಶೂನ್ಯ *pw;			/**< Client data */
};

/** Entry in high-level cache */
ಸಂರಚನಾ hlcache_entry {
	ಸಂರಚನಾ content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ಸಂರಚನಾ hlcache_s {
	ಸಂರಚನಾ hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned ಪೂರ್ಣಾಙ್ಕ hit_ಎಣಿಕೆ;
	unsigned ಪೂರ್ಣಾಙ್ಕ miss_ಎಣಿಕೆ;
};

/** high level cache state */
ಸ್ಥಿರ ಸಂರಚನಾ hlcache_s *hlcache = ರಿಕ್ತ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ಸ್ಥಿರ ಶೂನ್ಯ hlcache_clean(ಶೂನ್ಯ *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != ರಿಕ್ತ);

	ಪ್ರತಿ (entry = hlcache->content_list; entry != ರಿಕ್ತ; entry = next) {
		next = entry->next;

		ಯದಿ (entry->content == ರಿಕ್ತ)
			ಅನುವರ್ತ;

		ಯದಿ (content_ಎಣಿಕೆ_users(entry->content) != 0)
			ಅನುವರ್ತ;

		ಯದಿ (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			ಯದಿ (force_clean == false)
				ಅನುವರ್ತ;
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
		ಯದಿ (entry->prev == ರಿಕ್ತ)
			hlcache->content_list = entry->next;
		ಅನ್ಯಥಾ
			entry->prev->next = entry->next;

		ಯದಿ (entry->next != ರಿಕ್ತ)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		ಮುಕ್ತ(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ರಿಕ್ತ);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ಸ್ಥಿರ bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	ಪ್ರತ್ಯಾಗಮ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ಸ್ಥಿರ ಶೂನ್ಯ hlcache_content_callback(ಸಂರಚನಾ content *c, content_ಸಂದೇಶ ಸಂದೇಶ,
		ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data, ಶೂನ್ಯ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = ಸಂದೇಶ,
	};

	ಯದಿ (data != ರಿಕ್ತ) {
		event.data = *data;
	}

	ಯದಿ (handle->cb != ರಿಕ್ತ)
		error = handle->cb(handle, &event, handle->pw);

	ಯದಿ (error != NSERROR_OK)
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
ಸ್ಥಿರ nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	ಪ್ರತಿ (entry = hlcache->content_list; entry != ರಿಕ್ತ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ರಿಕ್ತ, ರಿಕ್ತ };
		ಧ್ರುವ llcache_handle *entry_llcache;

		ಯದಿ (entry->content == ರಿಕ್ತ)
			ಅನುವರ್ತ;

		/* Ignore contents in the error state */
		ಯದಿ (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			ಅನುವರ್ತ;

		/* Ensure that content is shareable */
		ಯದಿ (content_is_shareable(entry->content) == false)
			ಅನುವರ್ತ;

		/* Ensure that quirks mode is acceptable */
		ಯದಿ (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			ಅನುವರ್ತ;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		ಯದಿ (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			ಭಙ್ಗ;
	}

	ಯದಿ (entry == ರಿಕ್ತ) {
		/* No existing entry, so need to create one */
		entry = malloc(ಪರಿಮಾಣ(hlcache_entry));
		ಯದಿ (entry == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		ಯದಿ (entry->content == ರಿಕ್ತ) {
			ಮುಕ್ತ(entry);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = ರಿಕ್ತ;
		entry->next = hlcache->content_list;
		ಯದಿ (hlcache->content_list != ರಿಕ್ತ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_ಎಣಿಕೆ++;
	} ಅನ್ಯಥಾ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_ಎಣಿಕೆ++;
	}

	/* Associate handle with content */
	ಯದಿ (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
		content_status status = content_get_status(ctx->handle);

		ಯದಿ (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ಅನ್ಯಥಾ ಯದಿ (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
				event.type = CONTENT_ಸಂದೇಶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ಅನ್ಯಥಾ ಯದಿ (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
				event.type = CONTENT_ಸಂದೇಶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
				event.type = CONTENT_ಸಂದೇಶ_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ error;
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
ಸ್ಥಿರ nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	ಯದಿ ((effective_type != ರಿಕ್ತ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		ಯದಿ (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ಅನ್ಯಥಾ ಯದಿ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} ಅನ್ಯಥಾ {
		/* Unacceptable type: report error */
		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
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
	ಮುಕ್ತ((ವರ್ಣ *) ctx->child.charset);
	ಮುಕ್ತ(ctx);

	ಪ್ರತ್ಯಾಗಮ error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ಧ್ರುವ llcache_event *event,
			 ಶೂನ್ಯ *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ರಿಕ್ತ;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(ctx->llcache == handle);

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), ರಿಕ್ತ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ಯದಿ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ಯದಿ (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		ಪ್ರತ್ಯಾಗಮ error;

		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ಯದಿ (error != NSERROR_OK) {
			ಪ್ರತಿಜ್ಞಾ(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ಪ್ರತ್ಯಾಗಮ error;

		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				ರಿಕ್ತ, 0, false, false, &effective_type);
		ಯದಿ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ಪ್ರತ್ಯಾಗಮ error;
		}

		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = ರಿಕ್ತ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_ERROR:
		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.ಸಂದೇಶ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_PROGRESS:
		ಭಙ್ಗ;
	ಪಕ್ಷ LLCACHE_EVENT_REDIRECT:
		ಯದಿ (ctx->handle->cb != ರಿಕ್ತ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ಧ್ರುವ ಸಂರಚನಾ hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ hlcache_s));
	ಯದಿ (hlcache == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ಯದಿ (ret != NSERROR_OK) {
		ಮುಕ್ತ(hlcache);
		hlcache = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* See hlcache.h for documentation */
ಶೂನ್ಯ hlcache_stop(ಶೂನ್ಯ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, ರಿಕ್ತ);
}

/* See hlcache.h for documentation */
ಶೂನ್ಯ hlcache_finalise(ಶೂನ್ಯ)
{
	uint32_t ಅಂಕೆ_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	ಪ್ರತಿ (ಅಂಕೆ_contents = 0, entry = hlcache->content_list;
			entry != ರಿಕ್ತ; entry = entry->next) {
		ಅಂಕೆ_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      ಅಂಕೆ_contents);

	/* Drain cache */
	ಕುರು {
		prev_contents = ಅಂಕೆ_contents;

		hlcache_clean(ರಿಕ್ತ);

		ಪ್ರತಿ (ಅಂಕೆ_contents = 0, entry = hlcache->content_list;
				entry != ರಿಕ್ತ; entry = entry->next) {
			ಅಂಕೆ_contents++;
		}
	} ಯಾವತ್ (ಅಂಕೆ_contents > 0 && ಅಂಕೆ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			ಅಂಕೆ_contents);

	/* Drain cache again, forcing the matter */
	ಕುರು {
		prev_contents = ಅಂಕೆ_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		ಪ್ರತಿ (ಅಂಕೆ_contents = 0, entry = hlcache->content_list;
				entry != ರಿಕ್ತ; entry = entry->next) {
			ಅಂಕೆ_contents++;
		}
	} ಯಾವತ್ (ಅಂಕೆ_contents > 0 && ಅಂಕೆ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", ಅಂಕೆ_contents);
	ಪ್ರತಿ (entry = hlcache->content_list; entry != ರಿಕ್ತ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ರಿಕ್ತ, ರಿಕ್ತ };

		ಯದಿ (entry->content != ರಿಕ್ತ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_ಎಣಿಕೆ_users(entry->content));
		} ಅನ್ಯಥಾ {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	ಯದಿ (hlcache->retrieval_ctx_ring != ರಿಕ್ತ) {
		ctx = hlcache->retrieval_ctx_ring;

		ಕುರು {
			next = ctx->r_next;

			ಯದಿ (ctx->llcache != ರಿಕ್ತ)
				llcache_handle_release(ctx->llcache);

			ಯದಿ (ctx->handle != ರಿಕ್ತ)
				ಮುಕ್ತ(ctx->handle);

			ಯದಿ (ctx->child.charset != ರಿಕ್ತ)
				ಮುಕ್ತ((ವರ್ಣ *) ctx->child.charset);

			ಮುಕ್ತ(ctx);

			ctx = next;
		} ಯಾವತ್ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ರಿಕ್ತ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_ಎಣಿಕೆ,
	      hlcache->miss_ಎಣಿಕೆ);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, ರಿಕ್ತ);

	ಮುಕ್ತ(hlcache);
	hlcache = ರಿಕ್ತ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, ಶೂನ್ಯ *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(cb != ರಿಕ್ತ);

	ctx = calloc(1, ಪರಿಮಾಣ(hlcache_retrieval_ctx));
	ಯದಿ (ctx == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, ಪರಿಮಾಣ(hlcache_handle));
	ಯದಿ (ctx->handle == ರಿಕ್ತ) {
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (child != ರಿಕ್ತ) {
		ಯದಿ (child->charset != ರಿಕ್ತ) {
			ctx->child.charset = strdup(child->charset);
			ಯದಿ (ctx->child.charset == ರಿಕ್ತ) {
				ಮುಕ್ತ(ctx->handle);
				ಮುಕ್ತ(ctx);
				ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
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
	ಯದಿ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		ಮುಕ್ತ((ವರ್ಣ *) ctx->child.charset);
		ಮುಕ್ತ(ctx->handle);
		ಮುಕ್ತ(ctx);
	} ಅನ್ಯಥಾ {
		/* successfully started fetch so add new context to list */
		RING_ಸೇರಿಸು(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	ಪ್ರತ್ಯಾಗಮ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ಯದಿ (handle->entry != ರಿಕ್ತ) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} ಅನ್ಯಥಾ {
		RING_ITERATE_START(ಸಂರಚನಾ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಯದಿ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ಮುಕ್ತ((ವರ್ಣ *) ictx->child.charset);
				ಮುಕ್ತ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ರಿಕ್ತ;
	handle->pw = ರಿಕ್ತ;

	ಮುಕ್ತ(handle);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/* See hlcache.h for documentation */
ಸಂರಚನಾ content *hlcache_handle_get_content(ಧ್ರುವ hlcache_handle *handle)
{
	ಯದಿ ((handle != ರಿಕ್ತ) && (handle->entry != ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ handle->entry->content;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ಸಂರಚನಾ hlcache_entry *entry = handle->entry;
	ಸಂರಚನಾ content *c;

	ಯದಿ (entry == ರಿಕ್ತ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(ಸಂರಚನಾ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಯದಿ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ಮುಕ್ತ((ವರ್ಣ *) ictx->child.charset);
				ಮುಕ್ತ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	c = entry->content;

	ಯದಿ (content_ಎಣಿಕೆ_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ಸಂರಚನಾ content *clone = content_clone(c);

		ಯದಿ (clone == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

		entry = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ hlcache_entry));

		ಯದಿ (entry == ರಿಕ್ತ) {
			content_destroy(clone);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		ಯದಿ (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			ಮುಕ್ತ(entry);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = ರಿಕ್ತ;
		entry->next = hlcache->content_list;
		ಯದಿ (hlcache->content_list != ರಿಕ್ತ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	ಪ್ರತ್ಯಾಗಮ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, ಶೂನ್ಯ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ಧ್ರುವ hlcache_handle *handle)
{
	nsurl *result = ರಿಕ್ತ;

	ಪ್ರತಿಜ್ಞಾ(handle != ರಿಕ್ತ);

	ಯದಿ (handle->entry != ರಿಕ್ತ) {
		result = content_get_url(handle->entry->content);
	} ಅನ್ಯಥಾ {
		RING_ITERATE_START(ಸಂರಚನಾ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಯದಿ (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	ಪ್ರತ್ಯಾಗಮ result;
}
