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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/keypress.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "desktop/textarea.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/interaction.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/box_textarea.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, কাঠামো box *box, uint32_t চাবি)
{
	কাঠামো form_control *gadget = box->gadget;
	কাঠামো textarea *ta = gadget->তথ্য.text.ta;
	কাঠামো form* form = box->gadget->form;
	কাঠামো content *c = (কাঠামো content *)html;
	nserror res = NSERROR_OK;

	প্রতিজ্ঞা(ta != ফাঁকা);

	যদি (gadget->type == GADGET_TEXTAREA) {
		যদি (textarea_keypress(ta, চাবি)) {
			ফেরত NSERROR_OK;
		} অন্যথা {
			ফেরত NSERROR_INVALID;
		}
	}

	/* non textarea input */
	নির্বাচন (চাবি) {
	ক্ষেত্র NS_চাবি_NL:
	ক্ষেত্র NS_চাবি_CR:
		যদি (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ফাঁকা);
		}
		বিরতি;

	ক্ষেত্র NS_চাবি_TAB:
		{
			কাঠামো form_control *পরবর্তী_প্রবেশ;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			জন্য (পরবর্তী_প্রবেশ = gadget->পরবর্তী;
			     পরবর্তী_প্রবেশ &&
				     ((পরবর্তী_প্রবেশ->type != GADGET_TEXTBOX &&
				       পরবর্তী_প্রবেশ->type != GADGET_TEXTAREA &&
				       পরবর্তী_প্রবেশ->type != GADGET_PASSWORD) ||
				      !পরবর্তী_প্রবেশ->box);
			     পরবর্তী_প্রবেশ = পরবর্তী_প্রবেশ->পরবর্তী)
				;

			যদি (পরবর্তী_প্রবেশ != ফাঁকা) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(পরবর্তী_প্রবেশ->তথ্য.text.ta, 0);
			}
		}
		বিরতি;

	ক্ষেত্র NS_চাবি_SHIFT_TAB:
		{
			কাঠামো form_control *পূর্ব_প্রবেশ;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			জন্য (পূর্ব_প্রবেশ = gadget->পূর্ব;
			     পূর্ব_প্রবেশ &&
				     ((পূর্ব_প্রবেশ->type != GADGET_TEXTBOX &&
				       পূর্ব_প্রবেশ->type != GADGET_TEXTAREA &&
				       পূর্ব_প্রবেশ->type != GADGET_PASSWORD) ||
				      !পূর্ব_প্রবেশ->box);
			     পূর্ব_প্রবেশ = পূর্ব_প্রবেশ->পূর্ব)
				;

			যদি (পূর্ব_প্রবেশ != ফাঁকা) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(পূর্ব_প্রবেশ->তথ্য.text.ta, 0);
			}
		}
		বিরতি;

	পূর্বনির্ধারিত:
		/* Pass to textarea widget */
		যদি (!textarea_keypress(ta, চাবি)) {
			res = NSERROR_INVALID;
		}
		বিরতি;
	}

	ফেরত res;
}


/**
 * Callback for html form textareas.
 */
