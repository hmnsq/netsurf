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

#समावेश <string.h>

#समावेश "utils/config.h"
#समावेश "utils/utils.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/log.h"
#समावेश "utils/ascii.h"
#समावेश "utils/string.h"
#समावेश "utils/nsurl.h"
#समावेश "content/content.h"
#समावेश "javascript/js.h"

#समावेश "netsurf/bitmap.h"

#समावेश "html/private.h"
#समावेश "html/object.h"
#समावेश "html/css.h"
#समावेश "html/box.h"
#समावेश "html/box_construct.h"
#समावेश "html/form_internal.h"
#समावेश "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
अटल bool html_process_inserted_base(html_content *htmlc, dom_गाँठ *गाँठ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_href, &atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_तथ्य(atr_string), &url);
		dom_string_unref(atr_string);
		अगर (error == NSERROR_OK) {
			अगर (htmlc->base_url != शून्य) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	अगर (htmlc->base_target != शून्य) {
		वापस true;
	}

	exc = dom_element_get_attribute(गाँठ,
					corestring_dom_target,
					&atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		अगर (*dom_string_तथ्य(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_शीर्ष_2)) {
			htmlc->base_target = strdup(dom_string_तथ्य(atr_string));
		}
		dom_string_unref(atr_string);
	}

	वापस true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
अटल bool html_process_inserted_img(html_content *htmlc, dom_गाँठ *गाँठ)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	अगर (nsoption_bool(foreground_images) == false) {
		वापस true;
	}

	exc = dom_element_get_attribute(गाँठ, corestring_dom_src, &src);
	अगर (exc != DOM_NO_ERR || src == शून्य) {
		वापस true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_तथ्य(src), &url);
	अगर (err != NSERROR_OK) {
		dom_string_unref(src);
		वापस false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, शून्य, CONTENT_IMAGE, false);
	nsurl_unref(url);

	वापस success;
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
अटल bool html_process_inserted_link(html_content *c, dom_गाँठ *गाँठ)
{
	काष्ठा content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_गाँठ *)गाँठ);

	/* try Generic link handling */

	स_रखो(&link, 0, माप(काष्ठा content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_rel, &atr_string);
	अगर ((exc != DOM_NO_ERR) || (atr_string == शून्य)) {
		वापस false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_href, &atr_string);
	अगर ((exc != DOM_NO_ERR) || (atr_string == शून्य)) {
		lwc_string_unref(link.rel);
		वापस false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_तथ्य(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	अगर (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		वापस false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_hreflang, &atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		/* get a lwc string containing the href lang */
		(व्योम)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_type, &atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		/* get a lwc string containing the type */
		(व्योम)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_media, &atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		/* get a lwc string containing the media */
		(व्योम)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(गाँठ,
			corestring_dom_sizes, &atr_string);
	अगर ((exc == DOM_NO_ERR) && (atr_string != शून्य)) {
		/* get a lwc string containing the sizes */
		(व्योम)dom_string_intern(atr_string, &link.sizes);
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

	वापस true;
}


/* handler for a SCRIPT which has been added to a tree */
अटल व्योम
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	अगर (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		वापस;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	अगर (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		वापस;
	}

	अगर (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		वापस;
	}

	exc = dom_गाँठ_contains(htmlc->document, script, &within);
	अगर (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		वापस;
	}

	अगर (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		वापस;
	}

	res = html_process_script(htmlc, (dom_गाँठ *) script);
	अगर (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} अन्यथा {
		अगर (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
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
अटल nserror html_process_inserted_meta(html_content *c, dom_गाँठ *n)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	स्थिर अक्षर *url, *end, *refresh = शून्य;
	अक्षर *new_url;
	अक्षर quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	अगर (c->refresh) {
		/* refresh already delt with */
		वापस NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	अगर (exc != DOM_NO_ERR) {
		वापस NSERROR_DOM;
	}

	अगर (equiv == शून्य) {
		वापस NSERROR_OK;
	}

	अगर (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		वापस NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	अगर (exc != DOM_NO_ERR) {
		वापस NSERROR_DOM;
	}

	अगर (content == शून्य) {
		वापस NSERROR_OK;
	}

	end = dom_string_तथ्य(content) + dom_string_byte_लंबाई(content);

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

	url = dom_string_तथ्य(content);

	/* *LWS */
	जबतक (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	अगर (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		वापस NSERROR_OK;
	}

	संदेश_तथ्य.delay = (पूर्णांक) म_से_दीर्घ(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	अगर (संदेश_तथ्य.delay < 1) {
		संदेश_तथ्य.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	जबतक (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	जबतक (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	अगर (url < end && *url == ';')
		url++;

	/* *LWS */
	जबतक (url < end && ascii_is_space(*url)) {
		url++;
	}

	अगर (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_संदेश_REFRESH, &संदेश_तथ्य);

		वापस NSERROR_OK;
	}

	/* "url" */
	अगर (url <= end - 3) {
		अगर (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			वापस NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		वापस NSERROR_OK;
	}

	/* *LWS */
	जबतक (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	अगर (url < end) {
		अगर (*url == '=') {
			url++;
		} अन्यथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			वापस NSERROR_OK;
		}
	} अन्यथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		वापस NSERROR_OK;
	}

	/* *LWS */
	जबतक (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	अगर (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	अगर (quote != 0) {
		/* url-sq | url-dq */
		जबतक (url < end && *url != quote)
			url++;
	} अन्यथा {
		/* url-nq */
		जबतक (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	अगर (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		अगर (new_url == शून्य) {
			dom_string_unref(content);
			वापस NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		अगर (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_संदेश_REFRESH,
					  &संदेश_तथ्य);
			c->refresh = true;
		}

		मुक्त(new_url);

	}

	dom_string_unref(content);

	वापस error;
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
अटल nserror html_process_inserted_title(html_content *htmlc, dom_गाँठ *गाँठ)
{
	अगर (htmlc->title == शून्य) {
		/* only the first title is considered */
		htmlc->title = dom_गाँठ_ref(गाँठ);
	}
	वापस NSERROR_OK;
}


/** process title node */
अटल bool html_process_title(html_content *c, dom_गाँठ *गाँठ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	अक्षर *title_str;
	bool success;

	exc = dom_गाँठ_get_text_content(गाँठ, &title);
	अगर ((exc != DOM_NO_ERR) || (title == शून्य)) {
		वापस false;
	}

	title_str = squash_whitespace(dom_string_तथ्य(title));
	dom_string_unref(title);

	अगर (title_str == शून्य) {
		वापस false;
	}

	success = content__set_title(&c->base, title_str);

	मुक्त(title_str);

	वापस success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
अटल व्योम html_texty_element_update(html_content *htmlc, dom_गाँठ *गाँठ)
{
	काष्ठा box *box = box_for_गाँठ(गाँठ);
	अगर (box == शून्य) {
		वापस; /* No Box (yet?) so no gadget to update */
	}
	अगर (box->gadget == शून्य) {
		वापस; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
अटल व्योम
dom_default_action_DOMNodeInserted_cb(काष्ठा dom_event *evt, व्योम *pw)
{
	dom_event_target *गाँठ;
	dom_गाँठ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &गाँठ);
	अगर ((exc != DOM_NO_ERR) || (गाँठ == शून्य)) {
		/* failed to obtain the event target node */
		वापस;
	}

	exc = dom_गाँठ_get_गाँठ_type(गाँठ, &type);
	अगर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाँठ)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(गाँठ, &tag_type);
		अगर (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		चयन (tag_type) {
		हाल DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_गाँठ *)गाँठ);
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_गाँठ *)गाँठ);
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_गाँठ *)गाँठ);
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_गाँठ *)गाँठ);
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_STYLE:
			अगर (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_गाँठ *)गाँठ);
			}
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)गाँठ);
			अवरोध;

		हाल DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_गाँठ *)गाँठ);
			अवरोध;

		शेष:
			अवरोध;
		}

		अगर (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			अगर (htmlc->jsthread == शून्य) {
				जोड़ content_संदेश_तथ्य संदेश_तथ्य;

				संदेश_तथ्य.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_संदेश_GETTHREAD,
						  &संदेश_तथ्य);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			अगर (htmlc->jsthread != शून्य) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) गाँठ);
			}
		}
	}
	dom_गाँठ_unref(गाँठ);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
अटल व्योम
dom_default_action_DOMNodeInsertedIntoDocument_cb(काष्ठा dom_event *evt,
						  व्योम *pw)
{
	html_content *htmlc = pw;
	dom_event_target *गाँठ;
	dom_गाँठ_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &गाँठ);
	अगर ((exc == DOM_NO_ERR) && (गाँठ != शून्य)) {
		exc = dom_गाँठ_get_गाँठ_type(गाँठ, &type);
		अगर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाँठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाँठ, &tag_type);
			अगर (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			चयन (tag_type) {
			हाल DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) गाँठ);
				fallthrough;
			शेष:
				अवरोध;
			}
		}
		dom_गाँठ_unref(गाँठ);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
