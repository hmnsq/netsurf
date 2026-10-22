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

#अगर_अघोषित NETSURF_HTML_PRIVATE_H
#घोषणा NETSURF_HTML_PRIVATE_H

#समावेश <dom/bindings/hubbub/parser.h>

#समावेश "netsurf/types.h"
#समावेश "content/content_protected.h"
#समावेश "content/handlers/css/utils.h"


काष्ठा gui_layout_table;
काष्ठा scrollbar_संदेश_तथ्य;
काष्ठा content_redraw_तथ्य;
काष्ठा selection;

प्रकार क्रमागत {
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
जोड़ html_drag_owner {
	bool no_owner;
	काष्ठा box *content;
	काष्ठा scrollbar *scrollbar;
	काष्ठा box *textarea;
};

प्रकार क्रमागत {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
जोड़ html_selection_owner {
	bool none;
	काष्ठा box *textarea;
	काष्ठा box *content;
};

प्रकार क्रमागत {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
जोड़ html_focus_owner {
	bool self;
	काष्ठा box *textarea;
	काष्ठा box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
प्रकार काष्ठा html_content {
	काष्ठा content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	अक्षर *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	काष्ठा nsurl *base_url;
	/** Base target */
	अक्षर *base_target;

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
	पूर्णांक *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	व्योम *box_conversion_context;
	/** Box tree, or NULL. */
	काष्ठा box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	स्थिर काष्ठा gui_layout_table *font_func;

	/** Number of entries in scripts */
	अचिन्हित पूर्णांक scripts_गिनती;
	/** Scripts */
	काष्ठा html_script *scripts;
	/** javascript thread in use */
	काष्ठा jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	अचिन्हित पूर्णांक stylesheet_गिनती;
	/** Stylesheets. Each may be NULL. */
	काष्ठा html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	अचिन्हित पूर्णांक अंक_objects;
	/** List of objects. */
	काष्ठा content_html_object *object_list;
	/** Forms, in reverse order to document. */
	काष्ठा form *forms;
	/** Hash table of imagemaps. */
	काष्ठा imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	काष्ठा browser_window *bw;

	/** Frameset information */
	काष्ठा content_html_frames *frameset;

	/** Inline frame information */
	काष्ठा content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	काष्ठा html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	जोड़ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	जोड़ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	जोड़ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	काष्ठा selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	काष्ठा form_control *visible_select_menu;

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
व्योम html__redraw_a_box(html_content *htmlc, काष्ठा box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
व्योम html_finish_conversion(html_content *htmlc);


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
bool html_exec(काष्ठा content *c, स्थिर अक्षर *src, माप_प्रकार srclen);


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
bool html_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(काष्ठा box *box, पूर्णांक x_parent, पूर्णांक y_parent,
		पूर्णांक p_width, पूर्णांक p_height, स्थिर काष्ठा rect *clip, भग्न scale,
		स्थिर काष्ठा redraw_context *ctx);


bool html_redraw_inline_borders(काष्ठा box *box, काष्ठा rect b,
		स्थिर काष्ठा rect *clip, भग्न scale, bool first, bool last,
		स्थिर काष्ठा redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(व्योम *ctx, dom_गाँठ *गाँठ);


/* in html/forms.c */
काष्ठा form *html_forms_get_forms(स्थिर अक्षर *docenc, dom_html_document *doc);
काष्ठा form_control *html_forms_get_control_for_गाँठ(काष्ठा form *forms,
		dom_गाँठ *गाँठ);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(व्योम);
nserror html_css_fetcher_add_item(dom_string *तथ्य, काष्ठा nsurl *base_url,
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
काष्ठा dom_string;

बाह्य काष्ठा dom_string *html_dom_string_map;
बाह्य काष्ठा dom_string *html_dom_string_id;
बाह्य काष्ठा dom_string *html_dom_string_name;
बाह्य काष्ठा dom_string *html_dom_string_area;
बाह्य काष्ठा dom_string *html_dom_string_a;
बाह्य काष्ठा dom_string *html_dom_string_nohref;
बाह्य काष्ठा dom_string *html_dom_string_href;
बाह्य काष्ठा dom_string *html_dom_string_target;
बाह्य काष्ठा dom_string *html_dom_string_shape;
बाह्य काष्ठा dom_string *html_dom_string_default;
बाह्य काष्ठा dom_string *html_dom_string_rect;
बाह्य काष्ठा dom_string *html_dom_string_rectangle;
बाह्य काष्ठा dom_string *html_dom_string_coords;
बाह्य काष्ठा dom_string *html_dom_string_circle;
बाह्य काष्ठा dom_string *html_dom_string_poly;
बाह्य काष्ठा dom_string *html_dom_string_polygon;
बाह्य काष्ठा dom_string *html_dom_string_text_javascript;
बाह्य काष्ठा dom_string *html_dom_string_type;
बाह्य काष्ठा dom_string *html_dom_string_src;

#पूर्ण_अगर
