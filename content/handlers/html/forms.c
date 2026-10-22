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

#समावेश <string.h>

#समावेश "utils/config.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"

#समावेश "html/form_internal.h"
#समावेश "html/private.h"

/**
 * process form element from dom
 */
अटल काष्ठा form *
parse_form_element(स्थिर अक्षर *docenc, dom_गाँठ *गाँठ)
{
	dom_string *ds_action = शून्य;
	dom_string *ds_charset = शून्य;
	dom_string *ds_target = शून्य;
	dom_string *ds_method = शून्य;
	dom_string *ds_enctype = शून्य;
	अक्षर *action = शून्य, *charset = शून्य, *target = शून्य;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(गाँठ);
	काष्ठा form * ret = शून्य;

	/* Retrieve the attributes from the node */
	अगर (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		जाओ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	अगर (ds_action != शून्य)
		action = strndup(dom_string_तथ्य(ds_action),
				 dom_string_byte_लंबाई(ds_action));

	अगर (ds_charset != शून्य)
		charset = strndup(dom_string_तथ्य(ds_charset),
				  dom_string_byte_लंबाई(ds_charset));

	अगर (ds_target != शून्य)
		target = strndup(dom_string_तथ्य(ds_target),
				 dom_string_byte_लंबाई(ds_target));

	/* Determine the method */
	method = method_GET;
	अगर (ds_method != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			अगर (ds_enctype != शून्य) {
				अगर (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_तथ्य)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(गाँठ, action, target, method, charset, docenc);

out:
	अगर (ds_action != शून्य)
		dom_string_unref(ds_action);
	अगर (ds_charset != शून्य)
		dom_string_unref(ds_charset);
	अगर (ds_target != शून्य)
		dom_string_unref(ds_target);
	अगर (ds_method != शून्य)
		dom_string_unref(ds_method);
	अगर (ds_enctype != शून्य)
		dom_string_unref(ds_enctype);
	अगर (action != शून्य)
		मुक्त(action);
	अगर (charset != शून्य)
		मुक्त(charset);
	अगर (target != शून्य)
		मुक्त(target);
	वापस ret;
}

/* documented in html_internal.h */
काष्ठा form *html_forms_get_forms(स्थिर अक्षर *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	काष्ठा form *ret = शून्य, *newf;
	dom_गाँठ *गाँठ;
	अचिन्हित दीर्घ n;
	uint32_t nforms;

	अगर (doc == शून्य)
		वापस शून्य;

	/* Attempt to build a set of all the forms */
	अगर (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		वापस शून्य;

	/* Count the number of forms so we can iterate */
	अगर (dom_html_collection_get_लंबाई(forms, &nforms) != DOM_NO_ERR)
		जाओ out;

	/* Iterate the forms collection, making form structs for returning */
	क्रम (n = 0; n < nforms; ++n) {
		अगर (dom_html_collection_item(forms, n, &गाँठ) != DOM_NO_ERR) {
			जाओ out;
		}
		newf = parse_form_element(docenc, गाँठ);
		dom_गाँठ_unref(गाँठ);
		अगर (newf == शून्य) {
			जाओ err;
		}
		newf->पूर्व = ret;
		ret = newf;
	}

	/* All went well */
	जाओ out;
err:
	जबतक (ret != शून्य) {
		काष्ठा form *पूर्व = ret->पूर्व;
		/* Destroy ret */
		मुक्त(ret);
		ret = पूर्व;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	वापस ret;
}

अटल काष्ठा form *
find_form(काष्ठा form *forms, dom_html_form_element *form)
{
	जबतक (forms != शून्य) {
		अगर (forms->गाँठ == form)
			अवरोध;
		forms = forms->पूर्व;
	}

	वापस forms;
}

अटल काष्ठा form_control *
parse_button_element(काष्ठा form *forms, dom_html_button_element *button)
{
	काष्ठा form_control *control = शून्य;
	dom_exception err;
	dom_html_form_element *form = शून्य;
	dom_string *ds_type = शून्य;
	dom_string *ds_मूल्य = शून्य;
	dom_string *ds_name = शून्य;

	err = dom_html_button_element_get_form(button, &form);
	अगर (err != DOM_NO_ERR)
		जाओ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	अगर (err != DOM_NO_ERR)
		जाओ out;

	अगर (ds_type == शून्य) {
		control = form_new_control(button, GADGET_SUBMIT);
	} अन्यथा {
		अगर (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} अन्यथा अगर (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} अन्यथा {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	अगर (control == शून्य)
		जाओ out;

	err = dom_html_button_element_get_मूल्य(button, &ds_मूल्य);
	अगर (err != DOM_NO_ERR)
		जाओ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	अगर (err != DOM_NO_ERR)
		जाओ out;

	अगर (ds_मूल्य != शून्य) {
		control->मूल्य = strndup(
			dom_string_तथ्य(ds_मूल्य),
			dom_string_byte_लंबाई(ds_मूल्य));

		अगर (control->मूल्य == शून्य) {
			form_free_control(control);
			control = शून्य;
			जाओ out;
		}
	}

	अगर (ds_name != शून्य) {
		control->name = strndup(
			dom_string_तथ्य(ds_name),
			dom_string_byte_लंबाई(ds_name));

		अगर (control->name == शून्य) {
			form_free_control(control);
			control = शून्य;
			जाओ out;
		}
	}

	अगर (form != शून्य && control != शून्य)
		form_add_control(find_form(forms, form), control);

out:
	अगर (form != शून्य)
		dom_गाँठ_unref(form);
	अगर (ds_type != शून्य)
		dom_string_unref(ds_type);
	अगर (ds_मूल्य != शून्य)
		dom_string_unref(ds_मूल्य);
	अगर (ds_name != शून्य)
		dom_string_unref(ds_name);

	वापस control;
}

अटल काष्ठा form_control *
parse_निवेश_element(काष्ठा form *forms, dom_html_निवेश_element *निवेश)
{
	काष्ठा form_control *control = शून्य;
	dom_html_form_element *form = शून्य;
	dom_string *ds_type = शून्य;
	dom_string *ds_name = शून्य;
	dom_string *ds_मूल्य = शून्य;

	अक्षर *name = शून्य;

	अगर (dom_html_निवेश_element_get_form(निवेश, &form) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_निवेश_element_get_type(निवेश, &ds_type) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_निवेश_element_get_name(निवेश, &ds_name) != DOM_NO_ERR)
		जाओ out;

	अगर (ds_name != शून्य)
		name = strndup(dom_string_तथ्य(ds_name),
			       dom_string_byte_लंबाई(ds_name));

	अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(निवेश, GADGET_PASSWORD);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(निवेश, GADGET_FILE);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(निवेश, GADGET_HIDDEN);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(निवेश, GADGET_CHECKBOX);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(निवेश, GADGET_RADIO);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(निवेश, GADGET_SUBMIT);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(निवेश, GADGET_RESET);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(निवेश, GADGET_BUTTON);
	} अन्यथा अगर (ds_type != शून्य && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(निवेश, GADGET_IMAGE);
	} अन्यथा {
		control = form_new_control(निवेश, GADGET_TEXTBOX);
	}

	अगर (control == शून्य)
		जाओ out;

	अगर (name != शून्य) {
		/* Hand the name string over */
		control->name = name;
		name = शून्य;
	}

	अगर (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		अगर (dom_html_निवेश_element_get_checked(
			    निवेश, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	अगर (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		अगर (dom_html_निवेश_element_get_max_लंबाई(
			    निवेश, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		अगर (maxlength >= 0) {
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

	अगर (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		अगर (dom_html_निवेश_element_get_मूल्य(
			    निवेश, &ds_मूल्य) == DOM_NO_ERR) {
			अगर (ds_मूल्य != शून्य) {
				control->मूल्य = strndup(
					dom_string_तथ्य(ds_मूल्य),
					dom_string_byte_लंबाई(ds_मूल्य));
				अगर (control->मूल्य == शून्य) {
					form_free_control(control);
					control = शून्य;
					जाओ out;
				}
				control->लंबाई = म_माप(control->मूल्य);
			}
		}

		अगर (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			अगर (control->मूल्य == शून्य) {
				control->मूल्य = strdup("");
				अगर (control->मूल्य == शून्य) {
					form_free_control(control);
					control = शून्य;
					जाओ out;
				}

				control->लंबाई = 0;
			}

			control->initial_मूल्य = strdup(control->मूल्य);
			अगर (control->initial_मूल्य == शून्य) {
				form_free_control(control);
				control = शून्य;
				जाओ out;
			}

			control->last_synced_मूल्य = strdup(control->मूल्य);
			अगर (control->last_synced_मूल्य == शून्य) {
				form_free_control(control);
				control = शून्य;
				जाओ out;
			}

			control->गाँठ_मूल्य = dom_string_ref(ds_मूल्य);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	अगर (form != शून्य && control != शून्य)
		form_add_control(find_form(forms, form), control);

out:
	अगर (form != शून्य)
		dom_गाँठ_unref(form);
	अगर (ds_type != शून्य)
		dom_string_unref(ds_type);
	अगर (ds_name != शून्य)
		dom_string_unref(ds_name);
	अगर (ds_मूल्य != शून्य)
		dom_string_unref(ds_मूल्य);

	अगर (name != शून्य)
		मुक्त(name);

	वापस control;
}

अटल काष्ठा form_control *
parse_textarea_element(काष्ठा form *forms, dom_html_text_area_element *ta)
{
	काष्ठा form_control *control = शून्य;
	dom_html_form_element *form = शून्य;
	dom_string *ds_name = शून्य;

	अक्षर *name = शून्य;

	अगर (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		जाओ out;

	अगर (ds_name != शून्य)
		name = strndup(dom_string_तथ्य(ds_name),
			       dom_string_byte_लंबाई(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	अगर (control == शून्य)
		जाओ out;

	अगर (name != शून्य) {
		/* Hand the name string over */
		control->name = name;
		name = शून्य;
	}

	अगर (form != शून्य && control != शून्य)
		form_add_control(find_form(forms, form), control);

out:
	अगर (form != शून्य)
		dom_गाँठ_unref(form);
	अगर (ds_name != शून्य)
		dom_string_unref(ds_name);

	अगर (name != शून्य)
		मुक्त(name);


	वापस control;
}

अटल काष्ठा form_control *
parse_select_element(काष्ठा form *forms, dom_html_select_element *select)
{
	काष्ठा form_control *control = शून्य;
	dom_html_form_element *form = शून्य;
	dom_string *ds_name = शून्य;

	अक्षर *name = शून्य;

	अगर (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		जाओ out;

	अगर (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		जाओ out;

	अगर (ds_name != शून्य)
		name = strndup(dom_string_तथ्य(ds_name),
			       dom_string_byte_लंबाई(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	अगर (control == शून्य)
		जाओ out;

	अगर (name != शून्य) {
		/* Hand the name string over */
		control->name = name;
		name = शून्य;
	}

	dom_html_select_element_get_multiple(select,
			&(control->तथ्य.select.multiple));

	अगर (form != शून्य && control != शून्य)
		form_add_control(find_form(forms, form), control);

out:
	अगर (form != शून्य)
		dom_गाँठ_unref(form);
	अगर (ds_name != शून्य)
		dom_string_unref(ds_name);

	अगर (name != शून्य)
		मुक्त(name);


	वापस control;
}


अटल काष्ठा form_control *
invent_fake_gadget(dom_गाँठ *गाँठ)
{
	काष्ठा form_control *ctl = form_new_control(गाँठ, GADGET_HIDDEN);
	अगर (ctl != शून्य) {
		ctl->मूल्य = strdup("");
		ctl->initial_मूल्य = strdup("");
		ctl->name = strdup("foo");

		अगर (ctl->मूल्य == शून्य || ctl->initial_मूल्य == शून्य ||
				ctl->name == शून्य) {
			form_free_control(ctl);
			ctl = शून्य;
		}
	}
	वापस ctl;
}

/* documented in html_internal.h */
काष्ठा form_control *
html_forms_get_control_for_गाँठ(काष्ठा form *forms, dom_गाँठ *गाँठ)
{
	काष्ठा form *f;
	काष्ठा form_control *ctl = शून्य;
	dom_exception err;
	dom_string *ds_name = शून्य;

	/* Step one, see if we already have a control */
	क्रम (f = forms; f != शून्य; f = f->पूर्व) {
		क्रम (ctl = f->controls; ctl != शून्य; ctl = ctl->अग्र) {
			अगर (ctl->गाँठ == गाँठ)
				वापस ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(गाँठ, &ds_name);
	अगर (err == DOM_NO_ERR && ds_name != शून्य) {

		/* Step three, attempt to work out what gadget to make */
		अगर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) गाँठ);
		} अन्यथा अगर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_निवेश)) {
			ctl = parse_निवेश_element(forms,
					(dom_html_निवेश_element *) गाँठ);
		} अन्यथा अगर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) गाँठ);
		} अन्यथा अगर (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) गाँठ);
		}
	}

	/* If all else fails, fake gadget time */
	अगर (ctl == शून्य)
		ctl = invent_fake_gadget(गाँठ);

	अगर (ds_name != शून्य)
		dom_string_unref(ds_name);

	वापस ctl;
}
