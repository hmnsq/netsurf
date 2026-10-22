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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/css.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/css.h"

ਸਥਿਰ nsurl *html_default_stylesheet_url;
ਸਥਿਰ nsurl *html_adblock_stylesheet_url;
ਸਥਿਰ nsurl *html_quirks_stylesheet_url;
ਸਥਿਰ nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ਸਥਿਰ nserror css_error_ਮੰਜ਼ਿਲ_nserror(css_error error)
{
	ਚੋਣ (error) {
	ਹਾਲਤ CSS_OK:
		ਵਾਪਸ NSERROR_OK;

	ਹਾਲਤ CSS_NOMEM:
		ਵਾਪਸ NSERROR_NOMEM;

	ਹਾਲਤ CSS_BADPARM:
		ਵਾਪਸ NSERROR_BAD_PARAMETER;

	ਹਾਲਤ CSS_INVALID:
		ਵਾਪਸ NSERROR_INVALID;

	ਹਾਲਤ CSS_FILENOTFOUND:
		ਵਾਪਸ NSERROR_NOT_ਮਿਲਿਆ;

	ਹਾਲਤ CSS_NEEDDATA:
		ਵਾਪਸ NSERROR_NEED_ਅੰਕੜੇ;

	ਹਾਲਤ CSS_BADCHARSET:
		ਵਾਪਸ NSERROR_BAD_ENCODING;

	ਹਾਲਤ CSS_EOF:
	ਹਾਲਤ CSS_IMPORTS_PENDING:
	ਹਾਲਤ CSS_PROPERTY_NOT_ਸੈੱਟ:
	ਮੂਲ:
		ਰੋਕੋ;
	}
	ਵਾਪਸ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ਸਥਿਰ nserror
html_convert_css_callback(hlcache_handle *css,
			  ਅਚਲ hlcache_event *event,
			  ਖਾਲੀ *pw)
{
	html_content *ਮਾਪਾ = pw;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_stylesheet *s;

	/* Find sheet */
	ਲਈ (i = 0, s = ਮਾਪਾ->stylesheets;
	     i != ਮਾਪਾ->stylesheet_ਗਿਣਤੀ;
	     i++, s++) {
		ਜੇ (s->sheet == css)
			ਰੋਕੋ;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(i != ਮਾਪਾ->stylesheet_ਗਿਣਤੀ);

	ਚੋਣ (event->type) {

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਪੂਰਾ:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->ਅੰਕੜੇ.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ਨਿਰਾ;
		ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ--;
		NSLOG(netsurf, INFO, "%d fetches active", ਮਾਪਾ->ਅਧਾਰ.ਸਰਗਰਮ);
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸੁਨੇਹਾ_ਸੂਚਕ:
		/* Really don't want this to continue after the switch */
		ਵਾਪਸ NSERROR_OK;

	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਜੇ (html_can_begin_conversion(ਮਾਪਾ)) {
		html_begin_conversion(ਮਾਪਾ);
	}

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ਗੰਢ *ਗੰਢ,
			     hlcache_handle **sheet)
{
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t ਕੁੰਜੀ;
	ਅੱਖਰ urlbuf[64];

	ਬੱਚਾ.charset = c->encoding;
	ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

	exc = dom_ਗੰਢ_get_ਪਾਠ_content(ਗੰਢ, &style);
	ਜੇ ((exc != DOM_NO_ERR) || (style == ਨਿਰਾ)) {
		NSLOG(netsurf, INFO, "No text content");
		ਵਾਪਸ NSERROR_OK;
	}

	error = html_css_fetcher_ਜੋੜੋ_ਵਸਤੂ(style, c->ਅਧਾਰ_url, &ਕੁੰਜੀ);
	ਜੇ (error != NSERROR_OK) {
		dom_string_unref(style);
		ਵਾਪਸ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, ਆਕਾਰ(urlbuf), "x-ns-css:%"PRIu32"", ਕੁੰਜੀ);

	error = nsurl_create(urlbuf, &url);
	ਜੇ (error != NSERROR_OK) {
		ਵਾਪਸ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
			html_convert_css_callback, c, &ਬੱਚਾ, CONTENT_CSS,
			sheet);
	ਜੇ (error != NSERROR_OK) {
		nsurl_unref(url);
		ਵਾਪਸ error;
	}

	nsurl_unref(url);

	c->ਅਧਾਰ.ਸਰਗਰਮ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ਸਥਿਰ ਬਣਤਰ html_stylesheet *
html_create_style_element(html_content *c, dom_ਗੰਢ *style)
{
	dom_string *val;
	dom_exception exc;
	ਬਣਤਰ html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ਜੇ (exc == DOM_NO_ERR && val != ਨਿਰਾ) {
		ਜੇ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_ਪਾਠ_css)) {
			dom_string_unref(val);
			ਵਾਪਸ ਨਿਰਾ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ਜੇ (exc == DOM_NO_ERR && val != ਨਿਰਾ) {
		ਜੇ (strcasestr(dom_string_ਅੰਕੜੇ(val), "screen") == ਨਿਰਾ &&
				strcasestr(dom_string_ਅੰਕੜੇ(val),
						"all") == ਨਿਰਾ) {
			dom_string_unref(val);
			ਵਾਪਸ ਨਿਰਾ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      ਆਕਾਰ(ਬਣਤਰ html_stylesheet) *
			      (c->stylesheet_ਗਿਣਤੀ + 1));
	ਜੇ (stylesheets == ਨਿਰਾ) {

		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_ਗਿਣਤੀ].ਗੰਢ = dom_ਗੰਢ_ref(style);
	c->stylesheets[c->stylesheet_ਗਿਣਤੀ].sheet = ਨਿਰਾ;
	c->stylesheets[c->stylesheet_ਗਿਣਤੀ].modified = false;
	c->stylesheets[c->stylesheet_ਗਿਣਤੀ].ਅਣਵਰਤਿਆ = false;
	c->stylesheet_ਗਿਣਤੀ++;

	ਵਾਪਸ c->stylesheets + (c->stylesheet_ਗਿਣਤੀ - 1);
}


ਸਥਿਰ bool
html_css_process_modified_style(html_content *c, ਬਣਤਰ html_stylesheet *s)
{
	hlcache_handle *sheet = ਨਿਰਾ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ਗੰਢ, &sheet);
	ਜੇ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, error, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਜੇ (sheet != ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ਜੇ (s->sheet != ਨਿਰਾ) {
			ਚੋਣ (content_get_ਸਥਿਤੀ(s->sheet)) {
			ਹਾਲਤ CONTENT_ਸਥਿਤੀ_ਪੂਰਾ:
				ਰੋਕੋ;
			ਮੂਲ:
				hlcache_handle_abort(s->sheet);
				c->ਅਧਾਰ.ਸਰਗਰਮ--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->ਅਧਾਰ.ਸਰਗਰਮ);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	ਵਾਪਸ true;
}


/**
 * process a stylesheet that has been modified.
 */
ਸਥਿਰ ਖਾਲੀ html_css_process_modified_styles(ਖਾਲੀ *pw)
{
	html_content *c = pw;
	ਬਣਤਰ html_stylesheet *s;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	bool all_ਪੂਰਾ = true;

	ਲਈ (i = 0, s = c->stylesheets; i != c->stylesheet_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (c->stylesheets[i].modified) {
			all_ਪੂਰਾ &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ਜੇ (all_ਪੂਰਾ == false) {
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ਗੰਢ *style)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ html_stylesheet *s;

	/* Find sheet */
	ਲਈ (i = 0, s = c->stylesheets;	i != c->stylesheet_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->ਗੰਢ == style)
			ਰੋਕੋ;
	}
	ਜੇ (i == c->stylesheet_ਗਿਣਤੀ) {
		s = html_create_style_element(c, style);
	}
	ਜੇ (s == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ਵਾਪਸ false;
	}

	s->modified = true;

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(0, html_css_process_modified_styles, c);

	ਵਾਪਸ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ਗੰਢ *ਗੰਢ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	dom_string *val;
	dom_exception exc;
	ਬਣਤਰ html_stylesheet *s;

	/* Find sheet */
	ਲਈ (i = 0, s = c->stylesheets;	i != c->stylesheet_ਗਿਣਤੀ; i++, s++) {
		ਜੇ (s->ਗੰਢ == ਗੰਢ)
			ਰੋਕੋ;
	}

		/* Should already exist */
	ਜੇ (i == c->stylesheet_ਗਿਣਤੀ) {
		ਵਾਪਸ false;
	}

	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_media, &val);
	ਜੇ (exc == DOM_NO_ERR && val != ਨਿਰਾ) {
		ਜੇ (strcasestr(dom_string_ਅੰਕੜੇ(val), "screen") == ਨਿਰਾ &&
				strcasestr(dom_string_ਅੰਕੜੇ(val),
						"all") == ਨਿਰਾ) {
			s->ਅਣਵਰਤਿਆ = true;
		}
		dom_string_unref(val);
	}

	ਵਾਪਸ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ਗੰਢ *ਗੰਢ)
{
	dom_string *rel, *type_attr, *media, *href;
	ਬਣਤਰ html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_rel, &rel);
	ਜੇ (exc != DOM_NO_ERR || rel == ਨਿਰਾ)
		ਵਾਪਸ true;

	ਜੇ (strcasestr(dom_string_ਅੰਕੜੇ(rel), "stylesheet") == ਨਿਰਾ) {
		dom_string_unref(rel);
		ਵਾਪਸ true;
	} ਨਹੀਂਤਾਂ ਜੇ (strcasestr(dom_string_ਅੰਕੜੇ(rel), "alternate") != ਨਿਰਾ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ਵਾਪਸ true;
	}
	dom_string_unref(rel);

	ਜੇ (nsoption_bool(author_level_css) == false) {
		ਵਾਪਸ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_type, &type_attr);
	ਜੇ (exc == DOM_NO_ERR && type_attr != ਨਿਰਾ) {
		ਜੇ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_ਪਾਠ_css)) {
			dom_string_unref(type_attr);
			ਵਾਪਸ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_media, &media);
	ਜੇ (exc == DOM_NO_ERR && media != ਨਿਰਾ) {
		ਜੇ (strcasestr(dom_string_ਅੰਕੜੇ(media), "screen") == ਨਿਰਾ &&
		    strcasestr(dom_string_ਅੰਕੜੇ(media), "all") == ਨਿਰਾ) {
			dom_string_unref(media);
			ਵਾਪਸ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_href, &href);
	ਜੇ (exc != DOM_NO_ERR || href == ਨਿਰਾ)
		ਵਾਪਸ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->ਅਧਾਰ_url, dom_string_ਅੰਕੜੇ(href), &joined);
	ਜੇ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_ਗਿਣਤੀ, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      ਆਕਾਰ(ਬਣਤਰ html_stylesheet) *
			      (htmlc->stylesheet_ਗਿਣਤੀ + 1));
	ਜੇ (stylesheets == ਨਿਰਾ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_ਗਿਣਤੀ].ਗੰਢ = ਨਿਰਾ;
	htmlc->stylesheets[htmlc->stylesheet_ਗਿਣਤੀ].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_ਗਿਣਤੀ].ਅਣਵਰਤਿਆ = false;

	/* start fetch */
	ਬੱਚਾ.charset = htmlc->encoding;
	ਬੱਚਾ.quirks = htmlc->ਅਧਾਰ.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->ਅਧਾਰ),
			ਨਿਰਾ, html_convert_css_callback,
			htmlc, &ਬੱਚਾ, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_ਗਿਣਤੀ].sheet);

	nsurl_unref(joined);

	ਜੇ (ns_error != NSERROR_OK)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	htmlc->stylesheet_ਗਿਣਤੀ++;

	htmlc->ਅਧਾਰ.ਸਰਗਰਮ++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->ਅਧਾਰ.ਸਰਗਰਮ);

	ਵਾਪਸ true;

