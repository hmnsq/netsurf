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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/libdom.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/utf8.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/dom_event.h"
#ಅನ್ತರ್ಭಾವ "html/css.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/html_save.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/imagemap.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *html_types[] = {
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
ಸ್ಥಿರ bool fire_dom_event(dom_event *event, dom_ಗಂಟು *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	ಯದಿ (exc != DOM_NO_ERR) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_ಗಂಟು *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	ಯದಿ (exc != DOM_NO_ERR) ಪ್ರತ್ಯಾಗಮ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (ಪೂರ್ಣಾಙ್ಕ)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	ಪ್ರತ್ಯಾಗಮ result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_ಗಂಟು *target,
		bool bubbles, bool cancelable, uint32_t ಕೀಲಿ)
{
	bool is_special = ಕೀಲಿ <= 0x001F || (0x007F <= ಕೀಲಿ && ಕೀಲಿ <= 0x009F);
	dom_string *dom_ಕೀಲಿ = ರಿಕ್ತ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	ಯದಿ (is_special) {
		ವಿಕಲ್ಪ (ಕೀಲಿ) {
		ಪಕ್ಷ NS_ಕೀಲಿ_ESCAPE:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Escape);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_ಎಡ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಎಡ);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_ಬಲ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಬಲ);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_UP:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_ArrowUp);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_DOWN:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_ArrowDown);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_PAGE_UP:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_PageUp);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_PAGE_DOWN:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_PageDown);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_TEXT_START:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Home);
			ಭಙ್ಗ;
		ಪಕ್ಷ NS_ಕೀಲಿ_TEXT_END:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_End);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			dom_ಕೀಲಿ = ರಿಕ್ತ;
			ಭಙ್ಗ;
		}
	} ಅನ್ಯಥಾ {
		ವರ್ಣ utf8[6];
		size_t length = utf8_from_ucs4(ಕೀಲಿ, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((ಧ್ರುವ uint8_t *)utf8, strlen(utf8),
				&dom_ಕೀಲಿ);
		ಯದಿ (exc != DOM_NO_ERR) {
			ಪ್ರತ್ಯಾಗಮ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ರಿಕ್ತ,
			dom_ಕೀಲಿ, ರಿಕ್ತ, DOM_ಕೀಲಿ_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_ಕೀಲಿ);
	ಯದಿ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(ಪೂರ್ಣಾಙ್ಕ)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	ಪ್ರತ್ಯಾಗಮ result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_ಗಂಟು *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ರಿಕ್ತ;

	/* Clean up and report error if unsuccessful or aborted */
	ಯದಿ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ಯದಿ (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, ರಿಕ್ತ);
		} ಅನ್ಯಥಾ {
			content_broadcast_error(&c->base, NSERROR_STOPPED, ರಿಕ್ತ);
		}

		content_set_error(&c->base);
		ಪ್ರತ್ಯಾಗಮ;
	}


