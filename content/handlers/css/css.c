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

#شامل <string.h>
#شامل <assert.h>
#شامل <libwapcaplet/libwapcaplet.h>
#شامل <dom/dom.h>

#شامل "utils/errors.h"
#شامل "utils/corestrings.h"
#شامل "utils/utils.h"
#شامل "utils/http.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "content/fetch.h"
#شامل "content/hlcache.h"
#شامل "desktop/system_colour.h"

#شامل "css/css.h"
#شامل "css/hints.h"
#شامل "css/internal.h"

/* Define to trace import fetches */
#تعریف_منسوخ NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ساخت content_css_ڈیٹا;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
قسمتعریف خالی (*nscss_مکمل_callback)(ساخت content_css_ڈیٹا *css, خالی *pw);

/**
 * CSS content data
 */
ساخت content_css_ڈیٹا
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	حرف *charset;			/**< Character set of stylesheet */
	ساخت nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_گنتی;		/**< Number of sheets imported */
	uint32_t اگلا_مقصد_رجسٹر;	/**< Index of next import to register */
	nscss_مکمل_callback مکمل;	/**< Completion callback */
	خالی *pw;			/**< Client data */
};

/**
 * CSS content data
 */
قسمتعریف ساخت nscss_content
{
	ساخت content بنیاد;		/**< Underlying content object */

	ساخت content_css_ڈیٹا ڈیٹا;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
قسمتعریف ساخت {
	ساخت content_css_ڈیٹا *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ساکن bool nscss_convert(ساخت content *c);
ساکن خالی nscss_destroy(ساخت content *c);
ساکن nserror nscss_clone(ثابت ساخت content *old, ساخت content **newc);
ساکن bool nscss_matches_quirks(ثابت ساخت content *c, bool quirks);
ساکن content_type nscss_content_type(خالی);

ساکن nserror nscss_create_css_ڈیٹا(ساخت content_css_ڈیٹا *c,
		ثابت حرف *url, ثابت حرف *charset, bool quirks,
		nscss_مکمل_callback مکمل, خالی *pw);
ساکن css_error nscss_process_css_ڈیٹا(ساخت content_css_ڈیٹا *c, ثابت حرف *ڈیٹا,
		غیرعلامتی صحیح سائز);
ساکن css_error nscss_convert_css_ڈیٹا(ساخت content_css_ڈیٹا *c);
ساکن خالی nscss_destroy_css_ڈیٹا(ساخت content_css_ڈیٹا *c);

ساکن خالی nscss_content_مکمل(ساخت content_css_ڈیٹا *css, خالی *pw);
ساکن css_error nscss_handle_import(خالی *pw, css_stylesheet *والد,
		lwc_string *url);
ساکن nserror nscss_import(hlcache_handle *handle,
		ثابت hlcache_event *event, خالی *pw);
ساکن css_error nscss_import_complete(nscss_import_ctx *ctx);

ساکن css_error nscss_رجسٹر_imports(ساخت content_css_ڈیٹا *c);
ساکن css_error nscss_رجسٹر_import(ساخت content_css_ڈیٹا *c,
		ثابت hlcache_handle *import);


ساکن css_stylesheet *blank_import;


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
ساکن nserror
nscss_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت http_parameter *params,
	     llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     ساخت content **c)
{
	nscss_content *نتیجہ;
	ثابت حرف *charset = تہی;
	ثابت حرف *xnsbase = تہی;
	lwc_string *charset_قدر = تہی;
	nserror error;

	نتیجہ = صفر_حافظہ_دو(1, حجم(nscss_content));
	اگر (نتیجہ == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&نتیجہ->بنیاد, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(نتیجہ);
		واپس error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_ڈھونڈو_شے(params, corestring_lwc_charset,
			&charset_قدر);
	اگر (error != NSERROR_OK || lwc_string_لمبائی(charset_قدر) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} ورنہ {
		charset = lwc_string_ڈیٹا(charset_قدر);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_سرخی(llcache, "X-NS-Base");
	اگر (xnsbase == تہی) {
		xnsbase = nsurl_access(content_get_url(&نتیجہ->بنیاد));
	}

	error = nscss_create_css_ڈیٹا(&نتیجہ->ڈیٹا,
			xnsbase, charset, نتیجہ->بنیاد.quirks,
			nscss_content_مکمل, نتیجہ);
	اگر (error != NSERROR_OK) {
		content_نشر_error(&نتیجہ->بنیاد, NSERROR_NOMEM, تہی);
		lwc_string_unref(charset_قدر);
		آزاد(نتیجہ);
		واپس error;
	}

	lwc_string_unref(charset_قدر);

	*c = (ساخت content *) نتیجہ;

	واپس NSERROR_OK;
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
ساکن nserror nscss_create_css_ڈیٹا(ساخت content_css_ڈیٹا *c,
		ثابت حرف *url, ثابت حرف *charset, bool quirks,
		nscss_مکمل_callback مکمل, خالی *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->مکمل = مکمل;
	c->اگلا_مقصد_رجسٹر = (uint32_t) -1;
	c->import_گنتی = 0;
	c->imports = تہی;
	اگر (charset != تہی)
		c->charset = strdup(charset);
	ورنہ
		c->charset = تہی;

	params.params_نسخہ = CSS_STYLESHEET_PARAMS_نسخہ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = تہی;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = تہی;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_رنگ;
	params.color_pw = تہی;
	params.font = تہی;
	params.font_pw = تہی;

	error = css_stylesheet_create(&params, &c->sheet);
	اگر (error != CSS_OK) {
		واپس NSERROR_NOMEM;
	}

	واپس NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ساکن bool
nscss_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا, غیرعلامتی صحیح سائز)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_ڈیٹا(&css->ڈیٹا, ڈیٹا, سائز);
	اگر (error != CSS_OK && error != CSS_NEEDDATA) {
		content_نشر_error(c, NSERROR_CSS, تہی);
	}

	واپس (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ساکن css_error nscss_process_css_ڈیٹا(ساخت content_css_ڈیٹا *c,
		ثابت حرف *ڈیٹا, غیرعلامتی صحیح سائز)
{
	واپس css_stylesheet_append_ڈیٹا(c->sheet,
			(ثابت uint8_t *) ڈیٹا, سائز);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ساخت content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_ڈیٹا(&css->ڈیٹا);
	اگر (error != CSS_OK) {
		content_نشر_error(c, NSERROR_CSS, تہی);
		واپس false;
	}

	واپس true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ساکن css_error nscss_convert_css_ڈیٹا(ساخت content_css_ڈیٹا *c)
{
	css_error error;

	error = css_stylesheet_ڈیٹا_مکمل(c->sheet);

	/* Process pending imports */
	اگر (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		أكد(c->اگلا_مقصد_رجسٹر == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->اگلا_مقصد_رجسٹر = 0;
		error = nscss_رجسٹر_imports(c);
	} ورنہ اگر (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->مکمل(c, c->pw);
	} ورنہ {
		ثابت حرف *url;

		اگر (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} ورنہ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	واپس error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
خالی nscss_destroy(ساخت content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_ڈیٹا(&css->ڈیٹا);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ساکن خالی nscss_destroy_css_ڈیٹا(ساخت content_css_ڈیٹا *c)
{
	uint32_t i;

	برائے (i = 0; i < c->import_گنتی; i++) {
		اگر (c->imports[i].c != تہی) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = تہی;
	}

	آزاد(c->imports);

	اگر (c->sheet != تہی) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = تہی;
	}

	آزاد(c->charset);
}

nserror nscss_clone(ثابت ساخت content *old, ساخت content **newc)
{
	ثابت nscss_content *old_css = (ثابت nscss_content *) old;
	nscss_content *new_css;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;
	nserror error;

	new_css = صفر_حافظہ_دو(1, حجم(nscss_content));
	اگر (new_css == تہی)
		واپس NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_css->بنیاد);
		واپس error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_ڈیٹا(&new_css->ڈیٹا,
			nsurl_access(content_get_url(&new_css->بنیاد)),
			old_css->ڈیٹا.charset,
			new_css->بنیاد.quirks,
			nscss_content_مکمل, new_css);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_css->بنیاد);
		واپس error;
	}

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(&new_css->بنیاد, &سائز);
	اگر (سائز > 0) {
		اگر (nscss_process_ڈیٹا(&new_css->بنیاد,
				       (حرف *)ڈیٹا,
				       (غیرعلامتی صحیح)سائز) == false) {
			content_destroy(&new_css->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY ||
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (nscss_convert(&new_css->بنیاد) == false) {
			content_destroy(&new_css->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) new_css;

	واپس NSERROR_OK;
}

bool nscss_matches_quirks(ثابت ساخت content *c, bool quirks)
{
	واپس c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ساخت hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);

	واپس c->ڈیٹا.sheet;
}

/* exported interface documented in netsurf/css.h */
ساخت nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != تہی);
	أكد(n != تہی);

	*n = c->ڈیٹا.import_گنتی;

	واپس c->ڈیٹا.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(خالی)
{
	واپس CONTENT_CSS;
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
خالی nscss_content_مکمل(ساخت content_css_ڈیٹا *css, خالی *pw)
{
	ساخت content *c = pw;
	uint32_t i;
	size_t سائز;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_سائز(css->sheet, &سائز);
	اگر (error != CSS_OK) {
		content_نشر_error(c, NSERROR_CSS, تہی);
		content_سیٹ_error(c);
		واپس;
	}
	c->سائز += سائز;

	/* Add on the size of the imported sheets */
	برائے (i = 0; i < css->import_گنتی; i++) {
		اگر (css->imports[i].c != تہی) {
			ساخت content *import = hlcache_handle_get_content(
					css->imports[i].c);

			اگر (import != تہی) {
				c->سائز += import->سائز;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
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
css_error nscss_handle_import(خالی *pw, css_stylesheet *والد,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ساخت content_css_ڈیٹا *c = pw;
	nscss_import_ctx *ctx;
	hlcache_بچہ_context بچہ;
	ساخت nscss_import *imports;
	ثابت حرف *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	أكد(والد == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	اگر (error != CSS_OK) {
		واپس error;
	}

	ctx = حافظہ_دو(حجم(*ctx));
	اگر (ctx == تہی)
		واپس CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_گنتی;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_گنتی + 1) *
			حجم(ساخت nscss_import));
	اگر (imports == تہی) {
		آزاد(ctx);
		واپس CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	بچہ.charset = تہی;
	error = css_stylesheet_quirks_allowed(c->sheet, &بچہ.quirks);
	اگر (error != CSS_OK) {
		آزاد(ctx);
		واپس error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_ڈیٹا(url), &ns_url);
	اگر (nerror != NSERROR_OK) {
		آزاد(ctx);
		واپس CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	اگر (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		آزاد(ctx);
		واپس CSS_NOMEM;
	}

	/* Avoid importing ourself */
	اگر (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_گنتی].c = تہی;
		/* No longer require context as we're not fetching anything */
		آزاد(ctx);
		ctx = تہی;
	} ورنہ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, تہی, nscss_import, ctx,
				&بچہ, accept,
				&c->imports[c->import_گنتی].c);
		اگر (nerror != NSERROR_OK) {
			آزاد(ctx);
			واپس CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#اگر_تعریف_شدہ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_گنتی, lwc_string_ڈیٹا(url),
	      c->imports[c->import_گنتی].c, ctx);
#اگر_ختم

	c->import_گنتی++;

	واپس CSS_OK;
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
		ثابت hlcache_event *event, خالی *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#اگر_تعریف_شدہ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#اگر_ختم

	أكد(ctx->css->imports[ctx->index].c == handle);

	انتخاب (event->type) {
	حالت CONTENT_پیغام_مکمل:
		error = nscss_import_complete(ctx);
		توڑو;

	حالت CONTENT_پیغام_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = تہی;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		توڑو;
	طےشدہ:
		توڑو;
	}

	/* Preserve out-of-memory. Anything else is OK */
	واپس error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	اگر (ctx->css->اگلا_مقصد_رجسٹر == ctx->index)
		error = nscss_رجسٹر_imports(ctx->css);

#اگر_تعریف_شدہ NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#اگر_ختم

	/* No longer need import context */
	آزاد(ctx);

	واپس error;
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
css_error nscss_رجسٹر_imports(ساخت content_css_ڈیٹا *c)
{
	uint32_t index;
	css_error error;

	أكد(c->اگلا_مقصد_رجسٹر != (uint32_t) -1);
	أكد(c->اگلا_مقصد_رجسٹر < c->import_گنتی);

	/* Register imported sheets */
	برائے (index = c->اگلا_مقصد_رجسٹر; index < c->import_گنتی; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		اگر (c->imports[index].c != تہی &&
			content_get_حالت(c->imports[index].c) !=
				CONTENT_حالت_مکمل) {
			توڑو;
		}

		error = nscss_رجسٹر_import(c, c->imports[index].c);
		اگر (error != CSS_OK)
			واپس error;
	}

	/* Record identity of the next import to register */
	c->اگلا_مقصد_رجسٹر = (uint32_t) index;

	اگر (c->اگلا_مقصد_رجسٹر == c->import_گنتی) {
		/* No more imports: notify parent that we're DONE */
		c->مکمل(c, c->pw);
	}

	واپس CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_رجسٹر_import(ساخت content_css_ڈیٹا *c,
		ثابت hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	اگر (import != تہی) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->ڈیٹا.sheet;
	} ورنہ {
		/* Create a blank sheet if needed. */
		اگر (blank_import == تہی) {
			css_stylesheet_params params;

			params.params_نسخہ = CSS_STYLESHEET_PARAMS_نسخہ_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = تہی;
			params.url = "";
			params.title = تہی;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = تہی;
			params.import = تہی;
			params.import_pw = تہی;
			params.color = ns_system_رنگ;
			params.color_pw = تہی;
			params.font = تہی;
			params.font_pw = تہی;

			error = css_stylesheet_create(&params, &blank_import);
			اگر (error != CSS_OK) {
				واپس error;
			}

			error = css_stylesheet_ڈیٹا_مکمل(blank_import);
			اگر (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				واپس error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_رجسٹر_import(c->sheet, sheet);
	اگر (error != CSS_OK) {
		واپس error;
	}

	واپس error;
}

/**
 * Clean up after the CSS content handler
 */
ساکن خالی nscss_fini(خالی)
{
	اگر (blank_import != تہی) {
		css_stylesheet_destroy(blank_import);
		blank_import = تہی;
	}
	css_hint_fini();
}

ساکن ثابت content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_ڈیٹا = nscss_process_ڈیٹا,
	.ڈیٹا_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(خالی)
{
	nserror error;

	error = content_factory_رجسٹر_handler("text/css",
			&css_content_handler);
	اگر (error != NSERROR_OK)
		جاؤ error;

	error = css_hint_init();
	اگر (error != NSERROR_OK)
		جاؤ error;

	واپس NSERROR_OK;

error:
	nscss_fini();

	واپس error;
}
