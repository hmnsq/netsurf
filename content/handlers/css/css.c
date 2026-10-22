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
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ರಚನೆ content_css_ದತ್ತಾಂಶ;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*nscss_ಮುಗಿದ_callback)(ರಚನೆ content_css_ದತ್ತಾಂಶ *css, ಶೂನ್ಯ *pw);

/**
 * CSS content data
 */
ರಚನೆ content_css_ದತ್ತಾಂಶ
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	ಅಕ್ಷರ *charset;			/**< Character set of stylesheet */
	ರಚನೆ nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_ಎಣಿಕೆ;		/**< Number of sheets imported */
	uint32_t ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್;	/**< Index of next import to register */
	nscss_ಮುಗಿದ_callback ಮುಗಿದ;	/**< Completion callback */
	ಶೂನ್ಯ *pw;			/**< Client data */
};

/**
 * CSS content data
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nscss_content
{
	ರಚನೆ content ತಳ;		/**< Underlying content object */

	ರಚನೆ content_css_ದತ್ತಾಂಶ ದತ್ತಾಂಶ;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ರಚನೆ content_css_ದತ್ತಾಂಶ *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ಸ್ಥಿರ bool nscss_convert(ರಚನೆ content *c);
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy(ರಚನೆ content *c);
ಸ್ಥಿರ nserror nscss_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc);
ಸ್ಥಿರ bool nscss_matches_quirks(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, bool quirks);
ಸ್ಥಿರ content_type nscss_content_type(ಶೂನ್ಯ);

ಸ್ಥಿರ nserror nscss_create_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset, bool quirks,
		nscss_ಮುಗಿದ_callback ಮುಗಿದ, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_process_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2);
ಸ್ಥಿರ css_error nscss_convert_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c);
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c);

