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

#समावेश <assert.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <stdlib.h>
#समावेश <nsutils/time.h>

#समावेश "utils/utils.h"
#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/http.h"
#समावेश "utils/libdom.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/talloc.h"
#समावेश "utils/utf8.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/string.h"
#समावेश "utils/ascii.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/utf8.h"
#समावेश "netsurf/keypress.h"
#समावेश "netsurf/layout.h"
#समावेश "netsurf/misc.h"
#समावेश "content/hlcache.h"
#समावेश "content/content_factory.h"
#समावेश "content/textsearch.h"
#समावेश "desktop/selection.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/textarea.h"
#समावेश "netsurf/bitmap.h"
#समावेश "javascript/js.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/dom_event.h"
#समावेश "html/css.h"
#समावेश "html/object.h"
#समावेश "html/html_save.h"
#समावेश "html/interaction.h"
#समावेश "html/box.h"
#समावेश "html/box_construct.h"
#समावेश "html/box_inspect.h"
#समावेश "html/form_internal.h"
#समावेश "html/imagemap.h"
#समावेश "html/layout.h"
#समावेश "html/textselection.h"

#घोषणा CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#घोषणा ALWAYS_DUMP_FRAMESET 0
#घोषणा ALWAYS_DUMP_BOX 0

अटल स्थिर अक्षर *html_types[] = {
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
अटल bool fire_dom_event(dom_event *event, dom_गाँठ *target)
{
	dom_exception exc;
	bool परिणाम;

	exc = dom_event_target_dispatch_event(target, event, &परिणाम);
	अगर (exc != DOM_NO_ERR) {
		वापस false;
	}

	वापस परिणाम;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_गाँठ *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool परिणाम;

	exc = dom_event_create(&evt);
	अगर (exc != DOM_NO_ERR) वापस false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	अगर (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		वापस false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (पूर्णांक)dom_string_लंबाई(type), dom_string_तथ्य(type), target);
	परिणाम = fire_dom_event(evt, target);
	dom_event_unref(evt);
	वापस परिणाम;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_गाँठ *target,
		bool bubbles, bool cancelable, uint32_t कुंजी)
{
	bool is_special = कुंजी <= 0x001F || (0x007F <= कुंजी && कुंजी <= 0x009F);
	dom_string *dom_कुंजी = शून्य;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool परिणाम;

	अगर (is_special) {
		चयन (कुंजी) {
		हाल NS_कुंजी_ESCAPE:
			dom_कुंजी = dom_string_ref(corestring_dom_Escape);
			अवरोध;
		हाल NS_कुंजी_बायाँ:
			dom_कुंजी = dom_string_ref(corestring_dom_Arrow_बायाँ);
			अवरोध;
		हाल NS_कुंजी_दायाँ:
			dom_कुंजी = dom_string_ref(corestring_dom_Arrow_दायाँ);
			अवरोध;
		हाल NS_कुंजी_UP:
			dom_कुंजी = dom_string_ref(corestring_dom_ArrowUp);
			अवरोध;
		हाल NS_कुंजी_DOWN:
			dom_कुंजी = dom_string_ref(corestring_dom_ArrowDown);
			अवरोध;
		हाल NS_कुंजी_PAGE_UP:
			dom_कुंजी = dom_string_ref(corestring_dom_PageUp);
			अवरोध;
		हाल NS_कुंजी_PAGE_DOWN:
			dom_कुंजी = dom_string_ref(corestring_dom_PageDown);
			अवरोध;
		हाल NS_कुंजी_TEXT_START:
			dom_कुंजी = dom_string_ref(corestring_dom_Home);
			अवरोध;
		हाल NS_कुंजी_TEXT_END:
			dom_कुंजी = dom_string_ref(corestring_dom_End);
			अवरोध;
		शेष:
			dom_कुंजी = शून्य;
			अवरोध;
		}
	} अन्यथा {
		अक्षर utf8[6];
		माप_प्रकार लंबाई = utf8_from_ucs4(कुंजी, utf8);
		utf8[लंबाई] = '\0';

		exc = dom_string_create((स्थिर uint8_t *)utf8, म_माप(utf8),
				&dom_कुंजी);
		अगर (exc != DOM_NO_ERR) {
			वापस exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	अगर (exc != DOM_NO_ERR) {
		dom_string_unref(dom_कुंजी);
		वापस false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, शून्य,
			dom_कुंजी, शून्य, DOM_कुंजी_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_कुंजी);
	अगर (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		वापस false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(पूर्णांक)dom_string_लंबाई(type), dom_string_तथ्य(type), target);

	परिणाम = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	वापस परिणाम;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
अटल व्योम html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_गाँठ *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = शून्य;

	/* Clean up and report error if unsuccessful or aborted */
	अगर ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		अगर (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, शून्य);
		} अन्यथा {
			content_broadcast_error(&c->base, NSERROR_STOPPED, शून्य);
		}

		content_set_error(&c->base);
		वापस;
	}


#अगर ALWAYS_DUMP_BOX
	box_dump(मानक_त्रुटि, c->layout->children, 0, true);
#पूर्ण_अगर
#अगर ALWAYS_DUMP_FRAMESET
	अगर (c->frameset)
		html_dump_frameset(c->frameset, 0);
#पूर्ण_अगर

	exc = dom_document_get_document_element(c->document, (व्योम *) &html);
	अगर ((exc != DOM_NO_ERR) || (html == शून्य)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, शून्य);
		content_set_error(&c->base);
		वापस;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	अगर (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, शून्य);
		content_set_error(&c->base);
		dom_गाँठ_unref(html);
		वापस;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = शून्य;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_गाँठ_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	चयन (content__get_status(&html->base)) {
	हाल CONTENT_STATUS_READY:
		अगर (html->base.active == 0) {
			content_set_done(&html->base);
			वापस NSERROR_OK;
		}
		अवरोध;
	हाल CONTENT_STATUS_DONE:
		/* fallthrough */
	हाल CONTENT_STATUS_LOADING:
		वापस NSERROR_OK;
	शेष:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		अवरोध;
	}
	वापस NSERROR_UNKNOWN;
}


अटल व्योम html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	अचिन्हित f_आकार;
	अचिन्हित f_min;
	अचिन्हित w;
	अचिन्हित h;
	जोड़ content_संदेश_तथ्य संदेश_तथ्य = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_संदेश_GETDIMS, &संदेश_तथ्य);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.width  = w;
	htmlc->media.height = h;
	htmlc->unit_len_ctx.viewport_width  = w;
	htmlc->unit_len_ctx.viewport_height = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_आकार = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_आकार)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_आकार)), F_10)), F_72);

	htmlc->unit_len_ctx.font_आकार_default = f_आकार;
	htmlc->unit_len_ctx.font_आकार_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
