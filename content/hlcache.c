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

تعريفنوع بنية hlcache_entry hlcache_entry;
تعريفنوع بنية hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
بنية hlcache_retrieval_ctx {
	بنية hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	بنية hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
بنية hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	فراغ *pw;			/**< Client data */
};

/** Entry in high-level cache */
بنية hlcache_entry {
	بنية content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
بنية hlcache_s {
	بنية hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned صحيح hit_count;
	unsigned صحيح miss_count;
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
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != عدم);

	لكل (entry = hlcache->content_list; entry != عدم; entry = next) {
		next = entry->next;

		ܐܢ (entry->content == عدم)
			استمر;

		ܐܢ (content_count_users(entry->content) != 0)
			استمر;

		ܐܢ (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			ܐܢ (force_clean == false)
				استمر;
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
		ܐܢ (entry->prev == عدم)
			hlcache->content_list = entry->next;
		ܐܚܪܢܐ
			entry->prev->next = entry->next;

		ܐܢ (entry->next != عدم)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		حرر(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, عدم);
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

	ܦܢܐ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ساكن فراغ hlcache_content_callback(بنية content *c, content_msg msg,
		ثابت اتحاد content_msg_data *data, فراغ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = msg,
	};

	ܐܢ (data != عدم) {
		event.data = *data;
	}

	ܐܢ (handle->cb != عدم)
		error = handle->cb(handle, &event, handle->pw);

	ܐܢ (error != NSERROR_OK)
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
ساكن nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	لكل (entry = hlcache->content_list; entry != عدم; entry = entry->next) {
		hlcache_handle entry_handle = { entry, عدم, عدم };
		ثابت llcache_handle *entry_llcache;

		ܐܢ (entry->content == عدم)
			استمر;

		/* Ignore contents in the error state */
		ܐܢ (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			استمر;

		/* Ensure that content is shareable */
		ܐܢ (content_is_shareable(entry->content) == false)
			استمر;

		/* Ensure that quirks mode is acceptable */
		ܐܢ (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			استمر;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		ܐܢ (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			اكسر;
	}

	ܐܢ (entry == عدم) {
		/* No existing entry, so need to create one */
		entry = malloc(حجم(hlcache_entry));
		ܐܢ (entry == عدم)
			ܦܢܐ NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		ܐܢ (entry->content == عدم) {
			حرر(entry);
			ܦܢܐ NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = عدم;
		entry->next = hlcache->content_list;
		ܐܢ (hlcache->content_list != عدم)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_count++;
	} ܐܚܪܢܐ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_count++;
	}

	/* Associate handle with content */
	ܐܢ (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		ܦܢܐ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	ܐܢ (ctx->handle->cb != عدم) {
		content_status status = content_get_status(ctx->handle);

		ܐܢ (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ܐܚܪܢܐ ܐܢ (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ܐܢ (ctx->handle->cb != عدم) {
				event.type = CONTENT_MSG_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ܐܚܪܢܐ ܐܢ (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ܐܢ (ctx->handle->cb != عدم) {
				event.type = CONTENT_MSG_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ܐܢ (ctx->handle->cb != عدم) {
				event.type = CONTENT_MSG_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ܦܢܐ error;
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

	ctx->migrate_target = true;

	ܐܢ ((effective_type != عدم) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		ܐܢ (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			ܐܢ (ctx->handle->cb != عدم) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_MSG_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ܐܚܪܢܐ ܐܢ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} ܐܚܪܢܐ {
		/* Unacceptable type: report error */
		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_ERROR;
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
	حرر((حرف *) ctx->child.charset);
	حرر(ctx);

	ܦܢܐ error;
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
		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), عدم, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ܐܢ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ܐܢ (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		ܦܢܐ error;

		اكسر;
	حالة LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ܐܢ (error != NSERROR_OK) {
			أكد(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ܦܢܐ error;

		اكسر;
	حالة LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				عدم, 0, false, false, &effective_type);
		ܐܢ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ܦܢܐ error;
		}

		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = عدم;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_ERROR:
		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.msg;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	حالة LLCACHE_EVENT_PROGRESS:
		اكسر;
	حالة LLCACHE_EVENT_REDIRECT:
		ܐܢ (ctx->handle->cb != عدم) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		اكسر;
	}

	ܦܢܐ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ثابت بنية hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, حجم(بنية hlcache_s));
	ܐܢ (hlcache == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ܐܢ (ret != NSERROR_OK) {
		حرر(hlcache);
		hlcache = عدم;
		ܦܢܐ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, عدم);

	ܦܢܐ NSERROR_OK;
}

/* See hlcache.h for documentation */
فراغ hlcache_stop(فراغ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, عدم);
}

/* See hlcache.h for documentation */
فراغ hlcache_finalise(فراغ)
{
	uint32_t num_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	لكل (num_contents = 0, entry = hlcache->content_list;
			entry != عدم; entry = entry->next) {
		num_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      num_contents);

	/* Drain cache */
	ܥܒܕ {
		prev_contents = num_contents;

		hlcache_clean(عدم);

		لكل (num_contents = 0, entry = hlcache->content_list;
				entry != عدم; entry = entry->next) {
			num_contents++;
		}
	} ܟܡܐ (num_contents > 0 && num_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			num_contents);

	/* Drain cache again, forcing the matter */
	ܥܒܕ {
		prev_contents = num_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		لكل (num_contents = 0, entry = hlcache->content_list;
				entry != عدم; entry = entry->next) {
			num_contents++;
		}
	} ܟܡܐ (num_contents > 0 && num_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", num_contents);
	لكل (entry = hlcache->content_list; entry != عدم; entry = entry->next) {
		hlcache_handle entry_handle = { entry, عدم, عدم };

		ܐܢ (entry->content != عدم) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_count_users(entry->content));
		} ܐܚܪܢܐ {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	ܐܢ (hlcache->retrieval_ctx_ring != عدم) {
		ctx = hlcache->retrieval_ctx_ring;

		ܥܒܕ {
			next = ctx->r_next;

			ܐܢ (ctx->llcache != عدم)
				llcache_handle_release(ctx->llcache);

			ܐܢ (ctx->handle != عدم)
				حرر(ctx->handle);

			ܐܢ (ctx->child.charset != عدم)
				حرر((حرف *) ctx->child.charset);

			حرر(ctx);

			ctx = next;
		} ܟܡܐ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = عدم;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_count,
	      hlcache->miss_count);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, عدم);

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
			llcache_post_data *post,
			hlcache_handle_callback cb, فراغ *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	أكد(cb != عدم);

	ctx = calloc(1, حجم(hlcache_retrieval_ctx));
	ܐܢ (ctx == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, حجم(hlcache_handle));
	ܐܢ (ctx->handle == عدم) {
		حرر(ctx);
		ܦܢܐ NSERROR_NOMEM;
	}

	ܐܢ (child != عدم) {
		ܐܢ (child->charset != عدم) {
			ctx->child.charset = strdup(child->charset);
			ܐܢ (ctx->child.charset == عدم) {
				حرر(ctx->handle);
				حرر(ctx);
				ܦܢܐ NSERROR_NOMEM;
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
	ܐܢ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		حرر((حرف *) ctx->child.charset);
		حرر(ctx->handle);
		حرر(ctx);
	} ܐܚܪܢܐ {
		/* successfully started fetch so add new context to list */
		RING_INSERT(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	ܦܢܐ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ܐܢ (handle->entry != عدم) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} ܐܚܪܢܐ {
		RING_ITERATE_START(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ܐܢ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->child.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = عدم;
	handle->pw = عدم;

	حرر(handle);

	ܦܢܐ NSERROR_OK;
}

/* See hlcache.h for documentation */
بنية content *hlcache_handle_get_content(ثابت hlcache_handle *handle)
{
	ܐܢ ((handle != عدم) && (handle->entry != عدم)) {
		ܦܢܐ handle->entry->content;
	}

	ܦܢܐ عدم;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	بنية hlcache_entry *entry = handle->entry;
	بنية content *c;

	ܐܢ (entry == عدم) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ܐܢ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				حرر((حرف *) ictx->child.charset);
				حرر(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		ܦܢܐ NSERROR_OK;
	}

	c = entry->content;

	ܐܢ (content_count_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		بنية content *clone = content_clone(c);

		ܐܢ (clone == عدم)
			ܦܢܐ NSERROR_NOMEM;

		entry = calloc(1, حجم(بنية hlcache_entry));

		ܐܢ (entry == عدم) {
			content_destroy(clone);
			ܦܢܐ NSERROR_NOMEM;
		}

		ܐܢ (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			حرر(entry);
			ܦܢܐ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = عدم;
		entry->next = hlcache->content_list;
		ܐܢ (hlcache->content_list != عدم)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	ܦܢܐ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, فراغ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ܦܢܐ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = عدم;
	ܦܢܐ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ثابت hlcache_handle *handle)
{
	nsurl *result = عدم;

	أكد(handle != عدم);

	ܐܢ (handle->entry != عدم) {
		result = content_get_url(handle->entry->content);
	} ܐܚܪܢܐ {
		RING_ITERATE_START(بنية hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ܐܢ (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	ܦܢܐ result;
}
