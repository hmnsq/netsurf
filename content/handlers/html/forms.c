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
parse_form_element(ثابت حرف *docenc, dom_گره *گره)
{
	dom_string *ds_action = عدم;
	dom_string *ds_charset = عدم;
	dom_string *ds_هدف = عدم;
	dom_string *ds_روش = عدم;
	dom_string *ds_enctype = عدم;
	حرف *action = عدم, *charset = عدم, *هدف = عدم;
	form_روش روش;
	dom_html_form_element *formele = (dom_html_form_element *)(گره);
	بنية form * ret = عدم;

	/* Retrieve the attributes from the node */
	إذا (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_form_element_get_هدف(formele,
			&ds_هدف) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_form_element_get_روش(formele,
			&ds_روش) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		اذهب بیرون;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	إذا (ds_action != عدم)
		action = strndup(dom_string_داده(ds_action),
				 dom_string_byte_درازی(ds_action));

	إذا (ds_charset != عدم)
		charset = strndup(dom_string_داده(ds_charset),
				  dom_string_byte_درازی(ds_charset));

	إذا (ds_هدف != عدم)
		هدف = strndup(dom_string_داده(ds_هدف),
				 dom_string_byte_درازی(ds_هدف));

	/* Determine the method */
	روش = روش_GET;
	إذا (ds_روش != عدم) {
		إذا (dom_string_caseless_lwc_isequal(ds_روش,
				corestring_lwc_post)) {
			روش = روش_POST_URLENC;
			إذا (ds_enctype != عدم) {
				إذا (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_داده)) {

					روش = روش_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(گره, action, هدف, روش, charset, docenc);

بیرون:
	إذا (ds_action != عدم)
		dom_string_unref(ds_action);
	إذا (ds_charset != عدم)
		dom_string_unref(ds_charset);
	إذا (ds_هدف != عدم)
		dom_string_unref(ds_هدف);
	إذا (ds_روش != عدم)
		dom_string_unref(ds_روش);
	إذا (ds_enctype != عدم)
		dom_string_unref(ds_enctype);
	إذا (action != عدم)
		حرر(action);
	إذا (charset != عدم)
		حرر(charset);
	إذا (هدف != عدم)
		حرر(هدف);
	أرجع ret;
}

/* documented in html_internal.h */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	بنية form *ret = عدم, *newf;
	dom_گره *گره;
	بیعلامت دراز n;
	uint32_t nforms;

	إذا (doc == عدم)
		أرجع عدم;

	/* Attempt to build a set of all the forms */
	إذا (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		أرجع عدم;

	/* Count the number of forms so we can iterate */
	إذا (dom_html_collection_get_درازی(forms, &nforms) != DOM_NO_ERR)
		اذهب بیرون;

	/* Iterate the forms collection, making form structs for returning */
	لكل (n = 0; n < nforms; ++n) {
		إذا (dom_html_collection_قلم(forms, n, &گره) != DOM_NO_ERR) {
			اذهب بیرون;
		}
		newf = parse_form_element(docenc, گره);
		dom_گره_unref(گره);
		إذا (newf == عدم) {
			اذهب err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	اذهب بیرون;
err:
	طالما (ret != عدم) {
		بنية form *prev = ret->prev;
		/* Destroy ret */
		حرر(ret);
		ret = prev;
	}
بیرون:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	أرجع ret;
}

ساكن بنية form *
پیدا_کن_form(بنية form *forms, dom_html_form_element *form)
{
	طالما (forms != عدم) {
		إذا (forms->گره == form)
			اكسر;
		forms = forms->prev;
	}

	أرجع forms;
}

ساكن بنية form_control *
parse_button_element(بنية form *forms, dom_html_button_element *button)
{
	بنية form_control *control = عدم;
	dom_exception err;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_مقدار = عدم;
	dom_string *ds_name = عدم;

	err = dom_html_button_element_get_form(button, &form);
	إذا (err != DOM_NO_ERR)
		اذهب بیرون;

	err = dom_html_button_element_get_type(button, &ds_type);
	إذا (err != DOM_NO_ERR)
		اذهب بیرون;

	إذا (ds_type == عدم) {
		control = form_new_control(button, GADGET_SUBMIT);
	} وإلا {
		إذا (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} وإلا {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	إذا (control == عدم)
		اذهب بیرون;

	err = dom_html_button_element_get_مقدار(button, &ds_مقدار);
	إذا (err != DOM_NO_ERR)
		اذهب بیرون;
	err = dom_html_button_element_get_name(button, &ds_name);
	إذا (err != DOM_NO_ERR)
		اذهب بیرون;

	إذا (ds_مقدار != عدم) {
		control->مقدار = strndup(
			dom_string_داده(ds_مقدار),
			dom_string_byte_درازی(ds_مقدار));

		إذا (control->مقدار == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب بیرون;
		}
	}

	إذا (ds_name != عدم) {
		control->name = strndup(
			dom_string_داده(ds_name),
			dom_string_byte_درازی(ds_name));

		إذا (control->name == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب بیرون;
		}
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کن_control(پیدا_کن_form(forms, form), control);

بیرون:
	إذا (form != عدم)
		dom_گره_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_مقدار != عدم)
		dom_string_unref(ds_مقدار);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع control;
}

ساكن بنية form_control *
parse_درون_داد_element(بنية form *forms, dom_html_درون_داد_element *درون_داد)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_name = عدم;
	dom_string *ds_مقدار = عدم;

	حرف *name = عدم;

	إذا (dom_html_درون_داد_element_get_form(درون_داد, &form) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_درون_داد_element_get_type(درون_داد, &ds_type) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_درون_داد_element_get_name(درون_داد, &ds_name) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (ds_name != عدم)
		name = strndup(dom_string_داده(ds_name),
			       dom_string_byte_درازی(ds_name));

	إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(درون_داد, GADGET_PASSWORD);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(درون_داد, GADGET_FILE);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(درون_داد, GADGET_HIDDEN);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(درون_داد, GADGET_CHECKBOX);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(درون_داد, GADGET_RADIO);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(درون_داد, GADGET_SUBMIT);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(درون_داد, GADGET_RESET);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(درون_داد, GADGET_BUTTON);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(درون_داد, GADGET_IMAGE);
	} وإلا {
		control = form_new_control(درون_داد, GADGET_TEXTBOX);
	}

	إذا (control == عدم)
		اذهب بیرون;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		إذا (dom_html_درون_داد_element_get_checked(
			    درون_داد, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	إذا (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		إذا (dom_html_درون_داد_element_get_max_درازی(
			    درون_داد, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		إذا (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} وإلا {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	إذا (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		إذا (dom_html_درون_داد_element_get_مقدار(
			    درون_داد, &ds_مقدار) == DOM_NO_ERR) {
			إذا (ds_مقدار != عدم) {
				control->مقدار = strndup(
					dom_string_داده(ds_مقدار),
					dom_string_byte_درازی(ds_مقدار));
				إذا (control->مقدار == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب بیرون;
				}
				control->درازی = طول_رشته(control->مقدار);
			}
		}

		إذا (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			إذا (control->مقدار == عدم) {
				control->مقدار = strdup("");
				إذا (control->مقدار == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب بیرون;
				}

				control->درازی = 0;
			}

			control->initial_مقدار = strdup(control->مقدار);
			إذا (control->initial_مقدار == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب بیرون;
			}

			control->آخرین_synced_مقدار = strdup(control->مقدار);
			إذا (control->آخرین_synced_مقدار == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب بیرون;
			}

			control->گره_مقدار = dom_string_ref(ds_مقدار);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کن_control(پیدا_کن_form(forms, form), control);

بیرون:
	إذا (form != عدم)
		dom_گره_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);
	إذا (ds_مقدار != عدم)
		dom_string_unref(ds_مقدار);

	إذا (name != عدم)
		حرر(name);

	أرجع control;
}

ساكن بنية form_control *
parse_textarea_element(بنية form *forms, dom_html_متن_area_element *ta)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	إذا (dom_html_متن_area_element_get_form(ta, &form) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_متن_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (ds_name != عدم)
		name = strndup(dom_string_داده(ds_name),
			       dom_string_byte_درازی(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	إذا (control == عدم)
		اذهب بیرون;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کن_control(پیدا_کن_form(forms, form), control);

بیرون:
	إذا (form != عدم)
		dom_گره_unref(form);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	إذا (name != عدم)
		حرر(name);


	أرجع control;
}

ساكن بنية form_control *
parse_select_element(بنية form *forms, dom_html_select_element *select)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_name = عدم;

	حرف *name = عدم;

	إذا (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		اذهب بیرون;

	إذا (ds_name != عدم)
		name = strndup(dom_string_داده(ds_name),
			       dom_string_byte_درازی(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	إذا (control == عدم)
		اذهب بیرون;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	dom_html_select_element_get_مضرب(select,
			&(control->داده.select.مضرب));

	إذا (form != عدم && control != عدم)
		form_جمع_کن_control(پیدا_کن_form(forms, form), control);

بیرون:
	إذا (form != عدم)
		dom_گره_unref(form);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	إذا (name != عدم)
		حرر(name);


	أرجع control;
}


ساكن بنية form_control *
invent_fake_gadget(dom_گره *گره)
{
	بنية form_control *ctl = form_new_control(گره, GADGET_HIDDEN);
	إذا (ctl != عدم) {
		ctl->مقدار = strdup("");
		ctl->initial_مقدار = strdup("");
		ctl->name = strdup("foo");

		إذا (ctl->مقدار == عدم || ctl->initial_مقدار == عدم ||
				ctl->name == عدم) {
			form_free_control(ctl);
			ctl = عدم;
		}
	}
	أرجع ctl;
}

/* documented in html_internal.h */
بنية form_control *
html_forms_get_control_for_گره(بنية form *forms, dom_گره *گره)
{
	بنية form *f;
	بنية form_control *ctl = عدم;
	dom_exception err;
	dom_string *ds_name = عدم;

	/* Step one, see if we already have a control */
	لكل (f = forms; f != عدم; f = f->prev) {
		لكل (ctl = f->controls; ctl != عدم; ctl = ctl->بعدی) {
			إذا (ctl->گره == گره)
				أرجع ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_برچسب_name(گره, &ds_name);
	إذا (err == DOM_NO_ERR && ds_name != عدم) {

		/* Step three, attempt to work out what gadget to make */
		إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) گره);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_درون_داد)) {
			ctl = parse_درون_داد_element(forms,
					(dom_html_درون_داد_element *) گره);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_متن_area_element *) گره);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) گره);
		}
	}

	/* If all else fails, fake gadget time */
	إذا (ctl == عدم)
		ctl = invent_fake_gadget(گره);

	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع ctl;
}
