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

#تعریف_کن MAX_SELECT_ارتفاع 210
#تعریف_کن SELECT_سطر_SPACING 0.2
#تعریف_کن SELECT_BORDER_عرض 1
#تعریف_کن SELECT_SELECTED_رنگ 0xDB9370

بنية form_select_menu {
	صحيح سطر_ارتفاع;
	صحيح عرض, ارتفاع;
	بنية scrollbar *scrollbar;
	صحيح f_اندازه;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	فراغ *client_داده;
	بنية content *c;
};

ساكن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_رنگ = SELECT_SELECTED_رنگ,
};

ساكن plot_font_style_t plot_fstyle_مدخل = {
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
form_encode_قلم(ثابت حرف *قلم,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = عدم;
	حرف cset[256];

	إذا (!قلم || !charset)
		أرجع عدم;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_مقصد_enc(قلم, cset, 0, &ret);
	إذا (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_مقصد_enc(قلم, cset, len, &ret);

		إذا (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			إذا (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_مقصد_enc(قلم, cset, 0, &ret);

				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_مقصد_enc(قلم, cset, 0, &ret);
				}
			}

			إذا (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_مقصد_enc(قلم, "ISO-8859-1//TRANSLIT",
						0, &ret);
				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_مقصد_enc(قلم, "ISO-8859-1",
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
#تعریف_کن FETCH_داده_INT_مقدار_اندازه 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساكن nserror
fetch_داده_list_جمع_کن_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحيح مقدار,
			  بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	بنية fetch_multipart_داده *fetch_داده;
	صحيح keysize;

	fetch_داده = حافظه_صفر_بده(1, حجم(*fetch_داده));
	إذا (fetch_داده == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_داده->name, 0, "%s%s", name, ksfx);
	fetch_داده->name = حافظه_بده(keysize + 1); /* allow for null */
	إذا (fetch_داده->name == عدم) {
		حرر(fetch_داده);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_داده->name, keysize + 1, "%s%s", name, ksfx);

	/*مقدار*/
	fetch_داده->مقدار = حافظه_بده(FETCH_داده_INT_مقدار_اندازه);
	إذا (fetch_داده->مقدار == عدم) {
		حرر(fetch_داده->name);
		حرر(fetch_داده);
		NSLOG(netsurf, INFO, "value allocation failure");
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_داده->مقدار, FETCH_داده_INT_مقدار_اندازه, "%d", مقدار);

	/* link into list */
	**fetch_داده_بعدی_ptr = fetch_داده;
	*fetch_داده_بعدی_ptr = &fetch_داده->بعدی;

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
fetch_داده_list_جمع_کن(dom_string *name,
		    dom_string *مقدار,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	بنية fetch_multipart_داده *fetch_داده;

	أكد(name != عدم);

	fetch_داده = حافظه_صفر_بده(1, حجم(*fetch_داده));
	إذا (fetch_داده == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	fetch_داده->name = form_encode_قلم(dom_string_داده(name),
					    dom_string_byte_درازی(name),
					    form_charset,
					    docu_charset);
	إذا (fetch_داده->name == عدم) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		حرر(fetch_داده);
		أرجع NSERROR_NOMEM;
	}

	إذا (مقدار == عدم) {
		fetch_داده->مقدار = strdup("");
	} وإلا {
		fetch_داده->مقدار = form_encode_قلم(dom_string_داده(مقدار),
						     dom_string_byte_درازی(مقدار),
						     form_charset,
						     docu_charset);
	}
	إذا (fetch_داده->مقدار == عدم) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		حرر(fetch_داده->name);
		حرر(fetch_داده);
		أرجع NSERROR_NOMEM;
	}

	/* deal with raw file name */
	إذا (rawfile != عدم) {
		fetch_داده->file = true;
		fetch_داده->rawfile = strdup(rawfile);
		إذا (fetch_داده->rawfile == عدم) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			حرر(fetch_داده->مقدار);
			حرر(fetch_داده->name);
			حرر(fetch_داده);
			أرجع NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_داده_بعدی_ptr = fetch_داده;
	*fetch_داده_بعدی_ptr = &fetch_داده->بعدی;

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
form_dom_مقصد_داده_textarea(dom_html_متن_area_element *متن_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	dom_exception نمایی_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	نمایی_تابع = dom_html_متن_area_element_get_disabled(متن_area_element,
						      &element_disabled);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	نمایی_تابع = dom_html_متن_area_element_get_name(متن_area_element,
						  &inputname);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* obtain text area value */
	نمایی_تابع = dom_html_متن_area_element_get_مقدار(متن_area_element,
						   &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", نمایی_تابع);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(inputname,
				  inputvalue,
				  عدم,
				  form_charset,
				  doc_charset,
				  fetch_داده_بعدی_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_مقصد_داده_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	dom_string *مقدار;
	bool selected;

	نمایی_تابع = dom_html_option_element_get_selected(option_element, &selected);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		أرجع NSERROR_DOM;
	}

	إذا (!selected) {
		/* unselected options do not add fetch data entries */
		أرجع NSERROR_OK;
	}

	نمایی_تابع = dom_html_option_element_get_مقدار(option_element, &مقدار);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(keyname,
				  مقدار,
				  عدم,
				  form_charset,
				  docu_charset,
				  fetch_داده_بعدی_ptr);

	dom_string_unref(مقدار);

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
form_dom_مقصد_داده_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = عدم;
	uint32_t options_شمار;
	uint32_t option_شاخص;
	dom_گره *option_element = عدم;

	/* check if element is disabled */
	نمایی_تابع = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	نمایی_تابع = dom_html_select_element_get_name(select_element, &inputname);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get options collection */
	نمایی_تابع = dom_html_select_element_get_options(select_element, &options);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* get options collection length */
	نمایی_تابع = dom_html_options_collection_get_درازی(options, &options_شمار);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* iterate over options collection */
	لكل (option_شاخص = 0; option_شاخص < options_شمار; ++option_شاخص) {
		نمایی_تابع = dom_html_options_collection_قلم(options,
						       option_شاخص,
						       &option_element);
		إذا (نمایی_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_شاخص);
			res = NSERROR_DOM;
		} وإلا {
			res = form_dom_مقصد_داده_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_داده_بعدی_ptr);

			dom_گره_unref(option_element);
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
form_dom_مقصد_داده_درون_داد_submit(dom_html_درون_داد_element *درون_داد_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	dom_exception نمایی_تابع; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	إذا (*submit_button == عدم) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)درون_داد_element;
	} وإلا إذا (*submit_button != (dom_html_element *)درون_داد_element) {
		أرجع NSERROR_OK;
	}

	/* matched button used to submit form */
	نمایی_تابع = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_داده_بعدی_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_مقصد_داده_درون_داد_image(dom_html_درون_داد_element *درون_داد_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	بنية image_درون_داد_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	إذا (*submit_button != (dom_html_element *)درون_داد_element) {
		أرجع NSERROR_OK;
	}

	نمایی_تابع = dom_گره_get_user_داده((dom_گره *)درون_داد_element,
				     corestring_dom_ns_کلید_image_coords_گره_داده,
				     &coords);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		أرجع NSERROR_DOM;
	}

	إذا (coords == عدم) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		أرجع NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_قلم(dom_string_داده(inputname),
				    dom_string_byte_درازی(inputname),
				    charset,
				    document_charset);
	إذا (basename == عدم) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		أرجع NSERROR_NOMEM;
	}

	res = fetch_داده_list_جمع_کن_sname(basename, ".x",
					coords->x,
					fetch_داده_بعدی_ptr);

	إذا (res == NSERROR_OK) {
		res = fetch_داده_list_جمع_کن_sname(basename, ".y",
						coords->y,
						fetch_داده_بعدی_ptr);
	}

	حرر(basename);

	أرجع res;
}


ساكن nserror
form_dom_مقصد_داده_درون_داد_checkbox(dom_html_درون_داد_element *درون_داد_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	نمایی_تابع = dom_html_درون_داد_element_get_checked(درون_داد_element, &checked);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		أرجع NSERROR_DOM;
	}

	إذا (!checked) {
		/* unchecked items do not generate a data entry */
		أرجع NSERROR_OK;
	}

	نمایی_تابع = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		أرجع NSERROR_DOM;
	}

	/* ensure a default value */
	إذا (inputvalue == عدم) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_داده_بعدی_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_مقصد_داده_درون_داد_file(dom_html_درون_داد_element *درون_داد_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = عدم;

	نمایی_تابع = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		أرجع NSERROR_DOM;
	}

	نمایی_تابع = dom_گره_get_user_داده((dom_گره *)درون_داد_element,
				     corestring_dom_ns_کلید_file_name_گره_داده,
				     &rawfile);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		أرجع NSERROR_DOM;
	}

	إذا (rawfile == عدم) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_داده_بعدی_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_مقصد_داده_درون_داد_متن(dom_html_درون_داد_element *درون_داد_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	nserror res;
	dom_exception نمایی_تابع; /* the result from DOM operations */
	dom_string *inputvalue;

	نمایی_تابع = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_داده_list_جمع_کن(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_داده_بعدی_ptr);

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
form_dom_مقصد_داده_درون_داد(dom_html_درون_داد_element *درون_داد_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	dom_exception نمایی_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	نمایی_تابع = dom_html_درون_داد_element_get_disabled(درون_داد_element,
						  &element_disabled);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* disabled element requires no more processing */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	نمایی_تابع = dom_html_درون_داد_element_get_name(درون_داد_element, &inputname);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* element with no name is not converted */
		أرجع NSERROR_OK;
	}

	/* get input type */
	نمایی_تابع = dom_html_درون_داد_element_get_type(درون_داد_element, &inputtype);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* process according to input element type */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_مقصد_داده_درون_داد_submit(درون_داد_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_داده_بعدی_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_مقصد_داده_درون_داد_image(درون_داد_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_داده_بعدی_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_مقصد_داده_درون_داد_checkbox(درون_داد_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_داده_بعدی_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_مقصد_داده_درون_داد_file(درون_داد_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_داده_بعدی_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وإلا {
		/* Everything else is treated as text values */
		res = form_dom_مقصد_داده_درون_داد_متن(درون_داد_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_داده_بعدی_ptr);

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
form_dom_مقصد_داده_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			بنية fetch_multipart_داده ***fetch_داده_بعدی_ptr)
{
	dom_exception نمایی_تابع; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	نمایی_تابع = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* get the type attribute */
	نمایی_تابع = dom_html_button_element_get_type(button_element, &inputtype);
	إذا (نمایی_تابع != DOM_NO_ERR) {
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
	نمایی_تابع = dom_html_button_element_get_name(button_element, &inputname);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", نمایی_تابع);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	نمایی_تابع = dom_html_button_element_get_مقدار(button_element, &inputvalue);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} وإلا {
		res = fetch_داده_list_جمع_کن(inputname,
					  inputvalue,
					  عدم,
					  form_charset,
					  doc_charset,
					  fetch_داده_بعدی_ptr);

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
	حرف *موقت, *c;

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
	موقت = strdup(form->accept_charsets);
	إذا (!موقت)
		أرجع عدم;

	/* make it upper case */
	لكل (c = موقت; *c; c++) {
		*c = ascii_مقصد_فوقانی(*c);
	}

	/* is UTF-8 specified? */
	c = جستجوی_رشته(موقت, "UTF-8");
	إذا (c) {
		حرر(موقت);
		أرجع strdup("UTF-8");
	}

	/* dispense with temporary copy */
	حرر(موقت);

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
form_dom_مقصد_داده(بنية form *form,
		 بنية form_control *submit_control,
		 بنية fetch_multipart_داده **fetch_داده_بیرون)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception نمایی_تابع; /* the result from DOM operations */
	dom_html_collection *elements = عدم; /* the dom form elements */
	uint32_t element_شمار; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_گره *element = عدم; /* the DOM form element */
	dom_string *nodename = عدم; /* the DOM node name of the element */
	بنية fetch_multipart_داده *fetch_داده = عدم; /* fetch data list */
	بنية fetch_multipart_داده **fetch_داده_بعدی = &fetch_داده;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	إذا (submit_control != عدم) {
		submit_button = submit_control->گره;
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
	نمایی_تابع = dom_html_form_element_get_elements(form->گره, &elements);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		حرر(charset);
		أرجع NSERROR_DOM;
	}

	نمایی_تابع = dom_html_collection_get_درازی(elements, &element_شمار);
	إذا (نمایی_تابع != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		اذهب form_dom_مقصد_داده_error;
	}

	لكل (element_idx = 0; element_idx < element_شمار; element_idx++) {
		/* obtain a form element */
		نمایی_تابع = dom_html_collection_قلم(elements, element_idx, &element);
		إذا (نمایی_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, نمایی_تابع);
			res = NSERROR_DOM;
			اذهب form_dom_مقصد_داده_error;
		}

		/* node name from element */
		نمایی_تابع = dom_گره_get_گره_name(element, &nodename);
		إذا (نمایی_تابع != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, نمایی_تابع);
			dom_گره_unref(element);
			res = NSERROR_DOM;
			اذهب form_dom_مقصد_داده_error;
		}

		إذا (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_مقصد_داده_textarea(
				(dom_html_متن_area_element *)element,
				charset,
				form->document_charset,
				&fetch_داده_بعدی);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_مقصد_داده_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_داده_بعدی);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_درون_داد)) {
			/* Form element is HTMLInputElement */
			res = form_dom_مقصد_داده_درون_داد(
				(dom_html_درون_داد_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_داده_بعدی);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_مقصد_داده_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_داده_بعدی);

		} وإلا {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحيح)dom_string_byte_درازی(nodename),
			      dom_string_داده(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_گره_unref(element);

		/* abort form element enumeration on error */
		إذا (res != NSERROR_OK) {
			اذهب form_dom_مقصد_داده_error;
		}
	}

	*fetch_داده_بیرون = fetch_داده;
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع NSERROR_OK;

form_dom_مقصد_داده_error:
	fetch_multipart_داده_destroy(fetch_داده);
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
		بنية fetch_multipart_داده *control,
		حرف **encoded_بیرون)
{
	حرف *name, *مقدار;
	حرف *s, *s2;
	بیعلامت صحيح len, len1, len_init;
	nserror res;

	s = حافظه_بده(1);

	إذا (s == عدم) {
		أرجع NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	لكل (; control; control = control->بعدی) {
		res = url_escape(control->name, true, عدم, &name);
		إذا (res != NSERROR_OK) {
			حرر(s);
			أرجع res;
		}

		res = url_escape(control->مقدار, true, عدم, &مقدار);
		إذا (res != NSERROR_OK) {
			حرر(name);
			حرر(s);
			أرجع res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + طول_رشته(name) + طول_رشته(مقدار) + 2;
		s2 = realloc(s, len1 + 1);
		إذا (s2 == عدم) {
			حرر(مقدار);
			حرر(name);
			حرر(s);
			أرجع NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, مقدار);
		len = len1;
		حرر(name);
		حرر(مقدار);
	}

	إذا (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_بیرون = s;

	أرجع NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساكن فراغ
form_select_menu_scroll_callback(فراغ *client_داده,
				 بنية scrollbar_پیام_داده *scrollbar_داده)
{
	بنية form_control *control = client_داده;
	بنية form_select_menu *menu = control->داده.select.menu;
	html_content *html = (html_content *)menu->c;

	بدل (scrollbar_داده->پیام) {
		حالة SCROLLBAR_پیام_MOVED:
			menu->callback(menu->client_داده,
					0, 0,
					menu->عرض,
					menu->ارتفاع);
			اكسر;
		حالة SCROLLBAR_پیام_SCROLL_شروع:
		{
			بنية rect rect = {
				.x0 = scrollbar_داده->x0,
				.y0 = scrollbar_داده->y0,
				.x1 = scrollbar_داده->x1,
				.y1 = scrollbar_داده->y1
			};

			browser_پنجره_مجموعه_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			اكسر;
		حالة SCROLLBAR_پیام_SCROLL_پایانیافته:
			menu->scroll_capture = false;

			browser_پنجره_مجموعه_drag_type(html->bw,
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
			       صحيح قلم)
{
	بنية box *inline_box;
	بنية form_option *o;
	صحيح شمار;
	nserror ret = NSERROR_OK;

	أكد(control != عدم);
	أكد(html != عدم);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	لكل (شمار = 0, o = control->داده.select.اقلام;
			o != عدم;
			شمار++, o = o->بعدی) {
		إذا (!control->داده.select.مضرب && o->selected) {
			o->selected = false;
			dom_html_option_element_مجموعه_selected(o->گره, false);
		}

		إذا (شمار == قلم) {
			إذا (control->داده.select.مضرب) {
				إذا (o->selected) {
					o->selected = false;
					dom_html_option_element_مجموعه_selected(
							o->گره, false);
					control->داده.select.رقم_selected--;
				} وإلا {
					o->selected = true;
					dom_html_option_element_مجموعه_selected(
							o->گره, true);
					control->داده.select.رقم_selected++;
				}
			} وإلا {
				dom_html_option_element_مجموعه_selected(
						o->گره, true);
				o->selected = true;
			}
		}

		إذا (o->selected) {
			control->داده.select.current = o;
		}
	}

	talloc_free(inline_box->متن);
	inline_box->متن = 0;

	إذا (control->داده.select.رقم_selected == 0) {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وإلا إذا (control->داده.select.رقم_selected == 1) {
		inline_box->متن = talloc_strdup(html->bctx,
				control->داده.select.current->متن);
	} وإلا {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	إذا (!inline_box->متن) {
		ret = NSERROR_NOMEM;
		inline_box->درازی = 0;
	} وإلا {
		inline_box->درازی = طول_رشته(inline_box->متن);
	}
	inline_box->عرض = control->box->عرض;

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
	بنية form_select_menu *menu = control->داده.select.menu;
	بنية form_option *option;
	html_content *html = (html_content *)menu->c;
	صحيح سطر_ارتفاع, سطر_ارتفاع_with_spacing;
	صحيح قلم_bottom_y;
	صحيح scroll, i;

	scroll = scrollbar_get_جابه_جایی(menu->scrollbar);

	سطر_ارتفاع = menu->سطر_ارتفاع;
	سطر_ارتفاع_with_spacing = سطر_ارتفاع +
			سطر_ارتفاع * SELECT_سطر_SPACING;

	option = control->داده.select.اقلام;
	قلم_bottom_y = سطر_ارتفاع_with_spacing;
	i = 0;
	طالما (option && قلم_bottom_y < scroll + y) {
		قلم_bottom_y += سطر_ارتفاع_with_spacing;
		option = option->بعدی;
		i++;
	}

	إذا (option != عدم) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_داده, 0, 0, menu->عرض, menu->ارتفاع);
}


/* exported interface documented in html/form_internal.h */
فراغ form_جمع_کن_control(بنية form *form, بنية form_control *control)
{
	إذا (form == عدم) {
		أرجع;
	}

	control->form = form;

	إذا (form->controls != عدم) {
		أكد(form->آخرین_control);

		form->آخرین_control->بعدی = control;
		control->prev = form->آخرین_control;
		control->بعدی = عدم;
		form->آخرین_control = control;
	} وإلا {
		form->controls = form->آخرین_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_control(بنية form_control *control)
{
	بنية form_control *c;
	أكد(control != عدم);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->مقدار, control->initial_مقدار);
	حرر(control->name);
	حرر(control->مقدار);
	حرر(control->initial_مقدار);
	إذا (control->آخرین_synced_مقدار != عدم) {
		حرر(control->آخرین_synced_مقدار);
	}

	إذا (control->type == GADGET_SELECT) {
		بنية form_option *option, *بعدی;

		لكل (option = control->داده.select.اقلام; option;
				option = بعدی) {
			بعدی = option->بعدی;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->متن, option->مقدار);
			حرر(option->متن);
			حرر(option->مقدار);
			حرر(option);
		}
		إذا (control->داده.select.menu != عدم) {
			form_free_select_menu(control);
		}
	}

	إذا (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		إذا (control->داده.متن.initial != عدم) {
			dom_string_unref(control->داده.متن.initial);
		}

		إذا (control->داده.متن.ta != عدم) {
			textarea_destroy(control->داده.متن.ta);
		}
	}

	/* unlink the control from the form */
	إذا (control->form != عدم) {
		لكل (c = control->form->controls; c != عدم; c = c->بعدی) {
			إذا (c->بعدی == control) {
				c->بعدی = control->بعدی;
				إذا (control->form->آخرین_control == control)
					control->form->آخرین_control = c;
				اكسر;
			}
			إذا (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->بعدی;
				إذا (control->form->آخرین_control == control)
					control->form->controls =
						control->form->آخرین_control = عدم;
				اكسر;
			}
		}
	}

	إذا (control->گره_مقدار != عدم) {
		dom_string_unref(control->گره_مقدار);
	}

	حرر(control);
}


/* exported interface documented in html/form_internal.h */
bool form_جمع_کن_option(بنية form_control *control, حرف *مقدار, حرف *متن,
		     bool selected, فراغ *گره)
{
	بنية form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = حافظه_صفر_بده(1, حجم *option);
	إذا (!option)
		أرجع false;

	option->مقدار = مقدار;
	option->متن = متن;

	/* add to linked list */
	إذا (control->داده.select.اقلام == 0)
		control->داده.select.اقلام = option;
	وإلا
		control->داده.select.آخرین_قلم->بعدی = option;
	control->داده.select.آخرین_قلم = option;

	/* set selected */
	إذا (selected && (control->داده.select.رقم_selected == 0 ||
			control->داده.select.مضرب)) {
		option->selected = option->initial_selected = true;
		control->داده.select.رقم_selected++;
		control->داده.select.current = option;
	}

	control->داده.select.رقم_اقلام++;

	option->گره = گره;

	أرجع true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(فراغ *client_داده,
		      بنية form_control *control,
		      select_menu_redraw_callback callback,
		      بنية content *c)
{
	صحيح سطر_ارتفاع_with_spacing;
	بنية box *box;
	plot_font_style_t fstyle;
	صحيح مجموع_ارتفاع;
	بنية form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	إذا (control->داده.select.menu == عدم) {

		menu = حافظه_صفر_بده(1, حجم (بنية form_select_menu));
		إذا (menu == عدم) {
			أرجع NSERROR_NOMEM;
		}

		control->داده.select.menu = menu;

		box = control->box;

		menu->عرض = box->عرض +
			box->border[راست_2].عرض + box->padding[راست_2] +
			box->border[چپ_2].عرض + box->padding[چپ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_اندازه = fstyle.اندازه;

		menu->سطر_ارتفاع = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.اندازه / PLOT_STYLE_SCALE)))),
				F_72));

		سطر_ارتفاع_with_spacing = menu->سطر_ارتفاع +
				menu->سطر_ارتفاع *
				SELECT_سطر_SPACING;

		مجموع_ارتفاع = control->داده.select.رقم_اقلام *
				سطر_ارتفاع_with_spacing;
		menu->ارتفاع = مجموع_ارتفاع;

		إذا (menu->ارتفاع > MAX_SELECT_ارتفاع) {
			menu->ارتفاع = MAX_SELECT_ارتفاع;
		}

		menu->client_داده = client_داده;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->ارتفاع,
				       مجموع_ارتفاع,
				       menu->ارتفاع,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		إذا (res != NSERROR_OK) {
			control->داده.select.menu = عدم;
			حرر(menu);
			أرجع res;
		}
		menu->c = c;
	} وإلا {
		menu = control->داده.select.menu;
	}

	menu->callback(client_داده, 0, 0, menu->عرض, menu->ارتفاع);

	أرجع NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_select_menu(بنية form_control *control)
{
	إذا (control->داده.select.menu->scrollbar != عدم)
		scrollbar_destroy(control->داده.select.menu->scrollbar);
	حرر(control->داده.select.menu);
	control->داده.select.menu = عدم;
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
	بنية form_select_menu *menu = control->داده.select.menu;
	بنية form_option *option;
	صحيح سطر_ارتفاع, سطر_ارتفاع_with_spacing;
	صحيح عرض, ارتفاع;
	صحيح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحيح قلم_y;
	صحيح متن_pos_جابه_جایی, متن_x;
	صحيح scrollbar_عرض = SCROLLBAR_عرض;
	صحيح i;
	صحيح scroll;
	صحيح x_cp, y_cp;
	بنية rect r;
	بنية rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	عرض = menu->عرض;
	ارتفاع = menu->ارتفاع;
	سطر_ارتفاع = menu->سطر_ارتفاع;

	سطر_ارتفاع_with_spacing = سطر_ارتفاع +
			سطر_ارتفاع * SELECT_سطر_SPACING;
	scroll = scrollbar_get_جابه_جایی(menu->scrollbar);

	إذا (scale != 1.0) {
		x *= scale;
		y *= scale;
		عرض *= scale;
		ارتفاع *= scale;
		scrollbar_عرض *= scale;

		i = scroll / سطر_ارتفاع_with_spacing;
		scroll -= i * سطر_ارتفاع_with_spacing;
		سطر_ارتفاع *= scale;
		سطر_ارتفاع_with_spacing *= scale;
		scroll *= scale;
		scroll += i * سطر_ارتفاع_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + عرض - 1;
	y1 = y + ارتفاع - 1;
	scrollbar_x = x1 - scrollbar_عرض;

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

	x0 = x0 + SELECT_BORDER_عرض;
	y0 = y0 + SELECT_BORDER_عرض;
	x1 = x1 - SELECT_BORDER_عرض;
	y1 = y1 - SELECT_BORDER_عرض;
	ارتفاع = ارتفاع - 2 * SELECT_BORDER_عرض;

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

	option = control->داده.select.اقلام;
	قلم_y = سطر_ارتفاع_with_spacing;

	طالما (قلم_y < scroll) {
		option = option->بعدی;
		قلم_y += سطر_ارتفاع_with_spacing;
	}
	قلم_y -= سطر_ارتفاع_with_spacing;
	متن_pos_جابه_جایی = y - scroll +
			(صحيح) (سطر_ارتفاع * (0.75 + SELECT_سطر_SPACING));
	متن_x = x + (box->border[چپ_2].عرض + box->padding[چپ_2]) * scale;

	plot_fstyle_مدخل.اندازه = menu->f_اندازه;

	طالما (option && قلم_y - scroll < ارتفاع) {

		إذا (option->selected) {
			y2 = y + قلم_y - scroll;
			y3 = y + قلم_y + سطر_ارتفاع_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}

		y2 = متن_pos_جابه_جایی + قلم_y;
		res = ctx->plot->متن(ctx,
				      &plot_fstyle_مدخل,
				      متن_x, y2,
				      option->متن, طول_رشته(option->متن));
		إذا (res != NSERROR_OK) {
			أرجع false;
		}

		قلم_y += سطر_ارتفاع_with_spacing;
		option = option->بعدی;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->عرض - SCROLLBAR_عرض,
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
	بنية form_select_menu *menu = control->داده.select.menu;
	صحيح عرض, ارتفاع;


	عرض = menu->عرض;
	ارتفاع = menu->ارتفاع;

	إذا (scale != 1.0) {
		عرض *= scale;
		ارتفاع *= scale;
	}

	إذا (clip->x0 >= 0 &&
	    clip->x1 <= عرض &&
	    clip->y0 >= 0 &&
	    clip->y1 <= ارتفاع)
		أرجع true;

	أرجع false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(بنية form_control *control, صحيح قلم)
{
	أكد(control != عدم);

	أرجع form__select_process_selection(control->html, control, قلم);
}


/* exported interface documented in netsurf/form.h */
بنية form_option *
form_select_get_option(بنية form_control *control, صحيح قلم)
{
	بنية form_option *opt;

	opt = control->داده.select.اقلام;
	طالما ((opt != عدم) && (قلم > 0)) {
		opt = opt->بعدی;
		قلم--;
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
	بنية form_select_menu *menu = control->داده.select.menu;
	صحيح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *وضعیت = عدم;
	bool مضرب = control->داده.select.مضرب;

	x0 = 0;
	y0 = 0;
	x1 = menu->عرض;
	y1 = menu->ارتفاع;
	scrollbar_x = x1 - SCROLLBAR_عرض;

	إذا (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		أرجع scrollbar_mouse_وضعیت_مقصد_پیام(
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
			وضعیت = messages_get(control->داده.select.مضرب ?
					"SelectMClick" : "SelectClick");

	} وإلا إذا (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		وضعیت = messages_get("SelectClose");

	أرجع وضعیت;
}


/* private interface described in html/form_internal.h */
فراغ
form_select_mouse_drag_انتها(بنية form_control *control,
			   browser_mouse_state mouse,
			   صحيح x, صحيح y)
{
	صحيح x0, y0, x1, y1;
	صحيح box_x, box_y;
	بنية box *box;
	بنية form_select_menu *menu = control->داده.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[چپ_2].عرض;
	box_y += box->ارتفاع + box->border[BOTTOM].عرض +
			box->padding[BOTTOM] + box->padding[بالاترین_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	إذا (menu->scroll_capture) {
		x -= menu->عرض - SCROLLBAR_عرض;
		scrollbar_mouse_drag_انتها(menu->scrollbar, mouse, x, y);
		أرجع;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->عرض;
	y1 = menu->ارتفاع;


	إذا (x > x0 && x < x1 - SCROLLBAR_عرض && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *عرض, صحيح *ارتفاع)
{
	*عرض = control->داده.select.menu->عرض;
	*ارتفاع = control->داده.select.menu->ارتفاع;
}


/* private interface described in html/form_internal.h */
فراغ form_select_menu_callback(فراغ *client_داده,
		صحيح x, صحيح y, صحيح عرض, صحيح ارتفاع)
{
	html_content *html = client_داده;
	صحيح menu_x, menu_y;
	بنية box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[چپ_2].عرض;
	menu_y += box->ارتفاع + box->border[BOTTOM].عرض +
			box->padding[BOTTOM] +
			box->padding[بالاترین_2];
	content__request_redraw((بنية content *)html, menu_x + x, menu_y + y,
			عرض, ارتفاع);
}


/* private interface described in html/form_internal.h */
فراغ form_radio_مجموعه(بنية form_control *radio)
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
	     control = control->بعدی) {
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
			dom_html_درون_داد_element_مجموعه_checked(control->گره, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_درون_داد_element_مجموعه_checked(radio->گره, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *صفحه_url,
	    بنية browser_پنجره *هدف,
	    بنية form *form,
	    بنية form_control *submit_button)
{
	nserror res;
	حرف *داده = عدم; /* encoded form data */
	بنية fetch_multipart_داده *success = عدم; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != عدم);

	/* obtain list of controls from DOM */
	res = form_dom_مقصد_داده(form, submit_button, &success);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	إذا (res != NSERROR_OK) {
		fetch_multipart_داده_destroy(success);
		أرجع res;
	}

	بدل (form->روش) {
	حالة روش_GET:
		res = form_url_encode(form, success, &داده);
		إذا (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, داده, &query_url);
			إذا (res == NSERROR_OK) {
				res = browser_پنجره_navigate(هدف,
							      query_url,
							      صفحه_url,
							      BW_NAVIGATE_HISTORY,
							      عدم,
							      عدم,
							      عدم);

				nsurl_unref(query_url);
			}
			حرر(داده);
		}
		اكسر;

	حالة روش_POST_URLENC:
		res = form_url_encode(form, success, &داده);
		إذا (res == NSERROR_OK) {
			res = browser_پنجره_navigate(هدف,
						      action_url,
						      صفحه_url,
						      BW_NAVIGATE_HISTORY,
						      داده,
						      عدم,
						      عدم);
			حرر(داده);
		}
		اكسر;

	حالة روش_POST_MULTIPART:
		res = browser_پنجره_navigate(هدف,
					      action_url,
					      صفحه_url,
					      BW_NAVIGATE_HISTORY,
					      عدم,
					      success,
					      عدم);

		اكسر;
	}

	nsurl_unref(action_url);
	fetch_multipart_داده_destroy(success);

	أرجع res;
}


/* exported interface documented in html/form_internal.h */
فراغ form_gadget_update_مقدار(بنية form_control *control, حرف *مقدار)
{
	بدل (control->type) {
	حالة GADGET_HIDDEN:
	حالة GADGET_TEXTBOX:
	حالة GADGET_TEXTAREA:
	حالة GADGET_PASSWORD:
	حالة GADGET_FILE:
		إذا (control->مقدار != عدم) {
			حرر(control->مقدار);
		}
		control->مقدار = مقدار;
		إذا (control->گره != عدم) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)مقدار,
						طول_رشته(مقدار), &str);
			إذا (err == DOM_NO_ERR) {
				إذا (control->type == GADGET_TEXTAREA)
					err = dom_html_متن_area_element_مجموعه_مقدار(
						(dom_html_متن_area_element *)(control->گره),
						str);
				وإلا
					err = dom_html_درون_داد_element_مجموعه_مقدار(
						(dom_html_درون_داد_element *)(control->گره),
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
	dom_string *مقدار = عدم;
	bool تغییریافته_dom = false;

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
	إذا ((control->آخرین_synced_مقدار == عدم &&
	     control->مقدار != عدم &&
	     control->مقدار[0] != '\0') ||
	    (control->آخرین_synced_مقدار != عدم &&
	     control->مقدار != عدم &&
	     strcmp(control->مقدار, control->آخرین_synced_مقدار) != 0)) {
		حرف *dup = strdup(control->مقدار);
		إذا (dup == عدم) {
			اذهب بیرون;
		}
		إذا (control->آخرین_synced_مقدار != عدم) {
			حرر(control->آخرین_synced_مقدار);
		}
		control->آخرین_synced_مقدار = dup;
		exc = dom_string_create((uint8_t *)(control->مقدار),
					طول_رشته(control->مقدار), &مقدار);
		إذا (exc != DOM_NO_ERR) {
			اذهب بیرون;
		}
		إذا (control->گره_مقدار != عدم) {
			dom_string_unref(control->گره_مقدار);
		}
		control->گره_مقدار = مقدار;
		مقدار = عدم;
		إذا (control->type == GADGET_TEXTAREA) {
			exc = dom_html_متن_area_element_مجموعه_مقدار(control->گره, control->گره_مقدار);
		} وإلا {
			exc = dom_html_درون_داد_element_مجموعه_مقدار(control->گره, control->گره_مقدار);
		}
		إذا (exc != DOM_NO_ERR) {
			اذهب بیرون;
		}
		تغییریافته_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	إذا (control->type == GADGET_TEXTAREA) {
		exc = dom_html_متن_area_element_get_مقدار(control->گره, &مقدار);
	} وإلا {
		exc = dom_html_درون_داد_element_get_مقدار(control->گره, &مقدار);
	}

	إذا (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		اذهب بیرون;
	}

	إذا (!dom_string_isequal(control->گره_مقدار, مقدار)) {
		/* The DOM has changed */
		إذا (!تغییریافته_dom) {
			/* And it wasn't us */
			حرف *مقدار_s = strndup(
				dom_string_داده(مقدار),
				dom_string_byte_درازی(مقدار));
			حرف *dup = عدم;
			إذا (مقدار_s == عدم) {
				اذهب بیرون;
			}
			dup = strdup(مقدار_s);
			إذا (dup == عدم) {
				حرر(مقدار_s);
				اذهب بیرون;
			}
			حرر(control->مقدار);
			control->مقدار = مقدار_s;
			حرر(control->آخرین_synced_مقدار);
			control->آخرین_synced_مقدار = dup;
			إذا (control->type != GADGET_HIDDEN &&
			    control->داده.متن.ta != عدم) {
				textarea_مجموعه_متن(control->داده.متن.ta,
						  مقدار_s);
			}
		}
		control->گره_مقدار = مقدار;
		مقدار = عدم;
	}

بیرون:
	إذا (مقدار != عدم)
		dom_string_unref(مقدار);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
بنية form *
form_new(فراغ *گره,
	 ثابت حرف *action,
	 ثابت حرف *هدف,
	 form_روش روش,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	بنية form *form;

	form = حافظه_صفر_بده(1, حجم *form);
	إذا (!form)
		أرجع عدم;

	form->action = strdup(action != عدم ? action : "");
	إذا (form->action == عدم) {
		حرر(form);
		أرجع عدم;
	}

	form->هدف = هدف != عدم ? strdup(هدف) : عدم;
	إذا (هدف != عدم && form->هدف == عدم) {
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->روش = روش;

	form->accept_charsets = charset != عدم ? strdup(charset) : عدم;
	إذا (charset != عدم && form->accept_charsets == عدم) {
		حرر(form->هدف);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->document_charset = doc_charset != عدم ? strdup(doc_charset)
						     : عدم;
	إذا (doc_charset && form->document_charset == عدم) {
		حرر(form->accept_charsets);
		حرر(form->هدف);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->گره = گره;

	أرجع form;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free(بنية form *form)
{
	بنية form_control *c, *d;

	لكل (c = form->controls; c != عدم; c = d) {
		d = c->بعدی;

		form_free_control(c);
	}

	حرر(form->action);
	حرر(form->هدف);
	حرر(form->accept_charsets);
	حرر(form->document_charset);

	حرر(form);
}


/* exported interface documented in html/form_internal.h */
بنية form_control *form_new_control(فراغ *گره, form_control_type type)
{
	بنية form_control *control;

	control = حافظه_صفر_بده(1, حجم *control);
	إذا (control == عدم)
		أرجع عدم;

	control->گره = گره;
	control->type = type;

	أرجع control;
}
