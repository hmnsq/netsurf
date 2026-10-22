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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <setjmp.h>

#ضمن <webp/decode.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image_cache.h"

#ضمن "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
ساكن nserror
webp_create(ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت بنية http_parameter *params,
	      llcache_handle *llcache,
	      ثابت حرف *fallback_charset,
	      bool quirks,
	      بنية content **c)
{
	بنية content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, حجم(بنية content));
	𐤀𐤌 (webp_c == عدم) {
		أرجع NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	𐤀𐤌 (res != NSERROR_OK) {
		حرر(webp_c);
		أرجع res;
	}

	*c = webp_c;

	أرجع NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ساكن بنية bitmap *
webp_cache_convert(بنية content *c)
{
	ثابت uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned صحيح bmap_flags;
	uint8_t *pixels = عدم;
	uint8_t *decoded;
	size_t rowstride;
	بنية bitmap *bitmap = عدم;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	𐤀𐤌 (webpres != VP8_STATUS_OK) {
		أرجع عدم;
	}

	𐤀𐤌 (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} وإلا {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	𐤀𐤌 (bitmap == عدم) {
		/* empty bitmap could not be created */
		أرجع عدم;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	𐤀𐤌 (pixels == عدم) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		أرجع عدم;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	بدل (webp_fmt.layout) {
	افتراضي:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	حالة BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		اكسر;

	حالة BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		اكسر;

	حالة BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		اكسر;
	}
	𐤀𐤌 (decoded == عدم) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		أرجع عدم;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	أرجع bitmap;
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
ساكن bool webp_convert(بنية content *c)
{
	صحيح res;
	ثابت uint8_t* data;
	size_t data_size;
	صحيح width;
	صحيح height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	𐤀𐤌 (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		أرجع false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, عدم, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	أرجع true;
}

/**
 * Clone content.
 */
ساكن nserror webp_clone(ثابت بنية content *old, بنية content **new_c)
{
	بنية content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, حجم(بنية content));
	𐤀𐤌 (webp_c == عدم) {
		أرجع NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	𐤀𐤌 (res != NSERROR_OK) {
		content_destroy(webp_c);
		أرجع res;
	}

	/* re-convert if the content is ready */
	𐤀𐤌 ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		𐤀𐤌 (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	أرجع NSERROR_OK;
}

ساكن ثابت content_handler webp_content_handler = {
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

ساكن ثابت حرف *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
