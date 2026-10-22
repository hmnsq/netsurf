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
ساکن nserror
webp_create(ثابت content_handler *handler,
	      lwc_string *imime_type,
	      ثابت ساخت http_parameter *params,
	      llcache_handle *llcache,
	      ثابت حرف *fallback_charset,
	      bool quirks,
	      ساخت content **c)
{
	ساخت content *webp_c; /* webp content object */
	nserror res;

	webp_c = صفر_حافظہ_دو(1, حجم(ساخت content));
	اگر (webp_c == تہی) {
		واپس NSERROR_NOMEM;
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
		واپس res;
	}

	*c = webp_c;

	واپس NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ساکن ساخت bitmap *
webp_cache_convert(ساخت content *c)
{
	ثابت uint8_t *ماخذ_ڈیٹا; /* webp source data */
	size_t ماخذ_سائز; /* length of webp source data */
	VP8_حالت_رمز webpres;
	WebPBitstreamFeatures webpfeatures;
	غیرعلامتی صحیح bmap_flags;
	uint8_t *pixels = تہی;
	uint8_t *decoded;
	size_t rowstride;
	ساخت bitmap *bitmap = تہی;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	ماخذ_ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &ماخذ_سائز);

	webpres = WebPGetFeatures(ماخذ_ڈیٹا, ماخذ_سائز, &webpfeatures);

	اگر (webpres != VP8_حالت_OK) {
		واپس تہی;
	}

	اگر (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} ورنہ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.چوڑائی,
				      webpfeatures.اونچائی,
				      bmap_flags);
	اگر (bitmap == تہی) {
		/* empty bitmap could not be created */
		واپس تہی;
	}

	pixels = guit->bitmap->get_بفر(bitmap);
	اگر (pixels == تہی) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		واپس تہی;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	انتخاب (webp_fmt.layout) {
	طےشدہ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	حالت BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(ماخذ_ڈیٹا, ماخذ_سائز, pixels,
				rowstride * webpfeatures.اونچائی, rowstride);
		توڑو;

	حالت BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(ماخذ_ڈیٹا, ماخذ_سائز, pixels,
				rowstride * webpfeatures.اونچائی, rowstride);
		توڑو;

	حالت BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(ماخذ_ڈیٹا, ماخذ_سائز, pixels,
				rowstride * webpfeatures.اونچائی, rowstride);
		توڑو;
	}
	اگر (decoded == تہی) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		واپس تہی;
	}

	bitmap_format_مقصد_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	واپس bitmap;
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
ساکن bool webp_convert(ساخت content *c)
{
	صحیح res;
	ثابت uint8_t* ڈیٹا;
	size_t ڈیٹا_سائز;
	صحیح چوڑائی;
	صحیح اونچائی;

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &ڈیٹا_سائز);

	res = WebPGetInfo(ڈیٹا, ڈیٹا_سائز, &چوڑائی, &اونچائی);
	اگر (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		واپس false;
	}

	c->چوڑائی = چوڑائی;
	c->اونچائی = اونچائی;
	c->سائز = c->چوڑائی * c->اونچائی * 4;

	image_cache_جوڑو(c, تہی, webp_cache_convert);

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);

	واپس true;
}

/**
 * Clone content.
 */
ساکن nserror webp_clone(ثابت ساخت content *old, ساخت content **new_c)
{
	ساخت content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = صفر_حافظہ_دو(1, حجم(ساخت content));
	اگر (webp_c == تہی) {
		واپس NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	اگر (res != NSERROR_OK) {
		content_destroy(webp_c);
		واپس res;
	}

	/* re-convert if the content is ready */
	اگر ((old->حالت_2 == CONTENT_حالت_READY) ||
	    (old->حالت_2 == CONTENT_حالت_مکمل)) {
		اگر (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	واپس NSERROR_OK;
}

ساکن ثابت content_handler webp_content_handler = {
	.create = webp_create,
	.ڈیٹا_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_اندرونی = image_cache_get_اندرونی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_رجسٹر_TYPES(nswebp, webp_types, webp_content_handler);
