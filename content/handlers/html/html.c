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

#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <nsutils/time.h>

#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/http.h"
#অন্তর্ভাৱ "utils/libdom.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "utils/talloc.h"
#অন্তর্ভাৱ "utils/utf8.h"
#অন্তর্ভাৱ "utils/nsoption.h"
#অন্তর্ভাৱ "utils/string.h"
#অন্তর্ভাৱ "utils/ascii.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "netsurf/browser_window.h"
#অন্তর্ভাৱ "netsurf/utf8.h"
#অন্তর্ভাৱ "netsurf/keypress.h"
#অন্তর্ভাৱ "netsurf/layout.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "content/hlcache.h"
#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "content/textsearch.h"
#অন্তর্ভাৱ "desktop/selection.h"
#অন্তর্ভাৱ "desktop/scrollbar.h"
#অন্তর্ভাৱ "desktop/textarea.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "javascript/js.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"

#অন্তর্ভাৱ "html/html.h"
#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/dom_event.h"
#অন্তর্ভাৱ "html/css.h"
#অন্তর্ভাৱ "html/object.h"
#অন্তর্ভাৱ "html/html_save.h"
#অন্তর্ভাৱ "html/interaction.h"
#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/box_construct.h"
#অন্তর্ভাৱ "html/box_inspect.h"
#অন্তর্ভাৱ "html/form_internal.h"
#অন্তর্ভাৱ "html/imagemap.h"
#অন্তর্ভাৱ "html/layout.h"
#অন্তর্ভাৱ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

