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
parse_form_element(ثابت حرف *docenc, dom_غوټه *غوټه)
{
	dom_string *ds_action = عدم;
	dom_string *ds_charset = عدم;
	dom_string *ds_موخه = عدم;
	dom_string *ds_لاره = عدم;
	dom_string *ds_enctype = عدم;
	حرف *action = عدم, *charset = عدم, *موخه = عدم;
	form_لاره لاره;
	dom_html_form_element *formele = (dom_html_form_element *)(غوټه);
	بنية form * ret = عدم;

	/* Retrieve the attributes from the node */
	إذا (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_form_element_get_موخه(formele,
			&ds_موخه) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_form_element_get_لاره(formele,
			&ds_لاره) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		اذهب بهر;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	إذا (ds_action != عدم)
		action = strndup(dom_string_معلومات(ds_action),
				 dom_string_byte_اوږدتیا(ds_action));

	إذا (ds_charset != عدم)
		charset = strndup(dom_string_معلومات(ds_charset),
				  dom_string_byte_اوږدتیا(ds_charset));

	إذا (ds_موخه != عدم)
		موخه = strndup(dom_string_معلومات(ds_موخه),
				 dom_string_byte_اوږدتیا(ds_موخه));

	/* Determine the method */
	لاره = لاره_GET;
	إذا (ds_لاره != عدم) {
		إذا (dom_string_caseless_lwc_isequal(ds_لاره,
				corestring_lwc_post)) {
			لاره = لاره_POST_URLENC;
			إذا (ds_enctype != عدم) {
				إذا (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_معلومات)) {

					لاره = لاره_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(غوټه, action, موخه, لاره, charset, docenc);

بهر:
	إذا (ds_action != عدم)
		dom_string_unref(ds_action);
	إذا (ds_charset != عدم)
		dom_string_unref(ds_charset);
	إذا (ds_موخه != عدم)
		dom_string_unref(ds_موخه);
	إذا (ds_لاره != عدم)
		dom_string_unref(ds_لاره);
	إذا (ds_enctype != عدم)
		dom_string_unref(ds_enctype);
	إذا (action != عدم)
		حرر(action);
	إذا (charset != عدم)
		حرر(charset);
	إذا (موخه != عدم)
		حرر(موخه);
	أرجع ret;
}

/* documented in html_internal.h */
بنية form *html_forms_get_forms(ثابت حرف *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	بنية form *ret = عدم, *newf;
	dom_غوټه *غوټه;
	بینښه اوږد n;
	uint32_t nforms;

	إذا (doc == عدم)
		أرجع عدم;

	/* Attempt to build a set of all the forms */
	إذا (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		أرجع عدم;

	/* Count the number of forms so we can iterate */
	إذا (dom_html_collection_get_اوږدتیا(forms, &nforms) != DOM_NO_ERR)
		اذهب بهر;

	/* Iterate the forms collection, making form structs for returning */
	لكل (n = 0; n < nforms; ++n) {
		إذا (dom_html_collection_توکی(forms, n, &غوټه) != DOM_NO_ERR) {
			اذهب بهر;
		}
		newf = parse_form_element(docenc, غوټه);
		dom_غوټه_unref(غوټه);
		إذا (newf == عدم) {
			اذهب err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	اذهب بهر;
err:
	طالما (ret != عدم) {
		بنية form *prev = ret->prev;
		/* Destroy ret */
		حرر(ret);
		ret = prev;
	}
بهر:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	أرجع ret;
}

ساكن بنية form *
پیدا_کړه_form(بنية form *forms, dom_html_form_element *form)
{
	طالما (forms != عدم) {
		إذا (forms->غوټه == form)
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
	dom_string *ds_ارزښت = عدم;
	dom_string *ds_name = عدم;

	err = dom_html_button_element_get_form(button, &form);
	إذا (err != DOM_NO_ERR)
		اذهب بهر;

	err = dom_html_button_element_get_type(button, &ds_type);
	إذا (err != DOM_NO_ERR)
		اذهب بهر;

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
		اذهب بهر;

	err = dom_html_button_element_get_ارزښت(button, &ds_ارزښت);
	إذا (err != DOM_NO_ERR)
		اذهب بهر;
	err = dom_html_button_element_get_name(button, &ds_name);
	إذا (err != DOM_NO_ERR)
		اذهب بهر;

	إذا (ds_ارزښت != عدم) {
		control->ارزښت = strndup(
			dom_string_معلومات(ds_ارزښت),
			dom_string_byte_اوږدتیا(ds_ارزښت));

		إذا (control->ارزښت == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب بهر;
		}
	}

	إذا (ds_name != عدم) {
		control->name = strndup(
			dom_string_معلومات(ds_name),
			dom_string_byte_اوږدتیا(ds_name));

		إذا (control->name == عدم) {
			form_free_control(control);
			control = عدم;
			اذهب بهر;
		}
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کړه_control(پیدا_کړه_form(forms, form), control);

بهر:
	إذا (form != عدم)
		dom_غوټه_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_ارزښت != عدم)
		dom_string_unref(ds_ارزښت);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع control;
}

ساكن بنية form_control *
parse_ننوت_element(بنية form *forms, dom_html_ننوت_element *ننوت)
{
	بنية form_control *control = عدم;
	dom_html_form_element *form = عدم;
	dom_string *ds_type = عدم;
	dom_string *ds_name = عدم;
	dom_string *ds_ارزښت = عدم;

	حرف *name = عدم;

	إذا (dom_html_ننوت_element_get_form(ننوت, &form) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_ننوت_element_get_type(ننوت, &ds_type) != DOM_NO_ERR)
		اذهب بهر;

	إذا (dom_html_ننوت_element_get_name(ننوت, &ds_name) != DOM_NO_ERR)
		اذهب بهر;

	إذا (ds_name != عدم)
		name = strndup(dom_string_معلومات(ds_name),
			       dom_string_byte_اوږدتیا(ds_name));

	إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ننوت, GADGET_PASSWORD);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ننوت, GADGET_FILE);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ننوت, GADGET_HIDDEN);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ننوت, GADGET_CHECKBOX);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ننوت, GADGET_RADIO);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ننوت, GADGET_SUBMIT);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ننوت, GADGET_RESET);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ننوت, GADGET_BUTTON);
	} وإلا إذا (ds_type != عدم && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ننوت, GADGET_IMAGE);
	} وإلا {
		control = form_new_control(ننوت, GADGET_TEXTBOX);
	}

	إذا (control == عدم)
		اذهب بهر;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		إذا (dom_html_ننوت_element_get_checked(
			    ننوت, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	إذا (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		إذا (dom_html_ننوت_element_get_max_اوږدتیا(
			    ننوت, &maxlength) != DOM_NO_ERR) {
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
		إذا (dom_html_ننوت_element_get_ارزښت(
			    ننوت, &ds_ارزښت) == DOM_NO_ERR) {
			إذا (ds_ارزښت != عدم) {
				control->ارزښت = strndup(
					dom_string_معلومات(ds_ارزښت),
					dom_string_byte_اوږدتیا(ds_ارزښت));
				إذا (control->ارزښت == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب بهر;
				}
				control->اوږدتیا = مزي_اوږدوالی(control->ارزښت);
			}
		}

		إذا (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			إذا (control->ارزښت == عدم) {
				control->ارزښت = strdup("");
				إذا (control->ارزښت == عدم) {
					form_free_control(control);
					control = عدم;
					اذهب بهر;
				}

				control->اوږدتیا = 0;
			}

			control->initial_ارزښت = strdup(control->ارزښت);
			إذا (control->initial_ارزښت == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب بهر;
			}

			control->وروستی_synced_ارزښت = strdup(control->ارزښت);
			إذا (control->وروستی_synced_ارزښت == عدم) {
				form_free_control(control);
				control = عدم;
				اذهب بهر;
			}

			control->غوټه_ارزښت = dom_string_ref(ds_ارزښت);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کړه_control(پیدا_کړه_form(forms, form), control);

بهر:
	إذا (form != عدم)
		dom_غوټه_unref(form);
	إذا (ds_type != عدم)
		dom_string_unref(ds_type);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);
	إذا (ds_ارزښت != عدم)
		dom_string_unref(ds_ارزښت);

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
		اذهب بهر;

	إذا (dom_html_متن_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		اذهب بهر;

	إذا (ds_name != عدم)
		name = strndup(dom_string_معلومات(ds_name),
			       dom_string_byte_اوږدتیا(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	إذا (control == عدم)
		اذهب بهر;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	إذا (form != عدم && control != عدم)
		form_جمع_کړه_control(پیدا_کړه_form(forms, form), control);

بهر:
	إذا (form != عدم)
		dom_غوټه_unref(form);
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
		اذهب بهر;

	إذا (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		اذهب بهر;

	إذا (ds_name != عدم)
		name = strndup(dom_string_معلومات(ds_name),
			       dom_string_byte_اوږدتیا(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	إذا (control == عدم)
		اذهب بهر;

	إذا (name != عدم) {
		/* Hand the name string over */
		control->name = name;
		name = عدم;
	}

	dom_html_select_element_get_مضرب(select,
			&(control->معلومات.select.مضرب));

	إذا (form != عدم && control != عدم)
		form_جمع_کړه_control(پیدا_کړه_form(forms, form), control);

بهر:
	إذا (form != عدم)
		dom_غوټه_unref(form);
	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	إذا (name != عدم)
		حرر(name);


	أرجع control;
}


ساكن بنية form_control *
invent_fake_gadget(dom_غوټه *غوټه)
{
	بنية form_control *ctl = form_new_control(غوټه, GADGET_HIDDEN);
	إذا (ctl != عدم) {
		ctl->ارزښت = strdup("");
		ctl->initial_ارزښت = strdup("");
		ctl->name = strdup("foo");

		إذا (ctl->ارزښت == عدم || ctl->initial_ارزښت == عدم ||
				ctl->name == عدم) {
			form_free_control(ctl);
			ctl = عدم;
		}
	}
	أرجع ctl;
}

/* documented in html_internal.h */
بنية form_control *
html_forms_get_control_for_غوټه(بنية form *forms, dom_غوټه *غوټه)
{
	بنية form *f;
	بنية form_control *ctl = عدم;
	dom_exception err;
	dom_string *ds_name = عدم;

	/* Step one, see if we already have a control */
	لكل (f = forms; f != عدم; f = f->prev) {
		لكل (ctl = f->controls; ctl != عدم; ctl = ctl->بل) {
			إذا (ctl->غوټه == غوټه)
				أرجع ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_نښان_name(غوټه, &ds_name);
	إذا (err == DOM_NO_ERR && ds_name != عدم) {

		/* Step three, attempt to work out what gadget to make */
		إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) غوټه);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ننوت)) {
			ctl = parse_ننوت_element(forms,
					(dom_html_ننوت_element *) غوټه);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_متن_area_element *) غوټه);
		} وإلا إذا (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) غوټه);
		}
	}

	/* If all else fails, fake gadget time */
	إذا (ctl == عدم)
		ctl = invent_fake_gadget(غوټه);

	إذا (ds_name != عدم)
		dom_string_unref(ds_name);

	أرجع ctl;
}
