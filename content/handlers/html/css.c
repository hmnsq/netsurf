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

#अन्तर्भाव "utils/config.h"

#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "netsurf/inttypes.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "css/css.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/css.h"

स्थिर nsurl *html_default_stylesheet_url;
स्थिर nsurl *html_adblock_stylesheet_url;
स्थिर nsurl *html_quirks_stylesheet_url;
स्थिर nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
स्थिर nserror css_error_गन्तव्यम्_nserror(css_error error)
{
	विकल्प (error) {
	पक्ष CSS_OK:
		प्रत्यागम NSERROR_OK;

	पक्ष CSS_NOMEM:
		प्रत्यागम NSERROR_NOMEM;

	पक्ष CSS_BADPARM:
		प्रत्यागम NSERROR_BAD_PARAMETER;

	पक्ष CSS_INVALID:
		प्रत्यागम NSERROR_INVALID;

	पक्ष CSS_FILENOTFOUND:
		प्रत्यागम NSERROR_NOT_प्राप्तम्;

	पक्ष CSS_NEEDDATA:
		प्रत्यागम NSERROR_NEED_दत्त;

	पक्ष CSS_BADCHARSET:
		प्रत्यागम NSERROR_BAD_ENCODING;

	पक्ष CSS_EOF:
	पक्ष CSS_IMPORTS_PENDING:
	पक्ष CSS_PROPERTY_NOT_समुच्चयः:
	स्वभाव:
		भङ्ग;
	}
	प्रत्यागम NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
स्थिर nserror
html_convert_css_callback(hlcache_handle *css,
			  ध्रुव hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	अचिह्नित पूर्णाङ्क i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	प्रति (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_गणनम्;
	     i++, s++) {
		यदि (s->sheet == css)
			भङ्ग;
	}

	प्रतिज्ञा(i != parent->stylesheet_गणनम्);

	विकल्प (event->type) {

	पक्ष CONTENT_सन्देशः_समाप्तम्:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->आधारः.सक्रिय--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->आधारः.सक्रिय);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->दत्त.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = रिक्त;
		parent->आधारः.सक्रिय--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->आधारः.सक्रिय);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_POINTER:
		/* Really don't want this to continue after the switch */
		प्रत्यागम NSERROR_OK;

	स्वभाव:
		भङ्ग;
	}

	यदि (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	प्रत्यागम NSERROR_OK;
}


स्थिर nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_ग्रन्थिः *ग्रन्थिः,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t कुञ्चिका;
	वर्ण urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->आधारः.quirks;

	exc = dom_ग्रन्थिः_get_text_content(ग्रन्थिः, &style);
	यदि ((exc != DOM_NO_ERR) || (style == रिक्त)) {
		NSLOG(netsurf, INFO, "No text content");
		प्रत्यागम NSERROR_OK;
	}

	error = html_css_fetcher_सङ्कलय_item(style, c->आधारः_url, &कुञ्चिका);
	यदि (error != NSERROR_OK) {
		dom_string_unref(style);
		प्रत्यागम error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, परिमाण(urlbuf), "x-ns-css:%"PRIu32"", कुञ्चिका);

	error = nsurl_create(urlbuf, &url);
	यदि (error != NSERROR_OK) {
		प्रत्यागम error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->आधारः), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	यदि (error != NSERROR_OK) {
		nsurl_unref(url);
		प्रत्यागम error;
	}

	nsurl_unref(url);

	c->आधारः.सक्रिय++;
	NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

	प्रत्यागम NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
