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
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

ساختار content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
تعریفنوع تهی (*nscss_done_callback)(ساختار content_css_data *css, تهی *pw);

/**
 * CSS content data
 */
ساختار content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	نویسه *charset;			/**< Character set of stylesheet */
	ساختار nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_شمار;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	تهی *pw;			/**< Client data */
};

/**
 * CSS content data
 */
تعریفنوع ساختار nscss_content
{
	ساختار content base;		/**< Underlying content object */

	ساختار content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
تعریفنوع ساختار {
	ساختار content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

ایستا bool nscss_convert(ساختار content *c);
ایستا تهی nscss_destroy(ساختار content *c);
ایستا nserror nscss_clone(ثابت ساختار content *old, ساختار content **newc);
ایستا bool nscss_matches_quirks(ثابت ساختار content *c, bool quirks);
ایستا content_type nscss_content_type(تهی);

ایستا nserror nscss_create_css_data(ساختار content_css_data *c,
		ثابت نویسه *url, ثابت نویسه *charset, bool quirks,
		nscss_done_callback done, تهی *pw);
ایستا css_error nscss_process_css_data(ساختار content_css_data *c, ثابت نویسه *data,
		unsigned صحیح size);
ایستا css_error nscss_convert_css_data(ساختار content_css_data *c);
ایستا تهی nscss_destroy_css_data(ساختار content_css_data *c);

ایستا تهی nscss_content_done(ساختار content_css_data *css, تهی *pw);
ایستا css_error nscss_handle_import(تهی *pw, css_stylesheet *parent,
		lwc_string *url);
ایستا nserror nscss_import(hlcache_handle *handle,
		ثابت hlcache_event *event, تهی *pw);
ایستا css_error nscss_import_complete(nscss_import_ctx *ctx);

ایستا css_error nscss_register_imports(ساختار content_css_data *c);
ایستا css_error nscss_register_import(ساختار content_css_data *c,
		ثابت hlcache_handle *import);


ایستا css_stylesheet *blank_import;


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
ایستا nserror
nscss_create(ثابت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت http_parameter *params,
	     llcache_handle *llcache,
	     ثابت نویسه *fallback_charset,
	     bool quirks,
	     ساختار content **c)
{
	nscss_content *result;
	ثابت نویسه *charset = پوچ;
	ثابت نویسه *xnsbase = پوچ;
	lwc_string *charset_مقدار = پوچ;
	nserror error;

	result = calloc(1, اندازه(nscss_content));
	اگر (result == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(result);
		بازگشت error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_مقدار);
	اگر (error != NSERROR_OK || lwc_string_length(charset_مقدار) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} وگرنه {
		charset = lwc_string_data(charset_مقدار);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	اگر (xnsbase == پوچ) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	اگر (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, پوچ);
		lwc_string_unref(charset_مقدار);
		آزاد(result);
		بازگشت error;
	}

	lwc_string_unref(charset_مقدار);

	*c = (ساختار content *) result;

	بازگشت NSERROR_OK;
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
ایستا nserror nscss_create_css_data(ساختار content_css_data *c,
		ثابت نویسه *url, ثابت نویسه *charset, bool quirks,
		nscss_done_callback done, تهی *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_شمار = 0;
	c->imports = پوچ;
	اگر (charset != پوچ)
		c->charset = strdup(charset);
	وگرنه
		c->charset = پوچ;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = پوچ;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = پوچ;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = پوچ;
	params.font = پوچ;
	params.font_pw = پوچ;

	error = css_stylesheet_create(&params, &c->sheet);
	اگر (error != CSS_OK) {
		بازگشت NSERROR_NOMEM;
	}

	بازگشت NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
ایستا bool
nscss_process_data(ساختار content *c, ثابت نویسه *data, unsigned صحیح size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	اگر (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, پوچ);
	}

	بازگشت (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
ایستا css_error nscss_process_css_data(ساختار content_css_data *c,
		ثابت نویسه *data, unsigned صحیح size)
{
	بازگشت css_stylesheet_append_data(c->sheet,
			(ثابت uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(ساختار content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	اگر (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, پوچ);
		بازگشت false;
	}

	بازگشت true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
ایستا css_error nscss_convert_css_data(ساختار content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	اگر (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		أكد(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} وگرنه اگر (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} وگرنه {
		ثابت نویسه *url;

		اگر (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} وگرنه {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	بازگشت error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
تهی nscss_destroy(ساختار content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
ایستا تهی nscss_destroy_css_data(ساختار content_css_data *c)
{
	uint32_t i;

	برای (i = 0; i < c->import_شمار; i++) {
		اگر (c->imports[i].c != پوچ) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = پوچ;
	}

	آزاد(c->imports);

	اگر (c->sheet != پوچ) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = پوچ;
	}

	آزاد(c->charset);
}

nserror nscss_clone(ثابت ساختار content *old, ساختار content **newc)
{
	ثابت nscss_content *old_css = (ثابت nscss_content *) old;
	nscss_content *new_css;
	ثابت uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, اندازه(nscss_content));
	اگر (new_css == پوچ)
		بازگشت NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		بازگشت error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		بازگشت error;
	}

	data = content__get_source_data(&new_css->base, &size);
	اگر (size > 0) {
		اگر (nscss_process_data(&new_css->base,
				       (نویسه *)data,
				       (unsigned صحیح)size) == false) {
			content_destroy(&new_css->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	اگر (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		اگر (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *) new_css;

	بازگشت NSERROR_OK;
}

bool nscss_matches_quirks(ثابت ساختار content *c, bool quirks)
{
	بازگشت c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(ساختار hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);

	بازگشت c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
ساختار nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	أكد(c != پوچ);
	أكد(n != پوچ);

	*n = c->data.import_شمار;

	بازگشت c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(تهی)
{
	بازگشت CONTENT_CSS;
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
تهی nscss_content_done(ساختار content_css_data *css, تهی *pw)
{
	ساختار content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	اگر (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, پوچ);
		content_set_error(c);
		بازگشت;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	برای (i = 0; i < css->import_شمار; i++) {
		اگر (css->imports[i].c != پوچ) {
			ساختار content *import = hlcache_handle_get_content(
					css->imports[i].c);

			اگر (import != پوچ) {
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
css_error nscss_handle_import(تهی *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	ساختار content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	ساختار nscss_import *imports;
	ثابت نویسه *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	أكد(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	اگر (error != CSS_OK) {
		بازگشت error;
	}

	ctx = malloc(اندازه(*ctx));
	اگر (ctx == پوچ)
		بازگشت CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_شمار;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_شمار + 1) *
			اندازه(ساختار nscss_import));
	اگر (imports == پوچ) {
		آزاد(ctx);
		بازگشت CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = پوچ;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	اگر (error != CSS_OK) {
		آزاد(ctx);
		بازگشت error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	اگر (nerror != NSERROR_OK) {
		آزاد(ctx);
		بازگشت CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	اگر (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		آزاد(ctx);
		بازگشت CSS_NOMEM;
	}

	/* Avoid importing ourself */
	اگر (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_شمار].c = پوچ;
		/* No longer require context as we're not fetching anything */
		آزاد(ctx);
		ctx = پوچ;
	} وگرنه {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, پوچ, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_شمار].c);
		اگر (nerror != NSERROR_OK) {
			آزاد(ctx);
			بازگشت CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_شمار, lwc_string_data(url),
	      c->imports[c->import_شمار].c, ctx);
#endif

	c->import_شمار++;

	بازگشت CSS_OK;
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
		ثابت hlcache_event *event, تهی *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	أكد(ctx->css->imports[ctx->index].c == handle);

	انتخاب (event->type) {
	حالت CONTENT_پیام_DONE:
		error = nscss_import_complete(ctx);
		بشکن;

	حالت CONTENT_پیام_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = پوچ;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		بشکن;
	پیشفرض:
		بشکن;
	}

	/* Preserve out-of-memory. Anything else is OK */
	بازگشت error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	اگر (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	آزاد(ctx);

	بازگشت error;
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
css_error nscss_register_imports(ساختار content_css_data *c)
{
	uint32_t index;
	css_error error;

	أكد(c->next_to_register != (uint32_t) -1);
	أكد(c->next_to_register < c->import_شمار);

	/* Register imported sheets */
	برای (index = c->next_to_register; index < c->import_شمار; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		اگر (c->imports[index].c != پوچ &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			بشکن;
		}

		error = nscss_register_import(c, c->imports[index].c);
		اگر (error != CSS_OK)
			بازگشت error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	اگر (c->next_to_register == c->import_شمار) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	بازگشت CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(ساختار content_css_data *c,
		ثابت hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	اگر (import != پوچ) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} وگرنه {
		/* Create a blank sheet if needed. */
		اگر (blank_import == پوچ) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = پوچ;
			params.url = "";
			params.title = پوچ;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = پوچ;
			params.import = پوچ;
			params.import_pw = پوچ;
			params.color = ns_system_colour;
			params.color_pw = پوچ;
			params.font = پوچ;
			params.font_pw = پوچ;

			error = css_stylesheet_create(&params, &blank_import);
			اگر (error != CSS_OK) {
				بازگشت error;
			}

			error = css_stylesheet_data_done(blank_import);
			اگر (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				بازگشت error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	اگر (error != CSS_OK) {
		بازگشت error;
	}

	بازگشت error;
}

/**
 * Clean up after the CSS content handler
 */
ایستا تهی nscss_fini(تهی)
{
	اگر (blank_import != پوچ) {
		css_stylesheet_destroy(blank_import);
		blank_import = پوچ;
	}
	css_hint_fini();
}

ایستا ثابت content_handler css_content_handler = {
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
nserror nscss_init(تهی)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	اگر (error != NSERROR_OK)
		برو error;

	error = css_hint_init();
	اگر (error != NSERROR_OK)
		برو error;

	بازگشت NSERROR_OK;

error:
	nscss_fini();

	بازگشت error;
}
