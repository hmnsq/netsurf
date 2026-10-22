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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <webp/decode.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image_cache.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
ਸਥਿਰ nserror
webp_create(ਅਚਲ content_handler *handler,
	      lwc_string *imime_type,
	      ਅਚਲ ਬਣਤਰ http_parameter *params,
	      llcache_handle *llcache,
	      ਅਚਲ ਅੱਖਰ *fallback_charset,
	      bool quirks,
	      ਬਣਤਰ content **c)
{
	ਬਣਤਰ content *webp_c; /* webp content object */
	nserror res;

	webp_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content));
	ਜੇ (webp_c == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ਜੇ (res != NSERROR_OK) {
		ਮੁਕਤ(webp_c);
		ਵਾਪਸ res;
	}

	*c = webp_c;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ਸਥਿਰ ਬਣਤਰ bitmap *
webp_cache_convert(ਬਣਤਰ content *c)
{
	ਅਚਲ uint8_t *ਸਰੋਤ_ਅੰਕੜੇ; /* webp source data */
	size_t ਸਰੋਤ_ਮਾਪ; /* length of webp source data */
	VP8_ਸਥਿਤੀ_ਕੋਡ webpres;
	WebPBitstreamFeatures webpfeatures;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bmap_flags;
	uint8_t *pixels = ਨਿਰਾ;
	uint8_t *decoded;
	size_t rowstride;
	ਬਣਤਰ bitmap *bitmap = ਨਿਰਾ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	ਸਰੋਤ_ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਸਰੋਤ_ਮਾਪ);

	webpres = WebPGetFeatures(ਸਰੋਤ_ਅੰਕੜੇ, ਸਰੋਤ_ਮਾਪ, &webpfeatures);

	ਜੇ (webpres != VP8_ਸਥਿਤੀ_OK) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} ਨਹੀਂਤਾਂ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.ਚੌੜਾਈ,
				      webpfeatures.ਉਚਾਈ,
				      bmap_flags);
	ਜੇ (bitmap == ਨਿਰਾ) {
		/* empty bitmap could not be created */
		ਵਾਪਸ ਨਿਰਾ;
	}

	pixels = guit->bitmap->get_ਬਫਰ(bitmap);
	ਜੇ (pixels == ਨਿਰਾ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ਵਾਪਸ ਨਿਰਾ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	ਚੋਣ (webp_fmt.layout) {
	ਮੂਲ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	ਹਾਲਤ BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(ਸਰੋਤ_ਅੰਕੜੇ, ਸਰੋਤ_ਮਾਪ, pixels,
				rowstride * webpfeatures.ਉਚਾਈ, rowstride);
		ਰੋਕੋ;

	ਹਾਲਤ BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(ਸਰੋਤ_ਅੰਕੜੇ, ਸਰੋਤ_ਮਾਪ, pixels,
				rowstride * webpfeatures.ਉਚਾਈ, rowstride);
		ਰੋਕੋ;

	ਹਾਲਤ BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(ਸਰੋਤ_ਅੰਕੜੇ, ਸਰੋਤ_ਮਾਪ, pixels,
				rowstride * webpfeatures.ਉਚਾਈ, rowstride);
		ਰੋਕੋ;
	}
	ਜੇ (decoded == ਨਿਰਾ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ਵਾਪਸ ਨਿਰਾ;
	}

	bitmap_format_ਮੰਜ਼ਿਲ_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ਵਾਪਸ bitmap;
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
ਸਥਿਰ bool webp_convert(ਬਣਤਰ content *c)
{
	ਪੂਰਨ res;
	ਅਚਲ uint8_t* ਅੰਕੜੇ;
	size_t ਅੰਕੜੇ_ਮਾਪ;
	ਪੂਰਨ ਚੌੜਾਈ;
	ਪੂਰਨ ਉਚਾਈ;

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਅੰਕੜੇ_ਮਾਪ);

	res = WebPGetInfo(ਅੰਕੜੇ, ਅੰਕੜੇ_ਮਾਪ, &ਚੌੜਾਈ, &ਉਚਾਈ);
	ਜੇ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ਵਾਪਸ false;
	}

	c->ਚੌੜਾਈ = ਚੌੜਾਈ;
	c->ਉਚਾਈ = ਉਚਾਈ;
	c->ਮਾਪ = c->ਚੌੜਾਈ * c->ਉਚਾਈ * 4;

	image_cache_ਜੋੜੋ(c, ਨਿਰਾ, webp_cache_convert);

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);

	ਵਾਪਸ true;
}

/**
 * Clone content.
 */
ਸਥਿਰ nserror webp_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **new_c)
{
	ਬਣਤਰ content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ content));
	ਜੇ (webp_c == ਨਿਰਾ) {
		ਵਾਪਸ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ਜੇ (res != NSERROR_OK) {
		content_destroy(webp_c);
		ਵਾਪਸ res;
	}

	/* re-convert if the content is ready */
	ਜੇ ((old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) ||
	    (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		ਜੇ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਅਚਲ content_handler webp_content_handler = {
	.create = webp_create,
	.ਅੰਕੜੇ_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_ਅੰਦਰੂਨੀ = image_cache_get_ਅੰਦਰੂਨੀ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nswebp, webp_types, webp_content_handler);
