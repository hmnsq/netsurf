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

#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "utils/string.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "javascript/js.h"

#અન્તર્ભાવ "netsurf/bitmap.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/object.h"
#અન્તર્ભાવ "html/css.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_construct.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
સ્થિર bool html_process_inserted_base(html_content *htmlc, dom_ગાંઠ *ગાંઠ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_href, &atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		જો (error == NSERROR_OK) {
			જો (htmlc->base_url != ખાલી) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	જો (htmlc->base_target != ખાલી) {
		પાછા true;
	}

	exc = dom_element_get_attribute(ગાંઠ,
					corestring_dom_target,
					&atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		જો (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_ટોચ_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	પાછા true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
સ્થિર bool html_process_inserted_img(html_content *htmlc, dom_ગાંઠ *ગાંઠ)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	જો (nsoption_bool(foreground_images) == false) {
		પાછા true;
	}

	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_src, &src);
	જો (exc != DOM_NO_ERR || src == ખાલી) {
		પાછા true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	જો (err != NSERROR_OK) {
		dom_string_unref(src);
		પાછા false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ખાલી, CONTENT_IMAGE, false);
	nsurl_unref(url);

	પાછા success;
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
સ્થિર bool html_process_inserted_link(html_content *c, dom_ગાંઠ *ગાંઠ)
{
	સંરચના content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_ગાંઠ *)ગાંઠ);

	/* try Generic link handling */

	memset(&link, 0, કદ(સંરચના content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_rel, &atr_string);
	જો ((exc != DOM_NO_ERR) || (atr_string == ખાલી)) {
		પાછા false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(ગાંઠ, corestring_dom_href, &atr_string);
	જો ((exc != DOM_NO_ERR) || (atr_string == ખાલી)) {
		lwc_string_unref(link.rel);
		પાછા false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	જો (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		પાછા false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(ગાંઠ,
			corestring_dom_hreflang, &atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		/* get a lwc string containing the href lang */
		(શૂન્ય)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ગાંઠ,
			corestring_dom_type, &atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		/* get a lwc string containing the type */
		(શૂન્ય)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ગાંઠ,
			corestring_dom_media, &atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		/* get a lwc string containing the media */
		(શૂન્ય)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(ગાંઠ,
			corestring_dom_sizes, &atr_string);
	જો ((exc == DOM_NO_ERR) && (atr_string != ખાલી)) {
		/* get a lwc string containing the sizes */
		(શૂન્ય)dom_string_intern(atr_string, &link.sizes);
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

	પાછા true;
}


/* handler for a SCRIPT which has been added to a tree */
સ્થિર શૂન્ય
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	જો (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		પાછા;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	જો (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		પાછા;
	}

	જો (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		પાછા;
	}

	exc = dom_ગાંઠ_contains(htmlc->document, script, &within);
	જો (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		પાછા;
	}

	જો (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		પાછા;
	}

	res = html_process_script(htmlc, (dom_ગાંઠ *) script);
	જો (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} નહીંતર {
		જો (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} નહીંતર {
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
સ્થિર nserror html_process_inserted_meta(html_content *c, dom_ગાંઠ *n)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	અચલ અક્ષર *url, *end, *refresh = ખાલી;
	અક્ષર *new_url;
	અક્ષર quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	જો (c->refresh) {
		/* refresh already delt with */
		પાછા NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	જો (exc != DOM_NO_ERR) {
		પાછા NSERROR_DOM;
	}

	જો (equiv == ખાલી) {
		પાછા NSERROR_OK;
	}

	જો (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		પાછા NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	જો (exc != DOM_NO_ERR) {
		પાછા NSERROR_DOM;
	}

	જો (content == ખાલી) {
		પાછા NSERROR_OK;
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
	જ્યારે (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	જો (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		પાછા NSERROR_OK;
	}

	સંદેશ_data.delay = (પૂર્ણાંક) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	જો (સંદેશ_data.delay < 1) {
		સંદેશ_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	જ્યારે (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	જ્યારે (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	જો (url < end && *url == ';')
		url++;

	/* *LWS */
	જ્યારે (url < end && ascii_is_space(*url)) {
		url++;
	}

	જો (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_સંદેશ_REFRESH, &સંદેશ_data);

		પાછા NSERROR_OK;
	}

	/* "url" */
	જો (url <= end - 3) {
		જો (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} નહીંતર {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			પાછા NSERROR_OK;
		}
	} નહીંતર {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		પાછા NSERROR_OK;
	}

	/* *LWS */
	જ્યારે (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	જો (url < end) {
		જો (*url == '=') {
			url++;
		} નહીંતર {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			પાછા NSERROR_OK;
		}
	} નહીંતર {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		પાછા NSERROR_OK;
	}

	/* *LWS */
	જ્યારે (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	જો (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	જો (quote != 0) {
		/* url-sq | url-dq */
		જ્યારે (url < end && *url != quote)
			url++;
	} નહીંતર {
		/* url-nq */
		જ્યારે (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	જો (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		જો (new_url == ખાલી) {
			dom_string_unref(content);
			પાછા NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		જો (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_સંદેશ_REFRESH,
					  &સંદેશ_data);
			c->refresh = true;
		}

		મુક્ત(new_url);

	}

	dom_string_unref(content);

	પાછા error;
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
સ્થિર nserror html_process_inserted_title(html_content *htmlc, dom_ગાંઠ *ગાંઠ)
{
	જો (htmlc->title == ખાલી) {
		/* only the first title is considered */
		htmlc->title = dom_ગાંઠ_ref(ગાંઠ);
	}
	પાછા NSERROR_OK;
}


/** process title node */
સ્થિર bool html_process_title(html_content *c, dom_ગાંઠ *ગાંઠ)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	અક્ષર *title_str;
	bool success;

	exc = dom_ગાંઠ_get_text_content(ગાંઠ, &title);
	જો ((exc != DOM_NO_ERR) || (title == ખાલી)) {
		પાછા false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	જો (title_str == ખાલી) {
		પાછા false;
	}

	success = content__set_title(&c->base, title_str);

	મુક્ત(title_str);

	પાછા success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
સ્થિર શૂન્ય html_texty_element_update(html_content *htmlc, dom_ગાંઠ *ગાંઠ)
{
	સંરચના box *box = box_for_ગાંઠ(ગાંઠ);
	જો (box == ખાલી) {
		પાછા; /* No Box (yet?) so no gadget to update */
	}
	જો (box->gadget == ખાલી) {
		પાછા; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
સ્થિર શૂન્ય
dom_default_action_DOMNodeInserted_cb(સંરચના dom_event *evt, શૂન્ય *pw)
{
	dom_event_target *ગાંઠ;
	dom_ગાંઠ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ગાંઠ);
	જો ((exc != DOM_NO_ERR) || (ગાંઠ == ખાલી)) {
		/* failed to obtain the event target node */
		પાછા;
	}

	exc = dom_ગાંઠ_get_ગાંઠ_type(ગાંઠ, &type);
	જો ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ગાંઠ)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(ગાંઠ, &tag_type);
		જો (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		પસંદગી (tag_type) {
		સ્થિતિ DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_ગાંઠ *)ગાંઠ);
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_ગાંઠ *)ગાંઠ);
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_ગાંઠ *)ગાંઠ);
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_ગાંઠ *)ગાંઠ);
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_STYLE:
			જો (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_ગાંઠ *)ગાંઠ);
			}
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)ગાંઠ);
			તોડો;

		સ્થિતિ DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_ગાંઠ *)ગાંઠ);
			તોડો;

		મૂળભૂત:
			તોડો;
		}

		જો (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			જો (htmlc->jsthread == ખાલી) {
				સંઘ content_સંદેશ_data સંદેશ_data;

				સંદેશ_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_સંદેશ_GETTHREAD,
						  &સંદેશ_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			જો (htmlc->jsthread != ખાલી) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) ગાંઠ);
			}
		}
	}
	dom_ગાંઠ_unref(ગાંઠ);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
સ્થિર શૂન્ય
dom_default_action_DOMNodeInsertedIntoDocument_cb(સંરચના dom_event *evt,
						  શૂન્ય *pw)
{
	html_content *htmlc = pw;
	dom_event_target *ગાંઠ;
	dom_ગાંઠ_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &ગાંઠ);
	જો ((exc == DOM_NO_ERR) && (ગાંઠ != ખાલી)) {
		exc = dom_ગાંઠ_get_ગાંઠ_type(ગાંઠ, &type);
		જો ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ગાંઠ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ગાંઠ, &tag_type);
			જો (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			પસંદગી (tag_type) {
			સ્થિતિ DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) ગાંઠ);
				fallthrough;
			મૂળભૂત:
				તોડો;
			}
		}
		dom_ગાંઠ_unref(ગાંઠ);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
સ્થિર શૂન્ય
dom_default_action_DOMSubtreeModified_cb(સંરચના dom_event *evt, શૂન્ય *pw)
{
	dom_event_target *ગાંઠ;
	dom_ગાંઠ_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &ગાંઠ);
	જો ((exc == DOM_NO_ERR) && (ગાંઠ != ખાલી)) {
		જો (htmlc->title == (dom_ગાંઠ *)ગાંઠ) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_ગાંઠ *)ગાંઠ);
			dom_ગાંઠ_unref(ગાંઠ);
			પાછા;
		}

		exc = dom_ગાંઠ_get_ગાંઠ_type(ગાંઠ, &type);
		જો ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_ગાંઠ)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(ગાંઠ, &tag_type);
			જો (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			પસંદગી (tag_type) {
			સ્થિતિ DOM_HTML_ELEMENT_TYPE_STYLE:
				જો (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_ગાંઠ *)ગાંઠ);
				}
				તોડો;
			સ્થિતિ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			સ્થિતિ DOM_HTML_ELEMENT_TYPE_આગત:
				html_texty_element_update(htmlc, (dom_ગાંઠ *)ગાંઠ);
				fallthrough;
			મૂળભૂત:
				તોડો;
			}
		}
		dom_ગાંઠ_unref(ગાંઠ);
	}
}


/**
 * callback for default action finished
 */
સ્થિર શૂન્ય
dom_default_action_finished_cb(સંરચના dom_event *evt, શૂન્ય *pw)
{
	html_content *htmlc = pw;

	જો (htmlc->jsthread != ખાલી)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  શૂન્ય **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	જો (phase == DOM_DEFAULT_ACTION_END) {
		જો (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			પાછા dom_default_action_DOMNodeInserted_cb;
		} નહીંતર જો (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			પાછા dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} નહીંતર જો (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			પાછા dom_default_action_DOMSubtreeModified_cb;
		}
	} નહીંતર જો (phase == DOM_DEFAULT_ACTION_FINISHED) {
		પાછા dom_default_action_finished_cb;
	}
	પાછા ખાલી;
}
