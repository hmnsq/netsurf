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

#شامل <stdio.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/content_type.h"
#شامل "netsurf/mouse.h" /* mouse state enums */

ساختار nsurl;
ساختار content_redraw_data;
اجتماع content_پیام_data;
ساختار http_parameter;
ساختار llcache_handle;
ساختار object_params;
ساختار content;
ساختار redraw_context;
ساختار rect;
ساختار browser_window;
ساختار browser_window_features;
ساختار textsearch_context;
ساختار box;
ساختار selection;
ساختار selection_string;

تعریفنوع ساختار content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ساختار content_handler {
	تهی (*fini)(تهی);

	nserror (*create)(ثابت ساختار content_handler *handler,
                          lwc_string *imime_type,
                          ثابت ساختار http_parameter *params,
                          ساختار llcache_handle *llcache,
                          ثابت نویسه *fallback_charset, bool quirks,
                          ساختار content **c);

	bool (*process_data)(ساختار content *c,
			ثابت نویسه *data, unsigned صحیح size);
	bool (*data_complete)(ساختار content *c);
	تهی (*reformat)(ساختار content *c, صحیح width, صحیح height);
	تهی (*destroy)(ساختار content *c);
	تهی (*stop)(ساختار content *c);
	nserror (*mouse_track)(ساختار content *c, ساختار browser_window *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);
	nserror (*mouse_action)(ساختار content *c, ساختار browser_window *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);
	bool (*keypress)(ساختار content *c, uint32_t کلید);
	bool (*redraw)(ساختار content *c, ساختار content_redraw_data *data,
			ثابت ساختار rect *clip,
			ثابت ساختار redraw_context *ctx);
	nserror (*open)(ساختار content *c, ساختار browser_window *bw,
			ساختار content *page, ساختار object_params *params);
	nserror (*close)(ساختار content *c);
	تهی (*clear_selection)(ساختار content *c);
	نویسه * (*get_selection)(ساختار content *c);
	nserror (*get_contextual_content)(ساختار content *c, صحیح x, صحیح y,
			ساختار browser_window_features *data);
	bool (*scroll_at_point)(ساختار content *c, صحیح x, صحیح y,
			صحیح scrx, صحیح scry);
	bool (*drop_file_at_point)(ساختار content *c, صحیح x, صحیح y,
			نویسه *file);
	nserror (*debug_dump)(ساختار content *c, FILE *f, شمارش content_debug op);
	nserror (*debug)(ساختار content *c, شمارش content_debug op);
	nserror (*clone)(ثابت ساختار content *old, ساختار content **newc);
	bool (*matches_quirks)(ثابت ساختار content *c, bool quirks);
	ثابت نویسه *(*get_encoding)(ثابت ساختار content *c, شمارش content_encoding_type op);
	content_type (*type)(تهی);
	تهی (*add_user)(ساختار content *c);
	تهی (*remove_user)(ساختار content *c);
	bool (*exec)(ساختار content *c, ثابت نویسه *src, size_t srclen);
	bool (*saw_insecure_objects)(ساختار content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(ساختار content *c, ساختار textsearch_context *context, ثابت نویسه *pattern, صحیح p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ساختار content *c, unsigned start_idx, unsigned end_idx, ساختار box *start_ptr, ساختار box *end_ptr, ساختار rect *bounds_out);

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
	nserror (*textselection_redraw)(ساختار content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ساختار content *c, unsigned start_idx, unsigned end_idx, ساختار selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(ساختار content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	تهی *(*get_internal)(ثابت ساختار content *c, تهی *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ساختار content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ساختار content_user
{
	تهی (*callback)(
			ساختار content *c,
			content_پیام پیام,
			ثابت اجتماع content_پیام_data *data,
			تهی *pw);
	تهی *pw;

	ساختار content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
ساختار content {
	/**
	 * Low-level cache object
	 */
	ساختار llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ثابت ساختار content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	صحیح width;
	/**
	 * Height dimension, if applicable.
	 */
	صحیح height;
	/**
	 * Viewport width.
	 */
	صحیح available_width;
	/**
	 * Viewport height.
	 */
	صحیح available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	نویسه *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ساختار nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ساختار content_rfc5988_link *links;

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
	unsigned صحیح size;
	/**
	 * Title for browser window.
	 */
	نویسه *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned صحیح active;
	/**
	 * List of users.
	 */
	ساختار content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	نویسه status_message[120];
	/**
	 * Status of content.
	 */
	نویسه sub_status[80];
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
	ساختار {
		نویسه *string;
		ساختار textsearch_context *context;
	} textsearch;
};

بیرونی ثابت نویسه * ثابت content_type_name[];
بیرونی ثابت نویسه * ثابت content_status_name[];


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
nserror content__init(ساختار content *c, ثابت ساختار content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		ساختار llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ثابت ساختار content *c, ساختار content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
تهی content_set_ready(ساختار content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
تهی content_set_done(ساختار content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
تهی content_set_error(ساختار content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
تهی content_set_status(ساختار content *c, ثابت نویسه *status_message);

/**
 * Send a message to all users.
 */
تهی content_broadcast(ساختار content *c, content_پیام پیام, ثابت اجتماع content_پیام_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
تهی content_broadcast_error(ساختار content *c, nserror errorcode, ثابت نویسه *پیام);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(ساختار content *c, ثابت ساختار content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ساختار content_rfc5988_link *content__free_rfc5988_link(ساختار content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
تهی content__reformat(ساختار content *c, bool background, صحیح width, صحیح height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
تهی content__request_redraw(ساختار content *c, صحیح x, صحیح y, صحیح width, صحیح height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ساختار content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(ساختار content *c, ثابت نویسه *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ثابت نویسه *content__get_title(ساختار content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ثابت نویسه *content__get_status_message(ساختار content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
صحیح content__get_width(ساختار content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
صحیح content__get_height(ساختار content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
صحیح content__get_available_width(ساختار content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ثابت uint8_t *content__get_source_data(ساختار content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
تهی content__invalidate_reuse_data(ساختار content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ساختار nsurl *content__get_refresh_url(ساختار content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ساختار bitmap *content__get_bitmap(ساختار content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ساختار content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ثابت نویسه *content__get_encoding(ساختار content *c, شمارش content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ساختار content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
تهی content_destroy(ساختار content *c);

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
bool content_add_user(ساختار content *h,
		      تهی (*callback)(
				       ساختار content *c,
				       content_پیام پیام,
				       ثابت اجتماع content_پیام_data *data,
				       تهی *pw),
		      تهی *pw);

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
تهی content_remove_user(ساختار content *c,
			 تهی (*callback)(
					  ساختار content *c,
					  content_پیام پیام,
					  ثابت اجتماع content_پیام_data *data,
					  تهی *pw),
			 تهی *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_شمار_users(ساختار content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ساختار content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ساختار content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ثابت ساختار llcache_handle *content_get_llcache_handle(ساختار content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ساختار nsurl *content_get_url(ساختار content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ساختار content *content_clone(ساختار content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ساختار content *c);

#endif
