/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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
 * Content factory implementation
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/http.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ content_handler_ਇੰਦਰਾਜ {
	/** Next entry */
	ਬਣਤਰ content_handler_ਇੰਦਰਾਜ *ਅਗਲਾ;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ਅਚਲ content_handler *handler;
} content_handler_ਇੰਦਰਾਜ;

ਸਥਿਰ content_handler_ਇੰਦਰਾਜ *content_handlers;

/**
 * Clean up after the content factory
 */
ਖਾਲੀ content_factory_fini(ਖਾਲੀ)
{
	content_handler_ਇੰਦਰਾਜ *victim;

	ਜਦੋਂ (content_handlers != ਨਿਰਾ) {
		victim = content_handlers;

		content_handlers = content_handlers->ਅਗਲਾ;

		ਜੇ (victim->handler->fini != ਨਿਰਾ)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		ਮੁਕਤ(victim);
	}
}

/**
 * Register a handler with the content factory
 *
 * \param mime_type  MIME type to handle
 * \param handler    Content handler for MIME type
 * \return NSERROR_OK on success, appropriate error otherwise
 *
 * \note Latest registration for a MIME type wins
 */
nserror content_factory_ਰਜਿਸਟਰ_handler(ਅਚਲ ਅੱਖਰ *mime_type,
		ਅਚਲ content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
	bool ਮੇਲ;

	lerror = lwc_intern_string(mime_type, ਲੜੀ_ਲੰਬਾਈ(mime_type), &imime_type);
	ਜੇ (lerror != lwc_error_ok)
		ਵਾਪਸ NSERROR_NOMEM;

	ਲਈ (ਇੰਦਰਾਜ = content_handlers; ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		ਜੇ (lwc_string_caseless_isequal(imime_type, ਇੰਦਰਾਜ->mime_type,
				&ਮੇਲ) == lwc_error_ok && ਮੇਲ)
			ਰੋਕੋ;
	}

	ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ) {
		ਇੰਦਰਾਜ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(content_handler_ਇੰਦਰਾਜ));
		ਜੇ (ਇੰਦਰਾਜ == ਨਿਰਾ)
			ਵਾਪਸ NSERROR_NOMEM;

		ਇੰਦਰਾਜ->ਅਗਲਾ = content_handlers;
		content_handlers = ਇੰਦਰਾਜ;

		ਇੰਦਰਾਜ->mime_type = imime_type;
	} ਨਹੀਂਤਾਂ {
		lwc_string_unref(imime_type);
	}

	ਇੰਦਰਾਜ->handler = handler;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ਸਥਿਰ ਅਚਲ content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ;
	bool ਮੇਲ;

	ਲਈ (ਇੰਦਰਾਜ = content_handlers; ਇੰਦਰਾਜ != ਨਿਰਾ; ਇੰਦਰਾਜ = ਇੰਦਰਾਜ->ਅਗਲਾ) {
		ਜੇ (lwc_string_caseless_isequal(mime_type, ਇੰਦਰਾਜ->mime_type,
					&ਮੇਲ) == lwc_error_ok && ਮੇਲ) {
			ਰੋਕੋ;
		}
	}

	ਜੇ (ਇੰਦਰਾਜ != ਨਿਰਾ) {
		ਵਾਪਸ ਇੰਦਰਾਜ->handler;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ਅਚਲ content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	ਜੇ (handler != ਨਿਰਾ) {
		type = handler->type();
	}

	ਵਾਪਸ type;
}

/**
 * Create a content object
 *
 * \param llcache           Underlying source data handle
 * \param fallback_charset  Character set to fall back to if none specified
 * \param quirks            Quirkiness of containing document
 * \param effective_type    Effective MIME type of content
 * \return Pointer to content object, or NULL on failure
 */
ਬਣਤਰ content *content_factory_create_content(llcache_handle *llcache,
		ਅਚਲ ਅੱਖਰ *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ਬਣਤਰ content *c;
	ਅਚਲ ਅੱਖਰ *content_type_ਸਿਰਲੇਖ;
	ਅਚਲ content_handler *handler;
	http_content_type *ct = ਨਿਰਾ;
	nserror error;

	handler = content_lookup(effective_type);
	ਜੇ (handler == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(handler->create != ਨਿਰਾ);

	/* Use the parameters from the declared Content-Type header */
	content_type_ਸਿਰਲੇਖ = 
			llcache_handle_get_ਸਿਰਲੇਖ(llcache, "Content-Type");
	ਜੇ (content_type_ਸਿਰਲੇਖ != ਨਿਰਾ) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_ਸਿਰਲੇਖ, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ਨਿਰਾ ? ct->parameters : ਨਿਰਾ, 
			llcache, fallback_charset, quirks, 
			&c);

	ਜੇ (ct != ਨਿਰਾ)
		http_content_type_destroy(ct);

	ਜੇ (error != NSERROR_OK)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ c;
}

