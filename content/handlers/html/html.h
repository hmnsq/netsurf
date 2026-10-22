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

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content_type.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "desktop/frame_types.h"

ಸಂರಚನಾ fetch_multipart_data;
ಸಂರಚನಾ box;
ಸಂರಚನಾ rect;
ಸಂರಚನಾ browser_window;
ಸಂರಚನಾ content;
ಸಂರಚನಾ hlcache_handle;
ಸಂರಚನಾ http_parameter;
ಸಂರಚನಾ imagemap;
ಸಂರಚನಾ object_params;
ಸಂರಚನಾ plotters;
ಸಂರಚನಾ textarea;
ಸಂರಚನಾ scrollbar;
ಸಂರಚನಾ scrollbar_ಸಂದೇಶ_data;
ಸಂರಚನಾ ಹುಡುಕು_context;
ಸಂರಚನಾ selection;
ಸಂರಚನಾ nsurl;
ಸಂರಚನಾ plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ಸಂರಚನಾ html_stylesheet {
	ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು; /**< dom node associated with sheet */
	ಸಂರಚನಾ hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
ಸಂರಚನಾ html_script {
	/** Type of script */
	ಗಣನಾ html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	ಸಙ್ಘ {
		ಸಂರಚನಾ hlcache_handle *handle;
		ಸಂರಚನಾ dom_string *string;
	} data;	/**< Script data */
	ಸಂರಚನಾ dom_string *mimetype;
	ಸಂರಚನಾ dom_string *encoding;
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
ಸಂರಚನಾ content_html_object {
	ಸಂರಚನಾ content *parent;		/**< Parent document */
	ಸಂರಚನಾ content_html_object *next; /**< Next in chain */

	ಸಂರಚನಾ hlcache_handle *content;  /**< Content, or 0. */
	ಸಂರಚನಾ box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ಸಂರಚನಾ content_html_frames {
	ಪೂರ್ಣಾಙ್ಕ cols;	/** number of columns in frameset */
	ಪೂರ್ಣಾಙ್ಕ rows;	/** number of rows in frameset */

	ಸಂರಚನಾ frame_dimension width;	/** frame width */
	ಸಂರಚನಾ frame_dimension height;	/** frame width */
	ಪೂರ್ಣಾಙ್ಕ margin_width;	/** frame margin width */
	ಪೂರ್ಣಾಙ್ಕ margin_height;	/** frame margin height */

	ವರ್ಣ *name;	/** frame name (for targetting) */
	ಸಂರಚನಾ nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ಸಂರಚನಾ content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ಸಂರಚನಾ content_html_iframe {
	ಸಂರಚನಾ box *box;

	ಪೂರ್ಣಾಙ್ಕ margin_width;	/** frame margin width */
	ಪೂರ್ಣಾಙ್ಕ margin_height;	/** frame margin height */

	ವರ್ಣ *name;	/** frame name (for targetting) */
	ಸಂರಚನಾ nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ಸಂರಚನಾ content_html_iframe *next;
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
nserror html_init(ಶೂನ್ಯ);

/**
 * redraw a specific box
 *
 * used by core browser
 */
ಶೂನ್ಯ html_redraw_a_box(ಸಂರಚನಾ hlcache_handle *h, ಸಂರಚನಾ box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ಸಂರಚನಾ content_html_frames *html_get_frameset(ಸಂರಚನಾ hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ಸಂರಚನಾ content_html_iframe *html_get_iframe(ಸಂರಚನಾ hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ಧ್ರುವ ವರ್ಣ *html_get_base_target(ಸಂರಚನಾ hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
ಶೂನ್ಯ html_set_file_gadget_filename(ಸಂರಚನಾ hlcache_handle *hl,
	ಸಂರಚನಾ form_control *gadget, ಧ್ರುವ ವರ್ಣ *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ಸಂರಚನಾ html_stylesheet *html_get_stylesheets(ಸಂರಚನಾ hlcache_handle *h,
		unsigned ಪೂರ್ಣಾಙ್ಕ *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ಸಂರಚನಾ content_html_object *html_get_objects(ಸಂರಚನಾ hlcache_handle *h,
		unsigned ಪೂರ್ಣಾಙ್ಕ *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(ಸಂರಚನಾ hlcache_handle *h, lwc_string *frag_id,
		ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y);

#endif
