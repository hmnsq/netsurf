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

#include <stdio.h>
#include <libwapcaplet/libwapcaplet.h>

#include "netsurf/content_type.h"
#include "netsurf/mouse.h" /* mouse state enums */

sa_nracanaa nsurl;
sa_nracanaa content_redraw_data;
sa_ngha content_msg_data;
sa_nracanaa http_parameter;
sa_nracanaa llcache_handle;
sa_nracanaa object_params;
sa_nracanaa content;
sa_nracanaa redraw_context;
sa_nracanaa rect;
sa_nracanaa browser_window;
sa_nracanaa browser_window_features;
sa_nracanaa textsearch_context;
sa_nracanaa box;
sa_nracanaa selection;
sa_nracanaa selection_string;

typedef sa_nracanaa content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
sa_nracanaa content_handler {
	void (*fini)(void);

	nserror (*create)(const sa_nracanaa content_handler *handler,
                          lwc_string *imime_type,
                          const sa_nracanaa http_parameter *params,
                          sa_nracanaa llcache_handle *llcache,
                          const var_na *fallback_charset, bool quirks,
                          sa_nracanaa content **c);

	bool (*process_data)(sa_nracanaa content *c,
			const var_na *data, unsigned පූර්ණ_සංඛ්‍යා size);
	bool (*data_complete)(sa_nracanaa content *c);
	void (*reformat)(sa_nracanaa content *c, පූර්ණ_සංඛ්‍යා width, පූර්ණ_සංඛ්‍යා height);
	void (*destroy)(sa_nracanaa content *c);
	void (*stop)(sa_nracanaa content *c);
	nserror (*mouse_track)(sa_nracanaa content *c, sa_nracanaa browser_window *bw,
			browser_mouse_state mouse, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y);
	nserror (*mouse_action)(sa_nracanaa content *c, sa_nracanaa browser_window *bw,
			browser_mouse_state mouse, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y);
	bool (*keypress)(sa_nracanaa content *c, uint32_t key);
	bool (*redraw)(sa_nracanaa content *c, sa_nracanaa content_redraw_data *data,
			const sa_nracanaa rect *clip,
			const sa_nracanaa redraw_context *ctx);
	nserror (*open)(sa_nracanaa content *c, sa_nracanaa browser_window *bw,
			sa_nracanaa content *page, sa_nracanaa object_params *params);
	nserror (*close)(sa_nracanaa content *c);
	void (*clear_selection)(sa_nracanaa content *c);
	var_na * (*get_selection)(sa_nracanaa content *c);
	nserror (*get_contextual_content)(sa_nracanaa content *c, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y,
			sa_nracanaa browser_window_features *data);
	bool (*scroll_at_point)(sa_nracanaa content *c, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y,
			පූර්ණ_සංඛ්‍යා scrx, පූර්ණ_සංඛ්‍යා scry);
	bool (*drop_file_at_point)(sa_nracanaa content *c, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y,
			var_na *file);
	nserror (*debug_dump)(sa_nracanaa content *c, FILE *f, ga_nanaa content_debug op);
	nserror (*debug)(sa_nracanaa content *c, ga_nanaa content_debug op);
	nserror (*clone)(const sa_nracanaa content *old, sa_nracanaa content **newc);
	bool (*matches_quirks)(const sa_nracanaa content *c, bool quirks);
	const var_na *(*get_encoding)(const sa_nracanaa content *c, ga_nanaa content_encoding_type op);
	content_type (*type)(void);
	void (*add_user)(sa_nracanaa content *c);
	void (*remove_user)(sa_nracanaa content *c);
	bool (*exec)(sa_nracanaa content *c, const var_na *src, size_t srclen);
	bool (*saw_insecure_objects)(sa_nracanaa content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(sa_nracanaa content *c, sa_nracanaa textsearch_context *context, const var_na *pattern, පූර්ණ_සංඛ්‍යා p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(sa_nracanaa content *c, unsigned start_idx, unsigned end_idx, sa_nracanaa box *start_ptr, sa_nracanaa box *end_ptr, sa_nracanaa rect *bounds_out);

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
	nserror (*textselection_redraw)(sa_nracanaa content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(sa_nracanaa content *c, unsigned start_idx, unsigned end_idx, sa_nracanaa selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(sa_nracanaa content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	void *(*get_internal)(const sa_nracanaa content *c, void *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(sa_nracanaa content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
sa_nracanaa content_user
{
	void (*callback)(
			sa_nracanaa content *c,
			content_msg msg,
			const sa_ngha content_msg_data *data,
			void *pw);
	void *pw;

	sa_nracanaa content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
sa_nracanaa content {
	/**
	 * Low-level cache object
	 */
	sa_nracanaa llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	const sa_nracanaa content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	පූර්ණ_සංඛ්‍යා width;
	/**
	 * Height dimension, if applicable.
	 */
	පූර්ණ_සංඛ්‍යා height;
	/**
	 * Viewport width.
	 */
	පූර්ණ_සංඛ්‍යා available_width;
	/**
	 * Viewport height.
	 */
	පූර්ණ_සංඛ්‍යා available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	var_na *fallback_charset;

	/**
	 * URL for refresh request
	 */
	sa_nracanaa nsurl *refresh;

	/**
	 * list of metadata links
	 */
	sa_nracanaa content_rfc5988_link *links;

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
	unsigned පූර්ණ_සංඛ්‍යා size;
	/**
	 * Title for browser window.
	 */
	var_na *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned පූර්ණ_සංඛ්‍යා active;
	/**
	 * List of users.
	 */
	sa_nracanaa content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	var_na status_message[120];
	/**
	 * Status of content.
	 */
	var_na sub_status[80];
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
	sa_nracanaa {
		var_na *string;
		sa_nracanaa textsearch_context *context;
	} textsearch;
};

extern const var_na * const content_type_name[];
extern const var_na * const content_status_name[];


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
nserror content__init(sa_nracanaa content *c, const sa_nracanaa content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		sa_nracanaa llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(const sa_nracanaa content *c, sa_nracanaa content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
void content_set_ready(sa_nracanaa content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
void content_set_done(sa_nracanaa content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
void content_set_error(sa_nracanaa content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
void content_set_status(sa_nracanaa content *c, const var_na *status_message);

/**
 * Send a message to all users.
 */
void content_broadcast(sa_nracanaa content *c, content_msg msg, const sa_ngha content_msg_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
void content_broadcast_error(sa_nracanaa content *c, nserror errorcode, const var_na *msg);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(sa_nracanaa content *c, const sa_nracanaa content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
sa_nracanaa content_rfc5988_link *content__free_rfc5988_link(sa_nracanaa content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
void content__reformat(sa_nracanaa content *c, bool background, පූර්ණ_සංඛ්‍යා width, පූර්ණ_සංඛ්‍යා height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
void content__request_redraw(sa_nracanaa content *c, පූර්ණ_සංඛ්‍යා x, පූර්ණ_සංඛ්‍යා y, පූර්ණ_සංඛ්‍යා width, පූර්ණ_සංඛ්‍යා height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(sa_nracanaa content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(sa_nracanaa content *c, const var_na *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
const var_na *content__get_title(sa_nracanaa content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
const var_na *content__get_status_message(sa_nracanaa content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
පූර්ණ_සංඛ්‍යා content__get_width(sa_nracanaa content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
පූර්ණ_සංඛ්‍යා content__get_height(sa_nracanaa content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
පූර්ණ_සංඛ්‍යා content__get_available_width(sa_nracanaa content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
const uint8_t *content__get_source_data(sa_nracanaa content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
void content__invalidate_reuse_data(sa_nracanaa content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
sa_nracanaa nsurl *content__get_refresh_url(sa_nracanaa content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
sa_nracanaa bitmap *content__get_bitmap(sa_nracanaa content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(sa_nracanaa content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
const var_na *content__get_encoding(sa_nracanaa content *c, ga_nanaa content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(sa_nracanaa content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
void content_destroy(sa_nracanaa content *c);

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
bool content_add_user(sa_nracanaa content *h,
		      void (*callback)(
				       sa_nracanaa content *c,
				       content_msg msg,
				       const sa_ngha content_msg_data *data,
				       void *pw),
		      void *pw);

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
void content_remove_user(sa_nracanaa content *c,
			 void (*callback)(
					  sa_nracanaa content *c,
					  content_msg msg,
					  const sa_ngha content_msg_data *data,
					  void *pw),
			 void *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_count_users(sa_nracanaa content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(sa_nracanaa content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(sa_nracanaa content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
const sa_nracanaa llcache_handle *content_get_llcache_handle(sa_nracanaa content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
sa_nracanaa nsurl *content_get_url(sa_nracanaa content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
sa_nracanaa content *content_clone(sa_nracanaa content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(sa_nracanaa content *c);

#endif
