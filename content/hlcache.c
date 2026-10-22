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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/ring.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "content/mimesniff.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା hlcache_entry hlcache_entry;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
ସଂରଚନା hlcache_retrieval_ctx {
	ସଂରଚନା hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	ସଂରଚନା hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
ସଂରଚନା hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	ଶୂନ୍ୟ *pw;			/**< Client data */
};

/** Entry in high-level cache */
ସଂରଚନା hlcache_entry {
	ସଂରଚନା content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
ସଂରଚନା hlcache_s {
	ସଂରଚନା hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା hit_ଗଣନା;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା miss_ଗଣନା;
};

/** high level cache state */
ସ୍ଥିର ସଂରଚନା hlcache_s *hlcache = ଖାଲି;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
ସ୍ଥିର ଶୂନ୍ୟ hlcache_clean(ଶୂନ୍ୟ *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != ଖାଲି);

	ପାଇଁ (entry = hlcache->content_list; entry != ଖାଲି; entry = next) {
		next = entry->next;

		ଯଦି (entry->content == ଖାଲି)
			ଜାରି;

		ଯଦି (content_ଗଣନା_users(entry->content) != 0)
			ଜାରି;

		ଯଦି (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			ଯଦି (force_clean == false)
				ଜାରି;
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
		ଯଦି (entry->prev == ଖାଲି)
			hlcache->content_list = entry->next;
		ଅନ୍ୟଥା
			entry->prev->next = entry->next;

		ଯଦି (entry->next != ଖାଲି)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		ମୁକ୍ତ(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ଖାଲି);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
ସ୍ଥିର bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	ଫେରନ୍ତୁ ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
ସ୍ଥିର ଶୂନ୍ୟ hlcache_content_callback(ସଂରଚନା content *c, content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
		ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data, ଶୂନ୍ୟ *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = ସନ୍ଦେଶ,
	};

	ଯଦି (data != ଖାଲି) {
		event.data = *data;
	}

	ଯଦି (handle->cb != ଖାଲି)
		error = handle->cb(handle, &event, handle->pw);

	ଯଦି (error != NSERROR_OK)
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
ସ୍ଥିର nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	ପାଇଁ (entry = hlcache->content_list; entry != ଖାଲି; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ଖାଲି, ଖାଲି };
		ଅଚଳ llcache_handle *entry_llcache;

		ଯଦି (entry->content == ଖାଲି)
			ଜାରି;

		/* Ignore contents in the error state */
		ଯଦି (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			ଜାରି;

		/* Ensure that content is shareable */
		ଯଦି (content_is_shareable(entry->content) == false)
			ଜାରି;

		/* Ensure that quirks mode is acceptable */
		ଯଦି (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			ଜାରି;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		ଯଦି (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			ଭାଙ୍ଗ;
	}

	ଯଦି (entry == ଖାଲି) {
		/* No existing entry, so need to create one */
		entry = malloc(ଆକାର(hlcache_entry));
		ଯଦି (entry == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		ଯଦି (entry->content == ଖାଲି) {
			ମୁକ୍ତ(entry);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = ଖାଲି;
		entry->next = hlcache->content_list;
		ଯଦି (hlcache->content_list != ଖାଲି)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_ଗଣନା++;
	} ଅନ୍ୟଥା {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_ଗଣନା++;
	}

	/* Associate handle with content */
	ଯଦି (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	ଯଦି (ctx->handle->cb != ଖାଲି) {
		content_status status = content_get_status(ctx->handle);

		ଯଦି (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_ସନ୍ଦେଶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} ଅନ୍ୟଥା ଯଦି (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_ସନ୍ଦେଶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ଯଦି (ctx->handle->cb != ଖାଲି) {
				event.type = CONTENT_ସନ୍ଦେଶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} ଅନ୍ୟଥା ଯଦି (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_ସନ୍ଦେଶ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ଯଦି (ctx->handle->cb != ଖାଲି) {
				event.type = CONTENT_ସନ୍ଦେଶ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ଯଦି (ctx->handle->cb != ଖାଲି) {
				event.type = CONTENT_ସନ୍ଦେଶ_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	ଫେରନ୍ତୁ error;
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
ସ୍ଥିର nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	ଯଦି ((effective_type != ଖାଲି) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		ଯଦି (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			ଯଦି (ctx->handle->cb != ଖାଲି) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_ସନ୍ଦେଶ_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} ଅନ୍ୟଥା ଯଦି (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} ଅନ୍ୟଥା {
		/* Unacceptable type: report error */
		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_ERROR;
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
	ମୁକ୍ତ((ଅକ୍ଷର *) ctx->child.charset);
	ମୁକ୍ତ(ctx);

	ଫେରନ୍ତୁ error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
ସ୍ଥିର nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ଅଚଳ llcache_event *event,
			 ଶୂନ୍ୟ *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ଖାଲି;
	nserror error;

	ପ୍ରତିଜ୍ଞା(ctx->llcache == handle);

	ବାଛ (event->type) {
	ସ୍ଥିତି LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), ଖାଲି, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ଯଦି (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ଯଦି (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		ଫେରନ୍ତୁ error;

		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ଯଦି (error != NSERROR_OK) {
			ପ୍ରତିଜ୍ଞା(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		ଫେରନ୍ତୁ error;

		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				ଖାଲି, 0, false, false, &effective_type);
		ଯଦି (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			ଫେରନ୍ତୁ error;
		}

		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = ଖାଲି;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_ERROR:
		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.ସନ୍ଦେଶ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_PROGRESS:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି LLCACHE_EVENT_REDIRECT:
		ଯଦି (ctx->handle->cb != ଖାଲି) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_ସନ୍ଦେଶ_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ଅଚଳ ସଂରଚନା hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, ଆକାର(ସଂରଚନା hlcache_s));
	ଯଦି (hlcache == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ଯଦି (ret != NSERROR_OK) {
		ମୁକ୍ତ(hlcache);
		hlcache = ଖାଲି;
		ଫେରନ୍ତୁ ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ଖାଲି);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* See hlcache.h for documentation */
ଶୂନ୍ୟ hlcache_stop(ଶୂନ୍ୟ)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, ଖାଲି);
}

/* See hlcache.h for documentation */
ଶୂନ୍ୟ hlcache_finalise(ଶୂନ୍ୟ)
{
	uint32_t ଅଙ୍କ_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	ପାଇଁ (ଅଙ୍କ_contents = 0, entry = hlcache->content_list;
			entry != ଖାଲି; entry = entry->next) {
		ଅଙ୍କ_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      ଅଙ୍କ_contents);

	/* Drain cache */
	କର {
		prev_contents = ଅଙ୍କ_contents;

		hlcache_clean(ଖାଲି);

		ପାଇଁ (ଅଙ୍କ_contents = 0, entry = hlcache->content_list;
				entry != ଖାଲି; entry = entry->next) {
			ଅଙ୍କ_contents++;
		}
	} ଯେତେବେଳେ (ଅଙ୍କ_contents > 0 && ଅଙ୍କ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			ଅଙ୍କ_contents);

	/* Drain cache again, forcing the matter */
	କର {
		prev_contents = ଅଙ୍କ_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		ପାଇଁ (ଅଙ୍କ_contents = 0, entry = hlcache->content_list;
				entry != ଖାଲି; entry = entry->next) {
			ଅଙ୍କ_contents++;
		}
	} ଯେତେବେଳେ (ଅଙ୍କ_contents > 0 && ଅଙ୍କ_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", ଅଙ୍କ_contents);
	ପାଇଁ (entry = hlcache->content_list; entry != ଖାଲି; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ଖାଲି, ଖାଲି };

		ଯଦି (entry->content != ଖାଲି) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_ଗଣନା_users(entry->content));
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	ଯଦି (hlcache->retrieval_ctx_ring != ଖାଲି) {
		ctx = hlcache->retrieval_ctx_ring;

		କର {
			next = ctx->r_next;

			ଯଦି (ctx->llcache != ଖାଲି)
				llcache_handle_release(ctx->llcache);

			ଯଦି (ctx->handle != ଖାଲି)
				ମୁକ୍ତ(ctx->handle);

			ଯଦି (ctx->child.charset != ଖାଲି)
				ମୁକ୍ତ((ଅକ୍ଷର *) ctx->child.charset);

			ମୁକ୍ତ(ctx);

			ctx = next;
		} ଯେତେବେଳେ (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ଖାଲି;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_ଗଣନା,
	      hlcache->miss_ଗଣନା);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, ଖାଲି);

	ମୁକ୍ତ(hlcache);
	hlcache = ଖାଲି;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, ଶୂନ୍ୟ *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ପ୍ରତିଜ୍ଞା(cb != ଖାଲି);

	ctx = calloc(1, ଆକାର(hlcache_retrieval_ctx));
	ଯଦି (ctx == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, ଆକାର(hlcache_handle));
	ଯଦି (ctx->handle == ଖାଲି) {
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (child != ଖାଲି) {
		ଯଦି (child->charset != ଖାଲି) {
			ctx->child.charset = strdup(child->charset);
			ଯଦି (ctx->child.charset == ଖାଲି) {
				ମୁକ୍ତ(ctx->handle);
				ମୁକ୍ତ(ctx);
				ଫେରନ୍ତୁ NSERROR_NOMEM;
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
	ଯଦି (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		ମୁକ୍ତ((ଅକ୍ଷର *) ctx->child.charset);
		ମୁକ୍ତ(ctx->handle);
		ମୁକ୍ତ(ctx);
	} ଅନ୍ୟଥା {
		/* successfully started fetch so add new context to list */
		RING_ଭର୍ତ୍ତି(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	ଫେରନ୍ତୁ error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ଯଦି (handle->entry != ଖାଲି) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} ଅନ୍ୟଥା {
		RING_ITERATE_START(ସଂରଚନା hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ଯଦି (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ମୁକ୍ତ((ଅକ୍ଷର *) ictx->child.charset);
				ମୁକ୍ତ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ଖାଲି;
	handle->pw = ଖାଲି;

	ମୁକ୍ତ(handle);

	ଫେରନ୍ତୁ NSERROR_OK;
}

/* See hlcache.h for documentation */
ସଂରଚନା content *hlcache_handle_get_content(ଅଚଳ hlcache_handle *handle)
{
	ଯଦି ((handle != ଖାଲି) && (handle->entry != ଖାଲି)) {
		ଫେରନ୍ତୁ handle->entry->content;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	ସଂରଚନା hlcache_entry *entry = handle->entry;
	ସଂରଚନା content *c;

	ଯଦି (entry == ଖାଲି) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(ସଂରଚନା hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ଯଦି (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				ମୁକ୍ତ((ଅକ୍ଷର *) ictx->child.charset);
				ମୁକ୍ତ(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		ଫେରନ୍ତୁ NSERROR_OK;
	}

	c = entry->content;

	ଯଦି (content_ଗଣନା_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		ସଂରଚନା content *clone = content_clone(c);

		ଯଦି (clone == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		entry = calloc(1, ଆକାର(ସଂରଚନା hlcache_entry));

		ଯଦି (entry == ଖାଲି) {
			content_destroy(clone);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		ଯଦି (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			ମୁକ୍ତ(entry);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = ଖାଲି;
		entry->next = hlcache->content_list;
		ଯଦି (hlcache->content_list != ଖାଲି)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	ଫେରନ୍ତୁ content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, ଶୂନ୍ୟ *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	ଫେରନ୍ତୁ NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = ଖାଲି;
	ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ଅଚଳ hlcache_handle *handle)
{
	nsurl *result = ଖାଲି;

	ପ୍ରତିଜ୍ଞା(handle != ଖାଲି);

	ଯଦି (handle->entry != ଖାଲି) {
		result = content_get_url(handle->entry->content);
	} ଅନ୍ୟଥା {
		RING_ITERATE_START(ସଂରଚନା hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ଯଦି (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	ଫେରନ୍ତୁ result;
}
