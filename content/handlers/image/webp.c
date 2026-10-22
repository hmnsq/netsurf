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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <setjmp.h>

#അന്തര്ഭാവ <webp/decode.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image_cache.h"

#അന്തര്ഭാവ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
സ്ഥിതം nserror
webp_create(സ്ഥിരം content_handler *handler,
	      lwc_string *imime_type,
	      സ്ഥിരം ഘടന http_parameter *params,
	      llcache_handle *llcache,
	      സ്ഥിരം അക്ഷരം *fallback_charset,
	      bool quirks,
	      ഘടന content **c)
{
	ഘടന content *webp_c; /* webp content object */
	nserror res;

	webp_c = calloc(1, വലുപ്പം(ഘടന content));
	എങ്കിൽ (webp_c == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	എങ്കിൽ (res != NSERROR_OK) {
		സ്വതന്ത്രം(webp_c);
		തിരികെ res;
	}

	*c = webp_c;

	തിരികെ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
സ്ഥിതം ഘടന bitmap *
webp_cache_convert(ഘടന content *c)
{
	സ്ഥിരം uint8_t *source_data; /* webp source data */
	size_t source_size; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	unsigned പൂർണ്ണസംഖ്യ bmap_flags;
	uint8_t *pixels = ശൂന്യ;
	uint8_t *decoded;
	size_t rowstride;
	ഘടന bitmap *bitmap = ശൂന്യ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_data = content__get_source_data(c, &source_size);

	webpres = WebPGetFeatures(source_data, source_size, &webpfeatures);

	എങ്കിൽ (webpres != VP8_STATUS_OK) {
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} അല്ലെങ്കിൽ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	എങ്കിൽ (bitmap == ശൂന്യ) {
		/* empty bitmap could not be created */
		തിരികെ ശൂന്യ;
	}

	pixels = guit->bitmap->get_buffer(bitmap);
	എങ്കിൽ (pixels == ശൂന്യ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		തിരികെ ശൂന്യ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	തിരഞ്ഞെടുപ്പ് (webp_fmt.layout) {
	സ്ഥിരസ്ഥിതി:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	അവസ്ഥ BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		നിർത്തുക;

	അവസ്ഥ BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		നിർത്തുക;

	അവസ്ഥ BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_data, source_size, pixels,
				rowstride * webpfeatures.height, rowstride);
		നിർത്തുക;
	}
	എങ്കിൽ (decoded == ശൂന്യ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		തിരികെ ശൂന്യ;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	തിരികെ bitmap;
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
സ്ഥിതം bool webp_convert(ഘടന content *c)
{
	പൂർണ്ണസംഖ്യ res;
	സ്ഥിരം uint8_t* data;
	size_t data_size;
	പൂർണ്ണസംഖ്യ width;
	പൂർണ്ണസംഖ്യ height;

	data = content__get_source_data(c, &data_size);

	res = WebPGetInfo(data, data_size, &width, &height);
	എങ്കിൽ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		തിരികെ false;
	}

	c->width = width;
	c->height = height;
	c->size = c->width * c->height * 4;

	image_cache_add(c, ശൂന്യ, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	തിരികെ true;
}

/**
 * Clone content.
 */
സ്ഥിതം nserror webp_clone(സ്ഥിരം ഘടന content *old, ഘടന content **new_c)
{
	ഘടന content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = calloc(1, വലുപ്പം(ഘടന content));
	എങ്കിൽ (webp_c == ശൂന്യ) {
		തിരികെ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	എങ്കിൽ (res != NSERROR_OK) {
		content_destroy(webp_c);
		തിരികെ res;
	}

	/* re-convert if the content is ready */
	എങ്കിൽ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		എങ്കിൽ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം സ്ഥിരം content_handler webp_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
