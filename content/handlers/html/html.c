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

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <nsutils/time.h>

#include "utils/utils.h"
#include "utils/config.h"
#include "utils/corestrings.h"
#include "utils/http.h"
#include "utils/libdom.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/talloc.h"
#include "utils/utf8.h"
#include "utils/nsoption.h"
#include "utils/string.h"
#include "utils/ascii.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/utf8.h"
#include "netsurf/keypress.h"
#include "netsurf/layout.h"
#include "netsurf/misc.h"
#include "content/hlcache.h"
#include "content/content_factory.h"
#include "content/textsearch.h"
#include "desktop/selection.h"
#include "desktop/scrollbar.h"
#include "desktop/textarea.h"
#include "netsurf/bitmap.h"
#include "javascript/js.h"
#include "desktop/gui_internal.h"

#include "html/html.h"
#include "html/private.h"
#include "html/dom_event.h"
#include "html/css.h"
#include "html/object.h"
#include "html/html_save.h"
#include "html/interaction.h"
#include "html/box.h"
#include "html/box_construct.h"
#include "html/box_inspect.h"
#include "html/form_internal.h"
#include "html/imagemap.h"
#include "html/layout.h"
#include "html/textselection.h"

#define CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#define ALWAYS_DUMP_FRAMESET 0
#define ALWAYS_DUMP_BOX 0

static const var_na *html_types[] = {
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
static bool fire_dom_event(dom_event *event, dom_node *target)
{
	dom_exception exc;
	bool result;

	exc = dom_event_target_dispatch_event(target, event, &result);
	ᯀᯮᯝ (exc != DOM_NO_ERR) {
		ᯀᯮᯞ false;
	}

	ᯀᯮᯞ result;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_node *target,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool result;

	exc = dom_event_create(&evt);
	ᯀᯮᯝ (exc != DOM_NO_ERR) ᯀᯮᯞ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ᯀᯮᯝ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ᯀᯮᯞ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (puur_naa_nka)dom_string_length(type), dom_string_data(type), target);
	result = fire_dom_event(evt, target);
	dom_event_unref(evt);
	ᯀᯮᯞ result;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_node *target,
		bool bubbles, bool cancelable, uint32_t key)
{
	bool is_special = key <= 0x001F || (0x007F <= key && key <= 0x009F);
	dom_string *dom_key = NULL;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool result;

	ᯀᯮᯝ (is_special) {
		switch (key) {
		pak_sa NS_KEY_ESCAPE:
			dom_key = dom_string_ref(corestring_dom_Escape);
			bha_nga;
		pak_sa NS_KEY_LEFT:
			dom_key = dom_string_ref(corestring_dom_ArrowLeft);
			bha_nga;
		pak_sa NS_KEY_RIGHT:
			dom_key = dom_string_ref(corestring_dom_ArrowRight);
			bha_nga;
		pak_sa NS_KEY_UP:
			dom_key = dom_string_ref(corestring_dom_ArrowUp);
			bha_nga;
		pak_sa NS_KEY_DOWN:
			dom_key = dom_string_ref(corestring_dom_ArrowDown);
			bha_nga;
		pak_sa NS_KEY_PAGE_UP:
			dom_key = dom_string_ref(corestring_dom_PageUp);
			bha_nga;
		pak_sa NS_KEY_PAGE_DOWN:
			dom_key = dom_string_ref(corestring_dom_PageDown);
			bha_nga;
		pak_sa NS_KEY_TEXT_START:
			dom_key = dom_string_ref(corestring_dom_Home);
			bha_nga;
		pak_sa NS_KEY_TEXT_END:
			dom_key = dom_string_ref(corestring_dom_End);
			bha_nga;
		default:
			dom_key = NULL;
			bha_nga;
		}
	} ᯀᯮᯝᯂᯪ {
		var_na utf8[6];
		size_t length = utf8_from_ucs4(key, utf8);
		utf8[length] = '\0';

		exc = dom_string_create((const uint8_t *)utf8, strlen(utf8),
				&dom_key);
		ᯀᯮᯝ (exc != DOM_NO_ERR) {
			ᯀᯮᯞ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ᯀᯮᯝ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_key);
		ᯀᯮᯞ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, NULL,
			dom_key, NULL, DOM_KEY_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_key);
	ᯀᯮᯝ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ᯀᯮᯞ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(puur_naa_nka)dom_string_length(type), dom_string_data(type), target);

	result = fire_dom_event((dom_event *) evt, target);
	dom_event_unref(evt);
	ᯀᯮᯞ result;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
static void html_box_convert_done(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_node *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = NULL;

	/* Clean up and report error if unsuccessful or aborted */
	ᯀᯮᯝ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ᯀᯮᯝ (success == false) {
			content_broadcast_error(&c->base, NSERROR_BOX_CONVERT, NULL);
		} ᯀᯮᯝᯂᯪ {
			content_broadcast_error(&c->base, NSERROR_STOPPED, NULL);
		}

		content_set_error(&c->base);
		ᯀᯮᯞ;
	}


