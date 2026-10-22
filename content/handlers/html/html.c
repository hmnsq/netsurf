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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/libdom.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utf8.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/utf8.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/keypress.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/hlcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "javascript/js.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/dom_event.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/css.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/object.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/html_save.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/interaction.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_construct.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/imagemap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/textselection.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALWAYS_DUMP_FRAMESET 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALWAYS_DUMP_BOX 0

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *html_types[] = {
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
ਸਥਿਰ bool fire_dom_event(dom_event *event, dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ)
{
	dom_exception exc;
	bool ਨਤੀਜਾ;

	exc = dom_event_ਨਿਸ਼ਾਨਾ_dispatch_event(ਨਿਸ਼ਾਨਾ, event, &ਨਤੀਜਾ);
	ਜੇ (exc != DOM_NO_ERR) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ ਨਤੀਜਾ;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool ਨਤੀਜਾ;

	exc = dom_event_create(&evt);
	ਜੇ (exc != DOM_NO_ERR) ਵਾਪਸ false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ਵਾਪਸ false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (ਪੂਰਨ)dom_string_ਲੰਮਾਈ(type), dom_string_ਅੰਕੜੇ(type), ਨਿਸ਼ਾਨਾ);
	ਨਤੀਜਾ = fire_dom_event(evt, ਨਿਸ਼ਾਨਾ);
	dom_event_unref(evt);
	ਵਾਪਸ ਨਤੀਜਾ;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_ਗੰਢ *ਨਿਸ਼ਾਨਾ,
		bool bubbles, bool cancelable, uint32_t ਕੁੰਜੀ)
{
	bool is_special = ਕੁੰਜੀ <= 0x001F || (0x007F <= ਕੁੰਜੀ && ਕੁੰਜੀ <= 0x009F);
	dom_string *dom_ਕੁੰਜੀ = ਨਿਰਾ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool ਨਤੀਜਾ;

	ਜੇ (is_special) {
		ਚੋਣ (ਕੁੰਜੀ) {
		ਹਾਲਤ NS_ਕੁੰਜੀ_ESCAPE:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Escape);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਖੱਬਾ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Arrow_ਖੱਬਾ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਸੱਜਾ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Arrow_ਸੱਜਾ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਉੱਪਰ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Arrow_ਉੱਪਰ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਥੱਲੇ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Arrow_ਥੱਲੇ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਪੰਨਾ_ਉੱਪਰ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_ਪੰਨਾ_ਉੱਪਰ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਪੰਨਾ_ਥੱਲੇ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_ਪੰਨਾ_ਥੱਲੇ);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਪਾਠ_ਸ਼ੁਰੂ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_Home);
			ਰੋਕੋ;
		ਹਾਲਤ NS_ਕੁੰਜੀ_ਪਾਠ_ਸਿਰਾ:
			dom_ਕੁੰਜੀ = dom_string_ref(corestring_dom_ਸਿਰਾ);
			ਰੋਕੋ;
		ਮੂਲ:
			dom_ਕੁੰਜੀ = ਨਿਰਾ;
			ਰੋਕੋ;
		}
	} ਨਹੀਂਤਾਂ {
		ਅੱਖਰ utf8[6];
		size_t ਲੰਮਾਈ = utf8_from_ucs4(ਕੁੰਜੀ, utf8);
		utf8[ਲੰਮਾਈ] = '\0';

		exc = dom_string_create((ਅਚਲ uint8_t *)utf8, ਲੜੀ_ਲੰਬਾਈ(utf8),
				&dom_ਕੁੰਜੀ);
		ਜੇ (exc != DOM_NO_ERR) {
			ਵਾਪਸ exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_ਕੁੰਜੀ);
		ਵਾਪਸ false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ਨਿਰਾ,
			dom_ਕੁੰਜੀ, ਨਿਰਾ, DOM_ਕੁੰਜੀ_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_ਕੁੰਜੀ);
	ਜੇ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		ਵਾਪਸ false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(ਪੂਰਨ)dom_string_ਲੰਮਾਈ(type), dom_string_ਅੰਕੜੇ(type), ਨਿਸ਼ਾਨਾ);

	ਨਤੀਜਾ = fire_dom_event((dom_event *) evt, ਨਿਸ਼ਾਨਾ);
	dom_event_unref(evt);
	ਵਾਪਸ ਨਤੀਜਾ;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
ਸਥਿਰ ਖਾਲੀ html_box_convert_ਪੂਰਾ(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_ਗੰਢ *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ਨਿਰਾ;

	/* Clean up and report error if unsuccessful or aborted */
	ਜੇ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ਜੇ (success == false) {
			content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_BOX_CONVERT, ਨਿਰਾ);
		} ਨਹੀਂਤਾਂ {
			content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_STOPPED, ਨਿਰਾ);
		}

		content_ਸੈੱਟ_error(&c->ਅਧਾਰ);
		ਵਾਪਸ;
	}


#ਜੇ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#ਜੇ_ਅੰਤ
#ਜੇ ALWAYS_DUMP_FRAMESET
	ਜੇ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#ਜੇ_ਅੰਤ

	exc = dom_document_get_document_element(c->document, (ਖਾਲੀ *) &html);
	ਜੇ ((exc != DOM_NO_ERR) || (html == ਨਿਰਾ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_DOM, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&c->ਅਧਾਰ);
		ਵਾਪਸ;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_ਕੱਢੋ(c);
	ਜੇ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, err, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&c->ਅਧਾਰ);
		dom_ਗੰਢ_unref(html);
		ਵਾਪਸ;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ਨਿਰਾ;

	content_ਸੈੱਟ_ready(&c->ਅਧਾਰ);

	html_proceed_ਮੰਜ਼ਿਲ_ਪੂਰਾ(c);

	dom_ਗੰਢ_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_ਮੰਜ਼ਿਲ_ਪੂਰਾ(html_content *html)
{
	ਚੋਣ (content_get_ਸਥਿਤੀ_2(&html->ਅਧਾਰ)) {
	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_READY:
		ਜੇ (html->ਅਧਾਰ.ਸਰਗਰਮ == 0) {
			content_ਸੈੱਟ_ਪੂਰਾ(&html->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_OK;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_ਪੂਰਾ:
		/* fallthrough */
	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_LOADING:
		ਵਾਪਸ NSERROR_OK;
	ਮੂਲ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ਰੋਕੋ;
	}
	ਵਾਪਸ NSERROR_UNKNOWN;
}


ਸਥਿਰ ਖਾਲੀ html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	ਬਿਨਾਨਿਸ਼ਾਨੀ f_ਮਾਪ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ f_min;
	ਬਿਨਾਨਿਸ਼ਾਨੀ w;
	ਬਿਨਾਨਿਸ਼ਾਨੀ h;
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ = {
		.getdims = {
			.viewport_ਚੌੜਾਈ = &w,
			.viewport_ਉਚਾਈ = &h,
		},
	};

	content_ਪ੍ਰਸਾਰਣ(&htmlc->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_GETDIMS, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.ਚੌੜਾਈ  = w;
	htmlc->media.ਉਚਾਈ = h;
	htmlc->unit_len_ctx.viewport_ਚੌੜਾਈ  = w;
	htmlc->unit_len_ctx.viewport_ਉਚਾਈ = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_ਮਾਪ = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_ਮਾਪ)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_ਮਾਪ)), F_10)), F_72);

	htmlc->unit_len_ctx.font_ਮਾਪ_default = f_ਮਾਪ;
	htmlc->unit_len_ctx.font_ਮਾਪ_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
