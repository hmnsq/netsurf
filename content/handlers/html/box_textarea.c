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
	بنية textarea *ta = gadget->انکڑے.متن.ta;
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
			بنية form_control *اگلا_داخلہ;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (اگلا_داخلہ = gadget->اگلا;
			     اگلا_داخلہ &&
				     ((اگلا_داخلہ->type != GADGET_TEXTBOX &&
				       اگلا_داخلہ->type != GADGET_TEXTAREA &&
				       اگلا_داخلہ->type != GADGET_PASSWORD) ||
				      !اگلا_داخلہ->box);
			     اگلا_داخلہ = اگلا_داخلہ->اگلا)
				;

			إذا (اگلا_داخلہ != عدم) {
				textarea_سیٹ_caret(ta, -1);
				textarea_سیٹ_caret(اگلا_داخلہ->انکڑے.متن.ta, 0);
			}
		}
		اكسر;

	حالة NS_کلید_کھسکاؤ_TAB:
		{
			بنية form_control *prev_داخلہ;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			لكل (prev_داخلہ = gadget->prev;
			     prev_داخلہ &&
				     ((prev_داخلہ->type != GADGET_TEXTBOX &&
				       prev_داخلہ->type != GADGET_TEXTAREA &&
				       prev_داخلہ->type != GADGET_PASSWORD) ||
				      !prev_داخلہ->box);
			     prev_داخلہ = prev_داخلہ->prev)
				;

			إذا (prev_داخلہ != عدم) {
				textarea_سیٹ_caret(ta, -1);
				textarea_سیٹ_caret(prev_داخلہ->انکڑے.متن.ta, 0);
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
ساكن فراغ box_textarea_callback(فراغ *انکڑے, بنية textarea_پیغام *پیغام)
{
	بنية form_textarea_انکڑے *d = انکڑے;
	بنية form_control *gadget = d->gadget;
	بنية html_content *html = d->gadget->html;
	بنية box *box = gadget->box;

	بدل (پیغام->type) {
	حالة TEXTAREA_پیغام_DRAG_REPORT:
		إذا (پیغام->انکڑے.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_سیٹ_drag_type(html, drag_type, drag_owner,
					عدم);
		} وإلا {
			/* Textarea drag started */
			بنية rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = ودھ_توں_ودھ_عدد,
				.y1 = ودھ_توں_ودھ_عدد
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			بدل (پیغام->انکڑے.drag) {
			حالة TEXTAREA_DRAG_SCROLLBAR:
				html_سیٹ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				اكسر;

			حالة TEXTAREA_DRAG_SELECTION:
				html_سیٹ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				اكسر;

			افتراضي:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پیغام->انکڑے.drag);
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
				x + پیغام->انکڑے.redraw.x0,
				y + پیغام->انکڑے.redraw.y0,
				پیغام->انکڑے.redraw.x1 - پیغام->انکڑے.redraw.x0,
				پیغام->انکڑے.redraw.y1 - پیغام->انکڑے.redraw.y0);
	}
		اكسر;

	حالة TEXTAREA_پیغام_SELECTION_REPORT:
		إذا (پیغام->انکڑے.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_سیٹ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پیغام->انکڑے.selection.read_only);
		} وإلا {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_سیٹ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		اكسر;

	حالة TEXTAREA_پیغام_CARET_UPDATE:
		إذا (html->bw == عدم)
			اكسر;

		إذا (پیغام->انکڑے.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سیٹ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, عدم);
		} وإلا {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سیٹ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پیغام->انکڑے.caret.pos.x,
					پیغام->انکڑے.caret.pos.y,
					پیغام->انکڑے.caret.pos.اچائی,
					پیغام->انکڑے.caret.pos.clip);
		}
		اكسر;

	حالة TEXTAREA_پیغام_متن_MODIFIED:
		form_gadget_update_قدر(gadget,
					 strndup(پیغام->انکڑے.modified.متن,
						 پیغام->انکڑے.modified.len));
		اكسر;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		بنية box *box, بنية dom_گرہ *گرہ)
{
	dom_string *dom_متن = عدم;
	dom_exception err;
	textarea_تیاری ta_تیاری;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.ناپ = 10 * PLOT_STYLE_SCALE,
		.بھار = 400,
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
				(dom_html_متن_area_element *) گرہ;
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
		dom_html_داخلہ_element *داخلہ = (dom_html_داخلہ_element *) گرہ;

		err = dom_html_داخلہ_element_get_read_only(
				داخلہ, &read_only);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		err = dom_html_داخلہ_element_get_disabled(
				داخلہ, &disabled);
		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		وإلا
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_داخلہ_element_get_قدر(داخلہ, &dom_متن);
		إذا (err != DOM_NO_ERR)
			أرجع false;
	}

	إذا (dom_متن != عدم) {
		متن = dom_string_انکڑے(dom_متن);
	} وإلا {
		/* No initial text, or failed reading it;
		 * use a blank string */
		متن = "";
	}

	إذا (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->انکڑے.متن.انکڑے.gadget = gadget;

	/* Reset to correct values by layout */
	ta_تیاری.چوڑائی = 200;
	ta_تیاری.اچائی = 20;
	ta_تیاری.pad_چوٹی = 4;
	ta_تیاری.pad_دایاں = 4;
	ta_تیاری.pad_bottom = 4;
	ta_تیاری.pad_بایاں = 4;

	/* Set remaining data */
	ta_تیاری.border_چوڑائی = 0;
	ta_تیاری.border_خانہ = 0x000000;
	ta_تیاری.متن = fstyle;
	ta_تیاری.متن.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_تیاری.selected_bg = fstyle.foreground;
	ta_تیاری.selected_متن = رنگ_منزل_bw_furthest(ta_تیاری.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->انکڑے.متن.initial = dom_متن;

	gadget->انکڑے.متن.ta = textarea_create(ta_flags, &ta_تیاری,
			box_textarea_callback, &gadget->انکڑے.متن.انکڑے);

	إذا (gadget->انکڑے.متن.ta == عدم) {
		أرجع false;
	}

	إذا (!textarea_سیٹ_متن(gadget->انکڑے.متن.ta, متن))
		أرجع false;

	أرجع true;
}
