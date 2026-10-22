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

#ضمن <assert.h>
#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <dom/dom.h>

#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/talloc.h"
#ضمن "utils/url.h"
#ضمن "utils/utf8.h"
#ضمن "utils/ascii.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/inttypes.h"
#ضمن "netsurf/mouse.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/misc.h"
#ضمن "content/fetch.h"
#ضمن "content/hlcache.h"
#ضمن "css/utils.h"
#ضمن "desktop/knockout.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/layout.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"

#وصف_ڪريو MAX_SELECT_اوچائي 210
#وصف_ڪريو SELECT_سٽ_SPACING 0.2
#وصف_ڪريو SELECT_BORDER_ويڪر 1
#وصف_ڪريو SELECT_SELECTED_رنگ 0xDB9370

بنية form_select_menu {
	صحيح سٽ_اوچائي;
	صحيح ويڪر, اوچائي;
	بنية scrollbar *scrollbar;
	صحيح f_سائيز;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	فراغ *client_معلومات;
	بنية content *c;
};

ساكن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_رنگ = SELECT_SELECTED_رنگ,
};

ساكن plot_font_style_t plot_fstyle_داخلا = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.وزن = 400,
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
ساكن حرف *
form_encode_شي(ثابت حرف *شي,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = عدم;
	حرف cset[256];

	إذا (!شي || !charset)
		أرجع عدم;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_منزل_enc(شي, cset, 0, &ret);
	إذا (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_منزل_enc(شي, cset, len, &ret);

		إذا (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			إذا (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_منزل_enc(شي, cset, 0, &ret);

				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_منزل_enc(شي, cset, 0, &ret);
				}
			}

			إذا (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_منزل_enc(شي, "ISO-8859-1//TRANSLIT",
						0, &ret);
				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_منزل_enc(شي, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	إذا (err == NSERROR_NOMEM) {
		أرجع عدم;
	}

	أرجع ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#وصف_ڪريو FETCH_معلومات_INT_قدر_سائيز 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساكن nserror
fetch_معلومات_list_جوڙيو_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحيح قدر,
			  بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	بنية fetch_multipart_معلومات *fetch_معلومات;
	صحيح keysize;

	fetch_معلومات = صفر_يادگيري_ڏيو(1, حجم(*fetch_معلومات));
	إذا (fetch_معلومات == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_معلومات->name, 0, "%s%s", name, ksfx);
	fetch_معلومات->name = يادگيري_ڏيو(keysize + 1); /* allow for null */
	إذا (fetch_معلومات->name == عدم) {
		حرر(fetch_معلومات);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_معلومات->name, keysize + 1, "%s%s", name, ksfx);

	/*قدر*/
	fetch_معلومات->قدر = يادگيري_ڏيو(FETCH_معلومات_INT_قدر_سائيز);
	إذا (fetch_معلومات->قدر == عدم) {
		حرر(fetch_معلومات->name);
		حرر(fetch_معلومات);
		NSLOG(netsurf, INFO, "value allocation failure");
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_معلومات->قدر, FETCH_معلومات_INT_قدر_سائيز, "%d", قدر);

	/* link into list */
	**fetch_معلومات_ايندڙ_ptr = fetch_معلومات;
	*fetch_معلومات_ايندڙ_ptr = &fetch_معلومات->ايندڙ;

	أرجع NSERROR_OK;
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
ساكن nserror
fetch_معلومات_list_جوڙيو(dom_string *name,
		    dom_string *قدر,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	بنية fetch_multipart_معلومات *fetch_معلومات;

	أكد(name != عدم);

	fetch_معلومات = صفر_يادگيري_ڏيو(1, حجم(*fetch_معلومات));
	إذا (fetch_معلومات == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	fetch_معلومات->name = form_encode_شي(dom_string_معلومات(name),
					    dom_string_byte_ڊيگهائي(name),
					    form_charset,
					    docu_charset);
	إذا (fetch_معلومات->name == عدم) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		حرر(fetch_معلومات);
		أرجع NSERROR_NOMEM;
	}

	إذا (قدر == عدم) {
		fetch_معلومات->قدر = strdup("");
	} وإلا {
		fetch_معلومات->قدر = form_encode_شي(dom_string_معلومات(قدر),
						     dom_string_byte_ڊيگهائي(قدر),
						     form_charset,
						     docu_charset);
	}
	إذا (fetch_معلومات->قدر == عدم) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		حرر(fetch_معلومات->name);
		حرر(fetch_معلومات);
		أرجع NSERROR_NOMEM;
	}

	/* deal with raw file name */
	إذا (rawfile != عدم) {
		fetch_معلومات->file = true;
		fetch_معلومات->rawfile = strdup(rawfile);
		إذا (fetch_معلومات->rawfile == عدم) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			حرر(fetch_معلومات->قدر);
			حرر(fetch_معلومات->name);
			حرر(fetch_معلومات);
			أرجع NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_معلومات_ايندڙ_ptr = fetch_معلومات;
	*fetch_معلومات_ايندڙ_ptr = &fetch_معلومات->ايندڙ;

	أرجع NSERROR_OK;
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
ساكن nserror
form_dom_منزل_معلومات_textarea(dom_html_متن_area_element *متن_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	قوت_نما = dom_html_متن_area_element_get_disabled(متن_area_element,
						      &element_disabled);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_متن_area_element_get_name(متن_area_element,
						  &inputname);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* obtain text area value */
	قوت_نما = dom_html_متن_area_element_get_قدر(متن_area_element,
						   &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", قوت_نما);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(inputname,
				  inputvalue,
				  عدم,
				  form_charset,
				  doc_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *قدر;
	bool selected;

	قوت_نما = dom_html_option_element_get_selected(option_element, &selected);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		أرجع NSERROR_DOM;
	}

	إذا (!selected) {
		/* unselected options do not add fetch data entries */
		أرجع NSERROR_OK;
	}

	قوت_نما = dom_html_option_element_get_قدر(option_element, &قدر);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(keyname,
				  قدر,
				  عدم,
				  form_charset,
				  docu_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(قدر);

	أرجع res;
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
ساكن nserror
form_dom_منزل_معلومات_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = عدم;
	uint32_t options_ڳڻپ;
	uint32_t option_اشاريو;
	dom_ڳنڍ *option_element = عدم;

	/* check if element is disabled */
	قوت_نما = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_select_element_get_name(select_element, &inputname);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get options collection */
	قوت_نما = dom_html_select_element_get_options(select_element, &options);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* get options collection length */
	قوت_نما = dom_html_options_collection_get_ڊيگهائي(options, &options_ڳڻپ);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* iterate over options collection */
	لكل (option_اشاريو = 0; option_اشاريو < options_ڳڻپ; ++option_اشاريو) {
		قوت_نما = dom_html_options_collection_شي(options,
						       option_اشاريو,
						       &option_element);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_اشاريو);
			res = NSERROR_DOM;
		} وإلا {
			res = form_dom_منزل_معلومات_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_معلومات_ايندڙ_ptr);

			dom_ڳنڍ_unref(option_element);
		}

		إذا (res != NSERROR_OK) {
			اكسر;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_داخلا_submit(dom_html_داخلا_element *داخلا_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	إذا (*submit_button == عدم) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)داخلا_element;
	} وإلا إذا (*submit_button != (dom_html_element *)داخلا_element) {
		أرجع NSERROR_OK;
	}

	/* matched button used to submit form */
	قوت_نما = dom_html_داخلا_element_get_قدر(داخلا_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_داخلا_image(dom_html_داخلا_element *داخلا_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	بنية image_داخلا_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	إذا (*submit_button != (dom_html_element *)داخلا_element) {
		أرجع NSERROR_OK;
	}

	قوت_نما = dom_ڳنڍ_get_user_معلومات((dom_ڳنڍ *)داخلا_element,
				     corestring_dom_ns_ڪنجي_image_coords_ڳنڍ_معلومات,
				     &coords);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		أرجع NSERROR_DOM;
	}

	إذا (coords == عدم) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		أرجع NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_شي(dom_string_معلومات(inputname),
				    dom_string_byte_ڊيگهائي(inputname),
				    charset,
				    document_charset);
	إذا (basename == عدم) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		أرجع NSERROR_NOMEM;
	}

	res = fetch_معلومات_list_جوڙيو_sname(basename, ".x",
					coords->x,
					fetch_معلومات_ايندڙ_ptr);

	إذا (res == NSERROR_OK) {
		res = fetch_معلومات_list_جوڙيو_sname(basename, ".y",
						coords->y,
						fetch_معلومات_ايندڙ_ptr);
	}

	حرر(basename);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_داخلا_checkbox(dom_html_داخلا_element *داخلا_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلا_element_get_checked(داخلا_element, &checked);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		أرجع NSERROR_DOM;
	}

	إذا (!checked) {
		/* unchecked items do not generate a data entry */
		أرجع NSERROR_OK;
	}

	قوت_نما = dom_html_داخلا_element_get_قدر(داخلا_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		أرجع NSERROR_DOM;
	}

	/* ensure a default value */
	إذا (inputvalue == عدم) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_داخلا_file(dom_html_داخلا_element *داخلا_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = عدم;

	قوت_نما = dom_html_داخلا_element_get_قدر(داخلا_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		أرجع NSERROR_DOM;
	}

	قوت_نما = dom_ڳنڍ_get_user_معلومات((dom_ڳنڍ *)داخلا_element,
				     corestring_dom_ns_ڪنجي_file_name_ڳنڍ_معلومات,
				     &rawfile);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		أرجع NSERROR_DOM;
	}

	إذا (rawfile == عدم) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_معلومات_داخلا_متن(dom_html_داخلا_element *داخلا_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلا_element_get_قدر(داخلا_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_معلومات_list_جوڙيو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_معلومات_ايندڙ_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
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
ساكن nserror
form_dom_منزل_معلومات_داخلا(dom_html_داخلا_element *داخلا_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	قوت_نما = dom_html_داخلا_element_get_disabled(داخلا_element,
						  &element_disabled);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* disabled element requires no more processing */
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_داخلا_element_get_name(داخلا_element, &inputname);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* element with no name is not converted */
		أرجع NSERROR_OK;
	}

	/* get input type */
	قوت_نما = dom_html_داخلا_element_get_type(داخلا_element, &inputtype);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* process according to input element type */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_منزل_معلومات_داخلا_submit(داخلا_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_معلومات_ايندڙ_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_منزل_معلومات_داخلا_image(داخلا_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_معلومات_ايندڙ_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_منزل_معلومات_داخلا_checkbox(داخلا_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_معلومات_ايندڙ_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_منزل_معلومات_داخلا_file(داخلا_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_معلومات_ايندڙ_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وإلا {
		/* Everything else is treated as text values */
		res = form_dom_منزل_معلومات_داخلا_متن(داخلا_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_معلومات_ايندڙ_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	أرجع res;
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
ساكن nserror
form_dom_منزل_معلومات_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			بنية fetch_multipart_معلومات ***fetch_معلومات_ايندڙ_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	قوت_نما = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (element_disabled) {
		/* allow enumeration to continue after disabled element */
		أرجع NSERROR_OK;
	}

	/* get the type attribute */
	قوت_نما = dom_html_button_element_get_type(button_element, &inputtype);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		أرجع NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		أرجع NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	إذا (*submit_button == عدم) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	إذا (*submit_button != (dom_html_element *)button_element) {
		أرجع NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_button_element_get_name(button_element, &inputname);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", قوت_نما);
		أرجع NSERROR_DOM;
	}

	إذا (inputname == عدم) {
		/* allow enumeration to continue after element with no name */
		أرجع NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	قوت_نما = dom_html_button_element_get_قدر(button_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} وإلا {
		res = fetch_معلومات_list_جوڙيو(inputname,
					  inputvalue,
					  عدم,
					  form_charset,
					  doc_charset,
					  fetch_معلومات_ايندڙ_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	أرجع res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ساكن حرف *form_acceptable_charset(بنية form *form)
{
	حرف *عارضي, *c;

	إذا (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		إذا (form->document_charset) {
			/* document charset present, so use it */
			أرجع strdup(form->document_charset);
		} وإلا {
			/* no document charset, so default to 8859-1 */
			أرجع strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	عارضي = strdup(form->accept_charsets);
	إذا (!عارضي)
		أرجع عدم;

	/* make it upper case */
	لكل (c = عارضي; *c; c++) {
		*c = ascii_منزل_مٿانهون(*c);
	}

	/* is UTF-8 specified? */
	c = لڙي_ڳولا(عارضي, "UTF-8");
	إذا (c) {
		حرر(عارضي);
		أرجع strdup("UTF-8");
	}

	/* dispense with temporary copy */
	حرر(عارضي);

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
	طالما (*c && !ascii_is_space(*c)) {
		إذا (*c == ',')
			اكسر;
		c++;
	}

	أرجع strndup(form->accept_charsets, c - form->accept_charsets);
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
ساكن nserror
form_dom_منزل_معلومات(بنية form *form,
		 بنية form_control *submit_control,
		 بنية fetch_multipart_معلومات **fetch_معلومات_ٻاهريون)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_html_collection *elements = عدم; /* the dom form elements */
	uint32_t element_ڳڻپ; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ڳنڍ *element = عدم; /* the DOM form element */
	dom_string *nodename = عدم; /* the DOM node name of the element */
	بنية fetch_multipart_معلومات *fetch_معلومات = عدم; /* fetch data list */
	بنية fetch_multipart_معلومات **fetch_معلومات_ايندڙ = &fetch_معلومات;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	إذا (submit_control != عدم) {
		submit_button = submit_control->ڳنڍ;
	} وإلا {
		submit_button = عدم;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	إذا (charset == عدم) {
		NSLOG(netsurf, INFO, "failed to find charset");
		أرجع NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	قوت_نما = dom_html_form_element_get_elements(form->ڳنڍ, &elements);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		حرر(charset);
		أرجع NSERROR_DOM;
	}

	قوت_نما = dom_html_collection_get_ڊيگهائي(elements, &element_ڳڻپ);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		اذهب form_dom_منزل_معلومات_error;
	}

	لكل (element_idx = 0; element_idx < element_ڳڻپ; element_idx++) {
		/* obtain a form element */
		قوت_نما = dom_html_collection_شي(elements, element_idx, &element);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_معلومات_error;
		}

		/* node name from element */
		قوت_نما = dom_ڳنڍ_get_ڳنڍ_name(element, &nodename);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			dom_ڳنڍ_unref(element);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_معلومات_error;
		}

		إذا (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_منزل_معلومات_textarea(
				(dom_html_متن_area_element *)element,
				charset,
				form->document_charset,
				&fetch_معلومات_ايندڙ);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_منزل_معلومات_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_معلومات_ايندڙ);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_داخلا)) {
			/* Form element is HTMLInputElement */
			res = form_dom_منزل_معلومات_داخلا(
				(dom_html_داخلا_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_معلومات_ايندڙ);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_منزل_معلومات_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_معلومات_ايندڙ);

		} وإلا {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحيح)dom_string_byte_ڊيگهائي(nodename),
			      dom_string_معلومات(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ڳنڍ_unref(element);

		/* abort form element enumeration on error */
		إذا (res != NSERROR_OK) {
			اذهب form_dom_منزل_معلومات_error;
		}
	}

	*fetch_معلومات_ٻاهريون = fetch_معلومات;
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع NSERROR_OK;

form_dom_منزل_معلومات_error:
	fetch_multipart_معلومات_destroy(fetch_معلومات);
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ساكن nserror
form_url_encode(بنية form *form,
		بنية fetch_multipart_معلومات *control,
		حرف **encoded_ٻاهريون)
{
	حرف *name, *قدر;
	حرف *s, *s2;
	بنانشان صحيح len, len1, len_init;
	nserror res;

	s = يادگيري_ڏيو(1);

	إذا (s == عدم) {
		أرجع NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	لكل (; control; control = control->ايندڙ) {
		res = url_escape(control->name, true, عدم, &name);
		إذا (res != NSERROR_OK) {
			حرر(s);
			أرجع res;
		}

		res = url_escape(control->قدر, true, عدم, &قدر);
		إذا (res != NSERROR_OK) {
			حرر(name);
			حرر(s);
			أرجع res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + لڙي_ڊيگهه(name) + لڙي_ڊيگهه(قدر) + 2;
		s2 = realloc(s, len1 + 1);
		إذا (s2 == عدم) {
			حرر(قدر);
			حرر(name);
			حرر(s);
			أرجع NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, قدر);
		len = len1;
		حرر(name);
		حرر(قدر);
	}

	إذا (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_ٻاهريون = s;

	أرجع NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساكن فراغ
form_select_menu_scroll_callback(فراغ *client_معلومات,
				 بنية scrollbar_نياپو_معلومات *scrollbar_معلومات)
{
	بنية form_control *control = client_معلومات;
	بنية form_select_menu *menu = control->معلومات.select.menu;
	html_content *html = (html_content *)menu->c;

	بدل (scrollbar_معلومات->نياپو) {
		حالة SCROLLBAR_نياپو_MOVED:
			menu->callback(menu->client_معلومات,
					0, 0,
					menu->ويڪر,
					menu->اوچائي);
			اكسر;
		حالة SCROLLBAR_نياپو_SCROLL_شروعات:
		{
			بنية rect rect = {
				.x0 = scrollbar_معلومات->x0,
				.y0 = scrollbar_معلومات->y0,
				.x1 = scrollbar_معلومات->x1,
				.y1 = scrollbar_معلومات->y1
			};

			browser_دري_سيٽ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			اكسر;
		حالة SCROLLBAR_نياپو_SCROLL_ختم:
			menu->scroll_capture = false;

			browser_دري_سيٽ_drag_type(html->bw,
					DRAGGING_NONE, عدم);
			اكسر;
		افتراضي:
			اكسر;
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
ساكن nserror
form__select_process_selection(html_content *html,
			       بنية form_control *control,
			       صحيح شي)
{
	بنية box *inline_box;
	بنية form_option *o;
	صحيح ڳڻپ;
	nserror ret = NSERROR_OK;

	أكد(control != عدم);
	أكد(html != عدم);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	لكل (ڳڻپ = 0, o = control->معلومات.select.شيون;
			o != عدم;
			ڳڻپ++, o = o->ايندڙ) {
		إذا (!control->معلومات.select.گڻو && o->selected) {
			o->selected = false;
			dom_html_option_element_سيٽ_selected(o->ڳنڍ, false);
		}

		إذا (ڳڻپ == شي) {
			إذا (control->معلومات.select.گڻو) {
				إذا (o->selected) {
					o->selected = false;
					dom_html_option_element_سيٽ_selected(
							o->ڳنڍ, false);
					control->معلومات.select.انگ_selected--;
				} وإلا {
					o->selected = true;
					dom_html_option_element_سيٽ_selected(
							o->ڳنڍ, true);
					control->معلومات.select.انگ_selected++;
				}
			} وإلا {
				dom_html_option_element_سيٽ_selected(
						o->ڳنڍ, true);
				o->selected = true;
			}
		}

		إذا (o->selected) {
			control->معلومات.select.current = o;
		}
	}

	talloc_free(inline_box->متن);
	inline_box->متن = 0;

	إذا (control->معلومات.select.انگ_selected == 0) {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وإلا إذا (control->معلومات.select.انگ_selected == 1) {
		inline_box->متن = talloc_strdup(html->bctx,
				control->معلومات.select.current->متن);
	} وإلا {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	إذا (!inline_box->متن) {
		ret = NSERROR_NOMEM;
		inline_box->ڊيگهائي = 0;
	} وإلا {
		inline_box->ڊيگهائي = لڙي_ڊيگهه(inline_box->متن);
	}
	inline_box->ويڪر = control->box->ويڪر;

	html__redraw_a_box(html, control->box);

	أرجع ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ساكن فراغ form_select_menu_clicked(بنية form_control *control, صحيح x, صحيح y)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	بنية form_option *option;
	html_content *html = (html_content *)menu->c;
	صحيح سٽ_اوچائي, سٽ_اوچائي_with_spacing;
	صحيح شي_bottom_y;
	صحيح scroll, i;

	scroll = scrollbar_get_وٿي(menu->scrollbar);

	سٽ_اوچائي = menu->سٽ_اوچائي;
	سٽ_اوچائي_with_spacing = سٽ_اوچائي +
			سٽ_اوچائي * SELECT_سٽ_SPACING;

	option = control->معلومات.select.شيون;
	شي_bottom_y = سٽ_اوچائي_with_spacing;
	i = 0;
	طالما (option && شي_bottom_y < scroll + y) {
		شي_bottom_y += سٽ_اوچائي_with_spacing;
		option = option->ايندڙ;
		i++;
	}

	إذا (option != عدم) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_معلومات, 0, 0, menu->ويڪر, menu->اوچائي);
}


/* exported interface documented in html/form_internal.h */
فراغ form_جوڙيو_control(بنية form *form, بنية form_control *control)
{
	إذا (form == عدم) {
		أرجع;
	}

	control->form = form;

	إذا (form->controls != عدم) {
		أكد(form->آخري_control);

		form->آخري_control->ايندڙ = control;
		control->prev = form->آخري_control;
		control->ايندڙ = عدم;
		form->آخري_control = control;
	} وإلا {
		form->controls = form->آخري_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_control(بنية form_control *control)
{
	بنية form_control *c;
	أكد(control != عدم);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->قدر, control->initial_قدر);
	حرر(control->name);
	حرر(control->قدر);
	حرر(control->initial_قدر);
	إذا (control->آخري_synced_قدر != عدم) {
		حرر(control->آخري_synced_قدر);
	}

	إذا (control->type == GADGET_SELECT) {
		بنية form_option *option, *ايندڙ;

		لكل (option = control->معلومات.select.شيون; option;
				option = ايندڙ) {
			ايندڙ = option->ايندڙ;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->متن, option->قدر);
			حرر(option->متن);
			حرر(option->قدر);
			حرر(option);
		}
		إذا (control->معلومات.select.menu != عدم) {
			form_free_select_menu(control);
		}
	}

	إذا (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		إذا (control->معلومات.متن.initial != عدم) {
			dom_string_unref(control->معلومات.متن.initial);
		}

		إذا (control->معلومات.متن.ta != عدم) {
			textarea_destroy(control->معلومات.متن.ta);
		}
	}

	/* unlink the control from the form */
	إذا (control->form != عدم) {
		لكل (c = control->form->controls; c != عدم; c = c->ايندڙ) {
			إذا (c->ايندڙ == control) {
				c->ايندڙ = control->ايندڙ;
				إذا (control->form->آخري_control == control)
					control->form->آخري_control = c;
				اكسر;
			}
			إذا (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->ايندڙ;
				إذا (control->form->آخري_control == control)
					control->form->controls =
						control->form->آخري_control = عدم;
				اكسر;
			}
		}
	}

	إذا (control->ڳنڍ_قدر != عدم) {
		dom_string_unref(control->ڳنڍ_قدر);
	}

	حرر(control);
}


/* exported interface documented in html/form_internal.h */
bool form_جوڙيو_option(بنية form_control *control, حرف *قدر, حرف *متن,
		     bool selected, فراغ *ڳنڍ)
{
	بنية form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = صفر_يادگيري_ڏيو(1, حجم *option);
	إذا (!option)
		أرجع false;

	option->قدر = قدر;
	option->متن = متن;

	/* add to linked list */
	إذا (control->معلومات.select.شيون == 0)
		control->معلومات.select.شيون = option;
	وإلا
		control->معلومات.select.آخري_شي->ايندڙ = option;
	control->معلومات.select.آخري_شي = option;

	/* set selected */
	إذا (selected && (control->معلومات.select.انگ_selected == 0 ||
			control->معلومات.select.گڻو)) {
		option->selected = option->initial_selected = true;
		control->معلومات.select.انگ_selected++;
		control->معلومات.select.current = option;
	}

	control->معلومات.select.انگ_شيون++;

	option->ڳنڍ = ڳنڍ;

	أرجع true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(فراغ *client_معلومات,
		      بنية form_control *control,
		      select_menu_redraw_callback callback,
		      بنية content *c)
{
	صحيح سٽ_اوچائي_with_spacing;
	بنية box *box;
	plot_font_style_t fstyle;
	صحيح ڪل_اوچائي;
	بنية form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	إذا (control->معلومات.select.menu == عدم) {

		menu = صفر_يادگيري_ڏيو(1, حجم (بنية form_select_menu));
		إذا (menu == عدم) {
			أرجع NSERROR_NOMEM;
		}

		control->معلومات.select.menu = menu;

		box = control->box;

		menu->ويڪر = box->ويڪر +
			box->border[ساڄو_2].ويڪر + box->padding[ساڄو_2] +
			box->border[کاٻو_2].ويڪر + box->padding[کاٻو_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_سائيز = fstyle.سائيز;

		menu->سٽ_اوچائي = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.سائيز / PLOT_STYLE_SCALE)))),
				F_72));

		سٽ_اوچائي_with_spacing = menu->سٽ_اوچائي +
				menu->سٽ_اوچائي *
				SELECT_سٽ_SPACING;

		ڪل_اوچائي = control->معلومات.select.انگ_شيون *
				سٽ_اوچائي_with_spacing;
		menu->اوچائي = ڪل_اوچائي;

		إذا (menu->اوچائي > MAX_SELECT_اوچائي) {
			menu->اوچائي = MAX_SELECT_اوچائي;
		}

		menu->client_معلومات = client_معلومات;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->اوچائي,
				       ڪل_اوچائي,
				       menu->اوچائي,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		إذا (res != NSERROR_OK) {
			control->معلومات.select.menu = عدم;
			حرر(menu);
			أرجع res;
		}
		menu->c = c;
	} وإلا {
		menu = control->معلومات.select.menu;
	}

	menu->callback(client_معلومات, 0, 0, menu->ويڪر, menu->اوچائي);

	أرجع NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_select_menu(بنية form_control *control)
{
	إذا (control->معلومات.select.menu->scrollbar != عدم)
		scrollbar_destroy(control->معلومات.select.menu->scrollbar);
	حرر(control->معلومات.select.menu);
	control->معلومات.select.menu = عدم;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(بنية form_control *control,
			صحيح x, صحيح y,
			عائم scale,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx)
{
	بنية box *box;
	بنية form_select_menu *menu = control->معلومات.select.menu;
	بنية form_option *option;
	صحيح سٽ_اوچائي, سٽ_اوچائي_with_spacing;
	صحيح ويڪر, اوچائي;
	صحيح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحيح شي_y;
	صحيح متن_pos_وٿي, متن_x;
	صحيح scrollbar_ويڪر = SCROLLBAR_ويڪر;
	صحيح i;
	صحيح scroll;
	صحيح x_cp, y_cp;
	بنية rect r;
	بنية rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	ويڪر = menu->ويڪر;
	اوچائي = menu->اوچائي;
	سٽ_اوچائي = menu->سٽ_اوچائي;

	سٽ_اوچائي_with_spacing = سٽ_اوچائي +
			سٽ_اوچائي * SELECT_سٽ_SPACING;
	scroll = scrollbar_get_وٿي(menu->scrollbar);

	إذا (scale != 1.0) {
		x *= scale;
		y *= scale;
		ويڪر *= scale;
		اوچائي *= scale;
		scrollbar_ويڪر *= scale;

		i = scroll / سٽ_اوچائي_with_spacing;
		scroll -= i * سٽ_اوچائي_with_spacing;
		سٽ_اوچائي *= scale;
		سٽ_اوچائي_with_spacing *= scale;
		scroll *= scale;
		scroll += i * سٽ_اوچائي_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + ويڪر - 1;
	y1 = y + اوچائي - 1;
	scrollbar_x = x1 - scrollbar_ويڪر;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	x0 = x0 + SELECT_BORDER_ويڪر;
	y0 = y0 + SELECT_BORDER_ويڪر;
	x1 = x1 - SELECT_BORDER_ويڪر;
	y1 = y1 - SELECT_BORDER_ويڪر;
	اوچائي = اوچائي - 2 * SELECT_BORDER_ويڪر;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	option = control->معلومات.select.شيون;
	شي_y = سٽ_اوچائي_with_spacing;

	طالما (شي_y < scroll) {
		option = option->ايندڙ;
		شي_y += سٽ_اوچائي_with_spacing;
	}
	شي_y -= سٽ_اوچائي_with_spacing;
	متن_pos_وٿي = y - scroll +
			(صحيح) (سٽ_اوچائي * (0.75 + SELECT_سٽ_SPACING));
	متن_x = x + (box->border[کاٻو_2].ويڪر + box->padding[کاٻو_2]) * scale;

	plot_fstyle_داخلا.سائيز = menu->f_سائيز;

	طالما (option && شي_y - scroll < اوچائي) {

		إذا (option->selected) {
			y2 = y + شي_y - scroll;
			y3 = y + شي_y + سٽ_اوچائي_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}

		y2 = متن_pos_وٿي + شي_y;
		res = ctx->plot->متن(ctx,
				      &plot_fstyle_داخلا,
				      متن_x, y2,
				      option->متن, لڙي_ڊيگهه(option->متن));
		إذا (res != NSERROR_OK) {
			أرجع false;
		}

		شي_y += سٽ_اوچائي_with_spacing;
		option = option->ايندڙ;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->ويڪر - SCROLLBAR_ويڪر,
			       y_cp,
			       clip, scale, ctx);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	أرجع true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(بنية form_control *control,
			     عائم scale,
			     ثابت بنية rect *clip)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	صحيح ويڪر, اوچائي;


	ويڪر = menu->ويڪر;
	اوچائي = menu->اوچائي;

	إذا (scale != 1.0) {
		ويڪر *= scale;
		اوچائي *= scale;
	}

	إذا (clip->x0 >= 0 &&
	    clip->x1 <= ويڪر &&
	    clip->y0 >= 0 &&
	    clip->y1 <= اوچائي)
		أرجع true;

	أرجع false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(بنية form_control *control, صحيح شي)
{
	أكد(control != عدم);

	أرجع form__select_process_selection(control->html, control, شي);
}


/* exported interface documented in netsurf/form.h */
بنية form_option *
form_select_get_option(بنية form_control *control, صحيح شي)
{
	بنية form_option *opt;

	opt = control->معلومات.select.شيون;
	طالما ((opt != عدم) && (شي > 0)) {
		opt = opt->ايندڙ;
		شي--;
	}
	أرجع opt;
}


/* exported interface documented in netsurf/form.h */
حرف *form_control_get_name(بنية form_control *control)
{
	أرجع control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(بنية form_control *control, بنية rect *r)
{
	box_bounds( control->box, r );
	أرجع NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ثابت حرف *
form_select_mouse_action(بنية form_control *control,
			 browser_mouse_state mouse,
			 صحيح x, صحيح y)
{
	بنية form_select_menu *menu = control->معلومات.select.menu;
	صحيح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *حالت = عدم;
	bool گڻو = control->معلومات.select.گڻو;

	x0 = 0;
	y0 = 0;
	x1 = menu->ويڪر;
	y1 = menu->اوچائي;
	scrollbar_x = x1 - SCROLLBAR_ويڪر;

	إذا (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		أرجع scrollbar_mouse_حالت_منزل_پيغام(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	إذا (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		إذا (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		إذا (!(mouse & BROWSER_MOUSE_CLICK_1 && !گڻو))
			/* anything but a button 1 click over a single select
			   menu */
			حالت = messages_get(control->معلومات.select.گڻو ?
					"SelectMClick" : "SelectClick");

	} وإلا إذا (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		حالت = messages_get("SelectClose");

	أرجع حالت;
}


/* private interface described in html/form_internal.h */
فراغ
form_select_mouse_drag_پڇاڙي(بنية form_control *control,
			   browser_mouse_state mouse,
			   صحيح x, صحيح y)
{
	صحيح x0, y0, x1, y1;
	صحيح box_x, box_y;
	بنية box *box;
	بنية form_select_menu *menu = control->معلومات.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[کاٻو_2].ويڪر;
	box_y += box->اوچائي + box->border[BOTTOM].ويڪر +
			box->padding[BOTTOM] + box->padding[چوٽي_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	إذا (menu->scroll_capture) {
		x -= menu->ويڪر - SCROLLBAR_ويڪر;
		scrollbar_mouse_drag_پڇاڙي(menu->scrollbar, mouse, x, y);
		أرجع;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->ويڪر;
	y1 = menu->اوچائي;


	إذا (x > x0 && x < x1 - SCROLLBAR_ويڪر && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *ويڪر, صحيح *اوچائي)
{
	*ويڪر = control->معلومات.select.menu->ويڪر;
	*اوچائي = control->معلومات.select.menu->اوچائي;
}


/* private interface described in html/form_internal.h */
فراغ form_select_menu_callback(فراغ *client_معلومات,
		صحيح x, صحيح y, صحيح ويڪر, صحيح اوچائي)
{
	html_content *html = client_معلومات;
	صحيح menu_x, menu_y;
	بنية box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[کاٻو_2].ويڪر;
	menu_y += box->اوچائي + box->border[BOTTOM].ويڪر +
			box->padding[BOTTOM] +
			box->padding[چوٽي_2];
	content__request_redraw((بنية content *)html, menu_x + x, menu_y + y,
			ويڪر, اوچائي);
}


/* private interface described in html/form_internal.h */
فراغ form_radio_سيٽ(بنية form_control *radio)
{
	بنية form_control *control;

	أكد(radio);
	إذا (!radio->form)
		أرجع;

	إذا (radio->selected)
		أرجع;

	/* Clear selected state for other controls in
	 * the same radio button group */
	لكل (control = radio->form->controls;
	     control != عدم;
	     control = control->ايندڙ) {
		/* Only interested in radio inputs */
		إذا (control->type != GADGET_RADIO)
			استمر;

		/* Ignore ourself */
		إذا (control == radio)
			استمر;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		إذا ((control->name == عدم) ||
		    (radio->name == عدم) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			استمر;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		إذا (control->selected) {
			control->selected = false;
			dom_html_داخلا_element_سيٽ_checked(control->ڳنڍ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_داخلا_element_سيٽ_checked(radio->ڳنڍ, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *صفحو_url,
	    بنية browser_دري *هدف,
	    بنية form *form,
	    بنية form_control *submit_button)
{
	nserror res;
	حرف *معلومات = عدم; /* encoded form data */
	بنية fetch_multipart_معلومات *success = عدم; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != عدم);

	/* obtain list of controls from DOM */
	res = form_dom_منزل_معلومات(form, submit_button, &success);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	إذا (res != NSERROR_OK) {
		fetch_multipart_معلومات_destroy(success);
		أرجع res;
	}

	بدل (form->طريقو) {
	حالة طريقو_GET:
		res = form_url_encode(form, success, &معلومات);
		إذا (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, معلومات, &query_url);
			إذا (res == NSERROR_OK) {
				res = browser_دري_navigate(هدف,
							      query_url,
							      صفحو_url,
							      BW_NAVIGATE_HISTORY,
							      عدم,
							      عدم,
							      عدم);

				nsurl_unref(query_url);
			}
			حرر(معلومات);
		}
		اكسر;

	حالة طريقو_POST_URLENC:
		res = form_url_encode(form, success, &معلومات);
		إذا (res == NSERROR_OK) {
			res = browser_دري_navigate(هدف,
						      action_url,
						      صفحو_url,
						      BW_NAVIGATE_HISTORY,
						      معلومات,
						      عدم,
						      عدم);
			حرر(معلومات);
		}
		اكسر;

	حالة طريقو_POST_MULTIPART:
		res = browser_دري_navigate(هدف,
					      action_url,
					      صفحو_url,
					      BW_NAVIGATE_HISTORY,
					      عدم,
					      success,
					      عدم);

		اكسر;
	}

	nsurl_unref(action_url);
	fetch_multipart_معلومات_destroy(success);

	أرجع res;
}


/* exported interface documented in html/form_internal.h */
فراغ form_gadget_update_قدر(بنية form_control *control, حرف *قدر)
{
	بدل (control->type) {
	حالة GADGET_HIDDEN:
	حالة GADGET_TEXTBOX:
	حالة GADGET_TEXTAREA:
	حالة GADGET_PASSWORD:
	حالة GADGET_FILE:
		إذا (control->قدر != عدم) {
			حرر(control->قدر);
		}
		control->قدر = قدر;
		إذا (control->ڳنڍ != عدم) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)قدر,
						لڙي_ڊيگهه(قدر), &str);
			إذا (err == DOM_NO_ERR) {
				إذا (control->type == GADGET_TEXTAREA)
					err = dom_html_متن_area_element_سيٽ_قدر(
						(dom_html_متن_area_element *)(control->ڳنڍ),
						str);
				وإلا
					err = dom_html_داخلا_element_سيٽ_قدر(
						(dom_html_داخلا_element *)(control->ڳنڍ),
						str);
				dom_string_unref(str);
			}
		}
		اكسر;
	افتراضي:
		/* Do nothing */
		اكسر;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
فراغ
form_gadget_sync_with_dom(بنية form_control *control)
{
	dom_exception exc;
	dom_string *قدر = عدم;
	bool بدليو_dom = false;

	إذا (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		أرجع;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	إذا ((control->آخري_synced_قدر == عدم &&
	     control->قدر != عدم &&
	     control->قدر[0] != '\0') ||
	    (control->آخري_synced_قدر != عدم &&
	     control->قدر != عدم &&
	     strcmp(control->قدر, control->آخري_synced_قدر) != 0)) {
		حرف *dup = strdup(control->قدر);
		إذا (dup == عدم) {
			اذهب ٻاهريون;
		}
		إذا (control->آخري_synced_قدر != عدم) {
			حرر(control->آخري_synced_قدر);
		}
		control->آخري_synced_قدر = dup;
		exc = dom_string_create((uint8_t *)(control->قدر),
					لڙي_ڊيگهه(control->قدر), &قدر);
		إذا (exc != DOM_NO_ERR) {
			اذهب ٻاهريون;
		}
		إذا (control->ڳنڍ_قدر != عدم) {
			dom_string_unref(control->ڳنڍ_قدر);
		}
		control->ڳنڍ_قدر = قدر;
		قدر = عدم;
		إذا (control->type == GADGET_TEXTAREA) {
			exc = dom_html_متن_area_element_سيٽ_قدر(control->ڳنڍ, control->ڳنڍ_قدر);
		} وإلا {
			exc = dom_html_داخلا_element_سيٽ_قدر(control->ڳنڍ, control->ڳنڍ_قدر);
		}
		إذا (exc != DOM_NO_ERR) {
			اذهب ٻاهريون;
		}
		بدليو_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	إذا (control->type == GADGET_TEXTAREA) {
		exc = dom_html_متن_area_element_get_قدر(control->ڳنڍ, &قدر);
	} وإلا {
		exc = dom_html_داخلا_element_get_قدر(control->ڳنڍ, &قدر);
	}

	إذا (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		اذهب ٻاهريون;
	}

	إذا (!dom_string_isequal(control->ڳنڍ_قدر, قدر)) {
		/* The DOM has changed */
		إذا (!بدليو_dom) {
			/* And it wasn't us */
			حرف *قدر_s = strndup(
				dom_string_معلومات(قدر),
				dom_string_byte_ڊيگهائي(قدر));
			حرف *dup = عدم;
			إذا (قدر_s == عدم) {
				اذهب ٻاهريون;
			}
			dup = strdup(قدر_s);
			إذا (dup == عدم) {
				حرر(قدر_s);
				اذهب ٻاهريون;
			}
			حرر(control->قدر);
			control->قدر = قدر_s;
			حرر(control->آخري_synced_قدر);
			control->آخري_synced_قدر = dup;
			إذا (control->type != GADGET_HIDDEN &&
			    control->معلومات.متن.ta != عدم) {
				textarea_سيٽ_متن(control->معلومات.متن.ta,
						  قدر_s);
			}
		}
		control->ڳنڍ_قدر = قدر;
		قدر = عدم;
	}

ٻاهريون:
	إذا (قدر != عدم)
		dom_string_unref(قدر);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
بنية form *
form_new(فراغ *ڳنڍ,
	 ثابت حرف *action,
	 ثابت حرف *هدف,
	 form_طريقو طريقو,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	بنية form *form;

	form = صفر_يادگيري_ڏيو(1, حجم *form);
	إذا (!form)
		أرجع عدم;

	form->action = strdup(action != عدم ? action : "");
	إذا (form->action == عدم) {
		حرر(form);
		أرجع عدم;
	}

	form->هدف = هدف != عدم ? strdup(هدف) : عدم;
	إذا (هدف != عدم && form->هدف == عدم) {
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->طريقو = طريقو;

	form->accept_charsets = charset != عدم ? strdup(charset) : عدم;
	إذا (charset != عدم && form->accept_charsets == عدم) {
		حرر(form->هدف);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->document_charset = doc_charset != عدم ? strdup(doc_charset)
						     : عدم;
	إذا (doc_charset && form->document_charset == عدم) {
		حرر(form->accept_charsets);
		حرر(form->هدف);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->ڳنڍ = ڳنڍ;

	أرجع form;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free(بنية form *form)
{
	بنية form_control *c, *d;

	لكل (c = form->controls; c != عدم; c = d) {
		d = c->ايندڙ;

		form_free_control(c);
	}

	حرر(form->action);
	حرر(form->هدف);
	حرر(form->accept_charsets);
	حرر(form->document_charset);

	حرر(form);
}


/* exported interface documented in html/form_internal.h */
بنية form_control *form_new_control(فراغ *ڳنڍ, form_control_type type)
{
	بنية form_control *control;

	control = صفر_يادگيري_ڏيو(1, حجم *control);
	إذا (control == عدم)
		أرجع عدم;

	control->ڳنڍ = ڳنڍ;
	control->type = type;

	أرجع control;
}
