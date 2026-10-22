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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "utils/http.h"

#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
પ્રકારવ્યાખ્યા સંરચના content_handler_entry {
	/** Next entry */
	સંરચના content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	અચલ content_handler *handler;
} content_handler_entry;

સ્થિર content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
શૂન્ય content_factory_fini(શૂન્ય)
{
	content_handler_entry *victim;

	જ્યારે (content_handlers != ખાલી) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		જો (victim->handler->fini != ખાલી)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		મુક્ત(victim);
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
nserror content_factory_register_handler(અચલ અક્ષર *mime_type,
		અચલ content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	જો (lerror != lwc_error_ok)
		પાછા NSERROR_NOMEM;

	માટે (entry = content_handlers; entry != ખાલી; entry = entry->next) {
		જો (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			તોડો;
	}

	જો (entry == ખાલી) {
		entry = malloc(કદ(content_handler_entry));
		જો (entry == ખાલી)
			પાછા NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} નહીંતર {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	પાછા NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
સ્થિર અચલ content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	માટે (entry = content_handlers; entry != ખાલી; entry = entry->next) {
		જો (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			તોડો;
		}
	}

	જો (entry != ખાલી) {
		પાછા entry->handler;
	}

	પાછા ખાલી;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	અચલ content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	જો (handler != ખાલી) {
		type = handler->type();
	}

	પાછા type;
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
સંરચના content *content_factory_create_content(llcache_handle *llcache,
		અચલ અક્ષર *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	સંરચના content *c;
	અચલ અક્ષર *content_type_header;
	અચલ content_handler *handler;
	http_content_type *ct = ખાલી;
	nserror error;

	handler = content_lookup(effective_type);
	જો (handler == ખાલી)
		પાછા ખાલી;

	પ્રતિજ્ઞા(handler->create != ખાલી);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	જો (content_type_header != ખાલી) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ખાલી ? ct->parameters : ખાલી, 
			llcache, fallback_charset, quirks, 
			&c);

	જો (ct != ખાલી)
		http_content_type_destroy(ct);

	જો (error != NSERROR_OK)
		પાછા ખાલી;

	પાછા c;
}

