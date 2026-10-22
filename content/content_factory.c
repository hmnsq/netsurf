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

#समावेश <assert.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "utils/http.h"

#समावेश "content/content.h"
#समावेश "content/content_factory.h"
#समावेश "content/content_protected.h"
#समावेश "content/llcache.h"

/**
 * Entry in list of content handlers
 */
प्रकार काष्ठा content_handler_entry {
	/** Next entry */
	काष्ठा content_handler_entry *अग्र;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	स्थिर content_handler *handler;
} content_handler_entry;

अटल content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
व्योम content_factory_fini(व्योम)
{
	content_handler_entry *victim;

	जबतक (content_handlers != शून्य) {
		victim = content_handlers;

		content_handlers = content_handlers->अग्र;

		अगर (victim->handler->fini != शून्य)
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
nserror content_factory_register_handler(स्थिर अक्षर *mime_type,
		स्थिर content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, म_माप(mime_type), &imime_type);
	अगर (lerror != lwc_error_ok)
		वापस NSERROR_NOMEM;

	क्रम (entry = content_handlers; entry != शून्य; entry = entry->अग्र) {
		अगर (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			अवरोध;
	}

	अगर (entry == शून्य) {
		entry = दो_स्मृति(माप(content_handler_entry));
		अगर (entry == शून्य)
			वापस NSERROR_NOMEM;

		entry->अग्र = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} अन्यथा {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	वापस NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
अटल स्थिर content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	क्रम (entry = content_handlers; entry != शून्य; entry = entry->अग्र) {
		अगर (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			अवरोध;
		}
	}

	अगर (entry != शून्य) {
		वापस entry->handler;
	}

	वापस शून्य;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	स्थिर content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	अगर (handler != शून्य) {
		type = handler->type();
	}

	वापस type;
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
काष्ठा content *content_factory_create_content(llcache_handle *llcache,
		स्थिर अक्षर *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	काष्ठा content *c;
	स्थिर अक्षर *content_type_header;
	स्थिर content_handler *handler;
	http_content_type *ct = शून्य;
	nserror error;

	handler = content_lookup(effective_type);
	अगर (handler == शून्य)
		वापस शून्य;

	निश्चित(handler->create != शून्य);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	अगर (content_type_header != शून्य) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != शून्य ? ct->parameters : शून्य, 
			llcache, fallback_charset, quirks, 
			&c);

	अगर (ct != शून्य)
		http_content_type_destroy(ct);

	अगर (error != NSERROR_OK)
		वापस शून्य;

	वापस c;
}

