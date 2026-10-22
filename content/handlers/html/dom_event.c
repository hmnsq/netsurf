/*
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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
 * Implementation of HTML content DOM event handling.
 */

#include <string.h>

#include "utils/config.h"
#include "utils/utils.h"
#include "utils/corestrings.h"
#include "utils/nsoption.h"
#include "utils/log.h"
#include "utils/ascii.h"
#include "utils/string.h"
#include "utils/nsurl.h"
#include "content/content.h"
#include "javascript/js.h"

#include "netsurf/bitmap.h"

#include "html/private.h"
#include "html/object.h"
#include "html/css.h"
#include "html/box.h"
#include "html/box_construct.h"
#include "html/form_internal.h"
#include "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
static bool html_process_inserted_base(html_content *htmlc, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(node, corestring_dom_href, &atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		ᤀᤡ᤺ (error == NSERROR_OK) {
			ᤀᤡ᤺ (htmlc->base_url != NULL) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ᤀᤡ᤺ (htmlc->base_target != NULL) {
		ᤀᤡᤴ true;
	}

	exc = dom_element_get_attribute(node,
					corestring_dom_target,
					&atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ᤀᤡ᤺ (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__top)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ᤀᤡᤴ true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
static bool html_process_inserted_img(html_content *htmlc, dom_node *node)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	ᤀᤡ᤺ (nsoption_bool(foreground_images) == false) {
		ᤀᤡᤴ true;
	}

	exc = dom_element_get_attribute(node, corestring_dom_src, &src);
	ᤀᤡ᤺ (exc != DOM_NO_ERR || src == NULL) {
		ᤀᤡᤴ true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	ᤀᤡ᤺ (err != NSERROR_OK) {
		dom_string_unref(src);
		ᤀᤡᤴ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, NULL, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ᤀᤡᤴ success;
}


/**
 * process a LINK element being inserted into the DOM
 *
 * \note only the http-equiv attribute for refresh is currently considered
 *
 * \param htmlc The html content containing the DOM
 * \param n The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
static bool html_process_inserted_link(html_content *c, dom_node *node)
{
	sa_nracanaa content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_node *)node);

	/* try Generic link handling */

	memset(&link, 0, parimaa_na(sa_nracanaa content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &atr_string);
	ᤀᤡ᤺ ((exc != DOM_NO_ERR) || (atr_string == NULL)) {
		ᤀᤡᤴ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ᤀᤡ᤺ (exc != DOM_NO_ERR) {
		ᤀᤡᤴ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_href, &atr_string);
	ᤀᤡ᤺ ((exc != DOM_NO_ERR) || (atr_string == NULL)) {
		lwc_string_unref(link.rel);
		ᤀᤡᤴ false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ᤀᤡ᤺ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ᤀᤡᤴ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(node,
			corestring_dom_hreflang, &atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the href lang */
		(void)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_type, &atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the type */
		(void)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_media, &atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the media */
		(void)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_sizes, &atr_string);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (atr_string != NULL)) {
		/* get a lwc string containing the sizes */
		(void)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content__add_rfc5988_link(&c->base, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	ᤀᤡᤴ true;
}


/* handler for a SCRIPT which has been added to a tree */
static void
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ᤀᤡ᤺ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ᤀᤡᤴ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ᤀᤡ᤺ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ᤀᤡᤴ;
	}

	ᤀᤡ᤺ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ᤀᤡᤴ;
	}

	exc = dom_node_contains(htmlc->document, script, &within);
	ᤀᤡ᤺ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ᤀᤡᤴ;
	}

	ᤀᤡ᤺ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ᤀᤡᤴ;
	}

	res = html_process_script(htmlc, (dom_node *) script);
	ᤀᤡ᤺ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ᤀᤡ᤺ᤆᤡ {
		ᤀᤡ᤺ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ᤀᤡ᤺ᤆᤡ {
			NSLOG(netsurf, DEEPDEBUG, "Failure starting script");
		}
	}
}


/**
 * process a META element being inserted into the DOM
 *
 * \note only the http-equiv attribute for refresh is currently considered
 *
 * \param htmlc The html content containing the DOM
 * \param n The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
static nserror html_process_inserted_meta(html_content *c, dom_node *n)
{
	sa_ngha content_msg_data msg_data;
	const var_na *url, *end, *refresh = NULL;
	var_na *new_url;
	var_na quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ᤀᤡ᤺ (c->refresh) {
		/* refresh already delt with */
		ᤀᤡᤴ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ᤀᤡ᤺ (exc != DOM_NO_ERR) {
		ᤀᤡᤴ NSERROR_DOM;
	}

	ᤀᤡ᤺ (equiv == NULL) {
		ᤀᤡᤴ NSERROR_OK;
	}

	ᤀᤡ᤺ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ᤀᤡᤴ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ᤀᤡ᤺ (exc != DOM_NO_ERR) {
		ᤀᤡᤴ NSERROR_DOM;
	}

	ᤀᤡ᤺ (content == NULL) {
		ᤀᤡᤴ NSERROR_OK;
	}

	end = dom_string_data(content) + dom_string_byte_length(content);

	/* content  := *LWS intpart fracpart? *LWS [';' *LWS *1url *LWS]
	 * intpart  := 1*DIGIT
	 * fracpart := 1*('.' | DIGIT)
	 * url      := "url" *LWS '=' *LWS (url-nq | url-sq | url-dq)
	 * url-nq   := *urlchar
	 * url-sq   := "'" *(urlchar | '"') "'"
	 * url-dq   := '"' *(urlchar | "'") '"'
	 * urlchar  := [#x9#x21#x23-#x26#x28-#x7E] | nonascii
	 * nonascii := [#x80-#xD7FF#xE000-#xFFFD#x10000-#x10FFFF]
	 */

	url = dom_string_data(content);

	/* *LWS */
	ᤀᤠ᤺ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ᤀᤡ᤺ (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ᤀᤡᤴ NSERROR_OK;
	}

	msg_data.delay = (puur_naa_nka) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ᤀᤡ᤺ (msg_data.delay < 1) {
		msg_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ᤀᤠ᤺ (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ᤀᤠ᤺ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ᤀᤡ᤺ (url < end && *url == ';')
		url++;

	/* *LWS */
	ᤀᤠ᤺ (url < end && ascii_is_space(*url)) {
		url++;
	}

	ᤀᤡ᤺ (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_MSG_REFRESH, &msg_data);

		ᤀᤡᤴ NSERROR_OK;
	}

	/* "url" */
	ᤀᤡ᤺ (url <= end - 3) {
		ᤀᤡ᤺ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ᤀᤡ᤺ᤆᤡ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ᤀᤡᤴ NSERROR_OK;
		}
	} ᤀᤡ᤺ᤆᤡ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ᤀᤡᤴ NSERROR_OK;
	}

	/* *LWS */
	ᤀᤠ᤺ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ᤀᤡ᤺ (url < end) {
		ᤀᤡ᤺ (*url == '=') {
			url++;
		} ᤀᤡ᤺ᤆᤡ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ᤀᤡᤴ NSERROR_OK;
		}
	} ᤀᤡ᤺ᤆᤡ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ᤀᤡᤴ NSERROR_OK;
	}

	/* *LWS */
	ᤀᤠ᤺ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ᤀᤡ᤺ (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ᤀᤡ᤺ (quote != 0) {
		/* url-sq | url-dq */
		ᤀᤠ᤺ (url < end && *url != quote)
			url++;
	} ᤀᤡ᤺ᤆᤡ {
		/* url-nq */
		ᤀᤠ᤺ (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ᤀᤡ᤺ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ᤀᤡ᤺ (new_url == NULL) {
			dom_string_unref(content);
			ᤀᤡᤴ NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		ᤀᤡ᤺ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_MSG_REFRESH,
					  &msg_data);
			c->refresh = true;
		}

		free(new_url);

	}

	dom_string_unref(content);

	ᤀᤡᤴ error;
}


