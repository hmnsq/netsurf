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

#شامل <assert.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "utils/http.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/ring.h"
#شامل "utils/utils.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/content.h"
#شامل "desktop/gui_internal.h"

#شامل "content/mimesniff.h"
#شامل "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"

قسمتعریف ساخت hlcache_اندراج hlcache_اندراج;
قسمتعریف ساخت hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ساخت hlcache_retrieval_ctx {
	ساخت hlcache_retrieval_ctx *r_پچھلا; /**< Previous retrieval context in the ring */
	ساخت hlcache_retrieval_ctx *r_اگلا; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_بچہ_context بچہ;	/**< Child context */

	bool migrate_ہدف;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ساخت hlcache_handle {
	hlcache_اندراج *اندراج_2;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	خالی *pw;			/**< Client data */
};

/** Entry in high-level cache */
ساخت hlcache_اندراج {
	ساخت content *content;	/**< Pointer to associated content */

	hlcache_اندراج *اگلا;		/**< Next sibling */
	hlcache_اندراج *پچھلا;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ساخت hlcache_s {
	ساخت hlcache_parameters params;

	/** List of cached content objects */
	hlcache_اندراج *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	غیرعلامتی صحیح کامیابی_گنتی;
	غیرعلامتی صحیح ناکامی_گنتی;
};

/** high level cache state */
ساکن ساخت hlcache_s *hlcache = تہی;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ساکن خالی hlcache_clean(خالی *force_clean_flag)
{
	hlcache_اندراج *اندراج_2, *اگلا;
	bool force_clean = (force_clean_flag != تہی);

	برائے (اندراج_2 = hlcache->content_list; اندراج_2 != تہی; اندراج_2 = اگلا) {
		اگلا = اندراج_2->اگلا;

		اگر (اندراج_2->content == تہی)
			جاری;

		اگر (content_گنتی_users(اندراج_2->content) != 0)
			جاری;

		اگر (content_get_حالت_2(اندراج_2->content) == CONTENT_حالت_LOADING) {
			اگر (force_clean == false)
				جاری;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(اندراج_2->content);
			content_سیٹ_error(اندراج_2->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		اگر (اندراج_2->پچھلا == تہی)
			hlcache->content_list = اندراج_2->اگلا;
		ورنہ
			اندراج_2->پچھلا->اگلا = اندراج_2->اگلا;

		اگر (اندراج_2->اگلا != تہی)
			اندراج_2->اگلا->پچھلا = اندراج_2->پچھلا;

		/* Destroy content */
		content_destroy(اندراج_2->content);

		/* Destroy entry */
		آزاد(اندراج_2);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->جدول_وقت(hlcache->params.bg_clean_وقت, hlcache_clean, تہی);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ساکن bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	واپس ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ساکن خالی hlcache_content_callback(ساخت content *c, content_پیغام پیغام,
		ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا, خالی *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = پیغام,
	};

	اگر (ڈیٹا != تہی) {
		event.ڈیٹا = *ڈیٹا;
	}

	اگر (handle->cb != تہی)
		error = handle->cb(handle, &event, handle->pw);

	اگر (error != NSERROR_OK)
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
ساکن nserror hlcache_ڈھونڈو_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_اندراج *اندراج_2;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	برائے (اندراج_2 = hlcache->content_list; اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
		hlcache_handle اندراج_handle = { اندراج_2, تہی, تہی };
		ثابت llcache_handle *اندراج_llcache;

		اگر (اندراج_2->content == تہی)
			جاری;

		/* Ignore contents in the error state */
		اگر (content_get_حالت(&اندراج_handle) == CONTENT_حالت_ERROR)
			جاری;

		/* Ensure that content is shareable */
		اگر (content_is_shareable(اندراج_2->content) == false)
			جاری;

		/* Ensure that quirks mode is acceptable */
		اگر (content_matches_quirks(اندراج_2->content,
				ctx->بچہ.quirks) == false)
			جاری;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		اندراج_llcache = content_get_llcache_handle(اندراج_2->content);

		اگر (llcache_handle_references_same_object(اندراج_llcache,
				ctx->llcache))
			توڑو;
	}

	اگر (اندراج_2 == تہی) {
		/* No existing entry, so need to create one */
		اندراج_2 = حافظہ_دو(حجم(hlcache_اندراج));
		اگر (اندراج_2 == تہی)
			واپس NSERROR_NOMEM;

		/* Create content using llhandle */
		اندراج_2->content = content_factory_create_content(ctx->llcache,
				ctx->بچہ.charset, ctx->بچہ.quirks,
				effective_type);
		اگر (اندراج_2->content == تہی) {
			آزاد(اندراج_2);
			واپس NSERROR_NOMEM;
		}

		/* Insert into cache */
		اندراج_2->پچھلا = تہی;
		اندراج_2->اگلا = hlcache->content_list;
		اگر (hlcache->content_list != تہی)
			hlcache->content_list->پچھلا = اندراج_2;
		hlcache->content_list = اندراج_2;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_ڈیٹا;

		hlcache->ناکامی_گنتی++;
	} ورنہ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->کامیابی_گنتی++;
	}

	/* Associate handle with content */
	اگر (content_جوڑو_user(اندراج_2->content,
			hlcache_content_callback, ctx->handle) == false)
		واپس NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->اندراج_2 = اندراج_2;

	/* Catch handle up with state of content */
	اگر (ctx->handle->cb != تہی) {
		content_حالت حالت_2 = content_get_حالت(ctx->handle);

		اگر (حالت_2 == CONTENT_حالت_LOADING) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ورنہ اگر (حالت_2 == CONTENT_حالت_READY) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			اگر (ctx->handle->cb != تہی) {
				event.type = CONTENT_پیغام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ورنہ اگر (حالت_2 == CONTENT_حالت_مکمل) {
			event.type = CONTENT_پیغام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			اگر (ctx->handle->cb != تہی) {
				event.type = CONTENT_پیغام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			اگر (ctx->handle->cb != تہی) {
				event.type = CONTENT_پیغام_مکمل;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	واپس error;
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
ساکن nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_ہدف = true;

	اگر ((effective_type != تہی) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_ڈھونڈو_content(ctx, effective_type);
		اگر (error != NSERROR_OK && error != NSERROR_NEED_ڈیٹا) {
			اگر (ctx->handle->cb != تہی) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_پیغام_ERROR;
				hlevent.ڈیٹا.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.ڈیٹا.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ورنہ اگر (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_دھارا(ctx->llcache);

		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_DOWNLOAD;
			hlevent.ڈیٹا.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_ڈیٹا;
	} ورنہ {
		/* Unacceptable type: report error */
		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.ڈیٹا.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.ڈیٹا.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_ہدف = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	آزاد((حرف *) ctx->بچہ.charset);
	آزاد(ctx);

	واپس error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ساکن nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ثابت llcache_event *event,
			 خالی *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = تہی;
	nserror error;

	أكد(ctx->llcache == handle);

	انتخاب (event->type) {
	حالت LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_SSL_CERTS;
			hlevent.ڈیٹا.chain = event->ڈیٹا.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		توڑو;
	حالت LLCACHE_EVENT_HAD_سرخیاں:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"), تہی, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		اگر (error == NSERROR_OK || error == NSERROR_NOT_ملا) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		اگر (error == NSERROR_NEED_ڈیٹا)
			error = NSERROR_OK;

		واپس error;

		توڑو;
	حالت LLCACHE_EVENT_HAD_ڈیٹا:
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"),
				event->ڈیٹا.ڈیٹا.buf, event->ڈیٹا.ڈیٹا.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		اگر (error != NSERROR_OK) {
			أكد(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		واپس error;

		توڑو;
	حالت LLCACHE_EVENT_مکمل:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_سرخی(handle, "Content-Type"),
				تہی, 0, false, false, &effective_type);
		اگر (error == NSERROR_OK || error == NSERROR_NOT_ملا) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			واپس error;
		}

		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.ڈیٹا.errordata.errorcode = error;
			hlevent.ڈیٹا.errordata.errormsg = تہی;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		توڑو;
	حالت LLCACHE_EVENT_ERROR:
		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_ERROR;
			hlevent.ڈیٹا.errordata.errorcode = event->ڈیٹا.error.رمز;
			hlevent.ڈیٹا.errordata.errormsg = event->ڈیٹا.error.پیغام;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		توڑو;
	حالت LLCACHE_EVENT_پیش_رفت:
		توڑو;
	حالت LLCACHE_EVENT_REDIRECT:
		اگر (ctx->handle->cb != تہی) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیغام_REDIRECT;
			hlevent.ڈیٹا.redirect.from = event->ڈیٹا.redirect.from;
			hlevent.ڈیٹا.redirect.مقصد = event->ڈیٹا.redirect.مقصد;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		توڑو;
	}

	واپس NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ثابت ساخت hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = صفر_حافظہ_دو(1, حجم(ساخت hlcache_s));
	اگر (hlcache == تہی) {
		واپس NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	اگر (ret != NSERROR_OK) {
		آزاد(hlcache);
		hlcache = تہی;
		واپس ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->جدول_وقت(hlcache->params.bg_clean_وقت, hlcache_clean, تہی);

	واپس NSERROR_OK;
}

/* See hlcache.h for documentation */
خالی hlcache_stop(خالی)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->جدول_وقت(-1, hlcache_clean, تہی);
}

/* See hlcache.h for documentation */
خالی hlcache_finalise(خالی)
{
	uint32_t عدد_contents, پچھلا_contents;
	hlcache_اندراج *اندراج_2;
	hlcache_retrieval_ctx *ctx, *اگلا;

	/* Obtain initial count of contents remaining */
	برائے (عدد_contents = 0, اندراج_2 = hlcache->content_list;
			اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
		عدد_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      عدد_contents);

	/* Drain cache */
	کرو {
		پچھلا_contents = عدد_contents;

		hlcache_clean(تہی);

		برائے (عدد_contents = 0, اندراج_2 = hlcache->content_list;
				اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
			عدد_contents++;
		}
	} جب (عدد_contents > 0 && عدد_contents != پچھلا_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			عدد_contents);

	/* Drain cache again, forcing the matter */
	کرو {
		پچھلا_contents = عدد_contents;

		hlcache_clean(&اندراج_2); // Any non-NULL pointer will do

		برائے (عدد_contents = 0, اندراج_2 = hlcache->content_list;
				اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
			عدد_contents++;
		}
	} جب (عدد_contents > 0 && عدد_contents != پچھلا_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", عدد_contents);
	برائے (اندراج_2 = hlcache->content_list; اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
		hlcache_handle اندراج_handle = { اندراج_2, تہی, تہی };

		اگر (اندراج_2->content != تہی) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      اندراج_2,
			      nsurl_access(hlcache_handle_get_url(&اندراج_handle)),
			      content_گنتی_users(اندراج_2->content));
		} ورنہ {
			NSLOG(netsurf, INFO, "	%p", اندراج_2);
		}
	}

	/* Clean up retrieval contexts */
	اگر (hlcache->retrieval_ctx_ring != تہی) {
		ctx = hlcache->retrieval_ctx_ring;

		کرو {
			اگلا = ctx->r_اگلا;

			اگر (ctx->llcache != تہی)
				llcache_handle_release(ctx->llcache);

			اگر (ctx->handle != تہی)
				آزاد(ctx->handle);

			اگر (ctx->بچہ.charset != تہی)
				آزاد((حرف *) ctx->بچہ.charset);

			آزاد(ctx);

			ctx = اگلا;
		} جب (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = تہی;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->کامیابی_گنتی,
	      hlcache->ناکامی_گنتی);

	/* De-schedule ourselves */
	guit->misc->جدول_وقت(-1, hlcache_clean, تہی);

	آزاد(hlcache);
	hlcache = تہی;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_ڈیٹا *post,
			hlcache_handle_callback cb, خالی *pw,
			hlcache_بچہ_context *بچہ,
			content_type accepted_types,
			hlcache_handle **نتیجہ)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	أكد(cb != تہی);

	ctx = صفر_حافظہ_دو(1, حجم(hlcache_retrieval_ctx));
	اگر (ctx == تہی) {
		واپس NSERROR_NOMEM;
	}

	ctx->handle = صفر_حافظہ_دو(1, حجم(hlcache_handle));
	اگر (ctx->handle == تہی) {
		آزاد(ctx);
		واپس NSERROR_NOMEM;
	}

	اگر (بچہ != تہی) {
		اگر (بچہ->charset != تہی) {
			ctx->بچہ.charset = strdup(بچہ->charset);
			اگر (ctx->بچہ.charset == تہی) {
				آزاد(ctx->handle);
				آزاد(ctx);
				واپس NSERROR_NOMEM;
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
	اگر (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		آزاد((حرف *) ctx->بچہ.charset);
		آزاد(ctx->handle);
		آزاد(ctx);
	} ورنہ {
		/* successfully started fetch so add new context to list */
		RING_داخل_کرو(hlcache->retrieval_ctx_ring, ctx);

		*نتیجہ = ctx->handle;
	}
	واپس error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	اگر (handle->اندراج_2 != تہی) {
		content_remove_user(handle->اندراج_2->content,
				hlcache_content_callback, handle);
	} ورنہ {
		RING_ITERATE_آغاز(ساخت hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle &&
					ictx->migrate_ہدف == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				آزاد((حرف *) ictx->بچہ.charset);
				آزاد(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_انتہا(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = تہی;
	handle->pw = تہی;

	آزاد(handle);

	واپس NSERROR_OK;
}

/* See hlcache.h for documentation */
ساخت content *hlcache_handle_get_content(ثابت hlcache_handle *handle)
{
	اگر ((handle != تہی) && (handle->اندراج_2 != تہی)) {
		واپس handle->اندراج_2->content;
	}

	واپس تہی;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ساخت hlcache_اندراج *اندراج_2 = handle->اندراج_2;
	ساخت content *c;

	اگر (اندراج_2 == تہی) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_آغاز(ساخت hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle &&
					ictx->migrate_ہدف == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				آزاد((حرف *) ictx->بچہ.charset);
				آزاد(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_انتہا(hlcache->retrieval_ctx_ring, ictx);

		واپس NSERROR_OK;
	}

	c = اندراج_2->content;

	اگر (content_گنتی_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ساخت content *clone = content_clone(c);

		اگر (clone == تہی)
			واپس NSERROR_NOMEM;

		اندراج_2 = صفر_حافظہ_دو(1, حجم(ساخت hlcache_اندراج));

		اگر (اندراج_2 == تہی) {
			content_destroy(clone);
			واپس NSERROR_NOMEM;
		}

		اگر (content_جوڑو_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			آزاد(اندراج_2);
			واپس NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		اندراج_2->content = clone;
		handle->اندراج_2 = اندراج_2;
		اندراج_2->پچھلا = تہی;
		اندراج_2->اگلا = hlcache->content_list;
		اگر (hlcache->content_list != تہی)
			hlcache->content_list->پچھلا = اندراج_2;
		hlcache->content_list = اندراج_2;

		c = clone;
	}

	واپس content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, خالی *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	واپس NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **نتیجہ)
{
	*نتیجہ = تہی;
	واپس NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ثابت hlcache_handle *handle)
{
	nsurl *نتیجہ = تہی;

	أكد(handle != تہی);

	اگر (handle->اندراج_2 != تہی) {
		نتیجہ = content_get_url(handle->اندراج_2->content);
	} ورنہ {
		RING_ITERATE_آغاز(ساخت hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle) {
				/* This is the nascent context for us */
				نتیجہ = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_انتہا(hlcache->retrieval_ctx_ring, ictx);
	}

	واپس نتیجہ;
}
