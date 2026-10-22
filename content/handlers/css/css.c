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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "desktop/system_colour.h"

#ଅନ୍ତର୍ଭାଵ "css/css.h"
#ଅନ୍ତର୍ଭାଵ "css/hints.h"
#ଅନ୍ତର୍ଭାଵ "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ସଂରଚନା content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ (*nscss_done_callback)(ସଂରଚନା content_css_data *css, ଶୂନ୍ୟ *pw);

/**
 * CSS content data
 */
ସଂରଚନା content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	ଅକ୍ଷର *charset;			/**< Character set of stylesheet */
	ସଂରଚନା nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_ଗଣନା;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	ଶୂନ୍ୟ *pw;			/**< Client data */
};

/**
 * CSS content data
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nscss_content
{
	ସଂରଚନା content base;		/**< Underlying content object */

	ସଂରଚନା content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
	ସଂରଚନା content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ସ୍ଥିର bool nscss_convert(ସଂରଚନା content *c);
ସ୍ଥିର ଶୂନ୍ୟ nscss_destroy(ସଂରଚନା content *c);
ସ୍ଥିର nserror nscss_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc);
ସ୍ଥିର bool nscss_matches_quirks(ଅଚଳ ସଂରଚନା content *c, bool quirks);
ସ୍ଥିର content_type nscss_content_type(ଶୂନ୍ୟ);

ସ୍ଥିର nserror nscss_create_css_data(ସଂରଚନା content_css_data *c,
		ଅଚଳ ଅକ୍ଷର *url, ଅଚଳ ଅକ୍ଷର *charset, bool quirks,
		nscss_done_callback done, ଶୂନ୍ୟ *pw);
ସ୍ଥିର css_error nscss_process_css_data(ସଂରଚନା content_css_data *c, ଅଚଳ ଅକ୍ଷର *data,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size);
ସ୍ଥିର css_error nscss_convert_css_data(ସଂରଚନା content_css_data *c);
ସ୍ଥିର ଶୂନ୍ୟ nscss_destroy_css_data(ସଂରଚନା content_css_data *c);

ସ୍ଥିର ଶୂନ୍ୟ nscss_content_done(ସଂରଚନା content_css_data *css, ଶୂନ୍ୟ *pw);
ସ୍ଥିର css_error nscss_handle_import(ଶୂନ୍ୟ *pw, css_stylesheet *parent,
		lwc_string *url);
ସ୍ଥିର nserror nscss_import(hlcache_handle *handle,
		ଅଚଳ hlcache_event *event, ଶୂନ୍ୟ *pw);
ସ୍ଥିର css_error nscss_import_complete(nscss_import_ctx *ctx);

ସ୍ଥିର css_error nscss_register_imports(ସଂରଚନା content_css_data *c);
ସ୍ଥିର css_error nscss_register_import(ସଂରଚନା content_css_data *c,
		ଅଚଳ hlcache_handle *import);


