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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/url.h"
#అన్తర్భావ "utils/utf8.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/mouse.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "css/utils.h"
#అన్తర్భావ "desktop/knockout.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/textarea.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/layout.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/form_internal.h"

#నిర్వచించు MAX_SELECT_ఎత్తు 210
#నిర్వచించు SELECT_పంక్తి_SPACING 0.2
#నిర్వచించు SELECT_BORDER_వెడల్పు 1
#నిర్వచించు SELECT_SELECTED_రంగు 0xDB9370

నిర్మాణం form_select_menu {
	పూర్ణాంకం పంక్తి_ఎత్తు;
	పూర్ణాంకం వెడల్పు, ఎత్తు;
	నిర్మాణం scrollbar *scrollbar;
	పూర్ణాంకం f_కొలత;
	bool scroll_capture;
	select_menu_redraw_callback callback;
	శూన్య *client_దత్తాంశం;
	నిర్మాణం content *c;
};

స్థిర plot_style_t plot_style_fill_selected = {
	.fill_type = PLOT_OP_TYPE_SOLID,
	.fill_రంగు = SELECT_SELECTED_రంగు,
};

స్థిర plot_font_style_t plot_fstyle_నమోదు = {
	.family = PLOT_FONT_FAMILY_SANS_SERIF,
	.బరువు = 400,
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
స్థిర అక్షరం *
form_encode_అంశం(స్థిరరాశి అక్షరం *అంశం,
		 uint32_t len,
		 స్థిరరాశి అక్షరం *charset,
		 స్థిరరాశి అక్షరం *fallback)
{
	nserror err;
	అక్షరం *ret = ఖాళీ;
	అక్షరం cset[256];

	ఒకవేళ (!అంశం || !charset)
		తిరిగి ఖాళీ;

	snprintf(cset, పరిమాణం cset, "%s//TRANSLIT", charset);

	err = utf8_గమ్యం_enc(అంశం, cset, 0, &ret);
	ఒకవేళ (err == NSERROR_BAD_ENCODING) {
		/* charset not understood, try without transliteration */
		snprintf(cset, పరిమాణం cset, "%s", charset);
		err = utf8_గమ్యం_enc(అంశం, cset, len, &ret);

		ఒకవేళ (err == NSERROR_BAD_ENCODING) {
			/* nope, try fallback charset (if any) */
			ఒకవేళ (fallback) {
				snprintf(cset, పరిమాణం cset,
						"%s//TRANSLIT", fallback);
				err = utf8_గమ్యం_enc(అంశం, cset, 0, &ret);

				ఒకవేళ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					snprintf(cset, పరిమాణం cset,
							"%s", fallback);
					err = utf8_గమ్యం_enc(అంశం, cset, 0, &ret);
				}
			}

			ఒకవేళ (err == NSERROR_BAD_ENCODING) {
				/* that also failed, use 8859-1 */
				err = utf8_గమ్యం_enc(అంశం, "ISO-8859-1//TRANSLIT",
						0, &ret);
				ఒకవేళ (err == NSERROR_BAD_ENCODING) {
					/* and without transliteration */
					err = utf8_గమ్యం_enc(అంశం, "ISO-8859-1",
							0, &ret);
				}
			}
		}
	}
	ఒకవేళ (err == NSERROR_NOMEM) {
		తిరిగి ఖాళీ;
	}

	తిరిగి ret;
}


/**
 * string allocation size for numeric values in multipart data
 */
#నిర్వచించు FETCH_దత్తాంశం_INT_విలువ_కొలత 20


/**
 * append split key name and integer value to a multipart data list
 *
 * \param name key name
 * \param ksfx key name suffix
 * \param value The value to encode
 * \param fetch_data_next_ptr The multipart data list to append to.
 */
