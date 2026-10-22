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


nserror box_textarea_keypress(html_content *html, بنية box *box, uint32_t کلید)
{
	بنية form_control *gadget = box->gadget;
	بنية textarea *ta = gadget->داده.متن.ta;
	بنية form* form = box->gadget->form;
	بنية content *c = (بنية content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != عدم);

	إذا (gadget->type == GADGET_TEXTAREA) {
		إذا (textarea_keypress(ta, کلید)) {
			أرجع NSERROR_OK;
		} وإلا {
			أرجع NSERROR_INVALID;
		}
	}

	/* non textarea input */
	بدل (کلید) {
	حالة NS_کلید_NL:
	حالة NS_کلید_CR:
		إذا (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  عدم);
		}
		اكسر;

	حالة NS_کلید_TAB:
		{
			بنية form_control *بعدی_درون_داد;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (بعدی_درون_داد = gadget->بعدی;
			     بعدی_درون_داد &&
				     ((بعدی_درون_داد->type != GADGET_TEXTBOX &&
				       بعدی_درون_داد->type != GADGET_TEXTAREA &&
				       بعدی_درون_داد->type != GADGET_PASSWORD) ||
				      !بعدی_درون_داد->box);
			     بعدی_درون_داد = بعدی_درون_داد->بعدی)
				;

			إذا (بعدی_درون_داد != عدم) {
				textarea_مجموعه_caret(ta, -1);
				textarea_مجموعه_caret(بعدی_درون_داد->داده.متن.ta, 0);
			}
		}
		اكسر;

	حالة NS_کلید_لغزش_TAB:
		{
			بنية form_control *prev_درون_داد;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_درون_داد = gadget->prev;
			     prev_درون_داد &&
				     ((prev_درون_داد->type != GADGET_TEXTBOX &&
				       prev_درون_داد->type != GADGET_TEXTAREA &&
				       prev_درون_داد->type != GADGET_PASSWORD) ||
				      !prev_درون_داد->box);
			     prev_درون_داد = prev_درون_داد->prev)
				;

			إذا (prev_درون_داد != عدم) {
				textarea_مجموعه_caret(ta, -1);
				textarea_مجموعه_caret(prev_درون_داد->داده.متن.ta, 0);
			}
		}
		اكسر;

	افتراضي:
		/* Pass to textarea widget */
		إذا (!textarea_keypress(ta, کلید)) {
			res = NSERROR_INVALID;
		}
		اكسر;
	}

	أرجع res;
}


/**
 * Callback for html form textareas.
 */
ساكن فراغ box_textarea_callback(فراغ *داده, بنية textarea_پیام *پیام)
{
	بنية form_textarea_داده *d = داده;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (پیام->type) {
	حالة TEXTAREA_پیام_DRAG_REPORT:
		إذا (پیام->داده.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_مجموعه_drag_type(html, drag_type, drag_owner,
					عدم);
		} وإلا {
			/* Textarea drag started */
			بنية rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = بیشترین_صحیح,
				.y1 = بیشترین_صحیح
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			بدل (پیام->داده.drag) {
			حالة TEXTAREA_DRAG_SCROLLBAR:
				html_مجموعه_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				اكسر;

			حالة TEXTAREA_DRAG_SELECTION:
				html_مجموعه_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				اكسر;

			افتراضي:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پیام->داده.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				اكسر;
			}
		}
		اكسر;

	حالة TEXTAREA_پیام_REDRAW_REQUEST:
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
				x + پیام->داده.redraw.x0,
				y + پیام->داده.redraw.y0,
				پیام->داده.redraw.x1 - پیام->داده.redraw.x0,
				پیام->داده.redraw.y1 - پیام->داده.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_پیام_SELECTION_REPORT:
		إذا (پیام->داده.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_مجموعه_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پیام->داده.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_مجموعه_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_پیام_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (پیام->داده.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_مجموعه_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_مجموعه_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پیام->داده.caret.pos.x,
					پیام->داده.caret.pos.y,
					پیام->داده.caret.pos.ارتفاع,
					پیام->داده.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_پیام_متن_MODIFIED:
		form_gadget_update_مقدار(gadget,
					 strndup(پیام->داده.modified.متن,
						 پیام->داده.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_گره *گره)
{
	dom_string *dom_متن = عدم;
	dom_exception err;
	textarea_راهاندازی ta_راهاندازی;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.اندازه = 10 * PLOT_STYLE_SCALE,
		.وزن = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	بنية form_control *gadget = box->gadget;
	ثابت حرف *متن;

	أكد(gadget != عدم);
	أكد(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	إذا (gadget->type == GADGET_TEXTAREA) {
		dom_html_متن_area_element *textarea =
				(dom_html_متن_area_element *) گره;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_متن_area_element_get_read_only(
				textarea, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_متن_area_element_get_disabled(
				textarea, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		/* Get the textarea's initial content */
		err = dom_html_متن_area_element_get_مقدار(textarea, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;

	} وإلا {
		dom_html_درون_داد_element *درون_داد = (dom_html_درون_داد_element *) گره;

		err = dom_html_درون_داد_element_get_read_only(
				درون_داد, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_درون_داد_element_get_disabled(
				درون_داد, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_درون_داد_element_get_مقدار(درون_داد, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;
	}

	إذا (dom_متن != عدم) {
		متن = dom_string_داده(dom_متن);
	} وإلا {
		/* No initial text, or failed reading it;
		 * use a blank string */
		متن = "";
	}

	إذا (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->داده.متن.داده.gadget = gadget;

	/* Reset to correct values by layout */
	ta_راهاندازی.عرض = 200;
	ta_راهاندازی.ارتفاع = 20;
	ta_راهاندازی.pad_بالاترین = 4;
	ta_راهاندازی.pad_راست = 4;
	ta_راهاندازی.pad_bottom = 4;
	ta_راهاندازی.pad_چپ = 4;

	/* Set remaining data */
	ta_راهاندازی.border_عرض = 0;
	ta_راهاندازی.border_ستون = 0x000000;
	ta_راهاندازی.متن = fstyle;
	ta_راهاندازی.متن.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_راهاندازی.selected_bg = fstyle.foreground;
	ta_راهاندازی.selected_متن = رنگ_مقصد_bw_furthest(ta_راهاندازی.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->داده.متن.initial = dom_متن;

	gadget->داده.متن.ta = textarea_create(ta_flags, &ta_راهاندازی,
			box_textarea_callback, &gadget->داده.متن.داده);

	إذا (gadget->داده.متن.ta == عدم) {
		أرجع false;
	}

	إذا (!textarea_مجموعه_متن(gadget->داده.متن.ta, متن))
		أرجع false;

	أرجع true;
}
