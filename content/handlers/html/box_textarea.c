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

#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <dom/dom.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/keypress.h"
#অন্তর্ভাৱ "netsurf/misc.h"
#অন্তর্ভাৱ "desktop/textarea.h"

#অন্তর্ভাৱ "html/private.h"
#অন্তর্ভাৱ "html/interaction.h"
#অন্তর্ভাৱ "html/box.h"
#অন্তর্ভাৱ "html/box_inspect.h"
#অন্তর্ভাৱ "html/box_textarea.h"
#অন্তর্ভাৱ "html/font.h"
#অন্তর্ভাৱ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, সংরচনা box *box, uint32_t চাবি)
{
	সংরচনা form_control *gadget = box->gadget;
	সংরচনা textarea *ta = gadget->data.text.ta;
	সংরচনা form* form = box->gadget->form;
	সংরচনা content *c = (সংরচনা content *)html;
	nserror res = NSERROR_OK;

	প্রতিজ্ঞা(ta != রিক্ত);

	যদি (gadget->type == GADGET_TEXTAREA) {
		যদি (textarea_keypress(ta, চাবি)) {
			ঘূৰাই NSERROR_OK;
		} অন্যথা {
			ঘূৰাই NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ৱিকল্প (চাবি) {
	পক্ষ NS_চাবি_NL:
	পক্ষ NS_চাবি_CR:
		যদি (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  রিক্ত);
		}
		ভঙ্গ;

	পক্ষ NS_চাবি_TAB:
		{
			সংরচনা form_control *next_প্ৰবেশ;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			বাবে (next_প্ৰবেশ = gadget->next;
			     next_প্ৰবেশ &&
				     ((next_প্ৰবেশ->type != GADGET_TEXTBOX &&
				       next_প্ৰবেশ->type != GADGET_TEXTAREA &&
				       next_প্ৰবেশ->type != GADGET_PASSWORD) ||
				      !next_প্ৰবেশ->box);
			     next_প্ৰবেশ = next_প্ৰবেশ->next)
				;

			যদি (next_প্ৰবেশ != রিক্ত) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_প্ৰবেশ->data.text.ta, 0);
			}
		}
		ভঙ্গ;

	পক্ষ NS_চাবি_SHIFT_TAB:
		{
			সংরচনা form_control *prev_প্ৰবেশ;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			বাবে (prev_প্ৰবেশ = gadget->prev;
			     prev_প্ৰবেশ &&
				     ((prev_প্ৰবেশ->type != GADGET_TEXTBOX &&
				       prev_প্ৰবেশ->type != GADGET_TEXTAREA &&
				       prev_প্ৰবেশ->type != GADGET_PASSWORD) ||
				      !prev_প্ৰবেশ->box);
			     prev_প্ৰবেশ = prev_প্ৰবেশ->prev)
				;

			যদি (prev_প্ৰবেশ != রিক্ত) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_প্ৰবেশ->data.text.ta, 0);
			}
		}
		ভঙ্গ;

	স্ৱভাৱ:
		/* Pass to textarea widget */
		যদি (!textarea_keypress(ta, চাবি)) {
			res = NSERROR_INVALID;
		}
		ভঙ্গ;
	}

	ঘূৰাই res;
}


/**
 * Callback for html form textareas.
 */
স্থির শূন্য box_textarea_callback(শূন্য *data, সংরচনা textarea_বাৰ্তা *বাৰ্তা)
{
	সংরচনা form_textarea_data *d = data;
	সংরচনা form_control *gadget = d->gadget;
	সংরচনা html_content *html = d->gadget->html;
	সংরচনা box *box = gadget->box;

	ৱিকল্প (বাৰ্তা->type) {
	পক্ষ TEXTAREA_বাৰ্তা_DRAG_REPORT:
		যদি (বাৰ্তা->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			সঙ্ঘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					রিক্ত);
		} অন্যথা {
			/* Textarea drag started */
			সংরচনা rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			সঙ্ঘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ৱিকল্প (বাৰ্তা->data.drag) {
			পক্ষ TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ভঙ্গ;

			পক্ষ TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ভঙ্গ;

			স্ৱভাৱ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      বাৰ্তা->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				প্রতিজ্ঞা(0);
				ভঙ্গ;
			}
		}
		ভঙ্গ;

	পক্ষ TEXTAREA_বাৰ্তা_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		পূর্ণাঙ্ক x, y;

		যদি (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ভঙ্গ;
		}

		box_coords(box, &x, &y);

		content__request_redraw((সংরচনা content *)html,
				x + বাৰ্তা->data.redraw.x0,
				y + বাৰ্তা->data.redraw.y0,
				বাৰ্তা->data.redraw.x1 - বাৰ্তা->data.redraw.x0,
				বাৰ্তা->data.redraw.y1 - বাৰ্তা->data.redraw.y0);
	}
		ভঙ্গ;

	পক্ষ TEXTAREA_বাৰ্তা_SELECTION_REPORT:
		যদি (বাৰ্তা->data.selection.have_selection) {
			/* Textarea now has a selection */
			সঙ্ঘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					বাৰ্তা->data.selection.read_only);
		} অন্যথা {
			/* The textarea now has no selection */
			সঙ্ঘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ভঙ্গ;

	পক্ষ TEXTAREA_বাৰ্তা_CARET_UPDATE:
		যদি (html->bw == রিক্ত)
			ভঙ্গ;

		যদি (বাৰ্তা->data.caret.type == TEXTAREA_CARET_HIDE) {
			সঙ্ঘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, রিক্ত);
		} অন্যথা {
			সঙ্ঘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					বাৰ্তা->data.caret.pos.x,
					বাৰ্তা->data.caret.pos.y,
					বাৰ্তা->data.caret.pos.height,
					বাৰ্তা->data.caret.pos.clip);
		}
		ভঙ্গ;

	পক্ষ TEXTAREA_বাৰ্তা_TEXT_MODIFIED:
		form_gadget_update_মান(gadget,
					 strndup(বাৰ্তা->data.modified.text,
						 বাৰ্তা->data.modified.len));
		ভঙ্গ;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		সংরচনা box *box, সংরচনা dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_string *dom_text = রিক্ত;
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
	সংরচনা form_control *gadget = box->gadget;
	ধ্রুৱ ৱর্ণ *text;

	প্রতিজ্ঞা(gadget != রিক্ত);
	প্রতিজ্ঞা(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	যদি (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) গ্ৰন্থি;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_মান(textarea, &dom_text);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;

	} অন্যথা {
		dom_html_প্ৰবেশ_element *প্ৰবেশ = (dom_html_প্ৰবেশ_element *) গ্ৰন্থি;

		err = dom_html_প্ৰবেশ_element_get_read_only(
				প্ৰবেশ, &read_only);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;

		err = dom_html_প্ৰবেশ_element_get_disabled(
				প্ৰবেশ, &disabled);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;

		যদি (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		অন্যথা
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_প্ৰবেশ_element_get_মান(প্ৰবেশ, &dom_text);
		যদি (err != DOM_NO_ERR)
			ঘূৰাই false;
	}

	যদি (dom_text != রিক্ত) {
		text = dom_string_data(dom_text);
	} অন্যথা {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	যদি (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_শীৰ্ষ = 4;
	ta_setup.pad_ডান = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_বাম = 4;

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

	যদি (gadget->data.text.ta == রিক্ত) {
		ঘূৰাই false;
	}

	যদি (!textarea_set_text(gadget->data.text.ta, text))
		ঘূৰাই false;

	ঘূৰাই true;
}
