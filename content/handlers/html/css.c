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

#অন্তর্ভাৱ "utils/config.h"

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "netsurf/inttypes.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "content/hlcache.h"
#অন্তর্ভাৱ "css/css.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "html/html.h"
#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/css.h"

স্থির nsurl *html_default_stylesheet_url;
স্থির nsurl *html_adblock_stylesheet_url;
স্থির nsurl *html_quirks_stylesheet_url;
স্থির nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
স্থির nserror css_error_to_nserror(css_error error)
{
	ৱিকল্প (error) {
	পক্ষ CSS_OK:
		ঘূৰাই NSERROR_OK;

	পক্ষ CSS_NOMEM:
		ঘূৰাই NSERROR_NOMEM;

	পক্ষ CSS_BADPARM:
		ঘূৰাই NSERROR_BAD_PARAMETER;

	পক্ষ CSS_INVALID:
		ঘূৰাই NSERROR_INVALID;

	পক্ষ CSS_FILENOTFOUND:
		ঘূৰাই NSERROR_NOT_FOUND;

	পক্ষ CSS_NEEDDATA:
		ঘূৰাই NSERROR_NEED_DATA;

	পক্ষ CSS_BADCHARSET:
		ঘূৰাই NSERROR_BAD_ENCODING;

	পক্ষ CSS_EOF:
	পক্ষ CSS_IMPORTS_PENDING:
	পক্ষ CSS_PROPERTY_NOT_SET:
	স্ৱভাৱ:
		ভঙ্গ;
	}
	ঘূৰাই NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
স্থির nserror
html_convert_css_callback(hlcache_handle *css,
			  ধ্রুৱ hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_stylesheet *s;

	/* Find sheet */
	বাবে (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_গুনতি;
	     i++, s++) {
		যদি (s->sheet == css)
			ভঙ্গ;
	}

	প্রতিজ্ঞা(i != parent->stylesheet_গুনতি);

	ৱিকল্প (event->type) {

	পক্ষ CONTENT_বাৰ্তা_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = রিক্ত;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ভঙ্গ;

	পক্ষ CONTENT_বাৰ্তা_POINTER:
		/* Really don't want this to continue after the switch */
		ঘূৰাই NSERROR_OK;

	স্ৱভাৱ:
		ভঙ্গ;
	}

	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ঘূৰাই NSERROR_OK;
}


স্থির nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_গ্ৰন্থি *গ্ৰন্থি,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t চাবি;
	ৱর্ণ urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_গ্ৰন্থি_get_text_content(গ্ৰন্থি, &style);
	যদি ((exc != DOM_NO_ERR) || (style == রিক্ত)) {
		NSLOG(netsurf, INFO, "No text content");
		ঘূৰাই NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &চাবি);
	যদি (error != NSERROR_OK) {
		dom_string_unref(style);
		ঘূৰাই error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, পরিমাণ(urlbuf), "x-ns-css:%"PRIu32"", চাবি);

	error = nsurl_create(urlbuf, &url);
	যদি (error != NSERROR_OK) {
		ঘূৰাই error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), রিক্ত,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	যদি (error != NSERROR_OK) {
		nsurl_unref(url);
		ঘূৰাই error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ঘূৰাই NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
স্থির সংরচনা html_stylesheet *
html_create_style_element(html_content *c, dom_গ্ৰন্থি *style)
{
	dom_string *val;
	dom_exception exc;
	সংরচনা html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	যদি (exc == DOM_NO_ERR && val != রিক্ত) {
		যদি (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ঘূৰাই রিক্ত;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	যদি (exc == DOM_NO_ERR && val != রিক্ত) {
		যদি (strcasestr(dom_string_data(val), "screen") == রিক্ত &&
				strcasestr(dom_string_data(val),
						"all") == রিক্ত) {
			dom_string_unref(val);
			ঘূৰাই রিক্ত;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      পরিমাণ(সংরচনা html_stylesheet) *
			      (c->stylesheet_গুনতি + 1));
	যদি (stylesheets == রিক্ত) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_গুনতি].গ্ৰন্থি = dom_গ্ৰন্থি_ref(style);
	c->stylesheets[c->stylesheet_গুনতি].sheet = রিক্ত;
	c->stylesheets[c->stylesheet_গুনতি].modified = false;
	c->stylesheets[c->stylesheet_গুনতি].unused = false;
	c->stylesheet_গুনতি++;

	ঘূৰাই c->stylesheets + (c->stylesheet_গুনতি - 1);
}


স্থির bool
html_css_process_modified_style(html_content *c, সংরচনা html_stylesheet *s)
{
	hlcache_handle *sheet = রিক্ত;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->গ্ৰন্থি, &sheet);
	যদি (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, রিক্ত);
		ঘূৰাই false;
	}

	যদি (sheet != রিক্ত) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		যদি (s->sheet != রিক্ত) {
			ৱিকল্প (content_get_status(s->sheet)) {
			পক্ষ CONTENT_STATUS_DONE:
				ভঙ্গ;
			স্ৱভাৱ:
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

	ঘূৰাই true;
}


/**
 * process a stylesheet that has been modified.
 */
স্থির শূন্য html_css_process_modified_styles(শূন্য *pw)
{
	html_content *c = pw;
	সংরচনা html_stylesheet *s;
	unsigned পূর্ণাঙ্ক i;
	bool all_done = true;

	বাবে (i = 0, s = c->stylesheets; i != c->stylesheet_গুনতি; i++, s++) {
		যদি (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	যদি (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_গ্ৰন্থি *style)
{
	unsigned পূর্ণাঙ্ক i;
	সংরচনা html_stylesheet *s;

	/* Find sheet */
	বাবে (i = 0, s = c->stylesheets;	i != c->stylesheet_গুনতি; i++, s++) {
		যদি (s->গ্ৰন্থি == style)
			ভঙ্গ;
	}
	যদি (i == c->stylesheet_গুনতি) {
		s = html_create_style_element(c, style);
	}
	যদি (s == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ঘূৰাই false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ঘূৰাই true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	unsigned পূর্ণাঙ্ক i;
	dom_string *val;
	dom_exception exc;
	সংরচনা html_stylesheet *s;

	/* Find sheet */
	বাবে (i = 0, s = c->stylesheets;	i != c->stylesheet_গুনতি; i++, s++) {
		যদি (s->গ্ৰন্থি == গ্ৰন্থি)
			ভঙ্গ;
	}

		/* Should already exist */
	যদি (i == c->stylesheet_গুনতি) {
		ঘূৰাই false;
	}

	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_media, &val);
	যদি (exc == DOM_NO_ERR && val != রিক্ত) {
		যদি (strcasestr(dom_string_data(val), "screen") == রিক্ত &&
				strcasestr(dom_string_data(val),
						"all") == রিক্ত) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ঘূৰাই true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_string *rel, *type_attr, *media, *href;
	সংরচনা html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_rel, &rel);
	যদি (exc != DOM_NO_ERR || rel == রিক্ত)
		ঘূৰাই true;

	যদি (strcasestr(dom_string_data(rel), "stylesheet") == রিক্ত) {
		dom_string_unref(rel);
		ঘূৰাই true;
	} অন্যথা যদি (strcasestr(dom_string_data(rel), "alternate") != রিক্ত) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ঘূৰাই true;
	}
	dom_string_unref(rel);

	যদি (nsoption_bool(author_level_css) == false) {
		ঘূৰাই true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_type, &type_attr);
	যদি (exc == DOM_NO_ERR && type_attr != রিক্ত) {
		যদি (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ঘূৰাই true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_media, &media);
	যদি (exc == DOM_NO_ERR && media != রিক্ত) {
		যদি (strcasestr(dom_string_data(media), "screen") == রিক্ত &&
		    strcasestr(dom_string_data(media), "all") == রিক্ত) {
			dom_string_unref(media);
			ঘূৰাই true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_href, &href);
	যদি (exc != DOM_NO_ERR || href == রিক্ত)
		ঘূৰাই true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	যদি (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		গচ্ছ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_গুনতি, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      পরিমাণ(সংরচনা html_stylesheet) *
			      (htmlc->stylesheet_গুনতি + 1));
	যদি (stylesheets == রিক্ত) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		গচ্ছ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].গ্ৰন্থি = রিক্ত;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			রিক্ত, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_গুনতি].sheet);

	nsurl_unref(joined);

	যদি (ns_error != NSERROR_OK)
		গচ্ছ no_memory;

	htmlc->stylesheet_গুনতি++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ঘূৰাই true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, রিক্ত);
	ঘূৰাই false;
}


/* exported interface documented in html/html.h */
সংরচনা html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned পূর্ণাঙ্ক *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);
	প্রতিজ্ঞা(n != রিক্ত);

	*n = c->stylesheet_গুনতি;

	ঘূৰাই c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	সংরচনা html_stylesheet *s;
	unsigned পূর্ণাঙ্ক i;

	বাবে (i = 0, s = html->stylesheets; i < html->stylesheet_গুনতি;
	     i++, s++) {
		যদি (s->sheet != রিক্ত) {
			যদি (content_saw_insecure_objects(s->sheet)) {
				ঘূৰাই true;
			}
		}
	}

	ঘূৰাই false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned পূর্ণাঙ্ক i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	বাবে (i = 0; i != html->stylesheet_গুনতি; i++) {
		যদি (html->stylesheets[i].sheet != রিক্ত) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		যদি (html->stylesheets[i].গ্ৰন্থি != রিক্ত) {
			dom_গ্ৰন্থি_unref(html->stylesheets[i].গ্ৰন্থি);
		}
	}
	মুক্ত(html->stylesheets);

	ঘূৰাই NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	প্রতিজ্ঞা(c->stylesheets != রিক্ত);

	যদি (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), রিক্ত,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		যদি (ns_error != NSERROR_OK) {
			ঘূৰাই ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ঘূৰাই ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	যদি (c->stylesheets != রিক্ত) {
		ঘূৰাই NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			পরিমাণ(সংরচনা html_stylesheet));
	যদি (c->stylesheets == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = রিক্ত;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = রিক্ত;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = রিক্ত;
	c->stylesheets[STYLESHEET_USER].sheet = রিক্ত;
	c->stylesheet_গুনতি = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), রিক্ত,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	যদি (ns_error != NSERROR_OK) {
		ঘূৰাই ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	যদি (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), রিক্ত,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		যদি (ns_error != NSERROR_OK) {
			ঘূৰাই ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), রিক্ত,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	যদি (ns_error != NSERROR_OK) {
		ঘূৰাই ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ঘূৰাই ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	যদি (c->stylesheets[STYLESHEET_BASE].sheet == রিক্ত) {
		ঘূৰাই NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	যদি (css_ret != CSS_OK) {
		ঘূৰাই css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	বাবে (i = STYLESHEET_BASE; i != c->stylesheet_গুনতি; i++) {
		ধ্রুৱ সংরচনা html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = রিক্ত;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		যদি (hsheet->unused) {
			অনুৱর্ত;
		}

		যদি (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} অন্যথা যদি (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		যদি (hsheet->sheet != রিক্ত) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		যদি (sheet != রিক্ত) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			যদি (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ঘূৰাই css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ঘূৰাই NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(শূন্য)
{
	nserror error;

	error = html_css_fetcher_register();
	যদি (error != NSERROR_OK)
		ঘূৰাই error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	যদি (error != NSERROR_OK)
		ঘূৰাই error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	যদি (error != NSERROR_OK)
		ঘূৰাই error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	যদি (error != NSERROR_OK)
		ঘূৰাই error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ঘূৰাই error;
}


/* exported function documented in html/css.h */
শূন্য html_css_fini(শূন্য)
{
	যদি (html_user_stylesheet_url != রিক্ত) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = রিক্ত;
	}

	যদি (html_quirks_stylesheet_url != রিক্ত) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = রিক্ত;
	}

	যদি (html_adblock_stylesheet_url != রিক্ত) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = রিক্ত;
	}

	যদি (html_default_stylesheet_url != রিক্ত) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = রিক্ত;
	}
}