स्थिर संरचना html_stylesheet *
html_create_style_element(html_content *c, dom_ग्रन्थिः *style)
{
	dom_string *val;
	dom_exception exc;
	संरचना html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	यदि (exc == DOM_NO_ERR && val != रिक्त) {
		यदि (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			प्रत्यागम रिक्त;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	यदि (exc == DOM_NO_ERR && val != रिक्त) {
		यदि (strcasestr(dom_string_दत्त(val), "screen") == रिक्त &&
				strcasestr(dom_string_दत्त(val),
						"all") == रिक्त) {
			dom_string_unref(val);
			प्रत्यागम रिक्त;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      परिमाण(संरचना html_stylesheet) *
			      (c->stylesheet_गणनम् + 1));
	यदि (stylesheets == रिक्त) {

		content_प्रसारणम्_error(&c->आधारः, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_गणनम्].ग्रन्थिः = dom_ग्रन्थिः_ref(style);
	c->stylesheets[c->stylesheet_गणनम्].sheet = रिक्त;
	c->stylesheets[c->stylesheet_गणनम्].modified = false;
	c->stylesheets[c->stylesheet_गणनम्].अप्रयुक्तम् = false;
	c->stylesheet_गणनम्++;

	प्रत्यागम c->stylesheets + (c->stylesheet_गणनम् - 1);
}


स्थिर bool
html_css_process_modified_style(html_content *c, संरचना html_stylesheet *s)
{
	hlcache_handle *sheet = रिक्त;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ग्रन्थिः, &sheet);
	यदि (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_प्रसारणम्_error(&c->आधारः, error, रिक्त);
		प्रत्यागम false;
	}

	यदि (sheet != रिक्त) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		यदि (s->sheet != रिक्त) {
			विकल्प (content_get_स्थितिः(s->sheet)) {
			पक्ष CONTENT_स्थितिः_समाप्तम्:
				भङ्ग;
			स्वभाव:
				hlcache_handle_abort(s->sheet);
				c->आधारः.सक्रिय--;
				NSLOG(netsurf, INFO, "%d fetches active",
				      c->आधारः.सक्रिय);
			}
			hlcache_handle_release(s->sheet);
		}
		s->sheet = sheet;
	}

	s->modified = false;

	प्रत्यागम true;
}


/**
 * process a stylesheet that has been modified.
 */
स्थिर शून्य html_css_process_modified_styles(शून्य *pw)
{
	html_content *c = pw;
	संरचना html_stylesheet *s;
	अचिह्नित पूर्णाङ्क i;
	bool all_समाप्तम् = true;

	प्रति (i = 0, s = c->stylesheets; i != c->stylesheet_गणनम्; i++, s++) {
		यदि (c->stylesheets[i].modified) {
			all_समाप्तम् &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	यदि (all_समाप्तम् == false) {
		guit->misc->कालसारणी(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ग्रन्थिः *style)
{
	अचिह्नित पूर्णाङ्क i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	प्रति (i = 0, s = c->stylesheets;	i != c->stylesheet_गणनम्; i++, s++) {
		यदि (s->ग्रन्थिः == style)
			भङ्ग;
	}
	यदि (i == c->stylesheet_गणनम्) {
		s = html_create_style_element(c, style);
	}
	यदि (s == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		प्रत्यागम false;
	}

	s->modified = true;

	guit->misc->कालसारणी(0, html_css_process_modified_styles, c);

	प्रत्यागम true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ग्रन्थिः *ग्रन्थिः)
{
	अचिह्नित पूर्णाङ्क i;
	dom_string *val;
	dom_exception exc;
	संरचना html_stylesheet *s;

	/* Find sheet */
	प्रति (i = 0, s = c->stylesheets;	i != c->stylesheet_गणनम्; i++, s++) {
		यदि (s->ग्रन्थिः == ग्रन्थिः)
			भङ्ग;
	}

		/* Should already exist */
	यदि (i == c->stylesheet_गणनम्) {
		प्रत्यागम false;
	}

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_media, &val);
	यदि (exc == DOM_NO_ERR && val != रिक्त) {
		यदि (strcasestr(dom_string_दत्त(val), "screen") == रिक्त &&
				strcasestr(dom_string_दत्त(val),
						"all") == रिक्त) {
			s->अप्रयुक्तम् = true;
		}
		dom_string_unref(val);
	}

	प्रत्यागम true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_ग्रन्थिः *ग्रन्थिः)
{
	dom_string *rel, *type_attr, *media, *href;
	संरचना html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_rel, &rel);
	यदि (exc != DOM_NO_ERR || rel == रिक्त)
		प्रत्यागम true;

	यदि (strcasestr(dom_string_दत्त(rel), "stylesheet") == रिक्त) {
		dom_string_unref(rel);
		प्रत्यागम true;
	} अन्यथा यदि (strcasestr(dom_string_दत्त(rel), "alternate") != रिक्त) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		प्रत्यागम true;
	}
	dom_string_unref(rel);

	यदि (nsoption_bool(author_level_css) == false) {
		प्रत्यागम true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_type, &type_attr);
	यदि (exc == DOM_NO_ERR && type_attr != रिक्त) {
		यदि (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			प्रत्यागम true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_media, &media);
	यदि (exc == DOM_NO_ERR && media != रिक्त) {
		यदि (strcasestr(dom_string_दत्त(media), "screen") == रिक्त &&
		    strcasestr(dom_string_दत्त(media), "all") == रिक्त) {
			dom_string_unref(media);
			प्रत्यागम true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_href, &href);
	यदि (exc != DOM_NO_ERR || href == रिक्त)
		प्रत्यागम true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->आधारः_url, dom_string_दत्त(href), &joined);
	यदि (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		गच्छ no_स्मृतिः;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_गणनम्, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      परिमाण(संरचना html_stylesheet) *
			      (htmlc->stylesheet_गणनम् + 1));
	यदि (stylesheets == रिक्त) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		गच्छ no_स्मृतिः;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].ग्रन्थिः = रिक्त;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].अप्रयुक्तम् = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->आधारः.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->आधारः),
			रिक्त, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_गणनम्].sheet);

	nsurl_unref(joined);

	यदि (ns_error != NSERROR_OK)
		गच्छ no_स्मृतिः;

	htmlc->stylesheet_गणनम्++;

	htmlc->आधारः.सक्रिय++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->आधारः.सक्रिय);

	प्रत्यागम true;

