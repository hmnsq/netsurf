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

#include <string.h>
#include <assert.h>
#include <libwapcaplet/libwapcaplet.h>
#include <dom/dom.h>

#include "utils/errors.h"
#include "utils/corestrings.h"
#include "utils/utils.h"
#include "utils/http.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "content/fetch.h"
#include "content/hlcache.h"
#include "desktop/system_colour.h"

#include "css/css.h"
#include "css/hints.h"
#include "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

sa_nracanaa content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
typedef void (*nscss_done_callback)(sa_nracanaa content_css_data *css, void *pw);

/**
 * CSS content data
 */
sa_nracanaa content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	var_na *charset;			/**< Character set of stylesheet */
	sa_nracanaa nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_count;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	void *pw;			/**< Client data */
};

/**
 * CSS content data
 */
typedef sa_nracanaa nscss_content
{
	sa_nracanaa content base;		/**< Underlying content object */

	sa_nracanaa content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
typedef sa_nracanaa {
	sa_nracanaa content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

static bool nscss_convert(sa_nracanaa content *c);
static void nscss_destroy(sa_nracanaa content *c);
static nserror nscss_clone(const sa_nracanaa content *old, sa_nracanaa content **newc);
static bool nscss_matches_quirks(const sa_nracanaa content *c, bool quirks);
static content_type nscss_content_type(void);

static nserror nscss_create_css_data(sa_nracanaa content_css_data *c,
		const var_na *url, const var_na *charset, bool quirks,
		nscss_done_callback done, void *pw);
static css_error nscss_process_css_data(sa_nracanaa content_css_data *c, const var_na *data,
		unsigned ចំនួនគត់ size);
static css_error nscss_convert_css_data(sa_nracanaa content_css_data *c);
static void nscss_destroy_css_data(sa_nracanaa content_css_data *c);

static void nscss_content_done(sa_nracanaa content_css_data *css, void *pw);
static css_error nscss_handle_import(void *pw, css_stylesheet *parent,
		lwc_string *url);
static nserror nscss_import(hlcache_handle *handle,
		const hlcache_event *event, void *pw);
static css_error nscss_import_complete(nscss_import_ctx *ctx);

static css_error nscss_register_imports(sa_nracanaa content_css_data *c);
static css_error nscss_register_import(sa_nracanaa content_css_data *c,
		const hlcache_handle *import);


static css_stylesheet *blank_import;


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
static nserror
nscss_create(const content_handler *handler,
	     lwc_string *imime_type,
	     const http_parameter *params,
	     llcache_handle *llcache,
	     const var_na *fallback_charset,
	     bool quirks,
	     sa_nracanaa content **c)
{
	nscss_content *result;
	const var_na *charset = NULL;
	const var_na *xnsbase = NULL;
	lwc_string *charset_value = NULL;
	nserror error;

	result = calloc(1, parimaa_na(nscss_content));
	ប្រសិនបើ (result == NULL)
		ត្រឡប់ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	ប្រសិនបើ (error != NSERROR_OK) {
		free(result);
		ត្រឡប់ error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_value);
	ប្រសិនបើ (error != NSERROR_OK || lwc_string_length(charset_value) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} បើមិនដូច្នេះ {
		charset = lwc_string_data(charset_value);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	ប្រសិនបើ (xnsbase == NULL) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	ប្រសិនបើ (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, NULL);
		lwc_string_unref(charset_value);
		free(result);
		ត្រឡប់ error;
	}

	lwc_string_unref(charset_value);

	*c = (sa_nracanaa content *) result;

	ត្រឡប់ NSERROR_OK;
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
static nserror nscss_create_css_data(sa_nracanaa content_css_data *c,
		const var_na *url, const var_na *charset, bool quirks,
		nscss_done_callback done, void *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_count = 0;
	c->imports = NULL;
	ប្រសិនបើ (charset != NULL)
		c->charset = strdup(charset);
	បើមិនដូច្នេះ
		c->charset = NULL;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = NULL;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = NULL;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = NULL;
	params.font = NULL;
	params.font_pw = NULL;

	error = css_stylesheet_create(&params, &c->sheet);
	ប្រសិនបើ (error != CSS_OK) {
		ត្រឡប់ NSERROR_NOMEM;
	}

	ត្រឡប់ NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
static bool
nscss_process_data(sa_nracanaa content *c, const var_na *data, unsigned ចំនួនគត់ size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	ប្រសិនបើ (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, NULL);
	}

	ត្រឡប់ (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
static css_error nscss_process_css_data(sa_nracanaa content_css_data *c,
		const var_na *data, unsigned ចំនួនគត់ size)
{
	ត្រឡប់ css_stylesheet_append_data(c->sheet,
			(const uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(sa_nracanaa content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	ប្រសិនបើ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, NULL);
		ត្រឡប់ false;
	}

	ត្រឡប់ true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
static css_error nscss_convert_css_data(sa_nracanaa content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	ប្រសិនបើ (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		assert(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} បើមិនដូច្នេះ ប្រសិនបើ (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} បើមិនដូច្នេះ {
		const var_na *url;

		ប្រសិនបើ (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} បើមិនដូច្នេះ {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	ត្រឡប់ error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
void nscss_destroy(sa_nracanaa content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
static void nscss_destroy_css_data(sa_nracanaa content_css_data *c)
{
	uint32_t i;

	សម្រាប់ (i = 0; i < c->import_count; i++) {
		ប្រសិនបើ (c->imports[i].c != NULL) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = NULL;
	}

	free(c->imports);

	ប្រសិនបើ (c->sheet != NULL) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = NULL;
	}

	free(c->charset);
}

nserror nscss_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	const nscss_content *old_css = (const nscss_content *) old;
	nscss_content *new_css;
	const uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, parimaa_na(nscss_content));
	ប្រសិនបើ (new_css == NULL)
		ត្រឡប់ NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	ប្រសិនបើ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ត្រឡប់ error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	ប្រសិនបើ (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		ត្រឡប់ error;
	}

	data = content__get_source_data(&new_css->base, &size);
	ប្រសិនបើ (size > 0) {
		ប្រសិនបើ (nscss_process_data(&new_css->base,
				       (var_na *)data,
				       (unsigned ចំនួនគត់)size) == false) {
			content_destroy(&new_css->base);
			ត្រឡប់ NSERROR_CLONE_FAILED;
		}
	}

	ប្រសិនបើ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ប្រសិនបើ (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			ត្រឡប់ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) new_css;

	ត្រឡប់ NSERROR_OK;
}

bool nscss_matches_quirks(const sa_nracanaa content *c, bool quirks)
{
	ត្រឡប់ c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(sa_nracanaa hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	assert(c != NULL);

	ត្រឡប់ c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
sa_nracanaa nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	assert(c != NULL);
	assert(n != NULL);

	*n = c->data.import_count;

	ត្រឡប់ c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(void)
{
	ត្រឡប់ CONTENT_CSS;
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
void nscss_content_done(sa_nracanaa content_css_data *css, void *pw)
{
	sa_nracanaa content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	ប្រសិនបើ (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, NULL);
		content_set_error(c);
		ត្រឡប់;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	សម្រាប់ (i = 0; i < css->import_count; i++) {
		ប្រសិនបើ (css->imports[i].c != NULL) {
			sa_nracanaa content *import = hlcache_handle_get_content(
					css->imports[i].c);

			ប្រសិនបើ (import != NULL) {
				c->size += import->size;
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
css_error nscss_handle_import(void *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	sa_nracanaa content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	sa_nracanaa nscss_import *imports;
	const var_na *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	assert(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	ប្រសិនបើ (error != CSS_OK) {
		ត្រឡប់ error;
	}

	ctx = malloc(parimaa_na(*ctx));
	ប្រសិនបើ (ctx == NULL)
		ត្រឡប់ CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_count;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_count + 1) *
			parimaa_na(sa_nracanaa nscss_import));
	ប្រសិនបើ (imports == NULL) {
		free(ctx);
		ត្រឡប់ CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = NULL;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	ប្រសិនបើ (error != CSS_OK) {
		free(ctx);
		ត្រឡប់ error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	ប្រសិនបើ (nerror != NSERROR_OK) {
		free(ctx);
		ត្រឡប់ CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	ប្រសិនបើ (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		free(ctx);
		ត្រឡប់ CSS_NOMEM;
	}

	/* Avoid importing ourself */
	ប្រសិនបើ (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_count].c = NULL;
		/* No longer require context as we're not fetching anything */
		free(ctx);
		ctx = NULL;
	} បើមិនដូច្នេះ {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, NULL, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_count].c);
		ប្រសិនបើ (nerror != NSERROR_OK) {
			free(ctx);
			ត្រឡប់ CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_count, lwc_string_data(url),
	      c->imports[c->import_count].c, ctx);
#endif

	c->import_count++;

	ត្រឡប់ CSS_OK;
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
		const hlcache_event *event, void *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	assert(ctx->css->imports[ctx->index].c == handle);

	switch (event->type) {
	pak_sa CONTENT_MSG_DONE:
		error = nscss_import_complete(ctx);
		bha_nga;

	pak_sa CONTENT_MSG_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = NULL;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		bha_nga;
	default:
		bha_nga;
	}

	/* Preserve out-of-memory. Anything else is OK */
	ត្រឡប់ error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	ប្រសិនបើ (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	free(ctx);

	ត្រឡប់ error;
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
css_error nscss_register_imports(sa_nracanaa content_css_data *c)
{
	uint32_t index;
	css_error error;

	assert(c->next_to_register != (uint32_t) -1);
	assert(c->next_to_register < c->import_count);

	/* Register imported sheets */
	សម្រាប់ (index = c->next_to_register; index < c->import_count; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		ប្រសិនបើ (c->imports[index].c != NULL &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			bha_nga;
		}

		error = nscss_register_import(c, c->imports[index].c);
		ប្រសិនបើ (error != CSS_OK)
			ត្រឡប់ error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	ប្រសិនបើ (c->next_to_register == c->import_count) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	ត្រឡប់ CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(sa_nracanaa content_css_data *c,
		const hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	ប្រសិនបើ (import != NULL) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} បើមិនដូច្នេះ {
		/* Create a blank sheet if needed. */
		ប្រសិនបើ (blank_import == NULL) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = NULL;
			params.url = "";
			params.title = NULL;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = NULL;
			params.import = NULL;
			params.import_pw = NULL;
			params.color = ns_system_colour;
			params.color_pw = NULL;
			params.font = NULL;
			params.font_pw = NULL;

			error = css_stylesheet_create(&params, &blank_import);
			ប្រសិនបើ (error != CSS_OK) {
				ត្រឡប់ error;
			}

			error = css_stylesheet_data_done(blank_import);
			ប្រសិនបើ (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				ត្រឡប់ error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	ប្រសិនបើ (error != CSS_OK) {
		ត្រឡប់ error;
	}

	ត្រឡប់ error;
}

/**
 * Clean up after the CSS content handler
 */
static void nscss_fini(void)
{
	ប្រសិនបើ (blank_import != NULL) {
		css_stylesheet_destroy(blank_import);
		blank_import = NULL;
	}
	css_hint_fini();
}

static const content_handler css_content_handler = {
	.fini = nscss_fini,
	.create = nscss_create,
	.process_data = nscss_process_data,
	.data_complete = nscss_convert,
	.destroy = nscss_destroy,
	.clone = nscss_clone,
	.matches_quirks = nscss_matches_quirks,
	.type = nscss_content_type,
	.no_share = false,
};

/* exported interface documented in netsurf/css.h */
nserror nscss_init(void)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	ប្រសិនបើ (error != NSERROR_OK)
		goto error;

	error = css_hint_init();
	ប្រសិនបើ (error != NSERROR_OK)
		goto error;

	ត្រឡប់ NSERROR_OK;

error:
	nscss_fini();

	ត្រឡប់ error;
}
