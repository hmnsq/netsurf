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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <nsutils/time.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/config.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/libdom.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/utf8.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/string.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/utf8.h"
#అన్తర్భావ "netsurf/keypress.h"
#అన్తర్భావ "netsurf/layout.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/textsearch.h"
#అన్తర్భావ "desktop/selection.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/textarea.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "javascript/js.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/dom_event.h"
#అన్తర్భావ "html/css.h"
#అన్తర్భావ "html/object.h"
#అన్తర్భావ "html/html_save.h"
#అన్తర్భావ "html/interaction.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_construct.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/imagemap.h"
#అన్తర్భావ "html/layout.h"
#అన్తర్భావ "html/textselection.h"

#నిర్వచించు CHUNK 4096

/* Change these to 1 to cause a dump to stderr of the frameset or box
 * when the trees have been built.
 */
#నిర్వచించు ALWAYS_DUMP_FRAMESET 0
#నిర్వచించు ALWAYS_DUMP_BOX 0

స్థిర స్థిరరాశి అక్షరం *html_types[] = {
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
స్థిర bool fire_dom_event(dom_event *event, dom_కణుపు *లక్ష్యం)
{
	dom_exception exc;
	bool ఫలితం;

	exc = dom_event_లక్ష్యం_dispatch_event(లక్ష్యం, event, &ఫలితం);
	ఒకవేళ (exc != DOM_NO_ERR) {
		తిరిగి false;
	}

	తిరిగి ఫలితం;
}

/* Exported interface, see html_internal.h */
bool fire_generic_dom_event(dom_string *type, dom_కణుపు *లక్ష్యం,
		bool bubbles, bool cancelable)
{
	dom_exception exc;
	dom_event *evt;
	bool ఫలితం;

	exc = dom_event_create(&evt);
	ఒకవేళ (exc != DOM_NO_ERR) తిరిగి false;
	exc = dom_event_init(evt, type, bubbles, cancelable);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		తిరిగి false;
	}
	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
	      (పూర్ణాంకం)dom_string_నిడివి(type), dom_string_దత్తాంశం(type), లక్ష్యం);
	ఫలితం = fire_dom_event(evt, లక్ష్యం);
	dom_event_unref(evt);
	తిరిగి ఫలితం;
}

/* Exported interface, see html_internal.h */
bool fire_dom_keyboard_event(dom_string *type, dom_కణుపు *లక్ష్యం,
		bool bubbles, bool cancelable, uint32_t కీలకం)
{
	bool is_special = కీలకం <= 0x001F || (0x007F <= కీలకం && కీలకం <= 0x009F);
	dom_string *dom_కీలకం = ఖాళీ;
	dom_keyboard_event *evt;
	dom_exception exc;
	bool ఫలితం;

	ఒకవేళ (is_special) {
		ఎంపిక (కీలకం) {
		స్థితి NS_కీలకం_ESCAPE:
			dom_కీలకం = dom_string_ref(corestring_dom_Escape);
			ఆపు;
		స్థితి NS_కీలకం_ఎడమ:
			dom_కీలకం = dom_string_ref(corestring_dom_Arrow_ఎడమ);
			ఆపు;
		స్థితి NS_కీలకం_కుడి:
			dom_కీలకం = dom_string_ref(corestring_dom_Arrow_కుడి);
			ఆపు;
		స్థితి NS_కీలకం_పైకి:
			dom_కీలకం = dom_string_ref(corestring_dom_Arrow_పైకి);
			ఆపు;
		స్థితి NS_కీలకం_కిందికి:
			dom_కీలకం = dom_string_ref(corestring_dom_Arrow_కిందికి);
			ఆపు;
		స్థితి NS_కీలకం_పుట_పైకి:
			dom_కీలకం = dom_string_ref(corestring_dom_పుట_పైకి);
			ఆపు;
		స్థితి NS_కీలకం_పుట_కిందికి:
			dom_కీలకం = dom_string_ref(corestring_dom_పుట_కిందికి);
			ఆపు;
		స్థితి NS_కీలకం_పాఠ్యం_ప్రారంభం:
			dom_కీలకం = dom_string_ref(corestring_dom_Home);
			ఆపు;
		స్థితి NS_కీలకం_పాఠ్యం_చివర:
			dom_కీలకం = dom_string_ref(corestring_dom_చివర);
			ఆపు;
		అప్రమేయ:
			dom_కీలకం = ఖాళీ;
			ఆపు;
		}
	} లేకపోతే {
		అక్షరం utf8[6];
		size_t నిడివి = utf8_from_ucs4(కీలకం, utf8);
		utf8[నిడివి] = '\0';

		exc = dom_string_create((స్థిరరాశి uint8_t *)utf8, అక్షరశ్రేణి_పొడవు(utf8),
				&dom_కీలకం);
		ఒకవేళ (exc != DOM_NO_ERR) {
			తిరిగి exc;
		}
	}

	exc = dom_keyboard_event_create(&evt);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_string_unref(dom_కీలకం);
		తిరిగి false;
	}

	exc = dom_keyboard_event_init(evt, type, bubbles, cancelable, ఖాళీ,
			dom_కీలకం, ఖాళీ, DOM_కీలకం_LOCATION_STANDARD, false,
			false, false, false, false, false);
	dom_string_unref(dom_కీలకం);
	ఒకవేళ (exc != DOM_NO_ERR) {
		dom_event_unref(evt);
		తిరిగి false;
	}

	NSLOG(netsurf, INFO, "Dispatching '%*s' against %p",
			(పూర్ణాంకం)dom_string_నిడివి(type), dom_string_దత్తాంశం(type), లక్ష్యం);

	ఫలితం = fire_dom_event((dom_event *) evt, లక్ష్యం);
	dom_event_unref(evt);
	తిరిగి ఫలితం;
}

/**
 * Perform post-box-creation conversion of a document
 *
 * \param c        HTML content to complete conversion of
 * \param success  Whether box tree construction was successful
 */
స్థిర శూన్య html_box_convert_పూర్తి(html_content *c, bool success)
{
	nserror err;
	dom_exception exc; /* returned by libdom functions */
	dom_కణుపు *html;

	NSLOG(netsurf, INFO, "DOM to box conversion complete (content %p)", c);

	c->box_conversion_context = ఖాళీ;

	/* Clean up and report error if unsuccessful or aborted */
	ఒకవేళ ((success == false) || (c->aborted)) {
		html_object_free_objects(c);

		ఒకవేళ (success == false) {
			content_ప్రసారం_error(&c->పునాది, NSERROR_BOX_CONVERT, ఖాళీ);
		} లేకపోతే {
			content_ప్రసారం_error(&c->పునాది, NSERROR_STOPPED, ఖాళీ);
		}

		content_సమితి_error(&c->పునాది);
		తిరిగి;
	}


#ఒకవేళ ALWAYS_DUMP_BOX
	box_dump(stderr, c->layout->children, 0, true);
#అయితే_ముగింపు
#ఒకవేళ ALWAYS_DUMP_FRAMESET
	ఒకవేళ (c->frameset)
		html_dump_frameset(c->frameset, 0);
#అయితే_ముగింపు

	exc = dom_document_get_document_element(c->document, (శూన్య *) &html);
	ఒకవేళ ((exc != DOM_NO_ERR) || (html == ఖాళీ)) {
		/** @todo should this call html_object_free_objects(c);
		 * like the other error paths
		 */
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ప్రసారం_error(&c->పునాది, NSERROR_DOM, ఖాళీ);
		content_సమితి_error(&c->పునాది);
		తిరిగి;
	}

	/* extract image maps - can't do this sensibly in dom_to_box */
	err = imagemap_వెలికితీయి(c);
	ఒకవేళ (err != NSERROR_OK) {
		NSLOG(netsurf, INFO, "imagemap extraction failed");
		html_object_free_objects(c);
		content_ప్రసారం_error(&c->పునాది, err, ఖాళీ);
		content_సమితి_error(&c->పునాది);
		dom_కణుపు_unref(html);
		తిరిగి;
	}
	/*imagemap_dump(c);*/

	/* Destroy the parser binding */
	dom_hubbub_parser_destroy(c->parser);
	c->parser = ఖాళీ;

	content_సమితి_ready(&c->పునాది);

	html_proceed_గమ్యం_పూర్తి(c);

	dom_కణుపు_unref(html);
}

/* Documented in html_internal.h */
nserror
html_proceed_గమ్యం_పూర్తి(html_content *html)
{
	ఎంపిక (content_get_స్థితి_2(&html->పునాది)) {
	స్థితి CONTENT_స్థితి_READY:
		ఒకవేళ (html->పునాది.చురుకైన == 0) {
			content_సమితి_పూర్తి(&html->పునాది);
			తిరిగి NSERROR_OK;
		}
		ఆపు;
	స్థితి CONTENT_స్థితి_పూర్తి:
		/* fallthrough */
	స్థితి CONTENT_స్థితి_LOADING:
		తిరిగి NSERROR_OK;
	అప్రమేయ:
		NSLOG(netsurf, ERROR, "Content status unexpectedly not LOADING/READY/DONE");
		ఆపు;
	}
	తిరిగి NSERROR_UNKNOWN;
}


