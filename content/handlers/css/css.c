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

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <libwapcaplet/libwapcaplet.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/http.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/fetch.h"
#અન્તર્ભાવ "content/hlcache.h"
#અન્તર્ભાવ "desktop/system_colour.h"

#અન્તર્ભાવ "css/css.h"
#અન્તર્ભાવ "css/hints.h"
#અન્તર્ભાવ "css/internal.h"

/* Define to trace import fetches */
#undef NSCSS_IMPORT_TRACE

/** Screen DPI in fixed point units: defaults to 90, which RISC OS uses */
css_fixed nscss_screen_dpi = F_90;

સંરચના content_css_data;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
પ્રકારવ્યાખ્યા શૂન્ય (*nscss_done_callback)(સંરચના content_css_data *css, શૂન્ય *pw);

/**
 * CSS content data
 */
સંરચના content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	અક્ષર *charset;			/**< Character set of stylesheet */
	સંરચના nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_ગણતરી;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	શૂન્ય *pw;			/**< Client data */
};

/**
 * CSS content data
 */
પ્રકારવ્યાખ્યા સંરચના nscss_content
{
	સંરચના content base;		/**< Underlying content object */

	સંરચના content_css_data data;	/**< CSS data */
} nscss_content;

/**
 * Context for import fetches
 */
પ્રકારવ્યાખ્યા સંરચના {
	સંરચના content_css_data *css;		/**< Object containing import */
	uint32_t index;				/**< Index into parent sheet's
						 *   imports array */
} nscss_import_ctx;

સ્થિર bool nscss_convert(સંરચના content *c);
સ્થિર શૂન્ય nscss_destroy(સંરચના content *c);
સ્થિર nserror nscss_clone(અચલ સંરચના content *old, સંરચના content **newc);
સ્થિર bool nscss_matches_quirks(અચલ સંરચના content *c, bool quirks);
સ્થિર content_type nscss_content_type(શૂન્ય);

સ્થિર nserror nscss_create_css_data(સંરચના content_css_data *c,
		અચલ અક્ષર *url, અચલ અક્ષર *charset, bool quirks,
		nscss_done_callback done, શૂન્ય *pw);
સ્થિર css_error nscss_process_css_data(સંરચના content_css_data *c, અચલ અક્ષર *data,
		unsigned પૂર્ણાંક size);
સ્થિર css_error nscss_convert_css_data(સંરચના content_css_data *c);
સ્થિર શૂન્ય nscss_destroy_css_data(સંરચના content_css_data *c);

સ્થિર શૂન્ય nscss_content_done(સંરચના content_css_data *css, શૂન્ય *pw);
સ્થિર css_error nscss_handle_import(શૂન્ય *pw, css_stylesheet *parent,
		lwc_string *url);
સ્થિર nserror nscss_import(hlcache_handle *handle,
		અચલ hlcache_event *event, શૂન્ય *pw);
સ્થિર css_error nscss_import_complete(nscss_import_ctx *ctx);

સ્થિર css_error nscss_register_imports(સંરચના content_css_data *c);
સ્થિર css_error nscss_register_import(સંરચના content_css_data *c,
		અચલ hlcache_handle *import);


