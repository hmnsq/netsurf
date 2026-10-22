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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "utils/http.h"

#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
வகைவரை கட்டமைப்பு content_handler_entry {
	/** Next entry */
	கட்டமைப்பு content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	மாறிலி content_handler *handler;
} content_handler_entry;

நிலையான content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
வெறுமை content_factory_fini(வெறுமை)
{
	content_handler_entry *victim;

	போது (content_handlers != வெற்று) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		எனில் (victim->handler->fini != வெற்று)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		விடுவி(victim);
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
nserror content_factory_register_handler(மாறிலி எழுத்து *mime_type,
		மாறிலி content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	எனில் (lerror != lwc_error_ok)
		திரும்பு NSERROR_NOMEM;

	க்கு (entry = content_handlers; entry != வெற்று; entry = entry->next) {
		எனில் (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			நிறுத்து;
	}

	எனில் (entry == வெற்று) {
		entry = malloc(அளவு(content_handler_entry));
		எனில் (entry == வெற்று)
			திரும்பு NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} இல்லையெனில் {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	திரும்பு NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
நிலையான மாறிலி content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	க்கு (entry = content_handlers; entry != வெற்று; entry = entry->next) {
		எனில் (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			நிறுத்து;
		}
	}

	எனில் (entry != வெற்று) {
		திரும்பு entry->handler;
	}

	திரும்பு வெற்று;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	மாறிலி content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	எனில் (handler != வெற்று) {
		type = handler->type();
	}

	திரும்பு type;
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
கட்டமைப்பு content *content_factory_create_content(llcache_handle *llcache,
		மாறிலி எழுத்து *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	கட்டமைப்பு content *c;
	மாறிலி எழுத்து *content_type_header;
	மாறிலி content_handler *handler;
	http_content_type *ct = வெற்று;
	nserror error;

	handler = content_lookup(effective_type);
	எனில் (handler == வெற்று)
		திரும்பு வெற்று;

	ப்ரதிஜ்ஞா(handler->create != வெற்று);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	எனில் (content_type_header != வெற்று) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != வெற்று ? ct->parameters : வெற்று, 
			llcache, fallback_charset, quirks, 
			&c);

	எனில் (ct != வெற்று)
		http_content_type_destroy(ct);

	எனில் (error != NSERROR_OK)
		திரும்பு வெற்று;

	திரும்பு c;
}

