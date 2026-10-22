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


nserror box_textarea_keypress(html_content *html, بنية box *box, uint32_t کلی)
{
	بنية form_control *gadget = box->gadget;
	بنية textarea *ta = gadget->معلومات.متن.ta;
	بنية form* form = box->gadget->form;
	بنية content *c = (بنية content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != عدم);

	إذا (gadget->type == GADGET_TEXTAREA) {
		إذا (textarea_keypress(ta, کلی)) {
			أرجع NSERROR_OK;
		} وإلا {
			أرجع NSERROR_INVALID;
		}
	}

	/* non textarea input */
	بدل (کلی) {
	حالة NS_کلی_NL:
	حالة NS_کلی_CR:
		إذا (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  عدم);
		}
		اكسر;

	حالة NS_کلی_TAB:
		{
			بنية form_control *بل_ننوت;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (بل_ننوت = gadget->بل;
			     بل_ننوت &&
				     ((بل_ننوت->type != GADGET_TEXTBOX &&
				       بل_ننوت->type != GADGET_TEXTAREA &&
				       بل_ننوت->type != GADGET_PASSWORD) ||
				      !بل_ننوت->box);
			     بل_ننوت = بل_ننوت->بل)
				;

			إذا (بل_ننوت != عدم) {
				textarea_سېټ_caret(ta, -1);
				textarea_سېټ_caret(بل_ننوت->معلومات.متن.ta, 0);
			}
		}
		اكسر;

	حالة NS_کلی_ښويدنه_TAB:
		{
			بنية form_control *prev_ننوت;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_ننوت = gadget->prev;
			     prev_ننوت &&
				     ((prev_ننوت->type != GADGET_TEXTBOX &&
				       prev_ننوت->type != GADGET_TEXTAREA &&
				       prev_ننوت->type != GADGET_PASSWORD) ||
				      !prev_ننوت->box);
			     prev_ننوت = prev_ننوت->prev)
				;

			إذا (prev_ننوت != عدم) {
				textarea_سېټ_caret(ta, -1);
				textarea_سېټ_caret(prev_ننوت->معلومات.متن.ta, 0);
			}
		}
		اكسر;

	افتراضي:
		/* Pass to textarea widget */
		إذا (!textarea_keypress(ta, کلی)) {
			res = NSERROR_INVALID;
		}
		اكسر;
	}

	أرجع res;
}


/**
 * Callback for html form textareas.
 */
ساكن فراغ box_textarea_callback(فراغ *معلومات, بنية textarea_پیغام *پیغام)
{
	بنية form_textarea_معلومات *d = معلومات;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (پیغام->type) {
	حالة TEXTAREA_پیغام_DRAG_REPORT:
		إذا (پیغام->معلومات.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_سېټ_drag_type(html, drag_type, drag_owner,
					عدم);
		} وإلا {
			/* Textarea drag started */
			بنية rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = لوی_عدد,
				.y1 = لوی_عدد
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			بدل (پیغام->معلومات.drag) {
			حالة TEXTAREA_DRAG_SCROLLBAR:
				html_سېټ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				اكسر;

			حالة TEXTAREA_DRAG_SELECTION:
				html_سېټ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				اكسر;

			افتراضي:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پیغام->معلومات.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				اكسر;
			}
		}
		اكسر;

	حالة TEXTAREA_پیغام_REDRAW_REQUEST:
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
				x + پیغام->معلومات.redraw.x0,
				y + پیغام->معلومات.redraw.y0,
				پیغام->معلومات.redraw.x1 - پیغام->معلومات.redraw.x0,
				پیغام->معلومات.redraw.y1 - پیغام->معلومات.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_پیغام_SELECTION_REPORT:
		إذا (پیغام->معلومات.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_سېټ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پیغام->معلومات.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_سېټ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_پیغام_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (پیغام->معلومات.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سېټ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سېټ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پیغام->معلومات.caret.pos.x,
					پیغام->معلومات.caret.pos.y,
					پیغام->معلومات.caret.pos.لوړوالی,
					پیغام->معلومات.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_پیغام_متن_MODIFIED:
		form_gadget_update_ارزښت(gadget,
					 strndup(پیغام->معلومات.modified.متن,
						 پیغام->معلومات.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_غوټه *غوټه)
{
	dom_string *dom_متن = عدم;
	dom_exception err;
	textarea_تیاری ta_تیاری;
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
				(dom_html_متن_area_element *) غوټه;
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
		err = dom_html_متن_area_element_get_ارزښت(textarea, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;

	} وإلا {
		dom_html_ننوت_element *ننوت = (dom_html_ننوت_element *) غوټه;

		err = dom_html_ننوت_element_get_read_only(
				ننوت, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_ننوت_element_get_disabled(
				ننوت, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ننوت_element_get_ارزښت(ننوت, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;
	}

	إذا (dom_متن != عدم) {
		متن = dom_string_معلومات(dom_متن);
	} وإلا {
		/* No initial text, or failed reading it;
		 * use a blank string */
		متن = "";
	}

	إذا (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->معلومات.متن.معلومات.gadget = gadget;

	/* Reset to correct values by layout */
	ta_تیاری.پلنوالی = 200;
	ta_تیاری.لوړوالی = 20;
	ta_تیاری.pad_سر = 4;
	ta_تیاری.pad_ښی = 4;
	ta_تیاری.pad_bottom = 4;
	ta_تیاری.pad_کیڼ = 4;

	/* Set remaining data */
	ta_تیاری.border_پلنوالی = 0;
	ta_تیاری.border_ستون = 0x000000;
	ta_تیاری.متن = fstyle;
	ta_تیاری.متن.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_تیاری.selected_bg = fstyle.foreground;
	ta_تیاری.selected_متن = رنګ_منزل_bw_furthest(ta_تیاری.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->معلومات.متن.initial = dom_متن;

	gadget->معلومات.متن.ta = textarea_create(ta_flags, &ta_تیاری,
			box_textarea_callback, &gadget->معلومات.متن.معلومات);

	إذا (gadget->معلومات.متن.ta == عدم) {
		أرجع false;
	}

	إذا (!textarea_سېټ_متن(gadget->معلومات.متن.ta, متن))
		أرجع false;

	أرجع true;
}
