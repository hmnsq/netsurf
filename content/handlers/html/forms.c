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

#شامل <string.h>

#شامل "utils/config.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"

#شامل "html/form_internal.h"
#شامل "html/private.h"

/**
 * process form element from dom
 */
ایستا ساختار form *
parse_form_element(ثابت نویسه *docenc, dom_گره *گره)
{
	dom_string *ds_action = پوچ;
	dom_string *ds_charset = پوچ;
	dom_string *ds_target = پوچ;
	dom_string *ds_method = پوچ;
	dom_string *ds_enctype = پوچ;
	نویسه *action = پوچ, *charset = پوچ, *target = پوچ;
	form_method method;
	dom_html_form_element *formele = (dom_html_form_element *)(گره);
	ساختار form * ret = پوچ;

	/* Retrieve the attributes from the node */
	اگر (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_form_element_get_target(formele,
			&ds_target) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_form_element_get_method(formele,
			&ds_method) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		برو out;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	اگر (ds_action != پوچ)
		action = strndup(dom_string_data(ds_action),
				 dom_string_byte_length(ds_action));

	اگر (ds_charset != پوچ)
		charset = strndup(dom_string_data(ds_charset),
				  dom_string_byte_length(ds_charset));

	اگر (ds_target != پوچ)
		target = strndup(dom_string_data(ds_target),
				 dom_string_byte_length(ds_target));

	/* Determine the method */
	method = method_GET;
	اگر (ds_method != پوچ) {
		اگر (dom_string_caseless_lwc_isequal(ds_method,
				corestring_lwc_post)) {
			method = method_POST_URLENC;
			اگر (ds_enctype != پوچ) {
				اگر (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_data)) {

					method = method_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(گره, action, target, method, charset, docenc);

out:
	اگر (ds_action != پوچ)
		dom_string_unref(ds_action);
	اگر (ds_charset != پوچ)
		dom_string_unref(ds_charset);
	اگر (ds_target != پوچ)
		dom_string_unref(ds_target);
	اگر (ds_method != پوچ)
		dom_string_unref(ds_method);
	اگر (ds_enctype != پوچ)
		dom_string_unref(ds_enctype);
	اگر (action != پوچ)
		آزاد(action);
	اگر (charset != پوچ)
		آزاد(charset);
	اگر (target != پوچ)
		آزاد(target);
	بازگشت ret;
}

/* documented in html_internal.h */
ساختار form *html_forms_get_forms(ثابت نویسه *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ساختار form *ret = پوچ, *newf;
	dom_گره *گره;
	unsigned long n;
	uint32_t nforms;

	اگر (doc == پوچ)
		بازگشت پوچ;

	/* Attempt to build a set of all the forms */
	اگر (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		بازگشت پوچ;

	/* Count the number of forms so we can iterate */
	اگر (dom_html_collection_get_length(forms, &nforms) != DOM_NO_ERR)
		برو out;

	/* Iterate the forms collection, making form structs for returning */
	برای (n = 0; n < nforms; ++n) {
		اگر (dom_html_collection_item(forms, n, &گره) != DOM_NO_ERR) {
			برو out;
		}
		newf = parse_form_element(docenc, گره);
		dom_گره_unref(گره);
		اگر (newf == پوچ) {
			برو err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	برو out;
err:
	وقتی (ret != پوچ) {
		ساختار form *prev = ret->prev;
		/* Destroy ret */
		آزاد(ret);
		ret = prev;
	}
out:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	بازگشت ret;
}

ایستا ساختار form *
find_form(ساختار form *forms, dom_html_form_element *form)
{
	وقتی (forms != پوچ) {
		اگر (forms->گره == form)
			بشکن;
		forms = forms->prev;
	}

	بازگشت forms;
}

ایستا ساختار form_control *
parse_button_element(ساختار form *forms, dom_html_button_element *button)
{
	ساختار form_control *control = پوچ;
	dom_exception err;
	dom_html_form_element *form = پوچ;
	dom_string *ds_type = پوچ;
	dom_string *ds_مقدار = پوچ;
	dom_string *ds_name = پوچ;

	err = dom_html_button_element_get_form(button, &form);
	اگر (err != DOM_NO_ERR)
		برو out;

	err = dom_html_button_element_get_type(button, &ds_type);
	اگر (err != DOM_NO_ERR)
		برو out;

	اگر (ds_type == پوچ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} وگرنه {
		اگر (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} وگرنه اگر (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} وگرنه {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	اگر (control == پوچ)
		برو out;

	err = dom_html_button_element_get_مقدار(button, &ds_مقدار);
	اگر (err != DOM_NO_ERR)
		برو out;
	err = dom_html_button_element_get_name(button, &ds_name);
	اگر (err != DOM_NO_ERR)
		برو out;

	اگر (ds_مقدار != پوچ) {
		control->مقدار = strndup(
			dom_string_data(ds_مقدار),
			dom_string_byte_length(ds_مقدار));

		اگر (control->مقدار == پوچ) {
			form_free_control(control);
			control = پوچ;
			برو out;
		}
	}

	اگر (ds_name != پوچ) {
		control->name = strndup(
			dom_string_data(ds_name),
			dom_string_byte_length(ds_name));

		اگر (control->name == پوچ) {
			form_free_control(control);
			control = پوچ;
			برو out;
		}
	}

	اگر (form != پوچ && control != پوچ)
		form_add_control(find_form(forms, form), control);

out:
	اگر (form != پوچ)
		dom_گره_unref(form);
	اگر (ds_type != پوچ)
		dom_string_unref(ds_type);
	اگر (ds_مقدار != پوچ)
		dom_string_unref(ds_مقدار);
	اگر (ds_name != پوچ)
		dom_string_unref(ds_name);

	بازگشت control;
}

ایستا ساختار form_control *
parse_درون_داد_element(ساختار form *forms, dom_html_درون_داد_element *درون_داد)
{
	ساختار form_control *control = پوچ;
	dom_html_form_element *form = پوچ;
	dom_string *ds_type = پوچ;
	dom_string *ds_name = پوچ;
	dom_string *ds_مقدار = پوچ;

	نویسه *name = پوچ;

	اگر (dom_html_درون_داد_element_get_form(درون_داد, &form) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_درون_داد_element_get_type(درون_داد, &ds_type) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_درون_داد_element_get_name(درون_داد, &ds_name) != DOM_NO_ERR)
		برو out;

	اگر (ds_name != پوچ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(درون_داد, GADGET_PASSWORD);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(درون_داد, GADGET_FILE);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(درون_داد, GADGET_HIDDEN);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(درون_داد, GADGET_CHECKBOX);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(درون_داد, GADGET_RADIO);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(درون_داد, GADGET_SUBMIT);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(درون_داد, GADGET_RESET);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(درون_داد, GADGET_BUTTON);
	} وگرنه اگر (ds_type != پوچ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(درون_داد, GADGET_IMAGE);
	} وگرنه {
		control = form_new_control(درون_داد, GADGET_TEXTBOX);
	}

	اگر (control == پوچ)
		برو out;

	اگر (name != پوچ) {
		/* Hand the name string over */
		control->name = name;
		name = پوچ;
	}

	اگر (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		اگر (dom_html_درون_داد_element_get_checked(
			    درون_داد, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	اگر (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		اگر (dom_html_درون_داد_element_get_max_length(
			    درون_داد, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		اگر (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} وگرنه {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	اگر (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		اگر (dom_html_درون_داد_element_get_مقدار(
			    درون_داد, &ds_مقدار) == DOM_NO_ERR) {
			اگر (ds_مقدار != پوچ) {
				control->مقدار = strndup(
					dom_string_data(ds_مقدار),
					dom_string_byte_length(ds_مقدار));
				اگر (control->مقدار == پوچ) {
					form_free_control(control);
					control = پوچ;
					برو out;
				}
				control->length = strlen(control->مقدار);
			}
		}

		اگر (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			اگر (control->مقدار == پوچ) {
				control->مقدار = strdup("");
				اگر (control->مقدار == پوچ) {
					form_free_control(control);
					control = پوچ;
					برو out;
				}

				control->length = 0;
			}

			control->initial_مقدار = strdup(control->مقدار);
			اگر (control->initial_مقدار == پوچ) {
				form_free_control(control);
				control = پوچ;
				برو out;
			}

			control->last_synced_مقدار = strdup(control->مقدار);
			اگر (control->last_synced_مقدار == پوچ) {
				form_free_control(control);
				control = پوچ;
				برو out;
			}

			control->گره_مقدار = dom_string_ref(ds_مقدار);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	اگر (form != پوچ && control != پوچ)
		form_add_control(find_form(forms, form), control);

out:
	اگر (form != پوچ)
		dom_گره_unref(form);
	اگر (ds_type != پوچ)
		dom_string_unref(ds_type);
	اگر (ds_name != پوچ)
		dom_string_unref(ds_name);
	اگر (ds_مقدار != پوچ)
		dom_string_unref(ds_مقدار);

	اگر (name != پوچ)
		آزاد(name);

	بازگشت control;
}

ایستا ساختار form_control *
parse_textarea_element(ساختار form *forms, dom_html_text_area_element *ta)
{
	ساختار form_control *control = پوچ;
	dom_html_form_element *form = پوچ;
	dom_string *ds_name = پوچ;

	نویسه *name = پوچ;

	اگر (dom_html_text_area_element_get_form(ta, &form) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_text_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		برو out;

	اگر (ds_name != پوچ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	اگر (control == پوچ)
		برو out;

	اگر (name != پوچ) {
		/* Hand the name string over */
		control->name = name;
		name = پوچ;
	}

	اگر (form != پوچ && control != پوچ)
		form_add_control(find_form(forms, form), control);

out:
	اگر (form != پوچ)
		dom_گره_unref(form);
	اگر (ds_name != پوچ)
		dom_string_unref(ds_name);

	اگر (name != پوچ)
		آزاد(name);


	بازگشت control;
}

ایستا ساختار form_control *
parse_select_element(ساختار form *forms, dom_html_select_element *select)
{
	ساختار form_control *control = پوچ;
	dom_html_form_element *form = پوچ;
	dom_string *ds_name = پوچ;

	نویسه *name = پوچ;

	اگر (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		برو out;

	اگر (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		برو out;

	اگر (ds_name != پوچ)
		name = strndup(dom_string_data(ds_name),
			       dom_string_byte_length(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	اگر (control == پوچ)
		برو out;

	اگر (name != پوچ) {
		/* Hand the name string over */
		control->name = name;
		name = پوچ;
	}

	dom_html_select_element_get_multiple(select,
			&(control->data.select.multiple));

	اگر (form != پوچ && control != پوچ)
		form_add_control(find_form(forms, form), control);

out:
	اگر (form != پوچ)
		dom_گره_unref(form);
	اگر (ds_name != پوچ)
		dom_string_unref(ds_name);

	اگر (name != پوچ)
		آزاد(name);


	بازگشت control;
}


ایستا ساختار form_control *
invent_fake_gadget(dom_گره *گره)
{
	ساختار form_control *ctl = form_new_control(گره, GADGET_HIDDEN);
	اگر (ctl != پوچ) {
		ctl->مقدار = strdup("");
		ctl->initial_مقدار = strdup("");
		ctl->name = strdup("foo");

		اگر (ctl->مقدار == پوچ || ctl->initial_مقدار == پوچ ||
				ctl->name == پوچ) {
			form_free_control(ctl);
			ctl = پوچ;
		}
	}
	بازگشت ctl;
}

/* documented in html_internal.h */
ساختار form_control *
html_forms_get_control_for_گره(ساختار form *forms, dom_گره *گره)
{
	ساختار form *f;
	ساختار form_control *ctl = پوچ;
	dom_exception err;
	dom_string *ds_name = پوچ;

	/* Step one, see if we already have a control */
	برای (f = forms; f != پوچ; f = f->prev) {
		برای (ctl = f->controls; ctl != پوچ; ctl = ctl->next) {
			اگر (ctl->گره == گره)
				بازگشت ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_tag_name(گره, &ds_name);
	اگر (err == DOM_NO_ERR && ds_name != پوچ) {

		/* Step three, attempt to work out what gadget to make */
		اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) گره);
		} وگرنه اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_درون_داد)) {
			ctl = parse_درون_داد_element(forms,
					(dom_html_درون_داد_element *) گره);
		} وگرنه اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_text_area_element *) گره);
		} وگرنه اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) گره);
		}
	}

	/* If all else fails, fake gadget time */
	اگر (ctl == پوچ)
		ctl = invent_fake_gadget(گره);

	اگر (ds_name != پوچ)
		dom_string_unref(ds_name);

	بازگشت ctl;
}
