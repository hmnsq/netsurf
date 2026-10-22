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

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ hlcache_ನಮೂದು hlcache_ನಮೂದು;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ರಚನೆ hlcache_retrieval_ctx {
	ರಚನೆ hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ರಚನೆ hlcache_retrieval_ctx *r_ಮುಂದಿನ; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_ಮಗು_context ಮಗು;	/**< Child context */

	bool migrate_ಗುರಿ;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ರಚನೆ hlcache_handle {
	hlcache_ನಮೂದು *ನಮೂದು;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	ಶೂನ್ಯ *pw;			/**< Client data */
};

/** Entry in high-level cache */
ರಚನೆ hlcache_ನಮೂದು {
	ರಚನೆ content *content;	/**< Pointer to associated content */

	hlcache_ನಮೂದು *ಮುಂದಿನ;		/**< Next sibling */
	hlcache_ನಮೂದು *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ರಚನೆ hlcache_s {
	ರಚನೆ hlcache_parameters params;

	/** List of cached content objects */
	hlcache_ನಮೂದು *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ದೊರಕಿತು_ಎಣಿಕೆ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ತಪ್ಪಿದ_ಎಣಿಕೆ;
};

/** high level cache state */
ಸ್ಥಿರ ರಚನೆ hlcache_s *hlcache = ಖಾಲಿ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ಸ್ಥಿರ ಶೂನ್ಯ hlcache_clean(ಶೂನ್ಯ *force_clean_flag)
{
	hlcache_ನಮೂದು *ನಮೂದು, *ಮುಂದಿನ;
	bool force_clean = (force_clean_flag != ಖಾಲಿ);

	ಗಾಗಿ (ನಮೂದು = hlcache->content_list; ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ಮುಂದಿನ) {
		ಮುಂದಿನ = ನಮೂದು->ಮುಂದಿನ;

		ಆದರೆ (ನಮೂದು->content == ಖಾಲಿ)
			ಮುಂದುವರಿ;

		ಆದರೆ (content_ಎಣಿಕೆ_users(ನಮೂದು->content) != 0)
			ಮುಂದುವರಿ;

		ಆದರೆ (content_get_ಸ್ಥಿತಿ_2(ನಮೂದು->content) == CONTENT_ಸ್ಥಿತಿ_LOADING) {
			ಆದರೆ (force_clean == false)
				ಮುಂದುವರಿ;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(ನಮೂದು->content);
			content_ಗಣ_error(ನಮೂದು->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		ಆದರೆ (ನಮೂದು->prev == ಖಾಲಿ)
			hlcache->content_list = ನಮೂದು->ಮುಂದಿನ;
		ಇಲ್ಲದಿದ್ದರೆ
			ನಮೂದು->prev->ಮುಂದಿನ = ನಮೂದು->ಮುಂದಿನ;

		ಆದರೆ (ನಮೂದು->ಮುಂದಿನ != ಖಾಲಿ)
			ನಮೂದು->ಮುಂದಿನ->prev = ನಮೂದು->prev;

		/* Destroy content */
		content_destroy(ನಮೂದು->content);

		/* Destroy entry */
		ಬಿಡುಗಡೆ(ನಮೂದು);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(hlcache->params.bg_clean_ಸಮಯ, hlcache_clean, ಖಾಲಿ);
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

	ಹಿಂತಿರುಗಿ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ಸ್ಥಿರ ಶೂನ್ಯ hlcache_content_callback(ರಚನೆ content *c, content_ಸಂದೇಶ ಸಂದೇಶ,
		ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ, ಶೂನ್ಯ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = ಸಂದೇಶ,
	};

	ಆದರೆ (ದತ್ತಾಂಶ != ಖಾಲಿ) {
		event.ದತ್ತಾಂಶ = *ದತ್ತಾಂಶ;
	}

	ಆದರೆ (handle->cb != ಖಾಲಿ)
		error = handle->cb(handle, &event, handle->pw);

	ಆದರೆ (error != NSERROR_OK)
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
ಸ್ಥಿರ nserror hlcache_ಕಂಡುಹಿಡಿ_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_ನಮೂದು *ನಮೂದು;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	ಗಾಗಿ (ನಮೂದು = hlcache->content_list; ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		hlcache_handle ನಮೂದು_handle = { ನಮೂದು, ಖಾಲಿ, ಖಾಲಿ };
		ಸ್ಥಿರಾಂಕ llcache_handle *ನಮೂದು_llcache;

		ಆದರೆ (ನಮೂದು->content == ಖಾಲಿ)
			ಮುಂದುವರಿ;

		/* Ignore contents in the error state */
		ಆದರೆ (content_get_ಸ್ಥಿತಿ(&ನಮೂದು_handle) == CONTENT_ಸ್ಥಿತಿ_ERROR)
			ಮುಂದುವರಿ;

		/* Ensure that content is shareable */
		ಆದರೆ (content_is_shareable(ನಮೂದು->content) == false)
			ಮುಂದುವರಿ;

		/* Ensure that quirks mode is acceptable */
		ಆದರೆ (content_matches_quirks(ನಮೂದು->content,
				ctx->ಮಗು.quirks) == false)
			ಮುಂದುವರಿ;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		ನಮೂದು_llcache = content_get_llcache_handle(ನಮೂದು->content);

		ಆದರೆ (llcache_handle_references_same_object(ನಮೂದು_llcache,
				ctx->llcache))
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (ನಮೂದು == ಖಾಲಿ) {
		/* No existing entry, so need to create one */
		ನಮೂದು = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(hlcache_ನಮೂದು));
		ಆದರೆ (ನಮೂದು == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		/* Create content using llhandle */
		ನಮೂದು->content = content_factory_create_content(ctx->llcache,
				ctx->ಮಗು.charset, ctx->ಮಗು.quirks,
				effective_type);
		ಆದರೆ (ನಮೂದು->content == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ನಮೂದು);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		/* Insert into cache */
		ನಮೂದು->prev = ಖಾಲಿ;
		ನಮೂದು->ಮುಂದಿನ = hlcache->content_list;
		ಆದರೆ (hlcache->content_list != ಖಾಲಿ)
			hlcache->content_list->prev = ನಮೂದು;
		hlcache->content_list = ನಮೂದು;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_ದತ್ತಾಂಶ;

		hlcache->ತಪ್ಪಿದ_ಎಣಿಕೆ++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->ದೊರಕಿತು_ಎಣಿಕೆ++;
	}

	/* Associate handle with content */
	ಆದರೆ (content_ಕೂಡಿಸು_user(ನಮೂದು->content,
			hlcache_content_callback, ctx->handle) == false)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->ನಮೂದು = ನಮೂದು;

	/* Catch handle up with state of content */
	ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
		content_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2 = content_get_ಸ್ಥಿತಿ(ctx->handle);

		ಆದರೆ (ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_LOADING) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
				event.type = CONTENT_ಸಂದೇಶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
			event.type = CONTENT_ಸಂದೇಶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
				event.type = CONTENT_ಸಂದೇಶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
				event.type = CONTENT_ಸಂದೇಶ_ಮುಗಿದ;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ಹಿಂತಿರುಗಿ error;
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

	ctx->migrate_ಗುರಿ = true;

	ಆದರೆ ((effective_type != ಖಾಲಿ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_ಕಂಡುಹಿಡಿ_content(ctx, effective_type);
		ಆದರೆ (error != NSERROR_OK && error != NSERROR_NEED_ದತ್ತಾಂಶ) {
			ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
				hlevent.ದತ್ತಾಂಶ.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.ದತ್ತಾಂಶ.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_ಹರಿವು(ctx->llcache);

		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_DOWNLOAD;
			hlevent.ದತ್ತಾಂಶ.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_ದತ್ತಾಂಶ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Unacceptable type: report error */
		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
			hlevent.ದತ್ತಾಂಶ.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.ದತ್ತಾಂಶ.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_ಗುರಿ = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	ಬಿಡುಗಡೆ((ಅಕ್ಷರ *) ctx->ಮಗು.charset);
	ಬಿಡುಗಡೆ(ctx);

	ಹಿಂತಿರುಗಿ error;
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
			 ಸ್ಥಿರಾಂಕ llcache_event *event,
			 ಶೂನ್ಯ *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ಖಾಲಿ;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(ctx->llcache == handle);

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_SSL_CERTS;
			hlevent.ದತ್ತಾಂಶ.chain = event->ದತ್ತಾಂಶ.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_HAD_ಶೀರ್ಷಿಕೆಗಳು:
		error = mimesniff_compute_effective_type(llcache_handle_get_ಶೀರ್ಷಿಕೆ(handle, "Content-Type"), ಖಾಲಿ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ಆದರೆ (error == NSERROR_OK || error == NSERROR_NOT_ಸಿಕ್ಕಿತು) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ಆದರೆ (error == NSERROR_NEED_ದತ್ತಾಂಶ)
			error = NSERROR_OK;

		ಹಿಂತಿರುಗಿ error;

		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_HAD_ದತ್ತಾಂಶ:
		error = mimesniff_compute_effective_type(llcache_handle_get_ಶೀರ್ಷಿಕೆ(handle, "Content-Type"),
				event->ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.buf, event->ದತ್ತಾಂಶ.ದತ್ತಾಂಶ.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ಆದರೆ (error != NSERROR_OK) {
			ಪ್ರತಿಜ್ಞಾ(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ಹಿಂತಿರುಗಿ error;

		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ಮುಗಿದ:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_ಶೀರ್ಷಿಕೆ(handle, "Content-Type"),
				ಖಾಲಿ, 0, false, false, &effective_type);
		ಆದರೆ (error == NSERROR_OK || error == NSERROR_NOT_ಸಿಕ್ಕಿತು) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ಹಿಂತಿರುಗಿ error;
		}

		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
			hlevent.ದತ್ತಾಂಶ.errordata.errorcode = error;
			hlevent.ದತ್ತಾಂಶ.errordata.errormsg = ಖಾಲಿ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ERROR:
		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_ERROR;
			hlevent.ದತ್ತಾಂಶ.errordata.errorcode = event->ದತ್ತಾಂಶ.error.ಸಂಕೇತ_2;
			hlevent.ದತ್ತಾಂಶ.errordata.errormsg = event->ದತ್ತಾಂಶ.error.ಸಂದೇಶ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_ಪ್ರಗತಿ:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ LLCACHE_EVENT_REDIRECT:
		ಆದರೆ (ctx->handle->cb != ಖಾಲಿ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ಸಂದೇಶ_REDIRECT;
			hlevent.ದತ್ತಾಂಶ.redirect.from = event->ದತ್ತಾಂಶ.redirect.from;
			hlevent.ದತ್ತಾಂಶ.redirect.ಗಮ್ಯ = event->ದತ್ತಾಂಶ.redirect.ಗಮ್ಯ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ಸ್ಥಿರಾಂಕ ರಚನೆ hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ hlcache_s));
	ಆದರೆ (hlcache == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ಆದರೆ (ret != NSERROR_OK) {
		ಬಿಡುಗಡೆ(hlcache);
		hlcache = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(hlcache->params.bg_clean_ಸಮಯ, hlcache_clean, ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* See hlcache.h for documentation */
ಶೂನ್ಯ hlcache_stop(ಶೂನ್ಯ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, hlcache_clean, ಖಾಲಿ);
}

/* See hlcache.h for documentation */
ಶೂನ್ಯ hlcache_finalise(ಶೂನ್ಯ)
{
	uint32_t ಅಂಕೆ_contents, prev_contents;
	hlcache_ನಮೂದು *ನಮೂದು;
	hlcache_retrieval_ctx *ctx, *ಮುಂದಿನ;

	/* Obtain initial count of contents remaining */
	ಗಾಗಿ (ಅಂಕೆ_contents = 0, ನಮೂದು = hlcache->content_list;
			ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		ಅಂಕೆ_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      ಅಂಕೆ_contents);

	/* Drain cache */
	ಮಾಡು {
		prev_contents = ಅಂಕೆ_contents;

		hlcache_clean(ಖಾಲಿ);

		ಗಾಗಿ (ಅಂಕೆ_contents = 0, ನಮೂದು = hlcache->content_list;
				ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
			ಅಂಕೆ_contents++;
		}
	} ಆಗ (ಅಂಕೆ_contents > 0 && ಅಂಕೆ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			ಅಂಕೆ_contents);

	/* Drain cache again, forcing the matter */
	ಮಾಡು {
		prev_contents = ಅಂಕೆ_contents;

		hlcache_clean(&ನಮೂದು); // Any non-NULL pointer will do

		ಗಾಗಿ (ಅಂಕೆ_contents = 0, ನಮೂದು = hlcache->content_list;
				ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
			ಅಂಕೆ_contents++;
		}
	} ಆಗ (ಅಂಕೆ_contents > 0 && ಅಂಕೆ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", ಅಂಕೆ_contents);
	ಗಾಗಿ (ನಮೂದು = hlcache->content_list; ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		hlcache_handle ನಮೂದು_handle = { ನಮೂದು, ಖಾಲಿ, ಖಾಲಿ };

		ಆದರೆ (ನಮೂದು->content != ಖಾಲಿ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      ನಮೂದು,
			      nsurl_access(hlcache_handle_get_url(&ನಮೂದು_handle)),
			      content_ಎಣಿಕೆ_users(ನಮೂದು->content));
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, INFO, "	%p", ನಮೂದು);
		}
	}

	/* Clean up retrieval contexts */
	ಆದರೆ (hlcache->retrieval_ctx_ring != ಖಾಲಿ) {
		ctx = hlcache->retrieval_ctx_ring;

		ಮಾಡು {
			ಮುಂದಿನ = ctx->r_ಮುಂದಿನ;

			ಆದರೆ (ctx->llcache != ಖಾಲಿ)
				llcache_handle_release(ctx->llcache);

			ಆದರೆ (ctx->handle != ಖಾಲಿ)
				ಬಿಡುಗಡೆ(ctx->handle);

			ಆದರೆ (ctx->ಮಗು.charset != ಖಾಲಿ)
				ಬಿಡುಗಡೆ((ಅಕ್ಷರ *) ctx->ಮಗು.charset);

			ಬಿಡುಗಡೆ(ctx);

			ctx = ಮುಂದಿನ;
		} ಆಗ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ಖಾಲಿ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->ದೊರಕಿತು_ಎಣಿಕೆ,
	      hlcache->ತಪ್ಪಿದ_ಎಣಿಕೆ);

	/* De-schedule ourselves */
	guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, hlcache_clean, ಖಾಲಿ);

	ಬಿಡುಗಡೆ(hlcache);
	hlcache = ಖಾಲಿ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_ದತ್ತಾಂಶ *post,
			hlcache_handle_callback cb, ಶೂನ್ಯ *pw,
			hlcache_ಮಗು_context *ಮಗು,
			content_type accepted_types,
			hlcache_handle **ಫಲಿತಾಂಶ)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ಪ್ರತಿಜ್ಞಾ(cb != ಖಾಲಿ);

	ctx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(hlcache_retrieval_ctx));
	ಆದರೆ (ctx == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ctx->handle = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(hlcache_handle));
	ಆದರೆ (ctx->handle == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (ಮಗು != ಖಾಲಿ) {
		ಆದರೆ (ಮಗು->charset != ಖಾಲಿ) {
			ctx->ಮಗು.charset = strdup(ಮಗು->charset);
			ಆದರೆ (ctx->ಮಗು.charset == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ctx->handle);
				ಬಿಡುಗಡೆ(ctx);
				ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
			}
		}
		ctx->ಮಗು.quirks = ಮಗು->quirks;
	}

	ctx->flags = flags;
	ctx->accepted_types = accepted_types;

	ctx->handle->cb = cb;
	ctx->handle->pw = pw;

	error = llcache_handle_retrieve(url, flags, referer, post,
			hlcache_llcache_callback, ctx,
			&ctx->llcache);
	ಆದರೆ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		ಬಿಡುಗಡೆ((ಅಕ್ಷರ *) ctx->ಮಗು.charset);
		ಬಿಡುಗಡೆ(ctx->handle);
		ಬಿಡುಗಡೆ(ctx);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* successfully started fetch so add new context to list */
		RING_ಸೇರಿಸು(hlcache->retrieval_ctx_ring, ctx);

		*ಫಲಿತಾಂಶ = ctx->handle;
	}
	ಹಿಂತಿರುಗಿ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ಆದರೆ (handle->ನಮೂದು != ಖಾಲಿ) {
		content_remove_user(handle->ನಮೂದು->content,
				hlcache_content_callback, handle);
	} ಇಲ್ಲದಿದ್ದರೆ {
		RING_ITERATE_ಆರಂಭ(ರಚನೆ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಆದರೆ (ictx->handle == handle &&
					ictx->migrate_ಗುರಿ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ಬಿಡುಗಡೆ((ಅಕ್ಷರ *) ictx->ಮಗು.charset);
				ಬಿಡುಗಡೆ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ಕೊನೆ(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ಖಾಲಿ;
	handle->pw = ಖಾಲಿ;

	ಬಿಡುಗಡೆ(handle);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/* See hlcache.h for documentation */
ರಚನೆ content *hlcache_handle_get_content(ಸ್ಥಿರಾಂಕ hlcache_handle *handle)
{
	ಆದರೆ ((handle != ಖಾಲಿ) && (handle->ನಮೂದು != ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ handle->ನಮೂದು->content;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ರಚನೆ hlcache_ನಮೂದು *ನಮೂದು = handle->ನಮೂದು;
	ರಚನೆ content *c;

	ಆದರೆ (ನಮೂದು == ಖಾಲಿ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_ಆರಂಭ(ರಚನೆ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಆದರೆ (ictx->handle == handle &&
					ictx->migrate_ಗುರಿ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ಬಿಡುಗಡೆ((ಅಕ್ಷರ *) ictx->ಮಗು.charset);
				ಬಿಡುಗಡೆ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ಕೊನೆ(hlcache->retrieval_ctx_ring, ictx);

		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	c = ನಮೂದು->content;

	ಆದರೆ (content_ಎಣಿಕೆ_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ರಚನೆ content *clone = content_clone(c);

		ಆದರೆ (clone == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		ನಮೂದು = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ hlcache_ನಮೂದು));

		ಆದರೆ (ನಮೂದು == ಖಾಲಿ) {
			content_destroy(clone);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		ಆದರೆ (content_ಕೂಡಿಸು_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			ಬಿಡುಗಡೆ(ನಮೂದು);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		ನಮೂದು->content = clone;
		handle->ನಮೂದು = ನಮೂದು;
		ನಮೂದು->prev = ಖಾಲಿ;
		ನಮೂದು->ಮುಂದಿನ = hlcache->content_list;
		ಆದರೆ (hlcache->content_list != ಖಾಲಿ)
			hlcache->content_list->prev = ನಮೂದು;
		hlcache->content_list = ನಮೂದು;

		c = clone;
	}

	ಹಿಂತಿರುಗಿ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, ಶೂನ್ಯ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **ಫಲಿತಾಂಶ)
{
	*ಫಲಿತಾಂಶ = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ಸ್ಥಿರಾಂಕ hlcache_handle *handle)
{
	nsurl *ಫಲಿತಾಂಶ = ಖಾಲಿ;

	ಪ್ರತಿಜ್ಞಾ(handle != ಖಾಲಿ);

	ಆದರೆ (handle->ನಮೂದು != ಖಾಲಿ) {
		ಫಲಿತಾಂಶ = content_get_url(handle->ನಮೂದು->content);
	} ಇಲ್ಲದಿದ್ದರೆ {
		RING_ITERATE_ಆರಂಭ(ರಚನೆ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ಆದರೆ (ictx->handle == handle) {
				/* This is the nascent context for us */
				ಫಲಿತಾಂಶ = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ಕೊನೆ(hlcache->retrieval_ctx_ring, ictx);
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
