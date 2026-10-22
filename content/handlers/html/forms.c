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

#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"

#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/private.h"

/**
 * process form element from dom
 */
స్థిర నిర్మాణం form *
parse_form_element(స్థిరరాశి అక్షరం *docenc, dom_కణుపు *కణుపు)
{
	dom_string *ds_action = ఖాళీ;
	dom_string *ds_charset = ఖాళీ;
	dom_string *ds_లక్ష్యం = ఖాళీ;
	dom_string *ds_పద్ధతి = ఖాళీ;
	dom_string *ds_enctype = ఖాళీ;
	అక్షరం *action = ఖాళీ, *charset = ఖాళీ, *లక్ష్యం = ఖాళీ;
	form_పద్ధతి పద్ధతి;
	dom_html_form_element *formele = (dom_html_form_element *)(కణుపు);
	నిర్మాణం form * ret = ఖాళీ;

	/* Retrieve the attributes from the node */
	ఒకవేళ (dom_html_form_element_get_action(formele,
			&ds_action) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_form_element_get_accept_charset(formele,
			&ds_charset) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_form_element_get_లక్ష్యం(formele,
			&ds_లక్ష్యం) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_form_element_get_పద్ధతి(formele,
			&ds_పద్ధతి) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_form_element_get_enctype(formele,
			&ds_enctype) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	/* Extract the plain attributes ready for use.  We have to do this
	 * because we cannot guarantee that the dom_strings are NULL terminated
	 * and thus we copy them.
	 */
	ఒకవేళ (ds_action != ఖాళీ)
		action = strndup(dom_string_దత్తాంశం(ds_action),
				 dom_string_byte_నిడివి(ds_action));

	ఒకవేళ (ds_charset != ఖాళీ)
		charset = strndup(dom_string_దత్తాంశం(ds_charset),
				  dom_string_byte_నిడివి(ds_charset));

	ఒకవేళ (ds_లక్ష్యం != ఖాళీ)
		లక్ష్యం = strndup(dom_string_దత్తాంశం(ds_లక్ష్యం),
				 dom_string_byte_నిడివి(ds_లక్ష్యం));

	/* Determine the method */
	పద్ధతి = పద్ధతి_GET;
	ఒకవేళ (ds_పద్ధతి != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(ds_పద్ధతి,
				corestring_lwc_post)) {
			పద్ధతి = పద్ధతి_POST_URLENC;
			ఒకవేళ (ds_enctype != ఖాళీ) {
				ఒకవేళ (dom_string_caseless_lwc_isequal(ds_enctype,
					corestring_lwc_multipart_form_దత్తాంశం)) {

					పద్ధతి = పద్ధతి_POST_MULTIPART;
				}
			}
		}
	}

	/* Construct the form object */
	ret = form_new(కణుపు, action, లక్ష్యం, పద్ధతి, charset, docenc);

బహిర్గతం:
	ఒకవేళ (ds_action != ఖాళీ)
		dom_string_unref(ds_action);
	ఒకవేళ (ds_charset != ఖాళీ)
		dom_string_unref(ds_charset);
	ఒకవేళ (ds_లక్ష్యం != ఖాళీ)
		dom_string_unref(ds_లక్ష్యం);
	ఒకవేళ (ds_పద్ధతి != ఖాళీ)
		dom_string_unref(ds_పద్ధతి);
	ఒకవేళ (ds_enctype != ఖాళీ)
		dom_string_unref(ds_enctype);
	ఒకవేళ (action != ఖాళీ)
		విడుదల(action);
	ఒకవేళ (charset != ఖాళీ)
		విడుదల(charset);
	ఒకవేళ (లక్ష్యం != ఖాళీ)
		విడుదల(లక్ష్యం);
	తిరిగి ret;
}

