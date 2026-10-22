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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <libwapcaplet/libwapcaplet.h>

#അന്തര്ഭാവ "netsurf/content_type.h"
#അന്തര്ഭാവ "netsurf/mouse.h" /* mouse state enums */

ഘടന nsurl;
ഘടന content_redraw_data;
സംഘം content_സന്ദേശം_data;
ഘടന http_parameter;
ഘടന llcache_handle;
ഘടന object_params;
ഘടന content;
ഘടന redraw_context;
ഘടന rect;
ഘടന browser_window;
ഘടന browser_window_features;
ഘടന textsearch_context;
ഘടന box;
ഘടന selection;
ഘടന selection_string;

തരനിർവചനം ഘടന content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ഘടന content_handler {
	ശൂന്യം (*fini)(ശൂന്യം);

	nserror (*create)(സ്ഥിരം ഘടന content_handler *handler,
                          lwc_string *imime_type,
                          സ്ഥിരം ഘടന http_parameter *params,
                          ഘടന llcache_handle *llcache,
                          സ്ഥിരം അക്ഷരം *fallback_charset, bool quirks,
                          ഘടന content **c);

	bool (*process_data)(ഘടന content *c,
			സ്ഥിരം അക്ഷരം *data, unsigned പൂർണ്ണസംഖ്യ size);
	bool (*data_complete)(ഘടന content *c);
	ശൂന്യം (*reformat)(ഘടന content *c, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height);
	ശൂന്യം (*destroy)(ഘടന content *c);
	ശൂന്യം (*stop)(ഘടന content *c);
	nserror (*mouse_track)(ഘടന content *c, ഘടന browser_window *bw,
			browser_mouse_state mouse, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y);
	nserror (*mouse_action)(ഘടന content *c, ഘടന browser_window *bw,
			browser_mouse_state mouse, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y);
	bool (*keypress)(ഘടന content *c, uint32_t താക്കോൽ);
	bool (*redraw)(ഘടന content *c, ഘടന content_redraw_data *data,
			സ്ഥിരം ഘടന rect *clip,
			സ്ഥിരം ഘടന redraw_context *ctx);
	nserror (*open)(ഘടന content *c, ഘടന browser_window *bw,
			ഘടന content *page, ഘടന object_params *params);
	nserror (*close)(ഘടന content *c);
	ശൂന്യം (*clear_selection)(ഘടന content *c);
	അക്ഷരം * (*get_selection)(ഘടന content *c);
	nserror (*get_contextual_content)(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			ഘടന browser_window_features *data);
	bool (*scroll_at_point)(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			പൂർണ്ണസംഖ്യ scrx, പൂർണ്ണസംഖ്യ scry);
	bool (*drop_file_at_point)(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			അക്ഷരം *file);
	nserror (*debug_dump)(ഘടന content *c, FILE *f, എണ്ണൽ content_debug op);
	nserror (*debug)(ഘടന content *c, എണ്ണൽ content_debug op);
	nserror (*clone)(സ്ഥിരം ഘടന content *old, ഘടന content **newc);
	bool (*matches_quirks)(സ്ഥിരം ഘടന content *c, bool quirks);
	സ്ഥിരം അക്ഷരം *(*get_encoding)(സ്ഥിരം ഘടന content *c, എണ്ണൽ content_encoding_type op);
	content_type (*type)(ശൂന്യം);
	ശൂന്യം (*add_user)(ഘടന content *c);
	ശൂന്യം (*remove_user)(ഘടന content *c);
	bool (*exec)(ഘടന content *c, സ്ഥിരം അക്ഷരം *src, size_t srclen);
	bool (*saw_insecure_objects)(ഘടന content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(ഘടന content *c, ഘടന textsearch_context *context, സ്ഥിരം അക്ഷരം *pattern, പൂർണ്ണസംഖ്യ p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ഘടന content *c, unsigned start_idx, unsigned end_idx, ഘടന box *start_ptr, ഘടന box *end_ptr, ഘടന rect *bounds_out);

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
	nserror (*textselection_redraw)(ഘടന content *c, unsigned start_idx, unsigned end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ഘടന content *c, unsigned start_idx, unsigned end_idx, ഘടന selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(ഘടന content *c, unsigned *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	ശൂന്യം *(*get_internal)(സ്ഥിരം ഘടന content *c, ശൂന്യം *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ഘടന content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ഘടന content_user
{
	ശൂന്യം (*callback)(
			ഘടന content *c,
			content_സന്ദേശം സന്ദേശം,
			സ്ഥിരം സംഘം content_സന്ദേശം_data *data,
			ശൂന്യം *pw);
	ശൂന്യം *pw;

	ഘടന content_user *next;
};

/**
 * Content which corresponds to a single URL.
 */
ഘടന content {
	/**
	 * Low-level cache object
	 */
	ഘടന llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	സ്ഥിരം ഘടന content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	പൂർണ്ണസംഖ്യ width;
	/**
	 * Height dimension, if applicable.
	 */
	പൂർണ്ണസംഖ്യ height;
	/**
	 * Viewport width.
	 */
	പൂർണ്ണസംഖ്യ available_width;
	/**
	 * Viewport height.
	 */
	പൂർണ്ണസംഖ്യ available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	അക്ഷരം *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ഘടന nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ഘടന content_rfc5988_link *links;

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
	unsigned പൂർണ്ണസംഖ്യ size;
	/**
	 * Title for browser window.
	 */
	അക്ഷരം *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	unsigned പൂർണ്ണസംഖ്യ active;
	/**
	 * List of users.
	 */
	ഘടന content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	അക്ഷരം status_message[120];
	/**
	 * Status of content.
	 */
	അക്ഷരം sub_status[80];
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
	ഘടന {
		അക്ഷരം *string;
		ഘടന textsearch_context *context;
	} textsearch;
};

ബാഹ്യ സ്ഥിരം അക്ഷരം * സ്ഥിരം content_type_name[];
ബാഹ്യ സ്ഥിരം അക്ഷരം * സ്ഥിരം content_status_name[];


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
nserror content__init(ഘടന content *c, സ്ഥിരം ഘടന content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		ഘടന llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(സ്ഥിരം ഘടന content *c, ഘടന content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
ശൂന്യം content_set_ready(ഘടന content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
ശൂന്യം content_set_done(ഘടന content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
ശൂന്യം content_set_error(ഘടന content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
ശൂന്യം content_set_status(ഘടന content *c, സ്ഥിരം അക്ഷരം *status_message);

/**
 * Send a message to all users.
 */
ശൂന്യം content_broadcast(ഘടന content *c, content_സന്ദേശം സന്ദേശം, സ്ഥിരം സംഘം content_സന്ദേശം_data *data);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
ശൂന്യം content_broadcast_error(ഘടന content *c, nserror errorcode, സ്ഥിരം അക്ഷരം *സന്ദേശം);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(ഘടന content *c, സ്ഥിരം ഘടന content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ഘടന content_rfc5988_link *content__free_rfc5988_link(ഘടന content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
ശൂന്യം content__reformat(ഘടന content *c, bool background, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ശൂന്യം content__request_redraw(ഘടന content *c, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ഘടന content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(ഘടന content *c, സ്ഥിരം അക്ഷരം *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
സ്ഥിരം അക്ഷരം *content__get_title(ഘടന content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
സ്ഥിരം അക്ഷരം *content__get_status_message(ഘടന content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
പൂർണ്ണസംഖ്യ content__get_width(ഘടന content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
പൂർണ്ണസംഖ്യ content__get_height(ഘടന content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
പൂർണ്ണസംഖ്യ content__get_available_width(ഘടന content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
സ്ഥിരം uint8_t *content__get_source_data(ഘടന content *c, size_t *size);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
ശൂന്യം content__invalidate_reuse_data(ഘടന content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ഘടന nsurl *content__get_refresh_url(ഘടന content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ഘടന bitmap *content__get_bitmap(ഘടന content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ഘടന content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
സ്ഥിരം അക്ഷരം *content__get_encoding(ഘടന content *c, എണ്ണൽ content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ഘടന content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
ശൂന്യം content_destroy(ഘടന content *c);

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
bool content_add_user(ഘടന content *h,
		      ശൂന്യം (*callback)(
				       ഘടന content *c,
				       content_സന്ദേശം സന്ദേശം,
				       സ്ഥിരം സംഘം content_സന്ദേശം_data *data,
				       ശൂന്യം *pw),
		      ശൂന്യം *pw);

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
ശൂന്യം content_remove_user(ഘടന content *c,
			 ശൂന്യം (*callback)(
					  ഘടന content *c,
					  content_സന്ദേശം സന്ദേശം,
					  സ്ഥിരം സംഘം content_സന്ദേശം_data *data,
					  ശൂന്യം *pw),
			 ശൂന്യം *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_എണ്ണം_users(ഘടന content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ഘടന content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ഘടന content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
സ്ഥിരം ഘടന llcache_handle *content_get_llcache_handle(ഘടന content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ഘടന nsurl *content_get_url(ഘടന content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ഘടന content *content_clone(ഘടന content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ഘടന content *c);

#endif
