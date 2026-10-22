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


nserror box_textarea_keypress(html_content *html, بنية box *box, uint32_t ڪنجي)
{
	بنية form_control *gadget = box->gadget;
	بنية textarea *ta = gadget->معلومات.متن.ta;
	بنية form* form = box->gadget->form;
	بنية content *c = (بنية content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != عدم);

	إذا (gadget->type == GADGET_TEXTAREA) {
		إذا (textarea_keypress(ta, ڪنجي)) {
			أرجع NSERROR_OK;
		} وإلا {
			أرجع NSERROR_INVALID;
		}
	}

	/* non textarea input */
	بدل (ڪنجي) {
	حالة NS_ڪنجي_NL:
	حالة NS_ڪنجي_CR:
		إذا (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  عدم);
		}
		اكسر;

	حالة NS_ڪنجي_TAB:
		{
			بنية form_control *ايندڙ_داخلا;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (ايندڙ_داخلا = gadget->ايندڙ;
			     ايندڙ_داخلا &&
				     ((ايندڙ_داخلا->type != GADGET_TEXTBOX &&
				       ايندڙ_داخلا->type != GADGET_TEXTAREA &&
				       ايندڙ_داخلا->type != GADGET_PASSWORD) ||
				      !ايندڙ_داخلا->box);
			     ايندڙ_داخلا = ايندڙ_داخلا->ايندڙ)
				;

			إذا (ايندڙ_داخلا != عدم) {
				textarea_سيٽ_caret(ta, -1);
				textarea_سيٽ_caret(ايندڙ_داخلا->معلومات.متن.ta, 0);
			}
		}
		اكسر;

	حالة NS_ڪنجي_سرڪايو_TAB:
		{
			بنية form_control *prev_داخلا;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_داخلا = gadget->prev;
			     prev_داخلا &&
				     ((prev_داخلا->type != GADGET_TEXTBOX &&
				       prev_داخلا->type != GADGET_TEXTAREA &&
				       prev_داخلا->type != GADGET_PASSWORD) ||
				      !prev_داخلا->box);
			     prev_داخلا = prev_داخلا->prev)
				;

			إذا (prev_داخلا != عدم) {
				textarea_سيٽ_caret(ta, -1);
				textarea_سيٽ_caret(prev_داخلا->معلومات.متن.ta, 0);
			}
		}
		اكسر;

	افتراضي:
		/* Pass to textarea widget */
		إذا (!textarea_keypress(ta, ڪنجي)) {
			res = NSERROR_INVALID;
		}
		اكسر;
	}

	أرجع res;
}


/**
 * Callback for html form textareas.
 */
ساكن فراغ box_textarea_callback(فراغ *معلومات, بنية textarea_نياپو *نياپو)
{
	بنية form_textarea_معلومات *d = معلومات;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (نياپو->type) {
	حالة TEXTAREA_نياپو_DRAG_REPORT:
		إذا (نياپو->معلومات.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_سيٽ_drag_type(html, drag_type, drag_owner,
					عدم);
		} وإلا {
			/* Textarea drag started */
			بنية rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = وڌ_کان_وڌ_عدد,
				.y1 = وڌ_کان_وڌ_عدد
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			بدل (نياپو->معلومات.drag) {
			حالة TEXTAREA_DRAG_SCROLLBAR:
				html_سيٽ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				اكسر;

			حالة TEXTAREA_DRAG_SELECTION:
				html_سيٽ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				اكسر;

			افتراضي:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      نياپو->معلومات.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				اكسر;
			}
		}
		اكسر;

	حالة TEXTAREA_نياپو_REDRAW_REQUEST:
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
				x + نياپو->معلومات.redraw.x0,
				y + نياپو->معلومات.redraw.y0,
				نياپو->معلومات.redraw.x1 - نياپو->معلومات.redraw.x0,
				نياپو->معلومات.redraw.y1 - نياپو->معلومات.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_نياپو_SELECTION_REPORT:
		إذا (نياپو->معلومات.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_سيٽ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					نياپو->معلومات.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_سيٽ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_نياپو_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (نياپو->معلومات.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سيٽ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سيٽ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					نياپو->معلومات.caret.pos.x,
					نياپو->معلومات.caret.pos.y,
					نياپو->معلومات.caret.pos.اوچائي,
					نياپو->معلومات.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_نياپو_متن_MODIFIED:
		form_gadget_update_قدر(gadget,
					 strndup(نياپو->معلومات.modified.متن,
						 نياپو->معلومات.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_ڳنڍ *ڳنڍ)
{
	dom_string *dom_متن = عدم;
	dom_exception err;
	textarea_تياري ta_تياري;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.سائيز = 10 * PLOT_STYLE_SCALE,
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
				(dom_html_متن_area_element *) ڳنڍ;
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
		err = dom_html_متن_area_element_get_قدر(textarea, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;

	} وإلا {
		dom_html_داخلا_element *داخلا_2 = (dom_html_داخلا_element *) ڳنڍ;

		err = dom_html_داخلا_element_get_read_only(
				داخلا_2, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_داخلا_element_get_disabled(
				داخلا_2, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_داخلا_element_get_قدر(داخلا_2, &dom_متن);
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
	ta_تياري.ويڪر = 200;
	ta_تياري.اوچائي = 20;
	ta_تياري.pad_چوٽي = 4;
	ta_تياري.pad_ساڄو = 4;
	ta_تياري.pad_bottom = 4;
	ta_تياري.pad_کاٻو = 4;

	/* Set remaining data */
	ta_تياري.border_ويڪر = 0;
	ta_تياري.border_کانو = 0x000000;
	ta_تياري.متن = fstyle;
	ta_تياري.متن.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_تياري.selected_bg = fstyle.foreground;
	ta_تياري.selected_متن = رنگ_منزل_bw_furthest(ta_تياري.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->معلومات.متن.initial = dom_متن;

	gadget->معلومات.متن.ta = textarea_create(ta_flags, &ta_تياري,
			box_textarea_callback, &gadget->معلومات.متن.معلومات);

	إذا (gadget->معلومات.متن.ta == عدم) {
		أرجع false;
	}

	إذا (!textarea_سيٽ_متن(gadget->معلومات.متن.ta, متن))
		أرجع false;

	أرجع true;
}
