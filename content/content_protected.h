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

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_CONTENT_PROTECTED_H_
#నిర్వచించు NETSURF_CONTENT_CONTENT_PROTECTED_H_

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>

#అన్తర్భావ "netsurf/content_type.h"
#అన్తర్భావ "netsurf/mouse.h" /* mouse state enums */

నిర్మాణం nsurl;
నిర్మాణం content_redraw_దత్తాంశం;
సంఘం content_సందేశం_దత్తాంశం;
నిర్మాణం http_parameter;
నిర్మాణం llcache_handle;
నిర్మాణం object_params;
నిర్మాణం content;
నిర్మాణం redraw_context;
నిర్మాణం rect;
నిర్మాణం browser_కిటికీ;
నిర్మాణం browser_కిటికీ_features;
నిర్మాణం textsearch_context;
నిర్మాణం box;
నిర్మాణం selection;
నిర్మాణం selection_string;

రకనిర్వచనం నిర్మాణం content_handler content_handler;

/**
 * Content operation function table
 *
 * function table implementing a content type.
 */
నిర్మాణం content_handler {
	శూన్య (*fini)(శూన్య);

	nserror (*create)(స్థిరరాశి నిర్మాణం content_handler *handler,
                          lwc_string *imime_type,
                          స్థిరరాశి నిర్మాణం http_parameter *params,
                          నిర్మాణం llcache_handle *llcache,
                          స్థిరరాశి అక్షరం *fallback_charset, bool quirks,
                          నిర్మాణం content **c);

	bool (*process_దత్తాంశం)(నిర్మాణం content *c,
			స్థిరరాశి అక్షరం *దత్తాంశం, గుర్తులేని పూర్ణాంకం కొలత);
	bool (*దత్తాంశం_complete)(నిర్మాణం content *c);
	శూన్య (*reformat)(నిర్మాణం content *c, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);
	శూన్య (*destroy)(నిర్మాణం content *c);
	శూన్య (*stop)(నిర్మాణం content *c);
	nserror (*mouse_track)(నిర్మాణం content *c, నిర్మాణం browser_కిటికీ *bw,
			browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);
	nserror (*mouse_action)(నిర్మాణం content *c, నిర్మాణం browser_కిటికీ *bw,
			browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);
	bool (*keypress)(నిర్మాణం content *c, uint32_t కీలకం);
	bool (*redraw)(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
			స్థిరరాశి నిర్మాణం rect *clip,
			స్థిరరాశి నిర్మాణం redraw_context *ctx);
	nserror (*open)(నిర్మాణం content *c, నిర్మాణం browser_కిటికీ *bw,
			నిర్మాణం content *పుట, నిర్మాణం object_params *params);
	nserror (*close)(నిర్మాణం content *c);
	శూన్య (*clear_selection)(నిర్మాణం content *c);
	అక్షరం * (*get_selection)(నిర్మాణం content *c);
	nserror (*get_contextual_content)(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y,
			నిర్మాణం browser_కిటికీ_features *దత్తాంశం);
	bool (*scroll_at_బిందువు)(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y,
			పూర్ణాంకం scrx, పూర్ణాంకం scry);
	bool (*drop_file_at_బిందువు)(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y,
			అక్షరం *file);
	nserror (*debug_dump)(నిర్మాణం content *c, FILE *f, గణన content_debug op);
	nserror (*debug)(నిర్మాణం content *c, గణన content_debug op);
	nserror (*clone)(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc);
	bool (*matches_quirks)(స్థిరరాశి నిర్మాణం content *c, bool quirks);
	స్థిరరాశి అక్షరం *(*get_encoding)(స్థిరరాశి నిర్మాణం content *c, గణన content_encoding_type op);
	content_type (*type)(శూన్య);
	శూన్య (*కూడు_user)(నిర్మాణం content *c);
	శూన్య (*remove_user)(నిర్మాణం content *c);
	bool (*exec)(నిర్మాణం content *c, స్థిరరాశి అక్షరం *src, size_t srclen);
	bool (*saw_insecure_objects)(నిర్మాణం content *c);

	/**
	 * content specific free text search find
	 */
	nserror (*textsearch_కనుగొను)(నిర్మాణం content *c, నిర్మాణం textsearch_context *context, స్థిరరాశి అక్షరం *నమూనా, పూర్ణాంకం p_len, bool case_sens);

	/**
	 * get bounds of free text search match
	 */
	nserror (*textsearch_bounds)(నిర్మాణం content *c, గుర్తులేని ప్రారంభం_idx, గుర్తులేని చివర_idx, నిర్మాణం box *ప్రారంభం_ptr, నిర్మాణం box *end_ptr, నిర్మాణం rect *bounds_బహిర్గతం);

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
	nserror (*textselection_redraw)(నిర్మాణం content *c, గుర్తులేని ప్రారంభం_idx, గుర్తులేని చివర_idx);

	/**
	 * copy selected text into selection string possibly with formatting
	 */
	nserror (*textselection_copy)(నిర్మాణం content *c, గుర్తులేని ప్రారంభం_idx, గుర్తులేని చివర_idx, నిర్మాణం selection_string *selstr);

	/**
	 * get maximum index of text section.
	 *
	 * \param[in] c The content to measure
	 * \param[out] end_idx pointer to value to recive result
	 * \return NSERROR_OK and \a end_idx updated else error code
	 */
	nserror (*textselection_get_చివర)(నిర్మాణం content *c, గుర్తులేని *చివర_idx);

        /**
	 * handler dependant content sensitive internal data interface.
	 */
	శూన్య *(*get_అంతర్గత)(స్థిరరాశి నిర్మాణం content *c, శూన్య *context);

	/**
	 * are the content contents opaque.
	 *
	 * Determine if this content would obscure (not mix with) any background
	 *
	 * \param c The content to check
	 */
	bool (*is_opaque)(నిర్మాణం content *c);

	/**
	 * There must be one content per user for this type.
	 */
	bool no_share;
};

