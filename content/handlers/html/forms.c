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

#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"

#অন্তর্ভাব "html/form_internal.h"
#অন্তর্ভাব "html/private.h"

/**
 * process form element from dom
 */
স্থির কাঠামো form *
parse_form_element(ধ্রুবক অক্ষর *docenc, dom_গ্রন্থি *গ্রন্থি)
{
	dom_string *ds_action = ফাঁকা;
	dom_string *ds_charset = ফাঁকা;
	dom_string *ds_target = ফাঁকা;
	dom_string *ds_method = ফাঁকা;
	dom_string *ds_enctype = ফাঁকা;
	অক্ষর *action = ফাঁকা, *charset = ফাঁকা, *target = ফাঁকা;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(গ্রন্থি);
	কাঠামো form * ret = ফাঁকা;

	/* Retrieve the attributes from the node */
	যদি (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		যাও out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	যদি (ds_action != ফাঁকা)
		action = strndup(dom_string_তথ্য(ds_action),
				 dom_string_byte_দৈর্ঘ্য(ds_action));

	যদি (ds_charset != ফাঁকা)
		charset = strndup(dom_string_তথ্য(ds_charset),
				  dom_string_byte_দৈর্ঘ্য(ds_charset));

	যদি (ds_target != ফাঁকা)
		target = strndup(dom_string_তথ্য(ds_target),
				 dom_string_byte_দৈর্ঘ্য(ds_target));

	/* Determine the method */
	method = method_GET;
	যদি (ds_method != ফাঁকা) {
		যদি (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			যদি (ds_enctype != ফাঁকা) {
				যদি (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_তথ্য)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(গ্রন্থি, action, target, method, charset, docenc);

out:
	যদি (ds_action != ফাঁকা)
		dom_string_unref(ds_action);
	যদি (ds_charset != ফাঁকা)
		dom_string_unref(ds_charset);
	যদি (ds_target != ফাঁকা)
		dom_string_unref(ds_target);
	যদি (ds_method != ফাঁকা)
		dom_string_unref(ds_method);
	যদি (ds_enctype != ফাঁকা)
		dom_string_unref(ds_enctype);
	যদি (action != ফাঁকা)
		মুক্ত(action);
	যদি (charset != ফাঁকা)
		মুক্ত(charset);
	যদি (target != ফাঁকা)
		মুক্ত(target);
	ফেরত ret;
}

/* documented in html_internal.h */
কাঠামো form *html_forms_get_forms(ধ্রুবক অক্ষর *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	কাঠামো form *ret = ফাঁকা, *newf;
	dom_গ্রন্থি *গ্রন্থি;
	unsigned long n;
	uint32_t nforms;

	যদি (doc == ফাঁকা)
		ফেরত ফাঁকা;

	/* Attempt to build a set of all the forms */
	যদি (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ফেরত ফাঁকা;

	/* Count the number of forms so we can iterate */
	যদি (dom_html_collection_get_দৈর্ঘ্য(forms, &nforms) != DOM_NO_ERR)
		যাও out;

	/* Iterate the forms collection, making form structs for returning */
	জন্য (n = 0; n < nforms; ++n) {
		যদি (dom_html_collection_item(forms, n, &গ্রন্থি) != DOM_NO_ERR) {
			যাও out;
		}
		newf = parse_form_element(docenc, গ্রন্থি);
		dom_গ্রন্থি_unref(গ্রন্থি);
		যদি (newf == ফাঁকা) {
			যাও err;
		}
		newf->পূর্ব = ret;
		ret = newf;
	}

	/* All went well */
	যাও out;
err:
	যখন (ret != ফাঁকা) {
		কাঠামো form *পূর্ব = ret->পূর্ব;
		/* Destroy ret */
		মুক্ত(ret);
		ret = পূর্ব;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ফেরত ret;
}

স্থির কাঠামো form *
find_form(কাঠামো form *forms, dom_html_form_element *form)
{
	যখন (forms != ফাঁকা) {
		যদি (forms->গ্রন্থি == form)
			বিরতি;
		forms = forms->পূর্ব;
	}

	ফেরত forms;
}

স্থির কাঠামো form_control *
parse_button_element(কাঠামো form *forms, dom_html_button_element *button)
{
	কাঠামো form_control *control = ফাঁকা;
	dom_exception err;
	dom_html_form_element *form = ফাঁকা;
	dom_string *ds_type = ফাঁকা;
	dom_string *ds_মান = ফাঁকা;
	dom_string *ds_name = ফাঁকা;

	err = dom_html_button_element_get_form(button, &form);
	যদি (err != DOM_NO_ERR)
		যাও out;

	err = dom_html_button_element_get_type(button, &ds_type);
	যদি (err != DOM_NO_ERR)
		যাও out;

	যদি (ds_type == ফাঁকা) {
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

	যদি (control == ফাঁকা)
		যাও out;

	err = dom_html_button_element_get_মান(button, &ds_মান);
	যদি (err != DOM_NO_ERR)
		যাও out;
	err = dom_html_button_element_get_name(button, &ds_name);
	যদি (err != DOM_NO_ERR)
		যাও out;

	যদি (ds_মান != ফাঁকা) {
		control->মান = strndup(
			dom_string_তথ্য(ds_মান),
			dom_string_byte_দৈর্ঘ্য(ds_মান));

		যদি (control->মান == ফাঁকা) {
			form_free_control(control);
			control = ফাঁকা;
			যাও out;
		}
	}

	যদি (ds_name != ফাঁকা) {
		control->name = strndup(
			dom_string_তথ্য(ds_name),
			dom_string_byte_দৈর্ঘ্য(ds_name));

		যদি (control->name == ফাঁকা) {
			form_free_control(control);
			control = ফাঁকা;
			যাও out;
		}
	}

	যদি (form != ফাঁকা && control != ফাঁকা)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != ফাঁকা)
		dom_গ্রন্থি_unref(form);
	যদি (ds_type != ফাঁকা)
		dom_string_unref(ds_type);
	যদি (ds_মান != ফাঁকা)
		dom_string_unref(ds_মান);
	যদি (ds_name != ফাঁকা)
		dom_string_unref(ds_name);

	ফেরত control;
}

স্থির কাঠামো form_control *
parse_প্রবেশ_element(কাঠামো form *forms, dom_html_প্রবেশ_element *প্রবেশ)
{
	কাঠামো form_control *control = ফাঁকা;
	dom_html_form_element *form = ফাঁকা;
	dom_string *ds_type = ফাঁকা;
	dom_string *ds_name = ফাঁকা;
	dom_string *ds_মান = ফাঁকা;

	অক্ষর *name = ফাঁকা;

	যদি (dom_html_প্রবেশ_element_get_form(প্রবেশ, &form) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_প্রবেশ_element_get_type(প্রবেশ, &ds_type) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_প্রবেশ_element_get_name(প্রবেশ, &ds_name) != DOM_NO_ERR)
		যাও out;

	যদি (ds_name != ফাঁকা)
		name = strndup(dom_string_তথ্য(ds_name),
			       dom_string_byte_দৈর্ঘ্য(ds_name));

	যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(প্রবেশ, GADGET_PASSWORD);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(প্রবেশ, GADGET_FILE);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(প্রবেশ, GADGET_HIDDEN);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(প্রবেশ, GADGET_CHECKBOX);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(প্রবেশ, GADGET_RADIO);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(প্রবেশ, GADGET_SUBMIT);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(প্রবেশ, GADGET_RESET);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(প্রবেশ, GADGET_BUTTON);
	} অন্যথা যদি (ds_type != ফাঁকা && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(প্রবেশ, GADGET_IMAGE);
	} অন্যথা {
		control = form_new_control(প্রবেশ, GADGET_TEXTBOX);
	}

	যদি (control == ফাঁকা)
		যাও out;

	যদি (name != ফাঁকা) {
		/* Hand the name string over */
		control->name = name;
		name = ফাঁকা;
	}

	যদি (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		যদি (dom_html_প্রবেশ_element_get_checked(
			    প্রবেশ, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	যদি (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		যদি (dom_html_প্রবেশ_element_get_max_দৈর্ঘ্য(
			    প্রবেশ, &maxlength) != DOM_NO_ERR) {
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
		যদি (dom_html_প্রবেশ_element_get_মান(
			    প্রবেশ, &ds_মান) == DOM_NO_ERR) {
			যদি (ds_মান != ফাঁকা) {
				control->মান = strndup(
					dom_string_তথ্য(ds_মান),
					dom_string_byte_দৈর্ঘ্য(ds_মান));
				যদি (control->মান == ফাঁকা) {
					form_free_control(control);
					control = ফাঁকা;
					যাও out;
				}
				control->দৈর্ঘ্য = strlen(control->মান);
			}
		}

		যদি (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			যদি (control->মান == ফাঁকা) {
				control->মান = strdup("");
				যদি (control->মান == ফাঁকা) {
					form_free_control(control);
					control = ফাঁকা;
					যাও out;
				}

				control->দৈর্ঘ্য = 0;
			}

			control->initial_মান = strdup(control->মান);
			যদি (control->initial_মান == ফাঁকা) {
				form_free_control(control);
				control = ফাঁকা;
				যাও out;
			}

			control->last_synced_মান = strdup(control->মান);
			যদি (control->last_synced_মান == ফাঁকা) {
				form_free_control(control);
				control = ফাঁকা;
				যাও out;
			}

			control->গ্রন্থি_মান = dom_string_ref(ds_মান);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	যদি (form != ফাঁকা && control != ফাঁকা)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != ফাঁকা)
		dom_গ্রন্থি_unref(form);
	যদি (ds_type != ফাঁকা)
		dom_string_unref(ds_type);
	যদি (ds_name != ফাঁকা)
		dom_string_unref(ds_name);
	যদি (ds_মান != ফাঁকা)
		dom_string_unref(ds_মান);

	যদি (name != ফাঁকা)
		মুক্ত(name);

	ফেরত control;
}

স্থির কাঠামো form_control *
parse_textarea_element(কাঠামো form *forms, dom_html_text_area_element *ta)
{
	কাঠামো form_control *control = ফাঁকা;
	dom_html_form_element *form = ফাঁকা;
	dom_string *ds_name = ফাঁকা;

	অক্ষর *name = ফাঁকা;

	যদি (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		যাও out;

	যদি (ds_name != ফাঁকা)
		name = strndup(dom_string_তথ্য(ds_name),
			       dom_string_byte_দৈর্ঘ্য(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	যদি (control == ফাঁকা)
		যাও out;

	যদি (name != ফাঁকা) {
		/* Hand the name string over */
		control->name = name;
		name = ফাঁকা;
	}

	যদি (form != ফাঁকা && control != ফাঁকা)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != ফাঁকা)
		dom_গ্রন্থি_unref(form);
	যদি (ds_name != ফাঁকা)
		dom_string_unref(ds_name);

	যদি (name != ফাঁকা)
		মুক্ত(name);


	ফেরত control;
}

স্থির কাঠামো form_control *
parse_select_element(কাঠামো form *forms, dom_html_select_element *select)
{
	কাঠামো form_control *control = ফাঁকা;
	dom_html_form_element *form = ফাঁকা;
	dom_string *ds_name = ফাঁকা;

	অক্ষর *name = ফাঁকা;

	যদি (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		যাও out;

	যদি (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		যাও out;

	যদি (ds_name != ফাঁকা)
		name = strndup(dom_string_তথ্য(ds_name),
			       dom_string_byte_দৈর্ঘ্য(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	যদি (control == ফাঁকা)
		যাও out;

	যদি (name != ফাঁকা) {
		/* Hand the name string over */
		control->name = name;
		name = ফাঁকা;
	}

	dom_html_select_element_get_multiple(select,
			&(control->তথ্য.select.multiple));

	যদি (form != ফাঁকা && control != ফাঁকা)
		form_add_control(find_form(forms, form), control);

out:
	যদি (form != ফাঁকা)
		dom_গ্রন্থি_unref(form);
	যদি (ds_name != ফাঁকা)
		dom_string_unref(ds_name);

	যদি (name != ফাঁকা)
		মুক্ত(name);


	ফেরত control;
}


স্থির কাঠামো form_control *
invent_fake_gadget(dom_গ্রন্থি *গ্রন্থি)
{
	কাঠামো form_control *ctl = form_new_control(গ্রন্থি, GADGET_HIDDEN);
	যদি (ctl != ফাঁকা) {
		ctl->মান = strdup("");
		ctl->initial_মান = strdup("");
		ctl->name = strdup("foo");

		যদি (ctl->মান == ফাঁকা || ctl->initial_মান == ফাঁকা ||
				ctl->name == ফাঁকা) {
			form_free_control(ctl);
			ctl = ফাঁকা;
		}
	}
	ফেরত ctl;
}

/* documented in html_internal.h */
কাঠামো form_control *
html_forms_get_control_for_গ্রন্থি(কাঠামো form *forms, dom_গ্রন্থি *গ্রন্থি)
{
	কাঠামো form *f;
	কাঠামো form_control *ctl = ফাঁকা;
	dom_exception err;
	dom_string *ds_name = ফাঁকা;

	/* Step one, see if we already have a control */
	জন্য (f = forms; f != ফাঁকা; f = f->পূর্ব) {
		জন্য (ctl = f->controls; ctl != ফাঁকা; ctl = ctl->পরবর্তী) {
			যদি (ctl->গ্রন্থি == গ্রন্থি)
				ফেরত ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(গ্রন্থি, &ds_name);
	যদি (err == DOM_NO_ERR && ds_name != ফাঁকা) {

		/* Step three, attempt to work out what gadget to make */
		যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) গ্রন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_প্রবেশ)) {
			ctl = parse_প্রবেশ_element(forms,
					(dom_html_প্রবেশ_element *) গ্রন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) গ্রন্থি);
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) গ্রন্থি);
		}
	}

	/* If all else fails, fake gadget time */
	যদি (ctl == ফাঁকা)
		ctl = invent_fake_gadget(গ্রন্থি);

	যদি (ds_name != ফাঁকা)
		dom_string_unref(ds_name);

	ফেরত ctl;
}