no_स्मृतिः:
	content_प्रसारणम्_error(&htmlc->आधारः, ns_error, रिक्त);
	प्रत्यागम false;
}


/* exported interface documented in html/html.h */
संरचना html_stylesheet *html_get_stylesheets(hlcache_handle *h, अचिह्नित पूर्णाङ्क *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->stylesheet_गणनम्;

	प्रत्यागम c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	संरचना html_stylesheet *s;
	अचिह्नित पूर्णाङ्क i;

	प्रति (i = 0, s = html->stylesheets; i < html->stylesheet_गणनम्;
	     i++, s++) {
		यदि (s->sheet != रिक्त) {
			यदि (content_saw_insecure_objects(s->sheet)) {
				प्रत्यागम true;
			}
		}
	}

	प्रत्यागम false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	अचिह्नित पूर्णाङ्क i;

	guit->misc->कालसारणी(-1, html_css_process_modified_styles, html);

	प्रति (i = 0; i != html->stylesheet_गणनम्; i++) {
		यदि (html->stylesheets[i].sheet != रिक्त) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		यदि (html->stylesheets[i].ग्रन्थिः != रिक्त) {
			dom_ग्रन्थिः_unref(html->stylesheets[i].ग्रन्थिः);
		}
	}
	मुक्त(html->stylesheets);

	प्रत्यागम NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	प्रतिज्ञा(c->stylesheets != रिक्त);

	यदि (c->quirks == DOM_DOCUMENT_QUIRKS_बहुलकम्_पूर्णम्) {
		child.charset = c->encoding;
		child.quirks = c->आधारः.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->आधारः), रिक्त,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		यदि (ns_error != NSERROR_OK) {
			प्रत्यागम ns_error;
		}

		c->आधारः.सक्रिय++;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);
	}

	प्रत्यागम ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	यदि (c->stylesheets != रिक्त) {
		प्रत्यागम NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = शून्यस्मृतिं_देहि(STYLESHEET_START,
			परिमाण(संरचना html_stylesheet));
	यदि (c->stylesheets == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_आधारः].sheet = रिक्त;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = रिक्त;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = रिक्त;
	c->stylesheets[STYLESHEET_USER].sheet = रिक्त;
	c->stylesheet_गणनम् = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->आधारः.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->आधारः), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_आधारः].sheet);
	यदि (ns_error != NSERROR_OK) {
		प्रत्यागम ns_error;
	}

	c->आधारः.सक्रिय++;
	NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);


	यदि (nsoption_bool(खण्डम्_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->आधारः), रिक्त,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		यदि (ns_error != NSERROR_OK) {
			प्रत्यागम ns_error;
		}

		c->आधारः.सक्रिय++;
		NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->आधारः), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	यदि (ns_error != NSERROR_OK) {
		प्रत्यागम ns_error;
	}

	c->आधारः.सक्रिय++;
	NSLOG(netsurf, INFO, "%d fetches active", c->आधारः.सक्रिय);

	प्रत्यागम ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	यदि (c->stylesheets[STYLESHEET_आधारः].sheet == रिक्त) {
		प्रत्यागम NSERROR_CSS_आधारः;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	यदि (css_ret != CSS_OK) {
		प्रत्यागम css_error_गन्तव्यम्_nserror(css_ret);
	}

	/* Add sheets to it */
	प्रति (i = STYLESHEET_आधारः; i != c->stylesheet_गणनम्; i++) {
		ध्रुव संरचना html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = रिक्त;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		यदि (hsheet->अप्रयुक्तम्) {
			अनुवर्त;
		}

		यदि (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} अन्यथा यदि (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		यदि (hsheet->sheet != रिक्त) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		यदि (sheet != रिक्त) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			यदि (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				प्रत्यागम css_error_गन्तव्यम्_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	प्रत्यागम NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(शून्य)
{
	nserror error;

	error = html_css_fetcher_पञ्जिकाकोशः();
	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	यदि (error != NSERROR_OK)
		प्रत्यागम error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	प्रत्यागम error;
}


/* exported function documented in html/css.h */
शून्य html_css_fini(शून्य)
{
	यदि (html_user_stylesheet_url != रिक्त) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = रिक्त;
	}

	यदि (html_quirks_stylesheet_url != रिक्त) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = रिक्त;
	}

	यदि (html_adblock_stylesheet_url != रिक्त) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = रिक्त;
	}

	यदि (html_default_stylesheet_url != रिक्त) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = रिक्त;
	}
}
