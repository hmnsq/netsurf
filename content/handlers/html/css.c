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

#شامل "utils/config.h"

#شامل <assert.h>
#شامل <ctype.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdlib.h>

#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/content.h"
#شامل "content/hlcache.h"
#شامل "css/css.h"
#شامل "desktop/gui_internal.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/css.h"

ایستا nsurl *html_default_stylesheet_url;
ایستا nsurl *html_adblock_stylesheet_url;
ایستا nsurl *html_quirks_stylesheet_url;
ایستا nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ایستا nserror css_error_to_nserror(css_error error)
{
	انتخاب (error) {
	حالت CSS_OK:
		بازگشت NSERROR_OK;

	حالت CSS_NOMEM:
		بازگشت NSERROR_NOMEM;

	حالت CSS_BADPARM:
		بازگشت NSERROR_BAD_PARAMETER;

	حالت CSS_INVALID:
		بازگشت NSERROR_INVALID;

	حالت CSS_FILENOTFOUND:
		بازگشت NSERROR_NOT_FOUND;

	حالت CSS_NEEDDATA:
		بازگشت NSERROR_NEED_DATA;

	حالت CSS_BADCHARSET:
		بازگشت NSERROR_BAD_ENCODING;

	حالت CSS_EOF:
	حالت CSS_IMPORTS_PENDING:
	حالت CSS_PROPERTY_NOT_SET:
	پیشفرض:
		بشکن;
	}
	بازگشت NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ایستا nserror
html_convert_css_callback(hlcache_handle *css,
			  ثابت hlcache_event *event,
			  تهی *pw)
{
	html_content *parent = pw;
	unsigned صحیح i;
	ساختار html_stylesheet *s;

	/* Find sheet */
	برای (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_شمار;
	     i++, s++) {
		اگر (s->sheet == css)
			بشکن;
	}

	أكد(i != parent->stylesheet_شمار);

	انتخاب (event->type) {

	حالت CONTENT_پیام_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		بشکن;

	حالت CONTENT_پیام_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = پوچ;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		بشکن;

	حالت CONTENT_پیام_POINTER:
		/* Really don't want this to continue after the switch */
		بازگشت NSERROR_OK;

	پیشفرض:
		بشکن;
	}

	اگر (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	بازگشت NSERROR_OK;
}


ایستا nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_گره *گره,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t کلید;
	نویسه urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_گره_get_text_content(گره, &style);
	اگر ((exc != DOM_NO_ERR) || (style == پوچ)) {
		NSLOG(netsurf, INFO, "No text content");
		بازگشت NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &کلید);
	اگر (error != NSERROR_OK) {
		dom_string_unref(style);
		بازگشت error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, اندازه(urlbuf), "x-ns-css:%"PRIu32"", کلید);

	error = nsurl_create(urlbuf, &url);
	اگر (error != NSERROR_OK) {
		بازگشت error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), پوچ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	اگر (error != NSERROR_OK) {
		nsurl_unref(url);
		بازگشت error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	بازگشت NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ایستا ساختار html_stylesheet *
html_create_style_element(html_content *c, dom_گره *style)
{
	dom_string *val;
	dom_exception exc;
	ساختار html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	اگر (exc == DOM_NO_ERR && val != پوچ) {
		اگر (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			بازگشت پوچ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	اگر (exc == DOM_NO_ERR && val != پوچ) {
		اگر (strcasestr(dom_string_data(val), "screen") == پوچ &&
				strcasestr(dom_string_data(val),
						"all") == پوچ) {
			dom_string_unref(val);
			بازگشت پوچ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      اندازه(ساختار html_stylesheet) *
			      (c->stylesheet_شمار + 1));
	اگر (stylesheets == پوچ) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
		بازگشت false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_شمار].گره = dom_گره_ref(style);
	c->stylesheets[c->stylesheet_شمار].sheet = پوچ;
	c->stylesheets[c->stylesheet_شمار].modified = false;
	c->stylesheets[c->stylesheet_شمار].unused = false;
	c->stylesheet_شمار++;

	بازگشت c->stylesheets + (c->stylesheet_شمار - 1);
}


ایستا bool
html_css_process_modified_style(html_content *c, ساختار html_stylesheet *s)
{
	hlcache_handle *sheet = پوچ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->گره, &sheet);
	اگر (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, پوچ);
		بازگشت false;
	}

	اگر (sheet != پوچ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		اگر (s->sheet != پوچ) {
			انتخاب (content_get_status(s->sheet)) {
			حالت CONTENT_STATUS_DONE:
				بشکن;
			پیشفرض:
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

	بازگشت true;
}


/**
 * process a stylesheet that has been modified.
 */
ایستا تهی html_css_process_modified_styles(تهی *pw)
{
	html_content *c = pw;
	ساختار html_stylesheet *s;
	unsigned صحیح i;
	bool all_done = true;

	برای (i = 0, s = c->stylesheets; i != c->stylesheet_شمار; i++, s++) {
		اگر (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	اگر (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_گره *style)
{
	unsigned صحیح i;
	ساختار html_stylesheet *s;

	/* Find sheet */
	برای (i = 0, s = c->stylesheets;	i != c->stylesheet_شمار; i++, s++) {
		اگر (s->گره == style)
			بشکن;
	}
	اگر (i == c->stylesheet_شمار) {
		s = html_create_style_element(c, style);
	}
	اگر (s == پوچ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		بازگشت false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	بازگشت true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_گره *گره)
{
	unsigned صحیح i;
	dom_string *val;
	dom_exception exc;
	ساختار html_stylesheet *s;

	/* Find sheet */
	برای (i = 0, s = c->stylesheets;	i != c->stylesheet_شمار; i++, s++) {
		اگر (s->گره == گره)
			بشکن;
	}

		/* Should already exist */
	اگر (i == c->stylesheet_شمار) {
		بازگشت false;
	}

	exc = dom_element_get_attribute(گره, corestring_dom_media, &val);
	اگر (exc == DOM_NO_ERR && val != پوچ) {
		اگر (strcasestr(dom_string_data(val), "screen") == پوچ &&
				strcasestr(dom_string_data(val),
						"all") == پوچ) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	بازگشت true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_گره *گره)
{
	dom_string *rel, *type_attr, *media, *href;
	ساختار html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(گره, corestring_dom_rel, &rel);
	اگر (exc != DOM_NO_ERR || rel == پوچ)
		بازگشت true;

	اگر (strcasestr(dom_string_data(rel), "stylesheet") == پوچ) {
		dom_string_unref(rel);
		بازگشت true;
	} وگرنه اگر (strcasestr(dom_string_data(rel), "alternate") != پوچ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		بازگشت true;
	}
	dom_string_unref(rel);

	اگر (nsoption_bool(author_level_css) == false) {
		بازگشت true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(گره, corestring_dom_type, &type_attr);
	اگر (exc == DOM_NO_ERR && type_attr != پوچ) {
		اگر (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			بازگشت true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(گره, corestring_dom_media, &media);
	اگر (exc == DOM_NO_ERR && media != پوچ) {
		اگر (strcasestr(dom_string_data(media), "screen") == پوچ &&
		    strcasestr(dom_string_data(media), "all") == پوچ) {
			dom_string_unref(media);
			بازگشت true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(گره, corestring_dom_href, &href);
	اگر (exc != DOM_NO_ERR || href == پوچ)
		بازگشت true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	اگر (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		برو no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_شمار, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      اندازه(ساختار html_stylesheet) *
			      (htmlc->stylesheet_شمار + 1));
	اگر (stylesheets == پوچ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		برو no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_شمار].گره = پوچ;
	htmlc->stylesheets[htmlc->stylesheet_شمار].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_شمار].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			پوچ, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_شمار].sheet);

	nsurl_unref(joined);

	اگر (ns_error != NSERROR_OK)
		برو no_memory;

	htmlc->stylesheet_شمار++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	بازگشت true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, پوچ);
	بازگشت false;
}


/* exported interface documented in html/html.h */
ساختار html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned صحیح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);
	أكد(n != پوچ);

	*n = c->stylesheet_شمار;

	بازگشت c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ساختار html_stylesheet *s;
	unsigned صحیح i;

	برای (i = 0, s = html->stylesheets; i < html->stylesheet_شمار;
	     i++, s++) {
		اگر (s->sheet != پوچ) {
			اگر (content_saw_insecure_objects(s->sheet)) {
				بازگشت true;
			}
		}
	}

	بازگشت false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned صحیح i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	برای (i = 0; i != html->stylesheet_شمار; i++) {
		اگر (html->stylesheets[i].sheet != پوچ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		اگر (html->stylesheets[i].گره != پوچ) {
			dom_گره_unref(html->stylesheets[i].گره);
		}
	}
	آزاد(html->stylesheets);

	بازگشت NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	أكد(c->stylesheets != پوچ);

	اگر (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), پوچ,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		اگر (ns_error != NSERROR_OK) {
			بازگشت ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	بازگشت ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	اگر (c->stylesheets != پوچ) {
		بازگشت NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			اندازه(ساختار html_stylesheet));
	اگر (c->stylesheets == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = پوچ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = پوچ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = پوچ;
	c->stylesheets[STYLESHEET_USER].sheet = پوچ;
	c->stylesheet_شمار = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), پوچ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	اگر (ns_error != NSERROR_OK) {
		بازگشت ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	اگر (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), پوچ,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		اگر (ns_error != NSERROR_OK) {
			بازگشت ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), پوچ,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	اگر (ns_error != NSERROR_OK) {
		بازگشت ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	بازگشت ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	اگر (c->stylesheets[STYLESHEET_BASE].sheet == پوچ) {
		بازگشت NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	اگر (css_ret != CSS_OK) {
		بازگشت css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	برای (i = STYLESHEET_BASE; i != c->stylesheet_شمار; i++) {
		ثابت ساختار html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = پوچ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		اگر (hsheet->unused) {
			ادامه;
		}

		اگر (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} وگرنه اگر (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		اگر (hsheet->sheet != پوچ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		اگر (sheet != پوچ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			اگر (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				بازگشت css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	بازگشت NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(تهی)
{
	nserror error;

	error = html_css_fetcher_register();
	اگر (error != NSERROR_OK)
		بازگشت error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	اگر (error != NSERROR_OK)
		بازگشت error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	اگر (error != NSERROR_OK)
		بازگشت error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	اگر (error != NSERROR_OK)
		بازگشت error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	بازگشت error;
}


/* exported function documented in html/css.h */
تهی html_css_fini(تهی)
{
	اگر (html_user_stylesheet_url != پوچ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = پوچ;
	}

	اگر (html_quirks_stylesheet_url != پوچ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = پوچ;
	}

	اگر (html_adblock_stylesheet_url != پوچ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = پوچ;
	}

	اگر (html_default_stylesheet_url != پوچ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = پوچ;
	}
}
