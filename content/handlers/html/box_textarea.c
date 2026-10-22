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


nserror box_textarea_keypress(html_content *html, بنية box *box, uint32_t مفتاح)
{
	بنية form_control *gadget = box->gadget;
	بنية textarea *ta = gadget->data.text.ta;
	بنية form* form = box->gadget->form;
	بنية content *c = (بنية content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != عدم);

	إذا (gadget->type == GADGET_TEXTAREA) {
		إذا (textarea_keypress(ta, مفتاح)) {
			إرجاع NSERROR_OK;
		} وإلا {
			إرجاع NSERROR_INVALID;
		}
	}

	/* non textarea input */
	بدل (مفتاح) {
	حالة NS_مفتاح_NL:
	حالة NS_مفتاح_CR:
		إذا (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  عدم);
		}
		اكسر;

	حالة NS_مفتاح_TAB:
		{
			بنية form_control *next_إدخال;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (next_إدخال = gadget->next;
			     next_إدخال &&
				     ((next_إدخال->type != GADGET_TEXTBOX &&
				       next_إدخال->type != GADGET_TEXTAREA &&
				       next_إدخال->type != GADGET_PASSWORD) ||
				      !next_إدخال->box);
			     next_إدخال = next_إدخال->next)
				;

			إذا (next_إدخال != عدم) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_إدخال->data.text.ta, 0);
			}
		}
		اكسر;

	حالة NS_مفتاح_SHIFT_TAB:
		{
			بنية form_control *prev_إدخال;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_إدخال = gadget->prev;
			     prev_إدخال &&
				     ((prev_إدخال->type != GADGET_TEXTBOX &&
				       prev_إدخال->type != GADGET_TEXTAREA &&
				       prev_إدخال->type != GADGET_PASSWORD) ||
				      !prev_إدخال->box);
			     prev_إدخال = prev_إدخال->prev)
				;

			إذا (prev_إدخال != عدم) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_إدخال->data.text.ta, 0);
			}
		}
		اكسر;

	افتراضي:
		/* Pass to textarea widget */
		إذا (!textarea_keypress(ta, مفتاح)) {
			res = NSERROR_INVALID;
		}
		اكسر;
	}

	إرجاع res;
}


/**
 * Callback for html form textareas.
 */
ساكن فراغ box_textarea_callback(فراغ *data, بنية textarea_رسالة *رسالة)
{
	بنية form_textarea_data *d = data;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (رسالة->type) {
	حالة TEXTAREA_رسالة_DRAG_REPORT:
		إذا (رسالة->data.drag == TEXTAREA_DRAG_NONE) {
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

			بدل (رسالة->data.drag) {
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
				      رسالة->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				اكسر;
			}
		}
		اكسر;

	حالة TEXTAREA_رسالة_REDRAW_REQUEST:
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
				x + رسالة->data.redraw.x0,
				y + رسالة->data.redraw.y0,
				رسالة->data.redraw.x1 - رسالة->data.redraw.x0,
				رسالة->data.redraw.y1 - رسالة->data.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_رسالة_SELECTION_REPORT:
		إذا (رسالة->data.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					رسالة->data.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_رسالة_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (رسالة->data.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					رسالة->data.caret.pos.x,
					رسالة->data.caret.pos.y,
					رسالة->data.caret.pos.height,
					رسالة->data.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_رسالة_TEXT_MODIFIED:
		form_gadget_update_قيمة(gadget,
					 strndup(رسالة->data.modified.text,
						 رسالة->data.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_عقدة *عقدة)
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
				(dom_html_text_area_element *) عقدة;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		إذا (err != DOM_NO_ERR)
			إرجاع false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		إذا (err != DOM_NO_ERR)
			إرجاع false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_قيمة(textarea, &dom_text);
		إذا (err != DOM_NO_ERR)
			إرجاع false;

	} وإلا {
		dom_html_إدخال_element *إدخال = (dom_html_إدخال_element *) عقدة;

		err = dom_html_إدخال_element_get_read_only(
				إدخال, &read_only);
		إذا (err != DOM_NO_ERR)
			إرجاع false;

		err = dom_html_إدخال_element_get_disabled(
				إدخال, &disabled);
		إذا (err != DOM_NO_ERR)
			إرجاع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_إدخال_element_get_قيمة(إدخال, &dom_text);
		إذا (err != DOM_NO_ERR)
			إرجاع false;
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
	ta_setup.pad_قمة = 4;
	ta_setup.pad_يمين = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_يسار = 4;

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
		إرجاع false;
	}

	إذا (!textarea_set_text(gadget->data.text.ta, text))
		إرجاع false;

	إرجاع true;
}
