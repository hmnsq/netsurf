/*
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 * Copyright 2005-9 John-Mark Bell <jmb@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
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
 * Form handling functions (implementation).
 */

#ضمن <assert.h>
#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <dom/dom.h>

#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/talloc.h"
#ضمن "utils/url.h"
#ضمن "utils/utf8.h"
#ضمن "utils/ascii.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/mouse.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/misc.h"
#ضمن "content/fetch.h"
#ضمن "content/hlcache.h"
#ضمن "css/utils.h"
#ضمن "desktop/knockout.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/layout.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"

#تعریف_کړه MAX_SELECT_لوړوالی 210
#تعریف_کړه SELECT_کرښه_SPACING 0.2
#تعریف_کړه SELECT_BORDER_پلنوالی 1
#تعریف_کړه SELECT_SELECTED_رنګ 0xDB9370

بنية form_select_menu {
	صحيح کرښه_لوړوالی;
	صحيح پلنوالی, لوړوالی;
	بنية scrollbar *scrollbar;
	صحيح f_اندازه;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	فراغ *client_معلومات;
	بنية content *c;
};

ساكن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_رنګ = SELECT_SELECTED_رنګ,
};

ساكن plot_font_style_t plot_fstyle_ثبت = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.وزن = 400,
	.flags = FONTF_NONE,
	.background = 0xffffff,
	.foreground = 0x000000,
};


/**
 * Convert a string from UTF-8 to the specified charset
 * As a final fallback, this will attempt to convert to ISO-8859-1.
 *
 * \todo Return charset used?
 *
 * \param item String to convert
 * \param len Length of string to convert
 * \param charset Destination charset
 * \param fallback Fallback charset (may be NULL),
 *                 used iff converting to charset fails
 * \return Pointer to converted string (on heap, caller frees), or NULL
 */