স্থির শূন্য box_textarea_callback(শূন্য *তথ্য, কাঠামো textarea_বার্তা *বার্তা)
{
	কাঠামো form_textarea_তথ্য *d = তথ্য;
	কাঠামো form_control *gadget = d->gadget;
	কাঠামো html_content *html = d->gadget->html;
	কাঠামো box *box = gadget->box;

	নির্বাচন (বার্তা->type) {
	ক্ষেত্র TEXTAREA_বার্তা_DRAG_REPORT:
		যদি (বার্তা->তথ্য.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			সংঘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					ফাঁকা);
		} অন্যথা {
			/* Textarea drag started */
			কাঠামো rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			সংঘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			নির্বাচন (বার্তা->তথ্য.drag) {
			ক্ষেত্র TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				বিরতি;

			ক্ষেত্র TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				বিরতি;

			পূর্বনির্ধারিত:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      বার্তা->তথ্য.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				প্রতিজ্ঞা(0);
				বিরতি;
			}
		}
		বিরতি;

	ক্ষেত্র TEXTAREA_বার্তা_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		পূর্ণসংখ্যা x, y;

		যদি (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			বিরতি;
		}

		box_coords(box, &x, &y);

		content__request_redraw((কাঠামো content *)html,
				x + বার্তা->তথ্য.redraw.x0,
				y + বার্তা->তথ্য.redraw.y0,
				বার্তা->তথ্য.redraw.x1 - বার্তা->তথ্য.redraw.x0,
				বার্তা->তথ্য.redraw.y1 - বার্তা->তথ্য.redraw.y0);
	}
		বিরতি;

	ক্ষেত্র TEXTAREA_বার্তা_SELECTION_REPORT:
		যদি (বার্তা->তথ্য.selection.have_selection) {
			/* Textarea now has a selection */
			সংঘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					বার্তা->তথ্য.selection.read_only);
		} অন্যথা {
			/* The textarea now has no selection */
			সংঘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		বিরতি;

	ক্ষেত্র TEXTAREA_বার্তা_CARET_UPDATE:
		যদি (html->bw == ফাঁকা)
			বিরতি;

		যদি (বার্তা->তথ্য.caret.type == TEXTAREA_CARET_HIDE) {
			সংঘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ফাঁকা);
		} অন্যথা {
			সংঘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					বার্তা->তথ্য.caret.pos.x,
					বার্তা->তথ্য.caret.pos.y,
					বার্তা->তথ্য.caret.pos.height,
					বার্তা->তথ্য.caret.pos.clip);
		}
		বিরতি;

	ক্ষেত্র TEXTAREA_বার্তা_TEXT_MODIFIED:
		form_gadget_update_মান(gadget,
					 strndup(বার্তা->তথ্য.modified.text,
						 বার্তা->তথ্য.modified.len));
		বিরতি;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		কাঠামো box *box, কাঠামো dom_গ্রন্থি *গ্রন্থি)
{
	dom_string *dom_text = ফাঁকা;
	dom_exception err;
	textarea_setup ta_setup;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.আকার_2 = 10 * PLOT_STYLE_SCALE,
		.weight = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	কাঠামো form_control *gadget = box->gadget;
	ধ্রুবক অক্ষর *text;

	প্রতিজ্ঞা(gadget != ফাঁকা);
	প্রতিজ্ঞা(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	যদি (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) গ্রন্থি;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		যদি (err != DOM_NO_ERR)
			ফেরত false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		যদি (err != DOM_NO_ERR)
			ফেরত false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_মান(textarea, &dom_text);
		যদি (err != DOM_NO_ERR)
			ফেরত false;

	} অন্যথা {
		dom_html_প্রবেশ_element *প্রবেশ = (dom_html_প্রবেশ_element *) গ্রন্থি;

		err = dom_html_প্রবেশ_element_get_read_only(
				প্রবেশ, &read_only);
		যদি (err != DOM_NO_ERR)
			ফেরত false;

		err = dom_html_প্রবেশ_element_get_disabled(
				প্রবেশ, &disabled);
		যদি (err != DOM_NO_ERR)
			ফেরত false;

		যদি (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		অন্যথা
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_প্রবেশ_element_get_মান(প্রবেশ, &dom_text);
		যদি (err != DOM_NO_ERR)
			ফেরত false;
	}

	যদি (dom_text != ফাঁকা) {
		text = dom_string_তথ্য(dom_text);
	} অন্যথা {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	যদি (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->তথ্য.text.তথ্য.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_শীর্ষ = 4;
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
	gadget->তথ্য.text.initial = dom_text;

	gadget->তথ্য.text.ta = textarea_create(ta_flags, &ta_setup,
			box_textarea_callback, &gadget->তথ্য.text.তথ্য);

	যদি (gadget->তথ্য.text.ta == ফাঁকা) {
		ফেরত false;
	}

	যদি (!textarea_set_text(gadget->তথ্য.text.ta, text))
		ফেরত false;

	ফেরত true;
}
