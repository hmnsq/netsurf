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

#समावेश "utils/config.h"

#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <stdlib.h>

#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/misc.h"
#समावेश "netsurf/content.h"
#समावेश "content/hlcache.h"
#समावेश "css/css.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/css.h"

अटल nsurl *html_default_stylesheet_url;
अटल nsurl *html_adblock_stylesheet_url;
अटल nsurl *html_quirks_stylesheet_url;
अटल nsurl *html_user_stylesheet_url;

/**
 * Convert css error to netsurf error.
 */
अटल nserror css_error_to_nserror(css_error error)
{
	चयन (error) {
	हाल CSS_OK:
		वापस NSERROR_OK;

	हाल CSS_NOMEM:
		वापस NSERROR_NOMEM;

	हाल CSS_BADPARM:
		वापस NSERROR_BAD_PARAMETER;

	हाल CSS_INVALID:
		वापस NSERROR_INVALID;

	हाल CSS_FILENOTFOUND:
		वापस NSERROR_NOT_FOUND;

	हाल CSS_NEEDDATA:
		वापस NSERROR_NEED_तथ्य;

	हाल CSS_BADCHARSET:
		वापस NSERROR_BAD_ENCODING;

	हाल CSS_EOF:
	हाल CSS_IMPORTS_PENDING:
	हाल CSS_PROPERTY_NOT_SET:
	शेष:
		अवरोध;
	}
	वापस NSERROR_CSS;
}


/**
 * Callback for fetchcache() for stylesheets.
 */
अटल nserror
html_convert_css_callback(hlcache_handle *css,
			  स्थिर hlcache_event *event,
			  व्योम *pw)
{
	html_content *parent = pw;
	अचिन्हित पूर्णांक i;
	काष्ठा html_stylesheet *s;

	/* Find sheet */
	क्रम (i = 0, s = parent->stylesheets;
	     i != parent->stylesheet_गिनती;
	     i++, s++) {
		अगर (s->sheet == css)
			अवरोध;
	}

	निश्चित(i != parent->stylesheet_गिनती);

	चयन (event->type) {

	हाल CONTENT_संदेश_DONE:
		NSLOG(netsurf, INFO, "done stylesheet slot %d '%s'", i,
		      nsurl_access(hlcache_handle_get_url(css)));
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		NSLOG(netsurf, INFO, "stylesheet %s failed: %s",
		      nsurl_access(hlcache_handle_get_url(css)),
		      event->तथ्य.errordata.errormsg);

		hlcache_handle_release(css);
		s->sheet = शून्य;
		parent->base.active--;
		NSLOG(netsurf, INFO, "%d fetches active", parent->base.active);
		अवरोध;

	हाल CONTENT_संदेश_POINTER:
		/* Really don't want this to continue after the switch */
		वापस NSERROR_OK;

	शेष:
		अवरोध;
	}

	अगर (html_can_begin_conversion(parent)) {
		html_begin_conversion(parent);
	}

	वापस NSERROR_OK;
}


अटल nserror
html_stylesheet_from_domnode(html_content *c,
			     dom_गाँठ *गाँठ,
			     hlcache_handle **sheet)
{
	hlcache_child_context child;
	dom_string *style;
	nsurl *url;
	dom_exception exc;
	nserror error;
	uint32_t कुंजी;
	अक्षर urlbuf[64];

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	exc = dom_गाँठ_get_text_content(गाँठ, &style);
	अगर ((exc != DOM_NO_ERR) || (style == शून्य)) {
		NSLOG(netsurf, INFO, "No text content");
		वापस NSERROR_OK;
	}

	error = html_css_fetcher_add_item(style, c->base_url, &कुंजी);
	अगर (error != NSERROR_OK) {
		dom_string_unref(style);
		वापस error;
	}

	dom_string_unref(style);

	snprintf(urlbuf, माप(urlbuf), "x-ns-css:%"PRIu32"", कुंजी);

	error = nsurl_create(urlbuf, &url);
	अगर (error != NSERROR_OK) {
		वापस error;
	}

	error = hlcache_handle_retrieve(url, 0,
			content_get_url(&c->base), शून्य,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			sheet);
	अगर (error != NSERROR_OK) {
		nsurl_unref(url);
		वापस error;
	}

	nsurl_unref(url);

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	वापस NSERROR_OK;
}


