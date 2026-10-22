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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <setjmp.h>

#अन्तर्भाव <webp/decode.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"

#अन्तर्भाव "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
स्थिर nserror
webp_create(ध्रुव content_handler *handler,
	      lwc_string *imime_type,
	      ध्रुव संरचना http_parameter *params,
	      llcache_handle *llcache,
	      ध्रुव वर्ण *fallback_charset,
	      bool quirks,
	      संरचना content **c)
{
	संरचना content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, परिमाण(संरचना content));
	यदि (webp_c == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	यदि (res != NSERROR_OK) {
		मुक्त(webp_c);
		प्रत्यागम res;
	}

	*c = webp_c;

	प्रत्यागम NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
स्थिर संरचना bitmap *
webp_cache_convert(संरचना content *c)
{
	ध्रुव uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned पूर्णाङ्क bmap_flags;
	uint8_t *pixels = रिक्त;
	uint8_t *decoded;
	size_t rowstride;
	संरचना bitmap *bitmap = रिक्त;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	यदि (webpres != VP8_STATUS_OK) {
		प्रत्यागम रिक्त;
	}

	यदि (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} अन्यथा {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	यदि (bitmap == रिक्त) {
		/* empty bitmap could not be created */
		प्रत्यागम रिक्त;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	यदि (pixels == रिक्त) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		प्रत्यागम रिक्त;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	विकल्प (webp_fmt.layout) {
	स्वभाव:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	पक्ष BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		भङ्ग;

	पक्ष BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		भङ्ग;

	पक्ष BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		भङ्ग;
	}
	यदि (decoded == रिक्त) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		प्रत्यागम रिक्त;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	प्रत्यागम bitmap;
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
स्थिर bool webp_convert(संरचना content *c)
{
	पूर्णाङ्क res;
	ध्रुव uint8_t* data;
	size_t data_size;
	पूर्णाङ्क width;
	पूर्णाङ्क height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	यदि (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		प्रत्यागम false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, रिक्त, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	प्रत्यागम true;
}

/**
 * Clone content.
 */
स्थिर nserror webp_clone(ध्रुव संरचना content *old, संरचना content **new_c)
{
	संरचना content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, परिमाण(संरचना content));
	यदि (webp_c == रिक्त) {
		प्रत्यागम NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	यदि (res != NSERROR_OK) {
		content_destroy(webp_c);
		प्रत्यागम res;
	}

	/* re-convert if the content is ready */
	यदि ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		यदि (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	प्रत्यागम NSERROR_OK;
}

स्थिर ध्रुव content_handler webp_content_handler = {
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

स्थिर ध्रुव वर्ण *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
