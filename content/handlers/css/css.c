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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "desktop/system_colour.h"

#അന്തര്ഭാവ "css/css.h"
#അന്തര്ഭാവ "css/hints.h"
#അന്തര്ഭാവ "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ഘടന content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
തരനിർവചനം ശൂന്യം (*nscss_done_callback)(ഘടന content_css_data *css, ശൂന്യം *pw);

/**
 * CSS content data
 */
ഘടന content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	അക്ഷരം *charset;			/**< Character set of stylesheet */
	ഘടന nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_എണ്ണം;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	ശൂന്യം *pw;			/**< Client data */
};

/**
 * CSS content data
 */
തരനിർവചനം ഘടന nscss_content
{
	ഘടന content base;		/**< Underlying content object */

	ഘടന content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
തരനിർവചനം ഘടന {
	ഘടന content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

സ്ഥിതം bool nscss_convert(ഘടന content *c);
സ്ഥിതം ശൂന്യം nscss_destroy(ഘടന content *c);
സ്ഥിതം nserror nscss_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc);
സ്ഥിതം bool nscss_matches_quirks(സ്ഥിരം ഘടന content *c, bool quirks);
സ്ഥിതം content_type nscss_content_type(ശൂന്യം);

സ്ഥിതം nserror nscss_create_css_data(ഘടന content_css_data *c,
		സ്ഥിരം അക്ഷരം *url, സ്ഥിരം അക്ഷരം *charset, bool quirks,
		nscss_done_callback done, ശൂന്യം *pw);
സ്ഥിതം css_error nscss_process_css_data(ഘടന content_css_data *c, സ്ഥിരം അക്ഷരം *data,
		unsigned പൂർണ്ണസംഖ്യ size);
സ്ഥിതം css_error nscss_convert_css_data(ഘടന content_css_data *c);
സ്ഥിതം ശൂന്യം nscss_destroy_css_data(ഘടന content_css_data *c);

സ്ഥിതം ശൂന്യം nscss_content_done(ഘടന content_css_data *css, ശൂന്യം *pw);
സ്ഥിതം css_error nscss_handle_import(ശൂന്യം *pw, css_stylesheet *parent,
		lwc_string *url);
സ്ഥിതം nserror nscss_import(hlcache_handle *handle,
		സ്ഥിരം hlcache_event *event, ശൂന്യം *pw);
സ്ഥിതം css_error nscss_import_complete(nscss_import_ctx *ctx);

സ്ഥിതം css_error nscss_register_imports(ഘടന content_css_data *c);
സ്ഥിതം css_error nscss_register_import(ഘടന content_css_data *c,
		സ്ഥിരം hlcache_handle *import);


സ്ഥിതം css_stylesheet *blank_import;


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
സ്ഥിതം nserror
nscss_create(സ്ഥിരം content_handler *handler,
	     lwc_string *imime_type,
	     സ്ഥിരം http_parameter *params,
	     llcache_handle *llcache,
	     സ്ഥിരം അക്ഷരം *fallback_charset,
	     bool quirks,
	     ഘടന content **c)
{
	nscss_content *result;
	സ്ഥിരം അക്ഷരം *charset = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *xnsbase = ശൂന്യ;
	lwc_string *charset_മൂല്യം = ശൂന്യ;
	nserror error;

	result = calloc(1, വലുപ്പം(nscss_content));
	എങ്കിൽ (result == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_മൂല്യം);
	എങ്കിൽ (error != NSERROR_OK || lwc_string_length(charset_മൂല്യം) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} അല്ലെങ്കിൽ {
		charset = lwc_string_data(charset_മൂല്യം);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	എങ്കിൽ (xnsbase == ശൂന്യ) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	എങ്കിൽ (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, ശൂന്യ);
		lwc_string_unref(charset_മൂല്യം);
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	lwc_string_unref(charset_മൂല്യം);

	*c = (ഘടന content *) result;

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror nscss_create_css_data(ഘടന content_css_data *c,
		സ്ഥിരം അക്ഷരം *url, സ്ഥിരം അക്ഷരം *charset, bool quirks,
		nscss_done_callback done, ശൂന്യം *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_എണ്ണം = 0;
	c->imports = ശൂന്യ;
	എങ്കിൽ (charset != ശൂന്യ)
		c->charset = strdup(charset);
	അല്ലെങ്കിൽ
		c->charset = ശൂന്യ;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ശൂന്യ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ശൂന്യ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = ശൂന്യ;
	params.font = ശൂന്യ;
	params.font_pw = ശൂന്യ;

	error = css_stylesheet_create(&params, &c->sheet);
	എങ്കിൽ (error != CSS_OK) {
		തിരികെ NSERROR_NOMEM;
	}

	തിരികെ NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
സ്ഥിതം bool
nscss_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data, unsigned പൂർണ്ണസംഖ്യ size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	എങ്കിൽ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, ശൂന്യ);
	}

	തിരികെ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
സ്ഥിതം css_error nscss_process_css_data(ഘടന content_css_data *c,
		സ്ഥിരം അക്ഷരം *data, unsigned പൂർണ്ണസംഖ്യ size)
{
	തിരികെ css_stylesheet_append_data(c->sheet,
			(സ്ഥിരം uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ഘടന content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	എങ്കിൽ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ശൂന്യ);
		തിരികെ false;
	}

	തിരികെ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
സ്ഥിതം css_error nscss_convert_css_data(ഘടന content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	എങ്കിൽ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		പ്രതിജ്ഞാ(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} അല്ലെങ്കിൽ എങ്കിൽ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} അല്ലെങ്കിൽ {
		സ്ഥിരം അക്ഷരം *url;

		എങ്കിൽ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} അല്ലെങ്കിൽ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	തിരികെ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
ശൂന്യം nscss_destroy(ഘടന content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
സ്ഥിതം ശൂന്യം nscss_destroy_css_data(ഘടന content_css_data *c)
{
	uint32_t i;

	വേണ്ടി (i = 0; i < c->import_എണ്ണം; i++) {
		എങ്കിൽ (c->imports[i].c != ശൂന്യ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ശൂന്യ;
	}

	സ്വതന്ത്രം(c->imports);

	എങ്കിൽ (c->sheet != ശൂന്യ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ശൂന്യ;
	}

	സ്വതന്ത്രം(c->charset);
}

nserror nscss_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	സ്ഥിരം nscss_content *old_css = (സ്ഥിരം nscss_content *) old;
	nscss_content *new_css;
	സ്ഥിരം uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, വലുപ്പം(nscss_content));
	എങ്കിൽ (new_css == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		തിരികെ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		തിരികെ error;
	}

	data = content__get_source_data(&new_css->base, &size);
	എങ്കിൽ (size > 0) {
		എങ്കിൽ (nscss_process_data(&new_css->base,
				       (അക്ഷരം *)data,
				       (unsigned പൂർണ്ണസംഖ്യ)size) == false) {
			content_destroy(&new_css->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) new_css;

	തിരികെ NSERROR_OK;
}

bool nscss_matches_quirks(സ്ഥിരം ഘടന content *c, bool quirks)
{
	തിരികെ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ഘടന hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
ഘടന nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);
	പ്രതിജ്ഞാ(n != ശൂന്യ);

	*n = c->data.import_എണ്ണം;

	തിരികെ c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_CSS;
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
ശൂന്യം nscss_content_done(ഘടന content_css_data *css, ശൂന്യം *pw)
{
	ഘടന content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	എങ്കിൽ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ശൂന്യ);
		content_set_error(c);
		തിരികെ;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	വേണ്ടി (i = 0; i < css->import_എണ്ണം; i++) {
		എങ്കിൽ (css->imports[i].c != ശൂന്യ) {
			ഘടന content *import = hlcache_handle_get_content(
					css->imports[i].c);

			എങ്കിൽ (import != ശൂന്യ) {
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
css_error nscss_handle_import(ശൂന്യം *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ഘടന content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	ഘടന nscss_import *imports;
	സ്ഥിരം അക്ഷരം *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	പ്രതിജ്ഞാ(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	എങ്കിൽ (error != CSS_OK) {
		തിരികെ error;
	}

	ctx = malloc(വലുപ്പം(*ctx));
	എങ്കിൽ (ctx == ശൂന്യ)
		തിരികെ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_എണ്ണം;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_എണ്ണം + 1) *
			വലുപ്പം(ഘടന nscss_import));
	എങ്കിൽ (imports == ശൂന്യ) {
		സ്വതന്ത്രം(ctx);
		തിരികെ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = ശൂന്യ;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	എങ്കിൽ (error != CSS_OK) {
		സ്വതന്ത്രം(ctx);
		തിരികെ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	എങ്കിൽ (nerror != NSERROR_OK) {
		സ്വതന്ത്രം(ctx);
		തിരികെ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	എങ്കിൽ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		സ്വതന്ത്രം(ctx);
		തിരികെ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	എങ്കിൽ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_എണ്ണം].c = ശൂന്യ;
		/* No longer require context as we're not fetching anything */
		സ്വതന്ത്രം(ctx);
		ctx = ശൂന്യ;
	} അല്ലെങ്കിൽ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ശൂന്യ, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_എണ്ണം].c);
		എങ്കിൽ (nerror != NSERROR_OK) {
			സ്വതന്ത്രം(ctx);
			തിരികെ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_എണ്ണം, lwc_string_data(url),
	      c->imports[c->import_എണ്ണം].c, ctx);
#endif

	c->import_എണ്ണം++;

	തിരികെ CSS_OK;
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
		സ്ഥിരം hlcache_event *event, ശൂന്യം *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	പ്രതിജ്ഞാ(ctx->css->imports[ctx->index].c == handle);

	തിരഞ്ഞെടുപ്പ് (event->type) {
	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		error = nscss_import_complete(ctx);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ശൂന്യ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* Preserve out-of-memory. Anything else is OK */
	തിരികെ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	എങ്കിൽ (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	സ്വതന്ത്രം(ctx);

	തിരികെ error;
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
css_error nscss_register_imports(ഘടന content_css_data *c)
{
	uint32_t index;
	css_error error;

	പ്രതിജ്ഞാ(c->next_to_register != (uint32_t) -1);
	പ്രതിജ്ഞാ(c->next_to_register < c->import_എണ്ണം);

	/* Register imported sheets */
	വേണ്ടി (index = c->next_to_register; index < c->import_എണ്ണം; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		എങ്കിൽ (c->imports[index].c != ശൂന്യ &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			നിർത്തുക;
		}

		error = nscss_register_import(c, c->imports[index].c);
		എങ്കിൽ (error != CSS_OK)
			തിരികെ error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	എങ്കിൽ (c->next_to_register == c->import_എണ്ണം) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	തിരികെ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(ഘടന content_css_data *c,
		സ്ഥിരം hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	എങ്കിൽ (import != ശൂന്യ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} അല്ലെങ്കിൽ {
		/* Create a blank sheet if needed. */
		എങ്കിൽ (blank_import == ശൂന്യ) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ശൂന്യ;
			params.url = "";
			params.title = ശൂന്യ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ശൂന്യ;
			params.import = ശൂന്യ;
			params.import_pw = ശൂന്യ;
			params.color = ns_system_colour;
			params.color_pw = ശൂന്യ;
			params.font = ശൂന്യ;
			params.font_pw = ശൂന്യ;

			error = css_stylesheet_create(&params, &blank_import);
			എങ്കിൽ (error != CSS_OK) {
				തിരികെ error;
			}

			error = css_stylesheet_data_done(blank_import);
			എങ്കിൽ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				തിരികെ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	എങ്കിൽ (error != CSS_OK) {
		തിരികെ error;
	}

	തിരികെ error;
}

/**
 * Clean up after the CSS content handler
 */
സ്ഥിതം ശൂന്യം nscss_fini(ശൂന്യം)
{
	എങ്കിൽ (blank_import != ശൂന്യ) {
		css_stylesheet_destroy(blank_import);
		blank_import = ശൂന്യ;
	}
	css_hint_fini();
}

സ്ഥിതം സ്ഥിരം content_handler css_content_handler = {
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
nserror nscss_init(ശൂന്യം)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	എങ്കിൽ (error != NSERROR_OK)
		പോകുക error;

	error = css_hint_init();
	എങ്കിൽ (error != NSERROR_OK)
		പോകുക error;

	തിരികെ NSERROR_OK;

error:
	nscss_fini();

	തിരികെ error;
}