স্থির ধ্রুৱ ৱর্ণ *html_types[] = {
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
স্থির bool fire_dom_event(dom_event *event, dom_গ্ৰন্থি *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	যদি (exc != DOM_NO_ERR) {
		ঘূৰাই false;
	}

	ঘূৰাই result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_গ্ৰন্থি *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	যদি (exc != DOM_NO_ERR) ঘূৰাই false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	যদি (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ঘূৰাই false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (পূর্ণাঙ্ক)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	ঘূৰাই result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_গ্ৰন্থি *target,
		bool bubbles, bool cancelable, uint32_t চাবি)
{
	bool is_special = চাবি <= 0x001F || (0x007F <= চাবি && চাবি <= 0x009F);
	dom_string *dom_চাবি = রিক্ত;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	যদি (is_special) {
		ৱিকল্প (চাবি) {
		পক্ষ NS_চাবি_ESCAPE:
			dom_চাবি = dom_string_ref(corestring_dom_Escape);
			ভঙ্গ;
		পক্ষ NS_চাবি_বাম:
			dom_চাবি = dom_string_ref(corestring_dom_Arrow_বাম);
			ভঙ্গ;
		পক্ষ NS_চাবি_ডান:
			dom_চাবি = dom_string_ref(corestring_dom_Arrow_ডান);
			ভঙ্গ;
		পক্ষ NS_চাবি_UP:
			dom_চাবি = dom_string_ref(corestring_dom_ArrowUp);
			ভঙ্গ;
		পক্ষ NS_চাবি_DOWN:
			dom_চাবি = dom_string_ref(corestring_dom_ArrowDown);
			ভঙ্গ;
		পক্ষ NS_চাবি_PAGE_UP:
			dom_চাবি = dom_string_ref(corestring_dom_PageUp);
			ভঙ্গ;
		পক্ষ NS_চাবি_PAGE_DOWN:
			dom_চাবি = dom_string_ref(corestring_dom_PageDown);
			ভঙ্গ;
		পক্ষ NS_চাবি_TEXT_START:
			dom_চাবি = dom_string_ref(corestring_dom_Home);
			ভঙ্গ;
		পক্ষ NS_চাবি_TEXT_END:
			dom_চাবি = dom_string_ref(corestring_dom_End);
			ভঙ্গ;
		স্ৱভাৱ:
			dom_চাবি = রিক্ত;
			ভঙ্গ;
		}
	} অন্যথা {
		ৱর্ণ utf8[6];
		size_t length = utf8_from_ucs4(চাবি, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((ধ্রুৱ uint8_t *)utf8, strlen(utf8),
				&dom_চাবি);
		যদি (exc != DOM_NO_ERR) {
			ঘূৰাই exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	যদি (exc != DOM_NO_ERR) {
		dom_string_unref(dom_চাবি);
		ঘূৰাই false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, রিক্ত,
			dom_চাবি, রিক্ত, DOM_চাবি_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_চাবি);
	যদি (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ঘূৰাই false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(পূর্ণাঙ্ক)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	ঘূৰাই result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
স্থির শূন্য html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_গ্ৰন্থি *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = রিক্ত;

	/* Clean up and report error if unsuccessful or aborted */
	যদি ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		যদি (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, রিক্ত);
		} অন্যথা {
			content_broadcast_error(&c->base, NSERROR_STOPPED, রিক্ত);
		}

		content_set_error(&c->base);
		ঘূৰাই;
	}


#যদি ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#যদি ALWAYS_DUMP_FRAMESET
	যদি (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (শূন্য *) &html);
	যদি ((exc != DOM_NO_ERR) || (html == রিক্ত)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, রিক্ত);
		content_set_error(&c->base);
		ঘূৰাই;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	যদি (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, রিক্ত);
		content_set_error(&c->base);
		dom_গ্ৰন্থি_unref(html);
		ঘূৰাই;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = রিক্ত;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_গ্ৰন্থি_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	ৱিকল্প (content__get_status(&html->base)) {
	পক্ষ CONTENT_STATUS_READY:
		যদি (html->base.active == 0) {
			content_set_done(&html->base);
			ঘূৰাই NSERROR_OK;
		}
		ভঙ্গ;
	পক্ষ CONTENT_STATUS_DONE:
		/* fallthrough */
	পক্ষ CONTENT_STATUS_LOADING:
		ঘূৰাই NSERROR_OK;
	স্ৱভাৱ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ভঙ্গ;
	}
	ঘূৰাই NSERROR_UNKNOWN;
}


স্থির শূন্য html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_বাৰ্তা_GETDIMS, &বাৰ্তা_data);


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
শূন্য html_finish_conversion(html_content *htmlc)
{
	সঙ্ঘ content_বাৰ্তা_data বাৰ্তা_data;
	dom_exception exc; /* returned by libdom functions */
	dom_গ্ৰন্থি *html;
	nserror error;

	/* Bail out if we've been aborted */
	যদি (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, রিক্ত);
		content_set_error(&htmlc->base);
		ঘূৰাই;
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
	যদি (htmlc->select_ctx != রিক্ত) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ঘূৰাই;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	যদি (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, রিক্ত);
		content_set_error(&htmlc->base);
		ঘূৰাই;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	যদি (htmlc->jsthread != রিক্ত) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, রিক্ত);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	বাৰ্তা_data.explicit_status_text = রিক্ত;
	content_broadcast(&htmlc->base, CONTENT_বাৰ্তা_STATUS, &বাৰ্তা_data);

	exc = dom_document_get_document_element(htmlc->document, (শূন্য *) &html);
	যদি ((exc != DOM_NO_ERR) || (html == রিক্ত)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, রিক্ত);
		content_set_error(&htmlc->base);
		ঘূৰাই;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	যদি (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_গ্ৰন্থি_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, রিক্ত);
		content_set_error(&htmlc->base);
		ঘূৰাই;
	}

	dom_গ্ৰন্থি_unref(html);
}


স্থির শূন্য
html_document_user_data_handler(dom_গ্ৰন্থি_operation operation,
				dom_string *চাবি, শূন্য *data,
				সংরচনা dom_গ্ৰন্থি *src,
				সংরচনা dom_গ্ৰন্থি *dst)
{
	যদি (dom_string_isequal(corestring_dom_ns_চাবি_html_content_data,
			       চাবি) == false || data == রিক্ত) {
		ঘূৰাই;
	}

	ৱিকল্প (operation) {
	পক্ষ DOM_গ্ৰন্থি_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ভঙ্গ;
	পক্ষ DOM_গ্ৰন্থি_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ভঙ্গ;
	পক্ষ DOM_গ্ৰন্থি_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ভঙ্গ;
	পক্ষ DOM_গ্ৰন্থি_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ভঙ্গ;
	পক্ষ DOM_গ্ৰন্থি_DELETED:
		/* This is the only path I expect */
		ভঙ্গ;
	স্ৱভাৱ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		প্রতিজ্ঞা(0);
	}
}


স্থির nserror
html_create_html_data(html_content *c, ধ্রুৱ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	শূন্য *old_গ্ৰন্থি_data;
	ধ্রুৱ ৱর্ণ *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = রিক্ত;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = রিক্ত;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = রিক্ত;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = রিক্ত;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = রিক্ত;
	c->bctx = রিক্ত;
	c->layout = রিক্ত;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_গুনতি = 0;
	c->stylesheets = রিক্ত;
	c->select_ctx = রিক্ত;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = রিক্ত;
	c->অঙ্ক_objects = 0;
	c->object_list = রিক্ত;
	c->forms = রিক্ত;
	c->imagemaps = রিক্ত;
	c->bw = রিক্ত;
	c->frameset = রিক্ত;
	c->iframe = রিক্ত;
	c->page = রিক্ত;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_গুনতি = 0;
	c->scripts = রিক্ত;
	c->jsthread = রিক্ত;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	যদি (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ঘূৰাই NSERROR_NOMEM;
	}

	যদি (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = রিক্ত;
		ঘূৰাই NSERROR_NOMEM;
	}

	c->sel = selection_create((সংরচনা content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	যদি (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		যদি (c->encoding == রিক্ত) {
			lwc_string_unref(c->universal);
			c->universal = রিক্ত;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = রিক্ত;
			ঘূৰাই NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.বাৰ্তা = রিক্ত;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	যদি ((error != DOM_HUBBUB_OK) && (c->encoding != রিক্ত)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		মুক্ত(c->encoding);
		c->encoding = রিক্ত;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	যদি (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = রিক্ত;

		lwc_string_unref(c->universal);
		c->universal = রিক্ত;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = রিক্ত;

		ঘূৰাই libdom_hubbub_error_to_nserror(error);
	}

	err = dom_গ্ৰন্থি_set_user_data(c->document,
				     corestring_dom_ns_চাবি_html_content_data,
				     c, html_document_user_data_handler,
				     (শূন্য *) &old_গ্ৰন্থি_data);
	যদি (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = রিক্ত;
		nsurl_unref(c->base_url);
		c->base_url = রিক্ত;

		lwc_string_unref(c->universal);
		c->universal = রিক্ত;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = রিক্ত;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ঘূৰাই NSERROR_DOM;
	}

	প্রতিজ্ঞা(old_গ্ৰন্থি_data == রিক্ত);

	ঘূৰাই NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

স্থির nserror
html_create(ধ্রুৱ content_handler *handler,
	    lwc_string *imime_type,
	    ধ্রুৱ http_parameter *params,
	    llcache_handle *llcache,
	    ধ্রুৱ ৱর্ণ *fallback_charset,
	    bool quirks,
	    সংরচনা content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, পরিমাণ(html_content));
	যদি (html == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(html);
		ঘূৰাই error;
	}

	error = html_create_html_data(html, params);
	যদি (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, রিক্ত);
		মুক্ত(html);
		ঘূৰাই error;
	}

	error = html_css_new_stylesheets(html);
	যদি (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, রিক্ত);
		মুক্ত(html);
		ঘূৰাই error;
	}

	*c = (সংরচনা content *) html;

	ঘূৰাই NSERROR_OK;
}



স্থির nserror
html_process_encoding_change(সংরচনা content *c,
			     ধ্রুৱ ৱর্ণ *data,
			     unsigned পূর্ণাঙ্ক size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ধ্রুৱ ৱর্ণ *encoding;
	ধ্রুৱ uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	যদি (encoding == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	যদি (html->encoding != রিক্ত) {
		মুক্ত(html->encoding);
		html->encoding = রিক্ত;
	}

	html->encoding = strdup(encoding);
	যদি (html->encoding == রিক্ত) {
		ঘূৰাই NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = রিক্ত;

	যদি (html->document != রিক্ত) {
		dom_গ্ৰন্থি_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.বাৰ্তা = রিক্ত;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	যদি (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		মুক্ত(html->encoding);
		html->encoding = strdup("Windows-1252");
		যদি (html->encoding == রিক্ত) {
			ঘূৰাই NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		যদি (error != DOM_HUBBUB_OK) {
			ঘূৰাই libdom_hubbub_error_to_nserror(error);
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

	ঘূৰাই libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

স্থির bool
html_process_data(সংরচনা content *c, ধ্রুৱ ৱর্ণ *data, unsigned পূর্ণাঙ্ক size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ধ্রুৱ uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	যদি (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	যদি (err != NSERROR_OK) {
		content_broadcast_error(c, err, রিক্ত);
		ঘূৰাই false;
	}

	ঘূৰাই true;
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

স্থির bool html_convert(সংরচনা content *c)
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
	যদি (exc == DOM_NO_ERR) {
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
	যদি (html_can_begin_conversion(htmlc)) {
		ঘূৰাই html_begin_conversion(htmlc);
	}
	ঘূৰাই true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned পূর্ণাঙ্ক i;

	/* Cannot begin conversion if we're still fetching stuff */
	যদি (htmlc->base.active != 0)
		ঘূৰাই false;

	বাবে (i = 0; i != htmlc->stylesheet_গুনতি; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		যদি (htmlc->stylesheets[i].modified)
			ঘূৰাই false;
	}

	/* All is good, begin */
	ঘূৰাই true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_গ্ৰন্থি *html;
	nserror ns_error;
	সংরচনা form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *গ্ৰন্থি_name = রিক্ত;
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
	যদি (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		যদি (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ঘূৰাই true;
		}
		যদি (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						রিক্ত);

			ঘূৰাই false;
		}
		htmlc->parse_completed = true;
	}

	যদি (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ঘূৰাই true;
	}

	/* Give up processing if we've been aborted */
	যদি (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, রিক্ত);
		ঘূৰাই false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	যদি (htmlc->encoding == রিক্ত) {
		ধ্রুৱ ৱর্ণ *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		যদি (encoding == রিক্ত) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						রিক্ত);
			ঘূৰাই false;
		}

		htmlc->encoding = strdup(encoding);
		যদি (htmlc->encoding == রিক্ত) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						রিক্ত);
			ঘূৰাই false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (শূন্য *) &html);
	যদি ((exc != DOM_NO_ERR) || (html == রিক্ত)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, রিক্ত);
		ঘূৰাই false;
	}

	exc = dom_গ্ৰন্থি_get_গ্ৰন্থি_name(html, &গ্ৰন্থি_name);
	যদি ((exc != DOM_NO_ERR) ||
	    (গ্ৰন্থি_name == রিক্ত) ||
	    (!dom_string_caseless_lwc_isequal(গ্ৰন্থি_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, রিক্ত);
		dom_গ্ৰন্থি_unref(html);
		ঘূৰাই false;
	}
	dom_string_unref(গ্ৰন্থি_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	বাবে (f = htmlc->forms; f != রিক্ত; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		যদি (f->action == রিক্ত || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} অন্যথা {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		যদি (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, রিক্ত);

			dom_গ্ৰন্থি_unref(html);
			ঘূৰাই false;
		}

		মুক্ত(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		যদি (f->action == রিক্ত) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						রিক্ত);

			dom_গ্ৰন্থি_unref(html);
			ঘূৰাই false;
		}

		/* Ensure each form has a document encoding */
		যদি (f->document_charset == রিক্ত) {
			f->document_charset = strdup(htmlc->encoding);
			যদি (f->document_charset == রিক্ত) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							রিক্ত);
				dom_গ্ৰন্থি_unref(html);
				ঘূৰাই false;
			}
		}
	}

	dom_গ্ৰন্থি_unref(html);

	যদি (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	ঘূৰাই true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

স্থির শূন্য html_stop(সংরচনা content *c)
{
	html_content *htmlc = (html_content *) c;

	ৱিকল্প (c->status) {
	পক্ষ CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		যদি (htmlc->jsthread != রিক্ত) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ভঙ্গ;

	পক্ষ CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		যদি (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		ভঙ্গ;

	পক্ষ CONTENT_STATUS_DONE:
		/* Nothing to do */
		ভঙ্গ;

	স্ৱভাৱ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		প্রতিজ্ঞা(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

স্থির শূন্য html_reformat(সংরচনা content *c, পূর্ণাঙ্ক width, পূর্ণাঙ্ক height)
{
	html_content *htmlc = (html_content *) c;
	সংরচনা box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.মূল_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[বাম_2] + layout->width +
		layout->padding[ডান_2] + layout->border[ডান_2].width +
		layout->margin[ডান_2];
	c->height = layout->y + layout->padding[শীৰ্ষ_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	যদি (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	যদি (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	যদি (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

শূন্য html_redraw_a_box(hlcache_handle *h, সংরচনা box *box)
{
	পূর্ণাঙ্ক x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[বাম_2] + box->width + box->padding[ডান_2],
			box->padding[শীৰ্ষ_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

শূন্য html__redraw_a_box(সংরচনা html_content *html, সংরচনা box *box)
{
	পূর্ণাঙ্ক x, y;

	box_coords(box, &x, &y);

	content__request_redraw((সংরচনা content *)html, x, y,
			box->padding[বাম_2] + box->width + box->padding[ডান_2],
			box->padding[শীৰ্ষ_2] + box->height + box->padding[BOTTOM]);
}

স্থির শূন্য html_destroy_frameset(সংরচনা content_html_frames *frameset)
{
	পূর্ণাঙ্ক i;

	যদি (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = রিক্ত;
	}
	যদি (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = রিক্ত;
	}
	যদি (frameset->children) {
		বাবে (i = 0; i < (frameset->rows * frameset->cols); i++) {
			যদি (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = রিক্ত;
			}
			যদি (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = রিক্ত;
			}
			যদি (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = রিক্ত;
	}
}

স্থির শূন্য html_destroy_iframe(সংরচনা content_html_iframe *iframe)
{
	সংরচনা content_html_iframe *next;
	next = iframe;
	যেতিয়ালৈ ((iframe = next) != রিক্ত) {
		next = iframe->next;
		যদি (iframe->name)
			talloc_free(iframe->name);
		যদি (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = রিক্ত;
		}
		talloc_free(iframe);
	}
}


স্থির শূন্য html_free_layout(html_content *htmlc)
{
	যদি (htmlc->bctx != রিক্ত) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

স্থির শূন্য html_destroy(সংরচনা content *c)
{
	html_content *html = (html_content *) c;
	সংরচনা form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	যদি (html->box_conversion_context != রিক্ত) {
		যদি (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	বাবে (f = html->forms; f != রিক্ত; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	যদি (c->refresh)
		nsurl_unref(c->refresh);

	যদি (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	যদি (html->jsthread != রিক্ত) {
		js_destroythread(html->jsthread);
		html->jsthread = রিক্ত;
	}

	যদি (html->parser != রিক্ত) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = রিক্ত;
	}

	যদি (html->document != রিক্ত) {
		dom_গ্ৰন্থি_unref(html->document);
		html->document = রিক্ত;
	}

	যদি (html->title != রিক্ত) {
		dom_গ্ৰন্থি_unref(html->title);
		html->title = রিক্ত;
	}

	/* Free encoding */
	যদি (html->encoding != রিক্ত) {
		মুক্ত(html->encoding);
		html->encoding = রিক্ত;
	}

	/* Free base target */
	যদি (html->base_target != রিক্ত) {
		মুক্ত(html->base_target);
		html->base_target = রিক্ত;
	}

	/* Free frameset */
	যদি (html->frameset != রিক্ত) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = রিক্ত;
	}

	/* Free iframes */
	যদি (html->iframe != রিক্ত) {
		html_destroy_iframe(html->iframe);
		html->iframe = রিক্ত;
	}

	/* Destroy selection context */
	যদি (html->select_ctx != রিক্ত) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = রিক্ত;
	}

	lwc_string_unref(html->universal);
	html->universal = রিক্ত;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = রিক্ত;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


স্থির nserror html_clone(ধ্রুৱ সংরচনা content *old, সংরচনা content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	প্রতিজ্ঞা(0 && "html_clone should never be called");

	ঘূৰাই true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

স্থির nserror
html_open(সংরচনা content *c,
	  সংরচনা browser_window *bw,
	  সংরচনা content *page,
	  সংরচনা object_params *params)
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

	ঘূৰাই NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

স্থির nserror html_close(সংরচনা content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = রিক্ত;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	যদি (htmlc->jsthread != রিক্ত) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ঘূৰাই ret;
}


/**
 * Return an HTML content's selection context
 */

স্থির শূন্য html_clear_selection(সংরচনা content *c)
{
	html_content *html = (html_content *) c;

	ৱিকল্প (html->selection_type) {
	পক্ষ HTML_SELECTION_NONE:
		/* Nothing to do */
		প্রতিজ্ঞা(html->selection_owner.none == true);
		ভঙ্গ;
	পক্ষ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		ভঙ্গ;
	পক্ষ HTML_SELECTION_SELF:
		প্রতিজ্ঞা(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ভঙ্গ;
	পক্ষ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ভঙ্গ;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

স্থির ৱর্ণ *html_get_selection(সংরচনা content *c)
{
	html_content *html = (html_content *) c;

	ৱিকল্প (html->selection_type) {
	পক্ষ HTML_SELECTION_TEXTAREA:
		ঘূৰাই textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	পক্ষ HTML_SELECTION_SELF:
		প্রতিজ্ঞা(html->selection_owner.none == false);
		ঘূৰাই selection_get_copy(html->sel);
	পক্ষ HTML_SELECTION_CONTENT:
		ঘূৰাই content_get_selection(
				html->selection_owner.content->object);
	পক্ষ HTML_SELECTION_NONE:
		/* Nothing to do */
		প্রতিজ্ঞা(html->selection_owner.none == true);
		ভঙ্গ;
	স্ৱভাৱ:
		ভঙ্গ;
	}

	ঘূৰাই রিক্ত;
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
স্থির nserror
html_get_contextual_content(সংরচনা content *c, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y,
			    সংরচনা browser_window_features *data)
{
	html_content *html = (html_content *) c;

	সংরচনা box *box = html->layout;
	সংরচনা box *next;
	পূর্ণাঙ্ক box_x = 0, box_y = 0;

	যেতিয়ালৈ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != রিক্ত) {
		box = next;

		/* hidden boxes are ignored */
		যদি ((box->style != রিক্ত) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			অনুৱর্ত;
		}

		যদি (box->iframe) {
			ভিন্ন scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		যদি (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		যদি (box->object)
			data->object = box->object;

		যদি (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		যদি (box->usemap) {
			ধ্রুৱ ৱর্ণ *target = রিক্ত;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			যদি (url != রিক্ত)
				data->link = url;
		}
		যদি (box->gadget) {
			ৱিকল্প (box->gadget->type) {
			পক্ষ GADGET_TEXTBOX:
			পক্ষ GADGET_TEXTAREA:
			পক্ষ GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				ভঙ্গ;

			পক্ষ GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				ভঙ্গ;

			স্ৱভাৱ:
				data->form_features = CTX_FORM_NONE;
				ভঙ্গ;
			}
		}
	}
	ঘূৰাই NSERROR_OK;
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
স্থির bool
html_scroll_at_point(সংরচনা content *c, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, পূর্ণাঙ্ক scrx, পূর্ণাঙ্ক scry)
{
	html_content *html = (html_content *) c;

	সংরচনা box *box = html->layout;
	সংরচনা box *next;
	পূর্ণাঙ্ক box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	যেতিয়ালৈ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != রিক্ত) {
		box = next;

		যদি (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			অনুৱর্ত;

		/* Pass into iframe */
		যদি (box->iframe) {
			ভিন্ন scale = browser_window_get_scale(box->iframe);

			যদি (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ঘূৰাই true;
		}

		/* Pass into textarea widget */
		যদি (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			ঘূৰাই true;

		/* Pass into object */
		যদি (box->object != রিক্ত && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ঘূৰাই true;

		/* Handle box scrollbars */
		যদি (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		যদি (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		যদি (handled_scroll == true)
			ঘূৰাই true;
	}

	ঘূৰাই false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
স্থির শূন্য html__dom_user_data_handler(dom_গ্ৰন্থি_operation operation,
		dom_string *চাবি, শূন্য *_data, সংরচনা dom_গ্ৰন্থি *src,
		সংরচনা dom_গ্ৰন্থি *dst)
{
	ৱর্ণ *oldfile;
	ৱর্ণ *data = (ৱর্ণ *)_data;

	যদি (!dom_string_isequal(corestring_dom_ns_চাবি_file_name_গ্ৰন্থি_data,
				চাবি) || data == রিক্ত) {
		ঘূৰাই;
	}

	ৱিকল্প (operation) {
	পক্ষ DOM_গ্ৰন্থি_CLONED:
		যদি (dom_গ্ৰন্থি_set_user_data(dst,
					   corestring_dom_ns_চাবি_file_name_গ্ৰন্থি_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			যদি (oldfile != রিক্ত)
				মুক্ত(oldfile);
		}
		ভঙ্গ;

	পক্ষ DOM_গ্ৰন্থি_RENAMED:
	পক্ষ DOM_গ্ৰন্থি_IMPORTED:
	পক্ষ DOM_গ্ৰন্থি_ADOPTED:
		ভঙ্গ;

	পক্ষ DOM_গ্ৰন্থি_DELETED:
		মুক্ত(data);
		ভঙ্গ;
	স্ৱভাৱ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		প্রতিজ্ঞা(0);
	}
}

স্থির শূন্য html__set_file_gadget_filename(সংরচনা content *c,
	সংরচনা form_control *gadget, ধ্রুৱ ৱর্ণ *fn)
{
	nserror ret;
	ৱর্ণ *utf8_fn, *oldfile = রিক্ত;
	html_content *html = (html_content *)c;
	সংরচনা box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	যদি (ret != NSERROR_OK) {
		প্রতিজ্ঞা(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ঘূৰাই;
	}

	form_gadget_update_মান(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	যদি (dom_গ্ৰন্থি_set_user_data((dom_গ্ৰন্থি *)file_box->gadget->গ্ৰন্থি,
				   corestring_dom_ns_চাবি_file_name_গ্ৰন্থি_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		যদি (oldfile != রিক্ত)
			মুক্ত(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

শূন্য html_set_file_gadget_filename(সংরচনা hlcache_handle *hl,
	সংরচনা form_control *gadget, ধ্রুৱ ৱর্ণ *fn)
{
	ঘূৰাই html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
স্থির bool html_drop_file_at_point(সংরচনা content *c, পূর্ণাঙ্ক x, পূর্ণাঙ্ক y, ৱর্ণ *file)
{
	html_content *html = (html_content *) c;

	সংরচনা box *box = html->layout;
	সংরচনা box *next;
	সংরচনা box *file_box = রিক্ত;
	সংরচনা box *text_box = রিক্ত;
	পূর্ণাঙ্ক box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	যেতিয়ালৈ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != রিক্ত) {
		box = next;

		যদি (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			অনুৱর্ত;

		যদি (box->iframe) {
			ভিন্ন scale = browser_window_get_scale(box->iframe);
			ঘূৰাই browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		যদি (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			ঘূৰাই true;

		যদি (box->gadget) {
			ৱিকল্প (box->gadget->type) {
				পক্ষ GADGET_FILE:
					file_box = box;
				ভঙ্গ;

				পক্ষ GADGET_TEXTBOX:
				পক্ষ GADGET_TEXTAREA:
				পক্ষ GADGET_PASSWORD:
					text_box = box;
					ভঙ্গ;

				স্ৱভাৱ:	/* appease compiler */
					ভঙ্গ;
			}
		}
	}

	যদি (!file_box && !text_box)
		/* No box capable of handling drop */
		ঘূৰাই false;

	যদি (file == রিক্ত)
		/* There is a box capable of handling drop here */
		ঘূৰাই true;

	/* Handle the drop */
	যদি (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} অন্যথা {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = রিক্ত;
		ৱর্ণ *buffer;
		ৱর্ণ *utf8_buff;
		nserror ret;
		unsigned পূর্ণাঙ্ক size;
		পূর্ণাঙ্ক bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		যদি (fp == রিক্ত) {
			/* Couldn't open file, but drop was for us */
			ঘূৰাই true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		যদি ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ঘূৰাই true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		যদি (buffer == রিক্ত) {
			/* No memory, but drop was for us */
			fclose(fp);
			ঘূৰাই true;
		}

		/* Stick file into buffer */
		যদি (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			মুক্ত(buffer);
			fclose(fp);
			ঘূৰাই true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		যদি (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			প্রতিজ্ঞা(ret != NSERROR_BAD_ENCODING);
			মুক্ত(buffer);
			ঘূৰাই true;
		}

		/* Done with buffer */
		মুক্ত(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		মুক্ত(utf8_buff);
	}

	ঘূৰাই true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
স্থির nserror
html_debug(সংরচনা content *c, গণনা content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ঘূৰাই NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
স্থির nserror
html_debug_dump(সংরচনা content *c, FILE *f, গণনা content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_গ্ৰন্থি *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	প্রতিজ্ঞা(htmlc != রিক্ত);

	যদি (op == CONTENT_DEBUG_RENDER) {
		প্রতিজ্ঞা(htmlc->layout != রিক্ত);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} অন্যথা {
		যদি (htmlc->document == রিক্ত) {
			NSLOG(netsurf, INFO, "No document to dump");
			ঘূৰাই NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (শূন্য *) &html);
		যদি ((exc != DOM_NO_ERR) || (html == রিক্ত)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ঘূৰাই NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_গ্ৰন্থি_unref(html);
	}

	ঘূৰাই ret;
}


#যদি ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

স্থির শূন্য
html_dump_frameset(সংরচনা content_html_frames *frame, unsigned পূর্ণাঙ্ক depth)
{
	unsigned পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক row, col, index;
	ধ্রুৱ ৱর্ণ *unit[] = {"px", "%", "*"};
	ধ্রুৱ ৱর্ণ *scrolling[] = {"auto", "yes", "no"};

	প্রতিজ্ঞা(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.মান, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.মান,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	যদি (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	যদি (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	যদি (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	যদি (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned পূর্ণাঙ্ক) frame->border_colour);

	fprintf(stderr, "\n");

	যদি (frame->children) {
		বাবে (row = 0; row != frame->rows; row++) {
			বাবে (col = 0; col != frame->cols; col++) {
				বাবে (i = 0; i != depth; i++)
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

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->document;
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
সংরচনা box *html_get_box_বৃক্ষ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
স্থির ধ্রুৱ ৱর্ণ *html_encoding(ধ্রুৱ সংরচনা content *c, গণনা content_encoding_type op)
{
	html_content *html = (html_content *) c;
	স্থির ৱর্ণ enc_token[10] = "Encoding0";

	প্রতিজ্ঞা(html != রিক্ত);

	যদি (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		ঘূৰাই messages_get(enc_token);
	}

	ঘূৰাই html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
সংরচনা content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
সংরচনা content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->iframe;
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

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ধ্রুৱ ৱর্ণ *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	প্রতিজ্ঞা(c != রিক্ত);

	ঘূৰাই c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, পূর্ণাঙ্ক *x, পূর্ণাঙ্ক *y)
{
	সংরচনা box *pos;
	সংরচনা box *layout;

	যদি (content_get_type(h) != CONTENT_HTML)
		ঘূৰাই false;

	layout = html_get_box_বৃক্ষ(h);

	যদি ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ঘূৰাই true;
	}
	ঘূৰাই false;
}

bool html_exec(সংরচনা content *c, ধ্রুৱ ৱর্ণ *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_গ্ৰন্থি;
	dom_string *dom_src;
	dom_text *text_গ্ৰন্থি;
	dom_গ্ৰন্থি *spare_গ্ৰন্থি;
	dom_html_script_element *script_গ্ৰন্থি;

	যদি (htmlc->document == রিক্ত) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		গচ্ছ out_no_string;
	}

	err = dom_string_create((ধ্রুৱ uint8_t *)src, srclen, &dom_src);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		গচ্ছ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_গ্ৰন্থি);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		গচ্ছ out_no_body;
	}

	err = dom_document_create_text_গ্ৰন্থি(htmlc->document, dom_src, &text_গ্ৰন্থি);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		গচ্ছ out_no_text_গ্ৰন্থি;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_গ্ৰন্থি);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		গচ্ছ out_no_script_গ্ৰন্থি;
	}

	err = dom_গ্ৰন্থি_append_child(script_গ্ৰন্থি, text_গ্ৰন্থি, &spare_গ্ৰন্থি);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		গচ্ছ out_unparented;
	}
	dom_গ্ৰন্থি_unref(spare_গ্ৰন্থি); /* We do not need the spare ref at all */

	err = dom_গ্ৰন্থি_append_child(body_গ্ৰন্থি, script_গ্ৰন্থি, &spare_গ্ৰন্থি);
	যদি (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		গচ্ছ out_unparented;
	}
	dom_গ্ৰন্থি_unref(spare_গ্ৰন্থি); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_গ্ৰন্থি_get_parent_গ্ৰন্থি(script_গ্ৰন্থি, &spare_গ্ৰন্থি);
	যদি (err == DOM_NO_ERR && spare_গ্ৰন্থি != রিক্ত) {
		dom_গ্ৰন্থি *second_spare;
		err = dom_গ্ৰন্থি_remove_child(spare_গ্ৰন্থি, script_গ্ৰন্থি, &second_spare);
		যদি (err == DOM_NO_ERR) {
			dom_গ্ৰন্থি_unref(second_spare);
		}
		dom_গ্ৰন্থি_unref(spare_গ্ৰন্থি);
	}

out_unparented:
	dom_গ্ৰন্থি_unref(script_গ্ৰন্থি);
out_no_script_গ্ৰন্থি:
	dom_গ্ৰন্থি_unref(text_গ্ৰন্থি);
out_no_text_গ্ৰন্থি:
	dom_গ্ৰন্থি_unref(body_গ্ৰন্থি);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	ঘূৰাই result;
}

/* See \ref content_saw_insecure_objects */
স্থির bool
html_saw_insecure_objects(সংরচনা content *c)
{
	html_content *htmlc = (html_content *)c;
	সংরচনা content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	যেতিয়ালৈ (obj != রিক্ত) {
		যদি (obj->content != রিক্ত) {
			যদি (content_saw_insecure_objects(obj->content))
				ঘূৰাই true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	যদি (html_saw_insecure_scripts(htmlc)) {
		ঘূৰাই true;
	}

	/* Now check stylesheets */
	যদি (html_css_saw_insecure_stylesheets(htmlc)) {
		ঘূৰাই true;
	}

	ঘূৰাই false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
স্থির content_type html_content_type(শূন্য)
{
	ঘূৰাই CONTENT_HTML;
}


স্থির শূন্য html_fini(শূন্য)
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
স্থির nserror
find_occurrences_html_box(ধ্রুৱ ৱর্ণ *pattern,
			  পূর্ণাঙ্ক p_len,
			  সংরচনা box *cur,
			  bool case_sens,
			  সংরচনা textsearch_context *context)
{
	সংরচনা box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	যদি (!cur->object && cur->text) {
		ধ্রুৱ ৱর্ণ *text = cur->text;
		unsigned length = cur->length;

		যেতিয়ালৈ (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			ধ্রুৱ ৱর্ণ *new_text;
			ধ্রুৱ ৱর্ণ *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			যদি (!pos)
				ভঙ্গ;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			যদি (res != NSERROR_OK) {
				ঘূৰাই res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	বাবে (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		যদি (res != NSERROR_OK) {
			ঘূৰাই res;
		}
	}

	ঘূৰাই res;
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
স্থির nserror
html_textsearch_find(সংরচনা content *c,
		     সংরচনা textsearch_context *context,
		     ধ্রুৱ ৱর্ণ *pattern,
		     পূর্ণাঙ্ক p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	যদি (html->layout == রিক্ত) {
		ঘূৰাই NSERROR_INVALID;
	}

	ঘূৰাই find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


স্থির nserror
html_textsearch_bounds(সংরচনা content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       সংরচনা box *start_box,
		       সংরচনা box *end_box,
		       সংরচনা rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	ঘূৰাই NSERROR_OK;
}


/**
 * HTML content handler function table
 */
স্থির ধ্রুৱ content_handler html_content_handler = {
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
nserror html_init(শূন্য)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	যদি (error != NSERROR_OK)
		গচ্ছ error;

	বাবে (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		যদি (error != NSERROR_OK)
			গচ্ছ error;
	}

	ঘূৰাই NSERROR_OK;

error:
	html_fini();

	ঘূৰাই error;
}
