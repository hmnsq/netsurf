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

#ضمن <assert.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "utils/http.h"

#ضمن "content/content.h"
#ضمن "content/content_factory.h"
#ضمن "content/content_protected.h"
#ضمن "content/llcache.h"

/**
 * Entry in list of content handlers
 */
تعريفنوع بنية content_handler_مدخل {
	/** Next entry */
	بنية content_handler_مدخل *بعدی;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	ثابت content_handler *handler;
} content_handler_مدخل;

ساكن content_handler_مدخل *content_handlers;

/**
 * Clean up after the content factory
 */
فراغ content_factory_fini(فراغ)
{
	content_handler_مدخل *victim;

	طالما (content_handlers != عدم) {
		victim = content_handlers;

		content_handlers = content_handlers->بعدی;

		إذا (victim->handler->fini != عدم)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		حرر(victim);
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
nserror content_factory_ثبات_handler(ثابت حرف *mime_type,
		ثابت content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_مدخل *مدخل;
	bool تطابق;

	lerror = lwc_intern_string(mime_type, طول_رشته(mime_type), &imime_type);
	إذا (lerror != lwc_error_ok)
		أرجع NSERROR_NOMEM;

	لكل (مدخل = content_handlers; مدخل != عدم; مدخل = مدخل->بعدی) {
		إذا (lwc_string_caseless_isequal(imime_type, مدخل->mime_type,
				&تطابق) == lwc_error_ok && تطابق)
			اكسر;
	}

	إذا (مدخل == عدم) {
		مدخل = حافظه_بده(حجم(content_handler_مدخل));
		إذا (مدخل == عدم)
			أرجع NSERROR_NOMEM;

		مدخل->بعدی = content_handlers;
		content_handlers = مدخل;

		مدخل->mime_type = imime_type;
	} وإلا {
		lwc_string_unref(imime_type);
	}

	مدخل->handler = handler;

	أرجع NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
ساكن ثابت content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_مدخل *مدخل;
	bool تطابق;

	لكل (مدخل = content_handlers; مدخل != عدم; مدخل = مدخل->بعدی) {
		إذا (lwc_string_caseless_isequal(mime_type, مدخل->mime_type,
					&تطابق) == lwc_error_ok && تطابق) {
			اكسر;
		}
	}

	إذا (مدخل != عدم) {
		أرجع مدخل->handler;
	}

	أرجع عدم;
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
	إذا (handler != عدم) {
		type = handler->type();
	}

	أرجع type;
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
بنية content *content_factory_create_content(llcache_handle *llcache,
		ثابت حرف *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	بنية content *c;
	ثابت حرف *content_type_سرآیند;
	ثابت content_handler *handler;
	http_content_type *ct = عدم;
	nserror error;

	handler = content_lookup(effective_type);
	إذا (handler == عدم)
		أرجع عدم;

	أكد(handler->create != عدم);

	/* Use the parameters from the declared Content-Type header */
	content_type_سرآیند = 
			llcache_handle_get_سرآیند(llcache, "Content-Type");
	إذا (content_type_سرآیند != عدم) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_سرآیند, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != عدم ? ct->parameters : عدم, 
			llcache, fallback_charset, quirks, 
			&c);

	إذا (ct != عدم)
		http_content_type_destroy(ct);

	إذا (error != NSERROR_OK)
		أرجع عدم;

	أرجع c;
}

