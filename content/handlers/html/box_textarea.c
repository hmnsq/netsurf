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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/keypress.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/interaction.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, ਬਣਤਰ box *box, uint32_t ਕੁੰਜੀ)
{
	ਬਣਤਰ form_control *gadget = box->gadget;
	ਬਣਤਰ textarea *ta = gadget->ਅੰਕੜੇ.ਪਾਠ.ta;
	ਬਣਤਰ form* form = box->gadget->form;
	ਬਣਤਰ content *c = (ਬਣਤਰ content *)html;
	nserror res = NSERROR_OK;

	ਪ੍ਰਤਿਜ੍ਞਾ(ta != ਨਿਰਾ);

	ਜੇ (gadget->type == GADGET_TEXTAREA) {
		ਜੇ (textarea_keypress(ta, ਕੁੰਜੀ)) {
			ਵਾਪਸ NSERROR_OK;
		} ਨਹੀਂਤਾਂ {
			ਵਾਪਸ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ਚੋਣ (ਕੁੰਜੀ) {
	ਹਾਲਤ NS_ਕੁੰਜੀ_NL:
	ਹਾਲਤ NS_ਕੁੰਜੀ_CR:
		ਜੇ (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ਨਿਰਾ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ NS_ਕੁੰਜੀ_TAB:
		{
			ਬਣਤਰ form_control *ਅਗਲਾ_ਨਿਵੇਸ਼;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ਲਈ (ਅਗਲਾ_ਨਿਵੇਸ਼ = gadget->ਅਗਲਾ;
			     ਅਗਲਾ_ਨਿਵੇਸ਼ &&
				     ((ਅਗਲਾ_ਨਿਵੇਸ਼->type != GADGET_TEXTBOX &&
				       ਅਗਲਾ_ਨਿਵੇਸ਼->type != GADGET_TEXTAREA &&
				       ਅਗਲਾ_ਨਿਵੇਸ਼->type != GADGET_PASSWORD) ||
				      !ਅਗਲਾ_ਨਿਵੇਸ਼->box);
			     ਅਗਲਾ_ਨਿਵੇਸ਼ = ਅਗਲਾ_ਨਿਵੇਸ਼->ਅਗਲਾ)
				;

			ਜੇ (ਅਗਲਾ_ਨਿਵੇਸ਼ != ਨਿਰਾ) {
				textarea_ਸੈੱਟ_caret(ta, -1);
				textarea_ਸੈੱਟ_caret(ਅਗਲਾ_ਨਿਵੇਸ਼->ਅੰਕੜੇ.ਪਾਠ.ta, 0);
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ NS_ਕੁੰਜੀ_ਖਿਸਕਾਓ_TAB:
		{
			ਬਣਤਰ form_control *prev_ਨਿਵੇਸ਼;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ਲਈ (prev_ਨਿਵੇਸ਼ = gadget->prev;
			     prev_ਨਿਵੇਸ਼ &&
				     ((prev_ਨਿਵੇਸ਼->type != GADGET_TEXTBOX &&
				       prev_ਨਿਵੇਸ਼->type != GADGET_TEXTAREA &&
				       prev_ਨਿਵੇਸ਼->type != GADGET_PASSWORD) ||
				      !prev_ਨਿਵੇਸ਼->box);
			     prev_ਨਿਵੇਸ਼ = prev_ਨਿਵੇਸ਼->prev)
				;

			ਜੇ (prev_ਨਿਵੇਸ਼ != ਨਿਰਾ) {
				textarea_ਸੈੱਟ_caret(ta, -1);
				textarea_ਸੈੱਟ_caret(prev_ਨਿਵੇਸ਼->ਅੰਕੜੇ.ਪਾਠ.ta, 0);
			}
		}
		ਰੋਕੋ;

	ਮੂਲ:
		/* Pass to textarea widget */
		ਜੇ (!textarea_keypress(ta, ਕੁੰਜੀ)) {
			res = NSERROR_INVALID;
		}
		ਰੋਕੋ;
	}

	ਵਾਪਸ res;
}


/**
 * Callback for html form textareas.
 */
ਸਥਿਰ ਖਾਲੀ box_textarea_callback(ਖਾਲੀ *ਅੰਕੜੇ, ਬਣਤਰ textarea_ਸੁਨੇਹਾ *ਸੁਨੇਹਾ)
{
	ਬਣਤਰ form_textarea_ਅੰਕੜੇ *d = ਅੰਕੜੇ;
	ਬਣਤਰ form_control *gadget = d->gadget;
	ਬਣਤਰ html_content *html = d->gadget->html;
	ਬਣਤਰ box *box = gadget->box;

	ਚੋਣ (ਸੁਨੇਹਾ->type) {
	ਹਾਲਤ TEXTAREA_ਸੁਨੇਹਾ_DRAG_REPORT:
		ਜੇ (ਸੁਨੇਹਾ->ਅੰਕੜੇ.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			ਸੰਘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_ਸੈੱਟ_drag_type(html, drag_type, drag_owner,
					ਨਿਰਾ);
		} ਨਹੀਂਤਾਂ {
			/* Textarea drag started */
			ਬਣਤਰ rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ,
				.y1 = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ
			};
			ਸੰਘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ਚੋਣ (ਸੁਨੇਹਾ->ਅੰਕੜੇ.drag) {
			ਹਾਲਤ TEXTAREA_DRAG_SCROLLBAR:
				html_ਸੈੱਟ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ਰੋਕੋ;

			ਹਾਲਤ TEXTAREA_DRAG_SELECTION:
				html_ਸੈੱਟ_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ਰੋਕੋ;

			ਮੂਲ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      ਸੁਨੇਹਾ->ਅੰਕੜੇ.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ਪ੍ਰਤਿਜ੍ਞਾ(0);
				ਰੋਕੋ;
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ TEXTAREA_ਸੁਨੇਹਾ_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		ਪੂਰਨ x, y;

		ਜੇ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ਰੋਕੋ;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ਬਣਤਰ content *)html,
				x + ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.x0,
				y + ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.y0,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.x1 - ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.x0,
				ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.y1 - ਸੁਨੇਹਾ->ਅੰਕੜੇ.redraw.y0);
	}
		ਰੋਕੋ;

	ਹਾਲਤ TEXTAREA_ਸੁਨੇਹਾ_SELECTION_REPORT:
		ਜੇ (ਸੁਨੇਹਾ->ਅੰਕੜੇ.selection.have_selection) {
			/* Textarea now has a selection */
			ਸੰਘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_ਸੈੱਟ_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					ਸੁਨੇਹਾ->ਅੰਕੜੇ.selection.read_only);
		} ਨਹੀਂਤਾਂ {
			/* The textarea now has no selection */
			ਸੰਘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_ਸੈੱਟ_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ਰੋਕੋ;

	ਹਾਲਤ TEXTAREA_ਸੁਨੇਹਾ_CARET_UPDATE:
		ਜੇ (html->bw == ਨਿਰਾ)
			ਰੋਕੋ;

		ਜੇ (ਸੁਨੇਹਾ->ਅੰਕੜੇ.caret.type == TEXTAREA_CARET_HIDE) {
			ਸੰਘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_ਸੈੱਟ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ਨਿਰਾ);
		} ਨਹੀਂਤਾਂ {
			ਸੰਘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_ਸੈੱਟ_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					ਸੁਨੇਹਾ->ਅੰਕੜੇ.caret.pos.x,
					ਸੁਨੇਹਾ->ਅੰਕੜੇ.caret.pos.y,
					ਸੁਨੇਹਾ->ਅੰਕੜੇ.caret.pos.ਉਚਾਈ,
					ਸੁਨੇਹਾ->ਅੰਕੜੇ.caret.pos.clip);
		}
		ਰੋਕੋ;

	ਹਾਲਤ TEXTAREA_ਸੁਨੇਹਾ_ਪਾਠ_MODIFIED:
		form_gadget_update_ਮੁੱਲ(gadget,
					 strndup(ਸੁਨੇਹਾ->ਅੰਕੜੇ.modified.ਪਾਠ,
						 ਸੁਨੇਹਾ->ਅੰਕੜੇ.modified.len));
		ਰੋਕੋ;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ਬਣਤਰ box *box, ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ)
{
	dom_string *dom_ਪਾਠ = ਨਿਰਾ;
	dom_exception err;
	textarea_ਤਿਆਰੀ ta_ਤਿਆਰੀ;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.ਮਾਪ = 10 * PLOT_STYLE_SCALE,
		.ਭਾਰ = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	ਬਣਤਰ form_control *gadget = box->gadget;
	ਅਚਲ ਅੱਖਰ *ਪਾਠ;

	ਪ੍ਰਤਿਜ੍ਞਾ(gadget != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	ਜੇ (gadget->type == GADGET_TEXTAREA) {
		dom_html_ਪਾਠ_area_element *textarea =
				(dom_html_ਪਾਠ_area_element *) ਗੰਢ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_ਪਾਠ_area_element_get_read_only(
				textarea, &read_only);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		err = dom_html_ਪਾਠ_area_element_get_disabled(
				textarea, &disabled);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		/* Get the textarea's initial content */
		err = dom_html_ਪਾਠ_area_element_get_ਮੁੱਲ(textarea, &dom_ਪਾਠ);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ {
		dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼ = (dom_html_ਨਿਵੇਸ਼_element *) ਗੰਢ;

		err = dom_html_ਨਿਵੇਸ਼_element_get_read_only(
				ਨਿਵੇਸ਼, &read_only);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		err = dom_html_ਨਿਵੇਸ਼_element_get_disabled(
				ਨਿਵੇਸ਼, &disabled);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		ਜੇ (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		ਨਹੀਂਤਾਂ
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(ਨਿਵੇਸ਼, &dom_ਪਾਠ);
		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;
	}

	ਜੇ (dom_ਪਾਠ != ਨਿਰਾ) {
		ਪਾਠ = dom_string_ਅੰਕੜੇ(dom_ਪਾਠ);
	} ਨਹੀਂਤਾਂ {
		/* No initial text, or failed reading it;
		 * use a blank string */
		ਪਾਠ = "";
	}

	ਜੇ (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->ਅੰਕੜੇ.ਪਾਠ.ਅੰਕੜੇ.gadget = gadget;

	/* Reset to correct values by layout */
	ta_ਤਿਆਰੀ.ਚੌੜਾਈ = 200;
	ta_ਤਿਆਰੀ.ਉਚਾਈ = 20;
	ta_ਤਿਆਰੀ.pad_ਸਿਖਰ = 4;
	ta_ਤਿਆਰੀ.pad_ਸੱਜਾ = 4;
	ta_ਤਿਆਰੀ.pad_bottom = 4;
	ta_ਤਿਆਰੀ.pad_ਖੱਬਾ = 4;

	/* Set remaining data */
	ta_ਤਿਆਰੀ.border_ਚੌੜਾਈ = 0;
	ta_ਤਿਆਰੀ.border_ਖਾਨਾ = 0x000000;
	ta_ਤਿਆਰੀ.ਪਾਠ = fstyle;
	ta_ਤਿਆਰੀ.ਪਾਠ.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_ਤਿਆਰੀ.selected_bg = fstyle.foreground;
	ta_ਤਿਆਰੀ.selected_ਪਾਠ = ਰੰਗ_ਮੰਜ਼ਿਲ_bw_furthest(ta_ਤਿਆਰੀ.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->ਅੰਕੜੇ.ਪਾਠ.initial = dom_ਪਾਠ;

	gadget->ਅੰਕੜੇ.ਪਾਠ.ta = textarea_create(ta_flags, &ta_ਤਿਆਰੀ,
			box_textarea_callback, &gadget->ਅੰਕੜੇ.ਪਾਠ.ਅੰਕੜੇ);

	ਜੇ (gadget->ਅੰਕੜੇ.ਪਾਠ.ta == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	ਜੇ (!textarea_ਸੈੱਟ_ਪਾਠ(gadget->ਅੰਕੜੇ.ਪਾਠ.ta, ਪਾਠ))
		ਵਾਪਸ false;

	ਵਾਪਸ true;
}
