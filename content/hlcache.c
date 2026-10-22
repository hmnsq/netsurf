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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/ring.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "content/mimesniff.h"
#അന്തര്ഭാവ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"

തരനിർവചനം ഘടന hlcache_entry hlcache_entry;
തരനിർവചനം ഘടന hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ഘടന hlcache_retrieval_ctx {
	ഘടന hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ഘടന hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ഘടന hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	ശൂന്യം *pw;			/**< Client data */
};

/** Entry in high-level cache */
ഘടന hlcache_entry {
	ഘടന content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ഘടന hlcache_s {
	ഘടന hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned പൂർണ്ണസംഖ്യ hit_എണ്ണം;
	unsigned പൂർണ്ണസംഖ്യ miss_എണ്ണം;
};

/** high level cache state */
സ്ഥിതം ഘടന hlcache_s *hlcache = ശൂന്യ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
സ്ഥിതം ശൂന്യം hlcache_clean(ശൂന്യം *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != ശൂന്യ);

	വേണ്ടി (entry = hlcache->content_list; entry != ശൂന്യ; entry = next) {
		next = entry->next;

		എങ്കിൽ (entry->content == ശൂന്യ)
			തുടരുക;

		എങ്കിൽ (content_എണ്ണം_users(entry->content) != 0)
			തുടരുക;

		എങ്കിൽ (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			എങ്കിൽ (force_clean == false)
				തുടരുക;
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
		എങ്കിൽ (entry->prev == ശൂന്യ)
			hlcache->content_list = entry->next;
		അല്ലെങ്കിൽ
			entry->prev->next = entry->next;

		എങ്കിൽ (entry->next != ശൂന്യ)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		സ്വതന്ത്രം(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ശൂന്യ);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
സ്ഥിതം bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	തിരികെ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
സ്ഥിതം ശൂന്യം hlcache_content_callback(ഘടന content *c, content_സന്ദേശം സന്ദേശം,
		സ്ഥിരം സംഘം content_സന്ദേശം_data *data, ശൂന്യം *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = സന്ദേശം,
	};

	എങ്കിൽ (data != ശൂന്യ) {
		event.data = *data;
	}

	എങ്കിൽ (handle->cb != ശൂന്യ)
		error = handle->cb(handle, &event, handle->pw);

	എങ്കിൽ (error != NSERROR_OK)
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
സ്ഥിതം nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	വേണ്ടി (entry = hlcache->content_list; entry != ശൂന്യ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ശൂന്യ, ശൂന്യ };
		സ്ഥിരം llcache_handle *entry_llcache;

		എങ്കിൽ (entry->content == ശൂന്യ)
			തുടരുക;

		/* Ignore contents in the error state */
		എങ്കിൽ (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			തുടരുക;

		/* Ensure that content is shareable */
		എങ്കിൽ (content_is_shareable(entry->content) == false)
			തുടരുക;

		/* Ensure that quirks mode is acceptable */
		എങ്കിൽ (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			തുടരുക;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		എങ്കിൽ (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			നിർത്തുക;
	}

	എങ്കിൽ (entry == ശൂന്യ) {
		/* No existing entry, so need to create one */
		entry = malloc(വലുപ്പം(hlcache_entry));
		എങ്കിൽ (entry == ശൂന്യ)
			തിരികെ NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		എങ്കിൽ (entry->content == ശൂന്യ) {
			സ്വതന്ത്രം(entry);
			തിരികെ NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = ശൂന്യ;
		entry->next = hlcache->content_list;
		എങ്കിൽ (hlcache->content_list != ശൂന്യ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_എണ്ണം++;
	} അല്ലെങ്കിൽ {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_എണ്ണം++;
	}

	/* Associate handle with content */
	എങ്കിൽ (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		തിരികെ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
		content_status status = content_get_status(ctx->handle);

		എങ്കിൽ (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_സന്ദേശം_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} അല്ലെങ്കിൽ എങ്കിൽ (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_സന്ദേശം_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
				event.type = CONTENT_സന്ദേശം_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_സന്ദേശം_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
				event.type = CONTENT_സന്ദേശം_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
				event.type = CONTENT_സന്ദേശം_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	തിരികെ error;
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
സ്ഥിതം nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	എങ്കിൽ ((effective_type != ശൂന്യ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		എങ്കിൽ (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_സന്ദേശം_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} അല്ലെങ്കിൽ {
		/* Unacceptable type: report error */
		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_ERROR;
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
	സ്വതന്ത്രം((അക്ഷരം *) ctx->child.charset);
	സ്വതന്ത്രം(ctx);

	തിരികെ error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
സ്ഥിതം nserror
hlcache_llcache_callback(llcache_handle *handle,
			 സ്ഥിരം llcache_event *event,
			 ശൂന്യം *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ശൂന്യ;
	nserror error;

	പ്രതിജ്ഞാ(ctx->llcache == handle);

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), ശൂന്യ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		എങ്കിൽ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		എങ്കിൽ (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		തിരികെ error;

		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		എങ്കിൽ (error != NSERROR_OK) {
			പ്രതിജ്ഞാ(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		തിരികെ error;

		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				ശൂന്യ, 0, false, false, &effective_type);
		എങ്കിൽ (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			തിരികെ error;
		}

		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = ശൂന്യ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_ERROR:
		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.സന്ദേശം;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_PROGRESS:
		നിർത്തുക;
	അവസ്ഥ LLCACHE_EVENT_REDIRECT:
		എങ്കിൽ (ctx->handle->cb != ശൂന്യ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_സന്ദേശം_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		നിർത്തുക;
	}

	തിരികെ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(സ്ഥിരം ഘടന hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, വലുപ്പം(ഘടന hlcache_s));
	എങ്കിൽ (hlcache == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	എങ്കിൽ (ret != NSERROR_OK) {
		സ്വതന്ത്രം(hlcache);
		hlcache = ശൂന്യ;
		തിരികെ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ശൂന്യ);

	തിരികെ NSERROR_OK;
}

/* See hlcache.h for documentation */
ശൂന്യം hlcache_stop(ശൂന്യം)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, ശൂന്യ);
}

/* See hlcache.h for documentation */
ശൂന്യം hlcache_finalise(ശൂന്യം)
{
	uint32_t അക്കം_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	വേണ്ടി (അക്കം_contents = 0, entry = hlcache->content_list;
			entry != ശൂന്യ; entry = entry->next) {
		അക്കം_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      അക്കം_contents);

	/* Drain cache */
	ചെയ്യുക {
		prev_contents = അക്കം_contents;

		hlcache_clean(ശൂന്യ);

		വേണ്ടി (അക്കം_contents = 0, entry = hlcache->content_list;
				entry != ശൂന്യ; entry = entry->next) {
			അക്കം_contents++;
		}
	} ആകുമ്പോൾ (അക്കം_contents > 0 && അക്കം_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			അക്കം_contents);

	/* Drain cache again, forcing the matter */
	ചെയ്യുക {
		prev_contents = അക്കം_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		വേണ്ടി (അക്കം_contents = 0, entry = hlcache->content_list;
				entry != ശൂന്യ; entry = entry->next) {
			അക്കം_contents++;
		}
	} ആകുമ്പോൾ (അക്കം_contents > 0 && അക്കം_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", അക്കം_contents);
	വേണ്ടി (entry = hlcache->content_list; entry != ശൂന്യ; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ശൂന്യ, ശൂന്യ };

		എങ്കിൽ (entry->content != ശൂന്യ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_എണ്ണം_users(entry->content));
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	എങ്കിൽ (hlcache->retrieval_ctx_ring != ശൂന്യ) {
		ctx = hlcache->retrieval_ctx_ring;

		ചെയ്യുക {
			next = ctx->r_next;

			എങ്കിൽ (ctx->llcache != ശൂന്യ)
				llcache_handle_release(ctx->llcache);

			എങ്കിൽ (ctx->handle != ശൂന്യ)
				സ്വതന്ത്രം(ctx->handle);

			എങ്കിൽ (ctx->child.charset != ശൂന്യ)
				സ്വതന്ത്രം((അക്ഷരം *) ctx->child.charset);

			സ്വതന്ത്രം(ctx);

			ctx = next;
		} ആകുമ്പോൾ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ശൂന്യ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_എണ്ണം,
	      hlcache->miss_എണ്ണം);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, ശൂന്യ);

	സ്വതന്ത്രം(hlcache);
	hlcache = ശൂന്യ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, ശൂന്യം *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	പ്രതിജ്ഞാ(cb != ശൂന്യ);

	ctx = calloc(1, വലുപ്പം(hlcache_retrieval_ctx));
	എങ്കിൽ (ctx == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, വലുപ്പം(hlcache_handle));
	എങ്കിൽ (ctx->handle == ശൂന്യ) {
		സ്വതന്ത്രം(ctx);
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (child != ശൂന്യ) {
		എങ്കിൽ (child->charset != ശൂന്യ) {
			ctx->child.charset = strdup(child->charset);
			എങ്കിൽ (ctx->child.charset == ശൂന്യ) {
				സ്വതന്ത്രം(ctx->handle);
				സ്വതന്ത്രം(ctx);
				തിരികെ NSERROR_NOMEM;
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
	എങ്കിൽ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		സ്വതന്ത്രം((അക്ഷരം *) ctx->child.charset);
		സ്വതന്ത്രം(ctx->handle);
		സ്വതന്ത്രം(ctx);
	} അല്ലെങ്കിൽ {
		/* successfully started fetch so add new context to list */
		RING_തിരുകുക(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	തിരികെ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	എങ്കിൽ (handle->entry != ശൂന്യ) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} അല്ലെങ്കിൽ {
		RING_ITERATE_START(ഘടന hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			എങ്കിൽ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				സ്വതന്ത്രം((അക്ഷരം *) ictx->child.charset);
				സ്വതന്ത്രം(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ശൂന്യ;
	handle->pw = ശൂന്യ;

	സ്വതന്ത്രം(handle);

	തിരികെ NSERROR_OK;
}

/* See hlcache.h for documentation */
ഘടന content *hlcache_handle_get_content(സ്ഥിരം hlcache_handle *handle)
{
	എങ്കിൽ ((handle != ശൂന്യ) && (handle->entry != ശൂന്യ)) {
		തിരികെ handle->entry->content;
	}

	തിരികെ ശൂന്യ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ഘടന hlcache_entry *entry = handle->entry;
	ഘടന content *c;

	എങ്കിൽ (entry == ശൂന്യ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(ഘടന hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			എങ്കിൽ (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				സ്വതന്ത്രം((അക്ഷരം *) ictx->child.charset);
				സ്വതന്ത്രം(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		തിരികെ NSERROR_OK;
	}

	c = entry->content;

	എങ്കിൽ (content_എണ്ണം_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ഘടന content *clone = content_clone(c);

		എങ്കിൽ (clone == ശൂന്യ)
			തിരികെ NSERROR_NOMEM;

		entry = calloc(1, വലുപ്പം(ഘടന hlcache_entry));

		എങ്കിൽ (entry == ശൂന്യ) {
			content_destroy(clone);
			തിരികെ NSERROR_NOMEM;
		}

		എങ്കിൽ (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			സ്വതന്ത്രം(entry);
			തിരികെ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = ശൂന്യ;
		entry->next = hlcache->content_list;
		എങ്കിൽ (hlcache->content_list != ശൂന്യ)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	തിരികെ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, ശൂന്യം *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	തിരികെ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = ശൂന്യ;
	തിരികെ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(സ്ഥിരം hlcache_handle *handle)
{
	nsurl *result = ശൂന്യ;

	പ്രതിജ്ഞാ(handle != ശൂന്യ);

	എങ്കിൽ (handle->entry != ശൂന്യ) {
		result = content_get_url(handle->entry->content);
	} അല്ലെങ്കിൽ {
		RING_ITERATE_START(ഘടന hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			എങ്കിൽ (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	തിരികെ result;
}
