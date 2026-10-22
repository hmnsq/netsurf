/*
 * Copyright 2007 James Bursa <bursa@users.sourceforge.net>
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
 * Implementation of HTML content handling.
 */

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <nsutils/time.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/http.h"
#ಅನ್ತರ್ಭಾವ "utils/libdom.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/utf8.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/utf8.h"
#ಅನ್ತರ್ಭಾವ "netsurf/keypress.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "content/hlcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "javascript/js.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/dom_event.h"
#ಅನ್ತರ್ಭಾವ "html/css.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/html_save.h"
#ಅನ್ತರ್ಭಾವ "html/interaction.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/imagemap.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"
#ಅನ್ತರ್ಭಾವ "html/textselection.h"

#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ALWAYS_DUMP_FRAMESET 0
#ವ್ಯಾಖ್ಯಾನಿಸು ALWAYS_DUMP_BOX 0

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *html_types[] = {
	"application/xhtml+xml",
	"text/html"
};

/**
 * Fire an event at the DOM
 *
 * Helper that swallows DOM errors.
 *
 * \param[in] event   the event to fire at the DOM
 * \param[in] target  the event target
 * \return true on success
 */
ಸ್ಥಿರ bool fire_dom_event(dom_event *event, dom_ಗಂಟು *ಗುರಿ)
{
	dom_exception exc;
	bool ಫಲಿತಾಂಶ;

	exc = dom_event_ಗುರಿ_dispatch_event(ಗುರಿ, event, &ಫಲಿತಾಂಶ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_ಗಂಟು *ಗುರಿ,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool ಫಲಿತಾಂಶ;

	exc = dom_event_create(&evt);
	ಆದರೆ (exc != DOM_NO_ERR) ಹಿಂತಿರುಗಿ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಹಿಂತಿರುಗಿ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (ಪೂರ್ಣಾಂಕ)dom_string_ಉದ್ದಳತೆ(type), dom_string_ದತ್ತಾಂಶ(type), ಗುರಿ);
	ಫಲಿತಾಂಶ = fire_dom_event(evt, ಗುರಿ);
	dom_event_unref(evt);
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_ಗಂಟು *ಗುರಿ,
		bool bubbles, bool cancelable, uint32_t ಕೀಲಿ)
{
	bool is_special = ಕೀಲಿ <= 0x001F || (0x007F <= ಕೀಲಿ && ಕೀಲಿ <= 0x009F);
	dom_string *dom_ಕೀಲಿ = ಖಾಲಿ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool ಫಲಿತಾಂಶ;

	ಆದರೆ (is_special) {
		ಆಯ್ಕೆ (ಕೀಲಿ) {
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ESCAPE:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Escape);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಎಡ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಎಡ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಬಲ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಬಲ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಮೇಲೆ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಮೇಲೆ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಕೆಳಕ್ಕೆ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Arrow_ಕೆಳಕ್ಕೆ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಪುಟ_ಮೇಲೆ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_ಪುಟ_ಮೇಲೆ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಪುಟ_ಕೆಳಕ್ಕೆ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_ಪುಟ_ಕೆಳಕ್ಕೆ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಪಠ್ಯ_ಆರಂಭ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_Home);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NS_ಕೀಲಿ_ಪಠ್ಯ_ಕೊನೆ:
			dom_ಕೀಲಿ = dom_string_ref(corestring_dom_ಕೊನೆ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			dom_ಕೀಲಿ = ಖಾಲಿ;
			ನಿಲ್ಲಿಸು;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಕ್ಷರ utf8[6];
		size_t ಉದ್ದಳತೆ = utf8_from_ucs4(ಕೀಲಿ, utf8);
		utf8[ಉದ್ದಳತೆ] = '\0';

		exc = dom_string_create((ಸ್ಥಿರಾಂಕ uint8_t *)utf8, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(utf8),
				&dom_ಕೀಲಿ);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಹಿಂತಿರುಗಿ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ಖಾಲಿ,
			dom_ಕೀಲಿ, ಖಾಲಿ, DOM_ಕೀಲಿ_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_ಕೀಲಿ);
	ಆದರೆ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ಹಿಂತಿರುಗಿ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(ಪೂರ್ಣಾಂಕ)dom_string_ಉದ್ದಳತೆ(type), dom_string_ದತ್ತಾಂಶ(type), ಗುರಿ);

	ಫಲಿತಾಂಶ = fire_dom_event((dom_event *) evt, ಗುರಿ);
	dom_event_unref(evt);
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_box_convert_ಮುಗಿದ(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_ಗಂಟು *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ಖಾಲಿ;

	/* Clean up and report error if unsuccessful or aborted */
	ಆದರೆ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ಆದರೆ (success == false) {
			content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_BOX_CONVERT, ಖಾಲಿ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_STOPPED, ಖಾಲಿ);
		}

		content_ಗಣ_error(&c->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}


#ಆದರೆ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ALWAYS_DUMP_FRAMESET
	ಆದರೆ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#ಆದರೆ_ಅಂತ್ಯ

	exc = dom_document_get_document_element(c->document, (ಶೂನ್ಯ *) &html);
	ಆದರೆ ((exc != DOM_NO_ERR) || (html == ಖಾಲಿ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_DOM, ಖಾಲಿ);
		content_ಗಣ_error(&c->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_ಹೊರತೆಗೆ(c);
	ಆದರೆ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_ಪ್ರಸಾರ_error(&c->ತಳ, err, ಖಾಲಿ);
		content_ಗಣ_error(&c->ತಳ);
		dom_ಗಂಟು_unref(html);
		ಹಿಂತಿರುಗಿ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ಖಾಲಿ;

	content_ಗಣ_ready(&c->ತಳ);

	html_proceed_ಗಮ್ಯ_ಮುಗಿದ(c);

	dom_ಗಂಟು_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_ಗಮ್ಯ_ಮುಗಿದ(html_content *html)
{
	ಆಯ್ಕೆ (content_get_ಸ್ಥಿತಿ_2(&html->ತಳ)) {
	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_READY:
		ಆದರೆ (html->ತಳ.ಸಕ್ರಿಯ == 0) {
			content_ಗಣ_ಮುಗಿದ(&html->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_OK;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ:
		/* fallthrough */
	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_LOADING:
		ಹಿಂತಿರುಗಿ NSERROR_OK;
	ಪೂರ್ವನಿಯೋಜಿತ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ NSERROR_UNKNOWN;
}


ಸ್ಥಿರ ಶೂನ್ಯ html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	ಚಿಹ್ನೆರಹಿತ f_ಗಾತ್ರ;
	ಚಿಹ್ನೆರಹಿತ f_min;
	ಚಿಹ್ನೆರಹಿತ w;
	ಚಿಹ್ನೆರಹಿತ h;
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ = {
		.getdims = {
			.viewport_ಅಗಲ = &w,
			.viewport_ಎತ್ತರ = &h,
		},
	};

	content_ಪ್ರಸಾರ(&htmlc->ತಳ, CONTENT_ಸಂದೇಶ_GETDIMS, &ಸಂದೇಶ_ದತ್ತಾಂಶ);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.ಅಗಲ  = w;
	htmlc->media.ಎತ್ತರ = h;
	htmlc->unit_len_ctx.viewport_ಅಗಲ  = w;
	htmlc->unit_len_ctx.viewport_ಎತ್ತರ = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_ಗಾತ್ರ = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_ಗಾತ್ರ)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_ಗಾತ್ರ)), F_10)), F_72);

	htmlc->unit_len_ctx.font_ಗಾತ್ರ_default = f_ಗಾತ್ರ;
	htmlc->unit_len_ctx.font_ಗಾತ್ರ_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