સ્થિર css_stylesheet *blank_import;


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
સ્થિર nserror
nscss_create(અચલ content_handler *handler,
	     lwc_string *imime_type,
	     અચલ http_parameter *params,
	     llcache_handle *llcache,
	     અચલ અક્ષર *fallback_charset,
	     bool quirks,
	     સંરચના content **c)
{
	nscss_content *result;
	અચલ અક્ષર *charset = ખાલી;
	અચલ અક્ષર *xnsbase = ખાલી;
	lwc_string *charset_કિંમત = ખાલી;
	nserror error;

	result = calloc(1, કદ(nscss_content));
	જો (result == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type,
			params, llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(result);
		પાછા error;
	}

	/* Find charset specified on HTTP layer, if any */
	error = http_parameter_list_find_item(params, corestring_lwc_charset,
			&charset_કિંમત);
	જો (error != NSERROR_OK || lwc_string_length(charset_કિંમત) == 0) {
		/* No charset specified, use fallback, if any */
		/** \todo libcss will take this as gospel, which is wrong */
		charset = fallback_charset;
	} નહીંતર {
		charset = lwc_string_data(charset_કિંમત);
	}

	/* Compute base URL for stylesheet */
	xnsbase = llcache_handle_get_header(llcache, "X-NS-Base");
	જો (xnsbase == ખાલી) {
		xnsbase = nsurl_access(content_get_url(&result->base));
	}

	error = nscss_create_css_data(&result->data,
			xnsbase, charset, result->base.quirks,
			nscss_content_done, result);
	જો (error != NSERROR_OK) {
		content_broadcast_error(&result->base, NSERROR_NOMEM, ખાલી);
		lwc_string_unref(charset_કિંમત);
		મુક્ત(result);
		પાછા error;
	}

	lwc_string_unref(charset_કિંમત);

	*c = (સંરચના content *) result;

	પાછા NSERROR_OK;
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
સ્થિર nserror nscss_create_css_data(સંરચના content_css_data *c,
		અચલ અક્ષર *url, અચલ અક્ષર *charset, bool quirks,
		nscss_done_callback done, શૂન્ય *pw)
{
	css_error error;
	css_stylesheet_params params;

	c->pw = pw;
	c->done = done;
	c->next_to_register = (uint32_t) -1;
	c->import_ગણતરી = 0;
	c->imports = ખાલી;
	જો (charset != ખાલી)
		c->charset = strdup(charset);
	નહીંતર
		c->charset = ખાલી;

	params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
	params.level = CSS_LEVEL_DEFAULT;
	params.charset = charset;
	params.url = url;
	params.title = ખાલી;
	params.allow_quirks = quirks;
	params.inline_style = false;
	params.resolve = nscss_resolve_url;
	params.resolve_pw = ખાલી;
	params.import = nscss_handle_import;
	params.import_pw = c;
	params.color = ns_system_colour;
	params.color_pw = ખાલી;
	params.font = ખાલી;
	params.font_pw = ખાલી;

	error = css_stylesheet_create(&params, &c->sheet);
	જો (error != CSS_OK) {
		પાછા NSERROR_NOMEM;
	}

	પાછા NSERROR_OK;
}

/**
 * Process CSS source data
 *
 * \param c     Content structure
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return true on success, false on failure
 */
સ્થિર bool
nscss_process_data(સંરચના content *c, અચલ અક્ષર *data, unsigned પૂર્ણાંક size)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_process_css_data(&css->data, data, size);
	જો (error != CSS_OK && error != CSS_NEEDDATA) {
		content_broadcast_error(c, NSERROR_CSS, ખાલી);
	}

	પાછા (error == CSS_OK || error == CSS_NEEDDATA);
}

/**
 * Process CSS data
 *
 * \param c     CSS content object
 * \param data  Data to process
 * \param size  Number of bytes to process
 * \return CSS_OK on success, appropriate error otherwise
 */
સ્થિર css_error nscss_process_css_data(સંરચના content_css_data *c,
		અચલ અક્ષર *data, unsigned પૂર્ણાંક size)
{
	પાછા css_stylesheet_append_data(c->sheet,
			(અચલ uint8_t *) data, size);
}

/**
 * Convert a CSS content ready for use
 *
 * \param c  Content to convert
 * \return true on success, false on failure
 */
bool nscss_convert(સંરચના content *c)
{
	nscss_content *css = (nscss_content *) c;
	css_error error;

	error = nscss_convert_css_data(&css->data);
	જો (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ખાલી);
		પાછા false;
	}

	પાછા true;
}

/**
 * Convert CSS data ready for use
 *
 * \param c  CSS data to convert
 * \return CSS error
 */
