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

#അന്തര്ഭാവ "utils/config.h"

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "css/css.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/css.h"

സ്ഥിതം nsurl *html_default_stylesheet_url;
സ്ഥിതം nsurl *html_adblock_stylesheet_url;
സ്ഥിതം nsurl *html_quirks_stylesheet_url;
സ്ഥിതം nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
സ്ഥിതം nserror css_error_to_nserror(css_error error)
{
	തിരഞ്ഞെടുപ്പ് (error) {
	അവസ്ഥ CSS_OK:
		തിരികെ NSERROR_OK;

	അവസ്ഥ CSS_NOMEM:
		തിരികെ NSERROR_NOMEM;

	അവസ്ഥ CSS_BADPARM:
		തിരികെ NSERROR_BAD_PARAMETER;

	അവസ്ഥ CSS_INVALID:
		തിരികെ NSERROR_INVALID;

	അവസ്ഥ CSS_FILENOTFOUND:
		തിരികെ NSERROR_NOT_FOUND;

	അവസ്ഥ CSS_NEEDDATA:
		തിരികെ NSERROR_NEED_DATA;

	അവസ്ഥ CSS_BADCHARSET:
		തിരികെ NSERROR_BAD_ENCODING;

	അവസ്ഥ CSS_EOF:
	അവസ്ഥ CSS_IMPORTS_PENDING:
	അവസ്ഥ CSS_PROPERTY_NOT_SET:
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}
	തിരികെ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
സ്ഥിതം nserror
html_convert_css_callback(hlcache_handle *css,
			  സ്ഥിരം hlcache_event *event,
			  ശൂന്യം *pw)
{
	html_content *parent = pw;
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_stylesheet *s;

	/* Find sheet */
	വേണ്ടി (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_എണ്ണം;
	     i++, s++) {
		എങ്കിൽ (s->sheet == css)
			നിർത്തുക;
	}

	പ്രതിജ്ഞാ(i != parent->stylesheet_എണ്ണം);

	തിരഞ്ഞെടുപ്പ് (event->type) {

	അവസ്ഥ CONTENT_സന്ദേശം_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ശൂന്യ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		നിർത്തുക;

	അവസ്ഥ CONTENT_സന്ദേശം_POINTER:
		/* Really don't want this to continue after the switch */
		തിരികെ NSERROR_OK;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	എങ്കിൽ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	തിരികെ NSERROR_OK;
}


സ്ഥിതം nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_കണ്ണി *കണ്ണി,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t താക്കോൽ;
	അക്ഷരം urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_കണ്ണി_get_text_content(കണ്ണി, &style);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (style == ശൂന്യ)) {
		NSLOG(netsurf, INFO, "No text content");
		തിരികെ NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &താക്കോൽ);
	എങ്കിൽ (error != NSERROR_OK) {
		dom_string_unref(style);
		തിരികെ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, വലുപ്പം(urlbuf), "x-ns-css:%"PRIu32"", താക്കോൽ);

	error = nsurl_create(urlbuf, &url);
	എങ്കിൽ (error != NSERROR_OK) {
		തിരികെ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), ശൂന്യ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	എങ്കിൽ (error != NSERROR_OK) {
		nsurl_unref(url);
		തിരികെ error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	തിരികെ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
സ്ഥിതം ഘടന html_stylesheet *
html_create_style_element(html_content *c, dom_കണ്ണി *style)
{
	dom_string *val;
	dom_exception exc;
	ഘടന html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	എങ്കിൽ (exc == DOM_NO_ERR && val != ശൂന്യ) {
		എങ്കിൽ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			തിരികെ ശൂന്യ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	എങ്കിൽ (exc == DOM_NO_ERR && val != ശൂന്യ) {
		എങ്കിൽ (strcasestr(dom_string_data(val), "screen") == ശൂന്യ &&
				strcasestr(dom_string_data(val),
						"all") == ശൂന്യ) {
			dom_string_unref(val);
			തിരികെ ശൂന്യ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      വലുപ്പം(ഘടന html_stylesheet) *
			      (c->stylesheet_എണ്ണം + 1));
	എങ്കിൽ (stylesheets == ശൂന്യ) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_എണ്ണം].കണ്ണി = dom_കണ്ണി_ref(style);
	c->stylesheets[c->stylesheet_എണ്ണം].sheet = ശൂന്യ;
	c->stylesheets[c->stylesheet_എണ്ണം].modified = false;
	c->stylesheets[c->stylesheet_എണ്ണം].unused = false;
	c->stylesheet_എണ്ണം++;

	തിരികെ c->stylesheets + (c->stylesheet_എണ്ണം - 1);
}


സ്ഥിതം bool
html_css_process_modified_style(html_content *c, ഘടന html_stylesheet *s)
{
	hlcache_handle *sheet = ശൂന്യ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->കണ്ണി, &sheet);
	എങ്കിൽ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, ശൂന്യ);
		തിരികെ false;
	}

	എങ്കിൽ (sheet != ശൂന്യ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		എങ്കിൽ (s->sheet != ശൂന്യ) {
			തിരഞ്ഞെടുപ്പ് (content_get_status(s->sheet)) {
			അവസ്ഥ CONTENT_STATUS_DONE:
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
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

	തിരികെ true;
}


/**
 * process a stylesheet that has been modified.
 */
സ്ഥിതം ശൂന്യം html_css_process_modified_styles(ശൂന്യം *pw)
{
	html_content *c = pw;
	ഘടന html_stylesheet *s;
	unsigned പൂർണ്ണസംഖ്യ i;
	bool all_done = true;

	വേണ്ടി (i = 0, s = c->stylesheets; i != c->stylesheet_എണ്ണം; i++, s++) {
		എങ്കിൽ (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	എങ്കിൽ (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_കണ്ണി *style)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന html_stylesheet *s;

	/* Find sheet */
	വേണ്ടി (i = 0, s = c->stylesheets;	i != c->stylesheet_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->കണ്ണി == style)
			നിർത്തുക;
	}
	എങ്കിൽ (i == c->stylesheet_എണ്ണം) {
		s = html_create_style_element(c, style);
	}
	എങ്കിൽ (s == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		തിരികെ false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	തിരികെ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_കണ്ണി *കണ്ണി)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	dom_string *val;
	dom_exception exc;
	ഘടന html_stylesheet *s;

	/* Find sheet */
	വേണ്ടി (i = 0, s = c->stylesheets;	i != c->stylesheet_എണ്ണം; i++, s++) {
		എങ്കിൽ (s->കണ്ണി == കണ്ണി)
			നിർത്തുക;
	}

		/* Should already exist */
	എങ്കിൽ (i == c->stylesheet_എണ്ണം) {
		തിരികെ false;
	}

	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_media, &val);
	എങ്കിൽ (exc == DOM_NO_ERR && val != ശൂന്യ) {
		എങ്കിൽ (strcasestr(dom_string_data(val), "screen") == ശൂന്യ &&
				strcasestr(dom_string_data(val),
						"all") == ശൂന്യ) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	തിരികെ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_കണ്ണി *കണ്ണി)
{
	dom_string *rel, *type_attr, *media, *href;
	ഘടന html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_rel, &rel);
	എങ്കിൽ (exc != DOM_NO_ERR || rel == ശൂന്യ)
		തിരികെ true;

	എങ്കിൽ (strcasestr(dom_string_data(rel), "stylesheet") == ശൂന്യ) {
		dom_string_unref(rel);
		തിരികെ true;
	} അല്ലെങ്കിൽ എങ്കിൽ (strcasestr(dom_string_data(rel), "alternate") != ശൂന്യ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		തിരികെ true;
	}
	dom_string_unref(rel);

	എങ്കിൽ (nsoption_bool(author_level_css) == false) {
		തിരികെ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_type, &type_attr);
	എങ്കിൽ (exc == DOM_NO_ERR && type_attr != ശൂന്യ) {
		എങ്കിൽ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			തിരികെ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_media, &media);
	എങ്കിൽ (exc == DOM_NO_ERR && media != ശൂന്യ) {
		എങ്കിൽ (strcasestr(dom_string_data(media), "screen") == ശൂന്യ &&
		    strcasestr(dom_string_data(media), "all") == ശൂന്യ) {
			dom_string_unref(media);
			തിരികെ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_href, &href);
	എങ്കിൽ (exc != DOM_NO_ERR || href == ശൂന്യ)
		തിരികെ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	എങ്കിൽ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		പോകുക no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_എണ്ണം, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      വലുപ്പം(ഘടന html_stylesheet) *
			      (htmlc->stylesheet_എണ്ണം + 1));
	എങ്കിൽ (stylesheets == ശൂന്യ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		പോകുക no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_എണ്ണം].കണ്ണി = ശൂന്യ;
	htmlc->stylesheets[htmlc->stylesheet_എണ്ണം].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_എണ്ണം].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			ശൂന്യ, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_എണ്ണം].sheet);

	nsurl_unref(joined);

	എങ്കിൽ (ns_error != NSERROR_OK)
		പോകുക no_memory;

	htmlc->stylesheet_എണ്ണം++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	തിരികെ true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, ശൂന്യ);
	തിരികെ false;
}


