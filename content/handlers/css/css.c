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

#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <libwapcaplet/libwapcaplet.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/http.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/fetch.h"
#అన్తర్భావ "content/hlcache.h"
#అన్తర్భావ "desktop/system_colour.h"

#అన్తర్భావ "css/css.h"
#అన్తర్భావ "css/hints.h"
#అన్తర్భావ "css/internal.h"

/* Define to trace import fetches */
#నిర్వచనం_రద్దు NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

నిర్మాణం content_css_దత్తాంశం;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
రకనిర్వచనం శూన్య (*nscss_పూర్తి_callback)(నిర్మాణం content_css_దత్తాంశం *css, శూన్య *pw);

/**
 * CSS content data
 */
నిర్మాణం content_css_దత్తాంశం
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	అక్షరం *charset;			/**< Character set of stylesheet */
	నిర్మాణం nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_లెక్క;		/**< Number of sheets imported */
	uint32_t తదుపరి_గమ్యం_రిజిస్టరు;	/**< Index of next import to register */
	nscss_పూర్తి_callback పూర్తి;	/**< Completion callback */
	శూన్య *pw;			/**< Client data */
};

/**
 * CSS content data
 */
రకనిర్వచనం నిర్మాణం nscss_content
{
	నిర్మాణం content పునాది;		/**< Underlying content object */

	నిర్మాణం content_css_దత్తాంశం దత్తాంశం;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
రకనిర్వచనం నిర్మాణం {
	నిర్మాణం content_css_దత్తాంశం *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

స్థిర bool nscss_convert(నిర్మాణం content *c);
స్థిర శూన్య nscss_destroy(నిర్మాణం content *c);
స్థిర nserror nscss_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc);
స్థిర bool nscss_matches_quirks(స్థిరరాశి నిర్మాణం content *c, bool quirks);
స్థిర content_type nscss_content_type(శూన్య);

స్థిర nserror nscss_create_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c,
		స్థిరరాశి అక్షరం *url, స్థిరరాశి అక్షరం *charset, bool quirks,
		nscss_పూర్తి_callback పూర్తి, శూన్య *pw);
స్థిర css_error nscss_process_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c, స్థిరరాశి అక్షరం *దత్తాంశం,
		గుర్తులేని పూర్ణాంకం కొలత);
స్థిర css_error nscss_convert_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c);
స్థిర శూన్య nscss_destroy_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c);

స్థిర శూన్య nscss_content_పూర్తి(నిర్మాణం content_css_దత్తాంశం *css, శూన్య *pw);
స్థిర css_error nscss_handle_import(శూన్య *pw, css_stylesheet *మాతృ,
		lwc_string *url);
స్థిర nserror nscss_import(hlcache_handle *handle,
		స్థిరరాశి hlcache_event *event, శూన్య *pw);
స్థిర css_error nscss_import_complete(nscss_import_ctx *ctx);

స్థిర css_error nscss_రిజిస్టరు_imports(నిర్మాణం content_css_దత్తాంశం *c);
స్థిర css_error nscss_రిజిస్టరు_import(నిర్మాణం content_css_దత్తాంశం *c,
		స్థిరరాశి hlcache_handle *import);


