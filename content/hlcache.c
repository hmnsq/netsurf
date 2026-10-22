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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/ring.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "content/mimesniff.h"
#अन्तर्भाव "content/hlcache.h"
// Note, this is *ONLY* so that we can abort cleanly during shutdown of the cache
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"

प्रकारनिर्देश संरचना hlcache_प्रविष्टिः hlcache_प्रविष्टिः;
प्रकारनिर्देश संरचना hlcache_retrieval_ctx hlcache_retrieval_ctx;

/** High-level cache retrieval context */
संरचना hlcache_retrieval_ctx {
	संरचना hlcache_retrieval_ctx *r_पूर्व; /**< Previous retrieval context in the ring */
	संरचना hlcache_retrieval_ctx *r_अग्र; /**< Next retrieval context in the ring */

	llcache_handle *llcache;	/**< Low-level cache handle */

	hlcache_handle *handle;		/**< High-level handle for object */

	uint32_t flags;			/**< Retrieval flags */

	content_type accepted_types;	/**< Accepted types */

	hlcache_child_context child;	/**< Child context */

	bool migrate_target;		/**< Whether this context is the migration target */
};

/** High-level cache handle */
संरचना hlcache_handle {
	hlcache_प्रविष्टिः *प्रविष्टिः;		/**< Pointer to cache entry */

	hlcache_handle_callback cb;	/**< Client callback */
	शून्य *pw;			/**< Client data */
};

/** Entry in high-level cache */
संरचना hlcache_प्रविष्टिः {
	संरचना content *content;	/**< Pointer to associated content */

	hlcache_प्रविष्टिः *अग्र;		/**< Next sibling */
	hlcache_प्रविष्टिः *पूर्व;		/**< Previous sibling */
};

/** Current state of the cache.
 *
 * Global state of the cache.
 */
संरचना hlcache_s {
	संरचना hlcache_parameters params;

	/** List of cached content objects */
	hlcache_प्रविष्टिः *content_list;

	/** Ring of retrieval contexts */
	hlcache_retrieval_ctx *retrieval_ctx_ring;

	/* statistics */
	अचिह्नित पूर्णाङ्क लब्धम्_गणनम्;
	अचिह्नित पूर्णाङ्क अलब्धिः_गणनम्;
};

/** high level cache state */
स्थिर संरचना hlcache_s *hlcache = रिक्त;


/******************************************************************************
 * High-level cache internals						      *
 ******************************************************************************/


/**
 * Attempt to clean the cache
 */
स्थिर शून्य hlcache_clean(शून्य *force_clean_flag)
{
	hlcache_प्रविष्टिः *प्रविष्टिः, *अग्र;
	bool force_clean = (force_clean_flag != रिक्त);

	प्रति (प्रविष्टिः = hlcache->content_list; प्रविष्टिः != रिक्त; प्रविष्टिः = अग्र) {
		अग्र = प्रविष्टिः->अग्र;

		यदि (प्रविष्टिः->content == रिक्त)
			अनुवर्त;

		यदि (content_गणनम्_users(प्रविष्टिः->content) != 0)
			अनुवर्त;

		यदि (content_get_स्थितिः_2(प्रविष्टिः->content) == CONTENT_स्थितिः_LOADING) {
			यदि (force_clean == false)
				अनुवर्त;
			NSLOG(netsurf, DEBUG, "Forcing content cleanup during shutdown");
			content_abort(प्रविष्टिः->content);
			content_समुच्चयः_error(प्रविष्टिः->content);
		}

		/** \todo This is over-zealous: all unused contents
		 * will be immediately destroyed. Ideally, we want to
		 * purge all unused contents that are using stale
		 * source data, and enough fresh contents such that
		 * the cache fits in the configured cache size limit.
		 */

		/* Remove entry from cache */
		यदि (प्रविष्टिः->पूर्व == रिक्त)
			hlcache->content_list = प्रविष्टिः->अग्र;
		अन्यथा
			प्रविष्टिः->पूर्व->अग्र = प्रविष्टिः->अग्र;

		यदि (प्रविष्टिः->अग्र != रिक्त)
			प्रविष्टिः->अग्र->पूर्व = प्रविष्टिः->पूर्व;

		/* Destroy content */
		content_destroy(प्रविष्टिः->content);

		/* Destroy entry */
		मुक्त(प्रविष्टिः);
	}

	/* Attempt to clean the llcache */
	llcache_clean(false);

	/* Re-schedule ourselves */
	guit->misc->कालसारणी(hlcache->params.bg_clean_time, hlcache_clean, रिक्त);
}

