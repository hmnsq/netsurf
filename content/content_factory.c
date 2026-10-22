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
প্রকারলেখ কাঠামো content_handler_entry {
	/** Next entry */
	কাঠামো content_handler_entry *পরবর্তী;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ধ্রুবক content_handler *handler;
} content_handler_entry;

স্থির content_handler_entry *content_handlers;

/**
 * Clean up after the content factory
 */
শূন্য content_factory_fini(শূন্য)
{
	content_handler_entry *victim;

	যখন (content_handlers != ফাঁকা) {
		victim = content_handlers;

		content_handlers = content_handlers->পরবর্তী;

		যদি (victim->handler->fini != ফাঁকা)
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
nserror content_factory_register_handler(ধ্রুবক অক্ষর *mime_type,
		ধ্রুবক content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_entry *entry;
	bool match;

	lerror = lwc_intern_string(mime_type, strlen(mime_type), &imime_type);
	যদি (lerror != lwc_error_ok)
		ফেরত NSERROR_NOMEM;

	জন্য (entry = content_handlers; entry != ফাঁকা; entry = entry->পরবর্তী) {
		যদি (lwc_string_caseless_isequal(imime_type, entry->mime_type,
				&match) == lwc_error_ok && match)
			বিরতি;
	}

	যদি (entry == ফাঁকা) {
		entry = malloc(আকার(content_handler_entry));
		যদি (entry == ফাঁকা)
			ফেরত NSERROR_NOMEM;

		entry->পরবর্তী = content_handlers;
		content_handlers = entry;

		entry->mime_type = imime_type;
	} অন্যথা {
		lwc_string_unref(imime_type);
	}

	entry->handler = handler;

	ফেরত NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
স্থির ধ্রুবক content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_entry *entry;
	bool match;

	জন্য (entry = content_handlers; entry != ফাঁকা; entry = entry->পরবর্তী) {
		যদি (lwc_string_caseless_isequal(mime_type, entry->mime_type,
					&match) == lwc_error_ok && match) {
			বিরতি;
		}
	}

	যদি (entry != ফাঁকা) {
		ফেরত entry->handler;
	}

	ফেরত ফাঁকা;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	ধ্রুবক content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	যদি (handler != ফাঁকা) {
		type = handler->type();
	}

	ফেরত type;
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
কাঠামো content *content_factory_create_content(llcache_handle *llcache,
		ধ্রুবক অক্ষর *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	কাঠামো content *c;
	ধ্রুবক অক্ষর *content_type_header;
	ধ্রুবক content_handler *handler;
	http_content_type *ct = ফাঁকা;
	nserror error;

	handler = content_lookup(effective_type);
	যদি (handler == ফাঁকা)
		ফেরত ফাঁকা;

	প্রতিজ্ঞা(handler->create != ফাঁকা);

	/* Use the parameters from the declared Content-Type header */
	content_type_header = 
			llcache_handle_get_header(llcache, "Content-Type");
	যদি (content_type_header != ফাঁকা) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_header, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ফাঁকা ? ct->parameters : ফাঁকা, 
			llcache, fallback_charset, quirks, 
			&c);

	যদি (ct != ফাঁকা)
		http_content_type_destroy(ct);

	যদি (error != NSERROR_OK)
		ফেরত ফাঁকা;

	ফেরত c;
}

