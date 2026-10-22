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

#அந்தர்பாவ <dom/bindings/hubbub/parser.h>

#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/handlers/css/utils.h"


கட்டமைப்பு gui_layout_table;
கட்டமைப்பு scrollbar_செய்தி_data;
கட்டமைப்பு content_redraw_data;
கட்டமைப்பு selection;

வகைவரை எண்ணிக்கை {
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
ஒன்றிப்பு html_drag_owner {
	bool no_owner;
	கட்டமைப்பு box *content;
	கட்டமைப்பு scrollbar *scrollbar;
	கட்டமைப்பு box *textarea;
};

வகைவரை எண்ணிக்கை {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
ஒன்றிப்பு html_selection_owner {
	bool none;
	கட்டமைப்பு box *textarea;
	கட்டமைப்பு box *content;
};

வகைவரை எண்ணிக்கை {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
ஒன்றிப்பு html_focus_owner {
	bool self;
	கட்டமைப்பு box *textarea;
	கட்டமைப்பு box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
வகைவரை கட்டமைப்பு html_content {
	கட்டமைப்பு content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	எழுத்து *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	கட்டமைப்பு nsurl *base_url;
	/** Base target */
	எழுத்து *base_target;

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
	dom_முனை *title;

	/** A talloc context purely for the render box tree */
	முழு *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	வெறுமை *box_conversion_context;
	/** Box tree, or NULL. */
	கட்டமைப்பு box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	மாறிலி கட்டமைப்பு gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned முழு scripts_மொத்தம்;
	/** Scripts */
	கட்டமைப்பு html_script *scripts;
	/** javascript thread in use */
	கட்டமைப்பு jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned முழு stylesheet_மொத்தம்;
	/** Stylesheets. Each may be NULL. */
	கட்டமைப்பு html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned முழு இலக்கம்_objects;
	/** List of objects. */
	கட்டமைப்பு content_html_object *object_list;
	/** Forms, in reverse order to document. */
	கட்டமைப்பு form *forms;
	/** Hash table of imagemaps. */
	கட்டமைப்பு imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	கட்டமைப்பு browser_window *bw;

	/** Frameset information */
	கட்டமைப்பு content_html_frames *frameset;

	/** Inline frame information */
	கட்டமைப்பு content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	கட்டமைப்பு html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	ஒன்றிப்பு html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	ஒன்றிப்பு html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	ஒன்றிப்பு html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	கட்டமைப்பு selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	கட்டமைப்பு form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
வெளி bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
வெறுமை html__redraw_a_box(html_content *htmlc, கட்டமைப்பு box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
வெறுமை html_finish_conversion(html_content *htmlc);


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
bool html_exec(கட்டமைப்பு content *c, மாறிலி எழுத்து *src, size_t srclen);


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
bool html_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(கட்டமைப்பு box *box, முழு x_parent, முழு y_parent,
		முழு p_width, முழு p_height, மாறிலி கட்டமைப்பு rect *clip, தசம scale,
		மாறிலி கட்டமைப்பு redraw_context *ctx);


bool html_redraw_inline_borders(கட்டமைப்பு box *box, கட்டமைப்பு rect b,
		மாறிலி கட்டமைப்பு rect *clip, தசம scale, bool first, bool last,
		மாறிலி கட்டமைப்பு redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(வெறுமை *ctx, dom_முனை *முனை);


/* in html/forms.c */
கட்டமைப்பு form *html_forms_get_forms(மாறிலி எழுத்து *docenc, dom_html_document *doc);
கட்டமைப்பு form_control *html_forms_get_control_for_முனை(கட்டமைப்பு form *forms,
		dom_முனை *முனை);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(வெறுமை);
nserror html_css_fetcher_add_item(dom_string *data, கட்டமைப்பு nsurl *base_url,
		uint32_t *சாவி);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_முனை *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_முனை *target,
		bool bubbles, bool cancelable, uint32_t சாவி);

/* Useful dom_string pointers */
கட்டமைப்பு dom_string;

வெளி கட்டமைப்பு dom_string *html_dom_string_map;
வெளி கட்டமைப்பு dom_string *html_dom_string_id;
வெளி கட்டமைப்பு dom_string *html_dom_string_name;
வெளி கட்டமைப்பு dom_string *html_dom_string_area;
வெளி கட்டமைப்பு dom_string *html_dom_string_a;
வெளி கட்டமைப்பு dom_string *html_dom_string_nohref;
வெளி கட்டமைப்பு dom_string *html_dom_string_href;
வெளி கட்டமைப்பு dom_string *html_dom_string_target;
வெளி கட்டமைப்பு dom_string *html_dom_string_shape;
வெளி கட்டமைப்பு dom_string *html_dom_string_default;
வெளி கட்டமைப்பு dom_string *html_dom_string_rect;
வெளி கட்டமைப்பு dom_string *html_dom_string_rectangle;
வெளி கட்டமைப்பு dom_string *html_dom_string_coords;
வெளி கட்டமைப்பு dom_string *html_dom_string_circle;
வெளி கட்டமைப்பு dom_string *html_dom_string_poly;
வெளி கட்டமைப்பு dom_string *html_dom_string_polygon;
வெளி கட்டமைப்பு dom_string *html_dom_string_text_javascript;
வெளி கட்டமைப்பு dom_string *html_dom_string_type;
வெளி கட்டமைப்பு dom_string *html_dom_string_src;

#endif
