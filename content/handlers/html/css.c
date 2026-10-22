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
	निवड (error) {
	स्थिती CSS_OK:
		परत NSERROR_OK;

	स्थिती CSS_NOMEM:
		परत NSERROR_NOMEM;

	स्थिती CSS_BADPARM:
		परत NSERROR_BAD_PARAMETER;

	स्थिती CSS_INVALID:
		परत NSERROR_INVALID;

	स्थिती CSS_FILENOTFOUND:
		परत NSERROR_NOT_FOUND;

	स्थिती CSS_NEEDDATA:
		परत NSERROR_NEED_DATA;

	स्थिती CSS_BADCHARSET:
		परत NSERROR_BAD_ENCODING;

	स्थिती CSS_EOF:
	स्थिती CSS_IMPORTS_PENDING:
	स्थिती CSS_PROPERTY_NOT_SET:
	पूर्वनिर्धारित:
		थांबा;
	}
	परत NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
स्थिर nserror
html_convert_css_callback(hlcache_handle *css,
			  स्थिरांक hlcache_event *event,
			  शून्य *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	साठी (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_मोजणी;
	     i++, s++) {
		जर (s->sheet == css)
			थांबा;
	}

	प्रतिज्ञा(i != parent->stylesheet_मोजणी);

	निवड (event->type) {

	स्थिती CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		थांबा;

	स्थिती CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->data.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = रिक्त;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		थांबा;

	स्थिती CONTENT_संदेश_POINTER:
		/* Really don't want this to continue after the switch */
		परत NSERROR_OK;

	पूर्वनिर्धारित:
		थांबा;
	}

	जर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	परत NSERROR_OK;
}