స్థిర css_stylesheet *blank_import;


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
స్థిర nserror
nscss_create(స్థిరరాశి content_handler *handler,
	     lwc_string *imime_type,
	     స్థిరరాశి http_parameter *params,
	     llcache_handle *llcache,
	     స్థిరరాశి అక్షరం *fallback_charset,
	     bool quirks,
	     నిర్మాణం content **c)
{
	nscss_content *ఫలితం;
	స్థిరరాశి అక్షరం *charset = ఖాళీ;
	స్థిరరాశి అక్షరం *xnsbase = ఖాళీ;
	lwc_string *charset_విలువ = ఖాళీ;
	nserror error;

	ఫలితం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nscss_content));
	ఒకవేళ (ఫలితం == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&ఫలితం->పునాది, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_కనుగొను_అంశం(params, corestring_lwc_charset,
			&charset_విలువ);
	ఒకవేళ (error != NSERROR_OK || lwc_string_నిడివి(charset_విలువ) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} లేకపోతే {
		charset = lwc_string_దత్తాంశం(charset_విలువ);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_శీర్షిక(llcache, "X-NS-Base");
	ఒకవేళ (xnsbase == ఖాళీ) {
		xnsbase = nsurl_access(content_get_url(&ఫలితం->పునాది));
	}

	error = nscss_create_css_దత్తాంశం(&ఫలితం->దత్తాంశం,
			xnsbase, charset, ఫలితం->పునాది.quirks,
			nscss_content_పూర్తి, ఫలితం);
	ఒకవేళ (error != NSERROR_OK) {
		content_ప్రసారం_error(&ఫలితం->పునాది, NSERROR_NOMEM, ఖాళీ);
		lwc_string_unref(charset_విలువ);
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	lwc_string_unref(charset_విలువ);

	*c = (నిర్మాణం content *) ఫలితం;

	తిరిగి NSERROR_OK;
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
స్థిర nserror nscss_create_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c,
		స్థిరరాశి అక్షరం *url, స్థిరరాశి అక్షరం *charset, bool quirks,
		nscss_పూర్తి_callback పూర్తి, శూన్య *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->పూర్తి = పూర్తి;
	c->తదుపరి_గమ్యం_రిజిస్టరు = (uint32_t) -1;
	c->import_లెక్క = 0;
	c->imports = ఖాళీ;
	ఒకవేళ (charset != ఖాళీ)
		c->charset = strdup(charset);
	లేకపోతే
		c->charset = ఖాళీ;

	params.params_సంస్కరణ = CSS_STYLESHEET_PARAMS_సంస్కరణ_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ఖాళీ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ఖాళీ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_రంగు;
	params.color_pw = ఖాళీ;
	params.font = ఖాళీ;
	params.font_pw = ఖాళీ;

	error = css_stylesheet_create(&params, &c->sheet);
	ఒకవేళ (error != CSS_OK) {
		తిరిగి NSERROR_NOMEM;
	}

	తిరిగి NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
స్థిర bool
nscss_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం, గుర్తులేని పూర్ణాంకం కొలత)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_దత్తాంశం(&css->దత్తాంశం, దత్తాంశం, కొలత);
	ఒకవేళ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_ప్రసారం_error(c, NSERROR_CSS, ఖాళీ);
	}

	తిరిగి (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
స్థిర css_error nscss_process_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c,
		స్థిరరాశి అక్షరం *దత్తాంశం, గుర్తులేని పూర్ణాంకం కొలత)
{
	తిరిగి css_stylesheet_append_దత్తాంశం(c->sheet,
			(స్థిరరాశి uint8_t *) దత్తాంశం, కొలత);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(నిర్మాణం content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_దత్తాంశం(&css->దత్తాంశం);
	ఒకవేళ (error != CSS_OK) {
		content_ప్రసారం_error(c, NSERROR_CSS, ఖాళీ);
		తిరిగి false;
	}

	తిరిగి true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
స్థిర css_error nscss_convert_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c)
{
	css_error error;

	error = css_stylesheet_దత్తాంశం_పూర్తి(c->sheet);

	/* Process pending imports */
	ఒకవేళ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		ప్రతిజ్ఞా(c->తదుపరి_గమ్యం_రిజిస్టరు == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->తదుపరి_గమ్యం_రిజిస్టరు = 0;
		error = nscss_రిజిస్టరు_imports(c);
	} లేకపోతే ఒకవేళ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->పూర్తి(c, c->pw);
	} లేకపోతే {
		స్థిరరాశి అక్షరం *url;

		ఒకవేళ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} లేకపోతే {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	తిరిగి error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
శూన్య nscss_destroy(నిర్మాణం content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_దత్తాంశం(&css->దత్తాంశం);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
స్థిర శూన్య nscss_destroy_css_దత్తాంశం(నిర్మాణం content_css_దత్తాంశం *c)
{
	uint32_t i;

	కొరకు (i = 0; i < c->import_లెక్క; i++) {
		ఒకవేళ (c->imports[i].c != ఖాళీ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ఖాళీ;
	}

	విడుదల(c->imports);

	ఒకవేళ (c->sheet != ఖాళీ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ఖాళీ;
	}

	విడుదల(c->charset);
}

nserror nscss_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	స్థిరరాశి nscss_content *old_css = (స్థిరరాశి nscss_content *) old;
	nscss_content *new_css;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
	nserror error;

	new_css = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nscss_content));
	ఒకవేళ (new_css == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&new_css->పునాది);
		తిరిగి error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_దత్తాంశం(&new_css->దత్తాంశం,
			nsurl_access(content_get_url(&new_css->పునాది)),
			old_css->దత్తాంశం.charset,
			new_css->పునాది.quirks,
			nscss_content_పూర్తి, new_css);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&new_css->పునాది);
		తిరిగి error;
	}

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(&new_css->పునాది, &కొలత);
	ఒకవేళ (కొలత > 0) {
		ఒకవేళ (nscss_process_దత్తాంశం(&new_css->పునాది,
				       (అక్షరం *)దత్తాంశం,
				       (గుర్తులేని పూర్ణాంకం)కొలత) == false) {
			content_destroy(&new_css->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (nscss_convert(&new_css->పునాది) == false) {
			content_destroy(&new_css->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) new_css;

	తిరిగి NSERROR_OK;
}

bool nscss_matches_quirks(స్థిరరాశి నిర్మాణం content *c, bool quirks)
{
	తిరిగి c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(నిర్మాణం hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);

	తిరిగి c->దత్తాంశం.sheet;
}

/* exported interface documented in netsurf/css.h */
నిర్మాణం nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	ప్రతిజ్ఞా(c != ఖాళీ);
	ప్రతిజ్ఞా(n != ఖాళీ);

	*n = c->దత్తాంశం.import_లెక్క;

	తిరిగి c->దత్తాంశం.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(శూన్య)
{
	తిరిగి CONTENT_CSS;
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
శూన్య nscss_content_పూర్తి(నిర్మాణం content_css_దత్తాంశం *css, శూన్య *pw)
{
	నిర్మాణం content *c = pw;
	uint32_t i;
	size_t కొలత;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_కొలత(css->sheet, &కొలత);
	ఒకవేళ (error != CSS_OK) {
		content_ప్రసారం_error(c, NSERROR_CSS, ఖాళీ);
		content_సమితి_error(c);
		తిరిగి;
	}
	c->కొలత += కొలత;

	/* Add on the size of the imported sheets */
	కొరకు (i = 0; i < css->import_లెక్క; i++) {
		ఒకవేళ (css->imports[i].c != ఖాళీ) {
			నిర్మాణం content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ఒకవేళ (import != ఖాళీ) {
				c->కొలత += import->కొలత;
			}
		}
	}

	/* Finally, catch the content's users up with reality */
	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
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
css_error nscss_handle_import(శూన్య *pw, css_stylesheet *మాతృ,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	నిర్మాణం content_css_దత్తాంశం *c = pw;
	nscss_import_ctx *ctx;
	hlcache_బిడ్డ_context బిడ్డ;
	నిర్మాణం nscss_import *imports;
	స్థిరరాశి అక్షరం *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	ప్రతిజ్ఞా(మాతృ == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ఒకవేళ (error != CSS_OK) {
		తిరిగి error;
	}

	ctx = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*ctx));
	ఒకవేళ (ctx == ఖాళీ)
		తిరిగి CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_లెక్క;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_లెక్క + 1) *
			పరిమాణం(నిర్మాణం nscss_import));
	ఒకవేళ (imports == ఖాళీ) {
		విడుదల(ctx);
		తిరిగి CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	బిడ్డ.charset = ఖాళీ;
	error = css_stylesheet_quirks_allowed(c->sheet, &బిడ్డ.quirks);
	ఒకవేళ (error != CSS_OK) {
		విడుదల(ctx);
		తిరిగి error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_దత్తాంశం(url), &ns_url);
	ఒకవేళ (nerror != NSERROR_OK) {
		విడుదల(ctx);
		తిరిగి CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ఒకవేళ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		విడుదల(ctx);
		తిరిగి CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ఒకవేళ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_లెక్క].c = ఖాళీ;
		/* No longer require context as we're not fetching anything */
		విడుదల(ctx);
		ctx = ఖాళీ;
	} లేకపోతే {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ఖాళీ, nscss_import, ctx,
				&బిడ్డ, accept,
				&c->imports[c->import_లెక్క].c);
		ఒకవేళ (nerror != NSERROR_OK) {
			విడుదల(ctx);
			తిరిగి CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#నిర్వచితమైతే NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_లెక్క, lwc_string_దత్తాంశం(url),
	      c->imports[c->import_లెక్క].c, ctx);
#అయితే_ముగింపు

	c->import_లెక్క++;

	తిరిగి CSS_OK;
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
		స్థిరరాశి hlcache_event *event, శూన్య *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#నిర్వచితమైతే NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#అయితే_ముగింపు

	ప్రతిజ్ఞా(ctx->css->imports[ctx->index].c == handle);

	ఎంపిక (event->type) {
	స్థితి CONTENT_సందేశం_పూర్తి:
		error = nscss_import_complete(ctx);
		ఆపు;

	స్థితి CONTENT_సందేశం_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ఖాళీ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		ఆపు;
	అప్రమేయ:
		ఆపు;
	}

	/* Preserve out-of-memory. Anything else is OK */
	తిరిగి error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ఒకవేళ (ctx->css->తదుపరి_గమ్యం_రిజిస్టరు == ctx->index)
		error = nscss_రిజిస్టరు_imports(ctx->css);

#నిర్వచితమైతే NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#అయితే_ముగింపు

	/* No longer need import context */
	విడుదల(ctx);

	తిరిగి error;
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
css_error nscss_రిజిస్టరు_imports(నిర్మాణం content_css_దత్తాంశం *c)
{
	uint32_t index;
	css_error error;

	ప్రతిజ్ఞా(c->తదుపరి_గమ్యం_రిజిస్టరు != (uint32_t) -1);
	ప్రతిజ్ఞా(c->తదుపరి_గమ్యం_రిజిస్టరు < c->import_లెక్క);

	/* Register imported sheets */
	కొరకు (index = c->తదుపరి_గమ్యం_రిజిస్టరు; index < c->import_లెక్క; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ఒకవేళ (c->imports[index].c != ఖాళీ &&
			content_get_స్థితి(c->imports[index].c) !=
				CONTENT_స్థితి_పూర్తి) {
			ఆపు;
		}

		error = nscss_రిజిస్టరు_import(c, c->imports[index].c);
		ఒకవేళ (error != CSS_OK)
			తిరిగి error;
	}

	/* Record identity of the next import to register */
	c->తదుపరి_గమ్యం_రిజిస్టరు = (uint32_t) index;

	ఒకవేళ (c->తదుపరి_గమ్యం_రిజిస్టరు == c->import_లెక్క) {
		/* No more imports: notify parent that we're DONE */
		c->పూర్తి(c, c->pw);
	}

	తిరిగి CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_రిజిస్టరు_import(నిర్మాణం content_css_దత్తాంశం *c,
		స్థిరరాశి hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ఒకవేళ (import != ఖాళీ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->దత్తాంశం.sheet;
	} లేకపోతే {
		/* Create a blank sheet if needed. */
		ఒకవేళ (blank_import == ఖాళీ) {
			css_stylesheet_params params;

			params.params_సంస్కరణ = CSS_STYLESHEET_PARAMS_సంస్కరణ_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ఖాళీ;
			params.url = "";
			params.title = ఖాళీ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ఖాళీ;
			params.import = ఖాళీ;
			params.import_pw = ఖాళీ;
			params.color = ns_system_రంగు;
			params.color_pw = ఖాళీ;
			params.font = ఖాళీ;
			params.font_pw = ఖాళీ;

			error = css_stylesheet_create(&params, &blank_import);
			ఒకవేళ (error != CSS_OK) {
				తిరిగి error;
			}

			error = css_stylesheet_దత్తాంశం_పూర్తి(blank_import);
			ఒకవేళ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				తిరిగి error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_రిజిస్టరు_import(c->sheet, sheet);
	ఒకవేళ (error != CSS_OK) {
		తిరిగి error;
	}

	తిరిగి error;
}

/**
 * Clean up after the CSS content handler
 */
స్థిర శూన్య nscss_fini(శూన్య)
{
	ఒకవేళ (blank_import != ఖాళీ) {
		css_stylesheet_destroy(blank_import);
		blank_import = ఖాళీ;
	}
	css_hint_fini();
}

స్థిర స్థిరరాశి content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_దత్తాంశం = nscss_process_దత్తాంశం,
	.దత్తాంశం_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(శూన్య)
{
	nserror error;

	error = content_factory_రిజిస్టరు_handler("text/css",
			&css_content_handler);
	ఒకవేళ (error != NSERROR_OK)
		వెళ్ళు error;

	error = css_hint_init();
	ఒకవేళ (error != NSERROR_OK)
		వెళ్ళు error;

	తిరిగి NSERROR_OK;

error:
	nscss_fini();

	తిరిగి error;
}
