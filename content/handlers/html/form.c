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

#شامل <assert.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <dom/dom.h>

#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/talloc.h"
#شامل "utils/url.h"
#شامل "utils/utf8.h"
#شامل "utils/ascii.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/mouse.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/misc.h"
#شامل "content/fetch.h"
#شامل "content/hlcache.h"
#شامل "css/utils.h"
#شامل "desktop/knockout.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/layout.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_سطر_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

ساختار form_select_menu {
	صحیح سطر_height;
	صحیح width, height;
	ساختار scrollbar *scrollbar;
	صحیح f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	تهی *client_data;
	ساختار content *c;
};

ایستا plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

ایستا plot_font_style_t plot_fstyle_entry = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.weight = 400,
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
ایستا نویسه *
form_encode_item(ثابت نویسه *item,
		 uint32_t len,
		 ثابت نویسه *charset,
		 ثابت نویسه *fallback)
{
	nserror err;
	نویسه *ret = پوچ;
	نویسه cset[256];

	اگر (!item || !charset)
		بازگشت پوچ;

	snprintf(cset, اندازه cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	اگر (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, اندازه cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		اگر (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			اگر (fallback) {
				snprintf(cset, اندازه cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				اگر (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, اندازه cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			اگر (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				اگر (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	اگر (err == NSERROR_NOMEM) {
		بازگشت پوچ;
	}

	بازگشت ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_مقدار_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ایستا nserror
fetch_data_list_add_sname(ثابت نویسه *name,
			  ثابت نویسه *ksfx,
			  صحیح مقدار,
			  ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	ساختار fetch_multipart_data *fetch_data;
	صحیح keysize;

	fetch_data = calloc(1, اندازه(*fetch_data));
	اگر (fetch_data == پوچ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		بازگشت NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	اگر (fetch_data->name == پوچ) {
		آزاد(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		بازگشت NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*مقدار*/
	fetch_data->مقدار = malloc(FETCH_DATA_INT_مقدار_SIZE);
	اگر (fetch_data->مقدار == پوچ) {
		آزاد(fetch_data->name);
		آزاد(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		بازگشت NSERROR_NOMEM;
	}
	snprintf(fetch_data->مقدار, FETCH_DATA_INT_مقدار_SIZE, "%d", مقدار);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	بازگشت NSERROR_OK;
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
ایستا nserror
fetch_data_list_add(dom_string *name,
		    dom_string *مقدار,
		    ثابت نویسه *rawfile,
		    ثابت نویسه *form_charset,
		    ثابت نویسه *docu_charset,
		    ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	ساختار fetch_multipart_data *fetch_data;

	أكد(name != پوچ);

	fetch_data = calloc(1, اندازه(*fetch_data));
	اگر (fetch_data == پوچ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		بازگشت NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	اگر (fetch_data->name == پوچ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		آزاد(fetch_data);
		بازگشت NSERROR_NOMEM;
	}

	اگر (مقدار == پوچ) {
		fetch_data->مقدار = strdup("");
	} وگرنه {
		fetch_data->مقدار = form_encode_item(dom_string_data(مقدار),
						     dom_string_byte_length(مقدار),
						     form_charset,
						     docu_charset);
	}
	اگر (fetch_data->مقدار == پوچ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		آزاد(fetch_data->name);
		آزاد(fetch_data);
		بازگشت NSERROR_NOMEM;
	}

	/* deal with raw file name */
	اگر (rawfile != پوچ) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		اگر (fetch_data->rawfile == پوچ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			آزاد(fetch_data->مقدار);
			آزاد(fetch_data->name);
			آزاد(fetch_data);
			بازگشت NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	بازگشت NSERROR_OK;
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
ایستا nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  ثابت نویسه *form_charset,
			  ثابت نویسه *doc_charset,
			  ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		بازگشت NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (inputname == پوچ) {
		/* allow enumeration to continue after element with no name */
		بازگشت NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_مقدار(text_area_element,
						   &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		بازگشت NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  پوچ,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت نویسه *form_charset,
			      ثابت نویسه *docu_charset,
			      ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *مقدار;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		بازگشت NSERROR_DOM;
	}

	اگر (!selected) {
		/* unselected options do not add fetch data entries */
		بازگشت NSERROR_OK;
	}

	exp = dom_html_option_element_get_مقدار(option_element, &مقدار);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		بازگشت NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  مقدار,
				  پوچ,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(مقدار);

	بازگشت res;
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
ایستا nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			ثابت نویسه *form_charset,
			ثابت نویسه *doc_charset,
			ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = پوچ;
	uint32_t options_شمار;
	uint32_t option_index;
	dom_گره *option_element = پوچ;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		بازگشت NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (inputname == پوچ) {
		/* allow enumeration to continue after element with no name */
		بازگشت NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		بازگشت NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_شمار);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		بازگشت NSERROR_DOM;
	}

	/* iterate over options collection */
	برای (option_index = 0; option_index < options_شمار; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		اگر (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} وگرنه {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_گره_unref(option_element);
		}

		اگر (res != NSERROR_OK) {
			بشکن;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_درون_داد_submit(dom_html_درون_داد_element *درون_داد_element,
			      dom_string *inputname,
			      ثابت نویسه *charset,
			      ثابت نویسه *document_charset,
			      dom_html_element **submit_button,
			      ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	اگر (*submit_button == پوچ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)درون_داد_element;
	} وگرنه اگر (*submit_button != (dom_html_element *)درون_داد_element) {
		بازگشت NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		بازگشت NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  پوچ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_درون_داد_image(dom_html_درون_داد_element *درون_داد_element,
			     dom_string *inputname,
			     ثابت نویسه *charset,
			     ثابت نویسه *document_charset,
			     dom_html_element **submit_button,
			     ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	ساختار image_درون_داد_coords *coords;
	نویسه *basename;

	/* Only use an image input if it was the thing which activated us */
	اگر (*submit_button != (dom_html_element *)درون_داد_element) {
		بازگشت NSERROR_OK;
	}

	exp = dom_گره_get_user_data((dom_گره *)درون_داد_element,
				     corestring_dom_ns_کلید_image_coords_گره_data,
				     &coords);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		بازگشت NSERROR_DOM;
	}

	اگر (coords == پوچ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		بازگشت NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	اگر (basename == پوچ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		بازگشت NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	اگر (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	آزاد(basename);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_درون_داد_checkbox(dom_html_درون_داد_element *درون_داد_element,
				dom_string *inputname,
				ثابت نویسه *charset,
				ثابت نویسه *document_charset,
				ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_درون_داد_element_get_checked(درون_داد_element, &checked);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		بازگشت NSERROR_DOM;
	}

	اگر (!checked) {
		/* unchecked items do not generate a data entry */
		بازگشت NSERROR_OK;
	}

	exp = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		بازگشت NSERROR_DOM;
	}

	/* ensure a default value */
	اگر (inputvalue == پوچ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  پوچ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_درون_داد_file(dom_html_درون_داد_element *درون_داد_element,
			    dom_string *inputname,
			    ثابت نویسه *charset,
			    ثابت نویسه *document_charset,
			    ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت نویسه *rawfile = پوچ;

	exp = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		بازگشت NSERROR_DOM;
	}

	exp = dom_گره_get_user_data((dom_گره *)درون_داد_element,
				     corestring_dom_ns_کلید_file_name_گره_data,
				     &rawfile);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		بازگشت NSERROR_DOM;
	}

	اگر (rawfile == پوچ) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	بازگشت res;
}


ایستا nserror
form_dom_to_data_درون_داد_text(dom_html_درون_داد_element *درون_داد_element,
			    dom_string *inputname,
			    ثابت نویسه *charset,
			    ثابت نویسه *document_charset,
			    ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_درون_داد_element_get_مقدار(درون_داد_element, &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		بازگشت NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  پوچ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	بازگشت res;
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
ایستا nserror
form_dom_to_data_درون_داد(dom_html_درون_داد_element *درون_داد_element,
		       ثابت نویسه *charset,
		       ثابت نویسه *document_charset,
		       dom_html_element **submit_button,
		       ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_درون_داد_element_get_disabled(درون_داد_element,
						  &element_disabled);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* disabled element requires no more processing */
		بازگشت NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_درون_داد_element_get_name(درون_داد_element, &inputname);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (inputname == پوچ) {
		/* element with no name is not converted */
		بازگشت NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_درون_داد_element_get_type(درون_داد_element, &inputtype);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		بازگشت NSERROR_DOM;
	}

	/* process according to input element type */
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_درون_داد_submit(درون_داد_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} وگرنه اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_درون_داد_image(درون_داد_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} وگرنه اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_درون_داد_checkbox(درون_داد_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} وگرنه اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_درون_داد_file(درون_داد_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} وگرنه اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وگرنه {
		/* Everything else is treated as text values */
		res = form_dom_to_data_درون_داد_text(درون_داد_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	بازگشت res;
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
ایستا nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			ثابت نویسه *form_charset,
			ثابت نویسه *doc_charset,
			dom_html_element **submit_button,
			ساختار fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	exp = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		بازگشت NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		بازگشت NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		بازگشت NSERROR_OK;
	}
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		بازگشت NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	اگر (*submit_button == پوچ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	اگر (*submit_button != (dom_html_element *)button_element) {
		بازگشت NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		بازگشت NSERROR_DOM;
	}

	اگر (inputname == پوچ) {
		/* allow enumeration to continue after element with no name */
		بازگشت NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_مقدار(button_element, &inputvalue);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} وگرنه {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  پوچ,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	بازگشت res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ایستا نویسه *form_acceptable_charset(ساختار form *form)
{
	نویسه *temp, *c;

	اگر (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		اگر (form->document_charset) {
			/* document charset present, so use it */
			بازگشت strdup(form->document_charset);
		} وگرنه {
			/* no document charset, so default to 8859-1 */
			بازگشت strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	اگر (!temp)
		بازگشت پوچ;

	/* make it upper case */
	برای (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	اگر (c) {
		آزاد(temp);
		بازگشت strdup("UTF-8");
	}

	/* dispense with temporary copy */
	آزاد(temp);

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
	وقتی (*c && !ascii_is_space(*c)) {
		اگر (*c == ',')
			بشکن;
		c++;
	}

	بازگشت strndup(form->accept_charsets, c - form->accept_charsets);
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
ایستا nserror
form_dom_to_data(ساختار form *form,
		 ساختار form_control *submit_control,
		 ساختار fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	نویسه *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = پوچ; /* the dom form elements */
	uint32_t element_شمار; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_گره *element = پوچ; /* the DOM form element */
	dom_string *nodename = پوچ; /* the DOM node name of the element */
	ساختار fetch_multipart_data *fetch_data = پوچ; /* fetch data list */
	ساختار fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	اگر (submit_control != پوچ) {
		submit_button = submit_control->گره;
	} وگرنه {
		submit_button = پوچ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	اگر (charset == پوچ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		بازگشت NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->گره, &elements);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		آزاد(charset);
		بازگشت NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_شمار);
	اگر (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		برو form_dom_to_data_error;
	}

	برای (element_idx = 0; element_idx < element_شمار; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		اگر (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			برو form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_گره_get_گره_name(element, &nodename);
		اگر (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_گره_unref(element);
			res = NSERROR_DOM;
			برو form_dom_to_data_error;
		}

		اگر (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} وگرنه اگر (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} وگرنه اگر (dom_string_isequal(nodename, corestring_dom_درون_داد)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_درون_داد(
				(dom_html_درون_داد_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} وگرنه اگر (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} وگرنه {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحیح)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_گره_unref(element);

		/* abort form element enumeration on error */
		اگر (res != NSERROR_OK) {
			برو form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	آزاد(charset);

	بازگشت NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	آزاد(charset);

	بازگشت res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ایستا nserror
form_url_encode(ساختار form *form,
		ساختار fetch_multipart_data *control,
		نویسه **encoded_out)
{
	نویسه *name, *مقدار;
	نویسه *s, *s2;
	unsigned صحیح len, len1, len_init;
	nserror res;

	s = malloc(1);

	اگر (s == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	برای (; control; control = control->next) {
		res = url_escape(control->name, true, پوچ, &name);
		اگر (res != NSERROR_OK) {
			آزاد(s);
			بازگشت res;
		}

		res = url_escape(control->مقدار, true, پوچ, &مقدار);
		اگر (res != NSERROR_OK) {
			آزاد(name);
			آزاد(s);
			بازگشت res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(مقدار) + 2;
		s2 = realloc(s, len1 + 1);
		اگر (s2 == پوچ) {
			آزاد(مقدار);
			آزاد(name);
			آزاد(s);
			بازگشت NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, مقدار);
		len = len1;
		آزاد(name);
		آزاد(مقدار);
	}

	اگر (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	بازگشت NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ایستا تهی
form_select_menu_scroll_callback(تهی *client_data,
				 ساختار scrollbar_پیام_data *scrollbar_data)
{
	ساختار form_control *control = client_data;
	ساختار form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	انتخاب (scrollbar_data->پیام) {
		حالت SCROLLBAR_پیام_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			بشکن;
		حالت SCROLLBAR_پیام_SCROLL_START:
		{
			ساختار rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			بشکن;
		حالت SCROLLBAR_پیام_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, پوچ);
			بشکن;
		پیشفرض:
			بشکن;
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
ایستا nserror
form__select_process_selection(html_content *html,
			       ساختار form_control *control,
			       صحیح item)
{
	ساختار box *inline_box;
	ساختار form_option *o;
	صحیح شمار;
	nserror ret = NSERROR_OK;

	أكد(control != پوچ);
	أكد(html != پوچ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	برای (شمار = 0, o = control->data.select.اقلام;
			o != پوچ;
			شمار++, o = o->next) {
		اگر (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->گره, false);
		}

		اگر (شمار == item) {
			اگر (control->data.select.multiple) {
				اگر (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->گره, false);
					control->data.select.رقم_selected--;
				} وگرنه {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->گره, true);
					control->data.select.رقم_selected++;
				}
			} وگرنه {
				dom_html_option_element_set_selected(
						o->گره, true);
				o->selected = true;
			}
		}

		اگر (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	اگر (control->data.select.رقم_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وگرنه اگر (control->data.select.رقم_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} وگرنه {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	اگر (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} وگرنه {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	بازگشت ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ایستا تهی form_select_menu_clicked(ساختار form_control *control, صحیح x, صحیح y)
{
	ساختار form_select_menu *menu = control->data.select.menu;
	ساختار form_option *option;
	html_content *html = (html_content *)menu->c;
	صحیح سطر_height, سطر_height_with_spacing;
	صحیح item_bottom_y;
	صحیح scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	سطر_height = menu->سطر_height;
	سطر_height_with_spacing = سطر_height +
			سطر_height * SELECT_سطر_SPACING;

	option = control->data.select.اقلام;
	item_bottom_y = سطر_height_with_spacing;
	i = 0;
	وقتی (option && item_bottom_y < scroll + y) {
		item_bottom_y += سطر_height_with_spacing;
		option = option->next;
		i++;
	}

	اگر (option != پوچ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
تهی form_add_control(ساختار form *form, ساختار form_control *control)
{
	اگر (form == پوچ) {
		بازگشت;
	}

	control->form = form;

	اگر (form->controls != پوچ) {
		أكد(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = پوچ;
		form->last_control = control;
	} وگرنه {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
تهی form_free_control(ساختار form_control *control)
{
	ساختار form_control *c;
	أكد(control != پوچ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->مقدار, control->initial_مقدار);
	آزاد(control->name);
	آزاد(control->مقدار);
	آزاد(control->initial_مقدار);
	اگر (control->last_synced_مقدار != پوچ) {
		آزاد(control->last_synced_مقدار);
	}

	اگر (control->type == GADGET_SELECT) {
		ساختار form_option *option, *next;

		برای (option = control->data.select.اقلام; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->مقدار);
			آزاد(option->text);
			آزاد(option->مقدار);
			آزاد(option);
		}
		اگر (control->data.select.menu != پوچ) {
			form_free_select_menu(control);
		}
	}

	اگر (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		اگر (control->data.text.initial != پوچ) {
			dom_string_unref(control->data.text.initial);
		}

		اگر (control->data.text.ta != پوچ) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	اگر (control->form != پوچ) {
		برای (c = control->form->controls; c != پوچ; c = c->next) {
			اگر (c->next == control) {
				c->next = control->next;
				اگر (control->form->last_control == control)
					control->form->last_control = c;
				بشکن;
			}
			اگر (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				اگر (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = پوچ;
				بشکن;
			}
		}
	}

	اگر (control->گره_مقدار != پوچ) {
		dom_string_unref(control->گره_مقدار);
	}

	آزاد(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(ساختار form_control *control, نویسه *مقدار, نویسه *text,
		     bool selected, تهی *گره)
{
	ساختار form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = calloc(1, اندازه *option);
	اگر (!option)
		بازگشت false;

	option->مقدار = مقدار;
	option->text = text;

	/* add to linked list */
	اگر (control->data.select.اقلام == 0)
		control->data.select.اقلام = option;
	وگرنه
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	اگر (selected && (control->data.select.رقم_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.رقم_selected++;
		control->data.select.current = option;
	}

	control->data.select.رقم_اقلام++;

	option->گره = گره;

	بازگشت true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(تهی *client_data,
		      ساختار form_control *control,
		      select_menu_redraw_callback callback,
		      ساختار content *c)
{
	صحیح سطر_height_with_spacing;
	ساختار box *box;
	plot_font_style_t fstyle;
	صحیح total_height;
	ساختار form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	اگر (control->data.select.menu == پوچ) {

		menu = calloc(1, اندازه (ساختار form_select_menu));
		اگر (menu == پوچ) {
			بازگشت NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[راست_2].width + box->padding[راست_2] +
			box->border[چپ_2].width + box->padding[چپ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->سطر_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		سطر_height_with_spacing = menu->سطر_height +
				menu->سطر_height *
				SELECT_سطر_SPACING;

		total_height = control->data.select.رقم_اقلام *
				سطر_height_with_spacing;
		menu->height = total_height;

		اگر (menu->height > MAX_SELECT_HEIGHT) {
			menu->height = MAX_SELECT_HEIGHT;
		}

		menu->client_data = client_data;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->height,
				       total_height,
				       menu->height,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		اگر (res != NSERROR_OK) {
			control->data.select.menu = پوچ;
			آزاد(menu);
			بازگشت res;
		}
		menu->c = c;
	} وگرنه {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	بازگشت NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
تهی form_free_select_menu(ساختار form_control *control)
{
	اگر (control->data.select.menu->scrollbar != پوچ)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	آزاد(control->data.select.menu);
	control->data.select.menu = پوچ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ساختار form_control *control,
			صحیح x, صحیح y,
			اعشاری scale,
			ثابت ساختار rect *clip,
			ثابت ساختار redraw_context *ctx)
{
	ساختار box *box;
	ساختار form_select_menu *menu = control->data.select.menu;
	ساختار form_option *option;
	صحیح سطر_height, سطر_height_with_spacing;
	صحیح width, height;
	صحیح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحیح item_y;
	صحیح text_pos_offset, text_x;
	صحیح scrollbar_width = SCROLLBAR_WIDTH;
	صحیح i;
	صحیح scroll;
	صحیح x_cp, y_cp;
	ساختار rect r;
	ساختار rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	سطر_height = menu->سطر_height;

	سطر_height_with_spacing = سطر_height +
			سطر_height * SELECT_سطر_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	اگر (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / سطر_height_with_spacing;
		scroll -= i * سطر_height_with_spacing;
		سطر_height *= scale;
		سطر_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * سطر_height_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + width - 1;
	y1 = y + height - 1;
	scrollbar_x = x1 - scrollbar_width;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	x0 = x0 + SELECT_BORDER_WIDTH;
	y0 = y0 + SELECT_BORDER_WIDTH;
	x1 = x1 - SELECT_BORDER_WIDTH;
	y1 = y1 - SELECT_BORDER_WIDTH;
	height = height - 2 * SELECT_BORDER_WIDTH;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	option = control->data.select.اقلام;
	item_y = سطر_height_with_spacing;

	وقتی (item_y < scroll) {
		option = option->next;
		item_y += سطر_height_with_spacing;
	}
	item_y -= سطر_height_with_spacing;
	text_pos_offset = y - scroll +
			(صحیح) (سطر_height * (0.75 + SELECT_سطر_SPACING));
	text_x = x + (box->border[چپ_2].width + box->padding[چپ_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	وقتی (option && item_y - scroll < height) {

		اگر (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + سطر_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}

		item_y += سطر_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	بازگشت true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ساختار form_control *control,
			     اعشاری scale,
			     ثابت ساختار rect *clip)
{
	ساختار form_select_menu *menu = control->data.select.menu;
	صحیح width, height;


	width = menu->width;
	height = menu->height;

	اگر (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	اگر (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		بازگشت true;

	بازگشت false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ساختار form_control *control, صحیح item)
{
	أكد(control != پوچ);

	بازگشت form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
ساختار form_option *
form_select_get_option(ساختار form_control *control, صحیح item)
{
	ساختار form_option *opt;

	opt = control->data.select.اقلام;
	وقتی ((opt != پوچ) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	بازگشت opt;
}


/* exported interface documented in netsurf/form.h */
نویسه *form_control_get_name(ساختار form_control *control)
{
	بازگشت control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ساختار form_control *control, ساختار rect *r)
{
	box_bounds( control->box, r );
	بازگشت NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ثابت نویسه *
form_select_mouse_action(ساختار form_control *control,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	ساختار form_select_menu *menu = control->data.select.menu;
	صحیح x0, y0, x1, y1, scrollbar_x;
	ثابت نویسه *status = پوچ;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	اگر (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		بازگشت scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	اگر (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		اگر (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		اگر (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} وگرنه اگر (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	بازگشت status;
}


/* private interface described in html/form_internal.h */
تهی
form_select_mouse_drag_end(ساختار form_control *control,
			   browser_mouse_state mouse,
			   صحیح x, صحیح y)
{
	صحیح x0, y0, x1, y1;
	صحیح box_x, box_y;
	ساختار box *box;
	ساختار form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[چپ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[بالاترین_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	اگر (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		بازگشت;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	اگر (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
تهی form_select_get_dimensions(ساختار form_control *control,
		صحیح *width, صحیح *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
تهی form_select_menu_callback(تهی *client_data,
		صحیح x, صحیح y, صحیح width, صحیح height)
{
	html_content *html = client_data;
	صحیح menu_x, menu_y;
	ساختار box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[چپ_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[بالاترین_2];
	content__request_redraw((ساختار content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
تهی form_radio_set(ساختار form_control *radio)
{
	ساختار form_control *control;

	أكد(radio);
	اگر (!radio->form)
		بازگشت;

	اگر (radio->selected)
		بازگشت;

	/* Clear selected state for other controls in
	 * the same radio button group */
	برای (control = radio->form->controls;
	     control != پوچ;
	     control = control->next) {
		/* Only interested in radio inputs */
		اگر (control->type != GADGET_RADIO)
			ادامه;

		/* Ignore ourself */
		اگر (control == radio)
			ادامه;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		اگر ((control->name == پوچ) ||
		    (radio->name == پوچ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ادامه;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		اگر (control->selected) {
			control->selected = false;
			dom_html_درون_داد_element_set_checked(control->گره, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_درون_داد_element_set_checked(radio->گره, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    ساختار browser_window *target,
	    ساختار form *form,
	    ساختار form_control *submit_button)
{
	nserror res;
	نویسه *data = پوچ; /* encoded form data */
	ساختار fetch_multipart_data *success = پوچ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != پوچ);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	اگر (res != NSERROR_OK) {
		بازگشت res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	اگر (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		بازگشت res;
	}

	انتخاب (form->method) {
	حالت method_GET:
		res = form_url_encode(form, success, &data);
		اگر (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			اگر (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      پوچ,
							      پوچ,
							      پوچ);

				nsurl_unref(query_url);
			}
			آزاد(data);
		}
		بشکن;

	حالت method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		اگر (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      پوچ,
						      پوچ);
			آزاد(data);
		}
		بشکن;

	حالت method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      پوچ,
					      success,
					      پوچ);

		بشکن;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	بازگشت res;
}


/* exported interface documented in html/form_internal.h */
تهی form_gadget_update_مقدار(ساختار form_control *control, نویسه *مقدار)
{
	انتخاب (control->type) {
	حالت GADGET_HIDDEN:
	حالت GADGET_TEXTBOX:
	حالت GADGET_TEXTAREA:
	حالت GADGET_PASSWORD:
	حالت GADGET_FILE:
		اگر (control->مقدار != پوچ) {
			آزاد(control->مقدار);
		}
		control->مقدار = مقدار;
		اگر (control->گره != پوچ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)مقدار,
						strlen(مقدار), &str);
			اگر (err == DOM_NO_ERR) {
				اگر (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_مقدار(
						(dom_html_text_area_element *)(control->گره),
						str);
				وگرنه
					err = dom_html_درون_داد_element_set_مقدار(
						(dom_html_درون_داد_element *)(control->گره),
						str);
				dom_string_unref(str);
			}
		}
		بشکن;
	پیشفرض:
		/* Do nothing */
		بشکن;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
تهی
form_gadget_sync_with_dom(ساختار form_control *control)
{
	dom_exception exc;
	dom_string *مقدار = پوچ;
	bool changed_dom = false;

	اگر (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		بازگشت;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	اگر ((control->last_synced_مقدار == پوچ &&
	     control->مقدار != پوچ &&
	     control->مقدار[0] != '\0') ||
	    (control->last_synced_مقدار != پوچ &&
	     control->مقدار != پوچ &&
	     strcmp(control->مقدار, control->last_synced_مقدار) != 0)) {
		نویسه *dup = strdup(control->مقدار);
		اگر (dup == پوچ) {
			برو out;
		}
		اگر (control->last_synced_مقدار != پوچ) {
			آزاد(control->last_synced_مقدار);
		}
		control->last_synced_مقدار = dup;
		exc = dom_string_create((uint8_t *)(control->مقدار),
					strlen(control->مقدار), &مقدار);
		اگر (exc != DOM_NO_ERR) {
			برو out;
		}
		اگر (control->گره_مقدار != پوچ) {
			dom_string_unref(control->گره_مقدار);
		}
		control->گره_مقدار = مقدار;
		مقدار = پوچ;
		اگر (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_مقدار(control->گره, control->گره_مقدار);
		} وگرنه {
			exc = dom_html_درون_داد_element_set_مقدار(control->گره, control->گره_مقدار);
		}
		اگر (exc != DOM_NO_ERR) {
			برو out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	اگر (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_مقدار(control->گره, &مقدار);
	} وگرنه {
		exc = dom_html_درون_داد_element_get_مقدار(control->گره, &مقدار);
	}

	اگر (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		برو out;
	}

	اگر (!dom_string_isequal(control->گره_مقدار, مقدار)) {
		/* The DOM has changed */
		اگر (!changed_dom) {
			/* And it wasn't us */
			نویسه *مقدار_s = strndup(
				dom_string_data(مقدار),
				dom_string_byte_length(مقدار));
			نویسه *dup = پوچ;
			اگر (مقدار_s == پوچ) {
				برو out;
			}
			dup = strdup(مقدار_s);
			اگر (dup == پوچ) {
				آزاد(مقدار_s);
				برو out;
			}
			آزاد(control->مقدار);
			control->مقدار = مقدار_s;
			آزاد(control->last_synced_مقدار);
			control->last_synced_مقدار = dup;
			اگر (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != پوچ) {
				textarea_set_text(control->data.text.ta,
						  مقدار_s);
			}
		}
		control->گره_مقدار = مقدار;
		مقدار = پوچ;
	}

out:
	اگر (مقدار != پوچ)
		dom_string_unref(مقدار);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ساختار form *
form_new(تهی *گره,
	 ثابت نویسه *action,
	 ثابت نویسه *target,
	 form_method method,
	 ثابت نویسه *charset,
	 ثابت نویسه *doc_charset)
{
	ساختار form *form;

	form = calloc(1, اندازه *form);
	اگر (!form)
		بازگشت پوچ;

	form->action = strdup(action != پوچ ? action : "");
	اگر (form->action == پوچ) {
		آزاد(form);
		بازگشت پوچ;
	}

	form->target = target != پوچ ? strdup(target) : پوچ;
	اگر (target != پوچ && form->target == پوچ) {
		آزاد(form->action);
		آزاد(form);
		بازگشت پوچ;
	}

	form->method = method;

	form->accept_charsets = charset != پوچ ? strdup(charset) : پوچ;
	اگر (charset != پوچ && form->accept_charsets == پوچ) {
		آزاد(form->target);
		آزاد(form->action);
		آزاد(form);
		بازگشت پوچ;
	}

	form->document_charset = doc_charset != پوچ ? strdup(doc_charset)
						     : پوچ;
	اگر (doc_charset && form->document_charset == پوچ) {
		آزاد(form->accept_charsets);
		آزاد(form->target);
		آزاد(form->action);
		آزاد(form);
		بازگشت پوچ;
	}

	form->گره = گره;

	بازگشت form;
}


/* exported interface documented in html/form_internal.h */
تهی form_free(ساختار form *form)
{
	ساختار form_control *c, *d;

	برای (c = form->controls; c != پوچ; c = d) {
		d = c->next;

		form_free_control(c);
	}

	آزاد(form->action);
	آزاد(form->target);
	آزاد(form->accept_charsets);
	آزاد(form->document_charset);

	آزاد(form);
}


/* exported interface documented in html/form_internal.h */
ساختار form_control *form_new_control(تهی *گره, form_control_type type)
{
	ساختار form_control *control;

	control = calloc(1, اندازه *control);
	اگر (control == پوچ)
		بازگشت پوچ;

	control->گره = گره;
	control->type = type;

	بازگشت control;
}
