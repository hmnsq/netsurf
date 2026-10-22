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

#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "utils/string.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "javascript/js.h"

#अन्तर्भाव "netsurf/bitmap.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/object.h"
#अन्तर्भाव "html/css.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_construct.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
स्थिर bool html_process_inserted_base(html_content *htmlc, dom_गाठ *गाठ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_href, &atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		यदि (error == NSERROR_OK) {
			यदि (htmlc->base_url != रिक्त) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	यदि (htmlc->base_target != रिक्त) {
		प्रत्यागम true;
	}

	exc = dom_element_get_attribute(गाठ,
					corestring_dom_target,
					&atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		यदि (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_शीर्ष_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	प्रत्यागम true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
स्थिर bool html_process_inserted_img(html_content *htmlc, dom_गाठ *गाठ)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	यदि (nsoption_bool(foreground_images) == false) {
		प्रत्यागम true;
	}

	exc = dom_element_get_attribute(गाठ, corestring_dom_src, &src);
	यदि (exc != DOM_NO_ERR || src == रिक्त) {
		प्रत्यागम true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	यदि (err != NSERROR_OK) {
		dom_string_unref(src);
		प्रत्यागम false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, रिक्त, CONTENT_IMAGE, false);
	nsurl_unref(url);

	प्रत्यागम success;
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
स्थिर bool html_process_inserted_link(html_content *c, dom_गाठ *गाठ)
{
	संरचना content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_गाठ *)गाठ);

	/* try Generic link handling */

	memset(&link, 0, परिमाण(संरचना content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_rel, &atr_string);
	यदि ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		प्रत्यागम false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_href, &atr_string);
	यदि ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		lwc_string_unref(link.rel);
		प्रत्यागम false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	यदि (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		प्रत्यागम false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_hreflang, &atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the href lang */
		(शून्य)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_type, &atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the type */
		(शून्य)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_media, &atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the media */
		(शून्य)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_sizes, &atr_string);
	यदि ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the sizes */
		(शून्य)dom_string_intern(atr_string, &link.sizes);
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

	प्रत्यागम true;
}


/* handler for a SCRIPT which has been added to a tree */
स्थिर शून्य
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	यदि (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		प्रत्यागम;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	यदि (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		प्रत्यागम;
	}

	यदि (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		प्रत्यागम;
	}

	exc = dom_गाठ_contains(htmlc->document, script, &within);
	यदि (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		प्रत्यागम;
	}

	यदि (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		प्रत्यागम;
	}

	res = html_process_script(htmlc, (dom_गाठ *) script);
	यदि (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} अन्यथा {
		यदि (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} अन्यथा {
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
स्थिर nserror html_process_inserted_meta(html_content *c, dom_गाठ *n)
{
	सङ्घ content_संदेश_data संदेश_data;
	ध्रुव वर्ण *url, *end, *refresh = रिक्त;
	वर्ण *new_url;
	वर्ण quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	यदि (c->refresh) {
		/* refresh already delt with */
		प्रत्यागम NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम NSERROR_DOM;
	}

	यदि (equiv == रिक्त) {
		प्रत्यागम NSERROR_OK;
	}

	यदि (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		प्रत्यागम NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	यदि (exc != DOM_NO_ERR) {
		प्रत्यागम NSERROR_DOM;
	}

	यदि (content == रिक्त) {
		प्रत्यागम NSERROR_OK;
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
	यावत् (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	यदि (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		प्रत्यागम NSERROR_OK;
	}

	संदेश_data.delay = (पूर्णाङ्क) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	यदि (संदेश_data.delay < 1) {
		संदेश_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	यावत् (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	यावत् (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	यदि (url < end && *url == ';')
		url++;

	/* *LWS */
	यावत् (url < end && ascii_is_space(*url)) {
		url++;
	}

	यदि (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_संदेश_REFRESH, &संदेश_data);

		प्रत्यागम NSERROR_OK;
	}

	/* "url" */
	यदि (url <= end - 3) {
		यदि (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			प्रत्यागम NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		प्रत्यागम NSERROR_OK;
	}

	/* *LWS */
	यावत् (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	यदि (url < end) {
		यदि (*url == '=') {
			url++;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			प्रत्यागम NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		प्रत्यागम NSERROR_OK;
	}

	/* *LWS */
	यावत् (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	यदि (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	यदि (quote != 0) {
		/* url-sq | url-dq */
		यावत् (url < end && *url != quote)
			url++;
	} अन्यथा {
		/* url-nq */
		यावत् (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	यदि (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		यदि (new_url == रिक्त) {
			dom_string_unref(content);
			प्रत्यागम NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		यदि (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_संदेश_REFRESH,
					  &संदेश_data);
			c->refresh = true;
		}

		मुक्त(new_url);

	}

	dom_string_unref(content);

	प्रत्यागम error;
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
स्थिर nserror html_process_inserted_title(html_content *htmlc, dom_गाठ *गाठ)
{
	यदि (htmlc->title == रिक्त) {
		/* only the first title is considered */
		htmlc->title = dom_गाठ_ref(गाठ);
	}
	प्रत्यागम NSERROR_OK;
}


/** process title node */
स्थिर bool html_process_title(html_content *c, dom_गाठ *गाठ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	वर्ण *title_str;
	bool success;

	exc = dom_गाठ_get_text_content(गाठ, &title);
	यदि ((exc != DOM_NO_ERR) || (title == रिक्त)) {
		प्रत्यागम false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	यदि (title_str == रिक्त) {
		प्रत्यागम false;
	}

	success = content__set_title(&c->base, title_str);

	मुक्त(title_str);

	प्रत्यागम success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
स्थिर शून्य html_texty_element_update(html_content *htmlc, dom_गाठ *गाठ)
{
	संरचना box *box = box_for_गाठ(गाठ);
	यदि (box == रिक्त) {
		प्रत्यागम; /* No Box (yet?) so no gadget to update */
	}
	यदि (box->gadget == रिक्त) {
		प्रत्यागम; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
स्थिर शून्य
dom_default_action_DOMNodeInserted_cb(संरचना dom_event *evt, शून्य *pw)
{
	dom_event_target *गाठ;
	dom_गाठ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &गाठ);
	यदि ((exc != DOM_NO_ERR) || (गाठ == रिक्त)) {
		/* failed to obtain the event target node */
		प्रत्यागम;
	}

	exc = dom_गाठ_get_गाठ_type(गाठ, &type);
	यदि ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(गाठ, &tag_type);
		यदि (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		विकल्प (tag_type) {
		पक्ष DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_गाठ *)गाठ);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_गाठ *)गाठ);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_गाठ *)गाठ);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_गाठ *)गाठ);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_STYLE:
			यदि (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_गाठ *)गाठ);
			}
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)गाठ);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_गाठ *)गाठ);
			भङ्ग;

		स्वभाव:
			भङ्ग;
		}

		यदि (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			यदि (htmlc->jsthread == रिक्त) {
				सङ्घ content_संदेश_data संदेश_data;

				संदेश_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_संदेश_GETTHREAD,
						  &संदेश_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			यदि (htmlc->jsthread != रिक्त) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) गाठ);
			}
		}
	}
	dom_गाठ_unref(गाठ);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
स्थिर शून्य
dom_default_action_DOMNodeInsertedIntoDocument_cb(संरचना dom_event *evt,
						  शून्य *pw)
{
	html_content *htmlc = pw;
	dom_event_target *गाठ;
	dom_गाठ_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &गाठ);
	यदि ((exc == DOM_NO_ERR) && (गाठ != रिक्त)) {
		exc = dom_गाठ_get_गाठ_type(गाठ, &type);
		यदि ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाठ, &tag_type);
			यदि (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			विकल्प (tag_type) {
			पक्ष DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) गाठ);
				fallthrough;
			स्वभाव:
				भङ्ग;
			}
		}
		dom_गाठ_unref(गाठ);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
स्थिर शून्य
dom_default_action_DOMSubtreeModified_cb(संरचना dom_event *evt, शून्य *pw)
{
	dom_event_target *गाठ;
	dom_गाठ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &गाठ);
	यदि ((exc == DOM_NO_ERR) && (गाठ != रिक्त)) {
		यदि (htmlc->title == (dom_गाठ *)गाठ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_गाठ *)गाठ);
			dom_गाठ_unref(गाठ);
			प्रत्यागम;
		}

		exc = dom_गाठ_get_गाठ_type(गाठ, &type);
		यदि ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाठ, &tag_type);
			यदि (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			विकल्प (tag_type) {
			पक्ष DOM_HTML_ELEMENT_TYPE_STYLE:
				यदि (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_गाठ *)गाठ);
				}
				भङ्ग;
			पक्ष DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			पक्ष DOM_HTML_ELEMENT_TYPE_आदान:
				html_texty_element_update(htmlc, (dom_गाठ *)गाठ);
				fallthrough;
			स्वभाव:
				भङ्ग;
			}
		}
		dom_गाठ_unref(गाठ);
	}
}


/**
 * callback for default action finished
 */
स्थिर शून्य
dom_default_action_finished_cb(संरचना dom_event *evt, शून्य *pw)
{
	html_content *htmlc = pw;

	यदि (htmlc->jsthread != रिक्त)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  शून्य **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	यदि (phase == DOM_DEFAULT_ACTION_END) {
		यदि (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			प्रत्यागम dom_default_action_DOMNodeInserted_cb;
		} अन्यथा यदि (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			प्रत्यागम dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} अन्यथा यदि (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			प्रत्यागम dom_default_action_DOMSubtreeModified_cb;
		}
	} अन्यथा यदि (phase == DOM_DEFAULT_ACTION_FINISHED) {
		प्रत्यागम dom_default_action_finished_cb;
	}
	प्रत्यागम रिक्त;
}
