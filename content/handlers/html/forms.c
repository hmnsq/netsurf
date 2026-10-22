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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"

/**
 * process form element from dom
 */
ਸਥਿਰ ਬਣਤਰ form *
parse_form_element(ਅਚਲ ਅੱਖਰ *docenc, dom_ਗੰਢ *ਗੰਢ)
{
	dom_string *ds_action = ਨਿਰਾ;
	dom_string *ds_charset = ਨਿਰਾ;
	dom_string *ds_ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;
	dom_string *ds_ਵਿਧੀ = ਨਿਰਾ;
	dom_string *ds_enctype = ਨਿਰਾ;
	ਅੱਖਰ *action = ਨਿਰਾ, *charset = ਨਿਰਾ, *ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;
	form_ਵਿਧੀ ਵਿਧੀ;
	dom_html_form_element *formele = (dom_html_form_element *)(ਗੰਢ);
	ਬਣਤਰ form * ret = ਨਿਰਾ;

	/* Retrieve the attributes from the node */
	ਜੇ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_form_element_get_ਨਿਸ਼ਾਨਾ(formele,
			&ds_ਨਿਸ਼ਾਨਾ) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_form_element_get_ਵਿਧੀ(formele,
			&ds_ਵਿਧੀ) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ਜੇ (ds_action != ਨਿਰਾ)
		action = strndup(dom_string_ਅੰਕੜੇ(ds_action),
				 dom_string_byte_ਲੰਮਾਈ(ds_action));

	ਜੇ (ds_charset != ਨਿਰਾ)
		charset = strndup(dom_string_ਅੰਕੜੇ(ds_charset),
				  dom_string_byte_ਲੰਮਾਈ(ds_charset));

	ਜੇ (ds_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
		ਨਿਸ਼ਾਨਾ = strndup(dom_string_ਅੰਕੜੇ(ds_ਨਿਸ਼ਾਨਾ),
				 dom_string_byte_ਲੰਮਾਈ(ds_ਨਿਸ਼ਾਨਾ));

	/* Determine the method */
	ਵਿਧੀ = ਵਿਧੀ_GET;
	ਜੇ (ds_ਵਿਧੀ != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(ds_ਵਿਧੀ,
				corestring_lwc_post)) {
			ਵਿਧੀ = ਵਿਧੀ_POST_URLENC;
			ਜੇ (ds_enctype != ਨਿਰਾ) {
				ਜੇ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_ਅੰਕੜੇ)) {

					ਵਿਧੀ = ਵਿਧੀ_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(ਗੰਢ, action, ਨਿਸ਼ਾਨਾ, ਵਿਧੀ, charset, docenc);

ਬਾਹਰਲਾ:
	ਜੇ (ds_action != ਨਿਰਾ)
		dom_string_unref(ds_action);
	ਜੇ (ds_charset != ਨਿਰਾ)
		dom_string_unref(ds_charset);
	ਜੇ (ds_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
		dom_string_unref(ds_ਨਿਸ਼ਾਨਾ);
	ਜੇ (ds_ਵਿਧੀ != ਨਿਰਾ)
		dom_string_unref(ds_ਵਿਧੀ);
	ਜੇ (ds_enctype != ਨਿਰਾ)
		dom_string_unref(ds_enctype);
	ਜੇ (action != ਨਿਰਾ)
		ਮੁਕਤ(action);
	ਜੇ (charset != ਨਿਰਾ)
		ਮੁਕਤ(charset);
	ਜੇ (ਨਿਸ਼ਾਨਾ != ਨਿਰਾ)
		ਮੁਕਤ(ਨਿਸ਼ਾਨਾ);
	ਵਾਪਸ ret;
}

/* documented in html_internal.h */
ਬਣਤਰ form *html_forms_get_forms(ਅਚਲ ਅੱਖਰ *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	ਬਣਤਰ form *ret = ਨਿਰਾ, *newf;
	dom_ਗੰਢ *ਗੰਢ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ n;
	uint32_t nforms;

	ਜੇ (doc == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	/* Attempt to build a set of all the forms */
	ਜੇ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		ਵਾਪਸ ਨਿਰਾ;

	/* Count the number of forms so we can iterate */
	ਜੇ (dom_html_collection_get_ਲੰਮਾਈ(forms, &nforms) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	/* Iterate the forms collection, making form structs for returning */
	ਲਈ (n = 0; n < nforms; ++n) {
		ਜੇ (dom_html_collection_ਵਸਤੂ(forms, n, &ਗੰਢ) != DOM_NO_ERR) {
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		newf = parse_form_element(docenc, ਗੰਢ);
		dom_ਗੰਢ_unref(ਗੰਢ);
		ਜੇ (newf == ਨਿਰਾ) {
			ਜਾਓ err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	ਜਾਓ ਬਾਹਰਲਾ;
err:
	ਜਦੋਂ (ret != ਨਿਰਾ) {
		ਬਣਤਰ form *prev = ret->prev;
		/* Destroy ret */
		ਮੁਕਤ(ret);
		ret = prev;
	}
ਬਾਹਰਲਾ:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਬਣਤਰ form *
ਲੱਭੋ_form(ਬਣਤਰ form *forms, dom_html_form_element *form)
{
	ਜਦੋਂ (forms != ਨਿਰਾ) {
		ਜੇ (forms->ਗੰਢ == form)
			ਰੋਕੋ;
		forms = forms->prev;
	}

	ਵਾਪਸ forms;
}

ਸਥਿਰ ਬਣਤਰ form_control *
parse_button_element(ਬਣਤਰ form *forms, dom_html_button_element *button)
{
	ਬਣਤਰ form_control *control = ਨਿਰਾ;
	dom_exception err;
	dom_html_form_element *form = ਨਿਰਾ;
	dom_string *ds_type = ਨਿਰਾ;
	dom_string *ds_ਮੁੱਲ = ਨਿਰਾ;
	dom_string *ds_name = ਨਿਰਾ;

	err = dom_html_button_element_get_form(button, &form);
	ਜੇ (err != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	err = dom_html_button_element_get_type(button, &ds_type);
	ਜੇ (err != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (ds_type == ਨਿਰਾ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} ਨਹੀਂਤਾਂ {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ਜੇ (control == ਨਿਰਾ)
		ਜਾਓ ਬਾਹਰਲਾ;

	err = dom_html_button_element_get_ਮੁੱਲ(button, &ds_ਮੁੱਲ);
	ਜੇ (err != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;
	err = dom_html_button_element_get_name(button, &ds_name);
	ਜੇ (err != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (ds_ਮੁੱਲ != ਨਿਰਾ) {
		control->ਮੁੱਲ = strndup(
			dom_string_ਅੰਕੜੇ(ds_ਮੁੱਲ),
			dom_string_byte_ਲੰਮਾਈ(ds_ਮੁੱਲ));

		ਜੇ (control->ਮੁੱਲ == ਨਿਰਾ) {
			form_free_control(control);
			control = ਨਿਰਾ;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	}

	ਜੇ (ds_name != ਨਿਰਾ) {
		control->name = strndup(
			dom_string_ਅੰਕੜੇ(ds_name),
			dom_string_byte_ਲੰਮਾਈ(ds_name));

		ਜੇ (control->name == ਨਿਰਾ) {
			form_free_control(control);
			control = ਨਿਰਾ;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	}

	ਜੇ (form != ਨਿਰਾ && control != ਨਿਰਾ)
		form_ਜੋੜੋ_control(ਲੱਭੋ_form(forms, form), control);

ਬਾਹਰਲਾ:
	ਜੇ (form != ਨਿਰਾ)
		dom_ਗੰਢ_unref(form);
	ਜੇ (ds_type != ਨਿਰਾ)
		dom_string_unref(ds_type);
	ਜੇ (ds_ਮੁੱਲ != ਨਿਰਾ)
		dom_string_unref(ds_ਮੁੱਲ);
	ਜੇ (ds_name != ਨਿਰਾ)
		dom_string_unref(ds_name);

	ਵਾਪਸ control;
}

ਸਥਿਰ ਬਣਤਰ form_control *
parse_ਨਿਵੇਸ਼_element(ਬਣਤਰ form *forms, dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼)
{
	ਬਣਤਰ form_control *control = ਨਿਰਾ;
	dom_html_form_element *form = ਨਿਰਾ;
	dom_string *ds_type = ਨਿਰਾ;
	dom_string *ds_name = ਨਿਰਾ;
	dom_string *ds_ਮੁੱਲ = ਨਿਰਾ;

	ਅੱਖਰ *name = ਨਿਰਾ;

	ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_form(ਨਿਵੇਸ਼, &form) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_type(ਨਿਵੇਸ਼, &ds_type) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_name(ਨਿਵੇਸ਼, &ds_name) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (ds_name != ਨਿਰਾ)
		name = strndup(dom_string_ਅੰਕੜੇ(ds_name),
			       dom_string_byte_ਲੰਮਾਈ(ds_name));

	ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_PASSWORD);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_FILE);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_HIDDEN);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_CHECKBOX);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_RADIO);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_SUBMIT);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_RESET);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_BUTTON);
	} ਨਹੀਂਤਾਂ ਜੇ (ds_type != ਨਿਰਾ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_IMAGE);
	} ਨਹੀਂਤਾਂ {
		control = form_new_control(ਨਿਵੇਸ਼, GADGET_TEXTBOX);
	}

	ਜੇ (control == ਨਿਰਾ)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (name != ਨਿਰਾ) {
		/* Hand the name string over */
		control->name = name;
		name = ਨਿਰਾ;
	}

	ਜੇ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_checked(
			    ਨਿਵੇਸ਼, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ਜੇ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_max_ਲੰਮਾਈ(
			    ਨਿਵੇਸ਼, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ਜੇ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} ਨਹੀਂਤਾਂ {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ਜੇ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ਜੇ (dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(
			    ਨਿਵੇਸ਼, &ds_ਮੁੱਲ) == DOM_NO_ERR) {
			ਜੇ (ds_ਮੁੱਲ != ਨਿਰਾ) {
				control->ਮੁੱਲ = strndup(
					dom_string_ਅੰਕੜੇ(ds_ਮੁੱਲ),
					dom_string_byte_ਲੰਮਾਈ(ds_ਮੁੱਲ));
				ਜੇ (control->ਮੁੱਲ == ਨਿਰਾ) {
					form_free_control(control);
					control = ਨਿਰਾ;
					ਜਾਓ ਬਾਹਰਲਾ;
				}
				control->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(control->ਮੁੱਲ);
			}
		}

		ਜੇ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ਜੇ (control->ਮੁੱਲ == ਨਿਰਾ) {
				control->ਮੁੱਲ = strdup("");
				ਜੇ (control->ਮੁੱਲ == ਨਿਰਾ) {
					form_free_control(control);
					control = ਨਿਰਾ;
					ਜਾਓ ਬਾਹਰਲਾ;
				}

				control->ਲੰਮਾਈ = 0;
			}

			control->initial_ਮੁੱਲ = strdup(control->ਮੁੱਲ);
			ਜੇ (control->initial_ਮੁੱਲ == ਨਿਰਾ) {
				form_free_control(control);
				control = ਨਿਰਾ;
				ਜਾਓ ਬਾਹਰਲਾ;
			}

			control->ਆਖਰੀ_synced_ਮੁੱਲ = strdup(control->ਮੁੱਲ);
			ਜੇ (control->ਆਖਰੀ_synced_ਮੁੱਲ == ਨਿਰਾ) {
				form_free_control(control);
				control = ਨਿਰਾ;
				ਜਾਓ ਬਾਹਰਲਾ;
			}

			control->ਗੰਢ_ਮੁੱਲ = dom_string_ref(ds_ਮੁੱਲ);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ਜੇ (form != ਨਿਰਾ && control != ਨਿਰਾ)
		form_ਜੋੜੋ_control(ਲੱਭੋ_form(forms, form), control);

ਬਾਹਰਲਾ:
	ਜੇ (form != ਨਿਰਾ)
		dom_ਗੰਢ_unref(form);
	ਜੇ (ds_type != ਨਿਰਾ)
		dom_string_unref(ds_type);
	ਜੇ (ds_name != ਨਿਰਾ)
		dom_string_unref(ds_name);
	ਜੇ (ds_ਮੁੱਲ != ਨਿਰਾ)
		dom_string_unref(ds_ਮੁੱਲ);

	ਜੇ (name != ਨਿਰਾ)
		ਮੁਕਤ(name);

	ਵਾਪਸ control;
}

ਸਥਿਰ ਬਣਤਰ form_control *
parse_textarea_element(ਬਣਤਰ form *forms, dom_html_ਪਾਠ_area_element *ta)
{
	ਬਣਤਰ form_control *control = ਨਿਰਾ;
	dom_html_form_element *form = ਨਿਰਾ;
	dom_string *ds_name = ਨਿਰਾ;

	ਅੱਖਰ *name = ਨਿਰਾ;

	ਜੇ (dom_html_ਪਾਠ_area_element_get_form(ta, &form) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_ਪਾਠ_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (ds_name != ਨਿਰਾ)
		name = strndup(dom_string_ਅੰਕੜੇ(ds_name),
			       dom_string_byte_ਲੰਮਾਈ(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ਜੇ (control == ਨਿਰਾ)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (name != ਨਿਰਾ) {
		/* Hand the name string over */
		control->name = name;
		name = ਨਿਰਾ;
	}

	ਜੇ (form != ਨਿਰਾ && control != ਨਿਰਾ)
		form_ਜੋੜੋ_control(ਲੱਭੋ_form(forms, form), control);

ਬਾਹਰਲਾ:
	ਜੇ (form != ਨਿਰਾ)
		dom_ਗੰਢ_unref(form);
	ਜੇ (ds_name != ਨਿਰਾ)
		dom_string_unref(ds_name);

	ਜੇ (name != ਨਿਰਾ)
		ਮੁਕਤ(name);


	ਵਾਪਸ control;
}

ਸਥਿਰ ਬਣਤਰ form_control *
parse_select_element(ਬਣਤਰ form *forms, dom_html_select_element *select)
{
	ਬਣਤਰ form_control *control = ਨਿਰਾ;
	dom_html_form_element *form = ਨਿਰਾ;
	dom_string *ds_name = ਨਿਰਾ;

	ਅੱਖਰ *name = ਨਿਰਾ;

	ਜੇ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (ds_name != ਨਿਰਾ)
		name = strndup(dom_string_ਅੰਕੜੇ(ds_name),
			       dom_string_byte_ਲੰਮਾਈ(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ਜੇ (control == ਨਿਰਾ)
		ਜਾਓ ਬਾਹਰਲਾ;

	ਜੇ (name != ਨਿਰਾ) {
		/* Hand the name string over */
		control->name = name;
		name = ਨਿਰਾ;
	}

	dom_html_select_element_get_ਗੁਣਜ(select,
			&(control->ਅੰਕੜੇ.select.ਗੁਣਜ));

	ਜੇ (form != ਨਿਰਾ && control != ਨਿਰਾ)
		form_ਜੋੜੋ_control(ਲੱਭੋ_form(forms, form), control);

ਬਾਹਰਲਾ:
	ਜੇ (form != ਨਿਰਾ)
		dom_ਗੰਢ_unref(form);
	ਜੇ (ds_name != ਨਿਰਾ)
		dom_string_unref(ds_name);

	ਜੇ (name != ਨਿਰਾ)
		ਮੁਕਤ(name);


	ਵਾਪਸ control;
}


ਸਥਿਰ ਬਣਤਰ form_control *
invent_fake_gadget(dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ form_control *ctl = form_new_control(ਗੰਢ, GADGET_HIDDEN);
	ਜੇ (ctl != ਨਿਰਾ) {
		ctl->ਮੁੱਲ = strdup("");
		ctl->initial_ਮੁੱਲ = strdup("");
		ctl->name = strdup("foo");

		ਜੇ (ctl->ਮੁੱਲ == ਨਿਰਾ || ctl->initial_ਮੁੱਲ == ਨਿਰਾ ||
				ctl->name == ਨਿਰਾ) {
			form_free_control(ctl);
			ctl = ਨਿਰਾ;
		}
	}
	ਵਾਪਸ ctl;
}

/* documented in html_internal.h */
ਬਣਤਰ form_control *
html_forms_get_control_for_ਗੰਢ(ਬਣਤਰ form *forms, dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ form *f;
	ਬਣਤਰ form_control *ctl = ਨਿਰਾ;
	dom_exception err;
	dom_string *ds_name = ਨਿਰਾ;

	/* Step one, see if we already have a control */
	ਲਈ (f = forms; f != ਨਿਰਾ; f = f->prev) {
		ਲਈ (ctl = f->controls; ctl != ਨਿਰਾ; ctl = ctl->ਅਗਲਾ) {
			ਜੇ (ctl->ਗੰਢ == ਗੰਢ)
				ਵਾਪਸ ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_ਟੈਗ_name(ਗੰਢ, &ds_name);
	ਜੇ (err == DOM_NO_ERR && ds_name != ਨਿਰਾ) {

		/* Step three, attempt to work out what gadget to make */
		ਜੇ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) ਗੰਢ);
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ਨਿਵੇਸ਼)) {
			ctl = parse_ਨਿਵੇਸ਼_element(forms,
					(dom_html_ਨਿਵੇਸ਼_element *) ਗੰਢ);
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_ਪਾਠ_area_element *) ਗੰਢ);
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) ਗੰਢ);
		}
	}

	/* If all else fails, fake gadget time */
	ਜੇ (ctl == ਨਿਰਾ)
		ctl = invent_fake_gadget(ਗੰਢ);

	ਜੇ (ds_name != ਨਿਰਾ)
		dom_string_unref(ds_name);

	ਵਾਪਸ ctl;
}
