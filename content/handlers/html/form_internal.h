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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>

#ଅନ୍ତର୍ଭାଵ "netsurf/form.h"

ସଂରଚନା box;
ସଂରଚନା form_control;
ସଂରଚନା form_option;
ସଂରଚନା form_select_menu;
ସଂରଚନା form;
ସଂରଚନା html_content;
ସଂରଚନା dom_string;
ସଂରଚନା content;
ସଂରଚନା nsurl;
ସଂରଚନା fetch_multipart_data;
ସଂରଚନା redraw_context;
ସଂରଚନା browser_window;

ଗଣନା browser_mouse_state;

/** Type of a struct form_control. */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
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
ସଂରଚନା form_textarea_data {
	ସଂରଚନା form_control *gadget;
};

ସଂରଚନା image_ନିବେଶ_coords {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y;
};

/** Form control. */
ସଂରଚନା form_control {
	ଶୂନ୍ୟ *ଗଣ୍ଠି;			/**< Corresponding DOM node */
	ସଂରଚନା dom_string *ଗଣ୍ଠି_ମୂଲ୍ୟ;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	ସଂରଚନା html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	ସଂରଚନା form *form;		/**< Containing form */

	ଅକ୍ଷର *name;			/**< Control name */
	ଅକ୍ଷର *ମୂଲ୍ୟ;			/**< Current value of control */
	ଅକ୍ଷର *initial_ମୂଲ୍ୟ;		/**< Initial value of control */
	ଅକ୍ଷର *last_synced_ମୂଲ୍ୟ;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	ସଂରଚନା box *box;		/**< Box for control */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;		/**< Number of characters in control */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	ସଂଘ {
		ସଂରଚନା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା mx, my;
		} image;
		ସଂରଚନା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ;
			ସଂରଚନା form_option *ବସ୍ତୁଗୁଡ଼ିକ, *last_item;
			bool multiple;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_selected;
			/** Currently selected item, if num_selected == 1. */
			ସଂରଚନା form_option *current;
			ସଂରଚନା form_select_menu *menu;
		} select;
		ସଂରଚନା {
			ସଂରଚନା textarea *ta;
			ସଂରଚନା dom_string *initial;
			ସଂରଚନା form_textarea_data data;
		} text;			/**< input type=text or textarea */
	} data;

	ସଂରଚନା form_control *prev;      /**< Previous control in this form */
	ସଂରଚନା form_control *next;	/**< Next control in this form. */
};

/** Form submit method. */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
ସଂରଚନା form {
	ଶୂନ୍ୟ *ଗଣ୍ଠି;			/**< Corresponding DOM node */

	ଅକ୍ଷର *action;			/**< Absolute URL to submit to. */
	ଅକ୍ଷର *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	ଅକ୍ଷର *accept_charsets;		/**< Charset to submit form in */
	ଅକ୍ଷର *document_charset;		/**< Charset of document containing form */
	ସଂରଚନା form_control *controls;	/**< Linked list of controls. */
	ସଂରଚନା form_control *last_control;	/**< Last control in list. */

	ସଂରଚନା form *prev;		/**< Previous form in doc. */
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ(*select_menu_redraw_callback)(ଶୂନ୍ୟ *client_data,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height);

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
ସଂରଚନା form *form_new(ଶୂନ୍ୟ *ଗଣ୍ଠି, ଅଚଳ ଅକ୍ଷର *action, ଅଚଳ ଅକ୍ଷର *target,
		form_method method, ଅଚଳ ଅକ୍ଷର *charset,
		ଅଚଳ ଅକ୍ଷର *doc_charset);

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
ଶୂନ୍ୟ form_free(ସଂରଚନା form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
ସଂରଚନା form_control *form_new_control(ଶୂନ୍ୟ *ଗଣ୍ଠି, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
ଶୂନ୍ୟ form_add_control(ସଂରଚନା form *form, ସଂରଚନା form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
ଶୂନ୍ୟ form_free_control(ସଂରଚନା form_control *control);


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
bool form_add_option(ସଂରଚନା form_control *control, ଅକ୍ଷର *ମୂଲ୍ୟ, ଅକ୍ଷର *text,
		     bool selected, ଶୂନ୍ୟ *ଗଣ୍ଠି);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(ଶୂନ୍ୟ *client_data,
		ସଂରଚନା form_control *control,
		select_menu_redraw_callback redraw_callback,
		ସଂରଚନା content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
ଶୂନ୍ୟ form_free_select_menu(ସଂରଚନା form_control *control);


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
bool form_redraw_select_menu(ସଂରଚନା form_control *control, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		ଦଶମିକ scale, ଅଚଳ ସଂରଚନା rect *clip,
		ଅଚଳ ସଂରଚନା redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(ସଂରଚନା form_control *control, ଦଶମିକ scale,
		ଅଚଳ ସଂରଚନା rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
ଅଚଳ ଅକ୍ଷର *form_select_mouse_action(ସଂରଚନା form_control *control,
		ଗଣନା browser_mouse_state mouse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
ଶୂନ୍ୟ form_select_mouse_drag_end(ସଂରଚନା form_control *control,
		ଗଣନା browser_mouse_state mouse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
ଶୂନ୍ୟ form_select_get_dimensions(ସଂରଚନା form_control *control,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height);


/**
 * Callback for the core select menu.
 */
ଶୂନ୍ୟ form_select_menu_callback(ଶୂନ୍ୟ *client_data,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
ଶୂନ୍ୟ form_radio_set(ସଂରଚନା form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(ସଂରଚନା nsurl *page_url, ସଂରଚନା browser_window *target,
		ସଂରଚନା form *form, ସଂରଚନା form_control *submit_button);


/**
 * Update gadget value.
 */
ଶୂନ୍ୟ form_gadget_update_ମୂଲ୍ୟ(ସଂରଚନା form_control *control, ଅକ୍ଷର *ମୂଲ୍ୟ);


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
ଶୂନ୍ୟ form_gadget_sync_with_dom(ସଂରଚନା form_control *control);

#endif