/* documented in html_internal.h */
నిర్మాణం form *html_forms_get_forms(స్థిరరాశి అక్షరం *docenc, dom_html_document *doc)
{
	dom_html_collection *forms;
	నిర్మాణం form *ret = ఖాళీ, *newf;
	dom_కణుపు *కణుపు;
	గుర్తులేని దీర్ఘ n;
	uint32_t nforms;

	ఒకవేళ (doc == ఖాళీ)
		తిరిగి ఖాళీ;

	/* Attempt to build a set of all the forms */
	ఒకవేళ (dom_html_document_get_forms(doc, &forms) != DOM_NO_ERR)
		తిరిగి ఖాళీ;

	/* Count the number of forms so we can iterate */
	ఒకవేళ (dom_html_collection_get_నిడివి(forms, &nforms) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	/* Iterate the forms collection, making form structs for returning */
	కొరకు (n = 0; n < nforms; ++n) {
		ఒకవేళ (dom_html_collection_అంశం(forms, n, &కణుపు) != DOM_NO_ERR) {
			వెళ్ళు బహిర్గతం;
		}
		newf = parse_form_element(docenc, కణుపు);
		dom_కణుపు_unref(కణుపు);
		ఒకవేళ (newf == ఖాళీ) {
			వెళ్ళు err;
		}
		newf->prev = ret;
		ret = newf;
	}

	/* All went well */
	వెళ్ళు బహిర్గతం;
err:
	అయితే (ret != ఖాళీ) {
		నిర్మాణం form *prev = ret->prev;
		/* Destroy ret */
		విడుదల(ret);
		ret = prev;
	}
బహిర్గతం:
	/* Finished with the collection, return it */
	dom_html_collection_unref(forms);

	తిరిగి ret;
}

స్థిర నిర్మాణం form *
కనుగొను_form(నిర్మాణం form *forms, dom_html_form_element *form)
{
	అయితే (forms != ఖాళీ) {
		ఒకవేళ (forms->కణుపు == form)
			ఆపు;
		forms = forms->prev;
	}

	తిరిగి forms;
}

స్థిర నిర్మాణం form_control *
parse_button_element(నిర్మాణం form *forms, dom_html_button_element *button)
{
	నిర్మాణం form_control *control = ఖాళీ;
	dom_exception err;
	dom_html_form_element *form = ఖాళీ;
	dom_string *ds_type = ఖాళీ;
	dom_string *ds_విలువ = ఖాళీ;
	dom_string *ds_name = ఖాళీ;

	err = dom_html_button_element_get_form(button, &form);
	ఒకవేళ (err != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	err = dom_html_button_element_get_type(button, &ds_type);
	ఒకవేళ (err != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (ds_type == ఖాళీ) {
		control = form_new_control(button, GADGET_SUBMIT);
	} లేకపోతే {
		ఒకవేళ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_submit)) {
			control = form_new_control(button, GADGET_SUBMIT);
		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(ds_type,
				corestring_lwc_reset)) {
			control = form_new_control(button, GADGET_RESET);
		} లేకపోతే {
			control = form_new_control(button, GADGET_BUTTON);
		}
	}

	ఒకవేళ (control == ఖాళీ)
		వెళ్ళు బహిర్గతం;

	err = dom_html_button_element_get_విలువ(button, &ds_విలువ);
	ఒకవేళ (err != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;
	err = dom_html_button_element_get_name(button, &ds_name);
	ఒకవేళ (err != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (ds_విలువ != ఖాళీ) {
		control->విలువ = strndup(
			dom_string_దత్తాంశం(ds_విలువ),
			dom_string_byte_నిడివి(ds_విలువ));

		ఒకవేళ (control->విలువ == ఖాళీ) {
			form_free_control(control);
			control = ఖాళీ;
			వెళ్ళు బహిర్గతం;
		}
	}

	ఒకవేళ (ds_name != ఖాళీ) {
		control->name = strndup(
			dom_string_దత్తాంశం(ds_name),
			dom_string_byte_నిడివి(ds_name));

		ఒకవేళ (control->name == ఖాళీ) {
			form_free_control(control);
			control = ఖాళీ;
			వెళ్ళు బహిర్గతం;
		}
	}

	ఒకవేళ (form != ఖాళీ && control != ఖాళీ)
		form_కూడు_control(కనుగొను_form(forms, form), control);

బహిర్గతం:
	ఒకవేళ (form != ఖాళీ)
		dom_కణుపు_unref(form);
	ఒకవేళ (ds_type != ఖాళీ)
		dom_string_unref(ds_type);
	ఒకవేళ (ds_విలువ != ఖాళీ)
		dom_string_unref(ds_విలువ);
	ఒకవేళ (ds_name != ఖాళీ)
		dom_string_unref(ds_name);

	తిరిగి control;
}

స్థిర నిర్మాణం form_control *
parse_ఆదానం_element(నిర్మాణం form *forms, dom_html_ఆదానం_element *ఆదానం)
{
	నిర్మాణం form_control *control = ఖాళీ;
	dom_html_form_element *form = ఖాళీ;
	dom_string *ds_type = ఖాళీ;
	dom_string *ds_name = ఖాళీ;
	dom_string *ds_విలువ = ఖాళీ;

	అక్షరం *name = ఖాళీ;

	ఒకవేళ (dom_html_ఆదానం_element_get_form(ఆదానం, &form) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_ఆదానం_element_get_type(ఆదానం, &ds_type) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_ఆదానం_element_get_name(ఆదానం, &ds_name) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (ds_name != ఖాళీ)
		name = strndup(dom_string_దత్తాంశం(ds_name),
			       dom_string_byte_నిడివి(ds_name));

	ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_password)) {
		control = form_new_control(ఆదానం, GADGET_PASSWORD);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_file)) {
		control = form_new_control(ఆదానం, GADGET_FILE);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_hidden)) {
		control = form_new_control(ఆదానం, GADGET_HIDDEN);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_checkbox)) {
		control = form_new_control(ఆదానం, GADGET_CHECKBOX);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_radio)) {
		control = form_new_control(ఆదానం, GADGET_RADIO);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_submit)) {
		control = form_new_control(ఆదానం, GADGET_SUBMIT);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_reset)) {
		control = form_new_control(ఆదానం, GADGET_RESET);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_button)) {
		control = form_new_control(ఆదానం, GADGET_BUTTON);
	} లేకపోతే ఒకవేళ (ds_type != ఖాళీ && dom_string_caseless_lwc_isequal(ds_type,
			corestring_lwc_image)) {
		control = form_new_control(ఆదానం, GADGET_IMAGE);
	} లేకపోతే {
		control = form_new_control(ఆదానం, GADGET_TEXTBOX);
	}

	ఒకవేళ (control == ఖాళీ)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (name != ఖాళీ) {
		/* Hand the name string over */
		control->name = name;
		name = ఖాళీ;
	}

	ఒకవేళ (control->type == GADGET_CHECKBOX || control->type == GADGET_RADIO) {
		bool selected;
		ఒకవేళ (dom_html_ఆదానం_element_get_checked(
			    ఆదానం, &selected) == DOM_NO_ERR) {
			control->selected = selected;
		}
	}

	ఒకవేళ (control->type == GADGET_PASSWORD ||
	    control->type == GADGET_TEXTBOX) {
		int32_t maxlength;
		ఒకవేళ (dom_html_ఆదానం_element_get_max_నిడివి(
			    ఆదానం, &maxlength) != DOM_NO_ERR) {
			maxlength = -1;
		}

		ఒకవేళ (maxlength >= 0) {
			/* Got valid maxlength */
			control->maxlength = maxlength;
		} లేకపోతే {
			/* Input has no maxlength attr, or
			 * dom_html_input_element_get_max_length failed.
			 *
			 * Set it to something insane. */
			control->maxlength = UINT_MAX;
		}
	}

	ఒకవేళ (control->type != GADGET_FILE && control->type != GADGET_IMAGE) {
		ఒకవేళ (dom_html_ఆదానం_element_get_విలువ(
			    ఆదానం, &ds_విలువ) == DOM_NO_ERR) {
			ఒకవేళ (ds_విలువ != ఖాళీ) {
				control->విలువ = strndup(
					dom_string_దత్తాంశం(ds_విలువ),
					dom_string_byte_నిడివి(ds_విలువ));
				ఒకవేళ (control->విలువ == ఖాళీ) {
					form_free_control(control);
					control = ఖాళీ;
					వెళ్ళు బహిర్గతం;
				}
				control->నిడివి = అక్షరశ్రేణి_పొడవు(control->విలువ);
			}
		}

		ఒకవేళ (control->type == GADGET_TEXTBOX ||
				control->type == GADGET_PASSWORD) {
			ఒకవేళ (control->విలువ == ఖాళీ) {
				control->విలువ = strdup("");
				ఒకవేళ (control->విలువ == ఖాళీ) {
					form_free_control(control);
					control = ఖాళీ;
					వెళ్ళు బహిర్గతం;
				}

				control->నిడివి = 0;
			}

			control->initial_విలువ = strdup(control->విలువ);
			ఒకవేళ (control->initial_విలువ == ఖాళీ) {
				form_free_control(control);
				control = ఖాళీ;
				వెళ్ళు బహిర్గతం;
			}

			control->చివరి_synced_విలువ = strdup(control->విలువ);
			ఒకవేళ (control->చివరి_synced_విలువ == ఖాళీ) {
				form_free_control(control);
				control = ఖాళీ;
				వెళ్ళు బహిర్గతం;
			}

			control->కణుపు_విలువ = dom_string_ref(ds_విలువ);
		}
		/* Force the gadget and DOM to be in sync */
		form_gadget_sync_with_dom(control);
	}

	ఒకవేళ (form != ఖాళీ && control != ఖాళీ)
		form_కూడు_control(కనుగొను_form(forms, form), control);

