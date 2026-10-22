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

#ضمن <stdbool.h>

#ضمن "netsurf/types.h"
#ضمن "netsurf/content_type.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/mouse.h"
#ضمن "desktop/frame_types.h"

بنية fetch_multipart_data;
بنية box;
بنية rect;
بنية browser_window;
بنية content;
بنية hlcache_handle;
بنية http_parameter;
بنية imagemap;
بنية object_params;
بنية plotters;
بنية textarea;
بنية scrollbar;
بنية scrollbar_سكو_data;
بنية نيما_context;
بنية selection;
بنية nsurl;
بنية plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
بنية html_stylesheet {
	بنية dom_كلي *كلي; /**< dom node associated with sheet */
	بنية hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
بنية html_script {
	/** Type of script */
	تعداد html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	اتحاد {
		بنية hlcache_handle *handle;
		بنية dom_string *string;
	} data;	/**< Script data */
	بنية dom_string *mimetype;
	بنية dom_string *encoding;
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
بنية content_html_object {
	بنية content *parent;		/**< Parent document */
	بنية content_html_object *next; /**< Next in chain */

	بنية hlcache_handle *content;  /**< Content, or 0. */
	بنية box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
بنية content_html_frames {
	صحيح cols;	/** number of columns in frameset */
	صحيح rows;	/** number of rows in frameset */

	بنية frame_dimension width;	/** frame width */
	بنية frame_dimension height;	/** frame width */
	صحيح margin_width;	/** frame margin width */
	صحيح margin_height;	/** frame margin height */

	حرف *name;	/** frame name (for targetting) */
	بنية nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	بنية content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
بنية content_html_iframe {
	بنية box *box;

	صحيح margin_width;	/** frame margin width */
	صحيح margin_height;	/** frame margin height */

	حرف *name;	/** frame name (for targetting) */
	بنية nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	بنية content_html_iframe *next;
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
nserror html_init(فراغ);

/**
 * redraw a specific box
 *
 * used by core browser
 */
فراغ html_redraw_a_box(بنية hlcache_handle *h, بنية box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
بنية content_html_frames *html_get_frameset(بنية hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
بنية content_html_iframe *html_get_iframe(بنية hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ثابت حرف *html_get_base_target(بنية hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
فراغ html_set_file_gadget_filename(بنية hlcache_handle *hl,
	بنية form_control *gadget, ثابت حرف *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
بنية html_stylesheet *html_get_stylesheets(بنية hlcache_handle *h,
		unsigned صحيح *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
بنية content_html_object *html_get_objects(بنية hlcache_handle *h,
		unsigned صحيح *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(بنية hlcache_handle *h, lwc_string *frag_id,
		صحيح *x, صحيح *y);

#endif
