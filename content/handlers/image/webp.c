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
webp_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
	      lwc_string *imime_type,
	      ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
	      llcache_handle *llcache,
	      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	      bool quirks,
	      ರಚನೆ content **c)
{
	ರಚನೆ content *webp_c; /* webp content object */
	nserror res;

	webp_c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content));
	ಆದರೆ (webp_c == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ಆದರೆ (res != NSERROR_OK) {
		ಬಿಡುಗಡೆ(webp_c);
		ಹಿಂತಿರುಗಿ res;
	}

	*c = webp_c;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
ಸ್ಥಿರ ರಚನೆ bitmap *
webp_cache_convert(ರಚನೆ content *c)
{
	ಸ್ಥಿರಾಂಕ uint8_t *ಮೂಲ_ದತ್ತಾಂಶ; /* webp source data */
	size_t ಮೂಲ_ಗಾತ್ರ; /* length of webp source data */
	VP8_ಸ್ಥಿತಿ_ಸಂಕೇತ webpres;
	WebPBitstreamFeatures webpfeatures;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bmap_flags;
	uint8_t *pixels = ಖಾಲಿ;
	uint8_t *decoded;
	size_t rowstride;
	ರಚನೆ bitmap *bitmap = ಖಾಲಿ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	ಮೂಲ_ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಮೂಲ_ಗಾತ್ರ);

	webpres = WebPGetFeatures(ಮೂಲ_ದತ್ತಾಂಶ, ಮೂಲ_ಗಾತ್ರ, &webpfeatures);

	ಆದರೆ (webpres != VP8_ಸ್ಥಿತಿ_OK) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} ಇಲ್ಲದಿದ್ದರೆ {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.ಅಗಲ,
				      webpfeatures.ಎತ್ತರ,
				      bmap_flags);
	ಆದರೆ (bitmap == ಖಾಲಿ) {
		/* empty bitmap could not be created */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	pixels = guit->bitmap->get_ಬಫರ್(bitmap);
	ಆದರೆ (pixels == ಖಾಲಿ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	ಆಯ್ಕೆ (webp_fmt.layout) {
	ಪೂರ್ವನಿಯೋಜಿತ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	ಸ್ಥಿತಿ BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(ಮೂಲ_ದತ್ತಾಂಶ, ಮೂಲ_ಗಾತ್ರ, pixels,
				rowstride * webpfeatures.ಎತ್ತರ, rowstride);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(ಮೂಲ_ದತ್ತಾಂಶ, ಮೂಲ_ಗಾತ್ರ, pixels,
				rowstride * webpfeatures.ಎತ್ತರ, rowstride);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(ಮೂಲ_ದತ್ತಾಂಶ, ಮೂಲ_ಗಾತ್ರ, pixels,
				rowstride * webpfeatures.ಎತ್ತರ, rowstride);
		ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (decoded == ಖಾಲಿ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	bitmap_format_ಗಮ್ಯ_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	ಹಿಂತಿರುಗಿ bitmap;
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
ಸ್ಥಿರ bool webp_convert(ರಚನೆ content *c)
{
	ಪೂರ್ಣಾಂಕ res;
	ಸ್ಥಿರಾಂಕ uint8_t* ದತ್ತಾಂಶ;
	size_t ದತ್ತಾಂಶ_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ;

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ದತ್ತಾಂಶ_ಗಾತ್ರ);

	res = WebPGetInfo(ದತ್ತಾಂಶ, ದತ್ತಾಂಶ_ಗಾತ್ರ, &ಅಗಲ, &ಎತ್ತರ);
	ಆದರೆ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		ಹಿಂತಿರುಗಿ false;
	}

	c->ಅಗಲ = ಅಗಲ;
	c->ಎತ್ತರ = ಎತ್ತರ;
	c->ಗಾತ್ರ_2 = c->ಅಗಲ * c->ಎತ್ತರ * 4;

	image_cache_ಕೂಡಿಸು(c, ಖಾಲಿ, webp_cache_convert);

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);

	ಹಿಂತಿರುಗಿ true;
}

/**
 * Clone content.
 */
ಸ್ಥಿರ nserror webp_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **new_c)
{
	ರಚನೆ content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ content));
	ಆದರೆ (webp_c == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ಆದರೆ (res != NSERROR_OK) {
		content_destroy(webp_c);
		ಹಿಂತಿರುಗಿ res;
	}

	/* re-convert if the content is ready */
	ಆದರೆ ((old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) ||
	    (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		ಆದರೆ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler webp_content_handler = {
	.create = webp_create,
	.ದತ್ತಾಂಶ_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_ಆಂತರಿಕ = image_cache_get_ಆಂತರಿಕ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nswebp, webp_types, webp_content_handler);
