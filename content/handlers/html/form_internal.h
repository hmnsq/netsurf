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

#अगर_अघोषित NETSURF_HTML_FORM_INTERNAL_H
#घोषणा NETSURF_HTML_FORM_INTERNAL_H

#समावेश <stdbool.h>

#समावेश "netsurf/form.h"

काष्ठा box;
काष्ठा form_control;
काष्ठा form_option;
काष्ठा form_select_menu;
काष्ठा form;
काष्ठा html_content;
काष्ठा dom_string;
काष्ठा content;
काष्ठा nsurl;
काष्ठा fetch_multipart_तथ्य;
काष्ठा redraw_context;
काष्ठा browser_window;

क्रमागत browser_mouse_state;

/** Type of a struct form_control. */
प्रकार क्रमागत {
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
काष्ठा form_textarea_तथ्य {
	काष्ठा form_control *gadget;
};

काष्ठा image_निवेश_coords {
	पूर्णांक x;
	पूर्णांक y;
};

/** Form control. */
काष्ठा form_control {
	व्योम *गाँठ;			/**< Corresponding DOM node */
	काष्ठा dom_string *गाँठ_मूल्य;  /**< The last value sync'd with the DOM */
	bool syncing;                   /**< Set if a DOM sync is in-progress */
	काष्ठा html_content *html;	/**< HTML content containing control */

	form_control_type type;		/**< Type of control */

	काष्ठा form *form;		/**< Containing form */

	अक्षर *name;			/**< Control name */
	अक्षर *मूल्य;			/**< Current value of control */
	अक्षर *initial_मूल्य;		/**< Initial value of control */
	अक्षर *last_synced_मूल्य;        /**< The last value sync'd to the DOM */
	bool disabled;			/**< Whether control is disabled */

	काष्ठा box *box;		/**< Box for control */

	अचिन्हित पूर्णांक लंबाई;		/**< Number of characters in control */
	अचिन्हित पूर्णांक maxlength;		/**< Maximum characters permitted */

	bool selected;			/**< Whether control is selected */

	जोड़ {
		काष्ठा {
			पूर्णांक mx, my;
		} image;
		काष्ठा {
			पूर्णांक अंक_वस्तुएँ;
			काष्ठा form_option *वस्तुएँ, *last_item;
			bool multiple;
			पूर्णांक अंक_selected;
			/** Currently selected item, if num_selected == 1. */
			काष्ठा form_option *current;
			काष्ठा form_select_menu *menu;
		} select;
		काष्ठा {
			काष्ठा textarea *ta;
			काष्ठा dom_string *initial;
			काष्ठा form_textarea_तथ्य तथ्य;
		} text;			/**< input type=text or textarea */
	} तथ्य;

	काष्ठा form_control *पूर्व;      /**< Previous control in this form */
	काष्ठा form_control *अग्र;	/**< Next control in this form. */
};

/** Form submit method. */
प्रकार क्रमागत {
	method_GET,		/**< GET, always url encoded. */
	method_POST_URLENC,	/**< POST, url encoded. */
	method_POST_MULTIPART	/**< POST, multipart/form-data. */
} form_method;

/** HTML form. */
काष्ठा form {
	व्योम *गाँठ;			/**< Corresponding DOM node */

	अक्षर *action;			/**< Absolute URL to submit to. */
	अक्षर *target;			/**< Target to submit to. */
	form_method method;		/**< Method and enctype. */
	अक्षर *accept_charsets;		/**< Charset to submit form in */
	अक्षर *document_charset;		/**< Charset of document containing form */
	काष्ठा form_control *controls;	/**< Linked list of controls. */
	काष्ठा form_control *last_control;	/**< Last control in list. */

	काष्ठा form *पूर्व;		/**< Previous form in doc. */
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
प्रकार व्योम(*select_menu_redraw_callback)(व्योम *client_तथ्य,
		पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height);

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
काष्ठा form *form_new(व्योम *गाँठ, स्थिर अक्षर *action, स्थिर अक्षर *target,
		form_method method, स्थिर अक्षर *charset,
		स्थिर अक्षर *doc_charset);

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
व्योम form_free(काष्ठा form *form);


/**
 * Create a struct form_control.
 *
 * \param  node  Associated DOM node
 * \param  type  control type
 * \return  a new structure, or NULL on memory exhaustion
 */
काष्ठा form_control *form_new_control(व्योम *गाँठ, form_control_type type);


/**
 * Add a control to the list of controls in a form.
 *
 * \param form  The form to add the control to
 * \param control  The control to add
 */
व्योम form_add_control(काष्ठा form *form, काष्ठा form_control *control);


/**
 * Free a struct form_control.
 *
 * \param  control  structure to free
 */
व्योम form_free_control(काष्ठा form_control *control);


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
bool form_add_option(काष्ठा form_control *control, अक्षर *मूल्य, अक्षर *text,
		     bool selected, व्योम *गाँठ);


/**
 * Open a select menu for a select form control, creating it if necessary.
 *
 * \param client_data data passed to the redraw callback
 * \param control The select form control for which the menu is being opened
 * \param redraw_callback The callback to redraw the select menu.
 * \param c The content the select menu is opening for.
 * \return NSERROR_OK on sucess else error code.
 */
nserror form_open_select_menu(व्योम *client_तथ्य,
		काष्ठा form_control *control,
		select_menu_redraw_callback redraw_callback,
		काष्ठा content *c);


/**
 * Destroy a select menu and free allocated memory.
 *
 * \param control  the select form control owning the select menu being
 *                  destroyed.
 */
व्योम form_free_select_menu(काष्ठा form_control *control);


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
bool form_redraw_select_menu(काष्ठा form_control *control, पूर्णांक x, पूर्णांक y,
		भग्न scale, स्थिर काष्ठा rect *clip,
		स्थिर काष्ठा redraw_context *ctx);


/**
 * Check whether a clipping rectangle is completely contained in the
 * select menu.
 *
 * \param control  the select menu to check the clipping rectangle for
 * \param scale    the current browser window scale
 * \param clip     the clipping rectangle
 * \return true if inside false otherwise
 */
bool form_clip_inside_select_menu(काष्ठा form_control *control, भग्न scale,
		स्थिर काष्ठा rect *clip);


/**
 * Handle mouse action for the currently opened select menu.
 *
 * \param control the select menu which received the mouse action
 * \param mouse current mouse state
 * \param x X coordinate of click
 * \param y Y coordinate of click
 * \return text for the browser status bar or NULL if the menu has to be closed
 */
स्थिर अक्षर *form_select_mouse_action(काष्ठा form_control *control,
		क्रमागत browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);


/**
 * Handle mouse drag end for the currently opened select menu.
 *
 * \param control the select menu which received the mouse drag end
 * \param mouse current mouse state
 * \param x X coordinate of drag end
 * \param y Y coordinate of drag end
 */
व्योम form_select_mouse_drag_end(काष्ठा form_control *control,
		क्रमागत browser_mouse_state mouse, पूर्णांक x, पूर्णांक y);


/**
 * Get the dimensions of a select menu.
 *
 * \param control	the select menu to get the dimensions of
 * \param width		gets updated to menu width
 * \param height	gets updated to menu height
 */
व्योम form_select_get_dimensions(काष्ठा form_control *control,
		पूर्णांक *width, पूर्णांक *height);


/**
 * Callback for the core select menu.
 */
व्योम form_select_menu_callback(व्योम *client_तथ्य,
		पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height);


/**
 * Set a radio form control and clear the others in the group.
 *
 * \param radio form control of type GADGET_RADIO
 */
व्योम form_radio_set(काष्ठा form_control *radio);

/**
 * navigate browser window based on form submission.
 *
 * \param page_url content url
 * \param target The browsing context in which the navigation will occour.
 * \param form The form to submit.
 * \param submit_button The control used to submit the form.
 */
nserror form_submit(काष्ठा nsurl *page_url, काष्ठा browser_window *target,
		काष्ठा form *form, काष्ठा form_control *submit_button);


/**
 * Update gadget value.
 */
व्योम form_gadget_update_मूल्य(काष्ठा form_control *control, अक्षर *मूल्य);


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
व्योम form_gadget_sync_with_dom(काष्ठा form_control *control);

#पूर्ण_अगर
