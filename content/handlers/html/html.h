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

#അന്തര്ഭാവ <stdbool.h>

#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "netsurf/content_type.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/mouse.h"
#അന്തര്ഭാവ "desktop/frame_types.h"

ഘടന fetch_multipart_data;
ഘടന box;
ഘടന rect;
ഘടന browser_window;
ഘടന content;
ഘടന hlcache_handle;
ഘടന http_parameter;
ഘടന imagemap;
ഘടന object_params;
ഘടന plotters;
ഘടന textarea;
ഘടന scrollbar;
ഘടന scrollbar_സന്ദേശം_data;
ഘടന തിരയൽ_context;
ഘടന selection;
ഘടന nsurl;
ഘടന plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ഘടന html_stylesheet {
	ഘടന dom_കണ്ണി *കണ്ണി; /**< dom node associated with sheet */
	ഘടന hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
ഘടന html_script {
	/** Type of script */
	എണ്ണൽ html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	സംഘം {
		ഘടന hlcache_handle *handle;
		ഘടന dom_string *string;
	} data;	/**< Script data */
	ഘടന dom_string *mimetype;
	ഘടന dom_string *encoding;
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
ഘടന content_html_object {
	ഘടന content *parent;		/**< Parent document */
	ഘടന content_html_object *next; /**< Next in chain */

	ഘടന hlcache_handle *content;  /**< Content, or 0. */
	ഘടന box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ഘടന content_html_frames {
	പൂർണ്ണസംഖ്യ cols;	/** number of columns in frameset */
	പൂർണ്ണസംഖ്യ rows;	/** number of rows in frameset */

	ഘടന frame_dimension width;	/** frame width */
	ഘടന frame_dimension height;	/** frame width */
	പൂർണ്ണസംഖ്യ margin_width;	/** frame margin width */
	പൂർണ്ണസംഖ്യ margin_height;	/** frame margin height */

	അക്ഷരം *name;	/** frame name (for targetting) */
	ഘടന nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ഘടന content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ഘടന content_html_iframe {
	ഘടന box *box;

	പൂർണ്ണസംഖ്യ margin_width;	/** frame margin width */
	പൂർണ്ണസംഖ്യ margin_height;	/** frame margin height */

	അക്ഷരം *name;	/** frame name (for targetting) */
	ഘടന nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ഘടന content_html_iframe *next;
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
nserror html_init(ശൂന്യം);

/**
 * redraw a specific box
 *
 * used by core browser
 */
ശൂന്യം html_redraw_a_box(ഘടന hlcache_handle *h, ഘടന box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ഘടന content_html_frames *html_get_frameset(ഘടന hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ഘടന content_html_iframe *html_get_iframe(ഘടന hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
സ്ഥിരം അക്ഷരം *html_get_base_target(ഘടന hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
ശൂന്യം html_set_file_gadget_filename(ഘടന hlcache_handle *hl,
	ഘടന form_control *gadget, സ്ഥിരം അക്ഷരം *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ഘടന html_stylesheet *html_get_stylesheets(ഘടന hlcache_handle *h,
		unsigned പൂർണ്ണസംഖ്യ *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ഘടന content_html_object *html_get_objects(ഘടന hlcache_handle *h,
		unsigned പൂർണ്ണസംഖ്യ *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(ഘടന hlcache_handle *h, lwc_string *frag_id,
		പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y);

#endif
