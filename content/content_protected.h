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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <libwapcaplet/libwapcaplet.h>

#অন্তর্ভাব "netsurf/content_type.h"
#অন্তর্ভাব "netsurf/mouse.h" /* mouse state enums */

কাঠামো nsurl;
কাঠামো content_redraw_তথ্য;
সংঘ content_বার্তা_তথ্য;
কাঠামো http_parameter;
কাঠামো llcache_handle;
কাঠামো object_params;
কাঠামো content;
কাঠামো redraw_context;
কাঠামো rect;
কাঠামো browser_window;
কাঠামো browser_window_features;
কাঠামো textsearch_context;
কাঠামো box;
কাঠামো selection;
কাঠামো selection_string;

প্রকারলেখ কাঠামো content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
কাঠামো content_handler {
	শূন্য (*fini)(শূন্য);

	nserror (*create)(ধ্রুবক কাঠামো content_handler *handler,
                          lwc_string *imime_type,
                          ধ্রুবক কাঠামো http_parameter *params,
                          কাঠামো llcache_handle *llcache,
                          ধ্রুবক অক্ষর *fallback_charset, bool quirks,
                          কাঠামো content **c);

	bool (*process_তথ্য)(কাঠামো content *c,
			ধ্রুবক অক্ষর *তথ্য, unsigned পূর্ণসংখ্যা আকার_2);
	bool (*তথ্য_complete)(কাঠামো content *c);
	শূন্য (*reformat)(কাঠামো content *c, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height);
	শূন্য (*destroy)(কাঠামো content *c);
	শূন্য (*stop)(কাঠামো content *c);
	nserror (*mouse_track)(কাঠামো content *c, কাঠামো browser_window *bw,
			browser_mouse_state mouse, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y);
	nserror (*mouse_action)(কাঠামো content *c, কাঠামো browser_window *bw,
			browser_mouse_state mouse, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y);
	bool (*keypress)(কাঠামো content *c, uint32_t চাবি);
	bool (*redraw)(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
			ধ্রুবক কাঠামো rect *clip,
			ধ্রুবক কাঠামো redraw_context *ctx);
	nserror (*open)(কাঠামো content *c, কাঠামো browser_window *bw,
			কাঠামো content *page, কাঠামো object_params *params);
	nserror (*close)(কাঠামো content *c);
	শূন্য (*clear_selection)(কাঠামো content *c);
	অক্ষর * (*get_selection)(কাঠামো content *c);
	nserror (*get_contextual_content)(কাঠামো content *c, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			কাঠামো browser_window_features *তথ্য);
	bool (*scroll_at_point)(কাঠামো content *c, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			পূর্ণসংখ্যা scrx, পূর্ণসংখ্যা scry);
	bool (*drop_file_at_point)(কাঠামো content *c, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			অক্ষর *file);
	nserror (*debug_dump)(কাঠামো content *c, FILE *f, গণনা content_debug op);
	nserror (*debug)(কাঠামো content *c, গণনা content_debug op);
	nserror (*clone)(ধ্রুবক কাঠামো content *old, কাঠামো content **newc);
	bool (*matches_quirks)(ধ্রুবক কাঠামো content *c, bool quirks);
	ধ্রুবক অক্ষর *(*get_encoding)(ধ্রুবক কাঠামো content *c, গণনা content_encoding_type op);
	content_type (*type)(শূন্য);
	শূন্য (*add_user)(কাঠামো content *c);
	শূন্য (*remove_user)(কাঠামো content *c);
	bool (*exec)(কাঠামো content *c, ধ্রুবক অক্ষর *src, size_t srclen);
	bool (*saw_insecure_objects)(কাঠামো content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(কাঠামো content *c, কাঠামো textsearch_context *context, ধ্রুবক অক্ষর *pattern, পূর্ণসংখ্যা p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(কাঠামো content *c, unsigned start_idx, unsigned end_idx, কাঠামো box *start_ptr, কাঠামো box *end_ptr, কাঠামো rect *bounds_out);

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
	nserror (*textselection_redraw)(কাঠামো content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(কাঠামো content *c, unsigned start_idx, unsigned end_idx, কাঠামো selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(কাঠামো content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	শূন্য *(*get_internal)(ধ্রুবক কাঠামো content *c, শূন্য *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(কাঠামো content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
কাঠামো content_user
{
	শূন্য (*callback)(
			কাঠামো content *c,
			content_বার্তা বার্তা,
			ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য,
			শূন্য *pw);
	শূন্য *pw;

	কাঠামো content_user *পরবর্তী;
};

/**
 * Content which corresponds to a single URL.
 */
কাঠামো content {
	/**
	 * Low-level cache object
	 */
	কাঠামো llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ধ্রুবক কাঠামো content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	পূর্ণসংখ্যা width;
	/**
	 * Height dimension, if applicable.
	 */
	পূর্ণসংখ্যা height;
	/**
	 * Viewport width.
	 */
	পূর্ণসংখ্যা available_width;
	/**
	 * Viewport height.
	 */
	পূর্ণসংখ্যা available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	অক্ষর *fallback_charset;

	/**
	 * URL for refresh request
	 */
	কাঠামো nsurl *refresh;

	/**
	 * list of metadata links
	 */
	কাঠামো content_rfc5988_link *links;

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
	unsigned পূর্ণসংখ্যা আকার_2;
	/**
	 * Title for browser window.
	 */
	অক্ষর *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned পূর্ণসংখ্যা active;
	/**
	 * List of users.
	 */
	কাঠামো content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	অক্ষর status_message[120];
	/**
	 * Status of content.
	 */
	অক্ষর sub_status[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	unsigned long যোগ_আকার;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	long http_code;

	/**
	 * Free text search state
	 */
	কাঠামো {
		অক্ষর *string;
		কাঠামো textsearch_context *context;
	} textsearch;
};

বাহ্যিক ধ্রুবক অক্ষর * ধ্রুবক content_type_name[];
বাহ্যিক ধ্রুবক অক্ষর * ধ্রুবক content_status_name[];


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
nserror content__init(কাঠামো content *c, ধ্রুবক কাঠামো content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		কাঠামো llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ধ্রুবক কাঠামো content *c, কাঠামো content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
শূন্য content_set_ready(কাঠামো content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
শূন্য content_set_done(কাঠামো content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
শূন্য content_set_error(কাঠামো content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
শূন্য content_set_status(কাঠামো content *c, ধ্রুবক অক্ষর *status_message);

/**
 * Send a message to all users.
 */
শূন্য content_broadcast(কাঠামো content *c, content_বার্তা বার্তা, ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
শূন্য content_broadcast_error(কাঠামো content *c, nserror errorcode, ধ্রুবক অক্ষর *বার্তা);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(কাঠামো content *c, ধ্রুবক কাঠামো content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
কাঠামো content_rfc5988_link *content__free_rfc5988_link(কাঠামো content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
শূন্য content__reformat(কাঠামো content *c, bool background, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
শূন্য content__request_redraw(কাঠামো content *c, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(কাঠামো content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(কাঠামো content *c, ধ্রুবক অক্ষর *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ধ্রুবক অক্ষর *content__get_title(কাঠামো content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ধ্রুবক অক্ষর *content__get_status_message(কাঠামো content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
পূর্ণসংখ্যা content__get_width(কাঠামো content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
পূর্ণসংখ্যা content__get_height(কাঠামো content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
পূর্ণসংখ্যা content__get_available_width(কাঠামো content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ধ্রুবক uint8_t *content_get_source_তথ্য_2(কাঠামো content *c, size_t *আকার_2);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
শূন্য content_invalidate_reuse_তথ্য_2(কাঠামো content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
কাঠামো nsurl *content__get_refresh_url(কাঠামো content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
কাঠামো bitmap *content__get_bitmap(কাঠামো content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(কাঠামো content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ধ্রুবক অক্ষর *content__get_encoding(কাঠামো content *c, গণনা content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(কাঠামো content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
শূন্য content_destroy(কাঠামো content *c);

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
bool content_add_user(কাঠামো content *h,
		      শূন্য (*callback)(
				       কাঠামো content *c,
				       content_বার্তা বার্তা,
				       ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য,
				       শূন্য *pw),
		      শূন্য *pw);

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
শূন্য content_remove_user(কাঠামো content *c,
			 শূন্য (*callback)(
					  কাঠামো content *c,
					  content_বার্তা বার্তা,
					  ধ্রুবক সংঘ content_বার্তা_তথ্য *তথ্য,
					  শূন্য *pw),
			 শূন্য *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_গুনতি_users(কাঠামো content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(কাঠামো content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(কাঠামো content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ধ্রুবক কাঠামো llcache_handle *content_get_llcache_handle(কাঠামো content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
কাঠামো nsurl *content_get_url(কাঠামো content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
কাঠামো content *content_clone(কাঠামো content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(কাঠামো content *c);

#endif
