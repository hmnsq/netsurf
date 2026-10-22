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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/http.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "desktop/system_colour.h"

#অন্তর্ভাব "css/css.h"
#অন্তর্ভাব "css/hints.h"
#অন্তর্ভাব "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

সংরচনা content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
প্রকারনির্দেশ শূন্য (*nscss_done_callback)(সংরচনা content_css_data *css, শূন্য *pw);

/**
 * CSS content data
 */
সংরচনা content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	বর্ণ *charset;			/**< Character set of stylesheet */
	সংরচনা nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_গুনতি;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	শূন্য *pw;			/**< Client data */
};

/**
 * CSS content data
 */
প্রকারনির্দেশ সংরচনা nscss_content
{
	সংরচনা content base;		/**< Underlying content object */

	সংরচনা content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
প্রকারনির্দেশ সংরচনা {
	সংরচনা content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

স্থির bool nscss_convert(সংরচনা content *c);
স্থির শূন্য nscss_destroy(সংরচনা content *c);
স্থির nserror nscss_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc);
স্থির bool nscss_matches_quirks(ধ্রুব সংরচনা content *c, bool quirks);
স্থির content_type nscss_content_type(শূন্য);

স্থির nserror nscss_create_css_data(সংরচনা content_css_data *c,
		ধ্রুব বর্ণ *url, ধ্রুব বর্ণ *charset, bool quirks,
		nscss_done_callback done, শূন্য *pw);
স্থির css_error nscss_process_css_data(সংরচনা content_css_data *c, ধ্রুব বর্ণ *data,
		unsigned পূর্ণাঙ্ক size);
স্থির css_error nscss_convert_css_data(সংরচনা content_css_data *c);
স্থির শূন্য nscss_destroy_css_data(সংরচনা content_css_data *c);

স্থির শূন্য nscss_content_done(সংরচনা content_css_data *css, শূন্য *pw);
স্থির css_error nscss_handle_import(শূন্য *pw, css_stylesheet *parent,
		lwc_string *url);
স্থির nserror nscss_import(hlcache_handle *handle,
		ধ্রুব hlcache_event *event, শূন্য *pw);
স্থির css_error nscss_import_complete(nscss_import_ctx *ctx);

স্থির css_error nscss_register_imports(সংরচনা content_css_data *c);
স্থির css_error nscss_register_import(সংরচনা content_css_data *c,
		ধ্রুব hlcache_handle *import);


স্থির css_stylesheet *blank_import;


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
স্থির nserror
nscss_create(ধ্রুব content_handler *handler,
	     lwc_string *imime_type,
	     ধ্রুব http_parameter *params,
	     llcache_handle *llcache,
	     ধ্রুব বর্ণ *fallback_charset,
	     bool quirks,
	     সংরচনা content **c)
{
	nscss_content *result;
	ধ্রুব বর্ণ *charset = রিক্ত;
	ধ্রুব বর্ণ *xnsbase = রিক্ত;
	lwc_string *charset_মান = রিক্ত;
	nserror error;

	result = calloc(1, পরিমাণ(nscss_content));
	যদি (result == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(result);
		প্রত্যাগম error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_মান);
	যদি (error != NSERROR_OK || lwc_string_length(charset_মান) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} অন্যথা {
		charset = lwc_string_data(charset_মান);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	যদি (xnsbase == রিক্ত) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	যদি (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, রিক্ত);
		lwc_string_unref(charset_মান);
		মুক্ত(result);
		প্রত্যাগম error;
	}

	lwc_string_unref(charset_মান);

	*c = (সংরচনা content *) result;

	প্রত্যাগম NSERROR_OK;
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
স্থির nserror nscss_create_css_data(সংরচনা content_css_data *c,
		ধ্রুব বর্ণ *url, ধ্রুব বর্ণ *charset, bool quirks,
		nscss_done_callback done, শূন্য *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_গুনতি = 0;
	c->imports = রিক্ত;
	যদি (charset != রিক্ত)
		c->charset = strdup(charset);
	অন্যথা
		c->charset = রিক্ত;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = রিক্ত;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = রিক্ত;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = রিক্ত;
	params.font = রিক্ত;
	params.font_pw = রিক্ত;

	error = css_stylesheet_create(&params, &c->sheet);
	যদি (error != CSS_OK) {
		প্রত্যাগম NSERROR_NOMEM;
	}

	প্রত্যাগম NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
স্থির bool
nscss_process_data(সংরচনা content *c, ধ্রুব বর্ণ *data, unsigned পূর্ণাঙ্ক size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	যদি (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, রিক্ত);
	}

	প্রত্যাগম (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
স্থির css_error nscss_process_css_data(সংরচনা content_css_data *c,
		ধ্রুব বর্ণ *data, unsigned পূর্ণাঙ্ক size)
{
	প্রত্যাগম css_stylesheet_append_data(c->sheet,
			(ধ্রুব uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(সংরচনা content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	যদি (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, রিক্ত);
		প্রত্যাগম false;
	}

	প্রত্যাগম true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
স্থির css_error nscss_convert_css_data(সংরচনা content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	যদি (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		প্রতিজ্ঞা(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} অন্যথা যদি (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} অন্যথা {
		ধ্রুব বর্ণ *url;

		যদি (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} অন্যথা {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	প্রত্যাগম error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
শূন্য nscss_destroy(সংরচনা content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
স্থির শূন্য nscss_destroy_css_data(সংরচনা content_css_data *c)
{
	uint32_t i;

	প্রতি (i = 0; i < c->import_গুনতি; i++) {
		যদি (c->imports[i].c != রিক্ত) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = রিক্ত;
	}

	মুক্ত(c->imports);

	যদি (c->sheet != রিক্ত) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = রিক্ত;
	}

	মুক্ত(c->charset);
}

nserror nscss_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc)
{
	ধ্রুব nscss_content *old_css = (ধ্রুব nscss_content *) old;
	nscss_content *new_css;
	ধ্রুব uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, পরিমাণ(nscss_content));
	যদি (new_css == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		প্রত্যাগম error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	যদি (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		প্রত্যাগম error;
	}

	data = content__get_source_data(&new_css->base, &size);
	যদি (size > 0) {
		যদি (nscss_process_data(&new_css->base,
				       (বর্ণ *)data,
				       (unsigned পূর্ণাঙ্ক)size) == false) {
			content_destroy(&new_css->base);
			প্রত্যাগম NSERROR_CLONE_FAILED;
		}
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			প্রত্যাগম NSERROR_CLONE_FAILED;
		}
	}

	*newc = (সংরচনা content *) new_css;

	প্রত্যাগম NSERROR_OK;
}

bool nscss_matches_quirks(ধ্রুব সংরচনা content *c, bool quirks)
{
	প্রত্যাগম c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(সংরচনা hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	প্রত্যাগম c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
সংরচনা nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);
	প্রতিজ্ঞা(n != রিক্ত);

	*n = c->data.import_গুনতি;

	প্রত্যাগম c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(শূন্য)
{
	প্রত্যাগম CONTENT_CSS;
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
শূন্য nscss_content_done(সংরচনা content_css_data *css, শূন্য *pw)
{
	সংরচনা content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	যদি (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, রিক্ত);
		content_set_error(c);
		প্রত্যাগম;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	প্রতি (i = 0; i < css->import_গুনতি; i++) {
		যদি (css->imports[i].c != রিক্ত) {
			সংরচনা content *import = hlcache_handle_get_content(
					css->imports[i].c);

			যদি (import != রিক্ত) {
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
css_error nscss_handle_import(শূন্য *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	সংরচনা content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	সংরচনা nscss_import *imports;
	ধ্রুব বর্ণ *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	প্রতিজ্ঞা(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	যদি (error != CSS_OK) {
		প্রত্যাগম error;
	}

	ctx = malloc(পরিমাণ(*ctx));
	যদি (ctx == রিক্ত)
		প্রত্যাগম CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_গুনতি;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_গুনতি + 1) *
			পরিমাণ(সংরচনা nscss_import));
	যদি (imports == রিক্ত) {
		মুক্ত(ctx);
		প্রত্যাগম CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = রিক্ত;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	যদি (error != CSS_OK) {
		মুক্ত(ctx);
		প্রত্যাগম error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	যদি (nerror != NSERROR_OK) {
		মুক্ত(ctx);
		প্রত্যাগম CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	যদি (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		মুক্ত(ctx);
		প্রত্যাগম CSS_NOMEM;
	}

	/* Avoid importing ourself */
	যদি (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_গুনতি].c = রিক্ত;
		/* No longer require context as we're not fetching anything */
		মুক্ত(ctx);
		ctx = রিক্ত;
	} অন্যথা {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, রিক্ত, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_গুনতি].c);
		যদি (nerror != NSERROR_OK) {
			মুক্ত(ctx);
			প্রত্যাগম CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_গুনতি, lwc_string_data(url),
	      c->imports[c->import_গুনতি].c, ctx);
#endif

	c->import_গুনতি++;

	প্রত্যাগম CSS_OK;
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
		ধ্রুব hlcache_event *event, শূন্য *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	প্রতিজ্ঞা(ctx->css->imports[ctx->index].c == handle);

	বিকল্প (event->type) {
	পক্ষ CONTENT_বার্তা_DONE:
		error = nscss_import_complete(ctx);
		ভঙ্গ;

	পক্ষ CONTENT_বার্তা_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = রিক্ত;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ভঙ্গ;
	স্বভাব:
		ভঙ্গ;
	}

	/* Preserve out-of-memory. Anything else is OK */
	প্রত্যাগম error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	যদি (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	মুক্ত(ctx);

	প্রত্যাগম error;
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
css_error nscss_register_imports(সংরচনা content_css_data *c)
{
	uint32_t index;
	css_error error;

	প্রতিজ্ঞা(c->next_to_register != (uint32_t) -1);
	প্রতিজ্ঞা(c->next_to_register < c->import_গুনতি);

	/* Register imported sheets */
	প্রতি (index = c->next_to_register; index < c->import_গুনতি; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		যদি (c->imports[index].c != রিক্ত &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			ভঙ্গ;
		}

		error = nscss_register_import(c, c->imports[index].c);
		যদি (error != CSS_OK)
			প্রত্যাগম error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	যদি (c->next_to_register == c->import_গুনতি) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	প্রত্যাগম CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(সংরচনা content_css_data *c,
		ধ্রুব hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	যদি (import != রিক্ত) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} অন্যথা {
		/* Create a blank sheet if needed. */
		যদি (blank_import == রিক্ত) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = রিক্ত;
			params.url = "";
			params.title = রিক্ত;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = রিক্ত;
			params.import = রিক্ত;
			params.import_pw = রিক্ত;
			params.color = ns_system_colour;
			params.color_pw = রিক্ত;
			params.font = রিক্ত;
			params.font_pw = রিক্ত;

			error = css_stylesheet_create(&params, &blank_import);
			যদি (error != CSS_OK) {
				প্রত্যাগম error;
			}

			error = css_stylesheet_data_done(blank_import);
			যদি (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				প্রত্যাগম error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	যদি (error != CSS_OK) {
		প্রত্যাগম error;
	}

	প্রত্যাগম error;
}

/**
 * Clean up after the CSS content handler
 */
স্থির শূন্য nscss_fini(শূন্য)
{
	যদি (blank_import != রিক্ত) {
		css_stylesheet_destroy(blank_import);
		blank_import = রিক্ত;
	}
	css_hint_fini();
}

স্থির ধ্রুব content_handler css_content_handler = {
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
nserror nscss_init(শূন্য)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	যদি (error != NSERROR_OK)
		গচ্ছ error;

	error = css_hint_init();
	যদি (error != NSERROR_OK)
		গচ্ছ error;

	প্রত্যাগম NSERROR_OK;

error:
	nscss_fini();

	প্রত্যাগম error;
}
