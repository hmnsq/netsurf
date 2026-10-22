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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils/http.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/ring.h"
#include "utils/utils.h"
#include "netsurf/inttypes.h"
#include "netsurf/misc.h"
#include "netsurf/content.h"
#include "desktop/gui_internal.h"

#include "content/mimesniff.h"
#include "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#include "content/content_protected.h"
#include "content/content_factory.h"

typedef struct hlcache_entry hlcache_entry;
typedef struct hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
struct hlcache_retrieval_ctx {
	struct hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	struct hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
struct hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	void *pw;			/**< Client data */
};

/** Entry in high-level cache */
struct hlcache_entry {
	struct content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
struct hlcache_s {
	struct hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned số_nguyên hit_count;
	unsigned số_nguyên miss_count;
};

/** high level cache state */
static struct hlcache_s *hlcache = NULL;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
static void hlcache_clean(void *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != NULL);

	cho (entry = hlcache->content_list; entry != NULL; entry = next) {
		next = entry->next;

		nếu (entry->content == NULL)
			continue;

		nếu (content_count_users(entry->content) != 0)
			continue;

		nếu (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			nếu (force_clean == false)
				continue;
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
		nếu (entry->prev == NULL)
			hlcache->content_list = entry->next;
		không_thì
			entry->prev->next = entry->next;

		nếu (entry->next != NULL)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		free(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, NULL);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
static bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	trả_về ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
static void hlcache_content_callback(struct content *c, content_msg msg,
		const union content_msg_data *data, void *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = msg,
	};

	nếu (data != NULL) {
		event.data = *data;
	}

	nếu (handle->cb != NULL)
		error = handle->cb(handle, &event, handle->pw);

	nếu (error != NSERROR_OK)
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
static nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	cho (entry = hlcache->content_list; entry != NULL; entry = entry->next) {
		hlcache_handle entry_handle = { entry, NULL, NULL };
		const llcache_handle *entry_llcache;

		nếu (entry->content == NULL)
			continue;

		/* Ignore contents in the error state */
		nếu (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			continue;

		/* Ensure that content is shareable */
		nếu (content_is_shareable(entry->content) == false)
			continue;

		/* Ensure that quirks mode is acceptable */
		nếu (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			continue;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		nếu (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			break;
	}

	nếu (entry == NULL) {
		/* No existing entry, so need to create one */
		entry = malloc(sizeof(hlcache_entry));
		nếu (entry == NULL)
			trả_về NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		nếu (entry->content == NULL) {
			free(entry);
			trả_về NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = NULL;
		entry->next = hlcache->content_list;
		nếu (hlcache->content_list != NULL)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_count++;
	} không_thì {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_count++;
	}

	/* Associate handle with content */
	nếu (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		trả_về NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	nếu (ctx->handle->cb != NULL) {
		content_status status = content_get_status(ctx->handle);

		nếu (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} không_thì nếu (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			nếu (ctx->handle->cb != NULL) {
				event.type = CONTENT_MSG_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} không_thì nếu (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_MSG_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			nếu (ctx->handle->cb != NULL) {
				event.type = CONTENT_MSG_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			nếu (ctx->handle->cb != NULL) {
				event.type = CONTENT_MSG_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	trả_về error;
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
static nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	nếu ((effective_type != NULL) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		nếu (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			nếu (ctx->handle->cb != NULL) {
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
	} không_thì nếu (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		nếu (ctx->handle->cb != NULL) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} không_thì {
		/* Unacceptable type: report error */
		nếu (ctx->handle->cb != NULL) {
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
	free((char *) ctx->child.charset);
	free(ctx);

	trả_về error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
static nserror
hlcache_llcache_callback(llcache_handle *handle,
			 const llcache_event *event,
			 void *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = NULL;
	nserror error;

	assert(ctx->llcache == handle);

	switch (event->type) {
	case LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		nếu (ctx->handle->cb != NULL) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		break;
	case LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), NULL, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		nếu (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		nếu (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		trả_về error;

		break;
	case LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		nếu (error != NSERROR_OK) {
			assert(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		trả_về error;

		break;
	case LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				NULL, 0, false, false, &effective_type);
		nếu (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			trả_về error;
		}

		nếu (ctx->handle->cb != NULL) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = NULL;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		break;
	case LLCACHE_EVENT_ERROR:
		nếu (ctx->handle->cb != NULL) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.msg;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		break;
	case LLCACHE_EVENT_PROGRESS:
		break;
	case LLCACHE_EVENT_REDIRECT:
		nếu (ctx->handle->cb != NULL) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_MSG_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		break;
	}

	trả_về NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(const struct hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, sizeof(struct hlcache_s));
	nếu (hlcache == NULL) {
		trả_về NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	nếu (ret != NSERROR_OK) {
		free(hlcache);
		hlcache = NULL;
		trả_về ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, NULL);

	trả_về NSERROR_OK;
}

/* See hlcache.h for documentation */
void hlcache_stop(void)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, NULL);
}

/* See hlcache.h for documentation */
void hlcache_finalise(void)
{
	uint32_t num_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	cho (num_contents = 0, entry = hlcache->content_list;
			entry != NULL; entry = entry->next) {
		num_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      num_contents);

	/* Drain cache */
	do {
		prev_contents = num_contents;

		hlcache_clean(NULL);

		cho (num_contents = 0, entry = hlcache->content_list;
				entry != NULL; entry = entry->next) {
			num_contents++;
		}
	} trong_khi (num_contents > 0 && num_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			num_contents);

	/* Drain cache again, forcing the matter */
	do {
		prev_contents = num_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		cho (num_contents = 0, entry = hlcache->content_list;
				entry != NULL; entry = entry->next) {
			num_contents++;
		}
	} trong_khi (num_contents > 0 && num_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", num_contents);
	cho (entry = hlcache->content_list; entry != NULL; entry = entry->next) {
		hlcache_handle entry_handle = { entry, NULL, NULL };

		nếu (entry->content != NULL) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_count_users(entry->content));
		} không_thì {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	nếu (hlcache->retrieval_ctx_ring != NULL) {
		ctx = hlcache->retrieval_ctx_ring;

		do {
			next = ctx->r_next;

			nếu (ctx->llcache != NULL)
				llcache_handle_release(ctx->llcache);

			nếu (ctx->handle != NULL)
				free(ctx->handle);

			nếu (ctx->child.charset != NULL)
				free((char *) ctx->child.charset);

			free(ctx);

			ctx = next;
		} trong_khi (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = NULL;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_count,
	      hlcache->miss_count);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, NULL);

	free(hlcache);
	hlcache = NULL;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, void *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	assert(cb != NULL);

	ctx = calloc(1, sizeof(hlcache_retrieval_ctx));
	nếu (ctx == NULL) {
		trả_về NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, sizeof(hlcache_handle));
	nếu (ctx->handle == NULL) {
		free(ctx);
		trả_về NSERROR_NOMEM;
	}

	nếu (child != NULL) {
		nếu (child->charset != NULL) {
			ctx->child.charset = strdup(child->charset);
			nếu (ctx->child.charset == NULL) {
				free(ctx->handle);
				free(ctx);
				trả_về NSERROR_NOMEM;
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
	nếu (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		free((char *) ctx->child.charset);
		free(ctx->handle);
		free(ctx);
	} không_thì {
		/* successfully started fetch so add new context to list */
		RING_INSERT(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	trả_về error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	nếu (handle->entry != NULL) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} không_thì {
		RING_ITERATE_START(struct hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			nếu (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				free((char *) ictx->child.charset);
				free(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = NULL;
	handle->pw = NULL;

	free(handle);

	trả_về NSERROR_OK;
}

/* See hlcache.h for documentation */
struct content *hlcache_handle_get_content(const hlcache_handle *handle)
{
	nếu ((handle != NULL) && (handle->entry != NULL)) {
		trả_về handle->entry->content;
	}

	trả_về NULL;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	struct hlcache_entry *entry = handle->entry;
	struct content *c;

	nếu (entry == NULL) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(struct hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			nếu (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				free((char *) ictx->child.charset);
				free(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		trả_về NSERROR_OK;
	}

	c = entry->content;

	nếu (content_count_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		struct content *clone = content_clone(c);

		nếu (clone == NULL)
			trả_về NSERROR_NOMEM;

		entry = calloc(1, sizeof(struct hlcache_entry));

		nếu (entry == NULL) {
			content_destroy(clone);
			trả_về NSERROR_NOMEM;
		}

		nếu (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			free(entry);
			trả_về NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = NULL;
		entry->next = hlcache->content_list;
		nếu (hlcache->content_list != NULL)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	trả_về content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, void *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	trả_về NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = NULL;
	trả_về NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(const hlcache_handle *handle)
{
	nsurl *result = NULL;

	assert(handle != NULL);

	nếu (handle->entry != NULL) {
		result = content_get_url(handle->entry->content);
	} không_thì {
		RING_ITERATE_START(struct hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			nếu (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	trả_về result;
}
