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

#تعریف_کرو CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#تعریف_کرو ALWAYS_DUMP_FRAMESET 0
#تعریف_کرو ALWAYS_DUMP_BOX 0

ساکن ثابت حرف *html_types[] = {
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
ساکن bool fire_dom_event(dom_event *event, dom_گرہ *ہدف)
{
	dom_exception exc;
	bool نتیجہ;

	exc = dom_event_ہدف_dispatch_event(ہدف, event, &نتیجہ);
	اگر (exc != DOM_NO_ERR) {
		واپس false;
	}

	واپس نتیجہ;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_گرہ *ہدف,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool نتیجہ;

	exc = dom_event_create(&evt);
	اگر (exc != DOM_NO_ERR) واپس false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		واپس false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (صحیح)dom_string_لمبائی(type), dom_string_ڈیٹا(type), ہدف);
	نتیجہ = fire_dom_event(evt, ہدف);
	dom_event_unref(evt);
	واپس نتیجہ;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_گرہ *ہدف,
		bool bubbles, bool cancelable, uint32_t کلید)
{
	bool is_special = کلید <= 0x001F || (0x007F <= کلید && کلید <= 0x009F);
	dom_string *dom_کلید = تہی;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool نتیجہ;

	اگر (is_special) {
		انتخاب (کلید) {
		حالت NS_کلید_ESCAPE:
			dom_کلید = dom_string_ref(corestring_dom_Escape);
			توڑو;
		حالت NS_کلید_بایاں:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_بایاں);
			توڑو;
		حالت NS_کلید_دایاں:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_دایاں);
			توڑو;
		حالت NS_کلید_اوپر:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_اوپر);
			توڑو;
		حالت NS_کلید_نیچے_کو:
			dom_کلید = dom_string_ref(corestring_dom_Arrow_نیچے_کو);
			توڑو;
		حالت NS_کلید_صفحہ_اوپر:
			dom_کلید = dom_string_ref(corestring_dom_صفحہ_اوپر);
			توڑو;
		حالت NS_کلید_صفحہ_نیچے_کو:
			dom_کلید = dom_string_ref(corestring_dom_صفحہ_نیچے_کو);
			توڑو;
		حالت NS_کلید_متن_آغاز:
			dom_کلید = dom_string_ref(corestring_dom_Home);
			توڑو;
		حالت NS_کلید_متن_انتہا:
			dom_کلید = dom_string_ref(corestring_dom_انتہا);
			توڑو;
		طےشدہ:
			dom_کلید = تہی;
			توڑو;
		}
	} ورنہ {
		حرف utf8[6];
		size_t لمبائی = utf8_from_ucs4(کلید, utf8);
		utf8[لمبائی] = '\0';

		exc = dom_string_create((ثابت uint8_t *)utf8, لڑی_لمبائی(utf8),
				&dom_کلید);
		اگر (exc != DOM_NO_ERR) {
			واپس exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	اگر (exc != DOM_NO_ERR) {
		dom_string_unref(dom_کلید);
		واپس false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, تہی,
			dom_کلید, تہی, DOM_کلید_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_کلید);
	اگر (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		واپس false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(صحیح)dom_string_لمبائی(type), dom_string_ڈیٹا(type), ہدف);

	نتیجہ = fire_dom_event((dom_event *) evt, ہدف);
	dom_event_unref(evt);
	واپس نتیجہ;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ساکن خالی html_box_convert_مکمل(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_گرہ *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = تہی;

	/* Clean up and report error if unsuccessful or aborted */
	اگر ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		اگر (success == false) {
			content_نشر_error(&c->بنیاد, NSERROR_BOX_CONVERT, تہی);
		} ورنہ {
			content_نشر_error(&c->بنیاد, NSERROR_STOPPED, تہی);
		}

		content_سیٹ_error(&c->بنیاد);
		واپس;
	}


#اگر ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#اگر_ختم
#اگر ALWAYS_DUMP_FRAMESET
	اگر (c->frameset)
		html_dump_frameset(c->frameset, 0);
#اگر_ختم

	exc = dom_document_get_document_element(c->document, (خالی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == تہی)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_نشر_error(&c->بنیاد, NSERROR_DOM, تہی);
		content_سیٹ_error(&c->بنیاد);
		واپس;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_نکالو(c);
	اگر (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_نشر_error(&c->بنیاد, err, تہی);
		content_سیٹ_error(&c->بنیاد);
		dom_گرہ_unref(html);
		واپس;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = تہی;

	content_سیٹ_ready(&c->بنیاد);

	html_proceed_مقصد_مکمل(c);

	dom_گرہ_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_مقصد_مکمل(html_content *html)
{
	انتخاب (content_get_حالت_2(&html->بنیاد)) {
	حالت CONTENT_حالت_READY:
		اگر (html->بنیاد.فعال == 0) {
			content_سیٹ_مکمل(&html->بنیاد);
			واپس NSERROR_OK;
		}
		توڑو;
	حالت CONTENT_حالت_مکمل:
		/* fallthrough */
	حالت CONTENT_حالت_LOADING:
		واپس NSERROR_OK;
	طےشدہ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		توڑو;
	}
	واپس NSERROR_UNKNOWN;
}


ساکن خالی html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	غیرعلامتی f_سائز;
	غیرعلامتی f_min;
	غیرعلامتی w;
	غیرعلامتی h;
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا = {
		.getdims = {
			.viewport_چوڑائی = &w,
			.viewport_اونچائی = &h,
		},
	};

	content_نشر(&htmlc->بنیاد, CONTENT_پیغام_GETDIMS, &پیغام_ڈیٹا);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.چوڑائی  = w;
	htmlc->media.اونچائی = h;
	htmlc->unit_len_ctx.viewport_چوڑائی  = w;
	htmlc->unit_len_ctx.viewport_اونچائی = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_سائز = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_سائز)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_سائز)), F_10)), F_72);

	htmlc->unit_len_ctx.font_سائز_default = f_سائز;
	htmlc->unit_len_ctx.font_سائز_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
