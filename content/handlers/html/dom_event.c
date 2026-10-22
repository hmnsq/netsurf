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

#شامل <string.h>

#شامل "utils/config.h"
#شامل "utils/utils.h"
#شامل "utils/corestrings.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/ascii.h"
#شامل "utils/string.h"
#شامل "utils/nsurl.h"
#شامل "content/content.h"
#شامل "javascript/js.h"

#شامل "netsurf/bitmap.h"

#شامل "html/private.h"
#شامل "html/object.h"
#شامل "html/css.h"
#شامل "html/box.h"
#شامل "html/box_construct.h"
#شامل "html/form_internal.h"
#شامل "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ساکن bool html_process_inserted_بنیاد(html_content *htmlc, dom_گرہ *گرہ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(گرہ, corestring_dom_href, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_ڈیٹا(atr_string), &url);
		dom_string_unref(atr_string);
		اگر (error == NSERROR_OK) {
			اگر (htmlc->بنیاد_url != تہی) {
				nsurl_unref(htmlc->بنیاد_url);
			}
			htmlc->بنیاد_url = url;
		}
	}


	/* get target attribute if present and not already set */
	اگر (htmlc->بنیاد_ہدف != تہی) {
		واپس true;
	}

	exc = dom_element_get_attribute(گرہ,
					corestring_dom_ہدف,
					&atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		اگر (*dom_string_ڈیٹا(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_والد) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_چوٹی_2)) {
			htmlc->بنیاد_ہدف = strdup(dom_string_ڈیٹا(atr_string));
		}
		dom_string_unref(atr_string);
	}

	واپس true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ساکن bool html_process_inserted_img(html_content *htmlc, dom_گرہ *گرہ)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	اگر (nsoption_bool(foreground_images) == false) {
		واپس true;
	}

	exc = dom_element_get_attribute(گرہ, corestring_dom_src, &src);
	اگر (exc != DOM_NO_ERR || src == تہی) {
		واپس true;
	}

	err = nsurl_join(htmlc->بنیاد_url, dom_string_ڈیٹا(src), &url);
	اگر (err != NSERROR_OK) {
		dom_string_unref(src);
		واپس false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, تہی, CONTENT_IMAGE, false);
	nsurl_unref(url);

	واپس success;
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
ساکن bool html_process_inserted_link(html_content *c, dom_گرہ *گرہ)
{
	ساخت content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_گرہ *)گرہ);

	/* try Generic link handling */

	memset(&link, 0, حجم(ساخت content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گرہ, corestring_dom_rel, &atr_string);
	اگر ((exc != DOM_NO_ERR) || (atr_string == تہی)) {
		واپس false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گرہ, corestring_dom_href, &atr_string);
	اگر ((exc != DOM_NO_ERR) || (atr_string == تہی)) {
		lwc_string_unref(link.rel);
		واپس false;
	}

	/* get nsurl */
	error = nsurl_join(c->بنیاد_url, dom_string_ڈیٹا(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		واپس false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_hreflang, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		/* get a lwc string containing the href lang */
		(خالی)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_type, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		/* get a lwc string containing the type */
		(خالی)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_media, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		/* get a lwc string containing the media */
		(خالی)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گرہ,
			corestring_dom_sizes, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != تہی)) {
		/* get a lwc string containing the sizes */
		(خالی)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content_جوڑو_rfc5988_link(&c->بنیاد, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	واپس true;
}


/* handler for a SCRIPT which has been added to a tree */
ساکن خالی
dom_SCRIPT_showed_اوپر(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	اگر (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		واپس;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		واپس;
	}

	اگر (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		واپس;
	}

	exc = dom_گرہ_شامل_ہے(htmlc->document, script, &within);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		واپس;
	}

	اگر (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		واپس;
	}

	res = html_process_script(htmlc, (dom_گرہ *) script);
	اگر (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ورنہ {
		اگر (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ورنہ {
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
ساکن nserror html_process_inserted_meta(html_content *c, dom_گرہ *n)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	ثابت حرف *url, *انتہا, *refresh = تہی;
	حرف *new_url;
	حرف quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	اگر (c->refresh) {
		/* refresh already delt with */
		واپس NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	اگر (exc != DOM_NO_ERR) {
		واپس NSERROR_DOM;
	}

	اگر (equiv == تہی) {
		واپس NSERROR_OK;
	}

	اگر (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		واپس NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	اگر (exc != DOM_NO_ERR) {
		واپس NSERROR_DOM;
	}

	اگر (content == تہی) {
		واپس NSERROR_OK;
	}

	انتہا = dom_string_ڈیٹا(content) + dom_string_byte_لمبائی(content);

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

	url = dom_string_ڈیٹا(content);

	/* *LWS */
	جب (url < انتہا && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	اگر (url == انتہا || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		واپس NSERROR_OK;
	}

	پیغام_ڈیٹا.delay = (صحیح) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	اگر (پیغام_ڈیٹا.delay < 1) {
		پیغام_ڈیٹا.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	جب (url < انتہا && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	جب (url < انتہا && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	اگر (url < انتہا && *url == ';')
		url++;

	/* *LWS */
	جب (url < انتہا && ascii_is_space(*url)) {
		url++;
	}

	اگر (url == انتہا) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->بنیاد.refresh = nsurl_ref(content_get_url(&c->بنیاد));

		content_نشر(&c->بنیاد, CONTENT_پیغام_REFRESH, &پیغام_ڈیٹا);

		واپس NSERROR_OK;
	}

	/* "url" */
	اگر (url <= انتہا - 3) {
		اگر (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ورنہ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			واپس NSERROR_OK;
		}
	} ورنہ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		واپس NSERROR_OK;
	}

	/* *LWS */
	جب (url < انتہا && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	اگر (url < انتہا) {
		اگر (*url == '=') {
			url++;
		} ورنہ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			واپس NSERROR_OK;
		}
	} ورنہ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		واپس NSERROR_OK;
	}

	/* *LWS */
	جب (url < انتہا && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	اگر (url < انتہا && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	اگر (quote != 0) {
		/* url-sq | url-dq */
		جب (url < انتہا && *url != quote)
			url++;
	} ورنہ {
		/* url-nq */
		جب (url < انتہا && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	اگر (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		اگر (new_url == تہی) {
			dom_string_unref(content);
			واپس NSERROR_NOMEM;
		}

		error = nsurl_join(c->بنیاد_url, new_url, &nsurl);
		اگر (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->بنیاد.refresh = nsurl;

			content_نشر(&c->بنیاد,
					  CONTENT_پیغام_REFRESH,
					  &پیغام_ڈیٹا);
			c->refresh = true;
		}

		آزاد(new_url);

	}

	dom_string_unref(content);

	واپس error;
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
ساکن nserror html_process_inserted_title(html_content *htmlc, dom_گرہ *گرہ)
{
	اگر (htmlc->title == تہی) {
		/* only the first title is considered */
		htmlc->title = dom_گرہ_ref(گرہ);
	}
	واپس NSERROR_OK;
}


/** process title node */
ساکن bool html_process_title(html_content *c, dom_گرہ *گرہ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	حرف *title_str;
	bool success;

	exc = dom_گرہ_get_متن_content(گرہ, &title);
	اگر ((exc != DOM_NO_ERR) || (title == تہی)) {
		واپس false;
	}

	title_str = squash_whitespace(dom_string_ڈیٹا(title));
	dom_string_unref(title);

	اگر (title_str == تہی) {
		واپس false;
	}

	success = content_سیٹ_title(&c->بنیاد, title_str);

	آزاد(title_str);

	واپس success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ساکن خالی html_texty_element_update(html_content *htmlc, dom_گرہ *گرہ)
{
	ساخت box *box = box_for_گرہ(گرہ);
	اگر (box == تہی) {
		واپس; /* No Box (yet?) so no gadget to update */
	}
	اگر (box->gadget == تہی) {
		واپس; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ساکن خالی
dom_default_action_DOMNodeInserted_cb(ساخت dom_event *evt, خالی *pw)
{
	dom_event_ہدف *گرہ;
	dom_گرہ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ہدف(evt, &گرہ);
	اگر ((exc != DOM_NO_ERR) || (گرہ == تہی)) {
		/* failed to obtain the event target node */
		واپس;
	}

	exc = dom_گرہ_get_گرہ_type(گرہ, &type);
	اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گرہ)) {
		/* an element node has been inserted */
		dom_html_element_type لیبل_type;

		exc = dom_html_element_get_لیبل_type(گرہ, &لیبل_type);
		اگر (exc != DOM_NO_ERR) {
			لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		انتخاب (لیبل_type) {
		حالت DOM_HTML_ELEMENT_TYPE_بنیاد:
			html_process_inserted_بنیاد(htmlc, (dom_گرہ *)گرہ);
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_گرہ *)گرہ);
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_گرہ *)گرہ);
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_گرہ *)گرہ);
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_STYLE:
			اگر (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_گرہ *)گرہ);
			}
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_اوپر(htmlc,
					     (dom_html_script_element *)گرہ);
			توڑو;

		حالت DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_گرہ *)گرہ);
			توڑو;

		طےشدہ:
			توڑو;
		}

		اگر (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			اگر (htmlc->jsthread == تہی) {
				اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;

				پیغام_ڈیٹا.jsthread = &htmlc->jsthread;
				content_نشر(&htmlc->بنیاد,
						  CONTENT_پیغام_GETTHREAD,
						  &پیغام_ڈیٹا);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			اگر (htmlc->jsthread != تہی) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) گرہ);
			}
		}
	}
	dom_گرہ_unref(گرہ);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ساکن خالی
dom_default_action_DOMNodeInsertedIntoDocument_cb(ساخت dom_event *evt,
						  خالی *pw)
{
	html_content *htmlc = pw;
	dom_event_ہدف *گرہ;
	dom_گرہ_type type;
	dom_exception exc;

	exc = dom_event_get_ہدف(evt, &گرہ);
	اگر ((exc == DOM_NO_ERR) && (گرہ != تہی)) {
		exc = dom_گرہ_get_گرہ_type(گرہ, &type);
		اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گرہ)) {
			/* an element node has been modified */
			dom_html_element_type لیبل_type;

			exc = dom_html_element_get_لیبل_type(گرہ, &لیبل_type);
			اگر (exc != DOM_NO_ERR) {
				لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			انتخاب (لیبل_type) {
			حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_اوپر(htmlc, (dom_html_script_element *) گرہ);
				fallthrough;
			طےشدہ:
				توڑو;
			}
		}
		dom_گرہ_unref(گرہ);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ساکن خالی
dom_default_action_DOMSubtreeModified_cb(ساخت dom_event *evt, خالی *pw)
{
	dom_event_ہدف *گرہ;
	dom_گرہ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ہدف(evt, &گرہ);
	اگر ((exc == DOM_NO_ERR) && (گرہ != تہی)) {
		اگر (htmlc->title == (dom_گرہ *)گرہ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_گرہ *)گرہ);
			dom_گرہ_unref(گرہ);
			واپس;
		}

		exc = dom_گرہ_get_گرہ_type(گرہ, &type);
		اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گرہ)) {
			/* an element node has been modified */
			dom_html_element_type لیبل_type;

			exc = dom_html_element_get_لیبل_type(گرہ, &لیبل_type);
			اگر (exc != DOM_NO_ERR) {
				لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			انتخاب (لیبل_type) {
			حالت DOM_HTML_ELEMENT_TYPE_STYLE:
				اگر (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_گرہ *)گرہ);
				}
				توڑو;
			حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			حالت DOM_HTML_ELEMENT_TYPE_داخلہ:
				html_texty_element_update(htmlc, (dom_گرہ *)گرہ);
				fallthrough;
			طےشدہ:
				توڑو;
			}
		}
		dom_گرہ_unref(گرہ);
	}
}


/**
 * callback for default action finished
 */
ساکن خالی
dom_default_action_ختم_شدہ_cb(ساخت dom_event *evt, خالی *pw)
{
	html_content *htmlc = pw;

	اگر (htmlc->jsthread != تہی)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  خالی **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_ڈیٹا(type));

	اگر (phase == DOM_DEFAULT_ACTION_انتہا) {
		اگر (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			واپس dom_default_action_DOMNodeInserted_cb;
		} ورنہ اگر (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			واپس dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ورنہ اگر (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			واپس dom_default_action_DOMSubtreeModified_cb;
		}
	} ورنہ اگر (phase == DOM_DEFAULT_ACTION_ختم_شدہ) {
		واپس dom_default_action_ختم_شدہ_cb;
	}
	واپس تہی;
}
