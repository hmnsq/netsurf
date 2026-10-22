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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_CONTENT_PROTECTED_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_CONTENT_PROTECTED_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libwapcaplet/libwapcaplet.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content_type.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h" /* mouse state enums */

ਬਣਤਰ nsurl;
ਬਣਤਰ content_redraw_ਅੰਕੜੇ;
ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ;
ਬਣਤਰ http_parameter;
ਬਣਤਰ llcache_handle;
ਬਣਤਰ object_params;
ਬਣਤਰ content;
ਬਣਤਰ redraw_context;
ਬਣਤਰ rect;
ਬਣਤਰ browser_ਖਿੜਕੀ;
ਬਣਤਰ browser_ਖਿੜਕੀ_features;
ਬਣਤਰ textsearch_context;
ਬਣਤਰ box;
ਬਣਤਰ selection;
ਬਣਤਰ selection_string;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ਬਣਤਰ content_handler {
	ਖਾਲੀ (*fini)(ਖਾਲੀ);

	nserror (*create)(ਅਚਲ ਬਣਤਰ content_handler *handler,
                          lwc_string *imime_type,
                          ਅਚਲ ਬਣਤਰ http_parameter *params,
                          ਬਣਤਰ llcache_handle *llcache,
                          ਅਚਲ ਅੱਖਰ *fallback_charset, bool quirks,
                          ਬਣਤਰ content **c);

	bool (*process_ਅੰਕੜੇ)(ਬਣਤਰ content *c,
			ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ);
	bool (*ਅੰਕੜੇ_complete)(ਬਣਤਰ content *c);
	ਖਾਲੀ (*reformat)(ਬਣਤਰ content *c, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);
	ਖਾਲੀ (*destroy)(ਬਣਤਰ content *c);
	ਖਾਲੀ (*stop)(ਬਣਤਰ content *c);
	nserror (*mouse_track)(ਬਣਤਰ content *c, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);
	nserror (*mouse_action)(ਬਣਤਰ content *c, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			browser_mouse_state mouse, ਪੂਰਨ x, ਪੂਰਨ y);
	bool (*keypress)(ਬਣਤਰ content *c, uint32_t ਕੁੰਜੀ);
	bool (*redraw)(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
			ਅਚਲ ਬਣਤਰ rect *clip,
			ਅਚਲ ਬਣਤਰ redraw_context *ctx);
	nserror (*open)(ਬਣਤਰ content *c, ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
			ਬਣਤਰ content *ਪੰਨਾ, ਬਣਤਰ object_params *params);
	nserror (*close)(ਬਣਤਰ content *c);
	ਖਾਲੀ (*clear_selection)(ਬਣਤਰ content *c);
	ਅੱਖਰ * (*get_selection)(ਬਣਤਰ content *c);
	nserror (*get_contextual_content)(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y,
			ਬਣਤਰ browser_ਖਿੜਕੀ_features *ਅੰਕੜੇ);
	bool (*scroll_at_ਬਿੰਦੂ)(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y,
			ਪੂਰਨ scrx, ਪੂਰਨ scry);
	bool (*drop_file_at_ਬਿੰਦੂ)(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y,
			ਅੱਖਰ *file);
	nserror (*debug_dump)(ਬਣਤਰ content *c, FILE *f, ਗਿਣਤੀ content_debug op);
	nserror (*debug)(ਬਣਤਰ content *c, ਗਿਣਤੀ content_debug op);
	nserror (*clone)(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc);
	bool (*matches_quirks)(ਅਚਲ ਬਣਤਰ content *c, bool quirks);
	ਅਚਲ ਅੱਖਰ *(*get_encoding)(ਅਚਲ ਬਣਤਰ content *c, ਗਿਣਤੀ content_encoding_type op);
	content_type (*type)(ਖਾਲੀ);
	ਖਾਲੀ (*ਜੋੜੋ_user)(ਬਣਤਰ content *c);
	ਖਾਲੀ (*remove_user)(ਬਣਤਰ content *c);
	bool (*exec)(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *src, size_t srclen);
	bool (*saw_insecure_objects)(ਬਣਤਰ content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_ਲੱਭੋ)(ਬਣਤਰ content *c, ਬਣਤਰ textsearch_context *context, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ, ਪੂਰਨ p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx, ਬਣਤਰ box *ਸ਼ੁਰੂ_ptr, ਬਣਤਰ box *end_ptr, ਬਣਤਰ rect *bounds_ਬਾਹਰਲਾ);

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
	nserror (*textselection_redraw)(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx, ਬਣਤਰ selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_ਸਿਰਾ)(ਬਣਤਰ content *c, ਬਿਨਾਨਿਸ਼ਾਨੀ *ਸਿਰਾ_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	ਖਾਲੀ *(*get_ਅੰਦਰੂਨੀ)(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ਬਣਤਰ content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ਬਣਤਰ content_user
{
	ਖਾਲੀ (*callback)(
			ਬਣਤਰ content *c,
			content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
			ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ,
			ਖਾਲੀ *pw);
	ਖਾਲੀ *pw;

	ਬਣਤਰ content_user *ਅਗਲਾ;
};

/**
 * Content which corresponds to a single URL.
 */
ਬਣਤਰ content {
	/**
	 * Low-level cache object
	 */
	ਬਣਤਰ llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ਅਚਲ ਬਣਤਰ content_handler *handler;

	/**
	 * Current status.
	 */
	content_ਸਥਿਤੀ ਸਥਿਤੀ;

	/**
	 * Width dimension, if applicable.
	 */
	ਪੂਰਨ ਚੌੜਾਈ;
	/**
	 * Height dimension, if applicable.
	 */
	ਪੂਰਨ ਉਚਾਈ;
	/**
	 * Viewport width.
	 */
	ਪੂਰਨ ਉਪਲਬਧ_ਚੌੜਾਈ;
	/**
	 * Viewport height.
	 */
	ਪੂਰਨ ਉਪਲਬਧ_ਉਚਾਈ;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	ਅੱਖਰ *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ਬਣਤਰ nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ਬਣਤਰ content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t time;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_ਸਮਾਂ;

	/**
	 * Estimated size of all data associated with this content
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ;
	/**
	 * Title for browser window.
	 */
	ਅੱਖਰ *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਰਗਰਮ;
	/**
	 * List of users.
	 */
	ਬਣਤਰ content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	ਅੱਖਰ ਸਥਿਤੀ_ਸੁਨੇਹਾ[120];
	/**
	 * Status of content.
	 */
	ਅੱਖਰ sub_ਸਥਿਤੀ[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ_ਮਾਪ;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	ਲੰਮਾ http_ਕੋਡ;

	/**
	 * Free text search state
	 */
	ਬਣਤਰ {
		ਅੱਖਰ *string;
		ਬਣਤਰ textsearch_context *context;
	} textsearch;
};

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ * ਅਚਲ content_type_name[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ * ਅਚਲ content_ਸਥਿਤੀ_name[];


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
nserror content__init(ਬਣਤਰ content *c, ਅਚਲ ਬਣਤਰ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		ਬਣਤਰ llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ਅਚਲ ਬਣਤਰ content *c, ਬਣਤਰ content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
ਖਾਲੀ content_ਸੈੱਟ_ready(ਬਣਤਰ content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
ਖਾਲੀ content_ਸੈੱਟ_ਪੂਰਾ(ਬਣਤਰ content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
ਖਾਲੀ content_ਸੈੱਟ_error(ਬਣਤਰ content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
ਖਾਲੀ content_ਸੈੱਟ_ਸਥਿਤੀ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ_ਸੁਨੇਹਾ);

/**
 * Send a message to all users.
 */
ਖਾਲੀ content_ਪ੍ਰਸਾਰਣ(ਬਣਤਰ content *c, content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ, ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
ਖਾਲੀ content_ਪ੍ਰਸਾਰਣ_error(ਬਣਤਰ content *c, nserror errorcode, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content_ਜੋੜੋ_rfc5988_link(ਬਣਤਰ content *c, ਅਚਲ ਬਣਤਰ content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ਬਣਤਰ content_rfc5988_link *content__free_rfc5988_link(ਬਣਤਰ content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
ਖਾਲੀ content__reformat(ਬਣਤਰ content *c, bool background, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ਖਾਲੀ content__request_redraw(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ਬਣਤਰ content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content_ਸੈੱਟ_title(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ਅਚਲ ਅੱਖਰ *content__get_title(ਬਣਤਰ content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ਅਚਲ ਅੱਖਰ *content_get_ਸਥਿਤੀ_ਸੁਨੇਹਾ_2(ਬਣਤਰ content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
ਪੂਰਨ content_get_ਚੌੜਾਈ_2(ਬਣਤਰ content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
ਪੂਰਨ content_get_ਉਚਾਈ_2(ਬਣਤਰ content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
ਪੂਰਨ content_get_ਉਪਲਬਧ_ਚੌੜਾਈ_2(ਬਣਤਰ content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ਅਚਲ uint8_t *content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(ਬਣਤਰ content *c, size_t *ਮਾਪ);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
ਖਾਲੀ content_invalidate_reuse_ਅੰਕੜੇ_2(ਬਣਤਰ content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ਬਣਤਰ nsurl *content__get_refresh_url(ਬਣਤਰ content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ਬਣਤਰ bitmap *content__get_bitmap(ਬਣਤਰ content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ਬਣਤਰ content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ਅਚਲ ਅੱਖਰ *content__get_encoding(ਬਣਤਰ content *c, ਗਿਣਤੀ content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ਬਣਤਰ content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
ਖਾਲੀ content_destroy(ਬਣਤਰ content *c);

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
bool content_ਜੋੜੋ_user(ਬਣਤਰ content *h,
		      ਖਾਲੀ (*callback)(
				       ਬਣਤਰ content *c,
				       content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
				       ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ,
				       ਖਾਲੀ *pw),
		      ਖਾਲੀ *pw);

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
ਖਾਲੀ content_remove_user(ਬਣਤਰ content *c,
			 ਖਾਲੀ (*callback)(
					  ਬਣਤਰ content *c,
					  content_ਸੁਨੇਹਾ ਸੁਨੇਹਾ,
					  ਅਚਲ ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ *ਅੰਕੜੇ,
					  ਖਾਲੀ *pw),
			 ਖਾਲੀ *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_ਗਿਣਤੀ_users(ਬਣਤਰ content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ਬਣਤਰ content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ਬਣਤਰ content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ਅਚਲ ਬਣਤਰ llcache_handle *content_get_llcache_handle(ਬਣਤਰ content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ਬਣਤਰ nsurl *content_get_url(ਬਣਤਰ content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ਬਣਤਰ content *content_clone(ਬਣਤਰ content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ਬਣਤਰ content *c);

#ਜੇ_ਅੰਤ
