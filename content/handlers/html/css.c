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
ساكن nserror css_error_منزل_nserror(css_error error)
{
	بدل (error) {
	حالة CSS_OK:
		أرجع NSERROR_OK;

	حالة CSS_NOMEM:
		أرجع NSERROR_NOMEM;

	حالة CSS_BADPARM:
		أرجع NSERROR_BAD_PARAMETER;

	حالة CSS_INVALID:
		أرجع NSERROR_INVALID;

	حالة CSS_FILENOTFOUND:
		أرجع NSERROR_NOT_وموندل;

	حالة CSS_NEEDDATA:
		أرجع NSERROR_NEED_معلومات;

	حالة CSS_BADCHARSET:
		أرجع NSERROR_BAD_ENCODING;

	حالة CSS_EOF:
	حالة CSS_IMPORTS_PENDING:
	حالة CSS_PROPERTY_NOT_سېټ:
	افتراضي:
		اكسر;
	}
	أرجع NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ساكن nserror
html_convert_css_callback(hlcache_handle *css,
			  ثابت hlcache_event *event,
			  فراغ *pw)
{
	html_content *پلار = pw;
	بینښه صحيح i;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = پلار->stylesheets;
	     i != پلار->stylesheet_شمېر;
	     i++, s++) {
		إذا (s->sheet == css)
			اكسر;
	}

	أكد(i != پلار->stylesheet_شمېر);

	بدل (event->type) {

	حالة CONTENT_پیغام_بشپړ:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		پلار->بنسټ.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", پلار->بنسټ.فعال);
		اكسر;

	حالة CONTENT_پیغام_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->معلومات.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = عدم;
		پلار->بنسټ.فعال--;
		NSLOG(netsurf, INFO, "%d fetches active", پلار->بنسټ.فعال);
		اكسر;

	حالة CONTENT_پیغام_اشاره_ګر:
		/* Really don't want this to continue after the switch */
		أرجع NSERROR_OK;

	افتراضي:
		اكسر;
	}

	إذا (html_can_begin_conversion(پلار)) {
		html_begin_conversion(پلار);
	}

	أرجع NSERROR_OK;
}


ساكن nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_غوټه *غوټه,
			     hlcache_handle **sheet)
{
	hlcache_ماشوم_context ماشوم;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t کلی;
	حرف urlbuf[64];

	ماشوم.charset = c->encoding;
	ماشوم.quirks = c->بنسټ.quirks;

	exc = dom_غوټه_get_متن_content(غوټه, &style);
	إذا ((exc != DOM_NO_ERR) || (style == عدم)) {
		NSLOG(netsurf, INFO, "No text content");
		أرجع NSERROR_OK;
	}

	error = html_css_fetcher_جمع_کړه_توکی(style, c->بنسټ_url, &کلی);
	إذا (error != NSERROR_OK) {
		dom_string_unref(style);
		أرجع error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, حجم(urlbuf), "x-ns-css:%"PRIu32"", کلی);

	error = nsurl_create(urlbuf, &url);
	إذا (error != NSERROR_OK) {
		أرجع error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->بنسټ), عدم,
			html_convert_css_callback, c, &ماشوم, CONTENT_CSS,
			sheet);
	إذا (error != NSERROR_OK) {
		nsurl_unref(url);
		أرجع error;
	}

	nsurl_unref(url);

	c->بنسټ.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

	أرجع NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ساكن بنية html_stylesheet *