ਖਾਲੀ html_ਅੰਤ_conversion(html_content *htmlc)
{
	ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਸੁਨੇਹਾ_ਅੰਕੜੇ;
	dom_exception exc; /* returned by libdom functions */
	dom_ਗੰਢ *html;
	nserror error;

	/* Bail out if we've been aborted */
	ਜੇ (htmlc->aborted) {
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, NSERROR_STOPPED, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&htmlc->ਅਧਾਰ);
		ਵਾਪਸ;
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
	ਜੇ (htmlc->select_ctx != ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		ਵਾਪਸ;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ਜੇ (error != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, error, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&htmlc->ਅਧਾਰ);
		ਵਾਪਸ;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ਜੇ (htmlc->jsthread != ਨਿਰਾ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ਨਿਰਾ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_ਸੈੱਟ_ਸਥਿਤੀ(&htmlc->ਅਧਾਰ, messages_get("Processing"));
	ਸੁਨੇਹਾ_ਅੰਕੜੇ.explicit_ਸਥਿਤੀ_ਪਾਠ = ਨਿਰਾ;
	content_ਪ੍ਰਸਾਰਣ(&htmlc->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_ਸਥਿਤੀ, &ਸੁਨੇਹਾ_ਅੰਕੜੇ);

	exc = dom_document_get_document_element(htmlc->document, (ਖਾਲੀ *) &html);
	ਜੇ ((exc != DOM_NO_ERR) || (html == ਨਿਰਾ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, NSERROR_DOM, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&htmlc->ਅਧਾਰ);
		ਵਾਪਸ;
	}

	html_get_dimensions(htmlc);

	error = dom_ਮੰਜ਼ਿਲ_box(html, htmlc, html_box_convert_ਪੂਰਾ, &htmlc->box_conversion_context);
	ਜੇ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_ਗੰਢ_unref(html);
		html_object_free_objects(htmlc);
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, error, ਨਿਰਾ);
		content_ਸੈੱਟ_error(&htmlc->ਅਧਾਰ);
		ਵਾਪਸ;
	}

	dom_ਗੰਢ_unref(html);
}


