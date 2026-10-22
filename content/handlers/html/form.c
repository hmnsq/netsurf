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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/url.h"
#ଅନ୍ତର୍ଭାଵ "utils/utf8.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "content/fetch.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"
#ଅନ୍ତର୍ଭାଵ "desktop/knockout.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/layout.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_ଧାଡ଼ି_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

ସଂରଚନା form_select_menu {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
	ସଂରଚନା scrollbar *scrollbar;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	ଶୂନ୍ୟ *client_data;
	ସଂରଚନା content *c;
};

ସ୍ଥିର plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

ସ୍ଥିର plot_font_style_t plot_fstyle_entry = {
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
ସ୍ଥିର ଅକ୍ଷର *
form_encode_item(ଅଚଳ ଅକ୍ଷର *item,
		 uint32_t len,
		 ଅଚଳ ଅକ୍ଷର *charset,
		 ଅଚଳ ଅକ୍ଷର *fallback)
{
	nserror err;
	ଅକ୍ଷର *ret = ଖାଲି;
	ଅକ୍ଷର cset[256];

	ଯଦି (!item || !charset)
		ଫେରନ୍ତୁ ଖାଲି;

	snprintf(cset, ଆକାର cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	ଯଦି (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, ଆକାର cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		ଯଦି (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ଯଦି (fallback) {
				snprintf(cset, ଆକାର cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				ଯଦି (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, ଆକାର cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			ଯଦି (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ଯଦି (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ଯଦି (err == NSERROR_NOMEM) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_ମୂଲ୍ୟ_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ସ୍ଥିର nserror
fetch_data_list_add_sname(ଅଚଳ ଅକ୍ଷର *name,
			  ଅଚଳ ଅକ୍ଷର *ksfx,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ,
			  ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	ସଂରଚନା fetch_multipart_data *fetch_data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା keysize;

	fetch_data = calloc(1, ଆକାର(*fetch_data));
	ଯଦି (fetch_data == ଖାଲି) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	ଯଦି (fetch_data->name == ଖାଲି) {
		ମୁକ୍ତ(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*ମୂଲ୍ୟ*/
	fetch_data->ମୂଲ୍ୟ = malloc(FETCH_DATA_INT_ମୂଲ୍ୟ_SIZE);
	ଯଦି (fetch_data->ମୂଲ୍ୟ == ଖାଲି) {
		ମୁକ୍ତ(fetch_data->name);
		ମୁକ୍ତ(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}
	snprintf(fetch_data->ମୂଲ୍ୟ, FETCH_DATA_INT_ମୂଲ୍ୟ_SIZE, "%d", ମୂଲ୍ୟ);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
fetch_data_list_add(dom_string *name,
		    dom_string *ମୂଲ୍ୟ,
		    ଅଚଳ ଅକ୍ଷର *rawfile,
		    ଅଚଳ ଅକ୍ଷର *form_charset,
		    ଅଚଳ ଅକ୍ଷର *docu_charset,
		    ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	ସଂରଚନା fetch_multipart_data *fetch_data;

	ପ୍ରତିଜ୍ଞା(name != ଖାଲି);

	fetch_data = calloc(1, ଆକାର(*fetch_data));
	ଯଦି (fetch_data == ଖାଲି) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	ଯଦି (fetch_data->name == ଖାଲି) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		ମୁକ୍ତ(fetch_data);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଯଦି (ମୂଲ୍ୟ == ଖାଲି) {
		fetch_data->ମୂଲ୍ୟ = strdup("");
	} ଅନ୍ୟଥା {
		fetch_data->ମୂଲ୍ୟ = form_encode_item(dom_string_data(ମୂଲ୍ୟ),
						     dom_string_byte_length(ମୂଲ୍ୟ),
						     form_charset,
						     docu_charset);
	}
	ଯଦି (fetch_data->ମୂଲ୍ୟ == ଖାଲି) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		ମୁକ୍ତ(fetch_data->name);
		ମୁକ୍ତ(fetch_data);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ଯଦି (rawfile != ଖାଲି) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		ଯଦି (fetch_data->rawfile == ଖାଲି) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			ମୁକ୍ତ(fetch_data->ମୂଲ୍ୟ);
			ମୁକ୍ତ(fetch_data->name);
			ମୁକ୍ତ(fetch_data);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	ଫେରନ୍ତୁ NSERROR_OK;
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
ସ୍ଥିର nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  ଅଚଳ ଅକ୍ଷର *form_charset,
			  ଅଚଳ ଅକ୍ଷର *doc_charset,
			  ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (inputname == ଖାଲି) {
		/* allow enumeration to continue after element with no name */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_ମୂଲ୍ୟ(text_area_element,
						   &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ଖାଲି,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ଅଚଳ ଅକ୍ଷର *form_charset,
			      ଅଚଳ ଅକ୍ଷର *docu_charset,
			      ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *ମୂଲ୍ୟ;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (!selected) {
		/* unselected options do not add fetch data entries */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	exp = dom_html_option_element_get_ମୂଲ୍ୟ(option_element, &ମୂଲ୍ୟ);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  ମୂଲ୍ୟ,
				  ଖାଲି,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(ମୂଲ୍ୟ);

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			ଅଚଳ ଅକ୍ଷର *form_charset,
			ଅଚଳ ଅକ୍ଷର *doc_charset,
			ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ଖାଲି;
	uint32_t options_ଗଣନା;
	uint32_t option_index;
	dom_ଗଣ୍ଠି *option_element = ଖାଲି;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (inputname == ଖାଲି) {
		/* allow enumeration to continue after element with no name */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_ଗଣନା);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* iterate over options collection */
	ପାଇଁ (option_index = 0; option_index < options_ଗଣନା; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		ଯଦି (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} ଅନ୍ୟଥା {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_ଗଣ୍ଠି_unref(option_element);
		}

		ଯଦି (res != NSERROR_OK) {
			ଭାଙ୍ଗ;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ_submit(dom_html_ନିବେଶ_element *ନିବେଶ_element,
			      dom_string *inputname,
			      ଅଚଳ ଅକ୍ଷର *charset,
			      ଅଚଳ ଅକ୍ଷର *document_charset,
			      dom_html_element **submit_button,
			      ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ଯଦି (*submit_button == ଖାଲି) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ନିବେଶ_element;
	} ଅନ୍ୟଥା ଯଦି (*submit_button != (dom_html_element *)ନିବେଶ_element) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(ନିବେଶ_element, &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ଖାଲି,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ_image(dom_html_ନିବେଶ_element *ନିବେଶ_element,
			     dom_string *inputname,
			     ଅଚଳ ଅକ୍ଷର *charset,
			     ଅଚଳ ଅକ୍ଷର *document_charset,
			     dom_html_element **submit_button,
			     ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	ସଂରଚନା image_ନିବେଶ_coords *coords;
	ଅକ୍ଷର *basename;

	/* Only use an image input if it was the thing which activated us */
	ଯଦି (*submit_button != (dom_html_element *)ନିବେଶ_element) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	exp = dom_ଗଣ୍ଠି_get_user_data((dom_ଗଣ୍ଠି *)ନିବେଶ_element,
				     corestring_dom_ns_ଚାବି_image_coords_ଗଣ୍ଠି_data,
				     &coords);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (coords == ଖାଲି) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	ଯଦି (basename == ଖାଲି) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	ଯଦି (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	ମୁକ୍ତ(basename);

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ_checkbox(dom_html_ନିବେଶ_element *ନିବେଶ_element,
				dom_string *inputname,
				ଅଚଳ ଅକ୍ଷର *charset,
				ଅଚଳ ଅକ୍ଷର *document_charset,
				ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_ନିବେଶ_element_get_checked(ନିବେଶ_element, &checked);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (!checked) {
		/* unchecked items do not generate a data entry */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	exp = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(ନିବେଶ_element, &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* ensure a default value */
	ଯଦି (inputvalue == ଖାଲି) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ଖାଲି,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ_file(dom_html_ନିବେଶ_element *ନିବେଶ_element,
			    dom_string *inputname,
			    ଅଚଳ ଅକ୍ଷର *charset,
			    ଅଚଳ ଅକ୍ଷର *document_charset,
			    ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ଅଚଳ ଅକ୍ଷର *rawfile = ଖାଲି;

	exp = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(ନିବେଶ_element, &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	exp = dom_ଗଣ୍ଠି_get_user_data((dom_ଗଣ୍ଠି *)ନିବେଶ_element,
				     corestring_dom_ns_ଚାବି_file_name_ଗଣ୍ଠି_data,
				     &rawfile);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (rawfile == ଖାଲି) {
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

	ଫେରନ୍ତୁ res;
}


ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ_text(dom_html_ନିବେଶ_element *ନିବେଶ_element,
			    dom_string *inputname,
			    ଅଚଳ ଅକ୍ଷର *charset,
			    ଅଚଳ ଅକ୍ଷର *document_charset,
			    ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(ନିବେଶ_element, &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ଖାଲି,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror
form_dom_to_data_ନିବେଶ(dom_html_ନିବେଶ_element *ନିବେଶ_element,
		       ଅଚଳ ଅକ୍ଷର *charset,
		       ଅଚଳ ଅକ୍ଷର *document_charset,
		       dom_html_element **submit_button,
		       ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_ନିବେଶ_element_get_disabled(ନିବେଶ_element,
						  &element_disabled);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (element_disabled) {
		/* disabled element requires no more processing */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_ନିବେଶ_element_get_name(ନିବେଶ_element, &inputname);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (inputname == ଖାଲି) {
		/* element with no name is not converted */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_ନିବେଶ_element_get_type(ନିବେଶ_element, &inputtype);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* process according to input element type */
	ଯଦି (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_ନିବେଶ_submit(ନିବେଶ_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_ନିବେଶ_image(ନିବେଶ_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_ନିବେଶ_checkbox(ନିବେଶ_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_ନିବେଶ_file(ନିବେଶ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ଅନ୍ୟଥା {
		/* Everything else is treated as text values */
		res = form_dom_to_data_ନିବେଶ_text(ନିବେଶ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ଫେରନ୍ତୁ res;
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
ସ୍ଥିର nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			ଅଚଳ ଅକ୍ଷର *form_charset,
			ଅଚଳ ଅକ୍ଷର *doc_charset,
			dom_html_element **submit_button,
			ସଂରଚନା fetch_multipart_data ***fetch_data_next_ptr)
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
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ଯଦି (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ଫେରନ୍ତୁ NSERROR_OK;
	}
	ଯଦି (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ଫେରନ୍ତୁ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ଯଦି (*submit_button == ଖାଲି) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ଯଦି (*submit_button != (dom_html_element *)button_element) {
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	ଯଦି (inputname == ଖାଲି) {
		/* allow enumeration to continue after element with no name */
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_ମୂଲ୍ୟ(button_element, &inputvalue);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ଅନ୍ୟଥା {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  ଖାଲି,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ଫେରନ୍ତୁ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ସ୍ଥିର ଅକ୍ଷର *form_acceptable_charset(ସଂରଚନା form *form)
{
	ଅକ୍ଷର *temp, *c;

	ଯଦି (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ଯଦି (form->document_charset) {
			/* document charset present, so use it */
			ଫେରନ୍ତୁ strdup(form->document_charset);
		} ଅନ୍ୟଥା {
			/* no document charset, so default to 8859-1 */
			ଫେରନ୍ତୁ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	ଯଦି (!temp)
		ଫେରନ୍ତୁ ଖାଲି;

	/* make it upper case */
	ପାଇଁ (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	ଯଦି (c) {
		ମୁକ୍ତ(temp);
		ଫେରନ୍ତୁ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	ମୁକ୍ତ(temp);

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
	ଯେତେବେଳେ (*c && !ascii_is_space(*c)) {
		ଯଦି (*c == ',')
			ଭାଙ୍ଗ;
		c++;
	}

	ଫେରନ୍ତୁ strndup(form->accept_charsets, c - form->accept_charsets);
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
ସ୍ଥିର nserror
form_dom_to_data(ସଂରଚନା form *form,
		 ସଂରଚନା form_control *submit_control,
		 ସଂରଚନା fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	ଅକ୍ଷର *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = ଖାଲି; /* the dom form elements */
	uint32_t element_ଗଣନା; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ଗଣ୍ଠି *element = ଖାଲି; /* the DOM form element */
	dom_string *nodename = ଖାଲି; /* the DOM node name of the element */
	ସଂରଚନା fetch_multipart_data *fetch_data = ଖାଲି; /* fetch data list */
	ସଂରଚନା fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ଯଦି (submit_control != ଖାଲି) {
		submit_button = submit_control->ଗଣ୍ଠି;
	} ଅନ୍ୟଥା {
		submit_button = ଖାଲି;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ଯଦି (charset == ଖାଲି) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->ଗଣ୍ଠି, &elements);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		ମୁକ୍ତ(charset);
		ଫେରନ୍ତୁ NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_ଗଣନା);
	ଯଦି (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		ଯାଅ form_dom_to_data_error;
	}

	ପାଇଁ (element_idx = 0; element_idx < element_ଗଣନା; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		ଯଦି (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			ଯାଅ form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(element, &nodename);
		ଯଦି (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_ଗଣ୍ଠି_unref(element);
			res = NSERROR_DOM;
			ଯାଅ form_dom_to_data_error;
		}

		ଯଦି (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(nodename, corestring_dom_ନିବେଶ)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_ନିବେଶ(
				(dom_html_ନିବେଶ_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ଅନ୍ୟଥା ଯଦି (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} ଅନ୍ୟଥା {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ଗଣ୍ଠି_unref(element);

		/* abort form element enumeration on error */
		ଯଦି (res != NSERROR_OK) {
			ଯାଅ form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	ମୁକ୍ତ(charset);

	ଫେରନ୍ତୁ NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	ମୁକ୍ତ(charset);

	ଫେରନ୍ତୁ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ସ୍ଥିର nserror
form_url_encode(ସଂରଚନା form *form,
		ସଂରଚନା fetch_multipart_data *control,
		ଅକ୍ଷର **encoded_out)
{
	ଅକ୍ଷର *name, *ମୂଲ୍ୟ;
	ଅକ୍ଷର *s, *s2;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, len1, len_init;
	nserror res;

	s = malloc(1);

	ଯଦି (s == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	ପାଇଁ (; control; control = control->next) {
		res = url_escape(control->name, true, ଖାଲି, &name);
		ଯଦି (res != NSERROR_OK) {
			ମୁକ୍ତ(s);
			ଫେରନ୍ତୁ res;
		}

		res = url_escape(control->ମୂଲ୍ୟ, true, ଖାଲି, &ମୂଲ୍ୟ);
		ଯଦି (res != NSERROR_OK) {
			ମୁକ୍ତ(name);
			ମୁକ୍ତ(s);
			ଫେରନ୍ତୁ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(ମୂଲ୍ୟ) + 2;
		s2 = realloc(s, len1 + 1);
		ଯଦି (s2 == ଖାଲି) {
			ମୁକ୍ତ(ମୂଲ୍ୟ);
			ମୁକ୍ତ(name);
			ମୁକ୍ତ(s);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, ମୂଲ୍ୟ);
		len = len1;
		ମୁକ୍ତ(name);
		ମୁକ୍ତ(ମୂଲ୍ୟ);
	}

	ଯଦି (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ସ୍ଥିର ଶୂନ୍ୟ
form_select_menu_scroll_callback(ଶୂନ୍ୟ *client_data,
				 ସଂରଚନା scrollbar_ସନ୍ଦେଶ_data *scrollbar_data)
{
	ସଂରଚନା form_control *control = client_data;
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	ବାଛ (scrollbar_data->ସନ୍ଦେଶ) {
		ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_SCROLL_START:
		{
			ସଂରଚନା rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି SCROLLBAR_ସନ୍ଦେଶ_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, ଖାଲି);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
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
ସ୍ଥିର nserror
form__select_process_selection(html_content *html,
			       ସଂରଚନା form_control *control,
			       ପୂର୍ଣ୍ଣସଂଖ୍ୟା item)
{
	ସଂରଚନା box *inline_box;
	ସଂରଚନା form_option *o;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
	nserror ret = NSERROR_OK;

	ପ୍ରତିଜ୍ଞା(control != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(html != ଖାଲି);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	ପାଇଁ (ଗଣନା_2 = 0, o = control->data.select.ବସ୍ତୁଗୁଡ଼ିକ;
			o != ଖାଲି;
			ଗଣନା_2++, o = o->next) {
		ଯଦି (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->ଗଣ୍ଠି, false);
		}

		ଯଦି (ଗଣନା_2 == item) {
			ଯଦି (control->data.select.multiple) {
				ଯଦି (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->ଗଣ୍ଠି, false);
					control->data.select.ଅଙ୍କ_selected--;
				} ଅନ୍ୟଥା {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->ଗଣ୍ଠି, true);
					control->data.select.ଅଙ୍କ_selected++;
				}
			} ଅନ୍ୟଥା {
				dom_html_option_element_set_selected(
						o->ଗଣ୍ଠି, true);
				o->selected = true;
			}
		}

		ଯଦି (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	ଯଦି (control->data.select.ଅଙ୍କ_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ଅନ୍ୟଥା ଯଦି (control->data.select.ଅଙ୍କ_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} ଅନ୍ୟଥା {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ଯଦି (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} ଅନ୍ୟଥା {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	ଫେରନ୍ତୁ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ସ୍ଥିର ଶୂନ୍ୟ form_select_menu_clicked(ସଂରଚନା form_control *control, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;
	ସଂରଚନା form_option *option;
	html_content *html = (html_content *)menu->c;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_height, ଧାଡ଼ି_height_with_spacing;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା item_bottom_y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	ଧାଡ଼ି_height = menu->ଧାଡ଼ି_height;
	ଧାଡ଼ି_height_with_spacing = ଧାଡ଼ି_height +
			ଧାଡ଼ି_height * SELECT_ଧାଡ଼ି_SPACING;

	option = control->data.select.ବସ୍ତୁଗୁଡ଼ିକ;
	item_bottom_y = ଧାଡ଼ି_height_with_spacing;
	i = 0;
	ଯେତେବେଳେ (option && item_bottom_y < scroll + y) {
		item_bottom_y += ଧାଡ଼ି_height_with_spacing;
		option = option->next;
		i++;
	}

	ଯଦି (option != ଖାଲି) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
ଶୂନ୍ୟ form_add_control(ସଂରଚନା form *form, ସଂରଚନା form_control *control)
{
	ଯଦି (form == ଖାଲି) {
		ଫେରନ୍ତୁ;
	}

	control->form = form;

	ଯଦି (form->controls != ଖାଲି) {
		ପ୍ରତିଜ୍ଞା(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = ଖାଲି;
		form->last_control = control;
	} ଅନ୍ୟଥା {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
ଶୂନ୍ୟ form_free_control(ସଂରଚନା form_control *control)
{
	ସଂରଚନା form_control *c;
	ପ୍ରତିଜ୍ଞା(control != ଖାଲି);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->ମୂଲ୍ୟ, control->initial_ମୂଲ୍ୟ);
	ମୁକ୍ତ(control->name);
	ମୁକ୍ତ(control->ମୂଲ୍ୟ);
	ମୁକ୍ତ(control->initial_ମୂଲ୍ୟ);
	ଯଦି (control->last_synced_ମୂଲ୍ୟ != ଖାଲି) {
		ମୁକ୍ତ(control->last_synced_ମୂଲ୍ୟ);
	}

	ଯଦି (control->type == GADGET_SELECT) {
		ସଂରଚନା form_option *option, *next;

		ପାଇଁ (option = control->data.select.ବସ୍ତୁଗୁଡ଼ିକ; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->ମୂଲ୍ୟ);
			ମୁକ୍ତ(option->text);
			ମୁକ୍ତ(option->ମୂଲ୍ୟ);
			ମୁକ୍ତ(option);
		}
		ଯଦି (control->data.select.menu != ଖାଲି) {
			form_free_select_menu(control);
		}
	}

	ଯଦି (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ଯଦି (control->data.text.initial != ଖାଲି) {
			dom_string_unref(control->data.text.initial);
		}

		ଯଦି (control->data.text.ta != ଖାଲି) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	ଯଦି (control->form != ଖାଲି) {
		ପାଇଁ (c = control->form->controls; c != ଖାଲି; c = c->next) {
			ଯଦି (c->next == control) {
				c->next = control->next;
				ଯଦି (control->form->last_control == control)
					control->form->last_control = c;
				ଭାଙ୍ଗ;
			}
			ଯଦି (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				ଯଦି (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = ଖାଲି;
				ଭାଙ୍ଗ;
			}
		}
	}

	ଯଦି (control->ଗଣ୍ଠି_ମୂଲ୍ୟ != ଖାଲି) {
		dom_string_unref(control->ଗଣ୍ଠି_ମୂଲ୍ୟ);
	}

	ମୁକ୍ତ(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(ସଂରଚନା form_control *control, ଅକ୍ଷର *ମୂଲ୍ୟ, ଅକ୍ଷର *text,
		     bool selected, ଶୂନ୍ୟ *ଗଣ୍ଠି)
{
	ସଂରଚନା form_option *option;

	ପ୍ରତିଜ୍ଞା(control);
	ପ୍ରତିଜ୍ଞା(control->type == GADGET_SELECT);

	option = calloc(1, ଆକାର *option);
	ଯଦି (!option)
		ଫେରନ୍ତୁ false;

	option->ମୂଲ୍ୟ = ମୂଲ୍ୟ;
	option->text = text;

	/* add to linked list */
	ଯଦି (control->data.select.ବସ୍ତୁଗୁଡ଼ିକ == 0)
		control->data.select.ବସ୍ତୁଗୁଡ଼ିକ = option;
	ଅନ୍ୟଥା
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	ଯଦି (selected && (control->data.select.ଅଙ୍କ_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.ଅଙ୍କ_selected++;
		control->data.select.current = option;
	}

	control->data.select.ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ++;

	option->ଗଣ୍ଠି = ଗଣ୍ଠି;

	ଫେରନ୍ତୁ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(ଶୂନ୍ୟ *client_data,
		      ସଂରଚନା form_control *control,
		      select_menu_redraw_callback callback,
		      ସଂରଚନା content *c)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_height_with_spacing;
	ସଂରଚନା box *box;
	plot_font_style_t fstyle;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_height;
	ସଂରଚନା form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ଯଦି (control->data.select.menu == ଖାଲି) {

		menu = calloc(1, ଆକାର (ସଂରଚନା form_select_menu));
		ଯଦି (menu == ଖାଲି) {
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[ଡାହାଣ_2].width + box->padding[ଡାହାଣ_2] +
			box->border[ବାମ_2].width + box->padding[ବାମ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->ଧାଡ଼ି_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		ଧାଡ଼ି_height_with_spacing = menu->ଧାଡ଼ି_height +
				menu->ଧାଡ଼ି_height *
				SELECT_ଧାଡ଼ି_SPACING;

		total_height = control->data.select.ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ *
				ଧାଡ଼ି_height_with_spacing;
		menu->height = total_height;

		ଯଦି (menu->height > MAX_SELECT_HEIGHT) {
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
		ଯଦି (res != NSERROR_OK) {
			control->data.select.menu = ଖାଲି;
			ମୁକ୍ତ(menu);
			ଫେରନ୍ତୁ res;
		}
		menu->c = c;
	} ଅନ୍ୟଥା {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
ଶୂନ୍ୟ form_free_select_menu(ସଂରଚନା form_control *control)
{
	ଯଦି (control->data.select.menu->scrollbar != ଖାଲି)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	ମୁକ୍ତ(control->data.select.menu);
	control->data.select.menu = ଖାଲି;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ସଂରଚନା form_control *control,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
			ଦଶମିକ scale,
			ଅଚଳ ସଂରଚନା rect *clip,
			ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା box *box;
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;
	ସଂରଚନା form_option *option;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି_height, ଧାଡ଼ି_height_with_spacing;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, y0, x1, scrollbar_x, y1, y2, y3;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା item_y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା text_pos_offset, text_x;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scrollbar_width = SCROLLBAR_WIDTH;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା scroll;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_cp, y_cp;
	ସଂରଚନା rect r;
	ସଂରଚନା rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	ଧାଡ଼ି_height = menu->ଧାଡ଼ି_height;

	ଧାଡ଼ି_height_with_spacing = ଧାଡ଼ି_height +
			ଧାଡ଼ି_height * SELECT_ଧାଡ଼ି_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	ଯଦି (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / ଧାଡ଼ି_height_with_spacing;
		scroll -= i * ଧାଡ଼ି_height_with_spacing;
		ଧାଡ଼ି_height *= scale;
		ଧାଡ଼ି_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * ଧାଡ଼ି_height_with_spacing;
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
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
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
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	option = control->data.select.ବସ୍ତୁଗୁଡ଼ିକ;
	item_y = ଧାଡ଼ି_height_with_spacing;

	ଯେତେବେଳେ (item_y < scroll) {
		option = option->next;
		item_y += ଧାଡ଼ି_height_with_spacing;
	}
	item_y -= ଧାଡ଼ି_height_with_spacing;
	text_pos_offset = y - scroll +
			(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (ଧାଡ଼ି_height * (0.75 + SELECT_ଧାଡ଼ି_SPACING));
	text_x = x + (box->border[ବାମ_2].width + box->padding[ବାମ_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	ଯେତେବେଳେ (option && item_y - scroll < height) {

		ଯଦି (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + ଧାଡ଼ି_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}

		item_y += ଧାଡ଼ି_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ସଂରଚନା form_control *control,
			     ଦଶମିକ scale,
			     ଅଚଳ ସଂରଚନା rect *clip)
{
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;


	width = menu->width;
	height = menu->height;

	ଯଦି (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	ଯଦି (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		ଫେରନ୍ତୁ true;

	ଫେରନ୍ତୁ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ସଂରଚନା form_control *control, ପୂର୍ଣ୍ଣସଂଖ୍ୟା item)
{
	ପ୍ରତିଜ୍ଞା(control != ଖାଲି);

	ଫେରନ୍ତୁ form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
ସଂରଚନା form_option *
form_select_get_option(ସଂରଚନା form_control *control, ପୂର୍ଣ୍ଣସଂଖ୍ୟା item)
{
	ସଂରଚନା form_option *opt;

	opt = control->data.select.ବସ୍ତୁଗୁଡ଼ିକ;
	ଯେତେବେଳେ ((opt != ଖାଲି) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	ଫେରନ୍ତୁ opt;
}


/* exported interface documented in netsurf/form.h */
ଅକ୍ଷର *form_control_get_name(ସଂରଚନା form_control *control)
{
	ଫେରନ୍ତୁ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ସଂରଚନା form_control *control, ସଂରଚନା rect *r)
{
	box_bounds( control->box, r );
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ଅଚଳ ଅକ୍ଷର *
form_select_mouse_action(ସଂରଚନା form_control *control,
			 browser_mouse_state mouse,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, y0, x1, y1, scrollbar_x;
	ଅଚଳ ଅକ୍ଷର *status = ଖାଲି;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	ଯଦି (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ଫେରନ୍ତୁ scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ଯଦି (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ଯଦି (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ଯଦି (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} ଅନ୍ୟଥା ଯଦି (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	ଫେରନ୍ତୁ status;
}


/* private interface described in html/form_internal.h */
ଶୂନ୍ୟ
form_select_mouse_drag_end(ସଂରଚନା form_control *control,
			   browser_mouse_state mouse,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x0, y0, x1, y1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_x, box_y;
	ସଂରଚନା box *box;
	ସଂରଚନା form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ବାମ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[ଶୀର୍ଷ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ଯଦି (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		ଫେରନ୍ତୁ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	ଯଦି (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
ଶୂନ୍ୟ form_select_get_dimensions(ସଂରଚନା form_control *control,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
ଶୂନ୍ୟ form_select_menu_callback(ଶୂନ୍ୟ *client_data,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	html_content *html = client_data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_x, menu_y;
	ସଂରଚନା box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ବାମ_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[ଶୀର୍ଷ_2];
	content__request_redraw((ସଂରଚନା content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
ଶୂନ୍ୟ form_radio_set(ସଂରଚନା form_control *radio)
{
	ସଂରଚନା form_control *control;

	ପ୍ରତିଜ୍ଞା(radio);
	ଯଦି (!radio->form)
		ଫେରନ୍ତୁ;

	ଯଦି (radio->selected)
		ଫେରନ୍ତୁ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	ପାଇଁ (control = radio->form->controls;
	     control != ଖାଲି;
	     control = control->next) {
		/* Only interested in radio inputs */
		ଯଦି (control->type != GADGET_RADIO)
			ଜାରି;

		/* Ignore ourself */
		ଯଦି (control == radio)
			ଜାରି;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ଯଦି ((control->name == ଖାଲି) ||
		    (radio->name == ଖାଲି) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ଜାରି;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ଯଦି (control->selected) {
			control->selected = false;
			dom_html_ନିବେଶ_element_set_checked(control->ଗଣ୍ଠି, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ନିବେଶ_element_set_checked(radio->ଗଣ୍ଠି, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    ସଂରଚନା browser_window *target,
	    ସଂରଚନା form *form,
	    ସଂରଚନା form_control *submit_button)
{
	nserror res;
	ଅକ୍ଷର *data = ଖାଲି; /* encoded form data */
	ସଂରଚନା fetch_multipart_data *success = ଖାଲି; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ପ୍ରତିଜ୍ଞା(form != ଖାଲି);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ଯଦି (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		ଫେରନ୍ତୁ res;
	}

	ବାଛ (form->method) {
	ସ୍ଥିତି method_GET:
		res = form_url_encode(form, success, &data);
		ଯଦି (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			ଯଦି (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      ଖାଲି,
							      ଖାଲି,
							      ଖାଲି);

				nsurl_unref(query_url);
			}
			ମୁକ୍ତ(data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		ଯଦି (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      ଖାଲି,
						      ଖାଲି);
			ମୁକ୍ତ(data);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      ଖାଲି,
					      success,
					      ଖାଲି);

		ଭାଙ୍ଗ;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	ଫେରନ୍ତୁ res;
}


/* exported interface documented in html/form_internal.h */
ଶୂନ୍ୟ form_gadget_update_ମୂଲ୍ୟ(ସଂରଚନା form_control *control, ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ବାଛ (control->type) {
	ସ୍ଥିତି GADGET_HIDDEN:
	ସ୍ଥିତି GADGET_TEXTBOX:
	ସ୍ଥିତି GADGET_TEXTAREA:
	ସ୍ଥିତି GADGET_PASSWORD:
	ସ୍ଥିତି GADGET_FILE:
		ଯଦି (control->ମୂଲ୍ୟ != ଖାଲି) {
			ମୁକ୍ତ(control->ମୂଲ୍ୟ);
		}
		control->ମୂଲ୍ୟ = ମୂଲ୍ୟ;
		ଯଦି (control->ଗଣ୍ଠି != ଖାଲି) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)ମୂଲ୍ୟ,
						strlen(ମୂଲ୍ୟ), &str);
			ଯଦି (err == DOM_NO_ERR) {
				ଯଦି (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_ମୂଲ୍ୟ(
						(dom_html_text_area_element *)(control->ଗଣ୍ଠି),
						str);
				ଅନ୍ୟଥା
					err = dom_html_ନିବେଶ_element_set_ମୂଲ୍ୟ(
						(dom_html_ନିବେଶ_element *)(control->ଗଣ୍ଠି),
						str);
				dom_string_unref(str);
			}
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* Do nothing */
		ଭାଙ୍ଗ;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
ଶୂନ୍ୟ
form_gadget_sync_with_dom(ସଂରଚନା form_control *control)
{
	dom_exception exc;
	dom_string *ମୂଲ୍ୟ = ଖାଲି;
	bool changed_dom = false;

	ଯଦି (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ଫେରନ୍ତୁ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ଯଦି ((control->last_synced_ମୂଲ୍ୟ == ଖାଲି &&
	     control->ମୂଲ୍ୟ != ଖାଲି &&
	     control->ମୂଲ୍ୟ[0] != '\0') ||
	    (control->last_synced_ମୂଲ୍ୟ != ଖାଲି &&
	     control->ମୂଲ୍ୟ != ଖାଲି &&
	     strcmp(control->ମୂଲ୍ୟ, control->last_synced_ମୂଲ୍ୟ) != 0)) {
		ଅକ୍ଷର *dup = strdup(control->ମୂଲ୍ୟ);
		ଯଦି (dup == ଖାଲି) {
			ଯାଅ out;
		}
		ଯଦି (control->last_synced_ମୂଲ୍ୟ != ଖାଲି) {
			ମୁକ୍ତ(control->last_synced_ମୂଲ୍ୟ);
		}
		control->last_synced_ମୂଲ୍ୟ = dup;
		exc = dom_string_create((uint8_t *)(control->ମୂଲ୍ୟ),
					strlen(control->ମୂଲ୍ୟ), &ମୂଲ୍ୟ);
		ଯଦି (exc != DOM_NO_ERR) {
			ଯାଅ out;
		}
		ଯଦି (control->ଗଣ୍ଠି_ମୂଲ୍ୟ != ଖାଲି) {
			dom_string_unref(control->ଗଣ୍ଠି_ମୂଲ୍ୟ);
		}
		control->ଗଣ୍ଠି_ମୂଲ୍ୟ = ମୂଲ୍ୟ;
		ମୂଲ୍ୟ = ଖାଲି;
		ଯଦି (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_ମୂଲ୍ୟ(control->ଗଣ୍ଠି, control->ଗଣ୍ଠି_ମୂଲ୍ୟ);
		} ଅନ୍ୟଥା {
			exc = dom_html_ନିବେଶ_element_set_ମୂଲ୍ୟ(control->ଗଣ୍ଠି, control->ଗଣ୍ଠି_ମୂଲ୍ୟ);
		}
		ଯଦି (exc != DOM_NO_ERR) {
			ଯାଅ out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ଯଦି (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_ମୂଲ୍ୟ(control->ଗଣ୍ଠି, &ମୂଲ୍ୟ);
	} ଅନ୍ୟଥା {
		exc = dom_html_ନିବେଶ_element_get_ମୂଲ୍ୟ(control->ଗଣ୍ଠି, &ମୂଲ୍ୟ);
	}

	ଯଦି (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		ଯାଅ out;
	}

	ଯଦି (!dom_string_isequal(control->ଗଣ୍ଠି_ମୂଲ୍ୟ, ମୂଲ୍ୟ)) {
		/* The DOM has changed */
		ଯଦି (!changed_dom) {
			/* And it wasn't us */
			ଅକ୍ଷର *ମୂଲ୍ୟ_s = strndup(
				dom_string_data(ମୂଲ୍ୟ),
				dom_string_byte_length(ମୂଲ୍ୟ));
			ଅକ୍ଷର *dup = ଖାଲି;
			ଯଦି (ମୂଲ୍ୟ_s == ଖାଲି) {
				ଯାଅ out;
			}
			dup = strdup(ମୂଲ୍ୟ_s);
			ଯଦି (dup == ଖାଲି) {
				ମୁକ୍ତ(ମୂଲ୍ୟ_s);
				ଯାଅ out;
			}
			ମୁକ୍ତ(control->ମୂଲ୍ୟ);
			control->ମୂଲ୍ୟ = ମୂଲ୍ୟ_s;
			ମୁକ୍ତ(control->last_synced_ମୂଲ୍ୟ);
			control->last_synced_ମୂଲ୍ୟ = dup;
			ଯଦି (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != ଖାଲି) {
				textarea_set_text(control->data.text.ta,
						  ମୂଲ୍ୟ_s);
			}
		}
		control->ଗଣ୍ଠି_ମୂଲ୍ୟ = ମୂଲ୍ୟ;
		ମୂଲ୍ୟ = ଖାଲି;
	}

out:
	ଯଦି (ମୂଲ୍ୟ != ଖାଲି)
		dom_string_unref(ମୂଲ୍ୟ);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ସଂରଚନା form *
form_new(ଶୂନ୍ୟ *ଗଣ୍ଠି,
	 ଅଚଳ ଅକ୍ଷର *action,
	 ଅଚଳ ଅକ୍ଷର *target,
	 form_method method,
	 ଅଚଳ ଅକ୍ଷର *charset,
	 ଅଚଳ ଅକ୍ଷର *doc_charset)
{
	ସଂରଚନା form *form;

	form = calloc(1, ଆକାର *form);
	ଯଦି (!form)
		ଫେରନ୍ତୁ ଖାଲି;

	form->action = strdup(action != ଖାଲି ? action : "");
	ଯଦି (form->action == ଖାଲି) {
		ମୁକ୍ତ(form);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	form->target = target != ଖାଲି ? strdup(target) : ଖାଲି;
	ଯଦି (target != ଖାଲି && form->target == ଖାଲି) {
		ମୁକ୍ତ(form->action);
		ମୁକ୍ତ(form);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	form->method = method;

	form->accept_charsets = charset != ଖାଲି ? strdup(charset) : ଖାଲି;
	ଯଦି (charset != ଖାଲି && form->accept_charsets == ଖାଲି) {
		ମୁକ୍ତ(form->target);
		ମୁକ୍ତ(form->action);
		ମୁକ୍ତ(form);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	form->document_charset = doc_charset != ଖାଲି ? strdup(doc_charset)
						     : ଖାଲି;
	ଯଦି (doc_charset && form->document_charset == ଖାଲି) {
		ମୁକ୍ତ(form->accept_charsets);
		ମୁକ୍ତ(form->target);
		ମୁକ୍ତ(form->action);
		ମୁକ୍ତ(form);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	form->ଗଣ୍ଠି = ଗଣ୍ଠି;

	ଫେରନ୍ତୁ form;
}


/* exported interface documented in html/form_internal.h */
ଶୂନ୍ୟ form_free(ସଂରଚନା form *form)
{
	ସଂରଚନା form_control *c, *d;

	ପାଇଁ (c = form->controls; c != ଖାଲି; c = d) {
		d = c->next;

		form_free_control(c);
	}

	ମୁକ୍ତ(form->action);
	ମୁକ୍ତ(form->target);
	ମୁକ୍ତ(form->accept_charsets);
	ମୁକ୍ତ(form->document_charset);

	ମୁକ୍ତ(form);
}


/* exported interface documented in html/form_internal.h */
ସଂରଚନା form_control *form_new_control(ଶୂନ୍ୟ *ଗଣ୍ଠି, form_control_type type)
{
	ସଂରଚନା form_control *control;

	control = calloc(1, ଆକାର *control);
	ଯଦି (control == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	control->ଗଣ୍ଠି = ଗଣ୍ଠି;
	control->type = type;

	ଫେରନ୍ତୁ control;
}
