/*
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
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
 * Implementation of HTML content handling.
 */

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <nsutils/time.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/http.h"
#അന്തര്ഭാവ "utils/libdom.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/utf8.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/string.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/utf8.h"
#അന്തര്ഭാവ "netsurf/keypress.h"
#അന്തര്ഭാവ "netsurf/layout.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/textsearch.h"
#അന്തര്ഭാവ "desktop/selection.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/textarea.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "javascript/js.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/dom_event.h"
#അന്തര്ഭാവ "html/css.h"
#അന്തര്ഭാവ "html/object.h"
#അന്തര്ഭാവ "html/html_save.h"
#അന്തര്ഭാവ "html/interaction.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_construct.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/imagemap.h"
#അന്തര്ഭാവ "html/layout.h"
#അന്തര്ഭാവ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

സ്ഥിതം സ്ഥിരം അക്ഷരം *html_types[] = {
	"application/xhtml+xml",
	"text/html"
};

/**
 * Fire an event at the DOM
 *
 * Helper that swallows DOM errors.
 *
 * \param[in] event   the event to fire at the DOM
 * \param[in] target  the event target
 * \return true on success
 */
സ്ഥിതം bool fire_dom_event(dom_event *event, dom_കണ്ണി *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		തിരികെ false;
	}

	തിരികെ result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_കണ്ണി *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	എങ്കിൽ (exc != DOM_NO_ERR) തിരികെ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		തിരികെ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (പൂർണ്ണസംഖ്യ)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	തിരികെ result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_കണ്ണി *target,
		bool bubbles, bool cancelable, uint32_t താക്കോൽ)
{
	bool is_special = താക്കോൽ <= 0x001F || (0x007F <= താക്കോൽ && താക്കോൽ <= 0x009F);
	dom_string *dom_താക്കോൽ = ശൂന്യ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	എങ്കിൽ (is_special) {
		തിരഞ്ഞെടുപ്പ് (താക്കോൽ) {
		അവസ്ഥ NS_താക്കോൽ_ESCAPE:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_Escape);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_ഇടത്:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_Arrow_ഇടത്);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_വലത്:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_Arrow_വലത്);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_UP:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_ArrowUp);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_DOWN:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_ArrowDown);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_PAGE_UP:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_PageUp);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_PAGE_DOWN:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_PageDown);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_TEXT_START:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_Home);
			നിർത്തുക;
		അവസ്ഥ NS_താക്കോൽ_TEXT_END:
			dom_താക്കോൽ = dom_string_ref(corestring_dom_End);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			dom_താക്കോൽ = ശൂന്യ;
			നിർത്തുക;
		}
	} അല്ലെങ്കിൽ {
		അക്ഷരം utf8[6];
		size_t length = utf8_from_ucs4(താക്കോൽ, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((സ്ഥിരം uint8_t *)utf8, strlen(utf8),
				&dom_താക്കോൽ);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			തിരികെ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_താക്കോൽ);
		തിരികെ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ശൂന്യ,
			dom_താക്കോൽ, ശൂന്യ, DOM_താക്കോൽ_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_താക്കോൽ);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		തിരികെ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(പൂർണ്ണസംഖ്യ)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	തിരികെ result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
സ്ഥിതം ശൂന്യം html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_കണ്ണി *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ശൂന്യ;

	/* Clean up and report error if unsuccessful or aborted */
	എങ്കിൽ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		എങ്കിൽ (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, ശൂന്യ);
		} അല്ലെങ്കിൽ {
			content_broadcast_error(&c->base, NSERROR_STOPPED, ശൂന്യ);
		}

		content_set_error(&c->base);
		തിരികെ;
	}


#എങ്കിൽ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#എങ്കിൽ ALWAYS_DUMP_FRAMESET
	എങ്കിൽ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (ശൂന്യം *) &html);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (html == ശൂന്യ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, ശൂന്യ);
		content_set_error(&c->base);
		തിരികെ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	എങ്കിൽ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, ശൂന്യ);
		content_set_error(&c->base);
		dom_കണ്ണി_unref(html);
		തിരികെ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ശൂന്യ;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_കണ്ണി_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	തിരഞ്ഞെടുപ്പ് (content__get_status(&html->base)) {
	അവസ്ഥ CONTENT_STATUS_READY:
		എങ്കിൽ (html->base.active == 0) {
			content_set_done(&html->base);
			തിരികെ NSERROR_OK;
		}
		നിർത്തുക;
	അവസ്ഥ CONTENT_STATUS_DONE:
		/* fallthrough */
	അവസ്ഥ CONTENT_STATUS_LOADING:
		തിരികെ NSERROR_OK;
	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		നിർത്തുക;
	}
	തിരികെ NSERROR_UNKNOWN;
}


സ്ഥിതം ശൂന്യം html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	സംഘം content_സന്ദേശം_data സന്ദേശം_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_സന്ദേശം_GETDIMS, &സന്ദേശം_data);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.width  = w;
	htmlc->media.height = h;
	htmlc->unit_len_ctx.viewport_width  = w;
	htmlc->unit_len_ctx.viewport_height = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_size = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_size)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_size)), F_10)), F_72);

	htmlc->unit_len_ctx.font_size_default = f_size;
	htmlc->unit_len_ctx.font_size_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
