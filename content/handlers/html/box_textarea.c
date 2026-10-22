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

#अन्तर्भाव <string.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/keypress.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "desktop/textarea.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/interaction.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/box_textarea.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, संरचना box *box, uint32_t किल्ली)
{
	संरचना form_control *gadget = box->gadget;
	संरचना textarea *ta = gadget->data.text.ta;
	संरचना form* form = box->gadget->form;
	संरचना content *c = (संरचना content *)html;
	nserror res = NSERROR_OK;

	प्रतिज्ञा(ta != रिक्त);

	यदि (gadget->type == GADGET_TEXTAREA) {
		यदि (textarea_keypress(ta, किल्ली)) {
			प्रत्यागम NSERROR_OK;
		} अन्यथा {
			प्रत्यागम NSERROR_INVALID;
		}
	}

	/* non textarea input */
	विकल्प (किल्ली) {
	पक्ष NS_किल्ली_NL:
	पक्ष NS_किल्ली_CR:
		यदि (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  रिक्त);
		}
		भङ्ग;

	पक्ष NS_किल्ली_TAB:
		{
			संरचना form_control *next_आदान;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			प्रति (next_आदान = gadget->next;
			     next_आदान &&
				     ((next_आदान->type != GADGET_TEXTBOX &&
				       next_आदान->type != GADGET_TEXTAREA &&
				       next_आदान->type != GADGET_PASSWORD) ||
				      !next_आदान->box);
			     next_आदान = next_आदान->next)
				;

			यदि (next_आदान != रिक्त) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_आदान->data.text.ta, 0);
			}
		}
		भङ्ग;

	पक्ष NS_किल्ली_SHIFT_TAB:
		{
			संरचना form_control *prev_आदान;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			प्रति (prev_आदान = gadget->prev;
			     prev_आदान &&
				     ((prev_आदान->type != GADGET_TEXTBOX &&
				       prev_आदान->type != GADGET_TEXTAREA &&
				       prev_आदान->type != GADGET_PASSWORD) ||
				      !prev_आदान->box);
			     prev_आदान = prev_आदान->prev)
				;

			यदि (prev_आदान != रिक्त) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_आदान->data.text.ta, 0);
			}
		}
		भङ्ग;

	स्वभाव:
		/* Pass to textarea widget */
		यदि (!textarea_keypress(ta, किल्ली)) {
			res = NSERROR_INVALID;
		}
		भङ्ग;
	}

	प्रत्यागम res;
}


/**
 * Callback for html form textareas.
 */
स्थिर शून्य box_textarea_callback(शून्य *data, संरचना textarea_संदेश *संदेश)
{
	संरचना form_textarea_data *d = data;
	संरचना form_control *gadget = d->gadget;
	संरचना html_content *html = d->gadget->html;
	संरचना box *box = gadget->box;

	विकल्प (संदेश->type) {
	पक्ष TEXTAREA_संदेश_DRAG_REPORT:
		यदि (संदेश->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			सङ्घ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					रिक्त);
		} अन्यथा {
			/* Textarea drag started */
			संरचना rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = INT_MAX,
				.y1 = INT_MAX
			};
			सङ्घ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			विकल्प (संदेश->data.drag) {
			पक्ष TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				भङ्ग;

			पक्ष TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				भङ्ग;

			स्वभाव:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      संदेश->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				प्रतिज्ञा(0);
				भङ्ग;
			}
		}
		भङ्ग;

	पक्ष TEXTAREA_संदेश_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		पूर्णाङ्क x, y;

		यदि (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			भङ्ग;
		}

		box_coords(box, &x, &y);

		content__request_redraw((संरचना content *)html,
				x + संदेश->data.redraw.x0,
				y + संदेश->data.redraw.y0,
				संदेश->data.redraw.x1 - संदेश->data.redraw.x0,
				संदेश->data.redraw.y1 - संदेश->data.redraw.y0);
	}
		भङ्ग;

	पक्ष TEXTAREA_संदेश_SELECTION_REPORT:
		यदि (संदेश->data.selection.have_selection) {
			/* Textarea now has a selection */
			सङ्घ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					संदेश->data.selection.read_only);
		} अन्यथा {
			/* The textarea now has no selection */
			सङ्घ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		भङ्ग;

	पक्ष TEXTAREA_संदेश_CARET_UPDATE:
		यदि (html->bw == रिक्त)
			भङ्ग;

		यदि (संदेश->data.caret.type == TEXTAREA_CARET_HIDE) {
			सङ्घ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, रिक्त);
		} अन्यथा {
			सङ्घ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					संदेश->data.caret.pos.x,
					संदेश->data.caret.pos.y,
					संदेश->data.caret.pos.height,
					संदेश->data.caret.pos.clip);
		}
		भङ्ग;

	पक्ष TEXTAREA_संदेश_TEXT_MODIFIED:
		form_gadget_update_मूल्य(gadget,
					 strndup(संदेश->data.modified.text,
						 संदेश->data.modified.len));
		भङ्ग;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		संरचना box *box, संरचना dom_गाठ *गाठ)
{
	dom_string *dom_text = रिक्त;
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
	संरचना form_control *gadget = box->gadget;
	ध्रुव वर्ण *text;

	प्रतिज्ञा(gadget != रिक्त);
	प्रतिज्ञा(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	यदि (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) गाठ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_मूल्य(textarea, &dom_text);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

	} अन्यथा {
		dom_html_आदान_element *आदान = (dom_html_आदान_element *) गाठ;

		err = dom_html_आदान_element_get_read_only(
				आदान, &read_only);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		err = dom_html_आदान_element_get_disabled(
				आदान, &disabled);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		यदि (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		अन्यथा
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_आदान_element_get_मूल्य(आदान, &dom_text);
		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;
	}

	यदि (dom_text != रिक्त) {
		text = dom_string_data(dom_text);
	} अन्यथा {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	यदि (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->data.text.data.gadget = gadget;

	/* Reset to correct values by layout */
	ta_setup.width = 200;
	ta_setup.height = 20;
	ta_setup.pad_शीर्ष = 4;
	ta_setup.pad_उजवा = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_डावा = 4;

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

	यदि (gadget->data.text.ta == रिक्त) {
		प्रत्यागम false;
	}

	यदि (!textarea_set_text(gadget->data.text.ta, text))
		प्रत्यागम false;

	प्रत्यागम true;
}
