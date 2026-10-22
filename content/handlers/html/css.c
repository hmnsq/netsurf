/*
 * Copyright 2013 Vincent Sanders <vince@netsurf-browser.org>
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
 * Processing for html content css operations.
 */

#అన్తర్భావ "utils/config.h"

#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "netsurf/inttypes.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "css/css.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/css.h"

స్థిర nsurl *html_default_stylesheet_url;
స్థిర nsurl *html_adblock_stylesheet_url;
స్థిర nsurl *html_quirks_stylesheet_url;
స్థిర nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
స్థిర nserror css_error_గమ్యం_nserror(css_error error)
{
	ఎంపిక (error) {
	స్థితి CSS_OK:
		తిరిగి NSERROR_OK;

	స్థితి CSS_NOMEM:
		తిరిగి NSERROR_NOMEM;

	స్థితి CSS_BADPARM:
		తిరిగి NSERROR_BAD_PARAMETER;

	స్థితి CSS_INVALID:
		తిరిగి NSERROR_INVALID;

	స్థితి CSS_FILENOTFOUND:
		తిరిగి NSERROR_NOT_దొరికింది;

	స్థితి CSS_NEEDDATA:
		తిరిగి NSERROR_NEED_దత్తాంశం;

	స్థితి CSS_BADCHARSET:
		తిరిగి NSERROR_BAD_ENCODING;

	స్థితి CSS_EOF:
	స్థితి CSS_IMPORTS_PENDING:
	స్థితి CSS_PROPERTY_NOT_సమితి:
	అప్రమేయ:
		ఆపు;
	}
	తిరిగి NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
స్థిర nserror
html_convert_css_callback(hlcache_handle *css,
			  స్థిరరాశి hlcache_event *event,
			  శూన్య *pw)
{
	html_content *మాతృ = pw;
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_stylesheet *s;

	/* Find sheet */
	కొరకు (i = 0, s = మాతృ->stylesheets;
	     i != మాతృ->stylesheet_లెక్క;
	     i++, s++) {
		ఒకవేళ (s->sheet == css)
			ఆపు;
	}

	ప్రతిజ్ఞా(i != మాతృ->stylesheet_లెక్క);

	ఎంపిక (event->type) {

	స్థితి CONTENT_సందేశం_పూర్తి:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);
		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->దత్తాంశం.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ఖాళీ;
		మాతృ->పునాది.చురుకైన--;
		NSLOG(netsurf, INFO, "%d fetches active", మాతృ->పునాది.చురుకైన);
		ఆపు;

	స్థితి CONTENT_సందేశం_సూచిక:
		/* Really don't want this to continue after the switch */
		తిరిగి NSERROR_OK;

	అప్రమేయ:
		ఆపు;
	}

	ఒకవేళ (html_can_begin_conversion(మాతృ)) {
		html_begin_conversion(మాతృ);
	}

	తిరిగి NSERROR_OK;
}


స్థిర nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_కణుపు *కణుపు,
			     hlcache_handle **sheet)
{
	hlcache_బిడ్డ_context బిడ్డ;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t కీలకం;
	అక్షరం urlbuf[64];

	బిడ్డ.charset = c->encoding;
	బిడ్డ.quirks = c->పునాది.quirks;

	exc = dom_కణుపు_get_పాఠ్యం_content(కణుపు, &style);
	ఒకవేళ ((exc != DOM_NO_ERR) || (style == ఖాళీ)) {
		NSLOG(netsurf, INFO, "No text content");
		తిరిగి NSERROR_OK;
	}

	error = html_css_fetcher_కూడు_అంశం(style, c->పునాది_url, &కీలకం);
	ఒకవేళ (error != NSERROR_OK) {
		dom_string_unref(style);
		తిరిగి error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, పరిమాణం(urlbuf), "x-ns-css:%"PRIu32"", కీలకం);

	error = nsurl_create(urlbuf, &url);
	ఒకవేళ (error != NSERROR_OK) {
		తిరిగి error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->పునాది), ఖాళీ,
			html_convert_css_callback, c, &బిడ్డ, CONTENT_CSS,
			sheet);
	ఒకవేళ (error != NSERROR_OK) {
		nsurl_unref(url);
		తిరిగి error;
	}

	nsurl_unref(url);

	c->పునాది.చురుకైన++;
	NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

	తిరిగి NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
