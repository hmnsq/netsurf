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

#ضمن <string.h>

#ضمن "utils/config.h"
#ضمن "utils/utils.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/log.h"
#ضمن "utils/ascii.h"
#ضمن "utils/string.h"
#ضمن "utils/nsurl.h"
#ضمن "content/content.h"
#ضمن "javascript/js.h"

#ضمن "netsurf/bitmap.h"

#ضمن "html/private.h"
#ضمن "html/object.h"
#ضمن "html/css.h"
#ضمن "html/box.h"
#ضمن "html/box_construct.h"
#ضمن "html/form_internal.h"
#ضمن "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ساكن bool html_process_inserted_base(html_content *htmlc, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(node, corestring_dom_href, &atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		אם (error == NSERROR_OK) {
			אם (htmlc->base_url != عدم) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	אם (htmlc->base_target != عدم) {
		החזר true;
	}

	exc = dom_element_get_attribute(node,
					corestring_dom_target,
					&atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		אם (*dom_string_data(atr_string) != '_' ||
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

	החזר true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ساكن bool html_process_inserted_img(html_content *htmlc, dom_node *node)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	אם (nsoption_bool(foreground_images) == false) {
		החזר true;
	}

	exc = dom_element_get_attribute(node, corestring_dom_src, &src);
	אם (exc != DOM_NO_ERR || src == عدم) {
		החזר true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	אם (err != NSERROR_OK) {
		dom_string_unref(src);
		החזר false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, عدم, CONTENT_IMAGE, false);
	nsurl_unref(url);

	החזר success;
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
ساكن bool html_process_inserted_link(html_content *c, dom_node *node)
{
	بنية content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_node *)node);

	/* try Generic link handling */

	memset(&link, 0, حجم(بنية content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_rel, &atr_string);
	אם ((exc != DOM_NO_ERR) || (atr_string == عدم)) {
		החזר false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	אם (exc != DOM_NO_ERR) {
		החזר false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(node, corestring_dom_href, &atr_string);
	אם ((exc != DOM_NO_ERR) || (atr_string == عدم)) {
		lwc_string_unref(link.rel);
		החזר false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	אם (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		החזר false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(node,
			corestring_dom_hreflang, &atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the href lang */
		(فراغ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_type, &atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the type */
		(فراغ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_media, &atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the media */
		(فراغ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(node,
			corestring_dom_sizes, &atr_string);
	אם ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the sizes */
		(فراغ)dom_string_intern(atr_string, &link.sizes);
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

	החזר true;
}


/* handler for a SCRIPT which has been added to a tree */
ساكن فراغ
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	אם (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		החזר;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	אם (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		החזר;
	}

	אם (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		החזר;
	}

	exc = dom_node_contains(htmlc->document, script, &within);
	אם (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		החזר;
	}

	אם (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		החזר;
	}

	res = html_process_script(htmlc, (dom_node *) script);
	אם (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} אחרת {
		אם (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} אחרת {
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
ساكن nserror html_process_inserted_meta(html_content *c, dom_node *n)
{
	اتحاد content_msg_data msg_data;
	ثابت حرف *url, *end, *refresh = عدم;
	حرف *new_url;
	حرف quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	אם (c->refresh) {
		/* refresh already delt with */
		החזר NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	אם (exc != DOM_NO_ERR) {
		החזר NSERROR_DOM;
	}

	אם (equiv == عدم) {
		החזר NSERROR_OK;
	}

	אם (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		החזר NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	אם (exc != DOM_NO_ERR) {
		החזר NSERROR_DOM;
	}

	אם (content == عدم) {
		החזר NSERROR_OK;
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
	כלעוד (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	אם (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		החזר NSERROR_OK;
	}

	msg_data.delay = (صحيح) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	אם (msg_data.delay < 1) {
		msg_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	כלעוד (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	כלעוד (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	אם (url < end && *url == ';')
		url++;

	/* *LWS */
	כלעוד (url < end && ascii_is_space(*url)) {
		url++;
	}

	אם (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_MSG_REFRESH, &msg_data);

		החזר NSERROR_OK;
	}

	/* "url" */
	אם (url <= end - 3) {
		אם (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} אחרת {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			החזר NSERROR_OK;
		}
	} אחרת {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		החזר NSERROR_OK;
	}

	/* *LWS */
	כלעוד (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	אם (url < end) {
		אם (*url == '=') {
			url++;
		} אחרת {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			החזר NSERROR_OK;
		}
	} אחרת {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		החזר NSERROR_OK;
	}

	/* *LWS */
	כלעוד (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	אם (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	אם (quote != 0) {
		/* url-sq | url-dq */
		כלעוד (url < end && *url != quote)
			url++;
	} אחרת {
		/* url-nq */
		כלעוד (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	אם (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		אם (new_url == عدم) {
			dom_string_unref(content);
			החזר NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		אם (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_MSG_REFRESH,
					  &msg_data);
			c->refresh = true;
		}

		حرر(new_url);

	}

	dom_string_unref(content);

	החזר error;
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
ساكن nserror html_process_inserted_title(html_content *htmlc, dom_node *node)
{
	אם (htmlc->title == عدم) {
		/* only the first title is considered */
		htmlc->title = dom_node_ref(node);
	}
	החזר NSERROR_OK;
}


/** process title node */
ساكن bool html_process_title(html_content *c, dom_node *node)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	حرف *title_str;
	bool success;

	exc = dom_node_get_text_content(node, &title);
	אם ((exc != DOM_NO_ERR) || (title == عدم)) {
		החזר false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	אם (title_str == عدم) {
		החזר false;
	}

	success = content__set_title(&c->base, title_str);

	حرر(title_str);

	החזר success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ساكن فراغ html_texty_element_update(html_content *htmlc, dom_node *node)
{
	بنية box *box = box_for_node(node);
	אם (box == عدم) {
		החזר; /* No Box (yet?) so no gadget to update */
	}
	אם (box->gadget == عدم) {
		החזר; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ساكن فراغ
dom_default_action_DOMNodeInserted_cb(بنية dom_event *evt, فراغ *pw)
{
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &node);
	אם ((exc != DOM_NO_ERR) || (node == عدم)) {
		/* failed to obtain the event target node */
		החזר;
	}

	exc = dom_node_get_node_type(node, &type);
	אם ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(node, &tag_type);
		אם (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		بدل (tag_type) {
		حالة DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_node *)node);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_node *)node);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_node *)node);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_node *)node);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_STYLE:
			אם (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_node *)node);
			}
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)node);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_node *)node);
			اكسر;

		افتراضي:
			اكسر;
		}

		אם (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			אם (htmlc->jsthread == عدم) {
				اتحاد content_msg_data msg_data;

				msg_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_MSG_GETTHREAD,
						  &msg_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			אם (htmlc->jsthread != عدم) {
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
ساكن فراغ
dom_default_action_DOMNodeInsertedIntoDocument_cb(بنية dom_event *evt,
						  فراغ *pw)
{
	html_content *htmlc = pw;
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &node);
	אם ((exc == DOM_NO_ERR) && (node != عدم)) {
		exc = dom_node_get_node_type(node, &type);
		אם ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(node, &tag_type);
			אם (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			بدل (tag_type) {
			حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) node);
				fallthrough;
			افتراضي:
				اكسر;
			}
		}
		dom_node_unref(node);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ساكن فراغ
dom_default_action_DOMSubtreeModified_cb(بنية dom_event *evt, فراغ *pw)
{
	dom_event_target *node;
	dom_node_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &node);
	אם ((exc == DOM_NO_ERR) && (node != عدم)) {
		אם (htmlc->title == (dom_node *)node) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_node *)node);
			dom_node_unref(node);
			החזר;
		}

		exc = dom_node_get_node_type(node, &type);
		אם ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_NODE)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(node, &tag_type);
			אם (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			بدل (tag_type) {
			حالة DOM_HTML_ELEMENT_TYPE_STYLE:
				אם (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_node *)node);
				}
				اكسر;
			حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			حالة DOM_HTML_ELEMENT_TYPE_INPUT:
				html_texty_element_update(htmlc, (dom_node *)node);
				fallthrough;
			افتراضي:
				اكسر;
			}
		}
		dom_node_unref(node);
	}
}


/**
 * callback for default action finished
 */
ساكن فراغ
dom_default_action_finished_cb(بنية dom_event *evt, فراغ *pw)
{
	html_content *htmlc = pw;

	אם (htmlc->jsthread != عدم)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  فراغ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	אם (phase == DOM_DEFAULT_ACTION_END) {
		אם (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			החזר dom_default_action_DOMNodeInserted_cb;
		} אחרת אם (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			החזר dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} אחרת אם (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			החזר dom_default_action_DOMSubtreeModified_cb;
		}
	} אחרת אם (phase == DOM_DEFAULT_ACTION_FINISHED) {
		החזר dom_default_action_finished_cb;
	}
	החזר عدم;
}
