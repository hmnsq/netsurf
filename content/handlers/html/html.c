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

#ضمن <assert.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <stdlib.h>
#ضمن <nsutils/time.h>

#ضمن "utils/utils.h"
#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/http.h"
#ضمن "utils/libdom.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/talloc.h"
#ضمن "utils/utf8.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/string.h"
#ضمن "utils/ascii.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/utf8.h"
#ضمن "netsurf/keypress.h"
#ضمن "netsurf/layout.h"
#ضمن "netsurf/misc.h"
#ضمن "content/hlcache.h"
#ضمن "content/content_factory.h"
#ضمن "content/textsearch.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"
#ضمن "netsurf/bitmap.h"
#ضمن "javascript/js.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/dom_event.h"
#ضمن "html/css.h"
#ضمن "html/object.h"
#ضمن "html/html_save.h"
#ضمن "html/interaction.h"
#ضمن "html/box.h"
#ضمن "html/box_construct.h"
#ضمن "html/box_inspect.h"
#ضمن "html/form_internal.h"
#ضمن "html/imagemap.h"
#ضمن "html/layout.h"
#ضمن "html/textselection.h"

#تعریف_کن CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#تعریف_کن ALWAYS_DUMP_FRAMESET 0
#تعریف_کن ALWAYS_DUMP_BOX 0

ساكن ثابت حرف *html_types[] = {
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
ساكن bool fire_dom_event(dom_event *event, dom_گره *هدف)
{
	dom_exception exc;
	bool نتیجه;

	exc = dom_event_هدف_dispatch_event(هدف, event, &نتیجه);
	إذا (exc != DOM_NO_ERR) {
		أرجع false;
	}

	أرجع نتیجه;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_گره *هدف,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool نتیجه;

	exc = dom_event_create(&evt);
	إذا (exc != DOM_NO_ERR) أرجع false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	إذا (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		أرجع false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (صحيح)dom_string_درازی(type), dom_string_داده(type), هدف);
	نتیجه = fire_dom_event(evt, هدف);
	dom_event_unref(evt);
	أرجع نتیجه;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_گره *هدف,
		bool bubbles, bool cancelable, uint32_t کلید)
{
	bool is_special = کلید <= 0x001F || (0x007F <= کلید && کلید <= 0x009F);
	dom_string *dom_کلید = عدم;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool نتیجه;

	إذا (is_special) {
		بدل (کلید) {
		حالة NS_کلید_ESCAPE:
			dom_کلید = dom_string_ref(corestring_dom_Escape);
			اكسر;
		حالة NS_کلید_چپ:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_چپ);
			اكسر;
		حالة NS_کلید_راست:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_راست);
			اكسر;
		حالة NS_کلید_به_بالا:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_به_بالا);
			اكسر;
		حالة NS_کلید_پایین:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_پایین);
			اكسر;
		حالة NS_کلید_صفحه_به_بالا:
			dom_کلید = dom_string_ref(corestring_dom_صفحه_به_بالا);
			اكسر;
		حالة NS_کلید_صفحه_پایین:
			dom_کلید = dom_string_ref(corestring_dom_صفحه_پایین);
			اكسر;
		حالة NS_کلید_متن_شروع:
			dom_کلید = dom_string_ref(corestring_dom_Home);
			اكسر;
		حالة NS_کلید_متن_انتها:
			dom_کلید = dom_string_ref(corestring_dom_انتها);
			اكسر;
		افتراضي:
			dom_کلید = عدم;
			اكسر;
		}
	} وإلا {
		حرف utf8[6];
		size_t درازی = utf8_from_ucs4(کلید, utf8);
		utf8[درازی] = '\0';

		exc = dom_string_create((ثابت uint8_t *)utf8, طول_رشته(utf8),
				&dom_کلید);
		إذا (exc != DOM_NO_ERR) {
			أرجع exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	إذا (exc != DOM_NO_ERR) {
		dom_string_unref(dom_کلید);
		أرجع false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, عدم,
			dom_کلید, عدم, DOM_کلید_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_کلید);
	إذا (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		أرجع false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(صحيح)dom_string_درازی(type), dom_string_داده(type), هدف);

	نتیجه = fire_dom_event((dom_event *) evt, هدف);
	dom_event_unref(evt);
	أرجع نتیجه;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ساكن فراغ html_box_convert_انجامشده(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_گره *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = عدم;

	/* Clean up and report error if unsuccessful or aborted */
	إذا ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		إذا (success == false) {
			content_پخش_error(&c->پایه, NSERROR_BOX_CONVERT, عدم);
		} وإلا {
			content_پخش_error(&c->پایه, NSERROR_STOPPED, عدم);
		}

		content_مجموعه_error(&c->پایه);
		أرجع;
	}


#إذا ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#پایان_اگر
#إذا ALWAYS_DUMP_FRAMESET
	إذا (c->frameset)
		html_dump_frameset(c->frameset, 0);
#پایان_اگر

	exc = dom_document_get_document_element(c->document, (فراغ *) &html);
	إذا ((exc != DOM_NO_ERR) || (html == عدم)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_پخش_error(&c->پایه, NSERROR_DOM, عدم);
		content_مجموعه_error(&c->پایه);
		أرجع;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_استخراج(c);
	إذا (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_پخش_error(&c->پایه, err, عدم);
		content_مجموعه_error(&c->پایه);
		dom_گره_unref(html);
		أرجع;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = عدم;

	content_مجموعه_ready(&c->پایه);

	html_proceed_مقصد_انجامشده(c);

	dom_گره_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_مقصد_انجامشده(html_content *html)
{
	بدل (content_get_وضعیت_2(&html->پایه)) {
	حالة CONTENT_وضعیت_READY:
		إذا (html->پایه.فعال == 0) {
			content_مجموعه_انجامشده(&html->پایه);
			أرجع NSERROR_OK;
		}
		اكسر;
	حالة CONTENT_وضعیت_انجامشده:
		/* fallthrough */
	حالة CONTENT_وضعیت_LOADING:
		أرجع NSERROR_OK;
	افتراضي:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		اكسر;
	}
	أرجع NSERROR_UNKNOWN;
}


ساكن فراغ html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	بیعلامت f_اندازه;
	بیعلامت f_min;
	بیعلامت w;
	بیعلامت h;
	اتحاد content_پیام_داده پیام_داده = {
		.getdims = {
			.viewport_عرض = &w,
			.viewport_ارتفاع = &h,
		},
	};

	content_پخش(&htmlc->پایه, CONTENT_پیام_GETDIMS, &پیام_داده);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.عرض  = w;
	htmlc->media.ارتفاع = h;
	htmlc->unit_len_ctx.viewport_عرض  = w;
	htmlc->unit_len_ctx.viewport_ارتفاع = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_اندازه = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_اندازه)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_اندازه)), F_10)), F_72);

	htmlc->unit_len_ctx.font_اندازه_default = f_اندازه;
	htmlc->unit_len_ctx.font_اندازه_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
