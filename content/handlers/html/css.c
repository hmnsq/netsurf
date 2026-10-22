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

#ଅନ୍ତର୍ଭାଵ "utils/config.h"

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/inttypes.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/hlcache.h"
#ଅନ୍ତର୍ଭାଵ "css/css.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/css.h"

ସ୍ଥିର nsurl *html_default_stylesheet_url;
ସ୍ଥିର nsurl *html_adblock_stylesheet_url;
ସ୍ଥିର nsurl *html_quirks_stylesheet_url;
ସ୍ଥିର nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
ସ୍ଥିର nserror css_error_to_nserror(css_error error)
{
	ବାଛ (error) {
	ସ୍ଥିତି CSS_OK:
		ଫେରନ୍ତୁ NSERROR_OK;

	ସ୍ଥିତି CSS_NOMEM:
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	ସ୍ଥିତି CSS_BADPARM:
		ଫେରନ୍ତୁ NSERROR_BAD_PARAMETER;

	ସ୍ଥିତି CSS_INVALID:
		ଫେରନ୍ତୁ NSERROR_INVALID;

	ସ୍ଥିତି CSS_FILENOTFOUND:
		ଫେରନ୍ତୁ NSERROR_NOT_FOUND;

	ସ୍ଥିତି CSS_NEEDDATA:
		ଫେରନ୍ତୁ NSERROR_NEED_DATA;

	ସ୍ଥିତି CSS_BADCHARSET:
		ଫେରନ୍ତୁ NSERROR_BAD_ENCODING;

	ସ୍ଥିତି CSS_EOF:
	ସ୍ଥିତି CSS_IMPORTS_PENDING:
	ସ୍ଥିତି CSS_PROPERTY_NOT_SET:
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
ସ୍ଥିର nserror
html_convert_css_callback(hlcache_handle *css,
			  ଅଚଳ hlcache_event *event,
			  ଶୂନ୍ୟ *pw)
{
	html_content *parent = pw;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_stylesheet *s;

	/* Find sheet */
	ପାଇଁ (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_ଗଣନା;
	     i++, s++) {
		ଯଦି (s->sheet == css)
			ଭାଙ୍ଗ;
	}

	ପ୍ରତିଜ୍ଞା(i != parent->stylesheet_ଗଣନା);

	ବାଛ (event->type) {

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = ଖାଲି;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CONTENT_ସନ୍ଦେଶ_POINTER:
		/* Really don't want this to continue after the switch */
		ଫେରନ୍ତୁ NSERROR_OK;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	ଯଦି (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t ଚାବି;
	ଅକ୍ଷର urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_ଗଣ୍ଠି_get_text_content(ଗଣ୍ଠି, &style);
	ଯଦି ((exc != DOM_NO_ERR) || (style == ଖାଲି)) {
		NSLOG(netsurf, INFO, "No text content");
		ଫେରନ୍ତୁ NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &ଚାବି);
	ଯଦି (error != NSERROR_OK) {
		dom_string_unref(style);
		ଫେରନ୍ତୁ error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, ଆକାର(urlbuf), "x-ns-css:%"PRIu32"", ଚାବି);

	error = nsurl_create(urlbuf, &url);
	ଯଦି (error != NSERROR_OK) {
		ଫେରନ୍ତୁ error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), ଖାଲି,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	ଯଦି (error != NSERROR_OK) {
		nsurl_unref(url);
		ଫେରନ୍ତୁ error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
ସ୍ଥିର ସଂରଚନା html_stylesheet *
html_create_style_element(html_content *c, dom_ଗଣ୍ଠି *style)
{
	dom_string *val;
	dom_exception exc;
	ସଂରଚନା html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	ଯଦି (exc == DOM_NO_ERR && val != ଖାଲି) {
		ଯଦି (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	ଯଦି (exc == DOM_NO_ERR && val != ଖାଲି) {
		ଯଦି (strcasestr(dom_string_data(val), "screen") == ଖାଲି &&
				strcasestr(dom_string_data(val),
						"all") == ଖାଲି) {
			dom_string_unref(val);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      ଆକାର(ସଂରଚନା html_stylesheet) *
			      (c->stylesheet_ଗଣନା + 1));
	ଯଦି (stylesheets == ଖାଲି) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_ଗଣନା].ଗଣ୍ଠି = dom_ଗଣ୍ଠି_ref(style);
	c->stylesheets[c->stylesheet_ଗଣନା].sheet = ଖାଲି;
	c->stylesheets[c->stylesheet_ଗଣନା].modified = false;
	c->stylesheets[c->stylesheet_ଗଣନା].unused = false;
	c->stylesheet_ଗଣନା++;

	ଫେରନ୍ତୁ c->stylesheets + (c->stylesheet_ଗଣନା - 1);
}


ସ୍ଥିର bool
html_css_process_modified_style(html_content *c, ସଂରଚନା html_stylesheet *s)
{
	hlcache_handle *sheet = ଖାଲି;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ଗଣ୍ଠି, &sheet);
	ଯଦି (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (sheet != ଖାଲି) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		ଯଦି (s->sheet != ଖାଲି) {
			ବାଛ (content_get_status(s->sheet)) {
			ସ୍ଥିତି CONTENT_STATUS_DONE:
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				hlcache_handle_abort(s->sheet);
				c->base.active--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->base.active);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	ଫେରନ୍ତୁ true;
}


/**
 * process a stylesheet that has been modified.
 */
ସ୍ଥିର ଶୂନ୍ୟ html_css_process_modified_styles(ଶୂନ୍ୟ *pw)
{
	html_content *c = pw;
	ସଂରଚନା html_stylesheet *s;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	bool all_done = true;

	ପାଇଁ (i = 0, s = c->stylesheets; i != c->stylesheet_ଗଣନା; i++, s++) {
		ଯଦି (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	ଯଦି (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ଗଣ୍ଠି *style)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା html_stylesheet *s;

	/* Find sheet */
	ପାଇଁ (i = 0, s = c->stylesheets;	i != c->stylesheet_ଗଣନା; i++, s++) {
		ଯଦି (s->ଗଣ୍ଠି == style)
			ଭାଙ୍ଗ;
	}
	ଯଦି (i == c->stylesheet_ଗଣନା) {
		s = html_create_style_element(c, style);
	}
	ଯଦି (s == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		ଫେରନ୍ତୁ false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	ଫେରନ୍ତୁ true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	dom_string *val;
	dom_exception exc;
	ସଂରଚନା html_stylesheet *s;

	/* Find sheet */
	ପାଇଁ (i = 0, s = c->stylesheets;	i != c->stylesheet_ଗଣନା; i++, s++) {
		ଯଦି (s->ଗଣ୍ଠି == ଗଣ୍ଠି)
			ଭାଙ୍ଗ;
	}

		/* Should already exist */
	ଯଦି (i == c->stylesheet_ଗଣନା) {
		ଫେରନ୍ତୁ false;
	}

	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_media, &val);
	ଯଦି (exc == DOM_NO_ERR && val != ଖାଲି) {
		ଯଦି (strcasestr(dom_string_data(val), "screen") == ଖାଲି &&
				strcasestr(dom_string_data(val),
						"all") == ଖାଲି) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	ଫେରନ୍ତୁ true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	dom_string *rel, *type_attr, *media, *href;
	ସଂରଚନା html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_rel, &rel);
	ଯଦି (exc != DOM_NO_ERR || rel == ଖାଲି)
		ଫେରନ୍ତୁ true;

	ଯଦି (strcasestr(dom_string_data(rel), "stylesheet") == ଖାଲି) {
		dom_string_unref(rel);
		ଫେରନ୍ତୁ true;
	} ଅନ୍ୟଥା ଯଦି (strcasestr(dom_string_data(rel), "alternate") != ଖାଲି) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		ଫେରନ୍ତୁ true;
	}
	dom_string_unref(rel);

	ଯଦି (nsoption_bool(author_level_css) == false) {
		ଫେରନ୍ତୁ true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_type, &type_attr);
	ଯଦି (exc == DOM_NO_ERR && type_attr != ଖାଲି) {
		ଯଦି (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			ଫେରନ୍ତୁ true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_media, &media);
	ଯଦି (exc == DOM_NO_ERR && media != ଖାଲି) {
		ଯଦି (strcasestr(dom_string_data(media), "screen") == ଖାଲି &&
		    strcasestr(dom_string_data(media), "all") == ଖାଲି) {
			dom_string_unref(media);
			ଫେରନ୍ତୁ true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ଗଣ୍ଠି, corestring_dom_href, &href);
	ଯଦି (exc != DOM_NO_ERR || href == ଖାଲି)
		ଫେରନ୍ତୁ true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	ଯଦି (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		ଯାଅ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_ଗଣନା, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      ଆକାର(ସଂରଚନା html_stylesheet) *
			      (htmlc->stylesheet_ଗଣନା + 1));
	ଯଦି (stylesheets == ଖାଲି) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		ଯାଅ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_ଗଣନା].ଗଣ୍ଠି = ଖାଲି;
	htmlc->stylesheets[htmlc->stylesheet_ଗଣନା].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_ଗଣନା].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			ଖାଲି, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_ଗଣନା].sheet);

	nsurl_unref(joined);

	ଯଦି (ns_error != NSERROR_OK)
		ଯାଅ no_memory;

	htmlc->stylesheet_ଗଣନା++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	ଫେରନ୍ତୁ true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, ଖାଲି);
	ଫେରନ୍ତୁ false;
}


/* exported interface documented in html/html.h */
ସଂରଚନା html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	ପ୍ରତିଜ୍ଞା(c != ଖାଲି);
	ପ୍ରତିଜ୍ଞା(n != ଖାଲି);

	*n = c->stylesheet_ଗଣନା;

	ଫେରନ୍ତୁ c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	ସଂରଚନା html_stylesheet *s;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0, s = html->stylesheets; i < html->stylesheet_ଗଣନା;
	     i++, s++) {
		ଯଦି (s->sheet != ଖାଲି) {
			ଯଦି (content_saw_insecure_objects(s->sheet)) {
				ଫେରନ୍ତୁ true;
			}
		}
	}

	ଫେରନ୍ତୁ false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	ପାଇଁ (i = 0; i != html->stylesheet_ଗଣନା; i++) {
		ଯଦି (html->stylesheets[i].sheet != ଖାଲି) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		ଯଦି (html->stylesheets[i].ଗଣ୍ଠି != ଖାଲି) {
			dom_ଗଣ୍ଠି_unref(html->stylesheets[i].ଗଣ୍ଠି);
		}
	}
	ମୁକ୍ତ(html->stylesheets);

	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	ପ୍ରତିଜ୍ଞା(c->stylesheets != ଖାଲି);

	ଯଦି (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), ଖାଲି,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		ଯଦି (ns_error != NSERROR_OK) {
			ଫେରନ୍ତୁ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	ଫେରନ୍ତୁ ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	ଯଦି (c->stylesheets != ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			ଆକାର(ସଂରଚନା html_stylesheet));
	ଯଦି (c->stylesheets == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = ଖାଲି;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = ଖାଲି;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = ଖାଲି;
	c->stylesheets[STYLESHEET_USER].sheet = ଖାଲି;
	c->stylesheet_ଗଣନା = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), ଖାଲି,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	ଯଦି (ns_error != NSERROR_OK) {
		ଫେରନ୍ତୁ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	ଯଦି (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), ଖାଲି,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		ଯଦି (ns_error != NSERROR_OK) {
			ଫେରନ୍ତୁ ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), ଖାଲି,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	ଯଦି (ns_error != NSERROR_OK) {
		ଫେରନ୍ତୁ ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	ଫେରନ୍ତୁ ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	ଯଦି (c->stylesheets[STYLESHEET_BASE].sheet == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	ଯଦି (css_ret != CSS_OK) {
		ଫେରନ୍ତୁ css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	ପାଇଁ (i = STYLESHEET_BASE; i != c->stylesheet_ଗଣନା; i++) {
		ଅଚଳ ସଂରଚନା html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = ଖାଲି;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		ଯଦି (hsheet->unused) {
			ଜାରି;
		}

		ଯଦି (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} ଅନ୍ୟଥା ଯଦି (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		ଯଦି (hsheet->sheet != ଖାଲି) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		ଯଦି (sheet != ଖାଲି) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			ଯଦି (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				ଫେରନ୍ତୁ css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	ଫେରନ୍ତୁ NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(ଶୂନ୍ୟ)
{
	nserror error;

	error = html_css_fetcher_register();
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	ଫେରନ୍ତୁ error;
}


/* exported function documented in html/css.h */
ଶୂନ୍ୟ html_css_fini(ଶୂନ୍ୟ)
{
	ଯଦି (html_user_stylesheet_url != ଖାଲି) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = ଖାଲି;
	}

	ଯଦି (html_quirks_stylesheet_url != ଖାଲି) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = ଖାଲି;
	}

	ଯଦି (html_adblock_stylesheet_url != ଖାଲି) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = ଖାଲି;
	}

	ଯଦି (html_default_stylesheet_url != ଖାଲି) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = ଖାଲି;
	}
}
