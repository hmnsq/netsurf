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

প্রকারনির্দেশ সংরচনা hlcache_entry hlcache_entry;
প্রকারনির্দেশ সংরচনা hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
সংরচনা hlcache_retrieval_ctx {
	সংরচনা hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	সংরচনা hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
সংরচনা hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	শূন্য *pw;			/**< Client data */
};

/** Entry in high-level cache */
সংরচনা hlcache_entry {
	সংরচনা content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
সংরচনা hlcache_s {
	সংরচনা hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned পূর্ণাঙ্ক hit_গুনতি;
	unsigned পূর্ণাঙ্ক miss_গুনতি;
};

/** high level cache state */
স্থির সংরচনা hlcache_s *hlcache = রিক্ত;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
স্থির শূন্য hlcache_clean(শূন্য *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != রিক্ত);

	প্রতি (entry = hlcache->content_list; entry != রিক্ত; entry = next) {
		next = entry->next;

		যদি (entry->content == রিক্ত)
			অনুবর্ত;

		যদি (content_গুনতি_users(entry->content) != 0)
			অনুবর্ত;

		যদি (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			যদি (force_clean == false)
				অনুবর্ত;
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
		যদি (entry->prev == রিক্ত)
			hlcache->content_list = entry->next;
		অন্যথা
			entry->prev->next = entry->next;

		যদি (entry->next != রিক্ত)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		মুক্ত(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, রিক্ত);
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

	প্রত্যাগম ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
স্থির শূন্য hlcache_content_callback(সংরচনা content *c, content_বার্তা বার্তা,
		ধ্রুব সঙ্ঘ content_বার্তা_data *data, শূন্য *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = বার্তা,
	};

	যদি (data != রিক্ত) {
		event.data = *data;
	}

	যদি (handle->cb != রিক্ত)
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
	প্রতি (entry = hlcache->content_list; entry != রিক্ত; entry = entry->next) {
		hlcache_handle entry_handle = { entry, রিক্ত, রিক্ত };
		ধ্রুব llcache_handle *entry_llcache;

		যদি (entry->content == রিক্ত)
			অনুবর্ত;

		/* Ignore contents in the error state */
		যদি (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			অনুবর্ত;

		/* Ensure that content is shareable */
		যদি (content_is_shareable(entry->content) == false)
			অনুবর্ত;

		/* Ensure that quirks mode is acceptable */
		যদি (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			অনুবর্ত;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		যদি (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			ভঙ্গ;
	}

	যদি (entry == রিক্ত) {
		/* No existing entry, so need to create one */
		entry = malloc(পরিমাণ(hlcache_entry));
		যদি (entry == রিক্ত)
			প্রত্যাগম NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		যদি (entry->content == রিক্ত) {
			মুক্ত(entry);
			প্রত্যাগম NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = রিক্ত;
		entry->next = hlcache->content_list;
		যদি (hlcache->content_list != রিক্ত)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_গুনতি++;
	} অন্যথা {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_গুনতি++;
	}

	/* Associate handle with content */
	যদি (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		প্রত্যাগম NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	যদি (ctx->handle->cb != রিক্ত) {
		content_status status = content_get_status(ctx->handle);

		যদি (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} অন্যথা যদি (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			যদি (ctx->handle->cb != রিক্ত) {
				event.type = CONTENT_বার্তা_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} অন্যথা যদি (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_বার্তা_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			যদি (ctx->handle->cb != রিক্ত) {
				event.type = CONTENT_বার্তা_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			যদি (ctx->handle->cb != রিক্ত) {
				event.type = CONTENT_বার্তা_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	প্রত্যাগম error;
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

	যদি ((effective_type != রিক্ত) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		যদি (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			যদি (ctx->handle->cb != রিক্ত) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_বার্তা_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

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

		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} অন্যথা {
		/* Unacceptable type: report error */
		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
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
	মুক্ত((বর্ণ *) ctx->child.charset);
	মুক্ত(ctx);

	প্রত্যাগম error;
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
			 ধ্রুব llcache_event *event,
			 শূন্য *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = রিক্ত;
	nserror error;

	প্রতিজ্ঞা(ctx->llcache == handle);

	বিকল্প (event->type) {
	পক্ষ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), রিক্ত, 0,
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
		যদি (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		প্রত্যাগম error;

		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		যদি (error != NSERROR_OK) {
			প্রতিজ্ঞা(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		প্রত্যাগম error;

		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				রিক্ত, 0, false, false, &effective_type);
		যদি (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			প্রত্যাগম error;
		}

		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = রিক্ত;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_ERROR:
		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.বার্তা;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_PROGRESS:
		ভঙ্গ;
	পক্ষ LLCACHE_EVENT_REDIRECT:
		যদি (ctx->handle->cb != রিক্ত) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_বার্তা_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ভঙ্গ;
	}

	প্রত্যাগম NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ধ্রুব সংরচনা hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, পরিমাণ(সংরচনা hlcache_s));
	যদি (hlcache == রিক্ত) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	যদি (ret != NSERROR_OK) {
		মুক্ত(hlcache);
		hlcache = রিক্ত;
		প্রত্যাগম ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, রিক্ত);

	প্রত্যাগম NSERROR_OK;
}

/* See hlcache.h for documentation */
শূন্য hlcache_stop(শূন্য)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, রিক্ত);
}

/* See hlcache.h for documentation */
শূন্য hlcache_finalise(শূন্য)
{
	uint32_t অঙ্ক_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	প্রতি (অঙ্ক_contents = 0, entry = hlcache->content_list;
			entry != রিক্ত; entry = entry->next) {
		অঙ্ক_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      অঙ্ক_contents);

	/* Drain cache */
	কুরু {
		prev_contents = অঙ্ক_contents;

		hlcache_clean(রিক্ত);

		প্রতি (অঙ্ক_contents = 0, entry = hlcache->content_list;
				entry != রিক্ত; entry = entry->next) {
			অঙ্ক_contents++;
		}
	} যাবত্ (অঙ্ক_contents > 0 && অঙ্ক_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			অঙ্ক_contents);

	/* Drain cache again, forcing the matter */
	কুরু {
		prev_contents = অঙ্ক_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		প্রতি (অঙ্ক_contents = 0, entry = hlcache->content_list;
				entry != রিক্ত; entry = entry->next) {
			অঙ্ক_contents++;
		}
	} যাবত্ (অঙ্ক_contents > 0 && অঙ্ক_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", অঙ্ক_contents);
	প্রতি (entry = hlcache->content_list; entry != রিক্ত; entry = entry->next) {
		hlcache_handle entry_handle = { entry, রিক্ত, রিক্ত };

		যদি (entry->content != রিক্ত) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_গুনতি_users(entry->content));
		} অন্যথা {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	যদি (hlcache->retrieval_ctx_ring != রিক্ত) {
		ctx = hlcache->retrieval_ctx_ring;

		কুরু {
			next = ctx->r_next;

			যদি (ctx->llcache != রিক্ত)
				llcache_handle_release(ctx->llcache);

			যদি (ctx->handle != রিক্ত)
				মুক্ত(ctx->handle);

			যদি (ctx->child.charset != রিক্ত)
				মুক্ত((বর্ণ *) ctx->child.charset);

			মুক্ত(ctx);

			ctx = next;
		} যাবত্ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = রিক্ত;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_গুনতি,
	      hlcache->miss_গুনতি);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, রিক্ত);

	মুক্ত(hlcache);
	hlcache = রিক্ত;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, শূন্য *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	প্রতিজ্ঞা(cb != রিক্ত);

	ctx = calloc(1, পরিমাণ(hlcache_retrieval_ctx));
	যদি (ctx == রিক্ত) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, পরিমাণ(hlcache_handle));
	যদি (ctx->handle == রিক্ত) {
		মুক্ত(ctx);
		প্রত্যাগম NSERROR_NOMEM;
	}

	যদি (child != রিক্ত) {
		যদি (child->charset != রিক্ত) {
			ctx->child.charset = strdup(child->charset);
			যদি (ctx->child.charset == রিক্ত) {
				মুক্ত(ctx->handle);
				মুক্ত(ctx);
				প্রত্যাগম NSERROR_NOMEM;
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
		মুক্ত((বর্ণ *) ctx->child.charset);
		মুক্ত(ctx->handle);
		মুক্ত(ctx);
	} অন্যথা {
		/* successfully started fetch so add new context to list */
		RING_ঢোকাও(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	প্রত্যাগম error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	যদি (handle->entry != রিক্ত) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} অন্যথা {
		RING_ITERATE_START(সংরচনা hlcache_retrieval_ctx,
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
				মুক্ত((বর্ণ *) ictx->child.charset);
				মুক্ত(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = রিক্ত;
	handle->pw = রিক্ত;

	মুক্ত(handle);

	প্রত্যাগম NSERROR_OK;
}

/* See hlcache.h for documentation */
সংরচনা content *hlcache_handle_get_content(ধ্রুব hlcache_handle *handle)
{
	যদি ((handle != রিক্ত) && (handle->entry != রিক্ত)) {
		প্রত্যাগম handle->entry->content;
	}

	প্রত্যাগম রিক্ত;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	সংরচনা hlcache_entry *entry = handle->entry;
	সংরচনা content *c;

	যদি (entry == রিক্ত) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(সংরচনা hlcache_retrieval_ctx,
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
				মুক্ত((বর্ণ *) ictx->child.charset);
				মুক্ত(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		প্রত্যাগম NSERROR_OK;
	}

	c = entry->content;

	যদি (content_গুনতি_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		সংরচনা content *clone = content_clone(c);

		যদি (clone == রিক্ত)
			প্রত্যাগম NSERROR_NOMEM;

		entry = calloc(1, পরিমাণ(সংরচনা hlcache_entry));

		যদি (entry == রিক্ত) {
			content_destroy(clone);
			প্রত্যাগম NSERROR_NOMEM;
		}

		যদি (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			মুক্ত(entry);
			প্রত্যাগম NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = রিক্ত;
		entry->next = hlcache->content_list;
		যদি (hlcache->content_list != রিক্ত)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	প্রত্যাগম content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, শূন্য *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	প্রত্যাগম NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = রিক্ত;
	প্রত্যাগম NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ধ্রুব hlcache_handle *handle)
{
	nsurl *result = রিক্ত;

	প্রতিজ্ঞা(handle != রিক্ত);

	যদি (handle->entry != রিক্ত) {
		result = content_get_url(handle->entry->content);
	} অন্যথা {
		RING_ITERATE_START(সংরচনা hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			যদি (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	প্রত্যাগম result;
}
