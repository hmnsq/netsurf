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

#అన్తర్భావ <string.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/keypress.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "desktop/textarea.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/interaction.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/box_textarea.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/form_internal.h"


nserror box_textarea_keypress(html_content *html, నిర్మాణం box *box, uint32_t కీలకం)
{
	నిర్మాణం form_control *gadget = box->gadget;
	నిర్మాణం textarea *ta = gadget->దత్తాంశం.పాఠ్యం.ta;
	నిర్మాణం form* form = box->gadget->form;
	నిర్మాణం content *c = (నిర్మాణం content *)html;
	nserror res = NSERROR_OK;

	ప్రతిజ్ఞా(ta != ఖాళీ);

	ఒకవేళ (gadget->type == GADGET_TEXTAREA) {
		ఒకవేళ (textarea_keypress(ta, కీలకం)) {
			తిరిగి NSERROR_OK;
		} లేకపోతే {
			తిరిగి NSERROR_INVALID;
		}
	}

	/* non textarea input */
	ఎంపిక (కీలకం) {
	స్థితి NS_కీలకం_NL:
	స్థితి NS_కీలకం_CR:
		ఒకవేళ (form) {
			res = form_submit(content_get_url(c),
					  html->bw,
					  form,
					  ఖాళీ);
		}
		ఆపు;

	స్థితి NS_కీలకం_TAB:
		{
			నిర్మాణం form_control *తదుపరి_ఆదానం;
			/* Find next text entry field that is actually
			 * displayed (i.e. has an associated box) */
			కొరకు (తదుపరి_ఆదానం = gadget->తదుపరి;
			     తదుపరి_ఆదానం &&
				     ((తదుపరి_ఆదానం->type != GADGET_TEXTBOX &&
				       తదుపరి_ఆదానం->type != GADGET_TEXTAREA &&
				       తదుపరి_ఆదానం->type != GADGET_PASSWORD) ||
				      !తదుపరి_ఆదానం->box);
			     తదుపరి_ఆదానం = తదుపరి_ఆదానం->తదుపరి)
				;

			ఒకవేళ (తదుపరి_ఆదానం != ఖాళీ) {
				textarea_సమితి_caret(ta, -1);
				textarea_సమితి_caret(తదుపరి_ఆదానం->దత్తాంశం.పాఠ్యం.ta, 0);
			}
		}
		ఆపు;

	స్థితి NS_కీలకం_జరుపు_TAB:
		{
			నిర్మాణం form_control *prev_ఆదానం;
			/* Find previous text entry field that is actually
			 * displayed (i.e. has an associated box) */
			కొరకు (prev_ఆదానం = gadget->prev;
			     prev_ఆదానం &&
				     ((prev_ఆదానం->type != GADGET_TEXTBOX &&
				       prev_ఆదానం->type != GADGET_TEXTAREA &&
				       prev_ఆదానం->type != GADGET_PASSWORD) ||
				      !prev_ఆదానం->box);
			     prev_ఆదానం = prev_ఆదానం->prev)
				;

			ఒకవేళ (prev_ఆదానం != ఖాళీ) {
				textarea_సమితి_caret(ta, -1);
				textarea_సమితి_caret(prev_ఆదానం->దత్తాంశం.పాఠ్యం.ta, 0);
			}
		}
		ఆపు;

	అప్రమేయ:
		/* Pass to textarea widget */
		ఒకవేళ (!textarea_keypress(ta, కీలకం)) {
			res = NSERROR_INVALID;
		}
		ఆపు;
	}

	తిరిగి res;
}


/**
 * Callback for html form textareas.
 */
