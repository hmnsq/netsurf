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

#شامل <assert.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "utils/http.h"

#شامل "content/content.h"
#شامل "content/content_factory.h"
#شامل "content/content_protected.h"
#شامل "content/llcache.h"

/**
 * Entry in list of content handlers
 */
قسمتعریف ساخت content_handler_اندراج {
	/** Next entry */
	ساخت content_handler_اندراج *اگلا;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ثابت content_handler *handler;
} content_handler_اندراج;

ساکن content_handler_اندراج *content_handlers;

/**
 * Clean up after the content factory
 */
خالی content_factory_fini(خالی)
{
	content_handler_اندراج *victim;

	جب (content_handlers != تہی) {
		victim = content_handlers;

		content_handlers = content_handlers->اگلا;

		اگر (victim->handler->fini != تہی)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		آزاد(victim);
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
nserror content_factory_رجسٹر_handler(ثابت حرف *mime_type,
		ثابت content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_اندراج *اندراج_2;
	bool مطابقت;

	lerror = lwc_intern_string(mime_type, لڑی_لمبائی(mime_type), &imime_type);
	اگر (lerror != lwc_error_ok)
		واپس NSERROR_NOMEM;

	برائے (اندراج_2 = content_handlers; اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
		اگر (lwc_string_caseless_isequal(imime_type, اندراج_2->mime_type,
				&مطابقت) == lwc_error_ok && مطابقت)
			توڑو;
	}

	اگر (اندراج_2 == تہی) {
		اندراج_2 = حافظہ_دو(حجم(content_handler_اندراج));
		اگر (اندراج_2 == تہی)
			واپس NSERROR_NOMEM;

		اندراج_2->اگلا = content_handlers;
		content_handlers = اندراج_2;

		اندراج_2->mime_type = imime_type;
	} ورنہ {
		lwc_string_unref(imime_type);
	}

	اندراج_2->handler = handler;

	واپس NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ساکن ثابت content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_اندراج *اندراج_2;
	bool مطابقت;

	برائے (اندراج_2 = content_handlers; اندراج_2 != تہی; اندراج_2 = اندراج_2->اگلا) {
		اگر (lwc_string_caseless_isequal(mime_type, اندراج_2->mime_type,
					&مطابقت) == lwc_error_ok && مطابقت) {
			توڑو;
		}
	}

	اگر (اندراج_2 != تہی) {
		واپس اندراج_2->handler;
	}

	واپس تہی;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ثابت content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	اگر (handler != تہی) {
		type = handler->type();
	}

	واپس type;
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
ساخت content *content_factory_create_content(llcache_handle *llcache,
		ثابت حرف *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ساخت content *c;
	ثابت حرف *content_type_سرخی;
	ثابت content_handler *handler;
	http_content_type *ct = تہی;
	nserror error;

	handler = content_lookup(effective_type);
	اگر (handler == تہی)
		واپس تہی;

	أكد(handler->create != تہی);

	/* Use the parameters from the declared Content-Type header */
	content_type_سرخی = 
			llcache_handle_get_سرخی(llcache, "Content-Type");
	اگر (content_type_سرخی != تہی) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_سرخی, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != تہی ? ct->parameters : تہی, 
			llcache, fallback_charset, quirks, 
			&c);

	اگر (ct != تہی)
		http_content_type_destroy(ct);

	اگر (error != NSERROR_OK)
		واپس تہی;

	واپس c;
}

