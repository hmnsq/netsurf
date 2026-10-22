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

#परिभाषय MAX_SELECT_HEIGHT 210
#परिभाषय SELECT_पङ्क्तिः_SPACING 0.2
#परिभाषय SELECT_BORDER_विस्तारः 1
#परिभाषय SELECT_SELECTED_रङ्गः 0xDB9370

संरचना form_select_menu {
	पूर्णाङ्क पङ्क्तिः_height;
	पूर्णाङ्क विस्तारः, height;
	संरचना scrollbar *scrollbar;
	पूर्णाङ्क f_आकार;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	शून्य *client_दत्त;
	संरचना content *c;
};

स्थिर plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_रङ्गः = SELECT_SELECTED_रङ्गः,
};

स्थिर plot_font_style_t plot_fstyle_प्रविष्टिः = {
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

	यदि (!item || !charset)
		प्रत्यागम रिक्त;

	snprintf(cset, परिमाण cset, "%s//TRANSLIT", charset);

	err = utf8_गन्तव्यम्_enc(item, cset, 0, &ret);
	यदि (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, परिमाण cset, "%s", charset);
		err = utf8_गन्तव्यम्_enc(item, cset, len, &ret);

		यदि (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			यदि (fallback) {
				snprintf(cset, परिमाण cset,
						"%s//TRANSLIT", fallback);
				err = utf8_गन्तव्यम्_enc(item, cset, 0, &ret);

				यदि (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, परिमाण cset,
							"%s", fallback);
					err = utf8_गन्तव्यम्_enc(item, cset, 0, &ret);
				}
			}

			यदि (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_गन्तव्यम्_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				यदि (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_गन्तव्यम्_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	यदि (err == NSERROR_NOMEM) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#परिभाषय FETCH_दत्त_INT_मानम्_आकार 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
स्थिर nserror
fetch_दत्त_list_सङ्कलय_sname(ध्रुव वर्ण *name,
			  ध्रुव वर्ण *ksfx,
			  पूर्णाङ्क मानम्,
			  संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	संरचना fetch_multipart_दत्त *fetch_दत्त;
	पूर्णाङ्क keysize;

	fetch_दत्त = शून्यस्मृतिं_देहि(1, परिमाण(*fetch_दत्त));
	यदि (fetch_दत्त == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		प्रत्यागम NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_दत्त->name, 0, "%s%s", name, ksfx);
	fetch_दत्त->name = स्मृतिं_देहि(keysize + 1); /* allow for null */
	यदि (fetch_दत्त->name == रिक्त) {
		मुक्त(fetch_दत्त);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		प्रत्यागम NSERROR_NOMEM;
	}
	snprintf(fetch_दत्त->name, keysize + 1, "%s%s", name, ksfx);

	/*मानम्*/
	fetch_दत्त->मानम् = स्मृतिं_देहि(FETCH_दत्त_INT_मानम्_आकार);
	यदि (fetch_दत्त->मानम् == रिक्त) {
		मुक्त(fetch_दत्त->name);
		मुक्त(fetch_दत्त);
		NSLOG(netsurf, INFO, "value allocation failure");
		प्रत्यागम NSERROR_NOMEM;
	}
	snprintf(fetch_दत्त->मानम्, FETCH_दत्त_INT_मानम्_आकार, "%d", मानम्);

	/* link into list */
	**fetch_दत्त_अग्र_ptr = fetch_दत्त;
	*fetch_दत्त_अग्र_ptr = &fetch_दत्त->अग्र;

	प्रत्यागम NSERROR_OK;
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
fetch_दत्त_list_सङ्कलय(dom_string *name,
		    dom_string *मानम्,
		    ध्रुव वर्ण *rawfile,
		    ध्रुव वर्ण *form_charset,
		    ध्रुव वर्ण *docu_charset,
		    संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	संरचना fetch_multipart_दत्त *fetch_दत्त;

	प्रतिज्ञा(name != रिक्त);

	fetch_दत्त = शून्यस्मृतिं_देहि(1, परिमाण(*fetch_दत्त));
	यदि (fetch_दत्त == रिक्त) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		प्रत्यागम NSERROR_NOMEM;
	}

	fetch_दत्त->name = form_encode_item(dom_string_दत्त(name),
					    dom_string_byte_दीर्घता(name),
					    form_charset,
					    docu_charset);
	यदि (fetch_दत्त->name == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		मुक्त(fetch_दत्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	यदि (मानम् == रिक्त) {
		fetch_दत्त->मानम् = strdup("");
	} अन्यथा {
		fetch_दत्त->मानम् = form_encode_item(dom_string_दत्त(मानम्),
						     dom_string_byte_दीर्घता(मानम्),
						     form_charset,
						     docu_charset);
	}
	यदि (fetch_दत्त->मानम् == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		मुक्त(fetch_दत्त->name);
		मुक्त(fetch_दत्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	/* deal with raw file name */
	यदि (rawfile != रिक्त) {
		fetch_दत्त->file = true;
		fetch_दत्त->rawfile = strdup(rawfile);
		यदि (fetch_दत्त->rawfile == रिक्त) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			मुक्त(fetch_दत्त->मानम्);
			मुक्त(fetch_दत्त->name);
			मुक्त(fetch_दत्त);
			प्रत्यागम NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_दत्त_अग्र_ptr = fetch_दत्त;
	*fetch_दत्त_अग्र_ptr = &fetch_दत्त->अग्र;

	प्रत्यागम NSERROR_OK;
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
form_dom_गन्तव्यम्_दत्त_textarea(dom_html_text_area_element *text_area_element,
			  ध्रुव वर्ण *form_charset,
			  ध्रुव वर्ण *doc_charset,
			  संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	dom_exception घातांकः; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	घातांकः = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (element_disabled) {
		/* allow enumeration to continue after disabled element */
		प्रत्यागम NSERROR_OK;
	}

	/* obtain name property */
	घातांकः = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		प्रत्यागम NSERROR_OK;
	}

	/* obtain text area value */
	घातांकः = dom_html_text_area_element_get_मानम्(text_area_element,
						   &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", घातांकः);
		dom_string_unref(inputname);
		प्रत्यागम NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(inputname,
				  inputvalue,
				  रिक्त,
				  form_charset,
				  doc_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ध्रुव वर्ण *form_charset,
			      ध्रुव वर्ण *docu_charset,
			      संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res;
	dom_exception घातांकः; /* the result from DOM operations */
	dom_string *मानम्;
	bool selected;

	घातांकः = dom_html_option_element_get_selected(option_element, &selected);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		प्रत्यागम NSERROR_DOM;
	}

	यदि (!selected) {
		/* unselected options do not add fetch data entries */
		प्रत्यागम NSERROR_OK;
	}

	घातांकः = dom_html_option_element_get_मानम्(option_element, &मानम्);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		प्रत्यागम NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(keyname,
				  मानम्,
				  रिक्त,
				  form_charset,
				  docu_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(मानम्);

	प्रत्यागम res;
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
form_dom_गन्तव्यम्_दत्त_select(dom_html_select_element *select_element,
			ध्रुव वर्ण *form_charset,
			ध्रुव वर्ण *doc_charset,
			संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception घातांकः; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = रिक्त;
	uint32_t options_गणनम्;
	uint32_t option_सूचकांक;
	dom_ग्रन्थिः *option_element = रिक्त;

	/* check if element is disabled */
	घातांकः = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (element_disabled) {
		/* allow enumeration to continue after disabled element */
		प्रत्यागम NSERROR_OK;
	}

	/* obtain name property */
	घातांकः = dom_html_select_element_get_name(select_element, &inputname);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		प्रत्यागम NSERROR_OK;
	}

	/* get options collection */
	घातांकः = dom_html_select_element_get_options(select_element, &options);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		प्रत्यागम NSERROR_DOM;
	}

	/* get options collection length */
	घातांकः = dom_html_options_collection_get_दीर्घता(options, &options_गणनम्);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		प्रत्यागम NSERROR_DOM;
	}

	/* iterate over options collection */
	प्रति (option_सूचकांक = 0; option_सूचकांक < options_गणनम्; ++option_सूचकांक) {
		घातांकः = dom_html_options_collection_item(options,
						       option_सूचकांक,
						       &option_element);
		यदि (घातांकः != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_सूचकांक);
			res = NSERROR_DOM;
		} अन्यथा {
			res = form_dom_गन्तव्यम्_दत्त_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_दत्त_अग्र_ptr);

			dom_ग्रन्थिः_unref(option_element);
		}

		यदि (res != NSERROR_OK) {
			भङ्ग;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_निवेशः_submit(dom_html_निवेशः_element *निवेशः_element,
			      dom_string *inputname,
			      ध्रुव वर्ण *charset,
			      ध्रुव वर्ण *document_charset,
			      dom_html_element **submit_button,
			      संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	dom_exception घातांकः; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	यदि (*submit_button == रिक्त) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)निवेशः_element;
	} अन्यथा यदि (*submit_button != (dom_html_element *)निवेशः_element) {
		प्रत्यागम NSERROR_OK;
	}

	/* matched button used to submit form */
	घातांकः = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		प्रत्यागम NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(inputvalue);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_निवेशः_image(dom_html_निवेशः_element *निवेशः_element,
			     dom_string *inputname,
			     ध्रुव वर्ण *charset,
			     ध्रुव वर्ण *document_charset,
			     dom_html_element **submit_button,
			     संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res;
	dom_exception घातांकः; /* the result from DOM operations */
	संरचना image_निवेशः_coords *coords;
	वर्ण *basename;

	/* Only use an image input if it was the thing which activated us */
	यदि (*submit_button != (dom_html_element *)निवेशः_element) {
		प्रत्यागम NSERROR_OK;
	}

	घातांकः = dom_ग्रन्थिः_get_user_दत्त((dom_ग्रन्थिः *)निवेशः_element,
				     corestring_dom_ns_कुञ्चिका_image_coords_ग्रन्थिः_दत्त,
				     &coords);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		प्रत्यागम NSERROR_DOM;
	}

	यदि (coords == रिक्त) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		प्रत्यागम NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_दत्त(inputname),
				    dom_string_byte_दीर्घता(inputname),
				    charset,
				    document_charset);
	यदि (basename == रिक्त) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		प्रत्यागम NSERROR_NOMEM;
	}

	res = fetch_दत्त_list_सङ्कलय_sname(basename, ".x",
					coords->x,
					fetch_दत्त_अग्र_ptr);

	यदि (res == NSERROR_OK) {
		res = fetch_दत्त_list_सङ्कलय_sname(basename, ".y",
						coords->y,
						fetch_दत्त_अग्र_ptr);
	}

	मुक्त(basename);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_निवेशः_checkbox(dom_html_निवेशः_element *निवेशः_element,
				dom_string *inputname,
				ध्रुव वर्ण *charset,
				ध्रुव वर्ण *document_charset,
				संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res;
	dom_exception घातांकः; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	घातांकः = dom_html_निवेशः_element_get_checked(निवेशः_element, &checked);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		प्रत्यागम NSERROR_DOM;
	}

	यदि (!checked) {
		/* unchecked items do not generate a data entry */
		प्रत्यागम NSERROR_OK;
	}

	घातांकः = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		प्रत्यागम NSERROR_DOM;
	}

	/* ensure a default value */
	यदि (inputvalue == रिक्त) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(inputvalue);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_निवेशः_file(dom_html_निवेशः_element *निवेशः_element,
			    dom_string *inputname,
			    ध्रुव वर्ण *charset,
			    ध्रुव वर्ण *document_charset,
			    संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res;
	dom_exception घातांकः; /* the result from DOM operations */
	dom_string *inputvalue;
	ध्रुव वर्ण *rawfile = रिक्त;

	घातांकः = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		प्रत्यागम NSERROR_DOM;
	}

	घातांकः = dom_ग्रन्थिः_get_user_दत्त((dom_ग्रन्थिः *)निवेशः_element,
				     corestring_dom_ns_कुञ्चिका_file_name_ग्रन्थिः_दत्त,
				     &rawfile);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		प्रत्यागम NSERROR_DOM;
	}

	यदि (rawfile == रिक्त) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(inputvalue);

	प्रत्यागम res;
}


स्थिर nserror
form_dom_गन्तव्यम्_दत्त_निवेशः_text(dom_html_निवेशः_element *निवेशः_element,
			    dom_string *inputname,
			    ध्रुव वर्ण *charset,
			    ध्रुव वर्ण *document_charset,
			    संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	nserror res;
	dom_exception घातांकः; /* the result from DOM operations */
	dom_string *inputvalue;

	घातांकः = dom_html_निवेशः_element_get_मानम्(निवेशः_element, &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		प्रत्यागम NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_दत्त_list_सङ्कलय(inputname,
				  inputvalue,
				  रिक्त,
				  charset,
				  document_charset,
				  fetch_दत्त_अग्र_ptr);

	dom_string_unref(inputvalue);

	प्रत्यागम res;
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
form_dom_गन्तव्यम्_दत्त_निवेशः(dom_html_निवेशः_element *निवेशः_element,
		       ध्रुव वर्ण *charset,
		       ध्रुव वर्ण *document_charset,
		       dom_html_element **submit_button,
		       संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	dom_exception घातांकः; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	घातांकः = dom_html_निवेशः_element_get_disabled(निवेशः_element,
						  &element_disabled);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (element_disabled) {
		/* disabled element requires no more processing */
		प्रत्यागम NSERROR_OK;
	}

	/* obtain name property */
	घातांकः = dom_html_निवेशः_element_get_name(निवेशः_element, &inputname);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (inputname == रिक्त) {
		/* element with no name is not converted */
		प्रत्यागम NSERROR_OK;
	}

	/* get input type */
	घातांकः = dom_html_निवेशः_element_get_type(निवेशः_element, &inputtype);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		प्रत्यागम NSERROR_DOM;
	}

	/* process according to input element type */
	यदि (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_गन्तव्यम्_दत्त_निवेशः_submit(निवेशः_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_दत्त_अग्र_ptr);

	} अन्यथा यदि (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_गन्तव्यम्_दत्त_निवेशः_image(निवेशः_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_दत्त_अग्र_ptr);

	} अन्यथा यदि (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_गन्तव्यम्_दत्त_निवेशः_checkbox(निवेशः_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_दत्त_अग्र_ptr);

	} अन्यथा यदि (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_गन्तव्यम्_दत्त_निवेशः_file(निवेशः_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_दत्त_अग्र_ptr);

	} अन्यथा यदि (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} अन्यथा {
		/* Everything else is treated as text values */
		res = form_dom_गन्तव्यम्_दत्त_निवेशः_text(निवेशः_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_दत्त_अग्र_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	प्रत्यागम res;
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
form_dom_गन्तव्यम्_दत्त_button(dom_html_button_element *button_element,
			ध्रुव वर्ण *form_charset,
			ध्रुव वर्ण *doc_charset,
			dom_html_element **submit_button,
			संरचना fetch_multipart_दत्त ***fetch_दत्त_अग्र_ptr)
{
	dom_exception घातांकः; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	घातांकः = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (element_disabled) {
		/* allow enumeration to continue after disabled element */
		प्रत्यागम NSERROR_OK;
	}

	/* get the type attribute */
	घातांकः = dom_html_button_element_get_type(button_element, &inputtype);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		प्रत्यागम NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	यदि (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		प्रत्यागम NSERROR_OK;
	}
	यदि (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		प्रत्यागम NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	यदि (*submit_button == रिक्त) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	यदि (*submit_button != (dom_html_element *)button_element) {
		प्रत्यागम NSERROR_OK;
	}

	/* obtain name property */
	घातांकः = dom_html_button_element_get_name(button_element, &inputname);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", घातांकः);
		प्रत्यागम NSERROR_DOM;
	}

	यदि (inputname == रिक्त) {
		/* allow enumeration to continue after element with no name */
		प्रत्यागम NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	घातांकः = dom_html_button_element_get_मानम्(button_element, &inputvalue);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} अन्यथा {
		res = fetch_दत्त_list_सङ्कलय(inputname,
					  inputvalue,
					  रिक्त,
					  form_charset,
					  doc_charset,
					  fetch_दत्त_अग्र_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	प्रत्यागम res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
स्थिर वर्ण *form_acceptable_charset(संरचना form *form)
{
	वर्ण *अस्थायी, *c;

	यदि (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		यदि (form->document_charset) {
			/* document charset present, so use it */
			प्रत्यागम strdup(form->document_charset);
		} अन्यथा {
			/* no document charset, so default to 8859-1 */
			प्रत्यागम strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	अस्थायी = strdup(form->accept_charsets);
	यदि (!अस्थायी)
		प्रत्यागम रिक्त;

	/* make it upper case */
	प्रति (c = अस्थायी; *c; c++) {
		*c = ascii_गन्तव्यम्_ऊर्ध्वस्थम्(*c);
	}

	/* is UTF-8 specified? */
	c = वाक्ये_अन्विष(अस्थायी, "UTF-8");
	यदि (c) {
		मुक्त(अस्थायी);
		प्रत्यागम strdup("UTF-8");
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
	यावत् (*c && !ascii_is_space(*c)) {
		यदि (*c == ',')
			भङ्ग;
		c++;
	}

	प्रत्यागम strndup(form->accept_charsets, c - form->accept_charsets);
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
form_dom_गन्तव्यम्_दत्त(संरचना form *form,
		 संरचना form_control *submit_control,
		 संरचना fetch_multipart_दत्त **fetch_दत्त_बहिर्गमनम्)
{
	nserror res = NSERROR_OK;
	वर्ण *charset; /* form characterset */
	dom_exception घातांकः; /* the result from DOM operations */
	dom_html_collection *elements = रिक्त; /* the dom form elements */
	uint32_t element_गणनम्; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ग्रन्थिः *element = रिक्त; /* the DOM form element */
	dom_string *nodename = रिक्त; /* the DOM node name of the element */
	संरचना fetch_multipart_दत्त *fetch_दत्त = रिक्त; /* fetch data list */
	संरचना fetch_multipart_दत्त **fetch_दत्त_अग्र = &fetch_दत्त;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	यदि (submit_control != रिक्त) {
		submit_button = submit_control->ग्रन्थिः;
	} अन्यथा {
		submit_button = रिक्त;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	यदि (charset == रिक्त) {
		NSLOG(netsurf, INFO, "failed to find charset");
		प्रत्यागम NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	घातांकः = dom_html_form_element_get_elements(form->ग्रन्थिः, &elements);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		मुक्त(charset);
		प्रत्यागम NSERROR_DOM;
	}

	घातांकः = dom_html_collection_get_दीर्घता(elements, &element_गणनम्);
	यदि (घातांकः != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		गच्छ form_dom_गन्तव्यम्_दत्त_error;
	}

	प्रति (element_idx = 0; element_idx < element_गणनम्; element_idx++) {
		/* obtain a form element */
		घातांकः = dom_html_collection_item(elements, element_idx, &element);
		यदि (घातांकः != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, घातांकः);
			res = NSERROR_DOM;
			गच्छ form_dom_गन्तव्यम्_दत्त_error;
		}

		/* node name from element */
		घातांकः = dom_ग्रन्थिः_get_ग्रन्थिः_name(element, &nodename);
		यदि (घातांकः != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, घातांकः);
			dom_ग्रन्थिः_unref(element);
			res = NSERROR_DOM;
			गच्छ form_dom_गन्तव्यम्_दत्त_error;
		}

		यदि (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_गन्तव्यम्_दत्त_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_दत्त_अग्र);

		} अन्यथा यदि (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_गन्तव्यम्_दत्त_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_दत्त_अग्र);

		} अन्यथा यदि (dom_string_isequal(nodename, corestring_dom_निवेशः)) {
			/* Form element is HTMLInputElement */
			res = form_dom_गन्तव्यम्_दत्त_निवेशः(
				(dom_html_निवेशः_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_दत्त_अग्र);

		} अन्यथा यदि (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_गन्तव्यम्_दत्त_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_दत्त_अग्र);

		} अन्यथा {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (पूर्णाङ्क)dom_string_byte_दीर्घता(nodename),
			      dom_string_दत्त(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ग्रन्थिः_unref(element);

		/* abort form element enumeration on error */
		यदि (res != NSERROR_OK) {
			गच्छ form_dom_गन्तव्यम्_दत्त_error;
		}
	}

	*fetch_दत्त_बहिर्गमनम् = fetch_दत्त;
	dom_html_collection_unref(elements);
	मुक्त(charset);

	प्रत्यागम NSERROR_OK;

form_dom_गन्तव्यम्_दत्त_error:
	fetch_multipart_दत्त_destroy(fetch_दत्त);
	dom_html_collection_unref(elements);
	मुक्त(charset);

	प्रत्यागम res;
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
		संरचना fetch_multipart_दत्त *control,
		वर्ण **encoded_बहिर्गमनम्)
{
	वर्ण *name, *मानम्;
	वर्ण *s, *s2;
	अचिह्नित पूर्णाङ्क len, len1, len_init;
	nserror res;

	s = स्मृतिं_देहि(1);

	यदि (s == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	प्रति (; control; control = control->अग्र) {
		res = url_escape(control->name, true, रिक्त, &name);
		यदि (res != NSERROR_OK) {
			मुक्त(s);
			प्रत्यागम res;
		}

		res = url_escape(control->मानम्, true, रिक्त, &मानम्);
		यदि (res != NSERROR_OK) {
			मुक्त(name);
			मुक्त(s);
			प्रत्यागम res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + वाक्यदैर्घ्यम्(name) + वाक्यदैर्घ्यम्(मानम्) + 2;
		s2 = realloc(s, len1 + 1);
		यदि (s2 == रिक्त) {
			मुक्त(मानम्);
			मुक्त(name);
			मुक्त(s);
			प्रत्यागम NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, मानम्);
		len = len1;
		मुक्त(name);
		मुक्त(मानम्);
	}

	यदि (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_बहिर्गमनम् = s;

	प्रत्यागम NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
स्थिर शून्य
form_select_menu_scroll_callback(शून्य *client_दत्त,
				 संरचना scrollbar_सन्देशः_दत्त *scrollbar_दत्त)
{
	संरचना form_control *control = client_दत्त;
	संरचना form_select_menu *menu = control->दत्त.select.menu;
	html_content *html = (html_content *)menu->c;

	विकल्प (scrollbar_दत्त->सन्देशः) {
		पक्ष SCROLLBAR_सन्देशः_MOVED:
			menu->callback(menu->client_दत्त,
					0, 0,
					menu->विस्तारः,
					menu->height);
			भङ्ग;
		पक्ष SCROLLBAR_सन्देशः_SCROLL_START:
		{
			संरचना rect rect = {
				.x0 = scrollbar_दत्त->x0,
				.y0 = scrollbar_दत्त->y0,
				.x1 = scrollbar_दत्त->x1,
				.y1 = scrollbar_दत्त->y1
			};

			browser_गवाक्षः_समुच्चयः_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			भङ्ग;
		पक्ष SCROLLBAR_सन्देशः_SCROLL_परिसमाप्तम्:
			menu->scroll_capture = false;

			browser_गवाक्षः_समुच्चयः_drag_type(html->bw,
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

	प्रति (गणनम् = 0, o = control->दत्त.select.वस्तूनि;
			o != रिक्त;
			गणनम्++, o = o->अग्र) {
		यदि (!control->दत्त.select.गुणजः && o->selected) {
			o->selected = false;
			dom_html_option_element_समुच्चयः_selected(o->ग्रन्थिः, false);
		}

		यदि (गणनम् == item) {
			यदि (control->दत्त.select.गुणजः) {
				यदि (o->selected) {
					o->selected = false;
					dom_html_option_element_समुच्चयः_selected(
							o->ग्रन्थिः, false);
					control->दत्त.select.अङ्कः_selected--;
				} अन्यथा {
					o->selected = true;
					dom_html_option_element_समुच्चयः_selected(
							o->ग्रन्थिः, true);
					control->दत्त.select.अङ्कः_selected++;
				}
			} अन्यथा {
				dom_html_option_element_समुच्चयः_selected(
						o->ग्रन्थिः, true);
				o->selected = true;
			}
		}

		यदि (o->selected) {
			control->दत्त.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	यदि (control->दत्त.select.अङ्कः_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} अन्यथा यदि (control->दत्त.select.अङ्कः_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->दत्त.select.current->text);
	} अन्यथा {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	यदि (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->दीर्घता = 0;
	} अन्यथा {
		inline_box->दीर्घता = वाक्यदैर्घ्यम्(inline_box->text);
	}
	inline_box->विस्तारः = control->box->विस्तारः;

	html__redraw_a_box(html, control->box);

	प्रत्यागम ret;
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
	संरचना form_select_menu *menu = control->दत्त.select.menu;
	संरचना form_option *option;
	html_content *html = (html_content *)menu->c;
	पूर्णाङ्क पङ्क्तिः_height, पङ्क्तिः_height_with_spacing;
	पूर्णाङ्क item_bottom_y;
	पूर्णाङ्क scroll, i;

	scroll = scrollbar_get_विस्थापनम्(menu->scrollbar);

	पङ्क्तिः_height = menu->पङ्क्तिः_height;
	पङ्क्तिः_height_with_spacing = पङ्क्तिः_height +
			पङ्क्तिः_height * SELECT_पङ्क्तिः_SPACING;

	option = control->दत्त.select.वस्तूनि;
	item_bottom_y = पङ्क्तिः_height_with_spacing;
	i = 0;
	यावत् (option && item_bottom_y < scroll + y) {
		item_bottom_y += पङ्क्तिः_height_with_spacing;
		option = option->अग्र;
		i++;
	}

	यदि (option != रिक्त) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_दत्त, 0, 0, menu->विस्तारः, menu->height);
}


/* exported interface documented in html/form_internal.h */
शून्य form_सङ्कलय_control(संरचना form *form, संरचना form_control *control)
{
	यदि (form == रिक्त) {
		प्रत्यागम;
	}

	control->form = form;

	यदि (form->controls != रिक्त) {
		प्रतिज्ञा(form->चरमः_control);

		form->चरमः_control->अग्र = control;
		control->पूर्व = form->चरमः_control;
		control->अग्र = रिक्त;
		form->चरमः_control = control;
	} अन्यथा {
		form->controls = form->चरमः_control = control;
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
	यदि (control->चरमः_synced_मानम् != रिक्त) {
		मुक्त(control->चरमः_synced_मानम्);
	}

	यदि (control->type == GADGET_SELECT) {
		संरचना form_option *option, *अग्र;

		प्रति (option = control->दत्त.select.वस्तूनि; option;
				option = अग्र) {
			अग्र = option->अग्र;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->मानम्);
			मुक्त(option->text);
			मुक्त(option->मानम्);
			मुक्त(option);
		}
		यदि (control->दत्त.select.menu != रिक्त) {
			form_free_select_menu(control);
		}
	}

	यदि (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		यदि (control->दत्त.text.initial != रिक्त) {
			dom_string_unref(control->दत्त.text.initial);
		}

		यदि (control->दत्त.text.ta != रिक्त) {
			textarea_destroy(control->दत्त.text.ta);
		}
	}

	/* unlink the control from the form */
	यदि (control->form != रिक्त) {
		प्रति (c = control->form->controls; c != रिक्त; c = c->अग्र) {
			यदि (c->अग्र == control) {
				c->अग्र = control->अग्र;
				यदि (control->form->चरमः_control == control)
					control->form->चरमः_control = c;
				भङ्ग;
			}
			यदि (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->अग्र;
				यदि (control->form->चरमः_control == control)
					control->form->controls =
						control->form->चरमः_control = रिक्त;
				भङ्ग;
			}
		}
	}

	यदि (control->ग्रन्थिः_मानम् != रिक्त) {
		dom_string_unref(control->ग्रन्थिः_मानम्);
	}

	मुक्त(control);
}


/* exported interface documented in html/form_internal.h */
bool form_सङ्कलय_option(संरचना form_control *control, वर्ण *मानम्, वर्ण *text,
		     bool selected, शून्य *ग्रन्थिः)
{
	संरचना form_option *option;

	प्रतिज्ञा(control);
	प्रतिज्ञा(control->type == GADGET_SELECT);

	option = शून्यस्मृतिं_देहि(1, परिमाण *option);
	यदि (!option)
		प्रत्यागम false;

	option->मानम् = मानम्;
	option->text = text;

	/* add to linked list */
	यदि (control->दत्त.select.वस्तूनि == 0)
		control->दत्त.select.वस्तूनि = option;
	अन्यथा
		control->दत्त.select.चरमः_item->अग्र = option;
	control->दत्त.select.चरमः_item = option;

	/* set selected */
	यदि (selected && (control->दत्त.select.अङ्कः_selected == 0 ||
			control->दत्त.select.गुणजः)) {
		option->selected = option->initial_selected = true;
		control->दत्त.select.अङ्कः_selected++;
		control->दत्त.select.current = option;
	}

	control->दत्त.select.अङ्कः_वस्तूनि++;

	option->ग्रन्थिः = ग्रन्थिः;

	प्रत्यागम true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(शून्य *client_दत्त,
		      संरचना form_control *control,
		      select_menu_redraw_callback callback,
		      संरचना content *c)
{
	पूर्णाङ्क पङ्क्तिः_height_with_spacing;
	संरचना box *box;
	plot_font_style_t fstyle;
	पूर्णाङ्क योग_height;
	संरचना form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	यदि (control->दत्त.select.menu == रिक्त) {

		menu = शून्यस्मृतिं_देहि(1, परिमाण (संरचना form_select_menu));
		यदि (menu == रिक्त) {
			प्रत्यागम NSERROR_NOMEM;
		}

		control->दत्त.select.menu = menu;

		box = control->box;

		menu->विस्तारः = box->विस्तारः +
			box->border[दक्षिणः_2].विस्तारः + box->padding[दक्षिणः_2] +
			box->border[वामः_2].विस्तारः + box->padding[वामः_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_आकार = fstyle.आकार;

		menu->पङ्क्तिः_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.आकार / PLOT_STYLE_SCALE)))),
				F_72));

		पङ्क्तिः_height_with_spacing = menu->पङ्क्तिः_height +
				menu->पङ्क्तिः_height *
				SELECT_पङ्क्तिः_SPACING;

		योग_height = control->दत्त.select.अङ्कः_वस्तूनि *
				पङ्क्तिः_height_with_spacing;
		menu->height = योग_height;

		यदि (menu->height > MAX_SELECT_HEIGHT) {
			menu->height = MAX_SELECT_HEIGHT;
		}

		menu->client_दत्त = client_दत्त;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->height,
				       योग_height,
				       menu->height,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		यदि (res != NSERROR_OK) {
			control->दत्त.select.menu = रिक्त;
			मुक्त(menu);
			प्रत्यागम res;
		}
		menu->c = c;
	} अन्यथा {
		menu = control->दत्त.select.menu;
	}

	menu->callback(client_दत्त, 0, 0, menu->विस्तारः, menu->height);

	प्रत्यागम NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free_select_menu(संरचना form_control *control)
{
	यदि (control->दत्त.select.menu->scrollbar != रिक्त)
		scrollbar_destroy(control->दत्त.select.menu->scrollbar);
	मुक्त(control->दत्त.select.menu);
	control->दत्त.select.menu = रिक्त;
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
	संरचना form_select_menu *menu = control->दत्त.select.menu;
	संरचना form_option *option;
	पूर्णाङ्क पङ्क्तिः_height, पङ्क्तिः_height_with_spacing;
	पूर्णाङ्क विस्तारः, height;
	पूर्णाङ्क x0, y0, x1, scrollbar_x, y1, y2, y3;
	पूर्णाङ्क item_y;
	पूर्णाङ्क text_pos_विस्थापनम्, text_x;
	पूर्णाङ्क scrollbar_विस्तारः = SCROLLBAR_विस्तारः;
	पूर्णाङ्क i;
	पूर्णाङ्क scroll;
	पूर्णाङ्क x_cp, y_cp;
	संरचना rect r;
	संरचना rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	विस्तारः = menu->विस्तारः;
	height = menu->height;
	पङ्क्तिः_height = menu->पङ्क्तिः_height;

	पङ्क्तिः_height_with_spacing = पङ्क्तिः_height +
			पङ्क्तिः_height * SELECT_पङ्क्तिः_SPACING;
	scroll = scrollbar_get_विस्थापनम्(menu->scrollbar);

	यदि (scale != 1.0) {
		x *= scale;
		y *= scale;
		विस्तारः *= scale;
		height *= scale;
		scrollbar_विस्तारः *= scale;

		i = scroll / पङ्क्तिः_height_with_spacing;
		scroll -= i * पङ्क्तिः_height_with_spacing;
		पङ्क्तिः_height *= scale;
		पङ्क्तिः_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * पङ्क्तिः_height_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + विस्तारः - 1;
	y1 = y + height - 1;
	scrollbar_x = x1 - scrollbar_विस्तारः;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	x0 = x0 + SELECT_BORDER_विस्तारः;
	y0 = y0 + SELECT_BORDER_विस्तारः;
	x1 = x1 - SELECT_BORDER_विस्तारः;
	y1 = y1 - SELECT_BORDER_विस्तारः;
	height = height - 2 * SELECT_BORDER_विस्तारः;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	option = control->दत्त.select.वस्तूनि;
	item_y = पङ्क्तिः_height_with_spacing;

	यावत् (item_y < scroll) {
		option = option->अग्र;
		item_y += पङ्क्तिः_height_with_spacing;
	}
	item_y -= पङ्क्तिः_height_with_spacing;
	text_pos_विस्थापनम् = y - scroll +
			(पूर्णाङ्क) (पङ्क्तिः_height * (0.75 + SELECT_पङ्क्तिः_SPACING));
	text_x = x + (box->border[वामः_2].विस्तारः + box->padding[वामः_2]) * scale;

	plot_fstyle_प्रविष्टिः.आकार = menu->f_आकार;

	यावत् (option && item_y - scroll < height) {

		यदि (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + पङ्क्तिः_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			यदि (res != NSERROR_OK) {
				प्रत्यागम false;
			}
		}

		y2 = text_pos_विस्थापनम् + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_प्रविष्टिः,
				      text_x, y2,
				      option->text, वाक्यदैर्घ्यम्(option->text));
		यदि (res != NSERROR_OK) {
			प्रत्यागम false;
		}

		item_y += पङ्क्तिः_height_with_spacing;
		option = option->अग्र;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->विस्तारः - SCROLLBAR_विस्तारः,
			       y_cp,
			       clip, scale, ctx);
	यदि (res != NSERROR_OK) {
		प्रत्यागम false;
	}

	प्रत्यागम true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(संरचना form_control *control,
			     भिन्न scale,
			     ध्रुव संरचना rect *clip)
{
	संरचना form_select_menu *menu = control->दत्त.select.menu;
	पूर्णाङ्क विस्तारः, height;


	विस्तारः = menu->विस्तारः;
	height = menu->height;

	यदि (scale != 1.0) {
		विस्तारः *= scale;
		height *= scale;
	}

	यदि (clip->x0 >= 0 &&
	    clip->x1 <= विस्तारः &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		प्रत्यागम true;

	प्रत्यागम false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(संरचना form_control *control, पूर्णाङ्क item)
{
	प्रतिज्ञा(control != रिक्त);

	प्रत्यागम form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
संरचना form_option *
form_select_get_option(संरचना form_control *control, पूर्णाङ्क item)
{
	संरचना form_option *opt;

	opt = control->दत्त.select.वस्तूनि;
	यावत् ((opt != रिक्त) && (item > 0)) {
		opt = opt->अग्र;
		item--;
	}
	प्रत्यागम opt;
}


/* exported interface documented in netsurf/form.h */
वर्ण *form_control_get_name(संरचना form_control *control)
{
	प्रत्यागम control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(संरचना form_control *control, संरचना rect *r)
{
	box_bounds( control->box, r );
	प्रत्यागम NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ध्रुव वर्ण *
form_select_mouse_action(संरचना form_control *control,
			 browser_mouse_state mouse,
			 पूर्णाङ्क x, पूर्णाङ्क y)
{
	संरचना form_select_menu *menu = control->दत्त.select.menu;
	पूर्णाङ्क x0, y0, x1, y1, scrollbar_x;
	ध्रुव वर्ण *स्थितिः = रिक्त;
	bool गुणजः = control->दत्त.select.गुणजः;

	x0 = 0;
	y0 = 0;
	x1 = menu->विस्तारः;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_विस्तारः;

	यदि (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		प्रत्यागम scrollbar_mouse_स्थितिः_गन्तव्यम्_सन्देशः(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	यदि (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		यदि (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		यदि (!(mouse & BROWSER_MOUSE_CLICK_1 && !गुणजः))
			/* anything but a button 1 click over a single select
			   menu */
			स्थितिः = messages_get(control->दत्त.select.गुणजः ?
					"SelectMClick" : "SelectClick");

	} अन्यथा यदि (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		स्थितिः = messages_get("SelectClose");

	प्रत्यागम स्थितिः;
}


/* private interface described in html/form_internal.h */
शून्य
form_select_mouse_drag_अन्तः(संरचना form_control *control,
			   browser_mouse_state mouse,
			   पूर्णाङ्क x, पूर्णाङ्क y)
{
	पूर्णाङ्क x0, y0, x1, y1;
	पूर्णाङ्क box_x, box_y;
	संरचना box *box;
	संरचना form_select_menu *menu = control->दत्त.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[वामः_2].विस्तारः;
	box_y += box->height + box->border[BOTTOM].विस्तारः +
			box->padding[BOTTOM] + box->padding[शिखरम्_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	यदि (menu->scroll_capture) {
		x -= menu->विस्तारः - SCROLLBAR_विस्तारः;
		scrollbar_mouse_drag_अन्तः(menu->scrollbar, mouse, x, y);
		प्रत्यागम;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->विस्तारः;
	y1 = menu->height;


	यदि (x > x0 && x < x1 - SCROLLBAR_विस्तारः && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
शून्य form_select_get_dimensions(संरचना form_control *control,
		पूर्णाङ्क *विस्तारः, पूर्णाङ्क *height)
{
	*विस्तारः = control->दत्त.select.menu->विस्तारः;
	*height = control->दत्त.select.menu->height;
}


/* private interface described in html/form_internal.h */
शून्य form_select_menu_callback(शून्य *client_दत्त,
		पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क विस्तारः, पूर्णाङ्क height)
{
	html_content *html = client_दत्त;
	पूर्णाङ्क menu_x, menu_y;
	संरचना box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[वामः_2].विस्तारः;
	menu_y += box->height + box->border[BOTTOM].विस्तारः +
			box->padding[BOTTOM] +
			box->padding[शिखरम्_2];
	content__request_redraw((संरचना content *)html, menu_x + x, menu_y + y,
			विस्तारः, height);
}


/* private interface described in html/form_internal.h */
शून्य form_radio_समुच्चयः(संरचना form_control *radio)
{
	संरचना form_control *control;

	प्रतिज्ञा(radio);
	यदि (!radio->form)
		प्रत्यागम;

	यदि (radio->selected)
		प्रत्यागम;

	/* Clear selected state for other controls in
	 * the same radio button group */
	प्रति (control = radio->form->controls;
	     control != रिक्त;
	     control = control->अग्र) {
		/* Only interested in radio inputs */
		यदि (control->type != GADGET_RADIO)
			अनुवर्त;

		/* Ignore ourself */
		यदि (control == radio)
			अनुवर्त;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		यदि ((control->name == रिक्त) ||
		    (radio->name == रिक्त) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    वाक्यं_तुलय(control->name, radio->name) != 0)
			अनुवर्त;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		यदि (control->selected) {
			control->selected = false;
			dom_html_निवेशः_element_समुच्चयः_checked(control->ग्रन्थिः, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_निवेशः_element_समुच्चयः_checked(radio->ग्रन्थिः, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    संरचना browser_गवाक्षः *target,
	    संरचना form *form,
	    संरचना form_control *submit_button)
{
	nserror res;
	वर्ण *दत्त = रिक्त; /* encoded form data */
	संरचना fetch_multipart_दत्त *success = रिक्त; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	प्रतिज्ञा(form != रिक्त);

	/* obtain list of controls from DOM */
	res = form_dom_गन्तव्यम्_दत्त(form, submit_button, &success);
	यदि (res != NSERROR_OK) {
		प्रत्यागम res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	यदि (res != NSERROR_OK) {
		fetch_multipart_दत्त_destroy(success);
		प्रत्यागम res;
	}

	विकल्प (form->विधानम्) {
	पक्ष विधानम्_GET:
		res = form_url_encode(form, success, &दत्त);
		यदि (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, दत्त, &query_url);
			यदि (res == NSERROR_OK) {
				res = browser_गवाक्षः_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      रिक्त,
							      रिक्त,
							      रिक्त);

				nsurl_unref(query_url);
			}
			मुक्त(दत्त);
		}
		भङ्ग;

	पक्ष विधानम्_POST_URLENC:
		res = form_url_encode(form, success, &दत्त);
		यदि (res == NSERROR_OK) {
			res = browser_गवाक्षः_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      दत्त,
						      रिक्त,
						      रिक्त);
			मुक्त(दत्त);
		}
		भङ्ग;

	पक्ष विधानम्_POST_MULTIPART:
		res = browser_गवाक्षः_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      रिक्त,
					      success,
					      रिक्त);

		भङ्ग;
	}

	nsurl_unref(action_url);
	fetch_multipart_दत्त_destroy(success);

	प्रत्यागम res;
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
		यदि (control->मानम् != रिक्त) {
			मुक्त(control->मानम्);
		}
		control->मानम् = मानम्;
		यदि (control->ग्रन्थिः != रिक्त) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)मानम्,
						वाक्यदैर्घ्यम्(मानम्), &str);
			यदि (err == DOM_NO_ERR) {
				यदि (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_समुच्चयः_मानम्(
						(dom_html_text_area_element *)(control->ग्रन्थिः),
						str);
				अन्यथा
					err = dom_html_निवेशः_element_समुच्चयः_मानम्(
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
	bool परिवर्तितम्_dom = false;

	यदि (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		प्रत्यागम;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	यदि ((control->चरमः_synced_मानम् == रिक्त &&
	     control->मानम् != रिक्त &&
	     control->मानम्[0] != '\0') ||
	    (control->चरमः_synced_मानम् != रिक्त &&
	     control->मानम् != रिक्त &&
	     वाक्यं_तुलय(control->मानम्, control->चरमः_synced_मानम्) != 0)) {
		वर्ण *dup = strdup(control->मानम्);
		यदि (dup == रिक्त) {
			गच्छ बहिर्गमनम्;
		}
		यदि (control->चरमः_synced_मानम् != रिक्त) {
			मुक्त(control->चरमः_synced_मानम्);
		}
		control->चरमः_synced_मानम् = dup;
		exc = dom_string_create((uint8_t *)(control->मानम्),
					वाक्यदैर्घ्यम्(control->मानम्), &मानम्);
		यदि (exc != DOM_NO_ERR) {
			गच्छ बहिर्गमनम्;
		}
		यदि (control->ग्रन्थिः_मानम् != रिक्त) {
			dom_string_unref(control->ग्रन्थिः_मानम्);
		}
		control->ग्रन्थिः_मानम् = मानम्;
		मानम् = रिक्त;
		यदि (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_समुच्चयः_मानम्(control->ग्रन्थिः, control->ग्रन्थिः_मानम्);
		} अन्यथा {
			exc = dom_html_निवेशः_element_समुच्चयः_मानम्(control->ग्रन्थिः, control->ग्रन्थिः_मानम्);
		}
		यदि (exc != DOM_NO_ERR) {
			गच्छ बहिर्गमनम्;
		}
		परिवर्तितम्_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	यदि (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_मानम्(control->ग्रन्थिः, &मानम्);
	} अन्यथा {
		exc = dom_html_निवेशः_element_get_मानम्(control->ग्रन्थिः, &मानम्);
	}

	यदि (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		गच्छ बहिर्गमनम्;
	}

	यदि (!dom_string_isequal(control->ग्रन्थिः_मानम्, मानम्)) {
		/* The DOM has changed */
		यदि (!परिवर्तितम्_dom) {
			/* And it wasn't us */
			वर्ण *मानम्_s = strndup(
				dom_string_दत्त(मानम्),
				dom_string_byte_दीर्घता(मानम्));
			वर्ण *dup = रिक्त;
			यदि (मानम्_s == रिक्त) {
				गच्छ बहिर्गमनम्;
			}
			dup = strdup(मानम्_s);
			यदि (dup == रिक्त) {
				मुक्त(मानम्_s);
				गच्छ बहिर्गमनम्;
			}
			मुक्त(control->मानम्);
			control->मानम् = मानम्_s;
			मुक्त(control->चरमः_synced_मानम्);
			control->चरमः_synced_मानम् = dup;
			यदि (control->type != GADGET_HIDDEN &&
			    control->दत्त.text.ta != रिक्त) {
				textarea_समुच्चयः_text(control->दत्त.text.ta,
						  मानम्_s);
			}
		}
		control->ग्रन्थिः_मानम् = मानम्;
		मानम् = रिक्त;
	}

बहिर्गमनम्:
	यदि (मानम् != रिक्त)
		dom_string_unref(मानम्);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
संरचना form *
form_new(शून्य *ग्रन्थिः,
	 ध्रुव वर्ण *action,
	 ध्रुव वर्ण *target,
	 form_विधानम् विधानम्,
	 ध्रुव वर्ण *charset,
	 ध्रुव वर्ण *doc_charset)
{
	संरचना form *form;

	form = शून्यस्मृतिं_देहि(1, परिमाण *form);
	यदि (!form)
		प्रत्यागम रिक्त;

	form->action = strdup(action != रिक्त ? action : "");
	यदि (form->action == रिक्त) {
		मुक्त(form);
		प्रत्यागम रिक्त;
	}

	form->target = target != रिक्त ? strdup(target) : रिक्त;
	यदि (target != रिक्त && form->target == रिक्त) {
		मुक्त(form->action);
		मुक्त(form);
		प्रत्यागम रिक्त;
	}

	form->विधानम् = विधानम्;

	form->accept_charsets = charset != रिक्त ? strdup(charset) : रिक्त;
	यदि (charset != रिक्त && form->accept_charsets == रिक्त) {
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		प्रत्यागम रिक्त;
	}

	form->document_charset = doc_charset != रिक्त ? strdup(doc_charset)
						     : रिक्त;
	यदि (doc_charset && form->document_charset == रिक्त) {
		मुक्त(form->accept_charsets);
		मुक्त(form->target);
		मुक्त(form->action);
		मुक्त(form);
		प्रत्यागम रिक्त;
	}

	form->ग्रन्थिः = ग्रन्थिः;

	प्रत्यागम form;
}


/* exported interface documented in html/form_internal.h */
शून्य form_free(संरचना form *form)
{
	संरचना form_control *c, *d;

	प्रति (c = form->controls; c != रिक्त; c = d) {
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
संरचना form_control *form_new_control(शून्य *ग्रन्थिः, form_control_type type)
{
	संरचना form_control *control;

	control = शून्यस्मृतिं_देहि(1, परिमाण *control);
	यदि (control == रिक्त)
		प्रत्यागम रिक्त;

	control->ग्रन्थिः = ग्रन्थिः;
	control->type = type;

	प्रत्यागम control;
}
