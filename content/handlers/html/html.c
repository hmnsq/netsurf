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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <nsutils/time.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/libdom.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/utf8.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/string.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/utf8.h"
#अन्तर्भाव "netsurf/keypress.h"
#अन्तर्भाव "netsurf/layout.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/textsearch.h"
#अन्तर्भाव "desktop/selection.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/textarea.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "javascript/js.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/dom_event.h"
#अन्तर्भाव "html/css.h"
#अन्तर्भाव "html/object.h"
#अन्तर्भाव "html/html_save.h"
#अन्तर्भाव "html/interaction.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_construct.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/imagemap.h"
#अन्तर्भाव "html/layout.h"
#अन्तर्भाव "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

स्थिर अचर वर्ण *html_types[] = {
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
स्थिर bool fire_dom_event(dom_event *event, dom_गाँठो *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	यदि (exc != DOM_NO_ERR) {
		फर्काऊ false;
	}

	फर्काऊ result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_गाँठो *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	यदि (exc != DOM_NO_ERR) फर्काऊ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	यदि (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		फर्काऊ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (पूर्णाङ्क)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	फर्काऊ result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_गाँठो *target,
		bool bubbles, bool cancelable, uint32_t साँचो)
{
	bool is_special = साँचो <= 0x001F || (0x007F <= साँचो && साँचो <= 0x009F);
	dom_string *dom_साँचो = रिक्त;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	यदि (is_special) {
		विकल्प (साँचो) {
		पक्ष NS_साँचो_ESCAPE:
			dom_साँचो = dom_string_ref(corestring_dom_Escape);
			रोक;
		पक्ष NS_साँचो_बायाँ:
			dom_साँचो = dom_string_ref(corestring_dom_Arrow_बायाँ);
			रोक;
		पक्ष NS_साँचो_दायाँ:
			dom_साँचो = dom_string_ref(corestring_dom_Arrow_दायाँ);
			रोक;
		पक्ष NS_साँचो_UP:
			dom_साँचो = dom_string_ref(corestring_dom_ArrowUp);
			रोक;
		पक्ष NS_साँचो_DOWN:
			dom_साँचो = dom_string_ref(corestring_dom_ArrowDown);
			रोक;
		पक्ष NS_साँचो_PAGE_UP:
			dom_साँचो = dom_string_ref(corestring_dom_PageUp);
			रोक;
		पक्ष NS_साँचो_PAGE_DOWN:
			dom_साँचो = dom_string_ref(corestring_dom_PageDown);
			रोक;
		पक्ष NS_साँचो_TEXT_START:
			dom_साँचो = dom_string_ref(corestring_dom_Home);
			रोक;
		पक्ष NS_साँचो_TEXT_END:
			dom_साँचो = dom_string_ref(corestring_dom_End);
			रोक;
		स्वभाव:
			dom_साँचो = रिक्त;
			रोक;
		}
	} अन्यथा {
		वर्ण utf8[6];
		माप_प्रकार length = utf8_from_ucs4(साँचो, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((अचर uint8_t *)utf8, म_माप(utf8),
				&dom_साँचो);
		यदि (exc != DOM_NO_ERR) {
			फर्काऊ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	यदि (exc != DOM_NO_ERR) {
		dom_string_unref(dom_साँचो);
		फर्काऊ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, रिक्त,
			dom_साँचो, रिक्त, DOM_साँचो_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_साँचो);
	यदि (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		फर्काऊ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(पूर्णाङ्क)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	फर्काऊ result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
स्थिर शून्य html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_गाँठो *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = रिक्त;

	/* Clean up and report error if unsuccessful or aborted */
	यदि ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		यदि (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, रिक्त);
		} अन्यथा {
			content_broadcast_error(&c->base, NSERROR_STOPPED, रिक्त);
		}

		content_set_error(&c->base);
		फर्काऊ;
	}


#यदि ALWAYS_DUMP_BOX
	box_dump(मानक_त्रुटि, c->layout->children, 0, true);
#endif
#यदि ALWAYS_DUMP_FRAMESET
	यदि (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (शून्य *) &html);
	यदि ((exc != DOM_NO_ERR) || (html == रिक्त)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, रिक्त);
		content_set_error(&c->base);
		फर्काऊ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	यदि (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, रिक्त);
		content_set_error(&c->base);
		dom_गाँठो_unref(html);
		फर्काऊ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = रिक्त;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_गाँठो_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	विकल्प (content__get_status(&html->base)) {
	पक्ष CONTENT_STATUS_READY:
		यदि (html->base.active == 0) {
			content_set_done(&html->base);
			फर्काऊ NSERROR_OK;
		}
		रोक;
	पक्ष CONTENT_STATUS_DONE:
		/* fallthrough */
	पक्ष CONTENT_STATUS_LOADING:
		फर्काऊ NSERROR_OK;
	स्वभाव:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		रोक;
	}
	फर्काऊ NSERROR_UNKNOWN;
}


स्थिर शून्य html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	अचिन्हित f_size;
	अचिन्हित f_min;
	अचिन्हित w;
	अचिन्हित h;
	सङ्घ content_सन्देश_data सन्देश_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_सन्देश_GETDIMS, &सन्देश_data);


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
शून्य html_finish_conversion(html_content *htmlc)
{
	सङ्घ content_सन्देश_data सन्देश_data;
	dom_exception exc; /* returned by libdom functions */
	dom_गाँठो *html;
	nserror error;

	/* Bail out if we've been aborted */
	यदि (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, रिक्त);
		content_set_error(&htmlc->base);
		फर्काऊ;
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
	यदि (htmlc->select_ctx != रिक्त) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		फर्काऊ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	यदि (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, रिक्त);
		content_set_error(&htmlc->base);
		फर्काऊ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	यदि (htmlc->jsthread != रिक्त) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, रिक्त);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	सन्देश_data.explicit_status_text = रिक्त;
	content_broadcast(&htmlc->base, CONTENT_सन्देश_STATUS, &सन्देश_data);

	exc = dom_document_get_document_element(htmlc->document, (शून्य *) &html);
	यदि ((exc != DOM_NO_ERR) || (html == रिक्त)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, रिक्त);
		content_set_error(&htmlc->base);
		फर्काऊ;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	यदि (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_गाँठो_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, रिक्त);
		content_set_error(&htmlc->base);
		फर्काऊ;
	}

	dom_गाँठो_unref(html);
}


स्थिर शून्य
html_document_user_data_handler(dom_गाँठो_operation operation,
				dom_string *साँचो, शून्य *data,
				संरचना dom_गाँठो *src,
				संरचना dom_गाँठो *dst)
{
	यदि (dom_string_isequal(corestring_dom_ns_साँचो_html_content_data,
			       साँचो) == false || data == रिक्त) {
		फर्काऊ;
	}

	विकल्प (operation) {
	पक्ष DOM_गाँठो_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		रोक;
	पक्ष DOM_गाँठो_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		रोक;
	पक्ष DOM_गाँठो_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		रोक;
	पक्ष DOM_गाँठो_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		रोक;
	पक्ष DOM_गाँठो_DELETED:
		/* This is the only path I expect */
		रोक;
	स्वभाव:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		प्रतिज्ञा(0);
	}
}


स्थिर nserror
html_create_html_data(html_content *c, अचर http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	शून्य *old_गाँठो_data;
	अचर वर्ण *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = रिक्त;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = रिक्त;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = रिक्त;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = रिक्त;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = रिक्त;
	c->bctx = रिक्त;
	c->layout = रिक्त;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_गन्ती = 0;
	c->stylesheets = रिक्त;
	c->select_ctx = रिक्त;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = रिक्त;
	c->अङ्क_objects = 0;
	c->object_list = रिक्त;
	c->forms = रिक्त;
	c->imagemaps = रिक्त;
	c->bw = रिक्त;
	c->frameset = रिक्त;
	c->iframe = रिक्त;
	c->page = रिक्त;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_गन्ती = 0;
	c->scripts = रिक्त;
	c->jsthread = रिक्त;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	यदि (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		फर्काऊ NSERROR_NOMEM;
	}

	यदि (lwc_intern_string(prefer_color_mode, म_माप(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = रिक्त;
		फर्काऊ NSERROR_NOMEM;
	}

	c->sel = selection_create((संरचना content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	यदि (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		यदि (c->encoding == रिक्त) {
			lwc_string_unref(c->universal);
			c->universal = रिक्त;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = रिक्त;
			फर्काऊ NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.सन्देश = रिक्त;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	यदि ((error != DOM_HUBBUB_OK) && (c->encoding != रिक्त)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		मुक्त(c->encoding);
		c->encoding = रिक्त;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	यदि (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = रिक्त;

		lwc_string_unref(c->universal);
		c->universal = रिक्त;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = रिक्त;

		फर्काऊ libdom_hubbub_error_to_nserror(error);
	}

	err = dom_गाँठो_set_user_data(c->document,
				     corestring_dom_ns_साँचो_html_content_data,
				     c, html_document_user_data_handler,
				     (शून्य *) &old_गाँठो_data);
	यदि (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = रिक्त;
		nsurl_unref(c->base_url);
		c->base_url = रिक्त;

		lwc_string_unref(c->universal);
		c->universal = रिक्त;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = रिक्त;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		फर्काऊ NSERROR_DOM;
	}

	प्रतिज्ञा(old_गाँठो_data == रिक्त);

	फर्काऊ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

स्थिर nserror
html_create(अचर content_handler *handler,
	    lwc_string *imime_type,
	    अचर http_parameter *params,
	    llcache_handle *llcache,
	    अचर वर्ण *fallback_charset,
	    bool quirks,
	    संरचना content **c)
{
	html_content *html;
	nserror error;

	html = सुस्मृति(1, परिमाण(html_content));
	यदि (html == रिक्त)
		फर्काऊ NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(html);
		फर्काऊ error;
	}

	error = html_create_html_data(html, params);
	यदि (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, रिक्त);
		मुक्त(html);
		फर्काऊ error;
	}

	error = html_css_new_stylesheets(html);
	यदि (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, रिक्त);
		मुक्त(html);
		फर्काऊ error;
	}

	*c = (संरचना content *) html;

	फर्काऊ NSERROR_OK;
}



स्थिर nserror
html_process_encoding_change(संरचना content *c,
			     अचर वर्ण *data,
			     अचिन्हित पूर्णाङ्क size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	अचर वर्ण *encoding;
	अचर uint8_t *source_data;
	माप_प्रकार source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	यदि (encoding == रिक्त) {
		फर्काऊ NSERROR_NOMEM;
	}

	यदि (html->encoding != रिक्त) {
		मुक्त(html->encoding);
		html->encoding = रिक्त;
	}

	html->encoding = strdup(encoding);
	यदि (html->encoding == रिक्त) {
		फर्काऊ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = रिक्त;

	यदि (html->document != रिक्त) {
		dom_गाँठो_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.सन्देश = रिक्त;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	यदि (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		मुक्त(html->encoding);
		html->encoding = strdup("Windows-1252");
		यदि (html->encoding == रिक्त) {
			फर्काऊ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		यदि (error != DOM_HUBBUB_OK) {
			फर्काऊ libdom_hubbub_error_to_nserror(error);
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

	फर्काऊ libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

स्थिर bool
html_process_data(संरचना content *c, अचर वर्ण *data, अचिन्हित पूर्णाङ्क size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (अचर uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	यदि (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	यदि (err != NSERROR_OK) {
		content_broadcast_error(c, err, रिक्त);
		फर्काऊ false;
	}

	फर्काऊ true;
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

स्थिर bool html_convert(संरचना content *c)
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
	यदि (exc == DOM_NO_ERR) {
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
	यदि (html_can_begin_conversion(htmlc)) {
		फर्काऊ html_begin_conversion(htmlc);
	}
	फर्काऊ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	अचिन्हित पूर्णाङ्क i;

	/* Cannot begin conversion if we're still fetching stuff */
	यदि (htmlc->base.active != 0)
		फर्काऊ false;

	लागि (i = 0; i != htmlc->stylesheet_गन्ती; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		यदि (htmlc->stylesheets[i].modified)
			फर्काऊ false;
	}

	/* All is good, begin */
	फर्काऊ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_गाँठो *html;
	nserror ns_error;
	संरचना form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *गाँठो_name = रिक्त;
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
	यदि (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		यदि (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			फर्काऊ true;
		}
		यदि (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						रिक्त);

			फर्काऊ false;
		}
		htmlc->parse_completed = true;
	}

	यदि (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		फर्काऊ true;
	}

	/* Give up processing if we've been aborted */
	यदि (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, रिक्त);
		फर्काऊ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	यदि (htmlc->encoding == रिक्त) {
		अचर वर्ण *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		यदि (encoding == रिक्त) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						रिक्त);
			फर्काऊ false;
		}

		htmlc->encoding = strdup(encoding);
		यदि (htmlc->encoding == रिक्त) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						रिक्त);
			फर्काऊ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (शून्य *) &html);
	यदि ((exc != DOM_NO_ERR) || (html == रिक्त)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, रिक्त);
		फर्काऊ false;
	}

	exc = dom_गाँठो_get_गाँठो_name(html, &गाँठो_name);
	यदि ((exc != DOM_NO_ERR) ||
	    (गाँठो_name == रिक्त) ||
	    (!dom_string_caseless_lwc_isequal(गाँठो_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, रिक्त);
		dom_गाँठो_unref(html);
		फर्काऊ false;
	}
	dom_string_unref(गाँठो_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	लागि (f = htmlc->forms; f != रिक्त; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		यदि (f->action == रिक्त || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} अन्यथा {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		यदि (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, रिक्त);

			dom_गाँठो_unref(html);
			फर्काऊ false;
		}

		मुक्त(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		यदि (f->action == रिक्त) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						रिक्त);

			dom_गाँठो_unref(html);
			फर्काऊ false;
		}

		/* Ensure each form has a document encoding */
		यदि (f->document_charset == रिक्त) {
			f->document_charset = strdup(htmlc->encoding);
			यदि (f->document_charset == रिक्त) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							रिक्त);
				dom_गाँठो_unref(html);
				फर्काऊ false;
			}
		}
	}

	dom_गाँठो_unref(html);

	यदि (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	फर्काऊ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

स्थिर शून्य html_stop(संरचना content *c)
{
	html_content *htmlc = (html_content *) c;

	विकल्प (c->status) {
	पक्ष CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		यदि (htmlc->jsthread != रिक्त) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		रोक;

	पक्ष CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		यदि (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		रोक;

	पक्ष CONTENT_STATUS_DONE:
		/* Nothing to do */
		रोक;

	स्वभाव:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		प्रतिज्ञा(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

स्थिर शून्य html_reformat(संरचना content *c, पूर्णाङ्क width, पूर्णाङ्क height)
{
	html_content *htmlc = (html_content *) c;
	संरचना box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.जरा_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[बायाँ_2] + layout->width +
		layout->padding[दायाँ_2] + layout->border[दायाँ_2].width +
		layout->margin[दायाँ_2];
	c->height = layout->y + layout->padding[शीर्ष_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	यदि (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	यदि (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	यदि (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

शून्य html_redraw_a_box(hlcache_handle *h, संरचना box *box)
{
	पूर्णाङ्क x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2],
			box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

शून्य html__redraw_a_box(संरचना html_content *html, संरचना box *box)
{
	पूर्णाङ्क x, y;

	box_coords(box, &x, &y);

	content__request_redraw((संरचना content *)html, x, y,
			box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2],
			box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM]);
}

स्थिर शून्य html_destroy_frameset(संरचना content_html_frames *frameset)
{
	पूर्णाङ्क i;

	यदि (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = रिक्त;
	}
	यदि (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = रिक्त;
	}
	यदि (frameset->children) {
		लागि (i = 0; i < (frameset->rows * frameset->cols); i++) {
			यदि (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = रिक्त;
			}
			यदि (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = रिक्त;
			}
			यदि (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = रिक्त;
	}
}

स्थिर शून्य html_destroy_iframe(संरचना content_html_iframe *iframe)
{
	संरचना content_html_iframe *next;
	next = iframe;
	जबसम्म ((iframe = next) != रिक्त) {
		next = iframe->next;
		यदि (iframe->name)
			talloc_free(iframe->name);
		यदि (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = रिक्त;
		}
		talloc_free(iframe);
	}
}


स्थिर शून्य html_free_layout(html_content *htmlc)
{
	यदि (htmlc->bctx != रिक्त) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

स्थिर शून्य html_destroy(संरचना content *c)
{
	html_content *html = (html_content *) c;
	संरचना form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	यदि (html->box_conversion_context != रिक्त) {
		यदि (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	लागि (f = html->forms; f != रिक्त; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	यदि (c->refresh)
		nsurl_unref(c->refresh);

	यदि (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	यदि (html->jsthread != रिक्त) {
		js_destroythread(html->jsthread);
		html->jsthread = रिक्त;
	}

	यदि (html->parser != रिक्त) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = रिक्त;
	}

	यदि (html->document != रिक्त) {
		dom_गाँठो_unref(html->document);
		html->document = रिक्त;
	}

	यदि (html->title != रिक्त) {
		dom_गाँठो_unref(html->title);
		html->title = रिक्त;
	}

	/* Free encoding */
	यदि (html->encoding != रिक्त) {
		मुक्त(html->encoding);
		html->encoding = रिक्त;
	}

	/* Free base target */
	यदि (html->base_target != रिक्त) {
		मुक्त(html->base_target);
		html->base_target = रिक्त;
	}

	/* Free frameset */
	यदि (html->frameset != रिक्त) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = रिक्त;
	}

	/* Free iframes */
	यदि (html->iframe != रिक्त) {
		html_destroy_iframe(html->iframe);
		html->iframe = रिक्त;
	}

	/* Destroy selection context */
	यदि (html->select_ctx != रिक्त) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = रिक्त;
	}

	lwc_string_unref(html->universal);
	html->universal = रिक्त;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = रिक्त;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


स्थिर nserror html_clone(अचर संरचना content *old, संरचना content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	प्रतिज्ञा(0 && "html_clone should never be called");

	फर्काऊ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

स्थिर nserror
html_open(संरचना content *c,
	  संरचना browser_window *bw,
	  संरचना content *page,
	  संरचना object_params *params)
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

	फर्काऊ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

स्थिर nserror html_close(संरचना content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = रिक्त;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	यदि (htmlc->jsthread != रिक्त) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	फर्काऊ ret;
}


/**
 * Return an HTML content's selection context
 */

स्थिर शून्य html_clear_selection(संरचना content *c)
{
	html_content *html = (html_content *) c;

	विकल्प (html->selection_type) {
	पक्ष HTML_SELECTION_NONE:
		/* Nothing to do */
		प्रतिज्ञा(html->selection_owner.none == true);
		रोक;
	पक्ष HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		रोक;
	पक्ष HTML_SELECTION_SELF:
		प्रतिज्ञा(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		रोक;
	पक्ष HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		रोक;
	स्वभाव:
		रोक;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

स्थिर वर्ण *html_get_selection(संरचना content *c)
{
	html_content *html = (html_content *) c;

	विकल्प (html->selection_type) {
	पक्ष HTML_SELECTION_TEXTAREA:
		फर्काऊ textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	पक्ष HTML_SELECTION_SELF:
		प्रतिज्ञा(html->selection_owner.none == false);
		फर्काऊ selection_get_copy(html->sel);
	पक्ष HTML_SELECTION_CONTENT:
		फर्काऊ content_get_selection(
				html->selection_owner.content->object);
	पक्ष HTML_SELECTION_NONE:
		/* Nothing to do */
		प्रतिज्ञा(html->selection_owner.none == true);
		रोक;
	स्वभाव:
		रोक;
	}

	फर्काऊ रिक्त;
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
स्थिर nserror
html_get_contextual_content(संरचना content *c, पूर्णाङ्क x, पूर्णाङ्क y,
			    संरचना browser_window_features *data)
{
	html_content *html = (html_content *) c;

	संरचना box *box = html->layout;
	संरचना box *next;
	पूर्णाङ्क box_x = 0, box_y = 0;

	जबसम्म ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != रिक्त) {
		box = next;

		/* hidden boxes are ignored */
		यदि ((box->style != रिक्त) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			जारी;
		}

		यदि (box->iframe) {
			भिन्न scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		यदि (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		यदि (box->object)
			data->object = box->object;

		यदि (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		यदि (box->usemap) {
			अचर वर्ण *target = रिक्त;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			यदि (url != रिक्त)
				data->link = url;
		}
		यदि (box->gadget) {
			विकल्प (box->gadget->type) {
			पक्ष GADGET_TEXTBOX:
			पक्ष GADGET_TEXTAREA:
			पक्ष GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				रोक;

			पक्ष GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				रोक;

			स्वभाव:
				data->form_features = CTX_FORM_NONE;
				रोक;
			}
		}
	}
	फर्काऊ NSERROR_OK;
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
स्थिर bool
html_scroll_at_point(संरचना content *c, पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क scrx, पूर्णाङ्क scry)
{
	html_content *html = (html_content *) c;

	संरचना box *box = html->layout;
	संरचना box *next;
	पूर्णाङ्क box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	जबसम्म ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != रिक्त) {
		box = next;

		यदि (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			जारी;

		/* Pass into iframe */
		यदि (box->iframe) {
			भिन्न scale = browser_window_get_scale(box->iframe);

			यदि (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				फर्काऊ true;
		}

		/* Pass into textarea widget */
		यदि (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			फर्काऊ true;

		/* Pass into object */
		यदि (box->object != रिक्त && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			फर्काऊ true;

		/* Handle box scrollbars */
		यदि (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		यदि (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		यदि (handled_scroll == true)
			फर्काऊ true;
	}

	फर्काऊ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
स्थिर शून्य html__dom_user_data_handler(dom_गाँठो_operation operation,
		dom_string *साँचो, शून्य *_data, संरचना dom_गाँठो *src,
		संरचना dom_गाँठो *dst)
{
	वर्ण *oldfile;
	वर्ण *data = (वर्ण *)_data;

	यदि (!dom_string_isequal(corestring_dom_ns_साँचो_file_name_गाँठो_data,
				साँचो) || data == रिक्त) {
		फर्काऊ;
	}

	विकल्प (operation) {
	पक्ष DOM_गाँठो_CLONED:
		यदि (dom_गाँठो_set_user_data(dst,
					   corestring_dom_ns_साँचो_file_name_गाँठो_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			यदि (oldfile != रिक्त)
				मुक्त(oldfile);
		}
		रोक;

	पक्ष DOM_गाँठो_RENAMED:
	पक्ष DOM_गाँठो_IMPORTED:
	पक्ष DOM_गाँठो_ADOPTED:
		रोक;

	पक्ष DOM_गाँठो_DELETED:
		मुक्त(data);
		रोक;
	स्वभाव:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		प्रतिज्ञा(0);
	}
}

स्थिर शून्य html__set_file_gadget_filename(संरचना content *c,
	संरचना form_control *gadget, अचर वर्ण *fn)
{
	nserror ret;
	वर्ण *utf8_fn, *oldfile = रिक्त;
	html_content *html = (html_content *)c;
	संरचना box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	यदि (ret != NSERROR_OK) {
		प्रतिज्ञा(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		फर्काऊ;
	}

	form_gadget_update_मान(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	यदि (dom_गाँठो_set_user_data((dom_गाँठो *)file_box->gadget->गाँठो,
				   corestring_dom_ns_साँचो_file_name_गाँठो_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		यदि (oldfile != रिक्त)
			मुक्त(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

शून्य html_set_file_gadget_filename(संरचना hlcache_handle *hl,
	संरचना form_control *gadget, अचर वर्ण *fn)
{
	फर्काऊ html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
स्थिर bool html_drop_file_at_point(संरचना content *c, पूर्णाङ्क x, पूर्णाङ्क y, वर्ण *file)
{
	html_content *html = (html_content *) c;

	संरचना box *box = html->layout;
	संरचना box *next;
	संरचना box *file_box = रिक्त;
	संरचना box *text_box = रिक्त;
	पूर्णाङ्क box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	जबसम्म ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != रिक्त) {
		box = next;

		यदि (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			जारी;

		यदि (box->iframe) {
			भिन्न scale = browser_window_get_scale(box->iframe);
			फर्काऊ browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		यदि (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			फर्काऊ true;

		यदि (box->gadget) {
			विकल्प (box->gadget->type) {
				पक्ष GADGET_FILE:
					file_box = box;
				रोक;

				पक्ष GADGET_TEXTBOX:
				पक्ष GADGET_TEXTAREA:
				पक्ष GADGET_PASSWORD:
					text_box = box;
					रोक;

				स्वभाव:	/* appease compiler */
					रोक;
			}
		}
	}

	यदि (!file_box && !text_box)
		/* No box capable of handling drop */
		फर्काऊ false;

	यदि (file == रिक्त)
		/* There is a box capable of handling drop here */
		फर्काऊ true;

	/* Handle the drop */
	यदि (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} अन्यथा {
		/* File dropped on text input */

		माप_प्रकार file_len;
		खाता *fp = रिक्त;
		वर्ण *buffer;
		वर्ण *utf8_buff;
		nserror ret;
		अचिन्हित पूर्णाङ्क size;
		पूर्णाङ्क bx, by;

		/* Open file */
		fp = ख_खोलो(file, "rb");
		यदि (fp == रिक्त) {
			/* Couldn't open file, but drop was for us */
			फर्काऊ true;
		}

		/* Get filesize */
		ख_जाओ(fp, 0, अंत_से);
		file_len = ख_बताओ(fp);
		ख_जाओ(fp, 0, शुरू_से);

		यदि ((दीर्घ)file_len == -1) {
			/* unable to get file length, but drop was for us */
			ख_बंद(fp);
			फर्काऊ true;
		}

		/* Allocate buffer for file data */
		buffer = दो_स्मृति(file_len + 1);
		यदि (buffer == रिक्त) {
			/* No memory, but drop was for us */
			ख_बंद(fp);
			फर्काऊ true;
		}

		/* Stick file into buffer */
		यदि (file_len != ख_पढ़ो(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			मुक्त(buffer);
			ख_बंद(fp);
			फर्काऊ true;
		}

		/* Done with file */
		ख_बंद(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		यदि (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			प्रतिज्ञा(ret != NSERROR_BAD_ENCODING);
			मुक्त(buffer);
			फर्काऊ true;
		}

		/* Done with buffer */
		मुक्त(buffer);

		/* Get new length */
		size = म_माप(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		मुक्त(utf8_buff);
	}

	फर्काऊ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
स्थिर nserror
html_debug(संरचना content *c, गणना content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	फर्काऊ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
स्थिर nserror
html_debug_dump(संरचना content *c, खाता *f, गणना content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_गाँठो *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	प्रतिज्ञा(htmlc != रिक्त);

	यदि (op == CONTENT_DEBUG_RENDER) {
		प्रतिज्ञा(htmlc->layout != रिक्त);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} अन्यथा {
		यदि (htmlc->document == रिक्त) {
			NSLOG(netsurf, INFO, "No document to dump");
			फर्काऊ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (शून्य *) &html);
		यदि ((exc != DOM_NO_ERR) || (html == रिक्त)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			फर्काऊ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_गाँठो_unref(html);
	}

	फर्काऊ ret;
}


#यदि ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

स्थिर शून्य
html_dump_frameset(संरचना content_html_frames *frame, अचिन्हित पूर्णाङ्क depth)
{
	अचिन्हित पूर्णाङ्क i;
	पूर्णाङ्क row, col, index;
	अचर वर्ण *unit[] = {"px", "%", "*"};
	अचर वर्ण *scrolling[] = {"auto", "yes", "no"};

	प्रतिज्ञा(frame);

	ख_लिखो(मानक_त्रुटि, "%p ", frame);

	ख_लिखो(मानक_त्रुटि, "(%i %i) ", frame->rows, frame->cols);

	ख_लिखो(मानक_त्रुटि, "w%g%s ", frame->width.मान, unit[frame->width.unit]);
	ख_लिखो(मानक_त्रुटि, "h%g%s ", frame->height.मान,unit[frame->height.unit]);
	ख_लिखो(मानक_त्रुटि, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	यदि (frame->name)
		ख_लिखो(मानक_त्रुटि, "'%s' ", frame->name);
	यदि (frame->url)
		ख_लिखो(मानक_त्रुटि, "<%s> ", frame->url);

	यदि (frame->no_resize)
		ख_लिखो(मानक_त्रुटि, "noresize ");
	ख_लिखो(मानक_त्रुटि, "(scrolling %s) ", scrolling[frame->scrolling]);
	यदि (frame->border)
		ख_लिखो(मानक_त्रुटि, "border %x ",
				(अचिन्हित पूर्णाङ्क) frame->border_colour);

	ख_लिखो(मानक_त्रुटि, "\n");

	यदि (frame->children) {
		लागि (row = 0; row != frame->rows; row++) {
			लागि (col = 0; col != frame->cols; col++) {
				लागि (i = 0; i != depth; i++)
					ख_लिखो(मानक_त्रुटि, "  ");
				ख_लिखो(मानक_त्रुटि, "(%i %i): ", row, col);
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

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->document;
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
संरचना box *html_get_box_रूख(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
स्थिर अचर वर्ण *html_encoding(अचर संरचना content *c, गणना content_encoding_type op)
{
	html_content *html = (html_content *) c;
	स्थिर वर्ण enc_token[10] = "Encoding0";

	प्रतिज्ञा(html != रिक्त);

	यदि (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		फर्काऊ messages_get(enc_token);
	}

	फर्काऊ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
संरचना content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
संरचना content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->iframe;
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

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
अचर वर्ण *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	फर्काऊ c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, पूर्णाङ्क *x, पूर्णाङ्क *y)
{
	संरचना box *pos;
	संरचना box *layout;

	यदि (content_get_type(h) != CONTENT_HTML)
		फर्काऊ false;

	layout = html_get_box_रूख(h);

	यदि ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		फर्काऊ true;
	}
	फर्काऊ false;
}

bool html_exec(संरचना content *c, अचर वर्ण *src, माप_प्रकार srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_गाँठो;
	dom_string *dom_src;
	dom_text *text_गाँठो;
	dom_गाँठो *spare_गाँठो;
	dom_html_script_element *script_गाँठो;

	यदि (htmlc->document == रिक्त) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		गच्छ out_no_string;
	}

	err = dom_string_create((अचर uint8_t *)src, srclen, &dom_src);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		गच्छ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_गाँठो);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		गच्छ out_no_body;
	}

	err = dom_document_create_text_गाँठो(htmlc->document, dom_src, &text_गाँठो);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		गच्छ out_no_text_गाँठो;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_गाँठो);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		गच्छ out_no_script_गाँठो;
	}

	err = dom_गाँठो_append_child(script_गाँठो, text_गाँठो, &spare_गाँठो);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		गच्छ out_unparented;
	}
	dom_गाँठो_unref(spare_गाँठो); /* We do not need the spare ref at all */

	err = dom_गाँठो_append_child(body_गाँठो, script_गाँठो, &spare_गाँठो);
	यदि (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		गच्छ out_unparented;
	}
	dom_गाँठो_unref(spare_गाँठो); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_गाँठो_get_parent_गाँठो(script_गाँठो, &spare_गाँठो);
	यदि (err == DOM_NO_ERR && spare_गाँठो != रिक्त) {
		dom_गाँठो *second_spare;
		err = dom_गाँठो_remove_child(spare_गाँठो, script_गाँठो, &second_spare);
		यदि (err == DOM_NO_ERR) {
			dom_गाँठो_unref(second_spare);
		}
		dom_गाँठो_unref(spare_गाँठो);
	}

out_unparented:
	dom_गाँठो_unref(script_गाँठो);
out_no_script_गाँठो:
	dom_गाँठो_unref(text_गाँठो);
out_no_text_गाँठो:
	dom_गाँठो_unref(body_गाँठो);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	फर्काऊ result;
}

/* See \ref content_saw_insecure_objects */
स्थिर bool
html_saw_insecure_objects(संरचना content *c)
{
	html_content *htmlc = (html_content *)c;
	संरचना content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	जबसम्म (obj != रिक्त) {
		यदि (obj->content != रिक्त) {
			यदि (content_saw_insecure_objects(obj->content))
				फर्काऊ true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	यदि (html_saw_insecure_scripts(htmlc)) {
		फर्काऊ true;
	}

	/* Now check stylesheets */
	यदि (html_css_saw_insecure_stylesheets(htmlc)) {
		फर्काऊ true;
	}

	फर्काऊ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
स्थिर content_type html_content_type(शून्य)
{
	फर्काऊ CONTENT_HTML;
}


स्थिर शून्य html_fini(शून्य)
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
स्थिर nserror
find_occurrences_html_box(अचर वर्ण *pattern,
			  पूर्णाङ्क p_len,
			  संरचना box *cur,
			  bool case_sens,
			  संरचना textsearch_context *context)
{
	संरचना box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	यदि (!cur->object && cur->text) {
		अचर वर्ण *text = cur->text;
		अचिन्हित length = cur->length;

		जबसम्म (length > 0) {
			अचिन्हित match_length;
			अचिन्हित match_offset;
			अचर वर्ण *new_text;
			अचर वर्ण *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			यदि (!pos)
				रोक;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			यदि (res != NSERROR_OK) {
				फर्काऊ res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	लागि (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		यदि (res != NSERROR_OK) {
			फर्काऊ res;
		}
	}

	फर्काऊ res;
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
स्थिर nserror
html_textsearch_find(संरचना content *c,
		     संरचना textsearch_context *context,
		     अचर वर्ण *pattern,
		     पूर्णाङ्क p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	यदि (html->layout == रिक्त) {
		फर्काऊ NSERROR_INVALID;
	}

	फर्काऊ find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


स्थिर nserror
html_textsearch_bounds(संरचना content *c,
		       अचिन्हित start_idx,
		       अचिन्हित end_idx,
		       संरचना box *start_box,
		       संरचना box *end_box,
		       संरचना rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	फर्काऊ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
स्थिर अचर content_handler html_content_handler = {
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
	.खोलो = html_open,
	.बंद = html_close,
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
nserror html_init(शून्य)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	यदि (error != NSERROR_OK)
		गच्छ error;

	लागि (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		यदि (error != NSERROR_OK)
			गच्छ error;
	}

	फर्काऊ NSERROR_OK;

error:
	html_fini();

	फर्काऊ error;
}
