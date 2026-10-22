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
ಸ್ಥಿರ nserror css_error_to_nserror(css_error error)
{
	ವಿಕಲ್ಪ (error) {
	ಪಕ್ಷ CSS_OK:
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

	ಪಕ್ಷ CSS_NOMEM:
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	ಪಕ್ಷ CSS_BADPARM:
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_PARAMETER;

	ಪಕ್ಷ CSS_INVALID:
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;

	ಪಕ್ಷ CSS_FILENOTFOUND:
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOT_FOUND;

	ಪಕ್ಷ CSS_NEEDDATA:
		ಪ್ರತ್ಯಾಗಮ NSERROR_NEED_DATA;

	ಪಕ್ಷ CSS_BADCHARSET:
		ಪ್ರತ್ಯಾಗಮ NSERROR_BAD_ENCODING;

	ಪಕ್ಷ CSS_EOF:
	ಪಕ್ಷ CSS_IMPORTS_PENDING:
	ಪಕ್ಷ CSS_PROPERTY_NOT_SET:
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ಸ್ಥಿರ nserror
html_convert_css_callback(hlcache_handle *css,
			  ಧ್ರುವ hlcache_event *event,
			  ಶೂನ್ಯ *pw)
{
	html_content *parent = pw;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_stylesheet *s;

	/* Find sheet */
	ಪ್ರತಿ (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_ಎಣಿಕೆ;
	     i++, s++) {
		ಯದಿ (s->sheet == css)
			ಭಙ್ಗ;
	}

	ಪ್ರತಿಜ್ಞಾ(i != parent->stylesheet_ಎಣಿಕೆ);

	ವಿಕಲ್ಪ (event->type) {

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ರಿಕ್ತ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_ಸಂದೇಶ_POINTER:
		/* Really don't want this to continue after the switch */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಯದಿ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ಗಂಟು *ಗಂಟು,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t ಕೀಲಿ;
	ವರ್ಣ urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_ಗಂಟು_get_text_content(ಗಂಟು, &style);
	ಯದಿ ((exc != DOM_NO_ERR) || (style == ರಿಕ್ತ)) {
		NSLOG(netsurf, INFO, "No text content");
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &ಕೀಲಿ);
	ಯದಿ (error != NSERROR_OK) {
		dom_string_unref(style);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, ಪರಿಮಾಣ(urlbuf), "x-ns-css:%"PRIu32"", ಕೀಲಿ);

	error = nsurl_create(urlbuf, &url);
	ಯದಿ (error != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), ರಿಕ್ತ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	ಯದಿ (error != NSERROR_OK) {
		nsurl_unref(url);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ಸ್ಥಿರ ಸಂರಚನಾ html_stylesheet *
html_create_style_element(html_content *c, dom_ಗಂಟು *style)
{
	dom_string *val;
	dom_exception exc;
	ಸಂರಚನಾ html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ಯದಿ (exc == DOM_NO_ERR && val != ರಿಕ್ತ) {
		ಯದಿ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ಯದಿ (exc == DOM_NO_ERR && val != ರಿಕ್ತ) {
		ಯದಿ (strcasestr(dom_string_data(val), "screen") == ರಿಕ್ತ &&
				strcasestr(dom_string_data(val),
						"all") == ರಿಕ್ತ) {
			dom_string_unref(val);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      ಪರಿಮಾಣ(ಸಂರಚನಾ html_stylesheet) *
			      (c->stylesheet_ಎಣಿಕೆ + 1));
	ಯದಿ (stylesheets == ರಿಕ್ತ) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].ಗಂಟು = dom_ಗಂಟು_ref(style);
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].sheet = ರಿಕ್ತ;
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].modified = false;
	c->stylesheets[c->stylesheet_ಎಣಿಕೆ].unused = false;
	c->stylesheet_ಎಣಿಕೆ++;

	ಪ್ರತ್ಯಾಗಮ c->stylesheets + (c->stylesheet_ಎಣಿಕೆ - 1);
}


ಸ್ಥಿರ bool
html_css_process_modified_style(html_content *c, ಸಂರಚನಾ html_stylesheet *s)
{
	hlcache_handle *sheet = ರಿಕ್ತ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ಗಂಟು, &sheet);
	ಯದಿ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (sheet != ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ಯದಿ (s->sheet != ರಿಕ್ತ) {
			ವಿಕಲ್ಪ (content_get_status(s->sheet)) {
			ಪಕ್ಷ CONTENT_STATUS_DONE:
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				hlcache_handle_abort(s->sheet);
				c->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->base.active);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * process a stylesheet that has been modified.
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_css_process_modified_styles(ಶೂನ್ಯ *pw)
{
	html_content *c = pw;
	ಸಂರಚನಾ html_stylesheet *s;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	bool all_done = true;

	ಪ್ರತಿ (i = 0, s = c->stylesheets; i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ಯದಿ (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ಗಂಟು *style)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ html_stylesheet *s;

	/* Find sheet */
	ಪ್ರತಿ (i = 0, s = c->stylesheets;	i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->ಗಂಟು == style)
			ಭಙ್ಗ;
	}
	ಯದಿ (i == c->stylesheet_ಎಣಿಕೆ) {
		s = html_create_style_element(c, style);
	}
	ಯದಿ (s == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ಗಂಟು *ಗಂಟು)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	dom_string *val;
	dom_exception exc;
	ಸಂರಚನಾ html_stylesheet *s;

	/* Find sheet */
	ಪ್ರತಿ (i = 0, s = c->stylesheets;	i != c->stylesheet_ಎಣಿಕೆ; i++, s++) {
		ಯದಿ (s->ಗಂಟು == ಗಂಟು)
			ಭಙ್ಗ;
	}

		/* Should already exist */
	ಯದಿ (i == c->stylesheet_ಎಣಿಕೆ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_media, &val);
	ಯದಿ (exc == DOM_NO_ERR && val != ರಿಕ್ತ) {
		ಯದಿ (strcasestr(dom_string_data(val), "screen") == ರಿಕ್ತ &&
				strcasestr(dom_string_data(val),
						"all") == ರಿಕ್ತ) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *rel, *type_attr, *media, *href;
	ಸಂರಚನಾ html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_rel, &rel);
	ಯದಿ (exc != DOM_NO_ERR || rel == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (strcasestr(dom_string_data(rel), "stylesheet") == ರಿಕ್ತ) {
		dom_string_unref(rel);
		ಪ್ರತ್ಯಾಗಮ true;
	} ಅನ್ಯಥಾ ಯದಿ (strcasestr(dom_string_data(rel), "alternate") != ರಿಕ್ತ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ಪ್ರತ್ಯಾಗಮ true;
	}
	dom_string_unref(rel);

	ಯದಿ (nsoption_bool(author_level_css) == false) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_type, &type_attr);
	ಯದಿ (exc == DOM_NO_ERR && type_attr != ರಿಕ್ತ) {
		ಯದಿ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ಪ್ರತ್ಯಾಗಮ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_media, &media);
	ಯದಿ (exc == DOM_NO_ERR && media != ರಿಕ್ತ) {
		ಯದಿ (strcasestr(dom_string_data(media), "screen") == ರಿಕ್ತ &&
		    strcasestr(dom_string_data(media), "all") == ರಿಕ್ತ) {
			dom_string_unref(media);
			ಪ್ರತ್ಯಾಗಮ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &href);
	ಯದಿ (exc != DOM_NO_ERR || href == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	ಯದಿ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		ಗಚ್ಛ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_ಎಣಿಕೆ, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      ಪರಿಮಾಣ(ಸಂರಚನಾ html_stylesheet) *
			      (htmlc->stylesheet_ಎಣಿಕೆ + 1));
	ಯದಿ (stylesheets == ರಿಕ್ತ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		ಗಚ್ಛ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].ಗಂಟು = ರಿಕ್ತ;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			ರಿಕ್ತ, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_ಎಣಿಕೆ].sheet);

	nsurl_unref(joined);

	ಯದಿ (ns_error != NSERROR_OK)
		ಗಚ್ಛ no_memory;

	htmlc->stylesheet_ಎಣಿಕೆ++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ಪ್ರತ್ಯಾಗಮ true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in html/html.h */
ಸಂರಚನಾ html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned ಪೂರ್ಣಾಙ್ಕ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(n != ರಿಕ್ತ);

	*n = c->stylesheet_ಎಣಿಕೆ;

	ಪ್ರತ್ಯಾಗಮ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ಸಂರಚನಾ html_stylesheet *s;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0, s = html->stylesheets; i < html->stylesheet_ಎಣಿಕೆ;
	     i++, s++) {
		ಯದಿ (s->sheet != ರಿಕ್ತ) {
			ಯದಿ (content_saw_insecure_objects(s->sheet)) {
				ಪ್ರತ್ಯಾಗಮ true;
			}
		}
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	ಪ್ರತಿ (i = 0; i != html->stylesheet_ಎಣಿಕೆ; i++) {
		ಯದಿ (html->stylesheets[i].sheet != ರಿಕ್ತ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ಯದಿ (html->stylesheets[i].ಗಂಟು != ರಿಕ್ತ) {
			dom_ಗಂಟು_unref(html->stylesheets[i].ಗಂಟು);
		}
	}
	ಮುಕ್ತ(html->stylesheets);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	ಪ್ರತಿಜ್ಞಾ(c->stylesheets != ರಿಕ್ತ);

	ಯದಿ (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), ರಿಕ್ತ,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ಯದಿ (ns_error != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ಪ್ರತ್ಯಾಗಮ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	ಯದಿ (c->stylesheets != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			ಪರಿಮಾಣ(ಸಂರಚನಾ html_stylesheet));
	ಯದಿ (c->stylesheets == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = ರಿಕ್ತ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ರಿಕ್ತ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ರಿಕ್ತ;
	c->stylesheets[STYLESHEET_USER].sheet = ರಿಕ್ತ;
	c->stylesheet_ಎಣಿಕೆ = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), ರಿಕ್ತ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	ಯದಿ (ns_error != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	ಯದಿ (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), ರಿಕ್ತ,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ಯದಿ (ns_error != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), ರಿಕ್ತ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ಯದಿ (ns_error != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ಪ್ರತ್ಯಾಗಮ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ಯದಿ (c->stylesheets[STYLESHEET_BASE].sheet == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ಯದಿ (css_ret != CSS_OK) {
		ಪ್ರತ್ಯಾಗಮ css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	ಪ್ರತಿ (i = STYLESHEET_BASE; i != c->stylesheet_ಎಣಿಕೆ; i++) {
		ಧ್ರುವ ಸಂರಚನಾ html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ರಿಕ್ತ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ಯದಿ (hsheet->unused) {
			ಅನುವರ್ತ;
		}

		ಯದಿ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ಅನ್ಯಥಾ ಯದಿ (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		ಯದಿ (hsheet->sheet != ರಿಕ್ತ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ಯದಿ (sheet != ರಿಕ್ತ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ಯದಿ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ಪ್ರತ್ಯಾಗಮ css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(ಶೂನ್ಯ)
{
	nserror error;

	error = html_css_fetcher_register();
	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ಯದಿ (error != NSERROR_OK)
		ಪ್ರತ್ಯಾಗಮ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ಪ್ರತ್ಯಾಗಮ error;
}


/* exported function documented in html/css.h */
ಶೂನ್ಯ html_css_fini(ಶೂನ್ಯ)
{
	ಯದಿ (html_user_stylesheet_url != ರಿಕ್ತ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ರಿಕ್ತ;
	}

	ಯದಿ (html_quirks_stylesheet_url != ರಿಕ್ತ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ರಿಕ್ತ;
	}

	ಯದಿ (html_adblock_stylesheet_url != ರಿಕ್ತ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ರಿಕ್ತ;
	}

	ಯದಿ (html_default_stylesheet_url != ರಿಕ್ತ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ರಿಕ್ತ;
	}
}