స్థిర nserror
fetch_దత్తాంశం_list_కూడు_sname(స్థిరరాశి అక్షరం *name,
			  స్థిరరాశి అక్షరం *ksfx,
			  పూర్ణాంకం విలువ,
			  నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	నిర్మాణం fetch_multipart_దత్తాంశం *fetch_దత్తాంశం;
	పూర్ణాంకం keysize;

	fetch_దత్తాంశం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*fetch_దత్తాంశం));
	ఒకవేళ (fetch_దత్తాంశం == ఖాళీ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		తిరిగి NSERROR_NOMEM;
	}

	/* key name */
	keysize = snprintf(fetch_దత్తాంశం->name, 0, "%s%s", name, ksfx);
	fetch_దత్తాంశం->name = జ్ఞాపకం_ఇవ్వు(keysize + 1); /* allow for null */
	ఒకవేళ (fetch_దత్తాంశం->name == ఖాళీ) {
		విడుదల(fetch_దత్తాంశం);
		NSLOG(netsurf, INFO,
		      "keyname allocation failure for %s%s", name, ksfx);
		తిరిగి NSERROR_NOMEM;
	}
	snprintf(fetch_దత్తాంశం->name, keysize + 1, "%s%s", name, ksfx);

	/*విలువ*/
	fetch_దత్తాంశం->విలువ = జ్ఞాపకం_ఇవ్వు(FETCH_దత్తాంశం_INT_విలువ_కొలత);
	ఒకవేళ (fetch_దత్తాంశం->విలువ == ఖాళీ) {
		విడుదల(fetch_దత్తాంశం->name);
		విడుదల(fetch_దత్తాంశం);
		NSLOG(netsurf, INFO, "value allocation failure");
		తిరిగి NSERROR_NOMEM;
	}
	snprintf(fetch_దత్తాంశం->విలువ, FETCH_దత్తాంశం_INT_విలువ_కొలత, "%d", విలువ);

	/* link into list */
	**fetch_దత్తాంశం_తదుపరి_ptr = fetch_దత్తాంశం;
	*fetch_దత్తాంశం_తదుపరి_ptr = &fetch_దత్తాంశం->తదుపరి;

	తిరిగి NSERROR_OK;
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
స్థిర nserror
fetch_దత్తాంశం_list_కూడు(dom_string *name,
		    dom_string *విలువ,
		    స్థిరరాశి అక్షరం *rawfile,
		    స్థిరరాశి అక్షరం *form_charset,
		    స్థిరరాశి అక్షరం *docu_charset,
		    నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	నిర్మాణం fetch_multipart_దత్తాంశం *fetch_దత్తాంశం;

	ప్రతిజ్ఞా(name != ఖాళీ);

	fetch_దత్తాంశం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*fetch_దత్తాంశం));
	ఒకవేళ (fetch_దత్తాంశం == ఖాళీ) {
		NSLOG(netsurf, INFO, "failed allocation for fetch data");
		తిరిగి NSERROR_NOMEM;
	}

	fetch_దత్తాంశం->name = form_encode_అంశం(dom_string_దత్తాంశం(name),
					    dom_string_byte_నిడివి(name),
					    form_charset,
					    docu_charset);
	ఒకవేళ (fetch_దత్తాంశం->name == ఖాళీ) {
		NSLOG(netsurf, INFO, "Could not encode name for fetch data");
		విడుదల(fetch_దత్తాంశం);
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (విలువ == ఖాళీ) {
		fetch_దత్తాంశం->విలువ = strdup("");
	} లేకపోతే {
		fetch_దత్తాంశం->విలువ = form_encode_అంశం(dom_string_దత్తాంశం(విలువ),
						     dom_string_byte_నిడివి(విలువ),
						     form_charset,
						     docu_charset);
	}
	ఒకవేళ (fetch_దత్తాంశం->విలువ == ఖాళీ) {
		NSLOG(netsurf, INFO, "Could not encode value for fetch data");
		విడుదల(fetch_దత్తాంశం->name);
		విడుదల(fetch_దత్తాంశం);
		తిరిగి NSERROR_NOMEM;
	}

	/* deal with raw file name */
	ఒకవేళ (rawfile != ఖాళీ) {
		fetch_దత్తాంశం->file = true;
		fetch_దత్తాంశం->rawfile = strdup(rawfile);
		ఒకవేళ (fetch_దత్తాంశం->rawfile == ఖాళీ) {
			NSLOG(netsurf, INFO,
			      "Could not encode rawfile value for fetch data");
			విడుదల(fetch_దత్తాంశం->విలువ);
			విడుదల(fetch_దత్తాంశం->name);
			విడుదల(fetch_దత్తాంశం);
			తిరిగి NSERROR_NOMEM;
		}
	}

	/* link into list */
	**fetch_దత్తాంశం_తదుపరి_ptr = fetch_దత్తాంశం;
	*fetch_దత్తాంశం_తదుపరి_ptr = &fetch_దత్తాంశం->తదుపరి;

	తిరిగి NSERROR_OK;
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
స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_textarea(dom_html_పాఠ్యం_area_element *పాఠ్యం_area_element,
			  స్థిరరాశి అక్షరం *form_charset,
			  స్థిరరాశి అక్షరం *doc_charset,
			  నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	nserror res;

	/* check if element is disabled */
	ఘాతాంకం = dom_html_పాఠ్యం_area_element_get_disabled(పాఠ్యం_area_element,
						      &element_disabled);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area disabled property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		తిరిగి NSERROR_OK;
	}

	/* obtain name property */
	ఘాతాంకం = dom_html_పాఠ్యం_area_element_get_name(పాఠ్యం_area_element,
						  &inputname);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area name property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (inputname == ఖాళీ) {
		/* allow enumeration to continue after element with no name */
		తిరిగి NSERROR_OK;
	}

	/* obtain text area value */
	ఘాతాంకం = dom_html_పాఠ్యం_area_element_get_విలువ(పాఠ్యం_area_element,
						   &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get text area content. exp %d", ఘాతాంకం);
		dom_string_unref(inputname);
		తిరిగి NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(inputname,
				  inputvalue,
				  ఖాళీ,
				  form_charset,
				  doc_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(inputvalue);
	dom_string_unref(inputname);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_select_option(dom_html_option_element *option_element,
			      dom_string *keyname,
			      స్థిరరాశి అక్షరం *form_charset,
			      స్థిరరాశి అక్షరం *docu_charset,
			      నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	dom_string *విలువ;
	bool selected;

	ఘాతాంకం = dom_html_option_element_get_selected(option_element, &selected);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option selected property");
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (!selected) {
		/* unselected options do not add fetch data entries */
		తిరిగి NSERROR_OK;
	}

	ఘాతాంకం = dom_html_option_element_get_విలువ(option_element, &విలువ);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get option value");
		తిరిగి NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(keyname,
				  విలువ,
				  ఖాళీ,
				  form_charset,
				  docu_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(విలువ);

	తిరిగి res;
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
స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_select(dom_html_select_element *select_element,
			స్థిరరాశి అక్షరం *form_charset,
			స్థిరరాశి అక్షరం *doc_charset,
			నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res = NSERROR_OK;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_html_options_collection *options = ఖాళీ;
	uint32_t options_లెక్క;
	uint32_t option_సూచీ;
	dom_కణుపు *option_element = ఖాళీ;

	/* check if element is disabled */
	ఘాతాంకం = dom_html_select_element_get_disabled(select_element,
						   &element_disabled);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select disabled property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		తిరిగి NSERROR_OK;
	}

	/* obtain name property */
	ఘాతాంకం = dom_html_select_element_get_name(select_element, &inputname);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select name property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (inputname == ఖాళీ) {
		/* allow enumeration to continue after element with no name */
		తిరిగి NSERROR_OK;
	}

	/* get options collection */
	ఘాతాంకం = dom_html_select_element_get_options(select_element, &options);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection");
		dom_string_unref(inputname);
		తిరిగి NSERROR_DOM;
	}

	/* get options collection length */
	ఘాతాంకం = dom_html_options_collection_get_నిడివి(options, &options_లెక్క);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get select options collection length");
		dom_html_options_collection_unref(options);
		dom_string_unref(inputname);
		తిరిగి NSERROR_DOM;
	}

	/* iterate over options collection */
	కొరకు (option_సూచీ = 0; option_సూచీ < options_లెక్క; ++option_సూచీ) {
		ఘాతాంకం = dom_html_options_collection_అంశం(options,
						       option_సూచీ,
						       &option_element);
		ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "Could not get options item %"PRId32, option_సూచీ);
			res = NSERROR_DOM;
		} లేకపోతే {
			res = form_dom_గమ్యం_దత్తాంశం_select_option(
				(dom_html_option_element *)option_element,
				inputname,
				form_charset,
				doc_charset,
				fetch_దత్తాంశం_తదుపరి_ptr);

			dom_కణుపు_unref(option_element);
		}

		ఒకవేళ (res != NSERROR_OK) {
			ఆపు;
		}
	}

	dom_html_options_collection_unref(options);
	dom_string_unref(inputname);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం_submit(dom_html_ఆదానం_element *ఆదానం_element,
			      dom_string *inputname,
			      స్థిరరాశి అక్షరం *charset,
			      స్థిరరాశి అక్షరం *document_charset,
			      dom_html_element **submit_button,
			      నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	dom_string *inputvalue;
	nserror res;

	ఒకవేళ (*submit_button == ఖాళీ) {
		/* caller specified no button so use this one */
		*submit_button = (dom_html_element *)ఆదానం_element;
	} లేకపోతే ఒకవేళ (*submit_button != (dom_html_element *)ఆదానం_element) {
		తిరిగి NSERROR_OK;
	}

	/* matched button used to submit form */
	ఘాతాంకం = dom_html_ఆదానం_element_get_విలువ(ఆదానం_element, &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		తిరిగి NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(inputname,
				  inputvalue,
				  ఖాళీ,
				  charset,
				  document_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(inputvalue);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం_image(dom_html_ఆదానం_element *ఆదానం_element,
			     dom_string *inputname,
			     స్థిరరాశి అక్షరం *charset,
			     స్థిరరాశి అక్షరం *document_charset,
			     dom_html_element **submit_button,
			     నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	నిర్మాణం image_ఆదానం_coords *coords;
	అక్షరం *basename;

	/* Only use an image input if it was the thing which activated us */
	ఒకవేళ (*submit_button != (dom_html_element *)ఆదానం_element) {
		తిరిగి NSERROR_OK;
	}

	ఘాతాంకం = dom_కణుపు_get_user_దత్తాంశం((dom_కణుపు *)ఆదానం_element,
				     corestring_dom_ns_కీలకం_image_coords_కణుపు_దత్తాంశం,
				     &coords);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get image XY data");
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (coords == ఖాళీ) {
		NSLOG(netsurf, INFO, "No XY data on the image input");
		తిరిగి NSERROR_DOM;
	}

	/* encode input name once */
	basename = form_encode_అంశం(dom_string_దత్తాంశం(inputname),
				    dom_string_byte_నిడివి(inputname),
				    charset,
				    document_charset);
	ఒకవేళ (basename == ఖాళీ) {
		NSLOG(netsurf, INFO, "Could not encode basename");
		తిరిగి NSERROR_NOMEM;
	}

	res = fetch_దత్తాంశం_list_కూడు_sname(basename, ".x",
					coords->x,
					fetch_దత్తాంశం_తదుపరి_ptr);

	ఒకవేళ (res == NSERROR_OK) {
		res = fetch_దత్తాంశం_list_కూడు_sname(basename, ".y",
						coords->y,
						fetch_దత్తాంశం_తదుపరి_ptr);
	}

	విడుదల(basename);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం_checkbox(dom_html_ఆదానం_element *ఆదానం_element,
				dom_string *inputname,
				స్థిరరాశి అక్షరం *charset,
				స్థిరరాశి అక్షరం *document_charset,
				నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	bool checked;
	dom_string *inputvalue;

	ఘాతాంకం = dom_html_ఆదానం_element_get_checked(ఆదానం_element, &checked);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element checked");
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (!checked) {
		/* unchecked items do not generate a data entry */
		తిరిగి NSERROR_OK;
	}

	ఘాతాంకం = dom_html_ఆదానం_element_get_విలువ(ఆదానం_element, &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input element value");
		తిరిగి NSERROR_DOM;
	}

	/* ensure a default value */
	ఒకవేళ (inputvalue == ఖాళీ) {
		inputvalue = dom_string_ref(corestring_dom_on);
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(inputname,
				  inputvalue,
				  ఖాళీ,
				  charset,
				  document_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(inputvalue);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం_file(dom_html_ఆదానం_element *ఆదానం_element,
			    dom_string *inputname,
			    స్థిరరాశి అక్షరం *charset,
			    స్థిరరాశి అక్షరం *document_charset,
			    నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	dom_string *inputvalue;
	స్థిరరాశి అక్షరం *rawfile = ఖాళీ;

	ఘాతాంకం = dom_html_ఆదానం_element_get_విలువ(ఆదానం_element, &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file value");
		తిరిగి NSERROR_DOM;
	}

	ఘాతాంకం = dom_కణుపు_get_user_దత్తాంశం((dom_కణుపు *)ఆదానం_element,
				     corestring_dom_ns_కీలకం_file_name_కణుపు_దత్తాంశం,
				     &rawfile);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get file rawname");
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (rawfile == ఖాళీ) {
		rawfile = "";
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(inputname,
				  inputvalue,
				  rawfile,
				  charset,
				  document_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(inputvalue);

	తిరిగి res;
}


స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం_పాఠ్యం(dom_html_ఆదానం_element *ఆదానం_element,
			    dom_string *inputname,
			    స్థిరరాశి అక్షరం *charset,
			    స్థిరరాశి అక్షరం *document_charset,
			    నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	nserror res;
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	dom_string *inputvalue;

	ఘాతాంకం = dom_html_ఆదానం_element_get_విలువ(ఆదానం_element, &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input value");
		తిరిగి NSERROR_DOM;
	}

	/* add key/value pair to fetch data list */
	res = fetch_దత్తాంశం_list_కూడు(inputname,
				  inputvalue,
				  ఖాళీ,
				  charset,
				  document_charset,
				  fetch_దత్తాంశం_తదుపరి_ptr);

	dom_string_unref(inputvalue);

	తిరిగి res;
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
స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_ఆదానం(dom_html_ఆదానం_element *ఆదానం_element,
		       స్థిరరాశి అక్షరం *charset,
		       స్థిరరాశి అక్షరం *document_charset,
		       dom_html_element **submit_button,
		       నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputtype;
	nserror res;

	/* check if element is disabled */
	ఘాతాంకం = dom_html_ఆదానం_element_get_disabled(ఆదానం_element,
						  &element_disabled);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input disabled property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (element_disabled) {
		/* disabled element requires no more processing */
		తిరిగి NSERROR_OK;
	}

	/* obtain name property */
	ఘాతాంకం = dom_html_ఆదానం_element_get_name(ఆదానం_element, &inputname);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get input name property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (inputname == ఖాళీ) {
		/* element with no name is not converted */
		తిరిగి NSERROR_OK;
	}

	/* get input type */
	ఘాతాంకం = dom_html_ఆదానం_element_get_type(ఆదానం_element, &inputtype);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get input element type");
		dom_string_unref(inputname);
		తిరిగి NSERROR_DOM;
	}

	/* process according to input element type */
	ఒకవేళ (dom_string_caseless_isequal(inputtype, corestring_dom_submit)) {

		res = form_dom_గమ్యం_దత్తాంశం_ఆదానం_submit(ఆదానం_element,
						    inputname,
						    charset,
						    document_charset,
						    submit_button,
						    fetch_దత్తాంశం_తదుపరి_ptr);

	} లేకపోతే ఒకవేళ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_image)) {

		res = form_dom_గమ్యం_దత్తాంశం_ఆదానం_image(ఆదానం_element,
						   inputname,
						   charset,
						   document_charset,
						   submit_button,
						   fetch_దత్తాంశం_తదుపరి_ptr);

	} లేకపోతే ఒకవేళ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_radio) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_checkbox)) {

		res = form_dom_గమ్యం_దత్తాంశం_ఆదానం_checkbox(ఆదానం_element,
						      inputname,
						      charset,
						      document_charset,
						      fetch_దత్తాంశం_తదుపరి_ptr);

	} లేకపోతే ఒకవేళ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_file)) {

		res = form_dom_గమ్యం_దత్తాంశం_ఆదానం_file(ఆదానం_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_దత్తాంశం_తదుపరి_ptr);

	} లేకపోతే ఒకవేళ (dom_string_caseless_isequal(inputtype,
					       corestring_dom_reset) ||
		   dom_string_caseless_isequal(inputtype,
					       corestring_dom_button)) {
		/* Skip these */
		NSLOG(netsurf, INFO, "Skipping RESET and BUTTON");
		res = NSERROR_OK;

	} లేకపోతే {
		/* Everything else is treated as text values */
		res = form_dom_గమ్యం_దత్తాంశం_ఆదానం_పాఠ్యం(ఆదానం_element,
						  inputname,
						  charset,
						  document_charset,
						  fetch_దత్తాంశం_తదుపరి_ptr);

	}

	dom_string_unref(inputtype);
	dom_string_unref(inputname);

	తిరిగి res;
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
స్థిర nserror
form_dom_గమ్యం_దత్తాంశం_button(dom_html_button_element *button_element,
			స్థిరరాశి అక్షరం *form_charset,
			స్థిరరాశి అక్షరం *doc_charset,
			dom_html_element **submit_button,
			నిర్మాణం fetch_multipart_దత్తాంశం ***fetch_దత్తాంశం_తదుపరి_ptr)
{
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	bool element_disabled;
	dom_string *inputname;
	dom_string *inputvalue;
	dom_string *inputtype;
	nserror res = NSERROR_OK;

	/* check if element is disabled */
	ఘాతాంకం = dom_html_button_element_get_disabled(button_element,
						   &element_disabled);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Unable to get disabled property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (element_disabled) {
		/* allow enumeration to continue after disabled element */
		తిరిగి NSERROR_OK;
	}

	/* get the type attribute */
	ఘాతాంకం = dom_html_button_element_get_type(button_element, &inputtype);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get button element type");
		తిరిగి NSERROR_DOM;
	}

	/* If the type attribute is "reset" or "button" the element is
	 *  barred from constraint validation. Specification says
	 *  default and invalid values result in submit which will
	 *  be considered.
	 */
	ఒకవేళ (dom_string_caseless_isequal(inputtype, corestring_dom_reset)) {
		/* multipart data entry not required for reset type */
		dom_string_unref(inputtype);
		తిరిగి NSERROR_OK;
	}
	ఒకవేళ (dom_string_caseless_isequal(inputtype, corestring_dom_button)) {
		/* multipart data entry not required for button type */
		dom_string_unref(inputtype);
		తిరిగి NSERROR_OK;
	}
	dom_string_unref(inputtype);

	/* only submision button generates an element */
	ఒకవేళ (*submit_button == ఖాళీ) {
		/* no submission button selected yet so use this one */
		*submit_button = (dom_html_element *)button_element;
	}
	ఒకవేళ (*submit_button != (dom_html_element *)button_element) {
		తిరిగి NSERROR_OK;
	}

	/* obtain name property */
	ఘాతాంకం = dom_html_button_element_get_name(button_element, &inputname);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO,
		      "Could not get button name property. exp %d", ఘాతాంకం);
		తిరిగి NSERROR_DOM;
	}

	ఒకవేళ (inputname == ఖాళీ) {
		/* allow enumeration to continue after element with no name */
		తిరిగి NSERROR_OK;
	}

	/* get button value and add to fetch data list */
	ఘాతాంకం = dom_html_button_element_get_విలువ(button_element, &inputvalue);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get submit button value");
		res = NSERROR_DOM;
	} లేకపోతే {
		res = fetch_దత్తాంశం_list_కూడు(inputname,
					  inputvalue,
					  ఖాళీ,
					  form_charset,
					  doc_charset,
					  fetch_దత్తాంశం_తదుపరి_ptr);

		dom_string_unref(inputvalue);
	}

	dom_string_unref(inputname);

	తిరిగి res;
}


