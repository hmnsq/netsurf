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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "utils/http.h"

#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା content_handler_entry {
	/** Next entry */
	ସଂରଚନା content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ଅଚଳ content_handler *handler;
} content_handler_entry;

ସ୍ଥିର content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
ଶୂନ୍ୟ content_factory_fini(ଶୂନ୍ୟ)
{
	content_handler_entry *victim;

	ଯେତେବେଳେ (content_handlers != ଖାଲି) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		ଯଦି (victim->handler->fini != ଖାଲି)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		ମୁକ୍ତ(victim);
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
nserror content_factory_register_handler(ଅଚଳ ଅକ୍ଷର *mime_type,
		ଅଚଳ content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	ଯଦି (lerror != lwc_error_ok)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	ପାଇଁ (entry = content_handlers; entry != ଖାଲି; entry = entry->next) {
		ଯଦି (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			ଭାଙ୍ଗ;
	}

	ଯଦି (entry == ଖାଲି) {
		entry = malloc(ଆକାର(content_handler_entry));
		ଯଦି (entry == ଖାଲି)
			ଫେରନ୍ତୁ NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} ଅନ୍ୟଥା {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ସ୍ଥିର ଅଚଳ content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	ପାଇଁ (entry = content_handlers; entry != ଖାଲି; entry = entry->next) {
		ଯଦି (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (entry != ଖାଲି) {
		ଫେରନ୍ତୁ entry->handler;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ଅଚଳ content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	ଯଦି (handler != ଖାଲି) {
		type = handler->type();
	}

	ଫେରନ୍ତୁ type;
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
ସଂରଚନା content *content_factory_create_content(llcache_handle *llcache,
		ଅଚଳ ଅକ୍ଷର *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	ସଂରଚନା content *c;
	ଅଚଳ ଅକ୍ଷର *content_type_header;
	ଅଚଳ content_handler *handler;
	http_content_type *ct = ଖାଲି;
	nserror error;

	handler = content_lookup(effective_type);
	ଯଦି (handler == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	ପ୍ରତିଜ୍ଞା(handler->create != ଖାଲି);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	ଯଦି (content_type_header != ଖାଲି) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ଖାଲି ? ct->parameters : ଖାଲି, 
			llcache, fallback_charset, quirks, 
			&c);

	ଯଦି (ct != ଖାଲି)
		http_content_type_destroy(ct);

	ଯଦି (error != NSERROR_OK)
		ଫେରନ୍ତୁ ଖାଲି;

	ଫେରନ୍ତୁ c;
}

