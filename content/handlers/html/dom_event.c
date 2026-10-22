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

#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "utils/string.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "javascript/js.h"

#അന്തര്ഭാവ "netsurf/bitmap.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/object.h"
#അന്തര്ഭാവ "html/css.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_construct.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
സ്ഥിതം bool html_process_inserted_base(html_content *htmlc, dom_കണ്ണി *കണ്ണി)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_href, &atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		എങ്കിൽ (error == NSERROR_OK) {
			എങ്കിൽ (htmlc->base_url != ശൂന്യ) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	എങ്കിൽ (htmlc->base_target != ശൂന്യ) {
		തിരികെ true;
	}

	exc = dom_element_get_attribute(കണ്ണി,
					corestring_dom_target,
					&atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		എങ്കിൽ (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_മുകൾ_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	തിരികെ true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
സ്ഥിതം bool html_process_inserted_img(html_content *htmlc, dom_കണ്ണി *കണ്ണി)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	എങ്കിൽ (nsoption_bool(foreground_images) == false) {
		തിരികെ true;
	}

	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_src, &src);
	എങ്കിൽ (exc != DOM_NO_ERR || src == ശൂന്യ) {
		തിരികെ true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	എങ്കിൽ (err != NSERROR_OK) {
		dom_string_unref(src);
		തിരികെ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ശൂന്യ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	തിരികെ success;
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
സ്ഥിതം bool html_process_inserted_link(html_content *c, dom_കണ്ണി *കണ്ണി)
{
	ഘടന content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_കണ്ണി *)കണ്ണി);

	/* try Generic link handling */

	memset(&link, 0, വലുപ്പം(ഘടന content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_rel, &atr_string);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (atr_string == ശൂന്യ)) {
		തിരികെ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(കണ്ണി, corestring_dom_href, &atr_string);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (atr_string == ശൂന്യ)) {
		lwc_string_unref(link.rel);
		തിരികെ false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	എങ്കിൽ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		തിരികെ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(കണ്ണി,
			corestring_dom_hreflang, &atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		/* get a lwc string containing the href lang */
		(ശൂന്യം)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(കണ്ണി,
			corestring_dom_type, &atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		/* get a lwc string containing the type */
		(ശൂന്യം)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(കണ്ണി,
			corestring_dom_media, &atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		/* get a lwc string containing the media */
		(ശൂന്യം)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(കണ്ണി,
			corestring_dom_sizes, &atr_string);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (atr_string != ശൂന്യ)) {
		/* get a lwc string containing the sizes */
		(ശൂന്യം)dom_string_intern(atr_string, &link.sizes);
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

	തിരികെ true;
}


/* handler for a SCRIPT which has been added to a tree */
സ്ഥിതം ശൂന്യം
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	എങ്കിൽ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		തിരികെ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		തിരികെ;
	}

	എങ്കിൽ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		തിരികെ;
	}

	exc = dom_കണ്ണി_contains(htmlc->document, script, &within);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		തിരികെ;
	}

	എങ്കിൽ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		തിരികെ;
	}

	res = html_process_script(htmlc, (dom_കണ്ണി *) script);
	എങ്കിൽ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} അല്ലെങ്കിൽ {
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
സ്ഥിതം nserror html_process_inserted_meta(html_content *c, dom_കണ്ണി *n)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	സ്ഥിരം അക്ഷരം *url, *end, *refresh = ശൂന്യ;
	അക്ഷരം *new_url;
	അക്ഷരം quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	എങ്കിൽ (c->refresh) {
		/* refresh already delt with */
		തിരികെ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (equiv == ശൂന്യ) {
		തിരികെ NSERROR_OK;
	}

	എങ്കിൽ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		തിരികെ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (content == ശൂന്യ) {
		തിരികെ NSERROR_OK;
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
	ആകുമ്പോൾ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	എങ്കിൽ (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		തിരികെ NSERROR_OK;
	}

	സന്ദേശം_data.delay = (പൂർണ്ണസംഖ്യ) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	എങ്കിൽ (സന്ദേശം_data.delay < 1) {
		സന്ദേശം_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ആകുമ്പോൾ (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ആകുമ്പോൾ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	എങ്കിൽ (url < end && *url == ';')
		url++;

	/* *LWS */
	ആകുമ്പോൾ (url < end && ascii_is_space(*url)) {
		url++;
	}

	എങ്കിൽ (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_സന്ദേശം_REFRESH, &സന്ദേശം_data);

		തിരികെ NSERROR_OK;
	}

	/* "url" */
	എങ്കിൽ (url <= end - 3) {
		എങ്കിൽ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} അല്ലെങ്കിൽ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			തിരികെ NSERROR_OK;
		}
	} അല്ലെങ്കിൽ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		തിരികെ NSERROR_OK;
	}

	/* *LWS */
	ആകുമ്പോൾ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	എങ്കിൽ (url < end) {
		എങ്കിൽ (*url == '=') {
			url++;
		} അല്ലെങ്കിൽ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			തിരികെ NSERROR_OK;
		}
	} അല്ലെങ്കിൽ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		തിരികെ NSERROR_OK;
	}

	/* *LWS */
	ആകുമ്പോൾ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	എങ്കിൽ (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	എങ്കിൽ (quote != 0) {
		/* url-sq | url-dq */
		ആകുമ്പോൾ (url < end && *url != quote)
			url++;
	} അല്ലെങ്കിൽ {
		/* url-nq */
		ആകുമ്പോൾ (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	എങ്കിൽ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		എങ്കിൽ (new_url == ശൂന്യ) {
			dom_string_unref(content);
			തിരികെ NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		എങ്കിൽ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_സന്ദേശം_REFRESH,
					  &സന്ദേശം_data);
			c->refresh = true;
		}

		സ്വതന്ത്രം(new_url);

	}

	dom_string_unref(content);

	തിരികെ error;
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
സ്ഥിതം nserror html_process_inserted_title(html_content *htmlc, dom_കണ്ണി *കണ്ണി)
{
	എങ്കിൽ (htmlc->title == ശൂന്യ) {
		/* only the first title is considered */
		htmlc->title = dom_കണ്ണി_ref(കണ്ണി);
	}
	തിരികെ NSERROR_OK;
}


/** process title node */
സ്ഥിതം bool html_process_title(html_content *c, dom_കണ്ണി *കണ്ണി)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	അക്ഷരം *title_str;
	bool success;

	exc = dom_കണ്ണി_get_text_content(കണ്ണി, &title);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (title == ശൂന്യ)) {
		തിരികെ false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	എങ്കിൽ (title_str == ശൂന്യ) {
		തിരികെ false;
	}

	success = content__set_title(&c->base, title_str);

	സ്വതന്ത്രം(title_str);

	തിരികെ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
സ്ഥിതം ശൂന്യം html_texty_element_update(html_content *htmlc, dom_കണ്ണി *കണ്ണി)
{
	ഘടന box *box = box_for_കണ്ണി(കണ്ണി);
	എങ്കിൽ (box == ശൂന്യ) {
		തിരികെ; /* No Box (yet?) so no gadget to update */
	}
	എങ്കിൽ (box->gadget == ശൂന്യ) {
		തിരികെ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
സ്ഥിതം ശൂന്യം
dom_default_action_DOMNodeInserted_cb(ഘടന dom_event *evt, ശൂന്യം *pw)
{
	dom_event_target *കണ്ണി;
	dom_കണ്ണി_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &കണ്ണി);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (കണ്ണി == ശൂന്യ)) {
		/* failed to obtain the event target node */
		തിരികെ;
	}

	exc = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &type);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_കണ്ണി)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(കണ്ണി, &tag_type);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		തിരഞ്ഞെടുപ്പ് (tag_type) {
		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_കണ്ണി *)കണ്ണി);
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_കണ്ണി *)കണ്ണി);
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_കണ്ണി *)കണ്ണി);
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_കണ്ണി *)കണ്ണി);
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_STYLE:
			എങ്കിൽ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_കണ്ണി *)കണ്ണി);
			}
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)കണ്ണി);
			നിർത്തുക;

		അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_കണ്ണി *)കണ്ണി);
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}

		എങ്കിൽ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			എങ്കിൽ (htmlc->jsthread == ശൂന്യ) {
				സംഘം content_സന്ദേശം_data സന്ദേശം_data;

				സന്ദേശം_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_സന്ദേശം_GETTHREAD,
						  &സന്ദേശം_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			എങ്കിൽ (htmlc->jsthread != ശൂന്യ) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) കണ്ണി);
			}
		}
	}
	dom_കണ്ണി_unref(കണ്ണി);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
