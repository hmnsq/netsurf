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

ساکن nsurl *html_default_stylesheet_url;
ساکن nsurl *html_adblock_stylesheet_url;
ساکن nsurl *html_quirks_stylesheet_url;
ساکن nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ساکن nserror css_error_مقصد_nserror(css_error error)
{
	انتخاب (error) {
	حالت CSS_OK:
		واپس NSERROR_OK;

	حالت CSS_NOMEM:
		واپس NSERROR_NOMEM;

	حالت CSS_BADPARM:
		واپس NSERROR_BAD_PARAMETER;

	حالت CSS_INVALID:
		واپس NSERROR_INVALID;

	حالت CSS_FILENOTFOUND:
		واپس NSERROR_NOT_ملا;

	حالت CSS_NEEDDATA:
		واپس NSERROR_NEED_ڈیٹا;

	حالت CSS_BADCHARSET:
		واپس NSERROR_BAD_ENCODING;

	حالت CSS_EOF:
	حالت CSS_IMPORTS_PENDING:
	حالت CSS_PROPERTY_NOT_سیٹ:
	طےشدہ:
		توڑو;
	}
	واپس NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ساکن nserror
html_convert_css_callback(hlcache_handle *css,
			  ثابت hlcache_event *event,
			  خالی *pw)
{
	html_content *والد = pw;
	غیرعلامتی صحیح i;
	ساخت html_stylesheet *s;

	/* Find sheet */
	برائے (i = 0, s = والد->stylesheets;
	     i != والد->stylesheet_گنتی;
	     i++, s++) {
		اگر (s->sheet == css)
			توڑو;
	}

	أكد(i != والد->stylesheet_گنتی);

	انتخاب (event->type) {

	حالت CONTENT_پیغام_مکمل:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);
		توڑو;

	حالت CONTENT_پیغام_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->ڈیٹا.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = تہی;
		والد->بنیاد.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", والد->بنیاد.فعال);
		توڑو;

	حالت CONTENT_پیغام_اشارہ_گر:
		/* Really don't want this to continue after the switch */
		واپس NSERROR_OK;

	طےشدہ:
		توڑو;
	}

	اگر (html_can_begin_conversion(والد)) {
		html_begin_conversion(والد);
	}

	واپس NSERROR_OK;
}


ساکن nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_گرہ *گرہ,
			     hlcache_handle **sheet)
{
	hlcache_بچہ_context بچہ;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t کلید;
	حرف urlbuf[64];

	بچہ.charset = c->encoding;
	بچہ.quirks = c->بنیاد.quirks;

	exc = dom_گرہ_get_متن_content(گرہ, &style);
	اگر ((exc != DOM_NO_ERR) || (style == تہی)) {
		NSLOG(netsurf, INFO, "No text content");
		واپس NSERROR_OK;
	}

	error = html_css_fetcher_جوڑو_شے(style, c->بنیاد_url, &کلید);
	اگر (error != NSERROR_OK) {
		dom_string_unref(style);
		واپس error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, حجم(urlbuf), "x-ns-css:%"PRIu32"", کلید);

	error = nsurl_create(urlbuf, &url);
	اگر (error != NSERROR_OK) {
		واپس error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->بنیاد), تہی,
			html_convert_css_callback, c, &بچہ, CONTENT_CSS,
			sheet);
	اگر (error != NSERROR_OK) {
		nsurl_unref(url);
		واپس error;
	}

	nsurl_unref(url);

	c->بنیاد.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

	واپس NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ساکن ساخت html_stylesheet *
