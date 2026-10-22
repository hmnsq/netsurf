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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "utils/http.h"

#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ content_handler_ನಮೂದು {
	/** Next entry */
	ರಚನೆ content_handler_ನಮೂದು *ಮುಂದಿನ;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ಸ್ಥಿರಾಂಕ content_handler *handler;
} content_handler_ನಮೂದು;

ಸ್ಥಿರ content_handler_ನಮೂದು *content_handlers;

/**
 * Clean up after the content factory
 */
ಶೂನ್ಯ content_factory_fini(ಶೂನ್ಯ)
{
	content_handler_ನಮೂದು *victim;

	ಆಗ (content_handlers != ಖಾಲಿ) {
		victim = content_handlers;

		content_handlers = content_handlers->ಮುಂದಿನ;

		ಆದರೆ (victim->handler->fini != ಖಾಲಿ)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		ಬಿಡುಗಡೆ(victim);
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
nserror content_factory_ರಿಜಿಸ್ಟರ್_handler(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mime_type,
		ಸ್ಥಿರಾಂಕ content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_ನಮೂದು *ನಮೂದು;
	bool ಹೊಂದಿಕೆ;

	lerror = lwc_intern_string(mime_type, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(mime_type), &imime_type);
	ಆದರೆ (lerror != lwc_error_ok)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	ಗಾಗಿ (ನಮೂದು = content_handlers; ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		ಆದರೆ (lwc_string_caseless_isequal(imime_type, ನಮೂದು->mime_type,
				&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ (ನಮೂದು == ಖಾಲಿ) {
		ನಮೂದು = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(content_handler_ನಮೂದು));
		ಆದರೆ (ನಮೂದು == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

		ನಮೂದು->ಮುಂದಿನ = content_handlers;
		content_handlers = ನಮೂದು;

		ನಮೂದು->mime_type = imime_type;
	} ಇಲ್ಲದಿದ್ದರೆ {
		lwc_string_unref(imime_type);
	}

	ನಮೂದು->handler = handler;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_ನಮೂದು *ನಮೂದು;
	bool ಹೊಂದಿಕೆ;

	ಗಾಗಿ (ನಮೂದು = content_handlers; ನಮೂದು != ಖಾಲಿ; ನಮೂದು = ನಮೂದು->ಮುಂದಿನ) {
		ಆದರೆ (lwc_string_caseless_isequal(mime_type, ನಮೂದು->mime_type,
					&ಹೊಂದಿಕೆ) == lwc_error_ok && ಹೊಂದಿಕೆ) {
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (ನಮೂದು != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ನಮೂದು->handler;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ಸ್ಥಿರಾಂಕ content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	ಆದರೆ (handler != ಖಾಲಿ) {
		type = handler->type();
	}

	ಹಿಂತಿರುಗಿ type;
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
ರಚನೆ content *content_factory_create_content(llcache_handle *llcache,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ರಚನೆ content *c;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_type_ಶೀರ್ಷಿಕೆ;
	ಸ್ಥಿರಾಂಕ content_handler *handler;
	http_content_type *ct = ಖಾಲಿ;
	nserror error;

	handler = content_lookup(effective_type);
	ಆದರೆ (handler == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಪ್ರತಿಜ್ಞಾ(handler->create != ಖಾಲಿ);

	/* Use the parameters from the declared Content-Type header */
	content_type_ಶೀರ್ಷಿಕೆ = 
			llcache_handle_get_ಶೀರ್ಷಿಕೆ(llcache, "Content-Type");
	ಆದರೆ (content_type_ಶೀರ್ಷಿಕೆ != ಖಾಲಿ) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_ಶೀರ್ಷಿಕೆ, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ಖಾಲಿ ? ct->parameters : ಖಾಲಿ, 
			llcache, fallback_charset, quirks, 
			&c);

	ಆದರೆ (ct != ಖಾಲಿ)
		http_content_type_destroy(ct);

	ಆದರೆ (error != NSERROR_OK)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ c;
}

