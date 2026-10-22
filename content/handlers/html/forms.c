/*
 * Copyright 2011 Vincent Sanders <vince@netsurf-browser.org>
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
 * HTML form handling implementation
 */

#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"

#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"

/**
 * process form element from dom
 */
ಸ್ಥಿರ ಸಂರಚನಾ form *
parse_form_element(ಧ್ರುವ ವರ್ಣ *docenc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *ds_action = ರಿಕ್ತ;
	dom_string *ds_charset = ರಿಕ್ತ;
	dom_string *ds_target = ರಿಕ್ತ;
	dom_string *ds_method = ರಿಕ್ತ;
	dom_string *ds_enctype = ರಿಕ್ತ;
	ವರ್ಣ *action = ರಿಕ್ತ, *charset = ರಿಕ್ತ, *target = ರಿಕ್ತ;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(ಗಂಟು);
	ಸಂರಚನಾ form * ret = ರಿಕ್ತ;

	/* Retrieve the attributes from the node */
	ಯದಿ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ಯದಿ (ds_action != ರಿಕ್ತ)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	ಯದಿ (ds_charset != ರಿಕ್ತ)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	ಯದಿ (ds_target != ರಿಕ್ತ)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	ಯದಿ (ds_method != ರಿಕ್ತ) {
		ಯದಿ (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			ಯದಿ (ds_enctype != ರಿಕ್ತ) {
				ಯದಿ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ಗಂಟು, action, target, method, charset, docenc);

out:
	ಯದಿ (ds_action != ರಿಕ್ತ)
		dom_string_unref(ds_action);
	ಯದಿ (ds_charset != ರಿಕ್ತ)
		dom_string_unref(ds_charset);
	ಯದಿ (ds_target != ರಿಕ್ತ)
		dom_string_unref(ds_target);
	ಯದಿ (ds_method != ರಿಕ್ತ)
		dom_string_unref(ds_method);
	ಯದಿ (ds_enctype != ರಿಕ್ತ)
		dom_string_unref(ds_enctype);
	ಯದಿ (action != ರಿಕ್ತ)
		ಮುಕ್ತ(action);
	ಯದಿ (charset != ರಿಕ್ತ)
		ಮುಕ್ತ(charset);
	ಯದಿ (target != ರಿಕ್ತ)
		ಮುಕ್ತ(target);
	ಪ್ರತ್ಯಾಗಮ ret;
}

/* documented in html_internal.h */
ಸಂರಚನಾ form *html_forms_get_forms(ಧ್ರುವ ವರ್ಣ *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ಸಂರಚನಾ form *ret = ರಿಕ್ತ, *newf;
	dom_ಗಂಟು *ಗಂಟು;
	unsigned long n;
	uint32_t nforms;

	ಯದಿ (doc == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* Attempt to build a set of all the forms */
	ಯದಿ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* Count the number of forms so we can iterate */
	ಯದಿ (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	/* Iterate the forms collection, making form structs for returning */
	ಪ್ರತಿ (n = 0; n < nforms; ++n) {
		ಯದಿ (dom_html_collection_item(forms, n, &ಗಂಟು) != DOM_NO_ERR) {
			ಗಚ್ಛ out;
		}
		newf = parse_form_element(docenc, ಗಂಟು);
		dom_ಗಂಟು_unref(ಗಂಟು);
		ಯದಿ (newf == ರಿಕ್ತ) {
			ಗಚ್ಛ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	ಗಚ್ಛ out;
err:
	ಯಾವತ್ (ret != ರಿಕ್ತ) {
		ಸಂರಚನಾ form *prev = ret->prev;
		/* Destroy ret */
		ಮುಕ್ತ(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಸಂರಚನಾ form *
find_form(ಸಂರಚನಾ form *forms, dom_html_form_element *form)
{
	ಯಾವತ್ (forms != ರಿಕ್ತ) {
		ಯದಿ (forms->ಗಂಟು == form)
			ಭಙ್ಗ;
		forms = forms->prev;
	}

	ಪ್ರತ್ಯಾಗಮ forms;
}

ಸ್ಥಿರ ಸಂರಚನಾ form_control *
parse_button_element(ಸಂರಚನಾ form *forms, dom_html_button_element *button)
{
	ಸಂರಚನಾ form_control *control = ರಿಕ್ತ;
	dom_exception err;
	dom_html_form_element *form = ರಿಕ್ತ;
	dom_string *ds_type = ರಿಕ್ತ;
	dom_string *ds_ಮೌಲ್ಯ = ರಿಕ್ತ;
	dom_string *ds_name = ರಿಕ್ತ;

	err = dom_html_button_element_get_form(button, &form);
	ಯದಿ (err != DOM_NO_ERR)
		ಗಚ್ಛ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	ಯದಿ (err != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (ds_type == ರಿಕ್ತ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ಅನ್ಯಥಾ {
		ಯದಿ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ಅನ್ಯಥಾ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ಯದಿ (control == ರಿಕ್ತ)
		ಗಚ್ಛ out;

	err = dom_html_button_element_get_ಮೌಲ್ಯ(button, &ds_ಮೌಲ್ಯ);
	ಯದಿ (err != DOM_NO_ERR)
		ಗಚ್ಛ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	ಯದಿ (err != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (ds_ಮೌಲ್ಯ != ರಿಕ್ತ) {
		control->ಮೌಲ್ಯ = strndup(
			dom_string_data(ds_ಮೌಲ್ಯ),
			dom_string_byte_length(ds_ಮೌಲ್ಯ));

		ಯದಿ (control->ಮೌಲ್ಯ == ರಿಕ್ತ) {
			form_free_control(control);
			control = ರಿಕ್ತ;
			ಗಚ್ಛ out;
		}
	}

	ಯದಿ (ds_name != ರಿಕ್ತ) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		ಯದಿ (control->name == ರಿಕ್ತ) {
			form_free_control(control);
			control = ರಿಕ್ತ;
			ಗಚ್ಛ out;
		}
	}

	ಯದಿ (form != ರಿಕ್ತ && control != ರಿಕ್ತ)
		form_add_control(find_form(forms, form), control);

out:
	ಯದಿ (form != ರಿಕ್ತ)
		dom_ಗಂಟು_unref(form);
	ಯದಿ (ds_type != ರಿಕ್ತ)
		dom_string_unref(ds_type);
	ಯದಿ (ds_ಮೌಲ್ಯ != ರಿಕ್ತ)
		dom_string_unref(ds_ಮೌಲ್ಯ);
	ಯದಿ (ds_name != ರಿಕ್ತ)
		dom_string_unref(ds_name);

	ಪ್ರತ್ಯಾಗಮ control;
}

ಸ್ಥಿರ ಸಂರಚನಾ form_control *
parse_ಒಳಹರಿವು_element(ಸಂರಚನಾ form *forms, dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು)
{
	ಸಂರಚನಾ form_control *control = ರಿಕ್ತ;
	dom_html_form_element *form = ರಿಕ್ತ;
	dom_string *ds_type = ರಿಕ್ತ;
	dom_string *ds_name = ರಿಕ್ತ;
	dom_string *ds_ಮೌಲ್ಯ = ರಿಕ್ತ;

	ವರ್ಣ *name = ರಿಕ್ತ;

	ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_form(ಒಳಹರಿವು, &form) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_type(ಒಳಹರಿವು, &ds_type) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_name(ಒಳಹರಿವು, &ds_name) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (ds_name != ರಿಕ್ತ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_PASSWORD);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_FILE);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_HIDDEN);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_CHECKBOX);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_RADIO);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_SUBMIT);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_RESET);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_BUTTON);
	} ಅನ್ಯಥಾ ಯದಿ (ds_type != ರಿಕ್ತ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_IMAGE);
	} ಅನ್ಯಥಾ {
		control = form_new_control(ಒಳಹರಿವು, GADGET_TEXTBOX);
	}

	ಯದಿ (control == ರಿಕ್ತ)
		ಗಚ್ಛ out;

	ಯದಿ (name != ರಿಕ್ತ) {
		/* Hand the name string over */
		control->name = name;
		name = ರಿಕ್ತ;
	}

	ಯದಿ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_checked(
			    ಒಳಹರಿವು, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ಯದಿ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_max_length(
			    ಒಳಹರಿವು, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ಯದಿ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ಅನ್ಯಥಾ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ಯದಿ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ಯದಿ (dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(
			    ಒಳಹರಿವು, &ds_ಮೌಲ್ಯ) == DOM_NO_ERR) {
			ಯದಿ (ds_ಮೌಲ್ಯ != ರಿಕ್ತ) {
				control->ಮೌಲ್ಯ = strndup(
					dom_string_data(ds_ಮೌಲ್ಯ),
					dom_string_byte_length(ds_ಮೌಲ್ಯ));
				ಯದಿ (control->ಮೌಲ್ಯ == ರಿಕ್ತ) {
					form_free_control(control);
					control = ರಿಕ್ತ;
					ಗಚ್ಛ out;
				}
				control->length = strlen(control->ಮೌಲ್ಯ);
			}
		}

		ಯದಿ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ಯದಿ (control->ಮೌಲ್ಯ == ರಿಕ್ತ) {
				control->ಮೌಲ್ಯ = strdup("");
				ಯದಿ (control->ಮೌಲ್ಯ == ರಿಕ್ತ) {
					form_free_control(control);
					control = ರಿಕ್ತ;
					ಗಚ್ಛ out;
				}

				control->length = 0;
			}

			control->initial_ಮೌಲ್ಯ = strdup(control->ಮೌಲ್ಯ);
			ಯದಿ (control->initial_ಮೌಲ್ಯ == ರಿಕ್ತ) {
				form_free_control(control);
				control = ರಿಕ್ತ;
				ಗಚ್ಛ out;
			}

			control->last_synced_ಮೌಲ್ಯ = strdup(control->ಮೌಲ್ಯ);
			ಯದಿ (control->last_synced_ಮೌಲ್ಯ == ರಿಕ್ತ) {
				form_free_control(control);
				control = ರಿಕ್ತ;
				ಗಚ್ಛ out;
			}

			control->ಗಂಟು_ಮೌಲ್ಯ = dom_string_ref(ds_ಮೌಲ್ಯ);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ಯದಿ (form != ರಿಕ್ತ && control != ರಿಕ್ತ)
		form_add_control(find_form(forms, form), control);

out:
	ಯದಿ (form != ರಿಕ್ತ)
		dom_ಗಂಟು_unref(form);
	ಯದಿ (ds_type != ರಿಕ್ತ)
		dom_string_unref(ds_type);
	ಯದಿ (ds_name != ರಿಕ್ತ)
		dom_string_unref(ds_name);
	ಯದಿ (ds_ಮೌಲ್ಯ != ರಿಕ್ತ)
		dom_string_unref(ds_ಮೌಲ್ಯ);

	ಯದಿ (name != ರಿಕ್ತ)
		ಮುಕ್ತ(name);

	ಪ್ರತ್ಯಾಗಮ control;
}

ಸ್ಥಿರ ಸಂರಚನಾ form_control *
parse_textarea_element(ಸಂರಚನಾ form *forms, dom_html_text_area_element *ta)
{
	ಸಂರಚನಾ form_control *control = ರಿಕ್ತ;
	dom_html_form_element *form = ರಿಕ್ತ;
	dom_string *ds_name = ರಿಕ್ತ;

	ವರ್ಣ *name = ರಿಕ್ತ;

	ಯದಿ (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (ds_name != ರಿಕ್ತ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ಯದಿ (control == ರಿಕ್ತ)
		ಗಚ್ಛ out;

	ಯದಿ (name != ರಿಕ್ತ) {
		/* Hand the name string over */
		control->name = name;
		name = ರಿಕ್ತ;
	}

	ಯದಿ (form != ರಿಕ್ತ && control != ರಿಕ್ತ)
		form_add_control(find_form(forms, form), control);

out:
	ಯದಿ (form != ರಿಕ್ತ)
		dom_ಗಂಟು_unref(form);
	ಯದಿ (ds_name != ರಿಕ್ತ)
		dom_string_unref(ds_name);

	ಯದಿ (name != ರಿಕ್ತ)
		ಮುಕ್ತ(name);


	ಪ್ರತ್ಯಾಗಮ control;
}

ಸ್ಥಿರ ಸಂರಚನಾ form_control *
parse_select_element(ಸಂರಚನಾ form *forms, dom_html_select_element *select)
{
	ಸಂರಚನಾ form_control *control = ರಿಕ್ತ;
	dom_html_form_element *form = ರಿಕ್ತ;
	dom_string *ds_name = ರಿಕ್ತ;

	ವರ್ಣ *name = ರಿಕ್ತ;

	ಯದಿ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		ಗಚ್ಛ out;

	ಯದಿ (ds_name != ರಿಕ್ತ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ಯದಿ (control == ರಿಕ್ತ)
		ಗಚ್ಛ out;

	ಯದಿ (name != ರಿಕ್ತ) {
		/* Hand the name string over */
		control->name = name;
		name = ರಿಕ್ತ;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	ಯದಿ (form != ರಿಕ್ತ && control != ರಿಕ್ತ)
		form_add_control(find_form(forms, form), control);

out:
	ಯದಿ (form != ರಿಕ್ತ)
		dom_ಗಂಟು_unref(form);
	ಯದಿ (ds_name != ರಿಕ್ತ)
		dom_string_unref(ds_name);

	ಯದಿ (name != ರಿಕ್ತ)
		ಮುಕ್ತ(name);


	ಪ್ರತ್ಯಾಗಮ control;
}


ಸ್ಥಿರ ಸಂರಚನಾ form_control *
invent_fake_gadget(dom_ಗಂಟು *ಗಂಟು)
{
	ಸಂರಚನಾ form_control *ctl = form_new_control(ಗಂಟು, GADGET_HIDDEN);
	ಯದಿ (ctl != ರಿಕ್ತ) {
		ctl->ಮೌಲ್ಯ = strdup("");
		ctl->initial_ಮೌಲ್ಯ = strdup("");
		ctl->name = strdup("foo");

		ಯದಿ (ctl->ಮೌಲ್ಯ == ರಿಕ್ತ || ctl->initial_ಮೌಲ್ಯ == ರಿಕ್ತ ||
				ctl->name == ರಿಕ್ತ) {
			form_free_control(ctl);
			ctl = ರಿಕ್ತ;
		}
	}
	ಪ್ರತ್ಯಾಗಮ ctl;
}

/* documented in html_internal.h */
ಸಂರಚನಾ form_control *
html_forms_get_control_for_ಗಂಟು(ಸಂರಚನಾ form *forms, dom_ಗಂಟು *ಗಂಟು)
{
	ಸಂರಚನಾ form *f;
	ಸಂರಚನಾ form_control *ctl = ರಿಕ್ತ;
	dom_exception err;
	dom_string *ds_name = ರಿಕ್ತ;

	/* Step one, see if we already have a control */
	ಪ್ರತಿ (f = forms; f != ರಿಕ್ತ; f = f->prev) {
		ಪ್ರತಿ (ctl = f->controls; ctl != ರಿಕ್ತ; ctl = ctl->next) {
			ಯದಿ (ctl->ಗಂಟು == ಗಂಟು)
				ಪ್ರತ್ಯಾಗಮ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(ಗಂಟು, &ds_name);
	ಯದಿ (err == DOM_NO_ERR && ds_name != ರಿಕ್ತ) {

		/* Step three, attempt to work out what gadget to make */
		ಯದಿ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ಗಂಟು);
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ಒಳಹರಿವು)) {
			ctl = parse_ಒಳಹರಿವು_element(forms,
					(dom_html_ಒಳಹರಿವು_element *) ಗಂಟು);
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) ಗಂಟು);
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ಗಂಟು);
		}
	}

	/* If all else fails, fake gadget time */
	ಯದಿ (ctl == ರಿಕ್ತ)
		ctl = invent_fake_gadget(ಗಂಟು);

	ಯದಿ (ds_name != ರಿಕ್ತ)
		dom_string_unref(ds_name);

	ಪ್ರತ್ಯಾಗಮ ctl;
}