व्योम html_finish_conversion(html_content *htmlc)
{
	जोड़ content_संदेश_तथ्य संदेश_तथ्य;
	dom_exception exc; /* returned by libdom functions */
	dom_गाँठ *html;
	nserror error;

	/* Bail out if we've been aborted */
	अगर (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, शून्य);
		content_set_error(&htmlc->base);
		वापस;
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
	अगर (htmlc->select_ctx != शून्य) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		वापस;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	अगर (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, शून्य);
		content_set_error(&htmlc->base);
		वापस;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	अगर (htmlc->jsthread != शून्य) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, शून्य);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	संदेश_तथ्य.explicit_status_text = शून्य;
	content_broadcast(&htmlc->base, CONTENT_संदेश_STATUS, &संदेश_तथ्य);

	exc = dom_document_get_document_element(htmlc->document, (व्योम *) &html);
	अगर ((exc != DOM_NO_ERR) || (html == शून्य)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, शून्य);
		content_set_error(&htmlc->base);
		वापस;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	अगर (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_गाँठ_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, शून्य);
		content_set_error(&htmlc->base);
		वापस;
	}

	dom_गाँठ_unref(html);
}


अटल व्योम
html_document_user_तथ्य_handler(dom_गाँठ_operation operation,
				dom_string *कुंजी, व्योम *तथ्य,
				काष्ठा dom_गाँठ *src,
				काष्ठा dom_गाँठ *dst)
{
	अगर (dom_string_isequal(corestring_dom_ns_कुंजी_html_content_तथ्य,
			       कुंजी) == false || तथ्य == शून्य) {
		वापस;
	}

	चयन (operation) {
	हाल DOM_गाँठ_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		अवरोध;
	हाल DOM_गाँठ_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		अवरोध;
	हाल DOM_गाँठ_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		अवरोध;
	हाल DOM_गाँठ_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		अवरोध;
	हाल DOM_गाँठ_DELETED:
		/* This is the only path I expect */
		अवरोध;
	शेष:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		निश्चित(0);
	}
}


