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

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dom/dom.h>

#include "utils/corestrings.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/talloc.h"
#include "utils/url.h"
#include "utils/utf8.h"
#include "utils/ascii.h"
#include "netsurf/browser_window.h"
#include "netsurf/inttypes.h"
#include "netsurf/mouse.h"
#include "netsurf/plotters.h"
#include "netsurf/misc.h"
#include "content/fetch.h"
#include "content/hlcache.h"
#include "css/utils.h"
#include "desktop/knockout.h"
#include "desktop/scrollbar.h"
#include "desktop/textarea.h"

#include "html/html.h"
#include "html/private.h"
#include "html/layout.h"
#include "html/box.h"
#include "html/box_inspect.h"
#include "html/font.h"
#include "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_LINE_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

sa_nracanaa form_select_menu {
	puur_naa_nka line_height;
	puur_naa_nka width, height;
	sa_nracanaa scrollbar *scrollbar;
	puur_naa_nka f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	void *client_data;
	sa_nracanaa content *c;
};

static plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

static plot_font_style_t plot_fstyle_entry = {
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
static var_na *
form_encode_item(const var_na *item,
		 uint32_t len,
		 const var_na *charset,
		 const var_na *fallback)
{
	nserror err;
	var_na *ret = NULL;
	var_na cset[256];

	ᮚᮦᮔ᮪ (!item || !charset)
		ᮝᮜᮤ NULL;

	snprintf(cset, parimaa_na cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	ᮚᮦᮔ᮪ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, parimaa_na cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		ᮚᮦᮔ᮪ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ᮚᮦᮔ᮪ (fallback) {
				snprintf(cset, parimaa_na cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				ᮚᮦᮔ᮪ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, parimaa_na cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			ᮚᮦᮔ᮪ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ᮚᮦᮔ᮪ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ᮚᮦᮔ᮪ (err == NSERROR_NOMEM) {
		ᮝᮜᮤ NULL;
	}

	ᮝᮜᮤ ret;
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
static nserror
fetch_data_list_add_sname(const var_na *name,
			  const var_na *ksfx,
			  puur_naa_nka value,
			  sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	sa_nracanaa fetch_multipart_data *fetch_data;
	puur_naa_nka keysize;

	fetch_data = calloc(1, parimaa_na(*fetch_data));
	ᮚᮦᮔ᮪ (fetch_data == NULL) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	ᮚᮦᮔ᮪ (fetch_data->name == NULL) {
		free(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ᮝᮜᮤ NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/* value */
	fetch_data->value = malloc(FETCH_DATA_INT_VALUE_SIZE);
	ᮚᮦᮔ᮪ (fetch_data->value == NULL) {
		free(fetch_data->name);
		free(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		ᮝᮜᮤ NSERROR_NOMEM;
	}
	snprintf(fetch_data->value, FETCH_DATA_INT_VALUE_SIZE, "%d", value);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ᮝᮜᮤ NSERROR_OK;
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
static nserror
fetch_data_list_add(dom_string *name,
		    dom_string *value,
		    const var_na *rawfile,
		    const var_na *form_charset,
		    const var_na *docu_charset,
		    sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	sa_nracanaa fetch_multipart_data *fetch_data;

	assert(name != NULL);

	fetch_data = calloc(1, parimaa_na(*fetch_data));
	ᮚᮦᮔ᮪ (fetch_data == NULL) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	ᮚᮦᮔ᮪ (fetch_data->name == NULL) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		free(fetch_data);
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	ᮚᮦᮔ᮪ (value == NULL) {
		fetch_data->value = strdup("");
	} ᮜᮤᮚ {
		fetch_data->value = form_encode_item(dom_string_data(value),
						     dom_string_byte_length(value),
						     form_charset,
						     docu_charset);
	}
	ᮚᮦᮔ᮪ (fetch_data->value == NULL) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		free(fetch_data->name);
		free(fetch_data);
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ᮚᮦᮔ᮪ (rawfile != NULL) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		ᮚᮦᮔ᮪ (fetch_data->rawfile == NULL) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			free(fetch_data->value);
			free(fetch_data->name);
			free(fetch_data);
			ᮝᮜᮤ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ᮝᮜᮤ NSERROR_OK;
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
static nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  const var_na *form_charset,
			  const var_na *doc_charset,
			  sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (inputname == NULL) {
		/* allow enumeration to continue after element with no name */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_value(text_area_element,
						   &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  NULL,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      const var_na *form_charset,
			      const var_na *docu_charset,
			      sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *value;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (!selected) {
		/* unselected options do not add fetch data entries */
		ᮝᮜᮤ NSERROR_OK;
	}

	exp = dom_html_option_element_get_value(option_element, &value);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  value,
				  NULL,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(value);

	ᮝᮜᮤ res;
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
static nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			const var_na *form_charset,
			const var_na *doc_charset,
			sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = NULL;
	uint32_t options_count;
	uint32_t option_index;
	dom_node *option_element = NULL;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (inputname == NULL) {
		/* allow enumeration to continue after element with no name */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_count);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* iterate over options collection */
	for (option_index = 0; option_index < options_count; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} ᮜᮤᮚ {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_node_unref(option_element);
		}

		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			bha_nga;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_input_submit(dom_html_input_element *input_element,
			      dom_string *inputname,
			      const var_na *charset,
			      const var_na *document_charset,
			      dom_html_element **submit_button,
			      sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ᮚᮦᮔ᮪ (*submit_button == NULL) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)input_element;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (*submit_button != (dom_html_element *)input_element) {
		ᮝᮜᮤ NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  NULL,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_input_image(dom_html_input_element *input_element,
			     dom_string *inputname,
			     const var_na *charset,
			     const var_na *document_charset,
			     dom_html_element **submit_button,
			     sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	sa_nracanaa image_input_coords *coords;
	var_na *basename;

	/* Only use an image input if it was the thing which activated us */
	ᮚᮦᮔ᮪ (*submit_button != (dom_html_element *)input_element) {
		ᮝᮜᮤ NSERROR_OK;
	}

	exp = dom_node_get_user_data((dom_node *)input_element,
				     corestring_dom___ns_key_image_coords_node_data,
				     &coords);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (coords == NULL) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	ᮚᮦᮔ᮪ (basename == NULL) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	ᮚᮦᮔ᮪ (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	free(basename);

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_input_checkbox(dom_html_input_element *input_element,
				dom_string *inputname,
				const var_na *charset,
				const var_na *document_charset,
				sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_input_element_get_checked(input_element, &checked);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (!checked) {
		/* unchecked items do not generate a data entry */
		ᮝᮜᮤ NSERROR_OK;
	}

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* ensure a default value */
	ᮚᮦᮔ᮪ (inputvalue == NULL) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  NULL,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_input_file(dom_html_input_element *input_element,
			    dom_string *inputname,
			    const var_na *charset,
			    const var_na *document_charset,
			    sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	const var_na *rawfile = NULL;

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ᮝᮜᮤ NSERROR_DOM;
	}

	exp = dom_node_get_user_data((dom_node *)input_element,
				     corestring_dom___ns_key_file_name_node_data,
				     &rawfile);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (rawfile == NULL) {
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

	ᮝᮜᮤ res;
}


static nserror
form_dom_to_data_input_text(dom_html_input_element *input_element,
			    dom_string *inputname,
			    const var_na *charset,
			    const var_na *document_charset,
			    sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_input_element_get_value(input_element, &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  NULL,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ᮝᮜᮤ res;
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
static nserror
form_dom_to_data_input(dom_html_input_element *input_element,
		       const var_na *charset,
		       const var_na *document_charset,
		       dom_html_element **submit_button,
		       sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_input_element_get_disabled(input_element,
						  &element_disabled);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (element_disabled) {
		/* disabled element requires no more processing */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_input_element_get_name(input_element, &inputname);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (inputname == NULL) {
		/* element with no name is not converted */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_input_element_get_type(input_element, &inputtype);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* process according to input element type */
	ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_input_submit(input_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_input_image(input_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_input_checkbox(input_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_input_file(input_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ᮜᮤᮚ {
		/* Everything else is treated as text values */
		res = form_dom_to_data_input_text(input_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ᮝᮜᮤ res;
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
static nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			const var_na *form_charset,
			const var_na *doc_charset,
			dom_html_element **submit_button,
			sa_nracanaa fetch_multipart_data ***fetch_data_next_ptr)
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
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ᮝᮜᮤ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ᮝᮜᮤ NSERROR_OK;
	}
	ᮚᮦᮔ᮪ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ᮝᮜᮤ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ᮚᮦᮔ᮪ (*submit_button == NULL) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ᮚᮦᮔ᮪ (*submit_button != (dom_html_element *)button_element) {
		ᮝᮜᮤ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		ᮝᮜᮤ NSERROR_DOM;
	}

	ᮚᮦᮔ᮪ (inputname == NULL) {
		/* allow enumeration to continue after element with no name */
		ᮝᮜᮤ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_value(button_element, &inputvalue);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ᮜᮤᮚ {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  NULL,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ᮝᮜᮤ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
static var_na *form_acceptable_charset(sa_nracanaa form *form)
{
	var_na *temp, *c;

	ᮚᮦᮔ᮪ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ᮚᮦᮔ᮪ (form->document_charset) {
			/* document charset present, so use it */
			ᮝᮜᮤ strdup(form->document_charset);
		} ᮜᮤᮚ {
			/* no document charset, so default to 8859-1 */
			ᮝᮜᮤ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	ᮚᮦᮔ᮪ (!temp)
		ᮝᮜᮤ NULL;

	/* make it upper case */
	for (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	ᮚᮦᮔ᮪ (c) {
		free(temp);
		ᮝᮜᮤ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	free(temp);

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
	ᮞᮨᮓᮨᮜ (*c && !ascii_is_space(*c)) {
		ᮚᮦᮔ᮪ (*c == ',')
			bha_nga;
		c++;
	}

	ᮝᮜᮤ strndup(form->accept_charsets, c - form->accept_charsets);
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
static nserror
form_dom_to_data(sa_nracanaa form *form,
		 sa_nracanaa form_control *submit_control,
		 sa_nracanaa fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	var_na *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = NULL; /* the dom form elements */
	uint32_t element_count; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_node *element = NULL; /* the DOM form element */
	dom_string *nodename = NULL; /* the DOM node name of the element */
	sa_nracanaa fetch_multipart_data *fetch_data = NULL; /* fetch data list */
	sa_nracanaa fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ᮚᮦᮔ᮪ (submit_control != NULL) {
		submit_button = submit_control->node;
	} ᮜᮤᮚ {
		submit_button = NULL;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ᮚᮦᮔ᮪ (charset == NULL) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->node, &elements);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		free(charset);
		ᮝᮜᮤ NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_count);
	ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		goto form_dom_to_data_error;
	}

	for (element_idx = 0; element_idx < element_count; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			goto form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_node_get_node_name(element, &nodename);
		ᮚᮦᮔ᮪ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_node_unref(element);
			res = NSERROR_DOM;
			goto form_dom_to_data_error;
		}

		ᮚᮦᮔ᮪ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_isequal(nodename, corestring_dom_INPUT)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_input(
				(dom_html_input_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ᮜᮤᮚ ᮚᮦᮔ᮪ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ᮜᮤᮚ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (puur_naa_nka)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_node_unref(element);

		/* abort form element enumeration on error */
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			goto form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	free(charset);

	ᮝᮜᮤ NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	free(charset);

	ᮝᮜᮤ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
static nserror
form_url_encode(sa_nracanaa form *form,
		sa_nracanaa fetch_multipart_data *control,
		var_na **encoded_out)
{
	var_na *name, *value;
	var_na *s, *s2;
	unsigned puur_naa_nka len, len1, len_init;
	nserror res;

	s = malloc(1);

	ᮚᮦᮔ᮪ (s == NULL) {
		ᮝᮜᮤ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	for (; control; control = control->next) {
		res = url_escape(control->name, true, NULL, &name);
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			free(s);
			ᮝᮜᮤ res;
		}

		res = url_escape(control->value, true, NULL, &value);
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			free(name);
			free(s);
			ᮝᮜᮤ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(value) + 2;
		s2 = realloc(s, len1 + 1);
		ᮚᮦᮔ᮪ (s2 == NULL) {
			free(value);
			free(name);
			free(s);
			ᮝᮜᮤ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, value);
		len = len1;
		free(name);
		free(value);
	}

	ᮚᮦᮔ᮪ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	ᮝᮜᮤ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
static void
form_select_menu_scroll_callback(void *client_data,
				 sa_nracanaa scrollbar_msg_data *scrollbar_data)
{
	sa_nracanaa form_control *control = client_data;
	sa_nracanaa form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	switch (scrollbar_data->msg) {
		pak_sa SCROLLBAR_MSG_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			bha_nga;
		pak_sa SCROLLBAR_MSG_SCROLL_START:
		{
			sa_nracanaa rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			bha_nga;
		pak_sa SCROLLBAR_MSG_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, NULL);
			bha_nga;
		default:
			bha_nga;
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
static nserror
form__select_process_selection(html_content *html,
			       sa_nracanaa form_control *control,
			       puur_naa_nka item)
{
	sa_nracanaa box *inline_box;
	sa_nracanaa form_option *o;
	puur_naa_nka count;
	nserror ret = NSERROR_OK;

	assert(control != NULL);
	assert(html != NULL);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	for (count = 0, o = control->data.select.items;
			o != NULL;
			count++, o = o->next) {
		ᮚᮦᮔ᮪ (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->node, false);
		}

		ᮚᮦᮔ᮪ (count == item) {
			ᮚᮦᮔ᮪ (control->data.select.multiple) {
				ᮚᮦᮔ᮪ (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->node, false);
					control->data.select.num_selected--;
				} ᮜᮤᮚ {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->node, true);
					control->data.select.num_selected++;
				}
			} ᮜᮤᮚ {
				dom_html_option_element_set_selected(
						o->node, true);
				o->selected = true;
			}
		}

		ᮚᮦᮔ᮪ (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	ᮚᮦᮔ᮪ (control->data.select.num_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (control->data.select.num_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} ᮜᮤᮚ {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ᮚᮦᮔ᮪ (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} ᮜᮤᮚ {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	ᮝᮜᮤ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
static void form_select_menu_clicked(sa_nracanaa form_control *control, puur_naa_nka x, puur_naa_nka y)
{
	sa_nracanaa form_select_menu *menu = control->data.select.menu;
	sa_nracanaa form_option *option;
	html_content *html = (html_content *)menu->c;
	puur_naa_nka line_height, line_height_with_spacing;
	puur_naa_nka item_bottom_y;
	puur_naa_nka scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	line_height = menu->line_height;
	line_height_with_spacing = line_height +
			line_height * SELECT_LINE_SPACING;

	option = control->data.select.items;
	item_bottom_y = line_height_with_spacing;
	i = 0;
	ᮞᮨᮓᮨᮜ (option && item_bottom_y < scroll + y) {
		item_bottom_y += line_height_with_spacing;
		option = option->next;
		i++;
	}

	ᮚᮦᮔ᮪ (option != NULL) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
void form_add_control(sa_nracanaa form *form, sa_nracanaa form_control *control)
{
	ᮚᮦᮔ᮪ (form == NULL) {
		ᮝᮜᮤ;
	}

	control->form = form;

	ᮚᮦᮔ᮪ (form->controls != NULL) {
		assert(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = NULL;
		form->last_control = control;
	} ᮜᮤᮚ {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
void form_free_control(sa_nracanaa form_control *control)
{
	sa_nracanaa form_control *c;
	assert(control != NULL);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->value, control->initial_value);
	free(control->name);
	free(control->value);
	free(control->initial_value);
	ᮚᮦᮔ᮪ (control->last_synced_value != NULL) {
		free(control->last_synced_value);
	}

	ᮚᮦᮔ᮪ (control->type == GADGET_SELECT) {
		sa_nracanaa form_option *option, *next;

		for (option = control->data.select.items; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->value);
			free(option->text);
			free(option->value);
			free(option);
		}
		ᮚᮦᮔ᮪ (control->data.select.menu != NULL) {
			form_free_select_menu(control);
		}
	}

	ᮚᮦᮔ᮪ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ᮚᮦᮔ᮪ (control->data.text.initial != NULL) {
			dom_string_unref(control->data.text.initial);
		}

		ᮚᮦᮔ᮪ (control->data.text.ta != NULL) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	ᮚᮦᮔ᮪ (control->form != NULL) {
		for (c = control->form->controls; c != NULL; c = c->next) {
			ᮚᮦᮔ᮪ (c->next == control) {
				c->next = control->next;
				ᮚᮦᮔ᮪ (control->form->last_control == control)
					control->form->last_control = c;
				bha_nga;
			}
			ᮚᮦᮔ᮪ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				ᮚᮦᮔ᮪ (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = NULL;
				bha_nga;
			}
		}
	}

	ᮚᮦᮔ᮪ (control->node_value != NULL) {
		dom_string_unref(control->node_value);
	}

	free(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(sa_nracanaa form_control *control, var_na *value, var_na *text,
		     bool selected, void *node)
{
	sa_nracanaa form_option *option;

	assert(control);
	assert(control->type == GADGET_SELECT);

	option = calloc(1, parimaa_na *option);
	ᮚᮦᮔ᮪ (!option)
		ᮝᮜᮤ false;

	option->value = value;
	option->text = text;

	/* add to linked list */
	ᮚᮦᮔ᮪ (control->data.select.items == 0)
		control->data.select.items = option;
	ᮜᮤᮚ
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	ᮚᮦᮔ᮪ (selected && (control->data.select.num_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.num_selected++;
		control->data.select.current = option;
	}

	control->data.select.num_items++;

	option->node = node;

	ᮝᮜᮤ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(void *client_data,
		      sa_nracanaa form_control *control,
		      select_menu_redraw_callback callback,
		      sa_nracanaa content *c)
{
	puur_naa_nka line_height_with_spacing;
	sa_nracanaa box *box;
	plot_font_style_t fstyle;
	puur_naa_nka total_height;
	sa_nracanaa form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ᮚᮦᮔ᮪ (control->data.select.menu == NULL) {

		menu = calloc(1, parimaa_na (sa_nracanaa form_select_menu));
		ᮚᮦᮔ᮪ (menu == NULL) {
			ᮝᮜᮤ NSERROR_NOMEM;
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

		ᮚᮦᮔ᮪ (menu->height > MAX_SELECT_HEIGHT) {
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
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			control->data.select.menu = NULL;
			free(menu);
			ᮝᮜᮤ res;
		}
		menu->c = c;
	} ᮜᮤᮚ {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	ᮝᮜᮤ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
void form_free_select_menu(sa_nracanaa form_control *control)
{
	ᮚᮦᮔ᮪ (control->data.select.menu->scrollbar != NULL)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	free(control->data.select.menu);
	control->data.select.menu = NULL;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(sa_nracanaa form_control *control,
			puur_naa_nka x, puur_naa_nka y,
			float scale,
			const sa_nracanaa rect *clip,
			const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa box *box;
	sa_nracanaa form_select_menu *menu = control->data.select.menu;
	sa_nracanaa form_option *option;
	puur_naa_nka line_height, line_height_with_spacing;
	puur_naa_nka width, height;
	puur_naa_nka x0, y0, x1, scrollbar_x, y1, y2, y3;
	puur_naa_nka item_y;
	puur_naa_nka text_pos_offset, text_x;
	puur_naa_nka scrollbar_width = SCROLLBAR_WIDTH;
	puur_naa_nka i;
	puur_naa_nka scroll;
	puur_naa_nka x_cp, y_cp;
	sa_nracanaa rect r;
	sa_nracanaa rect rect;
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

	ᮚᮦᮔ᮪ (scale != 1.0) {
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
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ false;
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
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ false;
	}

	option = control->data.select.items;
	item_y = line_height_with_spacing;

	ᮞᮨᮓᮨᮜ (item_y < scroll) {
		option = option->next;
		item_y += line_height_with_spacing;
	}
	item_y -= line_height_with_spacing;
	text_pos_offset = y - scroll +
			(puur_naa_nka) (line_height * (0.75 + SELECT_LINE_SPACING));
	text_x = x + (box->border[LEFT].width + box->padding[LEFT]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	ᮞᮨᮓᮨᮜ (option && item_y - scroll < height) {

		ᮚᮦᮔ᮪ (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + line_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ᮚᮦᮔ᮪ (res != NSERROR_OK) {
				ᮝᮜᮤ false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		ᮚᮦᮔ᮪ (res != NSERROR_OK) {
			ᮝᮜᮤ false;
		}

		item_y += line_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ false;
	}

	ᮝᮜᮤ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(sa_nracanaa form_control *control,
			     float scale,
			     const sa_nracanaa rect *clip)
{
	sa_nracanaa form_select_menu *menu = control->data.select.menu;
	puur_naa_nka width, height;


	width = menu->width;
	height = menu->height;

	ᮚᮦᮔ᮪ (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	ᮚᮦᮔ᮪ (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		ᮝᮜᮤ true;

	ᮝᮜᮤ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(sa_nracanaa form_control *control, puur_naa_nka item)
{
	assert(control != NULL);

	ᮝᮜᮤ form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
sa_nracanaa form_option *
form_select_get_option(sa_nracanaa form_control *control, puur_naa_nka item)
{
	sa_nracanaa form_option *opt;

	opt = control->data.select.items;
	ᮞᮨᮓᮨᮜ ((opt != NULL) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	ᮝᮜᮤ opt;
}


/* exported interface documented in netsurf/form.h */
var_na *form_control_get_name(sa_nracanaa form_control *control)
{
	ᮝᮜᮤ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(sa_nracanaa form_control *control, sa_nracanaa rect *r)
{
	box_bounds( control->box, r );
	ᮝᮜᮤ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
const var_na *
form_select_mouse_action(sa_nracanaa form_control *control,
			 browser_mouse_state mouse,
			 puur_naa_nka x, puur_naa_nka y)
{
	sa_nracanaa form_select_menu *menu = control->data.select.menu;
	puur_naa_nka x0, y0, x1, y1, scrollbar_x;
	const var_na *status = NULL;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	ᮚᮦᮔ᮪ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ᮝᮜᮤ scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ᮚᮦᮔ᮪ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ᮚᮦᮔ᮪ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ᮚᮦᮔ᮪ (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	ᮝᮜᮤ status;
}


/* private interface described in html/form_internal.h */
void
form_select_mouse_drag_end(sa_nracanaa form_control *control,
			   browser_mouse_state mouse,
			   puur_naa_nka x, puur_naa_nka y)
{
	puur_naa_nka x0, y0, x1, y1;
	puur_naa_nka box_x, box_y;
	sa_nracanaa box *box;
	sa_nracanaa form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[LEFT].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[TOP];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ᮚᮦᮔ᮪ (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		ᮝᮜᮤ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	ᮚᮦᮔ᮪ (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
void form_select_get_dimensions(sa_nracanaa form_control *control,
		puur_naa_nka *width, puur_naa_nka *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
void form_select_menu_callback(void *client_data,
		puur_naa_nka x, puur_naa_nka y, puur_naa_nka width, puur_naa_nka height)
{
	html_content *html = client_data;
	puur_naa_nka menu_x, menu_y;
	sa_nracanaa box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[LEFT].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[TOP];
	content__request_redraw((sa_nracanaa content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
void form_radio_set(sa_nracanaa form_control *radio)
{
	sa_nracanaa form_control *control;

	assert(radio);
	ᮚᮦᮔ᮪ (!radio->form)
		ᮝᮜᮤ;

	ᮚᮦᮔ᮪ (radio->selected)
		ᮝᮜᮤ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	for (control = radio->form->controls;
	     control != NULL;
	     control = control->next) {
		/* Only interested in radio inputs */
		ᮚᮦᮔ᮪ (control->type != GADGET_RADIO)
			continue;

		/* Ignore ourself */
		ᮚᮦᮔ᮪ (control == radio)
			continue;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ᮚᮦᮔ᮪ ((control->name == NULL) ||
		    (radio->name == NULL) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			continue;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ᮚᮦᮔ᮪ (control->selected) {
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
	    sa_nracanaa browser_window *target,
	    sa_nracanaa form *form,
	    sa_nracanaa form_control *submit_button)
{
	nserror res;
	var_na *data = NULL; /* encoded form data */
	sa_nracanaa fetch_multipart_data *success = NULL; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	assert(form != NULL);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		ᮝᮜᮤ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ᮚᮦᮔ᮪ (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		ᮝᮜᮤ res;
	}

	switch (form->method) {
	pak_sa method_GET:
		res = form_url_encode(form, success, &data);
		ᮚᮦᮔ᮪ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			ᮚᮦᮔ᮪ (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      NULL,
							      NULL,
							      NULL);

				nsurl_unref(query_url);
			}
			free(data);
		}
		bha_nga;

	pak_sa method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		ᮚᮦᮔ᮪ (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      NULL,
						      NULL);
			free(data);
		}
		bha_nga;

	pak_sa method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      NULL,
					      success,
					      NULL);

		bha_nga;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	ᮝᮜᮤ res;
}


/* exported interface documented in html/form_internal.h */
void form_gadget_update_value(sa_nracanaa form_control *control, var_na *value)
{
	switch (control->type) {
	pak_sa GADGET_HIDDEN:
	pak_sa GADGET_TEXTBOX:
	pak_sa GADGET_TEXTAREA:
	pak_sa GADGET_PASSWORD:
	pak_sa GADGET_FILE:
		ᮚᮦᮔ᮪ (control->value != NULL) {
			free(control->value);
		}
		control->value = value;
		ᮚᮦᮔ᮪ (control->node != NULL) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)value,
						strlen(value), &str);
			ᮚᮦᮔ᮪ (err == DOM_NO_ERR) {
				ᮚᮦᮔ᮪ (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_value(
						(dom_html_text_area_element *)(control->node),
						str);
				ᮜᮤᮚ
					err = dom_html_input_element_set_value(
						(dom_html_input_element *)(control->node),
						str);
				dom_string_unref(str);
			}
		}
		bha_nga;
	default:
		/* Do nothing */
		bha_nga;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
void
form_gadget_sync_with_dom(sa_nracanaa form_control *control)
{
	dom_exception exc;
	dom_string *value = NULL;
	bool changed_dom = false;

	ᮚᮦᮔ᮪ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ᮝᮜᮤ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ᮚᮦᮔ᮪ ((control->last_synced_value == NULL &&
	     control->value != NULL &&
	     control->value[0] != '\0') ||
	    (control->last_synced_value != NULL &&
	     control->value != NULL &&
	     strcmp(control->value, control->last_synced_value) != 0)) {
		var_na *dup = strdup(control->value);
		ᮚᮦᮔ᮪ (dup == NULL) {
			goto out;
		}
		ᮚᮦᮔ᮪ (control->last_synced_value != NULL) {
			free(control->last_synced_value);
		}
		control->last_synced_value = dup;
		exc = dom_string_create((uint8_t *)(control->value),
					strlen(control->value), &value);
		ᮚᮦᮔ᮪ (exc != DOM_NO_ERR) {
			goto out;
		}
		ᮚᮦᮔ᮪ (control->node_value != NULL) {
			dom_string_unref(control->node_value);
		}
		control->node_value = value;
		value = NULL;
		ᮚᮦᮔ᮪ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_value(control->node, control->node_value);
		} ᮜᮤᮚ {
			exc = dom_html_input_element_set_value(control->node, control->node_value);
		}
		ᮚᮦᮔ᮪ (exc != DOM_NO_ERR) {
			goto out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ᮚᮦᮔ᮪ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_value(control->node, &value);
	} ᮜᮤᮚ {
		exc = dom_html_input_element_get_value(control->node, &value);
	}

	ᮚᮦᮔ᮪ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		goto out;
	}

	ᮚᮦᮔ᮪ (!dom_string_isequal(control->node_value, value)) {
		/* The DOM has changed */
		ᮚᮦᮔ᮪ (!changed_dom) {
			/* And it wasn't us */
			var_na *value_s = strndup(
				dom_string_data(value),
				dom_string_byte_length(value));
			var_na *dup = NULL;
			ᮚᮦᮔ᮪ (value_s == NULL) {
				goto out;
			}
			dup = strdup(value_s);
			ᮚᮦᮔ᮪ (dup == NULL) {
				free(value_s);
				goto out;
			}
			free(control->value);
			control->value = value_s;
			free(control->last_synced_value);
			control->last_synced_value = dup;
			ᮚᮦᮔ᮪ (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != NULL) {
				textarea_set_text(control->data.text.ta,
						  value_s);
			}
		}
		control->node_value = value;
		value = NULL;
	}

out:
	ᮚᮦᮔ᮪ (value != NULL)
		dom_string_unref(value);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
sa_nracanaa form *
form_new(void *node,
	 const var_na *action,
	 const var_na *target,
	 form_method method,
	 const var_na *charset,
	 const var_na *doc_charset)
{
	sa_nracanaa form *form;

	form = calloc(1, parimaa_na *form);
	ᮚᮦᮔ᮪ (!form)
		ᮝᮜᮤ NULL;

	form->action = strdup(action != NULL ? action : "");
	ᮚᮦᮔ᮪ (form->action == NULL) {
		free(form);
		ᮝᮜᮤ NULL;
	}

	form->target = target != NULL ? strdup(target) : NULL;
	ᮚᮦᮔ᮪ (target != NULL && form->target == NULL) {
		free(form->action);
		free(form);
		ᮝᮜᮤ NULL;
	}

	form->method = method;

	form->accept_charsets = charset != NULL ? strdup(charset) : NULL;
	ᮚᮦᮔ᮪ (charset != NULL && form->accept_charsets == NULL) {
		free(form->target);
		free(form->action);
		free(form);
		ᮝᮜᮤ NULL;
	}

	form->document_charset = doc_charset != NULL ? strdup(doc_charset)
						     : NULL;
	ᮚᮦᮔ᮪ (doc_charset && form->document_charset == NULL) {
		free(form->accept_charsets);
		free(form->target);
		free(form->action);
		free(form);
		ᮝᮜᮤ NULL;
	}

	form->node = node;

	ᮝᮜᮤ form;
}


/* exported interface documented in html/form_internal.h */
void form_free(sa_nracanaa form *form)
{
	sa_nracanaa form_control *c, *d;

	for (c = form->controls; c != NULL; c = d) {
		d = c->next;

		form_free_control(c);
	}

	free(form->action);
	free(form->target);
	free(form->accept_charsets);
	free(form->document_charset);

	free(form);
}


/* exported interface documented in html/form_internal.h */
sa_nracanaa form_control *form_new_control(void *node, form_control_type type)
{
	sa_nracanaa form_control *control;

	control = calloc(1, parimaa_na *control);
	ᮚᮦᮔ᮪ (control == NULL)
		ᮝᮜᮤ NULL;

	control->node = node;
	control->type = type;

	ᮝᮜᮤ control;
}
