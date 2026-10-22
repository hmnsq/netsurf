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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>

#ଅନ୍ତର୍ଭାଵ <webp/decode.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"

#ଅନ୍ତର୍ଭାଵ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
ସ୍ଥିର nserror
webp_create(ଅଚଳ content_handler *handler,
	      lwc_string *imime_type,
	      ଅଚଳ ସଂରଚନା http_parameter *params,
	      llcache_handle *llcache,
	      ଅଚଳ ଅକ୍ଷର *fallback_charset,
	      bool quirks,
	      ସଂରଚନା content **c)
{
	ସଂରଚନା content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, ଆକାର(ସଂରଚନା content));
	ଯଦି (webp_c == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ଯଦି (res != NSERROR_OK) {
		ମୁକ୍ତ(webp_c);
		ଫେରନ୍ତୁ res;
	}

	*c = webp_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ସ୍ଥିର ସଂରଚନା bitmap *
webp_cache_convert(ସଂରଚନା content *c)
{
	ଅଚଳ uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmap_flags;
	uint8_t *pixels = ଖାଲି;
	uint8_t *decoded;
	size_t rowstride;
	ସଂରଚନା bitmap *bitmap = ଖାଲି;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	ଯଦି (webpres != VP8_STATUS_OK) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} ଅନ୍ୟଥା {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	ଯଦି (bitmap == ଖାଲି) {
		/* empty bitmap could not be created */
		ଫେରନ୍ତୁ ଖାଲି;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	ଯଦି (pixels == ଖାଲି) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	ବାଛ (webp_fmt.layout) {
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	ସ୍ଥିତି BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		ଭାଙ୍ଗ;
	}
	ଯଦି (decoded == ଖାଲି) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ଫେରନ୍ତୁ bitmap;
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
ସ୍ଥିର bool webp_convert(ସଂରଚନା content *c)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
	ଅଚଳ uint8_t* data;
	size_t data_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	ଯଦି (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ଫେରନ୍ତୁ false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, ଖାଲି, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	ଫେରନ୍ତୁ true;
}

/**
 * Clone content.
 */
ସ୍ଥିର nserror webp_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **new_c)
{
	ସଂରଚନା content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, ଆକାର(ସଂରଚନା content));
	ଯଦି (webp_c == ଖାଲି) {
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ଯଦି (res != NSERROR_OK) {
		content_destroy(webp_c);
		ଫେରନ୍ତୁ res;
	}

	/* re-convert if the content is ready */
	ଯଦି ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ଯଦି (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଅଚଳ content_handler webp_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
