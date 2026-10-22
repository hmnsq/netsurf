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

#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "utils/string.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "javascript/js.h"

#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/object.h"
#ଅନ୍ତର୍ଭାଵ "html/css.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_construct.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ସ୍ଥିର bool html_process_inserted_base(html_content *htmlc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_href, &atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		ଯଦି (error == NSERROR_OK) {
			ଯଦି (htmlc->base_url != ଖାଲି) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ଯଦି (htmlc->base_target != ଖାଲି) {
		ଫେରନ୍ତୁ true;
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
					corestring_dom_target,
					&atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ଯଦି (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ଶୀର୍ଷ_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ଫେରନ୍ତୁ true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ସ୍ଥିର bool html_process_inserted_img(html_content *htmlc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	ଯଦି (nsoption_bool(foreground_images) == false) {
		ଫେରନ୍ତୁ true;
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_src, &src);
	ଯଦି (exc != DOM_NO_ERR || src == ଖାଲି) {
		ଫେରନ୍ତୁ true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	ଯଦି (err != NSERROR_OK) {
		dom_string_unref(src);
		ଫେରନ୍ତୁ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ଖାଲି, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ଫେରନ୍ତୁ success;
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
ସ୍ଥିର bool html_process_inserted_link(html_content *c, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);

	/* try Generic link handling */

	memset(&link, 0, ଆକାର(ସଂରଚନା content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_rel, &atr_string);
	ଯଦି ((exc != DOM_NO_ERR) || (atr_string == ଖାଲି)) {
		ଫେରନ୍ତୁ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_href, &atr_string);
	ଯଦି ((exc != DOM_NO_ERR) || (atr_string == ଖାଲି)) {
		lwc_string_unref(link.rel);
		ଫେରନ୍ତୁ false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ଯଦି (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ଫେରନ୍ତୁ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_hreflang, &atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		/* get a lwc string containing the href lang */
		(ଶୂନ୍ୟ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_type, &atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		/* get a lwc string containing the type */
		(ଶୂନ୍ୟ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_media, &atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		/* get a lwc string containing the media */
		(ଶୂନ୍ୟ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି,
			corestring_dom_sizes, &atr_string);
	ଯଦି ((exc == DOM_NO_ERR) && (atr_string != ଖାଲି)) {
		/* get a lwc string containing the sizes */
		(ଶୂନ୍ୟ)dom_string_intern(atr_string, &link.sizes);
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

	ଫେରନ୍ତୁ true;
}


/* handler for a SCRIPT which has been added to a tree */
ସ୍ଥିର ଶୂନ୍ୟ
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ଯଦି (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ଫେରନ୍ତୁ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ଯଦି (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ଫେରନ୍ତୁ;
	}

	ଯଦି (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ଫେରନ୍ତୁ;
	}

	exc = dom_ଗଣ୍ଠି_contains(htmlc->document, script, &within);
	ଯଦି (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ଫେରନ୍ତୁ;
	}

	ଯଦି (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ଫେରନ୍ତୁ;
	}

	res = html_process_script(htmlc, (dom_ଗଣ୍ଠି *) script);
	ଯଦି (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ଅନ୍ୟଥା {
		ଯଦି (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ଅନ୍ୟଥା {
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
ସ୍ଥିର nserror html_process_inserted_meta(html_content *c, dom_ଗଣ୍ଠି *n)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	ଅଚଳ ଅକ୍ଷର *url, *end, *refresh = ଖାଲି;
	ଅକ୍ଷର *new_url;
	ଅକ୍ଷର quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ଯଦି (c->refresh) {
		/* refresh already delt with */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (equiv == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ଯଦି (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (content == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK;
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
	ଯେତେବେଳେ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ଯଦି (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	ସନ୍ଦେଶ_data.delay = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ଯଦି (ସନ୍ଦେଶ_data.delay < 1) {
		ସନ୍ଦେଶ_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ଯେତେବେଳେ (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ଯେତେବେଳେ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ଯଦି (url < end && *url == ';')
		url++;

	/* *LWS */
	ଯେତେବେଳେ (url < end && ascii_is_space(*url)) {
		url++;
	}

	ଯଦି (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_ସନ୍ଦେଶ_REFRESH, &ସନ୍ଦେଶ_data);

		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* "url" */
	ଯଦି (url <= end - 3) {
		ଯଦି (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ଅନ୍ୟଥା {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	} ଅନ୍ୟଥା {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* *LWS */
	ଯେତେବେଳେ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ଯଦି (url < end) {
		ଯଦି (*url == '=') {
			url++;
		} ଅନ୍ୟଥା {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
	} ଅନ୍ୟଥା {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* *LWS */
	ଯେତେବେଳେ (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ଯଦି (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ଯଦି (quote != 0) {
		/* url-sq | url-dq */
		ଯେତେବେଳେ (url < end && *url != quote)
			url++;
	} ଅନ୍ୟଥା {
		/* url-nq */
		ଯେତେବେଳେ (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ଯଦି (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ଯଦି (new_url == ଖାଲି) {
			dom_string_unref(content);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		ଯଦି (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_ସନ୍ଦେଶ_REFRESH,
					  &ସନ୍ଦେଶ_data);
			c->refresh = true;
		}

		ମୁକ୍ତ(new_url);

	}

	dom_string_unref(content);

	ଫେରନ୍ତୁ error;
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
ସ୍ଥିର nserror html_process_inserted_title(html_content *htmlc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ଯଦି (htmlc->title == ଖାଲି) {
		/* only the first title is considered */
		htmlc->title = dom_ଗଣ୍ଠି_ref(ଗଣ୍ଠି);
	}
	ଫେରନ୍ତୁ NSERROR_OK;
}


/** process title node */
ସ୍ଥିର bool html_process_title(html_content *c, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	ଅକ୍ଷର *title_str;
	bool success;

	exc = dom_ଗଣ୍ଠି_get_text_content(ଗଣ୍ଠି, &title);
	ଯଦି ((exc != DOM_NO_ERR) || (title == ଖାଲି)) {
		ଫେରନ୍ତୁ false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	ଯଦି (title_str == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	success = content__set_title(&c->base, title_str);

	ମୁକ୍ତ(title_str);

	ଫେରନ୍ତୁ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ସ୍ଥିର ଶୂନ୍ୟ html_texty_element_update(html_content *htmlc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା box *box = box_for_ଗଣ୍ଠି(ଗଣ୍ଠି);
	ଯଦି (box == ଖାଲି) {
		ଫେରନ୍ତୁ; /* No Box (yet?) so no gadget to update */
	}
	ଯଦି (box->gadget == ଖାଲି) {
		ଫେରନ୍ତୁ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ସ୍ଥିର ଶୂନ୍ୟ
dom_default_action_DOMNodeInserted_cb(ସଂରଚନା dom_event *evt, ଶୂନ୍ୟ *pw)
{
	dom_event_target *ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ଗଣ୍ଠି);
	ଯଦି ((exc != DOM_NO_ERR) || (ଗଣ୍ଠି == ଖାଲି)) {
		/* failed to obtain the event target node */
		ଫେରନ୍ତୁ;
	}

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &type);
	ଯଦି ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ଗଣ୍ଠି)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &tag_type);
		ଯଦି (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ବାଛ (tag_type) {
		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_STYLE:
			ଯଦି (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}

		ଯଦି (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ଯଦି (htmlc->jsthread == ଖାଲି) {
				ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;

				ସନ୍ଦେଶ_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_ସନ୍ଦେଶ_GETTHREAD,
						  &ସନ୍ଦେଶ_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ଯଦି (htmlc->jsthread != ଖାଲି) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) ଗଣ୍ଠି);
			}
		}
	}
	dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ସ୍ଥିର ଶୂନ୍ୟ
dom_default_action_DOMNodeInsertedIntoDocument_cb(ସଂରଚନା dom_event *evt,
						  ଶୂନ୍ୟ *pw)
{
	html_content *htmlc = pw;
	dom_event_target *ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &ଗଣ୍ଠି);
	ଯଦି ((exc == DOM_NO_ERR) && (ଗଣ୍ଠି != ଖାଲି)) {
		exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &type);
		ଯଦି ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ଗଣ୍ଠି)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &tag_type);
			ଯଦି (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ବାଛ (tag_type) {
			ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) ଗଣ୍ଠି);
				fallthrough;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
			}
		}
		dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ସ୍ଥିର ଶୂନ୍ୟ
dom_default_action_DOMSubtreeModified_cb(ସଂରଚନା dom_event *evt, ଶୂନ୍ୟ *pw)
{
	dom_event_target *ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ଗଣ୍ଠି);
	ଯଦି ((exc == DOM_NO_ERR) && (ଗଣ୍ଠି != ଖାଲି)) {
		ଯଦି (htmlc->title == (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
			dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
			ଫେରନ୍ତୁ;
		}

		exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(ଗଣ୍ଠି, &type);
		ଯଦି ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ଗଣ୍ଠି)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &tag_type);
			ଯଦି (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ବାଛ (tag_type) {
			ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_STYLE:
				ଯଦି (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
				}
				ଭାଙ୍ଗ;
			ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ନିବେଶ:
				html_texty_element_update(htmlc, (dom_ଗଣ୍ଠି *)ଗଣ୍ଠି);
				fallthrough;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
			}
		}
		dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
	}
}


/**
 * callback for default action finished
 */
ସ୍ଥିର ଶୂନ୍ୟ
dom_default_action_finished_cb(ସଂରଚନା dom_event *evt, ଶୂନ୍ୟ *pw)
{
	html_content *htmlc = pw;

	ଯଦି (htmlc->jsthread != ଖାଲି)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  ଶୂନ୍ୟ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	ଯଦି (phase == DOM_DEFAULT_ACTION_END) {
		ଯଦି (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ଫେରନ୍ତୁ dom_default_action_DOMNodeInserted_cb;
		} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ଫେରନ୍ତୁ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ଫେରନ୍ତୁ dom_default_action_DOMSubtreeModified_cb;
		}
	} ଅନ୍ୟଥା ଯଦି (phase == DOM_DEFAULT_ACTION_FINISHED) {
		ଫେରନ୍ତୁ dom_default_action_finished_cb;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