html_create_style_element(html_content *c, dom_غوټه *style)
{
	dom_string *val;
	dom_exception exc;
	بنية html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	إذا (exc == DOM_NO_ERR && val != عدم) {
		إذا (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_متن_css)) {
			dom_string_unref(val);
			أرجع عدم;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	إذا (exc == DOM_NO_ERR && val != عدم) {
		إذا (strcasestr(dom_string_معلومات(val), "screen") == عدم &&
				strcasestr(dom_string_معلومات(val),
						"all") == عدم) {
			dom_string_unref(val);
			أرجع عدم;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      حجم(بنية html_stylesheet) *
			      (c->stylesheet_شمېر + 1));
	إذا (stylesheets == عدم) {

		content_خپرونه_error(&c->بنسټ, NSERROR_NOMEM, عدم);
		أرجع false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_شمېر].غوټه = dom_غوټه_ref(style);
	c->stylesheets[c->stylesheet_شمېر].sheet = عدم;
	c->stylesheets[c->stylesheet_شمېر].modified = false;
	c->stylesheets[c->stylesheet_شمېر].ناکارول_شوی = false;
	c->stylesheet_شمېر++;

	أرجع c->stylesheets + (c->stylesheet_شمېر - 1);
}


ساكن bool
html_css_process_modified_style(html_content *c, بنية html_stylesheet *s)
{
	hlcache_handle *sheet = عدم;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->غوټه, &sheet);
	إذا (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_خپرونه_error(&c->بنسټ, error, عدم);
		أرجع false;
	}

	إذا (sheet != عدم) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		إذا (s->sheet != عدم) {
			بدل (content_get_حالت(s->sheet)) {
			حالة CONTENT_حالت_بشپړ:
				اكسر;
			افتراضي:
				hlcache_handle_abort(s->sheet);
				c->بنسټ.فعال--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->بنسټ.فعال);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	أرجع true;
}


/**
 * process a stylesheet that has been modified.
 */
ساكن فراغ html_css_process_modified_styles(فراغ *pw)
{
	html_content *c = pw;
	بنية html_stylesheet *s;
	بینښه صحيح i;
	bool all_بشپړ = true;

	لكل (i = 0, s = c->stylesheets; i != c->stylesheet_شمېر; i++, s++) {
		إذا (c->stylesheets[i].modified) {
			all_بشپړ &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	إذا (all_بشپړ == false) {
		guit->misc->مهالوېش(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_غوټه *style)
{
	بینښه صحيح i;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = c->stylesheets;	i != c->stylesheet_شمېر; i++, s++) {
		إذا (s->غوټه == style)
			اكسر;
	}
	إذا (i == c->stylesheet_شمېر) {
		s = html_create_style_element(c, style);
	}
	إذا (s == عدم) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		أرجع false;
	}

	s->modified = true;

	guit->misc->مهالوېش(0, html_css_process_modified_styles, c);

	أرجع true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_غوټه *غوټه)
{
	بینښه صحيح i;
	dom_string *val;
	dom_exception exc;
	بنية html_stylesheet *s;

	/* Find sheet */
	لكل (i = 0, s = c->stylesheets;	i != c->stylesheet_شمېر; i++, s++) {
		إذا (s->غوټه == غوټه)
			اكسر;
	}

		/* Should already exist */
	إذا (i == c->stylesheet_شمېر) {
		أرجع false;
	}

	exc = dom_element_get_attribute(غوټه, corestring_dom_media, &val);
	إذا (exc == DOM_NO_ERR && val != عدم) {
		إذا (strcasestr(dom_string_معلومات(val), "screen") == عدم &&
				strcasestr(dom_string_معلومات(val),
						"all") == عدم) {
			s->ناکارول_شوی = true;
		}
		dom_string_unref(val);
	}

	أرجع true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_غوټه *غوټه)
{
	dom_string *rel, *type_attr, *media, *href;
	بنية html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_ماشوم_context ماشوم;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(غوټه, corestring_dom_rel, &rel);
	إذا (exc != DOM_NO_ERR || rel == عدم)
		أرجع true;

	إذا (strcasestr(dom_string_معلومات(rel), "stylesheet") == عدم) {
		dom_string_unref(rel);
		أرجع true;
	} وإلا إذا (strcasestr(dom_string_معلومات(rel), "alternate") != عدم) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		أرجع true;
	}
	dom_string_unref(rel);

	إذا (nsoption_bool(author_level_css) == false) {
		أرجع true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(غوټه, corestring_dom_type, &type_attr);
	إذا (exc == DOM_NO_ERR && type_attr != عدم) {
		إذا (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_متن_css)) {
			dom_string_unref(type_attr);
			أرجع true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(غوټه, corestring_dom_media, &media);
	إذا (exc == DOM_NO_ERR && media != عدم) {
		إذا (strcasestr(dom_string_معلومات(media), "screen") == عدم &&
		    strcasestr(dom_string_معلومات(media), "all") == عدم) {
			dom_string_unref(media);
			أرجع true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(غوټه, corestring_dom_href, &href);
	إذا (exc != DOM_NO_ERR || href == عدم)
		أرجع true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->بنسټ_url, dom_string_معلومات(href), &joined);
	إذا (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		اذهب no_حافظه;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_شمېر, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      حجم(بنية html_stylesheet) *
			      (htmlc->stylesheet_شمېر + 1));
	إذا (stylesheets == عدم) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		اذهب no_حافظه;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_شمېر].غوټه = عدم;
	htmlc->stylesheets[htmlc->stylesheet_شمېر].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_شمېر].ناکارول_شوی = false;

	/* start fetch */
	ماشوم.charset = htmlc->encoding;
	ماشوم.quirks = htmlc->بنسټ.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->بنسټ),
			عدم, html_convert_css_callback,
			htmlc, &ماشوم, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_شمېر].sheet);

	nsurl_unref(joined);

	إذا (ns_error != NSERROR_OK)
		اذهب no_حافظه;

	htmlc->stylesheet_شمېر++;

	htmlc->بنسټ.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->بنسټ.فعال);

	أرجع true;

no_حافظه:
	content_خپرونه_error(&htmlc->بنسټ, ns_error, عدم);
	أرجع false;
}