/**
 * Find an acceptable character set encoding with which to submit the form
 *
 * \param form  The form
 * \return Pointer to charset name (on heap, caller should free) or NULL
 */
స్థిర అక్షరం *form_acceptable_charset(నిర్మాణం form *form)
{
	అక్షరం *తాత్కాలిక, *c;

	ఒకవేళ (!form->accept_charsets) {
		/* no accept-charsets attribute for this form */
		ఒకవేళ (form->document_charset) {
			/* document charset present, so use it */
			తిరిగి strdup(form->document_charset);
		} లేకపోతే {
			/* no document charset, so default to 8859-1 */
			తిరిగి strdup("ISO-8859-1");
		}
	}

	/* make temporary copy of accept-charsets attribute */
	తాత్కాలిక = strdup(form->accept_charsets);
	ఒకవేళ (!తాత్కాలిక)
		తిరిగి ఖాళీ;

	/* make it upper case */
	కొరకు (c = తాత్కాలిక; *c; c++) {
		*c = ascii_గమ్యం_పై(*c);
	}

	/* is UTF-8 specified? */
	c = అక్షరశ్రేణి_శోధన(తాత్కాలిక, "UTF-8");
	ఒకవేళ (c) {
		విడుదల(తాత్కాలిక);
		తిరిగి strdup("UTF-8");
	}

	/* dispense with temporary copy */
	విడుదల(తాత్కాలిక);

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
	అయితే (*c && !ascii_is_space(*c)) {
		ఒకవేళ (*c == ',')
			ఆపు;
		c++;
	}

	తిరిగి strndup(form->accept_charsets, c - form->accept_charsets);
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
స్థిర nserror
form_dom_గమ్యం_దత్తాంశం(నిర్మాణం form *form,
		 నిర్మాణం form_control *submit_control,
		 నిర్మాణం fetch_multipart_దత్తాంశం **fetch_దత్తాంశం_బహిర్గతం)
{
	nserror res = NSERROR_OK;
	అక్షరం *charset; /* form characterset */
	dom_exception ఘాతాంకం; /* the result from DOM operations */
	dom_html_collection *elements = ఖాళీ; /* the dom form elements */
	uint32_t element_లెక్క; /* the number of elements in the DOM form */
	uint32_t element_idx; /* the index of thr enumerated element */
	dom_కణుపు *element = ఖాళీ; /* the DOM form element */
	dom_string *nodename = ఖాళీ; /* the DOM node name of the element */
	నిర్మాణం fetch_multipart_దత్తాంశం *fetch_దత్తాంశం = ఖాళీ; /* fetch data list */
	నిర్మాణం fetch_multipart_దత్తాంశం **fetch_దత్తాంశం_తదుపరి = &fetch_దత్తాంశం;
	dom_html_element *submit_button;

	/* obtain the submit_button DOM node from the control */
	ఒకవేళ (submit_control != ఖాళీ) {
		submit_button = submit_control->కణుపు;
	} లేకపోతే {
		submit_button = ఖాళీ;
	}

	/** \todo Replace this call with something DOMish */
	charset = form_acceptable_charset(form);
	ఒకవేళ (charset == ఖాళీ) {
		NSLOG(netsurf, INFO, "failed to find charset");
		తిరిగి NSERROR_NOMEM;
	}

	/* obtain the form elements and count */
	ఘాతాంకం = dom_html_form_element_get_elements(form->కణుపు, &elements);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form elements");
		విడుదల(charset);
		తిరిగి NSERROR_DOM;
	}

	ఘాతాంకం = dom_html_collection_get_నిడివి(elements, &element_లెక్క);
	ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
		NSLOG(netsurf, INFO, "Could not get form element count");
		res = NSERROR_DOM;
		వెళ్ళు form_dom_గమ్యం_దత్తాంశం_error;
	}

	కొరకు (element_idx = 0; element_idx < element_లెక్క; element_idx++) {
		/* obtain a form element */
		ఘాతాంకం = dom_html_collection_అంశం(elements, element_idx, &element);
		ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "retrieving form element %"PRIu32" failed with %d",
			      element_idx, ఘాతాంకం);
			res = NSERROR_DOM;
			వెళ్ళు form_dom_గమ్యం_దత్తాంశం_error;
		}

		/* node name from element */
		ఘాతాంకం = dom_కణుపు_get_కణుపు_name(element, &nodename);
		ఒకవేళ (ఘాతాంకం != DOM_NO_ERR) {
			NSLOG(netsurf, INFO,
			      "getting element node name %"PRIu32" failed with %d",
			      element_idx, ఘాతాంకం);
			dom_కణుపు_unref(element);
			res = NSERROR_DOM;
			వెళ్ళు form_dom_గమ్యం_దత్తాంశం_error;
		}

		ఒకవేళ (dom_string_isequal(nodename, corestring_dom_TEXTAREA)) {
			/* Form element is HTMLTextAreaElement */
			res = form_dom_గమ్యం_దత్తాంశం_textarea(
				(dom_html_పాఠ్యం_area_element *)element,
				charset,
				form->document_charset,
				&fetch_దత్తాంశం_తదుపరి);

		} లేకపోతే ఒకవేళ (dom_string_isequal(nodename, corestring_dom_SELECT)) {
			/* Form element is HTMLSelectElement */
			res = form_dom_గమ్యం_దత్తాంశం_select(
				(dom_html_select_element *)element,
				charset,
				form->document_charset,
				&fetch_దత్తాంశం_తదుపరి);

		} లేకపోతే ఒకవేళ (dom_string_isequal(nodename, corestring_dom_ఆదానం)) {
			/* Form element is HTMLInputElement */
			res = form_dom_గమ్యం_దత్తాంశం_ఆదానం(
				(dom_html_ఆదానం_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_దత్తాంశం_తదుపరి);

		} లేకపోతే ఒకవేళ (dom_string_isequal(nodename, corestring_dom_BUTTON)) {
			/* Form element is HTMLButtonElement */
			res = form_dom_గమ్యం_దత్తాంశం_button(
				(dom_html_button_element *)element,
				charset,
				form->document_charset,
				&submit_button,
				&fetch_దత్తాంశం_తదుపరి);

		} లేకపోతే {
			/* Form element is not handled */
			NSLOG(netsurf, INFO,
			      "Unhandled element type: %*s",
			      (పూర్ణాంకం)dom_string_byte_నిడివి(nodename),
			      dom_string_దత్తాంశం(nodename));
			res = NSERROR_DOM;

		}

		dom_string_unref(nodename);
		dom_కణుపు_unref(element);

		/* abort form element enumeration on error */
		ఒకవేళ (res != NSERROR_OK) {
			వెళ్ళు form_dom_గమ్యం_దత్తాంశం_error;
		}
	}

	*fetch_దత్తాంశం_బహిర్గతం = fetch_దత్తాంశం;
	dom_html_collection_unref(elements);
	విడుదల(charset);

	తిరిగి NSERROR_OK;