સ્થિર css_error nscss_convert_css_data(સંરચના content_css_data *c)
{
	css_error error;

	error = css_stylesheet_data_done(c->sheet);

	/* Process pending imports */
	જો (error == CSS_IMPORTS_PENDING) {
		/* We must not have registered any imports yet */
		પ્રતિજ્ઞા(c->next_to_register == (uint32_t) -1);

		/* Start registering, until we find one that
		 * hasn't finished fetching */
		c->next_to_register = 0;
		error = nscss_register_imports(c);
	} નહીંતર જો (error == CSS_OK) {
		/* No imports, and no errors, so complete conversion */
		c->done(c, c->pw);
	} નહીંતર {
		અચલ અક્ષર *url;

		જો (css_stylesheet_get_url(c->sheet, &url) == CSS_OK) {
			NSLOG(netsurf, INFO, "Failed converting %p %s (%d)",
			      c, url, error);
		} નહીંતર {
			NSLOG(netsurf, INFO, "Failed converting %p (%d)", c,
			      error);
		}
	}

	પાછા error;
}

/**
 * Clean up a CSS content
 *
 * \param c  Content to clean up
 */
શૂન્ય nscss_destroy(સંરચના content *c)
{
	nscss_content *css = (nscss_content *) c;

	nscss_destroy_css_data(&css->data);
}

/**
 * Clean up CSS data
 *
 * \param c  CSS data to clean up
 */
સ્થિર શૂન્ય nscss_destroy_css_data(સંરચના content_css_data *c)
{
	uint32_t i;

	માટે (i = 0; i < c->import_ગણતરી; i++) {
		જો (c->imports[i].c != ખાલી) {
			hlcache_handle_release(c->imports[i].c);
		}
		c->imports[i].c = ખાલી;
	}

	મુક્ત(c->imports);

	જો (c->sheet != ખાલી) {
		css_stylesheet_destroy(c->sheet);
		c->sheet = ખાલી;
	}

	મુક્ત(c->charset);
}

