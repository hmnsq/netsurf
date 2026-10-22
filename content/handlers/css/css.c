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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ضمن <string.h>
#ضمن <assert.h>
#ضمن <libwapcaplet/libwapcaplet.h>
#ضمن <dom/dom.h>

#ضمن "utils/errors.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/utils.h"
#ضمن "utils/http.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "content/fetch.h"
#ضمن "content/hlcache.h"
#ضمن "desktop/system_colour.h"

#ضمن "css/css.h"
#ضمن "css/hints.h"
#ضمن "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

بنية content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
تعريفنوع فراغ (*nscss_done_callback)(بنية content_css_data *css, فراغ *pw);

/**
 * CSS content data
 */
بنية content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	حرف *charset;			/**< Character set of stylesheet */
	بنية nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_count;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	فراغ *pw;			/**< Client data */
};

/**
 * CSS content data
 */
تعريفنوع بنية nscss_content
{
	بنية content base;		/**< Underlying content object */

	بنية content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
تعريفنوع بنية {
	بنية content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ساكن bool nscss_convert(بنية content *c);
ساكن فراغ nscss_destroy(بنية content *c);
ساكن nserror nscss_clone(ثابت بنية content *old, بنية content **newc);
ساكن bool nscss_matches_quirks(ثابت بنية content *c, bool quirks);
ساكن content_type nscss_content_type(فراغ);

ساكن nserror nscss_create_css_data(بنية content_css_data *c,
		ثابت حرف *url, ثابت حرف *charset, bool quirks,
		nscss_done_callback done, فراغ *pw);
ساكن css_error nscss_process_css_data(بنية content_css_data *c, ثابت حرف *data,
		unsigned صحيح size);
ساكن css_error nscss_convert_css_data(بنية content_css_data *c);
ساكن فراغ nscss_destroy_css_data(بنية content_css_data *c);

ساكن فراغ nscss_content_done(بنية content_css_data *css, فراغ *pw);
ساكن css_error nscss_handle_import(فراغ *pw, css_stylesheet *parent,
		lwc_string *url);
ساكن nserror nscss_import(hlcache_handle *handle,
		ثابت hlcache_event *event, فراغ *pw);
ساكن css_error nscss_import_complete(nscss_import_ctx *ctx);

ساكن css_error nscss_register_imports(بنية content_css_data *c);
ساكن css_error nscss_register_import(بنية content_css_data *c,
		ثابت hlcache_handle *import);


ساكن css_stylesheet *blank_import;


/**
 * Initialise a CSS content
 *
 * \param handler content handler
 * \param imime_type mime-type
 * \param params Content-Type parameters
 * \param llcache handle to content
 * \param fallback_charset The character set to fallback to.
 * \param quirks allow quirks
 * \param c Content to initialise
 * \return NSERROR_OK or error cod eon faliure
 */
ساكن nserror
nscss_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت http_parameter *params,
	     llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     بنية content **c)
{
	nscss_content *result;
	ثابت حرف *charset = عدم;
	ثابت حرف *xnsbase = عدم;
	lwc_string *charset_value = عدم;
	nserror error;

	result = calloc(1, حجم(nscss_content));
	𐤀𐤌 (result == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	𐤀𐤌 (error != NSERROR_OK) {
		حرر(result);
		أرجع error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_value);
	𐤀𐤌 (error != NSERROR_OK || lwc_string_length(charset_value) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} وإلا {
		charset = lwc_string_data(charset_value);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	𐤀𐤌 (xnsbase == عدم) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	𐤀𐤌 (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, عدم);
		lwc_string_unref(charset_value);
		حرر(result);
		أرجع error;
	}

	lwc_string_unref(charset_value);

	*c = (بنية content *) result;

	أرجع NSERROR_OK;
}

/**
 * Create a struct content_css_data, creating a stylesheet object
 *
 * \param c        Struct to populate
 * \param url      URL of stylesheet
 * \param charset  Stylesheet charset
 * \param quirks   Stylesheet quirks mode
 * \param done     Callback to call when content has completed
 * \param pw       Client data for \a done
 * \return NSERROR_OK on success, NSERROR_NOMEM on memory exhaustion
 */
ساكن nserror nscss_create_css_data(بنية content_css_data *c,
		ثابت حرف *url, ثابت حرف *charset, bool quirks,
		nscss_done_callback done, فراغ *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_count = 0;
	c->imports = عدم;
	𐤀𐤌 (charset != عدم)
		c->charset = strdup(charset);
	وإلا
		c->charset = عدم;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = عدم;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = عدم;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = عدم;
	params.font = عدم;
	params.font_pw = عدم;

	error = css_stylesheet_create(&params, &c->sheet);
	𐤀𐤌 (error != CSS_OK) {
		أرجع NSERROR_NOMEM;
	}

	أرجع NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ساكن bool
nscss_process_data(بنية content *c, ثابت حرف *data, unsigned صحيح size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	𐤀𐤌 (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, عدم);
	}

	أرجع (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ساكن css_error nscss_process_css_data(بنية content_css_data *c,
		ثابت حرف *data, unsigned صحيح size)
{
	أرجع css_stylesheet_append_data(c->sheet,
			(ثابت uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(بنية content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	𐤀𐤌 (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, عدم);
		أرجع false;
	}

	أرجع true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ساكن css_error nscss_convert_css_data(بنية content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	𐤀𐤌 (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		أكد(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} وإلا 𐤀𐤌 (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} وإلا {
		ثابت حرف *url;

		𐤀𐤌 (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} وإلا {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	أرجع error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
فراغ nscss_destroy(بنية content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ساكن فراغ nscss_destroy_css_data(بنية content_css_data *c)
{
	uint32_t i;

	لكل (i = 0; i < c->import_count; i++) {
		𐤀𐤌 (c->imports[i].c != عدم) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = عدم;
	}

	حرر(c->imports);

	𐤀𐤌 (c->sheet != عدم) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = عدم;
	}

	حرر(c->charset);
}

nserror nscss_clone(ثابت بنية content *old, بنية content **newc)
{
	ثابت nscss_content *old_css = (ثابت nscss_content *) old;
	nscss_content *new_css;
	ثابت uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, حجم(nscss_content));
	𐤀𐤌 (new_css == عدم)
		أرجع NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		أرجع error;
	}

	data = content__get_source_data(&new_css->base, &size);
	𐤀𐤌 (size > 0) {
		𐤀𐤌 (nscss_process_data(&new_css->base,
				       (حرف *)data,
				       (unsigned صحيح)size) == false) {
			content_destroy(&new_css->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	𐤀𐤌 (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		𐤀𐤌 (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) new_css;

	أرجع NSERROR_OK;
}

bool nscss_matches_quirks(ثابت بنية content *c, bool quirks)
{
	أرجع c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(بنية hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
بنية nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->data.import_count;

	أرجع c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(فراغ)
{
	أرجع CONTENT_CSS;
}

/*****************************************************************************
 * Object completion                                                         *
 *****************************************************************************/

/**
 * Handle notification that a CSS object is done
 *
 * \param css  CSS object
 * \param pw   Private data
 */
فراغ nscss_content_done(بنية content_css_data *css, فراغ *pw)
{
	بنية content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	𐤀𐤌 (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, عدم);
		content_set_error(c);
		أرجع;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	لكل (i = 0; i < css->import_count; i++) {
		𐤀𐤌 (css->imports[i].c != عدم) {
			بنية content *import = hlcache_handle_get_content(
					css->imports[i].c);

			𐤀𐤌 (import != عدم) {
				c->size += import->size;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_set_ready(c);
	content_set_done(c);
}

/*****************************************************************************
 * Import handling                                                           *
 *****************************************************************************/

/**
 * Handle notification of the need for an imported stylesheet
 *
 * \param pw      CSS object requesting the import
 * \param parent  Stylesheet requesting the import
 * \param url     URL of the imported sheet
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_handle_import(فراغ *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	بنية content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	بنية nscss_import *imports;
	ثابت حرف *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	أكد(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	𐤀𐤌 (error != CSS_OK) {
		أرجع error;
	}

	ctx = malloc(حجم(*ctx));
	𐤀𐤌 (ctx == عدم)
		أرجع CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_count;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_count + 1) *
			حجم(بنية nscss_import));
	𐤀𐤌 (imports == عدم) {
		حرر(ctx);
		أرجع CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = عدم;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	𐤀𐤌 (error != CSS_OK) {
		حرر(ctx);
		أرجع error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	𐤀𐤌 (nerror != NSERROR_OK) {
		حرر(ctx);
		أرجع CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	𐤀𐤌 (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		حرر(ctx);
		أرجع CSS_NOMEM;
	}

	/* Avoid importing ourself */
	𐤀𐤌 (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_count].c = عدم;
		/* No longer require context as we're not fetching anything */
		حرر(ctx);
		ctx = عدم;
	} وإلا {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, عدم, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_count].c);
		𐤀𐤌 (nerror != NSERROR_OK) {
			حرر(ctx);
			أرجع CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_count, lwc_string_data(url),
	      c->imports[c->import_count].c, ctx);
#endif

	c->import_count++;

	أرجع CSS_OK;
}

/**
 * Handler for imported stylesheet events
 *
 * \param handle  Handle for stylesheet
 * \param event   Event object
 * \param pw      Callback context
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror nscss_import(hlcache_handle *handle,
		ثابت hlcache_event *event, فراغ *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	أكد(ctx->css->imports[ctx->index].c == handle);

	بدل (event->type) {
	حالة CONTENT_MSG_DONE:
		error = nscss_import_complete(ctx);
		اكسر;

	حالة CONTENT_MSG_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = عدم;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		اكسر;
	افتراضي:
		اكسر;
	}

	/* Preserve out-of-memory. Anything else is OK */
	أرجع error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
}

/**
 * Handle an imported stylesheet completing
 *
 * \param ctx  Import context
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_import_complete(nscss_import_ctx *ctx)
{
	css_error error = CSS_OK;

	/* If this import is the next to be registered, do so */
	𐤀𐤌 (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	حرر(ctx);

	أرجع error;
}

/*****************************************************************************
 * Import registration                                                       *
 *****************************************************************************/

/**
 * Register imports with a stylesheet
 *
 * \param c  CSS object containing the imports
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_imports(بنية content_css_data *c)
{
	uint32_t index;
	css_error error;

	أكد(c->next_to_register != (uint32_t) -1);
	أكد(c->next_to_register < c->import_count);

	/* Register imported sheets */
	لكل (index = c->next_to_register; index < c->import_count; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		𐤀𐤌 (c->imports[index].c != عدم &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			اكسر;
		}

		error = nscss_register_import(c, c->imports[index].c);
		𐤀𐤌 (error != CSS_OK)
			أرجع error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	𐤀𐤌 (c->next_to_register == c->import_count) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	أرجع CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(بنية content_css_data *c,
		ثابت hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	𐤀𐤌 (import != عدم) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} وإلا {
		/* Create a blank sheet if needed. */
		𐤀𐤌 (blank_import == عدم) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = عدم;
			params.url = "";
			params.title = عدم;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = عدم;
			params.import = عدم;
			params.import_pw = عدم;
			params.color = ns_system_colour;
			params.color_pw = عدم;
			params.font = عدم;
			params.font_pw = عدم;

			error = css_stylesheet_create(&params, &blank_import);
			𐤀𐤌 (error != CSS_OK) {
				أرجع error;
			}

			error = css_stylesheet_data_done(blank_import);
			𐤀𐤌 (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				أرجع error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	𐤀𐤌 (error != CSS_OK) {
		أرجع error;
	}

	أرجع error;
}

/**
 * Clean up after the CSS content handler
 */
ساكن فراغ nscss_fini(فراغ)
{
	𐤀𐤌 (blank_import != عدم) {
		css_stylesheet_destroy(blank_import);
		blank_import = عدم;
	}
	css_hint_fini();
}

ساكن ثابت content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_data = nscss_process_data,
	.data_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(فراغ)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	𐤀𐤌 (error != NSERROR_OK)
		اذهب error;

	error = css_hint_init();
	𐤀𐤌 (error != NSERROR_OK)
		اذهب error;

	أرجع NSERROR_OK;

error:
	nscss_fini();

	أرجع error;
}
