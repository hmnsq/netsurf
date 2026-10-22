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

تعریفنوع ساختار hlcache_entry hlcache_entry;
تعریفنوع ساختار hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ساختار hlcache_retrieval_ctx {
	ساختار hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ساختار hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ساختار hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	تهی *pw;			/**< Client data */
};

/** Entry in high-level cache */
ساختار hlcache_entry {
	ساختار content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ساختار hlcache_s {
	ساختار hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned صحیح hit_شمار;
	unsigned صحیح miss_شمار;
};

/** high level cache state */
ایستا ساختار hlcache_s *hlcache = پوچ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ایستا تهی hlcache_clean(تهی *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != پوچ);

	برای (entry = hlcache->content_list; entry != پوچ; entry = next) {
		next = entry->next;

		اگر (entry->content == پوچ)
			ادامه;

		اگر (content_شمار_users(entry->content) != 0)
			ادامه;

		اگر (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			اگر (force_clean == false)
				ادامه;
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
		اگر (entry->prev == پوچ)
			hlcache->content_list = entry->next;
		وگرنه
			entry->prev->next = entry->next;

		اگر (entry->next != پوچ)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		آزاد(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, پوچ);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ایستا bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	بازگشت ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ایستا تهی hlcache_content_callback(ساختار content *c, content_پیام پیام,
		ثابت اجتماع content_پیام_data *data, تهی *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = پیام,
	};

	اگر (data != پوچ) {
		event.data = *data;
	}

	اگر (handle->cb != پوچ)
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
ایستا nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	برای (entry = hlcache->content_list; entry != پوچ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, پوچ, پوچ };
		ثابت llcache_handle *entry_llcache;

		اگر (entry->content == پوچ)
			ادامه;

		/* Ignore contents in the error state */
		اگر (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			ادامه;

		/* Ensure that content is shareable */
		اگر (content_is_shareable(entry->content) == false)
			ادامه;

		/* Ensure that quirks mode is acceptable */
		اگر (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			ادامه;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		اگر (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			بشکن;
	}

	اگر (entry == پوچ) {
		/* No existing entry, so need to create one */
		entry = malloc(اندازه(hlcache_entry));
		اگر (entry == پوچ)
			بازگشت NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		اگر (entry->content == پوچ) {
			آزاد(entry);
			بازگشت NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = پوچ;
		entry->next = hlcache->content_list;
		اگر (hlcache->content_list != پوچ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_شمار++;
	} وگرنه {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_شمار++;
	}

	/* Associate handle with content */
	اگر (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		بازگشت NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	اگر (ctx->handle->cb != پوچ) {
		content_status status = content_get_status(ctx->handle);

		اگر (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_پیام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} وگرنه اگر (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_پیام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			اگر (ctx->handle->cb != پوچ) {
				event.type = CONTENT_پیام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} وگرنه اگر (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_پیام_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			اگر (ctx->handle->cb != پوچ) {
				event.type = CONTENT_پیام_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			اگر (ctx->handle->cb != پوچ) {
				event.type = CONTENT_پیام_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	بازگشت error;
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
ایستا nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	اگر ((effective_type != پوچ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		اگر (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			اگر (ctx->handle->cb != پوچ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_پیام_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} وگرنه اگر (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} وگرنه {
		/* Unacceptable type: report error */
		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_ERROR;
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
	آزاد((نویسه *) ctx->child.charset);
	آزاد(ctx);

	بازگشت error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ایستا nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ثابت llcache_event *event,
			 تهی *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = پوچ;
	nserror error;

	أكد(ctx->llcache == handle);

	انتخاب (event->type) {
	حالت LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		بشکن;
	حالت LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), پوچ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		اگر (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		اگر (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		بازگشت error;

		بشکن;
	حالت LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		اگر (error != NSERROR_OK) {
			أكد(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		بازگشت error;

		بشکن;
	حالت LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				پوچ, 0, false, false, &effective_type);
		اگر (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			بازگشت error;
		}

		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = پوچ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		بشکن;
	حالت LLCACHE_EVENT_ERROR:
		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.پیام;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		بشکن;
	حالت LLCACHE_EVENT_PROGRESS:
		بشکن;
	حالت LLCACHE_EVENT_REDIRECT:
		اگر (ctx->handle->cb != پوچ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_پیام_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		بشکن;
	}

	بازگشت NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ثابت ساختار hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, اندازه(ساختار hlcache_s));
	اگر (hlcache == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	اگر (ret != NSERROR_OK) {
		آزاد(hlcache);
		hlcache = پوچ;
		بازگشت ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, پوچ);

	بازگشت NSERROR_OK;
}

/* See hlcache.h for documentation */
تهی hlcache_stop(تهی)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, پوچ);
}

/* See hlcache.h for documentation */
تهی hlcache_finalise(تهی)
{
	uint32_t رقم_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	برای (رقم_contents = 0, entry = hlcache->content_list;
			entry != پوچ; entry = entry->next) {
		رقم_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      رقم_contents);

	/* Drain cache */
	انجامبده {
		prev_contents = رقم_contents;

		hlcache_clean(پوچ);

		برای (رقم_contents = 0, entry = hlcache->content_list;
				entry != پوچ; entry = entry->next) {
			رقم_contents++;
		}
	} وقتی (رقم_contents > 0 && رقم_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			رقم_contents);

	/* Drain cache again, forcing the matter */
	انجامبده {
		prev_contents = رقم_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		برای (رقم_contents = 0, entry = hlcache->content_list;
				entry != پوچ; entry = entry->next) {
			رقم_contents++;
		}
	} وقتی (رقم_contents > 0 && رقم_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", رقم_contents);
	برای (entry = hlcache->content_list; entry != پوچ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, پوچ, پوچ };

		اگر (entry->content != پوچ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_شمار_users(entry->content));
		} وگرنه {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	اگر (hlcache->retrieval_ctx_ring != پوچ) {
		ctx = hlcache->retrieval_ctx_ring;

		انجامبده {
			next = ctx->r_next;

			اگر (ctx->llcache != پوچ)
				llcache_handle_release(ctx->llcache);

			اگر (ctx->handle != پوچ)
				آزاد(ctx->handle);

			اگر (ctx->child.charset != پوچ)
				آزاد((نویسه *) ctx->child.charset);

			آزاد(ctx);

			ctx = next;
		} وقتی (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = پوچ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_شمار,
	      hlcache->miss_شمار);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, پوچ);

	آزاد(hlcache);
	hlcache = پوچ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, تهی *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	أكد(cb != پوچ);

	ctx = calloc(1, اندازه(hlcache_retrieval_ctx));
	اگر (ctx == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, اندازه(hlcache_handle));
	اگر (ctx->handle == پوچ) {
		آزاد(ctx);
		بازگشت NSERROR_NOMEM;
	}

	اگر (child != پوچ) {
		اگر (child->charset != پوچ) {
			ctx->child.charset = strdup(child->charset);
			اگر (ctx->child.charset == پوچ) {
				آزاد(ctx->handle);
				آزاد(ctx);
				بازگشت NSERROR_NOMEM;
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
	اگر (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		آزاد((نویسه *) ctx->child.charset);
		آزاد(ctx->handle);
		آزاد(ctx);
	} وگرنه {
		/* successfully started fetch so add new context to list */
		RING_درج(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	بازگشت error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	اگر (handle->entry != پوچ) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} وگرنه {
		RING_ITERATE_START(ساختار hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				آزاد((نویسه *) ictx->child.charset);
				آزاد(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = پوچ;
	handle->pw = پوچ;

	آزاد(handle);

	بازگشت NSERROR_OK;
}

/* See hlcache.h for documentation */
ساختار content *hlcache_handle_get_content(ثابت hlcache_handle *handle)
{
	اگر ((handle != پوچ) && (handle->entry != پوچ)) {
		بازگشت handle->entry->content;
	}

	بازگشت پوچ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ساختار hlcache_entry *entry = handle->entry;
	ساختار content *c;

	اگر (entry == پوچ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(ساختار hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				آزاد((نویسه *) ictx->child.charset);
				آزاد(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		بازگشت NSERROR_OK;
	}

	c = entry->content;

	اگر (content_شمار_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ساختار content *clone = content_clone(c);

		اگر (clone == پوچ)
			بازگشت NSERROR_NOMEM;

		entry = calloc(1, اندازه(ساختار hlcache_entry));

		اگر (entry == پوچ) {
			content_destroy(clone);
			بازگشت NSERROR_NOMEM;
		}

		اگر (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			آزاد(entry);
			بازگشت NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = پوچ;
		entry->next = hlcache->content_list;
		اگر (hlcache->content_list != پوچ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	بازگشت content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, تهی *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	بازگشت NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = پوچ;
	بازگشت NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ثابت hlcache_handle *handle)
{
	nsurl *result = پوچ;

	أكد(handle != پوچ);

	اگر (handle->entry != پوچ) {
		result = content_get_url(handle->entry->content);
	} وگرنه {
		RING_ITERATE_START(ساختار hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			اگر (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	بازگشت result;
}
