/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Sean Fox <dyntryx@gmail.com>
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
 * implementation of content handler for BMP images.
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libnsbmp.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/bmp.h"

/** bmp context. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nsbmp_content {
	ರಚನೆ content ತಳ;

	bmp_image *bmp;	/** BMP image data */

	ರಚನೆ bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಶೂನ್ಯ *nsbmp_bitmap_create(ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bmp_state)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_ಸ್ಮರಣೆ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ಹಿಂತಿರುಗಿ guit->bitmap->create(ಅಗಲ, ಎತ್ತರ, bitmap_state);
}

ಸ್ಥಿರ nserror nsbmp_create_bmp_ದತ್ತಾಂಶ(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_ಬಫರ್ = guit->bitmap->get_ಬಫರ್,
	};

	bmp->bmp = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ bmp_image));
	ಆದರೆ (bmp->bmp == ಖಾಲಿ) {
		content_ಪ್ರಸಾರ_error(&bmp->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ nserror
nsbmp_create(ಸ್ಥಿರಾಂಕ ರಚನೆ content_handler *handler,
	     lwc_string *imime_type,
	     ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
	     llcache_handle *llcache,
	     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	     bool quirks,
	     ರಚನೆ content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nsbmp_content));
	ಆದರೆ (bmp == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&bmp->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(bmp);
		ಹಿಂತಿರುಗಿ error;
	}

	error = nsbmp_create_bmp_ದತ್ತಾಂಶ(bmp);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(bmp);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) bmp;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ bool nsbmp_convert(ರಚನೆ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_ಫಲಿತಾಂಶ res;
	uint32_t swidth;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *title;

	/* set the bmp data */
	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಗಾತ್ರ_2);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, ಗಾತ್ರ_2, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ದತ್ತಾಂಶ);
	ಆಯ್ಕೆ (res) {
		ಸ್ಥಿತಿ BMP_OK:
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ BMP_INSUFFICIENT_ಸ್ಮರಣೆ:
			content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
			ಹಿಂತಿರುಗಿ false;
		ಸ್ಥಿತಿ BMP_INSUFFICIENT_ದತ್ತಾಂಶ:
		ಸ್ಥಿತಿ BMP_ದತ್ತಾಂಶ_ERROR:
			content_ಪ್ರಸಾರ_error(c, NSERROR_BMP_ERROR, ಖಾಲಿ);
			ಹಿಂತಿರುಗಿ false;
	}

	/* Store our content width and description */
	c->ಅಗಲ = bmp->bmp->ಅಗಲ;
	c->ಎತ್ತರ = bmp->bmp->ಎತ್ತರ;
	swidth = ಗಾತ್ರ(uint32_t) * bmp->bmp->ಅಗಲ;
	c->ಗಾತ್ರ_2 += (swidth * bmp->bmp->ಎತ್ತರ) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_ಎಲೆ(llcache_handle_get_url(c->llcache)),
			c->ಅಗಲ, c->ಎತ್ತರ);
	ಆದರೆ (title != ಖಾಲಿ) {
		content_ಗಣ_title(c, title);
		ಬಿಡುಗಡೆ(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);

	/* Done: update status bar */
	content_ಗಣ_ಸ್ಥಿತಿ(c, "");
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool nsbmp_redraw(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಆದರೆ (bmp->bmp->decoded == false) {
		bmp_ಫಲಿತಾಂಶ res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ಆದರೆ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_ದತ್ತಾಂಶ) &&
		    (res != BMP_ದತ್ತಾಂಶ_ERROR)) {
			ಹಿಂತಿರುಗಿ false;
		}

		bitmap_format_ಗಮ್ಯ_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ಆದರೆ (ದತ್ತಾಂಶ->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ಆದರೆ (ದತ್ತಾಂಶ->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ಹಿಂತಿರುಗಿ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  ದತ್ತಾಂಶ->x, ದತ್ತಾಂಶ->y,
				  ದತ್ತಾಂಶ->ಅಗಲ, ದತ್ತಾಂಶ->ಎತ್ತರ,
				  ದತ್ತಾಂಶ->background_ಬಣ್ಣ,
				  flags) == NSERROR_OK);
}


ಸ್ಥಿರ ಶೂನ್ಯ nsbmp_destroy(ರಚನೆ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	ಬಿಡುಗಡೆ(bmp->bmp);
}


ಸ್ಥಿರ nserror nsbmp_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nsbmp_content));
	ಆದರೆ (new_bmp == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&new_bmp->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_ದತ್ತಾಂಶ(new_bmp);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&new_bmp->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY || 
			old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (nsbmp_convert(&new_bmp->ತಳ) == false) {
			content_destroy(&new_bmp->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *) new_bmp;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nsbmp_get_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ಹಿಂತಿರುಗಿ bmp->bitmap;
}

ಸ್ಥಿರ content_type nsbmp_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_IMAGE;
}

ಸ್ಥಿರ bool nsbmp_content_is_opaque(ರಚನೆ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ಆದರೆ (bmp->bitmap != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ guit->bitmap->get_opaque(bmp->bitmap);
	}

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.ದತ್ತಾಂಶ_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_ಆಂತರಿಕ = nsbmp_get_ಆಂತರಿಕ,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nsbmp_types[] = {
	"application/bmp",
	"application/preview",
	"application/x-bmp",
	"application/x-win-bitmap",
	"image/bmp",
	"image/ms-bmp",
	"image/x-bitmap",
	"image/x-bmp",
	"image/x-ms-bmp",
	"image/x-win-bitmap",
	"image/x-windows-bmp",
	"image/x-xbitmap"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
