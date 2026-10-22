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
 * Protected interface to Content handling.
 *
 * The content functions manipulate struct contents, which correspond to URLs.
 */

#ifndef NETSURF_CONTENT_CONTENT_PROTECTED_H_
#define NETSURF_CONTENT_CONTENT_PROTECTED_H_

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>

#अन्तर्भाव "netsurf/content_type.h"
#अन्तर्भाव "netsurf/mouse.h" /* mouse state enums */

संरचना nsurl;
संरचना content_redraw_data;
संघ content_संदेश_data;
संरचना http_parameter;
संरचना llcache_handle;
संरचना object_params;
संरचना content;
संरचना redraw_context;
संरचना rect;
संरचना browser_window;
संरचना browser_window_features;
संरचना textsearch_context;
संरचना box;
संरचना selection;
संरचना selection_string;

प्रकारव्याख्या संरचना content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
संरचना content_handler {
	शून्य (*fini)(शून्य);

	nserror (*create)(स्थिरांक संरचना content_handler *handler,
                          lwc_string *imime_type,
                          स्थिरांक संरचना http_parameter *params,
                          संरचना llcache_handle *llcache,
                          स्थिरांक अक्षर *fallback_charset, bool quirks,
                          संरचना content **c);

	bool (*process_data)(संरचना content *c,
			स्थिरांक अक्षर *data, अचिन्हित पूर्णांक size);
	bool (*data_complete)(संरचना content *c);
	शून्य (*reformat)(संरचना content *c, पूर्णांक width, पूर्णांक height);
	शून्य (*destroy)(संरचना content *c);
	शून्य (*stop)(संरचना content *c);
	nserror (*mouse_track)(संरचना content *c, संरचना browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);
	nserror (*mouse_action)(संरचना content *c, संरचना browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);
	bool (*keypress)(संरचना content *c, uint32_t किल्ली);
	bool (*redraw)(संरचना content *c, संरचना content_redraw_data *data,
			स्थिरांक संरचना rect *clip,
			स्थिरांक संरचना redraw_context *ctx);
	nserror (*खोलो)(संरचना content *c, संरचना browser_window *bw,
			संरचना content *page, संरचना object_params *params);
	nserror (*बंद)(संरचना content *c);
	शून्य (*clear_selection)(संरचना content *c);
	अक्षर * (*get_selection)(संरचना content *c);
	nserror (*get_contextual_content)(संरचना content *c, पूर्णांक x, पूर्णांक y,
			संरचना browser_window_features *data);
	bool (*scroll_at_point)(संरचना content *c, पूर्णांक x, पूर्णांक y,
			पूर्णांक scrx, पूर्णांक scry);
	bool (*drop_file_at_point)(संरचना content *c, पूर्णांक x, पूर्णांक y,
			अक्षर *file);
	nserror (*debug_dump)(संरचना content *c, खाता *f, गणना content_debug op);
	nserror (*debug)(संरचना content *c, गणना content_debug op);
	nserror (*clone)(स्थिरांक संरचना content *old, संरचना content **newc);
	bool (*matches_quirks)(स्थिरांक संरचना content *c, bool quirks);
	स्थिरांक अक्षर *(*get_encoding)(स्थिरांक संरचना content *c, गणना content_encoding_type op);
	content_type (*type)(शून्य);
	शून्य (*add_user)(संरचना content *c);
	शून्य (*remove_user)(संरचना content *c);
	bool (*exec)(संरचना content *c, स्थिरांक अक्षर *src, माप_प्रकार srclen);
	bool (*saw_insecure_objects)(संरचना content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(संरचना content *c, संरचना textsearch_context *context, स्थिरांक अक्षर *pattern, पूर्णांक p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(संरचना content *c, अचिन्हित start_idx, अचिन्हित end_idx, संरचना box *start_ptr, संरचना box *end_ptr, संरचना rect *bounds_out);

	/**
	 * redraw an area of selected text
	 *
	 * The defined text selection will cause an area of the
	 *   content to be marked as invalid and hence redrawn.
	 *
	 * \param c The content being redrawn
	 * \param start_idx The start index of the text region to be redrawn
	 * \param end_idx The end index of teh text region to be redrawn
	 * \return NSERROR_OK on success else error code
	 */
	nserror (*textselection_redraw)(संरचना content *c, अचिन्हित start_idx, अचिन्हित end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(संरचना content *c, अचिन्हित start_idx, अचिन्हित end_idx, संरचना selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(संरचना content *c, अचिन्हित *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	शून्य *(*get_internal)(स्थिरांक संरचना content *c, शून्य *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(संरचना content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
संरचना content_user
{
	शून्य (*callback)(
			संरचना content *c,
			content_संदेश संदेश,
			स्थिरांक संघ content_संदेश_data *data,
			शून्य *pw);
	शून्य *pw;

	संरचना content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
संरचना content {
	/**
	 * Low-level cache object
	 */
	संरचना llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	स्थिरांक संरचना content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	पूर्णांक width;
	/**
	 * Height dimension, if applicable.
	 */
	पूर्णांक height;
	/**
	 * Viewport width.
	 */
	पूर्णांक available_width;
	/**
	 * Viewport height.
	 */
	पूर्णांक available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	अक्षर *fallback_charset;

	/**
	 * URL for refresh request
	 */
	संरचना nsurl *refresh;

	/**
	 * list of metadata links
	 */
	संरचना content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t समय;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_time;

	/**
	 * Estimated size of all data associated with this content
	 */
	अचिन्हित पूर्णांक size;
	/**
	 * Title for browser window.
	 */
	अक्षर *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	अचिन्हित पूर्णांक active;
	/**
	 * List of users.
	 */
	संरचना content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	अक्षर status_message[120];
	/**
	 * Status of content.
	 */
	अक्षर sub_status[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	अचिन्हित दीर्घ total_size;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	दीर्घ http_code;

	/**
	 * Free text search state
	 */
	संरचना {
		अक्षर *string;
		संरचना textsearch_context *context;
	} textsearch;
};

बाह्य स्थिरांक अक्षर * स्थिरांक content_type_name[];
बाह्य स्थिरांक अक्षर * स्थिरांक content_status_name[];


/**
 * Initialise a new base content structure.
 *
 * \param c                 Content to initialise
 * \param handler           Content handler
 * \param imime_type        MIME type of content
 * \param params            HTTP parameters
 * \param llcache           Source data handle
 * \param fallback_charset  Fallback charset
 * \param quirks            Quirkiness of content
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__init(संरचना content *c, स्थिरांक संरचना content_handler *handler,
		lwc_string *imime_type, स्थिरांक संरचना http_parameter *params,
		संरचना llcache_handle *llcache, स्थिरांक अक्षर *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(स्थिरांक संरचना content *c, संरचना content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
शून्य content_set_ready(संरचना content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
शून्य content_set_done(संरचना content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
शून्य content_set_error(संरचना content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
शून्य content_set_status(संरचना content *c, स्थिरांक अक्षर *status_message);

/**
 * Send a message to all users.
 */
शून्य content_broadcast(संरचना content *c, content_संदेश संदेश, स्थिरांक संघ content_संदेश_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
शून्य content_broadcast_error(संरचना content *c, nserror errorcode, स्थिरांक अक्षर *संदेश);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(संरचना content *c, स्थिरांक संरचना content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
संरचना content_rfc5988_link *content__free_rfc5988_link(संरचना content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
शून्य content__reformat(संरचना content *c, bool background, पूर्णांक width, पूर्णांक height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
शून्य content__request_redraw(संरचना content *c, पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(संरचना content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(संरचना content *c, स्थिरांक अक्षर *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
स्थिरांक अक्षर *content__get_title(संरचना content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
स्थिरांक अक्षर *content__get_status_message(संरचना content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
पूर्णांक content__get_width(संरचना content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
पूर्णांक content__get_height(संरचना content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
पूर्णांक content__get_available_width(संरचना content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
स्थिरांक uint8_t *content__get_source_data(संरचना content *c, माप_प्रकार *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
शून्य content__invalidate_reuse_data(संरचना content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
संरचना nsurl *content__get_refresh_url(संरचना content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
संरचना bitmap *content__get_bitmap(संरचना content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(संरचना content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
स्थिरांक अक्षर *content__get_encoding(संरचना content *c, गणना content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(संरचना content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
शून्य content_destroy(संरचना content *c);

/**
 * Register a user for callbacks.
 *
 * \param c the content to register
 * \param callback the user callback function
 * \param pw callback private data
 * \return true on success, false otherwise on memory exhaustion
 *
 * The callback will be called when content_broadcast() is
 * called with the content.
 */
bool content_add_user(संरचना content *h,
		      शून्य (*callback)(
				       संरचना content *c,
				       content_संदेश संदेश,
				       स्थिरांक संघ content_संदेश_data *data,
				       शून्य *pw),
		      शून्य *pw);

/**
 * Remove a callback user.
 *
 * The callback function and pw must be identical to those passed to
 * content_add_user().
 *
 * \param c Content to remove user from
 * \param callback passed when added
 * \param ctx Context passed when added
 */
शून्य content_remove_user(संरचना content *c,
			 शून्य (*callback)(
					  संरचना content *c,
					  content_संदेश संदेश,
					  स्थिरांक संघ content_संदेश_data *data,
					  शून्य *pw),
			 शून्य *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_मोजणी_users(संरचना content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(संरचना content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(संरचना content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
स्थिरांक संरचना llcache_handle *content_get_llcache_handle(संरचना content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
संरचना nsurl *content_get_url(संरचना content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
संरचना content *content_clone(संरचना content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(संरचना content *c);

#endif