ಶೂನ್ಯ html_ಮುಕ್ತಾಯ_conversion(html_content *htmlc)
{
	ಒಕ್ಕೂಟ content_ಸಂದೇಶ_ದತ್ತಾಂಶ ಸಂದೇಶ_ದತ್ತಾಂಶ;
	dom_exception exc; /* returned by libdom functions */
	dom_ಗಂಟು *html;
	nserror error;

	/* Bail out if we've been aborted */
	ಆದರೆ (htmlc->aborted) {
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, NSERROR_STOPPED, ಖಾಲಿ);
		content_ಗಣ_error(&htmlc->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}

	/* If we already have a selection context, then we have already
	 * "finished" conversion.  We can get here twice if e.g. some JS
	 * adds a new stylesheet, and the stylesheet gets added after
	 * the HTML content is initially finished.
	 *
	 * If we didn't do this, the HTML content would try to rebuild the
	 * box tree for the html content when this new stylesheet is ready.
	 * NetSurf has no concept of dynamically changing documents, so this
	 * would break badly.
	 */
	ಆದರೆ (htmlc->select_ctx != ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ಹಿಂತಿರುಗಿ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ಆದರೆ (error != NSERROR_OK) {
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, error, ಖಾಲಿ);
		content_ಗಣ_error(&htmlc->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ಆದರೆ (htmlc->jsthread != ಖಾಲಿ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ಖಾಲಿ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_ಗಣ_ಸ್ಥಿತಿ(&htmlc->ತಳ, messages_get("Processing"));
	ಸಂದೇಶ_ದತ್ತಾಂಶ.explicit_ಸ್ಥಿತಿ_ಪಠ್ಯ = ಖಾಲಿ;
	content_ಪ್ರಸಾರ(&htmlc->ತಳ, CONTENT_ಸಂದೇಶ_ಸ್ಥಿತಿ, &ಸಂದೇಶ_ದತ್ತಾಂಶ);

	exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
	ಆದರೆ ((exc != DOM_NO_ERR) || (html == ಖಾಲಿ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, NSERROR_DOM, ಖಾಲಿ);
		content_ಗಣ_error(&htmlc->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}

	html_get_dimensions(htmlc);

	error = dom_ಗಮ್ಯ_box(html, htmlc, html_box_convert_ಮುಗಿದ, &htmlc->box_conversion_context);
	ಆದರೆ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_ಗಂಟು_unref(html);
		html_object_free_objects(htmlc);
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, error, ಖಾಲಿ);
		content_ಗಣ_error(&htmlc->ತಳ);
		ಹಿಂತಿರುಗಿ;
	}

	dom_ಗಂಟು_unref(html);
}


ಸ್ಥಿರ ಶೂನ್ಯ
html_document_user_ದತ್ತಾಂಶ_handler(dom_ಗಂಟು_operation operation,
				dom_string *ಕೀಲಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
				ರಚನೆ dom_ಗಂಟು *src,
				ರಚನೆ dom_ಗಂಟು *dst)
{
	ಆದರೆ (dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_html_content_ದತ್ತಾಂಶ,
			       ಕೀಲಿ) == false || ದತ್ತಾಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ (operation) {
	ಸ್ಥಿತಿ DOM_ಗಂಟು_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_ಗಂಟು_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_ಗಂಟು_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_ಗಂಟು_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DOM_ಗಂಟು_DELETED:
		/* This is the only path I expect */
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}


ಸ್ಥಿರ nserror
html_create_html_ದತ್ತಾಂಶ(html_content *c, ಸ್ಥಿರಾಂಕ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	ಶೂನ್ಯ *old_ಗಂಟು_ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prefer_color_ಬಹುಲಕ = (nsoption_bool(prefer_dark_ಬಹುಲಕ)) ?
			"dark" : "light";

	c->parser = ಖಾಲಿ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ಖಾಲಿ;
	c->quirks = DOM_DOCUMENT_QUIRKS_ಬಹುಲಕ_NONE;
	c->encoding = ಖಾಲಿ;
	c->ತಳ_url = nsurl_ref(content_get_url(&c->ತಳ));
	c->ತಳ_ಗುರಿ = ಖಾಲಿ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ಖಾಲಿ;
	c->bctx = ಖಾಲಿ;
	c->layout = ಖಾಲಿ;
	c->background_ಬಣ್ಣ = NS_TRANSPARENT;
	c->stylesheet_ಎಣಿಕೆ = 0;
	c->stylesheets = ಖಾಲಿ;
	c->select_ctx = ಖಾಲಿ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ಖಾಲಿ;
	c->ಅಂಕೆ_objects = 0;
	c->object_list = ಖಾಲಿ;
	c->forms = ಖಾಲಿ;
	c->imagemaps = ಖಾಲಿ;
	c->bw = ಖಾಲಿ;
	c->frameset = ಖಾಲಿ;
	c->iframe = ಖಾಲಿ;
	c->ಪುಟ = ಖಾಲಿ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_ಎಣಿಕೆ = 0;
	c->scripts = ಖಾಲಿ;
	c->jsthread = ಖಾಲಿ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->ತಳ.ಸಕ್ರಿಯ = 1; /* The html content itself is active */

	ಆದರೆ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (lwc_intern_string(prefer_color_ಬಹುಲಕ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(prefer_color_ಬಹುಲಕ),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	c->sel = selection_create((ರಚನೆ content *)c);

	nerror = http_parameter_list_ಕಂಡುಹಿಡಿ_ಅಂಶ(params, corestring_lwc_charset, &charset);
	ಆದರೆ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_ದತ್ತಾಂಶ(charset));

		lwc_string_unref(charset);

		ಆದರೆ (c->encoding == ಖಾಲಿ) {
			lwc_string_unref(c->universal);
			c->universal = ಖಾಲಿ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ಖಾಲಿ;
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		}
		c->encoding_ಮೂಲ = DOM_HUBBUB_ENCODING_ಮೂಲ_ಶೀರ್ಷಿಕೆ;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.ಸಂದೇಶ = ಖಾಲಿ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ಆದರೆ ((error != DOM_HUBBUB_OK) && (c->encoding != ಖಾಲಿ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		ಬಿಡುಗಡೆ(c->encoding);
		c->encoding = ಖಾಲಿ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ಆದರೆ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->ತಳ_url);
		c->ತಳ_url = ಖಾಲಿ;

		lwc_string_unref(c->universal);
		c->universal = ಖಾಲಿ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ಖಾಲಿ;

		ಹಿಂತಿರುಗಿ libdom_hubbub_error_ಗಮ್ಯ_nserror(error);
	}

	err = dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(c->document,
				     corestring_dom_ns_ಕೀಲಿ_html_content_ದತ್ತಾಂಶ,
				     c, html_document_user_ದತ್ತಾಂಶ_handler,
				     (ಶೂನ್ಯ *) &old_ಗಂಟು_ದತ್ತಾಂಶ);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ಖಾಲಿ;
		nsurl_unref(c->ತಳ_url);
		c->ತಳ_url = ಖಾಲಿ;

		lwc_string_unref(c->universal);
		c->universal = ಖಾಲಿ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ಖಾಲಿ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ಹಿಂತಿರುಗಿ NSERROR_DOM;
	}

	ಪ್ರತಿಜ್ಞಾ(old_ಗಂಟು_ದತ್ತಾಂಶ == ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ಸ್ಥಿರ nserror
html_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
	    lwc_string *imime_type,
	    ಸ್ಥಿರಾಂಕ http_parameter *params,
	    llcache_handle *llcache,
	    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	    bool quirks,
	    ರಚನೆ content **c)
{
	html_content *html;
	nserror error;

	html = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(html_content));
	ಆದರೆ (html == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&html->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(html);
		ಹಿಂತಿರುಗಿ error;
	}

	error = html_create_html_ದತ್ತಾಂಶ(html, params);
	ಆದರೆ (error != NSERROR_OK) {
		content_ಪ್ರಸಾರ_error(&html->ತಳ, error, ಖಾಲಿ);
		ಬಿಡುಗಡೆ(html);
		ಹಿಂತಿರುಗಿ error;
	}

	error = html_css_new_stylesheets(html);
	ಆದರೆ (error != NSERROR_OK) {
		content_ಪ್ರಸಾರ_error(&html->ತಳ, error, ಖಾಲಿ);
		ಬಿಡುಗಡೆ(html);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) html;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}



ಸ್ಥಿರ nserror
html_process_encoding_change(ರಚನೆ content *c,
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ,
			     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *encoding;
	ಸ್ಥಿರಾಂಕ uint8_t *ಮೂಲ_ದತ್ತಾಂಶ;
	size_t ಮೂಲ_ಗಾತ್ರ;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_ಮೂಲ);
	ಆದರೆ (encoding == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಆದರೆ (html->encoding != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(html->encoding);
		html->encoding = ಖಾಲಿ;
	}

	html->encoding = strdup(encoding);
	ಆದರೆ (html->encoding == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ಖಾಲಿ;

	ಆದರೆ (html->document != ಖಾಲಿ) {
		dom_ಗಂಟು_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.ಸಂದೇಶ = ಖಾಲಿ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ಆದರೆ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		ಬಿಡುಗಡೆ(html->encoding);
		html->encoding = strdup("Windows-1252");
		ಆದರೆ (html->encoding == ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ಆದರೆ (error != DOM_HUBBUB_OK) {
			ಹಿಂತಿರುಗಿ libdom_hubbub_error_ಗಮ್ಯ_nserror(error);
		}

	}

	ಮೂಲ_ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಮೂಲ_ಗಾತ್ರ);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      ಮೂಲ_ದತ್ತಾಂಶ,
					      ಮೂಲ_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ libdom_hubbub_error_ಗಮ್ಯ_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ಸ್ಥಿರ bool
html_process_ದತ್ತಾಂಶ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ಸ್ಥಿರಾಂಕ uint8_t *) ದತ್ತಾಂಶ,
					      ಗಾತ್ರ_2);

	err = libdom_hubbub_error_ಗಮ್ಯ_nserror(dom_ret);

	/* deal with encoding change */
	ಆದರೆ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, ದತ್ತಾಂಶ, ಗಾತ್ರ_2);
	}

	/* broadcast the error if necessary */
	ಆದರೆ (err != NSERROR_OK) {
		content_ಪ್ರಸಾರ_error(c, err, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Convert a CONTENT_HTML for display.
 *
 * The following steps are carried out in order:
 *
 *  - parsing to an XML tree is completed
 *  - stylesheets are fetched
 *  - the XML tree is converted to a box tree and object fetches are started
 *
 * On exit, the content status will be either CONTENT_STATUS_DONE if the
 * document is completely loaded or CONTENT_STATUS_READY if objects are still
 * being fetched.
 */

ಸ್ಥಿರ bool html_convert(ರಚನೆ content *c)
{
	html_content *htmlc = (html_content *) c;
	dom_exception exc; /* returned by libdom functions */

	/* The quirk check and associated stylesheet fetch is "safe"
	 * once the root node has been inserted into the document
	 * which must have happened by this point in the parse.
	 *
	 * faliure to retrive the quirk mode or to start the
	 * stylesheet fetch is non fatal as this "only" affects the
	 * render and it would annoy the user to fail the entire
	 * render for want of a quirks stylesheet.
	 */
	exc = dom_document_get_quirks_ಬಹುಲಕ(htmlc->document, &htmlc->quirks);
	ಆದರೆ (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->ತಳ.ಸಕ್ರಿಯ--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->ತಳ.ಸಕ್ರಿಯ, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	ಆದರೆ (html_can_begin_conversion(htmlc)) {
		ಹಿಂತಿರುಗಿ html_begin_conversion(htmlc);
	}
	ಹಿಂತಿರುಗಿ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	/* Cannot begin conversion if we're still fetching stuff */
	ಆದರೆ (htmlc->ತಳ.ಸಕ್ರಿಯ != 0)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (i = 0; i != htmlc->stylesheet_ಎಣಿಕೆ; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ಆದರೆ (htmlc->stylesheets[i].modified)
			ಹಿಂತಿರುಗಿ false;
	}

	/* All is good, begin */
	ಹಿಂತಿರುಗಿ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_ಗಂಟು *html;
	nserror ns_error;
	ರಚನೆ form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *ಗಂಟು_name = ಖಾಲಿ;
	dom_hubbub_error error;

	/* The act of completing the parse can result in additional data
	 * being flushed through the parser. This may result in new style or
	 * script nodes, upon which the conversion depends. Thus, once we
	 * have completed the parse, we must check again to see if we can
	 * begin the conversion. If we can't, we must stop and wait for the
	 * new styles/scripts to be processed. Once they have been processed,
	 * we will be called again to begin the conversion for real. Thus,
	 * we must also ensure that we don't attempt to complete the parse
	 * multiple times, so store a flag to indicate that parsing is
	 * complete to avoid repeating the completion pointlessly.
	 */
	ಆದರೆ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ಆದರೆ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->ತಳ.ಸಕ್ರಿಯ > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ಹಿಂತಿರುಗಿ true;
		}
		ಆದರೆ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_ಪ್ರಸಾರ_error(&htmlc->ತಳ,
						libdom_hubbub_error_ಗಮ್ಯ_nserror(error),
						ಖಾಲಿ);

			ಹಿಂತಿರುಗಿ false;
		}
		htmlc->parse_completed = true;
	}

	ಆದರೆ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ಹಿಂತಿರುಗಿ true;
	}

	/* Give up processing if we've been aborted */
	ಆದರೆ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->ತಳ.ಸಕ್ರಿಯ);
		content_ಗಣ_error(&htmlc->ತಳ);
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, NSERROR_STOPPED, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ಆದರೆ (htmlc->encoding == ಖಾಲಿ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_ಮೂಲ);
		ಆದರೆ (encoding == ಖಾಲಿ) {
			content_ಪ್ರಸಾರ_error(&htmlc->ತಳ,
						NSERROR_NOMEM,
						ಖಾಲಿ);
			ಹಿಂತಿರುಗಿ false;
		}

		htmlc->encoding = strdup(encoding);
		ಆದರೆ (htmlc->encoding == ಖಾಲಿ) {
			content_ಪ್ರಸಾರ_error(&htmlc->ತಳ,
						NSERROR_NOMEM,
						ಖಾಲಿ);
			ಹಿಂತಿರುಗಿ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
	ಆದರೆ ((exc != DOM_NO_ERR) || (html == ಖಾಲಿ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, NSERROR_DOM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	exc = dom_ಗಂಟು_get_ಗಂಟು_name(html, &ಗಂಟು_name);
	ಆದರೆ ((exc != DOM_NO_ERR) ||
	    (ಗಂಟು_name == ಖಾಲಿ) ||
	    (!dom_string_caseless_lwc_isequal(ಗಂಟು_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, NSERROR_DOM, ಖಾಲಿ);
		dom_ಗಂಟು_unref(html);
		ಹಿಂತಿರುಗಿ false;
	}
	dom_string_unref(ಗಂಟು_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	ಗಾಗಿ (f = htmlc->forms; f != ಖಾಲಿ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ಆದರೆ (f->action == ಖಾಲಿ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->ತಳ);
			ns_error = nsurl_join(htmlc->ತಳ_url,
					      nsurl_access(doc_addr),
					      &action);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ns_error = nsurl_join(htmlc->ತಳ_url,
					      f->action,
					      &action);
		}

		ಆದರೆ (ns_error != NSERROR_OK) {
			content_ಪ್ರಸಾರ_error(&htmlc->ತಳ, ns_error, ಖಾಲಿ);

			dom_ಗಂಟು_unref(html);
			ಹಿಂತಿರುಗಿ false;
		}

		ಬಿಡುಗಡೆ(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ಆದರೆ (f->action == ಖಾಲಿ) {
			content_ಪ್ರಸಾರ_error(&htmlc->ತಳ,
						NSERROR_NOMEM,
						ಖಾಲಿ);

			dom_ಗಂಟು_unref(html);
			ಹಿಂತಿರುಗಿ false;
		}

		/* Ensure each form has a document encoding */
		ಆದರೆ (f->document_charset == ಖಾಲಿ) {
			f->document_charset = strdup(htmlc->encoding);
			ಆದರೆ (f->document_charset == ಖಾಲಿ) {
				content_ಪ್ರಸಾರ_error(&htmlc->ತಳ,
							NSERROR_NOMEM,
							ಖಾಲಿ);
				dom_ಗಂಟು_unref(html);
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}

	dom_ಗಂಟು_unref(html);

	ಆದರೆ (htmlc->ತಳ.ಸಕ್ರಿಯ == 0) {
		html_ಮುಕ್ತಾಯ_conversion(htmlc);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_stop(ರಚನೆ content *c)
{
	html_content *htmlc = (html_content *) c;

	ಆಯ್ಕೆ (c->ಸ್ಥಿತಿ_2) {
	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ಆದರೆ (htmlc->jsthread != ಖಾಲಿ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ಆದರೆ (c->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY && c->ಸಕ್ರಿಯ == 0) {
			content_ಗಣ_ಮುಗಿದ(c);
		}

		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ:
		/* Nothing to do */
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->ಸ್ಥಿತಿ_2,
		      c);
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_reformat(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ)
{
	html_content *htmlc = (html_content *) c;
	ರಚನೆ box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_ಅಗಲ = css_unit_device2css_px(
			INTTOFIX(ಅಗಲ), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_ಎತ್ತರ = css_unit_device2css_px(
			INTTOFIX(ಎತ್ತರ), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ಬೇರು_style = htmlc->layout->style;

	layout_document(htmlc, ಅಗಲ, ಎತ್ತರ);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->ಅಗಲ = layout->x + layout->padding[ಎಡ_2] + layout->ಅಗಲ +
		layout->padding[ಬಲ_2] + layout->border[ಬಲ_2].ಅಗಲ +
		layout->margin[ಬಲ_2];
	c->ಎತ್ತರ = layout->y + layout->padding[ತುದಿ_2] + layout->ಎತ್ತರ +
		layout->padding[BOTTOM] + layout->border[BOTTOM].ಅಗಲ +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ಆದರೆ (c->ಅಗಲ < layout->x + layout->descendant_x1)
		c->ಅಗಲ = layout->x + layout->descendant_x1;
	ಆದರೆ (c->ಎತ್ತರ < layout->y + layout->descendant_y1)
		c->ಎತ್ತರ = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ಆದರೆ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_ಸಮಯ = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

ಶೂನ್ಯ html_redraw_a_box(hlcache_handle *h, ರಚನೆ box *box)
{
	ಪೂರ್ಣಾಂಕ x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2],
			box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

ಶೂನ್ಯ html__redraw_a_box(ರಚನೆ html_content *html, ರಚನೆ box *box)
{
	ಪೂರ್ಣಾಂಕ x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ರಚನೆ content *)html, x, y,
			box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2],
			box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM]);
}

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy_frameset(ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *frameset)
{
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ಖಾಲಿ;
	}
	ಆದರೆ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ಖಾಲಿ;
	}
	ಆದರೆ (frameset->children) {
		ಗಾಗಿ (i = 0; i < (frameset->ಅಡ್ಡಸಾಲುಗಳು * frameset->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ); i++) {
			ಆದರೆ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ಖಾಲಿ;
			}
			ಆದರೆ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ಖಾಲಿ;
			}
			ಆದರೆ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ಖಾಲಿ;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy_iframe(ರಚನೆ content_html_iframe *iframe)
{
	ರಚನೆ content_html_iframe *ಮುಂದಿನ;
	ಮುಂದಿನ = iframe;
	ಆಗ ((iframe = ಮುಂದಿನ) != ಖಾಲಿ) {
		ಮುಂದಿನ = iframe->ಮುಂದಿನ;
		ಆದರೆ (iframe->name)
			talloc_free(iframe->name);
		ಆದರೆ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ಖಾಲಿ;
		}
		talloc_free(iframe);
	}
}


ಸ್ಥಿರ ಶೂನ್ಯ html_free_layout(html_content *htmlc)
{
	ಆದರೆ (htmlc->bctx != ಖಾಲಿ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_destroy(ರಚನೆ content *c)
{
	html_content *html = (html_content *) c;
	ರಚನೆ form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ಆದರೆ (html->box_conversion_context != ಖಾಲಿ) {
		ಆದರೆ (cancel_dom_ಗಮ್ಯ_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	ಗಾಗಿ (f = html->forms; f != ಖಾಲಿ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ಆದರೆ (c->refresh)
		nsurl_unref(c->refresh);

	ಆದರೆ (html->ತಳ_url)
		nsurl_unref(html->ತಳ_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ಆದರೆ (html->jsthread != ಖಾಲಿ) {
		js_destroythread(html->jsthread);
		html->jsthread = ಖಾಲಿ;
	}

	ಆದರೆ (html->parser != ಖಾಲಿ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ಖಾಲಿ;
	}

	ಆದರೆ (html->document != ಖಾಲಿ) {
		dom_ಗಂಟು_unref(html->document);
		html->document = ಖಾಲಿ;
	}

	ಆದರೆ (html->title != ಖಾಲಿ) {
		dom_ಗಂಟು_unref(html->title);
		html->title = ಖಾಲಿ;
	}

	/* Free encoding */
	ಆದರೆ (html->encoding != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(html->encoding);
		html->encoding = ಖಾಲಿ;
	}

	/* Free base target */
	ಆದರೆ (html->ತಳ_ಗುರಿ != ಖಾಲಿ) {
		ಬಿಡುಗಡೆ(html->ತಳ_ಗುರಿ);
		html->ತಳ_ಗುರಿ = ಖಾಲಿ;
	}

	/* Free frameset */
	ಆದರೆ (html->frameset != ಖಾಲಿ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ಖಾಲಿ;
	}

	/* Free iframes */
	ಆದರೆ (html->iframe != ಖಾಲಿ) {
		html_destroy_iframe(html->iframe);
		html->iframe = ಖಾಲಿ;
	}

	/* Destroy selection context */
	ಆದರೆ (html->select_ctx != ಖಾಲಿ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ಖಾಲಿ;
	}

	lwc_string_unref(html->universal);
	html->universal = ಖಾಲಿ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ಖಾಲಿ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ಸ್ಥಿರ nserror html_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ಪ್ರತಿಜ್ಞಾ(0 && "html_clone should never be called");

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ಸ್ಥಿರ nserror
html_open(ರಚನೆ content *c,
	  ರಚನೆ browser_ಕಿಟಕಿ *bw,
	  ರಚನೆ content *ಪುಟ,
	  ರಚನೆ object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->ಪುಟ = (html_content *) ಪುಟ;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ಸ್ಥಿರ nserror html_close(ರಚನೆ content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ಖಾಲಿ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ಆದರೆ (htmlc->jsthread != ಖಾಲಿ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ಹಿಂತಿರುಗಿ ret;
}


/**
 * Return an HTML content's selection context
 */

ಸ್ಥಿರ ಶೂನ್ಯ html_clear_selection(ರಚನೆ content *c)
{
	html_content *html = (html_content *) c;

	ಆಯ್ಕೆ (html->selection_type) {
	ಸ್ಥಿತಿ HTML_SELECTION_NONE:
		/* Nothing to do */
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == true);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ಸ್ಥಿರ ಅಕ್ಷರ *html_get_selection(ರಚನೆ content *c)
{
	html_content *html = (html_content *) c;

	ಆಯ್ಕೆ (html->selection_type) {
	ಸ್ಥಿತಿ HTML_SELECTION_TEXTAREA:
		ಹಿಂತಿರುಗಿ textarea_get_selection(html->selection_owner.textarea->
				gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta);
	ಸ್ಥಿತಿ HTML_SELECTION_SELF:
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == false);
		ಹಿಂತಿರುಗಿ selection_get_copy(html->sel);
	ಸ್ಥಿತಿ HTML_SELECTION_CONTENT:
		ಹಿಂತಿರುಗಿ content_get_selection(
				html->selection_owner.content->object);
	ಸ್ಥಿತಿ HTML_SELECTION_NONE:
		/* Nothing to do */
		ಪ್ರತಿಜ್ಞಾ(html->selection_owner.none == true);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/**
 * Get access to any content, link URLs and objects (images) currently
 * at the given (x, y) coordinates.
 *
 * \param[in] c html content to look inside
 * \param[in] x x-coordinate of point of interest
 * \param[in] y y-coordinate of point of interest
 * \param[out] data Positional features struct to be updated with any
 *             relevent content, or set to NULL if none.
 * \return NSERROR_OK on success else appropriate error code.
 */
ಸ್ಥಿರ nserror
html_get_contextual_content(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
			    ರಚನೆ browser_ಕಿಟಕಿ_features *ದತ್ತಾಂಶ)
{
	html_content *html = (html_content *) c;

	ರಚನೆ box *box = html->layout;
	ರಚನೆ box *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ box_x = 0, box_y = 0;

	ಆಗ ((ಮುಂದಿನ = box_at_ಬಿಂದು(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ಖಾಲಿ) {
		box = ಮುಂದಿನ;

		/* hidden boxes are ignored */
		ಆದರೆ ((box->style != ಖಾಲಿ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (box->iframe) {
			ದಶಮಾಂಶ scale = browser_ಕಿಟಕಿ_get_scale(box->iframe);
			browser_ಕಿಟಕಿ_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    ದತ್ತಾಂಶ);
		}

		ಆದರೆ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, ದತ್ತಾಂಶ);

		ಆದರೆ (box->object)
			ದತ್ತಾಂಶ->object = box->object;

		ಆದರೆ (box->href) {
			ದತ್ತಾಂಶ->link = box->href;
			ದತ್ತಾಂಶ->link_title = box->ಪಠ್ಯ;
			ದತ್ತಾಂಶ->link_title_ಉದ್ದಳತೆ = box->ಉದ್ದಳತೆ;
		}

		ಆದರೆ (box->usemap) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ = ಖಾಲಿ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &ಗುರಿ);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ಆದರೆ (url != ಖಾಲಿ)
				ದತ್ತಾಂಶ->link = url;
		}
		ಆದರೆ (box->gadget) {
			ಆಯ್ಕೆ (box->gadget->type) {
			ಸ್ಥಿತಿ GADGET_TEXTBOX:
			ಸ್ಥಿತಿ GADGET_TEXTAREA:
			ಸ್ಥಿತಿ GADGET_PASSWORD:
				ದತ್ತಾಂಶ->form_features = CTX_FORM_ಪಠ್ಯ;
				ನಿಲ್ಲಿಸು;

			ಸ್ಥಿತಿ GADGET_FILE:
				ದತ್ತಾಂಶ->form_features = CTX_FORM_FILE;
				ನಿಲ್ಲಿಸು;

			ಪೂರ್ವನಿಯೋಜಿತ:
				ದತ್ತಾಂಶ->form_features = CTX_FORM_NONE;
				ನಿಲ್ಲಿಸು;
			}
		}
	}
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Scroll deepest thing within the content which can be scrolled at given point
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param scrx	number of px try to scroll something in x direction
 * \param scry	number of px try to scroll something in y direction
 * \return true iff scroll was consumed by something in the content
 */
ಸ್ಥಿರ bool
html_scroll_at_ಬಿಂದು(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ scrx, ಪೂರ್ಣಾಂಕ scry)
{
	html_content *html = (html_content *) c;

	ರಚನೆ box *box = html->layout;
	ರಚನೆ box *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ಆಗ ((ಮುಂದಿನ = box_at_ಬಿಂದು(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ಖಾಲಿ) {
		box = ಮುಂದಿನ;

		ಆದರೆ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ಮುಂದುವರಿ;

		/* Pass into iframe */
		ಆದರೆ (box->iframe) {
			ದಶಮಾಂಶ scale = browser_ಕಿಟಕಿ_get_scale(box->iframe);

			ಆದರೆ (browser_ಕಿಟಕಿ_scroll_at_ಬಿಂದು(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ಹಿಂತಿರುಗಿ true;
		}

		/* Pass into textarea widget */
		ಆದರೆ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
						scrx, scry) == true)
			ಹಿಂತಿರುಗಿ true;

		/* Pass into object */
		ಆದರೆ (box->object != ಖಾಲಿ && content_scroll_at_ಬಿಂದು(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ಹಿಂತಿರುಗಿ true;

		/* Handle box scrollbars */
		ಆದರೆ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ಆದರೆ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ಆದರೆ (handled_scroll == true)
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ಸ್ಥಿರ ಶೂನ್ಯ html_dom_user_ದತ್ತಾಂಶ_handler(dom_ಗಂಟು_operation operation,
		dom_string *ಕೀಲಿ, ಶೂನ್ಯ *ದತ್ತಾಂಶ_2, ರಚನೆ dom_ಗಂಟು *src,
		ರಚನೆ dom_ಗಂಟು *dst)
{
	ಅಕ್ಷರ *oldfile;
	ಅಕ್ಷರ *ದತ್ತಾಂಶ = (ಅಕ್ಷರ *)ದತ್ತಾಂಶ_2;

	ಆದರೆ (!dom_string_isequal(corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_ದತ್ತಾಂಶ,
				ಕೀಲಿ) || ದತ್ತಾಂಶ == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ (operation) {
	ಸ್ಥಿತಿ DOM_ಗಂಟು_CLONED:
		ಆದರೆ (dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ(dst,
					   corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_ದತ್ತಾಂಶ,
					   strdup(ದತ್ತಾಂಶ), html_dom_user_ದತ್ತಾಂಶ_handler,
					   &oldfile) == DOM_NO_ERR) {
			ಆದರೆ (oldfile != ಖಾಲಿ)
				ಬಿಡುಗಡೆ(oldfile);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_RENAMED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_IMPORTED:
	ಸ್ಥಿತಿ DOM_ಗಂಟು_ADOPTED:
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_ಗಂಟು_DELETED:
		ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ಪ್ರತಿಜ್ಞಾ(0);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ html_ಗಣ_file_gadget_filename(ರಚನೆ content *c,
	ರಚನೆ form_control *gadget, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fn)
{
	nserror ret;
	ಅಕ್ಷರ *utf8_fn, *oldfile = ಖಾಲಿ;
	html_content *html = (html_content *)c;
	ರಚನೆ box *file_box = gadget->box;

	ret = guit->utf8->local_ಗಮ್ಯ_utf8(fn, 0, &utf8_fn);
	ಆದರೆ (ret != NSERROR_OK) {
		ಪ್ರತಿಜ್ಞಾ(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ಹಿಂತಿರುಗಿ;
	}

	form_gadget_update_ಮೌಲ್ಯ(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ಆದರೆ (dom_ಗಂಟು_ಗಣ_user_ದತ್ತಾಂಶ((dom_ಗಂಟು *)file_box->gadget->ಗಂಟು,
				   corestring_dom_ns_ಕೀಲಿ_file_name_ಗಂಟು_ದತ್ತಾಂಶ,
				   strdup(fn), html_dom_user_ದತ್ತಾಂಶ_handler,
				   &oldfile) == DOM_NO_ERR) {
		ಆದರೆ (oldfile != ಖಾಲಿ)
			ಬಿಡುಗಡೆ(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

ಶೂನ್ಯ html_ಗಣ_file_gadget_filename_2(ರಚನೆ hlcache_handle *hl,
	ರಚನೆ form_control *gadget, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fn)
{
	ಹಿಂತಿರುಗಿ html_ಗಣ_file_gadget_filename(hlcache_handle_get_content(hl),
		gadget, fn);
}

/**
 * Drop a file onto a content at a particular point, or determine if a file
 * may be dropped onto the content at given point.
 *
 * \param c	html content to look inside
 * \param x	x-coordinate of point of interest
 * \param y	y-coordinate of point of interest
 * \param file	path to file to be dropped, or NULL to know if drop allowed
 * \return true iff file drop has been handled, or if drop possible (NULL file)
 */
ಸ್ಥಿರ bool html_drop_file_at_ಬಿಂದು(ರಚನೆ content *c, ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಅಕ್ಷರ *file)
{
	html_content *html = (html_content *) c;

	ರಚನೆ box *box = html->layout;
	ರಚನೆ box *ಮುಂದಿನ;
	ರಚನೆ box *file_box = ಖಾಲಿ;
	ರಚನೆ box *ಪಠ್ಯ_box = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ಆಗ ((ಮುಂದಿನ = box_at_ಬಿಂದು(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ಖಾಲಿ) {
		box = ಮುಂದಿನ;

		ಆದರೆ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ಮುಂದುವರಿ;

		ಆದರೆ (box->iframe) {
			ದಶಮಾಂಶ scale = browser_ಕಿಟಕಿ_get_scale(box->iframe);
			ಹಿಂತಿರುಗಿ browser_ಕಿಟಕಿ_drop_file_at_ಬಿಂದು(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ಆದರೆ (box->object &&
		    content_drop_file_at_ಬಿಂದು(box->object,
					x - box_x, y - box_y, file) == true)
			ಹಿಂತಿರುಗಿ true;

		ಆದರೆ (box->gadget) {
			ಆಯ್ಕೆ (box->gadget->type) {
				ಸ್ಥಿತಿ GADGET_FILE:
					file_box = box;
				ನಿಲ್ಲಿಸು;

				ಸ್ಥಿತಿ GADGET_TEXTBOX:
				ಸ್ಥಿತಿ GADGET_TEXTAREA:
				ಸ್ಥಿತಿ GADGET_PASSWORD:
					ಪಠ್ಯ_box = box;
					ನಿಲ್ಲಿಸು;

				ಪೂರ್ವನಿಯೋಜಿತ:	/* appease compiler */
					ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಆದರೆ (!file_box && !ಪಠ್ಯ_box)
		/* No box capable of handling drop */
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (file == ಖಾಲಿ)
		/* There is a box capable of handling drop here */
		ಹಿಂತಿರುಗಿ true;

	/* Handle the drop */
	ಆದರೆ (file_box) {
		/* File dropped on file input */
		html_ಗಣ_file_gadget_filename(c, file_box->gadget, file);

	} ಇಲ್ಲದಿದ್ದರೆ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ಖಾಲಿ;
		ಅಕ್ಷರ *ಬಫರ್;
		ಅಕ್ಷರ *utf8_buff;
		nserror ret;
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
		ಪೂರ್ಣಾಂಕ bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ಆದರೆ (fp == ಖಾಲಿ) {
			/* Couldn't open file, but drop was for us */
			ಹಿಂತಿರುಗಿ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ಆದರೆ ((ದೀರ್ಘ)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ಹಿಂತಿರುಗಿ true;
		}

		/* Allocate buffer for file data */
		ಬಫರ್ = ಸ್ಮರಣೆ_ಕೊಡು(file_len + 1);
		ಆದರೆ (ಬಫರ್ == ಖಾಲಿ) {
			/* No memory, but drop was for us */
			fclose(fp);
			ಹಿಂತಿರುಗಿ true;
		}

		/* Stick file into buffer */
		ಆದರೆ (file_len != fread(ಬಫರ್, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			ಬಿಡುಗಡೆ(ಬಫರ್);
			fclose(fp);
			ಹಿಂತಿರುಗಿ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		ಬಫರ್[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_ಗಮ್ಯ_utf8(ಬಫರ್, file_len, &utf8_buff);
		ಆದರೆ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ಪ್ರತಿಜ್ಞಾ(ret != NSERROR_BAD_ENCODING);
			ಬಿಡುಗಡೆ(ಬಫರ್);
			ಹಿಂತಿರುಗಿ true;
		}

		/* Done with buffer */
		ಬಿಡುಗಡೆ(ಬಫರ್);

		/* Get new length */
		ಗಾತ್ರ_2 = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(ಪಠ್ಯ_box, &bx, &by);
		textarea_mouse_action(ಪಠ್ಯ_box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_ಪಠ್ಯ(ಪಠ್ಯ_box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta,
				utf8_buff, ಗಾತ್ರ_2);

		ಬಿಡುಗಡೆ(utf8_buff);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ಸ್ಥಿರ nserror
html_debug(ರಚನೆ content *c, ಎಣಿಕೆ content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ಸ್ಥಿರ nserror
html_debug_dump(ರಚನೆ content *c, FILE *f, ಎಣಿಕೆ content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_ಗಂಟು *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ಪ್ರತಿಜ್ಞಾ(htmlc != ಖಾಲಿ);

	ಆದರೆ (op == CONTENT_DEBUG_RENDER) {
		ಪ್ರತಿಜ್ಞಾ(htmlc->layout != ಖಾಲಿ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (htmlc->document == ಖಾಲಿ) {
			NSLOG(netsurf, INFO, "No document to dump");
			ಹಿಂತಿರುಗಿ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (ಶೂನ್ಯ *) &html);
		ಆದರೆ ((exc != DOM_NO_ERR) || (html == ಖಾಲಿ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ಹಿಂತಿರುಗಿ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_ಗಂಟು_unref(html);
	}

	ಹಿಂತಿರುಗಿ ret;
}


#ಆದರೆ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ಸ್ಥಿರ ಶೂನ್ಯ
html_dump_frameset(ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *ಚೌಕಟ್ಟು, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆಳ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು, index;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *unit[] = {"px", "%", "*"};
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *scrolling[] = {"auto", "yes", "no"};

	ಪ್ರತಿಜ್ಞಾ(ಚೌಕಟ್ಟು);

	fprintf(stderr, "%p ", ಚೌಕಟ್ಟು);

	fprintf(stderr, "(%i %i) ", ಚೌಕಟ್ಟು->ಅಡ್ಡಸಾಲುಗಳು, ಚೌಕಟ್ಟು->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);

	fprintf(stderr, "w%g%s ", ಚೌಕಟ್ಟು->ಅಗಲ.ಮೌಲ್ಯ, unit[ಚೌಕಟ್ಟು->ಅಗಲ.unit]);
	fprintf(stderr, "h%g%s ", ಚೌಕಟ್ಟು->ಎತ್ತರ.ಮೌಲ್ಯ,unit[ಚೌಕಟ್ಟು->ಎತ್ತರ.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			ಚೌಕಟ್ಟು->margin_ಅಗಲ, ಚೌಕಟ್ಟು->margin_ಎತ್ತರ);

	ಆದರೆ (ಚೌಕಟ್ಟು->name)
		fprintf(stderr, "'%s' ", ಚೌಕಟ್ಟು->name);
	ಆದರೆ (ಚೌಕಟ್ಟು->url)
		fprintf(stderr, "<%s> ", ಚೌಕಟ್ಟು->url);

	ಆದರೆ (ಚೌಕಟ್ಟು->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[ಚೌಕಟ್ಟು->scrolling]);
	ಆದರೆ (ಚೌಕಟ್ಟು->border)
		fprintf(stderr, "border %x ",
				(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) ಚೌಕಟ್ಟು->border_ಬಣ್ಣ);

	fprintf(stderr, "\n");

	ಆದರೆ (ಚೌಕಟ್ಟು->children) {
		ಗಾಗಿ (ಅಡ್ಡಸಾಲು = 0; ಅಡ್ಡಸಾಲು != ಚೌಕಟ್ಟು->ಅಡ್ಡಸಾಲುಗಳು; ಅಡ್ಡಸಾಲು++) {
			ಗಾಗಿ (ಲಂಬಸಾಲು = 0; ಲಂಬಸಾಲು != ಚೌಕಟ್ಟು->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ; ಲಂಬಸಾಲು++) {
				ಗಾಗಿ (i = 0; i != ಆಳ; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು);
				index = (ಅಡ್ಡಸಾಲು * ಚೌಕಟ್ಟು->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) + ಲಂಬಸಾಲು;
				html_dump_frameset(&ಚೌಕಟ್ಟು->children[index],
						ಆಳ + 1);
			}
		}
	}
}

#ಆದರೆ_ಅಂತ್ಯ

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->document;
}

/**
 * Retrieve box tree
 *
 * \param h  HTML content to retrieve tree from
 * \return Pointer to box tree
 *
 * \todo This API must die, as must all use of the box tree outside of
 *         HTML content handler
 */
ರಚನೆ box *html_get_box_ವೃಕ್ಷ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *html_encoding(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಎಣಿಕೆ content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ಸ್ಥಿರ ಅಕ್ಷರ enc_token[10] = "Encoding0";

	ಪ್ರತಿಜ್ಞಾ(html != ಖಾಲಿ);

	ಆದರೆ (op == CONTENT_ENCODING_ಮೂಲ) {
		enc_token[8] = '0' + html->encoding_ಮೂಲ;
		ಹಿಂತಿರುಗಿ messages_get(enc_token);
	}

	ಹಿಂತಿರುಗಿ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ರಚನೆ content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_ತಳ_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->ತಳ_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *html_get_ತಳ_ಗುರಿ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ಪ್ರತಿಜ್ಞಾ(c != ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ c->ತಳ_ಗುರಿ;
}


/**
 * Retrieve layout coordinates of box with given id
 *
 * \param h        HTML document to search
 * \param frag_id  String containing an element id
 * \param x        Updated to global x coord iff id found
 * \param y        Updated to global y coord iff id found
 * \return  true iff id found
 */
bool html_get_id_ಸ್ಥಾನಾಂತರ(hlcache_handle *h, lwc_string *frag_id, ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y)
{
	ರಚನೆ box *pos;
	ರಚನೆ box *layout;

	ಆದರೆ (content_get_type(h) != CONTENT_HTML)
		ಹಿಂತಿರುಗಿ false;

	layout = html_get_box_ವೃಕ್ಷ(h);

	ಆದರೆ ((pos = box_ಕಂಡುಹಿಡಿ_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ಹಿಂತಿರುಗಿ true;
	}
	ಹಿಂತಿರುಗಿ false;
}

bool html_exec(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool ಫಲಿತಾಂಶ = false;
	dom_exception err;
	dom_html_body_element *body_ಗಂಟು;
	dom_string *dom_src;
	dom_ಪಠ್ಯ *ಪಠ್ಯ_ಗಂಟು;
	dom_ಗಂಟು *ಹೆಚ್ಚುವರಿ_ಗಂಟು;
	dom_html_script_element *script_ಗಂಟು;

	ಆದರೆ (htmlc->document == ಖಾಲಿ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		ಹೋಗು ಹೊರಹೋಗು_no_string;
	}

	err = dom_string_create((ಸ್ಥಿರಾಂಕ uint8_t *)src, srclen, &dom_src);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		ಹೋಗು ಹೊರಹೋಗು_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_ಗಂಟು);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		ಹೋಗು ಹೊರಹೋಗು_no_body;
	}

	err = dom_document_create_ಪಠ್ಯ_ಗಂಟು(htmlc->document, dom_src, &ಪಠ್ಯ_ಗಂಟು);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		ಹೋಗು ಹೊರಹೋಗು_no_ಪಠ್ಯ_ಗಂಟು;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_ಗಂಟು);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		ಹೋಗು ಹೊರಹೋಗು_no_script_ಗಂಟು;
	}

	err = dom_ಗಂಟು_append_ಮಗು(script_ಗಂಟು, ಪಠ್ಯ_ಗಂಟು, &ಹೆಚ್ಚುವರಿ_ಗಂಟು);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		ಹೋಗು ಹೊರಹೋಗು_unparented;
	}
	dom_ಗಂಟು_unref(ಹೆಚ್ಚುವರಿ_ಗಂಟು); /* We do not need the spare ref at all */

	err = dom_ಗಂಟು_append_ಮಗು(body_ಗಂಟು, script_ಗಂಟು, &ಹೆಚ್ಚುವರಿ_ಗಂಟು);
	ಆದರೆ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		ಹೋಗು ಹೊರಹೋಗು_unparented;
	}
	dom_ಗಂಟು_unref(ಹೆಚ್ಚುವರಿ_ಗಂಟು); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	ಫಲಿತಾಂಶ = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(script_ಗಂಟು, &ಹೆಚ್ಚುವರಿ_ಗಂಟು);
	ಆದರೆ (err == DOM_NO_ERR && ಹೆಚ್ಚುವರಿ_ಗಂಟು != ಖಾಲಿ) {
		dom_ಗಂಟು *ಎರಡನೇ_ಹೆಚ್ಚುವರಿ;
		err = dom_ಗಂಟು_remove_ಮಗು(ಹೆಚ್ಚುವರಿ_ಗಂಟು, script_ಗಂಟು, &ಎರಡನೇ_ಹೆಚ್ಚುವರಿ);
		ಆದರೆ (err == DOM_NO_ERR) {
			dom_ಗಂಟು_unref(ಎರಡನೇ_ಹೆಚ್ಚುವರಿ);
		}
		dom_ಗಂಟು_unref(ಹೆಚ್ಚುವರಿ_ಗಂಟು);
	}

ಹೊರಹೋಗು_unparented:
	dom_ಗಂಟು_unref(script_ಗಂಟು);
ಹೊರಹೋಗು_no_script_ಗಂಟು:
	dom_ಗಂಟು_unref(ಪಠ್ಯ_ಗಂಟು);
ಹೊರಹೋಗು_no_ಪಠ್ಯ_ಗಂಟು:
	dom_ಗಂಟು_unref(body_ಗಂಟು);
ಹೊರಹೋಗು_no_body:
	dom_string_unref(dom_src);
ಹೊರಹೋಗು_no_string:
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

/* See \ref content_saw_insecure_objects */
ಸ್ಥಿರ bool
html_saw_insecure_objects(ರಚನೆ content *c)
{
	html_content *htmlc = (html_content *)c;
	ರಚನೆ content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ಆಗ (obj != ಖಾಲಿ) {
		ಆದರೆ (obj->content != ಖಾಲಿ) {
			ಆದರೆ (content_saw_insecure_objects(obj->content))
				ಹಿಂತಿರುಗಿ true;
		}
		obj = obj->ಮುಂದಿನ;
	}

	/* Now check the script list */
	ಆದರೆ (html_saw_insecure_scripts(htmlc)) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* Now check stylesheets */
	ಆದರೆ (html_css_saw_insecure_stylesheets(htmlc)) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ಸ್ಥಿರ content_type html_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_HTML;
}


ಸ್ಥಿರ ಶೂನ್ಯ html_fini(ಶೂನ್ಯ)
{
	html_css_fini();
}

/**
 * Finds all occurrences of a given string in an html box
 *
 * \param pattern   the string pattern to search for
 * \param p_len     pattern length
 * \param cur       pointer to the current box
 * \param case_sens whether to perform a case sensitive search
 * \param context   The search context to add the entry to.
 * \return true on success, false on memory allocation failure
 */
ಸ್ಥಿರ nserror
ಕಂಡುಹಿಡಿ_occurrences_html_box(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ,
			  ಪೂರ್ಣಾಂಕ p_len,
			  ರಚನೆ box *cur,
			  bool case_sens,
			  ರಚನೆ textsearch_context *context)
{
	ರಚನೆ box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ಆದರೆ (!cur->object && cur->ಪಠ್ಯ) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ = cur->ಪಠ್ಯ;
		ಚಿಹ್ನೆರಹಿತ ಉದ್ದಳತೆ = cur->ಉದ್ದಳತೆ;

		ಆಗ (ಉದ್ದಳತೆ > 0) {
			ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
			ಚಿಹ್ನೆರಹಿತ ಹೊಂದಿಕೆ_ಸ್ಥಾನಾಂತರ;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *new_ಪಠ್ಯ;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pos;

			pos = content_textsearch_ಕಂಡುಹಿಡಿ_ನಮೂನೆ(ಪಠ್ಯ,
					   ಉದ್ದಳತೆ,
					   ನಮೂನೆ,
					   p_len,
					   case_sens,
					   &ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ);
			ಆದರೆ (!pos)
				ನಿಲ್ಲಿಸು;

			/* found string in box => add to list */
			ಹೊಂದಿಕೆ_ಸ್ಥಾನಾಂತರ = pos - cur->ಪಠ್ಯ;

			res = content_textsearch_ಕೂಡಿಸು_ಹೊಂದಿಕೆ(context,
					cur->byte_ಸ್ಥಾನಾಂತರ + ಹೊಂದಿಕೆ_ಸ್ಥಾನಾಂತರ,
					cur->byte_ಸ್ಥಾನಾಂತರ + ಹೊಂದಿಕೆ_ಸ್ಥಾನಾಂತರ + ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ,
					cur,
					cur);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}

			new_ಪಠ್ಯ = pos + ಹೊಂದಿಕೆ_ಉದ್ದಳತೆ;
			ಉದ್ದಳತೆ -= (new_ಪಠ್ಯ - ಪಠ್ಯ);
			ಪಠ್ಯ = new_ಪಠ್ಯ;
		}
	}

	/* and recurse */
	ಗಾಗಿ (a = cur->children; a; a = a->ಮುಂದಿನ) {
		res = ಕಂಡುಹಿಡಿ_occurrences_html_box(ನಮೂನೆ,
						p_len,
						a,
						case_sens,
						context);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ res;
		}
	}

	ಹಿಂತಿರುಗಿ res;
}

/**
 * Finds all occurrences of a given string in the html box tree
 *
 * \param pattern   the string pattern to search for
 * \param p_len     pattern length
 * \param c The content to search
 * \param csens whether to perform a case sensitive search
 * \param context   The search context to add the entry to.
 * \return true on success, false on memory allocation failure
 */
ಸ್ಥಿರ nserror
html_textsearch_ಕಂಡುಹಿಡಿ(ರಚನೆ content *c,
		     ರಚನೆ textsearch_context *context,
		     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ,
		     ಪೂರ್ಣಾಂಕ p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ಆದರೆ (html->layout == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_INVALID;
	}

	ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_occurrences_html_box(ನಮೂನೆ,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ಸ್ಥಿರ nserror
html_textsearch_bounds(ರಚನೆ content *c,
		       ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx,
		       ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx,
		       ರಚನೆ box *ಆರಂಭ_box,
		       ರಚನೆ box *ಕೊನೆ_box,
		       ರಚನೆ rect *bounds)
{
	/* get box position and jump to it */
	box_coords(ಆರಂಭ_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(ಕೊನೆ_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += ಕೊನೆ_box->ಅಗಲ;
	bounds->y1 += ಕೊನೆ_box->ಎತ್ತರ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_ದತ್ತಾಂಶ = html_process_ದತ್ತಾಂಶ,
	.ದತ್ತಾಂಶ_complete = html_convert,
	.reformat = html_reformat,
	.destroy = html_destroy,
	.stop = html_stop,
	.mouse_track = html_mouse_track,
	.mouse_action = html_mouse_action,
	.keypress = html_keypress,
	.redraw = html_redraw,
	.open = html_open,
	.close = html_close,
	.get_selection = html_get_selection,
	.clear_selection = html_clear_selection,
	.get_contextual_content = html_get_contextual_content,
	.scroll_at_ಬಿಂದು = html_scroll_at_ಬಿಂದು,
	.drop_file_at_ಬಿಂದು = html_drop_file_at_ಬಿಂದು,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_ಕಂಡುಹಿಡಿ = html_textsearch_ಕಂಡುಹಿಡಿ,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_ಕೊನೆ = html_textselection_get_ಕೊನೆ,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(ಶೂನ್ಯ)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ಆದರೆ (error != NSERROR_OK)
		ಹೋಗು error;

	ಗಾಗಿ (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_ರಿಜಿಸ್ಟರ್_handler(html_types[i],
				&html_content_handler);
		ಆದರೆ (error != NSERROR_OK)
			ಹೋಗು error;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;

error:
	html_fini();

	ಹಿಂತಿರುಗಿ error;
}