స్థిర శూన్య html_get_dimensions(html_content *htmlc)
{
	css_fixed device_dpi = nscss_screen_dpi;
	గుర్తులేని f_కొలత;
	గుర్తులేని f_min;
	గుర్తులేని w;
	గుర్తులేని h;
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం = {
		.getdims = {
			.viewport_వెడల్పు = &w,
			.viewport_ఎత్తు = &h,
		},
	};

	content_ప్రసారం(&htmlc->పునాది, CONTENT_సందేశం_GETDIMS, &సందేశం_దత్తాంశం);


	w = css_unit_device2css_px(INTTOFIX(w), device_dpi);
	h = css_unit_device2css_px(INTTOFIX(h), device_dpi);

	htmlc->media.వెడల్పు  = w;
	htmlc->media.ఎత్తు = h;
	htmlc->unit_len_ctx.viewport_వెడల్పు  = w;
	htmlc->unit_len_ctx.viewport_ఎత్తు = h;
	htmlc->unit_len_ctx.device_dpi = device_dpi;

	/** \todo Change nsoption font sizes to px. */
	f_కొలత = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_కొలత)), F_10)), F_72);
	f_min  = FDIV(FMUL(F_96, FDIV(INTTOFIX(nsoption_int(font_min_కొలత)), F_10)), F_72);

	htmlc->unit_len_ctx.font_కొలత_default = f_కొలత;
	htmlc->unit_len_ctx.font_కొలత_minimum = f_min;
}

/* exported function documented in html/html_internal.h */
శూన్య html_ముగింపు_conversion(html_content *htmlc)
{
	సంఘం content_సందేశం_దత్తాంశం సందేశం_దత్తాంశం;
	dom_exception exc; /* returned by libdom functions */
	dom_కణుపు *html;
	nserror error;

	/* Bail out if we've been aborted */
	ఒకవేళ (htmlc->aborted) {
		content_ప్రసారం_error(&htmlc->పునాది, NSERROR_STOPPED, ఖాళీ);
		content_సమితి_error(&htmlc->పునాది);
		తిరిగి;
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
	ఒకవేళ (htmlc->select_ctx != ఖాళీ) {
		NSLOG(netsurf, INFO,
				"Ignoring style change: NS layout is static.");
		తిరిగి;
	}

	/* create new css selection context */
	error = html_css_new_selection_context(htmlc, &htmlc->select_ctx);
	ఒకవేళ (error != NSERROR_OK) {
		content_ప్రసారం_error(&htmlc->పునాది, error, ఖాళీ);
		content_సమితి_error(&htmlc->పునాది);
		తిరిగి;
	}


	/* fire a simple event named load at the Document's Window
	 * object, but with its target set to the Document object (and
	 * the currentTarget set to the Window object)
	 */
	ఒకవేళ (htmlc->jsthread != ఖాళీ) {
		js_fire_event(htmlc->jsthread, "load", htmlc->document, ఖాళీ);
	}

	/* convert dom tree to box tree */
	NSLOG(netsurf, INFO, "DOM to box (%p)", htmlc);
	content_సమితి_స్థితి(&htmlc->పునాది, messages_get("Processing"));
	సందేశం_దత్తాంశం.explicit_స్థితి_పాఠ్యం = ఖాళీ;
	content_ప్రసారం(&htmlc->పునాది, CONTENT_సందేశం_స్థితి, &సందేశం_దత్తాంశం);

	exc = dom_document_get_document_element(htmlc->document, (శూన్య *) &html);
	ఒకవేళ ((exc != DOM_NO_ERR) || (html == ఖాళీ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ప్రసారం_error(&htmlc->పునాది, NSERROR_DOM, ఖాళీ);
		content_సమితి_error(&htmlc->పునాది);
		తిరిగి;
	}

	html_get_dimensions(htmlc);

	error = dom_గమ్యం_box(html, htmlc, html_box_convert_పూర్తి, &htmlc->box_conversion_context);
	ఒకవేళ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "box conversion failed");
		dom_కణుపు_unref(html);
		html_object_free_objects(htmlc);
		content_ప్రసారం_error(&htmlc->పునాది, error, ఖాళీ);
		content_సమితి_error(&htmlc->పునాది);
		తిరిగి;
	}

	dom_కణుపు_unref(html);
}


స్థిర శూన్య
html_document_user_దత్తాంశం_handler(dom_కణుపు_operation operation,
				dom_string *కీలకం, శూన్య *దత్తాంశం,
				నిర్మాణం dom_కణుపు *src,
				నిర్మాణం dom_కణుపు *dst)
{
	ఒకవేళ (dom_string_isequal(corestring_dom_ns_కీలకం_html_content_దత్తాంశం,
			       కీలకం) == false || దత్తాంశం == ఖాళీ) {
		తిరిగి;
	}

	ఎంపిక (operation) {
	స్థితి DOM_కణుపు_CLONED:
		NSLOG(netsurf, INFO, "Cloned");
		ఆపు;
	స్థితి DOM_కణుపు_RENAMED:
		NSLOG(netsurf, INFO, "Renamed");
		ఆపు;
	స్థితి DOM_కణుపు_IMPORTED:
		NSLOG(netsurf, INFO, "imported");
		ఆపు;
	స్థితి DOM_కణుపు_ADOPTED:
		NSLOG(netsurf, INFO, "Adopted");
		ఆపు;
	స్థితి DOM_కణుపు_DELETED:
		/* This is the only path I expect */
		ఆపు;
	అప్రమేయ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ప్రతిజ్ఞా(0);
	}
}


స్థిర nserror
html_create_html_దత్తాంశం(html_content *c, స్థిరరాశి http_parameter *params)
{
	lwc_string *charset;
	nserror nerror;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	dom_exception err;
	శూన్య *old_కణుపు_దత్తాంశం;
	స్థిరరాశి అక్షరం *prefer_color_బహుళకం = (nsoption_bool(prefer_dark_బహుళకం)) ?
			"dark" : "light";

	c->parser = ఖాళీ;
	c->parse_completed = false;
	c->conversion_begun = false;
	c->document = ఖాళీ;
	c->quirks = DOM_DOCUMENT_QUIRKS_బహుళకం_NONE;
	c->encoding = ఖాళీ;
	c->పునాది_url = nsurl_ref(content_get_url(&c->పునాది));
	c->పునాది_లక్ష్యం = ఖాళీ;
	c->aborted = false;
	c->refresh = false;
	c->reflowing = false;
	c->title = ఖాళీ;
	c->bctx = ఖాళీ;
	c->layout = ఖాళీ;
	c->background_రంగు = NS_TRANSPARENT;
	c->stylesheet_లెక్క = 0;
	c->stylesheets = ఖాళీ;
	c->select_ctx = ఖాళీ;
	c->media.type = CSS_MEDIA_SCREEN;
	c->universal = ఖాళీ;
	c->అంకె_objects = 0;
	c->object_list = ఖాళీ;
	c->forms = ఖాళీ;
	c->imagemaps = ఖాళీ;
	c->bw = ఖాళీ;
	c->frameset = ఖాళీ;
	c->iframe = ఖాళీ;
	c->పుట = ఖాళీ;
	c->font_func = guit->layout;
	c->drag_type = HTML_DRAG_NONE;
	c->drag_owner.no_owner = true;
	c->selection_type = HTML_SELECTION_NONE;
	c->selection_owner.none = true;
	c->focus_type = HTML_FOCUS_SELF;
	c->focus_owner.self = true;
	c->scripts_లెక్క = 0;
	c->scripts = ఖాళీ;
	c->jsthread = ఖాళీ;

	c->enable_scripting = nsoption_bool(enable_javascript);
	c->పునాది.చురుకైన = 1; /* The html content itself is active */

	ఒకవేళ (lwc_intern_string("*", SLEN("*"), &c->universal) != lwc_error_ok) {
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (lwc_intern_string(prefer_color_బహుళకం, అక్షరశ్రేణి_పొడవు(prefer_color_బహుళకం),
			&c->media.prefers_color_scheme) != lwc_error_ok) {
		lwc_string_unref(c->universal);
		c->universal = ఖాళీ;
		తిరిగి NSERROR_NOMEM;
	}

	c->sel = selection_create((నిర్మాణం content *)c);

	nerror = http_parameter_list_కనుగొను_అంశం(params, corestring_lwc_charset, &charset);
	ఒకవేళ (nerror == NSERROR_OK) {
		c->encoding = strdup(lwc_string_దత్తాంశం(charset));

		lwc_string_unref(charset);

		ఒకవేళ (c->encoding == ఖాళీ) {
			lwc_string_unref(c->universal);
			c->universal = ఖాళీ;
			lwc_string_unref(c->media.prefers_color_scheme);
			c->media.prefers_color_scheme = ఖాళీ;
			తిరిగి NSERROR_NOMEM;

		}
		c->encoding_ఆధారం = DOM_HUBBUB_ENCODING_ఆధారం_శీర్షిక;
	}

	/* Create the parser binding */
	parse_params.enc = c->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = c->enable_scripting;
	parse_params.సందేశం = ఖాళీ;
	parse_params.script = html_process_script;
	parse_params.ctx = c;
	parse_params.daf = html_dom_event_fetcher;

	error = dom_hubbub_parser_create(&parse_params,
					 &c->parser,
					 &c->document);
	ఒకవేళ ((error != DOM_HUBBUB_OK) && (c->encoding != ఖాళీ)) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to autodetect */
		విడుదల(c->encoding);
		c->encoding = ఖాళీ;

		parse_params.enc = c->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &c->parser,
						 &c->document);
	}
	ఒకవేళ (error != DOM_HUBBUB_OK) {
		nsurl_unref(c->పునాది_url);
		c->పునాది_url = ఖాళీ;

		lwc_string_unref(c->universal);
		c->universal = ఖాళీ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ఖాళీ;

		తిరిగి libdom_hubbub_error_గమ్యం_nserror(error);
	}

	err = dom_కణుపు_సమితి_user_దత్తాంశం(c->document,
				     corestring_dom_ns_కీలకం_html_content_దత్తాంశం,
				     c, html_document_user_దత్తాంశం_handler,
				     (శూన్య *) &old_కణుపు_దత్తాంశం);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_hubbub_parser_destroy(c->parser);
		c->parser = ఖాళీ;
		nsurl_unref(c->పునాది_url);
		c->పునాది_url = ఖాళీ;

		lwc_string_unref(c->universal);
		c->universal = ఖాళీ;
		lwc_string_unref(c->media.prefers_color_scheme);
		c->media.prefers_color_scheme = ఖాళీ;

		NSLOG(netsurf, INFO, "Unable to set user data.");
		తిరిగి NSERROR_DOM;
	}

	ప్రతిజ్ఞా(old_కణుపు_దత్తాంశం == ఖాళీ);

	తిరిగి NSERROR_OK;

}