خالی html_اختتام_conversion(html_content *htmlc)
{
	اتحاد content_پیغام_ڈیٹا پیغام_ڈیٹا;
	dom_exception exc; /* returned by libdom functions */
	dom_گرہ *html;
	nserror error;

	/* Bail out if we've been aborted */
	اگر (htmlc->aborted) {
		content_نشر_error(&htmlc->بنیاد, NSERROR_STOPPED, تہی);
		content_سیٹ_error(&htmlc->بنیاد);
		واپس;
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
	اگر (htmlc->select_ctx != تہی) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		واپس;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	اگر (error != NSERROR_OK) {
		content_نشر_error(&htmlc->بنیاد, error, تہی);
		content_سیٹ_error(&htmlc->بنیاد);
		واپس;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	اگر (htmlc->jsthread != تہی) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, تہی);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_سیٹ_حالت(&htmlc->بنیاد, messages_get("Processing"));
	پیغام_ڈیٹا.explicit_حالت_متن = تہی;
	content_نشر(&htmlc->بنیاد, CONTENT_پیغام_حالت, &پیغام_ڈیٹا);

	exc = dom_document_get_document_element(htmlc->document, (خالی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == تہی)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_نشر_error(&htmlc->بنیاد, NSERROR_DOM, تہی);
		content_سیٹ_error(&htmlc->بنیاد);
		واپس;
	}

	html_get_dimensions(htmlc);

	error = dom_مقصد_box(html, htmlc, html_box_convert_مکمل, &htmlc->box_conversion_context);
	اگر (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_گرہ_unref(html);
		html_object_free_objects(htmlc);
		content_نشر_error(&htmlc->بنیاد, error, تہی);
		content_سیٹ_error(&htmlc->بنیاد);
		واپس;
	}

	dom_گرہ_unref(html);
}


ساکن خالی
html_document_user_ڈیٹا_handler(dom_گرہ_operation operation,
				dom_string *کلید, خالی *ڈیٹا,
				ساخت dom_گرہ *src,
				ساخت dom_گرہ *dst)
{
	اگر (dom_string_isequal(corestring_dom_ns_کلید_html_content_ڈیٹا,
			       کلید) == false || ڈیٹا == تہی) {
		واپس;
	}

	انتخاب (operation) {
	حالت DOM_گرہ_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		توڑو;
	حالت DOM_گرہ_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		توڑو;
	حالت DOM_گرہ_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		توڑو;
	حالت DOM_گرہ_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		توڑو;
	حالت DOM_گرہ_DELETED:
		/* This is the only path I expect */
		توڑو;
	طےشدہ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}


ساکن nserror
html_create_html_ڈیٹا(html_content *c, ثابت http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	خالی *old_گرہ_ڈیٹا;
	ثابت حرف *prefer_color_منوال = (nsoption_bool(prefer_dark_منوال)) ?
			"dark" : "light";

	c->parser = تہی;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = تہی;
	c->quirks = DOM_DOCUMENT_QUIRKS_منوال_NONE;
	c->encoding = تہی;
	c->بنیاد_url = nsurl_ref(content_get_url(&c->بنیاد));
	c->بنیاد_ہدف = تہی;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = تہی;
	c->bctx = تہی;
	c->layout = تہی;
	c->background_رنگ = NS_TRANSPARENT;
	c->stylesheet_گنتی = 0;
	c->stylesheets = تہی;
	c->select_ctx = تہی;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = تہی;
	c->عدد_objects = 0;
	c->object_list = تہی;
	c->forms = تہی;
	c->imagemaps = تہی;
	c->bw = تہی;
	c->frameset = تہی;
	c->iframe = تہی;
	c->صفحہ = تہی;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_گنتی = 0;
	c->scripts = تہی;
	c->jsthread = تہی;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->بنیاد.فعال = 1; /* The html content itself is active */

	اگر (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		واپس NSERROR_NOMEM;
	}

	اگر (lwc_intern_string(prefer_color_منوال, لڑی_لمبائی(prefer_color_منوال),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = تہی;
		واپس NSERROR_NOMEM;
	}

	c->sel = selection_create((ساخت content *)c);

	nerror = http_parameter_list_ڈھونڈو_شے(params, corestring_lwc_charset, &charset);
	اگر (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_ڈیٹا(charset));

		lwc_string_unref(charset);

		اگر (c->encoding == تہی) {
			lwc_string_unref(c->universal);
			c->universal = تہی;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = تہی;
			واپس NSERROR_NOMEM;

		}
		c->encoding_ماخذ = DOM_HUBBUB_ENCODING_ماخذ_سرخی;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.پیغام = تہی;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	اگر ((error != DOM_HUBBUB_OK) && (c->encoding != تہی)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		آزاد(c->encoding);
		c->encoding = تہی;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	اگر (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->بنیاد_url);
		c->بنیاد_url = تہی;

		lwc_string_unref(c->universal);
		c->universal = تہی;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = تہی;

		واپس libdom_hubbub_error_مقصد_nserror(error);
	}

	err = dom_گرہ_سیٹ_user_ڈیٹا(c->document,
				     corestring_dom_ns_کلید_html_content_ڈیٹا,
				     c, html_document_user_ڈیٹا_handler,
				     (خالی *) &old_گرہ_ڈیٹا);
	اگر (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = تہی;
		nsurl_unref(c->بنیاد_url);
		c->بنیاد_url = تہی;

		lwc_string_unref(c->universal);
		c->universal = تہی;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = تہی;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		واپس NSERROR_DOM;
	}

	أكد(old_گرہ_ڈیٹا == تہی);

	واپس NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ساکن nserror
html_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت http_parameter *params,
	    llcache_handle *llcache,
	    ثابت حرف *fallback_charset,
	    bool quirks,
	    ساخت content **c)
{
	html_content *html;
	nserror error;

	html = صفر_حافظہ_دو(1, حجم(html_content));
	اگر (html == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&html->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(html);
		واپس error;
	}

	error = html_create_html_ڈیٹا(html, params);
	اگر (error != NSERROR_OK) {
		content_نشر_error(&html->بنیاد, error, تہی);
		آزاد(html);
		واپس error;
	}

	error = html_css_new_stylesheets(html);
	اگر (error != NSERROR_OK) {
		content_نشر_error(&html->بنیاد, error, تہی);
		آزاد(html);
		واپس error;
	}

	*c = (ساخت content *) html;

	واپس NSERROR_OK;
}



ساکن nserror
html_process_encoding_change(ساخت content *c,
			     ثابت حرف *ڈیٹا,
			     غیرعلامتی صحیح سائز)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ثابت حرف *encoding;
	ثابت uint8_t *ماخذ_ڈیٹا;
	size_t ماخذ_سائز;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_ماخذ);
	اگر (encoding == تہی) {
		واپس NSERROR_NOMEM;
	}

	اگر (html->encoding != تہی) {
		آزاد(html->encoding);
		html->encoding = تہی;
	}

	html->encoding = strdup(encoding);
	اگر (html->encoding == تہی) {
		واپس NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = تہی;

	اگر (html->document != تہی) {
		dom_گرہ_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.پیغام = تہی;
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
		اگر (html->encoding == تہی) {
			واپس NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		اگر (error != DOM_HUBBUB_OK) {
			واپس libdom_hubbub_error_مقصد_nserror(error);
		}

	}

	ماخذ_ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &ماخذ_سائز);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      ماخذ_ڈیٹا,
					      ماخذ_سائز);

	واپس libdom_hubbub_error_مقصد_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ساکن bool
html_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا, غیرعلامتی صحیح سائز)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ثابت uint8_t *) ڈیٹا,
					      سائز);

	err = libdom_hubbub_error_مقصد_nserror(dom_ret);

	/* deal with encoding change */
	اگر (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, ڈیٹا, سائز);
	}

	/* broadcast the error if necessary */
	اگر (err != NSERROR_OK) {
		content_نشر_error(c, err, تہی);
		واپس false;
	}

	واپس true;
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

