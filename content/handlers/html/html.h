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

#અન્તર્ભાવ <stdbool.h>

#અન્તર્ભાવ "netsurf/types.h"
#અન્તર્ભાવ "netsurf/content_type.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/mouse.h"
#અન્તર્ભાવ "desktop/frame_types.h"

સંરચના fetch_multipart_data;
સંરચના box;
સંરચના rect;
સંરચના browser_window;
સંરચના content;
સંરચના hlcache_handle;
સંરચના http_parameter;
સંરચના imagemap;
સંરચના object_params;
સંરચના plotters;
સંરચના textarea;
સંરચના scrollbar;
સંરચના scrollbar_સંદેશ_data;
સંરચના શોધ_context;
સંરચના selection;
સંરચના nsurl;
સંરચના plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
સંરચના html_stylesheet {
	સંરચના dom_ગાંઠ *ગાંઠ; /**< dom node associated with sheet */
	સંરચના hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
સંરચના html_script {
	/** Type of script */
	ગણના html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	સંઘ {
		સંરચના hlcache_handle *handle;
		સંરચના dom_string *string;
	} data;	/**< Script data */
	સંરચના dom_string *mimetype;
	સંરચના dom_string *encoding;
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
સંરચના content_html_object {
	સંરચના content *parent;		/**< Parent document */
	સંરચના content_html_object *next; /**< Next in chain */

	સંરચના hlcache_handle *content;  /**< Content, or 0. */
	સંરચના box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
સંરચના content_html_frames {
	પૂર્ણાંક cols;	/** number of columns in frameset */
	પૂર્ણાંક rows;	/** number of rows in frameset */

	સંરચના frame_dimension width;	/** frame width */
	સંરચના frame_dimension height;	/** frame width */
	પૂર્ણાંક margin_width;	/** frame margin width */
	પૂર્ણાંક margin_height;	/** frame margin height */

	અક્ષર *name;	/** frame name (for targetting) */
	સંરચના nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	સંરચના content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
સંરચના content_html_iframe {
	સંરચના box *box;

	પૂર્ણાંક margin_width;	/** frame margin width */
	પૂર્ણાંક margin_height;	/** frame margin height */

	અક્ષર *name;	/** frame name (for targetting) */
	સંરચના nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	સંરચના content_html_iframe *next;
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
nserror html_init(શૂન્ય);

/**
 * redraw a specific box
 *
 * used by core browser
 */
શૂન્ય html_redraw_a_box(સંરચના hlcache_handle *h, સંરચના box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
સંરચના content_html_frames *html_get_frameset(સંરચના hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
સંરચના content_html_iframe *html_get_iframe(સંરચના hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
અચલ અક્ષર *html_get_base_target(સંરચના hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
શૂન્ય html_set_file_gadget_filename(સંરચના hlcache_handle *hl,
	સંરચના form_control *gadget, અચલ અક્ષર *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
સંરચના html_stylesheet *html_get_stylesheets(સંરચના hlcache_handle *h,
		unsigned પૂર્ણાંક *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
સંરચના content_html_object *html_get_objects(સંરચના hlcache_handle *h,
		unsigned પૂર્ણાંક *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(સંરચના hlcache_handle *h, lwc_string *frag_id,
		પૂર્ણાંક *x, પૂર્ણાંક *y);

#endif
