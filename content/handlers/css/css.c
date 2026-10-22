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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/system_colour.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "css/css.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/hints.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/internal.h"

/* Define to trace import fetches */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ਬਣਤਰ content_css_ਅੰਕੜੇ;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*nscss_ਪੂਰਾ_callback)(ਬਣਤਰ content_css_ਅੰਕੜੇ *css, ਖਾਲੀ *pw);

/**
 * CSS content data
 */
ਬਣਤਰ content_css_ਅੰਕੜੇ
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	ਅੱਖਰ *charset;			/**< Character set of stylesheet */
	ਬਣਤਰ nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_ਗਿਣਤੀ;		/**< Number of sheets imported */
	uint32_t ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ;	/**< Index of next import to register */
	nscss_ਪੂਰਾ_callback ਪੂਰਾ;	/**< Completion callback */
	ਖਾਲੀ *pw;			/**< Client data */
};

/**
 * CSS content data
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nscss_content
{
	ਬਣਤਰ content ਅਧਾਰ;		/**< Underlying content object */

	ਬਣਤਰ content_css_ਅੰਕੜੇ ਅੰਕੜੇ;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	ਬਣਤਰ content_css_ਅੰਕੜੇ *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ਸਥਿਰ bool nscss_convert(ਬਣਤਰ content *c);
ਸਥਿਰ ਖਾਲੀ nscss_destroy(ਬਣਤਰ content *c);
ਸਥਿਰ nserror nscss_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc);
ਸਥਿਰ bool nscss_matches_quirks(ਅਚਲ ਬਣਤਰ content *c, bool quirks);
ਸਥਿਰ content_type nscss_content_type(ਖਾਲੀ);

ਸਥਿਰ nserror nscss_create_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c,
		ਅਚਲ ਅੱਖਰ *url, ਅਚਲ ਅੱਖਰ *charset, bool quirks,
		nscss_ਪੂਰਾ_callback ਪੂਰਾ, ਖਾਲੀ *pw);
ਸਥਿਰ css_error nscss_process_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ);
ਸਥਿਰ css_error nscss_convert_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c);
ਸਥਿਰ ਖਾਲੀ nscss_destroy_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c);

ਸਥਿਰ ਖਾਲੀ nscss_content_ਪੂਰਾ(ਬਣਤਰ content_css_ਅੰਕੜੇ *css, ਖਾਲੀ *pw);
ਸਥਿਰ css_error nscss_handle_import(ਖਾਲੀ *pw, css_stylesheet *ਮਾਪਾ,
		lwc_string *url);
ਸਥਿਰ nserror nscss_import(hlcache_handle *handle,
		ਅਚਲ hlcache_event *event, ਖਾਲੀ *pw);
ਸਥਿਰ css_error nscss_import_complete(nscss_import_ctx *ctx);

ਸਥਿਰ css_error nscss_ਰਜਿਸਟਰ_imports(ਬਣਤਰ content_css_ਅੰਕੜੇ *c);
ਸਥਿਰ css_error nscss_ਰਜਿਸਟਰ_import(ਬਣਤਰ content_css_ਅੰਕੜੇ *c,
		ਅਚਲ hlcache_handle *import);


