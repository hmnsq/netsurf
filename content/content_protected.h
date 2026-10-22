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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <libwapcaplet/libwapcaplet.h>

#அந்தர்பாவ "netsurf/content_type.h"
#அந்தர்பாவ "netsurf/mouse.h" /* mouse state enums */

கட்டமைப்பு nsurl;
கட்டமைப்பு content_redraw_data;
ஒன்றிப்பு content_செய்தி_data;
கட்டமைப்பு http_parameter;
கட்டமைப்பு llcache_handle;
கட்டமைப்பு object_params;
கட்டமைப்பு content;
கட்டமைப்பு redraw_context;
கட்டமைப்பு rect;
கட்டமைப்பு browser_window;
கட்டமைப்பு browser_window_features;
கட்டமைப்பு textsearch_context;
கட்டமைப்பு box;
கட்டமைப்பு selection;
கட்டமைப்பு selection_string;

வகைவரை கட்டமைப்பு content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
கட்டமைப்பு content_handler {
	வெறுமை (*fini)(வெறுமை);

	nserror (*create)(மாறிலி கட்டமைப்பு content_handler *handler,
                          lwc_string *imime_type,
                          மாறிலி கட்டமைப்பு http_parameter *params,
                          கட்டமைப்பு llcache_handle *llcache,
                          மாறிலி எழுத்து *fallback_charset, bool quirks,
                          கட்டமைப்பு content **c);

	bool (*process_data)(கட்டமைப்பு content *c,
			மாறிலி எழுத்து *data, unsigned முழு size);
	bool (*data_complete)(கட்டமைப்பு content *c);
	வெறுமை (*reformat)(கட்டமைப்பு content *c, முழு width, முழு height);
	வெறுமை (*destroy)(கட்டமைப்பு content *c);
	வெறுமை (*stop)(கட்டமைப்பு content *c);
	nserror (*mouse_track)(கட்டமைப்பு content *c, கட்டமைப்பு browser_window *bw,
			browser_mouse_state mouse, முழு x, முழு y);
	nserror (*mouse_action)(கட்டமைப்பு content *c, கட்டமைப்பு browser_window *bw,
			browser_mouse_state mouse, முழு x, முழு y);
	bool (*keypress)(கட்டமைப்பு content *c, uint32_t சாவி);
	bool (*redraw)(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
			மாறிலி கட்டமைப்பு rect *clip,
			மாறிலி கட்டமைப்பு redraw_context *ctx);
	nserror (*open)(கட்டமைப்பு content *c, கட்டமைப்பு browser_window *bw,
			கட்டமைப்பு content *page, கட்டமைப்பு object_params *params);
	nserror (*close)(கட்டமைப்பு content *c);
	வெறுமை (*clear_selection)(கட்டமைப்பு content *c);
	எழுத்து * (*get_selection)(கட்டமைப்பு content *c);
	nserror (*get_contextual_content)(கட்டமைப்பு content *c, முழு x, முழு y,
			கட்டமைப்பு browser_window_features *data);
	bool (*scroll_at_point)(கட்டமைப்பு content *c, முழு x, முழு y,
			முழு scrx, முழு scry);
	bool (*drop_file_at_point)(கட்டமைப்பு content *c, முழு x, முழு y,
			எழுத்து *file);
	nserror (*debug_dump)(கட்டமைப்பு content *c, FILE *f, எண்ணிக்கை content_debug op);
	nserror (*debug)(கட்டமைப்பு content *c, எண்ணிக்கை content_debug op);
	nserror (*clone)(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc);
	bool (*matches_quirks)(மாறிலி கட்டமைப்பு content *c, bool quirks);
	மாறிலி எழுத்து *(*get_encoding)(மாறிலி கட்டமைப்பு content *c, எண்ணிக்கை content_encoding_type op);
	content_type (*type)(வெறுமை);
	வெறுமை (*add_user)(கட்டமைப்பு content *c);
	வெறுமை (*remove_user)(கட்டமைப்பு content *c);
	bool (*exec)(கட்டமைப்பு content *c, மாறிலி எழுத்து *src, size_t srclen);
	bool (*saw_insecure_objects)(கட்டமைப்பு content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(கட்டமைப்பு content *c, கட்டமைப்பு textsearch_context *context, மாறிலி எழுத்து *pattern, முழு p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(கட்டமைப்பு content *c, unsigned start_idx, unsigned end_idx, கட்டமைப்பு box *start_ptr, கட்டமைப்பு box *end_ptr, கட்டமைப்பு rect *bounds_out);

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
	nserror (*textselection_redraw)(கட்டமைப்பு content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(கட்டமைப்பு content *c, unsigned start_idx, unsigned end_idx, கட்டமைப்பு selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(கட்டமைப்பு content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	வெறுமை *(*get_internal)(மாறிலி கட்டமைப்பு content *c, வெறுமை *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(கட்டமைப்பு content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
கட்டமைப்பு content_user
{
	வெறுமை (*callback)(
			கட்டமைப்பு content *c,
			content_செய்தி செய்தி,
			மாறிலி ஒன்றிப்பு content_செய்தி_data *data,
			வெறுமை *pw);
	வெறுமை *pw;

	கட்டமைப்பு content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
கட்டமைப்பு content {
	/**
	 * Low-level cache object
	 */
	கட்டமைப்பு llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	மாறிலி கட்டமைப்பு content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	முழு width;
	/**
	 * Height dimension, if applicable.
	 */
	முழு height;
	/**
	 * Viewport width.
	 */
	முழு available_width;
	/**
	 * Viewport height.
	 */
	முழு available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	எழுத்து *fallback_charset;

	/**
	 * URL for refresh request
	 */
	கட்டமைப்பு nsurl *refresh;

	/**
	 * list of metadata links
	 */
	கட்டமைப்பு content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t time;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_time;

	/**
	 * Estimated size of all data associated with this content
	 */
	unsigned முழு size;
	/**
	 * Title for browser window.
	 */
	எழுத்து *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned முழு active;
	/**
	 * List of users.
	 */
	கட்டமைப்பு content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	எழுத்து status_message[120];
	/**
	 * Status of content.
	 */
	எழுத்து sub_status[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	unsigned long total_size;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	long http_code;

	/**
	 * Free text search state
	 */
	கட்டமைப்பு {
		எழுத்து *string;
		கட்டமைப்பு textsearch_context *context;
	} textsearch;
};

வெளி மாறிலி எழுத்து * மாறிலி content_type_name[];
வெளி மாறிலி எழுத்து * மாறிலி content_status_name[];


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
nserror content__init(கட்டமைப்பு content *c, மாறிலி கட்டமைப்பு content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		கட்டமைப்பு llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(மாறிலி கட்டமைப்பு content *c, கட்டமைப்பு content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
வெறுமை content_set_ready(கட்டமைப்பு content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
வெறுமை content_set_done(கட்டமைப்பு content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
வெறுமை content_set_error(கட்டமைப்பு content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
வெறுமை content_set_status(கட்டமைப்பு content *c, மாறிலி எழுத்து *status_message);

/**
 * Send a message to all users.
 */
வெறுமை content_broadcast(கட்டமைப்பு content *c, content_செய்தி செய்தி, மாறிலி ஒன்றிப்பு content_செய்தி_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
வெறுமை content_broadcast_error(கட்டமைப்பு content *c, nserror errorcode, மாறிலி எழுத்து *செய்தி);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(கட்டமைப்பு content *c, மாறிலி கட்டமைப்பு content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
கட்டமைப்பு content_rfc5988_link *content__free_rfc5988_link(கட்டமைப்பு content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
வெறுமை content__reformat(கட்டமைப்பு content *c, bool background, முழு width, முழு height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
வெறுமை content__request_redraw(கட்டமைப்பு content *c, முழு x, முழு y, முழு width, முழு height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(கட்டமைப்பு content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(கட்டமைப்பு content *c, மாறிலி எழுத்து *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
மாறிலி எழுத்து *content__get_title(கட்டமைப்பு content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
மாறிலி எழுத்து *content__get_status_message(கட்டமைப்பு content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
முழு content__get_width(கட்டமைப்பு content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
முழு content__get_height(கட்டமைப்பு content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
முழு content__get_available_width(கட்டமைப்பு content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
மாறிலி uint8_t *content__get_source_data(கட்டமைப்பு content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
வெறுமை content__invalidate_reuse_data(கட்டமைப்பு content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
கட்டமைப்பு nsurl *content__get_refresh_url(கட்டமைப்பு content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
கட்டமைப்பு bitmap *content__get_bitmap(கட்டமைப்பு content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(கட்டமைப்பு content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
மாறிலி எழுத்து *content__get_encoding(கட்டமைப்பு content *c, எண்ணிக்கை content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(கட்டமைப்பு content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
வெறுமை content_destroy(கட்டமைப்பு content *c);

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
bool content_add_user(கட்டமைப்பு content *h,
		      வெறுமை (*callback)(
				       கட்டமைப்பு content *c,
				       content_செய்தி செய்தி,
				       மாறிலி ஒன்றிப்பு content_செய்தி_data *data,
				       வெறுமை *pw),
		      வெறுமை *pw);

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
வெறுமை content_remove_user(கட்டமைப்பு content *c,
			 வெறுமை (*callback)(
					  கட்டமைப்பு content *c,
					  content_செய்தி செய்தி,
					  மாறிலி ஒன்றிப்பு content_செய்தி_data *data,
					  வெறுமை *pw),
			 வெறுமை *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_மொத்தம்_users(கட்டமைப்பு content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(கட்டமைப்பு content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(கட்டமைப்பு content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
மாறிலி கட்டமைப்பு llcache_handle *content_get_llcache_handle(கட்டமைப்பு content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
கட்டமைப்பு nsurl *content_get_url(கட்டமைப்பு content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
கட்டமைப்பு content *content_clone(கட்டமைப்பு content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(கட்டமைப்பு content *c);

#endif
