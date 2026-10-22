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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_HTML_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_HTML_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content_type.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/frame_types.h"

ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ;
ਬਣਤਰ box;
ਬਣਤਰ rect;
ਬਣਤਰ browser_ਖਿੜਕੀ;
ਬਣਤਰ content;
ਬਣਤਰ hlcache_handle;
ਬਣਤਰ http_parameter;
ਬਣਤਰ imagemap;
ਬਣਤਰ object_params;
ਬਣਤਰ plotters;
ਬਣਤਰ textarea;
ਬਣਤਰ scrollbar;
ਬਣਤਰ scrollbar_ਸੁਨੇਹਾ_ਅੰਕੜੇ;
ਬਣਤਰ ਖੋਜ_context;
ਬਣਤਰ selection;
ਬਣਤਰ nsurl;
ਬਣਤਰ plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
ਬਣਤਰ html_stylesheet {
	ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ; /**< dom node associated with sheet */
	ਬਣਤਰ hlcache_handle *sheet;
	bool modified;
	bool ਅਣਵਰਤਿਆ;
};


/**
 * Container for scripts used by an HTML document
 */
ਬਣਤਰ html_script {
	/** Type of script */
	ਗਿਣਤੀ html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	ਸੰਘ {
		ਬਣਤਰ hlcache_handle *handle;
		ਬਣਤਰ dom_string *string;
	} ਅੰਕੜੇ;	/**< Script data */
	ਬਣਤਰ dom_string *mimetype;
	ਬਣਤਰ dom_string *encoding;
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
ਬਣਤਰ content_html_object {
	ਬਣਤਰ content *ਮਾਪਾ;		/**< Parent document */
	ਬਣਤਰ content_html_object *ਅਗਲਾ; /**< Next in chain */

	ਬਣਤਰ hlcache_handle *content;  /**< Content, or 0. */
	ਬਣਤਰ box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
ਬਣਤਰ content_html_ਚੌਖਟੇ {
	ਪੂਰਨ ਖਾਨੇ_ਗਿਣਤੀ;	/** number of columns in frameset */
	ਪੂਰਨ ਪੰਕਤੀਆਂ;	/** number of rows in frameset */

	ਬਣਤਰ ਚੌਖਟਾ_dimension ਚੌੜਾਈ;	/** frame width */
	ਬਣਤਰ ਚੌਖਟਾ_dimension ਉਚਾਈ;	/** frame width */
	ਪੂਰਨ margin_ਚੌੜਾਈ;	/** frame margin width */
	ਪੂਰਨ margin_ਉਚਾਈ;	/** frame margin height */

	ਅੱਖਰ *name;	/** frame name (for targetting) */
	ਬਣਤਰ nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	ਰੰਗ border_ਰੰਗ;	/** frame border colour */

	ਬਣਤਰ content_html_ਚੌਖਟੇ *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
ਬਣਤਰ content_html_iframe {
	ਬਣਤਰ box *box;

	ਪੂਰਨ margin_ਚੌੜਾਈ;	/** frame margin width */
	ਪੂਰਨ margin_ਉਚਾਈ;	/** frame margin height */

	ਅੱਖਰ *name;	/** frame name (for targetting) */
	ਬਣਤਰ nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	ਰੰਗ border_ਰੰਗ;	/** frame border colour */

	ਬਣਤਰ content_html_iframe *ਅਗਲਾ;
};

/* entries in stylesheet_content */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STYLESHEET_ਅਧਾਰ		0	/* base style sheet */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STYLESHEET_USER		3	/* user stylesheet */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STYLESHEET_ਸ਼ੁਰੂ	4	/* start of document stylesheets */

/**
 * initialise content handler
 *
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror html_init(ਖਾਲੀ);

/**
 * redraw a specific box
 *
 * used by core browser
 */
ਖਾਲੀ html_redraw_a_box(ਬਣਤਰ hlcache_handle *h, ਬਣਤਰ box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
ਬਣਤਰ content_html_ਚੌਖਟੇ *html_get_frameset(ਬਣਤਰ hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
ਬਣਤਰ content_html_iframe *html_get_iframe(ਬਣਤਰ hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
ਅਚਲ ਅੱਖਰ *html_get_ਅਧਾਰ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
ਖਾਲੀ html_ਸੈੱਟ_file_gadget_filename_2(ਬਣਤਰ hlcache_handle *hl,
	ਬਣਤਰ form_control *gadget, ਅਚਲ ਅੱਖਰ *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
ਬਣਤਰ html_stylesheet *html_get_stylesheets(ਬਣਤਰ hlcache_handle *h,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
ਬਣਤਰ content_html_object *html_get_objects(ਬਣਤਰ hlcache_handle *h,
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_ਵਿੱਥ(ਬਣਤਰ hlcache_handle *h, lwc_string *frag_id,
		ਪੂਰਨ *x, ਪੂਰਨ *y);

#ਜੇ_ਅੰਤ
