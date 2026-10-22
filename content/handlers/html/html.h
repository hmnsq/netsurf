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

#अन्तर्भाव <stdbool.h>

#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "netsurf/content_type.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/mouse.h"
#अन्तर्भाव "desktop/frame_types.h"

संरचना fetch_multipart_data;
संरचना box;
संरचना rect;
संरचना browser_window;
संरचना content;
संरचना hlcache_handle;
संरचना http_parameter;
संरचना imagemap;
संरचना object_params;
संरचना plotters;
संरचना textarea;
संरचना scrollbar;
संरचना scrollbar_संदेश_data;
संरचना खोज_context;
संरचना selection;
संरचना nsurl;
संरचना plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
संरचना html_stylesheet {
	संरचना dom_गाँठ *गाँठ; /**< dom node associated with sheet */
	संरचना hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
संरचना html_script {
	/** Type of script */
	गणना html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	सङ्घ {
		संरचना hlcache_handle *handle;
		संरचना dom_string *string;
	} data;	/**< Script data */
	संरचना dom_string *mimetype;
	संरचना dom_string *encoding;
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
संरचना content_html_object {
	संरचना content *parent;		/**< Parent document */
	संरचना content_html_object *next; /**< Next in chain */

	संरचना hlcache_handle *content;  /**< Content, or 0. */
	संरचना box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
संरचना content_html_frames {
	पूर्णाङ्क cols;	/** number of columns in frameset */
	पूर्णाङ्क rows;	/** number of rows in frameset */

	संरचना frame_dimension width;	/** frame width */
	संरचना frame_dimension height;	/** frame width */
	पूर्णाङ्क margin_width;	/** frame margin width */
	पूर्णाङ्क margin_height;	/** frame margin height */

	वर्ण *name;	/** frame name (for targetting) */
	संरचना nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	संरचना content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
संरचना content_html_iframe {
	संरचना box *box;

	पूर्णाङ्क margin_width;	/** frame margin width */
	पूर्णाङ्क margin_height;	/** frame margin height */

	वर्ण *name;	/** frame name (for targetting) */
	संरचना nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	संरचना content_html_iframe *next;
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
nserror html_init(शून्य);

/**
 * redraw a specific box
 *
 * used by core browser
 */
शून्य html_redraw_a_box(संरचना hlcache_handle *h, संरचना box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
संरचना content_html_frames *html_get_frameset(संरचना hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
संरचना content_html_iframe *html_get_iframe(संरचना hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ध्रुव वर्ण *html_get_base_target(संरचना hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
शून्य html_set_file_gadget_filename(संरचना hlcache_handle *hl,
	संरचना form_control *gadget, ध्रुव वर्ण *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
संरचना html_stylesheet *html_get_stylesheets(संरचना hlcache_handle *h,
		unsigned पूर्णाङ्क *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
संरचना content_html_object *html_get_objects(संरचना hlcache_handle *h,
		unsigned पूर्णाङ्क *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(संरचना hlcache_handle *h, lwc_string *frag_id,
		पूर्णाङ्क *x, पूर्णाङ्क *y);

#endif
