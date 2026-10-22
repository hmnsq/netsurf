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
	सचे (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		गच्छ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	सचे (ds_action != रिक्त)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	सचे (ds_charset != रिक्त)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	सचे (ds_target != रिक्त)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	सचे (ds_method != रिक्त) {
		सचे (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			सचे (ds_enctype != रिक्त) {
				सचे (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ग्रन्थिः, action, target, method, charset, docenc);

out:
	सचे (ds_action != रिक्त)
		dom_string_unref(ds_action);
	सचे (ds_charset != रिक्त)
		dom_string_unref(ds_charset);
	सचे (ds_target != रिक्त)
		dom_string_unref(ds_target);
	सचे (ds_method != रिक्त)
		dom_string_unref(ds_method);
	सचे (ds_enctype != रिक्त)
		dom_string_unref(ds_enctype);
	सचे (action != रिक्त)
		मुक्त(action);
	सचे (charset != रिक्त)
		मुक्त(charset);
	सचे (target != रिक्त)
		मुक्त(target);
	पटिनिवत्त ret;
}

/* documented in html_internal.h */
संरचना form *html_forms_get_forms(ध्रुव वर्ण *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	संरचना form *ret = रिक्त, *newf;
	dom_ग्रन्थिः *ग्रन्थिः;
	unsigned long n;
	uint32_t nforms;

	सचे (doc == रिक्त)
		पटिनिवत्त रिक्त;

	/* Attempt to build a set of all the forms */
	सचे (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		पटिनिवत्त रिक्त;

	/* Count the number of forms so we can iterate */
	सचे (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		गच्छ out;

	/* Iterate the forms collection, making form structs for returning */
	अत्थाय (n = 0; n < nforms; ++n) {
		सचे (dom_html_collection_item(forms, n, &ग्रन्थिः) != DOM_NO_ERR) {
			गच्छ out;
		}
		newf = parse_form_element(docenc, ग्रन्थिः);
		dom_ग्रन्थिः_unref(ग्रन्थिः);
		सचे (newf == रिक्त) {
			गच्छ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	गच्छ out;
err:
	याव (ret != रिक्त) {
		संरचना form *prev = ret->prev;
		/* Destroy ret */
		मुक्त(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	पटिनिवत्त ret;
}

स्थिर संरचना form *
find_form(संरचना form *forms, dom_html_form_element *form)
{
	याव (forms != रिक्त) {
		सचे (forms->ग्रन्थिः == form)
			भङ्ग;
		forms = forms->prev;
	}

	पटिनिवत्त forms;
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
	सचे (err != DOM_NO_ERR)
		गच्छ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	सचे (err != DOM_NO_ERR)
		गच्छ out;

	सचे (ds_type == रिक्त) {
		control = form_new_control(button, GADGET_SUBMIT);
	} अञ्ञथा {
		सचे (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} अञ्ञथा {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	सचे (control == रिक्त)
		गच्छ out;

	err = dom_html_button_element_get_मानम्(button, &ds_मानम्);
	सचे (err != DOM_NO_ERR)
		गच्छ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	सचे (err != DOM_NO_ERR)
		गच्छ out;

	सचे (ds_मानम् != रिक्त) {
		control->मानम् = strndup(
			dom_string_data(ds_मानम्),
			dom_string_byte_length(ds_मानम्));

		सचे (control->मानम् == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			गच्छ out;
		}
	}

	सचे (ds_name != रिक्त) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		सचे (control->name == रिक्त) {
			form_free_control(control);
			control = रिक्त;
			गच्छ out;
		}
	}

	सचे (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	सचे (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	सचे (ds_type != रिक्त)
		dom_string_unref(ds_type);
	सचे (ds_मानम् != रिक्त)
		dom_string_unref(ds_मानम्);
	सचे (ds_name != रिक्त)
		dom_string_unref(ds_name);

	पटिनिवत्त control;
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

	सचे (dom_html_निवेशः_element_get_form(निवेशः, &form) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_निवेशः_element_get_type(निवेशः, &ds_type) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_निवेशः_element_get_name(निवेशः, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	सचे (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(निवेशः, GADGET_PASSWORD);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(निवेशः, GADGET_FILE);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(निवेशः, GADGET_HIDDEN);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(निवेशः, GADGET_CHECKBOX);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(निवेशः, GADGET_RADIO);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(निवेशः, GADGET_SUBMIT);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(निवेशः, GADGET_RESET);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(निवेशः, GADGET_BUTTON);
	} अञ्ञथा सचे (ds_type != रिक्त && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(निवेशः, GADGET_IMAGE);
	} अञ्ञथा {
		control = form_new_control(निवेशः, GADGET_TEXTBOX);
	}

	सचे (control == रिक्त)
		गच्छ out;

	सचे (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	सचे (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		सचे (dom_html_निवेशः_element_get_checked(
			    निवेशः, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	सचे (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		सचे (dom_html_निवेशः_element_get_max_length(
			    निवेशः, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		सचे (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} अञ्ञथा {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	सचे (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		सचे (dom_html_निवेशः_element_get_मानम्(
			    निवेशः, &ds_मानम्) == DOM_NO_ERR) {
			सचे (ds_मानम् != रिक्त) {
				control->मानम् = strndup(
					dom_string_data(ds_मानम्),
					dom_string_byte_length(ds_मानम्));
				सचे (control->मानम् == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					गच्छ out;
				}
				control->length = strlen(control->मानम्);
			}
		}

		सचे (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			सचे (control->मानम् == रिक्त) {
				control->मानम् = strdup("");
				सचे (control->मानम् == रिक्त) {
					form_free_control(control);
					control = रिक्त;
					गच्छ out;
				}

				control->length = 0;
			}

			control->initial_मानम् = strdup(control->मानम्);
			सचे (control->initial_मानम् == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				गच्छ out;
			}

			control->last_synced_मानम् = strdup(control->मानम्);
			सचे (control->last_synced_मानम् == रिक्त) {
				form_free_control(control);
				control = रिक्त;
				गच्छ out;
			}

			control->ग्रन्थिः_मानम् = dom_string_ref(ds_मानम्);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	सचे (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	सचे (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	सचे (ds_type != रिक्त)
		dom_string_unref(ds_type);
	सचे (ds_name != रिक्त)
		dom_string_unref(ds_name);
	सचे (ds_मानम् != रिक्त)
		dom_string_unref(ds_मानम्);

	सचे (name != रिक्त)
		मुक्त(name);

	पटिनिवत्त control;
}

स्थिर संरचना form_control *
parse_textarea_element(संरचना form *forms, dom_html_text_area_element *ta)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	वर्ण *name = रिक्त;

	सचे (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	सचे (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	सचे (control == रिक्त)
		गच्छ out;

	सचे (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	सचे (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	सचे (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	सचे (ds_name != रिक्त)
		dom_string_unref(ds_name);

	सचे (name != रिक्त)
		मुक्त(name);


	पटिनिवत्त control;
}

स्थिर संरचना form_control *
parse_select_element(संरचना form *forms, dom_html_select_element *select)
{
	संरचना form_control *control = रिक्त;
	dom_html_form_element *form = रिक्त;
	dom_string *ds_name = रिक्त;

	वर्ण *name = रिक्त;

	सचे (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		गच्छ out;

	सचे (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		गच्छ out;

	सचे (ds_name != रिक्त)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	सचे (control == रिक्त)
		गच्छ out;

	सचे (name != रिक्त) {
		/* Hand the name string over */
		control->name = name;
		name = रिक्त;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	सचे (form != रिक्त && control != रिक्त)
		form_add_control(find_form(forms, form), control);

out:
	सचे (form != रिक्त)
		dom_ग्रन्थिः_unref(form);
	सचे (ds_name != रिक्त)
		dom_string_unref(ds_name);

	सचे (name != रिक्त)
		मुक्त(name);


	पटिनिवत्त control;
}


स्थिर संरचना form_control *
invent_fake_gadget(dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना form_control *ctl = form_new_control(ग्रन्थिः, GADGET_HIDDEN);
	सचे (ctl != रिक्त) {
		ctl->मानम् = strdup("");
		ctl->initial_मानम् = strdup("");
		ctl->name = strdup("foo");

		सचे (ctl->मानम् == रिक्त || ctl->initial_मानम् == रिक्त ||
				ctl->name == रिक्त) {
			form_free_control(ctl);
			ctl = रिक्त;
		}
	}
	पटिनिवत्त ctl;
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
	अत्थाय (f = forms; f != रिक्त; f = f->prev) {
		अत्थाय (ctl = f->controls; ctl != रिक्त; ctl = ctl->next) {
			सचे (ctl->ग्रन्थिः == ग्रन्थिः)
				पटिनिवत्त ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(ग्रन्थिः, &ds_name);
	सचे (err == DOM_NO_ERR && ds_name != रिक्त) {

		/* Step three, attempt to work out what gadget to make */
		सचे (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ग्रन्थिः);
		} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_निवेशः)) {
			ctl = parse_निवेशः_element(forms,
					(dom_html_निवेशः_element *) ग्रन्थिः);
		} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) ग्रन्थिः);
		} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ग्रन्थिः);
		}
	}

	/* If all else fails, fake gadget time */
	सचे (ctl == रिक्त)
		ctl = invent_fake_gadget(ग्रन्थिः);

	सचे (ds_name != रिक्त)
		dom_string_unref(ds_name);

	पटिनिवत्त ctl;
}
