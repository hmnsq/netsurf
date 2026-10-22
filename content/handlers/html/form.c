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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "utils/url.h"
#অন্তর্ভাব "utils/utf8.h"
#অন্তর্ভাব "utils/ascii.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/inttypes.h"
#অন্তর্ভাব "netsurf/mouse.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "content/fetch.h"
#অন্তর্ভাব "content/hlcache.h"
#অন্তর্ভাব "css/utils.h"
#অন্তর্ভাব "desktop/knockout.h"
#অন্তর্ভাব "desktop/scrollbar.h"
#অন্তর্ভাব "desktop/textarea.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/layout.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/form_internal.h"

#define MAX_SELECT_HEIGHT 210
#define SELECT_সারি_SPACING 0.2
#define SELECT_BORDER_WIDTH 1
#define SELECT_SELECTED_COLOUR 0xDB9370

কাঠামো form_select_menu {
	পূর্ণসংখ্যা সারি_height;
	পূর্ণসংখ্যা width, height;
	কাঠামো scrollbar *scrollbar;
	পূর্ণসংখ্যা f_আকার;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	শূন্য *client_তথ্য;
	কাঠামো content *c;
};

স্থির plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_colour = SELECT_SELECTED_COLOUR,
};

স্থির plot_font_style_t plot_fstyle_entry = {
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
স্থির অক্ষর *
form_encode_item(ধ্রুবক অক্ষর *item,
		 uint32_t len,
		 ধ্রুবক অক্ষর *charset,
		 ধ্রুবক অক্ষর *fallback)
{
	nserror err;
	অক্ষর *ret = ফাঁকা;
	অক্ষর cset[256];

	যদি (!item || !charset)
		ফেরত ফাঁকা;

	snprintf(cset, আকার cset, "%s//TRANSLIT", charset);

	err = utf8_to_enc(item, cset, 0, &ret);
	যদি (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, আকার cset, "%s", charset);
		err = utf8_to_enc(item, cset, len, &ret);

		যদি (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			যদি (fallback) {
				snprintf(cset, আকার cset,
						"%s//TRANSLIT", fallback);
				err = utf8_to_enc(item, cset, 0, &ret);

				যদি (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, আকার cset,
							"%s", fallback);
					err = utf8_to_enc(item, cset, 0, &ret);
				}
			}

			যদি (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_to_enc(item, "ISO-8859-1//TRANSLIT",
						0, &ret);
				যদি (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_to_enc(item, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	যদি (err == NSERROR_NOMEM) {
		ফেরত ফাঁকা;
	}

	ফেরত ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#define FETCH_তথ্য_INT_মান_আকার 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
স্থির nserror
fetch_তথ্য_list_add_sname(ধ্রুবক অক্ষর *name,
			  ধ্রুবক অক্ষর *ksfx,
			  পূর্ণসংখ্যা মান,
			  কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	কাঠামো fetch_multipart_তথ্য *fetch_তথ্য;
	পূর্ণসংখ্যা keysize;

	fetch_তথ্য = calloc(1, আকার(*fetch_তথ্য));
	যদি (fetch_তথ্য == ফাঁকা) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ফেরত NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_তথ্য->name, 0, "%s%s", name, ksfx);
	fetch_তথ্য->name = malloc(keysize + 1); /* allow for null */
	যদি (fetch_তথ্য->name == ফাঁকা) {
		মুক্ত(fetch_তথ্য);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ফেরত NSERROR_NOMEM;
	}
	snprintf(fetch_তথ্য->name, keysize + 1, "%s%s", name, ksfx);

	/*মান*/
	fetch_তথ্য->মান = malloc(FETCH_তথ্য_INT_মান_আকার);
	যদি (fetch_তথ্য->মান == ফাঁকা) {
		মুক্ত(fetch_তথ্য->name);
		মুক্ত(fetch_তথ্য);
		NSLOG(netsurf, INFO, "value allocation failure");
		ফেরত NSERROR_NOMEM;
	}
	snprintf(fetch_তথ্য->মান, FETCH_তথ্য_INT_মান_আকার, "%d", মান);

	/* link into list */
	**fetch_তথ্য_পরবর্তী_ptr = fetch_তথ্য;
	*fetch_তথ্য_পরবর্তী_ptr = &fetch_তথ্য->পরবর্তী;

	ফেরত NSERROR_OK;
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
স্থির nserror
fetch_তথ্য_list_add(dom_string *name,
		    dom_string *মান,
		    ধ্রুবক অক্ষর *rawfile,
		    ধ্রুবক অক্ষর *form_charset,
		    ধ্রুবক অক্ষর *docu_charset,
		    কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	কাঠামো fetch_multipart_তথ্য *fetch_তথ্য;

	প্রতিজ্ঞা(name != ফাঁকা);

	fetch_তথ্য = calloc(1, আকার(*fetch_তথ্য));
	যদি (fetch_তথ্য == ফাঁকা) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ফেরত NSERROR_NOMEM;
	}

	fetch_তথ্য->name = form_encode_item(dom_string_তথ্য(name),
					    dom_string_byte_দৈর্ঘ্য(name),
					    form_charset,
					    docu_charset);
	যদি (fetch_তথ্য->name == ফাঁকা) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		মুক্ত(fetch_তথ্য);
		ফেরত NSERROR_NOMEM;
	}

	যদি (মান == ফাঁকা) {
		fetch_তথ্য->মান = strdup("");
	} অন্যথা {
		fetch_তথ্য->মান = form_encode_item(dom_string_তথ্য(মান),
						     dom_string_byte_দৈর্ঘ্য(মান),
						     form_charset,
						     docu_charset);
	}
	যদি (fetch_তথ্য->মান == ফাঁকা) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		মুক্ত(fetch_তথ্য->name);
		মুক্ত(fetch_তথ্য);
		ফেরত NSERROR_NOMEM;
	}

	/* deal with raw file name */
	যদি (rawfile != ফাঁকা) {
		fetch_তথ্য->file = true;
		fetch_তথ্য->rawfile = strdup(rawfile);
		যদি (fetch_তথ্য->rawfile == ফাঁকা) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			মুক্ত(fetch_তথ্য->মান);
			মুক্ত(fetch_তথ্য->name);
			মুক্ত(fetch_তথ্য);
			ফেরত NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_তথ্য_পরবর্তী_ptr = fetch_তথ্য;
	*fetch_তথ্য_পরবর্তী_ptr = &fetch_তথ্য->পরবর্তী;

	ফেরত NSERROR_OK;
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
স্থির nserror
form_dom_to_তথ্য_textarea(dom_html_text_area_element *text_area_element,
			  ধ্রুবক অক্ষর *form_charset,
			  ধ্রুবক অক্ষর *doc_charset,
			  কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_text_area_element_get_disabled(text_area_element,
						      &element_disabled);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ফেরত NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_text_area_element_get_name(text_area_element,
						  &inputname);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (inputname == ফাঁকা) {
		/* allow enumeration to continue after element with no name */
		ফেরত NSERROR_OK;
	}

	/* obtain text area value */
	exp = dom_html_text_area_element_get_মান(text_area_element,
						   &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", exp);
		dom_string_unref(inputname);
		ফেরত NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(inputname,
				  inputvalue,
				  ফাঁকা,
				  form_charset,
				  doc_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ধ্রুবক অক্ষর *form_charset,
			      ধ্রুবক অক্ষর *docu_charset,
			      কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *মান;
	bool selected;

	exp = dom_html_option_element_get_selected(option_element, &selected);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ফেরত NSERROR_DOM;
	}

	যদি (!selected) {
		/* unselected options do not add fetch data entries */
		ফেরত NSERROR_OK;
	}

	exp = dom_html_option_element_get_মান(option_element, &মান);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ফেরত NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(keyname,
				  মান,
				  ফাঁকা,
				  form_charset,
				  docu_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(মান);

	ফেরত res;
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
স্থির nserror
form_dom_to_তথ্য_select(dom_html_select_element *select_element,
			ধ্রুবক অক্ষর *form_charset,
			ধ্রুবক অক্ষর *doc_charset,
			কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ফাঁকা;
	uint32_t options_গুনতি;
	uint32_t option_সূচক;
	dom_গ্রন্থি *option_element = ফাঁকা;

	/* check if element is disabled */
	exp = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ফেরত NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_select_element_get_name(select_element, &inputname);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (inputname == ফাঁকা) {
		/* allow enumeration to continue after element with no name */
		ফেরত NSERROR_OK;
	}

	/* get options collection */
	exp = dom_html_select_element_get_options(select_element, &options);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ফেরত NSERROR_DOM;
	}

	/* get options collection length */
	exp = dom_html_options_collection_get_দৈর্ঘ্য(options, &options_গুনতি);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ফেরত NSERROR_DOM;
	}

	/* iterate over options collection */
	জন্য (option_সূচক = 0; option_সূচক < options_গুনতি; ++option_সূচক) {
		exp = dom_html_options_collection_item(options,
						       option_সূচক,
						       &option_element);
		যদি (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_সূচক);
			res = NSERROR_DOM;
		} অন্যথা {
			res = form_dom_to_তথ্য_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_তথ্য_পরবর্তী_ptr);

			dom_গ্রন্থি_unref(option_element);
		}

		যদি (res != NSERROR_OK) {
			বিরতি;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_প্রবেশ_submit(dom_html_প্রবেশ_element *প্রবেশ_element,
			      dom_string *inputname,
			      ধ্রুবক অক্ষর *charset,
			      ধ্রুবক অক্ষর *document_charset,
			      dom_html_element **submit_button,
			      কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	যদি (*submit_button == ফাঁকা) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)প্রবেশ_element;
	} অন্যথা যদি (*submit_button != (dom_html_element *)প্রবেশ_element) {
		ফেরত NSERROR_OK;
	}

	/* matched button used to submit form */
	exp = dom_html_প্রবেশ_element_get_মান(প্রবেশ_element, &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ফেরত NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(inputname,
				  inputvalue,
				  ফাঁকা,
				  charset,
				  document_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(inputvalue);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_প্রবেশ_image(dom_html_প্রবেশ_element *প্রবেশ_element,
			     dom_string *inputname,
			     ধ্রুবক অক্ষর *charset,
			     ধ্রুবক অক্ষর *document_charset,
			     dom_html_element **submit_button,
			     কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	কাঠামো image_প্রবেশ_coords *coords;
	অক্ষর *basename;

	/* Only use an image input if it was the thing which activated us */
	যদি (*submit_button != (dom_html_element *)প্রবেশ_element) {
		ফেরত NSERROR_OK;
	}

	exp = dom_গ্রন্থি_get_user_তথ্য((dom_গ্রন্থি *)প্রবেশ_element,
				     corestring_dom_ns_চাবি_image_coords_গ্রন্থি_তথ্য,
				     &coords);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ফেরত NSERROR_DOM;
	}

	যদি (coords == ফাঁকা) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ফেরত NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_item(dom_string_তথ্য(inputname),
				    dom_string_byte_দৈর্ঘ্য(inputname),
				    charset,
				    document_charset);
	যদি (basename == ফাঁকা) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ফেরত NSERROR_NOMEM;
	}

	res = fetch_তথ্য_list_add_sname(basename, ".x",
					coords->x,
					fetch_তথ্য_পরবর্তী_ptr);

	যদি (res == NSERROR_OK) {
		res = fetch_তথ্য_list_add_sname(basename, ".y",
						coords->y,
						fetch_তথ্য_পরবর্তী_ptr);
	}

	মুক্ত(basename);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_প্রবেশ_checkbox(dom_html_প্রবেশ_element *প্রবেশ_element,
				dom_string *inputname,
				ধ্রুবক অক্ষর *charset,
				ধ্রুবক অক্ষর *document_charset,
				কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	exp = dom_html_প্রবেশ_element_get_checked(প্রবেশ_element, &checked);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ফেরত NSERROR_DOM;
	}

	যদি (!checked) {
		/* unchecked items do not generate a data entry */
		ফেরত NSERROR_OK;
	}

	exp = dom_html_প্রবেশ_element_get_মান(প্রবেশ_element, &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ফেরত NSERROR_DOM;
	}

	/* ensure a default value */
	যদি (inputvalue == ফাঁকা) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(inputname,
				  inputvalue,
				  ফাঁকা,
				  charset,
				  document_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(inputvalue);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_প্রবেশ_file(dom_html_প্রবেশ_element *প্রবেশ_element,
			    dom_string *inputname,
			    ধ্রুবক অক্ষর *charset,
			    ধ্রুবক অক্ষর *document_charset,
			    কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;
	ধ্রুবক অক্ষর *rawfile = ফাঁকা;

	exp = dom_html_প্রবেশ_element_get_মান(প্রবেশ_element, &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ফেরত NSERROR_DOM;
	}

	exp = dom_গ্রন্থি_get_user_তথ্য((dom_গ্রন্থি *)প্রবেশ_element,
				     corestring_dom_ns_চাবি_file_name_গ্রন্থি_তথ্য,
				     &rawfile);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ফেরত NSERROR_DOM;
	}

	যদি (rawfile == ফাঁকা) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(inputvalue);

	ফেরত res;
}


স্থির nserror
form_dom_to_তথ্য_প্রবেশ_text(dom_html_প্রবেশ_element *প্রবেশ_element,
			    dom_string *inputname,
			    ধ্রুবক অক্ষর *charset,
			    ধ্রুবক অক্ষর *document_charset,
			    কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	nserror res;
	dom_exception exp; /* the result from DOM operations */
	dom_string *inputvalue;

	exp = dom_html_প্রবেশ_element_get_মান(প্রবেশ_element, &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ফেরত NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_তথ্য_list_add(inputname,
				  inputvalue,
				  ফাঁকা,
				  charset,
				  document_charset,
				  fetch_তথ্য_পরবর্তী_ptr);

	dom_string_unref(inputvalue);

	ফেরত res;
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
স্থির nserror
form_dom_to_তথ্য_প্রবেশ(dom_html_প্রবেশ_element *প্রবেশ_element,
		       ধ্রুবক অক্ষর *charset,
		       ধ্রুবক অক্ষর *document_charset,
		       dom_html_element **submit_button,
		       কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
{
	dom_exception exp; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	exp = dom_html_প্রবেশ_element_get_disabled(প্রবেশ_element,
						  &element_disabled);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (element_disabled) {
		/* disabled element requires no more processing */
		ফেরত NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_প্রবেশ_element_get_name(প্রবেশ_element, &inputname);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (inputname == ফাঁকা) {
		/* element with no name is not converted */
		ফেরত NSERROR_OK;
	}

	/* get input type */
	exp = dom_html_প্রবেশ_element_get_type(প্রবেশ_element, &inputtype);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ফেরত NSERROR_DOM;
	}

	/* process according to input element type */
	যদি (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_to_তথ্য_প্রবেশ_submit(প্রবেশ_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_তথ্য_পরবর্তী_ptr);

	} অন্যথা যদি (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_to_তথ্য_প্রবেশ_image(প্রবেশ_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_তথ্য_পরবর্তী_ptr);

	} অন্যথা যদি (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_to_তথ্য_প্রবেশ_checkbox(প্রবেশ_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_তথ্য_পরবর্তী_ptr);

	} অন্যথা যদি (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_to_তথ্য_প্রবেশ_file(প্রবেশ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_তথ্য_পরবর্তী_ptr);

	} অন্যথা যদি (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} অন্যথা {
		/* Everything else is treated as text values */
		res = form_dom_to_তথ্য_প্রবেশ_text(প্রবেশ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_তথ্য_পরবর্তী_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ফেরত res;
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
স্থির nserror
form_dom_to_তথ্য_button(dom_html_button_element *button_element,
			ধ্রুবক অক্ষর *form_charset,
			ধ্রুবক অক্ষর *doc_charset,
			dom_html_element **submit_button,
			কাঠামো fetch_multipart_তথ্য ***fetch_তথ্য_পরবর্তী_ptr)
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
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ফেরত NSERROR_OK;
	}

	/* get the type attribute */
	exp = dom_html_button_element_get_type(button_element, &inputtype);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ফেরত NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	যদি (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ফেরত NSERROR_OK;
	}
	যদি (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ফেরত NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	যদি (*submit_button == ফাঁকা) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	যদি (*submit_button != (dom_html_element *)button_element) {
		ফেরত NSERROR_OK;
	}

	/* obtain name property */
	exp = dom_html_button_element_get_name(button_element, &inputname);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", exp);
		ফেরত NSERROR_DOM;
	}

	যদি (inputname == ফাঁকা) {
		/* allow enumeration to continue after element with no name */
		ফেরত NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	exp = dom_html_button_element_get_মান(button_element, &inputvalue);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} অন্যথা {
		res = fetch_তথ্য_list_add(inputname,
					  inputvalue,
					  ফাঁকা,
					  form_charset,
					  doc_charset,
					  fetch_তথ্য_পরবর্তী_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ফেরত res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
স্থির অক্ষর *form_acceptable_charset(কাঠামো form *form)
{
	অক্ষর *অস্থায়ী, *c;

	যদি (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		যদি (form->document_charset) {
			/* document charset present, so use it */
			ফেরত strdup(form->document_charset);
		} অন্যথা {
			/* no document charset, so default to 8859-1 */
			ফেরত strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	অস্থায়ী = strdup(form->accept_charsets);
	যদি (!অস্থায়ী)
		ফেরত ফাঁকা;

	/* make it upper case */
	জন্য (c = অস্থায়ী; *c; c++) {
		*c = ascii_to_upper(*c);
	}

	/* is UTF-8 specified? */
	c = strstr(অস্থায়ী, "UTF-8");
	যদি (c) {
		মুক্ত(অস্থায়ী);
		ফেরত strdup("UTF-8");
	}

	/* dispense with temporary copy */
	মুক্ত(অস্থায়ী);

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
	যখন (*c && !ascii_is_space(*c)) {
		যদি (*c == ',')
			বিরতি;
		c++;
	}

	ফেরত strndup(form->accept_charsets, c - form->accept_charsets);
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
স্থির nserror
form_dom_to_তথ্য(কাঠামো form *form,
		 কাঠামো form_control *submit_control,
		 কাঠামো fetch_multipart_তথ্য **fetch_তথ্য_out)
{
	nserror res = NSERROR_OK;
	অক্ষর *charset; /* form characterset */
	dom_exception exp; /* the result from DOM operations */
	dom_html_collection *elements = ফাঁকা; /* the dom form elements */
	uint32_t element_গুনতি; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_গ্রন্থি *element = ফাঁকা; /* the DOM form element */
	dom_string *nodename = ফাঁকা; /* the DOM node name of the element */
	কাঠামো fetch_multipart_তথ্য *fetch_তথ্য = ফাঁকা; /* fetch data list */
	কাঠামো fetch_multipart_তথ্য **fetch_তথ্য_পরবর্তী = &fetch_তথ্য;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	যদি (submit_control != ফাঁকা) {
		submit_button = submit_control->গ্রন্থি;
	} অন্যথা {
		submit_button = ফাঁকা;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	যদি (charset == ফাঁকা) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ফেরত NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	exp = dom_html_form_element_get_elements(form->গ্রন্থি, &elements);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		মুক্ত(charset);
		ফেরত NSERROR_DOM;
	}

	exp = dom_html_collection_get_দৈর্ঘ্য(elements, &element_গুনতি);
	যদি (exp != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		যাও form_dom_to_তথ্য_error;
	}

	জন্য (element_idx = 0; element_idx < element_গুনতি; element_idx++) {
		/* obtain a form element */
		exp = dom_html_collection_item(elements, element_idx, &element);
		যদি (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, exp);
			res = NSERROR_DOM;
			যাও form_dom_to_তথ্য_error;
		}

		/* node name from element */
		exp = dom_গ্রন্থি_get_গ্রন্থি_name(element, &nodename);
		যদি (exp != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, exp);
			dom_গ্রন্থি_unref(element);
			res = NSERROR_DOM;
			যাও form_dom_to_তথ্য_error;
		}

		যদি (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_to_তথ্য_textarea(
				(dom_html_text_area_element *)element,
				charset,
				form->document_charset,
				&fetch_তথ্য_পরবর্তী);

		} অন্যথা যদি (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_to_তথ্য_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_তথ্য_পরবর্তী);

		} অন্যথা যদি (dom_string_isequal(nodename, corestring_dom_প্রবেশ)) {
			/* Form element is HTMLInputElement */
			res = form_dom_to_তথ্য_প্রবেশ(
				(dom_html_প্রবেশ_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_তথ্য_পরবর্তী);

		} অন্যথা যদি (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_to_তথ্য_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_তথ্য_পরবর্তী);

		} অন্যথা {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (পূর্ণসংখ্যা)dom_string_byte_দৈর্ঘ্য(nodename),
			      dom_string_তথ্য(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_গ্রন্থি_unref(element);

		/* abort form element enumeration on error */
		যদি (res != NSERROR_OK) {
			যাও form_dom_to_তথ্য_error;
		}
	}

	*fetch_তথ্য_out = fetch_তথ্য;
	dom_html_collection_unref(elements);
	মুক্ত(charset);

	ফেরত NSERROR_OK;

form_dom_to_তথ্য_error:
	fetch_multipart_তথ্য_destroy(fetch_তথ্য);
	dom_html_collection_unref(elements);
	মুক্ত(charset);

	ফেরত res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
স্থির nserror
form_url_encode(কাঠামো form *form,
		কাঠামো fetch_multipart_তথ্য *control,
		অক্ষর **encoded_out)
{
	অক্ষর *name, *মান;
	অক্ষর *s, *s2;
	unsigned পূর্ণসংখ্যা len, len1, len_init;
	nserror res;

	s = malloc(1);

	যদি (s == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	জন্য (; control; control = control->পরবর্তী) {
		res = url_escape(control->name, true, ফাঁকা, &name);
		যদি (res != NSERROR_OK) {
			মুক্ত(s);
			ফেরত res;
		}

		res = url_escape(control->মান, true, ফাঁকা, &মান);
		যদি (res != NSERROR_OK) {
			মুক্ত(name);
			মুক্ত(s);
			ফেরত res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + strlen(name) + strlen(মান) + 2;
		s2 = realloc(s, len1 + 1);
		যদি (s2 == ফাঁকা) {
			মুক্ত(মান);
			মুক্ত(name);
			মুক্ত(s);
			ফেরত NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, মান);
		len = len1;
		মুক্ত(name);
		মুক্ত(মান);
	}

	যদি (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_out = s;

	ফেরত NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
স্থির শূন্য
form_select_menu_scroll_callback(শূন্য *client_তথ্য,
				 কাঠামো scrollbar_বার্তা_তথ্য *scrollbar_তথ্য)
{
	কাঠামো form_control *control = client_তথ্য;
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;
	html_content *html = (html_content *)menu->c;

	নির্বাচন (scrollbar_তথ্য->বার্তা) {
		ক্ষেত্র SCROLLBAR_বার্তা_MOVED:
			menu->callback(menu->client_তথ্য,
					0, 0,
					menu->width,
					menu->height);
			বিরতি;
		ক্ষেত্র SCROLLBAR_বার্তা_SCROLL_START:
		{
			কাঠামো rect rect = {
				.x0 = scrollbar_তথ্য->x0,
				.y0 = scrollbar_তথ্য->y0,
				.x1 = scrollbar_তথ্য->x1,
				.y1 = scrollbar_তথ্য->y1
			};

			browser_window_set_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			বিরতি;
		ক্ষেত্র SCROLLBAR_বার্তা_SCROLL_FINISHED:
			menu->scroll_capture = false;

			browser_window_set_drag_type(html->bw,
					DRAGGING_NONE, ফাঁকা);
			বিরতি;
		পূর্বনির্ধারিত:
			বিরতি;
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
স্থির nserror
form__select_process_selection(html_content *html,
			       কাঠামো form_control *control,
			       পূর্ণসংখ্যা item)
{
	কাঠামো box *inline_box;
	কাঠামো form_option *o;
	পূর্ণসংখ্যা গুনতি;
	nserror ret = NSERROR_OK;

	প্রতিজ্ঞা(control != ফাঁকা);
	প্রতিজ্ঞা(html != ফাঁকা);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	জন্য (গুনতি = 0, o = control->তথ্য.select.বস্তুসমূহ;
			o != ফাঁকা;
			গুনতি++, o = o->পরবর্তী) {
		যদি (!control->তথ্য.select.multiple && o->selected) {
			o->selected = false;
			dom_html_option_element_set_selected(o->গ্রন্থি, false);
		}

		যদি (গুনতি == item) {
			যদি (control->তথ্য.select.multiple) {
				যদি (o->selected) {
					o->selected = false;
					dom_html_option_element_set_selected(
							o->গ্রন্থি, false);
					control->তথ্য.select.অঙ্ক_selected--;
				} অন্যথা {
					o->selected = true;
					dom_html_option_element_set_selected(
							o->গ্রন্থি, true);
					control->তথ্য.select.অঙ্ক_selected++;
				}
			} অন্যথা {
				dom_html_option_element_set_selected(
						o->গ্রন্থি, true);
				o->selected = true;
			}
		}

		যদি (o->selected) {
			control->তথ্য.select.current = o;
		}
	}

	talloc_free(inline_box->text);
	inline_box->text = 0;

	যদি (control->তথ্য.select.অঙ্ক_selected == 0) {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} অন্যথা যদি (control->তথ্য.select.অঙ্ক_selected == 1) {
		inline_box->text = talloc_strdup(html->bctx,
				control->তথ্য.select.current->text);
	} অন্যথা {
		inline_box->text = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	যদি (!inline_box->text) {
		ret = NSERROR_NOMEM;
		inline_box->দৈর্ঘ্য = 0;
	} অন্যথা {
		inline_box->দৈর্ঘ্য = strlen(inline_box->text);
	}
	inline_box->width = control->box->width;

	html__redraw_a_box(html, control->box);

	ফেরত ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
স্থির শূন্য form_select_menu_clicked(কাঠামো form_control *control, পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;
	কাঠামো form_option *option;
	html_content *html = (html_content *)menu->c;
	পূর্ণসংখ্যা সারি_height, সারি_height_with_spacing;
	পূর্ণসংখ্যা item_bottom_y;
	পূর্ণসংখ্যা scroll, i;

	scroll = scrollbar_get_offset(menu->scrollbar);

	সারি_height = menu->সারি_height;
	সারি_height_with_spacing = সারি_height +
			সারি_height * SELECT_সারি_SPACING;

	option = control->তথ্য.select.বস্তুসমূহ;
	item_bottom_y = সারি_height_with_spacing;
	i = 0;
	যখন (option && item_bottom_y < scroll + y) {
		item_bottom_y += সারি_height_with_spacing;
		option = option->পরবর্তী;
		i++;
	}

	যদি (option != ফাঁকা) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_তথ্য, 0, 0, menu->width, menu->height);
}


/* exported interface documented in html/form_internal.h */
শূন্য form_add_control(কাঠামো form *form, কাঠামো form_control *control)
{
	যদি (form == ফাঁকা) {
		ফেরত;
	}

	control->form = form;

	যদি (form->controls != ফাঁকা) {
		প্রতিজ্ঞা(form->last_control);

		form->last_control->পরবর্তী = control;
		control->পূর্ব = form->last_control;
		control->পরবর্তী = ফাঁকা;
		form->last_control = control;
	} অন্যথা {
		form->controls = form->last_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
শূন্য form_free_control(কাঠামো form_control *control)
{
	কাঠামো form_control *c;
	প্রতিজ্ঞা(control != ফাঁকা);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->মান, control->initial_মান);
	মুক্ত(control->name);
	মুক্ত(control->মান);
	মুক্ত(control->initial_মান);
	যদি (control->last_synced_মান != ফাঁকা) {
		মুক্ত(control->last_synced_মান);
	}

	যদি (control->type == GADGET_SELECT) {
		কাঠামো form_option *option, *পরবর্তী;

		জন্য (option = control->তথ্য.select.বস্তুসমূহ; option;
				option = পরবর্তী) {
			পরবর্তী = option->পরবর্তী;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->text, option->মান);
			মুক্ত(option->text);
			মুক্ত(option->মান);
			মুক্ত(option);
		}
		যদি (control->তথ্য.select.menu != ফাঁকা) {
			form_free_select_menu(control);
		}
	}

	যদি (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		যদি (control->তথ্য.text.initial != ফাঁকা) {
			dom_string_unref(control->তথ্য.text.initial);
		}

		যদি (control->তথ্য.text.ta != ফাঁকা) {
			textarea_destroy(control->তথ্য.text.ta);
		}
	}

	/* unlink the control from the form */
	যদি (control->form != ফাঁকা) {
		জন্য (c = control->form->controls; c != ফাঁকা; c = c->পরবর্তী) {
			যদি (c->পরবর্তী == control) {
				c->পরবর্তী = control->পরবর্তী;
				যদি (control->form->last_control == control)
					control->form->last_control = c;
				বিরতি;
			}
			যদি (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->পরবর্তী;
				যদি (control->form->last_control == control)
					control->form->controls =
						control->form->last_control = ফাঁকা;
				বিরতি;
			}
		}
	}

	যদি (control->গ্রন্থি_মান != ফাঁকা) {
		dom_string_unref(control->গ্রন্থি_মান);
	}

	মুক্ত(control);
}


/* exported interface documented in html/form_internal.h */
bool form_add_option(কাঠামো form_control *control, অক্ষর *মান, অক্ষর *text,
		     bool selected, শূন্য *গ্রন্থি)
{
	কাঠামো form_option *option;

	প্রতিজ্ঞা(control);
	প্রতিজ্ঞা(control->type == GADGET_SELECT);

	option = calloc(1, আকার *option);
	যদি (!option)
		ফেরত false;

	option->মান = মান;
	option->text = text;

	/* add to linked list */
	যদি (control->তথ্য.select.বস্তুসমূহ == 0)
		control->তথ্য.select.বস্তুসমূহ = option;
	অন্যথা
		control->তথ্য.select.last_item->পরবর্তী = option;
	control->তথ্য.select.last_item = option;

	/* set selected */
	যদি (selected && (control->তথ্য.select.অঙ্ক_selected == 0 ||
			control->তথ্য.select.multiple)) {
		option->selected = option->initial_selected = true;
		control->তথ্য.select.অঙ্ক_selected++;
		control->তথ্য.select.current = option;
	}

	control->তথ্য.select.অঙ্ক_বস্তুসমূহ++;

	option->গ্রন্থি = গ্রন্থি;

	ফেরত true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(শূন্য *client_তথ্য,
		      কাঠামো form_control *control,
		      select_menu_redraw_callback callback,
		      কাঠামো content *c)
{
	পূর্ণসংখ্যা সারি_height_with_spacing;
	কাঠামো box *box;
	plot_font_style_t fstyle;
	পূর্ণসংখ্যা যোগ_height;
	কাঠামো form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	যদি (control->তথ্য.select.menu == ফাঁকা) {

		menu = calloc(1, আকার (কাঠামো form_select_menu));
		যদি (menu == ফাঁকা) {
			ফেরত NSERROR_NOMEM;
		}

		control->তথ্য.select.menu = menu;

		box = control->box;

		menu->width = box->width +
			box->border[ডান_2].width + box->padding[ডান_2] +
			box->border[বাম_2].width + box->padding[বাম_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_আকার = fstyle.আকার_2;

		menu->সারি_height = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.আকার_2 / PLOT_STYLE_SCALE)))),
				F_72));

		সারি_height_with_spacing = menu->সারি_height +
				menu->সারি_height *
				SELECT_সারি_SPACING;

		যোগ_height = control->তথ্য.select.অঙ্ক_বস্তুসমূহ *
				সারি_height_with_spacing;
		menu->height = যোগ_height;

		যদি (menu->height > MAX_SELECT_HEIGHT) {
			menu->height = MAX_SELECT_HEIGHT;
		}

		menu->client_তথ্য = client_তথ্য;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->height,
				       যোগ_height,
				       menu->height,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		যদি (res != NSERROR_OK) {
			control->তথ্য.select.menu = ফাঁকা;
			মুক্ত(menu);
			ফেরত res;
		}
		menu->c = c;
	} অন্যথা {
		menu = control->তথ্য.select.menu;
	}

	menu->callback(client_তথ্য, 0, 0, menu->width, menu->height);

	ফেরত NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
শূন্য form_free_select_menu(কাঠামো form_control *control)
{
	যদি (control->তথ্য.select.menu->scrollbar != ফাঁকা)
		scrollbar_destroy(control->তথ্য.select.menu->scrollbar);
	মুক্ত(control->তথ্য.select.menu);
	control->তথ্য.select.menu = ফাঁকা;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(কাঠামো form_control *control,
			পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
			দশমিক scale,
			ধ্রুবক কাঠামো rect *clip,
			ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো box *box;
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;
	কাঠামো form_option *option;
	পূর্ণসংখ্যা সারি_height, সারি_height_with_spacing;
	পূর্ণসংখ্যা width, height;
	পূর্ণসংখ্যা x0, y0, x1, scrollbar_x, y1, y2, y3;
	পূর্ণসংখ্যা item_y;
	পূর্ণসংখ্যা text_pos_offset, text_x;
	পূর্ণসংখ্যা scrollbar_width = SCROLLBAR_WIDTH;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা scroll;
	পূর্ণসংখ্যা x_cp, y_cp;
	কাঠামো rect r;
	কাঠামো rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	width = menu->width;
	height = menu->height;
	সারি_height = menu->সারি_height;

	সারি_height_with_spacing = সারি_height +
			সারি_height * SELECT_সারি_SPACING;
	scroll = scrollbar_get_offset(menu->scrollbar);

	যদি (scale != 1.0) {
		x *= scale;
		y *= scale;
		width *= scale;
		height *= scale;
		scrollbar_width *= scale;

		i = scroll / সারি_height_with_spacing;
		scroll -= i * সারি_height_with_spacing;
		সারি_height *= scale;
		সারি_height_with_spacing *= scale;
		scroll *= scale;
		scroll += i * সারি_height_with_spacing;
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
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
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
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	option = control->তথ্য.select.বস্তুসমূহ;
	item_y = সারি_height_with_spacing;

	যখন (item_y < scroll) {
		option = option->পরবর্তী;
		item_y += সারি_height_with_spacing;
	}
	item_y -= সারি_height_with_spacing;
	text_pos_offset = y - scroll +
			(পূর্ণসংখ্যা) (সারি_height * (0.75 + SELECT_সারি_SPACING));
	text_x = x + (box->border[বাম_2].width + box->padding[বাম_2]) * scale;

	plot_fstyle_entry.আকার_2 = menu->f_আকার;

	যখন (option && item_y - scroll < height) {

		যদি (option->selected) {
			y2 = y + item_y - scroll;
			y3 = y + item_y + সারি_height_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
		}

		y2 = text_pos_offset + item_y;
		res = ctx->plot->text(ctx,
				      &plot_fstyle_entry,
				      text_x, y2,
				      option->text, strlen(option->text));
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}

		item_y += সারি_height_with_spacing;
		option = option->পরবর্তী;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->width - SCROLLBAR_WIDTH,
			       y_cp,
			       clip, scale, ctx);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	ফেরত true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(কাঠামো form_control *control,
			     দশমিক scale,
			     ধ্রুবক কাঠামো rect *clip)
{
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;
	পূর্ণসংখ্যা width, height;


	width = menu->width;
	height = menu->height;

	যদি (scale != 1.0) {
		width *= scale;
		height *= scale;
	}

	যদি (clip->x0 >= 0 &&
	    clip->x1 <= width &&
	    clip->y0 >= 0 &&
	    clip->y1 <= height)
		ফেরত true;

	ফেরত false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(কাঠামো form_control *control, পূর্ণসংখ্যা item)
{
	প্রতিজ্ঞা(control != ফাঁকা);

	ফেরত form__select_process_selection(control->html, control, item);
}


/* exported interface documented in netsurf/form.h */
কাঠামো form_option *
form_select_get_option(কাঠামো form_control *control, পূর্ণসংখ্যা item)
{
	কাঠামো form_option *opt;

	opt = control->তথ্য.select.বস্তুসমূহ;
	যখন ((opt != ফাঁকা) && (item > 0)) {
		opt = opt->পরবর্তী;
		item--;
	}
	ফেরত opt;
}


/* exported interface documented in netsurf/form.h */
অক্ষর *form_control_get_name(কাঠামো form_control *control)
{
	ফেরত control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(কাঠামো form_control *control, কাঠামো rect *r)
{
	box_bounds( control->box, r );
	ফেরত NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ধ্রুবক অক্ষর *
form_select_mouse_action(কাঠামো form_control *control,
			 browser_mouse_state mouse,
			 পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;
	পূর্ণসংখ্যা x0, y0, x1, y1, scrollbar_x;
	ধ্রুবক অক্ষর *status = ফাঁকা;
	bool multiple = control->তথ্য.select.multiple;

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;
	scrollbar_x = x1 - SCROLLBAR_WIDTH;

	যদি (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ফেরত scrollbar_mouse_status_to_message(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	যদি (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		যদি (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		যদি (!(mouse & BROWSER_MOUSE_CLICK_1 && !multiple))
			/* anything but a button 1 click over a single select
			   menu */
			status = messages_get(control->তথ্য.select.multiple ?
					"SelectMClick" : "SelectClick");

	} অন্যথা যদি (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		status = messages_get("SelectClose");

	ফেরত status;
}


/* private interface described in html/form_internal.h */
শূন্য
form_select_mouse_drag_end(কাঠামো form_control *control,
			   browser_mouse_state mouse,
			   পূর্ণসংখ্যা x, পূর্ণসংখ্যা y)
{
	পূর্ণসংখ্যা x0, y0, x1, y1;
	পূর্ণসংখ্যা box_x, box_y;
	কাঠামো box *box;
	কাঠামো form_select_menu *menu = control->তথ্য.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[বাম_2].width;
	box_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] + box->padding[শীর্ষ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	যদি (menu->scroll_capture) {
		x -= menu->width - SCROLLBAR_WIDTH;
		scrollbar_mouse_drag_end(menu->scrollbar, mouse, x, y);
		ফেরত;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->width;
	y1 = menu->height;


	যদি (x > x0 && x < x1 - SCROLLBAR_WIDTH && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
শূন্য form_select_get_dimensions(কাঠামো form_control *control,
		পূর্ণসংখ্যা *width, পূর্ণসংখ্যা *height)
{
	*width = control->তথ্য.select.menu->width;
	*height = control->তথ্য.select.menu->height;
}


/* private interface described in html/form_internal.h */
শূন্য form_select_menu_callback(শূন্য *client_তথ্য,
		পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	html_content *html = client_তথ্য;
	পূর্ণসংখ্যা menu_x, menu_y;
	কাঠামো box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[বাম_2].width;
	menu_y += box->height + box->border[BOTTOM].width +
			box->padding[BOTTOM] +
			box->padding[শীর্ষ_2];
	content__request_redraw((কাঠামো content *)html, menu_x + x, menu_y + y,
			width, height);
}


/* private interface described in html/form_internal.h */
শূন্য form_radio_set(কাঠামো form_control *radio)
{
	কাঠামো form_control *control;

	প্রতিজ্ঞা(radio);
	যদি (!radio->form)
		ফেরত;

	যদি (radio->selected)
		ফেরত;

	/* Clear selected state for other controls in
	 * the same radio button group */
	জন্য (control = radio->form->controls;
	     control != ফাঁকা;
	     control = control->পরবর্তী) {
		/* Only interested in radio inputs */
		যদি (control->type != GADGET_RADIO)
			চালু;

		/* Ignore ourself */
		যদি (control == radio)
			চালু;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		যদি ((control->name == ফাঁকা) ||
		    (radio->name == ফাঁকা) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			চালু;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		যদি (control->selected) {
			control->selected = false;
			dom_html_প্রবেশ_element_set_checked(control->গ্রন্থি, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_প্রবেশ_element_set_checked(radio->গ্রন্থি, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *page_url,
	    কাঠামো browser_window *target,
	    কাঠামো form *form,
	    কাঠামো form_control *submit_button)
{
	nserror res;
	অক্ষর *তথ্য = ফাঁকা; /* encoded form data */
	কাঠামো fetch_multipart_তথ্য *success = ফাঁকা; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	প্রতিজ্ঞা(form != ফাঁকা);

	/* obtain list of controls from DOM */
	res = form_dom_to_তথ্য(form, submit_button, &success);
	যদি (res != NSERROR_OK) {
		ফেরত res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	যদি (res != NSERROR_OK) {
		fetch_multipart_তথ্য_destroy(success);
		ফেরত res;
	}

	নির্বাচন (form->method) {
	ক্ষেত্র method_GET:
		res = form_url_encode(form, success, &তথ্য);
		যদি (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, তথ্য, &query_url);
			যদি (res == NSERROR_OK) {
				res = browser_window_navigate(target,
							      query_url,
							      page_url,
							      BW_NAVIGATE_HISTORY,
							      ফাঁকা,
							      ফাঁকা,
							      ফাঁকা);

				nsurl_unref(query_url);
			}
			মুক্ত(তথ্য);
		}
		বিরতি;

	ক্ষেত্র method_POST_URLENC:
		res = form_url_encode(form, success, &তথ্য);
		যদি (res == NSERROR_OK) {
			res = browser_window_navigate(target,
						      action_url,
						      page_url,
						      BW_NAVIGATE_HISTORY,
						      তথ্য,
						      ফাঁকা,
						      ফাঁকা);
			মুক্ত(তথ্য);
		}
		বিরতি;

	ক্ষেত্র method_POST_MULTIPART:
		res = browser_window_navigate(target,
					      action_url,
					      page_url,
					      BW_NAVIGATE_HISTORY,
					      ফাঁকা,
					      success,
					      ফাঁকা);

		বিরতি;
	}

	nsurl_unref(action_url);
	fetch_multipart_তথ্য_destroy(success);

	ফেরত res;
}


/* exported interface documented in html/form_internal.h */
শূন্য form_gadget_update_মান(কাঠামো form_control *control, অক্ষর *মান)
{
	নির্বাচন (control->type) {
	ক্ষেত্র GADGET_HIDDEN:
	ক্ষেত্র GADGET_TEXTBOX:
	ক্ষেত্র GADGET_TEXTAREA:
	ক্ষেত্র GADGET_PASSWORD:
	ক্ষেত্র GADGET_FILE:
		যদি (control->মান != ফাঁকা) {
			মুক্ত(control->মান);
		}
		control->মান = মান;
		যদি (control->গ্রন্থি != ফাঁকা) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)মান,
						strlen(মান), &str);
			যদি (err == DOM_NO_ERR) {
				যদি (control->type == GADGET_TEXTAREA)
					err = dom_html_text_area_element_set_মান(
						(dom_html_text_area_element *)(control->গ্রন্থি),
						str);
				অন্যথা
					err = dom_html_প্রবেশ_element_set_মান(
						(dom_html_প্রবেশ_element *)(control->গ্রন্থি),
						str);
				dom_string_unref(str);
			}
		}
		বিরতি;
	পূর্বনির্ধারিত:
		/* Do nothing */
		বিরতি;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
শূন্য
form_gadget_sync_with_dom(কাঠামো form_control *control)
{
	dom_exception exc;
	dom_string *মান = ফাঁকা;
	bool changed_dom = false;

	যদি (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ফেরত;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	যদি ((control->last_synced_মান == ফাঁকা &&
	     control->মান != ফাঁকা &&
	     control->মান[0] != '\0') ||
	    (control->last_synced_মান != ফাঁকা &&
	     control->মান != ফাঁকা &&
	     strcmp(control->মান, control->last_synced_মান) != 0)) {
		অক্ষর *dup = strdup(control->মান);
		যদি (dup == ফাঁকা) {
			যাও out;
		}
		যদি (control->last_synced_মান != ফাঁকা) {
			মুক্ত(control->last_synced_মান);
		}
		control->last_synced_মান = dup;
		exc = dom_string_create((uint8_t *)(control->মান),
					strlen(control->মান), &মান);
		যদি (exc != DOM_NO_ERR) {
			যাও out;
		}
		যদি (control->গ্রন্থি_মান != ফাঁকা) {
			dom_string_unref(control->গ্রন্থি_মান);
		}
		control->গ্রন্থি_মান = মান;
		মান = ফাঁকা;
		যদি (control->type == GADGET_TEXTAREA) {
			exc = dom_html_text_area_element_set_মান(control->গ্রন্থি, control->গ্রন্থি_মান);
		} অন্যথা {
			exc = dom_html_প্রবেশ_element_set_মান(control->গ্রন্থি, control->গ্রন্থি_মান);
		}
		যদি (exc != DOM_NO_ERR) {
			যাও out;
		}
		changed_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	যদি (control->type == GADGET_TEXTAREA) {
		exc = dom_html_text_area_element_get_মান(control->গ্রন্থি, &মান);
	} অন্যথা {
		exc = dom_html_প্রবেশ_element_get_মান(control->গ্রন্থি, &মান);
	}

	যদি (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		যাও out;
	}

	যদি (!dom_string_isequal(control->গ্রন্থি_মান, মান)) {
		/* The DOM has changed */
		যদি (!changed_dom) {
			/* And it wasn't us */
			অক্ষর *মান_s = strndup(
				dom_string_তথ্য(মান),
				dom_string_byte_দৈর্ঘ্য(মান));
			অক্ষর *dup = ফাঁকা;
			যদি (মান_s == ফাঁকা) {
				যাও out;
			}
			dup = strdup(মান_s);
			যদি (dup == ফাঁকা) {
				মুক্ত(মান_s);
				যাও out;
			}
			মুক্ত(control->মান);
			control->মান = মান_s;
			মুক্ত(control->last_synced_মান);
			control->last_synced_মান = dup;
			যদি (control->type != GADGET_HIDDEN &&
			    control->তথ্য.text.ta != ফাঁকা) {
				textarea_set_text(control->তথ্য.text.ta,
						  মান_s);
			}
		}
		control->গ্রন্থি_মান = মান;
		মান = ফাঁকা;
	}

out:
	যদি (মান != ফাঁকা)
		dom_string_unref(মান);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
কাঠামো form *
form_new(শূন্য *গ্রন্থি,
	 ধ্রুবক অক্ষর *action,
	 ধ্রুবক অক্ষর *target,
	 form_method method,
	 ধ্রুবক অক্ষর *charset,
	 ধ্রুবক অক্ষর *doc_charset)
{
	কাঠামো form *form;

	form = calloc(1, আকার *form);
	যদি (!form)
		ফেরত ফাঁকা;

	form->action = strdup(action != ফাঁকা ? action : "");
	যদি (form->action == ফাঁকা) {
		মুক্ত(form);
		ফেরত ফাঁকা;
	}

	form->target = target != ফাঁকা ? strdup(target) : ফাঁকা;
	যদি (target != ফাঁকা && form->target == ফাঁকা) {
		মুক্ত(form->action);
		মুক্ত(form);
		ফেরত ফাঁকা;
	}

	form->method = method;

	form->accept_charsets = charset != ফাঁকা ? strdup(charset) : ফাঁকা;
	যদি (charset != ফাঁকা && form->accept_charsets == ফাঁকা) {
		মুক্ত(form->target);
		মুক্ত(form->action);
		মুক্ত(form);
		ফেরত ফাঁকা;
	}

	form->document_charset = doc_charset != ফাঁকা ? strdup(doc_charset)
						     : ফাঁকা;
	যদি (doc_charset && form->document_charset == ফাঁকা) {
		মুক্ত(form->accept_charsets);
		মুক্ত(form->target);
		মুক্ত(form->action);
		মুক্ত(form);
		ফেরত ফাঁকা;
	}

	form->গ্রন্থি = গ্রন্থি;

	ফেরত form;
}


/* exported interface documented in html/form_internal.h */
শূন্য form_free(কাঠামো form *form)
{
	কাঠামো form_control *c, *d;

	জন্য (c = form->controls; c != ফাঁকা; c = d) {
		d = c->পরবর্তী;

		form_free_control(c);
	}

	মুক্ত(form->action);
	মুক্ত(form->target);
	মুক্ত(form->accept_charsets);
	মুক্ত(form->document_charset);

	মুক্ত(form);
}


/* exported interface documented in html/form_internal.h */
কাঠামো form_control *form_new_control(শূন্য *গ্রন্থি, form_control_type type)
{
	কাঠামো form_control *control;

	control = calloc(1, আকার *control);
	যদি (control == ফাঁকা)
		ফেরত ফাঁকা;

	control->গ্রন্থি = গ্রন্থি;
	control->type = type;

	ফেরত control;
}
