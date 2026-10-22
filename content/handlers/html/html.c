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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <nsutils/time.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/http.h"
#அந்தர்பாவ "utils/libdom.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/utf8.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/string.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/utf8.h"
#அந்தர்பாவ "netsurf/keypress.h"
#அந்தர்பாவ "netsurf/layout.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/textsearch.h"
#அந்தர்பாவ "desktop/selection.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/textarea.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "javascript/js.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/dom_event.h"
#அந்தர்பாவ "html/css.h"
#அந்தர்பாவ "html/object.h"
#அந்தர்பாவ "html/html_save.h"
#அந்தர்பாவ "html/interaction.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_construct.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/imagemap.h"
#அந்தர்பாவ "html/layout.h"
#அந்தர்பாவ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

நிலையான மாறிலி எழுத்து *html_types[] = {
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
நிலையான bool fire_dom_event(dom_event *event, dom_முனை *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	எனில் (exc != DOM_NO_ERR) {
		திரும்பு false;
	}

	திரும்பு result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_முனை *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	எனில் (exc != DOM_NO_ERR) திரும்பு false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	எனில் (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		திரும்பு false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (முழு)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	திரும்பு result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_முனை *target,
		bool bubbles, bool cancelable, uint32_t சாவி)
{
	bool is_special = சாவி <= 0x001F || (0x007F <= சாவி && சாவி <= 0x009F);
	dom_string *dom_சாவி = வெற்று;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	எனில் (is_special) {
		தேர்வு (சாவி) {
		நிலை NS_சாவி_ESCAPE:
			dom_சாவி = dom_string_ref(corestring_dom_Escape);
			நிறுத்து;
		நிலை NS_சாவி_இடது:
			dom_சாவி = dom_string_ref(corestring_dom_Arrow_இடது);
			நிறுத்து;
		நிலை NS_சாவி_வலது:
			dom_சாவி = dom_string_ref(corestring_dom_Arrow_வலது);
			நிறுத்து;
		நிலை NS_சாவி_UP:
			dom_சாவி = dom_string_ref(corestring_dom_ArrowUp);
			நிறுத்து;
		நிலை NS_சாவி_DOWN:
			dom_சாவி = dom_string_ref(corestring_dom_ArrowDown);
			நிறுத்து;
		நிலை NS_சாவி_PAGE_UP:
			dom_சாவி = dom_string_ref(corestring_dom_PageUp);
			நிறுத்து;
		நிலை NS_சாவி_PAGE_DOWN:
			dom_சாவி = dom_string_ref(corestring_dom_PageDown);
			நிறுத்து;
		நிலை NS_சாவி_TEXT_START:
			dom_சாவி = dom_string_ref(corestring_dom_Home);
			நிறுத்து;
		நிலை NS_சாவி_TEXT_END:
			dom_சாவி = dom_string_ref(corestring_dom_End);
			நிறுத்து;
		இயல்பு:
			dom_சாவி = வெற்று;
			நிறுத்து;
		}
	} இல்லையெனில் {
		எழுத்து utf8[6];
		size_t length = utf8_from_ucs4(சாவி, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((மாறிலி uint8_t *)utf8, strlen(utf8),
				&dom_சாவி);
		எனில் (exc != DOM_NO_ERR) {
			திரும்பு exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	எனில் (exc != DOM_NO_ERR) {
		dom_string_unref(dom_சாவி);
		திரும்பு false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, வெற்று,
			dom_சாவி, வெற்று, DOM_சாவி_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_சாவி);
	எனில் (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		திரும்பு false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(முழு)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	திரும்பு result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
நிலையான வெறுமை html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_முனை *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = வெற்று;

	/* Clean up and report error if unsuccessful or aborted */
	எனில் ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		எனில் (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, வெற்று);
		} இல்லையெனில் {
			content_broadcast_error(&c->base, NSERROR_STOPPED, வெற்று);
		}

		content_set_error(&c->base);
		திரும்பு;
	}


#எனில் ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#எனில் ALWAYS_DUMP_FRAMESET
	எனில் (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (வெறுமை *) &html);
	எனில் ((exc != DOM_NO_ERR) || (html == வெற்று)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, வெற்று);
		content_set_error(&c->base);
		திரும்பு;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	எனில் (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, வெற்று);
		content_set_error(&c->base);
		dom_முனை_unref(html);
		திரும்பு;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = வெற்று;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_முனை_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	தேர்வு (content__get_status(&html->base)) {
	நிலை CONTENT_STATUS_READY:
		எனில் (html->base.active == 0) {
			content_set_done(&html->base);
			திரும்பு NSERROR_OK;
		}
		நிறுத்து;
	நிலை CONTENT_STATUS_DONE:
		/* fallthrough */
	நிலை CONTENT_STATUS_LOADING:
		திரும்பு NSERROR_OK;
	இயல்பு:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		நிறுத்து;
	}
	திரும்பு NSERROR_UNKNOWN;
}


நிலையான வெறுமை html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	ஒன்றிப்பு content_செய்தி_data செய்தி_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_செய்தி_GETDIMS, &செய்தி_data);


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
வெறுமை html_finish_conversion(html_content *htmlc)
{
	ஒன்றிப்பு content_செய்தி_data செய்தி_data;
	dom_exception exc; /* returned by libdom functions */
	dom_முனை *html;
	nserror error;

	/* Bail out if we've been aborted */
	எனில் (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, வெற்று);
		content_set_error(&htmlc->base);
		திரும்பு;
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
	எனில் (htmlc->select_ctx != வெற்று) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		திரும்பு;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	எனில் (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, வெற்று);
		content_set_error(&htmlc->base);
		திரும்பு;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	எனில் (htmlc->jsthread != வெற்று) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, வெற்று);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	செய்தி_data.explicit_status_text = வெற்று;
	content_broadcast(&htmlc->base, CONTENT_செய்தி_STATUS, &செய்தி_data);

	exc = dom_document_get_document_element(htmlc->document, (வெறுமை *) &html);
	எனில் ((exc != DOM_NO_ERR) || (html == வெற்று)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, வெற்று);
		content_set_error(&htmlc->base);
		திரும்பு;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	எனில் (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_முனை_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, வெற்று);
		content_set_error(&htmlc->base);
		திரும்பு;
	}

	dom_முனை_unref(html);
}


நிலையான வெறுமை
html_document_user_data_handler(dom_முனை_operation operation,
				dom_string *சாவி, வெறுமை *data,
				கட்டமைப்பு dom_முனை *src,
				கட்டமைப்பு dom_முனை *dst)
{
	எனில் (dom_string_isequal(corestring_dom_ns_சாவி_html_content_data,
			       சாவி) == false || data == வெற்று) {
		திரும்பு;
	}

	தேர்வு (operation) {
	நிலை DOM_முனை_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		நிறுத்து;
	நிலை DOM_முனை_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		நிறுத்து;
	நிலை DOM_முனை_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		நிறுத்து;
	நிலை DOM_முனை_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		நிறுத்து;
	நிலை DOM_முனை_DELETED:
		/* This is the only path I expect */
		நிறுத்து;
	இயல்பு:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ப்ரதிஜ்ஞா(0);
	}
}


நிலையான nserror
html_create_html_data(html_content *c, மாறிலி http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	வெறுமை *old_முனை_data;
	மாறிலி எழுத்து *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = வெற்று;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = வெற்று;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = வெற்று;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = வெற்று;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = வெற்று;
	c->bctx = வெற்று;
	c->layout = வெற்று;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_மொத்தம் = 0;
	c->stylesheets = வெற்று;
	c->select_ctx = வெற்று;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = வெற்று;
	c->இலக்கம்_objects = 0;
	c->object_list = வெற்று;
	c->forms = வெற்று;
	c->imagemaps = வெற்று;
	c->bw = வெற்று;
	c->frameset = வெற்று;
	c->iframe = வெற்று;
	c->page = வெற்று;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_மொத்தம் = 0;
	c->scripts = வெற்று;
	c->jsthread = வெற்று;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	எனில் (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = வெற்று;
		திரும்பு NSERROR_NOMEM;
	}

	c->sel = selection_create((கட்டமைப்பு content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	எனில் (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		எனில் (c->encoding == வெற்று) {
			lwc_string_unref(c->universal);
			c->universal = வெற்று;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = வெற்று;
			திரும்பு NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.செய்தி = வெற்று;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	எனில் ((error != DOM_HUBBUB_OK) && (c->encoding != வெற்று)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		விடுவி(c->encoding);
		c->encoding = வெற்று;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	எனில் (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = வெற்று;

		lwc_string_unref(c->universal);
		c->universal = வெற்று;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = வெற்று;

		திரும்பு libdom_hubbub_error_to_nserror(error);
	}

	err = dom_முனை_set_user_data(c->document,
				     corestring_dom_ns_சாவி_html_content_data,
				     c, html_document_user_data_handler,
				     (வெறுமை *) &old_முனை_data);
	எனில் (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = வெற்று;
		nsurl_unref(c->base_url);
		c->base_url = வெற்று;

		lwc_string_unref(c->universal);
		c->universal = வெற்று;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = வெற்று;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		திரும்பு NSERROR_DOM;
	}

	ப்ரதிஜ்ஞா(old_முனை_data == வெற்று);

	திரும்பு NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

நிலையான nserror
html_create(மாறிலி content_handler *handler,
	    lwc_string *imime_type,
	    மாறிலி http_parameter *params,
	    llcache_handle *llcache,
	    மாறிலி எழுத்து *fallback_charset,
	    bool quirks,
	    கட்டமைப்பு content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, அளவு(html_content));
	எனில் (html == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(html);
		திரும்பு error;
	}

	error = html_create_html_data(html, params);
	எனில் (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, வெற்று);
		விடுவி(html);
		திரும்பு error;
	}

	error = html_css_new_stylesheets(html);
	எனில் (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, வெற்று);
		விடுவி(html);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) html;

	திரும்பு NSERROR_OK;
}



நிலையான nserror
html_process_encoding_change(கட்டமைப்பு content *c,
			     மாறிலி எழுத்து *data,
			     unsigned முழு size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	மாறிலி எழுத்து *encoding;
	மாறிலி uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	எனில் (encoding == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (html->encoding != வெற்று) {
		விடுவி(html->encoding);
		html->encoding = வெற்று;
	}

	html->encoding = strdup(encoding);
	எனில் (html->encoding == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = வெற்று;

	எனில் (html->document != வெற்று) {
		dom_முனை_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.செய்தி = வெற்று;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	எனில் (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		விடுவி(html->encoding);
		html->encoding = strdup("Windows-1252");
		எனில் (html->encoding == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		எனில் (error != DOM_HUBBUB_OK) {
			திரும்பு libdom_hubbub_error_to_nserror(error);
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

	திரும்பு libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

நிலையான bool
html_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data, unsigned முழு size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (மாறிலி uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	எனில் (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	எனில் (err != NSERROR_OK) {
		content_broadcast_error(c, err, வெற்று);
		திரும்பு false;
	}

	திரும்பு true;
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

நிலையான bool html_convert(கட்டமைப்பு content *c)
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
	எனில் (exc == DOM_NO_ERR) {
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
	எனில் (html_can_begin_conversion(htmlc)) {
		திரும்பு html_begin_conversion(htmlc);
	}
	திரும்பு true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned முழு i;

	/* Cannot begin conversion if we're still fetching stuff */
	எனில் (htmlc->base.active != 0)
		திரும்பு false;

	க்கு (i = 0; i != htmlc->stylesheet_மொத்தம்; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		எனில் (htmlc->stylesheets[i].modified)
			திரும்பு false;
	}

	/* All is good, begin */
	திரும்பு true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_முனை *html;
	nserror ns_error;
	கட்டமைப்பு form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *முனை_name = வெற்று;
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
	எனில் (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		எனில் (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			திரும்பு true;
		}
		எனில் (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						வெற்று);

			திரும்பு false;
		}
		htmlc->parse_completed = true;
	}

	எனில் (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		திரும்பு true;
	}

	/* Give up processing if we've been aborted */
	எனில் (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, வெற்று);
		திரும்பு false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	எனில் (htmlc->encoding == வெற்று) {
		மாறிலி எழுத்து *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		எனில் (encoding == வெற்று) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						வெற்று);
			திரும்பு false;
		}

		htmlc->encoding = strdup(encoding);
		எனில் (htmlc->encoding == வெற்று) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						வெற்று);
			திரும்பு false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (வெறுமை *) &html);
	எனில் ((exc != DOM_NO_ERR) || (html == வெற்று)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, வெற்று);
		திரும்பு false;
	}

	exc = dom_முனை_get_முனை_name(html, &முனை_name);
	எனில் ((exc != DOM_NO_ERR) ||
	    (முனை_name == வெற்று) ||
	    (!dom_string_caseless_lwc_isequal(முனை_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, வெற்று);
		dom_முனை_unref(html);
		திரும்பு false;
	}
	dom_string_unref(முனை_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	க்கு (f = htmlc->forms; f != வெற்று; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		எனில் (f->action == வெற்று || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} இல்லையெனில் {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		எனில் (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, வெற்று);

			dom_முனை_unref(html);
			திரும்பு false;
		}

		விடுவி(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		எனில் (f->action == வெற்று) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						வெற்று);

			dom_முனை_unref(html);
			திரும்பு false;
		}

		/* Ensure each form has a document encoding */
		எனில் (f->document_charset == வெற்று) {
			f->document_charset = strdup(htmlc->encoding);
			எனில் (f->document_charset == வெற்று) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							வெற்று);
				dom_முனை_unref(html);
				திரும்பு false;
			}
		}
	}

	dom_முனை_unref(html);

	எனில் (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	திரும்பு true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

நிலையான வெறுமை html_stop(கட்டமைப்பு content *c)
{
	html_content *htmlc = (html_content *) c;

	தேர்வு (c->status) {
	நிலை CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		எனில் (htmlc->jsthread != வெற்று) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		நிறுத்து;

	நிலை CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		எனில் (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		நிறுத்து;

	நிலை CONTENT_STATUS_DONE:
		/* Nothing to do */
		நிறுத்து;

	இயல்பு:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		ப்ரதிஜ்ஞா(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

நிலையான வெறுமை html_reformat(கட்டமைப்பு content *c, முழு width, முழு height)
{
	html_content *htmlc = (html_content *) c;
	கட்டமைப்பு box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.வேர்_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[இடது_2] + layout->width +
		layout->padding[வலது_2] + layout->border[வலது_2].width +
		layout->margin[வலது_2];
	c->height = layout->y + layout->padding[உச்சி_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	எனில் (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	எனில் (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	எனில் (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

வெறுமை html_redraw_a_box(hlcache_handle *h, கட்டமைப்பு box *box)
{
	முழு x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[இடது_2] + box->width + box->padding[வலது_2],
			box->padding[உச்சி_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

வெறுமை html__redraw_a_box(கட்டமைப்பு html_content *html, கட்டமைப்பு box *box)
{
	முழு x, y;

	box_coords(box, &x, &y);

	content__request_redraw((கட்டமைப்பு content *)html, x, y,
			box->padding[இடது_2] + box->width + box->padding[வலது_2],
			box->padding[உச்சி_2] + box->height + box->padding[BOTTOM]);
}

நிலையான வெறுமை html_destroy_frameset(கட்டமைப்பு content_html_frames *frameset)
{
	முழு i;

	எனில் (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = வெற்று;
	}
	எனில் (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = வெற்று;
	}
	எனில் (frameset->children) {
		க்கு (i = 0; i < (frameset->rows * frameset->cols); i++) {
			எனில் (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = வெற்று;
			}
			எனில் (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = வெற்று;
			}
			எனில் (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = வெற்று;
	}
}

நிலையான வெறுமை html_destroy_iframe(கட்டமைப்பு content_html_iframe *iframe)
{
	கட்டமைப்பு content_html_iframe *next;
	next = iframe;
	போது ((iframe = next) != வெற்று) {
		next = iframe->next;
		எனில் (iframe->name)
			talloc_free(iframe->name);
		எனில் (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = வெற்று;
		}
		talloc_free(iframe);
	}
}


நிலையான வெறுமை html_free_layout(html_content *htmlc)
{
	எனில் (htmlc->bctx != வெற்று) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

நிலையான வெறுமை html_destroy(கட்டமைப்பு content *c)
{
	html_content *html = (html_content *) c;
	கட்டமைப்பு form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	எனில் (html->box_conversion_context != வெற்று) {
		எனில் (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	க்கு (f = html->forms; f != வெற்று; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	எனில் (c->refresh)
		nsurl_unref(c->refresh);

	எனில் (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	எனில் (html->jsthread != வெற்று) {
		js_destroythread(html->jsthread);
		html->jsthread = வெற்று;
	}

	எனில் (html->parser != வெற்று) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = வெற்று;
	}

	எனில் (html->document != வெற்று) {
		dom_முனை_unref(html->document);
		html->document = வெற்று;
	}

	எனில் (html->title != வெற்று) {
		dom_முனை_unref(html->title);
		html->title = வெற்று;
	}

	/* Free encoding */
	எனில் (html->encoding != வெற்று) {
		விடுவி(html->encoding);
		html->encoding = வெற்று;
	}

	/* Free base target */
	எனில் (html->base_target != வெற்று) {
		விடுவி(html->base_target);
		html->base_target = வெற்று;
	}

	/* Free frameset */
	எனில் (html->frameset != வெற்று) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = வெற்று;
	}

	/* Free iframes */
	எனில் (html->iframe != வெற்று) {
		html_destroy_iframe(html->iframe);
		html->iframe = வெற்று;
	}

	/* Destroy selection context */
	எனில் (html->select_ctx != வெற்று) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = வெற்று;
	}

	lwc_string_unref(html->universal);
	html->universal = வெற்று;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = வெற்று;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


நிலையான nserror html_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ப்ரதிஜ்ஞா(0 && "html_clone should never be called");

	திரும்பு true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

நிலையான nserror
html_open(கட்டமைப்பு content *c,
	  கட்டமைப்பு browser_window *bw,
	  கட்டமைப்பு content *page,
	  கட்டமைப்பு object_params *params)
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

	திரும்பு NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

நிலையான nserror html_close(கட்டமைப்பு content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = வெற்று;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	எனில் (htmlc->jsthread != வெற்று) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	திரும்பு ret;
}


/**
 * Return an HTML content's selection context
 */

நிலையான வெறுமை html_clear_selection(கட்டமைப்பு content *c)
{
	html_content *html = (html_content *) c;

	தேர்வு (html->selection_type) {
	நிலை HTML_SELECTION_NONE:
		/* Nothing to do */
		ப்ரதிஜ்ஞா(html->selection_owner.none == true);
		நிறுத்து;
	நிலை HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		நிறுத்து;
	நிலை HTML_SELECTION_SELF:
		ப்ரதிஜ்ஞா(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		நிறுத்து;
	நிலை HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

நிலையான எழுத்து *html_get_selection(கட்டமைப்பு content *c)
{
	html_content *html = (html_content *) c;

	தேர்வு (html->selection_type) {
	நிலை HTML_SELECTION_TEXTAREA:
		திரும்பு textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	நிலை HTML_SELECTION_SELF:
		ப்ரதிஜ்ஞா(html->selection_owner.none == false);
		திரும்பு selection_get_copy(html->sel);
	நிலை HTML_SELECTION_CONTENT:
		திரும்பு content_get_selection(
				html->selection_owner.content->object);
	நிலை HTML_SELECTION_NONE:
		/* Nothing to do */
		ப்ரதிஜ்ஞா(html->selection_owner.none == true);
		நிறுத்து;
	இயல்பு:
		நிறுத்து;
	}

	திரும்பு வெற்று;
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
நிலையான nserror
html_get_contextual_content(கட்டமைப்பு content *c, முழு x, முழு y,
			    கட்டமைப்பு browser_window_features *data)
{
	html_content *html = (html_content *) c;

	கட்டமைப்பு box *box = html->layout;
	கட்டமைப்பு box *next;
	முழு box_x = 0, box_y = 0;

	போது ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != வெற்று) {
		box = next;

		/* hidden boxes are ignored */
		எனில் ((box->style != வெற்று) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			தொடர்;
		}

		எனில் (box->iframe) {
			தசம scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		எனில் (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		எனில் (box->object)
			data->object = box->object;

		எனில் (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		எனில் (box->usemap) {
			மாறிலி எழுத்து *target = வெற்று;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			எனில் (url != வெற்று)
				data->link = url;
		}
		எனில் (box->gadget) {
			தேர்வு (box->gadget->type) {
			நிலை GADGET_TEXTBOX:
			நிலை GADGET_TEXTAREA:
			நிலை GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				நிறுத்து;

			நிலை GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				நிறுத்து;

			இயல்பு:
				data->form_features = CTX_FORM_NONE;
				நிறுத்து;
			}
		}
	}
	திரும்பு NSERROR_OK;
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
நிலையான bool
html_scroll_at_point(கட்டமைப்பு content *c, முழு x, முழு y, முழு scrx, முழு scry)
{
	html_content *html = (html_content *) c;

	கட்டமைப்பு box *box = html->layout;
	கட்டமைப்பு box *next;
	முழு box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	போது ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != வெற்று) {
		box = next;

		எனில் (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			தொடர்;

		/* Pass into iframe */
		எனில் (box->iframe) {
			தசம scale = browser_window_get_scale(box->iframe);

			எனில் (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				திரும்பு true;
		}

		/* Pass into textarea widget */
		எனில் (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			திரும்பு true;

		/* Pass into object */
		எனில் (box->object != வெற்று && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			திரும்பு true;

		/* Handle box scrollbars */
		எனில் (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		எனில் (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		எனில் (handled_scroll == true)
			திரும்பு true;
	}

	திரும்பு false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
நிலையான வெறுமை html__dom_user_data_handler(dom_முனை_operation operation,
		dom_string *சாவி, வெறுமை *_data, கட்டமைப்பு dom_முனை *src,
		கட்டமைப்பு dom_முனை *dst)
{
	எழுத்து *oldfile;
	எழுத்து *data = (எழுத்து *)_data;

	எனில் (!dom_string_isequal(corestring_dom_ns_சாவி_file_name_முனை_data,
				சாவி) || data == வெற்று) {
		திரும்பு;
	}

	தேர்வு (operation) {
	நிலை DOM_முனை_CLONED:
		எனில் (dom_முனை_set_user_data(dst,
					   corestring_dom_ns_சாவி_file_name_முனை_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			எனில் (oldfile != வெற்று)
				விடுவி(oldfile);
		}
		நிறுத்து;

	நிலை DOM_முனை_RENAMED:
	நிலை DOM_முனை_IMPORTED:
	நிலை DOM_முனை_ADOPTED:
		நிறுத்து;

	நிலை DOM_முனை_DELETED:
		விடுவி(data);
		நிறுத்து;
	இயல்பு:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ப்ரதிஜ்ஞா(0);
	}
}

நிலையான வெறுமை html__set_file_gadget_filename(கட்டமைப்பு content *c,
	கட்டமைப்பு form_control *gadget, மாறிலி எழுத்து *fn)
{
	nserror ret;
	எழுத்து *utf8_fn, *oldfile = வெற்று;
	html_content *html = (html_content *)c;
	கட்டமைப்பு box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	எனில் (ret != NSERROR_OK) {
		ப்ரதிஜ்ஞா(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		திரும்பு;
	}

	form_gadget_update_மதிப்பு(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	எனில் (dom_முனை_set_user_data((dom_முனை *)file_box->gadget->முனை,
				   corestring_dom_ns_சாவி_file_name_முனை_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		எனில் (oldfile != வெற்று)
			விடுவி(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

வெறுமை html_set_file_gadget_filename(கட்டமைப்பு hlcache_handle *hl,
	கட்டமைப்பு form_control *gadget, மாறிலி எழுத்து *fn)
{
	திரும்பு html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
நிலையான bool html_drop_file_at_point(கட்டமைப்பு content *c, முழு x, முழு y, எழுத்து *file)
{
	html_content *html = (html_content *) c;

	கட்டமைப்பு box *box = html->layout;
	கட்டமைப்பு box *next;
	கட்டமைப்பு box *file_box = வெற்று;
	கட்டமைப்பு box *text_box = வெற்று;
	முழு box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	போது ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != வெற்று) {
		box = next;

		எனில் (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			தொடர்;

		எனில் (box->iframe) {
			தசம scale = browser_window_get_scale(box->iframe);
			திரும்பு browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		எனில் (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			திரும்பு true;

		எனில் (box->gadget) {
			தேர்வு (box->gadget->type) {
				நிலை GADGET_FILE:
					file_box = box;
				நிறுத்து;

				நிலை GADGET_TEXTBOX:
				நிலை GADGET_TEXTAREA:
				நிலை GADGET_PASSWORD:
					text_box = box;
					நிறுத்து;

				இயல்பு:	/* appease compiler */
					நிறுத்து;
			}
		}
	}

	எனில் (!file_box && !text_box)
		/* No box capable of handling drop */
		திரும்பு false;

	எனில் (file == வெற்று)
		/* There is a box capable of handling drop here */
		திரும்பு true;

	/* Handle the drop */
	எனில் (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} இல்லையெனில் {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = வெற்று;
		எழுத்து *buffer;
		எழுத்து *utf8_buff;
		nserror ret;
		unsigned முழு size;
		முழு bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		எனில் (fp == வெற்று) {
			/* Couldn't open file, but drop was for us */
			திரும்பு true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		எனில் ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			திரும்பு true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		எனில் (buffer == வெற்று) {
			/* No memory, but drop was for us */
			fclose(fp);
			திரும்பு true;
		}

		/* Stick file into buffer */
		எனில் (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			விடுவி(buffer);
			fclose(fp);
			திரும்பு true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		எனில் (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ப்ரதிஜ்ஞா(ret != NSERROR_BAD_ENCODING);
			விடுவி(buffer);
			திரும்பு true;
		}

		/* Done with buffer */
		விடுவி(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		விடுவி(utf8_buff);
	}

	திரும்பு true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
நிலையான nserror
html_debug(கட்டமைப்பு content *c, எண்ணிக்கை content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	திரும்பு NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
நிலையான nserror
html_debug_dump(கட்டமைப்பு content *c, FILE *f, எண்ணிக்கை content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_முனை *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ப்ரதிஜ்ஞா(htmlc != வெற்று);

	எனில் (op == CONTENT_DEBUG_RENDER) {
		ப்ரதிஜ்ஞா(htmlc->layout != வெற்று);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} இல்லையெனில் {
		எனில் (htmlc->document == வெற்று) {
			NSLOG(netsurf, INFO, "No document to dump");
			திரும்பு NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (வெறுமை *) &html);
		எனில் ((exc != DOM_NO_ERR) || (html == வெற்று)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			திரும்பு NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_முனை_unref(html);
	}

	திரும்பு ret;
}


#எனில் ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

நிலையான வெறுமை
html_dump_frameset(கட்டமைப்பு content_html_frames *frame, unsigned முழு depth)
{
	unsigned முழு i;
	முழு row, col, index;
	மாறிலி எழுத்து *unit[] = {"px", "%", "*"};
	மாறிலி எழுத்து *scrolling[] = {"auto", "yes", "no"};

	ப்ரதிஜ்ஞா(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.மதிப்பு, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.மதிப்பு,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	எனில் (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	எனில் (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	எனில் (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	எனில் (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned முழு) frame->border_colour);

	fprintf(stderr, "\n");

	எனில் (frame->children) {
		க்கு (row = 0; row != frame->rows; row++) {
			க்கு (col = 0; col != frame->cols; col++) {
				க்கு (i = 0; i != depth; i++)
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

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->document;
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
கட்டமைப்பு box *html_get_box_மரம்(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
நிலையான மாறிலி எழுத்து *html_encoding(மாறிலி கட்டமைப்பு content *c, எண்ணிக்கை content_encoding_type op)
{
	html_content *html = (html_content *) c;
	நிலையான எழுத்து enc_token[10] = "Encoding0";

	ப்ரதிஜ்ஞா(html != வெற்று);

	எனில் (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		திரும்பு messages_get(enc_token);
	}

	திரும்பு html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
கட்டமைப்பு content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
கட்டமைப்பு content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->iframe;
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

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
மாறிலி எழுத்து *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ப்ரதிஜ்ஞா(c != வெற்று);

	திரும்பு c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, முழு *x, முழு *y)
{
	கட்டமைப்பு box *pos;
	கட்டமைப்பு box *layout;

	எனில் (content_get_type(h) != CONTENT_HTML)
		திரும்பு false;

	layout = html_get_box_மரம்(h);

	எனில் ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		திரும்பு true;
	}
	திரும்பு false;
}

bool html_exec(கட்டமைப்பு content *c, மாறிலி எழுத்து *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_முனை;
	dom_string *dom_src;
	dom_text *text_முனை;
	dom_முனை *spare_முனை;
	dom_html_script_element *script_முனை;

	எனில் (htmlc->document == வெற்று) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		செல் out_no_string;
	}

	err = dom_string_create((மாறிலி uint8_t *)src, srclen, &dom_src);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		செல் out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_முனை);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		செல் out_no_body;
	}

	err = dom_document_create_text_முனை(htmlc->document, dom_src, &text_முனை);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		செல் out_no_text_முனை;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_முனை);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		செல் out_no_script_முனை;
	}

	err = dom_முனை_append_child(script_முனை, text_முனை, &spare_முனை);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		செல் out_unparented;
	}
	dom_முனை_unref(spare_முனை); /* We do not need the spare ref at all */

	err = dom_முனை_append_child(body_முனை, script_முனை, &spare_முனை);
	எனில் (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		செல் out_unparented;
	}
	dom_முனை_unref(spare_முனை); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_முனை_get_parent_முனை(script_முனை, &spare_முனை);
	எனில் (err == DOM_NO_ERR && spare_முனை != வெற்று) {
		dom_முனை *second_spare;
		err = dom_முனை_remove_child(spare_முனை, script_முனை, &second_spare);
		எனில் (err == DOM_NO_ERR) {
			dom_முனை_unref(second_spare);
		}
		dom_முனை_unref(spare_முனை);
	}

out_unparented:
	dom_முனை_unref(script_முனை);
out_no_script_முனை:
	dom_முனை_unref(text_முனை);
out_no_text_முனை:
	dom_முனை_unref(body_முனை);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	திரும்பு result;
}

/* See \ref content_saw_insecure_objects */
நிலையான bool
html_saw_insecure_objects(கட்டமைப்பு content *c)
{
	html_content *htmlc = (html_content *)c;
	கட்டமைப்பு content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	போது (obj != வெற்று) {
		எனில் (obj->content != வெற்று) {
			எனில் (content_saw_insecure_objects(obj->content))
				திரும்பு true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	எனில் (html_saw_insecure_scripts(htmlc)) {
		திரும்பு true;
	}

	/* Now check stylesheets */
	எனில் (html_css_saw_insecure_stylesheets(htmlc)) {
		திரும்பு true;
	}

	திரும்பு false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
நிலையான content_type html_content_type(வெறுமை)
{
	திரும்பு CONTENT_HTML;
}


நிலையான வெறுமை html_fini(வெறுமை)
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
நிலையான nserror
find_occurrences_html_box(மாறிலி எழுத்து *pattern,
			  முழு p_len,
			  கட்டமைப்பு box *cur,
			  bool case_sens,
			  கட்டமைப்பு textsearch_context *context)
{
	கட்டமைப்பு box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	எனில் (!cur->object && cur->text) {
		மாறிலி எழுத்து *text = cur->text;
		unsigned length = cur->length;

		போது (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			மாறிலி எழுத்து *new_text;
			மாறிலி எழுத்து *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			எனில் (!pos)
				நிறுத்து;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	க்கு (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		எனில் (res != NSERROR_OK) {
			திரும்பு res;
		}
	}

	திரும்பு res;
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
நிலையான nserror
html_textsearch_find(கட்டமைப்பு content *c,
		     கட்டமைப்பு textsearch_context *context,
		     மாறிலி எழுத்து *pattern,
		     முழு p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	எனில் (html->layout == வெற்று) {
		திரும்பு NSERROR_INVALID;
	}

	திரும்பு find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


நிலையான nserror
html_textsearch_bounds(கட்டமைப்பு content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       கட்டமைப்பு box *start_box,
		       கட்டமைப்பு box *end_box,
		       கட்டமைப்பு rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	திரும்பு NSERROR_OK;
}


/**
 * HTML content handler function table
 */
நிலையான மாறிலி content_handler html_content_handler = {
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
nserror html_init(வெறுமை)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	எனில் (error != NSERROR_OK)
		செல் error;

	க்கு (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		எனில் (error != NSERROR_OK)
			செல் error;
	}

	திரும்பு NSERROR_OK;

error:
	html_fini();

	திரும்பு error;
}