ساکن bool html_convert(ساخت content *c)
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
	exc = dom_document_get_quirks_منوال(htmlc->document, &htmlc->quirks);
	اگر (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->بنیاد.فعال--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->بنیاد.فعال, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	اگر (html_can_begin_conversion(htmlc)) {
		واپس html_begin_conversion(htmlc);
	}
	واپس true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	غیرعلامتی صحیح i;

	/* Cannot begin conversion if we're still fetching stuff */
	اگر (htmlc->بنیاد.فعال != 0)
		واپس false;

	برائے (i = 0; i != htmlc->stylesheet_گنتی; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		اگر (htmlc->stylesheets[i].modified)
			واپس false;
	}

	/* All is good, begin */
	واپس true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_گرہ *html;
	nserror ns_error;
	ساخت form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *گرہ_name = تہی;
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
		اگر (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->بنیاد.فعال > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			واپس true;
		}
		اگر (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_نشر_error(&htmlc->بنیاد,
						libdom_hubbub_error_مقصد_nserror(error),
						تہی);

			واپس false;
		}
		htmlc->parse_completed = true;
	}

	اگر (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		واپس true;
	}

	/* Give up processing if we've been aborted */
	اگر (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->بنیاد.فعال);
		content_سیٹ_error(&htmlc->بنیاد);
		content_نشر_error(&htmlc->بنیاد, NSERROR_STOPPED, تہی);
		واپس false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	اگر (htmlc->encoding == تہی) {
		ثابت حرف *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_ماخذ);
		اگر (encoding == تہی) {
			content_نشر_error(&htmlc->بنیاد,
						NSERROR_NOMEM,
						تہی);
			واپس false;
		}

		htmlc->encoding = strdup(encoding);
		اگر (htmlc->encoding == تہی) {
			content_نشر_error(&htmlc->بنیاد,
						NSERROR_NOMEM,
						تہی);
			واپس false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (خالی *) &html);
	اگر ((exc != DOM_NO_ERR) || (html == تہی)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_نشر_error(&htmlc->بنیاد, NSERROR_DOM, تہی);
		واپس false;
	}

	exc = dom_گرہ_get_گرہ_name(html, &گرہ_name);
	اگر ((exc != DOM_NO_ERR) ||
	    (گرہ_name == تہی) ||
	    (!dom_string_caseless_lwc_isequal(گرہ_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_نشر_error(&htmlc->بنیاد, NSERROR_DOM, تہی);
		dom_گرہ_unref(html);
		واپس false;
	}
	dom_string_unref(گرہ_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	برائے (f = htmlc->forms; f != تہی; f = f->پچھلا) {
		nsurl *action;

		/* Make all actions absolute */
		اگر (f->action == تہی || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->بنیاد);
			ns_error = nsurl_join(htmlc->بنیاد_url,
					      nsurl_access(doc_addr),
					      &action);
		} ورنہ {
			ns_error = nsurl_join(htmlc->بنیاد_url,
					      f->action,
					      &action);
		}

		اگر (ns_error != NSERROR_OK) {
			content_نشر_error(&htmlc->بنیاد, ns_error, تہی);

			dom_گرہ_unref(html);
			واپس false;
		}

		آزاد(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		اگر (f->action == تہی) {
			content_نشر_error(&htmlc->بنیاد,
						NSERROR_NOMEM,
						تہی);

			dom_گرہ_unref(html);
			واپس false;
		}

		/* Ensure each form has a document encoding */
		اگر (f->document_charset == تہی) {
			f->document_charset = strdup(htmlc->encoding);
			اگر (f->document_charset == تہی) {
				content_نشر_error(&htmlc->بنیاد,
							NSERROR_NOMEM,
							تہی);
				dom_گرہ_unref(html);
				واپس false;
			}
		}
	}

	dom_گرہ_unref(html);

	اگر (htmlc->بنیاد.فعال == 0) {
		html_اختتام_conversion(htmlc);
	}

	واپس true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ساکن خالی html_stop(ساخت content *c)
{
	html_content *htmlc = (html_content *) c;

	انتخاب (c->حالت_2) {
	حالت CONTENT_حالت_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		اگر (htmlc->jsthread != تہی) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		توڑو;

	حالت CONTENT_حالت_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		اگر (c->حالت_2 == CONTENT_حالت_READY && c->فعال == 0) {
			content_سیٹ_مکمل(c);
		}

		توڑو;

	حالت CONTENT_حالت_مکمل:
		/* Nothing to do */
		توڑو;

	طےشدہ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->حالت_2,
		      c);
		أكد(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ساکن خالی html_reformat(ساخت content *c, صحیح چوڑائی, صحیح اونچائی)
{
	html_content *htmlc = (html_content *) c;
	ساخت box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_چوڑائی = css_unit_device2css_px(
			INTTOFIX(چوڑائی), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_اونچائی = css_unit_device2css_px(
			INTTOFIX(اونچائی), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.جڑ_style = htmlc->layout->style;

	layout_document(htmlc, چوڑائی, اونچائی);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->چوڑائی = layout->x + layout->padding[بایاں_2] + layout->چوڑائی +
		layout->padding[دایاں_2] + layout->border[دایاں_2].چوڑائی +
		layout->margin[دایاں_2];
	c->اونچائی = layout->y + layout->padding[چوٹی_2] + layout->اونچائی +
		layout->padding[BOTTOM] + layout->border[BOTTOM].چوڑائی +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	اگر (c->چوڑائی < layout->x + layout->descendant_x1)
		c->چوڑائی = layout->x + layout->descendant_x1;
	اگر (c->اونچائی < layout->y + layout->descendant_y1)
		c->اونچائی = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	اگر (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_وقت = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

خالی html_redraw_a_box(hlcache_handle *h, ساخت box *box)
{
	صحیح x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2],
			box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

خالی html__redraw_a_box(ساخت html_content *html, ساخت box *box)
{
	صحیح x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ساخت content *)html, x, y,
			box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2],
			box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM]);
}

ساکن خالی html_destroy_frameset(ساخت content_html_چوکھٹے *frameset)
{
	صحیح i;

	اگر (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = تہی;
	}
	اگر (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = تہی;
	}
	اگر (frameset->children) {
		برائے (i = 0; i < (frameset->صفیں * frameset->ستون_تعداد); i++) {
			اگر (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = تہی;
			}
			اگر (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = تہی;
			}
			اگر (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = تہی;
	}
}

ساکن خالی html_destroy_iframe(ساخت content_html_iframe *iframe)
{
	ساخت content_html_iframe *اگلا;
	اگلا = iframe;
	جب ((iframe = اگلا) != تہی) {
		اگلا = iframe->اگلا;
		اگر (iframe->name)
			talloc_free(iframe->name);
		اگر (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = تہی;
		}
		talloc_free(iframe);
	}
}


ساکن خالی html_free_layout(html_content *htmlc)
{
	اگر (htmlc->bctx != تہی) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ساکن خالی html_destroy(ساخت content *c)
{
	html_content *html = (html_content *) c;
	ساخت form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	اگر (html->box_conversion_context != تہی) {
		اگر (cancel_dom_مقصد_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	برائے (f = html->forms; f != تہی; f = g) {
		g = f->پچھلا;

		form_free(f);
	}

	imagemap_destroy(html);

	اگر (c->refresh)
		nsurl_unref(c->refresh);

	اگر (html->بنیاد_url)
		nsurl_unref(html->بنیاد_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	اگر (html->jsthread != تہی) {
		js_destroythread(html->jsthread);
		html->jsthread = تہی;
	}

	اگر (html->parser != تہی) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = تہی;
	}

	اگر (html->document != تہی) {
		dom_گرہ_unref(html->document);
		html->document = تہی;
	}

	اگر (html->title != تہی) {
		dom_گرہ_unref(html->title);
		html->title = تہی;
	}

	/* Free encoding */
	اگر (html->encoding != تہی) {
		آزاد(html->encoding);
		html->encoding = تہی;
	}

	/* Free base target */
	اگر (html->بنیاد_ہدف != تہی) {
		آزاد(html->بنیاد_ہدف);
		html->بنیاد_ہدف = تہی;
	}

	/* Free frameset */
	اگر (html->frameset != تہی) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = تہی;
	}

	/* Free iframes */
	اگر (html->iframe != تہی) {
		html_destroy_iframe(html->iframe);
		html->iframe = تہی;
	}

	/* Destroy selection context */
	اگر (html->select_ctx != تہی) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = تہی;
	}

	lwc_string_unref(html->universal);
	html->universal = تہی;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = تہی;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ساکن nserror html_clone(ثابت ساخت content *old, ساخت content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	أكد(0 && "html_clone should never be called");

	واپس true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ساکن nserror
html_open(ساخت content *c,
	  ساخت browser_کھڑکی *bw,
	  ساخت content *صفحہ,
	  ساخت object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->صفحہ = (html_content *) صفحہ;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	واپس NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ساکن nserror html_close(ساخت content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = تہی;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	اگر (htmlc->jsthread != تہی) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	واپس ret;
}


/**
 * Return an HTML content's selection context
 */

ساکن خالی html_clear_selection(ساخت content *c)
{
	html_content *html = (html_content *) c;

	انتخاب (html->selection_type) {
	حالت HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		توڑو;
	حالت HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->ڈیٹا.متن.ta);
		توڑو;
	حالت HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		توڑو;
	حالت HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ساکن حرف *html_get_selection(ساخت content *c)
{
	html_content *html = (html_content *) c;

	انتخاب (html->selection_type) {
	حالت HTML_SELECTION_TEXTAREA:
		واپس textarea_get_selection(html->selection_owner.textarea->
				gadget->ڈیٹا.متن.ta);
	حالت HTML_SELECTION_SELF:
		أكد(html->selection_owner.none == false);
		واپس selection_get_copy(html->sel);
	حالت HTML_SELECTION_CONTENT:
		واپس content_get_selection(
				html->selection_owner.content->object);
	حالت HTML_SELECTION_NONE:
		/* Nothing to do */
		أكد(html->selection_owner.none == true);
		توڑو;
	طےشدہ:
		توڑو;
	}

	واپس تہی;
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
ساکن nserror
html_get_contextual_content(ساخت content *c, صحیح x, صحیح y,
			    ساخت browser_کھڑکی_features *ڈیٹا)
{
	html_content *html = (html_content *) c;

	ساخت box *box = html->layout;
	ساخت box *اگلا;
	صحیح box_x = 0, box_y = 0;

	جب ((اگلا = box_at_نقطہ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != تہی) {
		box = اگلا;

		/* hidden boxes are ignored */
		اگر ((box->style != تہی) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			جاری;
		}

		اگر (box->iframe) {
			اعشاری scale = browser_کھڑکی_get_scale(box->iframe);
			browser_کھڑکی_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    ڈیٹا);
		}

		اگر (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, ڈیٹا);

		اگر (box->object)
			ڈیٹا->object = box->object;

		اگر (box->href) {
			ڈیٹا->link = box->href;
			ڈیٹا->link_title = box->متن;
			ڈیٹا->link_title_لمبائی = box->لمبائی;
		}

		اگر (box->usemap) {
			ثابت حرف *ہدف = تہی;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &ہدف);
			/* Box might have imagemap, but no actual link area
			 * at point */
			اگر (url != تہی)
				ڈیٹا->link = url;
		}
		اگر (box->gadget) {
			انتخاب (box->gadget->type) {
			حالت GADGET_TEXTBOX:
			حالت GADGET_TEXTAREA:
			حالت GADGET_PASSWORD:
				ڈیٹا->form_features = CTX_FORM_متن;
				توڑو;

			حالت GADGET_FILE:
				ڈیٹا->form_features = CTX_FORM_FILE;
				توڑو;

			طےشدہ:
				ڈیٹا->form_features = CTX_FORM_NONE;
				توڑو;
			}
		}
	}
	واپس NSERROR_OK;
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
ساکن bool
html_scroll_at_نقطہ(ساخت content *c, صحیح x, صحیح y, صحیح scrx, صحیح scry)
{
	html_content *html = (html_content *) c;

	ساخت box *box = html->layout;
	ساخت box *اگلا;
	صحیح box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	جب ((اگلا = box_at_نقطہ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != تہی) {
		box = اگلا;

		اگر (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			جاری;

		/* Pass into iframe */
		اگر (box->iframe) {
			اعشاری scale = browser_کھڑکی_get_scale(box->iframe);

			اگر (browser_کھڑکی_scroll_at_نقطہ(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				واپس true;
		}

		/* Pass into textarea widget */
		اگر (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->ڈیٹا.متن.ta,
						scrx, scry) == true)
			واپس true;

		/* Pass into object */
		اگر (box->object != تہی && content_scroll_at_نقطہ(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			واپس true;

		/* Handle box scrollbars */
		اگر (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		اگر (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		اگر (handled_scroll == true)
			واپس true;
	}

	واپس false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ساکن خالی html_dom_user_ڈیٹا_handler(dom_گرہ_operation operation,
		dom_string *کلید, خالی *ڈیٹا_2, ساخت dom_گرہ *src,
		ساخت dom_گرہ *dst)
{
	حرف *oldfile;
	حرف *ڈیٹا = (حرف *)ڈیٹا_2;

	اگر (!dom_string_isequal(corestring_dom_ns_کلید_file_name_گرہ_ڈیٹا,
				کلید) || ڈیٹا == تہی) {
		واپس;
	}

	انتخاب (operation) {
	حالت DOM_گرہ_CLONED:
		اگر (dom_گرہ_سیٹ_user_ڈیٹا(dst,
					   corestring_dom_ns_کلید_file_name_گرہ_ڈیٹا,
					   strdup(ڈیٹا), html_dom_user_ڈیٹا_handler,
					   &oldfile) == DOM_NO_ERR) {
			اگر (oldfile != تہی)
				آزاد(oldfile);
		}
		توڑو;

	حالت DOM_گرہ_RENAMED:
	حالت DOM_گرہ_IMPORTED:
	حالت DOM_گرہ_ADOPTED:
		توڑو;

	حالت DOM_گرہ_DELETED:
		آزاد(ڈیٹا);
		توڑو;
	طےشدہ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		أكد(0);
	}
}

ساکن خالی html_سیٹ_file_gadget_filename(ساخت content *c,
	ساخت form_control *gadget, ثابت حرف *fn)
{
	nserror ret;
	حرف *utf8_fn, *oldfile = تہی;
	html_content *html = (html_content *)c;
	ساخت box *file_box = gadget->box;

	ret = guit->utf8->local_مقصد_utf8(fn, 0, &utf8_fn);
	اگر (ret != NSERROR_OK) {
		أكد(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		واپس;
	}

	form_gadget_update_قدر(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	اگر (dom_گرہ_سیٹ_user_ڈیٹا((dom_گرہ *)file_box->gadget->گرہ,
				   corestring_dom_ns_کلید_file_name_گرہ_ڈیٹا,
				   strdup(fn), html_dom_user_ڈیٹا_handler,
				   &oldfile) == DOM_NO_ERR) {
		اگر (oldfile != تہی)
			آزاد(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

خالی html_سیٹ_file_gadget_filename_2(ساخت hlcache_handle *hl,
	ساخت form_control *gadget, ثابت حرف *fn)
{
	واپس html_سیٹ_file_gadget_filename(hlcache_handle_get_content(hl),
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
ساکن bool html_drop_file_at_نقطہ(ساخت content *c, صحیح x, صحیح y, حرف *file)
{
	html_content *html = (html_content *) c;

	ساخت box *box = html->layout;
	ساخت box *اگلا;
	ساخت box *file_box = تہی;
	ساخت box *متن_box = تہی;
	صحیح box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	جب ((اگلا = box_at_نقطہ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != تہی) {
		box = اگلا;

		اگر (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			جاری;

		اگر (box->iframe) {
			اعشاری scale = browser_کھڑکی_get_scale(box->iframe);
			واپس browser_کھڑکی_drop_file_at_نقطہ(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		اگر (box->object &&
		    content_drop_file_at_نقطہ(box->object,
					x - box_x, y - box_y, file) == true)
			واپس true;

		اگر (box->gadget) {
			انتخاب (box->gadget->type) {
				حالت GADGET_FILE:
					file_box = box;
				توڑو;

				حالت GADGET_TEXTBOX:
				حالت GADGET_TEXTAREA:
				حالت GADGET_PASSWORD:
					متن_box = box;
					توڑو;

				طےشدہ:	/* appease compiler */
					توڑو;
			}
		}
	}

	اگر (!file_box && !متن_box)
		/* No box capable of handling drop */
		واپس false;

	اگر (file == تہی)
		/* There is a box capable of handling drop here */
		واپس true;

	/* Handle the drop */
	اگر (file_box) {
		/* File dropped on file input */
		html_سیٹ_file_gadget_filename(c, file_box->gadget, file);

	} ورنہ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = تہی;
		حرف *بفر;
		حرف *utf8_buff;
		nserror ret;
		غیرعلامتی صحیح سائز;
		صحیح bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		اگر (fp == تہی) {
			/* Couldn't open file, but drop was for us */
			واپس true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		اگر ((طویل)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			واپس true;
		}

		/* Allocate buffer for file data */
		بفر = حافظہ_دو(file_len + 1);
		اگر (بفر == تہی) {
			/* No memory, but drop was for us */
			fclose(fp);
			واپس true;
		}

		/* Stick file into buffer */
		اگر (file_len != fread(بفر, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			آزاد(بفر);
			fclose(fp);
			واپس true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		بفر[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_مقصد_utf8(بفر, file_len, &utf8_buff);
		اگر (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			أكد(ret != NSERROR_BAD_ENCODING);
			آزاد(بفر);
			واپس true;
		}

		/* Done with buffer */
		آزاد(بفر);

		/* Get new length */
		سائز = لڑی_لمبائی(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(متن_box, &bx, &by);
		textarea_mouse_action(متن_box->gadget->ڈیٹا.متن.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_متن(متن_box->gadget->ڈیٹا.متن.ta,
				utf8_buff, سائز);

		آزاد(utf8_buff);
	}

	واپس true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ساکن nserror
html_debug(ساخت content *c, شمار content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	واپس NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ساکن nserror
html_debug_dump(ساخت content *c, FILE *f, شمار content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_گرہ *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	أكد(htmlc != تہی);

	اگر (op == CONTENT_DEBUG_RENDER) {
		أكد(htmlc->layout != تہی);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ورنہ {
		اگر (htmlc->document == تہی) {
			NSLOG(netsurf, INFO, "No document to dump");
			واپس NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (خالی *) &html);
		اگر ((exc != DOM_NO_ERR) || (html == تہی)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			واپس NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_گرہ_unref(html);
	}

	واپس ret;
}


#اگر ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ساکن خالی
html_dump_frameset(ساخت content_html_چوکھٹے *چوکھٹا, غیرعلامتی صحیح گہرائی)
{
	غیرعلامتی صحیح i;
	صحیح صف, ستون, index;
	ثابت حرف *unit[] = {"px", "%", "*"};
	ثابت حرف *scrolling[] = {"auto", "yes", "no"};

	أكد(چوکھٹا);

	fprintf(stderr, "%p ", چوکھٹا);

	fprintf(stderr, "(%i %i) ", چوکھٹا->صفیں, چوکھٹا->ستون_تعداد);

	fprintf(stderr, "w%g%s ", چوکھٹا->چوڑائی.قدر, unit[چوکھٹا->چوڑائی.unit]);
	fprintf(stderr, "h%g%s ", چوکھٹا->اونچائی.قدر,unit[چوکھٹا->اونچائی.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			چوکھٹا->margin_چوڑائی, چوکھٹا->margin_اونچائی);

	اگر (چوکھٹا->name)
		fprintf(stderr, "'%s' ", چوکھٹا->name);
	اگر (چوکھٹا->url)
		fprintf(stderr, "<%s> ", چوکھٹا->url);

	اگر (چوکھٹا->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[چوکھٹا->scrolling]);
	اگر (چوکھٹا->border)
		fprintf(stderr, "border %x ",
				(غیرعلامتی صحیح) چوکھٹا->border_رنگ);

	fprintf(stderr, "\n");

	اگر (چوکھٹا->children) {
		برائے (صف = 0; صف != چوکھٹا->صفیں; صف++) {
			برائے (ستون = 0; ستون != چوکھٹا->ستون_تعداد; ستون++) {
				برائے (i = 0; i != گہرائی; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", صف, ستون);
				index = (صف * چوکھٹا->ستون_تعداد) + ستون;
				html_dump_frameset(&چوکھٹا->children[index],
						گہرائی + 1);
			}
		}
	}
}

#اگر_ختم

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->document;
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
ساخت box *html_get_box_درخت(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ساکن ثابت حرف *html_encoding(ثابت ساخت content *c, شمار content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ساکن حرف enc_token[10] = "Encoding0";

	أكد(html != تہی);

	اگر (op == CONTENT_ENCODING_ماخذ) {
		enc_token[8] = '0' + html->encoding_ماخذ;
		واپس messages_get(enc_token);
	}

	واپس html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ساخت content_html_چوکھٹے *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ساخت content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_بنیاد_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->بنیاد_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ثابت حرف *html_get_بنیاد_ہدف(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->بنیاد_ہدف;
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
bool html_get_id_انحراف(hlcache_handle *h, lwc_string *frag_id, صحیح *x, صحیح *y)
{
	ساخت box *pos;
	ساخت box *layout;

	اگر (content_get_type(h) != CONTENT_HTML)
		واپس false;

	layout = html_get_box_درخت(h);

	اگر ((pos = box_ڈھونڈو_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		واپس true;
	}
	واپس false;
}

bool html_exec(ساخت content *c, ثابت حرف *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool نتیجہ = false;
	dom_exception err;
	dom_html_body_element *body_گرہ;
	dom_string *dom_src;
	dom_متن *متن_گرہ;
	dom_گرہ *فالتو_گرہ;
	dom_html_script_element *script_گرہ;

	اگر (htmlc->document == تہی) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		جاؤ برآمد_no_string;
	}

	err = dom_string_create((ثابت uint8_t *)src, srclen, &dom_src);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		جاؤ برآمد_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_گرہ);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		جاؤ برآمد_no_body;
	}

	err = dom_document_create_متن_گرہ(htmlc->document, dom_src, &متن_گرہ);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		جاؤ برآمد_no_متن_گرہ;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_گرہ);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		جاؤ برآمد_no_script_گرہ;
	}

	err = dom_گرہ_append_بچہ(script_گرہ, متن_گرہ, &فالتو_گرہ);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		جاؤ برآمد_unparented;
	}
	dom_گرہ_unref(فالتو_گرہ); /* We do not need the spare ref at all */

	err = dom_گرہ_append_بچہ(body_گرہ, script_گرہ, &فالتو_گرہ);
	اگر (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		جاؤ برآمد_unparented;
	}
	dom_گرہ_unref(فالتو_گرہ); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	نتیجہ = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_گرہ_get_والد_گرہ(script_گرہ, &فالتو_گرہ);
	اگر (err == DOM_NO_ERR && فالتو_گرہ != تہی) {
		dom_گرہ *دوسرا_فالتو;
		err = dom_گرہ_remove_بچہ(فالتو_گرہ, script_گرہ, &دوسرا_فالتو);
		اگر (err == DOM_NO_ERR) {
			dom_گرہ_unref(دوسرا_فالتو);
		}
		dom_گرہ_unref(فالتو_گرہ);
	}

برآمد_unparented:
	dom_گرہ_unref(script_گرہ);
برآمد_no_script_گرہ:
	dom_گرہ_unref(متن_گرہ);
برآمد_no_متن_گرہ:
	dom_گرہ_unref(body_گرہ);
برآمد_no_body:
	dom_string_unref(dom_src);
برآمد_no_string:
	واپس نتیجہ;
}

/* See \ref content_saw_insecure_objects */
ساکن bool
html_saw_insecure_objects(ساخت content *c)
{
	html_content *htmlc = (html_content *)c;
	ساخت content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	جب (obj != تہی) {
		اگر (obj->content != تہی) {
			اگر (content_saw_insecure_objects(obj->content))
				واپس true;
		}
		obj = obj->اگلا;
	}

	/* Now check the script list */
	اگر (html_saw_insecure_scripts(htmlc)) {
		واپس true;
	}

	/* Now check stylesheets */
	اگر (html_css_saw_insecure_stylesheets(htmlc)) {
		واپس true;
	}

	واپس false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ساکن content_type html_content_type(خالی)
{
	واپس CONTENT_HTML;
}


ساکن خالی html_fini(خالی)
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
ساکن nserror
ڈھونڈو_occurrences_html_box(ثابت حرف *نمونہ,
			  صحیح p_len,
			  ساخت box *cur,
			  bool case_sens,
			  ساخت textsearch_context *context)
{
	ساخت box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	اگر (!cur->object && cur->متن) {
		ثابت حرف *متن = cur->متن;
		غیرعلامتی لمبائی = cur->لمبائی;

		جب (لمبائی > 0) {
			غیرعلامتی مطابقت_لمبائی;
			غیرعلامتی مطابقت_انحراف;
			ثابت حرف *new_متن;
			ثابت حرف *pos;

			pos = content_textsearch_ڈھونڈو_نمونہ(متن,
					   لمبائی,
					   نمونہ,
					   p_len,
					   case_sens,
					   &مطابقت_لمبائی);
			اگر (!pos)
				توڑو;

			/* found string in box => add to list */
			مطابقت_انحراف = pos - cur->متن;

			res = content_textsearch_جوڑو_مطابقت(context,
					cur->byte_انحراف + مطابقت_انحراف,
					cur->byte_انحراف + مطابقت_انحراف + مطابقت_لمبائی,
					cur,
					cur);
			اگر (res != NSERROR_OK) {
				واپس res;
			}

			new_متن = pos + مطابقت_لمبائی;
			لمبائی -= (new_متن - متن);
			متن = new_متن;
		}
	}

	/* and recurse */
	برائے (a = cur->children; a; a = a->اگلا) {
		res = ڈھونڈو_occurrences_html_box(نمونہ,
						p_len,
						a,
						case_sens,
						context);
		اگر (res != NSERROR_OK) {
			واپس res;
		}
	}

	واپس res;
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
ساکن nserror
html_textsearch_ڈھونڈو(ساخت content *c,
		     ساخت textsearch_context *context,
		     ثابت حرف *نمونہ,
		     صحیح p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	اگر (html->layout == تہی) {
		واپس NSERROR_INVALID;
	}

	واپس ڈھونڈو_occurrences_html_box(نمونہ,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ساکن nserror
html_textsearch_bounds(ساخت content *c,
		       غیرعلامتی آغاز_idx,
		       غیرعلامتی انتہا_idx,
		       ساخت box *آغاز_box,
		       ساخت box *انتہا_box,
		       ساخت rect *bounds)
{
	/* get box position and jump to it */
	box_coords(آغاز_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(انتہا_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += انتہا_box->چوڑائی;
	bounds->y1 += انتہا_box->اونچائی;

	واپس NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ساکن ثابت content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_ڈیٹا = html_process_ڈیٹا,
	.ڈیٹا_complete = html_convert,
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
	.scroll_at_نقطہ = html_scroll_at_نقطہ,
	.drop_file_at_نقطہ = html_drop_file_at_نقطہ,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_ڈھونڈو = html_textsearch_ڈھونڈو,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_انتہا = html_textselection_get_انتہا,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(خالی)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	اگر (error != NSERROR_OK)
		جاؤ error;

	برائے (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_رجسٹر_handler(html_types[i],
				&html_content_handler);
		اگر (error != NSERROR_OK)
			جاؤ error;
	}

	واپس NSERROR_OK;

error:
	html_fini();

	واپس error;
}