/**
 * Create a CONTENT_HTML.
 *
 * The content_html_data structure is initialized and the HTML parser is
 * created.
 */

స్థిర nserror
html_create(స్థిరరాశి content_handler *handler,
	    lwc_string *imime_type,
	    స్థిరరాశి http_parameter *params,
	    llcache_handle *llcache,
	    స్థిరరాశి అక్షరం *fallback_charset,
	    bool quirks,
	    నిర్మాణం content **c)
{
	html_content *html;
	nserror error;

	html = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(html_content));
	ఒకవేళ (html == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&html->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(html);
		తిరిగి error;
	}

	error = html_create_html_దత్తాంశం(html, params);
	ఒకవేళ (error != NSERROR_OK) {
		content_ప్రసారం_error(&html->పునాది, error, ఖాళీ);
		విడుదల(html);
		తిరిగి error;
	}

	error = html_css_new_stylesheets(html);
	ఒకవేళ (error != NSERROR_OK) {
		content_ప్రసారం_error(&html->పునాది, error, ఖాళీ);
		విడుదల(html);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) html;

	తిరిగి NSERROR_OK;
}



స్థిర nserror
html_process_encoding_change(నిర్మాణం content *c,
			     స్థిరరాశి అక్షరం *దత్తాంశం,
			     గుర్తులేని పూర్ణాంకం కొలత)
{
	html_content *html = (html_content *) c;
	dom_hubbub_parser_params parse_params;
	dom_hubbub_error error;
	స్థిరరాశి అక్షరం *encoding;
	స్థిరరాశి uint8_t *ఆధారం_దత్తాంశం;
	size_t ఆధారం_కొలత;

	/* Retrieve new encoding */
	encoding = dom_hubbub_parser_get_encoding(html->parser,
						  &html->encoding_ఆధారం);
	ఒకవేళ (encoding == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	ఒకవేళ (html->encoding != ఖాళీ) {
		విడుదల(html->encoding);
		html->encoding = ఖాళీ;
	}

	html->encoding = strdup(encoding);
	ఒకవేళ (html->encoding == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	/* Destroy binding */
	dom_hubbub_parser_destroy(html->parser);
	html->parser = ఖాళీ;

	ఒకవేళ (html->document != ఖాళీ) {
		dom_కణుపు_unref(html->document);
	}

	parse_params.enc = html->encoding;
	parse_params.fix_enc = true;
	parse_params.enable_script = html->enable_scripting;
	parse_params.సందేశం = ఖాళీ;
	parse_params.script = html_process_script;
	parse_params.ctx = html;
	parse_params.daf = html_dom_event_fetcher;

	/* Create new binding, using the new encoding */
	error = dom_hubbub_parser_create(&parse_params,
					 &html->parser,
					 &html->document);
	ఒకవేళ (error != DOM_HUBBUB_OK) {
		/* Ok, we don't support the declared encoding. Bailing out
		 * isn't exactly user-friendly, so fall back to Windows-1252 */
		విడుదల(html->encoding);
		html->encoding = strdup("Windows-1252");
		ఒకవేళ (html->encoding == ఖాళీ) {
			తిరిగి NSERROR_NOMEM;
		}
		parse_params.enc = html->encoding;

		error = dom_hubbub_parser_create(&parse_params,
						 &html->parser,
						 &html->document);

		ఒకవేళ (error != DOM_HUBBUB_OK) {
			తిరిగి libdom_hubbub_error_గమ్యం_nserror(error);
		}

	}

	ఆధారం_దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &ఆధారం_కొలత);

	/* Reprocess all the data.  This is safe because
	 * the encoding is now specified at parser start which means
	 * it cannot be changed again.
	 */
	error = dom_hubbub_parser_parse_chunk(html->parser,
					      ఆధారం_దత్తాంశం,
					      ఆధారం_కొలత);

	తిరిగి libdom_hubbub_error_గమ్యం_nserror(error);
}


/**
 * Process data for CONTENT_HTML.
 */

స్థిర bool
html_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం, గుర్తులేని పూర్ణాంకం కొలత)
{
	html_content *html = (html_content *) c;
	dom_hubbub_error dom_ret;
	nserror err = NSERROR_OK; /* assume its all going to be ok */

	dom_ret = dom_hubbub_parser_parse_chunk(html->parser,
					      (స్థిరరాశి uint8_t *) దత్తాంశం,
					      కొలత);

	err = libdom_hubbub_error_గమ్యం_nserror(dom_ret);

	/* deal with encoding change */
	ఒకవేళ (err == NSERROR_ENCODING_CHANGE) {
		 err = html_process_encoding_change(c, దత్తాంశం, కొలత);
	}

	/* broadcast the error if necessary */
	ఒకవేళ (err != NSERROR_OK) {
		content_ప్రసారం_error(c, err, ఖాళీ);
		తిరిగి false;
	}

	తిరిగి true;
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

స్థిర bool html_convert(నిర్మాణం content *c)
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
	exc = dom_document_get_quirks_బహుళకం(htmlc->document, &htmlc->quirks);
	ఒకవేళ (exc == DOM_NO_ERR) {
		html_css_quirks_stylesheets(htmlc);
		NSLOG(netsurf, INFO, "quirks set to %d", htmlc->quirks);
	}

	htmlc->పునాది.చురుకైన--; /* the html fetch is no longer active */
	NSLOG(netsurf, INFO, "%d fetches active (%p)", htmlc->పునాది.చురుకైన, c);

	/* The parse cannot be completed here because it may be paused
	 * untill all the resources being fetched have completed.
	 */

	/* if there are no active fetches in progress no scripts are
	 * being fetched or they completed already.
	 */
	ఒకవేళ (html_can_begin_conversion(htmlc)) {
		తిరిగి html_begin_conversion(htmlc);
	}
	తిరిగి true;
}

/* Exported interface documented in html_internal.h */
bool html_can_begin_conversion(html_content *htmlc)
{
	గుర్తులేని పూర్ణాంకం i;

	/* Cannot begin conversion if we're still fetching stuff */
	ఒకవేళ (htmlc->పునాది.చురుకైన != 0)
		తిరిగి false;

	కొరకు (i = 0; i != htmlc->stylesheet_లెక్క; i++) {
		/* Cannot begin conversion if the stylesheets are modified */
		ఒకవేళ (htmlc->stylesheets[i].modified)
			తిరిగి false;
	}

	/* All is good, begin */
	తిరిగి true;
}

