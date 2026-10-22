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
parse_form_element(स्थिरांक अक्षर *docenc, dom_गाठ *गाठ)
{
	dom_string *ds_action = रिक्त;
	dom_string *ds_charset = रिक्त;
	dom_string *ds_target = रिक्त;
	dom_string *ds_method = रिक्त;
	dom_string *ds_enctype = रिक्त;
	अक्षर *action = रिक्त, *charset = रिक्त, *target = रिक्त;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(गाठ);
	संरचना form * ret = रिक्त;

	/* Retrieve the attributes from the node */
	जर (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		जा out;

	जर (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		जा out;

	जर (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		जा out;

	जर (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		जा out;

	जर (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		जा out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	जर (ds_action != रिक्त)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	जर (ds_charset != रिक्त)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	जर (ds_target != रिक्त)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	जर (ds_method != रिक्त) {
		जर (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			जर (ds_enctype != रिक्त) {
				जर (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(गाठ, action, target, method, charset, docenc);

out:
	जर (ds_action != रिक्त)
		dom_string_unref(ds_action);
	जर (ds_charset != रिक्त)
		dom_string_unref(ds_charset);
	जर (ds_target != रिक्त)
		dom_string_unref(ds_target);
	जर (ds_method != रिक्त)
		dom_string_unref(ds_method);
	जर (ds_enctype != रिक्त)
		dom_string_unref(ds_enctype);
	जर (action != रिक्त)
		मुक्त(action);
	जर (charset != रिक्त)
		मुक्त(charset);
	जर (target != रिक्त)
		मुक्त(target);
	परत ret;
}

/* documented in html_internal.h */
संरचना form *html_forms_get_forms(स्थिरांक अक्षर *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	संरचना form *ret = रिक्त, *newf;
	dom_गाठ *गाठ;
	अचिन्हित दीर्घ n;
	uint32_t nforms;

	जर (doc == रिक्त)
		परत रिक्त;

	/* Attempt to build a set of all the forms */
	जर (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		परत रिक्त;

	/* Count the number of forms so we can iterate */
	जर (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		जा out;

	/* Iterate the forms collection, making form structs for returning */
	साठी (n = 0; n < nforms; ++n) {
		जर (dom_html_collection_item(forms, n, &गाठ) != DOM_NO_ERR) {
			जा out;
		}
		newf = parse_form_element(docenc, गाठ);
		dom_गाठ_unref(गाठ);
		जर (newf == रिक्त) {
			जा err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	जा out;
err:
	पर्यंत (ret != रिक्त) {
		संरचना form *prev = ret->prev;
		/* Destroy ret */
		मुक्त(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	परत ret;
}

स्थिर संरचना form *
find_form(संरचना form *forms, dom_html_form_element *form)
{
	पर्यंत (forms != रिक्त) {
		जर (forms->गाठ == form)
			थांबा;
		forms = forms->prev;
	}

	परत forms;
}

स्थिर संरचना form_control *
parse_button_element(संरचना form *forms, dom_html_button_element *button)
{
	संरचना form_control *control = रिक्त;
	dom_exception err;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_type = रिक्त;
	dom_string *ds_मूल्य = रिक्त;
	dom_string *ds_name = रिक्त;

	err = dom_html_button_element_get_form(button, &form);
	जर (err != DOM_NO_ERR)
		जा out;

	err = dom_html_button_element_get_type(button, &ds_type);
	जर (err != DOM_NO_ERR)
		जा out;

	जर (ds_type == रिक्त) {
		control = form_new_control(button, GADGET_SUBMIT);
	} अन्यथा {
		जर (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} अन्यथा जर (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} अन्यथा {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	जर (control == रिक्त)
		जा out;

	err = dom_html_button_element_get_मूल्य(button, &ds_मूल्य);
	जर (err != DOM_NO_ERR)
		जा out;
	err = dom_html_button_element_get_name(button, &ds_name);
	जर (err != DOM_NO_ERR)
		जा out;

	जर (ds_मूल्य != रिक्त) {
		control->मूल्य = strndup(
			dom_string_data(ds_मूल्य),
			dom_string_byte_length(ds_मूल्य));

		जर (control->मूल्य == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			जा out;
		}
	}

	जर (ds_name != रिक्त) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		जर (control->name == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			जा out;
		}
	}

	जर (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जर (form != रिक्त)
		dom_गाठ_unref(form);
	जर (ds_type != रिक्त)
		dom_string_unref(ds_type);
	जर (ds_मूल्य != रिक्त)
		dom_string_unref(ds_मूल्य);
	जर (ds_name != रिक्त)
		dom_string_unref(ds_name);

	परत control;
}

स्थिर संरचना form_control *
parse_आदान_element(संरचना form *forms, dom_html_आदान_element *आदान)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_type = रिक्त;
	dom_string *ds_name = रिक्त;
	dom_string *ds_मूल्य = रिक्त;

	अक्षर *name = रिक्त;

	जर (dom_html_आदान_element_get_form(आदान, &form) != DOM_NO_ERR)
		जा out;

	जर (dom_html_आदान_element_get_type(आदान, &ds_type) != DOM_NO_ERR)
		जा out;

	जर (dom_html_आदान_element_get_name(आदान, &ds_name) != DOM_NO_ERR)
		जा out;

	जर (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(आदान, GADGET_PASSWORD);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(आदान, GADGET_FILE);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(आदान, GADGET_HIDDEN);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(आदान, GADGET_CHECKBOX);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(आदान, GADGET_RADIO);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(आदान, GADGET_SUBMIT);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(आदान, GADGET_RESET);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(आदान, GADGET_BUTTON);
	} अन्यथा जर (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(आदान, GADGET_IMAGE);
	} अन्यथा {
		control = form_new_control(आदान, GADGET_TEXTBOX);
	}

	जर (control == रिक्त)
		जा out;

	जर (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	जर (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		जर (dom_html_आदान_element_get_checked(
			    आदान, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	जर (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		जर (dom_html_आदान_element_get_max_length(
			    आदान, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		जर (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} अन्यथा {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = अच_पूर्णांक_उच्च;
		}
	}

	जर (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		जर (dom_html_आदान_element_get_मूल्य(
			    आदान, &ds_मूल्य) == DOM_NO_ERR) {
			जर (ds_मूल्य != रिक्त) {
				control->मूल्य = strndup(
					dom_string_data(ds_मूल्य),
					dom_string_byte_length(ds_मूल्य));
				जर (control->मूल्य == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					जा out;
				}
				control->length = म_माप(control->मूल्य);
			}
		}

		जर (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			जर (control->मूल्य == रिक्त) {
				control->मूल्य = strdup("");
				जर (control->मूल्य == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					जा out;
				}

				control->length = 0;
			}

			control->initial_मूल्य = strdup(control->मूल्य);
			जर (control->initial_मूल्य == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				जा out;
			}

			control->last_synced_मूल्य = strdup(control->मूल्य);
			जर (control->last_synced_मूल्य == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				जा out;
			}

			control->गाठ_मूल्य = dom_string_ref(ds_मूल्य);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	जर (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जर (form != रिक्त)
		dom_गाठ_unref(form);
	जर (ds_type != रिक्त)
		dom_string_unref(ds_type);
	जर (ds_name != रिक्त)
		dom_string_unref(ds_name);
	जर (ds_मूल्य != रिक्त)
		dom_string_unref(ds_मूल्य);

	जर (name != रिक्त)
		मुक्त(name);

	परत control;
}

स्थिर संरचना form_control *
parse_textarea_element(संरचना form *forms, dom_html_text_area_element *ta)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	अक्षर *name = रिक्त;

	जर (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		जा out;

	जर (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		जा out;

	जर (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	जर (control == रिक्त)
		जा out;

	जर (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	जर (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जर (form != रिक्त)
		dom_गाठ_unref(form);
	जर (ds_name != रिक्त)
		dom_string_unref(ds_name);

	जर (name != रिक्त)
		मुक्त(name);


	परत control;
}

स्थिर संरचना form_control *
parse_select_element(संरचना form *forms, dom_html_select_element *select)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	अक्षर *name = रिक्त;

	जर (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		जा out;

	जर (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		जा out;

	जर (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	जर (control == रिक्त)
		जा out;

	जर (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	जर (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	जर (form != रिक्त)
		dom_गाठ_unref(form);
	जर (ds_name != रिक्त)
		dom_string_unref(ds_name);

	जर (name != रिक्त)
		मुक्त(name);


	परत control;
}


स्थिर संरचना form_control *
invent_fake_gadget(dom_गाठ *गाठ)
{
	संरचना form_control *ctl = form_new_control(गाठ, GADGET_HIDDEN);
	जर (ctl != रिक्त) {
		ctl->मूल्य = strdup("");
		ctl->initial_मूल्य = strdup("");
		ctl->name = strdup("foo");

		जर (ctl->मूल्य == रिक्त || ctl->initial_मूल्य == रिक्त ||
				ctl->name == रिक्त) {
			form_free_control(ctl);
			ctl = रिक्त;
		}
	}
	परत ctl;
}

/* documented in html_internal.h */
संरचना form_control *
html_forms_get_control_for_गाठ(संरचना form *forms, dom_गाठ *गाठ)
{
	संरचना form *f;
	संरचना form_control *ctl = रिक्त;
	dom_exception err;
	dom_string *ds_name = रिक्त;

	/* Step one, see if we already have a control */
	साठी (f = forms; f != रिक्त; f = f->prev) {
		साठी (ctl = f->controls; ctl != रिक्त; ctl = ctl->next) {
			जर (ctl->गाठ == गाठ)
				परत ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(गाठ, &ds_name);
	जर (err == DOM_NO_ERR && ds_name != रिक्त) {

		/* Step three, attempt to work out what gadget to make */
		जर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) गाठ);
		} अन्यथा जर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_आदान)) {
			ctl = parse_आदान_element(forms,
					(dom_html_आदान_element *) गाठ);
		} अन्यथा जर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) गाठ);
		} अन्यथा जर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) गाठ);
		}
	}

	/* If all else fails, fake gadget time */
	जर (ctl == रिक्त)
		ctl = invent_fake_gadget(गाठ);

	जर (ds_name != रिक्त)
		dom_string_unref(ds_name);

	परत ctl;
}
