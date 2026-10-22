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

#define MAX_SELECT_HEIGHT 210
#define SELECT_LINE_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

بنية form_select_menu {
	صحيح line_height;
	صحيح width, height;
	بنية scrollbar *scrollbar;
	صحيح f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	فراغ *client_data;
	بنية content *c;
};

ساكن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

ساكن plot_font_style_t plot_fstyle_entry = {
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
ساكن حرف *
form_encode_item(ثابت حرف *item,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = عدم;
	حرف cset[256];

	𐡄𐡍 (!item || !charset)
		أرجع عدم;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	𐡄𐡍 (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		𐡄𐡍 (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			𐡄𐡍 (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				𐡄𐡍 (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			𐡄𐡍 (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				𐡄𐡍 (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	𐡄𐡍 (err == NSERROR_NOMEM) {
		أرجع عدم;
	}

	أرجع ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_VALUE_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساكن nserror
fetch_data_list_add_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحيح value,
			  بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	بنية fetch_multipart_data *fetch_data;
	صحيح keysize;

	fetch_data = calloc(1, حجم(*fetch_data));
	𐡄𐡍 (fetch_data == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	𐡄𐡍 (fetch_data->name == عدم) {
		حرر(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/* value */
	fetch_data->value = malloc(FETCH_DATA_INT_VALUE_SIZE);
	𐡄𐡍 (fetch_data->value == عدم) {
		حرر(fetch_data->name);
		حرر(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_data->value, FETCH_DATA_INT_VALUE_SIZE, "%d", value);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

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
fetch_data_list_add(dom_string *name,
		    dom_string *value,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	بنية fetch_multipart_data *fetch_data;

	أكد(name != عدم);

	fetch_data = calloc(1, حجم(*fetch_data));
	𐡄𐡍 (fetch_data == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	𐡄𐡍 (fetch_data->name == عدم) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		حرر(fetch_data);
		أرجع NSERROR_NOMEM;
	}

	𐡄𐡍 (value == عدم) {
		fetch_data->value = strdup("");
	} وإلا {
		fetch_data->value = form_encode_item(dom_string_data(value),
						     dom_string_byte_length(value),
						     form_charset,
						     docu_charset);
	}
	𐡄𐡍 (fetch_data->value == عدم) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		حرر(fetch_data->name);
		حرر(fetch_data);
		أرجع NSERROR_NOMEM;
	}

	/* deal with raw file name */
	𐡄𐡍 (rawfile != عدم) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		𐡄𐡍 (fetch_data->rawfile == عدم) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			حرر(fetch_data->value);
			حرر(fetch_data->name);
			حرر(fetch_data);
			أرجع NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

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
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_value(text_area_element,
						   &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  عدم,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *value;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (!selected) {
		/* unselected options do not add fetch data entries */
		أرجع NSERROR_OK;
	}

	exp = dom_html_option_element_get_value(option_element, &value);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  value,
				  عدم,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(value);

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
form_dom_to_data_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = عدم;
	uint32_t options_count;
	uint32_t option_index;
	dom_node *option_element = عدم;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_count);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* iterate over options collection */
	لكل (option_index = 0; option_index < options_count; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		𐡄𐡍 (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} وإلا {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_node_unref(option_element);
		}

		𐡄𐡍 (res != NSERROR_OK) {
			اكسر;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_to_data_input_submit(dom_html_input_element *input_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	𐡄𐡍 (*submit_button == عدم) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)input_element;
	} وإلا 𐡄𐡍 (*submit_button != (dom_html_element *)input_element) {
		أرجع NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_to_data_input_image(dom_html_input_element *input_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	بنية image_input_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	𐡄𐡍 (*submit_button != (dom_html_element *)input_element) {
		أرجع NSERROR_OK;
	}

	exp = dom_node_get_user_data((dom_node *)input_element,
				     corestring_dom___ns_key_image_coords_node_data,
				     &coords);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (coords == عدم) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		أرجع NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	𐡄𐡍 (basename == عدم) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		أرجع NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	𐡄𐡍 (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	حرر(basename);

	أرجع res;
}


ساكن nserror
form_dom_to_data_input_checkbox(dom_html_input_element *input_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_input_element_get_checked(input_element, &checked);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (!checked) {
		/* unchecked items do not generate a data entry */
		أرجع NSERROR_OK;
	}

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		أرجع NSERROR_DOM;
	}

	/* ensure a default value */
	𐡄𐡍 (inputvalue == عدم) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_to_data_input_file(dom_html_input_element *input_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = عدم;

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		أرجع NSERROR_DOM;
	}

	exp = dom_node_get_user_data((dom_node *)input_element,
				     corestring_dom___ns_key_file_name_node_data,
				     &rawfile);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (rawfile == عدم) {
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

	أرجع res;
}


ساكن nserror
form_dom_to_data_input_text(dom_html_input_element *input_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

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
form_dom_to_data_input(dom_html_input_element *input_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       بنية fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_input_element_get_disabled(input_element,
						  &element_disabled);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (element_disabled) {
		/* disabled element requires no more processing */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_input_element_get_name(input_element, &inputname);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (inputname == عدم) {
		/* element with no name is not converted */
		أرجع NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_input_element_get_type(input_element, &inputtype);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* process according to input element type */
	𐡄𐡍 (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_input_submit(input_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} وإلا 𐡄𐡍 (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_input_image(input_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} وإلا 𐡄𐡍 (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_input_checkbox(input_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} وإلا 𐡄𐡍 (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_input_file(input_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} وإلا 𐡄𐡍 (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وإلا {
		/* Everything else is treated as text values */
		res = form_dom_to_data_input_text(input_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

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
form_dom_to_data_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			بنية fetch_multipart_data ***fetch_data_next_ptr)
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
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		أرجع NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	𐡄𐡍 (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	𐡄𐡍 (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	𐡄𐡍 (*submit_button == عدم) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	𐡄𐡍 (*submit_button != (dom_html_element *)button_element) {
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		أرجع NSERROR_DOM;
	}

	𐡄𐡍 (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_value(button_element, &inputvalue);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} وإلا {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  عدم,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

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
	حرف *temp, *c;

	𐡄𐡍 (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		𐡄𐡍 (form->document_charset) {
			/* document charset present, so use it */
			أرجع strdup(form->document_charset);
		} وإلا {
			/* no document charset, so default to 8859-1 */
			أرجع strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	𐡄𐡍 (!temp)
		أرجع عدم;

	/* make it upper case */
	لكل (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	𐡄𐡍 (c) {
		حرر(temp);
		أرجع strdup("UTF-8");
	}

	/* dispense with temporary copy */
	حرر(temp);

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
		𐡄𐡍 (*c == ',')
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
form_dom_to_data(بنية form *form,
		 بنية form_control *submit_control,
		 بنية fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = عدم; /* the dom form elements */
	uint32_t element_count; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_node *element = عدم; /* the DOM form element */
	dom_string *nodename = عدم; /* the DOM node name of the element */
	بنية fetch_multipart_data *fetch_data = عدم; /* fetch data list */
	بنية fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	𐡄𐡍 (submit_control != عدم) {
		submit_button = submit_control->node;
	} وإلا {
		submit_button = عدم;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	𐡄𐡍 (charset == عدم) {
		NSLOG(netsurf, INFO, "failed to find charset");
		أرجع NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->node, &elements);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		حرر(charset);
		أرجع NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_count);
	𐡄𐡍 (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		اذهب form_dom_to_data_error;
	}

	لكل (element_idx = 0; element_idx < element_count; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		𐡄𐡍 (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			اذهب form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_node_get_node_name(element, &nodename);
		𐡄𐡍 (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_node_unref(element);
			res = NSERROR_DOM;
			اذهب form_dom_to_data_error;
		}

		𐡄𐡍 (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} وإلا 𐡄𐡍 (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} وإلا 𐡄𐡍 (dom_string_isequal(nodename, corestring_dom_INPUT)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_input(
				(dom_html_input_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} وإلا 𐡄𐡍 (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} وإلا {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحيح)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_node_unref(element);

		/* abort form element enumeration on error */
		𐡄𐡍 (res != NSERROR_OK) {
			اذهب form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
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
		بنية fetch_multipart_data *control,
		حرف **encoded_out)
{
	حرف *name, *value;
	حرف *s, *s2;
	unsigned صحيح len, len1, len_init;
	nserror res;

	s = malloc(1);

	𐡄𐡍 (s == عدم) {
		أرجع NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	لكل (; control; control = control->next) {
		res = url_escape(control->name, true, عدم, &name);
		𐡄𐡍 (res != NSERROR_OK) {
			حرر(s);
			أرجع res;
		}

		res = url_escape(control->value, true, عدم, &value);
		𐡄𐡍 (res != NSERROR_OK) {
			حرر(name);
			حرر(s);
			أرجع res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(value) + 2;
		s2 = realloc(s, len1 + 1);
		𐡄𐡍 (s2 == عدم) {
			حرر(value);
			حرر(name);
			حرر(s);
			أرجع NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, value);
		len = len1;
		حرر(name);
		حرر(value);
	}

	𐡄𐡍 (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	أرجع NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساكن فراغ
form_select_menu_scroll_callback(فراغ *client_data,
				 بنية scrollbar_msg_data *scrollbar_data)
{
	بنية form_control *control = client_data;
	بنية form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	بدل (scrollbar_data->msg) {
		حالة SCROLLBAR_MSG_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			اكسر;
		حالة SCROLLBAR_MSG_SCROLL_START:
		{
			بنية rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			اكسر;
		حالة SCROLLBAR_MSG_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
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
			       صحيح item)
{
	بنية box *inline_box;
	بنية form_option *o;
	صحيح count;
	nserror ret = NSERROR_OK;

	أكد(control != عدم);
	أكد(html != عدم);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	لكل (count = 0, o = control->data.select.items;
			o != عدم;
			count++, o = o->next) {
		𐡄𐡍 (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->node, false);
		}

		𐡄𐡍 (count == item) {
			𐡄𐡍 (control->data.select.multiple) {
				𐡄𐡍 (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->node, false);
					control->data.select.num_selected--;
				} وإلا {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->node, true);
					control->data.select.num_selected++;
				}
			} وإلا {
				dom_html_option_element_set_selected(
						o->node, true);
				o->selected = true;
			}
		}

		𐡄𐡍 (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	𐡄𐡍 (control->data.select.num_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وإلا 𐡄𐡍 (control->data.select.num_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} وإلا {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	𐡄𐡍 (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} وإلا {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

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
	بنية form_select_menu *menu = control->data.select.menu;
	بنية form_option *option;
	html_content *html = (html_content *)menu->c;
	صحيح line_height, line_height_with_spacing;
	صحيح item_bottom_y;
	صحيح scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	line_height = menu->line_height;
	line_height_with_spacing = line_height +
			line_height * SELECT_LINE_SPACING;

	option = control->data.select.items;
	item_bottom_y = line_height_with_spacing;
	i = 0;
	طالما (option && item_bottom_y < scroll + y) {
		item_bottom_y += line_height_with_spacing;
		option = option->next;
		i++;
	}

	𐡄𐡍 (option != عدم) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
فراغ form_add_control(بنية form *form, بنية form_control *control)
{
	𐡄𐡍 (form == عدم) {
		أرجع;
	}

	control->form = form;

	𐡄𐡍 (form->controls != عدم) {
		أكد(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = عدم;
		form->last_control = control;
	} وإلا {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_control(بنية form_control *control)
{
	بنية form_control *c;
	أكد(control != عدم);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->value, control->initial_value);
	حرر(control->name);
	حرر(control->value);
	حرر(control->initial_value);
	𐡄𐡍 (control->last_synced_value != عدم) {
		حرر(control->last_synced_value);
	}

	𐡄𐡍 (control->type == GADGET_SELECT) {
		بنية form_option *option, *next;

		لكل (option = control->data.select.items; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->value);
			حرر(option->text);
			حرر(option->value);
			حرر(option);
		}
		𐡄𐡍 (control->data.select.menu != عدم) {
			form_free_select_menu(control);
		}
	}

	𐡄𐡍 (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		𐡄𐡍 (control->data.text.initial != عدم) {
			dom_string_unref(control->data.text.initial);
		}

		𐡄𐡍 (control->data.text.ta != عدم) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	𐡄𐡍 (control->form != عدم) {
		لكل (c = control->form->controls; c != عدم; c = c->next) {
			𐡄𐡍 (c->next == control) {
				c->next = control->next;
				𐡄𐡍 (control->form->last_control == control)
					control->form->last_control = c;
				اكسر;
			}
			𐡄𐡍 (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				𐡄𐡍 (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = عدم;
				اكسر;
			}
		}
	}

	𐡄𐡍 (control->node_value != عدم) {
		dom_string_unref(control->node_value);
	}

	حرر(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(بنية form_control *control, حرف *value, حرف *text,
		     bool selected, فراغ *node)
{
	بنية form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = calloc(1, حجم *option);
	𐡄𐡍 (!option)
		أرجع false;

	option->value = value;
	option->text = text;

	/* add to linked list */
	𐡄𐡍 (control->data.select.items == 0)
		control->data.select.items = option;
	وإلا
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	𐡄𐡍 (selected && (control->data.select.num_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.num_selected++;
		control->data.select.current = option;
	}

	control->data.select.num_items++;

	option->node = node;

	أرجع true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(فراغ *client_data,
		      بنية form_control *control,
		      select_menu_redraw_callback callback,
		      بنية content *c)
{
	صحيح line_height_with_spacing;
	بنية box *box;
	plot_font_style_t fstyle;
	صحيح total_height;
	بنية form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	𐡄𐡍 (control->data.select.menu == عدم) {

		menu = calloc(1, حجم (بنية form_select_menu));
		𐡄𐡍 (menu == عدم) {
			أرجع NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[RIGHT].width + box->padding[RIGHT] +
			box->border[LEFT].width + box->padding[LEFT];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->line_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		line_height_with_spacing = menu->line_height +
				menu->line_height *
				SELECT_LINE_SPACING;

		total_height = control->data.select.num_items *
				line_height_with_spacing;
		menu->height = total_height;

		𐡄𐡍 (menu->height > MAX_SELECT_HEIGHT) {
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
		𐡄𐡍 (res != NSERROR_OK) {
			control->data.select.menu = عدم;
			حرر(menu);
			أرجع res;
		}
		menu->c = c;
	} وإلا {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	أرجع NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_select_menu(بنية form_control *control)
{
	𐡄𐡍 (control->data.select.menu->scrollbar != عدم)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	حرر(control->data.select.menu);
	control->data.select.menu = عدم;
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
	بنية form_select_menu *menu = control->data.select.menu;
	بنية form_option *option;
	صحيح line_height, line_height_with_spacing;
	صحيح width, height;
	صحيح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحيح item_y;
	صحيح text_pos_offset, text_x;
	صحيح scrollbar_width = SCROLLBAR_WIDTH;
	صحيح i;
	صحيح scroll;
	صحيح x_cp, y_cp;
	بنية rect r;
	بنية rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	line_height = menu->line_height;

	line_height_with_spacing = line_height +
			line_height * SELECT_LINE_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	𐡄𐡍 (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / line_height_with_spacing;
		scroll -= i * line_height_with_spacing;
		line_height *= scale;
		line_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * line_height_with_spacing;
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
	𐡄𐡍 (res != NSERROR_OK) {
		أرجع false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	𐡄𐡍 (res != NSERROR_OK) {
		أرجع false;
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
	𐡄𐡍 (res != NSERROR_OK) {
		أرجع false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	𐡄𐡍 (res != NSERROR_OK) {
		أرجع false;
	}

	option = control->data.select.items;
	item_y = line_height_with_spacing;

	طالما (item_y < scroll) {
		option = option->next;
		item_y += line_height_with_spacing;
	}
	item_y -= line_height_with_spacing;
	text_pos_offset = y - scroll +
			(صحيح) (line_height * (0.75 + SELECT_LINE_SPACING));
	text_x = x + (box->border[LEFT].width + box->padding[LEFT]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	طالما (option && item_y - scroll < height) {

		𐡄𐡍 (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + line_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			𐡄𐡍 (res != NSERROR_OK) {
				أرجع false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		𐡄𐡍 (res != NSERROR_OK) {
			أرجع false;
		}

		item_y += line_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	𐡄𐡍 (res != NSERROR_OK) {
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
	بنية form_select_menu *menu = control->data.select.menu;
	صحيح width, height;


	width = menu->width;
	height = menu->height;

	𐡄𐡍 (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	𐡄𐡍 (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		أرجع true;

	أرجع false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(بنية form_control *control, صحيح item)
{
	أكد(control != عدم);

	أرجع form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
بنية form_option *
form_select_get_option(بنية form_control *control, صحيح item)
{
	بنية form_option *opt;

	opt = control->data.select.items;
	طالما ((opt != عدم) && (item > 0)) {
		opt = opt->next;
		item--;
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
	بنية form_select_menu *menu = control->data.select.menu;
	صحيح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *status = عدم;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	𐡄𐡍 (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		أرجع scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	𐡄𐡍 (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		𐡄𐡍 (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		𐡄𐡍 (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} وإلا 𐡄𐡍 (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	أرجع status;
}


/* private interface described in html/form_internal.h */
فراغ
form_select_mouse_drag_end(بنية form_control *control,
			   browser_mouse_state mouse,
			   صحيح x, صحيح y)
{
	صحيح x0, y0, x1, y1;
	صحيح box_x, box_y;
	بنية box *box;
	بنية form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[LEFT].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[TOP];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	𐡄𐡍 (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		أرجع;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	𐡄𐡍 (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *width, صحيح *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
فراغ form_select_menu_callback(فراغ *client_data,
		صحيح x, صحيح y, صحيح width, صحيح height)
{
	html_content *html = client_data;
	صحيح menu_x, menu_y;
	بنية box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[LEFT].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[TOP];
	content__request_redraw((بنية content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
فراغ form_radio_set(بنية form_control *radio)
{
	بنية form_control *control;

	أكد(radio);
	𐡄𐡍 (!radio->form)
		أرجع;

	𐡄𐡍 (radio->selected)
		أرجع;

	/* Clear selected state for other controls in
	 * the same radio button group */
	لكل (control = radio->form->controls;
	     control != عدم;
	     control = control->next) {
		/* Only interested in radio inputs */
		𐡄𐡍 (control->type != GADGET_RADIO)
			استمر;

		/* Ignore ourself */
		𐡄𐡍 (control == radio)
			استمر;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		𐡄𐡍 ((control->name == عدم) ||
		    (radio->name == عدم) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			استمر;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		𐡄𐡍 (control->selected) {
			control->selected = false;
			dom_html_input_element_set_checked(control->node, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_input_element_set_checked(radio->node, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    بنية browser_window *target,
	    بنية form *form,
	    بنية form_control *submit_button)
{
	nserror res;
	حرف *data = عدم; /* encoded form data */
	بنية fetch_multipart_data *success = عدم; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != عدم);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	𐡄𐡍 (res != NSERROR_OK) {
		أرجع res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	𐡄𐡍 (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		أرجع res;
	}

	بدل (form->method) {
	حالة method_GET:
		res = form_url_encode(form, success, &data);
		𐡄𐡍 (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			𐡄𐡍 (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      عدم,
							      عدم,
							      عدم);

				nsurl_unref(query_url);
			}
			حرر(data);
		}
		اكسر;

	حالة method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		𐡄𐡍 (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      عدم,
						      عدم);
			حرر(data);
		}
		اكسر;

	حالة method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      عدم,
					      success,
					      عدم);

		اكسر;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	أرجع res;
}


/* exported interface documented in html/form_internal.h */
فراغ form_gadget_update_value(بنية form_control *control, حرف *value)
{
	بدل (control->type) {
	حالة GADGET_HIDDEN:
	حالة GADGET_TEXTBOX:
	حالة GADGET_TEXTAREA:
	حالة GADGET_PASSWORD:
	حالة GADGET_FILE:
		𐡄𐡍 (control->value != عدم) {
			حرر(control->value);
		}
		control->value = value;
		𐡄𐡍 (control->node != عدم) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)value,
						strlen(value), &str);
			𐡄𐡍 (err == DOM_NO_ERR) {
				𐡄𐡍 (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_value(
						(dom_html_text_area_element *)(control->node),
						str);
				وإلا
					err = dom_html_input_element_set_value(
						(dom_html_input_element *)(control->node),
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
	dom_string *value = عدم;
	bool changed_dom = false;

	𐡄𐡍 (control->syncing ||
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
	𐡄𐡍 ((control->last_synced_value == عدم &&
	     control->value != عدم &&
	     control->value[0] != '\0') ||
	    (control->last_synced_value != عدم &&
	     control->value != عدم &&
	     strcmp(control->value, control->last_synced_value) != 0)) {
		حرف *dup = strdup(control->value);
		𐡄𐡍 (dup == عدم) {
			اذهب out;
		}
		𐡄𐡍 (control->last_synced_value != عدم) {
			حرر(control->last_synced_value);
		}
		control->last_synced_value = dup;
		exc = dom_string_create((uint8_t *)(control->value),
					strlen(control->value), &value);
		𐡄𐡍 (exc != DOM_NO_ERR) {
			اذهب out;
		}
		𐡄𐡍 (control->node_value != عدم) {
			dom_string_unref(control->node_value);
		}
		control->node_value = value;
		value = عدم;
		𐡄𐡍 (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_value(control->node, control->node_value);
		} وإلا {
			exc = dom_html_input_element_set_value(control->node, control->node_value);
		}
		𐡄𐡍 (exc != DOM_NO_ERR) {
			اذهب out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	𐡄𐡍 (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_value(control->node, &value);
	} وإلا {
		exc = dom_html_input_element_get_value(control->node, &value);
	}

	𐡄𐡍 (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		اذهب out;
	}

	𐡄𐡍 (!dom_string_isequal(control->node_value, value)) {
		/* The DOM has changed */
		𐡄𐡍 (!changed_dom) {
			/* And it wasn't us */
			حرف *value_s = strndup(
				dom_string_data(value),
				dom_string_byte_length(value));
			حرف *dup = عدم;
			𐡄𐡍 (value_s == عدم) {
				اذهب out;
			}
			dup = strdup(value_s);
			𐡄𐡍 (dup == عدم) {
				حرر(value_s);
				اذهب out;
			}
			حرر(control->value);
			control->value = value_s;
			حرر(control->last_synced_value);
			control->last_synced_value = dup;
			𐡄𐡍 (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != عدم) {
				textarea_set_text(control->data.text.ta,
						  value_s);
			}
		}
		control->node_value = value;
		value = عدم;
	}

out:
	𐡄𐡍 (value != عدم)
		dom_string_unref(value);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
بنية form *
form_new(فراغ *node,
	 ثابت حرف *action,
	 ثابت حرف *target,
	 form_method method,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	بنية form *form;

	form = calloc(1, حجم *form);
	𐡄𐡍 (!form)
		أرجع عدم;

	form->action = strdup(action != عدم ? action : "");
	𐡄𐡍 (form->action == عدم) {
		حرر(form);
		أرجع عدم;
	}

	form->target = target != عدم ? strdup(target) : عدم;
	𐡄𐡍 (target != عدم && form->target == عدم) {
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->method = method;

	form->accept_charsets = charset != عدم ? strdup(charset) : عدم;
	𐡄𐡍 (charset != عدم && form->accept_charsets == عدم) {
		حرر(form->target);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->document_charset = doc_charset != عدم ? strdup(doc_charset)
						     : عدم;
	𐡄𐡍 (doc_charset && form->document_charset == عدم) {
		حرر(form->accept_charsets);
		حرر(form->target);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->node = node;

	أرجع form;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free(بنية form *form)
{
	بنية form_control *c, *d;

	لكل (c = form->controls; c != عدم; c = d) {
		d = c->next;

		form_free_control(c);
	}

	حرر(form->action);
	حرر(form->target);
	حرر(form->accept_charsets);
	حرر(form->document_charset);

	حرر(form);
}


/* exported interface documented in html/form_internal.h */
بنية form_control *form_new_control(فراغ *node, form_control_type type)
{
	بنية form_control *control;

	control = calloc(1, حجم *control);
	𐡄𐡍 (control == عدم)
		أرجع عدم;

	control->node = node;
	control->type = type;

	أرجع control;
}