स्थिर nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_गाठ *गाठ,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t किल्ली;
	अक्षर urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_गाठ_get_text_content(गाठ, &style);
	जर ((exc != DOM_NO_ERR) || (style == रिक्त)) {
		NSLOG(netsurf, INFO, "No text content");
		परत NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &किल्ली);
	जर (error != NSERROR_OK) {
		dom_string_unref(style);
		परत error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, आकार(urlbuf), "x-ns-css:%"PRIu32"", किल्ली);

	error = nsurl_create(urlbuf, &url);
	जर (error != NSERROR_OK) {
		परत error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	जर (error != NSERROR_OK) {
		nsurl_unref(url);
		परत error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	परत NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
स्थिर संरचना html_stylesheet *
html_create_style_element(html_content *c, dom_गाठ *style)
{
	dom_string *val;
	dom_exception exc;
	संरचना html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	जर (exc == DOM_NO_ERR && val != रिक्त) {
		जर (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			परत रिक्त;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	जर (exc == DOM_NO_ERR && val != रिक्त) {
		जर (strcasestr(dom_string_data(val), "screen") == रिक्त &&
				strcasestr(dom_string_data(val),
						"all") == रिक्त) {
			dom_string_unref(val);
			परत रिक्त;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = पुनः_स्मृति(c->stylesheets,
			      आकार(संरचना html_stylesheet) *
			      (c->stylesheet_मोजणी + 1));
	जर (stylesheets == रिक्त) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		परत false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_मोजणी].गाठ = dom_गाठ_ref(style);
	c->stylesheets[c->stylesheet_मोजणी].sheet = रिक्त;
	c->stylesheets[c->stylesheet_मोजणी].modified = false;
	c->stylesheets[c->stylesheet_मोजणी].unused = false;
	c->stylesheet_मोजणी++;

	परत c->stylesheets + (c->stylesheet_मोजणी - 1);
}


स्थिर bool
html_css_process_modified_style(html_content *c, संरचना html_stylesheet *s)
{
	hlcache_handle *sheet = रिक्त;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->गाठ, &sheet);
	जर (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, रिक्त);
		परत false;
	}

	जर (sheet != रिक्त) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		जर (s->sheet != रिक्त) {
			निवड (content_get_status(s->sheet)) {
			स्थिती CONTENT_STATUS_DONE:
				थांबा;
			पूर्वनिर्धारित:
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

	परत true;
}


/**
 * process a stylesheet that has been modified.
 */
स्थिर शून्य html_css_process_modified_styles(शून्य *pw)
{
	html_content *c = pw;
	संरचना html_stylesheet *s;
	अचिन्हित पूर्णांक i;
	bool all_done = true;

	साठी (i = 0, s = c->stylesheets; i != c->stylesheet_मोजणी; i++, s++) {
		जर (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	जर (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_गाठ *style)
{
	अचिन्हित पूर्णांक i;
	संरचना html_stylesheet *s;

	/* Find sheet */
	साठी (i = 0, s = c->stylesheets;	i != c->stylesheet_मोजणी; i++, s++) {
		जर (s->गाठ == style)
			थांबा;
	}
	जर (i == c->stylesheet_मोजणी) {
		s = html_create_style_element(c, style);
	}
	जर (s == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		परत false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	परत true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_गाठ *गाठ)
{
	अचिन्हित पूर्णांक i;
	dom_string *val;
	dom_exception exc;
	संरचना html_stylesheet *s;

	/* Find sheet */
	साठी (i = 0, s = c->stylesheets;	i != c->stylesheet_मोजणी; i++, s++) {
		जर (s->गाठ == गाठ)
			थांबा;
	}

		/* Should already exist */
	जर (i == c->stylesheet_मोजणी) {
		परत false;
	}

	exc = dom_element_get_attribute(गाठ, corestring_dom_media, &val);
	जर (exc == DOM_NO_ERR && val != रिक्त) {
		जर (strcasestr(dom_string_data(val), "screen") == रिक्त &&
				strcasestr(dom_string_data(val),
						"all") == रिक्त) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	परत true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_गाठ *गाठ)
{
	dom_string *rel, *type_attr, *media, *href;
	संरचना html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(गाठ, corestring_dom_rel, &rel);
	जर (exc != DOM_NO_ERR || rel == रिक्त)
		परत true;

	जर (strcasestr(dom_string_data(rel), "stylesheet") == रिक्त) {
		dom_string_unref(rel);
		परत true;
	} अन्यथा जर (strcasestr(dom_string_data(rel), "alternate") != रिक्त) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		परत true;
	}
	dom_string_unref(rel);

	जर (nsoption_bool(author_level_css) == false) {
		परत true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_type, &type_attr);
	जर (exc == DOM_NO_ERR && type_attr != रिक्त) {
		जर (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			परत true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(गाठ, corestring_dom_media, &media);
	जर (exc == DOM_NO_ERR && media != रिक्त) {
		जर (strcasestr(dom_string_data(media), "screen") == रिक्त &&
		    strcasestr(dom_string_data(media), "all") == रिक्त) {
			dom_string_unref(media);
			परत true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(गाठ, corestring_dom_href, &href);
	जर (exc != DOM_NO_ERR || href == रिक्त)
		परत true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_data(href), &joined);
	जर (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		जा no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_मोजणी, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = पुनः_स्मृति(htmlc->stylesheets,
			      आकार(संरचना html_stylesheet) *
			      (htmlc->stylesheet_मोजणी + 1));
	जर (stylesheets == रिक्त) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		जा no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_मोजणी].गाठ = रिक्त;
	htmlc->stylesheets[htmlc->stylesheet_मोजणी].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_मोजणी].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			रिक्त, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_मोजणी].sheet);

	nsurl_unref(joined);

	जर (ns_error != NSERROR_OK)
		जा no_memory;

	htmlc->stylesheet_मोजणी++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	परत true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, रिक्त);
	परत false;
}


/* exported interface documented in html/html.h */
संरचना html_stylesheet *html_get_stylesheets(hlcache_handle *h, अचिन्हित पूर्णांक *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->stylesheet_मोजणी;

	परत c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	संरचना html_stylesheet *s;
	अचिन्हित पूर्णांक i;

	साठी (i = 0, s = html->stylesheets; i < html->stylesheet_मोजणी;
	     i++, s++) {
		जर (s->sheet != रिक्त) {
			जर (content_saw_insecure_objects(s->sheet)) {
				परत true;
			}
		}
	}

	परत false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	अचिन्हित पूर्णांक i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	साठी (i = 0; i != html->stylesheet_मोजणी; i++) {
		जर (html->stylesheets[i].sheet != रिक्त) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		जर (html->stylesheets[i].गाठ != रिक्त) {
			dom_गाठ_unref(html->stylesheets[i].गाठ);
		}
	}
	मुक्त(html->stylesheets);

	परत NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	प्रतिज्ञा(c->stylesheets != रिक्त);

	जर (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), रिक्त,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		जर (ns_error != NSERROR_OK) {
			परत ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	परत ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	जर (c->stylesheets != रिक्त) {
		परत NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = सुस्मृति(STYLESHEET_START,
			आकार(संरचना html_stylesheet));
	जर (c->stylesheets == रिक्त) {
		परत NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = रिक्त;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = रिक्त;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = रिक्त;
	c->stylesheets[STYLESHEET_USER].sheet = रिक्त;
	c->stylesheet_मोजणी = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	जर (ns_error != NSERROR_OK) {
		परत ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	जर (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), रिक्त,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		जर (ns_error != NSERROR_OK) {
			परत ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), रिक्त,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	जर (ns_error != NSERROR_OK) {
		परत ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	परत ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	जर (c->stylesheets[STYLESHEET_BASE].sheet == रिक्त) {
		परत NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	जर (css_ret != CSS_OK) {
		परत css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	साठी (i = STYLESHEET_BASE; i != c->stylesheet_मोजणी; i++) {
		स्थिरांक संरचना html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = रिक्त;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		जर (hsheet->unused) {
			सुरूठेवा;
		}

		जर (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} अन्यथा जर (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		जर (hsheet->sheet != रिक्त) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		जर (sheet != रिक्त) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			जर (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				परत css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	परत NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(शून्य)
{
	nserror error;

	error = html_css_fetcher_register();
	जर (error != NSERROR_OK)
		परत error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	जर (error != NSERROR_OK)
		परत error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	जर (error != NSERROR_OK)
		परत error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	जर (error != NSERROR_OK)
		परत error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	परत error;
}


/* exported function documented in html/css.h */
शून्य html_css_fini(शून्य)
{
	जर (html_user_stylesheet_url != रिक्त) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = रिक्त;
	}

	जर (html_quirks_stylesheet_url != रिक्त) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = रिक्त;
	}

	जर (html_adblock_stylesheet_url != रिक्त) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = रिक्त;
	}

	जर (html_default_stylesheet_url != रिक्त) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = रिक्त;
	}
}
