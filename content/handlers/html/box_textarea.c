/*
 * Copyright 2013 Michael Drake <tlsa@netsurf-browser.org>
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
 * Box tree treeview box replacement (implementation).
 */

#include <string.h>
#include <dom/dom.h>

#include "utils/config.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/keypress.h"
#include "netsurf/misc.h"
#include "desktop/textarea.h"

#include "html/private.h"
#include "html/interaction.h"
#include "html/box.h"
#include "html/box_inspect.h"
#include "html/box_textarea.h"
#include "html/font.h"
#include "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, struct box *box, uint32_t सुसि)
{
	struct form_control *gadget = box->gadget;
	struct textarea *ta = gadget->data.text.ta;
	struct form* form = box->gadget->form;
	struct content *c = (struct content *)html;
	nserror res = NSERROR_OK;

	assert(ta != NULL);

	if (gadget->type == GADGET_TEXTAREA) {
		if (textarea_keypress(ta, सुसि)) {
			return NSERROR_OK;
		} else {
			return NSERROR_INVALID;
		}
	}

	/* non textarea input */
	switch (सुसि) {
	case NS_सुसि_NL:
	case NS_सुसि_CR:
		if (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  NULL);
		}
		break;

	case NS_सुसि_TAB:
		{
			struct form_control *next_पासोक;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			for (next_पासोक = gadget->next;
			     next_पासोक &&
				     ((next_पासोक->type != GADGET_TEXTBOX &&
				       next_पासोक->type != GADGET_TEXTAREA &&
				       next_पासोक->type != GADGET_PASSWORD) ||
				      !next_पासोक->box);
			     next_पासोक = next_पासोक->next)
				;

			if (next_पासोक != NULL) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_पासोक->data.text.ta, 0);
			}
		}
		break;

	case NS_सुसि_SHIFT_TAB:
		{
			struct form_control *prev_पासोक;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			for (prev_पासोक = gadget->prev;
			     prev_पासोक &&
				     ((prev_पासोक->type != GADGET_TEXTBOX &&
				       prev_पासोक->type != GADGET_TEXTAREA &&
				       prev_पासोक->type != GADGET_PASSWORD) ||
				      !prev_पासोक->box);
			     prev_पासोक = prev_पासोक->prev)
				;

			if (prev_पासोक != NULL) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_पासोक->data.text.ta, 0);
			}
		}
		break;

	default:
		/* Pass to textarea widget */
		if (!textarea_keypress(ta, सुसि)) {
			res = NSERROR_INVALID;
		}
		break;
	}

	return res;
}


/**
 * Callback for html form textareas.
 */
static void box_textarea_callback(void *data, struct textarea_मेनसाहे *मेनसाहे)
{
	struct form_textarea_data *d = data;
	struct form_control *gadget = d->gadget;
	struct html_content *html = d->gadget->html;
	struct box *box = gadget->box;

	switch (मेनसाहे->type) {
	case TEXTAREA_मेनसाहे_DRAG_REPORT:
		if (मेनसाहे->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			union html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					NULL);
		} else {
			/* Textarea drag started */
			struct rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			union html_drag_owner drag_owner;
			drag_owner.textarea = box;

			switch (मेनसाहे->data.drag) {
			case TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				break;

			case TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				break;

			default:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      मेनसाहे->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				assert(0);
				break;
			}
		}
		break;

	case TEXTAREA_मेनसाहे_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		int x, y;

		if (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			break;
		}

		box_coords(box, &x, &y);

		content__request_redraw((struct content *)html,
				x + मेनसाहे->data.redraw.x0,
				y + मेनसाहे->data.redraw.y0,
				मेनसाहे->data.redraw.x1 - मेनसाहे->data.redraw.x0,
				मेनसाहे->data.redraw.y1 - मेनसाहे->data.redraw.y0);
	}
		break;

	case TEXTAREA_मेनसाहे_SELECTION_REPORT:
		if (मेनसाहे->data.selection.have_selection) {
			/* Textarea now has a selection */
			union html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					मेनसाहे->data.selection.read_only);
		} else {
			/* The textarea now has no selection */
			union html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		break;

	case TEXTAREA_मेनसाहे_CARET_UPDATE:
		if (html->bw == NULL)
			break;

		if (मेनसाहे->data.caret.type == TEXTAREA_CARET_HIDE) {
			union html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, NULL);
		} else {
			union html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					मेनसाहे->data.caret.pos.x,
					मेनसाहे->data.caret.pos.y,
					मेनसाहे->data.caret.pos.height,
					मेनसाहे->data.caret.pos.clip);
		}
		break;

	case TEXTAREA_मेनसाहे_TEXT_MODIFIED:
		form_gadget_update_हालागा(gadget,
					 strndup(मेनसाहे->data.modified.text,
						 मेनसाहे->data.modified.len));
		break;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		struct box *box, struct dom_बुहोल *बुहोल)
{
	dom_string *dom_text = NULL;
	dom_exception err;
	textarea_setup ta_setup;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.size = 10 * PLOT_STYLE_SCALE,
		.weight = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	struct form_control *gadget = box->gadget;
	const char *text;

	assert(gadget != NULL);
	assert(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	if (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) बुहोल;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		if (err != DOM_NO_ERR)
			return false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		if (err != DOM_NO_ERR)
			return false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_हालागा(textarea, &dom_text);
		if (err != DOM_NO_ERR)
			return false;

	} else {
		dom_html_पासोक_element *पासोक = (dom_html_पासोक_element *) बुहोल;

		err = dom_html_पासोक_element_get_read_only(
				पासोक, &read_only);
		if (err != DOM_NO_ERR)
			return false;

		err = dom_html_पासोक_element_get_disabled(
				पासोक, &disabled);
		if (err != DOM_NO_ERR)
			return false;

		if (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		else
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_पासोक_element_get_हालागा(पासोक, &dom_text);
		if (err != DOM_NO_ERR)
			return false;
	}

	if (dom_text != NULL) {
		text = dom_string_data(dom_text);
	} else {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	if (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_इताअस = 4;
	ta_setup.pad_कानान = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_कालिवा = 4;

	/* Set remaining data */
	ta_setup.border_width = 0;
	ta_setup.border_col = 0x000000;
	ta_setup.text = fstyle;
	ta_setup.text.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_setup.selected_bg = fstyle.foreground;
	ta_setup.selected_text = colour_to_bw_furthest(ta_setup.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->data.text.initial = dom_text;

	gadget->data.text.ta = textarea_create(ta_flags, &ta_setup,
			box_textarea_callback, &gadget->data.text.data);

	if (gadget->data.text.ta == NULL) {
		return false;
	}

	if (!textarea_set_text(gadget->data.text.ta, text))
		return false;

	return true;
}
