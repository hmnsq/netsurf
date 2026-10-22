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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/url.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utf8.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/inttypes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/fetch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/knockout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SELECT_ਉਚਾਈ 210
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SELECT_ਸਤਰ_SPACING 0.2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SELECT_BORDER_ਚੌੜਾਈ 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SELECT_SELECTED_ਰੰਗ 0xDB9370

ਬਣਤਰ form_select_menu {
	ਪੂਰਨ ਸਤਰ_ਉਚਾਈ;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
	ਬਣਤਰ scrollbar *scrollbar;
	ਪੂਰਨ f_ਮਾਪ;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	ਖਾਲੀ *client_ਅੰਕੜੇ;
	ਬਣਤਰ content *c;
};

ਸਥਿਰ plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_ਰੰਗ = SELECT_SELECTED_ਰੰਗ,
};

ਸਥਿਰ plot_font_style_t plot_fstyle_ਇੰਦਰਾਜ = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.ਭਾਰ = 400,
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
ਸਥਿਰ ਅੱਖਰ *
form_encode_ਵਸਤੂ(ਅਚਲ ਅੱਖਰ *ਵਸਤੂ,
		 uint32_t len,
		 ਅਚਲ ਅੱਖਰ *charset,
		 ਅਚਲ ਅੱਖਰ *fallback)
{
	nserror err;
	ਅੱਖਰ *ret = ਨਿਰਾ;
	ਅੱਖਰ cset[256];

	ਜੇ (!ਵਸਤੂ || !charset)
		ਵਾਪਸ ਨਿਰਾ;

	snprintf(cset, ਆਕਾਰ cset, "%s//TRANSLIT", charset);

	err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, cset, 0, &ret);
	ਜੇ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, ਆਕਾਰ cset, "%s", charset);
		err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, cset, len, &ret);

		ਜੇ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ਜੇ (fallback) {
				snprintf(cset, ਆਕਾਰ cset,
						"%s//TRANSLIT", fallback);
				err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, cset, 0, &ret);

				ਜੇ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, ਆਕਾਰ cset,
							"%s", fallback);
					err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, cset, 0, &ret);
				}
			}

			ਜੇ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ਜੇ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_ਮੰਜ਼ਿਲ_enc(ਵਸਤੂ, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ਜੇ (err == NSERROR_NOMEM) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FETCH_ਅੰਕੜੇ_INT_ਮੁੱਲ_ਮਾਪ 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ਸਥਿਰ nserror
fetch_ਅੰਕੜੇ_list_ਜੋੜੋ_sname(ਅਚਲ ਅੱਖਰ *name,
			  ਅਚਲ ਅੱਖਰ *ksfx,
			  ਪੂਰਨ ਮੁੱਲ,
			  ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *fetch_ਅੰਕੜੇ;
	ਪੂਰਨ keysize;

	fetch_ਅੰਕੜੇ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*fetch_ਅੰਕੜੇ));
	ਜੇ (fetch_ਅੰਕੜੇ == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_ਅੰਕੜੇ->name, 0, "%s%s", name, ksfx);
	fetch_ਅੰਕੜੇ->name = ਸਮ੍ਰਿਤੀ_ਦਿਓ(keysize + 1); /* allow for null */
	ਜੇ (fetch_ਅੰਕੜੇ->name == ਨਿਰਾ) {
		ਮੁਕਤ(fetch_ਅੰਕੜੇ);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ਵਾਪਸ NSERROR_NOMEM;
	}
	snprintf(fetch_ਅੰਕੜੇ->name, keysize + 1, "%s%s", name, ksfx);

	/*ਮੁੱਲ*/
	fetch_ਅੰਕੜੇ->ਮੁੱਲ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(FETCH_ਅੰਕੜੇ_INT_ਮੁੱਲ_ਮਾਪ);
	ਜੇ (fetch_ਅੰਕੜੇ->ਮੁੱਲ == ਨਿਰਾ) {
		ਮੁਕਤ(fetch_ਅੰਕੜੇ->name);
		ਮੁਕਤ(fetch_ਅੰਕੜੇ);
		NSLOG(netsurf, INFO, "value allocation failure");
		ਵਾਪਸ NSERROR_NOMEM;
	}
	snprintf(fetch_ਅੰਕੜੇ->ਮੁੱਲ, FETCH_ਅੰਕੜੇ_INT_ਮੁੱਲ_ਮਾਪ, "%d", ਮੁੱਲ);

	/* link into list */
	**fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr = fetch_ਅੰਕੜੇ;
	*fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr = &fetch_ਅੰਕੜੇ->ਅਗਲਾ;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(dom_string *name,
		    dom_string *ਮੁੱਲ,
		    ਅਚਲ ਅੱਖਰ *rawfile,
		    ਅਚਲ ਅੱਖਰ *form_charset,
		    ਅਚਲ ਅੱਖਰ *docu_charset,
		    ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *fetch_ਅੰਕੜੇ;

	ਪ੍ਰਤਿਜ੍ਞਾ(name != ਨਿਰਾ);

	fetch_ਅੰਕੜੇ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*fetch_ਅੰਕੜੇ));
	ਜੇ (fetch_ਅੰਕੜੇ == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	fetch_ਅੰਕੜੇ->name = form_encode_ਵਸਤੂ(dom_string_ਅੰਕੜੇ(name),
					    dom_string_byte_ਲੰਮਾਈ(name),
					    form_charset,
					    docu_charset);
	ਜੇ (fetch_ਅੰਕੜੇ->name == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		ਮੁਕਤ(fetch_ਅੰਕੜੇ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (ਮੁੱਲ == ਨਿਰਾ) {
		fetch_ਅੰਕੜੇ->ਮੁੱਲ = strdup("");
	} ਨਹੀਂਤਾਂ {
		fetch_ਅੰਕੜੇ->ਮੁੱਲ = form_encode_ਵਸਤੂ(dom_string_ਅੰਕੜੇ(ਮੁੱਲ),
						     dom_string_byte_ਲੰਮਾਈ(ਮੁੱਲ),
						     form_charset,
						     docu_charset);
	}
	ਜੇ (fetch_ਅੰਕੜੇ->ਮੁੱਲ == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		ਮੁਕਤ(fetch_ਅੰਕੜੇ->name);
		ਮੁਕਤ(fetch_ਅੰਕੜੇ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ਜੇ (rawfile != ਨਿਰਾ) {
		fetch_ਅੰਕੜੇ->file = true;
		fetch_ਅੰਕੜੇ->rawfile = strdup(rawfile);
		ਜੇ (fetch_ਅੰਕੜੇ->rawfile == ਨਿਰਾ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			ਮੁਕਤ(fetch_ਅੰਕੜੇ->ਮੁੱਲ);
			ਮੁਕਤ(fetch_ਅੰਕੜੇ->name);
			ਮੁਕਤ(fetch_ਅੰਕੜੇ);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr = fetch_ਅੰਕੜੇ;
	*fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr = &fetch_ਅੰਕੜੇ->ਅਗਲਾ;

	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_textarea(dom_html_ਪਾਠ_area_element *ਪਾਠ_area_element,
			  ਅਚਲ ਅੱਖਰ *form_charset,
			  ਅਚਲ ਅੱਖਰ *doc_charset,
			  ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	ਘਾਤਾਂਕ = dom_html_ਪਾਠ_area_element_get_disabled(ਪਾਠ_area_element,
						      &element_disabled);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ਵਾਪਸ NSERROR_OK;
	}

	/* obtain name property */
	ਘਾਤਾਂਕ = dom_html_ਪਾਠ_area_element_get_name(ਪਾਠ_area_element,
						  &inputname);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (inputname == ਨਿਰਾ) {
		/* allow enumeration to continue after element with no name */
		ਵਾਪਸ NSERROR_OK;
	}

	/* obtain text area value */
	ਘਾਤਾਂਕ = dom_html_ਪਾਠ_area_element_get_ਮੁੱਲ(ਪਾਠ_area_element,
						   &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", ਘਾਤਾਂਕ);
		dom_string_unref(inputname);
		ਵਾਪਸ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
				  inputvalue,
				  ਨਿਰਾ,
				  form_charset,
				  doc_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ਅਚਲ ਅੱਖਰ *form_charset,
			      ਅਚਲ ਅੱਖਰ *docu_charset,
			      ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	dom_string *ਮੁੱਲ;
	bool selected;

	ਘਾਤਾਂਕ = dom_html_option_element_get_selected(option_element, &selected);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (!selected) {
		/* unselected options do not add fetch data entries */
		ਵਾਪਸ NSERROR_OK;
	}

	ਘਾਤਾਂਕ = dom_html_option_element_get_ਮੁੱਲ(option_element, &ਮੁੱਲ);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(keyname,
				  ਮੁੱਲ,
				  ਨਿਰਾ,
				  form_charset,
				  docu_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(ਮੁੱਲ);

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_select(dom_html_select_element *select_element,
			ਅਚਲ ਅੱਖਰ *form_charset,
			ਅਚਲ ਅੱਖਰ *doc_charset,
			ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ਨਿਰਾ;
	uint32_t options_ਗਿਣਤੀ;
	uint32_t option_ਸੂਚਕਾਂਕ;
	dom_ਗੰਢ *option_element = ਨਿਰਾ;

	/* check if element is disabled */
	ਘਾਤਾਂਕ = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ਵਾਪਸ NSERROR_OK;
	}

	/* obtain name property */
	ਘਾਤਾਂਕ = dom_html_select_element_get_name(select_element, &inputname);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (inputname == ਨਿਰਾ) {
		/* allow enumeration to continue after element with no name */
		ਵਾਪਸ NSERROR_OK;
	}

	/* get options collection */
	ਘਾਤਾਂਕ = dom_html_select_element_get_options(select_element, &options);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ਵਾਪਸ NSERROR_DOM;
	}

	/* get options collection length */
	ਘਾਤਾਂਕ = dom_html_options_collection_get_ਲੰਮਾਈ(options, &options_ਗਿਣਤੀ);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ਵਾਪਸ NSERROR_DOM;
	}

	/* iterate over options collection */
	ਲਈ (option_ਸੂਚਕਾਂਕ = 0; option_ਸੂਚਕਾਂਕ < options_ਗਿਣਤੀ; ++option_ਸੂਚਕਾਂਕ) {
		ਘਾਤਾਂਕ = dom_html_options_collection_ਵਸਤੂ(options,
						       option_ਸੂਚਕਾਂਕ,
						       &option_element);
		ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_ਸੂਚਕਾਂਕ);
			res = NSERROR_DOM;
		} ਨਹੀਂਤਾਂ {
			res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

			dom_ਗੰਢ_unref(option_element);
		}

		ਜੇ (res != NSERROR_OK) {
			ਰੋਕੋ;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_submit(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
			      dom_string *inputname,
			      ਅਚਲ ਅੱਖਰ *charset,
			      ਅਚਲ ਅੱਖਰ *document_charset,
			      dom_html_element **submit_button,
			      ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ਜੇ (*submit_button == ਨਿਰਾ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ਨਿਵੇਸ਼_element;
	} ਨਹੀਂਤਾਂ ਜੇ (*submit_button != (dom_html_element *)ਨਿਵੇਸ਼_element) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* matched button used to submit form */
	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(ਨਿਵੇਸ਼_element, &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
				  inputvalue,
				  ਨਿਰਾ,
				  charset,
				  document_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(inputvalue);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_image(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
			     dom_string *inputname,
			     ਅਚਲ ਅੱਖਰ *charset,
			     ਅਚਲ ਅੱਖਰ *document_charset,
			     dom_html_element **submit_button,
			     ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	ਬਣਤਰ image_ਨਿਵੇਸ਼_coords *coords;
	ਅੱਖਰ *basename;

	/* Only use an image input if it was the thing which activated us */
	ਜੇ (*submit_button != (dom_html_element *)ਨਿਵੇਸ਼_element) {
		ਵਾਪਸ NSERROR_OK;
	}

	ਘਾਤਾਂਕ = dom_ਗੰਢ_get_user_ਅੰਕੜੇ((dom_ਗੰਢ *)ਨਿਵੇਸ਼_element,
				     corestring_dom_ns_ਕੁੰਜੀ_image_coords_ਗੰਢ_ਅੰਕੜੇ,
				     &coords);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (coords == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_ਵਸਤੂ(dom_string_ਅੰਕੜੇ(inputname),
				    dom_string_byte_ਲੰਮਾਈ(inputname),
				    charset,
				    document_charset);
	ਜੇ (basename == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ_sname(basename, ".x",
					coords->x,
					fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	ਜੇ (res == NSERROR_OK) {
		res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ_sname(basename, ".y",
						coords->y,
						fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);
	}

	ਮੁਕਤ(basename);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_checkbox(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
				dom_string *inputname,
				ਅਚਲ ਅੱਖਰ *charset,
				ਅਚਲ ਅੱਖਰ *document_charset,
				ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_checked(ਨਿਵੇਸ਼_element, &checked);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (!checked) {
		/* unchecked items do not generate a data entry */
		ਵਾਪਸ NSERROR_OK;
	}

	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(ਨਿਵੇਸ਼_element, &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* ensure a default value */
	ਜੇ (inputvalue == ਨਿਰਾ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
				  inputvalue,
				  ਨਿਰਾ,
				  charset,
				  document_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(inputvalue);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_file(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
			    dom_string *inputname,
			    ਅਚਲ ਅੱਖਰ *charset,
			    ਅਚਲ ਅੱਖਰ *document_charset,
			    ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	dom_string *inputvalue;
	ਅਚਲ ਅੱਖਰ *rawfile = ਨਿਰਾ;

	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(ਨਿਵੇਸ਼_element, &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਘਾਤਾਂਕ = dom_ਗੰਢ_get_user_ਅੰਕੜੇ((dom_ਗੰਢ *)ਨਿਵੇਸ਼_element,
				     corestring_dom_ns_ਕੁੰਜੀ_file_name_ਗੰਢ_ਅੰਕੜੇ,
				     &rawfile);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (rawfile == ਨਿਰਾ) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(inputvalue);

	ਵਾਪਸ res;
}


ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_ਪਾਠ(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
			    dom_string *inputname,
			    ਅਚਲ ਅੱਖਰ *charset,
			    ਅਚਲ ਅੱਖਰ *document_charset,
			    ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	nserror res;
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	dom_string *inputvalue;

	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(ਨਿਵੇਸ਼_element, &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
				  inputvalue,
				  ਨਿਰਾ,
				  charset,
				  document_charset,
				  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	dom_string_unref(inputvalue);

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼(dom_html_ਨਿਵੇਸ਼_element *ਨਿਵੇਸ਼_element,
		       ਅਚਲ ਅੱਖਰ *charset,
		       ਅਚਲ ਅੱਖਰ *document_charset,
		       dom_html_element **submit_button,
		       ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_disabled(ਨਿਵੇਸ਼_element,
						  &element_disabled);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (element_disabled) {
		/* disabled element requires no more processing */
		ਵਾਪਸ NSERROR_OK;
	}

	/* obtain name property */
	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_name(ਨਿਵੇਸ਼_element, &inputname);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (inputname == ਨਿਰਾ) {
		/* element with no name is not converted */
		ਵਾਪਸ NSERROR_OK;
	}

	/* get input type */
	ਘਾਤਾਂਕ = dom_html_ਨਿਵੇਸ਼_element_get_type(ਨਿਵੇਸ਼_element, &inputtype);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ਵਾਪਸ NSERROR_DOM;
	}

	/* process according to input element type */
	ਜੇ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_submit(ਨਿਵੇਸ਼_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_image(ਨਿਵੇਸ਼_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_checkbox(ਨਿਵੇਸ਼_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_file(ਨਿਵੇਸ਼_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ਨਹੀਂਤਾਂ {
		/* Everything else is treated as text values */
		res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼_ਪਾਠ(ਨਿਵੇਸ਼_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_button(dom_html_button_element *button_element,
			ਅਚਲ ਅੱਖਰ *form_charset,
			ਅਚਲ ਅੱਖਰ *doc_charset,
			dom_html_element **submit_button,
			ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ ***fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr)
{
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	ਘਾਤਾਂਕ = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ਵਾਪਸ NSERROR_OK;
	}

	/* get the type attribute */
	ਘਾਤਾਂਕ = dom_html_button_element_get_type(button_element, &inputtype);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ਵਾਪਸ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ਜੇ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ਵਾਪਸ NSERROR_OK;
	}
	ਜੇ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ਵਾਪਸ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ਜੇ (*submit_button == ਨਿਰਾ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ਜੇ (*submit_button != (dom_html_element *)button_element) {
		ਵਾਪਸ NSERROR_OK;
	}

	/* obtain name property */
	ਘਾਤਾਂਕ = dom_html_button_element_get_name(button_element, &inputname);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", ਘਾਤਾਂਕ);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਜੇ (inputname == ਨਿਰਾ) {
		/* allow enumeration to continue after element with no name */
		ਵਾਪਸ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	ਘਾਤਾਂਕ = dom_html_button_element_get_ਮੁੱਲ(button_element, &inputvalue);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ਨਹੀਂਤਾਂ {
		res = fetch_ਅੰਕੜੇ_list_ਜੋੜੋ(inputname,
					  inputvalue,
					  ਨਿਰਾ,
					  form_charset,
					  doc_charset,
					  fetch_ਅੰਕੜੇ_ਅਗਲਾ_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ਵਾਪਸ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ਸਥਿਰ ਅੱਖਰ *form_acceptable_charset(ਬਣਤਰ form *form)
{
	ਅੱਖਰ *ਆਰਜ਼ੀ, *c;

	ਜੇ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ਜੇ (form->document_charset) {
			/* document charset present, so use it */
			ਵਾਪਸ strdup(form->document_charset);
		} ਨਹੀਂਤਾਂ {
			/* no document charset, so default to 8859-1 */
			ਵਾਪਸ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	ਆਰਜ਼ੀ = strdup(form->accept_charsets);
	ਜੇ (!ਆਰਜ਼ੀ)
		ਵਾਪਸ ਨਿਰਾ;

	/* make it upper case */
	ਲਈ (c = ਆਰਜ਼ੀ; *c; c++) {
		*c = ascii_ਮੰਜ਼ਿਲ_ਉੱਤਲਾ(*c);
	}

	/* is UTF-8 specified? */
	c = ਲੜੀ_ਖੋਜ(ਆਰਜ਼ੀ, "UTF-8");
	ਜੇ (c) {
		ਮੁਕਤ(ਆਰਜ਼ੀ);
		ਵਾਪਸ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	ਮੁਕਤ(ਆਰਜ਼ੀ);

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
	ਜਦੋਂ (*c && !ascii_is_space(*c)) {
		ਜੇ (*c == ',')
			ਰੋਕੋ;
		c++;
	}

	ਵਾਪਸ strndup(form->accept_charsets, c - form->accept_charsets);
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
ਸਥਿਰ nserror
form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ(ਬਣਤਰ form *form,
		 ਬਣਤਰ form_control *submit_control,
		 ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ **fetch_ਅੰਕੜੇ_ਬਾਹਰਲਾ)
{
	nserror res = NSERROR_OK;
	ਅੱਖਰ *charset; /* form characterset */
	dom_exception ਘਾਤਾਂਕ; /* the result from DOM operations */
	dom_html_collection *elements = ਨਿਰਾ; /* the dom form elements */
	uint32_t element_ਗਿਣਤੀ; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ਗੰਢ *element = ਨਿਰਾ; /* the DOM form element */
	dom_string *nodename = ਨਿਰਾ; /* the DOM node name of the element */
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *fetch_ਅੰਕੜੇ = ਨਿਰਾ; /* fetch data list */
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ **fetch_ਅੰਕੜੇ_ਅਗਲਾ = &fetch_ਅੰਕੜੇ;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ਜੇ (submit_control != ਨਿਰਾ) {
		submit_button = submit_control->ਗੰਢ;
	} ਨਹੀਂਤਾਂ {
		submit_button = ਨਿਰਾ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ਜੇ (charset == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	ਘਾਤਾਂਕ = dom_html_form_element_get_elements(form->ਗੰਢ, &elements);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		ਮੁਕਤ(charset);
		ਵਾਪਸ NSERROR_DOM;
	}

	ਘਾਤਾਂਕ = dom_html_collection_get_ਲੰਮਾਈ(elements, &element_ਗਿਣਤੀ);
	ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		ਜਾਓ form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_error;
	}

	ਲਈ (element_idx = 0; element_idx < element_ਗਿਣਤੀ; element_idx++) {
		/* obtain a form element */
		ਘਾਤਾਂਕ = dom_html_collection_ਵਸਤੂ(elements, element_idx, &element);
		ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, ਘਾਤਾਂਕ);
			res = NSERROR_DOM;
			ਜਾਓ form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_error;
		}

		/* node name from element */
		ਘਾਤਾਂਕ = dom_ਗੰਢ_get_ਗੰਢ_name(element, &nodename);
		ਜੇ (ਘਾਤਾਂਕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, ਘਾਤਾਂਕ);
			dom_ਗੰਢ_unref(element);
			res = NSERROR_DOM;
			ਜਾਓ form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_error;
		}

		ਜੇ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_textarea(
				(dom_html_ਪਾਠ_area_element *)element,
				charset,
				form->document_charset,
				&fetch_ਅੰਕੜੇ_ਅਗਲਾ);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_ਅੰਕੜੇ_ਅਗਲਾ);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(nodename, corestring_dom_ਨਿਵੇਸ਼)) {
			/* Form element is HTMLInputElement */
			res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_ਨਿਵੇਸ਼(
				(dom_html_ਨਿਵੇਸ਼_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ਅੰਕੜੇ_ਅਗਲਾ);

		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ਅੰਕੜੇ_ਅਗਲਾ);

		} ਨਹੀਂਤਾਂ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (ਪੂਰਨ)dom_string_byte_ਲੰਮਾਈ(nodename),
			      dom_string_ਅੰਕੜੇ(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ਗੰਢ_unref(element);

		/* abort form element enumeration on error */
		ਜੇ (res != NSERROR_OK) {
			ਜਾਓ form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_error;
		}
	}

	*fetch_ਅੰਕੜੇ_ਬਾਹਰਲਾ = fetch_ਅੰਕੜੇ;
	dom_html_collection_unref(elements);
	ਮੁਕਤ(charset);

	ਵਾਪਸ NSERROR_OK;

form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ_error:
	fetch_multipart_ਅੰਕੜੇ_destroy(fetch_ਅੰਕੜੇ);
	dom_html_collection_unref(elements);
	ਮੁਕਤ(charset);

	ਵਾਪਸ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ਸਥਿਰ nserror
form_url_encode(ਬਣਤਰ form *form,
		ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *control,
		ਅੱਖਰ **encoded_ਬਾਹਰਲਾ)
{
	ਅੱਖਰ *name, *ਮੁੱਲ;
	ਅੱਖਰ *s, *s2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len, len1, len_init;
	nserror res;

	s = ਸਮ੍ਰਿਤੀ_ਦਿਓ(1);

	ਜੇ (s == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	ਲਈ (; control; control = control->ਅਗਲਾ) {
		res = url_escape(control->name, true, ਨਿਰਾ, &name);
		ਜੇ (res != NSERROR_OK) {
			ਮੁਕਤ(s);
			ਵਾਪਸ res;
		}

		res = url_escape(control->ਮੁੱਲ, true, ਨਿਰਾ, &ਮੁੱਲ);
		ਜੇ (res != NSERROR_OK) {
			ਮੁਕਤ(name);
			ਮੁਕਤ(s);
			ਵਾਪਸ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + ਲੜੀ_ਲੰਬਾਈ(name) + ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ) + 2;
		s2 = realloc(s, len1 + 1);
		ਜੇ (s2 == ਨਿਰਾ) {
			ਮੁਕਤ(ਮੁੱਲ);
			ਮੁਕਤ(name);
			ਮੁਕਤ(s);
			ਵਾਪਸ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, ਮੁੱਲ);
		len = len1;
		ਮੁਕਤ(name);
		ਮੁਕਤ(ਮੁੱਲ);
	}

	ਜੇ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_ਬਾਹਰਲਾ = s;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ਸਥਿਰ ਖਾਲੀ
form_select_menu_scroll_callback(ਖਾਲੀ *client_ਅੰਕੜੇ,
				 ਬਣਤਰ scrollbar_ਸੁਨੇਹਾ_ਅੰਕੜੇ *scrollbar_ਅੰਕੜੇ)
{
	ਬਣਤਰ form_control *control = client_ਅੰਕੜੇ;
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;
	html_content *html = (html_content *)menu->c;

	ਚੋਣ (scrollbar_ਅੰਕੜੇ->ਸੁਨੇਹਾ) {
		ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_MOVED:
			menu->callback(menu->client_ਅੰਕੜੇ,
					0, 0,
					menu->ਚੌੜਾਈ,
					menu->ਉਚਾਈ);
			ਰੋਕੋ;
		ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_SCROLL_ਸ਼ੁਰੂ:
		{
			ਬਣਤਰ rect rect = {
				.x0 = scrollbar_ਅੰਕੜੇ->x0,
				.y0 = scrollbar_ਅੰਕੜੇ->y0,
				.x1 = scrollbar_ਅੰਕੜੇ->x1,
				.y1 = scrollbar_ਅੰਕੜੇ->y1
			};

			browser_ਖਿੜਕੀ_ਸੈੱਟ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			ਰੋਕੋ;
		ਹਾਲਤ SCROLLBAR_ਸੁਨੇਹਾ_SCROLL_ਮੁਕੰਮਲ:
			menu->scroll_capture = false;

			browser_ਖਿੜਕੀ_ਸੈੱਟ_drag_type(html->bw,
					DRAGGING_NONE, ਨਿਰਾ);
			ਰੋਕੋ;
		ਮੂਲ:
			ਰੋਕੋ;
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
ਸਥਿਰ nserror
form__select_process_selection(html_content *html,
			       ਬਣਤਰ form_control *control,
			       ਪੂਰਨ ਵਸਤੂ)
{
	ਬਣਤਰ box *inline_box;
	ਬਣਤਰ form_option *o;
	ਪੂਰਨ ਗਿਣਤੀ_2;
	nserror ret = NSERROR_OK;

	ਪ੍ਰਤਿਜ੍ਞਾ(control != ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(html != ਨਿਰਾ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	ਲਈ (ਗਿਣਤੀ_2 = 0, o = control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ;
			o != ਨਿਰਾ;
			ਗਿਣਤੀ_2++, o = o->ਅਗਲਾ) {
		ਜੇ (!control->ਅੰਕੜੇ.select.ਗੁਣਜ && o->selected) {
			o->selected = false;
			dom_html_option_element_ਸੈੱਟ_selected(o->ਗੰਢ, false);
		}

		ਜੇ (ਗਿਣਤੀ_2 == ਵਸਤੂ) {
			ਜੇ (control->ਅੰਕੜੇ.select.ਗੁਣਜ) {
				ਜੇ (o->selected) {
					o->selected = false;
					dom_html_option_element_ਸੈੱਟ_selected(
							o->ਗੰਢ, false);
					control->ਅੰਕੜੇ.select.ਅੰਕ_selected--;
				} ਨਹੀਂਤਾਂ {
					o->selected = true;
					dom_html_option_element_ਸੈੱਟ_selected(
							o->ਗੰਢ, true);
					control->ਅੰਕੜੇ.select.ਅੰਕ_selected++;
				}
			} ਨਹੀਂਤਾਂ {
				dom_html_option_element_ਸੈੱਟ_selected(
						o->ਗੰਢ, true);
				o->selected = true;
			}
		}

		ਜੇ (o->selected) {
			control->ਅੰਕੜੇ.select.current = o;
		}
	}

	talloc_free(inline_box->ਪਾਠ);
	inline_box->ਪਾਠ = 0;

	ਜੇ (control->ਅੰਕੜੇ.select.ਅੰਕ_selected == 0) {
		inline_box->ਪਾਠ = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ਨਹੀਂਤਾਂ ਜੇ (control->ਅੰਕੜੇ.select.ਅੰਕ_selected == 1) {
		inline_box->ਪਾਠ = talloc_strdup(html->bctx,
				control->ਅੰਕੜੇ.select.current->ਪਾਠ);
	} ਨਹੀਂਤਾਂ {
		inline_box->ਪਾਠ = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ਜੇ (!inline_box->ਪਾਠ) {
		ret = NSERROR_NOMEM;
		inline_box->ਲੰਮਾਈ = 0;
	} ਨਹੀਂਤਾਂ {
		inline_box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(inline_box->ਪਾਠ);
	}
	inline_box->ਚੌੜਾਈ = control->box->ਚੌੜਾਈ;

	html__redraw_a_box(html, control->box);

	ਵਾਪਸ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ਸਥਿਰ ਖਾਲੀ form_select_menu_clicked(ਬਣਤਰ form_control *control, ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;
	ਬਣਤਰ form_option *option;
	html_content *html = (html_content *)menu->c;
	ਪੂਰਨ ਸਤਰ_ਉਚਾਈ, ਸਤਰ_ਉਚਾਈ_with_spacing;
	ਪੂਰਨ ਵਸਤੂ_bottom_y;
	ਪੂਰਨ scroll, i;

	scroll = scrollbar_get_ਵਿੱਥ(menu->scrollbar);

	ਸਤਰ_ਉਚਾਈ = menu->ਸਤਰ_ਉਚਾਈ;
	ਸਤਰ_ਉਚਾਈ_with_spacing = ਸਤਰ_ਉਚਾਈ +
			ਸਤਰ_ਉਚਾਈ * SELECT_ਸਤਰ_SPACING;

	option = control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ;
	ਵਸਤੂ_bottom_y = ਸਤਰ_ਉਚਾਈ_with_spacing;
	i = 0;
	ਜਦੋਂ (option && ਵਸਤੂ_bottom_y < scroll + y) {
		ਵਸਤੂ_bottom_y += ਸਤਰ_ਉਚਾਈ_with_spacing;
		option = option->ਅਗਲਾ;
		i++;
	}

	ਜੇ (option != ਨਿਰਾ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_ਅੰਕੜੇ, 0, 0, menu->ਚੌੜਾਈ, menu->ਉਚਾਈ);
}


/* exported interface documented in html/form_internal.h */
ਖਾਲੀ form_ਜੋੜੋ_control(ਬਣਤਰ form *form, ਬਣਤਰ form_control *control)
{
	ਜੇ (form == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	control->form = form;

	ਜੇ (form->controls != ਨਿਰਾ) {
		ਪ੍ਰਤਿਜ੍ਞਾ(form->ਆਖਰੀ_control);

		form->ਆਖਰੀ_control->ਅਗਲਾ = control;
		control->prev = form->ਆਖਰੀ_control;
		control->ਅਗਲਾ = ਨਿਰਾ;
		form->ਆਖਰੀ_control = control;
	} ਨਹੀਂਤਾਂ {
		form->controls = form->ਆਖਰੀ_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
ਖਾਲੀ form_free_control(ਬਣਤਰ form_control *control)
{
	ਬਣਤਰ form_control *c;
	ਪ੍ਰਤਿਜ੍ਞਾ(control != ਨਿਰਾ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->ਮੁੱਲ, control->initial_ਮੁੱਲ);
	ਮੁਕਤ(control->name);
	ਮੁਕਤ(control->ਮੁੱਲ);
	ਮੁਕਤ(control->initial_ਮੁੱਲ);
	ਜੇ (control->ਆਖਰੀ_synced_ਮੁੱਲ != ਨਿਰਾ) {
		ਮੁਕਤ(control->ਆਖਰੀ_synced_ਮੁੱਲ);
	}

	ਜੇ (control->type == GADGET_SELECT) {
		ਬਣਤਰ form_option *option, *ਅਗਲਾ;

		ਲਈ (option = control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ; option;
				option = ਅਗਲਾ) {
			ਅਗਲਾ = option->ਅਗਲਾ;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->ਪਾਠ, option->ਮੁੱਲ);
			ਮੁਕਤ(option->ਪਾਠ);
			ਮੁਕਤ(option->ਮੁੱਲ);
			ਮੁਕਤ(option);
		}
		ਜੇ (control->ਅੰਕੜੇ.select.menu != ਨਿਰਾ) {
			form_free_select_menu(control);
		}
	}

	ਜੇ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ਜੇ (control->ਅੰਕੜੇ.ਪਾਠ.initial != ਨਿਰਾ) {
			dom_string_unref(control->ਅੰਕੜੇ.ਪਾਠ.initial);
		}

		ਜੇ (control->ਅੰਕੜੇ.ਪਾਠ.ta != ਨਿਰਾ) {
			textarea_destroy(control->ਅੰਕੜੇ.ਪਾਠ.ta);
		}
	}

	/* unlink the control from the form */
	ਜੇ (control->form != ਨਿਰਾ) {
		ਲਈ (c = control->form->controls; c != ਨਿਰਾ; c = c->ਅਗਲਾ) {
			ਜੇ (c->ਅਗਲਾ == control) {
				c->ਅਗਲਾ = control->ਅਗਲਾ;
				ਜੇ (control->form->ਆਖਰੀ_control == control)
					control->form->ਆਖਰੀ_control = c;
				ਰੋਕੋ;
			}
			ਜੇ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->ਅਗਲਾ;
				ਜੇ (control->form->ਆਖਰੀ_control == control)
					control->form->controls =
						control->form->ਆਖਰੀ_control = ਨਿਰਾ;
				ਰੋਕੋ;
			}
		}
	}

	ਜੇ (control->ਗੰਢ_ਮੁੱਲ != ਨਿਰਾ) {
		dom_string_unref(control->ਗੰਢ_ਮੁੱਲ);
	}

	ਮੁਕਤ(control);
}


/* exported interface documented in html/form_internal.h */
bool form_ਜੋੜੋ_option(ਬਣਤਰ form_control *control, ਅੱਖਰ *ਮੁੱਲ, ਅੱਖਰ *ਪਾਠ,
		     bool selected, ਖਾਲੀ *ਗੰਢ)
{
	ਬਣਤਰ form_option *option;

	ਪ੍ਰਤਿਜ੍ਞਾ(control);
	ਪ੍ਰਤਿਜ੍ਞਾ(control->type == GADGET_SELECT);

	option = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ *option);
	ਜੇ (!option)
		ਵਾਪਸ false;

	option->ਮੁੱਲ = ਮੁੱਲ;
	option->ਪਾਠ = ਪਾਠ;

	/* add to linked list */
	ਜੇ (control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ == 0)
		control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ = option;
	ਨਹੀਂਤਾਂ
		control->ਅੰਕੜੇ.select.ਆਖਰੀ_ਵਸਤੂ->ਅਗਲਾ = option;
	control->ਅੰਕੜੇ.select.ਆਖਰੀ_ਵਸਤੂ = option;

	/* set selected */
	ਜੇ (selected && (control->ਅੰਕੜੇ.select.ਅੰਕ_selected == 0 ||
			control->ਅੰਕੜੇ.select.ਗੁਣਜ)) {
		option->selected = option->initial_selected = true;
		control->ਅੰਕੜੇ.select.ਅੰਕ_selected++;
		control->ਅੰਕੜੇ.select.current = option;
	}

	control->ਅੰਕੜੇ.select.ਅੰਕ_ਚੀਜ਼ਾਂ++;

	option->ਗੰਢ = ਗੰਢ;

	ਵਾਪਸ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(ਖਾਲੀ *client_ਅੰਕੜੇ,
		      ਬਣਤਰ form_control *control,
		      select_menu_redraw_callback callback,
		      ਬਣਤਰ content *c)
{
	ਪੂਰਨ ਸਤਰ_ਉਚਾਈ_with_spacing;
	ਬਣਤਰ box *box;
	plot_font_style_t fstyle;
	ਪੂਰਨ ਕੁੱਲ_ਉਚਾਈ;
	ਬਣਤਰ form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ਜੇ (control->ਅੰਕੜੇ.select.menu == ਨਿਰਾ) {

		menu = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ (ਬਣਤਰ form_select_menu));
		ਜੇ (menu == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}

		control->ਅੰਕੜੇ.select.menu = menu;

		box = control->box;

		menu->ਚੌੜਾਈ = box->ਚੌੜਾਈ +
			box->border[ਸੱਜਾ_2].ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] +
			box->border[ਖੱਬਾ_2].ਚੌੜਾਈ + box->padding[ਖੱਬਾ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_ਮਾਪ = fstyle.ਮਾਪ;

		menu->ਸਤਰ_ਉਚਾਈ = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.ਮਾਪ / PLOT_STYLE_SCALE)))),
				F_72));

		ਸਤਰ_ਉਚਾਈ_with_spacing = menu->ਸਤਰ_ਉਚਾਈ +
				menu->ਸਤਰ_ਉਚਾਈ *
				SELECT_ਸਤਰ_SPACING;

		ਕੁੱਲ_ਉਚਾਈ = control->ਅੰਕੜੇ.select.ਅੰਕ_ਚੀਜ਼ਾਂ *
				ਸਤਰ_ਉਚਾਈ_with_spacing;
		menu->ਉਚਾਈ = ਕੁੱਲ_ਉਚਾਈ;

		ਜੇ (menu->ਉਚਾਈ > MAX_SELECT_ਉਚਾਈ) {
			menu->ਉਚਾਈ = MAX_SELECT_ਉਚਾਈ;
		}

		menu->client_ਅੰਕੜੇ = client_ਅੰਕੜੇ;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->ਉਚਾਈ,
				       ਕੁੱਲ_ਉਚਾਈ,
				       menu->ਉਚਾਈ,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		ਜੇ (res != NSERROR_OK) {
			control->ਅੰਕੜੇ.select.menu = ਨਿਰਾ;
			ਮੁਕਤ(menu);
			ਵਾਪਸ res;
		}
		menu->c = c;
	} ਨਹੀਂਤਾਂ {
		menu = control->ਅੰਕੜੇ.select.menu;
	}

	menu->callback(client_ਅੰਕੜੇ, 0, 0, menu->ਚੌੜਾਈ, menu->ਉਚਾਈ);

	ਵਾਪਸ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
ਖਾਲੀ form_free_select_menu(ਬਣਤਰ form_control *control)
{
	ਜੇ (control->ਅੰਕੜੇ.select.menu->scrollbar != ਨਿਰਾ)
		scrollbar_destroy(control->ਅੰਕੜੇ.select.menu->scrollbar);
	ਮੁਕਤ(control->ਅੰਕੜੇ.select.menu);
	control->ਅੰਕੜੇ.select.menu = ਨਿਰਾ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ਬਣਤਰ form_control *control,
			ਪੂਰਨ x, ਪੂਰਨ y,
			ਦਸ਼ਮਲਵ scale,
			ਅਚਲ ਬਣਤਰ rect *clip,
			ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ box *box;
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;
	ਬਣਤਰ form_option *option;
	ਪੂਰਨ ਸਤਰ_ਉਚਾਈ, ਸਤਰ_ਉਚਾਈ_with_spacing;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
	ਪੂਰਨ x0, y0, x1, scrollbar_x, y1, y2, y3;
	ਪੂਰਨ ਵਸਤੂ_y;
	ਪੂਰਨ ਪਾਠ_pos_ਵਿੱਥ, ਪਾਠ_x;
	ਪੂਰਨ scrollbar_ਚੌੜਾਈ = SCROLLBAR_ਚੌੜਾਈ;
	ਪੂਰਨ i;
	ਪੂਰਨ scroll;
	ਪੂਰਨ x_cp, y_cp;
	ਬਣਤਰ rect r;
	ਬਣਤਰ rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	ਚੌੜਾਈ = menu->ਚੌੜਾਈ;
	ਉਚਾਈ = menu->ਉਚਾਈ;
	ਸਤਰ_ਉਚਾਈ = menu->ਸਤਰ_ਉਚਾਈ;

	ਸਤਰ_ਉਚਾਈ_with_spacing = ਸਤਰ_ਉਚਾਈ +
			ਸਤਰ_ਉਚਾਈ * SELECT_ਸਤਰ_SPACING;
	scroll = scrollbar_get_ਵਿੱਥ(menu->scrollbar);

	ਜੇ (scale != 1.0) {
		x *= scale;
		y *= scale;
		ਚੌੜਾਈ *= scale;
		ਉਚਾਈ *= scale;
		scrollbar_ਚੌੜਾਈ *= scale;

		i = scroll / ਸਤਰ_ਉਚਾਈ_with_spacing;
		scroll -= i * ਸਤਰ_ਉਚਾਈ_with_spacing;
		ਸਤਰ_ਉਚਾਈ *= scale;
		ਸਤਰ_ਉਚਾਈ_with_spacing *= scale;
		scroll *= scale;
		scroll += i * ਸਤਰ_ਉਚਾਈ_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + ਚੌੜਾਈ - 1;
	y1 = y + ਉਚਾਈ - 1;
	scrollbar_x = x1 - scrollbar_ਚੌੜਾਈ;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	x0 = x0 + SELECT_BORDER_ਚੌੜਾਈ;
	y0 = y0 + SELECT_BORDER_ਚੌੜਾਈ;
	x1 = x1 - SELECT_BORDER_ਚੌੜਾਈ;
	y1 = y1 - SELECT_BORDER_ਚੌੜਾਈ;
	ਉਚਾਈ = ਉਚਾਈ - 2 * SELECT_BORDER_ਚੌੜਾਈ;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	option = control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ;
	ਵਸਤੂ_y = ਸਤਰ_ਉਚਾਈ_with_spacing;

	ਜਦੋਂ (ਵਸਤੂ_y < scroll) {
		option = option->ਅਗਲਾ;
		ਵਸਤੂ_y += ਸਤਰ_ਉਚਾਈ_with_spacing;
	}
	ਵਸਤੂ_y -= ਸਤਰ_ਉਚਾਈ_with_spacing;
	ਪਾਠ_pos_ਵਿੱਥ = y - scroll +
			(ਪੂਰਨ) (ਸਤਰ_ਉਚਾਈ * (0.75 + SELECT_ਸਤਰ_SPACING));
	ਪਾਠ_x = x + (box->border[ਖੱਬਾ_2].ਚੌੜਾਈ + box->padding[ਖੱਬਾ_2]) * scale;

	plot_fstyle_ਇੰਦਰਾਜ.ਮਾਪ = menu->f_ਮਾਪ;

	ਜਦੋਂ (option && ਵਸਤੂ_y - scroll < ਉਚਾਈ) {

		ਜੇ (option->selected) {
			y2 = y + ਵਸਤੂ_y - scroll;
			y3 = y + ਵਸਤੂ_y + ਸਤਰ_ਉਚਾਈ_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
		}

		y2 = ਪਾਠ_pos_ਵਿੱਥ + ਵਸਤੂ_y;
		res = ctx->plot->ਪਾਠ(ctx,
				      &plot_fstyle_ਇੰਦਰਾਜ,
				      ਪਾਠ_x, y2,
				      option->ਪਾਠ, ਲੜੀ_ਲੰਬਾਈ(option->ਪਾਠ));
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}

		ਵਸਤੂ_y += ਸਤਰ_ਉਚਾਈ_with_spacing;
		option = option->ਅਗਲਾ;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->ਚੌੜਾਈ - SCROLLBAR_ਚੌੜਾਈ,
			       y_cp,
			       clip, scale, ctx);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ਬਣਤਰ form_control *control,
			     ਦਸ਼ਮਲਵ scale,
			     ਅਚਲ ਬਣਤਰ rect *clip)
{
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;


	ਚੌੜਾਈ = menu->ਚੌੜਾਈ;
	ਉਚਾਈ = menu->ਉਚਾਈ;

	ਜੇ (scale != 1.0) {
		ਚੌੜਾਈ *= scale;
		ਉਚਾਈ *= scale;
	}

	ਜੇ (clip->x0 >= 0 &&
	    clip->x1 <= ਚੌੜਾਈ &&
	    clip->y0 >= 0 &&
	    clip->y1 <= ਉਚਾਈ)
		ਵਾਪਸ true;

	ਵਾਪਸ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ਬਣਤਰ form_control *control, ਪੂਰਨ ਵਸਤੂ)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(control != ਨਿਰਾ);

	ਵਾਪਸ form__select_process_selection(control->html, control, ਵਸਤੂ);
}


/* exported interface documented in netsurf/form.h */
ਬਣਤਰ form_option *
form_select_get_option(ਬਣਤਰ form_control *control, ਪੂਰਨ ਵਸਤੂ)
{
	ਬਣਤਰ form_option *opt;

	opt = control->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ;
	ਜਦੋਂ ((opt != ਨਿਰਾ) && (ਵਸਤੂ > 0)) {
		opt = opt->ਅਗਲਾ;
		ਵਸਤੂ--;
	}
	ਵਾਪਸ opt;
}


/* exported interface documented in netsurf/form.h */
ਅੱਖਰ *form_control_get_name(ਬਣਤਰ form_control *control)
{
	ਵਾਪਸ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ਬਣਤਰ form_control *control, ਬਣਤਰ rect *r)
{
	box_bounds( control->box, r );
	ਵਾਪਸ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ਅਚਲ ਅੱਖਰ *
form_select_mouse_action(ਬਣਤਰ form_control *control,
			 browser_mouse_state mouse,
			 ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;
	ਪੂਰਨ x0, y0, x1, y1, scrollbar_x;
	ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ = ਨਿਰਾ;
	bool ਗੁਣਜ = control->ਅੰਕੜੇ.select.ਗੁਣਜ;

	x0 = 0;
	y0 = 0;
	x1 = menu->ਚੌੜਾਈ;
	y1 = menu->ਉਚਾਈ;
	scrollbar_x = x1 - SCROLLBAR_ਚੌੜਾਈ;

	ਜੇ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ਵਾਪਸ scrollbar_mouse_ਸਥਿਤੀ_ਮੰਜ਼ਿਲ_ਸੁਨੇਹਾ(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ਜੇ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ਜੇ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ਜੇ (!(mouse & BROWSER_MOUSE_CLICK_1 && !ਗੁਣਜ))
			/* anything but a button 1 click over a single select
			   menu */
			ਸਥਿਤੀ = messages_get(control->ਅੰਕੜੇ.select.ਗੁਣਜ ?
					"SelectMClick" : "SelectClick");

	} ਨਹੀਂਤਾਂ ਜੇ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		ਸਥਿਤੀ = messages_get("SelectClose");

	ਵਾਪਸ ਸਥਿਤੀ;
}


/* private interface described in html/form_internal.h */
ਖਾਲੀ
form_select_mouse_drag_ਸਿਰਾ(ਬਣਤਰ form_control *control,
			   browser_mouse_state mouse,
			   ਪੂਰਨ x, ਪੂਰਨ y)
{
	ਪੂਰਨ x0, y0, x1, y1;
	ਪੂਰਨ box_x, box_y;
	ਬਣਤਰ box *box;
	ਬਣਤਰ form_select_menu *menu = control->ਅੰਕੜੇ.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	box_y += box->ਉਚਾਈ + box->border[BOTTOM].ਚੌੜਾਈ +
			box->padding[BOTTOM] + box->padding[ਸਿਖਰ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ਜੇ (menu->scroll_capture) {
		x -= menu->ਚੌੜਾਈ - SCROLLBAR_ਚੌੜਾਈ;
		scrollbar_mouse_drag_ਸਿਰਾ(menu->scrollbar, mouse, x, y);
		ਵਾਪਸ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->ਚੌੜਾਈ;
	y1 = menu->ਉਚਾਈ;


	ਜੇ (x > x0 && x < x1 - SCROLLBAR_ਚੌੜਾਈ && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
ਖਾਲੀ form_select_get_dimensions(ਬਣਤਰ form_control *control,
		ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ)
{
	*ਚੌੜਾਈ = control->ਅੰਕੜੇ.select.menu->ਚੌੜਾਈ;
	*ਉਚਾਈ = control->ਅੰਕੜੇ.select.menu->ਉਚਾਈ;
}


/* private interface described in html/form_internal.h */
ਖਾਲੀ form_select_menu_callback(ਖਾਲੀ *client_ਅੰਕੜੇ,
		ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	html_content *html = client_ਅੰਕੜੇ;
	ਪੂਰਨ menu_x, menu_y;
	ਬਣਤਰ box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	menu_y += box->ਉਚਾਈ + box->border[BOTTOM].ਚੌੜਾਈ +
			box->padding[BOTTOM] +
			box->padding[ਸਿਖਰ_2];
	content__request_redraw((ਬਣਤਰ content *)html, menu_x + x, menu_y + y,
			ਚੌੜਾਈ, ਉਚਾਈ);
}


/* private interface described in html/form_internal.h */
ਖਾਲੀ form_radio_ਸੈੱਟ(ਬਣਤਰ form_control *radio)
{
	ਬਣਤਰ form_control *control;

	ਪ੍ਰਤਿਜ੍ਞਾ(radio);
	ਜੇ (!radio->form)
		ਵਾਪਸ;

	ਜੇ (radio->selected)
		ਵਾਪਸ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	ਲਈ (control = radio->form->controls;
	     control != ਨਿਰਾ;
	     control = control->ਅਗਲਾ) {
		/* Only interested in radio inputs */
		ਜੇ (control->type != GADGET_RADIO)
			ਜਾਰੀ;

		/* Ignore ourself */
		ਜੇ (control == radio)
			ਜਾਰੀ;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ਜੇ ((control->name == ਨਿਰਾ) ||
		    (radio->name == ਨਿਰਾ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ਜਾਰੀ;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ਜੇ (control->selected) {
			control->selected = false;
			dom_html_ਨਿਵੇਸ਼_element_ਸੈੱਟ_checked(control->ਗੰਢ, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ਨਿਵੇਸ਼_element_ਸੈੱਟ_checked(radio->ਗੰਢ, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *ਪੰਨਾ_url,
	    ਬਣਤਰ browser_ਖਿੜਕੀ *ਨਿਸ਼ਾਨਾ,
	    ਬਣਤਰ form *form,
	    ਬਣਤਰ form_control *submit_button)
{
	nserror res;
	ਅੱਖਰ *ਅੰਕੜੇ = ਨਿਰਾ; /* encoded form data */
	ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *success = ਨਿਰਾ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ਪ੍ਰਤਿਜ੍ਞਾ(form != ਨਿਰਾ);

	/* obtain list of controls from DOM */
	res = form_dom_ਮੰਜ਼ਿਲ_ਅੰਕੜੇ(form, submit_button, &success);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ਜੇ (res != NSERROR_OK) {
		fetch_multipart_ਅੰਕੜੇ_destroy(success);
		ਵਾਪਸ res;
	}

	ਚੋਣ (form->ਵਿਧੀ) {
	ਹਾਲਤ ਵਿਧੀ_GET:
		res = form_url_encode(form, success, &ਅੰਕੜੇ);
		ਜੇ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, ਅੰਕੜੇ, &query_url);
			ਜੇ (res == NSERROR_OK) {
				res = browser_ਖਿੜਕੀ_navigate(ਨਿਸ਼ਾਨਾ,
							      query_url,
							      ਪੰਨਾ_url,
							      BW_NAVIGATE_HISTORY,
							      ਨਿਰਾ,
							      ਨਿਰਾ,
							      ਨਿਰਾ);

				nsurl_unref(query_url);
			}
			ਮੁਕਤ(ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ ਵਿਧੀ_POST_URLENC:
		res = form_url_encode(form, success, &ਅੰਕੜੇ);
		ਜੇ (res == NSERROR_OK) {
			res = browser_ਖਿੜਕੀ_navigate(ਨਿਸ਼ਾਨਾ,
						      action_url,
						      ਪੰਨਾ_url,
						      BW_NAVIGATE_HISTORY,
						      ਅੰਕੜੇ,
						      ਨਿਰਾ,
						      ਨਿਰਾ);
			ਮੁਕਤ(ਅੰਕੜੇ);
		}
		ਰੋਕੋ;

	ਹਾਲਤ ਵਿਧੀ_POST_MULTIPART:
		res = browser_ਖਿੜਕੀ_navigate(ਨਿਸ਼ਾਨਾ,
					      action_url,
					      ਪੰਨਾ_url,
					      BW_NAVIGATE_HISTORY,
					      ਨਿਰਾ,
					      success,
					      ਨਿਰਾ);

		ਰੋਕੋ;
	}

	nsurl_unref(action_url);
	fetch_multipart_ਅੰਕੜੇ_destroy(success);

	ਵਾਪਸ res;
}


/* exported interface documented in html/form_internal.h */
ਖਾਲੀ form_gadget_update_ਮੁੱਲ(ਬਣਤਰ form_control *control, ਅੱਖਰ *ਮੁੱਲ)
{
	ਚੋਣ (control->type) {
	ਹਾਲਤ GADGET_HIDDEN:
	ਹਾਲਤ GADGET_TEXTBOX:
	ਹਾਲਤ GADGET_TEXTAREA:
	ਹਾਲਤ GADGET_PASSWORD:
	ਹਾਲਤ GADGET_FILE:
		ਜੇ (control->ਮੁੱਲ != ਨਿਰਾ) {
			ਮੁਕਤ(control->ਮੁੱਲ);
		}
		control->ਮੁੱਲ = ਮੁੱਲ;
		ਜੇ (control->ਗੰਢ != ਨਿਰਾ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)ਮੁੱਲ,
						ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ), &str);
			ਜੇ (err == DOM_NO_ERR) {
				ਜੇ (control->type == GADGET_TEXTAREA)
					err = dom_html_ਪਾਠ_area_element_ਸੈੱਟ_ਮੁੱਲ(
						(dom_html_ਪਾਠ_area_element *)(control->ਗੰਢ),
						str);
				ਨਹੀਂਤਾਂ
					err = dom_html_ਨਿਵੇਸ਼_element_ਸੈੱਟ_ਮੁੱਲ(
						(dom_html_ਨਿਵੇਸ਼_element *)(control->ਗੰਢ),
						str);
				dom_string_unref(str);
			}
		}
		ਰੋਕੋ;
	ਮੂਲ:
		/* Do nothing */
		ਰੋਕੋ;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
ਖਾਲੀ
form_gadget_sync_with_dom(ਬਣਤਰ form_control *control)
{
	dom_exception exc;
	dom_string *ਮੁੱਲ = ਨਿਰਾ;
	bool ਬਦਲਿਆ_dom = false;

	ਜੇ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ਵਾਪਸ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ਜੇ ((control->ਆਖਰੀ_synced_ਮੁੱਲ == ਨਿਰਾ &&
	     control->ਮੁੱਲ != ਨਿਰਾ &&
	     control->ਮੁੱਲ[0] != '\0') ||
	    (control->ਆਖਰੀ_synced_ਮੁੱਲ != ਨਿਰਾ &&
	     control->ਮੁੱਲ != ਨਿਰਾ &&
	     strcmp(control->ਮੁੱਲ, control->ਆਖਰੀ_synced_ਮੁੱਲ) != 0)) {
		ਅੱਖਰ *dup = strdup(control->ਮੁੱਲ);
		ਜੇ (dup == ਨਿਰਾ) {
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		ਜੇ (control->ਆਖਰੀ_synced_ਮੁੱਲ != ਨਿਰਾ) {
			ਮੁਕਤ(control->ਆਖਰੀ_synced_ਮੁੱਲ);
		}
		control->ਆਖਰੀ_synced_ਮੁੱਲ = dup;
		exc = dom_string_create((uint8_t *)(control->ਮੁੱਲ),
					ਲੜੀ_ਲੰਬਾਈ(control->ਮੁੱਲ), &ਮੁੱਲ);
		ਜੇ (exc != DOM_NO_ERR) {
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		ਜੇ (control->ਗੰਢ_ਮੁੱਲ != ਨਿਰਾ) {
			dom_string_unref(control->ਗੰਢ_ਮੁੱਲ);
		}
		control->ਗੰਢ_ਮੁੱਲ = ਮੁੱਲ;
		ਮੁੱਲ = ਨਿਰਾ;
		ਜੇ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_ਪਾਠ_area_element_ਸੈੱਟ_ਮੁੱਲ(control->ਗੰਢ, control->ਗੰਢ_ਮੁੱਲ);
		} ਨਹੀਂਤਾਂ {
			exc = dom_html_ਨਿਵੇਸ਼_element_ਸੈੱਟ_ਮੁੱਲ(control->ਗੰਢ, control->ਗੰਢ_ਮੁੱਲ);
		}
		ਜੇ (exc != DOM_NO_ERR) {
			ਜਾਓ ਬਾਹਰਲਾ;
		}
		ਬਦਲਿਆ_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ਜੇ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_ਪਾਠ_area_element_get_ਮੁੱਲ(control->ਗੰਢ, &ਮੁੱਲ);
	} ਨਹੀਂਤਾਂ {
		exc = dom_html_ਨਿਵੇਸ਼_element_get_ਮੁੱਲ(control->ਗੰਢ, &ਮੁੱਲ);
	}

	ਜੇ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (!dom_string_isequal(control->ਗੰਢ_ਮੁੱਲ, ਮੁੱਲ)) {
		/* The DOM has changed */
		ਜੇ (!ਬਦਲਿਆ_dom) {
			/* And it wasn't us */
			ਅੱਖਰ *ਮੁੱਲ_s = strndup(
				dom_string_ਅੰਕੜੇ(ਮੁੱਲ),
				dom_string_byte_ਲੰਮਾਈ(ਮੁੱਲ));
			ਅੱਖਰ *dup = ਨਿਰਾ;
			ਜੇ (ਮੁੱਲ_s == ਨਿਰਾ) {
				ਜਾਓ ਬਾਹਰਲਾ;
			}
			dup = strdup(ਮੁੱਲ_s);
			ਜੇ (dup == ਨਿਰਾ) {
				ਮੁਕਤ(ਮੁੱਲ_s);
				ਜਾਓ ਬਾਹਰਲਾ;
			}
			ਮੁਕਤ(control->ਮੁੱਲ);
			control->ਮੁੱਲ = ਮੁੱਲ_s;
			ਮੁਕਤ(control->ਆਖਰੀ_synced_ਮੁੱਲ);
			control->ਆਖਰੀ_synced_ਮੁੱਲ = dup;
			ਜੇ (control->type != GADGET_HIDDEN &&
			    control->ਅੰਕੜੇ.ਪਾਠ.ta != ਨਿਰਾ) {
				textarea_ਸੈੱਟ_ਪਾਠ(control->ਅੰਕੜੇ.ਪਾਠ.ta,
						  ਮੁੱਲ_s);
			}
		}
		control->ਗੰਢ_ਮੁੱਲ = ਮੁੱਲ;
		ਮੁੱਲ = ਨਿਰਾ;
	}

ਬਾਹਰਲਾ:
	ਜੇ (ਮੁੱਲ != ਨਿਰਾ)
		dom_string_unref(ਮੁੱਲ);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ਬਣਤਰ form *
form_new(ਖਾਲੀ *ਗੰਢ,
	 ਅਚਲ ਅੱਖਰ *action,
	 ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ,
	 form_ਵਿਧੀ ਵਿਧੀ,
	 ਅਚਲ ਅੱਖਰ *charset,
	 ਅਚਲ ਅੱਖਰ *doc_charset)
{
	ਬਣਤਰ form *form;

	form = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ *form);
	ਜੇ (!form)
		ਵਾਪਸ ਨਿਰਾ;

	form->action = strdup(action != ਨਿਰਾ ? action : "");
	ਜੇ (form->action == ਨਿਰਾ) {
		ਮੁਕਤ(form);
		ਵਾਪਸ ਨਿਰਾ;
	}

	form->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ != ਨਿਰਾ ? strdup(ਨਿਸ਼ਾਨਾ) : ਨਿਰਾ;
	ਜੇ (ਨਿਸ਼ਾਨਾ != ਨਿਰਾ && form->ਨਿਸ਼ਾਨਾ == ਨਿਰਾ) {
		ਮੁਕਤ(form->action);
		ਮੁਕਤ(form);
		ਵਾਪਸ ਨਿਰਾ;
	}

	form->ਵਿਧੀ = ਵਿਧੀ;

	form->accept_charsets = charset != ਨਿਰਾ ? strdup(charset) : ਨਿਰਾ;
	ਜੇ (charset != ਨਿਰਾ && form->accept_charsets == ਨਿਰਾ) {
		ਮੁਕਤ(form->ਨਿਸ਼ਾਨਾ);
		ਮੁਕਤ(form->action);
		ਮੁਕਤ(form);
		ਵਾਪਸ ਨਿਰਾ;
	}

	form->document_charset = doc_charset != ਨਿਰਾ ? strdup(doc_charset)
						     : ਨਿਰਾ;
	ਜੇ (doc_charset && form->document_charset == ਨਿਰਾ) {
		ਮੁਕਤ(form->accept_charsets);
		ਮੁਕਤ(form->ਨਿਸ਼ਾਨਾ);
		ਮੁਕਤ(form->action);
		ਮੁਕਤ(form);
		ਵਾਪਸ ਨਿਰਾ;
	}

	form->ਗੰਢ = ਗੰਢ;

	ਵਾਪਸ form;
}


/* exported interface documented in html/form_internal.h */
ਖਾਲੀ form_free(ਬਣਤਰ form *form)
{
	ਬਣਤਰ form_control *c, *d;

	ਲਈ (c = form->controls; c != ਨਿਰਾ; c = d) {
		d = c->ਅਗਲਾ;

		form_free_control(c);
	}

	ਮੁਕਤ(form->action);
	ਮੁਕਤ(form->ਨਿਸ਼ਾਨਾ);
	ਮੁਕਤ(form->accept_charsets);
	ਮੁਕਤ(form->document_charset);

	ਮੁਕਤ(form);
}


/* exported interface documented in html/form_internal.h */
ਬਣਤਰ form_control *form_new_control(ਖਾਲੀ *ਗੰਢ, form_control_type type)
{
	ਬਣਤਰ form_control *control;

	control = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ *control);
	ਜੇ (control == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	control->ਗੰਢ = ਗੰਢ;
	control->type = type;

	ਵਾਪਸ control;
}
