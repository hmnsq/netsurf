 /*
 * Copyright 2008 James Shaw <js102@zepler.net>
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
 * librosprite implementation for content image/x-riscos-sprite
 */

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ <librosprite.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/nssprite.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nssprite_content {
	ರಚನೆ content ತಳ;
	ರಚನೆ bitmap *bitmap;	/**< Created NetSurf bitmap */

	ರಚನೆ rosprite_area* sprite_area;
} nssprite_content;


#ವ್ಯಾಖ್ಯಾನಿಸು ERRCHK(x) ಮಾಡು { \
	rosprite_error err = x; \
	ಆದರೆ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		ಹೋಗು ro_sprite_error; \
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		ಹೋಗು ro_sprite_error; \
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (err == ROSPRITE_OK) { \
	} ಇಲ್ಲದಿದ್ದರೆ { \
		ಹೋಗು ro_sprite_error; \
	} \
} ಆಗ(0)




ಸ್ಥಿರ nserror nssprite_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		ರಚನೆ llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks, ರಚನೆ content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nssprite_content));
	ಆದರೆ (sprite == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&sprite->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(sprite);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) sprite;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ಸ್ಥಿರ bool nssprite_convert(ರಚನೆ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ರಚನೆ rosprite_mem_context* ctx = ಖಾಲಿ;

	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *title;

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಗಾತ್ರ_2);

	ERRCHK(rosprite_create_mem_context((uint8_t *) ದತ್ತಾಂಶ, ಗಾತ್ರ_2, &ctx));

	ರಚನೆ rosprite_area* sprite_area;
	ERRCHK(rosprite_ತುಂಬು(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ಪ್ರತಿಜ್ಞಾ(sprite_area->sprite_ಎಣಿಕೆ > 0);

	ರಚನೆ rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->ಅಗಲ, sprite->ಎತ್ತರ, BITMAP_NONE);
	ಆದರೆ (!nssprite->bitmap) {
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}
	uint32_t* imagebuf = (uint32_t *)(ಶೂನ್ಯ *)guit->bitmap->get_ಬಫರ್(nssprite->bitmap);
	ಆದರೆ (!imagebuf) {
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *spritebuf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)sprite->image;

	ಸ್ಮರಣೆ_ನಕಲು(imagebuf, spritebuf, sprite->ಅಗಲ * sprite->ಎತ್ತರ * 4);

	c->ಅಗಲ = sprite->ಅಗಲ;
	c->ಎತ್ತರ = sprite->ಎತ್ತರ;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_ಎಲೆ(llcache_handle_get_url(c->llcache)),
			c->ಅಗಲ, c->ಎತ್ತರ);
	ಆದರೆ (title != ಖಾಲಿ) {
		content_ಗಣ_title(c, title);
		ಬಿಡುಗಡೆ(title);
	}

	bitmap_format_ಗಮ್ಯ_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
	content_ಗಣ_ಸ್ಥಿತಿ(c, ""); /* Done: update status bar */

	ಹಿಂತಿರುಗಿ true;

ro_sprite_error:
	ಆದರೆ (ctx != ಖಾಲಿ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_ಪ್ರಸಾರ_error(c, NSERROR_SPRITE_ERROR, ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ nssprite_destroy(ರಚನೆ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಆದರೆ (nssprite->sprite_area != ಖಾಲಿ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ಆದರೆ (nssprite->bitmap != ಖಾಲಿ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ಸ್ಥಿರ bool
nssprite_redraw(ರಚನೆ content *c,
		ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಆದರೆ (ದತ್ತಾಂಶ->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ಆದರೆ (ದತ್ತಾಂಶ->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ಹಿಂತಿರುಗಿ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  ದತ್ತಾಂಶ->x, ದತ್ತಾಂಶ->y,
				  ದತ್ತಾಂಶ->ಅಗಲ, ದತ್ತಾಂಶ->ಎತ್ತರ,
				  ದತ್ತಾಂಶ->background_ಬಣ್ಣ,
				  flags) == NSERROR_OK);
}


ಸ್ಥಿರ nserror nssprite_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(nssprite_content));
	ಆದರೆ (sprite == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &sprite->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&sprite->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay convert */
	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
			old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (nssprite_convert(&sprite->ತಳ) == false) {
			content_destroy(&sprite->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *) sprite;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nssprite_get_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಹಿಂತಿರುಗಿ nssprite->bitmap;
}

ಸ್ಥಿರ content_type nssprite_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_IMAGE;
}


ಸ್ಥಿರ bool nssprite_content_is_opaque(ರಚನೆ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಆದರೆ (nssprite->bitmap != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.ದತ್ತಾಂಶ_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_ಆಂತರಿಕ = nssprite_get_ಆಂತರಿಕ,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nssprite, nssprite_types, nssprite_content_handler);
