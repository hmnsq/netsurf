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
स्थिर nserror css_error_to_nserror(css_error error)
{
	विकल्प (error) {
	पक्ष CSS_OK:
		पटिनिवत्त NSERROR_OK;

	पक्ष CSS_NOMEM:
		पटिनिवत्त NSERROR_NOMEM;

	पक्ष CSS_BADPARM:
		पटिनिवत्त NSERROR_BAD_PARAMETER;

	पक्ष CSS_INVALID:
		पटिनिवत्त NSERROR_INVALID;

	पक्ष CSS_FILENOTFOUND:
		पटिनिवत्त NSERROR_NOT_FOUND;

	पक्ष CSS_NEEDDATA:
		पटिनिवत्त NSERROR_NEED_DATA;

	पक्ष CSS_BADCHARSET:
		पटिनिवत्त NSERROR_BAD_ENCODING;

	पक्ष CSS_EOF:
	पक्ष CSS_IMPORTS_PENDING:
	पक्ष CSS_PROPERTY_NOT_SET:
	स्वभाव:
		भङ्ग;
	}
	पटिनिवत्त NSERROR_CSS;
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
	unsigned पूर्णाङ्क i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	अत्थाय (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_गणनम्;
	     i++, s++) {
		सचे (s->sheet == css)
			भङ्ग;
	}

	प्रतिज्ञा(i != parent->stylesheet_गणनम्);

	विकल्प (event->type) {

	पक्ष CONTENT_सन्देशः_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_POINTER:
		/* Really don't want this to continue after the switch */
		पटिनिवत्त NSERROR_OK;

	स्वभाव:
		भङ्ग;
	}

	सचे (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	पटिनिवत्त NSERROR_OK;
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
	child.quirks = c->base.quirks;

	exc = dom_ग्रन्थिः_get_text_content(ग्रन्थिः, &style);
	सचे ((exc != DOM_NO_ERR) || (style == रिक्त)) {
		NSLOG(netsurf, INFO, "No text content");
		पटिनिवत्त NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &कुञ्चिका);
	सचे (error != NSERROR_OK) {
		dom_string_unref(style);
		पटिनिवत्त error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, परिमाण(urlbuf), "x-ns-css:%"PRIu32"", कुञ्चिका);

	error = nsurl_create(urlbuf, &url);
	सचे (error != NSERROR_OK) {
		पटिनिवत्त error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	सचे (error != NSERROR_OK) {
		nsurl_unref(url);
		पटिनिवत्त error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	पटिनिवत्त NSERROR_OK;
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
	सचे (exc == DOM_NO_ERR && val != रिक्त) {
		सचे (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			पटिनिवत्त रिक्त;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	सचे (exc == DOM_NO_ERR && val != रिक्त) {
		सचे (strcasestr(dom_string_data(val), "screen") == रिक्त &&
				strcasestr(dom_string_data(val),
						"all") == रिक्त) {
			dom_string_unref(val);
			पटिनिवत्त रिक्त;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = realloc(c->stylesheets,
			      परिमाण(संरचना html_stylesheet) *
			      (c->stylesheet_गणनम् + 1));
	सचे (stylesheets == रिक्त) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		पटिनिवत्त false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_गणनम्].ग्रन्थिः = dom_ग्रन्थिः_ref(style);
	c->stylesheets[c->stylesheet_गणनम्].sheet = रिक्त;
	c->stylesheets[c->stylesheet_गणनम्].modified = false;
	c->stylesheets[c->stylesheet_गणनम्].unused = false;
	c->stylesheet_गणनम्++;

	पटिनिवत्त c->stylesheets + (c->stylesheet_गणनम् - 1);
}


स्थिर bool
html_css_process_modified_style(html_content *c, संरचना html_stylesheet *s)
{
	hlcache_handle *sheet = रिक्त;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->ग्रन्थिः, &sheet);
	सचे (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, रिक्त);
		पटिनिवत्त false;
	}

	सचे (sheet != रिक्त) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		सचे (s->sheet != रिक्त) {
			विकल्प (content_get_status(s->sheet)) {
			पक्ष CONTENT_STATUS_DONE:
				भङ्ग;
			स्वभाव:
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

	पटिनिवत्त true;
}


/**
 * process a stylesheet that has been modified.
 */
स्थिर शून्य html_css_process_modified_styles(शून्य *pw)
{
	html_content *c = pw;
	संरचना html_stylesheet *s;
	unsigned पूर्णाङ्क i;
	bool all_done = true;

	अत्थाय (i = 0, s = c->stylesheets; i != c->stylesheet_गणनम्; i++, s++) {
		सचे (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	सचे (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_ग्रन्थिः *style)
{
	unsigned पूर्णाङ्क i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	अत्थाय (i = 0, s = c->stylesheets;	i != c->stylesheet_गणनम्; i++, s++) {
		सचे (s->ग्रन्थिः == style)
			भङ्ग;
	}
	सचे (i == c->stylesheet_गणनम्) {
		s = html_create_style_element(c, style);
	}
	सचे (s == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		पटिनिवत्त false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	पटिनिवत्त true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_ग्रन्थिः *ग्रन्थिः)
{
	unsigned पूर्णाङ्क i;
	dom_string *val;
	dom_exception exc;
	संरचना html_stylesheet *s;

	/* Find sheet */
	अत्थाय (i = 0, s = c->stylesheets;	i != c->stylesheet_गणनम्; i++, s++) {
		सचे (s->ग्रन्थिः == ग्रन्थिः)
			भङ्ग;
	}

		/* Should already exist */
	सचे (i == c->stylesheet_गणनम्) {
		पटिनिवत्त false;
	}

	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_media, &val);
	सचे (exc == DOM_NO_ERR && val != रिक्त) {
		सचे (strcasestr(dom_string_data(val), "screen") == रिक्त &&
				strcasestr(dom_string_data(val),
						"all") == रिक्त) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	पटिनिवत्त true;
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
	सचे (exc != DOM_NO_ERR || rel == रिक्त)
		पटिनिवत्त true;

	सचे (strcasestr(dom_string_data(rel), "stylesheet") == रिक्त) {
		dom_string_unref(rel);
		पटिनिवत्त true;
	} अञ्ञथा सचे (strcasestr(dom_string_data(rel), "alternate") != रिक्त) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		पटिनिवत्त true;
	}
	dom_string_unref(rel);

	सचे (nsoption_bool(author_level_css) == false) {
		पटिनिवत्त true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_type, &type_attr);
	सचे (exc == DOM_NO_ERR && type_attr != रिक्त) {
		सचे (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			पटिनिवत्त true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_media, &media);
	सचे (exc == DOM_NO_ERR && media != रिक्त) {
		सचे (strcasestr(dom_string_data(media), "screen") == रिक्त &&
		    strcasestr(dom_string_data(media), "all") == रिक्त) {
			dom_string_unref(media);
			पटिनिवत्त true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(ग्रन्थिः, corestring_dom_href, &href);
	सचे (exc != DOM_NO_ERR || href == रिक्त)
		पटिनिवत्त true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	सचे (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		गच्छ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_गणनम्, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = realloc(htmlc->stylesheets,
			      परिमाण(संरचना html_stylesheet) *
			      (htmlc->stylesheet_गणनम् + 1));
	सचे (stylesheets == रिक्त) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		गच्छ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].ग्रन्थिः = रिक्त;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_गणनम्].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			रिक्त, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_गणनम्].sheet);

	nsurl_unref(joined);

	सचे (ns_error != NSERROR_OK)
		गच्छ no_memory;

	htmlc->stylesheet_गणनम्++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	पटिनिवत्त true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, रिक्त);
	पटिनिवत्त false;
}


/* exported interface documented in html/html.h */
संरचना html_stylesheet *html_get_stylesheets(hlcache_handle *h, unsigned पूर्णाङ्क *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->stylesheet_गणनम्;

	पटिनिवत्त c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	संरचना html_stylesheet *s;
	unsigned पूर्णाङ्क i;

	अत्थाय (i = 0, s = html->stylesheets; i < html->stylesheet_गणनम्;
	     i++, s++) {
		सचे (s->sheet != रिक्त) {
			सचे (content_saw_insecure_objects(s->sheet)) {
				पटिनिवत्त true;
			}
		}
	}

	पटिनिवत्त false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	unsigned पूर्णाङ्क i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	अत्थाय (i = 0; i != html->stylesheet_गणनम्; i++) {
		सचे (html->stylesheets[i].sheet != रिक्त) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		सचे (html->stylesheets[i].ग्रन्थिः != रिक्त) {
			dom_ग्रन्थिः_unref(html->stylesheets[i].ग्रन्थिः);
		}
	}
	मुक्त(html->stylesheets);

	पटिनिवत्त NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	प्रतिज्ञा(c->stylesheets != रिक्त);

	सचे (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), रिक्त,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		सचे (ns_error != NSERROR_OK) {
			पटिनिवत्त ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	पटिनिवत्त ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	सचे (c->stylesheets != रिक्त) {
		पटिनिवत्त NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = calloc(STYLESHEET_START,
			परिमाण(संरचना html_stylesheet));
	सचे (c->stylesheets == रिक्त) {
		पटिनिवत्त NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = रिक्त;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = रिक्त;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = रिक्त;
	c->stylesheets[STYLESHEET_USER].sheet = रिक्त;
	c->stylesheet_गणनम् = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	सचे (ns_error != NSERROR_OK) {
		पटिनिवत्त ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	सचे (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), रिक्त,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		सचे (ns_error != NSERROR_OK) {
			पटिनिवत्त ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	सचे (ns_error != NSERROR_OK) {
		पटिनिवत्त ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	पटिनिवत्त ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	सचे (c->stylesheets[STYLESHEET_BASE].sheet == रिक्त) {
		पटिनिवत्त NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	सचे (css_ret != CSS_OK) {
		पटिनिवत्त css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	अत्थाय (i = STYLESHEET_BASE; i != c->stylesheet_गणनम्; i++) {
		ध्रुव संरचना html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = रिक्त;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		सचे (hsheet->unused) {
			अनुवर्त;
		}

		सचे (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} अञ्ञथा सचे (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		सचे (hsheet->sheet != रिक्त) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		सचे (sheet != रिक्त) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			सचे (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				पटिनिवत्त css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	पटिनिवत्त NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(शून्य)
{
	nserror error;

	error = html_css_fetcher_register();
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	सचे (error != NSERROR_OK)
		पटिनिवत्त error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	पटिनिवत्त error;
}


/* exported function documented in html/css.h */
शून्य html_css_fini(शून्य)
{
	सचे (html_user_stylesheet_url != रिक्त) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = रिक्त;
	}

	सचे (html_quirks_stylesheet_url != रिक्त) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = रिक्त;
	}

	सचे (html_adblock_stylesheet_url != रिक्त) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = रिक्त;
	}

	सचे (html_default_stylesheet_url != रिक्त) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = रिक्त;
	}
}
