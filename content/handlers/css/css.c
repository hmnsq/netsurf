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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "desktop/system_colour.h"

#ಅನ್ತರ್ಭಾವ "css/css.h"
#ಅನ್ತರ್ಭಾವ "css/hints.h"
#ಅನ್ತರ್ಭಾವ "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ಸಂರಚನಾ content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ (*nscss_done_callback)(ಸಂರಚನಾ content_css_data *css, ಶೂನ್ಯ *pw);

/**
 * CSS content data
 */
ಸಂರಚನಾ content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	ವರ್ಣ *charset;			/**< Character set of stylesheet */
	ಸಂರಚನಾ nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_ಎಣಿಕೆ;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	ಶೂನ್ಯ *pw;			/**< Client data */
};

/**
 * CSS content data
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nscss_content
{
	ಸಂರಚನಾ content base;		/**< Underlying content object */

	ಸಂರಚನಾ content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
	ಸಂರಚನಾ content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ಸ್ಥಿರ bool nscss_convert(ಸಂರಚನಾ content *c);
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy(ಸಂರಚನಾ content *c);
ಸ್ಥಿರ nserror nscss_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc);
ಸ್ಥಿರ bool nscss_matches_quirks(ಧ್ರುವ ಸಂರಚನಾ content *c, bool quirks);
ಸ್ಥಿರ content_type nscss_content_type(ಶೂನ್ಯ);

