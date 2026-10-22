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

#تعریف_کرو MAX_SELECT_اچائی 210
#تعریف_کرو SELECT_سطر_SPACING 0.2
#تعریف_کرو SELECT_BORDER_چوڑائی 1
#تعریف_کرو SELECT_SELECTED_رنگ 0xDB9370

بنية form_select_menu {
	صحيح سطر_اچائی;
	صحيح چوڑائی, اچائی;
	بنية scrollbar *scrollbar;
	صحيح f_ناپ;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	فراغ *client_انکڑے;
	بنية content *c;
};

ساكن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_رنگ = SELECT_SELECTED_رنگ,
};

ساكن plot_font_style_t plot_fstyle_اندراج = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.بھار = 400,
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
form_encode_شے(ثابت حرف *شے,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = عدم;
	حرف cset[256];

	إذا (!شے || !charset)
		أرجع عدم;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_منزل_enc(شے, cset, 0, &ret);
	إذا (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_منزل_enc(شے, cset, len, &ret);

		إذا (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			إذا (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_منزل_enc(شے, cset, 0, &ret);

				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_منزل_enc(شے, cset, 0, &ret);
				}
			}

			إذا (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_منزل_enc(شے, "ISO-8859-1//TRANSLIT",
						0, &ret);
				إذا (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_منزل_enc(شے, "ISO-8859-1",
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
#تعریف_کرو FETCH_انکڑے_INT_قدر_ناپ 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساكن nserror
fetch_انکڑے_list_جوڑو_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحيح قدر,
			  بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	بنية fetch_multipart_انکڑے *fetch_انکڑے;
	صحيح keysize;

	fetch_انکڑے = صفر_حافظہ_دیو(1, حجم(*fetch_انکڑے));
	إذا (fetch_انکڑے == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_انکڑے->name, 0, "%s%s", name, ksfx);
	fetch_انکڑے->name = حافظہ_دیو(keysize + 1); /* allow for null */
	إذا (fetch_انکڑے->name == عدم) {
		حرر(fetch_انکڑے);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_انکڑے->name, keysize + 1, "%s%s", name, ksfx);

	/*قدر*/
	fetch_انکڑے->قدر = حافظہ_دیو(FETCH_انکڑے_INT_قدر_ناپ);
	إذا (fetch_انکڑے->قدر == عدم) {
		حرر(fetch_انکڑے->name);
		حرر(fetch_انکڑے);
		NSLOG(netsurf, INFO, "value allocation failure");
		أرجع NSERROR_NOMEM;
	}
	snprintf(fetch_انکڑے->قدر, FETCH_انکڑے_INT_قدر_ناپ, "%d", قدر);

	/* link into list */
	**fetch_انکڑے_اگلا_ptr = fetch_انکڑے;
	*fetch_انکڑے_اگلا_ptr = &fetch_انکڑے->اگلا;

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
fetch_انکڑے_list_جوڑو(dom_string *name,
		    dom_string *قدر,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	بنية fetch_multipart_انکڑے *fetch_انکڑے;

	أكد(name != عدم);

	fetch_انکڑے = صفر_حافظہ_دیو(1, حجم(*fetch_انکڑے));
	إذا (fetch_انکڑے == عدم) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		أرجع NSERROR_NOMEM;
	}

	fetch_انکڑے->name = form_encode_شے(dom_string_انکڑے(name),
					    dom_string_byte_لمائی(name),
					    form_charset,
					    docu_charset);
	إذا (fetch_انکڑے->name == عدم) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		حرر(fetch_انکڑے);
		أرجع NSERROR_NOMEM;
	}

	إذا (قدر == عدم) {
		fetch_انکڑے->قدر = strdup("");
	} وإلا {
		fetch_انکڑے->قدر = form_encode_شے(dom_string_انکڑے(قدر),
						     dom_string_byte_لمائی(قدر),
						     form_charset,
						     docu_charset);
	}
	إذا (fetch_انکڑے->قدر == عدم) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		حرر(fetch_انکڑے->name);
		حرر(fetch_انکڑے);
		أرجع NSERROR_NOMEM;
	}

	/* deal with raw file name */
	إذا (rawfile != عدم) {
		fetch_انکڑے->file = true;
		fetch_انکڑے->rawfile = strdup(rawfile);
		إذا (fetch_انکڑے->rawfile == عدم) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			حرر(fetch_انکڑے->قدر);
			حرر(fetch_انکڑے->name);
			حرر(fetch_انکڑے);
			أرجع NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_انکڑے_اگلا_ptr = fetch_انکڑے;
	*fetch_انکڑے_اگلا_ptr = &fetch_انکڑے->اگلا;

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
form_dom_منزل_انکڑے_textarea(dom_html_متن_area_element *متن_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
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
	res = fetch_انکڑے_list_جوڑو(inputname,
				  inputvalue,
				  عدم,
				  form_charset,
				  doc_charset,
				  fetch_انکڑے_اگلا_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	أرجع res;
}


ساكن nserror
form_dom_منزل_انکڑے_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
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
	res = fetch_انکڑے_list_جوڑو(keyname,
				  قدر,
				  عدم,
				  form_charset,
				  docu_charset,
				  fetch_انکڑے_اگلا_ptr);

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
form_dom_منزل_انکڑے_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = عدم;
	uint32_t options_گنتی;
	uint32_t option_اشاریہ;
	dom_گرہ *option_element = عدم;

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
	قوت_نما = dom_html_options_collection_get_لمائی(options, &options_گنتی);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* iterate over options collection */
	لكل (option_اشاریہ = 0; option_اشاریہ < options_گنتی; ++option_اشاریہ) {
		قوت_نما = dom_html_options_collection_شے(options,
						       option_اشاریہ,
						       &option_element);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_اشاریہ);
			res = NSERROR_DOM;
		} وإلا {
			res = form_dom_منزل_انکڑے_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_انکڑے_اگلا_ptr);

			dom_گرہ_unref(option_element);
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
form_dom_منزل_انکڑے_داخلہ_submit(dom_html_داخلہ_element *داخلہ_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	إذا (*submit_button == عدم) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)داخلہ_element;
	} وإلا إذا (*submit_button != (dom_html_element *)داخلہ_element) {
		أرجع NSERROR_OK;
	}

	/* matched button used to submit form */
	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_انکڑے_list_جوڑو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_انکڑے_اگلا_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_انکڑے_داخلہ_image(dom_html_داخلہ_element *داخلہ_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	بنية image_داخلہ_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	إذا (*submit_button != (dom_html_element *)داخلہ_element) {
		أرجع NSERROR_OK;
	}

	قوت_نما = dom_گرہ_get_user_انکڑے((dom_گرہ *)داخلہ_element,
				     corestring_dom_ns_کلید_image_coords_گرہ_انکڑے,
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
	basename = form_encode_شے(dom_string_انکڑے(inputname),
				    dom_string_byte_لمائی(inputname),
				    charset,
				    document_charset);
	إذا (basename == عدم) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		أرجع NSERROR_NOMEM;
	}

	res = fetch_انکڑے_list_جوڑو_sname(basename, ".x",
					coords->x,
					fetch_انکڑے_اگلا_ptr);

	إذا (res == NSERROR_OK) {
		res = fetch_انکڑے_list_جوڑو_sname(basename, ".y",
						coords->y,
						fetch_انکڑے_اگلا_ptr);
	}

	حرر(basename);

	أرجع res;
}


ساكن nserror
form_dom_منزل_انکڑے_داخلہ_checkbox(dom_html_داخلہ_element *داخلہ_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلہ_element_get_checked(داخلہ_element, &checked);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		أرجع NSERROR_DOM;
	}

	إذا (!checked) {
		/* unchecked items do not generate a data entry */
		أرجع NSERROR_OK;
	}

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
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
	res = fetch_انکڑے_list_جوڑو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_انکڑے_اگلا_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_انکڑے_داخلہ_file(dom_html_داخلہ_element *داخلہ_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = عدم;

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		أرجع NSERROR_DOM;
	}

	قوت_نما = dom_گرہ_get_user_انکڑے((dom_گرہ *)داخلہ_element,
				     corestring_dom_ns_کلید_file_name_گرہ_انکڑے,
				     &rawfile);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		أرجع NSERROR_DOM;
	}

	إذا (rawfile == عدم) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_انکڑے_list_جوڑو(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_انکڑے_اگلا_ptr);

	dom_string_unref(inputvalue);

	أرجع res;
}


ساكن nserror
form_dom_منزل_انکڑے_داخلہ_متن(dom_html_داخلہ_element *داخلہ_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		أرجع NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_انکڑے_list_جوڑو(inputname,
				  inputvalue,
				  عدم,
				  charset,
				  document_charset,
				  fetch_انکڑے_اگلا_ptr);

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
form_dom_منزل_انکڑے_داخلہ(dom_html_داخلہ_element *داخلہ_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	قوت_نما = dom_html_داخلہ_element_get_disabled(داخلہ_element,
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
	قوت_نما = dom_html_داخلہ_element_get_name(داخلہ_element, &inputname);
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
	قوت_نما = dom_html_داخلہ_element_get_type(داخلہ_element, &inputtype);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		أرجع NSERROR_DOM;
	}

	/* process according to input element type */
	إذا (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_منزل_انکڑے_داخلہ_submit(داخلہ_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_انکڑے_اگلا_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_منزل_انکڑے_داخلہ_image(داخلہ_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_انکڑے_اگلا_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_منزل_انکڑے_داخلہ_checkbox(داخلہ_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_انکڑے_اگلا_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_منزل_انکڑے_داخلہ_file(داخلہ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_انکڑے_اگلا_ptr);

	} وإلا إذا (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} وإلا {
		/* Everything else is treated as text values */
		res = form_dom_منزل_انکڑے_داخلہ_متن(داخلہ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_انکڑے_اگلا_ptr);

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
form_dom_منزل_انکڑے_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			بنية fetch_multipart_انکڑے ***fetch_انکڑے_اگلا_ptr)
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
		res = fetch_انکڑے_list_جوڑو(inputname,
					  inputvalue,
					  عدم,
					  form_charset,
					  doc_charset,
					  fetch_انکڑے_اگلا_ptr);

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
	حرف *عارضی, *c;

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
	عارضی = strdup(form->accept_charsets);
	إذا (!عارضی)
		أرجع عدم;

	/* make it upper case */
	لكل (c = عارضی; *c; c++) {
		*c = ascii_منزل_اتلا(*c);
	}

	/* is UTF-8 specified? */
	c = لڑی_کھوج(عارضی, "UTF-8");
	إذا (c) {
		حرر(عارضی);
		أرجع strdup("UTF-8");
	}

	/* dispense with temporary copy */
	حرر(عارضی);

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
form_dom_منزل_انکڑے(بنية form *form,
		 بنية form_control *submit_control,
		 بنية fetch_multipart_انکڑے **fetch_انکڑے_باہرلا)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_html_collection *elements = عدم; /* the dom form elements */
	uint32_t element_گنتی; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_گرہ *element = عدم; /* the DOM form element */
	dom_string *nodename = عدم; /* the DOM node name of the element */
	بنية fetch_multipart_انکڑے *fetch_انکڑے = عدم; /* fetch data list */
	بنية fetch_multipart_انکڑے **fetch_انکڑے_اگلا = &fetch_انکڑے;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	إذا (submit_control != عدم) {
		submit_button = submit_control->گرہ;
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
	قوت_نما = dom_html_form_element_get_elements(form->گرہ, &elements);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		حرر(charset);
		أرجع NSERROR_DOM;
	}

	قوت_نما = dom_html_collection_get_لمائی(elements, &element_گنتی);
	إذا (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		اذهب form_dom_منزل_انکڑے_error;
	}

	لكل (element_idx = 0; element_idx < element_گنتی; element_idx++) {
		/* obtain a form element */
		قوت_نما = dom_html_collection_شے(elements, element_idx, &element);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_انکڑے_error;
		}

		/* node name from element */
		قوت_نما = dom_گرہ_get_گرہ_name(element, &nodename);
		إذا (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			dom_گرہ_unref(element);
			res = NSERROR_DOM;
			اذهب form_dom_منزل_انکڑے_error;
		}

		إذا (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_منزل_انکڑے_textarea(
				(dom_html_متن_area_element *)element,
				charset,
				form->document_charset,
				&fetch_انکڑے_اگلا);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_منزل_انکڑے_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_انکڑے_اگلا);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_داخلہ)) {
			/* Form element is HTMLInputElement */
			res = form_dom_منزل_انکڑے_داخلہ(
				(dom_html_داخلہ_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_انکڑے_اگلا);

		} وإلا إذا (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_منزل_انکڑے_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_انکڑے_اگلا);

		} وإلا {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحيح)dom_string_byte_لمائی(nodename),
			      dom_string_انکڑے(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_گرہ_unref(element);

		/* abort form element enumeration on error */
		إذا (res != NSERROR_OK) {
			اذهب form_dom_منزل_انکڑے_error;
		}
	}

	*fetch_انکڑے_باہرلا = fetch_انکڑے;
	dom_html_collection_unref(elements);
	حرر(charset);

	أرجع NSERROR_OK;

form_dom_منزل_انکڑے_error:
	fetch_multipart_انکڑے_destroy(fetch_انکڑے);
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
		بنية fetch_multipart_انکڑے *control,
		حرف **encoded_باہرلا)
{
	حرف *name, *قدر;
	حرف *s, *s2;
	بنانشانی صحيح len, len1, len_init;
	nserror res;

	s = حافظہ_دیو(1);

	إذا (s == عدم) {
		أرجع NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	لكل (; control; control = control->اگلا) {
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
		len1 = len + لڑی_لمبائی(name) + لڑی_لمبائی(قدر) + 2;
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

	*encoded_باہرلا = s;

	أرجع NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساكن فراغ
form_select_menu_scroll_callback(فراغ *client_انکڑے,
				 بنية scrollbar_پیغام_انکڑے *scrollbar_انکڑے)
{
	بنية form_control *control = client_انکڑے;
	بنية form_select_menu *menu = control->انکڑے.select.menu;
	html_content *html = (html_content *)menu->c;

	بدل (scrollbar_انکڑے->پیغام) {
		حالة SCROLLBAR_پیغام_MOVED:
			menu->callback(menu->client_انکڑے,
					0, 0,
					menu->چوڑائی,
					menu->اچائی);
			اكسر;
		حالة SCROLLBAR_پیغام_SCROLL_شروع:
		{
			بنية rect rect = {
				.x0 = scrollbar_انکڑے->x0,
				.y0 = scrollbar_انکڑے->y0,
				.x1 = scrollbar_انکڑے->x1,
				.y1 = scrollbar_انکڑے->y1
			};

			browser_کھڑکی_سیٹ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			اكسر;
		حالة SCROLLBAR_پیغام_SCROLL_مکمل:
			menu->scroll_capture = false;

			browser_کھڑکی_سیٹ_drag_type(html->bw,
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
			       صحيح شے)
{
	بنية box *inline_box;
	بنية form_option *o;
	صحيح گنتی;
	nserror ret = NSERROR_OK;

	أكد(control != عدم);
	أكد(html != عدم);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	لكل (گنتی = 0, o = control->انکڑے.select.اشیاء;
			o != عدم;
			گنتی++, o = o->اگلا) {
		إذا (!control->انکڑے.select.مضرب && o->selected) {
			o->selected = false;
			dom_html_option_element_سیٹ_selected(o->گرہ, false);
		}

		إذا (گنتی == شے) {
			إذا (control->انکڑے.select.مضرب) {
				إذا (o->selected) {
					o->selected = false;
					dom_html_option_element_سیٹ_selected(
							o->گرہ, false);
					control->انکڑے.select.عدد_selected--;
				} وإلا {
					o->selected = true;
					dom_html_option_element_سیٹ_selected(
							o->گرہ, true);
					control->انکڑے.select.عدد_selected++;
				}
			} وإلا {
				dom_html_option_element_سیٹ_selected(
						o->گرہ, true);
				o->selected = true;
			}
		}

		إذا (o->selected) {
			control->انکڑے.select.current = o;
		}
	}

	talloc_free(inline_box->متن);
	inline_box->متن = 0;

	إذا (control->انکڑے.select.عدد_selected == 0) {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} وإلا إذا (control->انکڑے.select.عدد_selected == 1) {
		inline_box->متن = talloc_strdup(html->bctx,
				control->انکڑے.select.current->متن);
	} وإلا {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	إذا (!inline_box->متن) {
		ret = NSERROR_NOMEM;
		inline_box->لمائی = 0;
	} وإلا {
		inline_box->لمائی = لڑی_لمبائی(inline_box->متن);
	}
	inline_box->چوڑائی = control->box->چوڑائی;

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
	بنية form_select_menu *menu = control->انکڑے.select.menu;
	بنية form_option *option;
	html_content *html = (html_content *)menu->c;
	صحيح سطر_اچائی, سطر_اچائی_with_spacing;
	صحيح شے_bottom_y;
	صحيح scroll, i;

	scroll = scrollbar_get_وتھ(menu->scrollbar);

	سطر_اچائی = menu->سطر_اچائی;
	سطر_اچائی_with_spacing = سطر_اچائی +
			سطر_اچائی * SELECT_سطر_SPACING;

	option = control->انکڑے.select.اشیاء;
	شے_bottom_y = سطر_اچائی_with_spacing;
	i = 0;
	طالما (option && شے_bottom_y < scroll + y) {
		شے_bottom_y += سطر_اچائی_with_spacing;
		option = option->اگلا;
		i++;
	}

	إذا (option != عدم) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_انکڑے, 0, 0, menu->چوڑائی, menu->اچائی);
}


/* exported interface documented in html/form_internal.h */
فراغ form_جوڑو_control(بنية form *form, بنية form_control *control)
{
	إذا (form == عدم) {
		أرجع;
	}

	control->form = form;

	إذا (form->controls != عدم) {
		أكد(form->آخری_control);

		form->آخری_control->اگلا = control;
		control->prev = form->آخری_control;
		control->اگلا = عدم;
		form->آخری_control = control;
	} وإلا {
		form->controls = form->آخری_control = control;
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
	إذا (control->آخری_synced_قدر != عدم) {
		حرر(control->آخری_synced_قدر);
	}

	إذا (control->type == GADGET_SELECT) {
		بنية form_option *option, *اگلا;

		لكل (option = control->انکڑے.select.اشیاء; option;
				option = اگلا) {
			اگلا = option->اگلا;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->متن, option->قدر);
			حرر(option->متن);
			حرر(option->قدر);
			حرر(option);
		}
		إذا (control->انکڑے.select.menu != عدم) {
			form_free_select_menu(control);
		}
	}

	إذا (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		إذا (control->انکڑے.متن.initial != عدم) {
			dom_string_unref(control->انکڑے.متن.initial);
		}

		إذا (control->انکڑے.متن.ta != عدم) {
			textarea_destroy(control->انکڑے.متن.ta);
		}
	}

	/* unlink the control from the form */
	إذا (control->form != عدم) {
		لكل (c = control->form->controls; c != عدم; c = c->اگلا) {
			إذا (c->اگلا == control) {
				c->اگلا = control->اگلا;
				إذا (control->form->آخری_control == control)
					control->form->آخری_control = c;
				اكسر;
			}
			إذا (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->اگلا;
				إذا (control->form->آخری_control == control)
					control->form->controls =
						control->form->آخری_control = عدم;
				اكسر;
			}
		}
	}

	إذا (control->گرہ_قدر != عدم) {
		dom_string_unref(control->گرہ_قدر);
	}

	حرر(control);
}


/* exported interface documented in html/form_internal.h */
bool form_جوڑو_option(بنية form_control *control, حرف *قدر, حرف *متن,
		     bool selected, فراغ *گرہ)
{
	بنية form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = صفر_حافظہ_دیو(1, حجم *option);
	إذا (!option)
		أرجع false;

	option->قدر = قدر;
	option->متن = متن;

	/* add to linked list */
	إذا (control->انکڑے.select.اشیاء == 0)
		control->انکڑے.select.اشیاء = option;
	وإلا
		control->انکڑے.select.آخری_شے->اگلا = option;
	control->انکڑے.select.آخری_شے = option;

	/* set selected */
	إذا (selected && (control->انکڑے.select.عدد_selected == 0 ||
			control->انکڑے.select.مضرب)) {
		option->selected = option->initial_selected = true;
		control->انکڑے.select.عدد_selected++;
		control->انکڑے.select.current = option;
	}

	control->انکڑے.select.عدد_اشیاء++;

	option->گرہ = گرہ;

	أرجع true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(فراغ *client_انکڑے,
		      بنية form_control *control,
		      select_menu_redraw_callback callback,
		      بنية content *c)
{
	صحيح سطر_اچائی_with_spacing;
	بنية box *box;
	plot_font_style_t fstyle;
	صحيح کل_اچائی;
	بنية form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	إذا (control->انکڑے.select.menu == عدم) {

		menu = صفر_حافظہ_دیو(1, حجم (بنية form_select_menu));
		إذا (menu == عدم) {
			أرجع NSERROR_NOMEM;
		}

		control->انکڑے.select.menu = menu;

		box = control->box;

		menu->چوڑائی = box->چوڑائی +
			box->border[دایاں_2].چوڑائی + box->padding[دایاں_2] +
			box->border[بایاں_2].چوڑائی + box->padding[بایاں_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_ناپ = fstyle.ناپ;

		menu->سطر_اچائی = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.ناپ / PLOT_STYLE_SCALE)))),
				F_72));

		سطر_اچائی_with_spacing = menu->سطر_اچائی +
				menu->سطر_اچائی *
				SELECT_سطر_SPACING;

		کل_اچائی = control->انکڑے.select.عدد_اشیاء *
				سطر_اچائی_with_spacing;
		menu->اچائی = کل_اچائی;

		إذا (menu->اچائی > MAX_SELECT_اچائی) {
			menu->اچائی = MAX_SELECT_اچائی;
		}

		menu->client_انکڑے = client_انکڑے;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->اچائی,
				       کل_اچائی,
				       menu->اچائی,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		إذا (res != NSERROR_OK) {
			control->انکڑے.select.menu = عدم;
			حرر(menu);
			أرجع res;
		}
		menu->c = c;
	} وإلا {
		menu = control->انکڑے.select.menu;
	}

	menu->callback(client_انکڑے, 0, 0, menu->چوڑائی, menu->اچائی);

	أرجع NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free_select_menu(بنية form_control *control)
{
	إذا (control->انکڑے.select.menu->scrollbar != عدم)
		scrollbar_destroy(control->انکڑے.select.menu->scrollbar);
	حرر(control->انکڑے.select.menu);
	control->انکڑے.select.menu = عدم;
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
	بنية form_select_menu *menu = control->انکڑے.select.menu;
	بنية form_option *option;
	صحيح سطر_اچائی, سطر_اچائی_with_spacing;
	صحيح چوڑائی, اچائی;
	صحيح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحيح شے_y;
	صحيح متن_pos_وتھ, متن_x;
	صحيح scrollbar_چوڑائی = SCROLLBAR_چوڑائی;
	صحيح i;
	صحيح scroll;
	صحيح x_cp, y_cp;
	بنية rect r;
	بنية rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	چوڑائی = menu->چوڑائی;
	اچائی = menu->اچائی;
	سطر_اچائی = menu->سطر_اچائی;

	سطر_اچائی_with_spacing = سطر_اچائی +
			سطر_اچائی * SELECT_سطر_SPACING;
	scroll = scrollbar_get_وتھ(menu->scrollbar);

	إذا (scale != 1.0) {
		x *= scale;
		y *= scale;
		چوڑائی *= scale;
		اچائی *= scale;
		scrollbar_چوڑائی *= scale;

		i = scroll / سطر_اچائی_with_spacing;
		scroll -= i * سطر_اچائی_with_spacing;
		سطر_اچائی *= scale;
		سطر_اچائی_with_spacing *= scale;
		scroll *= scale;
		scroll += i * سطر_اچائی_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + چوڑائی - 1;
	y1 = y + اچائی - 1;
	scrollbar_x = x1 - scrollbar_چوڑائی;

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

	x0 = x0 + SELECT_BORDER_چوڑائی;
	y0 = y0 + SELECT_BORDER_چوڑائی;
	x1 = x1 - SELECT_BORDER_چوڑائی;
	y1 = y1 - SELECT_BORDER_چوڑائی;
	اچائی = اچائی - 2 * SELECT_BORDER_چوڑائی;

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

	option = control->انکڑے.select.اشیاء;
	شے_y = سطر_اچائی_with_spacing;

	طالما (شے_y < scroll) {
		option = option->اگلا;
		شے_y += سطر_اچائی_with_spacing;
	}
	شے_y -= سطر_اچائی_with_spacing;
	متن_pos_وتھ = y - scroll +
			(صحيح) (سطر_اچائی * (0.75 + SELECT_سطر_SPACING));
	متن_x = x + (box->border[بایاں_2].چوڑائی + box->padding[بایاں_2]) * scale;

	plot_fstyle_اندراج.ناپ = menu->f_ناپ;

	طالما (option && شے_y - scroll < اچائی) {

		إذا (option->selected) {
			y2 = y + شے_y - scroll;
			y3 = y + شے_y + سطر_اچائی_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}

		y2 = متن_pos_وتھ + شے_y;
		res = ctx->plot->متن(ctx,
				      &plot_fstyle_اندراج,
				      متن_x, y2,
				      option->متن, لڑی_لمبائی(option->متن));
		إذا (res != NSERROR_OK) {
			أرجع false;
		}

		شے_y += سطر_اچائی_with_spacing;
		option = option->اگلا;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->چوڑائی - SCROLLBAR_چوڑائی,
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
	بنية form_select_menu *menu = control->انکڑے.select.menu;
	صحيح چوڑائی, اچائی;


	چوڑائی = menu->چوڑائی;
	اچائی = menu->اچائی;

	إذا (scale != 1.0) {
		چوڑائی *= scale;
		اچائی *= scale;
	}

	إذا (clip->x0 >= 0 &&
	    clip->x1 <= چوڑائی &&
	    clip->y0 >= 0 &&
	    clip->y1 <= اچائی)
		أرجع true;

	أرجع false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(بنية form_control *control, صحيح شے)
{
	أكد(control != عدم);

	أرجع form__select_process_selection(control->html, control, شے);
}


/* exported interface documented in netsurf/form.h */
بنية form_option *
form_select_get_option(بنية form_control *control, صحيح شے)
{
	بنية form_option *opt;

	opt = control->انکڑے.select.اشیاء;
	طالما ((opt != عدم) && (شے > 0)) {
		opt = opt->اگلا;
		شے--;
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
	بنية form_select_menu *menu = control->انکڑے.select.menu;
	صحيح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *حالت = عدم;
	bool مضرب = control->انکڑے.select.مضرب;

	x0 = 0;
	y0 = 0;
	x1 = menu->چوڑائی;
	y1 = menu->اچائی;
	scrollbar_x = x1 - SCROLLBAR_چوڑائی;

	إذا (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		أرجع scrollbar_mouse_حالت_منزل_سنیہا(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	إذا (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		إذا (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		إذا (!(mouse & BROWSER_MOUSE_CLICK_1 && !مضرب))
			/* anything but a button 1 click over a single select
			   menu */
			حالت = messages_get(control->انکڑے.select.مضرب ?
					"SelectMClick" : "SelectClick");

	} وإلا إذا (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		حالت = messages_get("SelectClose");

	أرجع حالت;
}


/* private interface described in html/form_internal.h */
فراغ
form_select_mouse_drag_سرا(بنية form_control *control,
			   browser_mouse_state mouse,
			   صحيح x, صحيح y)
{
	صحيح x0, y0, x1, y1;
	صحيح box_x, box_y;
	بنية box *box;
	بنية form_select_menu *menu = control->انکڑے.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[بایاں_2].چوڑائی;
	box_y += box->اچائی + box->border[BOTTOM].چوڑائی +
			box->padding[BOTTOM] + box->padding[چوٹی_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	إذا (menu->scroll_capture) {
		x -= menu->چوڑائی - SCROLLBAR_چوڑائی;
		scrollbar_mouse_drag_سرا(menu->scrollbar, mouse, x, y);
		أرجع;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->چوڑائی;
	y1 = menu->اچائی;


	إذا (x > x0 && x < x1 - SCROLLBAR_چوڑائی && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
فراغ form_select_get_dimensions(بنية form_control *control,
		صحيح *چوڑائی, صحيح *اچائی)
{
	*چوڑائی = control->انکڑے.select.menu->چوڑائی;
	*اچائی = control->انکڑے.select.menu->اچائی;
}


/* private interface described in html/form_internal.h */
فراغ form_select_menu_callback(فراغ *client_انکڑے,
		صحيح x, صحيح y, صحيح چوڑائی, صحيح اچائی)
{
	html_content *html = client_انکڑے;
	صحيح menu_x, menu_y;
	بنية box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[بایاں_2].چوڑائی;
	menu_y += box->اچائی + box->border[BOTTOM].چوڑائی +
			box->padding[BOTTOM] +
			box->padding[چوٹی_2];
	content__request_redraw((بنية content *)html, menu_x + x, menu_y + y,
			چوڑائی, اچائی);
}


/* private interface described in html/form_internal.h */
فراغ form_radio_سیٹ(بنية form_control *radio)
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
	     control = control->اگلا) {
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
			dom_html_داخلہ_element_سیٹ_checked(control->گرہ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_داخلہ_element_سیٹ_checked(radio->گرہ, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *ورقہ_url,
	    بنية browser_کھڑکی *نشانہ,
	    بنية form *form,
	    بنية form_control *submit_button)
{
	nserror res;
	حرف *انکڑے = عدم; /* encoded form data */
	بنية fetch_multipart_انکڑے *success = عدم; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != عدم);

	/* obtain list of controls from DOM */
	res = form_dom_منزل_انکڑے(form, submit_button, &success);
	إذا (res != NSERROR_OK) {
		أرجع res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	إذا (res != NSERROR_OK) {
		fetch_multipart_انکڑے_destroy(success);
		أرجع res;
	}

	بدل (form->طریقہ) {
	حالة طریقہ_GET:
		res = form_url_encode(form, success, &انکڑے);
		إذا (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, انکڑے, &query_url);
			إذا (res == NSERROR_OK) {
				res = browser_کھڑکی_navigate(نشانہ,
							      query_url,
							      ورقہ_url,
							      BW_NAVIGATE_HISTORY,
							      عدم,
							      عدم,
							      عدم);

				nsurl_unref(query_url);
			}
			حرر(انکڑے);
		}
		اكسر;

	حالة طریقہ_POST_URLENC:
		res = form_url_encode(form, success, &انکڑے);
		إذا (res == NSERROR_OK) {
			res = browser_کھڑکی_navigate(نشانہ,
						      action_url,
						      ورقہ_url,
						      BW_NAVIGATE_HISTORY,
						      انکڑے,
						      عدم,
						      عدم);
			حرر(انکڑے);
		}
		اكسر;

	حالة طریقہ_POST_MULTIPART:
		res = browser_کھڑکی_navigate(نشانہ,
					      action_url,
					      ورقہ_url,
					      BW_NAVIGATE_HISTORY,
					      عدم,
					      success,
					      عدم);

		اكسر;
	}

	nsurl_unref(action_url);
	fetch_multipart_انکڑے_destroy(success);

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
		إذا (control->گرہ != عدم) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)قدر,
						لڑی_لمبائی(قدر), &str);
			إذا (err == DOM_NO_ERR) {
				إذا (control->type == GADGET_TEXTAREA)
					err = dom_html_متن_area_element_سیٹ_قدر(
						(dom_html_متن_area_element *)(control->گرہ),
						str);
				وإلا
					err = dom_html_داخلہ_element_سیٹ_قدر(
						(dom_html_داخلہ_element *)(control->گرہ),
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
	bool بدلیا_dom = false;

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
	إذا ((control->آخری_synced_قدر == عدم &&
	     control->قدر != عدم &&
	     control->قدر[0] != '\0') ||
	    (control->آخری_synced_قدر != عدم &&
	     control->قدر != عدم &&
	     strcmp(control->قدر, control->آخری_synced_قدر) != 0)) {
		حرف *dup = strdup(control->قدر);
		إذا (dup == عدم) {
			اذهب باہرلا;
		}
		إذا (control->آخری_synced_قدر != عدم) {
			حرر(control->آخری_synced_قدر);
		}
		control->آخری_synced_قدر = dup;
		exc = dom_string_create((uint8_t *)(control->قدر),
					لڑی_لمبائی(control->قدر), &قدر);
		إذا (exc != DOM_NO_ERR) {
			اذهب باہرلا;
		}
		إذا (control->گرہ_قدر != عدم) {
			dom_string_unref(control->گرہ_قدر);
		}
		control->گرہ_قدر = قدر;
		قدر = عدم;
		إذا (control->type == GADGET_TEXTAREA) {
			exc = dom_html_متن_area_element_سیٹ_قدر(control->گرہ, control->گرہ_قدر);
		} وإلا {
			exc = dom_html_داخلہ_element_سیٹ_قدر(control->گرہ, control->گرہ_قدر);
		}
		إذا (exc != DOM_NO_ERR) {
			اذهب باہرلا;
		}
		بدلیا_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	إذا (control->type == GADGET_TEXTAREA) {
		exc = dom_html_متن_area_element_get_قدر(control->گرہ, &قدر);
	} وإلا {
		exc = dom_html_داخلہ_element_get_قدر(control->گرہ, &قدر);
	}

	إذا (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		اذهب باہرلا;
	}

	إذا (!dom_string_isequal(control->گرہ_قدر, قدر)) {
		/* The DOM has changed */
		إذا (!بدلیا_dom) {
			/* And it wasn't us */
			حرف *قدر_s = strndup(
				dom_string_انکڑے(قدر),
				dom_string_byte_لمائی(قدر));
			حرف *dup = عدم;
			إذا (قدر_s == عدم) {
				اذهب باہرلا;
			}
			dup = strdup(قدر_s);
			إذا (dup == عدم) {
				حرر(قدر_s);
				اذهب باہرلا;
			}
			حرر(control->قدر);
			control->قدر = قدر_s;
			حرر(control->آخری_synced_قدر);
			control->آخری_synced_قدر = dup;
			إذا (control->type != GADGET_HIDDEN &&
			    control->انکڑے.متن.ta != عدم) {
				textarea_سیٹ_متن(control->انکڑے.متن.ta,
						  قدر_s);
			}
		}
		control->گرہ_قدر = قدر;
		قدر = عدم;
	}

باہرلا:
	إذا (قدر != عدم)
		dom_string_unref(قدر);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
بنية form *
form_new(فراغ *گرہ,
	 ثابت حرف *action,
	 ثابت حرف *نشانہ,
	 form_طریقہ طریقہ,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	بنية form *form;

	form = صفر_حافظہ_دیو(1, حجم *form);
	إذا (!form)
		أرجع عدم;

	form->action = strdup(action != عدم ? action : "");
	إذا (form->action == عدم) {
		حرر(form);
		أرجع عدم;
	}

	form->نشانہ = نشانہ != عدم ? strdup(نشانہ) : عدم;
	إذا (نشانہ != عدم && form->نشانہ == عدم) {
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->طریقہ = طریقہ;

	form->accept_charsets = charset != عدم ? strdup(charset) : عدم;
	إذا (charset != عدم && form->accept_charsets == عدم) {
		حرر(form->نشانہ);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->document_charset = doc_charset != عدم ? strdup(doc_charset)
						     : عدم;
	إذا (doc_charset && form->document_charset == عدم) {
		حرر(form->accept_charsets);
		حرر(form->نشانہ);
		حرر(form->action);
		حرر(form);
		أرجع عدم;
	}

	form->گرہ = گرہ;

	أرجع form;
}


/* exported interface documented in html/form_internal.h */
فراغ form_free(بنية form *form)
{
	بنية form_control *c, *d;

	لكل (c = form->controls; c != عدم; c = d) {
		d = c->اگلا;

		form_free_control(c);
	}

	حرر(form->action);
	حرر(form->نشانہ);
	حرر(form->accept_charsets);
	حرر(form->document_charset);

	حرر(form);
}


/* exported interface documented in html/form_internal.h */
بنية form_control *form_new_control(فراغ *گرہ, form_control_type type)
{
	بنية form_control *control;

	control = صفر_حافظہ_دیو(1, حجم *control);
	إذا (control == عدم)
		أرجع عدم;

	control->گرہ = گرہ;
	control->type = type;

	أرجع control;
}