స్థిర నిర్మాణం html_stylesheet *
html_create_style_element(html_content *c, dom_కణుపు *style)
{
	dom_string *val;
	dom_exception exc;
	నిర్మాణం html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ఒకవేళ (exc == DOM_NO_ERR && val != ఖాళీ) {
		ఒకవేళ (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_పాఠ్యం_css)) {
			dom_string_unref(val);
			తిరిగి ఖాళీ;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ఒకవేళ (exc == DOM_NO_ERR && val != ఖాళీ) {
		ఒకవేళ (strcasestr(dom_string_దత్తాంశం(val), "screen") == ఖాళీ &&
				strcasestr(dom_string_దత్తాంశం(val),
						"all") == ఖాళీ) {
			dom_string_unref(val);
			తిరిగి ఖాళీ;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      పరిమాణం(నిర్మాణం html_stylesheet) *
			      (c->stylesheet_లెక్క + 1));
	ఒకవేళ (stylesheets == ఖాళీ) {

		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_లెక్క].కణుపు = dom_కణుపు_ref(style);
	c->stylesheets[c->stylesheet_లెక్క].sheet = ఖాళీ;
	c->stylesheets[c->stylesheet_లెక్క].modified = false;
	c->stylesheets[c->stylesheet_లెక్క].వాడని = false;
	c->stylesheet_లెక్క++;

	తిరిగి c->stylesheets + (c->stylesheet_లెక్క - 1);
}


స్థిర bool
html_css_process_modified_style(html_content *c, నిర్మాణం html_stylesheet *s)
{
	hlcache_handle *sheet = ఖాళీ;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->కణుపు, &sheet);
	ఒకవేళ (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_ప్రసారం_error(&c->పునాది, error, ఖాళీ);
		తిరిగి false;
	}

	ఒకవేళ (sheet != ఖాళీ) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ఒకవేళ (s->sheet != ఖాళీ) {
			ఎంపిక (content_get_స్థితి(s->sheet)) {
			స్థితి CONTENT_స్థితి_పూర్తి:
				ఆపు;
			అప్రమేయ:
				hlcache_handle_abort(s->sheet);
				c->పునాది.చురుకైన--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->పునాది.చురుకైన);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	తిరిగి true;
}


/**
 * process a stylesheet that has been modified.
 */
స్థిర శూన్య html_css_process_modified_styles(శూన్య *pw)
{
	html_content *c = pw;
	నిర్మాణం html_stylesheet *s;
	గుర్తులేని పూర్ణాంకం i;
	bool all_పూర్తి = true;

	కొరకు (i = 0, s = c->stylesheets; i != c->stylesheet_లెక్క; i++, s++) {
		ఒకవేళ (c->stylesheets[i].modified) {
			all_పూర్తి &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ఒకవేళ (all_పూర్తి == false) {
		guit->misc->కాలపట్టిక(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_కణుపు *style)
{
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం html_stylesheet *s;

	/* Find sheet */
	కొరకు (i = 0, s = c->stylesheets;	i != c->stylesheet_లెక్క; i++, s++) {
		ఒకవేళ (s->కణుపు == style)
			ఆపు;
	}
	ఒకవేళ (i == c->stylesheet_లెక్క) {
		s = html_create_style_element(c, style);
	}
	ఒకవేళ (s == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		తిరిగి false;
	}

	s->modified = true;

	guit->misc->కాలపట్టిక(0, html_css_process_modified_styles, c);

	తిరిగి true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_కణుపు *కణుపు)
{
	గుర్తులేని పూర్ణాంకం i;
	dom_string *val;
	dom_exception exc;
	నిర్మాణం html_stylesheet *s;

	/* Find sheet */
	కొరకు (i = 0, s = c->stylesheets;	i != c->stylesheet_లెక్క; i++, s++) {
		ఒకవేళ (s->కణుపు == కణుపు)
			ఆపు;
	}

		/* Should already exist */
	ఒకవేళ (i == c->stylesheet_లెక్క) {
		తిరిగి false;
	}

	exc = dom_element_get_attribute(కణుపు, corestring_dom_media, &val);
	ఒకవేళ (exc == DOM_NO_ERR && val != ఖాళీ) {
		ఒకవేళ (strcasestr(dom_string_దత్తాంశం(val), "screen") == ఖాళీ &&
				strcasestr(dom_string_దత్తాంశం(val),
						"all") == ఖాళీ) {
			s->వాడని = true;
		}
		dom_string_unref(val);
	}

	తిరిగి true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_కణుపు *కణుపు)
{
	dom_string *rel, *type_attr, *media, *href;
	నిర్మాణం html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_బిడ్డ_context బిడ్డ;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_rel, &rel);
	ఒకవేళ (exc != DOM_NO_ERR || rel == ఖాళీ)
		తిరిగి true;

	ఒకవేళ (strcasestr(dom_string_దత్తాంశం(rel), "stylesheet") == ఖాళీ) {
		dom_string_unref(rel);
		తిరిగి true;
	} లేకపోతే ఒకవేళ (strcasestr(dom_string_దత్తాంశం(rel), "alternate") != ఖాళీ) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		తిరిగి true;
	}
	dom_string_unref(rel);

	ఒకవేళ (nsoption_bool(author_level_css) == false) {
		తిరిగి true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_type, &type_attr);
	ఒకవేళ (exc == DOM_NO_ERR && type_attr != ఖాళీ) {
		ఒకవేళ (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_పాఠ్యం_css)) {
			dom_string_unref(type_attr);
			తిరిగి true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_media, &media);
	ఒకవేళ (exc == DOM_NO_ERR && media != ఖాళీ) {
		ఒకవేళ (strcasestr(dom_string_దత్తాంశం(media), "screen") == ఖాళీ &&
		    strcasestr(dom_string_దత్తాంశం(media), "all") == ఖాళీ) {
			dom_string_unref(media);
			తిరిగి true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(కణుపు, corestring_dom_href, &href);
	ఒకవేళ (exc != DOM_NO_ERR || href == ఖాళీ)
		తిరిగి true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->పునాది_url, dom_string_దత్తాంశం(href), &joined);
	ఒకవేళ (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		వెళ్ళు no_జ్ఞాపకం;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_లెక్క, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      పరిమాణం(నిర్మాణం html_stylesheet) *
			      (htmlc->stylesheet_లెక్క + 1));
	ఒకవేళ (stylesheets == ఖాళీ) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		వెళ్ళు no_జ్ఞాపకం;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_లెక్క].కణుపు = ఖాళీ;
	htmlc->stylesheets[htmlc->stylesheet_లెక్క].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_లెక్క].వాడని = false;

	/* start fetch */
	బిడ్డ.charset = htmlc->encoding;
	బిడ్డ.quirks = htmlc->పునాది.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->పునాది),
			ఖాళీ, html_convert_css_callback,
			htmlc, &బిడ్డ, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_లెక్క].sheet);

	nsurl_unref(joined);

	ఒకవేళ (ns_error != NSERROR_OK)
		వెళ్ళు no_జ్ఞాపకం;

	htmlc->stylesheet_లెక్క++;

	htmlc->పునాది.చురుకైన++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->పునాది.చురుకైన);

	తిరిగి true;