/**
 * Process title element being inserted into the DOM.
 *
 * https://html.spec.whatwg.org/multipage/semantics.html#the-title-element
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
static nserror html_process_inserted_title(html_content *htmlc, dom_node *node)
{
	ᤀᤡ᤺ (htmlc->title == NULL) {
		/* only the first title is considered */
		htmlc->title = dom_node_ref(node);
	}
	ᤀᤡᤴ NSERROR_OK;
}


/** process title node */
static bool html_process_title(html_content *c, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	var_na *title_str;
	bool success;

	exc = dom_node_get_text_content(node, &title);
	ᤀᤡ᤺ ((exc != DOM_NO_ERR) || (title == NULL)) {
		ᤀᤡᤴ false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	ᤀᤡ᤺ (title_str == NULL) {
		ᤀᤡᤴ false;
	}

	success = content__set_title(&c->base, title_str);

	free(title_str);

	ᤀᤡᤴ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
static void html_texty_element_update(html_content *htmlc, dom_node *node)
{
	sa_nracanaa box *box = box_for_node(node);
	ᤀᤡ᤺ (box == NULL) {
		ᤀᤡᤴ; /* No Box (yet?) so no gadget to update */
	}
	ᤀᤡ᤺ (box->gadget == NULL) {
		ᤀᤡᤴ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
static void
dom_default_action_DOMNodeInserted_cb(sa_nracanaa dom_event *evt, void *pw)
{
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &node);
	ᤀᤡ᤺ ((exc != DOM_NO_ERR) || (node == NULL)) {
		/* failed to obtain the event target node */
		ᤀᤡᤴ;
	}

	exc = dom_node_get_node_type(node, &type);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(node, &tag_type);
		ᤀᤡ᤺ (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		switch (tag_type) {
		pak_sa DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_node *)node);
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_node *)node);
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_node *)node);
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_node *)node);
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_STYLE:
			ᤀᤡ᤺ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_node *)node);
			}
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)node);
			bha_nga;

		pak_sa DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_node *)node);
			bha_nga;

		default:
			bha_nga;
		}

		ᤀᤡ᤺ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ᤀᤡ᤺ (htmlc->jsthread == NULL) {
				sa_ngha content_msg_data msg_data;

				msg_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_MSG_GETTHREAD,
						  &msg_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ᤀᤡ᤺ (htmlc->jsthread != NULL) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) node);
			}
		}
	}
	dom_node_unref(node);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
