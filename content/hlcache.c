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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/mimesniff.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ hlcache_ਇੰਦਰਾਜ hlcache_ਇੰਦਰਾਜ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ਬਣਤਰ hlcache_retrieval_ctx {
	ਬਣਤਰ hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ਬਣਤਰ hlcache_retrieval_ctx *r_ਅਗਲਾ; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_ਬੱਚਾ_context ਬੱਚਾ;	/**< Child context */

	bool migrate_ਨਿਸ਼ਾਨਾ;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ਬਣਤਰ hlcache_handle {
	hlcache_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	ਖਾਲੀ *pw;			/**< Client data */
};

/** Entry in high-level cache */
ਬਣਤਰ hlcache_ਇੰਦਰਾਜ {
	ਬਣਤਰ content *content;	/**< Pointer to associated content */

	hlcache_ਇੰਦਰਾਜ *ਅਗਲਾ;		/**< Next sibling */
	hlcache_ਇੰਦਰਾਜ *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ਬਣਤਰ hlcache_s {
	ਬਣਤਰ hlcache_parameters params;

	/** List of cached content objects */
	hlcache_ਇੰਦਰਾਜ *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਖੁੰਝ_ਗਿਣਤੀ;
};

/** high level cache state */
ਸਥਿਰ ਬਣਤਰ hlcache_s *hlcache = ਨਿਰਾ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ਸਥਿਰ ਖਾਲੀ hlcache_clean(ਖਾਲੀ *force_clean_flag)
{
	hlcache_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ, *ਅਗਲਾ;
	bool force_clean = (force_clean_flag != ਨਿਰਾ);

	ਲਈ (ਇੰਦਰਾਜ = hlcache->content_list; ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਅਗਲਾ) {
		ਅਗਲਾ = ਇੰਦਰਾਜ->ਅਗਲਾ;

		ਜੇ (ਇੰਦਰਾਜ->content == ਨਿਰਾ)
			ਜਾਰੀ;

		ਜੇ (content_ਗਿਣਤੀ_users(ਇੰਦਰਾਜ->content) != 0)
			ਜਾਰੀ;

		ਜੇ (content_get_ਸਥਿਤੀ_2(ਇੰਦਰਾਜ->content) == CONTENT_ਸਥਿਤੀ_LOADING) {
			ਜੇ (force_clean == false)
				ਜਾਰੀ;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(ਇੰਦਰਾਜ->content);
			content_ਸੈੱਟ_error(ਇੰਦਰਾਜ->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		ਜੇ (ਇੰਦਰਾਜ->prev == ਨਿਰਾ)
			hlcache->content_list = ਇੰਦਰਾਜ->ਅਗਲਾ;
		ਨਹੀਂਤਾਂ
			ਇੰਦਰਾਜ->prev->ਅਗਲਾ = ਇੰਦਰਾਜ->ਅਗਲਾ;

		ਜੇ (ਇੰਦਰਾਜ->ਅਗਲਾ != ਨਿਰਾ)
			ਇੰਦਰਾਜ->ਅਗਲਾ->prev = ਇੰਦਰਾਜ->prev;

		/* Destroy content */
		content_destroy(ਇੰਦਰਾਜ->content);

		/* Destroy entry */
		ਮੁਕਤ(ਇੰਦਰਾਜ);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(hlcache->params.bg_clean_ਸਮਾਂ, hlcache_clean, ਨਿਰਾ);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ਸਥਿਰ bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	ਵਾਪਸ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ਸਥਿਰ ਖਾਲੀ hlcache_content_callback(ਬਣਤਰ content *c, content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
		ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ, ਖਾਲੀ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = ਸੁਨੇਹਾ,
	};

	ਜੇ (ਅੰਕੜੇ != ਨਿਰਾ) {
		event.ਅੰਕੜੇ = *ਅੰਕੜੇ;
	}

	ਜੇ (handle->cb != ਨਿਰਾ)
		error = handle->cb(handle, &event, handle->pw);

	ਜੇ (error != NSERROR_OK)
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
ਸਥਿਰ nserror hlcache_ਲੱਭੋ_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	ਲਈ (ਇੰਦਰਾਜ = hlcache->content_list; ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		hlcache_handle ਇੰਦਰਾਜ_handle = { ਇੰਦਰਾਜ, ਨਿਰਾ, ਨਿਰਾ };
		ਅਚਲ llcache_handle *ਇੰਦਰਾਜ_llcache;

		ਜੇ (ਇੰਦਰਾਜ->content == ਨਿਰਾ)
			ਜਾਰੀ;

		/* Ignore contents in the error state */
		ਜੇ (content_get_ਸਥਿਤੀ(&ਇੰਦਰਾਜ_handle) == CONTENT_ਸਥਿਤੀ_ERROR)
			ਜਾਰੀ;

		/* Ensure that content is shareable */
		ਜੇ (content_is_shareable(ਇੰਦਰਾਜ->content) == false)
			ਜਾਰੀ;

		/* Ensure that quirks mode is acceptable */
		ਜੇ (content_matches_quirks(ਇੰਦਰਾਜ->content,
				ctx->ਬੱਚਾ.quirks) == false)
			ਜਾਰੀ;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		ਇੰਦਰਾਜ_llcache = content_get_llcache_handle(ਇੰਦਰਾਜ->content);

		ਜੇ (llcache_handle_references_same_object(ਇੰਦਰਾਜ_llcache,
				ctx->llcache))
			ਰੋਕੋ;
	}

	ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ) {
		/* No existing entry, so need to create one */
		ਇੰਦਰਾਜ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(hlcache_ਇੰਦਰਾਜ));
		ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		/* Create content using llhandle */
		ਇੰਦਰਾਜ->content = content_factory_create_content(ctx->llcache,
				ctx->ਬੱਚਾ.charset, ctx->ਬੱਚਾ.quirks,
				effective_type);
		ਜੇ (ਇੰਦਰਾਜ->content == ਨਿਰਾ) {
			ਮੁਕਤ(ਇੰਦਰਾਜ);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		/* Insert into cache */
		ਇੰਦਰਾਜ->prev = ਨਿਰਾ;
		ਇੰਦਰਾਜ->ਅਗਲਾ = hlcache->content_list;
		ਜੇ (hlcache->content_list != ਨਿਰਾ)
			hlcache->content_list->prev = ਇੰਦਰਾਜ;
		hlcache->content_list = ਇੰਦਰਾਜ;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_ਅੰਕੜੇ;

		hlcache->ਖੁੰਝ_ਗਿਣਤੀ++;
	} ਨਹੀਂਤਾਂ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ++;
	}

	/* Associate handle with content */
	ਜੇ (content_ਜੋੜੋ_user(ਇੰਦਰਾਜ->content,
			hlcache_content_callback, ctx->handle) == false)
		ਵਾਪਸ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->ਇੰਦਰਾਜ = ਇੰਦਰਾਜ;

	/* Catch handle up with state of content */
	ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
		content_ਸਥਿਤੀ ਸਥਿਤੀ = content_get_ਸਥਿਤੀ(ctx->handle);

		ਜੇ (ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_LOADING) {
			event.type = CONTENT_ਸੁਨੇਹਾ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) {
			event.type = CONTENT_ਸੁਨੇਹਾ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
				event.type = CONTENT_ਸੁਨੇਹਾ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ਨਹੀਂਤਾਂ ਜੇ (ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
			event.type = CONTENT_ਸੁਨੇਹਾ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
				event.type = CONTENT_ਸੁਨੇਹਾ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
				event.type = CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ਵਾਪਸ error;
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
ਸਥਿਰ nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_ਨਿਸ਼ਾਨਾ = true;

	ਜੇ ((effective_type != ਨਿਰਾ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_ਲੱਭੋ_content(ctx, effective_type);
		ਜੇ (error != NSERROR_OK && error != NSERROR_NEED_ਅੰਕੜੇ) {
			ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_ਸੁਨੇਹਾ_ERROR;
				hlevent.ਅੰਕੜੇ.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.ਅੰਕੜੇ.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_ਧਾਰਾ(ctx->llcache);

		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_DOWNLOAD;
			hlevent.ਅੰਕੜੇ.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_ਅੰਕੜੇ;
	} ਨਹੀਂਤਾਂ {
		/* Unacceptable type: report error */
		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_ERROR;
			hlevent.ਅੰਕੜੇ.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.ਅੰਕੜੇ.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_ਨਿਸ਼ਾਨਾ = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	ਮੁਕਤ((ਅੱਖਰ *) ctx->ਬੱਚਾ.charset);
	ਮੁਕਤ(ctx);

	ਵਾਪਸ error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ਸਥਿਰ nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ਅਚਲ llcache_event *event,
			 ਖਾਲੀ *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ਨਿਰਾ;
	nserror error;

	ਪ੍ਰਤਿਜ੍ਞਾ(ctx->llcache == handle);

	ਚੋਣ (event->type) {
	ਹਾਲਤ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_SSL_CERTS;
			hlevent.ਅੰਕੜੇ.chain = event->ਅੰਕੜੇ.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_HAD_ਸਿਰਲੇਖਾਂ:
		error = mimesniff_compute_effective_type(llcache_handle_get_ਸਿਰਲੇਖ(handle, "Content-Type"), ਨਿਰਾ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ਜੇ (error == NSERROR_OK || error == NSERROR_NOT_ਮਿਲਿਆ) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ਜੇ (error == NSERROR_NEED_ਅੰਕੜੇ)
			error = NSERROR_OK;

		ਵਾਪਸ error;

		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_HAD_ਅੰਕੜੇ:
		error = mimesniff_compute_effective_type(llcache_handle_get_ਸਿਰਲੇਖ(handle, "Content-Type"),
				event->ਅੰਕੜੇ.ਅੰਕੜੇ.buf, event->ਅੰਕੜੇ.ਅੰਕੜੇ.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ਜੇ (error != NSERROR_OK) {
			ਪ੍ਰਤਿਜ੍ਞਾ(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ਵਾਪਸ error;

		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ਪੂਰਾ:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_ਸਿਰਲੇਖ(handle, "Content-Type"),
				ਨਿਰਾ, 0, false, false, &effective_type);
		ਜੇ (error == NSERROR_OK || error == NSERROR_NOT_ਮਿਲਿਆ) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ਵਾਪਸ error;
		}

		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_ERROR;
			hlevent.ਅੰਕੜੇ.errordata.errorcode = error;
			hlevent.ਅੰਕੜੇ.errordata.errormsg = ਨਿਰਾ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ERROR:
		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_ERROR;
			hlevent.ਅੰਕੜੇ.errordata.errorcode = event->ਅੰਕੜੇ.error.ਕੋਡ;
			hlevent.ਅੰਕੜੇ.errordata.errormsg = event->ਅੰਕੜੇ.error.ਸੁਨੇਹਾ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_ਤਰੱਕੀ:
		ਰੋਕੋ;
	ਹਾਲਤ LLCACHE_EVENT_REDIRECT:
		ਜੇ (ctx->handle->cb != ਨਿਰਾ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ਸੁਨੇਹਾ_REDIRECT;
			hlevent.ਅੰਕੜੇ.redirect.from = event->ਅੰਕੜੇ.redirect.from;
			hlevent.ਅੰਕੜੇ.redirect.ਮੰਜ਼ਿਲ = event->ਅੰਕੜੇ.redirect.ਮੰਜ਼ਿਲ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ਰੋਕੋ;
	}

	ਵਾਪਸ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ਅਚਲ ਬਣਤਰ hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ hlcache_s));
	ਜੇ (hlcache == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ਜੇ (ret != NSERROR_OK) {
		ਮੁਕਤ(hlcache);
		hlcache = ਨਿਰਾ;
		ਵਾਪਸ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(hlcache->params.bg_clean_ਸਮਾਂ, hlcache_clean, ਨਿਰਾ);

	ਵਾਪਸ NSERROR_OK;
}

/* See hlcache.h for documentation */
ਖਾਲੀ hlcache_stop(ਖਾਲੀ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, hlcache_clean, ਨਿਰਾ);
}

/* See hlcache.h for documentation */
ਖਾਲੀ hlcache_finalise(ਖਾਲੀ)
{
	uint32_t ਅੰਕ_contents, prev_contents;
	hlcache_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
	hlcache_retrieval_ctx *ctx, *ਅਗਲਾ;

	/* Obtain initial count of contents remaining */
	ਲਈ (ਅੰਕ_contents = 0, ਇੰਦਰਾਜ = hlcache->content_list;
			ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		ਅੰਕ_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      ਅੰਕ_contents);

	/* Drain cache */
	ਕਰੋ {
		prev_contents = ਅੰਕ_contents;

		hlcache_clean(ਨਿਰਾ);

		ਲਈ (ਅੰਕ_contents = 0, ਇੰਦਰਾਜ = hlcache->content_list;
				ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
			ਅੰਕ_contents++;
		}
	} ਜਦੋਂ (ਅੰਕ_contents > 0 && ਅੰਕ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			ਅੰਕ_contents);

	/* Drain cache again, forcing the matter */
	ਕਰੋ {
		prev_contents = ਅੰਕ_contents;

		hlcache_clean(&ਇੰਦਰਾਜ); // Any non-NULL pointer will do

		ਲਈ (ਅੰਕ_contents = 0, ਇੰਦਰਾਜ = hlcache->content_list;
				ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
			ਅੰਕ_contents++;
		}
	} ਜਦੋਂ (ਅੰਕ_contents > 0 && ਅੰਕ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", ਅੰਕ_contents);
	ਲਈ (ਇੰਦਰਾਜ = hlcache->content_list; ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		hlcache_handle ਇੰਦਰਾਜ_handle = { ਇੰਦਰਾਜ, ਨਿਰਾ, ਨਿਰਾ };

		ਜੇ (ਇੰਦਰਾਜ->content != ਨਿਰਾ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      ਇੰਦਰਾਜ,
			      nsurl_access(hlcache_handle_get_url(&ਇੰਦਰਾਜ_handle)),
			      content_ਗਿਣਤੀ_users(ਇੰਦਰਾਜ->content));
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, INFO, "	%p", ਇੰਦਰਾਜ);
		}
	}

	/* Clean up retrieval contexts */
	ਜੇ (hlcache->retrieval_ctx_ring != ਨਿਰਾ) {
		ctx = hlcache->retrieval_ctx_ring;

		ਕਰੋ {
			ਅਗਲਾ = ctx->r_ਅਗਲਾ;

			ਜੇ (ctx->llcache != ਨਿਰਾ)
				llcache_handle_release(ctx->llcache);

			ਜੇ (ctx->handle != ਨਿਰਾ)
				ਮੁਕਤ(ctx->handle);

			ਜੇ (ctx->ਬੱਚਾ.charset != ਨਿਰਾ)
				ਮੁਕਤ((ਅੱਖਰ *) ctx->ਬੱਚਾ.charset);

			ਮੁਕਤ(ctx);

			ctx = ਅਗਲਾ;
		} ਜਦੋਂ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ਨਿਰਾ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->ਪ੍ਰਾਪਤੀ_ਗਿਣਤੀ,
	      hlcache->ਖੁੰਝ_ਗਿਣਤੀ);

	/* De-schedule ourselves */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, hlcache_clean, ਨਿਰਾ);

	ਮੁਕਤ(hlcache);
	hlcache = ਨਿਰਾ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_ਅੰਕੜੇ *post,
			hlcache_handle_callback cb, ਖਾਲੀ *pw,
			hlcache_ਬੱਚਾ_context *ਬੱਚਾ,
			content_type accepted_types,
			hlcache_handle **ਨਤੀਜਾ)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ਪ੍ਰਤਿਜ੍ਞਾ(cb != ਨਿਰਾ);

	ctx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(hlcache_retrieval_ctx));
	ਜੇ (ctx == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ctx->handle = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(hlcache_handle));
	ਜੇ (ctx->handle == ਨਿਰਾ) {
		ਮੁਕਤ(ctx);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (ਬੱਚਾ != ਨਿਰਾ) {
		ਜੇ (ਬੱਚਾ->charset != ਨਿਰਾ) {
			ctx->ਬੱਚਾ.charset = strdup(ਬੱਚਾ->charset);
			ਜੇ (ctx->ਬੱਚਾ.charset == ਨਿਰਾ) {
				ਮੁਕਤ(ctx->handle);
				ਮੁਕਤ(ctx);
				ਵਾਪਸ NSERROR_NOMEM;
			}
		}
		ctx->ਬੱਚਾ.quirks = ਬੱਚਾ->quirks;
	}

	ctx->flags = flags;
	ctx->accepted_types = accepted_types;

	ctx->handle->cb = cb;
	ctx->handle->pw = pw;

	error = llcache_handle_retrieve(url, flags, referer, post,
			hlcache_llcache_callback, ctx,
			&ctx->llcache);
	ਜੇ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		ਮੁਕਤ((ਅੱਖਰ *) ctx->ਬੱਚਾ.charset);
		ਮੁਕਤ(ctx->handle);
		ਮੁਕਤ(ctx);
	} ਨਹੀਂਤਾਂ {
		/* successfully started fetch so add new context to list */
		RING_ਪਾਓ(hlcache->retrieval_ctx_ring, ctx);

		*ਨਤੀਜਾ = ctx->handle;
	}
	ਵਾਪਸ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ਜੇ (handle->ਇੰਦਰਾਜ != ਨਿਰਾ) {
		content_remove_user(handle->ਇੰਦਰਾਜ->content,
				hlcache_content_callback, handle);
	} ਨਹੀਂਤਾਂ {
		RING_ITERATE_ਸ਼ੁਰੂ(ਬਣਤਰ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ਜੇ (ictx->handle == handle &&
					ictx->migrate_ਨਿਸ਼ਾਨਾ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ਮੁਕਤ((ਅੱਖਰ *) ictx->ਬੱਚਾ.charset);
				ਮੁਕਤ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ਸਿਰਾ(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ਨਿਰਾ;
	handle->pw = ਨਿਰਾ;

	ਮੁਕਤ(handle);

	ਵਾਪਸ NSERROR_OK;
}

/* See hlcache.h for documentation */
ਬਣਤਰ content *hlcache_handle_get_content(ਅਚਲ hlcache_handle *handle)
{
	ਜੇ ((handle != ਨਿਰਾ) && (handle->ਇੰਦਰਾਜ != ਨਿਰਾ)) {
		ਵਾਪਸ handle->ਇੰਦਰਾਜ->content;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ਬਣਤਰ hlcache_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = handle->ਇੰਦਰਾਜ;
	ਬਣਤਰ content *c;

	ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_ਸ਼ੁਰੂ(ਬਣਤਰ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ਜੇ (ictx->handle == handle &&
					ictx->migrate_ਨਿਸ਼ਾਨਾ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ਮੁਕਤ((ਅੱਖਰ *) ictx->ਬੱਚਾ.charset);
				ਮੁਕਤ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ਸਿਰਾ(hlcache->retrieval_ctx_ring, ictx);

		ਵਾਪਸ NSERROR_OK;
	}

	c = ਇੰਦਰਾਜ->content;

	ਜੇ (content_ਗਿਣਤੀ_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ਬਣਤਰ content *clone = content_clone(c);

		ਜੇ (clone == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		ਇੰਦਰਾਜ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ hlcache_ਇੰਦਰਾਜ));

		ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ) {
			content_destroy(clone);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		ਜੇ (content_ਜੋੜੋ_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			ਮੁਕਤ(ਇੰਦਰਾਜ);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		ਇੰਦਰਾਜ->content = clone;
		handle->ਇੰਦਰਾਜ = ਇੰਦਰਾਜ;
		ਇੰਦਰਾਜ->prev = ਨਿਰਾ;
		ਇੰਦਰਾਜ->ਅਗਲਾ = hlcache->content_list;
		ਜੇ (hlcache->content_list != ਨਿਰਾ)
			hlcache->content_list->prev = ਇੰਦਰਾਜ;
		hlcache->content_list = ਇੰਦਰਾਜ;

		c = clone;
	}

	ਵਾਪਸ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, ਖਾਲੀ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ਵਾਪਸ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **ਨਤੀਜਾ)
{
	*ਨਤੀਜਾ = ਨਿਰਾ;
	ਵਾਪਸ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ਅਚਲ hlcache_handle *handle)
{
	nsurl *ਨਤੀਜਾ = ਨਿਰਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(handle != ਨਿਰਾ);

	ਜੇ (handle->ਇੰਦਰਾਜ != ਨਿਰਾ) {
		ਨਤੀਜਾ = content_get_url(handle->ਇੰਦਰਾਜ->content);
	} ਨਹੀਂਤਾਂ {
		RING_ITERATE_ਸ਼ੁਰੂ(ਬਣਤਰ hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ਜੇ (ictx->handle == handle) {
				/* This is the nascent context for us */
				ਨਤੀਜਾ = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_ਸਿਰਾ(hlcache->retrieval_ctx_ring, ictx);
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}
