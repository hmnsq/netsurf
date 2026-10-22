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
parse_form_element(ثابت حرف *docenc, dom_گرہ *گرہ)
{
	dom_string *ds_action = عدم;
	dom_string *ds_charset = عدم;
	dom_string *ds_نشانہ = عدم;
	dom_string *ds_طریقہ = عدم;
	dom_string *ds_enctype = عدم;
	حرف *action = عدم, *charset = عدم, *نشانہ = عدم;
	form_طریقہ طریقہ;
	dom_html_form_element *formele = (dom_html_form_element *)(گرہ);
	بنية form * ret = عدم;

	/* Retrieve the attributes from the node */
	إذا (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_form_element_get_نشانہ(formele,
			&ds_نشانہ) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_form_element_get_طریقہ(formele,
			&ds_طریقہ) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		اذهب باہرلا;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	إذا (ds_action != عدم)
		action = strndup(dom_string_انکڑے(ds_action),
				 dom_string_byte_لمائی(ds_action));

	إذا (ds_charset != عدم)
		charset = strndup(dom_string_انکڑے(ds_charset),
				  dom_string_byte_لمائی(ds_charset));

	إذا (ds_نشانہ != عدم)
		نشانہ = strndup(dom_string_انکڑے(ds_نشانہ),
				 dom_string_byte_لمائی(ds_نشانہ));

	/* Determine the method */
	طریقہ = طریقہ_GET;
	إذا (ds_طریقہ != عدم) {
		إذا (dom_string_caseless_lwc_isequal(ds_طریقہ,
				corestring_lwc_post)) {
			طریقہ = طریقہ_POST_URLENC;
			إذا (ds_enctype != عدم) {
				إذا (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_انکڑے)) {

					طریقہ = طریقہ_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(گرہ, action, نشانہ, طریقہ, charset, docenc);

باہرلا:
	إذا (ds_action != عدم)
		dom_string_unref(ds_action);
	إذا (ds_charset != عدم)
		dom_string_unref(ds_charset);
	إذا (ds_نشانہ != عدم)
		dom_string_unref(ds_نشانہ);
	إذا (ds_طریقہ != عدم)
		dom_string_unref(ds_طریقہ);
	إذا (ds_enctype != عدم)
		dom_string_unref(ds_enctype);
	إذا (action != عدم)
		حرر(action);
	إذا (charset != عدم)
		حرر(charset);
	إذا (نشانہ != عدم)
		حرر(نشانہ);
	أرجع ret;
}

/* documented in html_internal.h */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	بنية form *ret = عدم, *newf;
	dom_گرہ *گرہ;
	بنانشانی لما n;
	uint32_t nforms;

	إذا (doc == عدم)
		أرجع عدم;

	/* Attempt to build a set of all the forms */
	إذا (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		أرجع عدم;

	/* Count the number of forms so we can iterate */
	إذا (dom_html_collection_get_لمائی(forms, &nforms) != DOM_NO_ERR)
		اذهب باہرلا;

	/* Iterate the forms collection, making form structs for returning */
	لكل (n = 0; n < nforms; ++n) {
		إذا (dom_html_collection_شے(forms, n, &گرہ) != DOM_NO_ERR) {
			اذهب باہرلا;
		}
		newf = parse_form_element(docenc, گرہ);
		dom_گرہ_unref(گرہ);
		إذا (newf == عدم) {
			اذهب err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	اذهب باہرلا;
err:
	طالما (ret != عدم) {
		بنية form *prev = ret->prev;
		/* Destroy ret */
		حرر(ret);
		ret = prev;
	}
باہرلا:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	أرجع ret;
}

ساكن بنية form *
لبھو_form(بنية form *forms, dom_html_form_element *form)
{
	طالما (forms != عدم) {
		إذا (forms->گرہ == form)
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
	dom_string *ds_قدر = عدم;
	dom_string *ds_name = عدم;

	err = dom_html_button_element_get_form(button, &form);
	إذا (err != DOM_NO_ERR)
		اذهب باہرلا;

	err = dom_html_button_element_get_type(button, &ds_type);
	إذا (err != DOM_NO_ERR)
		اذهب باہرلا;

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
		اذهب باہرلا;

	err = dom_html_button_element_get_قدر(button, &ds_قدر);
	إذا (err != DOM_NO_ERR)
		اذهب باہرلا;
	err = dom_html_button_element_get_name(button, &ds_name);
	إذا (err != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (ds_قدر != عدم) {
		control->قدر = strndup(
			dom_string_انکڑے(ds_قدر),
			dom_string_byte_لمائی(ds_قدر));

		إذا (control->قدر == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب باہرلا;
		}
	}

	إذا (ds_name != عدم) {
		control->name = strndup(
			dom_string_انکڑے(ds_name),
			dom_string_byte_لمائی(ds_name));

		إذا (control->name == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب باہرلا;
		}
	}

	إذا (form != عدم && control != عدم)
		form_جوڑو_control(لبھو_form(forms, form), control);

باہرلا:
	إذا (form != عدم)
		dom_گرہ_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_قدر != عدم)
		dom_string_unref(ds_قدر);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع control;
}

ساكن بنية form_control *
parse_داخلہ_element(بنية form *forms, dom_html_داخلہ_element *داخلہ)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_name = عدم;
	dom_string *ds_قدر = عدم;

	حرف *name = عدم;

	إذا (dom_html_داخلہ_element_get_form(داخلہ, &form) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_داخلہ_element_get_type(داخلہ, &ds_type) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (dom_html_داخلہ_element_get_name(داخلہ, &ds_name) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (ds_name != عدم)
		name = strndup(dom_string_انکڑے(ds_name),
			       dom_string_byte_لمائی(ds_name));

	إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(داخلہ, GADGET_PASSWORD);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(داخلہ, GADGET_FILE);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(داخلہ, GADGET_HIDDEN);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(داخلہ, GADGET_CHECKBOX);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(داخلہ, GADGET_RADIO);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(داخلہ, GADGET_SUBMIT);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(داخلہ, GADGET_RESET);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(داخلہ, GADGET_BUTTON);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(داخلہ, GADGET_IMAGE);
	} وإلا {
		control = form_new_control(داخلہ, GADGET_TEXTBOX);
	}

	إذا (control == عدم)
		اذهب باہرلا;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		إذا (dom_html_داخلہ_element_get_checked(
			    داخلہ, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	إذا (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		إذا (dom_html_داخلہ_element_get_max_لمائی(
			    داخلہ, &maxlength) != DOM_NO_ERR) {
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
		إذا (dom_html_داخلہ_element_get_قدر(
			    داخلہ, &ds_قدر) == DOM_NO_ERR) {
			إذا (ds_قدر != عدم) {
				control->قدر = strndup(
					dom_string_انکڑے(ds_قدر),
					dom_string_byte_لمائی(ds_قدر));
				إذا (control->قدر == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب باہرلا;
				}
				control->لمائی = لڑی_لمبائی(control->قدر);
			}
		}

		إذا (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			إذا (control->قدر == عدم) {
				control->قدر = strdup("");
				إذا (control->قدر == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب باہرلا;
				}

				control->لمائی = 0;
			}

			control->initial_قدر = strdup(control->قدر);
			إذا (control->initial_قدر == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب باہرلا;
			}

			control->آخری_synced_قدر = strdup(control->قدر);
			إذا (control->آخری_synced_قدر == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب باہرلا;
			}

			control->گرہ_قدر = dom_string_ref(ds_قدر);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	إذا (form != عدم && control != عدم)
		form_جوڑو_control(لبھو_form(forms, form), control);

باہرلا:
	إذا (form != عدم)
		dom_گرہ_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);
	إذا (ds_قدر != عدم)
		dom_string_unref(ds_قدر);

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
		اذهب باہرلا;

	إذا (dom_html_متن_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (ds_name != عدم)
		name = strndup(dom_string_انکڑے(ds_name),
			       dom_string_byte_لمائی(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	إذا (control == عدم)
		اذهب باہرلا;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (form != عدم && control != عدم)
		form_جوڑو_control(لبھو_form(forms, form), control);

باہرلا:
	إذا (form != عدم)
		dom_گرہ_unref(form);
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
		اذهب باہرلا;

	إذا (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		اذهب باہرلا;

	إذا (ds_name != عدم)
		name = strndup(dom_string_انکڑے(ds_name),
			       dom_string_byte_لمائی(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	إذا (control == عدم)
		اذهب باہرلا;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	dom_html_select_element_get_مضرب(select,
			&(control->انکڑے.select.مضرب));

	إذا (form != عدم && control != عدم)
		form_جوڑو_control(لبھو_form(forms, form), control);

باہرلا:
	إذا (form != عدم)
		dom_گرہ_unref(form);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	إذا (name != عدم)
		حرر(name);


	أرجع control;
}


ساكن بنية form_control *
invent_fake_gadget(dom_گرہ *گرہ)
{
	بنية form_control *ctl = form_new_control(گرہ, GADGET_HIDDEN);
	إذا (ctl != عدم) {
		ctl->قدر = strdup("");
		ctl->initial_قدر = strdup("");
		ctl->name = strdup("foo");

		إذا (ctl->قدر == عدم || ctl->initial_قدر == عدم ||
				ctl->name == عدم) {
			form_free_control(ctl);
			ctl = عدم;
		}
	}
	أرجع ctl;
}

/* documented in html_internal.h */
بنية form_control *
html_forms_get_control_for_گرہ(بنية form *forms, dom_گرہ *گرہ)
{
	بنية form *f;
	بنية form_control *ctl = عدم;
	dom_exception err;
	dom_string *ds_name = عدم;

	/* Step one, see if we already have a control */
	لكل (f = forms; f != عدم; f = f->prev) {
		لكل (ctl = f->controls; ctl != عدم; ctl = ctl->اگلا) {
			إذا (ctl->گرہ == گرہ)
				أرجع ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_ٹیگ_name(گرہ, &ds_name);
	إذا (err == DOM_NO_ERR && ds_name != عدم) {

		/* Step three, attempt to work out what gadget to make */
		إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) گرہ);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_داخلہ)) {
			ctl = parse_داخلہ_element(forms,
					(dom_html_داخلہ_element *) گرہ);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_متن_area_element *) گرہ);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) گرہ);
		}
	}

	/* If all else fails, fake gadget time */
	إذا (ctl == عدم)
		ctl = invent_fake_gadget(گرہ);

	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع ctl;
}
