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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "utils/http.h"

#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/llcache.h"

/**
 * Entry in list of content handlers
 */
প্রকারনির্দেশ সংরচনা content_handler_entry {
	/** Next entry */
	সংরচনা content_handler_entry *next;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ধ্রুব content_handler *handler;
} content_handler_entry;

স্থির content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
শূন্য content_factory_fini(শূন্য)
{
	content_handler_entry *victim;

	যাবত্ (content_handlers != রিক্ত) {
		victim = content_handlers;

		content_handlers = content_handlers->next;

		যদি (victim->handler->fini != রিক্ত)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		মুক্ত(victim);
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
nserror content_factory_register_handler(ধ্রুব বর্ণ *mime_type,
		ধ্রুব content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	যদি (lerror != lwc_error_ok)
		প্রত্যাগম NSERROR_NOMEM;

	প্রতি (entry = content_handlers; entry != রিক্ত; entry = entry->next) {
		যদি (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			ভঙ্গ;
	}

	যদি (entry == রিক্ত) {
		entry = malloc(পরিমাণ(content_handler_entry));
		যদি (entry == রিক্ত)
			প্রত্যাগম NSERROR_NOMEM;

		entry->next = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} অন্যথা {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	প্রত্যাগম NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
স্থির ধ্রুব content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	প্রতি (entry = content_handlers; entry != রিক্ত; entry = entry->next) {
		যদি (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			ভঙ্গ;
		}
	}

	যদি (entry != রিক্ত) {
		প্রত্যাগম entry->handler;
	}

	প্রত্যাগম রিক্ত;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ধ্রুব content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	যদি (handler != রিক্ত) {
		type = handler->type();
	}

	প্রত্যাগম type;
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
সংরচনা content *content_factory_create_content(llcache_handle *llcache,
		ধ্রুব বর্ণ *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	সংরচনা content *c;
	ধ্রুব বর্ণ *content_type_header;
	ধ্রুব content_handler *handler;
	http_content_type *ct = রিক্ত;
	nserror error;

	handler = content_lookup(effective_type);
	যদি (handler == রিক্ত)
		প্রত্যাগম রিক্ত;

	প্রতিজ্ঞা(handler->create != রিক্ত);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	যদি (content_type_header != রিক্ত) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != রিক্ত ? ct->parameters : রিক্ত, 
			llcache, fallback_charset, quirks, 
			&c);

	যদি (ct != রিক্ত)
		http_content_type_destroy(ct);

	যদি (error != NSERROR_OK)
		প্রত্যাগম রিক্ত;

	প্রত্যাগম c;
}