/**
 * Linked list of users of a content.
 */
నిర్మాణం content_user
{
	శూన్య (*callback)(
			నిర్మాణం content *c,
			content_సందేశం సందేశం,
			స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం,
			శూన్య *pw);
	శూన్య *pw;

	నిర్మాణం content_user *తదుపరి;
};

/**
 * Content which corresponds to a single URL.
 */
నిర్మాణం content {
	/**
	 * Low-level cache object
	 */
	నిర్మాణం llcache_handle *llcache;

	/**
	 * Original MIME type of data
	 */
	lwc_string *mime_type;

	/**
	 * Handler for content
	 */
	స్థిరరాశి నిర్మాణం content_handler *handler;

	/**
	 * Current status.
	 */
	content_స్థితి స్థితి_2;

	/**
	 * Width dimension, if applicable.
	 */
	పూర్ణాంకం వెడల్పు;
	/**
	 * Height dimension, if applicable.
	 */
	పూర్ణాంకం ఎత్తు;
	/**
	 * Viewport width.
	 */
	పూర్ణాంకం అందుబాటు_వెడల్పు;
	/**
	 * Viewport height.
	 */
	పూర్ణాంకం అందుబాటు_ఎత్తు;

	/**
	 * Content is in quirks mode
	 */
	bool quirks;
	/**
	 * Fallback charset, or NULL
	 */
	అక్షరం *fallback_charset;

	/**
	 * URL for refresh request
	 */
	నిర్మాణం nsurl *refresh;

	/**
	 * list of metadata links
	 */
	నిర్మాణం content_rfc5988_link *links;

	/**
	 * Creation timestamp when LOADING or READY.  Total time in ms
	 * when DONE.
	 */
	uint64_t time;

	/**
	 * Earliest time to attempt a period reflow while fetching a
	 * page's objects.
	 */
	uint64_t reformat_సమయం;

	/**
	 * Estimated size of all data associated with this content
	 */
	గుర్తులేని పూర్ణాంకం కొలత;
	/**
	 * Title for browser window.
	 */
	అక్షరం *title;
	/**
	 * Number of child fetches or conversions currently in progress.
	 */
	గుర్తులేని పూర్ణాంకం చురుకైన;
	/**
	 * List of users.
	 */
	నిర్మాణం content_user *user_list;
	/**
	 * Full text for status bar.
	 */
	అక్షరం స్థితి_సందేశం[120];
	/**
	 * Status of content.
	 */
	అక్షరం sub_స్థితి[80];
	/**
	 * Content is being processed: data structures may be
	 * inconsistent and content must not be redrawn or modified.
	 */
	bool locked;

	/**
	 * Total data size, 0 if unknown.
	 */
	గుర్తులేని దీర్ఘ మొత్తం_కొలత;
	/**
	 * HTTP status code, 0 if not HTTP.
	 */
	దీర్ఘ http_సంకేతం;

	/**
	 * Free text search state
	 */
	నిర్మాణం {
		అక్షరం *string;
		నిర్మాణం textsearch_context *context;
	} textsearch;
};

