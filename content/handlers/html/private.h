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

#ضمن <dom/bindings/hubbub/parser.h>

#ضمن "netsurf/types.h"
#ضمن "content/content_protected.h"
#ضمن "content/handlers/css/utils.h"


بنية gui_layout_table;
بنية scrollbar_رسالة_data;
بنية content_redraw_data;
بنية selection;

تعريفنوع تعداد {
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
اتحاد html_drag_owner {
	bool no_owner;
	بنية box *content;
	بنية scrollbar *scrollbar;
	بنية box *textarea;
};

تعريفنوع تعداد {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
اتحاد html_selection_owner {
	bool none;
	بنية box *textarea;
	بنية box *content;
};

تعريفنوع تعداد {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
اتحاد html_focus_owner {
	bool self;
	بنية box *textarea;
	بنية box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
تعريفنوع بنية html_content {
	بنية content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	حرف *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	بنية nsurl *base_url;
	/** Base target */
	حرف *base_target;

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
	dom_عقدة *title;

	/** A talloc context purely for the render box tree */
	صحيح *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	فراغ *box_conversion_context;
	/** Box tree, or NULL. */
	بنية box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	ثابت بنية gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned صحيح scripts_تعداد;
	/** Scripts */
	بنية html_script *scripts;
	/** javascript thread in use */
	بنية jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned صحيح stylesheet_تعداد;
	/** Stylesheets. Each may be NULL. */
	بنية html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned صحيح رقم_objects;
	/** List of objects. */
	بنية content_html_object *object_list;
	/** Forms, in reverse order to document. */
	بنية form *forms;
	/** Hash table of imagemaps. */
	بنية imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	بنية browser_window *bw;

	/** Frameset information */
	بنية content_html_frames *frameset;

	/** Inline frame information */
	بنية content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	بنية html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	اتحاد html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	اتحاد html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	اتحاد html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	بنية selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	بنية form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
خارجي bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
فراغ html__redraw_a_box(html_content *htmlc, بنية box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
فراغ html_finish_conversion(html_content *htmlc);


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
bool html_exec(بنية content *c, ثابت حرف *src, size_t srclen);


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
bool html_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(بنية box *box, صحيح x_parent, صحيح y_parent,
		صحيح p_width, صحيح p_height, ثابت بنية rect *clip, عائم scale,
		ثابت بنية redraw_context *ctx);


bool html_redraw_inline_borders(بنية box *box, بنية rect b,
		ثابت بنية rect *clip, عائم scale, bool first, bool last,
		ثابت بنية redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(فراغ *ctx, dom_عقدة *عقدة);


/* in html/forms.c */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc);
بنية form_control *html_forms_get_control_for_عقدة(بنية form *forms,
		dom_عقدة *عقدة);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(فراغ);
nserror html_css_fetcher_add_item(dom_string *data, بنية nsurl *base_url,
		uint32_t *مفتاح);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_عقدة *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_عقدة *target,
		bool bubbles, bool cancelable, uint32_t مفتاح);

/* Useful dom_string pointers */
بنية dom_string;

خارجي بنية dom_string *html_dom_string_map;
خارجي بنية dom_string *html_dom_string_id;
خارجي بنية dom_string *html_dom_string_name;
خارجي بنية dom_string *html_dom_string_area;
خارجي بنية dom_string *html_dom_string_a;
خارجي بنية dom_string *html_dom_string_nohref;
خارجي بنية dom_string *html_dom_string_href;
خارجي بنية dom_string *html_dom_string_target;
خارجي بنية dom_string *html_dom_string_shape;
خارجي بنية dom_string *html_dom_string_default;
خارجي بنية dom_string *html_dom_string_rect;
خارجي بنية dom_string *html_dom_string_rectangle;
خارجي بنية dom_string *html_dom_string_coords;
خارجي بنية dom_string *html_dom_string_circle;
خارجي بنية dom_string *html_dom_string_poly;
خارجي بنية dom_string *html_dom_string_polygon;
خارجي بنية dom_string *html_dom_string_text_javascript;
خارجي بنية dom_string *html_dom_string_type;
خارجي بنية dom_string *html_dom_string_src;

#endif
