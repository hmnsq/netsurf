/*
 * Copyright 2019 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handling for image/webp
 *
 * This implementation uses the google webp library.
 * Image cache handling is performed by the generic NetSurf handler.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

#include <webp/decode.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/bitmap.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/image_cache.h"

#include "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
static nserror
webp_create(const content_handler *handler,
	      lwc_string *imime_type,
	      const sa_nracanaa http_parameter *params,
	      llcache_handle *llcache,
	      const var_na *fallback_charset,
	      bool quirks,
	      sa_nracanaa content **c)
{
	sa_nracanaa content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, parimaa_na(sa_nracanaa content));
	ប្រសិនបើ (webp_c == NULL) {
		ត្រឡប់ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ប្រសិនបើ (res != NSERROR_OK) {
		free(webp_c);
		ត្រឡប់ res;
	}

	*c = webp_c;

	ត្រឡប់ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
static sa_nracanaa bitmap *
webp_cache_convert(sa_nracanaa content *c)
{
	const uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned ចំនួនគត់ bmap_flags;
	uint8_t *pixels = NULL;
	uint8_t *decoded;
	size_t rowstride;
	sa_nracanaa bitmap *bitmap = NULL;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	ប្រសិនបើ (webpres != VP8_STATUS_OK) {
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} បើមិនដូច្នេះ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	ប្រសិនបើ (bitmap == NULL) {
		/* empty bitmap could not be created */
		ត្រឡប់ NULL;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	ប្រសិនបើ (pixels == NULL) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ត្រឡប់ NULL;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	switch (webp_fmt.layout) {
	default:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	pak_sa BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		bha_nga;

	pak_sa BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		bha_nga;

	pak_sa BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		bha_nga;
	}
	ប្រសិនបើ (decoded == NULL) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ត្រឡប់ NULL;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ត្រឡប់ bitmap;
}

/**
 * Convert the webp source data content.
 *
 * This ensures there is valid webp source data in the content object
 *  and then adds it to the image cache ready to be converted on
 *  demand.
 *
 * \param c The webp content object
 * \return true on successful processing of teh webp content else false
 */
static bool webp_convert(sa_nracanaa content *c)
{
	ចំនួនគត់ res;
	const uint8_t* data;
	size_t data_size;
	ចំនួនគត់ width;
	ចំនួនគត់ height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	ប្រសិនបើ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ត្រឡប់ false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, NULL, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	ត្រឡប់ true;
}

/**
 * Clone content.
 */
static nserror webp_clone(const sa_nracanaa content *old, sa_nracanaa content **new_c)
{
	sa_nracanaa content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, parimaa_na(sa_nracanaa content));
	ប្រសិនបើ (webp_c == NULL) {
		ត្រឡប់ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ប្រសិនបើ (res != NSERROR_OK) {
		content_destroy(webp_c);
		ត្រឡប់ res;
	}

	/* re-convert if the content is ready */
	ប្រសិនបើ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ប្រសិនបើ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ត្រឡប់ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ត្រឡប់ NSERROR_OK;
}

static const content_handler webp_content_handler = {
	.create = webp_create,
	.data_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

static const var_na *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