html_create_style_element(html_content *c, dom_گرہ *style)
{
	dom_string *val;
	dom_exception exc;
	ساخت html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	اگر (exc == DOM_NO_ERR && val != تہی) {
		اگر (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_متن_css)) {
			dom_string_unref(val);
			واپس تہی;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	اگر (exc == DOM_NO_ERR && val != تہی) {
		اگر (strcasestr(dom_string_ڈیٹا(val), "screen") == تہی &&
				strcasestr(dom_string_ڈیٹا(val),
						"all") == تہی) {
			dom_string_unref(val);
			واپس تہی;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      حجم(ساخت html_stylesheet) *
			      (c->stylesheet_گنتی + 1));
	اگر (stylesheets == تہی) {

		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_گنتی].گرہ = dom_گرہ_ref(style);
	c->stylesheets[c->stylesheet_گنتی].sheet = تہی;
	c->stylesheets[c->stylesheet_گنتی].modified = false;
	c->stylesheets[c->stylesheet_گنتی].غیر_مستعمل = false;
	c->stylesheet_گنتی++;

	واپس c->stylesheets + (c->stylesheet_گنتی - 1);
}


ساکن bool
html_css_process_modified_style(html_content *c, ساخت html_stylesheet *s)
{
	hlcache_handle *sheet = تہی;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->گرہ, &sheet);
	اگر (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_نشر_error(&c->بنیاد, error, تہی);
		واپس false;
	}

	اگر (sheet != تہی) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		اگر (s->sheet != تہی) {
			انتخاب (content_get_حالت(s->sheet)) {
			حالت CONTENT_حالت_مکمل:
				توڑو;
			طےشدہ:
				hlcache_handle_abort(s->sheet);
				c->بنیاد.فعال--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->بنیاد.فعال);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	واپس true;
}


/**
 * process a stylesheet that has been modified.
 */
ساکن خالی html_css_process_modified_styles(خالی *pw)
{
	html_content *c = pw;
	ساخت html_stylesheet *s;
	غیرعلامتی صحیح i;
	bool all_مکمل = true;

	برائے (i = 0, s = c->stylesheets; i != c->stylesheet_گنتی; i++, s++) {
		اگر (c->stylesheets[i].modified) {
			all_مکمل &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	اگر (all_مکمل == false) {
		guit->misc->جدول_وقت(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_گرہ *style)
{
	غیرعلامتی صحیح i;
	ساخت html_stylesheet *s;

	/* Find sheet */
	برائے (i = 0, s = c->stylesheets;	i != c->stylesheet_گنتی; i++, s++) {
		اگر (s->گرہ == style)
			توڑو;
	}
	اگر (i == c->stylesheet_گنتی) {
		s = html_create_style_element(c, style);
	}
	اگر (s == تہی) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		واپس false;
	}

	s->modified = true;

	guit->misc->جدول_وقت(0, html_css_process_modified_styles, c);

	واپس true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_گرہ *گرہ)
{
	غیرعلامتی صحیح i;
	dom_string *val;
	dom_exception exc;
	ساخت html_stylesheet *s;

	/* Find sheet */
	برائے (i = 0, s = c->stylesheets;	i != c->stylesheet_گنتی; i++, s++) {
		اگر (s->گرہ == گرہ)
			توڑو;
	}

		/* Should already exist */
	اگر (i == c->stylesheet_گنتی) {
		واپس false;
	}

	exc = dom_element_get_attribute(گرہ, corestring_dom_media, &val);
	اگر (exc == DOM_NO_ERR && val != تہی) {
		اگر (strcasestr(dom_string_ڈیٹا(val), "screen") == تہی &&
				strcasestr(dom_string_ڈیٹا(val),
						"all") == تہی) {
			s->غیر_مستعمل = true;
		}
		dom_string_unref(val);
	}

	واپس true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_گرہ *گرہ)
{
	dom_string *rel, *type_attr, *media, *href;
	ساخت html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_بچہ_context بچہ;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(گرہ, corestring_dom_rel, &rel);
	اگر (exc != DOM_NO_ERR || rel == تہی)
		واپس true;

	اگر (strcasestr(dom_string_ڈیٹا(rel), "stylesheet") == تہی) {
		dom_string_unref(rel);
		واپس true;
	} ورنہ اگر (strcasestr(dom_string_ڈیٹا(rel), "alternate") != تہی) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		واپس true;
	}
	dom_string_unref(rel);

	اگر (nsoption_bool(author_level_css) == false) {
		واپس true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(گرہ, corestring_dom_type, &type_attr);
	اگر (exc == DOM_NO_ERR && type_attr != تہی) {
		اگر (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_متن_css)) {
			dom_string_unref(type_attr);
			واپس true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(گرہ, corestring_dom_media, &media);
	اگر (exc == DOM_NO_ERR && media != تہی) {
		اگر (strcasestr(dom_string_ڈیٹا(media), "screen") == تہی &&
		    strcasestr(dom_string_ڈیٹا(media), "all") == تہی) {
			dom_string_unref(media);
			واپس true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(گرہ, corestring_dom_href, &href);
	اگر (exc != DOM_NO_ERR || href == تہی)
		واپس true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->بنیاد_url, dom_string_ڈیٹا(href), &joined);
	اگر (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		جاؤ no_حافظہ;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_گنتی, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      حجم(ساخت html_stylesheet) *
			      (htmlc->stylesheet_گنتی + 1));
	اگر (stylesheets == تہی) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		جاؤ no_حافظہ;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_گنتی].گرہ = تہی;
	htmlc->stylesheets[htmlc->stylesheet_گنتی].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_گنتی].غیر_مستعمل = false;

	/* start fetch */
	بچہ.charset = htmlc->encoding;
	بچہ.quirks = htmlc->بنیاد.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->بنیاد),
			تہی, html_convert_css_callback,
			htmlc, &بچہ, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_گنتی].sheet);

	nsurl_unref(joined);

	اگر (ns_error != NSERROR_OK)
		جاؤ no_حافظہ;

	htmlc->stylesheet_گنتی++;

	htmlc->بنیاد.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->بنیاد.فعال);

	واپس true;

no_حافظہ:
	content_نشر_error(&htmlc->بنیاد, ns_error, تہی);
	واپس false;
}


/* exported interface documented in html/html.h */
ساخت html_stylesheet *html_get_stylesheets(hlcache_handle *h, غیرعلامتی صحیح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);
	أكد(n != تہی);

	*n = c->stylesheet_گنتی;

	واپس c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ساخت html_stylesheet *s;
	غیرعلامتی صحیح i;

	برائے (i = 0, s = html->stylesheets; i < html->stylesheet_گنتی;
	     i++, s++) {
		اگر (s->sheet != تہی) {
			اگر (content_saw_insecure_objects(s->sheet)) {
				واپس true;
			}
		}
	}

	واپس false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	غیرعلامتی صحیح i;

	guit->misc->جدول_وقت(-1, html_css_process_modified_styles, html);

	برائے (i = 0; i != html->stylesheet_گنتی; i++) {
		اگر (html->stylesheets[i].sheet != تہی) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		اگر (html->stylesheets[i].گرہ != تہی) {
			dom_گرہ_unref(html->stylesheets[i].گرہ);
		}
	}
	آزاد(html->stylesheets);

	واپس NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_بچہ_context بچہ;

	أكد(c->stylesheets != تہی);

	اگر (c->quirks == DOM_DOCUMENT_QUIRKS_منوال_بھرا) {
		بچہ.charset = c->encoding;
		بچہ.quirks = c->بنیاد.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->بنیاد), تہی,
				html_convert_css_callback, c, &بچہ,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		اگر (ns_error != NSERROR_OK) {
			واپس ns_error;
		}

		c->بنیاد.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);
	}

	واپس ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_بچہ_context بچہ;

	اگر (c->stylesheets != تہی) {
		واپس NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = صفر_حافظہ_دو(STYLESHEET_آغاز,
			حجم(ساخت html_stylesheet));
	اگر (c->stylesheets == تہی) {
		واپس NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_بنیاد].sheet = تہی;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = تہی;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = تہی;
	c->stylesheets[STYLESHEET_USER].sheet = تہی;
	c->stylesheet_گنتی = STYLESHEET_آغاز;

	بچہ.charset = c->encoding;
	بچہ.quirks = c->بنیاد.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->بنیاد), تہی,
			html_convert_css_callback, c, &بچہ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_بنیاد].sheet);
	اگر (ns_error != NSERROR_OK) {
		واپس ns_error;
	}

	c->بنیاد.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);


	اگر (nsoption_bool(بلاک_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->بنیاد), تہی,
				html_convert_css_callback,
				c, &بچہ, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		اگر (ns_error != NSERROR_OK) {
			واپس ns_error;
		}

		c->بنیاد.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->بنیاد), تہی,
			html_convert_css_callback, c, &بچہ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	اگر (ns_error != NSERROR_OK) {
		واپس ns_error;
	}

	c->بنیاد.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنیاد.فعال);

	واپس ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	اگر (c->stylesheets[STYLESHEET_بنیاد].sheet == تہی) {
		واپس NSERROR_CSS_بنیاد;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	اگر (css_ret != CSS_OK) {
		واپس css_error_مقصد_nserror(css_ret);
	}

	/* Add sheets to it */
	برائے (i = STYLESHEET_بنیاد; i != c->stylesheet_گنتی; i++) {
		ثابت ساخت html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = تہی;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		اگر (hsheet->غیر_مستعمل) {
			جاری;
		}

		اگر (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ورنہ اگر (i < STYLESHEET_آغاز) {
			origin = CSS_ORIGIN_USER;
		}

		اگر (hsheet->sheet != تہی) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		اگر (sheet != تہی) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			اگر (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				واپس css_error_مقصد_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	واپس NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(خالی)
{
	nserror error;

	error = html_css_fetcher_رجسٹر();
	اگر (error != NSERROR_OK)
		واپس error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	اگر (error != NSERROR_OK)
		واپس error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	اگر (error != NSERROR_OK)
		واپس error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	اگر (error != NSERROR_OK)
		واپس error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	واپس error;
}


/* exported function documented in html/css.h */
خالی html_css_fini(خالی)
{
	اگر (html_user_stylesheet_url != تہی) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = تہی;
	}

	اگر (html_quirks_stylesheet_url != تہی) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = تہی;
	}

	اگر (html_adblock_stylesheet_url != تہی) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = تہی;
	}

	اگر (html_default_stylesheet_url != تہی) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = تہی;
	}
}
