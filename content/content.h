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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_CONTENT_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_CONTENT_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content_type.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h" /* mouse state enums */
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/console.h" /* console state and flags enums */

ਬਣਤਰ browser_ਖਿੜਕੀ;
ਬਣਤਰ browser_ਖਿੜਕੀ_features;
ਬਣਤਰ content;
ਬਣਤਰ llcache_handle;
ਬਣਤਰ hlcache_handle;
ਬਣਤਰ object_params;
ਬਣਤਰ rect;
ਬਣਤਰ redraw_context;
ਬਣਤਰ cert_chain;


/** RFC5988 metadata link */
ਬਣਤਰ content_rfc5988_link {
	ਬਣਤਰ content_rfc5988_link *ਅਗਲਾ; /**< next rfc5988_link in list */

	lwc_string *rel; /**< the link relationship - must be present */
	ਬਣਤਰ nsurl *href; /**< the link href - must be present */
	lwc_string *hreflang;
	lwc_string *type;
	lwc_string *media;
	lwc_string *sizes;
};

/** Extra data for some content_msg messages. */
ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ {
	/**
	 * CONTENT_MSG_LOG - Information for logging
	 */
	ਬਣਤਰ {
		/** The source of the logging */
		browser_ਖਿੜਕੀ_console_ਸਰੋਤ src;
		/** The message to log */
		ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
		/** The length of that message */
		size_t msglen;
		/** The flags of the logging */
		browser_ਖਿੜਕੀ_console_flags flags;
	} ਲਘੂਗਣਕ;

	/**
	 * CONTENT_MSG_SSL_CERTS - The certificate chain from the
	 *   underlying fetch
	 */
	ਅਚਲ ਬਣਤਰ cert_chain *chain;

	/**
	 * CONTENT_MSG_ERROR - Error from content or underlying fetch
	 */
	ਬਣਤਰ {
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
		ਅਚਲ ਅੱਖਰ *errormsg;
	} errordata;

	/**
	 * CONTENT_MSG_REDIRECT - Redirect info
	 */
	ਬਣਤਰ {
		ਬਣਤਰ nsurl *from;	/**< Redirect origin */
		ਬਣਤਰ nsurl *ਮੰਜ਼ਿਲ;	/**< Redirect target */
	} redirect;		/**< Fetch URL redirect occured */

	/**
	 * CONTENT_MSG_REDRAW - Area of content which needs redrawing
	 */
	ਬਣਤਰ {
		ਪੂਰਨ x, y, ਚੌੜਾਈ, ਉਚਾਈ;
	} redraw;

	/**
	 * CONTENT_MSG_REFRESH - Minimum delay
	 */
	ਪੂਰਨ delay;

	/**
	 * CONTENT_MSG_REFORMAT - Reformat should not cause a redraw
	 */
	bool background;

	/**
	 * CONTENT_MSG_STATUS - Status message update.  If NULL, the
	 * content's internal status text has been updated, and
	 * listener should use content_get_status_message()
	 */
	ਅਚਲ ਅੱਖਰ *explicit_ਸਥਿਤੀ_ਪਾਠ;

	/**
	 * CONTENT_MSG_DOWNLOAD - Low-level cache handle
	 */
	ਬਣਤਰ llcache_handle *download;

	/**
	 * CONTENT_MSG_RFC5988_LINK - rfc5988 link data
	 */
	ਬਣਤਰ content_rfc5988_link *rfc5988_link;

	/**
	 * CONTENT_MSG_GETTHREAD - Javascript context (thread)
	 */
	ਬਣਤਰ jsthread **jsthread;

	/**
	 * CONTENT_MSG_GETDIMS - Get the viewport dimensions
	 */
	ਬਣਤਰ {
		/** \todo Consider getting screen_width, screen_height too. */
		ਬਿਨਾਨਿਸ਼ਾਨੀ *viewport_ਚੌੜਾਈ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ *viewport_ਉਚਾਈ;
	} getdims;

	/**
	 * CONTENT_MSG_SCROLL - Part of content to scroll to show
	 */
	ਬਣਤਰ {
		/*
		 * if true, scroll to show area given by (x0, y0) and (x1,y1).
		 * if false, scroll point (x0, y0) to top left of viewport
		 */
		bool area;
		ਪੂਰਨ x0, y0;
		ਪੂਰਨ x1, y1;
	} scroll;

	/**
	 * CONTENT_MSG_DRAGSAVE - Drag save a content
	 */
	ਬਣਤਰ {
		ਗਿਣਤੀ {
			CONTENT_SAVE_ORIG,
			CONTENT_SAVE_NATIVE,
			CONTENT_SAVE_COMPLETE,
			CONTENT_SAVE_ਸਰੋਤ
		} type;
		 /** if NULL, save the content generating the message */
		ਬਣਤਰ hlcache_handle *content;
	} dragsave;

	/**
	 * CONTENT_MSG_SAVELINK - Save a URL
	 */
	ਬਣਤਰ {
		ਬਣਤਰ nsurl *url;
		ਅਚਲ ਅੱਖਰ *title;
	} savelink;

	/**
	 * CONTENT_MSG_POINTER - Mouse pointer to set
	 */
	browser_ਸੂਚਕ_shape ਸੂਚਕ;

	/**
	 * CONTENT_MSG_SELECTION - Selection made or cleared
	 */
	ਬਣਤਰ {
		bool selection; /**< false for selection cleared */
		bool read_only;
	} selection;

	/**
	 * CONTENT_MSG_CARET - set caret position or, hide caret
	 */
	ਬਣਤਰ {
		ਗਿਣਤੀ {
			CONTENT_CARET_ਸੈੱਟ_POS,
			CONTENT_CARET_HIDE,
			CONTENT_CARET_REMOVE
		} type;
		ਬਣਤਰ {
			ਪੂਰਨ x;				/**< Carret x-coord */
			ਪੂਰਨ y;				/**< Carret y-coord */
			ਪੂਰਨ ਉਚਾਈ;			/**< Carret height */
			ਅਚਲ ਬਣਤਰ rect *clip;	/**< Carret clip rect */
		} pos;			/**< With CONTENT_CARET_SET_POS */
	} caret;

	/**
	 * CONTENT_MSG_DRAG - Drag start or end
	 */
	ਬਣਤਰ {
		ਗਿਣਤੀ {
			CONTENT_DRAG_NONE,
			CONTENT_DRAG_SCROLL,
			CONTENT_DRAG_SELECTION
		} type;
		ਅਚਲ ਬਣਤਰ rect *rect;
	} drag;

	/**
	 * CONTENT_MSG_SELECTMENU - Create select menu at pointer
	 */
	ਬਣਤਰ {
		ਬਣਤਰ form_control *gadget;
	} select_menu;

	/**
	 * CONTENT_MSG_GADGETCLICK - User clicked on a form gadget
	 */
	ਬਣਤਰ {
		ਬਣਤਰ form_control *gadget;
	} gadget_click;

	/**
	 * CONTENT_MSG_TEXTSEARCH - Free text search action
	 */
	ਬਣਤਰ {
		/**
		 * The type of text search operation
		 */
		ਗਿਣਤੀ {
		      /**
		       * Free text search find operation has started or finished
		       */
		      CONTENT_TEXTSEARCH_ਲੱਭੋ,
		      /**
		       * Free text search match state has changed
		       */
		      CONTENT_TEXTSEARCH_ਮੇਲ,
		      /**
		       * Free text search back available state changed
		       */
		      CONTENT_TEXTSEARCH_ਪਿੱਠ,
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
		ਖਾਲੀ *ctx;
		/**
		 * state for operation
		 */
		bool state;
		/**
		 * search string
		 */
		ਅਚਲ ਅੱਖਰ *string;
	} textsearch;

};


/**
 * Get whether a content can reformat
 *
 * \param h  content to check
 * \return whether the content can reformat
 */
bool content_can_reformat(ਬਣਤਰ hlcache_handle *h);

/**
 * Reformat to new size.
 *
 * Calls the reformat function for the content.
 */
ਖਾਲੀ content_reformat(ਬਣਤਰ hlcache_handle *h, bool background,
		ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);

/**
 * Request a redraw of an area of a content
 *
 * \param h	  high-level cache handle
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ਖਾਲੀ content_request_redraw(ਬਣਤਰ hlcache_handle *h,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);

/**
 * Handle mouse movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
ਖਾਲੀ content_mouse_track(ਬਣਤਰ hlcache_handle *h, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);

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
ਖਾਲੀ content_mouse_action(ਬਣਤਰ hlcache_handle *h, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);

/**
 * Handle keypresses.
 *
 * \param  h	Content handle
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool content_keypress(ਬਣਤਰ hlcache_handle *h, uint32_t ਕੁੰਜੀ);


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
nserror content_open(ਬਣਤਰ hlcache_handle *h, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
		ਬਣਤਰ content *ਪੰਨਾ, ਬਣਤਰ object_params *params);

/**
 * The window containing the content has been closed.
 *
 * Calls the close function for the content.
 */
nserror content_close(ਬਣਤਰ hlcache_handle *h);

/**
 * Tell a content that any selection it has, or one of its objects
 * has, must be cleared.
 */
ਖਾਲੀ content_clear_selection(ਬਣਤਰ hlcache_handle *h);

/**
 * Get a text selection from a content.  Ownership is passed to the caller,
 * who must free() it.
 */
ਅੱਖਰ * content_get_selection(ਬਣਤਰ hlcache_handle *h);

/**
 * Get positional contextural information for a content.
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 * \param[out] data The context structure to fill in.
 */
nserror content_get_contextual_content(ਬਣਤਰ hlcache_handle *h,
		ਪੂਰਨ x, ਪੂਰਨ y, ਬਣਤਰ browser_ਖਿੜਕੀ_features *ਅੰਕੜੇ);

/**
 * scroll content at coordnate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_scroll_at_ਬਿੰਦੂ(ਬਣਤਰ hlcache_handle *h,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ scrx, ਪੂਰਨ scry);

/**
 * Drag and drop a file at coordinate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_drop_file_at_ਬਿੰਦੂ(ਬਣਤਰ hlcache_handle *h,
		ਪੂਰਨ x, ਪੂਰਨ y, ਅੱਖਰ *file);


/**
 * Control debug con a content.
 *
 * \param h content handle to debug.
 * \param op Debug operation type.
 */
nserror content_debug(ਬਣਤਰ hlcache_handle *h, ਗਿਣਤੀ content_debug op);


/**
 * find link in content that matches the rel string.
 *
 * \param h handle to the content to retrieve tyoe of.
 * \param rel The string to match.
 * \return A matching rfc5988 link or NULL if none is found.
 *
 */
ਬਣਤਰ content_rfc5988_link *content_ਲੱਭੋ_rfc5988_link(ਬਣਤਰ hlcache_handle *h, lwc_string *rel);


/**
 * Retrieve status of content
 *
 * \param h handle to the content to retrieve status from
 * \return Content status
 */
content_ਸਥਿਤੀ content_get_ਸਥਿਤੀ(ਬਣਤਰ hlcache_handle *h);


/**
 * Retrieve status of content
 *
 * \param c Content to retrieve status from.
 * \return Content status
 */
content_ਸਥਿਤੀ content_get_ਸਥਿਤੀ_2(ਬਣਤਰ content *c);


/**
 * Retrieve status message associated with content
 *
 * \param h handle to the content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ਅਚਲ ਅੱਖਰ *content_get_ਸਥਿਤੀ_ਸੁਨੇਹਾ(ਬਣਤਰ hlcache_handle *h);


/**
 * Retrieve available width of content
 *
 * \param h handle to the content to get available width of.
 * \return Available width of content.
 */
ਪੂਰਨ content_get_ਉਪਲਬਧ_ਚੌੜਾਈ(ਬਣਤਰ hlcache_handle *h);


/**
 * Retrieve the refresh URL for a content
 *
 * \param h Content to retrieve refresh URL from
 * \return Pointer to URL, or NULL if none
 */
ਬਣਤਰ nsurl *content_get_refresh_url(ਬਣਤਰ hlcache_handle *h);


/**
 * Determine if a content is opaque from handle
 *
 * \param h high level cache handle to retrieve opacity from.
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content_get_opaque(ਬਣਤਰ hlcache_handle *h);


/**
 * Retrieve quirkiness of a content
 *
 * \param h Content to examine
 * \return True if content is quirky, false otherwise
 */
bool content_get_quirks(ਬਣਤਰ hlcache_handle *h);


/**
 * Return whether a content is currently locked
 *
 * \param h handle to the content.
 * \return true iff locked, else false
 */
bool content_is_locked(ਬਣਤਰ hlcache_handle *h);


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
bool content_exec(ਬਣਤਰ hlcache_handle *h, ਅਚਲ ਅੱਖਰ *src, size_t srclen);

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
bool content_saw_insecure_objects(ਬਣਤਰ hlcache_handle *h);

#ਜੇ_ਅੰਤ
