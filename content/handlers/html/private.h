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

#अन्तर्भाव <dom/bindings/hubbub/parser.h>

#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/handlers/css/utils.h"


संरचना gui_layout_table;
संरचना scrollbar_संदेश_data;
संरचना content_redraw_data;
संरचना selection;

प्रकारनिर्देश गणना {
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
सङ्घ html_drag_owner {
	bool no_owner;
	संरचना box *content;
	संरचना scrollbar *scrollbar;
	संरचना box *textarea;
};

प्रकारनिर्देश गणना {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
सङ्घ html_selection_owner {
	bool none;
	संरचना box *textarea;
	संरचना box *content;
};

प्रकारनिर्देश गणना {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
सङ्घ html_focus_owner {
	bool self;
	संरचना box *textarea;
	संरचना box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
प्रकारनिर्देश संरचना html_content {
	संरचना content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	वर्ण *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	संरचना nsurl *base_url;
	/** Base target */
	वर्ण *base_target;

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
	dom_गाँठ *title;

	/** A talloc context purely for the render box tree */
	पूर्णाङ्क *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	शून्य *box_conversion_context;
	/** Box tree, or NULL. */
	संरचना box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	ध्रुव संरचना gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned पूर्णाङ्क scripts_गिनती;
	/** Scripts */
	संरचना html_script *scripts;
	/** javascript thread in use */
	संरचना jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned पूर्णाङ्क stylesheet_गिनती;
	/** Stylesheets. Each may be NULL. */
	संरचना html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned पूर्णाङ्क अंक_objects;
	/** List of objects. */
	संरचना content_html_object *object_list;
	/** Forms, in reverse order to document. */
	संरचना form *forms;
	/** Hash table of imagemaps. */
	संरचना imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	संरचना browser_window *bw;

	/** Frameset information */
	संरचना content_html_frames *frameset;

	/** Inline frame information */
	संरचना content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	संरचना html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	सङ्घ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	सङ्घ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	सङ्घ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	संरचना selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	संरचना form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
बाह्य bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
शून्य html__redraw_a_box(html_content *htmlc, संरचना box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
शून्य html_finish_conversion(html_content *htmlc);


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
bool html_exec(संरचना content *c, ध्रुव वर्ण *src, size_t srclen);


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
bool html_redraw(संरचना content *c, संरचना content_redraw_data *data,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(संरचना box *box, पूर्णाङ्क x_parent, पूर्णाङ्क y_parent,
		पूर्णाङ्क p_width, पूर्णाङ्क p_height, ध्रुव संरचना rect *clip, भिन्न scale,
		ध्रुव संरचना redraw_context *ctx);


bool html_redraw_inline_borders(संरचना box *box, संरचना rect b,
		ध्रुव संरचना rect *clip, भिन्न scale, bool first, bool last,
		ध्रुव संरचना redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(शून्य *ctx, dom_गाँठ *गाँठ);


/* in html/forms.c */
संरचना form *html_forms_get_forms(ध्रुव वर्ण *docenc, dom_html_document *doc);
संरचना form_control *html_forms_get_control_for_गाँठ(संरचना form *forms,
		dom_गाँठ *गाँठ);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(शून्य);
nserror html_css_fetcher_add_item(dom_string *data, संरचना nsurl *base_url,
		uint32_t *कुंजी);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_गाँठ *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_गाँठ *target,
		bool bubbles, bool cancelable, uint32_t कुंजी);

/* Useful dom_string pointers */
संरचना dom_string;

बाह्य संरचना dom_string *html_dom_string_map;
बाह्य संरचना dom_string *html_dom_string_id;
बाह्य संरचना dom_string *html_dom_string_name;
बाह्य संरचना dom_string *html_dom_string_area;
बाह्य संरचना dom_string *html_dom_string_a;
बाह्य संरचना dom_string *html_dom_string_nohref;
बाह्य संरचना dom_string *html_dom_string_href;
बाह्य संरचना dom_string *html_dom_string_target;
बाह्य संरचना dom_string *html_dom_string_shape;
बाह्य संरचना dom_string *html_dom_string_default;
बाह्य संरचना dom_string *html_dom_string_rect;
बाह्य संरचना dom_string *html_dom_string_rectangle;
बाह्य संरचना dom_string *html_dom_string_coords;
बाह्य संरचना dom_string *html_dom_string_circle;
बाह्य संरचना dom_string *html_dom_string_poly;
बाह्य संरचना dom_string *html_dom_string_polygon;
बाह्य संरचना dom_string *html_dom_string_text_javascript;
बाह्य संरचना dom_string *html_dom_string_type;
बाह्य संरचना dom_string *html_dom_string_src;

#endif
