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

#समावेश <assert.h>
#समावेश <limits.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <dom/dom.h>

#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/talloc.h"
#समावेश "utils/url.h"
#समावेश "utils/utf8.h"
#समावेश "utils/ascii.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/mouse.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/misc.h"
#समावेश "content/fetch.h"
#समावेश "content/hlcache.h"
#समावेश "css/utils.h"
#समावेश "desktop/knockout.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/textarea.h"

#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/layout.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/font.h"
#समावेश "html/form_internal.h"

#घोषणा MAX_SELECT_HEIGHT 210
#घोषणा SELECT_पंक्ति_SPACING 0.2
#घोषणा SELECT_BORDER_WIDTH 1
#घोषणा SELECT_SELECTED_COLOUR 0xDB9370

काष्ठा form_select_menu {
	पूर्णांक पंक्ति_height;
	पूर्णांक width, height;
	काष्ठा scrollbar *scrollbar;
	पूर्णांक f_आकार;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	व्योम *client_तथ्य;
	काष्ठा content *c;
};

अटल plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

अटल plot_font_style_t plot_fstyle_entry = {
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
अटल अक्षर *
form_encode_item(स्थिर अक्षर *item,
		 uint32_t len,
		 स्थिर अक्षर *charset,
		 स्थिर अक्षर *fallback)
{
	nserror err;
	अक्षर *ret = शून्य;
	अक्षर cset[256];

	अगर (!item || !charset)
		वापस शून्य;

	snprintf(cset, माप cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	अगर (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, माप cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		अगर (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			अगर (fallback) {
				snprintf(cset, माप cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				अगर (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, माप cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			अगर (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				अगर (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	अगर (err == NSERROR_NOMEM) {
		वापस शून्य;
	}

	वापस ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#घोषणा FETCH_तथ्य_INT_मूल्य_आकार 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
अटल nserror
fetch_तथ्य_list_add_sname(स्थिर अक्षर *name,
			  स्थिर अक्षर *ksfx,
			  पूर्णांक मूल्य,
			  काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	काष्ठा fetch_multipart_तथ्य *fetch_तथ्य;
	पूर्णांक keysize;

	fetch_तथ्य = सुस्मृति(1, माप(*fetch_तथ्य));
	अगर (fetch_तथ्य == शून्य) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		वापस NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_तथ्य->name, 0, "%s%s", name, ksfx);
	fetch_तथ्य->name = दो_स्मृति(keysize + 1); /* allow for null */
	अगर (fetch_तथ्य->name == शून्य) {
		मुक्त(fetch_तथ्य);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		वापस NSERROR_NOMEM;
	}
	snprintf(fetch_तथ्य->name, keysize + 1, "%s%s", name, ksfx);

	/*मूल्य*/
	fetch_तथ्य->मूल्य = दो_स्मृति(FETCH_तथ्य_INT_मूल्य_आकार);
	अगर (fetch_तथ्य->मूल्य == शून्य) {
		मुक्त(fetch_तथ्य->name);
		मुक्त(fetch_तथ्य);
		NSLOG(netsurf, INFO, "value allocation failure");
		वापस NSERROR_NOMEM;
	}
	snprintf(fetch_तथ्य->मूल्य, FETCH_तथ्य_INT_मूल्य_आकार, "%d", मूल्य);

	/* link into list */
	**fetch_तथ्य_अग्र_ptr = fetch_तथ्य;
	*fetch_तथ्य_अग्र_ptr = &fetch_तथ्य->अग्र;

	वापस NSERROR_OK;
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
अटल nserror
fetch_तथ्य_list_add(dom_string *name,
		    dom_string *मूल्य,
		    स्थिर अक्षर *rawfile,
		    स्थिर अक्षर *form_charset,
		    स्थिर अक्षर *docu_charset,
		    काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	काष्ठा fetch_multipart_तथ्य *fetch_तथ्य;

	निश्चित(name != शून्य);

	fetch_तथ्य = सुस्मृति(1, माप(*fetch_तथ्य));
	अगर (fetch_तथ्य == शून्य) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		वापस NSERROR_NOMEM;
	}

	fetch_तथ्य->name = form_encode_item(dom_string_तथ्य(name),
					    dom_string_byte_लंबाई(name),
					    form_charset,
					    docu_charset);
	अगर (fetch_तथ्य->name == शून्य) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		मुक्त(fetch_तथ्य);
		वापस NSERROR_NOMEM;
	}

	अगर (मूल्य == शून्य) {
		fetch_तथ्य->मूल्य = strdup("");
	} अन्यथा {
		fetch_तथ्य->मूल्य = form_encode_item(dom_string_तथ्य(मूल्य),
						     dom_string_byte_लंबाई(मूल्य),
						     form_charset,
						     docu_charset);
	}
	अगर (fetch_तथ्य->मूल्य == शून्य) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		मुक्त(fetch_तथ्य->name);
		मुक्त(fetch_तथ्य);
		वापस NSERROR_NOMEM;
	}

	/* deal with raw file name */
	अगर (rawfile != शून्य) {
		fetch_तथ्य->file = true;
		fetch_तथ्य->rawfile = strdup(rawfile);
		अगर (fetch_तथ्य->rawfile == शून्य) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			मुक्त(fetch_तथ्य->मूल्य);
			मुक्त(fetch_तथ्य->name);
			मुक्त(fetch_तथ्य);
			वापस NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_तथ्य_अग्र_ptr = fetch_तथ्य;
	*fetch_तथ्य_अग्र_ptr = &fetch_तथ्य->अग्र;

	वापस NSERROR_OK;
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
अटल nserror
form_dom_to_तथ्य_textarea(dom_html_text_area_element *text_area_element,
			  स्थिर अक्षर *form_charset,
			  स्थिर अक्षर *doc_charset,
			  काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		वापस NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (inputname == शून्य) {
		/* allow enumeration to continue after element with no name */
		वापस NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_मूल्य(text_area_element,
						   &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		वापस NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(inputname,
				  inputvalue,
				  शून्य,
				  form_charset,
				  doc_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      स्थिर अक्षर *form_charset,
			      स्थिर अक्षर *docu_charset,
			      काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *मूल्य;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		वापस NSERROR_DOM;
	}

	अगर (!selected) {
		/* unselected options do not add fetch data entries */
		वापस NSERROR_OK;
	}

	exp = dom_html_option_element_get_मूल्य(option_element, &मूल्य);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		वापस NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(keyname,
				  मूल्य,
				  शून्य,
				  form_charset,
				  docu_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(मूल्य);

	वापस res;
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
अटल nserror
form_dom_to_तथ्य_select(dom_html_select_element *select_element,
			स्थिर अक्षर *form_charset,
			स्थिर अक्षर *doc_charset,
			काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = शून्य;
	uint32_t options_गिनती;
	uint32_t option_सूचकांक;
	dom_गाँठ *option_element = शून्य;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		वापस NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (inputname == शून्य) {
		/* allow enumeration to continue after element with no name */
		वापस NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		वापस NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_लंबाई(options, &options_गिनती);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		वापस NSERROR_DOM;
	}

	/* iterate over options collection */
	क्रम (option_सूचकांक = 0; option_सूचकांक < options_गिनती; ++option_सूचकांक) {
		exp = dom_html_options_collection_item(options,
						       option_सूचकांक,
						       &option_element);
		अगर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_सूचकांक);
			res = NSERROR_DOM;
		} अन्यथा {
			res = form_dom_to_तथ्य_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_तथ्य_अग्र_ptr);

			dom_गाँठ_unref(option_element);
		}

		अगर (res != NSERROR_OK) {
			अवरोध;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_निवेश_submit(dom_html_निवेश_element *निवेश_element,
			      dom_string *inputname,
			      स्थिर अक्षर *charset,
			      स्थिर अक्षर *document_charset,
			      dom_html_element **submit_button,
			      काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	अगर (*submit_button == शून्य) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)निवेश_element;
	} अन्यथा अगर (*submit_button != (dom_html_element *)निवेश_element) {
		वापस NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_निवेश_element_get_मूल्य(निवेश_element, &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		वापस NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(inputname,
				  inputvalue,
				  शून्य,
				  charset,
				  document_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(inputvalue);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_निवेश_image(dom_html_निवेश_element *निवेश_element,
			     dom_string *inputname,
			     स्थिर अक्षर *charset,
			     स्थिर अक्षर *document_charset,
			     dom_html_element **submit_button,
			     काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	काष्ठा image_निवेश_coords *coords;
	अक्षर *basename;

	/* Only use an image input if it was the thing which activated us */
	अगर (*submit_button != (dom_html_element *)निवेश_element) {
		वापस NSERROR_OK;
	}

	exp = dom_गाँठ_get_user_तथ्य((dom_गाँठ *)निवेश_element,
				     corestring_dom_ns_कुंजी_image_coords_गाँठ_तथ्य,
				     &coords);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		वापस NSERROR_DOM;
	}

	अगर (coords == शून्य) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		वापस NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_तथ्य(inputname),
				    dom_string_byte_लंबाई(inputname),
				    charset,
				    document_charset);
	अगर (basename == शून्य) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		वापस NSERROR_NOMEM;
	}

	res = fetch_तथ्य_list_add_sname(basename, ".x",
					coords->x,
					fetch_तथ्य_अग्र_ptr);

	अगर (res == NSERROR_OK) {
		res = fetch_तथ्य_list_add_sname(basename, ".y",
						coords->y,
						fetch_तथ्य_अग्र_ptr);
	}

	मुक्त(basename);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_निवेश_checkbox(dom_html_निवेश_element *निवेश_element,
				dom_string *inputname,
				स्थिर अक्षर *charset,
				स्थिर अक्षर *document_charset,
				काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_निवेश_element_get_checked(निवेश_element, &checked);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		वापस NSERROR_DOM;
	}

	अगर (!checked) {
		/* unchecked items do not generate a data entry */
		वापस NSERROR_OK;
	}

	exp = dom_html_निवेश_element_get_मूल्य(निवेश_element, &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		वापस NSERROR_DOM;
	}

	/* ensure a default value */
	अगर (inputvalue == शून्य) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(inputname,
				  inputvalue,
				  शून्य,
				  charset,
				  document_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(inputvalue);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_निवेश_file(dom_html_निवेश_element *निवेश_element,
			    dom_string *inputname,
			    स्थिर अक्षर *charset,
			    स्थिर अक्षर *document_charset,
			    काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	स्थिर अक्षर *rawfile = शून्य;

	exp = dom_html_निवेश_element_get_मूल्य(निवेश_element, &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		वापस NSERROR_DOM;
	}

	exp = dom_गाँठ_get_user_तथ्य((dom_गाँठ *)निवेश_element,
				     corestring_dom_ns_कुंजी_file_name_गाँठ_तथ्य,
				     &rawfile);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		वापस NSERROR_DOM;
	}

	अगर (rawfile == शून्य) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(inputvalue);

	वापस res;
}


अटल nserror
form_dom_to_तथ्य_निवेश_text(dom_html_निवेश_element *निवेश_element,
			    dom_string *inputname,
			    स्थिर अक्षर *charset,
			    स्थिर अक्षर *document_charset,
			    काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_निवेश_element_get_मूल्य(निवेश_element, &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		वापस NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_तथ्य_list_add(inputname,
				  inputvalue,
				  शून्य,
				  charset,
				  document_charset,
				  fetch_तथ्य_अग्र_ptr);

	dom_string_unref(inputvalue);

	वापस res;
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
अटल nserror
form_dom_to_तथ्य_निवेश(dom_html_निवेश_element *निवेश_element,
		       स्थिर अक्षर *charset,
		       स्थिर अक्षर *document_charset,
		       dom_html_element **submit_button,
		       काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_निवेश_element_get_disabled(निवेश_element,
						  &element_disabled);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (element_disabled) {
		/* disabled element requires no more processing */
		वापस NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_निवेश_element_get_name(निवेश_element, &inputname);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (inputname == शून्य) {
		/* element with no name is not converted */
		वापस NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_निवेश_element_get_type(निवेश_element, &inputtype);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		वापस NSERROR_DOM;
	}

	/* process according to input element type */
	अगर (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_तथ्य_निवेश_submit(निवेश_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_तथ्य_अग्र_ptr);

	} अन्यथा अगर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_तथ्य_निवेश_image(निवेश_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_तथ्य_अग्र_ptr);

	} अन्यथा अगर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_तथ्य_निवेश_checkbox(निवेश_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_तथ्य_अग्र_ptr);

	} अन्यथा अगर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_तथ्य_निवेश_file(निवेश_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_तथ्य_अग्र_ptr);

	} अन्यथा अगर (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} अन्यथा {
		/* Everything else is treated as text values */
		res = form_dom_to_तथ्य_निवेश_text(निवेश_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_तथ्य_अग्र_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	वापस res;
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
अटल nserror
form_dom_to_तथ्य_button(dom_html_button_element *button_element,
			स्थिर अक्षर *form_charset,
			स्थिर अक्षर *doc_charset,
			dom_html_element **submit_button,
			काष्ठा fetch_multipart_तथ्य ***fetch_तथ्य_अग्र_ptr)
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
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (element_disabled) {
		/* allow enumeration to continue after disabled element */
		वापस NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		वापस NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	अगर (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		वापस NSERROR_OK;
	}
	अगर (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		वापस NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	अगर (*submit_button == शून्य) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	अगर (*submit_button != (dom_html_element *)button_element) {
		वापस NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		वापस NSERROR_DOM;
	}

	अगर (inputname == शून्य) {
		/* allow enumeration to continue after element with no name */
		वापस NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_मूल्य(button_element, &inputvalue);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} अन्यथा {
		res = fetch_तथ्य_list_add(inputname,
					  inputvalue,
					  शून्य,
					  form_charset,
					  doc_charset,
					  fetch_तथ्य_अग्र_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	वापस res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
अटल अक्षर *form_acceptable_charset(काष्ठा form *form)
{
	अक्षर *अस्थायी, *c;

	अगर (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		अगर (form->document_charset) {
			/* document charset present, so use it */
			वापस strdup(form->document_charset);
		} अन्यथा {
			/* no document charset, so default to 8859-1 */
			वापस strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	अस्थायी = strdup(form->accept_charsets);
	अगर (!अस्थायी)
		वापस शून्य;

	/* make it upper case */
	क्रम (c = अस्थायी; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = म_माला(अस्थायी, "UTF-8");
	अगर (c) {
		मुक्त(अस्थायी);
		वापस strdup("UTF-8");
	}

	/* dispense with temporary copy */
	मुक्त(अस्थायी);

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
	जबतक (*c && !ascii_is_space(*c)) {
		अगर (*c == ',')
			अवरोध;
		c++;
	}

	वापस strndup(form->accept_charsets, c - form->accept_charsets);
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
अटल nserror
form_dom_to_तथ्य(काष्ठा form *form,
		 काष्ठा form_control *submit_control,
		 काष्ठा fetch_multipart_तथ्य **fetch_तथ्य_out)
{
	nserror res = NSERROR_OK;
	अक्षर *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = शून्य; /* the dom form elements */
	uint32_t element_गिनती; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_गाँठ *element = शून्य; /* the DOM form element */
	dom_string *nodename = शून्य; /* the DOM node name of the element */
	काष्ठा fetch_multipart_तथ्य *fetch_तथ्य = शून्य; /* fetch data list */
	काष्ठा fetch_multipart_तथ्य **fetch_तथ्य_अग्र = &fetch_तथ्य;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	अगर (submit_control != शून्य) {
		submit_button = submit_control->गाँठ;
	} अन्यथा {
		submit_button = शून्य;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	अगर (charset == शून्य) {
		NSLOG(netsurf, INFO, "failed to find charset");
		वापस NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->गाँठ, &elements);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		मुक्त(charset);
		वापस NSERROR_DOM;
	}

	exp = dom_html_collection_get_लंबाई(elements, &element_गिनती);
	अगर (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		जाओ form_dom_to_तथ्य_error;
	}

	क्रम (element_idx = 0; element_idx < element_गिनती; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		अगर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			जाओ form_dom_to_तथ्य_error;
		}

		/* node name from element */
		exp = dom_गाँठ_get_गाँठ_name(element, &nodename);
		अगर (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_गाँठ_unref(element);
			res = NSERROR_DOM;
			जाओ form_dom_to_तथ्य_error;
		}

		अगर (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_तथ्य_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_तथ्य_अग्र);

		} अन्यथा अगर (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_तथ्य_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_तथ्य_अग्र);

		} अन्यथा अगर (dom_string_isequal(nodename, corestring_dom_निवेश)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_तथ्य_निवेश(
				(dom_html_निवेश_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_तथ्य_अग्र);

		} अन्यथा अगर (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_तथ्य_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_तथ्य_अग्र);

		} अन्यथा {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (पूर्णांक)dom_string_byte_लंबाई(nodename),
			      dom_string_तथ्य(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_गाँठ_unref(element);

		/* abort form element enumeration on error */
		अगर (res != NSERROR_OK) {
			जाओ form_dom_to_तथ्य_error;
		}
	}

	*fetch_तथ्य_out = fetch_तथ्य;
	dom_html_collection_unref(elements);
	मुक्त(charset);

	वापस NSERROR_OK;

form_dom_to_तथ्य_error:
	fetch_multipart_तथ्य_destroy(fetch_तथ्य);
	dom_html_collection_unref(elements);
	मुक्त(charset);

	वापस res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
अटल nserror
form_url_encode(काष्ठा form *form,
		काष्ठा fetch_multipart_तथ्य *control,
		अक्षर **encoded_out)
{
	अक्षर *name, *मूल्य;
	अक्षर *s, *s2;
	अचिन्हित पूर्णांक len, len1, len_init;
	nserror res;

	s = दो_स्मृति(1);

	अगर (s == शून्य) {
		वापस NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	क्रम (; control; control = control->अग्र) {
		res = url_escape(control->name, true, शून्य, &name);
		अगर (res != NSERROR_OK) {
			मुक्त(s);
			वापस res;
		}

		res = url_escape(control->मूल्य, true, शून्य, &मूल्य);
		अगर (res != NSERROR_OK) {
			मुक्त(name);
			मुक्त(s);
			वापस res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + म_माप(name) + म_माप(मूल्य) + 2;
		s2 = पुनः_स्मृति(s, len1 + 1);
		अगर (s2 == शून्य) {
			मुक्त(मूल्य);
			मुक्त(name);
			मुक्त(s);
			वापस NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, मूल्य);
		len = len1;
		मुक्त(name);
		मुक्त(मूल्य);
	}

	अगर (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	वापस NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
अटल व्योम
form_select_menu_scroll_callback(व्योम *client_तथ्य,
				 काष्ठा scrollbar_संदेश_तथ्य *scrollbar_तथ्य)
{
	काष्ठा form_control *control = client_तथ्य;
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;
	html_content *html = (html_content *)menu->c;

	चयन (scrollbar_तथ्य->संदेश) {
		हाल SCROLLBAR_संदेश_MOVED:
			menu->callback(menu->client_तथ्य,
					0, 0,
					menu->width,
					menu->height);
			अवरोध;
		हाल SCROLLBAR_संदेश_SCROLL_START:
		{
			काष्ठा rect rect = {
				.x0 = scrollbar_तथ्य->x0,
				.y0 = scrollbar_तथ्य->y0,
				.x1 = scrollbar_तथ्य->x1,
				.y1 = scrollbar_तथ्य->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			अवरोध;
		हाल SCROLLBAR_संदेश_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, शून्य);
			अवरोध;
		शेष:
			अवरोध;
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
अटल nserror
form__select_process_selection(html_content *html,
			       काष्ठा form_control *control,
			       पूर्णांक item)
{
	काष्ठा box *inline_box;
	काष्ठा form_option *o;
	पूर्णांक गिनती;
	nserror ret = NSERROR_OK;

	निश्चित(control != शून्य);
	निश्चित(html != शून्य);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	क्रम (गिनती = 0, o = control->तथ्य.select.वस्तुएँ;
			o != शून्य;
			गिनती++, o = o->अग्र) {
		अगर (!control->तथ्य.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->गाँठ, false);
		}

		अगर (गिनती == item) {
			अगर (control->तथ्य.select.multiple) {
				अगर (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->गाँठ, false);
					control->तथ्य.select.अंक_selected--;
				} अन्यथा {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->गाँठ, true);
					control->तथ्य.select.अंक_selected++;
				}
			} अन्यथा {
				dom_html_option_element_set_selected(
						o->गाँठ, true);
				o->selected = true;
			}
		}

		अगर (o->selected) {
			control->तथ्य.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	अगर (control->तथ्य.select.अंक_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} अन्यथा अगर (control->तथ्य.select.अंक_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->तथ्य.select.current->text);
	} अन्यथा {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	अगर (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->लंबाई = 0;
	} अन्यथा {
		inline_box->लंबाई = म_माप(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	वापस ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
अटल व्योम form_select_menu_clicked(काष्ठा form_control *control, पूर्णांक x, पूर्णांक y)
{
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;
	काष्ठा form_option *option;
	html_content *html = (html_content *)menu->c;
	पूर्णांक पंक्ति_height, पंक्ति_height_with_spacing;
	पूर्णांक item_bottom_y;
	पूर्णांक scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	पंक्ति_height = menu->पंक्ति_height;
	पंक्ति_height_with_spacing = पंक्ति_height +
			पंक्ति_height * SELECT_पंक्ति_SPACING;

	option = control->तथ्य.select.वस्तुएँ;
	item_bottom_y = पंक्ति_height_with_spacing;
	i = 0;
	जबतक (option && item_bottom_y < scroll + y) {
		item_bottom_y += पंक्ति_height_with_spacing;
		option = option->अग्र;
		i++;
	}

	अगर (option != शून्य) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_तथ्य, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
व्योम form_add_control(काष्ठा form *form, काष्ठा form_control *control)
{
	अगर (form == शून्य) {
		वापस;
	}

	control->form = form;

	अगर (form->controls != शून्य) {
		निश्चित(form->last_control);

		form->last_control->अग्र = control;
		control->पूर्व = form->last_control;
		control->अग्र = शून्य;
		form->last_control = control;
	} अन्यथा {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
व्योम form_free_control(काष्ठा form_control *control)
{
	काष्ठा form_control *c;
	निश्चित(control != शून्य);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->मूल्य, control->initial_मूल्य);
	मुक्त(control->name);
	मुक्त(control->मूल्य);
	मुक्त(control->initial_मूल्य);
	अगर (control->last_synced_मूल्य != शून्य) {
		मुक्त(control->last_synced_मूल्य);
	}

	अगर (control->type == GADGET_SELECT) {
		काष्ठा form_option *option, *अग्र;

		क्रम (option = control->तथ्य.select.वस्तुएँ; option;
				option = अग्र) {
			अग्र = option->अग्र;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->मूल्य);
			मुक्त(option->text);
			मुक्त(option->मूल्य);
			मुक्त(option);
		}
		अगर (control->तथ्य.select.menu != शून्य) {
			form_free_select_menu(control);
		}
	}

	अगर (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		अगर (control->तथ्य.text.initial != शून्य) {
			dom_string_unref(control->तथ्य.text.initial);
		}

		अगर (control->तथ्य.text.ta != शून्य) {
			textarea_destroy(control->तथ्य.text.ta);
		}
	}

	/* unlink the control from the form */
	अगर (control->form != शून्य) {
		क्रम (c = control->form->controls; c != शून्य; c = c->अग्र) {
			अगर (c->अग्र == control) {
				c->अग्र = control->अग्र;
				अगर (control->form->last_control == control)
					control->form->last_control = c;
				अवरोध;
			}
			अगर (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->अग्र;
				अगर (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = शून्य;
				अवरोध;
			}
		}
	}

	अगर (control->गाँठ_मूल्य != शून्य) {
		dom_string_unref(control->गाँठ_मूल्य);
	}

	मुक्त(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(काष्ठा form_control *control, अक्षर *मूल्य, अक्षर *text,
		     bool selected, व्योम *गाँठ)
{
	काष्ठा form_option *option;

	निश्चित(control);
	निश्चित(control->type == GADGET_SELECT);

	option = सुस्मृति(1, माप *option);
	अगर (!option)
		वापस false;

	option->मूल्य = मूल्य;
	option->text = text;

	/* add to linked list */
	अगर (control->तथ्य.select.वस्तुएँ == 0)
		control->तथ्य.select.वस्तुएँ = option;
	अन्यथा
		control->तथ्य.select.last_item->अग्र = option;
	control->तथ्य.select.last_item = option;

	/* set selected */
	अगर (selected && (control->तथ्य.select.अंक_selected == 0 ||
			control->तथ्य.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->तथ्य.select.अंक_selected++;
		control->तथ्य.select.current = option;
	}

	control->तथ्य.select.अंक_वस्तुएँ++;

	option->गाँठ = गाँठ;

	वापस true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(व्योम *client_तथ्य,
		      काष्ठा form_control *control,
		      select_menu_redraw_callback callback,
		      काष्ठा content *c)
{
	पूर्णांक पंक्ति_height_with_spacing;
	काष्ठा box *box;
	plot_font_style_t fstyle;
	पूर्णांक योग_height;
	काष्ठा form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	अगर (control->तथ्य.select.menu == शून्य) {

		menu = सुस्मृति(1, माप (काष्ठा form_select_menu));
		अगर (menu == शून्य) {
			वापस NSERROR_NOMEM;
		}

		control->तथ्य.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[दायाँ_2].width + box->padding[दायाँ_2] +
			box->border[बायाँ_2].width + box->padding[बायाँ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_आकार = fstyle.आकार;

		menu->पंक्ति_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.आकार / PLOT_STYLE_SCALE)))),
				F_72));

		पंक्ति_height_with_spacing = menu->पंक्ति_height +
				menu->पंक्ति_height *
				SELECT_पंक्ति_SPACING;

		योग_height = control->तथ्य.select.अंक_वस्तुएँ *
				पंक्ति_height_with_spacing;
		menu->height = योग_height;

		अगर (menu->height > MAX_SELECT_HEIGHT) {
			menu->height = MAX_SELECT_HEIGHT;
		}

		menu->client_तथ्य = client_तथ्य;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->height,
				       योग_height,
				       menu->height,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		अगर (res != NSERROR_OK) {
			control->तथ्य.select.menu = शून्य;
			मुक्त(menu);
			वापस res;
		}
		menu->c = c;
	} अन्यथा {
		menu = control->तथ्य.select.menu;
	}

	menu->callback(client_तथ्य, 0, 0, menu->width, menu->height);

	वापस NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
व्योम form_free_select_menu(काष्ठा form_control *control)
{
	अगर (control->तथ्य.select.menu->scrollbar != शून्य)
		scrollbar_destroy(control->तथ्य.select.menu->scrollbar);
	मुक्त(control->तथ्य.select.menu);
	control->तथ्य.select.menu = शून्य;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(काष्ठा form_control *control,
			पूर्णांक x, पूर्णांक y,
			भग्न scale,
			स्थिर काष्ठा rect *clip,
			स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा box *box;
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;
	काष्ठा form_option *option;
	पूर्णांक पंक्ति_height, पंक्ति_height_with_spacing;
	पूर्णांक width, height;
	पूर्णांक x0, y0, x1, scrollbar_x, y1, y2, y3;
	पूर्णांक item_y;
	पूर्णांक text_pos_offset, text_x;
	पूर्णांक scrollbar_width = SCROLLBAR_WIDTH;
	पूर्णांक i;
	पूर्णांक scroll;
	पूर्णांक x_cp, y_cp;
	काष्ठा rect r;
	काष्ठा rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	पंक्ति_height = menu->पंक्ति_height;

	पंक्ति_height_with_spacing = पंक्ति_height +
			पंक्ति_height * SELECT_पंक्ति_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	अगर (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / पंक्ति_height_with_spacing;
		scroll -= i * पंक्ति_height_with_spacing;
		पंक्ति_height *= scale;
		पंक्ति_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * पंक्ति_height_with_spacing;
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
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
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
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	option = control->तथ्य.select.वस्तुएँ;
	item_y = पंक्ति_height_with_spacing;

	जबतक (item_y < scroll) {
		option = option->अग्र;
		item_y += पंक्ति_height_with_spacing;
	}
	item_y -= पंक्ति_height_with_spacing;
	text_pos_offset = y - scroll +
			(पूर्णांक) (पंक्ति_height * (0.75 + SELECT_पंक्ति_SPACING));
	text_x = x + (box->border[बायाँ_2].width + box->padding[बायाँ_2]) * scale;

	plot_fstyle_entry.आकार = menu->f_आकार;

	जबतक (option && item_y - scroll < height) {

		अगर (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + पंक्ति_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, म_माप(option->text));
		अगर (res != NSERROR_OK) {
			वापस false;
		}

		item_y += पंक्ति_height_with_spacing;
		option = option->अग्र;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	वापस true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(काष्ठा form_control *control,
			     भग्न scale,
			     स्थिर काष्ठा rect *clip)
{
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;
	पूर्णांक width, height;


	width = menu->width;
	height = menu->height;

	अगर (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	अगर (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		वापस true;

	वापस false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(काष्ठा form_control *control, पूर्णांक item)
{
	निश्चित(control != शून्य);

	वापस form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
काष्ठा form_option *
form_select_get_option(काष्ठा form_control *control, पूर्णांक item)
{
	काष्ठा form_option *opt;

	opt = control->तथ्य.select.वस्तुएँ;
	जबतक ((opt != शून्य) && (item > 0)) {
		opt = opt->अग्र;
		item--;
	}
	वापस opt;
}


/* exported interface documented in netsurf/form.h */
अक्षर *form_control_get_name(काष्ठा form_control *control)
{
	वापस control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(काष्ठा form_control *control, काष्ठा rect *r)
{
	box_bounds( control->box, r );
	वापस NSERROR_OK;
}


/* private interface described in html/form_internal.h */
स्थिर अक्षर *
form_select_mouse_action(काष्ठा form_control *control,
			 browser_mouse_state mouse,
			 पूर्णांक x, पूर्णांक y)
{
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;
	पूर्णांक x0, y0, x1, y1, scrollbar_x;
	स्थिर अक्षर *status = शून्य;
	bool multiple = control->तथ्य.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	अगर (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		वापस scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	अगर (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		अगर (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		अगर (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->तथ्य.select.multiple ?
					"SelectMClick" : "SelectClick");

	} अन्यथा अगर (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	वापस status;
}


/* private interface described in html/form_internal.h */
व्योम
form_select_mouse_drag_end(काष्ठा form_control *control,
			   browser_mouse_state mouse,
			   पूर्णांक x, पूर्णांक y)
{
	पूर्णांक x0, y0, x1, y1;
	पूर्णांक box_x, box_y;
	काष्ठा box *box;
	काष्ठा form_select_menu *menu = control->तथ्य.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[बायाँ_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[शीर्ष_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	अगर (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		वापस;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	अगर (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
व्योम form_select_get_dimensions(काष्ठा form_control *control,
		पूर्णांक *width, पूर्णांक *height)
{
	*width = control->तथ्य.select.menu->width;
	*height = control->तथ्य.select.menu->height;
}


/* private interface described in html/form_internal.h */
व्योम form_select_menu_callback(व्योम *client_तथ्य,
		पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height)
{
	html_content *html = client_तथ्य;
	पूर्णांक menu_x, menu_y;
	काष्ठा box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[बायाँ_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[शीर्ष_2];
	content__request_redraw((काष्ठा content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
व्योम form_radio_set(काष्ठा form_control *radio)
{
	काष्ठा form_control *control;

	निश्चित(radio);
	अगर (!radio->form)
		वापस;

	अगर (radio->selected)
		वापस;

	/* Clear selected state for other controls in
	 * the same radio button group */
	क्रम (control = radio->form->controls;
	     control != शून्य;
	     control = control->अग्र) {
		/* Only interested in radio inputs */
		अगर (control->type != GADGET_RADIO)
			जारी;

		/* Ignore ourself */
		अगर (control == radio)
			जारी;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		अगर ((control->name == शून्य) ||
		    (radio->name == शून्य) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    म_भेद(control->name, radio->name) != 0)
			जारी;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		अगर (control->selected) {
			control->selected = false;
			dom_html_निवेश_element_set_checked(control->गाँठ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_निवेश_element_set_checked(radio->गाँठ, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    काष्ठा browser_window *target,
	    काष्ठा form *form,
	    काष्ठा form_control *submit_button)
{
	nserror res;
	अक्षर *तथ्य = शून्य; /* encoded form data */
	काष्ठा fetch_multipart_तथ्य *success = शून्य; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	निश्चित(form != शून्य);

	/* obtain list of controls from DOM */
	res = form_dom_to_तथ्य(form, submit_button, &success);
	अगर (res != NSERROR_OK) {
		वापस res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	अगर (res != NSERROR_OK) {
		fetch_multipart_तथ्य_destroy(success);
		वापस res;
	}

	चयन (form->method) {
	हाल method_GET:
		res = form_url_encode(form, success, &तथ्य);
		अगर (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, तथ्य, &query_url);
			अगर (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      शून्य,
							      शून्य,
							      शून्य);

				nsurl_unref(query_url);
			}
			मुक्त(तथ्य);
		}
		अवरोध;

	हाल method_POST_URLENC:
		res = form_url_encode(form, success, &तथ्य);
		अगर (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      तथ्य,
						      शून्य,
						      शून्य);
			मुक्त(तथ्य);
		}
		अवरोध;

	हाल method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      शून्य,
					      success,
					      शून्य);

		अवरोध;
	}

	nsurl_unref(action_url);
	fetch_multipart_तथ्य_destroy(success);

	वापस res;
}


/* exported interface documented in html/form_internal.h */
व्योम form_gadget_update_मूल्य(काष्ठा form_control *control, अक्षर *मूल्य)
{
	चयन (control->type) {
	हाल GADGET_HIDDEN:
	हाल GADGET_TEXTBOX:
	हाल GADGET_TEXTAREA:
	हाल GADGET_PASSWORD:
	हाल GADGET_FILE:
		अगर (control->मूल्य != शून्य) {
			मुक्त(control->मूल्य);
		}
		control->मूल्य = मूल्य;
		अगर (control->गाँठ != शून्य) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)मूल्य,
						म_माप(मूल्य), &str);
			अगर (err == DOM_NO_ERR) {
				अगर (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_मूल्य(
						(dom_html_text_area_element *)(control->गाँठ),
						str);
				अन्यथा
					err = dom_html_निवेश_element_set_मूल्य(
						(dom_html_निवेश_element *)(control->गाँठ),
						str);
				dom_string_unref(str);
			}
		}
		अवरोध;
	शेष:
		/* Do nothing */
		अवरोध;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
व्योम
form_gadget_sync_with_dom(काष्ठा form_control *control)
{
	dom_exception exc;
	dom_string *मूल्य = शून्य;
	bool changed_dom = false;

	अगर (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		वापस;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	अगर ((control->last_synced_मूल्य == शून्य &&
	     control->मूल्य != शून्य &&
	     control->मूल्य[0] != '\0') ||
	    (control->last_synced_मूल्य != शून्य &&
	     control->मूल्य != शून्य &&
	     म_भेद(control->मूल्य, control->last_synced_मूल्य) != 0)) {
		अक्षर *dup = strdup(control->मूल्य);
		अगर (dup == शून्य) {
			जाओ out;
		}
		अगर (control->last_synced_मूल्य != शून्य) {
			मुक्त(control->last_synced_मूल्य);
		}
		control->last_synced_मूल्य = dup;
		exc = dom_string_create((uint8_t *)(control->मूल्य),
					म_माप(control->मूल्य), &मूल्य);
		अगर (exc != DOM_NO_ERR) {
			जाओ out;
		}
		अगर (control->गाँठ_मूल्य != शून्य) {
			dom_string_unref(control->गाँठ_मूल्य);
		}
		control->गाँठ_मूल्य = मूल्य;
		मूल्य = शून्य;
		अगर (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_मूल्य(control->गाँठ, control->गाँठ_मूल्य);
		} अन्यथा {
			exc = dom_html_निवेश_element_set_मूल्य(control->गाँठ, control->गाँठ_मूल्य);
		}
		अगर (exc != DOM_NO_ERR) {
			जाओ out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	अगर (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_मूल्य(control->गाँठ, &मूल्य);
	} अन्यथा {
		exc = dom_html_निवेश_element_get_मूल्य(control->गाँठ, &मूल्य);
	}

	अगर (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		जाओ out;
	}

	अगर (!dom_string_isequal(control->गाँठ_मूल्य, मूल्य)) {
		/* The DOM has changed */
		अगर (!changed_dom) {
			/* And it wasn't us */
			अक्षर *मूल्य_s = strndup(
				dom_string_तथ्य(मूल्य),
				dom_string_byte_लंबाई(मूल्य));
			अक्षर *dup = शून्य;
			अगर (मूल्य_s == शून्य) {
				जाओ out;
			}
			dup = strdup(मूल्य_s);
			अगर (dup == शून्य) {
				मुक्त(मूल्य_s);
				जाओ out;
			}
			मुक्त(control->मूल्य);
			control->मूल्य = मूल्य_s;
			मुक्त(control->last_synced_मूल्य);
			control->last_synced_मूल्य = dup;
			अगर (control->type != GADGET_HIDDEN &&
			    control->तथ्य.text.ta != शून्य) {
				textarea_set_text(control->तथ्य.text.ta,
						  मूल्य_s);
			}
		}
		control->गाँठ_मूल्य = मूल्य;
		मूल्य = शून्य;
	}

out:
	अगर (मूल्य != शून्य)
		dom_string_unref(मूल्य);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
काष्ठा form *
form_new(व्योम *गाँठ,
	 स्थिर अक्षर *action,
	 स्थिर अक्षर *target,
	 form_method method,
	 स्थिर अक्षर *charset,
	 स्थिर अक्षर *doc_charset)
{
	काष्ठा form *form;

	form = सुस्मृति(1, माप *form);
	अगर (!form)
		वापस शून्य;

	form->action = strdup(action != शून्य ? action : "");
	अगर (form->action == शून्य) {
		मुक्त(form);
		वापस शून्य;
	}

	form->target = target != शून्य ? strdup(target) : शून्य;
	अगर (target != शून्य && form->target == शून्य) {
		मुक्त(form->action);
		मुक्त(form);
		वापस शून्य;
	}

	form->method = method;

	form->accept_charsets = charset != शून्य ? strdup(charset) : शून्य;
	अगर (charset != शून्य && form->accept_charsets == शून्य) {
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		वापस शून्य;
	}

	form->document_charset = doc_charset != शून्य ? strdup(doc_charset)
						     : शून्य;
	अगर (doc_charset && form->document_charset == शून्य) {
		मुक्त(form->accept_charsets);
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		वापस शून्य;
	}

	form->गाँठ = गाँठ;

	वापस form;
}


/* exported interface documented in html/form_internal.h */
व्योम form_free(काष्ठा form *form)
{
	काष्ठा form_control *c, *d;

	क्रम (c = form->controls; c != शून्य; c = d) {
		d = c->अग्र;

		form_free_control(c);
	}

	मुक्त(form->action);
	मुक्त(form->target);
	मुक्त(form->accept_charsets);
	मुक्त(form->document_charset);

	मुक्त(form);
}


/* exported interface documented in html/form_internal.h */
काष्ठा form_control *form_new_control(व्योम *गाँठ, form_control_type type)
{
	काष्ठा form_control *control;

	control = सुस्मृति(1, माप *control);
	अगर (control == शून्य)
		वापस शून्य;

	control->गाँठ = गाँठ;
	control->type = type;

	वापस control;
}
