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

#شامل <stdbool.h>

#شامل "netsurf/types.h"
#شامل "netsurf/content_type.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/mouse.h"
#شامل "desktop/frame_types.h"

ساختار fetch_multipart_data;
ساختار box;
ساختار rect;
ساختار browser_window;
ساختار content;
ساختار hlcache_handle;
ساختار http_parameter;
ساختار imagemap;
ساختار object_params;
ساختار plotters;
ساختار textarea;
ساختار scrollbar;
ساختار scrollbar_پیام_data;
ساختار جستجو_context;
ساختار selection;
ساختار nsurl;
ساختار plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ساختار html_stylesheet {
	ساختار dom_گره *گره; /**< dom node associated with sheet */
	ساختار hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
ساختار html_script {
	/** Type of script */
	شمارش html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	اجتماع {
		ساختار hlcache_handle *handle;
		ساختار dom_string *string;
	} data;	/**< Script data */
	ساختار dom_string *mimetype;
	ساختار dom_string *encoding;
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
ساختار content_html_object {
	ساختار content *parent;		/**< Parent document */
	ساختار content_html_object *next; /**< Next in chain */

	ساختار hlcache_handle *content;  /**< Content, or 0. */
	ساختار box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ساختار content_html_frames {
	صحیح cols;	/** number of columns in frameset */
	صحیح rows;	/** number of rows in frameset */

	ساختار frame_dimension width;	/** frame width */
	ساختار frame_dimension height;	/** frame width */
	صحیح margin_width;	/** frame margin width */
	صحیح margin_height;	/** frame margin height */

	نویسه *name;	/** frame name (for targetting) */
	ساختار nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ساختار content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ساختار content_html_iframe {
	ساختار box *box;

	صحیح margin_width;	/** frame margin width */
	صحیح margin_height;	/** frame margin height */

	نویسه *name;	/** frame name (for targetting) */
	ساختار nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ساختار content_html_iframe *next;
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
nserror html_init(تهی);

/**
 * redraw a specific box
 *
 * used by core browser
 */
تهی html_redraw_a_box(ساختار hlcache_handle *h, ساختار box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ساختار content_html_frames *html_get_frameset(ساختار hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ساختار content_html_iframe *html_get_iframe(ساختار hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ثابت نویسه *html_get_base_target(ساختار hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
تهی html_set_file_gadget_filename(ساختار hlcache_handle *hl,
	ساختار form_control *gadget, ثابت نویسه *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ساختار html_stylesheet *html_get_stylesheets(ساختار hlcache_handle *h,
		unsigned صحیح *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ساختار content_html_object *html_get_objects(ساختار hlcache_handle *h,
		unsigned صحیح *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(ساختار hlcache_handle *h, lwc_string *frag_id,
		صحیح *x, صحیح *y);

#endif
