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

#அந்தர்பாவ "utils/config.h"

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "css/css.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/css.h"

நிலையான nsurl *html_default_stylesheet_url;
நிலையான nsurl *html_adblock_stylesheet_url;
நிலையான nsurl *html_quirks_stylesheet_url;
நிலையான nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
நிலையான nserror css_error_to_nserror(css_error error)
{
	தேர்வு (error) {
	நிலை CSS_OK:
		திரும்பு NSERROR_OK;

	நிலை CSS_NOMEM:
		திரும்பு NSERROR_NOMEM;

	நிலை CSS_BADPARM:
		திரும்பு NSERROR_BAD_PARAMETER;

	நிலை CSS_INVALID:
		திரும்பு NSERROR_INVALID;

	நிலை CSS_FILENOTFOUND:
		திரும்பு NSERROR_NOT_FOUND;

	நிலை CSS_NEEDDATA:
		திரும்பு NSERROR_NEED_DATA;

	நிலை CSS_BADCHARSET:
		திரும்பு NSERROR_BAD_ENCODING;

	நிலை CSS_EOF:
	நிலை CSS_IMPORTS_PENDING:
	நிலை CSS_PROPERTY_NOT_SET:
	இயல்பு:
		நிறுத்து;
	}
	திரும்பு NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
நிலையான nserror
html_convert_css_callback(hlcache_handle *css,
			  மாறிலி hlcache_event *event,
			  வெறுமை *pw)
{
	html_content *parent = pw;
	unsigned முழு i;
	கட்டமைப்பு html_stylesheet *s;

	/* Find sheet */
	க்கு (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_மொத்தம்;
	     i++, s++) {
		எனில் (s->sheet == css)
			நிறுத்து;
	}

	ப்ரதிஜ்ஞா(i != parent->stylesheet_மொத்தம்);

	தேர்வு (event->type) {

	நிலை CONTENT_செய்தி_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		நிறுத்து;

	நிலை CONTENT_செய்தி_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = வெற்று;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		நிறுத்து;

	நிலை CONTENT_செய்தி_POINTER:
		/* Really don't want this to continue after the switch */
		திரும்பு NSERROR_OK;

	இயல்பு:
		நிறுத்து;
	}

	எனில் (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	திரும்பு NSERROR_OK;
}


நிலையான nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_முனை *முனை,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t சாவி;
	எழுத்து urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_முனை_get_text_content(முனை, &style);
	எனில் ((exc != DOM_NO_ERR) || (style == வெற்று)) {
		NSLOG(netsurf, INFO, "No text content");
		திரும்பு NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &சாவி);
	எனில் (error != NSERROR_OK) {
		dom_string_unref(style);
		திரும்பு error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, அளவு(urlbuf), "x-ns-css:%"PRIu32"", சாவி);

	error = nsurl_create(urlbuf, &url);
	எனில் (error != NSERROR_OK) {
		திரும்பு error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), வெற்று,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	எனில் (error != NSERROR_OK) {
		nsurl_unref(url);
		திரும்பு error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	திரும்பு NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
நிலையான கட்டமைப்பு html_stylesheet *
html_create_style_element(html_content *c, dom_முனை *style)
{
	dom_string *val;
	dom_exception exc;
	கட்டமைப்பு html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	எனில் (exc == DOM_NO_ERR && val != வெற்று) {
		எனில் (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			திரும்பு வெற்று;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	எனில் (exc == DOM_NO_ERR && val != வெற்று) {
		எனில் (strcasestr(dom_string_data(val), "screen") == வெற்று &&
				strcasestr(dom_string_data(val),
						"all") == வெற்று) {
			dom_string_unref(val);
			திரும்பு வெற்று;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      அளவு(கட்டமைப்பு html_stylesheet) *
			      (c->stylesheet_மொத்தம் + 1));
	எனில் (stylesheets == வெற்று) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_மொத்தம்].முனை = dom_முனை_ref(style);
	c->stylesheets[c->stylesheet_மொத்தம்].sheet = வெற்று;
	c->stylesheets[c->stylesheet_மொத்தம்].modified = false;
	c->stylesheets[c->stylesheet_மொத்தம்].unused = false;
	c->stylesheet_மொத்தம்++;

	திரும்பு c->stylesheets + (c->stylesheet_மொத்தம் - 1);
}


நிலையான bool
html_css_process_modified_style(html_content *c, கட்டமைப்பு html_stylesheet *s)
{
	hlcache_handle *sheet = வெற்று;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->முனை, &sheet);
	எனில் (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, வெற்று);
		திரும்பு false;
	}

	எனில் (sheet != வெற்று) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		எனில் (s->sheet != வெற்று) {
			தேர்வு (content_get_status(s->sheet)) {
			நிலை CONTENT_STATUS_DONE:
				நிறுத்து;
			இயல்பு:
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

	திரும்பு true;
}


/**
 * process a stylesheet that has been modified.
 */
நிலையான வெறுமை html_css_process_modified_styles(வெறுமை *pw)
{
	html_content *c = pw;
	கட்டமைப்பு html_stylesheet *s;
	unsigned முழு i;
	bool all_done = true;

	க்கு (i = 0, s = c->stylesheets; i != c->stylesheet_மொத்தம்; i++, s++) {
		எனில் (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	எனில் (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_முனை *style)
{
	unsigned முழு i;
	கட்டமைப்பு html_stylesheet *s;

	/* Find sheet */
	க்கு (i = 0, s = c->stylesheets;	i != c->stylesheet_மொத்தம்; i++, s++) {
		எனில் (s->முனை == style)
			நிறுத்து;
	}
	எனில் (i == c->stylesheet_மொத்தம்) {
		s = html_create_style_element(c, style);
	}
	எனில் (s == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		திரும்பு false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	திரும்பு true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_முனை *முனை)
{
	unsigned முழு i;
	dom_string *val;
	dom_exception exc;
	கட்டமைப்பு html_stylesheet *s;

	/* Find sheet */
	க்கு (i = 0, s = c->stylesheets;	i != c->stylesheet_மொத்தம்; i++, s++) {
		எனில் (s->முனை == முனை)
			நிறுத்து;
	}

		/* Should already exist */
	எனில் (i == c->stylesheet_மொத்தம்) {
		திரும்பு false;
	}

	exc = dom_element_get_attribute(முனை, corestring_dom_media, &val);
	எனில் (exc == DOM_NO_ERR && val != வெற்று) {
		எனில் (strcasestr(dom_string_data(val), "screen") == வெற்று &&
				strcasestr(dom_string_data(val),
						"all") == வெற்று) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	திரும்பு true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_முனை *முனை)
{
	dom_string *rel, *type_attr, *media, *href;
	கட்டமைப்பு html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(முனை, corestring_dom_rel, &rel);
	எனில் (exc != DOM_NO_ERR || rel == வெற்று)
		திரும்பு true;

	எனில் (strcasestr(dom_string_data(rel), "stylesheet") == வெற்று) {
		dom_string_unref(rel);
		திரும்பு true;
	} இல்லையெனில் எனில் (strcasestr(dom_string_data(rel), "alternate") != வெற்று) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		திரும்பு true;
	}
	dom_string_unref(rel);

	எனில் (nsoption_bool(author_level_css) == false) {
		திரும்பு true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(முனை, corestring_dom_type, &type_attr);
	எனில் (exc == DOM_NO_ERR && type_attr != வெற்று) {
		எனில் (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			திரும்பு true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(முனை, corestring_dom_media, &media);
	எனில் (exc == DOM_NO_ERR && media != வெற்று) {
		எனில் (strcasestr(dom_string_data(media), "screen") == வெற்று &&
		    strcasestr(dom_string_data(media), "all") == வெற்று) {
			dom_string_unref(media);
			திரும்பு true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(முனை, corestring_dom_href, &href);
	எனில் (exc != DOM_NO_ERR || href == வெற்று)
		திரும்பு true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	எனில் (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		செல் no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_மொத்தம், nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      அளவு(கட்டமைப்பு html_stylesheet) *
			      (htmlc->stylesheet_மொத்தம் + 1));
	எனில் (stylesheets == வெற்று) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		செல் no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_மொத்தம்].முனை = வெற்று;
	htmlc->stylesheets[htmlc->stylesheet_மொத்தம்].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_மொத்தம்].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			வெற்று, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_மொத்தம்].sheet);

	nsurl_unref(joined);

	எனில் (ns_error != NSERROR_OK)
		செல் no_memory;

	htmlc->stylesheet_மொத்தம்++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	திரும்பு true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, வெற்று);
	திரும்பு false;
}


/* exported interface documented in html/html.h */
கட்டமைப்பு html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned முழு *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);
	ப்ரதிஜ்ஞா(n != வெற்று);

	*n = c->stylesheet_மொத்தம்;

	திரும்பு c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	கட்டமைப்பு html_stylesheet *s;
	unsigned முழு i;

	க்கு (i = 0, s = html->stylesheets; i < html->stylesheet_மொத்தம்;
	     i++, s++) {
		எனில் (s->sheet != வெற்று) {
			எனில் (content_saw_insecure_objects(s->sheet)) {
				திரும்பு true;
			}
		}
	}

	திரும்பு false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned முழு i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	க்கு (i = 0; i != html->stylesheet_மொத்தம்; i++) {
		எனில் (html->stylesheets[i].sheet != வெற்று) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		எனில் (html->stylesheets[i].முனை != வெற்று) {
			dom_முனை_unref(html->stylesheets[i].முனை);
		}
	}
	விடுவி(html->stylesheets);

	திரும்பு NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	ப்ரதிஜ்ஞா(c->stylesheets != வெற்று);

	எனில் (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), வெற்று,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		எனில் (ns_error != NSERROR_OK) {
			திரும்பு ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	திரும்பு ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	எனில் (c->stylesheets != வெற்று) {
		திரும்பு NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			அளவு(கட்டமைப்பு html_stylesheet));
	எனில் (c->stylesheets == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = வெற்று;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = வெற்று;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = வெற்று;
	c->stylesheets[STYLESHEET_USER].sheet = வெற்று;
	c->stylesheet_மொத்தம் = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), வெற்று,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	எனில் (ns_error != NSERROR_OK) {
		திரும்பு ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	எனில் (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), வெற்று,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		எனில் (ns_error != NSERROR_OK) {
			திரும்பு ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), வெற்று,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	எனில் (ns_error != NSERROR_OK) {
		திரும்பு ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	திரும்பு ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	எனில் (c->stylesheets[STYLESHEET_BASE].sheet == வெற்று) {
		திரும்பு NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	எனில் (css_ret != CSS_OK) {
		திரும்பு css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	க்கு (i = STYLESHEET_BASE; i != c->stylesheet_மொத்தம்; i++) {
		மாறிலி கட்டமைப்பு html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = வெற்று;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		எனில் (hsheet->unused) {
			தொடர்;
		}

		எனில் (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} இல்லையெனில் எனில் (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		எனில் (hsheet->sheet != வெற்று) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		எனில் (sheet != வெற்று) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			எனில் (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				திரும்பு css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	திரும்பு NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(வெறுமை)
{
	nserror error;

	error = html_css_fetcher_register();
	எனில் (error != NSERROR_OK)
		திரும்பு error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	எனில் (error != NSERROR_OK)
		திரும்பு error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	எனில் (error != NSERROR_OK)
		திரும்பு error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	எனில் (error != NSERROR_OK)
		திரும்பு error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	திரும்பு error;
}


/* exported function documented in html/css.h */
வெறுமை html_css_fini(வெறுமை)
{
	எனில் (html_user_stylesheet_url != வெற்று) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = வெற்று;
	}

	எனில் (html_quirks_stylesheet_url != வெற்று) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = வெற்று;
	}

	எனில் (html_adblock_stylesheet_url != வெற்று) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = வெற்று;
	}

	எனில் (html_default_stylesheet_url != வெற்று) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = வெற்று;
	}
}
