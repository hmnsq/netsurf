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

#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "utils/string.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "javascript/js.h"

#అన్తర్భావ "netsurf/bitmap.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/object.h"
#అన్తర్భావ "html/css.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_construct.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
స్థిర bool html_process_inserted_పునాది(html_content *htmlc, dom_కణుపు *కణుపు)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_href, &atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_దత్తాంశం(atr_string), &url);
		dom_string_unref(atr_string);
		ఒకవేళ (error == NSERROR_OK) {
			ఒకవేళ (htmlc->పునాది_url != ఖాళీ) {
				nsurl_unref(htmlc->పునాది_url);
			}
			htmlc->పునాది_url = url;
		}
	}


	/* get target attribute if present and not already set */
	ఒకవేళ (htmlc->పునాది_లక్ష్యం != ఖాళీ) {
		తిరిగి true;
	}

	exc = dom_element_get_attribute(కణుపు,
					corestring_dom_లక్ష్యం,
					&atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		ఒకవేళ (*dom_string_దత్తాంశం(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_మాతృ) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_శిఖరం_2)) {
			htmlc->పునాది_లక్ష్యం = strdup(dom_string_దత్తాంశం(atr_string));
		}
		dom_string_unref(atr_string);
	}

	తిరిగి true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
స్థిర bool html_process_inserted_img(html_content *htmlc, dom_కణుపు *కణుపు)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	ఒకవేళ (nsoption_bool(foreground_images) == false) {
		తిరిగి true;
	}

	exc = dom_element_get_attribute(కణుపు, corestring_dom_src, &src);
	ఒకవేళ (exc != DOM_NO_ERR || src == ఖాళీ) {
		తిరిగి true;
	}

	err = nsurl_join(htmlc->పునాది_url, dom_string_దత్తాంశం(src), &url);
	ఒకవేళ (err != NSERROR_OK) {
		dom_string_unref(src);
		తిరిగి false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, ఖాళీ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	తిరిగి success;
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
స్థిర bool html_process_inserted_link(html_content *c, dom_కణుపు *కణుపు)
{
	నిర్మాణం content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_కణుపు *)కణుపు);

	/* try Generic link handling */

	memset(&link, 0, పరిమాణం(నిర్మాణం content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_rel, &atr_string);
	ఒకవేళ ((exc != DOM_NO_ERR) || (atr_string == ఖాళీ)) {
		తిరిగి false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_href, &atr_string);
	ఒకవేళ ((exc != DOM_NO_ERR) || (atr_string == ఖాళీ)) {
		lwc_string_unref(link.rel);
		తిరిగి false;
	}

	/* get nsurl */
	error = nsurl_join(c->పునాది_url, dom_string_దత్తాంశం(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	ఒకవేళ (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		తిరిగి false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_hreflang, &atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		/* get a lwc string containing the href lang */
		(శూన్య)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_type, &atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		/* get a lwc string containing the type */
		(శూన్య)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_media, &atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		/* get a lwc string containing the media */
		(శూన్య)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(కణుపు,
			corestring_dom_sizes, &atr_string);
	ఒకవేళ ((exc == DOM_NO_ERR) && (atr_string != ఖాళీ)) {
		/* get a lwc string containing the sizes */
		(శూన్య)dom_string_intern(atr_string, &link.sizes);
		dom_string_unref(atr_string);
	}

	/* add to content */
	content_కూడు_rfc5988_link(&c->పునాది, &link);

	lwc_string_unref(link.sizes);
	lwc_string_unref(link.media);
	lwc_string_unref(link.type);
	lwc_string_unref(link.hreflang);

	nsurl_unref(link.href);
	lwc_string_unref(link.rel);

	తిరిగి true;
}


/* handler for a SCRIPT which has been added to a tree */
స్థిర శూన్య
dom_SCRIPT_showed_పైకి(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	ఒకవేళ (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		తిరిగి;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	ఒకవేళ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		తిరిగి;
	}

	ఒకవేళ (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		తిరిగి;
	}

	exc = dom_కణుపు_కలిగియుంది(htmlc->document, script, &within);
	ఒకవేళ (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		తిరిగి;
	}

	ఒకవేళ (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		తిరిగి;
	}

	res = html_process_script(htmlc, (dom_కణుపు *) script);
	ఒకవేళ (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} లేకపోతే {
		ఒకవేళ (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} లేకపోతే {
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
స్థిర nserror html_process_inserted_meta(html_content *c, dom_కణుపు *n)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	స్థిరరాశి అక్షరం *url, *చివర, *refresh = ఖాళీ;
	అక్షరం *new_url;
	అక్షరం quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	ఒకవేళ (c->refresh) {
		/* refresh already delt with */
		తిరిగి NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (equiv == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	ఒకవేళ (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		తిరిగి NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (content == ఖాళీ) {
		తిరిగి NSERROR_OK;
	}

	చివర = dom_string_దత్తాంశం(content) + dom_string_byte_నిడివి(content);

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

	url = dom_string_దత్తాంశం(content);

	/* *LWS */
	అయితే (url < చివర && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	ఒకవేళ (url == చివర || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		తిరిగి NSERROR_OK;
	}

	సందేశం_దత్తాంశం.delay = (పూర్ణాంకం) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	ఒకవేళ (సందేశం_దత్తాంశం.delay < 1) {
		సందేశం_దత్తాంశం.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	అయితే (url < చివర && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	అయితే (url < చివర && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	ఒకవేళ (url < చివర && *url == ';')
		url++;

	/* *LWS */
	అయితే (url < చివర && ascii_is_space(*url)) {
		url++;
	}

	ఒకవేళ (url == చివర) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->పునాది.refresh = nsurl_ref(content_get_url(&c->పునాది));

		content_ప్రసారం(&c->పునాది, CONTENT_సందేశం_REFRESH, &సందేశం_దత్తాంశం);

		తిరిగి NSERROR_OK;
	}

	/* "url" */
	ఒకవేళ (url <= చివర - 3) {
		ఒకవేళ (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} లేకపోతే {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			తిరిగి NSERROR_OK;
		}
	} లేకపోతే {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		తిరిగి NSERROR_OK;
	}

	/* *LWS */
	అయితే (url < చివర && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	ఒకవేళ (url < చివర) {
		ఒకవేళ (*url == '=') {
			url++;
		} లేకపోతే {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			తిరిగి NSERROR_OK;
		}
	} లేకపోతే {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		తిరిగి NSERROR_OK;
	}

	/* *LWS */
	అయితే (url < చివర && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	ఒకవేళ (url < చివర && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	ఒకవేళ (quote != 0) {
		/* url-sq | url-dq */
		అయితే (url < చివర && *url != quote)
			url++;
	} లేకపోతే {
		/* url-nq */
		అయితే (url < చివర && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	ఒకవేళ (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		ఒకవేళ (new_url == ఖాళీ) {
			dom_string_unref(content);
			తిరిగి NSERROR_NOMEM;
		}

		error = nsurl_join(c->పునాది_url, new_url, &nsurl);
		ఒకవేళ (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->పునాది.refresh = nsurl;

			content_ప్రసారం(&c->పునాది,
					  CONTENT_సందేశం_REFRESH,
					  &సందేశం_దత్తాంశం);
			c->refresh = true;
		}

		విడుదల(new_url);

	}

	dom_string_unref(content);

	తిరిగి error;
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
స్థిర nserror html_process_inserted_title(html_content *htmlc, dom_కణుపు *కణుపు)
{
	ఒకవేళ (htmlc->title == ఖాళీ) {
		/* only the first title is considered */
		htmlc->title = dom_కణుపు_ref(కణుపు);
	}
	తిరిగి NSERROR_OK;
}


/** process title node */
స్థిర bool html_process_title(html_content *c, dom_కణుపు *కణుపు)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	అక్షరం *title_str;
	bool success;

	exc = dom_కణుపు_get_పాఠ్యం_content(కణుపు, &title);
	ఒకవేళ ((exc != DOM_NO_ERR) || (title == ఖాళీ)) {
		తిరిగి false;
	}

	title_str = squash_whitespace(dom_string_దత్తాంశం(title));
	dom_string_unref(title);

	ఒకవేళ (title_str == ఖాళీ) {
		తిరిగి false;
	}

	success = content_సమితి_title(&c->పునాది, title_str);

	విడుదల(title_str);

	తిరిగి success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
స్థిర శూన్య html_texty_element_update(html_content *htmlc, dom_కణుపు *కణుపు)
{
	నిర్మాణం box *box = box_for_కణుపు(కణుపు);
	ఒకవేళ (box == ఖాళీ) {
		తిరిగి; /* No Box (yet?) so no gadget to update */
	}
	ఒకవేళ (box->gadget == ఖాళీ) {
		తిరిగి; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
స్థిర శూన్య
dom_default_action_DOMNodeInserted_cb(నిర్మాణం dom_event *evt, శూన్య *pw)
{
	dom_event_లక్ష్యం *కణుపు;
	dom_కణుపు_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_లక్ష్యం(evt, &కణుపు);
	ఒకవేళ ((exc != DOM_NO_ERR) || (కణుపు == ఖాళీ)) {
		/* failed to obtain the event target node */
		తిరిగి;
	}

	exc = dom_కణుపు_get_కణుపు_type(కణుపు, &type);
	ఒకవేళ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_కణుపు)) {
		/* an element node has been inserted */
		dom_html_element_type ట్యాగ్_type;

		exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &ట్యాగ్_type);
		ఒకవేళ (exc != DOM_NO_ERR) {
			ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		ఎంపిక (ట్యాగ్_type) {
		స్థితి DOM_HTML_ELEMENT_TYPE_పునాది:
			html_process_inserted_పునాది(htmlc, (dom_కణుపు *)కణుపు);
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_కణుపు *)కణుపు);
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_కణుపు *)కణుపు);
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_కణుపు *)కణుపు);
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_STYLE:
			ఒకవేళ (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_కణుపు *)కణుపు);
			}
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_పైకి(htmlc,
					     (dom_html_script_element *)కణుపు);
			ఆపు;

		స్థితి DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_కణుపు *)కణుపు);
			ఆపు;

		అప్రమేయ:
			ఆపు;
		}

		ఒకవేళ (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			ఒకవేళ (htmlc->jsthread == ఖాళీ) {
				సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;

				సందేశం_దత్తాంశం.jsthread = &htmlc->jsthread;
				content_ప్రసారం(&htmlc->పునాది,
						  CONTENT_సందేశం_GETTHREAD,
						  &సందేశం_దత్తాంశం);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			ఒకవేళ (htmlc->jsthread != ఖాళీ) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) కణుపు);
			}
		}
	}
	dom_కణుపు_unref(కణుపు);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
స్థిర శూన్య
dom_default_action_DOMNodeInsertedIntoDocument_cb(నిర్మాణం dom_event *evt,
						  శూన్య *pw)
{
	html_content *htmlc = pw;
	dom_event_లక్ష్యం *కణుపు;
	dom_కణుపు_type type;
	dom_exception exc;

	exc = dom_event_get_లక్ష్యం(evt, &కణుపు);
	ఒకవేళ ((exc == DOM_NO_ERR) && (కణుపు != ఖాళీ)) {
		exc = dom_కణుపు_get_కణుపు_type(కణుపు, &type);
		ఒకవేళ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_కణుపు)) {
			/* an element node has been modified */
			dom_html_element_type ట్యాగ్_type;

			exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &ట్యాగ్_type);
			ఒకవేళ (exc != DOM_NO_ERR) {
				ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ఎంపిక (ట్యాగ్_type) {
			స్థితి DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_పైకి(htmlc, (dom_html_script_element *) కణుపు);
				fallthrough;
			అప్రమేయ:
				ఆపు;
			}
		}
		dom_కణుపు_unref(కణుపు);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
స్థిర శూన్య
dom_default_action_DOMSubtreeModified_cb(నిర్మాణం dom_event *evt, శూన్య *pw)
{
	dom_event_లక్ష్యం *కణుపు;
	dom_కణుపు_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_లక్ష్యం(evt, &కణుపు);
	ఒకవేళ ((exc == DOM_NO_ERR) && (కణుపు != ఖాళీ)) {
		ఒకవేళ (htmlc->title == (dom_కణుపు *)కణుపు) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_కణుపు *)కణుపు);
			dom_కణుపు_unref(కణుపు);
			తిరిగి;
		}

		exc = dom_కణుపు_get_కణుపు_type(కణుపు, &type);
		ఒకవేళ ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_కణుపు)) {
			/* an element node has been modified */
			dom_html_element_type ట్యాగ్_type;

			exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &ట్యాగ్_type);
			ఒకవేళ (exc != DOM_NO_ERR) {
				ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			ఎంపిక (ట్యాగ్_type) {
			స్థితి DOM_HTML_ELEMENT_TYPE_STYLE:
				ఒకవేళ (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_కణుపు *)కణుపు);
				}
				ఆపు;
			స్థితి DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			స్థితి DOM_HTML_ELEMENT_TYPE_ఆదానం:
				html_texty_element_update(htmlc, (dom_కణుపు *)కణుపు);
				fallthrough;
			అప్రమేయ:
				ఆపు;
			}
		}
		dom_కణుపు_unref(కణుపు);
	}
}


/**
 * callback for default action finished
 */
స్థిర శూన్య
dom_default_action_ముగిసిన_cb(నిర్మాణం dom_event *evt, శూన్య *pw)
{
	html_content *htmlc = pw;

	ఒకవేళ (htmlc->jsthread != ఖాళీ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  శూన్య **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_దత్తాంశం(type));

	ఒకవేళ (phase == DOM_DEFAULT_ACTION_చివర) {
		ఒకవేళ (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			తిరిగి dom_default_action_DOMNodeInserted_cb;
		} లేకపోతే ఒకవేళ (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			తిరిగి dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} లేకపోతే ఒకవేళ (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			తిరిగి dom_default_action_DOMSubtreeModified_cb;
		}
	} లేకపోతే ఒకవేళ (phase == DOM_DEFAULT_ACTION_ముగిసిన) {
		తిరిగి dom_default_action_ముగిసిన_cb;
	}
	తిరిగి ఖాళీ;
}