#ᯀᯮᯝ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#endif
#ᯀᯮᯝ ALWAYS_DUMP_FRAMESET
	ᯀᯮᯝ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#endif

	exc = dom_document_get_document_element(c->document, (void *) &html);
	ᯀᯮᯝ ((exc != DOM_NO_ERR) || (html == NULL)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&c->base, NSERROR_DOM, NULL);
		content_set_error(&c->base);
		ᯀᯮᯞ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_extract(c);
	ᯀᯮᯝ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_broadcast_error(&c->base, err, NULL);
		content_set_error(&c->base);
		dom_node_unref(html);
		ᯀᯮᯞ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = NULL;

	content_set_ready(&c->base);

	html_proceed_to_done(c);

	dom_node_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_to_done(html_content *html)
{
	switch (content__get_status(&html->base)) {
	pak_sa CONTENT_STATUS_READY:
		ᯀᯮᯝ (html->base.active == 0) {
			content_set_done(&html->base);
			ᯀᯮᯞ NSERROR_OK;
		}
		bha_nga;
	pak_sa CONTENT_STATUS_DONE:
		/* fallthrough */
	pak_sa CONTENT_STATUS_LOADING:
		ᯀᯮᯞ NSERROR_OK;
	default:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		bha_nga;
	}
	ᯀᯮᯞ NSERROR_UNKNOWN;
}


static void html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	unsigned f_size;
	unsigned f_min;
	unsigned w;
	unsigned h;
	sa_ngha content_msg_data msg_data = {
		.getdims = {
			.viewport_width = &w,
			.viewport_height = &h,
		},
	};

	content_broadcast(&htmlc->base, CONTENT_MSG_GETDIMS, &msg_data);


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
void html_finish_conversion(html_content *htmlc)
{
	sa_ngha content_msg_data msg_data;
	dom_exception exc; /* returned by libdom functions */
	dom_node *html;
	nserror error;

	/* Bail out if we've been aborted */
	ᯀᯮᯝ (htmlc->aborted) {
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, NULL);
		content_set_error(&htmlc->base);
		ᯀᯮᯞ;
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
	ᯀᯮᯝ (htmlc->select_ctx != NULL) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ᯀᯮᯞ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ᯀᯮᯝ (error != NSERROR_OK) {
		content_broadcast_error(&htmlc->base, error, NULL);
		content_set_error(&htmlc->base);
		ᯀᯮᯞ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ᯀᯮᯝ (htmlc->jsthread != NULL) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, NULL);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_set_status(&htmlc->base, messages_get("Processing"));
	msg_data.explicit_status_text = NULL;
	content_broadcast(&htmlc->base, CONTENT_MSG_STATUS, &msg_data);

	exc = dom_document_get_document_element(htmlc->document, (void *) &html);
	ᯀᯮᯝ ((exc != DOM_NO_ERR) || (html == NULL)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, NULL);
		content_set_error(&htmlc->base);
		ᯀᯮᯞ;
	}

	html_get_dimensions(htmlc);

	error = dom_to_box(html, htmlc, html_box_convert_done, &htmlc->box_conversion_context);
	ᯀᯮᯝ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_node_unref(html);
		html_object_free_objects(htmlc);
		content_broadcast_error(&htmlc->base, error, NULL);
		content_set_error(&htmlc->base);
		ᯀᯮᯞ;
	}

	dom_node_unref(html);
}


static void
html_document_user_data_handler(dom_node_operation operation,
				dom_string *key, void *data,
				sa_nracanaa dom_node *src,
				sa_nracanaa dom_node *dst)
{
	ᯀᯮᯝ (dom_string_isequal(corestring_dom___ns_key_html_content_data,
			       key) == false || data == NULL) {
		ᯀᯮᯞ;
	}

	switch (operation) {
	pak_sa DOM_NODE_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		bha_nga;
	pak_sa DOM_NODE_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		bha_nga;
	pak_sa DOM_NODE_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		bha_nga;
	pak_sa DOM_NODE_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		bha_nga;
	pak_sa DOM_NODE_DELETED:
		/* This is the only path I expect */
		bha_nga;
	default:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		assert(0);
	}
}


static nserror
html_create_html_data(html_content *c, const http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	void *old_node_data;
	const var_na *prefer_color_mode = (nsoption_bool(prefer_dark_mode)) ?
			"dark" : "light";

	c->parser = NULL;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = NULL;
	c->quirks = DOM_DOCUMENT_QUIRKS_MODE_NONE;
	c->encoding = NULL;
	c->base_url = nsurl_ref(content_get_url(&c->base));
	c->base_target = NULL;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = NULL;
	c->bctx = NULL;
	c->layout = NULL;
	c->background_colour = NS_TRANSPARENT;
	c->stylesheet_count = 0;
	c->stylesheets = NULL;
	c->select_ctx = NULL;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = NULL;
	c->num_objects = 0;
	c->object_list = NULL;
	c->forms = NULL;
	c->imagemaps = NULL;
	c->bw = NULL;
	c->frameset = NULL;
	c->iframe = NULL;
	c->page = NULL;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_count = 0;
	c->scripts = NULL;
	c->jsthread = NULL;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->base.active = 1; /* The html content itself is active */

	ᯀᯮᯝ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ᯀᯮᯞ NSERROR_NOMEM;
	}

	ᯀᯮᯝ (lwc_intern_string(prefer_color_mode, strlen(prefer_color_mode),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = NULL;
		ᯀᯮᯞ NSERROR_NOMEM;
	}

	c->sel = selection_create((sa_nracanaa content *)c);

	nerror = http_parameter_list_find_item(params, corestring_lwc_charset, &charset);
	ᯀᯮᯝ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_data(charset));

		lwc_string_unref(charset);

		ᯀᯮᯝ (c->encoding == NULL) {
			lwc_string_unref(c->universal);
			c->universal = NULL;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = NULL;
			ᯀᯮᯞ NSERROR_NOMEM;

		}
		c->encoding_source = DOM_HUBBUB_ENCODING_SOURCE_HEADER;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.msg = NULL;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ᯀᯮᯝ ((error != DOM_HUBBUB_OK) && (c->encoding != NULL)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		free(c->encoding);
		c->encoding = NULL;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ᯀᯮᯝ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->base_url);
		c->base_url = NULL;

		lwc_string_unref(c->universal);
		c->universal = NULL;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = NULL;

		ᯀᯮᯞ libdom_hubbub_error_to_nserror(error);
	}

	err = dom_node_set_user_data(c->document,
				     corestring_dom___ns_key_html_content_data,
				     c, html_document_user_data_handler,
				     (void *) &old_node_data);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = NULL;
		nsurl_unref(c->base_url);
		c->base_url = NULL;

		lwc_string_unref(c->universal);
		c->universal = NULL;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = NULL;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ᯀᯮᯞ NSERROR_DOM;
	}

	assert(old_node_data == NULL);

	ᯀᯮᯞ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

