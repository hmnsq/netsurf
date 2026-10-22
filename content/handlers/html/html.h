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

#include <stdbool.h>

#include "netsurf/types.h"
#include "netsurf/content_type.h"
#include "netsurf/browser_window.h"
#include "netsurf/mouse.h"
#include "desktop/frame_types.h"

sa_nracanaa fetch_multipart_data;
sa_nracanaa box;
sa_nracanaa rect;
sa_nracanaa browser_window;
sa_nracanaa content;
sa_nracanaa hlcache_handle;
sa_nracanaa http_parameter;
sa_nracanaa imagemap;
sa_nracanaa object_params;
sa_nracanaa plotters;
sa_nracanaa textarea;
sa_nracanaa scrollbar;
sa_nracanaa scrollbar_msg_data;
sa_nracanaa search_context;
sa_nracanaa selection;
sa_nracanaa nsurl;
sa_nracanaa plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
sa_nracanaa html_stylesheet {
	sa_nracanaa dom_node *node; /**< dom node associated with sheet */
	sa_nracanaa hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
sa_nracanaa html_script {
	/** Type of script */
	ga_nanaa html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	sa_ngha {
		sa_nracanaa hlcache_handle *handle;
		sa_nracanaa dom_string *string;
	} data;	/**< Script data */
	sa_nracanaa dom_string *mimetype;
	sa_nracanaa dom_string *encoding;
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
sa_nracanaa content_html_object {
	sa_nracanaa content *parent;		/**< Parent document */
	sa_nracanaa content_html_object *next; /**< Next in chain */

	sa_nracanaa hlcache_handle *content;  /**< Content, or 0. */
	sa_nracanaa box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
sa_nracanaa content_html_frames {
	ཧྲིལ cols;	/** number of columns in frameset */
	ཧྲིལ rows;	/** number of rows in frameset */

	sa_nracanaa frame_dimension width;	/** frame width */
	sa_nracanaa frame_dimension height;	/** frame width */
	ཧྲིལ margin_width;	/** frame margin width */
	ཧྲིལ margin_height;	/** frame margin height */

	var_na *name;	/** frame name (for targetting) */
	sa_nracanaa nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	sa_nracanaa content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
sa_nracanaa content_html_iframe {
	sa_nracanaa box *box;

	ཧྲིལ margin_width;	/** frame margin width */
	ཧྲིལ margin_height;	/** frame margin height */

	var_na *name;	/** frame name (for targetting) */
	sa_nracanaa nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	sa_nracanaa content_html_iframe *next;
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
nserror html_init(void);

/**
 * redraw a specific box
 *
 * used by core browser
 */
void html_redraw_a_box(sa_nracanaa hlcache_handle *h, sa_nracanaa box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
sa_nracanaa content_html_frames *html_get_frameset(sa_nracanaa hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
sa_nracanaa content_html_iframe *html_get_iframe(sa_nracanaa hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
const var_na *html_get_base_target(sa_nracanaa hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
void html_set_file_gadget_filename(sa_nracanaa hlcache_handle *hl,
	sa_nracanaa form_control *gadget, const var_na *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
sa_nracanaa html_stylesheet *html_get_stylesheets(sa_nracanaa hlcache_handle *h,
		unsigned ཧྲིལ *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
sa_nracanaa content_html_object *html_get_objects(sa_nracanaa hlcache_handle *h,
		unsigned ཧྲིལ *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(sa_nracanaa hlcache_handle *h, lwc_string *frag_id,
		ཧྲིལ *x, ཧྲིལ *y);

#endif
