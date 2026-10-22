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
ساکن ساخت form *
parse_form_element(ثابت حرف *docenc, dom_گرہ *گرہ)
{
	dom_string *ds_action = تہی;
	dom_string *ds_charset = تہی;
	dom_string *ds_ہدف = تہی;
	dom_string *ds_طریقہ = تہی;
	dom_string *ds_enctype = تہی;
	حرف *action = تہی, *charset = تہی, *ہدف = تہی;
	form_طریقہ طریقہ;
	dom_html_form_element *formele = (dom_html_form_element *)(گرہ);
	ساخت form * ret = تہی;

	/* Retrieve the attributes from the node */
	اگر (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_form_element_get_ہدف(formele,
			&ds_ہدف) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_form_element_get_طریقہ(formele,
			&ds_طریقہ) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		جاؤ برآمد;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	اگر (ds_action != تہی)
		action = strndup(dom_string_ڈیٹا(ds_action),
				 dom_string_byte_لمبائی(ds_action));

	اگر (ds_charset != تہی)
		charset = strndup(dom_string_ڈیٹا(ds_charset),
				  dom_string_byte_لمبائی(ds_charset));

	اگر (ds_ہدف != تہی)
		ہدف = strndup(dom_string_ڈیٹا(ds_ہدف),
				 dom_string_byte_لمبائی(ds_ہدف));

	/* Determine the method */
	طریقہ = طریقہ_GET;
	اگر (ds_طریقہ != تہی) {
		اگر (dom_string_caseless_lwc_isequal(ds_طریقہ,
				corestring_lwc_post)) {
			طریقہ = طریقہ_POST_URLENC;
			اگر (ds_enctype != تہی) {
				اگر (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_ڈیٹا)) {

					طریقہ = طریقہ_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(گرہ, action, ہدف, طریقہ, charset, docenc);

برآمد:
	اگر (ds_action != تہی)
		dom_string_unref(ds_action);
	اگر (ds_charset != تہی)
		dom_string_unref(ds_charset);
	اگر (ds_ہدف != تہی)
		dom_string_unref(ds_ہدف);
	اگر (ds_طریقہ != تہی)
		dom_string_unref(ds_طریقہ);
	اگر (ds_enctype != تہی)
		dom_string_unref(ds_enctype);
	اگر (action != تہی)
		آزاد(action);
	اگر (charset != تہی)
		آزاد(charset);
	اگر (ہدف != تہی)
		آزاد(ہدف);
	واپس ret;
}

/* documented in html_internal.h */
ساخت form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ساخت form *ret = تہی, *newf;
	dom_گرہ *گرہ;
	غیرعلامتی طویل n;
	uint32_t nforms;

	اگر (doc == تہی)
		واپس تہی;

	/* Attempt to build a set of all the forms */
	اگر (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		واپس تہی;

	/* Count the number of forms so we can iterate */
	اگر (dom_html_collection_get_لمبائی(forms, &nforms) != DOM_NO_ERR)
		جاؤ برآمد;

	/* Iterate the forms collection, making form structs for returning */
	برائے (n = 0; n < nforms; ++n) {
		اگر (dom_html_collection_شے(forms, n, &گرہ) != DOM_NO_ERR) {
			جاؤ برآمد;
		}
		newf = parse_form_element(docenc, گرہ);
		dom_گرہ_unref(گرہ);
		اگر (newf == تہی) {
			جاؤ err;
		}
		newf->پچھلا = ret;
		ret = newf;
	}

	/* All went well */
	جاؤ برآمد;
err:
	جب (ret != تہی) {
		ساخت form *پچھلا = ret->پچھلا;
		/* Destroy ret */
		آزاد(ret);
		ret = پچھلا;
	}
برآمد:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	واپس ret;
}

ساکن ساخت form *
ڈھونڈو_form(ساخت form *forms, dom_html_form_element *form)
{
	جب (forms != تہی) {
		اگر (forms->گرہ == form)
			توڑو;
		forms = forms->پچھلا;
	}

	واپس forms;
}

ساکن ساخت form_control *
parse_button_element(ساخت form *forms, dom_html_button_element *button)
{
	ساخت form_control *control = تہی;
	dom_exception err;
	dom_html_form_element *form = تہی;
	dom_string *ds_type = تہی;
	dom_string *ds_قدر = تہی;
	dom_string *ds_name = تہی;

	err = dom_html_button_element_get_form(button, &form);
	اگر (err != DOM_NO_ERR)
		جاؤ برآمد;

	err = dom_html_button_element_get_type(button, &ds_type);
	اگر (err != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (ds_type == تہی) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ورنہ {
		اگر (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ورنہ اگر (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ورنہ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	اگر (control == تہی)
		جاؤ برآمد;

	err = dom_html_button_element_get_قدر(button, &ds_قدر);
	اگر (err != DOM_NO_ERR)
		جاؤ برآمد;
	err = dom_html_button_element_get_name(button, &ds_name);
	اگر (err != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (ds_قدر != تہی) {
		control->قدر = strndup(
			dom_string_ڈیٹا(ds_قدر),
			dom_string_byte_لمبائی(ds_قدر));

		اگر (control->قدر == تہی) {
			form_free_control(control);
			control = تہی;
			جاؤ برآمد;
		}
	}

	اگر (ds_name != تہی) {
		control->name = strndup(
			dom_string_ڈیٹا(ds_name),
			dom_string_byte_لمبائی(ds_name));

		اگر (control->name == تہی) {
			form_free_control(control);
			control = تہی;
			جاؤ برآمد;
		}
	}

	اگر (form != تہی && control != تہی)
		form_جوڑو_control(ڈھونڈو_form(forms, form), control);

برآمد:
	اگر (form != تہی)
		dom_گرہ_unref(form);
	اگر (ds_type != تہی)
		dom_string_unref(ds_type);
	اگر (ds_قدر != تہی)
		dom_string_unref(ds_قدر);
	اگر (ds_name != تہی)
		dom_string_unref(ds_name);

	واپس control;
}

ساکن ساخت form_control *
parse_داخلہ_element(ساخت form *forms, dom_html_داخلہ_element *داخلہ)
{
	ساخت form_control *control = تہی;
	dom_html_form_element *form = تہی;
	dom_string *ds_type = تہی;
	dom_string *ds_name = تہی;
	dom_string *ds_قدر = تہی;

	حرف *name = تہی;

	اگر (dom_html_داخلہ_element_get_form(داخلہ, &form) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_داخلہ_element_get_type(داخلہ, &ds_type) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_داخلہ_element_get_name(داخلہ, &ds_name) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (ds_name != تہی)
		name = strndup(dom_string_ڈیٹا(ds_name),
			       dom_string_byte_لمبائی(ds_name));

	اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(داخلہ, GADGET_PASSWORD);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(داخلہ, GADGET_FILE);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(داخلہ, GADGET_HIDDEN);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(داخلہ, GADGET_CHECKBOX);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(داخلہ, GADGET_RADIO);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(داخلہ, GADGET_SUBMIT);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(داخلہ, GADGET_RESET);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(داخلہ, GADGET_BUTTON);
	} ورنہ اگر (ds_type != تہی && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(داخلہ, GADGET_IMAGE);
	} ورنہ {
		control = form_new_control(داخلہ, GADGET_TEXTBOX);
	}

	اگر (control == تہی)
		جاؤ برآمد;

	اگر (name != تہی) {
		/* Hand the name string over */
		control->name = name;
		name = تہی;
	}

	اگر (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		اگر (dom_html_داخلہ_element_get_checked(
			    داخلہ, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	اگر (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		اگر (dom_html_داخلہ_element_get_max_لمبائی(
			    داخلہ, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		اگر (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ورنہ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	اگر (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		اگر (dom_html_داخلہ_element_get_قدر(
			    داخلہ, &ds_قدر) == DOM_NO_ERR) {
			اگر (ds_قدر != تہی) {
				control->قدر = strndup(
					dom_string_ڈیٹا(ds_قدر),
					dom_string_byte_لمبائی(ds_قدر));
				اگر (control->قدر == تہی) {
					form_free_control(control);
					control = تہی;
					جاؤ برآمد;
				}
				control->لمبائی = لڑی_لمبائی(control->قدر);
			}
		}

		اگر (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			اگر (control->قدر == تہی) {
				control->قدر = strdup("");
				اگر (control->قدر == تہی) {
					form_free_control(control);
					control = تہی;
					جاؤ برآمد;
				}

				control->لمبائی = 0;
			}

			control->initial_قدر = strdup(control->قدر);
			اگر (control->initial_قدر == تہی) {
				form_free_control(control);
				control = تہی;
				جاؤ برآمد;
			}

			control->آخری_synced_قدر = strdup(control->قدر);
			اگر (control->آخری_synced_قدر == تہی) {
				form_free_control(control);
				control = تہی;
				جاؤ برآمد;
			}

			control->گرہ_قدر = dom_string_ref(ds_قدر);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	اگر (form != تہی && control != تہی)
		form_جوڑو_control(ڈھونڈو_form(forms, form), control);

برآمد:
	اگر (form != تہی)
		dom_گرہ_unref(form);
	اگر (ds_type != تہی)
		dom_string_unref(ds_type);
	اگر (ds_name != تہی)
		dom_string_unref(ds_name);
	اگر (ds_قدر != تہی)
		dom_string_unref(ds_قدر);

	اگر (name != تہی)
		آزاد(name);

	واپس control;
}

ساکن ساخت form_control *
parse_textarea_element(ساخت form *forms, dom_html_متن_area_element *ta)
{
	ساخت form_control *control = تہی;
	dom_html_form_element *form = تہی;
	dom_string *ds_name = تہی;

	حرف *name = تہی;

	اگر (dom_html_متن_area_element_get_form(ta, &form) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_متن_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (ds_name != تہی)
		name = strndup(dom_string_ڈیٹا(ds_name),
			       dom_string_byte_لمبائی(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	اگر (control == تہی)
		جاؤ برآمد;

	اگر (name != تہی) {
		/* Hand the name string over */
		control->name = name;
		name = تہی;
	}

	اگر (form != تہی && control != تہی)
		form_جوڑو_control(ڈھونڈو_form(forms, form), control);

برآمد:
	اگر (form != تہی)
		dom_گرہ_unref(form);
	اگر (ds_name != تہی)
		dom_string_unref(ds_name);

	اگر (name != تہی)
		آزاد(name);


	واپس control;
}

ساکن ساخت form_control *
parse_select_element(ساخت form *forms, dom_html_select_element *select)
{
	ساخت form_control *control = تہی;
	dom_html_form_element *form = تہی;
	dom_string *ds_name = تہی;

	حرف *name = تہی;

	اگر (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		جاؤ برآمد;

	اگر (ds_name != تہی)
		name = strndup(dom_string_ڈیٹا(ds_name),
			       dom_string_byte_لمبائی(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	اگر (control == تہی)
		جاؤ برآمد;

	اگر (name != تہی) {
		/* Hand the name string over */
		control->name = name;
		name = تہی;
	}

	dom_html_select_element_get_مضرب(select,
			&(control->ڈیٹا.select.مضرب));

	اگر (form != تہی && control != تہی)
		form_جوڑو_control(ڈھونڈو_form(forms, form), control);

برآمد:
	اگر (form != تہی)
		dom_گرہ_unref(form);
	اگر (ds_name != تہی)
		dom_string_unref(ds_name);

	اگر (name != تہی)
		آزاد(name);


	واپس control;
}


ساکن ساخت form_control *
invent_fake_gadget(dom_گرہ *گرہ)
{
	ساخت form_control *ctl = form_new_control(گرہ, GADGET_HIDDEN);
	اگر (ctl != تہی) {
		ctl->قدر = strdup("");
		ctl->initial_قدر = strdup("");
		ctl->name = strdup("foo");

		اگر (ctl->قدر == تہی || ctl->initial_قدر == تہی ||
				ctl->name == تہی) {
			form_free_control(ctl);
			ctl = تہی;
		}
	}
	واپس ctl;
}

/* documented in html_internal.h */
ساخت form_control *
html_forms_get_control_for_گرہ(ساخت form *forms, dom_گرہ *گرہ)
{
	ساخت form *f;
	ساخت form_control *ctl = تہی;
	dom_exception err;
	dom_string *ds_name = تہی;

	/* Step one, see if we already have a control */
	برائے (f = forms; f != تہی; f = f->پچھلا) {
		برائے (ctl = f->controls; ctl != تہی; ctl = ctl->اگلا) {
			اگر (ctl->گرہ == گرہ)
				واپس ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_لیبل_name(گرہ, &ds_name);
	اگر (err == DOM_NO_ERR && ds_name != تہی) {

		/* Step three, attempt to work out what gadget to make */
		اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) گرہ);
		} ورنہ اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_داخلہ)) {
			ctl = parse_داخلہ_element(forms,
					(dom_html_داخلہ_element *) گرہ);
		} ورنہ اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_متن_area_element *) گرہ);
		} ورنہ اگر (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) گرہ);
		}
	}

	/* If all else fails, fake gadget time */
	اگر (ctl == تہی)
		ctl = invent_fake_gadget(گرہ);

	اگر (ds_name != تہی)
		dom_string_unref(ds_name);

	واپس ctl;
}