bool
html_begin_conversion(html_content *htmlc)
{
	dom_కణుపు *html;
	nserror ns_error;
	నిర్మాణం form *f;
	dom_exception exc; /* returned by libdom functions */
	dom_string *కణుపు_name = ఖాళీ;
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
	ఒకవేళ (htmlc->parse_completed == false) {
		NSLOG(netsurf, INFO, "Completing parse (%p)", htmlc);
		/* complete parsing */
		error = dom_hubbub_parser_completed(htmlc->parser);
		ఒకవేళ (error == DOM_HUBBUB_HUBBUB_ERR_PAUSED && htmlc->పునాది.చురుకైన > 0) {
			/* The act of completing the parse failed because we've
			 * encountered a sync script which needs to run
			 */
			NSLOG(netsurf, INFO, "Completing parse brought synchronous JS to light, cannot complete yet");
			తిరిగి true;
		}
		ఒకవేళ (error != DOM_HUBBUB_OK) {
			NSLOG(netsurf, INFO, "Parsing failed");

			content_ప్రసారం_error(&htmlc->పునాది,
						libdom_hubbub_error_గమ్యం_nserror(error),
						ఖాళీ);

			తిరిగి false;
		}
		htmlc->parse_completed = true;
	}

	ఒకవేళ (html_can_begin_conversion(htmlc) == false) {
		NSLOG(netsurf, INFO, "Can't begin conversion (%p)", htmlc);
		/* We can't proceed (see commentary above) */
		తిరిగి true;
	}

	/* Give up processing if we've been aborted */
	ఒకవేళ (htmlc->aborted) {
		NSLOG(netsurf, INFO, "Conversion aborted (%p) (active: %u)",
		      htmlc, htmlc->పునాది.చురుకైన);
		content_సమితి_error(&htmlc->పునాది);
		content_ప్రసారం_error(&htmlc->పునాది, NSERROR_STOPPED, ఖాళీ);
		తిరిగి false;
	}

	/* Conversion begins proper at this point */
	htmlc->conversion_begun = true;

	/* complete script execution, including deferred scripts */
	html_script_exec(htmlc, true);

	/* fire a simple event that bubbles named DOMContentLoaded at
	 * the Document.
	 */

	/* get encoding */
	ఒకవేళ (htmlc->encoding == ఖాళీ) {
		స్థిరరాశి అక్షరం *encoding;

		encoding = dom_hubbub_parser_get_encoding(htmlc->parser,
					&htmlc->encoding_ఆధారం);
		ఒకవేళ (encoding == ఖాళీ) {
			content_ప్రసారం_error(&htmlc->పునాది,
						NSERROR_NOMEM,
						ఖాళీ);
			తిరిగి false;
		}

		htmlc->encoding = strdup(encoding);
		ఒకవేళ (htmlc->encoding == ఖాళీ) {
			content_ప్రసారం_error(&htmlc->పునాది,
						NSERROR_NOMEM,
						ఖాళీ);
			తిరిగి false;
		}
	}

	/* locate root element and ensure it is html */
	exc = dom_document_get_document_element(htmlc->document, (శూన్య *) &html);
	ఒకవేళ ((exc != DOM_NO_ERR) || (html == ఖాళీ)) {
		NSLOG(netsurf, INFO, "error retrieving html element from dom");
		content_ప్రసారం_error(&htmlc->పునాది, NSERROR_DOM, ఖాళీ);
		తిరిగి false;
	}

	exc = dom_కణుపు_get_కణుపు_name(html, &కణుపు_name);
	ఒకవేళ ((exc != DOM_NO_ERR) ||
	    (కణుపు_name == ఖాళీ) ||
	    (!dom_string_caseless_lwc_isequal(కణుపు_name,
			corestring_lwc_html))) {
		NSLOG(netsurf, INFO, "root element not html");
		content_ప్రసారం_error(&htmlc->పునాది, NSERROR_DOM, ఖాళీ);
		dom_కణుపు_unref(html);
		తిరిగి false;
	}
	dom_string_unref(కణుపు_name);

	/* Retrieve forms from parser */
	htmlc->forms = html_forms_get_forms(htmlc->encoding,
			(dom_html_document *) htmlc->document);
	కొరకు (f = htmlc->forms; f != ఖాళీ; f = f->prev) {
		nsurl *action;

		/* Make all actions absolute */
		ఒకవేళ (f->action == ఖాళీ || f->action[0] == '\0') {
			/* HTML5 4.10.22.3 step 9 */
			nsurl *doc_addr = content_get_url(&htmlc->పునాది);
			ns_error = nsurl_join(htmlc->పునాది_url,
					      nsurl_access(doc_addr),
					      &action);
		} లేకపోతే {
			ns_error = nsurl_join(htmlc->పునాది_url,
					      f->action,
					      &action);
		}

		ఒకవేళ (ns_error != NSERROR_OK) {
			content_ప్రసారం_error(&htmlc->పునాది, ns_error, ఖాళీ);

			dom_కణుపు_unref(html);
			తిరిగి false;
		}

		విడుదల(f->action);
		f->action = strdup(nsurl_access(action));
		nsurl_unref(action);
		ఒకవేళ (f->action == ఖాళీ) {
			content_ప్రసారం_error(&htmlc->పునాది,
						NSERROR_NOMEM,
						ఖాళీ);

			dom_కణుపు_unref(html);
			తిరిగి false;
		}

		/* Ensure each form has a document encoding */
		ఒకవేళ (f->document_charset == ఖాళీ) {
			f->document_charset = strdup(htmlc->encoding);
			ఒకవేళ (f->document_charset == ఖాళీ) {
				content_ప్రసారం_error(&htmlc->పునాది,
							NSERROR_NOMEM,
							ఖాళీ);
				dom_కణుపు_unref(html);
				తిరిగి false;
			}
		}
	}

	dom_కణుపు_unref(html);

	ఒకవేళ (htmlc->పునాది.చురుకైన == 0) {
		html_ముగింపు_conversion(htmlc);
	}

	తిరిగి true;
}


/**
 * Stop loading a CONTENT_HTML.
 *
 * called when the content is aborted. This must clean up any state
 * created during the fetch.
 */

స్థిర శూన్య html_stop(నిర్మాణం content *c)
{
	html_content *htmlc = (html_content *) c;

	ఎంపిక (c->స్థితి_2) {
	స్థితి CONTENT_స్థితి_LOADING:
		/* Still loading; simply flag that we've been aborted
		 * html_convert/html_finish_conversion will do the rest */
		htmlc->aborted = true;
		ఒకవేళ (htmlc->jsthread != ఖాళీ) {
			/* Close the JS thread to cancel out any callbacks */
			js_closethread(htmlc->jsthread);
		}
		ఆపు;

	స్థితి CONTENT_స్థితి_READY:
		html_object_abort_objects(htmlc);

		/* If there are no further active fetches and we're still
		 * in the READY state, transition to the DONE state. */
		ఒకవేళ (c->స్థితి_2 == CONTENT_స్థితి_READY && c->చురుకైన == 0) {
			content_సమితి_పూర్తి(c);
		}

		ఆపు;

	స్థితి CONTENT_స్థితి_పూర్తి:
		/* Nothing to do */
		ఆపు;

	అప్రమేయ:
		NSLOG(netsurf, INFO, "Unexpected status %d (%p)", c->స్థితి_2,
		      c);
		ప్రతిజ్ఞా(0);
	}
}


/**
 * Reformat a CONTENT_HTML to a new width.
 */

స్థిర శూన్య html_reformat(నిర్మాణం content *c, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	html_content *htmlc = (html_content *) c;
	నిర్మాణం box *layout;
	uint64_t ms_before;
	uint64_t ms_after;
	uint64_t ms_interval;

	nsu_getmonotonic_ms(&ms_before);

	htmlc->reflowing = true;

	htmlc->unit_len_ctx.viewport_వెడల్పు = css_unit_device2css_px(
			INTTOFIX(వెడల్పు), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.viewport_ఎత్తు = css_unit_device2css_px(
			INTTOFIX(ఎత్తు), htmlc->unit_len_ctx.device_dpi);
	htmlc->unit_len_ctx.మూలం_style = htmlc->layout->style;

	layout_document(htmlc, వెడల్పు, ఎత్తు);
	layout = htmlc->layout;

	/* width and height are at least margin box of document */
	c->వెడల్పు = layout->x + layout->padding[ఎడమ_2] + layout->వెడల్పు +
		layout->padding[కుడి_2] + layout->border[కుడి_2].వెడల్పు +
		layout->margin[కుడి_2];
	c->ఎత్తు = layout->y + layout->padding[శిఖరం_2] + layout->ఎత్తు +
		layout->padding[BOTTOM] + layout->border[BOTTOM].వెడల్పు +
		layout->margin[BOTTOM];

	/* if boxes overflow right or bottom edge, expand to contain it */
	ఒకవేళ (c->వెడల్పు < layout->x + layout->descendant_x1)
		c->వెడల్పు = layout->x + layout->descendant_x1;
	ఒకవేళ (c->ఎత్తు < layout->y + layout->descendant_y1)
		c->ఎత్తు = layout->y + layout->descendant_y1;

	selection_reinit(htmlc->sel);

	htmlc->reflowing = false;
	htmlc->had_initial_layout = true;

	/* calculate next reflow time at three times what it took to reflow */
	nsu_getmonotonic_ms(&ms_after);

	ms_interval = (ms_after - ms_before) * 3;
	ఒకవేళ (ms_interval < (nsoption_uint(min_reflow_period) * 10)) {
		ms_interval = nsoption_uint(min_reflow_period) * 10;
	}
	c->reformat_సమయం = ms_after + ms_interval;
}


/**
 * Redraw a box.
 *
 * \param  h	content containing the box, of type CONTENT_HTML
 * \param  box  box to redraw
 */

శూన్య html_redraw_a_box(hlcache_handle *h, నిర్మాణం box *box)
{
	పూర్ణాంకం x, y;

	box_coords(box, &x, &y);

	content_request_redraw(h, x, y,
			box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2],
			box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM]);
}


/**
 * Redraw a box.
 *
 * \param html  content containing the box, of type CONTENT_HTML
 * \param box  box to redraw.
 */

శూన్య html__redraw_a_box(నిర్మాణం html_content *html, నిర్మాణం box *box)
{
	పూర్ణాంకం x, y;

	box_coords(box, &x, &y);

	content__request_redraw((నిర్మాణం content *)html, x, y,
			box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2],
			box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM]);
}

స్థిర శూన్య html_destroy_frameset(నిర్మాణం content_html_చట్రాలు *frameset)
{
	పూర్ణాంకం i;

	ఒకవేళ (frameset->name) {
		talloc_free(frameset->name);
		frameset->name = ఖాళీ;
	}
	ఒకవేళ (frameset->url) {
		talloc_free(frameset->url);
		frameset->url = ఖాళీ;
	}
	ఒకవేళ (frameset->children) {
		కొరకు (i = 0; i < (frameset->అడ్డువరుసలు * frameset->నిలువుల_సంఖ్య); i++) {
			ఒకవేళ (frameset->children[i].name) {
				talloc_free(frameset->children[i].name);
				frameset->children[i].name = ఖాళీ;
			}
			ఒకవేళ (frameset->children[i].url) {
				nsurl_unref(frameset->children[i].url);
				frameset->children[i].url = ఖాళీ;
			}
			ఒకవేళ (frameset->children[i].children)
				html_destroy_frameset(&frameset->children[i]);
		}
		talloc_free(frameset->children);
		frameset->children = ఖాళీ;
	}
}

