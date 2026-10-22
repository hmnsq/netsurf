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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/url.h"
#അന്തര്ഭാവ "utils/utf8.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/inttypes.h"
#അന്തര്ഭാവ "netsurf/mouse.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "content/fetch.h"
#അന്തര്ഭാവ "content/hlcache.h"
#അന്തര്ഭാവ "css/utils.h"
#അന്തര്ഭാവ "desktop/knockout.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/textarea.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/layout.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_വരി_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

ഘടന form_select_menu {
	പൂർണ്ണസംഖ്യ വരി_height;
	പൂർണ്ണസംഖ്യ width, height;
	ഘടന scrollbar *scrollbar;
	പൂർണ്ണസംഖ്യ f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	ശൂന്യം *client_data;
	ഘടന content *c;
};

സ്ഥിതം plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

സ്ഥിതം plot_font_style_t plot_fstyle_entry = {
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
സ്ഥിതം അക്ഷരം *
form_encode_item(സ്ഥിരം അക്ഷരം *item,
		 uint32_t len,
		 സ്ഥിരം അക്ഷരം *charset,
		 സ്ഥിരം അക്ഷരം *fallback)
{
	nserror err;
	അക്ഷരം *ret = ശൂന്യ;
	അക്ഷരം cset[256];

	എങ്കിൽ (!item || !charset)
		തിരികെ ശൂന്യ;

	snprintf(cset, വലുപ്പം cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	എങ്കിൽ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, വലുപ്പം cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		എങ്കിൽ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			എങ്കിൽ (fallback) {
				snprintf(cset, വലുപ്പം cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				എങ്കിൽ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, വലുപ്പം cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			എങ്കിൽ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				എങ്കിൽ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	എങ്കിൽ (err == NSERROR_NOMEM) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_മൂല്യം_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
സ്ഥിതം nserror
fetch_data_list_add_sname(സ്ഥിരം അക്ഷരം *name,
			  സ്ഥിരം അക്ഷരം *ksfx,
			  പൂർണ്ണസംഖ്യ മൂല്യം,
			  ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	ഘടന fetch_multipart_data *fetch_data;
	പൂർണ്ണസംഖ്യ keysize;

	fetch_data = calloc(1, വലുപ്പം(*fetch_data));
	എങ്കിൽ (fetch_data == ശൂന്യ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		തിരികെ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	എങ്കിൽ (fetch_data->name == ശൂന്യ) {
		സ്വതന്ത്രം(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		തിരികെ NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*മൂല്യം*/
	fetch_data->മൂല്യം = malloc(FETCH_DATA_INT_മൂല്യം_SIZE);
	എങ്കിൽ (fetch_data->മൂല്യം == ശൂന്യ) {
		സ്വതന്ത്രം(fetch_data->name);
		സ്വതന്ത്രം(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		തിരികെ NSERROR_NOMEM;
	}
	snprintf(fetch_data->മൂല്യം, FETCH_DATA_INT_മൂല്യം_SIZE, "%d", മൂല്യം);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror
fetch_data_list_add(dom_string *name,
		    dom_string *മൂല്യം,
		    സ്ഥിരം അക്ഷരം *rawfile,
		    സ്ഥിരം അക്ഷരം *form_charset,
		    സ്ഥിരം അക്ഷരം *docu_charset,
		    ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	ഘടന fetch_multipart_data *fetch_data;

	പ്രതിജ്ഞാ(name != ശൂന്യ);

	fetch_data = calloc(1, വലുപ്പം(*fetch_data));
	എങ്കിൽ (fetch_data == ശൂന്യ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		തിരികെ NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	എങ്കിൽ (fetch_data->name == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		സ്വതന്ത്രം(fetch_data);
		തിരികെ NSERROR_NOMEM;
	}

	എങ്കിൽ (മൂല്യം == ശൂന്യ) {
		fetch_data->മൂല്യം = strdup("");
	} അല്ലെങ്കിൽ {
		fetch_data->മൂല്യം = form_encode_item(dom_string_data(മൂല്യം),
						     dom_string_byte_length(മൂല്യം),
						     form_charset,
						     docu_charset);
	}
	എങ്കിൽ (fetch_data->മൂല്യം == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		സ്വതന്ത്രം(fetch_data->name);
		സ്വതന്ത്രം(fetch_data);
		തിരികെ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	എങ്കിൽ (rawfile != ശൂന്യ) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		എങ്കിൽ (fetch_data->rawfile == ശൂന്യ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			സ്വതന്ത്രം(fetch_data->മൂല്യം);
			സ്വതന്ത്രം(fetch_data->name);
			സ്വതന്ത്രം(fetch_data);
			തിരികെ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	തിരികെ NSERROR_OK;
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
സ്ഥിതം nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  സ്ഥിരം അക്ഷരം *form_charset,
			  സ്ഥിരം അക്ഷരം *doc_charset,
			  ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		തിരികെ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (inputname == ശൂന്യ) {
		/* allow enumeration to continue after element with no name */
		തിരികെ NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_മൂല്യം(text_area_element,
						   &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		തിരികെ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ശൂന്യ,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      സ്ഥിരം അക്ഷരം *form_charset,
			      സ്ഥിരം അക്ഷരം *docu_charset,
			      ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *മൂല്യം;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (!selected) {
		/* unselected options do not add fetch data entries */
		തിരികെ NSERROR_OK;
	}

	exp = dom_html_option_element_get_മൂല്യം(option_element, &മൂല്യം);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		തിരികെ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  മൂല്യം,
				  ശൂന്യ,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(മൂല്യം);

	തിരികെ res;
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
സ്ഥിതം nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			സ്ഥിരം അക്ഷരം *form_charset,
			സ്ഥിരം അക്ഷരം *doc_charset,
			ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ശൂന്യ;
	uint32_t options_എണ്ണം;
	uint32_t option_index;
	dom_കണ്ണി *option_element = ശൂന്യ;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		തിരികെ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (inputname == ശൂന്യ) {
		/* allow enumeration to continue after element with no name */
		തിരികെ NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		തിരികെ NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_എണ്ണം);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		തിരികെ NSERROR_DOM;
	}

	/* iterate over options collection */
	വേണ്ടി (option_index = 0; option_index < options_എണ്ണം; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		എങ്കിൽ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} അല്ലെങ്കിൽ {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_കണ്ണി_unref(option_element);
		}

		എങ്കിൽ (res != NSERROR_OK) {
			നിർത്തുക;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_നിവേശം_submit(dom_html_നിവേശം_element *നിവേശം_element,
			      dom_string *inputname,
			      സ്ഥിരം അക്ഷരം *charset,
			      സ്ഥിരം അക്ഷരം *document_charset,
			      dom_html_element **submit_button,
			      ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	എങ്കിൽ (*submit_button == ശൂന്യ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)നിവേശം_element;
	} അല്ലെങ്കിൽ എങ്കിൽ (*submit_button != (dom_html_element *)നിവേശം_element) {
		തിരികെ NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_നിവേശം_element_get_മൂല്യം(നിവേശം_element, &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		തിരികെ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ശൂന്യ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_നിവേശം_image(dom_html_നിവേശം_element *നിവേശം_element,
			     dom_string *inputname,
			     സ്ഥിരം അക്ഷരം *charset,
			     സ്ഥിരം അക്ഷരം *document_charset,
			     dom_html_element **submit_button,
			     ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	ഘടന image_നിവേശം_coords *coords;
	അക്ഷരം *basename;

	/* Only use an image input if it was the thing which activated us */
	എങ്കിൽ (*submit_button != (dom_html_element *)നിവേശം_element) {
		തിരികെ NSERROR_OK;
	}

	exp = dom_കണ്ണി_get_user_data((dom_കണ്ണി *)നിവേശം_element,
				     corestring_dom_ns_താക്കോൽ_image_coords_കണ്ണി_data,
				     &coords);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (coords == ശൂന്യ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		തിരികെ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	എങ്കിൽ (basename == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		തിരികെ NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	എങ്കിൽ (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	സ്വതന്ത്രം(basename);

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_നിവേശം_checkbox(dom_html_നിവേശം_element *നിവേശം_element,
				dom_string *inputname,
				സ്ഥിരം അക്ഷരം *charset,
				സ്ഥിരം അക്ഷരം *document_charset,
				ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_നിവേശം_element_get_checked(നിവേശം_element, &checked);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (!checked) {
		/* unchecked items do not generate a data entry */
		തിരികെ NSERROR_OK;
	}

	exp = dom_html_നിവേശം_element_get_മൂല്യം(നിവേശം_element, &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		തിരികെ NSERROR_DOM;
	}

	/* ensure a default value */
	എങ്കിൽ (inputvalue == ശൂന്യ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ശൂന്യ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_നിവേശം_file(dom_html_നിവേശം_element *നിവേശം_element,
			    dom_string *inputname,
			    സ്ഥിരം അക്ഷരം *charset,
			    സ്ഥിരം അക്ഷരം *document_charset,
			    ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	സ്ഥിരം അക്ഷരം *rawfile = ശൂന്യ;

	exp = dom_html_നിവേശം_element_get_മൂല്യം(നിവേശം_element, &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		തിരികെ NSERROR_DOM;
	}

	exp = dom_കണ്ണി_get_user_data((dom_കണ്ണി *)നിവേശം_element,
				     corestring_dom_ns_താക്കോൽ_file_name_കണ്ണി_data,
				     &rawfile);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (rawfile == ശൂന്യ) {
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

	തിരികെ res;
}


സ്ഥിതം nserror
form_dom_to_data_നിവേശം_text(dom_html_നിവേശം_element *നിവേശം_element,
			    dom_string *inputname,
			    സ്ഥിരം അക്ഷരം *charset,
			    സ്ഥിരം അക്ഷരം *document_charset,
			    ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_നിവേശം_element_get_മൂല്യം(നിവേശം_element, &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		തിരികെ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  ശൂന്യ,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	തിരികെ res;
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
സ്ഥിതം nserror
form_dom_to_data_നിവേശം(dom_html_നിവേശം_element *നിവേശം_element,
		       സ്ഥിരം അക്ഷരം *charset,
		       സ്ഥിരം അക്ഷരം *document_charset,
		       dom_html_element **submit_button,
		       ഘടന fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_നിവേശം_element_get_disabled(നിവേശം_element,
						  &element_disabled);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (element_disabled) {
		/* disabled element requires no more processing */
		തിരികെ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_നിവേശം_element_get_name(നിവേശം_element, &inputname);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (inputname == ശൂന്യ) {
		/* element with no name is not converted */
		തിരികെ NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_നിവേശം_element_get_type(നിവേശം_element, &inputtype);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		തിരികെ NSERROR_DOM;
	}

	/* process according to input element type */
	എങ്കിൽ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_നിവേശം_submit(നിവേശം_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_നിവേശം_image(നിവേശം_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_നിവേശം_checkbox(നിവേശം_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_നിവേശം_file(നിവേശം_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} അല്ലെങ്കിൽ {
		/* Everything else is treated as text values */
		res = form_dom_to_data_നിവേശം_text(നിവേശം_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	തിരികെ res;
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
സ്ഥിതം nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			സ്ഥിരം അക്ഷരം *form_charset,
			സ്ഥിരം അക്ഷരം *doc_charset,
			dom_html_element **submit_button,
			ഘടന fetch_multipart_data ***fetch_data_next_ptr)
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
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		തിരികെ NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		തിരികെ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	എങ്കിൽ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		തിരികെ NSERROR_OK;
	}
	എങ്കിൽ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		തിരികെ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	എങ്കിൽ (*submit_button == ശൂന്യ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	എങ്കിൽ (*submit_button != (dom_html_element *)button_element) {
		തിരികെ NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		തിരികെ NSERROR_DOM;
	}

	എങ്കിൽ (inputname == ശൂന്യ) {
		/* allow enumeration to continue after element with no name */
		തിരികെ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_മൂല്യം(button_element, &inputvalue);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} അല്ലെങ്കിൽ {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  ശൂന്യ,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	തിരികെ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
സ്ഥിതം അക്ഷരം *form_acceptable_charset(ഘടന form *form)
{
	അക്ഷരം *temp, *c;

	എങ്കിൽ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		എങ്കിൽ (form->document_charset) {
			/* document charset present, so use it */
			തിരികെ strdup(form->document_charset);
		} അല്ലെങ്കിൽ {
			/* no document charset, so default to 8859-1 */
			തിരികെ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	എങ്കിൽ (!temp)
		തിരികെ ശൂന്യ;

	/* make it upper case */
	വേണ്ടി (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	എങ്കിൽ (c) {
		സ്വതന്ത്രം(temp);
		തിരികെ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	സ്വതന്ത്രം(temp);

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
	ആകുമ്പോൾ (*c && !ascii_is_space(*c)) {
		എങ്കിൽ (*c == ',')
			നിർത്തുക;
		c++;
	}

	തിരികെ strndup(form->accept_charsets, c - form->accept_charsets);
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
സ്ഥിതം nserror
form_dom_to_data(ഘടന form *form,
		 ഘടന form_control *submit_control,
		 ഘടന fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	അക്ഷരം *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = ശൂന്യ; /* the dom form elements */
	uint32_t element_എണ്ണം; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_കണ്ണി *element = ശൂന്യ; /* the DOM form element */
	dom_string *nodename = ശൂന്യ; /* the DOM node name of the element */
	ഘടന fetch_multipart_data *fetch_data = ശൂന്യ; /* fetch data list */
	ഘടന fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	എങ്കിൽ (submit_control != ശൂന്യ) {
		submit_button = submit_control->കണ്ണി;
	} അല്ലെങ്കിൽ {
		submit_button = ശൂന്യ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	എങ്കിൽ (charset == ശൂന്യ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		തിരികെ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->കണ്ണി, &elements);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		സ്വതന്ത്രം(charset);
		തിരികെ NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_എണ്ണം);
	എങ്കിൽ (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		പോകുക form_dom_to_data_error;
	}

	വേണ്ടി (element_idx = 0; element_idx < element_എണ്ണം; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		എങ്കിൽ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			പോകുക form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_കണ്ണി_get_കണ്ണി_name(element, &nodename);
		എങ്കിൽ (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_കണ്ണി_unref(element);
			res = NSERROR_DOM;
			പോകുക form_dom_to_data_error;
		}

		എങ്കിൽ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(nodename, corestring_dom_നിവേശം)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_നിവേശം(
				(dom_html_നിവേശം_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} അല്ലെങ്കിൽ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (പൂർണ്ണസംഖ്യ)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_കണ്ണി_unref(element);

		/* abort form element enumeration on error */
		എങ്കിൽ (res != NSERROR_OK) {
			പോകുക form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	സ്വതന്ത്രം(charset);

	തിരികെ NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	സ്വതന്ത്രം(charset);

	തിരികെ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
സ്ഥിതം nserror
form_url_encode(ഘടന form *form,
		ഘടന fetch_multipart_data *control,
		അക്ഷരം **encoded_out)
{
	അക്ഷരം *name, *മൂല്യം;
	അക്ഷരം *s, *s2;
	unsigned പൂർണ്ണസംഖ്യ len, len1, len_init;
	nserror res;

	s = malloc(1);

	എങ്കിൽ (s == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	വേണ്ടി (; control; control = control->next) {
		res = url_escape(control->name, true, ശൂന്യ, &name);
		എങ്കിൽ (res != NSERROR_OK) {
			സ്വതന്ത്രം(s);
			തിരികെ res;
		}

		res = url_escape(control->മൂല്യം, true, ശൂന്യ, &മൂല്യം);
		എങ്കിൽ (res != NSERROR_OK) {
			സ്വതന്ത്രം(name);
			സ്വതന്ത്രം(s);
			തിരികെ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(മൂല്യം) + 2;
		s2 = realloc(s, len1 + 1);
		എങ്കിൽ (s2 == ശൂന്യ) {
			സ്വതന്ത്രം(മൂല്യം);
			സ്വതന്ത്രം(name);
			സ്വതന്ത്രം(s);
			തിരികെ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, മൂല്യം);
		len = len1;
		സ്വതന്ത്രം(name);
		സ്വതന്ത്രം(മൂല്യം);
	}

	എങ്കിൽ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	തിരികെ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
സ്ഥിതം ശൂന്യം
form_select_menu_scroll_callback(ശൂന്യം *client_data,
				 ഘടന scrollbar_സന്ദേശം_data *scrollbar_data)
{
	ഘടന form_control *control = client_data;
	ഘടന form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	തിരഞ്ഞെടുപ്പ് (scrollbar_data->സന്ദേശം) {
		അവസ്ഥ SCROLLBAR_സന്ദേശം_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			നിർത്തുക;
		അവസ്ഥ SCROLLBAR_സന്ദേശം_SCROLL_START:
		{
			ഘടന rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			നിർത്തുക;
		അവസ്ഥ SCROLLBAR_സന്ദേശം_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, ശൂന്യ);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
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
സ്ഥിതം nserror
form__select_process_selection(html_content *html,
			       ഘടന form_control *control,
			       പൂർണ്ണസംഖ്യ item)
{
	ഘടന box *inline_box;
	ഘടന form_option *o;
	പൂർണ്ണസംഖ്യ എണ്ണം;
	nserror ret = NSERROR_OK;

	പ്രതിജ്ഞാ(control != ശൂന്യ);
	പ്രതിജ്ഞാ(html != ശൂന്യ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	വേണ്ടി (എണ്ണം = 0, o = control->data.select.ഇനങ്ങൾ;
			o != ശൂന്യ;
			എണ്ണം++, o = o->next) {
		എങ്കിൽ (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->കണ്ണി, false);
		}

		എങ്കിൽ (എണ്ണം == item) {
			എങ്കിൽ (control->data.select.multiple) {
				എങ്കിൽ (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->കണ്ണി, false);
					control->data.select.അക്കം_selected--;
				} അല്ലെങ്കിൽ {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->കണ്ണി, true);
					control->data.select.അക്കം_selected++;
				}
			} അല്ലെങ്കിൽ {
				dom_html_option_element_set_selected(
						o->കണ്ണി, true);
				o->selected = true;
			}
		}

		എങ്കിൽ (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	എങ്കിൽ (control->data.select.അക്കം_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} അല്ലെങ്കിൽ എങ്കിൽ (control->data.select.അക്കം_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} അല്ലെങ്കിൽ {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	എങ്കിൽ (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} അല്ലെങ്കിൽ {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	തിരികെ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
സ്ഥിതം ശൂന്യം form_select_menu_clicked(ഘടന form_control *control, പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന form_select_menu *menu = control->data.select.menu;
	ഘടന form_option *option;
	html_content *html = (html_content *)menu->c;
	പൂർണ്ണസംഖ്യ വരി_height, വരി_height_with_spacing;
	പൂർണ്ണസംഖ്യ item_bottom_y;
	പൂർണ്ണസംഖ്യ scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	വരി_height = menu->വരി_height;
	വരി_height_with_spacing = വരി_height +
			വരി_height * SELECT_വരി_SPACING;

	option = control->data.select.ഇനങ്ങൾ;
	item_bottom_y = വരി_height_with_spacing;
	i = 0;
	ആകുമ്പോൾ (option && item_bottom_y < scroll + y) {
		item_bottom_y += വരി_height_with_spacing;
		option = option->next;
		i++;
	}

	എങ്കിൽ (option != ശൂന്യ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
ശൂന്യം form_add_control(ഘടന form *form, ഘടന form_control *control)
{
	എങ്കിൽ (form == ശൂന്യ) {
		തിരികെ;
	}

	control->form = form;

	എങ്കിൽ (form->controls != ശൂന്യ) {
		പ്രതിജ്ഞാ(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = ശൂന്യ;
		form->last_control = control;
	} അല്ലെങ്കിൽ {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
ശൂന്യം form_free_control(ഘടന form_control *control)
{
	ഘടന form_control *c;
	പ്രതിജ്ഞാ(control != ശൂന്യ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->മൂല്യം, control->initial_മൂല്യം);
	സ്വതന്ത്രം(control->name);
	സ്വതന്ത്രം(control->മൂല്യം);
	സ്വതന്ത്രം(control->initial_മൂല്യം);
	എങ്കിൽ (control->last_synced_മൂല്യം != ശൂന്യ) {
		സ്വതന്ത്രം(control->last_synced_മൂല്യം);
	}

	എങ്കിൽ (control->type == GADGET_SELECT) {
		ഘടന form_option *option, *next;

		വേണ്ടി (option = control->data.select.ഇനങ്ങൾ; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->മൂല്യം);
			സ്വതന്ത്രം(option->text);
			സ്വതന്ത്രം(option->മൂല്യം);
			സ്വതന്ത്രം(option);
		}
		എങ്കിൽ (control->data.select.menu != ശൂന്യ) {
			form_free_select_menu(control);
		}
	}

	എങ്കിൽ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		എങ്കിൽ (control->data.text.initial != ശൂന്യ) {
			dom_string_unref(control->data.text.initial);
		}

		എങ്കിൽ (control->data.text.ta != ശൂന്യ) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	എങ്കിൽ (control->form != ശൂന്യ) {
		വേണ്ടി (c = control->form->controls; c != ശൂന്യ; c = c->next) {
			എങ്കിൽ (c->next == control) {
				c->next = control->next;
				എങ്കിൽ (control->form->last_control == control)
					control->form->last_control = c;
				നിർത്തുക;
			}
			എങ്കിൽ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				എങ്കിൽ (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = ശൂന്യ;
				നിർത്തുക;
			}
		}
	}

	എങ്കിൽ (control->കണ്ണി_മൂല്യം != ശൂന്യ) {
		dom_string_unref(control->കണ്ണി_മൂല്യം);
	}

	സ്വതന്ത്രം(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(ഘടന form_control *control, അക്ഷരം *മൂല്യം, അക്ഷരം *text,
		     bool selected, ശൂന്യം *കണ്ണി)
{
	ഘടന form_option *option;

	പ്രതിജ്ഞാ(control);
	പ്രതിജ്ഞാ(control->type == GADGET_SELECT);

	option = calloc(1, വലുപ്പം *option);
	എങ്കിൽ (!option)
		തിരികെ false;

	option->മൂല്യം = മൂല്യം;
	option->text = text;

	/* add to linked list */
	എങ്കിൽ (control->data.select.ഇനങ്ങൾ == 0)
		control->data.select.ഇനങ്ങൾ = option;
	അല്ലെങ്കിൽ
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	എങ്കിൽ (selected && (control->data.select.അക്കം_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.അക്കം_selected++;
		control->data.select.current = option;
	}

	control->data.select.അക്കം_ഇനങ്ങൾ++;

	option->കണ്ണി = കണ്ണി;

	തിരികെ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(ശൂന്യം *client_data,
		      ഘടന form_control *control,
		      select_menu_redraw_callback callback,
		      ഘടന content *c)
{
	പൂർണ്ണസംഖ്യ വരി_height_with_spacing;
	ഘടന box *box;
	plot_font_style_t fstyle;
	പൂർണ്ണസംഖ്യ total_height;
	ഘടന form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	എങ്കിൽ (control->data.select.menu == ശൂന്യ) {

		menu = calloc(1, വലുപ്പം (ഘടന form_select_menu));
		എങ്കിൽ (menu == ശൂന്യ) {
			തിരികെ NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[വലത്_2].width + box->padding[വലത്_2] +
			box->border[ഇടത്_2].width + box->padding[ഇടത്_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->വരി_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		വരി_height_with_spacing = menu->വരി_height +
				menu->വരി_height *
				SELECT_വരി_SPACING;

		total_height = control->data.select.അക്കം_ഇനങ്ങൾ *
				വരി_height_with_spacing;
		menu->height = total_height;

		എങ്കിൽ (menu->height > MAX_SELECT_HEIGHT) {
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
		എങ്കിൽ (res != NSERROR_OK) {
			control->data.select.menu = ശൂന്യ;
			സ്വതന്ത്രം(menu);
			തിരികെ res;
		}
		menu->c = c;
	} അല്ലെങ്കിൽ {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	തിരികെ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
ശൂന്യം form_free_select_menu(ഘടന form_control *control)
{
	എങ്കിൽ (control->data.select.menu->scrollbar != ശൂന്യ)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	സ്വതന്ത്രം(control->data.select.menu);
	control->data.select.menu = ശൂന്യ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ഘടന form_control *control,
			പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
			ദശാംശം scale,
			സ്ഥിരം ഘടന rect *clip,
			സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന box *box;
	ഘടന form_select_menu *menu = control->data.select.menu;
	ഘടന form_option *option;
	പൂർണ്ണസംഖ്യ വരി_height, വരി_height_with_spacing;
	പൂർണ്ണസംഖ്യ width, height;
	പൂർണ്ണസംഖ്യ x0, y0, x1, scrollbar_x, y1, y2, y3;
	പൂർണ്ണസംഖ്യ item_y;
	പൂർണ്ണസംഖ്യ text_pos_offset, text_x;
	പൂർണ്ണസംഖ്യ scrollbar_width = SCROLLBAR_WIDTH;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ scroll;
	പൂർണ്ണസംഖ്യ x_cp, y_cp;
	ഘടന rect r;
	ഘടന rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	വരി_height = menu->വരി_height;

	വരി_height_with_spacing = വരി_height +
			വരി_height * SELECT_വരി_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	എങ്കിൽ (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / വരി_height_with_spacing;
		scroll -= i * വരി_height_with_spacing;
		വരി_height *= scale;
		വരി_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * വരി_height_with_spacing;
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
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
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
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	option = control->data.select.ഇനങ്ങൾ;
	item_y = വരി_height_with_spacing;

	ആകുമ്പോൾ (item_y < scroll) {
		option = option->next;
		item_y += വരി_height_with_spacing;
	}
	item_y -= വരി_height_with_spacing;
	text_pos_offset = y - scroll +
			(പൂർണ്ണസംഖ്യ) (വരി_height * (0.75 + SELECT_വരി_SPACING));
	text_x = x + (box->border[ഇടത്_2].width + box->padding[ഇടത്_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	ആകുമ്പോൾ (option && item_y - scroll < height) {

		എങ്കിൽ (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + വരി_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}

		item_y += വരി_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	തിരികെ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ഘടന form_control *control,
			     ദശാംശം scale,
			     സ്ഥിരം ഘടന rect *clip)
{
	ഘടന form_select_menu *menu = control->data.select.menu;
	പൂർണ്ണസംഖ്യ width, height;


	width = menu->width;
	height = menu->height;

	എങ്കിൽ (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	എങ്കിൽ (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		തിരികെ true;

	തിരികെ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ഘടന form_control *control, പൂർണ്ണസംഖ്യ item)
{
	പ്രതിജ്ഞാ(control != ശൂന്യ);

	തിരികെ form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
ഘടന form_option *
form_select_get_option(ഘടന form_control *control, പൂർണ്ണസംഖ്യ item)
{
	ഘടന form_option *opt;

	opt = control->data.select.ഇനങ്ങൾ;
	ആകുമ്പോൾ ((opt != ശൂന്യ) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	തിരികെ opt;
}


/* exported interface documented in netsurf/form.h */
അക്ഷരം *form_control_get_name(ഘടന form_control *control)
{
	തിരികെ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ഘടന form_control *control, ഘടന rect *r)
{
	box_bounds( control->box, r );
	തിരികെ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
സ്ഥിരം അക്ഷരം *
form_select_mouse_action(ഘടന form_control *control,
			 browser_mouse_state mouse,
			 പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	ഘടന form_select_menu *menu = control->data.select.menu;
	പൂർണ്ണസംഖ്യ x0, y0, x1, y1, scrollbar_x;
	സ്ഥിരം അക്ഷരം *status = ശൂന്യ;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	എങ്കിൽ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		തിരികെ scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	എങ്കിൽ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		എങ്കിൽ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		എങ്കിൽ (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} അല്ലെങ്കിൽ എങ്കിൽ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	തിരികെ status;
}


/* private interface described in html/form_internal.h */
ശൂന്യം
form_select_mouse_drag_end(ഘടന form_control *control,
			   browser_mouse_state mouse,
			   പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y)
{
	പൂർണ്ണസംഖ്യ x0, y0, x1, y1;
	പൂർണ്ണസംഖ്യ box_x, box_y;
	ഘടന box *box;
	ഘടന form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ഇടത്_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[മുകൾ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	എങ്കിൽ (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		തിരികെ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	എങ്കിൽ (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
ശൂന്യം form_select_get_dimensions(ഘടന form_control *control,
		പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
ശൂന്യം form_select_menu_callback(ശൂന്യം *client_data,
		പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	html_content *html = client_data;
	പൂർണ്ണസംഖ്യ menu_x, menu_y;
	ഘടന box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ഇടത്_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[മുകൾ_2];
	content__request_redraw((ഘടന content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
ശൂന്യം form_radio_set(ഘടന form_control *radio)
{
	ഘടന form_control *control;

	പ്രതിജ്ഞാ(radio);
	എങ്കിൽ (!radio->form)
		തിരികെ;

	എങ്കിൽ (radio->selected)
		തിരികെ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	വേണ്ടി (control = radio->form->controls;
	     control != ശൂന്യ;
	     control = control->next) {
		/* Only interested in radio inputs */
		എങ്കിൽ (control->type != GADGET_RADIO)
			തുടരുക;

		/* Ignore ourself */
		എങ്കിൽ (control == radio)
			തുടരുക;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		എങ്കിൽ ((control->name == ശൂന്യ) ||
		    (radio->name == ശൂന്യ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			തുടരുക;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		എങ്കിൽ (control->selected) {
			control->selected = false;
			dom_html_നിവേശം_element_set_checked(control->കണ്ണി, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_നിവേശം_element_set_checked(radio->കണ്ണി, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    ഘടന browser_window *target,
	    ഘടന form *form,
	    ഘടന form_control *submit_button)
{
	nserror res;
	അക്ഷരം *data = ശൂന്യ; /* encoded form data */
	ഘടന fetch_multipart_data *success = ശൂന്യ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	പ്രതിജ്ഞാ(form != ശൂന്യ);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	എങ്കിൽ (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		തിരികെ res;
	}

	തിരഞ്ഞെടുപ്പ് (form->method) {
	അവസ്ഥ method_GET:
		res = form_url_encode(form, success, &data);
		എങ്കിൽ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			എങ്കിൽ (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      ശൂന്യ,
							      ശൂന്യ,
							      ശൂന്യ);

				nsurl_unref(query_url);
			}
			സ്വതന്ത്രം(data);
		}
		നിർത്തുക;

	അവസ്ഥ method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		എങ്കിൽ (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      ശൂന്യ,
						      ശൂന്യ);
			സ്വതന്ത്രം(data);
		}
		നിർത്തുക;

	അവസ്ഥ method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      ശൂന്യ,
					      success,
					      ശൂന്യ);

		നിർത്തുക;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	തിരികെ res;
}


/* exported interface documented in html/form_internal.h */
ശൂന്യം form_gadget_update_മൂല്യം(ഘടന form_control *control, അക്ഷരം *മൂല്യം)
{
	തിരഞ്ഞെടുപ്പ് (control->type) {
	അവസ്ഥ GADGET_HIDDEN:
	അവസ്ഥ GADGET_TEXTBOX:
	അവസ്ഥ GADGET_TEXTAREA:
	അവസ്ഥ GADGET_PASSWORD:
	അവസ്ഥ GADGET_FILE:
		എങ്കിൽ (control->മൂല്യം != ശൂന്യ) {
			സ്വതന്ത്രം(control->മൂല്യം);
		}
		control->മൂല്യം = മൂല്യം;
		എങ്കിൽ (control->കണ്ണി != ശൂന്യ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)മൂല്യം,
						strlen(മൂല്യം), &str);
			എങ്കിൽ (err == DOM_NO_ERR) {
				എങ്കിൽ (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_മൂല്യം(
						(dom_html_text_area_element *)(control->കണ്ണി),
						str);
				അല്ലെങ്കിൽ
					err = dom_html_നിവേശം_element_set_മൂല്യം(
						(dom_html_നിവേശം_element *)(control->കണ്ണി),
						str);
				dom_string_unref(str);
			}
		}
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		/* Do nothing */
		നിർത്തുക;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
ശൂന്യം
form_gadget_sync_with_dom(ഘടന form_control *control)
{
	dom_exception exc;
	dom_string *മൂല്യം = ശൂന്യ;
	bool changed_dom = false;

	എങ്കിൽ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		തിരികെ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	എങ്കിൽ ((control->last_synced_മൂല്യം == ശൂന്യ &&
	     control->മൂല്യം != ശൂന്യ &&
	     control->മൂല്യം[0] != '\0') ||
	    (control->last_synced_മൂല്യം != ശൂന്യ &&
	     control->മൂല്യം != ശൂന്യ &&
	     strcmp(control->മൂല്യം, control->last_synced_മൂല്യം) != 0)) {
		അക്ഷരം *dup = strdup(control->മൂല്യം);
		എങ്കിൽ (dup == ശൂന്യ) {
			പോകുക out;
		}
		എങ്കിൽ (control->last_synced_മൂല്യം != ശൂന്യ) {
			സ്വതന്ത്രം(control->last_synced_മൂല്യം);
		}
		control->last_synced_മൂല്യം = dup;
		exc = dom_string_create((uint8_t *)(control->മൂല്യം),
					strlen(control->മൂല്യം), &മൂല്യം);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			പോകുക out;
		}
		എങ്കിൽ (control->കണ്ണി_മൂല്യം != ശൂന്യ) {
			dom_string_unref(control->കണ്ണി_മൂല്യം);
		}
		control->കണ്ണി_മൂല്യം = മൂല്യം;
		മൂല്യം = ശൂന്യ;
		എങ്കിൽ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_മൂല്യം(control->കണ്ണി, control->കണ്ണി_മൂല്യം);
		} അല്ലെങ്കിൽ {
			exc = dom_html_നിവേശം_element_set_മൂല്യം(control->കണ്ണി, control->കണ്ണി_മൂല്യം);
		}
		എങ്കിൽ (exc != DOM_NO_ERR) {
			പോകുക out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	എങ്കിൽ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_മൂല്യം(control->കണ്ണി, &മൂല്യം);
	} അല്ലെങ്കിൽ {
		exc = dom_html_നിവേശം_element_get_മൂല്യം(control->കണ്ണി, &മൂല്യം);
	}

	എങ്കിൽ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		പോകുക out;
	}

	എങ്കിൽ (!dom_string_isequal(control->കണ്ണി_മൂല്യം, മൂല്യം)) {
		/* The DOM has changed */
		എങ്കിൽ (!changed_dom) {
			/* And it wasn't us */
			അക്ഷരം *മൂല്യം_s = strndup(
				dom_string_data(മൂല്യം),
				dom_string_byte_length(മൂല്യം));
			അക്ഷരം *dup = ശൂന്യ;
			എങ്കിൽ (മൂല്യം_s == ശൂന്യ) {
				പോകുക out;
			}
			dup = strdup(മൂല്യം_s);
			എങ്കിൽ (dup == ശൂന്യ) {
				സ്വതന്ത്രം(മൂല്യം_s);
				പോകുക out;
			}
			സ്വതന്ത്രം(control->മൂല്യം);
			control->മൂല്യം = മൂല്യം_s;
			സ്വതന്ത്രം(control->last_synced_മൂല്യം);
			control->last_synced_മൂല്യം = dup;
			എങ്കിൽ (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != ശൂന്യ) {
				textarea_set_text(control->data.text.ta,
						  മൂല്യം_s);
			}
		}
		control->കണ്ണി_മൂല്യം = മൂല്യം;
		മൂല്യം = ശൂന്യ;
	}

out:
	എങ്കിൽ (മൂല്യം != ശൂന്യ)
		dom_string_unref(മൂല്യം);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ഘടന form *
form_new(ശൂന്യം *കണ്ണി,
	 സ്ഥിരം അക്ഷരം *action,
	 സ്ഥിരം അക്ഷരം *target,
	 form_method method,
	 സ്ഥിരം അക്ഷരം *charset,
	 സ്ഥിരം അക്ഷരം *doc_charset)
{
	ഘടന form *form;

	form = calloc(1, വലുപ്പം *form);
	എങ്കിൽ (!form)
		തിരികെ ശൂന്യ;

	form->action = strdup(action != ശൂന്യ ? action : "");
	എങ്കിൽ (form->action == ശൂന്യ) {
		സ്വതന്ത്രം(form);
		തിരികെ ശൂന്യ;
	}

	form->target = target != ശൂന്യ ? strdup(target) : ശൂന്യ;
	എങ്കിൽ (target != ശൂന്യ && form->target == ശൂന്യ) {
		സ്വതന്ത്രം(form->action);
		സ്വതന്ത്രം(form);
		തിരികെ ശൂന്യ;
	}

	form->method = method;

	form->accept_charsets = charset != ശൂന്യ ? strdup(charset) : ശൂന്യ;
	എങ്കിൽ (charset != ശൂന്യ && form->accept_charsets == ശൂന്യ) {
		സ്വതന്ത്രം(form->target);
		സ്വതന്ത്രം(form->action);
		സ്വതന്ത്രം(form);
		തിരികെ ശൂന്യ;
	}

	form->document_charset = doc_charset != ശൂന്യ ? strdup(doc_charset)
						     : ശൂന്യ;
	എങ്കിൽ (doc_charset && form->document_charset == ശൂന്യ) {
		സ്വതന്ത്രം(form->accept_charsets);
		സ്വതന്ത്രം(form->target);
		സ്വതന്ത്രം(form->action);
		സ്വതന്ത്രം(form);
		തിരികെ ശൂന്യ;
	}

	form->കണ്ണി = കണ്ണി;

	തിരികെ form;
}


/* exported interface documented in html/form_internal.h */
ശൂന്യം form_free(ഘടന form *form)
{
	ഘടന form_control *c, *d;

	വേണ്ടി (c = form->controls; c != ശൂന്യ; c = d) {
		d = c->next;

		form_free_control(c);
	}

	സ്വതന്ത്രം(form->action);
	സ്വതന്ത്രം(form->target);
	സ്വതന്ത്രം(form->accept_charsets);
	സ്വതന്ത്രം(form->document_charset);

	സ്വതന്ത്രം(form);
}


/* exported interface documented in html/form_internal.h */
ഘടന form_control *form_new_control(ശൂന്യം *കണ്ണി, form_control_type type)
{
	ഘടന form_control *control;

	control = calloc(1, വലുപ്പം *control);
	എങ്കിൽ (control == ശൂന്യ)
		തിരികെ ശൂന്യ;

	control->കണ്ണി = കണ്ണി;
	control->type = type;

	തിരികെ control;
}
