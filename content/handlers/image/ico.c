/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * implementation for image/ico content handler
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libnsbmp.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image.h"
#ಅನ್ತರ್ಭಾವ "image/ico.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nsico_content {
	ರಚನೆ content ತಳ;

	ರಚನೆ ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಶೂನ್ಯ *nsico_bitmap_create(ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bmp_state)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_ಸ್ಮರಣೆ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ಹಿಂತಿರುಗಿ guit->bitmap->create(ಅಗಲ, ಎತ್ತರ, bitmap_state);
}

ಸ್ಥಿರ nserror nsico_create_ico_ದತ್ತಾಂಶ(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_ಬಫರ್ = guit->bitmap->get_ಬಫರ್,
	};

	c->ico = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ico_collection));
	ಆದರೆ (c->ico == ಖಾಲಿ) {
		content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ nserror nsico_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks, ರಚನೆ content **c)
{
	nsico_content *ಫಲಿತಾಂಶ;
	nserror error;

	ಫಲಿತಾಂಶ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nsico_content));
	ಆದರೆ (ಫಲಿತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&ಫಲಿತಾಂಶ->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ error;
	}

	error = nsico_create_ico_ದತ್ತಾಂಶ(ಫಲಿತಾಂಶ);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(ಫಲಿತಾಂಶ);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) ಫಲಿತಾಂಶ;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}



ಸ್ಥಿರ bool nsico_convert(ರಚನೆ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ರಚನೆ bmp_image *bmp;
	bmp_ಫಲಿತಾಂಶ res;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *title;

	/* set the ico data */
	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಗಾತ್ರ_2);

	/* analyse the ico */
	res = ico_analyse(ico->ico, ಗಾತ್ರ_2, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) ದತ್ತಾಂಶ);

	ಆಯ್ಕೆ (res) {
	ಸ್ಥಿತಿ BMP_OK:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ BMP_INSUFFICIENT_ಸ್ಮರಣೆ:
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	ಸ್ಥಿತಿ BMP_INSUFFICIENT_ದತ್ತಾಂಶ:
	ಸ್ಥಿತಿ BMP_ದತ್ತಾಂಶ_ERROR:
		content_ಪ್ರಸಾರ_error(c, NSERROR_ICO_ERROR, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	/* Store our content width, height and calculate size */
	c->ಅಗಲ = ico->ico->ಅಗಲ;
	c->ಎತ್ತರ = ico->ico->ಎತ್ತರ;
	c->ಗಾತ್ರ_2 += (ico->ico->ಅಗಲ * ico->ico->ಎತ್ತರ * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_ಎಲೆ(llcache_handle_get_url(c->llcache)),
			c->ಅಗಲ, c->ಎತ್ತರ);
	ಆದರೆ (title != ಖಾಲಿ) {
		content_ಗಣ_title(c, title);
		ಬಿಡುಗಡೆ(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_ಕಂಡುಹಿಡಿ(ico->ico, 255, 255);
	ಆದರೆ (bmp == ಖಾಲಿ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಹಿಂತಿರುಗಿ false;
	}

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);

	/* Done: update status bar */
	content_ಗಣ_ಸ್ಥಿತಿ(c, "");
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool nsico__decode(ರಚನೆ bmp_image *ico)
{
	ಆದರೆ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ಆದರೆ (bmp_decode(ico) != BMP_OK) {
			ಹಿಂತಿರುಗಿ false;
		}

		bitmap_format_ಗಮ್ಯ_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool nsico_redraw(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ರಚನೆ bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_ಕಂಡುಹಿಡಿ(ico->ico, ದತ್ತಾಂಶ->ಅಗಲ, ದತ್ತಾಂಶ->ಎತ್ತರ);
	ಆದರೆ (bmp == ಖಾಲಿ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಹಿಂತಿರುಗಿ false;
	}

	/* ensure its decided */
	ಆದರೆ (!nsico__decode(bmp)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ image_bitmap_plot(bmp->bitmap, ದತ್ತಾಂಶ, clip, ctx);
}


ಸ್ಥಿರ ಶೂನ್ಯ nsico_destroy(ರಚನೆ content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	ಬಿಡುಗಡೆ(ico->ico);
}

ಸ್ಥಿರ nserror nsico_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nsico_content));
	ಆದರೆ (ico == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &ico->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&ico->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_ದತ್ತಾಂಶ(ico);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&ico->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
			old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (nsico_convert(&ico->ತಳ) == false) {
			content_destroy(&ico->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *) ico;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nsico_get_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ರಚನೆ bmp_image *bmp;

	bmp = ico_ಕಂಡುಹಿಡಿ(ico->ico, 16, 16);
	ಆದರೆ (bmp == ಖಾಲಿ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (!nsico__decode(bmp)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ bmp->bitmap;
}

ಸ್ಥಿರ content_type nsico_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_IMAGE;
}

ಸ್ಥಿರ bool nsico_is_opaque(ರಚನೆ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ರಚನೆ bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_ಕಂಡುಹಿಡಿ(ico->ico, 16, 16);
	ಆದರೆ (bmp == ಖಾಲಿ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!nsico__decode(bmp)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ guit->bitmap->get_opaque(bmp->bitmap);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler nsico_content_handler = {
	.create = nsico_create,
	.ದತ್ತಾಂಶ_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_ಆಂತರಿಕ = nsico_get_ಆಂತರಿಕ,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nsico, nsico_types, nsico_content_handler);
