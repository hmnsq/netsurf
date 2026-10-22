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


nserror box_textarea_keypress(html_content *html, ساخت box *box, uint32_t کلید)
{
	ساخت form_control *gadget = box->gadget;
	ساخت textarea *ta = gadget->ڈیٹا.متن.ta;
	ساخت form* form = box->gadget->form;
	ساخت content *c = (ساخت content *)html;
	nserror res = NSERROR_OK;

	أكد(ta != تہی);

	اگر (gadget->type == GADGET_TEXTAREA) {
		اگر (textarea_keypress(ta, کلید)) {
			واپس NSERROR_OK;
		} ورنہ {
			واپس NSERROR_INVALID;
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
					  تہی);
		}
		توڑو;

	حالت NS_کلید_TAB:
		{
			ساخت form_control *اگلا_داخلہ;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			برائے (اگلا_داخلہ = gadget->اگلا;
			     اگلا_داخلہ &&
				     ((اگلا_داخلہ->type != GADGET_TEXTBOX &&
				       اگلا_داخلہ->type != GADGET_TEXTAREA &&
				       اگلا_داخلہ->type != GADGET_PASSWORD) ||
				      !اگلا_داخلہ->box);
			     اگلا_داخلہ = اگلا_داخلہ->اگلا)
				;

			اگر (اگلا_داخلہ != تہی) {
				textarea_سیٹ_caret(ta, -1);
				textarea_سیٹ_caret(اگلا_داخلہ->ڈیٹا.متن.ta, 0);
			}
		}
		توڑو;

	حالت NS_کلید_سرکاؤ_TAB:
		{
			ساخت form_control *پچھلا_داخلہ;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			برائے (پچھلا_داخلہ = gadget->پچھلا;
			     پچھلا_داخلہ &&
				     ((پچھلا_داخلہ->type != GADGET_TEXTBOX &&
				       پچھلا_داخلہ->type != GADGET_TEXTAREA &&
				       پچھلا_داخلہ->type != GADGET_PASSWORD) ||
				      !پچھلا_داخلہ->box);
			     پچھلا_داخلہ = پچھلا_داخلہ->پچھلا)
				;

			اگر (پچھلا_داخلہ != تہی) {
				textarea_سیٹ_caret(ta, -1);
				textarea_سیٹ_caret(پچھلا_داخلہ->ڈیٹا.متن.ta, 0);
			}
		}
		توڑو;

	طےشدہ:
		/* Pass to textarea widget */
		اگر (!textarea_keypress(ta, کلید)) {
			res = NSERROR_INVALID;
		}
		توڑو;
	}

	واپس res;
}


/**
 * Callback for html form textareas.
 */
