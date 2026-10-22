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

تعريفنوع بنية hlcache_اندراج hlcache_اندراج;
تعريفنوع بنية hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
بنية hlcache_retrieval_ctx {
	بنية hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	بنية hlcache_retrieval_ctx *r_اگلا; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_بچہ_context بچہ;	/**< Child context */

	bool migrate_نشانہ;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
بنية hlcache_handle {
	hlcache_اندراج *اندراج;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	فراغ *pw;			/**< Client data */
};

/** Entry in high-level cache */
بنية hlcache_اندراج {
	بنية content *content;	/**< Pointer to associated content */

	hlcache_اندراج *اگلا;		/**< Next sibling */
	hlcache_اندراج *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
بنية hlcache_s {
	بنية hlcache_parameters params;

	/** List of cached content objects */
	hlcache_اندراج *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	بنانشانی صحيح کامیابی_گنتی;
	بنانشانی صحيح کھنج_گنتی;
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
	hlcache_اندراج *اندراج, *اگلا;
	bool force_clean = (force_clean_flag != عدم);

	لكل (اندراج = hlcache->content_list; اندراج != عدم; اندراج = اگلا) {
		اگلا = اندراج->اگلا;

		إذا (اندراج->content == عدم)
			استمر;

		إذا (content_گنتی_users(اندراج->content) != 0)
			استمر;

		إذا (content_get_حالت_2(اندراج->content) == CONTENT_حالت_LOADING) {
			إذا (force_clean == false)
				استمر;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(اندراج->content);
			content_سیٹ_error(اندراج->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		إذا (اندراج->prev == عدم)
			hlcache->content_list = اندراج->اگلا;
		وإلا
			اندراج->prev->اگلا = اندراج->اگلا;

		إذا (اندراج->اگلا != عدم)
			اندراج->اگلا->prev = اندراج->prev;

		/* Destroy content */
		content_destroy(اندراج->content);

		/* Destroy entry */
		حرر(اندراج);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->ویلا_سارنی(hlcache->params.bg_clean_ویلا, hlcache_clean, عدم);
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
ساكن فراغ hlcache_content_callback(بنية content *c, content_پیغام پیغام,
		ثابت اتحاد content_پیغام_انکڑے *انکڑے, فراغ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = پیغام,
	};

	إذا (انکڑے != عدم) {
		event.انکڑے = *انکڑے;
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
ساكن nserror hlcache_لبھو_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_اندراج *اندراج;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	لكل (اندراج = hlcache->content_list; اندراج != عدم; اندراج = اندراج->اگلا) {
		hlcache_handle اندراج_handle = { اندراج, عدم, عدم };
		ثابت llcache_handle *اندراج_llcache;

		إذا (اندراج->content == عدم)
			استمر;

		/* Ignore contents in the error state */
		إذا (content_get_حالت(&اندراج_handle) == CONTENT_حالت_ERROR)
			استمر;

		/* Ensure that content is shareable */
		إذا (content_is_shareable(اندراج->content) == false)
			استمر;

		/* Ensure that quirks mode is acceptable */
		إذا (content_matches_quirks(اندراج->content,
				ctx->بچہ.quirks) == false)
			استمر;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		اندراج_llcache = content_get_llcache_handle(اندراج->content);

		إذا (llcache_handle_references_same_object(اندراج_llcache,
				ctx->llcache))
			اكسر;
	}

	إذا (اندراج == عدم) {
		/* No existing entry, so need to create one */
		اندراج = حافظہ_دیو(حجم(hlcache_اندراج));
		إذا (اندراج == عدم)
			أرجع NSERROR_NOMEM;

		/* Create content using llhandle */
		اندراج->content = content_factory_create_content(ctx->llcache,
				ctx->بچہ.charset, ctx->بچہ.quirks,
				effective_type);
		إذا (اندراج->content == عدم) {
			حرر(اندراج);
			أرجع NSERROR_NOMEM;
		}

		/* Insert into cache */
		اندراج->prev = عدم;
		اندراج->اگلا = hlcache->content_list;
		إذا (hlcache->content_list != عدم)
			hlcache->content_list->prev = اندراج;
		hlcache->content_list = اندراج;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_انکڑے;

		hlcache->کھنج_گنتی++;
	} وإلا {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->کامیابی_گنتی++;
	}

	/* Associate handle with content */
	إذا (content_جوڑو_user(اندراج->content,
			hlcache_content_callback, ctx->handle) == false)
		أرجع NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->اندراج = اندراج;

	/* Catch handle up with state of content */
	إذا (ctx->handle->cb != عدم) {
		content_حالت حالت = content_get_حالت(ctx->handle);

		إذا (حالت == CONTENT_حالت_LOADING) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} وإلا إذا (حالت == CONTENT_حالت_READY) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_پیغام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} وإلا إذا (حالت == CONTENT_حالت_پورا) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_پیغام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			إذا (ctx->handle->cb != عدم) {
				event.type = CONTENT_پیغام_پورا;
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

	ctx->migrate_نشانہ = true;

	إذا ((effective_type != عدم) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_لبھو_content(ctx, effective_type);
		إذا (error != NSERROR_OK && error != NSERROR_NEED_انکڑے) {
			إذا (ctx->handle->cb != عدم) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_پیغام_ERROR;
				hlevent.انکڑے.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.انکڑے.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} وإلا إذا (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_دھارا(ctx->llcache);

		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_DOWNLOAD;
			hlevent.انکڑے.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_انکڑے;
	} وإلا {
		/* Unacceptable type: report error */
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.انکڑے.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.انکڑے.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_نشانہ = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	حرر((حرف *) ctx->بچہ.charset);
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

			hlevent.type = CONTENT_پیغام_SSL_CERTS;
			hlevent.انکڑے.chain = event->انکڑے.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_HAD_سرخیاں:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"), عدم, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		إذا (error == NSERROR_OK || error == NSERROR_NOT_لبھیا) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		إذا (error == NSERROR_NEED_انکڑے)
			error = NSERROR_OK;

		أرجع error;

		اكسر;
	حالة LLCACHE_EVENT_HAD_انکڑے:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"),
				event->انکڑے.انکڑے.buf, event->انکڑے.انکڑے.len,
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
	حالة LLCACHE_EVENT_پورا:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"),
				عدم, 0, false, false, &effective_type);
		إذا (error == NSERROR_OK || error == NSERROR_NOT_لبھیا) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			أرجع error;
		}

		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.انکڑے.errordata.errorcode = error;
			hlevent.انکڑے.errordata.errormsg = عدم;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_ERROR:
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.انکڑے.errordata.errorcode = event->انکڑے.error.کوڈ;
			hlevent.انکڑے.errordata.errormsg = event->انکڑے.error.پیغام;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_ترقی:
		اكسر;
	حالة LLCACHE_EVENT_REDIRECT:
		إذا (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_REDIRECT;
			hlevent.انکڑے.redirect.from = event->انکڑے.redirect.from;
			hlevent.انکڑے.redirect.منزل = event->انکڑے.redirect.منزل;

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

	hlcache = صفر_حافظہ_دیو(1, حجم(بنية hlcache_s));
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
	guit->misc->ویلا_سارنی(hlcache->params.bg_clean_ویلا, hlcache_clean, عدم);

	أرجع NSERROR_OK;
}

/* See hlcache.h for documentation */
فراغ hlcache_stop(فراغ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->ویلا_سارنی(-1, hlcache_clean, عدم);
}

/* See hlcache.h for documentation */
فراغ hlcache_finalise(فراغ)
{
	uint32_t عدد_contents, prev_contents;
	hlcache_اندراج *اندراج;
	hlcache_retrieval_ctx *ctx, *اگلا;

	/* Obtain initial count of contents remaining */
	لكل (عدد_contents = 0, اندراج = hlcache->content_list;
			اندراج != عدم; اندراج = اندراج->اگلا) {
		عدد_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      عدد_contents);

	/* Drain cache */
	افعل {
		prev_contents = عدد_contents;

		hlcache_clean(عدم);

		لكل (عدد_contents = 0, اندراج = hlcache->content_list;
				اندراج != عدم; اندراج = اندراج->اگلا) {
			عدد_contents++;
		}
	} طالما (عدد_contents > 0 && عدد_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			عدد_contents);

	/* Drain cache again, forcing the matter */
	افعل {
		prev_contents = عدد_contents;

		hlcache_clean(&اندراج); // Any non-NULL pointer will do

		لكل (عدد_contents = 0, اندراج = hlcache->content_list;
				اندراج != عدم; اندراج = اندراج->اگلا) {
			عدد_contents++;
		}
	} طالما (عدد_contents > 0 && عدد_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", عدد_contents);
	لكل (اندراج = hlcache->content_list; اندراج != عدم; اندراج = اندراج->اگلا) {
		hlcache_handle اندراج_handle = { اندراج, عدم, عدم };

		إذا (اندراج->content != عدم) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      اندراج,
			      nsurl_access(hlcache_handle_get_url(&اندراج_handle)),
			      content_گنتی_users(اندراج->content));
		} وإلا {
			NSLOG(netsurf, INFO, "	%p", اندراج);
		}
	}

	/* Clean up retrieval contexts */
	إذا (hlcache->retrieval_ctx_ring != عدم) {
		ctx = hlcache->retrieval_ctx_ring;

		افعل {
			اگلا = ctx->r_اگلا;

			إذا (ctx->llcache != عدم)
				llcache_handle_release(ctx->llcache);

			إذا (ctx->handle != عدم)
				حرر(ctx->handle);

			إذا (ctx->بچہ.charset != عدم)
				حرر((حرف *) ctx->بچہ.charset);

			حرر(ctx);

			ctx = اگلا;
		} طالما (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = عدم;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->کامیابی_گنتی,
	      hlcache->کھنج_گنتی);

	/* De-schedule ourselves */
	guit->misc->ویلا_سارنی(-1, hlcache_clean, عدم);

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
			llcache_post_انکڑے *post,
			hlcache_handle_callback cb, فراغ *pw,
			hlcache_بچہ_context *بچہ,
			content_type accepted_types,
			hlcache_handle **نتیجہ)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	أكد(cb != عدم);

	ctx = صفر_حافظہ_دیو(1, حجم(hlcache_retrieval_ctx));
	إذا (ctx == عدم) {
		أرجع NSERROR_NOMEM;
	}

	ctx->handle = صفر_حافظہ_دیو(1, حجم(hlcache_handle));
	إذا (ctx->handle == عدم) {
		حرر(ctx);
		أرجع NSERROR_NOMEM;
	}

	إذا (بچہ != عدم) {
		إذا (بچہ->charset != عدم) {
			ctx->بچہ.charset = strdup(بچہ->charset);
			إذا (ctx->بچہ.charset == عدم) {
				حرر(ctx->handle);
				حرر(ctx);
				أرجع NSERROR_NOMEM;
			}
		}
		ctx->بچہ.quirks = بچہ->quirks;
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
		حرر((حرف *) ctx->بچہ.charset);
		حرر(ctx->handle);
		حرر(ctx);
	} وإلا {
		/* successfully started fetch so add new context to list */
		RING_داخل_کرو(hlcache->retrieval_ctx_ring, ctx);

		*نتیجہ = ctx->handle;
	}
	أرجع error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	إذا (handle->اندراج != عدم) {
		content_remove_user(handle->اندراج->content,
				hlcache_content_callback, handle);
	} وإلا {
		RING_ITERATE_شروع(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle &&
					ictx->migrate_نشانہ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->بچہ.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_سرا(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = عدم;
	handle->pw = عدم;

	حرر(handle);

	أرجع NSERROR_OK;
}

/* See hlcache.h for documentation */
بنية content *hlcache_handle_get_content(ثابت hlcache_handle *handle)
{
	إذا ((handle != عدم) && (handle->اندراج != عدم)) {
		أرجع handle->اندراج->content;
	}

	أرجع عدم;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	بنية hlcache_اندراج *اندراج = handle->اندراج;
	بنية content *c;

	إذا (اندراج == عدم) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_شروع(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle &&
					ictx->migrate_نشانہ == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->بچہ.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_سرا(hlcache->retrieval_ctx_ring, ictx);

		أرجع NSERROR_OK;
	}

	c = اندراج->content;

	إذا (content_گنتی_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		بنية content *clone = content_clone(c);

		إذا (clone == عدم)
			أرجع NSERROR_NOMEM;

		اندراج = صفر_حافظہ_دیو(1, حجم(بنية hlcache_اندراج));

		إذا (اندراج == عدم) {
			content_destroy(clone);
			أرجع NSERROR_NOMEM;
		}

		إذا (content_جوڑو_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			حرر(اندراج);
			أرجع NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		اندراج->content = clone;
		handle->اندراج = اندراج;
		اندراج->prev = عدم;
		اندراج->اگلا = hlcache->content_list;
		إذا (hlcache->content_list != عدم)
			hlcache->content_list->prev = اندراج;
		hlcache->content_list = اندراج;

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

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **نتیجہ)
{
	*نتیجہ = عدم;
	أرجع NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ثابت hlcache_handle *handle)
{
	nsurl *نتیجہ = عدم;

	أكد(handle != عدم);

	إذا (handle->اندراج != عدم) {
		نتیجہ = content_get_url(handle->اندراج->content);
	} وإلا {
		RING_ITERATE_شروع(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			إذا (ictx->handle == handle) {
				/* This is the nascent context for us */
				نتیجہ = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_سرا(hlcache->retrieval_ctx_ring, ictx);
	}

	أرجع نتیجہ;
}
