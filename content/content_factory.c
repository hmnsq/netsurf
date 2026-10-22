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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "utils/http.h"

#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/llcache.h"

/**
 * Entry in list of content handlers
 */
రకనిర్వచనం నిర్మాణం content_handler_నమోదు {
	/** Next entry */
	నిర్మాణం content_handler_నమోదు *తదుపరి;

	/** MIME type handled by handler */
	lwc_string *mime_type;
	/** Content handler object */
	స్థిరరాశి content_handler *handler;
} content_handler_నమోదు;

స్థిర content_handler_నమోదు *content_handlers;

/**
 * Clean up after the content factory
 */
శూన్య content_factory_fini(శూన్య)
{
	content_handler_నమోదు *victim;

	అయితే (content_handlers != ఖాళీ) {
		victim = content_handlers;

		content_handlers = content_handlers->తదుపరి;

		ఒకవేళ (victim->handler->fini != ఖాళీ)
			victim->handler->fini();

		lwc_string_unref(victim->mime_type);

		విడుదల(victim);
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
nserror content_factory_రిజిస్టరు_handler(స్థిరరాశి అక్షరం *mime_type,
		స్థిరరాశి content_handler *handler)
{
	lwc_string *imime_type;
	lwc_error lerror;
	content_handler_నమోదు *నమోదు_2;
	bool సరిపోలిక;

	lerror = lwc_intern_string(mime_type, అక్షరశ్రేణి_పొడవు(mime_type), &imime_type);
	ఒకవేళ (lerror != lwc_error_ok)
		తిరిగి NSERROR_NOMEM;

	కొరకు (నమోదు_2 = content_handlers; నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
		ఒకవేళ (lwc_string_caseless_isequal(imime_type, నమోదు_2->mime_type,
				&సరిపోలిక) == lwc_error_ok && సరిపోలిక)
			ఆపు;
	}

	ఒకవేళ (నమోదు_2 == ఖాళీ) {
		నమోదు_2 = జ్ఞాపకం_ఇవ్వు(పరిమాణం(content_handler_నమోదు));
		ఒకవేళ (నమోదు_2 == ఖాళీ)
			తిరిగి NSERROR_NOMEM;

		నమోదు_2->తదుపరి = content_handlers;
		content_handlers = నమోదు_2;

		నమోదు_2->mime_type = imime_type;
	} లేకపోతే {
		lwc_string_unref(imime_type);
	}

	నమోదు_2->handler = handler;

	తిరిగి NSERROR_OK;
}

/**
 * Find a handler for a MIME type.
 *
 * \param mime_type  MIME type to search for
 * \return Associated handler, or NULL if none
 */
స్థిర స్థిరరాశి content_handler *content_lookup(lwc_string *mime_type)
{
	content_handler_నమోదు *నమోదు_2;
	bool సరిపోలిక;

	కొరకు (నమోదు_2 = content_handlers; నమోదు_2 != ఖాళీ; నమోదు_2 = నమోదు_2->తదుపరి) {
		ఒకవేళ (lwc_string_caseless_isequal(mime_type, నమోదు_2->mime_type,
					&సరిపోలిక) == lwc_error_ok && సరిపోలిక) {
			ఆపు;
		}
	}

	ఒకవేళ (నమోదు_2 != ఖాళీ) {
		తిరిగి నమోదు_2->handler;
	}

	తిరిగి ఖాళీ;
}

/**
 * Compute the generic content type for a MIME type
 *
 * \param mime_type  MIME type to consider
 * \return Generic content type
 */
content_type content_factory_type_from_mime_type(lwc_string *mime_type)
{
	స్థిరరాశి content_handler *handler;
	content_type type = CONTENT_NONE;

	handler = content_lookup(mime_type);
	ఒకవేళ (handler != ఖాళీ) {
		type = handler->type();
	}

	తిరిగి type;
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
నిర్మాణం content *content_factory_create_content(llcache_handle *llcache,
		స్థిరరాశి అక్షరం *fallback_charset, bool quirks,
		lwc_string *effective_type)
{
	నిర్మాణం content *c;
	స్థిరరాశి అక్షరం *content_type_శీర్షిక;
	స్థిరరాశి content_handler *handler;
	http_content_type *ct = ఖాళీ;
	nserror error;

	handler = content_lookup(effective_type);
	ఒకవేళ (handler == ఖాళీ)
		తిరిగి ఖాళీ;

	ప్రతిజ్ఞా(handler->create != ఖాళీ);

	/* Use the parameters from the declared Content-Type header */
	content_type_శీర్షిక = 
			llcache_handle_get_శీర్షిక(llcache, "Content-Type");
	ఒకవేళ (content_type_శీర్షిక != ఖాళీ) {
		/* We don't care if this fails */
		http_parse_content_type(content_type_శీర్షిక, &ct);
	}

	error = handler->create(handler, effective_type, 
			ct != ఖాళీ ? ct->parameters : ఖాళీ, 
			llcache, fallback_charset, quirks, 
			&c);

	ఒకవేళ (ct != ఖాళీ)
		http_content_type_destroy(ct);

	ఒకవేళ (error != NSERROR_OK)
		తిరిగి ఖాళీ;

	తిరిగి c;
}

