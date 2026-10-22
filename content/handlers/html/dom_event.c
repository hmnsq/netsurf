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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/object.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/css.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_construct.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ਸਥਿਰ bool html_process_inserted_ਅਧਾਰ(html_content *htmlc, dom_ਗੰਢ *ਗੰਢ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_href, &atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_ਅੰਕੜੇ(atr_string), &url);
		dom_string_unref(atr_string);
		ਜੇ (error == NSERROR_OK) {
			ਜੇ (htmlc->ਅਧਾਰ_url != ਨਿਰਾ) {
				nsurl_unref(htmlc->ਅਧਾਰ_url);
			}
			htmlc->ਅਧਾਰ_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ਜੇ (htmlc->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ) {
		ਵਾਪਸ true;
	}

	exc = dom_element_get_attribute(ਗੰਢ,
					corestring_dom_ਨਿਸ਼ਾਨਾ,
					&atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ਜੇ (*dom_string_ਅੰਕੜੇ(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ਮਾਪਾ) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ਸਿਖਰ_2)) {
			htmlc->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ = strdup(dom_string_ਅੰਕੜੇ(atr_string));
		}
		dom_string_unref(atr_string);
	}

	ਵਾਪਸ true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ਸਥਿਰ bool html_process_inserted_img(html_content *htmlc, dom_ਗੰਢ *ਗੰਢ)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	ਜੇ (nsoption_bool(foreground_images) == false) {
		ਵਾਪਸ true;
	}

	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_src, &src);
	ਜੇ (exc != DOM_NO_ERR || src == ਨਿਰਾ) {
		ਵਾਪਸ true;
	}

	err = nsurl_join(htmlc->ਅਧਾਰ_url, dom_string_ਅੰਕੜੇ(src), &url);
	ਜੇ (err != NSERROR_OK) {
		dom_string_unref(src);
		ਵਾਪਸ false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ਨਿਰਾ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	ਵਾਪਸ success;
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
ਸਥਿਰ bool html_process_inserted_link(html_content *c, dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ਗੰਢ *)ਗੰਢ);

	/* try Generic link handling */

	memset(&link, 0, ਆਕਾਰ(ਬਣਤਰ content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_rel, &atr_string);
	ਜੇ ((exc != DOM_NO_ERR) || (atr_string == ਨਿਰਾ)) {
		ਵਾਪਸ false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ਗੰਢ, corestring_dom_href, &atr_string);
	ਜੇ ((exc != DOM_NO_ERR) || (atr_string == ਨਿਰਾ)) {
		lwc_string_unref(link.rel);
		ਵਾਪਸ false;
	}

	/* get nsurl */
	error = nsurl_join(c->ਅਧਾਰ_url, dom_string_ਅੰਕੜੇ(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ਜੇ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		ਵਾਪਸ false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_hreflang, &atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		/* get a lwc string containing the href lang */
		(ਖਾਲੀ)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_type, &atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		/* get a lwc string containing the type */
		(ਖਾਲੀ)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_media, &atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		/* get a lwc string containing the media */
		(ਖਾਲੀ)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ਗੰਢ,
			corestring_dom_sizes, &atr_string);
	ਜੇ ((exc == DOM_NO_ERR) && (atr_string != ਨਿਰਾ)) {
		/* get a lwc string containing the sizes */
		(ਖਾਲੀ)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content_ਜੋੜੋ_rfc5988_link(&c->ਅਧਾਰ, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	ਵਾਪਸ true;
}


/* handler for a SCRIPT which has been added to a tree */
ਸਥਿਰ ਖਾਲੀ
dom_SCRIPT_showed_ਉੱਪਰ(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ਜੇ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		ਵਾਪਸ;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ਜੇ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		ਵਾਪਸ;
	}

	ਜੇ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		ਵਾਪਸ;
	}

	exc = dom_ਗੰਢ_ਸ਼ਾਮਲ_ਹੈ(htmlc->document, script, &within);
	ਜੇ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		ਵਾਪਸ;
	}

	ਜੇ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		ਵਾਪਸ;
	}

	res = html_process_script(htmlc, (dom_ਗੰਢ *) script);
	ਜੇ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} ਨਹੀਂਤਾਂ {
		ਜੇ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} ਨਹੀਂਤਾਂ {
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
ਸਥਿਰ nserror html_process_inserted_meta(html_content *c, dom_ਗੰਢ *n)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	ਅਚਲ ਅੱਖਰ *url, *ਸਿਰਾ, *refresh = ਨਿਰਾ;
	ਅੱਖਰ *new_url;
	ਅੱਖਰ quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ਜੇ (c->refresh) {
		/* refresh already delt with */
		ਵਾਪਸ NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (equiv == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	ਜੇ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		ਵਾਪਸ NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (content == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_OK;
	}

	ਸਿਰਾ = dom_string_ਅੰਕੜੇ(content) + dom_string_byte_ਲੰਮਾਈ(content);

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

	url = dom_string_ਅੰਕੜੇ(content);

	/* *LWS */
	ਜਦੋਂ (url < ਸਿਰਾ && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ਜੇ (url == ਸਿਰਾ || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		ਵਾਪਸ NSERROR_OK;
	}

	ਸੁਨੇਹਾ_ਅੰਕੜੇ.delay = (ਪੂਰਨ) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ਜੇ (ਸੁਨੇਹਾ_ਅੰਕੜੇ.delay < 1) {
		ਸੁਨੇਹਾ_ਅੰਕੜੇ.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	ਜਦੋਂ (url < ਸਿਰਾ && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	ਜਦੋਂ (url < ਸਿਰਾ && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ਜੇ (url < ਸਿਰਾ && *url == ';')
		url++;

	/* *LWS */
	ਜਦੋਂ (url < ਸਿਰਾ && ascii_is_space(*url)) {
		url++;
	}

	ਜੇ (url == ਸਿਰਾ) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->ਅਧਾਰ.refresh = nsurl_ref(content_get_url(&c->ਅਧਾਰ));

		content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_REFRESH, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

		ਵਾਪਸ NSERROR_OK;
	}

	/* "url" */
	ਜੇ (url <= ਸਿਰਾ - 3) {
		ਜੇ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} ਨਹੀਂਤਾਂ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ਵਾਪਸ NSERROR_OK;
		}
	} ਨਹੀਂਤਾਂ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ਵਾਪਸ NSERROR_OK;
	}

	/* *LWS */
	ਜਦੋਂ (url < ਸਿਰਾ && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ਜੇ (url < ਸਿਰਾ) {
		ਜੇ (*url == '=') {
			url++;
		} ਨਹੀਂਤਾਂ {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			ਵਾਪਸ NSERROR_OK;
		}
	} ਨਹੀਂਤਾਂ {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		ਵਾਪਸ NSERROR_OK;
	}

	/* *LWS */
	ਜਦੋਂ (url < ਸਿਰਾ && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ਜੇ (url < ਸਿਰਾ && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ਜੇ (quote != 0) {
		/* url-sq | url-dq */
		ਜਦੋਂ (url < ਸਿਰਾ && *url != quote)
			url++;
	} ਨਹੀਂਤਾਂ {
		/* url-nq */
		ਜਦੋਂ (url < ਸਿਰਾ && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ਜੇ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ਜੇ (new_url == ਨਿਰਾ) {
			dom_string_unref(content);
			ਵਾਪਸ NSERROR_NOMEM;
		}

		error = nsurl_join(c->ਅਧਾਰ_url, new_url, &nsurl);
		ਜੇ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->ਅਧਾਰ.refresh = nsurl;

			content_ਪ੍ਰਸਾਰਣ(&c->ਅਧਾਰ,
					  CONTENT_ਸੁਨੇਹਾ_REFRESH,
					  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
			c->refresh = true;
		}

		ਮੁਕਤ(new_url);

	}

	dom_string_unref(content);

	ਵਾਪਸ error;
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
ਸਥਿਰ nserror html_process_inserted_title(html_content *htmlc, dom_ਗੰਢ *ਗੰਢ)
{
	ਜੇ (htmlc->title == ਨਿਰਾ) {
		/* only the first title is considered */
		htmlc->title = dom_ਗੰਢ_ref(ਗੰਢ);
	}
	ਵਾਪਸ NSERROR_OK;
}


/** process title node */
ਸਥਿਰ bool html_process_title(html_content *c, dom_ਗੰਢ *ਗੰਢ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	ਅੱਖਰ *title_str;
	bool success;

	exc = dom_ਗੰਢ_get_ਪਾਠ_content(ਗੰਢ, &title);
	ਜੇ ((exc != DOM_NO_ERR) || (title == ਨਿਰਾ)) {
		ਵਾਪਸ false;
	}

	title_str = squash_whitespace(dom_string_ਅੰਕੜੇ(title));
	dom_string_unref(title);

	ਜੇ (title_str == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	success = content_ਸੈੱਟ_title(&c->ਅਧਾਰ, title_str);

	ਮੁਕਤ(title_str);

	ਵਾਪਸ success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ਸਥਿਰ ਖਾਲੀ html_texty_element_update(html_content *htmlc, dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ box *box = box_for_ਗੰਢ(ਗੰਢ);
	ਜੇ (box == ਨਿਰਾ) {
		ਵਾਪਸ; /* No Box (yet?) so no gadget to update */
	}
	ਜੇ (box->gadget == ਨਿਰਾ) {
		ਵਾਪਸ; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ਸਥਿਰ ਖਾਲੀ
dom_default_action_DOMNodeInserted_cb(ਬਣਤਰ dom_event *evt, ਖਾਲੀ *pw)
{
	dom_event_ਨਿਸ਼ਾਨਾ *ਗੰਢ;
	dom_ਗੰਢ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ਨਿਸ਼ਾਨਾ(evt, &ਗੰਢ);
	ਜੇ ((exc != DOM_NO_ERR) || (ਗੰਢ == ਨਿਰਾ)) {
		/* failed to obtain the event target node */
		ਵਾਪਸ;
	}

	exc = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &type);
	ਜੇ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ਗੰਢ)) {
		/* an element node has been inserted */
		dom_html_element_type ਟੈਗ_type;

		exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &ਟੈਗ_type);
		ਜੇ (exc != DOM_NO_ERR) {
			ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ਚੋਣ (ਟੈਗ_type) {
		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਅਧਾਰ:
			html_process_inserted_ਅਧਾਰ(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_STYLE:
			ਜੇ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			}
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_ਉੱਪਰ(htmlc,
					     (dom_html_script_element *)ਗੰਢ);
			ਰੋਕੋ;

		ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			ਰੋਕੋ;

		ਮੂਲ:
			ਰੋਕੋ;
		}

		ਜੇ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ਜੇ (htmlc->jsthread == ਨਿਰਾ) {
				ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;

				ਸੁਨੇਹਾ_ਅੰਕੜੇ.jsthread = &htmlc->jsthread;
				content_ਪ੍ਰਸਾਰਣ(&htmlc->ਅਧਾਰ,
						  CONTENT_ਸੁਨੇਹਾ_GETTHREAD,
						  &ਸੁਨੇਹਾ_ਅੰਕੜੇ);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ਜੇ (htmlc->jsthread != ਨਿਰਾ) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) ਗੰਢ);
			}
		}
	}
	dom_ਗੰਢ_unref(ਗੰਢ);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ਸਥਿਰ ਖਾਲੀ
dom_default_action_DOMNodeInsertedIntoDocument_cb(ਬਣਤਰ dom_event *evt,
						  ਖਾਲੀ *pw)
{
	html_content *htmlc = pw;
	dom_event_ਨਿਸ਼ਾਨਾ *ਗੰਢ;
	dom_ਗੰਢ_type type;
	dom_exception exc;

	exc = dom_event_get_ਨਿਸ਼ਾਨਾ(evt, &ਗੰਢ);
	ਜੇ ((exc == DOM_NO_ERR) && (ਗੰਢ != ਨਿਰਾ)) {
		exc = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &type);
		ਜੇ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ਗੰਢ)) {
			/* an element node has been modified */
			dom_html_element_type ਟੈਗ_type;

			exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &ਟੈਗ_type);
			ਜੇ (exc != DOM_NO_ERR) {
				ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ਚੋਣ (ਟੈਗ_type) {
			ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_ਉੱਪਰ(htmlc, (dom_html_script_element *) ਗੰਢ);
				fallthrough;
			ਮੂਲ:
				ਰੋਕੋ;
			}
		}
		dom_ਗੰਢ_unref(ਗੰਢ);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ਸਥਿਰ ਖਾਲੀ
dom_default_action_DOMSubtreeModified_cb(ਬਣਤਰ dom_event *evt, ਖਾਲੀ *pw)
{
	dom_event_ਨਿਸ਼ਾਨਾ *ਗੰਢ;
	dom_ਗੰਢ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_ਨਿਸ਼ਾਨਾ(evt, &ਗੰਢ);
	ਜੇ ((exc == DOM_NO_ERR) && (ਗੰਢ != ਨਿਰਾ)) {
		ਜੇ (htmlc->title == (dom_ਗੰਢ *)ਗੰਢ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
			dom_ਗੰਢ_unref(ਗੰਢ);
			ਵਾਪਸ;
		}

		exc = dom_ਗੰਢ_get_ਗੰਢ_type(ਗੰਢ, &type);
		ਜੇ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ਗੰਢ)) {
			/* an element node has been modified */
			dom_html_element_type ਟੈਗ_type;

			exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &ਟੈਗ_type);
			ਜੇ (exc != DOM_NO_ERR) {
				ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ਚੋਣ (ਟੈਗ_type) {
			ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_STYLE:
				ਜੇ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ਗੰਢ *)ਗੰਢ);
				}
				ਰੋਕੋ;
			ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਨਿਵੇਸ਼:
				html_texty_element_update(htmlc, (dom_ਗੰਢ *)ਗੰਢ);
				fallthrough;
			ਮੂਲ:
				ਰੋਕੋ;
			}
		}
		dom_ਗੰਢ_unref(ਗੰਢ);
	}
}


/**
 * callback for default action finished
 */
ਸਥਿਰ ਖਾਲੀ
dom_default_action_ਮੁਕੰਮਲ_cb(ਬਣਤਰ dom_event *evt, ਖਾਲੀ *pw)
{
	html_content *htmlc = pw;

	ਜੇ (htmlc->jsthread != ਨਿਰਾ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  ਖਾਲੀ **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_ਅੰਕੜੇ(type));

	ਜੇ (phase == DOM_DEFAULT_ACTION_ਸਿਰਾ) {
		ਜੇ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			ਵਾਪਸ dom_default_action_DOMNodeInserted_cb;
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			ਵਾਪਸ dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			ਵਾਪਸ dom_default_action_DOMSubtreeModified_cb;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (phase == DOM_DEFAULT_ACTION_ਮੁਕੰਮਲ) {
		ਵਾਪਸ dom_default_action_ਮੁਕੰਮਲ_cb;
	}
	ਵਾਪਸ ਨਿਰਾ;
}
