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
ساكن bool html_process_inserted_پایه(html_content *htmlc, dom_گره *گره)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(گره, corestring_dom_href, &atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_داده(atr_string), &url);
		dom_string_unref(atr_string);
		إذا (error == NSERROR_OK) {
			إذا (htmlc->پایه_url != عدم) {
				nsurl_unref(htmlc->پایه_url);
			}
			htmlc->پایه_url = url;
		}
	}


	/* get target attribute if present and not already set */
	إذا (htmlc->پایه_هدف != عدم) {
		أرجع true;
	}

	exc = dom_element_get_attribute(گره,
					corestring_dom_هدف,
					&atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		إذا (*dom_string_داده(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_والد) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_بالاترین_2)) {
			htmlc->پایه_هدف = strdup(dom_string_داده(atr_string));
		}
		dom_string_unref(atr_string);
	}

	أرجع true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ساكن bool html_process_inserted_img(html_content *htmlc, dom_گره *گره)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	إذا (nsoption_bool(foreground_images) == false) {
		أرجع true;
	}

	exc = dom_element_get_attribute(گره, corestring_dom_src, &src);
	إذا (exc != DOM_NO_ERR || src == عدم) {
		أرجع true;
	}

	err = nsurl_join(htmlc->پایه_url, dom_string_داده(src), &url);
	إذا (err != NSERROR_OK) {
		dom_string_unref(src);
		أرجع false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, عدم, CONTENT_IMAGE, false);
	nsurl_unref(url);

	أرجع success;
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
ساكن bool html_process_inserted_link(html_content *c, dom_گره *گره)
{
	بنية content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_گره *)گره);

	/* try Generic link handling */

	memset(&link, 0, حجم(بنية content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گره, corestring_dom_rel, &atr_string);
	إذا ((exc != DOM_NO_ERR) || (atr_string == عدم)) {
		أرجع false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گره, corestring_dom_href, &atr_string);
	إذا ((exc != DOM_NO_ERR) || (atr_string == عدم)) {
		lwc_string_unref(link.rel);
		أرجع false;
	}

	/* get nsurl */
	error = nsurl_join(c->پایه_url, dom_string_داده(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	إذا (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		أرجع false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(گره,
			corestring_dom_hreflang, &atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the href lang */
		(فراغ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_type, &atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the type */
		(فراغ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_media, &atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the media */
		(فراغ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_sizes, &atr_string);
	إذا ((exc == DOM_NO_ERR) && (atr_string != عدم)) {
		/* get a lwc string containing the sizes */
		(فراغ)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content_جمع_کن_rfc5988_link(&c->پایه, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	أرجع true;
}


/* handler for a SCRIPT which has been added to a tree */
ساكن فراغ
dom_SCRIPT_showed_به_بالا(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	إذا (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		أرجع;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	إذا (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		أرجع;
	}

	إذا (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		أرجع;
	}

	exc = dom_گره_شامل_است(htmlc->document, script, &within);
	إذا (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		أرجع;
	}

	إذا (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		أرجع;
	}

	res = html_process_script(htmlc, (dom_گره *) script);
	إذا (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} وإلا {
		إذا (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} وإلا {
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
ساكن nserror html_process_inserted_meta(html_content *c, dom_گره *n)
{
	اتحاد content_پیام_داده پیام_داده;
	ثابت حرف *url, *انتها, *refresh = عدم;
	حرف *new_url;
	حرف quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	إذا (c->refresh) {
		/* refresh already delt with */
		أرجع NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	إذا (equiv == عدم) {
		أرجع NSERROR_OK;
	}

	إذا (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		أرجع NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	إذا (exc != DOM_NO_ERR) {
		أرجع NSERROR_DOM;
	}

	إذا (content == عدم) {
		أرجع NSERROR_OK;
	}

	انتها = dom_string_داده(content) + dom_string_byte_درازی(content);

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

	url = dom_string_داده(content);

	/* *LWS */
	طالما (url < انتها && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	إذا (url == انتها || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		أرجع NSERROR_OK;
	}

	پیام_داده.delay = (صحيح) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	إذا (پیام_داده.delay < 1) {
		پیام_داده.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	طالما (url < انتها && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	طالما (url < انتها && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	إذا (url < انتها && *url == ';')
		url++;

	/* *LWS */
	طالما (url < انتها && ascii_is_space(*url)) {
		url++;
	}

	إذا (url == انتها) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->پایه.refresh = nsurl_ref(content_get_url(&c->پایه));

		content_پخش(&c->پایه, CONTENT_پیام_REFRESH, &پیام_داده);

		أرجع NSERROR_OK;
	}

	/* "url" */
	إذا (url <= انتها - 3) {
		إذا (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} وإلا {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			أرجع NSERROR_OK;
		}
	} وإلا {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		أرجع NSERROR_OK;
	}

	/* *LWS */
	طالما (url < انتها && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	إذا (url < انتها) {
		إذا (*url == '=') {
			url++;
		} وإلا {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			أرجع NSERROR_OK;
		}
	} وإلا {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		أرجع NSERROR_OK;
	}

	/* *LWS */
	طالما (url < انتها && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	إذا (url < انتها && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	إذا (quote != 0) {
		/* url-sq | url-dq */
		طالما (url < انتها && *url != quote)
			url++;
	} وإلا {
		/* url-nq */
		طالما (url < انتها && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	إذا (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		إذا (new_url == عدم) {
			dom_string_unref(content);
			أرجع NSERROR_NOMEM;
		}

		error = nsurl_join(c->پایه_url, new_url, &nsurl);
		إذا (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->پایه.refresh = nsurl;

			content_پخش(&c->پایه,
					  CONTENT_پیام_REFRESH,
					  &پیام_داده);
			c->refresh = true;
		}

		حرر(new_url);

	}

	dom_string_unref(content);

	أرجع error;
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
ساكن nserror html_process_inserted_title(html_content *htmlc, dom_گره *گره)
{
	إذا (htmlc->title == عدم) {
		/* only the first title is considered */
		htmlc->title = dom_گره_ref(گره);
	}
	أرجع NSERROR_OK;
}


/** process title node */
ساكن bool html_process_title(html_content *c, dom_گره *گره)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	حرف *title_str;
	bool success;

	exc = dom_گره_get_متن_content(گره, &title);
	إذا ((exc != DOM_NO_ERR) || (title == عدم)) {
		أرجع false;
	}

	title_str = squash_whitespace(dom_string_داده(title));
	dom_string_unref(title);

	إذا (title_str == عدم) {
		أرجع false;
	}

	success = content_مجموعه_title(&c->پایه, title_str);

	حرر(title_str);

	أرجع success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ساكن فراغ html_texty_element_update(html_content *htmlc, dom_گره *گره)
{
	بنية box *box = box_for_گره(گره);
	إذا (box == عدم) {
		أرجع; /* No Box (yet?) so no gadget to update */
	}
	إذا (box->gadget == عدم) {
		أرجع; /* No gadget yet (under construction perhaps?) */
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
	dom_event_هدف *گره;
	dom_گره_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_هدف(evt, &گره);
	إذا ((exc != DOM_NO_ERR) || (گره == عدم)) {
		/* failed to obtain the event target node */
		أرجع;
	}

	exc = dom_گره_get_گره_type(گره, &type);
	إذا ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
		/* an element node has been inserted */
		dom_html_element_type برچسب_type;

		exc = dom_html_element_get_برچسب_type(گره, &برچسب_type);
		إذا (exc != DOM_NO_ERR) {
			برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		بدل (برچسب_type) {
		حالة DOM_HTML_ELEMENT_TYPE_پایه:
			html_process_inserted_پایه(htmlc, (dom_گره *)گره);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_گره *)گره);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_گره *)گره);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_گره *)گره);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_STYLE:
			إذا (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_گره *)گره);
			}
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_به_بالا(htmlc,
					     (dom_html_script_element *)گره);
			اكسر;

		حالة DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_گره *)گره);
			اكسر;

		افتراضي:
			اكسر;
		}

		إذا (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			إذا (htmlc->jsthread == عدم) {
				اتحاد content_پیام_داده پیام_داده;

				پیام_داده.jsthread = &htmlc->jsthread;
				content_پخش(&htmlc->پایه,
						  CONTENT_پیام_GETTHREAD,
						  &پیام_داده);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			إذا (htmlc->jsthread != عدم) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) گره);
			}
		}
	}
	dom_گره_unref(گره);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ساكن فراغ
dom_default_action_DOMNodeInsertedIntoDocument_cb(بنية dom_event *evt,
						  فراغ *pw)
{
	html_content *htmlc = pw;
	dom_event_هدف *گره;
	dom_گره_type type;
	dom_exception exc;

	exc = dom_event_get_هدف(evt, &گره);
	إذا ((exc == DOM_NO_ERR) && (گره != عدم)) {
		exc = dom_گره_get_گره_type(گره, &type);
		إذا ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
			/* an element node has been modified */
			dom_html_element_type برچسب_type;

			exc = dom_html_element_get_برچسب_type(گره, &برچسب_type);
			إذا (exc != DOM_NO_ERR) {
				برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			بدل (برچسب_type) {
			حالة DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_به_بالا(htmlc, (dom_html_script_element *) گره);
				fallthrough;
			افتراضي:
				اكسر;
			}
		}
		dom_گره_unref(گره);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ساكن فراغ
dom_default_action_DOMSubtreeModified_cb(بنية dom_event *evt, فراغ *pw)
{
	dom_event_هدف *گره;
	dom_گره_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_هدف(evt, &گره);
	إذا ((exc == DOM_NO_ERR) && (گره != عدم)) {
		إذا (htmlc->title == (dom_گره *)گره) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_گره *)گره);
			dom_گره_unref(گره);
			أرجع;
		}

		exc = dom_گره_get_گره_type(گره, &type);
		إذا ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
			/* an element node has been modified */
			dom_html_element_type برچسب_type;

			exc = dom_html_element_get_برچسب_type(گره, &برچسب_type);
			إذا (exc != DOM_NO_ERR) {
				برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			بدل (برچسب_type) {
			حالة DOM_HTML_ELEMENT_TYPE_STYLE:
				إذا (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_گره *)گره);
				}
				اكسر;
			حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			حالة DOM_HTML_ELEMENT_TYPE_درون_داد:
				html_texty_element_update(htmlc, (dom_گره *)گره);
				fallthrough;
			افتراضي:
				اكسر;
			}
		}
		dom_گره_unref(گره);
	}
}


/**
 * callback for default action finished
 */
ساكن فراغ
dom_default_action_پایانیافته_cb(بنية dom_event *evt, فراغ *pw)
{
	html_content *htmlc = pw;

	إذا (htmlc->jsthread != عدم)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  فراغ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_داده(type));

	إذا (phase == DOM_DEFAULT_ACTION_انتها) {
		إذا (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			أرجع dom_default_action_DOMNodeInserted_cb;
		} وإلا إذا (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			أرجع dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} وإلا إذا (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			أرجع dom_default_action_DOMSubtreeModified_cb;
		}
	} وإلا إذا (phase == DOM_DEFAULT_ACTION_پایانیافته) {
		أرجع dom_default_action_پایانیافته_cb;
	}
	أرجع عدم;
}