ساكن حرف *
form_encode_توکی(ثابت حرف *توکی,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = عدم;
	حرف cset[256];

	إذا (!توکی || !charset)
		أرجع عدم;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_منزل_enc(توکی, cset, 0, &ret);
	إذا (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_منزل_enc(توکی, cset, len, &ret);

		إذا (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			إذا (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_منزل_enc(توکی, cset, 0, &ret);

				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_منزل_enc(توکی, cset, 0, &ret);
				}
			}

			إذا (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_منزل_enc(توکی, "ISO-8859-1//TRANSLIT",
						0, &ret);
				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_منزل_enc(توکی, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	إذا (err == NSERROR_NOMEM) {
		أرجع عدم;
	}

	أرجع ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#تعریف_کړه FETCH_معلومات_INT_ارزښت_اندازه 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساكن nserror
fetch_معلومات_list_جمع_کړه_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحيح ارزښت,
			  بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	بنية fetch_multipart_معلومات *fetch_معلومات;
	صحيح keysize;

	fetch_معلومات = حافظه_صفر_ورکړه(1, حجم(*fetch_معلومات));
	إذا (fetch_معلومات == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_معلومات->name, 0, "%s%s", name, ksfx);
	fetch_معلومات->name = حافظه_ورکړه(keysize + 1); /* allow for null */
	إذا (fetch_معلومات->name == عدم) {
		حرر(fetch_معلومات);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_معلومات->name, keysize + 1, "%s%s", name, ksfx);

	/*ارزښت*/
	fetch_معلومات->ارزښت = حافظه_ورکړه(FETCH_معلومات_INT_ارزښت_اندازه);
	إذا (fetch_معلومات->ارزښت == عدم) {
		حرر(fetch_معلومات->name);
		حرر(fetch_معلومات);
		NSLOG(netsurf, INFO, "value allocation failure");
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_معلومات->ارزښت, FETCH_معلومات_INT_ارزښت_اندازه, "%d", ارزښت);

	/* link into list */
	**fetch_معلومات_بل_ptr = fetch_معلومات;
	*fetch_معلومات_بل_ptr = &fetch_معلومات->بل;

	أرجع NSERROR_OK;
}


/**
 * append DOM string name/value pair to a multipart data list
 *
 * \param name key name
 * \param value the value to associate with the key
 * \param rawfile the raw file value to associate with the key.
 * \param form_charset The form character set
 * \param docu_charset The document character set for fallback
 * \param fetch_data_next_ptr The multipart data list being constructed.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
fetch_معلومات_list_جمع_کړه(dom_string *name,
		    dom_string *ارزښت,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	بنية fetch_multipart_معلومات *fetch_معلومات;

	أكد(name != عدم);

	fetch_معلومات = حافظه_صفر_ورکړه(1, حجم(*fetch_معلومات));
	إذا (fetch_معلومات == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	fetch_معلومات->name = form_encode_توکی(dom_string_معلومات(name),
					    dom_string_byte_اوږدتیا(name),
					    form_charset,
					    docu_charset);
	إذا (fetch_معلومات->name == عدم) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		حرر(fetch_معلومات);
		أرجع NSERROR_NOMEM;
	}

	إذا (ارزښت == عدم) {
		fetch_معلومات->ارزښت = strdup("");
	} وإلا {
		fetch_معلومات->ارزښت = form_encode_توکی(dom_string_معلومات(ارزښت),
						     dom_string_byte_اوږدتیا(ارزښت),
						     form_charset,
						     docu_charset);
	}
	إذا (fetch_معلومات->ارزښت == عدم) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		حرر(fetch_معلومات->name);
		حرر(fetch_معلومات);
		أرجع NSERROR_NOMEM;
	}

	/* deal with raw file name */
	إذا (rawfile != عدم) {
		fetch_معلومات->file = true;
		fetch_معلومات->rawfile = strdup(rawfile);
		إذا (fetch_معلومات->rawfile == عدم) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			حرر(fetch_معلومات->ارزښت);
			حرر(fetch_معلومات->name);
			حرر(fetch_معلومات);
			أرجع NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_معلومات_بل_ptr = fetch_معلومات;
	*fetch_معلومات_بل_ptr = &fetch_معلومات->بل;

	أرجع NSERROR_OK;
}


/**
 * process form HTMLTextAreaElement into multipart data.
 *
 * \param text_area_element The form select DOM element to convert.
 * \param form_charset The form character set
 * \param doc_charset The document character set for fallback
 * \param fetch_data_next_ptr The multipart data list being constructed.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
form_dom_منزل_معلومات_textarea(dom_html_متن_area_element *متن_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	dom_exception توانیز_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	توانیز_تابع = dom_html_متن_area_element_get_disabled(متن_area_element,
						      &element_disabled);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	توانیز_تابع = dom_html_متن_area_element_get_name(متن_area_element,
						  &inputname);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* obtain text area value */
	توانیز_تابع = dom_html_متن_area_element_get_ارزښت(متن_area_element,
						   &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", توانیز_تابع);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(inputname,
				  inputvalue,
				  عدم,
				  form_charset,
				  doc_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	dom_string *ارزښت;
	bool selected;

	توانیز_تابع = dom_html_option_element_get_selected(option_element, &selected);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		أرجع NSERROR_DOM;
	}

	إذا (!selected) {
		/* unselected options do not add fetch data entries */
		أرجع NSERROR_OK;
	}

	توانیز_تابع = dom_html_option_element_get_ارزښت(option_element, &ارزښت);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(keyname,
				  ارزښت,
				  عدم,
				  form_charset,
				  docu_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(ارزښت);

	أرجع res;
}


/**
 * process form HTMLSelectElement into multipart data.
 *
 * \param select_element The form select DOM element to convert.
 * \param form_charset The form character set
 * \param doc_charset The document character set for fallback
 * \param fetch_data_next_ptr The multipart data list being constructed.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
form_dom_منزل_معلومات_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = عدم;
	uint32_t options_شمېر;
	uint32_t option_شاخص;
	dom_غوټه *option_element = عدم;

	/* check if element is disabled */
	توانیز_تابع = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	توانیز_تابع = dom_html_select_element_get_name(select_element, &inputname);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get options collection */
	توانیز_تابع = dom_html_select_element_get_options(select_element, &options);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* get options collection length */
	توانیز_تابع = dom_html_options_collection_get_اوږدتیا(options, &options_شمېر);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* iterate over options collection */
	لكل (option_شاخص = 0; option_شاخص < options_شمېر; ++option_شاخص) {
		توانیز_تابع = dom_html_options_collection_توکی(options,
						       option_شاخص,
						       &option_element);
		إذا (توانیز_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_شاخص);
			res = NSERROR_DOM;
		} وإلا {
			res = form_dom_منزل_معلومات_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_معلومات_بل_ptr);

			dom_غوټه_unref(option_element);
		}

		إذا (res != NSERROR_OK) {
			اكسر;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_ننوت_submit(dom_html_ننوت_element *ننوت_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	dom_exception توانیز_تابع; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	إذا (*submit_button == عدم) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ننوت_element;
	} وإلا إذا (*submit_button != (dom_html_element *)ننوت_element) {
		أرجع NSERROR_OK;
	}

	/* matched button used to submit form */
	توانیز_تابع = dom_html_ننوت_element_get_ارزښت(ننوت_element, &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_ننوت_image(dom_html_ننوت_element *ننوت_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	بنية image_ننوت_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	إذا (*submit_button != (dom_html_element *)ننوت_element) {
		أرجع NSERROR_OK;
	}

	توانیز_تابع = dom_غوټه_get_user_معلومات((dom_غوټه *)ننوت_element,
				     corestring_dom_ns_کلی_image_coords_غوټه_معلومات,
				     &coords);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		أرجع NSERROR_DOM;
	}

	إذا (coords == عدم) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		أرجع NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_توکی(dom_string_معلومات(inputname),
				    dom_string_byte_اوږدتیا(inputname),
				    charset,
				    document_charset);
	إذا (basename == عدم) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		أرجع NSERROR_NOMEM;
	}

	res = fetch_معلومات_list_جمع_کړه_sname(basename, ".x",
					coords->x,
					fetch_معلومات_بل_ptr);

	إذا (res == NSERROR_OK) {
		res = fetch_معلومات_list_جمع_کړه_sname(basename, ".y",
						coords->y,
						fetch_معلومات_بل_ptr);
	}

	حرر(basename);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_ننوت_checkbox(dom_html_ننوت_element *ننوت_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	توانیز_تابع = dom_html_ننوت_element_get_checked(ننوت_element, &checked);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		أرجع NSERROR_DOM;
	}

	إذا (!checked) {
		/* unchecked items do not generate a data entry */
		أرجع NSERROR_OK;
	}

	توانیز_تابع = dom_html_ننوت_element_get_ارزښت(ننوت_element, &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		أرجع NSERROR_DOM;
	}

	/* ensure a default value */
	إذا (inputvalue == عدم) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_ننوت_file(dom_html_ننوت_element *ننوت_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = عدم;

	توانیز_تابع = dom_html_ننوت_element_get_ارزښت(ننوت_element, &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		أرجع NSERROR_DOM;
	}

	توانیز_تابع = dom_غوټه_get_user_معلومات((dom_غوټه *)ننوت_element,
				     corestring_dom_ns_کلی_file_name_غوټه_معلومات,
				     &rawfile);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		أرجع NSERROR_DOM;
	}

	إذا (rawfile == عدم) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_ننوت_متن(dom_html_ننوت_element *ننوت_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	nserror res;
	dom_exception توانیز_تابع; /* the result from DOM operations */
	dom_string *inputvalue;

	توانیز_تابع = dom_html_ننوت_element_get_ارزښت(ننوت_element, &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جمع_کړه(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_بل_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


/**
 * process form input element into multipart data.
 *
 * \param input_element The form input DOM element to convert.
 * \param charset The form character set
 * \param document_charset The document character set for fallback
 * \param submit_button The DOM element of the button submitting the form
 * \param had_submit A boolean value indicating if the submit button
 *                   has already been processed in the form element enumeration.
 * \param fetch_data_next_ptr The multipart data list being constructed.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
form_dom_منزل_معلومات_ننوت(dom_html_ننوت_element *ننوت_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	dom_exception توانیز_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	توانیز_تابع = dom_html_ننوت_element_get_disabled(ننوت_element,
						  &element_disabled);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* disabled element requires no more processing */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	توانیز_تابع = dom_html_ننوت_element_get_name(ننوت_element, &inputname);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* element with no name is not converted */
		أرجع NSERROR_OK;
	}

	/* get input type */
	توانیز_تابع = dom_html_ننوت_element_get_type(ننوت_element, &inputtype);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* process according to input element type */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_منزل_معلومات_ننوت_submit(ننوت_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_معلومات_بل_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_منزل_معلومات_ننوت_image(ننوت_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_معلومات_بل_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_منزل_معلومات_ننوت_checkbox(ننوت_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_معلومات_بل_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_منزل_معلومات_ننوت_file(ننوت_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_معلومات_بل_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وإلا {
		/* Everything else is treated as text values */
		res = form_dom_منزل_معلومات_ننوت_متن(ننوت_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_معلومات_بل_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	أرجع res;
}


/**
 * process form HTMLButtonElement into multipart data.
 *
 * https://html.spec.whatwg.org/multipage/form-elements.html#the-button-element
 *
 * \param button_element The form button DOM element to convert.
 * \param form_charset The form character set
 * \param doc_charset The document character set for fallback
 * \param submit_button The DOM element of the button submitting the form
 * \param fetch_data_next_ptr The multipart data list being constructed.
 * \return NSERROR_OK on success or appropriate error code.
 */
ساكن nserror
form_dom_منزل_معلومات_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			بنية fetch_multipart_معلومات ***fetch_معلومات_بل_ptr)
{
	dom_exception توانیز_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	توانیز_تابع = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* get the type attribute */
	توانیز_تابع = dom_html_button_element_get_type(button_element, &inputtype);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		أرجع NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	إذا (*submit_button == عدم) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	إذا (*submit_button != (dom_html_element *)button_element) {
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	توانیز_تابع = dom_html_button_element_get_name(button_element, &inputname);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", توانیز_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	توانیز_تابع = dom_html_button_element_get_ارزښت(button_element, &inputvalue);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} وإلا {
		res = fetch_معلومات_list_جمع_کړه(inputname,
					  inputvalue,
					  عدم,
					  form_charset,
					  doc_charset,
					  fetch_معلومات_بل_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	أرجع res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ساكن حرف *form_acceptable_charset(بنية form *form)
{
	حرف *لنډمهاله, *c;

	إذا (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		إذا (form->document_charset) {
			/* document charset present, so use it */
			أرجع strdup(form->document_charset);
		} وإلا {
			/* no document charset, so default to 8859-1 */
			أرجع strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	لنډمهاله = strdup(form->accept_charsets);
	إذا (!لنډمهاله)
		أرجع عدم;

	/* make it upper case */
	لكل (c = لنډمهاله; *c; c++) {
		*c = ascii_منزل_پاسنی(*c);
	}

	/* is UTF-8 specified? */
	c = مزي_پلټنه(لنډمهاله, "UTF-8");
	إذا (c) {
		حرر(لنډمهاله);
		أرجع strdup("UTF-8");
	}

	/* dispense with temporary copy */
	حرر(لنډمهاله);

	/* according to RFC2070, the accept-charsets attribute of the
	 * form element contains a space and/or comma separated list */
	c = form->accept_charsets;

	/** \todo an improvement would be to choose an encoding
	 * acceptable to the server which covers as much of the input
	 * values as possible. Additionally, we need to handle the
	 * case where none of the acceptable encodings cover all the
	 * textual input values.  For now, we just extract the first
	 * element of the charset list
	 */
	طالما (*c && !ascii_is_space(*c)) {
		إذا (*c == ',')
			اكسر;
		c++;
	}

	أرجع strndup(form->accept_charsets, c - form->accept_charsets);
}


/**
 * Construct multipart data list from 'successful' controls via the DOM.
 *
 * All text strings in the successful controls list will be in the charset most
 * appropriate for submission. Therefore, no utf8_to_* processing should be
 * performed upon them.
 *
 * \todo The chosen charset needs to be made available such that it can be
 * included in the submission request (e.g. in the fetch's Content-Type header)
 *
 * See HTML 4.01 section 17.13.2.
 *
 * \note care is taken to abort even if the error is recoverable as it
 *       is not desirable to submit incomplete form data.
 *
 * \param[in] form form to search for successful controls
 * \param[in] submit_button control used to submit the form, if any
 * \param[out] fetch_data_out updated to point to linked list of
 *                             fetch_multipart_data, NULL if no controls
 * \return NSERROR_OK on success or appropriate error code
 */
ساكن nserror
form_dom_منزل_معلومات(بنية form *form,
		 بنية form_control *submit_control,
		 بنية fetch_multipart_معلومات **fetch_معلومات_بهر)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception توانیز_تابع; /* the result from DOM operations */
	dom_html_collection *elements = عدم; /* the dom form elements */
	uint32_t element_شمېر; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_غوټه *element = عدم; /* the DOM form element */
	dom_string *nodename = عدم; /* the DOM node name of the element */
	بنية fetch_multipart_معلومات *fetch_معلومات = عدم; /* fetch data list */
	بنية fetch_multipart_معلومات **fetch_معلومات_بل = &fetch_معلومات;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	إذا (submit_control != عدم) {
		submit_button = submit_control->غوټه;
	} وإلا {
		submit_button = عدم;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	إذا (charset == عدم) {
		NSLOG(netsurf, INFO, "failed to find charset");
		أرجع NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	توانیز_تابع = dom_html_form_element_get_elements(form->غوټه, &elements);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		حرر(charset);
		أرجع NSERROR_DOM;
	}

	توانیز_تابع = dom_html_collection_get_اوږدتیا(elements, &element_شمېر);
	إذا (توانیز_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		اذهب form_dom_منزل_معلومات_error;
	}

	لكل (element_idx = 0; element_idx < element_شمېر; element_idx++) {
		/* obtain a form element */
		توانیز_تابع = dom_html_collection_توکی(elements, element_idx, &element);
		إذا (توانیز_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, توانیز_تابع);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_معلومات_error;
		}

		/* node name from element */
		توانیز_تابع = dom_غوټه_get_غوټه_name(element, &nodename);
		إذا (توانیز_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, توانیز_تابع);
			dom_غوټه_unref(element);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_معلومات_error;
		}

		إذا (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_منزل_معلومات_textarea(
				(dom_html_متن_area_element *)element,
				charset,
				form->document_charset,
				&fetch_معلومات_بل);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_منزل_معلومات_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_معلومات_بل);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_ننوت)) {
			/* Form element is HTMLInputElement */
			res = form_dom_منزل_معلومات_ننوت(
				(dom_html_ننوت_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_معلومات_بل);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_منزل_معلومات_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_معلومات_بل);

		} وإلا {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحيح)dom_string_byte_اوږدتیا(nodename),
			      dom_string_معلومات(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_غوټه_unref(element);

		/* abort form element enumeration on error */
		إذا (res != NSERROR_OK) {
			اذهب form_dom_منزل_معلومات_error;
		}
	}

	*fetch_معلومات_بهر = fetch_معلومات;
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع NSERROR_OK;

form_dom_منزل_معلومات_error:
	fetch_multipart_معلومات_destroy(fetch_معلومات);
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ساكن nserror
form_url_encode(بنية form *form,
		بنية fetch_multipart_معلومات *control,
		حرف **encoded_بهر)
{
	حرف *name, *ارزښت;
	حرف *s, *s2;
	بینښه صحيح len, len1, len_init;
	nserror res;

	s = حافظه_ورکړه(1);

	إذا (s == عدم) {
		أرجع NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	لكل (; control; control = control->بل) {
		res = url_escape(control->name, true, عدم, &name);
		إذا (res != NSERROR_OK) {
			حرر(s);
			أرجع res;
		}

		res = url_escape(control->ارزښت, true, عدم, &ارزښت);
		إذا (res != NSERROR_OK) {
			حرر(name);
			حرر(s);
			أرجع res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + مزي_اوږدوالی(name) + مزي_اوږدوالی(ارزښت) + 2;
		s2 = realloc(s, len1 + 1);
		إذا (s2 == عدم) {
			حرر(ارزښت);
			حرر(name);
			حرر(s);
			أرجع NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, ارزښت);
		len = len1;
		حرر(name);
		حرر(ارزښت);
	}

	إذا (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_بهر = s;

	أرجع NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساكن فراغ
form_select_menu_scroll_callback(فراغ *client_معلومات,
				 بنية scrollbar_پیغام_معلومات *scrollbar_معلومات)
{
	بنية form_control *control = client_معلومات;
	بنية form_select_menu *menu = control->معلومات.select.menu;
	html_content *html = (html_content *)menu->c;

	بدل (scrollbar_معلومات->پیغام) {
		حالة SCROLLBAR_پیغام_MOVED:
			menu->callback(menu->client_معلومات,
					0, 0,
					menu->پلنوالی,
					menu->لوړوالی);
			اكسر;
		حالة SCROLLBAR_پیغام_SCROLL_پیل:
		{
			بنية rect rect = {
				.x0 = scrollbar_معلومات->x0,
				.y0 = scrollbar_معلومات->y0,
				.x1 = scrollbar_معلومات->x1,
				.y1 = scrollbar_معلومات->y1
			};

			browser_کړکۍ_سېټ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			اكسر;
		حالة SCROLLBAR_پیغام_SCROLL_پای_ته_رسېدلی:
			menu->scroll_capture = false;

			browser_کړکۍ_سېټ_drag_type(html->bw,
					DRAGGING_NONE, عدم);
			اكسر;
		افتراضي:
			اكسر;
	}
}


/**
 * Process a selection from a form select menu.
 *
 * \param  html The html content handle for the form
 * \param  control  form control with menu
 * \param  item	    index of item selected from the menu
 * \return NSERROR_OK or appropriate error code.
 */
ساكن nserror
form__select_process_selection(html_content *html,
			       بنية form_control *control,
			       صحيح توکی)
{
	بنية box *inline_box;
	بنية form_option *o;
	صحيح شمېر;
	nserror ret = NSERROR_OK;

	أكد(control != عدم);
	أكد(html != عدم);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	لكل (شمېر = 0, o = control->معلومات.select.توکي;
			o != عدم;
			شمېر++, o = o->بل) {
		إذا (!control->معلومات.select.مضرب && o->selected) {
			o->selected = false;
			dom_html_option_element_سېټ_selected(o->غوټه, false);
		}

		إذا (شمېر == توکی) {
			إذا (control->معلومات.select.مضرب) {
				إذا (o->selected) {
					o->selected = false;
					dom_html_option_element_سېټ_selected(
							o->غوټه, false);
					control->معلومات.select.عدد_selected--;
				} وإلا {
					o->selected = true;
					dom_html_option_element_سېټ_selected(
							o->غوټه, true);
					control->معلومات.select.عدد_selected++;
				}
			} وإلا {
				dom_html_option_element_سېټ_selected(
						o->غوټه, true);
				o->selected = true;
			}
		}

		إذا (o->selected) {
			control->معلومات.select.current = o;
		}
	}

	talloc_free(inline_box->متن);
	inline_box->متن = 0;

	إذا (control->معلومات.select.عدد_selected == 0) {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وإلا إذا (control->معلومات.select.عدد_selected == 1) {
		inline_box->متن = talloc_strdup(html->bctx,
				control->معلومات.select.current->متن);
	} وإلا {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	إذا (!inline_box->متن) {
		ret = NSERROR_NOMEM;
		inline_box->اوږدتیا = 0;
	} وإلا {
		inline_box->اوږدتیا = مزي_اوږدوالی(inline_box->متن);
	}
	inline_box->پلنوالی = control->box->پلنوالی;

	html__redraw_a_box(html, control->box);

	أرجع ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ساكن فراغ form_select_menu_clicked(بنية form_control *control, صحيح x, صحيح y)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	بنية form_option *option;
	html_content *html = (html_content *)menu->c;
	صحيح کرښه_لوړوالی, کرښه_لوړوالی_with_spacing;
	صحيح توکی_bottom_y;
	صحيح scroll, i;

	scroll = scrollbar_get_دننه_واټن(menu->scrollbar);

	کرښه_لوړوالی = menu->کرښه_لوړوالی;
	کرښه_لوړوالی_with_spacing = کرښه_لوړوالی +
			کرښه_لوړوالی * SELECT_کرښه_SPACING;

	option = control->معلومات.select.توکي;
	توکی_bottom_y = کرښه_لوړوالی_with_spacing;
	i = 0;
	طالما (option && توکی_bottom_y < scroll + y) {
		توکی_bottom_y += کرښه_لوړوالی_with_spacing;
		option = option->بل;
		i++;
	}

	إذا (option != عدم) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_معلومات, 0, 0, menu->پلنوالی, menu->لوړوالی);
}


/* exported interface documented in html/form_internal.h */
فراغ form_جمع_کړه_control(بنية form *form, بنية form_control *control)
{
	إذا (form == عدم) {
		أرجع;
	}

	control->form = form;

	إذا (form->controls != عدم) {
		أكد(form->وروستی_control);

		form->وروستی_control->بل = control;
		control->prev = form->وروستی_control;
		control->بل = عدم;
		form->وروستی_control = control;
	} وإلا {
		form->controls = form->وروستی_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_control(بنية form_control *control)
{
	بنية form_control *c;
	أكد(control != عدم);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->ارزښت, control->initial_ارزښت);
	حرر(control->name);
	حرر(control->ارزښت);
	حرر(control->initial_ارزښت);
	إذا (control->وروستی_synced_ارزښت != عدم) {
		حرر(control->وروستی_synced_ارزښت);
	}

	إذا (control->type == GADGET_SELECT) {
		بنية form_option *option, *بل;

		لكل (option = control->معلومات.select.توکي; option;
				option = بل) {
			بل = option->بل;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->متن, option->ارزښت);
			حرر(option->متن);
			حرر(option->ارزښت);
			حرر(option);
		}
		إذا (control->معلومات.select.menu != عدم) {
			form_free_select_menu(control);
		}
	}

	إذا (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		إذا (control->معلومات.متن.initial != عدم) {
			dom_string_unref(control->معلومات.متن.initial);
		}

		إذا (control->معلومات.متن.ta != عدم) {
			textarea_destroy(control->معلومات.متن.ta);
		}
	}

	/* unlink the control from the form */
	إذا (control->form != عدم) {
		لكل (c = control->form->controls; c != عدم; c = c->بل) {
			إذا (c->بل == control) {
				c->بل = control->بل;
				إذا (control->form->وروستی_control == control)
					control->form->وروستی_control = c;
				اكسر;
			}
			إذا (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->بل;
				إذا (control->form->وروستی_control == control)
					control->form->controls =
						control->form->وروستی_control = عدم;
				اكسر;
			}
		}
	}

	إذا (control->غوټه_ارزښت != عدم) {
		dom_string_unref(control->غوټه_ارزښت);
	}

	حرر(control);
}


/* exported interface documented in html/form_internal.h */
bool form_جمع_کړه_option(بنية form_control *control, حرف *ارزښت, حرف *متن,
		     bool selected, فراغ *غوټه)
{
	بنية form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = حافظه_صفر_ورکړه(1, حجم *option);
	إذا (!option)
		أرجع false;

	option->ارزښت = ارزښت;
	option->متن = متن;

	/* add to linked list */
	إذا (control->معلومات.select.توکي == 0)
		control->معلومات.select.توکي = option;
	وإلا
		control->معلومات.select.وروستی_توکی->بل = option;
	control->معلومات.select.وروستی_توکی = option;

	/* set selected */
	إذا (selected && (control->معلومات.select.عدد_selected == 0 ||
			control->معلومات.select.مضرب)) {
		option->selected = option->initial_selected = true;
		control->معلومات.select.عدد_selected++;
		control->معلومات.select.current = option;
	}

	control->معلومات.select.عدد_توکي++;

	option->غوټه = غوټه;

	أرجع true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(فراغ *client_معلومات,
		      بنية form_control *control,
		      select_menu_redraw_callback callback,
		      بنية content *c)
{
	صحيح کرښه_لوړوالی_with_spacing;
	بنية box *box;
	plot_font_style_t fstyle;
	صحيح ټول_لوړوالی;
	بنية form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	إذا (control->معلومات.select.menu == عدم) {

		menu = حافظه_صفر_ورکړه(1, حجم (بنية form_select_menu));
		إذا (menu == عدم) {
			أرجع NSERROR_NOMEM;
		}

		control->معلومات.select.menu = menu;

		box = control->box;

		menu->پلنوالی = box->پلنوالی +
			box->border[ښی_2].پلنوالی + box->padding[ښی_2] +
			box->border[کیڼ_2].پلنوالی + box->padding[کیڼ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_اندازه = fstyle.اندازه;

		menu->کرښه_لوړوالی = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.اندازه / PLOT_STYLE_SCALE)))),
				F_72));

		کرښه_لوړوالی_with_spacing = menu->کرښه_لوړوالی +
				menu->کرښه_لوړوالی *
				SELECT_کرښه_SPACING;

		ټول_لوړوالی = control->معلومات.select.عدد_توکي *
				کرښه_لوړوالی_with_spacing;
		menu->لوړوالی = ټول_لوړوالی;

		إذا (menu->لوړوالی > MAX_SELECT_لوړوالی) {
			menu->لوړوالی = MAX_SELECT_لوړوالی;
		}

		menu->client_معلومات = client_معلومات;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->لوړوالی,
				       ټول_لوړوالی,
				       menu->لوړوالی,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		إذا (res != NSERROR_OK) {
			control->معلومات.select.menu = عدم;
			حرر(menu);
			أرجع res;
		}
		menu->c = c;
	} وإلا {
		menu = control->معلومات.select.menu;
	}

	menu->callback(client_معلومات, 0, 0, menu->پلنوالی, menu->لوړوالی);

	أرجع NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_select_menu(بنية form_control *control)
{
	إذا (control->معلومات.select.menu->scrollbar != عدم)
		scrollbar_destroy(control->معلومات.select.menu->scrollbar);
	حرر(control->معلومات.select.menu);
	control->معلومات.select.menu = عدم;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(بنية form_control *control,
			صحيح x, صحيح y,
			عائم scale,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx)
{
	بنية box *box;
	بنية form_select_menu *menu = control->معلومات.select.menu;
	بنية form_option *option;
	صحيح کرښه_لوړوالی, کرښه_لوړوالی_with_spacing;
	صحيح پلنوالی, لوړوالی;
	صحيح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحيح توکی_y;
	صحيح متن_pos_دننه_واټن, متن_x;
	صحيح scrollbar_پلنوالی = SCROLLBAR_پلنوالی;
	صحيح i;
	صحيح scroll;
	صحيح x_cp, y_cp;
	بنية rect r;
	بنية rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	پلنوالی = menu->پلنوالی;
	لوړوالی = menu->لوړوالی;
	کرښه_لوړوالی = menu->کرښه_لوړوالی;

	کرښه_لوړوالی_with_spacing = کرښه_لوړوالی +
			کرښه_لوړوالی * SELECT_کرښه_SPACING;
	scroll = scrollbar_get_دننه_واټن(menu->scrollbar);

	إذا (scale != 1.0) {
		x *= scale;
		y *= scale;
		پلنوالی *= scale;
		لوړوالی *= scale;
		scrollbar_پلنوالی *= scale;

		i = scroll / کرښه_لوړوالی_with_spacing;
		scroll -= i * کرښه_لوړوالی_with_spacing;
		کرښه_لوړوالی *= scale;
		کرښه_لوړوالی_with_spacing *= scale;
		scroll *= scale;
		scroll += i * کرښه_لوړوالی_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + پلنوالی - 1;
	y1 = y + لوړوالی - 1;
	scrollbar_x = x1 - scrollbar_پلنوالی;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	x0 = x0 + SELECT_BORDER_پلنوالی;
	y0 = y0 + SELECT_BORDER_پلنوالی;
	x1 = x1 - SELECT_BORDER_پلنوالی;
	y1 = y1 - SELECT_BORDER_پلنوالی;
	لوړوالی = لوړوالی - 2 * SELECT_BORDER_پلنوالی;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	option = control->معلومات.select.توکي;
	توکی_y = کرښه_لوړوالی_with_spacing;

	طالما (توکی_y < scroll) {
		option = option->بل;
		توکی_y += کرښه_لوړوالی_with_spacing;
	}
	توکی_y -= کرښه_لوړوالی_with_spacing;
	متن_pos_دننه_واټن = y - scroll +
			(صحيح) (کرښه_لوړوالی * (0.75 + SELECT_کرښه_SPACING));
	متن_x = x + (box->border[کیڼ_2].پلنوالی + box->padding[کیڼ_2]) * scale;

	plot_fstyle_ثبت.اندازه = menu->f_اندازه;

	طالما (option && توکی_y - scroll < لوړوالی) {

		إذا (option->selected) {
			y2 = y + توکی_y - scroll;
			y3 = y + توکی_y + کرښه_لوړوالی_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}

		y2 = متن_pos_دننه_واټن + توکی_y;
		res = ctx->plot->متن(ctx,
				      &plot_fstyle_ثبت,
				      متن_x, y2,
				      option->متن, مزي_اوږدوالی(option->متن));
		إذا (res != NSERROR_OK) {
			أرجع false;
		}

		توکی_y += کرښه_لوړوالی_with_spacing;
		option = option->بل;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->پلنوالی - SCROLLBAR_پلنوالی,
			       y_cp,
			       clip, scale, ctx);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	أرجع true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(بنية form_control *control,
			     عائم scale,
			     ثابت بنية rect *clip)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	صحيح پلنوالی, لوړوالی;


	پلنوالی = menu->پلنوالی;
	لوړوالی = menu->لوړوالی;

	إذا (scale != 1.0) {
		پلنوالی *= scale;
		لوړوالی *= scale;
	}

	إذا (clip->x0 >= 0 &&
	    clip->x1 <= پلنوالی &&
	    clip->y0 >= 0 &&
	    clip->y1 <= لوړوالی)
		أرجع true;

	أرجع false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(بنية form_control *control, صحيح توکی)
{
	أكد(control != عدم);

	أرجع form__select_process_selection(control->html, control, توکی);
}


/* exported interface documented in netsurf/form.h */
بنية form_option *
form_select_get_option(بنية form_control *control, صحيح توکی)
{
	بنية form_option *opt;

	opt = control->معلومات.select.توکي;
	طالما ((opt != عدم) && (توکی > 0)) {
		opt = opt->بل;
		توکی--;
	}
	أرجع opt;
}


/* exported interface documented in netsurf/form.h */
حرف *form_control_get_name(بنية form_control *control)
{
	أرجع control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(بنية form_control *control, بنية rect *r)
{
	box_bounds( control->box, r );
	أرجع NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ثابت حرف *
form_select_mouse_action(بنية form_control *control,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	صحيح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *حالت = عدم;
	bool مضرب = control->معلومات.select.مضرب;

	x0 = 0;
	y0 = 0;
	x1 = menu->پلنوالی;
	y1 = menu->لوړوالی;
	scrollbar_x = x1 - SCROLLBAR_پلنوالی;

	إذا (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		أرجع scrollbar_mouse_حالت_منزل_پیغام(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	إذا (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		إذا (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		إذا (!(mouse & BROWSER_MOUSE_CLICK_1 && !مضرب))
			/* anything but a button 1 click over a single select
			   menu */
			حالت = messages_get(control->معلومات.select.مضرب ?
					"SelectMClick" : "SelectClick");

	} وإلا إذا (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		حالت = messages_get("SelectClose");

	أرجع حالت;
}


/* private interface described in html/form_internal.h */
فراغ
form_select_mouse_drag_پای_ټکی(بنية form_control *control,
			   browser_mouse_state mouse,
			   صحيح x, صحيح y)
{
	صحيح x0, y0, x1, y1;
	صحيح box_x, box_y;
	بنية box *box;
	بنية form_select_menu *menu = control->معلومات.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[کیڼ_2].پلنوالی;
	box_y += box->لوړوالی + box->border[BOTTOM].پلنوالی +
			box->padding[BOTTOM] + box->padding[سر_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	إذا (menu->scroll_capture) {
		x -= menu->پلنوالی - SCROLLBAR_پلنوالی;
		scrollbar_mouse_drag_پای_ټکی(menu->scrollbar, mouse, x, y);
		أرجع;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->پلنوالی;
	y1 = menu->لوړوالی;


	إذا (x > x0 && x < x1 - SCROLLBAR_پلنوالی && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *پلنوالی, صحيح *لوړوالی)
{
	*پلنوالی = control->معلومات.select.menu->پلنوالی;
	*لوړوالی = control->معلومات.select.menu->لوړوالی;
}


/* private interface described in html/form_internal.h */
فراغ form_select_menu_callback(فراغ *client_معلومات,
		صحيح x, صحيح y, صحيح پلنوالی, صحيح لوړوالی)
{
	html_content *html = client_معلومات;
	صحيح menu_x, menu_y;
	بنية box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[کیڼ_2].پلنوالی;
	menu_y += box->لوړوالی + box->border[BOTTOM].پلنوالی +
			box->padding[BOTTOM] +
			box->padding[سر_2];
	content__request_redraw((بنية content *)html, menu_x + x, menu_y + y,
			پلنوالی, لوړوالی);
}


/* private interface described in html/form_internal.h */
فراغ form_radio_سېټ(بنية form_control *radio)
{
	بنية form_control *control;

	أكد(radio);
	إذا (!radio->form)
		أرجع;

	إذا (radio->selected)
		أرجع;

	/* Clear selected state for other controls in
	 * the same radio button group */
	لكل (control = radio->form->controls;
	     control != عدم;
	     control = control->بل) {
		/* Only interested in radio inputs */
		إذا (control->type != GADGET_RADIO)
			استمر;

		/* Ignore ourself */
		إذا (control == radio)
			استمر;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		إذا ((control->name == عدم) ||
		    (radio->name == عدم) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			استمر;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		إذا (control->selected) {
			control->selected = false;
			dom_html_ننوت_element_سېټ_checked(control->غوټه, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ننوت_element_سېټ_checked(radio->غوټه, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *پاڼه_url,
	    بنية browser_کړکۍ *موخه,
	    بنية form *form,
	    بنية form_control *submit_button)
{
	nserror res;
	حرف *معلومات = عدم; /* encoded form data */
	بنية fetch_multipart_معلومات *success = عدم; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != عدم);

	/* obtain list of controls from DOM */
	res = form_dom_منزل_معلومات(form, submit_button, &success);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	إذا (res != NSERROR_OK) {
		fetch_multipart_معلومات_destroy(success);
		أرجع res;
	}

	بدل (form->لاره) {
	حالة لاره_GET:
		res = form_url_encode(form, success, &معلومات);
		إذا (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, معلومات, &query_url);
			إذا (res == NSERROR_OK) {
				res = browser_کړکۍ_navigate(موخه,
							      query_url,
							      پاڼه_url,
							      BW_NAVIGATE_HISTORY,
							      عدم,
							      عدم,
							      عدم);

				nsurl_unref(query_url);
			}
			حرر(معلومات);
		}
		اكسر;

	حالة لاره_POST_URLENC:
		res = form_url_encode(form, success, &معلومات);
		إذا (res == NSERROR_OK) {
			res = browser_کړکۍ_navigate(موخه,
						      action_url,
						      پاڼه_url,
						      BW_NAVIGATE_HISTORY,
						      معلومات,
						      عدم,
						      عدم);
			حرر(معلومات);
		}
		اكسر;

	حالة لاره_POST_MULTIPART:
		res = browser_کړکۍ_navigate(موخه,
					      action_url,
					      پاڼه_url,
					      BW_NAVIGATE_HISTORY,
					      عدم,
					      success,
					      عدم);

		اكسر;
	}

	nsurl_unref(action_url);
	fetch_multipart_معلومات_destroy(success);

	أرجع res;
}


/* exported interface documented in html/form_internal.h */
فراغ form_gadget_update_ارزښت(بنية form_control *control, حرف *ارزښت)
{
	بدل (control->type) {
	حالة GADGET_HIDDEN:
	حالة GADGET_TEXTBOX:
	حالة GADGET_TEXTAREA:
	حالة GADGET_PASSWORD:
	حالة GADGET_FILE:
		إذا (control->ارزښت != عدم) {
			حرر(control->ارزښت);
		}
		control->ارزښت = ارزښت;
		إذا (control->غوټه != عدم) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)ارزښت,
						مزي_اوږدوالی(ارزښت), &str);
			إذا (err == DOM_NO_ERR) {
				إذا (control->type == GADGET_TEXTAREA)
					err = dom_html_متن_area_element_سېټ_ارزښت(
						(dom_html_متن_area_element *)(control->غوټه),
						str);
				وإلا
					err = dom_html_ننوت_element_سېټ_ارزښت(
						(dom_html_ننوت_element *)(control->غوټه),
						str);
				dom_string_unref(str);
			}
		}
		اكسر;
	افتراضي:
		/* Do nothing */
		اكسر;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
فراغ
form_gadget_sync_with_dom(بنية form_control *control)
{
	dom_exception exc;
	dom_string *ارزښت = عدم;
	bool بدل_شوی_dom = false;

	إذا (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		أرجع;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	إذا ((control->وروستی_synced_ارزښت == عدم &&
	     control->ارزښت != عدم &&
	     control->ارزښت[0] != '\0') ||
	    (control->وروستی_synced_ارزښت != عدم &&
	     control->ارزښت != عدم &&
	     strcmp(control->ارزښت, control->وروستی_synced_ارزښت) != 0)) {
		حرف *dup = strdup(control->ارزښت);
		إذا (dup == عدم) {
			اذهب بهر;
		}
		إذا (control->وروستی_synced_ارزښت != عدم) {
			حرر(control->وروستی_synced_ارزښت);
		}
		control->وروستی_synced_ارزښت = dup;
		exc = dom_string_create((uint8_t *)(control->ارزښت),
					مزي_اوږدوالی(control->ارزښت), &ارزښت);
		إذا (exc != DOM_NO_ERR) {
			اذهب بهر;
		}
		إذا (control->غوټه_ارزښت != عدم) {
			dom_string_unref(control->غوټه_ارزښت);
		}
		control->غوټه_ارزښت = ارزښت;
		ارزښت = عدم;
		إذا (control->type == GADGET_TEXTAREA) {
			exc = dom_html_متن_area_element_سېټ_ارزښت(control->غوټه, control->غوټه_ارزښت);
		} وإلا {
			exc = dom_html_ننوت_element_سېټ_ارزښت(control->غوټه, control->غوټه_ارزښت);
		}
		إذا (exc != DOM_NO_ERR) {
			اذهب بهر;
		}
		بدل_شوی_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	إذا (control->type == GADGET_TEXTAREA) {
		exc = dom_html_متن_area_element_get_ارزښت(control->غوټه, &ارزښت);
	} وإلا {
		exc = dom_html_ننوت_element_get_ارزښت(control->غوټه, &ارزښت);
	}

	إذا (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		اذهب بهر;
	}

	إذا (!dom_string_isequal(control->غوټه_ارزښت, ارزښت)) {
		/* The DOM has changed */
		إذا (!بدل_شوی_dom) {
			/* And it wasn't us */
			حرف *ارزښت_s = strndup(
				dom_string_معلومات(ارزښت),
				dom_string_byte_اوږدتیا(ارزښت));
			حرف *dup = عدم;
			إذا (ارزښت_s == عدم) {
				اذهب بهر;
			}
			dup = strdup(ارزښت_s);
			إذا (dup == عدم) {
				حرر(ارزښت_s);
				اذهب بهر;
			}
			حرر(control->ارزښت);
			control->ارزښت = ارزښت_s;
			حرر(control->وروستی_synced_ارزښت);
			control->وروستی_synced_ارزښت = dup;
			إذا (control->type != GADGET_HIDDEN &&
			    control->معلومات.متن.ta != عدم) {
				textarea_سېټ_متن(control->معلومات.متن.ta,
						  ارزښت_s);
			}
		}
		control->غوټه_ارزښت = ارزښت;
		ارزښت = عدم;
	}

بهر:
	إذا (ارزښت != عدم)
		dom_string_unref(ارزښت);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
بنية form *
form_new(فراغ *غوټه,
	 ثابت حرف *action,
	 ثابت حرف *موخه,
	 form_لاره لاره,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	بنية form *form;

	form = حافظه_صفر_ورکړه(1, حجم *form);
	إذا (!form)
		أرجع عدم;

	form->action = strdup(action != عدم ? action : "");
	إذا (form->action == عدم) {
		حرر(form);
		أرجع عدم;
	}

	form->موخه = موخه != عدم ? strdup(موخه) : عدم;
	إذا (موخه != عدم && form->موخه == عدم) {
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->لاره = لاره;

	form->accept_charsets = charset != عدم ? strdup(charset) : عدم;
	إذا (charset != عدم && form->accept_charsets == عدم) {
		حرر(form->موخه);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->document_charset = doc_charset != عدم ? strdup(doc_charset)
						     : عدم;
	إذا (doc_charset && form->document_charset == عدم) {
		حرر(form->accept_charsets);
		حرر(form->موخه);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->غوټه = غوټه;

	أرجع form;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free(بنية form *form)
{
	بنية form_control *c, *d;

	لكل (c = form->controls; c != عدم; c = d) {
		d = c->بل;

		form_free_control(c);
	}

	حرر(form->action);
	حرر(form->موخه);
	حرر(form->accept_charsets);
	حرر(form->document_charset);

	حرر(form);
}


/* exported interface documented in html/form_internal.h */
بنية form_control *form_new_control(فراغ *غوټه, form_control_type type)
{
	بنية form_control *control;

	control = حافظه_صفر_ورکړه(1, حجم *control);
	إذا (control == عدم)
		أرجع عدم;

	control->غوټه = غوټه;
	control->type = type;

	أرجع control;
}
