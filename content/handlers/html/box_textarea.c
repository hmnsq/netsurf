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

#ضمن <string.h>
#ضمن <dom/dom.h>

#ضمن "utils/config.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/keypress.h"
#ضمن "netsurf/misc.h"
#ضمن "desktop/textarea.h"

#ضمن "html/private.h"
#ضمن "html/interaction.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/box_textarea.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, بنية box *box, uint32_t کلیل)
{
	بنية form_control *gadget = box->gadget;
	بنية textarea *ta = gadget->data.text.ta;
	بنية form* form = box->gadget->form;
	بنية content *c = (بنية content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != عدم);

	إذا (gadget->type == GADGET_TEXTAREA) {
		إذا (textarea_keypress(ta, کلیل)) {
			أرجع NSERROR_OK;
		} وإلا {
			أرجع NSERROR_INVALID;
		}
	}

	/* non textarea input */
	بدل (کلیل) {
	حالة NS_کلیل_NL:
	حالة NS_کلیل_CR:
		إذا (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  عدم);
		}
		اكسر;

	حالة NS_کلیل_TAB:
		{
			بنية form_control *next_تێخستن;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (next_تێخستن = gadget->next;
			     next_تێخستن &&
				     ((next_تێخستن->type != GADGET_TEXTBOX &&
				       next_تێخستن->type != GADGET_TEXTAREA &&
				       next_تێخستن->type != GADGET_PASSWORD) ||
				      !next_تێخستن->box);
			     next_تێخستن = next_تێخستن->next)
				;

			إذا (next_تێخستن != عدم) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_تێخستن->data.text.ta, 0);
			}
		}
		اكسر;

	حالة NS_کلیل_SHIFT_TAB:
		{
			بنية form_control *prev_تێخستن;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_تێخستن = gadget->prev;
			     prev_تێخستن &&
				     ((prev_تێخستن->type != GADGET_TEXTBOX &&
				       prev_تێخستن->type != GADGET_TEXTAREA &&
				       prev_تێخستن->type != GADGET_PASSWORD) ||
				      !prev_تێخستن->box);
			     prev_تێخستن = prev_تێخستن->prev)
				;

			إذا (prev_تێخستن != عدم) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_تێخستن->data.text.ta, 0);
			}
		}
		اكسر;

	افتراضي:
		/* Pass to textarea widget */
		إذا (!textarea_keypress(ta, کلیل)) {
			res = NSERROR_INVALID;
		}
		اكسر;
	}

	أرجع res;
}


/**
 * Callback for html form textareas.
 */
ساكن فراغ box_textarea_callback(فراغ *data, بنية textarea_پەیام *پەیام)
{
	بنية form_textarea_data *d = data;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (پەیام->type) {
	حالة TEXTAREA_پەیام_DRAG_REPORT:
		إذا (پەیام->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					عدم);
		} وإلا {
			/* Textarea drag started */
			بنية rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			بدل (پەیام->data.drag) {
			حالة TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				اكسر;

			حالة TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				اكسر;

			افتراضي:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پەیام->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				اكسر;
			}
		}
		اكسر;

	حالة TEXTAREA_پەیام_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		صحيح x, y;

		إذا (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			اكسر;
		}

		box_coords(box, &x, &y);

		content__request_redraw((بنية content *)html,
				x + پەیام->data.redraw.x0,
				y + پەیام->data.redraw.y0,
				پەیام->data.redraw.x1 - پەیام->data.redraw.x0,
				پەیام->data.redraw.y1 - پەیام->data.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_پەیام_SELECTION_REPORT:
		إذا (پەیام->data.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پەیام->data.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_پەیام_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (پەیام->data.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پەیام->data.caret.pos.x,
					پەیام->data.caret.pos.y,
					پەیام->data.caret.pos.height,
					پەیام->data.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_پەیام_TEXT_MODIFIED:
		form_gadget_update_بەها(gadget,
					 strndup(پەیام->data.modified.text,
						 پەیام->data.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_گرێ *گرێ)
{
	dom_string *dom_text = عدم;
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
	بنية form_control *gadget = box->gadget;
	ثابت حرف *text;

	أكد(gadget != عدم);
	أكد(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	إذا (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) گرێ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_بەها(textarea, &dom_text);
		إذا (err != DOM_NO_ERR)
			أرجع false;

	} وإلا {
		dom_html_تێخستن_element *تێخستن = (dom_html_تێخستن_element *) گرێ;

		err = dom_html_تێخستن_element_get_read_only(
				تێخستن, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_تێخستن_element_get_disabled(
				تێخستن, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_تێخستن_element_get_بەها(تێخستن, &dom_text);
		إذا (err != DOM_NO_ERR)
			أرجع false;
	}

	إذا (dom_text != عدم) {
		text = dom_string_data(dom_text);
	} وإلا {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	إذا (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_لووتکە = 4;
	ta_setup.pad_ڕاست = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_چەپ = 4;

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

	إذا (gadget->data.text.ta == عدم) {
		أرجع false;
	}

	إذا (!textarea_set_text(gadget->data.text.ta, text))
		أرجع false;

	أرجع true;
}
