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

#அந்தர்பாவ <stdbool.h>

#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "netsurf/content_type.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/mouse.h"
#அந்தர்பாவ "desktop/frame_types.h"

கட்டமைப்பு fetch_multipart_data;
கட்டமைப்பு box;
கட்டமைப்பு rect;
கட்டமைப்பு browser_window;
கட்டமைப்பு content;
கட்டமைப்பு hlcache_handle;
கட்டமைப்பு http_parameter;
கட்டமைப்பு imagemap;
கட்டமைப்பு object_params;
கட்டமைப்பு plotters;
கட்டமைப்பு textarea;
கட்டமைப்பு scrollbar;
கட்டமைப்பு scrollbar_செய்தி_data;
கட்டமைப்பு தேடல்_context;
கட்டமைப்பு selection;
கட்டமைப்பு nsurl;
கட்டமைப்பு plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
கட்டமைப்பு html_stylesheet {
	கட்டமைப்பு dom_முனை *முனை; /**< dom node associated with sheet */
	கட்டமைப்பு hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
கட்டமைப்பு html_script {
	/** Type of script */
	எண்ணிக்கை html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	ஒன்றிப்பு {
		கட்டமைப்பு hlcache_handle *handle;
		கட்டமைப்பு dom_string *string;
	} data;	/**< Script data */
	கட்டமைப்பு dom_string *mimetype;
	கட்டமைப்பு dom_string *encoding;
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
கட்டமைப்பு content_html_object {
	கட்டமைப்பு content *parent;		/**< Parent document */
	கட்டமைப்பு content_html_object *next; /**< Next in chain */

	கட்டமைப்பு hlcache_handle *content;  /**< Content, or 0. */
	கட்டமைப்பு box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
கட்டமைப்பு content_html_frames {
	முழு cols;	/** number of columns in frameset */
	முழு rows;	/** number of rows in frameset */

	கட்டமைப்பு frame_dimension width;	/** frame width */
	கட்டமைப்பு frame_dimension height;	/** frame width */
	முழு margin_width;	/** frame margin width */
	முழு margin_height;	/** frame margin height */

	எழுத்து *name;	/** frame name (for targetting) */
	கட்டமைப்பு nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	கட்டமைப்பு content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
கட்டமைப்பு content_html_iframe {
	கட்டமைப்பு box *box;

	முழு margin_width;	/** frame margin width */
	முழு margin_height;	/** frame margin height */

	எழுத்து *name;	/** frame name (for targetting) */
	கட்டமைப்பு nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	கட்டமைப்பு content_html_iframe *next;
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
nserror html_init(வெறுமை);

/**
 * redraw a specific box
 *
 * used by core browser
 */
வெறுமை html_redraw_a_box(கட்டமைப்பு hlcache_handle *h, கட்டமைப்பு box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
கட்டமைப்பு content_html_frames *html_get_frameset(கட்டமைப்பு hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
கட்டமைப்பு content_html_iframe *html_get_iframe(கட்டமைப்பு hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
மாறிலி எழுத்து *html_get_base_target(கட்டமைப்பு hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
வெறுமை html_set_file_gadget_filename(கட்டமைப்பு hlcache_handle *hl,
	கட்டமைப்பு form_control *gadget, மாறிலி எழுத்து *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
கட்டமைப்பு html_stylesheet *html_get_stylesheets(கட்டமைப்பு hlcache_handle *h,
		unsigned முழு *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
கட்டமைப்பு content_html_object *html_get_objects(கட்டமைப்பு hlcache_handle *h,
		unsigned முழு *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(கட்டமைப்பு hlcache_handle *h, lwc_string *frag_id,
		முழு *x, முழு *y);

#endif