فراغ html_پایان_conversion(html_content *htmlc)
{
	اتحاد content_پیام_داده پیام_داده;
	dom_exception exc; /* returned by libdom functions */
	dom_گره *html;
	nserror error;

	/* Bail out if we've been aborted */
	إذا (htmlc->aborted) {
		content_پخش_error(&htmlc->پایه, NSERROR_STOPPED, عدم);
		content_مجموعه_error(&htmlc->پایه);
		أرجع;
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
	إذا (htmlc->select_ctx != عدم) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		أرجع;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	إذا (error != NSERROR_OK) {
		content_پخش_error(&htmlc->پایه, error, عدم);
		content_مجموعه_error(&htmlc->پایه);
		أرجع;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	إذا (htmlc->jsthread != عدم) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, عدم);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_مجموعه_وضعیت(&htmlc->پایه, messages_get("Processing"));
	پیام_داده.explicit_وضعیت_متن = عدم;
	content_پخش(&htmlc->پایه, CONTENT_پیام_وضعیت, &پیام_داده);

	exc = dom_document_get_document_element(htmlc->document, (فراغ *) &html);
	إذا ((exc != DOM_NO_ERR) || (html == عدم)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_پخش_error(&htmlc->پایه, NSERROR_DOM, عدم);
		content_مجموعه_error(&htmlc->پایه);
		أرجع;
	}

	html_get_dimensions(htmlc);

	error = dom_مقصد_box(html, htmlc, html_box_convert_انجامشده, &htmlc->box_conversion_context);
	إذا (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_گره_unref(html);
		html_object_free_objects(htmlc);
		content_پخش_error(&htmlc->پایه, error, عدم);
		content_مجموعه_error(&htmlc->پایه);
		أرجع;
	}

	dom_گره_unref(html);
}


ساكن فراغ
html_document_user_داده_handler(dom_گره_operation operation,
				dom_string *کلید, فراغ *داده,
				بنية dom_گره *src,
				بنية dom_گره *dst)
{
	إذا (dom_string_isequal(corestring_dom_ns_کلید_html_content_داده,
			       کلید) == false || داده == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_گره_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		اكسر;
	حالة DOM_گره_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		اكسر;
	حالة DOM_گره_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		اكسر;
	حالة DOM_گره_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		اكسر;
	حالة DOM_گره_DELETED:
		/* This is the only path I expect */
		اكسر;
	افتراضي:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}


ساكن nserror
html_create_html_داده(html_content *c, ثابت http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	فراغ *old_گره_داده;
	ثابت حرف *prefer_color_نما = (nsoption_bool(prefer_dark_نما)) ?
			"dark" : "light";

	c->parser = عدم;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = عدم;
	c->quirks = DOM_DOCUMENT_QUIRKS_نما_NONE;
	c->encoding = عدم;
	c->پایه_url = nsurl_ref(content_get_url(&c->پایه));
	c->پایه_هدف = عدم;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = عدم;
	c->bctx = عدم;
	c->layout = عدم;
	c->background_رنگ = NS_TRANSPARENT;
	c->stylesheet_شمار = 0;
	c->stylesheets = عدم;
	c->select_ctx = عدم;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = عدم;
	c->رقم_objects = 0;
	c->object_list = عدم;
	c->forms = عدم;
	c->imagemaps = عدم;
	c->bw = عدم;
	c->frameset = عدم;
	c->iframe = عدم;
	c->صفحه = عدم;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_شمار = 0;
	c->scripts = عدم;
	c->jsthread = عدم;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->پایه.فعال = 1; /* The html content itself is active */

	إذا (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		أرجع NSERROR_NOMEM;
	}

	إذا (lwc_intern_string(prefer_color_نما, طول_رشته(prefer_color_نما),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = عدم;
		أرجع NSERROR_NOMEM;
	}

	c->sel = selection_create((بنية content *)c);

	nerror = http_parameter_list_پیدا_کن_قلم(params, corestring_lwc_charset, &charset);
	إذا (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_داده(charset));

		lwc_string_unref(charset);

		إذا (c->encoding == عدم) {
			lwc_string_unref(c->universal);
			c->universal = عدم;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = عدم;
			أرجع NSERROR_NOMEM;

		}
		c->encoding_منبع = DOM_HUBBUB_ENCODING_منبع_سرآیند;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.پیام = عدم;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	إذا ((error != DOM_HUBBUB_OK) && (c->encoding != عدم)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		حرر(c->encoding);
		c->encoding = عدم;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	إذا (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->پایه_url);
		c->پایه_url = عدم;

		lwc_string_unref(c->universal);
		c->universal = عدم;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = عدم;

		أرجع libdom_hubbub_error_مقصد_nserror(error);
	}

	err = dom_گره_مجموعه_user_داده(c->document,
				     corestring_dom_ns_کلید_html_content_داده,
				     c, html_document_user_داده_handler,
				     (فراغ *) &old_گره_داده);
	إذا (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = عدم;
		nsurl_unref(c->پایه_url);
		c->پایه_url = عدم;

		lwc_string_unref(c->universal);
		c->universal = عدم;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = عدم;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		أرجع NSERROR_DOM;
	}

	أكد(old_گره_داده == عدم);

	أرجع NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ساكن nserror
html_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت http_parameter *params,
	    llcache_handle *llcache,
	    ثابت حرف *fallback_charset,
	    bool quirks,
	    بنية content **c)
{
	html_content *html;
	nserror error;

	html = حافظه_صفر_بده(1, حجم(html_content));
	إذا (html == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&html->پایه, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(html);
		أرجع error;
	}

	error = html_create_html_داده(html, params);
	إذا (error != NSERROR_OK) {
		content_پخش_error(&html->پایه, error, عدم);
		حرر(html);
		أرجع error;
	}

	error = html_css_new_stylesheets(html);
	إذا (error != NSERROR_OK) {
		content_پخش_error(&html->پایه, error, عدم);
		حرر(html);
		أرجع error;
	}

	*c = (بنية content *) html;

	أرجع NSERROR_OK;
}



ساكن nserror
html_process_encoding_change(بنية content *c,
			     ثابت حرف *داده,
			     بیعلامت صحيح اندازه)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ثابت حرف *encoding;
	ثابت uint8_t *منبع_داده;
	size_t منبع_اندازه;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_منبع);
	إذا (encoding == عدم) {
		أرجع NSERROR_NOMEM;
	}

	إذا (html->encoding != عدم) {
		حرر(html->encoding);
		html->encoding = عدم;
	}

	html->encoding = strdup(encoding);
	إذا (html->encoding == عدم) {
		أرجع NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = عدم;

	إذا (html->document != عدم) {
		dom_گره_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.پیام = عدم;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	إذا (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		حرر(html->encoding);
		html->encoding = strdup("Windows-1252");
		إذا (html->encoding == عدم) {
			أرجع NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		إذا (error != DOM_HUBBUB_OK) {
			أرجع libdom_hubbub_error_مقصد_nserror(error);
		}

	}

	منبع_داده = content_get_منبع_داده_2(c, &منبع_اندازه);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      منبع_داده,
					      منبع_اندازه);

	أرجع libdom_hubbub_error_مقصد_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ساكن bool
html_process_داده(بنية content *c, ثابت حرف *داده, بیعلامت صحيح اندازه)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ثابت uint8_t *) داده,
					      اندازه);

	err = libdom_hubbub_error_مقصد_nserror(dom_ret);

	/* deal with encoding change */
	إذا (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, داده, اندازه);
	}

	/* broadcast the error if necessary */
	إذا (err != NSERROR_OK) {
		content_پخش_error(c, err, عدم);
		أرجع false;
	}

	أرجع true;
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

ساكن bool html_convert(بنية content *c)
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
	exc = dom_document_get_quirks_نما(htmlc->document, &htmlc->quirks);
	إذا (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->پایه.فعال--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->پایه.فعال, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	إذا (html_can_begin_conversion(htmlc)) {
		أرجع html_begin_conversion(htmlc);
	}
	أرجع true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	بیعلامت صحيح i;

	/* Cannot begin conversion if we're still fetching stuff */
	إذا (htmlc->پایه.فعال != 0)
		أرجع false;

	لكل (i = 0; i != htmlc->stylesheet_شمار; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		إذا (htmlc->stylesheets[i].modified)
			أرجع false;
	}

	/* All is good, begin */
	أرجع true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_گره *html;
	nserror ns_error;
	بنية form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *گره_name = عدم;
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
	إذا (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		إذا (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->پایه.فعال > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			أرجع true;
		}
		إذا (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_پخش_error(&htmlc->پایه,
						libdom_hubbub_error_مقصد_nserror(error),
						عدم);

			أرجع false;
		}
		htmlc->parse_completed = true;
	}

	إذا (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		أرجع true;
	}

	/* Give up processing if we've been aborted */
	إذا (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->پایه.فعال);
		content_مجموعه_error(&htmlc->پایه);
		content_پخش_error(&htmlc->پایه, NSERROR_STOPPED, عدم);
		أرجع false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	إذا (htmlc->encoding == عدم) {
		ثابت حرف *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_منبع);
		إذا (encoding == عدم) {
			content_پخش_error(&htmlc->پایه,
						NSERROR_NOMEM,
						عدم);
			أرجع false;
		}

		htmlc->encoding = strdup(encoding);
		إذا (htmlc->encoding == عدم) {
			content_پخش_error(&htmlc->پایه,
						NSERROR_NOMEM,
						عدم);
			أرجع false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (فراغ *) &html);
	إذا ((exc != DOM_NO_ERR) || (html == عدم)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_پخش_error(&htmlc->پایه, NSERROR_DOM, عدم);
		أرجع false;
	}

	exc = dom_گره_get_گره_name(html, &گره_name);
	إذا ((exc != DOM_NO_ERR) ||
	    (گره_name == عدم) ||
	    (!dom_string_caseless_lwc_isequal(گره_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_پخش_error(&htmlc->پایه, NSERROR_DOM, عدم);
		dom_گره_unref(html);
		أرجع false;
	}
	dom_string_unref(گره_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	لكل (f = htmlc->forms; f != عدم; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		إذا (f->action == عدم || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->پایه);
			ns_error = nsurl_join(htmlc->پایه_url,
					      nsurl_access(doc_addr),
					      &action);
		} وإلا {
			ns_error = nsurl_join(htmlc->پایه_url,
					      f->action,
					      &action);
		}

		إذا (ns_error != NSERROR_OK) {
			content_پخش_error(&htmlc->پایه, ns_error, عدم);

			dom_گره_unref(html);
			أرجع false;
		}

		حرر(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		إذا (f->action == عدم) {
			content_پخش_error(&htmlc->پایه,
						NSERROR_NOMEM,
						عدم);

			dom_گره_unref(html);
			أرجع false;
		}

		/* Ensure each form has a document encoding */
		إذا (f->document_charset == عدم) {
			f->document_charset = strdup(htmlc->encoding);
			إذا (f->document_charset == عدم) {
				content_پخش_error(&htmlc->پایه,
							NSERROR_NOMEM,
							عدم);
				dom_گره_unref(html);
				أرجع false;
			}
		}
	}

	dom_گره_unref(html);

	إذا (htmlc->پایه.فعال == 0) {
		html_پایان_conversion(htmlc);
	}

	أرجع true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ساكن فراغ html_stop(بنية content *c)
{
	html_content *htmlc = (html_content *) c;

	بدل (c->وضعیت) {
	حالة CONTENT_وضعیت_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		إذا (htmlc->jsthread != عدم) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		اكسر;

	حالة CONTENT_وضعیت_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		إذا (c->وضعیت == CONTENT_وضعیت_READY && c->فعال == 0) {
			content_مجموعه_انجامشده(c);
		}

		اكسر;

	حالة CONTENT_وضعیت_انجامشده:
		/* Nothing to do */
		اكسر;

	افتراضي:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->وضعیت,
		      c);
		أكد(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ساكن فراغ html_reformat(بنية content *c, صحيح عرض, صحيح ارتفاع)
{
	html_content *htmlc = (html_content *) c;
	بنية box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_عرض = css_unit_device2css_px(
			INTTOFIX(عرض), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_ارتفاع = css_unit_device2css_px(
			INTTOFIX(ارتفاع), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ریشه_style = htmlc->layout->style;

	layout_document(htmlc, عرض, ارتفاع);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->عرض = layout->x + layout->padding[چپ_2] + layout->عرض +
		layout->padding[راست_2] + layout->border[راست_2].عرض +
		layout->margin[راست_2];
	c->ارتفاع = layout->y + layout->padding[بالاترین_2] + layout->ارتفاع +
		layout->padding[BOTTOM] + layout->border[BOTTOM].عرض +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	إذا (c->عرض < layout->x + layout->descendant_x1)
		c->عرض = layout->x + layout->descendant_x1;
	إذا (c->ارتفاع < layout->y + layout->descendant_y1)
		c->ارتفاع = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	إذا (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_زمان = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

فراغ html_redraw_a_box(hlcache_handle *h, بنية box *box)
{
	صحيح x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[چپ_2] + box->عرض + box->padding[راست_2],
			box->padding[بالاترین_2] + box->ارتفاع + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

فراغ html__redraw_a_box(بنية html_content *html, بنية box *box)
{
	صحيح x, y;

	box_coords(box, &x, &y);

	content__request_redraw((بنية content *)html, x, y,
			box->padding[چپ_2] + box->عرض + box->padding[راست_2],
			box->padding[بالاترین_2] + box->ارتفاع + box->padding[BOTTOM]);
}

ساكن فراغ html_destroy_frameset(بنية content_html_قابها *frameset)
{
	صحيح i;

	إذا (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = عدم;
	}
	إذا (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = عدم;
	}
	إذا (frameset->children) {
		لكل (i = 0; i < (frameset->ردیفها * frameset->تعداد_ستون); i++) {
			إذا (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = عدم;
			}
			إذا (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = عدم;
			}
			إذا (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = عدم;
	}
}

ساكن فراغ html_destroy_iframe(بنية content_html_iframe *iframe)
{
	بنية content_html_iframe *بعدی;
	بعدی = iframe;
	طالما ((iframe = بعدی) != عدم) {
		بعدی = iframe->بعدی;
		إذا (iframe->name)
			talloc_free(iframe->name);
		إذا (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = عدم;
		}
		talloc_free(iframe);
	}
}


ساكن فراغ html_free_layout(html_content *htmlc)
{
	إذا (htmlc->bctx != عدم) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ساكن فراغ html_destroy(بنية content *c)
{
	html_content *html = (html_content *) c;
	بنية form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	إذا (html->box_conversion_context != عدم) {
		إذا (cancel_dom_مقصد_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	لكل (f = html->forms; f != عدم; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	إذا (c->refresh)
		nsurl_unref(c->refresh);

	إذا (html->پایه_url)
		nsurl_unref(html->پایه_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	إذا (html->jsthread != عدم) {
		js_destroythread(html->jsthread);
		html->jsthread = عدم;
	}

	إذا (html->parser != عدم) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = عدم;
	}

	إذا (html->document != عدم) {
		dom_گره_unref(html->document);
		html->document = عدم;
	}

	إذا (html->title != عدم) {
		dom_گره_unref(html->title);
		html->title = عدم;
	}

	/* Free encoding */
	إذا (html->encoding != عدم) {
		حرر(html->encoding);
		html->encoding = عدم;
	}

	/* Free base target */
	إذا (html->پایه_هدف != عدم) {
		حرر(html->پایه_هدف);
		html->پایه_هدف = عدم;
	}

	/* Free frameset */
	إذا (html->frameset != عدم) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = عدم;
	}

	/* Free iframes */
	إذا (html->iframe != عدم) {
		html_destroy_iframe(html->iframe);
		html->iframe = عدم;
	}

	/* Destroy selection context */
	إذا (html->select_ctx != عدم) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = عدم;
	}

	lwc_string_unref(html->universal);
	html->universal = عدم;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = عدم;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ساكن nserror html_clone(ثابت بنية content *old, بنية content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	أكد(0 && "html_clone should never be called");

	أرجع true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ساكن nserror
html_open(بنية content *c,
	  بنية browser_پنجره *bw,
	  بنية content *صفحه,
	  بنية object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->صفحه = (html_content *) صفحه;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	أرجع NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ساكن nserror html_close(بنية content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = عدم;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	إذا (htmlc->jsthread != عدم) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	أرجع ret;
}


/**
 * Return an HTML content's selection context
 */

ساكن فراغ html_clear_selection(بنية content *c)
{
	html_content *html = (html_content *) c;

	بدل (html->selection_type) {
	حالة HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		اكسر;
	حالة HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->داده.متن.ta);
		اكسر;
	حالة HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		اكسر;
	حالة HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		اكسر;
	افتراضي:
		اكسر;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ساكن حرف *html_get_selection(بنية content *c)
{
	html_content *html = (html_content *) c;

	بدل (html->selection_type) {
	حالة HTML_SELECTION_TEXTAREA:
		أرجع textarea_get_selection(html->selection_owner.textarea->
				gadget->داده.متن.ta);
	حالة HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		أرجع selection_get_copy(html->sel);
	حالة HTML_SELECTION_CONTENT:
		أرجع content_get_selection(
				html->selection_owner.content->object);
	حالة HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		اكسر;
	افتراضي:
		اكسر;
	}

	أرجع عدم;
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
ساكن nserror
html_get_contextual_content(بنية content *c, صحيح x, صحيح y,
			    بنية browser_پنجره_features *داده)
{
	html_content *html = (html_content *) c;

	بنية box *box = html->layout;
	بنية box *بعدی;
	صحيح box_x = 0, box_y = 0;

	طالما ((بعدی = box_at_نقطه(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != عدم) {
		box = بعدی;

		/* hidden boxes are ignored */
		إذا ((box->style != عدم) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			استمر;
		}

		إذا (box->iframe) {
			عائم scale = browser_پنجره_get_scale(box->iframe);
			browser_پنجره_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    داده);
		}

		إذا (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, داده);

		إذا (box->object)
			داده->object = box->object;

		إذا (box->href) {
			داده->link = box->href;
			داده->link_title = box->متن;
			داده->link_title_درازی = box->درازی;
		}

		إذا (box->usemap) {
			ثابت حرف *هدف = عدم;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &هدف);
			/* Box might have imagemap, but no actual link area
			 * at point */
			إذا (url != عدم)
				داده->link = url;
		}
		إذا (box->gadget) {
			بدل (box->gadget->type) {
			حالة GADGET_TEXTBOX:
			حالة GADGET_TEXTAREA:
			حالة GADGET_PASSWORD:
				داده->form_features = CTX_FORM_متن;
				اكسر;

			حالة GADGET_FILE:
				داده->form_features = CTX_FORM_FILE;
				اكسر;

			افتراضي:
				داده->form_features = CTX_FORM_NONE;
				اكسر;
			}
		}
	}
	أرجع NSERROR_OK;
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
ساكن bool
html_scroll_at_نقطه(بنية content *c, صحيح x, صحيح y, صحيح scrx, صحيح scry)
{
	html_content *html = (html_content *) c;

	بنية box *box = html->layout;
	بنية box *بعدی;
	صحيح box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	طالما ((بعدی = box_at_نقطه(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != عدم) {
		box = بعدی;

		إذا (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			استمر;

		/* Pass into iframe */
		إذا (box->iframe) {
			عائم scale = browser_پنجره_get_scale(box->iframe);

			إذا (browser_پنجره_scroll_at_نقطه(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				أرجع true;
		}

		/* Pass into textarea widget */
		إذا (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->داده.متن.ta,
						scrx, scry) == true)
			أرجع true;

		/* Pass into object */
		إذا (box->object != عدم && content_scroll_at_نقطه(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			أرجع true;

		/* Handle box scrollbars */
		إذا (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		إذا (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		إذا (handled_scroll == true)
			أرجع true;
	}

	أرجع false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ساكن فراغ html_dom_user_داده_handler(dom_گره_operation operation,
		dom_string *کلید, فراغ *داده_2, بنية dom_گره *src,
		بنية dom_گره *dst)
{
	حرف *oldfile;
	حرف *داده = (حرف *)داده_2;

	إذا (!dom_string_isequal(corestring_dom_ns_کلید_file_name_گره_داده,
				کلید) || داده == عدم) {
		أرجع;
	}

	بدل (operation) {
	حالة DOM_گره_CLONED:
		إذا (dom_گره_مجموعه_user_داده(dst,
					   corestring_dom_ns_کلید_file_name_گره_داده,
					   strdup(داده), html_dom_user_داده_handler,
					   &oldfile) == DOM_NO_ERR) {
			إذا (oldfile != عدم)
				حرر(oldfile);
		}
		اكسر;

	حالة DOM_گره_RENAMED:
	حالة DOM_گره_IMPORTED:
	حالة DOM_گره_ADOPTED:
		اكسر;

	حالة DOM_گره_DELETED:
		حرر(داده);
		اكسر;
	افتراضي:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}

ساكن فراغ html_مجموعه_file_gadget_filename(بنية content *c,
	بنية form_control *gadget, ثابت حرف *fn)
{
	nserror ret;
	حرف *utf8_fn, *oldfile = عدم;
	html_content *html = (html_content *)c;
	بنية box *file_box = gadget->box;

	ret = guit->utf8->local_مقصد_utf8(fn, 0, &utf8_fn);
	إذا (ret != NSERROR_OK) {
		أكد(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		أرجع;
	}

	form_gadget_update_مقدار(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	إذا (dom_گره_مجموعه_user_داده((dom_گره *)file_box->gadget->گره,
				   corestring_dom_ns_کلید_file_name_گره_داده,
				   strdup(fn), html_dom_user_داده_handler,
				   &oldfile) == DOM_NO_ERR) {
		إذا (oldfile != عدم)
			حرر(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

فراغ html_مجموعه_file_gadget_filename_2(بنية hlcache_handle *hl,
	بنية form_control *gadget, ثابت حرف *fn)
{
	أرجع html_مجموعه_file_gadget_filename(hlcache_handle_get_content(hl),
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
ساكن bool html_drop_file_at_نقطه(بنية content *c, صحيح x, صحيح y, حرف *file)
{
	html_content *html = (html_content *) c;

	بنية box *box = html->layout;
	بنية box *بعدی;
	بنية box *file_box = عدم;
	بنية box *متن_box = عدم;
	صحيح box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	طالما ((بعدی = box_at_نقطه(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != عدم) {
		box = بعدی;

		إذا (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			استمر;

		إذا (box->iframe) {
			عائم scale = browser_پنجره_get_scale(box->iframe);
			أرجع browser_پنجره_drop_file_at_نقطه(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		إذا (box->object &&
		    content_drop_file_at_نقطه(box->object,
					x - box_x, y - box_y, file) == true)
			أرجع true;

		إذا (box->gadget) {
			بدل (box->gadget->type) {
				حالة GADGET_FILE:
					file_box = box;
				اكسر;

				حالة GADGET_TEXTBOX:
				حالة GADGET_TEXTAREA:
				حالة GADGET_PASSWORD:
					متن_box = box;
					اكسر;

				افتراضي:	/* appease compiler */
					اكسر;
			}
		}
	}

	إذا (!file_box && !متن_box)
		/* No box capable of handling drop */
		أرجع false;

	إذا (file == عدم)
		/* There is a box capable of handling drop here */
		أرجع true;

	/* Handle the drop */
	إذا (file_box) {
		/* File dropped on file input */
		html_مجموعه_file_gadget_filename(c, file_box->gadget, file);

	} وإلا {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = عدم;
		حرف *میانگیر;
		حرف *utf8_buff;
		nserror ret;
		بیعلامت صحيح اندازه;
		صحيح bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		إذا (fp == عدم) {
			/* Couldn't open file, but drop was for us */
			أرجع true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		إذا ((دراز)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			أرجع true;
		}

		/* Allocate buffer for file data */
		میانگیر = حافظه_بده(file_len + 1);
		إذا (میانگیر == عدم) {
			/* No memory, but drop was for us */
			fclose(fp);
			أرجع true;
		}

		/* Stick file into buffer */
		إذا (file_len != fread(میانگیر, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			حرر(میانگیر);
			fclose(fp);
			أرجع true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		میانگیر[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_مقصد_utf8(میانگیر, file_len, &utf8_buff);
		إذا (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			أكد(ret != NSERROR_BAD_ENCODING);
			حرر(میانگیر);
			أرجع true;
		}

		/* Done with buffer */
		حرر(میانگیر);

		/* Get new length */
		اندازه = طول_رشته(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(متن_box, &bx, &by);
		textarea_mouse_action(متن_box->gadget->داده.متن.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_متن(متن_box->gadget->داده.متن.ta,
				utf8_buff, اندازه);

		حرر(utf8_buff);
	}

	أرجع true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ساكن nserror
html_debug(بنية content *c, تعداد content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	أرجع NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ساكن nserror
html_debug_dump(بنية content *c, FILE *f, تعداد content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_گره *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	أكد(htmlc != عدم);

	إذا (op == CONTENT_DEBUG_RENDER) {
		أكد(htmlc->layout != عدم);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} وإلا {
		إذا (htmlc->document == عدم) {
			NSLOG(netsurf, INFO, "No document to dump");
			أرجع NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (فراغ *) &html);
		إذا ((exc != DOM_NO_ERR) || (html == عدم)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			أرجع NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_گره_unref(html);
	}

	أرجع ret;
}


#إذا ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ساكن فراغ
html_dump_frameset(بنية content_html_قابها *قاب, بیعلامت صحيح عمق)
{
	بیعلامت صحيح i;
	صحيح ردیف, ستون, index;
	ثابت حرف *unit[] = {"px", "%", "*"};
	ثابت حرف *scrolling[] = {"auto", "yes", "no"};

	أكد(قاب);

	fprintf(stderr, "%p ", قاب);

	fprintf(stderr, "(%i %i) ", قاب->ردیفها, قاب->تعداد_ستون);

	fprintf(stderr, "w%g%s ", قاب->عرض.مقدار, unit[قاب->عرض.unit]);
	fprintf(stderr, "h%g%s ", قاب->ارتفاع.مقدار,unit[قاب->ارتفاع.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			قاب->margin_عرض, قاب->margin_ارتفاع);

	إذا (قاب->name)
		fprintf(stderr, "'%s' ", قاب->name);
	إذا (قاب->url)
		fprintf(stderr, "<%s> ", قاب->url);

	إذا (قاب->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[قاب->scrolling]);
	إذا (قاب->border)
		fprintf(stderr, "border %x ",
				(بیعلامت صحيح) قاب->border_رنگ);

	fprintf(stderr, "\n");

	إذا (قاب->children) {
		لكل (ردیف = 0; ردیف != قاب->ردیفها; ردیف++) {
			لكل (ستون = 0; ستون != قاب->تعداد_ستون; ستون++) {
				لكل (i = 0; i != عمق; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", ردیف, ستون);
				index = (ردیف * قاب->تعداد_ستون) + ستون;
				html_dump_frameset(&قاب->children[index],
						عمق + 1);
			}
		}
	}
}

#پایان_اگر

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->document;
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
بنية box *html_get_box_درخت(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ساكن ثابت حرف *html_encoding(ثابت بنية content *c, تعداد content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ساكن حرف enc_token[10] = "Encoding0";

	أكد(html != عدم);

	إذا (op == CONTENT_ENCODING_منبع) {
		enc_token[8] = '0' + html->encoding_منبع;
		أرجع messages_get(enc_token);
	}

	أرجع html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
بنية content_html_قابها *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
بنية content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_پایه_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->پایه_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ثابت حرف *html_get_پایه_هدف(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != عدم);

	أرجع c->پایه_هدف;
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
bool html_get_id_جابه_جایی(hlcache_handle *h, lwc_string *frag_id, صحيح *x, صحيح *y)
{
	بنية box *pos;
	بنية box *layout;

	إذا (content_get_type(h) != CONTENT_HTML)
		أرجع false;

	layout = html_get_box_درخت(h);

	إذا ((pos = box_پیدا_کن_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		أرجع true;
	}
	أرجع false;
}

bool html_exec(بنية content *c, ثابت حرف *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool نتیجه = false;
	dom_exception err;
	dom_html_body_element *body_گره;
	dom_string *dom_src;
	dom_متن *متن_گره;
	dom_گره *یدکی_گره;
	dom_html_script_element *script_گره;

	إذا (htmlc->document == عدم) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		اذهب بیرون_no_string;
	}

	err = dom_string_create((ثابت uint8_t *)src, srclen, &dom_src);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		اذهب بیرون_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_گره);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		اذهب بیرون_no_body;
	}

	err = dom_document_create_متن_گره(htmlc->document, dom_src, &متن_گره);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		اذهب بیرون_no_متن_گره;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_گره);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		اذهب بیرون_no_script_گره;
	}

	err = dom_گره_append_فرزند(script_گره, متن_گره, &یدکی_گره);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		اذهب بیرون_unparented;
	}
	dom_گره_unref(یدکی_گره); /* We do not need the spare ref at all */

	err = dom_گره_append_فرزند(body_گره, script_گره, &یدکی_گره);
	إذا (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		اذهب بیرون_unparented;
	}
	dom_گره_unref(یدکی_گره); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	نتیجه = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_گره_get_والد_گره(script_گره, &یدکی_گره);
	إذا (err == DOM_NO_ERR && یدکی_گره != عدم) {
		dom_گره *دومی_یدکی;
		err = dom_گره_remove_فرزند(یدکی_گره, script_گره, &دومی_یدکی);
		إذا (err == DOM_NO_ERR) {
			dom_گره_unref(دومی_یدکی);
		}
		dom_گره_unref(یدکی_گره);
	}

بیرون_unparented:
	dom_گره_unref(script_گره);
بیرون_no_script_گره:
	dom_گره_unref(متن_گره);
بیرون_no_متن_گره:
	dom_گره_unref(body_گره);
بیرون_no_body:
	dom_string_unref(dom_src);
بیرون_no_string:
	أرجع نتیجه;
}

/* See \ref content_saw_insecure_objects */
ساكن bool
html_saw_insecure_objects(بنية content *c)
{
	html_content *htmlc = (html_content *)c;
	بنية content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	طالما (obj != عدم) {
		إذا (obj->content != عدم) {
			إذا (content_saw_insecure_objects(obj->content))
				أرجع true;
		}
		obj = obj->بعدی;
	}

	/* Now check the script list */
	إذا (html_saw_insecure_scripts(htmlc)) {
		أرجع true;
	}

	/* Now check stylesheets */
	إذا (html_css_saw_insecure_stylesheets(htmlc)) {
		أرجع true;
	}

	أرجع false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ساكن content_type html_content_type(فراغ)
{
	أرجع CONTENT_HTML;
}


ساكن فراغ html_fini(فراغ)
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
ساكن nserror
پیدا_کن_occurrences_html_box(ثابت حرف *الگو,
			  صحيح p_len,
			  بنية box *cur,
			  bool case_sens,
			  بنية textsearch_context *context)
{
	بنية box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	إذا (!cur->object && cur->متن) {
		ثابت حرف *متن = cur->متن;
		بیعلامت درازی = cur->درازی;

		طالما (درازی > 0) {
			بیعلامت تطابق_درازی;
			بیعلامت تطابق_جابه_جایی;
			ثابت حرف *new_متن;
			ثابت حرف *pos;

			pos = content_textsearch_پیدا_کن_الگو(متن,
					   درازی,
					   الگو,
					   p_len,
					   case_sens,
					   &تطابق_درازی);
			إذا (!pos)
				اكسر;

			/* found string in box => add to list */
			تطابق_جابه_جایی = pos - cur->متن;

			res = content_textsearch_جمع_کن_تطابق(context,
					cur->byte_جابه_جایی + تطابق_جابه_جایی,
					cur->byte_جابه_جایی + تطابق_جابه_جایی + تطابق_درازی,
					cur,
					cur);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			new_متن = pos + تطابق_درازی;
			درازی -= (new_متن - متن);
			متن = new_متن;
		}
	}

	/* and recurse */
	لكل (a = cur->children; a; a = a->بعدی) {
		res = پیدا_کن_occurrences_html_box(الگو,
						p_len,
						a,
						case_sens,
						context);
		إذا (res != NSERROR_OK) {
			أرجع res;
		}
	}

	أرجع res;
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
ساكن nserror
html_textsearch_پیدا_کن(بنية content *c,
		     بنية textsearch_context *context,
		     ثابت حرف *الگو,
		     صحيح p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	إذا (html->layout == عدم) {
		أرجع NSERROR_INVALID;
	}

	أرجع پیدا_کن_occurrences_html_box(الگو,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ساكن nserror
html_textsearch_bounds(بنية content *c,
		       بیعلامت شروع_idx,
		       بیعلامت انتها_idx,
		       بنية box *شروع_box,
		       بنية box *انتها_box,
		       بنية rect *bounds)
{
	/* get box position and jump to it */
	box_coords(شروع_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(انتها_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += انتها_box->عرض;
	bounds->y1 += انتها_box->ارتفاع;

	أرجع NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ساكن ثابت content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_داده = html_process_داده,
	.داده_complete = html_convert,
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
	.scroll_at_نقطه = html_scroll_at_نقطه,
	.drop_file_at_نقطه = html_drop_file_at_نقطه,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_پیدا_کن = html_textsearch_پیدا_کن,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_انتها = html_textselection_get_انتها,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(فراغ)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	إذا (error != NSERROR_OK)
		اذهب error;

	لكل (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_ثبات_handler(html_types[i],
				&html_content_handler);
		إذا (error != NSERROR_OK)
			اذهب error;
	}

	أرجع NSERROR_OK;

error:
	html_fini();

	أرجع error;
}
