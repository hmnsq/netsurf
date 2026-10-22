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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_ಖಾಸಗಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_ಖಾಸಗಿ_H

#ಅನ್ತರ್ಭಾವ <dom/bindings/hubbub/parser.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/handlers/css/utils.h"


ರಚನೆ gui_layout_ಕೋಷ್ಟಕ;
ರಚನೆ scrollbar_ಸಂದೇಶ_ದತ್ತಾಂಶ;
ರಚನೆ content_redraw_ದತ್ತಾಂಶ;
ರಚನೆ selection;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
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
ಒಕ್ಕೂಟ html_drag_owner {
	bool no_owner;
	ರಚನೆ box *content;
	ರಚನೆ scrollbar *scrollbar;
	ರಚನೆ box *textarea;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
ಒಕ್ಕೂಟ html_selection_owner {
	bool none;
	ರಚನೆ box *textarea;
	ರಚನೆ box *content;
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
ಒಕ್ಕೂಟ html_focus_owner {
	bool self;
	ರಚನೆ box *textarea;
	ರಚನೆ box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ html_content {
	ರಚನೆ content ತಳ;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_ಬಹುಲಕ quirks;

	/** Encoding of source, NULL if unknown. */
	ಅಕ್ಷರ *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_ಮೂಲ encoding_ಮೂಲ;

	/** Base URL (may be a copy of content->url). */
	ರಚನೆ nsurl *ತಳ_url;
	/** Base target */
	ಅಕ್ಷರ *ತಳ_ಗುರಿ;

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
	dom_ಗಂಟು *title;

	/** A talloc context purely for the render box tree */
	ಪೂರ್ಣಾಂಕ *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	ಶೂನ್ಯ *box_conversion_context;
	/** Box tree, or NULL. */
	ರಚನೆ box *layout;
	/** Document background colour. */
	ಬಣ್ಣ background_ಬಣ್ಣ;

	/** Font callback table */
	ಸ್ಥಿರಾಂಕ ರಚನೆ gui_layout_ಕೋಷ್ಟಕ *font_func;

	/** Number of entries in scripts */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ scripts_ಎಣಿಕೆ;
	/** Scripts */
	ರಚನೆ html_script *scripts;
	/** javascript thread in use */
	ರಚನೆ jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ stylesheet_ಎಣಿಕೆ;
	/** Stylesheets. Each may be NULL. */
	ರಚನೆ html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_objects;
	/** List of objects. */
	ರಚನೆ content_html_object *object_list;
	/** Forms, in reverse order to document. */
	ರಚನೆ form *forms;
	/** Hash table of imagemaps. */
	ರಚನೆ imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	ರಚನೆ browser_ಕಿಟಕಿ *bw;

	/** Frameset information */
	ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *frameset;

	/** Inline frame information */
	ರಚನೆ content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	ರಚನೆ html_content *ಪುಟ;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	ಒಕ್ಕೂಟ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	ಒಕ್ಕೂಟ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	ಒಕ್ಕೂಟ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	ರಚನೆ selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	ರಚನೆ form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
ಬಾಹ್ಯ bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
ಶೂನ್ಯ html__redraw_a_box(html_content *htmlc, ರಚನೆ box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
ಶೂನ್ಯ html_ಮುಕ್ತಾಯ_conversion(html_content *htmlc);


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
bool html_exec(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t srclen);


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
nserror html_proceed_ಗಮ್ಯ_ಮುಗಿದ(html_content *html);


/* in html/redraw.c */
bool html_redraw(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(ರಚನೆ box *box, ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ,
		ಪೂರ್ಣಾಂಕ p_ಅಗಲ, ಪೂರ್ಣಾಂಕ p_ಎತ್ತರ, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ದಶಮಾಂಶ scale,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);


bool html_redraw_inline_borders(ರಚನೆ box *box, ರಚನೆ rect b,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ದಶಮಾಂಶ scale, bool ಮೊದಲ, bool ಕೊನೆಯ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(ಶೂನ್ಯ *ctx, dom_ಗಂಟು *ಗಂಟು);


/* in html/forms.c */
ರಚನೆ form *html_forms_get_forms(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *docenc, dom_html_document *doc);
ರಚನೆ form_control *html_forms_get_control_for_ಗಂಟು(ರಚನೆ form *forms,
		dom_ಗಂಟು *ಗಂಟು);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_ರಿಜಿಸ್ಟರ್(ಶೂನ್ಯ);
nserror html_css_fetcher_ಕೂಡಿಸು_ಅಂಶ(dom_string *ದತ್ತಾಂಶ, ರಚನೆ nsurl *ತಳ_url,
		uint32_t *ಕೀಲಿ);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_ಗಂಟು *ಗುರಿ,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_ಗಂಟು *ಗುರಿ,
		bool bubbles, bool cancelable, uint32_t ಕೀಲಿ);

/* Useful dom_string pointers */
ರಚನೆ dom_string;

ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_map;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_id;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_name;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_area;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_a;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_nohref;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_href;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_ಗುರಿ;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_shape;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_default;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_rect;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_rectangle;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_coords;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_circle;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_poly;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_polygon;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_ಪಠ್ಯ_javascript;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_type;
ಬಾಹ್ಯ ರಚನೆ dom_string *html_dom_string_src;

#ಆದರೆ_ಅಂತ್ಯ
