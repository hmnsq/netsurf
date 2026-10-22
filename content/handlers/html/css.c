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

#ضمن "utils/config.h"

#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <stdlib.h>

#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/misc.h"
#ضمن "netsurf/content.h"
#ضمن "content/hlcache.h"
#ضمن "css/css.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/css.h"

ساكن nsurl *html_default_stylesheet_url;
ساكن nsurl *html_adblock_stylesheet_url;
ساكن nsurl *html_quirks_stylesheet_url;
ساكن nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ساكن nserror css_error_to_nserror(css_error error)
{
	بدل (error) {
	حالة CSS_OK:
		ܦܢܐ NSERROR_OK;

	حالة CSS_NOMEM:
		ܦܢܐ NSERROR_NOMEM;

	حالة CSS_BADPARM:
		ܦܢܐ NSERROR_BAD_PARAMETER;

	حالة CSS_INVALID:
		ܦܢܐ NSERROR_INVALID;

	حالة CSS_FILENOTFOUND:
		ܦܢܐ NSERROR_NOT_FOUND;

	حالة CSS_NEEDDATA:
		ܦܢܐ NSERROR_NEED_DATA;

	حالة CSS_BADCHARSET:
		ܦܢܐ NSERROR_BAD_ENCODING;

	حالة CSS_EOF:
	حالة CSS_IMPORTS_PENDING:
	حالة CSS_PROPERTY_NOT_SET:
	افتراضي:
		اكسر;
	}
	ܦܢܐ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ساكن nserror
html_convert_css_callback(hlcache_handle *css,
			  ثابت hlcache_event *event,
			  فراغ *pw)
{
	html_content *parent = pw;
	unsigned صحيح i;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_count;
	     i++, s++) {
		ܐܢ (s->sheet == css)
			اكسر;
	}

	أكد(i != parent->stylesheet_count);

	بدل (event->type) {

	حالة CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		اكسر;

	حالة CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = عدم;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		اكسر;

	حالة CONTENT_MSG_POINTER:
		/* Really don't want this to continue after the switch */
		ܦܢܐ NSERROR_OK;

	افتراضي:
		اكسر;
	}

	ܐܢ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ܦܢܐ NSERROR_OK;
}


ساكن nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_node *node,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t key;
	حرف urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_node_get_text_content(node, &style);
	ܐܢ ((exc != DOM_NO_ERR) || (style == عدم)) {
		NSLOG(netsurf, INFO, "No text content");
		ܦܢܐ NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &key);
	ܐܢ (error != NSERROR_OK) {
		dom_string_unref(style);
		ܦܢܐ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, حجم(urlbuf), "x-ns-css:%"PRIu32"", key);

	error = nsurl_create(urlbuf, &url);
	ܐܢ (error != NSERROR_OK) {
		ܦܢܐ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), عدم,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	ܐܢ (error != NSERROR_OK) {
		nsurl_unref(url);
		ܦܢܐ error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ܦܢܐ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ساكن بنية html_stylesheet *
html_create_style_element(html_content *c, dom_node *style)
{
	dom_string *val;
	dom_exception exc;
	بنية html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ܐܢ (exc == DOM_NO_ERR && val != عدم) {
		ܐܢ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ܦܢܐ عدم;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ܐܢ (exc == DOM_NO_ERR && val != عدم) {
		ܐܢ (strcasestr(dom_string_data(val), "screen") == عدم &&
				strcasestr(dom_string_data(val),
						"all") == عدم) {
			dom_string_unref(val);
			ܦܢܐ عدم;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      حجم(بنية html_stylesheet) *
			      (c->stylesheet_count + 1));
	ܐܢ (stylesheets == عدم) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		ܦܢܐ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_count].node = dom_node_ref(style);
	c->stylesheets[c->stylesheet_count].sheet = عدم;
	c->stylesheets[c->stylesheet_count].modified = false;
	c->stylesheets[c->stylesheet_count].unused = false;
	c->stylesheet_count++;

	ܦܢܐ c->stylesheets + (c->stylesheet_count - 1);
}


ساكن bool
html_css_process_modified_style(html_content *c, بنية html_stylesheet *s)
{
	hlcache_handle *sheet = عدم;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->node, &sheet);
	ܐܢ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, عدم);
		ܦܢܐ false;
	}

	ܐܢ (sheet != عدم) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ܐܢ (s->sheet != عدم) {
			بدل (content_get_status(s->sheet)) {
			حالة CONTENT_STATUS_DONE:
				اكسر;
			افتراضي:
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

	ܦܢܐ true;
}


/**
 * process a stylesheet that has been modified.
 */
ساكن فراغ html_css_process_modified_styles(فراغ *pw)
{
	html_content *c = pw;
	بنية html_stylesheet *s;
	unsigned صحيح i;
	bool all_done = true;

	لكل (i = 0, s = c->stylesheets; i != c->stylesheet_count; i++, s++) {
		ܐܢ (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ܐܢ (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_node *style)
{
	unsigned صحيح i;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		ܐܢ (s->node == style)
			اكسر;
	}
	ܐܢ (i == c->stylesheet_count) {
		s = html_create_style_element(c, style);
	}
	ܐܢ (s == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ܦܢܐ false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ܦܢܐ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_node *node)
{
	unsigned صحيح i;
	dom_string *val;
	dom_exception exc;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		ܐܢ (s->node == node)
			اكسر;
	}

		/* Should already exist */
	ܐܢ (i == c->stylesheet_count) {
		ܦܢܐ false;
	}

	exc = dom_element_get_attribute(node, corestring_dom_media, &val);
	ܐܢ (exc == DOM_NO_ERR && val != عدم) {
		ܐܢ (strcasestr(dom_string_data(val), "screen") == عدم &&
				strcasestr(dom_string_data(val),
						"all") == عدم) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ܦܢܐ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_node *node)
{
	dom_string *rel, *type_attr, *media, *href;
	بنية html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &rel);
	ܐܢ (exc != DOM_NO_ERR || rel == عدم)
		ܦܢܐ true;

	ܐܢ (strcasestr(dom_string_data(rel), "stylesheet") == عدم) {
		dom_string_unref(rel);
		ܦܢܐ true;
	} ܐܚܪܢܐ ܐܢ (strcasestr(dom_string_data(rel), "alternate") != عدم) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ܦܢܐ true;
	}
	dom_string_unref(rel);

	ܐܢ (nsoption_bool(author_level_css) == false) {
		ܦܢܐ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_type, &type_attr);
	ܐܢ (exc == DOM_NO_ERR && type_attr != عدم) {
		ܐܢ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ܦܢܐ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_media, &media);
	ܐܢ (exc == DOM_NO_ERR && media != عدم) {
		ܐܢ (strcasestr(dom_string_data(media), "screen") == عدم &&
		    strcasestr(dom_string_data(media), "all") == عدم) {
			dom_string_unref(media);
			ܦܢܐ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(node, corestring_dom_href, &href);
	ܐܢ (exc != DOM_NO_ERR || href == عدم)
		ܦܢܐ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	ܐܢ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		اذهب no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_count, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      حجم(بنية html_stylesheet) *
			      (htmlc->stylesheet_count + 1));
	ܐܢ (stylesheets == عدم) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		اذهب no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_count].node = عدم;
	htmlc->stylesheets[htmlc->stylesheet_count].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_count].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			عدم, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_count].sheet);

	nsurl_unref(joined);

	ܐܢ (ns_error != NSERROR_OK)
		اذهب no_memory;

	htmlc->stylesheet_count++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ܦܢܐ true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, عدم);
	ܦܢܐ false;
}


