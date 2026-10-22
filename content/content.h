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

#که_نه_تعریف_شوی NETSURF_CONTENT_CONTENT_H_
#تعریف_کړه NETSURF_CONTENT_CONTENT_H_

#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/content_type.h"
#ضمن "netsurf/mouse.h" /* mouse state enums */
#ضمن "netsurf/console.h" /* console state and flags enums */

بنية browser_کړکۍ;
بنية browser_کړکۍ_features;
بنية content;
بنية llcache_handle;
بنية hlcache_handle;
بنية object_params;
بنية rect;
بنية redraw_context;
بنية cert_chain;


/** RFC5988 metadata link */
بنية content_rfc5988_link {
	بنية content_rfc5988_link *بل; /**< next rfc5988_link in list */

	lwc_string *rel; /**< the link relationship - must be present */
	بنية nsurl *href; /**< the link href - must be present */
	lwc_string *hreflang;
	lwc_string *type;
	lwc_string *media;
	lwc_string *sizes;
};

/** Extra data for some content_msg messages. */
اتحاد content_پیغام_معلومات {
	/**
	 * CONTENT_MSG_LOG - Information for logging
	 */
	بنية {
		/** The source of the logging */
		browser_کړکۍ_console_سرچینه src;
		/** The message to log */
		ثابت حرف *پیغام;
		/** The length of that message */
		size_t msglen;
		/** The flags of the logging */
		browser_کړکۍ_console_flags flags;
	} لوګاریتم;

	/**
	 * CONTENT_MSG_SSL_CERTS - The certificate chain from the
	 *   underlying fetch
	 */
	ثابت بنية cert_chain *chain;

	/**
	 * CONTENT_MSG_ERROR - Error from content or underlying fetch
	 */
	بنية {
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
		ثابت حرف *errormsg;
	} errordata;

	/**
	 * CONTENT_MSG_REDIRECT - Redirect info
	 */
	بنية {
		بنية nsurl *from;	/**< Redirect origin */
		بنية nsurl *منزل;	/**< Redirect target */
	} redirect;		/**< Fetch URL redirect occured */

	/**
	 * CONTENT_MSG_REDRAW - Area of content which needs redrawing
	 */
	بنية {
		صحيح x, y, پلنوالی, لوړوالی;
	} redraw;

	/**
	 * CONTENT_MSG_REFRESH - Minimum delay
	 */
	صحيح delay;

	/**
	 * CONTENT_MSG_REFORMAT - Reformat should not cause a redraw
	 */
	bool background;

	/**
	 * CONTENT_MSG_STATUS - Status message update.  If NULL, the
	 * content's internal status text has been updated, and
	 * listener should use content_get_status_message()
	 */
	ثابت حرف *explicit_حالت_متن;

	/**
	 * CONTENT_MSG_DOWNLOAD - Low-level cache handle
	 */
	بنية llcache_handle *download;

	/**
	 * CONTENT_MSG_RFC5988_LINK - rfc5988 link data
	 */
	بنية content_rfc5988_link *rfc5988_link;

	/**
	 * CONTENT_MSG_GETTHREAD - Javascript context (thread)
	 */
	بنية jsthread **jsthread;

	/**
	 * CONTENT_MSG_GETDIMS - Get the viewport dimensions
	 */
	بنية {
		/** \todo Consider getting screen_width, screen_height too. */
		بینښه *viewport_پلنوالی;
		بینښه *viewport_لوړوالی;
	} getdims;

	/**
	 * CONTENT_MSG_SCROLL - Part of content to scroll to show
	 */
	بنية {
		/*
		 * if true, scroll to show area given by (x0, y0) and (x1,y1).
		 * if false, scroll point (x0, y0) to top left of viewport
		 */
		bool area;
		صحيح x0, y0;
		صحيح x1, y1;
	} scroll;

	/**
	 * CONTENT_MSG_DRAGSAVE - Drag save a content
	 */
	بنية {
		تعداد {
			CONTENT_SAVE_ORIG,
			CONTENT_SAVE_NATIVE,
			CONTENT_SAVE_COMPLETE,
			CONTENT_SAVE_سرچینه
		} type;
		 /** if NULL, save the content generating the message */
		بنية hlcache_handle *content;
	} dragsave;

	/**
	 * CONTENT_MSG_SAVELINK - Save a URL
	 */
	بنية {
		بنية nsurl *url;
		ثابت حرف *title;
	} savelink;

	/**
	 * CONTENT_MSG_POINTER - Mouse pointer to set
	 */
	browser_اشاره_ګر_shape اشاره_ګر;

	/**
	 * CONTENT_MSG_SELECTION - Selection made or cleared
	 */
	بنية {
		bool selection; /**< false for selection cleared */
		bool read_only;
	} selection;

	/**
	 * CONTENT_MSG_CARET - set caret position or, hide caret
	 */
	بنية {
		تعداد {
			CONTENT_CARET_سېټ_POS,
			CONTENT_CARET_HIDE,
			CONTENT_CARET_REMOVE
		} type;
		بنية {
			صحيح x;				/**< Carret x-coord */
			صحيح y;				/**< Carret y-coord */
			صحيح لوړوالی;			/**< Carret height */
			ثابت بنية rect *clip;	/**< Carret clip rect */
		} pos;			/**< With CONTENT_CARET_SET_POS */
	} caret;

	/**
	 * CONTENT_MSG_DRAG - Drag start or end
	 */
	بنية {
		تعداد {
			CONTENT_DRAG_NONE,
			CONTENT_DRAG_SCROLL,
			CONTENT_DRAG_SELECTION
		} type;
		ثابت بنية rect *rect;
	} drag;

	/**
	 * CONTENT_MSG_SELECTMENU - Create select menu at pointer
	 */
	بنية {
		بنية form_control *gadget;
	} select_menu;

	/**
	 * CONTENT_MSG_GADGETCLICK - User clicked on a form gadget
	 */
	بنية {
		بنية form_control *gadget;
	} gadget_click;

	/**
	 * CONTENT_MSG_TEXTSEARCH - Free text search action
	 */
	بنية {
		/**
		 * The type of text search operation
		 */
		تعداد {
		      /**
		       * Free text search find operation has started or finished
		       */
		      CONTENT_TEXTSEARCH_پیدا_کړه,
		      /**
		       * Free text search match state has changed
		       */
		      CONTENT_TEXTSEARCH_سمون,
		      /**
		       * Free text search back available state changed
		       */
		      CONTENT_TEXTSEARCH_شا,
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
		فراغ *ctx;
		/**
		 * state for operation
		 */
		bool state;
		/**
		 * search string
		 */
		ثابت حرف *string;
	} textsearch;

};


/**
 * Get whether a content can reformat
 *
 * \param h  content to check
 * \return whether the content can reformat
 */
bool content_can_reformat(بنية hlcache_handle *h);

/**
 * Reformat to new size.
 *
 * Calls the reformat function for the content.
 */
فراغ content_reformat(بنية hlcache_handle *h, bool background,
		صحيح پلنوالی, صحيح لوړوالی);

/**
 * Request a redraw of an area of a content
 *
 * \param h	  high-level cache handle
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
فراغ content_request_redraw(بنية hlcache_handle *h,
		صحيح x, صحيح y, صحيح پلنوالی, صحيح لوړوالی);

/**
 * Handle mouse movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
فراغ content_mouse_track(بنية hlcache_handle *h, بنية browser_کړکۍ *bw,
		browser_mouse_state mouse, صحيح x, صحيح y);

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
فراغ content_mouse_action(بنية hlcache_handle *h, بنية browser_کړکۍ *bw,
		browser_mouse_state mouse, صحيح x, صحيح y);

/**
 * Handle keypresses.
 *
 * \param  h	Content handle
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool content_keypress(بنية hlcache_handle *h, uint32_t کلی);


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
nserror content_open(بنية hlcache_handle *h, بنية browser_کړکۍ *bw,
		بنية content *پاڼه, بنية object_params *params);

/**
 * The window containing the content has been closed.
 *
 * Calls the close function for the content.
 */
nserror content_close(بنية hlcache_handle *h);

/**
 * Tell a content that any selection it has, or one of its objects
 * has, must be cleared.
 */
فراغ content_clear_selection(بنية hlcache_handle *h);

/**
 * Get a text selection from a content.  Ownership is passed to the caller,
 * who must free() it.
 */
حرف * content_get_selection(بنية hlcache_handle *h);

/**
 * Get positional contextural information for a content.
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 * \param[out] data The context structure to fill in.
 */
nserror content_get_contextual_content(بنية hlcache_handle *h,
		صحيح x, صحيح y, بنية browser_کړکۍ_features *معلومات);

/**
 * scroll content at coordnate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_scroll_at_ټکی(بنية hlcache_handle *h,
		صحيح x, صحيح y, صحيح scrx, صحيح scry);

/**
 * Drag and drop a file at coordinate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_drop_file_at_ټکی(بنية hlcache_handle *h,
		صحيح x, صحيح y, حرف *file);


/**
 * Control debug con a content.
 *
 * \param h content handle to debug.
 * \param op Debug operation type.
 */
nserror content_debug(بنية hlcache_handle *h, تعداد content_debug op);


/**
 * find link in content that matches the rel string.
 *
 * \param h handle to the content to retrieve tyoe of.
 * \param rel The string to match.
 * \return A matching rfc5988 link or NULL if none is found.
 *
 */
بنية content_rfc5988_link *content_پیدا_کړه_rfc5988_link(بنية hlcache_handle *h, lwc_string *rel);


/**
 * Retrieve status of content
 *
 * \param h handle to the content to retrieve status from
 * \return Content status
 */
content_حالت content_get_حالت(بنية hlcache_handle *h);


/**
 * Retrieve status of content
 *
 * \param c Content to retrieve status from.
 * \return Content status
 */
content_حالت content_get_حالت_2(بنية content *c);


/**
 * Retrieve status message associated with content
 *
 * \param h handle to the content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ثابت حرف *content_get_حالت_پیغام(بنية hlcache_handle *h);


/**
 * Retrieve available width of content
 *
 * \param h handle to the content to get available width of.
 * \return Available width of content.
 */
صحيح content_get_شته_پلنوالی(بنية hlcache_handle *h);


/**
 * Retrieve the refresh URL for a content
 *
 * \param h Content to retrieve refresh URL from
 * \return Pointer to URL, or NULL if none
 */
بنية nsurl *content_get_refresh_url(بنية hlcache_handle *h);


/**
 * Determine if a content is opaque from handle
 *
 * \param h high level cache handle to retrieve opacity from.
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content_get_opaque(بنية hlcache_handle *h);


/**
 * Retrieve quirkiness of a content
 *
 * \param h Content to examine
 * \return True if content is quirky, false otherwise
 */
bool content_get_quirks(بنية hlcache_handle *h);


/**
 * Return whether a content is currently locked
 *
 * \param h handle to the content.
 * \return true iff locked, else false
 */
bool content_is_locked(بنية hlcache_handle *h);


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
bool content_exec(بنية hlcache_handle *h, ثابت حرف *src, size_t srclen);

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
bool content_saw_insecure_objects(بنية hlcache_handle *h);

#د_که_پای
