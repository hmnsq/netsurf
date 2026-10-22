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

#شامل <assert.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <string.h>
#شامل <dom/dom.h>

#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/talloc.h"
#شامل "utils/url.h"
#شامل "utils/utf8.h"
#شامل "utils/ascii.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/inttypes.h"
#شامل "netsurf/mouse.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/misc.h"
#شامل "content/fetch.h"
#شامل "content/hlcache.h"
#شامل "css/utils.h"
#شامل "desktop/knockout.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/layout.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"

#تعریف_کرو MAX_SELECT_اونچائی 210
#تعریف_کرو SELECT_سطر_SPACING 0.2
#تعریف_کرو SELECT_BORDER_چوڑائی 1
#تعریف_کرو SELECT_SELECTED_رنگ 0xDB9370

ساخت form_select_menu {
	صحیح سطر_اونچائی;
	صحیح چوڑائی, اونچائی;
	ساخت scrollbar *scrollbar;
	صحیح f_سائز;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	خالی *client_ڈیٹا;
	ساخت content *c;
};

ساکن plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_رنگ = SELECT_SELECTED_رنگ,
};

ساکن plot_font_style_t plot_fstyle_اندراج = {
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
ساکن حرف *
form_encode_شے(ثابت حرف *شے,
		 uint32_t len,
		 ثابت حرف *charset,
		 ثابت حرف *fallback)
{
	nserror err;
	حرف *ret = تہی;
	حرف cset[256];

	اگر (!شے || !charset)
		واپس تہی;

	snprintf(cset, حجم cset, "%s//TRANSLIT", charset);

	err = utf8_مقصد_enc(شے, cset, 0, &ret);
	اگر (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, حجم cset, "%s", charset);
		err = utf8_مقصد_enc(شے, cset, len, &ret);

		اگر (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			اگر (fallback) {
				snprintf(cset, حجم cset,
						"%s//TRANSLIT", fallback);
				err = utf8_مقصد_enc(شے, cset, 0, &ret);

				اگر (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, حجم cset,
							"%s", fallback);
					err = utf8_مقصد_enc(شے, cset, 0, &ret);
				}
			}

			اگر (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_مقصد_enc(شے, "ISO-8859-1//TRANSLIT",
						0, &ret);
				اگر (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_مقصد_enc(شے, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	اگر (err == NSERROR_NOMEM) {
		واپس تہی;
	}

	واپس ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#تعریف_کرو FETCH_ڈیٹا_INT_قدر_سائز 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ساکن nserror
fetch_ڈیٹا_list_جوڑو_sname(ثابت حرف *name,
			  ثابت حرف *ksfx,
			  صحیح قدر,
			  ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	ساخت fetch_multipart_ڈیٹا *fetch_ڈیٹا;
	صحیح keysize;

	fetch_ڈیٹا = صفر_حافظہ_دو(1, حجم(*fetch_ڈیٹا));
	اگر (fetch_ڈیٹا == تہی) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		واپس NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_ڈیٹا->name, 0, "%s%s", name, ksfx);
	fetch_ڈیٹا->name = حافظہ_دو(keysize + 1); /* allow for null */
	اگر (fetch_ڈیٹا->name == تہی) {
		آزاد(fetch_ڈیٹا);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		واپس NSERROR_NOMEM;
	}
	snprintf(fetch_ڈیٹا->name, keysize + 1, "%s%s", name, ksfx);

	/*قدر*/
	fetch_ڈیٹا->قدر = حافظہ_دو(FETCH_ڈیٹا_INT_قدر_سائز);
	اگر (fetch_ڈیٹا->قدر == تہی) {
		آزاد(fetch_ڈیٹا->name);
		آزاد(fetch_ڈیٹا);
		NSLOG(netsurf, INFO, "value allocation failure");
		واپس NSERROR_NOMEM;
	}
	snprintf(fetch_ڈیٹا->قدر, FETCH_ڈیٹا_INT_قدر_سائز, "%d", قدر);

	/* link into list */
	**fetch_ڈیٹا_اگلا_ptr = fetch_ڈیٹا;
	*fetch_ڈیٹا_اگلا_ptr = &fetch_ڈیٹا->اگلا;

	واپس NSERROR_OK;
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
ساکن nserror
fetch_ڈیٹا_list_جوڑو(dom_string *name,
		    dom_string *قدر,
		    ثابت حرف *rawfile,
		    ثابت حرف *form_charset,
		    ثابت حرف *docu_charset,
		    ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	ساخت fetch_multipart_ڈیٹا *fetch_ڈیٹا;

	أكد(name != تہی);

	fetch_ڈیٹا = صفر_حافظہ_دو(1, حجم(*fetch_ڈیٹا));
	اگر (fetch_ڈیٹا == تہی) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		واپس NSERROR_NOMEM;
	}

	fetch_ڈیٹا->name = form_encode_شے(dom_string_ڈیٹا(name),
					    dom_string_byte_لمبائی(name),
					    form_charset,
					    docu_charset);
	اگر (fetch_ڈیٹا->name == تہی) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		آزاد(fetch_ڈیٹا);
		واپس NSERROR_NOMEM;
	}

	اگر (قدر == تہی) {
		fetch_ڈیٹا->قدر = strdup("");
	} ورنہ {
		fetch_ڈیٹا->قدر = form_encode_شے(dom_string_ڈیٹا(قدر),
						     dom_string_byte_لمبائی(قدر),
						     form_charset,
						     docu_charset);
	}
	اگر (fetch_ڈیٹا->قدر == تہی) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		آزاد(fetch_ڈیٹا->name);
		آزاد(fetch_ڈیٹا);
		واپس NSERROR_NOMEM;
	}

	/* deal with raw file name */
	اگر (rawfile != تہی) {
		fetch_ڈیٹا->file = true;
		fetch_ڈیٹا->rawfile = strdup(rawfile);
		اگر (fetch_ڈیٹا->rawfile == تہی) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			آزاد(fetch_ڈیٹا->قدر);
			آزاد(fetch_ڈیٹا->name);
			آزاد(fetch_ڈیٹا);
			واپس NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_ڈیٹا_اگلا_ptr = fetch_ڈیٹا;
	*fetch_ڈیٹا_اگلا_ptr = &fetch_ڈیٹا->اگلا;

	واپس NSERROR_OK;
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
ساکن nserror
form_dom_مقصد_ڈیٹا_textarea(dom_html_متن_area_element *متن_area_element,
			  ثابت حرف *form_charset,
			  ثابت حرف *doc_charset,
			  ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	قوت_نما = dom_html_متن_area_element_get_disabled(متن_area_element,
						      &element_disabled);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		واپس NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_متن_area_element_get_name(متن_area_element,
						  &inputname);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (inputname == تہی) {
		/* allow enumeration to continue after element with no name */
		واپس NSERROR_OK;
	}

	/* obtain text area value */
	قوت_نما = dom_html_متن_area_element_get_قدر(متن_area_element,
						   &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", قوت_نما);
		dom_string_unref(inputname);
		واپس NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(inputname,
				  inputvalue,
				  تہی,
				  form_charset,
				  doc_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ثابت حرف *form_charset,
			      ثابت حرف *docu_charset,
			      ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *قدر;
	bool selected;

	قوت_نما = dom_html_option_element_get_selected(option_element, &selected);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		واپس NSERROR_DOM;
	}

	اگر (!selected) {
		/* unselected options do not add fetch data entries */
		واپس NSERROR_OK;
	}

	قوت_نما = dom_html_option_element_get_قدر(option_element, &قدر);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		واپس NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(keyname,
				  قدر,
				  تہی,
				  form_charset,
				  docu_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(قدر);

	واپس res;
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
ساکن nserror
form_dom_مقصد_ڈیٹا_select(dom_html_select_element *select_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = تہی;
	uint32_t options_گنتی;
	uint32_t option_اشاریہ;
	dom_گرہ *option_element = تہی;

	/* check if element is disabled */
	قوت_نما = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		واپس NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_select_element_get_name(select_element, &inputname);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (inputname == تہی) {
		/* allow enumeration to continue after element with no name */
		واپس NSERROR_OK;
	}

	/* get options collection */
	قوت_نما = dom_html_select_element_get_options(select_element, &options);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		واپس NSERROR_DOM;
	}

	/* get options collection length */
	قوت_نما = dom_html_options_collection_get_لمبائی(options, &options_گنتی);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		واپس NSERROR_DOM;
	}

	/* iterate over options collection */
	برائے (option_اشاریہ = 0; option_اشاریہ < options_گنتی; ++option_اشاریہ) {
		قوت_نما = dom_html_options_collection_شے(options,
						       option_اشاریہ,
						       &option_element);
		اگر (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_اشاریہ);
			res = NSERROR_DOM;
		} ورنہ {
			res = form_dom_مقصد_ڈیٹا_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_ڈیٹا_اگلا_ptr);

			dom_گرہ_unref(option_element);
		}

		اگر (res != NSERROR_OK) {
			توڑو;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ_submit(dom_html_داخلہ_element *داخلہ_element,
			      dom_string *inputname,
			      ثابت حرف *charset,
			      ثابت حرف *document_charset,
			      dom_html_element **submit_button,
			      ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	اگر (*submit_button == تہی) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)داخلہ_element;
	} ورنہ اگر (*submit_button != (dom_html_element *)داخلہ_element) {
		واپس NSERROR_OK;
	}

	/* matched button used to submit form */
	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		واپس NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(inputname,
				  inputvalue,
				  تہی,
				  charset,
				  document_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(inputvalue);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ_image(dom_html_داخلہ_element *داخلہ_element,
			     dom_string *inputname,
			     ثابت حرف *charset,
			     ثابت حرف *document_charset,
			     dom_html_element **submit_button,
			     ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	ساخت image_داخلہ_coords *coords;
	حرف *basename;

	/* Only use an image input if it was the thing which activated us */
	اگر (*submit_button != (dom_html_element *)داخلہ_element) {
		واپس NSERROR_OK;
	}

	قوت_نما = dom_گرہ_get_user_ڈیٹا((dom_گرہ *)داخلہ_element,
				     corestring_dom_ns_کلید_image_coords_گرہ_ڈیٹا,
				     &coords);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		واپس NSERROR_DOM;
	}

	اگر (coords == تہی) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		واپس NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_شے(dom_string_ڈیٹا(inputname),
				    dom_string_byte_لمبائی(inputname),
				    charset,
				    document_charset);
	اگر (basename == تہی) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		واپس NSERROR_NOMEM;
	}

	res = fetch_ڈیٹا_list_جوڑو_sname(basename, ".x",
					coords->x,
					fetch_ڈیٹا_اگلا_ptr);

	اگر (res == NSERROR_OK) {
		res = fetch_ڈیٹا_list_جوڑو_sname(basename, ".y",
						coords->y,
						fetch_ڈیٹا_اگلا_ptr);
	}

	آزاد(basename);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ_checkbox(dom_html_داخلہ_element *داخلہ_element,
				dom_string *inputname,
				ثابت حرف *charset,
				ثابت حرف *document_charset,
				ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلہ_element_get_checked(داخلہ_element, &checked);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		واپس NSERROR_DOM;
	}

	اگر (!checked) {
		/* unchecked items do not generate a data entry */
		واپس NSERROR_OK;
	}

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		واپس NSERROR_DOM;
	}

	/* ensure a default value */
	اگر (inputvalue == تہی) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(inputname,
				  inputvalue,
				  تہی,
				  charset,
				  document_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(inputvalue);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ_file(dom_html_داخلہ_element *داخلہ_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;
	ثابت حرف *rawfile = تہی;

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		واپس NSERROR_DOM;
	}

	قوت_نما = dom_گرہ_get_user_ڈیٹا((dom_گرہ *)داخلہ_element,
				     corestring_dom_ns_کلید_file_name_گرہ_ڈیٹا,
				     &rawfile);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		واپس NSERROR_DOM;
	}

	اگر (rawfile == تہی) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(inputvalue);

	واپس res;
}


ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ_متن(dom_html_داخلہ_element *داخلہ_element,
			    dom_string *inputname,
			    ثابت حرف *charset,
			    ثابت حرف *document_charset,
			    ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	nserror res;
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_string *inputvalue;

	قوت_نما = dom_html_داخلہ_element_get_قدر(داخلہ_element, &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		واپس NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ڈیٹا_list_جوڑو(inputname,
				  inputvalue,
				  تہی,
				  charset,
				  document_charset,
				  fetch_ڈیٹا_اگلا_ptr);

	dom_string_unref(inputvalue);

	واپس res;
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
ساکن nserror
form_dom_مقصد_ڈیٹا_داخلہ(dom_html_داخلہ_element *داخلہ_element,
		       ثابت حرف *charset,
		       ثابت حرف *document_charset,
		       dom_html_element **submit_button,
		       ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
{
	dom_exception قوت_نما; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	قوت_نما = dom_html_داخلہ_element_get_disabled(داخلہ_element,
						  &element_disabled);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* disabled element requires no more processing */
		واپس NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_داخلہ_element_get_name(داخلہ_element, &inputname);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (inputname == تہی) {
		/* element with no name is not converted */
		واپس NSERROR_OK;
	}

	/* get input type */
	قوت_نما = dom_html_داخلہ_element_get_type(داخلہ_element, &inputtype);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		واپس NSERROR_DOM;
	}

	/* process according to input element type */
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_مقصد_ڈیٹا_داخلہ_submit(داخلہ_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_ڈیٹا_اگلا_ptr);

	} ورنہ اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_مقصد_ڈیٹا_داخلہ_image(داخلہ_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_ڈیٹا_اگلا_ptr);

	} ورنہ اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_مقصد_ڈیٹا_داخلہ_checkbox(داخلہ_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_ڈیٹا_اگلا_ptr);

	} ورنہ اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_مقصد_ڈیٹا_داخلہ_file(داخلہ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ڈیٹا_اگلا_ptr);

	} ورنہ اگر (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ورنہ {
		/* Everything else is treated as text values */
		res = form_dom_مقصد_ڈیٹا_داخلہ_متن(داخلہ_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ڈیٹا_اگلا_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	واپس res;
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
ساکن nserror
form_dom_مقصد_ڈیٹا_button(dom_html_button_element *button_element,
			ثابت حرف *form_charset,
			ثابت حرف *doc_charset,
			dom_html_element **submit_button,
			ساخت fetch_multipart_ڈیٹا ***fetch_ڈیٹا_اگلا_ptr)
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
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (element_disabled) {
		/* allow enumeration to continue after disabled element */
		واپس NSERROR_OK;
	}

	/* get the type attribute */
	قوت_نما = dom_html_button_element_get_type(button_element, &inputtype);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		واپس NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		واپس NSERROR_OK;
	}
	اگر (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		واپس NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	اگر (*submit_button == تہی) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	اگر (*submit_button != (dom_html_element *)button_element) {
		واپس NSERROR_OK;
	}

	/* obtain name property */
	قوت_نما = dom_html_button_element_get_name(button_element, &inputname);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", قوت_نما);
		واپس NSERROR_DOM;
	}

	اگر (inputname == تہی) {
		/* allow enumeration to continue after element with no name */
		واپس NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	قوت_نما = dom_html_button_element_get_قدر(button_element, &inputvalue);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ورنہ {
		res = fetch_ڈیٹا_list_جوڑو(inputname,
					  inputvalue,
					  تہی,
					  form_charset,
					  doc_charset,
					  fetch_ڈیٹا_اگلا_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	واپس res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ساکن حرف *form_acceptable_charset(ساخت form *form)
{
	حرف *عارضی, *c;

	اگر (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		اگر (form->document_charset) {
			/* document charset present, so use it */
			واپس strdup(form->document_charset);
		} ورنہ {
			/* no document charset, so default to 8859-1 */
			واپس strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	عارضی = strdup(form->accept_charsets);
	اگر (!عارضی)
		واپس تہی;

	/* make it upper case */
	برائے (c = عارضی; *c; c++) {
		*c = ascii_مقصد_بالائی(*c);
	}

	/* is UTF-8 specified? */
	c = لڑی_تلاش(عارضی, "UTF-8");
	اگر (c) {
		آزاد(عارضی);
		واپس strdup("UTF-8");
	}

	/* dispense with temporary copy */
	آزاد(عارضی);

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
	جب (*c && !ascii_is_space(*c)) {
		اگر (*c == ',')
			توڑو;
		c++;
	}

	واپس strndup(form->accept_charsets, c - form->accept_charsets);
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
ساکن nserror
form_dom_مقصد_ڈیٹا(ساخت form *form,
		 ساخت form_control *submit_control,
		 ساخت fetch_multipart_ڈیٹا **fetch_ڈیٹا_برآمد)
{
	nserror res = NSERROR_OK;
	حرف *charset; /* form characterset */
	dom_exception قوت_نما; /* the result from DOM operations */
	dom_html_collection *elements = تہی; /* the dom form elements */
	uint32_t element_گنتی; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_گرہ *element = تہی; /* the DOM form element */
	dom_string *nodename = تہی; /* the DOM node name of the element */
	ساخت fetch_multipart_ڈیٹا *fetch_ڈیٹا = تہی; /* fetch data list */
	ساخت fetch_multipart_ڈیٹا **fetch_ڈیٹا_اگلا = &fetch_ڈیٹا;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	اگر (submit_control != تہی) {
		submit_button = submit_control->گرہ;
	} ورنہ {
		submit_button = تہی;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	اگر (charset == تہی) {
		NSLOG(netsurf, INFO, "failed to find charset");
		واپس NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	قوت_نما = dom_html_form_element_get_elements(form->گرہ, &elements);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		آزاد(charset);
		واپس NSERROR_DOM;
	}

	قوت_نما = dom_html_collection_get_لمبائی(elements, &element_گنتی);
	اگر (قوت_نما != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		جاؤ form_dom_مقصد_ڈیٹا_error;
	}

	برائے (element_idx = 0; element_idx < element_گنتی; element_idx++) {
		/* obtain a form element */
		قوت_نما = dom_html_collection_شے(elements, element_idx, &element);
		اگر (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			res = NSERROR_DOM;
			جاؤ form_dom_مقصد_ڈیٹا_error;
		}

		/* node name from element */
		قوت_نما = dom_گرہ_get_گرہ_name(element, &nodename);
		اگر (قوت_نما != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, قوت_نما);
			dom_گرہ_unref(element);
			res = NSERROR_DOM;
			جاؤ form_dom_مقصد_ڈیٹا_error;
		}

		اگر (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_مقصد_ڈیٹا_textarea(
				(dom_html_متن_area_element *)element,
				charset,
				form->document_charset,
				&fetch_ڈیٹا_اگلا);

		} ورنہ اگر (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_مقصد_ڈیٹا_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_ڈیٹا_اگلا);

		} ورنہ اگر (dom_string_isequal(nodename, corestring_dom_داخلہ)) {
			/* Form element is HTMLInputElement */
			res = form_dom_مقصد_ڈیٹا_داخلہ(
				(dom_html_داخلہ_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ڈیٹا_اگلا);

		} ورنہ اگر (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_مقصد_ڈیٹا_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ڈیٹا_اگلا);

		} ورنہ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (صحیح)dom_string_byte_لمبائی(nodename),
			      dom_string_ڈیٹا(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_گرہ_unref(element);

		/* abort form element enumeration on error */
		اگر (res != NSERROR_OK) {
			جاؤ form_dom_مقصد_ڈیٹا_error;
		}
	}

	*fetch_ڈیٹا_برآمد = fetch_ڈیٹا;
	dom_html_collection_unref(elements);
	آزاد(charset);

	واپس NSERROR_OK;

form_dom_مقصد_ڈیٹا_error:
	fetch_multipart_ڈیٹا_destroy(fetch_ڈیٹا);
	dom_html_collection_unref(elements);
	آزاد(charset);

	واپس res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ساکن nserror
form_url_encode(ساخت form *form,
		ساخت fetch_multipart_ڈیٹا *control,
		حرف **encoded_برآمد)
{
	حرف *name, *قدر;
	حرف *s, *s2;
	غیرعلامتی صحیح len, len1, len_init;
	nserror res;

	s = حافظہ_دو(1);

	اگر (s == تہی) {
		واپس NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	برائے (; control; control = control->اگلا) {
		res = url_escape(control->name, true, تہی, &name);
		اگر (res != NSERROR_OK) {
			آزاد(s);
			واپس res;
		}

		res = url_escape(control->قدر, true, تہی, &قدر);
		اگر (res != NSERROR_OK) {
			آزاد(name);
			آزاد(s);
			واپس res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + لڑی_لمبائی(name) + لڑی_لمبائی(قدر) + 2;
		s2 = realloc(s, len1 + 1);
		اگر (s2 == تہی) {
			آزاد(قدر);
			آزاد(name);
			آزاد(s);
			واپس NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, قدر);
		len = len1;
		آزاد(name);
		آزاد(قدر);
	}

	اگر (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_برآمد = s;

	واپس NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ساکن خالی
form_select_menu_scroll_callback(خالی *client_ڈیٹا,
				 ساخت scrollbar_پیغام_ڈیٹا *scrollbar_ڈیٹا)
{
	ساخت form_control *control = client_ڈیٹا;
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;
	html_content *html = (html_content *)menu->c;

	انتخاب (scrollbar_ڈیٹا->پیغام) {
		حالت SCROLLBAR_پیغام_MOVED:
			menu->callback(menu->client_ڈیٹا,
					0, 0,
					menu->چوڑائی,
					menu->اونچائی);
			توڑو;
		حالت SCROLLBAR_پیغام_SCROLL_آغاز:
		{
			ساخت rect rect = {
				.x0 = scrollbar_ڈیٹا->x0,
				.y0 = scrollbar_ڈیٹا->y0,
				.x1 = scrollbar_ڈیٹا->x1,
				.y1 = scrollbar_ڈیٹا->y1
			};

			browser_کھڑکی_سیٹ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			توڑو;
		حالت SCROLLBAR_پیغام_SCROLL_ختم_شدہ:
			menu->scroll_capture = false;

			browser_کھڑکی_سیٹ_drag_type(html->bw,
					DRAGGING_NONE, تہی);
			توڑو;
		طےشدہ:
			توڑو;
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
ساکن nserror
form__select_process_selection(html_content *html,
			       ساخت form_control *control,
			       صحیح شے)
{
	ساخت box *inline_box;
	ساخت form_option *o;
	صحیح گنتی;
	nserror ret = NSERROR_OK;

	أكد(control != تہی);
	أكد(html != تہی);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	برائے (گنتی = 0, o = control->ڈیٹا.select.اشیاء;
			o != تہی;
			گنتی++, o = o->اگلا) {
		اگر (!control->ڈیٹا.select.مضرب && o->selected) {
			o->selected = false;
			dom_html_option_element_سیٹ_selected(o->گرہ, false);
		}

		اگر (گنتی == شے) {
			اگر (control->ڈیٹا.select.مضرب) {
				اگر (o->selected) {
					o->selected = false;
					dom_html_option_element_سیٹ_selected(
							o->گرہ, false);
					control->ڈیٹا.select.عدد_selected--;
				} ورنہ {
					o->selected = true;
					dom_html_option_element_سیٹ_selected(
							o->گرہ, true);
					control->ڈیٹا.select.عدد_selected++;
				}
			} ورنہ {
				dom_html_option_element_سیٹ_selected(
						o->گرہ, true);
				o->selected = true;
			}
		}

		اگر (o->selected) {
			control->ڈیٹا.select.current = o;
		}
	}

	talloc_free(inline_box->متن);
	inline_box->متن = 0;

	اگر (control->ڈیٹا.select.عدد_selected == 0) {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ورنہ اگر (control->ڈیٹا.select.عدد_selected == 1) {
		inline_box->متن = talloc_strdup(html->bctx,
				control->ڈیٹا.select.current->متن);
	} ورنہ {
		inline_box->متن = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	اگر (!inline_box->متن) {
		ret = NSERROR_NOMEM;
		inline_box->لمبائی = 0;
	} ورنہ {
		inline_box->لمبائی = لڑی_لمبائی(inline_box->متن);
	}
	inline_box->چوڑائی = control->box->چوڑائی;

	html__redraw_a_box(html, control->box);

	واپس ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ساکن خالی form_select_menu_clicked(ساخت form_control *control, صحیح x, صحیح y)
{
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;
	ساخت form_option *option;
	html_content *html = (html_content *)menu->c;
	صحیح سطر_اونچائی, سطر_اونچائی_with_spacing;
	صحیح شے_bottom_y;
	صحیح scroll, i;

	scroll = scrollbar_get_انحراف(menu->scrollbar);

	سطر_اونچائی = menu->سطر_اونچائی;
	سطر_اونچائی_with_spacing = سطر_اونچائی +
			سطر_اونچائی * SELECT_سطر_SPACING;

	option = control->ڈیٹا.select.اشیاء;
	شے_bottom_y = سطر_اونچائی_with_spacing;
	i = 0;
	جب (option && شے_bottom_y < scroll + y) {
		شے_bottom_y += سطر_اونچائی_with_spacing;
		option = option->اگلا;
		i++;
	}

	اگر (option != تہی) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_ڈیٹا, 0, 0, menu->چوڑائی, menu->اونچائی);
}


/* exported interface documented in html/form_internal.h */
خالی form_جوڑو_control(ساخت form *form, ساخت form_control *control)
{
	اگر (form == تہی) {
		واپس;
	}

	control->form = form;

	اگر (form->controls != تہی) {
		أكد(form->آخری_control);

		form->آخری_control->اگلا = control;
		control->پچھلا = form->آخری_control;
		control->اگلا = تہی;
		form->آخری_control = control;
	} ورنہ {
		form->controls = form->آخری_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
خالی form_free_control(ساخت form_control *control)
{
	ساخت form_control *c;
	أكد(control != تہی);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->قدر, control->initial_قدر);
	آزاد(control->name);
	آزاد(control->قدر);
	آزاد(control->initial_قدر);
	اگر (control->آخری_synced_قدر != تہی) {
		آزاد(control->آخری_synced_قدر);
	}

	اگر (control->type == GADGET_SELECT) {
		ساخت form_option *option, *اگلا;

		برائے (option = control->ڈیٹا.select.اشیاء; option;
				option = اگلا) {
			اگلا = option->اگلا;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->متن, option->قدر);
			آزاد(option->متن);
			آزاد(option->قدر);
			آزاد(option);
		}
		اگر (control->ڈیٹا.select.menu != تہی) {
			form_free_select_menu(control);
		}
	}

	اگر (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		اگر (control->ڈیٹا.متن.initial != تہی) {
			dom_string_unref(control->ڈیٹا.متن.initial);
		}

		اگر (control->ڈیٹا.متن.ta != تہی) {
			textarea_destroy(control->ڈیٹا.متن.ta);
		}
	}

	/* unlink the control from the form */
	اگر (control->form != تہی) {
		برائے (c = control->form->controls; c != تہی; c = c->اگلا) {
			اگر (c->اگلا == control) {
				c->اگلا = control->اگلا;
				اگر (control->form->آخری_control == control)
					control->form->آخری_control = c;
				توڑو;
			}
			اگر (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->اگلا;
				اگر (control->form->آخری_control == control)
					control->form->controls =
						control->form->آخری_control = تہی;
				توڑو;
			}
		}
	}

	اگر (control->گرہ_قدر != تہی) {
		dom_string_unref(control->گرہ_قدر);
	}

	آزاد(control);
}


/* exported interface documented in html/form_internal.h */
bool form_جوڑو_option(ساخت form_control *control, حرف *قدر, حرف *متن,
		     bool selected, خالی *گرہ)
{
	ساخت form_option *option;

	أكد(control);
	أكد(control->type == GADGET_SELECT);

	option = صفر_حافظہ_دو(1, حجم *option);
	اگر (!option)
		واپس false;

	option->قدر = قدر;
	option->متن = متن;

	/* add to linked list */
	اگر (control->ڈیٹا.select.اشیاء == 0)
		control->ڈیٹا.select.اشیاء = option;
	ورنہ
		control->ڈیٹا.select.آخری_شے->اگلا = option;
	control->ڈیٹا.select.آخری_شے = option;

	/* set selected */
	اگر (selected && (control->ڈیٹا.select.عدد_selected == 0 ||
			control->ڈیٹا.select.مضرب)) {
		option->selected = option->initial_selected = true;
		control->ڈیٹا.select.عدد_selected++;
		control->ڈیٹا.select.current = option;
	}

	control->ڈیٹا.select.عدد_اشیاء++;

	option->گرہ = گرہ;

	واپس true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(خالی *client_ڈیٹا,
		      ساخت form_control *control,
		      select_menu_redraw_callback callback,
		      ساخت content *c)
{
	صحیح سطر_اونچائی_with_spacing;
	ساخت box *box;
	plot_font_style_t fstyle;
	صحیح کل_اونچائی;
	ساخت form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	اگر (control->ڈیٹا.select.menu == تہی) {

		menu = صفر_حافظہ_دو(1, حجم (ساخت form_select_menu));
		اگر (menu == تہی) {
			واپس NSERROR_NOMEM;
		}

		control->ڈیٹا.select.menu = menu;

		box = control->box;

		menu->چوڑائی = box->چوڑائی +
			box->border[دایاں_2].چوڑائی + box->padding[دایاں_2] +
			box->border[بایاں_2].چوڑائی + box->padding[بایاں_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_سائز = fstyle.سائز;

		menu->سطر_اونچائی = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.سائز / PLOT_STYLE_SCALE)))),
				F_72));

		سطر_اونچائی_with_spacing = menu->سطر_اونچائی +
				menu->سطر_اونچائی *
				SELECT_سطر_SPACING;

		کل_اونچائی = control->ڈیٹا.select.عدد_اشیاء *
				سطر_اونچائی_with_spacing;
		menu->اونچائی = کل_اونچائی;

		اگر (menu->اونچائی > MAX_SELECT_اونچائی) {
			menu->اونچائی = MAX_SELECT_اونچائی;
		}

		menu->client_ڈیٹا = client_ڈیٹا;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->اونچائی,
				       کل_اونچائی,
				       menu->اونچائی,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		اگر (res != NSERROR_OK) {
			control->ڈیٹا.select.menu = تہی;
			آزاد(menu);
			واپس res;
		}
		menu->c = c;
	} ورنہ {
		menu = control->ڈیٹا.select.menu;
	}

	menu->callback(client_ڈیٹا, 0, 0, menu->چوڑائی, menu->اونچائی);

	واپس NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
خالی form_free_select_menu(ساخت form_control *control)
{
	اگر (control->ڈیٹا.select.menu->scrollbar != تہی)
		scrollbar_destroy(control->ڈیٹا.select.menu->scrollbar);
	آزاد(control->ڈیٹا.select.menu);
	control->ڈیٹا.select.menu = تہی;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ساخت form_control *control,
			صحیح x, صحیح y,
			اعشاری scale,
			ثابت ساخت rect *clip,
			ثابت ساخت redraw_context *ctx)
{
	ساخت box *box;
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;
	ساخت form_option *option;
	صحیح سطر_اونچائی, سطر_اونچائی_with_spacing;
	صحیح چوڑائی, اونچائی;
	صحیح x0, y0, x1, scrollbar_x, y1, y2, y3;
	صحیح شے_y;
	صحیح متن_pos_انحراف, متن_x;
	صحیح scrollbar_چوڑائی = SCROLLBAR_چوڑائی;
	صحیح i;
	صحیح scroll;
	صحیح x_cp, y_cp;
	ساخت rect r;
	ساخت rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	چوڑائی = menu->چوڑائی;
	اونچائی = menu->اونچائی;
	سطر_اونچائی = menu->سطر_اونچائی;

	سطر_اونچائی_with_spacing = سطر_اونچائی +
			سطر_اونچائی * SELECT_سطر_SPACING;
	scroll = scrollbar_get_انحراف(menu->scrollbar);

	اگر (scale != 1.0) {
		x *= scale;
		y *= scale;
		چوڑائی *= scale;
		اونچائی *= scale;
		scrollbar_چوڑائی *= scale;

		i = scroll / سطر_اونچائی_with_spacing;
		scroll -= i * سطر_اونچائی_with_spacing;
		سطر_اونچائی *= scale;
		سطر_اونچائی_with_spacing *= scale;
		scroll *= scale;
		scroll += i * سطر_اونچائی_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + چوڑائی - 1;
	y1 = y + اونچائی - 1;
	scrollbar_x = x1 - scrollbar_چوڑائی;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	x0 = x0 + SELECT_BORDER_چوڑائی;
	y0 = y0 + SELECT_BORDER_چوڑائی;
	x1 = x1 - SELECT_BORDER_چوڑائی;
	y1 = y1 - SELECT_BORDER_چوڑائی;
	اونچائی = اونچائی - 2 * SELECT_BORDER_چوڑائی;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	option = control->ڈیٹا.select.اشیاء;
	شے_y = سطر_اونچائی_with_spacing;

	جب (شے_y < scroll) {
		option = option->اگلا;
		شے_y += سطر_اونچائی_with_spacing;
	}
	شے_y -= سطر_اونچائی_with_spacing;
	متن_pos_انحراف = y - scroll +
			(صحیح) (سطر_اونچائی * (0.75 + SELECT_سطر_SPACING));
	متن_x = x + (box->border[بایاں_2].چوڑائی + box->padding[بایاں_2]) * scale;

	plot_fstyle_اندراج.سائز = menu->f_سائز;

	جب (option && شے_y - scroll < اونچائی) {

		اگر (option->selected) {
			y2 = y + شے_y - scroll;
			y3 = y + شے_y + سطر_اونچائی_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
		}

		y2 = متن_pos_انحراف + شے_y;
		res = ctx->plot->متن(ctx,
				      &plot_fstyle_اندراج,
				      متن_x, y2,
				      option->متن, لڑی_لمبائی(option->متن));
		اگر (res != NSERROR_OK) {
			واپس false;
		}

		شے_y += سطر_اونچائی_with_spacing;
		option = option->اگلا;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->چوڑائی - SCROLLBAR_چوڑائی,
			       y_cp,
			       clip, scale, ctx);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	واپس true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ساخت form_control *control,
			     اعشاری scale,
			     ثابت ساخت rect *clip)
{
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;
	صحیح چوڑائی, اونچائی;


	چوڑائی = menu->چوڑائی;
	اونچائی = menu->اونچائی;

	اگر (scale != 1.0) {
		چوڑائی *= scale;
		اونچائی *= scale;
	}

	اگر (clip->x0 >= 0 &&
	    clip->x1 <= چوڑائی &&
	    clip->y0 >= 0 &&
	    clip->y1 <= اونچائی)
		واپس true;

	واپس false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ساخت form_control *control, صحیح شے)
{
	أكد(control != تہی);

	واپس form__select_process_selection(control->html, control, شے);
}


/* exported interface documented in netsurf/form.h */
ساخت form_option *
form_select_get_option(ساخت form_control *control, صحیح شے)
{
	ساخت form_option *opt;

	opt = control->ڈیٹا.select.اشیاء;
	جب ((opt != تہی) && (شے > 0)) {
		opt = opt->اگلا;
		شے--;
	}
	واپس opt;
}


/* exported interface documented in netsurf/form.h */
حرف *form_control_get_name(ساخت form_control *control)
{
	واپس control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ساخت form_control *control, ساخت rect *r)
{
	box_bounds( control->box, r );
	واپس NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ثابت حرف *
form_select_mouse_action(ساخت form_control *control,
			 browser_mouse_state mouse,
			 صحیح x, صحیح y)
{
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;
	صحیح x0, y0, x1, y1, scrollbar_x;
	ثابت حرف *حالت_2 = تہی;
	bool مضرب = control->ڈیٹا.select.مضرب;

	x0 = 0;
	y0 = 0;
	x1 = menu->چوڑائی;
	y1 = menu->اونچائی;
	scrollbar_x = x1 - SCROLLBAR_چوڑائی;

	اگر (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		واپس scrollbar_mouse_حالت_مقصد_پیغام(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	اگر (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		اگر (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		اگر (!(mouse & BROWSER_MOUSE_CLICK_1 && !مضرب))
			/* anything but a button 1 click over a single select
			   menu */
			حالت_2 = messages_get(control->ڈیٹا.select.مضرب ?
					"SelectMClick" : "SelectClick");

	} ورنہ اگر (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		حالت_2 = messages_get("SelectClose");

	واپس حالت_2;
}


/* private interface described in html/form_internal.h */
خالی
form_select_mouse_drag_انتہا(ساخت form_control *control,
			   browser_mouse_state mouse,
			   صحیح x, صحیح y)
{
	صحیح x0, y0, x1, y1;
	صحیح box_x, box_y;
	ساخت box *box;
	ساخت form_select_menu *menu = control->ڈیٹا.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[بایاں_2].چوڑائی;
	box_y += box->اونچائی + box->border[BOTTOM].چوڑائی +
			box->padding[BOTTOM] + box->padding[چوٹی_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	اگر (menu->scroll_capture) {
		x -= menu->چوڑائی - SCROLLBAR_چوڑائی;
		scrollbar_mouse_drag_انتہا(menu->scrollbar, mouse, x, y);
		واپس;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->چوڑائی;
	y1 = menu->اونچائی;


	اگر (x > x0 && x < x1 - SCROLLBAR_چوڑائی && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
خالی form_select_get_dimensions(ساخت form_control *control,
		صحیح *چوڑائی, صحیح *اونچائی)
{
	*چوڑائی = control->ڈیٹا.select.menu->چوڑائی;
	*اونچائی = control->ڈیٹا.select.menu->اونچائی;
}


/* private interface described in html/form_internal.h */
خالی form_select_menu_callback(خالی *client_ڈیٹا,
		صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی)
{
	html_content *html = client_ڈیٹا;
	صحیح menu_x, menu_y;
	ساخت box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[بایاں_2].چوڑائی;
	menu_y += box->اونچائی + box->border[BOTTOM].چوڑائی +
			box->padding[BOTTOM] +
			box->padding[چوٹی_2];
	content__request_redraw((ساخت content *)html, menu_x + x, menu_y + y,
			چوڑائی, اونچائی);
}


/* private interface described in html/form_internal.h */
خالی form_radio_سیٹ(ساخت form_control *radio)
{
	ساخت form_control *control;

	أكد(radio);
	اگر (!radio->form)
		واپس;

	اگر (radio->selected)
		واپس;

	/* Clear selected state for other controls in
	 * the same radio button group */
	برائے (control = radio->form->controls;
	     control != تہی;
	     control = control->اگلا) {
		/* Only interested in radio inputs */
		اگر (control->type != GADGET_RADIO)
			جاری;

		/* Ignore ourself */
		اگر (control == radio)
			جاری;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		اگر ((control->name == تہی) ||
		    (radio->name == تہی) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			جاری;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		اگر (control->selected) {
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
form_submit(nsurl *صفحہ_url,
	    ساخت browser_کھڑکی *ہدف,
	    ساخت form *form,
	    ساخت form_control *submit_button)
{
	nserror res;
	حرف *ڈیٹا = تہی; /* encoded form data */
	ساخت fetch_multipart_ڈیٹا *success = تہی; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	أكد(form != تہی);

	/* obtain list of controls from DOM */
	res = form_dom_مقصد_ڈیٹا(form, submit_button, &success);
	اگر (res != NSERROR_OK) {
		واپس res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	اگر (res != NSERROR_OK) {
		fetch_multipart_ڈیٹا_destroy(success);
		واپس res;
	}

	انتخاب (form->طریقہ) {
	حالت طریقہ_GET:
		res = form_url_encode(form, success, &ڈیٹا);
		اگر (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, ڈیٹا, &query_url);
			اگر (res == NSERROR_OK) {
				res = browser_کھڑکی_navigate(ہدف,
							      query_url,
							      صفحہ_url,
							      BW_NAVIGATE_HISTORY,
							      تہی,
							      تہی,
							      تہی);

				nsurl_unref(query_url);
			}
			آزاد(ڈیٹا);
		}
		توڑو;

	حالت طریقہ_POST_URLENC:
		res = form_url_encode(form, success, &ڈیٹا);
		اگر (res == NSERROR_OK) {
			res = browser_کھڑکی_navigate(ہدف,
						      action_url,
						      صفحہ_url,
						      BW_NAVIGATE_HISTORY,
						      ڈیٹا,
						      تہی,
						      تہی);
			آزاد(ڈیٹا);
		}
		توڑو;

	حالت طریقہ_POST_MULTIPART:
		res = browser_کھڑکی_navigate(ہدف,
					      action_url,
					      صفحہ_url,
					      BW_NAVIGATE_HISTORY,
					      تہی,
					      success,
					      تہی);

		توڑو;
	}

	nsurl_unref(action_url);
	fetch_multipart_ڈیٹا_destroy(success);

	واپس res;
}


/* exported interface documented in html/form_internal.h */
خالی form_gadget_update_قدر(ساخت form_control *control, حرف *قدر)
{
	انتخاب (control->type) {
	حالت GADGET_HIDDEN:
	حالت GADGET_TEXTBOX:
	حالت GADGET_TEXTAREA:
	حالت GADGET_PASSWORD:
	حالت GADGET_FILE:
		اگر (control->قدر != تہی) {
			آزاد(control->قدر);
		}
		control->قدر = قدر;
		اگر (control->گرہ != تہی) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)قدر,
						لڑی_لمبائی(قدر), &str);
			اگر (err == DOM_NO_ERR) {
				اگر (control->type == GADGET_TEXTAREA)
					err = dom_html_متن_area_element_سیٹ_قدر(
						(dom_html_متن_area_element *)(control->گرہ),
						str);
				ورنہ
					err = dom_html_داخلہ_element_سیٹ_قدر(
						(dom_html_داخلہ_element *)(control->گرہ),
						str);
				dom_string_unref(str);
			}
		}
		توڑو;
	طےشدہ:
		/* Do nothing */
		توڑو;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
خالی
form_gadget_sync_with_dom(ساخت form_control *control)
{
	dom_exception exc;
	dom_string *قدر = تہی;
	bool بدلا_dom = false;

	اگر (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		واپس;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	اگر ((control->آخری_synced_قدر == تہی &&
	     control->قدر != تہی &&
	     control->قدر[0] != '\0') ||
	    (control->آخری_synced_قدر != تہی &&
	     control->قدر != تہی &&
	     strcmp(control->قدر, control->آخری_synced_قدر) != 0)) {
		حرف *dup = strdup(control->قدر);
		اگر (dup == تہی) {
			جاؤ برآمد;
		}
		اگر (control->آخری_synced_قدر != تہی) {
			آزاد(control->آخری_synced_قدر);
		}
		control->آخری_synced_قدر = dup;
		exc = dom_string_create((uint8_t *)(control->قدر),
					لڑی_لمبائی(control->قدر), &قدر);
		اگر (exc != DOM_NO_ERR) {
			جاؤ برآمد;
		}
		اگر (control->گرہ_قدر != تہی) {
			dom_string_unref(control->گرہ_قدر);
		}
		control->گرہ_قدر = قدر;
		قدر = تہی;
		اگر (control->type == GADGET_TEXTAREA) {
			exc = dom_html_متن_area_element_سیٹ_قدر(control->گرہ, control->گرہ_قدر);
		} ورنہ {
			exc = dom_html_داخلہ_element_سیٹ_قدر(control->گرہ, control->گرہ_قدر);
		}
		اگر (exc != DOM_NO_ERR) {
			جاؤ برآمد;
		}
		بدلا_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	اگر (control->type == GADGET_TEXTAREA) {
		exc = dom_html_متن_area_element_get_قدر(control->گرہ, &قدر);
	} ورنہ {
		exc = dom_html_داخلہ_element_get_قدر(control->گرہ, &قدر);
	}

	اگر (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		جاؤ برآمد;
	}

	اگر (!dom_string_isequal(control->گرہ_قدر, قدر)) {
		/* The DOM has changed */
		اگر (!بدلا_dom) {
			/* And it wasn't us */
			حرف *قدر_s = strndup(
				dom_string_ڈیٹا(قدر),
				dom_string_byte_لمبائی(قدر));
			حرف *dup = تہی;
			اگر (قدر_s == تہی) {
				جاؤ برآمد;
			}
			dup = strdup(قدر_s);
			اگر (dup == تہی) {
				آزاد(قدر_s);
				جاؤ برآمد;
			}
			آزاد(control->قدر);
			control->قدر = قدر_s;
			آزاد(control->آخری_synced_قدر);
			control->آخری_synced_قدر = dup;
			اگر (control->type != GADGET_HIDDEN &&
			    control->ڈیٹا.متن.ta != تہی) {
				textarea_سیٹ_متن(control->ڈیٹا.متن.ta,
						  قدر_s);
			}
		}
		control->گرہ_قدر = قدر;
		قدر = تہی;
	}

برآمد:
	اگر (قدر != تہی)
		dom_string_unref(قدر);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ساخت form *
form_new(خالی *گرہ,
	 ثابت حرف *action,
	 ثابت حرف *ہدف,
	 form_طریقہ طریقہ,
	 ثابت حرف *charset,
	 ثابت حرف *doc_charset)
{
	ساخت form *form;

	form = صفر_حافظہ_دو(1, حجم *form);
	اگر (!form)
		واپس تہی;

	form->action = strdup(action != تہی ? action : "");
	اگر (form->action == تہی) {
		آزاد(form);
		واپس تہی;
	}

	form->ہدف = ہدف != تہی ? strdup(ہدف) : تہی;
	اگر (ہدف != تہی && form->ہدف == تہی) {
		آزاد(form->action);
		آزاد(form);
		واپس تہی;
	}

	form->طریقہ = طریقہ;

	form->accept_charsets = charset != تہی ? strdup(charset) : تہی;
	اگر (charset != تہی && form->accept_charsets == تہی) {
		آزاد(form->ہدف);
		آزاد(form->action);
		آزاد(form);
		واپس تہی;
	}

	form->document_charset = doc_charset != تہی ? strdup(doc_charset)
						     : تہی;
	اگر (doc_charset && form->document_charset == تہی) {
		آزاد(form->accept_charsets);
		آزاد(form->ہدف);
		آزاد(form->action);
		آزاد(form);
		واپس تہی;
	}

	form->گرہ = گرہ;

	واپس form;
}


/* exported interface documented in html/form_internal.h */
خالی form_free(ساخت form *form)
{
	ساخت form_control *c, *d;

	برائے (c = form->controls; c != تہی; c = d) {
		d = c->اگلا;

		form_free_control(c);
	}

	آزاد(form->action);
	آزاد(form->ہدف);
	آزاد(form->accept_charsets);
	آزاد(form->document_charset);

	آزاد(form);
}


/* exported interface documented in html/form_internal.h */
ساخت form_control *form_new_control(خالی *گرہ, form_control_type type)
{
	ساخت form_control *control;

	control = صفر_حافظہ_دو(1, حجم *control);
	اگر (control == تہی)
		واپس تہی;

	control->گرہ = گرہ;
	control->type = type;

	واپس control;
}
