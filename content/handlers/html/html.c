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

#شامل <assert.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <stdlib.h>
#شامل <nsutils/time.h>

#شامل "utils/utils.h"
#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/http.h"
#شامل "utils/libdom.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/talloc.h"
#شامل "utils/utf8.h"
#شامل "utils/nsoption.h"
#شامل "utils/string.h"
#شامل "utils/ascii.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/utf8.h"
#شامل "netsurf/keypress.h"
#شامل "netsurf/layout.h"
#شامل "netsurf/misc.h"
#شامل "content/hlcache.h"
#شامل "content/content_factory.h"
#شامل "content/textsearch.h"
#شامل "desktop/selection.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"
#شامل "netsurf/bitmap.h"
#شامل "javascript/js.h"
#شامل "desktop/gui_internal.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/dom_event.h"
#شامل "html/css.h"
#شامل "html/object.h"
#شامل "html/html_save.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_construct.h"
#شامل "html/box_inspect.h"
#شامل "html/form_internal.h"
#شامل "html/imagemap.h"
#شامل "html/layout.h"
#شامل "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

ایستا ثابت نویسه *html_types[] = {
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
ایستا bool fire_dom_event(dom_event *event, dom_گره *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	اگر (exc != DOM_NO_ERR) {
		بازگشت false;
	}

	بازگشت result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_گره *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	اگر (exc != DOM_NO_ERR) بازگشت false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		بازگشت false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (صحیح)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	بازگشت result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_گره *target,
		bool bubbles, bool cancelable, uint32_t کلید)
{
	bool is_special = کلید <= 0x001F || (0x007F <= کلید && کلید <= 0x009F);
	dom_string *dom_کلید = پوچ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	اگر (is_special) {
		انتخاب (کلید) {
		حالت NS_کلید_ESCAPE:
			dom_کلید = dom_string_ref(corestring_dom_Escape);
			بشکن;
		حالت NS_کلید_چپ:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_چپ);
			بشکن;
		حالت NS_کلید_راست:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_راست);
			بشکن;
		حالت NS_کلید_UP:
			dom_کلید = dom_string_ref(corestring_dom_ArrowUp);
			بشکن;
		حالت NS_کلید_DOWN:
			dom_کلید = dom_string_ref(corestring_dom_ArrowDown);
			بشکن;
		حالت NS_کلید_PAGE_UP:
			dom_کلید = dom_string_ref(corestring_dom_PageUp);
			بشکن;
		حالت NS_کلید_PAGE_DOWN:
			dom_کلید = dom_string_ref(corestring_dom_PageDown);
			بشکن;
		حالت NS_کلید_TEXT_START:
			dom_کلید = dom_string_ref(corestring_dom_Home);
			بشکن;
		حالت NS_کلید_TEXT_END:
			dom_کلید = dom_string_ref(corestring_dom_End);
			بشکن;
		پیشفرض:
			dom_کلید = پوچ;
			بشکن;
		}
	} وگرنه {
		نویسه utf8[6];
		size_t length = utf8_from_ucs4(کلید, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((ثابت uint8_t *)utf8, strlen(utf8),
				&dom_کلید);
		اگر (exc != DOM_NO_ERR) {
			بازگشت exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	اگر (exc != DOM_NO_ERR) {
		dom_string_unref(dom_کلید);
		بازگشت false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, پوچ,
			dom_کلید, پوچ, DOM_کلید_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_کلید);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		بازگشت false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(صحیح)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	بازگشت result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ایستا تهی html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_گره *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = پوچ;

	/* Clean up and report error if unsuccessful or aborted */
	اگر ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		اگر (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, پوچ);
		} وگرنه {
			content_broadcast_error(&c->base, NSERROR_STOPPED, پوچ);
		}

		content_set_error(&c->base);
		بازگشت;
	}


#اگر ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#اگر ALWAYS_DUMP_FRAMESET
	اگر (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (تهی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == پوچ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, پوچ);
		content_set_error(&c->base);
		بازگشت;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	اگر (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, پوچ);
		content_set_error(&c->base);
		dom_گره_unref(html);
		بازگشت;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = پوچ;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_گره_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	انتخاب (content__get_status(&html->base)) {
	حالت CONTENT_STATUS_READY:
		اگر (html->base.active == 0) {
			content_set_done(&html->base);
			بازگشت NSERROR_OK;
		}
		بشکن;
	حالت CONTENT_STATUS_DONE:
		/* fallthrough */
	حالت CONTENT_STATUS_LOADING:
		بازگشت NSERROR_OK;
	پیشفرض:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		بشکن;
	}
	بازگشت NSERROR_UNKNOWN;
}


ایستا تهی html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	اجتماع content_پیام_data پیام_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_پیام_GETDIMS, &پیام_data);


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
تهی html_finish_conversion(html_content *htmlc)
{
	اجتماع content_پیام_data پیام_data;
	dom_exception exc; /* returned by libdom functions */
	dom_گره *html;
	nserror error;

	/* Bail out if we've been aborted */
	اگر (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, پوچ);
		content_set_error(&htmlc->base);
		بازگشت;
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
	اگر (htmlc->select_ctx != پوچ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		بازگشت;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	اگر (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, پوچ);
		content_set_error(&htmlc->base);
		بازگشت;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	اگر (htmlc->jsthread != پوچ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, پوچ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	پیام_data.explicit_status_text = پوچ;
	content_broadcast(&htmlc->base, CONTENT_پیام_STATUS, &پیام_data);

	exc = dom_document_get_document_element(htmlc->document, (تهی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == پوچ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, پوچ);
		content_set_error(&htmlc->base);
		بازگشت;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	اگر (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_گره_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, پوچ);
		content_set_error(&htmlc->base);
		بازگشت;
	}

	dom_گره_unref(html);
}


ایستا تهی
html_document_user_data_handler(dom_گره_operation operation,
				dom_string *کلید, تهی *data,
				ساختار dom_گره *src,
				ساختار dom_گره *dst)
{
	اگر (dom_string_isequal(corestring_dom_ns_کلید_html_content_data,
			       کلید) == false || data == پوچ) {
		بازگشت;
	}

	انتخاب (operation) {
	حالت DOM_گره_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		بشکن;
	حالت DOM_گره_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		بشکن;
	حالت DOM_گره_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		بشکن;
	حالت DOM_گره_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		بشکن;
	حالت DOM_گره_DELETED:
		/* This is the only path I expect */
		بشکن;
	پیشفرض:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}


ایستا nserror
html_create_html_data(html_content *c, ثابت http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	تهی *old_گره_data;
	ثابت نویسه *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = پوچ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = پوچ;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = پوچ;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = پوچ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = پوچ;
	c->bctx = پوچ;
	c->layout = پوچ;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_شمار = 0;
	c->stylesheets = پوچ;
	c->select_ctx = پوچ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = پوچ;
	c->رقم_objects = 0;
	c->object_list = پوچ;
	c->forms = پوچ;
	c->imagemaps = پوچ;
	c->bw = پوچ;
	c->frameset = پوچ;
	c->iframe = پوچ;
	c->page = پوچ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_شمار = 0;
	c->scripts = پوچ;
	c->jsthread = پوچ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	اگر (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		بازگشت NSERROR_NOMEM;
	}

	اگر (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = پوچ;
		بازگشت NSERROR_NOMEM;
	}

	c->sel = selection_create((ساختار content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	اگر (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		اگر (c->encoding == پوچ) {
			lwc_string_unref(c->universal);
			c->universal = پوچ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = پوچ;
			بازگشت NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.پیام = پوچ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	اگر ((error != DOM_HUBBUB_OK) && (c->encoding != پوچ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		آزاد(c->encoding);
		c->encoding = پوچ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	اگر (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = پوچ;

		lwc_string_unref(c->universal);
		c->universal = پوچ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = پوچ;

		بازگشت libdom_hubbub_error_to_nserror(error);
	}

	err = dom_گره_set_user_data(c->document,
				     corestring_dom_ns_کلید_html_content_data,
				     c, html_document_user_data_handler,
				     (تهی *) &old_گره_data);
	اگر (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = پوچ;
		nsurl_unref(c->base_url);
		c->base_url = پوچ;

		lwc_string_unref(c->universal);
		c->universal = پوچ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = پوچ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		بازگشت NSERROR_DOM;
	}

	أكد(old_گره_data == پوچ);

	بازگشت NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ایستا nserror
html_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت http_parameter *params,
	    llcache_handle *llcache,
	    ثابت نویسه *fallback_charset,
	    bool quirks,
	    ساختار content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, اندازه(html_content));
	اگر (html == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(html);
		بازگشت error;
	}

	error = html_create_html_data(html, params);
	اگر (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, پوچ);
		آزاد(html);
		بازگشت error;
	}

	error = html_css_new_stylesheets(html);
	اگر (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, پوچ);
		آزاد(html);
		بازگشت error;
	}

	*c = (ساختار content *) html;

	بازگشت NSERROR_OK;
}



ایستا nserror
html_process_encoding_change(ساختار content *c,
			     ثابت نویسه *data,
			     unsigned صحیح size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ثابت نویسه *encoding;
	ثابت uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	اگر (encoding == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	اگر (html->encoding != پوچ) {
		آزاد(html->encoding);
		html->encoding = پوچ;
	}

	html->encoding = strdup(encoding);
	اگر (html->encoding == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = پوچ;

	اگر (html->document != پوچ) {
		dom_گره_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.پیام = پوچ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	اگر (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		آزاد(html->encoding);
		html->encoding = strdup("Windows-1252");
		اگر (html->encoding == پوچ) {
			بازگشت NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		اگر (error != DOM_HUBBUB_OK) {
			بازگشت libdom_hubbub_error_to_nserror(error);
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

	بازگشت libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ایستا bool
html_process_data(ساختار content *c, ثابت نویسه *data, unsigned صحیح size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ثابت uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	اگر (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	اگر (err != NSERROR_OK) {
		content_broadcast_error(c, err, پوچ);
		بازگشت false;
	}

	بازگشت true;
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

ایستا bool html_convert(ساختار content *c)
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
	اگر (exc == DOM_NO_ERR) {
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
	اگر (html_can_begin_conversion(htmlc)) {
		بازگشت html_begin_conversion(htmlc);
	}
	بازگشت true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned صحیح i;

	/* Cannot begin conversion if we're still fetching stuff */
	اگر (htmlc->base.active != 0)
		بازگشت false;

	برای (i = 0; i != htmlc->stylesheet_شمار; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		اگر (htmlc->stylesheets[i].modified)
			بازگشت false;
	}

	/* All is good, begin */
	بازگشت true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_گره *html;
	nserror ns_error;
	ساختار form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *گره_name = پوچ;
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
	اگر (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		اگر (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			بازگشت true;
		}
		اگر (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						پوچ);

			بازگشت false;
		}
		htmlc->parse_completed = true;
	}

	اگر (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		بازگشت true;
	}

	/* Give up processing if we've been aborted */
	اگر (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, پوچ);
		بازگشت false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	اگر (htmlc->encoding == پوچ) {
		ثابت نویسه *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		اگر (encoding == پوچ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						پوچ);
			بازگشت false;
		}

		htmlc->encoding = strdup(encoding);
		اگر (htmlc->encoding == پوچ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						پوچ);
			بازگشت false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (تهی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == پوچ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, پوچ);
		بازگشت false;
	}

	exc = dom_گره_get_گره_name(html, &گره_name);
	اگر ((exc != DOM_NO_ERR) ||
	    (گره_name == پوچ) ||
	    (!dom_string_caseless_lwc_isequal(گره_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, پوچ);
		dom_گره_unref(html);
		بازگشت false;
	}
	dom_string_unref(گره_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	برای (f = htmlc->forms; f != پوچ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		اگر (f->action == پوچ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} وگرنه {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		اگر (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, پوچ);

			dom_گره_unref(html);
			بازگشت false;
		}

		آزاد(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		اگر (f->action == پوچ) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						پوچ);

			dom_گره_unref(html);
			بازگشت false;
		}

		/* Ensure each form has a document encoding */
		اگر (f->document_charset == پوچ) {
			f->document_charset = strdup(htmlc->encoding);
			اگر (f->document_charset == پوچ) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							پوچ);
				dom_گره_unref(html);
				بازگشت false;
			}
		}
	}

	dom_گره_unref(html);

	اگر (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	بازگشت true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ایستا تهی html_stop(ساختار content *c)
{
	html_content *htmlc = (html_content *) c;

	انتخاب (c->status) {
	حالت CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		اگر (htmlc->jsthread != پوچ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		بشکن;

	حالت CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		اگر (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		بشکن;

	حالت CONTENT_STATUS_DONE:
		/* Nothing to do */
		بشکن;

	پیشفرض:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		أكد(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ایستا تهی html_reformat(ساختار content *c, صحیح width, صحیح height)
{
	html_content *htmlc = (html_content *) c;
	ساختار box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ریشه_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[چپ_2] + layout->width +
		layout->padding[راست_2] + layout->border[راست_2].width +
		layout->margin[راست_2];
	c->height = layout->y + layout->padding[بالاترین_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	اگر (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	اگر (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	اگر (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

تهی html_redraw_a_box(hlcache_handle *h, ساختار box *box)
{
	صحیح x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[چپ_2] + box->width + box->padding[راست_2],
			box->padding[بالاترین_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

تهی html__redraw_a_box(ساختار html_content *html, ساختار box *box)
{
	صحیح x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ساختار content *)html, x, y,
			box->padding[چپ_2] + box->width + box->padding[راست_2],
			box->padding[بالاترین_2] + box->height + box->padding[BOTTOM]);
}

ایستا تهی html_destroy_frameset(ساختار content_html_frames *frameset)
{
	صحیح i;

	اگر (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = پوچ;
	}
	اگر (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = پوچ;
	}
	اگر (frameset->children) {
		برای (i = 0; i < (frameset->rows * frameset->cols); i++) {
			اگر (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = پوچ;
			}
			اگر (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = پوچ;
			}
			اگر (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = پوچ;
	}
}

ایستا تهی html_destroy_iframe(ساختار content_html_iframe *iframe)
{
	ساختار content_html_iframe *next;
	next = iframe;
	وقتی ((iframe = next) != پوچ) {
		next = iframe->next;
		اگر (iframe->name)
			talloc_free(iframe->name);
		اگر (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = پوچ;
		}
		talloc_free(iframe);
	}
}


ایستا تهی html_free_layout(html_content *htmlc)
{
	اگر (htmlc->bctx != پوچ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ایستا تهی html_destroy(ساختار content *c)
{
	html_content *html = (html_content *) c;
	ساختار form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	اگر (html->box_conversion_context != پوچ) {
		اگر (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	برای (f = html->forms; f != پوچ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	اگر (c->refresh)
		nsurl_unref(c->refresh);

	اگر (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	اگر (html->jsthread != پوچ) {
		js_destroythread(html->jsthread);
		html->jsthread = پوچ;
	}

	اگر (html->parser != پوچ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = پوچ;
	}

	اگر (html->document != پوچ) {
		dom_گره_unref(html->document);
		html->document = پوچ;
	}

	اگر (html->title != پوچ) {
		dom_گره_unref(html->title);
		html->title = پوچ;
	}

	/* Free encoding */
	اگر (html->encoding != پوچ) {
		آزاد(html->encoding);
		html->encoding = پوچ;
	}

	/* Free base target */
	اگر (html->base_target != پوچ) {
		آزاد(html->base_target);
		html->base_target = پوچ;
	}

	/* Free frameset */
	اگر (html->frameset != پوچ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = پوچ;
	}

	/* Free iframes */
	اگر (html->iframe != پوچ) {
		html_destroy_iframe(html->iframe);
		html->iframe = پوچ;
	}

	/* Destroy selection context */
	اگر (html->select_ctx != پوچ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = پوچ;
	}

	lwc_string_unref(html->universal);
	html->universal = پوچ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = پوچ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ایستا nserror html_clone(ثابت ساختار content *old, ساختار content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	أكد(0 && "html_clone should never be called");

	بازگشت true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ایستا nserror
html_open(ساختار content *c,
	  ساختار browser_window *bw,
	  ساختار content *page,
	  ساختار object_params *params)
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

	بازگشت NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ایستا nserror html_close(ساختار content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = پوچ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	اگر (htmlc->jsthread != پوچ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	بازگشت ret;
}


/**
 * Return an HTML content's selection context
 */

ایستا تهی html_clear_selection(ساختار content *c)
{
	html_content *html = (html_content *) c;

	انتخاب (html->selection_type) {
	حالت HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		بشکن;
	حالت HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		بشکن;
	حالت HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		بشکن;
	حالت HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ایستا نویسه *html_get_selection(ساختار content *c)
{
	html_content *html = (html_content *) c;

	انتخاب (html->selection_type) {
	حالت HTML_SELECTION_TEXTAREA:
		بازگشت textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	حالت HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		بازگشت selection_get_copy(html->sel);
	حالت HTML_SELECTION_CONTENT:
		بازگشت content_get_selection(
				html->selection_owner.content->object);
	حالت HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		بشکن;
	پیشفرض:
		بشکن;
	}

	بازگشت پوچ;
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
ایستا nserror
html_get_contextual_content(ساختار content *c, صحیح x, صحیح y,
			    ساختار browser_window_features *data)
{
	html_content *html = (html_content *) c;

	ساختار box *box = html->layout;
	ساختار box *next;
	صحیح box_x = 0, box_y = 0;

	وقتی ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != پوچ) {
		box = next;

		/* hidden boxes are ignored */
		اگر ((box->style != پوچ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ادامه;
		}

		اگر (box->iframe) {
			اعشاری scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		اگر (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		اگر (box->object)
			data->object = box->object;

		اگر (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		اگر (box->usemap) {
			ثابت نویسه *target = پوچ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			اگر (url != پوچ)
				data->link = url;
		}
		اگر (box->gadget) {
			انتخاب (box->gadget->type) {
			حالت GADGET_TEXTBOX:
			حالت GADGET_TEXTAREA:
			حالت GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				بشکن;

			حالت GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				بشکن;

			پیشفرض:
				data->form_features = CTX_FORM_NONE;
				بشکن;
			}
		}
	}
	بازگشت NSERROR_OK;
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
ایستا bool
html_scroll_at_point(ساختار content *c, صحیح x, صحیح y, صحیح scrx, صحیح scry)
{
	html_content *html = (html_content *) c;

	ساختار box *box = html->layout;
	ساختار box *next;
	صحیح box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	وقتی ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != پوچ) {
		box = next;

		اگر (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ادامه;

		/* Pass into iframe */
		اگر (box->iframe) {
			اعشاری scale = browser_window_get_scale(box->iframe);

			اگر (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				بازگشت true;
		}

		/* Pass into textarea widget */
		اگر (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			بازگشت true;

		/* Pass into object */
		اگر (box->object != پوچ && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			بازگشت true;

		/* Handle box scrollbars */
		اگر (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		اگر (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		اگر (handled_scroll == true)
			بازگشت true;
	}

	بازگشت false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ایستا تهی html__dom_user_data_handler(dom_گره_operation operation,
		dom_string *کلید, تهی *_data, ساختار dom_گره *src,
		ساختار dom_گره *dst)
{
	نویسه *oldfile;
	نویسه *data = (نویسه *)_data;

	اگر (!dom_string_isequal(corestring_dom_ns_کلید_file_name_گره_data,
				کلید) || data == پوچ) {
		بازگشت;
	}

	انتخاب (operation) {
	حالت DOM_گره_CLONED:
		اگر (dom_گره_set_user_data(dst,
					   corestring_dom_ns_کلید_file_name_گره_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			اگر (oldfile != پوچ)
				آزاد(oldfile);
		}
		بشکن;

	حالت DOM_گره_RENAMED:
	حالت DOM_گره_IMPORTED:
	حالت DOM_گره_ADOPTED:
		بشکن;

	حالت DOM_گره_DELETED:
		آزاد(data);
		بشکن;
	پیشفرض:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}

ایستا تهی html__set_file_gadget_filename(ساختار content *c,
	ساختار form_control *gadget, ثابت نویسه *fn)
{
	nserror ret;
	نویسه *utf8_fn, *oldfile = پوچ;
	html_content *html = (html_content *)c;
	ساختار box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	اگر (ret != NSERROR_OK) {
		أكد(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		بازگشت;
	}

	form_gadget_update_مقدار(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	اگر (dom_گره_set_user_data((dom_گره *)file_box->gadget->گره,
				   corestring_dom_ns_کلید_file_name_گره_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		اگر (oldfile != پوچ)
			آزاد(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

تهی html_set_file_gadget_filename(ساختار hlcache_handle *hl,
	ساختار form_control *gadget, ثابت نویسه *fn)
{
	بازگشت html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
ایستا bool html_drop_file_at_point(ساختار content *c, صحیح x, صحیح y, نویسه *file)
{
	html_content *html = (html_content *) c;

	ساختار box *box = html->layout;
	ساختار box *next;
	ساختار box *file_box = پوچ;
	ساختار box *text_box = پوچ;
	صحیح box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	وقتی ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != پوچ) {
		box = next;

		اگر (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ادامه;

		اگر (box->iframe) {
			اعشاری scale = browser_window_get_scale(box->iframe);
			بازگشت browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		اگر (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			بازگشت true;

		اگر (box->gadget) {
			انتخاب (box->gadget->type) {
				حالت GADGET_FILE:
					file_box = box;
				بشکن;

				حالت GADGET_TEXTBOX:
				حالت GADGET_TEXTAREA:
				حالت GADGET_PASSWORD:
					text_box = box;
					بشکن;

				پیشفرض:	/* appease compiler */
					بشکن;
			}
		}
	}

	اگر (!file_box && !text_box)
		/* No box capable of handling drop */
		بازگشت false;

	اگر (file == پوچ)
		/* There is a box capable of handling drop here */
		بازگشت true;

	/* Handle the drop */
	اگر (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} وگرنه {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = پوچ;
		نویسه *buffer;
		نویسه *utf8_buff;
		nserror ret;
		unsigned صحیح size;
		صحیح bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		اگر (fp == پوچ) {
			/* Couldn't open file, but drop was for us */
			بازگشت true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		اگر ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			بازگشت true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		اگر (buffer == پوچ) {
			/* No memory, but drop was for us */
			fclose(fp);
			بازگشت true;
		}

		/* Stick file into buffer */
		اگر (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			آزاد(buffer);
			fclose(fp);
			بازگشت true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		اگر (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			أكد(ret != NSERROR_BAD_ENCODING);
			آزاد(buffer);
			بازگشت true;
		}

		/* Done with buffer */
		آزاد(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		آزاد(utf8_buff);
	}

	بازگشت true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ایستا nserror
html_debug(ساختار content *c, شمارش content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	بازگشت NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ایستا nserror
html_debug_dump(ساختار content *c, FILE *f, شمارش content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_گره *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	أكد(htmlc != پوچ);

	اگر (op == CONTENT_DEBUG_RENDER) {
		أكد(htmlc->layout != پوچ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} وگرنه {
		اگر (htmlc->document == پوچ) {
			NSLOG(netsurf, INFO, "No document to dump");
			بازگشت NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (تهی *) &html);
		اگر ((exc != DOM_NO_ERR) || (html == پوچ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			بازگشت NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_گره_unref(html);
	}

	بازگشت ret;
}


#اگر ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ایستا تهی
html_dump_frameset(ساختار content_html_frames *frame, unsigned صحیح depth)
{
	unsigned صحیح i;
	صحیح row, col, index;
	ثابت نویسه *unit[] = {"px", "%", "*"};
	ثابت نویسه *scrolling[] = {"auto", "yes", "no"};

	أكد(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.مقدار, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.مقدار,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	اگر (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	اگر (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	اگر (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	اگر (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned صحیح) frame->border_colour);

	fprintf(stderr, "\n");

	اگر (frame->children) {
		برای (row = 0; row != frame->rows; row++) {
			برای (col = 0; col != frame->cols; col++) {
				برای (i = 0; i != depth; i++)
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

	أكد(c != پوچ);

	بازگشت c->document;
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
ساختار box *html_get_box_درخت(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);

	بازگشت c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ایستا ثابت نویسه *html_encoding(ثابت ساختار content *c, شمارش content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ایستا نویسه enc_token[10] = "Encoding0";

	أكد(html != پوچ);

	اگر (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		بازگشت messages_get(enc_token);
	}

	بازگشت html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ساختار content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);

	بازگشت c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ساختار content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);

	بازگشت c->iframe;
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

	أكد(c != پوچ);

	بازگشت c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ثابت نویسه *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);

	بازگشت c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, صحیح *x, صحیح *y)
{
	ساختار box *pos;
	ساختار box *layout;

	اگر (content_get_type(h) != CONTENT_HTML)
		بازگشت false;

	layout = html_get_box_درخت(h);

	اگر ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		بازگشت true;
	}
	بازگشت false;
}

bool html_exec(ساختار content *c, ثابت نویسه *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_گره;
	dom_string *dom_src;
	dom_text *text_گره;
	dom_گره *spare_گره;
	dom_html_script_element *script_گره;

	اگر (htmlc->document == پوچ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		برو out_no_string;
	}

	err = dom_string_create((ثابت uint8_t *)src, srclen, &dom_src);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		برو out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_گره);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		برو out_no_body;
	}

	err = dom_document_create_text_گره(htmlc->document, dom_src, &text_گره);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		برو out_no_text_گره;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_گره);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		برو out_no_script_گره;
	}

	err = dom_گره_append_child(script_گره, text_گره, &spare_گره);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		برو out_unparented;
	}
	dom_گره_unref(spare_گره); /* We do not need the spare ref at all */

	err = dom_گره_append_child(body_گره, script_گره, &spare_گره);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		برو out_unparented;
	}
	dom_گره_unref(spare_گره); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_گره_get_parent_گره(script_گره, &spare_گره);
	اگر (err == DOM_NO_ERR && spare_گره != پوچ) {
		dom_گره *second_spare;
		err = dom_گره_remove_child(spare_گره, script_گره, &second_spare);
		اگر (err == DOM_NO_ERR) {
			dom_گره_unref(second_spare);
		}
		dom_گره_unref(spare_گره);
	}

out_unparented:
	dom_گره_unref(script_گره);
out_no_script_گره:
	dom_گره_unref(text_گره);
out_no_text_گره:
	dom_گره_unref(body_گره);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	بازگشت result;
}

/* See \ref content_saw_insecure_objects */
ایستا bool
html_saw_insecure_objects(ساختار content *c)
{
	html_content *htmlc = (html_content *)c;
	ساختار content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	وقتی (obj != پوچ) {
		اگر (obj->content != پوچ) {
			اگر (content_saw_insecure_objects(obj->content))
				بازگشت true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	اگر (html_saw_insecure_scripts(htmlc)) {
		بازگشت true;
	}

	/* Now check stylesheets */
	اگر (html_css_saw_insecure_stylesheets(htmlc)) {
		بازگشت true;
	}

	بازگشت false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ایستا content_type html_content_type(تهی)
{
	بازگشت CONTENT_HTML;
}


ایستا تهی html_fini(تهی)
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
ایستا nserror
find_occurrences_html_box(ثابت نویسه *pattern,
			  صحیح p_len,
			  ساختار box *cur,
			  bool case_sens,
			  ساختار textsearch_context *context)
{
	ساختار box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	اگر (!cur->object && cur->text) {
		ثابت نویسه *text = cur->text;
		unsigned length = cur->length;

		وقتی (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			ثابت نویسه *new_text;
			ثابت نویسه *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			اگر (!pos)
				بشکن;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	برای (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		اگر (res != NSERROR_OK) {
			بازگشت res;
		}
	}

	بازگشت res;
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
ایستا nserror
html_textsearch_find(ساختار content *c,
		     ساختار textsearch_context *context,
		     ثابت نویسه *pattern,
		     صحیح p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	اگر (html->layout == پوچ) {
		بازگشت NSERROR_INVALID;
	}

	بازگشت find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ایستا nserror
html_textsearch_bounds(ساختار content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       ساختار box *start_box,
		       ساختار box *end_box,
		       ساختار rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	بازگشت NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ایستا ثابت content_handler html_content_handler = {
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
nserror html_init(تهی)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	اگر (error != NSERROR_OK)
		برو error;

	برای (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		اگر (error != NSERROR_OK)
			برو error;
	}

	بازگشت NSERROR_OK;

error:
	html_fini();

	بازگشت error;
}
