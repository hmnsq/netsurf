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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/box_textarea.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, ಸಂರಚನಾ box *box, uint32_t ಕೀಲಿ)
{
	ಸಂರಚನಾ form_control *gadget = box->gadget;
	ಸಂರಚನಾ textarea *ta = gadget->data.text.ta;
	ಸಂರಚನಾ form* form = box->gadget->form;
	ಸಂರಚನಾ content *c = (ಸಂರಚನಾ content *)html;
	nserror res = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(ta != ರಿಕ್ತ);

	ಯದಿ (gadget->type == GADGET_TEXTAREA) {
		ಯದಿ (textarea_keypress(ta, ಕೀಲಿ)) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
		} ಅನ್ಯಥಾ {
			ಪ್ರತ್ಯಾಗಮ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ವಿಕಲ್ಪ (ಕೀಲಿ) {
	ಪಕ್ಷ NS_ಕೀಲಿ_NL:
	ಪಕ್ಷ NS_ಕೀಲಿ_CR:
		ಯದಿ (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ರಿಕ್ತ);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ NS_ಕೀಲಿ_TAB:
		{
			ಸಂರಚನಾ form_control *next_ಒಳಹರಿವು;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ಪ್ರತಿ (next_ಒಳಹರಿವು = gadget->next;
			     next_ಒಳಹರಿವು &&
				     ((next_ಒಳಹರಿವು->type != GADGET_TEXTBOX &&
				       next_ಒಳಹರಿವು->type != GADGET_TEXTAREA &&
				       next_ಒಳಹರಿವು->type != GADGET_PASSWORD) ||
				      !next_ಒಳಹರಿವು->box);
			     next_ಒಳಹರಿವು = next_ಒಳಹರಿವು->next)
				;

			ಯದಿ (next_ಒಳಹರಿವು != ರಿಕ್ತ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_ಒಳಹರಿವು->data.text.ta, 0);
			}
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ NS_ಕೀಲಿ_SHIFT_TAB:
		{
			ಸಂರಚನಾ form_control *prev_ಒಳಹರಿವು;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ಪ್ರತಿ (prev_ಒಳಹರಿವು = gadget->prev;
			     prev_ಒಳಹರಿವು &&
				     ((prev_ಒಳಹರಿವು->type != GADGET_TEXTBOX &&
				       prev_ಒಳಹರಿವು->type != GADGET_TEXTAREA &&
				       prev_ಒಳಹರಿವು->type != GADGET_PASSWORD) ||
				      !prev_ಒಳಹರಿವು->box);
			     prev_ಒಳಹರಿವು = prev_ಒಳಹರಿವು->prev)
				;

			ಯದಿ (prev_ಒಳಹರಿವು != ರಿಕ್ತ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_ಒಳಹರಿವು->data.text.ta, 0);
			}
		}
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		/* Pass to textarea widget */
		ಯದಿ (!textarea_keypress(ta, ಕೀಲಿ)) {
			res = NSERROR_INVALID;
		}
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ res;
}


/**
 * Callback for html form textareas.
 */
ಸ್ಥಿರ ಶೂನ್ಯ box_textarea_callback(ಶೂನ್ಯ *data, ಸಂರಚನಾ textarea_ಸಂದೇಶ *ಸಂದೇಶ)
{
	ಸಂರಚನಾ form_textarea_data *d = data;
	ಸಂರಚನಾ form_control *gadget = d->gadget;
	ಸಂರಚನಾ html_content *html = d->gadget->html;
	ಸಂರಚನಾ box *box = gadget->box;

	ವಿಕಲ್ಪ (ಸಂದೇಶ->type) {
	ಪಕ್ಷ TEXTAREA_ಸಂದೇಶ_DRAG_REPORT:
		ಯದಿ (ಸಂದೇಶ->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			ಸಙ್ಘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					ರಿಕ್ತ);
		} ಅನ್ಯಥಾ {
			/* Textarea drag started */
			ಸಂರಚನಾ rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			ಸಙ್ಘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ವಿಕಲ್ಪ (ಸಂದೇಶ->data.drag) {
			ಪಕ್ಷ TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ಭಙ್ಗ;

			ಪಕ್ಷ TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ಭಙ್ಗ;

			ಸ್ವಭಾವ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      ಸಂದೇಶ->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ಪ್ರತಿಜ್ಞಾ(0);
				ಭಙ್ಗ;
			}
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ TEXTAREA_ಸಂದೇಶ_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		ಪೂರ್ಣಾಙ್ಕ x, y;

		ಯದಿ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ಭಙ್ಗ;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ಸಂರಚನಾ content *)html,
				x + ಸಂದೇಶ->data.redraw.x0,
				y + ಸಂದೇಶ->data.redraw.y0,
				ಸಂದೇಶ->data.redraw.x1 - ಸಂದೇಶ->data.redraw.x0,
				ಸಂದೇಶ->data.redraw.y1 - ಸಂದೇಶ->data.redraw.y0);
	}
		ಭಙ್ಗ;

	ಪಕ್ಷ TEXTAREA_ಸಂದೇಶ_SELECTION_REPORT:
		ಯದಿ (ಸಂದೇಶ->data.selection.have_selection) {
			/* Textarea now has a selection */
			ಸಙ್ಘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					ಸಂದೇಶ->data.selection.read_only);
		} ಅನ್ಯಥಾ {
			/* The textarea now has no selection */
			ಸಙ್ಘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ TEXTAREA_ಸಂದೇಶ_CARET_UPDATE:
		ಯದಿ (html->bw == ರಿಕ್ತ)
			ಭಙ್ಗ;

		ಯದಿ (ಸಂದೇಶ->data.caret.type == TEXTAREA_CARET_HIDE) {
			ಸಙ್ಘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ರಿಕ್ತ);
		} ಅನ್ಯಥಾ {
			ಸಙ್ಘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					ಸಂದೇಶ->data.caret.pos.x,
					ಸಂದೇಶ->data.caret.pos.y,
					ಸಂದೇಶ->data.caret.pos.height,
					ಸಂದೇಶ->data.caret.pos.clip);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ TEXTAREA_ಸಂದೇಶ_TEXT_MODIFIED:
		form_gadget_update_ಮೌಲ್ಯ(gadget,
					 strndup(ಸಂದೇಶ->data.modified.text,
						 ಸಂದೇಶ->data.modified.len));
		ಭಙ್ಗ;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ಸಂರಚನಾ box *box, ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *dom_text = ರಿಕ್ತ;
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
	ಸಂರಚನಾ form_control *gadget = box->gadget;
	ಧ್ರುವ ವರ್ಣ *text;

	ಪ್ರತಿಜ್ಞಾ(gadget != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	ಯದಿ (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) ಗಂಟು;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_ಮೌಲ್ಯ(textarea, &dom_text);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ {
		dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು = (dom_html_ಒಳಹರಿವು_element *) ಗಂಟು;

		err = dom_html_ಒಳಹರಿವು_element_get_read_only(
				ಒಳಹರಿವು, &read_only);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		err = dom_html_ಒಳಹರಿವು_element_get_disabled(
				ಒಳಹರಿವು, &disabled);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		ಅನ್ಯಥಾ
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು, &dom_text);
		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (dom_text != ರಿಕ್ತ) {
		text = dom_string_data(dom_text);
	} ಅನ್ಯಥಾ {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	ಯದಿ (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_ತುದಿ = 4;
	ta_setup.pad_ಬಲ = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_ಎಡ = 4;

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

	ಯದಿ (gadget->data.text.ta == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!textarea_set_text(gadget->data.text.ta, text))
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ true;
}