ശൂന്യം html_finish_conversion(html_content *htmlc)
{
	സംഘം content_സന്ദേശം_data സന്ദേശം_data;
	dom_exception exc; /* returned by libdom functions */
	dom_കണ്ണി *html;
	nserror error;

	/* Bail out if we've been aborted */
	എങ്കിൽ (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ശൂന്യ);
		content_set_error(&htmlc->base);
		തിരികെ;
	}

	/* If we already have a selection context, then we have already
	 * "finished" conversion.  We can get here twice if e.g. some JS
	 * adds a new stylesheet, and the stylesheet gets added after
	 * the HTML content is initially finished.
	 *
	 * If we didn't do this, the HTML content would try to rebuild the
	 * box tree for the html content when this new stylesheet is ready.
	 * NetSurf has no concept of dynamically changing documents, so this
	 * would break badly.
	 */
	എങ്കിൽ (htmlc->select_ctx != ശൂന്യ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		തിരികെ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	എങ്കിൽ (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, ശൂന്യ);
		content_set_error(&htmlc->base);
		തിരികെ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	എങ്കിൽ (htmlc->jsthread != ശൂന്യ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ശൂന്യ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	സന്ദേശം_data.explicit_status_text = ശൂന്യ;
	content_broadcast(&htmlc->base, CONTENT_സന്ദേശം_STATUS, &സന്ദേശം_data);

	exc = dom_document_get_document_element(htmlc->document, (ശൂന്യം *) &html);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (html == ശൂന്യ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ശൂന്യ);
		content_set_error(&htmlc->base);
		തിരികെ;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	എങ്കിൽ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_കണ്ണി_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, ശൂന്യ);
		content_set_error(&htmlc->base);
		തിരികെ;
	}

	dom_കണ്ണി_unref(html);
}


സ്ഥിതം ശൂന്യം
html_document_user_data_handler(dom_കണ്ണി_operation operation,
				dom_string *താക്കോൽ, ശൂന്യം *data,
				ഘടന dom_കണ്ണി *src,
				ഘടന dom_കണ്ണി *dst)
{
	എങ്കിൽ (dom_string_isequal(corestring_dom_ns_താക്കോൽ_html_content_data,
			       താക്കോൽ) == false || data == ശൂന്യ) {
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ് (operation) {
	അവസ്ഥ DOM_കണ്ണി_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		നിർത്തുക;
	അവസ്ഥ DOM_കണ്ണി_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		നിർത്തുക;
	അവസ്ഥ DOM_കണ്ണി_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		നിർത്തുക;
	അവസ്ഥ DOM_കണ്ണി_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		നിർത്തുക;
	അവസ്ഥ DOM_കണ്ണി_DELETED:
		/* This is the only path I expect */
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		പ്രതിജ്ഞാ(0);
	}
}


സ്ഥിതം nserror
html_create_html_data(html_content *c, സ്ഥിരം http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	ശൂന്യം *old_കണ്ണി_data;
	സ്ഥിരം അക്ഷരം *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = ശൂന്യ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ശൂന്യ;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = ശൂന്യ;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = ശൂന്യ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ശൂന്യ;
	c->bctx = ശൂന്യ;
	c->layout = ശൂന്യ;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_എണ്ണം = 0;
	c->stylesheets = ശൂന്യ;
	c->select_ctx = ശൂന്യ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ശൂന്യ;
	c->അക്കം_objects = 0;
	c->object_list = ശൂന്യ;
	c->forms = ശൂന്യ;
	c->imagemaps = ശൂന്യ;
	c->bw = ശൂന്യ;
	c->frameset = ശൂന്യ;
	c->iframe = ശൂന്യ;
	c->page = ശൂന്യ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_എണ്ണം = 0;
	c->scripts = ശൂന്യ;
	c->jsthread = ശൂന്യ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	എങ്കിൽ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ശൂന്യ;
		തിരികെ NSERROR_NOMEM;
	}

	c->sel = selection_create((ഘടന content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	എങ്കിൽ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		എങ്കിൽ (c->encoding == ശൂന്യ) {
			lwc_string_unref(c->universal);
			c->universal = ശൂന്യ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ശൂന്യ;
			തിരികെ NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.സന്ദേശം = ശൂന്യ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	എങ്കിൽ ((error != DOM_HUBBUB_OK) && (c->encoding != ശൂന്യ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		സ്വതന്ത്രം(c->encoding);
		c->encoding = ശൂന്യ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	എങ്കിൽ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = ശൂന്യ;

		lwc_string_unref(c->universal);
		c->universal = ശൂന്യ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ശൂന്യ;

		തിരികെ libdom_hubbub_error_to_nserror(error);
	}

	err = dom_കണ്ണി_set_user_data(c->document,
				     corestring_dom_ns_താക്കോൽ_html_content_data,
				     c, html_document_user_data_handler,
				     (ശൂന്യം *) &old_കണ്ണി_data);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ശൂന്യ;
		nsurl_unref(c->base_url);
		c->base_url = ശൂന്യ;

		lwc_string_unref(c->universal);
		c->universal = ശൂന്യ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ശൂന്യ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		തിരികെ NSERROR_DOM;
	}

	പ്രതിജ്ഞാ(old_കണ്ണി_data == ശൂന്യ);

	തിരികെ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

സ്ഥിതം nserror
html_create(സ്ഥിരം content_handler *handler,
	    lwc_string *imime_type,
	    സ്ഥിരം http_parameter *params,
	    llcache_handle *llcache,
	    സ്ഥിരം അക്ഷരം *fallback_charset,
	    bool quirks,
	    ഘടന content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, വലുപ്പം(html_content));
	എങ്കിൽ (html == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(html);
		തിരികെ error;
	}

	error = html_create_html_data(html, params);
	എങ്കിൽ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ശൂന്യ);
		സ്വതന്ത്രം(html);
		തിരികെ error;
	}

	error = html_css_new_stylesheets(html);
	എങ്കിൽ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ശൂന്യ);
		സ്വതന്ത്രം(html);
		തിരികെ error;
	}

	*c = (ഘടന content *) html;

	തിരികെ NSERROR_OK;
}