ಸ್ಥಿರ ಶೂನ್ಯ nscss_content_ಮುಗಿದ(ರಚನೆ content_css_ದತ್ತಾಂಶ *css, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_handle_import(ಶೂನ್ಯ *pw, css_stylesheet *ಪೋಷಕ,
		lwc_string *url);
ಸ್ಥಿರ nserror nscss_import(hlcache_handle *handle,
		ಸ್ಥಿರಾಂಕ hlcache_event *event, ಶೂನ್ಯ *pw);
ಸ್ಥಿರ css_error nscss_import_complete(nscss_import_ctx *ctx);

ಸ್ಥಿರ css_error nscss_ರಿಜಿಸ್ಟರ್_imports(ರಚನೆ content_css_ದತ್ತಾಂಶ *c);
ಸ್ಥಿರ css_error nscss_ರಿಜಿಸ್ಟರ್_import(ರಚನೆ content_css_ದತ್ತಾಂಶ *c,
		ಸ್ಥಿರಾಂಕ hlcache_handle *import);


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
nscss_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
	     lwc_string *imime_type,
	     ಸ್ಥಿರಾಂಕ http_parameter *params,
	     llcache_handle *llcache,
	     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	     bool quirks,
	     ರಚನೆ content **c)
{
	nscss_content *ಫಲಿತಾಂಶ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *xnsbase = ಖಾಲಿ;
	lwc_string *charset_ಮೌಲ್ಯ = ಖಾಲಿ;
	nserror error;

	ಫಲಿತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nscss_content));
	ಆದರೆ (ಫಲಿತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&ಫಲಿತಾಂಶ->ತಳ, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_ಕಂಡುಹಿಡಿ_ಅಂಶ(params, corestring_lwc_charset,
			&charset_ಮೌಲ್ಯ);
	ಆದರೆ (error != NSERROR_OK || lwc_string_ಉದ್ದಳತೆ(charset_ಮೌಲ್ಯ) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} ಇಲ್ಲದಿದ್ದರೆ {
		charset = lwc_string_ದತ್ತಾಂಶ(charset_ಮೌಲ್ಯ);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_ಶೀರ್ಷಿಕೆ(llcache, "X-NS-Base");
	ಆದರೆ (xnsbase == ಖಾಲಿ) {
		xnsbase = nsurl_access(content_get_url(&ಫಲಿತಾಂಶ->ತಳ));
	}

	error = nscss_create_css_ದತ್ತಾಂಶ(&ಫಲಿತಾಂಶ->ದತ್ತಾಂಶ,
			xnsbase, charset, ಫಲಿತಾಂಶ->ತಳ.quirks,
			nscss_content_ಮುಗಿದ, ಫಲಿತಾಂಶ);
	ಆದರೆ (error != NSERROR_OK) {
		content_ಪ್ರಸಾರ_error(&ಫಲಿತಾಂಶ->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		lwc_string_unref(charset_ಮೌಲ್ಯ);
		ಬಿಡುಗಡೆ(ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ error;
	}

	lwc_string_unref(charset_ಮೌಲ್ಯ);

	*c = (ರಚನೆ content *) ಫಲಿತಾಂಶ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror nscss_create_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset, bool quirks,
		nscss_ಮುಗಿದ_callback ಮುಗಿದ, ಶೂನ್ಯ *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->ಮುಗಿದ = ಮುಗಿದ;
	c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ = (uint32_t) -1;
	c->import_ಎಣಿಕೆ = 0;
	c->imports = ಖಾಲಿ;
	ಆದರೆ (charset != ಖಾಲಿ)
		c->charset = strdup(charset);
	ಇಲ್ಲದಿದ್ದರೆ
		c->charset = ಖಾಲಿ;

	params.params_ಆವೃತ್ತಿ = CSS_STYLESHEET_PARAMS_ಆವೃತ್ತಿ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ಖಾಲಿ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ಖಾಲಿ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_ಬಣ್ಣ;
	params.color_pw = ಖಾಲಿ;
	params.font = ಖಾಲಿ;
	params.font_pw = ಖಾಲಿ;

	error = css_stylesheet_create(&params, &c->sheet);
	ಆದರೆ (error != CSS_OK) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
nscss_process_ದತ್ತಾಂಶ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_ದತ್ತಾಂಶ(&css->ದತ್ತಾಂಶ, ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
	ಆದರೆ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_ಪ್ರಸಾರ_error(c, NSERROR_CSS, ಖಾಲಿ);
	}

	ಹಿಂತಿರುಗಿ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ಸ್ಥಿರ css_error nscss_process_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ css_stylesheet_append_ದತ್ತಾಂಶ(c->sheet,
			(ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ರಚನೆ content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_ದತ್ತಾಂಶ(&css->ದತ್ತಾಂಶ);
	ಆದರೆ (error != CSS_OK) {
		content_ಪ್ರಸಾರ_error(c, NSERROR_CSS, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ಸ್ಥಿರ css_error nscss_convert_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c)
{
	css_error error;

	error = css_stylesheet_ದತ್ತಾಂಶ_ಮುಗಿದ(c->sheet);

	/* Process pending imports */
	ಆದರೆ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ಪ್ರತಿಜ್ಞಾ(c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ = 0;
		error = nscss_ರಿಜಿಸ್ಟರ್_imports(c);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->ಮುಗಿದ(c, c->pw);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url;

		ಆದರೆ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} ಇಲ್ಲದಿದ್ದರೆ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	ಹಿಂತಿರುಗಿ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
ಶೂನ್ಯ nscss_destroy(ರಚನೆ content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_ದತ್ತಾಂಶ(&css->ದತ್ತಾಂಶ);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_destroy_css_ದತ್ತಾಂಶ(ರಚನೆ content_css_ದತ್ತಾಂಶ *c)
{
	uint32_t i;

	ಗಾಗಿ (i = 0; i < c->import_ಎಣಿಕೆ; i++) {
		ಆದರೆ (c->imports[i].c != ಖಾಲಿ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ಖಾಲಿ;
	}

	ಬಿಡುಗಡೆ(c->imports);

	ಆದರೆ (c->sheet != ಖಾಲಿ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ಖಾಲಿ;
	}

	ಬಿಡುಗಡೆ(c->charset);
}

nserror nscss_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	ಸ್ಥಿರಾಂಕ nscss_content *old_css = (ಸ್ಥಿರಾಂಕ nscss_content *) old;
	nscss_content *new_css;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
	nserror error;

	new_css = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nscss_content));
	ಆದರೆ (new_css == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&new_css->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_ದತ್ತಾಂಶ(&new_css->ದತ್ತಾಂಶ,
			nsurl_access(content_get_url(&new_css->ತಳ)),
			old_css->ದತ್ತಾಂಶ.charset,
			new_css->ತಳ.quirks,
			nscss_content_ಮುಗಿದ, new_css);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&new_css->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(&new_css->ತಳ, &ಗಾತ್ರ_2);
	ಆದರೆ (ಗಾತ್ರ_2 > 0) {
		ಆದರೆ (nscss_process_ದತ್ತಾಂಶ(&new_css->ತಳ,
				       (ಅಕ್ಷರ *)ದತ್ತಾಂಶ,
				       (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2) == false) {
			content_destroy(&new_css->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
			old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (nscss_convert(&new_css->ತಳ) == false) {
			content_destroy(&new_css->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *) new_css;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

bool nscss_matches_quirks(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, bool quirks)
{
	ಹಿಂತಿರುಗಿ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ರಚನೆ hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->ದತ್ತಾಂಶ.sheet;
}

/* exported interface documented in netsurf/css.h */
ರಚನೆ nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(n != ಖಾಲಿ);

	*n = c->ದತ್ತಾಂಶ.import_ಎಣಿಕೆ;

	ಹಿಂತಿರುಗಿ c->ದತ್ತಾಂಶ.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_CSS;
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
ಶೂನ್ಯ nscss_content_ಮುಗಿದ(ರಚನೆ content_css_ದತ್ತಾಂಶ *css, ಶೂನ್ಯ *pw)
{
	ರಚನೆ content *c = pw;
	uint32_t i;
	size_t ಗಾತ್ರ_2;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_ಗಾತ್ರ(css->sheet, &ಗಾತ್ರ_2);
	ಆದರೆ (error != CSS_OK) {
		content_ಪ್ರಸಾರ_error(c, NSERROR_CSS, ಖಾಲಿ);
		content_ಗಣ_error(c);
		ಹಿಂತಿರುಗಿ;
	}
	c->ಗಾತ್ರ_2 += ಗಾತ್ರ_2;

	/* Add on the size of the imported sheets */
	ಗಾಗಿ (i = 0; i < css->import_ಎಣಿಕೆ; i++) {
		ಆದರೆ (css->imports[i].c != ಖಾಲಿ) {
			ರಚನೆ content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ಆದರೆ (import != ಖಾಲಿ) {
				c->ಗಾತ್ರ_2 += import->ಗಾತ್ರ_2;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
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
css_error nscss_handle_import(ಶೂನ್ಯ *pw, css_stylesheet *ಪೋಷಕ,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ರಚನೆ content_css_ದತ್ತಾಂಶ *c = pw;
	nscss_import_ctx *ctx;
	hlcache_ಮಗು_context ಮಗು;
	ರಚನೆ nscss_import *imports;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ಆದರೆ (error != CSS_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	ctx = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*ctx));
	ಆದರೆ (ctx == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_ಎಣಿಕೆ;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_ಎಣಿಕೆ + 1) *
			ಗಾತ್ರ(ರಚನೆ nscss_import));
	ಆದರೆ (imports == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	ಮಗು.charset = ಖಾಲಿ;
	error = css_stylesheet_quirks_allowed(c->sheet, &ಮಗು.quirks);
	ಆದರೆ (error != CSS_OK) {
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_ದತ್ತಾಂಶ(url), &ns_url);
	ಆದರೆ (nerror != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ಆದರೆ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		ಬಿಡುಗಡೆ(ctx);
		ಹಿಂತಿರುಗಿ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ಆದರೆ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_ಎಣಿಕೆ].c = ಖಾಲಿ;
		/* No longer require context as we're not fetching anything */
		ಬಿಡುಗಡೆ(ctx);
		ctx = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ಖಾಲಿ, nscss_import, ctx,
				&ಮಗು, accept,
				&c->imports[c->import_ಎಣಿಕೆ].c);
		ಆದರೆ (nerror != NSERROR_OK) {
			ಬಿಡುಗಡೆ(ctx);
			ಹಿಂತಿರುಗಿ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_ಎಣಿಕೆ, lwc_string_ದತ್ತಾಂಶ(url),
	      c->imports[c->import_ಎಣಿಕೆ].c, ctx);
#ಆದರೆ_ಅಂತ್ಯ

	c->import_ಎಣಿಕೆ++;

	ಹಿಂತಿರುಗಿ CSS_OK;
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
		ಸ್ಥಿರಾಂಕ hlcache_event *event, ಶೂನ್ಯ *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#ಆದರೆ_ಅಂತ್ಯ

	ಪ್ರತಿಜ್ಞಾ(ctx->css->imports[ctx->index].c == handle);

	ಆಯ್ಕೆ (event->type) {
	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ಮುಗಿದ:
		error = nscss_import_complete(ctx);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ಖಾಲಿ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* Preserve out-of-memory. Anything else is OK */
	ಹಿಂತಿರುಗಿ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ಆದರೆ (ctx->css->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ == ctx->index)
		error = nscss_ರಿಜಿಸ್ಟರ್_imports(ctx->css);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#ಆದರೆ_ಅಂತ್ಯ

	/* No longer need import context */
	ಬಿಡುಗಡೆ(ctx);

	ಹಿಂತಿರುಗಿ error;
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
css_error nscss_ರಿಜಿಸ್ಟರ್_imports(ರಚನೆ content_css_ದತ್ತಾಂಶ *c)
{
	uint32_t index;
	css_error error;

	ಪ್ರತಿಜ್ಞಾ(c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ != (uint32_t) -1);
	ಪ್ರತಿಜ್ಞಾ(c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ < c->import_ಎಣಿಕೆ);

	/* Register imported sheets */
	ಗಾಗಿ (index = c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್; index < c->import_ಎಣಿಕೆ; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ಆದರೆ (c->imports[index].c != ಖಾಲಿ &&
			content_get_ಸ್ಥಿತಿ(c->imports[index].c) !=
				CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
			ನಿಲ್ಲಿಸು;
		}

		error = nscss_ರಿಜಿಸ್ಟರ್_import(c, c->imports[index].c);
		ಆದರೆ (error != CSS_OK)
			ಹಿಂತಿರುಗಿ error;
	}

	/* Record identity of the next import to register */
	c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ = (uint32_t) index;

	ಆದರೆ (c->ಮುಂದಿನ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್ == c->import_ಎಣಿಕೆ) {
		/* No more imports: notify parent that we're DONE */
		c->ಮುಗಿದ(c, c->pw);
	}

	ಹಿಂತಿರುಗಿ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_ರಿಜಿಸ್ಟರ್_import(ರಚನೆ content_css_ದತ್ತಾಂಶ *c,
		ಸ್ಥಿರಾಂಕ hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ಆದರೆ (import != ಖಾಲಿ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->ದತ್ತಾಂಶ.sheet;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Create a blank sheet if needed. */
		ಆದರೆ (blank_import == ಖಾಲಿ) {
			css_stylesheet_params params;

			params.params_ಆವೃತ್ತಿ = CSS_STYLESHEET_PARAMS_ಆವೃತ್ತಿ_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ಖಾಲಿ;
			params.url = "";
			params.title = ಖಾಲಿ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ಖಾಲಿ;
			params.import = ಖಾಲಿ;
			params.import_pw = ಖಾಲಿ;
			params.color = ns_system_ಬಣ್ಣ;
			params.color_pw = ಖಾಲಿ;
			params.font = ಖಾಲಿ;
			params.font_pw = ಖಾಲಿ;

			error = css_stylesheet_create(&params, &blank_import);
			ಆದರೆ (error != CSS_OK) {
				ಹಿಂತಿರುಗಿ error;
			}

			error = css_stylesheet_ದತ್ತಾಂಶ_ಮುಗಿದ(blank_import);
			ಆದರೆ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				ಹಿಂತಿರುಗಿ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_ರಿಜಿಸ್ಟರ್_import(c->sheet, sheet);
	ಆದರೆ (error != CSS_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	ಹಿಂತಿರುಗಿ error;
}

/**
 * Clean up after the CSS content handler
 */
ಸ್ಥಿರ ಶೂನ್ಯ nscss_fini(ಶೂನ್ಯ)
{
	ಆದರೆ (blank_import != ಖಾಲಿ) {
		css_stylesheet_destroy(blank_import);
		blank_import = ಖಾಲಿ;
	}
	css_hint_fini();
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_ದತ್ತಾಂಶ = nscss_process_ದತ್ತಾಂಶ,
	.ದತ್ತಾಂಶ_complete = nscss_convert,
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

	error = content_factory_ರಿಜಿಸ್ಟರ್_handler("text/css",
			&css_content_handler);
	ಆದರೆ (error != NSERROR_OK)
		ಹೋಗು error;

	error = css_hint_init();
	ಆದರೆ (error != NSERROR_OK)
		ಹೋಗು error;

	ಹಿಂತಿರುಗಿ NSERROR_OK;

error:
	nscss_fini();

	ಹಿಂತಿರುಗಿ error;
}
