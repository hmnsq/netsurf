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

#अगर_अघोषित NETSURF_CONTENT_CONTENT_PROTECTED_H_
#घोषणा NETSURF_CONTENT_CONTENT_PROTECTED_H_

#समावेश <stdio.h>
#समावेश <libwapcaplet/libwapcaplet.h>

#समावेश "netsurf/content_type.h"
#समावेश "netsurf/mouse.h" /* mouse state enums */

काष्ठा nsurl;
काष्ठा content_redraw_तथ्य;
जोड़ content_संदेश_तथ्य;
काष्ठा http_parameter;
काष्ठा llcache_handle;
काष्ठा object_params;
काष्ठा content;
काष्ठा redraw_context;
काष्ठा rect;
काष्ठा browser_window;
काष्ठा browser_window_features;
काष्ठा textsearch_context;
काष्ठा box;
काष्ठा selection;
काष्ठा selection_string;

प्रकार काष्ठा content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
काष्ठा content_handler {
	व्योम (*fini)(व्योम);

	nserror (*create)(स्थिर काष्ठा content_handler *handler,
                          lwc_string *imime_type,
                          स्थिर काष्ठा http_parameter *params,
                          काष्ठा llcache_handle *llcache,
                          स्थिर अक्षर *fallback_charset, bool quirks,
                          काष्ठा content **c);

	bool (*process_तथ्य)(काष्ठा content *c,
			स्थिर अक्षर *तथ्य, अचिन्हित पूर्णांक आकार);
	bool (*तथ्य_complete)(काष्ठा content *c);
	व्योम (*reformat)(काष्ठा content *c, पूर्णांक width, पूर्णांक height);
	व्योम (*destroy)(काष्ठा content *c);
	व्योम (*stop)(काष्ठा content *c);
	nserror (*mouse_track)(काष्ठा content *c, काष्ठा browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);
	nserror (*mouse_action)(काष्ठा content *c, काष्ठा browser_window *bw,
			browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);
	bool (*keypress)(काष्ठा content *c, uint32_t कुंजी);
	bool (*redraw)(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
			स्थिर काष्ठा rect *clip,
			स्थिर काष्ठा redraw_context *ctx);
	nserror (*खोलो)(काष्ठा content *c, काष्ठा browser_window *bw,
			काष्ठा content *page, काष्ठा object_params *params);
	nserror (*बंद)(काष्ठा content *c);
	व्योम (*clear_selection)(काष्ठा content *c);
	अक्षर * (*get_selection)(काष्ठा content *c);
	nserror (*get_contextual_content)(काष्ठा content *c, पूर्णांक x, पूर्णांक y,
			काष्ठा browser_window_features *तथ्य);
	bool (*scroll_at_point)(काष्ठा content *c, पूर्णांक x, पूर्णांक y,
			पूर्णांक scrx, पूर्णांक scry);
	bool (*drop_file_at_point)(काष्ठा content *c, पूर्णांक x, पूर्णांक y,
			अक्षर *file);
	nserror (*debug_dump)(काष्ठा content *c, खाता *f, क्रमागत content_debug op);
	nserror (*debug)(काष्ठा content *c, क्रमागत content_debug op);
	nserror (*clone)(स्थिर काष्ठा content *old, काष्ठा content **newc);
	bool (*matches_quirks)(स्थिर काष्ठा content *c, bool quirks);
	स्थिर अक्षर *(*get_encoding)(स्थिर काष्ठा content *c, क्रमागत content_encoding_type op);
	content_type (*type)(व्योम);
	व्योम (*add_user)(काष्ठा content *c);
	व्योम (*remove_user)(काष्ठा content *c);
	bool (*exec)(काष्ठा content *c, स्थिर अक्षर *src, माप_प्रकार srclen);
	bool (*saw_insecure_objects)(काष्ठा content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_find)(काष्ठा content *c, काष्ठा textsearch_context *context, स्थिर अक्षर *pattern, पूर्णांक p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(काष्ठा content *c, अचिन्हित start_idx, अचिन्हित end_idx, काष्ठा box *start_ptr, काष्ठा box *end_ptr, काष्ठा rect *bounds_out);

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
	nserror (*textselection_redraw)(काष्ठा content *c, अचिन्हित start_idx, अचिन्हित end_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(काष्ठा content *c, अचिन्हित start_idx, अचिन्हित end_idx, काष्ठा selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_end)(काष्ठा content *c, अचिन्हित *end_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	व्योम *(*get_internal)(स्थिर काष्ठा content *c, व्योम *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(काष्ठा content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
काष्ठा content_user
{
	व्योम (*callback)(
			काष्ठा content *c,
			content_संदेश संदेश,
			स्थिर जोड़ content_संदेश_तथ्य *तथ्य,
			व्योम *pw);
	व्योम *pw;

	काष्ठा content_user *अग्र;
};

/**
 * Content which corresponds to a single URL.
 */
काष्ठा content {
	/**
	 * Low-level cache object
	 */
	काष्ठा llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	स्थिर काष्ठा content_handler *handler;

	/**
	 * Current status.
	 */
	content_status status;

	/**
	 * Width dimension, if applicable.
	 */
	पूर्णांक width;
	/**
	 * Height dimension, if applicable.
	 */
	पूर्णांक height;
	/**
	 * Viewport width.
	 */
	पूर्णांक available_width;
	/**
	 * Viewport height.
	 */
	पूर्णांक available_height;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	अक्षर *fallback_charset;

	/**
	 * URL for refresh request
	 */
	काष्ठा nsurl *refresh;

	/**
	 * list of metadata links
	 */
	काष्ठा content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t समय;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_time;

	/**
	 * Estimated size of all data associated with this content
	 */
	अचिन्हित पूर्णांक आकार;
	/**
	 * Title for browser window.
	 */
	अक्षर *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	अचिन्हित पूर्णांक active;
	/**
	 * List of users.
	 */
	काष्ठा content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	अक्षर status_message[120];
	/**
	 * Status of content.
	 */
	अक्षर sub_status[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	अचिन्हित दीर्घ योग_आकार;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	दीर्घ http_code;

	/**
	 * Free text search state
	 */
	काष्ठा {
		अक्षर *string;
		काष्ठा textsearch_context *context;
	} textsearch;
};

बाह्य स्थिर अक्षर * स्थिर content_type_name[];
बाह्य स्थिर अक्षर * स्थिर content_status_name[];


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
nserror content__init(काष्ठा content *c, स्थिर काष्ठा content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		काष्ठा llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(स्थिर काष्ठा content *c, काष्ठा content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
व्योम content_set_ready(काष्ठा content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
व्योम content_set_done(काष्ठा content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
व्योम content_set_error(काष्ठा content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
व्योम content_set_status(काष्ठा content *c, स्थिर अक्षर *status_message);

/**
 * Send a message to all users.
 */
व्योम content_broadcast(काष्ठा content *c, content_संदेश संदेश, स्थिर जोड़ content_संदेश_तथ्य *तथ्य);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
व्योम content_broadcast_error(काष्ठा content *c, nserror errorcode, स्थिर अक्षर *संदेश);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content__add_rfc5988_link(काष्ठा content *c, स्थिर काष्ठा content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
काष्ठा content_rfc5988_link *content__free_rfc5988_link(काष्ठा content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
व्योम content__reformat(काष्ठा content *c, bool background, पूर्णांक width, पूर्णांक height);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
व्योम content__request_redraw(काष्ठा content *c, पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(काष्ठा content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content__set_title(काष्ठा content *c, स्थिर अक्षर *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
स्थिर अक्षर *content__get_title(काष्ठा content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
स्थिर अक्षर *content__get_status_message(काष्ठा content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
पूर्णांक content__get_width(काष्ठा content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
पूर्णांक content__get_height(काष्ठा content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
पूर्णांक content__get_available_width(काष्ठा content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
स्थिर uint8_t *content_get_source_तथ्य_2(काष्ठा content *c, माप_प्रकार *आकार);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
व्योम content_invalidate_reuse_तथ्य_2(काष्ठा content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
काष्ठा nsurl *content__get_refresh_url(काष्ठा content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
काष्ठा bitmap *content__get_bitmap(काष्ठा content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(काष्ठा content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
स्थिर अक्षर *content__get_encoding(काष्ठा content *c, क्रमागत content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(काष्ठा content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
व्योम content_destroy(काष्ठा content *c);

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
bool content_add_user(काष्ठा content *h,
		      व्योम (*callback)(
				       काष्ठा content *c,
				       content_संदेश संदेश,
				       स्थिर जोड़ content_संदेश_तथ्य *तथ्य,
				       व्योम *pw),
		      व्योम *pw);

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
व्योम content_remove_user(काष्ठा content *c,
			 व्योम (*callback)(
					  काष्ठा content *c,
					  content_संदेश संदेश,
					  स्थिर जोड़ content_संदेश_तथ्य *तथ्य,
					  व्योम *pw),
			 व्योम *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_गिनती_users(काष्ठा content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(काष्ठा content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(काष्ठा content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
स्थिर काष्ठा llcache_handle *content_get_llcache_handle(काष्ठा content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
काष्ठा nsurl *content_get_url(काष्ठा content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
काष्ठा content *content_clone(काष्ठा content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(काष्ठा content *c);

#पूर्ण_अगर
