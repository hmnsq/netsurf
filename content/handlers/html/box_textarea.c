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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/keypress.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "desktop/textarea.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/interaction.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/box_textarea.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, கட்டமைப்பு box *box, uint32_t சாவி)
{
	கட்டமைப்பு form_control *gadget = box->gadget;
	கட்டமைப்பு textarea *ta = gadget->data.text.ta;
	கட்டமைப்பு form* form = box->gadget->form;
	கட்டமைப்பு content *c = (கட்டமைப்பு content *)html;
	nserror res = NSERROR_OK;

	ப்ரதிஜ்ஞா(ta != வெற்று);

	எனில் (gadget->type == GADGET_TEXTAREA) {
		எனில் (textarea_keypress(ta, சாவி)) {
			திரும்பு NSERROR_OK;
		} இல்லையெனில் {
			திரும்பு NSERROR_INVALID;
		}
	}

	/* non textarea input */
	தேர்வு (சாவி) {
	நிலை NS_சாவி_NL:
	நிலை NS_சாவி_CR:
		எனில் (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  வெற்று);
		}
		நிறுத்து;

	நிலை NS_சாவி_TAB:
		{
			கட்டமைப்பு form_control *next_உள்ளீடு;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			க்கு (next_உள்ளீடு = gadget->next;
			     next_உள்ளீடு &&
				     ((next_உள்ளீடு->type != GADGET_TEXTBOX &&
				       next_உள்ளீடு->type != GADGET_TEXTAREA &&
				       next_உள்ளீடு->type != GADGET_PASSWORD) ||
				      !next_உள்ளீடு->box);
			     next_உள்ளீடு = next_உள்ளீடு->next)
				;

			எனில் (next_உள்ளீடு != வெற்று) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_உள்ளீடு->data.text.ta, 0);
			}
		}
		நிறுத்து;

	நிலை NS_சாவி_SHIFT_TAB:
		{
			கட்டமைப்பு form_control *prev_உள்ளீடு;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			க்கு (prev_உள்ளீடு = gadget->prev;
			     prev_உள்ளீடு &&
				     ((prev_உள்ளீடு->type != GADGET_TEXTBOX &&
				       prev_உள்ளீடு->type != GADGET_TEXTAREA &&
				       prev_உள்ளீடு->type != GADGET_PASSWORD) ||
				      !prev_உள்ளீடு->box);
			     prev_உள்ளீடு = prev_உள்ளீடு->prev)
				;

			எனில் (prev_உள்ளீடு != வெற்று) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_உள்ளீடு->data.text.ta, 0);
			}
		}
		நிறுத்து;

	இயல்பு:
		/* Pass to textarea widget */
		எனில் (!textarea_keypress(ta, சாவி)) {
			res = NSERROR_INVALID;
		}
		நிறுத்து;
	}

	திரும்பு res;
}


/**
 * Callback for html form textareas.
 */
நிலையான வெறுமை box_textarea_callback(வெறுமை *data, கட்டமைப்பு textarea_செய்தி *செய்தி)
{
	கட்டமைப்பு form_textarea_data *d = data;
	கட்டமைப்பு form_control *gadget = d->gadget;
	கட்டமைப்பு html_content *html = d->gadget->html;
	கட்டமைப்பு box *box = gadget->box;

	தேர்வு (செய்தி->type) {
	நிலை TEXTAREA_செய்தி_DRAG_REPORT:
		எனில் (செய்தி->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			ஒன்றிப்பு html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					வெற்று);
		} இல்லையெனில் {
			/* Textarea drag started */
			கட்டமைப்பு rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			ஒன்றிப்பு html_drag_owner drag_owner;
			drag_owner.textarea = box;

			தேர்வு (செய்தி->data.drag) {
			நிலை TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				நிறுத்து;

			நிலை TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				நிறுத்து;

			இயல்பு:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      செய்தி->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ப்ரதிஜ்ஞா(0);
				நிறுத்து;
			}
		}
		நிறுத்து;

	நிலை TEXTAREA_செய்தி_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		முழு x, y;

		எனில் (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			நிறுத்து;
		}

		box_coords(box, &x, &y);

		content__request_redraw((கட்டமைப்பு content *)html,
				x + செய்தி->data.redraw.x0,
				y + செய்தி->data.redraw.y0,
				செய்தி->data.redraw.x1 - செய்தி->data.redraw.x0,
				செய்தி->data.redraw.y1 - செய்தி->data.redraw.y0);
	}
		நிறுத்து;

	நிலை TEXTAREA_செய்தி_SELECTION_REPORT:
		எனில் (செய்தி->data.selection.have_selection) {
			/* Textarea now has a selection */
			ஒன்றிப்பு html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					செய்தி->data.selection.read_only);
		} இல்லையெனில் {
			/* The textarea now has no selection */
			ஒன்றிப்பு html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		நிறுத்து;

	நிலை TEXTAREA_செய்தி_CARET_UPDATE:
		எனில் (html->bw == வெற்று)
			நிறுத்து;

		எனில் (செய்தி->data.caret.type == TEXTAREA_CARET_HIDE) {
			ஒன்றிப்பு html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, வெற்று);
		} இல்லையெனில் {
			ஒன்றிப்பு html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					செய்தி->data.caret.pos.x,
					செய்தி->data.caret.pos.y,
					செய்தி->data.caret.pos.height,
					செய்தி->data.caret.pos.clip);
		}
		நிறுத்து;

	நிலை TEXTAREA_செய்தி_TEXT_MODIFIED:
		form_gadget_update_மதிப்பு(gadget,
					 strndup(செய்தி->data.modified.text,
						 செய்தி->data.modified.len));
		நிறுத்து;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		கட்டமைப்பு box *box, கட்டமைப்பு dom_முனை *முனை)
{
	dom_string *dom_text = வெற்று;
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
	கட்டமைப்பு form_control *gadget = box->gadget;
	மாறிலி எழுத்து *text;

	ப்ரதிஜ்ஞா(gadget != வெற்று);
	ப்ரதிஜ்ஞா(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	எனில் (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) முனை;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_மதிப்பு(textarea, &dom_text);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

	} இல்லையெனில் {
		dom_html_உள்ளீடு_element *உள்ளீடு = (dom_html_உள்ளீடு_element *) முனை;

		err = dom_html_உள்ளீடு_element_get_read_only(
				உள்ளீடு, &read_only);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		err = dom_html_உள்ளீடு_element_get_disabled(
				உள்ளீடு, &disabled);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		எனில் (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		இல்லையெனில்
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_உள்ளீடு_element_get_மதிப்பு(உள்ளீடு, &dom_text);
		எனில் (err != DOM_NO_ERR)
			திரும்பு false;
	}

	எனில் (dom_text != வெற்று) {
		text = dom_string_data(dom_text);
	} இல்லையெனில் {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	எனில் (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_உச்சி = 4;
	ta_setup.pad_வலது = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_இடது = 4;

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

	எனில் (gadget->data.text.ta == வெற்று) {
		திரும்பு false;
	}

	எனில் (!textarea_set_text(gadget->data.text.ta, text))
		திரும்பு false;

	திரும்பு true;
}