/* exported interface documented in html/html.h */
بنية html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned صحيح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->stylesheet_count;

	ܦܢܐ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	بنية html_stylesheet *s;
	unsigned صحيح i;

	لكل (i = 0, s = html->stylesheets; i < html->stylesheet_count;
	     i++, s++) {
		ܐܢ (s->sheet != عدم) {
			ܐܢ (content_saw_insecure_objects(s->sheet)) {
				ܦܢܐ true;
			}
		}
	}

	ܦܢܐ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned صحيح i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	لكل (i = 0; i != html->stylesheet_count; i++) {
		ܐܢ (html->stylesheets[i].sheet != عدم) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ܐܢ (html->stylesheets[i].node != عدم) {
			dom_node_unref(html->stylesheets[i].node);
		}
	}
	حرر(html->stylesheets);

	ܦܢܐ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	أكد(c->stylesheets != عدم);

	ܐܢ (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), عدم,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ܐܢ (ns_error != NSERROR_OK) {
			ܦܢܐ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ܦܢܐ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	ܐܢ (c->stylesheets != عدم) {
		ܦܢܐ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			حجم(بنية html_stylesheet));
	ܐܢ (c->stylesheets == عدم) {
		ܦܢܐ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = عدم;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = عدم;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = عدم;
	c->stylesheets[STYLESHEET_USER].sheet = عدم;
	c->stylesheet_count = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), عدم,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	ܐܢ (ns_error != NSERROR_OK) {
		ܦܢܐ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	ܐܢ (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), عدم,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ܐܢ (ns_error != NSERROR_OK) {
			ܦܢܐ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), عدم,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ܐܢ (ns_error != NSERROR_OK) {
		ܦܢܐ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ܦܢܐ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ܐܢ (c->stylesheets[STYLESHEET_BASE].sheet == عدم) {
		ܦܢܐ NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ܐܢ (css_ret != CSS_OK) {
		ܦܢܐ css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	لكل (i = STYLESHEET_BASE; i != c->stylesheet_count; i++) {
		ثابت بنية html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = عدم;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ܐܢ (hsheet->unused) {
			استمر;
		}

		ܐܢ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ܐܚܪܢܐ ܐܢ (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		ܐܢ (hsheet->sheet != عدم) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ܐܢ (sheet != عدم) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ܐܢ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ܦܢܐ css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ܦܢܐ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(فراغ)
{
	nserror error;

	error = html_css_fetcher_register();
	ܐܢ (error != NSERROR_OK)
		ܦܢܐ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ܐܢ (error != NSERROR_OK)
		ܦܢܐ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ܐܢ (error != NSERROR_OK)
		ܦܢܐ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ܐܢ (error != NSERROR_OK)
		ܦܢܐ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ܦܢܐ error;
}


/* exported function documented in html/css.h */
فراغ html_css_fini(فراغ)
{
	ܐܢ (html_user_stylesheet_url != عدم) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = عدم;
	}

	ܐܢ (html_quirks_stylesheet_url != عدم) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = عدم;
	}

	ܐܢ (html_adblock_stylesheet_url != عدم) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = عدم;
	}

	ܐܢ (html_default_stylesheet_url != عدم) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = عدم;
	}
}