സ്ഥിതം nserror
html_process_encoding_change(ഘടന content *c,
			     സ്ഥിരം അക്ഷരം *data,
			     unsigned പൂർണ്ണസംഖ്യ size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	സ്ഥിരം അക്ഷരം *encoding;
	സ്ഥിരം uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	എങ്കിൽ (encoding == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (html->encoding != ശൂന്യ) {
		സ്വതന്ത്രം(html->encoding);
		html->encoding = ശൂന്യ;
	}

	html->encoding = strdup(encoding);
	എങ്കിൽ (html->encoding == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ശൂന്യ;

	എങ്കിൽ (html->document != ശൂന്യ) {
		dom_കണ്ണി_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.സന്ദേശം = ശൂന്യ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	എങ്കിൽ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		സ്വതന്ത്രം(html->encoding);
		html->encoding = strdup("Windows-1252");
		എങ്കിൽ (html->encoding == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		എങ്കിൽ (error != DOM_HUBBUB_OK) {
			തിരികെ libdom_hubbub_error_to_nserror(error);
		}

	}

	source_data = content__get_source_data(c, &source_size);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      source_data,
					      source_size);

	തിരികെ libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

സ്ഥിതം bool
html_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data, unsigned പൂർണ്ണസംഖ്യ size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (സ്ഥിരം uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	എങ്കിൽ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	എങ്കിൽ (err != NSERROR_OK) {
		content_broadcast_error(c, err, ശൂന്യ);
		തിരികെ false;
	}

	തിരികെ true;
}


/**
 * Convert a CONTENT_HTML for display.
 *
 * The following steps are carried out in order:
 *
 *  - parsing to an XML tree is completed
 *  - stylesheets are fetched
 *  - the XML tree is converted to a box tree and object fetches are started
 *
 * On exit, the content status will be either CONTENT_STATUS_DONE if the
 * document is completely loaded or CONTENT_STATUS_READY if objects are still
 * being fetched.
 */

സ്ഥിതം bool html_convert(ഘടന content *c)
{
	html_content *htmlc = (html_content *) c;
	dom_exception exc; /* returned by libdom functions */

	/* The quirk check and associated stylesheet fetch is "safe"
	 * once the root node has been inserted into the document
	 * which must have happened by this point in the parse.
	 *
	 * faliure to retrive the quirk mode or to start the
	 * stylesheet fetch is non fatal as this "only" affects the
	 * render and it would annoy the user to fail the entire
	 * render for want of a quirks stylesheet.
	 */
	exc = dom_document_get_quirks_mode(htmlc->document, &htmlc->quirks);
	എങ്കിൽ (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->base.active--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->base.active, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	എങ്കിൽ (html_can_begin_conversion(htmlc)) {
		തിരികെ html_begin_conversion(htmlc);
	}
	തിരികെ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	/* Cannot begin conversion if we're still fetching stuff */
	എങ്കിൽ (htmlc->base.active != 0)
		തിരികെ false;

	വേണ്ടി (i = 0; i != htmlc->stylesheet_എണ്ണം; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		എങ്കിൽ (htmlc->stylesheets[i].modified)
			തിരികെ false;
	}

	/* All is good, begin */
	തിരികെ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_കണ്ണി *html;
	nserror ns_error;
	ഘടന form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *കണ്ണി_name = ശൂന്യ;
	dom_hubbub_error error;

	/* The act of completing the parse can result in additional data
	 * being flushed through the parser. This may result in new style or
	 * script nodes, upon which the conversion depends. Thus, once we
	 * have completed the parse, we must check again to see if we can
	 * begin the conversion. If we can't, we must stop and wait for the
	 * new styles/scripts to be processed. Once they have been processed,
	 * we will be called again to begin the conversion for real. Thus,
	 * we must also ensure that we don't attempt to complete the parse
	 * multiple times, so store a flag to indicate that parsing is
	 * complete to avoid repeating the completion pointlessly.
	 */
	എങ്കിൽ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		എങ്കിൽ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			തിരികെ true;
		}
		എങ്കിൽ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						ശൂന്യ);

			തിരികെ false;
		}
		htmlc->parse_completed = true;
	}

	എങ്കിൽ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		തിരികെ true;
	}

	/* Give up processing if we've been aborted */
	എങ്കിൽ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ശൂന്യ);
		തിരികെ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	എങ്കിൽ (htmlc->encoding == ശൂന്യ) {
		സ്ഥിരം അക്ഷരം *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		എങ്കിൽ (encoding == ശൂന്യ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ശൂന്യ);
			തിരികെ false;
		}

		htmlc->encoding = strdup(encoding);
		എങ്കിൽ (htmlc->encoding == ശൂന്യ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ശൂന്യ);
			തിരികെ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (ശൂന്യം *) &html);
	എങ്കിൽ ((exc != DOM_NO_ERR) || (html == ശൂന്യ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ശൂന്യ);
		തിരികെ false;
	}

	exc = dom_കണ്ണി_get_കണ്ണി_name(html, &കണ്ണി_name);
	എങ്കിൽ ((exc != DOM_NO_ERR) ||
	    (കണ്ണി_name == ശൂന്യ) ||
	    (!dom_string_caseless_lwc_isequal(കണ്ണി_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ശൂന്യ);
		dom_കണ്ണി_unref(html);
		തിരികെ false;
	}
	dom_string_unref(കണ്ണി_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	വേണ്ടി (f = htmlc->forms; f != ശൂന്യ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		എങ്കിൽ (f->action == ശൂന്യ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} അല്ലെങ്കിൽ {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		എങ്കിൽ (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, ശൂന്യ);

			dom_കണ്ണി_unref(html);
			തിരികെ false;
		}

		സ്വതന്ത്രം(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		എങ്കിൽ (f->action == ശൂന്യ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ശൂന്യ);

			dom_കണ്ണി_unref(html);
			തിരികെ false;
		}

		/* Ensure each form has a document encoding */
		എങ്കിൽ (f->document_charset == ശൂന്യ) {
			f->document_charset = strdup(htmlc->encoding);
			എങ്കിൽ (f->document_charset == ശൂന്യ) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							ശൂന്യ);
				dom_കണ്ണി_unref(html);
				തിരികെ false;
			}
		}
	}

	dom_കണ്ണി_unref(html);

	എങ്കിൽ (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	തിരികെ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

സ്ഥിതം ശൂന്യം html_stop(ഘടന content *c)
{
	html_content *htmlc = (html_content *) c;

	തിരഞ്ഞെടുപ്പ് (c->status) {
	അവസ്ഥ CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		എങ്കിൽ (htmlc->jsthread != ശൂന്യ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		നിർത്തുക;

	അവസ്ഥ CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		എങ്കിൽ (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		നിർത്തുക;

	അവസ്ഥ CONTENT_STATUS_DONE:
		/* Nothing to do */
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		പ്രതിജ്ഞാ(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

സ്ഥിതം ശൂന്യം html_reformat(ഘടന content *c, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	html_content *htmlc = (html_content *) c;
	ഘടന box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.വേര്_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[ഇടത്_2] + layout->width +
		layout->padding[വലത്_2] + layout->border[വലത്_2].width +
		layout->margin[വലത്_2];
	c->height = layout->y + layout->padding[മുകൾ_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	എങ്കിൽ (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	എങ്കിൽ (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	എങ്കിൽ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_time = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

ശൂന്യം html_redraw_a_box(hlcache_handle *h, ഘടന box *box)
{
	പൂർണ്ണസംഖ്യ x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2],
			box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

ശൂന്യം html__redraw_a_box(ഘടന html_content *html, ഘടന box *box)
{
	പൂർണ്ണസംഖ്യ x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ഘടന content *)html, x, y,
			box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2],
			box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM]);
}

സ്ഥിതം ശൂന്യം html_destroy_frameset(ഘടന content_html_frames *frameset)
{
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ശൂന്യ;
	}
	എങ്കിൽ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ശൂന്യ;
	}
	എങ്കിൽ (frameset->children) {
		വേണ്ടി (i = 0; i < (frameset->rows * frameset->cols); i++) {
			എങ്കിൽ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ശൂന്യ;
			}
			എങ്കിൽ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ശൂന്യ;
			}
			എങ്കിൽ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ശൂന്യ;
	}
}

സ്ഥിതം ശൂന്യം html_destroy_iframe(ഘടന content_html_iframe *iframe)
{
	ഘടന content_html_iframe *next;
	next = iframe;
	ആകുമ്പോൾ ((iframe = next) != ശൂന്യ) {
		next = iframe->next;
		എങ്കിൽ (iframe->name)
			talloc_free(iframe->name);
		എങ്കിൽ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ശൂന്യ;
		}
		talloc_free(iframe);
	}
}


സ്ഥിതം ശൂന്യം html_free_layout(html_content *htmlc)
{
	എങ്കിൽ (htmlc->bctx != ശൂന്യ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

സ്ഥിതം ശൂന്യം html_destroy(ഘടന content *c)
{
	html_content *html = (html_content *) c;
	ഘടന form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	എങ്കിൽ (html->box_conversion_context != ശൂന്യ) {
		എങ്കിൽ (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	വേണ്ടി (f = html->forms; f != ശൂന്യ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	എങ്കിൽ (c->refresh)
		nsurl_unref(c->refresh);

	എങ്കിൽ (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	എങ്കിൽ (html->jsthread != ശൂന്യ) {
		js_destroythread(html->jsthread);
		html->jsthread = ശൂന്യ;
	}

	എങ്കിൽ (html->parser != ശൂന്യ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ശൂന്യ;
	}

	എങ്കിൽ (html->document != ശൂന്യ) {
		dom_കണ്ണി_unref(html->document);
		html->document = ശൂന്യ;
	}

	എങ്കിൽ (html->title != ശൂന്യ) {
		dom_കണ്ണി_unref(html->title);
		html->title = ശൂന്യ;
	}

	/* Free encoding */
	എങ്കിൽ (html->encoding != ശൂന്യ) {
		സ്വതന്ത്രം(html->encoding);
		html->encoding = ശൂന്യ;
	}

	/* Free base target */
	എങ്കിൽ (html->base_target != ശൂന്യ) {
		സ്വതന്ത്രം(html->base_target);
		html->base_target = ശൂന്യ;
	}

	/* Free frameset */
	എങ്കിൽ (html->frameset != ശൂന്യ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ശൂന്യ;
	}

	/* Free iframes */
	എങ്കിൽ (html->iframe != ശൂന്യ) {
		html_destroy_iframe(html->iframe);
		html->iframe = ശൂന്യ;
	}

	/* Destroy selection context */
	എങ്കിൽ (html->select_ctx != ശൂന്യ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ശൂന്യ;
	}

	lwc_string_unref(html->universal);
	html->universal = ശൂന്യ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ശൂന്യ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


സ്ഥിതം nserror html_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	പ്രതിജ്ഞാ(0 && "html_clone should never be called");

	തിരികെ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

സ്ഥിതം nserror
html_open(ഘടന content *c,
	  ഘടന browser_window *bw,
	  ഘടന content *page,
	  ഘടന object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->page = (html_content *) page;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	തിരികെ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

സ്ഥിതം nserror html_close(ഘടന content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ശൂന്യ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	എങ്കിൽ (htmlc->jsthread != ശൂന്യ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	തിരികെ ret;
}


/**
 * Return an HTML content's selection context
 */

സ്ഥിതം ശൂന്യം html_clear_selection(ഘടന content *c)
{
	html_content *html = (html_content *) c;

	തിരഞ്ഞെടുപ്പ് (html->selection_type) {
	അവസ്ഥ HTML_SELECTION_NONE:
		/* Nothing to do */
		പ്രതിജ്ഞാ(html->selection_owner.none == true);
		നിർത്തുക;
	അവസ്ഥ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		നിർത്തുക;
	അവസ്ഥ HTML_SELECTION_SELF:
		പ്രതിജ്ഞാ(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		നിർത്തുക;
	അവസ്ഥ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

സ്ഥിതം അക്ഷരം *html_get_selection(ഘടന content *c)
{
	html_content *html = (html_content *) c;

	തിരഞ്ഞെടുപ്പ് (html->selection_type) {
	അവസ്ഥ HTML_SELECTION_TEXTAREA:
		തിരികെ textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	അവസ്ഥ HTML_SELECTION_SELF:
		പ്രതിജ്ഞാ(html->selection_owner.none == false);
		തിരികെ selection_get_copy(html->sel);
	അവസ്ഥ HTML_SELECTION_CONTENT:
		തിരികെ content_get_selection(
				html->selection_owner.content->object);
	അവസ്ഥ HTML_SELECTION_NONE:
		/* Nothing to do */
		പ്രതിജ്ഞാ(html->selection_owner.none == true);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	തിരികെ ശൂന്യ;
}


/**
 * Get access to any content, link URLs and objects (images) currently
 * at the given (x, y) coordinates.
 *
 * \param[in] c html content to look inside
 * \param[in] x x-coordinate of point of interest
 * \param[in] y y-coordinate of point of interest
 * \param[out] data Positional features struct to be updated with any
 *             relevent content, or set to NULL if none.
 * \return NSERROR_OK on success else appropriate error code.
 */
സ്ഥിതം nserror
html_get_contextual_content(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			    ഘടന browser_window_features *data)
{
	html_content *html = (html_content *) c;

	ഘടന box *box = html->layout;
	ഘടന box *next;
	പൂർണ്ണസംഖ്യ box_x = 0, box_y = 0;

	ആകുമ്പോൾ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ശൂന്യ) {
		box = next;

		/* hidden boxes are ignored */
		എങ്കിൽ ((box->style != ശൂന്യ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			തുടരുക;
		}

		എങ്കിൽ (box->iframe) {
			ദശാംശം scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		എങ്കിൽ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		എങ്കിൽ (box->object)
			data->object = box->object;

		എങ്കിൽ (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		എങ്കിൽ (box->usemap) {
			സ്ഥിരം അക്ഷരം *target = ശൂന്യ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			എങ്കിൽ (url != ശൂന്യ)
				data->link = url;
		}
		എങ്കിൽ (box->gadget) {
			തിരഞ്ഞെടുപ്പ് (box->gadget->type) {
			അവസ്ഥ GADGET_TEXTBOX:
			അവസ്ഥ GADGET_TEXTAREA:
			അവസ്ഥ GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				നിർത്തുക;

			അവസ്ഥ GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				നിർത്തുക;

			സ്ഥിരസ്ഥിതി:
				data->form_features = CTX_FORM_NONE;
				നിർത്തുക;
			}
		}
	}
	തിരികെ NSERROR_OK;
}


/**
 * Scroll deepest thing within the content which can be scrolled at given point
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param scrx	number of px try to scroll something in x direction
 * \param scry	number of px try to scroll something in y direction
 * \return true iff scroll was consumed by something in the content
 */
സ്ഥിതം bool
html_scroll_at_point(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ scrx, പൂർണ്ണസംഖ്യ scry)
{
	html_content *html = (html_content *) c;

	ഘടന box *box = html->layout;
	ഘടന box *next;
	പൂർണ്ണസംഖ്യ box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ആകുമ്പോൾ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ശൂന്യ) {
		box = next;

		എങ്കിൽ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			തുടരുക;

		/* Pass into iframe */
		എങ്കിൽ (box->iframe) {
			ദശാംശം scale = browser_window_get_scale(box->iframe);

			എങ്കിൽ (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				തിരികെ true;
		}

		/* Pass into textarea widget */
		എങ്കിൽ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			തിരികെ true;

		/* Pass into object */
		എങ്കിൽ (box->object != ശൂന്യ && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			തിരികെ true;

		/* Handle box scrollbars */
		എങ്കിൽ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		എങ്കിൽ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		എങ്കിൽ (handled_scroll == true)
			തിരികെ true;
	}

	തിരികെ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
സ്ഥിതം ശൂന്യം html__dom_user_data_handler(dom_കണ്ണി_operation operation,
		dom_string *താക്കോൽ, ശൂന്യം *_data, ഘടന dom_കണ്ണി *src,
		ഘടന dom_കണ്ണി *dst)
{
	അക്ഷരം *oldfile;
	അക്ഷരം *data = (അക്ഷരം *)_data;

	എങ്കിൽ (!dom_string_isequal(corestring_dom_ns_താക്കോൽ_file_name_കണ്ണി_data,
				താക്കോൽ) || data == ശൂന്യ) {
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ് (operation) {
	അവസ്ഥ DOM_കണ്ണി_CLONED:
		എങ്കിൽ (dom_കണ്ണി_set_user_data(dst,
					   corestring_dom_ns_താക്കോൽ_file_name_കണ്ണി_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			എങ്കിൽ (oldfile != ശൂന്യ)
				സ്വതന്ത്രം(oldfile);
		}
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_RENAMED:
	അവസ്ഥ DOM_കണ്ണി_IMPORTED:
	അവസ്ഥ DOM_കണ്ണി_ADOPTED:
		നിർത്തുക;

	അവസ്ഥ DOM_കണ്ണി_DELETED:
		സ്വതന്ത്രം(data);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		പ്രതിജ്ഞാ(0);
	}
}

സ്ഥിതം ശൂന്യം html__set_file_gadget_filename(ഘടന content *c,
	ഘടന form_control *gadget, സ്ഥിരം അക്ഷരം *fn)
{
	nserror ret;
	അക്ഷരം *utf8_fn, *oldfile = ശൂന്യ;
	html_content *html = (html_content *)c;
	ഘടന box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	എങ്കിൽ (ret != NSERROR_OK) {
		പ്രതിജ്ഞാ(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		തിരികെ;
	}

	form_gadget_update_മൂല്യം(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	എങ്കിൽ (dom_കണ്ണി_set_user_data((dom_കണ്ണി *)file_box->gadget->കണ്ണി,
				   corestring_dom_ns_താക്കോൽ_file_name_കണ്ണി_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		എങ്കിൽ (oldfile != ശൂന്യ)
			സ്വതന്ത്രം(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

ശൂന്യം html_set_file_gadget_filename(ഘടന hlcache_handle *hl,
	ഘടന form_control *gadget, സ്ഥിരം അക്ഷരം *fn)
{
	തിരികെ html__set_file_gadget_filename(hlcache_handle_get_content(hl),
		gadget, fn);
}

/**
 * Drop a file onto a content at a particular point, or determine if a file
 * may be dropped onto the content at given point.
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param file	path to file to be dropped, or NULL to know if drop allowed
 * \return true iff file drop has been handled, or if drop possible (NULL file)
 */
സ്ഥിതം bool html_drop_file_at_point(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, അക്ഷരം *file)
{
	html_content *html = (html_content *) c;

	ഘടന box *box = html->layout;
	ഘടന box *next;
	ഘടന box *file_box = ശൂന്യ;
	ഘടന box *text_box = ശൂന്യ;
	പൂർണ്ണസംഖ്യ box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ആകുമ്പോൾ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ശൂന്യ) {
		box = next;

		എങ്കിൽ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			തുടരുക;

		എങ്കിൽ (box->iframe) {
			ദശാംശം scale = browser_window_get_scale(box->iframe);
			തിരികെ browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		എങ്കിൽ (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			തിരികെ true;

		എങ്കിൽ (box->gadget) {
			തിരഞ്ഞെടുപ്പ് (box->gadget->type) {
				അവസ്ഥ GADGET_FILE:
					file_box = box;
				നിർത്തുക;

				അവസ്ഥ GADGET_TEXTBOX:
				അവസ്ഥ GADGET_TEXTAREA:
				അവസ്ഥ GADGET_PASSWORD:
					text_box = box;
					നിർത്തുക;

				സ്ഥിരസ്ഥിതി:	/* appease compiler */
					നിർത്തുക;
			}
		}
	}

	എങ്കിൽ (!file_box && !text_box)
		/* No box capable of handling drop */
		തിരികെ false;

	എങ്കിൽ (file == ശൂന്യ)
		/* There is a box capable of handling drop here */
		തിരികെ true;

	/* Handle the drop */
	എങ്കിൽ (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} അല്ലെങ്കിൽ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ശൂന്യ;
		അക്ഷരം *buffer;
		അക്ഷരം *utf8_buff;
		nserror ret;
		unsigned പൂർണ്ണസംഖ്യ size;
		പൂർണ്ണസംഖ്യ bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		എങ്കിൽ (fp == ശൂന്യ) {
			/* Couldn't open file, but drop was for us */
			തിരികെ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		എങ്കിൽ ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			തിരികെ true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		എങ്കിൽ (buffer == ശൂന്യ) {
			/* No memory, but drop was for us */
			fclose(fp);
			തിരികെ true;
		}

		/* Stick file into buffer */
		എങ്കിൽ (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			സ്വതന്ത്രം(buffer);
			fclose(fp);
			തിരികെ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		എങ്കിൽ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			പ്രതിജ്ഞാ(ret != NSERROR_BAD_ENCODING);
			സ്വതന്ത്രം(buffer);
			തിരികെ true;
		}

		/* Done with buffer */
		സ്വതന്ത്രം(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		സ്വതന്ത്രം(utf8_buff);
	}

	തിരികെ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
സ്ഥിതം nserror
html_debug(ഘടന content *c, എണ്ണൽ content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	തിരികെ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
സ്ഥിതം nserror
html_debug_dump(ഘടന content *c, FILE *f, എണ്ണൽ content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_കണ്ണി *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	പ്രതിജ്ഞാ(htmlc != ശൂന്യ);

	എങ്കിൽ (op == CONTENT_DEBUG_RENDER) {
		പ്രതിജ്ഞാ(htmlc->layout != ശൂന്യ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (htmlc->document == ശൂന്യ) {
			NSLOG(netsurf, INFO, "No document to dump");
			തിരികെ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (ശൂന്യം *) &html);
		എങ്കിൽ ((exc != DOM_NO_ERR) || (html == ശൂന്യ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			തിരികെ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_കണ്ണി_unref(html);
	}

	തിരികെ ret;
}


#എങ്കിൽ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

സ്ഥിതം ശൂന്യം
html_dump_frameset(ഘടന content_html_frames *frame, unsigned പൂർണ്ണസംഖ്യ depth)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ row, col, index;
	സ്ഥിരം അക്ഷരം *unit[] = {"px", "%", "*"};
	സ്ഥിരം അക്ഷരം *scrolling[] = {"auto", "yes", "no"};

	പ്രതിജ്ഞാ(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.മൂല്യം, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.മൂല്യം,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	എങ്കിൽ (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	എങ്കിൽ (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	എങ്കിൽ (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	എങ്കിൽ (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned പൂർണ്ണസംഖ്യ) frame->border_colour);

	fprintf(stderr, "\n");

	എങ്കിൽ (frame->children) {
		വേണ്ടി (row = 0; row != frame->rows; row++) {
			വേണ്ടി (col = 0; col != frame->cols; col++) {
				വേണ്ടി (i = 0; i != depth; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", row, col);
				index = (row * frame->cols) + col;
				html_dump_frameset(&frame->children[index],
						depth + 1);
			}
		}
	}
}

#endif

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->document;
}

/**
 * Retrieve box tree
 *
 * \param h  HTML content to retrieve tree from
 * \return Pointer to box tree
 *
 * \todo This API must die, as must all use of the box tree outside of
 *         HTML content handler
 */
ഘടന box *html_get_box_വൃക്ഷം(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
സ്ഥിതം സ്ഥിരം അക്ഷരം *html_encoding(സ്ഥിരം ഘടന content *c, എണ്ണൽ content_encoding_type op)
{
	html_content *html = (html_content *) c;
	സ്ഥിതം അക്ഷരം enc_token[10] = "Encoding0";

	പ്രതിജ്ഞാ(html != ശൂന്യ);

	എങ്കിൽ (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		തിരികെ messages_get(enc_token);
	}

	തിരികെ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ഘടന content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ഘടന content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_base_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
സ്ഥിരം അക്ഷരം *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	പ്രതിജ്ഞാ(c != ശൂന്യ);

	തിരികെ c->base_target;
}


/**
 * Retrieve layout coordinates of box with given id
 *
 * \param h        HTML document to search
 * \param frag_id  String containing an element id
 * \param x        Updated to global x coord iff id found
 * \param y        Updated to global y coord iff id found
 * \return  true iff id found
 */
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y)
{
	ഘടന box *pos;
	ഘടന box *layout;

	എങ്കിൽ (content_get_type(h) != CONTENT_HTML)
		തിരികെ false;

	layout = html_get_box_വൃക്ഷം(h);

	എങ്കിൽ ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		തിരികെ true;
	}
	തിരികെ false;
}

bool html_exec(ഘടന content *c, സ്ഥിരം അക്ഷരം *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_കണ്ണി;
	dom_string *dom_src;
	dom_text *text_കണ്ണി;
	dom_കണ്ണി *spare_കണ്ണി;
	dom_html_script_element *script_കണ്ണി;

	എങ്കിൽ (htmlc->document == ശൂന്യ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		പോകുക out_no_string;
	}

	err = dom_string_create((സ്ഥിരം uint8_t *)src, srclen, &dom_src);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		പോകുക out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_കണ്ണി);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		പോകുക out_no_body;
	}

	err = dom_document_create_text_കണ്ണി(htmlc->document, dom_src, &text_കണ്ണി);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		പോകുക out_no_text_കണ്ണി;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_കണ്ണി);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		പോകുക out_no_script_കണ്ണി;
	}

	err = dom_കണ്ണി_append_child(script_കണ്ണി, text_കണ്ണി, &spare_കണ്ണി);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		പോകുക out_unparented;
	}
	dom_കണ്ണി_unref(spare_കണ്ണി); /* We do not need the spare ref at all */

	err = dom_കണ്ണി_append_child(body_കണ്ണി, script_കണ്ണി, &spare_കണ്ണി);
	എങ്കിൽ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		പോകുക out_unparented;
	}
	dom_കണ്ണി_unref(spare_കണ്ണി); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_കണ്ണി_get_parent_കണ്ണി(script_കണ്ണി, &spare_കണ്ണി);
	എങ്കിൽ (err == DOM_NO_ERR && spare_കണ്ണി != ശൂന്യ) {
		dom_കണ്ണി *second_spare;
		err = dom_കണ്ണി_remove_child(spare_കണ്ണി, script_കണ്ണി, &second_spare);
		എങ്കിൽ (err == DOM_NO_ERR) {
			dom_കണ്ണി_unref(second_spare);
		}
		dom_കണ്ണി_unref(spare_കണ്ണി);
	}

out_unparented:
	dom_കണ്ണി_unref(script_കണ്ണി);
out_no_script_കണ്ണി:
	dom_കണ്ണി_unref(text_കണ്ണി);
out_no_text_കണ്ണി:
	dom_കണ്ണി_unref(body_കണ്ണി);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	തിരികെ result;
}

/* See \ref content_saw_insecure_objects */
സ്ഥിതം bool
html_saw_insecure_objects(ഘടന content *c)
{
	html_content *htmlc = (html_content *)c;
	ഘടന content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ആകുമ്പോൾ (obj != ശൂന്യ) {
		എങ്കിൽ (obj->content != ശൂന്യ) {
			എങ്കിൽ (content_saw_insecure_objects(obj->content))
				തിരികെ true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	എങ്കിൽ (html_saw_insecure_scripts(htmlc)) {
		തിരികെ true;
	}

	/* Now check stylesheets */
	എങ്കിൽ (html_css_saw_insecure_stylesheets(htmlc)) {
		തിരികെ true;
	}

	തിരികെ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
സ്ഥിതം content_type html_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_HTML;
}


സ്ഥിതം ശൂന്യം html_fini(ശൂന്യം)
{
	html_css_fini();
}

/**
 * Finds all occurrences of a given string in an html box
 *
 * \param pattern   the string pattern to search for
 * \param p_len     pattern length
 * \param cur       pointer to the current box
 * \param case_sens whether to perform a case sensitive search
 * \param context   The search context to add the entry to.
 * \return true on success, false on memory allocation failure
 */
സ്ഥിതം nserror
find_occurrences_html_box(സ്ഥിരം അക്ഷരം *pattern,
			  പൂർണ്ണസംഖ്യ p_len,
			  ഘടന box *cur,
			  bool case_sens,
			  ഘടന textsearch_context *context)
{
	ഘടന box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	എങ്കിൽ (!cur->object && cur->text) {
		സ്ഥിരം അക്ഷരം *text = cur->text;
		unsigned length = cur->length;

		ആകുമ്പോൾ (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			സ്ഥിരം അക്ഷരം *new_text;
			സ്ഥിരം അക്ഷരം *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			എങ്കിൽ (!pos)
				നിർത്തുക;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	വേണ്ടി (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ res;
		}
	}

	തിരികെ res;
}

/**
 * Finds all occurrences of a given string in the html box tree
 *
 * \param pattern   the string pattern to search for
 * \param p_len     pattern length
 * \param c The content to search
 * \param csens whether to perform a case sensitive search
 * \param context   The search context to add the entry to.
 * \return true on success, false on memory allocation failure
 */
സ്ഥിതം nserror
html_textsearch_find(ഘടന content *c,
		     ഘടന textsearch_context *context,
		     സ്ഥിരം അക്ഷരം *pattern,
		     പൂർണ്ണസംഖ്യ p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	എങ്കിൽ (html->layout == ശൂന്യ) {
		തിരികെ NSERROR_INVALID;
	}

	തിരികെ find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


സ്ഥിതം nserror
html_textsearch_bounds(ഘടന content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       ഘടന box *start_box,
		       ഘടന box *end_box,
		       ഘടന rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	തിരികെ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
സ്ഥിതം സ്ഥിരം content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_data = html_process_data,
	.data_complete = html_convert,
	.reformat = html_reformat,
	.destroy = html_destroy,
	.stop = html_stop,
	.mouse_track = html_mouse_track,
	.mouse_action = html_mouse_action,
	.keypress = html_keypress,
	.redraw = html_redraw,
	.open = html_open,
	.close = html_close,
	.get_selection = html_get_selection,
	.clear_selection = html_clear_selection,
	.get_contextual_content = html_get_contextual_content,
	.scroll_at_point = html_scroll_at_point,
	.drop_file_at_point = html_drop_file_at_point,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_find = html_textsearch_find,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_end = html_textselection_get_end,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(ശൂന്യം)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	എങ്കിൽ (error != NSERROR_OK)
		പോകുക error;

	വേണ്ടി (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		എങ്കിൽ (error != NSERROR_OK)
			പോകുക error;
	}

	തിരികെ NSERROR_OK;

error:
	html_fini();

	തിരികെ error;
}
