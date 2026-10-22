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
	case CSS_OK:
		върни NSERROR_OK;

	case CSS_NOMEM:
		върни NSERROR_NOMEM;

	case CSS_BADPARM:
		върни NSERROR_BAD_PARAMETER;

	case CSS_INVALID:
		върни NSERROR_INVALID;

	case CSS_FILENOTFOUND:
		върни NSERROR_NOT_FOUND;

	case CSS_NEEDDATA:
		върни NSERROR_NEED_DATA;

	case CSS_BADCHARSET:
		върни NSERROR_BAD_ENCODING;

	case CSS_EOF:
	case CSS_IMPORTS_PENDING:
	case CSS_PROPERTY_NOT_SET:
	default:
		break;
	}
	върни NSERROR_CSS;
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
	unsigned цяло i;
	struct html_stylesheet *s;

	/* Find sheet */
	за (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_count;
	     i++, s++) {
		ако (s->sheet == css)
			break;
	}

	assert(i != parent->stylesheet_count);

	switch (event->type) {

	case CONTENT_MSG_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		break;

	case CONTENT_MSG_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = NULL;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		break;

	case CONTENT_MSG_POINTER:
		/* Really don't want this to continue after the switch */
		върни NSERROR_OK;

	default:
		break;
	}

	ако (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	върни NSERROR_OK;
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
	char urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_node_get_text_content(node, &style);
	ако ((exc != DOM_NO_ERR) || (style == NULL)) {
		NSLOG(netsurf, INFO, "No text content");
		върни NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &key);
	ако (error != NSERROR_OK) {
		dom_string_unref(style);
		върни error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, sizeof(urlbuf), "x-ns-css:%"PRIu32"", key);

	error = nsurl_create(urlbuf, &url);
	ако (error != NSERROR_OK) {
		върни error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	ако (error != NSERROR_OK) {
		nsurl_unref(url);
		върни error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	върни NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
static struct html_stylesheet *
html_create_style_element(html_content *c, dom_node *style)
{
	dom_string *val;
	dom_exception exc;
	struct html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ако (exc == DOM_NO_ERR && val != NULL) {
		ако (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			върни NULL;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ако (exc == DOM_NO_ERR && val != NULL) {
		ако (strcasestr(dom_string_data(val), "screen") == NULL &&
				strcasestr(dom_string_data(val),
						"all") == NULL) {
			dom_string_unref(val);
			върни NULL;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      sizeof(struct html_stylesheet) *
			      (c->stylesheet_count + 1));
	ако (stylesheets == NULL) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		върни false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_count].node = dom_node_ref(style);
	c->stylesheets[c->stylesheet_count].sheet = NULL;
	c->stylesheets[c->stylesheet_count].modified = false;
	c->stylesheets[c->stylesheet_count].unused = false;
	c->stylesheet_count++;

	върни c->stylesheets + (c->stylesheet_count - 1);
}


static bool
html_css_process_modified_style(html_content *c, struct html_stylesheet *s)
{
	hlcache_handle *sheet = NULL;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->node, &sheet);
	ако (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, NULL);
		върни false;
	}

	ако (sheet != NULL) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ако (s->sheet != NULL) {
			switch (content_get_status(s->sheet)) {
			case CONTENT_STATUS_DONE:
				break;
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

	върни true;
}


/**
 * process a stylesheet that has been modified.
 */
static void html_css_process_modified_styles(void *pw)
{
	html_content *c = pw;
	struct html_stylesheet *s;
	unsigned цяло i;
	bool all_done = true;

	за (i = 0, s = c->stylesheets; i != c->stylesheet_count; i++, s++) {
		ако (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ако (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_node *style)
{
	unsigned цяло i;
	struct html_stylesheet *s;

	/* Find sheet */
	за (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		ако (s->node == style)
			break;
	}
	ако (i == c->stylesheet_count) {
		s = html_create_style_element(c, style);
	}
	ако (s == NULL) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		върни false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	върни true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_node *node)
{
	unsigned цяло i;
	dom_string *val;
	dom_exception exc;
	struct html_stylesheet *s;

	/* Find sheet */
	за (i = 0, s = c->stylesheets;	i != c->stylesheet_count; i++, s++) {
		ако (s->node == node)
			break;
	}

		/* Should already exist */
	ако (i == c->stylesheet_count) {
		върни false;
	}

	exc = dom_element_get_attribute(node, corestring_dom_media, &val);
	ако (exc == DOM_NO_ERR && val != NULL) {
		ако (strcasestr(dom_string_data(val), "screen") == NULL &&
				strcasestr(dom_string_data(val),
						"all") == NULL) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	върни true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_node *node)
{
	dom_string *rel, *type_attr, *media, *href;
	struct html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &rel);
	ако (exc != DOM_NO_ERR || rel == NULL)
		върни true;

	ако (strcasestr(dom_string_data(rel), "stylesheet") == NULL) {
		dom_string_unref(rel);
		върни true;
	} иначе ако (strcasestr(dom_string_data(rel), "alternate") != NULL) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		върни true;
	}
	dom_string_unref(rel);

	ако (nsoption_bool(author_level_css) == false) {
		върни true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_type, &type_attr);
	ако (exc == DOM_NO_ERR && type_attr != NULL) {
		ако (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			върни true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(node, corestring_dom_media, &media);
	ако (exc == DOM_NO_ERR && media != NULL) {
		ако (strcasestr(dom_string_data(media), "screen") == NULL &&
		    strcasestr(dom_string_data(media), "all") == NULL) {
			dom_string_unref(media);
			върни true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(node, corestring_dom_href, &href);
	ако (exc != DOM_NO_ERR || href == NULL)
		върни true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	ако (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		goto no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_count, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      sizeof(struct html_stylesheet) *
			      (htmlc->stylesheet_count + 1));
	ако (stylesheets == NULL) {
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

	ако (ns_error != NSERROR_OK)
		goto no_memory;

	htmlc->stylesheet_count++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	върни true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, NULL);
	върни false;
}


/* exported interface documented in html/html.h */
struct html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned цяло *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->stylesheet_count;

	върни c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	struct html_stylesheet *s;
	unsigned цяло i;

	за (i = 0, s = html->stylesheets; i < html->stylesheet_count;
	     i++, s++) {
		ако (s->sheet != NULL) {
			ако (content_saw_insecure_objects(s->sheet)) {
				върни true;
			}
		}
	}

	върни false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned цяло i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	за (i = 0; i != html->stylesheet_count; i++) {
		ако (html->stylesheets[i].sheet != NULL) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ако (html->stylesheets[i].node != NULL) {
			dom_node_unref(html->stylesheets[i].node);
		}
	}
	free(html->stylesheets);

	върни NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	assert(c->stylesheets != NULL);

	ако (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ако (ns_error != NSERROR_OK) {
			върни ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	върни ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	ако (c->stylesheets != NULL) {
		върни NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			sizeof(struct html_stylesheet));
	ако (c->stylesheets == NULL) {
		върни NSERROR_NOMEM;
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
	ако (ns_error != NSERROR_OK) {
		върни ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	ако (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), NULL,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ако (ns_error != NSERROR_OK) {
			върни ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), NULL,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ако (ns_error != NSERROR_OK) {
		върни ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	върни ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ако (c->stylesheets[STYLESHEET_BASE].sheet == NULL) {
		върни NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ако (css_ret != CSS_OK) {
		върни css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	за (i = STYLESHEET_BASE; i != c->stylesheet_count; i++) {
		const struct html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = NULL;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ако (hsheet->unused) {
			continue;
		}

		ако (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} иначе ако (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		ако (hsheet->sheet != NULL) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ако (sheet != NULL) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ако (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				върни css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	върни NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(void)
{
	nserror error;

	error = html_css_fetcher_register();
	ако (error != NSERROR_OK)
		върни error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ако (error != NSERROR_OK)
		върни error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ако (error != NSERROR_OK)
		върни error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ако (error != NSERROR_OK)
		върни error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	върни error;
}


/* exported function documented in html/css.h */
void html_css_fini(void)
{
	ако (html_user_stylesheet_url != NULL) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = NULL;
	}

	ако (html_quirks_stylesheet_url != NULL) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = NULL;
	}

	ако (html_adblock_stylesheet_url != NULL) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = NULL;
	}

	ако (html_default_stylesheet_url != NULL) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = NULL;
	}
}