ਸਥਿਰ css_stylesheet *blank_import;


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
ਸਥਿਰ nserror
nscss_create(ਅਚਲ content_handler *handler,
	     lwc_string *imime_type,
	     ਅਚਲ http_parameter *params,
	     llcache_handle *llcache,
	     ਅਚਲ ਅੱਖਰ *fallback_charset,
	     bool quirks,
	     ਬਣਤਰ content **c)
{
	nscss_content *ਨਤੀਜਾ;
	ਅਚਲ ਅੱਖਰ *charset = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *xnsbase = ਨਿਰਾ;
	lwc_string *charset_ਮੁੱਲ = ਨਿਰਾ;
	nserror error;

	ਨਤੀਜਾ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nscss_content));
	ਜੇ (ਨਤੀਜਾ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&ਨਤੀਜਾ->ਅਧਾਰ, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_ਲੱਭੋ_ਵਸਤੂ(params, corestring_lwc_charset,
			&charset_ਮੁੱਲ);
	ਜੇ (error != NSERROR_OK || lwc_string_ਲੰਮਾਈ(charset_ਮੁੱਲ) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} ਨਹੀਂਤਾਂ {
		charset = lwc_string_ਅੰਕੜੇ(charset_ਮੁੱਲ);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_ਸਿਰਲੇਖ(llcache, "X-NS-Base");
	ਜੇ (xnsbase == ਨਿਰਾ) {
		xnsbase = nsurl_access(content_get_url(&ਨਤੀਜਾ->ਅਧਾਰ));
	}

	error = nscss_create_css_ਅੰਕੜੇ(&ਨਤੀਜਾ->ਅੰਕੜੇ,
			xnsbase, charset, ਨਤੀਜਾ->ਅਧਾਰ.quirks,
			nscss_content_ਪੂਰਾ, ਨਤੀਜਾ);
	ਜੇ (error != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(&ਨਤੀਜਾ->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		lwc_string_unref(charset_ਮੁੱਲ);
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	lwc_string_unref(charset_ਮੁੱਲ);

	*c = (ਬਣਤਰ content *) ਨਤੀਜਾ;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror nscss_create_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c,
		ਅਚਲ ਅੱਖਰ *url, ਅਚਲ ਅੱਖਰ *charset, bool quirks,
		nscss_ਪੂਰਾ_callback ਪੂਰਾ, ਖਾਲੀ *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->ਪੂਰਾ = ਪੂਰਾ;
	c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ = (uint32_t) -1;
	c->import_ਗਿਣਤੀ = 0;
	c->imports = ਨਿਰਾ;
	ਜੇ (charset != ਨਿਰਾ)
		c->charset = strdup(charset);
	ਨਹੀਂਤਾਂ
		c->charset = ਨਿਰਾ;

	params.params_ਸੰਸਕਰਣ = CSS_STYLESHEET_PARAMS_ਸੰਸਕਰਣ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ਨਿਰਾ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ਨਿਰਾ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_ਰੰਗ;
	params.color_pw = ਨਿਰਾ;
	params.font = ਨਿਰਾ;
	params.font_pw = ਨਿਰਾ;

	error = css_stylesheet_create(&params, &c->sheet);
	ਜੇ (error != CSS_OK) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ਸਥਿਰ bool
nscss_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_ਅੰਕੜੇ(&css->ਅੰਕੜੇ, ਅੰਕੜੇ, ਮਾਪ);
	ਜੇ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_CSS, ਨਿਰਾ);
	}

	ਵਾਪਸ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ਸਥਿਰ css_error nscss_process_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c,
		ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	ਵਾਪਸ css_stylesheet_append_ਅੰਕੜੇ(c->sheet,
			(ਅਚਲ uint8_t *) ਅੰਕੜੇ, ਮਾਪ);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ਬਣਤਰ content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_ਅੰਕੜੇ(&css->ਅੰਕੜੇ);
	ਜੇ (error != CSS_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_CSS, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ਸਥਿਰ css_error nscss_convert_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c)
{
	css_error error;

	error = css_stylesheet_ਅੰਕੜੇ_ਪੂਰਾ(c->sheet);

	/* Process pending imports */
	ਜੇ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ਪ੍ਰਤਿਜ੍ਞਾ(c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ = 0;
		error = nscss_ਰਜਿਸਟਰ_imports(c);
	} ਨਹੀਂਤਾਂ ਜੇ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->ਪੂਰਾ(c, c->pw);
	} ਨਹੀਂਤਾਂ {
		ਅਚਲ ਅੱਖਰ *url;

		ਜੇ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} ਨਹੀਂਤਾਂ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	ਵਾਪਸ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
ਖਾਲੀ nscss_destroy(ਬਣਤਰ content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_ਅੰਕੜੇ(&css->ਅੰਕੜੇ);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ਸਥਿਰ ਖਾਲੀ nscss_destroy_css_ਅੰਕੜੇ(ਬਣਤਰ content_css_ਅੰਕੜੇ *c)
{
	uint32_t i;

	ਲਈ (i = 0; i < c->import_ਗਿਣਤੀ; i++) {
		ਜੇ (c->imports[i].c != ਨਿਰਾ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ਨਿਰਾ;
	}

	ਮੁਕਤ(c->imports);

	ਜੇ (c->sheet != ਨਿਰਾ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ਨਿਰਾ;
	}

	ਮੁਕਤ(c->charset);
}

nserror nscss_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	ਅਚਲ nscss_content *old_css = (ਅਚਲ nscss_content *) old;
	nscss_content *new_css;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	nserror error;

	new_css = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nscss_content));
	ਜੇ (new_css == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&new_css->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_ਅੰਕੜੇ(&new_css->ਅੰਕੜੇ,
			nsurl_access(content_get_url(&new_css->ਅਧਾਰ)),
			old_css->ਅੰਕੜੇ.charset,
			new_css->ਅਧਾਰ.quirks,
			nscss_content_ਪੂਰਾ, new_css);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&new_css->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(&new_css->ਅਧਾਰ, &ਮਾਪ);
	ਜੇ (ਮਾਪ > 0) {
		ਜੇ (nscss_process_ਅੰਕੜੇ(&new_css->ਅਧਾਰ,
				       (ਅੱਖਰ *)ਅੰਕੜੇ,
				       (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਮਾਪ) == false) {
			content_destroy(&new_css->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (nscss_convert(&new_css->ਅਧਾਰ) == false) {
			content_destroy(&new_css->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) new_css;

	ਵਾਪਸ NSERROR_OK;
}

bool nscss_matches_quirks(ਅਚਲ ਬਣਤਰ content *c, bool quirks)
{
	ਵਾਪਸ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ਬਣਤਰ hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->ਅੰਕੜੇ.sheet;
}

/* exported interface documented in netsurf/css.h */
ਬਣਤਰ nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(n != ਨਿਰਾ);

	*n = c->ਅੰਕੜੇ.import_ਗਿਣਤੀ;

	ਵਾਪਸ c->ਅੰਕੜੇ.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_CSS;
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
ਖਾਲੀ nscss_content_ਪੂਰਾ(ਬਣਤਰ content_css_ਅੰਕੜੇ *css, ਖਾਲੀ *pw)
{
	ਬਣਤਰ content *c = pw;
	uint32_t i;
	size_t ਮਾਪ;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_ਮਾਪ(css->sheet, &ਮਾਪ);
	ਜੇ (error != CSS_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_CSS, ਨਿਰਾ);
		content_ਸੈੱਟ_error(c);
		ਵਾਪਸ;
	}
	c->ਮਾਪ += ਮਾਪ;

	/* Add on the size of the imported sheets */
	ਲਈ (i = 0; i < css->import_ਗਿਣਤੀ; i++) {
		ਜੇ (css->imports[i].c != ਨਿਰਾ) {
			ਬਣਤਰ content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ਜੇ (import != ਨਿਰਾ) {
				c->ਮਾਪ += import->ਮਾਪ;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
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
css_error nscss_handle_import(ਖਾਲੀ *pw, css_stylesheet *ਮਾਪਾ,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ਬਣਤਰ content_css_ਅੰਕੜੇ *c = pw;
	nscss_import_ctx *ctx;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;
	ਬਣਤਰ nscss_import *imports;
	ਅਚਲ ਅੱਖਰ *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ਜੇ (error != CSS_OK) {
		ਵਾਪਸ error;
	}

	ctx = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*ctx));
	ਜੇ (ctx == ਨਿਰਾ)
		ਵਾਪਸ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_ਗਿਣਤੀ;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_ਗਿਣਤੀ + 1) *
			ਆਕਾਰ(ਬਣਤਰ nscss_import));
	ਜੇ (imports == ਨਿਰਾ) {
		ਮੁਕਤ(ctx);
		ਵਾਪਸ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	ਬੱਚਾ.charset = ਨਿਰਾ;
	error = css_stylesheet_quirks_allowed(c->sheet, &ਬੱਚਾ.quirks);
	ਜੇ (error != CSS_OK) {
		ਮੁਕਤ(ctx);
		ਵਾਪਸ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_ਅੰਕੜੇ(url), &ns_url);
	ਜੇ (nerror != NSERROR_OK) {
		ਮੁਕਤ(ctx);
		ਵਾਪਸ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ਜੇ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		ਮੁਕਤ(ctx);
		ਵਾਪਸ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ਜੇ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_ਗਿਣਤੀ].c = ਨਿਰਾ;
		/* No longer require context as we're not fetching anything */
		ਮੁਕਤ(ctx);
		ctx = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ਨਿਰਾ, nscss_import, ctx,
				&ਬੱਚਾ, accept,
				&c->imports[c->import_ਗਿਣਤੀ].c);
		ਜੇ (nerror != NSERROR_OK) {
			ਮੁਕਤ(ctx);
			ਵਾਪਸ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_ਗਿਣਤੀ, lwc_string_ਅੰਕੜੇ(url),
	      c->imports[c->import_ਗਿਣਤੀ].c, ctx);
#ਜੇ_ਅੰਤ

	c->import_ਗਿਣਤੀ++;

	ਵਾਪਸ CSS_OK;
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
		ਅਚਲ hlcache_event *event, ਖਾਲੀ *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#ਜੇ_ਅੰਤ

	ਪ੍ਰਤਿਜ੍ਞਾ(ctx->css->imports[ctx->index].c == handle);

	ਚੋਣ (event->type) {
	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		error = nscss_import_complete(ctx);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ਨਿਰਾ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* Preserve out-of-memory. Anything else is OK */
	ਵਾਪਸ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ਜੇ (ctx->css->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ == ctx->index)
		error = nscss_ਰਜਿਸਟਰ_imports(ctx->css);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#ਜੇ_ਅੰਤ

	/* No longer need import context */
	ਮੁਕਤ(ctx);

	ਵਾਪਸ error;
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
css_error nscss_ਰਜਿਸਟਰ_imports(ਬਣਤਰ content_css_ਅੰਕੜੇ *c)
{
	uint32_t index;
	css_error error;

	ਪ੍ਰਤਿਜ੍ਞਾ(c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ != (uint32_t) -1);
	ਪ੍ਰਤਿਜ੍ਞਾ(c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ < c->import_ਗਿਣਤੀ);

	/* Register imported sheets */
	ਲਈ (index = c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ; index < c->import_ਗਿਣਤੀ; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ਜੇ (c->imports[index].c != ਨਿਰਾ &&
			content_get_ਸਥਿਤੀ(c->imports[index].c) !=
				CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
			ਰੋਕੋ;
		}

		error = nscss_ਰਜਿਸਟਰ_import(c, c->imports[index].c);
		ਜੇ (error != CSS_OK)
			ਵਾਪਸ error;
	}

	/* Record identity of the next import to register */
	c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ = (uint32_t) index;

	ਜੇ (c->ਅਗਲਾ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ == c->import_ਗਿਣਤੀ) {
		/* No more imports: notify parent that we're DONE */
		c->ਪੂਰਾ(c, c->pw);
	}

	ਵਾਪਸ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_ਰਜਿਸਟਰ_import(ਬਣਤਰ content_css_ਅੰਕੜੇ *c,
		ਅਚਲ hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ਜੇ (import != ਨਿਰਾ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->ਅੰਕੜੇ.sheet;
	} ਨਹੀਂਤਾਂ {
		/* Create a blank sheet if needed. */
		ਜੇ (blank_import == ਨਿਰਾ) {
			css_stylesheet_params params;

			params.params_ਸੰਸਕਰਣ = CSS_STYLESHEET_PARAMS_ਸੰਸਕਰਣ_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ਨਿਰਾ;
			params.url = "";
			params.title = ਨਿਰਾ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ਨਿਰਾ;
			params.import = ਨਿਰਾ;
			params.import_pw = ਨਿਰਾ;
			params.color = ns_system_ਰੰਗ;
			params.color_pw = ਨਿਰਾ;
			params.font = ਨਿਰਾ;
			params.font_pw = ਨਿਰਾ;

			error = css_stylesheet_create(&params, &blank_import);
			ਜੇ (error != CSS_OK) {
				ਵਾਪਸ error;
			}

			error = css_stylesheet_ਅੰਕੜੇ_ਪੂਰਾ(blank_import);
			ਜੇ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				ਵਾਪਸ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_ਰਜਿਸਟਰ_import(c->sheet, sheet);
	ਜੇ (error != CSS_OK) {
		ਵਾਪਸ error;
	}

	ਵਾਪਸ error;
}

/**
 * Clean up after the CSS content handler
 */
ਸਥਿਰ ਖਾਲੀ nscss_fini(ਖਾਲੀ)
{
	ਜੇ (blank_import != ਨਿਰਾ) {
		css_stylesheet_destroy(blank_import);
		blank_import = ਨਿਰਾ;
	}
	css_hint_fini();
}

ਸਥਿਰ ਅਚਲ content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_ਅੰਕੜੇ = nscss_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(ਖਾਲੀ)
{
	nserror error;

	error = content_factory_ਰਜਿਸਟਰ_handler("text/css",
			&css_content_handler);
	ਜੇ (error != NSERROR_OK)
		ਜਾਓ error;

	error = css_hint_init();
	ਜੇ (error != NSERROR_OK)
		ਜਾਓ error;

	ਵਾਪਸ NSERROR_OK;

error:
	nscss_fini();

	ਵਾਪਸ error;
}
