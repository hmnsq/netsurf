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
ಸ್ಥಿರ ರಚನೆ form *
parse_form_element(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *docenc, dom_ಗಂಟು *ಗಂಟು)
{
	dom_string *ds_action = ಖಾಲಿ;
	dom_string *ds_charset = ಖಾಲಿ;
	dom_string *ds_ಗುರಿ = ಖಾಲಿ;
	dom_string *ds_ವಿಧಾನ = ಖಾಲಿ;
	dom_string *ds_enctype = ಖಾಲಿ;
	ಅಕ್ಷರ *action = ಖಾಲಿ, *charset = ಖಾಲಿ, *ಗುರಿ = ಖಾಲಿ;
	form_ವಿಧಾನ ವಿಧಾನ;
	dom_html_form_element *formele = (dom_html_form_element *)(ಗಂಟು);
	ರಚನೆ form * ret = ಖಾಲಿ;

	/* Retrieve the attributes from the node */
	ಆದರೆ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_form_element_get_ಗುರಿ(formele,
			&ds_ಗುರಿ) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_form_element_get_ವಿಧಾನ(formele,
			&ds_ವಿಧಾನ) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ಆದರೆ (ds_action != ಖಾಲಿ)
		action = strndup(dom_string_ದತ್ತಾಂಶ(ds_action),
				 dom_string_byte_ಉದ್ದಳತೆ(ds_action));

	ಆದರೆ (ds_charset != ಖಾಲಿ)
		charset = strndup(dom_string_ದತ್ತಾಂಶ(ds_charset),
				  dom_string_byte_ಉದ್ದಳತೆ(ds_charset));

	ಆದರೆ (ds_ಗುರಿ != ಖಾಲಿ)
		ಗುರಿ = strndup(dom_string_ದತ್ತಾಂಶ(ds_ಗುರಿ),
				 dom_string_byte_ಉದ್ದಳತೆ(ds_ಗುರಿ));

	/* Determine the method */
	ವಿಧಾನ = ವಿಧಾನ_GET;
	ಆದರೆ (ds_ವಿಧಾನ != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(ds_ವಿಧಾನ,
				corestring_lwc_post)) {
			ವಿಧಾನ = ವಿಧಾನ_POST_URLENC;
			ಆದರೆ (ds_enctype != ಖಾಲಿ) {
				ಆದರೆ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_ದತ್ತಾಂಶ)) {

					ವಿಧಾನ = ವಿಧಾನ_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ಗಂಟು, action, ಗುರಿ, ವಿಧಾನ, charset, docenc);

ಹೊರಹೋಗು:
	ಆದರೆ (ds_action != ಖಾಲಿ)
		dom_string_unref(ds_action);
	ಆದರೆ (ds_charset != ಖಾಲಿ)
		dom_string_unref(ds_charset);
	ಆದರೆ (ds_ಗುರಿ != ಖಾಲಿ)
		dom_string_unref(ds_ಗುರಿ);
	ಆದರೆ (ds_ವಿಧಾನ != ಖಾಲಿ)
		dom_string_unref(ds_ವಿಧಾನ);
	ಆದರೆ (ds_enctype != ಖಾಲಿ)
		dom_string_unref(ds_enctype);
	ಆದರೆ (action != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(action);
	ಆದರೆ (charset != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(charset);
	ಆದರೆ (ಗುರಿ != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(ಗುರಿ);
	ಹಿಂತಿರುಗಿ ret;
}

/* documented in html_internal.h */
ರಚನೆ form *html_forms_get_forms(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ರಚನೆ form *ret = ಖಾಲಿ, *newf;
	dom_ಗಂಟು *ಗಂಟು;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ n;
	uint32_t nforms;

	ಆದರೆ (doc == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* Attempt to build a set of all the forms */
	ಆದರೆ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* Count the number of forms so we can iterate */
	ಆದರೆ (dom_html_collection_get_ಉದ್ದಳತೆ(forms, &nforms) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	/* Iterate the forms collection, making form structs for returning */
	ಗಾಗಿ (n = 0; n < nforms; ++n) {
		ಆದರೆ (dom_html_collection_ಅಂಶ(forms, n, &ಗಂಟು) != DOM_NO_ERR) {
			ಹೋಗು ಹೊರಹೋಗು;
		}
		newf = parse_form_element(docenc, ಗಂಟು);
		dom_ಗಂಟು_unref(ಗಂಟು);
		ಆದರೆ (newf == ಖಾಲಿ) {
			ಹೋಗು err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	ಹೋಗು ಹೊರಹೋಗು;
err:
	ಆಗ (ret != ಖಾಲಿ) {
		ರಚನೆ form *prev = ret->prev;
		/* Destroy ret */
		ಬಿಡುಗಡೆ(ret);
		ret = prev;
	}
ಹೊರಹೋಗು:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ರಚನೆ form *
ಕಂಡುಹಿಡಿ_form(ರಚನೆ form *forms, dom_html_form_element *form)
{
	ಆಗ (forms != ಖಾಲಿ) {
		ಆದರೆ (forms->ಗಂಟು == form)
			ನಿಲ್ಲಿಸು;
		forms = forms->prev;
	}

	ಹಿಂತಿರುಗಿ forms;
}

ಸ್ಥಿರ ರಚನೆ form_control *
parse_button_element(ರಚನೆ form *forms, dom_html_button_element *button)
{
	ರಚನೆ form_control *control = ಖಾಲಿ;
	dom_exception err;
	dom_html_form_element *form = ಖಾಲಿ;
	dom_string *ds_type = ಖಾಲಿ;
	dom_string *ds_ಮೌಲ್ಯ = ಖಾಲಿ;
	dom_string *ds_name = ಖಾಲಿ;

	err = dom_html_button_element_get_form(button, &form);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	err = dom_html_button_element_get_type(button, &ds_type);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (ds_type == ಖಾಲಿ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ಇಲ್ಲದಿದ್ದರೆ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ಆದರೆ (control == ಖಾಲಿ)
		ಹೋಗು ಹೊರಹೋಗು;

	err = dom_html_button_element_get_ಮೌಲ್ಯ(button, &ds_ಮೌಲ್ಯ);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;
	err = dom_html_button_element_get_name(button, &ds_name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (ds_ಮೌಲ್ಯ != ಖಾಲಿ) {
		control->ಮೌಲ್ಯ = strndup(
			dom_string_ದತ್ತಾಂಶ(ds_ಮೌಲ್ಯ),
			dom_string_byte_ಉದ್ದಳತೆ(ds_ಮೌಲ್ಯ));

		ಆದರೆ (control->ಮೌಲ್ಯ == ಖಾಲಿ) {
			form_free_control(control);
			control = ಖಾಲಿ;
			ಹೋಗು ಹೊರಹೋಗು;
		}
	}

	ಆದರೆ (ds_name != ಖಾಲಿ) {
		control->name = strndup(
			dom_string_ದತ್ತಾಂಶ(ds_name),
			dom_string_byte_ಉದ್ದಳತೆ(ds_name));

		ಆದರೆ (control->name == ಖಾಲಿ) {
			form_free_control(control);
			control = ಖಾಲಿ;
			ಹೋಗು ಹೊರಹೋಗು;
		}
	}

	ಆದರೆ (form != ಖಾಲಿ && control != ಖಾಲಿ)
		form_ಕೂಡಿಸು_control(ಕಂಡುಹಿಡಿ_form(forms, form), control);

ಹೊರಹೋಗು:
	ಆದರೆ (form != ಖಾಲಿ)
		dom_ಗಂಟು_unref(form);
	ಆದರೆ (ds_type != ಖಾಲಿ)
		dom_string_unref(ds_type);
	ಆದರೆ (ds_ಮೌಲ್ಯ != ಖಾಲಿ)
		dom_string_unref(ds_ಮೌಲ್ಯ);
	ಆದರೆ (ds_name != ಖಾಲಿ)
		dom_string_unref(ds_name);

	ಹಿಂತಿರುಗಿ control;
}

ಸ್ಥಿರ ರಚನೆ form_control *
parse_ಒಳಹರಿವು_element(ರಚನೆ form *forms, dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು)
{
	ರಚನೆ form_control *control = ಖಾಲಿ;
	dom_html_form_element *form = ಖಾಲಿ;
	dom_string *ds_type = ಖಾಲಿ;
	dom_string *ds_name = ಖಾಲಿ;
	dom_string *ds_ಮೌಲ್ಯ = ಖಾಲಿ;

	ಅಕ್ಷರ *name = ಖಾಲಿ;

	ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_form(ಒಳಹರಿವು, &form) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_type(ಒಳಹರಿವು, &ds_type) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_name(ಒಳಹರಿವು, &ds_name) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (ds_name != ಖಾಲಿ)
		name = strndup(dom_string_ದತ್ತಾಂಶ(ds_name),
			       dom_string_byte_ಉದ್ದಳತೆ(ds_name));

	ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_PASSWORD);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_FILE);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_HIDDEN);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_CHECKBOX);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_RADIO);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_SUBMIT);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_RESET);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_BUTTON);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ds_type != ಖಾಲಿ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ಒಳಹರಿವು, GADGET_IMAGE);
	} ಇಲ್ಲದಿದ್ದರೆ {
		control = form_new_control(ಒಳಹರಿವು, GADGET_TEXTBOX);
	}

	ಆದರೆ (control == ಖಾಲಿ)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (name != ಖಾಲಿ) {
		/* Hand the name string over */
		control->name = name;
		name = ಖಾಲಿ;
	}

	ಆದರೆ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_checked(
			    ಒಳಹರಿವು, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ಆದರೆ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_max_ಉದ್ದಳತೆ(
			    ಒಳಹರಿವು, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ಆದರೆ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ಆದರೆ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ಆದರೆ (dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(
			    ಒಳಹರಿವು, &ds_ಮೌಲ್ಯ) == DOM_NO_ERR) {
			ಆದರೆ (ds_ಮೌಲ್ಯ != ಖಾಲಿ) {
				control->ಮೌಲ್ಯ = strndup(
					dom_string_ದತ್ತಾಂಶ(ds_ಮೌಲ್ಯ),
					dom_string_byte_ಉದ್ದಳತೆ(ds_ಮೌಲ್ಯ));
				ಆದರೆ (control->ಮೌಲ್ಯ == ಖಾಲಿ) {
					form_free_control(control);
					control = ಖಾಲಿ;
					ಹೋಗು ಹೊರಹೋಗು;
				}
				control->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(control->ಮೌಲ್ಯ);
			}
		}

		ಆದರೆ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ಆದರೆ (control->ಮೌಲ್ಯ == ಖಾಲಿ) {
				control->ಮೌಲ್ಯ = strdup("");
				ಆದರೆ (control->ಮೌಲ್ಯ == ಖಾಲಿ) {
					form_free_control(control);
					control = ಖಾಲಿ;
					ಹೋಗು ಹೊರಹೋಗು;
				}

				control->ಉದ್ದಳತೆ = 0;
			}

			control->initial_ಮೌಲ್ಯ = strdup(control->ಮೌಲ್ಯ);
			ಆದರೆ (control->initial_ಮೌಲ್ಯ == ಖಾಲಿ) {
				form_free_control(control);
				control = ಖಾಲಿ;
				ಹೋಗು ಹೊರಹೋಗು;
			}

			control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ = strdup(control->ಮೌಲ್ಯ);
			ಆದರೆ (control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ == ಖಾಲಿ) {
				form_free_control(control);
				control = ಖಾಲಿ;
				ಹೋಗು ಹೊರಹೋಗು;
			}

			control->ಗಂಟು_ಮೌಲ್ಯ = dom_string_ref(ds_ಮೌಲ್ಯ);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ಆದರೆ (form != ಖಾಲಿ && control != ಖಾಲಿ)
		form_ಕೂಡಿಸು_control(ಕಂಡುಹಿಡಿ_form(forms, form), control);

ಹೊರಹೋಗು:
	ಆದರೆ (form != ಖಾಲಿ)
		dom_ಗಂಟು_unref(form);
	ಆದರೆ (ds_type != ಖಾಲಿ)
		dom_string_unref(ds_type);
	ಆದರೆ (ds_name != ಖಾಲಿ)
		dom_string_unref(ds_name);
	ಆದರೆ (ds_ಮೌಲ್ಯ != ಖಾಲಿ)
		dom_string_unref(ds_ಮೌಲ್ಯ);

	ಆದರೆ (name != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(name);

	ಹಿಂತಿರುಗಿ control;
}

ಸ್ಥಿರ ರಚನೆ form_control *
parse_textarea_element(ರಚನೆ form *forms, dom_html_ಪಠ್ಯ_area_element *ta)
{
	ರಚನೆ form_control *control = ಖಾಲಿ;
	dom_html_form_element *form = ಖಾಲಿ;
	dom_string *ds_name = ಖಾಲಿ;

	ಅಕ್ಷರ *name = ಖಾಲಿ;

	ಆದರೆ (dom_html_ಪಠ್ಯ_area_element_get_form(ta, &form) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_ಪಠ್ಯ_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (ds_name != ಖಾಲಿ)
		name = strndup(dom_string_ದತ್ತಾಂಶ(ds_name),
			       dom_string_byte_ಉದ್ದಳತೆ(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ಆದರೆ (control == ಖಾಲಿ)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (name != ಖಾಲಿ) {
		/* Hand the name string over */
		control->name = name;
		name = ಖಾಲಿ;
	}

	ಆದರೆ (form != ಖಾಲಿ && control != ಖಾಲಿ)
		form_ಕೂಡಿಸು_control(ಕಂಡುಹಿಡಿ_form(forms, form), control);

ಹೊರಹೋಗು:
	ಆದರೆ (form != ಖಾಲಿ)
		dom_ಗಂಟು_unref(form);
	ಆದರೆ (ds_name != ಖಾಲಿ)
		dom_string_unref(ds_name);

	ಆದರೆ (name != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(name);


	ಹಿಂತಿರುಗಿ control;
}

ಸ್ಥಿರ ರಚನೆ form_control *
parse_select_element(ರಚನೆ form *forms, dom_html_select_element *select)
{
	ರಚನೆ form_control *control = ಖಾಲಿ;
	dom_html_form_element *form = ಖಾಲಿ;
	dom_string *ds_name = ಖಾಲಿ;

	ಅಕ್ಷರ *name = ಖಾಲಿ;

	ಆದರೆ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (ds_name != ಖಾಲಿ)
		name = strndup(dom_string_ದತ್ತಾಂಶ(ds_name),
			       dom_string_byte_ಉದ್ದಳತೆ(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ಆದರೆ (control == ಖಾಲಿ)
		ಹೋಗು ಹೊರಹೋಗು;

	ಆದರೆ (name != ಖಾಲಿ) {
		/* Hand the name string over */
		control->name = name;
		name = ಖಾಲಿ;
	}

	dom_html_select_element_get_ಅಪವರ್ತ್ಯ(select,
			&(control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ));

	ಆದರೆ (form != ಖಾಲಿ && control != ಖಾಲಿ)
		form_ಕೂಡಿಸು_control(ಕಂಡುಹಿಡಿ_form(forms, form), control);

ಹೊರಹೋಗು:
	ಆದರೆ (form != ಖಾಲಿ)
		dom_ಗಂಟು_unref(form);
	ಆದರೆ (ds_name != ಖಾಲಿ)
		dom_string_unref(ds_name);

	ಆದರೆ (name != ಖಾಲಿ)
		ಬಿಡುಗಡೆ(name);


	ಹಿಂತಿರುಗಿ control;
}


ಸ್ಥಿರ ರಚನೆ form_control *
invent_fake_gadget(dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ form_control *ctl = form_new_control(ಗಂಟು, GADGET_HIDDEN);
	ಆದರೆ (ctl != ಖಾಲಿ) {
		ctl->ಮೌಲ್ಯ = strdup("");
		ctl->initial_ಮೌಲ್ಯ = strdup("");
		ctl->name = strdup("foo");

		ಆದರೆ (ctl->ಮೌಲ್ಯ == ಖಾಲಿ || ctl->initial_ಮೌಲ್ಯ == ಖಾಲಿ ||
				ctl->name == ಖಾಲಿ) {
			form_free_control(ctl);
			ctl = ಖಾಲಿ;
		}
	}
	ಹಿಂತಿರುಗಿ ctl;
}

/* documented in html_internal.h */
ರಚನೆ form_control *
html_forms_get_control_for_ಗಂಟು(ರಚನೆ form *forms, dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ form *f;
	ರಚನೆ form_control *ctl = ಖಾಲಿ;
	dom_exception err;
	dom_string *ds_name = ಖಾಲಿ;

	/* Step one, see if we already have a control */
	ಗಾಗಿ (f = forms; f != ಖಾಲಿ; f = f->prev) {
		ಗಾಗಿ (ctl = f->controls; ctl != ಖಾಲಿ; ctl = ctl->ಮುಂದಿನ) {
			ಆದರೆ (ctl->ಗಂಟು == ಗಂಟು)
				ಹಿಂತಿರುಗಿ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_ಟ್ಯಾಗ್_name(ಗಂಟು, &ds_name);
	ಆದರೆ (err == DOM_NO_ERR && ds_name != ಖಾಲಿ) {

		/* Step three, attempt to work out what gadget to make */
		ಆದರೆ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ಗಂಟು);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ಒಳಹರಿವು)) {
			ctl = parse_ಒಳಹರಿವು_element(forms,
					(dom_html_ಒಳಹರಿವು_element *) ಗಂಟು);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_ಪಠ್ಯ_area_element *) ಗಂಟು);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ಗಂಟು);
		}
	}

	/* If all else fails, fake gadget time */
	ಆದರೆ (ctl == ಖಾಲಿ)
		ctl = invent_fake_gadget(ಗಂಟು);

	ಆದರೆ (ds_name != ಖಾಲಿ)
		dom_string_unref(ds_name);

	ಹಿಂತಿರುಗಿ ctl;
}
