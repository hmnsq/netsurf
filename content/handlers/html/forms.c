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
	אם (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		اذهب out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	אם (ds_action != عدم)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	אם (ds_charset != عدم)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	אם (ds_target != عدم)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	אם (ds_method != عدم) {
		אם (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			אם (ds_enctype != عدم) {
				אם (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(node, action, target, method, charset, docenc);

out:
	אם (ds_action != عدم)
		dom_string_unref(ds_action);
	אם (ds_charset != عدم)
		dom_string_unref(ds_charset);
	אם (ds_target != عدم)
		dom_string_unref(ds_target);
	אם (ds_method != عدم)
		dom_string_unref(ds_method);
	אם (ds_enctype != عدم)
		dom_string_unref(ds_enctype);
	אם (action != عدم)
		حرر(action);
	אם (charset != عدم)
		حرر(charset);
	אם (target != عدم)
		حرر(target);
	החזר ret;
}

/* documented in html_internal.h */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	بنية form *ret = عدم, *newf;
	dom_node *node;
	unsigned long n;
	uint32_t nforms;

	אם (doc == عدم)
		החזר عدم;

	/* Attempt to build a set of all the forms */
	אם (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		החזר عدم;

	/* Count the number of forms so we can iterate */
	אם (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		اذهب out;

	/* Iterate the forms collection, making form structs for returning */
	עבור (n = 0; n < nforms; ++n) {
		אם (dom_html_collection_item(forms, n, &node) != DOM_NO_ERR) {
			اذهب out;
		}
		newf = parse_form_element(docenc, node);
		dom_node_unref(node);
		אם (newf == عدم) {
			اذهب err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	اذهب out;
err:
	כלעוד (ret != عدم) {
		بنية form *prev = ret->prev;
		/* Destroy ret */
		حرر(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	החזר ret;
}

ساكن بنية form *
find_form(بنية form *forms, dom_html_form_element *form)
{
	כלעוד (forms != عدم) {
		אם (forms->node == form)
			اكسر;
		forms = forms->prev;
	}

	החזר forms;
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
	אם (err != DOM_NO_ERR)
		اذهب out;

	err = dom_html_button_element_get_type(button, &ds_type);
	אם (err != DOM_NO_ERR)
		اذهب out;

	אם (ds_type == عدم) {
		control = form_new_control(button, GADGET_SUBMIT);
	} אחרת {
		אם (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} אחרת אם (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} אחרת {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	אם (control == عدم)
		اذهب out;

	err = dom_html_button_element_get_value(button, &ds_value);
	אם (err != DOM_NO_ERR)
		اذهب out;
	err = dom_html_button_element_get_name(button, &ds_name);
	אם (err != DOM_NO_ERR)
		اذهب out;

	אם (ds_value != عدم) {
		control->value = strndup(
			dom_string_data(ds_value),
			dom_string_byte_length(ds_value));

		אם (control->value == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب out;
		}
	}

	אם (ds_name != عدم) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		אם (control->name == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب out;
		}
	}

	אם (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	אם (form != عدم)
		dom_node_unref(form);
	אם (ds_type != عدم)
		dom_string_unref(ds_type);
	אם (ds_value != عدم)
		dom_string_unref(ds_value);
	אם (ds_name != عدم)
		dom_string_unref(ds_name);

	החזר control;
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

	אם (dom_html_input_element_get_form(input, &form) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_input_element_get_type(input, &ds_type) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_input_element_get_name(input, &ds_name) != DOM_NO_ERR)
		اذهب out;

	אם (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(input, GADGET_PASSWORD);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(input, GADGET_FILE);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(input, GADGET_HIDDEN);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(input, GADGET_CHECKBOX);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(input, GADGET_RADIO);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(input, GADGET_SUBMIT);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(input, GADGET_RESET);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(input, GADGET_BUTTON);
	} אחרת אם (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(input, GADGET_IMAGE);
	} אחרת {
		control = form_new_control(input, GADGET_TEXTBOX);
	}

	אם (control == عدم)
		اذهب out;

	אם (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	אם (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		אם (dom_html_input_element_get_checked(
			    input, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	אם (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		אם (dom_html_input_element_get_max_length(
			    input, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		אם (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} אחרת {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	אם (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		אם (dom_html_input_element_get_value(
			    input, &ds_value) == DOM_NO_ERR) {
			אם (ds_value != عدم) {
				control->value = strndup(
					dom_string_data(ds_value),
					dom_string_byte_length(ds_value));
				אם (control->value == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب out;
				}
				control->length = strlen(control->value);
			}
		}

		אם (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			אם (control->value == عدم) {
				control->value = strdup("");
				אם (control->value == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب out;
				}

				control->length = 0;
			}

			control->initial_value = strdup(control->value);
			אם (control->initial_value == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب out;
			}

			control->last_synced_value = strdup(control->value);
			אם (control->last_synced_value == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب out;
			}

			control->node_value = dom_string_ref(ds_value);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	אם (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	אם (form != عدم)
		dom_node_unref(form);
	אם (ds_type != عدم)
		dom_string_unref(ds_type);
	אם (ds_name != عدم)
		dom_string_unref(ds_name);
	אם (ds_value != عدم)
		dom_string_unref(ds_value);

	אם (name != عدم)
		حرر(name);

	החזר control;
}

ساكن بنية form_control *
parse_textarea_element(بنية form *forms, dom_html_text_area_element *ta)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	אם (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		اذهب out;

	אם (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	אם (control == عدم)
		اذهب out;

	אם (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	אם (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	אם (form != عدم)
		dom_node_unref(form);
	אם (ds_name != عدم)
		dom_string_unref(ds_name);

	אם (name != عدم)
		حرر(name);


	החזר control;
}

ساكن بنية form_control *
parse_select_element(بنية form *forms, dom_html_select_element *select)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	אם (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		اذهب out;

	אם (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		اذهب out;

	אם (ds_name != عدم)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	אם (control == عدم)
		اذهب out;

	אם (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	אם (form != عدم && control != عدم)
		form_add_control(find_form(forms, form), control);

out:
	אם (form != عدم)
		dom_node_unref(form);
	אם (ds_name != عدم)
		dom_string_unref(ds_name);

	אם (name != عدم)
		حرر(name);


	החזר control;
}


ساكن بنية form_control *
invent_fake_gadget(dom_node *node)
{
	بنية form_control *ctl = form_new_control(node, GADGET_HIDDEN);
	אם (ctl != عدم) {
		ctl->value = strdup("");
		ctl->initial_value = strdup("");
		ctl->name = strdup("foo");

		אם (ctl->value == عدم || ctl->initial_value == عدم ||
				ctl->name == عدم) {
			form_free_control(ctl);
			ctl = عدم;
		}
	}
	החזר ctl;
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
	עבור (f = forms; f != عدم; f = f->prev) {
		עבור (ctl = f->controls; ctl != عدم; ctl = ctl->next) {
			אם (ctl->node == node)
				החזר ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(node, &ds_name);
	אם (err == DOM_NO_ERR && ds_name != عدم) {

		/* Step three, attempt to work out what gadget to make */
		אם (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) node);
		} אחרת אם (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_input)) {
			ctl = parse_input_element(forms,
					(dom_html_input_element *) node);
		} אחרת אם (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) node);
		} אחרת אם (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) node);
		}
	}

	/* If all else fails, fake gadget time */
	אם (ctl == عدم)
		ctl = invent_fake_gadget(node);

	אם (ds_name != عدم)
		dom_string_unref(ds_name);

	החזר ctl;
}
