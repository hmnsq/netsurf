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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content_type.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h"
#ଅନ୍ତର୍ଭାଵ "desktop/frame_types.h"

ସଂରଚନା fetch_multipart_data;
ସଂରଚନା box;
ସଂରଚନା rect;
ସଂରଚନା browser_window;
ସଂରଚନା content;
ସଂରଚନା hlcache_handle;
ସଂରଚନା http_parameter;
ସଂରଚନା imagemap;
ସଂରଚନା object_params;
ସଂରଚନା plotters;
ସଂରଚନା textarea;
ସଂରଚନା scrollbar;
ସଂରଚନା scrollbar_ସନ୍ଦେଶ_data;
ସଂରଚନା ସନ୍ଧାନ_context;
ସଂରଚନା selection;
ସଂରଚନା nsurl;
ସଂରଚନା plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ସଂରଚନା html_stylesheet {
	ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି; /**< dom node associated with sheet */
	ସଂରଚନା hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
ସଂରଚନା html_script {
	/** Type of script */
	ଗଣନା html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	ସଂଘ {
		ସଂରଚନା hlcache_handle *handle;
		ସଂରଚନା dom_string *string;
	} data;	/**< Script data */
	ସଂରଚନା dom_string *mimetype;
	ସଂରଚନା dom_string *encoding;
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
ସଂରଚନା content_html_object {
	ସଂରଚନା content *parent;		/**< Parent document */
	ସଂରଚନା content_html_object *next; /**< Next in chain */

	ସଂରଚନା hlcache_handle *content;  /**< Content, or 0. */
	ସଂରଚନା box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ସଂରଚନା content_html_frames {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cols;	/** number of columns in frameset */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows;	/** number of rows in frameset */

	ସଂରଚନା frame_dimension width;	/** frame width */
	ସଂରଚନା frame_dimension height;	/** frame width */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_width;	/** frame margin width */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_height;	/** frame margin height */

	ଅକ୍ଷର *name;	/** frame name (for targetting) */
	ସଂରଚନା nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ସଂରଚନା content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ସଂରଚନା content_html_iframe {
	ସଂରଚନା box *box;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_width;	/** frame margin width */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_height;	/** frame margin height */

	ଅକ୍ଷର *name;	/** frame name (for targetting) */
	ସଂରଚନା nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	ସଂରଚନା content_html_iframe *next;
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
nserror html_init(ଶୂନ୍ୟ);

/**
 * redraw a specific box
 *
 * used by core browser
 */
ଶୂନ୍ୟ html_redraw_a_box(ସଂରଚନା hlcache_handle *h, ସଂରଚନା box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ସଂରଚନା content_html_frames *html_get_frameset(ସଂରଚନା hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ସଂରଚନା content_html_iframe *html_get_iframe(ସଂରଚନା hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ଅଚଳ ଅକ୍ଷର *html_get_base_target(ସଂରଚନା hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
ଶୂନ୍ୟ html_set_file_gadget_filename(ସଂରଚନା hlcache_handle *hl,
	ସଂରଚନା form_control *gadget, ଅଚଳ ଅକ୍ଷର *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ସଂରଚନା html_stylesheet *html_get_stylesheets(ସଂରଚନା hlcache_handle *h,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ସଂରଚନା content_html_object *html_get_objects(ସଂରଚନା hlcache_handle *h,
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(ସଂରଚନା hlcache_handle *h, lwc_string *frag_id,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y);

#endif