/**
 * Determine if the specified MIME type is acceptable
 *
 * \param mime_type       MIME type to consider
 * \param accepted_types  Array of acceptable types, or NULL for any
 * \param computed_type	  Pointer to location to receive computed type of object
 * \return True if the type is acceptable, false otherwise
 */
स्थिर bool hlcache_type_is_acceptable(lwc_string *mime_type,
		content_type accepted_types, content_type *computed_type)
{
	content_type type;

	type = content_factory_type_from_mime_type(mime_type);

	*computed_type = type;

	प्रत्यागम ((accepted_types & type) != 0);
}

/**
 * Veneer between content callback API and hlcache callback API
 *
 * \param c	Content to emit message for
 * \param msg	Message to emit
 * \param data	Data for message
 * \param pw	Pointer to private data (hlcache_handle)
 */
स्थिर शून्य hlcache_content_callback(संरचना content *c, content_सन्देशः सन्देशः,
		ध्रुव सङ्घ content_सन्देशः_दत्त *दत्त, शून्य *pw)
{
	hlcache_handle *handle = pw;
	nserror error = NSERROR_OK;
	hlcache_event event = {
		.type = सन्देशः,
	};

	यदि (दत्त != रिक्त) {
		event.दत्त = *दत्त;
	}

	यदि (handle->cb != रिक्त)
		error = handle->cb(handle, &event, handle->pw);

	यदि (error != NSERROR_OK)
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
स्थिर nserror hlcache_find_content(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	hlcache_प्रविष्टिः *प्रविष्टिः;
	hlcache_event event;
	nserror error = NSERROR_OK;

	/* Search list of cached contents for a suitable one */
	प्रति (प्रविष्टिः = hlcache->content_list; प्रविष्टिः != रिक्त; प्रविष्टिः = प्रविष्टिः->अग्र) {
		hlcache_handle प्रविष्टिः_handle = { प्रविष्टिः, रिक्त, रिक्त };
		ध्रुव llcache_handle *प्रविष्टिः_llcache;

		यदि (प्रविष्टिः->content == रिक्त)
			अनुवर्त;

		/* Ignore contents in the error state */
		यदि (content_get_स्थितिः(&प्रविष्टिः_handle) == CONTENT_स्थितिः_ERROR)
			अनुवर्त;

		/* Ensure that content is shareable */
		यदि (content_is_shareable(प्रविष्टिः->content) == false)
			अनुवर्त;

		/* Ensure that quirks mode is acceptable */
		यदि (content_matches_quirks(प्रविष्टिः->content,
				ctx->child.quirks) == false)
			अनुवर्त;

		/* Ensure that content uses same low-level object as
		 * low-level handle */
		प्रविष्टिः_llcache = content_get_llcache_handle(प्रविष्टिः->content);

		यदि (llcache_handle_references_same_object(प्रविष्टिः_llcache,
				ctx->llcache))
			भङ्ग;
	}

	यदि (प्रविष्टिः == रिक्त) {
		/* No existing entry, so need to create one */
		प्रविष्टिः = स्मृतिं_देहि(परिमाण(hlcache_प्रविष्टिः));
		यदि (प्रविष्टिः == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		/* Create content using llhandle */
		प्रविष्टिः->content = content_factory_create_content(ctx->llcache,
				ctx->child.charset, ctx->child.quirks,
				effective_type);
		यदि (प्रविष्टिः->content == रिक्त) {
			मुक्त(प्रविष्टिः);
			प्रत्यागम NSERROR_NOMEM;
		}

		/* Insert into cache */
		प्रविष्टिः->पूर्व = रिक्त;
		प्रविष्टिः->अग्र = hlcache->content_list;
		यदि (hlcache->content_list != रिक्त)
			hlcache->content_list->पूर्व = प्रविष्टिः;
		hlcache->content_list = प्रविष्टिः;

		/* Signal to caller that we created a content */
		error = NSERROR_NEED_दत्त;

		hlcache->अलब्धिः_गणनम्++;
	} अन्यथा {
		/* Found a suitable content: no longer need low-level handle */
		llcache_handle_release(ctx->llcache);
		hlcache->लब्धम्_गणनम्++;
	}

	/* Associate handle with content */
	यदि (content_सङ्कलय_user(प्रविष्टिः->content,
			hlcache_content_callback, ctx->handle) == false)
		प्रत्यागम NSERROR_NOMEM;

	/* Associate cache entry with handle */
	ctx->handle->प्रविष्टिः = प्रविष्टिः;

	/* Catch handle up with state of content */
	यदि (ctx->handle->cb != रिक्त) {
		content_स्थितिः स्थितिः = content_get_स्थितिः(ctx->handle);

		यदि (स्थितिः == CONTENT_स्थितिः_LOADING) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);
		} अन्यथा यदि (स्थितिः == CONTENT_स्थितिः_READY) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			यदि (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		} अन्यथा यदि (स्थितिः == CONTENT_स्थितिः_समाप्तम्) {
			event.type = CONTENT_सन्देशः_LOADING;
			ctx->handle->cb(ctx->handle, &event, ctx->handle->pw);

			यदि (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_READY;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}

			यदि (ctx->handle->cb != रिक्त) {
				event.type = CONTENT_सन्देशः_समाप्तम्;
				ctx->handle->cb(ctx->handle, &event,
						ctx->handle->pw);
			}
		}
	}

	प्रत्यागम error;
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
स्थिर nserror hlcache_migrate_ctx(hlcache_retrieval_ctx *ctx,
		lwc_string *effective_type)
{
	content_type type = CONTENT_NONE;
	nserror error = NSERROR_OK;

	ctx->migrate_target = true;

	यदि ((effective_type != रिक्त) &&
	    hlcache_type_is_acceptable(effective_type,
				       ctx->accepted_types,
				       &type)) {
		error = hlcache_find_content(ctx, effective_type);
		यदि (error != NSERROR_OK && error != NSERROR_NEED_दत्त) {
			यदि (ctx->handle->cb != रिक्त) {
				hlcache_event hlevent;

				hlevent.type = CONTENT_सन्देशः_ERROR;
				hlevent.दत्त.errordata.errorcode = NSERROR_UNKNOWN;
				hlevent.दत्त.errordata.errormsg = messages_get("MiscError");

				ctx->handle->cb(ctx->handle, &hlevent,
						ctx->handle->pw);
			}

			llcache_handle_abort(ctx->llcache);
			llcache_handle_release(ctx->llcache);
		}
	} अन्यथा यदि (type == CONTENT_NONE &&
			(ctx->flags & HLCACHE_RETRIEVE_MAY_DOWNLOAD)) {
		/* Unknown type, and we can download, so convert */
		llcache_handle_force_प्रवाहः(ctx->llcache);

		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_DOWNLOAD;
			hlevent.दत्त.download = ctx->llcache;

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		/* Ensure caller knows we need data */
		error = NSERROR_NEED_दत्त;
	} अन्यथा {
		/* Unacceptable type: report error */
		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
			hlevent.दत्त.errordata.errorcode = NSERROR_UNKNOWN;
			hlevent.दत्त.errordata.errormsg = messages_get("UnacceptableType");

			ctx->handle->cb(ctx->handle, &hlevent,
					ctx->handle->pw);
		}

		llcache_handle_abort(ctx->llcache);
		llcache_handle_release(ctx->llcache);
	}

	ctx->migrate_target = false;

	/* No longer require retrieval context */
	RING_REMOVE(hlcache->retrieval_ctx_ring, ctx);
	मुक्त((वर्ण *) ctx->child.charset);
	मुक्त(ctx);

	प्रत्यागम error;
}

