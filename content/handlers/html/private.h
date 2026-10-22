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

#അന്തര്ഭാവ <dom/bindings/hubbub/parser.h>

#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/handlers/css/utils.h"


ഘടന gui_layout_table;
ഘടന scrollbar_സന്ദേശം_data;
ഘടന content_redraw_data;
ഘടന selection;

തരനിർവചനം എണ്ണൽ {
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
സംഘം html_drag_owner {
	bool no_owner;
	ഘടന box *content;
	ഘടന scrollbar *scrollbar;
	ഘടന box *textarea;
};

തരനിർവചനം എണ്ണൽ {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
സംഘം html_selection_owner {
	bool none;
	ഘടന box *textarea;
	ഘടന box *content;
};

തരനിർവചനം എണ്ണൽ {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
സംഘം html_focus_owner {
	bool self;
	ഘടന box *textarea;
	ഘടന box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
തരനിർവചനം ഘടന html_content {
	ഘടന content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	അക്ഷരം *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	ഘടന nsurl *base_url;
	/** Base target */
	അക്ഷരം *base_target;

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
	dom_കണ്ണി *title;

	/** A talloc context purely for the render box tree */
	പൂർണ്ണസംഖ്യ *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	ശൂന്യം *box_conversion_context;
	/** Box tree, or NULL. */
	ഘടന box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	സ്ഥിരം ഘടന gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned പൂർണ്ണസംഖ്യ scripts_എണ്ണം;
	/** Scripts */
	ഘടന html_script *scripts;
	/** javascript thread in use */
	ഘടന jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned പൂർണ്ണസംഖ്യ stylesheet_എണ്ണം;
	/** Stylesheets. Each may be NULL. */
	ഘടന html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned പൂർണ്ണസംഖ്യ അക്കം_objects;
	/** List of objects. */
	ഘടന content_html_object *object_list;
	/** Forms, in reverse order to document. */
	ഘടന form *forms;
	/** Hash table of imagemaps. */
	ഘടന imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	ഘടന browser_window *bw;

	/** Frameset information */
	ഘടന content_html_frames *frameset;

	/** Inline frame information */
	ഘടന content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	ഘടന html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	സംഘം html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	സംഘം html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	സംഘം html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	ഘടന selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	ഘടന form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
ബാഹ്യ bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
ശൂന്യം html__redraw_a_box(html_content *htmlc, ഘടന box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
ശൂന്യം html_finish_conversion(html_content *htmlc);


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
bool html_exec(ഘടന content *c, സ്ഥിരം അക്ഷരം *src, size_t srclen);


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
bool html_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(ഘടന box *box, പൂർണ്ണസംഖ്യ x_parent, പൂർണ്ണസംഖ്യ y_parent,
		പൂർണ്ണസംഖ്യ p_width, പൂർണ്ണസംഖ്യ p_height, സ്ഥിരം ഘടന rect *clip, ദശാംശം scale,
		സ്ഥിരം ഘടന redraw_context *ctx);


bool html_redraw_inline_borders(ഘടന box *box, ഘടന rect b,
		സ്ഥിരം ഘടന rect *clip, ദശാംശം scale, bool first, bool last,
		സ്ഥിരം ഘടന redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(ശൂന്യം *ctx, dom_കണ്ണി *കണ്ണി);


/* in html/forms.c */
ഘടന form *html_forms_get_forms(സ്ഥിരം അക്ഷരം *docenc, dom_html_document *doc);
ഘടന form_control *html_forms_get_control_for_കണ്ണി(ഘടന form *forms,
		dom_കണ്ണി *കണ്ണി);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(ശൂന്യം);
nserror html_css_fetcher_add_item(dom_string *data, ഘടന nsurl *base_url,
		uint32_t *താക്കോൽ);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_കണ്ണി *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_കണ്ണി *target,
		bool bubbles, bool cancelable, uint32_t താക്കോൽ);

/* Useful dom_string pointers */
ഘടന dom_string;

ബാഹ്യ ഘടന dom_string *html_dom_string_map;
ബാഹ്യ ഘടന dom_string *html_dom_string_id;
ബാഹ്യ ഘടന dom_string *html_dom_string_name;
ബാഹ്യ ഘടന dom_string *html_dom_string_area;
ബാഹ്യ ഘടന dom_string *html_dom_string_a;
ബാഹ്യ ഘടന dom_string *html_dom_string_nohref;
ബാഹ്യ ഘടന dom_string *html_dom_string_href;
ബാഹ്യ ഘടന dom_string *html_dom_string_target;
ബാഹ്യ ഘടന dom_string *html_dom_string_shape;
ബാഹ്യ ഘടന dom_string *html_dom_string_default;
ബാഹ്യ ഘടന dom_string *html_dom_string_rect;
ബാഹ്യ ഘടന dom_string *html_dom_string_rectangle;
ബാഹ്യ ഘടന dom_string *html_dom_string_coords;
ബാഹ്യ ഘടന dom_string *html_dom_string_circle;
ബാഹ്യ ഘടന dom_string *html_dom_string_poly;
ബാഹ്യ ഘടന dom_string *html_dom_string_polygon;
ബാഹ്യ ഘടന dom_string *html_dom_string_text_javascript;
ബാഹ്യ ഘടന dom_string *html_dom_string_type;
ബാഹ്യ ഘടന dom_string *html_dom_string_src;

#endif
