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

#ضمن <assert.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "utils/http.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/ring.h"
#ضمن "utils/utils.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/misc.h"
#ضمن "netsurf/content.h"
#ضمن "desktop/gui_internal.h"

#ضمن "content/mimesniff.h"
#ضمن "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"

تعريفنوع بنية hlcache_داخلا hlcache_داخلا;
تعريفنوع بنية hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
بنية hlcache_retrieval_ctx {
	بنية hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	بنية hlcache_retrieval_ctx *r_ايندڙ; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_ٻار_context ٻار;	/**< Child context */

	bool migrate_هدف;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
بنية hlcache_handle {
	hlcache_داخلا *داخلا;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	فراغ *pw;			/**< Client data */
};

/** Entry in high-level cache */
بنية hlcache_داخلا {
	بنية content *content;	/**< Pointer to associated content */

	hlcache_داخلا *ايندڙ;		/**< Next sibling */
	hlcache_داخلا *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
بنية hlcache_s {
	بنية hlcache_parameters params;

	/** List of cached content objects */
	hlcache_داخلا *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	بنانشان صحيح ڪاميابي_ڳڻپ;
	بنانشان صحيح ناڪامي_ڳڻپ;
};

/** high level cache state */
ساكن بنية hlcache_s *hlcache = عدم;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ساكن فراغ hlcache_clean(فراغ *force_clean_flag)
{
	hlcache_داخلا *داخلا, *ايندڙ;
	bool force_clean = (force_clean_flag != عدم);

	لكل (داخلا = hlcache->content_list; داخلا != عدم; داخلا = ايندڙ) {
		ايندڙ = داخلا->ايندڙ;

		إذا (داخلا->content == عدم)
			استمر;

		إذا (content_ڳڻپ_users(داخلا->content) != 0)
			استمر;

		إذا (content_get_حالت_2(داخلا->content) == CONTENT_حالت_LOADING) {
			إذا (force_clean == false)
				استمر;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(داخلا->content);
			content_سيٽ_error(داخلا->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		إذا (داخلا->prev == عدم)
			hlcache->content_list = داخلا->ايندڙ;
		وإلا
			داخلا->prev->ايندڙ = داخلا->ايندڙ;

		إذا (داخلا->ايندڙ != عدم)
			داخلا->ايندڙ->prev = داخلا->prev;

		/* Destroy content */
		content_destroy(داخلا->content);

		/* Destroy entry */
		حرر(داخلا);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->وقت_جدول(hlcache->params.bg_clean_وقت, hlcache_clean, عدم);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ساكن bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	أرجع ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ساكن فراغ hlcache_content_callback(بنية content *c, content_نياپو نياپو,
		ثابت اتحاد content_نياپو_معلومات *معلومات, فراغ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = نياپو,
	};

	إذا (معلومات != عدم) {
		event.معلومات = *معلومات;
	}

	إذا (handle->cb != عدم)
		error = handle->cb(handle, &event, handle->pw);

	إذا (error != NSERROR_OK)
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
ساكن nserror hlcache_ڳوليو_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_داخلا *داخلا;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	لكل (داخلا = hlcache->content_list; داخلا != عدم; داخلا = داخلا->ايندڙ) {
		hlcache_handle داخلا_handle = { داخلا, عدم, عدم };
		ثابت llcache_handle *داخلا_llcache;

		إذا (داخلا->content == عدم)
			استمر;

		/* Ignore contents in the error state */
		إذا (content_get_حالت(&داخلا_handle) == CONTENT_حالت_ERROR)
			استمر;

		/* Ensure that content is shareable */
		إذا (content_is_shareable(داخلا->content) == false)
			استمر;

		/* Ensure that quirks mode is acceptable */
		إذا (content_matches_quirks(داخلا->content,
				ctx->ٻار.quirks) == false)
			استمر;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		داخلا_llcache = content_get_llcache_handle(داخلا->content);

		إذا (llcache_handle_references_same_object(داخلا_llcache,
				ctx->llcache))
			اكسر;
	}

	إذا (داخلا == عدم) {
		/* No existing entry, so need to create one */
		داخلا = يادگيري_ڏيو(حجم(hlcache_داخلا));
		إذا (داخلا == عدم)
			أرجع NSERROR_NOMEM;

		/* Create content using llhandle */
		داخلا->content = content_factory_create_content(ctx->llcache,
				ctx->ٻار.charset, ctx->ٻار.quirks,
				effective_type);
		إذا (داخلا->content == عدم) {
			حرر(داخلا);
			أرجع NSERROR_NOMEM;
		}

		/* Insert into cache */
		داخلا->prev = عدم;
		داخلا->ايندڙ = hlcache->content_list;
		إذا (hlcache->content_list != عدم)
			hlcache->content_list->prev = داخلا;
		hlcache->content_list = داخلا;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_معلومات;

		hlcache->ناڪامي_ڳڻپ++;
	} وإلا {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->ڪاميابي_ڳڻپ++;
	}

	/* Associate handle with content */
	إذا (content_جوڙيو_user(داخلا->content,
			hlcache_content_callback, ctx->handle) == false)
		أرجع NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->داخلا = داخلا;

	/* Catch handle up with state of content */
	إذا (ctx->handle->cb != عدم) {
		content_حالت حالت = content_get_حالت(ctx->handle);

		إذا (حالت == CONTENT_حالت_LOADING) {
			event.type = CONTENT_نياپو_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} وإلا إذا (حالت == CONTENT_حالت_READY) {
			event.type = CONTENT_نياپو_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_نياپو_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} وإلا إذا (حالت == CONTENT_حالت_پورو) {
			event.type = CONTENT_نياپو_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_نياپو_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_نياپو_پورو;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	أرجع error;
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
ساكن nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_هدف = true;

	إذا ((effective_type != عدم) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_ڳوليو_content(ctx, effective_type);
		إذا (error != NSERROR_OK && error != NSERROR_NEED_معلومات) {
			إذا (ctx->handle->cb != عدم) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_نياپو_ERROR;
				hlevent.معلومات.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.معلومات.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} وإلا إذا (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_وهڪرو(ctx->llcache);

		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_DOWNLOAD;
			hlevent.معلومات.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_معلومات;
	} وإلا {
		/* Unacceptable type: report error */
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_ERROR;
			hlevent.معلومات.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.معلومات.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_هدف = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	حرر((حرف *) ctx->ٻار.charset);
	حرر(ctx);

	أرجع error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساكن nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ثابت llcache_event *event,
			 فراغ *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = عدم;
	nserror error;

	أكد(ctx->llcache == handle);

	بدل (event->type) {
	حالة LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_SSL_CERTS;
			hlevent.معلومات.chain = event->معلومات.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_HAD_سرا:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرو(handle, "Content-Type"), عدم, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		إذا (error == NSERROR_OK || error == NSERROR_NOT_مليو) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		إذا (error == NSERROR_NEED_معلومات)
			error = NSERROR_OK;

		أرجع error;

		اكسر;
	حالة LLCACHE_EVENT_HAD_معلومات:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرو(handle, "Content-Type"),
				event->معلومات.معلومات.buf, event->معلومات.معلومات.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		إذا (error != NSERROR_OK) {
			أكد(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		أرجع error;

		اكسر;
	حالة LLCACHE_EVENT_پورو:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_سرو(handle, "Content-Type"),
				عدم, 0, false, false, &effective_type);
		إذا (error == NSERROR_OK || error == NSERROR_NOT_مليو) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			أرجع error;
		}

		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_ERROR;
			hlevent.معلومات.errordata.errorcode = error;
			hlevent.معلومات.errordata.errormsg = عدم;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_ERROR:
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_ERROR;
			hlevent.معلومات.errordata.errorcode = event->معلومات.error.ڪوڊ;
			hlevent.معلومات.errordata.errormsg = event->معلومات.error.نياپو;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_ترقي:
		اكسر;
	حالة LLCACHE_EVENT_REDIRECT:
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_نياپو_REDIRECT;
			hlevent.معلومات.redirect.from = event->معلومات.redirect.from;
			hlevent.معلومات.redirect.منزل = event->معلومات.redirect.منزل;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	}

	أرجع NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ثابت بنية hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = صفر_يادگيري_ڏيو(1, حجم(بنية hlcache_s));
	إذا (hlcache == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	إذا (ret != NSERROR_OK) {
		حرر(hlcache);
		hlcache = عدم;
		أرجع ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->وقت_جدول(hlcache->params.bg_clean_وقت, hlcache_clean, عدم);

	أرجع NSERROR_OK;
}

/* See hlcache.h for documentation */
فراغ hlcache_stop(فراغ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->وقت_جدول(-1, hlcache_clean, عدم);
}

/* See hlcache.h for documentation */
فراغ hlcache_finalise(فراغ)
{
	uint32_t انگ_contents, prev_contents;
	hlcache_داخلا *داخلا;
	hlcache_retrieval_ctx *ctx, *ايندڙ;

	/* Obtain initial count of contents remaining */
	لكل (انگ_contents = 0, داخلا = hlcache->content_list;
			داخلا != عدم; داخلا = داخلا->ايندڙ) {
		انگ_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      انگ_contents);

	/* Drain cache */
	افعل {
		prev_contents = انگ_contents;

		hlcache_clean(عدم);

		لكل (انگ_contents = 0, داخلا = hlcache->content_list;
				داخلا != عدم; داخلا = داخلا->ايندڙ) {
			انگ_contents++;
		}
	} طالما (انگ_contents > 0 && انگ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			انگ_contents);

	/* Drain cache again, forcing the matter */
	افعل {
		prev_contents = انگ_contents;

		hlcache_clean(&داخلا); // Any non-NULL pointer will do

		لكل (انگ_contents = 0, داخلا = hlcache->content_list;
				داخلا != عدم; داخلا = داخلا->ايندڙ) {
			انگ_contents++;
		}
	} طالما (انگ_contents > 0 && انگ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", انگ_contents);
	لكل (داخلا = hlcache->content_list; داخلا != عدم; داخلا = داخلا->ايندڙ) {
		hlcache_handle داخلا_handle = { داخلا, عدم, عدم };

		إذا (داخلا->content != عدم) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      داخلا,
			      nsurl_access(hlcache_handle_get_url(&داخلا_handle)),
			      content_ڳڻپ_users(داخلا->content));
		} وإلا {
			NSLOG(netsurf, INFO, "	%p", داخلا);
		}
	}

	/* Clean up retrieval contexts */
	إذا (hlcache->retrieval_ctx_ring != عدم) {
		ctx = hlcache->retrieval_ctx_ring;

		افعل {
			ايندڙ = ctx->r_ايندڙ;

			إذا (ctx->llcache != عدم)
				llcache_handle_release(ctx->llcache);

			إذا (ctx->handle != عدم)
				حرر(ctx->handle);

			إذا (ctx->ٻار.charset != عدم)
				حرر((حرف *) ctx->ٻار.charset);

			حرر(ctx);

			ctx = ايندڙ;
		} طالما (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = عدم;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->ڪاميابي_ڳڻپ,
	      hlcache->ناڪامي_ڳڻپ);

	/* De-schedule ourselves */
	guit->misc->وقت_جدول(-1, hlcache_clean, عدم);

	حرر(hlcache);
	hlcache = عدم;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_معلومات *post,
			hlcache_handle_callback cb, فراغ *pw,
			hlcache_ٻار_context *ٻار,
			content_type accepted_types,
			hlcache_handle **نتيجو)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	أكد(cb != عدم);

	ctx = صفر_يادگيري_ڏيو(1, حجم(hlcache_retrieval_ctx));
	إذا (ctx == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ctx->handle = صفر_يادگيري_ڏيو(1, حجم(hlcache_handle));
	إذا (ctx->handle == عدم) {
		حرر(ctx);
		أرجع NSERROR_NOMEM;
	}

	إذا (ٻار != عدم) {
		إذا (ٻار->charset != عدم) {
			ctx->ٻار.charset = strdup(ٻار->charset);
			إذا (ctx->ٻار.charset == عدم) {
				حرر(ctx->handle);
				حرر(ctx);
				أرجع NSERROR_NOMEM;
			}
		}
		ctx->ٻار.quirks = ٻار->quirks;
	}

	ctx->flags = flags;
	ctx->accepted_types = accepted_types;

	ctx->handle->cb = cb;
	ctx->handle->pw = pw;

	error = llcache_handle_retrieve(url, flags, referer, post,
			hlcache_llcache_callback, ctx,
			&ctx->llcache);
	إذا (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		حرر((حرف *) ctx->ٻار.charset);
		حرر(ctx->handle);
		حرر(ctx);
	} وإلا {
		/* successfully started fetch so add new context to list */
		RING_وجهو(hlcache->retrieval_ctx_ring, ctx);

		*نتيجو = ctx->handle;
	}
	أرجع error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	إذا (handle->داخلا != عدم) {
		content_remove_user(handle->داخلا->content,
				hlcache_content_callback, handle);
	} وإلا {
		RING_ITERATE_شروعات(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle &&
					ictx->migrate_هدف == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->ٻار.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_پڇاڙي(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = عدم;
	handle->pw = عدم;

	حرر(handle);

	أرجع NSERROR_OK;
}

/* See hlcache.h for documentation */
بنية content *hlcache_handle_get_content(ثابت hlcache_handle *handle)
{
	إذا ((handle != عدم) && (handle->داخلا != عدم)) {
		أرجع handle->داخلا->content;
	}

	أرجع عدم;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	بنية hlcache_داخلا *داخلا = handle->داخلا;
	بنية content *c;

	إذا (داخلا == عدم) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_شروعات(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle &&
					ictx->migrate_هدف == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->ٻار.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_پڇاڙي(hlcache->retrieval_ctx_ring, ictx);

		أرجع NSERROR_OK;
	}

	c = داخلا->content;

	إذا (content_ڳڻپ_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		بنية content *clone = content_clone(c);

		إذا (clone == عدم)
			أرجع NSERROR_NOMEM;

		داخلا = صفر_يادگيري_ڏيو(1, حجم(بنية hlcache_داخلا));

		إذا (داخلا == عدم) {
			content_destroy(clone);
			أرجع NSERROR_NOMEM;
		}

		إذا (content_جوڙيو_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			حرر(داخلا);
			أرجع NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		داخلا->content = clone;
		handle->داخلا = داخلا;
		داخلا->prev = عدم;
		داخلا->ايندڙ = hlcache->content_list;
		إذا (hlcache->content_list != عدم)
			hlcache->content_list->prev = داخلا;
		hlcache->content_list = داخلا;

		c = clone;
	}

	أرجع content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, فراغ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	أرجع NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **نتيجو)
{
	*نتيجو = عدم;
	أرجع NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ثابت hlcache_handle *handle)
{
	nsurl *نتيجو = عدم;

	أكد(handle != عدم);

	إذا (handle->داخلا != عدم) {
		نتيجو = content_get_url(handle->داخلا->content);
	} وإلا {
		RING_ITERATE_شروعات(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle) {
				/* This is the nascent context for us */
				نتيجو = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_پڇاڙي(hlcache->retrieval_ctx_ring, ictx);
	}

	أرجع نتيجو;
}
