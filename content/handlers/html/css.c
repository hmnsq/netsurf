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

#અન્તર્ભાવ "utils/config.h"

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "css/css.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/css.h"

સ્થિર nsurl *html_default_stylesheet_url;
સ્થિર nsurl *html_adblock_stylesheet_url;
સ્થિર nsurl *html_quirks_stylesheet_url;
સ્થિર nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
સ્થિર nserror css_error_to_nserror(css_error error)
{
	પસંદગી (error) {
	સ્થિતિ CSS_OK:
		પાછા NSERROR_OK;

	સ્થિતિ CSS_NOMEM:
		પાછા NSERROR_NOMEM;

	સ્થિતિ CSS_BADPARM:
		પાછા NSERROR_BAD_PARAMETER;

	સ્થિતિ CSS_INVALID:
		પાછા NSERROR_INVALID;

	સ્થિતિ CSS_FILENOTFOUND:
		પાછા NSERROR_NOT_FOUND;

	સ્થિતિ CSS_NEEDDATA:
		પાછા NSERROR_NEED_DATA;

	સ્થિતિ CSS_BADCHARSET:
		પાછા NSERROR_BAD_ENCODING;

	સ્થિતિ CSS_EOF:
	સ્થિતિ CSS_IMPORTS_PENDING:
	સ્થિતિ CSS_PROPERTY_NOT_SET:
	મૂળભૂત:
		તોડો;
	}
	પાછા NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
સ્થિર nserror
html_convert_css_callback(hlcache_handle *css,
			  અચલ hlcache_event *event,
			  શૂન્ય *pw)
{
	html_content *parent = pw;
	unsigned પૂર્ણાંક i;
	સંરચના html_stylesheet *s;

	/* Find sheet */
	માટે (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_ગણતરી;
	     i++, s++) {
		જો (s->sheet == css)
			તોડો;
	}

	પ્રતિજ્ઞા(i != parent->stylesheet_ગણતરી);

	પસંદગી (event->type) {

	સ્થિતિ CONTENT_સંદેશ_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ખાલી;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_POINTER:
		/* Really don't want this to continue after the switch */
		પાછા NSERROR_OK;

	મૂળભૂત:
		તોડો;
	}

	જો (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	પાછા NSERROR_OK;
}


સ્થિર nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ગાંઠ *ગાંઠ,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t ચાવી;
	અક્ષર urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_ગાંઠ_get_text_content(ગાંઠ, &style);
	જો ((exc != DOM_NO_ERR) || (style == ખાલી)) {
		NSLOG(netsurf, INFO, "No text content");
		પાછા NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &ચાવી);
	જો (error != NSERROR_OK) {
		dom_string_unref(style);
		પાછા error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, કદ(urlbuf), "x-ns-css:%"PRIu32"", ચાવી);

	error = nsurl_create(urlbuf, &url);
	જો (error != NSERROR_OK) {
		પાછા error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), ખાલી,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	જો (error != NSERROR_OK) {
		nsurl_unref(url);
		પાછા error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	પાછા NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
સ્થિર સંરચના html_stylesheet *
html_create_style_element(html_content *c, dom_ગાંઠ *style)
{
	dom_string *val;
	dom_exception exc;
	સંરચના html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	જો (exc == DOM_NO_ERR && val != ખાલી) {
		જો (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			પાછા ખાલી;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	જો (exc == DOM_NO_ERR && val != ખાલી) {
		જો (strcasestr(dom_string_data(val), "screen") == ખાલી &&
				strcasestr(dom_string_data(val),
						"all") == ખાલી) {
			dom_string_unref(val);
			પાછા ખાલી;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      કદ(સંરચના html_stylesheet) *
			      (c->stylesheet_ગણતરી + 1));
	જો (stylesheets == ખાલી) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_ગણતરી].ગાંઠ = dom_ગાંઠ_ref(style);
	c->stylesheets[c->stylesheet_ગણતરી].sheet = ખાલી;
	c->stylesheets[c->stylesheet_ગણતરી].modified = false;
	c->stylesheets[c->stylesheet_ગણતરી].unused = false;
	c->stylesheet_ગણતરી++;

	પાછા c->stylesheets + (c->stylesheet_ગણતરી - 1);
}


સ્થિર bool
html_css_process_modified_style(html_content *c, સંરચના html_stylesheet *s)
{
	hlcache_handle *sheet = ખાલી;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ગાંઠ, &sheet);
	જો (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, ખાલી);
		પાછા false;
	}

	જો (sheet != ખાલી) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		જો (s->sheet != ખાલી) {
			પસંદગી (content_get_status(s->sheet)) {
			સ્થિતિ CONTENT_STATUS_DONE:
				તોડો;
			મૂળભૂત:
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

	પાછા true;
}


/**
 * process a stylesheet that has been modified.
 */
સ્થિર શૂન્ય html_css_process_modified_styles(શૂન્ય *pw)
{
	html_content *c = pw;
	સંરચના html_stylesheet *s;
	unsigned પૂર્ણાંક i;
	bool all_done = true;

	માટે (i = 0, s = c->stylesheets; i != c->stylesheet_ગણતરી; i++, s++) {
		જો (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	જો (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ગાંઠ *style)
{
	unsigned પૂર્ણાંક i;
	સંરચના html_stylesheet *s;

	/* Find sheet */
	માટે (i = 0, s = c->stylesheets;	i != c->stylesheet_ગણતરી; i++, s++) {
		જો (s->ગાંઠ == style)
			તોડો;
	}
	જો (i == c->stylesheet_ગણતરી) {
		s = html_create_style_element(c, style);
	}
	જો (s == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		પાછા false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	પાછા true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ગાંઠ *ગાંઠ)
{
	unsigned પૂર્ણાંક i;
	dom_string *val;
	dom_exception exc;
	સંરચના html_stylesheet *s;

	/* Find sheet */
	માટે (i = 0, s = c->stylesheets;	i != c->stylesheet_ગણતરી; i++, s++) {
		જો (s->ગાંઠ == ગાંઠ)
			તોડો;
	}

		/* Should already exist */
	જો (i == c->stylesheet_ગણતરી) {
		પાછા false;
	}

	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_media, &val);
	જો (exc == DOM_NO_ERR && val != ખાલી) {
		જો (strcasestr(dom_string_data(val), "screen") == ખાલી &&
				strcasestr(dom_string_data(val),
						"all") == ખાલી) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	પાછા true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ગાંઠ *ગાંઠ)
{
	dom_string *rel, *type_attr, *media, *href;
	સંરચના html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_rel, &rel);
	જો (exc != DOM_NO_ERR || rel == ખાલી)
		પાછા true;

	જો (strcasestr(dom_string_data(rel), "stylesheet") == ખાલી) {
		dom_string_unref(rel);
		પાછા true;
	} નહીંતર જો (strcasestr(dom_string_data(rel), "alternate") != ખાલી) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		પાછા true;
	}
	dom_string_unref(rel);

	જો (nsoption_bool(author_level_css) == false) {
		પાછા true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_type, &type_attr);
	જો (exc == DOM_NO_ERR && type_attr != ખાલી) {
		જો (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			પાછા true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_media, &media);
	જો (exc == DOM_NO_ERR && media != ખાલી) {
		જો (strcasestr(dom_string_data(media), "screen") == ખાલી &&
		    strcasestr(dom_string_data(media), "all") == ખાલી) {
			dom_string_unref(media);
			પાછા true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_href, &href);
	જો (exc != DOM_NO_ERR || href == ખાલી)
		પાછા true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	જો (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		જાઓ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_ગણતરી, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      કદ(સંરચના html_stylesheet) *
			      (htmlc->stylesheet_ગણતરી + 1));
	જો (stylesheets == ખાલી) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		જાઓ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_ગણતરી].ગાંઠ = ખાલી;
	htmlc->stylesheets[htmlc->stylesheet_ગણતરી].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_ગણતરી].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			ખાલી, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_ગણતરી].sheet);

	nsurl_unref(joined);

	જો (ns_error != NSERROR_OK)
		જાઓ no_memory;

	htmlc->stylesheet_ગણતરી++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	પાછા true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, ખાલી);
	પાછા false;
}


/* exported interface documented in html/html.h */
સંરચના html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned પૂર્ણાંક *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);
	પ્રતિજ્ઞા(n != ખાલી);

	*n = c->stylesheet_ગણતરી;

	પાછા c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	સંરચના html_stylesheet *s;
	unsigned પૂર્ણાંક i;

	માટે (i = 0, s = html->stylesheets; i < html->stylesheet_ગણતરી;
	     i++, s++) {
		જો (s->sheet != ખાલી) {
			જો (content_saw_insecure_objects(s->sheet)) {
				પાછા true;
			}
		}
	}

	પાછા false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned પૂર્ણાંક i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	માટે (i = 0; i != html->stylesheet_ગણતરી; i++) {
		જો (html->stylesheets[i].sheet != ખાલી) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		જો (html->stylesheets[i].ગાંઠ != ખાલી) {
			dom_ગાંઠ_unref(html->stylesheets[i].ગાંઠ);
		}
	}
	મુક્ત(html->stylesheets);

	પાછા NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	પ્રતિજ્ઞા(c->stylesheets != ખાલી);

	જો (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), ખાલી,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		જો (ns_error != NSERROR_OK) {
			પાછા ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	પાછા ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	જો (c->stylesheets != ખાલી) {
		પાછા NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			કદ(સંરચના html_stylesheet));
	જો (c->stylesheets == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = ખાલી;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ખાલી;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ખાલી;
	c->stylesheets[STYLESHEET_USER].sheet = ખાલી;
	c->stylesheet_ગણતરી = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), ખાલી,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	જો (ns_error != NSERROR_OK) {
		પાછા ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	જો (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), ખાલી,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		જો (ns_error != NSERROR_OK) {
			પાછા ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), ખાલી,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	જો (ns_error != NSERROR_OK) {
		પાછા ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	પાછા ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	જો (c->stylesheets[STYLESHEET_BASE].sheet == ખાલી) {
		પાછા NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	જો (css_ret != CSS_OK) {
		પાછા css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	માટે (i = STYLESHEET_BASE; i != c->stylesheet_ગણતરી; i++) {
		અચલ સંરચના html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ખાલી;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		જો (hsheet->unused) {
			ચાલુ;
		}

		જો (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} નહીંતર જો (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		જો (hsheet->sheet != ખાલી) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		જો (sheet != ખાલી) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			જો (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				પાછા css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	પાછા NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(શૂન્ય)
{
	nserror error;

	error = html_css_fetcher_register();
	જો (error != NSERROR_OK)
		પાછા error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	જો (error != NSERROR_OK)
		પાછા error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	જો (error != NSERROR_OK)
		પાછા error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	જો (error != NSERROR_OK)
		પાછા error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	પાછા error;
}


/* exported function documented in html/css.h */
શૂન્ય html_css_fini(શૂન્ય)
{
	જો (html_user_stylesheet_url != ખાલી) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ખાલી;
	}

	જો (html_quirks_stylesheet_url != ખાલી) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ખાલી;
	}

	જો (html_adblock_stylesheet_url != ખાલી) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ખાલી;
	}

	જો (html_default_stylesheet_url != ખાલી) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ખાલી;
	}
}
