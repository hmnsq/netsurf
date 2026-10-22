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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <setjmp.h>

#ಅನ್ತರ್ಭಾವ <webp/decode.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"

#ಅನ್ತರ್ಭಾವ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
ಸ್ಥಿರ nserror
webp_create(ಧ್ರುವ content_handler *handler,
	      lwc_string *imime_type,
	      ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
	      llcache_handle *llcache,
	      ಧ್ರುವ ವರ್ಣ *fallback_charset,
	      bool quirks,
	      ಸಂರಚನಾ content **c)
{
	ಸಂರಚನಾ content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content));
	ಯದಿ (webp_c == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ಯದಿ (res != NSERROR_OK) {
		ಮುಕ್ತ(webp_c);
		ಪ್ರತ್ಯಾಗಮ res;
	}

	*c = webp_c;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ಸ್ಥಿರ ಸಂರಚನಾ bitmap *
webp_cache_convert(ಸಂರಚನಾ content *c)
{
	ಧ್ರುವ uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned ಪೂರ್ಣಾಙ್ಕ bmap_flags;
	uint8_t *pixels = ರಿಕ್ತ;
	uint8_t *decoded;
	size_t rowstride;
	ಸಂರಚನಾ bitmap *bitmap = ರಿಕ್ತ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	ಯದಿ (webpres != VP8_STATUS_OK) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} ಅನ್ಯಥಾ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	ಯದಿ (bitmap == ರಿಕ್ತ) {
		/* empty bitmap could not be created */
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	ಯದಿ (pixels == ರಿಕ್ತ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	ವಿಕಲ್ಪ (webp_fmt.layout) {
	ಸ್ವಭಾವ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	ಪಕ್ಷ BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ಭಙ್ಗ;

	ಪಕ್ಷ BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ಭಙ್ಗ;

	ಪಕ್ಷ BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ಭಙ್ಗ;
	}
	ಯದಿ (decoded == ರಿಕ್ತ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ಪ್ರತ್ಯಾಗಮ bitmap;
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
ಸ್ಥಿರ bool webp_convert(ಸಂರಚನಾ content *c)
{
	ಪೂರ್ಣಾಙ್ಕ res;
	ಧ್ರುವ uint8_t* data;
	size_t data_size;
	ಪೂರ್ಣಾಙ್ಕ width;
	ಪೂರ್ಣಾಙ್ಕ height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	ಯದಿ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, ರಿಕ್ತ, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Clone content.
 */
ಸ್ಥಿರ nserror webp_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **new_c)
{
	ಸಂರಚನಾ content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ content));
	ಯದಿ (webp_c == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ಯದಿ (res != NSERROR_OK) {
		content_destroy(webp_c);
		ಪ್ರತ್ಯಾಗಮ res;
	}

	/* re-convert if the content is ready */
	ಯದಿ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ಯದಿ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler webp_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
