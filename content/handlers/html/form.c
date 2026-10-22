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
#define SELECT_ओळ_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

संरचना form_select_menu {
	पूर्णांक ओळ_height;
	पूर्णांक width, height;
	संरचना scrollbar *scrollbar;
	पूर्णांक f_size;
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
स्थिर अक्षर *
form_encode_item(स्थिरांक अक्षर *item,
		 uint32_t len,
		 स्थिरांक अक्षर *charset,
		 स्थिरांक अक्षर *fallback)
{
	nserror err;
	अक्षर *ret = रिक्त;
	अक्षर cset[256];

	जर (!item || !charset)
		परत रिक्त;

	snprintf(cset, आकार cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	जर (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, आकार cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		जर (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			जर (fallback) {
				snprintf(cset, आकार cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				जर (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, आकार cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			जर (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				जर (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	जर (err == NSERROR_NOMEM) {
		परत रिक्त;
	}

	परत ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_DATA_INT_मूल्य_SIZE 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
स्थिर nserror
fetch_data_list_add_sname(स्थिरांक अक्षर *name,
			  स्थिरांक अक्षर *ksfx,
			  पूर्णांक मूल्य,
			  संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	संरचना fetch_multipart_data *fetch_data;
	पूर्णांक keysize;

	fetch_data = सुस्मृति(1, आकार(*fetch_data));
	जर (fetch_data == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		परत NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_data->name, 0, "%s%s", name, ksfx);
	fetch_data->name = दो_स्मृति(keysize + 1); /* allow for null */
	जर (fetch_data->name == रिक्त) {
		मुक्त(fetch_data);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		परत NSERROR_NOMEM;
	}
	snprintf(fetch_data->name, keysize + 1, "%s%s", name, ksfx);

	/*मूल्य*/
	fetch_data->मूल्य = दो_स्मृति(FETCH_DATA_INT_मूल्य_SIZE);
	जर (fetch_data->मूल्य == रिक्त) {
		मुक्त(fetch_data->name);
		मुक्त(fetch_data);
		NSLOG(netsurf, INFO, "value allocation failure");
		परत NSERROR_NOMEM;
	}
	snprintf(fetch_data->मूल्य, FETCH_DATA_INT_मूल्य_SIZE, "%d", मूल्य);

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	परत NSERROR_OK;
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
		    dom_string *मूल्य,
		    स्थिरांक अक्षर *rawfile,
		    स्थिरांक अक्षर *form_charset,
		    स्थिरांक अक्षर *docu_charset,
		    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	संरचना fetch_multipart_data *fetch_data;

	प्रतिज्ञा(name != रिक्त);

	fetch_data = सुस्मृति(1, आकार(*fetch_data));
	जर (fetch_data == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		परत NSERROR_NOMEM;
	}

	fetch_data->name = form_encode_item(dom_string_data(name),
					    dom_string_byte_length(name),
					    form_charset,
					    docu_charset);
	जर (fetch_data->name == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		मुक्त(fetch_data);
		परत NSERROR_NOMEM;
	}

	जर (मूल्य == रिक्त) {
		fetch_data->मूल्य = strdup("");
	} अन्यथा {
		fetch_data->मूल्य = form_encode_item(dom_string_data(मूल्य),
						     dom_string_byte_length(मूल्य),
						     form_charset,
						     docu_charset);
	}
	जर (fetch_data->मूल्य == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		मुक्त(fetch_data->name);
		मुक्त(fetch_data);
		परत NSERROR_NOMEM;
	}

	/* deal with raw file name */
	जर (rawfile != रिक्त) {
		fetch_data->file = true;
		fetch_data->rawfile = strdup(rawfile);
		जर (fetch_data->rawfile == रिक्त) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			मुक्त(fetch_data->मूल्य);
			मुक्त(fetch_data->name);
			मुक्त(fetch_data);
			परत NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_data_next_ptr = fetch_data;
	*fetch_data_next_ptr = &fetch_data->next;

	परत NSERROR_OK;
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
			  स्थिरांक अक्षर *form_charset,
			  स्थिरांक अक्षर *doc_charset,
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
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		परत NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		परत NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_मूल्य(text_area_element,
						   &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		परत NSERROR_DOM;
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

	परत res;
}


स्थिर nserror
form_dom_to_data_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      स्थिरांक अक्षर *form_charset,
			      स्थिरांक अक्षर *docu_charset,
			      संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *मूल्य;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		परत NSERROR_DOM;
	}

	जर (!selected) {
		/* unselected options do not add fetch data entries */
		परत NSERROR_OK;
	}

	exp = dom_html_option_element_get_मूल्य(option_element, &मूल्य);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		परत NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(keyname,
				  मूल्य,
				  रिक्त,
				  form_charset,
				  docu_charset,
				  fetch_data_next_ptr);

	dom_string_unref(मूल्य);

	परत res;
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
			स्थिरांक अक्षर *form_charset,
			स्थिरांक अक्षर *doc_charset,
			संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = रिक्त;
	uint32_t options_मोजणी;
	uint32_t option_index;
	dom_गाठ *option_element = रिक्त;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		परत NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		परत NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		परत NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_length(options, &options_मोजणी);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		परत NSERROR_DOM;
	}

	/* iterate over options collection */
	साठी (option_index = 0; option_index < options_मोजणी; ++option_index) {
		exp = dom_html_options_collection_item(options,
						       option_index,
						       &option_element);
		जर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_index);
			res = NSERROR_DOM;
		} अन्यथा {
			res = form_dom_to_data_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_data_next_ptr);

			dom_गाठ_unref(option_element);
		}

		जर (res != NSERROR_OK) {
			थांबा;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	परत res;
}


स्थिर nserror
form_dom_to_data_आदान_submit(dom_html_आदान_element *आदान_element,
			      dom_string *inputname,
			      स्थिरांक अक्षर *charset,
			      स्थिरांक अक्षर *document_charset,
			      dom_html_element **submit_button,
			      संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	जर (*submit_button == रिक्त) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)आदान_element;
	} अन्यथा जर (*submit_button != (dom_html_element *)आदान_element) {
		परत NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_आदान_element_get_मूल्य(आदान_element, &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		परत NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	परत res;
}


स्थिर nserror
form_dom_to_data_आदान_image(dom_html_आदान_element *आदान_element,
			     dom_string *inputname,
			     स्थिरांक अक्षर *charset,
			     स्थिरांक अक्षर *document_charset,
			     dom_html_element **submit_button,
			     संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	संरचना image_आदान_coords *coords;
	अक्षर *basename;

	/* Only use an image input if it was the thing which activated us */
	जर (*submit_button != (dom_html_element *)आदान_element) {
		परत NSERROR_OK;
	}

	exp = dom_गाठ_get_user_data((dom_गाठ *)आदान_element,
				     corestring_dom_ns_किल्ली_image_coords_गाठ_data,
				     &coords);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		परत NSERROR_DOM;
	}

	जर (coords == रिक्त) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		परत NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_data(inputname),
				    dom_string_byte_length(inputname),
				    charset,
				    document_charset);
	जर (basename == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		परत NSERROR_NOMEM;
	}

	res = fetch_data_list_add_sname(basename, ".x",
					coords->x,
					fetch_data_next_ptr);

	जर (res == NSERROR_OK) {
		res = fetch_data_list_add_sname(basename, ".y",
						coords->y,
						fetch_data_next_ptr);
	}

	मुक्त(basename);

	परत res;
}


स्थिर nserror
form_dom_to_data_आदान_checkbox(dom_html_आदान_element *आदान_element,
				dom_string *inputname,
				स्थिरांक अक्षर *charset,
				स्थिरांक अक्षर *document_charset,
				संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_आदान_element_get_checked(आदान_element, &checked);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		परत NSERROR_DOM;
	}

	जर (!checked) {
		/* unchecked items do not generate a data entry */
		परत NSERROR_OK;
	}

	exp = dom_html_आदान_element_get_मूल्य(आदान_element, &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		परत NSERROR_DOM;
	}

	/* ensure a default value */
	जर (inputvalue == रिक्त) {
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

	परत res;
}


स्थिर nserror
form_dom_to_data_आदान_file(dom_html_आदान_element *आदान_element,
			    dom_string *inputname,
			    स्थिरांक अक्षर *charset,
			    स्थिरांक अक्षर *document_charset,
			    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	स्थिरांक अक्षर *rawfile = रिक्त;

	exp = dom_html_आदान_element_get_मूल्य(आदान_element, &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		परत NSERROR_DOM;
	}

	exp = dom_गाठ_get_user_data((dom_गाठ *)आदान_element,
				     corestring_dom_ns_किल्ली_file_name_गाठ_data,
				     &rawfile);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		परत NSERROR_DOM;
	}

	जर (rawfile == रिक्त) {
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

	परत res;
}


स्थिर nserror
form_dom_to_data_आदान_text(dom_html_आदान_element *आदान_element,
			    dom_string *inputname,
			    स्थिरांक अक्षर *charset,
			    स्थिरांक अक्षर *document_charset,
			    संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_आदान_element_get_मूल्य(आदान_element, &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		परत NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_data_list_add(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_data_next_ptr);

	dom_string_unref(inputvalue);

	परत res;
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
form_dom_to_data_आदान(dom_html_आदान_element *आदान_element,
		       स्थिरांक अक्षर *charset,
		       स्थिरांक अक्षर *document_charset,
		       dom_html_element **submit_button,
		       संरचना fetch_multipart_data ***fetch_data_next_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_आदान_element_get_disabled(आदान_element,
						  &element_disabled);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (element_disabled) {
		/* disabled element requires no more processing */
		परत NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_आदान_element_get_name(आदान_element, &inputname);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (inputname == रिक्त) {
		/* element with no name is not converted */
		परत NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_आदान_element_get_type(आदान_element, &inputtype);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		परत NSERROR_DOM;
	}

	/* process according to input element type */
	जर (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_data_आदान_submit(आदान_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_data_next_ptr);

	} अन्यथा जर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_data_आदान_image(आदान_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_data_next_ptr);

	} अन्यथा जर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_data_आदान_checkbox(आदान_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_data_next_ptr);

	} अन्यथा जर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_data_आदान_file(आदान_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	} अन्यथा जर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} अन्यथा {
		/* Everything else is treated as text values */
		res = form_dom_to_data_आदान_text(आदान_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_data_next_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	परत res;
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
			स्थिरांक अक्षर *form_charset,
			स्थिरांक अक्षर *doc_charset,
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
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		परत NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		परत NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	जर (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		परत NSERROR_OK;
	}
	जर (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		परत NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	जर (*submit_button == रिक्त) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	जर (*submit_button != (dom_html_element *)button_element) {
		परत NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		परत NSERROR_DOM;
	}

	जर (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		परत NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_मूल्य(button_element, &inputvalue);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} अन्यथा {
		res = fetch_data_list_add(inputname,
					  inputvalue,
					  रिक्त,
					  form_charset,
					  doc_charset,
					  fetch_data_next_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	परत res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
स्थिर अक्षर *form_acceptable_charset(संरचना form *form)
{
	अक्षर *temp, *c;

	जर (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		जर (form->document_charset) {
			/* document charset present, so use it */
			परत strdup(form->document_charset);
		} अन्यथा {
			/* no document charset, so default to 8859-1 */
			परत strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	temp = strdup(form->accept_charsets);
	जर (!temp)
		परत रिक्त;

	/* make it upper case */
	साठी (c = temp; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = म_माला(temp, "UTF-8");
	जर (c) {
		मुक्त(temp);
		परत strdup("UTF-8");
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
	पर्यंत (*c && !ascii_is_space(*c)) {
		जर (*c == ',')
			थांबा;
		c++;
	}

	परत strndup(form->accept_charsets, c - form->accept_charsets);
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
	अक्षर *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = रिक्त; /* the dom form elements */
	uint32_t element_मोजणी; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_गाठ *element = रिक्त; /* the DOM form element */
	dom_string *nodename = रिक्त; /* the DOM node name of the element */
	संरचना fetch_multipart_data *fetch_data = रिक्त; /* fetch data list */
	संरचना fetch_multipart_data **fetch_data_next = &fetch_data;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	जर (submit_control != रिक्त) {
		submit_button = submit_control->गाठ;
	} अन्यथा {
		submit_button = रिक्त;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	जर (charset == रिक्त) {
		NSLOG(netsurf, INFO, "failed to find charset");
		परत NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->गाठ, &elements);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		मुक्त(charset);
		परत NSERROR_DOM;
	}

	exp = dom_html_collection_get_length(elements, &element_मोजणी);
	जर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		जा form_dom_to_data_error;
	}

	साठी (element_idx = 0; element_idx < element_मोजणी; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		जर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			जा form_dom_to_data_error;
		}

		/* node name from element */
		exp = dom_गाठ_get_गाठ_name(element, &nodename);
		जर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_गाठ_unref(element);
			res = NSERROR_DOM;
			जा form_dom_to_data_error;
		}

		जर (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_data_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} अन्यथा जर (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_data_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_data_next);

		} अन्यथा जर (dom_string_isequal(nodename, corestring_dom_आदान)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_data_आदान(
				(dom_html_आदान_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} अन्यथा जर (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_data_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_data_next);

		} अन्यथा {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (पूर्णांक)dom_string_byte_length(nodename),
			      dom_string_data(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_गाठ_unref(element);

		/* abort form element enumeration on error */
		जर (res != NSERROR_OK) {
			जा form_dom_to_data_error;
		}
	}

	*fetch_data_out = fetch_data;
	dom_html_collection_unref(elements);
	मुक्त(charset);

	परत NSERROR_OK;

form_dom_to_data_error:
	fetch_multipart_data_destroy(fetch_data);
	dom_html_collection_unref(elements);
	मुक्त(charset);

	परत res;
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
		अक्षर **encoded_out)
{
	अक्षर *name, *मूल्य;
	अक्षर *s, *s2;
	अचिन्हित पूर्णांक len, len1, len_init;
	nserror res;

	s = दो_स्मृति(1);

	जर (s == रिक्त) {
		परत NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	साठी (; control; control = control->next) {
		res = url_escape(control->name, true, रिक्त, &name);
		जर (res != NSERROR_OK) {
			मुक्त(s);
			परत res;
		}

		res = url_escape(control->मूल्य, true, रिक्त, &मूल्य);
		जर (res != NSERROR_OK) {
			मुक्त(name);
			मुक्त(s);
			परत res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + म_माप(name) + म_माप(मूल्य) + 2;
		s2 = पुनः_स्मृति(s, len1 + 1);
		जर (s2 == रिक्त) {
			मुक्त(मूल्य);
			मुक्त(name);
			मुक्त(s);
			परत NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, मूल्य);
		len = len1;
		मुक्त(name);
		मुक्त(मूल्य);
	}

	जर (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	परत NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
स्थिर शून्य
form_select_menu_scroll_callback(शून्य *client_data,
				 संरचना scrollbar_संदेश_data *scrollbar_data)
{
	संरचना form_control *control = client_data;
	संरचना form_select_menu *menu = control->data.select.menu;
	html_content *html = (html_content *)menu->c;

	निवड (scrollbar_data->संदेश) {
		स्थिती SCROLLBAR_संदेश_MOVED:
			menu->callback(menu->client_data,
					0, 0,
					menu->width,
					menu->height);
			थांबा;
		स्थिती SCROLLBAR_संदेश_SCROLL_START:
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
			थांबा;
		स्थिती SCROLLBAR_संदेश_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, रिक्त);
			थांबा;
		पूर्वनिर्धारित:
			थांबा;
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
			       पूर्णांक item)
{
	संरचना box *inline_box;
	संरचना form_option *o;
	पूर्णांक मोजणी;
	nserror ret = NSERROR_OK;

	प्रतिज्ञा(control != रिक्त);
	प्रतिज्ञा(html != रिक्त);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	साठी (मोजणी = 0, o = control->data.select.वस्तुसंच;
			o != रिक्त;
			मोजणी++, o = o->next) {
		जर (!control->data.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->गाठ, false);
		}

		जर (मोजणी == item) {
			जर (control->data.select.multiple) {
				जर (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->गाठ, false);
					control->data.select.अंक_selected--;
				} अन्यथा {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->गाठ, true);
					control->data.select.अंक_selected++;
				}
			} अन्यथा {
				dom_html_option_element_set_selected(
						o->गाठ, true);
				o->selected = true;
			}
		}

		जर (o->selected) {
			control->data.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	जर (control->data.select.अंक_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} अन्यथा जर (control->data.select.अंक_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->data.select.current->text);
	} अन्यथा {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	जर (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->length = 0;
	} अन्यथा {
		inline_box->length = म_माप(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	परत ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
स्थिर शून्य form_select_menu_clicked(संरचना form_control *control, पूर्णांक x, पूर्णांक y)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	संरचना form_option *option;
	html_content *html = (html_content *)menu->c;
	पूर्णांक ओळ_height, ओळ_height_with_spacing;
	पूर्णांक item_bottom_y;
	पूर्णांक scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	ओळ_height = menu->ओळ_height;
	ओळ_height_with_spacing = ओळ_height +
			ओळ_height * SELECT_ओळ_SPACING;

	option = control->data.select.वस्तुसंच;
	item_bottom_y = ओळ_height_with_spacing;
	i = 0;
	पर्यंत (option && item_bottom_y < scroll + y) {
		item_bottom_y += ओळ_height_with_spacing;
		option = option->next;
		i++;
	}

	जर (option != रिक्त) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_data, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
शून्य form_add_control(संरचना form *form, संरचना form_control *control)
{
	जर (form == रिक्त) {
		परत;
	}

	control->form = form;

	जर (form->controls != रिक्त) {
		प्रतिज्ञा(form->last_control);

		form->last_control->next = control;
		control->prev = form->last_control;
		control->next = रिक्त;
		form->last_control = control;
	} अन्यथा {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
शून्य form_free_control(संरचना form_control *control)
{
	संरचना form_control *c;
	प्रतिज्ञा(control != रिक्त);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->मूल्य, control->initial_मूल्य);
	मुक्त(control->name);
	मुक्त(control->मूल्य);
	मुक्त(control->initial_मूल्य);
	जर (control->last_synced_मूल्य != रिक्त) {
		मुक्त(control->last_synced_मूल्य);
	}

	जर (control->type == GADGET_SELECT) {
		संरचना form_option *option, *next;

		साठी (option = control->data.select.वस्तुसंच; option;
				option = next) {
			next = option->next;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->मूल्य);
			मुक्त(option->text);
			मुक्त(option->मूल्य);
			मुक्त(option);
		}
		जर (control->data.select.menu != रिक्त) {
			form_free_select_menu(control);
		}
	}

	जर (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		जर (control->data.text.initial != रिक्त) {
			dom_string_unref(control->data.text.initial);
		}

		जर (control->data.text.ta != रिक्त) {
			textarea_destroy(control->data.text.ta);
		}
	}

	/* unlink the control from the form */
	जर (control->form != रिक्त) {
		साठी (c = control->form->controls; c != रिक्त; c = c->next) {
			जर (c->next == control) {
				c->next = control->next;
				जर (control->form->last_control == control)
					control->form->last_control = c;
				थांबा;
			}
			जर (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->next;
				जर (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = रिक्त;
				थांबा;
			}
		}
	}

	जर (control->गाठ_मूल्य != रिक्त) {
		dom_string_unref(control->गाठ_मूल्य);
	}

	मुक्त(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(संरचना form_control *control, अक्षर *मूल्य, अक्षर *text,
		     bool selected, शून्य *गाठ)
{
	संरचना form_option *option;

	प्रतिज्ञा(control);
	प्रतिज्ञा(control->type == GADGET_SELECT);

	option = सुस्मृति(1, आकार *option);
	जर (!option)
		परत false;

	option->मूल्य = मूल्य;
	option->text = text;

	/* add to linked list */
	जर (control->data.select.वस्तुसंच == 0)
		control->data.select.वस्तुसंच = option;
	अन्यथा
		control->data.select.last_item->next = option;
	control->data.select.last_item = option;

	/* set selected */
	जर (selected && (control->data.select.अंक_selected == 0 ||
			control->data.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->data.select.अंक_selected++;
		control->data.select.current = option;
	}

	control->data.select.अंक_वस्तुसंच++;

	option->गाठ = गाठ;

	परत true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(शून्य *client_data,
		      संरचना form_control *control,
		      select_menu_redraw_callback callback,
		      संरचना content *c)
{
	पूर्णांक ओळ_height_with_spacing;
	संरचना box *box;
	plot_font_style_t fstyle;
	पूर्णांक total_height;
	संरचना form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	जर (control->data.select.menu == रिक्त) {

		menu = सुस्मृति(1, आकार (संरचना form_select_menu));
		जर (menu == रिक्त) {
			परत NSERROR_NOMEM;
		}

		control->data.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[उजवा_2].width + box->padding[उजवा_2] +
			box->border[डावा_2].width + box->padding[डावा_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_size = fstyle.size;

		menu->ओळ_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.size / PLOT_STYLE_SCALE)))),
				F_72));

		ओळ_height_with_spacing = menu->ओळ_height +
				menu->ओळ_height *
				SELECT_ओळ_SPACING;

		total_height = control->data.select.अंक_वस्तुसंच *
				ओळ_height_with_spacing;
		menu->height = total_height;

		जर (menu->height > MAX_SELECT_HEIGHT) {
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
		जर (res != NSERROR_OK) {
			control->data.select.menu = रिक्त;
			मुक्त(menu);
			परत res;
		}
		menu->c = c;
	} अन्यथा {
		menu = control->data.select.menu;
	}

	menu->callback(client_data, 0, 0, menu->width, menu->height);

	परत NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free_select_menu(संरचना form_control *control)
{
	जर (control->data.select.menu->scrollbar != रिक्त)
		scrollbar_destroy(control->data.select.menu->scrollbar);
	मुक्त(control->data.select.menu);
	control->data.select.menu = रिक्त;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(संरचना form_control *control,
			पूर्णांक x, पूर्णांक y,
			दशांश scale,
			स्थिरांक संरचना rect *clip,
			स्थिरांक संरचना redraw_context *ctx)
{
	संरचना box *box;
	संरचना form_select_menu *menu = control->data.select.menu;
	संरचना form_option *option;
	पूर्णांक ओळ_height, ओळ_height_with_spacing;
	पूर्णांक width, height;
	पूर्णांक x0, y0, x1, scrollbar_x, y1, y2, y3;
	पूर्णांक item_y;
	पूर्णांक text_pos_offset, text_x;
	पूर्णांक scrollbar_width = SCROLLBAR_WIDTH;
	पूर्णांक i;
	पूर्णांक scroll;
	पूर्णांक x_cp, y_cp;
	संरचना rect r;
	संरचना rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	ओळ_height = menu->ओळ_height;

	ओळ_height_with_spacing = ओळ_height +
			ओळ_height * SELECT_ओळ_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	जर (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / ओळ_height_with_spacing;
		scroll -= i * ओळ_height_with_spacing;
		ओळ_height *= scale;
		ओळ_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * ओळ_height_with_spacing;
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
	जर (res != NSERROR_OK) {
		परत false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	जर (res != NSERROR_OK) {
		परत false;
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
	जर (res != NSERROR_OK) {
		परत false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	जर (res != NSERROR_OK) {
		परत false;
	}

	option = control->data.select.वस्तुसंच;
	item_y = ओळ_height_with_spacing;

	पर्यंत (item_y < scroll) {
		option = option->next;
		item_y += ओळ_height_with_spacing;
	}
	item_y -= ओळ_height_with_spacing;
	text_pos_offset = y - scroll +
			(पूर्णांक) (ओळ_height * (0.75 + SELECT_ओळ_SPACING));
	text_x = x + (box->border[डावा_2].width + box->padding[डावा_2]) * scale;

	plot_fstyle_entry.size = menu->f_size;

	पर्यंत (option && item_y - scroll < height) {

		जर (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + ओळ_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			जर (res != NSERROR_OK) {
				परत false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, म_माप(option->text));
		जर (res != NSERROR_OK) {
			परत false;
		}

		item_y += ओळ_height_with_spacing;
		option = option->next;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	जर (res != NSERROR_OK) {
		परत false;
	}

	परत true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(संरचना form_control *control,
			     दशांश scale,
			     स्थिरांक संरचना rect *clip)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	पूर्णांक width, height;


	width = menu->width;
	height = menu->height;

	जर (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	जर (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		परत true;

	परत false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(संरचना form_control *control, पूर्णांक item)
{
	प्रतिज्ञा(control != रिक्त);

	परत form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
संरचना form_option *
form_select_get_option(संरचना form_control *control, पूर्णांक item)
{
	संरचना form_option *opt;

	opt = control->data.select.वस्तुसंच;
	पर्यंत ((opt != रिक्त) && (item > 0)) {
		opt = opt->next;
		item--;
	}
	परत opt;
}


/* exported interface documented in netsurf/form.h */
अक्षर *form_control_get_name(संरचना form_control *control)
{
	परत control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(संरचना form_control *control, संरचना rect *r)
{
	box_bounds( control->box, r );
	परत NSERROR_OK;
}


/* private interface described in html/form_internal.h */
स्थिरांक अक्षर *
form_select_mouse_action(संरचना form_control *control,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	संरचना form_select_menu *menu = control->data.select.menu;
	पूर्णांक x0, y0, x1, y1, scrollbar_x;
	स्थिरांक अक्षर *status = रिक्त;
	bool multiple = control->data.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	जर (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		परत scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	जर (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		जर (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		जर (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->data.select.multiple ?
					"SelectMClick" : "SelectClick");

	} अन्यथा जर (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	परत status;
}


/* private interface described in html/form_internal.h */
शून्य
form_select_mouse_drag_end(संरचना form_control *control,
			   browser_mouse_state mouse,
			   पूर्णांक x, पूर्णांक y)
{
	पूर्णांक x0, y0, x1, y1;
	पूर्णांक box_x, box_y;
	संरचना box *box;
	संरचना form_select_menu *menu = control->data.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[डावा_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[शीर्ष_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	जर (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		परत;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	जर (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
शून्य form_select_get_dimensions(संरचना form_control *control,
		पूर्णांक *width, पूर्णांक *height)
{
	*width = control->data.select.menu->width;
	*height = control->data.select.menu->height;
}


/* private interface described in html/form_internal.h */
शून्य form_select_menu_callback(शून्य *client_data,
		पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height)
{
	html_content *html = client_data;
	पूर्णांक menu_x, menu_y;
	संरचना box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[डावा_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[शीर्ष_2];
	content__request_redraw((संरचना content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
शून्य form_radio_set(संरचना form_control *radio)
{
	संरचना form_control *control;

	प्रतिज्ञा(radio);
	जर (!radio->form)
		परत;

	जर (radio->selected)
		परत;

	/* Clear selected state for other controls in
	 * the same radio button group */
	साठी (control = radio->form->controls;
	     control != रिक्त;
	     control = control->next) {
		/* Only interested in radio inputs */
		जर (control->type != GADGET_RADIO)
			सुरूठेवा;

		/* Ignore ourself */
		जर (control == radio)
			सुरूठेवा;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		जर ((control->name == रिक्त) ||
		    (radio->name == रिक्त) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    म_भेद(control->name, radio->name) != 0)
			सुरूठेवा;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		जर (control->selected) {
			control->selected = false;
			dom_html_आदान_element_set_checked(control->गाठ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_आदान_element_set_checked(radio->गाठ, true);
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
	अक्षर *data = रिक्त; /* encoded form data */
	संरचना fetch_multipart_data *success = रिक्त; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	प्रतिज्ञा(form != रिक्त);

	/* obtain list of controls from DOM */
	res = form_dom_to_data(form, submit_button, &success);
	जर (res != NSERROR_OK) {
		परत res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	जर (res != NSERROR_OK) {
		fetch_multipart_data_destroy(success);
		परत res;
	}

	निवड (form->method) {
	स्थिती method_GET:
		res = form_url_encode(form, success, &data);
		जर (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, data, &query_url);
			जर (res == NSERROR_OK) {
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
		थांबा;

	स्थिती method_POST_URLENC:
		res = form_url_encode(form, success, &data);
		जर (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      data,
						      रिक्त,
						      रिक्त);
			मुक्त(data);
		}
		थांबा;

	स्थिती method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      रिक्त,
					      success,
					      रिक्त);

		थांबा;
	}

	nsurl_unref(action_url);
	fetch_multipart_data_destroy(success);

	परत res;
}


/* exported interface documented in html/form_internal.h */
शून्य form_gadget_update_मूल्य(संरचना form_control *control, अक्षर *मूल्य)
{
	निवड (control->type) {
	स्थिती GADGET_HIDDEN:
	स्थिती GADGET_TEXTBOX:
	स्थिती GADGET_TEXTAREA:
	स्थिती GADGET_PASSWORD:
	स्थिती GADGET_FILE:
		जर (control->मूल्य != रिक्त) {
			मुक्त(control->मूल्य);
		}
		control->मूल्य = मूल्य;
		जर (control->गाठ != रिक्त) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)मूल्य,
						म_माप(मूल्य), &str);
			जर (err == DOM_NO_ERR) {
				जर (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_मूल्य(
						(dom_html_text_area_element *)(control->गाठ),
						str);
				अन्यथा
					err = dom_html_आदान_element_set_मूल्य(
						(dom_html_आदान_element *)(control->गाठ),
						str);
				dom_string_unref(str);
			}
		}
		थांबा;
	पूर्वनिर्धारित:
		/* Do nothing */
		थांबा;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
शून्य
form_gadget_sync_with_dom(संरचना form_control *control)
{
	dom_exception exc;
	dom_string *मूल्य = रिक्त;
	bool changed_dom = false;

	जर (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		परत;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	जर ((control->last_synced_मूल्य == रिक्त &&
	     control->मूल्य != रिक्त &&
	     control->मूल्य[0] != '\0') ||
	    (control->last_synced_मूल्य != रिक्त &&
	     control->मूल्य != रिक्त &&
	     म_भेद(control->मूल्य, control->last_synced_मूल्य) != 0)) {
		अक्षर *dup = strdup(control->मूल्य);
		जर (dup == रिक्त) {
			जा out;
		}
		जर (control->last_synced_मूल्य != रिक्त) {
			मुक्त(control->last_synced_मूल्य);
		}
		control->last_synced_मूल्य = dup;
		exc = dom_string_create((uint8_t *)(control->मूल्य),
					म_माप(control->मूल्य), &मूल्य);
		जर (exc != DOM_NO_ERR) {
			जा out;
		}
		जर (control->गाठ_मूल्य != रिक्त) {
			dom_string_unref(control->गाठ_मूल्य);
		}
		control->गाठ_मूल्य = मूल्य;
		मूल्य = रिक्त;
		जर (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_मूल्य(control->गाठ, control->गाठ_मूल्य);
		} अन्यथा {
			exc = dom_html_आदान_element_set_मूल्य(control->गाठ, control->गाठ_मूल्य);
		}
		जर (exc != DOM_NO_ERR) {
			जा out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	जर (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_मूल्य(control->गाठ, &मूल्य);
	} अन्यथा {
		exc = dom_html_आदान_element_get_मूल्य(control->गाठ, &मूल्य);
	}

	जर (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		जा out;
	}

	जर (!dom_string_isequal(control->गाठ_मूल्य, मूल्य)) {
		/* The DOM has changed */
		जर (!changed_dom) {
			/* And it wasn't us */
			अक्षर *मूल्य_s = strndup(
				dom_string_data(मूल्य),
				dom_string_byte_length(मूल्य));
			अक्षर *dup = रिक्त;
			जर (मूल्य_s == रिक्त) {
				जा out;
			}
			dup = strdup(मूल्य_s);
			जर (dup == रिक्त) {
				मुक्त(मूल्य_s);
				जा out;
			}
			मुक्त(control->मूल्य);
			control->मूल्य = मूल्य_s;
			मुक्त(control->last_synced_मूल्य);
			control->last_synced_मूल्य = dup;
			जर (control->type != GADGET_HIDDEN &&
			    control->data.text.ta != रिक्त) {
				textarea_set_text(control->data.text.ta,
						  मूल्य_s);
			}
		}
		control->गाठ_मूल्य = मूल्य;
		मूल्य = रिक्त;
	}

out:
	जर (मूल्य != रिक्त)
		dom_string_unref(मूल्य);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
संरचना form *
form_new(शून्य *गाठ,
	 स्थिरांक अक्षर *action,
	 स्थिरांक अक्षर *target,
	 form_method method,
	 स्थिरांक अक्षर *charset,
	 स्थिरांक अक्षर *doc_charset)
{
	संरचना form *form;

	form = सुस्मृति(1, आकार *form);
	जर (!form)
		परत रिक्त;

	form->action = strdup(action != रिक्त ? action : "");
	जर (form->action == रिक्त) {
		मुक्त(form);
		परत रिक्त;
	}

	form->target = target != रिक्त ? strdup(target) : रिक्त;
	जर (target != रिक्त && form->target == रिक्त) {
		मुक्त(form->action);
		मुक्त(form);
		परत रिक्त;
	}

	form->method = method;

	form->accept_charsets = charset != रिक्त ? strdup(charset) : रिक्त;
	जर (charset != रिक्त && form->accept_charsets == रिक्त) {
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		परत रिक्त;
	}

	form->document_charset = doc_charset != रिक्त ? strdup(doc_charset)
						     : रिक्त;
	जर (doc_charset && form->document_charset == रिक्त) {
		मुक्त(form->accept_charsets);
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		परत रिक्त;
	}

	form->गाठ = गाठ;

	परत form;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free(संरचना form *form)
{
	संरचना form_control *c, *d;

	साठी (c = form->controls; c != रिक्त; c = d) {
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
संरचना form_control *form_new_control(शून्य *गाठ, form_control_type type)
{
	संरचना form_control *control;

	control = सुस्मृति(1, आकार *control);
	जर (control == रिक्त)
		परत रिक्त;

	control->गाठ = गाठ;
	control->type = type;

	परत control;
}