స్థిర శూన్య html_destroy_iframe(నిర్మాణం content_html_iframe *iframe)
{
	నిర్మాణం content_html_iframe *తదుపరి;
	తదుపరి = iframe;
	అయితే ((iframe = తదుపరి) != ఖాళీ) {
		తదుపరి = iframe->తదుపరి;
		ఒకవేళ (iframe->name)
			talloc_free(iframe->name);
		ఒకవేళ (iframe->url) {
			nsurl_unref(iframe->url);
			iframe->url = ఖాళీ;
		}
		talloc_free(iframe);
	}
}


స్థిర శూన్య html_free_layout(html_content *htmlc)
{
	ఒకవేళ (htmlc->bctx != ఖాళీ) {
		/* freeing talloc context should let the entire box
		 * set be destroyed
		 */
		talloc_free(htmlc->bctx);
	}
}

/**
 * Destroy a CONTENT_HTML and free all resources it owns.
 */

స్థిర శూన్య html_destroy(నిర్మాణం content *c)
{
	html_content *html = (html_content *) c;
	నిర్మాణం form *f, *g;

	NSLOG(netsurf, INFO, "content %p", c);

	/* If we're still converting a layout, cancel it */
	ఒకవేళ (html->box_conversion_context != ఖాళీ) {
		ఒకవేళ (cancel_dom_గమ్యం_box(html->box_conversion_context) != NSERROR_OK) {
			NSLOG(netsurf, CRITICAL, "WARNING, Unable to cancel conversion context, browser may crash");
		}
	}

	selection_destroy(html->sel);

	/* Destroy forms */
	కొరకు (f = html->forms; f != ఖాళీ; f = g) {
		g = f->prev;

		form_free(f);
	}

	imagemap_destroy(html);

	ఒకవేళ (c->refresh)
		nsurl_unref(c->refresh);

	ఒకవేళ (html->పునాది_url)
		nsurl_unref(html->పునాది_url);

	/* At this point we can be moderately confident the JS is offline
	 * so we destroy the JS thread.
	 */
	ఒకవేళ (html->jsthread != ఖాళీ) {
		js_destroythread(html->jsthread);
		html->jsthread = ఖాళీ;
	}

	ఒకవేళ (html->parser != ఖాళీ) {
		dom_hubbub_parser_destroy(html->parser);
		html->parser = ఖాళీ;
	}

	ఒకవేళ (html->document != ఖాళీ) {
		dom_కణుపు_unref(html->document);
		html->document = ఖాళీ;
	}

	ఒకవేళ (html->title != ఖాళీ) {
		dom_కణుపు_unref(html->title);
		html->title = ఖాళీ;
	}

	/* Free encoding */
	ఒకవేళ (html->encoding != ఖాళీ) {
		విడుదల(html->encoding);
		html->encoding = ఖాళీ;
	}

	/* Free base target */
	ఒకవేళ (html->పునాది_లక్ష్యం != ఖాళీ) {
		విడుదల(html->పునాది_లక్ష్యం);
		html->పునాది_లక్ష్యం = ఖాళీ;
	}

	/* Free frameset */
	ఒకవేళ (html->frameset != ఖాళీ) {
		html_destroy_frameset(html->frameset);
		talloc_free(html->frameset);
		html->frameset = ఖాళీ;
	}

	/* Free iframes */
	ఒకవేళ (html->iframe != ఖాళీ) {
		html_destroy_iframe(html->iframe);
		html->iframe = ఖాళీ;
	}

	/* Destroy selection context */
	ఒకవేళ (html->select_ctx != ఖాళీ) {
		css_select_ctx_destroy(html->select_ctx);
		html->select_ctx = ఖాళీ;
	}

	lwc_string_unref(html->universal);
	html->universal = ఖాళీ;

	lwc_string_unref(html->media.prefers_color_scheme);
	html->media.prefers_color_scheme = ఖాళీ;

	/* Free stylesheets */
	html_css_free_stylesheets(html);

	/* Free scripts */
	html_script_free(html);

	/* Free objects */
	html_object_free_objects(html);

	/* free layout */
	html_free_layout(html);
}


స్థిర nserror html_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	/** \todo Clone HTML specifics */

	/* In the meantime, we should never be called, as HTML contents
	 * cannot be shared and we're not intending to fix printing's
	 * cloning of documents. */
	ప్రతిజ్ఞా(0 && "html_clone should never be called");

	తిరిగి true;
}


/**
 * Handle a window containing a CONTENT_HTML being opened.
 */

స్థిర nserror
html_open(నిర్మాణం content *c,
	  నిర్మాణం browser_కిటికీ *bw,
	  నిర్మాణం content *పుట,
	  నిర్మాణం object_params *params)
{
	html_content *html = (html_content *) c;

	html->bw = bw;
	html->పుట = (html_content *) పుట;

	html->drag_type = HTML_DRAG_NONE;
	html->drag_owner.no_owner = true;

	/* text selection */
	selection_init(html->sel);
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;

	html_object_open_objects(html, bw);

	తిరిగి NSERROR_OK;
}


/**
 * Handle a window containing a CONTENT_HTML being closed.
 */

స్థిర nserror html_close(నిర్మాణం content *c)
{
	html_content *htmlc = (html_content *) c;
	nserror ret = NSERROR_OK;

	selection_clear(htmlc->sel, false);

	/* clear the html content reference to the browser window */
	htmlc->bw = ఖాళీ;

	/* remove all object references from the html content */
	html_object_close_objects(htmlc);

	ఒకవేళ (htmlc->jsthread != ఖాళీ) {
		/* Close, but do not destroy (yet) the JS thread */
		ret = js_closethread(htmlc->jsthread);
	}

	తిరిగి ret;
}


/**
 * Return an HTML content's selection context
 */