#ಯದಿ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#ಯದಿ ALWAYS_DUMP_FRAMESET
	ಯದಿ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (ಶೂನ್ಯ *) &html);
	ಯದಿ ((exc != DOM_NO_ERR) || (html == ರಿಕ್ತ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, ರಿಕ್ತ);
		content_set_error(&c->base);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	ಯದಿ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, ರಿಕ್ತ);
		content_set_error(&c->base);
		dom_ಗಂಟು_unref(html);
		ಪ್ರತ್ಯಾಗಮ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ರಿಕ್ತ;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_ಗಂಟು_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	ವಿಕಲ್ಪ (content__get_status(&html->base)) {
	ಪಕ್ಷ CONTENT_STATUS_READY:
		ಯದಿ (html->base.active == 0) {
			content_set_done(&html->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CONTENT_STATUS_DONE:
		/* fallthrough */
	ಪಕ್ಷ CONTENT_STATUS_LOADING:
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	ಸ್ವಭಾವ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_UNKNOWN;
}


ಸ್ಥಿರ ಶೂನ್ಯ html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_ಸಂದೇಶ_GETDIMS, &ಸಂದೇಶ_data);


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
ಶೂನ್ಯ html_finish_conversion(html_content *htmlc)
{
	ಸಙ್ಘ content_ಸಂದೇಶ_data ಸಂದೇಶ_data;
	dom_exception exc; /* returned by libdom functions */
	dom_ಗಂಟು *html;
	nserror error;

	/* Bail out if we've been aborted */
	ಯದಿ (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ರಿಕ್ತ);
		content_set_error(&htmlc->base);
		ಪ್ರತ್ಯಾಗಮ;
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
	ಯದಿ (htmlc->select_ctx != ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ಯದಿ (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, ರಿಕ್ತ);
		content_set_error(&htmlc->base);
		ಪ್ರತ್ಯಾಗಮ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ಯದಿ (htmlc->jsthread != ರಿಕ್ತ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ರಿಕ್ತ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	ಸಂದೇಶ_data.explicit_status_text = ರಿಕ್ತ;
	content_broadcast(&htmlc->base, CONTENT_ಸಂದೇಶ_STATUS, &ಸಂದೇಶ_data);

	exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
	ಯದಿ ((exc != DOM_NO_ERR) || (html == ರಿಕ್ತ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ರಿಕ್ತ);
		content_set_error(&htmlc->base);
		ಪ್ರತ್ಯಾಗಮ;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	ಯದಿ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_ಗಂಟು_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, ರಿಕ್ತ);
		content_set_error(&htmlc->base);
		ಪ್ರತ್ಯಾಗಮ;
	}

	dom_ಗಂಟು_unref(html);
}


ಸ್ಥಿರ ಶೂನ್ಯ
html_document_user_data_handler(dom_ಗಂಟು_operation operation,
				dom_string *ಕೀಲಿ, ಶೂನ್ಯ *data,
				ಸಂರಚನಾ dom_ಗಂಟು *src,
				ಸಂರಚನಾ dom_ಗಂಟು *dst)
{
	ಯದಿ (dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_html_content_data,
			       ಕೀಲಿ) == false || data == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ (operation) {
	ಪಕ್ಷ DOM_ಗಂಟು_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_ಗಂಟು_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_ಗಂಟು_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_ಗಂಟು_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ಭಙ್ಗ;
	ಪಕ್ಷ DOM_ಗಂಟು_DELETED:
		/* This is the only path I expect */
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}


ಸ್ಥಿರ nserror
html_create_html_data(html_content *c, ಧ್ರುವ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	ಶೂನ್ಯ *old_ಗಂಟು_data;
	ಧ್ರುವ ವರ್ಣ *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = ರಿಕ್ತ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ರಿಕ್ತ;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = ರಿಕ್ತ;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = ರಿಕ್ತ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ರಿಕ್ತ;
	c->bctx = ರಿಕ್ತ;
	c->layout = ರಿಕ್ತ;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_ಎಣಿಕೆ = 0;
	c->stylesheets = ರಿಕ್ತ;
	c->select_ctx = ರಿಕ್ತ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ರಿಕ್ತ;
	c->ಅಂಕೆ_objects = 0;
	c->object_list = ರಿಕ್ತ;
	c->forms = ರಿಕ್ತ;
	c->imagemaps = ರಿಕ್ತ;
	c->bw = ರಿಕ್ತ;
	c->frameset = ರಿಕ್ತ;
	c->iframe = ರಿಕ್ತ;
	c->page = ರಿಕ್ತ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_ಎಣಿಕೆ = 0;
	c->scripts = ರಿಕ್ತ;
	c->jsthread = ರಿಕ್ತ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	ಯದಿ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	c->sel = selection_create((ಸಂರಚನಾ content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	ಯದಿ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		ಯದಿ (c->encoding == ರಿಕ್ತ) {
			lwc_string_unref(c->universal);
			c->universal = ರಿಕ್ತ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ರಿಕ್ತ;
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.ಸಂದೇಶ = ರಿಕ್ತ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ಯದಿ ((error != DOM_HUBBUB_OK) && (c->encoding != ರಿಕ್ತ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		ಮುಕ್ತ(c->encoding);
		c->encoding = ರಿಕ್ತ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ಯದಿ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = ರಿಕ್ತ;

		lwc_string_unref(c->universal);
		c->universal = ರಿಕ್ತ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ರಿಕ್ತ;

		ಪ್ರತ್ಯಾಗಮ libdom_hubbub_error_to_nserror(error);
	}

	err = dom_ಗಂಟು_set_user_data(c->document,
				     corestring_dom_ns_ಕೀಲಿ_html_content_data,
				     c, html_document_user_data_handler,
				     (ಶೂನ್ಯ *) &old_ಗಂಟು_data);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ರಿಕ್ತ;
		nsurl_unref(c->base_url);
		c->base_url = ರಿಕ್ತ;

		lwc_string_unref(c->universal);
		c->universal = ರಿಕ್ತ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ರಿಕ್ತ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಪ್ರತಿಜ್ಞಾ(old_ಗಂಟು_data == ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ಸ್ಥಿರ nserror
html_create(ಧ್ರುವ content_handler *handler,
	    lwc_string *imime_type,
	    ಧ್ರುವ http_parameter *params,
	    llcache_handle *llcache,
	    ಧ್ರುವ ವರ್ಣ *fallback_charset,
	    bool quirks,
	    ಸಂರಚನಾ content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, ಪರಿಮಾಣ(html_content));
	ಯದಿ (html == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(html);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = html_create_html_data(html, params);
	ಯದಿ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ರಿಕ್ತ);
		ಮುಕ್ತ(html);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = html_css_new_stylesheets(html);
	ಯದಿ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ರಿಕ್ತ);
		ಮುಕ್ತ(html);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) html;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}



ಸ್ಥಿರ nserror
html_process_encoding_change(ಸಂರಚನಾ content *c,
			     ಧ್ರುವ ವರ್ಣ *data,
			     unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ಧ್ರುವ ವರ್ಣ *encoding;
	ಧ್ರುವ uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	ಯದಿ (encoding == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (html->encoding != ರಿಕ್ತ) {
		ಮುಕ್ತ(html->encoding);
		html->encoding = ರಿಕ್ತ;
	}

	html->encoding = strdup(encoding);
	ಯದಿ (html->encoding == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ರಿಕ್ತ;

	ಯದಿ (html->document != ರಿಕ್ತ) {
		dom_ಗಂಟು_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.ಸಂದೇಶ = ರಿಕ್ತ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ಯದಿ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		ಮುಕ್ತ(html->encoding);
		html->encoding = strdup("Windows-1252");
		ಯದಿ (html->encoding == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ಯದಿ (error != DOM_HUBBUB_OK) {
			ಪ್ರತ್ಯಾಗಮ libdom_hubbub_error_to_nserror(error);
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

	ಪ್ರತ್ಯಾಗಮ libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ಸ್ಥಿರ bool
html_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data, unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ಧ್ರುವ uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	ಯದಿ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	ಯದಿ (err != NSERROR_OK) {
		content_broadcast_error(c, err, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_convert(ಸಂರಚನಾ content *c)
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
	ಯದಿ (exc == DOM_NO_ERR) {
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
	ಯದಿ (html_can_begin_conversion(htmlc)) {
		ಪ್ರತ್ಯಾಗಮ html_begin_conversion(htmlc);
	}
	ಪ್ರತ್ಯಾಗಮ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;

	/* Cannot begin conversion if we're still fetching stuff */
	ಯದಿ (htmlc->base.active != 0)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (i = 0; i != htmlc->stylesheet_ಎಣಿಕೆ; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ಯದಿ (htmlc->stylesheets[i].modified)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* All is good, begin */
	ಪ್ರತ್ಯಾಗಮ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_ಗಂಟು *html;
	nserror ns_error;
	ಸಂರಚನಾ form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *ಗಂಟು_name = ರಿಕ್ತ;
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
	ಯದಿ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ಯದಿ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ಪ್ರತ್ಯಾಗಮ true;
		}
		ಯದಿ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						ರಿಕ್ತ);

			ಪ್ರತ್ಯಾಗಮ false;
		}
		htmlc->parse_completed = true;
	}

	ಯದಿ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Give up processing if we've been aborted */
	ಯದಿ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ಯದಿ (htmlc->encoding == ರಿಕ್ತ) {
		ಧ್ರುವ ವರ್ಣ *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		ಯದಿ (encoding == ರಿಕ್ತ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ರಿಕ್ತ);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		htmlc->encoding = strdup(encoding);
		ಯದಿ (htmlc->encoding == ರಿಕ್ತ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ರಿಕ್ತ);
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
	ಯದಿ ((exc != DOM_NO_ERR) || (html == ರಿಕ್ತ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(html, &ಗಂಟು_name);
	ಯದಿ ((exc != DOM_NO_ERR) ||
	    (ಗಂಟು_name == ರಿಕ್ತ) ||
	    (!dom_string_caseless_lwc_isequal(ಗಂಟು_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ರಿಕ್ತ);
		dom_ಗಂಟು_unref(html);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	dom_string_unref(ಗಂಟು_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	ಪ್ರತಿ (f = htmlc->forms; f != ರಿಕ್ತ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ಯದಿ (f->action == ರಿಕ್ತ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} ಅನ್ಯಥಾ {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		ಯದಿ (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, ರಿಕ್ತ);

			dom_ಗಂಟು_unref(html);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಮುಕ್ತ(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ಯದಿ (f->action == ರಿಕ್ತ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ರಿಕ್ತ);

			dom_ಗಂಟು_unref(html);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		/* Ensure each form has a document encoding */
		ಯದಿ (f->document_charset == ರಿಕ್ತ) {
			f->document_charset = strdup(htmlc->encoding);
			ಯದಿ (f->document_charset == ರಿಕ್ತ) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							ರಿಕ್ತ);
				dom_ಗಂಟು_unref(html);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}

	dom_ಗಂಟು_unref(html);

	ಯದಿ (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_stop(ಸಂರಚನಾ content *c)
{
	html_content *htmlc = (html_content *) c;

	ವಿಕಲ್ಪ (c->status) {
	ಪಕ್ಷ CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ಯದಿ (htmlc->jsthread != ರಿಕ್ತ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ಯದಿ (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		ಭಙ್ಗ;

	ಪಕ್ಷ CONTENT_STATUS_DONE:
		/* Nothing to do */
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_reformat(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	html_content *htmlc = (html_content *) c;
	ಸಂರಚನಾ box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ಬೇರು_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[ಎಡ_2] + layout->width +
		layout->padding[ಬಲ_2] + layout->border[ಬಲ_2].width +
		layout->margin[ಬಲ_2];
	c->height = layout->y + layout->padding[ತುದಿ_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ಯದಿ (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	ಯದಿ (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ಯದಿ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

ಶೂನ್ಯ html_redraw_a_box(hlcache_handle *h, ಸಂರಚನಾ box *box)
{
	ಪೂರ್ಣಾಙ್ಕ x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2],
			box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

ಶೂನ್ಯ html__redraw_a_box(ಸಂರಚನಾ html_content *html, ಸಂರಚನಾ box *box)
{
	ಪೂರ್ಣಾಙ್ಕ x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ಸಂರಚನಾ content *)html, x, y,
			box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2],
			box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM]);
}

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy_frameset(ಸಂರಚನಾ content_html_frames *frameset)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ರಿಕ್ತ;
	}
	ಯದಿ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ರಿಕ್ತ;
	}
	ಯದಿ (frameset->children) {
		ಪ್ರತಿ (i = 0; i < (frameset->rows * frameset->cols); i++) {
			ಯದಿ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ರಿಕ್ತ;
			}
			ಯದಿ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ರಿಕ್ತ;
			}
			ಯದಿ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ರಿಕ್ತ;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy_iframe(ಸಂರಚನಾ content_html_iframe *iframe)
{
	ಸಂರಚನಾ content_html_iframe *next;
	next = iframe;
	ಯಾವತ್ ((iframe = next) != ರಿಕ್ತ) {
		next = iframe->next;
		ಯದಿ (iframe->name)
			talloc_free(iframe->name);
		ಯದಿ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ರಿಕ್ತ;
		}
		talloc_free(iframe);
	}
}


ಸ್ಥಿರ ಶೂನ್ಯ html_free_layout(html_content *htmlc)
{
	ಯದಿ (htmlc->bctx != ರಿಕ್ತ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy(ಸಂರಚನಾ content *c)
{
	html_content *html = (html_content *) c;
	ಸಂರಚನಾ form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ಯದಿ (html->box_conversion_context != ರಿಕ್ತ) {
		ಯದಿ (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	ಪ್ರತಿ (f = html->forms; f != ರಿಕ್ತ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ಯದಿ (c->refresh)
		nsurl_unref(c->refresh);

	ಯದಿ (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ಯದಿ (html->jsthread != ರಿಕ್ತ) {
		js_destroythread(html->jsthread);
		html->jsthread = ರಿಕ್ತ;
	}

	ಯದಿ (html->parser != ರಿಕ್ತ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ರಿಕ್ತ;
	}

	ಯದಿ (html->document != ರಿಕ್ತ) {
		dom_ಗಂಟು_unref(html->document);
		html->document = ರಿಕ್ತ;
	}

	ಯದಿ (html->title != ರಿಕ್ತ) {
		dom_ಗಂಟು_unref(html->title);
		html->title = ರಿಕ್ತ;
	}

	/* Free encoding */
	ಯದಿ (html->encoding != ರಿಕ್ತ) {
		ಮುಕ್ತ(html->encoding);
		html->encoding = ರಿಕ್ತ;
	}

	/* Free base target */
	ಯದಿ (html->base_target != ರಿಕ್ತ) {
		ಮುಕ್ತ(html->base_target);
		html->base_target = ರಿಕ್ತ;
	}

	/* Free frameset */
	ಯದಿ (html->frameset != ರಿಕ್ತ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ರಿಕ್ತ;
	}

	/* Free iframes */
	ಯದಿ (html->iframe != ರಿಕ್ತ) {
		html_destroy_iframe(html->iframe);
		html->iframe = ರಿಕ್ತ;
	}

	/* Destroy selection context */
	ಯದಿ (html->select_ctx != ರಿಕ್ತ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ರಿಕ್ತ;
	}

	lwc_string_unref(html->universal);
	html->universal = ರಿಕ್ತ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ರಿಕ್ತ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ಸ್ಥಿರ nserror html_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ಪ್ರತಿಜ್ಞಾ(0 && "html_clone should never be called");

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ಸ್ಥಿರ nserror
html_open(ಸಂರಚನಾ content *c,
	  ಸಂರಚನಾ browser_window *bw,
	  ಸಂರಚನಾ content *page,
	  ಸಂರಚನಾ object_params *params)
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

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ಸ್ಥಿರ nserror html_close(ಸಂರಚನಾ content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ರಿಕ್ತ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ಯದಿ (htmlc->jsthread != ರಿಕ್ತ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}


/**
 * Return an HTML content's selection context
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_clear_selection(ಸಂರಚನಾ content *c)
{
	html_content *html = (html_content *) c;

	ವಿಕಲ್ಪ (html->selection_type) {
	ಪಕ್ಷ HTML_SELECTION_NONE:
		/* Nothing to do */
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == true);
		ಭಙ್ಗ;
	ಪಕ್ಷ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		ಭಙ್ಗ;
	ಪಕ್ಷ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ಭಙ್ಗ;
	ಪಕ್ಷ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ಸ್ಥಿರ ವರ್ಣ *html_get_selection(ಸಂರಚನಾ content *c)
{
	html_content *html = (html_content *) c;

	ವಿಕಲ್ಪ (html->selection_type) {
	ಪಕ್ಷ HTML_SELECTION_TEXTAREA:
		ಪ್ರತ್ಯಾಗಮ textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	ಪಕ್ಷ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == false);
		ಪ್ರತ್ಯಾಗಮ selection_get_copy(html->sel);
	ಪಕ್ಷ HTML_SELECTION_CONTENT:
		ಪ್ರತ್ಯಾಗಮ content_get_selection(
				html->selection_owner.content->object);
	ಪಕ್ಷ HTML_SELECTION_NONE:
		/* Nothing to do */
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == true);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
ಸ್ಥಿರ nserror
html_get_contextual_content(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			    ಸಂರಚನಾ browser_window_features *data)
{
	html_content *html = (html_content *) c;

	ಸಂರಚನಾ box *box = html->layout;
	ಸಂರಚನಾ box *next;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0, box_y = 0;

	ಯಾವತ್ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ರಿಕ್ತ) {
		box = next;

		/* hidden boxes are ignored */
		ಯದಿ ((box->style != ರಿಕ್ತ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ಅನುವರ್ತ;
		}

		ಯದಿ (box->iframe) {
			ಭಿನ್ನ scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		ಯದಿ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		ಯದಿ (box->object)
			data->object = box->object;

		ಯದಿ (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		ಯದಿ (box->usemap) {
			ಧ್ರುವ ವರ್ಣ *target = ರಿಕ್ತ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ಯದಿ (url != ರಿಕ್ತ)
				data->link = url;
		}
		ಯದಿ (box->gadget) {
			ವಿಕಲ್ಪ (box->gadget->type) {
			ಪಕ್ಷ GADGET_TEXTBOX:
			ಪಕ್ಷ GADGET_TEXTAREA:
			ಪಕ್ಷ GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				ಭಙ್ಗ;

			ಪಕ್ಷ GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				ಭಙ್ಗ;

			ಸ್ವಭಾವ:
				data->form_features = CTX_FORM_NONE;
				ಭಙ್ಗ;
			}
		}
	}
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ bool
html_scroll_at_point(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ scrx, ಪೂರ್ಣಾಙ್ಕ scry)
{
	html_content *html = (html_content *) c;

	ಸಂರಚನಾ box *box = html->layout;
	ಸಂರಚನಾ box *next;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ಯಾವತ್ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ರಿಕ್ತ) {
		box = next;

		ಯದಿ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ಅನುವರ್ತ;

		/* Pass into iframe */
		ಯದಿ (box->iframe) {
			ಭಿನ್ನ scale = browser_window_get_scale(box->iframe);

			ಯದಿ (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Pass into textarea widget */
		ಯದಿ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			ಪ್ರತ್ಯಾಗಮ true;

		/* Pass into object */
		ಯದಿ (box->object != ರಿಕ್ತ && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ಪ್ರತ್ಯಾಗಮ true;

		/* Handle box scrollbars */
		ಯದಿ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ಯದಿ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ಯದಿ (handled_scroll == true)
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ಸ್ಥಿರ ಶೂನ್ಯ html__dom_user_data_handler(dom_ಗಂಟು_operation operation,
		dom_string *ಕೀಲಿ, ಶೂನ್ಯ *_data, ಸಂರಚನಾ dom_ಗಂಟು *src,
		ಸಂರಚನಾ dom_ಗಂಟು *dst)
{
	ವರ್ಣ *oldfile;
	ವರ್ಣ *data = (ವರ್ಣ *)_data;

	ಯದಿ (!dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_data,
				ಕೀಲಿ) || data == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ (operation) {
	ಪಕ್ಷ DOM_ಗಂಟು_CLONED:
		ಯದಿ (dom_ಗಂಟು_set_user_data(dst,
					   corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			ಯದಿ (oldfile != ರಿಕ್ತ)
				ಮುಕ್ತ(oldfile);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_RENAMED:
	ಪಕ್ಷ DOM_ಗಂಟು_IMPORTED:
	ಪಕ್ಷ DOM_ಗಂಟು_ADOPTED:
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_ಗಂಟು_DELETED:
		ಮುಕ್ತ(data);
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ html__set_file_gadget_filename(ಸಂರಚನಾ content *c,
	ಸಂರಚನಾ form_control *gadget, ಧ್ರುವ ವರ್ಣ *fn)
{
	nserror ret;
	ವರ್ಣ *utf8_fn, *oldfile = ರಿಕ್ತ;
	html_content *html = (html_content *)c;
	ಸಂರಚನಾ box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	ಯದಿ (ret != NSERROR_OK) {
		ಪ್ರತಿಜ್ಞಾ(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ಪ್ರತ್ಯಾಗಮ;
	}

	form_gadget_update_ಮೌಲ್ಯ(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ಯದಿ (dom_ಗಂಟು_set_user_data((dom_ಗಂಟು *)file_box->gadget->ಗಂಟು,
				   corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		ಯದಿ (oldfile != ರಿಕ್ತ)
			ಮುಕ್ತ(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

ಶೂನ್ಯ html_set_file_gadget_filename(ಸಂರಚನಾ hlcache_handle *hl,
	ಸಂರಚನಾ form_control *gadget, ಧ್ರುವ ವರ್ಣ *fn)
{
	ಪ್ರತ್ಯಾಗಮ html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
ಸ್ಥಿರ bool html_drop_file_at_point(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ವರ್ಣ *file)
{
	html_content *html = (html_content *) c;

	ಸಂರಚನಾ box *box = html->layout;
	ಸಂರಚನಾ box *next;
	ಸಂರಚನಾ box *file_box = ರಿಕ್ತ;
	ಸಂರಚನಾ box *text_box = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ಯಾವತ್ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ರಿಕ್ತ) {
		box = next;

		ಯದಿ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ಅನುವರ್ತ;

		ಯದಿ (box->iframe) {
			ಭಿನ್ನ scale = browser_window_get_scale(box->iframe);
			ಪ್ರತ್ಯಾಗಮ browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ಯದಿ (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			ಪ್ರತ್ಯಾಗಮ true;

		ಯದಿ (box->gadget) {
			ವಿಕಲ್ಪ (box->gadget->type) {
				ಪಕ್ಷ GADGET_FILE:
					file_box = box;
				ಭಙ್ಗ;

				ಪಕ್ಷ GADGET_TEXTBOX:
				ಪಕ್ಷ GADGET_TEXTAREA:
				ಪಕ್ಷ GADGET_PASSWORD:
					text_box = box;
					ಭಙ್ಗ;

				ಸ್ವಭಾವ:	/* appease compiler */
					ಭಙ್ಗ;
			}
		}
	}

	ಯದಿ (!file_box && !text_box)
		/* No box capable of handling drop */
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (file == ರಿಕ್ತ)
		/* There is a box capable of handling drop here */
		ಪ್ರತ್ಯಾಗಮ true;

	/* Handle the drop */
	ಯದಿ (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} ಅನ್ಯಥಾ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ರಿಕ್ತ;
		ವರ್ಣ *buffer;
		ವರ್ಣ *utf8_buff;
		nserror ret;
		unsigned ಪೂರ್ಣಾಙ್ಕ size;
		ಪೂರ್ಣಾಙ್ಕ bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ಯದಿ (fp == ರಿಕ್ತ) {
			/* Couldn't open file, but drop was for us */
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ಯದಿ ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		ಯದಿ (buffer == ರಿಕ್ತ) {
			/* No memory, but drop was for us */
			fclose(fp);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Stick file into buffer */
		ಯದಿ (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			ಮುಕ್ತ(buffer);
			fclose(fp);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		ಯದಿ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ಪ್ರತಿಜ್ಞಾ(ret != NSERROR_BAD_ENCODING);
			ಮುಕ್ತ(buffer);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Done with buffer */
		ಮುಕ್ತ(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		ಮುಕ್ತ(utf8_buff);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ಸ್ಥಿರ nserror
html_debug(ಸಂರಚನಾ content *c, ಗಣನಾ content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ಸ್ಥಿರ nserror
html_debug_dump(ಸಂರಚನಾ content *c, FILE *f, ಗಣನಾ content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_ಗಂಟು *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ಪ್ರತಿಜ್ಞಾ(htmlc != ರಿಕ್ತ);

	ಯದಿ (op == CONTENT_DEBUG_RENDER) {
		ಪ್ರತಿಜ್ಞಾ(htmlc->layout != ರಿಕ್ತ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ಅನ್ಯಥಾ {
		ಯದಿ (htmlc->document == ರಿಕ್ತ) {
			NSLOG(netsurf, INFO, "No document to dump");
			ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
		ಯದಿ ((exc != DOM_NO_ERR) || (html == ರಿಕ್ತ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_ಗಂಟು_unref(html);
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}


#ಯದಿ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_dump_frameset(ಸಂರಚನಾ content_html_frames *frame, unsigned ಪೂರ್ಣಾಙ್ಕ depth)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ row, col, index;
	ಧ್ರುವ ವರ್ಣ *unit[] = {"px", "%", "*"};
	ಧ್ರುವ ವರ್ಣ *scrolling[] = {"auto", "yes", "no"};

	ಪ್ರತಿಜ್ಞಾ(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.ಮೌಲ್ಯ, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.ಮೌಲ್ಯ,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	ಯದಿ (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	ಯದಿ (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	ಯದಿ (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	ಯದಿ (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned ಪೂರ್ಣಾಙ್ಕ) frame->border_colour);

	fprintf(stderr, "\n");

	ಯದಿ (frame->children) {
		ಪ್ರತಿ (row = 0; row != frame->rows; row++) {
			ಪ್ರತಿ (col = 0; col != frame->cols; col++) {
				ಪ್ರತಿ (i = 0; i != depth; i++)
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

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->document;
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
ಸಂರಚನಾ box *html_get_box_ವೃಕ್ಷ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *html_encoding(ಧ್ರುವ ಸಂರಚನಾ content *c, ಗಣನಾ content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ಸ್ಥಿರ ವರ್ಣ enc_token[10] = "Encoding0";

	ಪ್ರತಿಜ್ಞಾ(html != ರಿಕ್ತ);

	ಯದಿ (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		ಪ್ರತ್ಯಾಗಮ messages_get(enc_token);
	}

	ಪ್ರತ್ಯಾಗಮ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ಸಂರಚನಾ content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ಸಂರಚನಾ content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->iframe;
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

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ಧ್ರುವ ವರ್ಣ *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y)
{
	ಸಂರಚನಾ box *pos;
	ಸಂರಚನಾ box *layout;

	ಯದಿ (content_get_type(h) != CONTENT_HTML)
		ಪ್ರತ್ಯಾಗಮ false;

	layout = html_get_box_ವೃಕ್ಷ(h);

	ಯದಿ ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ಪ್ರತ್ಯಾಗಮ true;
	}
	ಪ್ರತ್ಯಾಗಮ false;
}

bool html_exec(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_ಗಂಟು;
	dom_string *dom_src;
	dom_text *text_ಗಂಟು;
	dom_ಗಂಟು *spare_ಗಂಟು;
	dom_html_script_element *script_ಗಂಟು;

	ಯದಿ (htmlc->document == ರಿಕ್ತ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		ಗಚ್ಛ out_no_string;
	}

	err = dom_string_create((ಧ್ರುವ uint8_t *)src, srclen, &dom_src);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		ಗಚ್ಛ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_ಗಂಟು);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		ಗಚ್ಛ out_no_body;
	}

	err = dom_document_create_text_ಗಂಟು(htmlc->document, dom_src, &text_ಗಂಟು);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		ಗಚ್ಛ out_no_text_ಗಂಟು;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_ಗಂಟು);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		ಗಚ್ಛ out_no_script_ಗಂಟು;
	}

	err = dom_ಗಂಟು_append_child(script_ಗಂಟು, text_ಗಂಟು, &spare_ಗಂಟು);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		ಗಚ್ಛ out_unparented;
	}
	dom_ಗಂಟು_unref(spare_ಗಂಟು); /* We do not need the spare ref at all */

	err = dom_ಗಂಟು_append_child(body_ಗಂಟು, script_ಗಂಟು, &spare_ಗಂಟು);
	ಯದಿ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		ಗಚ್ಛ out_unparented;
	}
	dom_ಗಂಟು_unref(spare_ಗಂಟು); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_ಗಂಟು_get_parent_ಗಂಟು(script_ಗಂಟು, &spare_ಗಂಟು);
	ಯದಿ (err == DOM_NO_ERR && spare_ಗಂಟು != ರಿಕ್ತ) {
		dom_ಗಂಟು *second_spare;
		err = dom_ಗಂಟು_remove_child(spare_ಗಂಟು, script_ಗಂಟು, &second_spare);
		ಯದಿ (err == DOM_NO_ERR) {
			dom_ಗಂಟು_unref(second_spare);
		}
		dom_ಗಂಟು_unref(spare_ಗಂಟು);
	}

out_unparented:
	dom_ಗಂಟು_unref(script_ಗಂಟು);
out_no_script_ಗಂಟು:
	dom_ಗಂಟು_unref(text_ಗಂಟು);
out_no_text_ಗಂಟು:
	dom_ಗಂಟು_unref(body_ಗಂಟು);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	ಪ್ರತ್ಯಾಗಮ result;
}

/* See \ref content_saw_insecure_objects */
ಸ್ಥಿರ bool
html_saw_insecure_objects(ಸಂರಚನಾ content *c)
{
	html_content *htmlc = (html_content *)c;
	ಸಂರಚನಾ content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ಯಾವತ್ (obj != ರಿಕ್ತ) {
		ಯದಿ (obj->content != ರಿಕ್ತ) {
			ಯದಿ (content_saw_insecure_objects(obj->content))
				ಪ್ರತ್ಯಾಗಮ true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	ಯದಿ (html_saw_insecure_scripts(htmlc)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Now check stylesheets */
	ಯದಿ (html_css_saw_insecure_stylesheets(htmlc)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ಸ್ಥಿರ content_type html_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_HTML;
}


ಸ್ಥಿರ ಶೂನ್ಯ html_fini(ಶೂನ್ಯ)
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
ಸ್ಥಿರ nserror
find_occurrences_html_box(ಧ್ರುವ ವರ್ಣ *pattern,
			  ಪೂರ್ಣಾಙ್ಕ p_len,
			  ಸಂರಚನಾ box *cur,
			  bool case_sens,
			  ಸಂರಚನಾ textsearch_context *context)
{
	ಸಂರಚನಾ box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ಯದಿ (!cur->object && cur->text) {
		ಧ್ರುವ ವರ್ಣ *text = cur->text;
		unsigned length = cur->length;

		ಯಾವತ್ (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			ಧ್ರುವ ವರ್ಣ *new_text;
			ಧ್ರುವ ವರ್ಣ *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			ಯದಿ (!pos)
				ಭಙ್ಗ;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	ಪ್ರತಿ (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ res;
		}
	}

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ nserror
html_textsearch_find(ಸಂರಚನಾ content *c,
		     ಸಂರಚನಾ textsearch_context *context,
		     ಧ್ರುವ ವರ್ಣ *pattern,
		     ಪೂರ್ಣಾಙ್ಕ p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ಯದಿ (html->layout == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
	}

	ಪ್ರತ್ಯಾಗಮ find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ಸ್ಥಿರ nserror
html_textsearch_bounds(ಸಂರಚನಾ content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       ಸಂರಚನಾ box *start_box,
		       ಸಂರಚನಾ box *end_box,
		       ಸಂರಚನಾ rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ಸ್ಥಿರ ಧ್ರುವ content_handler html_content_handler = {
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
nserror html_init(ಶೂನ್ಯ)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ಯದಿ (error != NSERROR_OK)
		ಗಚ್ಛ error;

	ಪ್ರತಿ (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		ಯದಿ (error != NSERROR_OK)
			ಗಚ್ಛ error;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

error:
	html_fini();

	ಪ್ರತ್ಯಾಗಮ error;
}
