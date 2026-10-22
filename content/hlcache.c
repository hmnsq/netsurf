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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/ring.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "content/mimesniff.h"
#అన్తర్భావ "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"

రకనిర్వచనం నిర్మాణం hlcache_నమోదు hlcache_నమోదు;
రకనిర్వచనం నిర్మాణం hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
నిర్మాణం hlcache_retrieval_ctx {
	నిర్మాణం hlcache_retrieval_ctx *r_prev; /**< Previous retrieval context in the ring */
	నిర్మాణం hlcache_retrieval_ctx *r_తదుపరి; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_బిడ్డ_context బిడ్డ;	/**< Child context */

	bool migrate_లక్ష్యం;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
నిర్మాణం hlcache_handle {
	hlcache_నమోదు *నమోదు_2;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	శూన్య *pw;			/**< Client data */
};

/** Entry in high-level cache */
నిర్మాణం hlcache_నమోదు {
	నిర్మాణం content *content;	/**< Pointer to associated content */

	hlcache_నమోదు *తదుపరి;		/**< Next sibling */
	hlcache_నమోదు *prev;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
నిర్మాణం hlcache_s {
	నిర్మాణం hlcache_parameters params;

	/** List of cached content objects */
	hlcache_నమోదు *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	గుర్తులేని పూర్ణాంకం తగిలింది_లెక్క;
	గుర్తులేని పూర్ణాంకం తప్పిపోవు_లెక్క;
};

/** high level cache state */
స్థిర నిర్మాణం hlcache_s *hlcache = ఖాళీ;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
స్థిర శూన్య hlcache_clean(శూన్య *force_clean_flag)
{
	hlcache_నమోదు *నమోదు_2, *తదుపరి;
	bool force_clean = (force_clean_flag != ఖాళీ);

	కొరకు (నమోదు_2 = hlcache->content_list; నమోదు_2 != ఖాళీ; నమోదు_2 = తదుపరి) {
		తదుపరి = నమోదు_2->తదుపరి;

		ఒకవేళ (నమోదు_2->content == ఖాళీ)
			కొనసాగు;

		ఒకవేళ (content_లెక్క_users(నమోదు_2->content) != 0)
			కొనసాగు;

		ఒకవేళ (content_get_స్థితి_2(నమోదు_2->content) == CONTENT_స్థితి_LOADING) {
			ఒకవేళ (force_clean == false)
				కొనసాగు;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(నమోదు_2->content);
			content_సమితి_error(నమోదు_2->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		ఒకవేళ (నమోదు_2->prev == ఖాళీ)
			hlcache->content_list = నమోదు_2->తదుపరి;
		లేకపోతే
			నమోదు_2->prev->తదుపరి = నమోదు_2->తదుపరి;

		ఒకవేళ (నమోదు_2->తదుపరి != ఖాళీ)
			నమోదు_2->తదుపరి->prev = నమోదు_2->prev;

		/* Destroy content */
		content_destroy(నమోదు_2->content);

		/* Destroy entry */
		విడుదల(నమోదు_2);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->కాలపట్టిక(hlcache->params.bg_clean_సమయం, hlcache_clean, ఖాళీ);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
స్థిర bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	తిరిగి ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
స్థిర శూన్య hlcache_content_callback(నిర్మాణం content *c, content_సందేశం సందేశం,
		స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం, శూన్య *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = సందేశం,
	};

	ఒకవేళ (దత్తాంశం != ఖాళీ) {
		event.దత్తాంశం = *దత్తాంశం;
	}

	ఒకవేళ (handle->cb != ఖాళీ)
		error = handle->cb(handle, &event, handle->pw);

	ఒకవేళ (error != NSERROR_OK)
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
స్థిర nserror hlcache_కనుగొను_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_నమోదు *నమోదు_2;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	కొరకు (నమోదు_2 = hlcache->content_list; నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
		hlcache_handle నమోదు_handle = { నమోదు_2, ఖాళీ, ఖాళీ };
		స్థిరరాశి llcache_handle *నమోదు_llcache;

		ఒకవేళ (నమోదు_2->content == ఖాళీ)
			కొనసాగు;

		/* Ignore contents in the error state */
		ఒకవేళ (content_get_స్థితి(&నమోదు_handle) == CONTENT_స్థితి_ERROR)
			కొనసాగు;

		/* Ensure that content is shareable */
		ఒకవేళ (content_is_shareable(నమోదు_2->content) == false)
			కొనసాగు;

		/* Ensure that quirks mode is acceptable */
		ఒకవేళ (content_matches_quirks(నమోదు_2->content,
				ctx->బిడ్డ.quirks) == false)
			కొనసాగు;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		నమోదు_llcache = content_get_llcache_handle(నమోదు_2->content);

		ఒకవేళ (llcache_handle_references_same_object(నమోదు_llcache,
				ctx->llcache))
			ఆపు;
	}

	ఒకవేళ (నమోదు_2 == ఖాళీ) {
		/* No existing entry, so need to create one */
		నమోదు_2 = జ్ఞాపకం_ఇవ్వు(పరిమాణం(hlcache_నమోదు));
		ఒకవేళ (నమోదు_2 == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		/* Create content using llhandle */
		నమోదు_2->content = content_factory_create_content(ctx->llcache,
				ctx->బిడ్డ.charset, ctx->బిడ్డ.quirks,
				effective_type);
		ఒకవేళ (నమోదు_2->content == ఖాళీ) {
			విడుదల(నమోదు_2);
			తిరిగి NSERROR_NOMEM;
		}

		/* Insert into cache */
		నమోదు_2->prev = ఖాళీ;
		నమోదు_2->తదుపరి = hlcache->content_list;
		ఒకవేళ (hlcache->content_list != ఖాళీ)
			hlcache->content_list->prev = నమోదు_2;
		hlcache->content_list = నమోదు_2;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_దత్తాంశం;

		hlcache->తప్పిపోవు_లెక్క++;
	} లేకపోతే {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->తగిలింది_లెక్క++;
	}

	/* Associate handle with content */
	ఒకవేళ (content_కూడు_user(నమోదు_2->content,
			hlcache_content_callback, ctx->handle) == false)
		తిరిగి NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->నమోదు_2 = నమోదు_2;

	/* Catch handle up with state of content */
	ఒకవేళ (ctx->handle->cb != ఖాళీ) {
		content_స్థితి స్థితి_2 = content_get_స్థితి(ctx->handle);

		ఒకవేళ (స్థితి_2 == CONTENT_స్థితి_LOADING) {
			event.type = CONTENT_సందేశం_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} లేకపోతే ఒకవేళ (స్థితి_2 == CONTENT_స్థితి_READY) {
			event.type = CONTENT_సందేశం_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ఒకవేళ (ctx->handle->cb != ఖాళీ) {
				event.type = CONTENT_సందేశం_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} లేకపోతే ఒకవేళ (స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
			event.type = CONTENT_సందేశం_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			ఒకవేళ (ctx->handle->cb != ఖాళీ) {
				event.type = CONTENT_సందేశం_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			ఒకవేళ (ctx->handle->cb != ఖాళీ) {
				event.type = CONTENT_సందేశం_పూర్తి;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	తిరిగి error;
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
స్థిర nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_లక్ష్యం = true;

	ఒకవేళ ((effective_type != ఖాళీ) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_కనుగొను_content(ctx, effective_type);
		ఒకవేళ (error != NSERROR_OK && error != NSERROR_NEED_దత్తాంశం) {
			ఒకవేళ (ctx->handle->cb != ఖాళీ) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_సందేశం_ERROR;
				hlevent.దత్తాంశం.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.దత్తాంశం.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} లేకపోతే ఒకవేళ (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_ప్రవాహం(ctx->llcache);

		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_DOWNLOAD;
			hlevent.దత్తాంశం.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_దత్తాంశం;
	} లేకపోతే {
		/* Unacceptable type: report error */
		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_ERROR;
			hlevent.దత్తాంశం.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.దత్తాంశం.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_లక్ష్యం = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	విడుదల((అక్షరం *) ctx->బిడ్డ.charset);
	విడుదల(ctx);

	తిరిగి error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
స్థిర nserror
hlcache_llcache_callback(llcache_handle *handle,
			 స్థిరరాశి llcache_event *event,
			 శూన్య *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = ఖాళీ;
	nserror error;

	ప్రతిజ్ఞా(ctx->llcache == handle);

	ఎంపిక (event->type) {
	స్థితి LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_SSL_CERTS;
			hlevent.దత్తాంశం.chain = event->దత్తాంశం.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ఆపు;
	స్థితి LLCACHE_EVENT_HAD_శీర్షికలు:
		error = mimesniff_compute_effective_type(llcache_handle_get_శీర్షిక(handle, "Content-Type"), ఖాళీ, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ఒకవేళ (error == NSERROR_OK || error == NSERROR_NOT_దొరికింది) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		ఒకవేళ (error == NSERROR_NEED_దత్తాంశం)
			error = NSERROR_OK;

		తిరిగి error;

		ఆపు;
	స్థితి LLCACHE_EVENT_HAD_దత్తాంశం:
		error = mimesniff_compute_effective_type(llcache_handle_get_శీర్షిక(handle, "Content-Type"),
				event->దత్తాంశం.దత్తాంశం.buf, event->దత్తాంశం.దత్తాంశం.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		ఒకవేళ (error != NSERROR_OK) {
			ప్రతిజ్ఞా(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		తిరిగి error;

		ఆపు;
	స్థితి LLCACHE_EVENT_పూర్తి:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_శీర్షిక(handle, "Content-Type"),
				ఖాళీ, 0, false, false, &effective_type);
		ఒకవేళ (error == NSERROR_OK || error == NSERROR_NOT_దొరికింది) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			తిరిగి error;
		}

		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_ERROR;
			hlevent.దత్తాంశం.errordata.errorcode = error;
			hlevent.దత్తాంశం.errordata.errormsg = ఖాళీ;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ఆపు;
	స్థితి LLCACHE_EVENT_ERROR:
		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_ERROR;
			hlevent.దత్తాంశం.errordata.errorcode = event->దత్తాంశం.error.సంకేతం_2;
			hlevent.దత్తాంశం.errordata.errormsg = event->దత్తాంశం.error.సందేశం;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ఆపు;
	స్థితి LLCACHE_EVENT_పురోగతి:
		ఆపు;
	స్థితి LLCACHE_EVENT_REDIRECT:
		ఒకవేళ (ctx->handle->cb != ఖాళీ) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_సందేశం_REDIRECT;
			hlevent.దత్తాంశం.redirect.from = event->దత్తాంశం.redirect.from;
			hlevent.దత్తాంశం.redirect.గమ్యం = event->దత్తాంశం.redirect.గమ్యం;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		ఆపు;
	}

	తిరిగి NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(స్థిరరాశి నిర్మాణం hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం hlcache_s));
	ఒకవేళ (hlcache == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	ఒకవేళ (ret != NSERROR_OK) {
		విడుదల(hlcache);
		hlcache = ఖాళీ;
		తిరిగి ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->కాలపట్టిక(hlcache->params.bg_clean_సమయం, hlcache_clean, ఖాళీ);

	తిరిగి NSERROR_OK;
}

/* See hlcache.h for documentation */
శూన్య hlcache_stop(శూన్య)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->కాలపట్టిక(-1, hlcache_clean, ఖాళీ);
}

/* See hlcache.h for documentation */
శూన్య hlcache_finalise(శూన్య)
{
	uint32_t అంకె_contents, prev_contents;
	hlcache_నమోదు *నమోదు_2;
	hlcache_retrieval_ctx *ctx, *తదుపరి;

	/* Obtain initial count of contents remaining */
	కొరకు (అంకె_contents = 0, నమోదు_2 = hlcache->content_list;
			నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
		అంకె_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      అంకె_contents);

	/* Drain cache */
	చేయి {
		prev_contents = అంకె_contents;

		hlcache_clean(ఖాళీ);

		కొరకు (అంకె_contents = 0, నమోదు_2 = hlcache->content_list;
				నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
			అంకె_contents++;
		}
	} అయితే (అంకె_contents > 0 && అంకె_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			అంకె_contents);

	/* Drain cache again, forcing the matter */
	చేయి {
		prev_contents = అంకె_contents;

		hlcache_clean(&నమోదు_2); // Any non-NULL pointer will do

		కొరకు (అంకె_contents = 0, నమోదు_2 = hlcache->content_list;
				నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
			అంకె_contents++;
		}
	} అయితే (అంకె_contents > 0 && అంకె_contents != prev_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", అంకె_contents);
	కొరకు (నమోదు_2 = hlcache->content_list; నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
		hlcache_handle నమోదు_handle = { నమోదు_2, ఖాళీ, ఖాళీ };

		ఒకవేళ (నమోదు_2->content != ఖాళీ) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      నమోదు_2,
			      nsurl_access(hlcache_handle_get_url(&నమోదు_handle)),
			      content_లెక్క_users(నమోదు_2->content));
		} లేకపోతే {
			NSLOG(netsurf, INFO, "	%p", నమోదు_2);
		}
	}

	/* Clean up retrieval contexts */
	ఒకవేళ (hlcache->retrieval_ctx_ring != ఖాళీ) {
		ctx = hlcache->retrieval_ctx_ring;

		చేయి {
			తదుపరి = ctx->r_తదుపరి;

			ఒకవేళ (ctx->llcache != ఖాళీ)
				llcache_handle_release(ctx->llcache);

			ఒకవేళ (ctx->handle != ఖాళీ)
				విడుదల(ctx->handle);

			ఒకవేళ (ctx->బిడ్డ.charset != ఖాళీ)
				విడుదల((అక్షరం *) ctx->బిడ్డ.charset);

			విడుదల(ctx);

			ctx = తదుపరి;
		} అయితే (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = ఖాళీ;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->తగిలింది_లెక్క,
	      hlcache->తప్పిపోవు_లెక్క);

	/* De-schedule ourselves */
	guit->misc->కాలపట్టిక(-1, hlcache_clean, ఖాళీ);

	విడుదల(hlcache);
	hlcache = ఖాళీ;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_దత్తాంశం *post,
			hlcache_handle_callback cb, శూన్య *pw,
			hlcache_బిడ్డ_context *బిడ్డ,
			content_type accepted_types,
			hlcache_handle **ఫలితం)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	ప్రతిజ్ఞా(cb != ఖాళీ);

	ctx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(hlcache_retrieval_ctx));
	ఒకవేళ (ctx == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ctx->handle = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(hlcache_handle));
	ఒకవేళ (ctx->handle == ఖాళీ) {
		విడుదల(ctx);
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (బిడ్డ != ఖాళీ) {
		ఒకవేళ (బిడ్డ->charset != ఖాళీ) {
			ctx->బిడ్డ.charset = strdup(బిడ్డ->charset);
			ఒకవేళ (ctx->బిడ్డ.charset == ఖాళీ) {
				విడుదల(ctx->handle);
				విడుదల(ctx);
				తిరిగి NSERROR_NOMEM;
			}
		}
		ctx->బిడ్డ.quirks = బిడ్డ->quirks;
	}

	ctx->flags = flags;
	ctx->accepted_types = accepted_types;

	ctx->handle->cb = cb;
	ctx->handle->pw = pw;

	error = llcache_handle_retrieve(url, flags, referer, post,
			hlcache_llcache_callback, ctx,
			&ctx->llcache);
	ఒకవేళ (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		విడుదల((అక్షరం *) ctx->బిడ్డ.charset);
		విడుదల(ctx->handle);
		విడుదల(ctx);
	} లేకపోతే {
		/* successfully started fetch so add new context to list */
		RING_చొప్పించు(hlcache->retrieval_ctx_ring, ctx);

		*ఫలితం = ctx->handle;
	}
	తిరిగి error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	ఒకవేళ (handle->నమోదు_2 != ఖాళీ) {
		content_remove_user(handle->నమోదు_2->content,
				hlcache_content_callback, handle);
	} లేకపోతే {
		RING_ITERATE_ప్రారంభం(నిర్మాణం hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ఒకవేళ (ictx->handle == handle &&
					ictx->migrate_లక్ష్యం == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				విడుదల((అక్షరం *) ictx->బిడ్డ.charset);
				విడుదల(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_చివర(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = ఖాళీ;
	handle->pw = ఖాళీ;

	విడుదల(handle);

	తిరిగి NSERROR_OK;
}

/* See hlcache.h for documentation */
నిర్మాణం content *hlcache_handle_get_content(స్థిరరాశి hlcache_handle *handle)
{
	ఒకవేళ ((handle != ఖాళీ) && (handle->నమోదు_2 != ఖాళీ)) {
		తిరిగి handle->నమోదు_2->content;
	}

	తిరిగి ఖాళీ;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	నిర్మాణం hlcache_నమోదు *నమోదు_2 = handle->నమోదు_2;
	నిర్మాణం content *c;

	ఒకవేళ (నమోదు_2 == ఖాళీ) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_ప్రారంభం(నిర్మాణం hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ఒకవేళ (ictx->handle == handle &&
					ictx->migrate_లక్ష్యం == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				విడుదల((అక్షరం *) ictx->బిడ్డ.charset);
				విడుదల(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_చివర(hlcache->retrieval_ctx_ring, ictx);

		తిరిగి NSERROR_OK;
	}

	c = నమోదు_2->content;

	ఒకవేళ (content_లెక్క_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		నిర్మాణం content *clone = content_clone(c);

		ఒకవేళ (clone == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		నమోదు_2 = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం hlcache_నమోదు));

		ఒకవేళ (నమోదు_2 == ఖాళీ) {
			content_destroy(clone);
			తిరిగి NSERROR_NOMEM;
		}

		ఒకవేళ (content_కూడు_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			విడుదల(నమోదు_2);
			తిరిగి NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		నమోదు_2->content = clone;
		handle->నమోదు_2 = నమోదు_2;
		నమోదు_2->prev = ఖాళీ;
		నమోదు_2->తదుపరి = hlcache->content_list;
		ఒకవేళ (hlcache->content_list != ఖాళీ)
			hlcache->content_list->prev = నమోదు_2;
		hlcache->content_list = నమోదు_2;

		c = clone;
	}

	తిరిగి content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, శూన్య *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	తిరిగి NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **ఫలితం)
{
	*ఫలితం = ఖాళీ;
	తిరిగి NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(స్థిరరాశి hlcache_handle *handle)
{
	nsurl *ఫలితం = ఖాళీ;

	ప్రతిజ్ఞా(handle != ఖాళీ);

	ఒకవేళ (handle->నమోదు_2 != ఖాళీ) {
		ఫలితం = content_get_url(handle->నమోదు_2->content);
	} లేకపోతే {
		RING_ITERATE_ప్రారంభం(నిర్మాణం hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			ఒకవేళ (ictx->handle == handle) {
				/* This is the nascent context for us */
				ఫలితం = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_చివర(hlcache->retrieval_ctx_ring, ictx);
	}

	తిరిగి ఫలితం;
}
