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

#अगर_अघोषित NETSURF_HTML_HTML_H
#घोषणा NETSURF_HTML_HTML_H

#समावेश <stdbool.h>

#समावेश "netsurf/types.h"
#समावेश "netsurf/content_type.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/mouse.h"
#समावेश "desktop/frame_types.h"

काष्ठा fetch_multipart_तथ्य;
काष्ठा box;
काष्ठा rect;
काष्ठा browser_window;
काष्ठा content;
काष्ठा hlcache_handle;
काष्ठा http_parameter;
काष्ठा imagemap;
काष्ठा object_params;
काष्ठा plotters;
काष्ठा textarea;
काष्ठा scrollbar;
काष्ठा scrollbar_संदेश_तथ्य;
काष्ठा खोज_context;
काष्ठा selection;
काष्ठा nsurl;
काष्ठा plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
काष्ठा html_stylesheet {
	काष्ठा dom_गाँठ *गाँठ; /**< dom node associated with sheet */
	काष्ठा hlcache_handle *sheet;
	bool modified;
	bool unused;
};


/**
 * Container for scripts used by an HTML document
 */
काष्ठा html_script {
	/** Type of script */
	क्रमागत html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	जोड़ {
		काष्ठा hlcache_handle *handle;
		काष्ठा dom_string *string;
	} तथ्य;	/**< Script data */
	काष्ठा dom_string *mimetype;
	काष्ठा dom_string *encoding;
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
काष्ठा content_html_object {
	काष्ठा content *parent;		/**< Parent document */
	काष्ठा content_html_object *अग्र; /**< Next in chain */

	काष्ठा hlcache_handle *content;  /**< Content, or 0. */
	काष्ठा box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
काष्ठा content_html_frames {
	पूर्णांक cols;	/** number of columns in frameset */
	पूर्णांक rows;	/** number of rows in frameset */

	काष्ठा frame_dimension width;	/** frame width */
	काष्ठा frame_dimension height;	/** frame width */
	पूर्णांक margin_width;	/** frame margin width */
	पूर्णांक margin_height;	/** frame margin height */

	अक्षर *name;	/** frame name (for targetting) */
	काष्ठा nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	काष्ठा content_html_frames *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
काष्ठा content_html_iframe {
	काष्ठा box *box;

	पूर्णांक margin_width;	/** frame margin width */
	पूर्णांक margin_height;	/** frame margin height */

	अक्षर *name;	/** frame name (for targetting) */
	काष्ठा nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	colour border_colour;	/** frame border colour */

	काष्ठा content_html_iframe *अग्र;
};

/* entries in stylesheet_content */
#घोषणा STYLESHEET_BASE		0	/* base style sheet */
#घोषणा STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#घोषणा STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#घोषणा STYLESHEET_USER		3	/* user stylesheet */
#घोषणा STYLESHEET_START	4	/* start of document stylesheets */

/**
 * initialise content handler
 *
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror html_init(व्योम);

/**
 * redraw a specific box
 *
 * used by core browser
 */
व्योम html_redraw_a_box(काष्ठा hlcache_handle *h, काष्ठा box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
काष्ठा content_html_frames *html_get_frameset(काष्ठा hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
काष्ठा content_html_iframe *html_get_iframe(काष्ठा hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
स्थिर अक्षर *html_get_base_target(काष्ठा hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
व्योम html_set_file_gadget_filename(काष्ठा hlcache_handle *hl,
	काष्ठा form_control *gadget, स्थिर अक्षर *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
काष्ठा html_stylesheet *html_get_stylesheets(काष्ठा hlcache_handle *h,
		अचिन्हित पूर्णांक *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
काष्ठा content_html_object *html_get_objects(काष्ठा hlcache_handle *h,
		अचिन्हित पूर्णांक *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_offset(काष्ठा hlcache_handle *h, lwc_string *frag_id,
		पूर्णांक *x, पूर्णांक *y);

#पूर्ण_अगर
