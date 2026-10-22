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

#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "utils/string.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "javascript/js.h"

#அந்தர்பாவ "netsurf/bitmap.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/object.h"
#அந்தர்பாவ "html/css.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_construct.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
நிலையான bool html_process_inserted_base(html_content *htmlc, dom_முனை *முனை)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(முனை, corestring_dom_href, &atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		எனில் (error == NSERROR_OK) {
			எனில் (htmlc->base_url != வெற்று) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	எனில் (htmlc->base_target != வெற்று) {
		திரும்பு true;
	}

	exc = dom_element_get_attribute(முனை,
					corestring_dom_target,
					&atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		எனில் (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_உச்சி_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	திரும்பு true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
நிலையான bool html_process_inserted_img(html_content *htmlc, dom_முனை *முனை)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	எனில் (nsoption_bool(foreground_images) == false) {
		திரும்பு true;
	}

	exc = dom_element_get_attribute(முனை, corestring_dom_src, &src);
	எனில் (exc != DOM_NO_ERR || src == வெற்று) {
		திரும்பு true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	எனில் (err != NSERROR_OK) {
		dom_string_unref(src);
		திரும்பு false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, வெற்று, CONTENT_IMAGE, false);
	nsurl_unref(url);

	திரும்பு success;
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
நிலையான bool html_process_inserted_link(html_content *c, dom_முனை *முனை)
{
	கட்டமைப்பு content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_முனை *)முனை);

	/* try Generic link handling */

	memset(&link, 0, அளவு(கட்டமைப்பு content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(முனை, corestring_dom_rel, &atr_string);
	எனில் ((exc != DOM_NO_ERR) || (atr_string == வெற்று)) {
		திரும்பு false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(முனை, corestring_dom_href, &atr_string);
	எனில் ((exc != DOM_NO_ERR) || (atr_string == வெற்று)) {
		lwc_string_unref(link.rel);
		திரும்பு false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	எனில் (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		திரும்பு false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(முனை,
			corestring_dom_hreflang, &atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		/* get a lwc string containing the href lang */
		(வெறுமை)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(முனை,
			corestring_dom_type, &atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		/* get a lwc string containing the type */
		(வெறுமை)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(முனை,
			corestring_dom_media, &atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		/* get a lwc string containing the media */
		(வெறுமை)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(முனை,
			corestring_dom_sizes, &atr_string);
	எனில் ((exc == DOM_NO_ERR) && (atr_string != வெற்று)) {
		/* get a lwc string containing the sizes */
		(வெறுமை)dom_string_intern(atr_string, &link.sizes);
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

	திரும்பு true;
}


/* handler for a SCRIPT which has been added to a tree */
நிலையான வெறுமை
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	எனில் (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		திரும்பு;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	எனில் (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		திரும்பு;
	}

	எனில் (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		திரும்பு;
	}

	exc = dom_முனை_contains(htmlc->document, script, &within);
	எனில் (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		திரும்பு;
	}

	எனில் (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		திரும்பு;
	}

	res = html_process_script(htmlc, (dom_முனை *) script);
	எனில் (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} இல்லையெனில் {
		எனில் (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} இல்லையெனில் {
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
நிலையான nserror html_process_inserted_meta(html_content *c, dom_முனை *n)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	மாறிலி எழுத்து *url, *end, *refresh = வெற்று;
	எழுத்து *new_url;
	எழுத்து quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	எனில் (c->refresh) {
		/* refresh already delt with */
		திரும்பு NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு NSERROR_DOM;
	}

	எனில் (equiv == வெற்று) {
		திரும்பு NSERROR_OK;
	}

	எனில் (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		திரும்பு NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு NSERROR_DOM;
	}

	எனில் (content == வெற்று) {
		திரும்பு NSERROR_OK;
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
	போது (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	எனில் (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		திரும்பு NSERROR_OK;
	}

	செய்தி_data.delay = (முழு) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	எனில் (செய்தி_data.delay < 1) {
		செய்தி_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	போது (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	போது (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	எனில் (url < end && *url == ';')
		url++;

	/* *LWS */
	போது (url < end && ascii_is_space(*url)) {
		url++;
	}

	எனில் (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_செய்தி_REFRESH, &செய்தி_data);

		திரும்பு NSERROR_OK;
	}

	/* "url" */
	எனில் (url <= end - 3) {
		எனில் (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} இல்லையெனில் {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			திரும்பு NSERROR_OK;
		}
	} இல்லையெனில் {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		திரும்பு NSERROR_OK;
	}

	/* *LWS */
	போது (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	எனில் (url < end) {
		எனில் (*url == '=') {
			url++;
		} இல்லையெனில் {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			திரும்பு NSERROR_OK;
		}
	} இல்லையெனில் {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		திரும்பு NSERROR_OK;
	}

	/* *LWS */
	போது (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	எனில் (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	எனில் (quote != 0) {
		/* url-sq | url-dq */
		போது (url < end && *url != quote)
			url++;
	} இல்லையெனில் {
		/* url-nq */
		போது (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	எனில் (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		எனில் (new_url == வெற்று) {
			dom_string_unref(content);
			திரும்பு NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		எனில் (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_செய்தி_REFRESH,
					  &செய்தி_data);
			c->refresh = true;
		}

		விடுவி(new_url);

	}

	dom_string_unref(content);

	திரும்பு error;
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
நிலையான nserror html_process_inserted_title(html_content *htmlc, dom_முனை *முனை)
{
	எனில் (htmlc->title == வெற்று) {
		/* only the first title is considered */
		htmlc->title = dom_முனை_ref(முனை);
	}
	திரும்பு NSERROR_OK;
}


/** process title node */
நிலையான bool html_process_title(html_content *c, dom_முனை *முனை)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	எழுத்து *title_str;
	bool success;

	exc = dom_முனை_get_text_content(முனை, &title);
	எனில் ((exc != DOM_NO_ERR) || (title == வெற்று)) {
		திரும்பு false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	எனில் (title_str == வெற்று) {
		திரும்பு false;
	}

	success = content__set_title(&c->base, title_str);

	விடுவி(title_str);

	திரும்பு success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
நிலையான வெறுமை html_texty_element_update(html_content *htmlc, dom_முனை *முனை)
{
	கட்டமைப்பு box *box = box_for_முனை(முனை);
	எனில் (box == வெற்று) {
		திரும்பு; /* No Box (yet?) so no gadget to update */
	}
	எனில் (box->gadget == வெற்று) {
		திரும்பு; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
நிலையான வெறுமை
dom_default_action_DOMNodeInserted_cb(கட்டமைப்பு dom_event *evt, வெறுமை *pw)
{
	dom_event_target *முனை;
	dom_முனை_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &முனை);
	எனில் ((exc != DOM_NO_ERR) || (முனை == வெற்று)) {
		/* failed to obtain the event target node */
		திரும்பு;
	}

	exc = dom_முனை_get_முனை_type(முனை, &type);
	எனில் ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_முனை)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(முனை, &tag_type);
		எனில் (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		தேர்வு (tag_type) {
		நிலை DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_முனை *)முனை);
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_முனை *)முனை);
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_முனை *)முனை);
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_முனை *)முனை);
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_STYLE:
			எனில் (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_முனை *)முனை);
			}
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)முனை);
			நிறுத்து;

		நிலை DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_முனை *)முனை);
			நிறுத்து;

		இயல்பு:
			நிறுத்து;
		}

		எனில் (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			எனில் (htmlc->jsthread == வெற்று) {
				ஒன்றிப்பு content_செய்தி_data செய்தி_data;

				செய்தி_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_செய்தி_GETTHREAD,
						  &செய்தி_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			எனில் (htmlc->jsthread != வெற்று) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) முனை);
			}
		}
	}
	dom_முனை_unref(முனை);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
நிலையான வெறுமை
dom_default_action_DOMNodeInsertedIntoDocument_cb(கட்டமைப்பு dom_event *evt,
						  வெறுமை *pw)
{
	html_content *htmlc = pw;
	dom_event_target *முனை;
	dom_முனை_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &முனை);
	எனில் ((exc == DOM_NO_ERR) && (முனை != வெற்று)) {
		exc = dom_முனை_get_முனை_type(முனை, &type);
		எனில் ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_முனை)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(முனை, &tag_type);
			எனில் (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			தேர்வு (tag_type) {
			நிலை DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) முனை);
				fallthrough;
			இயல்பு:
				நிறுத்து;
			}
		}
		dom_முனை_unref(முனை);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
நிலையான வெறுமை
dom_default_action_DOMSubtreeModified_cb(கட்டமைப்பு dom_event *evt, வெறுமை *pw)
{
	dom_event_target *முனை;
	dom_முனை_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &முனை);
	எனில் ((exc == DOM_NO_ERR) && (முனை != வெற்று)) {
		எனில் (htmlc->title == (dom_முனை *)முனை) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_முனை *)முனை);
			dom_முனை_unref(முனை);
			திரும்பு;
		}

		exc = dom_முனை_get_முனை_type(முனை, &type);
		எனில் ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_முனை)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(முனை, &tag_type);
			எனில் (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			தேர்வு (tag_type) {
			நிலை DOM_HTML_ELEMENT_TYPE_STYLE:
				எனில் (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_முனை *)முனை);
				}
				நிறுத்து;
			நிலை DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			நிலை DOM_HTML_ELEMENT_TYPE_உள்ளீடு:
				html_texty_element_update(htmlc, (dom_முனை *)முனை);
				fallthrough;
			இயல்பு:
				நிறுத்து;
			}
		}
		dom_முனை_unref(முனை);
	}
}


/**
 * callback for default action finished
 */
நிலையான வெறுமை
dom_default_action_finished_cb(கட்டமைப்பு dom_event *evt, வெறுமை *pw)
{
	html_content *htmlc = pw;

	எனில் (htmlc->jsthread != வெற்று)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  வெறுமை **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	எனில் (phase == DOM_DEFAULT_ACTION_END) {
		எனில் (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			திரும்பு dom_default_action_DOMNodeInserted_cb;
		} இல்லையெனில் எனில் (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			திரும்பு dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} இல்லையெனில் எனில் (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			திரும்பு dom_default_action_DOMSubtreeModified_cb;
		}
	} இல்லையெனில் எனில் (phase == DOM_DEFAULT_ACTION_FINISHED) {
		திரும்பு dom_default_action_finished_cb;
	}
	திரும்பு வெற்று;
}
