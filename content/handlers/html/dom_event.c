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
स्थिर bool html_process_inserted_base(html_content *htmlc, dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_href, &atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		सचे (error == NSERROR_OK) {
			सचे (htmlc->base_url != रिक्त) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	सचे (htmlc->base_target != रिक्त) {
		पटिनिवत्त true;
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
					corestring_dom_target,
					&atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		सचे (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_शिखरम्_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	पटिनिवत्त true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
स्थिर bool html_process_inserted_img(html_content *htmlc, dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	सचे (nsoption_bool(foreground_images) == false) {
		पटिनिवत्त true;
	}

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_src, &src);
	सचे (exc != DOM_NO_ERR || src == रिक्त) {
		पटिनिवत्त true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	सचे (err != NSERROR_OK) {
		dom_string_unref(src);
		पटिनिवत्त false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, रिक्त, CONTENT_IMAGE, false);
	nsurl_unref(url);

	पटिनिवत्त success;
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
स्थिर bool html_process_inserted_link(html_content *c, dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ग्रन्थिः *)ग्रन्थिः);

	/* try Generic link handling */

	memset(&link, 0, परिमाण(संरचना content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_rel, &atr_string);
	सचे ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		पटिनिवत्त false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	सचे (exc != DOM_NO_ERR) {
		पटिनिवत्त false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_href, &atr_string);
	सचे ((exc != DOM_NO_ERR) || (atr_string == रिक्त)) {
		lwc_string_unref(link.rel);
		पटिनिवत्त false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	सचे (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		पटिनिवत्त false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_hreflang, &atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the href lang */
		(शून्य)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_type, &atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the type */
		(शून्य)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_media, &atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
		/* get a lwc string containing the media */
		(शून्य)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ग्रन्थिः,
			corestring_dom_sizes, &atr_string);
	सचे ((exc == DOM_NO_ERR) && (atr_string != रिक्त)) {
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

	पटिनिवत्त true;
}


/* handler for a SCRIPT which has been added to a tree */
स्थिर शून्य
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	सचे (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		पटिनिवत्त;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	सचे (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		पटिनिवत्त;
	}

	सचे (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		पटिनिवत्त;
	}

	exc = dom_ग्रन्थिः_contains(htmlc->document, script, &within);
	सचे (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		पटिनिवत्त;
	}

	सचे (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		पटिनिवत्त;
	}

	res = html_process_script(htmlc, (dom_ग्रन्थिः *) script);
	सचे (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} अञ्ञथा {
		सचे (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} अञ्ञथा {
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
स्थिर nserror html_process_inserted_meta(html_content *c, dom_ग्रन्थिः *n)
{
	सङ्घ content_सन्देशः_data सन्देशः_data;
	ध्रुव वर्ण *url, *end, *refresh = रिक्त;
	वर्ण *new_url;
	वर्ण quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	सचे (c->refresh) {
		/* refresh already delt with */
		पटिनिवत्त NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	सचे (exc != DOM_NO_ERR) {
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (equiv == रिक्त) {
		पटिनिवत्त NSERROR_OK;
	}

	सचे (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		पटिनिवत्त NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	सचे (exc != DOM_NO_ERR) {
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (content == रिक्त) {
		पटिनिवत्त NSERROR_OK;
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
	याव (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	सचे (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		पटिनिवत्त NSERROR_OK;
	}

	सन्देशः_data.delay = (पूर्णाङ्क) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	सचे (सन्देशः_data.delay < 1) {
		सन्देशः_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	याव (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	याव (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	सचे (url < end && *url == ';')
		url++;

	/* *LWS */
	याव (url < end && ascii_is_space(*url)) {
		url++;
	}

	सचे (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_सन्देशः_REFRESH, &सन्देशः_data);

		पटिनिवत्त NSERROR_OK;
	}

	/* "url" */
	सचे (url <= end - 3) {
		सचे (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} अञ्ञथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			पटिनिवत्त NSERROR_OK;
		}
	} अञ्ञथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		पटिनिवत्त NSERROR_OK;
	}

	/* *LWS */
	याव (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	सचे (url < end) {
		सचे (*url == '=') {
			url++;
		} अञ्ञथा {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			पटिनिवत्त NSERROR_OK;
		}
	} अञ्ञथा {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		पटिनिवत्त NSERROR_OK;
	}

	/* *LWS */
	याव (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	सचे (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	सचे (quote != 0) {
		/* url-sq | url-dq */
		याव (url < end && *url != quote)
			url++;
	} अञ्ञथा {
		/* url-nq */
		याव (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	सचे (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		सचे (new_url == रिक्त) {
			dom_string_unref(content);
			पटिनिवत्त NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		सचे (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_सन्देशः_REFRESH,
					  &सन्देशः_data);
			c->refresh = true;
		}

		मुक्त(new_url);

	}

	dom_string_unref(content);

	पटिनिवत्त error;
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
स्थिर nserror html_process_inserted_title(html_content *htmlc, dom_ग्रन्थिः *ग्रन्थिः)
{
	सचे (htmlc->title == रिक्त) {
		/* only the first title is considered */
		htmlc->title = dom_ग्रन्थिः_ref(ग्रन्थिः);
	}
	पटिनिवत्त NSERROR_OK;
}


/** process title node */
स्थिर bool html_process_title(html_content *c, dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	वर्ण *title_str;
	bool success;

	exc = dom_ग्रन्थिः_get_text_content(ग्रन्थिः, &title);
	सचे ((exc != DOM_NO_ERR) || (title == रिक्त)) {
		पटिनिवत्त false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	सचे (title_str == रिक्त) {
		पटिनिवत्त false;
	}

	success = content__set_title(&c->base, title_str);

	मुक्त(title_str);

	पटिनिवत्त success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
स्थिर शून्य html_texty_element_update(html_content *htmlc, dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना box *box = box_for_ग्रन्थिः(ग्रन्थिः);
	सचे (box == रिक्त) {
		पटिनिवत्त; /* No Box (yet?) so no gadget to update */
	}
	सचे (box->gadget == रिक्त) {
		पटिनिवत्त; /* No gadget yet (under construction perhaps?) */
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
	dom_event_target *ग्रन्थिः;
	dom_ग्रन्थिः_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ग्रन्थिः);
	सचे ((exc != DOM_NO_ERR) || (ग्रन्थिः == रिक्त)) {
		/* failed to obtain the event target node */
		पटिनिवत्त;
	}

	exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &type);
	सचे ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ग्रन्थिः)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(ग्रन्थिः, &tag_type);
		सचे (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		विकल्प (tag_type) {
		पक्ष DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_STYLE:
			सचे (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			}
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)ग्रन्थिः);
			भङ्ग;

		पक्ष DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			भङ्ग;

		स्वभाव:
			भङ्ग;
		}

		सचे (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			सचे (htmlc->jsthread == रिक्त) {
				सङ्घ content_सन्देशः_data सन्देशः_data;

				सन्देशः_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_सन्देशः_GETTHREAD,
						  &सन्देशः_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			सचे (htmlc->jsthread != रिक्त) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) ग्रन्थिः);
			}
		}
	}
	dom_ग्रन्थिः_unref(ग्रन्थिः);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
स्थिर शून्य
dom_default_action_DOMNodeInsertedIntoDocument_cb(संरचना dom_event *evt,
						  शून्य *pw)
{
	html_content *htmlc = pw;
	dom_event_target *ग्रन्थिः;
	dom_ग्रन्थिः_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &ग्रन्थिः);
	सचे ((exc == DOM_NO_ERR) && (ग्रन्थिः != रिक्त)) {
		exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &type);
		सचे ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ग्रन्थिः)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ग्रन्थिः, &tag_type);
			सचे (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			विकल्प (tag_type) {
			पक्ष DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) ग्रन्थिः);
				fallthrough;
			स्वभाव:
				भङ्ग;
			}
		}
		dom_ग्रन्थिः_unref(ग्रन्थिः);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
स्थिर शून्य
dom_default_action_DOMSubtreeModified_cb(संरचना dom_event *evt, शून्य *pw)
{
	dom_event_target *ग्रन्थिः;
	dom_ग्रन्थिः_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ग्रन्थिः);
	सचे ((exc == DOM_NO_ERR) && (ग्रन्थिः != रिक्त)) {
		सचे (htmlc->title == (dom_ग्रन्थिः *)ग्रन्थिः) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
			dom_ग्रन्थिः_unref(ग्रन्थिः);
			पटिनिवत्त;
		}

		exc = dom_ग्रन्थिः_get_ग्रन्थिः_type(ग्रन्थिः, &type);
		सचे ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ग्रन्थिः)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ग्रन्थिः, &tag_type);
			सचे (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			विकल्प (tag_type) {
			पक्ष DOM_HTML_ELEMENT_TYPE_STYLE:
				सचे (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ग्रन्थिः *)ग्रन्थिः);
				}
				भङ्ग;
			पक्ष DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			पक्ष DOM_HTML_ELEMENT_TYPE_निवेशः:
				html_texty_element_update(htmlc, (dom_ग्रन्थिः *)ग्रन्थिः);
				fallthrough;
			स्वभाव:
				भङ्ग;
			}
		}
		dom_ग्रन्थिः_unref(ग्रन्थिः);
	}
}


/**
 * callback for default action finished
 */
स्थिर शून्य
dom_default_action_finished_cb(संरचना dom_event *evt, शून्य *pw)
{
	html_content *htmlc = pw;

	सचे (htmlc->jsthread != रिक्त)
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

	सचे (phase == DOM_DEFAULT_ACTION_END) {
		सचे (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			पटिनिवत्त dom_default_action_DOMNodeInserted_cb;
		} अञ्ञथा सचे (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			पटिनिवत्त dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} अञ्ञथा सचे (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			पटिनिवत्त dom_default_action_DOMSubtreeModified_cb;
		}
	} अञ्ञथा सचे (phase == DOM_DEFAULT_ACTION_FINISHED) {
		पटिनिवत्त dom_default_action_finished_cb;
	}
	पटिनिवत्त रिक्त;
}
