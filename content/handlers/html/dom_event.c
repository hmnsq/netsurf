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
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		जर (error == NSERROR_OK) {
			जर (htmlc->base_url != रिक्त) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	जर (htmlc->base_target != रिक्त) {
		परत true;
	}

	exc = dom_element_get_attribute(गाठ,
					corestring_dom_target,
					&atr_string);
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		जर (*dom_string_data(atr_string) != '_' ||
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

	परत true;
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
	जर (nsoption_bool(foreground_images) == false) {
		परत true;
	}

	exc = dom_element_get_attribute(गाठ, corestring_dom_src, &src);
	जर (exc != DOM_NO_ERR || src == रिक्त) {
		परत true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	जर (err != NSERROR_OK) {
		dom_string_unref(src);
		परत false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, रिक्त, CONTENT_IMAGE, false);
	nsurl_unref(url);

	परत success;
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

	स_रखो(&link, 0, आकार(संरचना content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_rel, &atr_string);
	जर ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		परत false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	जर (exc != DOM_NO_ERR) {
		परत false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_href, &atr_string);
	जर ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		lwc_string_unref(link.rel);
		परत false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	जर (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		परत false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_hreflang, &atr_string);
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the href lang */
		(शून्य)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_type, &atr_string);
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the type */
		(शून्य)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_media, &atr_string);
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the media */
		(शून्य)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाठ,
			corestring_dom_sizes, &atr_string);
	जर ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
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

	परत true;
}


/* handler for a SCRIPT which has been added to a tree */
स्थिर शून्य
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	जर (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		परत;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	जर (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		परत;
	}

	जर (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		परत;
	}

	exc = dom_गाठ_contains(htmlc->document, script, &within);
	जर (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		परत;
	}

	जर (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		परत;
	}

	res = html_process_script(htmlc, (dom_गाठ *) script);
	जर (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} अन्यथा {
		जर (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
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
	संघ content_संदेश_data संदेश_data;
	स्थिरांक अक्षर *url, *end, *refresh = रिक्त;
	अक्षर *new_url;
	अक्षर quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	जर (c->refresh) {
		/* refresh already delt with */
		परत NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	जर (exc != DOM_NO_ERR) {
		परत NSERROR_DOM;
	}

	जर (equiv == रिक्त) {
		परत NSERROR_OK;
	}

	जर (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		परत NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	जर (exc != DOM_NO_ERR) {
		परत NSERROR_DOM;
	}

	जर (content == रिक्त) {
		परत NSERROR_OK;
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
	पर्यंत (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	जर (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		परत NSERROR_OK;
	}

	संदेश_data.delay = (पूर्णांक) म_से_दीर्घ(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	जर (संदेश_data.delay < 1) {
		संदेश_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	पर्यंत (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	पर्यंत (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	जर (url < end && *url == ';')
		url++;

	/* *LWS */
	पर्यंत (url < end && ascii_is_space(*url)) {
		url++;
	}

	जर (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_संदेश_REFRESH, &संदेश_data);

		परत NSERROR_OK;
	}

	/* "url" */
	जर (url <= end - 3) {
		जर (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			परत NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		परत NSERROR_OK;
	}

	/* *LWS */
	पर्यंत (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	जर (url < end) {
		जर (*url == '=') {
			url++;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			परत NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		परत NSERROR_OK;
	}

	/* *LWS */
	पर्यंत (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	जर (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	जर (quote != 0) {
		/* url-sq | url-dq */
		पर्यंत (url < end && *url != quote)
			url++;
	} अन्यथा {
		/* url-nq */
		पर्यंत (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	जर (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		जर (new_url == रिक्त) {
			dom_string_unref(content);
			परत NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		जर (error == NSERROR_OK) {
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

	परत error;
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
	जर (htmlc->title == रिक्त) {
		/* only the first title is considered */
		htmlc->title = dom_गाठ_ref(गाठ);
	}
	परत NSERROR_OK;
}


/** process title node */
स्थिर bool html_process_title(html_content *c, dom_गाठ *गाठ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	अक्षर *title_str;
	bool success;

	exc = dom_गाठ_get_text_content(गाठ, &title);
	जर ((exc != DOM_NO_ERR) || (title == रिक्त)) {
		परत false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	जर (title_str == रिक्त) {
		परत false;
	}

	success = content__set_title(&c->base, title_str);

	मुक्त(title_str);

	परत success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
स्थिर शून्य html_texty_element_update(html_content *htmlc, dom_गाठ *गाठ)
{
	संरचना box *box = box_for_गाठ(गाठ);
	जर (box == रिक्त) {
		परत; /* No Box (yet?) so no gadget to update */
	}
	जर (box->gadget == रिक्त) {
		परत; /* No gadget yet (under construction perhaps?) */
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
	जर ((exc != DOM_NO_ERR) || (गाठ == रिक्त)) {
		/* failed to obtain the event target node */
		परत;
	}

	exc = dom_गाठ_get_गाठ_type(गाठ, &type);
	जर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(गाठ, &tag_type);
		जर (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		निवड (tag_type) {
		स्थिती DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_गाठ *)गाठ);
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_गाठ *)गाठ);
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_गाठ *)गाठ);
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_गाठ *)गाठ);
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_STYLE:
			जर (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_गाठ *)गाठ);
			}
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)गाठ);
			थांबा;

		स्थिती DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_गाठ *)गाठ);
			थांबा;

		पूर्वनिर्धारित:
			थांबा;
		}

		जर (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			जर (htmlc->jsthread == रिक्त) {
				संघ content_संदेश_data संदेश_data;

				संदेश_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_संदेश_GETTHREAD,
						  &संदेश_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			जर (htmlc->jsthread != रिक्त) {
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
	जर ((exc == DOM_NO_ERR) && (गाठ != रिक्त)) {
		exc = dom_गाठ_get_गाठ_type(गाठ, &type);
		जर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाठ, &tag_type);
			जर (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			निवड (tag_type) {
			स्थिती DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) गाठ);
				fallthrough;
			पूर्वनिर्धारित:
				थांबा;
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
	जर ((exc == DOM_NO_ERR) && (गाठ != रिक्त)) {
		जर (htmlc->title == (dom_गाठ *)गाठ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_गाठ *)गाठ);
			dom_गाठ_unref(गाठ);
			परत;
		}

		exc = dom_गाठ_get_गाठ_type(गाठ, &type);
		जर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाठ, &tag_type);
			जर (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			निवड (tag_type) {
			स्थिती DOM_HTML_ELEMENT_TYPE_STYLE:
				जर (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_गाठ *)गाठ);
				}
				थांबा;
			स्थिती DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			स्थिती DOM_HTML_ELEMENT_TYPE_आदान:
				html_texty_element_update(htmlc, (dom_गाठ *)गाठ);
				fallthrough;
			पूर्वनिर्धारित:
				थांबा;
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

	जर (htmlc->jsthread != रिक्त)
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

	जर (phase == DOM_DEFAULT_ACTION_END) {
		जर (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			परत dom_default_action_DOMNodeInserted_cb;
		} अन्यथा जर (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			परत dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} अन्यथा जर (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			परत dom_default_action_DOMSubtreeModified_cb;
		}
	} अन्यथा जर (phase == DOM_DEFAULT_ACTION_FINISHED) {
		परत dom_default_action_finished_cb;
	}
	परत रिक्त;
}
