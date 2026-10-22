/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
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
 * Private data for text/html content.
 */

#ifndef NETSURF_HTML_PRIVATE_H
#define NETSURF_HTML_PRIVATE_H

#অন্তর্ভাৱ <dom/bindings/hubbub/parser.h>

#অন্তর্ভাৱ "netsurf/types.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/handlers/css/utils.h"


সংরচনা gui_layout_table;
সংরচনা scrollbar_বাৰ্তা_data;
সংরচনা content_redraw_data;
সংরচনা selection;

প্রকারনির্দেশ গণনা {
	HTML_DRAG_NONE,			/** No drag */
	HTML_DRAG_SELECTION,		/** Own; Text selection */
	HTML_DRAG_SCROLLBAR,		/** Not own; drag in scrollbar widget */
	HTML_DRAG_TEXTAREA_SELECTION,	/** Not own; drag in textarea widget */
	HTML_DRAG_TEXTAREA_SCROLLBAR,	/** Not own; drag in textarea widget */
	HTML_DRAG_CONTENT_SELECTION,	/** Not own; drag in child content */
	HTML_DRAG_CONTENT_SCROLL	/** Not own; drag in child content */
} html_drag_type;

/**
 * For drags we don't own
 */
সঙ্ঘ html_drag_owner {
	bool no_owner;
	সংরচনা box *content;
	সংরচনা scrollbar *scrollbar;
	সংরচনা box *textarea;
};

