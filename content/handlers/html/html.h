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

#నిర్వచితం_కాకపోతే NETSURF_HTML_HTML_H
#నిర్వచించు NETSURF_HTML_HTML_H

#అన్తర్భావ <stdbool.h>

#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "netsurf/content_type.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/mouse.h"
#అన్తర్భావ "desktop/frame_types.h"

నిర్మాణం fetch_multipart_దత్తాంశం;
నిర్మాణం box;
నిర్మాణం rect;
నిర్మాణం browser_కిటికీ;
నిర్మాణం content;
నిర్మాణం hlcache_handle;
నిర్మాణం http_parameter;
నిర్మాణం imagemap;
నిర్మాణం object_params;
నిర్మాణం plotters;
నిర్మాణం textarea;
నిర్మాణం scrollbar;
నిర్మాణం scrollbar_సందేశం_దత్తాంశం;
నిర్మాణం శోధన_context;
నిర్మాణం selection;
నిర్మాణం nsurl;
నిర్మాణం plot_font_style;

/**
 * Container for stylesheets used by an HTML document
 */
నిర్మాణం html_stylesheet {
	నిర్మాణం dom_కణుపు *కణుపు; /**< dom node associated with sheet */
	నిర్మాణం hlcache_handle *sheet;
	bool modified;
	bool వాడని;
};


/**
 * Container for scripts used by an HTML document
 */
నిర్మాణం html_script {
	/** Type of script */
	గణన html_script_type { HTML_SCRIPT_INLINE,
				HTML_SCRIPT_SYNC,
				HTML_SCRIPT_DEFER,
				HTML_SCRIPT_ASYNC } type;
	సంఘం {
		నిర్మాణం hlcache_handle *handle;
		నిర్మాణం dom_string *string;
	} దత్తాంశం;	/**< Script data */
	నిర్మాణం dom_string *mimetype;
	నిర్మాణం dom_string *encoding;
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
నిర్మాణం content_html_object {
	నిర్మాణం content *మాతృ;		/**< Parent document */
	నిర్మాణం content_html_object *తదుపరి; /**< Next in chain */

	నిర్మాణం hlcache_handle *content;  /**< Content, or 0. */
	నిర్మాణం box *box;  /**< Node in box tree containing it. */
	/** Bitmap of acceptable content types */
	content_type permitted_types;
	bool background;  /**< This object is a background image. */
};


/**
 * Frame tree (frameset or frame tag)
 */
నిర్మాణం content_html_చట్రాలు {
	పూర్ణాంకం నిలువుల_సంఖ్య;	/** number of columns in frameset */
	పూర్ణాంకం అడ్డువరుసలు;	/** number of rows in frameset */

	నిర్మాణం చట్రం_dimension వెడల్పు;	/** frame width */
	నిర్మాణం చట్రం_dimension ఎత్తు;	/** frame width */
	పూర్ణాంకం margin_వెడల్పు;	/** frame margin width */
	పూర్ణాంకం margin_ఎత్తు;	/** frame margin height */

	అక్షరం *name;	/** frame name (for targetting) */
	నిర్మాణం nsurl *url;	/** frame url */

	bool no_resize;	/** frame is not resizable */
	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	రంగు border_రంగు;	/** frame border colour */

	నిర్మాణం content_html_చట్రాలు *children; /** [cols * rows] children */
};

/**
 * Inline frame list (iframe tag)
 */
నిర్మాణం content_html_iframe {
	నిర్మాణం box *box;

	పూర్ణాంకం margin_వెడల్పు;	/** frame margin width */
	పూర్ణాంకం margin_ఎత్తు;	/** frame margin height */

	అక్షరం *name;	/** frame name (for targetting) */
	నిర్మాణం nsurl *url;	/** frame url */

	browser_scrolling scrolling;	/** scrolling characteristics */
	bool border;	/** frame has a border */
	రంగు border_రంగు;	/** frame border colour */

	నిర్మాణం content_html_iframe *తదుపరి;
};

/* entries in stylesheet_content */
#నిర్వచించు STYLESHEET_పునాది		0	/* base style sheet */
#నిర్వచించు STYLESHEET_QUIRKS	1	/* quirks mode stylesheet */
#నిర్వచించు STYLESHEET_ADBLOCK	2	/* adblocking stylesheet */
#నిర్వచించు STYLESHEET_USER		3	/* user stylesheet */
#నిర్వచించు STYLESHEET_ప్రారంభం	4	/* start of document stylesheets */

/**
 * initialise content handler
 *
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror html_init(శూన్య);

/**
 * redraw a specific box
 *
 * used by core browser
 */
శూన్య html_redraw_a_box(నిర్మాణం hlcache_handle *h, నిర్మాణం box *box);

/**
 * obtain html frame content from handle
 *
 * used by core browser
 */
నిర్మాణం content_html_చట్రాలు *html_get_frameset(నిర్మాణం hlcache_handle *h);

/**
 * obtain html iframe content from handle
 *
 * used by core browser
 */
నిర్మాణం content_html_iframe *html_get_iframe(నిర్మాణం hlcache_handle *h);

/**
 * obtain html base target from handle
 *
 * used by core browser
 */
స్థిరరాశి అక్షరం *html_get_పునాది_లక్ష్యం(నిర్మాణం hlcache_handle *h);

/**
 * set filename on a file gadget
 *
 * used by core browser
 */
శూన్య html_సమితి_file_gadget_filename_2(నిర్మాణం hlcache_handle *hl,
	నిర్మాణం form_control *gadget, స్థిరరాశి అక్షరం *fn);

/**
 * Retrieve stylesheets used by HTML document
 *
 * \param h Content to retrieve stylesheets from
 * \param n Pointer to location to receive number of sheets
 * \return Pointer to array of stylesheets
 */
నిర్మాణం html_stylesheet *html_get_stylesheets(నిర్మాణం hlcache_handle *h,
		గుర్తులేని పూర్ణాంకం *n);

/**
 * Retrieve objects used by HTML document
 *
 * \param h Content to retrieve objects from
 * \param n Pointer to location to receive number of objects
 * \return Pointer to array of objects
 */
నిర్మాణం content_html_object *html_get_objects(నిర్మాణం hlcache_handle *h,
		గుర్తులేని పూర్ణాంకం *n);

/**
 * get the offset within the docuemnt of a fragment id
 */
bool html_get_id_స్థానభ్రంశం(నిర్మాణం hlcache_handle *h, lwc_string *frag_id,
		పూర్ణాంకం *x, పూర్ణాంకం *y);

#అయితే_ముగింపు