స్థిర శూన్య html_clear_selection(నిర్మాణం content *c)
{
	html_content *html = (html_content *) c;

	ఎంపిక (html->selection_type) {
	స్థితి HTML_SELECTION_NONE:
		/* Nothing to do */
		ప్రతిజ్ఞా(html->selection_owner.none == true);
		ఆపు;
	స్థితి HTML_SELECTION_TEXTAREA:
		textarea_clear_selection(html->selection_owner.textarea->
				gadget->దత్తాంశం.పాఠ్యం.ta);
		ఆపు;
	స్థితి HTML_SELECTION_SELF:
		ప్రతిజ్ఞా(html->selection_owner.none == false);
		selection_clear(html->sel, true);
		ఆపు;
	స్థితి HTML_SELECTION_CONTENT:
		content_clear_selection(html->selection_owner.content->object);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* There is no selection now. */
	html->selection_type = HTML_SELECTION_NONE;
	html->selection_owner.none = true;
}


/**
 * Return an HTML content's selection context
 */

స్థిర అక్షరం *html_get_selection(నిర్మాణం content *c)
{
	html_content *html = (html_content *) c;

	ఎంపిక (html->selection_type) {
	స్థితి HTML_SELECTION_TEXTAREA:
		తిరిగి textarea_get_selection(html->selection_owner.textarea->
				gadget->దత్తాంశం.పాఠ్యం.ta);
	స్థితి HTML_SELECTION_SELF:
		ప్రతిజ్ఞా(html->selection_owner.none == false);
		తిరిగి selection_get_copy(html->sel);
	స్థితి HTML_SELECTION_CONTENT:
		తిరిగి content_get_selection(
				html->selection_owner.content->object);
	స్థితి HTML_SELECTION_NONE:
		/* Nothing to do */
		ప్రతిజ్ఞా(html->selection_owner.none == true);
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	తిరిగి ఖాళీ;
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
స్థిర nserror
html_get_contextual_content(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y,
			    నిర్మాణం browser_కిటికీ_features *దత్తాంశం)
{
	html_content *html = (html_content *) c;

	నిర్మాణం box *box = html->layout;
	నిర్మాణం box *తదుపరి;
	పూర్ణాంకం box_x = 0, box_y = 0;

	అయితే ((తదుపరి = box_at_బిందువు(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ఖాళీ) {
		box = తదుపరి;

		/* hidden boxes are ignored */
		ఒకవేళ ((box->style != ఖాళీ) &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
			కొనసాగు;
		}

		ఒకవేళ (box->iframe) {
			దశాంశం scale = browser_కిటికీ_get_scale(box->iframe);
			browser_కిటికీ_get_features(box->iframe,
						    (x - box_x) * scale,
						    (y - box_y) * scale,
						    దత్తాంశం);
		}

		ఒకవేళ (box->object)
			content_get_contextual_content(box->object,
					x - box_x, y - box_y, దత్తాంశం);

		ఒకవేళ (box->object)
			దత్తాంశం->object = box->object;

		ఒకవేళ (box->href) {
			దత్తాంశం->link = box->href;
			దత్తాంశం->link_title = box->పాఠ్యం;
			దత్తాంశం->link_title_నిడివి = box->నిడివి;
		}

		ఒకవేళ (box->usemap) {
			స్థిరరాశి అక్షరం *లక్ష్యం = ఖాళీ;
			nsurl *url = imagemap_get(html, box->usemap, box_x,
					box_y, x, y, &లక్ష్యం);
			/* Box might have imagemap, but no actual link area
			 * at point */
			ఒకవేళ (url != ఖాళీ)
				దత్తాంశం->link = url;
		}
		ఒకవేళ (box->gadget) {
			ఎంపిక (box->gadget->type) {
			స్థితి GADGET_TEXTBOX:
			స్థితి GADGET_TEXTAREA:
			స్థితి GADGET_PASSWORD:
				దత్తాంశం->form_features = CTX_FORM_పాఠ్యం;
				ఆపు;

			స్థితి GADGET_FILE:
				దత్తాంశం->form_features = CTX_FORM_FILE;
				ఆపు;

			అప్రమేయ:
				దత్తాంశం->form_features = CTX_FORM_NONE;
				ఆపు;
			}
		}
	}
	తిరిగి NSERROR_OK;
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
స్థిర bool
html_scroll_at_బిందువు(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం scrx, పూర్ణాంకం scry)
{
	html_content *html = (html_content *) c;

	నిర్మాణం box *box = html->layout;
	నిర్మాణం box *తదుపరి;
	పూర్ణాంకం box_x = 0, box_y = 0;
	bool handled_scroll = false;

	/* TODO: invert order; visit deepest box first */

	అయితే ((తదుపరి = box_at_బిందువు(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ఖాళీ) {
		box = తదుపరి;

		ఒకవేళ (box->style && css_computed_visibility(box->style) ==
				CSS_VISIBILITY_HIDDEN)
			కొనసాగు;

		/* Pass into iframe */
		ఒకవేళ (box->iframe) {
			దశాంశం scale = browser_కిటికీ_get_scale(box->iframe);

			ఒకవేళ (browser_కిటికీ_scroll_at_బిందువు(box->iframe,
							   (x - box_x) * scale,
							   (y - box_y) * scale,
							   scrx, scry) == true)
				తిరిగి true;
		}

		/* Pass into textarea widget */
		ఒకవేళ (box->gadget && (box->gadget->type == GADGET_TEXTAREA ||
				box->gadget->type == GADGET_PASSWORD ||
				box->gadget->type == GADGET_TEXTBOX) &&
				textarea_scroll(box->gadget->దత్తాంశం.పాఠ్యం.ta,
						scrx, scry) == true)
			తిరిగి true;

		/* Pass into object */
		ఒకవేళ (box->object != ఖాళీ && content_scroll_at_బిందువు(
				box->object, x - box_x, y - box_y,
				scrx, scry) == true)
			తిరిగి true;

		/* Handle box scrollbars */
		ఒకవేళ (box->scroll_y && scrollbar_scroll(box->scroll_y, scry))
			handled_scroll = true;

		ఒకవేళ (box->scroll_x && scrollbar_scroll(box->scroll_x, scrx))
			handled_scroll = true;

		ఒకవేళ (handled_scroll == true)
			తిరిగి true;
	}

	తిరిగి false;
}

/** Helper for file gadgets to store their filename unencoded on the
 * dom node associated with the gadget.
 *
 * \todo Get rid of this crap eventually
 */
స్థిర శూన్య html_dom_user_దత్తాంశం_handler(dom_కణుపు_operation operation,
		dom_string *కీలకం, శూన్య *దత్తాంశం_2, నిర్మాణం dom_కణుపు *src,
		నిర్మాణం dom_కణుపు *dst)
{
	అక్షరం *oldfile;
	అక్షరం *దత్తాంశం = (అక్షరం *)దత్తాంశం_2;

	ఒకవేళ (!dom_string_isequal(corestring_dom_ns_కీలకం_file_name_కణుపు_దత్తాంశం,
				కీలకం) || దత్తాంశం == ఖాళీ) {
		తిరిగి;
	}

	ఎంపిక (operation) {
	స్థితి DOM_కణుపు_CLONED:
		ఒకవేళ (dom_కణుపు_సమితి_user_దత్తాంశం(dst,
					   corestring_dom_ns_కీలకం_file_name_కణుపు_దత్తాంశం,
					   strdup(దత్తాంశం), html_dom_user_దత్తాంశం_handler,
					   &oldfile) == DOM_NO_ERR) {
			ఒకవేళ (oldfile != ఖాళీ)
				విడుదల(oldfile);
		}
		ఆపు;

	స్థితి DOM_కణుపు_RENAMED:
	స్థితి DOM_కణుపు_IMPORTED:
	స్థితి DOM_కణుపు_ADOPTED:
		ఆపు;

	స్థితి DOM_కణుపు_DELETED:
		విడుదల(దత్తాంశం);
		ఆపు;
	అప్రమేయ:
		NSLOG(netsurf, INFO, "User data operation not handled.");
		ప్రతిజ్ఞా(0);
	}
}

స్థిర శూన్య html_సమితి_file_gadget_filename(నిర్మాణం content *c,
	నిర్మాణం form_control *gadget, స్థిరరాశి అక్షరం *fn)
{
	nserror ret;
	అక్షరం *utf8_fn, *oldfile = ఖాళీ;
	html_content *html = (html_content *)c;
	నిర్మాణం box *file_box = gadget->box;

	ret = guit->utf8->local_గమ్యం_utf8(fn, 0, &utf8_fn);
	ఒకవేళ (ret != NSERROR_OK) {
		ప్రతిజ్ఞా(ret != NSERROR_BAD_ENCODING);
		NSLOG(netsurf, INFO,
		      "utf8 to local encoding conversion failed");
		/* Load was for us - just no memory */
		తిరిగి;
	}

	form_gadget_update_విలువ(gadget, utf8_fn);

	/* corestring_dom___ns_key_file_name_node_data */
	ఒకవేళ (dom_కణుపు_సమితి_user_దత్తాంశం((dom_కణుపు *)file_box->gadget->కణుపు,
				   corestring_dom_ns_కీలకం_file_name_కణుపు_దత్తాంశం,
				   strdup(fn), html_dom_user_దత్తాంశం_handler,
				   &oldfile) == DOM_NO_ERR) {
		ఒకవేళ (oldfile != ఖాళీ)
			విడుదల(oldfile);
	}

	/* Redraw box. */
	html__redraw_a_box(html, file_box);
}

శూన్య html_సమితి_file_gadget_filename_2(నిర్మాణం hlcache_handle *hl,
	నిర్మాణం form_control *gadget, స్థిరరాశి అక్షరం *fn)
{
	తిరిగి html_సమితి_file_gadget_filename(hlcache_handle_get_content(hl),
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
స్థిర bool html_drop_file_at_బిందువు(నిర్మాణం content *c, పూర్ణాంకం x, పూర్ణాంకం y, అక్షరం *file)
{
	html_content *html = (html_content *) c;

	నిర్మాణం box *box = html->layout;
	నిర్మాణం box *తదుపరి;
	నిర్మాణం box *file_box = ఖాళీ;
	నిర్మాణం box *పాఠ్యం_box = ఖాళీ;
	పూర్ణాంకం box_x = 0, box_y = 0;

	/* Scan box tree for boxes that can handle drop */
	అయితే ((తదుపరి = box_at_బిందువు(&html->unit_len_ctx, box, x, y,
			&box_x, &box_y)) != ఖాళీ) {
		box = తదుపరి;

		ఒకవేళ (box->style &&
		    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN)
			కొనసాగు;

		ఒకవేళ (box->iframe) {
			దశాంశం scale = browser_కిటికీ_get_scale(box->iframe);
			తిరిగి browser_కిటికీ_drop_file_at_బిందువు(
				box->iframe,
				(x - box_x) * scale,
				(y - box_y) * scale,
				file);
		}

		ఒకవేళ (box->object &&
		    content_drop_file_at_బిందువు(box->object,
					x - box_x, y - box_y, file) == true)
			తిరిగి true;

		ఒకవేళ (box->gadget) {
			ఎంపిక (box->gadget->type) {
				స్థితి GADGET_FILE:
					file_box = box;
				ఆపు;

				స్థితి GADGET_TEXTBOX:
				స్థితి GADGET_TEXTAREA:
				స్థితి GADGET_PASSWORD:
					పాఠ్యం_box = box;
					ఆపు;

				అప్రమేయ:	/* appease compiler */
					ఆపు;
			}
		}
	}

	ఒకవేళ (!file_box && !పాఠ్యం_box)
		/* No box capable of handling drop */
		తిరిగి false;

	ఒకవేళ (file == ఖాళీ)
		/* There is a box capable of handling drop here */
		తిరిగి true;

	/* Handle the drop */
	ఒకవేళ (file_box) {
		/* File dropped on file input */
		html_సమితి_file_gadget_filename(c, file_box->gadget, file);

	} లేకపోతే {
		/* File dropped on text input */

		size_t file_len;
		FILE *fp = ఖాళీ;
		అక్షరం *బఫరు;
		అక్షరం *utf8_buff;
		nserror ret;
		గుర్తులేని పూర్ణాంకం కొలత;
		పూర్ణాంకం bx, by;

		/* Open file */
		fp = fopen(file, "rb");
		ఒకవేళ (fp == ఖాళీ) {
			/* Couldn't open file, but drop was for us */
			తిరిగి true;
		}

		/* Get filesize */
		fseek(fp, 0, SEEK_END);
		file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		ఒకవేళ ((దీర్ఘ)file_len == -1) {
			/* unable to get file length, but drop was for us */
			fclose(fp);
			తిరిగి true;
		}

		/* Allocate buffer for file data */
		బఫరు = జ్ఞాపకం_ఇవ్వు(file_len + 1);
		ఒకవేళ (బఫరు == ఖాళీ) {
			/* No memory, but drop was for us */
			fclose(fp);
			తిరిగి true;
		}

		/* Stick file into buffer */
		ఒకవేళ (file_len != fread(బఫరు, 1, file_len, fp)) {
			/* Failed, but drop was for us */
			విడుదల(బఫరు);
			fclose(fp);
			తిరిగి true;
		}

		/* Done with file */
		fclose(fp);

		/* Ensure buffer's string termination */
		బఫరు[file_len] = '\0';

		/* TODO: Sniff for text? */

		/* Convert to UTF-8 */
		ret = guit->utf8->local_గమ్యం_utf8(బఫరు, file_len, &utf8_buff);
		ఒకవేళ (ret != NSERROR_OK) {
			/* bad encoding shouldn't happen */
			NSLOG(netsurf, ERROR,
			      "local to utf8 encoding failed (%s)",
			      messages_get_errorcode(ret));
			ప్రతిజ్ఞా(ret != NSERROR_BAD_ENCODING);
			విడుదల(బఫరు);
			తిరిగి true;
		}

		/* Done with buffer */
		విడుదల(బఫరు);

		/* Get new length */
		కొలత = అక్షరశ్రేణి_పొడవు(utf8_buff);

		/* Simulate a click over the input box, to place caret */
		box_coords(పాఠ్యం_box, &bx, &by);
		textarea_mouse_action(పాఠ్యం_box->gadget->దత్తాంశం.పాఠ్యం.ta,
				BROWSER_MOUSE_PRESS_1, x - bx, y - by);

		/* Paste the file as text */
		textarea_drop_పాఠ్యం(పాఠ్యం_box->gadget->దత్తాంశం.పాఠ్యం.ta,
				utf8_buff, కొలత);

		విడుదల(utf8_buff);
	}

	తిరిగి true;
}


/**
 * set debug status.
 *
 * \param c The content to debug
 * \param op The debug operation type
 */
స్థిర nserror
html_debug(నిర్మాణం content *c, గణన content_debug op)
{
	html_redraw_debug = !html_redraw_debug;

	తిరిగి NSERROR_OK;
}


/**
 * Dump debug info concerning the html_content
 *
 * \param c The content to debug
 * \param f The file to dump to
 * \param op The debug dump type
 */
స్థిర nserror
html_debug_dump(నిర్మాణం content *c, FILE *f, గణన content_debug op)
{
	html_content *htmlc = (html_content *)c;
	dom_కణుపు *html;
	dom_exception exc; /* returned by libdom functions */
	nserror ret;

	ప్రతిజ్ఞా(htmlc != ఖాళీ);

	ఒకవేళ (op == CONTENT_DEBUG_RENDER) {
		ప్రతిజ్ఞా(htmlc->layout != ఖాళీ);
		box_dump(f, htmlc->layout, 0, true);
		ret = NSERROR_OK;
	} లేకపోతే {
		ఒకవేళ (htmlc->document == ఖాళీ) {
			NSLOG(netsurf, INFO, "No document to dump");
			తిరిగి NSERROR_DOM;
		}

		exc = dom_document_get_document_element(htmlc->document, (శూన్య *) &html);
		ఒకవేళ ((exc != DOM_NO_ERR) || (html == ఖాళీ)) {
			NSLOG(netsurf, INFO, "Unable to obtain root node");
			తిరిగి NSERROR_DOM;
		}

		ret = libdom_dump_structure(html, f, 0);

		NSLOG(netsurf, INFO, "DOM structure dump returning %d", ret);

		dom_కణుపు_unref(html);
	}

	తిరిగి ret;
}


#ఒకవేళ ALWAYS_DUMP_FRAMESET
/**
 * Print a frameset tree to stderr.
 */

స్థిర శూన్య
html_dump_frameset(నిర్మాణం content_html_చట్రాలు *చట్రం, గుర్తులేని పూర్ణాంకం లోతు)
{
	గుర్తులేని పూర్ణాంకం i;
	పూర్ణాంకం అడ్డువరుస, నిలువు, index;
	స్థిరరాశి అక్షరం *unit[] = {"px", "%", "*"};
	స్థిరరాశి అక్షరం *scrolling[] = {"auto", "yes", "no"};

	ప్రతిజ్ఞా(చట్రం);

	fprintf(stderr, "%p ", చట్రం);

	fprintf(stderr, "(%i %i) ", చట్రం->అడ్డువరుసలు, చట్రం->నిలువుల_సంఖ్య);

	fprintf(stderr, "w%g%s ", చట్రం->వెడల్పు.విలువ, unit[చట్రం->వెడల్పు.unit]);
	fprintf(stderr, "h%g%s ", చట్రం->ఎత్తు.విలువ,unit[చట్రం->ఎత్తు.unit]);
	fprintf(stderr, "(margin w%i h%i) ",
			చట్రం->margin_వెడల్పు, చట్రం->margin_ఎత్తు);

	ఒకవేళ (చట్రం->name)
		fprintf(stderr, "'%s' ", చట్రం->name);
	ఒకవేళ (చట్రం->url)
		fprintf(stderr, "<%s> ", చట్రం->url);

	ఒకవేళ (చట్రం->no_resize)
		fprintf(stderr, "noresize ");
	fprintf(stderr, "(scrolling %s) ", scrolling[చట్రం->scrolling]);
	ఒకవేళ (చట్రం->border)
		fprintf(stderr, "border %x ",
				(గుర్తులేని పూర్ణాంకం) చట్రం->border_రంగు);

	fprintf(stderr, "\n");

	ఒకవేళ (చట్రం->children) {
		కొరకు (అడ్డువరుస = 0; అడ్డువరుస != చట్రం->అడ్డువరుసలు; అడ్డువరుస++) {
			కొరకు (నిలువు = 0; నిలువు != చట్రం->నిలువుల_సంఖ్య; నిలువు++) {
				కొరకు (i = 0; i != లోతు; i++)
					fprintf(stderr, "  ");
				fprintf(stderr, "(%i %i): ", అడ్డువరుస, నిలువు);
				index = (అడ్డువరుస * చట్రం->నిలువుల_సంఖ్య) + నిలువు;
				html_dump_frameset(&చట్రం->children[index],
						లోతు + 1);
			}
		}
	}
}

#అయితే_ముగింపు

/**
 * Retrieve HTML document tree
 *
 * \param h  HTML content to retrieve document tree from
 * \return Pointer to document tree
 */
dom_document *html_get_document(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->document;
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
నిర్మాణం box *html_get_box_వృక్షం(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->layout;
}

/**
 * Retrieve the charset of an HTML document
 *
 * \param c Content to retrieve charset from
 * \param op The content encoding operation to perform.
 * \return Pointer to charset, or NULL
 */
స్థిర స్థిరరాశి అక్షరం *html_encoding(స్థిరరాశి నిర్మాణం content *c, గణన content_encoding_type op)
{
	html_content *html = (html_content *) c;
	స్థిర అక్షరం enc_token[10] = "Encoding0";

	ప్రతిజ్ఞా(html != ఖాళీ);

	ఒకవేళ (op == CONTENT_ENCODING_ఆధారం) {
		enc_token[8] = '0' + html->encoding_ఆధారం;
		తిరిగి messages_get(enc_token);
	}

	తిరిగి html->encoding;
}


/**
 * Retrieve framesets used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to framesets, or NULL if none
 */
నిర్మాణం content_html_చట్రాలు *html_get_frameset(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->frameset;
}

/**
 * Retrieve iframes used in an HTML document
 *
 * \param h  Content to inspect
 * \return Pointer to iframes, or NULL if none
 */
నిర్మాణం content_html_iframe *html_get_iframe(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->iframe;
}

/**
 * Retrieve an HTML content's base URL
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to URL
 */
nsurl *html_get_పునాది_url(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->పునాది_url;
}

/**
 * Retrieve an HTML content's base target
 *
 * \param h  Content to retrieve base target from
 * \return Pointer to target, or NULL if none
 */
స్థిరరాశి అక్షరం *html_get_పునాది_లక్ష్యం(hlcache_handle *h)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->పునాది_లక్ష్యం;
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
bool html_get_id_స్థానభ్రంశం(hlcache_handle *h, lwc_string *frag_id, పూర్ణాంకం *x, పూర్ణాంకం *y)
{
	నిర్మాణం box *pos;
	నిర్మాణం box *layout;

	ఒకవేళ (content_get_type(h) != CONTENT_HTML)
		తిరిగి false;

	layout = html_get_box_వృక్షం(h);

	ఒకవేళ ((pos = box_కనుగొను_by_id(layout, frag_id)) != 0) {
		box_coords(pos, x, y);
		తిరిగి true;
	}
	తిరిగి false;
}

bool html_exec(నిర్మాణం content *c, స్థిరరాశి అక్షరం *src, size_t srclen)
{
	html_content *htmlc = (html_content *)c;
	bool ఫలితం = false;
	dom_exception err;
	dom_html_body_element *body_కణుపు;
	dom_string *dom_src;
	dom_పాఠ్యం *పాఠ్యం_కణుపు;
	dom_కణుపు *అదనపు_కణుపు;
	dom_html_script_element *script_కణుపు;

	ఒకవేళ (htmlc->document == ఖాళీ) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, no document");
		వెళ్ళు బహిర్గతం_no_string;
	}

	err = dom_string_create((స్థిరరాశి uint8_t *)src, srclen, &dom_src);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create string");
		వెళ్ళు బహిర్గతం_no_string;
	}

	err = dom_html_document_get_body(htmlc->document, &body_కణుపు);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to retrieve body element");
		వెళ్ళు బహిర్గతం_no_body;
	}

	err = dom_document_create_పాఠ్యం_కణుపు(htmlc->document, dom_src, &పాఠ్యం_కణుపు);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create text node");
		వెళ్ళు బహిర్గతం_no_పాఠ్యం_కణుపు;
	}

	err = dom_document_create_element(htmlc->document, corestring_dom_SCRIPT, &script_కణుపు);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not create script node");
		వెళ్ళు బహిర్గతం_no_script_కణుపు;
	}

	err = dom_కణుపు_append_బిడ్డ(script_కణుపు, పాఠ్యం_కణుపు, &అదనపు_కణుపు);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert code node into script node");
		వెళ్ళు బహిర్గతం_unparented;
	}
	dom_కణుపు_unref(అదనపు_కణుపు); /* We do not need the spare ref at all */

	err = dom_కణుపు_append_బిడ్డ(body_కణుపు, script_కణుపు, &అదనపు_కణుపు);
	ఒకవేళ (err != DOM_NO_ERR) {
		NSLOG(netsurf, DEEPDEBUG, "Unable to exec, could not insert script node into document body");
		వెళ్ళు బహిర్గతం_unparented;
	}
	dom_కణుపు_unref(అదనపు_కణుపు); /* Again no need for the spare ref */

	/* We successfully inserted the node into the DOM */

	ఫలితం = true;

	/* Now we unwind, starting by removing the script from wherever it
	 * ended up parented
	 */

	err = dom_కణుపు_get_మాతృ_కణుపు(script_కణుపు, &అదనపు_కణుపు);
	ఒకవేళ (err == DOM_NO_ERR && అదనపు_కణుపు != ఖాళీ) {
		dom_కణుపు *రెండవ_అదనపు;
		err = dom_కణుపు_remove_బిడ్డ(అదనపు_కణుపు, script_కణుపు, &రెండవ_అదనపు);
		ఒకవేళ (err == DOM_NO_ERR) {
			dom_కణుపు_unref(రెండవ_అదనపు);
		}
		dom_కణుపు_unref(అదనపు_కణుపు);
	}

బహిర్గతం_unparented:
	dom_కణుపు_unref(script_కణుపు);
బహిర్గతం_no_script_కణుపు:
	dom_కణుపు_unref(పాఠ్యం_కణుపు);
బహిర్గతం_no_పాఠ్యం_కణుపు:
	dom_కణుపు_unref(body_కణుపు);
బహిర్గతం_no_body:
	dom_string_unref(dom_src);
బహిర్గతం_no_string:
	తిరిగి ఫలితం;
}

/* See \ref content_saw_insecure_objects */
స్థిర bool
html_saw_insecure_objects(నిర్మాణం content *c)
{
	html_content *htmlc = (html_content *)c;
	నిర్మాణం content_html_object *obj = htmlc->object_list;

	/* Check through the object list */
	అయితే (obj != ఖాళీ) {
		ఒకవేళ (obj->content != ఖాళీ) {
			ఒకవేళ (content_saw_insecure_objects(obj->content))
				తిరిగి true;
		}
		obj = obj->తదుపరి;
	}

	/* Now check the script list */
	ఒకవేళ (html_saw_insecure_scripts(htmlc)) {
		తిరిగి true;
	}

	/* Now check stylesheets */
	ఒకవేళ (html_css_saw_insecure_stylesheets(htmlc)) {
		తిరిగి true;
	}

	తిరిగి false;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_HTML
 */
స్థిర content_type html_content_type(శూన్య)
{
	తిరిగి CONTENT_HTML;
}


స్థిర శూన్య html_fini(శూన్య)
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
స్థిర nserror
కనుగొను_occurrences_html_box(స్థిరరాశి అక్షరం *నమూనా,
			  పూర్ణాంకం p_len,
			  నిర్మాణం box *cur,
			  bool case_sens,
			  నిర్మాణం textsearch_context *context)
{
	నిర్మాణం box *a;
	nserror res = NSERROR_OK;

	/* ignore this box, if there's no visible text */
	ఒకవేళ (!cur->object && cur->పాఠ్యం) {
		స్థిరరాశి అక్షరం *పాఠ్యం = cur->పాఠ్యం;
		గుర్తులేని నిడివి = cur->నిడివి;

		అయితే (నిడివి > 0) {
			గుర్తులేని సరిపోలిక_నిడివి;
			గుర్తులేని సరిపోలిక_స్థానభ్రంశం;
			స్థిరరాశి అక్షరం *new_పాఠ్యం;
			స్థిరరాశి అక్షరం *pos;

			pos = content_textsearch_కనుగొను_నమూనా(పాఠ్యం,
					   నిడివి,
					   నమూనా,
					   p_len,
					   case_sens,
					   &సరిపోలిక_నిడివి);
			ఒకవేళ (!pos)
				ఆపు;

			/* found string in box => add to list */
			సరిపోలిక_స్థానభ్రంశం = pos - cur->పాఠ్యం;

			res = content_textsearch_కూడు_సరిపోలిక(context,
					cur->byte_స్థానభ్రంశం + సరిపోలిక_స్థానభ్రంశం,
					cur->byte_స్థానభ్రంశం + సరిపోలిక_స్థానభ్రంశం + సరిపోలిక_నిడివి,
					cur,
					cur);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}

			new_పాఠ్యం = pos + సరిపోలిక_నిడివి;
			నిడివి -= (new_పాఠ్యం - పాఠ్యం);
			పాఠ్యం = new_పాఠ్యం;
		}
	}

	/* and recurse */
	కొరకు (a = cur->children; a; a = a->తదుపరి) {
		res = కనుగొను_occurrences_html_box(నమూనా,
						p_len,
						a,
						case_sens,
						context);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి res;
		}
	}

	తిరిగి res;
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
స్థిర nserror
html_textsearch_కనుగొను(నిర్మాణం content *c,
		     నిర్మాణం textsearch_context *context,
		     స్థిరరాశి అక్షరం *నమూనా,
		     పూర్ణాంకం p_len,
		     bool csens)
{
	html_content *html = (html_content *)c;

	ఒకవేళ (html->layout == ఖాళీ) {
		తిరిగి NSERROR_INVALID;
	}

	తిరిగి కనుగొను_occurrences_html_box(నమూనా,
					 p_len,
					 html->layout,
					 csens,
					 context);
}


స్థిర nserror
html_textsearch_bounds(నిర్మాణం content *c,
		       గుర్తులేని ప్రారంభం_idx,
		       గుర్తులేని చివర_idx,
		       నిర్మాణం box *ప్రారంభం_box,
		       నిర్మాణం box *చివర_box,
		       నిర్మాణం rect *bounds)
{
	/* get box position and jump to it */
	box_coords(ప్రారంభం_box, &bounds->x0, &bounds->y0);
	/* \todo: move x0 in by correct idx */
	box_coords(చివర_box, &bounds->x1, &bounds->y1);
	/* \todo: move x1 in by correct idx */
	bounds->x1 += చివర_box->వెడల్పు;
	bounds->y1 += చివర_box->ఎత్తు;

	తిరిగి NSERROR_OK;
}


/**
 * HTML content handler function table
 */
స్థిర స్థిరరాశి content_handler html_content_handler = {
	.fini = html_fini,
	.create = html_create,
	.process_దత్తాంశం = html_process_దత్తాంశం,
	.దత్తాంశం_complete = html_convert,
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
	.scroll_at_బిందువు = html_scroll_at_బిందువు,
	.drop_file_at_బిందువు = html_drop_file_at_బిందువు,
	.debug_dump = html_debug_dump,
	.debug = html_debug,
	.clone = html_clone,
	.get_encoding = html_encoding,
	.type = html_content_type,
	.exec = html_exec,
	.saw_insecure_objects = html_saw_insecure_objects,
	.textsearch_కనుగొను = html_textsearch_కనుగొను,
	.textsearch_bounds = html_textsearch_bounds,
	.textselection_redraw = html_textselection_redraw,
	.textselection_copy = html_textselection_copy,
	.textselection_get_చివర = html_textselection_get_చివర,
	.no_share = true,
};


/* exported function documented in html/html.h */
nserror html_init(శూన్య)
{
	uint32_t i;
	nserror error;

	error = html_css_init();
	ఒకవేళ (error != NSERROR_OK)
		వెళ్ళు error;

	కొరకు (i = 0; i < NOF_ELEMENTS(html_types); i++) {
		error = content_factory_రిజిస్టరు_handler(html_types[i],
				&html_content_handler);
		ఒకవేళ (error != NSERROR_OK)
			వెళ్ళు error;
	}

	తిరిగి NSERROR_OK;

error:
	html_fini();

	తిరిగి error;
}
