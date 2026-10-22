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

#شامل <string.h>

#شامل "utils/config.h"
#شامل "utils/utils.h"
#شامل "utils/corestrings.h"
#شامل "utils/nsoption.h"
#شامل "utils/log.h"
#شامل "utils/ascii.h"
#شامل "utils/string.h"
#شامل "utils/nsurl.h"
#شامل "content/content.h"
#شامل "javascript/js.h"

#شامل "netsurf/bitmap.h"

#شامل "html/private.h"
#شامل "html/object.h"
#شامل "html/css.h"
#شامل "html/box.h"
#شامل "html/box_construct.h"
#شامل "html/form_internal.h"
#شامل "html/dom_event.h"


/**
 * process a base element being inserted into the DOM
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ایستا bool html_process_inserted_base(html_content *htmlc, dom_گره *گره)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;

	/* get href attribute if present */
	exc = dom_element_get_attribute(گره, corestring_dom_href, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		nsurl *url;
		nserror error;

		/* get url from string */
		error = nsurl_create(dom_string_data(atr_string), &url);
		dom_string_unref(atr_string);
		اگر (error == NSERROR_OK) {
			اگر (htmlc->base_url != پوچ) {
				nsurl_unref(htmlc->base_url);
			}
			htmlc->base_url = url;
		}
	}


	/* get target attribute if present and not already set */
	اگر (htmlc->base_target != پوچ) {
		بازگشت true;
	}

	exc = dom_element_get_attribute(گره,
					corestring_dom_target,
					&atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		/* Validation rules from the HTML5 spec for the base element:
		 *  The target must be one of _blank, _self, _parent, or
		 *  _top or any identifier which does not begin with an
		 *  underscore
		 */
		اگر (*dom_string_data(atr_string) != '_' ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__blank) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__self) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc__parent) ||
				dom_string_caseless_lwc_isequal(atr_string,
						corestring_lwc_بالاترین_2)) {
			htmlc->base_target = strdup(dom_string_data(atr_string));
		}
		dom_string_unref(atr_string);
	}

	بازگشت true;
}



/**
 * Process img element being inserted into the DOM.
 *
 * \param htmlc The html content containing the DOM
 * \param node The DOM node being inserted
 * \return NSERROR_OK on success else appropriate error code
 */