స్థిర శూన్య box_textarea_callback(శూన్య *దత్తాంశం, నిర్మాణం textarea_సందేశం *సందేశం)
{
	నిర్మాణం form_textarea_దత్తాంశం *d = దత్తాంశం;
	నిర్మాణం form_control *gadget = d->gadget;
	నిర్మాణం html_content *html = d->gadget->html;
	నిర్మాణం box *box = gadget->box;

	ఎంపిక (సందేశం->type) {
	స్థితి TEXTAREA_సందేశం_DRAG_REPORT:
		ఒకవేళ (సందేశం->దత్తాంశం.drag == TEXTAREA_DRAG_NONE) {
			/* Textarea drag finished */
			html_drag_type drag_type = HTML_DRAG_NONE;
			సంఘం html_drag_owner drag_owner;
			drag_owner.no_owner = true;

			html_సమితి_drag_type(html, drag_type, drag_owner,
					ఖాళీ);
		} లేకపోతే {
			/* Textarea drag started */
			నిర్మాణం rect rect = {
				.x0 = INT_MIN,
				.y0 = INT_MIN,
				.x1 = గరిష్ఠ_పూర్ణాంకం,
				.y1 = గరిష్ఠ_పూర్ణాంకం
			};
			సంఘం html_drag_owner drag_owner;
			drag_owner.textarea = box;

			ఎంపిక (సందేశం->దత్తాంశం.drag) {
			స్థితి TEXTAREA_DRAG_SCROLLBAR:
				html_సమితి_drag_type(html,
						   HTML_DRAG_TEXTAREA_SCROLLBAR,
						   drag_owner,
						   &rect);
				ఆపు;

			స్థితి TEXTAREA_DRAG_SELECTION:
				html_సమితి_drag_type(html,
						   HTML_DRAG_TEXTAREA_SELECTION,
						   drag_owner,
						   &rect);
				ఆపు;

			అప్రమేయ:
				NSLOG(netsurf, INFO,
				      "Drag type %d not handled.",
				      సందేశం->దత్తాంశం.drag);
				/* This is a logic faliure in the
				 * front end code so abort.
				 */
				ప్రతిజ్ఞా(0);
				ఆపు;
			}
		}
		ఆపు;

	స్థితి TEXTAREA_సందేశం_REDRAW_REQUEST:
	{
		/* Request redraw of the required textarea rectangle */
		పూర్ణాంకం x, y;

		ఒకవేళ (html->reflowing == true) {
			/* Can't redraw during layout, and it will
			 * be redrawn after layout anyway. */
			ఆపు;
		}

		box_coords(box, &x, &y);

		content__request_redraw((నిర్మాణం content *)html,
				x + సందేశం->దత్తాంశం.redraw.x0,
				y + సందేశం->దత్తాంశం.redraw.y0,
				సందేశం->దత్తాంశం.redraw.x1 - సందేశం->దత్తాంశం.redraw.x0,
				సందేశం->దత్తాంశం.redraw.y1 - సందేశం->దత్తాంశం.redraw.y0);
	}
		ఆపు;

	స్థితి TEXTAREA_సందేశం_SELECTION_REPORT:
		ఒకవేళ (సందేశం->దత్తాంశం.selection.have_selection) {
			/* Textarea now has a selection */
			సంఘం html_selection_owner sel_owner;
			sel_owner.textarea = box;

			html_సమితి_selection(html, HTML_SELECTION_TEXTAREA,
					sel_owner,
					సందేశం->దత్తాంశం.selection.read_only);
		} లేకపోతే {
			/* The textarea now has no selection */
			సంఘం html_selection_owner sel_owner;
			sel_owner.none = true;

			html_సమితి_selection(html, HTML_SELECTION_NONE,
					sel_owner, true);
		}
		ఆపు;

	స్థితి TEXTAREA_సందేశం_CARET_UPDATE:
		ఒకవేళ (html->bw == ఖాళీ)
			ఆపు;

		ఒకవేళ (సందేశం->దత్తాంశం.caret.type == TEXTAREA_CARET_HIDE) {
			సంఘం html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_సమితి_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, true, 0, 0, 0, ఖాళీ);
		} లేకపోతే {
			సంఘం html_focus_owner focus_owner;
			focus_owner.textarea = box;
			html_సమితి_focus(html, HTML_FOCUS_TEXTAREA,
					focus_owner, false,
					సందేశం->దత్తాంశం.caret.pos.x,
					సందేశం->దత్తాంశం.caret.pos.y,
					సందేశం->దత్తాంశం.caret.pos.ఎత్తు,
					సందేశం->దత్తాంశం.caret.pos.clip);
		}
		ఆపు;

	స్థితి TEXTAREA_సందేశం_పాఠ్యం_MODIFIED:
		form_gadget_update_విలువ(gadget,
					 strndup(సందేశం->దత్తాంశం.modified.పాఠ్యం,
						 సందేశం->దత్తాంశం.modified.len));
		ఆపు;
	}
}


