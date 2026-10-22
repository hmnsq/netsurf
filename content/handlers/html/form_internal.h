/*
 * Copyright 2014 Vincent Sanders <vince@netsurf-browser.org>
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
 * Interface to form handling functions internal to HTML content handler.
 */

#నిర్వచితం_కాకపోతే NETSURF_HTML_FORM_అంతర్గత_H
#నిర్వచించు NETSURF_HTML_FORM_అంతర్గత_H

#అన్తర్భావ <stdbool.h>

#అన్తర్భావ "netsurf/form.h"

నిర్మాణం box;
నిర్మాణం form_control;
నిర్మాణం form_option;
నిర్మాణం form_select_menu;
నిర్మాణం form;
నిర్మాణం html_content;
నిర్మాణం dom_string;
నిర్మాణం content;
నిర్మాణం nsurl;
నిర్మాణం fetch_multipart_దత్తాంశం;
నిర్మాణం redraw_context;
నిర్మాణం browser_కిటికీ;

గణన browser_mouse_state;

/** Type of a struct form_control. */
రకనిర్వచనం గణన {
	GADGET_HIDDEN,
	GADGET_TEXTBOX,
	GADGET_RADIO,
	GADGET_CHECKBOX,
	GADGET_SELECT,
	GADGET_TEXTAREA,
	GADGET_IMAGE,
	GADGET_PASSWORD,
	GADGET_SUBMIT,
	GADGET_RESET,
	GADGET_FILE,
	GADGET_BUTTON
} form_control_type;

/** Data for textarea */
నిర్మాణం form_textarea_దత్తాంశం {
	నిర్మాణం form_control *gadget;
};

నిర్మాణం image_ఆదానం_coords {
	పూర్ణాంకం x;
	పూర్ణాంకం y;
};

/** Form control. */
నిర్మాణం form_control {
	శూన్య *కణుపు;			/**< Corresponding DOM node */
	నిర్మాణం dom_string *కణుపు_విలువ;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	నిర్మాణం html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	నిర్మాణం form *form;		/**< Containing form */

	అక్షరం *name;			/**< Control name */
	అక్షరం *విలువ;			/**< Current value of control */
	అక్షరం *initial_విలువ;		/**< Initial value of control */
	అక్షరం *చివరి_synced_విలువ;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	నిర్మాణం box *box;		/**< Box for control */

	గుర్తులేని పూర్ణాంకం నిడివి;		/**< Number of characters in control */
	గుర్తులేని పూర్ణాంకం maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	సంఘం {
		నిర్మాణం {
			పూర్ణాంకం mx, my;
		} image;
		నిర్మాణం {
			పూర్ణాంకం అంకె_వస్తువులు;
			నిర్మాణం form_option *వస్తువులు, *చివరి_అంశం;
			bool గుణిజం;
			పూర్ణాంకం అంకె_selected;
			/** Currently selected item, if num_selected == 1. */
			నిర్మాణం form_option *current;
			నిర్మాణం form_select_menu *menu;
		} select;
		నిర్మాణం {
			నిర్మాణం textarea *ta;
			నిర్మాణం dom_string *initial;
			నిర్మాణం form_textarea_దత్తాంశం దత్తాంశం;
		} పాఠ్యం;			/**< input type=text or textarea */
	} దత్తాంశం;

	నిర్మాణం form_control *prev;      /**< Previous control in this form */
	నిర్మాణం form_control *తదుపరి;	/**< Next control in this form. */
};