ಸ್ಥಿರ nserror nscss_create_css_data(ಸಂರಚನಾ content_css_data *c,
		ಧ್ರುವ ವರ್ಣ *url, ಧ್ರುವ ವರ್ಣ *charset, bool quirks,
		nscss_done_callback done, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_process_css_data(ಸಂರಚನಾ content_css_data *c, ಧ್ರುವ ವರ್ಣ *data,
		unsigned ಪೂರ್ಣಾಙ್ಕ size);
ಸ್ಥಿರ css_error nscss_convert_css_data(ಸಂರಚನಾ content_css_data *c);
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy_css_data(ಸಂರಚನಾ content_css_data *c);

ಸ್ಥಿರ ಶೂನ್ಯ nscss_content_done(ಸಂರಚನಾ content_css_data *css, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_handle_import(ಶೂನ್ಯ *pw, css_stylesheet *parent,
		lwc_string *url);
ಸ್ಥಿರ nserror nscss_import(hlcache_handle *handle,
		ಧ್ರುವ hlcache_event *event, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_import_complete(nscss_import_ctx *ctx);

ಸ್ಥಿರ css_error nscss_register_imports(ಸಂರಚನಾ content_css_data *c);
ಸ್ಥಿರ css_error nscss_register_import(ಸಂರಚನಾ content_css_data *c,
		ಧ್ರುವ hlcache_handle *import);


ಸ್ಥಿರ css_stylesheet *blank_import;


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
ಸ್ಥಿರ nserror
nscss_create(ಧ್ರುವ content_handler *handler,
	     lwc_string *imime_type,
	     ಧ್ರುವ http_parameter *params,
	     llcache_handle *llcache,
	     ಧ್ರುವ ವರ್ಣ *fallback_charset,
	     bool quirks,
	     ಸಂರಚನಾ content **c)
{
	nscss_content *result;
	ಧ್ರುವ ವರ್ಣ *charset = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *xnsbase = ರಿಕ್ತ;
	lwc_string *charset_ಮೌಲ್ಯ = ರಿಕ್ತ;
	nserror error;

	result = calloc(1, ಪರಿಮಾಣ(nscss_content));
	ಯದಿ (result == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_ಮೌಲ್ಯ);
	ಯದಿ (error != NSERROR_OK || lwc_string_length(charset_ಮೌಲ್ಯ) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} ಅನ್ಯಥಾ {
		charset = lwc_string_data(charset_ಮೌಲ್ಯ);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	ಯದಿ (xnsbase == ರಿಕ್ತ) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	ಯದಿ (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, ರಿಕ್ತ);
		lwc_string_unref(charset_ಮೌಲ್ಯ);
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	lwc_string_unref(charset_ಮೌಲ್ಯ);

	*c = (ಸಂರಚನಾ content *) result;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ nserror nscss_create_css_data(ಸಂರಚನಾ content_css_data *c,
		ಧ್ರುವ ವರ್ಣ *url, ಧ್ರುವ ವರ್ಣ *charset, bool quirks,
		nscss_done_callback done, ಶೂನ್ಯ *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_ಎಣಿಕೆ = 0;
	c->imports = ರಿಕ್ತ;
	ಯದಿ (charset != ರಿಕ್ತ)
		c->charset = strdup(charset);
	ಅನ್ಯಥಾ
		c->charset = ರಿಕ್ತ;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ರಿಕ್ತ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ರಿಕ್ತ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = ರಿಕ್ತ;
	params.font = ರಿಕ್ತ;
	params.font_pw = ರಿಕ್ತ;

	error = css_stylesheet_create(&params, &c->sheet);
	ಯದಿ (error != CSS_OK) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ಸ್ಥಿರ bool
nscss_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data, unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	ಯದಿ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, ರಿಕ್ತ);
	}

	ಪ್ರತ್ಯಾಗಮ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ css_error nscss_process_css_data(ಸಂರಚನಾ content_css_data *c,
		ಧ್ರುವ ವರ್ಣ *data, unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪ್ರತ್ಯಾಗಮ css_stylesheet_append_data(c->sheet,
			(ಧ್ರುವ uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ಸಂರಚನಾ content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	ಯದಿ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ಸ್ಥಿರ css_error nscss_convert_css_data(ಸಂರಚನಾ content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	ಯದಿ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ಪ್ರತಿಜ್ಞಾ(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} ಅನ್ಯಥಾ ಯದಿ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} ಅನ್ಯಥಾ {
		ಧ್ರುವ ವರ್ಣ *url;

		ಯದಿ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} ಅನ್ಯಥಾ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	ಪ್ರತ್ಯಾಗಮ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
ಶೂನ್ಯ nscss_destroy(ಸಂರಚನಾ content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy_css_data(ಸಂರಚನಾ content_css_data *c)
{
	uint32_t i;

	ಪ್ರತಿ (i = 0; i < c->import_ಎಣಿಕೆ; i++) {
		ಯದಿ (c->imports[i].c != ರಿಕ್ತ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ರಿಕ್ತ;
	}

	ಮುಕ್ತ(c->imports);

	ಯದಿ (c->sheet != ರಿಕ್ತ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ರಿಕ್ತ;
	}

	ಮುಕ್ತ(c->charset);
}

nserror nscss_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	ಧ್ರುವ nscss_content *old_css = (ಧ್ರುವ nscss_content *) old;
	nscss_content *new_css;
	ಧ್ರುವ uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, ಪರಿಮಾಣ(nscss_content));
	ಯದಿ (new_css == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	data = content__get_source_data(&new_css->base, &size);
	ಯದಿ (size > 0) {
		ಯದಿ (nscss_process_data(&new_css->base,
				       (ವರ್ಣ *)data,
				       (unsigned ಪೂರ್ಣಾಙ್ಕ)size) == false) {
			content_destroy(&new_css->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) new_css;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

bool nscss_matches_quirks(ಧ್ರುವ ಸಂರಚನಾ content *c, bool quirks)
{
	ಪ್ರತ್ಯಾಗಮ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ಸಂರಚನಾ hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
ಸಂರಚನಾ nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(n != ರಿಕ್ತ);

	*n = c->data.import_ಎಣಿಕೆ;

	ಪ್ರತ್ಯಾಗಮ c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_CSS;
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
ಶೂನ್ಯ nscss_content_done(ಸಂರಚನಾ content_css_data *css, ಶೂನ್ಯ *pw)
{
	ಸಂರಚನಾ content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	ಯದಿ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ರಿಕ್ತ);
		content_set_error(c);
		ಪ್ರತ್ಯಾಗಮ;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	ಪ್ರತಿ (i = 0; i < css->import_ಎಣಿಕೆ; i++) {
		ಯದಿ (css->imports[i].c != ರಿಕ್ತ) {
			ಸಂರಚನಾ content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ಯದಿ (import != ರಿಕ್ತ) {
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
css_error nscss_handle_import(ಶೂನ್ಯ *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ಸಂರಚನಾ content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	ಸಂರಚನಾ nscss_import *imports;
	ಧ್ರುವ ವರ್ಣ *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ಪ್ರತಿಜ್ಞಾ(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ಯದಿ (error != CSS_OK) {
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ctx = malloc(ಪರಿಮಾಣ(*ctx));
	ಯದಿ (ctx == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_ಎಣಿಕೆ;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_ಎಣಿಕೆ + 1) *
			ಪರಿಮಾಣ(ಸಂರಚನಾ nscss_import));
	ಯದಿ (imports == ರಿಕ್ತ) {
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = ರಿಕ್ತ;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	ಯದಿ (error != CSS_OK) {
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	ಯದಿ (nerror != NSERROR_OK) {
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ಯದಿ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		ಮುಕ್ತ(ctx);
		ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ಯದಿ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_ಎಣಿಕೆ].c = ರಿಕ್ತ;
		/* No longer require context as we're not fetching anything */
		ಮುಕ್ತ(ctx);
		ctx = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ರಿಕ್ತ, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_ಎಣಿಕೆ].c);
		ಯದಿ (nerror != NSERROR_OK) {
			ಮುಕ್ತ(ctx);
			ಪ್ರತ್ಯಾಗಮ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_ಎಣಿಕೆ, lwc_string_data(url),
	      c->imports[c->import_ಎಣಿಕೆ].c, ctx);
#endif

	c->import_ಎಣಿಕೆ++;

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
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
		ಧ್ರುವ hlcache_event *event, ಶೂನ್ಯ *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	ಪ್ರತಿಜ್ಞಾ(ctx->css->imports[ctx->index].c == handle);

	ವಿಕಲ್ಪ (event->type) {
	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		error = nscss_import_complete(ctx);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ರಿಕ್ತ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* Preserve out-of-memory. Anything else is OK */
	ಪ್ರತ್ಯಾಗಮ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ಯದಿ (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	ಮುಕ್ತ(ctx);

	ಪ್ರತ್ಯಾಗಮ error;
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
css_error nscss_register_imports(ಸಂರಚನಾ content_css_data *c)
{
	uint32_t index;
	css_error error;

	ಪ್ರತಿಜ್ಞಾ(c->next_to_register != (uint32_t) -1);
	ಪ್ರತಿಜ್ಞಾ(c->next_to_register < c->import_ಎಣಿಕೆ);

	/* Register imported sheets */
	ಪ್ರತಿ (index = c->next_to_register; index < c->import_ಎಣಿಕೆ; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ಯದಿ (c->imports[index].c != ರಿಕ್ತ &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			ಭಙ್ಗ;
		}

		error = nscss_register_import(c, c->imports[index].c);
		ಯದಿ (error != CSS_OK)
			ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	ಯದಿ (c->next_to_register == c->import_ಎಣಿಕೆ) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	ಪ್ರತ್ಯಾಗಮ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(ಸಂರಚನಾ content_css_data *c,
		ಧ್ರುವ hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ಯದಿ (import != ರಿಕ್ತ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} ಅನ್ಯಥಾ {
		/* Create a blank sheet if needed. */
		ಯದಿ (blank_import == ರಿಕ್ತ) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ರಿಕ್ತ;
			params.url = "";
			params.title = ರಿಕ್ತ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ರಿಕ್ತ;
			params.import = ರಿಕ್ತ;
			params.import_pw = ರಿಕ್ತ;
			params.color = ns_system_colour;
			params.color_pw = ರಿಕ್ತ;
			params.font = ರಿಕ್ತ;
			params.font_pw = ರಿಕ್ತ;

			error = css_stylesheet_create(&params, &blank_import);
			ಯದಿ (error != CSS_OK) {
				ಪ್ರತ್ಯಾಗಮ error;
			}

			error = css_stylesheet_data_done(blank_import);
			ಯದಿ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				ಪ್ರತ್ಯಾಗಮ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	ಯದಿ (error != CSS_OK) {
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ಪ್ರತ್ಯಾಗಮ error;
}

/**
 * Clean up after the CSS content handler
 */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_fini(ಶೂನ್ಯ)
{
	ಯದಿ (blank_import != ರಿಕ್ತ) {
		css_stylesheet_destroy(blank_import);
		blank_import = ರಿಕ್ತ;
	}
	css_hint_fini();
}

ಸ್ಥಿರ ಧ್ರುವ content_handler css_content_handler = {
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
nserror nscss_init(ಶೂನ್ಯ)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	ಯದಿ (error != NSERROR_OK)
		ಗಚ್ಛ error;

	error = css_hint_init();
	ಯದಿ (error != NSERROR_OK)
		ಗಚ್ಛ error;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

error:
	nscss_fini();

	ಪ್ರತ್ಯಾಗಮ error;
}
