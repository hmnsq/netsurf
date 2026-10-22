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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_HTML_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_HTML_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content_type.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "desktop/frame_types.h"

ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ;
ರಚನೆ box;
ರಚನೆ rect;
ರಚನೆ browser_ಕಿಟಕಿ;
ರಚನೆ content;
ರಚನೆ hlcache_handle;
ರಚನೆ http_parameter;
ರಚನೆ imagemap;
ರಚನೆ object_params;
ರಚನೆ plotters;
ರಚನೆ textarea;
ರಚನೆ scrollbar;
ರಚನೆ scrollbar_ಸಂದೇಶ_ದತ್ತಾಂಶ;
ರಚನೆ ಹುಡುಕು_context;
ರಚನೆ selection;
ರಚನೆ nsurl;
ರಚನೆ plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ರಚನೆ html_stylesheet {
	ರಚನೆ dom_ಗಂಟು *ಗಂಟು; /**< dom node associated with sheet */
	ರಚನೆ hlcache_handle *sheet;
	bool modified;
	bool ಬಳಸದ;
};


/**
 * Container for scripts used by an HTML document
 */
ರಚನೆ html_script {
	/** Type of script */
	ಎಣಿಕೆ html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	ಒಕ್ಕೂಟ {
		ರಚನೆ hlcache_handle *handle;
		ರಚನೆ dom_string *string;
	} ದತ್ತಾಂಶ;	/**< Script data */
	ರಚನೆ dom_string *mimetype;
	ರಚನೆ dom_string *encoding;
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
ರಚನೆ content_html_object {
	ರಚನೆ content *ಪೋಷಕ;		/**< Parent document */
	ರಚನೆ content_html_object *ಮುಂದಿನ; /**< Next in chain */

	ರಚನೆ hlcache_handle *content;  /**< Content, or 0. */
	ರಚನೆ box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು {
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;	/** number of columns in frameset */
	ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು;	/** number of rows in frameset */

	ರಚನೆ ಚೌಕಟ್ಟು_dimension ಅಗಲ;	/** frame width */
	ರಚನೆ ಚೌಕಟ್ಟು_dimension ಎತ್ತರ;	/** frame width */
	ಪೂರ್ಣಾಂಕ margin_ಅಗಲ;	/** frame margin width */
	ಪೂರ್ಣಾಂಕ margin_ಎತ್ತರ;	/** frame margin height */

	ಅಕ್ಷರ *name;	/** frame name (for targetting) */
	ರಚನೆ nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	ಬಣ್ಣ border_ಬಣ್ಣ;	/** frame border colour */

	ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ರಚನೆ content_html_iframe {
	ರಚನೆ box *box;

	ಪೂರ್ಣಾಂಕ margin_ಅಗಲ;	/** frame margin width */
	ಪೂರ್ಣಾಂಕ margin_ಎತ್ತರ;	/** frame margin height */

	ಅಕ್ಷರ *name;	/** frame name (for targetting) */
	ರಚನೆ nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	ಬಣ್ಣ border_ಬಣ್ಣ;	/** frame border colour */

	ರಚನೆ content_html_iframe *ಮುಂದಿನ;
};

/* entries in stylesheet_content */
#ವ್ಯಾಖ್ಯಾನಿಸು STYLESHEET_ತಳ		0	/* base style sheet */
#ವ್ಯಾಖ್ಯಾನಿಸು STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#ವ್ಯಾಖ್ಯಾನಿಸು STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#ವ್ಯಾಖ್ಯಾನಿಸು STYLESHEET_USER		3	/* user stylesheet */
#ವ್ಯಾಖ್ಯಾನಿಸು STYLESHEET_ಆರಂಭ	4	/* start of document stylesheets */

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
ಶೂನ್ಯ html_redraw_a_box(ರಚನೆ hlcache_handle *h, ರಚನೆ box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *html_get_frameset(ರಚನೆ hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ರಚನೆ content_html_iframe *html_get_iframe(ರಚನೆ hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *html_get_ತಳ_ಗುರಿ(ರಚನೆ hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
ಶೂನ್ಯ html_ಗಣ_file_gadget_filename_2(ರಚನೆ hlcache_handle *hl,
	ರಚನೆ form_control *gadget, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ರಚನೆ html_stylesheet *html_get_stylesheets(ರಚನೆ hlcache_handle *h,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ರಚನೆ content_html_object *html_get_objects(ರಚನೆ hlcache_handle *h,
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_ಸ್ಥಾನಾಂತರ(ರಚನೆ hlcache_handle *h, lwc_string *frag_id,
		ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y);

#ಆದರೆ_ಅಂತ್ಯ
