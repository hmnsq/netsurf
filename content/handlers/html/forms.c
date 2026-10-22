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

#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"

#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/private.h"

/**
 * process form element from dom
 */
સ્થિર સંરચના form *
parse_form_element(અચલ અક્ષર *docenc, dom_ગાંઠ *ગાંઠ)
{
	dom_string *ds_action = ખાલી;
	dom_string *ds_charset = ખાલી;
	dom_string *ds_target = ખાલી;
	dom_string *ds_method = ખાલી;
	dom_string *ds_enctype = ખાલી;
	અક્ષર *action = ખાલી, *charset = ખાલી, *target = ખાલી;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(ગાંઠ);
	સંરચના form * ret = ખાલી;

	/* Retrieve the attributes from the node */
	જો (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		જાઓ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	જો (ds_action != ખાલી)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	જો (ds_charset != ખાલી)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	જો (ds_target != ખાલી)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	જો (ds_method != ખાલી) {
		જો (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			જો (ds_enctype != ખાલી) {
				જો (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ગાંઠ, action, target, method, charset, docenc);

out:
	જો (ds_action != ખાલી)
		dom_string_unref(ds_action);
	જો (ds_charset != ખાલી)
		dom_string_unref(ds_charset);
	જો (ds_target != ખાલી)
		dom_string_unref(ds_target);
	જો (ds_method != ખાલી)
		dom_string_unref(ds_method);
	જો (ds_enctype != ખાલી)
		dom_string_unref(ds_enctype);
	જો (action != ખાલી)
		મુક્ત(action);
	જો (charset != ખાલી)
		મુક્ત(charset);
	જો (target != ખાલી)
		મુક્ત(target);
	પાછા ret;
}

/* documented in html_internal.h */
સંરચના form *html_forms_get_forms(અચલ અક્ષર *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	સંરચના form *ret = ખાલી, *newf;
	dom_ગાંઠ *ગાંઠ;
	unsigned long n;
	uint32_t nforms;

	જો (doc == ખાલી)
		પાછા ખાલી;

	/* Attempt to build a set of all the forms */
	જો (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		પાછા ખાલી;

	/* Count the number of forms so we can iterate */
	જો (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		જાઓ out;

	/* Iterate the forms collection, making form structs for returning */
	માટે (n = 0; n < nforms; ++n) {
		જો (dom_html_collection_item(forms, n, &ગાંઠ) != DOM_NO_ERR) {
			જાઓ out;
		}
		newf = parse_form_element(docenc, ગાંઠ);
		dom_ગાંઠ_unref(ગાંઠ);
		જો (newf == ખાલી) {
			જાઓ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	જાઓ out;
err:
	જ્યારે (ret != ખાલી) {
		સંરચના form *prev = ret->prev;
		/* Destroy ret */
		મુક્ત(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	પાછા ret;
}

સ્થિર સંરચના form *
find_form(સંરચના form *forms, dom_html_form_element *form)
{
	જ્યારે (forms != ખાલી) {
		જો (forms->ગાંઠ == form)
			તોડો;
		forms = forms->prev;
	}

	પાછા forms;
}

સ્થિર સંરચના form_control *
parse_button_element(સંરચના form *forms, dom_html_button_element *button)
{
	સંરચના form_control *control = ખાલી;
	dom_exception err;
	dom_html_form_element *form = ખાલી;
	dom_string *ds_type = ખાલી;
	dom_string *ds_કિંમત = ખાલી;
	dom_string *ds_name = ખાલી;

	err = dom_html_button_element_get_form(button, &form);
	જો (err != DOM_NO_ERR)
		જાઓ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	જો (err != DOM_NO_ERR)
		જાઓ out;

	જો (ds_type == ખાલી) {
		control = form_new_control(button, GADGET_SUBMIT);
	} નહીંતર {
		જો (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} નહીંતર જો (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} નહીંતર {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	જો (control == ખાલી)
		જાઓ out;

	err = dom_html_button_element_get_કિંમત(button, &ds_કિંમત);
	જો (err != DOM_NO_ERR)
		જાઓ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	જો (err != DOM_NO_ERR)
		જાઓ out;

	જો (ds_કિંમત != ખાલી) {
		control->કિંમત = strndup(
			dom_string_data(ds_કિંમત),
			dom_string_byte_length(ds_કિંમત));

		જો (control->કિંમત == ખાલી) {
			form_free_control(control);
			control = ખાલી;
			જાઓ out;
		}
	}

	જો (ds_name != ખાલી) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		જો (control->name == ખાલી) {
			form_free_control(control);
			control = ખાલી;
			જાઓ out;
		}
	}

	જો (form != ખાલી && control != ખાલી)
		form_add_control(find_form(forms, form), control);

out:
	જો (form != ખાલી)
		dom_ગાંઠ_unref(form);
	જો (ds_type != ખાલી)
		dom_string_unref(ds_type);
	જો (ds_કિંમત != ખાલી)
		dom_string_unref(ds_કિંમત);
	જો (ds_name != ખાલી)
		dom_string_unref(ds_name);

	પાછા control;
}

સ્થિર સંરચના form_control *
parse_આગત_element(સંરચના form *forms, dom_html_આગત_element *આગત)
{
	સંરચના form_control *control = ખાલી;
	dom_html_form_element *form = ખાલી;
	dom_string *ds_type = ખાલી;
	dom_string *ds_name = ખાલી;
	dom_string *ds_કિંમત = ખાલી;

	અક્ષર *name = ખાલી;

	જો (dom_html_આગત_element_get_form(આગત, &form) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_આગત_element_get_type(આગત, &ds_type) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_આગત_element_get_name(આગત, &ds_name) != DOM_NO_ERR)
		જાઓ out;

	જો (ds_name != ખાલી)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(આગત, GADGET_PASSWORD);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(આગત, GADGET_FILE);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(આગત, GADGET_HIDDEN);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(આગત, GADGET_CHECKBOX);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(આગત, GADGET_RADIO);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(આગત, GADGET_SUBMIT);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(આગત, GADGET_RESET);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(આગત, GADGET_BUTTON);
	} નહીંતર જો (ds_type != ખાલી && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(આગત, GADGET_IMAGE);
	} નહીંતર {
		control = form_new_control(આગત, GADGET_TEXTBOX);
	}

	જો (control == ખાલી)
		જાઓ out;

	જો (name != ખાલી) {
		/* Hand the name string over */
		control->name = name;
		name = ખાલી;
	}

	જો (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		જો (dom_html_આગત_element_get_checked(
			    આગત, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	જો (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		જો (dom_html_આગત_element_get_max_length(
			    આગત, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		જો (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} નહીંતર {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	જો (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		જો (dom_html_આગત_element_get_કિંમત(
			    આગત, &ds_કિંમત) == DOM_NO_ERR) {
			જો (ds_કિંમત != ખાલી) {
				control->કિંમત = strndup(
					dom_string_data(ds_કિંમત),
					dom_string_byte_length(ds_કિંમત));
				જો (control->કિંમત == ખાલી) {
					form_free_control(control);
					control = ખાલી;
					જાઓ out;
				}
				control->length = strlen(control->કિંમત);
			}
		}

		જો (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			જો (control->કિંમત == ખાલી) {
				control->કિંમત = strdup("");
				જો (control->કિંમત == ખાલી) {
					form_free_control(control);
					control = ખાલી;
					જાઓ out;
				}

				control->length = 0;
			}

			control->initial_કિંમત = strdup(control->કિંમત);
			જો (control->initial_કિંમત == ખાલી) {
				form_free_control(control);
				control = ખાલી;
				જાઓ out;
			}

			control->last_synced_કિંમત = strdup(control->કિંમત);
			જો (control->last_synced_કિંમત == ખાલી) {
				form_free_control(control);
				control = ખાલી;
				જાઓ out;
			}

			control->ગાંઠ_કિંમત = dom_string_ref(ds_કિંમત);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	જો (form != ખાલી && control != ખાલી)
		form_add_control(find_form(forms, form), control);

out:
	જો (form != ખાલી)
		dom_ગાંઠ_unref(form);
	જો (ds_type != ખાલી)
		dom_string_unref(ds_type);
	જો (ds_name != ખાલી)
		dom_string_unref(ds_name);
	જો (ds_કિંમત != ખાલી)
		dom_string_unref(ds_કિંમત);

	જો (name != ખાલી)
		મુક્ત(name);

	પાછા control;
}

સ્થિર સંરચના form_control *
parse_textarea_element(સંરચના form *forms, dom_html_text_area_element *ta)
{
	સંરચના form_control *control = ખાલી;
	dom_html_form_element *form = ખાલી;
	dom_string *ds_name = ખાલી;

	અક્ષર *name = ખાલી;

	જો (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		જાઓ out;

	જો (ds_name != ખાલી)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	જો (control == ખાલી)
		જાઓ out;

	જો (name != ખાલી) {
		/* Hand the name string over */
		control->name = name;
		name = ખાલી;
	}

	જો (form != ખાલી && control != ખાલી)
		form_add_control(find_form(forms, form), control);

out:
	જો (form != ખાલી)
		dom_ગાંઠ_unref(form);
	જો (ds_name != ખાલી)
		dom_string_unref(ds_name);

	જો (name != ખાલી)
		મુક્ત(name);


	પાછા control;
}

સ્થિર સંરચના form_control *
parse_select_element(સંરચના form *forms, dom_html_select_element *select)
{
	સંરચના form_control *control = ખાલી;
	dom_html_form_element *form = ખાલી;
	dom_string *ds_name = ખાલી;

	અક્ષર *name = ખાલી;

	જો (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		જાઓ out;

	જો (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		જાઓ out;

	જો (ds_name != ખાલી)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	જો (control == ખાલી)
		જાઓ out;

	જો (name != ખાલી) {
		/* Hand the name string over */
		control->name = name;
		name = ખાલી;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	જો (form != ખાલી && control != ખાલી)
		form_add_control(find_form(forms, form), control);

out:
	જો (form != ખાલી)
		dom_ગાંઠ_unref(form);
	જો (ds_name != ખાલી)
		dom_string_unref(ds_name);

	જો (name != ખાલી)
		મુક્ત(name);


	પાછા control;
}


સ્થિર સંરચના form_control *
invent_fake_gadget(dom_ગાંઠ *ગાંઠ)
{
	સંરચના form_control *ctl = form_new_control(ગાંઠ, GADGET_HIDDEN);
	જો (ctl != ખાલી) {
		ctl->કિંમત = strdup("");
		ctl->initial_કિંમત = strdup("");
		ctl->name = strdup("foo");

		જો (ctl->કિંમત == ખાલી || ctl->initial_કિંમત == ખાલી ||
				ctl->name == ખાલી) {
			form_free_control(ctl);
			ctl = ખાલી;
		}
	}
	પાછા ctl;
}

/* documented in html_internal.h */
સંરચના form_control *
html_forms_get_control_for_ગાંઠ(સંરચના form *forms, dom_ગાંઠ *ગાંઠ)
{
	સંરચના form *f;
	સંરચના form_control *ctl = ખાલી;
	dom_exception err;
	dom_string *ds_name = ખાલી;

	/* Step one, see if we already have a control */
	માટે (f = forms; f != ખાલી; f = f->prev) {
		માટે (ctl = f->controls; ctl != ખાલી; ctl = ctl->next) {
			જો (ctl->ગાંઠ == ગાંઠ)
				પાછા ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(ગાંઠ, &ds_name);
	જો (err == DOM_NO_ERR && ds_name != ખાલી) {

		/* Step three, attempt to work out what gadget to make */
		જો (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ગાંઠ);
		} નહીંતર જો (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_આગત)) {
			ctl = parse_આગત_element(forms,
					(dom_html_આગત_element *) ગાંઠ);
		} નહીંતર જો (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) ગાંઠ);
		} નહીંતર જો (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ગાંઠ);
		}
	}

	/* If all else fails, fake gadget time */
	જો (ctl == ખાલી)
		ctl = invent_fake_gadget(ગાંઠ);

	જો (ds_name != ખાલી)
		dom_string_unref(ds_name);

	પાછા ctl;
}
