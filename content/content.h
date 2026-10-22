/*
 * Copyright 2005-2007 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Philip Pemberton <philpem@users.sourceforge.net>
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
 * Content handling interface.
 *
 * The content functions manipulate struct contents, which correspond to URLs.
 */

#ifndef NETSURF_CONTENT_CONTENT_H_
#define NETSURF_CONTENT_CONTENT_H_

#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/content_type.h"
#अन्तर्भाव "netsurf/mouse.h" /* mouse state enums */
#अन्तर्भाव "netsurf/console.h" /* console state and flags enums */

संरचना browser_window;
संरचना browser_window_features;
संरचना content;
संरचना llcache_handle;
संरचना hlcache_handle;
संरचना object_params;
संरचना rect;
संरचना redraw_context;
संरचना cert_chain;


/** RFC5988 metadata link */
संरचना content_rfc5988_link {
	संरचना content_rfc5988_link *next; /**< next rfc5988_link in list */

	lwc_string *rel; /**< the link relationship - must be present */
	संरचना nsurl *href; /**< the link href - must be present */
	lwc_string *hreflang;
	lwc_string *type;
	lwc_string *media;
	lwc_string *sizes;
};

/** Extra data for some content_msg messages. */
सङ्घ content_संदेश_data {
	/**
	 * CONTENT_MSG_LOG - Information for logging
	 */
	संरचना {
		/** The source of the logging */
		browser_window_console_source src;
		/** The message to log */
		ध्रुव वर्ण *संदेश;
		/** The length of that message */
		size_t msglen;
		/** The flags of the logging */
		browser_window_console_flags flags;
	} log;

	/**
	 * CONTENT_MSG_SSL_CERTS - The certificate chain from the
	 *   underlying fetch
	 */
	ध्रुव संरचना cert_chain *chain;

	/**
	 * CONTENT_MSG_ERROR - Error from content or underlying fetch
	 */
	संरचना {
		/**
		 * The error code to convey meaning
		 */
		nserror errorcode;
		/**
		 * The message.  if NSERROR_UNKNOWN then this is the direct
		 *   message, otherwise is some kind of metadata (e.g. a
		 *   message name or somesuch) but always a null terminated
		 *   string.
		 */
		ध्रुव वर्ण *errormsg;
	} errordata;

	/**
	 * CONTENT_MSG_REDIRECT - Redirect info
	 */
	संरचना {
		संरचना nsurl *from;	/**< Redirect origin */
		संरचना nsurl *to;	/**< Redirect target */
	} redirect;		/**< Fetch URL redirect occured */

	/**
	 * CONTENT_MSG_REDRAW - Area of content which needs redrawing
	 */
	संरचना {
		पूर्णाङ्क x, y, width, height;
	} redraw;

	/**
	 * CONTENT_MSG_REFRESH - Minimum delay
	 */
	पूर्णाङ्क delay;

	/**
	 * CONTENT_MSG_REFORMAT - Reformat should not cause a redraw
	 */
	bool background;

	/**
	 * CONTENT_MSG_STATUS - Status message update.  If NULL, the
	 * content's internal status text has been updated, and
	 * listener should use content_get_status_message()
	 */
	ध्रुव वर्ण *explicit_status_text;

	/**
	 * CONTENT_MSG_DOWNLOAD - Low-level cache handle
	 */
	संरचना llcache_handle *download;

	/**
	 * CONTENT_MSG_RFC5988_LINK - rfc5988 link data
	 */
	संरचना content_rfc5988_link *rfc5988_link;

	/**
	 * CONTENT_MSG_GETTHREAD - Javascript context (thread)
	 */
	संरचना jsthread **jsthread;

	/**
	 * CONTENT_MSG_GETDIMS - Get the viewport dimensions
	 */
	संरचना {
		/** \todo Consider getting screen_width, screen_height too. */
		unsigned *viewport_width;
		unsigned *viewport_height;
	} getdims;

	/**
	 * CONTENT_MSG_SCROLL - Part of content to scroll to show
	 */
	संरचना {
		/*
		 * if true, scroll to show area given by (x0, y0) and (x1,y1).
		 * if false, scroll point (x0, y0) to top left of viewport
		 */
		bool area;
		पूर्णाङ्क x0, y0;
		पूर्णाङ्क x1, y1;
	} scroll;

	/**
	 * CONTENT_MSG_DRAGSAVE - Drag save a content
	 */
	संरचना {
		गणना {
			CONTENT_SAVE_ORIG,
			CONTENT_SAVE_NATIVE,
			CONTENT_SAVE_COMPLETE,
			CONTENT_SAVE_SOURCE
		} type;
		 /** if NULL, save the content generating the message */
		संरचना hlcache_handle *content;
	} dragsave;

	/**
	 * CONTENT_MSG_SAVELINK - Save a URL
	 */
	संरचना {
		संरचना nsurl *url;
		ध्रुव वर्ण *title;
	} savelink;

	/**
	 * CONTENT_MSG_POINTER - Mouse pointer to set
	 */
	browser_pointer_shape pointer;

	/**
	 * CONTENT_MSG_SELECTION - Selection made or cleared
	 */
	संरचना {
		bool selection; /**< false for selection cleared */
		bool read_only;
	} selection;

	/**
	 * CONTENT_MSG_CARET - set caret position or, hide caret
	 */
	संरचना {
		गणना {
			CONTENT_CARET_SET_POS,
			CONTENT_CARET_HIDE,
			CONTENT_CARET_REMOVE
		} type;
		संरचना {
			पूर्णाङ्क x;				/**< Carret x-coord */
			पूर्णाङ्क y;				/**< Carret y-coord */
			पूर्णाङ्क height;			/**< Carret height */
			ध्रुव संरचना rect *clip;	/**< Carret clip rect */
		} pos;			/**< With CONTENT_CARET_SET_POS */
	} caret;

	/**
	 * CONTENT_MSG_DRAG - Drag start or end
	 */
	संरचना {
		गणना {
			CONTENT_DRAG_NONE,
			CONTENT_DRAG_SCROLL,
			CONTENT_DRAG_SELECTION
		} type;
		ध्रुव संरचना rect *rect;
	} drag;

	/**
	 * CONTENT_MSG_SELECTMENU - Create select menu at pointer
	 */
	संरचना {
		संरचना form_control *gadget;
	} select_menu;

	/**
	 * CONTENT_MSG_GADGETCLICK - User clicked on a form gadget
	 */
	संरचना {
		संरचना form_control *gadget;
	} gadget_click;

	/**
	 * CONTENT_MSG_TEXTSEARCH - Free text search action
	 */
	संरचना {
		/**
		 * The type of text search operation
		 */
		गणना {
		      /**
		       * Free text search find operation has started or finished
		       */
		      CONTENT_TEXTSEARCH_FIND,
		      /**
		       * Free text search match state has changed
		       */
		      CONTENT_TEXTSEARCH_MATCH,
		      /**
		       * Free text search back available state changed
		       */
		      CONTENT_TEXTSEARCH_BACK,
		      /**
		       * Free text search forward available state changed
		       */
		      CONTENT_TEXTSEARCH_FORWARD,
		      /**
		       * add a search query string to the recent searches
		       */
		      CONTENT_TEXTSEARCH_RECENT
		} type;
		/**
		 * context passed to browser_window_search()
		 */
		शून्य *ctx;
		/**
		 * state for operation
		 */
		bool state;
		/**
		 * search string
		 */
		ध्रुव वर्ण *string;
	} textsearch;

};


