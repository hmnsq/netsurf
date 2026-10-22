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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/keypress.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/interaction.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/box_textarea.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, ସଂରଚନା box *box, uint32_t ଚାବି)
{
	ସଂରଚନା form_control *gadget = box->gadget;
	ସଂରଚନା textarea *ta = gadget->data.text.ta;
	ସଂରଚନା form* form = box->gadget->form;
	ସଂରଚନା content *c = (ସଂରଚନା content *)html;
	nserror res = NSERROR_OK;

	ପ୍ରତିଜ୍ଞା(ta != ଖାଲି);

	ଯଦି (gadget->type == GADGET_TEXTAREA) {
		ଯଦି (textarea_keypress(ta, ଚାବି)) {
			ଫେରନ୍ତୁ NSERROR_OK;
		} ଅନ୍ୟଥା {
			ଫେରନ୍ତୁ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ବାଛ (ଚାବି) {
	ସ୍ଥିତି NS_ଚାବି_NL:
	ସ୍ଥିତି NS_ଚାବି_CR:
		ଯଦି (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ଖାଲି);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି NS_ଚାବି_TAB:
		{
			ସଂରଚନା form_control *next_ନିବେଶ;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ପାଇଁ (next_ନିବେଶ = gadget->next;
			     next_ନିବେଶ &&
				     ((next_ନିବେଶ->type != GADGET_TEXTBOX &&
				       next_ନିବେଶ->type != GADGET_TEXTAREA &&
				       next_ନିବେଶ->type != GADGET_PASSWORD) ||
				      !next_ନିବେଶ->box);
			     next_ନିବେଶ = next_ନିବେଶ->next)
				;

			ଯଦି (next_ନିବେଶ != ଖାଲି) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_ନିବେଶ->data.text.ta, 0);
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି NS_ଚାବି_SHIFT_TAB:
		{
			ସଂରଚନା form_control *prev_ନିବେଶ;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			ପାଇଁ (prev_ନିବେଶ = gadget->prev;
			     prev_ନିବେଶ &&
				     ((prev_ନିବେଶ->type != GADGET_TEXTBOX &&
				       prev_ନିବେଶ->type != GADGET_TEXTAREA &&
				       prev_ନିବେଶ->type != GADGET_PASSWORD) ||
				      !prev_ନିବେଶ->box);
			     prev_ନିବେଶ = prev_ନିବେଶ->prev)
				;

			ଯଦି (prev_ନିବେଶ != ଖାଲି) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_ନିବେଶ->data.text.ta, 0);
			}
		}
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Pass to textarea widget */
		ଯଦି (!textarea_keypress(ta, ଚାବି)) {
			res = NSERROR_INVALID;
		}
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ res;
}


/**
 * Callback for html form textareas.
 */
ସ୍ଥିର ଶୂନ୍ୟ box_textarea_callback(ଶୂନ୍ୟ *data, ସଂରଚନା textarea_ସନ୍ଦେଶ *ସନ୍ଦେଶ)
{
	ସଂରଚନା form_textarea_data *d = data;
	ସଂରଚନା form_control *gadget = d->gadget;
	ସଂରଚନା html_content *html = d->gadget->html;
	ସଂରଚନା box *box = gadget->box;

	ବାଛ (ସନ୍ଦେଶ->type) {
	ସ୍ଥିତି TEXTAREA_ସନ୍ଦେଶ_DRAG_REPORT:
		ଯଦି (ସନ୍ଦେଶ->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			ସଂଘ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					ଖାଲି);
		} ଅନ୍ୟଥା {
			/* Textarea drag started */
			ସଂରଚନା rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			ସଂଘ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ବାଛ (ସନ୍ଦେଶ->data.drag) {
			ସ୍ଥିତି TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ଭାଙ୍ଗ;

			ସ୍ଥିତି TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ଭାଙ୍ଗ;

			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      ସନ୍ଦେଶ->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ପ୍ରତିଜ୍ଞା(0);
				ଭାଙ୍ଗ;
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି TEXTAREA_ସନ୍ଦେଶ_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;

		ଯଦି (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ଭାଙ୍ଗ;
		}

		box_coords(box, &x, &y);

		content__request_redraw((ସଂରଚନା content *)html,
				x + ସନ୍ଦେଶ->data.redraw.x0,
				y + ସନ୍ଦେଶ->data.redraw.y0,
				ସନ୍ଦେଶ->data.redraw.x1 - ସନ୍ଦେଶ->data.redraw.x0,
				ସନ୍ଦେଶ->data.redraw.y1 - ସନ୍ଦେଶ->data.redraw.y0);
	}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି TEXTAREA_ସନ୍ଦେଶ_SELECTION_REPORT:
		ଯଦି (ସନ୍ଦେଶ->data.selection.have_selection) {
			/* Textarea now has a selection */
			ସଂଘ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					ସନ୍ଦେଶ->data.selection.read_only);
		} ଅନ୍ୟଥା {
			/* The textarea now has no selection */
			ସଂଘ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି TEXTAREA_ସନ୍ଦେଶ_CARET_UPDATE:
		ଯଦି (html->bw == ଖାଲି)
			ଭାଙ୍ଗ;

		ଯଦି (ସନ୍ଦେଶ->data.caret.type == TEXTAREA_CARET_HIDE) {
			ସଂଘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ଖାଲି);
		} ଅନ୍ୟଥା {
			ସଂଘ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					ସନ୍ଦେଶ->data.caret.pos.x,
					ସନ୍ଦେଶ->data.caret.pos.y,
					ସନ୍ଦେଶ->data.caret.pos.height,
					ସନ୍ଦେଶ->data.caret.pos.clip);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି TEXTAREA_ସନ୍ଦେଶ_TEXT_MODIFIED:
		form_gadget_update_ମୂଲ୍ୟ(gadget,
					 strndup(ସନ୍ଦେଶ->data.modified.text,
						 ସନ୍ଦେଶ->data.modified.len));
		ଭାଙ୍ଗ;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		ସଂରଚନା box *box, ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_string *dom_text = ଖାଲି;
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
	ସଂରଚନା form_control *gadget = box->gadget;
	ଅଚଳ ଅକ୍ଷର *text;

	ପ୍ରତିଜ୍ଞା(gadget != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	ଯଦି (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) ଗଣ୍ଠି;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_ମୂଲ୍ୟ(textarea, &dom_text);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା {
		dom_html_ନିବେଶ_element *ନିବେଶ = (dom_html_ନିବେଶ_element *) ଗଣ୍ଠି;

		err = dom_html_ନିବେଶ_element_get_read_only(
				ନିବେଶ, &read_only);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		err = dom_html_ନିବେଶ_element_get_disabled(
				ନିବେଶ, &disabled);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		ଯଦି (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		ଅନ୍ୟଥା
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(ନିବେଶ, &dom_text);
		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;
	}

	ଯଦି (dom_text != ଖାଲି) {
		text = dom_string_data(dom_text);
	} ଅନ୍ୟଥା {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	ଯଦି (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_ଶୀର୍ଷ = 4;
	ta_setup.pad_ଡାହାଣ = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_ବାମ = 4;

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

	ଯଦି (gadget->data.text.ta == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!textarea_set_text(gadget->data.text.ta, text))
		ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ true;
}
