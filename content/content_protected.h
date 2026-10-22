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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_CONTENT_PROTECTED_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_CONTENT_PROTECTED_H_

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/content_type.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h" /* mouse state enums */

ರಚನೆ nsurl;
ರಚನೆ content_redraw_ದತ್ತಾಂಶ;
ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ;
ರಚನೆ http_parameter;
ರಚನೆ llcache_handle;
ರಚನೆ object_params;
ರಚನೆ content;
ರಚನೆ redraw_context;
ರಚನೆ rect;
ರಚನೆ browser_ಕಿಟಕಿ;
ರಚನೆ browser_ಕಿಟಕಿ_features;
ರಚನೆ textsearch_context;
ರಚನೆ box;
ರಚನೆ selection;
ರಚನೆ selection_string;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
ರಚನೆ content_handler {
	ಶೂನ್ಯ (*fini)(ಶೂನ್ಯ);

	nserror (*create)(ಸ್ಥಿರಾಂಕ ರಚನೆ content_handler *handler,
                          lwc_string *imime_type,
                          ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
                          ರಚನೆ llcache_handle *llcache,
                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset, bool quirks,
                          ರಚನೆ content **c);

	bool (*process_ದತ್ತಾಂಶ)(ರಚನೆ content *c,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2);
	bool (*ದತ್ತಾಂಶ_complete)(ರಚನೆ content *c);
	ಶೂನ್ಯ (*reformat)(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ);
	ಶೂನ್ಯ (*destroy)(ರಚನೆ content *c);
	ಶೂನ್ಯ (*stop)(ರಚನೆ content *c);
	nserror (*mouse_track)(ರಚನೆ content *c, ರಚನೆ browser_ಕಿಟಕಿ *bw,
			browser_mouse_state mouse, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y);
	nserror (*mouse_action)(ರಚನೆ content *c, ರಚನೆ browser_ಕಿಟಕಿ *bw,
			browser_mouse_state mouse, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y);
	bool (*keypress)(ರಚನೆ content *c, uint32_t ಕೀಲಿ);
	bool (*redraw)(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
			ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
			ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);
	nserror (*open)(ರಚನೆ content *c, ರಚನೆ browser_ಕಿಟಕಿ *bw,
			ರಚನೆ content *ಪುಟ, ರಚನೆ object_params *params);
	nserror (*close)(ರಚನೆ content *c);
	ಶೂನ್ಯ (*clear_selection)(ರಚನೆ content *c);
	ಅಕ್ಷರ * (*get_selection)(ರಚನೆ content *c);
	nserror (*get_contextual_content)(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			ರಚನೆ browser_ಕಿಟಕಿ_features *ದತ್ತಾಂಶ);
	bool (*scroll_at_ಬಿಂದು)(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			ಪೂರ್ಣಾಂಕ scrx, ಪೂರ್ಣಾಂಕ scry);
	bool (*drop_file_at_ಬಿಂದು)(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			ಅಕ್ಷರ *file);
	nserror (*debug_dump)(ರಚನೆ content *c, FILE *f, ಎಣಿಕೆ content_debug op);
	nserror (*debug)(ರಚನೆ content *c, ಎಣಿಕೆ content_debug op);
	nserror (*clone)(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc);
	bool (*matches_quirks)(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, bool quirks);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*get_encoding)(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಎಣಿಕೆ content_encoding_type op);
	content_type (*type)(ಶೂನ್ಯ);
	ಶೂನ್ಯ (*ಕೂಡಿಸು_user)(ರಚನೆ content *c);
	ಶೂನ್ಯ (*remove_user)(ರಚನೆ content *c);
	bool (*exec)(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t srclen);
	bool (*saw_insecure_objects)(ರಚನೆ content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_ಕಂಡುಹಿಡಿ)(ರಚನೆ content *c, ರಚನೆ textsearch_context *context, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ, ಪೂರ್ಣಾಂಕ p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx, ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx, ರಚನೆ box *ಆರಂಭ_ptr, ರಚನೆ box *end_ptr, ರಚನೆ rect *bounds_ಹೊರಹೋಗು);

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
	nserror (*textselection_redraw)(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx, ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx, ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx, ರಚನೆ selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_ಕೊನೆ)(ರಚನೆ content *c, ಚಿಹ್ನೆರಹಿತ *ಕೊನೆ_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	ಶೂನ್ಯ *(*get_ಆಂತರಿಕ)(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(ರಚನೆ content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
ರಚನೆ content_user
{
	ಶೂನ್ಯ (*callback)(
			ರಚನೆ content *c,
			content_ಸಂದೇಶ ಸಂದೇಶ,
			ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
			ಶೂನ್ಯ *pw);
	ಶೂನ್ಯ *pw;

	ರಚನೆ content_user *ಮುಂದಿನ;
};

/**
 * Content which corresponds to a single URL.
 */
ರಚನೆ content {
	/**
	 * Low-level cache object
	 */
	ರಚನೆ llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	ಸ್ಥಿರಾಂಕ ರಚನೆ content_handler *handler;

	/**
	 * Current status.
	 */
	content_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;

	/**
	 * Width dimension, if applicable.
	 */
	ಪೂರ್ಣಾಂಕ ಅಗಲ;
	/**
	 * Height dimension, if applicable.
	 */
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ;
	/**
	 * Viewport width.
	 */
	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಅಗಲ;
	/**
	 * Viewport height.
	 */
	ಪೂರ್ಣಾಂಕ ಲಭ್ಯ_ಎತ್ತರ;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	ಅಕ್ಷರ *fallback_charset;

	/**
	 * URL for refresh request
	 */
	ರಚನೆ nsurl *refresh;

	/**
	 * list of metadata links
	 */
	ರಚನೆ content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t time;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_ಸಮಯ;

	/**
	 * Estimated size of all data associated with this content
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
	/**
	 * Title for browser window.
	 */
	ಅಕ್ಷರ *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸಕ್ರಿಯ;
	/**
	 * List of users.
	 */
	ರಚನೆ content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	ಅಕ್ಷರ ಸ್ಥಿತಿ_ಸಂದೇಶ[120];
	/**
	 * Status of content.
	 */
	ಅಕ್ಷರ sub_ಸ್ಥಿತಿ[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು_ಗಾತ್ರ;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	ದೀರ್ಘ http_ಸಂಕೇತ;

	/**
	 * Free text search state
	 */
	ರಚನೆ {
		ಅಕ್ಷರ *string;
		ರಚನೆ textsearch_context *context;
	} textsearch;
};

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ content_type_name[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ content_ಸ್ಥಿತಿ_name[];


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
nserror content__init(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ರಚನೆ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		ರಚನೆ llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ರಚನೆ content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
ಶೂನ್ಯ content_ಗಣ_ready(ರಚನೆ content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
ಶೂನ್ಯ content_ಗಣ_ಮುಗಿದ(ರಚನೆ content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
ಶೂನ್ಯ content_ಗಣ_error(ರಚನೆ content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
ಶೂನ್ಯ content_ಗಣ_ಸ್ಥಿತಿ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_ಸಂದೇಶ);

/**
 * Send a message to all users.
 */
ಶೂನ್ಯ content_ಪ್ರಸಾರ(ರಚನೆ content *c, content_ಸಂದೇಶ ಸಂದೇಶ, ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
ಶೂನ್ಯ content_ಪ್ರಸಾರ_error(ರಚನೆ content *c, nserror errorcode, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content_ಕೂಡಿಸು_rfc5988_link(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ರಚನೆ content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
ರಚನೆ content_rfc5988_link *content__free_rfc5988_link(ರಚನೆ content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
ಶೂನ್ಯ content__reformat(ರಚನೆ content *c, bool background, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
ಶೂನ್ಯ content__request_redraw(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(ರಚನೆ content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content_ಗಣ_title(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content__get_title(ರಚನೆ content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_get_ಸ್ಥಿತಿ_ಸಂದೇಶ_2(ರಚನೆ content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
ಪೂರ್ಣಾಂಕ content_get_ಅಗಲ_2(ರಚನೆ content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
ಪೂರ್ಣಾಂಕ content_get_ಎತ್ತರ_2(ರಚನೆ content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
ಪೂರ್ಣಾಂಕ content_get_ಲಭ್ಯ_ಅಗಲ_2(ರಚನೆ content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
ಸ್ಥಿರಾಂಕ uint8_t *content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(ರಚನೆ content *c, size_t *ಗಾತ್ರ_2);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
ಶೂನ್ಯ content_invalidate_reuse_ದತ್ತಾಂಶ_2(ರಚನೆ content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
ರಚನೆ nsurl *content__get_refresh_url(ರಚನೆ content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
ರಚನೆ bitmap *content__get_bitmap(ರಚನೆ content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(ರಚನೆ content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content__get_encoding(ರಚನೆ content *c, ಎಣಿಕೆ content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(ರಚನೆ content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
ಶೂನ್ಯ content_destroy(ರಚನೆ content *c);

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
bool content_ಕೂಡಿಸು_user(ರಚನೆ content *h,
		      ಶೂನ್ಯ (*callback)(
				       ರಚನೆ content *c,
				       content_ಸಂದೇಶ ಸಂದೇಶ,
				       ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
				       ಶೂನ್ಯ *pw),
		      ಶೂನ್ಯ *pw);

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
ಶೂನ್ಯ content_remove_user(ರಚನೆ content *c,
			 ಶೂನ್ಯ (*callback)(
					  ರಚನೆ content *c,
					  content_ಸಂದೇಶ ಸಂದೇಶ,
					  ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
					  ಶೂನ್ಯ *pw),
			 ಶೂನ್ಯ *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_ಎಣಿಕೆ_users(ರಚನೆ content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(ರಚನೆ content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(ರಚನೆ content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
ಸ್ಥಿರಾಂಕ ರಚನೆ llcache_handle *content_get_llcache_handle(ರಚನೆ content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
ರಚನೆ nsurl *content_get_url(ರಚನೆ content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
ರಚನೆ content *content_clone(ರಚನೆ content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(ರಚನೆ content *c);

#ಆದರೆ_ಅಂತ್ಯ