బాహ్య స్థిరరాశి అక్షరం * స్థిరరాశి content_type_name[];
బాహ్య స్థిరరాశి అక్షరం * స్థిరరాశి content_స్థితి_name[];


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
nserror content__init(నిర్మాణం content *c, స్థిరరాశి నిర్మాణం content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		నిర్మాణం llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks);

/**
 * Clone a content's data members
 *
 * \param c   Content to clone
 * \param nc  Content to populate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror content__clone(స్థిరరాశి నిర్మాణం content *c, నిర్మాణం content *nc);

/**
 * Put a content in status CONTENT_STATUS_READY and unlock the content.
 */
శూన్య content_సమితి_ready(నిర్మాణం content *c);

/**
 * Put a content in status CONTENT_STATUS_DONE.
 */
శూన్య content_సమితి_పూర్తి(నిర్మాణం content *c);

/**
 * Put a content in status CONTENT_STATUS_ERROR and unlock the content.
 *
 * \note We expect the caller to broadcast an error report if needed.
 */
శూన్య content_సమితి_error(నిర్మాణం content *c);

/**
 * Updates content with new status.
 *
 * The textual status contained in the content is updated with given string.
 *
 * \param c The content to set status in.
 * \param status_message new textual status
 */
శూన్య content_సమితి_స్థితి(నిర్మాణం content *c, స్థిరరాశి అక్షరం *స్థితి_సందేశం);

/**
 * Send a message to all users.
 */
శూన్య content_ప్రసారం(నిర్మాణం content *c, content_సందేశం సందేశం, స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం);

/**
 * Send an error message to all users.
 *
 * \param c The content whose users should be informed of an error
 * \param errorcode The nserror code to send
 * \param msg The error message to send alongside
 */
శూన్య content_ప్రసారం_error(నిర్మాణం content *c, nserror errorcode, స్థిరరాశి అక్షరం *సందేశం);

/**
 * associate a metadata link with a content.
 *
 * \param c content to add link to
 * \param link The rfc5988 link to add
 */
bool content_కూడు_rfc5988_link(నిర్మాణం content *c, స్థిరరాశి నిర్మాణం content_rfc5988_link *link);

/**
 * free a rfc5988 link
 *
 * \param link The link to free
 * \return The next link in the chain
 */
నిర్మాణం content_rfc5988_link *content__free_rfc5988_link(నిర్మాణం content_rfc5988_link *link);

/**
 * cause a content to be reformatted.
 *
 * \param c content to be reformatted
 * \param background perform reformat in background
 * \param width The available width to reformat content in
 * \param height The available height to reformat content in
 */
శూన్య content__reformat(నిర్మాణం content *c, bool background, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);

/**
 * Request a redraw of an area of a content
 *
 * \param c	  Content
 * \param x	  x co-ord of left edge
 * \param y	  y co-ord of top edge
 * \param width	  Width of rectangle
 * \param height  Height of rectangle
 */
శూన్య content__request_redraw(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);

/**
 * Retrieve mime-type of content
 *
 * \param c Content to retrieve mime-type of
 * \return Pointer to referenced mime-type, or NULL if not found.
 */
lwc_string *content__get_mime_type(నిర్మాణం content *c);

/**
 * Set title associated with content
 *
 * \param c Content to set title on.
 * \param title The new title to set.
 * \return true on sucess else false.
 */
bool content_సమితి_title(నిర్మాణం content *c, స్థిరరాశి అక్షరం *title);

/**
 * Retrieve title associated with content
 *
 * \param c Content to retrieve title from
 * \return Pointer to title, or NULL if not found.
 */
స్థిరరాశి అక్షరం *content__get_title(నిర్మాణం content *c);

/**
 * Retrieve status message associated with content
 *
 * \param c Content to retrieve status message from
 * \return Pointer to status message, or NULL if not found.
 */
స్థిరరాశి అక్షరం *content_get_స్థితి_సందేశం_2(నిర్మాణం content *c);

/**
 * Retrieve width of content
 *
 * \param c Content to retrieve width of
 * \return Content width
 */
పూర్ణాంకం content_get_వెడల్పు_2(నిర్మాణం content *c);

/**
 * Retrieve height of content
 *
 * \param c Content to retrieve height of
 * \return Content height
 */