form_dom_గమ్యం_దత్తాంశం_error:
	fetch_multipart_దత్తాంశం_destroy(fetch_దత్తాంశం);
	dom_html_collection_unref(elements);
	విడుదల(charset);

	తిరిగి res;
}

/**
 * Encode controls using application/x-www-form-urlencoded.
 *
 * \param[in] form form to which successful controls relate
 * \param[in] control linked list of fetch_multipart_data
 * \param[out] encoded_out URL-encoded form data
 * \return NSERROR_OK on success and \a encoded_out updated else appropriate error code
 */
స్థిర nserror
form_url_encode(నిర్మాణం form *form,
		నిర్మాణం fetch_multipart_దత్తాంశం *control,
		అక్షరం **encoded_బహిర్గతం)
{
	అక్షరం *name, *విలువ;
	అక్షరం *s, *s2;
	గుర్తులేని పూర్ణాంకం len, len1, len_init;
	nserror res;

	s = జ్ఞాపకం_ఇవ్వు(1);

	ఒకవేళ (s == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	s[0] = '\0';
	len_init = len = 0;

	కొరకు (; control; control = control->తదుపరి) {
		res = url_escape(control->name, true, ఖాళీ, &name);
		ఒకవేళ (res != NSERROR_OK) {
			విడుదల(s);
			తిరిగి res;
		}

		res = url_escape(control->విలువ, true, ఖాళీ, &విలువ);
		ఒకవేళ (res != NSERROR_OK) {
			విడుదల(name);
			విడుదల(s);
			తిరిగి res;
		}

		/* resize string to allow for new key/value pair,
		 *  equals, amphersand and terminator
		 */
		len1 = len + అక్షరశ్రేణి_పొడవు(name) + అక్షరశ్రేణి_పొడవు(విలువ) + 2;
		s2 = realloc(s, len1 + 1);
		ఒకవేళ (s2 == ఖాళీ) {
			విడుదల(విలువ);
			విడుదల(name);
			విడుదల(s);
			తిరిగి NSERROR_NOMEM;
		}
		s = s2;

		snprintf(s + len, (len1 + 1) - len, "%s=%s&", name, విలువ);
		len = len1;
		విడుదల(name);
		విడుదల(విలువ);
	}

	ఒకవేళ (len > len_init) {
		/* Replace trailing '&' */
		s[len - 1] = '\0';
	}

	*encoded_బహిర్గతం = s;

	తిరిగి NSERROR_OK;
}


/**
 * Callback for the select menus scroll
 */
స్థిర శూన్య
form_select_menu_scroll_callback(శూన్య *client_దత్తాంశం,
				 నిర్మాణం scrollbar_సందేశం_దత్తాంశం *scrollbar_దత్తాంశం)
{
	నిర్మాణం form_control *control = client_దత్తాంశం;
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;
	html_content *html = (html_content *)menu->c;

	ఎంపిక (scrollbar_దత్తాంశం->సందేశం) {
		స్థితి SCROLLBAR_సందేశం_MOVED:
			menu->callback(menu->client_దత్తాంశం,
					0, 0,
					menu->వెడల్పు,
					menu->ఎత్తు);
			ఆపు;
		స్థితి SCROLLBAR_సందేశం_SCROLL_ప్రారంభం:
		{
			నిర్మాణం rect rect = {
				.x0 = scrollbar_దత్తాంశం->x0,
				.y0 = scrollbar_దత్తాంశం->y0,
				.x1 = scrollbar_దత్తాంశం->x1,
				.y1 = scrollbar_దత్తాంశం->y1
			};

			browser_కిటికీ_సమితి_drag_type(html->bw,
					DRAGGING_CONTENT_SCROLLBAR, &rect);

			menu->scroll_capture = true;
		}
			ఆపు;
		స్థితి SCROLLBAR_సందేశం_SCROLL_ముగిసిన:
			menu->scroll_capture = false;

			browser_కిటికీ_సమితి_drag_type(html->bw,
					DRAGGING_NONE, ఖాళీ);
			ఆపు;
		అప్రమేయ:
			ఆపు;
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
స్థిర nserror
form__select_process_selection(html_content *html,
			       నిర్మాణం form_control *control,
			       పూర్ణాంకం అంశం)
{
	నిర్మాణం box *inline_box;
	నిర్మాణం form_option *o;
	పూర్ణాంకం లెక్క;
	nserror ret = NSERROR_OK;

	ప్రతిజ్ఞా(control != ఖాళీ);
	ప్రతిజ్ఞా(html != ఖాళీ);

	/**
	 * \todo Even though the form code is effectively part of the html
	 *        content handler, poking around inside contents is not good
	 */

	inline_box = control->box->children->children;

	కొరకు (లెక్క = 0, o = control->దత్తాంశం.select.వస్తువులు;
			o != ఖాళీ;
			లెక్క++, o = o->తదుపరి) {
		ఒకవేళ (!control->దత్తాంశం.select.గుణిజం && o->selected) {
			o->selected = false;
			dom_html_option_element_సమితి_selected(o->కణుపు, false);
		}

		ఒకవేళ (లెక్క == అంశం) {
			ఒకవేళ (control->దత్తాంశం.select.గుణిజం) {
				ఒకవేళ (o->selected) {
					o->selected = false;
					dom_html_option_element_సమితి_selected(
							o->కణుపు, false);
					control->దత్తాంశం.select.అంకె_selected--;
				} లేకపోతే {
					o->selected = true;
					dom_html_option_element_సమితి_selected(
							o->కణుపు, true);
					control->దత్తాంశం.select.అంకె_selected++;
				}
			} లేకపోతే {
				dom_html_option_element_సమితి_selected(
						o->కణుపు, true);
				o->selected = true;
			}
		}

		ఒకవేళ (o->selected) {
			control->దత్తాంశం.select.current = o;
		}
	}

	talloc_free(inline_box->పాఠ్యం);
	inline_box->పాఠ్యం = 0;

	ఒకవేళ (control->దత్తాంశం.select.అంకె_selected == 0) {
		inline_box->పాఠ్యం = talloc_strdup(html->bctx,
				messages_get("Form_None"));
	} లేకపోతే ఒకవేళ (control->దత్తాంశం.select.అంకె_selected == 1) {
		inline_box->పాఠ్యం = talloc_strdup(html->bctx,
				control->దత్తాంశం.select.current->పాఠ్యం);
	} లేకపోతే {
		inline_box->పాఠ్యం = talloc_strdup(html->bctx,
				messages_get("Form_Many"));
	}

	ఒకవేళ (!inline_box->పాఠ్యం) {
		ret = NSERROR_NOMEM;
		inline_box->నిడివి = 0;
	} లేకపోతే {
		inline_box->నిడివి = అక్షరశ్రేణి_పొడవు(inline_box->పాఠ్యం);
	}
	inline_box->వెడల్పు = control->box->వెడల్పు;

	html__redraw_a_box(html, control->box);

	తిరిగి ret;
}


/**
 * Handle a click on the area of the currently opened select menu.
 *
 * \param control the select menu which received the click
 * \param x X coordinate of click
 * \param y Y coordinate of click
 */
స్థిర శూన్య form_select_menu_clicked(నిర్మాణం form_control *control, పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;
	నిర్మాణం form_option *option;
	html_content *html = (html_content *)menu->c;
	పూర్ణాంకం పంక్తి_ఎత్తు, పంక్తి_ఎత్తు_with_spacing;
	పూర్ణాంకం అంశం_bottom_y;
	పూర్ణాంకం scroll, i;

	scroll = scrollbar_get_స్థానభ్రంశం(menu->scrollbar);

	పంక్తి_ఎత్తు = menu->పంక్తి_ఎత్తు;
	పంక్తి_ఎత్తు_with_spacing = పంక్తి_ఎత్తు +
			పంక్తి_ఎత్తు * SELECT_పంక్తి_SPACING;

	option = control->దత్తాంశం.select.వస్తువులు;
	అంశం_bottom_y = పంక్తి_ఎత్తు_with_spacing;
	i = 0;
	అయితే (option && అంశం_bottom_y < scroll + y) {
		అంశం_bottom_y += పంక్తి_ఎత్తు_with_spacing;
		option = option->తదుపరి;
		i++;
	}

	ఒకవేళ (option != ఖాళీ) {
		form__select_process_selection(html, control, i);
	}

	menu->callback(menu->client_దత్తాంశం, 0, 0, menu->వెడల్పు, menu->ఎత్తు);
}


/* exported interface documented in html/form_internal.h */
శూన్య form_కూడు_control(నిర్మాణం form *form, నిర్మాణం form_control *control)
{
	ఒకవేళ (form == ఖాళీ) {
		తిరిగి;
	}

	control->form = form;

	ఒకవేళ (form->controls != ఖాళీ) {
		ప్రతిజ్ఞా(form->చివరి_control);

		form->చివరి_control->తదుపరి = control;
		control->prev = form->చివరి_control;
		control->తదుపరి = ఖాళీ;
		form->చివరి_control = control;
	} లేకపోతే {
		form->controls = form->చివరి_control = control;
	}
}


/* exported interface documented in html/form_internal.h */
శూన్య form_free_control(నిర్మాణం form_control *control)
{
	నిర్మాణం form_control *c;
	ప్రతిజ్ఞా(control != ఖాళీ);

	NSLOG(netsurf, INFO, "Control:%p name:%p value:%p initial:%p",
	      control, control->name, control->విలువ, control->initial_విలువ);
	విడుదల(control->name);
	విడుదల(control->విలువ);
	విడుదల(control->initial_విలువ);
	ఒకవేళ (control->చివరి_synced_విలువ != ఖాళీ) {
		విడుదల(control->చివరి_synced_విలువ);
	}

	ఒకవేళ (control->type == GADGET_SELECT) {
		నిర్మాణం form_option *option, *తదుపరి;

		కొరకు (option = control->దత్తాంశం.select.వస్తువులు; option;
				option = తదుపరి) {
			తదుపరి = option->తదుపరి;
			NSLOG(netsurf, INFO,
			      "select option:%p text:%p value:%p", option,
			      option->పాఠ్యం, option->విలువ);
			విడుదల(option->పాఠ్యం);
			విడుదల(option->విలువ);
			విడుదల(option);
		}
		ఒకవేళ (control->దత్తాంశం.select.menu != ఖాళీ) {
			form_free_select_menu(control);
		}
	}

	ఒకవేళ (control->type == GADGET_TEXTAREA ||
			control->type == GADGET_TEXTBOX ||
			control->type == GADGET_PASSWORD) {

		ఒకవేళ (control->దత్తాంశం.పాఠ్యం.initial != ఖాళీ) {
			dom_string_unref(control->దత్తాంశం.పాఠ్యం.initial);
		}

		ఒకవేళ (control->దత్తాంశం.పాఠ్యం.ta != ఖాళీ) {
			textarea_destroy(control->దత్తాంశం.పాఠ్యం.ta);
		}
	}

	/* unlink the control from the form */
	ఒకవేళ (control->form != ఖాళీ) {
		కొరకు (c = control->form->controls; c != ఖాళీ; c = c->తదుపరి) {
			ఒకవేళ (c->తదుపరి == control) {
				c->తదుపరి = control->తదుపరి;
				ఒకవేళ (control->form->చివరి_control == control)
					control->form->చివరి_control = c;
				ఆపు;
			}
			ఒకవేళ (c == control) {
				/* can only happen if control was first control */
				control->form->controls = control->తదుపరి;
				ఒకవేళ (control->form->చివరి_control == control)
					control->form->controls =
						control->form->చివరి_control = ఖాళీ;
				ఆపు;
			}
		}
	}

	ఒకవేళ (control->కణుపు_విలువ != ఖాళీ) {
		dom_string_unref(control->కణుపు_విలువ);
	}

	విడుదల(control);
}


/* exported interface documented in html/form_internal.h */
bool form_కూడు_option(నిర్మాణం form_control *control, అక్షరం *విలువ, అక్షరం *పాఠ్యం,
		     bool selected, శూన్య *కణుపు)
{
	నిర్మాణం form_option *option;

	ప్రతిజ్ఞా(control);
	ప్రతిజ్ఞా(control->type == GADGET_SELECT);

	option = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం *option);
	ఒకవేళ (!option)
		తిరిగి false;

	option->విలువ = విలువ;
	option->పాఠ్యం = పాఠ్యం;

	/* add to linked list */
	ఒకవేళ (control->దత్తాంశం.select.వస్తువులు == 0)
		control->దత్తాంశం.select.వస్తువులు = option;
	లేకపోతే
		control->దత్తాంశం.select.చివరి_అంశం->తదుపరి = option;
	control->దత్తాంశం.select.చివరి_అంశం = option;

	/* set selected */
	ఒకవేళ (selected && (control->దత్తాంశం.select.అంకె_selected == 0 ||
			control->దత్తాంశం.select.గుణిజం)) {
		option->selected = option->initial_selected = true;
		control->దత్తాంశం.select.అంకె_selected++;
		control->దత్తాంశం.select.current = option;
	}

	control->దత్తాంశం.select.అంకె_వస్తువులు++;

	option->కణుపు = కణుపు;

	తిరిగి true;
}


/* exported interface documented in html/form_internal.h */
nserror
form_open_select_menu(శూన్య *client_దత్తాంశం,
		      నిర్మాణం form_control *control,
		      select_menu_redraw_callback callback,
		      నిర్మాణం content *c)
{
	పూర్ణాంకం పంక్తి_ఎత్తు_with_spacing;
	నిర్మాణం box *box;
	plot_font_style_t fstyle;
	పూర్ణాంకం మొత్తం_ఎత్తు;
	నిర్మాణం form_select_menu *menu;
	html_content *html = (html_content *)c;
	nserror res;

	/* if the menu is opened for the first time */
	ఒకవేళ (control->దత్తాంశం.select.menu == ఖాళీ) {

		menu = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం (నిర్మాణం form_select_menu));
		ఒకవేళ (menu == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}

		control->దత్తాంశం.select.menu = menu;

		box = control->box;

		menu->వెడల్పు = box->వెడల్పు +
			box->border[కుడి_2].వెడల్పు + box->padding[కుడి_2] +
			box->border[ఎడమ_2].వెడల్పు + box->padding[ఎడమ_2];

		font_plot_style_from_css(&html->unit_len_ctx,
				control->box->style, &fstyle);
		menu->f_కొలత = fstyle.కొలత;

		menu->పంక్తి_ఎత్తు = FIXTOINT(FDIV((FMUL(FLTTOFIX(1.2),
				FMUL(html->unit_len_ctx.device_dpi,
				INTTOFIX(fstyle.కొలత / PLOT_STYLE_SCALE)))),
				F_72));

		పంక్తి_ఎత్తు_with_spacing = menu->పంక్తి_ఎత్తు +
				menu->పంక్తి_ఎత్తు *
				SELECT_పంక్తి_SPACING;

		మొత్తం_ఎత్తు = control->దత్తాంశం.select.అంకె_వస్తువులు *
				పంక్తి_ఎత్తు_with_spacing;
		menu->ఎత్తు = మొత్తం_ఎత్తు;

		ఒకవేళ (menu->ఎత్తు > MAX_SELECT_ఎత్తు) {
			menu->ఎత్తు = MAX_SELECT_ఎత్తు;
		}

		menu->client_దత్తాంశం = client_దత్తాంశం;
		menu->callback = callback;
		res = scrollbar_create(false,
				       menu->ఎత్తు,
				       మొత్తం_ఎత్తు,
				       menu->ఎత్తు,
				       control,
				       form_select_menu_scroll_callback,
				       &(menu->scrollbar));
		ఒకవేళ (res != NSERROR_OK) {
			control->దత్తాంశం.select.menu = ఖాళీ;
			విడుదల(menu);
			తిరిగి res;
		}
		menu->c = c;
	} లేకపోతే {
		menu = control->దత్తాంశం.select.menu;
	}

	menu->callback(client_దత్తాంశం, 0, 0, menu->వెడల్పు, menu->ఎత్తు);

	తిరిగి NSERROR_OK;
}


/* exported interface documented in html/form_internal.h */
శూన్య form_free_select_menu(నిర్మాణం form_control *control)
{
	ఒకవేళ (control->దత్తాంశం.select.menu->scrollbar != ఖాళీ)
		scrollbar_destroy(control->దత్తాంశం.select.menu->scrollbar);
	విడుదల(control->దత్తాంశం.select.menu);
	control->దత్తాంశం.select.menu = ఖాళీ;
}


/* exported interface documented in html/form_internal.h */
bool
form_redraw_select_menu(నిర్మాణం form_control *control,
			పూర్ణాంకం x, పూర్ణాంకం y,
			దశాంశం scale,
			స్థిరరాశి నిర్మాణం rect *clip,
			స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం box *box;
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;
	నిర్మాణం form_option *option;
	పూర్ణాంకం పంక్తి_ఎత్తు, పంక్తి_ఎత్తు_with_spacing;
	పూర్ణాంకం వెడల్పు, ఎత్తు;
	పూర్ణాంకం x0, y0, x1, scrollbar_x, y1, y2, y3;
	పూర్ణాంకం అంశం_y;
	పూర్ణాంకం పాఠ్యం_pos_స్థానభ్రంశం, పాఠ్యం_x;
	పూర్ణాంకం scrollbar_వెడల్పు = SCROLLBAR_వెడల్పు;
	పూర్ణాంకం i;
	పూర్ణాంకం scroll;
	పూర్ణాంకం x_cp, y_cp;
	నిర్మాణం rect r;
	నిర్మాణం rect rect;
	nserror res;

	box = control->box;

	x_cp = x;
	y_cp = y;
	వెడల్పు = menu->వెడల్పు;
	ఎత్తు = menu->ఎత్తు;
	పంక్తి_ఎత్తు = menu->పంక్తి_ఎత్తు;

	పంక్తి_ఎత్తు_with_spacing = పంక్తి_ఎత్తు +
			పంక్తి_ఎత్తు * SELECT_పంక్తి_SPACING;
	scroll = scrollbar_get_స్థానభ్రంశం(menu->scrollbar);

	ఒకవేళ (scale != 1.0) {
		x *= scale;
		y *= scale;
		వెడల్పు *= scale;
		ఎత్తు *= scale;
		scrollbar_వెడల్పు *= scale;

		i = scroll / పంక్తి_ఎత్తు_with_spacing;
		scroll -= i * పంక్తి_ఎత్తు_with_spacing;
		పంక్తి_ఎత్తు *= scale;
		పంక్తి_ఎత్తు_with_spacing *= scale;
		scroll *= scale;
		scroll += i * పంక్తి_ఎత్తు_with_spacing;
	}


	x0 = x;
	y0 = y;
	x1 = x + వెడల్పు - 1;
	y1 = y + ఎత్తు - 1;
	scrollbar_x = x1 - scrollbar_వెడల్పు;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	rect.x0 = x0;
	rect.y0 = y0;
	rect.x1 = x1;
	rect.y1 = y1;
	res = ctx->plot->rectangle(ctx, plot_style_stroke_darkwbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	x0 = x0 + SELECT_BORDER_వెడల్పు;
	y0 = y0 + SELECT_BORDER_వెడల్పు;
	x1 = x1 - SELECT_BORDER_వెడల్పు;
	y1 = y1 - SELECT_BORDER_వెడల్పు;
	ఎత్తు = ఎత్తు - 2 * SELECT_BORDER_వెడల్పు;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1 + 1;
	r.y1 = y1 + 1;
	res = ctx->plot->clip(ctx, &r);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	res = ctx->plot->rectangle(ctx, plot_style_fill_lightwbasec, &r);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	option = control->దత్తాంశం.select.వస్తువులు;
	అంశం_y = పంక్తి_ఎత్తు_with_spacing;

	అయితే (అంశం_y < scroll) {
		option = option->తదుపరి;
		అంశం_y += పంక్తి_ఎత్తు_with_spacing;
	}
	అంశం_y -= పంక్తి_ఎత్తు_with_spacing;
	పాఠ్యం_pos_స్థానభ్రంశం = y - scroll +
			(పూర్ణాంకం) (పంక్తి_ఎత్తు * (0.75 + SELECT_పంక్తి_SPACING));
	పాఠ్యం_x = x + (box->border[ఎడమ_2].వెడల్పు + box->padding[ఎడమ_2]) * scale;

	plot_fstyle_నమోదు.కొలత = menu->f_కొలత;

	అయితే (option && అంశం_y - scroll < ఎత్తు) {

		ఒకవేళ (option->selected) {
			y2 = y + అంశం_y - scroll;
			y3 = y + అంశం_y + పంక్తి_ఎత్తు_with_spacing - scroll;

			rect.x0 = x0;
			rect.y0 = y0 > y2 ? y0 : y2;
			rect.x1 = scrollbar_x + 1;
			rect.y1 = y3 < y1 + 1 ? y3 : y1 + 1;
			res = ctx->plot->rectangle(ctx, &plot_style_fill_selected, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
		}

		y2 = పాఠ్యం_pos_స్థానభ్రంశం + అంశం_y;
		res = ctx->plot->పాఠ్యం(ctx,
				      &plot_fstyle_నమోదు,
				      పాఠ్యం_x, y2,
				      option->పాఠ్యం, అక్షరశ్రేణి_పొడవు(option->పాఠ్యం));
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}

		అంశం_y += పంక్తి_ఎత్తు_with_spacing;
		option = option->తదుపరి;
	}

	res = scrollbar_redraw(menu->scrollbar,
			       x_cp + menu->వెడల్పు - SCROLLBAR_వెడల్పు,
			       y_cp,
			       clip, scale, ctx);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	తిరిగి true;
}


/* private interface described in html/form_internal.h */
bool
form_clip_inside_select_menu(నిర్మాణం form_control *control,
			     దశాంశం scale,
			     స్థిరరాశి నిర్మాణం rect *clip)
{
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;
	పూర్ణాంకం వెడల్పు, ఎత్తు;


	వెడల్పు = menu->వెడల్పు;
	ఎత్తు = menu->ఎత్తు;

	ఒకవేళ (scale != 1.0) {
		వెడల్పు *= scale;
		ఎత్తు *= scale;
	}

	ఒకవేళ (clip->x0 >= 0 &&
	    clip->x1 <= వెడల్పు &&
	    clip->y0 >= 0 &&
	    clip->y1 <= ఎత్తు)
		తిరిగి true;

	తిరిగి false;
}


/* exported interface documented in netsurf/form.h */
nserror form_select_process_selection(నిర్మాణం form_control *control, పూర్ణాంకం అంశం)
{
	ప్రతిజ్ఞా(control != ఖాళీ);

	తిరిగి form__select_process_selection(control->html, control, అంశం);
}


/* exported interface documented in netsurf/form.h */
నిర్మాణం form_option *
form_select_get_option(నిర్మాణం form_control *control, పూర్ణాంకం అంశం)
{
	నిర్మాణం form_option *opt;

	opt = control->దత్తాంశం.select.వస్తువులు;
	అయితే ((opt != ఖాళీ) && (అంశం > 0)) {
		opt = opt->తదుపరి;
		అంశం--;
	}
	తిరిగి opt;
}


/* exported interface documented in netsurf/form.h */
అక్షరం *form_control_get_name(నిర్మాణం form_control *control)
{
	తిరిగి control->name;
}


/* exported interface documented in netsurf/form.h */
nserror form_control_bounding_rect(నిర్మాణం form_control *control, నిర్మాణం rect *r)
{
	box_bounds( control->box, r );
	తిరిగి NSERROR_OK;
}


/* private interface described in html/form_internal.h */
స్థిరరాశి అక్షరం *
form_select_mouse_action(నిర్మాణం form_control *control,
			 browser_mouse_state mouse,
			 పూర్ణాంకం x, పూర్ణాంకం y)
{
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;
	పూర్ణాంకం x0, y0, x1, y1, scrollbar_x;
	స్థిరరాశి అక్షరం *స్థితి_2 = ఖాళీ;
	bool గుణిజం = control->దత్తాంశం.select.గుణిజం;

	x0 = 0;
	y0 = 0;
	x1 = menu->వెడల్పు;
	y1 = menu->ఎత్తు;
	scrollbar_x = x1 - SCROLLBAR_వెడల్పు;

	ఒకవేళ (menu->scroll_capture ||
			(x > scrollbar_x && x < x1 && y > y0 && y < y1)) {
		/* The scroll is currently capturing all events or the mouse
		 * event is taking place on the scrollbar widget area
		 */
		x -= scrollbar_x;
		తిరిగి scrollbar_mouse_స్థితి_గమ్యం_సందేశం(
				scrollbar_mouse_action(menu->scrollbar,
						mouse, x, y));
	}


	ఒకవేళ (x > x0 && x < scrollbar_x && y > y0 && y < y1) {
		/* over option area */

		ఒకవేళ (mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2))
			/* button 1 or 2 click */
			form_select_menu_clicked(control, x, y);

		ఒకవేళ (!(mouse & BROWSER_MOUSE_CLICK_1 && !గుణిజం))
			/* anything but a button 1 click over a single select
			   menu */
			స్థితి_2 = messages_get(control->దత్తాంశం.select.గుణిజం ?
					"SelectMClick" : "SelectClick");

	} లేకపోతే ఒకవేళ (!(mouse & (BROWSER_MOUSE_CLICK_1 | BROWSER_MOUSE_CLICK_2)))
		/* if not a button 1 or 2 click*/
		స్థితి_2 = messages_get("SelectClose");

	తిరిగి స్థితి_2;
}


/* private interface described in html/form_internal.h */
శూన్య
form_select_mouse_drag_చివర(నిర్మాణం form_control *control,
			   browser_mouse_state mouse,
			   పూర్ణాంకం x, పూర్ణాంకం y)
{
	పూర్ణాంకం x0, y0, x1, y1;
	పూర్ణాంకం box_x, box_y;
	నిర్మాణం box *box;
	నిర్మాణం form_select_menu *menu = control->దత్తాంశం.select.menu;

	box = control->box;

	/* Get global coords of scrollbar */
	box_coords(box, &box_x, &box_y);
	box_x -= box->border[ఎడమ_2].వెడల్పు;
	box_y += box->ఎత్తు + box->border[BOTTOM].వెడల్పు +
			box->padding[BOTTOM] + box->padding[శిఖరం_2];

	/* Get drag end coords relative to scrollbar */
	x = x - box_x;
	y = y - box_y;

	ఒకవేళ (menu->scroll_capture) {
		x -= menu->వెడల్పు - SCROLLBAR_వెడల్పు;
		scrollbar_mouse_drag_చివర(menu->scrollbar, mouse, x, y);
		తిరిగి;
	}

	x0 = 0;
	y0 = 0;
	x1 = menu->వెడల్పు;
	y1 = menu->ఎత్తు;


	ఒకవేళ (x > x0 && x < x1 - SCROLLBAR_వెడల్పు && y >  y0 && y < y1) {
		/* handle drag end above the option area like a regular click */
		form_select_menu_clicked(control, x, y);
	}
}


/* private interface described in html/form_internal.h */
శూన్య form_select_get_dimensions(నిర్మాణం form_control *control,
		పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు)
{
	*వెడల్పు = control->దత్తాంశం.select.menu->వెడల్పు;
	*ఎత్తు = control->దత్తాంశం.select.menu->ఎత్తు;
}


/* private interface described in html/form_internal.h */
శూన్య form_select_menu_callback(శూన్య *client_దత్తాంశం,
		పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	html_content *html = client_దత్తాంశం;
	పూర్ణాంకం menu_x, menu_y;
	నిర్మాణం box *box;

	box = html->visible_select_menu->box;
	box_coords(box, &menu_x, &menu_y);

	menu_x -= box->border[ఎడమ_2].వెడల్పు;
	menu_y += box->ఎత్తు + box->border[BOTTOM].వెడల్పు +
			box->padding[BOTTOM] +
			box->padding[శిఖరం_2];
	content__request_redraw((నిర్మాణం content *)html, menu_x + x, menu_y + y,
			వెడల్పు, ఎత్తు);
}


/* private interface described in html/form_internal.h */
శూన్య form_radio_సమితి(నిర్మాణం form_control *radio)
{
	నిర్మాణం form_control *control;

	ప్రతిజ్ఞా(radio);
	ఒకవేళ (!radio->form)
		తిరిగి;

	ఒకవేళ (radio->selected)
		తిరిగి;

	/* Clear selected state for other controls in
	 * the same radio button group */
	కొరకు (control = radio->form->controls;
	     control != ఖాళీ;
	     control = control->తదుపరి) {
		/* Only interested in radio inputs */
		ఒకవేళ (control->type != GADGET_RADIO)
			కొనసాగు;

		/* Ignore ourself */
		ఒకవేళ (control == radio)
			కొనసాగు;

		/* Ignore inputs where:
		 *   a) this or the other control have no name attribute
		 *   b) this or the other control have an empty name attribute
		 *   c) the control names do not match
		 */
		ఒకవేళ ((control->name == ఖాళీ) ||
		    (radio->name == ఖాళీ) ||
		    (control->name[0] == '\0') ||
		    (radio->name[0] == '\0') ||
		    strcmp(control->name, radio->name) != 0)
			కొనసాగు;

		/* Other control is in the same radio button group: clear its
		 * selected state */
		ఒకవేళ (control->selected) {
			control->selected = false;
			dom_html_ఆదానం_element_సమితి_checked(control->కణుపు, false);
			html__redraw_a_box(radio->html, control->box);
		}
	}

	radio->selected = true;
	dom_html_ఆదానం_element_సమితి_checked(radio->కణుపు, true);
	html__redraw_a_box(radio->html, radio->box);
}


/* private interface described in html/form_internal.h */
nserror
form_submit(nsurl *పుట_url,
	    నిర్మాణం browser_కిటికీ *లక్ష్యం,
	    నిర్మాణం form *form,
	    నిర్మాణం form_control *submit_button)
{
	nserror res;
	అక్షరం *దత్తాంశం = ఖాళీ; /* encoded form data */
	నిర్మాణం fetch_multipart_దత్తాంశం *success = ఖాళీ; /* gcc is incapable of correctly reasoning about use and generates "maybe used uninitialised" warnings */
	nsurl *action_url;
	nsurl *query_url;

	ప్రతిజ్ఞా(form != ఖాళీ);

	/* obtain list of controls from DOM */
	res = form_dom_గమ్యం_దత్తాంశం(form, submit_button, &success);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి res;
	}

	/* Decompose action */
	res = nsurl_create(form->action, &action_url);
	ఒకవేళ (res != NSERROR_OK) {
		fetch_multipart_దత్తాంశం_destroy(success);
		తిరిగి res;
	}

	ఎంపిక (form->పద్ధతి) {
	స్థితి పద్ధతి_GET:
		res = form_url_encode(form, success, &దత్తాంశం);
		ఒకవేళ (res == NSERROR_OK) {
			/* Replace query segment */
			res = nsurl_replace_query(action_url, దత్తాంశం, &query_url);
			ఒకవేళ (res == NSERROR_OK) {
				res = browser_కిటికీ_navigate(లక్ష్యం,
							      query_url,
							      పుట_url,
							      BW_NAVIGATE_HISTORY,
							      ఖాళీ,
							      ఖాళీ,
							      ఖాళీ);

				nsurl_unref(query_url);
			}
			విడుదల(దత్తాంశం);
		}
		ఆపు;

	స్థితి పద్ధతి_POST_URLENC:
		res = form_url_encode(form, success, &దత్తాంశం);
		ఒకవేళ (res == NSERROR_OK) {
			res = browser_కిటికీ_navigate(లక్ష్యం,
						      action_url,
						      పుట_url,
						      BW_NAVIGATE_HISTORY,
						      దత్తాంశం,
						      ఖాళీ,
						      ఖాళీ);
			విడుదల(దత్తాంశం);
		}
		ఆపు;

	స్థితి పద్ధతి_POST_MULTIPART:
		res = browser_కిటికీ_navigate(లక్ష్యం,
					      action_url,
					      పుట_url,
					      BW_NAVIGATE_HISTORY,
					      ఖాళీ,
					      success,
					      ఖాళీ);

		ఆపు;
	}

	nsurl_unref(action_url);
	fetch_multipart_దత్తాంశం_destroy(success);

	తిరిగి res;
}