/* exported interface documented in html/html.h */
بنية html_stylesheet *html_get_stylesheets(hlcache_handle *h, بینښه صحيح *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);
	أكد(n != عدم);

	*n = c->stylesheet_شمېر;

	أرجع c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	بنية html_stylesheet *s;
	بینښه صحيح i;

	لكل (i = 0, s = html->stylesheets; i < html->stylesheet_شمېر;
	     i++, s++) {
		إذا (s->sheet != عدم) {
			إذا (content_saw_insecure_objects(s->sheet)) {
				أرجع true;
			}
		}
	}

	أرجع false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	بینښه صحيح i;

	guit->misc->مهالوېش(-1, html_css_process_modified_styles, html);

	لكل (i = 0; i != html->stylesheet_شمېر; i++) {
		إذا (html->stylesheets[i].sheet != عدم) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		إذا (html->stylesheets[i].غوټه != عدم) {
			dom_غوټه_unref(html->stylesheets[i].غوټه);
		}
	}
	حرر(html->stylesheets);

	أرجع NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_ماشوم_context ماشوم;

	أكد(c->stylesheets != عدم);

	إذا (c->quirks == DOM_DOCUMENT_QUIRKS_ډېرپېښ_ډک) {
		ماشوم.charset = c->encoding;
		ماشوم.quirks = c->بنسټ.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->بنسټ), عدم,
				html_convert_css_callback, c, &ماشوم,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		إذا (ns_error != NSERROR_OK) {
			أرجع ns_error;
		}

		c->بنسټ.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);
	}

	أرجع ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_ماشوم_context ماشوم;

	إذا (c->stylesheets != عدم) {
		أرجع NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = حافظه_صفر_ورکړه(STYLESHEET_پیل,
			حجم(بنية html_stylesheet));
	إذا (c->stylesheets == عدم) {
		أرجع NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_بنسټ].sheet = عدم;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = عدم;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = عدم;
	c->stylesheets[STYLESHEET_USER].sheet = عدم;
	c->stylesheet_شمېر = STYLESHEET_پیل;

	ماشوم.charset = c->encoding;
	ماشوم.quirks = c->بنسټ.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->بنسټ), عدم,
			html_convert_css_callback, c, &ماشوم, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_بنسټ].sheet);
	إذا (ns_error != NSERROR_OK) {
		أرجع ns_error;
	}

	c->بنسټ.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);


	إذا (nsoption_bool(بلاک_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->بنسټ), عدم,
				html_convert_css_callback,
				c, &ماشوم, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		إذا (ns_error != NSERROR_OK) {
			أرجع ns_error;
		}

		c->بنسټ.فعال++;
		NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->بنسټ), عدم,
			html_convert_css_callback, c, &ماشوم, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	إذا (ns_error != NSERROR_OK) {
		أرجع ns_error;
	}

	c->بنسټ.فعال++;
	NSLOG(netsurf, INFO, "%d fetches active", c->بنسټ.فعال);

	أرجع ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	إذا (c->stylesheets[STYLESHEET_بنسټ].sheet == عدم) {
		أرجع NSERROR_CSS_بنسټ;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	إذا (css_ret != CSS_OK) {
		أرجع css_error_منزل_nserror(css_ret);
	}

	/* Add sheets to it */
	لكل (i = STYLESHEET_بنسټ; i != c->stylesheet_شمېر; i++) {
		ثابت بنية html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = عدم;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		إذا (hsheet->ناکارول_شوی) {
			استمر;
		}

		إذا (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} وإلا إذا (i < STYLESHEET_پیل) {
			origin = CSS_ORIGIN_USER;
		}

		إذا (hsheet->sheet != عدم) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		إذا (sheet != عدم) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			إذا (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				أرجع css_error_منزل_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	أرجع NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(فراغ)
{
	nserror error;

	error = html_css_fetcher_ثبتوونکی();
	إذا (error != NSERROR_OK)
		أرجع error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	إذا (error != NSERROR_OK)
		أرجع error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	إذا (error != NSERROR_OK)
		أرجع error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	إذا (error != NSERROR_OK)
		أرجع error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	أرجع error;
}


/* exported function documented in html/css.h */
فراغ html_css_fini(فراغ)
{
	إذا (html_user_stylesheet_url != عدم) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = عدم;
	}

	إذا (html_quirks_stylesheet_url != عدم) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = عدم;
	}

	إذا (html_adblock_stylesheet_url != عدم) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = عدم;
	}

	إذا (html_default_stylesheet_url != عدم) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = عدم;
	}
}
