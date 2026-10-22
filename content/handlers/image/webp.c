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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <setjmp.h>

#அந்தர்பாவ <webp/decode.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image_cache.h"

#அந்தர்பாவ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
நிலையான nserror
webp_create(மாறிலி content_handler *handler,
	      lwc_string *imime_type,
	      மாறிலி கட்டமைப்பு http_parameter *params,
	      llcache_handle *llcache,
	      மாறிலி எழுத்து *fallback_charset,
	      bool quirks,
	      கட்டமைப்பு content **c)
{
	கட்டமைப்பு content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, அளவு(கட்டமைப்பு content));
	எனில் (webp_c == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	எனில் (res != NSERROR_OK) {
		விடுவி(webp_c);
		திரும்பு res;
	}

	*c = webp_c;

	திரும்பு NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
நிலையான கட்டமைப்பு bitmap *
webp_cache_convert(கட்டமைப்பு content *c)
{
	மாறிலி uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned முழு bmap_flags;
	uint8_t *pixels = வெற்று;
	uint8_t *decoded;
	size_t rowstride;
	கட்டமைப்பு bitmap *bitmap = வெற்று;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	எனில் (webpres != VP8_STATUS_OK) {
		திரும்பு வெற்று;
	}

	எனில் (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} இல்லையெனில் {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	எனில் (bitmap == வெற்று) {
		/* empty bitmap could not be created */
		திரும்பு வெற்று;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	எனில் (pixels == வெற்று) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		திரும்பு வெற்று;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	தேர்வு (webp_fmt.layout) {
	இயல்பு:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	நிலை BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		நிறுத்து;

	நிலை BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		நிறுத்து;

	நிலை BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		நிறுத்து;
	}
	எனில் (decoded == வெற்று) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		திரும்பு வெற்று;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	திரும்பு bitmap;
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
நிலையான bool webp_convert(கட்டமைப்பு content *c)
{
	முழு res;
	மாறிலி uint8_t* data;
	size_t data_size;
	முழு width;
	முழு height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	எனில் (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		திரும்பு false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, வெற்று, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	திரும்பு true;
}

/**
 * Clone content.
 */
நிலையான nserror webp_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **new_c)
{
	கட்டமைப்பு content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, அளவு(கட்டமைப்பு content));
	எனில் (webp_c == வெற்று) {
		திரும்பு NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	எனில் (res != NSERROR_OK) {
		content_destroy(webp_c);
		திரும்பு res;
	}

	/* re-convert if the content is ready */
	எனில் ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		எனில் (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	திரும்பு NSERROR_OK;
}

நிலையான மாறிலி content_handler webp_content_handler = {
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

நிலையான மாறிலி எழுத்து *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