ساکن خالی box_textarea_callback(خالی *ڈیٹا, ساخت textarea_پیغام *پیغام)
{
	ساخت form_textarea_ڈیٹا *d = ڈیٹا;
	ساخت form_control *gadget = d->gadget;
	ساخت html_content *html = d->gadget->html;
	ساخت box *box = gadget->box;

	انتخاب (پیغام->type) {
	حالت TEXTAREA_پیغام_DRAG_REPORT:
		اگر (پیغام->ڈیٹا.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			اتحاد html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_سیٹ_drag_type(html, drag_type, drag_owner,
					تہی);
		} ورنہ {
			/* Textarea drag started */
			ساخت rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = زیادہ_سے_زیادہ_عدد,
				.y1 = زیادہ_سے_زیادہ_عدد
			};
			اتحاد html_drag_owner drag_owner;
			drag_owner.textarea = box;

			انتخاب (پیغام->ڈیٹا.drag) {
			حالت TEXTAREA_DRAG_SCROLLBAR:
				html_سیٹ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				توڑو;

			حالت TEXTAREA_DRAG_SELECTION:
				html_سیٹ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				توڑو;

			طےشدہ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      پیغام->ڈیٹا.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				أكد(0);
				توڑو;
			}
		}
		توڑو;

	حالت TEXTAREA_پیغام_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		صحیح x, y;

		اگر (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			توڑو;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ساخت content *)html,
				x + پیغام->ڈیٹا.redraw.x0,
				y + پیغام->ڈیٹا.redraw.y0,
				پیغام->ڈیٹا.redraw.x1 - پیغام->ڈیٹا.redraw.x0,
				پیغام->ڈیٹا.redraw.y1 - پیغام->ڈیٹا.redraw.y0);
	}
		توڑو;

	حالت TEXTAREA_پیغام_SELECTION_REPORT:
		اگر (پیغام->ڈیٹا.selection.have_selection) {
			/* Textarea now has a selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_سیٹ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					پیغام->ڈیٹا.selection.read_only);
		} ورنہ {
			/* The textarea now has no selection */
			اتحاد html_selection_owner sel_owner;
			sel_owner.none = true;

			html_سیٹ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		توڑو;

	حالت TEXTAREA_پیغام_CARET_UPDATE:
		اگر (html->bw == تہی)
			توڑو;

		اگر (پیغام->ڈیٹا.caret.type == TEXTAREA_CARET_HIDE) {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سیٹ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, تہی);
		} ورنہ {
			اتحاد html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_سیٹ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					پیغام->ڈیٹا.caret.pos.x,
					پیغام->ڈیٹا.caret.pos.y,
					پیغام->ڈیٹا.caret.pos.اونچائی,
					پیغام->ڈیٹا.caret.pos.clip);
		}
		توڑو;

	حالت TEXTAREA_پیغام_متن_MODIFIED:
		form_gadget_update_قدر(gadget,
					 strndup(پیغام->ڈیٹا.modified.متن,
						 پیغام->ڈیٹا.modified.len));
		توڑو;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ساخت box *box, ساخت dom_گرہ *گرہ)
{
	dom_string *dom_متن = تہی;
	dom_exception err;
	textarea_تیاری ta_تیاری;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.سائز = 10 * PLOT_STYLE_SCALE,
		.وزن = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	ساخت form_control *gadget = box->gadget;
	ثابت حرف *متن;

	أكد(gadget != تہی);
	أكد(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	اگر (gadget->type == GADGET_TEXTAREA) {
		dom_html_متن_area_element *textarea =
				(dom_html_متن_area_element *) گرہ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_متن_area_element_get_read_only(
				textarea, &read_only);
		اگر (err != DOM_NO_ERR)
			واپس false;

		err = dom_html_متن_area_element_get_disabled(
				textarea, &disabled);
		اگر (err != DOM_NO_ERR)
			واپس false;

		/* Get the textarea's initial content */
		err = dom_html_متن_area_element_get_قدر(textarea, &dom_متن);
		اگر (err != DOM_NO_ERR)
			واپس false;

	} ورنہ {
		dom_html_داخلہ_element *داخلہ = (dom_html_داخلہ_element *) گرہ;

		err = dom_html_داخلہ_element_get_read_only(
				داخلہ, &read_only);
		اگر (err != DOM_NO_ERR)
			واپس false;

		err = dom_html_داخلہ_element_get_disabled(
				داخلہ, &disabled);
		اگر (err != DOM_NO_ERR)
			واپس false;

		اگر (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		ورنہ
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_داخلہ_element_get_قدر(داخلہ, &dom_متن);
		اگر (err != DOM_NO_ERR)
			واپس false;
	}

	اگر (dom_متن != تہی) {
		متن = dom_string_ڈیٹا(dom_متن);
	} ورنہ {
		/* No initial text, or failed reading it;
		 * use a blank string */
		متن = "";
	}

	اگر (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->ڈیٹا.متن.ڈیٹا.gadget = gadget;

	/* Reset to correct values by layout */
	ta_تیاری.چوڑائی = 200;
	ta_تیاری.اونچائی = 20;
	ta_تیاری.pad_چوٹی = 4;
	ta_تیاری.pad_دایاں = 4;
	ta_تیاری.pad_bottom = 4;
	ta_تیاری.pad_بایاں = 4;

	/* Set remaining data */
	ta_تیاری.border_چوڑائی = 0;
	ta_تیاری.border_ستون = 0x000000;
	ta_تیاری.متن = fstyle;
	ta_تیاری.متن.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_تیاری.selected_bg = fstyle.foreground;
	ta_تیاری.selected_متن = رنگ_مقصد_bw_furthest(ta_تیاری.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->ڈیٹا.متن.initial = dom_متن;

	gadget->ڈیٹا.متن.ta = textarea_create(ta_flags, &ta_تیاری,
			box_textarea_callback, &gadget->ڈیٹا.متن.ڈیٹا);

	اگر (gadget->ڈیٹا.متن.ta == تہی) {
		واپس false;
	}

	اگر (!textarea_سیٹ_متن(gadget->ڈیٹا.متن.ta, متن))
		واپس false;

	واپس true;
}
