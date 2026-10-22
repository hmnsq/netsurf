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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <nsutils/time.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/http.h"
#ଅନ୍ତର୍ଭାଵ "utils/libdom.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/utf8.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/string.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/utf8.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/keypress.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/layout.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "javascript/js.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/dom_event.h"
#ଅନ୍ତର୍ଭାଵ "html/css.h"
#ଅନ୍ତର୍ଭାଵ "html/object.h"
#ଅନ୍ତର୍ଭାଵ "html/html_save.h"
#ଅନ୍ତର୍ଭାଵ "html/interaction.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_construct.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/imagemap.h"
#ଅନ୍ତର୍ଭାଵ "html/layout.h"
#ଅନ୍ତର୍ଭାଵ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *html_types[] = {
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
ସ୍ଥିର bool fire_dom_event(dom_event *event, dom_ଗଣ୍ଠି *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	ଯଦି (exc != DOM_NO_ERR) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_ଗଣ୍ଠି *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	ଯଦି (exc != DOM_NO_ERR) ଫେରନ୍ତୁ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ଫେରନ୍ତୁ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	ଫେରନ୍ତୁ result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_ଗଣ୍ଠି *target,
		bool bubbles, bool cancelable, uint32_t ଚାବି)
{
	bool is_special = ଚାବି <= 0x001F || (0x007F <= ଚାବି && ଚାବି <= 0x009F);
	dom_string *dom_ଚାବି = ଖାଲି;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	ଯଦି (is_special) {
		ବାଛ (ଚାବି) {
		ସ୍ଥିତି NS_ଚାବି_ESCAPE:
			dom_ଚାବି = dom_string_ref(corestring_dom_Escape);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_ବାମ:
			dom_ଚାବି = dom_string_ref(corestring_dom_Arrow_ବାମ);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_ଡାହାଣ:
			dom_ଚାବି = dom_string_ref(corestring_dom_Arrow_ଡାହାଣ);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_UP:
			dom_ଚାବି = dom_string_ref(corestring_dom_ArrowUp);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_DOWN:
			dom_ଚାବି = dom_string_ref(corestring_dom_ArrowDown);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_PAGE_UP:
			dom_ଚାବି = dom_string_ref(corestring_dom_PageUp);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_PAGE_DOWN:
			dom_ଚାବି = dom_string_ref(corestring_dom_PageDown);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_TEXT_START:
			dom_ଚାବି = dom_string_ref(corestring_dom_Home);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NS_ଚାବି_TEXT_END:
			dom_ଚାବି = dom_string_ref(corestring_dom_End);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			dom_ଚାବି = ଖାଲି;
			ଭାଙ୍ଗ;
		}
	} ଅନ୍ୟଥା {
		ଅକ୍ଷର utf8[6];
		size_t length = utf8_from_ucs4(ଚାବି, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((ଅଚଳ uint8_t *)utf8, strlen(utf8),
				&dom_ଚାବି);
		ଯଦି (exc != DOM_NO_ERR) {
			ଫେରନ୍ତୁ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_string_unref(dom_ଚାବି);
		ଫେରନ୍ତୁ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ଖାଲି,
			dom_ଚାବି, ଖାଲି, DOM_ଚାବି_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_ଚାବି);
	ଯଦି (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ଫେରନ୍ତୁ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	ଫେରନ୍ତୁ result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ସ୍ଥିର ଶୂନ୍ୟ html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_ଗଣ୍ଠି *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ଖାଲି;

	/* Clean up and report error if unsuccessful or aborted */
	ଯଦି ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ଯଦି (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, ଖାଲି);
		} ଅନ୍ୟଥା {
			content_broadcast_error(&c->base, NSERROR_STOPPED, ଖାଲି);
		}

		content_set_error(&c->base);
		ଫେରନ୍ତୁ;
	}


#ଯଦି ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#ଯଦି ALWAYS_DUMP_FRAMESET
	ଯଦି (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (ଶୂନ୍ୟ *) &html);
	ଯଦି ((exc != DOM_NO_ERR) || (html == ଖାଲି)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, ଖାଲି);
		content_set_error(&c->base);
		ଫେରନ୍ତୁ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	ଯଦି (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, ଖାଲି);
		content_set_error(&c->base);
		dom_ଗଣ୍ଠି_unref(html);
		ଫେରନ୍ତୁ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ଖାଲି;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_ଗଣ୍ଠି_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	ବାଛ (content__get_status(&html->base)) {
	ସ୍ଥିତି CONTENT_STATUS_READY:
		ଯଦି (html->base.active == 0) {
			content_set_done(&html->base);
			ଫେରନ୍ତୁ NSERROR_OK;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CONTENT_STATUS_DONE:
		/* fallthrough */
	ସ୍ଥିତି CONTENT_STATUS_LOADING:
		ଫେରନ୍ତୁ NSERROR_OK;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ NSERROR_UNKNOWN;
}


ସ୍ଥିର ଶୂନ୍ୟ html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_ସନ୍ଦେଶ_GETDIMS, &ସନ୍ଦେଶ_data);


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
ଶୂନ୍ୟ html_finish_conversion(html_content *htmlc)
{
	ସଂଘ content_ସନ୍ଦେଶ_data ସନ୍ଦେଶ_data;
	dom_exception exc; /* returned by libdom functions */
	dom_ଗଣ୍ଠି *html;
	nserror error;

	/* Bail out if we've been aborted */
	ଯଦି (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ଖାଲି);
		content_set_error(&htmlc->base);
		ଫେରନ୍ତୁ;
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
	ଯଦି (htmlc->select_ctx != ଖାଲି) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ଫେରନ୍ତୁ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ଯଦି (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, ଖାଲି);
		content_set_error(&htmlc->base);
		ଫେରନ୍ତୁ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ଯଦି (htmlc->jsthread != ଖାଲି) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ଖାଲି);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	ସନ୍ଦେଶ_data.explicit_status_text = ଖାଲି;
	content_broadcast(&htmlc->base, CONTENT_ସନ୍ଦେଶ_STATUS, &ସନ୍ଦେଶ_data);

	exc = dom_document_get_document_element(htmlc->document, (ଶୂନ୍ୟ *) &html);
	ଯଦି ((exc != DOM_NO_ERR) || (html == ଖାଲି)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ଖାଲି);
		content_set_error(&htmlc->base);
		ଫେରନ୍ତୁ;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	ଯଦି (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_ଗଣ୍ଠି_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, ଖାଲି);
		content_set_error(&htmlc->base);
		ଫେରନ୍ତୁ;
	}

	dom_ଗଣ୍ଠି_unref(html);
}


