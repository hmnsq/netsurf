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

#اگر_غیر_تعریف_شدہ NETSURF_HTML_نجی_H
#تعریف_کرو NETSURF_HTML_نجی_H

#شامل <dom/bindings/hubbub/parser.h>

#شامل "netsurf/types.h"
#شامل "content/content_protected.h"
#شامل "content/handlers/css/utils.h"


ساخت gui_layout_جدول;
ساخت scrollbar_پیغام_ڈیٹا;
ساخت content_redraw_ڈیٹا;
ساخت selection;

قسمتعریف شمار {
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
	ساخت box *content;
	ساخت scrollbar *scrollbar;
	ساخت box *textarea;
};

قسمتعریف شمار {
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
	ساخت box *textarea;
	ساخت box *content;
};

قسمتعریف شمار {
	HTML_FOCUS_SELF,		/**< Focus is our own */
	HTML_FOCUS_CONTENT,		/**< Focus belongs to child content */
	HTML_FOCUS_TEXTAREA		/**< Focus belongs to textarea */
} html_focus_type;

/**
 * For directing input
 */
اتحاد html_focus_owner {
	bool self;
	ساخت box *textarea;
	ساخت box *content;
};

/**
 * Data specific to CONTENT_HTML.
 */
قسمتعریف ساخت html_content {
	ساخت content بنیاد;

	dom_hubbub_parser *parser; /**< Parser object handle */
	bool parse_completed; /**< Whether the parse has been completed */
	bool conversion_begun; /**< Whether or not the conversion has begun */

	/** Document tree */
	dom_document *document;
	/** Quirkyness of document */
	dom_document_quirks_منوال quirks;

	/** Encoding of source, NULL if unknown. */
	حرف *encoding;
	/** Source of encoding information. */
	dom_hubbub_encoding_ماخذ encoding_ماخذ;

	/** Base URL (may be a copy of content->url). */
	ساخت nsurl *بنیاد_url;
	/** Base target */
	حرف *بنیاد_ہدف;

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
	dom_گرہ *title;

	/** A talloc context purely for the render box tree */
	صحیح *bctx;
	/** A context pointer for the box conversion, NULL if no conversion
	 * is in progress.
	 */
	خالی *box_conversion_context;
	/** Box tree, or NULL. */
	ساخت box *layout;
	/** Document background colour. */
	رنگ background_رنگ;

	/** Font callback table */
	ثابت ساخت gui_layout_جدول *font_func;

	/** Number of entries in scripts */
	غیرعلامتی صحیح scripts_گنتی;
	/** Scripts */
	ساخت html_script *scripts;
	/** javascript thread in use */
	ساخت jsthread *jsthread;

	/** Number of entries in stylesheet_content. */
	غیرعلامتی صحیح stylesheet_گنتی;
	/** Stylesheets. Each may be NULL. */
	ساخت html_stylesheet *stylesheets;
	/**< Style selection context */
	css_select_ctx *select_ctx;
	/**< Style selection media specification */
	css_media media;
	/** CSS length conversion context for document. */
	css_unit_ctx unit_len_ctx;
	/**< Universal selector */
	lwc_string *universal;

	/** Number of entries in object_list. */
	غیرعلامتی صحیح عدد_objects;
	/** List of objects. */
	ساخت content_html_object *object_list;
	/** Forms, in reverse order to document. */
	ساخت form *forms;
	/** Hash table of imagemaps. */
	ساخت imagemap **imagemaps;

	/** Browser window containing this document, or NULL if not open. */
	ساخت browser_کھڑکی *bw;

	/** Frameset information */
	ساخت content_html_چوکھٹے *frameset;

	/** Inline frame information */
	ساخت content_html_iframe *iframe;

	/** Content of type CONTENT_HTML containing this, or NULL if not an
	 * object within a page. */
	ساخت html_content *صفحہ;

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
	ساخت selection *sel;

	/**
	 * Open core-handled form SELECT menu, or NULL if none
	 *  currently open.
	 */
	ساخت form_control *visible_select_menu;

} html_content;

/**
 * Render padding and margin box outlines in html_redraw().
 */
خارجی bool html_redraw_debug;


/* in html/html.c */

/**
 * redraw a box
 *
 * \param htmlc HTML content
 * \param box The box to redraw.
 */
خالی html__redraw_a_box(html_content *htmlc, ساخت box *box);


/**
 * Complete conversion of an HTML document
 *
 * \param htmlc Content to convert
 */
خالی html_اختتام_conversion(html_content *htmlc);


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
bool html_exec(ساخت content *c, ثابت حرف *src, size_t srclen);


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
nserror html_proceed_مقصد_مکمل(html_content *html);


/* in html/redraw.c */
bool html_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx);


/* in html/redraw_border.c */
bool html_redraw_borders(ساخت box *box, صحیح x_والد, صحیح y_والد,
		صحیح p_چوڑائی, صحیح p_اونچائی, ثابت ساخت rect *clip, اعشاری scale,
		ثابت ساخت redraw_context *ctx);


bool html_redraw_inline_borders(ساخت box *box, ساخت rect b,
		ثابت ساخت rect *clip, اعشاری scale, bool پہلا, bool آخری,
		ثابت ساخت redraw_context *ctx);


/* in html/script.c */
dom_hubbub_error html_process_script(خالی *ctx, dom_گرہ *گرہ);


/* in html/forms.c */
ساخت form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc);
ساخت form_control *html_forms_get_control_for_گرہ(ساخت form *forms,
		dom_گرہ *گرہ);


/* in html/css_fetcher.c */
/**
 * Register the fetcher for the pseudo x-ns-css scheme.
 *
 * \return NSERROR_OK on successful registration or error code on failure.
 */
nserror html_css_fetcher_رجسٹر(خالی);
nserror html_css_fetcher_جوڑو_شے(dom_string *ڈیٹا, ساخت nsurl *بنیاد_url,
		uint32_t *کلید);


/* Events */
/**
 * Construct an event and fire it at the DOM
 *
 */
bool fire_generic_dom_event(dom_string *type, dom_گرہ *ہدف,
		    bool bubbles, bool cancelable);

/**
 * Construct a keyboard event and fire it at the DOM
 */
bool fire_dom_keyboard_event(dom_string *type, dom_گرہ *ہدف,
		bool bubbles, bool cancelable, uint32_t کلید);

/* Useful dom_string pointers */
ساخت dom_string;

خارجی ساخت dom_string *html_dom_string_map;
خارجی ساخت dom_string *html_dom_string_id;
خارجی ساخت dom_string *html_dom_string_name;
خارجی ساخت dom_string *html_dom_string_area;
خارجی ساخت dom_string *html_dom_string_a;
خارجی ساخت dom_string *html_dom_string_nohref;
خارجی ساخت dom_string *html_dom_string_href;
خارجی ساخت dom_string *html_dom_string_ہدف;
خارجی ساخت dom_string *html_dom_string_shape;
خارجی ساخت dom_string *html_dom_string_default;
خارجی ساخت dom_string *html_dom_string_rect;
خارجی ساخت dom_string *html_dom_string_rectangle;
خارجی ساخت dom_string *html_dom_string_coords;
خارجی ساخت dom_string *html_dom_string_circle;
خارجی ساخت dom_string *html_dom_string_poly;
خارجی ساخت dom_string *html_dom_string_polygon;
خارجی ساخت dom_string *html_dom_string_متن_javascript;
خارجی ساخت dom_string *html_dom_string_type;
خارجی ساخت dom_string *html_dom_string_src;

#اگر_ختم
