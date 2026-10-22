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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/url.h"
#अन्तर्भाव "utils/utf8.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/mouse.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "css/utils.h"
#अन्तर्भाव "desktop/knockout.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/textarea.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/layout.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_पङ्क्तिः_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

संरचना form_select_menu {
	पूर्णाङ्क पङ्क्तिः_height;
	पूर्णाङ्क width, height;
	संरचना scrollbar *scrollbar;
	पूर्णाङ्क f_size;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	शून्य *client_data;
	संरचना content *c;
};

स्थिर plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

स्थिर plot_font_style_t plot_fstyle_entry = {
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
स्थिर वर्ण *
form_encode_item(ध्रुव वर्ण *item,
		 uint32_t len,
		 ध्रुव वर्ण *charset,
		 ध्रुव वर्ण *fallback)
{
	nserror err;
	वर्ण *ret = रिक्त;
	वर्ण cset[256];

	सचे (!item || !charset)
		पटिनिवत्त रिक्त;

	snprintf(cset, परिमाण cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	सचे (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, परिमाण cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		सचे (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			सचे (fallback) {
				snprintf(cset, परिमाण cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				सचे (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, परिमाण cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			सचे (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				सचे (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	सचे (err == NSERROR_NOMEM) {
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_मानम्_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
स्थिर nserror
fetch_data_list_add_sname(ध्रुव वर्ण *name,
			  ध्रुव वर्ण *ksfx,
			  पूर्णाङ्क मानम्,
			  संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	संरचना fetch_multipart_data *fetch_data;
	पूर्णाङ्क keysize;

	fetch_data = calloc(1, परिमाण(*fetch_data));
	सचे (fetch_data == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		पटिनिवत्त NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = malloc(keysize + 1); /* allow for null */
	सचे (fetch_data->name == रिक्त) {
		मुक्त(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		पटिनिवत्त NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*मानम्*/
	fetch_data->मानम् = malloc(FETCH_DATA_INT_मानम्_SIZE);
	सचे (fetch_data->मानम् == रिक्त) {
		मुक्त(fetch_data->name);
		मुक्त(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		पटिनिवत्त NSERROR_NOMEM;
	}
	snprintf(fetch_data->मानम्, FETCH_DATA_INT_मानम्_SIZE, "%d", मानम्);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	पटिनिवत्त NSERROR_OK;
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
स्थिर nserror
fetch_data_list_add(dom_string *name,
		    dom_string *मानम्,
		    ध्रुव वर्ण *rawfile,
		    ध्रुव वर्ण *form_charset,
		    ध्रुव वर्ण *docu_charset,
		    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	संरचना fetch_multipart_data *fetch_data;

	प्रतिज्ञा(name != रिक्त);

	fetch_data = calloc(1, परिमाण(*fetch_data));
	सचे (fetch_data == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		पटिनिवत्त NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	सचे (fetch_data->name == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		मुक्त(fetch_data);
		पटिनिवत्त NSERROR_NOMEM;
	}

	सचे (मानम् == रिक्त) {
		fetch_data->मानम् = strdup("");
	} अञ्ञथा {
		fetch_data->मानम् = form_encode_item(dom_string_data(मानम्),
						     dom_string_byte_length(मानम्),
						     form_charset,
						     docu_charset);
	}
	सचे (fetch_data->मानम् == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		मुक्त(fetch_data->name);
		मुक्त(fetch_data);
		पटिनिवत्त NSERROR_NOMEM;
	}

	/* deal with raw file name */
	सचे (rawfile != रिक्त) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		सचे (fetch_data->rawfile == रिक्त) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			मुक्त(fetch_data->मानम्);
			मुक्त(fetch_data->name);
			मुक्त(fetch_data);
			पटिनिवत्त NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	पटिनिवत्त NSERROR_OK;
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
स्थिर nserror
form_dom_to_data_textarea(dom_html_text_area_element *text_area_element,
			  ध्रुव वर्ण *form_charset,
			  ध्रुव वर्ण *doc_charset,
			  संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (element_disabled) {
		/* allow enumeration to continue after disabled element */
		पटिनिवत्त NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		पटिनिवत्त NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_मानम्(text_area_element,
						   &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		पटिनिवत्त NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  form_charset,
				  doc_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ध्रुव वर्ण *form_charset,
			      ध्रुव वर्ण *docu_charset,
			      संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *मानम्;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (!selected) {
		/* unselected options do not add fetch data entries */
		पटिनिवत्त NSERROR_OK;
	}

	exp = dom_html_option_element_get_मानम्(option_element, &मानम्);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		पटिनिवत्त NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  मानम्,
				  रिक्त,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(मानम्);

	पटिनिवत्त res;
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
स्थिर nserror
form_dom_to_data_select(dom_html_select_element *select_element,
			ध्रुव वर्ण *form_charset,
			ध्रुव वर्ण *doc_charset,
			संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = रिक्त;
	uint32_t options_गणनम्;
	uint32_t option_index;
	dom_ग्रन्थिः *option_element = रिक्त;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (element_disabled) {
		/* allow enumeration to continue after disabled element */
		पटिनिवत्त NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		पटिनिवत्त NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		पटिनिवत्त NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_गणनम्);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		पटिनिवत्त NSERROR_DOM;
	}

	/* iterate over options collection */
	अत्थाय (option_index = 0; option_index < options_गणनम्; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		सचे (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} अञ्ञथा {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_ग्रन्थिः_unref(option_element);
		}

		सचे (res != NSERROR_OK) {
			भङ्ग;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_निवेशः_submit(dom_html_निवेशः_element *निवेशः_element,
			      dom_string *inputname,
			      ध्रुव वर्ण *charset,
			      ध्रुव वर्ण *document_charset,
			      dom_html_element **submit_button,
			      संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	सचे (*submit_button == रिक्त) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)निवेशः_element;
	} अञ्ञथा सचे (*submit_button != (dom_html_element *)निवेशः_element) {
		पटिनिवत्त NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		पटिनिवत्त NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_निवेशः_image(dom_html_निवेशः_element *निवेशः_element,
			     dom_string *inputname,
			     ध्रुव वर्ण *charset,
			     ध्रुव वर्ण *document_charset,
			     dom_html_element **submit_button,
			     संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	संरचना image_निवेशः_coords *coords;
	वर्ण *basename;

	/* Only use an image input if it was the thing which activated us */
	सचे (*submit_button != (dom_html_element *)निवेशः_element) {
		पटिनिवत्त NSERROR_OK;
	}

	exp = dom_ग्रन्थिः_get_user_data((dom_ग्रन्थिः *)निवेशः_element,
				     corestring_dom_ns_कुञ्चिका_image_coords_ग्रन्थिः_data,
				     &coords);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (coords == रिक्त) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		पटिनिवत्त NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	सचे (basename == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		पटिनिवत्त NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	सचे (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	मुक्त(basename);

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_निवेशः_checkbox(dom_html_निवेशः_element *निवेशः_element,
				dom_string *inputname,
				ध्रुव वर्ण *charset,
				ध्रुव वर्ण *document_charset,
				संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_निवेशः_element_get_checked(निवेशः_element, &checked);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (!checked) {
		/* unchecked items do not generate a data entry */
		पटिनिवत्त NSERROR_OK;
	}

	exp = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		पटिनिवत्त NSERROR_DOM;
	}

	/* ensure a default value */
	सचे (inputvalue == रिक्त) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_निवेशः_file(dom_html_निवेशः_element *निवेशः_element,
			    dom_string *inputname,
			    ध्रुव वर्ण *charset,
			    ध्रुव वर्ण *document_charset,
			    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ध्रुव वर्ण *rawfile = रिक्त;

	exp = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		पटिनिवत्त NSERROR_DOM;
	}

	exp = dom_ग्रन्थिः_get_user_data((dom_ग्रन्थिः *)निवेशः_element,
				     corestring_dom_ns_कुञ्चिका_file_name_ग्रन्थिः_data,
				     &rawfile);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (rawfile == रिक्त) {
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

	पटिनिवत्त res;
}


स्थिर nserror
form_dom_to_data_निवेशः_text(dom_html_निवेशः_element *निवेशः_element,
			    dom_string *inputname,
			    ध्रुव वर्ण *charset,
			    ध्रुव वर्ण *document_charset,
			    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		पटिनिवत्त NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	पटिनिवत्त res;
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
स्थिर nserror
form_dom_to_data_निवेशः(dom_html_निवेशः_element *निवेशः_element,
		       ध्रुव वर्ण *charset,
		       ध्रुव वर्ण *document_charset,
		       dom_html_element **submit_button,
		       संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_निवेशः_element_get_disabled(निवेशः_element,
						  &element_disabled);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (element_disabled) {
		/* disabled element requires no more processing */
		पटिनिवत्त NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_निवेशः_element_get_name(निवेशः_element, &inputname);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (inputname == रिक्त) {
		/* element with no name is not converted */
		पटिनिवत्त NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_निवेशः_element_get_type(निवेशः_element, &inputtype);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		पटिनिवत्त NSERROR_DOM;
	}

	/* process according to input element type */
	सचे (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_निवेशः_submit(निवेशः_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} अञ्ञथा सचे (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_निवेशः_image(निवेशः_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} अञ्ञथा सचे (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_निवेशः_checkbox(निवेशः_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} अञ्ञथा सचे (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_निवेशः_file(निवेशः_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} अञ्ञथा सचे (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} अञ्ञथा {
		/* Everything else is treated as text values */
		res = form_dom_to_data_निवेशः_text(निवेशः_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	पटिनिवत्त res;
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
स्थिर nserror
form_dom_to_data_button(dom_html_button_element *button_element,
			ध्रुव वर्ण *form_charset,
			ध्रुव वर्ण *doc_charset,
			dom_html_element **submit_button,
			संरचना fetch_multipart_data ***fetch_data_next_ptr)
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
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (element_disabled) {
		/* allow enumeration to continue after disabled element */
		पटिनिवत्त NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		पटिनिवत्त NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	सचे (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		पटिनिवत्त NSERROR_OK;
	}
	सचे (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		पटिनिवत्त NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	सचे (*submit_button == रिक्त) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	सचे (*submit_button != (dom_html_element *)button_element) {
		पटिनिवत्त NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		पटिनिवत्त NSERROR_DOM;
	}

	सचे (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		पटिनिवत्त NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_मानम्(button_element, &inputvalue);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} अञ्ञथा {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  रिक्त,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	पटिनिवत्त res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
स्थिर वर्ण *form_acceptable_charset(संरचना form *form)
{
	वर्ण *temp, *c;

	सचे (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		सचे (form->document_charset) {
			/* document charset present, so use it */
			पटिनिवत्त strdup(form->document_charset);
		} अञ्ञथा {
			/* no document charset, so default to 8859-1 */
			पटिनिवत्त strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	सचे (!temp)
		पटिनिवत्त रिक्त;

	/* make it upper case */
	अत्थाय (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(temp, "UTF-8");
	सचे (c) {
		मुक्त(temp);
		पटिनिवत्त strdup("UTF-8");
	}

	/* dispense with temporary copy */
	मुक्त(temp);

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
	याव (*c && !ascii_is_space(*c)) {
		सचे (*c == ',')
			भङ्ग;
		c++;
	}

	पटिनिवत्त strndup(form->accept_charsets, c - form->accept_charsets);
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
स्थिर nserror
form_dom_to_data(संरचना form *form,
		 संरचना form_control *submit_control,
		 संरचना fetch_multipart_data **fetch_data_out)
{
	nserror res = NSERROR_OK;
	वर्ण *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = रिक्त; /* the dom form elements */
	uint32_t element_गणनम्; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ग्रन्थिः *element = रिक्त; /* the DOM form element */
	dom_string *nodename = रिक्त; /* the DOM node name of the element */
	संरचना fetch_multipart_data *fetch_data = रिक्त; /* fetch data list */
	संरचना fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	सचे (submit_control != रिक्त) {
		submit_button = submit_control->ग्रन्थिः;
	} अञ्ञथा {
		submit_button = रिक्त;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	सचे (charset == रिक्त) {
		NSLOG(netsurf, INFO, "failed to find charset");
		पटिनिवत्त NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->ग्रन्थिः, &elements);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		मुक्त(charset);
		पटिनिवत्त NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_गणनम्);
	सचे (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		गच्छ form_dom_to_data_error;
	}

	अत्थाय (element_idx = 0; element_idx < element_गणनम्; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		सचे (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			गच्छ form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_ग्रन्थिः_get_ग्रन्थिः_name(element, &nodename);
		सचे (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_ग्रन्थिः_unref(element);
			res = NSERROR_DOM;
			गच्छ form_dom_to_data_error;
		}

		सचे (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} अञ्ञथा सचे (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} अञ्ञथा सचे (dom_string_isequal(nodename, corestring_dom_निवेशः)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_निवेशः(
				(dom_html_निवेशः_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} अञ्ञथा सचे (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} अञ्ञथा {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (पूर्णाङ्क)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ग्रन्थिः_unref(element);

		/* abort form element enumeration on error */
		सचे (res != NSERROR_OK) {
			गच्छ form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	मुक्त(charset);

	पटिनिवत्त NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	मुक्त(charset);

	पटिनिवत्त res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
स्थिर nserror
form_url_encode(संरचना form *form,
		संरचना fetch_multipart_data *control,
		वर्ण **encoded_out)
{
	वर्ण *name, *मानम्;
	वर्ण *s, *s2;
	unsigned पूर्णाङ्क len, len1, len_init;
	nserror res;

	s = malloc(1);

	सचे (s == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	अत्थाय (; control; control = control->next) {
		res = url_escape(control->name, true, रिक्त, &name);
		सचे (res != NSERROR_OK) {
			मुक्त(s);
			पटिनिवत्त res;
		}

		res = url_escape(control->मानम्, true, रिक्त, &मानम्);
		सचे (res != NSERROR_OK) {
			मुक्त(name);
			मुक्त(s);
			पटिनिवत्त res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(मानम्) + 2;
		s2 = realloc(s, len1 + 1);
		सचे (s2 == रिक्त) {
			मुक्त(मानम्);
			मुक्त(name);
			मुक्त(s);
			पटिनिवत्त NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, मानम्);
		len = len1;
		मुक्त(name);
		मुक्त(मानम्);
	}

	सचे (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	पटिनिवत्त NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
स्थिर शून्य
form_select_menu_scroll_callback(शून्य *client_data,
				 संरचना scrollbar_सन्देशः_data *scrollbar_data)
{
	संरचना form_control *control = client_data;
	संरचना form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	विकल्प (scrollbar_data->सन्देशः) {
		पक्ष SCROLLBAR_सन्देशः_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			भङ्ग;
		पक्ष SCROLLBAR_सन्देशः_SCROLL_START:
		{
			संरचना rect rect = {
				.x0 = scrollbar_data->x0,
				.y0 = scrollbar_data->y0,
				.x1 = scrollbar_data->x1,
				.y1 = scrollbar_data->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			भङ्ग;
		पक्ष SCROLLBAR_सन्देशः_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, रिक्त);
			भङ्ग;
		स्वभाव:
			भङ्ग;
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
स्थिर nserror
form__select_process_selection(html_content *html,
			       संरचना form_control *control,
			       पूर्णाङ्क item)
{
	संरचना box *inline_box;
	संरचना form_option *o;
	पूर्णाङ्क गणनम्;
	nserror ret = NSERROR_OK;

	प्रतिज्ञा(control != रिक्त);
	प्रतिज्ञा(html != रिक्त);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	अत्थाय (गणनम् = 0, o = control->data.select.वस्तूनि;
			o != रिक्त;
			गणनम्++, o = o->next) {
		सचे (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->ग्रन्थिः, false);
		}

		सचे (गणनम् == item) {
			सचे (control->data.select.multiple) {
				सचे (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->ग्रन्थिः, false);
					control->data.select.अङ्कः_selected--;
				} अञ्ञथा {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->ग्रन्थिः, true);
					control->data.select.अङ्कः_selected++;
				}
			} अञ्ञथा {
				dom_html_option_element_set_selected(
						o->ग्रन्थिः, true);
				o->selected = true;
			}
		}

		सचे (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	सचे (control->data.select.अङ्कः_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} अञ्ञथा सचे (control->data.select.अङ्कः_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} अञ्ञथा {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	सचे (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} अञ्ञथा {
		inline_box->length = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	पटिनिवत्त ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
स्थिर शून्य form_select_menu_clicked(संरचना form_control *control, पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	संरचना form_option *option;
	html_content *html = (html_content *)menu->c;
	पूर्णाङ्क पङ्क्तिः_height, पङ्क्तिः_height_with_spacing;
	पूर्णाङ्क item_bottom_y;
	पूर्णाङ्क scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	पङ्क्तिः_height = menu->पङ्क्तिः_height;
	पङ्क्तिः_height_with_spacing = पङ्क्तिः_height +
			पङ्क्तिः_height * SELECT_पङ्क्तिः_SPACING;

	option = control->data.select.वस्तूनि;
	item_bottom_y = पङ्क्तिः_height_with_spacing;
	i = 0;
	याव (option && item_bottom_y < scroll + y) {
		item_bottom_y += पङ्क्तिः_height_with_spacing;
		option = option->next;
		i++;
	}

	सचे (option != रिक्त) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
शून्य form_add_control(संरचना form *form, संरचना form_control *control)
{
	सचे (form == रिक्त) {
		पटिनिवत्त;
	}

	control->form = form;

	सचे (form->controls != रिक्त) {
		प्रतिज्ञा(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = रिक्त;
		form->last_control = control;
	} अञ्ञथा {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
शून्य form_free_control(संरचना form_control *control)
{
	संरचना form_control *c;
	प्रतिज्ञा(control != रिक्त);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->मानम्, control->initial_मानम्);
	मुक्त(control->name);
	मुक्त(control->मानम्);
	मुक्त(control->initial_मानम्);
	सचे (control->last_synced_मानम् != रिक्त) {
		मुक्त(control->last_synced_मानम्);
	}

	सचे (control->type == GADGET_SELECT) {
		संरचना form_option *option, *next;

		अत्थाय (option = control->data.select.वस्तूनि; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->मानम्);
			मुक्त(option->text);
			मुक्त(option->मानम्);
			मुक्त(option);
		}
		सचे (control->data.select.menu != रिक्त) {
			form_free_select_menu(control);
		}
	}

	सचे (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		सचे (control->data.text.initial != रिक्त) {
			dom_string_unref(control->data.text.initial);
		}

		सचे (control->data.text.ta != रिक्त) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	सचे (control->form != रिक्त) {
		अत्थाय (c = control->form->controls; c != रिक्त; c = c->next) {
			सचे (c->next == control) {
				c->next = control->next;
				सचे (control->form->last_control == control)
					control->form->last_control = c;
				भङ्ग;
			}
			सचे (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				सचे (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = रिक्त;
				भङ्ग;
			}
		}
	}

	सचे (control->ग्रन्थिः_मानम् != रिक्त) {
		dom_string_unref(control->ग्रन्थिः_मानम्);
	}

	मुक्त(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(संरचना form_control *control, वर्ण *मानम्, वर्ण *text,
		     bool selected, शून्य *ग्रन्थिः)
{
	संरचना form_option *option;

	प्रतिज्ञा(control);
	प्रतिज्ञा(control->type == GADGET_SELECT);

	option = calloc(1, परिमाण *option);
	सचे (!option)
		पटिनिवत्त false;

	option->मानम् = मानम्;
	option->text = text;

	/* add to linked list */
	सचे (control->data.select.वस्तूनि == 0)
		control->data.select.वस्तूनि = option;
	अञ्ञथा
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	सचे (selected && (control->data.select.अङ्कः_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.अङ्कः_selected++;
		control->data.select.current = option;
	}

	control->data.select.अङ्कः_वस्तूनि++;

	option->ग्रन्थिः = ग्रन्थिः;

	पटिनिवत्त true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(शून्य *client_data,
		      संरचना form_control *control,
		      select_menu_redraw_callback callback,
		      संरचना content *c)
{
	पूर्णाङ्क पङ्क्तिः_height_with_spacing;
	संरचना box *box;
	plot_font_style_t fstyle;
	पूर्णाङ्क total_height;
	संरचना form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	सचे (control->data.select.menu == रिक्त) {

		menu = calloc(1, परिमाण (संरचना form_select_menu));
		सचे (menu == रिक्त) {
			पटिनिवत्त NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[दक्षिणः_2].width + box->padding[दक्षिणः_2] +
			box->border[वामः_2].width + box->padding[वामः_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->पङ्क्तिः_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		पङ्क्तिः_height_with_spacing = menu->पङ्क्तिः_height +
				menu->पङ्क्तिः_height *
				SELECT_पङ्क्तिः_SPACING;

		total_height = control->data.select.अङ्कः_वस्तूनि *
				पङ्क्तिः_height_with_spacing;
		menu->height = total_height;

		सचे (menu->height > MAX_SELECT_HEIGHT) {
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
		सचे (res != NSERROR_OK) {
			control->data.select.menu = रिक्त;
			मुक्त(menu);
			पटिनिवत्त res;
		}
		menu->c = c;
	} अञ्ञथा {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	पटिनिवत्त NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free_select_menu(संरचना form_control *control)
{
	सचे (control->data.select.menu->scrollbar != रिक्त)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	मुक्त(control->data.select.menu);
	control->data.select.menu = रिक्त;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(संरचना form_control *control,
			पूर्णाङ्क x, पूर्णाङ्क y,
			भिन्न scale,
			ध्रुव संरचना rect *clip,
			ध्रुव संरचना redraw_context *ctx)
{
	संरचना box *box;
	संरचना form_select_menu *menu = control->data.select.menu;
	संरचना form_option *option;
	पूर्णाङ्क पङ्क्तिः_height, पङ्क्तिः_height_with_spacing;
	पूर्णाङ्क width, height;
	पूर्णाङ्क x0, y0, x1, scrollbar_x, y1, y2, y3;
	पूर्णाङ्क item_y;
	पूर्णाङ्क text_pos_offset, text_x;
	पूर्णाङ्क scrollbar_width = SCROLLBAR_WIDTH;
	पूर्णाङ्क i;
	पूर्णाङ्क scroll;
	पूर्णाङ्क x_cp, y_cp;
	संरचना rect r;
	संरचना rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	पङ्क्तिः_height = menu->पङ्क्तिः_height;

	पङ्क्तिः_height_with_spacing = पङ्क्तिः_height +
			पङ्क्तिः_height * SELECT_पङ्क्तिः_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	सचे (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / पङ्क्तिः_height_with_spacing;
		scroll -= i * पङ्क्तिः_height_with_spacing;
		पङ्क्तिः_height *= scale;
		पङ्क्तिः_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * पङ्क्तिः_height_with_spacing;
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
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
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
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	option = control->data.select.वस्तूनि;
	item_y = पङ्क्तिः_height_with_spacing;

	याव (item_y < scroll) {
		option = option->next;
		item_y += पङ्क्तिः_height_with_spacing;
	}
	item_y -= पङ्क्तिः_height_with_spacing;
	text_pos_offset = y - scroll +
			(पूर्णाङ्क) (पङ्क्तिः_height * (0.75 + SELECT_पङ्क्तिः_SPACING));
	text_x = x + (box->border[वामः_2].width + box->padding[वामः_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	याव (option && item_y - scroll < height) {

		सचे (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + पङ्क्तिः_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}

		item_y += पङ्क्तिः_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	पटिनिवत्त true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(संरचना form_control *control,
			     भिन्न scale,
			     ध्रुव संरचना rect *clip)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	पूर्णाङ्क width, height;


	width = menu->width;
	height = menu->height;

	सचे (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	सचे (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		पटिनिवत्त true;

	पटिनिवत्त false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(संरचना form_control *control, पूर्णाङ्क item)
{
	प्रतिज्ञा(control != रिक्त);

	पटिनिवत्त form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
संरचना form_option *
form_select_get_option(संरचना form_control *control, पूर्णाङ्क item)
{
	संरचना form_option *opt;

	opt = control->data.select.वस्तूनि;
	याव ((opt != रिक्त) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	पटिनिवत्त opt;
}


/* exported interface documented in netsurf/form.h */
वर्ण *form_control_get_name(संरचना form_control *control)
{
	पटिनिवत्त control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(संरचना form_control *control, संरचना rect *r)
{
	box_bounds( control->box, r );
	पटिनिवत्त NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ध्रुव वर्ण *
form_select_mouse_action(संरचना form_control *control,
			 browser_mouse_state mouse,
			 पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	पूर्णाङ्क x0, y0, x1, y1, scrollbar_x;
	ध्रुव वर्ण *status = रिक्त;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	सचे (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		पटिनिवत्त scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	सचे (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		सचे (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		सचे (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} अञ्ञथा सचे (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	पटिनिवत्त status;
}


/* private interface described in html/form_internal.h */
शून्य
form_select_mouse_drag_end(संरचना form_control *control,
			   browser_mouse_state mouse,
			   पूर्णाङ्क x, पूर्णाङ्क y)
{
	पूर्णाङ्क x0, y0, x1, y1;
	पूर्णाङ्क box_x, box_y;
	संरचना box *box;
	संरचना form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[वामः_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[शिखरम्_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	सचे (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		पटिनिवत्त;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	सचे (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
शून्य form_select_get_dimensions(संरचना form_control *control,
		पूर्णाङ्क *width, पूर्णाङ्क *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
शून्य form_select_menu_callback(शून्य *client_data,
		पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height)
{
	html_content *html = client_data;
	पूर्णाङ्क menu_x, menu_y;
	संरचना box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[वामः_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[शिखरम्_2];
	content__request_redraw((संरचना content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
शून्य form_radio_set(संरचना form_control *radio)
{
	संरचना form_control *control;

	प्रतिज्ञा(radio);
	सचे (!radio->form)
		पटिनिवत्त;

	सचे (radio->selected)
		पटिनिवत्त;

	/* Clear selected state for other controls in
	 * the same radio button group */
	अत्थाय (control = radio->form->controls;
	     control != रिक्त;
	     control = control->next) {
		/* Only interested in radio inputs */
		सचे (control->type != GADGET_RADIO)
			अनुवर्त;

		/* Ignore ourself */
		सचे (control == radio)
			अनुवर्त;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		सचे ((control->name == रिक्त) ||
		    (radio->name == रिक्त) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			अनुवर्त;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		सचे (control->selected) {
			control->selected = false;
			dom_html_निवेशः_element_set_checked(control->ग्रन्थिः, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_निवेशः_element_set_checked(radio->ग्रन्थिः, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    संरचना browser_window *target,
	    संरचना form *form,
	    संरचना form_control *submit_button)
{
	nserror res;
	वर्ण *data = रिक्त; /* encoded form data */
	संरचना fetch_multipart_data *success = रिक्त; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	प्रतिज्ञा(form != रिक्त);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	सचे (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		पटिनिवत्त res;
	}

	विकल्प (form->method) {
	पक्ष method_GET:
		res = form_url_encode(form, success, &data);
		सचे (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			सचे (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      रिक्त,
							      रिक्त,
							      रिक्त);

				nsurl_unref(query_url);
			}
			मुक्त(data);
		}
		भङ्ग;

	पक्ष method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		सचे (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      रिक्त,
						      रिक्त);
			मुक्त(data);
		}
		भङ्ग;

	पक्ष method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      रिक्त,
					      success,
					      रिक्त);

		भङ्ग;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	पटिनिवत्त res;
}


/* exported interface documented in html/form_internal.h */
शून्य form_gadget_update_मानम्(संरचना form_control *control, वर्ण *मानम्)
{
	विकल्प (control->type) {
	पक्ष GADGET_HIDDEN:
	पक्ष GADGET_TEXTBOX:
	पक्ष GADGET_TEXTAREA:
	पक्ष GADGET_PASSWORD:
	पक्ष GADGET_FILE:
		सचे (control->मानम् != रिक्त) {
			मुक्त(control->मानम्);
		}
		control->मानम् = मानम्;
		सचे (control->ग्रन्थिः != रिक्त) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)मानम्,
						strlen(मानम्), &str);
			सचे (err == DOM_NO_ERR) {
				सचे (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_मानम्(
						(dom_html_text_area_element *)(control->ग्रन्थिः),
						str);
				अञ्ञथा
					err = dom_html_निवेशः_element_set_मानम्(
						(dom_html_निवेशः_element *)(control->ग्रन्थिः),
						str);
				dom_string_unref(str);
			}
		}
		भङ्ग;
	स्वभाव:
		/* Do nothing */
		भङ्ग;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
शून्य
form_gadget_sync_with_dom(संरचना form_control *control)
{
	dom_exception exc;
	dom_string *मानम् = रिक्त;
	bool changed_dom = false;

	सचे (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		पटिनिवत्त;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	सचे ((control->last_synced_मानम् == रिक्त &&
	     control->मानम् != रिक्त &&
	     control->मानम्[0] != '\0') ||
	    (control->last_synced_मानम् != रिक्त &&
	     control->मानम् != रिक्त &&
	     strcmp(control->मानम्, control->last_synced_मानम्) != 0)) {
		वर्ण *dup = strdup(control->मानम्);
		सचे (dup == रिक्त) {
			गच्छ out;
		}
		सचे (control->last_synced_मानम् != रिक्त) {
			मुक्त(control->last_synced_मानम्);
		}
		control->last_synced_मानम् = dup;
		exc = dom_string_create((uint8_t *)(control->मानम्),
					strlen(control->मानम्), &मानम्);
		सचे (exc != DOM_NO_ERR) {
			गच्छ out;
		}
		सचे (control->ग्रन्थिः_मानम् != रिक्त) {
			dom_string_unref(control->ग्रन्थिः_मानम्);
		}
		control->ग्रन्थिः_मानम् = मानम्;
		मानम् = रिक्त;
		सचे (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_मानम्(control->ग्रन्थिः, control->ग्रन्थिः_मानम्);
		} अञ्ञथा {
			exc = dom_html_निवेशः_element_set_मानम्(control->ग्रन्थिः, control->ग्रन्थिः_मानम्);
		}
		सचे (exc != DOM_NO_ERR) {
			गच्छ out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	सचे (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_मानम्(control->ग्रन्थिः, &मानम्);
	} अञ्ञथा {
		exc = dom_html_निवेशः_element_get_मानम्(control->ग्रन्थिः, &मानम्);
	}

	सचे (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		गच्छ out;
	}

	सचे (!dom_string_isequal(control->ग्रन्थिः_मानम्, मानम्)) {
		/* The DOM has changed */
		सचे (!changed_dom) {
			/* And it wasn't us */
			वर्ण *मानम्_s = strndup(
				dom_string_data(मानम्),
				dom_string_byte_length(मानम्));
			वर्ण *dup = रिक्त;
			सचे (मानम्_s == रिक्त) {
				गच्छ out;
			}
			dup = strdup(मानम्_s);
			सचे (dup == रिक्त) {
				मुक्त(मानम्_s);
				गच्छ out;
			}
			मुक्त(control->मानम्);
			control->मानम् = मानम्_s;
			मुक्त(control->last_synced_मानम्);
			control->last_synced_मानम् = dup;
			सचे (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != रिक्त) {
				textarea_set_text(control->data.text.ta,
						  मानम्_s);
			}
		}
		control->ग्रन्थिः_मानम् = मानम्;
		मानम् = रिक्त;
	}

out:
	सचे (मानम् != रिक्त)
		dom_string_unref(मानम्);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
संरचना form *
form_new(शून्य *ग्रन्थिः,
	 ध्रुव वर्ण *action,
	 ध्रुव वर्ण *target,
	 form_method method,
	 ध्रुव वर्ण *charset,
	 ध्रुव वर्ण *doc_charset)
{
	संरचना form *form;

	form = calloc(1, परिमाण *form);
	सचे (!form)
		पटिनिवत्त रिक्त;

	form->action = strdup(action != रिक्त ? action : "");
	सचे (form->action == रिक्त) {
		मुक्त(form);
		पटिनिवत्त रिक्त;
	}

	form->target = target != रिक्त ? strdup(target) : रिक्त;
	सचे (target != रिक्त && form->target == रिक्त) {
		मुक्त(form->action);
		मुक्त(form);
		पटिनिवत्त रिक्त;
	}

	form->method = method;

	form->accept_charsets = charset != रिक्त ? strdup(charset) : रिक्त;
	सचे (charset != रिक्त && form->accept_charsets == रिक्त) {
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		पटिनिवत्त रिक्त;
	}

	form->document_charset = doc_charset != रिक्त ? strdup(doc_charset)
						     : रिक्त;
	सचे (doc_charset && form->document_charset == रिक्त) {
		मुक्त(form->accept_charsets);
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		पटिनिवत्त रिक्त;
	}

	form->ग्रन्थिः = ग्रन्थिः;

	पटिनिवत्त form;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free(संरचना form *form)
{
	संरचना form_control *c, *d;

	अत्थाय (c = form->controls; c != रिक्त; c = d) {
		d = c->next;

		form_free_control(c);
	}

	मुक्त(form->action);
	मुक्त(form->target);
	मुक्त(form->accept_charsets);
	मुक्त(form->document_charset);

	मुक्त(form);
}


/* exported interface documented in html/form_internal.h */
संरचना form_control *form_new_control(शून्य *ग्रन्थिः, form_control_type type)
{
	संरचना form_control *control;

	control = calloc(1, परिमाण *control);
	सचे (control == रिक्त)
		पटिनिवत्त रिक्त;

	control->ग्रन्थिः = ग्रन्थिः;
	control->type = type;

	पटिनिवत्त control;
}
