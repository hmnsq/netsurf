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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/http.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/ring.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "content/mimesniff.h"
#અન્તર્ભાવ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"

પ્રકારવ્યાખ્યા સંરચના hlcache_entry hlcache_entry;
પ્રકારવ્યાખ્યા સંરચના hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
સંરચના hlcache_retrieval_ctx {
	સંરચના hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	સંરચના hlcache_retrieval_ctx *r_next; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
સંરચના hlcache_handle {
	hlcache_entry *entry;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	શૂન્ય *pw;			/**< Client data */
};

/** Entry in high-level cache */
સંરચના hlcache_entry {
	સંરચના content *content;	/**< Pointer to associated content */

	hlcache_entry *next;		/**< Next sibling */
	hlcache_entry *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
સંરચના hlcache_s {
	સંરચના hlcache_parameters params;

	/** List of cached content objects */
	hlcache_entry *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	unsigned પૂર્ણાંક hit_ગણતરી;
	unsigned પૂર્ણાંક miss_ગણતરી;
};

/** high level cache state */
સ્થિર સંરચના hlcache_s *hlcache = ખાલી;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
સ્થિર શૂન્ય hlcache_clean(શૂન્ય *force_clean_flag)
{
	hlcache_entry *entry, *next;
	bool force_clean = (force_clean_flag != ખાલી);

	માટે (entry = hlcache->content_list; entry != ખાલી; entry = next) {
		next = entry->next;

		જો (entry->content == ખાલી)
			ચાલુ;

		જો (content_ગણતરી_users(entry->content) != 0)
			ચાલુ;

		જો (content__get_status(entry->content) == CONTENT_STATUS_LOADING) {
			જો (force_clean == false)
				ચાલુ;
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
		જો (entry->prev == ખાલી)
			hlcache->content_list = entry->next;
		નહીંતર
			entry->prev->next = entry->next;

		જો (entry->next != ખાલી)
			entry->next->prev = entry->prev;

		/* Destroy content */
		content_destroy(entry->content);

		/* Destroy entry */
		મુક્ત(entry);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ખાલી);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
સ્થિર bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	પાછા ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
સ્થિર શૂન્ય hlcache_content_callback(સંરચના content *c, content_સંદેશ સંદેશ,
		અચલ સંઘ content_સંદેશ_data *data, શૂન્ય *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = સંદેશ,
	};

	જો (data != ખાલી) {
		event.data = *data;
	}

	જો (handle->cb != ખાલી)
		error = handle->cb(handle, &event, handle->pw);

	જો (error != NSERROR_OK)
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
સ્થિર nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_entry *entry;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	માટે (entry = hlcache->content_list; entry != ખાલી; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ખાલી, ખાલી };
		અચલ llcache_handle *entry_llcache;

		જો (entry->content == ખાલી)
			ચાલુ;

		/* Ignore contents in the error state */
		જો (content_get_status(&entry_handle) == CONTENT_STATUS_ERROR)
			ચાલુ;

		/* Ensure that content is shareable */
		જો (content_is_shareable(entry->content) == false)
			ચાલુ;

		/* Ensure that quirks mode is acceptable */
		જો (content_matches_quirks(entry->content,
				ctx->child.quirks) == false)
			ચાલુ;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		entry_llcache = content_get_llcache_handle(entry->content);

		જો (llcache_handle_references_same_object(entry_llcache,
				ctx->llcache))
			તોડો;
	}

	જો (entry == ખાલી) {
		/* No existing entry, so need to create one */
		entry = malloc(કદ(hlcache_entry));
		જો (entry == ખાલી)
			પાછા NSERROR_NOMEM;

		/* Create content using llhandle */
		entry->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		જો (entry->content == ખાલી) {
			મુક્ત(entry);
			પાછા NSERROR_NOMEM;
		}

		/* Insert into cache */
		entry->prev = ખાલી;
		entry->next = hlcache->content_list;
		જો (hlcache->content_list != ખાલી)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_DATA;

		hlcache->miss_ગણતરી++;
	} નહીંતર {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->hit_ગણતરી++;
	}

	/* Associate handle with content */
	જો (content_add_user(entry->content,
			hlcache_content_callback, ctx->handle) == false)
		પાછા NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->entry = entry;

	/* Catch handle up with state of content */
	જો (ctx->handle->cb != ખાલી) {
		content_status status = content_get_status(ctx->handle);

		જો (status == CONTENT_STATUS_LOADING) {
			event.type = CONTENT_સંદેશ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} નહીંતર જો (status == CONTENT_STATUS_READY) {
			event.type = CONTENT_સંદેશ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			જો (ctx->handle->cb != ખાલી) {
				event.type = CONTENT_સંદેશ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} નહીંતર જો (status == CONTENT_STATUS_DONE) {
			event.type = CONTENT_સંદેશ_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			જો (ctx->handle->cb != ખાલી) {
				event.type = CONTENT_સંદેશ_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			જો (ctx->handle->cb != ખાલી) {
				event.type = CONTENT_સંદેશ_DONE;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	પાછા error;
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
સ્થિર nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	જો ((effective_type != ખાલી) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		જો (error != NSERROR_OK && error != NSERROR_NEED_DATA) {
			જો (ctx->handle->cb != ખાલી) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_સંદેશ_ERROR;
				hlevent.data.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.data.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} નહીંતર જો (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_stream(ctx->llcache);

		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_DOWNLOAD;
			hlevent.data.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_DATA;
	} નહીંતર {
		/* Unacceptable type: report error */
		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_ERROR;
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
	મુક્ત((અક્ષર *) ctx->child.charset);
	મુક્ત(ctx);

	પાછા error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
સ્થિર nserror
hlcache_llcache_callback(llcache_handle *handle,
			 અચલ llcache_event *event,
			 શૂન્ય *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ખાલી;
	nserror error;

	પ્રતિજ્ઞા(ctx->llcache == handle);

	પસંદગી (event->type) {
	સ્થિતિ LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_SSL_CERTS;
			hlevent.data.chain = event->data.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		તોડો;
	સ્થિતિ LLCACHE_EVENT_HAD_HEADERS:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"), ખાલી, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		જો (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		જો (error == NSERROR_NEED_DATA)
			error = NSERROR_OK;

		પાછા error;

		તોડો;
	સ્થિતિ LLCACHE_EVENT_HAD_DATA:
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				event->data.data.buf, event->data.data.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		જો (error != NSERROR_OK) {
			પ્રતિજ્ઞા(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		પાછા error;

		તોડો;
	સ્થિતિ LLCACHE_EVENT_DONE:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_header(handle, "Content-Type"),
				ખાલી, 0, false, false, &effective_type);
		જો (error == NSERROR_OK || error == NSERROR_NOT_FOUND) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			પાછા error;
		}

		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_ERROR;
			hlevent.data.errordata.errorcode = error;
			hlevent.data.errordata.errormsg = ખાલી;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		તોડો;
	સ્થિતિ LLCACHE_EVENT_ERROR:
		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_ERROR;
			hlevent.data.errordata.errorcode = event->data.error.code;
			hlevent.data.errordata.errormsg = event->data.error.સંદેશ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		તોડો;
	સ્થિતિ LLCACHE_EVENT_PROGRESS:
		તોડો;
	સ્થિતિ LLCACHE_EVENT_REDIRECT:
		જો (ctx->handle->cb != ખાલી) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_સંદેશ_REDIRECT;
			hlevent.data.redirect.from = event->data.redirect.from;
			hlevent.data.redirect.to = event->data.redirect.to;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		તોડો;
	}

	પાછા NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(અચલ સંરચના hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = calloc(1, કદ(સંરચના hlcache_s));
	જો (hlcache == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	જો (ret != NSERROR_OK) {
		મુક્ત(hlcache);
		hlcache = ખાલી;
		પાછા ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->schedule(hlcache->params.bg_clean_time, hlcache_clean, ખાલી);

	પાછા NSERROR_OK;
}

/* See hlcache.h for documentation */
શૂન્ય hlcache_stop(શૂન્ય)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->schedule(-1, hlcache_clean, ખાલી);
}

/* See hlcache.h for documentation */
શૂન્ય hlcache_finalise(શૂન્ય)
{
	uint32_t અંક_contents, prev_contents;
	hlcache_entry *entry;
	hlcache_retrieval_ctx *ctx, *next;

	/* Obtain initial count of contents remaining */
	માટે (અંક_contents = 0, entry = hlcache->content_list;
			entry != ખાલી; entry = entry->next) {
		અંક_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      અંક_contents);

	/* Drain cache */
	કરો {
		prev_contents = અંક_contents;

		hlcache_clean(ખાલી);

		માટે (અંક_contents = 0, entry = hlcache->content_list;
				entry != ખાલી; entry = entry->next) {
			અંક_contents++;
		}
	} જ્યારે (અંક_contents > 0 && અંક_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			અંક_contents);

	/* Drain cache again, forcing the matter */
	કરો {
		prev_contents = અંક_contents;

		hlcache_clean(&entry); // Any non-NULL pointer will do

		માટે (અંક_contents = 0, entry = hlcache->content_list;
				entry != ખાલી; entry = entry->next) {
			અંક_contents++;
		}
	} જ્યારે (અંક_contents > 0 && અંક_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", અંક_contents);
	માટે (entry = hlcache->content_list; entry != ખાલી; entry = entry->next) {
		hlcache_handle entry_handle = { entry, ખાલી, ખાલી };

		જો (entry->content != ખાલી) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      entry,
			      nsurl_access(hlcache_handle_get_url(&entry_handle)),
			      content_ગણતરી_users(entry->content));
		} નહીંતર {
			NSLOG(netsurf, INFO, "	%p", entry);
		}
	}

	/* Clean up retrieval contexts */
	જો (hlcache->retrieval_ctx_ring != ખાલી) {
		ctx = hlcache->retrieval_ctx_ring;

		કરો {
			next = ctx->r_next;

			જો (ctx->llcache != ખાલી)
				llcache_handle_release(ctx->llcache);

			જો (ctx->handle != ખાલી)
				મુક્ત(ctx->handle);

			જો (ctx->child.charset != ખાલી)
				મુક્ત((અક્ષર *) ctx->child.charset);

			મુક્ત(ctx);

			ctx = next;
		} જ્યારે (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ખાલી;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->hit_ગણતરી,
	      hlcache->miss_ગણતરી);

	/* De-schedule ourselves */
	guit->misc->schedule(-1, hlcache_clean, ખાલી);

	મુક્ત(hlcache);
	hlcache = ખાલી;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_data *post,
			hlcache_handle_callback cb, શૂન્ય *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **result)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	પ્રતિજ્ઞા(cb != ખાલી);

	ctx = calloc(1, કદ(hlcache_retrieval_ctx));
	જો (ctx == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	ctx->handle = calloc(1, કદ(hlcache_handle));
	જો (ctx->handle == ખાલી) {
		મુક્ત(ctx);
		પાછા NSERROR_NOMEM;
	}

	જો (child != ખાલી) {
		જો (child->charset != ખાલી) {
			ctx->child.charset = strdup(child->charset);
			જો (ctx->child.charset == ખાલી) {
				મુક્ત(ctx->handle);
				મુક્ત(ctx);
				પાછા NSERROR_NOMEM;
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
	જો (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		મુક્ત((અક્ષર *) ctx->child.charset);
		મુક્ત(ctx->handle);
		મુક્ત(ctx);
	} નહીંતર {
		/* successfully started fetch so add new context to list */
		RING_ઉમેરો(hlcache->retrieval_ctx_ring, ctx);

		*result = ctx->handle;
	}
	પાછા error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	જો (handle->entry != ખાલી) {
		content_remove_user(handle->entry->content,
				hlcache_content_callback, handle);
	} નહીંતર {
		RING_ITERATE_START(સંરચના hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			જો (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				મુક્ત((અક્ષર *) ictx->child.charset);
				મુક્ત(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ખાલી;
	handle->pw = ખાલી;

	મુક્ત(handle);

	પાછા NSERROR_OK;
}

/* See hlcache.h for documentation */
સંરચના content *hlcache_handle_get_content(અચલ hlcache_handle *handle)
{
	જો ((handle != ખાલી) && (handle->entry != ખાલી)) {
		પાછા handle->entry->content;
	}

	પાછા ખાલી;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	સંરચના hlcache_entry *entry = handle->entry;
	સંરચના content *c;

	જો (entry == ખાલી) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(સંરચના hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			જો (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				મુક્ત((અક્ષર *) ictx->child.charset);
				મુક્ત(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);

		પાછા NSERROR_OK;
	}

	c = entry->content;

	જો (content_ગણતરી_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		સંરચના content *clone = content_clone(c);

		જો (clone == ખાલી)
			પાછા NSERROR_NOMEM;

		entry = calloc(1, કદ(સંરચના hlcache_entry));

		જો (entry == ખાલી) {
			content_destroy(clone);
			પાછા NSERROR_NOMEM;
		}

		જો (content_add_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			મુક્ત(entry);
			પાછા NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		entry->content = clone;
		handle->entry = entry;
		entry->prev = ખાલી;
		entry->next = hlcache->content_list;
		જો (hlcache->content_list != ખાલી)
			hlcache->content_list->prev = entry;
		hlcache->content_list = entry;

		c = clone;
	}

	પાછા content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, શૂન્ય *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	પાછા NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **result)
{
	*result = ખાલી;
	પાછા NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(અચલ hlcache_handle *handle)
{
	nsurl *result = ખાલી;

	પ્રતિજ્ઞા(handle != ખાલી);

	જો (handle->entry != ખાલી) {
		result = content_get_url(handle->entry->content);
	} નહીંતર {
		RING_ITERATE_START(સંરચના hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			જો (ictx->handle == handle) {
				/* This is the nascent context for us */
				result = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_END(hlcache->retrieval_ctx_ring, ictx);
	}

	પાછા result;
}
