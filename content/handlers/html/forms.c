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

#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"

#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"

/**
 * process form element from dom
 */
ସ୍ଥିର ସଂରଚନା form *
parse_form_element(ଅଚଳ ଅକ୍ଷର *docenc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_string *ds_action = ଖାଲି;
	dom_string *ds_charset = ଖାଲି;
	dom_string *ds_target = ଖାଲି;
	dom_string *ds_method = ଖାଲି;
	dom_string *ds_enctype = ଖାଲି;
	ଅକ୍ଷର *action = ଖାଲି, *charset = ଖାଲି, *target = ଖାଲି;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(ଗଣ୍ଠି);
	ସଂରଚନା form * ret = ଖାଲି;

	/* Retrieve the attributes from the node */
	ଯଦି (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		ଯାଅ out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ଯଦି (ds_action != ଖାଲି)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	ଯଦି (ds_charset != ଖାଲି)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	ଯଦି (ds_target != ଖାଲି)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	ଯଦି (ds_method != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			ଯଦି (ds_enctype != ଖାଲି) {
				ଯଦି (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ଗଣ୍ଠି, action, target, method, charset, docenc);

out:
	ଯଦି (ds_action != ଖାଲି)
		dom_string_unref(ds_action);
	ଯଦି (ds_charset != ଖାଲି)
		dom_string_unref(ds_charset);
	ଯଦି (ds_target != ଖାଲି)
		dom_string_unref(ds_target);
	ଯଦି (ds_method != ଖାଲି)
		dom_string_unref(ds_method);
	ଯଦି (ds_enctype != ଖାଲି)
		dom_string_unref(ds_enctype);
	ଯଦି (action != ଖାଲି)
		ମୁକ୍ତ(action);
	ଯଦି (charset != ଖାଲି)
		ମୁକ୍ତ(charset);
	ଯଦି (target != ଖାଲି)
		ମୁକ୍ତ(target);
	ଫେରନ୍ତୁ ret;
}

/* documented in html_internal.h */
ସଂରଚନା form *html_forms_get_forms(ଅଚଳ ଅକ୍ଷର *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ସଂରଚନା form *ret = ଖାଲି, *newf;
	dom_ଗଣ୍ଠି *ଗଣ୍ଠି;
	unsigned long n;
	uint32_t nforms;

	ଯଦି (doc == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	/* Attempt to build a set of all the forms */
	ଯଦି (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ଫେରନ୍ତୁ ଖାଲି;

	/* Count the number of forms so we can iterate */
	ଯଦି (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		ଯାଅ out;

	/* Iterate the forms collection, making form structs for returning */
	ପାଇଁ (n = 0; n < nforms; ++n) {
		ଯଦି (dom_html_collection_item(forms, n, &ଗଣ୍ଠି) != DOM_NO_ERR) {
			ଯାଅ out;
		}
		newf = parse_form_element(docenc, ଗଣ୍ଠି);
		dom_ଗଣ୍ଠି_unref(ଗଣ୍ଠି);
		ଯଦି (newf == ଖାଲି) {
			ଯାଅ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	ଯାଅ out;
err:
	ଯେତେବେଳେ (ret != ଖାଲି) {
		ସଂରଚନା form *prev = ret->prev;
		/* Destroy ret */
		ମୁକ୍ତ(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ସଂରଚନା form *
find_form(ସଂରଚନା form *forms, dom_html_form_element *form)
{
	ଯେତେବେଳେ (forms != ଖାଲି) {
		ଯଦି (forms->ଗଣ୍ଠି == form)
			ଭାଙ୍ଗ;
		forms = forms->prev;
	}

	ଫେରନ୍ତୁ forms;
}

ସ୍ଥିର ସଂରଚନା form_control *
parse_button_element(ସଂରଚନା form *forms, dom_html_button_element *button)
{
	ସଂରଚନା form_control *control = ଖାଲି;
	dom_exception err;
	dom_html_form_element *form = ଖାଲି;
	dom_string *ds_type = ଖାଲି;
	dom_string *ds_ମୂଲ୍ୟ = ଖାଲି;
	dom_string *ds_name = ଖାଲି;

	err = dom_html_button_element_get_form(button, &form);
	ଯଦି (err != DOM_NO_ERR)
		ଯାଅ out;

	err = dom_html_button_element_get_type(button, &ds_type);
	ଯଦି (err != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (ds_type == ଖାଲି) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ଅନ୍ୟଥା {
		ଯଦି (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ଅନ୍ୟଥା {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ଯଦି (control == ଖାଲି)
		ଯାଅ out;

	err = dom_html_button_element_get_ମୂଲ୍ୟ(button, &ds_ମୂଲ୍ୟ);
	ଯଦି (err != DOM_NO_ERR)
		ଯାଅ out;
	err = dom_html_button_element_get_name(button, &ds_name);
	ଯଦି (err != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (ds_ମୂଲ୍ୟ != ଖାଲି) {
		control->ମୂଲ୍ୟ = strndup(
			dom_string_data(ds_ମୂଲ୍ୟ),
			dom_string_byte_length(ds_ମୂଲ୍ୟ));

		ଯଦି (control->ମୂଲ୍ୟ == ଖାଲି) {
			form_free_control(control);
			control = ଖାଲି;
			ଯାଅ out;
		}
	}

	ଯଦି (ds_name != ଖାଲି) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		ଯଦି (control->name == ଖାଲି) {
			form_free_control(control);
			control = ଖାଲି;
			ଯାଅ out;
		}
	}

	ଯଦି (form != ଖାଲି && control != ଖାଲି)
		form_add_control(find_form(forms, form), control);

out:
	ଯଦି (form != ଖାଲି)
		dom_ଗଣ୍ଠି_unref(form);
	ଯଦି (ds_type != ଖାଲି)
		dom_string_unref(ds_type);
	ଯଦି (ds_ମୂଲ୍ୟ != ଖାଲି)
		dom_string_unref(ds_ମୂଲ୍ୟ);
	ଯଦି (ds_name != ଖାଲି)
		dom_string_unref(ds_name);

	ଫେରନ୍ତୁ control;
}

ସ୍ଥିର ସଂରଚନା form_control *
parse_ନିବେଶ_element(ସଂରଚନା form *forms, dom_html_ନିବେଶ_element *ନିବେଶ)
{
	ସଂରଚନା form_control *control = ଖାଲି;
	dom_html_form_element *form = ଖାଲି;
	dom_string *ds_type = ଖାଲି;
	dom_string *ds_name = ଖାଲି;
	dom_string *ds_ମୂଲ୍ୟ = ଖାଲି;

	ଅକ୍ଷର *name = ଖାଲି;

	ଯଦି (dom_html_ନିବେଶ_element_get_form(ନିବେଶ, &form) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_ନିବେଶ_element_get_type(ନିବେଶ, &ds_type) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_ନିବେଶ_element_get_name(ନିବେଶ, &ds_name) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (ds_name != ଖାଲି)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ନିବେଶ, GADGET_PASSWORD);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ନିବେଶ, GADGET_FILE);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ନିବେଶ, GADGET_HIDDEN);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ନିବେଶ, GADGET_CHECKBOX);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ନିବେଶ, GADGET_RADIO);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ନିବେଶ, GADGET_SUBMIT);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ନିବେଶ, GADGET_RESET);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ନିବେଶ, GADGET_BUTTON);
	} ଅନ୍ୟଥା ଯଦି (ds_type != ଖାଲି && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ନିବେଶ, GADGET_IMAGE);
	} ଅନ୍ୟଥା {
		control = form_new_control(ନିବେଶ, GADGET_TEXTBOX);
	}

	ଯଦି (control == ଖାଲି)
		ଯାଅ out;

	ଯଦି (name != ଖାଲି) {
		/* Hand the name string over */
		control->name = name;
		name = ଖାଲି;
	}

	ଯଦି (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ଯଦି (dom_html_ନିବେଶ_element_get_checked(
			    ନିବେଶ, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ଯଦି (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ଯଦି (dom_html_ନିବେଶ_element_get_max_length(
			    ନିବେଶ, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ଯଦି (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ଅନ୍ୟଥା {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ଯଦି (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ଯଦି (dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(
			    ନିବେଶ, &ds_ମୂଲ୍ୟ) == DOM_NO_ERR) {
			ଯଦି (ds_ମୂଲ୍ୟ != ଖାଲି) {
				control->ମୂଲ୍ୟ = strndup(
					dom_string_data(ds_ମୂଲ୍ୟ),
					dom_string_byte_length(ds_ମୂଲ୍ୟ));
				ଯଦି (control->ମୂଲ୍ୟ == ଖାଲି) {
					form_free_control(control);
					control = ଖାଲି;
					ଯାଅ out;
				}
				control->length = strlen(control->ମୂଲ୍ୟ);
			}
		}

		ଯଦି (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ଯଦି (control->ମୂଲ୍ୟ == ଖାଲି) {
				control->ମୂଲ୍ୟ = strdup("");
				ଯଦି (control->ମୂଲ୍ୟ == ଖାଲି) {
					form_free_control(control);
					control = ଖାଲି;
					ଯାଅ out;
				}

				control->length = 0;
			}

			control->initial_ମୂଲ୍ୟ = strdup(control->ମୂଲ୍ୟ);
			ଯଦି (control->initial_ମୂଲ୍ୟ == ଖାଲି) {
				form_free_control(control);
				control = ଖାଲି;
				ଯାଅ out;
			}

			control->last_synced_ମୂଲ୍ୟ = strdup(control->ମୂଲ୍ୟ);
			ଯଦି (control->last_synced_ମୂଲ୍ୟ == ଖାଲି) {
				form_free_control(control);
				control = ଖାଲି;
				ଯାଅ out;
			}

			control->ଗଣ୍ଠି_ମୂଲ୍ୟ = dom_string_ref(ds_ମୂଲ୍ୟ);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ଯଦି (form != ଖାଲି && control != ଖାଲି)
		form_add_control(find_form(forms, form), control);

out:
	ଯଦି (form != ଖାଲି)
		dom_ଗଣ୍ଠି_unref(form);
	ଯଦି (ds_type != ଖାଲି)
		dom_string_unref(ds_type);
	ଯଦି (ds_name != ଖାଲି)
		dom_string_unref(ds_name);
	ଯଦି (ds_ମୂଲ୍ୟ != ଖାଲି)
		dom_string_unref(ds_ମୂଲ୍ୟ);

	ଯଦି (name != ଖାଲି)
		ମୁକ୍ତ(name);

	ଫେରନ୍ତୁ control;
}

ସ୍ଥିର ସଂରଚନା form_control *
parse_textarea_element(ସଂରଚନା form *forms, dom_html_text_area_element *ta)
{
	ସଂରଚନା form_control *control = ଖାଲି;
	dom_html_form_element *form = ଖାଲି;
	dom_string *ds_name = ଖାଲି;

	ଅକ୍ଷର *name = ଖାଲି;

	ଯଦି (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (ds_name != ଖାଲି)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ଯଦି (control == ଖାଲି)
		ଯାଅ out;

	ଯଦି (name != ଖାଲି) {
		/* Hand the name string over */
		control->name = name;
		name = ଖାଲି;
	}

	ଯଦି (form != ଖାଲି && control != ଖାଲି)
		form_add_control(find_form(forms, form), control);

out:
	ଯଦି (form != ଖାଲି)
		dom_ଗଣ୍ଠି_unref(form);
	ଯଦି (ds_name != ଖାଲି)
		dom_string_unref(ds_name);

	ଯଦି (name != ଖାଲି)
		ମୁକ୍ତ(name);


	ଫେରନ୍ତୁ control;
}

ସ୍ଥିର ସଂରଚନା form_control *
parse_select_element(ସଂରଚନା form *forms, dom_html_select_element *select)
{
	ସଂରଚନା form_control *control = ଖାଲି;
	dom_html_form_element *form = ଖାଲି;
	dom_string *ds_name = ଖାଲି;

	ଅକ୍ଷର *name = ଖାଲି;

	ଯଦି (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		ଯାଅ out;

	ଯଦି (ds_name != ଖାଲି)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ଯଦି (control == ଖାଲି)
		ଯାଅ out;

	ଯଦି (name != ଖାଲି) {
		/* Hand the name string over */
		control->name = name;
		name = ଖାଲି;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	ଯଦି (form != ଖାଲି && control != ଖାଲି)
		form_add_control(find_form(forms, form), control);

out:
	ଯଦି (form != ଖାଲି)
		dom_ଗଣ୍ଠି_unref(form);
	ଯଦି (ds_name != ଖାଲି)
		dom_string_unref(ds_name);

	ଯଦି (name != ଖାଲି)
		ମୁକ୍ତ(name);


	ଫେରନ୍ତୁ control;
}


ସ୍ଥିର ସଂରଚନା form_control *
invent_fake_gadget(dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା form_control *ctl = form_new_control(ଗଣ୍ଠି, GADGET_HIDDEN);
	ଯଦି (ctl != ଖାଲି) {
		ctl->ମୂଲ୍ୟ = strdup("");
		ctl->initial_ମୂଲ୍ୟ = strdup("");
		ctl->name = strdup("foo");

		ଯଦି (ctl->ମୂଲ୍ୟ == ଖାଲି || ctl->initial_ମୂଲ୍ୟ == ଖାଲି ||
				ctl->name == ଖାଲି) {
			form_free_control(ctl);
			ctl = ଖାଲି;
		}
	}
	ଫେରନ୍ତୁ ctl;
}

/* documented in html_internal.h */
ସଂରଚନା form_control *
html_forms_get_control_for_ଗଣ୍ଠି(ସଂରଚନା form *forms, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା form *f;
	ସଂରଚନା form_control *ctl = ଖାଲି;
	dom_exception err;
	dom_string *ds_name = ଖାଲି;

	/* Step one, see if we already have a control */
	ପାଇଁ (f = forms; f != ଖାଲି; f = f->prev) {
		ପାଇଁ (ctl = f->controls; ctl != ଖାଲି; ctl = ctl->next) {
			ଯଦି (ctl->ଗଣ୍ଠି == ଗଣ୍ଠି)
				ଫେରନ୍ତୁ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(ଗଣ୍ଠି, &ds_name);
	ଯଦି (err == DOM_NO_ERR && ds_name != ଖାଲି) {

		/* Step three, attempt to work out what gadget to make */
		ଯଦି (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ଗଣ୍ଠି);
		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ନିବେଶ)) {
			ctl = parse_ନିବେଶ_element(forms,
					(dom_html_ନିବେଶ_element *) ଗଣ୍ଠି);
		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) ଗଣ୍ଠି);
		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ଗଣ୍ଠି);
		}
	}

	/* If all else fails, fake gadget time */
	ଯଦି (ctl == ଖାଲି)
		ctl = invent_fake_gadget(ଗଣ୍ଠି);

	ଯଦି (ds_name != ଖାଲି)
		dom_string_unref(ds_name);

	ଫେରନ୍ତୁ ctl;
}