/* exported interface documented in html/html.h */
ഘടന html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned പൂർണ്ണസംഖ്യ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);
	പ്രതിജ്ഞാ(n != ശൂന്യ);

	*n = c->stylesheet_എണ്ണം;

	തിരികെ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ഘടന html_stylesheet *s;
	unsigned പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0, s = html->stylesheets; i < html->stylesheet_എണ്ണം;
	     i++, s++) {
		എങ്കിൽ (s->sheet != ശൂന്യ) {
			എങ്കിൽ (content_saw_insecure_objects(s->sheet)) {
				തിരികെ true;
			}
		}
	}

	തിരികെ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	വേണ്ടി (i = 0; i != html->stylesheet_എണ്ണം; i++) {
		എങ്കിൽ (html->stylesheets[i].sheet != ശൂന്യ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		എങ്കിൽ (html->stylesheets[i].കണ്ണി != ശൂന്യ) {
			dom_കണ്ണി_unref(html->stylesheets[i].കണ്ണി);
		}
	}
	സ്വതന്ത്രം(html->stylesheets);

	തിരികെ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	പ്രതിജ്ഞാ(c->stylesheets != ശൂന്യ);

	എങ്കിൽ (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), ശൂന്യ,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		എങ്കിൽ (ns_error != NSERROR_OK) {
			തിരികെ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	തിരികെ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	എങ്കിൽ (c->stylesheets != ശൂന്യ) {
		തിരികെ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			വലുപ്പം(ഘടന html_stylesheet));
	എങ്കിൽ (c->stylesheets == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = ശൂന്യ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ശൂന്യ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ശൂന്യ;
	c->stylesheets[STYLESHEET_USER].sheet = ശൂന്യ;
	c->stylesheet_എണ്ണം = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), ശൂന്യ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	എങ്കിൽ (ns_error != NSERROR_OK) {
		തിരികെ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	എങ്കിൽ (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), ശൂന്യ,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		എങ്കിൽ (ns_error != NSERROR_OK) {
			തിരികെ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), ശൂന്യ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	എങ്കിൽ (ns_error != NSERROR_OK) {
		തിരികെ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	തിരികെ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	എങ്കിൽ (c->stylesheets[STYLESHEET_BASE].sheet == ശൂന്യ) {
		തിരികെ NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	എങ്കിൽ (css_ret != CSS_OK) {
		തിരികെ css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	വേണ്ടി (i = STYLESHEET_BASE; i != c->stylesheet_എണ്ണം; i++) {
		സ്ഥിരം ഘടന html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ശൂന്യ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		എങ്കിൽ (hsheet->unused) {
			തുടരുക;
		}

		എങ്കിൽ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} അല്ലെങ്കിൽ എങ്കിൽ (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		എങ്കിൽ (hsheet->sheet != ശൂന്യ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		എങ്കിൽ (sheet != ശൂന്യ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			എങ്കിൽ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				തിരികെ css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	തിരികെ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(ശൂന്യം)
{
	nserror error;

	error = html_css_fetcher_register();
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	തിരികെ error;
}


/* exported function documented in html/css.h */
ശൂന്യം html_css_fini(ശൂന്യം)
{
	എങ്കിൽ (html_user_stylesheet_url != ശൂന്യ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ശൂന്യ;
	}

	എങ്കിൽ (html_quirks_stylesheet_url != ശൂന്യ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ശൂന്യ;
	}

	എങ്കിൽ (html_adblock_stylesheet_url != ശൂന്യ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ശൂന്യ;
	}

	എങ്കിൽ (html_default_stylesheet_url != ശൂന്യ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ശൂന്യ;
	}
}
