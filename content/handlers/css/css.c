/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#समावेश <string.h>
#समावेश <assert.h>
#समावेश <libwapcaplet/libwapcaplet.h>
#समावेश <dom/dom.h>

#समावेश "utils/errors.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/utils.h"
#समावेश "utils/http.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "content/fetch.h"
#समावेश "content/hlcache.h"
#समावेश "desktop/system_colour.h"

#समावेश "css/css.h"
#समावेश "css/hints.h"
#समावेश "css/internal.h"

/* Define to trace import fetches */
#अघोषित NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

काष्ठा content_css_तथ्य;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
प्रकार व्योम (*nscss_done_callback)(काष्ठा content_css_तथ्य *css, व्योम *pw);

/**
 * CSS content data
 */
काष्ठा content_css_तथ्य
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	अक्षर *charset;			/**< Character set of stylesheet */
	काष्ठा nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_गिनती;		/**< Number of sheets imported */
	uint32_t अग्र_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	व्योम *pw;			/**< Client data */
};

/**
 * CSS content data
 */
प्रकार काष्ठा nscss_content
{
	काष्ठा content base;		/**< Underlying content object */

	काष्ठा content_css_तथ्य तथ्य;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
प्रकार काष्ठा {
	काष्ठा content_css_तथ्य *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

अटल bool nscss_convert(काष्ठा content *c);
अटल व्योम nscss_destroy(काष्ठा content *c);
अटल nserror nscss_clone(स्थिर काष्ठा content *old, काष्ठा content **newc);
अटल bool nscss_matches_quirks(स्थिर काष्ठा content *c, bool quirks);
अटल content_type nscss_content_type(व्योम);

अटल nserror nscss_create_css_तथ्य(काष्ठा content_css_तथ्य *c,
		स्थिर अक्षर *url, स्थिर अक्षर *charset, bool quirks,
		nscss_done_callback done, व्योम *pw);
अटल css_error nscss_process_css_तथ्य(काष्ठा content_css_तथ्य *c, स्थिर अक्षर *तथ्य,
		अचिन्हित पूर्णांक आकार);
अटल css_error nscss_convert_css_तथ्य(काष्ठा content_css_तथ्य *c);
अटल व्योम nscss_destroy_css_तथ्य(काष्ठा content_css_तथ्य *c);

अटल व्योम nscss_content_done(काष्ठा content_css_तथ्य *css, व्योम *pw);
अटल css_error nscss_handle_import(व्योम *pw, css_stylesheet *parent,
		lwc_string *url);
अटल nserror nscss_import(hlcache_handle *handle,
		स्थिर hlcache_event *event, व्योम *pw);
अटल css_error nscss_import_complete(nscss_import_ctx *ctx);

अटल css_error nscss_register_imports(काष्ठा content_css_तथ्य *c);
अटल css_error nscss_register_import(काष्ठा content_css_तथ्य *c,
		स्थिर hlcache_handle *import);


अटल css_stylesheet *blank_import;


/**
 * Initialise a CSS content
 *
 * \param handler content handler
 * \param imime_type mime-type
 * \param params Content-Type parameters
 * \param llcache handle to content
 * \param fallback_charset The character set to fallback to.
 * \param quirks allow quirks
 * \param c Content to initialise
 * \return NSERROR_OK or error cod eon faliure
 */
अटल nserror
nscss_create(स्थिर content_handler *handler,
	     lwc_string *imime_type,
	     स्थिर http_parameter *params,
	     llcache_handle *llcache,
	     स्थिर अक्षर *fallback_charset,
	     bool quirks,
	     काष्ठा content **c)
{
	nscss_content *परिणाम;
	स्थिर अक्षर *charset = शून्य;
	स्थिर अक्षर *xnsbase = शून्य;
	lwc_string *charset_मूल्य = शून्य;
	nserror error;

	परिणाम = सुस्मृति(1, माप(nscss_content));
	अगर (परिणाम == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&परिणाम->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(परिणाम);
		वापस error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_मूल्य);
	अगर (error != NSERROR_OK || lwc_string_लंबाई(charset_मूल्य) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} अन्यथा {
		charset = lwc_string_तथ्य(charset_मूल्य);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	अगर (xnsbase == शून्य) {
		xnsbase = nsurl_access(content_get_url(&परिणाम->base));
	}

	error = nscss_create_css_तथ्य(&परिणाम->तथ्य,
			xnsbase, charset, परिणाम->base.quirks,
			nscss_content_done, परिणाम);
	अगर (error != NSERROR_OK) {
		content_broadcast_error(&परिणाम->base, NSERROR_NOMEM, शून्य);
		lwc_string_unref(charset_मूल्य);
		मुक्त(परिणाम);
		वापस error;
	}

	lwc_string_unref(charset_मूल्य);

	*c = (काष्ठा content *) परिणाम;

	वापस NSERROR_OK;
}

/**
 * Create a struct content_css_data, creating a stylesheet object
 *
 * \param c        Struct to populate
 * \param url      URL of stylesheet
 * \param charset  Stylesheet charset
 * \param quirks   Stylesheet quirks mode
 * \param done     Callback to call when content has completed
 * \param pw       Client data for \a done
 * \return NSERROR_OK on success, NSERROR_NOMEM on memory exhaustion
 */
अटल nserror nscss_create_css_तथ्य(काष्ठा content_css_तथ्य *c,
		स्थिर अक्षर *url, स्थिर अक्षर *charset, bool quirks,
		nscss_done_callback done, व्योम *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->अग्र_to_register = (uint32_t) -1;
	c->import_गिनती = 0;
	c->imports = शून्य;
	अगर (charset != शून्य)
		c->charset = strdup(charset);
	अन्यथा
		c->charset = शून्य;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = शून्य;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = शून्य;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = शून्य;
	params.font = शून्य;
	params.font_pw = शून्य;

	error = css_stylesheet_create(&params, &c->sheet);
	अगर (error != CSS_OK) {
		वापस NSERROR_NOMEM;
	}

	वापस NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
अटल bool
nscss_process_तथ्य(काष्ठा content *c, स्थिर अक्षर *तथ्य, अचिन्हित पूर्णांक आकार)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_तथ्य(&css->तथ्य, तथ्य, आकार);
	अगर (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, शून्य);
	}

	वापस (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
अटल css_error nscss_process_css_तथ्य(काष्ठा content_css_तथ्य *c,
		स्थिर अक्षर *तथ्य, अचिन्हित पूर्णांक आकार)
{
	वापस css_stylesheet_append_तथ्य(c->sheet,
			(स्थिर uint8_t *) तथ्य, आकार);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(काष्ठा content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_तथ्य(&css->तथ्य);
	अगर (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, शून्य);
		वापस false;
	}

	वापस true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
अटल css_error nscss_convert_css_तथ्य(काष्ठा content_css_तथ्य *c)
{
	css_error error;

	error = css_stylesheet_तथ्य_done(c->sheet);

	/* Process pending imports */
	अगर (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		निश्चित(c->अग्र_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->अग्र_to_register = 0;
		error = nscss_register_imports(c);
	} अन्यथा अगर (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} अन्यथा {
		स्थिर अक्षर *url;

		अगर (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} अन्यथा {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	वापस error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
व्योम nscss_destroy(काष्ठा content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_तथ्य(&css->तथ्य);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
अटल व्योम nscss_destroy_css_तथ्य(काष्ठा content_css_तथ्य *c)
{
	uint32_t i;

	क्रम (i = 0; i < c->import_गिनती; i++) {
		अगर (c->imports[i].c != शून्य) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = शून्य;
	}

	मुक्त(c->imports);

	अगर (c->sheet != शून्य) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = शून्य;
	}

	मुक्त(c->charset);
}

nserror nscss_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	स्थिर nscss_content *old_css = (स्थिर nscss_content *) old;
	nscss_content *new_css;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;
	nserror error;

	new_css = सुस्मृति(1, माप(nscss_content));
	अगर (new_css == शून्य)
		वापस NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		वापस error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_तथ्य(&new_css->तथ्य,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->तथ्य.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	अगर (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		वापस error;
	}

	तथ्य = content_get_source_तथ्य_2(&new_css->base, &आकार);
	अगर (आकार > 0) {
		अगर (nscss_process_तथ्य(&new_css->base,
				       (अक्षर *)तथ्य,
				       (अचिन्हित पूर्णांक)आकार) == false) {
			content_destroy(&new_css->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	अगर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		अगर (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) new_css;

	वापस NSERROR_OK;
}

bool nscss_matches_quirks(स्थिर काष्ठा content *c, bool quirks)
{
	वापस c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(काष्ठा hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);

	वापस c->तथ्य.sheet;
}

/* exported interface documented in netsurf/css.h */
काष्ठा nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	निश्चित(c != शून्य);
	निश्चित(n != शून्य);

	*n = c->तथ्य.import_गिनती;

	वापस c->तथ्य.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(व्योम)
{
	वापस CONTENT_CSS;
}

/*****************************************************************************
 * Object completion                                                         *
 *****************************************************************************/

/**
 * Handle notification that a CSS object is done
 *
 * \param css  CSS object
 * \param pw   Private data
 */
व्योम nscss_content_done(काष्ठा content_css_तथ्य *css, व्योम *pw)
{
	काष्ठा content *c = pw;
	uint32_t i;
	माप_प्रकार आकार;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_आकार(css->sheet, &आकार);
	अगर (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, शून्य);
		content_set_error(c);
		वापस;
	}
	c->आकार += आकार;

	/* Add on the size of the imported sheets */
	क्रम (i = 0; i < css->import_गिनती; i++) {
		अगर (css->imports[i].c != शून्य) {
			काष्ठा content *import = hlcache_handle_get_content(
					css->imports[i].c);

			अगर (import != शून्य) {
				c->आकार += import->आकार;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_set_ready(c);
	content_set_done(c);
}

/*****************************************************************************
 * Import handling                                                           *
 *****************************************************************************/

/**
 * Handle notification of the need for an imported stylesheet
 *
 * \param pw      CSS object requesting the import
 * \param parent  Stylesheet requesting the import
 * \param url     URL of the imported sheet
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_handle_import(व्योम *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	काष्ठा content_css_तथ्य *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	काष्ठा nscss_import *imports;
	स्थिर अक्षर *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	निश्चित(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	अगर (error != CSS_OK) {
		वापस error;
	}

	ctx = दो_स्मृति(माप(*ctx));
	अगर (ctx == शून्य)
		वापस CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_गिनती;

	/* Increase space in table */
	imports = पुनः_स्मृति(c->imports, (c->import_गिनती + 1) *
			माप(काष्ठा nscss_import));
	अगर (imports == शून्य) {
		मुक्त(ctx);
		वापस CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = शून्य;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	अगर (error != CSS_OK) {
		मुक्त(ctx);
		वापस error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_तथ्य(url), &ns_url);
	अगर (nerror != NSERROR_OK) {
		मुक्त(ctx);
		वापस CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	अगर (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		मुक्त(ctx);
		वापस CSS_NOMEM;
	}

	/* Avoid importing ourself */
	अगर (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_गिनती].c = शून्य;
		/* No longer require context as we're not fetching anything */
		मुक्त(ctx);
		ctx = शून्य;
	} अन्यथा {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, शून्य, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_गिनती].c);
		अगर (nerror != NSERROR_OK) {
			मुक्त(ctx);
			वापस CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#अगर_घोषित NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_गिनती, lwc_string_तथ्य(url),
	      c->imports[c->import_गिनती].c, ctx);
#पूर्ण_अगर

	c->import_गिनती++;

	वापस CSS_OK;
}

/**
 * Handler for imported stylesheet events
 *
 * \param handle  Handle for stylesheet
 * \param event   Event object
 * \param pw      Callback context
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror nscss_import(hlcache_handle *handle,
		स्थिर hlcache_event *event, व्योम *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#अगर_घोषित NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#पूर्ण_अगर

	निश्चित(ctx->css->imports[ctx->index].c == handle);

	चयन (event->type) {
	हाल CONTENT_संदेश_DONE:
		error = nscss_import_complete(ctx);
		अवरोध;

	हाल CONTENT_संदेश_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = शून्य;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		अवरोध;
	शेष:
		अवरोध;
	}

	/* Preserve out-of-memory. Anything else is OK */
	वापस error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
}

/**
 * Handle an imported stylesheet completing
 *
 * \param ctx  Import context
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_import_complete(nscss_import_ctx *ctx)
{
	css_error error = CSS_OK;

	/* If this import is the next to be registered, do so */
	अगर (ctx->css->अग्र_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#अगर_घोषित NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#पूर्ण_अगर

	/* No longer need import context */
	मुक्त(ctx);

	वापस error;
}

/*****************************************************************************
 * Import registration                                                       *
 *****************************************************************************/

/**
 * Register imports with a stylesheet
 *
 * \param c  CSS object containing the imports
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_imports(काष्ठा content_css_तथ्य *c)
{
	uint32_t index;
	css_error error;

	निश्चित(c->अग्र_to_register != (uint32_t) -1);
	निश्चित(c->अग्र_to_register < c->import_गिनती);

	/* Register imported sheets */
	क्रम (index = c->अग्र_to_register; index < c->import_गिनती; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		अगर (c->imports[index].c != शून्य &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			अवरोध;
		}

		error = nscss_register_import(c, c->imports[index].c);
		अगर (error != CSS_OK)
			वापस error;
	}

	/* Record identity of the next import to register */
	c->अग्र_to_register = (uint32_t) index;

	अगर (c->अग्र_to_register == c->import_गिनती) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	वापस CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(काष्ठा content_css_तथ्य *c,
		स्थिर hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	अगर (import != शून्य) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->तथ्य.sheet;
	} अन्यथा {
		/* Create a blank sheet if needed. */
		अगर (blank_import == शून्य) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = शून्य;
			params.url = "";
			params.title = शून्य;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = शून्य;
			params.import = शून्य;
			params.import_pw = शून्य;
			params.color = ns_system_colour;
			params.color_pw = शून्य;
			params.font = शून्य;
			params.font_pw = शून्य;

			error = css_stylesheet_create(&params, &blank_import);
			अगर (error != CSS_OK) {
				वापस error;
			}

			error = css_stylesheet_तथ्य_done(blank_import);
			अगर (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				वापस error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	अगर (error != CSS_OK) {
		वापस error;
	}

	वापस error;
}

/**
 * Clean up after the CSS content handler
 */
अटल व्योम nscss_fini(व्योम)
{
	अगर (blank_import != शून्य) {
		css_stylesheet_destroy(blank_import);
		blank_import = शून्य;
	}
	css_hint_fini();
}

अटल स्थिर content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_तथ्य = nscss_process_तथ्य,
	.तथ्य_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(व्योम)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	अगर (error != NSERROR_OK)
		जाओ error;

	error = css_hint_init();
	अगर (error != NSERROR_OK)
		जाओ error;

	वापस NSERROR_OK;

error:
	nscss_fini();

	वापस error;
}