अटल nserror
html_create_html_तथ्य(html_content *c, स्थिर http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	व्योम *old_गाँठ_तथ्य;
	स्थिर अक्षर *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = शून्य;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = शून्य;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = शून्य;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = शून्य;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = शून्य;
	c->bctx = शून्य;
	c->layout = शून्य;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_गिनती = 0;
	c->stylesheets = शून्य;
	c->select_ctx = शून्य;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = शून्य;
	c->अंक_objects = 0;
	c->object_list = शून्य;
	c->forms = शून्य;
	c->imagemaps = शून्य;
	c->bw = शून्य;
	c->frameset = शून्य;
	c->iframe = शून्य;
	c->page = शून्य;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_गिनती = 0;
	c->scripts = शून्य;
	c->jsthread = शून्य;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	अगर (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		वापस NSERROR_NOMEM;
	}

	अगर (lwc_intern_string(prefer_color_mode, म_माप(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = शून्य;
		वापस NSERROR_NOMEM;
	}

	c->sel = selection_create((काष्ठा content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	अगर (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_तथ्य(charset));

		lwc_string_unref(charset);

		अगर (c->encoding == शून्य) {
			lwc_string_unref(c->universal);
			c->universal = शून्य;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = शून्य;
			वापस NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.संदेश = शून्य;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	अगर ((error != DOM_HUBBUB_OK) && (c->encoding != शून्य)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		मुक्त(c->encoding);
		c->encoding = शून्य;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	अगर (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = शून्य;

		lwc_string_unref(c->universal);
		c->universal = शून्य;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = शून्य;

		वापस libdom_hubbub_error_to_nserror(error);
	}

	err = dom_गाँठ_set_user_तथ्य(c->document,
				     corestring_dom_ns_कुंजी_html_content_तथ्य,
				     c, html_document_user_तथ्य_handler,
				     (व्योम *) &old_गाँठ_तथ्य);
	अगर (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = शून्य;
		nsurl_unref(c->base_url);
		c->base_url = शून्य;

		lwc_string_unref(c->universal);
		c->universal = शून्य;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = शून्य;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		वापस NSERROR_DOM;
	}

	निश्चित(old_गाँठ_तथ्य == शून्य);

	वापस NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

अटल nserror
html_create(स्थिर content_handler *handler,
	    lwc_string *imime_type,
	    स्थिर http_parameter *params,
	    llcache_handle *llcache,
	    स्थिर अक्षर *fallback_charset,
	    bool quirks,
	    काष्ठा content **c)
{
	html_content *html;
	nserror error;

	html = सुस्मृति(1, माप(html_content));
	अगर (html == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(html);
		वापस error;
	}

	error = html_create_html_तथ्य(html, params);
	अगर (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, शून्य);
		मुक्त(html);
		वापस error;
	}

	error = html_css_new_stylesheets(html);
	अगर (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, शून्य);
		मुक्त(html);
		वापस error;
	}

	*c = (काष्ठा content *) html;

	वापस NSERROR_OK;
}



अटल nserror
html_process_encoding_change(काष्ठा content *c,
			     स्थिर अक्षर *तथ्य,
			     अचिन्हित पूर्णांक आकार)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	स्थिर अक्षर *encoding;
	स्थिर uint8_t *source_तथ्य;
	माप_प्रकार source_आकार;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	अगर (encoding == शून्य) {
		वापस NSERROR_NOMEM;
	}

	अगर (html->encoding != शून्य) {
		मुक्त(html->encoding);
		html->encoding = शून्य;
	}

	html->encoding = strdup(encoding);
	अगर (html->encoding == शून्य) {
		वापस NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = शून्य;

	अगर (html->document != शून्य) {
		dom_गाँठ_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.संदेश = शून्य;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	अगर (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		मुक्त(html->encoding);
		html->encoding = strdup("Windows-1252");
		अगर (html->encoding == शून्य) {
			वापस NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		अगर (error != DOM_HUBBUB_OK) {
			वापस libdom_hubbub_error_to_nserror(error);
		}

	}

	source_तथ्य = content_get_source_तथ्य_2(c, &source_आकार);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      source_तथ्य,
					      source_आकार);

	वापस libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

अटल bool
html_process_तथ्य(काष्ठा content *c, स्थिर अक्षर *तथ्य, अचिन्हित पूर्णांक आकार)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (स्थिर uint8_t *) तथ्य,
					      आकार);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	अगर (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, तथ्य, आकार);
	}

	/* broadcast the error if necessary */
	अगर (err != NSERROR_OK) {
		content_broadcast_error(c, err, शून्य);
		वापस false;
	}

	वापस true;
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

अटल bool html_convert(काष्ठा content *c)
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
	अगर (exc == DOM_NO_ERR) {
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
	अगर (html_can_begin_conversion(htmlc)) {
		वापस html_begin_conversion(htmlc);
	}
	वापस true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	अचिन्हित पूर्णांक i;

	/* Cannot begin conversion if we're still fetching stuff */
	अगर (htmlc->base.active != 0)
		वापस false;

	क्रम (i = 0; i != htmlc->stylesheet_गिनती; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		अगर (htmlc->stylesheets[i].modified)
			वापस false;
	}

	/* All is good, begin */
	वापस true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_गाँठ *html;
	nserror ns_error;
	काष्ठा form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *गाँठ_name = शून्य;
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
	अगर (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		अगर (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			वापस true;
		}
		अगर (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						शून्य);

			वापस false;
		}
		htmlc->parse_completed = true;
	}

	अगर (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		वापस true;
	}

	/* Give up processing if we've been aborted */
	अगर (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, शून्य);
		वापस false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	अगर (htmlc->encoding == शून्य) {
		स्थिर अक्षर *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		अगर (encoding == शून्य) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						शून्य);
			वापस false;
		}

		htmlc->encoding = strdup(encoding);
		अगर (htmlc->encoding == शून्य) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						शून्य);
			वापस false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (व्योम *) &html);
	अगर ((exc != DOM_NO_ERR) || (html == शून्य)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, शून्य);
		वापस false;
	}

	exc = dom_गाँठ_get_गाँठ_name(html, &गाँठ_name);
	अगर ((exc != DOM_NO_ERR) ||
	    (गाँठ_name == शून्य) ||
	    (!dom_string_caseless_lwc_isequal(गाँठ_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, शून्य);
		dom_गाँठ_unref(html);
		वापस false;
	}
	dom_string_unref(गाँठ_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	क्रम (f = htmlc->forms; f != शून्य; f = f->पूर्व) {
		nsurl *action;

		/* Make all actions absolute */
		अगर (f->action == शून्य || f->action[0] == '\0') {
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

		अगर (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, शून्य);

			dom_गाँठ_unref(html);
			वापस false;
		}

		मुक्त(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		अगर (f->action == शून्य) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						शून्य);

			dom_गाँठ_unref(html);
			वापस false;
		}

		/* Ensure each form has a document encoding */
		अगर (f->document_charset == शून्य) {
			f->document_charset = strdup(htmlc->encoding);
			अगर (f->document_charset == शून्य) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							शून्य);
				dom_गाँठ_unref(html);
				वापस false;
			}
		}
	}

	dom_गाँठ_unref(html);

	अगर (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	वापस true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

अटल व्योम html_stop(काष्ठा content *c)
{
	html_content *htmlc = (html_content *) c;

	चयन (c->status) {
	हाल CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		अगर (htmlc->jsthread != शून्य) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		अवरोध;

	हाल CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		अगर (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		अवरोध;

	हाल CONTENT_STATUS_DONE:
		/* Nothing to do */
		अवरोध;

	शेष:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		निश्चित(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

अटल व्योम html_reformat(काष्ठा content *c, पूर्णांक width, पूर्णांक height)
{
	html_content *htmlc = (html_content *) c;
	काष्ठा box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.मूल_style = htmlc->layout->style;

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
	अगर (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	अगर (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	अगर (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

व्योम html_redraw_a_box(hlcache_handle *h, काष्ठा box *box)
{
	पूर्णांक x, y;

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

व्योम html__redraw_a_box(काष्ठा html_content *html, काष्ठा box *box)
{
	पूर्णांक x, y;

	box_coords(box, &x, &y);

	content__request_redraw((काष्ठा content *)html, x, y,
			box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2],
			box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM]);
}

अटल व्योम html_destroy_frameset(काष्ठा content_html_frames *frameset)
{
	पूर्णांक i;

	अगर (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = शून्य;
	}
	अगर (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = शून्य;
	}
	अगर (frameset->children) {
		क्रम (i = 0; i < (frameset->rows * frameset->cols); i++) {
			अगर (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = शून्य;
			}
			अगर (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = शून्य;
			}
			अगर (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = शून्य;
	}
}

अटल व्योम html_destroy_iframe(काष्ठा content_html_iframe *iframe)
{
	काष्ठा content_html_iframe *अग्र;
	अग्र = iframe;
	जबतक ((iframe = अग्र) != शून्य) {
		अग्र = iframe->अग्र;
		अगर (iframe->name)
			talloc_free(iframe->name);
		अगर (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = शून्य;
		}
		talloc_free(iframe);
	}
}


अटल व्योम html_free_layout(html_content *htmlc)
{
	अगर (htmlc->bctx != शून्य) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

अटल व्योम html_destroy(काष्ठा content *c)
{
	html_content *html = (html_content *) c;
	काष्ठा form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	अगर (html->box_conversion_context != शून्य) {
		अगर (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	क्रम (f = html->forms; f != शून्य; f = g) {
		g = f->पूर्व;

		form_free(f);
	}

	imagemap_destroy(html);

	अगर (c->refresh)
		nsurl_unref(c->refresh);

	अगर (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	अगर (html->jsthread != शून्य) {
		js_destroythread(html->jsthread);
		html->jsthread = शून्य;
	}

	अगर (html->parser != शून्य) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = शून्य;
	}

	अगर (html->document != शून्य) {
		dom_गाँठ_unref(html->document);
		html->document = शून्य;
	}

	अगर (html->title != शून्य) {
		dom_गाँठ_unref(html->title);
		html->title = शून्य;
	}

	/* Free encoding */
	अगर (html->encoding != शून्य) {
		मुक्त(html->encoding);
		html->encoding = शून्य;
	}

	/* Free base target */
	अगर (html->base_target != शून्य) {
		मुक्त(html->base_target);
		html->base_target = शून्य;
	}

	/* Free frameset */
	अगर (html->frameset != शून्य) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = शून्य;
	}

	/* Free iframes */
	अगर (html->iframe != शून्य) {
		html_destroy_iframe(html->iframe);
		html->iframe = शून्य;
	}

	/* Destroy selection context */
	अगर (html->select_ctx != शून्य) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = शून्य;
	}

	lwc_string_unref(html->universal);
	html->universal = शून्य;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = शून्य;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


अटल nserror html_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	निश्चित(0 && "html_clone should never be called");

	वापस true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

अटल nserror
html_open(काष्ठा content *c,
	  काष्ठा browser_window *bw,
	  काष्ठा content *page,
	  काष्ठा object_params *params)
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

	वापस NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

अटल nserror html_close(काष्ठा content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = शून्य;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	अगर (htmlc->jsthread != शून्य) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	वापस ret;
}


/**
 * Return an HTML content's selection context
 */

अटल व्योम html_clear_selection(काष्ठा content *c)
{
	html_content *html = (html_content *) c;

	चयन (html->selection_type) {
	हाल HTML_SELECTION_NONE:
		/* Nothing to do */
		निश्चित(html->selection_owner.none == true);
		अवरोध;
	हाल HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->तथ्य.text.ta);
		अवरोध;
	हाल HTML_SELECTION_SELF:
		निश्चित(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		अवरोध;
	हाल HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		अवरोध;
	शेष:
		अवरोध;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

अटल अक्षर *html_get_selection(काष्ठा content *c)
{
	html_content *html = (html_content *) c;

	चयन (html->selection_type) {
	हाल HTML_SELECTION_TEXTAREA:
		वापस textarea_get_selection(html->selection_owner.textarea->
				gadget->तथ्य.text.ta);
	हाल HTML_SELECTION_SELF:
		निश्चित(html->selection_owner.none == false);
		वापस selection_get_copy(html->sel);
	हाल HTML_SELECTION_CONTENT:
		वापस content_get_selection(
				html->selection_owner.content->object);
	हाल HTML_SELECTION_NONE:
		/* Nothing to do */
		निश्चित(html->selection_owner.none == true);
		अवरोध;
	शेष:
		अवरोध;
	}

	वापस शून्य;
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
अटल nserror
html_get_contextual_content(काष्ठा content *c, पूर्णांक x, पूर्णांक y,
			    काष्ठा browser_window_features *तथ्य)
{
	html_content *html = (html_content *) c;

	काष्ठा box *box = html->layout;
	काष्ठा box *अग्र;
	पूर्णांक box_x = 0, box_y = 0;

	जबतक ((अग्र = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != शून्य) {
		box = अग्र;

		/* hidden boxes are ignored */
		अगर ((box->style != शून्य) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			जारी;
		}

		अगर (box->iframe) {
			भग्न scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    तथ्य);
		}

		अगर (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, तथ्य);

		अगर (box->object)
			तथ्य->object = box->object;

		अगर (box->href) {
			तथ्य->link = box->href;
			तथ्य->link_title = box->text;
			तथ्य->link_title_लंबाई = box->लंबाई;
		}

		अगर (box->usemap) {
			स्थिर अक्षर *target = शून्य;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			अगर (url != शून्य)
				तथ्य->link = url;
		}
		अगर (box->gadget) {
			चयन (box->gadget->type) {
			हाल GADGET_TEXTBOX:
			हाल GADGET_TEXTAREA:
			हाल GADGET_PASSWORD:
				तथ्य->form_features = CTX_FORM_TEXT;
				अवरोध;

			हाल GADGET_FILE:
				तथ्य->form_features = CTX_FORM_FILE;
				अवरोध;

			शेष:
				तथ्य->form_features = CTX_FORM_NONE;
				अवरोध;
			}
		}
	}
	वापस NSERROR_OK;
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
अटल bool
html_scroll_at_point(काष्ठा content *c, पूर्णांक x, पूर्णांक y, पूर्णांक scrx, पूर्णांक scry)
{
	html_content *html = (html_content *) c;

	काष्ठा box *box = html->layout;
	काष्ठा box *अग्र;
	पूर्णांक box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	जबतक ((अग्र = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != शून्य) {
		box = अग्र;

		अगर (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			जारी;

		/* Pass into iframe */
		अगर (box->iframe) {
			भग्न scale = browser_window_get_scale(box->iframe);

			अगर (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				वापस true;
		}

		/* Pass into textarea widget */
		अगर (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->तथ्य.text.ta,
						scrx, scry) == true)
			वापस true;

		/* Pass into object */
		अगर (box->object != शून्य && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			वापस true;

		/* Handle box scrollbars */
		अगर (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		अगर (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		अगर (handled_scroll == true)
			वापस true;
	}

	वापस false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
अटल व्योम html_dom_user_तथ्य_handler(dom_गाँठ_operation operation,
		dom_string *कुंजी, व्योम *तथ्य_2, काष्ठा dom_गाँठ *src,
		काष्ठा dom_गाँठ *dst)
{
	अक्षर *oldfile;
	अक्षर *तथ्य = (अक्षर *)तथ्य_2;

	अगर (!dom_string_isequal(corestring_dom_ns_कुंजी_file_name_गाँठ_तथ्य,
				कुंजी) || तथ्य == शून्य) {
		वापस;
	}

	चयन (operation) {
	हाल DOM_गाँठ_CLONED:
		अगर (dom_गाँठ_set_user_तथ्य(dst,
					   corestring_dom_ns_कुंजी_file_name_गाँठ_तथ्य,
					   strdup(तथ्य), html_dom_user_तथ्य_handler,
					   &oldfile) == DOM_NO_ERR) {
			अगर (oldfile != शून्य)
				मुक्त(oldfile);
		}
		अवरोध;

	हाल DOM_गाँठ_RENAMED:
	हाल DOM_गाँठ_IMPORTED:
	हाल DOM_गाँठ_ADOPTED:
		अवरोध;

	हाल DOM_गाँठ_DELETED:
		मुक्त(तथ्य);
		अवरोध;
	शेष:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		निश्चित(0);
	}
}

अटल व्योम html__set_file_gadget_filename(काष्ठा content *c,
	काष्ठा form_control *gadget, स्थिर अक्षर *fn)
{
	nserror ret;
	अक्षर *utf8_fn, *oldfile = शून्य;
	html_content *html = (html_content *)c;
	काष्ठा box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	अगर (ret != NSERROR_OK) {
		निश्चित(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		वापस;
	}

	form_gadget_update_मूल्य(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	अगर (dom_गाँठ_set_user_तथ्य((dom_गाँठ *)file_box->gadget->गाँठ,
				   corestring_dom_ns_कुंजी_file_name_गाँठ_तथ्य,
				   strdup(fn), html_dom_user_तथ्य_handler,
				   &oldfile) == DOM_NO_ERR) {
		अगर (oldfile != शून्य)
			मुक्त(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

व्योम html_set_file_gadget_filename(काष्ठा hlcache_handle *hl,
	काष्ठा form_control *gadget, स्थिर अक्षर *fn)
{
	वापस html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
अटल bool html_drop_file_at_point(काष्ठा content *c, पूर्णांक x, पूर्णांक y, अक्षर *file)
{
	html_content *html = (html_content *) c;

	काष्ठा box *box = html->layout;
	काष्ठा box *अग्र;
	काष्ठा box *file_box = शून्य;
	काष्ठा box *text_box = शून्य;
	पूर्णांक box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	जबतक ((अग्र = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != शून्य) {
		box = अग्र;

		अगर (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			जारी;

		अगर (box->iframe) {
			भग्न scale = browser_window_get_scale(box->iframe);
			वापस browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		अगर (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			वापस true;

		अगर (box->gadget) {
			चयन (box->gadget->type) {
				हाल GADGET_FILE:
					file_box = box;
				अवरोध;

				हाल GADGET_TEXTBOX:
				हाल GADGET_TEXTAREA:
				हाल GADGET_PASSWORD:
					text_box = box;
					अवरोध;

				शेष:	/* appease compiler */
					अवरोध;
			}
		}
	}

	अगर (!file_box && !text_box)
		/* No box capable of handling drop */
		वापस false;

	अगर (file == शून्य)
		/* There is a box capable of handling drop here */
		वापस true;

	/* Handle the drop */
	अगर (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} अन्यथा {
		/* File dropped on text input */

		माप_प्रकार file_len;
		खाता *fp = शून्य;
		अक्षर *बफर;
		अक्षर *utf8_buff;
		nserror ret;
		अचिन्हित पूर्णांक आकार;
		पूर्णांक bx, by;

		/* Open file */
		fp = ख_खोलो(file, "rb");
		अगर (fp == शून्य) {
			/* Couldn't open file, but drop was for us */
			वापस true;
		}

		/* Get filesize */
		ख_जाओ(fp, 0, अंत_से);
		file_len = ख_बताओ(fp);
		ख_जाओ(fp, 0, शुरू_से);

		अगर ((दीर्घ)file_len == -1) {
			/* unable to get file length, but drop was for us */
			ख_बंद(fp);
			वापस true;
		}

		/* Allocate buffer for file data */
		बफर = दो_स्मृति(file_len + 1);
		अगर (बफर == शून्य) {
			/* No memory, but drop was for us */
			ख_बंद(fp);
			वापस true;
		}

		/* Stick file into buffer */
		अगर (file_len != ख_पढ़ो(बफर, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			मुक्त(बफर);
			ख_बंद(fp);
			वापस true;
		}

		/* Done with file */
		ख_बंद(fp);

		/* Ensure buffer's string termination */
		बफर[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(बफर, file_len, &utf8_buff);
		अगर (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			निश्चित(ret != NSERROR_BAD_ENCODING);
			मुक्त(बफर);
			वापस true;
		}

		/* Done with buffer */
		मुक्त(बफर);

		/* Get new length */
		आकार = म_माप(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->तथ्य.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->तथ्य.text.ta,
				utf8_buff, आकार);

		मुक्त(utf8_buff);
	}

	वापस true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
अटल nserror
html_debug(काष्ठा content *c, क्रमागत content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	वापस NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
अटल nserror
html_debug_dump(काष्ठा content *c, खाता *f, क्रमागत content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_गाँठ *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	निश्चित(htmlc != शून्य);

	अगर (op == CONTENT_DEBUG_RENDER) {
		निश्चित(htmlc->layout != शून्य);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} अन्यथा {
		अगर (htmlc->document == शून्य) {
			NSLOG(netsurf, INFO, "No document to dump");
			वापस NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (व्योम *) &html);
		अगर ((exc != DOM_NO_ERR) || (html == शून्य)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			वापस NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_गाँठ_unref(html);
	}

	वापस ret;
}


#अगर ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

अटल व्योम
html_dump_frameset(काष्ठा content_html_frames *frame, अचिन्हित पूर्णांक depth)
{
	अचिन्हित पूर्णांक i;
	पूर्णांक row, col, index;
	स्थिर अक्षर *unit[] = {"px", "%", "*"};
	स्थिर अक्षर *scrolling[] = {"auto", "yes", "no"};

	निश्चित(frame);

	ख_लिखो(मानक_त्रुटि, "%p ", frame);

	ख_लिखो(मानक_त्रुटि, "(%i %i) ", frame->rows, frame->cols);

	ख_लिखो(मानक_त्रुटि, "w%g%s ", frame->width.मूल्य, unit[frame->width.unit]);
	ख_लिखो(मानक_त्रुटि, "h%g%s ", frame->height.मूल्य,unit[frame->height.unit]);
	ख_लिखो(मानक_त्रुटि, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	अगर (frame->name)
		ख_लिखो(मानक_त्रुटि, "'%s' ", frame->name);
	अगर (frame->url)
		ख_लिखो(मानक_त्रुटि, "<%s> ", frame->url);

	अगर (frame->no_resize)
		ख_लिखो(मानक_त्रुटि, "noresize ");
	ख_लिखो(मानक_त्रुटि, "(scrolling %s) ", scrolling[frame->scrolling]);
	अगर (frame->border)
		ख_लिखो(मानक_त्रुटि, "border %x ",
				(अचिन्हित पूर्णांक) frame->border_colour);

	ख_लिखो(मानक_त्रुटि, "\n");

	अगर (frame->children) {
		क्रम (row = 0; row != frame->rows; row++) {
			क्रम (col = 0; col != frame->cols; col++) {
				क्रम (i = 0; i != depth; i++)
					ख_लिखो(मानक_त्रुटि, "  ");
				ख_लिखो(मानक_त्रुटि, "(%i %i): ", row, col);
				index = (row * frame->cols) + col;
				html_dump_frameset(&frame->children[index],
						depth + 1);
			}
		}
	}
}

#पूर्ण_अगर

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->document;
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
काष्ठा box *html_get_box_वृक्ष(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
अटल स्थिर अक्षर *html_encoding(स्थिर काष्ठा content *c, क्रमागत content_encoding_type op)
{
	html_content *html = (html_content *) c;
	अटल अक्षर enc_token[10] = "Encoding0";

	निश्चित(html != शून्य);

	अगर (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		वापस messages_get(enc_token);
	}

	वापस html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
काष्ठा content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
काष्ठा content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->iframe;
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

	निश्चित(c != शून्य);

	वापस c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
स्थिर अक्षर *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, पूर्णांक *x, पूर्णांक *y)
{
	काष्ठा box *pos;
	काष्ठा box *layout;

	अगर (content_get_type(h) != CONTENT_HTML)
		वापस false;

	layout = html_get_box_वृक्ष(h);

	अगर ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		वापस true;
	}
	वापस false;
}

bool html_exec(काष्ठा content *c, स्थिर अक्षर *src, माप_प्रकार srclen)
{
	html_content *htmlc = (html_content *)c;
	bool परिणाम = false;
	dom_exception err;
	dom_html_body_element *body_गाँठ;
	dom_string *dom_src;
	dom_text *text_गाँठ;
	dom_गाँठ *spare_गाँठ;
	dom_html_script_element *script_गाँठ;

	अगर (htmlc->document == शून्य) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		जाओ out_no_string;
	}

	err = dom_string_create((स्थिर uint8_t *)src, srclen, &dom_src);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		जाओ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_गाँठ);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		जाओ out_no_body;
	}

	err = dom_document_create_text_गाँठ(htmlc->document, dom_src, &text_गाँठ);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		जाओ out_no_text_गाँठ;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_गाँठ);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		जाओ out_no_script_गाँठ;
	}

	err = dom_गाँठ_append_child(script_गाँठ, text_गाँठ, &spare_गाँठ);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		जाओ out_unparented;
	}
	dom_गाँठ_unref(spare_गाँठ); /* We do not need the spare ref at all */

	err = dom_गाँठ_append_child(body_गाँठ, script_गाँठ, &spare_गाँठ);
	अगर (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		जाओ out_unparented;
	}
	dom_गाँठ_unref(spare_गाँठ); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	परिणाम = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_गाँठ_get_parent_गाँठ(script_गाँठ, &spare_गाँठ);
	अगर (err == DOM_NO_ERR && spare_गाँठ != शून्य) {
		dom_गाँठ *second_spare;
		err = dom_गाँठ_remove_child(spare_गाँठ, script_गाँठ, &second_spare);
		अगर (err == DOM_NO_ERR) {
			dom_गाँठ_unref(second_spare);
		}
		dom_गाँठ_unref(spare_गाँठ);
	}

out_unparented:
	dom_गाँठ_unref(script_गाँठ);
out_no_script_गाँठ:
	dom_गाँठ_unref(text_गाँठ);
out_no_text_गाँठ:
	dom_गाँठ_unref(body_गाँठ);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	वापस परिणाम;
}

/* See \ref content_saw_insecure_objects */
अटल bool
html_saw_insecure_objects(काष्ठा content *c)
{
	html_content *htmlc = (html_content *)c;
	काष्ठा content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	जबतक (obj != शून्य) {
		अगर (obj->content != शून्य) {
			अगर (content_saw_insecure_objects(obj->content))
				वापस true;
		}
		obj = obj->अग्र;
	}

	/* Now check the script list */
	अगर (html_saw_insecure_scripts(htmlc)) {
		वापस true;
	}

	/* Now check stylesheets */
	अगर (html_css_saw_insecure_stylesheets(htmlc)) {
		वापस true;
	}

	वापस false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
अटल content_type html_content_type(व्योम)
{
	वापस CONTENT_HTML;
}


अटल व्योम html_fini(व्योम)
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
अटल nserror
find_occurrences_html_box(स्थिर अक्षर *pattern,
			  पूर्णांक p_len,
			  काष्ठा box *cur,
			  bool case_sens,
			  काष्ठा textsearch_context *context)
{
	काष्ठा box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	अगर (!cur->object && cur->text) {
		स्थिर अक्षर *text = cur->text;
		अचिन्हित लंबाई = cur->लंबाई;

		जबतक (लंबाई > 0) {
			अचिन्हित match_लंबाई;
			अचिन्हित match_offset;
			स्थिर अक्षर *new_text;
			स्थिर अक्षर *pos;

			pos = content_textsearch_find_pattern(text,
					   लंबाई,
					   pattern,
					   p_len,
					   case_sens,
					   &match_लंबाई);
			अगर (!pos)
				अवरोध;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_लंबाई,
					cur,
					cur);
			अगर (res != NSERROR_OK) {
				वापस res;
			}

			new_text = pos + match_लंबाई;
			लंबाई -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	क्रम (a = cur->children; a; a = a->अग्र) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		अगर (res != NSERROR_OK) {
			वापस res;
		}
	}

	वापस res;
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
अटल nserror
html_textsearch_find(काष्ठा content *c,
		     काष्ठा textsearch_context *context,
		     स्थिर अक्षर *pattern,
		     पूर्णांक p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	अगर (html->layout == शून्य) {
		वापस NSERROR_INVALID;
	}

	वापस find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


अटल nserror
html_textsearch_bounds(काष्ठा content *c,
		       अचिन्हित start_idx,
		       अचिन्हित end_idx,
		       काष्ठा box *start_box,
		       काष्ठा box *end_box,
		       काष्ठा rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	वापस NSERROR_OK;
}


/**
 * HTML content handler function table
 */
अटल स्थिर content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_तथ्य = html_process_तथ्य,
	.तथ्य_complete = html_convert,
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
nserror html_init(व्योम)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	अगर (error != NSERROR_OK)
		जाओ error;

	क्रम (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		अगर (error != NSERROR_OK)
			जाओ error;
	}

	वापस NSERROR_OK;

error:
	html_fini();

	वापस error;
}
