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

#ଅନ୍ତର୍ଭାଵ <dom/bindings/hubbub/parser.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/handlers/css/utils.h"


ସଂରଚନା gui_layout_table;
ସଂରଚନା scrollbar_ସନ୍ଦେଶ_data;
ସଂରଚନା content_redraw_data;
ସଂରଚନା selection;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
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
ସଂଘ html_drag_owner {
	bool no_owner;
	ସଂରଚନା box *content;
	ସଂରଚନା scrollbar *scrollbar;
	ସଂରଚନା box *textarea;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
ସଂଘ html_selection_owner {
	bool none;
	ସଂରଚନା box *textarea;
	ସଂରଚନା box *content;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
ସଂଘ html_focus_owner {
	bool self;
	ସଂରଚନା box *textarea;
	ସଂରଚନା box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା html_content {
	ସଂରଚନା content base;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_mode quirks;

	/** Encoding of source, NULL if unknown. */
	ଅକ୍ଷର *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_source encoding_source;

	/** Base URL (may be a copy of content->url). */
	ସଂରଚନା nsurl *base_url;
	/** Base target */
	ଅକ୍ଷର *base_target;

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
	dom_ଗଣ୍ଠି *title;

	/** A talloc context purely for the render box tree */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	ଶୂନ୍ୟ *box_conversion_context;
	/** Box tree, or NULL. */
	ସଂରଚନା box *layout;
	/** Document background colour. */
	colour background_colour;

	/** Font callback table */
	ଅଚଳ ସଂରଚନା gui_layout_table *font_func;

	/** Number of entries in scripts */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା scripts_ଗଣନା;
	/** Scripts */
	ସଂରଚନା html_script *scripts;
	/** javascript thread in use */
	ସଂରଚନା jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା stylesheet_ଗଣନା;
	/** Stylesheets. Each may be NULL. */
	ସଂରଚନା html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_objects;
	/** List of objects. */
	ସଂରଚନା content_html_object *object_list;
	/** Forms, in reverse order to document. */
	ସଂରଚନା form *forms;
	/** Hash table of imagemaps. */
	ସଂରଚନା imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	ସଂରଚନା browser_window *bw;

	/** Frameset information */
	ସଂରଚନା content_html_frames *frameset;

	/** Inline frame information */
	ସଂରଚନା content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	ସଂରଚନା html_content *page;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	ସଂଘ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	ସଂଘ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	ସଂଘ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	ସଂରଚନା selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	ସଂରଚନା form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
ବାହ୍ୟ bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
ଶୂନ୍ୟ html__redraw_a_box(html_content *htmlc, ସଂରଚନା box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
ଶୂନ୍ୟ html_finish_conversion(html_content *htmlc);


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
bool html_exec(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *src, size_t srclen);


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
bool html_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(ସଂରଚନା box *box, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_height, ଅଚଳ ସଂରଚନା rect *clip, ଦଶମିକ scale,
		ଅଚଳ ସଂରଚନା redraw_context *ctx);


bool html_redraw_inline_borders(ସଂରଚନା box *box, ସଂରଚନା rect b,
		ଅଚଳ ସଂରଚନା rect *clip, ଦଶମିକ scale, bool first, bool last,
		ଅଚଳ ସଂରଚନା redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(ଶୂନ୍ୟ *ctx, dom_ଗଣ୍ଠି *ଗଣ୍ଠି);


/* in html/forms.c */
ସଂରଚନା form *html_forms_get_forms(ଅଚଳ ଅକ୍ଷର *docenc, dom_html_document *doc);
ସଂରଚନା form_control *html_forms_get_control_for_ଗଣ୍ଠି(ସଂରଚନା form *forms,
		dom_ଗଣ୍ଠି *ଗଣ୍ଠି);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_register(ଶୂନ୍ୟ);
nserror html_css_fetcher_add_item(dom_string *data, ସଂରଚନା nsurl *base_url,
		uint32_t *ଚାବି);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_ଗଣ୍ଠି *target,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_ଗଣ୍ଠି *target,
		bool bubbles, bool cancelable, uint32_t ଚାବି);

/* Useful dom_string pointers */
ସଂରଚନା dom_string;

ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_map;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_id;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_name;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_area;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_a;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_nohref;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_href;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_target;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_shape;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_default;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_rect;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_rectangle;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_coords;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_circle;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_poly;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_polygon;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_text_javascript;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_type;
ବାହ୍ୟ ସଂରଚନା dom_string *html_dom_string_src;

#endif