/**
 * Handler for low-level cache events
 *
 * \param handle  Handle for which event is issued
 * \param event	  Event data
 * \param pw	  Pointer to client-specific data
 * \return NSERROR_OK on success, appropriate error otherwise
 */
स्थिर nserror
hlcache_llcache_callback(llcache_handle *handle,
			 ध्रुव llcache_event *event,
			 शून्य *pw)
{
	hlcache_retrieval_ctx *ctx = pw;
	lwc_string *effective_type = रिक्त;
	nserror error;

	प्रतिज्ञा(ctx->llcache == handle);

	विकल्प (event->type) {
	पक्ष LLCACHE_EVENT_GOT_CERTS:
		/* Pass them on upward */
		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_SSL_CERTS;
			hlevent.दत्त.chain = event->दत्त.chain;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_शीर्षकाणि:
		error = mimesniff_compute_effective_type(llcache_handle_get_शीर्षकम्(handle, "Content-Type"), रिक्त, 0,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		यदि (error == NSERROR_OK || error == NSERROR_NOT_प्राप्तम्) {
			/* If the sniffer was successful or failed to find
			 * a Content-Type header when sniffing was
			 * prohibited, we must migrate the retrieval context. */
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
		}

		/* No need to report that we need data:
		 * we'll get some anyway if there is any */
		यदि (error == NSERROR_NEED_दत्त)
			error = NSERROR_OK;

		प्रत्यागम error;

		भङ्ग;
	पक्ष LLCACHE_EVENT_HAD_दत्त:
		error = mimesniff_compute_effective_type(llcache_handle_get_शीर्षकम्(handle, "Content-Type"),
				event->दत्त.दत्त.buf, event->दत्त.दत्त.len,
				ctx->flags & HLCACHE_RETRIEVE_SNIFF_TYPE,
				ctx->accepted_types == CONTENT_IMAGE,
				&effective_type);
		यदि (error != NSERROR_OK) {
			प्रतिज्ञा(0 && "MIME sniff failed with data");
		}

		error = hlcache_migrate_ctx(ctx, effective_type);

		lwc_string_unref(effective_type);

		प्रत्यागम error;

		भङ्ग;
	पक्ष LLCACHE_EVENT_समाप्तम्:
		/* DONE event before we could determine the effective MIME type.
		 */
		error = mimesniff_compute_effective_type(llcache_handle_get_शीर्षकम्(handle, "Content-Type"),
				रिक्त, 0, false, false, &effective_type);
		यदि (error == NSERROR_OK || error == NSERROR_NOT_प्राप्तम्) {
			error = hlcache_migrate_ctx(ctx, effective_type);

			lwc_string_unref(effective_type);
			प्रत्यागम error;
		}

		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
			hlevent.दत्त.errordata.errorcode = error;
			hlevent.दत्त.errordata.errormsg = रिक्त;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_ERROR:
		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_ERROR;
			hlevent.दत्त.errordata.errorcode = event->दत्त.error.संहिता;
			hlevent.दत्त.errordata.errormsg = event->दत्त.error.सन्देशः;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	पक्ष LLCACHE_EVENT_प्रगतिः:
		भङ्ग;
	पक्ष LLCACHE_EVENT_REDIRECT:
		यदि (ctx->handle->cb != रिक्त) {
			hlcache_event hlevent;

			hlevent.type = CONTENT_सन्देशः_REDIRECT;
			hlevent.दत्त.redirect.from = event->दत्त.redirect.from;
			hlevent.दत्त.redirect.गन्तव्यम् = event->दत्त.redirect.गन्तव्यम्;

			ctx->handle->cb(ctx->handle, &hlevent, ctx->handle->pw);
		}
		भङ्ग;
	}

	प्रत्यागम NSERROR_OK;
}


/******************************************************************************
 * Public API								      *
 ******************************************************************************/


nserror
hlcache_initialise(ध्रुव संरचना hlcache_parameters *hlcache_parameters)
{
	nserror ret;

	hlcache = शून्यस्मृतिं_देहि(1, परिमाण(संरचना hlcache_s));
	यदि (hlcache == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	ret = llcache_initialise(&hlcache_parameters->llcache);
	यदि (ret != NSERROR_OK) {
		मुक्त(hlcache);
		hlcache = रिक्त;
		प्रत्यागम ret;
	}

	hlcache->params = *hlcache_parameters;

	/* Schedule the cache cleanup */
	guit->misc->कालसारणी(hlcache->params.bg_clean_time, hlcache_clean, रिक्त);

	प्रत्यागम NSERROR_OK;
}

/* See hlcache.h for documentation */
शून्य hlcache_stop(शून्य)
{
	/* Remove the hlcache_clean schedule */
	guit->misc->कालसारणी(-1, hlcache_clean, रिक्त);
}

/* See hlcache.h for documentation */
शून्य hlcache_finalise(शून्य)
{
	uint32_t अङ्कः_contents, पूर्व_contents;
	hlcache_प्रविष्टिः *प्रविष्टिः;
	hlcache_retrieval_ctx *ctx, *अग्र;

	/* Obtain initial count of contents remaining */
	प्रति (अङ्कः_contents = 0, प्रविष्टिः = hlcache->content_list;
			प्रविष्टिः != रिक्त; प्रविष्टिः = प्रविष्टिः->अग्र) {
		अङ्कः_contents++;
	}

	NSLOG(netsurf, INFO, "%"PRIu32" contents remain before cache drain",
	      अङ्कः_contents);

	/* Drain cache */
	कुरु {
		पूर्व_contents = अङ्कः_contents;

		hlcache_clean(रिक्त);

		प्रति (अङ्कः_contents = 0, प्रविष्टिः = hlcache->content_list;
				प्रविष्टिः != रिक्त; प्रविष्टिः = प्रविष्टिः->अग्र) {
			अङ्कः_contents++;
		}
	} यावत् (अङ्कः_contents > 0 && अङ्कः_contents != पूर्व_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining after being polite",
			अङ्कः_contents);

	/* Drain cache again, forcing the matter */
	कुरु {
		पूर्व_contents = अङ्कः_contents;

		hlcache_clean(&प्रविष्टिः); // Any non-NULL pointer will do

		प्रति (अङ्कः_contents = 0, प्रविष्टिः = hlcache->content_list;
				प्रविष्टिः != रिक्त; प्रविष्टिः = प्रविष्टिः->अग्र) {
			अङ्कः_contents++;
		}
	} यावत् (अङ्कः_contents > 0 && अङ्कः_contents != पूर्व_contents);

	NSLOG(netsurf, INFO, "%"PRIu32" contents remaining:", अङ्कः_contents);
	प्रति (प्रविष्टिः = hlcache->content_list; प्रविष्टिः != रिक्त; प्रविष्टिः = प्रविष्टिः->अग्र) {
		hlcache_handle प्रविष्टिः_handle = { प्रविष्टिः, रिक्त, रिक्त };

		यदि (प्रविष्टिः->content != रिक्त) {
			NSLOG(netsurf, INFO, "	%p : %s (%"PRIu32" users)",
			      प्रविष्टिः,
			      nsurl_access(hlcache_handle_get_url(&प्रविष्टिः_handle)),
			      content_गणनम्_users(प्रविष्टिः->content));
		} अन्यथा {
			NSLOG(netsurf, INFO, "	%p", प्रविष्टिः);
		}
	}

	/* Clean up retrieval contexts */
	यदि (hlcache->retrieval_ctx_ring != रिक्त) {
		ctx = hlcache->retrieval_ctx_ring;

		कुरु {
			अग्र = ctx->r_अग्र;

			यदि (ctx->llcache != रिक्त)
				llcache_handle_release(ctx->llcache);

			यदि (ctx->handle != रिक्त)
				मुक्त(ctx->handle);

			यदि (ctx->child.charset != रिक्त)
				मुक्त((वर्ण *) ctx->child.charset);

			मुक्त(ctx);

			ctx = अग्र;
		} यावत् (ctx != hlcache->retrieval_ctx_ring);

		hlcache->retrieval_ctx_ring = रिक्त;
	}

	NSLOG(netsurf, INFO, "hit/miss %d/%d", hlcache->लब्धम्_गणनम्,
	      hlcache->अलब्धिः_गणनम्);

	/* De-schedule ourselves */
	guit->misc->कालसारणी(-1, hlcache_clean, रिक्त);

	मुक्त(hlcache);
	hlcache = रिक्त;

	NSLOG(netsurf, INFO, "Finalising low-level cache");
	llcache_finalise();
}

/* See hlcache.h for documentation */
nserror
hlcache_handle_retrieve(nsurl *url,
			uint32_t flags,
			nsurl *referer,
			llcache_post_दत्त *post,
			hlcache_handle_callback cb, शून्य *pw,
			hlcache_child_context *child,
			content_type accepted_types,
			hlcache_handle **परिणाम)
{
	hlcache_retrieval_ctx *ctx;
	nserror error;

	प्रतिज्ञा(cb != रिक्त);

	ctx = शून्यस्मृतिं_देहि(1, परिमाण(hlcache_retrieval_ctx));
	यदि (ctx == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	ctx->handle = शून्यस्मृतिं_देहि(1, परिमाण(hlcache_handle));
	यदि (ctx->handle == रिक्त) {
		मुक्त(ctx);
		प्रत्यागम NSERROR_NOMEM;
	}

	यदि (child != रिक्त) {
		यदि (child->charset != रिक्त) {
			ctx->child.charset = strdup(child->charset);
			यदि (ctx->child.charset == रिक्त) {
				मुक्त(ctx->handle);
				मुक्त(ctx);
				प्रत्यागम NSERROR_NOMEM;
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
	यदि (error != NSERROR_OK) {
		/* error retrieving handle so free context and return error */
		मुक्त((वर्ण *) ctx->child.charset);
		मुक्त(ctx->handle);
		मुक्त(ctx);
	} अन्यथा {
		/* successfully started fetch so add new context to list */
		RING_निवेशय(hlcache->retrieval_ctx_ring, ctx);

		*परिणाम = ctx->handle;
	}
	प्रत्यागम error;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_release(hlcache_handle *handle)
{
	यदि (handle->प्रविष्टिः != रिक्त) {
		content_remove_user(handle->प्रविष्टिः->content,
				hlcache_content_callback, handle);
	} अन्यथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			यदि (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((वर्ण *) ictx->child.charset);
				मुक्त(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_अन्तः(hlcache->retrieval_ctx_ring, ictx);
	}

	handle->cb = रिक्त;
	handle->pw = रिक्त;

	मुक्त(handle);

	प्रत्यागम NSERROR_OK;
}

/* See hlcache.h for documentation */
संरचना content *hlcache_handle_get_content(ध्रुव hlcache_handle *handle)
{
	यदि ((handle != रिक्त) && (handle->प्रविष्टिः != रिक्त)) {
		प्रत्यागम handle->प्रविष्टिः->content;
	}

	प्रत्यागम रिक्त;
}

/* See hlcache.h for documentation */
nserror hlcache_handle_abort(hlcache_handle *handle)
{
	संरचना hlcache_प्रविष्टिः *प्रविष्टिः = handle->प्रविष्टिः;
	संरचना content *c;

	यदि (प्रविष्टिः == रिक्त) {
		/* This handle is not yet associated with a cache entry.
		 * The implication is that the fetch for the handle has
		 * not progressed to the point where the entry can be
		 * created. */

		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			यदि (ictx->handle == handle &&
					ictx->migrate_target == false) {
				/* This is the nascent context for us,
				 * so abort the fetch */
				llcache_handle_abort(ictx->llcache);
				llcache_handle_release(ictx->llcache);
				/* Remove us from the ring */
				RING_REMOVE(hlcache->retrieval_ctx_ring, ictx);
				/* Throw us away */
				मुक्त((वर्ण *) ictx->child.charset);
				मुक्त(ictx);
				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_अन्तः(hlcache->retrieval_ctx_ring, ictx);

		प्रत्यागम NSERROR_OK;
	}

	c = प्रविष्टिः->content;

	यदि (content_गणनम्_users(c) > 1) {
		/* We are not the only user of 'c' so clone it. */
		संरचना content *clone = content_clone(c);

		यदि (clone == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		प्रविष्टिः = शून्यस्मृतिं_देहि(1, परिमाण(संरचना hlcache_प्रविष्टिः));

		यदि (प्रविष्टिः == रिक्त) {
			content_destroy(clone);
			प्रत्यागम NSERROR_NOMEM;
		}

		यदि (content_सङ्कलय_user(clone,
				hlcache_content_callback, handle) == false) {
			content_destroy(clone);
			मुक्त(प्रविष्टिः);
			प्रत्यागम NSERROR_NOMEM;
		}

		content_remove_user(c, hlcache_content_callback, handle);

		प्रविष्टिः->content = clone;
		handle->प्रविष्टिः = प्रविष्टिः;
		प्रविष्टिः->पूर्व = रिक्त;
		प्रविष्टिः->अग्र = hlcache->content_list;
		यदि (hlcache->content_list != रिक्त)
			hlcache->content_list->पूर्व = प्रविष्टिः;
		hlcache->content_list = प्रविष्टिः;

		c = clone;
	}

	प्रत्यागम content_abort(c);
}

/* See hlcache.h for documentation */
nserror hlcache_handle_replace_callback(hlcache_handle *handle,
		hlcache_handle_callback cb, शून्य *pw)
{
	handle->cb = cb;
	handle->pw = pw;

	प्रत्यागम NSERROR_OK;
}

nserror hlcache_handle_clone(hlcache_handle *handle, hlcache_handle **परिणाम)
{
	*परिणाम = रिक्त;
	प्रत्यागम NSERROR_CLONE_FAILED;
}

/* See hlcache.h for documentation */
nsurl *hlcache_handle_get_url(ध्रुव hlcache_handle *handle)
{
	nsurl *परिणाम = रिक्त;

	प्रतिज्ञा(handle != रिक्त);

	यदि (handle->प्रविष्टिः != रिक्त) {
		परिणाम = content_get_url(handle->प्रविष्टिः->content);
	} अन्यथा {
		RING_ITERATE_START(संरचना hlcache_retrieval_ctx,
				   hlcache->retrieval_ctx_ring,
				   ictx) {
			यदि (ictx->handle == handle) {
				/* This is the nascent context for us */
				परिणाम = llcache_handle_get_url(ictx->llcache);

				/* And stop */
				RING_ITERATE_STOP(hlcache->retrieval_ctx_ring,
						ictx);
			}
		} RING_ITERATE_अन्तः(hlcache->retrieval_ctx_ring, ictx);
	}

	प्रत्यागम परिणाम;
}
