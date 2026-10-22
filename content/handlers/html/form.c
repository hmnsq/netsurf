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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/url.h"
#અન્તર્ભાવ "utils/utf8.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/inttypes.h"
#અન્તર્ભાવ "netsurf/mouse.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "css/utils.h"
#અન્તર્ભાવ "desktop/knockout.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/textarea.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/layout.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/font.h"
#અન્તર્ભાવ "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_લીટી_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

સંરચના form_select_menu {
	પૂર્ણાંક લીટી_height;
	પૂર્ણાંક width, height;
	સંરચના scrollbar *scrollbar;
	પૂર્ણાંક f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	શૂન્ય *client_data;
	સંરચના content *c;
};

સ્થિર plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

સ્થિર plot_font_style_t plot_fstyle_entry = {
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
સ્થિર અક્ષર *
form_encode_item(અચલ અક્ષર *item,
		 uint32_t len,
		 અચલ અક્ષર *charset,
		 અચલ અક્ષર *fallback)
{
	nserror err;
	અક્ષર *ret = ખાલી;
	અક્ષર cset[256];

	જો (!item || !charset)
		પાછા ખાલી;

	snprintf(cset, કદ cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	જો (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, કદ cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		જો (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			જો (fallback) {
				snprintf(cset, કદ cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				જો (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, કદ cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			જો (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				જો (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	જો (err == NSERROR_NOMEM) {
		પાછા ખાલી;
	}

	પાછા ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_કિંમત_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
સ્થિર nserror
fetch_data_list_add_sname(અચલ અક્ષર *name,
			  અચલ અક્ષર *ksfx,
			  પૂર્ણાંક કિંમત,
			  સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	સંરચના fetch_multipart_data *fetch_data;
	પૂર્ણાંક keysize;

	fetch_data = calloc(1, કદ(*fetch_data));
	જો (fetch_data == ખાલી) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		પાછા NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	જો (fetch_data->name == ખાલી) {
		મુક્ત(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		પાછા NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*કિંમત*/
	fetch_data->કિંમત = malloc(FETCH_DATA_INT_કિંમત_SIZE);
	જો (fetch_data->કિંમત == ખાલી) {
		મુક્ત(fetch_data->name);
		મુક્ત(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		પાછા NSERROR_NOMEM;
	}
	snprintf(fetch_data->કિંમત, FETCH_DATA_INT_કિંમત_SIZE, "%d", કિંમત);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	પાછા NSERROR_OK;
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
સ્થિર nserror
fetch_data_list_add(dom_string *name,
		    dom_string *કિંમત,
		    અચલ અક્ષર *rawfile,
		    અચલ અક્ષર *form_charset,
		    અચલ અક્ષર *docu_charset,
		    સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	સંરચના fetch_multipart_data *fetch_data;

	પ્રતિજ્ઞા(name != ખાલી);

	fetch_data = calloc(1, કદ(*fetch_data));
	જો (fetch_data == ખાલી) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		પાછા NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	જો (fetch_data->name == ખાલી) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		મુક્ત(fetch_data);
		પાછા NSERROR_NOMEM;
	}

	જો (કિંમત == ખાલી) {
		fetch_data->કિંમત = strdup("");
	} નહીંતર {
		fetch_data->કિંમત = form_encode_item(dom_string_data(કિંમત),
						     dom_string_byte_length(કિંમત),
						     form_charset,
						     docu_charset);
	}
	જો (fetch_data->કિંમત == ખાલી) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		મુક્ત(fetch_data->name);
		મુક્ત(fetch_data);
		પાછા NSERROR_NOMEM;
	}

	/* deal with raw file name */
	જો (rawfile != ખાલી) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		જો (fetch_data->rawfile == ખાલી) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			મુક્ત(fetch_data->કિંમત);
			મુક્ત(fetch_data->name);
			મુક્ત(fetch_data);
			પાછા NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	પાછા NSERROR_OK;
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
સ્થિર nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  અચલ અક્ષર *form_charset,
			  અચલ અક્ષર *doc_charset,
			  સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (element_disabled) {
		/* allow enumeration to continue after disabled element */
		પાછા NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (inputname == ખાલી) {
		/* allow enumeration to continue after element with no name */
		પાછા NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_કિંમત(text_area_element,
						   &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		પાછા NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ખાલી,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      અચલ અક્ષર *form_charset,
			      અચલ અક્ષર *docu_charset,
			      સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *કિંમત;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		પાછા NSERROR_DOM;
	}

	જો (!selected) {
		/* unselected options do not add fetch data entries */
		પાછા NSERROR_OK;
	}

	exp = dom_html_option_element_get_કિંમત(option_element, &કિંમત);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		પાછા NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  કિંમત,
				  ખાલી,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(કિંમત);

	પાછા res;
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
સ્થિર nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			અચલ અક્ષર *form_charset,
			અચલ અક્ષર *doc_charset,
			સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ખાલી;
	uint32_t options_ગણતરી;
	uint32_t option_index;
	dom_ગાંઠ *option_element = ખાલી;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (element_disabled) {
		/* allow enumeration to continue after disabled element */
		પાછા NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (inputname == ખાલી) {
		/* allow enumeration to continue after element with no name */
		પાછા NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		પાછા NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_ગણતરી);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		પાછા NSERROR_DOM;
	}

	/* iterate over options collection */
	માટે (option_index = 0; option_index < options_ગણતરી; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		જો (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} નહીંતર {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_ગાંઠ_unref(option_element);
		}

		જો (res != NSERROR_OK) {
			તોડો;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_આગત_submit(dom_html_આગત_element *આગત_element,
			      dom_string *inputname,
			      અચલ અક્ષર *charset,
			      અચલ અક્ષર *document_charset,
			      dom_html_element **submit_button,
			      સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	જો (*submit_button == ખાલી) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)આગત_element;
	} નહીંતર જો (*submit_button != (dom_html_element *)આગત_element) {
		પાછા NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_આગત_element_get_કિંમત(આગત_element, &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		પાછા NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ખાલી,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_આગત_image(dom_html_આગત_element *આગત_element,
			     dom_string *inputname,
			     અચલ અક્ષર *charset,
			     અચલ અક્ષર *document_charset,
			     dom_html_element **submit_button,
			     સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	સંરચના image_આગત_coords *coords;
	અક્ષર *basename;

	/* Only use an image input if it was the thing which activated us */
	જો (*submit_button != (dom_html_element *)આગત_element) {
		પાછા NSERROR_OK;
	}

	exp = dom_ગાંઠ_get_user_data((dom_ગાંઠ *)આગત_element,
				     corestring_dom_ns_ચાવી_image_coords_ગાંઠ_data,
				     &coords);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		પાછા NSERROR_DOM;
	}

	જો (coords == ખાલી) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		પાછા NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	જો (basename == ખાલી) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		પાછા NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	જો (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	મુક્ત(basename);

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_આગત_checkbox(dom_html_આગત_element *આગત_element,
				dom_string *inputname,
				અચલ અક્ષર *charset,
				અચલ અક્ષર *document_charset,
				સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_આગત_element_get_checked(આગત_element, &checked);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		પાછા NSERROR_DOM;
	}

	જો (!checked) {
		/* unchecked items do not generate a data entry */
		પાછા NSERROR_OK;
	}

	exp = dom_html_આગત_element_get_કિંમત(આગત_element, &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		પાછા NSERROR_DOM;
	}

	/* ensure a default value */
	જો (inputvalue == ખાલી) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ખાલી,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_આગત_file(dom_html_આગત_element *આગત_element,
			    dom_string *inputname,
			    અચલ અક્ષર *charset,
			    અચલ અક્ષર *document_charset,
			    સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	અચલ અક્ષર *rawfile = ખાલી;

	exp = dom_html_આગત_element_get_કિંમત(આગત_element, &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		પાછા NSERROR_DOM;
	}

	exp = dom_ગાંઠ_get_user_data((dom_ગાંઠ *)આગત_element,
				     corestring_dom_ns_ચાવી_file_name_ગાંઠ_data,
				     &rawfile);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		પાછા NSERROR_DOM;
	}

	જો (rawfile == ખાલી) {
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

	પાછા res;
}


સ્થિર nserror
form_dom_to_data_આગત_text(dom_html_આગત_element *આગત_element,
			    dom_string *inputname,
			    અચલ અક્ષર *charset,
			    અચલ અક્ષર *document_charset,
			    સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_આગત_element_get_કિંમત(આગત_element, &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		પાછા NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ખાલી,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	પાછા res;
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
સ્થિર nserror
form_dom_to_data_આગત(dom_html_આગત_element *આગત_element,
		       અચલ અક્ષર *charset,
		       અચલ અક્ષર *document_charset,
		       dom_html_element **submit_button,
		       સંરચના fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_આગત_element_get_disabled(આગત_element,
						  &element_disabled);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (element_disabled) {
		/* disabled element requires no more processing */
		પાછા NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_આગત_element_get_name(આગત_element, &inputname);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (inputname == ખાલી) {
		/* element with no name is not converted */
		પાછા NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_આગત_element_get_type(આગત_element, &inputtype);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		પાછા NSERROR_DOM;
	}

	/* process according to input element type */
	જો (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_આગત_submit(આગત_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} નહીંતર જો (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_આગત_image(આગત_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} નહીંતર જો (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_આગત_checkbox(આગત_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} નહીંતર જો (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_આગત_file(આગત_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} નહીંતર જો (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} નહીંતર {
		/* Everything else is treated as text values */
		res = form_dom_to_data_આગત_text(આગત_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	પાછા res;
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
સ્થિર nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			અચલ અક્ષર *form_charset,
			અચલ અક્ષર *doc_charset,
			dom_html_element **submit_button,
			સંરચના fetch_multipart_data ***fetch_data_next_ptr)
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
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (element_disabled) {
		/* allow enumeration to continue after disabled element */
		પાછા NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		પાછા NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	જો (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		પાછા NSERROR_OK;
	}
	જો (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		પાછા NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	જો (*submit_button == ખાલી) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	જો (*submit_button != (dom_html_element *)button_element) {
		પાછા NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		પાછા NSERROR_DOM;
	}

	જો (inputname == ખાલી) {
		/* allow enumeration to continue after element with no name */
		પાછા NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_કિંમત(button_element, &inputvalue);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} નહીંતર {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  ખાલી,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	પાછા res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
સ્થિર અક્ષર *form_acceptable_charset(સંરચના form *form)
{
	અક્ષર *temp, *c;

	જો (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		જો (form->document_charset) {
			/* document charset present, so use it */
			પાછા strdup(form->document_charset);
		} નહીંતર {
			/* no document charset, so default to 8859-1 */
			પાછા strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	જો (!temp)
		પાછા ખાલી;

	/* make it upper case */
	માટે (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	જો (c) {
		મુક્ત(temp);
		પાછા strdup("UTF-8");
	}

	/* dispense with temporary copy */
	મુક્ત(temp);

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
	જ્યારે (*c && !ascii_is_space(*c)) {
		જો (*c == ',')
			તોડો;
		c++;
	}

	પાછા strndup(form->accept_charsets, c - form->accept_charsets);
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
સ્થિર nserror
form_dom_to_data(સંરચના form *form,
		 સંરચના form_control *submit_control,
		 સંરચના fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	અક્ષર *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = ખાલી; /* the dom form elements */
	uint32_t element_ગણતરી; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ગાંઠ *element = ખાલી; /* the DOM form element */
	dom_string *nodename = ખાલી; /* the DOM node name of the element */
	સંરચના fetch_multipart_data *fetch_data = ખાલી; /* fetch data list */
	સંરચના fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	જો (submit_control != ખાલી) {
		submit_button = submit_control->ગાંઠ;
	} નહીંતર {
		submit_button = ખાલી;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	જો (charset == ખાલી) {
		NSLOG(netsurf, INFO, "failed to find charset");
		પાછા NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->ગાંઠ, &elements);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		મુક્ત(charset);
		પાછા NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_ગણતરી);
	જો (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		જાઓ form_dom_to_data_error;
	}

	માટે (element_idx = 0; element_idx < element_ગણતરી; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		જો (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			જાઓ form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_ગાંઠ_get_ગાંઠ_name(element, &nodename);
		જો (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_ગાંઠ_unref(element);
			res = NSERROR_DOM;
			જાઓ form_dom_to_data_error;
		}

		જો (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} નહીંતર જો (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} નહીંતર જો (dom_string_isequal(nodename, corestring_dom_આગત)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_આગત(
				(dom_html_આગત_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} નહીંતર જો (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} નહીંતર {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (પૂર્ણાંક)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ગાંઠ_unref(element);

		/* abort form element enumeration on error */
		જો (res != NSERROR_OK) {
			જાઓ form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	મુક્ત(charset);

	પાછા NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	મુક્ત(charset);

	પાછા res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
સ્થિર nserror
form_url_encode(સંરચના form *form,
		સંરચના fetch_multipart_data *control,
		અક્ષર **encoded_out)
{
	અક્ષર *name, *કિંમત;
	અક્ષર *s, *s2;
	unsigned પૂર્ણાંક len, len1, len_init;
	nserror res;

	s = malloc(1);

	જો (s == ખાલી) {
		પાછા NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	માટે (; control; control = control->next) {
		res = url_escape(control->name, true, ખાલી, &name);
		જો (res != NSERROR_OK) {
			મુક્ત(s);
			પાછા res;
		}

		res = url_escape(control->કિંમત, true, ખાલી, &કિંમત);
		જો (res != NSERROR_OK) {
			મુક્ત(name);
			મુક્ત(s);
			પાછા res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(કિંમત) + 2;
		s2 = realloc(s, len1 + 1);
		જો (s2 == ખાલી) {
			મુક્ત(કિંમત);
			મુક્ત(name);
			મુક્ત(s);
			પાછા NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, કિંમત);
		len = len1;
		મુક્ત(name);
		મુક્ત(કિંમત);
	}

	જો (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	પાછા NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
સ્થિર શૂન્ય
form_select_menu_scroll_callback(શૂન્ય *client_data,
				 સંરચના scrollbar_સંદેશ_data *scrollbar_data)
{
	સંરચના form_control *control = client_data;
	સંરચના form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	પસંદગી (scrollbar_data->સંદેશ) {
		સ્થિતિ SCROLLBAR_સંદેશ_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			તોડો;
		સ્થિતિ SCROLLBAR_સંદેશ_SCROLL_START:
		{
			સંરચના rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			તોડો;
		સ્થિતિ SCROLLBAR_સંદેશ_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, ખાલી);
			તોડો;
		મૂળભૂત:
			તોડો;
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
સ્થિર nserror
form__select_process_selection(html_content *html,
			       સંરચના form_control *control,
			       પૂર્ણાંક item)
{
	સંરચના box *inline_box;
	સંરચના form_option *o;
	પૂર્ણાંક ગણતરી;
	nserror ret = NSERROR_OK;

	પ્રતિજ્ઞા(control != ખાલી);
	પ્રતિજ્ઞા(html != ખાલી);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	માટે (ગણતરી = 0, o = control->data.select.વસ્તુઓ;
			o != ખાલી;
			ગણતરી++, o = o->next) {
		જો (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->ગાંઠ, false);
		}

		જો (ગણતરી == item) {
			જો (control->data.select.multiple) {
				જો (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->ગાંઠ, false);
					control->data.select.અંક_selected--;
				} નહીંતર {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->ગાંઠ, true);
					control->data.select.અંક_selected++;
				}
			} નહીંતર {
				dom_html_option_element_set_selected(
						o->ગાંઠ, true);
				o->selected = true;
			}
		}

		જો (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	જો (control->data.select.અંક_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} નહીંતર જો (control->data.select.અંક_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} નહીંતર {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	જો (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} નહીંતર {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	પાછા ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
સ્થિર શૂન્ય form_select_menu_clicked(સંરચના form_control *control, પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના form_select_menu *menu = control->data.select.menu;
	સંરચના form_option *option;
	html_content *html = (html_content *)menu->c;
	પૂર્ણાંક લીટી_height, લીટી_height_with_spacing;
	પૂર્ણાંક item_bottom_y;
	પૂર્ણાંક scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	લીટી_height = menu->લીટી_height;
	લીટી_height_with_spacing = લીટી_height +
			લીટી_height * SELECT_લીટી_SPACING;

	option = control->data.select.વસ્તુઓ;
	item_bottom_y = લીટી_height_with_spacing;
	i = 0;
	જ્યારે (option && item_bottom_y < scroll + y) {
		item_bottom_y += લીટી_height_with_spacing;
		option = option->next;
		i++;
	}

	જો (option != ખાલી) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
શૂન્ય form_add_control(સંરચના form *form, સંરચના form_control *control)
{
	જો (form == ખાલી) {
		પાછા;
	}

	control->form = form;

	જો (form->controls != ખાલી) {
		પ્રતિજ્ઞા(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = ખાલી;
		form->last_control = control;
	} નહીંતર {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
શૂન્ય form_free_control(સંરચના form_control *control)
{
	સંરચના form_control *c;
	પ્રતિજ્ઞા(control != ખાલી);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->કિંમત, control->initial_કિંમત);
	મુક્ત(control->name);
	મુક્ત(control->કિંમત);
	મુક્ત(control->initial_કિંમત);
	જો (control->last_synced_કિંમત != ખાલી) {
		મુક્ત(control->last_synced_કિંમત);
	}

	જો (control->type == GADGET_SELECT) {
		સંરચના form_option *option, *next;

		માટે (option = control->data.select.વસ્તુઓ; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->કિંમત);
			મુક્ત(option->text);
			મુક્ત(option->કિંમત);
			મુક્ત(option);
		}
		જો (control->data.select.menu != ખાલી) {
			form_free_select_menu(control);
		}
	}

	જો (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		જો (control->data.text.initial != ખાલી) {
			dom_string_unref(control->data.text.initial);
		}

		જો (control->data.text.ta != ખાલી) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	જો (control->form != ખાલી) {
		માટે (c = control->form->controls; c != ખાલી; c = c->next) {
			જો (c->next == control) {
				c->next = control->next;
				જો (control->form->last_control == control)
					control->form->last_control = c;
				તોડો;
			}
			જો (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				જો (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = ખાલી;
				તોડો;
			}
		}
	}

	જો (control->ગાંઠ_કિંમત != ખાલી) {
		dom_string_unref(control->ગાંઠ_કિંમત);
	}

	મુક્ત(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(સંરચના form_control *control, અક્ષર *કિંમત, અક્ષર *text,
		     bool selected, શૂન્ય *ગાંઠ)
{
	સંરચના form_option *option;

	પ્રતિજ્ઞા(control);
	પ્રતિજ્ઞા(control->type == GADGET_SELECT);

	option = calloc(1, કદ *option);
	જો (!option)
		પાછા false;

	option->કિંમત = કિંમત;
	option->text = text;

	/* add to linked list */
	જો (control->data.select.વસ્તુઓ == 0)
		control->data.select.વસ્તુઓ = option;
	નહીંતર
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	જો (selected && (control->data.select.અંક_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.અંક_selected++;
		control->data.select.current = option;
	}

	control->data.select.અંક_વસ્તુઓ++;

	option->ગાંઠ = ગાંઠ;

	પાછા true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(શૂન્ય *client_data,
		      સંરચના form_control *control,
		      select_menu_redraw_callback callback,
		      સંરચના content *c)
{
	પૂર્ણાંક લીટી_height_with_spacing;
	સંરચના box *box;
	plot_font_style_t fstyle;
	પૂર્ણાંક total_height;
	સંરચના form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	જો (control->data.select.menu == ખાલી) {

		menu = calloc(1, કદ (સંરચના form_select_menu));
		જો (menu == ખાલી) {
			પાછા NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[જમણું_2].width + box->padding[જમણું_2] +
			box->border[ડાબું_2].width + box->padding[ડાબું_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->લીટી_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		લીટી_height_with_spacing = menu->લીટી_height +
				menu->લીટી_height *
				SELECT_લીટી_SPACING;

		total_height = control->data.select.અંક_વસ્તુઓ *
				લીટી_height_with_spacing;
		menu->height = total_height;

		જો (menu->height > MAX_SELECT_HEIGHT) {
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
		જો (res != NSERROR_OK) {
			control->data.select.menu = ખાલી;
			મુક્ત(menu);
			પાછા res;
		}
		menu->c = c;
	} નહીંતર {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	પાછા NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
શૂન્ય form_free_select_menu(સંરચના form_control *control)
{
	જો (control->data.select.menu->scrollbar != ખાલી)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	મુક્ત(control->data.select.menu);
	control->data.select.menu = ખાલી;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(સંરચના form_control *control,
			પૂર્ણાંક x, પૂર્ણાંક y,
			દશાંશ scale,
			અચલ સંરચના rect *clip,
			અચલ સંરચના redraw_context *ctx)
{
	સંરચના box *box;
	સંરચના form_select_menu *menu = control->data.select.menu;
	સંરચના form_option *option;
	પૂર્ણાંક લીટી_height, લીટી_height_with_spacing;
	પૂર્ણાંક width, height;
	પૂર્ણાંક x0, y0, x1, scrollbar_x, y1, y2, y3;
	પૂર્ણાંક item_y;
	પૂર્ણાંક text_pos_offset, text_x;
	પૂર્ણાંક scrollbar_width = SCROLLBAR_WIDTH;
	પૂર્ણાંક i;
	પૂર્ણાંક scroll;
	પૂર્ણાંક x_cp, y_cp;
	સંરચના rect r;
	સંરચના rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	લીટી_height = menu->લીટી_height;

	લીટી_height_with_spacing = લીટી_height +
			લીટી_height * SELECT_લીટી_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	જો (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / લીટી_height_with_spacing;
		scroll -= i * લીટી_height_with_spacing;
		લીટી_height *= scale;
		લીટી_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * લીટી_height_with_spacing;
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
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
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
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	option = control->data.select.વસ્તુઓ;
	item_y = લીટી_height_with_spacing;

	જ્યારે (item_y < scroll) {
		option = option->next;
		item_y += લીટી_height_with_spacing;
	}
	item_y -= લીટી_height_with_spacing;
	text_pos_offset = y - scroll +
			(પૂર્ણાંક) (લીટી_height * (0.75 + SELECT_લીટી_SPACING));
	text_x = x + (box->border[ડાબું_2].width + box->padding[ડાબું_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	જ્યારે (option && item_y - scroll < height) {

		જો (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + લીટી_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		જો (res != NSERROR_OK) {
			પાછા false;
		}

		item_y += લીટી_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	પાછા true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(સંરચના form_control *control,
			     દશાંશ scale,
			     અચલ સંરચના rect *clip)
{
	સંરચના form_select_menu *menu = control->data.select.menu;
	પૂર્ણાંક width, height;


	width = menu->width;
	height = menu->height;

	જો (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	જો (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		પાછા true;

	પાછા false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(સંરચના form_control *control, પૂર્ણાંક item)
{
	પ્રતિજ્ઞા(control != ખાલી);

	પાછા form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
સંરચના form_option *
form_select_get_option(સંરચના form_control *control, પૂર્ણાંક item)
{
	સંરચના form_option *opt;

	opt = control->data.select.વસ્તુઓ;
	જ્યારે ((opt != ખાલી) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	પાછા opt;
}


/* exported interface documented in netsurf/form.h */
અક્ષર *form_control_get_name(સંરચના form_control *control)
{
	પાછા control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(સંરચના form_control *control, સંરચના rect *r)
{
	box_bounds( control->box, r );
	પાછા NSERROR_OK;
}


/* private interface described in html/form_internal.h */
અચલ અક્ષર *
form_select_mouse_action(સંરચના form_control *control,
			 browser_mouse_state mouse,
			 પૂર્ણાંક x, પૂર્ણાંક y)
{
	સંરચના form_select_menu *menu = control->data.select.menu;
	પૂર્ણાંક x0, y0, x1, y1, scrollbar_x;
	અચલ અક્ષર *status = ખાલી;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	જો (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		પાછા scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	જો (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		જો (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		જો (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} નહીંતર જો (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	પાછા status;
}


/* private interface described in html/form_internal.h */
શૂન્ય
form_select_mouse_drag_end(સંરચના form_control *control,
			   browser_mouse_state mouse,
			   પૂર્ણાંક x, પૂર્ણાંક y)
{
	પૂર્ણાંક x0, y0, x1, y1;
	પૂર્ણાંક box_x, box_y;
	સંરચના box *box;
	સંરચના form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ડાબું_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[ટોચ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	જો (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		પાછા;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	જો (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
શૂન્ય form_select_get_dimensions(સંરચના form_control *control,
		પૂર્ણાંક *width, પૂર્ણાંક *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
શૂન્ય form_select_menu_callback(શૂન્ય *client_data,
		પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height)
{
	html_content *html = client_data;
	પૂર્ણાંક menu_x, menu_y;
	સંરચના box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ડાબું_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[ટોચ_2];
	content__request_redraw((સંરચના content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
શૂન્ય form_radio_set(સંરચના form_control *radio)
{
	સંરચના form_control *control;

	પ્રતિજ્ઞા(radio);
	જો (!radio->form)
		પાછા;

	જો (radio->selected)
		પાછા;

	/* Clear selected state for other controls in
	 * the same radio button group */
	માટે (control = radio->form->controls;
	     control != ખાલી;
	     control = control->next) {
		/* Only interested in radio inputs */
		જો (control->type != GADGET_RADIO)
			ચાલુ;

		/* Ignore ourself */
		જો (control == radio)
			ચાલુ;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		જો ((control->name == ખાલી) ||
		    (radio->name == ખાલી) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ચાલુ;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		જો (control->selected) {
			control->selected = false;
			dom_html_આગત_element_set_checked(control->ગાંઠ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_આગત_element_set_checked(radio->ગાંઠ, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    સંરચના browser_window *target,
	    સંરચના form *form,
	    સંરચના form_control *submit_button)
{
	nserror res;
	અક્ષર *data = ખાલી; /* encoded form data */
	સંરચના fetch_multipart_data *success = ખાલી; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	પ્રતિજ્ઞા(form != ખાલી);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	જો (res != NSERROR_OK) {
		પાછા res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	જો (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		પાછા res;
	}

	પસંદગી (form->method) {
	સ્થિતિ method_GET:
		res = form_url_encode(form, success, &data);
		જો (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			જો (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      ખાલી,
							      ખાલી,
							      ખાલી);

				nsurl_unref(query_url);
			}
			મુક્ત(data);
		}
		તોડો;

	સ્થિતિ method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		જો (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      ખાલી,
						      ખાલી);
			મુક્ત(data);
		}
		તોડો;

	સ્થિતિ method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      ખાલી,
					      success,
					      ખાલી);

		તોડો;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	પાછા res;
}


/* exported interface documented in html/form_internal.h */
શૂન્ય form_gadget_update_કિંમત(સંરચના form_control *control, અક્ષર *કિંમત)
{
	પસંદગી (control->type) {
	સ્થિતિ GADGET_HIDDEN:
	સ્થિતિ GADGET_TEXTBOX:
	સ્થિતિ GADGET_TEXTAREA:
	સ્થિતિ GADGET_PASSWORD:
	સ્થિતિ GADGET_FILE:
		જો (control->કિંમત != ખાલી) {
			મુક્ત(control->કિંમત);
		}
		control->કિંમત = કિંમત;
		જો (control->ગાંઠ != ખાલી) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)કિંમત,
						strlen(કિંમત), &str);
			જો (err == DOM_NO_ERR) {
				જો (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_કિંમત(
						(dom_html_text_area_element *)(control->ગાંઠ),
						str);
				નહીંતર
					err = dom_html_આગત_element_set_કિંમત(
						(dom_html_આગત_element *)(control->ગાંઠ),
						str);
				dom_string_unref(str);
			}
		}
		તોડો;
	મૂળભૂત:
		/* Do nothing */
		તોડો;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
શૂન્ય
form_gadget_sync_with_dom(સંરચના form_control *control)
{
	dom_exception exc;
	dom_string *કિંમત = ખાલી;
	bool changed_dom = false;

	જો (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		પાછા;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	જો ((control->last_synced_કિંમત == ખાલી &&
	     control->કિંમત != ખાલી &&
	     control->કિંમત[0] != '\0') ||
	    (control->last_synced_કિંમત != ખાલી &&
	     control->કિંમત != ખાલી &&
	     strcmp(control->કિંમત, control->last_synced_કિંમત) != 0)) {
		અક્ષર *dup = strdup(control->કિંમત);
		જો (dup == ખાલી) {
			જાઓ out;
		}
		જો (control->last_synced_કિંમત != ખાલી) {
			મુક્ત(control->last_synced_કિંમત);
		}
		control->last_synced_કિંમત = dup;
		exc = dom_string_create((uint8_t *)(control->કિંમત),
					strlen(control->કિંમત), &કિંમત);
		જો (exc != DOM_NO_ERR) {
			જાઓ out;
		}
		જો (control->ગાંઠ_કિંમત != ખાલી) {
			dom_string_unref(control->ગાંઠ_કિંમત);
		}
		control->ગાંઠ_કિંમત = કિંમત;
		કિંમત = ખાલી;
		જો (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_કિંમત(control->ગાંઠ, control->ગાંઠ_કિંમત);
		} નહીંતર {
			exc = dom_html_આગત_element_set_કિંમત(control->ગાંઠ, control->ગાંઠ_કિંમત);
		}
		જો (exc != DOM_NO_ERR) {
			જાઓ out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	જો (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_કિંમત(control->ગાંઠ, &કિંમત);
	} નહીંતર {
		exc = dom_html_આગત_element_get_કિંમત(control->ગાંઠ, &કિંમત);
	}

	જો (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		જાઓ out;
	}

	જો (!dom_string_isequal(control->ગાંઠ_કિંમત, કિંમત)) {
		/* The DOM has changed */
		જો (!changed_dom) {
			/* And it wasn't us */
			અક્ષર *કિંમત_s = strndup(
				dom_string_data(કિંમત),
				dom_string_byte_length(કિંમત));
			અક્ષર *dup = ખાલી;
			જો (કિંમત_s == ખાલી) {
				જાઓ out;
			}
			dup = strdup(કિંમત_s);
			જો (dup == ખાલી) {
				મુક્ત(કિંમત_s);
				જાઓ out;
			}
			મુક્ત(control->કિંમત);
			control->કિંમત = કિંમત_s;
			મુક્ત(control->last_synced_કિંમત);
			control->last_synced_કિંમત = dup;
			જો (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != ખાલી) {
				textarea_set_text(control->data.text.ta,
						  કિંમત_s);
			}
		}
		control->ગાંઠ_કિંમત = કિંમત;
		કિંમત = ખાલી;
	}

out:
	જો (કિંમત != ખાલી)
		dom_string_unref(કિંમત);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
સંરચના form *
form_new(શૂન્ય *ગાંઠ,
	 અચલ અક્ષર *action,
	 અચલ અક્ષર *target,
	 form_method method,
	 અચલ અક્ષર *charset,
	 અચલ અક્ષર *doc_charset)
{
	સંરચના form *form;

	form = calloc(1, કદ *form);
	જો (!form)
		પાછા ખાલી;

	form->action = strdup(action != ખાલી ? action : "");
	જો (form->action == ખાલી) {
		મુક્ત(form);
		પાછા ખાલી;
	}

	form->target = target != ખાલી ? strdup(target) : ખાલી;
	જો (target != ખાલી && form->target == ખાલી) {
		મુક્ત(form->action);
		મુક્ત(form);
		પાછા ખાલી;
	}

	form->method = method;

	form->accept_charsets = charset != ખાલી ? strdup(charset) : ખાલી;
	જો (charset != ખાલી && form->accept_charsets == ખાલી) {
		મુક્ત(form->target);
		મુક્ત(form->action);
		મુક્ત(form);
		પાછા ખાલી;
	}

	form->document_charset = doc_charset != ખાલી ? strdup(doc_charset)
						     : ખાલી;
	જો (doc_charset && form->document_charset == ખાલી) {
		મુક્ત(form->accept_charsets);
		મુક્ત(form->target);
		મુક્ત(form->action);
		મુક્ત(form);
		પાછા ખાલી;
	}

	form->ગાંઠ = ગાંઠ;

	પાછા form;
}


/* exported interface documented in html/form_internal.h */
શૂન્ય form_free(સંરચના form *form)
{
	સંરચના form_control *c, *d;

	માટે (c = form->controls; c != ખાલી; c = d) {
		d = c->next;

		form_free_control(c);
	}

	મુક્ત(form->action);
	મુક્ત(form->target);
	મુક્ત(form->accept_charsets);
	મુક્ત(form->document_charset);

	મુક્ત(form);
}


/* exported interface documented in html/form_internal.h */
સંરચના form_control *form_new_control(શૂન્ય *ગાંઠ, form_control_type type)
{
	સંરચના form_control *control;

	control = calloc(1, કદ *control);
	જો (control == ખાલી)
		પાછા ખાલી;

	control->ગાંઠ = ગાંઠ;
	control->type = type;

	પાછા control;
}
