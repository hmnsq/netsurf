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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_ਨਿੱਜੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_ਨਿੱਜੀ_H

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/bindings/hubbub/parser.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/handlers/css/utils.h"


ਬਣਤਰ gui_layout_ਸਾਰਣੀ;
ਬਣਤਰ scrollbar_ਸੁਨੇਹਾ_ਅੰਕੜੇ;
ਬਣਤਰ content_redraw_ਅੰਕੜੇ;
ਬਣਤਰ selection;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
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
ਸੰਘ html_drag_owner {
	bool no_owner;
	ਬਣਤਰ box *content;
	ਬਣਤਰ scrollbar *scrollbar;
	ਬਣਤਰ box *textarea;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	HTML_SELECTION_NONE,		/** No selection */
	HTML_SELECTION_TEXTAREA,	/** Selection in one of our textareas */
	HTML_SELECTION_SELF,		/** Selection in this html content */
	HTML_SELECTION_CONTENT		/** Selection in child content */
} html_selection_type;

/**
 * For getting at selections in this content or things in this content
 */
ਸੰਘ html_selection_owner {
	bool none;
	ਬਣਤਰ box *textarea;
	ਬਣਤਰ box *content;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
ਸੰਘ html_focus_owner {
	bool self;
	ਬਣਤਰ box *textarea;
	ਬਣਤਰ box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ html_content {
	ਬਣਤਰ content ਅਧਾਰ;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_ਬਹੁਲਕ quirks;

	/** Encoding of source, NULL if unknown. */
	ਅੱਖਰ *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_ਸਰੋਤ encoding_ਸਰੋਤ;

	/** Base URL (may be a copy of content->url). */
	ਬਣਤਰ nsurl *ਅਧਾਰ_url;
	/** Base target */
	ਅੱਖਰ *ਅਧਾਰ_ਨਿਸ਼ਾਨਾ;

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
	dom_ਗੰਢ *title;

	/** A talloc context purely for the render box tree */
	ਪੂਰਨ *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	ਖਾਲੀ *box_conversion_context;
	/** Box tree, or NULL. */
	ਬਣਤਰ box *layout;
	/** Document background colour. */
	ਰੰਗ background_ਰੰਗ;

	/** Font callback table */
	ਅਚਲ ਬਣਤਰ gui_layout_ਸਾਰਣੀ *font_func;

	/** Number of entries in scripts */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ scripts_ਗਿਣਤੀ;
	/** Scripts */
	ਬਣਤਰ html_script *scripts;
	/** javascript thread in use */
	ਬਣਤਰ jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ stylesheet_ਗਿਣਤੀ;
	/** Stylesheets. Each may be NULL. */
	ਬਣਤਰ html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_objects;
	/** List of objects. */
	ਬਣਤਰ content_html_object *object_list;
	/** Forms, in reverse order to document. */
	ਬਣਤਰ form *forms;
	/** Hash table of imagemaps. */
	ਬਣਤਰ imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	ਬਣਤਰ browser_ਖਿੜਕੀ *bw;

	/** Frameset information */
	ਬਣਤਰ content_html_ਚੌਖਟੇ *frameset;

	/** Inline frame information */
	ਬਣਤਰ content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	ਬਣਤਰ html_content *ਪੰਨਾ;

	/** Current drag type */
	html_drag_type drag_type;
	/** Widget capturing all mouse events */
	ਸੰਘ html_drag_owner drag_owner;

	/** Current selection state */
	html_selection_type selection_type;
	/** Current selection owner */
	ਸੰਘ html_selection_owner selection_owner;

	/** Current input focus target type */
	html_focus_type focus_type;
	/** Current input focus target */
	ਸੰਘ html_focus_owner focus_owner;

	/** HTML content's own text selection object */
	ਬਣਤਰ selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	ਬਣਤਰ form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
ਬਾਹਰੀ bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
ਖਾਲੀ html__redraw_a_box(html_content *htmlc, ਬਣਤਰ box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
ਖਾਲੀ html_ਅੰਤ_conversion(html_content *htmlc);


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
bool html_exec(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *src, size_t srclen);


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
nserror html_proceed_ਮੰਜ਼ਿਲ_ਪੂਰਾ(html_content *html);


/* in html/redraw.c */
bool html_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(ਬਣਤਰ box *box, ਪੂਰਨ x_ਮਾਪਾ, ਪੂਰਨ y_ਮਾਪਾ,
		ਪੂਰਨ p_ਚੌੜਾਈ, ਪੂਰਨ p_ਉਚਾਈ, ਅਚਲ ਬਣਤਰ rect *clip, ਦਸ਼ਮਲਵ scale,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx);


bool html_redraw_inline_borders(ਬਣਤਰ box *box, ਬਣਤਰ rect b,
		ਅਚਲ ਬਣਤਰ rect *clip, ਦਸ਼ਮਲਵ scale, bool ਪਹਿਲਾ, bool ਆਖਰੀ,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(ਖਾਲੀ *ctx, dom_ਗੰਢ *ਗੰਢ);


/* in html/forms.c */
ਬਣਤਰ form *html_forms_get_forms(ਅਚਲ ਅੱਖਰ *docenc, dom_html_document *doc);
ਬਣਤਰ form_control *html_forms_get_control_for_ਗੰਢ(ਬਣਤਰ form *forms,
		dom_ਗੰਢ *ਗੰਢ);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_ਰਜਿਸਟਰ(ਖਾਲੀ);
nserror html_css_fetcher_ਜੋੜੋ_ਵਸਤੂ(dom_string *ਅੰਕੜੇ, ਬਣਤਰ nsurl *ਅਧਾਰ_url,
		uint32_t *ਕੁੰਜੀ);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ,
		bool bubbles, bool cancelable, uint32_t ਕੁੰਜੀ);

/* Useful dom_string pointers */
ਬਣਤਰ dom_string;

ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_map;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_id;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_name;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_area;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_a;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_nohref;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_href;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_ਨਿਸ਼ਾਨਾ;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_shape;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_default;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_rect;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_rectangle;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_coords;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_circle;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_poly;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_polygon;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_ਪਾਠ_javascript;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_type;
ਬਾਹਰੀ ਬਣਤਰ dom_string *html_dom_string_src;

#ਜੇ_ਅੰਤ
