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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/url.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "content/fetch.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/knockout.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SELECT_ಎತ್ತರ 210
#ವ್ಯಾಖ್ಯಾನಿಸು SELECT_ಸಾಲು_SPACING 0.2
#ವ್ಯಾಖ್ಯಾನಿಸು SELECT_BORDER_ಅಗಲ 1
#ವ್ಯಾಖ್ಯಾನಿಸು SELECT_SELECTED_ಬಣ್ಣ 0xDB9370

ರಚನೆ form_select_menu {
	ಪೂರ್ಣಾಂಕ ಸಾಲು_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
	ರಚನೆ scrollbar *scrollbar;
	ಪೂರ್ಣಾಂಕ f_ಗಾತ್ರ;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
	ರಚನೆ content *c;
};

ಸ್ಥಿರ plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_ಬಣ್ಣ = SELECT_SELECTED_ಬಣ್ಣ,
};

ಸ್ಥಿರ plot_font_style_t plot_fstyle_ನಮೂದು = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.ತೂಕ = 400,
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
ಸ್ಥಿರ ಅಕ್ಷರ *
form_encode_ಅಂಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಂಶ,
		 uint32_t len,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
		 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback)
{
	nserror err;
	ಅಕ್ಷರ *ret = ಖಾಲಿ;
	ಅಕ್ಷರ cset[256];

	ಆದರೆ (!ಅಂಶ || !charset)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	snprintf(cset, ಗಾತ್ರ cset, "%s//TRANSLIT", charset);

	err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, cset, 0, &ret);
	ಆದರೆ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, ಗಾತ್ರ cset, "%s", charset);
		err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, cset, len, &ret);

		ಆದರೆ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ಆದರೆ (fallback) {
				snprintf(cset, ಗಾತ್ರ cset,
						"%s//TRANSLIT", fallback);
				err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, cset, 0, &ret);

				ಆದರೆ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, ಗಾತ್ರ cset,
							"%s", fallback);
					err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, cset, 0, &ret);
				}
			}

			ಆದರೆ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ಆದರೆ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_ಗಮ್ಯ_enc(ಅಂಶ, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ಆದರೆ (err == NSERROR_NOMEM) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FETCH_ದತ್ತಾಂಶ_INT_ಮೌಲ್ಯ_ಗಾತ್ರ 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
ಸ್ಥಿರ nserror
fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು_sname(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ksfx,
			  ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ,
			  ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *fetch_ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ keysize;

	fetch_ದತ್ತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*fetch_ದತ್ತಾಂಶ));
	ಆದರೆ (fetch_ದತ್ತಾಂಶ == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_ದತ್ತಾಂಶ->name, 0, "%s%s", name, ksfx);
	fetch_ದತ್ತಾಂಶ->name = ಸ್ಮರಣೆ_ಕೊಡು(keysize + 1); /* allow for null */
	ಆದರೆ (fetch_ದತ್ತಾಂಶ->name == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}
	snprintf(fetch_ದತ್ತಾಂಶ->name, keysize + 1, "%s%s", name, ksfx);

	/*ಮೌಲ್ಯ*/
	fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ = ಸ್ಮರಣೆ_ಕೊಡು(FETCH_ದತ್ತಾಂಶ_INT_ಮೌಲ್ಯ_ಗಾತ್ರ);
	ಆದರೆ (fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ->name);
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ);
		NSLOG(netsurf, INFO, "value allocation failure");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}
	snprintf(fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ, FETCH_ದತ್ತಾಂಶ_INT_ಮೌಲ್ಯ_ಗಾತ್ರ, "%d", ಮೌಲ್ಯ);

	/* link into list */
	**fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr = fetch_ದತ್ತಾಂಶ;
	*fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr = &fetch_ದತ್ತಾಂಶ->ಮುಂದಿನ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(dom_string *name,
		    dom_string *ಮೌಲ್ಯ,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rawfile,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_charset,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *docu_charset,
		    ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *fetch_ದತ್ತಾಂಶ;

	ಪ್ರತಿಜ್ಞಾ(name != ಖಾಲಿ);

	fetch_ದತ್ತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*fetch_ದತ್ತಾಂಶ));
	ಆದರೆ (fetch_ದತ್ತಾಂಶ == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	fetch_ದತ್ತಾಂಶ->name = form_encode_ಅಂಶ(dom_string_ದತ್ತಾಂಶ(name),
					    dom_string_byte_ಉದ್ದಳತೆ(name),
					    form_charset,
					    docu_charset);
	ಆದರೆ (fetch_ದತ್ತಾಂಶ->name == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (ಮೌಲ್ಯ == ಖಾಲಿ) {
		fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ = strdup("");
	} ಇಲ್ಲದಿದ್ದರೆ {
		fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ = form_encode_ಅಂಶ(dom_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ),
						     dom_string_byte_ಉದ್ದಳತೆ(ಮೌಲ್ಯ),
						     form_charset,
						     docu_charset);
	}
	ಆದರೆ (fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ->name);
		ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ಆದರೆ (rawfile != ಖಾಲಿ) {
		fetch_ದತ್ತಾಂಶ->file = true;
		fetch_ದತ್ತಾಂಶ->rawfile = strdup(rawfile);
		ಆದರೆ (fetch_ದತ್ತಾಂಶ->rawfile == ಖಾಲಿ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ->ಮೌಲ್ಯ);
			ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ->name);
			ಬಿಡುಗಡೆ(fetch_ದತ್ತಾಂಶ);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr = fetch_ದತ್ತಾಂಶ;
	*fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr = &fetch_ದತ್ತಾಂಶ->ಮುಂದಿನ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
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
ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_textarea(dom_html_ಪಠ್ಯ_area_element *ಪಠ್ಯ_area_element,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_charset,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *doc_charset,
			  ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	ಘಾತಾಂಕ = dom_html_ಪಠ್ಯ_area_element_get_disabled(ಪಠ್ಯ_area_element,
						      &element_disabled);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* obtain name property */
	ಘಾತಾಂಕ = dom_html_ಪಠ್ಯ_area_element_get_name(ಪಠ್ಯ_area_element,
						  &inputname);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (inputname == ಖಾಲಿ) {
		/* allow enumeration to continue after element with no name */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* obtain text area value */
	ಘಾತಾಂಕ = dom_html_ಪಠ್ಯ_area_element_get_ಮೌಲ್ಯ(ಪಠ್ಯ_area_element,
						   &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", ಘಾತಾಂಕ);
		dom_string_unref(inputname);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
				  inputvalue,
				  ಖಾಲಿ,
				  form_charset,
				  doc_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_charset,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *docu_charset,
			      ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	dom_string *ಮೌಲ್ಯ;
	bool selected;

	ಘಾತಾಂಕ = dom_html_option_element_get_selected(option_element, &selected);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (!selected) {
		/* unselected options do not add fetch data entries */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಘಾತಾಂಕ = dom_html_option_element_get_ಮೌಲ್ಯ(option_element, &ಮೌಲ್ಯ);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(keyname,
				  ಮೌಲ್ಯ,
				  ಖಾಲಿ,
				  form_charset,
				  docu_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(ಮೌಲ್ಯ);

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_select(dom_html_select_element *select_element,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_charset,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *doc_charset,
			ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ಖಾಲಿ;
	uint32_t options_ಎಣಿಕೆ;
	uint32_t option_ಸೂಚ್ಯಂಕ;
	dom_ಗಂಟು *option_element = ಖಾಲಿ;

	/* check if element is disabled */
	ಘಾತಾಂಕ = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* obtain name property */
	ಘಾತಾಂಕ = dom_html_select_element_get_name(select_element, &inputname);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (inputname == ಖಾಲಿ) {
		/* allow enumeration to continue after element with no name */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* get options collection */
	ಘಾತಾಂಕ = dom_html_select_element_get_options(select_element, &options);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* get options collection length */
	ಘಾತಾಂಕ = dom_html_options_collection_get_ಉದ್ದಳತೆ(options, &options_ಎಣಿಕೆ);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* iterate over options collection */
	ಗಾಗಿ (option_ಸೂಚ್ಯಂಕ = 0; option_ಸೂಚ್ಯಂಕ < options_ಎಣಿಕೆ; ++option_ಸೂಚ್ಯಂಕ) {
		ಘಾತಾಂಕ = dom_html_options_collection_ಅಂಶ(options,
						       option_ಸೂಚ್ಯಂಕ,
						       &option_element);
		ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_ಸೂಚ್ಯಂಕ);
			res = NSERROR_DOM;
		} ಇಲ್ಲದಿದ್ದರೆ {
			res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

			dom_ಗಂಟು_unref(option_element);
		}

		ಆದರೆ (res != NSERROR_OK) {
			ನಿಲ್ಲಿಸು;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_submit(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			      dom_string *inputname,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
			      dom_html_element **submit_button,
			      ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ಆದರೆ (*submit_button == ಖಾಲಿ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ಒಳಹರಿವು_element;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*submit_button != (dom_html_element *)ಒಳಹರಿವು_element) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* matched button used to submit form */
	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
				  inputvalue,
				  ಖಾಲಿ,
				  charset,
				  document_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(inputvalue);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_image(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			     dom_string *inputname,
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
			     dom_html_element **submit_button,
			     ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	ರಚನೆ image_ಒಳಹರಿವು_coords *coords;
	ಅಕ್ಷರ *basename;

	/* Only use an image input if it was the thing which activated us */
	ಆದರೆ (*submit_button != (dom_html_element *)ಒಳಹರಿವು_element) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಘಾತಾಂಕ = dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ((dom_ಗಂಟು *)ಒಳಹರಿವು_element,
				     corestring_dom_ns_ಕೀಲಿ_image_coords_ಗಂಟು_ದತ್ತಾಂಶ,
				     &coords);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (coords == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_ಅಂಶ(dom_string_ದತ್ತಾಂಶ(inputname),
				    dom_string_byte_ಉದ್ದಳತೆ(inputname),
				    charset,
				    document_charset);
	ಆದರೆ (basename == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು_sname(basename, ".x",
					coords->x,
					fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	ಆದರೆ (res == NSERROR_OK) {
		res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು_sname(basename, ".y",
						coords->y,
						fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);
	}

	ಬಿಡುಗಡೆ(basename);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_checkbox(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
				dom_string *inputname,
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
				ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_checked(ಒಳಹರಿವು_element, &checked);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (!checked) {
		/* unchecked items do not generate a data entry */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* ensure a default value */
	ಆದರೆ (inputvalue == ಖಾಲಿ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
				  inputvalue,
				  ಖಾಲಿ,
				  charset,
				  document_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(inputvalue);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_file(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			    dom_string *inputname,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
			    ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	dom_string *inputvalue;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rawfile = ಖಾಲಿ;

	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಘಾತಾಂಕ = dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ((dom_ಗಂಟು *)ಒಳಹರಿವು_element,
				     corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_ದತ್ತಾಂಶ,
				     &rawfile);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (rawfile == ಖಾಲಿ) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(inputvalue);

	ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_ಪಠ್ಯ(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
			    dom_string *inputname,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
			    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
			    ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	nserror res;
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	dom_string *inputvalue;

	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(ಒಳಹರಿವು_element, &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
				  inputvalue,
				  ಖಾಲಿ,
				  charset,
				  document_charset,
				  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	dom_string_unref(inputvalue);

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು(dom_html_ಒಳಹರಿವು_element *ಒಳಹರಿವು_element,
		       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
		       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *document_charset,
		       dom_html_element **submit_button,
		       ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_disabled(ಒಳಹರಿವು_element,
						  &element_disabled);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (element_disabled) {
		/* disabled element requires no more processing */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* obtain name property */
	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_name(ಒಳಹರಿವು_element, &inputname);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (inputname == ಖಾಲಿ) {
		/* element with no name is not converted */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* get input type */
	ಘಾತಾಂಕ = dom_html_ಒಳಹರಿವು_element_get_type(ಒಳಹರಿವು_element, &inputtype);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* process according to input element type */
	ಆದರೆ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_submit(ಒಳಹರಿವು_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_image(ಒಳಹರಿವು_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_checkbox(ಒಳಹರಿವು_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_file(ಒಳಹರಿವು_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Everything else is treated as text values */
		res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು_ಪಠ್ಯ(ಒಳಹರಿವು_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_button(dom_html_button_element *button_element,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form_charset,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *doc_charset,
			dom_html_element **submit_button,
			ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ ***fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr)
{
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	ಘಾತಾಂಕ = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* get the type attribute */
	ಘಾತಾಂಕ = dom_html_button_element_get_type(button_element, &inputtype);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ಆದರೆ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}
	ಆದರೆ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ಆದರೆ (*submit_button == ಖಾಲಿ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ಆದರೆ (*submit_button != (dom_html_element *)button_element) {
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* obtain name property */
	ಘಾತಾಂಕ = dom_html_button_element_get_name(button_element, &inputname);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", ಘಾತಾಂಕ);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಆದರೆ (inputname == ಖಾಲಿ) {
		/* allow enumeration to continue after element with no name */
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	ಘಾತಾಂಕ = dom_html_button_element_get_ಮೌಲ್ಯ(button_element, &inputvalue);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} ಇಲ್ಲದಿದ್ದರೆ {
		res = fetch_ದತ್ತಾಂಶ_list_ಕೂಡಿಸು(inputname,
					  inputvalue,
					  ಖಾಲಿ,
					  form_charset,
					  doc_charset,
					  fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	ಹಿಂತಿರುಗಿ res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
ಸ್ಥಿರ ಅಕ್ಷರ *form_acceptable_charset(ರಚನೆ form *form)
{
	ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ, *c;

	ಆದರೆ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ಆದರೆ (form->document_charset) {
			/* document charset present, so use it */
			ಹಿಂತಿರುಗಿ strdup(form->document_charset);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* no document charset, so default to 8859-1 */
			ಹಿಂತಿರುಗಿ strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	ತಾತ್ಕಾಲಿಕ = strdup(form->accept_charsets);
	ಆದರೆ (!ತಾತ್ಕಾಲಿಕ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* make it upper case */
	ಗಾಗಿ (c = ತಾತ್ಕಾಲಿಕ; *c; c++) {
		*c = ascii_ಗಮ್ಯ_ಊರ್ಧ್ವ(*c);
	}

	/* is UTF-8 specified? */
	c = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ತಾತ್ಕಾಲಿಕ, "UTF-8");
	ಆದರೆ (c) {
		ಬಿಡುಗಡೆ(ತಾತ್ಕಾಲಿಕ);
		ಹಿಂತಿರುಗಿ strdup("UTF-8");
	}

	/* dispense with temporary copy */
	ಬಿಡುಗಡೆ(ತಾತ್ಕಾಲಿಕ);

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
	ಆಗ (*c && !ascii_is_space(*c)) {
		ಆದರೆ (*c == ',')
			ನಿಲ್ಲಿಸು;
		c++;
	}

	ಹಿಂತಿರುಗಿ strndup(form->accept_charsets, c - form->accept_charsets);
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
ಸ್ಥಿರ nserror
form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ(ರಚನೆ form *form,
		 ರಚನೆ form_control *submit_control,
		 ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ **fetch_ದತ್ತಾಂಶ_ಹೊರಹೋಗು)
{
	nserror res = NSERROR_OK;
	ಅಕ್ಷರ *charset; /* form characterset */
	dom_exception ಘಾತಾಂಕ; /* the result from DOM operations */
	dom_html_collection *elements = ಖಾಲಿ; /* the dom form elements */
	uint32_t element_ಎಣಿಕೆ; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_ಗಂಟು *element = ಖಾಲಿ; /* the DOM form element */
	dom_string *nodename = ಖಾಲಿ; /* the DOM node name of the element */
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *fetch_ದತ್ತಾಂಶ = ಖಾಲಿ; /* fetch data list */
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ **fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ = &fetch_ದತ್ತಾಂಶ;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ಆದರೆ (submit_control != ಖಾಲಿ) {
		submit_button = submit_control->ಗಂಟು;
	} ಇಲ್ಲದಿದ್ದರೆ {
		submit_button = ಖಾಲಿ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ಆದರೆ (charset == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	ಘಾತಾಂಕ = dom_html_form_element_get_elements(form->ಗಂಟು, &elements);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		ಬಿಡುಗಡೆ(charset);
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಘಾತಾಂಕ = dom_html_collection_get_ಉದ್ದಳತೆ(elements, &element_ಎಣಿಕೆ);
	ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		ಹೋಗು form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_error;
	}

	ಗಾಗಿ (element_idx = 0; element_idx < element_ಎಣಿಕೆ; element_idx++) {
		/* obtain a form element */
		ಘಾತಾಂಕ = dom_html_collection_ಅಂಶ(elements, element_idx, &element);
		ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, ಘಾತಾಂಕ);
			res = NSERROR_DOM;
			ಹೋಗು form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_error;
		}

		/* node name from element */
		ಘಾತಾಂಕ = dom_ಗಂಟು_get_ಗಂಟು_name(element, &nodename);
		ಆದರೆ (ಘಾತಾಂಕ != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, ಘಾತಾಂಕ);
			dom_ಗಂಟು_unref(element);
			res = NSERROR_DOM;
			ಹೋಗು form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_error;
		}

		ಆದರೆ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_textarea(
				(dom_html_ಪಠ್ಯ_area_element *)element,
				charset,
				form->document_charset,
				&fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(nodename, corestring_dom_ಒಳಹರಿವು)) {
			/* Form element is HTMLInputElement */
			res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_ಒಳಹರಿವು(
				(dom_html_ಒಳಹರಿವು_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ);

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_ದತ್ತಾಂಶ_ಮುಂದಿನ);

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (ಪೂರ್ಣಾಂಕ)dom_string_byte_ಉದ್ದಳತೆ(nodename),
			      dom_string_ದತ್ತಾಂಶ(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_ಗಂಟು_unref(element);

		/* abort form element enumeration on error */
		ಆದರೆ (res != NSERROR_OK) {
			ಹೋಗು form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_error;
		}
	}

	*fetch_ದತ್ತಾಂಶ_ಹೊರಹೋಗು = fetch_ದತ್ತಾಂಶ;
	dom_html_collection_unref(elements);
	ಬಿಡುಗಡೆ(charset);

	ಹಿಂತಿರುಗಿ NSERROR_OK;

form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ_error:
	fetch_multipart_ದತ್ತಾಂಶ_destroy(fetch_ದತ್ತಾಂಶ);
	dom_html_collection_unref(elements);
	ಬಿಡುಗಡೆ(charset);

	ಹಿಂತಿರುಗಿ res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
ಸ್ಥಿರ nserror
form_url_encode(ರಚನೆ form *form,
		ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *control,
		ಅಕ್ಷರ **encoded_ಹೊರಹೋಗು)
{
	ಅಕ್ಷರ *name, *ಮೌಲ್ಯ;
	ಅಕ್ಷರ *s, *s2;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len, len1, len_init;
	nserror res;

	s = ಸ್ಮರಣೆ_ಕೊಡು(1);

	ಆದರೆ (s == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	ಗಾಗಿ (; control; control = control->ಮುಂದಿನ) {
		res = url_escape(control->name, true, ಖಾಲಿ, &name);
		ಆದರೆ (res != NSERROR_OK) {
			ಬಿಡುಗಡೆ(s);
			ಹಿಂತಿರುಗಿ res;
		}

		res = url_escape(control->ಮೌಲ್ಯ, true, ಖಾಲಿ, &ಮೌಲ್ಯ);
		ಆದರೆ (res != NSERROR_OK) {
			ಬಿಡುಗಡೆ(name);
			ಬಿಡುಗಡೆ(s);
			ಹಿಂತಿರುಗಿ res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ) + 2;
		s2 = realloc(s, len1 + 1);
		ಆದರೆ (s2 == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
			ಬಿಡುಗಡೆ(name);
			ಬಿಡುಗಡೆ(s);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, ಮೌಲ್ಯ);
		len = len1;
		ಬಿಡುಗಡೆ(name);
		ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
	}

	ಆದರೆ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_ಹೊರಹೋಗು = s;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
ಸ್ಥಿರ ಶೂನ್ಯ
form_select_menu_scroll_callback(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
				 ರಚನೆ scrollbar_ಸಂದೇಶ_ದತ್ತಾಂಶ *scrollbar_ದತ್ತಾಂಶ)
{
	ರಚನೆ form_control *control = client_ದತ್ತಾಂಶ;
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;
	html_content *html = (html_content *)menu->c;

	ಆಯ್ಕೆ (scrollbar_ದತ್ತಾಂಶ->ಸಂದೇಶ) {
		ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_MOVED:
			menu->callback(menu->client_ದತ್ತಾಂಶ,
					0, 0,
					menu->ಅಗಲ,
					menu->ಎತ್ತರ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_SCROLL_ಆರಂಭ:
		{
			ರಚನೆ rect rect = {
				.x0 = scrollbar_ದತ್ತಾಂಶ->x0,
				.y0 = scrollbar_ದತ್ತಾಂಶ->y0,
				.x1 = scrollbar_ದತ್ತಾಂಶ->x1,
				.y1 = scrollbar_ದತ್ತಾಂಶ->y1
			};

			browser_ಕಿಟಕಿ_ಗಣ_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ SCROLLBAR_ಸಂದೇಶ_SCROLL_ಪೂರ್ಣಗೊಂಡ:
			menu->scroll_capture = false;

			browser_ಕಿಟಕಿ_ಗಣ_drag_type(html->bw,
					DRAGGING_NONE, ಖಾಲಿ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
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
ಸ್ಥಿರ nserror
form__select_process_selection(html_content *html,
			       ರಚನೆ form_control *control,
			       ಪೂರ್ಣಾಂಕ ಅಂಶ)
{
	ರಚನೆ box *inline_box;
	ರಚನೆ form_option *o;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
	nserror ret = NSERROR_OK;

	ಪ್ರತಿಜ್ಞಾ(control != ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(html != ಖಾಲಿ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	ಗಾಗಿ (ಎಣಿಕೆ_2 = 0, o = control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು;
			o != ಖಾಲಿ;
			ಎಣಿಕೆ_2++, o = o->ಮುಂದಿನ) {
		ಆದರೆ (!control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ && o->selected) {
			o->selected = false;
			dom_html_option_element_ಗಣ_selected(o->ಗಂಟು, false);
		}

		ಆದರೆ (ಎಣಿಕೆ_2 == ಅಂಶ) {
			ಆದರೆ (control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ) {
				ಆದರೆ (o->selected) {
					o->selected = false;
					dom_html_option_element_ಗಣ_selected(
							o->ಗಂಟು, false);
					control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected--;
				} ಇಲ್ಲದಿದ್ದರೆ {
					o->selected = true;
					dom_html_option_element_ಗಣ_selected(
							o->ಗಂಟು, true);
					control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected++;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				dom_html_option_element_ಗಣ_selected(
						o->ಗಂಟು, true);
				o->selected = true;
			}
		}

		ಆದರೆ (o->selected) {
			control->ದತ್ತಾಂಶ.select.current = o;
		}
	}

	talloc_free(inline_box->ಪಠ್ಯ);
	inline_box->ಪಠ್ಯ = 0;

	ಆದರೆ (control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 0) {
		inline_box->ಪಠ್ಯ = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 1) {
		inline_box->ಪಠ್ಯ = talloc_strdup(html->bctx,
				control->ದತ್ತಾಂಶ.select.current->ಪಠ್ಯ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		inline_box->ಪಠ್ಯ = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ಆದರೆ (!inline_box->ಪಠ್ಯ) {
		ret = NSERROR_NOMEM;
		inline_box->ಉದ್ದಳತೆ = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		inline_box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(inline_box->ಪಠ್ಯ);
	}
	inline_box->ಅಗಲ = control->box->ಅಗಲ;

	html__redraw_a_box(html, control->box);

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
ಸ್ಥಿರ ಶೂನ್ಯ form_select_menu_clicked(ರಚನೆ form_control *control, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;
	ರಚನೆ form_option *option;
	html_content *html = (html_content *)menu->c;
	ಪೂರ್ಣಾಂಕ ಸಾಲು_ಎತ್ತರ, ಸಾಲು_ಎತ್ತರ_with_spacing;
	ಪೂರ್ಣಾಂಕ ಅಂಶ_bottom_y;
	ಪೂರ್ಣಾಂಕ scroll, i;

	scroll = scrollbar_get_ಸ್ಥಾನಾಂತರ(menu->scrollbar);

	ಸಾಲು_ಎತ್ತರ = menu->ಸಾಲು_ಎತ್ತರ;
	ಸಾಲು_ಎತ್ತರ_with_spacing = ಸಾಲು_ಎತ್ತರ +
			ಸಾಲು_ಎತ್ತರ * SELECT_ಸಾಲು_SPACING;

	option = control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು;
	ಅಂಶ_bottom_y = ಸಾಲು_ಎತ್ತರ_with_spacing;
	i = 0;
	ಆಗ (option && ಅಂಶ_bottom_y < scroll + y) {
		ಅಂಶ_bottom_y += ಸಾಲು_ಎತ್ತರ_with_spacing;
		option = option->ಮುಂದಿನ;
		i++;
	}

	ಆದರೆ (option != ಖಾಲಿ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_ದತ್ತಾಂಶ, 0, 0, menu->ಅಗಲ, menu->ಎತ್ತರ);
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_ಕೂಡಿಸು_control(ರಚನೆ form *form, ರಚನೆ form_control *control)
{
	ಆದರೆ (form == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	control->form = form;

	ಆದರೆ (form->controls != ಖಾಲಿ) {
		ಪ್ರತಿಜ್ಞಾ(form->ಕೊನೆಯ_control);

		form->ಕೊನೆಯ_control->ಮುಂದಿನ = control;
		control->prev = form->ಕೊನೆಯ_control;
		control->ಮುಂದಿನ = ಖಾಲಿ;
		form->ಕೊನೆಯ_control = control;
	} ಇಲ್ಲದಿದ್ದರೆ {
		form->controls = form->ಕೊನೆಯ_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free_control(ರಚನೆ form_control *control)
{
	ರಚನೆ form_control *c;
	ಪ್ರತಿಜ್ಞಾ(control != ಖಾಲಿ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->ಮೌಲ್ಯ, control->initial_ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(control->name);
	ಬಿಡುಗಡೆ(control->ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(control->initial_ಮೌಲ್ಯ);
	ಆದರೆ (control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ);
	}

	ಆದರೆ (control->type == GADGET_SELECT) {
		ರಚನೆ form_option *option, *ಮುಂದಿನ;

		ಗಾಗಿ (option = control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು; option;
				option = ಮುಂದಿನ) {
			ಮುಂದಿನ = option->ಮುಂದಿನ;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->ಪಠ್ಯ, option->ಮೌಲ್ಯ);
			ಬಿಡುಗಡೆ(option->ಪಠ್ಯ);
			ಬಿಡುಗಡೆ(option->ಮೌಲ್ಯ);
			ಬಿಡುಗಡೆ(option);
		}
		ಆದರೆ (control->ದತ್ತಾಂಶ.select.menu != ಖಾಲಿ) {
			form_free_select_menu(control);
		}
	}

	ಆದರೆ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ಆದರೆ (control->ದತ್ತಾಂಶ.ಪಠ್ಯ.initial != ಖಾಲಿ) {
			dom_string_unref(control->ದತ್ತಾಂಶ.ಪಠ್ಯ.initial);
		}

		ಆದರೆ (control->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta != ಖಾಲಿ) {
			textarea_destroy(control->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta);
		}
	}

	/* unlink the control from the form */
	ಆದರೆ (control->form != ಖಾಲಿ) {
		ಗಾಗಿ (c = control->form->controls; c != ಖಾಲಿ; c = c->ಮುಂದಿನ) {
			ಆದರೆ (c->ಮುಂದಿನ == control) {
				c->ಮುಂದಿನ = control->ಮುಂದಿನ;
				ಆದರೆ (control->form->ಕೊನೆಯ_control == control)
					control->form->ಕೊನೆಯ_control = c;
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->ಮುಂದಿನ;
				ಆದರೆ (control->form->ಕೊನೆಯ_control == control)
					control->form->controls =
						control->form->ಕೊನೆಯ_control = ಖಾಲಿ;
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಆದರೆ (control->ಗಂಟು_ಮೌಲ್ಯ != ಖಾಲಿ) {
		dom_string_unref(control->ಗಂಟು_ಮೌಲ್ಯ);
	}

	ಬಿಡುಗಡೆ(control);
}


/* exported interface documented in html/form_internal.h */
bool form_ಕೂಡಿಸು_option(ರಚನೆ form_control *control, ಅಕ್ಷರ *ಮೌಲ್ಯ, ಅಕ್ಷರ *ಪಠ್ಯ,
		     bool selected, ಶೂನ್ಯ *ಗಂಟು)
{
	ರಚನೆ form_option *option;

	ಪ್ರತಿಜ್ಞಾ(control);
	ಪ್ರತಿಜ್ಞಾ(control->type == GADGET_SELECT);

	option = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ *option);
	ಆದರೆ (!option)
		ಹಿಂತಿರುಗಿ false;

	option->ಮೌಲ್ಯ = ಮೌಲ್ಯ;
	option->ಪಠ್ಯ = ಪಠ್ಯ;

	/* add to linked list */
	ಆದರೆ (control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು == 0)
		control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು = option;
	ಇಲ್ಲದಿದ್ದರೆ
		control->ದತ್ತಾಂಶ.select.ಕೊನೆಯ_ಅಂಶ->ಮುಂದಿನ = option;
	control->ದತ್ತಾಂಶ.select.ಕೊನೆಯ_ಅಂಶ = option;

	/* set selected */
	ಆದರೆ (selected && (control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 0 ||
			control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ)) {
		option->selected = option->initial_selected = true;
		control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected++;
		control->ದತ್ತಾಂಶ.select.current = option;
	}

	control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_ವಸ್ತುಗಳು++;

	option->ಗಂಟು = ಗಂಟು;

	ಹಿಂತಿರುಗಿ true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		      ರಚನೆ form_control *control,
		      select_menu_redraw_callback callback,
		      ರಚನೆ content *c)
{
	ಪೂರ್ಣಾಂಕ ಸಾಲು_ಎತ್ತರ_with_spacing;
	ರಚನೆ box *box;
	plot_font_style_t fstyle;
	ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_ಎತ್ತರ;
	ರಚನೆ form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ಆದರೆ (control->ದತ್ತಾಂಶ.select.menu == ಖಾಲಿ) {

		menu = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ (ರಚನೆ form_select_menu));
		ಆದರೆ (menu == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}

		control->ದತ್ತಾಂಶ.select.menu = menu;

		box = control->box;

		menu->ಅಗಲ = box->ಅಗಲ +
			box->border[ಬಲ_2].ಅಗಲ + box->padding[ಬಲ_2] +
			box->border[ಎಡ_2].ಅಗಲ + box->padding[ಎಡ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_ಗಾತ್ರ = fstyle.ಗಾತ್ರ_2;

		menu->ಸಾಲು_ಎತ್ತರ = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.ಗಾತ್ರ_2 / PLOT_STYLE_SCALE)))),
				F_72));

		ಸಾಲು_ಎತ್ತರ_with_spacing = menu->ಸಾಲು_ಎತ್ತರ +
				menu->ಸಾಲು_ಎತ್ತರ *
				SELECT_ಸಾಲು_SPACING;

		ಒಟ್ಟು_ಎತ್ತರ = control->ದತ್ತಾಂಶ.select.ಅಂಕೆ_ವಸ್ತುಗಳು *
				ಸಾಲು_ಎತ್ತರ_with_spacing;
		menu->ಎತ್ತರ = ಒಟ್ಟು_ಎತ್ತರ;

		ಆದರೆ (menu->ಎತ್ತರ > MAX_SELECT_ಎತ್ತರ) {
			menu->ಎತ್ತರ = MAX_SELECT_ಎತ್ತರ;
		}

		menu->client_ದತ್ತಾಂಶ = client_ದತ್ತಾಂಶ;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->ಎತ್ತರ,
				       ಒಟ್ಟು_ಎತ್ತರ,
				       menu->ಎತ್ತರ,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		ಆದರೆ (res != NSERROR_OK) {
			control->ದತ್ತಾಂಶ.select.menu = ಖಾಲಿ;
			ಬಿಡುಗಡೆ(menu);
			ಹಿಂತಿರುಗಿ res;
		}
		menu->c = c;
	} ಇಲ್ಲದಿದ್ದರೆ {
		menu = control->ದತ್ತಾಂಶ.select.menu;
	}

	menu->callback(client_ದತ್ತಾಂಶ, 0, 0, menu->ಅಗಲ, menu->ಎತ್ತರ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free_select_menu(ರಚನೆ form_control *control)
{
	ಆದರೆ (control->ದತ್ತಾಂಶ.select.menu->scrollbar != ಖಾಲಿ)
		scrollbar_destroy(control->ದತ್ತಾಂಶ.select.menu->scrollbar);
	ಬಿಡುಗಡೆ(control->ದತ್ತಾಂಶ.select.menu);
	control->ದತ್ತಾಂಶ.select.menu = ಖಾಲಿ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(ರಚನೆ form_control *control,
			ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			ದಶಮಾಂಶ scale,
			ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
			ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ box *box;
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;
	ರಚನೆ form_option *option;
	ಪೂರ್ಣಾಂಕ ಸಾಲು_ಎತ್ತರ, ಸಾಲು_ಎತ್ತರ_with_spacing;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ x0, y0, x1, scrollbar_x, y1, y2, y3;
	ಪೂರ್ಣಾಂಕ ಅಂಶ_y;
	ಪೂರ್ಣಾಂಕ ಪಠ್ಯ_pos_ಸ್ಥಾನಾಂತರ, ಪಠ್ಯ_x;
	ಪೂರ್ಣಾಂಕ scrollbar_ಅಗಲ = SCROLLBAR_ಅಗಲ;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ scroll;
	ಪೂರ್ಣಾಂಕ x_cp, y_cp;
	ರಚನೆ rect r;
	ರಚನೆ rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	ಅಗಲ = menu->ಅಗಲ;
	ಎತ್ತರ = menu->ಎತ್ತರ;
	ಸಾಲು_ಎತ್ತರ = menu->ಸಾಲು_ಎತ್ತರ;

	ಸಾಲು_ಎತ್ತರ_with_spacing = ಸಾಲು_ಎತ್ತರ +
			ಸಾಲು_ಎತ್ತರ * SELECT_ಸಾಲು_SPACING;
	scroll = scrollbar_get_ಸ್ಥಾನಾಂತರ(menu->scrollbar);

	ಆದರೆ (scale != 1.0) {
		x *= scale;
		y *= scale;
		ಅಗಲ *= scale;
		ಎತ್ತರ *= scale;
		scrollbar_ಅಗಲ *= scale;

		i = scroll / ಸಾಲು_ಎತ್ತರ_with_spacing;
		scroll -= i * ಸಾಲು_ಎತ್ತರ_with_spacing;
		ಸಾಲು_ಎತ್ತರ *= scale;
		ಸಾಲು_ಎತ್ತರ_with_spacing *= scale;
		scroll *= scale;
		scroll += i * ಸಾಲು_ಎತ್ತರ_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + ಅಗಲ - 1;
	y1 = y + ಎತ್ತರ - 1;
	scrollbar_x = x1 - scrollbar_ಅಗಲ;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	x0 = x0 + SELECT_BORDER_ಅಗಲ;
	y0 = y0 + SELECT_BORDER_ಅಗಲ;
	x1 = x1 - SELECT_BORDER_ಅಗಲ;
	y1 = y1 - SELECT_BORDER_ಅಗಲ;
	ಎತ್ತರ = ಎತ್ತರ - 2 * SELECT_BORDER_ಅಗಲ;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	option = control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು;
	ಅಂಶ_y = ಸಾಲು_ಎತ್ತರ_with_spacing;

	ಆಗ (ಅಂಶ_y < scroll) {
		option = option->ಮುಂದಿನ;
		ಅಂಶ_y += ಸಾಲು_ಎತ್ತರ_with_spacing;
	}
	ಅಂಶ_y -= ಸಾಲು_ಎತ್ತರ_with_spacing;
	ಪಠ್ಯ_pos_ಸ್ಥಾನಾಂತರ = y - scroll +
			(ಪೂರ್ಣಾಂಕ) (ಸಾಲು_ಎತ್ತರ * (0.75 + SELECT_ಸಾಲು_SPACING));
	ಪಠ್ಯ_x = x + (box->border[ಎಡ_2].ಅಗಲ + box->padding[ಎಡ_2]) * scale;

	plot_fstyle_ನಮೂದು.ಗಾತ್ರ_2 = menu->f_ಗಾತ್ರ;

	ಆಗ (option && ಅಂಶ_y - scroll < ಎತ್ತರ) {

		ಆದರೆ (option->selected) {
			y2 = y + ಅಂಶ_y - scroll;
			y3 = y + ಅಂಶ_y + ಸಾಲು_ಎತ್ತರ_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
		}

		y2 = ಪಠ್ಯ_pos_ಸ್ಥಾನಾಂತರ + ಅಂಶ_y;
		res = ctx->plot->ಪಠ್ಯ(ctx,
				      &plot_fstyle_ನಮೂದು,
				      ಪಠ್ಯ_x, y2,
				      option->ಪಠ್ಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(option->ಪಠ್ಯ));
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}

		ಅಂಶ_y += ಸಾಲು_ಎತ್ತರ_with_spacing;
		option = option->ಮುಂದಿನ;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->ಅಗಲ - SCROLLBAR_ಅಗಲ,
			       y_cp,
			       clip, scale, ctx);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(ರಚನೆ form_control *control,
			     ದಶಮಾಂಶ scale,
			     ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip)
{
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;


	ಅಗಲ = menu->ಅಗಲ;
	ಎತ್ತರ = menu->ಎತ್ತರ;

	ಆದರೆ (scale != 1.0) {
		ಅಗಲ *= scale;
		ಎತ್ತರ *= scale;
	}

	ಆದರೆ (clip->x0 >= 0 &&
	    clip->x1 <= ಅಗಲ &&
	    clip->y0 >= 0 &&
	    clip->y1 <= ಎತ್ತರ)
		ಹಿಂತಿರುಗಿ true;

	ಹಿಂತಿರುಗಿ false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(ರಚನೆ form_control *control, ಪೂರ್ಣಾಂಕ ಅಂಶ)
{
	ಪ್ರತಿಜ್ಞಾ(control != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ form__select_process_selection(control->html, control, ಅಂಶ);
}


/* exported interface documented in netsurf/form.h */
ರಚನೆ form_option *
form_select_get_option(ರಚನೆ form_control *control, ಪೂರ್ಣಾಂಕ ಅಂಶ)
{
	ರಚನೆ form_option *opt;

	opt = control->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು;
	ಆಗ ((opt != ಖಾಲಿ) && (ಅಂಶ > 0)) {
		opt = opt->ಮುಂದಿನ;
		ಅಂಶ--;
	}
	ಹಿಂತಿರುಗಿ opt;
}


/* exported interface documented in netsurf/form.h */
ಅಕ್ಷರ *form_control_get_name(ರಚನೆ form_control *control)
{
	ಹಿಂತಿರುಗಿ control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(ರಚನೆ form_control *control, ರಚನೆ rect *r)
{
	box_bounds( control->box, r );
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/* private interface described in html/form_internal.h */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
form_select_mouse_action(ರಚನೆ form_control *control,
			 browser_mouse_state mouse,
			 ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;
	ಪೂರ್ಣಾಂಕ x0, y0, x1, y1, scrollbar_x;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_2 = ಖಾಲಿ;
	bool ಅಪವರ್ತ್ಯ = control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ;

	x0 = 0;
	y0 = 0;
	x1 = menu->ಅಗಲ;
	y1 = menu->ಎತ್ತರ;
	scrollbar_x = x1 - SCROLLBAR_ಅಗಲ;

	ಆದರೆ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		ಹಿಂತಿರುಗಿ scrollbar_mouse_ಸ್ಥಿತಿ_ಗಮ್ಯ_ಸಂದೇಶ(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ಆದರೆ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ಆದರೆ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ಆದರೆ (!(mouse & BROWSER_MOUSE_CLICK_1 && !ಅಪವರ್ತ್ಯ))
			/* anything but a button 1 click over a single select
			   menu */
			ಸ್ಥಿತಿ_2 = messages_get(control->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ ?
					"SelectMClick" : "SelectClick");

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		ಸ್ಥಿತಿ_2 = messages_get("SelectClose");

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ
form_select_mouse_drag_ಕೊನೆ(ರಚನೆ form_control *control,
			   browser_mouse_state mouse,
			   ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y)
{
	ಪೂರ್ಣಾಂಕ x0, y0, x1, y1;
	ಪೂರ್ಣಾಂಕ box_x, box_y;
	ರಚನೆ box *box;
	ರಚನೆ form_select_menu *menu = control->ದತ್ತಾಂಶ.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ಎಡ_2].ಅಗಲ;
	box_y += box->ಎತ್ತರ + box->border[BOTTOM].ಅಗಲ +
			box->padding[BOTTOM] + box->padding[ತುದಿ_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ಆದರೆ (menu->scroll_capture) {
		x -= menu->ಅಗಲ - SCROLLBAR_ಅಗಲ;
		scrollbar_mouse_drag_ಕೊನೆ(menu->scrollbar, mouse, x, y);
		ಹಿಂತಿರುಗಿ;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->ಅಗಲ;
	y1 = menu->ಎತ್ತರ;


	ಆದರೆ (x > x0 && x < x1 - SCROLLBAR_ಅಗಲ && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_select_get_dimensions(ರಚನೆ form_control *control,
		ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ)
{
	*ಅಗಲ = control->ದತ್ತಾಂಶ.select.menu->ಅಗಲ;
	*ಎತ್ತರ = control->ದತ್ತಾಂಶ.select.menu->ಎತ್ತರ;
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_select_menu_callback(ಶೂನ್ಯ *client_ದತ್ತಾಂಶ,
		ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	html_content *html = client_ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ menu_x, menu_y;
	ರಚನೆ box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ಎಡ_2].ಅಗಲ;
	menu_y += box->ಎತ್ತರ + box->border[BOTTOM].ಅಗಲ +
			box->padding[BOTTOM] +
			box->padding[ತುದಿ_2];
	content__request_redraw((ರಚನೆ content *)html, menu_x + x, menu_y + y,
			ಅಗಲ, ಎತ್ತರ);
}


/* private interface described in html/form_internal.h */
ಶೂನ್ಯ form_radio_ಗಣ(ರಚನೆ form_control *radio)
{
	ರಚನೆ form_control *control;

	ಪ್ರತಿಜ್ಞಾ(radio);
	ಆದರೆ (!radio->form)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (radio->selected)
		ಹಿಂತಿರುಗಿ;

	/* Clear selected state for other controls in
	 * the same radio button group */
	ಗಾಗಿ (control = radio->form->controls;
	     control != ಖಾಲಿ;
	     control = control->ಮುಂದಿನ) {
		/* Only interested in radio inputs */
		ಆದರೆ (control->type != GADGET_RADIO)
			ಮುಂದುವರಿ;

		/* Ignore ourself */
		ಆದರೆ (control == radio)
			ಮುಂದುವರಿ;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ಆದರೆ ((control->name == ಖಾಲಿ) ||
		    (radio->name == ಖಾಲಿ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			ಮುಂದುವರಿ;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ಆದರೆ (control->selected) {
			control->selected = false;
			dom_html_ಒಳಹರಿವು_element_ಗಣ_checked(control->ಗಂಟು, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ಒಳಹರಿವು_element_ಗಣ_checked(radio->ಗಂಟು, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *ಪುಟ_url,
	    ರಚನೆ browser_ಕಿಟಕಿ *ಗುರಿ,
	    ರಚನೆ form *form,
	    ರಚನೆ form_control *submit_button)
{
	nserror res;
	ಅಕ್ಷರ *ದತ್ತಾಂಶ = ಖಾಲಿ; /* encoded form data */
	ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *success = ಖಾಲಿ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ಪ್ರತಿಜ್ಞಾ(form != ಖಾಲಿ);

	/* obtain list of controls from DOM */
	res = form_dom_ಗಮ್ಯ_ದತ್ತಾಂಶ(form, submit_button, &success);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ಆದರೆ (res != NSERROR_OK) {
		fetch_multipart_ದತ್ತಾಂಶ_destroy(success);
		ಹಿಂತಿರುಗಿ res;
	}

	ಆಯ್ಕೆ (form->ವಿಧಾನ) {
	ಸ್ಥಿತಿ ವಿಧಾನ_GET:
		res = form_url_encode(form, success, &ದತ್ತಾಂಶ);
		ಆದರೆ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, ದತ್ತಾಂಶ, &query_url);
			ಆದರೆ (res == NSERROR_OK) {
				res = browser_ಕಿಟಕಿ_navigate(ಗುರಿ,
							      query_url,
							      ಪುಟ_url,
							      BW_NAVIGATE_HISTORY,
							      ಖಾಲಿ,
							      ಖಾಲಿ,
							      ಖಾಲಿ);

				nsurl_unref(query_url);
			}
			ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ವಿಧಾನ_POST_URLENC:
		res = form_url_encode(form, success, &ದತ್ತಾಂಶ);
		ಆದರೆ (res == NSERROR_OK) {
			res = browser_ಕಿಟಕಿ_navigate(ಗುರಿ,
						      action_url,
						      ಪುಟ_url,
						      BW_NAVIGATE_HISTORY,
						      ದತ್ತಾಂಶ,
						      ಖಾಲಿ,
						      ಖಾಲಿ);
			ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ ವಿಧಾನ_POST_MULTIPART:
		res = browser_ಕಿಟಕಿ_navigate(ಗುರಿ,
					      action_url,
					      ಪುಟ_url,
					      BW_NAVIGATE_HISTORY,
					      ಖಾಲಿ,
					      success,
					      ಖಾಲಿ);

		ನಿಲ್ಲಿಸು;
	}

	nsurl_unref(action_url);
	fetch_multipart_ದತ್ತಾಂಶ_destroy(success);

	ಹಿಂತಿರುಗಿ res;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_gadget_update_ಮೌಲ್ಯ(ರಚನೆ form_control *control, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಆಯ್ಕೆ (control->type) {
	ಸ್ಥಿತಿ GADGET_HIDDEN:
	ಸ್ಥಿತಿ GADGET_TEXTBOX:
	ಸ್ಥಿತಿ GADGET_TEXTAREA:
	ಸ್ಥಿತಿ GADGET_PASSWORD:
	ಸ್ಥಿತಿ GADGET_FILE:
		ಆದರೆ (control->ಮೌಲ್ಯ != ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(control->ಮೌಲ್ಯ);
		}
		control->ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಆದರೆ (control->ಗಂಟು != ಖಾಲಿ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)ಮೌಲ್ಯ,
						ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ), &str);
			ಆದರೆ (err == DOM_NO_ERR) {
				ಆದರೆ (control->type == GADGET_TEXTAREA)
					err = dom_html_ಪಠ್ಯ_area_element_ಗಣ_ಮೌಲ್ಯ(
						(dom_html_ಪಠ್ಯ_area_element *)(control->ಗಂಟು),
						str);
				ಇಲ್ಲದಿದ್ದರೆ
					err = dom_html_ಒಳಹರಿವು_element_ಗಣ_ಮೌಲ್ಯ(
						(dom_html_ಒಳಹರಿವು_element *)(control->ಗಂಟು),
						str);
				dom_string_unref(str);
			}
		}
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* Do nothing */
		ನಿಲ್ಲಿಸು;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
ಶೂನ್ಯ
form_gadget_sync_with_dom(ರಚನೆ form_control *control)
{
	dom_exception exc;
	dom_string *ಮೌಲ್ಯ = ಖಾಲಿ;
	bool ಬದಲಾಯಿತು_dom = false;

	ಆದರೆ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		ಹಿಂತಿರುಗಿ;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ಆದರೆ ((control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ == ಖಾಲಿ &&
	     control->ಮೌಲ್ಯ != ಖಾಲಿ &&
	     control->ಮೌಲ್ಯ[0] != '\0') ||
	    (control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ != ಖಾಲಿ &&
	     control->ಮೌಲ್ಯ != ಖಾಲಿ &&
	     strcmp(control->ಮೌಲ್ಯ, control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ) != 0)) {
		ಅಕ್ಷರ *dup = strdup(control->ಮೌಲ್ಯ);
		ಆದರೆ (dup == ಖಾಲಿ) {
			ಹೋಗು ಹೊರಹೋಗು;
		}
		ಆದರೆ (control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ != ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ);
		}
		control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ = dup;
		exc = dom_string_create((uint8_t *)(control->ಮೌಲ್ಯ),
					ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(control->ಮೌಲ್ಯ), &ಮೌಲ್ಯ);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಹೋಗು ಹೊರಹೋಗು;
		}
		ಆದರೆ (control->ಗಂಟು_ಮೌಲ್ಯ != ಖಾಲಿ) {
			dom_string_unref(control->ಗಂಟು_ಮೌಲ್ಯ);
		}
		control->ಗಂಟು_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಮೌಲ್ಯ = ಖಾಲಿ;
		ಆದರೆ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_ಪಠ್ಯ_area_element_ಗಣ_ಮೌಲ್ಯ(control->ಗಂಟು, control->ಗಂಟು_ಮೌಲ್ಯ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			exc = dom_html_ಒಳಹರಿವು_element_ಗಣ_ಮೌಲ್ಯ(control->ಗಂಟು, control->ಗಂಟು_ಮೌಲ್ಯ);
		}
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಹೋಗು ಹೊರಹೋಗು;
		}
		ಬದಲಾಯಿತು_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ಆದರೆ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_ಪಠ್ಯ_area_element_get_ಮೌಲ್ಯ(control->ಗಂಟು, &ಮೌಲ್ಯ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		exc = dom_html_ಒಳಹರಿವು_element_get_ಮೌಲ್ಯ(control->ಗಂಟು, &ಮೌಲ್ಯ);
	}

	ಆದರೆ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (!dom_string_isequal(control->ಗಂಟು_ಮೌಲ್ಯ, ಮೌಲ್ಯ)) {
		/* The DOM has changed */
		ಆದರೆ (!ಬದಲಾಯಿತು_dom) {
			/* And it wasn't us */
			ಅಕ್ಷರ *ಮೌಲ್ಯ_s = strndup(
				dom_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ),
				dom_string_byte_ಉದ್ದಳತೆ(ಮೌಲ್ಯ));
			ಅಕ್ಷರ *dup = ಖಾಲಿ;
			ಆದರೆ (ಮೌಲ್ಯ_s == ಖಾಲಿ) {
				ಹೋಗು ಹೊರಹೋಗು;
			}
			dup = strdup(ಮೌಲ್ಯ_s);
			ಆದರೆ (dup == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(ಮೌಲ್ಯ_s);
				ಹೋಗು ಹೊರಹೋಗು;
			}
			ಬಿಡುಗಡೆ(control->ಮೌಲ್ಯ);
			control->ಮೌಲ್ಯ = ಮೌಲ್ಯ_s;
			ಬಿಡುಗಡೆ(control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ);
			control->ಕೊನೆಯ_synced_ಮೌಲ್ಯ = dup;
			ಆದರೆ (control->type != GADGET_HIDDEN &&
			    control->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta != ಖಾಲಿ) {
				textarea_ಗಣ_ಪಠ್ಯ(control->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
						  ಮೌಲ್ಯ_s);
			}
		}
		control->ಗಂಟು_ಮೌಲ್ಯ = ಮೌಲ್ಯ;
		ಮೌಲ್ಯ = ಖಾಲಿ;
	}

ಹೊರಹೋಗು:
	ಆದರೆ (ಮೌಲ್ಯ != ಖಾಲಿ)
		dom_string_unref(ಮೌಲ್ಯ);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
ರಚನೆ form *
form_new(ಶೂನ್ಯ *ಗಂಟು,
	 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *action,
	 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ,
	 form_ವಿಧಾನ ವಿಧಾನ,
	 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset,
	 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *doc_charset)
{
	ರಚನೆ form *form;

	form = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ *form);
	ಆದರೆ (!form)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	form->action = strdup(action != ಖಾಲಿ ? action : "");
	ಆದರೆ (form->action == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(form);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	form->ಗುರಿ = ಗುರಿ != ಖಾಲಿ ? strdup(ಗುರಿ) : ಖಾಲಿ;
	ಆದರೆ (ಗುರಿ != ಖಾಲಿ && form->ಗುರಿ == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(form->action);
		ಬಿಡುಗಡೆ(form);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	form->ವಿಧಾನ = ವಿಧಾನ;

	form->accept_charsets = charset != ಖಾಲಿ ? strdup(charset) : ಖಾಲಿ;
	ಆದರೆ (charset != ಖಾಲಿ && form->accept_charsets == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(form->ಗುರಿ);
		ಬಿಡುಗಡೆ(form->action);
		ಬಿಡುಗಡೆ(form);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	form->document_charset = doc_charset != ಖಾಲಿ ? strdup(doc_charset)
						     : ಖಾಲಿ;
	ಆದರೆ (doc_charset && form->document_charset == ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(form->accept_charsets);
		ಬಿಡುಗಡೆ(form->ಗುರಿ);
		ಬಿಡುಗಡೆ(form->action);
		ಬಿಡುಗಡೆ(form);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	form->ಗಂಟು = ಗಂಟು;

	ಹಿಂತಿರುಗಿ form;
}


/* exported interface documented in html/form_internal.h */
ಶೂನ್ಯ form_free(ರಚನೆ form *form)
{
	ರಚನೆ form_control *c, *d;

	ಗಾಗಿ (c = form->controls; c != ಖಾಲಿ; c = d) {
		d = c->ಮುಂದಿನ;

		form_free_control(c);
	}

	ಬಿಡುಗಡೆ(form->action);
	ಬಿಡುಗಡೆ(form->ಗುರಿ);
	ಬಿಡುಗಡೆ(form->accept_charsets);
	ಬಿಡುಗಡೆ(form->document_charset);

	ಬಿಡುಗಡೆ(form);
}


/* exported interface documented in html/form_internal.h */
ರಚನೆ form_control *form_new_control(ಶೂನ್ಯ *ಗಂಟು, form_control_type type)
{
	ರಚನೆ form_control *control;

	control = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ *control);
	ಆದರೆ (control == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	control->ಗಂಟು = ಗಂಟು;
	control->type = type;

	ಹಿಂತಿರುಗಿ control;
}
