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

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/keypress.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "desktop/textarea.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/interaction.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/box_textarea.h"
#અન્તર્ભાવ "html/font.h"
#અન્તર્ભાવ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, સંરચના box *box, uint32_t ચાવી)
{
	સંરચના form_control *gadget = box->gadget;
	સંરચના textarea *ta = gadget->data.text.ta;
	સંરચના form* form = box->gadget->form;
	સંરચના content *c = (સંરચના content *)html;
	nserror res = NSERROR_OK;

	પ્રતિજ્ઞા(ta != ખાલી);

	જો (gadget->type == GADGET_TEXTAREA) {
		જો (textarea_keypress(ta, ચાવી)) {
			પાછા NSERROR_OK;
		} નહીંતર {
			પાછા NSERROR_INVALID;
		}
	}

	/* non textarea input */
	પસંદગી (ચાવી) {
	સ્થિતિ NS_ચાવી_NL:
	સ્થિતિ NS_ચાવી_CR:
		જો (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ખાલી);
		}
		તોડો;

	સ્થિતિ NS_ચાવી_TAB:
		{
			સંરચના form_control *next_આગત;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			માટે (next_આગત = gadget->next;
			     next_આગત &&
				     ((next_આગત->type != GADGET_TEXTBOX &&
				       next_આગત->type != GADGET_TEXTAREA &&
				       next_આગત->type != GADGET_PASSWORD) ||
				      !next_આગત->box);
			     next_આગત = next_આગત->next)
				;

			જો (next_આગત != ખાલી) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_આગત->data.text.ta, 0);
			}
		}
		તોડો;

	સ્થિતિ NS_ચાવી_SHIFT_TAB:
		{
			સંરચના form_control *prev_આગત;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			માટે (prev_આગત = gadget->prev;
			     prev_આગત &&
				     ((prev_આગત->type != GADGET_TEXTBOX &&
				       prev_આગત->type != GADGET_TEXTAREA &&
				       prev_આગત->type != GADGET_PASSWORD) ||
				      !prev_આગત->box);
			     prev_આગત = prev_આગત->prev)
				;

			જો (prev_આગત != ખાલી) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_આગત->data.text.ta, 0);
			}
		}
		તોડો;

	મૂળભૂત:
		/* Pass to textarea widget */
		જો (!textarea_keypress(ta, ચાવી)) {
			res = NSERROR_INVALID;
		}
		તોડો;
	}

	પાછા res;
}


/**
 * Callback for html form textareas.
 */
સ્થિર શૂન્ય box_textarea_callback(શૂન્ય *data, સંરચના textarea_સંદેશ *સંદેશ)
{
	સંરચના form_textarea_data *d = data;
	સંરચના form_control *gadget = d->gadget;
	સંરચના html_content *html = d->gadget->html;
	સંરચના box *box = gadget->box;

	પસંદગી (સંદેશ->type) {
	સ્થિતિ TEXTAREA_સંદેશ_DRAG_REPORT:
		જો (સંદેશ->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			સંઘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					ખાલી);
		} નહીંતર {
			/* Textarea drag started */
			સંરચના rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			સંઘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			પસંદગી (સંદેશ->data.drag) {
			સ્થિતિ TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				તોડો;

			સ્થિતિ TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				તોડો;

			મૂળભૂત:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      સંદેશ->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				પ્રતિજ્ઞા(0);
				તોડો;
			}
		}
		તોડો;

	સ્થિતિ TEXTAREA_સંદેશ_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		પૂર્ણાંક x, y;

		જો (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			તોડો;
		}

		box_coords(box, &x, &y);

		content__request_redraw((સંરચના content *)html,
				x + સંદેશ->data.redraw.x0,
				y + સંદેશ->data.redraw.y0,
				સંદેશ->data.redraw.x1 - સંદેશ->data.redraw.x0,
				સંદેશ->data.redraw.y1 - સંદેશ->data.redraw.y0);
	}
		તોડો;

	સ્થિતિ TEXTAREA_સંદેશ_SELECTION_REPORT:
		જો (સંદેશ->data.selection.have_selection) {
			/* Textarea now has a selection */
			સંઘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					સંદેશ->data.selection.read_only);
		} નહીંતર {
			/* The textarea now has no selection */
			સંઘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		તોડો;

	સ્થિતિ TEXTAREA_સંદેશ_CARET_UPDATE:
		જો (html->bw == ખાલી)
			તોડો;

		જો (સંદેશ->data.caret.type == TEXTAREA_CARET_HIDE) {
			સંઘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ખાલી);
		} નહીંતર {
			સંઘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					સંદેશ->data.caret.pos.x,
					સંદેશ->data.caret.pos.y,
					સંદેશ->data.caret.pos.height,
					સંદેશ->data.caret.pos.clip);
		}
		તોડો;

	સ્થિતિ TEXTAREA_સંદેશ_TEXT_MODIFIED:
		form_gadget_update_કિંમત(gadget,
					 strndup(સંદેશ->data.modified.text,
						 સંદેશ->data.modified.len));
		તોડો;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		સંરચના box *box, સંરચના dom_ગાંઠ *ગાંઠ)
{
	dom_string *dom_text = ખાલી;
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
	સંરચના form_control *gadget = box->gadget;
	અચલ અક્ષર *text;

	પ્રતિજ્ઞા(gadget != ખાલી);
	પ્રતિજ્ઞા(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	જો (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) ગાંઠ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		જો (err != DOM_NO_ERR)
			પાછા false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		જો (err != DOM_NO_ERR)
			પાછા false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_કિંમત(textarea, &dom_text);
		જો (err != DOM_NO_ERR)
			પાછા false;

	} નહીંતર {
		dom_html_આગત_element *આગત = (dom_html_આગત_element *) ગાંઠ;

		err = dom_html_આગત_element_get_read_only(
				આગત, &read_only);
		જો (err != DOM_NO_ERR)
			પાછા false;

		err = dom_html_આગત_element_get_disabled(
				આગત, &disabled);
		જો (err != DOM_NO_ERR)
			પાછા false;

		જો (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		નહીંતર
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_આગત_element_get_કિંમત(આગત, &dom_text);
		જો (err != DOM_NO_ERR)
			પાછા false;
	}

	જો (dom_text != ખાલી) {
		text = dom_string_data(dom_text);
	} નહીંતર {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	જો (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_ટોચ = 4;
	ta_setup.pad_જમણું = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_ડાબું = 4;

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

	જો (gadget->data.text.ta == ખાલી) {
		પાછા false;
	}

	જો (!textarea_set_text(gadget->data.text.ta, text))
		પાછા false;

	પાછા true;
}