no_ਸਮ੍ਰਿਤੀ:
	content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, ns_error, ਨਿਰਾ);
	ਵਾਪਸ false;
}


/* exported interface documented in html/html.h */
ਬਣਤਰ html_stylesheet *html_get_stylesheets(hlcache_handle *h, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(n != ਨਿਰਾ);

	*n = c->stylesheet_ਗਿਣਤੀ;

	ਵਾਪਸ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ਬਣਤਰ html_stylesheet *s;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਲਈ (i = 0, s = html->stylesheets; i < html->stylesheet_ਗਿਣਤੀ;
	     i++, s++) {
		ਜੇ (s->sheet != ਨਿਰਾ) {
			ਜੇ (content_saw_insecure_objects(s->sheet)) {
				ਵਾਪਸ true;
			}
		}
	}

	ਵਾਪਸ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, html_css_process_modified_styles, html);

	ਲਈ (i = 0; i != html->stylesheet_ਗਿਣਤੀ; i++) {
		ਜੇ (html->stylesheets[i].sheet != ਨਿਰਾ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ਜੇ (html->stylesheets[i].ਗੰਢ != ਨਿਰਾ) {
			dom_ਗੰਢ_unref(html->stylesheets[i].ਗੰਢ);
		}
	}
	ਮੁਕਤ(html->stylesheets);

	ਵਾਪਸ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(c->stylesheets != ਨਿਰਾ);

	ਜੇ (c->quirks == DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_ਭਰਿਆ) {
		ਬੱਚਾ.charset = c->encoding;
		ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
				html_convert_css_callback, c, &ਬੱਚਾ,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ਜੇ (ns_error != NSERROR_OK) {
			ਵਾਪਸ ns_error;
		}

		c->ਅਧਾਰ.ਸਰਗਰਮ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);
	}

	ਵਾਪਸ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_ਬੱਚਾ_context ਬੱਚਾ;

	ਜੇ (c->stylesheets != ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(STYLESHEET_ਸ਼ੁਰੂ,
			ਆਕਾਰ(ਬਣਤਰ html_stylesheet));
	ਜੇ (c->stylesheets == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_ਅਧਾਰ].sheet = ਨਿਰਾ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ਨਿਰਾ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ਨਿਰਾ;
	c->stylesheets[STYLESHEET_USER].sheet = ਨਿਰਾ;
	c->stylesheet_ਗਿਣਤੀ = STYLESHEET_ਸ਼ੁਰੂ;

	ਬੱਚਾ.charset = c->encoding;
	ਬੱਚਾ.quirks = c->ਅਧਾਰ.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
			html_convert_css_callback, c, &ਬੱਚਾ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_ਅਧਾਰ].sheet);
	ਜੇ (ns_error != NSERROR_OK) {
		ਵਾਪਸ ns_error;
	}

	c->ਅਧਾਰ.ਸਰਗਰਮ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);


	ਜੇ (nsoption_bool(ਖੰਡ_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
				html_convert_css_callback,
				c, &ਬੱਚਾ, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ਜੇ (ns_error != NSERROR_OK) {
			ਵਾਪਸ ns_error;
		}

		c->ਅਧਾਰ.ਸਰਗਰਮ++;
		NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->ਅਧਾਰ), ਨਿਰਾ,
			html_convert_css_callback, c, &ਬੱਚਾ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ਜੇ (ns_error != NSERROR_OK) {
		ਵਾਪਸ ns_error;
	}

	c->ਅਧਾਰ.ਸਰਗਰਮ++;
	NSLOG(netsurf, INFO, "%d fetches active", c->ਅਧਾਰ.ਸਰਗਰਮ);

	ਵਾਪਸ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ਜੇ (c->stylesheets[STYLESHEET_ਅਧਾਰ].sheet == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_CSS_ਅਧਾਰ;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ਜੇ (css_ret != CSS_OK) {
		ਵਾਪਸ css_error_ਮੰਜ਼ਿਲ_nserror(css_ret);
	}

	/* Add sheets to it */
	ਲਈ (i = STYLESHEET_ਅਧਾਰ; i != c->stylesheet_ਗਿਣਤੀ; i++) {
		ਅਚਲ ਬਣਤਰ html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ਨਿਰਾ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ਜੇ (hsheet->ਅਣਵਰਤਿਆ) {
			ਜਾਰੀ;
		}

		ਜੇ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ਨਹੀਂਤਾਂ ਜੇ (i < STYLESHEET_ਸ਼ੁਰੂ) {
			origin = CSS_ORIGIN_USER;
		}

		ਜੇ (hsheet->sheet != ਨਿਰਾ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ਜੇ (sheet != ਨਿਰਾ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ਜੇ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ਵਾਪਸ css_error_ਮੰਜ਼ਿਲ_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ਵਾਪਸ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(ਖਾਲੀ)
{
	nserror error;

	error = html_css_fetcher_ਰਜਿਸਟਰ();
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ਵਾਪਸ error;
}


/* exported function documented in html/css.h */
ਖਾਲੀ html_css_fini(ਖਾਲੀ)
{
	ਜੇ (html_user_stylesheet_url != ਨਿਰਾ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ਨਿਰਾ;
	}

	ਜੇ (html_quirks_stylesheet_url != ਨਿਰਾ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ਨਿਰਾ;
	}

	ਜੇ (html_adblock_stylesheet_url != ਨਿਰਾ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ਨਿਰਾ;
	}

	ਜੇ (html_default_stylesheet_url != ਨਿਰਾ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ਨਿਰਾ;
	}
}
