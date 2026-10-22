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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/url.h"
#அந்தர்பாவ "utils/utf8.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/inttypes.h"
#அந்தர்பாவ "netsurf/mouse.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "content/fetch.h"
#அந்தர்பாவ "content/hlcache.h"
#அந்தர்பாவ "css/utils.h"
#அந்தர்பாவ "desktop/knockout.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/textarea.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/layout.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_வரி_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

கட்டமைப்பு form_select_menu {
	முழு வரி_height;
	முழு width, height;
	கட்டமைப்பு scrollbar *scrollbar;
	முழு f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	வெறுமை *client_data;
	கட்டமைப்பு content *c;
};

நிலையான plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

நிலையான plot_font_style_t plot_fstyle_entry = {
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
நிலையான எழுத்து *
form_encode_item(மாறிலி எழுத்து *item,
		 uint32_t len,
		 மாறிலி எழுத்து *charset,
		 மாறிலி எழுத்து *fallback)
{
	nserror err;
	எழுத்து *ret = வெற்று;
	எழுத்து cset[256];

	எனில் (!item || !charset)
		திரும்பு வெற்று;

	snprintf(cset, அளவு cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	எனில் (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, அளவு cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		எனில் (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			எனில் (fallback) {
				snprintf(cset, அளவு cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				எனில் (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, அளவு cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			எனில் (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				எனில் (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	எனில் (err == NSERROR_NOMEM) {
		திரும்பு வெற்று;
	}

	திரும்பு ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_மதிப்பு_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
நிலையான nserror
fetch_data_list_add_sname(மாறிலி எழுத்து *name,
			  மாறிலி எழுத்து *ksfx,
			  முழு மதிப்பு,
			  கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	கட்டமைப்பு fetch_multipart_data *fetch_data;
	முழு keysize;

	fetch_data = calloc(1, அளவு(*fetch_data));
	எனில் (fetch_data == வெற்று) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		திரும்பு NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	எனில் (fetch_data->name == வெற்று) {
		விடுவி(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		திரும்பு NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*மதிப்பு*/
	fetch_data->மதிப்பு = malloc(FETCH_DATA_INT_மதிப்பு_SIZE);
	எனில் (fetch_data->மதிப்பு == வெற்று) {
		விடுவி(fetch_data->name);
		விடுவி(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		திரும்பு NSERROR_NOMEM;
	}
	snprintf(fetch_data->மதிப்பு, FETCH_DATA_INT_மதிப்பு_SIZE, "%d", மதிப்பு);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	திரும்பு NSERROR_OK;
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
நிலையான nserror
fetch_data_list_add(dom_string *name,
		    dom_string *மதிப்பு,
		    மாறிலி எழுத்து *rawfile,
		    மாறிலி எழுத்து *form_charset,
		    மாறிலி எழுத்து *docu_charset,
		    கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	கட்டமைப்பு fetch_multipart_data *fetch_data;

	ப்ரதிஜ்ஞா(name != வெற்று);

	fetch_data = calloc(1, அளவு(*fetch_data));
	எனில் (fetch_data == வெற்று) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		திரும்பு NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	எனில் (fetch_data->name == வெற்று) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		விடுவி(fetch_data);
		திரும்பு NSERROR_NOMEM;
	}

	எனில் (மதிப்பு == வெற்று) {
		fetch_data->மதிப்பு = strdup("");
	} இல்லையெனில் {
		fetch_data->மதிப்பு = form_encode_item(dom_string_data(மதிப்பு),
						     dom_string_byte_length(மதிப்பு),
						     form_charset,
						     docu_charset);
	}
	எனில் (fetch_data->மதிப்பு == வெற்று) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		விடுவி(fetch_data->name);
		விடுவி(fetch_data);
		திரும்பு NSERROR_NOMEM;
	}

	/* deal with raw file name */
	எனில் (rawfile != வெற்று) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		எனில் (fetch_data->rawfile == வெற்று) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			விடுவி(fetch_data->மதிப்பு);
			விடுவி(fetch_data->name);
			விடுவி(fetch_data);
			திரும்பு NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	திரும்பு NSERROR_OK;
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
நிலையான nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  மாறிலி எழுத்து *form_charset,
			  மாறிலி எழுத்து *doc_charset,
			  கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (element_disabled) {
		/* allow enumeration to continue after disabled element */
		திரும்பு NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (inputname == வெற்று) {
		/* allow enumeration to continue after element with no name */
		திரும்பு NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_மதிப்பு(text_area_element,
						   &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		திரும்பு NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  வெற்று,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      மாறிலி எழுத்து *form_charset,
			      மாறிலி எழுத்து *docu_charset,
			      கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *மதிப்பு;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		திரும்பு NSERROR_DOM;
	}

	எனில் (!selected) {
		/* unselected options do not add fetch data entries */
		திரும்பு NSERROR_OK;
	}

	exp = dom_html_option_element_get_மதிப்பு(option_element, &மதிப்பு);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		திரும்பு NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  மதிப்பு,
				  வெற்று,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(மதிப்பு);

	திரும்பு res;
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
நிலையான nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			மாறிலி எழுத்து *form_charset,
			மாறிலி எழுத்து *doc_charset,
			கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = வெற்று;
	uint32_t options_மொத்தம்;
	uint32_t option_index;
	dom_முனை *option_element = வெற்று;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (element_disabled) {
		/* allow enumeration to continue after disabled element */
		திரும்பு NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (inputname == வெற்று) {
		/* allow enumeration to continue after element with no name */
		திரும்பு NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		திரும்பு NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_மொத்தம்);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		திரும்பு NSERROR_DOM;
	}

	/* iterate over options collection */
	க்கு (option_index = 0; option_index < options_மொத்தம்; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		எனில் (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} இல்லையெனில் {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_முனை_unref(option_element);
		}

		எனில் (res != NSERROR_OK) {
			நிறுத்து;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_உள்ளீடு_submit(dom_html_உள்ளீடு_element *உள்ளீடு_element,
			      dom_string *inputname,
			      மாறிலி எழுத்து *charset,
			      மாறிலி எழுத்து *document_charset,
			      dom_html_element **submit_button,
			      கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	எனில் (*submit_button == வெற்று) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)உள்ளீடு_element;
	} இல்லையெனில் எனில் (*submit_button != (dom_html_element *)உள்ளீடு_element) {
		திரும்பு NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_உள்ளீடு_element_get_மதிப்பு(உள்ளீடு_element, &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		திரும்பு NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  வெற்று,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_உள்ளீடு_image(dom_html_உள்ளீடு_element *உள்ளீடு_element,
			     dom_string *inputname,
			     மாறிலி எழுத்து *charset,
			     மாறிலி எழுத்து *document_charset,
			     dom_html_element **submit_button,
			     கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	கட்டமைப்பு image_உள்ளீடு_coords *coords;
	எழுத்து *basename;

	/* Only use an image input if it was the thing which activated us */
	எனில் (*submit_button != (dom_html_element *)உள்ளீடு_element) {
		திரும்பு NSERROR_OK;
	}

	exp = dom_முனை_get_user_data((dom_முனை *)உள்ளீடு_element,
				     corestring_dom_ns_சாவி_image_coords_முனை_data,
				     &coords);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		திரும்பு NSERROR_DOM;
	}

	எனில் (coords == வெற்று) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		திரும்பு NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	எனில் (basename == வெற்று) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		திரும்பு NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	எனில் (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	விடுவி(basename);

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_உள்ளீடு_checkbox(dom_html_உள்ளீடு_element *உள்ளீடு_element,
				dom_string *inputname,
				மாறிலி எழுத்து *charset,
				மாறிலி எழுத்து *document_charset,
				கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_உள்ளீடு_element_get_checked(உள்ளீடு_element, &checked);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		திரும்பு NSERROR_DOM;
	}

	எனில் (!checked) {
		/* unchecked items do not generate a data entry */
		திரும்பு NSERROR_OK;
	}

	exp = dom_html_உள்ளீடு_element_get_மதிப்பு(உள்ளீடு_element, &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		திரும்பு NSERROR_DOM;
	}

	/* ensure a default value */
	எனில் (inputvalue == வெற்று) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  வெற்று,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_உள்ளீடு_file(dom_html_உள்ளீடு_element *உள்ளீடு_element,
			    dom_string *inputname,
			    மாறிலி எழுத்து *charset,
			    மாறிலி எழுத்து *document_charset,
			    கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	மாறிலி எழுத்து *rawfile = வெற்று;

	exp = dom_html_உள்ளீடு_element_get_மதிப்பு(உள்ளீடு_element, &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		திரும்பு NSERROR_DOM;
	}

	exp = dom_முனை_get_user_data((dom_முனை *)உள்ளீடு_element,
				     corestring_dom_ns_சாவி_file_name_முனை_data,
				     &rawfile);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		திரும்பு NSERROR_DOM;
	}

	எனில் (rawfile == வெற்று) {
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

	திரும்பு res;
}


நிலையான nserror
form_dom_to_data_உள்ளீடு_text(dom_html_உள்ளீடு_element *உள்ளீடு_element,
			    dom_string *inputname,
			    மாறிலி எழுத்து *charset,
			    மாறிலி எழுத்து *document_charset,
			    கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_உள்ளீடு_element_get_மதிப்பு(உள்ளீடு_element, &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		திரும்பு NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  வெற்று,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	திரும்பு res;
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
நிலையான nserror
form_dom_to_data_உள்ளீடு(dom_html_உள்ளீடு_element *உள்ளீடு_element,
		       மாறிலி எழுத்து *charset,
		       மாறிலி எழுத்து *document_charset,
		       dom_html_element **submit_button,
		       கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_உள்ளீடு_element_get_disabled(உள்ளீடு_element,
						  &element_disabled);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (element_disabled) {
		/* disabled element requires no more processing */
		திரும்பு NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_உள்ளீடு_element_get_name(உள்ளீடு_element, &inputname);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (inputname == வெற்று) {
		/* element with no name is not converted */
		திரும்பு NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_உள்ளீடு_element_get_type(உள்ளீடு_element, &inputtype);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		திரும்பு NSERROR_DOM;
	}

	/* process according to input element type */
	எனில் (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_உள்ளீடு_submit(உள்ளீடு_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} இல்லையெனில் எனில் (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_உள்ளீடு_image(உள்ளீடு_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} இல்லையெனில் எனில் (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_உள்ளீடு_checkbox(உள்ளீடு_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} இல்லையெனில் எனில் (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_உள்ளீடு_file(உள்ளீடு_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} இல்லையெனில் எனில் (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} இல்லையெனில் {
		/* Everything else is treated as text values */
		res = form_dom_to_data_உள்ளீடு_text(உள்ளீடு_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	திரும்பு res;
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
நிலையான nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			மாறிலி எழுத்து *form_charset,
			மாறிலி எழுத்து *doc_charset,
			dom_html_element **submit_button,
			கட்டமைப்பு fetch_multipart_data ***fetch_data_next_ptr)
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
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (element_disabled) {
		/* allow enumeration to continue after disabled element */
		திரும்பு NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		திரும்பு NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	எனில் (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		திரும்பு NSERROR_OK;
	}
	எனில் (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		திரும்பு NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	எனில் (*submit_button == வெற்று) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	எனில் (*submit_button != (dom_html_element *)button_element) {
		திரும்பு NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		திரும்பு NSERROR_DOM;
	}

	எனில் (inputname == வெற்று) {
		/* allow enumeration to continue after element with no name */
		திரும்பு NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_மதிப்பு(button_element, &inputvalue);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} இல்லையெனில் {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  வெற்று,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	திரும்பு res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
நிலையான எழுத்து *form_acceptable_charset(கட்டமைப்பு form *form)
{
	எழுத்து *temp, *c;

	எனில் (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		எனில் (form->document_charset) {
			/* document charset present, so use it */
			திரும்பு strdup(form->document_charset);
		} இல்லையெனில் {
			/* no document charset, so default to 8859-1 */
			திரும்பு strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	எனில் (!temp)
		திரும்பு வெற்று;

	/* make it upper case */
	க்கு (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	எனில் (c) {
		விடுவி(temp);
		திரும்பு strdup("UTF-8");
	}

	/* dispense with temporary copy */
	விடுவி(temp);

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
	போது (*c && !ascii_is_space(*c)) {
		எனில் (*c == ',')
			நிறுத்து;
		c++;
	}

	திரும்பு strndup(form->accept_charsets, c - form->accept_charsets);
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
நிலையான nserror
form_dom_to_data(கட்டமைப்பு form *form,
		 கட்டமைப்பு form_control *submit_control,
		 கட்டமைப்பு fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	எழுத்து *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = வெற்று; /* the dom form elements */
	uint32_t element_மொத்தம்; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_முனை *element = வெற்று; /* the DOM form element */
	dom_string *nodename = வெற்று; /* the DOM node name of the element */
	கட்டமைப்பு fetch_multipart_data *fetch_data = வெற்று; /* fetch data list */
	கட்டமைப்பு fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	எனில் (submit_control != வெற்று) {
		submit_button = submit_control->முனை;
	} இல்லையெனில் {
		submit_button = வெற்று;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	எனில் (charset == வெற்று) {
		NSLOG(netsurf, INFO, "failed to find charset");
		திரும்பு NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->முனை, &elements);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		விடுவி(charset);
		திரும்பு NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_மொத்தம்);
	எனில் (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		செல் form_dom_to_data_error;
	}

	க்கு (element_idx = 0; element_idx < element_மொத்தம்; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		எனில் (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			செல் form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_முனை_get_முனை_name(element, &nodename);
		எனில் (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_முனை_unref(element);
			res = NSERROR_DOM;
			செல் form_dom_to_data_error;
		}

		எனில் (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} இல்லையெனில் எனில் (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} இல்லையெனில் எனில் (dom_string_isequal(nodename, corestring_dom_உள்ளீடு)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_உள்ளீடு(
				(dom_html_உள்ளீடு_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} இல்லையெனில் எனில் (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} இல்லையெனில் {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (முழு)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_முனை_unref(element);

		/* abort form element enumeration on error */
		எனில் (res != NSERROR_OK) {
			செல் form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	விடுவி(charset);

	திரும்பு NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	விடுவி(charset);

	திரும்பு res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
நிலையான nserror
form_url_encode(கட்டமைப்பு form *form,
		கட்டமைப்பு fetch_multipart_data *control,
		எழுத்து **encoded_out)
{
	எழுத்து *name, *மதிப்பு;
	எழுத்து *s, *s2;
	unsigned முழு len, len1, len_init;
	nserror res;

	s = malloc(1);

	எனில் (s == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	க்கு (; control; control = control->next) {
		res = url_escape(control->name, true, வெற்று, &name);
		எனில் (res != NSERROR_OK) {
			விடுவி(s);
			திரும்பு res;
		}

		res = url_escape(control->மதிப்பு, true, வெற்று, &மதிப்பு);
		எனில் (res != NSERROR_OK) {
			விடுவி(name);
			விடுவி(s);
			திரும்பு res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(மதிப்பு) + 2;
		s2 = realloc(s, len1 + 1);
		எனில் (s2 == வெற்று) {
			விடுவி(மதிப்பு);
			விடுவி(name);
			விடுவி(s);
			திரும்பு NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, மதிப்பு);
		len = len1;
		விடுவி(name);
		விடுவி(மதிப்பு);
	}

	எனில் (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	திரும்பு NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
நிலையான வெறுமை
form_select_menu_scroll_callback(வெறுமை *client_data,
				 கட்டமைப்பு scrollbar_செய்தி_data *scrollbar_data)
{
	கட்டமைப்பு form_control *control = client_data;
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	தேர்வு (scrollbar_data->செய்தி) {
		நிலை SCROLLBAR_செய்தி_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			நிறுத்து;
		நிலை SCROLLBAR_செய்தி_SCROLL_START:
		{
			கட்டமைப்பு rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			நிறுத்து;
		நிலை SCROLLBAR_செய்தி_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, வெற்று);
			நிறுத்து;
		இயல்பு:
			நிறுத்து;
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
நிலையான nserror
form__select_process_selection(html_content *html,
			       கட்டமைப்பு form_control *control,
			       முழு item)
{
	கட்டமைப்பு box *inline_box;
	கட்டமைப்பு form_option *o;
	முழு மொத்தம்;
	nserror ret = NSERROR_OK;

	ப்ரதிஜ்ஞா(control != வெற்று);
	ப்ரதிஜ்ஞா(html != வெற்று);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	க்கு (மொத்தம் = 0, o = control->data.select.பொருட்கள்;
			o != வெற்று;
			மொத்தம்++, o = o->next) {
		எனில் (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->முனை, false);
		}

		எனில் (மொத்தம் == item) {
			எனில் (control->data.select.multiple) {
				எனில் (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->முனை, false);
					control->data.select.இலக்கம்_selected--;
				} இல்லையெனில் {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->முனை, true);
					control->data.select.இலக்கம்_selected++;
				}
			} இல்லையெனில் {
				dom_html_option_element_set_selected(
						o->முனை, true);
				o->selected = true;
			}
		}

		எனில் (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	எனில் (control->data.select.இலக்கம்_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} இல்லையெனில் எனில் (control->data.select.இலக்கம்_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} இல்லையெனில் {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	எனில் (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} இல்லையெனில் {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	திரும்பு ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
நிலையான வெறுமை form_select_menu_clicked(கட்டமைப்பு form_control *control, முழு x, முழு y)
{
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;
	கட்டமைப்பு form_option *option;
	html_content *html = (html_content *)menu->c;
	முழு வரி_height, வரி_height_with_spacing;
	முழு item_bottom_y;
	முழு scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	வரி_height = menu->வரி_height;
	வரி_height_with_spacing = வரி_height +
			வரி_height * SELECT_வரி_SPACING;

	option = control->data.select.பொருட்கள்;
	item_bottom_y = வரி_height_with_spacing;
	i = 0;
	போது (option && item_bottom_y < scroll + y) {
		item_bottom_y += வரி_height_with_spacing;
		option = option->next;
		i++;
	}

	எனில் (option != வெற்று) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
வெறுமை form_add_control(கட்டமைப்பு form *form, கட்டமைப்பு form_control *control)
{
	எனில் (form == வெற்று) {
		திரும்பு;
	}

	control->form = form;

	எனில் (form->controls != வெற்று) {
		ப்ரதிஜ்ஞா(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = வெற்று;
		form->last_control = control;
	} இல்லையெனில் {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
வெறுமை form_free_control(கட்டமைப்பு form_control *control)
{
	கட்டமைப்பு form_control *c;
	ப்ரதிஜ்ஞா(control != வெற்று);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->மதிப்பு, control->initial_மதிப்பு);
	விடுவி(control->name);
	விடுவி(control->மதிப்பு);
	விடுவி(control->initial_மதிப்பு);
	எனில் (control->last_synced_மதிப்பு != வெற்று) {
		விடுவி(control->last_synced_மதிப்பு);
	}

	எனில் (control->type == GADGET_SELECT) {
		கட்டமைப்பு form_option *option, *next;

		க்கு (option = control->data.select.பொருட்கள்; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->மதிப்பு);
			விடுவி(option->text);
			விடுவி(option->மதிப்பு);
			விடுவி(option);
		}
		எனில் (control->data.select.menu != வெற்று) {
			form_free_select_menu(control);
		}
	}

	எனில் (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		எனில் (control->data.text.initial != வெற்று) {
			dom_string_unref(control->data.text.initial);
		}

		எனில் (control->data.text.ta != வெற்று) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	எனில் (control->form != வெற்று) {
		க்கு (c = control->form->controls; c != வெற்று; c = c->next) {
			எனில் (c->next == control) {
				c->next = control->next;
				எனில் (control->form->last_control == control)
					control->form->last_control = c;
				நிறுத்து;
			}
			எனில் (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				எனில் (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = வெற்று;
				நிறுத்து;
			}
		}
	}

	எனில் (control->முனை_மதிப்பு != வெற்று) {
		dom_string_unref(control->முனை_மதிப்பு);
	}

	விடுவி(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(கட்டமைப்பு form_control *control, எழுத்து *மதிப்பு, எழுத்து *text,
		     bool selected, வெறுமை *முனை)
{
	கட்டமைப்பு form_option *option;

	ப்ரதிஜ்ஞா(control);
	ப்ரதிஜ்ஞா(control->type == GADGET_SELECT);

	option = calloc(1, அளவு *option);
	எனில் (!option)
		திரும்பு false;

	option->மதிப்பு = மதிப்பு;
	option->text = text;

	/* add to linked list */
	எனில் (control->data.select.பொருட்கள் == 0)
		control->data.select.பொருட்கள் = option;
	இல்லையெனில்
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	எனில் (selected && (control->data.select.இலக்கம்_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.இலக்கம்_selected++;
		control->data.select.current = option;
	}

	control->data.select.இலக்கம்_பொருட்கள்++;

	option->முனை = முனை;

	திரும்பு true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(வெறுமை *client_data,
		      கட்டமைப்பு form_control *control,
		      select_menu_redraw_callback callback,
		      கட்டமைப்பு content *c)
{
	முழு வரி_height_with_spacing;
	கட்டமைப்பு box *box;
	plot_font_style_t fstyle;
	முழு total_height;
	கட்டமைப்பு form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	எனில் (control->data.select.menu == வெற்று) {

		menu = calloc(1, அளவு (கட்டமைப்பு form_select_menu));
		எனில் (menu == வெற்று) {
			திரும்பு NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[வலது_2].width + box->padding[வலது_2] +
			box->border[இடது_2].width + box->padding[இடது_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->வரி_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		வரி_height_with_spacing = menu->வரி_height +
				menu->வரி_height *
				SELECT_வரி_SPACING;

		total_height = control->data.select.இலக்கம்_பொருட்கள் *
				வரி_height_with_spacing;
		menu->height = total_height;

		எனில் (menu->height > MAX_SELECT_HEIGHT) {
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
		எனில் (res != NSERROR_OK) {
			control->data.select.menu = வெற்று;
			விடுவி(menu);
			திரும்பு res;
		}
		menu->c = c;
	} இல்லையெனில் {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	திரும்பு NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
வெறுமை form_free_select_menu(கட்டமைப்பு form_control *control)
{
	எனில் (control->data.select.menu->scrollbar != வெற்று)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	விடுவி(control->data.select.menu);
	control->data.select.menu = வெற்று;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(கட்டமைப்பு form_control *control,
			முழு x, முழு y,
			தசம scale,
			மாறிலி கட்டமைப்பு rect *clip,
			மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு box *box;
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;
	கட்டமைப்பு form_option *option;
	முழு வரி_height, வரி_height_with_spacing;
	முழு width, height;
	முழு x0, y0, x1, scrollbar_x, y1, y2, y3;
	முழு item_y;
	முழு text_pos_offset, text_x;
	முழு scrollbar_width = SCROLLBAR_WIDTH;
	முழு i;
	முழு scroll;
	முழு x_cp, y_cp;
	கட்டமைப்பு rect r;
	கட்டமைப்பு rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	வரி_height = menu->வரி_height;

	வரி_height_with_spacing = வரி_height +
			வரி_height * SELECT_வரி_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	எனில் (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / வரி_height_with_spacing;
		scroll -= i * வரி_height_with_spacing;
		வரி_height *= scale;
		வரி_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * வரி_height_with_spacing;
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
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
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
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	option = control->data.select.பொருட்கள்;
	item_y = வரி_height_with_spacing;

	போது (item_y < scroll) {
		option = option->next;
		item_y += வரி_height_with_spacing;
	}
	item_y -= வரி_height_with_spacing;
	text_pos_offset = y - scroll +
			(முழு) (வரி_height * (0.75 + SELECT_வரி_SPACING));
	text_x = x + (box->border[இடது_2].width + box->padding[இடது_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	போது (option && item_y - scroll < height) {

		எனில் (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + வரி_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}

		item_y += வரி_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	திரும்பு true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(கட்டமைப்பு form_control *control,
			     தசம scale,
			     மாறிலி கட்டமைப்பு rect *clip)
{
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;
	முழு width, height;


	width = menu->width;
	height = menu->height;

	எனில் (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	எனில் (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		திரும்பு true;

	திரும்பு false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(கட்டமைப்பு form_control *control, முழு item)
{
	ப்ரதிஜ்ஞா(control != வெற்று);

	திரும்பு form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
கட்டமைப்பு form_option *
form_select_get_option(கட்டமைப்பு form_control *control, முழு item)
{
	கட்டமைப்பு form_option *opt;

	opt = control->data.select.பொருட்கள்;
	போது ((opt != வெற்று) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	திரும்பு opt;
}


/* exported interface documented in netsurf/form.h */
எழுத்து *form_control_get_name(கட்டமைப்பு form_control *control)
{
	திரும்பு control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(கட்டமைப்பு form_control *control, கட்டமைப்பு rect *r)
{
	box_bounds( control->box, r );
	திரும்பு NSERROR_OK;
}


/* private interface described in html/form_internal.h */
மாறிலி எழுத்து *
form_select_mouse_action(கட்டமைப்பு form_control *control,
			 browser_mouse_state mouse,
			 முழு x, முழு y)
{
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;
	முழு x0, y0, x1, y1, scrollbar_x;
	மாறிலி எழுத்து *status = வெற்று;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	எனில் (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		திரும்பு scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	எனில் (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		எனில் (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		எனில் (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} இல்லையெனில் எனில் (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	திரும்பு status;
}


/* private interface described in html/form_internal.h */
வெறுமை
form_select_mouse_drag_end(கட்டமைப்பு form_control *control,
			   browser_mouse_state mouse,
			   முழு x, முழு y)
{
	முழு x0, y0, x1, y1;
	முழு box_x, box_y;
	கட்டமைப்பு box *box;
	கட்டமைப்பு form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[இடது_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[உச்சி_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	எனில் (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		திரும்பு;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	எனில் (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
வெறுமை form_select_get_dimensions(கட்டமைப்பு form_control *control,
		முழு *width, முழு *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
வெறுமை form_select_menu_callback(வெறுமை *client_data,
		முழு x, முழு y, முழு width, முழு height)
{
	html_content *html = client_data;
	முழு menu_x, menu_y;
	கட்டமைப்பு box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[இடது_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[உச்சி_2];
	content__request_redraw((கட்டமைப்பு content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
வெறுமை form_radio_set(கட்டமைப்பு form_control *radio)
{
	கட்டமைப்பு form_control *control;

	ப்ரதிஜ்ஞா(radio);
	எனில் (!radio->form)
		திரும்பு;

	எனில் (radio->selected)
		திரும்பு;

	/* Clear selected state for other controls in
	 * the same radio button group */
	க்கு (control = radio->form->controls;
	     control != வெற்று;
	     control = control->next) {
		/* Only interested in radio inputs */
		எனில் (control->type != GADGET_RADIO)
			தொடர்;

		/* Ignore ourself */
		எனில் (control == radio)
			தொடர்;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		எனில் ((control->name == வெற்று) ||
		    (radio->name == வெற்று) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			தொடர்;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		எனில் (control->selected) {
			control->selected = false;
			dom_html_உள்ளீடு_element_set_checked(control->முனை, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_உள்ளீடு_element_set_checked(radio->முனை, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    கட்டமைப்பு browser_window *target,
	    கட்டமைப்பு form *form,
	    கட்டமைப்பு form_control *submit_button)
{
	nserror res;
	எழுத்து *data = வெற்று; /* encoded form data */
	கட்டமைப்பு fetch_multipart_data *success = வெற்று; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ப்ரதிஜ்ஞா(form != வெற்று);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	எனில் (res != NSERROR_OK) {
		திரும்பு res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	எனில் (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		திரும்பு res;
	}

	தேர்வு (form->method) {
	நிலை method_GET:
		res = form_url_encode(form, success, &data);
		எனில் (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			எனில் (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      வெற்று,
							      வெற்று,
							      வெற்று);

				nsurl_unref(query_url);
			}
			விடுவி(data);
		}
		நிறுத்து;

	நிலை method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		எனில் (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      வெற்று,
						      வெற்று);
			விடுவி(data);
		}
		நிறுத்து;

	நிலை method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      வெற்று,
					      success,
					      வெற்று);

		நிறுத்து;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	திரும்பு res;
}


/* exported interface documented in html/form_internal.h */
வெறுமை form_gadget_update_மதிப்பு(கட்டமைப்பு form_control *control, எழுத்து *மதிப்பு)
{
	தேர்வு (control->type) {
	நிலை GADGET_HIDDEN:
	நிலை GADGET_TEXTBOX:
	நிலை GADGET_TEXTAREA:
	நிலை GADGET_PASSWORD:
	நிலை GADGET_FILE:
		எனில் (control->மதிப்பு != வெற்று) {
			விடுவி(control->மதிப்பு);
		}
		control->மதிப்பு = மதிப்பு;
		எனில் (control->முனை != வெற்று) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)மதிப்பு,
						strlen(மதிப்பு), &str);
			எனில் (err == DOM_NO_ERR) {
				எனில் (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_மதிப்பு(
						(dom_html_text_area_element *)(control->முனை),
						str);
				இல்லையெனில்
					err = dom_html_உள்ளீடு_element_set_மதிப்பு(
						(dom_html_உள்ளீடு_element *)(control->முனை),
						str);
				dom_string_unref(str);
			}
		}
		நிறுத்து;
	இயல்பு:
		/* Do nothing */
		நிறுத்து;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
வெறுமை
form_gadget_sync_with_dom(கட்டமைப்பு form_control *control)
{
	dom_exception exc;
	dom_string *மதிப்பு = வெற்று;
	bool changed_dom = false;

	எனில் (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		திரும்பு;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	எனில் ((control->last_synced_மதிப்பு == வெற்று &&
	     control->மதிப்பு != வெற்று &&
	     control->மதிப்பு[0] != '\0') ||
	    (control->last_synced_மதிப்பு != வெற்று &&
	     control->மதிப்பு != வெற்று &&
	     strcmp(control->மதிப்பு, control->last_synced_மதிப்பு) != 0)) {
		எழுத்து *dup = strdup(control->மதிப்பு);
		எனில் (dup == வெற்று) {
			செல் out;
		}
		எனில் (control->last_synced_மதிப்பு != வெற்று) {
			விடுவி(control->last_synced_மதிப்பு);
		}
		control->last_synced_மதிப்பு = dup;
		exc = dom_string_create((uint8_t *)(control->மதிப்பு),
					strlen(control->மதிப்பு), &மதிப்பு);
		எனில் (exc != DOM_NO_ERR) {
			செல் out;
		}
		எனில் (control->முனை_மதிப்பு != வெற்று) {
			dom_string_unref(control->முனை_மதிப்பு);
		}
		control->முனை_மதிப்பு = மதிப்பு;
		மதிப்பு = வெற்று;
		எனில் (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_மதிப்பு(control->முனை, control->முனை_மதிப்பு);
		} இல்லையெனில் {
			exc = dom_html_உள்ளீடு_element_set_மதிப்பு(control->முனை, control->முனை_மதிப்பு);
		}
		எனில் (exc != DOM_NO_ERR) {
			செல் out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	எனில் (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_மதிப்பு(control->முனை, &மதிப்பு);
	} இல்லையெனில் {
		exc = dom_html_உள்ளீடு_element_get_மதிப்பு(control->முனை, &மதிப்பு);
	}

	எனில் (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		செல் out;
	}

	எனில் (!dom_string_isequal(control->முனை_மதிப்பு, மதிப்பு)) {
		/* The DOM has changed */
		எனில் (!changed_dom) {
			/* And it wasn't us */
			எழுத்து *மதிப்பு_s = strndup(
				dom_string_data(மதிப்பு),
				dom_string_byte_length(மதிப்பு));
			எழுத்து *dup = வெற்று;
			எனில் (மதிப்பு_s == வெற்று) {
				செல் out;
			}
			dup = strdup(மதிப்பு_s);
			எனில் (dup == வெற்று) {
				விடுவி(மதிப்பு_s);
				செல் out;
			}
			விடுவி(control->மதிப்பு);
			control->மதிப்பு = மதிப்பு_s;
			விடுவி(control->last_synced_மதிப்பு);
			control->last_synced_மதிப்பு = dup;
			எனில் (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != வெற்று) {
				textarea_set_text(control->data.text.ta,
						  மதிப்பு_s);
			}
		}
		control->முனை_மதிப்பு = மதிப்பு;
		மதிப்பு = வெற்று;
	}

out:
	எனில் (மதிப்பு != வெற்று)
		dom_string_unref(மதிப்பு);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
கட்டமைப்பு form *
form_new(வெறுமை *முனை,
	 மாறிலி எழுத்து *action,
	 மாறிலி எழுத்து *target,
	 form_method method,
	 மாறிலி எழுத்து *charset,
	 மாறிலி எழுத்து *doc_charset)
{
	கட்டமைப்பு form *form;

	form = calloc(1, அளவு *form);
	எனில் (!form)
		திரும்பு வெற்று;

	form->action = strdup(action != வெற்று ? action : "");
	எனில் (form->action == வெற்று) {
		விடுவி(form);
		திரும்பு வெற்று;
	}

	form->target = target != வெற்று ? strdup(target) : வெற்று;
	எனில் (target != வெற்று && form->target == வெற்று) {
		விடுவி(form->action);
		விடுவி(form);
		திரும்பு வெற்று;
	}

	form->method = method;

	form->accept_charsets = charset != வெற்று ? strdup(charset) : வெற்று;
	எனில் (charset != வெற்று && form->accept_charsets == வெற்று) {
		விடுவி(form->target);
		விடுவி(form->action);
		விடுவி(form);
		திரும்பு வெற்று;
	}

	form->document_charset = doc_charset != வெற்று ? strdup(doc_charset)
						     : வெற்று;
	எனில் (doc_charset && form->document_charset == வெற்று) {
		விடுவி(form->accept_charsets);
		விடுவி(form->target);
		விடுவி(form->action);
		விடுவி(form);
		திரும்பு வெற்று;
	}

	form->முனை = முனை;

	திரும்பு form;
}


/* exported interface documented in html/form_internal.h */
வெறுமை form_free(கட்டமைப்பு form *form)
{
	கட்டமைப்பு form_control *c, *d;

	க்கு (c = form->controls; c != வெற்று; c = d) {
		d = c->next;

		form_free_control(c);
	}

	விடுவி(form->action);
	விடுவி(form->target);
	விடுவி(form->accept_charsets);
	விடுவி(form->document_charset);

	விடுவி(form);
}


/* exported interface documented in html/form_internal.h */
கட்டமைப்பு form_control *form_new_control(வெறுமை *முனை, form_control_type type)
{
	கட்டமைப்பு form_control *control;

	control = calloc(1, அளவு *control);
	எனில் (control == வெற்று)
		திரும்பு வெற்று;

	control->முனை = முனை;
	control->type = type;

	திரும்பு control;
}
