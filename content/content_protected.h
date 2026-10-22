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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/content_type.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h" /* mouse state enums */

ಸಂರಚನಾ nsurl;
ಸಂರಚನಾ content_redraw_data;
ಸಙ್ಘ content_ಸಂದೇಶ_data;
ಸಂರಚನಾ http_parameter;
ಸಂರಚನಾ llcache_handle;
ಸಂರಚನಾ object_params;
ಸಂರಚನಾ content;
ಸಂರಚನಾ redraw_context;
ಸಂರಚನಾ rect;
ಸಂರಚನಾ browser_window;
ಸಂರಚನಾ browser_window_features;
ಸಂರಚನಾ textsearch_context;
ಸಂರಚನಾ box;
ಸಂರಚನಾ selection;
ಸಂರಚನಾ selection_string;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ಸಂರಚನಾ content_handler {
	ಶೂನ್ಯ (*fini)(ಶೂನ್ಯ);

	nserror (*create)(ಧ್ರುವ ಸಂರಚನಾ content_handler *handler,
                          lwc_string *imime_type,
                          ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
                          ಸಂರಚನಾ llcache_handle *llcache,
                          ಧ್ರುವ ವರ್ಣ *fallback_charset, bool quirks,
                          ಸಂರಚನಾ content **c);

	bool (*process_data)(ಸಂರಚನಾ content *c,
			ಧ್ರುವ ವರ್ಣ *data, unsigned ಪೂರ್ಣಾಙ್ಕ size);
	bool (*data_complete)(ಸಂರಚನಾ content *c);
	ಶೂನ್ಯ (*reformat)(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height);
	ಶೂನ್ಯ (*destroy)(ಸಂರಚನಾ content *c);
	ಶೂನ್ಯ (*stop)(ಸಂರಚನಾ content *c);
	nserror (*mouse_track)(ಸಂರಚನಾ content *c, ಸಂರಚನಾ browser_window *bw,
			browser_mouse_state mouse, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y);
	nserror (*mouse_action)(ಸಂರಚನಾ content *c, ಸಂರಚನಾ browser_window *bw,
			browser_mouse_state mouse, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y);
	bool (*keypress)(ಸಂರಚನಾ content *c, uint32_t ಕೀಲಿ);
	bool (*redraw)(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
			ಧ್ರುವ ಸಂರಚನಾ rect *clip,
			ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx);
	nserror (*open)(ಸಂರಚನಾ content *c, ಸಂರಚನಾ browser_window *bw,
			ಸಂರಚನಾ content *page, ಸಂರಚನಾ object_params *params);
	nserror (*close)(ಸಂರಚನಾ content *c);
	ಶೂನ್ಯ (*clear_selection)(ಸಂರಚನಾ content *c);
	ವರ್ಣ * (*get_selection)(ಸಂರಚನಾ content *c);
	nserror (*get_contextual_content)(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			ಸಂರಚನಾ browser_window_features *data);
	bool (*scroll_at_point)(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			ಪೂರ್ಣಾಙ್ಕ scrx, ಪೂರ್ಣಾಙ್ಕ scry);
	bool (*drop_file_at_point)(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			ವರ್ಣ *file);
	nserror (*debug_dump)(ಸಂರಚನಾ content *c, FILE *f, ಗಣನಾ content_debug op);
	nserror (*debug)(ಸಂರಚನಾ content *c, ಗಣನಾ content_debug op);
	nserror (*clone)(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc);
	bool (*matches_quirks)(ಧ್ರುವ ಸಂರಚನಾ content *c, bool quirks);
	ಧ್ರುವ ವರ್ಣ *(*get_encoding)(ಧ್ರುವ ಸಂರಚನಾ content *c, ಗಣನಾ content_encoding_type op);
	content_type (*type)(ಶೂನ್ಯ);
	ಶೂನ್ಯ (*add_user)(ಸಂರಚನಾ content *c);
	ಶೂನ್ಯ (*remove_user)(ಸಂರಚನಾ content *c);
	bool (*exec)(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *src, size_t srclen);
	bool (*saw_insecure_objects)(ಸಂರಚನಾ content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(ಸಂರಚನಾ content *c, ಸಂರಚನಾ textsearch_context *context, ಧ್ರುವ ವರ್ಣ *pattern, ಪೂರ್ಣಾಙ್ಕ p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ಸಂರಚನಾ content *c, unsigned start_idx, unsigned end_idx, ಸಂರಚನಾ box *start_ptr, ಸಂರಚನಾ box *end_ptr, ಸಂರಚನಾ rect *bounds_out);

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
	nserror (*textselection_redraw)(ಸಂರಚನಾ content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ಸಂರಚನಾ content *c, unsigned start_idx, unsigned end_idx, ಸಂರಚನಾ selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(ಸಂರಚನಾ content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	ಶೂನ್ಯ *(*get_internal)(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ಸಂರಚನಾ content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ಸಂರಚನಾ content_user
{
	ಶೂನ್ಯ (*callback)(
			ಸಂರಚನಾ content *c,
			content_ಸಂದೇಶ ಸಂದೇಶ,
			ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data,
			ಶೂನ್ಯ *pw);
	ಶೂನ್ಯ *pw;

	ಸಂರಚನಾ content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
ಸಂರಚನಾ content {
	/**
	 * Low-level cache object
	 */
	ಸಂರಚನಾ llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ಧ್ರುವ ಸಂರಚನಾ content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	ಪೂರ್ಣಾಙ್ಕ width;
	/**
	 * Height dimension, if applicable.
	 */
	ಪೂರ್ಣಾಙ್ಕ height;
	/**
	 * Viewport width.
	 */
	ಪೂರ್ಣಾಙ್ಕ available_width;
	/**
	 * Viewport height.
	 */
	ಪೂರ್ಣಾಙ್ಕ available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	ವರ್ಣ *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ಸಂರಚನಾ nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ಸಂರಚನಾ content_rfc5988_link *links;

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
	unsigned ಪೂರ್ಣಾಙ್ಕ size;
	/**
	 * Title for browser window.
	 */
	ವರ್ಣ *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned ಪೂರ್ಣಾಙ್ಕ active;
	/**
	 * List of users.
	 */
	ಸಂರಚನಾ content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	ವರ್ಣ status_message[120];
	/**
	 * Status of content.
	 */
	ವರ್ಣ sub_status[80];
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
	ಸಂರಚನಾ {
		ವರ್ಣ *string;
		ಸಂರಚನಾ textsearch_context *context;
	} textsearch;
};

ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ content_type_name[];
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ content_status_name[];


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
nserror content__init(ಸಂರಚನಾ content *c, ಧ್ರುವ ಸಂರಚನಾ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		ಸಂರಚನಾ llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ಧ್ರುವ ಸಂರಚನಾ content *c, ಸಂರಚನಾ content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
ಶೂನ್ಯ content_set_ready(ಸಂರಚನಾ content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
ಶೂನ್ಯ content_set_done(ಸಂರಚನಾ content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
ಶೂನ್ಯ content_set_error(ಸಂರಚನಾ content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
ಶೂನ್ಯ content_set_status(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *status_message);

/**
 * Send a message to all users.
 */
ಶೂನ್ಯ content_broadcast(ಸಂರಚನಾ content *c, content_ಸಂದೇಶ ಸಂದೇಶ, ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
ಶೂನ್ಯ content_broadcast_error(ಸಂರಚನಾ content *c, nserror errorcode, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(ಸಂರಚನಾ content *c, ಧ್ರುವ ಸಂರಚನಾ content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ಸಂರಚನಾ content_rfc5988_link *content__free_rfc5988_link(ಸಂರಚನಾ content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
ಶೂನ್ಯ content__reformat(ಸಂರಚನಾ content *c, bool background, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ಶೂನ್ಯ content__request_redraw(ಸಂರಚನಾ content *c, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ಸಂರಚನಾ content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ಧ್ರುವ ವರ್ಣ *content__get_title(ಸಂರಚನಾ content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ಧ್ರುವ ವರ್ಣ *content__get_status_message(ಸಂರಚನಾ content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
ಪೂರ್ಣಾಙ್ಕ content__get_width(ಸಂರಚನಾ content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
ಪೂರ್ಣಾಙ್ಕ content__get_height(ಸಂರಚನಾ content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
ಪೂರ್ಣಾಙ್ಕ content__get_available_width(ಸಂರಚನಾ content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ಧ್ರುವ uint8_t *content__get_source_data(ಸಂರಚನಾ content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
ಶೂನ್ಯ content__invalidate_reuse_data(ಸಂರಚನಾ content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ಸಂರಚನಾ nsurl *content__get_refresh_url(ಸಂರಚನಾ content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ಸಂರಚನಾ bitmap *content__get_bitmap(ಸಂರಚನಾ content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ಸಂರಚನಾ content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ಧ್ರುವ ವರ್ಣ *content__get_encoding(ಸಂರಚನಾ content *c, ಗಣನಾ content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ಸಂರಚನಾ content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
ಶೂನ್ಯ content_destroy(ಸಂರಚನಾ content *c);

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
bool content_add_user(ಸಂರಚನಾ content *h,
		      ಶೂನ್ಯ (*callback)(
				       ಸಂರಚನಾ content *c,
				       content_ಸಂದೇಶ ಸಂದೇಶ,
				       ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data,
				       ಶೂನ್ಯ *pw),
		      ಶೂನ್ಯ *pw);

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
ಶೂನ್ಯ content_remove_user(ಸಂರಚನಾ content *c,
			 ಶೂನ್ಯ (*callback)(
					  ಸಂರಚನಾ content *c,
					  content_ಸಂದೇಶ ಸಂದೇಶ,
					  ಧ್ರುವ ಸಙ್ಘ content_ಸಂದೇಶ_data *data,
					  ಶೂನ್ಯ *pw),
			 ಶೂನ್ಯ *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_ಎಣಿಕೆ_users(ಸಂರಚನಾ content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ಸಂರಚನಾ content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ಸಂರಚನಾ content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ಧ್ರುವ ಸಂರಚನಾ llcache_handle *content_get_llcache_handle(ಸಂರಚನಾ content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ಸಂರಚನಾ nsurl *content_get_url(ಸಂರಚನಾ content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ಸಂರಚನಾ content *content_clone(ಸಂರಚನಾ content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ಸಂರಚನಾ content *c);

#endif