अटल व्योम
dom_default_action_DOMSubtreeModified_cb(काष्ठा dom_event *evt, व्योम *pw)
{
	dom_event_target *गाँठ;
	dom_गाँठ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &गाँठ);
	अगर ((exc == DOM_NO_ERR) && (गाँठ != शून्य)) {
		अगर (htmlc->title == (dom_गाँठ *)गाँठ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_गाँठ *)गाँठ);
			dom_गाँठ_unref(गाँठ);
			वापस;
		}

		exc = dom_गाँठ_get_गाँठ_type(गाँठ, &type);
		अगर ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_गाँठ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(गाँठ, &tag_type);
			अगर (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			चयन (tag_type) {
			हाल DOM_HTML_ELEMENT_TYPE_STYLE:
				अगर (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_गाँठ *)गाँठ);
				}
				अवरोध;
			हाल DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			हाल DOM_HTML_ELEMENT_TYPE_निवेश:
				html_texty_element_update(htmlc, (dom_गाँठ *)गाँठ);
				fallthrough;
			शेष:
				अवरोध;
			}
		}
		dom_गाँठ_unref(गाँठ);
	}
}


/**
 * callback for default action finished
 */
अटल व्योम
dom_default_action_finished_cb(काष्ठा dom_event *evt, व्योम *pw)
{
	html_content *htmlc = pw;

	अगर (htmlc->jsthread != शून्य)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  व्योम **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_तथ्य(type));

	अगर (phase == DOM_DEFAULT_ACTION_END) {
		अगर (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			वापस dom_default_action_DOMNodeInserted_cb;
		} अन्यथा अगर (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			वापस dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} अन्यथा अगर (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			वापस dom_default_action_DOMSubtreeModified_cb;
		}
	} अन्यथा अगर (phase == DOM_DEFAULT_ACTION_FINISHED) {
		वापस dom_default_action_finished_cb;
	}
	वापस शून्य;
}
