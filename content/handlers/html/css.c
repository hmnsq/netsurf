/*
 * Copyright 2013 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 * Processing for html content css operations.
 */

#ಅನ್ತರ್ಭಾವ "utils/config.h"

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "css/css.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/css.h"

ಸ್ಥಿರ nsurl *html_default_stylesheet_url;
ಸ್ಥಿರ nsurl *html_adblock_stylesheet_url;
ಸ್ಥಿರ nsurl *html_quirks_stylesheet_url;
ಸ್ಥಿರ nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ಸ್ಥಿರ nserror css_error_ಗಮ್ಯ_nserror(css_error error)
{
	ಆಯ್ಕೆ (error) {
	ಸ್ಥಿತಿ CSS_OK:
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಸ್ಥಿತಿ CSS_NOMEM:
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	ಸ್ಥಿತಿ CSS_BADPARM:
		ಹಿಂತಿರುಗಿ NSERROR_BAD_PARAMETER;

	ಸ್ಥಿತಿ CSS_INVALID:
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;

	ಸ್ಥಿತಿ CSS_FILENOTFOUND:
		ಹಿಂತಿರುಗಿ NSERROR_NOT_ಸಿಕ್ಕಿತು;

	ಸ್ಥಿತಿ CSS_NEEDDATA:
		ಹಿಂತಿರುಗಿ NSERROR_NEED_ದತ್ತಾಂಶ;

	ಸ್ಥಿತಿ CSS_BADCHARSET:
		ಹಿಂತಿರುಗಿ NSERROR_BAD_ENCODING;

	ಸ್ಥಿತಿ CSS_EOF:
	ಸ್ಥಿತಿ CSS_IMPORTS_PENDING:
	ಸ್ಥಿತಿ CSS_PROPERTY_NOT_ಗಣ:
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ಸ್ಥಿರ nserror
html_convert_css_callback(hlcache_handle *css,
			  ಸ್ಥಿರಾಂಕ hlcache_event *event,
			  ಶೂನ್ಯ *pw)
{
	html_content *ಪೋಷಕ = pw;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ html_stylesheet *s;

	/* Find sheet */
	ಗಾಗಿ (i = 0, s = ಪೋಷಕ->stylesheets;
	     i != ಪೋಷಕ->stylesheet_ಎಣಿಕೆ;
	     i++, s++) {
		ಆದರೆ (s->sheet == css)
			ನಿಲ್ಲಿಸು;
	}

	ಪ್ರತಿಜ್ಞಾ(i != ಪೋಷಕ->stylesheet_ಎಣಿಕೆ);

	ಆಯ್ಕೆ (event->type) {

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ಮುಗಿದ:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		ಪೋಷಕ->ತಳ.ಸಕ್ರಿಯ--;
		NSLOG(netsurf, INFO, "%d fetches active", ಪೋಷಕ->ತಳ.ಸಕ್ರಿಯ);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->ದತ್ತಾಂಶ.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ಖಾಲಿ;
		ಪೋಷಕ->ತಳ.ಸಕ್ರಿಯ--;
		NSLOG(netsurf, INFO, "%d fetches active", ಪೋಷಕ->ತಳ.ಸಕ್ರಿಯ);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸಂದೇಶ_ಸೂಚಕ:
		/* Really don't want this to continue after the switch */
		ಹಿಂತಿರುಗಿ NSERROR_OK;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (html_can_begin_conversion(ಪೋಷಕ)) {
		html_begin_conversion(ಪೋಷಕ);
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ಗಂಟು *ಗಂಟು,
			     hlcache_handle **sheet)
{
	hlcache_ಮಗು_context ಮಗು;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t ಕೀಲಿ;
	ಅಕ್ಷರ urlbuf[64];

	ಮಗು.charset = c->encoding;
	ಮಗು.quirks = c->ತಳ.quirks;

	exc = dom_ಗಂಟು_get_ಪಠ್ಯ_content(ಗಂಟು, &style);
	ಆದರೆ ((exc != DOM_NO_ERR) || (style == ಖಾಲಿ)) {
		NSLOG(netsurf, INFO, "No text content");
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	error = html_css_fetcher_ಕೂಡಿಸು_ಅಂಶ(style, c->ತಳ_url, &ಕೀಲಿ);
	ಆದರೆ (error != NSERROR_OK) {
		dom_string_unref(style);
		ಹಿಂತಿರುಗಿ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, ಗಾತ್ರ(urlbuf), "x-ns-css:%"PRIu32"", ಕೀಲಿ);

	error = nsurl_create(urlbuf, &url);
	ಆದರೆ (error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->ತಳ), ಖಾಲಿ,
			html_convert_css_callback, c, &ಮಗು, CONTENT_CSS,
			sheet);
	ಆದರೆ (error != NSERROR_OK) {
		nsurl_unref(url);
		ಹಿಂತಿರುಗಿ error;
	}

	nsurl_unref(url);

	c->ತಳ.ಸಕ್ರಿಯ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ಸ್ಥಿರ ರಚನೆ html_stylesheet *
html_create_style_element(html_content *c, dom_ಗಂಟು *style)
{
	dom_string *val;
	dom_exception exc;
	ರಚನೆ html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ಆದರೆ (exc == DOM_NO_ERR && val != ಖಾಲಿ) {
		ಆದರೆ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_ಪಠ್ಯ_css)) {
			dom_string_unref(val);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ಆದರೆ (exc == DOM_NO_ERR && val != ಖಾಲಿ) {
		ಆದರೆ (strcasestr(dom_string_ದತ್ತಾಂಶ(val), "screen") == ಖಾಲಿ &&
				strcasestr(dom_string_ದತ್ತಾಂಶ(val),
						"all") == ಖಾಲಿ) {
			dom_string_unref(val);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      ಗಾತ್ರ(ರಚನೆ html_stylesheet) *
			      (c->stylesheet_ಎಣಿಕೆ + 1));
	ಆದರೆ (stylesheets == ಖಾಲಿ) {

		content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].ಗಂಟು = dom_ಗಂಟು_ref(style);
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].sheet = ಖಾಲಿ;
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].modified = false;
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].ಬಳಸದ = false;
	c->stylesheet_ಎಣಿಕೆ++;

	ಹಿಂತಿರುಗಿ c->stylesheets + (c->stylesheet_ಎಣಿಕೆ - 1);
}


ಸ್ಥಿರ bool
html_css_process_modified_style(html_content *c, ರಚನೆ html_stylesheet *s)
{
	hlcache_handle *sheet = ಖಾಲಿ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ಗಂಟು, &sheet);
	ಆದರೆ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_ಪ್ರಸಾರ_error(&c->ತಳ, error, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (sheet != ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ಆದರೆ (s->sheet != ಖಾಲಿ) {
			ಆಯ್ಕೆ (content_get_ಸ್ಥಿತಿ(s->sheet)) {
			ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ:
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				hlcache_handle_abort(s->sheet);
				c->ತಳ.ಸಕ್ರಿಯ--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->ತಳ.ಸಕ್ರಿಯ);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	ಹಿಂತಿರುಗಿ true;
}


/**
 * process a stylesheet that has been modified.
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_css_process_modified_styles(ಶೂನ್ಯ *pw)
{
	html_content *c = pw;
	ರಚನೆ html_stylesheet *s;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	bool all_ಮುಗಿದ = true;

	ಗಾಗಿ (i = 0, s = c->stylesheets; i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಆದರೆ (c->stylesheets[i].modified) {
			all_ಮುಗಿದ &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ಆದರೆ (all_ಮುಗಿದ == false) {
		guit->misc->ವೇಳಾಪಟ್ಟಿ(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ಗಂಟು *style)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ html_stylesheet *s;

	/* Find sheet */
	ಗಾಗಿ (i = 0, s = c->stylesheets;	i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಆದರೆ (s->ಗಂಟು == style)
			ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (i == c->stylesheet_ಎಣಿಕೆ) {
		s = html_create_style_element(c, style);
	}
	ಆದರೆ (s == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ಹಿಂತಿರುಗಿ false;
	}

	s->modified = true;

	guit->misc->ವೇಳಾಪಟ್ಟಿ(0, html_css_process_modified_styles, c);

	ಹಿಂತಿರುಗಿ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ಗಂಟು *ಗಂಟು)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	dom_string *val;
	dom_exception exc;
	ರಚನೆ html_stylesheet *s;

	/* Find sheet */
	ಗಾಗಿ (i = 0, s = c->stylesheets;	i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಆದರೆ (s->ಗಂಟು == ಗಂಟು)
			ನಿಲ್ಲಿಸು;
	}

		/* Should already exist */
	ಆದರೆ (i == c->stylesheet_ಎಣಿಕೆ) {
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_media, &val);
	ಆದರೆ (exc == DOM_NO_ERR && val != ಖಾಲಿ) {
		ಆದರೆ (strcasestr(dom_string_ದತ್ತಾಂಶ(val), "screen") == ಖಾಲಿ &&
				strcasestr(dom_string_ದತ್ತಾಂಶ(val),
						"all") == ಖಾಲಿ) {
			s->ಬಳಸದ = true;
		}
		dom_string_unref(val);
	}

	ಹಿಂತಿರುಗಿ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *rel, *type_attr, *media, *href;
	ರಚನೆ html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_ಮಗು_context ಮಗು;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_rel, &rel);
	ಆದರೆ (exc != DOM_NO_ERR || rel == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (strcasestr(dom_string_ದತ್ತಾಂಶ(rel), "stylesheet") == ಖಾಲಿ) {
		dom_string_unref(rel);
		ಹಿಂತಿರುಗಿ true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcasestr(dom_string_ದತ್ತಾಂಶ(rel), "alternate") != ಖಾಲಿ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ಹಿಂತಿರುಗಿ true;
	}
	dom_string_unref(rel);

	ಆದರೆ (nsoption_bool(author_level_css) == false) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_type, &type_attr);
	ಆದರೆ (exc == DOM_NO_ERR && type_attr != ಖಾಲಿ) {
		ಆದರೆ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_ಪಠ್ಯ_css)) {
			dom_string_unref(type_attr);
			ಹಿಂತಿರುಗಿ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_media, &media);
	ಆದರೆ (exc == DOM_NO_ERR && media != ಖಾಲಿ) {
		ಆದರೆ (strcasestr(dom_string_ದತ್ತಾಂಶ(media), "screen") == ಖಾಲಿ &&
		    strcasestr(dom_string_ದತ್ತಾಂಶ(media), "all") == ಖಾಲಿ) {
			dom_string_unref(media);
			ಹಿಂತಿರುಗಿ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &href);
	ಆದರೆ (exc != DOM_NO_ERR || href == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->ತಳ_url, dom_string_ದತ್ತಾಂಶ(href), &joined);
	ಆದರೆ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		ಹೋಗು no_ಸ್ಮರಣೆ;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_ಎಣಿಕೆ, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      ಗಾತ್ರ(ರಚನೆ html_stylesheet) *
			      (htmlc->stylesheet_ಎಣಿಕೆ + 1));
	ಆದರೆ (stylesheets == ಖಾಲಿ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		ಹೋಗು no_ಸ್ಮರಣೆ;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].ಗಂಟು = ಖಾಲಿ;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].ಬಳಸದ = false;

	/* start fetch */
	ಮಗು.charset = htmlc->encoding;
	ಮಗು.quirks = htmlc->ತಳ.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->ತಳ),
			ಖಾಲಿ, html_convert_css_callback,
			htmlc, &ಮಗು, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].sheet);

	nsurl_unref(joined);

	ಆದರೆ (ns_error != NSERROR_OK)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	htmlc->stylesheet_ಎಣಿಕೆ++;

	htmlc->ತಳ.ಸಕ್ರಿಯ++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->ತಳ.ಸಕ್ರಿಯ);

	ಹಿಂತಿರುಗಿ true;