സ്ഥിതം ശൂന്യം
dom_default_action_DOMNodeInsertedIntoDocument_cb(ഘടന dom_event *evt,
						  ശൂന്യം *pw)
{
	html_content *htmlc = pw;
	dom_event_target *കണ്ണി;
	dom_കണ്ണി_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &കണ്ണി);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (കണ്ണി != ശൂന്യ)) {
		exc = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &type);
		എങ്കിൽ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_കണ്ണി)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(കണ്ണി, &tag_type);
			എങ്കിൽ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			തിരഞ്ഞെടുപ്പ് (tag_type) {
			അവസ്ഥ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) കണ്ണി);
				fallthrough;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
			}
		}
		dom_കണ്ണി_unref(കണ്ണി);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
സ്ഥിതം ശൂന്യം
dom_default_action_DOMSubtreeModified_cb(ഘടന dom_event *evt, ശൂന്യം *pw)
{
	dom_event_target *കണ്ണി;
	dom_കണ്ണി_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &കണ്ണി);
	എങ്കിൽ ((exc == DOM_NO_ERR) && (കണ്ണി != ശൂന്യ)) {
		എങ്കിൽ (htmlc->title == (dom_കണ്ണി *)കണ്ണി) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_കണ്ണി *)കണ്ണി);
			dom_കണ്ണി_unref(കണ്ണി);
			തിരികെ;
		}

		exc = dom_കണ്ണി_get_കണ്ണി_type(കണ്ണി, &type);
		എങ്കിൽ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_കണ്ണി)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(കണ്ണി, &tag_type);
			എങ്കിൽ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			തിരഞ്ഞെടുപ്പ് (tag_type) {
			അവസ്ഥ DOM_HTML_ELEMENT_TYPE_STYLE:
				എങ്കിൽ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_കണ്ണി *)കണ്ണി);
				}
				നിർത്തുക;
			അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			അവസ്ഥ DOM_HTML_ELEMENT_TYPE_നിവേശം:
				html_texty_element_update(htmlc, (dom_കണ്ണി *)കണ്ണി);
				fallthrough;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
			}
		}
		dom_കണ്ണി_unref(കണ്ണി);
	}
}


/**
 * callback for default action finished
 */
സ്ഥിതം ശൂന്യം
dom_default_action_finished_cb(ഘടന dom_event *evt, ശൂന്യം *pw)
{
	html_content *htmlc = pw;

	എങ്കിൽ (htmlc->jsthread != ശൂന്യ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  ശൂന്യം **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	എങ്കിൽ (phase == DOM_DEFAULT_ACTION_END) {
		എങ്കിൽ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			തിരികെ dom_default_action_DOMNodeInserted_cb;
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			തിരികെ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			തിരികെ dom_default_action_DOMSubtreeModified_cb;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (phase == DOM_DEFAULT_ACTION_FINISHED) {
		തിരികെ dom_default_action_finished_cb;
	}
	തിരികെ ശൂന്യ;
}
