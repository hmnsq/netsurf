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

#अगर_अघोषित NETSURF_CONTENT_CONTENT_H_
#घोषणा NETSURF_CONTENT_CONTENT_H_

#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/content_type.h"
#समावेश "netsurf/mouse.h" /* mouse state enums */
#समावेश "netsurf/console.h" /* console state and flags enums */

काष्ठा browser_window;
काष्ठा browser_window_features;
काष्ठा content;
काष्ठा llcache_handle;
काष्ठा hlcache_handle;
काष्ठा object_params;
काष्ठा rect;
काष्ठा redraw_context;
काष्ठा cert_chain;


/** RFC5988 metadata link */
काष्ठा content_rfc5988_link {
	काष्ठा content_rfc5988_link *अग्र; /**< next rfc5988_link in list */

	lwc_string *rel; /**< the link relationship - must be present */
	काष्ठा nsurl *href; /**< the link href - must be present */
	lwc_string *hreflang;
	lwc_string *type;
	lwc_string *media;
	lwc_string *sizes;
};

/** Extra data for some content_msg messages. */
जोड़ content_संदेश_तथ्य {
	/**
	 * CONTENT_MSG_LOG - Information for logging
	 */
	काष्ठा {
		/** The source of the logging */
		browser_window_console_source src;
		/** The message to log */
		स्थिर अक्षर *संदेश;
		/** The length of that message */
		माप_प्रकार msglen;
		/** The flags of the logging */
		browser_window_console_flags flags;
	} log;

	/**
	 * CONTENT_MSG_SSL_CERTS - The certificate chain from the
	 *   underlying fetch
	 */
	स्थिर काष्ठा cert_chain *chain;

	/**
	 * CONTENT_MSG_ERROR - Error from content or underlying fetch
	 */
	काष्ठा {
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
		स्थिर अक्षर *errormsg;
	} errordata;

	/**
	 * CONTENT_MSG_REDIRECT - Redirect info
	 */
	काष्ठा {
		काष्ठा nsurl *from;	/**< Redirect origin */
		काष्ठा nsurl *to;	/**< Redirect target */
	} redirect;		/**< Fetch URL redirect occured */

	/**
	 * CONTENT_MSG_REDRAW - Area of content which needs redrawing
	 */
	काष्ठा {
		पूर्णांक x, y, width, height;
	} redraw;

	/**
	 * CONTENT_MSG_REFRESH - Minimum delay
	 */
	पूर्णांक delay;

	/**
	 * CONTENT_MSG_REFORMAT - Reformat should not cause a redraw
	 */
	bool background;

	/**
	 * CONTENT_MSG_STATUS - Status message update.  If NULL, the
	 * content's internal status text has been updated, and
	 * listener should use content_get_status_message()
	 */
	स्थिर अक्षर *explicit_status_text;

	/**
	 * CONTENT_MSG_DOWNLOAD - Low-level cache handle
	 */
	काष्ठा llcache_handle *download;

	/**
	 * CONTENT_MSG_RFC5988_LINK - rfc5988 link data
	 */
	काष्ठा content_rfc5988_link *rfc5988_link;

	/**
	 * CONTENT_MSG_GETTHREAD - Javascript context (thread)
	 */
	काष्ठा jsthread **jsthread;

	/**
	 * CONTENT_MSG_GETDIMS - Get the viewport dimensions
	 */
	काष्ठा {
		/** \todo Consider getting screen_width, screen_height too. */
		अचिन्हित *viewport_width;
		अचिन्हित *viewport_height;
	} getdims;

	/**
	 * CONTENT_MSG_SCROLL - Part of content to scroll to show
	 */
	काष्ठा {
		/*
		 * if true, scroll to show area given by (x0, y0) and (x1,y1).
		 * if false, scroll point (x0, y0) to top left of viewport
		 */
		bool area;
		पूर्णांक x0, y0;
		पूर्णांक x1, y1;
	} scroll;

	/**
	 * CONTENT_MSG_DRAGSAVE - Drag save a content
	 */
	काष्ठा {
		क्रमागत {
			CONTENT_SAVE_ORIG,
			CONTENT_SAVE_NATIVE,
			CONTENT_SAVE_COMPLETE,
			CONTENT_SAVE_SOURCE
		} type;
		 /** if NULL, save the content generating the message */
		काष्ठा hlcache_handle *content;
	} dragsave;

	/**
	 * CONTENT_MSG_SAVELINK - Save a URL
	 */
	काष्ठा {
		काष्ठा nsurl *url;
		स्थिर अक्षर *title;
	} savelink;

	/**
	 * CONTENT_MSG_POINTER - Mouse pointer to set
	 */
	browser_pointer_shape pointer;

	/**
	 * CONTENT_MSG_SELECTION - Selection made or cleared
	 */
	काष्ठा {
		bool selection; /**< false for selection cleared */
		bool read_only;
	} selection;

	/**
	 * CONTENT_MSG_CARET - set caret position or, hide caret
	 */
	काष्ठा {
		क्रमागत {
			CONTENT_CARET_SET_POS,
			CONTENT_CARET_HIDE,
			CONTENT_CARET_REMOVE
		} type;
		काष्ठा {
			पूर्णांक x;				/**< Carret x-coord */
			पूर्णांक y;				/**< Carret y-coord */
			पूर्णांक height;			/**< Carret height */
			स्थिर काष्ठा rect *clip;	/**< Carret clip rect */
		} pos;			/**< With CONTENT_CARET_SET_POS */
	} caret;

	/**
	 * CONTENT_MSG_DRAG - Drag start or end
	 */
	काष्ठा {
		क्रमागत {
			CONTENT_DRAG_NONE,
			CONTENT_DRAG_SCROLL,
			CONTENT_DRAG_SELECTION
		} type;
		स्थिर काष्ठा rect *rect;
	} drag;

	/**
	 * CONTENT_MSG_SELECTMENU - Create select menu at pointer
	 */
	काष्ठा {
		काष्ठा form_control *gadget;
	} select_menu;

	/**
	 * CONTENT_MSG_GADGETCLICK - User clicked on a form gadget
	 */
	काष्ठा {
		काष्ठा form_control *gadget;
	} gadget_click;

	/**
	 * CONTENT_MSG_TEXTSEARCH - Free text search action
	 */
	काष्ठा {
		/**
		 * The type of text search operation
		 */
		क्रमागत {
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
		व्योम *ctx;
		/**
		 * state for operation
		 */
		bool state;
		/**
		 * search string
		 */
		स्थिर अक्षर *string;
	} textsearch;

};


/**
 * Get whether a content can reformat
 *
 * \param h  content to check
 * \return whether the content can reformat
 */
bool content_can_reformat(काष्ठा hlcache_handle *h);

/**
 * Reformat to new size.
 *
 * Calls the reformat function for the content.
 */
व्योम content_reformat(काष्ठा hlcache_handle *h, bool background,
		पूर्णांक width, पूर्णांक height);

/**
 * Request a redraw of an area of a content
 *
 * \param h	  high-level cache handle
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
व्योम content_request_redraw(काष्ठा hlcache_handle *h,
		पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height);

/**
 * Handle mouse movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
व्योम content_mouse_track(काष्ठा hlcache_handle *h, काष्ठा browser_window *bw,
		browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);

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
व्योम content_mouse_action(काष्ठा hlcache_handle *h, काष्ठा browser_window *bw,
		browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);

/**
 * Handle keypresses.
 *
 * \param  h	Content handle
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool content_keypress(काष्ठा hlcache_handle *h, uint32_t कुंजी);


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
nserror content_open(काष्ठा hlcache_handle *h, काष्ठा browser_window *bw,
		काष्ठा content *page, काष्ठा object_params *params);

/**
 * The window containing the content has been closed.
 *
 * Calls the close function for the content.
 */
nserror content_close(काष्ठा hlcache_handle *h);

/**
 * Tell a content that any selection it has, or one of its objects
 * has, must be cleared.
 */
व्योम content_clear_selection(काष्ठा hlcache_handle *h);

/**
 * Get a text selection from a content.  Ownership is passed to the caller,
 * who must free() it.
 */
अक्षर * content_get_selection(काष्ठा hlcache_handle *h);

/**
 * Get positional contextural information for a content.
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 * \param[out] data The context structure to fill in.
 */
nserror content_get_contextual_content(काष्ठा hlcache_handle *h,
		पूर्णांक x, पूर्णांक y, काष्ठा browser_window_features *तथ्य);

/**
 * scroll content at coordnate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_scroll_at_point(काष्ठा hlcache_handle *h,
		पूर्णांक x, पूर्णांक y, पूर्णांक scrx, पूर्णांक scry);

/**
 * Drag and drop a file at coordinate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_drop_file_at_point(काष्ठा hlcache_handle *h,
		पूर्णांक x, पूर्णांक y, अक्षर *file);


/**
 * Control debug con a content.
 *
 * \param h content handle to debug.
 * \param op Debug operation type.
 */
nserror content_debug(काष्ठा hlcache_handle *h, क्रमागत content_debug op);


/**
 * find link in content that matches the rel string.
 *
 * \param h handle to the content to retrieve tyoe of.
 * \param rel The string to match.
 * \return A matching rfc5988 link or NULL if none is found.
 *
 */
काष्ठा content_rfc5988_link *content_find_rfc5988_link(काष्ठा hlcache_handle *h, lwc_string *rel);


/**
 * Retrieve status of content
 *
 * \param h handle to the content to retrieve status from
 * \return Content status
 */
content_status content_get_status(काष्ठा hlcache_handle *h);


/**
 * Retrieve status of content
 *
 * \param c Content to retrieve status from.
 * \return Content status
 */
content_status content__get_status(काष्ठा content *c);


/**
 * Retrieve status message associated with content
 *
 * \param h handle to the content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
स्थिर अक्षर *content_get_status_message(काष्ठा hlcache_handle *h);


/**
 * Retrieve available width of content
 *
 * \param h handle to the content to get available width of.
 * \return Available width of content.
 */
पूर्णांक content_get_available_width(काष्ठा hlcache_handle *h);


/**
 * Retrieve the refresh URL for a content
 *
 * \param h Content to retrieve refresh URL from
 * \return Pointer to URL, or NULL if none
 */
काष्ठा nsurl *content_get_refresh_url(काष्ठा hlcache_handle *h);


/**
 * Determine if a content is opaque from handle
 *
 * \param h high level cache handle to retrieve opacity from.
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content_get_opaque(काष्ठा hlcache_handle *h);


/**
 * Retrieve quirkiness of a content
 *
 * \param h Content to examine
 * \return True if content is quirky, false otherwise
 */
bool content_get_quirks(काष्ठा hlcache_handle *h);


/**
 * Return whether a content is currently locked
 *
 * \param h handle to the content.
 * \return true iff locked, else false
 */
bool content_is_locked(काष्ठा hlcache_handle *h);


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
bool content_exec(काष्ठा hlcache_handle *h, स्थिर अक्षर *src, माप_प्रकार srclen);

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
bool content_saw_insecure_objects(काष्ठा hlcache_handle *h);

#पूर्ण_अगर
