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

#ضمن <string.h>

#ضمن "utils/config.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"

#ضمن "html/form_internal.h"
#ضمن "html/private.h"

/**
 * process form element from dom
 */
ساكن بنية form *
parse_form_element(ثابت حرف *docenc, dom_node *node)
{
	dom_string *ds_action = عدم;
	dom_string *ds_charset = عدم;
	dom_string *ds_target = عدم;
	dom_string *ds_method = عدم;
	dom_string *ds_enctype = عدم;
	حرف *action = عدم, *charset = عدم, *target = عدم;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(node);
	بنية form * ret = عدم;

	/* Retrieve the attributes from the node */
	ܐܢ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		اذهب out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ܐܢ (ds_action != عدم)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	ܐܢ (ds_charset != عدم)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	ܐܢ (ds_target != عدم)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	ܐܢ (ds_method != عدم) {
		ܐܢ (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			ܐܢ (ds_enctype != عدم) {
				ܐܢ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(node, action, target, method, charset, docenc);

out:
	ܐܢ (ds_action != عدم)
		dom_string_unref(ds_action);
	ܐܢ (ds_charset != عدم)
		dom_string_unref(ds_charset);
	ܐܢ (ds_target != عدم)
		dom_string_unref(ds_target);
	ܐܢ (ds_method != عدم)
		dom_string_unref(ds_method);
	ܐܢ (ds_enctype != عدم)
		dom_string_unref(ds_enctype);
	ܐܢ (action != عدم)
		حرر(action);
	ܐܢ (charset != عدم)
		حرر(charset);
	ܐܢ (target != عدم)
		حرر(target);
	ܦܢܐ ret;
}

/* documented in html_internal.h */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	بنية form *ret = عدم, *newf;
	dom_node *node;
	unsigned long n;
	uint32_t nforms;

	ܐܢ (doc == عدم)
		ܦܢܐ عدم;

	/* Attempt to build a set of all the forms */
	ܐܢ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ܦܢܐ عدم;

	/* Count the number of forms so we can iterate */
	ܐܢ (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		اذهب out;

	/* Iterate the forms collection, making form structs for returning */
	لكل (n = 0; n < nforms; ++n) {
		ܐܢ (dom_html_collection_item(forms, n, &node) != DOM_NO_ERR) {
			اذهب out;
		}
		newf = parse_form_element(docenc, node);
		dom_node_unref(node);
		ܐܢ (newf == عدم) {
			اذهب err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	اذهب out;
err:
	ܟܡܐ (ret != عدم) {
		بنية form *prev = ret->prev;
		/* Destroy ret */
		حرر(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ܦܢܐ ret;
}

ساكن بنية form *
find_form(بنية form *forms, dom_html_form_element *form)
{
	ܟܡܐ (forms != عدم) {
		ܐܢ (forms->node == form)
			اكسر;
		forms = forms->prev;
	}

	ܦܢܐ forms;
}

ساكن بنية form_control *
parse_button_element(بنية form *forms, dom_html_button_element *button)
{
	بنية form_control *control = عدم;
	dom_exception err;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_value = عدم;
	dom_string *ds_name = عدم;

	err = dom_html_button_element_get_form(button, &form);
	ܐܢ (err != DOM_NO_ERR)
		اذهب out;

	err = dom_html_button_element_get_type(button, &ds_type);
	ܐܢ (err != DOM_NO_ERR)
		اذهب out;

	ܐܢ (ds_type == عدم) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ܐܚܪܢܐ {
		ܐܢ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ܐܚܪܢܐ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ܐܢ (control == عدم)
		اذهب out;

	err = dom_html_button_element_get_value(button, &ds_value);
	ܐܢ (err != DOM_NO_ERR)
		اذهب out;
	err = dom_html_button_element_get_name(button, &ds_name);
	ܐܢ (err != DOM_NO_ERR)
		اذهب out;

	ܐܢ (ds_value != عدم) {
		control->value = strndup(
			dom_string_data(ds_value),
			dom_string_byte_length(ds_value));

		ܐܢ (control->value == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب out;
		}
	}

	ܐܢ (ds_name != عدم) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		ܐܢ (control->name == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب out;
		}
	}

	ܐܢ (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	ܐܢ (form != عدم)
		dom_node_unref(form);
	ܐܢ (ds_type != عدم)
		dom_string_unref(ds_type);
	ܐܢ (ds_value != عدم)
		dom_string_unref(ds_value);
	ܐܢ (ds_name != عدم)
		dom_string_unref(ds_name);

	ܦܢܐ control;
}

ساكن بنية form_control *
parse_input_element(بنية form *forms, dom_html_input_element *input)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_name = عدم;
	dom_string *ds_value = عدم;

	حرف *name = عدم;

	ܐܢ (dom_html_input_element_get_form(input, &form) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_input_element_get_type(input, &ds_type) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_input_element_get_name(input, &ds_name) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(input, GADGET_PASSWORD);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(input, GADGET_FILE);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(input, GADGET_HIDDEN);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(input, GADGET_CHECKBOX);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(input, GADGET_RADIO);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(input, GADGET_SUBMIT);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(input, GADGET_RESET);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(input, GADGET_BUTTON);
	} ܐܚܪܢܐ ܐܢ (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(input, GADGET_IMAGE);
	} ܐܚܪܢܐ {
		control = form_new_control(input, GADGET_TEXTBOX);
	}

	ܐܢ (control == عدم)
		اذهب out;

	ܐܢ (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	ܐܢ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ܐܢ (dom_html_input_element_get_checked(
			    input, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ܐܢ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ܐܢ (dom_html_input_element_get_max_length(
			    input, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ܐܢ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ܐܚܪܢܐ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ܐܢ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ܐܢ (dom_html_input_element_get_value(
			    input, &ds_value) == DOM_NO_ERR) {
			ܐܢ (ds_value != عدم) {
				control->value = strndup(
					dom_string_data(ds_value),
					dom_string_byte_length(ds_value));
				ܐܢ (control->value == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب out;
				}
				control->length = strlen(control->value);
			}
		}

		ܐܢ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ܐܢ (control->value == عدم) {
				control->value = strdup("");
				ܐܢ (control->value == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب out;
				}

				control->length = 0;
			}

			control->initial_value = strdup(control->value);
			ܐܢ (control->initial_value == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب out;
			}

			control->last_synced_value = strdup(control->value);
			ܐܢ (control->last_synced_value == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب out;
			}

			control->node_value = dom_string_ref(ds_value);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ܐܢ (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	ܐܢ (form != عدم)
		dom_node_unref(form);
	ܐܢ (ds_type != عدم)
		dom_string_unref(ds_type);
	ܐܢ (ds_name != عدم)
		dom_string_unref(ds_name);
	ܐܢ (ds_value != عدم)
		dom_string_unref(ds_value);

	ܐܢ (name != عدم)
		حرر(name);

	ܦܢܐ control;
}

ساكن بنية form_control *
parse_textarea_element(بنية form *forms, dom_html_text_area_element *ta)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	ܐܢ (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ܐܢ (control == عدم)
		اذهب out;

	ܐܢ (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	ܐܢ (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	ܐܢ (form != عدم)
		dom_node_unref(form);
	ܐܢ (ds_name != عدم)
		dom_string_unref(ds_name);

	ܐܢ (name != عدم)
		حرر(name);


	ܦܢܐ control;
}

ساكن بنية form_control *
parse_select_element(بنية form *forms, dom_html_select_element *select)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	ܐܢ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		اذهب out;

	ܐܢ (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ܐܢ (control == عدم)
		اذهب out;

	ܐܢ (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	ܐܢ (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	ܐܢ (form != عدم)
		dom_node_unref(form);
	ܐܢ (ds_name != عدم)
		dom_string_unref(ds_name);

	ܐܢ (name != عدم)
		حرر(name);


	ܦܢܐ control;
}


ساكن بنية form_control *
invent_fake_gadget(dom_node *node)
{
	بنية form_control *ctl = form_new_control(node, GADGET_HIDDEN);
	ܐܢ (ctl != عدم) {
		ctl->value = strdup("");
		ctl->initial_value = strdup("");
		ctl->name = strdup("foo");

		ܐܢ (ctl->value == عدم || ctl->initial_value == عدم ||
				ctl->name == عدم) {
			form_free_control(ctl);
			ctl = عدم;
		}
	}
	ܦܢܐ ctl;
}

/* documented in html_internal.h */
بنية form_control *
html_forms_get_control_for_node(بنية form *forms, dom_node *node)
{
	بنية form *f;
	بنية form_control *ctl = عدم;
	dom_exception err;
	dom_string *ds_name = عدم;

	/* Step one, see if we already have a control */
	لكل (f = forms; f != عدم; f = f->prev) {
		لكل (ctl = f->controls; ctl != عدم; ctl = ctl->next) {
			ܐܢ (ctl->node == node)
				ܦܢܐ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(node, &ds_name);
	ܐܢ (err == DOM_NO_ERR && ds_name != عدم) {

		/* Step three, attempt to work out what gadget to make */
		ܐܢ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) node);
		} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_input)) {
			ctl = parse_input_element(forms,
					(dom_html_input_element *) node);
		} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) node);
		} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) node);
		}
	}

	/* If all else fails, fake gadget time */
	ܐܢ (ctl == عدم)
		ctl = invent_fake_gadget(node);

	ܐܢ (ds_name != عدم)
		dom_string_unref(ds_name);

	ܦܢܐ ctl;
}