no_ಸ್ಮರಣೆ:
	content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, ns_error, ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in html/html.h */
ರಚನೆ html_stylesheet *html_get_stylesheets(hlcache_handle *h, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(n != ಖಾಲಿ);

	*n = c->stylesheet_ಎಣಿಕೆ;

	ಹಿಂತಿರುಗಿ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ರಚನೆ html_stylesheet *s;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0, s = html->stylesheets; i < html->stylesheet_ಎಣಿಕೆ;
	     i++, s++) {
		ಆದರೆ (s->sheet != ಖಾಲಿ) {
			ಆದರೆ (content_saw_insecure_objects(s->sheet)) {
				ಹಿಂತಿರುಗಿ true;
			}
		}
	}

	ಹಿಂತಿರುಗಿ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	guit->misc->ವೇಳಾಪಟ್ಟಿ(-1, html_css_process_modified_styles, html);

	ಗಾಗಿ (i = 0; i != html->stylesheet_ಎಣಿಕೆ; i++) {
		ಆದರೆ (html->stylesheets[i].sheet != ಖಾಲಿ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ಆದರೆ (html->stylesheets[i].ಗಂಟು != ಖಾಲಿ) {
			dom_ಗಂಟು_unref(html->stylesheets[i].ಗಂಟು);
		}
	}
	ಬಿಡುಗಡೆ(html->stylesheets);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_ಮಗು_context ಮಗು;

	ಪ್ರತಿಜ್ಞಾ(c->stylesheets != ಖಾಲಿ);

	ಆದರೆ (c->quirks == DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_ತುಂಬಿದ) {
		ಮಗು.charset = c->encoding;
		ಮಗು.quirks = c->ತಳ.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->ತಳ), ಖಾಲಿ,
				html_convert_css_callback, c, &ಮಗು,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ಆದರೆ (ns_error != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ ns_error;
		}

		c->ತಳ.ಸಕ್ರಿಯ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);
	}

	ಹಿಂತಿರುಗಿ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_ಮಗು_context ಮಗು;

	ಆದರೆ (c->stylesheets != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(STYLESHEET_ಆರಂಭ,
			ಗಾತ್ರ(ರಚನೆ html_stylesheet));
	ಆದರೆ (c->stylesheets == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_ತಳ].sheet = ಖಾಲಿ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ಖಾಲಿ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ಖಾಲಿ;
	c->stylesheets[STYLESHEET_USER].sheet = ಖಾಲಿ;
	c->stylesheet_ಎಣಿಕೆ = STYLESHEET_ಆರಂಭ;

	ಮಗು.charset = c->encoding;
	ಮಗು.quirks = c->ತಳ.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->ತಳ), ಖಾಲಿ,
			html_convert_css_callback, c, &ಮಗು, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_ತಳ].sheet);
	ಆದರೆ (ns_error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ns_error;
	}

	c->ತಳ.ಸಕ್ರಿಯ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);


	ಆದರೆ (nsoption_bool(ಖಂಡ_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->ತಳ), ಖಾಲಿ,
				html_convert_css_callback,
				c, &ಮಗು, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ಆದರೆ (ns_error != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ ns_error;
		}

		c->ತಳ.ಸಕ್ರಿಯ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->ತಳ), ಖಾಲಿ,
			html_convert_css_callback, c, &ಮಗು, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ಆದರೆ (ns_error != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ ns_error;
	}

	c->ತಳ.ಸಕ್ರಿಯ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ತಳ.ಸಕ್ರಿಯ);

	ಹಿಂತಿರುಗಿ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ಆದರೆ (c->stylesheets[STYLESHEET_ತಳ].sheet == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_CSS_ತಳ;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ಆದರೆ (css_ret != CSS_OK) {
		ಹಿಂತಿರುಗಿ css_error_ಗಮ್ಯ_nserror(css_ret);
	}

	/* Add sheets to it */
	ಗಾಗಿ (i = STYLESHEET_ತಳ; i != c->stylesheet_ಎಣಿಕೆ; i++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ಖಾಲಿ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ಆದರೆ (hsheet->ಬಳಸದ) {
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i < STYLESHEET_ಆರಂಭ) {
			origin = CSS_ORIGIN_USER;
		}

		ಆದರೆ (hsheet->sheet != ಖಾಲಿ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ಆದರೆ (sheet != ಖಾಲಿ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ಆದರೆ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ಹಿಂತಿರುಗಿ css_error_ಗಮ್ಯ_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(ಶೂನ್ಯ)
{
	nserror error;

	error = html_css_fetcher_ರಿಜಿಸ್ಟರ್();
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ಹಿಂತಿರುಗಿ error;
}


/* exported function documented in html/css.h */
ಶೂನ್ಯ html_css_fini(ಶೂನ್ಯ)
{
	ಆದರೆ (html_user_stylesheet_url != ಖಾಲಿ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ಖಾಲಿ;
	}

	ಆದರೆ (html_quirks_stylesheet_url != ಖಾಲಿ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ಖಾಲಿ;
	}

	ಆದರೆ (html_adblock_stylesheet_url != ಖಾಲಿ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ಖಾಲಿ;
	}

	ಆದರೆ (html_default_stylesheet_url != ಖಾಲಿ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ಖಾಲಿ;
	}
}
