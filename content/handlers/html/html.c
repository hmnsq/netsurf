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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <nsutils/time.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/http.h"
#અન્તર્ભાવ "utils/libdom.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/utf8.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/string.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/utf8.h"
#અન્તર્ભાવ "netsurf/keypress.h"
#અન્તર્ભાવ "netsurf/layout.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/textsearch.h"
#અન્તર્ભાવ "desktop/selection.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/textarea.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "javascript/js.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/dom_event.h"
#અન્તર્ભાવ "html/css.h"
#અન્તર્ભાવ "html/object.h"
#અન્તર્ભાવ "html/html_save.h"
#અન્તર્ભાવ "html/interaction.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_construct.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/imagemap.h"
#અન્તર્ભાવ "html/layout.h"
#અન્તર્ભાવ "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

સ્થિર અચલ અક્ષર *html_types[] = {
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
સ્થિર bool fire_dom_event(dom_event *event, dom_ગાંઠ *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	જો (exc != DOM_NO_ERR) {
		પાછા false;
	}

	પાછા result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_ગાંઠ *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	જો (exc != DOM_NO_ERR) પાછા false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	જો (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		પાછા false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (પૂર્ણાંક)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	પાછા result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_ગાંઠ *target,
		bool bubbles, bool cancelable, uint32_t ચાવી)
{
	bool is_special = ચાવી <= 0x001F || (0x007F <= ચાવી && ચાવી <= 0x009F);
	dom_string *dom_ચાવી = ખાલી;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	જો (is_special) {
		પસંદગી (ચાવી) {
		સ્થિતિ NS_ચાવી_ESCAPE:
			dom_ચાવી = dom_string_ref(corestring_dom_Escape);
			તોડો;
		સ્થિતિ NS_ચાવી_ડાબું:
			dom_ચાવી = dom_string_ref(corestring_dom_Arrow_ડાબું);
			તોડો;
		સ્થિતિ NS_ચાવી_જમણું:
			dom_ચાવી = dom_string_ref(corestring_dom_Arrow_જમણું);
			તોડો;
		સ્થિતિ NS_ચાવી_UP:
			dom_ચાવી = dom_string_ref(corestring_dom_ArrowUp);
			તોડો;
		સ્થિતિ NS_ચાવી_DOWN:
			dom_ચાવી = dom_string_ref(corestring_dom_ArrowDown);
			તોડો;
		સ્થિતિ NS_ચાવી_PAGE_UP:
			dom_ચાવી = dom_string_ref(corestring_dom_PageUp);
			તોડો;
		સ્થિતિ NS_ચાવી_PAGE_DOWN:
			dom_ચાવી = dom_string_ref(corestring_dom_PageDown);
			તોડો;
		સ્થિતિ NS_ચાવી_TEXT_START:
			dom_ચાવી = dom_string_ref(corestring_dom_Home);
			તોડો;
		સ્થિતિ NS_ચાવી_TEXT_END:
			dom_ચાવી = dom_string_ref(corestring_dom_End);
			તોડો;
		મૂળભૂત:
			dom_ચાવી = ખાલી;
			તોડો;
		}
	} નહીંતર {
		અક્ષર utf8[6];
		size_t length = utf8_from_ucs4(ચાવી, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((અચલ uint8_t *)utf8, strlen(utf8),
				&dom_ચાવી);
		જો (exc != DOM_NO_ERR) {
			પાછા exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	જો (exc != DOM_NO_ERR) {
		dom_string_unref(dom_ચાવી);
		પાછા false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ખાલી,
			dom_ચાવી, ખાલી, DOM_ચાવી_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_ચાવી);
	જો (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		પાછા false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(પૂર્ણાંક)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	પાછા result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
સ્થિર શૂન્ય html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_ગાંઠ *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ખાલી;

	/* Clean up and report error if unsuccessful or aborted */
	જો ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		જો (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, ખાલી);
		} નહીંતર {
			content_broadcast_error(&c->base, NSERROR_STOPPED, ખાલી);
		}

		content_set_error(&c->base);
		પાછા;
	}


#જો ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#જો ALWAYS_DUMP_FRAMESET
	જો (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (શૂન્ય *) &html);
	જો ((exc != DOM_NO_ERR) || (html == ખાલી)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, ખાલી);
		content_set_error(&c->base);
		પાછા;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	જો (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, ખાલી);
		content_set_error(&c->base);
		dom_ગાંઠ_unref(html);
		પાછા;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ખાલી;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_ગાંઠ_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	પસંદગી (content__get_status(&html->base)) {
	સ્થિતિ CONTENT_STATUS_READY:
		જો (html->base.active == 0) {
			content_set_done(&html->base);
			પાછા NSERROR_OK;
		}
		તોડો;
	સ્થિતિ CONTENT_STATUS_DONE:
		/* fallthrough */
	સ્થિતિ CONTENT_STATUS_LOADING:
		પાછા NSERROR_OK;
	મૂળભૂત:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		તોડો;
	}
	પાછા NSERROR_UNKNOWN;
}


સ્થિર શૂન્ય html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	સંઘ content_સંદેશ_data સંદેશ_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_સંદેશ_GETDIMS, &સંદેશ_data);


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
શૂન્ય html_finish_conversion(html_content *htmlc)
{
	સંઘ content_સંદેશ_data સંદેશ_data;
	dom_exception exc; /* returned by libdom functions */
	dom_ગાંઠ *html;
	nserror error;

	/* Bail out if we've been aborted */
	જો (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ખાલી);
		content_set_error(&htmlc->base);
		પાછા;
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
	જો (htmlc->select_ctx != ખાલી) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		પાછા;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	જો (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, ખાલી);
		content_set_error(&htmlc->base);
		પાછા;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	જો (htmlc->jsthread != ખાલી) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ખાલી);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	સંદેશ_data.explicit_status_text = ખાલી;
	content_broadcast(&htmlc->base, CONTENT_સંદેશ_STATUS, &સંદેશ_data);

	exc = dom_document_get_document_element(htmlc->document, (શૂન્ય *) &html);
	જો ((exc != DOM_NO_ERR) || (html == ખાલી)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ખાલી);
		content_set_error(&htmlc->base);
		પાછા;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	જો (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_ગાંઠ_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, ખાલી);
		content_set_error(&htmlc->base);
		પાછા;
	}

	dom_ગાંઠ_unref(html);
}


સ્થિર શૂન્ય
html_document_user_data_handler(dom_ગાંઠ_operation operation,
				dom_string *ચાવી, શૂન્ય *data,
				સંરચના dom_ગાંઠ *src,
				સંરચના dom_ગાંઠ *dst)
{
	જો (dom_string_isequal(corestring_dom_ns_ચાવી_html_content_data,
			       ચાવી) == false || data == ખાલી) {
		પાછા;
	}

	પસંદગી (operation) {
	સ્થિતિ DOM_ગાંઠ_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		તોડો;
	સ્થિતિ DOM_ગાંઠ_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		તોડો;
	સ્થિતિ DOM_ગાંઠ_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		તોડો;
	સ્થિતિ DOM_ગાંઠ_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		તોડો;
	સ્થિતિ DOM_ગાંઠ_DELETED:
		/* This is the only path I expect */
		તોડો;
	મૂળભૂત:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		પ્રતિજ્ઞા(0);
	}
}


સ્થિર nserror
html_create_html_data(html_content *c, અચલ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	શૂન્ય *old_ગાંઠ_data;
	અચલ અક્ષર *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = ખાલી;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ખાલી;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = ખાલી;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = ખાલી;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ખાલી;
	c->bctx = ખાલી;
	c->layout = ખાલી;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_ગણતરી = 0;
	c->stylesheets = ખાલી;
	c->select_ctx = ખાલી;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ખાલી;
	c->અંક_objects = 0;
	c->object_list = ખાલી;
	c->forms = ખાલી;
	c->imagemaps = ખાલી;
	c->bw = ખાલી;
	c->frameset = ખાલી;
	c->iframe = ખાલી;
	c->page = ખાલી;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_ગણતરી = 0;
	c->scripts = ખાલી;
	c->jsthread = ખાલી;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	જો (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		પાછા NSERROR_NOMEM;
	}

	જો (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ખાલી;
		પાછા NSERROR_NOMEM;
	}

	c->sel = selection_create((સંરચના content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	જો (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		જો (c->encoding == ખાલી) {
			lwc_string_unref(c->universal);
			c->universal = ખાલી;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ખાલી;
			પાછા NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.સંદેશ = ખાલી;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	જો ((error != DOM_HUBBUB_OK) && (c->encoding != ખાલી)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		મુક્ત(c->encoding);
		c->encoding = ખાલી;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	જો (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = ખાલી;

		lwc_string_unref(c->universal);
		c->universal = ખાલી;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ખાલી;

		પાછા libdom_hubbub_error_to_nserror(error);
	}

	err = dom_ગાંઠ_set_user_data(c->document,
				     corestring_dom_ns_ચાવી_html_content_data,
				     c, html_document_user_data_handler,
				     (શૂન્ય *) &old_ગાંઠ_data);
	જો (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ખાલી;
		nsurl_unref(c->base_url);
		c->base_url = ખાલી;

		lwc_string_unref(c->universal);
		c->universal = ખાલી;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ખાલી;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		પાછા NSERROR_DOM;
	}

	પ્રતિજ્ઞા(old_ગાંઠ_data == ખાલી);

	પાછા NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

સ્થિર nserror
html_create(અચલ content_handler *handler,
	    lwc_string *imime_type,
	    અચલ http_parameter *params,
	    llcache_handle *llcache,
	    અચલ અક્ષર *fallback_charset,
	    bool quirks,
	    સંરચના content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, કદ(html_content));
	જો (html == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(html);
		પાછા error;
	}

	error = html_create_html_data(html, params);
	જો (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ખાલી);
		મુક્ત(html);
		પાછા error;
	}

	error = html_css_new_stylesheets(html);
	જો (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, ખાલી);
		મુક્ત(html);
		પાછા error;
	}

	*c = (સંરચના content *) html;

	પાછા NSERROR_OK;
}



સ્થિર nserror
html_process_encoding_change(સંરચના content *c,
			     અચલ અક્ષર *data,
			     unsigned પૂર્ણાંક size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	અચલ અક્ષર *encoding;
	અચલ uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	જો (encoding == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	જો (html->encoding != ખાલી) {
		મુક્ત(html->encoding);
		html->encoding = ખાલી;
	}

	html->encoding = strdup(encoding);
	જો (html->encoding == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ખાલી;

	જો (html->document != ખાલી) {
		dom_ગાંઠ_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.સંદેશ = ખાલી;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	જો (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		મુક્ત(html->encoding);
		html->encoding = strdup("Windows-1252");
		જો (html->encoding == ખાલી) {
			પાછા NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		જો (error != DOM_HUBBUB_OK) {
			પાછા libdom_hubbub_error_to_nserror(error);
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

	પાછા libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

સ્થિર bool
html_process_data(સંરચના content *c, અચલ અક્ષર *data, unsigned પૂર્ણાંક size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (અચલ uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	જો (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	જો (err != NSERROR_OK) {
		content_broadcast_error(c, err, ખાલી);
		પાછા false;
	}

	પાછા true;
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

સ્થિર bool html_convert(સંરચના content *c)
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
	જો (exc == DOM_NO_ERR) {
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
	જો (html_can_begin_conversion(htmlc)) {
		પાછા html_begin_conversion(htmlc);
	}
	પાછા true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned પૂર્ણાંક i;

	/* Cannot begin conversion if we're still fetching stuff */
	જો (htmlc->base.active != 0)
		પાછા false;

	માટે (i = 0; i != htmlc->stylesheet_ગણતરી; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		જો (htmlc->stylesheets[i].modified)
			પાછા false;
	}

	/* All is good, begin */
	પાછા true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_ગાંઠ *html;
	nserror ns_error;
	સંરચના form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *ગાંઠ_name = ખાલી;
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
	જો (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		જો (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			પાછા true;
		}
		જો (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						ખાલી);

			પાછા false;
		}
		htmlc->parse_completed = true;
	}

	જો (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		પાછા true;
	}

	/* Give up processing if we've been aborted */
	જો (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, ખાલી);
		પાછા false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	જો (htmlc->encoding == ખાલી) {
		અચલ અક્ષર *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		જો (encoding == ખાલી) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ખાલી);
			પાછા false;
		}

		htmlc->encoding = strdup(encoding);
		જો (htmlc->encoding == ખાલી) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ખાલી);
			પાછા false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (શૂન્ય *) &html);
	જો ((exc != DOM_NO_ERR) || (html == ખાલી)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ખાલી);
		પાછા false;
	}

	exc = dom_ગાંઠ_get_ગાંઠ_name(html, &ગાંઠ_name);
	જો ((exc != DOM_NO_ERR) ||
	    (ગાંઠ_name == ખાલી) ||
	    (!dom_string_caseless_lwc_isequal(ગાંઠ_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, ખાલી);
		dom_ગાંઠ_unref(html);
		પાછા false;
	}
	dom_string_unref(ગાંઠ_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	માટે (f = htmlc->forms; f != ખાલી; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		જો (f->action == ખાલી || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} નહીંતર {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		જો (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, ખાલી);

			dom_ગાંઠ_unref(html);
			પાછા false;
		}

		મુક્ત(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		જો (f->action == ખાલી) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						ખાલી);

			dom_ગાંઠ_unref(html);
			પાછા false;
		}

		/* Ensure each form has a document encoding */
		જો (f->document_charset == ખાલી) {
			f->document_charset = strdup(htmlc->encoding);
			જો (f->document_charset == ખાલી) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							ખાલી);
				dom_ગાંઠ_unref(html);
				પાછા false;
			}
		}
	}

	dom_ગાંઠ_unref(html);

	જો (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	પાછા true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

સ્થિર શૂન્ય html_stop(સંરચના content *c)
{
	html_content *htmlc = (html_content *) c;

	પસંદગી (c->status) {
	સ્થિતિ CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		જો (htmlc->jsthread != ખાલી) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		તોડો;

	સ્થિતિ CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		જો (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		તોડો;

	સ્થિતિ CONTENT_STATUS_DONE:
		/* Nothing to do */
		તોડો;

	મૂળભૂત:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		પ્રતિજ્ઞા(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

સ્થિર શૂન્ય html_reformat(સંરચના content *c, પૂર્ણાંક width, પૂર્ણાંક height)
{
	html_content *htmlc = (html_content *) c;
	સંરચના box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.મૂળ_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[ડાબું_2] + layout->width +
		layout->padding[જમણું_2] + layout->border[જમણું_2].width +
		layout->margin[જમણું_2];
	c->height = layout->y + layout->padding[ટોચ_2] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	જો (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	જો (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	જો (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

શૂન્ય html_redraw_a_box(hlcache_handle *h, સંરચના box *box)
{
	પૂર્ણાંક x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ડાબું_2] + box->width + box->padding[જમણું_2],
			box->padding[ટોચ_2] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

શૂન્ય html__redraw_a_box(સંરચના html_content *html, સંરચના box *box)
{
	પૂર્ણાંક x, y;

	box_coords(box, &x, &y);

	content__request_redraw((સંરચના content *)html, x, y,
			box->padding[ડાબું_2] + box->width + box->padding[જમણું_2],
			box->padding[ટોચ_2] + box->height + box->padding[BOTTOM]);
}

સ્થિર શૂન્ય html_destroy_frameset(સંરચના content_html_frames *frameset)
{
	પૂર્ણાંક i;

	જો (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ખાલી;
	}
	જો (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ખાલી;
	}
	જો (frameset->children) {
		માટે (i = 0; i < (frameset->rows * frameset->cols); i++) {
			જો (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ખાલી;
			}
			જો (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ખાલી;
			}
			જો (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ખાલી;
	}
}

સ્થિર શૂન્ય html_destroy_iframe(સંરચના content_html_iframe *iframe)
{
	સંરચના content_html_iframe *next;
	next = iframe;
	જ્યારે ((iframe = next) != ખાલી) {
		next = iframe->next;
		જો (iframe->name)
			talloc_free(iframe->name);
		જો (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ખાલી;
		}
		talloc_free(iframe);
	}
}


સ્થિર શૂન્ય html_free_layout(html_content *htmlc)
{
	જો (htmlc->bctx != ખાલી) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

સ્થિર શૂન્ય html_destroy(સંરચના content *c)
{
	html_content *html = (html_content *) c;
	સંરચના form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	જો (html->box_conversion_context != ખાલી) {
		જો (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	માટે (f = html->forms; f != ખાલી; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	જો (c->refresh)
		nsurl_unref(c->refresh);

	જો (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	જો (html->jsthread != ખાલી) {
		js_destroythread(html->jsthread);
		html->jsthread = ખાલી;
	}

	જો (html->parser != ખાલી) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ખાલી;
	}

	જો (html->document != ખાલી) {
		dom_ગાંઠ_unref(html->document);
		html->document = ખાલી;
	}

	જો (html->title != ખાલી) {
		dom_ગાંઠ_unref(html->title);
		html->title = ખાલી;
	}

	/* Free encoding */
	જો (html->encoding != ખાલી) {
		મુક્ત(html->encoding);
		html->encoding = ખાલી;
	}

	/* Free base target */
	જો (html->base_target != ખાલી) {
		મુક્ત(html->base_target);
		html->base_target = ખાલી;
	}

	/* Free frameset */
	જો (html->frameset != ખાલી) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ખાલી;
	}

	/* Free iframes */
	જો (html->iframe != ખાલી) {
		html_destroy_iframe(html->iframe);
		html->iframe = ખાલી;
	}

	/* Destroy selection context */
	જો (html->select_ctx != ખાલી) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ખાલી;
	}

	lwc_string_unref(html->universal);
	html->universal = ખાલી;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ખાલી;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


સ્થિર nserror html_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	પ્રતિજ્ઞા(0 && "html_clone should never be called");

	પાછા true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

સ્થિર nserror
html_open(સંરચના content *c,
	  સંરચના browser_window *bw,
	  સંરચના content *page,
	  સંરચના object_params *params)
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

	પાછા NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

સ્થિર nserror html_close(સંરચના content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ખાલી;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	જો (htmlc->jsthread != ખાલી) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	પાછા ret;
}


/**
 * Return an HTML content's selection context
 */

સ્થિર શૂન્ય html_clear_selection(સંરચના content *c)
{
	html_content *html = (html_content *) c;

	પસંદગી (html->selection_type) {
	સ્થિતિ HTML_SELECTION_NONE:
		/* Nothing to do */
		પ્રતિજ્ઞા(html->selection_owner.none == true);
		તોડો;
	સ્થિતિ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		તોડો;
	સ્થિતિ HTML_SELECTION_SELF:
		પ્રતિજ્ઞા(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		તોડો;
	સ્થિતિ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

સ્થિર અક્ષર *html_get_selection(સંરચના content *c)
{
	html_content *html = (html_content *) c;

	પસંદગી (html->selection_type) {
	સ્થિતિ HTML_SELECTION_TEXTAREA:
		પાછા textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	સ્થિતિ HTML_SELECTION_SELF:
		પ્રતિજ્ઞા(html->selection_owner.none == false);
		પાછા selection_get_copy(html->sel);
	સ્થિતિ HTML_SELECTION_CONTENT:
		પાછા content_get_selection(
				html->selection_owner.content->object);
	સ્થિતિ HTML_SELECTION_NONE:
		/* Nothing to do */
		પ્રતિજ્ઞા(html->selection_owner.none == true);
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	પાછા ખાલી;
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
સ્થિર nserror
html_get_contextual_content(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y,
			    સંરચના browser_window_features *data)
{
	html_content *html = (html_content *) c;

	સંરચના box *box = html->layout;
	સંરચના box *next;
	પૂર્ણાંક box_x = 0, box_y = 0;

	જ્યારે ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ખાલી) {
		box = next;

		/* hidden boxes are ignored */
		જો ((box->style != ખાલી) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ચાલુ;
		}

		જો (box->iframe) {
			દશાંશ scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		જો (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		જો (box->object)
			data->object = box->object;

		જો (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		જો (box->usemap) {
			અચલ અક્ષર *target = ખાલી;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			જો (url != ખાલી)
				data->link = url;
		}
		જો (box->gadget) {
			પસંદગી (box->gadget->type) {
			સ્થિતિ GADGET_TEXTBOX:
			સ્થિતિ GADGET_TEXTAREA:
			સ્થિતિ GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				તોડો;

			સ્થિતિ GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				તોડો;

			મૂળભૂત:
				data->form_features = CTX_FORM_NONE;
				તોડો;
			}
		}
	}
	પાછા NSERROR_OK;
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
સ્થિર bool
html_scroll_at_point(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક scrx, પૂર્ણાંક scry)
{
	html_content *html = (html_content *) c;

	સંરચના box *box = html->layout;
	સંરચના box *next;
	પૂર્ણાંક box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	જ્યારે ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ખાલી) {
		box = next;

		જો (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ચાલુ;

		/* Pass into iframe */
		જો (box->iframe) {
			દશાંશ scale = browser_window_get_scale(box->iframe);

			જો (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				પાછા true;
		}

		/* Pass into textarea widget */
		જો (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			પાછા true;

		/* Pass into object */
		જો (box->object != ખાલી && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			પાછા true;

		/* Handle box scrollbars */
		જો (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		જો (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		જો (handled_scroll == true)
			પાછા true;
	}

	પાછા false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
સ્થિર શૂન્ય html__dom_user_data_handler(dom_ગાંઠ_operation operation,
		dom_string *ચાવી, શૂન્ય *_data, સંરચના dom_ગાંઠ *src,
		સંરચના dom_ગાંઠ *dst)
{
	અક્ષર *oldfile;
	અક્ષર *data = (અક્ષર *)_data;

	જો (!dom_string_isequal(corestring_dom_ns_ચાવી_file_name_ગાંઠ_data,
				ચાવી) || data == ખાલી) {
		પાછા;
	}

	પસંદગી (operation) {
	સ્થિતિ DOM_ગાંઠ_CLONED:
		જો (dom_ગાંઠ_set_user_data(dst,
					   corestring_dom_ns_ચાવી_file_name_ગાંઠ_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			જો (oldfile != ખાલી)
				મુક્ત(oldfile);
		}
		તોડો;

	સ્થિતિ DOM_ગાંઠ_RENAMED:
	સ્થિતિ DOM_ગાંઠ_IMPORTED:
	સ્થિતિ DOM_ગાંઠ_ADOPTED:
		તોડો;

	સ્થિતિ DOM_ગાંઠ_DELETED:
		મુક્ત(data);
		તોડો;
	મૂળભૂત:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		પ્રતિજ્ઞા(0);
	}
}

સ્થિર શૂન્ય html__set_file_gadget_filename(સંરચના content *c,
	સંરચના form_control *gadget, અચલ અક્ષર *fn)
{
	nserror ret;
	અક્ષર *utf8_fn, *oldfile = ખાલી;
	html_content *html = (html_content *)c;
	સંરચના box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	જો (ret != NSERROR_OK) {
		પ્રતિજ્ઞા(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		પાછા;
	}

	form_gadget_update_કિંમત(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	જો (dom_ગાંઠ_set_user_data((dom_ગાંઠ *)file_box->gadget->ગાંઠ,
				   corestring_dom_ns_ચાવી_file_name_ગાંઠ_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		જો (oldfile != ખાલી)
			મુક્ત(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

શૂન્ય html_set_file_gadget_filename(સંરચના hlcache_handle *hl,
	સંરચના form_control *gadget, અચલ અક્ષર *fn)
{
	પાછા html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
સ્થિર bool html_drop_file_at_point(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y, અક્ષર *file)
{
	html_content *html = (html_content *) c;

	સંરચના box *box = html->layout;
	સંરચના box *next;
	સંરચના box *file_box = ખાલી;
	સંરચના box *text_box = ખાલી;
	પૂર્ણાંક box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	જ્યારે ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ખાલી) {
		box = next;

		જો (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ચાલુ;

		જો (box->iframe) {
			દશાંશ scale = browser_window_get_scale(box->iframe);
			પાછા browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		જો (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			પાછા true;

		જો (box->gadget) {
			પસંદગી (box->gadget->type) {
				સ્થિતિ GADGET_FILE:
					file_box = box;
				તોડો;

				સ્થિતિ GADGET_TEXTBOX:
				સ્થિતિ GADGET_TEXTAREA:
				સ્થિતિ GADGET_PASSWORD:
					text_box = box;
					તોડો;

				મૂળભૂત:	/* appease compiler */
					તોડો;
			}
		}
	}

	જો (!file_box && !text_box)
		/* No box capable of handling drop */
		પાછા false;

	જો (file == ખાલી)
		/* There is a box capable of handling drop here */
		પાછા true;

	/* Handle the drop */
	જો (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} નહીંતર {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ખાલી;
		અક્ષર *buffer;
		અક્ષર *utf8_buff;
		nserror ret;
		unsigned પૂર્ણાંક size;
		પૂર્ણાંક bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		જો (fp == ખાલી) {
			/* Couldn't open file, but drop was for us */
			પાછા true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		જો ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			પાછા true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		જો (buffer == ખાલી) {
			/* No memory, but drop was for us */
			fclose(fp);
			પાછા true;
		}

		/* Stick file into buffer */
		જો (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			મુક્ત(buffer);
			fclose(fp);
			પાછા true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		જો (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			પ્રતિજ્ઞા(ret != NSERROR_BAD_ENCODING);
			મુક્ત(buffer);
			પાછા true;
		}

		/* Done with buffer */
		મુક્ત(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		મુક્ત(utf8_buff);
	}

	પાછા true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
સ્થિર nserror
html_debug(સંરચના content *c, ગણના content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	પાછા NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
સ્થિર nserror
html_debug_dump(સંરચના content *c, FILE *f, ગણના content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_ગાંઠ *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	પ્રતિજ્ઞા(htmlc != ખાલી);

	જો (op == CONTENT_DEBUG_RENDER) {
		પ્રતિજ્ઞા(htmlc->layout != ખાલી);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} નહીંતર {
		જો (htmlc->document == ખાલી) {
			NSLOG(netsurf, INFO, "No document to dump");
			પાછા NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (શૂન્ય *) &html);
		જો ((exc != DOM_NO_ERR) || (html == ખાલી)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			પાછા NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_ગાંઠ_unref(html);
	}

	પાછા ret;
}


#જો ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

સ્થિર શૂન્ય
html_dump_frameset(સંરચના content_html_frames *frame, unsigned પૂર્ણાંક depth)
{
	unsigned પૂર્ણાંક i;
	પૂર્ણાંક row, col, index;
	અચલ અક્ષર *unit[] = {"px", "%", "*"};
	અચલ અક્ષર *scrolling[] = {"auto", "yes", "no"};

	પ્રતિજ્ઞા(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.કિંમત, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.કિંમત,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	જો (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	જો (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	જો (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	જો (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned પૂર્ણાંક) frame->border_colour);

	fprintf(stderr, "\n");

	જો (frame->children) {
		માટે (row = 0; row != frame->rows; row++) {
			માટે (col = 0; col != frame->cols; col++) {
				માટે (i = 0; i != depth; i++)
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

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->document;
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
સંરચના box *html_get_box_વૃક્ષ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
સ્થિર અચલ અક્ષર *html_encoding(અચલ સંરચના content *c, ગણના content_encoding_type op)
{
	html_content *html = (html_content *) c;
	સ્થિર અક્ષર enc_token[10] = "Encoding0";

	પ્રતિજ્ઞા(html != ખાલી);

	જો (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		પાછા messages_get(enc_token);
	}

	પાછા html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
સંરચના content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
સંરચના content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->iframe;
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

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
અચલ અક્ષર *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, પૂર્ણાંક *x, પૂર્ણાંક *y)
{
	સંરચના box *pos;
	સંરચના box *layout;

	જો (content_get_type(h) != CONTENT_HTML)
		પાછા false;

	layout = html_get_box_વૃક્ષ(h);

	જો ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		પાછા true;
	}
	પાછા false;
}

bool html_exec(સંરચના content *c, અચલ અક્ષર *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_ગાંઠ;
	dom_string *dom_src;
	dom_text *text_ગાંઠ;
	dom_ગાંઠ *spare_ગાંઠ;
	dom_html_script_element *script_ગાંઠ;

	જો (htmlc->document == ખાલી) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		જાઓ out_no_string;
	}

	err = dom_string_create((અચલ uint8_t *)src, srclen, &dom_src);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		જાઓ out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_ગાંઠ);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		જાઓ out_no_body;
	}

	err = dom_document_create_text_ગાંઠ(htmlc->document, dom_src, &text_ગાંઠ);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		જાઓ out_no_text_ગાંઠ;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_ગાંઠ);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		જાઓ out_no_script_ગાંઠ;
	}

	err = dom_ગાંઠ_append_child(script_ગાંઠ, text_ગાંઠ, &spare_ગાંઠ);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		જાઓ out_unparented;
	}
	dom_ગાંઠ_unref(spare_ગાંઠ); /* We do not need the spare ref at all */

	err = dom_ગાંઠ_append_child(body_ગાંઠ, script_ગાંઠ, &spare_ગાંઠ);
	જો (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		જાઓ out_unparented;
	}
	dom_ગાંઠ_unref(spare_ગાંઠ); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_ગાંઠ_get_parent_ગાંઠ(script_ગાંઠ, &spare_ગાંઠ);
	જો (err == DOM_NO_ERR && spare_ગાંઠ != ખાલી) {
		dom_ગાંઠ *second_spare;
		err = dom_ગાંઠ_remove_child(spare_ગાંઠ, script_ગાંઠ, &second_spare);
		જો (err == DOM_NO_ERR) {
			dom_ગાંઠ_unref(second_spare);
		}
		dom_ગાંઠ_unref(spare_ગાંઠ);
	}

out_unparented:
	dom_ગાંઠ_unref(script_ગાંઠ);
out_no_script_ગાંઠ:
	dom_ગાંઠ_unref(text_ગાંઠ);
out_no_text_ગાંઠ:
	dom_ગાંઠ_unref(body_ગાંઠ);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	પાછા result;
}

/* See \ref content_saw_insecure_objects */
સ્થિર bool
html_saw_insecure_objects(સંરચના content *c)
{
	html_content *htmlc = (html_content *)c;
	સંરચના content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	જ્યારે (obj != ખાલી) {
		જો (obj->content != ખાલી) {
			જો (content_saw_insecure_objects(obj->content))
				પાછા true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	જો (html_saw_insecure_scripts(htmlc)) {
		પાછા true;
	}

	/* Now check stylesheets */
	જો (html_css_saw_insecure_stylesheets(htmlc)) {
		પાછા true;
	}

	પાછા false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
સ્થિર content_type html_content_type(શૂન્ય)
{
	પાછા CONTENT_HTML;
}


સ્થિર શૂન્ય html_fini(શૂન્ય)
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
સ્થિર nserror
find_occurrences_html_box(અચલ અક્ષર *pattern,
			  પૂર્ણાંક p_len,
			  સંરચના box *cur,
			  bool case_sens,
			  સંરચના textsearch_context *context)
{
	સંરચના box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	જો (!cur->object && cur->text) {
		અચલ અક્ષર *text = cur->text;
		unsigned length = cur->length;

		જ્યારે (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			અચલ અક્ષર *new_text;
			અચલ અક્ષર *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			જો (!pos)
				તોડો;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			જો (res != NSERROR_OK) {
				પાછા res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	માટે (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		જો (res != NSERROR_OK) {
			પાછા res;
		}
	}

	પાછા res;
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
સ્થિર nserror
html_textsearch_find(સંરચના content *c,
		     સંરચના textsearch_context *context,
		     અચલ અક્ષર *pattern,
		     પૂર્ણાંક p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	જો (html->layout == ખાલી) {
		પાછા NSERROR_INVALID;
	}

	પાછા find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


સ્થિર nserror
html_textsearch_bounds(સંરચના content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       સંરચના box *start_box,
		       સંરચના box *end_box,
		       સંરચના rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	પાછા NSERROR_OK;
}


/**
 * HTML content handler function table
 */
સ્થિર અચલ content_handler html_content_handler = {
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
nserror html_init(શૂન્ય)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	જો (error != NSERROR_OK)
		જાઓ error;

	માટે (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		જો (error != NSERROR_OK)
			જાઓ error;
	}

	પાછા NSERROR_OK;

error:
	html_fini();

	પાછા error;
}
