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

#समावेश <string.h>
#समावेश <dom/dom.h>

#समावेश "utils/config.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/keypress.h"
#समावेश "netsurf/misc.h"
#समावेश "desktop/textarea.h"

#समावेश "html/private.h"
#समावेश "html/interaction.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/box_textarea.h"
#समावेश "html/font.h"
#समावेश "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, काष्ठा box *box, uint32_t कुंजी)
{
	काष्ठा form_control *gadget = box->gadget;
	काष्ठा textarea *ta = gadget->तथ्य.text.ta;
	काष्ठा form* form = box->gadget->form;
	काष्ठा content *c = (काष्ठा content *)html;
	nserror res = NSERROR_OK;

	निश्चित(ta != शून्य);

	अगर (gadget->type == GADGET_TEXTAREA) {
		अगर (textarea_keypress(ta, कुंजी)) {
			वापस NSERROR_OK;
		} अन्यथा {
			वापस NSERROR_INVALID;
		}
	}

	/* non textarea input */
	चयन (कुंजी) {
	हाल NS_कुंजी_NL:
	हाल NS_कुंजी_CR:
		अगर (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  शून्य);
		}
		अवरोध;

	हाल NS_कुंजी_TAB:
		{
			काष्ठा form_control *अग्र_निवेश;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			क्रम (अग्र_निवेश = gadget->अग्र;
			     अग्र_निवेश &&
				     ((अग्र_निवेश->type != GADGET_TEXTBOX &&
				       अग्र_निवेश->type != GADGET_TEXTAREA &&
				       अग्र_निवेश->type != GADGET_PASSWORD) ||
				      !अग्र_निवेश->box);
			     अग्र_निवेश = अग्र_निवेश->अग्र)
				;

			अगर (अग्र_निवेश != शून्य) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(अग्र_निवेश->तथ्य.text.ta, 0);
			}
		}
		अवरोध;

	हाल NS_कुंजी_SHIFT_TAB:
		{
			काष्ठा form_control *पूर्व_निवेश;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			क्रम (पूर्व_निवेश = gadget->पूर्व;
			     पूर्व_निवेश &&
				     ((पूर्व_निवेश->type != GADGET_TEXTBOX &&
				       पूर्व_निवेश->type != GADGET_TEXTAREA &&
				       पूर्व_निवेश->type != GADGET_PASSWORD) ||
				      !पूर्व_निवेश->box);
			     पूर्व_निवेश = पूर्व_निवेश->पूर्व)
				;

			अगर (पूर्व_निवेश != शून्य) {
				textarea_set_caret(ta, -1);
				textarea_set_caret(पूर्व_निवेश->तथ्य.text.ta, 0);
			}
		}
		अवरोध;

	शेष:
		/* Pass to textarea widget */
		अगर (!textarea_keypress(ta, कुंजी)) {
			res = NSERROR_INVALID;
		}
		अवरोध;
	}

	वापस res;
}


/**
 * Callback for html form textareas.
 */