ایستا bool html_process_inserted_img(html_content *htmlc, dom_گره *گره)
{
	dom_string *src;
	nsurl *url;
	nserror err;
	dom_exception exc;
	bool success;

	/* Do nothing if foreground images are disabled */
	اگر (nsoption_bool(foreground_images) == false) {
		بازگشت true;
	}

	exc = dom_element_get_attribute(گره, corestring_dom_src, &src);
	اگر (exc != DOM_NO_ERR || src == پوچ) {
		بازگشت true;
	}

	err = nsurl_join(htmlc->base_url, dom_string_data(src), &url);
	اگر (err != NSERROR_OK) {
		dom_string_unref(src);
		بازگشت false;
	}
	dom_string_unref(src);

	/* Speculatively fetch the image */
	success = html_fetch_object(htmlc, url, پوچ, CONTENT_IMAGE, false);
	nsurl_unref(url);

	بازگشت success;
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
ایستا bool html_process_inserted_link(html_content *c, dom_گره *گره)
{
	ساختار content_rfc5988_link link; /* the link added to the content */
	dom_exception exc; /* returned by libdom functions */
	dom_string *atr_string;
	nserror error;

	/* Handle stylesheet loading */
	html_css_process_link(c, (dom_گره *)گره);

	/* try Generic link handling */

	memset(&link, 0, اندازه(ساختار content_rfc5988_link));

	/* check that the relation exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گره, corestring_dom_rel, &atr_string);
	اگر ((exc != DOM_NO_ERR) || (atr_string == پوچ)) {
		بازگشت false;
	}
	/* get a lwc string containing the link relation */
	exc = dom_string_intern(atr_string, &link.rel);
	dom_string_unref(atr_string);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	/* check that the href exists - w3c spec says must be present */
	exc = dom_element_get_attribute(گره, corestring_dom_href, &atr_string);
	اگر ((exc != DOM_NO_ERR) || (atr_string == پوچ)) {
		lwc_string_unref(link.rel);
		بازگشت false;
	}

	/* get nsurl */
	error = nsurl_join(c->base_url, dom_string_data(atr_string),
			&link.href);
	dom_string_unref(atr_string);
	اگر (error != NSERROR_OK) {
		lwc_string_unref(link.rel);
		بازگشت false;
	}

	/* look for optional properties -- we don't care if internment fails */

	exc = dom_element_get_attribute(گره,
			corestring_dom_hreflang, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		/* get a lwc string containing the href lang */
		(تهی)dom_string_intern(atr_string, &link.hreflang);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_type, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		/* get a lwc string containing the type */
		(تهی)dom_string_intern(atr_string, &link.type);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_media, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		/* get a lwc string containing the media */
		(تهی)dom_string_intern(atr_string, &link.media);
		dom_string_unref(atr_string);
	}

	exc = dom_element_get_attribute(گره,
			corestring_dom_sizes, &atr_string);
	اگر ((exc == DOM_NO_ERR) && (atr_string != پوچ)) {
		/* get a lwc string containing the sizes */
		(تهی)dom_string_intern(atr_string, &link.sizes);
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

	بازگشت true;
}


/* handler for a SCRIPT which has been added to a tree */
ایستا تهی
dom_SCRIPT_showed_up(html_content *htmlc, dom_html_script_element *script)
{
	dom_exception exc;
	dom_html_script_element_flags flags;
	dom_hubbub_error res;
	bool within;

	اگر (!htmlc->enable_scripting) {
		NSLOG(netsurf, INFO, "Encountered a script, but scripting is off, ignoring");
		بازگشت;
	}

	NSLOG(netsurf, DEEPDEBUG, "Encountered a script, node %p showed up", script);

	exc = dom_html_script_element_get_flags(script, &flags);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve flags, giving up");
		بازگشت;
	}

	اگر (flags & DOM_HTML_SCRIPT_ELEMENT_FLAG_PARSER_INSERTED) {
		NSLOG(netsurf, DEBUG, "Script was parser inserted, skipping");
		بازگشت;
	}

	exc = dom_گره_contains(htmlc->document, script, &within);
	اگر (exc != DOM_NO_ERR) {
		NSLOG(netsurf, DEBUG, "Unable to determine if script was within document, ignoring");
		بازگشت;
	}

	اگر (!within) {
		NSLOG(netsurf, DEBUG, "Script was not within the document, ignoring for now");
		بازگشت;
	}

	res = html_process_script(htmlc, (dom_گره *) script);
	اگر (res == DOM_HUBBUB_OK) {
		NSLOG(netsurf, DEEPDEBUG, "Inserted script has finished running");
	} وگرنه {
		اگر (res == (DOM_HUBBUB_HUBBUB_ERR | HUBBUB_PAUSED)) {
			NSLOG(netsurf, DEEPDEBUG, "Inserted script has launced asynchronously");
		} وگرنه {
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
ایستا nserror html_process_inserted_meta(html_content *c, dom_گره *n)
{
	اجتماع content_پیام_data پیام_data;
	ثابت نویسه *url, *end, *refresh = پوچ;
	نویسه *new_url;
	نویسه quote = '\0';
	dom_string *equiv, *content;
	dom_exception exc;
	nsurl *nsurl;
	nserror error = NSERROR_OK;

	اگر (c->refresh) {
		/* refresh already delt with */
		بازگشت NSERROR_OK;
	}

	exc = dom_element_get_attribute(n, corestring_dom_http_equiv, &equiv);
	اگر (exc != DOM_NO_ERR) {
		بازگشت NSERROR_DOM;
	}

	اگر (equiv == پوچ) {
		بازگشت NSERROR_OK;
	}

	اگر (!dom_string_caseless_lwc_isequal(equiv, corestring_lwc_refresh)) {
		dom_string_unref(equiv);
		بازگشت NSERROR_OK;
	}

	dom_string_unref(equiv);

	exc = dom_element_get_attribute(n, corestring_dom_content, &content);
	اگر (exc != DOM_NO_ERR) {
		بازگشت NSERROR_DOM;
	}

	اگر (content == پوچ) {
		بازگشت NSERROR_OK;
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
	وقتی (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* intpart */
	اگر (url == end || (*url < '0' || '9' < *url)) {
		/* Empty content, or invalid timeval */
		dom_string_unref(content);
		بازگشت NSERROR_OK;
	}

	پیام_data.delay = (صحیح) strtol(url, &new_url, 10);
	/* a very small delay and self-referencing URL can cause a loop
	 * that grinds machines to a halt. To prevent this we set a
	 * minimum refresh delay of 1s. */
	اگر (پیام_data.delay < 1) {
		پیام_data.delay = 1;
	}

	url = new_url;

	/* fracpart? (ignored, as delay is integer only) */
	وقتی (url < end && (('0' <= *url && *url <= '9') ||
			*url == '.')) {
		url++;
	}

	/* *LWS */
	وقتی (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* ';' */
	اگر (url < end && *url == ';')
		url++;

	/* *LWS */
	وقتی (url < end && ascii_is_space(*url)) {
		url++;
	}

	اگر (url == end) {
		/* Just delay specified, so refresh current page */
		dom_string_unref(content);

		c->base.refresh = nsurl_ref(content_get_url(&c->base));

		content_broadcast(&c->base, CONTENT_پیام_REFRESH, &پیام_data);

		بازگشت NSERROR_OK;
	}

	/* "url" */
	اگر (url <= end - 3) {
		اگر (strncasecmp(url, "url", 3) == 0) {
			url += 3;
		} وگرنه {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			بازگشت NSERROR_OK;
		}
	} وگرنه {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		بازگشت NSERROR_OK;
	}

	/* *LWS */
	وقتی (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '=' */
	اگر (url < end) {
		اگر (*url == '=') {
			url++;
		} وگرنه {
			/* Unexpected input, ignore this header */
			dom_string_unref(content);
			بازگشت NSERROR_OK;
		}
	} وگرنه {
		/* Insufficient input, ignore this header */
		dom_string_unref(content);
		بازگشت NSERROR_OK;
	}

	/* *LWS */
	وقتی (url < end && ascii_is_space(*url)) {
		url++;
	}

	/* '"' or "'" */
	اگر (url < end && (*url == '"' || *url == '\'')) {
		quote = *url;
		url++;
	}

	/* Start of URL */
	refresh = url;

	اگر (quote != 0) {
		/* url-sq | url-dq */
		وقتی (url < end && *url != quote)
			url++;
	} وگرنه {
		/* url-nq */
		وقتی (url < end && !ascii_is_space(*url))
			url++;
	}

	/* '"' or "'" or *LWS (we don't care) */
	اگر (url > refresh) {
		/* There's a URL */
		new_url = strndup(refresh, url - refresh);
		اگر (new_url == پوچ) {
			dom_string_unref(content);
			بازگشت NSERROR_NOMEM;
		}

		error = nsurl_join(c->base_url, new_url, &nsurl);
		اگر (error == NSERROR_OK) {
			/* broadcast valid refresh url */

			c->base.refresh = nsurl;

			content_broadcast(&c->base,
					  CONTENT_پیام_REFRESH,
					  &پیام_data);
			c->refresh = true;
		}

		آزاد(new_url);

	}

	dom_string_unref(content);

	بازگشت error;
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
ایستا nserror html_process_inserted_title(html_content *htmlc, dom_گره *گره)
{
	اگر (htmlc->title == پوچ) {
		/* only the first title is considered */
		htmlc->title = dom_گره_ref(گره);
	}
	بازگشت NSERROR_OK;
}


/** process title node */
ایستا bool html_process_title(html_content *c, dom_گره *گره)
{
	dom_exception exc; /* returned by libdom functions */
	dom_string *title;
	نویسه *title_str;
	bool success;

	exc = dom_گره_get_text_content(گره, &title);
	اگر ((exc != DOM_NO_ERR) || (title == پوچ)) {
		بازگشت false;
	}

	title_str = squash_whitespace(dom_string_data(title));
	dom_string_unref(title);

	اگر (title_str == پوچ) {
		بازگشت false;
	}

	success = content__set_title(&c->base, title_str);

	آزاد(title_str);

	بازگشت success;
}


/**
 * Deal with input elements being modified by resyncing their gadget
 * if they have one.
 */
ایستا تهی html_texty_element_update(html_content *htmlc, dom_گره *گره)
{
	ساختار box *box = box_for_گره(گره);
	اگر (box == پوچ) {
		بازگشت; /* No Box (yet?) so no gadget to update */
	}
	اگر (box->gadget == پوچ) {
		بازگشت; /* No gadget yet (under construction perhaps?) */
	}
	form_gadget_sync_with_dom(box->gadget);
	/* And schedule a redraw for the box */
	html__redraw_a_box(htmlc, box);
}


/**
 * callback for DOMNodeInserted end type
 */
ایستا تهی
dom_default_action_DOMNodeInserted_cb(ساختار dom_event *evt, تهی *pw)
{
	dom_event_target *گره;
	dom_گره_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &گره);
	اگر ((exc != DOM_NO_ERR) || (گره == پوچ)) {
		/* failed to obtain the event target node */
		بازگشت;
	}

	exc = dom_گره_get_گره_type(گره, &type);
	اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
		/* an element node has been inserted */
		dom_html_element_type tag_type;

		exc = dom_html_element_get_tag_type(گره, &tag_type);
		اگر (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}

		انتخاب (tag_type) {
		حالت DOM_HTML_ELEMENT_TYPE_BASE:
			html_process_inserted_base(htmlc, (dom_گره *)گره);
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_IMG:
			html_process_inserted_img(htmlc, (dom_گره *)گره);
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_LINK:
			html_process_inserted_link(htmlc, (dom_گره *)گره);
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_META:
			html_process_inserted_meta(htmlc, (dom_گره *)گره);
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_STYLE:
			اگر (nsoption_bool(author_level_css)) {
				html_css_process_style(htmlc, (dom_گره *)گره);
			}
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
			dom_SCRIPT_showed_up(htmlc,
					     (dom_html_script_element *)گره);
			بشکن;

		حالت DOM_HTML_ELEMENT_TYPE_TITLE:
			html_process_inserted_title(htmlc, (dom_گره *)گره);
			بشکن;

		پیشفرض:
			بشکن;
		}

		اگر (htmlc->enable_scripting) {
			/* ensure javascript context is available */
			اگر (htmlc->jsthread == پوچ) {
				اجتماع content_پیام_data پیام_data;

				پیام_data.jsthread = &htmlc->jsthread;
				content_broadcast(&htmlc->base,
						  CONTENT_پیام_GETTHREAD,
						  &پیام_data);
				NSLOG(netsurf, INFO,
				      "javascript context: %p (htmlc: %p)",
				      htmlc->jsthread,
				      htmlc);
			}
			اگر (htmlc->jsthread != پوچ) {
				js_handle_new_element(htmlc->jsthread,
						      (dom_element *) گره);
			}
		}
	}
	dom_گره_unref(گره);
}


/**
 * callback for DOMNodeInsertedIntoDocument end type
 */
ایستا تهی
dom_default_action_DOMNodeInsertedIntoDocument_cb(ساختار dom_event *evt,
						  تهی *pw)
{
	html_content *htmlc = pw;
	dom_event_target *گره;
	dom_گره_type type;
	dom_exception exc;

	exc = dom_event_get_target(evt, &گره);
	اگر ((exc == DOM_NO_ERR) && (گره != پوچ)) {
		exc = dom_گره_get_گره_type(گره, &type);
		اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(گره, &tag_type);
			اگر (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			انتخاب (tag_type) {
			حالت DOM_HTML_ELEMENT_TYPE_SCRIPT:
				dom_SCRIPT_showed_up(htmlc, (dom_html_script_element *) گره);
				fallthrough;
			پیشفرض:
				بشکن;
			}
		}
		dom_گره_unref(گره);
	}
}


/**
 * callback for DOMSubtreeModified end type
 */
ایستا تهی
dom_default_action_DOMSubtreeModified_cb(ساختار dom_event *evt, تهی *pw)
{
	dom_event_target *گره;
	dom_گره_type type;
	dom_exception exc;
	html_content *htmlc = pw;

	exc = dom_event_get_target(evt, &گره);
	اگر ((exc == DOM_NO_ERR) && (گره != پوچ)) {
		اگر (htmlc->title == (dom_گره *)گره) {
			/* Node is our title node */
			html_process_title(htmlc, (dom_گره *)گره);
			dom_گره_unref(گره);
			بازگشت;
		}

		exc = dom_گره_get_گره_type(گره, &type);
		اگر ((exc == DOM_NO_ERR) && (type == DOM_ELEMENT_گره)) {
			/* an element node has been modified */
			dom_html_element_type tag_type;

			exc = dom_html_element_get_tag_type(گره, &tag_type);
			اگر (exc != DOM_NO_ERR) {
				tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
			}

			انتخاب (tag_type) {
			حالت DOM_HTML_ELEMENT_TYPE_STYLE:
				اگر (nsoption_bool(author_level_css)) {
					html_css_update_style(htmlc,
							(dom_گره *)گره);
				}
				بشکن;
			حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
			حالت DOM_HTML_ELEMENT_TYPE_درون_داد:
				html_texty_element_update(htmlc, (dom_گره *)گره);
				fallthrough;
			پیشفرض:
				بشکن;
			}
		}
		dom_گره_unref(گره);
	}
}


/**
 * callback for default action finished
 */
ایستا تهی
dom_default_action_finished_cb(ساختار dom_event *evt, تهی *pw)
{
	html_content *htmlc = pw;

	اگر (htmlc->jsthread != پوچ)
		js_event_cleanup(htmlc->jsthread, evt);
}


/* exported interface documented in html/dom_event.c */
dom_default_action_callback
html_dom_event_fetcher(dom_string *type,
		  dom_default_action_phase phase,
		  تهی **pw)
{
	NSLOG(netsurf, DEEPDEBUG,
	      "phase:%d type:%s", phase, dom_string_data(type));

	اگر (phase == DOM_DEFAULT_ACTION_END) {
		اگر (dom_string_isequal(type, corestring_dom_DOMNodeInserted)) {
			بازگشت dom_default_action_DOMNodeInserted_cb;
		} وگرنه اگر (dom_string_isequal(type, corestring_dom_DOMNodeInsertedIntoDocument)) {
			بازگشت dom_default_action_DOMNodeInsertedIntoDocument_cb;
		} وگرنه اگر (dom_string_isequal(type, corestring_dom_DOMSubtreeModified)) {
			بازگشت dom_default_action_DOMSubtreeModified_cb;
		}
	} وگرنه اگر (phase == DOM_DEFAULT_ACTION_FINISHED) {
		بازگشت dom_default_action_finished_cb;
	}
	بازگشت پوچ;
}
