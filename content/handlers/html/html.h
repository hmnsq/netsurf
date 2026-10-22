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

#ifndef NETSURF_HTML_HTML_H
#define NETSURF_HTML_HTML_H

#অন্তর্ভাব <stdbool.h>

#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "netsurf/content_type.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/mouse.h"
#অন্তর্ভাব "desktop/frame_types.h"

সংরচনা fetch_multipart_data;
সংরচনা box;
সংরচনা rect;
সংরচনা browser_window;
সংরচনা content;
সংরচনা hlcache_handle;
সংরচনা http_parameter;
সংরচনা imagemap;
সংরচনা object_params;
সংরচনা plotters;
সংরচনা textarea;
সংরচনা scrollbar;
সংরচনা scrollbar_বার্তা_data;
সংরচনা অনুসন্ধান_context;
সংরচনা selection;
সংরচনা nsurl;
সংরচনা plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
সংরচনা html_stylesheet {
	সংরচনা dom_গ্রন্থি *গ্রন্থি; /**< dom node associated with sheet */
	সংরচনা hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
সংরচনা html_script {
	/** Type of script */
	গণনা html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	সঙ্ঘ {
		সংরচনা hlcache_handle *handle;
		সংরচনা dom_string *string;
	} data;	/**< Script data */
	সংরচনা dom_string *mimetype;
	সংরচনা dom_string *encoding;
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
সংরচনা content_html_object {
	সংরচনা content *parent;		/**< Parent document */
	সংরচনা content_html_object *next; /**< Next in chain */

	সংরচনা hlcache_handle *content;  /**< Content, or 0. */
	সংরচনা box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
সংরচনা content_html_frames {
	পূর্ণাঙ্ক cols;	/** number of columns in frameset */
	পূর্ণাঙ্ক rows;	/** number of rows in frameset */

	সংরচনা frame_dimension width;	/** frame width */
	সংরচনা frame_dimension height;	/** frame width */
	পূর্ণাঙ্ক margin_width;	/** frame margin width */
	পূর্ণাঙ্ক margin_height;	/** frame margin height */

	বর্ণ *name;	/** frame name (for targetting) */
	সংরচনা nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	সংরচনা content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
সংরচনা content_html_iframe {
	সংরচনা box *box;

	পূর্ণাঙ্ক margin_width;	/** frame margin width */
	পূর্ণাঙ্ক margin_height;	/** frame margin height */

	বর্ণ *name;	/** frame name (for targetting) */
	সংরচনা nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	সংরচনা content_html_iframe *next;
};

/* entries in stylesheet_content */
#define STYLESHEET_BASE		0	/* base style sheet */
#define STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#define STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#define STYLESHEET_USER		3	/* user stylesheet */
#define STYLESHEET_START	4	/* start of document stylesheets */

/**
 * initialise content handler
 *
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror html_init(শূন্য);

/**
 * redraw a specific box
 *
 * used by core browser
 */
শূন্য html_redraw_a_box(সংরচনা hlcache_handle *h, সংরচনা box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
সংরচনা content_html_frames *html_get_frameset(সংরচনা hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
সংরচনা content_html_iframe *html_get_iframe(সংরচনা hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ধ্রুব বর্ণ *html_get_base_target(সংরচনা hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
শূন্য html_set_file_gadget_filename(সংরচনা hlcache_handle *hl,
	সংরচনা form_control *gadget, ধ্রুব বর্ণ *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
সংরচনা html_stylesheet *html_get_stylesheets(সংরচনা hlcache_handle *h,
		unsigned পূর্ণাঙ্ক *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
সংরচনা content_html_object *html_get_objects(সংরচনা hlcache_handle *h,
		unsigned পূর্ণাঙ্ক *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(সংরচনা hlcache_handle *h, lwc_string *frag_id,
		পূর্ণাঙ্ক *x, পূর্ণাঙ্ক *y);

#endif
