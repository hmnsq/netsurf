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
تعریفنوع ساختار content_handler_entry {
	/** Next entry */
	ساختار content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ثابت content_handler *handler;
} content_handler_entry;

ایستا content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
تهی content_factory_fini(تهی)
{
	content_handler_entry *victim;

	وقتی (content_handlers != پوچ) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		اگر (victim->handler->fini != پوچ)
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
nserror content_factory_register_handler(ثابت نویسه *mime_type,
		ثابت content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	اگر (lerror != lwc_error_ok)
		بازگشت NSERROR_NOMEM;

	برای (entry = content_handlers; entry != پوچ; entry = entry->next) {
		اگر (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			بشکن;
	}

	اگر (entry == پوچ) {
		entry = malloc(اندازه(content_handler_entry));
		اگر (entry == پوچ)
			بازگشت NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} وگرنه {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	بازگشت NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ایستا ثابت content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	برای (entry = content_handlers; entry != پوچ; entry = entry->next) {
		اگر (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			بشکن;
		}
	}

	اگر (entry != پوچ) {
		بازگشت entry->handler;
	}

	بازگشت پوچ;
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
	اگر (handler != پوچ) {
		type = handler->type();
	}

	بازگشت type;
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
ساختار content *content_factory_create_content(llcache_handle *llcache,
		ثابت نویسه *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ساختار content *c;
	ثابت نویسه *content_type_header;
	ثابت content_handler *handler;
	http_content_type *ct = پوچ;
	nserror error;

	handler = content_lookup(effective_type);
	اگر (handler == پوچ)
		بازگشت پوچ;

	أكد(handler->create != پوچ);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	اگر (content_type_header != پوچ) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != پوچ ? ct->parameters : پوچ, 
			llcache, fallback_charset, quirks, 
			&c);

	اگر (ct != پوچ)
		http_content_type_destroy(ct);

	اگر (error != NSERROR_OK)
		بازگشت پوچ;

	بازگشت c;
}