/**
 * Get whether a content can reformat
 *
 * \param h  content to check
 * \return whether the content can reformat
 */
bool content_can_reformat(संरचना hlcache_handle *h);

/**
 * Reformat to new size.
 *
 * Calls the reformat function for the content.
 */
शून्य content_reformat(संरचना hlcache_handle *h, bool background,
		पूर्णाङ्क width, पूर्णाङ्क height);

/**
 * Request a redraw of an area of a content
 *
 * \param h	  high-level cache handle
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
शून्य content_request_redraw(संरचना hlcache_handle *h,
		पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height);

/**
 * Handle mouse movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
शून्य content_mouse_track(संरचना hlcache_handle *h, संरचना browser_window *bw,
		browser_mouse_state mouse, पूर्णाङ्क x, पूर्णाङ्क y);

/**
 * Handle mouse clicks and movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 *
 * This function handles both hovering and clicking. It is important that the
 * code path is identical (except that hovering doesn't carry out the action),
 * so that the status bar reflects exactly what will happen. Having separate
 * code paths opens the possibility that an attacker will make the status bar
 * show some harmless action where clicking will be harmful.
 */
शून्य content_mouse_action(संरचना hlcache_handle *h, संरचना browser_window *bw,
		browser_mouse_state mouse, पूर्णाङ्क x, पूर्णाङ्क y);

