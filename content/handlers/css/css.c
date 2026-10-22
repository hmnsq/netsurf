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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "desktop/system_colour.h"

#அந்தர்பாவ "css/css.h"
#அந்தர்பாவ "css/hints.h"
#அந்தர்பாவ "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

கட்டமைப்பு content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
வகைவரை வெறுமை (*nscss_done_callback)(கட்டமைப்பு content_css_data *css, வெறுமை *pw);

/**
 * CSS content data
 */
கட்டமைப்பு content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	எழுத்து *charset;			/**< Character set of stylesheet */
	கட்டமைப்பு nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_மொத்தம்;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	வெறுமை *pw;			/**< Client data */
};

/**
 * CSS content data
 */
வகைவரை கட்டமைப்பு nscss_content
{
	கட்டமைப்பு content base;		/**< Underlying content object */

	கட்டமைப்பு content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
வகைவரை கட்டமைப்பு {
	கட்டமைப்பு content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

நிலையான bool nscss_convert(கட்டமைப்பு content *c);
நிலையான வெறுமை nscss_destroy(கட்டமைப்பு content *c);
நிலையான nserror nscss_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc);
நிலையான bool nscss_matches_quirks(மாறிலி கட்டமைப்பு content *c, bool quirks);
நிலையான content_type nscss_content_type(வெறுமை);

நிலையான nserror nscss_create_css_data(கட்டமைப்பு content_css_data *c,
		மாறிலி எழுத்து *url, மாறிலி எழுத்து *charset, bool quirks,
		nscss_done_callback done, வெறுமை *pw);
நிலையான css_error nscss_process_css_data(கட்டமைப்பு content_css_data *c, மாறிலி எழுத்து *data,
		unsigned முழு size);
நிலையான css_error nscss_convert_css_data(கட்டமைப்பு content_css_data *c);
நிலையான வெறுமை nscss_destroy_css_data(கட்டமைப்பு content_css_data *c);

நிலையான வெறுமை nscss_content_done(கட்டமைப்பு content_css_data *css, வெறுமை *pw);
நிலையான css_error nscss_handle_import(வெறுமை *pw, css_stylesheet *parent,
		lwc_string *url);
நிலையான nserror nscss_import(hlcache_handle *handle,
		மாறிலி hlcache_event *event, வெறுமை *pw);
நிலையான css_error nscss_import_complete(nscss_import_ctx *ctx);

நிலையான css_error nscss_register_imports(கட்டமைப்பு content_css_data *c);
நிலையான css_error nscss_register_import(கட்டமைப்பு content_css_data *c,
		மாறிலி hlcache_handle *import);


நிலையான css_stylesheet *blank_import;


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
நிலையான nserror
nscss_create(மாறிலி content_handler *handler,
	     lwc_string *imime_type,
	     மாறிலி http_parameter *params,
	     llcache_handle *llcache,
	     மாறிலி எழுத்து *fallback_charset,
	     bool quirks,
	     கட்டமைப்பு content **c)
{
	nscss_content *result;
	மாறிலி எழுத்து *charset = வெற்று;
	மாறிலி எழுத்து *xnsbase = வெற்று;
	lwc_string *charset_மதிப்பு = வெற்று;
	nserror error;

	result = calloc(1, அளவு(nscss_content));
	எனில் (result == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(result);
		திரும்பு error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_மதிப்பு);
	எனில் (error != NSERROR_OK || lwc_string_length(charset_மதிப்பு) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} இல்லையெனில் {
		charset = lwc_string_data(charset_மதிப்பு);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	எனில் (xnsbase == வெற்று) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	எனில் (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, வெற்று);
		lwc_string_unref(charset_மதிப்பு);
		விடுவி(result);
		திரும்பு error;
	}

	lwc_string_unref(charset_மதிப்பு);

	*c = (கட்டமைப்பு content *) result;

	திரும்பு NSERROR_OK;
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
நிலையான nserror nscss_create_css_data(கட்டமைப்பு content_css_data *c,
		மாறிலி எழுத்து *url, மாறிலி எழுத்து *charset, bool quirks,
		nscss_done_callback done, வெறுமை *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_மொத்தம் = 0;
	c->imports = வெற்று;
	எனில் (charset != வெற்று)
		c->charset = strdup(charset);
	இல்லையெனில்
		c->charset = வெற்று;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = வெற்று;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = வெற்று;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = வெற்று;
	params.font = வெற்று;
	params.font_pw = வெற்று;

	error = css_stylesheet_create(&params, &c->sheet);
	எனில் (error != CSS_OK) {
		திரும்பு NSERROR_NOMEM;
	}

	திரும்பு NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
நிலையான bool
nscss_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data, unsigned முழு size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	எனில் (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, வெற்று);
	}

	திரும்பு (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
நிலையான css_error nscss_process_css_data(கட்டமைப்பு content_css_data *c,
		மாறிலி எழுத்து *data, unsigned முழு size)
{
	திரும்பு css_stylesheet_append_data(c->sheet,
			(மாறிலி uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(கட்டமைப்பு content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	எனில் (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, வெற்று);
		திரும்பு false;
	}

	திரும்பு true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
நிலையான css_error nscss_convert_css_data(கட்டமைப்பு content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	எனில் (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ப்ரதிஜ்ஞா(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} இல்லையெனில் எனில் (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} இல்லையெனில் {
		மாறிலி எழுத்து *url;

		எனில் (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} இல்லையெனில் {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	திரும்பு error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
வெறுமை nscss_destroy(கட்டமைப்பு content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
நிலையான வெறுமை nscss_destroy_css_data(கட்டமைப்பு content_css_data *c)
{
	uint32_t i;

	க்கு (i = 0; i < c->import_மொத்தம்; i++) {
		எனில் (c->imports[i].c != வெற்று) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = வெற்று;
	}

	விடுவி(c->imports);

	எனில் (c->sheet != வெற்று) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = வெற்று;
	}

	விடுவி(c->charset);
}

nserror nscss_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	மாறிலி nscss_content *old_css = (மாறிலி nscss_content *) old;
	nscss_content *new_css;
	மாறிலி uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, அளவு(nscss_content));
	எனில் (new_css == வெற்று)
		திரும்பு NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		திரும்பு error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	எனில் (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		திரும்பு error;
	}

	data = content__get_source_data(&new_css->base, &size);
	எனில் (size > 0) {
		எனில் (nscss_process_data(&new_css->base,
				       (எழுத்து *)data,
				       (unsigned முழு)size) == false) {
			content_destroy(&new_css->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	எனில் (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		எனில் (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) new_css;

	திரும்பு NSERROR_OK;
}

bool nscss_matches_quirks(மாறிலி கட்டமைப்பு content *c, bool quirks)
{
	திரும்பு c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(கட்டமைப்பு hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
கட்டமைப்பு nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);
	ப்ரதிஜ்ஞா(n != வெற்று);

	*n = c->data.import_மொத்தம்;

	திரும்பு c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(வெறுமை)
{
	திரும்பு CONTENT_CSS;
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
வெறுமை nscss_content_done(கட்டமைப்பு content_css_data *css, வெறுமை *pw)
{
	கட்டமைப்பு content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	எனில் (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, வெற்று);
		content_set_error(c);
		திரும்பு;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	க்கு (i = 0; i < css->import_மொத்தம்; i++) {
		எனில் (css->imports[i].c != வெற்று) {
			கட்டமைப்பு content *import = hlcache_handle_get_content(
					css->imports[i].c);

			எனில் (import != வெற்று) {
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
css_error nscss_handle_import(வெறுமை *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	கட்டமைப்பு content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	கட்டமைப்பு nscss_import *imports;
	மாறிலி எழுத்து *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ப்ரதிஜ்ஞா(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	எனில் (error != CSS_OK) {
		திரும்பு error;
	}

	ctx = malloc(அளவு(*ctx));
	எனில் (ctx == வெற்று)
		திரும்பு CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_மொத்தம்;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_மொத்தம் + 1) *
			அளவு(கட்டமைப்பு nscss_import));
	எனில் (imports == வெற்று) {
		விடுவி(ctx);
		திரும்பு CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = வெற்று;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	எனில் (error != CSS_OK) {
		விடுவி(ctx);
		திரும்பு error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	எனில் (nerror != NSERROR_OK) {
		விடுவி(ctx);
		திரும்பு CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	எனில் (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		விடுவி(ctx);
		திரும்பு CSS_NOMEM;
	}

	/* Avoid importing ourself */
	எனில் (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_மொத்தம்].c = வெற்று;
		/* No longer require context as we're not fetching anything */
		விடுவி(ctx);
		ctx = வெற்று;
	} இல்லையெனில் {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, வெற்று, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_மொத்தம்].c);
		எனில் (nerror != NSERROR_OK) {
			விடுவி(ctx);
			திரும்பு CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_மொத்தம், lwc_string_data(url),
	      c->imports[c->import_மொத்தம்].c, ctx);
#endif

	c->import_மொத்தம்++;

	திரும்பு CSS_OK;
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
		மாறிலி hlcache_event *event, வெறுமை *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	ப்ரதிஜ்ஞா(ctx->css->imports[ctx->index].c == handle);

	தேர்வு (event->type) {
	நிலை CONTENT_செய்தி_DONE:
		error = nscss_import_complete(ctx);
		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = வெற்று;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* Preserve out-of-memory. Anything else is OK */
	திரும்பு error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	எனில் (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	விடுவி(ctx);

	திரும்பு error;
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
css_error nscss_register_imports(கட்டமைப்பு content_css_data *c)
{
	uint32_t index;
	css_error error;

	ப்ரதிஜ்ஞா(c->next_to_register != (uint32_t) -1);
	ப்ரதிஜ்ஞா(c->next_to_register < c->import_மொத்தம்);

	/* Register imported sheets */
	க்கு (index = c->next_to_register; index < c->import_மொத்தம்; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		எனில் (c->imports[index].c != வெற்று &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			நிறுத்து;
		}

		error = nscss_register_import(c, c->imports[index].c);
		எனில் (error != CSS_OK)
			திரும்பு error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	எனில் (c->next_to_register == c->import_மொத்தம்) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	திரும்பு CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(கட்டமைப்பு content_css_data *c,
		மாறிலி hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	எனில் (import != வெற்று) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} இல்லையெனில் {
		/* Create a blank sheet if needed. */
		எனில் (blank_import == வெற்று) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = வெற்று;
			params.url = "";
			params.title = வெற்று;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = வெற்று;
			params.import = வெற்று;
			params.import_pw = வெற்று;
			params.color = ns_system_colour;
			params.color_pw = வெற்று;
			params.font = வெற்று;
			params.font_pw = வெற்று;

			error = css_stylesheet_create(&params, &blank_import);
			எனில் (error != CSS_OK) {
				திரும்பு error;
			}

			error = css_stylesheet_data_done(blank_import);
			எனில் (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				திரும்பு error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	எனில் (error != CSS_OK) {
		திரும்பு error;
	}

	திரும்பு error;
}

/**
 * Clean up after the CSS content handler
 */
நிலையான வெறுமை nscss_fini(வெறுமை)
{
	எனில் (blank_import != வெற்று) {
		css_stylesheet_destroy(blank_import);
		blank_import = வெற்று;
	}
	css_hint_fini();
}

நிலையான மாறிலி content_handler css_content_handler = {
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
nserror nscss_init(வெறுமை)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	எனில் (error != NSERROR_OK)
		செல் error;

	error = css_hint_init();
	எனில் (error != NSERROR_OK)
		செல் error;

	திரும்பு NSERROR_OK;

error:
	nscss_fini();

	திரும்பு error;
}
