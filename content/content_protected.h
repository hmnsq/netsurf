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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <libwapcaplet/libwapcaplet.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/content_type.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h" /* mouse state enums */

ସଂରଚନା nsurl;
ସଂରଚନା content_redraw_data;
ସଂଘ content_ସନ୍ଦେଶ_data;
ସଂରଚନା http_parameter;
ସଂରଚନା llcache_handle;
ସଂରଚନା object_params;
ସଂରଚନା content;
ସଂରଚନା redraw_context;
ସଂରଚନା rect;
ସଂରଚନା browser_window;
ସଂରଚନା browser_window_features;
ସଂରଚନା textsearch_context;
ସଂରଚନା box;
ସଂରଚନା selection;
ସଂରଚନା selection_string;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ସଂରଚନା content_handler {
	ଶୂନ୍ୟ (*fini)(ଶୂନ୍ୟ);

	nserror (*create)(ଅଚଳ ସଂରଚନା content_handler *handler,
                          lwc_string *imime_type,
                          ଅଚଳ ସଂରଚନା http_parameter *params,
                          ସଂରଚନା llcache_handle *llcache,
                          ଅଚଳ ଅକ୍ଷର *fallback_charset, bool quirks,
                          ସଂରଚନା content **c);

	bool (*process_data)(ସଂରଚନା content *c,
			ଅଚଳ ଅକ୍ଷର *data, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size);
	bool (*data_complete)(ସଂରଚନା content *c);
	ଶୂନ୍ୟ (*reformat)(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height);
	ଶୂନ୍ୟ (*destroy)(ସଂରଚନା content *c);
	ଶୂନ୍ୟ (*stop)(ସଂରଚନା content *c);
	nserror (*mouse_track)(ସଂରଚନା content *c, ସଂରଚନା browser_window *bw,
			browser_mouse_state mouse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y);
	nserror (*mouse_action)(ସଂରଚନା content *c, ସଂରଚନା browser_window *bw,
			browser_mouse_state mouse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y);
	bool (*keypress)(ସଂରଚନା content *c, uint32_t ଚାବି);
	bool (*redraw)(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
			ଅଚଳ ସଂରଚନା rect *clip,
			ଅଚଳ ସଂରଚନା redraw_context *ctx);
	nserror (*open)(ସଂରଚନା content *c, ସଂରଚନା browser_window *bw,
			ସଂରଚନା content *page, ସଂରଚନା object_params *params);
	nserror (*close)(ସଂରଚନା content *c);
	ଶୂନ୍ୟ (*clear_selection)(ସଂରଚନା content *c);
	ଅକ୍ଷର * (*get_selection)(ସଂରଚନା content *c);
	nserror (*get_contextual_content)(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			ସଂରଚନା browser_window_features *data);
	bool (*scroll_at_point)(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା scry);
	bool (*drop_file_at_point)(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			ଅକ୍ଷର *file);
	nserror (*debug_dump)(ସଂରଚନା content *c, FILE *f, ଗଣନା content_debug op);
	nserror (*debug)(ସଂରଚନା content *c, ଗଣନା content_debug op);
	nserror (*clone)(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc);
	bool (*matches_quirks)(ଅଚଳ ସଂରଚନା content *c, bool quirks);
	ଅଚଳ ଅକ୍ଷର *(*get_encoding)(ଅଚଳ ସଂରଚନା content *c, ଗଣନା content_encoding_type op);
	content_type (*type)(ଶୂନ୍ୟ);
	ଶୂନ୍ୟ (*add_user)(ସଂରଚନା content *c);
	ଶୂନ୍ୟ (*remove_user)(ସଂରଚନା content *c);
	bool (*exec)(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *src, size_t srclen);
	bool (*saw_insecure_objects)(ସଂରଚନା content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(ସଂରଚନା content *c, ସଂରଚନା textsearch_context *context, ଅଚଳ ଅକ୍ଷର *pattern, ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ସଂରଚନା content *c, unsigned start_idx, unsigned end_idx, ସଂରଚନା box *start_ptr, ସଂରଚନା box *end_ptr, ସଂରଚନା rect *bounds_out);

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
	nserror (*textselection_redraw)(ସଂରଚନା content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ସଂରଚନା content *c, unsigned start_idx, unsigned end_idx, ସଂରଚନା selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(ସଂରଚନା content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	ଶୂନ୍ୟ *(*get_internal)(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ସଂରଚନା content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ସଂରଚନା content_user
{
	ଶୂନ୍ୟ (*callback)(
			ସଂରଚନା content *c,
			content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
			ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data,
			ଶୂନ୍ୟ *pw);
	ଶୂନ୍ୟ *pw;

	ସଂରଚନା content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
ସଂରଚନା content {
	/**
	 * Low-level cache object
	 */
	ସଂରଚନା llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ଅଚଳ ସଂରଚନା content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
	/**
	 * Height dimension, if applicable.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height;
	/**
	 * Viewport width.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_width;
	/**
	 * Viewport height.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	ଅକ୍ଷର *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ସଂରଚନା nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ସଂରଚନା content_rfc5988_link *links;

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
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
	/**
	 * Title for browser window.
	 */
	ଅକ୍ଷର *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା active;
	/**
	 * List of users.
	 */
	ସଂରଚନା content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	ଅକ୍ଷର status_message[120];
	/**
	 * Status of content.
	 */
	ଅକ୍ଷର sub_status[80];
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
	ସଂରଚନା {
		ଅକ୍ଷର *string;
		ସଂରଚନା textsearch_context *context;
	} textsearch;
};

ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର * ଅଚଳ content_type_name[];
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର * ଅଚଳ content_status_name[];


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
nserror content__init(ସଂରଚନା content *c, ଅଚଳ ସଂରଚନା content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		ସଂରଚନା llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ଅଚଳ ସଂରଚନା content *c, ସଂରଚନା content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
ଶୂନ୍ୟ content_set_ready(ସଂରଚନା content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
ଶୂନ୍ୟ content_set_done(ସଂରଚନା content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
ଶୂନ୍ୟ content_set_error(ସଂରଚନା content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
ଶୂନ୍ୟ content_set_status(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *status_message);

/**
 * Send a message to all users.
 */
ଶୂନ୍ୟ content_broadcast(ସଂରଚନା content *c, content_ସନ୍ଦେଶ ସନ୍ଦେଶ, ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
ଶୂନ୍ୟ content_broadcast_error(ସଂରଚନା content *c, nserror errorcode, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(ସଂରଚନା content *c, ଅଚଳ ସଂରଚନା content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ସଂରଚନା content_rfc5988_link *content__free_rfc5988_link(ସଂରଚନା content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
ଶୂନ୍ୟ content__reformat(ସଂରଚନା content *c, bool background, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ଶୂନ୍ୟ content__request_redraw(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ସଂରଚନା content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ଅଚଳ ଅକ୍ଷର *content__get_title(ସଂରଚନା content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ଅଚଳ ଅକ୍ଷର *content__get_status_message(ସଂରଚନା content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_width(ସଂରଚନା content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_height(ସଂରଚନା content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା content__get_available_width(ସଂରଚନା content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ଅଚଳ uint8_t *content__get_source_data(ସଂରଚନା content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
ଶୂନ୍ୟ content__invalidate_reuse_data(ସଂରଚନା content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ସଂରଚନା nsurl *content__get_refresh_url(ସଂରଚନା content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ସଂରଚନା bitmap *content__get_bitmap(ସଂରଚନା content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ସଂରଚନା content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ଅଚଳ ଅକ୍ଷର *content__get_encoding(ସଂରଚନା content *c, ଗଣନା content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ସଂରଚନା content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
ଶୂନ୍ୟ content_destroy(ସଂରଚନା content *c);

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
bool content_add_user(ସଂରଚନା content *h,
		      ଶୂନ୍ୟ (*callback)(
				       ସଂରଚନା content *c,
				       content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
				       ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data,
				       ଶୂନ୍ୟ *pw),
		      ଶୂନ୍ୟ *pw);

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
ଶୂନ୍ୟ content_remove_user(ସଂରଚନା content *c,
			 ଶୂନ୍ୟ (*callback)(
					  ସଂରଚନା content *c,
					  content_ସନ୍ଦେଶ ସନ୍ଦେଶ,
					  ଅଚଳ ସଂଘ content_ସନ୍ଦେଶ_data *data,
					  ଶୂନ୍ୟ *pw),
			 ଶୂନ୍ୟ *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_ଗଣନା_users(ସଂରଚନା content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ସଂରଚନା content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ସଂରଚନା content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ଅଚଳ ସଂରଚନା llcache_handle *content_get_llcache_handle(ସଂରଚନା content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ସଂରଚନା nsurl *content_get_url(ସଂରଚନା content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ସଂରଚନା content *content_clone(ସଂରଚନା content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ସଂରଚନା content *c);

#endif
