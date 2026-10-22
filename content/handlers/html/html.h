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
 * Interface to text/html content handler.
 *
 * These functions should in general be called via the content interface.
 */

#اگر_غیر_تعریف_شدہ NETSURF_HTML_HTML_H
#تعریف_کرو NETSURF_HTML_HTML_H

#شامل <stdbool.h>

#شامل "netsurf/types.h"
#شامل "netsurf/content_type.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/mouse.h"
#شامل "desktop/frame_types.h"

ساخت fetch_multipart_ڈیٹا;
ساخت box;
ساخت rect;
ساخت browser_کھڑکی;
ساخت content;
ساخت hlcache_handle;
ساخت http_parameter;
ساخت imagemap;
ساخت object_params;
ساخت plotters;
ساخت textarea;
ساخت scrollbar;
ساخت scrollbar_پیغام_ڈیٹا;
ساخت تلاش_context;
ساخت selection;
ساخت nsurl;
ساخت plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ساخت html_stylesheet {
	ساخت dom_گرہ *گرہ; /**< dom node associated with sheet */
	ساخت hlcache_handle *sheet;
	bool modified;
	bool غیر_مستعمل;
};


/**
 * Container for scripts used by an HTML document
 */
ساخت html_script {
	/** Type of script */
	شمار html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	اتحاد {
		ساخت hlcache_handle *handle;
		ساخت dom_string *string;
	} ڈیٹا;	/**< Script data */
	ساخت dom_string *mimetype;
	ساخت dom_string *encoding;
	bool already_started;
	bool parser_inserted;
	bool force_async;
	bool ready_exec;
	bool async;
	bool defer;
};


/**
 * An object (img, object, etc. tag) in a CONTENT_HTML document.
 */
ساخت content_html_object {
	ساخت content *والد;		/**< Parent document */
	ساخت content_html_object *اگلا; /**< Next in chain */

	ساخت hlcache_handle *content;  /**< Content, or 0. */
	ساخت box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ساخت content_html_چوکھٹے {
	صحیح ستون_تعداد;	/** number of columns in frameset */
	صحیح صفیں;	/** number of rows in frameset */

	ساخت چوکھٹا_dimension چوڑائی;	/** frame width */
	ساخت چوکھٹا_dimension اونچائی;	/** frame width */
	صحیح margin_چوڑائی;	/** frame margin width */
	صحیح margin_اونچائی;	/** frame margin height */

	حرف *name;	/** frame name (for targetting) */
	ساخت nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	رنگ border_رنگ;	/** frame border colour */

	ساخت content_html_چوکھٹے *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ساخت content_html_iframe {
	ساخت box *box;

	صحیح margin_چوڑائی;	/** frame margin width */
	صحیح margin_اونچائی;	/** frame margin height */

	حرف *name;	/** frame name (for targetting) */
	ساخت nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	رنگ border_رنگ;	/** frame border colour */

	ساخت content_html_iframe *اگلا;
};

/* entries in stylesheet_content */
#تعریف_کرو STYLESHEET_بنیاد		0	/* base style sheet */
#تعریف_کرو STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#تعریف_کرو STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#تعریف_کرو STYLESHEET_USER		3	/* user stylesheet */
#تعریف_کرو STYLESHEET_آغاز	4	/* start of document stylesheets */

/**
 * initialise content handler
 *
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror html_init(خالی);

/**
 * redraw a specific box
 *
 * used by core browser
 */
خالی html_redraw_a_box(ساخت hlcache_handle *h, ساخت box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ساخت content_html_چوکھٹے *html_get_frameset(ساخت hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ساخت content_html_iframe *html_get_iframe(ساخت hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ثابت حرف *html_get_بنیاد_ہدف(ساخت hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
خالی html_سیٹ_file_gadget_filename_2(ساخت hlcache_handle *hl,
	ساخت form_control *gadget, ثابت حرف *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ساخت html_stylesheet *html_get_stylesheets(ساخت hlcache_handle *h,
		غیرعلامتی صحیح *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ساخت content_html_object *html_get_objects(ساخت hlcache_handle *h,
		غیرعلامتی صحیح *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_انحراف(ساخت hlcache_handle *h, lwc_string *frag_id,
		صحیح *x, صحیح *y);

#اگر_ختم