ସ୍ଥିର css_stylesheet *blank_import;


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
ସ୍ଥିର nserror
nscss_create(ଅଚଳ content_handler *handler,
	     lwc_string *imime_type,
	     ଅଚଳ http_parameter *params,
	     llcache_handle *llcache,
	     ଅଚଳ ଅକ୍ଷର *fallback_charset,
	     bool quirks,
	     ସଂରଚନା content **c)
{
	nscss_content *result;
	ଅଚଳ ଅକ୍ଷର *charset = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *xnsbase = ଖାଲି;
	lwc_string *charset_ମୂଲ୍ୟ = ଖାଲି;
	nserror error;

	result = calloc(1, ଆକାର(nscss_content));
	ଯଦି (result == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_ମୂଲ୍ୟ);
	ଯଦି (error != NSERROR_OK || lwc_string_length(charset_ମୂଲ୍ୟ) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} ଅନ୍ୟଥା {
		charset = lwc_string_data(charset_ମୂଲ୍ୟ);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	ଯଦି (xnsbase == ଖାଲି) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	ଯଦି (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, ଖାଲି);
		lwc_string_unref(charset_ମୂଲ୍ୟ);
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	lwc_string_unref(charset_ମୂଲ୍ୟ);

	*c = (ସଂରଚନା content *) result;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror nscss_create_css_data(ସଂରଚନା content_css_data *c,
		ଅଚଳ ଅକ୍ଷର *url, ଅଚଳ ଅକ୍ଷର *charset, bool quirks,
		nscss_done_callback done, ଶୂନ୍ୟ *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_ଗଣନା = 0;
	c->imports = ଖାଲି;
	ଯଦି (charset != ଖାଲି)
		c->charset = strdup(charset);
	ଅନ୍ୟଥା
		c->charset = ଖାଲି;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ଖାଲି;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ଖାଲି;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = ଖାଲି;
	params.font = ଖାଲି;
	params.font_pw = ଖାଲି;

	error = css_stylesheet_create(&params, &c->sheet);
	ଯଦି (error != CSS_OK) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ସ୍ଥିର bool
nscss_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	ଯଦି (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, ଖାଲି);
	}

	ଫେରନ୍ତୁ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ସ୍ଥିର css_error nscss_process_css_data(ସଂରଚନା content_css_data *c,
		ଅଚଳ ଅକ୍ଷର *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ଫେରନ୍ତୁ css_stylesheet_append_data(c->sheet,
			(ଅଚଳ uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ସଂରଚନା content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	ଯଦି (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ସ୍ଥିର css_error nscss_convert_css_data(ସଂରଚନା content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	ଯଦି (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ପ୍ରତିଜ୍ଞା(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} ଅନ୍ୟଥା ଯଦି (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} ଅନ୍ୟଥା {
		ଅଚଳ ଅକ୍ଷର *url;

		ଯଦି (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} ଅନ୍ୟଥା {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	ଫେରନ୍ତୁ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
ଶୂନ୍ୟ nscss_destroy(ସଂରଚନା content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ସ୍ଥିର ଶୂନ୍ୟ nscss_destroy_css_data(ସଂରଚନା content_css_data *c)
{
	uint32_t i;

	ପାଇଁ (i = 0; i < c->import_ଗଣନା; i++) {
		ଯଦି (c->imports[i].c != ଖାଲି) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ଖାଲି;
	}

	ମୁକ୍ତ(c->imports);

	ଯଦି (c->sheet != ଖାଲି) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ଖାଲି;
	}

	ମୁକ୍ତ(c->charset);
}

nserror nscss_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	ଅଚଳ nscss_content *old_css = (ଅଚଳ nscss_content *) old;
	nscss_content *new_css;
	ଅଚଳ uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, ଆକାର(nscss_content));
	ଯଦି (new_css == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ଫେରନ୍ତୁ error;
	}

	data = content__get_source_data(&new_css->base, &size);
	ଯଦି (size > 0) {
		ଯଦି (nscss_process_data(&new_css->base,
				       (ଅକ୍ଷର *)data,
				       (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size) == false) {
			content_destroy(&new_css->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) new_css;

	ଫେରନ୍ତୁ NSERROR_OK;
}

bool nscss_matches_quirks(ଅଚଳ ସଂରଚନା content *c, bool quirks)
{
	ଫେରନ୍ତୁ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ସଂରଚନା hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
ସଂରଚନା nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(n != ଖାଲି);

	*n = c->data.import_ଗଣନା;

	ଫେରନ୍ତୁ c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_CSS;
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
ଶୂନ୍ୟ nscss_content_done(ସଂରଚନା content_css_data *css, ଶୂନ୍ୟ *pw)
{
	ସଂରଚନା content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	ଯଦି (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ଖାଲି);
		content_set_error(c);
		ଫେରନ୍ତୁ;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	ପାଇଁ (i = 0; i < css->import_ଗଣନା; i++) {
		ଯଦି (css->imports[i].c != ଖାଲି) {
			ସଂରଚନା content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ଯଦି (import != ଖାଲି) {
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
css_error nscss_handle_import(ଶୂନ୍ୟ *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ସଂରଚନା content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	ସଂରଚନା nscss_import *imports;
	ଅଚଳ ଅକ୍ଷର *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ପ୍ରତିଜ୍ଞା(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ଯଦି (error != CSS_OK) {
		ଫେରନ୍ତୁ error;
	}

	ctx = malloc(ଆକାର(*ctx));
	ଯଦି (ctx == ଖାଲି)
		ଫେରନ୍ତୁ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_ଗଣନା;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_ଗଣନା + 1) *
			ଆକାର(ସଂରଚନା nscss_import));
	ଯଦି (imports == ଖାଲି) {
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = ଖାଲି;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	ଯଦି (error != CSS_OK) {
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	ଯଦି (nerror != NSERROR_OK) {
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ଯଦି (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		ମୁକ୍ତ(ctx);
		ଫେରନ୍ତୁ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ଯଦି (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_ଗଣନା].c = ଖାଲି;
		/* No longer require context as we're not fetching anything */
		ମୁକ୍ତ(ctx);
		ctx = ଖାଲି;
	} ଅନ୍ୟଥା {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ଖାଲି, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_ଗଣନା].c);
		ଯଦି (nerror != NSERROR_OK) {
			ମୁକ୍ତ(ctx);
			ଫେରନ୍ତୁ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_ଗଣନା, lwc_string_data(url),
	      c->imports[c->import_ଗଣନା].c, ctx);
#endif

	c->import_ଗଣନା++;

	ଫେରନ୍ତୁ CSS_OK;
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
		ଅଚଳ hlcache_event *event, ଶୂନ୍ୟ *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	ପ୍ରତିଜ୍ଞା(ctx->css->imports[ctx->index].c == handle);

	ବାଛ (event->type) {
	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		error = nscss_import_complete(ctx);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ଖାଲି;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* Preserve out-of-memory. Anything else is OK */
	ଫେରନ୍ତୁ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ଯଦି (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	ମୁକ୍ତ(ctx);

	ଫେରନ୍ତୁ error;
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
css_error nscss_register_imports(ସଂରଚନା content_css_data *c)
{
	uint32_t index;
	css_error error;

	ପ୍ରତିଜ୍ଞା(c->next_to_register != (uint32_t) -1);
	ପ୍ରତିଜ୍ଞା(c->next_to_register < c->import_ଗଣନା);

	/* Register imported sheets */
	ପାଇଁ (index = c->next_to_register; index < c->import_ଗଣନା; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ଯଦି (c->imports[index].c != ଖାଲି &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			ଭାଙ୍ଗ;
		}

		error = nscss_register_import(c, c->imports[index].c);
		ଯଦି (error != CSS_OK)
			ଫେରନ୍ତୁ error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	ଯଦି (c->next_to_register == c->import_ଗଣନା) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	ଫେରନ୍ତୁ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(ସଂରଚନା content_css_data *c,
		ଅଚଳ hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ଯଦି (import != ଖାଲି) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} ଅନ୍ୟଥା {
		/* Create a blank sheet if needed. */
		ଯଦି (blank_import == ଖାଲି) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ଖାଲି;
			params.url = "";
			params.title = ଖାଲି;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ଖାଲି;
			params.import = ଖାଲି;
			params.import_pw = ଖାଲି;
			params.color = ns_system_colour;
			params.color_pw = ଖାଲି;
			params.font = ଖାଲି;
			params.font_pw = ଖାଲି;

			error = css_stylesheet_create(&params, &blank_import);
			ଯଦି (error != CSS_OK) {
				ଫେରନ୍ତୁ error;
			}

			error = css_stylesheet_data_done(blank_import);
			ଯଦି (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				ଫେରନ୍ତୁ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	ଯଦି (error != CSS_OK) {
		ଫେରନ୍ତୁ error;
	}

	ଫେରନ୍ତୁ error;
}

/**
 * Clean up after the CSS content handler
 */
ସ୍ଥିର ଶୂନ୍ୟ nscss_fini(ଶୂନ୍ୟ)
{
	ଯଦି (blank_import != ଖାଲି) {
		css_stylesheet_destroy(blank_import);
		blank_import = ଖାଲି;
	}
	css_hint_fini();
}

ସ୍ଥିର ଅଚଳ content_handler css_content_handler = {
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
nserror nscss_init(ଶୂନ୍ୟ)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	ଯଦି (error != NSERROR_OK)
		ଯାଅ error;

	error = css_hint_init();
	ଯଦି (error != NSERROR_OK)
		ଯାଅ error;

	ଫେରନ୍ତୁ NSERROR_OK;

error:
	nscss_fini();

	ଫେରନ୍ତୁ error;
}