প্রকারনির্দেশ গণনা {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
সঙ্ঘ html_selection_owner {
	bool none;
	সংরচনা box *textarea;
	সংরচনা box *content;
};

প্রকারনির্দেশ গণনা {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
সঙ্ঘ html_focus_owner {
	bool self;
	সংরচনা box *textarea;
	সংরচনা box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
প্রকারনির্দেশ সংরচনা html_content {
	সংরচনা content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	ৱর্ণ *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	সংরচনা nsurl *base_url;
	/** Base target */
	ৱর্ণ *base_target;

	/** Content has been aborted in the LOADING state */
	bool aborted;

	/** Whether a meta refresh has been handled */
	bool refresh;

	/** Whether a layout (reflow) is in progress */
	bool reflowing;

	/** Whether an initial layout has been done */
	bool had_initial_layout;

	/** Whether scripts are enabled for this content */
	bool enable_scripting;

	/* Title element node */
	dom_গ্ৰন্থি *title;

	/** A talloc context purely for the render box tree */
	পূর্ণাঙ্ক *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	শূন্য *box_conversion_context;
	/** Box tree, or NULL. */
	সংরচনা box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	ধ্রুৱ সংরচনা gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned পূর্ণাঙ্ক scripts_গুনতি;
	/** Scripts */
	সংরচনা html_script *scripts;
	/** javascript thread in use */
	সংরচনা jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned পূর্ণাঙ্ক stylesheet_গুনতি;
	/** Stylesheets. Each may be NULL. */
	সংরচনা html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned পূর্ণাঙ্ক অঙ্ক_objects;
	/** List of objects. */
	সংরচনা content_html_object *object_list;
	/** Forms, in reverse order to document. */
	সংরচনা form *forms;
	/** Hash table of imagemaps. */
	সংরচনা imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	সংরচনা browser_window *bw;

	/** Frameset information */
	সংরচনা content_html_frames *frameset;

	/** Inline frame information */
	সংরচনা content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	সংরচনা html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	সঙ্ঘ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	সঙ্ঘ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	সঙ্ঘ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	সংরচনা selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	সংরচনা form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
বাহ্য bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
শূন্য html__redraw_a_box(html_content *htmlc, সংরচনা box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
শূন্য html_finish_conversion(html_content *htmlc);


/**
 * Test if an HTML content conversion can begin
 *
 * \param htmlc		html content to test
 * \return true iff the html content conversion can begin
 */
bool html_can_begin_conversion(html_content *htmlc);


/**
 * Begin conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
bool html_begin_conversion(html_content *htmlc);


/**
 * execute some text as a script element
 */
bool html_exec(সংরচনা content *c, ধ্রুৱ ৱর্ণ *src, size_t srclen);


/**
 * Attempt script execution for defer and async scripts
 *
 * execute scripts using algorithm found in:
 * http://www.whatwg.org/specs/web-apps/current-work/multipage/scripting-1.html#the-script-element
 *
 * \param htmlc html content.
 * \param allow_defer allow deferred execution, if not, only async scripts.
 * \return NSERROR_OK error code.
 */
nserror html_script_exec(html_content *htmlc, bool allow_defer);


/**
 * Free all script resources and references for a html content.
 *
 * \param htmlc html content.
 * \return NSERROR_OK or error code.
 */
nserror html_script_free(html_content *htmlc);


/**
 * Check if any of the scripts loaded were insecure
 */
bool html_saw_insecure_scripts(html_content *htmlc);


/**
 * Complete the HTML content state machine *iff* all scripts are finished
 */
nserror html_proceed_to_done(html_content *html);


/* in html/redraw.c */
bool html_redraw(সংরচনা content *c, সংরচনা content_redraw_data *data,
		ধ্রুৱ সংরচনা rect *clip, ধ্রুৱ সংরচনা redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(সংরচনা box *box, পূর্ণাঙ্ক x_parent, পূর্ণাঙ্ক y_parent,
		পূর্ণাঙ্ক p_width, পূর্ণাঙ্ক p_height, ধ্রুৱ সংরচনা rect *clip, ভিন্ন scale,
		ধ্রুৱ সংরচনা redraw_context *ctx);


bool html_redraw_inline_borders(সংরচনা box *box, সংরচনা rect b,
		ধ্রুৱ সংরচনা rect *clip, ভিন্ন scale, bool first, bool last,
		ধ্রুৱ সংরচনা redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(শূন্য *ctx, dom_গ্ৰন্থি *গ্ৰন্থি);


/* in html/forms.c */
সংরচনা form *html_forms_get_forms(ধ্রুৱ ৱর্ণ *docenc, dom_html_document *doc);
সংরচনা form_control *html_forms_get_control_for_গ্ৰন্থি(সংরচনা form *forms,
		dom_গ্ৰন্থি *গ্ৰন্থি);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(শূন্য);
nserror html_css_fetcher_add_item(dom_string *data, সংরচনা nsurl *base_url,
		uint32_t *চাবি);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_গ্ৰন্থি *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_গ্ৰন্থি *target,
		bool bubbles, bool cancelable, uint32_t চাবি);

/* Useful dom_string pointers */
সংরচনা dom_string;

বাহ্য সংরচনা dom_string *html_dom_string_map;
বাহ্য সংরচনা dom_string *html_dom_string_id;
বাহ্য সংরচনা dom_string *html_dom_string_name;
বাহ্য সংরচনা dom_string *html_dom_string_area;
বাহ্য সংরচনা dom_string *html_dom_string_a;
বাহ্য সংরচনা dom_string *html_dom_string_nohref;
বাহ্য সংরচনা dom_string *html_dom_string_href;
বাহ্য সংরচনা dom_string *html_dom_string_target;
বাহ্য সংরচনা dom_string *html_dom_string_shape;
বাহ্য সংরচনা dom_string *html_dom_string_default;
বাহ্য সংরচনা dom_string *html_dom_string_rect;
বাহ্য সংরচনা dom_string *html_dom_string_rectangle;
বাহ্য সংরচনা dom_string *html_dom_string_coords;
বাহ্য সংরচনা dom_string *html_dom_string_circle;
বাহ্য সংরচনা dom_string *html_dom_string_poly;
বাহ্য সংরচনা dom_string *html_dom_string_polygon;
বাহ্য সংরচনা dom_string *html_dom_string_text_javascript;
বাহ্য সংরচনা dom_string *html_dom_string_type;
বাহ্য সংরচনা dom_string *html_dom_string_src;

#endif