/* exported interface documented in html/form_internal.h */
శూన్య form_gadget_update_విలువ(నిర్మాణం form_control *control, అక్షరం *విలువ)
{
	ఎంపిక (control->type) {
	స్థితి GADGET_HIDDEN:
	స్థితి GADGET_TEXTBOX:
	స్థితి GADGET_TEXTAREA:
	స్థితి GADGET_PASSWORD:
	స్థితి GADGET_FILE:
		ఒకవేళ (control->విలువ != ఖాళీ) {
			విడుదల(control->విలువ);
		}
		control->విలువ = విలువ;
		ఒకవేళ (control->కణుపు != ఖాళీ) {
			dom_exception err;
			dom_string *str;
			err = dom_string_create((uint8_t *)విలువ,
						అక్షరశ్రేణి_పొడవు(విలువ), &str);
			ఒకవేళ (err == DOM_NO_ERR) {
				ఒకవేళ (control->type == GADGET_TEXTAREA)
					err = dom_html_పాఠ్యం_area_element_సమితి_విలువ(
						(dom_html_పాఠ్యం_area_element *)(control->కణుపు),
						str);
				లేకపోతే
					err = dom_html_ఆదానం_element_సమితి_విలువ(
						(dom_html_ఆదానం_element *)(control->కణుపు),
						str);
				dom_string_unref(str);
			}
		}
		ఆపు;
	అప్రమేయ:
		/* Do nothing */
		ఆపు;
	}

	/* Finally, sync this with the DOM */
	form_gadget_sync_with_dom(control);
}


