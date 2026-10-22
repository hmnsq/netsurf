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

#ifndef NETSURF_HTML_FORM_INTERNAL_H
#define NETSURF_HTML_FORM_INTERNAL_H

#অন্তর্ভাব <stdbool.h>

#অন্তর্ভাব "netsurf/form.h"

কাঠামো box;
কাঠামো form_control;
কাঠামো form_option;
কাঠামো form_select_menu;
কাঠামো form;
কাঠামো html_content;
কাঠামো dom_string;
কাঠামো content;
কাঠামো nsurl;
কাঠামো fetch_multipart_তথ্য;
কাঠামো redraw_context;
কাঠামো browser_window;

গণনা browser_mouse_state;

/** Type of a struct form_control. */
প্রকারলেখ গণনা {
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
কাঠামো form_textarea_তথ্য {
	কাঠামো form_control *gadget;
};

কাঠামো image_প্রবেশ_coords {
	পূর্ণসংখ্যা x;
	পূর্ণসংখ্যা y;
};

/** Form control. */
কাঠামো form_control {
	শূন্য *গ্রন্থি;			/**< Corresponding DOM node */
	কাঠামো dom_string *গ্রন্থি_মান;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	কাঠামো html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	কাঠামো form *form;		/**< Containing form */

	অক্ষর *name;			/**< Control name */
	অক্ষর *মান;			/**< Current value of control */
	অক্ষর *initial_মান;		/**< Initial value of control */
	অক্ষর *last_synced_মান;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	কাঠামো box *box;		/**< Box for control */

	unsigned পূর্ণসংখ্যা দৈর্ঘ্য;		/**< Number of characters in control */
	unsigned পূর্ণসংখ্যা maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	সংঘ {
		কাঠামো {
			পূর্ণসংখ্যা mx, my;
		} image;
		কাঠামো {
			পূর্ণসংখ্যা অঙ্ক_বস্তুসমূহ;
			কাঠামো form_option *বস্তুসমূহ, *last_item;
			bool multiple;
			পূর্ণসংখ্যা অঙ্ক_selected;
			/** Currently selected item, if num_selected == 1. */
			কাঠামো form_option *current;
			কাঠামো form_select_menu *menu;
		} select;
		কাঠামো {
			কাঠামো textarea *ta;
			কাঠামো dom_string *initial;
			কাঠামো form_textarea_তথ্য তথ্য;
		} text;			/**< input type=text or textarea */
	} তথ্য;

	কাঠামো form_control *পূর্ব;      /**< Previous control in this form */
	কাঠামো form_control *পরবর্তী;	/**< Next control in this form. */
};

/** Form submit method. */
প্রকারলেখ গণনা {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
কাঠামো form {
	শূন্য *গ্রন্থি;			/**< Corresponding DOM node */

	অক্ষর *action;			/**< Absolute URL to submit to. */
	অক্ষর *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	অক্ষর *accept_charsets;		/**< Charset to submit form in */
	অক্ষর *document_charset;		/**< Charset of document containing form */
	কাঠামো form_control *controls;	/**< Linked list of controls. */
	কাঠামো form_control *last_control;	/**< Last control in list. */

	কাঠামো form *পূর্ব;		/**< Previous form in doc. */
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
প্রকারলেখ শূন্য(*select_menu_redraw_callback)(শূন্য *client_তথ্য,
		পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height);

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
কাঠামো form *form_new(শূন্য *গ্রন্থি, ধ্রুবক অক্ষর *action, ধ্রুবক অক্ষর *target,
		form_method method, ধ্রুবক অক্ষর *charset,
		ধ্রুবক অক্ষর *doc_charset);

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
শূন্য form_free(কাঠামো form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
কাঠামো form_control *form_new_control(শূন্য *গ্রন্থি, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
শূন্য form_add_control(কাঠামো form *form, কাঠামো form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
শূন্য form_free_control(কাঠামো form_control *control);


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
bool form_add_option(কাঠামো form_control *control, অক্ষর *মান, অক্ষর *text,
		     bool selected, শূন্য *গ্রন্থি);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(শূন্য *client_তথ্য,
		কাঠামো form_control *control,
		select_menu_redraw_callback redraw_callback,
		কাঠামো content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
শূন্য form_free_select_menu(কাঠামো form_control *control);


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
bool form_redraw_select_menu(কাঠামো form_control *control, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		দশমিক scale, ধ্রুবক কাঠামো rect *clip,
		ধ্রুবক কাঠামো redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(কাঠামো form_control *control, দশমিক scale,
		ধ্রুবক কাঠামো rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ধ্রুবক অক্ষর *form_select_mouse_action(কাঠামো form_control *control,
		গণনা browser_mouse_state mouse, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
শূন্য form_select_mouse_drag_end(কাঠামো form_control *control,
		গণনা browser_mouse_state mouse, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
শূন্য form_select_get_dimensions(কাঠামো form_control *control,
		পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height);


/**
 * Callback for the core select menu.
 */
শূন্য form_select_menu_callback(শূন্য *client_তথ্য,
		পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
শূন্য form_radio_set(কাঠামো form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(কাঠামো nsurl *page_url, কাঠামো browser_window *target,
		কাঠামো form *form, কাঠামো form_control *submit_button);


/**
 * Update gadget value.
 */
শূন্য form_gadget_update_মান(কাঠামো form_control *control, অক্ষর *মান);


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
শূন্য form_gadget_sync_with_dom(কাঠামো form_control *control);

#endif
