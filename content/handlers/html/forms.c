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
static struct form *
parse_form_element(const char *docenc, dom_node *node)
{
	dom_string *ds_action = NULL;
	dom_string *ds_charset = NULL;
	dom_string *ds_target = NULL;
	dom_string *ds_method = NULL;
	dom_string *ds_enctype = NULL;
	char *action = NULL, *charset = NULL, *target = NULL;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(node);
	struct form * ret = NULL;

	/* Retrieve the attributes from the node */
	もし (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		goto out;

	もし (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		goto out;

	もし (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		goto out;

	もし (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		goto out;

	もし (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		goto out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	もし (ds_action != NULL)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	もし (ds_charset != NULL)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	もし (ds_target != NULL)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	もし (ds_method != NULL) {
		もし (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			もし (ds_enctype != NULL) {
				もし (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(node, action, target, method, charset, docenc);

out:
	もし (ds_action != NULL)
		dom_string_unref(ds_action);
	もし (ds_charset != NULL)
		dom_string_unref(ds_charset);
	もし (ds_target != NULL)
		dom_string_unref(ds_target);
	もし (ds_method != NULL)
		dom_string_unref(ds_method);
	もし (ds_enctype != NULL)
		dom_string_unref(ds_enctype);
	もし (action != NULL)
		free(action);
	もし (charset != NULL)
		free(charset);
	もし (target != NULL)
		free(target);
	戻る ret;
}

/* documented in html_internal.h */
struct form *html_forms_get_forms(const char *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	struct form *ret = NULL, *newf;
	dom_node *node;
	unsigned long n;
	uint32_t nforms;

	もし (doc == NULL)
		戻る NULL;

	/* Attempt to build a set of all the forms */
	もし (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		戻る NULL;

	/* Count the number of forms so we can iterate */
	もし (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		goto out;

	/* Iterate the forms collection, making form structs for returning */
	ために (n = 0; n < nforms; ++n) {
		もし (dom_html_collection_item(forms, n, &node) != DOM_NO_ERR) {
			goto out;
		}
		newf = parse_form_element(docenc, node);
		dom_node_unref(node);
		もし (newf == NULL) {
			goto err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	goto out;
err:
	ながら (ret != NULL) {
		struct form *prev = ret->prev;
		/* Destroy ret */
		free(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	戻る ret;
}

static struct form *
find_form(struct form *forms, dom_html_form_element *form)
{
	ながら (forms != NULL) {
		もし (forms->node == form)
			break;
		forms = forms->prev;
	}

	戻る forms;
}

static struct form_control *
parse_button_element(struct form *forms, dom_html_button_element *button)
{
	struct form_control *control = NULL;
	dom_exception err;
	dom_html_form_element *form = NULL;
	dom_string *ds_type = NULL;
	dom_string *ds_value = NULL;
	dom_string *ds_name = NULL;

	err = dom_html_button_element_get_form(button, &form);
	もし (err != DOM_NO_ERR)
		goto out;

	err = dom_html_button_element_get_type(button, &ds_type);
	もし (err != DOM_NO_ERR)
		goto out;

	もし (ds_type == NULL) {
		control = form_new_control(button, GADGET_SUBMIT);
	} 否则 {
		もし (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} 否则 もし (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} 否则 {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	もし (control == NULL)
		goto out;

	err = dom_html_button_element_get_value(button, &ds_value);
	もし (err != DOM_NO_ERR)
		goto out;
	err = dom_html_button_element_get_name(button, &ds_name);
	もし (err != DOM_NO_ERR)
		goto out;

	もし (ds_value != NULL) {
		control->value = strndup(
			dom_string_data(ds_value),
			dom_string_byte_length(ds_value));

		もし (control->value == NULL) {
			form_free_control(control);
			control = NULL;
			goto out;
		}
	}

	もし (ds_name != NULL) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		もし (control->name == NULL) {
			form_free_control(control);
			control = NULL;
			goto out;
		}
	}

	もし (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	もし (form != NULL)
		dom_node_unref(form);
	もし (ds_type != NULL)
		dom_string_unref(ds_type);
	もし (ds_value != NULL)
		dom_string_unref(ds_value);
	もし (ds_name != NULL)
		dom_string_unref(ds_name);

	戻る control;
}

static struct form_control *
parse_input_element(struct form *forms, dom_html_input_element *input)
{
	struct form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_type = NULL;
	dom_string *ds_name = NULL;
	dom_string *ds_value = NULL;

	char *name = NULL;

	もし (dom_html_input_element_get_form(input, &form) != DOM_NO_ERR)
		goto out;

	もし (dom_html_input_element_get_type(input, &ds_type) != DOM_NO_ERR)
		goto out;

	もし (dom_html_input_element_get_name(input, &ds_name) != DOM_NO_ERR)
		goto out;

	もし (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(input, GADGET_PASSWORD);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(input, GADGET_FILE);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(input, GADGET_HIDDEN);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(input, GADGET_CHECKBOX);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(input, GADGET_RADIO);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(input, GADGET_SUBMIT);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(input, GADGET_RESET);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(input, GADGET_BUTTON);
	} 否则 もし (ds_type != NULL && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(input, GADGET_IMAGE);
	} 否则 {
		control = form_new_control(input, GADGET_TEXTBOX);
	}

	もし (control == NULL)
		goto out;

	もし (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	もし (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		もし (dom_html_input_element_get_checked(
			    input, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	もし (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		もし (dom_html_input_element_get_max_length(
			    input, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		もし (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} 否则 {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	もし (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		もし (dom_html_input_element_get_value(
			    input, &ds_value) == DOM_NO_ERR) {
			もし (ds_value != NULL) {
				control->value = strndup(
					dom_string_data(ds_value),
					dom_string_byte_length(ds_value));
				もし (control->value == NULL) {
					form_free_control(control);
					control = NULL;
					goto out;
				}
				control->length = strlen(control->value);
			}
		}

		もし (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			もし (control->value == NULL) {
				control->value = strdup("");
				もし (control->value == NULL) {
					form_free_control(control);
					control = NULL;
					goto out;
				}

				control->length = 0;
			}

			control->initial_value = strdup(control->value);
			もし (control->initial_value == NULL) {
				form_free_control(control);
				control = NULL;
				goto out;
			}

			control->last_synced_value = strdup(control->value);
			もし (control->last_synced_value == NULL) {
				form_free_control(control);
				control = NULL;
				goto out;
			}

			control->node_value = dom_string_ref(ds_value);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	もし (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	もし (form != NULL)
		dom_node_unref(form);
	もし (ds_type != NULL)
		dom_string_unref(ds_type);
	もし (ds_name != NULL)
		dom_string_unref(ds_name);
	もし (ds_value != NULL)
		dom_string_unref(ds_value);

	もし (name != NULL)
		free(name);

	戻る control;
}

static struct form_control *
parse_textarea_element(struct form *forms, dom_html_text_area_element *ta)
{
	struct form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_name = NULL;

	char *name = NULL;

	もし (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		goto out;

	もし (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		goto out;

	もし (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	もし (control == NULL)
		goto out;

	もし (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	もし (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	もし (form != NULL)
		dom_node_unref(form);
	もし (ds_name != NULL)
		dom_string_unref(ds_name);

	もし (name != NULL)
		free(name);


	戻る control;
}

static struct form_control *
parse_select_element(struct form *forms, dom_html_select_element *select)
{
	struct form_control *control = NULL;
	dom_html_form_element *form = NULL;
	dom_string *ds_name = NULL;

	char *name = NULL;

	もし (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		goto out;

	もし (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		goto out;

	もし (ds_name != NULL)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	もし (control == NULL)
		goto out;

	もし (name != NULL) {
		/* Hand the name string over */
		control->name = name;
		name = NULL;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	もし (form != NULL && control != NULL)
		form_add_control(find_form(forms, form), control);

out:
	もし (form != NULL)
		dom_node_unref(form);
	もし (ds_name != NULL)
		dom_string_unref(ds_name);

	もし (name != NULL)
		free(name);


	戻る control;
}


static struct form_control *
invent_fake_gadget(dom_node *node)
{
	struct form_control *ctl = form_new_control(node, GADGET_HIDDEN);
	もし (ctl != NULL) {
		ctl->value = strdup("");
		ctl->initial_value = strdup("");
		ctl->name = strdup("foo");

		もし (ctl->value == NULL || ctl->initial_value == NULL ||
				ctl->name == NULL) {
			form_free_control(ctl);
			ctl = NULL;
		}
	}
	戻る ctl;
}

/* documented in html_internal.h */
struct form_control *
html_forms_get_control_for_node(struct form *forms, dom_node *node)
{
	struct form *f;
	struct form_control *ctl = NULL;
	dom_exception err;
	dom_string *ds_name = NULL;

	/* Step one, see if we already have a control */
	ために (f = forms; f != NULL; f = f->prev) {
		ために (ctl = f->controls; ctl != NULL; ctl = ctl->next) {
			もし (ctl->node == node)
				戻る ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(node, &ds_name);
	もし (err == DOM_NO_ERR && ds_name != NULL) {

		/* Step three, attempt to work out what gadget to make */
		もし (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) node);
		} 否则 もし (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_input)) {
			ctl = parse_input_element(forms,
					(dom_html_input_element *) node);
		} 否则 もし (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) node);
		} 否则 もし (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) node);
		}
	}

	/* If all else fails, fake gadget time */
	もし (ctl == NULL)
		ctl = invent_fake_gadget(node);

	もし (ds_name != NULL)
		dom_string_unref(ds_name);

	戻る ctl;
}
