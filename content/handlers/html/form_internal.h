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

#ضمن <stdbool.h>

#ضمن "netsurf/form.h"

بنية box;
بنية form_control;
بنية form_option;
بنية form_select_menu;
بنية form;
بنية html_content;
بنية dom_string;
بنية content;
بنية nsurl;
بنية fetch_multipart_data;
بنية redraw_context;
بنية browser_window;

تعداد browser_mouse_state;

/** Type of a struct form_control. */
تعريفنوع تعداد {
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
بنية form_textarea_data {
	بنية form_control *gadget;
};

بنية image_داخلہ_coords {
	صحيح x;
	صحيح y;
};

/** Form control. */
بنية form_control {
	فراغ *گرہ;			/**< Corresponding DOM node */
	بنية dom_string *گرہ_قدر;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	بنية html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	بنية form *form;		/**< Containing form */

	حرف *name;			/**< Control name */
	حرف *قدر;			/**< Current value of control */
	حرف *initial_قدر;		/**< Initial value of control */
	حرف *last_synced_قدر;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	بنية box *box;		/**< Box for control */

	unsigned صحيح length;		/**< Number of characters in control */
	unsigned صحيح maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	اتحاد {
		بنية {
			صحيح mx, my;
		} image;
		بنية {
			صحيح عدد_اشیاء;
			بنية form_option *اشیاء, *last_item;
			bool multiple;
			صحيح عدد_selected;
			/** Currently selected item, if num_selected == 1. */
			بنية form_option *current;
			بنية form_select_menu *menu;
		} select;
		بنية {
			بنية textarea *ta;
			بنية dom_string *initial;
			بنية form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	بنية form_control *prev;      /**< Previous control in this form */
	بنية form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
تعريفنوع تعداد {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
بنية form {
	فراغ *گرہ;			/**< Corresponding DOM node */

	حرف *action;			/**< Absolute URL to submit to. */
	حرف *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	حرف *accept_charsets;		/**< Charset to submit form in */
	حرف *document_charset;		/**< Charset of document containing form */
	بنية form_control *controls;	/**< Linked list of controls. */
	بنية form_control *last_control;	/**< Last control in list. */

	بنية form *prev;		/**< Previous form in doc. */
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
تعريفنوع فراغ(*select_menu_redraw_callback)(فراغ *client_data,
		صحيح x, صحيح y, صحيح width, صحيح height);

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
بنية form *form_new(فراغ *گرہ, ثابت حرف *action, ثابت حرف *target,
		form_method method, ثابت حرف *charset,
		ثابت حرف *doc_charset);

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
فراغ form_free(بنية form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
بنية form_control *form_new_control(فراغ *گرہ, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
فراغ form_add_control(بنية form *form, بنية form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
فراغ form_free_control(بنية form_control *control);


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
bool form_add_option(بنية form_control *control, حرف *قدر, حرف *text,
		     bool selected, فراغ *گرہ);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(فراغ *client_data,
		بنية form_control *control,
		select_menu_redraw_callback redraw_callback,
		بنية content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
فراغ form_free_select_menu(بنية form_control *control);


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
bool form_redraw_select_menu(بنية form_control *control, صحيح x, صحيح y,
		عائم scale, ثابت بنية rect *clip,
		ثابت بنية redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(بنية form_control *control, عائم scale,
		ثابت بنية rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ثابت حرف *form_select_mouse_action(بنية form_control *control,
		تعداد browser_mouse_state mouse, صحيح x, صحيح y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
فراغ form_select_mouse_drag_end(بنية form_control *control,
		تعداد browser_mouse_state mouse, صحيح x, صحيح y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *width, صحيح *height);


/**
 * Callback for the core select menu.
 */
فراغ form_select_menu_callback(فراغ *client_data,
		صحيح x, صحيح y, صحيح width, صحيح height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
فراغ form_radio_set(بنية form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(بنية nsurl *page_url, بنية browser_window *target,
		بنية form *form, بنية form_control *submit_button);


/**
 * Update gadget value.
 */
فراغ form_gadget_update_قدر(بنية form_control *control, حرف *قدر);


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
فراغ form_gadget_sync_with_dom(بنية form_control *control);

#endif