/* Exported API, see html/form_internal.h */
శూన్య
form_gadget_sync_with_dom(నిర్మాణం form_control *control)
{
	dom_exception exc;
	dom_string *విలువ = ఖాళీ;
	bool మారింది_dom = false;

	ఒకవేళ (control->syncing ||
	    (control->type != GADGET_TEXTBOX &&
	     control->type != GADGET_PASSWORD &&
	     control->type != GADGET_HIDDEN &&
	     control->type != GADGET_TEXTAREA)) {
		/* Not a control we support, or the control is already
		 * mid-sync so we don't want to disrupt that
		 */
		తిరిగి;
	}

	control->syncing = true;

	/* If we've changed value, sync that toward the DOM */
	ఒకవేళ ((control->చివరి_synced_విలువ == ఖాళీ &&
	     control->విలువ != ఖాళీ &&
	     control->విలువ[0] != '\0') ||
	    (control->చివరి_synced_విలువ != ఖాళీ &&
	     control->విలువ != ఖాళీ &&
	     strcmp(control->విలువ, control->చివరి_synced_విలువ) != 0)) {
		అక్షరం *dup = strdup(control->విలువ);
		ఒకవేళ (dup == ఖాళీ) {
			వెళ్ళు బహిర్గతం;
		}
		ఒకవేళ (control->చివరి_synced_విలువ != ఖాళీ) {
			విడుదల(control->చివరి_synced_విలువ);
		}
		control->చివరి_synced_విలువ = dup;
		exc = dom_string_create((uint8_t *)(control->విలువ),
					అక్షరశ్రేణి_పొడవు(control->విలువ), &విలువ);
		ఒకవేళ (exc != DOM_NO_ERR) {
			వెళ్ళు బహిర్గతం;
		}
		ఒకవేళ (control->కణుపు_విలువ != ఖాళీ) {
			dom_string_unref(control->కణుపు_విలువ);
		}
		control->కణుపు_విలువ = విలువ;
		విలువ = ఖాళీ;
		ఒకవేళ (control->type == GADGET_TEXTAREA) {
			exc = dom_html_పాఠ్యం_area_element_సమితి_విలువ(control->కణుపు, control->కణుపు_విలువ);
		} లేకపోతే {
			exc = dom_html_ఆదానం_element_సమితి_విలువ(control->కణుపు, control->కణుపు_విలువ);
		}
		ఒకవేళ (exc != DOM_NO_ERR) {
			వెళ్ళు బహిర్గతం;
		}
		మారింది_dom = true;
	}

	/* Now check if the DOM has changed since our last go */
	ఒకవేళ (control->type == GADGET_TEXTAREA) {
		exc = dom_html_పాఠ్యం_area_element_get_విలువ(control->కణుపు, &విలువ);
	} లేకపోతే {
		exc = dom_html_ఆదానం_element_get_విలువ(control->కణుపు, &విలువ);
	}

	ఒకవేళ (exc != DOM_NO_ERR) {
		/* Nothing much we can do here */
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (!dom_string_isequal(control->కణుపు_విలువ, విలువ)) {
		/* The DOM has changed */
		ఒకవేళ (!మారింది_dom) {
			/* And it wasn't us */
			అక్షరం *విలువ_s = strndup(
				dom_string_దత్తాంశం(విలువ),
				dom_string_byte_నిడివి(విలువ));
			అక్షరం *dup = ఖాళీ;
			ఒకవేళ (విలువ_s == ఖాళీ) {
				వెళ్ళు బహిర్గతం;
			}
			dup = strdup(విలువ_s);
			ఒకవేళ (dup == ఖాళీ) {
				విడుదల(విలువ_s);
				వెళ్ళు బహిర్గతం;
			}
			విడుదల(control->విలువ);
			control->విలువ = విలువ_s;
			విడుదల(control->చివరి_synced_విలువ);
			control->చివరి_synced_విలువ = dup;
			ఒకవేళ (control->type != GADGET_HIDDEN &&
			    control->దత్తాంశం.పాఠ్యం.ta != ఖాళీ) {
				textarea_సమితి_పాఠ్యం(control->దత్తాంశం.పాఠ్యం.ta,
						  విలువ_s);
			}
		}
		control->కణుపు_విలువ = విలువ;
		విలువ = ఖాళీ;
	}

బహిర్గతం:
	ఒకవేళ (విలువ != ఖాళీ)
		dom_string_unref(విలువ);
	control->syncing = false;
}


/* exported interface documented in html/form_internal.h */
నిర్మాణం form *
form_new(శూన్య *కణుపు,
	 స్థిరరాశి అక్షరం *action,
	 స్థిరరాశి అక్షరం *లక్ష్యం,
	 form_పద్ధతి పద్ధతి,
	 స్థిరరాశి అక్షరం *charset,
	 స్థిరరాశి అక్షరం *doc_charset)
{
	నిర్మాణం form *form;

	form = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం *form);
	ఒకవేళ (!form)
		తిరిగి ఖాళీ;

	form->action = strdup(action != ఖాళీ ? action : "");
	ఒకవేళ (form->action == ఖాళీ) {
		విడుదల(form);
		తిరిగి ఖాళీ;
	}

	form->లక్ష్యం = లక్ష్యం != ఖాళీ ? strdup(లక్ష్యం) : ఖాళీ;
	ఒకవేళ (లక్ష్యం != ఖాళీ && form->లక్ష్యం == ఖాళీ) {
		విడుదల(form->action);
		విడుదల(form);
		తిరిగి ఖాళీ;
	}

	form->పద్ధతి = పద్ధతి;

	form->accept_charsets = charset != ఖాళీ ? strdup(charset) : ఖాళీ;
	ఒకవేళ (charset != ఖాళీ && form->accept_charsets == ఖాళీ) {
		విడుదల(form->లక్ష్యం);
		విడుదల(form->action);
		విడుదల(form);
		తిరిగి ఖాళీ;
	}

	form->document_charset = doc_charset != ఖాళీ ? strdup(doc_charset)
						     : ఖాళీ;
	ఒకవేళ (doc_charset && form->document_charset == ఖాళీ) {
		విడుదల(form->accept_charsets);
		విడుదల(form->లక్ష్యం);
		విడుదల(form->action);
		విడుదల(form);
		తిరిగి ఖాళీ;
	}

	form->కణుపు = కణుపు;

	తిరిగి form;
}


/* exported interface documented in html/form_internal.h */
శూన్య form_free(నిర్మాణం form *form)
{
	నిర్మాణం form_control *c, *d;

	కొరకు (c = form->controls; c != ఖాళీ; c = d) {
		d = c->తదుపరి;

		form_free_control(c);
	}

	విడుదల(form->action);
	విడుదల(form->లక్ష్యం);
	విడుదల(form->accept_charsets);
	విడుదల(form->document_charset);

	విడుదల(form);
}


/* exported interface documented in html/form_internal.h */
నిర్మాణం form_control *form_new_control(శూన్య *కణుపు, form_control_type type)
{
	నిర్మాణం form_control *control;

	control = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం *control);
	ఒకవేళ (control == ఖాళీ)
		తిరిగి ఖాళీ;

	control->కణుపు = కణుపు;
	control->type = type;

	తిరిగి control;
}
