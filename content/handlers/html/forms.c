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

#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "utils/config.h"
#অন্তর্ভাৱ "utils/corestrings.h"
#অন্তর্ভাৱ "utils/log.h"

#অন্তর্ভাৱ "html/form_internal.h"
#অন্তর্ভাৱ "html/private.h"

/**
 * process form element from dom
 */
স্থির সংরচনা form *
parse_form_element(ধ্রুৱ ৱর্ণ *docenc, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	dom_string *ds_action = রিক্ত;
	dom_string *ds_charset = রিক্ত;
	dom_string *ds_target = রিক্ত;
	dom_string *ds_method = রিক্ত;
	dom_string *ds_enctype = রিক্ত;
	ৱর্ণ *action = রিক্ত, *charset = রিক্ত, *target = রিক্ত;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(গ্ৰন্থি);
	সংরচনা form * ret = রিক্ত;

	/* Retrieve the attributes from the node */
	যদি (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		গচ্ছ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	যদি (ds_action != রিক্ত)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	যদি (ds_charset != রিক্ত)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	যদি (ds_target != রিক্ত)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	যদি (ds_method != রিক্ত) {
		যদি (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			যদি (ds_enctype != রিক্ত) {
				যদি (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(গ্ৰন্থি, action, target, method, charset, docenc);

out:
	যদি (ds_action != রিক্ত)
		dom_string_unref(ds_action);
	যদি (ds_charset != রিক্ত)
		dom_string_unref(ds_charset);
	যদি (ds_target != রিক্ত)
		dom_string_unref(ds_target);
	যদি (ds_method != রিক্ত)
		dom_string_unref(ds_method);
	যদি (ds_enctype != রিক্ত)
		dom_string_unref(ds_enctype);
	যদি (action != রিক্ত)
		মুক্ত(action);
	যদি (charset != রিক্ত)
		মুক্ত(charset);
	যদি (target != রিক্ত)
		মুক্ত(target);
	ঘূৰাই ret;
}

/* documented in html_internal.h */
সংরচনা form *html_forms_get_forms(ধ্রুৱ ৱর্ণ *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	সংরচনা form *ret = রিক্ত, *newf;
	dom_গ্ৰন্থি *গ্ৰন্থি;
	unsigned long n;
	uint32_t nforms;

	যদি (doc == রিক্ত)
		ঘূৰাই রিক্ত;

	/* Attempt to build a set of all the forms */
	যদি (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ঘূৰাই রিক্ত;

	/* Count the number of forms so we can iterate */
	যদি (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		গচ্ছ out;

	/* Iterate the forms collection, making form structs for returning */
	বাবে (n = 0; n < nforms; ++n) {
		যদি (dom_html_collection_item(forms, n, &গ্ৰন্থি) != DOM_NO_ERR) {
			গচ্ছ out;
		}
		newf = parse_form_element(docenc, গ্ৰন্থি);
		dom_গ্ৰন্থি_unref(গ্ৰন্থি);
		যদি (newf == রিক্ত) {
			গচ্ছ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	গচ্ছ out;
err:
	যেতিয়ালৈ (ret != রিক্ত) {
		সংরচনা form *prev = ret->prev;
		/* Destroy ret */
		মুক্ত(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ঘূৰাই ret;
}

স্থির সংরচনা form *
find_form(সংরচনা form *forms, dom_html_form_element *form)
{
	যেতিয়ালৈ (forms != রিক্ত) {
		যদি (forms->গ্ৰন্থি == form)
			ভঙ্গ;
		forms = forms->prev;
	}

	ঘূৰাই forms;
}

স্থির সংরচনা form_control *
parse_button_element(সংরচনা form *forms, dom_html_button_element *button)
{
	সংরচনা form_control *control = রিক্ত;
	dom_exception err;
	dom_html_form_element *form = রিক্ত;
	dom_string *ds_type = রিক্ত;
	dom_string *ds_মান = রিক্ত;
	dom_string *ds_name = রিক্ত;

	err = dom_html_button_element_get_form(button, &form);
	যদি (err != DOM_NO_ERR)
		গচ্ছ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	যদি (err != DOM_NO_ERR)
		গচ্ছ out;

	যদি (ds_type == রিক্ত) {
		control = form_new_control(button, GADGET_SUBMIT);
	} অন্যথা {
		যদি (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} অন্যথা {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	যদি (control == রিক্ত)
		গচ্ছ out;

	err = dom_html_button_element_get_মান(button, &ds_মান);
	যদি (err != DOM_NO_ERR)
		গচ্ছ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	যদি (err != DOM_NO_ERR)
		গচ্ছ out;

	যদি (ds_মান != রিক্ত) {
		control->মান = strndup(
			dom_string_data(ds_মান),
			dom_string_byte_length(ds_মান));

		যদি (control->মান == রিক্ত) {
			form_free_control(control);
			control = রিক্ত;
			গচ্ছ out;
		}
	}

	যদি (ds_name != রিক্ত) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		যদি (control->name == রিক্ত) {
			form_free_control(control);
			control = রিক্ত;
			গচ্ছ out;
		}
	}

	যদি (form != রিক্ত && control != রিক্ত)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != রিক্ত)
		dom_গ্ৰন্থি_unref(form);
	যদি (ds_type != রিক্ত)
		dom_string_unref(ds_type);
	যদি (ds_মান != রিক্ত)
		dom_string_unref(ds_মান);
	যদি (ds_name != রিক্ত)
		dom_string_unref(ds_name);

	ঘূৰাই control;
}

স্থির সংরচনা form_control *
parse_প্ৰবেশ_element(সংরচনা form *forms, dom_html_প্ৰবেশ_element *প্ৰবেশ)
{
	সংরচনা form_control *control = রিক্ত;
	dom_html_form_element *form = রিক্ত;
	dom_string *ds_type = রিক্ত;
	dom_string *ds_name = রিক্ত;
	dom_string *ds_মান = রিক্ত;

	ৱর্ণ *name = রিক্ত;

	যদি (dom_html_প্ৰবেশ_element_get_form(প্ৰবেশ, &form) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_প্ৰবেশ_element_get_type(প্ৰবেশ, &ds_type) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_প্ৰবেশ_element_get_name(প্ৰবেশ, &ds_name) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (ds_name != রিক্ত)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(প্ৰবেশ, GADGET_PASSWORD);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(প্ৰবেশ, GADGET_FILE);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(প্ৰবেশ, GADGET_HIDDEN);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(প্ৰবেশ, GADGET_CHECKBOX);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(প্ৰবেশ, GADGET_RADIO);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(প্ৰবেশ, GADGET_SUBMIT);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(প্ৰবেশ, GADGET_RESET);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(প্ৰবেশ, GADGET_BUTTON);
	} অন্যথা যদি (ds_type != রিক্ত && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(প্ৰবেশ, GADGET_IMAGE);
	} অন্যথা {
		control = form_new_control(প্ৰবেশ, GADGET_TEXTBOX);
	}

	যদি (control == রিক্ত)
		গচ্ছ out;

	যদি (name != রিক্ত) {
		/* Hand the name string over */
		control->name = name;
		name = রিক্ত;
	}

	যদি (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		যদি (dom_html_প্ৰবেশ_element_get_checked(
			    প্ৰবেশ, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	যদি (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		যদি (dom_html_প্ৰবেশ_element_get_max_length(
			    প্ৰবেশ, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		যদি (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} অন্যথা {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	যদি (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		যদি (dom_html_প্ৰবেশ_element_get_মান(
			    প্ৰবেশ, &ds_মান) == DOM_NO_ERR) {
			যদি (ds_মান != রিক্ত) {
				control->মান = strndup(
					dom_string_data(ds_মান),
					dom_string_byte_length(ds_মান));
				যদি (control->মান == রিক্ত) {
					form_free_control(control);
					control = রিক্ত;
					গচ্ছ out;
				}
				control->length = strlen(control->মান);
			}
		}

		যদি (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			যদি (control->মান == রিক্ত) {
				control->মান = strdup("");
				যদি (control->মান == রিক্ত) {
					form_free_control(control);
					control = রিক্ত;
					গচ্ছ out;
				}

				control->length = 0;
			}

			control->initial_মান = strdup(control->মান);
			যদি (control->initial_মান == রিক্ত) {
				form_free_control(control);
				control = রিক্ত;
				গচ্ছ out;
			}

			control->last_synced_মান = strdup(control->মান);
			যদি (control->last_synced_মান == রিক্ত) {
				form_free_control(control);
				control = রিক্ত;
				গচ্ছ out;
			}

			control->গ্ৰন্থি_মান = dom_string_ref(ds_মান);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	যদি (form != রিক্ত && control != রিক্ত)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != রিক্ত)
		dom_গ্ৰন্থি_unref(form);
	যদি (ds_type != রিক্ত)
		dom_string_unref(ds_type);
	যদি (ds_name != রিক্ত)
		dom_string_unref(ds_name);
	যদি (ds_মান != রিক্ত)
		dom_string_unref(ds_মান);

	যদি (name != রিক্ত)
		মুক্ত(name);

	ঘূৰাই control;
}

স্থির সংরচনা form_control *
parse_textarea_element(সংরচনা form *forms, dom_html_text_area_element *ta)
{
	সংরচনা form_control *control = রিক্ত;
	dom_html_form_element *form = রিক্ত;
	dom_string *ds_name = রিক্ত;

	ৱর্ণ *name = রিক্ত;

	যদি (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (ds_name != রিক্ত)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	যদি (control == রিক্ত)
		গচ্ছ out;

	যদি (name != রিক্ত) {
		/* Hand the name string over */
		control->name = name;
		name = রিক্ত;
	}

	যদি (form != রিক্ত && control != রিক্ত)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != রিক্ত)
		dom_গ্ৰন্থি_unref(form);
	যদি (ds_name != রিক্ত)
		dom_string_unref(ds_name);

	যদি (name != রিক্ত)
		মুক্ত(name);


	ঘূৰাই control;
}

স্থির সংরচনা form_control *
parse_select_element(সংরচনা form *forms, dom_html_select_element *select)
{
	সংরচনা form_control *control = রিক্ত;
	dom_html_form_element *form = রিক্ত;
	dom_string *ds_name = রিক্ত;

	ৱর্ণ *name = রিক্ত;

	যদি (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		গচ্ছ out;

	যদি (ds_name != রিক্ত)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	যদি (control == রিক্ত)
		গচ্ছ out;

	যদি (name != রিক্ত) {
		/* Hand the name string over */
		control->name = name;
		name = রিক্ত;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	যদি (form != রিক্ত && control != রিক্ত)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != রিক্ত)
		dom_গ্ৰন্থি_unref(form);
	যদি (ds_name != রিক্ত)
		dom_string_unref(ds_name);

	যদি (name != রিক্ত)
		মুক্ত(name);


	ঘূৰাই control;
}


স্থির সংরচনা form_control *
invent_fake_gadget(dom_গ্ৰন্থি *গ্ৰন্থি)
{
	সংরচনা form_control *ctl = form_new_control(গ্ৰন্থি, GADGET_HIDDEN);
	যদি (ctl != রিক্ত) {
		ctl->মান = strdup("");
		ctl->initial_মান = strdup("");
		ctl->name = strdup("foo");

		যদি (ctl->মান == রিক্ত || ctl->initial_মান == রিক্ত ||
				ctl->name == রিক্ত) {
			form_free_control(ctl);
			ctl = রিক্ত;
		}
	}
	ঘূৰাই ctl;
}

/* documented in html_internal.h */
সংরচনা form_control *
html_forms_get_control_for_গ্ৰন্থি(সংরচনা form *forms, dom_গ্ৰন্থি *গ্ৰন্থি)
{
	সংরচনা form *f;
	সংরচনা form_control *ctl = রিক্ত;
	dom_exception err;
	dom_string *ds_name = রিক্ত;

	/* Step one, see if we already have a control */
	বাবে (f = forms; f != রিক্ত; f = f->prev) {
		বাবে (ctl = f->controls; ctl != রিক্ত; ctl = ctl->next) {
			যদি (ctl->গ্ৰন্থি == গ্ৰন্থি)
				ঘূৰাই ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(গ্ৰন্থি, &ds_name);
	যদি (err == DOM_NO_ERR && ds_name != রিক্ত) {

		/* Step three, attempt to work out what gadget to make */
		যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) গ্ৰন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_প্ৰবেশ)) {
			ctl = parse_প্ৰবেশ_element(forms,
					(dom_html_প্ৰবেশ_element *) গ্ৰন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) গ্ৰন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) গ্ৰন্থি);
		}
	}

	/* If all else fails, fake gadget time */
	যদি (ctl == রিক্ত)
		ctl = invent_fake_gadget(গ্ৰন্থি);

	যদি (ds_name != রিক্ত)
		dom_string_unref(ds_name);

	ঘূৰাই ctl;
}