no_జ్ఞాపకం:
	content_ప్రసారం_error(&htmlc->పునాది, ns_error, ఖాళీ);
	తిరిగి false;
}


/* exported interface documented in html/html.h */
నిర్మాణం html_stylesheet *html_get_stylesheets(hlcache_handle *h, గుర్తులేని పూర్ణాంకం *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);
	ప్రతిజ్ఞా(n != ఖాళీ);

	*n = c->stylesheet_లెక్క;

	తిరిగి c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	నిర్మాణం html_stylesheet *s;
	గుర్తులేని పూర్ణాంకం i;

	కొరకు (i = 0, s = html->stylesheets; i < html->stylesheet_లెక్క;
	     i++, s++) {
		ఒకవేళ (s->sheet != ఖాళీ) {
			ఒకవేళ (content_saw_insecure_objects(s->sheet)) {
				తిరిగి true;
			}
		}
	}

	తిరిగి false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	గుర్తులేని పూర్ణాంకం i;

	guit->misc->కాలపట్టిక(-1, html_css_process_modified_styles, html);

	కొరకు (i = 0; i != html->stylesheet_లెక్క; i++) {
		ఒకవేళ (html->stylesheets[i].sheet != ఖాళీ) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ఒకవేళ (html->stylesheets[i].కణుపు != ఖాళీ) {
			dom_కణుపు_unref(html->stylesheets[i].కణుపు);
		}
	}
	విడుదల(html->stylesheets);

	తిరిగి NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_బిడ్డ_context బిడ్డ;

	ప్రతిజ్ఞా(c->stylesheets != ఖాళీ);

	ఒకవేళ (c->quirks == DOM_DOCUMENT_QUIRKS_బహుళకం_నిండిన) {
		బిడ్డ.charset = c->encoding;
		బిడ్డ.quirks = c->పునాది.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->పునాది), ఖాళీ,
				html_convert_css_callback, c, &బిడ్డ,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ఒకవేళ (ns_error != NSERROR_OK) {
			తిరిగి ns_error;
		}

		c->పునాది.చురుకైన++;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);
	}

	తిరిగి ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_బిడ్డ_context బిడ్డ;

	ఒకవేళ (c->stylesheets != ఖాళీ) {
		తిరిగి NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = సున్నా_జ్ఞాపకం_ఇవ్వు(STYLESHEET_ప్రారంభం,
			పరిమాణం(నిర్మాణం html_stylesheet));
	ఒకవేళ (c->stylesheets == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_పునాది].sheet = ఖాళీ;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ఖాళీ;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ఖాళీ;
	c->stylesheets[STYLESHEET_USER].sheet = ఖాళీ;
	c->stylesheet_లెక్క = STYLESHEET_ప్రారంభం;

	బిడ్డ.charset = c->encoding;
	బిడ్డ.quirks = c->పునాది.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->పునాది), ఖాళీ,
			html_convert_css_callback, c, &బిడ్డ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_పునాది].sheet);
	ఒకవేళ (ns_error != NSERROR_OK) {
		తిరిగి ns_error;
	}

	c->పునాది.చురుకైన++;
	NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);


	ఒకవేళ (nsoption_bool(ఖండం_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->పునాది), ఖాళీ,
				html_convert_css_callback,
				c, &బిడ్డ, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ఒకవేళ (ns_error != NSERROR_OK) {
			తిరిగి ns_error;
		}

		c->పునాది.చురుకైన++;
		NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->పునాది), ఖాళీ,
			html_convert_css_callback, c, &బిడ్డ, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ఒకవేళ (ns_error != NSERROR_OK) {
		తిరిగి ns_error;
	}

	c->పునాది.చురుకైన++;
	NSLOG(netsurf, INFO, "%d fetches active", c->పునాది.చురుకైన);

	తిరిగి ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ఒకవేళ (c->stylesheets[STYLESHEET_పునాది].sheet == ఖాళీ) {
		తిరిగి NSERROR_CSS_పునాది;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ఒకవేళ (css_ret != CSS_OK) {
		తిరిగి css_error_గమ్యం_nserror(css_ret);
	}

	/* Add sheets to it */
	కొరకు (i = STYLESHEET_పునాది; i != c->stylesheet_లెక్క; i++) {
		స్థిరరాశి నిర్మాణం html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ఖాళీ;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ఒకవేళ (hsheet->వాడని) {
			కొనసాగు;
		}

		ఒకవేళ (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} లేకపోతే ఒకవేళ (i < STYLESHEET_ప్రారంభం) {
			origin = CSS_ORIGIN_USER;
		}

		ఒకవేళ (hsheet->sheet != ఖాళీ) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ఒకవేళ (sheet != ఖాళీ) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ఒకవేళ (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				తిరిగి css_error_గమ్యం_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	తిరిగి NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(శూన్య)
{
	nserror error;

	error = html_css_fetcher_రిజిస్టరు();
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ఒకవేళ (error != NSERROR_OK)
		తిరిగి error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	తిరిగి error;
}


/* exported function documented in html/css.h */
శూన్య html_css_fini(శూన్య)
{
	ఒకవేళ (html_user_stylesheet_url != ఖాళీ) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ఖాళీ;
	}

	ఒకవేళ (html_quirks_stylesheet_url != ఖాళీ) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ఖాళీ;
	}

	ఒకవేళ (html_adblock_stylesheet_url != ఖాళీ) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ఖాళీ;
	}

	ఒకవేళ (html_default_stylesheet_url != ఖాళీ) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ఖాళీ;
	}
}