static void
dom_default_action_DOMNodeInsertedIntoDocument_cb(sa_nracanaa dom_event *evt,
						  void *pw)
{
	html_content *htmlc = pw;
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &node);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (node != NULL)) {
		exc = dom_node_get_node_type(node, &type);
		ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(node, &tag_type);
			ᤀᤡ᤺ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			switch (tag_type) {
			pak_sa DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) node);
				fallthrough;
			default:
				bha_nga;
			}
		}
		dom_node_unref(node);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
static void
dom_default_action_DOMSubtreeModified_cb(sa_nracanaa dom_event *evt, void *pw)
{
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &node);
	ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (node != NULL)) {
		ᤀᤡ᤺ (htmlc->title == (dom_node *)node) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_node *)node);
			dom_node_unref(node);
			ᤀᤡᤴ;
		}

		exc = dom_node_get_node_type(node, &type);
		ᤀᤡ᤺ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(node, &tag_type);
			ᤀᤡ᤺ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			switch (tag_type) {
			pak_sa DOM_HTML_ELEMENT_TYPE_STYLE:
				ᤀᤡ᤺ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_node *)node);
				}
				bha_nga;
			pak_sa DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			pak_sa DOM_HTML_ELEMENT_TYPE_INPUT:
				html_texty_element_update(htmlc, (dom_node *)node);
				fallthrough;
			default:
				bha_nga;
			}
		}
		dom_node_unref(node);
	}
}


/**
 * callback for default action finished
 */
static void
dom_default_action_finished_cb(sa_nracanaa dom_event *evt, void *pw)
{
	html_content *htmlc = pw;

	ᤀᤡ᤺ (htmlc->jsthread != NULL)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  void **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	ᤀᤡ᤺ (phase == DOM_DEFAULT_ACTION_END) {
		ᤀᤡ᤺ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ᤀᤡᤴ dom_default_action_DOMNodeInserted_cb;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ᤀᤡᤴ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ᤀᤡᤴ dom_default_action_DOMSubtreeModified_cb;
		}
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (phase == DOM_DEFAULT_ACTION_FINISHED) {
		ᤀᤡᤴ dom_default_action_finished_cb;
	}
	ᤀᤡᤴ NULL;
}
