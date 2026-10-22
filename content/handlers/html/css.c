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

#include "utils/config.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "utils/nsoption.h"
#include "utils/corestrings.h"
#include "utils/log.h"
#include "netsurf/inttypes.h"
#include "netsurf/misc.h"
#include "netsurf/content.h"
#include "content/hlcache.h"
#include "css/css.h"
#include "desktop/gui_internal.h"

#include "html/html.h"
#include "html/private.h"
#include "html/css.h"

static nsurl *html_default_stylesheet_url;
static nsurl *html_adblock_stylesheet_url;
static nsurl *html_quirks_stylesheet_url;
static nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
static nserror css_error_to_nserror(css_error error)
{
	switch (error) {
	pak_sa CSS_OK:
		ཕྱིར NSERROR_OK;

	pak_sa CSS_NOMEM:
		ཕྱིར NSERROR_NOMEM;

	pak_sa CSS_BADPARM:
		ཕྱིར NSERROR_BAD_PARAMETER;

	pak_sa CSS_INVALID:
		ཕྱིར NSERROR_INVALID;

	pak_sa CSS_FILENOTFOUND:
		ཕྱིར NSERROR_NOT_FOUND;

	pak_sa CSS_NEEDDATA:
		ཕྱིར NSERROR_NEED_DATA;

	pak_sa CSS_BADCHARSET:
		ཕྱིར NSERROR_BAD_ENCODING;

	pak_sa CSS_EOF:
	pak_sa CSS_IMPORTS_PENDING:
	pak_sa CSS_PROPERTY_NOT_SET:
	default:
		bha_nga;
	}
	ཕྱིར NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
static nserror
html_convert_css_callback(hlcache_handle *css,
			  const hlcache_event *event,
			  void *pw)
{
	html_content *parent = pw;
	unsigned ཧྲིལ i;
	sa_nracanaa html_stylesheet *s;

	/* Find sheet */
	ཆེད་དུ (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_count;
	     i++, s++) {
		གལ་ཏེ (s->sheet == css)
			bha_nga;
	}

	assert(i != parent->stylesheet_count);

	switch (event->type) {

	pak_sa CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = NULL;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		bha_nga;

	pak_sa CONTENT_MSG_POINTER:
		/* Really don't want this to continue after the switch */
		ཕྱིར NSERROR_OK;

	default:
		bha_nga;
	}

	གལ་ཏེ (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ཕྱིར NSERROR_OK;
}


static nserror
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
	var_na urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_node_get_text_content(node, &style);
	གལ་ཏེ ((exc != DOM_NO_ERR) || (style == NULL)) {
		NSLOG(netsurf, INFO, "No text content");
		ཕྱིར NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &key);
	གལ་ཏེ (error != NSERROR_OK) {
		dom_string_unref(style);
		ཕྱིར error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, parimaa_na(urlbuf), "x-ns-css:%"PRIu32"", key);

	error = nsurl_create(urlbuf, &url);
	གལ་ཏེ (error != NSERROR_OK) {
		ཕྱིར error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	གལ་ཏེ (error != NSERROR_OK) {
		nsurl_unref(url);
		ཕྱིར error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ཕྱིར NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
static sa_nracanaa html_stylesheet *
html_create_style_element(html_content *c, dom_node *style)
{
	dom_string *val;
	dom_exception exc;
	sa_nracanaa html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	གལ་ཏེ (exc == DOM_NO_ERR && val != NULL) {
		གལ་ཏེ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ཕྱིར NULL;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	གལ་ཏེ (exc == DOM_NO_ERR && val != NULL) {
		གལ་ཏེ (strcasestr(dom_string_data(val), "screen") == NULL &&
				strcasestr(dom_string_data(val),
						"all") == NULL) {
			dom_string_unref(val);
			ཕྱིར NULL;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      parimaa_na(sa_nracanaa html_stylesheet) *
			      (c->stylesheet_count + 1));
	གལ་ཏེ (stylesheets == NULL) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ཕྱིར false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_count].node = dom_node_ref(style);
	c->stylesheets[c->stylesheet_count].sheet = NULL;
	c->stylesheets[c->stylesheet_count].modified = false;
	c->stylesheets[c->stylesheet_count].unused = false;
	c->stylesheet_count++;

	ཕྱིར c->stylesheets + (c->stylesheet_count - 1);
}


static bool
html_css_process_modified_style(html_content *c, sa_nracanaa html_stylesheet *s)
{
	hlcache_handle *sheet = NULL;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->node, &sheet);
	གལ་ཏེ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, NULL);
		ཕྱིར false;
	}

	གལ་ཏེ (sheet != NULL) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		གལ་ཏེ (s->sheet != NULL) {
			switch (content_get_status(s->sheet)) {
			pak_sa CONTENT_STATUS_DONE:
				bha_nga;
			default:
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

	ཕྱིར true;
}


/**
 * process a stylesheet that has been modified.
 */
static void html_css_process_modified_styles(void *pw)
{
	html_content *c = pw;
	sa_nracanaa html_stylesheet *s;
	unsigned ཧྲིལ i;
	bool all_done = true;

	ཆེད་དུ (i = 0, s = c->stylesheets; i != c->stylesheet_count; i++, s++) {
		གལ་ཏེ (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	གལ་ཏེ (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_node *style)
{
	unsigned ཧྲིལ i;
	sa_nracanaa html_stylesheet *s;

	/* Find sheet */
	ཆེད་དུ (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		གལ་ཏེ (s->node == style)
			bha_nga;
	}
	གལ་ཏེ (i == c->stylesheet_count) {
		s = html_create_style_element(c, style);
	}
	གལ་ཏེ (s == NULL) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ཕྱིར false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ཕྱིར true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_node *node)
{
	unsigned ཧྲིལ i;
	dom_string *val;
	dom_exception exc;
	sa_nracanaa html_stylesheet *s;

	/* Find sheet */
	ཆེད་དུ (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		གལ་ཏེ (s->node == node)
			bha_nga;
	}

		/* Should already exist */
	གལ་ཏེ (i == c->stylesheet_count) {
		ཕྱིར false;
	}

	exc = dom_element_get_attribute(node, corestring_dom_media, &val);
	གལ་ཏེ (exc == DOM_NO_ERR && val != NULL) {
		གལ་ཏེ (strcasestr(dom_string_data(val), "screen") == NULL &&
				strcasestr(dom_string_data(val),
						"all") == NULL) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ཕྱིར true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_node *node)
{
	dom_string *rel, *type_attr, *media, *href;
	sa_nracanaa html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &rel);
	གལ་ཏེ (exc != DOM_NO_ERR || rel == NULL)
		ཕྱིར true;

	གལ་ཏེ (strcasestr(dom_string_data(rel), "stylesheet") == NULL) {
		dom_string_unref(rel);
		ཕྱིར true;
	} དེ་མིན གལ་ཏེ (strcasestr(dom_string_data(rel), "alternate") != NULL) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ཕྱིར true;
	}
	dom_string_unref(rel);

	གལ་ཏེ (nsoption_bool(author_level_css) == false) {
		ཕྱིར true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_type, &type_attr);
	གལ་ཏེ (exc == DOM_NO_ERR && type_attr != NULL) {
		གལ་ཏེ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ཕྱིར true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_media, &media);
	གལ་ཏེ (exc == DOM_NO_ERR && media != NULL) {
		གལ་ཏེ (strcasestr(dom_string_data(media), "screen") == NULL &&
		    strcasestr(dom_string_data(media), "all") == NULL) {
			dom_string_unref(media);
			ཕྱིར true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(node, corestring_dom_href, &href);
	གལ་ཏེ (exc != DOM_NO_ERR || href == NULL)
		ཕྱིར true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	གལ་ཏེ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		goto no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_count, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      parimaa_na(sa_nracanaa html_stylesheet) *
			      (htmlc->stylesheet_count + 1));
	གལ་ཏེ (stylesheets == NULL) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		goto no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_count].node = NULL;
	htmlc->stylesheets[htmlc->stylesheet_count].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_count].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			NULL, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_count].sheet);

	nsurl_unref(joined);

	གལ་ཏེ (ns_error != NSERROR_OK)
		goto no_memory;

	htmlc->stylesheet_count++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ཕྱིར true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, NULL);
	ཕྱིར false;
}


/* exported interface documented in html/html.h */
sa_nracanaa html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned ཧྲིལ *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->stylesheet_count;

	ཕྱིར c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	sa_nracanaa html_stylesheet *s;
	unsigned ཧྲིལ i;

	ཆེད་དུ (i = 0, s = html->stylesheets; i < html->stylesheet_count;
	     i++, s++) {
		གལ་ཏེ (s->sheet != NULL) {
			གལ་ཏེ (content_saw_insecure_objects(s->sheet)) {
				ཕྱིར true;
			}
		}
	}

	ཕྱིར false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned ཧྲིལ i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	ཆེད་དུ (i = 0; i != html->stylesheet_count; i++) {
		གལ་ཏེ (html->stylesheets[i].sheet != NULL) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		གལ་ཏེ (html->stylesheets[i].node != NULL) {
			dom_node_unref(html->stylesheets[i].node);
		}
	}
	free(html->stylesheets);

	ཕྱིར NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	assert(c->stylesheets != NULL);

	གལ་ཏེ (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		གལ་ཏེ (ns_error != NSERROR_OK) {
			ཕྱིར ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ཕྱིར ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	གལ་ཏེ (c->stylesheets != NULL) {
		ཕྱིར NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			parimaa_na(sa_nracanaa html_stylesheet));
	གལ་ཏེ (c->stylesheets == NULL) {
		ཕྱིར NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = NULL;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = NULL;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = NULL;
	c->stylesheets[STYLESHEET_USER].sheet = NULL;
	c->stylesheet_count = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	གལ་ཏེ (ns_error != NSERROR_OK) {
		ཕྱིར ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	གལ་ཏེ (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		གལ་ཏེ (ns_error != NSERROR_OK) {
			ཕྱིར ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	གལ་ཏེ (ns_error != NSERROR_OK) {
		ཕྱིར ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ཕྱིར ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	གལ་ཏེ (c->stylesheets[STYLESHEET_BASE].sheet == NULL) {
		ཕྱིར NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	གལ་ཏེ (css_ret != CSS_OK) {
		ཕྱིར css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	ཆེད་དུ (i = STYLESHEET_BASE; i != c->stylesheet_count; i++) {
		const sa_nracanaa html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = NULL;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		གལ་ཏེ (hsheet->unused) {
			continue;
		}

		གལ་ཏེ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} དེ་མིན གལ་ཏེ (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		གལ་ཏེ (hsheet->sheet != NULL) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		གལ་ཏེ (sheet != NULL) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			གལ་ཏེ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ཕྱིར css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ཕྱིར NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(void)
{
	nserror error;

	error = html_css_fetcher_register();
	གལ་ཏེ (error != NSERROR_OK)
		ཕྱིར error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	གལ་ཏེ (error != NSERROR_OK)
		ཕྱིར error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	གལ་ཏེ (error != NSERROR_OK)
		ཕྱིར error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	གལ་ཏེ (error != NSERROR_OK)
		ཕྱིར error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ཕྱིར error;
}


/* exported function documented in html/css.h */
void html_css_fini(void)
{
	གལ་ཏེ (html_user_stylesheet_url != NULL) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = NULL;
	}

	གལ་ཏེ (html_quirks_stylesheet_url != NULL) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = NULL;
	}

	གལ་ཏེ (html_adblock_stylesheet_url != NULL) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = NULL;
	}

	གལ་ཏེ (html_default_stylesheet_url != NULL) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = NULL;
	}
}
