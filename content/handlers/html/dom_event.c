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

#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "javascript/js.h"

#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/css.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ಸ್ಥಿರ bool html_process_inserted_base(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		ಯದಿ (error == NSERROR_OK) {
			ಯದಿ (htmlc->base_url != ರಿಕ್ತ) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ಯದಿ (htmlc->base_target != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	exc = dom_element_get_attribute(ಗಂಟು,
					corestring_dom_target,
					&atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ಯದಿ (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ತುದಿ_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ಸ್ಥಿರ bool html_process_inserted_img(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	ಯದಿ (nsoption_bool(foreground_images) == false) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_src, &src);
	ಯದಿ (exc != DOM_NO_ERR || src == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	ಯದಿ (err != NSERROR_OK) {
		dom_string_unref(src);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ರಿಕ್ತ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ಪ್ರತ್ಯಾಗಮ success;
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
ಸ್ಥಿರ bool html_process_inserted_link(html_content *c, dom_ಗಂಟು *ಗಂಟು)
{
	ಸಂರಚನಾ content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ಗಂಟು *)ಗಂಟು);

	/* try Generic link handling */

	memset(&link, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_rel, &atr_string);
	ಯದಿ ((exc != DOM_NO_ERR) || (atr_string == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &atr_string);
	ಯದಿ ((exc != DOM_NO_ERR) || (atr_string == ರಿಕ್ತ)) {
		lwc_string_unref(link.rel);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ಯದಿ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_hreflang, &atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		/* get a lwc string containing the href lang */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_type, &atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		/* get a lwc string containing the type */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_media, &atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		/* get a lwc string containing the media */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_sizes, &atr_string);
	ಯದಿ ((exc == DOM_NO_ERR) && (atr_string != ರಿಕ್ತ)) {
		/* get a lwc string containing the sizes */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.sizes);
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

	ಪ್ರತ್ಯಾಗಮ true;
}


/* handler for a SCRIPT which has been added to a tree */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ಯದಿ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ಪ್ರತ್ಯಾಗಮ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ಯದಿ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ಪ್ರತ್ಯಾಗಮ;
	}

	exc = dom_ಗಂಟು_contains(htmlc->document, script, &within);
	ಯದಿ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ಪ್ರತ್ಯಾಗಮ;
	}

	res = html_process_script(htmlc, (dom_ಗಂಟು *) script);
	ಯದಿ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ಅನ್ಯಥಾ {
		ಯದಿ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ಅನ್ಯಥಾ {
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
ಸ್ಥಿರ nserror html_process_inserted_meta(html_content *c, dom_ಗಂಟು *n)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	ಧ್ರುವ ವರ್ಣ *url, *end, *refresh = ರಿಕ್ತ;
	ವರ್ಣ *new_url;
	ವರ್ಣ quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ಯದಿ (c->refresh) {
		/* refresh already delt with */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (equiv == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	ಯದಿ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (content == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
	ಯಾವತ್ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ಯದಿ (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	ಸಂದೇಶ_data.delay = (ಪೂರ್ಣಾಙ್ಕ) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ಯದಿ (ಸಂದೇಶ_data.delay < 1) {
		ಸಂದೇಶ_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ಯಾವತ್ (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ಯಾವತ್ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ಯದಿ (url < end && *url == ';')
		url++;

	/* *LWS */
	ಯಾವತ್ (url < end && ascii_is_space(*url)) {
		url++;
	}

	ಯದಿ (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_ಸಂದೇಶ_REFRESH, &ಸಂದೇಶ_data);

		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* "url" */
	ಯದಿ (url <= end - 3) {
		ಯದಿ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ಅನ್ಯಥಾ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
		}
	} ಅನ್ಯಥಾ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* *LWS */
	ಯಾವತ್ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ಯದಿ (url < end) {
		ಯದಿ (*url == '=') {
			url++;
		} ಅನ್ಯಥಾ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
		}
	} ಅನ್ಯಥಾ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* *LWS */
	ಯಾವತ್ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ಯದಿ (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ಯದಿ (quote != 0) {
		/* url-sq | url-dq */
		ಯಾವತ್ (url < end && *url != quote)
			url++;
	} ಅನ್ಯಥಾ {
		/* url-nq */
		ಯಾವತ್ (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ಯದಿ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ಯದಿ (new_url == ರಿಕ್ತ) {
			dom_string_unref(content);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		ಯದಿ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_ಸಂದೇಶ_REFRESH,
					  &ಸಂದೇಶ_data);
			c->refresh = true;
		}

		ಮುಕ್ತ(new_url);

	}

	dom_string_unref(content);

	ಪ್ರತ್ಯಾಗಮ error;
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
ಸ್ಥಿರ nserror html_process_inserted_title(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	ಯದಿ (htmlc->title == ರಿಕ್ತ) {
		/* only the first title is considered */
		htmlc->title = dom_ಗಂಟು_ref(ಗಂಟು);
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/** process title node */
ಸ್ಥಿರ bool html_process_title(html_content *c, dom_ಗಂಟು *ಗಂಟು)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	ವರ್ಣ *title_str;
	bool success;

	exc = dom_ಗಂಟು_get_text_content(ಗಂಟು, &title);
	ಯದಿ ((exc != DOM_NO_ERR) || (title == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	ಯದಿ (title_str == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	success = content__set_title(&c->base, title_str);

	ಮುಕ್ತ(title_str);

	ಪ್ರತ್ಯಾಗಮ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_texty_element_update(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	ಸಂರಚನಾ box *box = box_for_ಗಂಟು(ಗಂಟು);
	ಯದಿ (box == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ; /* No Box (yet?) so no gadget to update */
	}
	ಯದಿ (box->gadget == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_DOMNodeInserted_cb(ಸಂರಚನಾ dom_event *evt, ಶೂನ್ಯ *pw)
{
	dom_event_target *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ಗಂಟು);
	ಯದಿ ((exc != DOM_NO_ERR) || (ಗಂಟು == ರಿಕ್ತ)) {
		/* failed to obtain the event target node */
		ಪ್ರತ್ಯಾಗಮ;
	}

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
	ಯದಿ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(ಗಂಟು, &tag_type);
		ಯದಿ (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ವಿಕಲ್ಪ (tag_type) {
		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_STYLE:
			ಯದಿ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)ಗಂಟು);
			ಭಙ್ಗ;

		ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}

		ಯದಿ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ಯದಿ (htmlc->jsthread == ರಿಕ್ತ) {
				ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;

				ಸಂದೇಶ_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_ಸಂದೇಶ_GETTHREAD,
						  &ಸಂದೇಶ_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ಯದಿ (htmlc->jsthread != ರಿಕ್ತ) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) ಗಂಟು);
			}
		}
	}
	dom_ಗಂಟು_unref(ಗಂಟು);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_DOMNodeInsertedIntoDocument_cb(ಸಂರಚನಾ dom_event *evt,
						  ಶೂನ್ಯ *pw)
{
	html_content *htmlc = pw;
	dom_event_target *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &ಗಂಟು);
	ಯದಿ ((exc == DOM_NO_ERR) && (ಗಂಟು != ರಿಕ್ತ)) {
		exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
		ಯದಿ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ಗಂಟು, &tag_type);
			ಯದಿ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ವಿಕಲ್ಪ (tag_type) {
			ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) ಗಂಟು);
				fallthrough;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
			}
		}
		dom_ಗಂಟು_unref(ಗಂಟು);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_DOMSubtreeModified_cb(ಸಂರಚನಾ dom_event *evt, ಶೂನ್ಯ *pw)
{
	dom_event_target *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ಗಂಟು);
	ಯದಿ ((exc == DOM_NO_ERR) && (ಗಂಟು != ರಿಕ್ತ)) {
		ಯದಿ (htmlc->title == (dom_ಗಂಟು *)ಗಂಟು) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			dom_ಗಂಟು_unref(ಗಂಟು);
			ಪ್ರತ್ಯಾಗಮ;
		}

		exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
		ಯದಿ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ಗಂಟು, &tag_type);
			ಯದಿ (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ವಿಕಲ್ಪ (tag_type) {
			ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_STYLE:
				ಯದಿ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ಗಂಟು *)ಗಂಟು);
				}
				ಭಙ್ಗ;
			ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
				html_texty_element_update(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
				fallthrough;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
			}
		}
		dom_ಗಂಟು_unref(ಗಂಟು);
	}
}


/**
 * callback for default action finished
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_finished_cb(ಸಂರಚನಾ dom_event *evt, ಶೂನ್ಯ *pw)
{
	html_content *htmlc = pw;

	ಯದಿ (htmlc->jsthread != ರಿಕ್ತ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  ಶೂನ್ಯ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	ಯದಿ (phase == DOM_DEFAULT_ACTION_END) {
		ಯದಿ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ಪ್ರತ್ಯಾಗಮ dom_default_action_DOMNodeInserted_cb;
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ಪ್ರತ್ಯಾಗಮ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ಪ್ರತ್ಯಾಗಮ dom_default_action_DOMSubtreeModified_cb;
		}
	} ಅನ್ಯಥಾ ಯದಿ (phase == DOM_DEFAULT_ACTION_FINISHED) {
		ಪ್ರತ್ಯಾಗಮ dom_default_action_finished_cb;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
