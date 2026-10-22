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

#ضمن <stdio.h>
#ضمن <libwapcaplet/libwapcaplet.h>

#ضمن "netsurf/content_type.h"
#ضمن "netsurf/mouse.h" /* mouse state enums */

بنية nsurl;
بنية content_redraw_data;
اتحاد content_رسالة_data;
بنية http_parameter;
بنية llcache_handle;
بنية object_params;
بنية content;
بنية redraw_context;
بنية rect;
بنية browser_window;
بنية browser_window_features;
بنية textsearch_context;
بنية box;
بنية selection;
بنية selection_string;

تعريفنوع بنية content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
بنية content_handler {
	فراغ (*fini)(فراغ);

	nserror (*create)(ثابت بنية content_handler *handler,
                          lwc_string *imime_type,
                          ثابت بنية http_parameter *params,
                          بنية llcache_handle *llcache,
                          ثابت حرف *fallback_charset, bool quirks,
                          بنية content **c);

	bool (*process_data)(بنية content *c,
			ثابت حرف *data, unsigned صحيح size);
	bool (*data_complete)(بنية content *c);
	فراغ (*reformat)(بنية content *c, صحيح width, صحيح height);
	فراغ (*destroy)(بنية content *c);
	فراغ (*stop)(بنية content *c);
	nserror (*mouse_track)(بنية content *c, بنية browser_window *bw,
			browser_mouse_state mouse, صحيح x, صحيح y);
	nserror (*mouse_action)(بنية content *c, بنية browser_window *bw,
			browser_mouse_state mouse, صحيح x, صحيح y);
	bool (*keypress)(بنية content *c, uint32_t مفتاح);
	bool (*redraw)(بنية content *c, بنية content_redraw_data *data,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx);
	nserror (*open)(بنية content *c, بنية browser_window *bw,
			بنية content *page, بنية object_params *params);
	nserror (*close)(بنية content *c);
	فراغ (*clear_selection)(بنية content *c);
	حرف * (*get_selection)(بنية content *c);
	nserror (*get_contextual_content)(بنية content *c, صحيح x, صحيح y,
			بنية browser_window_features *data);
	bool (*scroll_at_point)(بنية content *c, صحيح x, صحيح y,
			صحيح scrx, صحيح scry);
	bool (*drop_file_at_point)(بنية content *c, صحيح x, صحيح y,
			حرف *file);
	nserror (*debug_dump)(بنية content *c, FILE *f, تعداد content_debug op);
	nserror (*debug)(بنية content *c, تعداد content_debug op);
	nserror (*clone)(ثابت بنية content *old, بنية content **newc);
	bool (*matches_quirks)(ثابت بنية content *c, bool quirks);
	ثابت حرف *(*get_encoding)(ثابت بنية content *c, تعداد content_encoding_type op);
	content_type (*type)(فراغ);
	فراغ (*add_user)(بنية content *c);
	فراغ (*remove_user)(بنية content *c);
	bool (*exec)(بنية content *c, ثابت حرف *src, size_t srclen);
	bool (*saw_insecure_objects)(بنية content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(بنية content *c, بنية textsearch_context *context, ثابت حرف *pattern, صحيح p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(بنية content *c, unsigned start_idx, unsigned end_idx, بنية box *start_ptr, بنية box *end_ptr, بنية rect *bounds_out);

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
	nserror (*textselection_redraw)(بنية content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(بنية content *c, unsigned start_idx, unsigned end_idx, بنية selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(بنية content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	فراغ *(*get_internal)(ثابت بنية content *c, فراغ *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(بنية content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
بنية content_user
{
	فراغ (*callback)(
			بنية content *c,
			content_رسالة رسالة,
			ثابت اتحاد content_رسالة_data *data,
			فراغ *pw);
	فراغ *pw;

	بنية content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
بنية content {
	/**
	 * Low-level cache object
	 */
	بنية llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ثابت بنية content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	صحيح width;
	/**
	 * Height dimension, if applicable.
	 */
	صحيح height;
	/**
	 * Viewport width.
	 */
	صحيح available_width;
	/**
	 * Viewport height.
	 */
	صحيح available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	حرف *fallback_charset;

	/**
	 * URL for refresh request
	 */
	بنية nsurl *refresh;

	/**
	 * list of metadata links
	 */
	بنية content_rfc5988_link *links;

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
	unsigned صحيح size;
	/**
	 * Title for browser window.
	 */
	حرف *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned صحيح active;
	/**
	 * List of users.
	 */
	بنية content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	حرف status_message[120];
	/**
	 * Status of content.
	 */
	حرف sub_status[80];
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
	بنية {
		حرف *string;
		بنية textsearch_context *context;
	} textsearch;
};

خارجي ثابت حرف * ثابت content_type_name[];
خارجي ثابت حرف * ثابت content_status_name[];


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
nserror content__init(بنية content *c, ثابت بنية content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		بنية llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ثابت بنية content *c, بنية content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
فراغ content_set_ready(بنية content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
فراغ content_set_done(بنية content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
فراغ content_set_error(بنية content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
فراغ content_set_status(بنية content *c, ثابت حرف *status_message);

/**
 * Send a message to all users.
 */
فراغ content_broadcast(بنية content *c, content_رسالة رسالة, ثابت اتحاد content_رسالة_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
فراغ content_broadcast_error(بنية content *c, nserror errorcode, ثابت حرف *رسالة);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(بنية content *c, ثابت بنية content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
بنية content_rfc5988_link *content__free_rfc5988_link(بنية content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
فراغ content__reformat(بنية content *c, bool background, صحيح width, صحيح height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
فراغ content__request_redraw(بنية content *c, صحيح x, صحيح y, صحيح width, صحيح height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(بنية content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(بنية content *c, ثابت حرف *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ثابت حرف *content__get_title(بنية content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ثابت حرف *content__get_status_message(بنية content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
صحيح content__get_width(بنية content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
صحيح content__get_height(بنية content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
صحيح content__get_available_width(بنية content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ثابت uint8_t *content__get_source_data(بنية content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
فراغ content__invalidate_reuse_data(بنية content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
بنية nsurl *content__get_refresh_url(بنية content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
بنية bitmap *content__get_bitmap(بنية content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(بنية content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ثابت حرف *content__get_encoding(بنية content *c, تعداد content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(بنية content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
فراغ content_destroy(بنية content *c);

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
bool content_add_user(بنية content *h,
		      فراغ (*callback)(
				       بنية content *c,
				       content_رسالة رسالة,
				       ثابت اتحاد content_رسالة_data *data,
				       فراغ *pw),
		      فراغ *pw);

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
فراغ content_remove_user(بنية content *c,
			 فراغ (*callback)(
					  بنية content *c,
					  content_رسالة رسالة,
					  ثابت اتحاد content_رسالة_data *data,
					  فراغ *pw),
			 فراغ *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_عدد_users(بنية content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(بنية content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(بنية content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ثابت بنية llcache_handle *content_get_llcache_handle(بنية content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
بنية nsurl *content_get_url(بنية content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
بنية content *content_clone(بنية content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(بنية content *c);

#endif