బహిర్గతం:
	ఒకవేళ (form != ఖాళీ)
		dom_కణుపు_unref(form);
	ఒకవేళ (ds_type != ఖాళీ)
		dom_string_unref(ds_type);
	ఒకవేళ (ds_name != ఖాళీ)
		dom_string_unref(ds_name);
	ఒకవేళ (ds_విలువ != ఖాళీ)
		dom_string_unref(ds_విలువ);

	ఒకవేళ (name != ఖాళీ)
		విడుదల(name);

	తిరిగి control;
}

స్థిర నిర్మాణం form_control *
parse_textarea_element(నిర్మాణం form *forms, dom_html_పాఠ్యం_area_element *ta)
{
	నిర్మాణం form_control *control = ఖాళీ;
	dom_html_form_element *form = ఖాళీ;
	dom_string *ds_name = ఖాళీ;

	అక్షరం *name = ఖాళీ;

	ఒకవేళ (dom_html_పాఠ్యం_area_element_get_form(ta, &form) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_పాఠ్యం_area_element_get_name(ta, &ds_name) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (ds_name != ఖాళీ)
		name = strndup(dom_string_దత్తాంశం(ds_name),
			       dom_string_byte_నిడివి(ds_name));

	control = form_new_control(ta, GADGET_TEXTAREA);

	ఒకవేళ (control == ఖాళీ)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (name != ఖాళీ) {
		/* Hand the name string over */
		control->name = name;
		name = ఖాళీ;
	}

	ఒకవేళ (form != ఖాళీ && control != ఖాళీ)
		form_కూడు_control(కనుగొను_form(forms, form), control);

బహిర్గతం:
	ఒకవేళ (form != ఖాళీ)
		dom_కణుపు_unref(form);
	ఒకవేళ (ds_name != ఖాళీ)
		dom_string_unref(ds_name);

	ఒకవేళ (name != ఖాళీ)
		విడుదల(name);


	తిరిగి control;
}

స్థిర నిర్మాణం form_control *
parse_select_element(నిర్మాణం form *forms, dom_html_select_element *select)
{
	నిర్మాణం form_control *control = ఖాళీ;
	dom_html_form_element *form = ఖాళీ;
	dom_string *ds_name = ఖాళీ;

	అక్షరం *name = ఖాళీ;

	ఒకవేళ (dom_html_select_element_get_form(select, &form) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (dom_html_select_element_get_name(select, &ds_name) != DOM_NO_ERR)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (ds_name != ఖాళీ)
		name = strndup(dom_string_దత్తాంశం(ds_name),
			       dom_string_byte_నిడివి(ds_name));

	control = form_new_control(select, GADGET_SELECT);

	ఒకవేళ (control == ఖాళీ)
		వెళ్ళు బహిర్గతం;

	ఒకవేళ (name != ఖాళీ) {
		/* Hand the name string over */
		control->name = name;
		name = ఖాళీ;
	}

	dom_html_select_element_get_గుణిజం(select,
			&(control->దత్తాంశం.select.గుణిజం));

	ఒకవేళ (form != ఖాళీ && control != ఖాళీ)
		form_కూడు_control(కనుగొను_form(forms, form), control);

బహిర్గతం:
	ఒకవేళ (form != ఖాళీ)
		dom_కణుపు_unref(form);
	ఒకవేళ (ds_name != ఖాళీ)
		dom_string_unref(ds_name);

	ఒకవేళ (name != ఖాళీ)
		విడుదల(name);


	తిరిగి control;
}


స్థిర నిర్మాణం form_control *
invent_fake_gadget(dom_కణుపు *కణుపు)
{
	నిర్మాణం form_control *ctl = form_new_control(కణుపు, GADGET_HIDDEN);
	ఒకవేళ (ctl != ఖాళీ) {
		ctl->విలువ = strdup("");
		ctl->initial_విలువ = strdup("");
		ctl->name = strdup("foo");

		ఒకవేళ (ctl->విలువ == ఖాళీ || ctl->initial_విలువ == ఖాళీ ||
				ctl->name == ఖాళీ) {
			form_free_control(ctl);
			ctl = ఖాళీ;
		}
	}
	తిరిగి ctl;
}

/* documented in html_internal.h */
నిర్మాణం form_control *
html_forms_get_control_for_కణుపు(నిర్మాణం form *forms, dom_కణుపు *కణుపు)
{
	నిర్మాణం form *f;
	నిర్మాణం form_control *ctl = ఖాళీ;
	dom_exception err;
	dom_string *ds_name = ఖాళీ;

	/* Step one, see if we already have a control */
	కొరకు (f = forms; f != ఖాళీ; f = f->prev) {
		కొరకు (ctl = f->controls; ctl != ఖాళీ; ctl = ctl->తదుపరి) {
			ఒకవేళ (ctl->కణుపు == కణుపు)
				తిరిగి ctl;
		}
	}

	/* Step two, extract the node's name so we can construct a gadget. */
	err = dom_element_get_ట్యాగ్_name(కణుపు, &ds_name);
	ఒకవేళ (err == DOM_NO_ERR && ds_name != ఖాళీ) {

		/* Step three, attempt to work out what gadget to make */
		ఒకవేళ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_button)) {
			ctl = parse_button_element(forms,
					(dom_html_button_element *) కణుపు);
		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_ఆదానం)) {
			ctl = parse_ఆదానం_element(forms,
					(dom_html_ఆదానం_element *) కణుపు);
		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_textarea)) {
			ctl = parse_textarea_element(forms,
					(dom_html_పాఠ్యం_area_element *) కణుపు);
		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(ds_name,
				corestring_lwc_select)) {
			ctl = parse_select_element(forms,
					(dom_html_select_element *) కణుపు);
		}
	}

	/* If all else fails, fake gadget time */
	ఒకవేళ (ctl == ఖాళీ)
		ctl = invent_fake_gadget(కణుపు);

	ఒకవేళ (ds_name != ఖాళీ)
		dom_string_unref(ds_name);

	తిరిగి ctl;
}
