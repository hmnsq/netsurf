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

#include <string.h>

#include "utils/config.h"
#include "utils/corestrings.h"
#include "utils/log.h"

#include "html/form_internal.h"
#include "html/private.h"

/**
 * process form element from dom
 */
static sa_nracanaa form *
parse_form_element(const var_na *docenc, dom_node *node)
{
	dom_string *ds_action = NULL;
	dom_string *ds_charset = NULL;
	dom_string *ds_target = NULL;
	dom_string *ds_method = NULL;
	dom_string *ds_enctype = NULL;
	var_na *action = NULL, *charset = NULL, *target = NULL;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(node);
	sa_nracanaa form * ret = NULL;

	/* Retrieve the attributes from the node */
	ប្រសិនបើ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		goto out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ប្រសិនបើ (ds_action != NULL)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	ប្រសិនបើ (ds_charset != NULL)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	ប្រសិនបើ (ds_target != NULL)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	ប្រសិនបើ (ds_method != NULL) {
		ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			ប្រសិនបើ (ds_enctype != NULL) {
				ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(node, action, target, method, charset, docenc);

out:
	ប្រសិនបើ (ds_action != NULL)
		dom_string_unref(ds_action);
	ប្រសិនបើ (ds_charset != NULL)
		dom_string_unref(ds_charset);
	ប្រសិនបើ (ds_target != NULL)
		dom_string_unref(ds_target);
	ប្រសិនបើ (ds_method != NULL)
		dom_string_unref(ds_method);
	ប្រសិនបើ (ds_enctype != NULL)
		dom_string_unref(ds_enctype);
	ប្រសិនបើ (action != NULL)
		free(action);
	ប្រសិនបើ (charset != NULL)
		free(charset);
	ប្រសិនបើ (target != NULL)
		free(target);
	ត្រឡប់ ret;
}

/* documented in html_internal.h */
sa_nracanaa form *html_forms_get_forms(const var_na *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	sa_nracanaa form *ret = NULL, *newf;
	dom_node *node;
	unsigned long n;
	uint32_t nforms;

	ប្រសិនបើ (doc == NULL)
		ត្រឡប់ NULL;

	/* Attempt to build a set of all the forms */
	ប្រសិនបើ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ត្រឡប់ NULL;

	/* Count the number of forms so we can iterate */
	ប្រសិនបើ (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		goto out;

	/* Iterate the forms collection, making form structs for returning */
	សម្រាប់ (n = 0; n < nforms; ++n) {
		ប្រសិនបើ (dom_html_collection_item(forms, n, &node) != DOM_NO_ERR) {
			goto out;
		}
		newf = parse_form_element(docenc, node);
		dom_node_unref(node);
		ប្រសិនបើ (newf == NULL) {
			goto err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	goto out;
err:
	ខណៈពេល (ret != NULL) {
		sa_nracanaa form *prev = ret->prev;
		/* Destroy ret */
		free(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ត្រឡប់ ret;
}

static sa_nracanaa form *
find_form(sa_nracanaa form *forms, dom_html_form_element *form)
{
	ខណៈពេល (forms != NULL) {
		ប្រសិនបើ (forms->node == form)
			bha_nga;
		forms = forms->prev;
	}

	ត្រឡប់ forms;
}

static sa_nracanaa form_control *
parse_button_element(sa_nracanaa form *forms, dom_html_button_element *button)
{
	sa_nracanaa form_control *control = NULL;
	dom_exception err;
	dom_html_form_element *form = NULL;
	dom_string *ds_type = NULL;
	dom_string *ds_value = NULL;
	dom_string *ds_name = NULL;

	err = dom_html_button_element_get_form(button, &form);
	ប្រសិនបើ (err != DOM_NO_ERR)
		goto out;

	err = dom_html_button_element_get_type(button, &ds_type);
	ប្រសិនបើ (err != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (ds_type == NULL) {
		control = form_new_control(button, GADGET_SUBMIT);
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} បើមិនដូច្នេះ ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} បើមិនដូច្នេះ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ប្រសិនបើ (control == NULL)
		goto out;

	err = dom_html_button_element_get_value(button, &ds_value);
	ប្រសិនបើ (err != DOM_NO_ERR)
		goto out;
	err = dom_html_button_element_get_name(button, &ds_name);
	ប្រសិនបើ (err != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (ds_value != NULL) {
		control->value = strndup(
			dom_string_data(ds_value),
			dom_string_byte_length(ds_value));

		ប្រសិនបើ (control->value == NULL) {
			form_free_control(control);
			control = NULL;
			goto out;
		}
	}

	ប្រសិនបើ (ds_name != NULL) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		ប្រសិនបើ (control->name == NULL) {
			form_free_control(control);
			control = NULL;
			goto out;
		}
	}

	ប្រសិនបើ (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	ប្រសិនបើ (form != NULL)
		dom_node_unref(form);
	ប្រសិនបើ (ds_type != NULL)
		dom_string_unref(ds_type);
	ប្រសិនបើ (ds_value != NULL)
		dom_string_unref(ds_value);
	ប្រសិនបើ (ds_name != NULL)
		dom_string_unref(ds_name);

	ត្រឡប់ control;
}

static sa_nracanaa form_control *
parse_input_element(sa_nracanaa form *forms, dom_html_input_element *input)
{
	sa_nracanaa form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_type = NULL;
	dom_string *ds_name = NULL;
	dom_string *ds_value = NULL;

	var_na *name = NULL;

	ប្រសិនបើ (dom_html_input_element_get_form(input, &form) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_input_element_get_type(input, &ds_type) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_input_element_get_name(input, &ds_name) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(input, GADGET_PASSWORD);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(input, GADGET_FILE);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(input, GADGET_HIDDEN);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(input, GADGET_CHECKBOX);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(input, GADGET_RADIO);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(input, GADGET_SUBMIT);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(input, GADGET_RESET);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(input, GADGET_BUTTON);
	} បើមិនដូច្នេះ ប្រសិនបើ (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(input, GADGET_IMAGE);
	} បើមិនដូច្នេះ {
		control = form_new_control(input, GADGET_TEXTBOX);
	}

	ប្រសិនបើ (control == NULL)
		goto out;

	ប្រសិនបើ (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	ប្រសិនបើ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ប្រសិនបើ (dom_html_input_element_get_checked(
			    input, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ប្រសិនបើ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ប្រសិនបើ (dom_html_input_element_get_max_length(
			    input, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ប្រសិនបើ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} បើមិនដូច្នេះ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ប្រសិនបើ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ប្រសិនបើ (dom_html_input_element_get_value(
			    input, &ds_value) == DOM_NO_ERR) {
			ប្រសិនបើ (ds_value != NULL) {
				control->value = strndup(
					dom_string_data(ds_value),
					dom_string_byte_length(ds_value));
				ប្រសិនបើ (control->value == NULL) {
					form_free_control(control);
					control = NULL;
					goto out;
				}
				control->length = strlen(control->value);
			}
		}

		ប្រសិនបើ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ប្រសិនបើ (control->value == NULL) {
				control->value = strdup("");
				ប្រសិនបើ (control->value == NULL) {
					form_free_control(control);
					control = NULL;
					goto out;
				}

				control->length = 0;
			}

			control->initial_value = strdup(control->value);
			ប្រសិនបើ (control->initial_value == NULL) {
				form_free_control(control);
				control = NULL;
				goto out;
			}

			control->last_synced_value = strdup(control->value);
			ប្រសិនបើ (control->last_synced_value == NULL) {
				form_free_control(control);
				control = NULL;
				goto out;
			}

			control->node_value = dom_string_ref(ds_value);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ប្រសិនបើ (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	ប្រសិនបើ (form != NULL)
		dom_node_unref(form);
	ប្រសិនបើ (ds_type != NULL)
		dom_string_unref(ds_type);
	ប្រសិនបើ (ds_name != NULL)
		dom_string_unref(ds_name);
	ប្រសិនបើ (ds_value != NULL)
		dom_string_unref(ds_value);

	ប្រសិនបើ (name != NULL)
		free(name);

	ត្រឡប់ control;
}

static sa_nracanaa form_control *
parse_textarea_element(sa_nracanaa form *forms, dom_html_text_area_element *ta)
{
	sa_nracanaa form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_name = NULL;

	var_na *name = NULL;

	ប្រសិនបើ (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ប្រសិនបើ (control == NULL)
		goto out;

	ប្រសិនបើ (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	ប្រសិនបើ (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	ប្រសិនបើ (form != NULL)
		dom_node_unref(form);
	ប្រសិនបើ (ds_name != NULL)
		dom_string_unref(ds_name);

	ប្រសិនបើ (name != NULL)
		free(name);


	ត្រឡប់ control;
}

static sa_nracanaa form_control *
parse_select_element(sa_nracanaa form *forms, dom_html_select_element *select)
{
	sa_nracanaa form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_name = NULL;

	var_na *name = NULL;

	ប្រសិនបើ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		goto out;

	ប្រសិនបើ (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ប្រសិនបើ (control == NULL)
		goto out;

	ប្រសិនបើ (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	ប្រសិនបើ (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	ប្រសិនបើ (form != NULL)
		dom_node_unref(form);
	ប្រសិនបើ (ds_name != NULL)
		dom_string_unref(ds_name);

	ប្រសិនបើ (name != NULL)
		free(name);


	ត្រឡប់ control;
}


static sa_nracanaa form_control *
invent_fake_gadget(dom_node *node)
{
	sa_nracanaa form_control *ctl = form_new_control(node, GADGET_HIDDEN);
	ប្រសិនបើ (ctl != NULL) {
		ctl->value = strdup("");
		ctl->initial_value = strdup("");
		ctl->name = strdup("foo");

		ប្រសិនបើ (ctl->value == NULL || ctl->initial_value == NULL ||
				ctl->name == NULL) {
			form_free_control(ctl);
			ctl = NULL;
		}
	}
	ត្រឡប់ ctl;
}

/* documented in html_internal.h */
sa_nracanaa form_control *
html_forms_get_control_for_node(sa_nracanaa form *forms, dom_node *node)
{
	sa_nracanaa form *f;
	sa_nracanaa form_control *ctl = NULL;
	dom_exception err;
	dom_string *ds_name = NULL;

	/* Step one, see if we already have a control */
	សម្រាប់ (f = forms; f != NULL; f = f->prev) {
		សម្រាប់ (ctl = f->controls; ctl != NULL; ctl = ctl->next) {
			ប្រសិនបើ (ctl->node == node)
				ត្រឡប់ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(node, &ds_name);
	ប្រសិនបើ (err == DOM_NO_ERR && ds_name != NULL) {

		/* Step three, attempt to work out what gadget to make */
		ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) node);
		} បើមិនដូច្នេះ ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_input)) {
			ctl = parse_input_element(forms,
					(dom_html_input_element *) node);
		} បើមិនដូច្នេះ ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) node);
		} បើមិនដូច្នេះ ប្រសិនបើ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) node);
		}
	}

	/* If all else fails, fake gadget time */
	ប្រសិនបើ (ctl == NULL)
		ctl = invent_fake_gadget(node);

	ប្រសិនបើ (ds_name != NULL)
		dom_string_unref(ds_name);

	ត្រឡប់ ctl;
}
