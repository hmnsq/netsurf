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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "utils/http.h"

#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
തരനിർവചനം ഘടന content_handler_entry {
	/** Next entry */
	ഘടന content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	സ്ഥിരം content_handler *handler;
} content_handler_entry;

സ്ഥിതം content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
ശൂന്യം content_factory_fini(ശൂന്യം)
{
	content_handler_entry *victim;

	ആകുമ്പോൾ (content_handlers != ശൂന്യ) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		എങ്കിൽ (victim->handler->fini != ശൂന്യ)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		സ്വതന്ത്രം(victim);
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
nserror content_factory_register_handler(സ്ഥിരം അക്ഷരം *mime_type,
		സ്ഥിരം content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	എങ്കിൽ (lerror != lwc_error_ok)
		തിരികെ NSERROR_NOMEM;

	വേണ്ടി (entry = content_handlers; entry != ശൂന്യ; entry = entry->next) {
		എങ്കിൽ (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			നിർത്തുക;
	}

	എങ്കിൽ (entry == ശൂന്യ) {
		entry = malloc(വലുപ്പം(content_handler_entry));
		എങ്കിൽ (entry == ശൂന്യ)
			തിരികെ NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} അല്ലെങ്കിൽ {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	തിരികെ NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
സ്ഥിതം സ്ഥിരം content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	വേണ്ടി (entry = content_handlers; entry != ശൂന്യ; entry = entry->next) {
		എങ്കിൽ (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			നിർത്തുക;
		}
	}

	എങ്കിൽ (entry != ശൂന്യ) {
		തിരികെ entry->handler;
	}

	തിരികെ ശൂന്യ;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	സ്ഥിരം content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	എങ്കിൽ (handler != ശൂന്യ) {
		type = handler->type();
	}

	തിരികെ type;
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
ഘടന content *content_factory_create_content(llcache_handle *llcache,
		സ്ഥിരം അക്ഷരം *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ഘടന content *c;
	സ്ഥിരം അക്ഷരം *content_type_header;
	സ്ഥിരം content_handler *handler;
	http_content_type *ct = ശൂന്യ;
	nserror error;

	handler = content_lookup(effective_type);
	എങ്കിൽ (handler == ശൂന്യ)
		തിരികെ ശൂന്യ;

	പ്രതിജ്ഞാ(handler->create != ശൂന്യ);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	എങ്കിൽ (content_type_header != ശൂന്യ) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ശൂന്യ ? ct->parameters : ശൂന്യ, 
			llcache, fallback_charset, quirks, 
			&c);

	എങ്കിൽ (ct != ശൂന്യ)
		http_content_type_destroy(ct);

	എങ്കിൽ (error != NSERROR_OK)
		തിരികെ ശൂന്യ;

	തിരികെ c;
}

