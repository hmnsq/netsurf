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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <setjmp.h>

#شامل <webp/decode.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/bitmap.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image_cache.h"

#شامل "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
ایستا nserror
webp_create(ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت ساختار http_parameter *params,
	      llcache_handle *llcache,
	      ثابت نویسه *fallback_charset,
	      bool quirks,
	      ساختار content **c)
{
	ساختار content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, اندازه(ساختار content));
	اگر (webp_c == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	اگر (res != NSERROR_OK) {
		آزاد(webp_c);
		بازگشت res;
	}

	*c = webp_c;

	بازگشت NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ایستا ساختار bitmap *
webp_cache_convert(ساختار content *c)
{
	ثابت uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned صحیح bmap_flags;
	uint8_t *pixels = پوچ;
	uint8_t *decoded;
	size_t rowstride;
	ساختار bitmap *bitmap = پوچ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	اگر (webpres != VP8_STATUS_OK) {
		بازگشت پوچ;
	}

	اگر (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} وگرنه {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	اگر (bitmap == پوچ) {
		/* empty bitmap could not be created */
		بازگشت پوچ;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	اگر (pixels == پوچ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		بازگشت پوچ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	انتخاب (webp_fmt.layout) {
	پیشفرض:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	حالت BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		بشکن;

	حالت BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		بشکن;

	حالت BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		بشکن;
	}
	اگر (decoded == پوچ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		بازگشت پوچ;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	بازگشت bitmap;
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
ایستا bool webp_convert(ساختار content *c)
{
	صحیح res;
	ثابت uint8_t* data;
	size_t data_size;
	صحیح width;
	صحیح height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	اگر (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		بازگشت false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, پوچ, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	بازگشت true;
}

/**
 * Clone content.
 */
ایستا nserror webp_clone(ثابت ساختار content *old, ساختار content **new_c)
{
	ساختار content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, اندازه(ساختار content));
	اگر (webp_c == پوچ) {
		بازگشت NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	اگر (res != NSERROR_OK) {
		content_destroy(webp_c);
		بازگشت res;
	}

	/* re-convert if the content is ready */
	اگر ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		اگر (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	بازگشت NSERROR_OK;
}

ایستا ثابت content_handler webp_content_handler = {
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

ایستا ثابت نویسه *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
