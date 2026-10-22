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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "utils/http.h"

#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/llcache.h"

/**
 * Entry in list of content handlers
 */
प्रकारनिर्देश संरचना content_handler_entry {
	/** Next entry */
	संरचना content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ध्रुव content_handler *handler;
} content_handler_entry;

स्थिर content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
शून्य content_factory_fini(शून्य)
{
	content_handler_entry *victim;

	यावत् (content_handlers != रिक्त) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		यदि (victim->handler->fini != रिक्त)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		मुक्त(victim);
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
nserror content_factory_register_handler(ध्रुव वर्ण *mime_type,
		ध्रुव content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, म_माप(mime_type), &imime_type);
	यदि (lerror != lwc_error_ok)
		प्रत्यागम NSERROR_NOMEM;

	प्रति (entry = content_handlers; entry != रिक्त; entry = entry->next) {
		यदि (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			भङ्ग;
	}

	यदि (entry == रिक्त) {
		entry = दो_स्मृति(परिमाण(content_handler_entry));
		यदि (entry == रिक्त)
			प्रत्यागम NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} अन्यथा {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	प्रत्यागम NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
स्थिर ध्रुव content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	प्रति (entry = content_handlers; entry != रिक्त; entry = entry->next) {
		यदि (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			भङ्ग;
		}
	}

	यदि (entry != रिक्त) {
		प्रत्यागम entry->handler;
	}

	प्रत्यागम रिक्त;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ध्रुव content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	यदि (handler != रिक्त) {
		type = handler->type();
	}

	प्रत्यागम type;
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
संरचना content *content_factory_create_content(llcache_handle *llcache,
		ध्रुव वर्ण *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	संरचना content *c;
	ध्रुव वर्ण *content_type_header;
	ध्रुव content_handler *handler;
	http_content_type *ct = रिक्त;
	nserror error;

	handler = content_lookup(effective_type);
	यदि (handler == रिक्त)
		प्रत्यागम रिक्त;

	प्रतिज्ञा(handler->create != रिक्त);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	यदि (content_type_header != रिक्त) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != रिक्त ? ct->parameters : रिक्त, 
			llcache, fallback_charset, quirks, 
			&c);

	यदि (ct != रिक्त)
		http_content_type_destroy(ct);

	यदि (error != NSERROR_OK)
		प्रत्यागम रिक्त;

	प्रत्यागम c;
}

