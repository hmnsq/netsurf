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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>

#અન્તર્ભાવ "netsurf/content_type.h"
#અન્તર્ભાવ "netsurf/mouse.h" /* mouse state enums */

સંરચના nsurl;
સંરચના content_redraw_data;
સંઘ content_સંદેશ_data;
સંરચના http_parameter;
સંરચના llcache_handle;
સંરચના object_params;
સંરચના content;
સંરચના redraw_context;
સંરચના rect;
સંરચના browser_window;
સંરચના browser_window_features;
સંરચના textsearch_context;
સંરચના box;
સંરચના selection;
સંરચના selection_string;

પ્રકારવ્યાખ્યા સંરચના content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
સંરચના content_handler {
	શૂન્ય (*fini)(શૂન્ય);

	nserror (*create)(અચલ સંરચના content_handler *handler,
                          lwc_string *imime_type,
                          અચલ સંરચના http_parameter *params,
                          સંરચના llcache_handle *llcache,
                          અચલ અક્ષર *fallback_charset, bool quirks,
                          સંરચના content **c);

	bool (*process_data)(સંરચના content *c,
			અચલ અક્ષર *data, unsigned પૂર્ણાંક size);
	bool (*data_complete)(સંરચના content *c);
	શૂન્ય (*reformat)(સંરચના content *c, પૂર્ણાંક width, પૂર્ણાંક height);
	શૂન્ય (*destroy)(સંરચના content *c);
	શૂન્ય (*stop)(સંરચના content *c);
	nserror (*mouse_track)(સંરચના content *c, સંરચના browser_window *bw,
			browser_mouse_state mouse, પૂર્ણાંક x, પૂર્ણાંક y);
	nserror (*mouse_action)(સંરચના content *c, સંરચના browser_window *bw,
			browser_mouse_state mouse, પૂર્ણાંક x, પૂર્ણાંક y);
	bool (*keypress)(સંરચના content *c, uint32_t ચાવી);
	bool (*redraw)(સંરચના content *c, સંરચના content_redraw_data *data,
			અચલ સંરચના rect *clip,
			અચલ સંરચના redraw_context *ctx);
	nserror (*open)(સંરચના content *c, સંરચના browser_window *bw,
			સંરચના content *page, સંરચના object_params *params);
	nserror (*close)(સંરચના content *c);
	શૂન્ય (*clear_selection)(સંરચના content *c);
	અક્ષર * (*get_selection)(સંરચના content *c);
	nserror (*get_contextual_content)(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y,
			સંરચના browser_window_features *data);
	bool (*scroll_at_point)(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y,
			પૂર્ણાંક scrx, પૂર્ણાંક scry);
	bool (*drop_file_at_point)(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y,
			અક્ષર *file);
	nserror (*debug_dump)(સંરચના content *c, FILE *f, ગણના content_debug op);
	nserror (*debug)(સંરચના content *c, ગણના content_debug op);
	nserror (*clone)(અચલ સંરચના content *old, સંરચના content **newc);
	bool (*matches_quirks)(અચલ સંરચના content *c, bool quirks);
	અચલ અક્ષર *(*get_encoding)(અચલ સંરચના content *c, ગણના content_encoding_type op);
	content_type (*type)(શૂન્ય);
	શૂન્ય (*add_user)(સંરચના content *c);
	શૂન્ય (*remove_user)(સંરચના content *c);
	bool (*exec)(સંરચના content *c, અચલ અક્ષર *src, size_t srclen);
	bool (*saw_insecure_objects)(સંરચના content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(સંરચના content *c, સંરચના textsearch_context *context, અચલ અક્ષર *pattern, પૂર્ણાંક p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(સંરચના content *c, unsigned start_idx, unsigned end_idx, સંરચના box *start_ptr, સંરચના box *end_ptr, સંરચના rect *bounds_out);

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
	nserror (*textselection_redraw)(સંરચના content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(સંરચના content *c, unsigned start_idx, unsigned end_idx, સંરચના selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(સંરચના content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	શૂન્ય *(*get_internal)(અચલ સંરચના content *c, શૂન્ય *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(સંરચના content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
સંરચના content_user
{
	શૂન્ય (*callback)(
			સંરચના content *c,
			content_સંદેશ સંદેશ,
			અચલ સંઘ content_સંદેશ_data *data,
			શૂન્ય *pw);
	શૂન્ય *pw;

	સંરચના content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
સંરચના content {
	/**
	 * Low-level cache object
	 */
	સંરચના llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	અચલ સંરચના content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	પૂર્ણાંક width;
	/**
	 * Height dimension, if applicable.
	 */
	પૂર્ણાંક height;
	/**
	 * Viewport width.
	 */
	પૂર્ણાંક available_width;
	/**
	 * Viewport height.
	 */
	પૂર્ણાંક available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	અક્ષર *fallback_charset;

	/**
	 * URL for refresh request
	 */
	સંરચના nsurl *refresh;

	/**
	 * list of metadata links
	 */
	સંરચના content_rfc5988_link *links;

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
	unsigned પૂર્ણાંક size;
	/**
	 * Title for browser window.
	 */
	અક્ષર *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned પૂર્ણાંક active;
	/**
	 * List of users.
	 */
	સંરચના content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	અક્ષર status_message[120];
	/**
	 * Status of content.
	 */
	અક્ષર sub_status[80];
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
	સંરચના {
		અક્ષર *string;
		સંરચના textsearch_context *context;
	} textsearch;
};

બાહ્ય અચલ અક્ષર * અચલ content_type_name[];
બાહ્ય અચલ અક્ષર * અચલ content_status_name[];


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
nserror content__init(સંરચના content *c, અચલ સંરચના content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		સંરચના llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(અચલ સંરચના content *c, સંરચના content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
શૂન્ય content_set_ready(સંરચના content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
શૂન્ય content_set_done(સંરચના content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
શૂન્ય content_set_error(સંરચના content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
શૂન્ય content_set_status(સંરચના content *c, અચલ અક્ષર *status_message);

/**
 * Send a message to all users.
 */
શૂન્ય content_broadcast(સંરચના content *c, content_સંદેશ સંદેશ, અચલ સંઘ content_સંદેશ_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
શૂન્ય content_broadcast_error(સંરચના content *c, nserror errorcode, અચલ અક્ષર *સંદેશ);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(સંરચના content *c, અચલ સંરચના content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
સંરચના content_rfc5988_link *content__free_rfc5988_link(સંરચના content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
શૂન્ય content__reformat(સંરચના content *c, bool background, પૂર્ણાંક width, પૂર્ણાંક height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
શૂન્ય content__request_redraw(સંરચના content *c, પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(સંરચના content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(સંરચના content *c, અચલ અક્ષર *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
અચલ અક્ષર *content__get_title(સંરચના content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
અચલ અક્ષર *content__get_status_message(સંરચના content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
પૂર્ણાંક content__get_width(સંરચના content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
પૂર્ણાંક content__get_height(સંરચના content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
પૂર્ણાંક content__get_available_width(સંરચના content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
અચલ uint8_t *content__get_source_data(સંરચના content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
શૂન્ય content__invalidate_reuse_data(સંરચના content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
સંરચના nsurl *content__get_refresh_url(સંરચના content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
સંરચના bitmap *content__get_bitmap(સંરચના content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(સંરચના content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
અચલ અક્ષર *content__get_encoding(સંરચના content *c, ગણના content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(સંરચના content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
શૂન્ય content_destroy(સંરચના content *c);

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
bool content_add_user(સંરચના content *h,
		      શૂન્ય (*callback)(
				       સંરચના content *c,
				       content_સંદેશ સંદેશ,
				       અચલ સંઘ content_સંદેશ_data *data,
				       શૂન્ય *pw),
		      શૂન્ય *pw);

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
શૂન્ય content_remove_user(સંરચના content *c,
			 શૂન્ય (*callback)(
					  સંરચના content *c,
					  content_સંદેશ સંદેશ,
					  અચલ સંઘ content_સંદેશ_data *data,
					  શૂન્ય *pw),
			 શૂન્ય *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_ગણતરી_users(સંરચના content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(સંરચના content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(સંરચના content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
અચલ સંરચના llcache_handle *content_get_llcache_handle(સંરચના content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
સંરચના nsurl *content_get_url(સંરચના content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
સંરચના content *content_clone(સંરચના content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(સંરચના content *c);

#endif
