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

#অন্তর্ভাব "utils/config.h"

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "css/css.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/css.h"

স্থির nsurl *html_default_stylesheet_url;
স্থির nsurl *html_adblock_stylesheet_url;
স্থির nsurl *html_quirks_stylesheet_url;
স্থির nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
স্থির nserror css_error_to_nserror(css_error error)
{
	নির্বাচন (error) {
	ক্ষেত্র CSS_OK:
		ফেরত NSERROR_OK;

	ক্ষেত্র CSS_NOMEM:
		ফেরত NSERROR_NOMEM;

	ক্ষেত্র CSS_BADPARM:
		ফেরত NSERROR_BAD_PARAMETER;

	ক্ষেত্র CSS_INVALID:
		ফেরত NSERROR_INVALID;

	ক্ষেত্র CSS_FILENOTFOUND:
		ফেরত NSERROR_NOT_FOUND;

	ক্ষেত্র CSS_NEEDDATA:
		ফেরত NSERROR_NEED_তথ্য;

	ক্ষেত্র CSS_BADCHARSET:
		ফেরত NSERROR_BAD_ENCODING;

	ক্ষেত্র CSS_EOF:
	ক্ষেত্র CSS_IMPORTS_PENDING:
	ক্ষেত্র CSS_PROPERTY_NOT_SET:
	পূর্বনির্ধারিত:
		বিরতি;
	}
	ফেরত NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
স্থির nserror
html_convert_css_callback(hlcache_handle *css,
			  ধ্রুবক hlcache_event *event,
			  শূন্য *pw)
{
	html_content *parent = pw;
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_stylesheet *s;

	/* Find sheet */
	জন্য (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_গুনতি;
	     i++, s++) {
		যদি (s->sheet == css)
			বিরতি;
	}

	প্রতিজ্ঞা(i != parent->stylesheet_গুনতি);

	নির্বাচন (event->type) {

	ক্ষেত্র CONTENT_বার্তা_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->তথ্য.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ফাঁকা;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		বিরতি;

	ক্ষেত্র CONTENT_বার্তা_POINTER:
		/* Really don't want this to continue after the switch */
		ফেরত NSERROR_OK;

	পূর্বনির্ধারিত:
		বিরতি;
	}

	যদি (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ফেরত NSERROR_OK;
}


স্থির nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_গ্রন্থি *গ্রন্থি,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t চাবি;
	অক্ষর urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_গ্রন্থি_get_text_content(গ্রন্থি, &style);
	যদি ((exc != DOM_NO_ERR) || (style == ফাঁকা)) {
		NSLOG(netsurf, INFO, "No text content");
		ফেরত NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &চাবি);
	যদি (error != NSERROR_OK) {
		dom_string_unref(style);
		ফেরত error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, আকার(urlbuf), "x-ns-css:%"PRIu32"", চাবি);

	error = nsurl_create(urlbuf, &url);
	যদি (error != NSERROR_OK) {
		ফেরত error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), ফাঁকা,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	যদি (error != NSERROR_OK) {
		nsurl_unref(url);
		ফেরত error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ফেরত NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
স্থির কাঠামো html_stylesheet *
html_create_style_element(html_content *c, dom_গ্রন্থি *style)
{
	dom_string *val;
	dom_exception exc;
	কাঠামো html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	যদি (exc == DOM_NO_ERR && val != ফাঁকা) {
		যদি (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ফেরত ফাঁকা;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	যদি (exc == DOM_NO_ERR && val != ফাঁকা) {
		যদি (strcasestr(dom_string_তথ্য(val), "screen") == ফাঁকা &&
				strcasestr(dom_string_তথ্য(val),
						"all") == ফাঁকা) {
			dom_string_unref(val);
			ফেরত ফাঁকা;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      আকার(কাঠামো html_stylesheet) *
			      (c->stylesheet_গুনতি + 1));
	যদি (stylesheets == ফাঁকা) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_গুনতি].গ্রন্থি = dom_গ্রন্থি_ref(style);
	c->stylesheets[c->stylesheet_গুনতি].sheet = ফাঁকা;
	c->stylesheets[c->stylesheet_গুনতি].modified = false;
	c->stylesheets[c->stylesheet_গুনতি].unused = false;
	c->stylesheet_গুনতি++;

	ফেরত c->stylesheets + (c->stylesheet_গুনতি - 1);
}


স্থির bool
html_css_process_modified_style(html_content *c, কাঠামো html_stylesheet *s)
{
	hlcache_handle *sheet = ফাঁকা;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->গ্রন্থি, &sheet);
	যদি (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, ফাঁকা);
		ফেরত false;
	}

	যদি (sheet != ফাঁকা) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		যদি (s->sheet != ফাঁকা) {
			নির্বাচন (content_get_status(s->sheet)) {
			ক্ষেত্র CONTENT_STATUS_DONE:
				বিরতি;
			পূর্বনির্ধারিত:
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

	ফেরত true;
}


/**
 * process a stylesheet that has been modified.
 */
স্থির শূন্য html_css_process_modified_styles(শূন্য *pw)
{
	html_content *c = pw;
	কাঠামো html_stylesheet *s;
	unsigned পূর্ণসংখ্যা i;
	bool all_done = true;

	জন্য (i = 0, s = c->stylesheets; i != c->stylesheet_গুনতি; i++, s++) {
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
bool html_css_update_style(html_content *c, dom_গ্রন্থি *style)
{
	unsigned পূর্ণসংখ্যা i;
	কাঠামো html_stylesheet *s;

	/* Find sheet */
	জন্য (i = 0, s = c->stylesheets;	i != c->stylesheet_গুনতি; i++, s++) {
		যদি (s->গ্রন্থি == style)
			বিরতি;
	}
	যদি (i == c->stylesheet_গুনতি) {
		s = html_create_style_element(c, style);
	}
	যদি (s == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ফেরত false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ফেরত true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_গ্রন্থি *গ্রন্থি)
{
	unsigned পূর্ণসংখ্যা i;
	dom_string *val;
	dom_exception exc;
	কাঠামো html_stylesheet *s;

	/* Find sheet */
	জন্য (i = 0, s = c->stylesheets;	i != c->stylesheet_গুনতি; i++, s++) {
		যদি (s->গ্রন্থি == গ্রন্থি)
			বিরতি;
	}

		/* Should already exist */
	যদি (i == c->stylesheet_গুনতি) {
		ফেরত false;
	}

	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_media, &val);
	যদি (exc == DOM_NO_ERR && val != ফাঁকা) {
		যদি (strcasestr(dom_string_তথ্য(val), "screen") == ফাঁকা &&
				strcasestr(dom_string_তথ্য(val),
						"all") == ফাঁকা) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ফেরত true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_গ্রন্থি *গ্রন্থি)
{
	dom_string *rel, *type_attr, *media, *href;
	কাঠামো html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_rel, &rel);
	যদি (exc != DOM_NO_ERR || rel == ফাঁকা)
		ফেরত true;

	যদি (strcasestr(dom_string_তথ্য(rel), "stylesheet") == ফাঁকা) {
		dom_string_unref(rel);
		ফেরত true;
	} অন্যথা যদি (strcasestr(dom_string_তথ্য(rel), "alternate") != ফাঁকা) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ফেরত true;
	}
	dom_string_unref(rel);

	যদি (nsoption_bool(author_level_css) == false) {
		ফেরত true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_type, &type_attr);
	যদি (exc == DOM_NO_ERR && type_attr != ফাঁকা) {
		যদি (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ফেরত true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_media, &media);
	যদি (exc == DOM_NO_ERR && media != ফাঁকা) {
		যদি (strcasestr(dom_string_তথ্য(media), "screen") == ফাঁকা &&
		    strcasestr(dom_string_তথ্য(media), "all") == ফাঁকা) {
			dom_string_unref(media);
			ফেরত true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(গ্রন্থি, corestring_dom_href, &href);
	যদি (exc != DOM_NO_ERR || href == ফাঁকা)
		ফেরত true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_তথ্য(href), &joined);
	যদি (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		যাও no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_গুনতি, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      আকার(কাঠামো html_stylesheet) *
			      (htmlc->stylesheet_গুনতি + 1));
	যদি (stylesheets == ফাঁকা) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		যাও no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].গ্রন্থি = ফাঁকা;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_গুনতি].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			ফাঁকা, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_গুনতি].sheet);

	nsurl_unref(joined);

	যদি (ns_error != NSERROR_OK)
		যাও no_memory;

	htmlc->stylesheet_গুনতি++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ফেরত true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, ফাঁকা);
	ফেরত false;
}


/* exported interface documented in html/html.h */
কাঠামো html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned পূর্ণসংখ্যা *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != ফাঁকা);
	প্রতিজ্ঞা(n != ফাঁকা);

	*n = c->stylesheet_গুনতি;

	ফেরত c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	কাঠামো html_stylesheet *s;
	unsigned পূর্ণসংখ্যা i;

	জন্য (i = 0, s = html->stylesheets; i < html->stylesheet_গুনতি;
	     i++, s++) {
		যদি (s->sheet != ফাঁকা) {
			যদি (content_saw_insecure_objects(s->sheet)) {
				ফেরত true;
			}
		}
	}

	ফেরত false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned পূর্ণসংখ্যা i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	জন্য (i = 0; i != html->stylesheet_গুনতি; i++) {
		যদি (html->stylesheets[i].sheet != ফাঁকা) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		যদি (html->stylesheets[i].গ্রন্থি != ফাঁকা) {
			dom_গ্রন্থি_unref(html->stylesheets[i].গ্রন্থি);
		}
	}
	মুক্ত(html->stylesheets);

	ফেরত NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	প্রতিজ্ঞা(c->stylesheets != ফাঁকা);

	যদি (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), ফাঁকা,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		যদি (ns_error != NSERROR_OK) {
			ফেরত ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ফেরত ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	যদি (c->stylesheets != ফাঁকা) {
		ফেরত NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			আকার(কাঠামো html_stylesheet));
	যদি (c->stylesheets == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = ফাঁকা;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ফাঁকা;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ফাঁকা;
	c->stylesheets[STYLESHEET_USER].sheet = ফাঁকা;
	c->stylesheet_গুনতি = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), ফাঁকা,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	যদি (ns_error != NSERROR_OK) {
		ফেরত ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	যদি (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), ফাঁকা,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		যদি (ns_error != NSERROR_OK) {
			ফেরত ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), ফাঁকা,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	যদি (ns_error != NSERROR_OK) {
		ফেরত ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ফেরত ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	যদি (c->stylesheets[STYLESHEET_BASE].sheet == ফাঁকা) {
		ফেরত NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	যদি (css_ret != CSS_OK) {
		ফেরত css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	জন্য (i = STYLESHEET_BASE; i != c->stylesheet_গুনতি; i++) {
		ধ্রুবক কাঠামো html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ফাঁকা;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		যদি (hsheet->unused) {
			চালু;
		}

		যদি (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} অন্যথা যদি (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		যদি (hsheet->sheet != ফাঁকা) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		যদি (sheet != ফাঁকা) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			যদি (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ফেরত css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ফেরত NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(শূন্য)
{
	nserror error;

	error = html_css_fetcher_register();
	যদি (error != NSERROR_OK)
		ফেরত error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	যদি (error != NSERROR_OK)
		ফেরত error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	যদি (error != NSERROR_OK)
		ফেরত error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	যদি (error != NSERROR_OK)
		ফেরত error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ফেরত error;
}


/* exported function documented in html/css.h */
শূন্য html_css_fini(শূন্য)
{
	যদি (html_user_stylesheet_url != ফাঁকা) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ফাঁকা;
	}

	যদি (html_quirks_stylesheet_url != ফাঁকা) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ফাঁকা;
	}

	যদি (html_adblock_stylesheet_url != ফাঁকা) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ফাঁকা;
	}

	যদি (html_default_stylesheet_url != ফাঁকা) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ফাঁকা;
	}
}
