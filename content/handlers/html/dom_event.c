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
ಸ್ಥಿರ bool html_process_inserted_ತಳ(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_ದತ್ತಾಂಶ(atr_string), &url);
		dom_string_unref(atr_string);
		ಆದರೆ (error == NSERROR_OK) {
			ಆದರೆ (htmlc->ತಳ_url != ಖಾಲಿ) {
				nsurl_unref(htmlc->ತಳ_url);
			}
			htmlc->ತಳ_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ಆದರೆ (htmlc->ತಳ_ಗುರಿ != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ true;
	}

	exc = dom_element_get_attribute(ಗಂಟು,
					corestring_dom_ಗುರಿ,
					&atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ಆದರೆ (*dom_string_ದತ್ತಾಂಶ(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ಪೋಷಕ) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ತುದಿ_2)) {
			htmlc->ತಳ_ಗುರಿ = strdup(dom_string_ದತ್ತಾಂಶ(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ಹಿಂತಿರುಗಿ true;
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
	ಆದರೆ (nsoption_bool(foreground_images) == false) {
		ಹಿಂತಿರುಗಿ true;
	}

	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_src, &src);
	ಆದರೆ (exc != DOM_NO_ERR || src == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ true;
	}

	err = nsurl_join(htmlc->ತಳ_url, dom_string_ದತ್ತಾಂಶ(src), &url);
	ಆದರೆ (err != NSERROR_OK) {
		dom_string_unref(src);
		ಹಿಂತಿರುಗಿ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ಖಾಲಿ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ಹಿಂತಿರುಗಿ success;
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
	ರಚನೆ content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ಗಂಟು *)ಗಂಟು);

	/* try Generic link handling */

	memset(&link, 0, ಗಾತ್ರ(ರಚನೆ content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_rel, &atr_string);
	ಆದರೆ ((exc != DOM_NO_ERR) || (atr_string == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ಗಂಟು, corestring_dom_href, &atr_string);
	ಆದರೆ ((exc != DOM_NO_ERR) || (atr_string == ಖಾಲಿ)) {
		lwc_string_unref(link.rel);
		ಹಿಂತಿರುಗಿ false;
	}

	/* get nsurl */
	error = nsurl_join(c->ತಳ_url, dom_string_ದತ್ತಾಂಶ(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ಆದರೆ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ಹಿಂತಿರುಗಿ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_hreflang, &atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		/* get a lwc string containing the href lang */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_type, &atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		/* get a lwc string containing the type */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_media, &atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		/* get a lwc string containing the media */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ಗಂಟು,
			corestring_dom_sizes, &atr_string);
	ಆದರೆ ((exc == DOM_NO_ERR) && (atr_string != ಖಾಲಿ)) {
		/* get a lwc string containing the sizes */
		(ಶೂನ್ಯ)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content_ಕೂಡಿಸು_rfc5988_link(&c->ತಳ, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	ಹಿಂತಿರುಗಿ true;
}


/* handler for a SCRIPT which has been added to a tree */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_SCRIPT_showed_ಮೇಲೆ(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ಆದರೆ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ಹಿಂತಿರುಗಿ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ಆದರೆ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ಹಿಂತಿರುಗಿ;
	}

	exc = dom_ಗಂಟು_ಒಳಗೊಂಡಿದೆ(htmlc->document, script, &within);
	ಆದರೆ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ಹಿಂತಿರುಗಿ;
	}

	res = html_process_script(htmlc, (dom_ಗಂಟು *) script);
	ಆದರೆ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ಇಲ್ಲದಿದ್ದರೆ {
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
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url, *ಕೊನೆ, *refresh = ಖಾಲಿ;
	ಅಕ್ಷರ *new_url;
	ಅಕ್ಷರ quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ಆದರೆ (c->refresh) {
		/* refresh already delt with */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (equiv == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಆದರೆ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (content == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಕೊನೆ = dom_string_ದತ್ತಾಂಶ(content) + dom_string_byte_ಉದ್ದಳತೆ(content);

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

	url = dom_string_ದತ್ತಾಂಶ(content);

	/* *LWS */
	ಆಗ (url < ಕೊನೆ && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ಆದರೆ (url == ಕೊನೆ || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಸಂದೇಶ_ದತ್ತಾಂಶ.delay = (ಪೂರ್ಣಾಂಕ) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ಆದರೆ (ಸಂದೇಶ_ದತ್ತಾಂಶ.delay < 1) {
		ಸಂದೇಶ_ದತ್ತಾಂಶ.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ಆಗ (url < ಕೊನೆ && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ಆಗ (url < ಕೊನೆ && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ಆದರೆ (url < ಕೊನೆ && *url == ';')
		url++;

	/* *LWS */
	ಆಗ (url < ಕೊನೆ && ascii_is_space(*url)) {
		url++;
	}

	ಆದರೆ (url == ಕೊನೆ) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->ತಳ.refresh = nsurl_ref(content_get_url(&c->ತಳ));

		content_ಪ್ರಸಾರ(&c->ತಳ, CONTENT_ಸಂದೇಶ_REFRESH, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* "url" */
	ಆದರೆ (url <= ಕೊನೆ - 3) {
		ಆದರೆ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* *LWS */
	ಆಗ (url < ಕೊನೆ && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ಆದರೆ (url < ಕೊನೆ) {
		ಆದರೆ (*url == '=') {
			url++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* *LWS */
	ಆಗ (url < ಕೊನೆ && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ಆದರೆ (url < ಕೊನೆ && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ಆದರೆ (quote != 0) {
		/* url-sq | url-dq */
		ಆಗ (url < ಕೊನೆ && *url != quote)
			url++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* url-nq */
		ಆಗ (url < ಕೊನೆ && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ಆದರೆ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ಆದರೆ (new_url == ಖಾಲಿ) {
			dom_string_unref(content);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		error = nsurl_join(c->ತಳ_url, new_url, &nsurl);
		ಆದರೆ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->ತಳ.refresh = nsurl;

			content_ಪ್ರಸಾರ(&c->ತಳ,
					  CONTENT_ಸಂದೇಶ_REFRESH,
					  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
			c->refresh = true;
		}

		ಬಿಡುಗಡೆ(new_url);

	}

	dom_string_unref(content);

	ಹಿಂತಿರುಗಿ error;
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
	ಆದರೆ (htmlc->title == ಖಾಲಿ) {
		/* only the first title is considered */
		htmlc->title = dom_ಗಂಟು_ref(ಗಂಟು);
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/** process title node */
ಸ್ಥಿರ bool html_process_title(html_content *c, dom_ಗಂಟು *ಗಂಟು)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	ಅಕ್ಷರ *title_str;
	bool success;

	exc = dom_ಗಂಟು_get_ಪಠ್ಯ_content(ಗಂಟು, &title);
	ಆದರೆ ((exc != DOM_NO_ERR) || (title == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ false;
	}

	title_str = squash_whitespace(dom_string_ದತ್ತಾಂಶ(title));
	dom_string_unref(title);

	ಆದರೆ (title_str == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	success = content_ಗಣ_title(&c->ತಳ, title_str);

	ಬಿಡುಗಡೆ(title_str);

	ಹಿಂತಿರುಗಿ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_texty_element_update(html_content *htmlc, dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ box *box = box_for_ಗಂಟು(ಗಂಟು);
	ಆದರೆ (box == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ; /* No Box (yet?) so no gadget to update */
	}
	ಆದರೆ (box->gadget == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_DOMNodeInserted_cb(ರಚನೆ dom_event *evt, ಶೂನ್ಯ *pw)
{
	dom_event_ಗುರಿ *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ಗುರಿ(evt, &ಗಂಟು);
	ಆದರೆ ((exc != DOM_NO_ERR) || (ಗಂಟು == ಖಾಲಿ)) {
		/* failed to obtain the event target node */
		ಹಿಂತಿರುಗಿ;
	}

	exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
	ಆದರೆ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
		/* an element node has been inserted */
		dom_html_element_type ಟ್ಯಾಗ್_type;

		exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &ಟ್ಯಾಗ್_type);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ಆಯ್ಕೆ (ಟ್ಯಾಗ್_type) {
		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ತಳ:
			html_process_inserted_ತಳ(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_STYLE:
			ಆದರೆ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_ಮೇಲೆ(htmlc,
					     (dom_html_script_element *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ಆದರೆ (htmlc->jsthread == ಖಾಲಿ) {
				ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;

				ಸಂದೇಶ_ದತ್ತಾಂಶ.jsthread = &htmlc->jsthread;
				content_ಪ್ರಸಾರ(&htmlc->ತಳ,
						  CONTENT_ಸಂದೇಶ_GETTHREAD,
						  &ಸಂದೇಶ_ದತ್ತಾಂಶ);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ಆದರೆ (htmlc->jsthread != ಖಾಲಿ) {
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
dom_default_action_DOMNodeInsertedIntoDocument_cb(ರಚನೆ dom_event *evt,
						  ಶೂನ್ಯ *pw)
{
	html_content *htmlc = pw;
	dom_event_ಗುರಿ *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;

	exc = dom_event_get_ಗುರಿ(evt, &ಗಂಟು);
	ಆದರೆ ((exc == DOM_NO_ERR) && (ಗಂಟು != ಖಾಲಿ)) {
		exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
		ಆದರೆ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
			/* an element node has been modified */
			dom_html_element_type ಟ್ಯಾಗ್_type;

			exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &ಟ್ಯಾಗ್_type);
			ಆದರೆ (exc != DOM_NO_ERR) {
				ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ಆಯ್ಕೆ (ಟ್ಯಾಗ್_type) {
			ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_ಮೇಲೆ(htmlc, (dom_html_script_element *) ಗಂಟು);
				fallthrough;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
			}
		}
		dom_ಗಂಟು_unref(ಗಂಟು);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_DOMSubtreeModified_cb(ರಚನೆ dom_event *evt, ಶೂನ್ಯ *pw)
{
	dom_event_ಗುರಿ *ಗಂಟು;
	dom_ಗಂಟು_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ಗುರಿ(evt, &ಗಂಟು);
	ಆದರೆ ((exc == DOM_NO_ERR) && (ಗಂಟು != ಖಾಲಿ)) {
		ಆದರೆ (htmlc->title == (dom_ಗಂಟು *)ಗಂಟು) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
			dom_ಗಂಟು_unref(ಗಂಟು);
			ಹಿಂತಿರುಗಿ;
		}

		exc = dom_ಗಂಟು_get_ಗಂಟು_type(ಗಂಟು, &type);
		ಆದರೆ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ಗಂಟು)) {
			/* an element node has been modified */
			dom_html_element_type ಟ್ಯಾಗ್_type;

			exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &ಟ್ಯಾಗ್_type);
			ಆದರೆ (exc != DOM_NO_ERR) {
				ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ಆಯ್ಕೆ (ಟ್ಯಾಗ್_type) {
			ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_STYLE:
				ಆದರೆ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ಗಂಟು *)ಗಂಟು);
				}
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
				html_texty_element_update(htmlc, (dom_ಗಂಟು *)ಗಂಟು);
				fallthrough;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
			}
		}
		dom_ಗಂಟು_unref(ಗಂಟು);
	}
}


/**
 * callback for default action finished
 */
ಸ್ಥಿರ ಶೂನ್ಯ
dom_default_action_ಪೂರ್ಣಗೊಂಡ_cb(ರಚನೆ dom_event *evt, ಶೂನ್ಯ *pw)
{
	html_content *htmlc = pw;

	ಆದರೆ (htmlc->jsthread != ಖಾಲಿ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  ಶೂನ್ಯ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_ದತ್ತಾಂಶ(type));

	ಆದರೆ (phase == DOM_DEFAULT_ACTION_ಕೊನೆ) {
		ಆದರೆ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ಹಿಂತಿರುಗಿ dom_default_action_DOMNodeInserted_cb;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ಹಿಂತಿರುಗಿ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ಹಿಂತಿರುಗಿ dom_default_action_DOMSubtreeModified_cb;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (phase == DOM_DEFAULT_ACTION_ಪೂರ್ಣಗೊಂಡ) {
		ಹಿಂತಿರುಗಿ dom_default_action_ಪೂರ್ಣಗೊಂಡ_cb;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
