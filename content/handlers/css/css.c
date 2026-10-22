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

#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <libwapcaplet/libwapcaplet.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/http.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/fetch.h"
#अन्तर्भाव "content/hlcache.h"
#अन्तर्भाव "desktop/system_colour.h"

#अन्तर्भाव "css/css.h"
#अन्तर्भाव "css/hints.h"
#अन्तर्भाव "css/internal.h"

/* Define to trace import fetches */
#अपरिभाषय NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

संरचना content_css_दत्त;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
प्रकारनिर्देश शून्य (*nscss_समाप्तम्_callback)(संरचना content_css_दत्त *css, शून्य *pw);

/**
 * CSS content data
 */
संरचना content_css_दत्त
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	वर्ण *charset;			/**< Character set of stylesheet */
	संरचना nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_गणनम्;		/**< Number of sheets imported */
	uint32_t अग्र_गन्तव्यम्_पञ्जिकाकोशः;	/**< Index of next import to register */
	nscss_समाप्तम्_callback समाप्तम्;	/**< Completion callback */
	शून्य *pw;			/**< Client data */
};

/**
 * CSS content data
 */
प्रकारनिर्देश संरचना nscss_content
{
	संरचना content आधारः;		/**< Underlying content object */

	संरचना content_css_दत्त दत्त;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
प्रकारनिर्देश संरचना {
	संरचना content_css_दत्त *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

स्थिर bool nscss_convert(संरचना content *c);
स्थिर शून्य nscss_destroy(संरचना content *c);
स्थिर nserror nscss_clone(ध्रुव संरचना content *old, संरचना content **newc);
स्थिर bool nscss_matches_quirks(ध्रुव संरचना content *c, bool quirks);
स्थिर content_type nscss_content_type(शून्य);

स्थिर nserror nscss_create_css_दत्त(संरचना content_css_दत्त *c,
		ध्रुव वर्ण *url, ध्रुव वर्ण *charset, bool quirks,
		nscss_समाप्तम्_callback समाप्तम्, शून्य *pw);
स्थिर css_error nscss_process_css_दत्त(संरचना content_css_दत्त *c, ध्रुव वर्ण *दत्त,
		अचिह्नित पूर्णाङ्क आकार);
स्थिर css_error nscss_convert_css_दत्त(संरचना content_css_दत्त *c);
स्थिर शून्य nscss_destroy_css_दत्त(संरचना content_css_दत्त *c);

स्थिर शून्य nscss_content_समाप्तम्(संरचना content_css_दत्त *css, शून्य *pw);
स्थिर css_error nscss_handle_import(शून्य *pw, css_stylesheet *parent,
		lwc_string *url);
स्थिर nserror nscss_import(hlcache_handle *handle,
		ध्रुव hlcache_event *event, शून्य *pw);
स्थिर css_error nscss_import_complete(nscss_import_ctx *ctx);

स्थिर css_error nscss_पञ्जिकाकोशः_imports(संरचना content_css_दत्त *c);
स्थिर css_error nscss_पञ्जिकाकोशः_import(संरचना content_css_दत्त *c,
		ध्रुव hlcache_handle *import);


स्थिर css_stylesheet *blank_import;


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
स्थिर nserror
nscss_create(ध्रुव content_handler *handler,
	     lwc_string *imime_type,
	     ध्रुव http_parameter *params,
	     llcache_handle *llcache,
	     ध्रुव वर्ण *fallback_charset,
	     bool quirks,
	     संरचना content **c)
{
	nscss_content *परिणाम;
	ध्रुव वर्ण *charset = रिक्त;
	ध्रुव वर्ण *xnsbase = रिक्त;
	lwc_string *charset_मानम् = रिक्त;
	nserror error;

	परिणाम = शून्यस्मृतिं_देहि(1, परिमाण(nscss_content));
	यदि (परिणाम == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&परिणाम->आधारः, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(परिणाम);
		प्रत्यागम error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_मानम्);
	यदि (error != NSERROR_OK || lwc_string_दीर्घता(charset_मानम्) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} अन्यथा {
		charset = lwc_string_दत्त(charset_मानम्);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_शीर्षकम्(llcache, "X-NS-Base");
	यदि (xnsbase == रिक्त) {
		xnsbase = nsurl_access(content_get_url(&परिणाम->आधारः));
	}

	error = nscss_create_css_दत्त(&परिणाम->दत्त,
			xnsbase, charset, परिणाम->आधारः.quirks,
			nscss_content_समाप्तम्, परिणाम);
	यदि (error != NSERROR_OK) {
		content_प्रसारणम्_error(&परिणाम->आधारः, NSERROR_NOMEM, रिक्त);
		lwc_string_unref(charset_मानम्);
		मुक्त(परिणाम);
		प्रत्यागम error;
	}

	lwc_string_unref(charset_मानम्);

	*c = (संरचना content *) परिणाम;

	प्रत्यागम NSERROR_OK;
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
स्थिर nserror nscss_create_css_दत्त(संरचना content_css_दत्त *c,
		ध्रुव वर्ण *url, ध्रुव वर्ण *charset, bool quirks,
		nscss_समाप्तम्_callback समाप्तम्, शून्य *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->समाप्तम् = समाप्तम्;
	c->अग्र_गन्तव्यम्_पञ्जिकाकोशः = (uint32_t) -1;
	c->import_गणनम् = 0;
	c->imports = रिक्त;
	यदि (charset != रिक्त)
		c->charset = strdup(charset);
	अन्यथा
		c->charset = रिक्त;

	params.params_संस्करणम् = CSS_STYLESHEET_PARAMS_संस्करणम्_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = रिक्त;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = रिक्त;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_रङ्गः;
	params.color_pw = रिक्त;
	params.font = रिक्त;
	params.font_pw = रिक्त;

	error = css_stylesheet_create(&params, &c->sheet);
	यदि (error != CSS_OK) {
		प्रत्यागम NSERROR_NOMEM;
	}

	प्रत्यागम NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
स्थिर bool
nscss_process_दत्त(संरचना content *c, ध्रुव वर्ण *दत्त, अचिह्नित पूर्णाङ्क आकार)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_दत्त(&css->दत्त, दत्त, आकार);
	यदि (error != CSS_OK && error != CSS_NEEDDATA) {
		content_प्रसारणम्_error(c, NSERROR_CSS, रिक्त);
	}

	प्रत्यागम (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
स्थिर css_error nscss_process_css_दत्त(संरचना content_css_दत्त *c,
		ध्रुव वर्ण *दत्त, अचिह्नित पूर्णाङ्क आकार)
{
	प्रत्यागम css_stylesheet_append_दत्त(c->sheet,
			(ध्रुव uint8_t *) दत्त, आकार);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(संरचना content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_दत्त(&css->दत्त);
	यदि (error != CSS_OK) {
		content_प्रसारणम्_error(c, NSERROR_CSS, रिक्त);
		प्रत्यागम false;
	}

	प्रत्यागम true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
स्थिर css_error nscss_convert_css_दत्त(संरचना content_css_दत्त *c)
{
	css_error error;

	error = css_stylesheet_दत्त_समाप्तम्(c->sheet);

	/* Process pending imports */
	यदि (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		प्रतिज्ञा(c->अग्र_गन्तव्यम्_पञ्जिकाकोशः == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->अग्र_गन्तव्यम्_पञ्जिकाकोशः = 0;
		error = nscss_पञ्जिकाकोशः_imports(c);
	} अन्यथा यदि (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->समाप्तम्(c, c->pw);
	} अन्यथा {
		ध्रुव वर्ण *url;

		यदि (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} अन्यथा {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	प्रत्यागम error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
शून्य nscss_destroy(संरचना content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_दत्त(&css->दत्त);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
स्थिर शून्य nscss_destroy_css_दत्त(संरचना content_css_दत्त *c)
{
	uint32_t i;

	प्रति (i = 0; i < c->import_गणनम्; i++) {
		यदि (c->imports[i].c != रिक्त) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = रिक्त;
	}

	मुक्त(c->imports);

	यदि (c->sheet != रिक्त) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = रिक्त;
	}

	मुक्त(c->charset);
}

nserror nscss_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	ध्रुव nscss_content *old_css = (ध्रुव nscss_content *) old;
	nscss_content *new_css;
	ध्रुव uint8_t *दत्त;
	size_t आकार;
	nserror error;

	new_css = शून्यस्मृतिं_देहि(1, परिमाण(nscss_content));
	यदि (new_css == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&new_css->आधारः);
		प्रत्यागम error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_दत्त(&new_css->दत्त,
			nsurl_access(content_get_url(&new_css->आधारः)),
			old_css->दत्त.charset,
			new_css->आधारः.quirks,
			nscss_content_समाप्तम्, new_css);
	यदि (error != NSERROR_OK) {
		content_destroy(&new_css->आधारः);
		प्रत्यागम error;
	}

	दत्त = content_get_source_दत्त_2(&new_css->आधारः, &आकार);
	यदि (आकार > 0) {
		यदि (nscss_process_दत्त(&new_css->आधारः,
				       (वर्ण *)दत्त,
				       (अचिह्नित पूर्णाङ्क)आकार) == false) {
			content_destroy(&new_css->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	यदि (old->स्थितिः == CONTENT_स्थितिः_READY ||
			old->स्थितिः == CONTENT_स्थितिः_समाप्तम्) {
		यदि (nscss_convert(&new_css->आधारः) == false) {
			content_destroy(&new_css->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) new_css;

	प्रत्यागम NSERROR_OK;
}

bool nscss_matches_quirks(ध्रुव संरचना content *c, bool quirks)
{
	प्रत्यागम c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(संरचना hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);

	प्रत्यागम c->दत्त.sheet;
}

/* exported interface documented in netsurf/css.h */
संरचना nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	प्रतिज्ञा(c != रिक्त);
	प्रतिज्ञा(n != रिक्त);

	*n = c->दत्त.import_गणनम्;

	प्रत्यागम c->दत्त.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(शून्य)
{
	प्रत्यागम CONTENT_CSS;
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
शून्य nscss_content_समाप्तम्(संरचना content_css_दत्त *css, शून्य *pw)
{
	संरचना content *c = pw;
	uint32_t i;
	size_t आकार;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_आकार(css->sheet, &आकार);
	यदि (error != CSS_OK) {
		content_प्रसारणम्_error(c, NSERROR_CSS, रिक्त);
		content_समुच्चयः_error(c);
		प्रत्यागम;
	}
	c->आकार += आकार;

	/* Add on the size of the imported sheets */
	प्रति (i = 0; i < css->import_गणनम्; i++) {
		यदि (css->imports[i].c != रिक्त) {
			संरचना content *import = hlcache_handle_get_content(
					css->imports[i].c);

			यदि (import != रिक्त) {
				c->आकार += import->आकार;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);
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
css_error nscss_handle_import(शून्य *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	संरचना content_css_दत्त *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	संरचना nscss_import *imports;
	ध्रुव वर्ण *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	प्रतिज्ञा(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	यदि (error != CSS_OK) {
		प्रत्यागम error;
	}

	ctx = स्मृतिं_देहि(परिमाण(*ctx));
	यदि (ctx == रिक्त)
		प्रत्यागम CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_गणनम्;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_गणनम् + 1) *
			परिमाण(संरचना nscss_import));
	यदि (imports == रिक्त) {
		मुक्त(ctx);
		प्रत्यागम CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = रिक्त;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	यदि (error != CSS_OK) {
		मुक्त(ctx);
		प्रत्यागम error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_दत्त(url), &ns_url);
	यदि (nerror != NSERROR_OK) {
		मुक्त(ctx);
		प्रत्यागम CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	यदि (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		मुक्त(ctx);
		प्रत्यागम CSS_NOMEM;
	}

	/* Avoid importing ourself */
	यदि (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_गणनम्].c = रिक्त;
		/* No longer require context as we're not fetching anything */
		मुक्त(ctx);
		ctx = रिक्त;
	} अन्यथा {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, रिक्त, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_गणनम्].c);
		यदि (nerror != NSERROR_OK) {
			मुक्त(ctx);
			प्रत्यागम CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#यदि_परिभाषितम् NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_गणनम्, lwc_string_दत्त(url),
	      c->imports[c->import_गणनम्].c, ctx);
#यदि_अन्तः

	c->import_गणनम्++;

	प्रत्यागम CSS_OK;
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
		ध्रुव hlcache_event *event, शून्य *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#यदि_परिभाषितम् NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#यदि_अन्तः

	प्रतिज्ञा(ctx->css->imports[ctx->index].c == handle);

	विकल्प (event->type) {
	पक्ष CONTENT_सन्देशः_समाप्तम्:
		error = nscss_import_complete(ctx);
		भङ्ग;

	पक्ष CONTENT_सन्देशः_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = रिक्त;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		भङ्ग;
	स्वभाव:
		भङ्ग;
	}

	/* Preserve out-of-memory. Anything else is OK */
	प्रत्यागम error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	यदि (ctx->css->अग्र_गन्तव्यम्_पञ्जिकाकोशः == ctx->index)
		error = nscss_पञ्जिकाकोशः_imports(ctx->css);

#यदि_परिभाषितम् NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#यदि_अन्तः

	/* No longer need import context */
	मुक्त(ctx);

	प्रत्यागम error;
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
css_error nscss_पञ्जिकाकोशः_imports(संरचना content_css_दत्त *c)
{
	uint32_t index;
	css_error error;

	प्रतिज्ञा(c->अग्र_गन्तव्यम्_पञ्जिकाकोशः != (uint32_t) -1);
	प्रतिज्ञा(c->अग्र_गन्तव्यम्_पञ्जिकाकोशः < c->import_गणनम्);

	/* Register imported sheets */
	प्रति (index = c->अग्र_गन्तव्यम्_पञ्जिकाकोशः; index < c->import_गणनम्; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		यदि (c->imports[index].c != रिक्त &&
			content_get_स्थितिः(c->imports[index].c) !=
				CONTENT_स्थितिः_समाप्तम्) {
			भङ्ग;
		}

		error = nscss_पञ्जिकाकोशः_import(c, c->imports[index].c);
		यदि (error != CSS_OK)
			प्रत्यागम error;
	}

	/* Record identity of the next import to register */
	c->अग्र_गन्तव्यम्_पञ्जिकाकोशः = (uint32_t) index;

	यदि (c->अग्र_गन्तव्यम्_पञ्जिकाकोशः == c->import_गणनम्) {
		/* No more imports: notify parent that we're DONE */
		c->समाप्तम्(c, c->pw);
	}

	प्रत्यागम CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_पञ्जिकाकोशः_import(संरचना content_css_दत्त *c,
		ध्रुव hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	यदि (import != रिक्त) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->दत्त.sheet;
	} अन्यथा {
		/* Create a blank sheet if needed. */
		यदि (blank_import == रिक्त) {
			css_stylesheet_params params;

			params.params_संस्करणम् = CSS_STYLESHEET_PARAMS_संस्करणम्_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = रिक्त;
			params.url = "";
			params.title = रिक्त;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = रिक्त;
			params.import = रिक्त;
			params.import_pw = रिक्त;
			params.color = ns_system_रङ्गः;
			params.color_pw = रिक्त;
			params.font = रिक्त;
			params.font_pw = रिक्त;

			error = css_stylesheet_create(&params, &blank_import);
			यदि (error != CSS_OK) {
				प्रत्यागम error;
			}

			error = css_stylesheet_दत्त_समाप्तम्(blank_import);
			यदि (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				प्रत्यागम error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_पञ्जिकाकोशः_import(c->sheet, sheet);
	यदि (error != CSS_OK) {
		प्रत्यागम error;
	}

	प्रत्यागम error;
}

/**
 * Clean up after the CSS content handler
 */
स्थिर शून्य nscss_fini(शून्य)
{
	यदि (blank_import != रिक्त) {
		css_stylesheet_destroy(blank_import);
		blank_import = रिक्त;
	}
	css_hint_fini();
}

स्थिर ध्रुव content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_दत्त = nscss_process_दत्त,
	.दत्त_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(शून्य)
{
	nserror error;

	error = content_factory_पञ्जिकाकोशः_handler("text/css",
			&css_content_handler);
	यदि (error != NSERROR_OK)
		गच्छ error;

	error = css_hint_init();
	यदि (error != NSERROR_OK)
		गच्छ error;

	प्रत्यागम NSERROR_OK;

error:
	nscss_fini();

	प्रत्यागम error;
}
