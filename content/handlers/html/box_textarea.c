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

#شامل <string.h>
#شامل <dom/dom.h>

#شامل "utils/config.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/keypress.h"
#شامل "netsurf/misc.h"
#شامل "desktop/textarea.h"

#شامل "html/private.h"
#شامل "html/interaction.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/box_textarea.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, ساختار box *box, uint32_t کلید)
{
	ساختار form_control *gadget = box->gadget;
	ساختار textarea *ta = gadget->data.text.ta;
	ساختار form* form = box->gadget->form;
	ساختار content *c = (ساختار content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != پوچ);

	اگر (gadget->type == GADGET_TEXTAREA) {
		اگر (textarea_keypress(ta, کلید)) {
			بازگشت NSERROR_OK;
		} وگرنه {
			بازگشت NSERROR_INVALID;
		}
	}

	/* non textarea input */
	انتخاب (کلید) {
	حالت NS_کلید_NL:
	حالت NS_کلید_CR:
		اگر (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  پوچ);
		}
		بشکن;

	حالت NS_کلید_TAB:
		{
			ساختار form_control *next_درون_داد;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			برای (next_درون_داد = gadget->next;
			     next_درون_داد &&
				     ((next_درون_داد->type != GADGET_TEXTBOX &&
				       next_درون_داد->type != GADGET_TEXTAREA &&
				       next_درون_داد->type != GADGET_PASSWORD) ||
				      !next_درون_داد->box);
			     next_درون_داد = next_درون_داد->next)
				;

			اگر (next_درون_داد != پوچ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_درون_داد->data.text.ta, 0);
			}
		}
		بشکن;

	حالت NS_کلید_SHIFT_TAB:
		{
			ساختار form_control *prev_درون_داد;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			برای (prev_درون_داد = gadget->prev;
			     prev_درون_داد &&
				     ((prev_درون_داد->type != GADGET_TEXTBOX &&
				       prev_درون_داد->type != GADGET_TEXTAREA &&
				       prev_درون_داد->type != GADGET_PASSWORD) ||
				      !prev_درون_داد->box);
			     prev_درون_داد = prev_درون_داد->prev)
				;

			اگر (prev_درون_داد != پوچ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_درون_داد->data.text.ta, 0);
			}
		}
		بشکن;

	پیشفرض:
		/* Pass to textarea widget */
		اگر (!textarea_keypress(ta, کلید)) {
			res = NSERROR_INVALID;
		}
		بشکن;
	}

	بازگشت res;
}


/**
 * Callback for html form textareas.
 */
ایستا تهی box_textarea_callback(تهی *data, ساختار textarea_پیام *پیام)
{
	ساختار form_textarea_data *d = data;
	ساختار form_control *gadget = d->gadget;
	ساختار html_content *html = d->gadget->html;
	ساختار box *box = gadget->box;

	انتخاب (پیام->type) {
	حالت TEXTAREA_پیام_DRAG_REPORT:
		اگر (پیام->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اجتماع html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					پوچ);
		} وگرنه {
			/* Textarea drag started */
			ساختار rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			اجتماع html_drag_owner drag_owner;
			drag_owner.textarea = box;

			انتخاب (پیام->data.drag) {
			حالت TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				بشکن;

			حالت TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				بشکن;

			پیشفرض:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پیام->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				بشکن;
			}
		}
		بشکن;

	حالت TEXTAREA_پیام_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		صحیح x, y;

		اگر (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			بشکن;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ساختار content *)html,
				x + پیام->data.redraw.x0,
				y + پیام->data.redraw.y0,
				پیام->data.redraw.x1 - پیام->data.redraw.x0,
				پیام->data.redraw.y1 - پیام->data.redraw.y0);
	}
		بشکن;

	حالت TEXTAREA_پیام_SELECTION_REPORT:
		اگر (پیام->data.selection.have_selection) {
			/* Textarea now has a selection */
			اجتماع html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پیام->data.selection.read_only);
		} وگرنه {
			/* The textarea now has no selection */
			اجتماع html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		بشکن;

	حالت TEXTAREA_پیام_CARET_UPDATE:
		اگر (html->bw == پوچ)
			بشکن;

		اگر (پیام->data.caret.type == TEXTAREA_CARET_HIDE) {
			اجتماع html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, پوچ);
		} وگرنه {
			اجتماع html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پیام->data.caret.pos.x,
					پیام->data.caret.pos.y,
					پیام->data.caret.pos.height,
					پیام->data.caret.pos.clip);
		}
		بشکن;

	حالت TEXTAREA_پیام_TEXT_MODIFIED:
		form_gadget_update_مقدار(gadget,
					 strndup(پیام->data.modified.text,
						 پیام->data.modified.len));
		بشکن;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ساختار box *box, ساختار dom_گره *گره)
{
	dom_string *dom_text = پوچ;
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
	ساختار form_control *gadget = box->gadget;
	ثابت نویسه *text;

	أكد(gadget != پوچ);
	أكد(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	اگر (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) گره;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		اگر (err != DOM_NO_ERR)
			بازگشت false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		اگر (err != DOM_NO_ERR)
			بازگشت false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_مقدار(textarea, &dom_text);
		اگر (err != DOM_NO_ERR)
			بازگشت false;

	} وگرنه {
		dom_html_درون_داد_element *درون_داد = (dom_html_درون_داد_element *) گره;

		err = dom_html_درون_داد_element_get_read_only(
				درون_داد, &read_only);
		اگر (err != DOM_NO_ERR)
			بازگشت false;

		err = dom_html_درون_داد_element_get_disabled(
				درون_داد, &disabled);
		اگر (err != DOM_NO_ERR)
			بازگشت false;

		اگر (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وگرنه
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_درون_داد_element_get_مقدار(درون_داد, &dom_text);
		اگر (err != DOM_NO_ERR)
			بازگشت false;
	}

	اگر (dom_text != پوچ) {
		text = dom_string_data(dom_text);
	} وگرنه {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	اگر (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_بالاترین = 4;
	ta_setup.pad_راست = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_چپ = 4;

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

	اگر (gadget->data.text.ta == پوچ) {
		بازگشت false;
	}

	اگر (!textarea_set_text(gadget->data.text.ta, text))
		بازگشت false;

	بازگشت true;
}