/**
 * Handle keypresses.
 *
 * \param  h	Content handle
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool content_keypress(संरचना hlcache_handle *h, uint32_t कुंजी);


/**
 * A window containing the content has been opened.
 *
 * \param h	 handle to content that has been opened
 * \param bw	 browser window containing the content
 * \param page   content of type CONTENT_HTML containing h, or NULL if not an
 *		   object within a page
 * \param params object parameters, or NULL if not an object
 *
 * Calls the open function for the content.
 */
nserror content_open(संरचना hlcache_handle *h, संरचना browser_window *bw,
		संरचना content *page, संरचना object_params *params);

/**
 * The window containing the content has been closed.
 *
 * Calls the close function for the content.
 */
nserror content_close(संरचना hlcache_handle *h);

/**
 * Tell a content that any selection it has, or one of its objects
 * has, must be cleared.
 */
शून्य content_clear_selection(संरचना hlcache_handle *h);

/**
 * Get a text selection from a content.  Ownership is passed to the caller,
 * who must free() it.
 */
वर्ण * content_get_selection(संरचना hlcache_handle *h);

/**
 * Get positional contextural information for a content.
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 * \param[out] data The context structure to fill in.
 */
nserror content_get_contextual_content(संरचना hlcache_handle *h,
		पूर्णाङ्क x, पूर्णाङ्क y, संरचना browser_window_features *data);

/**
 * scroll content at coordnate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_scroll_at_point(संरचना hlcache_handle *h,
		पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क scrx, पूर्णाङ्क scry);

/**
 * Drag and drop a file at coordinate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_drop_file_at_point(संरचना hlcache_handle *h,
		पूर्णाङ्क x, पूर्णाङ्क y, वर्ण *file);


/**
 * Control debug con a content.
 *
 * \param h content handle to debug.
 * \param op Debug operation type.
 */
nserror content_debug(संरचना hlcache_handle *h, गणना content_debug op);


/**
 * find link in content that matches the rel string.
 *
 * \param h handle to the content to retrieve tyoe of.
 * \param rel The string to match.
 * \return A matching rfc5988 link or NULL if none is found.
 *
 */
संरचना content_rfc5988_link *content_find_rfc5988_link(संरचना hlcache_handle *h, lwc_string *rel);


/**
 * Retrieve status of content
 *
 * \param h handle to the content to retrieve status from
 * \return Content status
 */
content_status content_get_status(संरचना hlcache_handle *h);


/**
 * Retrieve status of content
 *
 * \param c Content to retrieve status from.
 * \return Content status
 */
content_status content__get_status(संरचना content *c);


/**
 * Retrieve status message associated with content
 *
 * \param h handle to the content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ध्रुव वर्ण *content_get_status_message(संरचना hlcache_handle *h);


/**
 * Retrieve available width of content
 *
 * \param h handle to the content to get available width of.
 * \return Available width of content.
 */
पूर्णाङ्क content_get_available_width(संरचना hlcache_handle *h);


/**
 * Retrieve the refresh URL for a content
 *
 * \param h Content to retrieve refresh URL from
 * \return Pointer to URL, or NULL if none
 */
संरचना nsurl *content_get_refresh_url(संरचना hlcache_handle *h);


/**
 * Determine if a content is opaque from handle
 *
 * \param h high level cache handle to retrieve opacity from.
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content_get_opaque(संरचना hlcache_handle *h);


/**
 * Retrieve quirkiness of a content
 *
 * \param h Content to examine
 * \return True if content is quirky, false otherwise
 */
bool content_get_quirks(संरचना hlcache_handle *h);


/**
 * Return whether a content is currently locked
 *
 * \param h handle to the content.
 * \return true iff locked, else false
 */
bool content_is_locked(संरचना hlcache_handle *h);


/**
 * Execute some JavaScript code inside a content object.
 *
 * Runs the passed in JavaScript code in the content object's context.
 *
 * \param h The handle to the content
 * \param src The JavaScript source code
 * \param srclen The length of the source code
 * \return Whether the JS function was successfully injected into the content
 */
bool content_exec(संरचना hlcache_handle *h, ध्रुव वर्ण *src, size_t srclen);

/**
 * Determine if the content referred to any insecure objects.
 *
 * Query the content to determine if any of its referred objects were loaded
 * in a manner not considered secure.  For a content to be recursively
 * secure it must only load over https and must not have certificate overrides
 * in place.
 *
 * \param h The handle to the content
 * \return Whether the content referred to any insecure objects
 */
bool content_saw_insecure_objects(संरचना hlcache_handle *h);

#endif
