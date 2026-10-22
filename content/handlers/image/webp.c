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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <setjmp.h>

#অন্তর্ভাব <webp/decode.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image_cache.h"

#অন্তর্ভাব "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
স্থির nserror
webp_create(ধ্রুবক content_handler *handler,
	      lwc_string *imime_type,
	      ধ্রুবক কাঠামো http_parameter *params,
	      llcache_handle *llcache,
	      ধ্রুবক অক্ষর *fallback_charset,
	      bool quirks,
	      কাঠামো content **c)
{
	কাঠামো content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, আকার(কাঠামো content));
	যদি (webp_c == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	যদি (res != NSERROR_OK) {
		মুক্ত(webp_c);
		ফেরত res;
	}

	*c = webp_c;

	ফেরত NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
স্থির কাঠামো bitmap *
webp_cache_convert(কাঠামো content *c)
{
	ধ্রুবক uint8_t *source_তথ্য; /* webp source data */
	size_t source_আকার; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned পূর্ণসংখ্যা bmap_flags;
	uint8_t *pixels = ফাঁকা;
	uint8_t *decoded;
	size_t rowstride;
	কাঠামো bitmap *bitmap = ফাঁকা;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_তথ্য = content_get_source_তথ্য_2(c, &source_আকার);

	webpres = WebPGetFeatures(source_তথ্য, source_আকার, &webpfeatures);

	যদি (webpres != VP8_STATUS_OK) {
		ফেরত ফাঁকা;
	}

	যদি (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} অন্যথা {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	যদি (bitmap == ফাঁকা) {
		/* empty bitmap could not be created */
		ফেরত ফাঁকা;
	}

	pixels = guit->bitmap->get_বাফার(bitmap);
	যদি (pixels == ফাঁকা) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ফেরত ফাঁকা;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	নির্বাচন (webp_fmt.layout) {
	পূর্বনির্ধারিত:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	ক্ষেত্র BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_তথ্য, source_আকার, pixels,
				rowstride * webpfeatures.height, rowstride);
		বিরতি;

	ক্ষেত্র BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_তথ্য, source_আকার, pixels,
				rowstride * webpfeatures.height, rowstride);
		বিরতি;

	ক্ষেত্র BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_তথ্য, source_আকার, pixels,
				rowstride * webpfeatures.height, rowstride);
		বিরতি;
	}
	যদি (decoded == ফাঁকা) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ফেরত ফাঁকা;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ফেরত bitmap;
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
স্থির bool webp_convert(কাঠামো content *c)
{
	পূর্ণসংখ্যা res;
	ধ্রুবক uint8_t* তথ্য;
	size_t তথ্য_আকার;
	পূর্ণসংখ্যা width;
	পূর্ণসংখ্যা height;

	তথ্য = content_get_source_তথ্য_2(c, &তথ্য_আকার);

	res = WebPGetInfo(তথ্য, তথ্য_আকার, &width, &height);
	যদি (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ফেরত false;
	}

	c->width = width;
	c->height = height;
	c->আকার_2 = c->width * c->height * 4;

	image_cache_add(c, ফাঁকা, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	ফেরত true;
}

/**
 * Clone content.
 */
স্থির nserror webp_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **new_c)
{
	কাঠামো content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, আকার(কাঠামো content));
	যদি (webp_c == ফাঁকা) {
		ফেরত NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	যদি (res != NSERROR_OK) {
		content_destroy(webp_c);
		ফেরত res;
	}

	/* re-convert if the content is ready */
	যদি ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		যদি (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ফেরত NSERROR_OK;
}

স্থির ধ্রুবক content_handler webp_content_handler = {
	.create = webp_create,
	.তথ্য_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
