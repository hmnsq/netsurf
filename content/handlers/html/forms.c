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

#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"

#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/private.h"

/**
 * process form element from dom
 */
स्थिर संरचना form *
parse_form_element(ध्रुव वर्ण *docenc, dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_string *ds_action = रिक्त;
	dom_string *ds_charset = रिक्त;
	dom_string *ds_target = रिक्त;
	dom_string *ds_method = रिक्त;
	dom_string *ds_enctype = रिक्त;
	वर्ण *action = रिक्त, *charset = रिक्त, *target = रिक्त;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(ग्रन्थिः);
	संरचना form * ret = रिक्त;

	/* Retrieve the attributes from the node */
	जइ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		गच्छ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	जइ (ds_action != रिक्त)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	जइ (ds_charset != रिक्त)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	जइ (ds_target != रिक्त)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	जइ (ds_method != रिक्त) {
		जइ (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			जइ (ds_enctype != रिक्त) {
				जइ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ग्रन्थिः, action, target, method, charset, docenc);

out:
	जइ (ds_action != रिक्त)
		dom_string_unref(ds_action);
	जइ (ds_charset != रिक्त)
		dom_string_unref(ds_charset);
	जइ (ds_target != रिक्त)
		dom_string_unref(ds_target);
	जइ (ds_method != रिक्त)
		dom_string_unref(ds_method);
	जइ (ds_enctype != रिक्त)
		dom_string_unref(ds_enctype);
	जइ (action != रिक्त)
		मुक्त(action);
	जइ (charset != रिक्त)
		मुक्त(charset);
	जइ (target != रिक्त)
		मुक्त(target);
	प्रत्यागम ret;
}

/* documented in html_internal.h */
संरचना form *html_forms_get_forms(ध्रुव वर्ण *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	संरचना form *ret = रिक्त, *newf;
	dom_ग्रन्थिः *ग्रन्थिः;
	unsigned long n;
	uint32_t nforms;

	जइ (doc == रिक्त)
		प्रत्यागम रिक्त;

	/* Attempt to build a set of all the forms */
	जइ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		प्रत्यागम रिक्त;

	/* Count the number of forms so we can iterate */
	जइ (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		गच्छ out;

	/* Iterate the forms collection, making form structs for returning */
	प्रति (n = 0; n < nforms; ++n) {
		जइ (dom_html_collection_item(forms, n, &ग्रन्थिः) != DOM_NO_ERR) {
			गच्छ out;
		}
		newf = parse_form_element(docenc, ग्रन्थिः);
		dom_ग्रन्थिः_unref(ग्रन्थिः);
		जइ (newf == रिक्त) {
			गच्छ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	गच्छ out;
err:
	जाव (ret != रिक्त) {
		संरचना form *prev = ret->prev;
		/* Destroy ret */
		मुक्त(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	प्रत्यागम ret;
}

स्थिर संरचना form *
find_form(संरचना form *forms, dom_html_form_element *form)
{
	जाव (forms != रिक्त) {
		जइ (forms->ग्रन्थिः == form)
			भङ्ग;
		forms = forms->prev;
	}

	प्रत्यागम forms;
}

स्थिर संरचना form_control *
parse_button_element(संरचना form *forms, dom_html_button_element *button)
{
	संरचना form_control *control = रिक्त;
	dom_exception err;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_type = रिक्त;
	dom_string *ds_मानम् = रिक्त;
	dom_string *ds_name = रिक्त;

	err = dom_html_button_element_get_form(button, &form);
	जइ (err != DOM_NO_ERR)
		गच्छ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	जइ (err != DOM_NO_ERR)
		गच्छ out;

	जइ (ds_type == रिक्त) {
		control = form_new_control(button, GADGET_SUBMIT);
	} अण्णहा {
		जइ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} अण्णहा जइ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} अण्णहा {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	जइ (control == रिक्त)
		गच्छ out;

	err = dom_html_button_element_get_मानम्(button, &ds_मानम्);
	जइ (err != DOM_NO_ERR)
		गच्छ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	जइ (err != DOM_NO_ERR)
		गच्छ out;

	जइ (ds_मानम् != रिक्त) {
		control->मानम् = strndup(
			dom_string_data(ds_मानम्),
			dom_string_byte_length(ds_मानम्));

		जइ (control->मानम् == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			गच्छ out;
		}
	}

	जइ (ds_name != रिक्त) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		जइ (control->name == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			गच्छ out;
		}
	}

	जइ (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जइ (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	जइ (ds_type != रिक्त)
		dom_string_unref(ds_type);
	जइ (ds_मानम् != रिक्त)
		dom_string_unref(ds_मानम्);
	जइ (ds_name != रिक्त)
		dom_string_unref(ds_name);

	प्रत्यागम control;
}

स्थिर संरचना form_control *
parse_निवेशः_element(संरचना form *forms, dom_html_निवेशः_element *निवेशः)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_type = रिक्त;
	dom_string *ds_name = रिक्त;
	dom_string *ds_मानम् = रिक्त;

	वर्ण *name = रिक्त;

	जइ (dom_html_निवेशः_element_get_form(निवेशः, &form) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_निवेशः_element_get_type(निवेशः, &ds_type) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_निवेशः_element_get_name(निवेशः, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	जइ (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(निवेशः, GADGET_PASSWORD);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(निवेशः, GADGET_FILE);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(निवेशः, GADGET_HIDDEN);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(निवेशः, GADGET_CHECKBOX);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(निवेशः, GADGET_RADIO);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(निवेशः, GADGET_SUBMIT);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(निवेशः, GADGET_RESET);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(निवेशः, GADGET_BUTTON);
	} अण्णहा जइ (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(निवेशः, GADGET_IMAGE);
	} अण्णहा {
		control = form_new_control(निवेशः, GADGET_TEXTBOX);
	}

	जइ (control == रिक्त)
		गच्छ out;

	जइ (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	जइ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		जइ (dom_html_निवेशः_element_get_checked(
			    निवेशः, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	जइ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		जइ (dom_html_निवेशः_element_get_max_length(
			    निवेशः, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		जइ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} अण्णहा {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	जइ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		जइ (dom_html_निवेशः_element_get_मानम्(
			    निवेशः, &ds_मानम्) == DOM_NO_ERR) {
			जइ (ds_मानम् != रिक्त) {
				control->मानम् = strndup(
					dom_string_data(ds_मानम्),
					dom_string_byte_length(ds_मानम्));
				जइ (control->मानम् == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					गच्छ out;
				}
				control->length = strlen(control->मानम्);
			}
		}

		जइ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			जइ (control->मानम् == रिक्त) {
				control->मानम् = strdup("");
				जइ (control->मानम् == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					गच्छ out;
				}

				control->length = 0;
			}

			control->initial_मानम् = strdup(control->मानम्);
			जइ (control->initial_मानम् == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				गच्छ out;
			}

			control->last_synced_मानम् = strdup(control->मानम्);
			जइ (control->last_synced_मानम् == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				गच्छ out;
			}

			control->ग्रन्थिः_मानम् = dom_string_ref(ds_मानम्);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	जइ (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जइ (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	जइ (ds_type != रिक्त)
		dom_string_unref(ds_type);
	जइ (ds_name != रिक्त)
		dom_string_unref(ds_name);
	जइ (ds_मानम् != रिक्त)
		dom_string_unref(ds_मानम्);

	जइ (name != रिक्त)
		मुक्त(name);

	प्रत्यागम control;
}

स्थिर संरचना form_control *
parse_textarea_element(संरचना form *forms, dom_html_text_area_element *ta)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	वर्ण *name = रिक्त;

	जइ (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	जइ (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	जइ (control == रिक्त)
		गच्छ out;

	जइ (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	जइ (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जइ (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	जइ (ds_name != रिक्त)
		dom_string_unref(ds_name);

	जइ (name != रिक्त)
		मुक्त(name);


	प्रत्यागम control;
}

स्थिर संरचना form_control *
parse_select_element(संरचना form *forms, dom_html_select_element *select)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	वर्ण *name = रिक्त;

	जइ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		गच्छ out;

	जइ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	जइ (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	जइ (control == रिक्त)
		गच्छ out;

	जइ (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	जइ (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जइ (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	जइ (ds_name != रिक्त)
		dom_string_unref(ds_name);

	जइ (name != रिक्त)
		मुक्त(name);


	प्रत्यागम control;
}


स्थिर संरचना form_control *
invent_fake_gadget(dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना form_control *ctl = form_new_control(ग्रन्थिः, GADGET_HIDDEN);
	जइ (ctl != रिक्त) {
		ctl->मानम् = strdup("");
		ctl->initial_मानम् = strdup("");
		ctl->name = strdup("foo");

		जइ (ctl->मानम् == रिक्त || ctl->initial_मानम् == रिक्त ||
				ctl->name == रिक्त) {
			form_free_control(ctl);
			ctl = रिक्त;
		}
	}
	प्रत्यागम ctl;
}

/* documented in html_internal.h */
संरचना form_control *
html_forms_get_control_for_ग्रन्थिः(संरचना form *forms, dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना form *f;
	संरचना form_control *ctl = रिक्त;
	dom_exception err;
	dom_string *ds_name = रिक्त;

	/* Step one, see if we already have a control */
	प्रति (f = forms; f != रिक्त; f = f->prev) {
		प्रति (ctl = f->controls; ctl != रिक्त; ctl = ctl->next) {
			जइ (ctl->ग्रन्थिः == ग्रन्थिः)
				प्रत्यागम ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(ग्रन्थिः, &ds_name);
	जइ (err == DOM_NO_ERR && ds_name != रिक्त) {

		/* Step three, attempt to work out what gadget to make */
		जइ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ग्रन्थिः);
		} अण्णहा जइ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_निवेशः)) {
			ctl = parse_निवेशः_element(forms,
					(dom_html_निवेशः_element *) ग्रन्थिः);
		} अण्णहा जइ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) ग्रन्थिः);
		} अण्णहा जइ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ग्रन्थिः);
		}
	}

	/* If all else fails, fake gadget time */
	जइ (ctl == रिक्त)
		ctl = invent_fake_gadget(ग्रन्थिः);

	जइ (ds_name != रिक्त)
		dom_string_unref(ds_name);

	प्रत्यागम ctl;
}