/* Exported interface, documented in box_textarea.h */
bool box_textarea_create_textarea(html_content *html,
		నిర్మాణం box *box, నిర్మాణం dom_కణుపు *కణుపు)
{
	dom_string *dom_పాఠ్యం = ఖాళీ;
	dom_exception err;
	textarea_అమరిక ta_అమరిక;
	textarea_flags ta_flags;
	plot_font_style_t fstyle = {
		.family = PLOT_FONT_FAMILY_SANS_SERIF,
		.కొలత = 10 * PLOT_STYLE_SCALE,
		.బరువు = 400,
		.flags = FONTF_NONE,
		.background = 0,
		.foreground = 0,
	};
	bool read_only = false;
	bool disabled = false;
	నిర్మాణం form_control *gadget = box->gadget;
	స్థిరరాశి అక్షరం *పాఠ్యం;

	ప్రతిజ్ఞా(gadget != ఖాళీ);
	ప్రతిజ్ఞా(gadget->type == GADGET_TEXTAREA ||
			gadget->type == GADGET_TEXTBOX ||
			gadget->type == GADGET_PASSWORD);

	ఒకవేళ (gadget->type == GADGET_TEXTAREA) {
		dom_html_పాఠ్యం_area_element *textarea =
				(dom_html_పాఠ్యం_area_element *) కణుపు;
		ta_flags = TEXTAREA_MULTILINE;

		err = dom_html_పాఠ్యం_area_element_get_read_only(
				textarea, &read_only);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		err = dom_html_పాఠ్యం_area_element_get_disabled(
				textarea, &disabled);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		/* Get the textarea's initial content */
		err = dom_html_పాఠ్యం_area_element_get_విలువ(textarea, &dom_పాఠ్యం);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

	} లేకపోతే {
		dom_html_ఆదానం_element *ఆదానం = (dom_html_ఆదానం_element *) కణుపు;

		err = dom_html_ఆదానం_element_get_read_only(
				ఆదానం, &read_only);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		err = dom_html_ఆదానం_element_get_disabled(
				ఆదానం, &disabled);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		ఒకవేళ (gadget->type == GADGET_PASSWORD)
			ta_flags = TEXTAREA_PASSWORD;
		లేకపోతే
			ta_flags = TEXTAREA_DEFAULT;

		/* Get initial text */
		err = dom_html_ఆదానం_element_get_విలువ(ఆదానం, &dom_పాఠ్యం);
		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;
	}

	ఒకవేళ (dom_పాఠ్యం != ఖాళీ) {
		పాఠ్యం = dom_string_దత్తాంశం(dom_పాఠ్యం);
	} లేకపోతే {
		/* No initial text, or failed reading it;
		 * use a blank string */
		పాఠ్యం = "";
	}

	ఒకవేళ (read_only || disabled)
		ta_flags |= TEXTAREA_READONLY;

	gadget->దత్తాంశం.పాఠ్యం.దత్తాంశం.gadget = gadget;

	/* Reset to correct values by layout */
	ta_అమరిక.వెడల్పు = 200;
	ta_అమరిక.ఎత్తు = 20;
	ta_అమరిక.pad_శిఖరం = 4;
	ta_అమరిక.pad_కుడి = 4;
	ta_అమరిక.pad_bottom = 4;
	ta_అమరిక.pad_ఎడమ = 4;

	/* Set remaining data */
	ta_అమరిక.border_వెడల్పు = 0;
	ta_అమరిక.border_నిలువు = 0x000000;
	ta_అమరిక.పాఠ్యం = fstyle;
	ta_అమరిక.పాఠ్యం.background = NS_TRANSPARENT;
	/* Make selected text either black or white, as gives greatest contrast
	 * with background colour. */
	ta_అమరిక.selected_bg = fstyle.foreground;
	ta_అమరిక.selected_పాఠ్యం = రంగు_గమ్యం_bw_furthest(ta_అమరిక.selected_bg);

	/* Hand reference to dom text over to gadget */
	gadget->దత్తాంశం.పాఠ్యం.initial = dom_పాఠ్యం;

	gadget->దత్తాంశం.పాఠ్యం.ta = textarea_create(ta_flags, &ta_అమరిక,
			box_textarea_callback, &gadget->దత్తాంశం.పాఠ్యం.దత్తాంశం);

	ఒకవేళ (gadget->దత్తాంశం.పాఠ్యం.ta == ఖాళీ) {
		తిరిగి false;
	}

	ఒకవేళ (!textarea_సమితి_పాఠ్యం(gadget->దత్తాంశం.పాఠ్యం.ta, పాఠ్యం))
		తిరిగి false;

	తిరిగి true;
}
