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

#اگر_غیر_تعریف_شدہ NETSURF_CONTENT_CONTENT_PROTECTED_H_
#تعریف_کرو NETSURF_CONTENT_CONTENT_PROTECTED_H_

#شامل <stdio.h>
#شامل <libwapcaplet/libwapcaplet.h>

#شامل "netsurf/content_type.h"
#شامل "netsurf/mouse.h" /* mouse state enums */

ساخت nsurl;
ساخت content_redraw_ڈیٹا;
اتحاد content_پیغام_ڈیٹا;
ساخت http_parameter;
ساخت llcache_handle;
ساخت object_params;
ساخت content;
ساخت redraw_context;
ساخت rect;
ساخت browser_کھڑکی;
ساخت browser_کھڑکی_features;
ساخت textsearch_context;
ساخت box;
ساخت selection;
ساخت selection_string;

قسمتعریف ساخت content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ساخت content_handler {
	خالی (*fini)(خالی);

	nserror (*create)(ثابت ساخت content_handler *handler,
                          lwc_string *imime_type,
                          ثابت ساخت http_parameter *params,
                          ساخت llcache_handle *llcache,
                          ثابت حرف *fallback_charset, bool quirks,
                          ساخت content **c);

	bool (*process_ڈیٹا)(ساخت content *c,
			ثابت حرف *ڈیٹا, غیرعلامتی صحیح سائز);
	bool (*ڈیٹا_complete)(ساخت content *c);
	خالی (*reformat)(ساخت content *c, صحیح چوڑائی, صحیح اونچائی);
	خالی (*destroy)(ساخت content *c);
	خالی (*stop)(ساخت content *c);
	nserror (*mouse_track)(ساخت content *c, ساخت browser_کھڑکی *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);
	nserror (*mouse_action)(ساخت content *c, ساخت browser_کھڑکی *bw,
			browser_mouse_state mouse, صحیح x, صحیح y);
	bool (*keypress)(ساخت content *c, uint32_t کلید);
	bool (*redraw)(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
			ثابت ساخت rect *clip,
			ثابت ساخت redraw_context *ctx);
	nserror (*open)(ساخت content *c, ساخت browser_کھڑکی *bw,
			ساخت content *صفحہ, ساخت object_params *params);
	nserror (*close)(ساخت content *c);
	خالی (*clear_selection)(ساخت content *c);
	حرف * (*get_selection)(ساخت content *c);
	nserror (*get_contextual_content)(ساخت content *c, صحیح x, صحیح y,
			ساخت browser_کھڑکی_features *ڈیٹا);
	bool (*scroll_at_نقطہ)(ساخت content *c, صحیح x, صحیح y,
			صحیح scrx, صحیح scry);
	bool (*drop_file_at_نقطہ)(ساخت content *c, صحیح x, صحیح y,
			حرف *file);
	nserror (*debug_dump)(ساخت content *c, FILE *f, شمار content_debug op);
	nserror (*debug)(ساخت content *c, شمار content_debug op);
	nserror (*clone)(ثابت ساخت content *old, ساخت content **newc);
	bool (*matches_quirks)(ثابت ساخت content *c, bool quirks);
	ثابت حرف *(*get_encoding)(ثابت ساخت content *c, شمار content_encoding_type op);
	content_type (*type)(خالی);
	خالی (*جوڑو_user)(ساخت content *c);
	خالی (*remove_user)(ساخت content *c);
	bool (*exec)(ساخت content *c, ثابت حرف *src, size_t srclen);
	bool (*saw_insecure_objects)(ساخت content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_ڈھونڈو)(ساخت content *c, ساخت textsearch_context *context, ثابت حرف *نمونہ, صحیح p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ساخت content *c, غیرعلامتی آغاز_idx, غیرعلامتی انتہا_idx, ساخت box *آغاز_ptr, ساخت box *end_ptr, ساخت rect *bounds_برآمد);

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
	nserror (*textselection_redraw)(ساخت content *c, غیرعلامتی آغاز_idx, غیرعلامتی انتہا_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ساخت content *c, غیرعلامتی آغاز_idx, غیرعلامتی انتہا_idx, ساخت selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_انتہا)(ساخت content *c, غیرعلامتی *انتہا_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	خالی *(*get_اندرونی)(ثابت ساخت content *c, خالی *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ساخت content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ساخت content_user
{
	خالی (*callback)(
			ساخت content *c,
			content_پیغام پیغام,
			ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا,
			خالی *pw);
	خالی *pw;

	ساخت content_user *اگلا;
};

/**
 * Content which corresponds to a single URL.
 */
ساخت content {
	/**
	 * Low-level cache object
	 */
	ساخت llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ثابت ساخت content_handler *handler;

	/**
	 * Current status.
	 */
	content_حالت حالت_2;

	/**
	 * Width dimension, if applicable.
	 */
	صحیح چوڑائی;
	/**
	 * Height dimension, if applicable.
	 */
	صحیح اونچائی;
	/**
	 * Viewport width.
	 */
	صحیح دستیاب_چوڑائی;
	/**
	 * Viewport height.
	 */
	صحیح دستیاب_اونچائی;

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
	ساخت nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ساخت content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t time;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_وقت;

	/**
	 * Estimated size of all data associated with this content
	 */
	غیرعلامتی صحیح سائز;
	/**
	 * Title for browser window.
	 */
	حرف *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	غیرعلامتی صحیح فعال;
	/**
	 * List of users.
	 */
	ساخت content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	حرف حالت_پیغام[120];
	/**
	 * Status of content.
	 */
	حرف sub_حالت[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	غیرعلامتی طویل کل_سائز;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	طویل http_رمز;

	/**
	 * Free text search state
	 */
	ساخت {
		حرف *string;
		ساخت textsearch_context *context;
	} textsearch;
};

خارجی ثابت حرف * ثابت content_type_name[];
خارجی ثابت حرف * ثابت content_حالت_name[];


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
nserror content__init(ساخت content *c, ثابت ساخت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		ساخت llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ثابت ساخت content *c, ساخت content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
خالی content_سیٹ_ready(ساخت content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
خالی content_سیٹ_مکمل(ساخت content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
خالی content_سیٹ_error(ساخت content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
خالی content_سیٹ_حالت(ساخت content *c, ثابت حرف *حالت_پیغام);

/**
 * Send a message to all users.
 */
خالی content_نشر(ساخت content *c, content_پیغام پیغام, ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
خالی content_نشر_error(ساخت content *c, nserror errorcode, ثابت حرف *پیغام);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content_جوڑو_rfc5988_link(ساخت content *c, ثابت ساخت content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ساخت content_rfc5988_link *content__free_rfc5988_link(ساخت content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
خالی content__reformat(ساخت content *c, bool background, صحیح چوڑائی, صحیح اونچائی);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
خالی content__request_redraw(ساخت content *c, صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ساخت content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content_سیٹ_title(ساخت content *c, ثابت حرف *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ثابت حرف *content__get_title(ساخت content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ثابت حرف *content_get_حالت_پیغام_2(ساخت content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
صحیح content_get_چوڑائی_2(ساخت content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
صحیح content_get_اونچائی_2(ساخت content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
صحیح content_get_دستیاب_چوڑائی_2(ساخت content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ثابت uint8_t *content_get_ماخذ_ڈیٹا_2(ساخت content *c, size_t *سائز);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
خالی content_invalidate_reuse_ڈیٹا_2(ساخت content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ساخت nsurl *content__get_refresh_url(ساخت content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ساخت bitmap *content__get_bitmap(ساخت content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ساخت content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ثابت حرف *content__get_encoding(ساخت content *c, شمار content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ساخت content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
خالی content_destroy(ساخت content *c);

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
bool content_جوڑو_user(ساخت content *h,
		      خالی (*callback)(
				       ساخت content *c,
				       content_پیغام پیغام,
				       ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا,
				       خالی *pw),
		      خالی *pw);

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
خالی content_remove_user(ساخت content *c,
			 خالی (*callback)(
					  ساخت content *c,
					  content_پیغام پیغام,
					  ثابت اتحاد content_پیغام_ڈیٹا *ڈیٹا,
					  خالی *pw),
			 خالی *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_گنتی_users(ساخت content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ساخت content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ساخت content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ثابت ساخت llcache_handle *content_get_llcache_handle(ساخت content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ساخت nsurl *content_get_url(ساخت content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ساخت content *content_clone(ساخت content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ساخت content *c);

#اگر_ختم
