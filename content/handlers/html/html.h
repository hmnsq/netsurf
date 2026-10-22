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

কাঠামো fetch_multipart_তথ্য;
কাঠামো box;
কাঠামো rect;
কাঠামো browser_window;
কাঠামো content;
কাঠামো hlcache_handle;
কাঠামো http_parameter;
কাঠামো imagemap;
কাঠামো object_params;
কাঠামো plotters;
কাঠামো textarea;
কাঠামো scrollbar;
কাঠামো scrollbar_বার্তা_তথ্য;
কাঠামো অনুসন্ধান_context;
কাঠামো selection;
কাঠামো nsurl;
কাঠামো plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
কাঠামো html_stylesheet {
	কাঠামো dom_গ্রন্থি *গ্রন্থি; /**< dom node associated with sheet */
	কাঠামো hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
কাঠামো html_script {
	/** Type of script */
	গণনা html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	সংঘ {
		কাঠামো hlcache_handle *handle;
		কাঠামো dom_string *string;
	} তথ্য;	/**< Script data */
	কাঠামো dom_string *mimetype;
	কাঠামো dom_string *encoding;
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
কাঠামো content_html_object {
	কাঠামো content *parent;		/**< Parent document */
	কাঠামো content_html_object *পরবর্তী; /**< Next in chain */

	কাঠামো hlcache_handle *content;  /**< Content, or 0. */
	কাঠামো box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
কাঠামো content_html_frames {
	পূর্ণসংখ্যা cols;	/** number of columns in frameset */
	পূর্ণসংখ্যা rows;	/** number of rows in frameset */

	কাঠামো frame_dimension width;	/** frame width */
	কাঠামো frame_dimension height;	/** frame width */
	পূর্ণসংখ্যা margin_width;	/** frame margin width */
	পূর্ণসংখ্যা margin_height;	/** frame margin height */

	অক্ষর *name;	/** frame name (for targetting) */
	কাঠামো nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	কাঠামো content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
কাঠামো content_html_iframe {
	কাঠামো box *box;

	পূর্ণসংখ্যা margin_width;	/** frame margin width */
	পূর্ণসংখ্যা margin_height;	/** frame margin height */

	অক্ষর *name;	/** frame name (for targetting) */
	কাঠামো nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	কাঠামো content_html_iframe *পরবর্তী;
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
শূন্য html_redraw_a_box(কাঠামো hlcache_handle *h, কাঠামো box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
কাঠামো content_html_frames *html_get_frameset(কাঠামো hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
কাঠামো content_html_iframe *html_get_iframe(কাঠামো hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ধ্রুবক অক্ষর *html_get_base_target(কাঠামো hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
শূন্য html_set_file_gadget_filename(কাঠামো hlcache_handle *hl,
	কাঠামো form_control *gadget, ধ্রুবক অক্ষর *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
কাঠামো html_stylesheet *html_get_stylesheets(কাঠামো hlcache_handle *h,
		unsigned পূর্ণসংখ্যা *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
কাঠামো content_html_object *html_get_objects(কাঠামো hlcache_handle *h,
		unsigned পূর্ণসংখ্যা *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(কাঠামো hlcache_handle *h, lwc_string *frag_id,
		পূর্ণসংখ্যা *x, পূর্ণসংখ্যা *y);

#endif