/** Form submit method. */
రకనిర్వచనం గణన {
	పద్ధతి_GET,		/**< GET, always url encoded. */
	పద్ధతి_POST_URLENC,	/**< POST, url encoded. */
	పద్ధతి_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_పద్ధతి;

/** HTML form. */
నిర్మాణం form {
	శూన్య *కణుపు;			/**< Corresponding DOM node */

	అక్షరం *action;			/**< Absolute URL to submit to. */
	అక్షరం *లక్ష్యం;			/**< Target to submit to. */
	form_పద్ధతి పద్ధతి;		/**< Method and enctype. */
	అక్షరం *accept_charsets;		/**< Charset to submit form in */
	అక్షరం *document_charset;		/**< Charset of document containing form */
	నిర్మాణం form_control *controls;	/**< Linked list of controls. */
	నిర్మాణం form_control *చివరి_control;	/**< Last control in list. */

	నిర్మాణం form *prev;		/**< Previous form in doc. */
};

/**
 * Called by the select menu when it wants an area to be redrawn. The
 * coordinates are menu origin relative.
 *
 * \param client_data	data which was passed to form_open_select_menu
 * \param x		X coordinate of redraw rectangle
 * \param y		Y coordinate of redraw rectangle
 * \param width		width of redraw rectangle
 * \param height	height of redraw rectangle
 */
రకనిర్వచనం శూన్య(*select_menu_redraw_callback)(శూన్య *client_దత్తాంశం,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);

/**
 * Create a struct form.
 *
 * \param node         DOM node associated with form
 * \param action       URL to submit form to, or NULL for default
 * \param target       Target frame of form, or NULL for default
 * \param method       method and enctype
 * \param charset      acceptable encodings for form submission, or NULL
 * \param doc_charset  encoding of containing document, or NULL
 * \return A new form or NULL on memory exhaustion
 */
నిర్మాణం form *form_new(శూన్య *కణుపు, స్థిరరాశి అక్షరం *action, స్థిరరాశి అక్షరం *లక్ష్యం,
		form_పద్ధతి పద్ధతి, స్థిరరాశి అక్షరం *charset,
		స్థిరరాశి అక్షరం *doc_charset);

/**
 * Free a form and any controls it owns.
 *
 * \note There may exist controls attached to box tree nodes which are not
 * associated with any form. These will leak at present. Ideally, they will
 * be cleaned up when the box tree is destroyed. As that currently happens
 * via talloc, this won't happen. These controls are distinguishable, as their
 * form field will be NULL.
 *
 * \param form The form to free
 */
శూన్య form_free(నిర్మాణం form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
నిర్మాణం form_control *form_new_control(శూన్య *కణుపు, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
శూన్య form_కూడు_control(నిర్మాణం form *form, నిర్మాణం form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
శూన్య form_free_control(నిర్మాణం form_control *control);


/**
 * Add an option to a form select control.
 *
 * \param  control   form control of type GADGET_SELECT
 * \param  value     value of option, used directly (not copied)
 * \param  text      text for option, used directly (not copied)
 * \param  selected  this option is selected
 * \param  node      the DOM node this option is associated with
 * \return  true on success, false on memory exhaustion
 */
bool form_కూడు_option(నిర్మాణం form_control *control, అక్షరం *విలువ, అక్షరం *పాఠ్యం,
		     bool selected, శూన్య *కణుపు);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(శూన్య *client_దత్తాంశం,
		నిర్మాణం form_control *control,
		select_menu_redraw_callback redraw_callback,
		నిర్మాణం content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
శూన్య form_free_select_menu(నిర్మాణం form_control *control);


/**
 * Redraw an opened select menu.
 *
 * \param control  the select menu being redrawn
 * \param x        the X coordinate to draw the menu at
 * \param y        the Y coordinate to draw the menu at
 * \param scale    current redraw scale
 * \param clip     clipping rectangle
 * \param ctx      current redraw context
 * \return         true on success, false otherwise
 */
bool form_redraw_select_menu(నిర్మాణం form_control *control, పూర్ణాంకం x, పూర్ణాంకం y,
		దశాంశం scale, స్థిరరాశి నిర్మాణం rect *clip,
		స్థిరరాశి నిర్మాణం redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(నిర్మాణం form_control *control, దశాంశం scale,
		స్థిరరాశి నిర్మాణం rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
స్థిరరాశి అక్షరం *form_select_mouse_action(నిర్మాణం form_control *control,
		గణన browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
శూన్య form_select_mouse_drag_చివర(నిర్మాణం form_control *control,
		గణన browser_mouse_state mouse, పూర్ణాంకం x, పూర్ణాంకం y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
శూన్య form_select_get_dimensions(నిర్మాణం form_control *control,
		పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు);


/**
 * Callback for the core select menu.
 */
శూన్య form_select_menu_callback(శూన్య *client_దత్తాంశం,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
శూన్య form_radio_సమితి(నిర్మాణం form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(నిర్మాణం nsurl *పుట_url, నిర్మాణం browser_కిటికీ *లక్ష్యం,
		నిర్మాణం form *form, నిర్మాణం form_control *submit_button);


/**
 * Update gadget value.
 */
శూన్య form_gadget_update_విలువ(నిర్మాణం form_control *control, అక్షరం *విలువ);


/**
 * Synchronise this gadget with its associated DOM node.
 *
 * If the DOM has changed and the gadget has not, the DOM's new value is
 * imported into the gadget.  If the gadget's value has changed and the DOM's
 * has not, the gadget's value is pushed into the DOM.
 * If both have changed, the gadget's value wins.
 *
 * \param control The form gadget to synchronise
 *
 * \note Currently this will only synchronise input gadgets (text/password)
 */
శూన్య form_gadget_sync_with_dom(నిర్మాణం form_control *control);

#అయితే_ముగింపు