static nserror
html_create(const content_handler *handler,
	    lwc_string *imime_type,
	    const http_parameter *params,
	    llcache_handle *llcache,
	    const var_na *fallback_charset,
	    bool quirks,
	    sa_nracanaa content **c)
{
	html_content *html;
	nserror error;

	html = calloc(1, parimaa_na(html_content));
	ᯀᯮᯝ (html == NULL)
		ᯀᯮᯞ NSERROR_NOMEM;

	error = content__init(&html->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ᯀᯮᯝ (error != NSERROR_OK) {
		free(html);
		ᯀᯮᯞ error;
	}

	error = html_create_html_data(html, params);
	ᯀᯮᯝ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, NULL);
		free(html);
		ᯀᯮᯞ error;
	}

	error = html_css_new_stylesheets(html);
	ᯀᯮᯝ (error != NSERROR_OK) {
		content_broadcast_error(&html->base, error, NULL);
		free(html);
		ᯀᯮᯞ error;
	}

	*c = (sa_nracanaa content *) html;

	ᯀᯮᯞ NSERROR_OK;
}



static nserror
html_process_encoding_change(sa_nracanaa content *c,
			     const var_na *data,
			     unsigned puur_naa_nka size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	const var_na *encoding;
	const uint8_t *source_data;
	size_t source_size;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_source);
	ᯀᯮᯝ (encoding == NULL) {
		ᯀᯮᯞ NSERROR_NOMEM;
	}

	ᯀᯮᯝ (html->encoding != NULL) {
		free(html->encoding);
		html->encoding = NULL;
	}

	html->encoding = strdup(encoding);
	ᯀᯮᯝ (html->encoding == NULL) {
		ᯀᯮᯞ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = NULL;

	ᯀᯮᯝ (html->document != NULL) {
		dom_node_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.msg = NULL;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ᯀᯮᯝ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		free(html->encoding);
		html->encoding = strdup("Windows-1252");
		ᯀᯮᯝ (html->encoding == NULL) {
			ᯀᯮᯞ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ᯀᯮᯝ (error != DOM_HUBBUB_OK) {
			ᯀᯮᯞ libdom_hubbub_error_to_nserror(error);
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

	ᯀᯮᯞ libdom_hubbub_error_to_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

static bool
html_process_data(sa_nracanaa content *c, const var_na *data, unsigned puur_naa_nka size)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (const uint8_t *) data,
					      size);

	err = libdom_hubbub_error_to_nserror(dom_ret);

	/* deal with encoding change */
	ᯀᯮᯝ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, data, size);
	}

	/* broadcast the error if necessary */
	ᯀᯮᯝ (err != NSERROR_OK) {
		content_broadcast_error(c, err, NULL);
		ᯀᯮᯞ false;
	}

	ᯀᯮᯞ true;
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

static bool html_convert(sa_nracanaa content *c)
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
	ᯀᯮᯝ (exc == DOM_NO_ERR) {
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
	ᯀᯮᯝ (html_can_begin_conversion(htmlc)) {
		ᯀᯮᯞ html_begin_conversion(htmlc);
	}
	ᯀᯮᯞ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	unsigned puur_naa_nka i;

	/* Cannot begin conversion if we're still fetching stuff */
	ᯀᯮᯝ (htmlc->base.active != 0)
		ᯀᯮᯞ false;

	for (i = 0; i != htmlc->stylesheet_count; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ᯀᯮᯝ (htmlc->stylesheets[i].modified)
			ᯀᯮᯞ false;
	}

	/* All is good, begin */
	ᯀᯮᯞ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_node *html;
	nserror ns_error;
	sa_nracanaa form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *node_name = NULL;
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
	ᯀᯮᯝ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ᯀᯮᯝ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->base.active > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ᯀᯮᯞ true;
		}
		ᯀᯮᯝ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_broadcast_error(&htmlc->base,
						libdom_hubbub_error_to_nserror(error),
						NULL);

			ᯀᯮᯞ false;
		}
		htmlc->parse_completed = true;
	}

	ᯀᯮᯝ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ᯀᯮᯞ true;
	}

	/* Give up processing if we've been aborted */
	ᯀᯮᯝ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->base.active);
		content_set_error(&htmlc->base);
		content_broadcast_error(&htmlc->base, NSERROR_STOPPED, NULL);
		ᯀᯮᯞ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ᯀᯮᯝ (htmlc->encoding == NULL) {
		const var_na *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_source);
		ᯀᯮᯝ (encoding == NULL) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						NULL);
			ᯀᯮᯞ false;
		}

		htmlc->encoding = strdup(encoding);
		ᯀᯮᯝ (htmlc->encoding == NULL) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						NULL);
			ᯀᯮᯞ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (void *) &html);
	ᯀᯮᯝ ((exc != DOM_NO_ERR) || (html == NULL)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, NULL);
		ᯀᯮᯞ false;
	}

	exc = dom_node_get_node_name(html, &node_name);
	ᯀᯮᯝ ((exc != DOM_NO_ERR) ||
	    (node_name == NULL) ||
	    (!dom_string_caseless_lwc_isequal(node_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_broadcast_error(&htmlc->base, NSERROR_DOM, NULL);
		dom_node_unref(html);
		ᯀᯮᯞ false;
	}
	dom_string_unref(node_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	for (f = htmlc->forms; f != NULL; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ᯀᯮᯝ (f->action == NULL || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->base);
			ns_error = nsurl_join(htmlc->base_url,
					      nsurl_access(doc_addr),
					      &action);
		} ᯀᯮᯝᯂᯪ {
			ns_error = nsurl_join(htmlc->base_url,
					      f->action,
					      &action);
		}

		ᯀᯮᯝ (ns_error != NSERROR_OK) {
			content_broadcast_error(&htmlc->base, ns_error, NULL);

			dom_node_unref(html);
			ᯀᯮᯞ false;
		}

		free(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ᯀᯮᯝ (f->action == NULL) {
			content_broadcast_error(&htmlc->base,
						NSERROR_NOMEM,
						NULL);

			dom_node_unref(html);
			ᯀᯮᯞ false;
		}

		/* Ensure each form has a document encoding */
		ᯀᯮᯝ (f->document_charset == NULL) {
			f->document_charset = strdup(htmlc->encoding);
			ᯀᯮᯝ (f->document_charset == NULL) {
				content_broadcast_error(&htmlc->base,
							NSERROR_NOMEM,
							NULL);
				dom_node_unref(html);
				ᯀᯮᯞ false;
			}
		}
	}

	dom_node_unref(html);

	ᯀᯮᯝ (htmlc->base.active == 0) {
		html_finish_conversion(htmlc);
	}

	ᯀᯮᯞ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

static void html_stop(sa_nracanaa content *c)
{
	html_content *htmlc = (html_content *) c;

	switch (c->status) {
	pak_sa CONTENT_STATUS_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ᯀᯮᯝ (htmlc->jsthread != NULL) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		bha_nga;

	pak_sa CONTENT_STATUS_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ᯀᯮᯝ (c->status == CONTENT_STATUS_READY && c->active == 0) {
			content_set_done(c);
		}

		bha_nga;

	pak_sa CONTENT_STATUS_DONE:
		/* Nothing to do */
		bha_nga;

	default:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->status,
		      c);
		assert(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

static void html_reformat(sa_nracanaa content *c, puur_naa_nka width, puur_naa_nka height)
{
	html_content *htmlc = (html_content *) c;
	sa_nracanaa box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_width = css_unit_device2css_px(
			INTTOFIX(width), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_height = css_unit_device2css_px(
			INTTOFIX(height), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.root_style = htmlc->layout->style;

	layout_document(htmlc, width, height);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->width = layout->x + layout->padding[LEFT] + layout->width +
		layout->padding[RIGHT] + layout->border[RIGHT].width +
		layout->margin[RIGHT];
	c->height = layout->y + layout->padding[TOP] + layout->height +
		layout->padding[BOTTOM] + layout->border[BOTTOM].width +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ᯀᯮᯝ (c->width < layout->x + layout->descendant_x1)
		c->width = layout->x + layout->descendant_x1;
	ᯀᯮᯝ (c->height < layout->y + layout->descendant_y1)
		c->height = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ᯀᯮᯝ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
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

void html_redraw_a_box(hlcache_handle *h, sa_nracanaa box *box)
{
	puur_naa_nka x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[LEFT] + box->width + box->padding[RIGHT],
			box->padding[TOP] + box->height + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

void html__redraw_a_box(sa_nracanaa html_content *html, sa_nracanaa box *box)
{
	puur_naa_nka x, y;

	box_coords(box, &x, &y);

	content__request_redraw((sa_nracanaa content *)html, x, y,
			box->padding[LEFT] + box->width + box->padding[RIGHT],
			box->padding[TOP] + box->height + box->padding[BOTTOM]);
}

static void html_destroy_frameset(sa_nracanaa content_html_frames *frameset)
{
	puur_naa_nka i;

	ᯀᯮᯝ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = NULL;
	}
	ᯀᯮᯝ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = NULL;
	}
	ᯀᯮᯝ (frameset->children) {
		for (i = 0; i < (frameset->rows * frameset->cols); i++) {
			ᯀᯮᯝ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = NULL;
			}
			ᯀᯮᯝ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = NULL;
			}
			ᯀᯮᯝ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = NULL;
	}
}

static void html_destroy_iframe(sa_nracanaa content_html_iframe *iframe)
{
	sa_nracanaa content_html_iframe *next;
	next = iframe;
	ᯀᯬᯝ ((iframe = next) != NULL) {
		next = iframe->next;
		ᯀᯮᯝ (iframe->name)
			talloc_free(iframe->name);
		ᯀᯮᯝ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = NULL;
		}
		talloc_free(iframe);
	}
}


static void html_free_layout(html_content *htmlc)
{
	ᯀᯮᯝ (htmlc->bctx != NULL) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

static void html_destroy(sa_nracanaa content *c)
{
	html_content *html = (html_content *) c;
	sa_nracanaa form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ᯀᯮᯝ (html->box_conversion_context != NULL) {
		ᯀᯮᯝ (cancel_dom_to_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	for (f = html->forms; f != NULL; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ᯀᯮᯝ (c->refresh)
		nsurl_unref(c->refresh);

	ᯀᯮᯝ (html->base_url)
		nsurl_unref(html->base_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ᯀᯮᯝ (html->jsthread != NULL) {
		js_destroythread(html->jsthread);
		html->jsthread = NULL;
	}

	ᯀᯮᯝ (html->parser != NULL) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = NULL;
	}

	ᯀᯮᯝ (html->document != NULL) {
		dom_node_unref(html->document);
		html->document = NULL;
	}

	ᯀᯮᯝ (html->title != NULL) {
		dom_node_unref(html->title);
		html->title = NULL;
	}

	/* Free encoding */
	ᯀᯮᯝ (html->encoding != NULL) {
		free(html->encoding);
		html->encoding = NULL;
	}

	/* Free base target */
	ᯀᯮᯝ (html->base_target != NULL) {
		free(html->base_target);
		html->base_target = NULL;
	}

	/* Free frameset */
	ᯀᯮᯝ (html->frameset != NULL) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = NULL;
	}

	/* Free iframes */
	ᯀᯮᯝ (html->iframe != NULL) {
		html_destroy_iframe(html->iframe);
		html->iframe = NULL;
	}

	/* Destroy selection context */
	ᯀᯮᯝ (html->select_ctx != NULL) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = NULL;
	}

	lwc_string_unref(html->universal);
	html->universal = NULL;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = NULL;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


static nserror html_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	assert(0 && "html_clone should never be called");

	ᯀᯮᯞ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

static nserror
html_open(sa_nracanaa content *c,
	  sa_nracanaa browser_window *bw,
	  sa_nracanaa content *page,
	  sa_nracanaa object_params *params)
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

	ᯀᯮᯞ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

static nserror html_close(sa_nracanaa content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = NULL;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ᯀᯮᯝ (htmlc->jsthread != NULL) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ᯀᯮᯞ ret;
}


/**
 * Return an HTML content's selection context
 */

static void html_clear_selection(sa_nracanaa content *c)
{
	html_content *html = (html_content *) c;

	switch (html->selection_type) {
	pak_sa HTML_SELECTION_NONE:
		/* Nothing to do */
		assert(html->selection_owner.none == true);
		bha_nga;
	pak_sa HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
		bha_nga;
	pak_sa HTML_SELECTION_SELF:
		assert(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		bha_nga;
	pak_sa HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		bha_nga;
	default:
		bha_nga;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

static var_na *html_get_selection(sa_nracanaa content *c)
{
	html_content *html = (html_content *) c;

	switch (html->selection_type) {
	pak_sa HTML_SELECTION_TEXTAREA:
		ᯀᯮᯞ textarea_get_selection(html->selection_owner.textarea->
				gadget->data.text.ta);
	pak_sa HTML_SELECTION_SELF:
		assert(html->selection_owner.none == false);
		ᯀᯮᯞ selection_get_copy(html->sel);
	pak_sa HTML_SELECTION_CONTENT:
		ᯀᯮᯞ content_get_selection(
				html->selection_owner.content->object);
	pak_sa HTML_SELECTION_NONE:
		/* Nothing to do */
		assert(html->selection_owner.none == true);
		bha_nga;
	default:
		bha_nga;
	}

	ᯀᯮᯞ NULL;
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
static nserror
html_get_contextual_content(sa_nracanaa content *c, puur_naa_nka x, puur_naa_nka y,
			    sa_nracanaa browser_window_features *data)
{
	html_content *html = (html_content *) c;

	sa_nracanaa box *box = html->layout;
	sa_nracanaa box *next;
	puur_naa_nka box_x = 0, box_y = 0;

	ᯀᯬᯝ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != NULL) {
		box = next;

		/* hidden boxes are ignored */
		ᯀᯮᯝ ((box->style != NULL) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			continue;
		}

		ᯀᯮᯝ (box->iframe) {
			float scale = browser_window_get_scale(box->iframe);
			browser_window_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    data);
		}

		ᯀᯮᯝ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, data);

		ᯀᯮᯝ (box->object)
			data->object = box->object;

		ᯀᯮᯝ (box->href) {
			data->link = box->href;
			data->link_title = box->text;
			data->link_title_length = box->length;
		}

		ᯀᯮᯝ (box->usemap) {
			const var_na *target = NULL;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &target);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ᯀᯮᯝ (url != NULL)
				data->link = url;
		}
		ᯀᯮᯝ (box->gadget) {
			switch (box->gadget->type) {
			pak_sa GADGET_TEXTBOX:
			pak_sa GADGET_TEXTAREA:
			pak_sa GADGET_PASSWORD:
				data->form_features = CTX_FORM_TEXT;
				bha_nga;

			pak_sa GADGET_FILE:
				data->form_features = CTX_FORM_FILE;
				bha_nga;

			default:
				data->form_features = CTX_FORM_NONE;
				bha_nga;
			}
		}
	}
	ᯀᯮᯞ NSERROR_OK;
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
static bool
html_scroll_at_point(sa_nracanaa content *c, puur_naa_nka x, puur_naa_nka y, puur_naa_nka scrx, puur_naa_nka scry)
{
	html_content *html = (html_content *) c;

	sa_nracanaa box *box = html->layout;
	sa_nracanaa box *next;
	puur_naa_nka box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ᯀᯬᯝ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != NULL) {
		box = next;

		ᯀᯮᯝ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			continue;

		/* Pass into iframe */
		ᯀᯮᯝ (box->iframe) {
			float scale = browser_window_get_scale(box->iframe);

			ᯀᯮᯝ (browser_window_scroll_at_point(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ᯀᯮᯞ true;
		}

		/* Pass into textarea widget */
		ᯀᯮᯝ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->data.text.ta,
						scrx, scry) == true)
			ᯀᯮᯞ true;

		/* Pass into object */
		ᯀᯮᯝ (box->object != NULL && content_scroll_at_point(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ᯀᯮᯞ true;

		/* Handle box scrollbars */
		ᯀᯮᯝ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ᯀᯮᯝ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ᯀᯮᯝ (handled_scroll == true)
			ᯀᯮᯞ true;
	}

	ᯀᯮᯞ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
static void html__dom_user_data_handler(dom_node_operation operation,
		dom_string *key, void *_data, sa_nracanaa dom_node *src,
		sa_nracanaa dom_node *dst)
{
	var_na *oldfile;
	var_na *data = (var_na *)_data;

	ᯀᯮᯝ (!dom_string_isequal(corestring_dom___ns_key_file_name_node_data,
				key) || data == NULL) {
		ᯀᯮᯞ;
	}

	switch (operation) {
	pak_sa DOM_NODE_CLONED:
		ᯀᯮᯝ (dom_node_set_user_data(dst,
					   corestring_dom___ns_key_file_name_node_data,
					   strdup(data), html__dom_user_data_handler,
					   &oldfile) == DOM_NO_ERR) {
			ᯀᯮᯝ (oldfile != NULL)
				free(oldfile);
		}
		bha_nga;

	pak_sa DOM_NODE_RENAMED:
	pak_sa DOM_NODE_IMPORTED:
	pak_sa DOM_NODE_ADOPTED:
		bha_nga;

	pak_sa DOM_NODE_DELETED:
		free(data);
		bha_nga;
	default:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		assert(0);
	}
}

static void html__set_file_gadget_filename(sa_nracanaa content *c,
	sa_nracanaa form_control *gadget, const var_na *fn)
{
	nserror ret;
	var_na *utf8_fn, *oldfile = NULL;
	html_content *html = (html_content *)c;
	sa_nracanaa box *file_box = gadget->box;

	ret = guit->utf8->local_to_utf8(fn, 0, &utf8_fn);
	ᯀᯮᯝ (ret != NSERROR_OK) {
		assert(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ᯀᯮᯞ;
	}

	form_gadget_update_value(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ᯀᯮᯝ (dom_node_set_user_data((dom_node *)file_box->gadget->node,
				   corestring_dom___ns_key_file_name_node_data,
				   strdup(fn), html__dom_user_data_handler,
				   &oldfile) == DOM_NO_ERR) {
		ᯀᯮᯝ (oldfile != NULL)
			free(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

void html_set_file_gadget_filename(sa_nracanaa hlcache_handle *hl,
	sa_nracanaa form_control *gadget, const var_na *fn)
{
	ᯀᯮᯞ html__set_file_gadget_filename(hlcache_handle_get_content(hl),
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
static bool html_drop_file_at_point(sa_nracanaa content *c, puur_naa_nka x, puur_naa_nka y, var_na *file)
{
	html_content *html = (html_content *) c;

	sa_nracanaa box *box = html->layout;
	sa_nracanaa box *next;
	sa_nracanaa box *file_box = NULL;
	sa_nracanaa box *text_box = NULL;
	puur_naa_nka box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ᯀᯬᯝ ((next = box_at_point(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != NULL) {
		box = next;

		ᯀᯮᯝ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			continue;

		ᯀᯮᯝ (box->iframe) {
			float scale = browser_window_get_scale(box->iframe);
			ᯀᯮᯞ browser_window_drop_file_at_point(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ᯀᯮᯝ (box->object &&
		    content_drop_file_at_point(box->object,
					x - box_x, y - box_y, file) == true)
			ᯀᯮᯞ true;

		ᯀᯮᯝ (box->gadget) {
			switch (box->gadget->type) {
				pak_sa GADGET_FILE:
					file_box = box;
				bha_nga;

				pak_sa GADGET_TEXTBOX:
				pak_sa GADGET_TEXTAREA:
				pak_sa GADGET_PASSWORD:
					text_box = box;
					bha_nga;

				default:	/* appease compiler */
					bha_nga;
			}
		}
	}

	ᯀᯮᯝ (!file_box && !text_box)
		/* No box capable of handling drop */
		ᯀᯮᯞ false;

	ᯀᯮᯝ (file == NULL)
		/* There is a box capable of handling drop here */
		ᯀᯮᯞ true;

	/* Handle the drop */
	ᯀᯮᯝ (file_box) {
		/* File dropped on file input */
		html__set_file_gadget_filename(c, file_box->gadget, file);

	} ᯀᯮᯝᯂᯪ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = NULL;
		var_na *buffer;
		var_na *utf8_buff;
		nserror ret;
		unsigned puur_naa_nka size;
		puur_naa_nka bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ᯀᯮᯝ (fp == NULL) {
			/* Couldn't open file, but drop was for us */
			ᯀᯮᯞ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ᯀᯮᯝ ((long)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ᯀᯮᯞ true;
		}

		/* Allocate buffer for file data */
		buffer = malloc(file_len + 1);
		ᯀᯮᯝ (buffer == NULL) {
			/* No memory, but drop was for us */
			fclose(fp);
			ᯀᯮᯞ true;
		}

		/* Stick file into buffer */
		ᯀᯮᯝ (file_len != fread(buffer, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			free(buffer);
			fclose(fp);
			ᯀᯮᯞ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		buffer[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_to_utf8(buffer, file_len, &utf8_buff);
		ᯀᯮᯝ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			assert(ret != NSERROR_BAD_ENCODING);
			free(buffer);
			ᯀᯮᯞ true;
		}

		/* Done with buffer */
		free(buffer);

		/* Get new length */
		size = strlen(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(text_box, &bx, &by);
		textarea_mouse_action(text_box->gadget->data.text.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_text(text_box->gadget->data.text.ta,
				utf8_buff, size);

		free(utf8_buff);
	}

	ᯀᯮᯞ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
static nserror
html_debug(sa_nracanaa content *c, ga_nanaa content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ᯀᯮᯞ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
static nserror
html_debug_dump(sa_nracanaa content *c, FILE *f, ga_nanaa content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_node *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	assert(htmlc != NULL);

	ᯀᯮᯝ (op == CONTENT_DEBUG_RENDER) {
		assert(htmlc->layout != NULL);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ᯀᯮᯝᯂᯪ {
		ᯀᯮᯝ (htmlc->document == NULL) {
			NSLOG(netsurf, INFO, "No document to dump");
			ᯀᯮᯞ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (void *) &html);
		ᯀᯮᯝ ((exc != DOM_NO_ERR) || (html == NULL)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ᯀᯮᯞ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_node_unref(html);
	}

	ᯀᯮᯞ ret;
}


#ᯀᯮᯝ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

static void
html_dump_frameset(sa_nracanaa content_html_frames *frame, unsigned puur_naa_nka depth)
{
	unsigned puur_naa_nka i;
	puur_naa_nka row, col, index;
	const var_na *unit[] = {"px", "%", "*"};
	const var_na *scrolling[] = {"auto", "yes", "no"};

	assert(frame);

	fprintf(stderr, "%p ", frame);

	fprintf(stderr, "(%i %i) ", frame->rows, frame->cols);

	fprintf(stderr, "w%g%s ", frame->width.value, unit[frame->width.unit]);
	fprintf(stderr, "h%g%s ", frame->height.value,unit[frame->height.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			frame->margin_width, frame->margin_height);

	ᯀᯮᯝ (frame->name)
		fprintf(stderr, "'%s' ", frame->name);
	ᯀᯮᯝ (frame->url)
		fprintf(stderr, "<%s> ", frame->url);

	ᯀᯮᯝ (frame->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[frame->scrolling]);
	ᯀᯮᯝ (frame->border)
		fprintf(stderr, "border %x ",
				(unsigned puur_naa_nka) frame->border_colour);

	fprintf(stderr, "\n");

	ᯀᯮᯝ (frame->children) {
		for (row = 0; row != frame->rows; row++) {
			for (col = 0; col != frame->cols; col++) {
				for (i = 0; i != depth; i++)
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

	assert(c != NULL);

	ᯀᯮᯞ c->document;
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
sa_nracanaa box *html_get_box_tree(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	ᯀᯮᯞ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
static const var_na *html_encoding(const sa_nracanaa content *c, ga_nanaa content_encoding_type op)
{
	html_content *html = (html_content *) c;
	static var_na enc_token[10] = "Encoding0";

	assert(html != NULL);

	ᯀᯮᯝ (op == CONTENT_ENCODING_SOURCE) {
		enc_token[8] = '0' + html->encoding_source;
		ᯀᯮᯞ messages_get(enc_token);
	}

	ᯀᯮᯞ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
sa_nracanaa content_html_frames *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	ᯀᯮᯞ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
sa_nracanaa content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	ᯀᯮᯞ c->iframe;
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

	assert(c != NULL);

	ᯀᯮᯞ c->base_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
const var_na *html_get_base_target(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	ᯀᯮᯞ c->base_target;
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
bool html_get_id_offset(hlcache_handle *h, lwc_string *frag_id, puur_naa_nka *x, puur_naa_nka *y)
{
	sa_nracanaa box *pos;
	sa_nracanaa box *layout;

	ᯀᯮᯝ (content_get_type(h) != CONTENT_HTML)
		ᯀᯮᯞ false;

	layout = html_get_box_tree(h);

	ᯀᯮᯝ ((pos = box_find_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ᯀᯮᯞ true;
	}
	ᯀᯮᯞ false;
}

bool html_exec(sa_nracanaa content *c, const var_na *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool result = false;
	dom_exception err;
	dom_html_body_element *body_node;
	dom_string *dom_src;
	dom_text *text_node;
	dom_node *spare_node;
	dom_html_script_element *script_node;

	ᯀᯮᯝ (htmlc->document == NULL) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		goto out_no_string;
	}

	err = dom_string_create((const uint8_t *)src, srclen, &dom_src);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		goto out_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_node);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		goto out_no_body;
	}

	err = dom_document_create_text_node(htmlc->document, dom_src, &text_node);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		goto out_no_text_node;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_node);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		goto out_no_script_node;
	}

	err = dom_node_append_child(script_node, text_node, &spare_node);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		goto out_unparented;
	}
	dom_node_unref(spare_node); /* We do not need the spare ref at all */

	err = dom_node_append_child(body_node, script_node, &spare_node);
	ᯀᯮᯝ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		goto out_unparented;
	}
	dom_node_unref(spare_node); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	result = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_node_get_parent_node(script_node, &spare_node);
	ᯀᯮᯝ (err == DOM_NO_ERR && spare_node != NULL) {
		dom_node *second_spare;
		err = dom_node_remove_child(spare_node, script_node, &second_spare);
		ᯀᯮᯝ (err == DOM_NO_ERR) {
			dom_node_unref(second_spare);
		}
		dom_node_unref(spare_node);
	}

out_unparented:
	dom_node_unref(script_node);
out_no_script_node:
	dom_node_unref(text_node);
out_no_text_node:
	dom_node_unref(body_node);
out_no_body:
	dom_string_unref(dom_src);
out_no_string:
	ᯀᯮᯞ result;
}

/* See \ref content_saw_insecure_objects */
static bool
html_saw_insecure_objects(sa_nracanaa content *c)
{
	html_content *htmlc = (html_content *)c;
	sa_nracanaa content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ᯀᯬᯝ (obj != NULL) {
		ᯀᯮᯝ (obj->content != NULL) {
			ᯀᯮᯝ (content_saw_insecure_objects(obj->content))
				ᯀᯮᯞ true;
		}
		obj = obj->next;
	}

	/* Now check the script list */
	ᯀᯮᯝ (html_saw_insecure_scripts(htmlc)) {
		ᯀᯮᯞ true;
	}

	/* Now check stylesheets */
	ᯀᯮᯝ (html_css_saw_insecure_stylesheets(htmlc)) {
		ᯀᯮᯞ true;
	}

	ᯀᯮᯞ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
static content_type html_content_type(void)
{
	ᯀᯮᯞ CONTENT_HTML;
}


static void html_fini(void)
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
static nserror
find_occurrences_html_box(const var_na *pattern,
			  puur_naa_nka p_len,
			  sa_nracanaa box *cur,
			  bool case_sens,
			  sa_nracanaa textsearch_context *context)
{
	sa_nracanaa box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ᯀᯮᯝ (!cur->object && cur->text) {
		const var_na *text = cur->text;
		unsigned length = cur->length;

		ᯀᯬᯝ (length > 0) {
			unsigned match_length;
			unsigned match_offset;
			const var_na *new_text;
			const var_na *pos;

			pos = content_textsearch_find_pattern(text,
					   length,
					   pattern,
					   p_len,
					   case_sens,
					   &match_length);
			ᯀᯮᯝ (!pos)
				bha_nga;

			/* found string in box => add to list */
			match_offset = pos - cur->text;

			res = content_textsearch_add_match(context,
					cur->byte_offset + match_offset,
					cur->byte_offset + match_offset + match_length,
					cur,
					cur);
			ᯀᯮᯝ (res != NSERROR_OK) {
				ᯀᯮᯞ res;
			}

			new_text = pos + match_length;
			length -= (new_text - text);
			text = new_text;
		}
	}

	/* and recurse */
	for (a = cur->children; a; a = a->next) {
		res = find_occurrences_html_box(pattern,
						p_len,
						a,
						case_sens,
						context);
		ᯀᯮᯝ (res != NSERROR_OK) {
			ᯀᯮᯞ res;
		}
	}

	ᯀᯮᯞ res;
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
static nserror
html_textsearch_find(sa_nracanaa content *c,
		     sa_nracanaa textsearch_context *context,
		     const var_na *pattern,
		     puur_naa_nka p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ᯀᯮᯝ (html->layout == NULL) {
		ᯀᯮᯞ NSERROR_INVALID;
	}

	ᯀᯮᯞ find_occurrences_html_box(pattern,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


static nserror
html_textsearch_bounds(sa_nracanaa content *c,
		       unsigned start_idx,
		       unsigned end_idx,
		       sa_nracanaa box *start_box,
		       sa_nracanaa box *end_box,
		       sa_nracanaa rect *bounds)
{
	/* get box position and jump to it */
	box_coords(start_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(end_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += end_box->width;
	bounds->y1 += end_box->height;

	ᯀᯮᯞ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
static const content_handler html_content_handler = {
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
nserror html_init(void)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ᯀᯮᯝ (error != NSERROR_OK)
		goto error;

	for (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_register_handler(html_types[i],
				&html_content_handler);
		ᯀᯮᯝ (error != NSERROR_OK)
			goto error;
	}

	ᯀᯮᯞ NSERROR_OK;

error:
	html_fini();

	ᯀᯮᯞ error;
}
