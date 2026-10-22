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


nserror box_textarea_keypress(html_content *html, संरचना box *box, uint32_t साँचो)
{
	संरचना form_control *gadget = box->gadget;
	संरचना textarea *ta = gadget->data.text.ta;
	संरचना form* form = box->gadget->form;
	संरचना content *c = (संरचना content *)html;
	nserror res = NSERROR_OK;

	प्रतिज्ञा(ta != रिक्त);

	यदि (gadget->type == GADGET_TEXTAREA) {
		यदि (textarea_keypress(ta, साँचो)) {
			फर्काऊ NSERROR_OK;
		} अन्यथा {
			फर्काऊ NSERROR_INVALID;
		}
	}

	/* non textarea input */
	विकल्प (साँचो) {
	पक्ष NS_साँचो_NL:
	पक्ष NS_साँचो_CR:
		यदि (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  रिक्त);
		}
		रोक;

	पक्ष NS_साँचो_TAB:
		{
			संरचना form_control *next_आगत;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			लागि (next_आगत = gadget->next;
			     next_आगत &&
				     ((next_आगत->type != GADGET_TEXTBOX &&
				       next_आगत->type != GADGET_TEXTAREA &&
				       next_आगत->type != GADGET_PASSWORD) ||
				      !next_आगत->box);
			     next_आगत = next_आगत->next)
				;

			यदि (next_आगत != रिक्त) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(next_आगत->data.text.ta, 0);
			}
		}
		रोक;

	पक्ष NS_साँचो_SHIFT_TAB:
		{
			संरचना form_control *prev_आगत;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			लागि (prev_आगत = gadget->prev;
			     prev_आगत &&
				     ((prev_आगत->type != GADGET_TEXTBOX &&
				       prev_आगत->type != GADGET_TEXTAREA &&
				       prev_आगत->type != GADGET_PASSWORD) ||
				      !prev_आगत->box);
			     prev_आगत = prev_आगत->prev)
				;

			यदि (prev_आगत != रिक्त) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(prev_आगत->data.text.ta, 0);
			}
		}
		रोक;

	स्वभाव:
		/* Pass to textarea widget */
		यदि (!textarea_keypress(ta, साँचो)) {
			res = NSERROR_INVALID;
		}
		रोक;
	}

	फर्काऊ res;
}


/**
 * Callback for html form textareas.
 */
स्थिर शून्य box_textarea_callback(शून्य *data, संरचना textarea_सन्देश *सन्देश)
{
	संरचना form_textarea_data *d = data;
	संरचना form_control *gadget = d->gadget;
	संरचना html_content *html = d->gadget->html;
	संरचना box *box = gadget->box;

	विकल्प (सन्देश->type) {
	पक्ष TEXTAREA_सन्देश_DRAG_REPORT:
		यदि (सन्देश->data.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			सङ्घ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					रिक्त);
		} अन्यथा {
			/* Textarea drag started */
			संरचना rect rect = {
				.x0 = पूर्णांक_न्यून,
				.y0 = पूर्णांक_न्यून,
				.x1 = पूर्णांक_उच्च,
				.y1 = पूर्णांक_उच्च
			};
			सङ्घ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			विकल्प (सन्देश->data.drag) {
			पक्ष TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				रोक;

			पक्ष TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				रोक;

			स्वभाव:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      सन्देश->data.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				प्रतिज्ञा(0);
				रोक;
			}
		}
		रोक;

	पक्ष TEXTAREA_सन्देश_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		पूर्णाङ्क x, y;

		यदि (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			रोक;
		}

		box_coords(box, &x, &y);

		content__request_redraw((संरचना content *)html,
				x + सन्देश->data.redraw.x0,
				y + सन्देश->data.redraw.y0,
				सन्देश->data.redraw.x1 - सन्देश->data.redraw.x0,
				सन्देश->data.redraw.y1 - सन्देश->data.redraw.y0);
	}
		रोक;

	पक्ष TEXTAREA_सन्देश_SELECTION_REPORT:
		यदि (सन्देश->data.selection.have_selection) {
			/* Textarea now has a selection */
			सङ्घ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					सन्देश->data.selection.read_only);
		} अन्यथा {
			/* The textarea now has no selection */
			सङ्घ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		रोक;

	पक्ष TEXTAREA_सन्देश_CARET_UPDATE:
		यदि (html->bw == रिक्त)
			रोक;

		यदि (सन्देश->data.caret.type == TEXTAREA_CARET_HIDE) {
			सङ्घ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, रिक्त);
		} अन्यथा {
			सङ्घ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					सन्देश->data.caret.pos.x,
					सन्देश->data.caret.pos.y,
					सन्देश->data.caret.pos.height,
					सन्देश->data.caret.pos.clip);
		}
		रोक;

	पक्ष TEXTAREA_सन्देश_TEXT_MODIFIED:
		form_gadget_update_मान(gadget,
					 strndup(सन्देश->data.modified.text,
						 सन्देश->data.modified.len));
		रोक;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		संरचना box *box, संरचना dom_गाँठो *गाँठो)
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
	अचर वर्ण *text;

	प्रतिज्ञा(gadget != रिक्त);
	प्रतिज्ञा(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	यदि (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) गाँठो;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_मान(textarea, &dom_text);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;

	} अन्यथा {
		dom_html_आगत_element *आगत = (dom_html_आगत_element *) गाँठो;

		err = dom_html_आगत_element_get_read_only(
				आगत, &read_only);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;

		err = dom_html_आगत_element_get_disabled(
				आगत, &disabled);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;

		यदि (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		अन्यथा
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_आगत_element_get_मान(आगत, &dom_text);
		यदि (err != DOM_NO_ERR)
			फर्काऊ false;
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
	ta_setup.pad_दायाँ = 4;
	ta_setup.pad_bottom = 4;
	ta_setup.pad_बायाँ = 4;

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
		फर्काऊ false;
	}

	यदि (!textarea_set_text(gadget->data.text.ta, text))
		फर्काऊ false;

	फर्काऊ true;
}
