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

#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/ascii.h"
#অন্তর্ভাৱ "utils/string.h"
#অন্তর্ভাৱ "utils/nsurl.h"
#অন্তর্ভাৱ "content/content.h"
#অন্তর্ভাৱ "javascript/js.h"

#অন্তর্ভাৱ "netsurf/bitmap.h"

#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/object.h"
#অন্তর্ভাৱ "html/css.h"
#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/box_construct.h"
#অন্তর্ভাৱ "html/form_internal.h"
#অন্তর্ভাৱ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
স্থির bool html_process_inserted_base(html_content *htmlc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_href, &atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		যদি (error == NSERROR_OK) {
			যদি (htmlc->base_url != রিক্ত) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	যদি (htmlc->base_target != রিক্ত) {
		ঘূৰাই true;
	}

	exc = dom_element_get_attribute(গ্ৰন্থি,
					corestring_dom_target,
					&atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		যদি (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_শীৰ্ষ_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ঘূৰাই true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
স্থির bool html_process_inserted_img(html_content *htmlc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	যদি (nsoption_bool(foreground_images) == false) {
		ঘূৰাই true;
	}

	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_src, &src);
	যদি (exc != DOM_NO_ERR || src == রিক্ত) {
		ঘূৰাই true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	যদি (err != NSERROR_OK) {
		dom_string_unref(src);
		ঘূৰাই false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, রিক্ত, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ঘূৰাই success;
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
স্থির bool html_process_inserted_link(html_content *c, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	সংরচনা content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_গ্ৰন্থি *)গ্ৰন্থি);

	/* try Generic link handling */

	memset(&link, 0, পরিমাণ(সংরচনা content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_rel, &atr_string);
	যদি ((exc != DOM_NO_ERR) || (atr_string == রিক্ত)) {
		ঘূৰাই false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(গ্ৰন্থি, corestring_dom_href, &atr_string);
	যদি ((exc != DOM_NO_ERR) || (atr_string == রিক্ত)) {
		lwc_string_unref(link.rel);
		ঘূৰাই false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	যদি (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ঘূৰাই false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(গ্ৰন্থি,
			corestring_dom_hreflang, &atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		/* get a lwc string containing the href lang */
		(শূন্য)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(গ্ৰন্থি,
			corestring_dom_type, &atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		/* get a lwc string containing the type */
		(শূন্য)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(গ্ৰন্থি,
			corestring_dom_media, &atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		/* get a lwc string containing the media */
		(শূন্য)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(গ্ৰন্থি,
			corestring_dom_sizes, &atr_string);
	যদি ((exc == DOM_NO_ERR) && (atr_string != রিক্ত)) {
		/* get a lwc string containing the sizes */
		(শূন্য)dom_string_intern(atr_string, &link.sizes);
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

	ঘূৰাই true;
}


/* handler for a SCRIPT which has been added to a tree */
স্থির শূন্য
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	যদি (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ঘূৰাই;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	যদি (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ঘূৰাই;
	}

	যদি (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ঘূৰাই;
	}

	exc = dom_গ্ৰন্থি_contains(htmlc->document, script, &within);
	যদি (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ঘূৰাই;
	}

	যদি (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ঘূৰাই;
	}

	res = html_process_script(htmlc, (dom_গ্ৰন্থি *) script);
	যদি (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} অন্যথা {
		যদি (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} অন্যথা {
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
স্থির nserror html_process_inserted_meta(html_content *c, dom_গ্ৰন্থি *n)
{
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	ধ্রুৱ ৱর্ণ *url, *end, *refresh = রিক্ত;
	ৱর্ণ *new_url;
	ৱর্ণ quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	যদি (c->refresh) {
		/* refresh already delt with */
		ঘূৰাই NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই NSERROR_DOM;
	}

	যদি (equiv == রিক্ত) {
		ঘূৰাই NSERROR_OK;
	}

	যদি (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ঘূৰাই NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই NSERROR_DOM;
	}

	যদি (content == রিক্ত) {
		ঘূৰাই NSERROR_OK;
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
	যেতিয়ালৈ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	যদি (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ঘূৰাই NSERROR_OK;
	}

	বাৰ্তা_data.delay = (পূর্ণাঙ্ক) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	যদি (বাৰ্তা_data.delay < 1) {
		বাৰ্তা_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	যেতিয়ালৈ (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	যেতিয়ালৈ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	যদি (url < end && *url == ';')
		url++;

	/* *LWS */
	যেতিয়ালৈ (url < end && ascii_is_space(*url)) {
		url++;
	}

	যদি (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_বাৰ্তা_REFRESH, &বাৰ্তা_data);

		ঘূৰাই NSERROR_OK;
	}

	/* "url" */
	যদি (url <= end - 3) {
		যদি (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} অন্যথা {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ঘূৰাই NSERROR_OK;
		}
	} অন্যথা {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ঘূৰাই NSERROR_OK;
	}

	/* *LWS */
	যেতিয়ালৈ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	যদি (url < end) {
		যদি (*url == '=') {
			url++;
		} অন্যথা {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ঘূৰাই NSERROR_OK;
		}
	} অন্যথা {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ঘূৰাই NSERROR_OK;
	}

	/* *LWS */
	যেতিয়ালৈ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	যদি (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	যদি (quote != 0) {
		/* url-sq | url-dq */
		যেতিয়ালৈ (url < end && *url != quote)
			url++;
	} অন্যথা {
		/* url-nq */
		যেতিয়ালৈ (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	যদি (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		যদি (new_url == রিক্ত) {
			dom_string_unref(content);
			ঘূৰাই NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		যদি (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_বাৰ্তা_REFRESH,
					  &বাৰ্তা_data);
			c->refresh = true;
		}

		মুক্ত(new_url);

	}

	dom_string_unref(content);

	ঘূৰাই error;
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
স্থির nserror html_process_inserted_title(html_content *htmlc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	যদি (htmlc->title == রিক্ত) {
		/* only the first title is considered */
		htmlc->title = dom_গ্ৰন্থি_ref(গ্ৰন্থি);
	}
	ঘূৰাই NSERROR_OK;
}


/** process title node */
স্থির bool html_process_title(html_content *c, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	ৱর্ণ *title_str;
	bool success;

	exc = dom_গ্ৰন্থি_get_text_content(গ্ৰন্থি, &title);
	যদি ((exc != DOM_NO_ERR) || (title == রিক্ত)) {
		ঘূৰাই false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	যদি (title_str == রিক্ত) {
		ঘূৰাই false;
	}

	success = content__set_title(&c->base, title_str);

	মুক্ত(title_str);

	ঘূৰাই success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
স্থির শূন্য html_texty_element_update(html_content *htmlc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	সংরচনা box *box = box_for_গ্ৰন্থি(গ্ৰন্থি);
	যদি (box == রিক্ত) {
		ঘূৰাই; /* No Box (yet?) so no gadget to update */
	}
	যদি (box->gadget == রিক্ত) {
		ঘূৰাই; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
স্থির শূন্য
dom_default_action_DOMNodeInserted_cb(সংরচনা dom_event *evt, শূন্য *pw)
{
	dom_event_target *গ্ৰন্থি;
	dom_গ্ৰন্থি_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &গ্ৰন্থি);
	যদি ((exc != DOM_NO_ERR) || (গ্ৰন্থি == রিক্ত)) {
		/* failed to obtain the event target node */
		ঘূৰাই;
	}

	exc = dom_গ্ৰন্থি_get_গ্ৰন্থি_type(গ্ৰন্থি, &type);
	যদি ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_গ্ৰন্থি)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(গ্ৰন্থি, &tag_type);
		যদি (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ৱিকল্প (tag_type) {
		পক্ষ DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_STYLE:
			যদি (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			}
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)গ্ৰন্থি);
			ভঙ্গ;

		পক্ষ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			ভঙ্গ;

		স্ৱভাৱ:
			ভঙ্গ;
		}

		যদি (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			যদি (htmlc->jsthread == রিক্ত) {
				সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;

				বাৰ্তা_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_বাৰ্তা_GETTHREAD,
						  &বাৰ্তা_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			যদি (htmlc->jsthread != রিক্ত) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) গ্ৰন্থি);
			}
		}
	}
	dom_গ্ৰন্থি_unref(গ্ৰন্থি);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
স্থির শূন্য
dom_default_action_DOMNodeInsertedIntoDocument_cb(সংরচনা dom_event *evt,
						  শূন্য *pw)
{
	html_content *htmlc = pw;
	dom_event_target *গ্ৰন্থি;
	dom_গ্ৰন্থি_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &গ্ৰন্থি);
	যদি ((exc == DOM_NO_ERR) && (গ্ৰন্থি != রিক্ত)) {
		exc = dom_গ্ৰন্থি_get_গ্ৰন্থি_type(গ্ৰন্থি, &type);
		যদি ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_গ্ৰন্থি)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(গ্ৰন্থি, &tag_type);
			যদি (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ৱিকল্প (tag_type) {
			পক্ষ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) গ্ৰন্থি);
				fallthrough;
			স্ৱভাৱ:
				ভঙ্গ;
			}
		}
		dom_গ্ৰন্থি_unref(গ্ৰন্থি);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
স্থির শূন্য
dom_default_action_DOMSubtreeModified_cb(সংরচনা dom_event *evt, শূন্য *pw)
{
	dom_event_target *গ্ৰন্থি;
	dom_গ্ৰন্থি_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &গ্ৰন্থি);
	যদি ((exc == DOM_NO_ERR) && (গ্ৰন্থি != রিক্ত)) {
		যদি (htmlc->title == (dom_গ্ৰন্থি *)গ্ৰন্থি) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
			dom_গ্ৰন্থি_unref(গ্ৰন্থি);
			ঘূৰাই;
		}

		exc = dom_গ্ৰন্থি_get_গ্ৰন্থি_type(গ্ৰন্থি, &type);
		যদি ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_গ্ৰন্থি)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(গ্ৰন্থি, &tag_type);
			যদি (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ৱিকল্প (tag_type) {
			পক্ষ DOM_HTML_ELEMENT_TYPE_STYLE:
				যদি (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_গ্ৰন্থি *)গ্ৰন্থি);
				}
				ভঙ্গ;
			পক্ষ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			পক্ষ DOM_HTML_ELEMENT_TYPE_প্ৰবেশ:
				html_texty_element_update(htmlc, (dom_গ্ৰন্থি *)গ্ৰন্থি);
				fallthrough;
			স্ৱভাৱ:
				ভঙ্গ;
			}
		}
		dom_গ্ৰন্থি_unref(গ্ৰন্থি);
	}
}


/**
 * callback for default action finished
 */
স্থির শূন্য
dom_default_action_finished_cb(সংরচনা dom_event *evt, শূন্য *pw)
{
	html_content *htmlc = pw;

	যদি (htmlc->jsthread != রিক্ত)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  শূন্য **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	যদি (phase == DOM_DEFAULT_ACTION_END) {
		যদি (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ঘূৰাই dom_default_action_DOMNodeInserted_cb;
		} অন্যথা যদি (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ঘূৰাই dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} অন্যথা যদি (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ঘূৰাই dom_default_action_DOMSubtreeModified_cb;
		}
	} অন্যথা যদি (phase == DOM_DEFAULT_ACTION_FINISHED) {
		ঘূৰাই dom_default_action_finished_cb;
	}
	ঘূৰাই রিক্ত;
}