/**
 * Process an inline stylesheet in the document.
 *
 * \param  c      content structure
 * \param  style  xml node of style element
 * \return  true on success, false if an error occurred
 */
अटल काष्ठा html_stylesheet *
html_create_style_element(html_content *c, dom_गाँठ *style)
{
	dom_string *val;
	dom_exception exc;
	काष्ठा html_stylesheet *stylesheets;

	/* type='text/css', or not present (invalid but common) */
	exc = dom_element_get_attribute(style, corestring_dom_type, &val);
	अगर (exc == DOM_NO_ERR && val != शून्य) {
		अगर (!dom_string_caseless_lwc_isequal(val,
				corestring_lwc_text_css)) {
			dom_string_unref(val);
			वापस शून्य;
		}
		dom_string_unref(val);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(style, corestring_dom_media, &val);
	अगर (exc == DOM_NO_ERR && val != शून्य) {
		अगर (strcasestr(dom_string_तथ्य(val), "screen") == शून्य &&
				strcasestr(dom_string_तथ्य(val),
						"all") == शून्य) {
			dom_string_unref(val);
			वापस शून्य;
		}
		dom_string_unref(val);
	}

	/* Extend array */
	stylesheets = पुनः_स्मृति(c->stylesheets,
			      माप(काष्ठा html_stylesheet) *
			      (c->stylesheet_गिनती + 1));
	अगर (stylesheets == शून्य) {

		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस false;

	}
	c->stylesheets = stylesheets;

	c->stylesheets[c->stylesheet_गिनती].गाँठ = dom_गाँठ_ref(style);
	c->stylesheets[c->stylesheet_गिनती].sheet = शून्य;
	c->stylesheets[c->stylesheet_गिनती].modified = false;
	c->stylesheets[c->stylesheet_गिनती].unused = false;
	c->stylesheet_गिनती++;

	वापस c->stylesheets + (c->stylesheet_गिनती - 1);
}


अटल bool
html_css_process_modified_style(html_content *c, काष्ठा html_stylesheet *s)
{
	hlcache_handle *sheet = शून्य;
	nserror error;

	error = html_stylesheet_from_domnode(c, s->गाँठ, &sheet);
	अगर (error != NSERROR_OK) {
		NSLOG(netsurf, INFO, "Failed to update sheet");
		content_broadcast_error(&c->base, error, शून्य);
		वापस false;
	}

	अगर (sheet != शून्य) {
		NSLOG(netsurf, INFO, "Updating sheet %p with %p", s->sheet,
		      sheet);

		अगर (s->sheet != शून्य) {
			चयन (content_get_status(s->sheet)) {
			हाल CONTENT_STATUS_DONE:
				अवरोध;
			शेष:
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

	वापस true;
}


/**
 * process a stylesheet that has been modified.
 */
अटल व्योम html_css_process_modified_styles(व्योम *pw)
{
	html_content *c = pw;
	काष्ठा html_stylesheet *s;
	अचिन्हित पूर्णांक i;
	bool all_done = true;

	क्रम (i = 0, s = c->stylesheets; i != c->stylesheet_गिनती; i++, s++) {
		अगर (c->stylesheets[i].modified) {
			all_done &= html_css_process_modified_style(c, s);
		}
	}

	/* If we failed to process any sheet, schedule a retry */
	अगर (all_done == false) {
		guit->misc->schedule(1000, html_css_process_modified_styles, c);
	}
}


/* exported function documented in html/css.h */
bool html_css_update_style(html_content *c, dom_गाँठ *style)
{
	अचिन्हित पूर्णांक i;
	काष्ठा html_stylesheet *s;

	/* Find sheet */
	क्रम (i = 0, s = c->stylesheets;	i != c->stylesheet_गिनती; i++, s++) {
		अगर (s->गाँठ == style)
			अवरोध;
	}
	अगर (i == c->stylesheet_गिनती) {
		s = html_create_style_element(c, style);
	}
	अगर (s == शून्य) {
		NSLOG(netsurf, INFO,
		      "Could not find or create inline stylesheet for %p",
		      style);
		वापस false;
	}

	s->modified = true;

	guit->misc->schedule(0, html_css_process_modified_styles, c);

	वापस true;
}


/* exported function documented in html/css.h */
bool html_css_process_style(html_content *c, dom_गाँठ *गाँठ)
{
	अचिन्हित पूर्णांक i;
	dom_string *val;
	dom_exception exc;
	काष्ठा html_stylesheet *s;

	/* Find sheet */
	क्रम (i = 0, s = c->stylesheets;	i != c->stylesheet_गिनती; i++, s++) {
		अगर (s->गाँठ == गाँठ)
			अवरोध;
	}

		/* Should already exist */
	अगर (i == c->stylesheet_गिनती) {
		वापस false;
	}

	exc = dom_element_get_attribute(गाँठ, corestring_dom_media, &val);
	अगर (exc == DOM_NO_ERR && val != शून्य) {
		अगर (strcasestr(dom_string_तथ्य(val), "screen") == शून्य &&
				strcasestr(dom_string_तथ्य(val),
						"all") == शून्य) {
			s->unused = true;
		}
		dom_string_unref(val);
	}

	वापस true;
}


/* exported function documented in html/css.h */
bool html_css_process_link(html_content *htmlc, dom_गाँठ *गाँठ)
{
	dom_string *rel, *type_attr, *media, *href;
	काष्ठा html_stylesheet *stylesheets;
	nsurl *joined;
	dom_exception exc;
	nserror ns_error;
	hlcache_child_context child;

	/* rel=<space separated list, including 'stylesheet'> */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_rel, &rel);
	अगर (exc != DOM_NO_ERR || rel == शून्य)
		वापस true;

	अगर (strcasestr(dom_string_तथ्य(rel), "stylesheet") == शून्य) {
		dom_string_unref(rel);
		वापस true;
	} अन्यथा अगर (strcasestr(dom_string_तथ्य(rel), "alternate") != शून्य) {
		/* Ignore alternate stylesheets */
		dom_string_unref(rel);
		वापस true;
	}
	dom_string_unref(rel);

	अगर (nsoption_bool(author_level_css) == false) {
		वापस true;
	}

	/* type='text/css' or not present */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_type, &type_attr);
	अगर (exc == DOM_NO_ERR && type_attr != शून्य) {
		अगर (!dom_string_caseless_lwc_isequal(type_attr,
				corestring_lwc_text_css)) {
			dom_string_unref(type_attr);
			वापस true;
		}
		dom_string_unref(type_attr);
	}

	/* media contains 'screen' or 'all' or not present */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_media, &media);
	अगर (exc == DOM_NO_ERR && media != शून्य) {
		अगर (strcasestr(dom_string_तथ्य(media), "screen") == शून्य &&
		    strcasestr(dom_string_तथ्य(media), "all") == शून्य) {
			dom_string_unref(media);
			वापस true;
		}
		dom_string_unref(media);
	}

	/* href='...' */
	exc = dom_element_get_attribute(गाँठ, corestring_dom_href, &href);
	अगर (exc != DOM_NO_ERR || href == शून्य)
		वापस true;

	/* TODO: only the first preferred stylesheets (ie.
	 * those with a title attribute) should be loaded
	 * (see HTML4 14.3) */

	ns_error = nsurl_join(htmlc->base_url, dom_string_तथ्य(href), &joined);
	अगर (ns_error != NSERROR_OK) {
		dom_string_unref(href);
		जाओ no_memory;
	}
	dom_string_unref(href);

	NSLOG(netsurf, INFO, "linked stylesheet %i '%s'",
	      htmlc->stylesheet_गिनती, nsurl_access(joined));

	/* extend stylesheets array to allow for new sheet */
	stylesheets = पुनः_स्मृति(htmlc->stylesheets,
			      माप(काष्ठा html_stylesheet) *
			      (htmlc->stylesheet_गिनती + 1));
	अगर (stylesheets == शून्य) {
		nsurl_unref(joined);
		ns_error = NSERROR_NOMEM;
		जाओ no_memory;
	}

	htmlc->stylesheets = stylesheets;
	htmlc->stylesheets[htmlc->stylesheet_गिनती].गाँठ = शून्य;
	htmlc->stylesheets[htmlc->stylesheet_गिनती].modified = false;
	htmlc->stylesheets[htmlc->stylesheet_गिनती].unused = false;

	/* start fetch */
	child.charset = htmlc->encoding;
	child.quirks = htmlc->base.quirks;

	ns_error = hlcache_handle_retrieve(joined, 0,
			content_get_url(&htmlc->base),
			शून्य, html_convert_css_callback,
			htmlc, &child, CONTENT_CSS,
			&htmlc->stylesheets[htmlc->stylesheet_गिनती].sheet);

	nsurl_unref(joined);

	अगर (ns_error != NSERROR_OK)
		जाओ no_memory;

	htmlc->stylesheet_गिनती++;

	htmlc->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", htmlc->base.active);

	वापस true;

no_memory:
	content_broadcast_error(&htmlc->base, ns_error, शून्य);
	वापस false;
}


/* exported interface documented in html/html.h */
काष्ठा html_stylesheet *html_get_stylesheets(hlcache_handle *h, अचिन्हित पूर्णांक *n)
{
	html_content *c = (html_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);
	निश्चित(n != शून्य);

	*n = c->stylesheet_गिनती;

	वापस c->stylesheets;
}


/* exported function documented in html/css.h */
bool html_css_saw_insecure_stylesheets(html_content *html)
{
	काष्ठा html_stylesheet *s;
	अचिन्हित पूर्णांक i;

	क्रम (i = 0, s = html->stylesheets; i < html->stylesheet_गिनती;
	     i++, s++) {
		अगर (s->sheet != शून्य) {
			अगर (content_saw_insecure_objects(s->sheet)) {
				वापस true;
			}
		}
	}

	वापस false;
}


/* exported function documented in html/css.h */
nserror html_css_free_stylesheets(html_content *html)
{
	अचिन्हित पूर्णांक i;

	guit->misc->schedule(-1, html_css_process_modified_styles, html);

	क्रम (i = 0; i != html->stylesheet_गिनती; i++) {
		अगर (html->stylesheets[i].sheet != शून्य) {
			hlcache_handle_release(html->stylesheets[i].sheet);
		}
		अगर (html->stylesheets[i].गाँठ != शून्य) {
			dom_गाँठ_unref(html->stylesheets[i].गाँठ);
		}
	}
	मुक्त(html->stylesheets);

	वापस NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_quirks_stylesheets(html_content *c)
{
	nserror ns_error = NSERROR_OK;
	hlcache_child_context child;

	निश्चित(c->stylesheets != शून्य);

	अगर (c->quirks == DOM_DOCUMENT_QUIRKS_MODE_FULL) {
		child.charset = c->encoding;
		child.quirks = c->base.quirks;

		ns_error = hlcache_handle_retrieve(html_quirks_stylesheet_url,
				0, content_get_url(&c->base), शून्य,
				html_convert_css_callback, c, &child,
				CONTENT_CSS,
				&c->stylesheets[STYLESHEET_QUIRKS].sheet);
		अगर (ns_error != NSERROR_OK) {
			वापस ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);
	}

	वापस ns_error;
}


/* exported function documented in html/css.h */
nserror html_css_new_stylesheets(html_content *c)
{
	nserror ns_error;
	hlcache_child_context child;

	अगर (c->stylesheets != शून्य) {
		वापस NSERROR_OK; /* already initialised */
	}

	/* stylesheet 0 is the base style sheet,
	 * stylesheet 1 is the quirks mode style sheet,
	 * stylesheet 2 is the adblocking stylesheet,
	 * stylesheet 3 is the user stylesheet */
	c->stylesheets = सुस्मृति(STYLESHEET_START,
			माप(काष्ठा html_stylesheet));
	अगर (c->stylesheets == शून्य) {
		वापस NSERROR_NOMEM;
	}

	c->stylesheets[STYLESHEET_BASE].sheet = शून्य;
	c->stylesheets[STYLESHEET_QUIRKS].sheet = शून्य;
	c->stylesheets[STYLESHEET_ADBLOCK].sheet = शून्य;
	c->stylesheets[STYLESHEET_USER].sheet = शून्य;
	c->stylesheet_गिनती = STYLESHEET_START;

	child.charset = c->encoding;
	child.quirks = c->base.quirks;

	ns_error = hlcache_handle_retrieve(html_default_stylesheet_url, 0,
			content_get_url(&c->base), शून्य,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_BASE].sheet);
	अगर (ns_error != NSERROR_OK) {
		वापस ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);


	अगर (nsoption_bool(block_advertisements)) {
		ns_error = hlcache_handle_retrieve(html_adblock_stylesheet_url,
				0, content_get_url(&c->base), शून्य,
				html_convert_css_callback,
				c, &child, CONTENT_CSS,
				&c->stylesheets[STYLESHEET_ADBLOCK].sheet);
		अगर (ns_error != NSERROR_OK) {
			वापस ns_error;
		}

		c->base.active++;
		NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	}

	ns_error = hlcache_handle_retrieve(html_user_stylesheet_url, 0,
			content_get_url(&c->base), शून्य,
			html_convert_css_callback, c, &child, CONTENT_CSS,
			&c->stylesheets[STYLESHEET_USER].sheet);
	अगर (ns_error != NSERROR_OK) {
		वापस ns_error;
	}

	c->base.active++;
	NSLOG(netsurf, INFO, "%d fetches active", c->base.active);

	वापस ns_error;
}


/* exported function documented in html/css.h */
nserror
html_css_new_selection_context(html_content *c, css_select_ctx **ret_select_ctx)
{
	uint32_t i;
	css_error css_ret;
	css_select_ctx *select_ctx;

	/* check that the base stylesheet loaded; layout fails without it */
	अगर (c->stylesheets[STYLESHEET_BASE].sheet == शून्य) {
		वापस NSERROR_CSS_BASE;
	}

	/* Create selection context */
	css_ret = css_select_ctx_create(&select_ctx);
	अगर (css_ret != CSS_OK) {
		वापस css_error_to_nserror(css_ret);
	}

	/* Add sheets to it */
	क्रम (i = STYLESHEET_BASE; i != c->stylesheet_गिनती; i++) {
		स्थिर काष्ठा html_stylesheet *hsheet = &c->stylesheets[i];
		css_stylesheet *sheet = शून्य;
		css_origin origin = CSS_ORIGIN_AUTHOR;

		/* Filter out stylesheets for non-screen media. */
		/* TODO: We should probably pass the sheet in anyway, and let
		 *       libcss handle the filtering.
		 */
		अगर (hsheet->unused) {
			जारी;
		}

		अगर (i < STYLESHEET_USER) {
			origin = CSS_ORIGIN_UA;
		} अन्यथा अगर (i < STYLESHEET_START) {
			origin = CSS_ORIGIN_USER;
		}

		अगर (hsheet->sheet != शून्य) {
			sheet = nscss_get_stylesheet(hsheet->sheet);
		}

		अगर (sheet != शून्य) {
			/* TODO: Pass the sheet's full media query, instead of
			 *       "screen".
			 */
			css_ret = css_select_ctx_append_sheet(select_ctx,
							      sheet,
							      origin,
							      "screen");
			अगर (css_ret != CSS_OK) {
				css_select_ctx_destroy(select_ctx);
				वापस css_error_to_nserror(css_ret);
			}
		}
	}

	/* return new selection context to caller */
	*ret_select_ctx = select_ctx;
	वापस NSERROR_OK;
}


/* exported function documented in html/css.h */
nserror html_css_init(व्योम)
{
	nserror error;

	error = html_css_fetcher_register();
	अगर (error != NSERROR_OK)
		वापस error;

	error = nsurl_create("resource:default.css",
			&html_default_stylesheet_url);
	अगर (error != NSERROR_OK)
		वापस error;

	error = nsurl_create("resource:adblock.css",
			&html_adblock_stylesheet_url);
	अगर (error != NSERROR_OK)
		वापस error;

	error = nsurl_create("resource:quirks.css",
			&html_quirks_stylesheet_url);
	अगर (error != NSERROR_OK)
		वापस error;

	error = nsurl_create("resource:user.css",
			&html_user_stylesheet_url);

	वापस error;
}


/* exported function documented in html/css.h */
व्योम html_css_fini(व्योम)
{
	अगर (html_user_stylesheet_url != शून्य) {
		nsurl_unref(html_user_stylesheet_url);
		html_user_stylesheet_url = शून्य;
	}

	अगर (html_quirks_stylesheet_url != शून्य) {
		nsurl_unref(html_quirks_stylesheet_url);
		html_quirks_stylesheet_url = शून्य;
	}

	अगर (html_adblock_stylesheet_url != शून्य) {
		nsurl_unref(html_adblock_stylesheet_url);
		html_adblock_stylesheet_url = शून्य;
	}

	अगर (html_default_stylesheet_url != शून्य) {
		nsurl_unref(html_default_stylesheet_url);
		html_default_stylesheet_url = शून्य;
	}
}