nserror nscss_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	અચલ nscss_content *old_css = (અચલ nscss_content *) old;
	nscss_content *new_css;
	અચલ uint8_t *data;
	size_t size;
	nserror error;

	new_css = calloc(1, કદ(nscss_content));
	જો (new_css == ખાલી)
		પાછા NSERROR_NOMEM;

	/* Clone content */
	error = content__clone(old, &new_css->base);
	જો (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		પાછા error;
	}

	/* Simply replay create/process/convert */
	error = nscss_create_css_data(&new_css->data,
			nsurl_access(content_get_url(&new_css->base)),
			old_css->data.charset,
			new_css->base.quirks,
			nscss_content_done, new_css);
	જો (error != NSERROR_OK) {
		content_destroy(&new_css->base);
		પાછા error;
	}

	data = content__get_source_data(&new_css->base, &size);
	જો (size > 0) {
		જો (nscss_process_data(&new_css->base,
				       (અક્ષર *)data,
				       (unsigned પૂર્ણાંક)size) == false) {
			content_destroy(&new_css->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (nscss_convert(&new_css->base) == false) {
			content_destroy(&new_css->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) new_css;

	પાછા NSERROR_OK;
}

bool nscss_matches_quirks(અચલ સંરચના content *c, bool quirks)
{
	પાછા c->quirks == quirks;
}

/* exported interface documented in netsurf/css.h */
css_stylesheet *nscss_get_stylesheet(સંરચના hlcache_handle *h)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);

	પાછા c->data.sheet;
}

/* exported interface documented in netsurf/css.h */
સંરચના nscss_import *nscss_get_imports(hlcache_handle *h, uint32_t *n)
{
	nscss_content *c = (nscss_content *) hlcache_handle_get_content(h);

	પ્રતિજ્ઞા(c != ખાલી);
	પ્રતિજ્ઞા(n != ખાલી);

	*n = c->data.import_ગણતરી;

	પાછા c->data.imports;
}

/**
 * Compute the type of a content
 *
 * \return CONTENT_CSS
 */
content_type nscss_content_type(શૂન્ય)
{
	પાછા CONTENT_CSS;
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
શૂન્ય nscss_content_done(સંરચના content_css_data *css, શૂન્ય *pw)
{
	સંરચના content *c = pw;
	uint32_t i;
	size_t size;
	css_error error;

	/* Retrieve the size of this sheet */
	error = css_stylesheet_size(css->sheet, &size);
	જો (error != CSS_OK) {
		content_broadcast_error(c, NSERROR_CSS, ખાલી);
		content_set_error(c);
		પાછા;
	}
	c->size += size;

	/* Add on the size of the imported sheets */
	માટે (i = 0; i < css->import_ગણતરી; i++) {
		જો (css->imports[i].c != ખાલી) {
			સંરચના content *import = hlcache_handle_get_content(
					css->imports[i].c);

			જો (import != ખાલી) {
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
css_error nscss_handle_import(શૂન્ય *pw, css_stylesheet *parent,
		lwc_string *url)
{
	content_type accept = CONTENT_CSS;
	સંરચના content_css_data *c = pw;
	nscss_import_ctx *ctx;
	hlcache_child_context child;
	સંરચના nscss_import *imports;
	અચલ અક્ષર *referer;
	css_error error;
	nserror nerror;

	nsurl *ns_url;
	nsurl *ns_ref;

	પ્રતિજ્ઞા(parent == c->sheet);

	error = css_stylesheet_get_url(c->sheet, &referer);
	જો (error != CSS_OK) {
		પાછા error;
	}

	ctx = malloc(કદ(*ctx));
	જો (ctx == ખાલી)
		પાછા CSS_NOMEM;

	ctx->css = c;
	ctx->index = c->import_ગણતરી;

	/* Increase space in table */
	imports = realloc(c->imports, (c->import_ગણતરી + 1) *
			કદ(સંરચના nscss_import));
	જો (imports == ખાલી) {
		મુક્ત(ctx);
		પાછા CSS_NOMEM;
	}
	c->imports = imports;

	/** \todo fallback charset */
	child.charset = ખાલી;
	error = css_stylesheet_quirks_allowed(c->sheet, &child.quirks);
	જો (error != CSS_OK) {
		મુક્ત(ctx);
		પાછા error;
	}

	/* Create content */

	/** \todo Why aren't we getting a relative url part, to join? */
	nerror = nsurl_create(lwc_string_data(url), &ns_url);
	જો (nerror != NSERROR_OK) {
		મુક્ત(ctx);
		પાછા CSS_NOMEM;
	}

	/** \todo Constructing nsurl for referer here is silly, avoid */
	nerror = nsurl_create(referer, &ns_ref);
	જો (nerror != NSERROR_OK) {
		nsurl_unref(ns_url);
		મુક્ત(ctx);
		પાછા CSS_NOMEM;
	}

	/* Avoid importing ourself */
	જો (nsurl_compare(ns_url, ns_ref, NSURL_COMPLETE)) {
		c->imports[c->import_ગણતરી].c = ખાલી;
		/* No longer require context as we're not fetching anything */
		મુક્ત(ctx);
		ctx = ખાલી;
	} નહીંતર {
		nerror = hlcache_handle_retrieve(ns_url,
				0, ns_ref, ખાલી, nscss_import, ctx,
				&child, accept,
				&c->imports[c->import_ગણતરી].c);
		જો (nerror != NSERROR_OK) {
			મુક્ત(ctx);
			પાછા CSS_NOMEM;
		}
	}

	nsurl_unref(ns_url);
	nsurl_unref(ns_ref);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Import %d '%s' -> (handle: %p ctx: %p)",
	      c->import_ગણતરી, lwc_string_data(url),
	      c->imports[c->import_ગણતરી].c, ctx);
#endif

	c->import_ગણતરી++;

	પાછા CSS_OK;
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
		અચલ hlcache_event *event, શૂન્ય *pw)
{
	nscss_import_ctx *ctx = pw;
	css_error error = CSS_OK;

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Event %d for %p (%p)", event->type, handle, ctx);
#endif

	પ્રતિજ્ઞા(ctx->css->imports[ctx->index].c == handle);

	પસંદગી (event->type) {
	સ્થિતિ CONTENT_સંદેશ_DONE:
		error = nscss_import_complete(ctx);
		તોડો;

	સ્થિતિ CONTENT_સંદેશ_ERROR:
		hlcache_handle_release(handle);
		ctx->css->imports[ctx->index].c = ખાલી;

		error = nscss_import_complete(ctx);
		/* Already released handle */
		તોડો;
	મૂળભૂત:
		તોડો;
	}

	/* Preserve out-of-memory. Anything else is OK */
	પાછા error == CSS_NOMEM ? NSERROR_NOMEM : NSERROR_OK;
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
	જો (ctx->css->next_to_register == ctx->index)
		error = nscss_register_imports(ctx->css);

#ifdef NSCSS_IMPORT_TRACE
	NSLOG(netsurf, INFO, "Destroying import context %p for %d", ctx,
	      ctx->index);
#endif

	/* No longer need import context */
	મુક્ત(ctx);

	પાછા error;
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
css_error nscss_register_imports(સંરચના content_css_data *c)
{
	uint32_t index;
	css_error error;

	પ્રતિજ્ઞા(c->next_to_register != (uint32_t) -1);
	પ્રતિજ્ઞા(c->next_to_register < c->import_ગણતરી);

	/* Register imported sheets */
	માટે (index = c->next_to_register; index < c->import_ગણતરી; index++) {
		/* Stop registering if we encounter one whose fetch hasn't
		 * completed yet. We'll resume at this point when it has
		 * completed.
		 */
		જો (c->imports[index].c != ખાલી &&
			content_get_status(c->imports[index].c) !=
				CONTENT_STATUS_DONE) {
			તોડો;
		}

		error = nscss_register_import(c, c->imports[index].c);
		જો (error != CSS_OK)
			પાછા error;
	}

	/* Record identity of the next import to register */
	c->next_to_register = (uint32_t) index;

	જો (c->next_to_register == c->import_ગણતરી) {
		/* No more imports: notify parent that we're DONE */
		c->done(c, c->pw);
	}

	પાછા CSS_OK;
}


/**
 * Register an import with a stylesheet
 *
 * \param c       CSS object that requested the import
 * \param import  Cache handle of import, or NULL for blank
 * \return CSS_OK on success, appropriate error otherwise
 */
css_error nscss_register_import(સંરચના content_css_data *c,
		અચલ hlcache_handle *import)
{
	css_stylesheet *sheet;
	css_error error;

	જો (import != ખાલી) {
		nscss_content *s =
			(nscss_content *) hlcache_handle_get_content(import);
		sheet = s->data.sheet;
	} નહીંતર {
		/* Create a blank sheet if needed. */
		જો (blank_import == ખાલી) {
			css_stylesheet_params params;

			params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
			params.level = CSS_LEVEL_DEFAULT;
			params.charset = ખાલી;
			params.url = "";
			params.title = ખાલી;
			params.allow_quirks = false;
			params.inline_style = false;
			params.resolve = nscss_resolve_url;
			params.resolve_pw = ખાલી;
			params.import = ખાલી;
			params.import_pw = ખાલી;
			params.color = ns_system_colour;
			params.color_pw = ખાલી;
			params.font = ખાલી;
			params.font_pw = ખાલી;

			error = css_stylesheet_create(&params, &blank_import);
			જો (error != CSS_OK) {
				પાછા error;
			}

			error = css_stylesheet_data_done(blank_import);
			જો (error != CSS_OK) {
				css_stylesheet_destroy(blank_import);
				પાછા error;
			}
		}

		sheet = blank_import;
	}

	error = css_stylesheet_register_import(c->sheet, sheet);
	જો (error != CSS_OK) {
		પાછા error;
	}

	પાછા error;
}

/**
 * Clean up after the CSS content handler
 */
સ્થિર શૂન્ય nscss_fini(શૂન્ય)
{
	જો (blank_import != ખાલી) {
		css_stylesheet_destroy(blank_import);
		blank_import = ખાલી;
	}
	css_hint_fini();
}

સ્થિર અચલ content_handler css_content_handler = {
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
nserror nscss_init(શૂન્ય)
{
	nserror error;

	error = content_factory_register_handler("text/css",
			&css_content_handler);
	જો (error != NSERROR_OK)
		જાઓ error;

	error = css_hint_init();
	જો (error != NSERROR_OK)
		જાઓ error;

	પાછા NSERROR_OK;

error:
	nscss_fini();

	પાછા error;
}
