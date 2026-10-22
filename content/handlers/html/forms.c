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

#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"

#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/private.h"

/**
 * process form element from dom
 */
സ്ഥിതം ഘടന form *
parse_form_element(സ്ഥിരം അക്ഷരം *docenc, dom_കണ്ണി *കണ്ണി)
{
	dom_string *ds_action = ശൂന്യ;
	dom_string *ds_charset = ശൂന്യ;
	dom_string *ds_target = ശൂന്യ;
	dom_string *ds_method = ശൂന്യ;
	dom_string *ds_enctype = ശൂന്യ;
	അക്ഷരം *action = ശൂന്യ, *charset = ശൂന്യ, *target = ശൂന്യ;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(കണ്ണി);
	ഘടന form * ret = ശൂന്യ;

	/* Retrieve the attributes from the node */
	എങ്കിൽ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		പോകുക out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	എങ്കിൽ (ds_action != ശൂന്യ)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	എങ്കിൽ (ds_charset != ശൂന്യ)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	എങ്കിൽ (ds_target != ശൂന്യ)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	എങ്കിൽ (ds_method != ശൂന്യ) {
		എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			എങ്കിൽ (ds_enctype != ശൂന്യ) {
				എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(കണ്ണി, action, target, method, charset, docenc);

out:
	എങ്കിൽ (ds_action != ശൂന്യ)
		dom_string_unref(ds_action);
	എങ്കിൽ (ds_charset != ശൂന്യ)
		dom_string_unref(ds_charset);
	എങ്കിൽ (ds_target != ശൂന്യ)
		dom_string_unref(ds_target);
	എങ്കിൽ (ds_method != ശൂന്യ)
		dom_string_unref(ds_method);
	എങ്കിൽ (ds_enctype != ശൂന്യ)
		dom_string_unref(ds_enctype);
	എങ്കിൽ (action != ശൂന്യ)
		സ്വതന്ത്രം(action);
	എങ്കിൽ (charset != ശൂന്യ)
		സ്വതന്ത്രം(charset);
	എങ്കിൽ (target != ശൂന്യ)
		സ്വതന്ത്രം(target);
	തിരികെ ret;
}

/* documented in html_internal.h */
ഘടന form *html_forms_get_forms(സ്ഥിരം അക്ഷരം *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ഘടന form *ret = ശൂന്യ, *newf;
	dom_കണ്ണി *കണ്ണി;
	unsigned long n;
	uint32_t nforms;

	എങ്കിൽ (doc == ശൂന്യ)
		തിരികെ ശൂന്യ;

	/* Attempt to build a set of all the forms */
	എങ്കിൽ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		തിരികെ ശൂന്യ;

	/* Count the number of forms so we can iterate */
	എങ്കിൽ (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		പോകുക out;

	/* Iterate the forms collection, making form structs for returning */
	വേണ്ടി (n = 0; n < nforms; ++n) {
		എങ്കിൽ (dom_html_collection_item(forms, n, &കണ്ണി) != DOM_NO_ERR) {
			പോകുക out;
		}
		newf = parse_form_element(docenc, കണ്ണി);
		dom_കണ്ണി_unref(കണ്ണി);
		എങ്കിൽ (newf == ശൂന്യ) {
			പോകുക err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	പോകുക out;
err:
	ആകുമ്പോൾ (ret != ശൂന്യ) {
		ഘടന form *prev = ret->prev;
		/* Destroy ret */
		സ്വതന്ത്രം(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	തിരികെ ret;
}

സ്ഥിതം ഘടന form *
find_form(ഘടന form *forms, dom_html_form_element *form)
{
	ആകുമ്പോൾ (forms != ശൂന്യ) {
		എങ്കിൽ (forms->കണ്ണി == form)
			നിർത്തുക;
		forms = forms->prev;
	}

	തിരികെ forms;
}

സ്ഥിതം ഘടന form_control *
parse_button_element(ഘടന form *forms, dom_html_button_element *button)
{
	ഘടന form_control *control = ശൂന്യ;
	dom_exception err;
	dom_html_form_element *form = ശൂന്യ;
	dom_string *ds_type = ശൂന്യ;
	dom_string *ds_മൂല്യം = ശൂന്യ;
	dom_string *ds_name = ശൂന്യ;

	err = dom_html_button_element_get_form(button, &form);
	എങ്കിൽ (err != DOM_NO_ERR)
		പോകുക out;

	err = dom_html_button_element_get_type(button, &ds_type);
	എങ്കിൽ (err != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (ds_type == ശൂന്യ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} അല്ലെങ്കിൽ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	എങ്കിൽ (control == ശൂന്യ)
		പോകുക out;

	err = dom_html_button_element_get_മൂല്യം(button, &ds_മൂല്യം);
	എങ്കിൽ (err != DOM_NO_ERR)
		പോകുക out;
	err = dom_html_button_element_get_name(button, &ds_name);
	എങ്കിൽ (err != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (ds_മൂല്യം != ശൂന്യ) {
		control->മൂല്യം = strndup(
			dom_string_data(ds_മൂല്യം),
			dom_string_byte_length(ds_മൂല്യം));

		എങ്കിൽ (control->മൂല്യം == ശൂന്യ) {
			form_free_control(control);
			control = ശൂന്യ;
			പോകുക out;
		}
	}

	എങ്കിൽ (ds_name != ശൂന്യ) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		എങ്കിൽ (control->name == ശൂന്യ) {
			form_free_control(control);
			control = ശൂന്യ;
			പോകുക out;
		}
	}

	എങ്കിൽ (form != ശൂന്യ && control != ശൂന്യ)
		form_add_control(find_form(forms, form), control);

out:
	എങ്കിൽ (form != ശൂന്യ)
		dom_കണ്ണി_unref(form);
	എങ്കിൽ (ds_type != ശൂന്യ)
		dom_string_unref(ds_type);
	എങ്കിൽ (ds_മൂല്യം != ശൂന്യ)
		dom_string_unref(ds_മൂല്യം);
	എങ്കിൽ (ds_name != ശൂന്യ)
		dom_string_unref(ds_name);

	തിരികെ control;
}

സ്ഥിതം ഘടന form_control *
parse_നിവേശം_element(ഘടന form *forms, dom_html_നിവേശം_element *നിവേശം)
{
	ഘടന form_control *control = ശൂന്യ;
	dom_html_form_element *form = ശൂന്യ;
	dom_string *ds_type = ശൂന്യ;
	dom_string *ds_name = ശൂന്യ;
	dom_string *ds_മൂല്യം = ശൂന്യ;

	അക്ഷരം *name = ശൂന്യ;

	എങ്കിൽ (dom_html_നിവേശം_element_get_form(നിവേശം, &form) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_നിവേശം_element_get_type(നിവേശം, &ds_type) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_നിവേശം_element_get_name(നിവേശം, &ds_name) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (ds_name != ശൂന്യ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(നിവേശം, GADGET_PASSWORD);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(നിവേശം, GADGET_FILE);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(നിവേശം, GADGET_HIDDEN);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(നിവേശം, GADGET_CHECKBOX);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(നിവേശം, GADGET_RADIO);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(നിവേശം, GADGET_SUBMIT);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(നിവേശം, GADGET_RESET);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(നിവേശം, GADGET_BUTTON);
	} അല്ലെങ്കിൽ എങ്കിൽ (ds_type != ശൂന്യ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(നിവേശം, GADGET_IMAGE);
	} അല്ലെങ്കിൽ {
		control = form_new_control(നിവേശം, GADGET_TEXTBOX);
	}

	എങ്കിൽ (control == ശൂന്യ)
		പോകുക out;

	എങ്കിൽ (name != ശൂന്യ) {
		/* Hand the name string over */
		control->name = name;
		name = ശൂന്യ;
	}

	എങ്കിൽ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		എങ്കിൽ (dom_html_നിവേശം_element_get_checked(
			    നിവേശം, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	എങ്കിൽ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		എങ്കിൽ (dom_html_നിവേശം_element_get_max_length(
			    നിവേശം, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		എങ്കിൽ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} അല്ലെങ്കിൽ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	എങ്കിൽ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		എങ്കിൽ (dom_html_നിവേശം_element_get_മൂല്യം(
			    നിവേശം, &ds_മൂല്യം) == DOM_NO_ERR) {
			എങ്കിൽ (ds_മൂല്യം != ശൂന്യ) {
				control->മൂല്യം = strndup(
					dom_string_data(ds_മൂല്യം),
					dom_string_byte_length(ds_മൂല്യം));
				എങ്കിൽ (control->മൂല്യം == ശൂന്യ) {
					form_free_control(control);
					control = ശൂന്യ;
					പോകുക out;
				}
				control->length = strlen(control->മൂല്യം);
			}
		}

		എങ്കിൽ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			എങ്കിൽ (control->മൂല്യം == ശൂന്യ) {
				control->മൂല്യം = strdup("");
				എങ്കിൽ (control->മൂല്യം == ശൂന്യ) {
					form_free_control(control);
					control = ശൂന്യ;
					പോകുക out;
				}

				control->length = 0;
			}

			control->initial_മൂല്യം = strdup(control->മൂല്യം);
			എങ്കിൽ (control->initial_മൂല്യം == ശൂന്യ) {
				form_free_control(control);
				control = ശൂന്യ;
				പോകുക out;
			}

			control->last_synced_മൂല്യം = strdup(control->മൂല്യം);
			എങ്കിൽ (control->last_synced_മൂല്യം == ശൂന്യ) {
				form_free_control(control);
				control = ശൂന്യ;
				പോകുക out;
			}

			control->കണ്ണി_മൂല്യം = dom_string_ref(ds_മൂല്യം);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	എങ്കിൽ (form != ശൂന്യ && control != ശൂന്യ)
		form_add_control(find_form(forms, form), control);

out:
	എങ്കിൽ (form != ശൂന്യ)
		dom_കണ്ണി_unref(form);
	എങ്കിൽ (ds_type != ശൂന്യ)
		dom_string_unref(ds_type);
	എങ്കിൽ (ds_name != ശൂന്യ)
		dom_string_unref(ds_name);
	എങ്കിൽ (ds_മൂല്യം != ശൂന്യ)
		dom_string_unref(ds_മൂല്യം);

	എങ്കിൽ (name != ശൂന്യ)
		സ്വതന്ത്രം(name);

	തിരികെ control;
}

സ്ഥിതം ഘടന form_control *
parse_textarea_element(ഘടന form *forms, dom_html_text_area_element *ta)
{
	ഘടന form_control *control = ശൂന്യ;
	dom_html_form_element *form = ശൂന്യ;
	dom_string *ds_name = ശൂന്യ;

	അക്ഷരം *name = ശൂന്യ;

	എങ്കിൽ (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (ds_name != ശൂന്യ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	എങ്കിൽ (control == ശൂന്യ)
		പോകുക out;

	എങ്കിൽ (name != ശൂന്യ) {
		/* Hand the name string over */
		control->name = name;
		name = ശൂന്യ;
	}

	എങ്കിൽ (form != ശൂന്യ && control != ശൂന്യ)
		form_add_control(find_form(forms, form), control);

out:
	എങ്കിൽ (form != ശൂന്യ)
		dom_കണ്ണി_unref(form);
	എങ്കിൽ (ds_name != ശൂന്യ)
		dom_string_unref(ds_name);

	എങ്കിൽ (name != ശൂന്യ)
		സ്വതന്ത്രം(name);


	തിരികെ control;
}

സ്ഥിതം ഘടന form_control *
parse_select_element(ഘടന form *forms, dom_html_select_element *select)
{
	ഘടന form_control *control = ശൂന്യ;
	dom_html_form_element *form = ശൂന്യ;
	dom_string *ds_name = ശൂന്യ;

	അക്ഷരം *name = ശൂന്യ;

	എങ്കിൽ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		പോകുക out;

	എങ്കിൽ (ds_name != ശൂന്യ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	എങ്കിൽ (control == ശൂന്യ)
		പോകുക out;

	എങ്കിൽ (name != ശൂന്യ) {
		/* Hand the name string over */
		control->name = name;
		name = ശൂന്യ;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	എങ്കിൽ (form != ശൂന്യ && control != ശൂന്യ)
		form_add_control(find_form(forms, form), control);

out:
	എങ്കിൽ (form != ശൂന്യ)
		dom_കണ്ണി_unref(form);
	എങ്കിൽ (ds_name != ശൂന്യ)
		dom_string_unref(ds_name);

	എങ്കിൽ (name != ശൂന്യ)
		സ്വതന്ത്രം(name);


	തിരികെ control;
}


സ്ഥിതം ഘടന form_control *
invent_fake_gadget(dom_കണ്ണി *കണ്ണി)
{
	ഘടന form_control *ctl = form_new_control(കണ്ണി, GADGET_HIDDEN);
	എങ്കിൽ (ctl != ശൂന്യ) {
		ctl->മൂല്യം = strdup("");
		ctl->initial_മൂല്യം = strdup("");
		ctl->name = strdup("foo");

		എങ്കിൽ (ctl->മൂല്യം == ശൂന്യ || ctl->initial_മൂല്യം == ശൂന്യ ||
				ctl->name == ശൂന്യ) {
			form_free_control(ctl);
			ctl = ശൂന്യ;
		}
	}
	തിരികെ ctl;
}

/* documented in html_internal.h */
ഘടന form_control *
html_forms_get_control_for_കണ്ണി(ഘടന form *forms, dom_കണ്ണി *കണ്ണി)
{
	ഘടന form *f;
	ഘടന form_control *ctl = ശൂന്യ;
	dom_exception err;
	dom_string *ds_name = ശൂന്യ;

	/* Step one, see if we already have a control */
	വേണ്ടി (f = forms; f != ശൂന്യ; f = f->prev) {
		വേണ്ടി (ctl = f->controls; ctl != ശൂന്യ; ctl = ctl->next) {
			എങ്കിൽ (ctl->കണ്ണി == കണ്ണി)
				തിരികെ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(കണ്ണി, &ds_name);
	എങ്കിൽ (err == DOM_NO_ERR && ds_name != ശൂന്യ) {

		/* Step three, attempt to work out what gadget to make */
		എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) കണ്ണി);
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_നിവേശം)) {
			ctl = parse_നിവേശം_element(forms,
					(dom_html_നിവേശം_element *) കണ്ണി);
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) കണ്ണി);
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) കണ്ണി);
		}
	}

	/* If all else fails, fake gadget time */
	എങ്കിൽ (ctl == ശൂന്യ)
		ctl = invent_fake_gadget(കണ്ണി);

	എങ്കിൽ (ds_name != ശൂന്യ)
		dom_string_unref(ds_name);

	തിരികെ ctl;
}
