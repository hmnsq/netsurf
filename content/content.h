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

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_CONTENT_H_
#నిర్వచించు NETSURF_CONTENT_CONTENT_H_

#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/content_type.h"
#అన్తర్భావ "netsurf/mouse.h" /* mouse state enums */
#అన్తర్భావ "netsurf/console.h" /* console state and flags enums */

నిర్మాణం browser_కిటికీ;
నిర్మాణం browser_కిటికీ_features;
నిర్మాణం content;
నిర్మాణం llcache_handle;
నిర్మాణం hlcache_handle;
నిర్మాణం object_params;
నిర్మాణం rect;
నిర్మాణం redraw_context;
నిర్మాణం cert_chain;


/** RFC5988 metadata link */
నిర్మాణం content_rfc5988_link {
	నిర్మాణం content_rfc5988_link *తదుపరి; /**< next rfc5988_link in list */

	lwc_string *rel; /**< the link relationship - must be present */
	నిర్మాణం nsurl *href; /**< the link href - must be present */
	lwc_string *hreflang;
	lwc_string *type;
	lwc_string *media;
	lwc_string *sizes;
};

/** Extra data for some content_msg messages. */
సంఘం content_సందేశం_దత్తాంశం {
	/**
	 * CONTENT_MSG_LOG - Information for logging
	 */
	నిర్మాణం {
		/** The source of the logging */
		browser_కిటికీ_console_ఆధారం src;
		/** The message to log */
		స్థిరరాశి అక్షరం *సందేశం;
		/** The length of that message */
		size_t msglen;
		/** The flags of the logging */
		browser_కిటికీ_console_flags flags;
	} సంవర్గమానం;

	/**
	 * CONTENT_MSG_SSL_CERTS - The certificate chain from the
	 *   underlying fetch
	 */
	స్థిరరాశి నిర్మాణం cert_chain *chain;

	/**
	 * CONTENT_MSG_ERROR - Error from content or underlying fetch
	 */
	నిర్మాణం {
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
		స్థిరరాశి అక్షరం *errormsg;
	} errordata;

	/**
	 * CONTENT_MSG_REDIRECT - Redirect info
	 */
	నిర్మాణం {
		నిర్మాణం nsurl *from;	/**< Redirect origin */
		నిర్మాణం nsurl *గమ్యం;	/**< Redirect target */
	} redirect;		/**< Fetch URL redirect occured */

	/**
	 * CONTENT_MSG_REDRAW - Area of content which needs redrawing
	 */
	నిర్మాణం {
		పూర్ణాంకం x, y, వెడల్పు, ఎత్తు;
	} redraw;

	/**
	 * CONTENT_MSG_REFRESH - Minimum delay
	 */
	పూర్ణాంకం delay;

	/**
	 * CONTENT_MSG_REFORMAT - Reformat should not cause a redraw
	 */
	bool background;

	/**
	 * CONTENT_MSG_STATUS - Status message update.  If NULL, the
	 * content's internal status text has been updated, and
	 * listener should use content_get_status_message()
	 */
	స్థిరరాశి అక్షరం *explicit_స్థితి_పాఠ్యం;

	/**
	 * CONTENT_MSG_DOWNLOAD - Low-level cache handle
	 */
	నిర్మాణం llcache_handle *download;

	/**
	 * CONTENT_MSG_RFC5988_LINK - rfc5988 link data
	 */
	నిర్మాణం content_rfc5988_link *rfc5988_link;

	/**
	 * CONTENT_MSG_GETTHREAD - Javascript context (thread)
	 */
	నిర్మాణం jsthread **jsthread;

	/**
	 * CONTENT_MSG_GETDIMS - Get the viewport dimensions
	 */
	నిర్మాణం {
		/** \todo Consider getting screen_width, screen_height too. */
		గుర్తులేని *viewport_వెడల్పు;
		గుర్తులేని *viewport_ఎత్తు;
	} getdims;

	/**
	 * CONTENT_MSG_SCROLL - Part of content to scroll to show
	 */
	నిర్మాణం {
		/*
		 * if true, scroll to show area given by (x0, y0) and (x1,y1).
		 * if false, scroll point (x0, y0) to top left of viewport
		 */
		bool area;
		పూర్ణాంకం x0, y0;
		పూర్ణాంకం x1, y1;
	} scroll;

	/**
	 * CONTENT_MSG_DRAGSAVE - Drag save a content
	 */
	నిర్మాణం {
		గణన {
			CONTENT_SAVE_ORIG,
			CONTENT_SAVE_NATIVE,
			CONTENT_SAVE_COMPLETE,
			CONTENT_SAVE_ఆధారం
		} type;
		 /** if NULL, save the content generating the message */
		నిర్మాణం hlcache_handle *content;
	} dragsave;

	/**
	 * CONTENT_MSG_SAVELINK - Save a URL
	 */
	నిర్మాణం {
		నిర్మాణం nsurl *url;
		స్థిరరాశి అక్షరం *title;
	} savelink;

	/**
	 * CONTENT_MSG_POINTER - Mouse pointer to set
	 */
	browser_సూచిక_shape సూచిక;

	/**
	 * CONTENT_MSG_SELECTION - Selection made or cleared
	 */
	నిర్మాణం {
		bool selection; /**< false for selection cleared */
		bool read_only;
	} selection;

	/**
	 * CONTENT_MSG_CARET - set caret position or, hide caret
	 */
	నిర్మాణం {
		గణన {
			CONTENT_CARET_సమితి_POS,
			CONTENT_CARET_HIDE,
			CONTENT_CARET_REMOVE
		} type;
		నిర్మాణం {
			పూర్ణాంకం x;				/**< Carret x-coord */
			పూర్ణాంకం y;				/**< Carret y-coord */
			పూర్ణాంకం ఎత్తు;			/**< Carret height */
			స్థిరరాశి నిర్మాణం rect *clip;	/**< Carret clip rect */
		} pos;			/**< With CONTENT_CARET_SET_POS */
	} caret;

	/**
	 * CONTENT_MSG_DRAG - Drag start or end
	 */
	నిర్మాణం {
		గణన {
			CONTENT_DRAG_NONE,
			CONTENT_DRAG_SCROLL,
			CONTENT_DRAG_SELECTION
		} type;
		స్థిరరాశి నిర్మాణం rect *rect;
	} drag;

	/**
	 * CONTENT_MSG_SELECTMENU - Create select menu at pointer
	 */
	నిర్మాణం {
		నిర్మాణం form_control *gadget;
	} select_menu;

	/**
	 * CONTENT_MSG_GADGETCLICK - User clicked on a form gadget
	 */
	నిర్మాణం {
		నిర్మాణం form_control *gadget;
	} gadget_click;

	/**
	 * CONTENT_MSG_TEXTSEARCH - Free text search action
	 */
	నిర్మాణం {
		/**
		 * The type of text search operation
		 */
		గణన {
		      /**
		       * Free text search find operation has started or finished
		       */
		      CONTENT_TEXTSEARCH_కనుగొను,
		      /**
		       * Free text search match state has changed
		       */
		      CONTENT_TEXTSEARCH_సరిపోలిక,
		      /**
		       * Free text search back available state changed
		       */
		      CONTENT_TEXTSEARCH_వెనుక,
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
		శూన్య *ctx;
		/**
		 * state for operation
		 */
		bool state;
		/**
		 * search string
		 */
		స్థిరరాశి అక్షరం *string;
	} textsearch;

};


/**
 * Get whether a content can reformat
 *
 * \param h  content to check
 * \return whether the content can reformat
 */
bool content_can_reformat(నిర్మాణం hlcache_handle *h);

/**
 * Reformat to new size.
 *
 * Calls the reformat function for the content.
 */
శూన్య content_reformat(నిర్మాణం hlcache_handle *h, bool background,
		పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);

/**
 * Request a redraw of an area of a content
 *
 * \param h	  high-level cache handle
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
శూన్య content_request_redraw(నిర్మాణం hlcache_handle *h,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);

/**
 * Handle mouse movements in a content window.
 *
 * \param  h	  Content handle
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */
శూన్య content_mouse_track(నిర్మాణం hlcache_handle *h, నిర్మాణం browser_కిటికీ *bw,
		browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);

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
శూన్య content_mouse_action(నిర్మాణం hlcache_handle *h, నిర్మాణం browser_కిటికీ *bw,
		browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);

/**
 * Handle keypresses.
 *
 * \param  h	Content handle
 * \param  key	The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool content_keypress(నిర్మాణం hlcache_handle *h, uint32_t కీలకం);


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
nserror content_open(నిర్మాణం hlcache_handle *h, నిర్మాణం browser_కిటికీ *bw,
		నిర్మాణం content *పుట, నిర్మాణం object_params *params);

/**
 * The window containing the content has been closed.
 *
 * Calls the close function for the content.
 */
nserror content_close(నిర్మాణం hlcache_handle *h);

/**
 * Tell a content that any selection it has, or one of its objects
 * has, must be cleared.
 */
శూన్య content_clear_selection(నిర్మాణం hlcache_handle *h);

/**
 * Get a text selection from a content.  Ownership is passed to the caller,
 * who must free() it.
 */
అక్షరం * content_get_selection(నిర్మాణం hlcache_handle *h);

/**
 * Get positional contextural information for a content.
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 * \param[out] data The context structure to fill in.
 */
nserror content_get_contextual_content(నిర్మాణం hlcache_handle *h,
		పూర్ణాంకం x, పూర్ణాంకం y, నిర్మాణం browser_కిటికీ_features *దత్తాంశం);

/**
 * scroll content at coordnate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_scroll_at_బిందువు(నిర్మాణం hlcache_handle *h,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం scrx, పూర్ణాంకం scry);

/**
 * Drag and drop a file at coordinate
 *
 * \param[in] h Handle to content to examine.
 * \param[in] x The x coordinate to examine.
 * \param[in] y The y coordinate to examine.
 */
bool content_drop_file_at_బిందువు(నిర్మాణం hlcache_handle *h,
		పూర్ణాంకం x, పూర్ణాంకం y, అక్షరం *file);


/**
 * Control debug con a content.
 *
 * \param h content handle to debug.
 * \param op Debug operation type.
 */
nserror content_debug(నిర్మాణం hlcache_handle *h, గణన content_debug op);


/**
 * find link in content that matches the rel string.
 *
 * \param h handle to the content to retrieve tyoe of.
 * \param rel The string to match.
 * \return A matching rfc5988 link or NULL if none is found.
 *
 */
నిర్మాణం content_rfc5988_link *content_కనుగొను_rfc5988_link(నిర్మాణం hlcache_handle *h, lwc_string *rel);


/**
 * Retrieve status of content
 *
 * \param h handle to the content to retrieve status from
 * \return Content status
 */
content_స్థితి content_get_స్థితి(నిర్మాణం hlcache_handle *h);


/**
 * Retrieve status of content
 *
 * \param c Content to retrieve status from.
 * \return Content status
 */
content_స్థితి content_get_స్థితి_2(నిర్మాణం content *c);


/**
 * Retrieve status message associated with content
 *
 * \param h handle to the content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
స్థిరరాశి అక్షరం *content_get_స్థితి_సందేశం(నిర్మాణం hlcache_handle *h);


/**
 * Retrieve available width of content
 *
 * \param h handle to the content to get available width of.
 * \return Available width of content.
 */
పూర్ణాంకం content_get_అందుబాటు_వెడల్పు(నిర్మాణం hlcache_handle *h);


/**
 * Retrieve the refresh URL for a content
 *
 * \param h Content to retrieve refresh URL from
 * \return Pointer to URL, or NULL if none
 */
నిర్మాణం nsurl *content_get_refresh_url(నిర్మాణం hlcache_handle *h);


/**
 * Determine if a content is opaque from handle
 *
 * \param h high level cache handle to retrieve opacity from.
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content_get_opaque(నిర్మాణం hlcache_handle *h);


/**
 * Retrieve quirkiness of a content
 *
 * \param h Content to examine
 * \return True if content is quirky, false otherwise
 */
bool content_get_quirks(నిర్మాణం hlcache_handle *h);


/**
 * Return whether a content is currently locked
 *
 * \param h handle to the content.
 * \return true iff locked, else false
 */
bool content_is_locked(నిర్మాణం hlcache_handle *h);


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
bool content_exec(నిర్మాణం hlcache_handle *h, స్థిరరాశి అక్షరం *src, size_t srclen);

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
bool content_saw_insecure_objects(నిర్మాణం hlcache_handle *h);

#అయితే_ముగింపు
