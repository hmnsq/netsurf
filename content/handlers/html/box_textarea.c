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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/keypress.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "desktop/textarea.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/interaction.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/box_textarea.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, ഘടന box *box, uint32_t താക്കോൽ)
{
	ഘടന form_control *gadget = box->gadget;
	ഘടന textarea *ta = gadget->data.text.ta;
	ഘടന form* form = box->gadget->form;
	ഘടന content *c = (ഘടന content *)html;
	nserror res = NSERROR_OK;

	പ്രതിജ്ഞാ(ta != ശൂന്യ);

	എങ്കിൽ (gadget->type == GADGET_TEXTAREA) {
		എങ്കിൽ (textarea_keypress(ta, താക്കോൽ)) {
			തിരികെ NSERROR_OK;
		} അല്ലെങ്കിൽ {
			തിരികെ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	തിരഞ്ഞെടുപ്പ് (താക്കോൽ) {
	അവസ്ഥ NS_താക്കോൽ_NL:
	അവസ്ഥ NS_താക്കോൽ_CR:
		എങ്കിൽ (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ശൂന്യ);
		}
		നിർത്തുക;

	അവസ്ഥ NS_താക്കോൽ_TAB:
		{
			ഘടന form_control *next_നിവേശം;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			വേണ്ടി (next_നിവേശം = gadget->next;
			     next_നിവേശം &&
				     ((next_നിവേശം->type != GADGET_TEXTBOX &&
				       next_നിവേശം->type != GADGET_TEXTAREA &&
				       next_നിവേശം->type != GADGET_PASSWORD) ||
				      !next_നിവേശം->box);
			     next_നിവേശം = next_നിവേശം->next)
				;

			എങ്കിൽ (next_നിവേശം != ശൂന്യ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_നിവേശം->data.text.ta, 0);
			}
		}
		നിർത്തുക;

	അവസ്ഥ NS_താക്കോൽ_SHIFT_TAB:
		{
			ഘടന form_control *prev_നിവേശം;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			വേണ്ടി (prev_നിവേശം = gadget->prev;
			     prev_നിവേശം &&
				     ((prev_നിവേശം->type != GADGET_TEXTBOX &&
				       prev_നിവേശം->type != GADGET_TEXTAREA &&
				       prev_നിവേശം->type != GADGET_PASSWORD) ||
				      !prev_നിവേശം->box);
			     prev_നിവേശം = prev_നിവേശം->prev)
				;

			എങ്കിൽ (prev_നിവേശം != ശൂന്യ) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_നിവേശം->data.text.ta, 0);
			}
		}
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		/* Pass to textarea widget */
		എങ്കിൽ (!textarea_keypress(ta, താക്കോൽ)) {
			res = NSERROR_INVALID;
		}
		നിർത്തുക;
	}

	തിരികെ res;
}


/**
 * Callback for html form textareas.
 */
സ്ഥിതം ശൂന്യം box_textarea_callback(ശൂന്യം *data, ഘടന textarea_സന്ദേശം *സന്ദേശം)
{
	ഘടന form_textarea_data *d = data;
	ഘടന form_control *gadget = d->gadget;
	ഘടന html_content *html = d->gadget->html;
	ഘടന box *box = gadget->box;

	തിരഞ്ഞെടുപ്പ് (സന്ദേശം->type) {
	അവസ്ഥ TEXTAREA_സന്ദേശം_DRAG_REPORT:
		എങ്കിൽ (സന്ദേശം->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			സംഘം html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					ശൂന്യ);
		} അല്ലെങ്കിൽ {
			/* Textarea drag started */
			ഘടന rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			സംഘം html_drag_owner drag_owner;
			drag_owner.textarea = box;

			തിരഞ്ഞെടുപ്പ് (സന്ദേശം->data.drag) {
			അവസ്ഥ TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				നിർത്തുക;

			അവസ്ഥ TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				നിർത്തുക;

			സ്ഥിരസ്ഥിതി:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      സന്ദേശം->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				പ്രതിജ്ഞാ(0);
				നിർത്തുക;
			}
		}
		നിർത്തുക;

	അവസ്ഥ TEXTAREA_സന്ദേശം_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		പൂർണ്ണസംഖ്യ x, y;

		എങ്കിൽ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			നിർത്തുക;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ഘടന content *)html,
				x + സന്ദേശം->data.redraw.x0,
				y + സന്ദേശം->data.redraw.y0,
				സന്ദേശം->data.redraw.x1 - സന്ദേശം->data.redraw.x0,
				സന്ദേശം->data.redraw.y1 - സന്ദേശം->data.redraw.y0);
	}
		നിർത്തുക;

	അവസ്ഥ TEXTAREA_സന്ദേശം_SELECTION_REPORT:
		എങ്കിൽ (സന്ദേശം->data.selection.have_selection) {
			/* Textarea now has a selection */
			സംഘം html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					സന്ദേശം->data.selection.read_only);
		} അല്ലെങ്കിൽ {
			/* The textarea now has no selection */
			സംഘം html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		നിർത്തുക;

	അവസ്ഥ TEXTAREA_സന്ദേശം_CARET_UPDATE:
		എങ്കിൽ (html->bw == ശൂന്യ)
			നിർത്തുക;

		എങ്കിൽ (സന്ദേശം->data.caret.type == TEXTAREA_CARET_HIDE) {
			സംഘം html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ശൂന്യ);
		} അല്ലെങ്കിൽ {
			സംഘം html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					സന്ദേശം->data.caret.pos.x,
					സന്ദേശം->data.caret.pos.y,
					സന്ദേശം->data.caret.pos.height,
					സന്ദേശം->data.caret.pos.clip);
		}
		നിർത്തുക;

	അവസ്ഥ TEXTAREA_സന്ദേശം_TEXT_MODIFIED:
		form_gadget_update_മൂല്യം(gadget,
					 strndup(സന്ദേശം->data.modified.text,
						 സന്ദേശം->data.modified.len));
		നിർത്തുക;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ഘടന box *box, ഘടന dom_കണ്ണി *കണ്ണി)
{
	dom_string *dom_text = ശൂന്യ;
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
	ഘടന form_control *gadget = box->gadget;
	സ്ഥിരം അക്ഷരം *text;

	പ്രതിജ്ഞാ(gadget != ശൂന്യ);
	പ്രതിജ്ഞാ(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	എങ്കിൽ (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) കണ്ണി;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_മൂല്യം(textarea, &dom_text);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

	} അല്ലെങ്കിൽ {
		dom_html_നിവേശം_element *നിവേശം = (dom_html_നിവേശം_element *) കണ്ണി;

		err = dom_html_നിവേശം_element_get_read_only(
				നിവേശം, &read_only);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		err = dom_html_നിവേശം_element_get_disabled(
				നിവേശം, &disabled);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		എങ്കിൽ (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		അല്ലെങ്കിൽ
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_നിവേശം_element_get_മൂല്യം(നിവേശം, &dom_text);
		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;
	}

	എങ്കിൽ (dom_text != ശൂന്യ) {
		text = dom_string_data(dom_text);
	} അല്ലെങ്കിൽ {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	എങ്കിൽ (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_മുകൾ = 4;
	ta_setup.pad_വലത് = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_ഇടത് = 4;

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

	എങ്കിൽ (gadget->data.text.ta == ശൂന്യ) {
		തിരികെ false;
	}

	എങ്കിൽ (!textarea_set_text(gadget->data.text.ta, text))
		തിരികെ false;

	തിരികെ true;
}