ਸਥਿਰ ਖਾਲੀ
html_document_user_ਅੰਕੜੇ_handler(dom_ਗੰਢ_operation operation,
				dom_string *ਕੁੰਜੀ, ਖਾਲੀ *ਅੰਕੜੇ,
				ਬਣਤਰ dom_ਗੰਢ *src,
				ਬਣਤਰ dom_ਗੰਢ *dst)
{
	ਜੇ (dom_string_isequal(corestring_dom_ns_ਕੁੰਜੀ_html_content_ਅੰਕੜੇ,
			       ਕੁੰਜੀ) == false || ਅੰਕੜੇ == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ਚੋਣ (operation) {
	ਹਾਲਤ DOM_ਗੰਢ_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ਰੋਕੋ;
	ਹਾਲਤ DOM_ਗੰਢ_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ਰੋਕੋ;
	ਹਾਲਤ DOM_ਗੰਢ_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ਰੋਕੋ;
	ਹਾਲਤ DOM_ਗੰਢ_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ਰੋਕੋ;
	ਹਾਲਤ DOM_ਗੰਢ_DELETED:
		/* This is the only path I expect */
		ਰੋਕੋ;
	ਮੂਲ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
	}
}


ਸਥਿਰ nserror
html_create_html_ਅੰਕੜੇ(html_content *c, ਅਚਲ http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	ਖਾਲੀ *old_ਗੰਢ_ਅੰਕੜੇ;
	ਅਚਲ ਅੱਖਰ *prefer_color_ਬਹੁਲਕ = (nsoption_bool(prefer_dark_ਬਹੁਲਕ)) ?
			"dark" : "light";

	c->parser = ਨਿਰਾ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ਨਿਰਾ;
	c->quirks = DOM_DOCUMENT_QUIRKS_ਬਹੁਲਕ_NONE;
	c->encoding = ਨਿਰਾ;
	c->ਅਧਾਰ_url = nsurl_ref(content_get_url(&c->ਅਧਾਰ));
	c->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ਨਿਰਾ;
	c->bctx = ਨਿਰਾ;
	c->layout = ਨਿਰਾ;
	c->background_ਰੰਗ = NS_TRANSPARENT;
	c->stylesheet_ਗਿਣਤੀ = 0;
	c->stylesheets = ਨਿਰਾ;
	c->select_ctx = ਨਿਰਾ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ਨਿਰਾ;
	c->ਅੰਕ_objects = 0;
	c->object_list = ਨਿਰਾ;
	c->forms = ਨਿਰਾ;
	c->imagemaps = ਨਿਰਾ;
	c->bw = ਨਿਰਾ;
	c->frameset = ਨਿਰਾ;
	c->iframe = ਨਿਰਾ;
	c->ਪੰਨਾ = ਨਿਰਾ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_ਗਿਣਤੀ = 0;
	c->scripts = ਨਿਰਾ;
	c->jsthread = ਨਿਰਾ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->ਅਧਾਰ.ਸਰਗਰਮ = 1; /* The html content itself is active */

	ਜੇ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (lwc_intern_string(prefer_color_ਬਹੁਲਕ, ਲੜੀ_ਲੰਬਾਈ(prefer_color_ਬਹੁਲਕ),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ਨਿਰਾ;
		ਵਾਪਸ NSERROR_NOMEM;
	}

	c->sel = selection_create((ਬਣਤਰ content *)c);

	nerror = http_parameter_list_ਲੱਭੋ_ਵਸਤੂ(params, corestring_lwc_charset, &charset);
	ਜੇ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_ਅੰਕੜੇ(charset));

		lwc_string_unref(charset);

		ਜੇ (c->encoding == ਨਿਰਾ) {
			lwc_string_unref(c->universal);
			c->universal = ਨਿਰਾ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ਨਿਰਾ;
			ਵਾਪਸ NSERROR_NOMEM;

		}
		c->encoding_ਸਰੋਤ = DOM_HUBBUB_ENCODING_ਸਰੋਤ_ਸਿਰਲੇਖ;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.ਸੁਨੇਹਾ = ਨਿਰਾ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ਜੇ ((error != DOM_HUBBUB_OK) && (c->encoding != ਨਿਰਾ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		ਮੁਕਤ(c->encoding);
		c->encoding = ਨਿਰਾ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ਜੇ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->ਅਧਾਰ_url);
		c->ਅਧਾਰ_url = ਨਿਰਾ;

		lwc_string_unref(c->universal);
		c->universal = ਨਿਰਾ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ਨਿਰਾ;

		ਵਾਪਸ libdom_hubbub_error_ਮੰਜ਼ਿਲ_nserror(error);
	}

	err = dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(c->document,
				     corestring_dom_ns_ਕੁੰਜੀ_html_content_ਅੰਕੜੇ,
				     c, html_document_user_ਅੰਕੜੇ_handler,
				     (ਖਾਲੀ *) &old_ਗੰਢ_ਅੰਕੜੇ);
	ਜੇ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ਨਿਰਾ;
		nsurl_unref(c->ਅਧਾਰ_url);
		c->ਅਧਾਰ_url = ਨਿਰਾ;

		lwc_string_unref(c->universal);
		c->universal = ਨਿਰਾ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ਨਿਰਾ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		ਵਾਪਸ NSERROR_DOM;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(old_ਗੰਢ_ਅੰਕੜੇ == ਨਿਰਾ);

	ਵਾਪਸ NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

ਸਥਿਰ nserror
html_create(ਅਚਲ content_handler *handler,
	    lwc_string *imime_type,
	    ਅਚਲ http_parameter *params,
	    llcache_handle *llcache,
	    ਅਚਲ ਅੱਖਰ *fallback_charset,
	    bool quirks,
	    ਬਣਤਰ content **c)
{
	html_content *html;
	nserror error;

	html = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(html_content));
	ਜੇ (html == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&html->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(html);
		ਵਾਪਸ error;
	}

	error = html_create_html_ਅੰਕੜੇ(html, params);
	ਜੇ (error != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(&html->ਅਧਾਰ, error, ਨਿਰਾ);
		ਮੁਕਤ(html);
		ਵਾਪਸ error;
	}

	error = html_css_new_stylesheets(html);
	ਜੇ (error != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(&html->ਅਧਾਰ, error, ਨਿਰਾ);
		ਮੁਕਤ(html);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) html;

	ਵਾਪਸ NSERROR_OK;
}



ਸਥਿਰ nserror
html_process_encoding_change(ਬਣਤਰ content *c,
			     ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	ਅਚਲ ਅੱਖਰ *encoding;
	ਅਚਲ uint8_t *ਸਰੋਤ_ਅੰਕੜੇ;
	size_t ਸਰੋਤ_ਮਾਪ;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_ਸਰੋਤ);
	ਜੇ (encoding == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਜੇ (html->encoding != ਨਿਰਾ) {
		ਮੁਕਤ(html->encoding);
		html->encoding = ਨਿਰਾ;
	}

	html->encoding = strdup(encoding);
	ਜੇ (html->encoding == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ਨਿਰਾ;

	ਜੇ (html->document != ਨਿਰਾ) {
		dom_ਗੰਢ_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.ਸੁਨੇਹਾ = ਨਿਰਾ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ਜੇ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		ਮੁਕਤ(html->encoding);
		html->encoding = strdup("Windows-1252");
		ਜੇ (html->encoding == ਨਿਰਾ) {
			ਵਾਪਸ NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ਜੇ (error != DOM_HUBBUB_OK) {
			ਵਾਪਸ libdom_hubbub_error_ਮੰਜ਼ਿਲ_nserror(error);
		}

	}

	ਸਰੋਤ_ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਸਰੋਤ_ਮਾਪ);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      ਸਰੋਤ_ਅੰਕੜੇ,
					      ਸਰੋਤ_ਮਾਪ);

	ਵਾਪਸ libdom_hubbub_error_ਮੰਜ਼ਿਲ_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

ਸਥਿਰ bool
html_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (ਅਚਲ uint8_t *) ਅੰਕੜੇ,
					      ਮਾਪ);

	err = libdom_hubbub_error_ਮੰਜ਼ਿਲ_nserror(dom_ret);

	/* deal with encoding change */
	ਜੇ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, ਅੰਕੜੇ, ਮਾਪ);
	}

	/* broadcast the error if necessary */
	ਜੇ (err != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(c, err, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_convert(ਬਣਤਰ content *c)
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
	exc = dom_document_get_quirks_ਬਹੁਲਕ(htmlc->document, &htmlc->quirks);
	ਜੇ (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->ਅਧਾਰ.ਸਰਗਰਮ--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->ਅਧਾਰ.ਸਰਗਰਮ, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	ਜੇ (html_can_begin_conversion(htmlc)) {
		ਵਾਪਸ html_begin_conversion(htmlc);
	}
	ਵਾਪਸ true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	/* Cannot begin conversion if we're still fetching stuff */
	ਜੇ (htmlc->ਅਧਾਰ.ਸਰਗਰਮ != 0)
		ਵਾਪਸ false;

	ਲਈ (i = 0; i != htmlc->stylesheet_ਗਿਣਤੀ; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ਜੇ (htmlc->stylesheets[i].modified)
			ਵਾਪਸ false;
	}

	/* All is good, begin */
	ਵਾਪਸ true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_ਗੰਢ *html;
	nserror ns_error;
	ਬਣਤਰ form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *ਗੰਢ_name = ਨਿਰਾ;
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
	ਜੇ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ਜੇ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->ਅਧਾਰ.ਸਰਗਰਮ > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			ਵਾਪਸ true;
		}
		ਜੇ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ,
						libdom_hubbub_error_ਮੰਜ਼ਿਲ_nserror(error),
						ਨਿਰਾ);

			ਵਾਪਸ false;
		}
		htmlc->parse_completed = true;
	}

	ਜੇ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		ਵਾਪਸ true;
	}

	/* Give up processing if we've been aborted */
	ਜੇ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->ਅਧਾਰ.ਸਰਗਰਮ);
		content_ਸੈੱਟ_error(&htmlc->ਅਧਾਰ);
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, NSERROR_STOPPED, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ਜੇ (htmlc->encoding == ਨਿਰਾ) {
		ਅਚਲ ਅੱਖਰ *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_ਸਰੋਤ);
		ਜੇ (encoding == ਨਿਰਾ) {
			content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ,
						NSERROR_NOMEM,
						ਨਿਰਾ);
			ਵਾਪਸ false;
		}

		htmlc->encoding = strdup(encoding);
		ਜੇ (htmlc->encoding == ਨਿਰਾ) {
			content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ,
						NSERROR_NOMEM,
						ਨਿਰਾ);
			ਵਾਪਸ false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (ਖਾਲੀ *) &html);
	ਜੇ ((exc != DOM_NO_ERR) || (html == ਨਿਰਾ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, NSERROR_DOM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	exc = dom_ਗੰਢ_get_ਗੰਢ_name(html, &ਗੰਢ_name);
	ਜੇ ((exc != DOM_NO_ERR) ||
	    (ਗੰਢ_name == ਨਿਰਾ) ||
	    (!dom_string_caseless_lwc_isequal(ਗੰਢ_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, NSERROR_DOM, ਨਿਰਾ);
		dom_ਗੰਢ_unref(html);
		ਵਾਪਸ false;
	}
	dom_string_unref(ਗੰਢ_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	ਲਈ (f = htmlc->forms; f != ਨਿਰਾ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ਜੇ (f->action == ਨਿਰਾ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->ਅਧਾਰ);
			ns_error = nsurl_join(htmlc->ਅਧਾਰ_url,
					      nsurl_access(doc_addr),
					      &action);
		} ਨਹੀਂਤਾਂ {
			ns_error = nsurl_join(htmlc->ਅਧਾਰ_url,
					      f->action,
					      &action);
		}

		ਜੇ (ns_error != NSERROR_OK) {
			content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ, ns_error, ਨਿਰਾ);

			dom_ਗੰਢ_unref(html);
			ਵਾਪਸ false;
		}

		ਮੁਕਤ(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ਜੇ (f->action == ਨਿਰਾ) {
			content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ,
						NSERROR_NOMEM,
						ਨਿਰਾ);

			dom_ਗੰਢ_unref(html);
			ਵਾਪਸ false;
		}

		/* Ensure each form has a document encoding */
		ਜੇ (f->document_charset == ਨਿਰਾ) {
			f->document_charset = strdup(htmlc->encoding);
			ਜੇ (f->document_charset == ਨਿਰਾ) {
				content_ਪ੍ਰਸਾਰਣ_error(&htmlc->ਅਧਾਰ,
							NSERROR_NOMEM,
							ਨਿਰਾ);
				dom_ਗੰਢ_unref(html);
				ਵਾਪਸ false;
			}
		}
	}

	dom_ਗੰਢ_unref(html);

	ਜੇ (htmlc->ਅਧਾਰ.ਸਰਗਰਮ == 0) {
		html_ਅੰਤ_conversion(htmlc);
	}

	ਵਾਪਸ true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

ਸਥਿਰ ਖਾਲੀ html_stop(ਬਣਤਰ content *c)
{
	html_content *htmlc = (html_content *) c;

	ਚੋਣ (c->ਸਥਿਤੀ) {
	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ਜੇ (htmlc->jsthread != ਨਿਰਾ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ਜੇ (c->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY && c->ਸਰਗਰਮ == 0) {
			content_ਸੈੱਟ_ਪੂਰਾ(c);
		}

		ਰੋਕੋ;

	ਹਾਲਤ CONTENT_ਸਥਿਤੀ_ਪੂਰਾ:
		/* Nothing to do */
		ਰੋਕੋ;

	ਮੂਲ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->ਸਥਿਤੀ,
		      c);
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

ਸਥਿਰ ਖਾਲੀ html_reformat(ਬਣਤਰ content *c, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	html_content *htmlc = (html_content *) c;
	ਬਣਤਰ box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_ਚੌੜਾਈ = css_unit_device2css_px(
			INTTOFIX(ਚੌੜਾਈ), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_ਉਚਾਈ = css_unit_device2css_px(
			INTTOFIX(ਉਚਾਈ), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.ਜੜ੍ਹ_style = htmlc->layout->style;

	layout_document(htmlc, ਚੌੜਾਈ, ਉਚਾਈ);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->ਚੌੜਾਈ = layout->x + layout->padding[ਖੱਬਾ_2] + layout->ਚੌੜਾਈ +
		layout->padding[ਸੱਜਾ_2] + layout->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
		layout->margin[ਸੱਜਾ_2];
	c->ਉਚਾਈ = layout->y + layout->padding[ਸਿਖਰ_2] + layout->ਉਚਾਈ +
		layout->padding[BOTTOM] + layout->border[BOTTOM].ਚੌੜਾਈ +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ਜੇ (c->ਚੌੜਾਈ < layout->x + layout->descendant_x1)
		c->ਚੌੜਾਈ = layout->x + layout->descendant_x1;
	ਜੇ (c->ਉਚਾਈ < layout->y + layout->descendant_y1)
		c->ਉਚਾਈ = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ਜੇ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_ਸਮਾਂ = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

ਖਾਲੀ html_redraw_a_box(hlcache_handle *h, ਬਣਤਰ box *box)
{
	ਪੂਰਨ x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2],
			box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

ਖਾਲੀ html__redraw_a_box(ਬਣਤਰ html_content *html, ਬਣਤਰ box *box)
{
	ਪੂਰਨ x, y;

	box_coords(box, &x, &y);

	content__request_redraw((ਬਣਤਰ content *)html, x, y,
			box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2],
			box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM]);
}

ਸਥਿਰ ਖਾਲੀ html_destroy_frameset(ਬਣਤਰ content_html_ਚੌਖਟੇ *frameset)
{
	ਪੂਰਨ i;

	ਜੇ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ਨਿਰਾ;
	}
	ਜੇ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ਨਿਰਾ;
	}
	ਜੇ (frameset->children) {
		ਲਈ (i = 0; i < (frameset->ਪੰਕਤੀਆਂ * frameset->ਖਾਨੇ_ਗਿਣਤੀ); i++) {
			ਜੇ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ਨਿਰਾ;
			}
			ਜੇ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ਨਿਰਾ;
			}
			ਜੇ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ਨਿਰਾ;
	}
}

ਸਥਿਰ ਖਾਲੀ html_destroy_iframe(ਬਣਤਰ content_html_iframe *iframe)
{
	ਬਣਤਰ content_html_iframe *ਅਗਲਾ;
	ਅਗਲਾ = iframe;
	ਜਦੋਂ ((iframe = ਅਗਲਾ) != ਨਿਰਾ) {
		ਅਗਲਾ = iframe->ਅਗਲਾ;
		ਜੇ (iframe->name)
			talloc_free(iframe->name);
		ਜੇ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ਨਿਰਾ;
		}
		talloc_free(iframe);
	}
}


ਸਥਿਰ ਖਾਲੀ html_free_layout(html_content *htmlc)
{
	ਜੇ (htmlc->bctx != ਨਿਰਾ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

ਸਥਿਰ ਖਾਲੀ html_destroy(ਬਣਤਰ content *c)
{
	html_content *html = (html_content *) c;
	ਬਣਤਰ form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ਜੇ (html->box_conversion_context != ਨਿਰਾ) {
		ਜੇ (cancel_dom_ਮੰਜ਼ਿਲ_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	ਲਈ (f = html->forms; f != ਨਿਰਾ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ਜੇ (c->refresh)
		nsurl_unref(c->refresh);

	ਜੇ (html->ਅਧਾਰ_url)
		nsurl_unref(html->ਅਧਾਰ_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ਜੇ (html->jsthread != ਨਿਰਾ) {
		js_destroythread(html->jsthread);
		html->jsthread = ਨਿਰਾ;
	}

	ਜੇ (html->parser != ਨਿਰਾ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ਨਿਰਾ;
	}

	ਜੇ (html->document != ਨਿਰਾ) {
		dom_ਗੰਢ_unref(html->document);
		html->document = ਨਿਰਾ;
	}

	ਜੇ (html->title != ਨਿਰਾ) {
		dom_ਗੰਢ_unref(html->title);
		html->title = ਨਿਰਾ;
	}

	/* Free encoding */
	ਜੇ (html->encoding != ਨਿਰਾ) {
		ਮੁਕਤ(html->encoding);
		html->encoding = ਨਿਰਾ;
	}

	/* Free base target */
	ਜੇ (html->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ != ਨਿਰਾ) {
		ਮੁਕਤ(html->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ);
		html->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;
	}

	/* Free frameset */
	ਜੇ (html->frameset != ਨਿਰਾ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ਨਿਰਾ;
	}

	/* Free iframes */
	ਜੇ (html->iframe != ਨਿਰਾ) {
		html_destroy_iframe(html->iframe);
		html->iframe = ਨਿਰਾ;
	}

	/* Destroy selection context */
	ਜੇ (html->select_ctx != ਨਿਰਾ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ਨਿਰਾ;
	}

	lwc_string_unref(html->universal);
	html->universal = ਨਿਰਾ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ਨਿਰਾ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


ਸਥਿਰ nserror html_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ਪ੍ਰਤਿਜ੍ਞਾ(0 && "html_clone should never be called");

	ਵਾਪਸ true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

ਸਥਿਰ nserror
html_open(ਬਣਤਰ content *c,
	  ਬਣਤਰ browser_ਖਿੜਕੀ *bw,
	  ਬਣਤਰ content *ਪੰਨਾ,
	  ਬਣਤਰ object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->ਪੰਨਾ = (html_content *) ਪੰਨਾ;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

ਸਥਿਰ nserror html_close(ਬਣਤਰ content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ਨਿਰਾ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ਜੇ (htmlc->jsthread != ਨਿਰਾ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	ਵਾਪਸ ret;
}


/**
 * Return an HTML content's selection context
 */

ਸਥਿਰ ਖਾਲੀ html_clear_selection(ਬਣਤਰ content *c)
{
	html_content *html = (html_content *) c;

	ਚੋਣ (html->selection_type) {
	ਹਾਲਤ HTML_SELECTION_NONE:
		/* Nothing to do */
		ਪ੍ਰਤਿਜ੍ਞਾ(html->selection_owner.none == true);
		ਰੋਕੋ;
	ਹਾਲਤ HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->ਅੰਕੜੇ.ਪਾਠ.ta);
		ਰੋਕੋ;
	ਹਾਲਤ HTML_SELECTION_SELF:
		ਪ੍ਰਤਿਜ੍ਞਾ(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ਰੋਕੋ;
	ਹਾਲਤ HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

ਸਥਿਰ ਅੱਖਰ *html_get_selection(ਬਣਤਰ content *c)
{
	html_content *html = (html_content *) c;

	ਚੋਣ (html->selection_type) {
	ਹਾਲਤ HTML_SELECTION_TEXTAREA:
		ਵਾਪਸ textarea_get_selection(html->selection_owner.textarea->
				gadget->ਅੰਕੜੇ.ਪਾਠ.ta);
	ਹਾਲਤ HTML_SELECTION_SELF:
		ਪ੍ਰਤਿਜ੍ਞਾ(html->selection_owner.none == false);
		ਵਾਪਸ selection_get_copy(html->sel);
	ਹਾਲਤ HTML_SELECTION_CONTENT:
		ਵਾਪਸ content_get_selection(
				html->selection_owner.content->object);
	ਹਾਲਤ HTML_SELECTION_NONE:
		/* Nothing to do */
		ਪ੍ਰਤਿਜ੍ਞਾ(html->selection_owner.none == true);
		ਰੋਕੋ;
	ਮੂਲ:
		ਰੋਕੋ;
	}

	ਵਾਪਸ ਨਿਰਾ;
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
ਸਥਿਰ nserror
html_get_contextual_content(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y,
			    ਬਣਤਰ browser_ਖਿੜਕੀ_features *ਅੰਕੜੇ)
{
	html_content *html = (html_content *) c;

	ਬਣਤਰ box *box = html->layout;
	ਬਣਤਰ box *ਅਗਲਾ;
	ਪੂਰਨ box_x = 0, box_y = 0;

	ਜਦੋਂ ((ਅਗਲਾ = box_at_ਬਿੰਦੂ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ਨਿਰਾ) {
		box = ਅਗਲਾ;

		/* hidden boxes are ignored */
		ਜੇ ((box->style != ਨਿਰਾ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			ਜਾਰੀ;
		}

		ਜੇ (box->iframe) {
			ਦਸ਼ਮਲਵ scale = browser_ਖਿੜਕੀ_get_scale(box->iframe);
			browser_ਖਿੜਕੀ_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    ਅੰਕੜੇ);
		}

		ਜੇ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, ਅੰਕੜੇ);

		ਜੇ (box->object)
			ਅੰਕੜੇ->object = box->object;

		ਜੇ (box->href) {
			ਅੰਕੜੇ->link = box->href;
			ਅੰਕੜੇ->link_title = box->ਪਾਠ;
			ਅੰਕੜੇ->link_title_ਲੰਮਾਈ = box->ਲੰਮਾਈ;
		}

		ਜੇ (box->usemap) {
			ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ = ਨਿਰਾ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &ਨਿਸ਼ਾਨਾ);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ਜੇ (url != ਨਿਰਾ)
				ਅੰਕੜੇ->link = url;
		}
		ਜੇ (box->gadget) {
			ਚੋਣ (box->gadget->type) {
			ਹਾਲਤ GADGET_TEXTBOX:
			ਹਾਲਤ GADGET_TEXTAREA:
			ਹਾਲਤ GADGET_PASSWORD:
				ਅੰਕੜੇ->form_features = CTX_FORM_ਪਾਠ;
				ਰੋਕੋ;

			ਹਾਲਤ GADGET_FILE:
				ਅੰਕੜੇ->form_features = CTX_FORM_FILE;
				ਰੋਕੋ;

			ਮੂਲ:
				ਅੰਕੜੇ->form_features = CTX_FORM_NONE;
				ਰੋਕੋ;
			}
		}
	}
	ਵਾਪਸ NSERROR_OK;
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
ਸਥਿਰ bool
html_scroll_at_ਬਿੰਦੂ(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ scrx, ਪੂਰਨ scry)
{
	html_content *html = (html_content *) c;

	ਬਣਤਰ box *box = html->layout;
	ਬਣਤਰ box *ਅਗਲਾ;
	ਪੂਰਨ box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	ਜਦੋਂ ((ਅਗਲਾ = box_at_ਬਿੰਦੂ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ਨਿਰਾ) {
		box = ਅਗਲਾ;

		ਜੇ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			ਜਾਰੀ;

		/* Pass into iframe */
		ਜੇ (box->iframe) {
			ਦਸ਼ਮਲਵ scale = browser_ਖਿੜਕੀ_get_scale(box->iframe);

			ਜੇ (browser_ਖਿੜਕੀ_scroll_at_ਬਿੰਦੂ(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				ਵਾਪਸ true;
		}

		/* Pass into textarea widget */
		ਜੇ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta,
						scrx, scry) == true)
			ਵਾਪਸ true;

		/* Pass into object */
		ਜੇ (box->object != ਨਿਰਾ && content_scroll_at_ਬਿੰਦੂ(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			ਵਾਪਸ true;

		/* Handle box scrollbars */
		ਜੇ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ਜੇ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ਜੇ (handled_scroll == true)
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
ਸਥਿਰ ਖਾਲੀ html_dom_user_ਅੰਕੜੇ_handler(dom_ਗੰਢ_operation operation,
		dom_string *ਕੁੰਜੀ, ਖਾਲੀ *ਅੰਕੜੇ_2, ਬਣਤਰ dom_ਗੰਢ *src,
		ਬਣਤਰ dom_ਗੰਢ *dst)
{
	ਅੱਖਰ *oldfile;
	ਅੱਖਰ *ਅੰਕੜੇ = (ਅੱਖਰ *)ਅੰਕੜੇ_2;

	ਜੇ (!dom_string_isequal(corestring_dom_ns_ਕੁੰਜੀ_file_name_ਗੰਢ_ਅੰਕੜੇ,
				ਕੁੰਜੀ) || ਅੰਕੜੇ == ਨਿਰਾ) {
		ਵਾਪਸ;
	}

	ਚੋਣ (operation) {
	ਹਾਲਤ DOM_ਗੰਢ_CLONED:
		ਜੇ (dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ(dst,
					   corestring_dom_ns_ਕੁੰਜੀ_file_name_ਗੰਢ_ਅੰਕੜੇ,
					   strdup(ਅੰਕੜੇ), html_dom_user_ਅੰਕੜੇ_handler,
					   &oldfile) == DOM_NO_ERR) {
			ਜੇ (oldfile != ਨਿਰਾ)
				ਮੁਕਤ(oldfile);
		}
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_RENAMED:
	ਹਾਲਤ DOM_ਗੰਢ_IMPORTED:
	ਹਾਲਤ DOM_ਗੰਢ_ADOPTED:
		ਰੋਕੋ;

	ਹਾਲਤ DOM_ਗੰਢ_DELETED:
		ਮੁਕਤ(ਅੰਕੜੇ);
		ਰੋਕੋ;
	ਮੂਲ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ਪ੍ਰਤਿਜ੍ਞਾ(0);
	}
}

ਸਥਿਰ ਖਾਲੀ html_ਸੈੱਟ_file_gadget_filename(ਬਣਤਰ content *c,
	ਬਣਤਰ form_control *gadget, ਅਚਲ ਅੱਖਰ *fn)
{
	nserror ret;
	ਅੱਖਰ *utf8_fn, *oldfile = ਨਿਰਾ;
	html_content *html = (html_content *)c;
	ਬਣਤਰ box *file_box = gadget->box;

	ret = guit->utf8->local_ਮੰਜ਼ਿਲ_utf8(fn, 0, &utf8_fn);
	ਜੇ (ret != NSERROR_OK) {
		ਪ੍ਰਤਿਜ੍ਞਾ(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		ਵਾਪਸ;
	}

	form_gadget_update_ਮੁੱਲ(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ਜੇ (dom_ਗੰਢ_ਸੈੱਟ_user_ਅੰਕੜੇ((dom_ਗੰਢ *)file_box->gadget->ਗੰਢ,
				   corestring_dom_ns_ਕੁੰਜੀ_file_name_ਗੰਢ_ਅੰਕੜੇ,
				   strdup(fn), html_dom_user_ਅੰਕੜੇ_handler,
				   &oldfile) == DOM_NO_ERR) {
		ਜੇ (oldfile != ਨਿਰਾ)
			ਮੁਕਤ(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

ਖਾਲੀ html_ਸੈੱਟ_file_gadget_filename_2(ਬਣਤਰ hlcache_handle *hl,
	ਬਣਤਰ form_control *gadget, ਅਚਲ ਅੱਖਰ *fn)
{
	ਵਾਪਸ html_ਸੈੱਟ_file_gadget_filename(hlcache_handle_get_content(hl),
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
ਸਥਿਰ bool html_drop_file_at_ਬਿੰਦੂ(ਬਣਤਰ content *c, ਪੂਰਨ x, ਪੂਰਨ y, ਅੱਖਰ *file)
{
	html_content *html = (html_content *) c;

	ਬਣਤਰ box *box = html->layout;
	ਬਣਤਰ box *ਅਗਲਾ;
	ਬਣਤਰ box *file_box = ਨਿਰਾ;
	ਬਣਤਰ box *ਪਾਠ_box = ਨਿਰਾ;
	ਪੂਰਨ box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	ਜਦੋਂ ((ਅਗਲਾ = box_at_ਬਿੰਦੂ(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ਨਿਰਾ) {
		box = ਅਗਲਾ;

		ਜੇ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			ਜਾਰੀ;

		ਜੇ (box->iframe) {
			ਦਸ਼ਮਲਵ scale = browser_ਖਿੜਕੀ_get_scale(box->iframe);
			ਵਾਪਸ browser_ਖਿੜਕੀ_drop_file_at_ਬਿੰਦੂ(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ਜੇ (box->object &&
		    content_drop_file_at_ਬਿੰਦੂ(box->object,
					x - box_x, y - box_y, file) == true)
			ਵਾਪਸ true;

		ਜੇ (box->gadget) {
			ਚੋਣ (box->gadget->type) {
				ਹਾਲਤ GADGET_FILE:
					file_box = box;
				ਰੋਕੋ;

				ਹਾਲਤ GADGET_TEXTBOX:
				ਹਾਲਤ GADGET_TEXTAREA:
				ਹਾਲਤ GADGET_PASSWORD:
					ਪਾਠ_box = box;
					ਰੋਕੋ;

				ਮੂਲ:	/* appease compiler */
					ਰੋਕੋ;
			}
		}
	}

	ਜੇ (!file_box && !ਪਾਠ_box)
		/* No box capable of handling drop */
		ਵਾਪਸ false;

	ਜੇ (file == ਨਿਰਾ)
		/* There is a box capable of handling drop here */
		ਵਾਪਸ true;

	/* Handle the drop */
	ਜੇ (file_box) {
		/* File dropped on file input */
		html_ਸੈੱਟ_file_gadget_filename(c, file_box->gadget, file);

	} ਨਹੀਂਤਾਂ {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ਨਿਰਾ;
		ਅੱਖਰ *ਬਫਰ;
		ਅੱਖਰ *utf8_buff;
		nserror ret;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ;
		ਪੂਰਨ bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ਜੇ (fp == ਨਿਰਾ) {
			/* Couldn't open file, but drop was for us */
			ਵਾਪਸ true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ਜੇ ((ਲੰਮਾ)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			ਵਾਪਸ true;
		}

		/* Allocate buffer for file data */
		ਬਫਰ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(file_len + 1);
		ਜੇ (ਬਫਰ == ਨਿਰਾ) {
			/* No memory, but drop was for us */
			fclose(fp);
			ਵਾਪਸ true;
		}

		/* Stick file into buffer */
		ਜੇ (file_len != fread(ਬਫਰ, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			ਮੁਕਤ(ਬਫਰ);
			fclose(fp);
			ਵਾਪਸ true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		ਬਫਰ[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_ਮੰਜ਼ਿਲ_utf8(ਬਫਰ, file_len, &utf8_buff);
		ਜੇ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ਪ੍ਰਤਿਜ੍ਞਾ(ret != NSERROR_BAD_ENCODING);
			ਮੁਕਤ(ਬਫਰ);
			ਵਾਪਸ true;
		}

		/* Done with buffer */
		ਮੁਕਤ(ਬਫਰ);

		/* Get new length */
		ਮਾਪ = ਲੜੀ_ਲੰਬਾਈ(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(ਪਾਠ_box, &bx, &by);
		textarea_mouse_action(ਪਾਠ_box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_ਪਾਠ(ਪਾਠ_box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta,
				utf8_buff, ਮਾਪ);

		ਮੁਕਤ(utf8_buff);
	}

	ਵਾਪਸ true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
ਸਥਿਰ nserror
html_debug(ਬਣਤਰ content *c, ਗਿਣਤੀ content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
ਸਥਿਰ nserror
html_debug_dump(ਬਣਤਰ content *c, FILE *f, ਗਿਣਤੀ content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_ਗੰਢ *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ਪ੍ਰਤਿਜ੍ਞਾ(htmlc != ਨਿਰਾ);

	ਜੇ (op == CONTENT_DEBUG_RENDER) {
		ਪ੍ਰਤਿਜ੍ਞਾ(htmlc->layout != ਨਿਰਾ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} ਨਹੀਂਤਾਂ {
		ਜੇ (htmlc->document == ਨਿਰਾ) {
			NSLOG(netsurf, INFO, "No document to dump");
			ਵਾਪਸ NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (ਖਾਲੀ *) &html);
		ਜੇ ((exc != DOM_NO_ERR) || (html == ਨਿਰਾ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			ਵਾਪਸ NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_ਗੰਢ_unref(html);
	}

	ਵਾਪਸ ret;
}


#ਜੇ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

ਸਥਿਰ ਖਾਲੀ
html_dump_frameset(ਬਣਤਰ content_html_ਚੌਖਟੇ *ਚੌਖਟਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਡੂੰਘਾਈ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਪੂਰਨ ਪੰਕਤੀ, ਖਾਨਾ, index;
	ਅਚਲ ਅੱਖਰ *unit[] = {"px", "%", "*"};
	ਅਚਲ ਅੱਖਰ *scrolling[] = {"auto", "yes", "no"};

	ਪ੍ਰਤਿਜ੍ਞਾ(ਚੌਖਟਾ);

	fprintf(stderr, "%p ", ਚੌਖਟਾ);

	fprintf(stderr, "(%i %i) ", ਚੌਖਟਾ->ਪੰਕਤੀਆਂ, ਚੌਖਟਾ->ਖਾਨੇ_ਗਿਣਤੀ);

	fprintf(stderr, "w%g%s ", ਚੌਖਟਾ->ਚੌੜਾਈ.ਮੁੱਲ, unit[ਚੌਖਟਾ->ਚੌੜਾਈ.unit]);
	fprintf(stderr, "h%g%s ", ਚੌਖਟਾ->ਉਚਾਈ.ਮੁੱਲ,unit[ਚੌਖਟਾ->ਉਚਾਈ.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			ਚੌਖਟਾ->margin_ਚੌੜਾਈ, ਚੌਖਟਾ->margin_ਉਚਾਈ);

	ਜੇ (ਚੌਖਟਾ->name)
		fprintf(stderr, "'%s' ", ਚੌਖਟਾ->name);
	ਜੇ (ਚੌਖਟਾ->url)
		fprintf(stderr, "<%s> ", ਚੌਖਟਾ->url);

	ਜੇ (ਚੌਖਟਾ->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[ਚੌਖਟਾ->scrolling]);
	ਜੇ (ਚੌਖਟਾ->border)
		fprintf(stderr, "border %x ",
				(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) ਚੌਖਟਾ->border_ਰੰਗ);

	fprintf(stderr, "\n");

	ਜੇ (ਚੌਖਟਾ->children) {
		ਲਈ (ਪੰਕਤੀ = 0; ਪੰਕਤੀ != ਚੌਖਟਾ->ਪੰਕਤੀਆਂ; ਪੰਕਤੀ++) {
			ਲਈ (ਖਾਨਾ = 0; ਖਾਨਾ != ਚੌਖਟਾ->ਖਾਨੇ_ਗਿਣਤੀ; ਖਾਨਾ++) {
				ਲਈ (i = 0; i != ਡੂੰਘਾਈ; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", ਪੰਕਤੀ, ਖਾਨਾ);
				index = (ਪੰਕਤੀ * ਚੌਖਟਾ->ਖਾਨੇ_ਗਿਣਤੀ) + ਖਾਨਾ;
				html_dump_frameset(&ਚੌਖਟਾ->children[index],
						ਡੂੰਘਾਈ + 1);
			}
		}
	}
}

#ਜੇ_ਅੰਤ

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->document;
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
ਬਣਤਰ box *html_get_box_ਰੁੱਖ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *html_encoding(ਅਚਲ ਬਣਤਰ content *c, ਗਿਣਤੀ content_encoding_type op)
{
	html_content *html = (html_content *) c;
	ਸਥਿਰ ਅੱਖਰ enc_token[10] = "Encoding0";

	ਪ੍ਰਤਿਜ੍ਞਾ(html != ਨਿਰਾ);

	ਜੇ (op == CONTENT_ENCODING_ਸਰੋਤ) {
		enc_token[8] = '0' + html->encoding_ਸਰੋਤ;
		ਵਾਪਸ messages_get(enc_token);
	}

	ਵਾਪਸ html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
ਬਣਤਰ content_html_ਚੌਖਟੇ *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
ਬਣਤਰ content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_ਅਧਾਰ_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->ਅਧਾਰ_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
ਅਚਲ ਅੱਖਰ *html_get_ਅਧਾਰ_ਨਿਸ਼ਾਨਾ(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ਪ੍ਰਤਿਜ੍ਞਾ(c != ਨਿਰਾ);

	ਵਾਪਸ c->ਅਧਾਰ_ਨਿਸ਼ਾਨਾ;
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
bool html_get_id_ਵਿੱਥ(hlcache_handle *h, lwc_string *frag_id, ਪੂਰਨ *x, ਪੂਰਨ *y)
{
	ਬਣਤਰ box *pos;
	ਬਣਤਰ box *layout;

	ਜੇ (content_get_type(h) != CONTENT_HTML)
		ਵਾਪਸ false;

	layout = html_get_box_ਰੁੱਖ(h);

	ਜੇ ((pos = box_ਲੱਭੋ_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		ਵਾਪਸ true;
	}
	ਵਾਪਸ false;
}

bool html_exec(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool ਨਤੀਜਾ = false;
	dom_exception err;
	dom_html_body_element *body_ਗੰਢ;
	dom_string *dom_src;
	dom_ਪਾਠ *ਪਾਠ_ਗੰਢ;
	dom_ਗੰਢ *ਵਾਧੂ_ਗੰਢ;
	dom_html_script_element *script_ਗੰਢ;

	ਜੇ (htmlc->document == ਨਿਰਾ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		ਜਾਓ ਬਾਹਰਲਾ_no_string;
	}

	err = dom_string_create((ਅਚਲ uint8_t *)src, srclen, &dom_src);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		ਜਾਓ ਬਾਹਰਲਾ_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_ਗੰਢ);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		ਜਾਓ ਬਾਹਰਲਾ_no_body;
	}

	err = dom_document_create_ਪਾਠ_ਗੰਢ(htmlc->document, dom_src, &ਪਾਠ_ਗੰਢ);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		ਜਾਓ ਬਾਹਰਲਾ_no_ਪਾਠ_ਗੰਢ;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_ਗੰਢ);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		ਜਾਓ ਬਾਹਰਲਾ_no_script_ਗੰਢ;
	}

	err = dom_ਗੰਢ_append_ਬੱਚਾ(script_ਗੰਢ, ਪਾਠ_ਗੰਢ, &ਵਾਧੂ_ਗੰਢ);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		ਜਾਓ ਬਾਹਰਲਾ_unparented;
	}
	dom_ਗੰਢ_unref(ਵਾਧੂ_ਗੰਢ); /* We do not need the spare ref at all */

	err = dom_ਗੰਢ_append_ਬੱਚਾ(body_ਗੰਢ, script_ਗੰਢ, &ਵਾਧੂ_ਗੰਢ);
	ਜੇ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		ਜਾਓ ਬਾਹਰਲਾ_unparented;
	}
	dom_ਗੰਢ_unref(ਵਾਧੂ_ਗੰਢ); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	ਨਤੀਜਾ = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(script_ਗੰਢ, &ਵਾਧੂ_ਗੰਢ);
	ਜੇ (err == DOM_NO_ERR && ਵਾਧੂ_ਗੰਢ != ਨਿਰਾ) {
		dom_ਗੰਢ *ਦੂਜਾ_ਵਾਧੂ;
		err = dom_ਗੰਢ_remove_ਬੱਚਾ(ਵਾਧੂ_ਗੰਢ, script_ਗੰਢ, &ਦੂਜਾ_ਵਾਧੂ);
		ਜੇ (err == DOM_NO_ERR) {
			dom_ਗੰਢ_unref(ਦੂਜਾ_ਵਾਧੂ);
		}
		dom_ਗੰਢ_unref(ਵਾਧੂ_ਗੰਢ);
	}

ਬਾਹਰਲਾ_unparented:
	dom_ਗੰਢ_unref(script_ਗੰਢ);
ਬਾਹਰਲਾ_no_script_ਗੰਢ:
	dom_ਗੰਢ_unref(ਪਾਠ_ਗੰਢ);
ਬਾਹਰਲਾ_no_ਪਾਠ_ਗੰਢ:
	dom_ਗੰਢ_unref(body_ਗੰਢ);
ਬਾਹਰਲਾ_no_body:
	dom_string_unref(dom_src);
ਬਾਹਰਲਾ_no_string:
	ਵਾਪਸ ਨਤੀਜਾ;
}

/* See \ref content_saw_insecure_objects */
ਸਥਿਰ bool
html_saw_insecure_objects(ਬਣਤਰ content *c)
{
	html_content *htmlc = (html_content *)c;
	ਬਣਤਰ content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	ਜਦੋਂ (obj != ਨਿਰਾ) {
		ਜੇ (obj->content != ਨਿਰਾ) {
			ਜੇ (content_saw_insecure_objects(obj->content))
				ਵਾਪਸ true;
		}
		obj = obj->ਅਗਲਾ;
	}

	/* Now check the script list */
	ਜੇ (html_saw_insecure_scripts(htmlc)) {
		ਵਾਪਸ true;
	}

	/* Now check stylesheets */
	ਜੇ (html_css_saw_insecure_stylesheets(htmlc)) {
		ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
ਸਥਿਰ content_type html_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_HTML;
}


ਸਥਿਰ ਖਾਲੀ html_fini(ਖਾਲੀ)
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
ਸਥਿਰ nserror
ਲੱਭੋ_occurrences_html_box(ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ,
			  ਪੂਰਨ p_len,
			  ਬਣਤਰ box *cur,
			  bool case_sens,
			  ਬਣਤਰ textsearch_context *context)
{
	ਬਣਤਰ box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ਜੇ (!cur->object && cur->ਪਾਠ) {
		ਅਚਲ ਅੱਖਰ *ਪਾਠ = cur->ਪਾਠ;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾਈ = cur->ਲੰਮਾਈ;

		ਜਦੋਂ (ਲੰਮਾਈ > 0) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ_ਲੰਮਾਈ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੇਲ_ਵਿੱਥ;
			ਅਚਲ ਅੱਖਰ *new_ਪਾਠ;
			ਅਚਲ ਅੱਖਰ *pos;

			pos = content_textsearch_ਲੱਭੋ_ਨਮੂਨਾ(ਪਾਠ,
					   ਲੰਮਾਈ,
					   ਨਮੂਨਾ,
					   p_len,
					   case_sens,
					   &ਮੇਲ_ਲੰਮਾਈ);
			ਜੇ (!pos)
				ਰੋਕੋ;

			/* found string in box => add to list */
			ਮੇਲ_ਵਿੱਥ = pos - cur->ਪਾਠ;

			res = content_textsearch_ਜੋੜੋ_ਮੇਲ(context,
					cur->byte_ਵਿੱਥ + ਮੇਲ_ਵਿੱਥ,
					cur->byte_ਵਿੱਥ + ਮੇਲ_ਵਿੱਥ + ਮੇਲ_ਲੰਮਾਈ,
					cur,
					cur);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}

			new_ਪਾਠ = pos + ਮੇਲ_ਲੰਮਾਈ;
			ਲੰਮਾਈ -= (new_ਪਾਠ - ਪਾਠ);
			ਪਾਠ = new_ਪਾਠ;
		}
	}

	/* and recurse */
	ਲਈ (a = cur->children; a; a = a->ਅਗਲਾ) {
		res = ਲੱਭੋ_occurrences_html_box(ਨਮੂਨਾ,
						p_len,
						a,
						case_sens,
						context);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ res;
		}
	}

	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
html_textsearch_ਲੱਭੋ(ਬਣਤਰ content *c,
		     ਬਣਤਰ textsearch_context *context,
		     ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ,
		     ਪੂਰਨ p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ਜੇ (html->layout == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_INVALID;
	}

	ਵਾਪਸ ਲੱਭੋ_occurrences_html_box(ਨਮੂਨਾ,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


ਸਥਿਰ nserror
html_textsearch_bounds(ਬਣਤਰ content *c,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx,
		       ਬਣਤਰ box *ਸ਼ੁਰੂ_box,
		       ਬਣਤਰ box *ਸਿਰਾ_box,
		       ਬਣਤਰ rect *bounds)
{
	/* get box position and jump to it */
	box_coords(ਸ਼ੁਰੂ_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(ਸਿਰਾ_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += ਸਿਰਾ_box->ਚੌੜਾਈ;
	bounds->y1 += ਸਿਰਾ_box->ਉਚਾਈ;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * HTML content handler function table
 */
ਸਥਿਰ ਅਚਲ content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_ਅੰਕੜੇ = html_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = html_convert,
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
	.scroll_at_ਬਿੰਦੂ = html_scroll_at_ਬਿੰਦੂ,
	.drop_file_at_ਬਿੰਦੂ = html_drop_file_at_ਬਿੰਦੂ,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_ਲੱਭੋ = html_textsearch_ਲੱਭੋ,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_ਸਿਰਾ = html_textselection_get_ਸਿਰਾ,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(ਖਾਲੀ)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ਜੇ (error != NSERROR_OK)
		ਜਾਓ error;

	ਲਈ (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_ਰਜਿਸਟਰ_handler(html_types[i],
				&html_content_handler);
		ਜੇ (error != NSERROR_OK)
			ਜਾਓ error;
	}

	ਵਾਪਸ NSERROR_OK;

error:
	html_fini();

	ਵਾਪਸ error;
}
