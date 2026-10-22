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

#నిర్వచితం_కాకపోతే NETSURF_HTML_వ్యక్తిగత_H
#నిర్వచించు NETSURF_HTML_వ్యక్తిగత_H

#అన్తర్భావ <dom/bindings/hubbub/parser.h>

#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/handlers/css/utils.h"


నిర్మాణం gui_layout_పట్టిక;
నిర్మాణం scrollbar_సందేశం_దత్తాంశం;
నిర్మాణం content_redraw_దత్తాంశం;
నిర్మాణం selection;

రకనిర్వచనం గణన {
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
సంఘం html_drag_owner {
	bool no_owner;
	నిర్మాణం box *content;
	నిర్మాణం scrollbar *scrollbar;
	నిర్మాణం box *textarea;
};

రకనిర్వచనం గణన {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
సంఘం html_selection_owner {
	bool none;
	నిర్మాణం box *textarea;
	నిర్మాణం box *content;
};

రకనిర్వచనం గణన {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
సంఘం html_focus_owner {
	bool self;
	నిర్మాణం box *textarea;
	నిర్మాణం box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
రకనిర్వచనం నిర్మాణం html_content {
	నిర్మాణం content పునాది;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_బహుళకం quirks;

	/** Encoding of source, NULL if unknown. */
	అక్షరం *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_ఆధారం encoding_ఆధారం;

	/** Base URL (may be a copy of content->url). */
	నిర్మాణం nsurl *పునాది_url;
	/** Base target */
	అక్షరం *పునాది_లక్ష్యం;

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
	dom_కణుపు *title;

	/** A talloc context purely for the render box tree */
	పూర్ణాంకం *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	శూన్య *box_conversion_context;
	/** Box tree, or NULL. */
	నిర్మాణం box *layout;
	/** Document background colour. */
	రంగు background_రంగు;

	/** Font callback table */
	స్థిరరాశి నిర్మాణం gui_layout_పట్టిక *font_func;

	/** Number of entries in scripts */
	గుర్తులేని పూర్ణాంకం scripts_లెక్క;
	/** Scripts */
	నిర్మాణం html_script *scripts;
	/** javascript thread in use */
	నిర్మాణం jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	గుర్తులేని పూర్ణాంకం stylesheet_లెక్క;
	/** Stylesheets. Each may be NULL. */
	నిర్మాణం html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	గుర్తులేని పూర్ణాంకం అంకె_objects;
	/** List of objects. */
	నిర్మాణం content_html_object *object_list;
	/** Forms, in reverse order to document. */
	నిర్మాణం form *forms;
	/** Hash table of imagemaps. */
	నిర్మాణం imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	నిర్మాణం browser_కిటికీ *bw;

	/** Frameset information */
	నిర్మాణం content_html_చట్రాలు *frameset;

	/** Inline frame information */
	నిర్మాణం content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	నిర్మాణం html_content *పుట;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	సంఘం html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	సంఘం html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	సంఘం html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	నిర్మాణం selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	నిర్మాణం form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
బాహ్య bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
శూన్య html__redraw_a_box(html_content *htmlc, నిర్మాణం box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
శూన్య html_ముగింపు_conversion(html_content *htmlc);


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
bool html_exec(నిర్మాణం content *c, స్థిరరాశి అక్షరం *src, size_t srclen);


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
nserror html_proceed_గమ్యం_పూర్తి(html_content *html);


/* in html/redraw.c */
bool html_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(నిర్మాణం box *box, పూర్ణాంకం x_మాతృ, పూర్ణాంకం y_మాతృ,
		పూర్ణాంకం p_వెడల్పు, పూర్ణాంకం p_ఎత్తు, స్థిరరాశి నిర్మాణం rect *clip, దశాంశం scale,
		స్థిరరాశి నిర్మాణం redraw_context *ctx);


bool html_redraw_inline_borders(నిర్మాణం box *box, నిర్మాణం rect b,
		స్థిరరాశి నిర్మాణం rect *clip, దశాంశం scale, bool మొదటి, bool చివరి,
		స్థిరరాశి నిర్మాణం redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(శూన్య *ctx, dom_కణుపు *కణుపు);


/* in html/forms.c */
నిర్మాణం form *html_forms_get_forms(స్థిరరాశి అక్షరం *docenc, dom_html_document *doc);
నిర్మాణం form_control *html_forms_get_control_for_కణుపు(నిర్మాణం form *forms,
		dom_కణుపు *కణుపు);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_రిజిస్టరు(శూన్య);
nserror html_css_fetcher_కూడు_అంశం(dom_string *దత్తాంశం, నిర్మాణం nsurl *పునాది_url,
		uint32_t *కీలకం);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_కణుపు *లక్ష్యం,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_కణుపు *లక్ష్యం,
		bool bubbles, bool cancelable, uint32_t కీలకం);

/* Useful dom_string pointers */
నిర్మాణం dom_string;

బాహ్య నిర్మాణం dom_string *html_dom_string_map;
బాహ్య నిర్మాణం dom_string *html_dom_string_id;
బాహ్య నిర్మాణం dom_string *html_dom_string_name;
బాహ్య నిర్మాణం dom_string *html_dom_string_area;
బాహ్య నిర్మాణం dom_string *html_dom_string_a;
బాహ్య నిర్మాణం dom_string *html_dom_string_nohref;
బాహ్య నిర్మాణం dom_string *html_dom_string_href;
బాహ్య నిర్మాణం dom_string *html_dom_string_లక్ష్యం;
బాహ్య నిర్మాణం dom_string *html_dom_string_shape;
బాహ్య నిర్మాణం dom_string *html_dom_string_default;
బాహ్య నిర్మాణం dom_string *html_dom_string_rect;
బాహ్య నిర్మాణం dom_string *html_dom_string_rectangle;
బాహ్య నిర్మాణం dom_string *html_dom_string_coords;
బాహ్య నిర్మాణం dom_string *html_dom_string_circle;
బాహ్య నిర్మాణం dom_string *html_dom_string_poly;
బాహ్య నిర్మాణం dom_string *html_dom_string_polygon;
బాహ్య నిర్మాణం dom_string *html_dom_string_పాఠ్యం_javascript;
బాహ్య నిర్మాణం dom_string *html_dom_string_type;
బాహ్య నిర్మాణం dom_string *html_dom_string_src;

#అయితే_ముగింపు
