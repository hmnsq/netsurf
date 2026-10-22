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


nserror box_textarea_keypress(html_content *html, ರಚನೆ box *box, uint32_t ಕೀಲಿ)
{
	ರಚನೆ form_control *gadget = box->gadget;
	ರಚನೆ textarea *ta = gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta;
	ರಚನೆ form* form = box->gadget->form;
	ರಚನೆ content *c = (ರಚನೆ content *)html;
	nserror res = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(ta != ಖಾಲಿ);

	ಆದರೆ (gadget->type == GADGET_TEXTAREA) {
		ಆದರೆ (textarea_keypress(ta, ಕೀಲಿ)) {
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹಿಂತಿರುಗಿ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ಆಯ್ಕೆ (ಕೀಲಿ) {
	ಸ್ಥಿತಿ NS_ಕೀಲಿ_NL:
	ಸ್ಥಿತಿ NS_ಕೀಲಿ_CR:
		ಆದರೆ (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ಖಾಲಿ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_TAB:
		{
			ರಚನೆ form_control *ಮುಂದಿನ_ಒಳಹರಿವು;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ಗಾಗಿ (ಮುಂದಿನ_ಒಳಹರಿವು = gadget->ಮುಂದಿನ;
			     ಮುಂದಿನ_ಒಳಹರಿವು &&
				     ((ಮುಂದಿನ_ಒಳಹರಿವು->type != GADGET_TEXTBOX &&
				       ಮುಂದಿನ_ಒಳಹರಿವು->type != GADGET_TEXTAREA &&
				       ಮುಂದಿನ_ಒಳಹರಿವು->type != GADGET_PASSWORD) ||
				      !ಮುಂದಿನ_ಒಳಹರಿವು->box);
			     ಮುಂದಿನ_ಒಳಹರಿವು = ಮುಂದಿನ_ಒಳಹರಿವು->ಮುಂದಿನ)
				;

			ಆದರೆ (ಮುಂದಿನ_ಒಳಹರಿವು != ಖಾಲಿ) {
				textarea_ಗಣ_caret(ta, -1);
				textarea_ಗಣ_caret(ಮುಂದಿನ_ಒಳಹರಿವು->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta, 0);
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಸರಿಸು_TAB:
		{
			ರಚನೆ form_control *prev_ಒಳಹರಿವು;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ಗಾಗಿ (prev_ಒಳಹರಿವು = gadget->prev;
			     prev_ಒಳಹರಿವು &&
				     ((prev_ಒಳಹರಿವು->type != GADGET_TEXTBOX &&
				       prev_ಒಳಹರಿವು->type != GADGET_TEXTAREA &&
				       prev_ಒಳಹರಿವು->type != GADGET_PASSWORD) ||
				      !prev_ಒಳಹರಿವು->box);
			     prev_ಒಳಹರಿವು = prev_ಒಳಹರಿವು->prev)
				;

			ಆದರೆ (prev_ಒಳಹರಿವು != ಖಾಲಿ) {
				textarea_ಗಣ_caret(ta, -1);
				textarea_ಗಣ_caret(prev_ಒಳಹರಿವು->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta, 0);
			}
		}
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Pass to textarea widget */
		ಆದರೆ (!textarea_keypress(ta, ಕೀಲಿ)) {
			res = NSERROR_INVALID;
		}
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Callback for html form textareas.
 */
ಸ್ಥಿರ ಶೂನ್ಯ box_textarea_callback(ಶೂನ್ಯ *ದತ್ತಾಂಶ, ರಚನೆ textarea_ಸಂದೇಶ *ಸಂದೇಶ)
{
	ರಚನೆ form_textarea_ದತ್ತಾಂಶ *d = ದತ್ತಾಂಶ;
	ರಚನೆ form_control *gadget = d->gadget;
	ರಚನೆ html_content *html = d->gadget->html;
	ರಚನೆ box *box = gadget->box;

	ಆಯ್ಕೆ (ಸಂದೇಶ->type) {
	ಸ್ಥಿತಿ TEXTAREA_ಸಂದೇಶ_DRAG_REPORT:
		ಆದರೆ (ಸಂದೇಶ->ದತ್ತಾಂಶ.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			ಒಕ್ಕೂಟ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_ಗಣ_drag_type(html, drag_type, drag_owner,
					ಖಾಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Textarea drag started */
			ರಚನೆ rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ,
				.y1 = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ
			};
			ಒಕ್ಕೂಟ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ಆಯ್ಕೆ (ಸಂದೇಶ->ದತ್ತಾಂಶ.drag) {
			ಸ್ಥಿತಿ TEXTAREA_DRAG_SCROLLBAR:
				html_ಗಣ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ TEXTAREA_DRAG_SELECTION:
				html_ಗಣ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ನಿಲ್ಲಿಸು;

			ಪೂರ್ವನಿಯೋಜಿತ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      ಸಂದೇಶ->ದತ್ತಾಂಶ.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ಪ್ರತಿಜ್ಞಾ(0);
				ನಿಲ್ಲಿಸು;
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ TEXTAREA_ಸಂದೇಶ_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		ಪೂರ್ಣಾಂಕ x, y;

		ಆದರೆ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ನಿಲ್ಲಿಸು;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ರಚನೆ content *)html,
				x + ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.x0,
				y + ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.y0,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.x1 - ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.x0,
				ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.y1 - ಸಂದೇಶ->ದತ್ತಾಂಶ.redraw.y0);
	}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ TEXTAREA_ಸಂದೇಶ_SELECTION_REPORT:
		ಆದರೆ (ಸಂದೇಶ->ದತ್ತಾಂಶ.selection.have_selection) {
			/* Textarea now has a selection */
			ಒಕ್ಕೂಟ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_ಗಣ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					ಸಂದೇಶ->ದತ್ತಾಂಶ.selection.read_only);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* The textarea now has no selection */
			ಒಕ್ಕೂಟ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_ಗಣ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ TEXTAREA_ಸಂದೇಶ_CARET_UPDATE:
		ಆದರೆ (html->bw == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (ಸಂದೇಶ->ದತ್ತಾಂಶ.caret.type == TEXTAREA_CARET_HIDE) {
			ಒಕ್ಕೂಟ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_ಗಣ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ಖಾಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಒಕ್ಕೂಟ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_ಗಣ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					ಸಂದೇಶ->ದತ್ತಾಂಶ.caret.pos.x,
					ಸಂದೇಶ->ದತ್ತಾಂಶ.caret.pos.y,
					ಸಂದೇಶ->ದತ್ತಾಂಶ.caret.pos.ಎತ್ತರ,
					ಸಂದೇಶ->ದತ್ತಾಂಶ.caret.pos.clip);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ TEXTAREA_ಸಂದೇಶ_ಪಠ್ಯ_MODIFIED:
		form_gadget_update_ಮೌಲ್ಯ(gadget,
					 strndup(ಸಂದೇಶ->ದತ್ತಾಂಶ.modified.ಪಠ್ಯ,
						 ಸಂದೇಶ->ದತ್ತಾಂಶ.modified.len));
		ನಿಲ್ಲಿಸು;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ರಚನೆ box *box, ರಚನೆ dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *dom_ಪಠ್ಯ = ಖಾಲಿ;
	dom_exception err;
	textarea_ಸಿದ್ಧತೆ ta_ಸಿದ್ಧತೆ;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.ಗಾತ್ರ_2 = 10 * PLOT_STYLE_SCALE,
		.ತೂಕ = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	ರಚನೆ form_control *gadget = box->gadget;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ;

	ಪ್ರತಿಜ್ಞಾ(gadget != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	ಆದರೆ (gadget->type == GADGET_TEXTAREA) {
		dom_html_ಪಠ್ಯ_area_element *textarea =
				(dom_html_ಪಠ್ಯ_area_element *) ಗಂಟು;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_ಪಠ್ಯ_area_element_get_read_only(
				textarea, &read_only);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		err = dom_html_ಪಠ್ಯ_area_element_get_disabled(
				textarea, &disabled);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		/* Get the textarea's initial content */
		err = dom_html_ಪಠ್ಯ_area_element_get_ಮೌಲ್ಯ(textarea, &dom_ಪಠ್ಯ);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ {
		dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು = (dom_html_ಒಳಹರಿವು_element *) ಗಂಟು;

		err = dom_html_ಒಳಹರಿವು_element_get_read_only(
				ಒಳಹರಿವು, &read_only);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		err = dom_html_ಒಳಹರಿವು_element_get_disabled(
				ಒಳಹರಿವು, &disabled);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		ಇಲ್ಲದಿದ್ದರೆ
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು, &dom_ಪಠ್ಯ);
		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (dom_ಪಠ್ಯ != ಖಾಲಿ) {
		ಪಠ್ಯ = dom_string_ದತ್ತಾಂಶ(dom_ಪಠ್ಯ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* No initial text, or failed reading it;
		 * use a blank string */
		ಪಠ್ಯ = "";
	}

	ಆದರೆ (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ದತ್ತಾಂಶ.gadget = gadget;

	/* Reset to correct values by layout */
	ta_ಸಿದ್ಧತೆ.ಅಗಲ = 200;
	ta_ಸಿದ್ಧತೆ.ಎತ್ತರ = 20;
	ta_ಸಿದ್ಧತೆ.pad_ತುದಿ = 4;
	ta_ಸಿದ್ಧತೆ.pad_ಬಲ = 4;
	ta_ಸಿದ್ಧತೆ.pad_bottom = 4;
	ta_ಸಿದ್ಧತೆ.pad_ಎಡ = 4;

	/* Set remaining data */
	ta_ಸಿದ್ಧತೆ.border_ಅಗಲ = 0;
	ta_ಸಿದ್ಧತೆ.border_ಲಂಬಸಾಲು = 0x000000;
	ta_ಸಿದ್ಧತೆ.ಪಠ್ಯ = fstyle;
	ta_ಸಿದ್ಧತೆ.ಪಠ್ಯ.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_ಸಿದ್ಧತೆ.selected_bg = fstyle.foreground;
	ta_ಸಿದ್ಧತೆ.selected_ಪಠ್ಯ = ಬಣ್ಣ_ಗಮ್ಯ_bw_furthest(ta_ಸಿದ್ಧತೆ.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.initial = dom_ಪಠ್ಯ;

	gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta = textarea_create(ta_flags, &ta_ಸಿದ್ಧತೆ,
			box_textarea_callback, &gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ದತ್ತಾಂಶ);

	ಆದರೆ (gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!textarea_ಗಣ_ಪಠ್ಯ(gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta, ಪಠ್ಯ))
		ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ true;
}
