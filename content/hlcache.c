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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/http.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/ring.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "content/mimesniff.h"
#অন্তর্ভাব "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"

প্রকারলেখ কাঠামো hlcache_entry hlcache_entry;
প্রকারলেখ কাঠামো hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
কাঠামো hlcache_retrieval_ctx {
	কাঠামো hlcache_retrieval_ctx *r_পূর্ব; /**< Previous retrieval context in the ring */
	কাঠামো hlcache_retrieval_ctx *r_পরবর্তী; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
কাঠামো hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	শূন্য *pw;			/**< Client data */
};

/** Entry in high-level cache */
কাঠামো hlcache_entry {
	কাঠামো content *content;	/**< Pointer to associated content */

	hlcache_entry *পরবর্তী;		/**< Next sibling */
	hlcache_entry *পূর্ব;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
কাঠামো hlcache_s {
	কাঠামো hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned পূর্ণসংখ্যা hit_গুনতি;
	unsigned পূর্ণসংখ্যা miss_গুনতি;
};

/** high level cache state */
স্থির কাঠামো hlcache_s *hlcache = ফাঁকা;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
স্থির শূন্য hlcache_clean(শূন্য *force_clean_flag)
{
	hlcache_entry *entry, *পরবর্তী;
	bool force_clean = (force_clean_flag != ফাঁকা);

	জন্য (entry = hlcache->content_list; entry != ফাঁকা; entry = পরবর্তী) {
		পরবর্তী = entry->পরবর্তী;

		যদি (entry->content == ফাঁকা)
			চালু;

		যদি (content_গুনতি_users(entry->content) != 0)
			চালু;

		যদি (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			যদি (force_clean == false)
				চালু;
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
		যদি (entry->পূর্ব == ফাঁকা)
			hlcache->content_list = entry->পরবর্তী;
		অন্যথা
			entry->পূর্ব->পরবর্তী = entry->পরবর্তী;

		যদি (entry->পরবর্তী != ফাঁকা)
			entry->পরবর্তী->পূর্ব = entry->পূর্ব;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		মুক্ত(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ফাঁকা);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
স্থির bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	ফেরত ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
স্থির শূন্য hlcache_content_callback(কাঠামো content *c, content_বার্তা বার্তা,
		ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য, শূন্য *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = বার্তা,
	};

	যদি (তথ্য != ফাঁকা) {
		event.তথ্য = *তথ্য;
	}

	যদি (handle->cb != ফাঁকা)
		error = handle->cb(handle, &event, handle->pw);

	যদি (error != NSERROR_OK)
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
স্থির nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	জন্য (entry = hlcache->content_list; entry != ফাঁকা; entry = entry->পরবর্তী) {
		hlcache_handle entry_handle = { entry, ফাঁকা, ফাঁকা };
		ধ্রুবক llcache_handle *entry_llcache;

		যদি (entry->content == ফাঁকা)
			চালু;

		/* Ignore contents in the error state */
		যদি (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			চালু;

		/* Ensure that content is shareable */
		যদি (content_is_shareable(entry->content) == false)
			চালু;

		/* Ensure that quirks mode is acceptable */
		যদি (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			চালু;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		যদি (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			বিরতি;
	}

	যদি (entry == ফাঁকা) {
		/* No existing entry, so need to create one */
		entry = malloc(আকার(hlcache_entry));
		যদি (entry == ফাঁকা)
			ফেরত NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		যদি (entry->content == ফাঁকা) {
			মুক্ত(entry);
			ফেরত NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->পূর্ব = ফাঁকা;
		entry->পরবর্তী = hlcache->content_list;
		যদি (hlcache->content_list != ফাঁকা)
			hlcache->content_list->পূর্ব = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_তথ্য;

		hlcache->miss_গুনতি++;
	} অন্যথা {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_গুনতি++;
	}

	/* Associate handle with content */
	যদি (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		ফেরত NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	যদি (ctx->handle->cb != ফাঁকা) {
		content_status status = content_get_status(ctx->handle);

		যদি (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} অন্যথা যদি (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			যদি (ctx->handle->cb != ফাঁকা) {
				event.type = CONTENT_বার্তা_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} অন্যথা যদি (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			যদি (ctx->handle->cb != ফাঁকা) {
				event.type = CONTENT_বার্তা_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			যদি (ctx->handle->cb != ফাঁকা) {
				event.type = CONTENT_বার্তা_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ফেরত error;
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
স্থির nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	যদি ((effective_type != ফাঁকা) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		যদি (error != NSERROR_OK && error != NSERROR_NEED_তথ্য) {
			যদি (ctx->handle->cb != ফাঁকা) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_বার্তা_ERROR;
				hlevent.তথ্য.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.তথ্য.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} অন্যথা যদি (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_DOWNLOAD;
			hlevent.তথ্য.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_তথ্য;
	} অন্যথা {
		/* Unacceptable type: report error */
		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
			hlevent.তথ্য.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.তথ্য.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_target = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	মুক্ত((অক্ষর *) ctx->child.charset);
	মুক্ত(ctx);

	ফেরত error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
স্থির nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ধ্রুবক llcache_event *event,
			 শূন্য *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ফাঁকা;
	nserror error;

	প্রতিজ্ঞা(ctx->llcache == handle);

	নির্বাচন (event->type) {
	ক্ষেত্র LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_SSL_CERTS;
			hlevent.তথ্য.chain = event->তথ্য.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), ফাঁকা, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		যদি (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		যদি (error == NSERROR_NEED_তথ্য)
			error = NSERROR_OK;

		ফেরত error;

		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_HAD_তথ্য:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->তথ্য.তথ্য.buf, event->তথ্য.তথ্য.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		যদি (error != NSERROR_OK) {
			প্রতিজ্ঞা(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ফেরত error;

		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				ফাঁকা, 0, false, false, &effective_type);
		যদি (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ফেরত error;
		}

		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
			hlevent.তথ্য.errordata.errorcode = error;
			hlevent.তথ্য.errordata.errormsg = ফাঁকা;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_ERROR:
		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
			hlevent.তথ্য.errordata.errorcode = event->তথ্য.error.code;
			hlevent.তথ্য.errordata.errormsg = event->তথ্য.error.বার্তা;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_PROGRESS:
		বিরতি;
	ক্ষেত্র LLCACHE_EVENT_REDIRECT:
		যদি (ctx->handle->cb != ফাঁকা) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_REDIRECT;
			hlevent.তথ্য.redirect.from = event->তথ্য.redirect.from;
			hlevent.তথ্য.redirect.to = event->তথ্য.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		বিরতি;
	}

	ফেরত NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ধ্রুবক কাঠামো hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, আকার(কাঠামো hlcache_s));
	যদি (hlcache == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	যদি (ret != NSERROR_OK) {
		মুক্ত(hlcache);
		hlcache = ফাঁকা;
		ফেরত ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ফাঁকা);

	ফেরত NSERROR_OK;
}

/* See hlcache.h for documentation */
শূন্য hlcache_stop(শূন্য)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, ফাঁকা);
}

/* See hlcache.h for documentation */
শূন্য hlcache_finalise(শূন্য)
{
	uint32_t অঙ্ক_contents, পূর্ব_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *পরবর্তী;

	/* Obtain initial count of contents remaining */
	জন্য (অঙ্ক_contents = 0, entry = hlcache->content_list;
			entry != ফাঁকা; entry = entry->পরবর্তী) {
		অঙ্ক_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      অঙ্ক_contents);

	/* Drain cache */
	করো {
		পূর্ব_contents = অঙ্ক_contents;

		hlcache_clean(ফাঁকা);

		জন্য (অঙ্ক_contents = 0, entry = hlcache->content_list;
				entry != ফাঁকা; entry = entry->পরবর্তী) {
			অঙ্ক_contents++;
		}
	} যখন (অঙ্ক_contents > 0 && অঙ্ক_contents != পূর্ব_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			অঙ্ক_contents);

	/* Drain cache again, forcing the matter */
	করো {
		পূর্ব_contents = অঙ্ক_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		জন্য (অঙ্ক_contents = 0, entry = hlcache->content_list;
				entry != ফাঁকা; entry = entry->পরবর্তী) {
			অঙ্ক_contents++;
		}
	} যখন (অঙ্ক_contents > 0 && অঙ্ক_contents != পূর্ব_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", অঙ্ক_contents);
	জন্য (entry = hlcache->content_list; entry != ফাঁকা; entry = entry->পরবর্তী) {
		hlcache_handle entry_handle = { entry, ফাঁকা, ফাঁকা };

		যদি (entry->content != ফাঁকা) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_গুনতি_users(entry->content));
		} অন্যথা {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	যদি (hlcache->retrieval_ctx_ring != ফাঁকা) {
		ctx = hlcache->retrieval_ctx_ring;

		করো {
			পরবর্তী = ctx->r_পরবর্তী;

			যদি (ctx->llcache != ফাঁকা)
				llcache_handle_release(ctx->llcache);

			যদি (ctx->handle != ফাঁকা)
				মুক্ত(ctx->handle);

			যদি (ctx->child.charset != ফাঁকা)
				মুক্ত((অক্ষর *) ctx->child.charset);

			মুক্ত(ctx);

			ctx = পরবর্তী;
		} যখন (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ফাঁকা;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_গুনতি,
	      hlcache->miss_গুনতি);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, ফাঁকা);

	মুক্ত(hlcache);
	hlcache = ফাঁকা;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_তথ্য *post,
			hlcache_handle_callback cb, শূন্য *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **ফলাফল)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	প্রতিজ্ঞা(cb != ফাঁকা);

	ctx = calloc(1, আকার(hlcache_retrieval_ctx));
	যদি (ctx == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, আকার(hlcache_handle));
	যদি (ctx->handle == ফাঁকা) {
		মুক্ত(ctx);
		ফেরত NSERROR_NOMEM;
	}

	যদি (child != ফাঁকা) {
		যদি (child->charset != ফাঁকা) {
			ctx->child.charset = strdup(child->charset);
			যদি (ctx->child.charset == ফাঁকা) {
				মুক্ত(ctx->handle);
				মুক্ত(ctx);
				ফেরত NSERROR_NOMEM;
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
	যদি (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		মুক্ত((অক্ষর *) ctx->child.charset);
		মুক্ত(ctx->handle);
		মুক্ত(ctx);
	} অন্যথা {
		/* successfully started fetch so add new context to list */
		RING_ঢোকাও(hlcache->retrieval_ctx_ring, ctx);

		*ফলাফল = ctx->handle;
	}
	ফেরত error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	যদি (handle->entry != ফাঁকা) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} অন্যথা {
		RING_ITERATE_START(কাঠামো hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			যদি (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				মুক্ত((অক্ষর *) ictx->child.charset);
				মুক্ত(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ফাঁকা;
	handle->pw = ফাঁকা;

	মুক্ত(handle);

	ফেরত NSERROR_OK;
}

/* See hlcache.h for documentation */
কাঠামো content *hlcache_handle_get_content(ধ্রুবক hlcache_handle *handle)
{
	যদি ((handle != ফাঁকা) && (handle->entry != ফাঁকা)) {
		ফেরত handle->entry->content;
	}

	ফেরত ফাঁকা;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	কাঠামো hlcache_entry *entry = handle->entry;
	কাঠামো content *c;

	যদি (entry == ফাঁকা) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(কাঠামো hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			যদি (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				মুক্ত((অক্ষর *) ictx->child.charset);
				মুক্ত(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		ফেরত NSERROR_OK;
	}

	c = entry->content;

	যদি (content_গুনতি_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		কাঠামো content *clone = content_clone(c);

		যদি (clone == ফাঁকা)
			ফেরত NSERROR_NOMEM;

		entry = calloc(1, আকার(কাঠামো hlcache_entry));

		যদি (entry == ফাঁকা) {
			content_destroy(clone);
			ফেরত NSERROR_NOMEM;
		}

		যদি (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			মুক্ত(entry);
			ফেরত NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->পূর্ব = ফাঁকা;
		entry->পরবর্তী = hlcache->content_list;
		যদি (hlcache->content_list != ফাঁকা)
			hlcache->content_list->পূর্ব = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	ফেরত content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, শূন্য *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ফেরত NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **ফলাফল)
{
	*ফলাফল = ফাঁকা;
	ফেরত NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ধ্রুবক hlcache_handle *handle)
{
	nsurl *ফলাফল = ফাঁকা;

	প্রতিজ্ঞা(handle != ফাঁকা);

	যদি (handle->entry != ফাঁকা) {
		ফলাফল = content_get_url(handle->entry->content);
	} অন্যথা {
		RING_ITERATE_START(কাঠামো hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			যদি (ictx->handle == handle) {
				/* This is the nascent context for us */
				ফলাফল = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	ফেরত ফলাফল;
}