अटल व्योम box_textarea_callback(व्योम *तथ्य, काष्ठा textarea_संदेश *संदेश)
{
	काष्ठा form_textarea_तथ्य *d = तथ्य;
	काष्ठा form_control *gadget = d->gadget;
	काष्ठा html_content *html = d->gadget->html;
	काष्ठा box *box = gadget->box;

	चयन (संदेश->type) {
	हाल TEXTAREA_संदेश_DRAG_REPORT:
		अगर (संदेश->तथ्य.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			जोड़ html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_set_drag_type(html, drag_type, drag_owner,
					शून्य);
		} अन्यथा {
			/* Textarea drag started */
			काष्ठा rect rect = {
				.x0 = पूर्णांक_न्यून,
				.y0 = पूर्णांक_न्यून,
				.x1 = पूर्णांक_उच्च,
				.y1 = पूर्णांक_उच्च
			};
			जोड़ html_drag_owner drag_owner;
			drag_owner.textarea = box;

			चयन (संदेश->तथ्य.drag) {
			हाल TEXTAREA_DRAG_SCROLLBAR:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				अवरोध;

			हाल TEXTAREA_DRAG_SELECTION:
				html_set_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				अवरोध;

			शेष:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      संदेश->तथ्य.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				निश्चित(0);
				अवरोध;
			}
		}
		अवरोध;

	हाल TEXTAREA_संदेश_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		पूर्णांक x, y;

		अगर (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			अवरोध;
		}

		box_coords(box, &x, &y);

		content__request_redraw((काष्ठा content *)html,
				x + संदेश->तथ्य.redraw.x0,
				y + संदेश->तथ्य.redraw.y0,
				संदेश->तथ्य.redraw.x1 - संदेश->तथ्य.redraw.x0,
				संदेश->तथ्य.redraw.y1 - संदेश->तथ्य.redraw.y0);
	}
		अवरोध;

	हाल TEXTAREA_संदेश_SELECTION_REPORT:
		अगर (संदेश->तथ्य.selection.have_selection) {
			/* Textarea now has a selection */
			जोड़ html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_set_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					संदेश->तथ्य.selection.read_only);
		} अन्यथा {
			/* The textarea now has no selection */
			जोड़ html_selection_owner sel_owner;
			sel_owner.none = true;

			html_set_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		अवरोध;

	हाल TEXTAREA_संदेश_CARET_UPDATE:
		अगर (html->bw == शून्य)
			अवरोध;

		अगर (संदेश->तथ्य.caret.type == TEXTAREA_CARET_HIDE) {
			जोड़ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, शून्य);
		} अन्यथा {
			जोड़ html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_set_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					संदेश->तथ्य.caret.pos.x,
					संदेश->तथ्य.caret.pos.y,
					संदेश->तथ्य.caret.pos.height,
					संदेश->तथ्य.caret.pos.clip);
		}
		अवरोध;

	हाल TEXTAREA_संदेश_TEXT_MODIFIED:
		form_gadget_update_मूल्य(gadget,
					 strndup(संदेश->तथ्य.modified.text,
						 संदेश->तथ्य.modified.len));
		अवरोध;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		काष्ठा box *box, काष्ठा dom_गाँठ *गाँठ)
{
	dom_string *dom_text = शून्य;
	dom_exception err;
	textarea_setup ta_setup;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.आकार = 10 * PLOT_STYLE_SCALE,
		.weight = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	काष्ठा form_control *gadget = box->gadget;
	स्थिर अक्षर *text;

	निश्चित(gadget != शून्य);
	निश्चित(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	अगर (gadget->type == GADGET_TEXTAREA) {
		dom_html_text_area_element *textarea =
				(dom_html_text_area_element *) गाँठ;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_text_area_element_get_read_only(
				textarea, &read_only);
		अगर (err != DOM_NO_ERR)
			वापस false;

		err = dom_html_text_area_element_get_disabled(
				textarea, &disabled);
		अगर (err != DOM_NO_ERR)
			वापस false;

		/* Get the textarea's initial content */
		err = dom_html_text_area_element_get_मूल्य(textarea, &dom_text);
		अगर (err != DOM_NO_ERR)
			वापस false;

	} अन्यथा {
		dom_html_निवेश_element *निवेश = (dom_html_निवेश_element *) गाँठ;

		err = dom_html_निवेश_element_get_read_only(
				निवेश, &read_only);
		अगर (err != DOM_NO_ERR)
			वापस false;

		err = dom_html_निवेश_element_get_disabled(
				निवेश, &disabled);
		अगर (err != DOM_NO_ERR)
			वापस false;

		अगर (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		अन्यथा
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_निवेश_element_get_मूल्य(निवेश, &dom_text);
		अगर (err != DOM_NO_ERR)
			वापस false;
	}

	अगर (dom_text != शून्य) {
		text = dom_string_तथ्य(dom_text);
	} अन्यथा {
		/* No initial text, or failed reading it;
		 * use a blank string */
		text = "";
	}

	अगर (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->तथ्य.text.तथ्य.gadget = gadget;

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
	gadget->तथ्य.text.initial = dom_text;

	gadget->तथ्य.text.ta = textarea_create(ta_flags, &ta_setup,
			box_textarea_callback, &gadget->तथ्य.text.तथ्य);

	अगर (gadget->तथ्य.text.ta == शून्य) {
		वापस false;
	}

	अगर (!textarea_set_text(gadget->तथ्य.text.ta, text))
		वापस false;

	वापस true;
}