ସ୍ଥିର ଶୂନ୍ୟ
html_document_user_data_handler(dom_ଗଣ୍ଠି_operation operation,
				dom_string *ଚାବି, ଶୂନ୍ୟ *data,
				ସଂରଚନା dom_ଗଣ୍ଠି *src,
				ସଂରଚନା dom_ଗଣ୍ଠି *dst)
{
	ଯଦି (dom_string_isequal(corestring_dom_ns_ଚାବି_html_content_data,
			       ଚାବି) == false || data == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ବାଛ (operation) {
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_DELETED:
		/* This is the only path I expect */
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ପ୍ରତିଜ୍ଞା(0);
	}
}


ସ୍ଥିର nserror
html_create_html_data(html_content *c, ଅଚଳ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	ଶୂନ୍ୟ *old_ଗଣ୍ଠି_data;
	ଅଚଳ ଅକ୍ଷର *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = ଖାଲି;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ଖାଲି;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = ଖାଲି;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = ଖାଲି;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ଖାଲି;
	c->bctx = ଖାଲି;
	c->layout = ଖାଲି;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_ଗଣନା = 0;
	c->stylesheets = ଖାଲି;
	c->select_ctx = ଖାଲି;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ଖାଲି;
	c->ଅଙ୍କ_objects = 0;
	c->object_list = ଖାଲି;
	c->forms = ଖାଲି;
	c->imagemaps = ଖାଲି;
	c->bw = ଖାଲି;
	c->frameset = ଖାଲି;
	c->iframe = ଖାଲି;
	c->page = ଖାଲି;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_ଗଣନା = 0;
	c->scripts = ଖାଲି;
	c->jsthread = ଖାଲି;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	ଯଦି (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ଖାଲି;
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	c->sel = selection_create((ସଂରଚନା content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	ଯଦି (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		ଯଦି (c->encoding == ଖାଲି) {
			lwc_string_unref(c->universal);
			c->universal = ଖାଲି;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ଖାଲି;
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.ସନ୍ଦେଶ = ଖାଲି;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ଯଦି ((error != DOM_HUBBUB_OK) && (c->encoding != ଖାଲି)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		ମୁକ୍ତ(c->encoding);
		c->encoding = ଖାଲି;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ଯଦି (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = ଖାଲି;

		lwc_string_unref(c->universal);
		c->universal = ଖାଲି;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ଖାଲି;

		ଫେରନ୍ତୁ libdom_hubbub_error_to_nserror(error);
	}

	err = dom_ଗଣ୍ଠି_set_user_data(c->document,
				     corestring_dom_ns_ଚାବି_html_content_data,
				     c, html_document_user_data_handler,
				     (ଶୂନ୍ୟ *) &old_ଗଣ୍ଠି_data);
	ଯଦି (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ଖାଲି;
		nsurl_unref(c->base_url);
		c->base_url = ଖାଲି;

		lwc_string_unref(c->universal);
		c->universal = ଖାଲି;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ଖାଲି;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ପ୍ରତିଜ୍ଞା(old_ଗଣ୍ଠି_data == ଖାଲି);

	ଫେରନ୍ତୁ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ସ୍ଥିର nserror
html_create(ଅଚଳ content_handler *handler,
	    lwc_string *imime_type,
	    ଅଚଳ http_parameter *params,
	    llcache_handle *llcache,
	    ଅଚଳ ଅକ୍ଷର *fallback_charset,
	    bool quirks,
	    ସଂରଚନା content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, ଆକାର(html_content));
	ଯଦି (html == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(html);
		ଫେରନ୍ତୁ error;
	}

	error = html_create_html_data(html, params);
	ଯଦି (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ଖାଲି);
		ମୁକ୍ତ(html);
		ଫେରନ୍ତୁ error;
	}

	error = html_css_new_stylesheets(html);
	ଯଦି (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ଖାଲି);
		ମୁକ୍ତ(html);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) html;

	ଫେରନ୍ତୁ NSERROR_OK;
}



ସ୍ଥିର nserror
html_process_encoding_change(ସଂରଚନା content *c,
			     ଅଚଳ ଅକ୍ଷର *data,
			     unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ଅଚଳ ଅକ୍ଷର *encoding;
	ଅଚଳ uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	ଯଦି (encoding == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (html->encoding != ଖାଲି) {
		ମୁକ୍ତ(html->encoding);
		html->encoding = ଖାଲି;
	}

	html->encoding = strdup(encoding);
	ଯଦି (html->encoding == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ଖାଲି;

	ଯଦି (html->document != ଖାଲି) {
		dom_ଗଣ୍ଠି_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.ସନ୍ଦେଶ = ଖାଲି;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ଯଦି (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		ମୁକ୍ତ(html->encoding);
		html->encoding = strdup("Windows-1252");
		ଯଦି (html->encoding == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ଯଦି (error != DOM_HUBBUB_OK) {
			ଫେରନ୍ତୁ libdom_hubbub_error_to_nserror(error);
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

	ଫେରନ୍ତୁ libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ସ୍ଥିର bool
html_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ଅଚଳ uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	ଯଦି (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	ଯଦି (err != NSERROR_OK) {
		content_broadcast_error(c, err, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_convert(ସଂରଚନା content *c)
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
	ଯଦି (exc == DOM_NO_ERR) {
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
	ଯଦି (html_can_begin_conversion(htmlc)) {
		ଫେରନ୍ତୁ html_begin_conversion(htmlc);
	}
	ଫେରନ୍ତୁ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	/* Cannot begin conversion if we're still fetching stuff */
	ଯଦି (htmlc->base.active != 0)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (i = 0; i != htmlc->stylesheet_ଗଣନା; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ଯଦି (htmlc->stylesheets[i].modified)
			ଫେରନ୍ତୁ false;
	}

	/* All is good, begin */
	ଫେରନ୍ତୁ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_ଗଣ୍ଠି *html;
	nserror ns_error;
	ସଂରଚନା form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *ଗଣ୍ଠି_name = ଖାଲି;
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
	ଯଦି (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ଯଦି (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ଫେରନ୍ତୁ true;
		}
		ଯଦି (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						ଖାଲି);

			ଫେରନ୍ତୁ false;
		}
		htmlc->parse_completed = true;
	}

	ଯଦି (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ଫେରନ୍ତୁ true;
	}

	/* Give up processing if we've been aborted */
	ଯଦି (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ଯଦି (htmlc->encoding == ଖାଲି) {
		ଅଚଳ ଅକ୍ଷର *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		ଯଦି (encoding == ଖାଲି) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ଖାଲି);
			ଫେରନ୍ତୁ false;
		}

		htmlc->encoding = strdup(encoding);
		ଯଦି (htmlc->encoding == ଖାଲି) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ଖାଲି);
			ଫେରନ୍ତୁ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (ଶୂନ୍ୟ *) &html);
	ଯଦି ((exc != DOM_NO_ERR) || (html == ଖାଲି)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	exc = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(html, &ଗଣ୍ଠି_name);
	ଯଦି ((exc != DOM_NO_ERR) ||
	    (ଗଣ୍ଠି_name == ଖାଲି) ||
	    (!dom_string_caseless_lwc_isequal(ଗଣ୍ଠି_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ଖାଲି);
		dom_ଗଣ୍ଠି_unref(html);
		ଫେରନ୍ତୁ false;
	}
	dom_string_unref(ଗଣ୍ଠି_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	ପାଇଁ (f = htmlc->forms; f != ଖାଲି; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ଯଦି (f->action == ଖାଲି || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} ଅନ୍ୟଥା {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		ଯଦି (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, ଖାଲି);

			dom_ଗଣ୍ଠି_unref(html);
			ଫେରନ୍ତୁ false;
		}

		ମୁକ୍ତ(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ଯଦି (f->action == ଖାଲି) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ଖାଲି);

			dom_ଗଣ୍ଠି_unref(html);
			ଫେରନ୍ତୁ false;
		}

		/* Ensure each form has a document encoding */
		ଯଦି (f->document_charset == ଖାଲି) {
			f->document_charset = strdup(htmlc->encoding);
			ଯଦି (f->document_charset == ଖାଲି) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							ଖାଲି);
				dom_ଗଣ୍ଠି_unref(html);
				ଫେରନ୍ତୁ false;
			}
		}
	}

	dom_ଗଣ୍ଠି_unref(html);

	ଯଦି (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ସ୍ଥିର ଶୂନ୍ୟ html_stop(ସଂରଚନା content *c)
{
	html_content *htmlc = (html_content *) c;

	ବାଛ (c->status) {
	ସ୍ଥିତି CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ଯଦି (htmlc->jsthread != ଖାଲି) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ଯଦି (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_STATUS_DONE:
		/* Nothing to do */
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		ପ୍ରତିଜ୍ଞା(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ସ୍ଥିର ଶୂନ୍ୟ html_reformat(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	html_content *htmlc = (html_content *) c;
	ସଂରଚନା box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ମୂଳ_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[ବାମ_2] + layout->width +
		layout->padding[ଡାହାଣ_2] + layout->border[ଡାହାଣ_2].width +
		layout->margin[ଡାହାଣ_2];
	c->height = layout->y + layout->padding[ଶୀର୍ଷ_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ଯଦି (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	ଯଦି (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ଯଦି (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

ଶୂନ୍ୟ html_redraw_a_box(hlcache_handle *h, ସଂରଚନା box *box)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2],
			box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

ଶୂନ୍ୟ html__redraw_a_box(ସଂରଚନା html_content *html, ସଂରଚନା box *box)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ସଂରଚନା content *)html, x, y,
			box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2],
			box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM]);
}

ସ୍ଥିର ଶୂନ୍ୟ html_destroy_frameset(ସଂରଚନା content_html_frames *frameset)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ଖାଲି;
	}
	ଯଦି (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ଖାଲି;
	}
	ଯଦି (frameset->children) {
		ପାଇଁ (i = 0; i < (frameset->rows * frameset->cols); i++) {
			ଯଦି (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ଖାଲି;
			}
			ଯଦି (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ଖାଲି;
			}
			ଯଦି (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ଖାଲି;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ html_destroy_iframe(ସଂରଚନା content_html_iframe *iframe)
{
	ସଂରଚନା content_html_iframe *next;
	next = iframe;
	ଯେତେବେଳେ ((iframe = next) != ଖାଲି) {
		next = iframe->next;
		ଯଦି (iframe->name)
			talloc_free(iframe->name);
		ଯଦି (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ଖାଲି;
		}
		talloc_free(iframe);
	}
}


ସ୍ଥିର ଶୂନ୍ୟ html_free_layout(html_content *htmlc)
{
	ଯଦି (htmlc->bctx != ଖାଲି) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ସ୍ଥିର ଶୂନ୍ୟ html_destroy(ସଂରଚନା content *c)
{
	html_content *html = (html_content *) c;
	ସଂରଚନା form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ଯଦି (html->box_conversion_context != ଖାଲି) {
		ଯଦି (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	ପାଇଁ (f = html->forms; f != ଖାଲି; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ଯଦି (c->refresh)
		nsurl_unref(c->refresh);

	ଯଦି (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ଯଦି (html->jsthread != ଖାଲି) {
		js_destroythread(html->jsthread);
		html->jsthread = ଖାଲି;
	}

	ଯଦି (html->parser != ଖାଲି) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ଖାଲି;
	}

	ଯଦି (html->document != ଖାଲି) {
		dom_ଗଣ୍ଠି_unref(html->document);
		html->document = ଖାଲି;
	}

	ଯଦି (html->title != ଖାଲି) {
		dom_ଗଣ୍ଠି_unref(html->title);
		html->title = ଖାଲି;
	}

	/* Free encoding */
	ଯଦି (html->encoding != ଖାଲି) {
		ମୁକ୍ତ(html->encoding);
		html->encoding = ଖାଲି;
	}

	/* Free base target */
	ଯଦି (html->base_target != ଖାଲି) {
		ମୁକ୍ତ(html->base_target);
		html->base_target = ଖାଲି;
	}

	/* Free frameset */
	ଯଦି (html->frameset != ଖାଲି) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ଖାଲି;
	}

	/* Free iframes */
	ଯଦି (html->iframe != ଖାଲି) {
		html_destroy_iframe(html->iframe);
		html->iframe = ଖାଲି;
	}

	/* Destroy selection context */
	ଯଦି (html->select_ctx != ଖାଲି) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ଖାଲି;
	}

	lwc_string_unref(html->universal);
	html->universal = ଖାଲି;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ଖାଲି;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ସ୍ଥିର nserror html_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ପ୍ରତିଜ୍ଞା(0 && "html_clone should never be called");

	ଫେରନ୍ତୁ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ସ୍ଥିର nserror
html_open(ସଂରଚନା content *c,
	  ସଂରଚନା browser_window *bw,
	  ସଂରଚନା content *page,
	  ସଂରଚନା object_params *params)
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

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ସ୍ଥିର nserror html_close(ସଂରଚନା content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ଖାଲି;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ଯଦି (htmlc->jsthread != ଖାଲି) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ଫେରନ୍ତୁ ret;
}


/**
 * Return an HTML content's selection context
 */

ସ୍ଥିର ଶୂନ୍ୟ html_clear_selection(ସଂରଚନା content *c)
{
	html_content *html = (html_content *) c;

	ବାଛ (html->selection_type) {
	ସ୍ଥିତି HTML_SELECTION_NONE:
		/* Nothing to do */
		ପ୍ରତିଜ୍ଞା(html->selection_owner.none == true);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HTML_SELECTION_SELF:
		ପ୍ରତିଜ୍ଞା(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ସ୍ଥିର ଅକ୍ଷର *html_get_selection(ସଂରଚନା content *c)
{
	html_content *html = (html_content *) c;

	ବାଛ (html->selection_type) {
	ସ୍ଥିତି HTML_SELECTION_TEXTAREA:
		ଫେରନ୍ତୁ textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	ସ୍ଥିତି HTML_SELECTION_SELF:
		ପ୍ରତିଜ୍ଞା(html->selection_owner.none == false);
		ଫେରନ୍ତୁ selection_get_copy(html->sel);
	ସ୍ଥିତି HTML_SELECTION_CONTENT:
		ଫେରନ୍ତୁ content_get_selection(
				html->selection_owner.content->object);
	ସ୍ଥିତି HTML_SELECTION_NONE:
		/* Nothing to do */
		ପ୍ରତିଜ୍ଞା(html->selection_owner.none == true);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ ଖାଲି;
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
ସ୍ଥିର nserror
html_get_contextual_content(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			    ସଂରଚନା browser_window_features *data)
{
	html_content *html = (html_content *) c;

	ସଂରଚନା box *box = html->layout;
	ସଂରଚନା box *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0, box_y = 0;

	ଯେତେବେଳେ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ଖାଲି) {
		box = next;

		/* hidden boxes are ignored */
		ଯଦି ((box->style != ଖାଲି) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ଜାରି;
		}

		ଯଦି (box->iframe) {
			ଦଶମିକ scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		ଯଦି (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		ଯଦି (box->object)
			data->object = box->object;

		ଯଦି (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		ଯଦି (box->usemap) {
			ଅଚଳ ଅକ୍ଷର *target = ଖାଲି;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ଯଦି (url != ଖାଲି)
				data->link = url;
		}
		ଯଦି (box->gadget) {
			ବାଛ (box->gadget->type) {
			ସ୍ଥିତି GADGET_TEXTBOX:
			ସ୍ଥିତି GADGET_TEXTAREA:
			ସ୍ଥିତି GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				ଭାଙ୍ଗ;

			ସ୍ଥିତି GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				ଭାଙ୍ଗ;

			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				data->form_features = CTX_FORM_NONE;
				ଭାଙ୍ଗ;
			}
		}
	}
	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର bool
html_scroll_at_point(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା scry)
{
	html_content *html = (html_content *) c;

	ସଂରଚନା box *box = html->layout;
	ସଂରଚନା box *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ଯେତେବେଳେ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ଖାଲି) {
		box = next;

		ଯଦି (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ଜାରି;

		/* Pass into iframe */
		ଯଦି (box->iframe) {
			ଦଶମିକ scale = browser_window_get_scale(box->iframe);

			ଯଦି (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ଫେରନ୍ତୁ true;
		}

		/* Pass into textarea widget */
		ଯଦି (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			ଫେରନ୍ତୁ true;

		/* Pass into object */
		ଯଦି (box->object != ଖାଲି && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ଫେରନ୍ତୁ true;

		/* Handle box scrollbars */
		ଯଦି (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ଯଦି (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ଯଦି (handled_scroll == true)
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ସ୍ଥିର ଶୂନ୍ୟ html__dom_user_data_handler(dom_ଗଣ୍ଠି_operation operation,
		dom_string *ଚାବି, ଶୂନ୍ୟ *_data, ସଂରଚନା dom_ଗଣ୍ଠି *src,
		ସଂରଚନା dom_ଗଣ୍ଠି *dst)
{
	ଅକ୍ଷର *oldfile;
	ଅକ୍ଷର *data = (ଅକ୍ଷର *)_data;

	ଯଦି (!dom_string_isequal(corestring_dom_ns_ଚାବି_file_name_ଗଣ୍ଠି_data,
				ଚାବି) || data == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	ବାଛ (operation) {
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_CLONED:
		ଯଦି (dom_ଗଣ୍ଠି_set_user_data(dst,
					   corestring_dom_ns_ଚାବି_file_name_ଗଣ୍ଠି_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			ଯଦି (oldfile != ଖାଲି)
				ମୁକ୍ତ(oldfile);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_RENAMED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_IMPORTED:
	ସ୍ଥିତି DOM_ଗଣ୍ଠି_ADOPTED:
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_ଗଣ୍ଠି_DELETED:
		ମୁକ୍ତ(data);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ପ୍ରତିଜ୍ଞା(0);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ html__set_file_gadget_filename(ସଂରଚନା content *c,
	ସଂରଚନା form_control *gadget, ଅଚଳ ଅକ୍ଷର *fn)
{
	nserror ret;
	ଅକ୍ଷର *utf8_fn, *oldfile = ଖାଲି;
	html_content *html = (html_content *)c;
	ସଂରଚନା box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	ଯଦି (ret != NSERROR_OK) {
		ପ୍ରତିଜ୍ଞା(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ଫେରନ୍ତୁ;
	}

	form_gadget_update_ମୂଲ୍ୟ(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ଯଦି (dom_ଗଣ୍ଠି_set_user_data((dom_ଗଣ୍ଠି *)file_box->gadget->ଗଣ୍ଠି,
				   corestring_dom_ns_ଚାବି_file_name_ଗଣ୍ଠି_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		ଯଦି (oldfile != ଖାଲି)
			ମୁକ୍ତ(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

ଶୂନ୍ୟ html_set_file_gadget_filename(ସଂରଚନା hlcache_handle *hl,
	ସଂରଚନା form_control *gadget, ଅଚଳ ଅକ୍ଷର *fn)
{
	ଫେରନ୍ତୁ html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
ସ୍ଥିର bool html_drop_file_at_point(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ଅକ୍ଷର *file)
{
	html_content *html = (html_content *) c;

	ସଂରଚନା box *box = html->layout;
	ସଂରଚନା box *next;
	ସଂରଚନା box *file_box = ଖାଲି;
	ସଂରଚନା box *text_box = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ଯେତେବେଳେ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ଖାଲି) {
		box = next;

		ଯଦି (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ଜାରି;

		ଯଦି (box->iframe) {
			ଦଶମିକ scale = browser_window_get_scale(box->iframe);
			ଫେରନ୍ତୁ browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ଯଦି (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			ଫେରନ୍ତୁ true;

		ଯଦି (box->gadget) {
			ବାଛ (box->gadget->type) {
				ସ୍ଥିତି GADGET_FILE:
					file_box = box;
				ଭାଙ୍ଗ;

				ସ୍ଥିତି GADGET_TEXTBOX:
				ସ୍ଥିତି GADGET_TEXTAREA:
				ସ୍ଥିତି GADGET_PASSWORD:
					text_box = box;
					ଭାଙ୍ଗ;

				ପୂର୍ବନିର୍ଦ୍ଧାରିତ:	/* appease compiler */
					ଭାଙ୍ଗ;
			}
		}
	}

	ଯଦି (!file_box && !text_box)
		/* No box capable of handling drop */
		ଫେରନ୍ତୁ false;

	ଯଦି (file == ଖାଲି)
		/* There is a box capable of handling drop here */
		ଫେରନ୍ତୁ true;

	/* Handle the drop */
	ଯଦି (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} ଅନ୍ୟଥା {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ଖାଲି;
		ଅକ୍ଷର *buffer;
		ଅକ୍ଷର *utf8_buff;
		nserror ret;
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ଯଦି (fp == ଖାଲି) {
			/* Couldn't open file, but drop was for us */
			ଫେରନ୍ତୁ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ଯଦି ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ଫେରନ୍ତୁ true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		ଯଦି (buffer == ଖାଲି) {
			/* No memory, but drop was for us */
			fclose(fp);
			ଫେରନ୍ତୁ true;
		}

		/* Stick file into buffer */
		ଯଦି (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			ମୁକ୍ତ(buffer);
			fclose(fp);
			ଫେରନ୍ତୁ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		ଯଦି (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ପ୍ରତିଜ୍ଞା(ret != NSERROR_BAD_ENCODING);
			ମୁକ୍ତ(buffer);
			ଫେରନ୍ତୁ true;
		}

		/* Done with buffer */
		ମୁକ୍ତ(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		ମୁକ୍ତ(utf8_buff);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ସ୍ଥିର nserror
html_debug(ସଂରଚନା content *c, ଗଣନା content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ସ୍ଥିର nserror
html_debug_dump(ସଂରଚନା content *c, FILE *f, ଗଣନା content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_ଗଣ୍ଠି *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ପ୍ରତିଜ୍ଞା(htmlc != ଖାଲି);

	ଯଦି (op == CONTENT_DEBUG_RENDER) {
		ପ୍ରତିଜ୍ଞା(htmlc->layout != ଖାଲି);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ଅନ୍ୟଥା {
		ଯଦି (htmlc->document == ଖାଲି) {
			NSLOG(netsurf, INFO, "No document to dump");
			ଫେରନ୍ତୁ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (ଶୂନ୍ୟ *) &html);
		ଯଦି ((exc != DOM_NO_ERR) || (html == ଖାଲି)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ଫେରନ୍ତୁ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_ଗଣ୍ଠି_unref(html);
	}

	ଫେରନ୍ତୁ ret;
}


#ଯଦି ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ସ୍ଥିର ଶୂନ୍ୟ
html_dump_frameset(ସଂରଚନା content_html_frames *frame, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା row, col, index;
	ଅଚଳ ଅକ୍ଷର *unit[] = {"px", "%", "*"};
	ଅଚଳ ଅକ୍ଷର *scrolling[] = {"auto", "yes", "no"};

	ପ୍ରତିଜ୍ଞା(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.ମୂଲ୍ୟ, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.ମୂଲ୍ୟ,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	ଯଦି (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	ଯଦି (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	ଯଦି (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	ଯଦି (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) frame->border_colour);

	fprintf(stderr, "\n");

	ଯଦି (frame->children) {
		ପାଇଁ (row = 0; row != frame->rows; row++) {
			ପାଇଁ (col = 0; col != frame->cols; col++) {
				ପାଇଁ (i = 0; i != depth; i++)
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

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->document;
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
ସଂରଚନା box *html_get_box_ବୃକ୍ଷ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *html_encoding(ଅଚଳ ସଂରଚନା content *c, ଗଣନା content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ସ୍ଥିର ଅକ୍ଷର enc_token[10] = "Encoding0";

	ପ୍ରତିଜ୍ଞା(html != ଖାଲି);

	ଯଦି (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		ଫେରନ୍ତୁ messages_get(enc_token);
	}

	ଫେରନ୍ତୁ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ସଂରଚନା content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ସଂରଚନା content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->iframe;
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

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ଅଚଳ ଅକ୍ଷର *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);

	ଫେରନ୍ତୁ c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y)
{
	ସଂରଚନା box *pos;
	ସଂରଚନା box *layout;

	ଯଦି (content_get_type(h) != CONTENT_HTML)
		ଫେରନ୍ତୁ false;

	layout = html_get_box_ବୃକ୍ଷ(h);

	ଯଦି ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ଫେରନ୍ତୁ true;
	}
	ଫେରନ୍ତୁ false;
}

bool html_exec(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_ଗଣ୍ଠି;
	dom_string *dom_src;
	dom_text *text_ଗଣ୍ଠି;
	dom_ଗଣ୍ଠି *spare_ଗଣ୍ଠି;
	dom_html_script_element *script_ଗଣ୍ଠି;

	ଯଦି (htmlc->document == ଖାଲି) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		ଯାଅ out_no_string;
	}

	err = dom_string_create((ଅଚଳ uint8_t *)src, srclen, &dom_src);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		ଯାଅ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_ଗଣ୍ଠି);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		ଯାଅ out_no_body;
	}

	err = dom_document_create_text_ଗଣ୍ଠି(htmlc->document, dom_src, &text_ଗଣ୍ଠି);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		ଯାଅ out_no_text_ଗଣ୍ଠି;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_ଗଣ୍ଠି);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		ଯାଅ out_no_script_ଗଣ୍ଠି;
	}

	err = dom_ଗଣ୍ଠି_append_child(script_ଗଣ୍ଠି, text_ଗଣ୍ଠି, &spare_ଗଣ୍ଠି);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		ଯାଅ out_unparented;
	}
	dom_ଗଣ୍ଠି_unref(spare_ଗଣ୍ଠି); /* We do not need the spare ref at all */

	err = dom_ଗଣ୍ଠି_append_child(body_ଗଣ୍ଠି, script_ଗଣ୍ଠି, &spare_ଗଣ୍ଠି);
	ଯଦି (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		ଯାଅ out_unparented;
	}
	dom_ଗଣ୍ଠି_unref(spare_ଗଣ୍ଠି); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(script_ଗଣ୍ଠି, &spare_ଗଣ୍ଠି);
	ଯଦି (err == DOM_NO_ERR && spare_ଗଣ୍ଠି != ଖାଲି) {
		dom_ଗଣ୍ଠି *second_spare;
		err = dom_ଗଣ୍ଠି_remove_child(spare_ଗଣ୍ଠି, script_ଗଣ୍ଠି, &second_spare);
		ଯଦି (err == DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(second_spare);
		}
		dom_ଗଣ୍ଠି_unref(spare_ଗଣ୍ଠି);
	}

out_unparented:
	dom_ଗଣ୍ଠି_unref(script_ଗଣ୍ଠି);
out_no_script_ଗଣ୍ଠି:
	dom_ଗଣ୍ଠି_unref(text_ଗଣ୍ଠି);
out_no_text_ଗଣ୍ଠି:
	dom_ଗଣ୍ଠି_unref(body_ଗଣ୍ଠି);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	ଫେରନ୍ତୁ result;
}

/* See \ref content_saw_insecure_objects */
ସ୍ଥିର bool
html_saw_insecure_objects(ସଂରଚନା content *c)
{
	html_content *htmlc = (html_content *)c;
	ସଂରଚନା content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ଯେତେବେଳେ (obj != ଖାଲି) {
		ଯଦି (obj->content != ଖାଲି) {
			ଯଦି (content_saw_insecure_objects(obj->content))
				ଫେରନ୍ତୁ true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	ଯଦି (html_saw_insecure_scripts(htmlc)) {
		ଫେରନ୍ତୁ true;
	}

	/* Now check stylesheets */
	ଯଦି (html_css_saw_insecure_stylesheets(htmlc)) {
		ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ସ୍ଥିର content_type html_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_HTML;
}


ସ୍ଥିର ଶୂନ୍ୟ html_fini(ଶୂନ୍ୟ)
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
ସ୍ଥିର nserror
find_occurrences_html_box(ଅଚଳ ଅକ୍ଷର *pattern,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_len,
			  ସଂରଚନା box *cur,
			  bool case_sens,
			  ସଂରଚନା textsearch_context *context)
{
	ସଂରଚନା box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ଯଦି (!cur->object && cur->text) {
		ଅଚଳ ଅକ୍ଷର *text = cur->text;
		unsigned length = cur->length;

		ଯେତେବେଳେ (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			ଅଚଳ ଅକ୍ଷର *new_text;
			ଅଚଳ ଅକ୍ଷର *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			ଯଦି (!pos)
				ଭାଙ୍ଗ;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	ପାଇଁ (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ res;
		}
	}

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror
html_textsearch_find(ସଂରଚନା content *c,
		     ସଂରଚନା textsearch_context *context,
		     ଅଚଳ ଅକ୍ଷର *pattern,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ଯଦି (html->layout == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_INVALID;
	}

	ଫେରନ୍ତୁ find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ସ୍ଥିର nserror
html_textsearch_bounds(ସଂରଚନା content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       ସଂରଚନା box *start_box,
		       ସଂରଚନା box *end_box,
		       ସଂରଚନା rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ସ୍ଥିର ଅଚଳ content_handler html_content_handler = {
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
nserror html_init(ଶୂନ୍ୟ)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ଯଦି (error != NSERROR_OK)
		ଯାଅ error;

	ପାଇଁ (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		ଯଦି (error != NSERROR_OK)
			ଯାଅ error;
	}

	ଫେରନ୍ତୁ NSERROR_OK;

error:
	html_fini();

	ଫେରନ୍ତୁ error;
}
