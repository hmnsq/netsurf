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

#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"

#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/private.h"

/**
 * process form element from dom
 */
நிலையான கட்டமைப்பு form *
parse_form_element(மாறிலி எழுத்து *docenc, dom_முனை *முனை)
{
	dom_string *ds_action = வெற்று;
	dom_string *ds_charset = வெற்று;
	dom_string *ds_target = வெற்று;
	dom_string *ds_method = வெற்று;
	dom_string *ds_enctype = வெற்று;
	எழுத்து *action = வெற்று, *charset = வெற்று, *target = வெற்று;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(முனை);
	கட்டமைப்பு form * ret = வெற்று;

	/* Retrieve the attributes from the node */
	எனில் (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		செல் out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	எனில் (ds_action != வெற்று)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	எனில் (ds_charset != வெற்று)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	எனில் (ds_target != வெற்று)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	எனில் (ds_method != வெற்று) {
		எனில் (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			எனில் (ds_enctype != வெற்று) {
				எனில் (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(முனை, action, target, method, charset, docenc);

out:
	எனில் (ds_action != வெற்று)
		dom_string_unref(ds_action);
	எனில் (ds_charset != வெற்று)
		dom_string_unref(ds_charset);
	எனில் (ds_target != வெற்று)
		dom_string_unref(ds_target);
	எனில் (ds_method != வெற்று)
		dom_string_unref(ds_method);
	எனில் (ds_enctype != வெற்று)
		dom_string_unref(ds_enctype);
	எனில் (action != வெற்று)
		விடுவி(action);
	எனில் (charset != வெற்று)
		விடுவி(charset);
	எனில் (target != வெற்று)
		விடுவி(target);
	திரும்பு ret;
}

/* documented in html_internal.h */
கட்டமைப்பு form *html_forms_get_forms(மாறிலி எழுத்து *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	கட்டமைப்பு form *ret = வெற்று, *newf;
	dom_முனை *முனை;
	unsigned long n;
	uint32_t nforms;

	எனில் (doc == வெற்று)
		திரும்பு வெற்று;

	/* Attempt to build a set of all the forms */
	எனில் (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		திரும்பு வெற்று;

	/* Count the number of forms so we can iterate */
	எனில் (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		செல் out;

	/* Iterate the forms collection, making form structs for returning */
	க்கு (n = 0; n < nforms; ++n) {
		எனில் (dom_html_collection_item(forms, n, &முனை) != DOM_NO_ERR) {
			செல் out;
		}
		newf = parse_form_element(docenc, முனை);
		dom_முனை_unref(முனை);
		எனில் (newf == வெற்று) {
			செல் err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	செல் out;
err:
	போது (ret != வெற்று) {
		கட்டமைப்பு form *prev = ret->prev;
		/* Destroy ret */
		விடுவி(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	திரும்பு ret;
}

நிலையான கட்டமைப்பு form *
find_form(கட்டமைப்பு form *forms, dom_html_form_element *form)
{
	போது (forms != வெற்று) {
		எனில் (forms->முனை == form)
			நிறுத்து;
		forms = forms->prev;
	}

	திரும்பு forms;
}

நிலையான கட்டமைப்பு form_control *
parse_button_element(கட்டமைப்பு form *forms, dom_html_button_element *button)
{
	கட்டமைப்பு form_control *control = வெற்று;
	dom_exception err;
	dom_html_form_element *form = வெற்று;
	dom_string *ds_type = வெற்று;
	dom_string *ds_மதிப்பு = வெற்று;
	dom_string *ds_name = வெற்று;

	err = dom_html_button_element_get_form(button, &form);
	எனில் (err != DOM_NO_ERR)
		செல் out;

	err = dom_html_button_element_get_type(button, &ds_type);
	எனில் (err != DOM_NO_ERR)
		செல் out;

	எனில் (ds_type == வெற்று) {
		control = form_new_control(button, GADGET_SUBMIT);
	} இல்லையெனில் {
		எனில் (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} இல்லையெனில் {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	எனில் (control == வெற்று)
		செல் out;

	err = dom_html_button_element_get_மதிப்பு(button, &ds_மதிப்பு);
	எனில் (err != DOM_NO_ERR)
		செல் out;
	err = dom_html_button_element_get_name(button, &ds_name);
	எனில் (err != DOM_NO_ERR)
		செல் out;

	எனில் (ds_மதிப்பு != வெற்று) {
		control->மதிப்பு = strndup(
			dom_string_data(ds_மதிப்பு),
			dom_string_byte_length(ds_மதிப்பு));

		எனில் (control->மதிப்பு == வெற்று) {
			form_free_control(control);
			control = வெற்று;
			செல் out;
		}
	}

	எனில் (ds_name != வெற்று) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		எனில் (control->name == வெற்று) {
			form_free_control(control);
			control = வெற்று;
			செல் out;
		}
	}

	எனில் (form != வெற்று && control != வெற்று)
		form_add_control(find_form(forms, form), control);

out:
	எனில் (form != வெற்று)
		dom_முனை_unref(form);
	எனில் (ds_type != வெற்று)
		dom_string_unref(ds_type);
	எனில் (ds_மதிப்பு != வெற்று)
		dom_string_unref(ds_மதிப்பு);
	எனில் (ds_name != வெற்று)
		dom_string_unref(ds_name);

	திரும்பு control;
}

நிலையான கட்டமைப்பு form_control *
parse_உள்ளீடு_element(கட்டமைப்பு form *forms, dom_html_உள்ளீடு_element *உள்ளீடு)
{
	கட்டமைப்பு form_control *control = வெற்று;
	dom_html_form_element *form = வெற்று;
	dom_string *ds_type = வெற்று;
	dom_string *ds_name = வெற்று;
	dom_string *ds_மதிப்பு = வெற்று;

	எழுத்து *name = வெற்று;

	எனில் (dom_html_உள்ளீடு_element_get_form(உள்ளீடு, &form) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_உள்ளீடு_element_get_type(உள்ளீடு, &ds_type) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_உள்ளீடு_element_get_name(உள்ளீடு, &ds_name) != DOM_NO_ERR)
		செல் out;

	எனில் (ds_name != வெற்று)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(உள்ளீடு, GADGET_PASSWORD);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(உள்ளீடு, GADGET_FILE);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(உள்ளீடு, GADGET_HIDDEN);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(உள்ளீடு, GADGET_CHECKBOX);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(உள்ளீடு, GADGET_RADIO);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(உள்ளீடு, GADGET_SUBMIT);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(உள்ளீடு, GADGET_RESET);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(உள்ளீடு, GADGET_BUTTON);
	} இல்லையெனில் எனில் (ds_type != வெற்று && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(உள்ளீடு, GADGET_IMAGE);
	} இல்லையெனில் {
		control = form_new_control(உள்ளீடு, GADGET_TEXTBOX);
	}

	எனில் (control == வெற்று)
		செல் out;

	எனில் (name != வெற்று) {
		/* Hand the name string over */
		control->name = name;
		name = வெற்று;
	}

	எனில் (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		எனில் (dom_html_உள்ளீடு_element_get_checked(
			    உள்ளீடு, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	எனில் (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		எனில் (dom_html_உள்ளீடு_element_get_max_length(
			    உள்ளீடு, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		எனில் (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} இல்லையெனில் {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	எனில் (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		எனில் (dom_html_உள்ளீடு_element_get_மதிப்பு(
			    உள்ளீடு, &ds_மதிப்பு) == DOM_NO_ERR) {
			எனில் (ds_மதிப்பு != வெற்று) {
				control->மதிப்பு = strndup(
					dom_string_data(ds_மதிப்பு),
					dom_string_byte_length(ds_மதிப்பு));
				எனில் (control->மதிப்பு == வெற்று) {
					form_free_control(control);
					control = வெற்று;
					செல் out;
				}
				control->length = strlen(control->மதிப்பு);
			}
		}

		எனில் (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			எனில் (control->மதிப்பு == வெற்று) {
				control->மதிப்பு = strdup("");
				எனில் (control->மதிப்பு == வெற்று) {
					form_free_control(control);
					control = வெற்று;
					செல் out;
				}

				control->length = 0;
			}

			control->initial_மதிப்பு = strdup(control->மதிப்பு);
			எனில் (control->initial_மதிப்பு == வெற்று) {
				form_free_control(control);
				control = வெற்று;
				செல் out;
			}

			control->last_synced_மதிப்பு = strdup(control->மதிப்பு);
			எனில் (control->last_synced_மதிப்பு == வெற்று) {
				form_free_control(control);
				control = வெற்று;
				செல் out;
			}

			control->முனை_மதிப்பு = dom_string_ref(ds_மதிப்பு);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	எனில் (form != வெற்று && control != வெற்று)
		form_add_control(find_form(forms, form), control);

out:
	எனில் (form != வெற்று)
		dom_முனை_unref(form);
	எனில் (ds_type != வெற்று)
		dom_string_unref(ds_type);
	எனில் (ds_name != வெற்று)
		dom_string_unref(ds_name);
	எனில் (ds_மதிப்பு != வெற்று)
		dom_string_unref(ds_மதிப்பு);

	எனில் (name != வெற்று)
		விடுவி(name);

	திரும்பு control;
}

நிலையான கட்டமைப்பு form_control *
parse_textarea_element(கட்டமைப்பு form *forms, dom_html_text_area_element *ta)
{
	கட்டமைப்பு form_control *control = வெற்று;
	dom_html_form_element *form = வெற்று;
	dom_string *ds_name = வெற்று;

	எழுத்து *name = வெற்று;

	எனில் (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		செல் out;

	எனில் (ds_name != வெற்று)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	எனில் (control == வெற்று)
		செல் out;

	எனில் (name != வெற்று) {
		/* Hand the name string over */
		control->name = name;
		name = வெற்று;
	}

	எனில் (form != வெற்று && control != வெற்று)
		form_add_control(find_form(forms, form), control);

out:
	எனில் (form != வெற்று)
		dom_முனை_unref(form);
	எனில் (ds_name != வெற்று)
		dom_string_unref(ds_name);

	எனில் (name != வெற்று)
		விடுவி(name);


	திரும்பு control;
}

நிலையான கட்டமைப்பு form_control *
parse_select_element(கட்டமைப்பு form *forms, dom_html_select_element *select)
{
	கட்டமைப்பு form_control *control = வெற்று;
	dom_html_form_element *form = வெற்று;
	dom_string *ds_name = வெற்று;

	எழுத்து *name = வெற்று;

	எனில் (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		செல் out;

	எனில் (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		செல் out;

	எனில் (ds_name != வெற்று)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	எனில் (control == வெற்று)
		செல் out;

	எனில் (name != வெற்று) {
		/* Hand the name string over */
		control->name = name;
		name = வெற்று;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	எனில் (form != வெற்று && control != வெற்று)
		form_add_control(find_form(forms, form), control);

out:
	எனில் (form != வெற்று)
		dom_முனை_unref(form);
	எனில் (ds_name != வெற்று)
		dom_string_unref(ds_name);

	எனில் (name != வெற்று)
		விடுவி(name);


	திரும்பு control;
}


நிலையான கட்டமைப்பு form_control *
invent_fake_gadget(dom_முனை *முனை)
{
	கட்டமைப்பு form_control *ctl = form_new_control(முனை, GADGET_HIDDEN);
	எனில் (ctl != வெற்று) {
		ctl->மதிப்பு = strdup("");
		ctl->initial_மதிப்பு = strdup("");
		ctl->name = strdup("foo");

		எனில் (ctl->மதிப்பு == வெற்று || ctl->initial_மதிப்பு == வெற்று ||
				ctl->name == வெற்று) {
			form_free_control(ctl);
			ctl = வெற்று;
		}
	}
	திரும்பு ctl;
}

/* documented in html_internal.h */
கட்டமைப்பு form_control *
html_forms_get_control_for_முனை(கட்டமைப்பு form *forms, dom_முனை *முனை)
{
	கட்டமைப்பு form *f;
	கட்டமைப்பு form_control *ctl = வெற்று;
	dom_exception err;
	dom_string *ds_name = வெற்று;

	/* Step one, see if we already have a control */
	க்கு (f = forms; f != வெற்று; f = f->prev) {
		க்கு (ctl = f->controls; ctl != வெற்று; ctl = ctl->next) {
			எனில் (ctl->முனை == முனை)
				திரும்பு ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(முனை, &ds_name);
	எனில் (err == DOM_NO_ERR && ds_name != வெற்று) {

		/* Step three, attempt to work out what gadget to make */
		எனில் (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) முனை);
		} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_உள்ளீடு)) {
			ctl = parse_உள்ளீடு_element(forms,
					(dom_html_உள்ளீடு_element *) முனை);
		} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) முனை);
		} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) முனை);
		}
	}

	/* If all else fails, fake gadget time */
	எனில் (ctl == வெற்று)
		ctl = invent_fake_gadget(முனை);

	எனில் (ds_name != வெற்று)
		dom_string_unref(ds_name);

	திரும்பு ctl;
}
