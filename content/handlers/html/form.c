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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/url.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/knockout.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_ಸಾಲು_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

ಸಂರಚನಾ form_select_menu {
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_height;
	ಪೂರ್ಣಾಙ್ಕ width, height;
	ಸಂರಚನಾ scrollbar *scrollbar;
	ಪೂರ್ಣಾಙ್ಕ f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	ಶೂನ್ಯ *client_data;
	ಸಂರಚನಾ content *c;
};

ಸ್ಥಿರ plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

ಸ್ಥಿರ plot_font_style_t plot_fstyle_entry = {
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
ಸ್ಥಿರ ವರ್ಣ *
form_encode_item(ಧ್ರುವ ವರ್ಣ *item,
		 uint32_t len,
		 ಧ್ರುವ ವರ್ಣ *charset,
		 ಧ್ರುವ ವರ್ಣ *fallback)
{
	nserror err;
	ವರ್ಣ *ret = ರಿಕ್ತ;
	ವರ್ಣ cset[256];

	ಯದಿ (!item || !charset)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	snprintf(cset, ಪರಿಮಾಣ cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	ಯದಿ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, ಪರಿಮಾಣ cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		ಯದಿ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ಯದಿ (fallback) {
				snprintf(cset, ಪರಿಮಾಣ cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				ಯದಿ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, ಪರಿಮಾಣ cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			ಯದಿ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ಯದಿ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ಯದಿ (err == NSERROR_NOMEM) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_ಮೌಲ್ಯ_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ಸ್ಥಿರ nserror
fetch_data_list_add_sname(ಧ್ರುವ ವರ್ಣ *name,
			  ಧ್ರುವ ವರ್ಣ *ksfx,
			  ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ,
			  ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	ಸಂರಚನಾ fetch_multipart_data *fetch_data;
	ಪೂರ್ಣಾಙ್ಕ keysize;

	fetch_data = calloc(1, ಪರಿಮಾಣ(*fetch_data));
	ಯದಿ (fetch_data == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	ಯದಿ (fetch_data->name == ರಿಕ್ತ) {
		ಮುಕ್ತ(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*ಮೌಲ್ಯ*/
	fetch_data->ಮೌಲ್ಯ = malloc(FETCH_DATA_INT_ಮೌಲ್ಯ_SIZE);
	ಯದಿ (fetch_data->ಮೌಲ್ಯ == ರಿಕ್ತ) {
		ಮುಕ್ತ(fetch_data->name);
		ಮುಕ್ತ(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}
	snprintf(fetch_data->ಮೌಲ್ಯ, FETCH_DATA_INT_ಮೌಲ್ಯ_SIZE, "%d", ಮೌಲ್ಯ);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ nserror
fetch_data_list_add(dom_string *name,
		    dom_string *ಮೌಲ್ಯ,
		    ಧ್ರುವ ವರ್ಣ *rawfile,
		    ಧ್ರುವ ವರ್ಣ *form_charset,
		    ಧ್ರುವ ವರ್ಣ *docu_charset,
		    ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	ಸಂರಚನಾ fetch_multipart_data *fetch_data;

	ಪ್ರತಿಜ್ಞಾ(name != ರಿಕ್ತ);

	fetch_data = calloc(1, ಪರಿಮಾಣ(*fetch_data));
	ಯದಿ (fetch_data == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	ಯದಿ (fetch_data->name == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		ಮುಕ್ತ(fetch_data);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಯದಿ (ಮೌಲ್ಯ == ರಿಕ್ತ) {
		fetch_data->ಮೌಲ್ಯ = strdup("");
	} ಅನ್ಯಥಾ {
		fetch_data->ಮೌಲ್ಯ = form_encode_item(dom_string_data(ಮೌಲ್ಯ),
						     dom_string_byte_length(ಮೌಲ್ಯ),
						     form_charset,
						     docu_charset);
	}
	ಯದಿ (fetch_data->ಮೌಲ್ಯ == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		ಮುಕ್ತ(fetch_data->name);
		ಮುಕ್ತ(fetch_data);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ಯದಿ (rawfile != ರಿಕ್ತ) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		ಯದಿ (fetch_data->rawfile == ರಿಕ್ತ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			ಮುಕ್ತ(fetch_data->ಮೌಲ್ಯ);
			ಮುಕ್ತ(fetch_data->name);
			ಮುಕ್ತ(fetch_data);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
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
ಸ್ಥಿರ nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  ಧ್ರುವ ವರ್ಣ *form_charset,
			  ಧ್ರುವ ವರ್ಣ *doc_charset,
			  ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (inputname == ರಿಕ್ತ) {
		/* allow enumeration to continue after element with no name */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_ಮೌಲ್ಯ(text_area_element,
						   &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ರಿಕ್ತ,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ಧ್ರುವ ವರ್ಣ *form_charset,
			      ಧ್ರುವ ವರ್ಣ *docu_charset,
			      ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *ಮೌಲ್ಯ;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (!selected) {
		/* unselected options do not add fetch data entries */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	exp = dom_html_option_element_get_ಮೌಲ್ಯ(option_element, &ಮೌಲ್ಯ);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  ಮೌಲ್ಯ,
				  ರಿಕ್ತ,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(ಮೌಲ್ಯ);

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			ಧ್ರುವ ವರ್ಣ *form_charset,
			ಧ್ರುವ ವರ್ಣ *doc_charset,
			ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ರಿಕ್ತ;
	uint32_t options_ಎಣಿಕೆ;
	uint32_t option_index;
	dom_ಗಂಟು *option_element = ರಿಕ್ತ;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (inputname == ರಿಕ್ತ) {
		/* allow enumeration to continue after element with no name */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_ಎಣಿಕೆ);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* iterate over options collection */
	ಪ್ರತಿ (option_index = 0; option_index < options_ಎಣಿಕೆ; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		ಯದಿ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} ಅನ್ಯಥಾ {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_ಗಂಟು_unref(option_element);
		}

		ಯದಿ (res != NSERROR_OK) {
			ಭಙ್ಗ;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು_submit(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			      dom_string *inputname,
			      ಧ್ರುವ ವರ್ಣ *charset,
			      ಧ್ರುವ ವರ್ಣ *document_charset,
			      dom_html_element **submit_button,
			      ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ಯದಿ (*submit_button == ರಿಕ್ತ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ಒಳಹರಿವು_element;
	} ಅನ್ಯಥಾ ಯದಿ (*submit_button != (dom_html_element *)ಒಳಹರಿವು_element) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ರಿಕ್ತ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು_image(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			     dom_string *inputname,
			     ಧ್ರುವ ವರ್ಣ *charset,
			     ಧ್ರುವ ವರ್ಣ *document_charset,
			     dom_html_element **submit_button,
			     ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	ಸಂರಚನಾ image_ಒಳಹರಿವು_coords *coords;
	ವರ್ಣ *basename;

	/* Only use an image input if it was the thing which activated us */
	ಯದಿ (*submit_button != (dom_html_element *)ಒಳಹರಿವು_element) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	exp = dom_ಗಂಟು_get_user_data((dom_ಗಂಟು *)ಒಳಹರಿವು_element,
				     corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_data,
				     &coords);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (coords == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	ಯದಿ (basename == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	ಯದಿ (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	ಮುಕ್ತ(basename);

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು_checkbox(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
				dom_string *inputname,
				ಧ್ರುವ ವರ್ಣ *charset,
				ಧ್ರುವ ವರ್ಣ *document_charset,
				ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_ಒಳಹರಿವು_element_get_checked(ಒಳಹರಿವು_element, &checked);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (!checked) {
		/* unchecked items do not generate a data entry */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	exp = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* ensure a default value */
	ಯದಿ (inputvalue == ರಿಕ್ತ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ರಿಕ್ತ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು_file(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			    dom_string *inputname,
			    ಧ್ರುವ ವರ್ಣ *charset,
			    ಧ್ರುವ ವರ್ಣ *document_charset,
			    ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ಧ್ರುವ ವರ್ಣ *rawfile = ರಿಕ್ತ;

	exp = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	exp = dom_ಗಂಟು_get_user_data((dom_ಗಂಟು *)ಒಳಹರಿವು_element,
				     corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_data,
				     &rawfile);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (rawfile == ರಿಕ್ತ) {
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

	ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು_text(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			    dom_string *inputname,
			    ಧ್ರುವ ವರ್ಣ *charset,
			    ಧ್ರುವ ವರ್ಣ *document_charset,
			    ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ರಿಕ್ತ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ nserror
form_dom_to_data_ಒಳಹರಿವು(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
		       ಧ್ರುವ ವರ್ಣ *charset,
		       ಧ್ರುವ ವರ್ಣ *document_charset,
		       dom_html_element **submit_button,
		       ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_ಒಳಹರಿವು_element_get_disabled(ಒಳಹರಿವು_element,
						  &element_disabled);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (element_disabled) {
		/* disabled element requires no more processing */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_ಒಳಹರಿವು_element_get_name(ಒಳಹರಿವು_element, &inputname);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (inputname == ರಿಕ್ತ) {
		/* element with no name is not converted */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_ಒಳಹರಿವು_element_get_type(ಒಳಹರಿವು_element, &inputtype);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* process according to input element type */
	ಯದಿ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_ಒಳಹರಿವು_submit(ಒಳಹರಿವು_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_ಒಳಹರಿವು_image(ಒಳಹರಿವು_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_ಒಳಹರಿವು_checkbox(ಒಳಹರಿವು_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_ಒಳಹರಿವು_file(ಒಳಹರಿವು_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ಅನ್ಯಥಾ {
		/* Everything else is treated as text values */
		res = form_dom_to_data_ಒಳಹರಿವು_text(ಒಳಹರಿವು_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			ಧ್ರುವ ವರ್ಣ *form_charset,
			ಧ್ರುವ ವರ್ಣ *doc_charset,
			dom_html_element **submit_button,
			ಸಂರಚನಾ fetch_multipart_data ***fetch_data_next_ptr)
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
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ಯದಿ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}
	ಯದಿ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ಯದಿ (*submit_button == ರಿಕ್ತ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ಯದಿ (*submit_button != (dom_html_element *)button_element) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	ಯದಿ (inputname == ರಿಕ್ತ) {
		/* allow enumeration to continue after element with no name */
		ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_ಮೌಲ್ಯ(button_element, &inputvalue);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ಅನ್ಯಥಾ {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  ರಿಕ್ತ,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ಪ್ರತ್ಯಾಗಮ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ಸ್ಥಿರ ವರ್ಣ *form_acceptable_charset(ಸಂರಚನಾ form *form)
{
	ವರ್ಣ *temp, *c;

	ಯದಿ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ಯದಿ (form->document_charset) {
			/* document charset present, so use it */
			ಪ್ರತ್ಯಾಗಮ strdup(form->document_charset);
		} ಅನ್ಯಥಾ {
			/* no document charset, so default to 8859-1 */
			ಪ್ರತ್ಯಾಗಮ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	ಯದಿ (!temp)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* make it upper case */
	ಪ್ರತಿ (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	ಯದಿ (c) {
		ಮುಕ್ತ(temp);
		ಪ್ರತ್ಯಾಗಮ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	ಮುಕ್ತ(temp);

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
	ಯಾವತ್ (*c && !ascii_is_space(*c)) {
		ಯದಿ (*c == ',')
			ಭಙ್ಗ;
		c++;
	}

	ಪ್ರತ್ಯಾಗಮ strndup(form->accept_charsets, c - form->accept_charsets);
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
ಸ್ಥಿರ nserror
form_dom_to_data(ಸಂರಚನಾ form *form,
		 ಸಂರಚನಾ form_control *submit_control,
		 ಸಂರಚನಾ fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	ವರ್ಣ *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = ರಿಕ್ತ; /* the dom form elements */
	uint32_t element_ಎಣಿಕೆ; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ಗಂಟು *element = ರಿಕ್ತ; /* the DOM form element */
	dom_string *nodename = ರಿಕ್ತ; /* the DOM node name of the element */
	ಸಂರಚನಾ fetch_multipart_data *fetch_data = ರಿಕ್ತ; /* fetch data list */
	ಸಂರಚನಾ fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ಯದಿ (submit_control != ರಿಕ್ತ) {
		submit_button = submit_control->ಗಂಟು;
	} ಅನ್ಯಥಾ {
		submit_button = ರಿಕ್ತ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ಯದಿ (charset == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->ಗಂಟು, &elements);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		ಮುಕ್ತ(charset);
		ಪ್ರತ್ಯಾಗಮ NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_ಎಣಿಕೆ);
	ಯದಿ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		ಗಚ್ಛ form_dom_to_data_error;
	}

	ಪ್ರತಿ (element_idx = 0; element_idx < element_ಎಣಿಕೆ; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		ಯದಿ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			ಗಚ್ಛ form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_ಗಂಟು_get_ಗಂಟು_name(element, &nodename);
		ಯದಿ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_ಗಂಟು_unref(element);
			res = NSERROR_DOM;
			ಗಚ್ಛ form_dom_to_data_error;
		}

		ಯದಿ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(nodename, corestring_dom_ಒಳಹರಿವು)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_ಒಳಹರಿವು(
				(dom_html_ಒಳಹರಿವು_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ಅನ್ಯಥಾ ಯದಿ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ಅನ್ಯಥಾ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (ಪೂರ್ಣಾಙ್ಕ)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ಗಂಟು_unref(element);

		/* abort form element enumeration on error */
		ಯದಿ (res != NSERROR_OK) {
			ಗಚ್ಛ form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	ಮುಕ್ತ(charset);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	ಮುಕ್ತ(charset);

	ಪ್ರತ್ಯಾಗಮ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ಸ್ಥಿರ nserror
form_url_encode(ಸಂರಚನಾ form *form,
		ಸಂರಚನಾ fetch_multipart_data *control,
		ವರ್ಣ **encoded_out)
{
	ವರ್ಣ *name, *ಮೌಲ್ಯ;
	ವರ್ಣ *s, *s2;
	unsigned ಪೂರ್ಣಾಙ್ಕ len, len1, len_init;
	nserror res;

	s = malloc(1);

	ಯದಿ (s == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	ಪ್ರತಿ (; control; control = control->next) {
		res = url_escape(control->name, true, ರಿಕ್ತ, &name);
		ಯದಿ (res != NSERROR_OK) {
			ಮುಕ್ತ(s);
			ಪ್ರತ್ಯಾಗಮ res;
		}

		res = url_escape(control->ಮೌಲ್ಯ, true, ರಿಕ್ತ, &ಮೌಲ್ಯ);
		ಯದಿ (res != NSERROR_OK) {
			ಮುಕ್ತ(name);
			ಮುಕ್ತ(s);
			ಪ್ರತ್ಯಾಗಮ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(ಮೌಲ್ಯ) + 2;
		s2 = realloc(s, len1 + 1);
		ಯದಿ (s2 == ರಿಕ್ತ) {
			ಮುಕ್ತ(ಮೌಲ್ಯ);
			ಮುಕ್ತ(name);
			ಮುಕ್ತ(s);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, ಮೌಲ್ಯ);
		len = len1;
		ಮುಕ್ತ(name);
		ಮುಕ್ತ(ಮೌಲ್ಯ);
	}

	ಯದಿ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ಸ್ಥಿರ ಶೂನ್ಯ
form_select_menu_scroll_callback(ಶೂನ್ಯ *client_data,
				 ಸಂರಚನಾ scrollbar_ಸಂದೇಶ_data *scrollbar_data)
{
	ಸಂರಚನಾ form_control *control = client_data;
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	ವಿಕಲ್ಪ (scrollbar_data->ಸಂದೇಶ) {
		ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			ಭಙ್ಗ;
		ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_SCROLL_START:
		{
			ಸಂರಚನಾ rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			ಭಙ್ಗ;
		ಪಕ್ಷ SCROLLBAR_ಸಂದೇಶ_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, ರಿಕ್ತ);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
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
ಸ್ಥಿರ nserror
form__select_process_selection(html_content *html,
			       ಸಂರಚನಾ form_control *control,
			       ಪೂರ್ಣಾಙ್ಕ item)
{
	ಸಂರಚನಾ box *inline_box;
	ಸಂರಚನಾ form_option *o;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
	nserror ret = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(control != ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(html != ರಿಕ್ತ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	ಪ್ರತಿ (ಎಣಿಕೆ = 0, o = control->data.select.ವಸ್ತುಗಳು;
			o != ರಿಕ್ತ;
			ಎಣಿಕೆ++, o = o->next) {
		ಯದಿ (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->ಗಂಟು, false);
		}

		ಯದಿ (ಎಣಿಕೆ == item) {
			ಯದಿ (control->data.select.multiple) {
				ಯದಿ (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->ಗಂಟು, false);
					control->data.select.ಅಂಕೆ_selected--;
				} ಅನ್ಯಥಾ {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->ಗಂಟು, true);
					control->data.select.ಅಂಕೆ_selected++;
				}
			} ಅನ್ಯಥಾ {
				dom_html_option_element_set_selected(
						o->ಗಂಟು, true);
				o->selected = true;
			}
		}

		ಯದಿ (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	ಯದಿ (control->data.select.ಅಂಕೆ_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ಅನ್ಯಥಾ ಯದಿ (control->data.select.ಅಂಕೆ_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} ಅನ್ಯಥಾ {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ಯದಿ (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} ಅನ್ಯಥಾ {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	ಪ್ರತ್ಯಾಗಮ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ಸ್ಥಿರ ಶೂನ್ಯ form_select_menu_clicked(ಸಂರಚನಾ form_control *control, ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;
	ಸಂರಚನಾ form_option *option;
	html_content *html = (html_content *)menu->c;
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_height, ಸಾಲು_height_with_spacing;
	ಪೂರ್ಣಾಙ್ಕ item_bottom_y;
	ಪೂರ್ಣಾಙ್ಕ scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	ಸಾಲು_height = menu->ಸಾಲು_height;
	ಸಾಲು_height_with_spacing = ಸಾಲು_height +
			ಸಾಲು_height * SELECT_ಸಾಲು_SPACING;

	option = control->data.select.ವಸ್ತುಗಳು;
	item_bottom_y = ಸಾಲು_height_with_spacing;
	i = 0;
	ಯಾವತ್ (option && item_bottom_y < scroll + y) {
		item_bottom_y += ಸಾಲು_height_with_spacing;
		option = option->next;
		i++;
	}

	ಯದಿ (option != ರಿಕ್ತ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_add_control(ಸಂರಚನಾ form *form, ಸಂರಚನಾ form_control *control)
{
	ಯದಿ (form == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	control->form = form;

	ಯದಿ (form->controls != ರಿಕ್ತ) {
		ಪ್ರತಿಜ್ಞಾ(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = ರಿಕ್ತ;
		form->last_control = control;
	} ಅನ್ಯಥಾ {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free_control(ಸಂರಚನಾ form_control *control)
{
	ಸಂರಚನಾ form_control *c;
	ಪ್ರತಿಜ್ಞಾ(control != ರಿಕ್ತ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->ಮೌಲ್ಯ, control->initial_ಮೌಲ್ಯ);
	ಮುಕ್ತ(control->name);
	ಮುಕ್ತ(control->ಮೌಲ್ಯ);
	ಮುಕ್ತ(control->initial_ಮೌಲ್ಯ);
	ಯದಿ (control->last_synced_ಮೌಲ್ಯ != ರಿಕ್ತ) {
		ಮುಕ್ತ(control->last_synced_ಮೌಲ್ಯ);
	}

	ಯದಿ (control->type == GADGET_SELECT) {
		ಸಂರಚನಾ form_option *option, *next;

		ಪ್ರತಿ (option = control->data.select.ವಸ್ತುಗಳು; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->ಮೌಲ್ಯ);
			ಮುಕ್ತ(option->text);
			ಮುಕ್ತ(option->ಮೌಲ್ಯ);
			ಮುಕ್ತ(option);
		}
		ಯದಿ (control->data.select.menu != ರಿಕ್ತ) {
			form_free_select_menu(control);
		}
	}

	ಯದಿ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ಯದಿ (control->data.text.initial != ರಿಕ್ತ) {
			dom_string_unref(control->data.text.initial);
		}

		ಯದಿ (control->data.text.ta != ರಿಕ್ತ) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	ಯದಿ (control->form != ರಿಕ್ತ) {
		ಪ್ರತಿ (c = control->form->controls; c != ರಿಕ್ತ; c = c->next) {
			ಯದಿ (c->next == control) {
				c->next = control->next;
				ಯದಿ (control->form->last_control == control)
					control->form->last_control = c;
				ಭಙ್ಗ;
			}
			ಯದಿ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				ಯದಿ (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = ರಿಕ್ತ;
				ಭಙ್ಗ;
			}
		}
	}

	ಯದಿ (control->ಗಂಟು_ಮೌಲ್ಯ != ರಿಕ್ತ) {
		dom_string_unref(control->ಗಂಟು_ಮೌಲ್ಯ);
	}

	ಮುಕ್ತ(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(ಸಂರಚನಾ form_control *control, ವರ್ಣ *ಮೌಲ್ಯ, ವರ್ಣ *text,
		     bool selected, ಶೂನ್ಯ *ಗಂಟು)
{
	ಸಂರಚನಾ form_option *option;

	ಪ್ರತಿಜ್ಞಾ(control);
	ಪ್ರತಿಜ್ಞಾ(control->type == GADGET_SELECT);

	option = calloc(1, ಪರಿಮಾಣ *option);
	ಯದಿ (!option)
		ಪ್ರತ್ಯಾಗಮ false;

	option->ಮೌಲ್ಯ = ಮೌಲ್ಯ;
	option->text = text;

	/* add to linked list */
	ಯದಿ (control->data.select.ವಸ್ತುಗಳು == 0)
		control->data.select.ವಸ್ತುಗಳು = option;
	ಅನ್ಯಥಾ
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	ಯದಿ (selected && (control->data.select.ಅಂಕೆ_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.ಅಂಕೆ_selected++;
		control->data.select.current = option;
	}

	control->data.select.ಅಂಕೆ_ವಸ್ತುಗಳು++;

	option->ಗಂಟು = ಗಂಟು;

	ಪ್ರತ್ಯಾಗಮ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(ಶೂನ್ಯ *client_data,
		      ಸಂರಚನಾ form_control *control,
		      select_menu_redraw_callback callback,
		      ಸಂರಚನಾ content *c)
{
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_height_with_spacing;
	ಸಂರಚನಾ box *box;
	plot_font_style_t fstyle;
	ಪೂರ್ಣಾಙ್ಕ total_height;
	ಸಂರಚನಾ form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ಯದಿ (control->data.select.menu == ರಿಕ್ತ) {

		menu = calloc(1, ಪರಿಮಾಣ (ಸಂರಚನಾ form_select_menu));
		ಯದಿ (menu == ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[ಬಲ_2].width + box->padding[ಬಲ_2] +
			box->border[ಎಡ_2].width + box->padding[ಎಡ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->ಸಾಲು_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		ಸಾಲು_height_with_spacing = menu->ಸಾಲು_height +
				menu->ಸಾಲು_height *
				SELECT_ಸಾಲು_SPACING;

		total_height = control->data.select.ಅಂಕೆ_ವಸ್ತುಗಳು *
				ಸಾಲು_height_with_spacing;
		menu->height = total_height;

		ಯದಿ (menu->height > MAX_SELECT_HEIGHT) {
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
		ಯದಿ (res != NSERROR_OK) {
			control->data.select.menu = ರಿಕ್ತ;
			ಮುಕ್ತ(menu);
			ಪ್ರತ್ಯಾಗಮ res;
		}
		menu->c = c;
	} ಅನ್ಯಥಾ {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free_select_menu(ಸಂರಚನಾ form_control *control)
{
	ಯದಿ (control->data.select.menu->scrollbar != ರಿಕ್ತ)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	ಮುಕ್ತ(control->data.select.menu);
	control->data.select.menu = ರಿಕ್ತ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ಸಂರಚನಾ form_control *control,
			ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
			ಭಿನ್ನ scale,
			ಧ್ರುವ ಸಂರಚನಾ rect *clip,
			ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ box *box;
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;
	ಸಂರಚನಾ form_option *option;
	ಪೂರ್ಣಾಙ್ಕ ಸಾಲು_height, ಸಾಲು_height_with_spacing;
	ಪೂರ್ಣಾಙ್ಕ width, height;
	ಪೂರ್ಣಾಙ್ಕ x0, y0, x1, scrollbar_x, y1, y2, y3;
	ಪೂರ್ಣಾಙ್ಕ item_y;
	ಪೂರ್ಣಾಙ್ಕ text_pos_offset, text_x;
	ಪೂರ್ಣಾಙ್ಕ scrollbar_width = SCROLLBAR_WIDTH;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ scroll;
	ಪೂರ್ಣಾಙ್ಕ x_cp, y_cp;
	ಸಂರಚನಾ rect r;
	ಸಂರಚನಾ rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	ಸಾಲು_height = menu->ಸಾಲು_height;

	ಸಾಲು_height_with_spacing = ಸಾಲು_height +
			ಸಾಲು_height * SELECT_ಸಾಲು_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	ಯದಿ (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / ಸಾಲು_height_with_spacing;
		scroll -= i * ಸಾಲು_height_with_spacing;
		ಸಾಲು_height *= scale;
		ಸಾಲು_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * ಸಾಲು_height_with_spacing;
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
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
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
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	option = control->data.select.ವಸ್ತುಗಳು;
	item_y = ಸಾಲು_height_with_spacing;

	ಯಾವತ್ (item_y < scroll) {
		option = option->next;
		item_y += ಸಾಲು_height_with_spacing;
	}
	item_y -= ಸಾಲು_height_with_spacing;
	text_pos_offset = y - scroll +
			(ಪೂರ್ಣಾಙ್ಕ) (ಸಾಲು_height * (0.75 + SELECT_ಸಾಲು_SPACING));
	text_x = x + (box->border[ಎಡ_2].width + box->padding[ಎಡ_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	ಯಾವತ್ (option && item_y - scroll < height) {

		ಯದಿ (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + ಸಾಲು_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		item_y += ಸಾಲು_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ಸಂರಚನಾ form_control *control,
			     ಭಿನ್ನ scale,
			     ಧ್ರುವ ಸಂರಚನಾ rect *clip)
{
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;
	ಪೂರ್ಣಾಙ್ಕ width, height;


	width = menu->width;
	height = menu->height;

	ಯದಿ (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	ಯದಿ (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		ಪ್ರತ್ಯಾಗಮ true;

	ಪ್ರತ್ಯಾಗಮ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ಸಂರಚನಾ form_control *control, ಪೂರ್ಣಾಙ್ಕ item)
{
	ಪ್ರತಿಜ್ಞಾ(control != ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
ಸಂರಚನಾ form_option *
form_select_get_option(ಸಂರಚನಾ form_control *control, ಪೂರ್ಣಾಙ್ಕ item)
{
	ಸಂರಚನಾ form_option *opt;

	opt = control->data.select.ವಸ್ತುಗಳು;
	ಯಾವತ್ ((opt != ರಿಕ್ತ) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	ಪ್ರತ್ಯಾಗಮ opt;
}


/* exported interface documented in netsurf/form.h */
ವರ್ಣ *form_control_get_name(ಸಂರಚನಾ form_control *control)
{
	ಪ್ರತ್ಯಾಗಮ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ಸಂರಚನಾ form_control *control, ಸಂರಚನಾ rect *r)
{
	box_bounds( control->box, r );
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ಧ್ರುವ ವರ್ಣ *
form_select_mouse_action(ಸಂರಚನಾ form_control *control,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;
	ಪೂರ್ಣಾಙ್ಕ x0, y0, x1, y1, scrollbar_x;
	ಧ್ರುವ ವರ್ಣ *status = ರಿಕ್ತ;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	ಯದಿ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ಪ್ರತ್ಯಾಗಮ scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ಯದಿ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ಯದಿ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ಯದಿ (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} ಅನ್ಯಥಾ ಯದಿ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	ಪ್ರತ್ಯಾಗಮ status;
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ
form_select_mouse_drag_end(ಸಂರಚನಾ form_control *control,
			   browser_mouse_state mouse,
			   ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y)
{
	ಪೂರ್ಣಾಙ್ಕ x0, y0, x1, y1;
	ಪೂರ್ಣಾಙ್ಕ box_x, box_y;
	ಸಂರಚನಾ box *box;
	ಸಂರಚನಾ form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ಎಡ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[ತುದಿ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ಯದಿ (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		ಪ್ರತ್ಯಾಗಮ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	ಯದಿ (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_select_get_dimensions(ಸಂರಚನಾ form_control *control,
		ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_select_menu_callback(ಶೂನ್ಯ *client_data,
		ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	html_content *html = client_data;
	ಪೂರ್ಣಾಙ್ಕ menu_x, menu_y;
	ಸಂರಚನಾ box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ಎಡ_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[ತುದಿ_2];
	content__request_redraw((ಸಂರಚನಾ content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_radio_set(ಸಂರಚನಾ form_control *radio)
{
	ಸಂರಚನಾ form_control *control;

	ಪ್ರತಿಜ್ಞಾ(radio);
	ಯದಿ (!radio->form)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (radio->selected)
		ಪ್ರತ್ಯಾಗಮ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	ಪ್ರತಿ (control = radio->form->controls;
	     control != ರಿಕ್ತ;
	     control = control->next) {
		/* Only interested in radio inputs */
		ಯದಿ (control->type != GADGET_RADIO)
			ಅನುವರ್ತ;

		/* Ignore ourself */
		ಯದಿ (control == radio)
			ಅನುವರ್ತ;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ಯದಿ ((control->name == ರಿಕ್ತ) ||
		    (radio->name == ರಿಕ್ತ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ಅನುವರ್ತ;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ಯದಿ (control->selected) {
			control->selected = false;
			dom_html_ಒಳಹರಿವು_element_set_checked(control->ಗಂಟು, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ಒಳಹರಿವು_element_set_checked(radio->ಗಂಟು, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    ಸಂರಚನಾ browser_window *target,
	    ಸಂರಚನಾ form *form,
	    ಸಂರಚನಾ form_control *submit_button)
{
	nserror res;
	ವರ್ಣ *data = ರಿಕ್ತ; /* encoded form data */
	ಸಂರಚನಾ fetch_multipart_data *success = ರಿಕ್ತ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ಪ್ರತಿಜ್ಞಾ(form != ರಿಕ್ತ);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ಯದಿ (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		ಪ್ರತ್ಯಾಗಮ res;
	}

	ವಿಕಲ್ಪ (form->method) {
	ಪಕ್ಷ method_GET:
		res = form_url_encode(form, success, &data);
		ಯದಿ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			ಯದಿ (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      ರಿಕ್ತ,
							      ರಿಕ್ತ,
							      ರಿಕ್ತ);

				nsurl_unref(query_url);
			}
			ಮುಕ್ತ(data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		ಯದಿ (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      ರಿಕ್ತ,
						      ರಿಕ್ತ);
			ಮುಕ್ತ(data);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      ರಿಕ್ತ,
					      success,
					      ರಿಕ್ತ);

		ಭಙ್ಗ;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	ಪ್ರತ್ಯಾಗಮ res;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_gadget_update_ಮೌಲ್ಯ(ಸಂರಚನಾ form_control *control, ವರ್ಣ *ಮೌಲ್ಯ)
{
	ವಿಕಲ್ಪ (control->type) {
	ಪಕ್ಷ GADGET_HIDDEN:
	ಪಕ್ಷ GADGET_TEXTBOX:
	ಪಕ್ಷ GADGET_TEXTAREA:
	ಪಕ್ಷ GADGET_PASSWORD:
	ಪಕ್ಷ GADGET_FILE:
		ಯದಿ (control->ಮೌಲ್ಯ != ರಿಕ್ತ) {
			ಮುಕ್ತ(control->ಮೌಲ್ಯ);
		}
		control->ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಯದಿ (control->ಗಂಟು != ರಿಕ್ತ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)ಮೌಲ್ಯ,
						strlen(ಮೌಲ್ಯ), &str);
			ಯದಿ (err == DOM_NO_ERR) {
				ಯದಿ (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_ಮೌಲ್ಯ(
						(dom_html_text_area_element *)(control->ಗಂಟು),
						str);
				ಅನ್ಯಥಾ
					err = dom_html_ಒಳಹರಿವು_element_set_ಮೌಲ್ಯ(
						(dom_html_ಒಳಹರಿವು_element *)(control->ಗಂಟು),
						str);
				dom_string_unref(str);
			}
		}
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		/* Do nothing */
		ಭಙ್ಗ;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
ಶೂನ್ಯ
form_gadget_sync_with_dom(ಸಂರಚನಾ form_control *control)
{
	dom_exception exc;
	dom_string *ಮೌಲ್ಯ = ರಿಕ್ತ;
	bool changed_dom = false;

	ಯದಿ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ಪ್ರತ್ಯಾಗಮ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ಯದಿ ((control->last_synced_ಮೌಲ್ಯ == ರಿಕ್ತ &&
	     control->ಮೌಲ್ಯ != ರಿಕ್ತ &&
	     control->ಮೌಲ್ಯ[0] != '\0') ||
	    (control->last_synced_ಮೌಲ್ಯ != ರಿಕ್ತ &&
	     control->ಮೌಲ್ಯ != ರಿಕ್ತ &&
	     strcmp(control->ಮೌಲ್ಯ, control->last_synced_ಮೌಲ್ಯ) != 0)) {
		ವರ್ಣ *dup = strdup(control->ಮೌಲ್ಯ);
		ಯದಿ (dup == ರಿಕ್ತ) {
			ಗಚ್ಛ out;
		}
		ಯದಿ (control->last_synced_ಮೌಲ್ಯ != ರಿಕ್ತ) {
			ಮುಕ್ತ(control->last_synced_ಮೌಲ್ಯ);
		}
		control->last_synced_ಮೌಲ್ಯ = dup;
		exc = dom_string_create((uint8_t *)(control->ಮೌಲ್ಯ),
					strlen(control->ಮೌಲ್ಯ), &ಮೌಲ್ಯ);
		ಯದಿ (exc != DOM_NO_ERR) {
			ಗಚ್ಛ out;
		}
		ಯದಿ (control->ಗಂಟು_ಮೌಲ್ಯ != ರಿಕ್ತ) {
			dom_string_unref(control->ಗಂಟು_ಮೌಲ್ಯ);
		}
		control->ಗಂಟು_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಮೌಲ್ಯ = ರಿಕ್ತ;
		ಯದಿ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_ಮೌಲ್ಯ(control->ಗಂಟು, control->ಗಂಟು_ಮೌಲ್ಯ);
		} ಅನ್ಯಥಾ {
			exc = dom_html_ಒಳಹರಿವು_element_set_ಮೌಲ್ಯ(control->ಗಂಟು, control->ಗಂಟು_ಮೌಲ್ಯ);
		}
		ಯದಿ (exc != DOM_NO_ERR) {
			ಗಚ್ಛ out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ಯದಿ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_ಮೌಲ್ಯ(control->ಗಂಟು, &ಮೌಲ್ಯ);
	} ಅನ್ಯಥಾ {
		exc = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(control->ಗಂಟು, &ಮೌಲ್ಯ);
	}

	ಯದಿ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		ಗಚ್ಛ out;
	}

	ಯದಿ (!dom_string_isequal(control->ಗಂಟು_ಮೌಲ್ಯ, ಮೌಲ್ಯ)) {
		/* The DOM has changed */
		ಯದಿ (!changed_dom) {
			/* And it wasn't us */
			ವರ್ಣ *ಮೌಲ್ಯ_s = strndup(
				dom_string_data(ಮೌಲ್ಯ),
				dom_string_byte_length(ಮೌಲ್ಯ));
			ವರ್ಣ *dup = ರಿಕ್ತ;
			ಯದಿ (ಮೌಲ್ಯ_s == ರಿಕ್ತ) {
				ಗಚ್ಛ out;
			}
			dup = strdup(ಮೌಲ್ಯ_s);
			ಯದಿ (dup == ರಿಕ್ತ) {
				ಮುಕ್ತ(ಮೌಲ್ಯ_s);
				ಗಚ್ಛ out;
			}
			ಮುಕ್ತ(control->ಮೌಲ್ಯ);
			control->ಮೌಲ್ಯ = ಮೌಲ್ಯ_s;
			ಮುಕ್ತ(control->last_synced_ಮೌಲ್ಯ);
			control->last_synced_ಮೌಲ್ಯ = dup;
			ಯದಿ (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != ರಿಕ್ತ) {
				textarea_set_text(control->data.text.ta,
						  ಮೌಲ್ಯ_s);
			}
		}
		control->ಗಂಟು_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಮೌಲ್ಯ = ರಿಕ್ತ;
	}

out:
	ಯದಿ (ಮೌಲ್ಯ != ರಿಕ್ತ)
		dom_string_unref(ಮೌಲ್ಯ);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ಸಂರಚನಾ form *
form_new(ಶೂನ್ಯ *ಗಂಟು,
	 ಧ್ರುವ ವರ್ಣ *action,
	 ಧ್ರುವ ವರ್ಣ *target,
	 form_method method,
	 ಧ್ರುವ ವರ್ಣ *charset,
	 ಧ್ರುವ ವರ್ಣ *doc_charset)
{
	ಸಂರಚನಾ form *form;

	form = calloc(1, ಪರಿಮಾಣ *form);
	ಯದಿ (!form)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	form->action = strdup(action != ರಿಕ್ತ ? action : "");
	ಯದಿ (form->action == ರಿಕ್ತ) {
		ಮುಕ್ತ(form);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	form->target = target != ರಿಕ್ತ ? strdup(target) : ರಿಕ್ತ;
	ಯದಿ (target != ರಿಕ್ತ && form->target == ರಿಕ್ತ) {
		ಮುಕ್ತ(form->action);
		ಮುಕ್ತ(form);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	form->method = method;

	form->accept_charsets = charset != ರಿಕ್ತ ? strdup(charset) : ರಿಕ್ತ;
	ಯದಿ (charset != ರಿಕ್ತ && form->accept_charsets == ರಿಕ್ತ) {
		ಮುಕ್ತ(form->target);
		ಮುಕ್ತ(form->action);
		ಮುಕ್ತ(form);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	form->document_charset = doc_charset != ರಿಕ್ತ ? strdup(doc_charset)
						     : ರಿಕ್ತ;
	ಯದಿ (doc_charset && form->document_charset == ರಿಕ್ತ) {
		ಮುಕ್ತ(form->accept_charsets);
		ಮುಕ್ತ(form->target);
		ಮುಕ್ತ(form->action);
		ಮುಕ್ತ(form);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	form->ಗಂಟು = ಗಂಟು;

	ಪ್ರತ್ಯಾಗಮ form;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free(ಸಂರಚನಾ form *form)
{
	ಸಂರಚನಾ form_control *c, *d;

	ಪ್ರತಿ (c = form->controls; c != ರಿಕ್ತ; c = d) {
		d = c->next;

		form_free_control(c);
	}

	ಮುಕ್ತ(form->action);
	ಮುಕ್ತ(form->target);
	ಮುಕ್ತ(form->accept_charsets);
	ಮುಕ್ತ(form->document_charset);

	ಮುಕ್ತ(form);
}


/* exported interface documented in html/form_internal.h */
ಸಂರಚನಾ form_control *form_new_control(ಶೂನ್ಯ *ಗಂಟು, form_control_type type)
{
	ಸಂರಚನಾ form_control *control;

	control = calloc(1, ಪರಿಮಾಣ *control);
	ಯದಿ (control == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	control->ಗಂಟು = ಗಂಟು;
	control->type = type;

	ಪ್ರತ್ಯಾಗಮ control;
}