పూర్ణాంకం content_get_ఎత్తు_2(నిర్మాణం content *c);

/**
 * Retrieve available width of content
 *
 * \param c content to get available width of.
 * \return Available width of content.
 */
పూర్ణాంకం content_get_అందుబాటు_వెడల్పు_2(నిర్మాణం content *c);

/**
 * Retrieve source of content.
 *
 * \param c    Content to retrieve source of.
 * \param size Pointer to location to receive byte size of source.
 * \return Pointer to source data.
 */
స్థిరరాశి uint8_t *content_get_ఆధారం_దత్తాంశం_2(నిర్మాణం content *c, size_t *కొలత);

/**
 * Invalidate content reuse data.
 *
 * causes subsequent requests for content URL to query server to
 * determine if content can be reused. This is required behaviour for
 * forced reloads etc.
 *
 * \param c Content to invalidate.
 */
శూన్య content_invalidate_reuse_దత్తాంశం_2(నిర్మాణం content *c);

/**
 * Retrieve the refresh URL for a content
 *
 * \param c Content to retrieve refresh URL from
 * \return Pointer to URL or NULL if none
 */
నిర్మాణం nsurl *content__get_refresh_url(నిర్మాణం content *c);

/**
 * Retrieve the bitmap contained in an image content
 *
 * \param c Content to retrieve opacity from
 * \return Pointer to bitmap or NULL if none.
 */
నిర్మాణం bitmap *content__get_bitmap(నిర్మాణం content *c);

/**
 * Determine if a content is opaque
 *
 * \param c Content to retrieve opacity from
 * \return false if the content is not opaque or information is not
 *         known else true.
 */
bool content__get_opaque(నిర్మాణం content *c);

/**
 * Retrieve the encoding of a content
 *
 * \param c the content to examine the encoding of.
 * \param op encoding operation.
 * \return Pointer to content info or NULL if none.
 */
స్థిరరాశి అక్షరం *content__get_encoding(నిర్మాణం content *c, గణన content_encoding_type op);

/**
 * Return whether a content is currently locked
 *
 * \param c Content to test
 * \return true iff locked, else false
 */
bool content__is_locked(నిర్మాణం content *c);

/**
 * Destroy and free a content.
 *
 * Calls the destroy function for the content, and frees the structure.
 */
శూన్య content_destroy(నిర్మాణం content *c);

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
bool content_కూడు_user(నిర్మాణం content *h,
		      శూన్య (*callback)(
				       నిర్మాణం content *c,
				       content_సందేశం సందేశం,
				       స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం,
				       శూన్య *pw),
		      శూన్య *pw);

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
శూన్య content_remove_user(నిర్మాణం content *c,
			 శూన్య (*callback)(
					  నిర్మాణం content *c,
					  content_సందేశం సందేశం,
					  స్థిరరాశి సంఘం content_సందేశం_దత్తాంశం *దత్తాంశం,
					  శూన్య *pw),
			 శూన్య *ctx);


/**
 * Count users for the content.
 *
 * \param c Content to consider
 */
uint32_t content_లెక్క_users(నిర్మాణం content *c);


/**
 * Determine if quirks mode matches
 *
 * \param c Content to consider
 * \param quirks  Quirks mode to match
 * \return True if quirks match, false otherwise
 */
bool content_matches_quirks(నిర్మాణం content *c, bool quirks);

/**
 * Determine if a content is shareable
 *
 * \param c  Content to consider
 * \return True if content is shareable, false otherwise
 */
bool content_is_shareable(నిర్మాణం content *c);

/**
 * Retrieve the low-level cache handle for a content
 *
 * \note only used by hlcache
 *
 * \param c Content to retrieve from
 * \return Low-level cache handle
 */
స్థిరరాశి నిర్మాణం llcache_handle *content_get_llcache_handle(నిర్మాణం content *c);

/**
 * Retrieve URL associated with content
 *
 * \param c  Content to retrieve URL from
 * \return Pointer to URL, or NULL if not found.
 */
నిర్మాణం nsurl *content_get_url(నిర్మాణం content *c);

/**
 * Clone a content object in its current state.
 *
 * \param c  Content to clone
 * \return Clone of \a c
 */
నిర్మాణం content *content_clone(నిర్మాణం content *c);

/**
 * Abort a content object
 *
 * \param c The content object to abort
 * \return NSERROR_OK on success, otherwise appropriate error
 */
nserror content_abort(నిర్మాణం content *c);

#అయితే_ముగింపు
