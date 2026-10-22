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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nssprite_content {
	ಸಂರಚನಾ content base;
	ಸಂರಚನಾ bitmap *bitmap;	/**< Created NetSurf bitmap */

	ಸಂರಚನಾ rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) ಕುರು { \
	rosprite_error err = x; \
	ಯದಿ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		ಗಚ್ಛ ro_sprite_error; \
	} ಅನ್ಯಥಾ ಯದಿ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		ಗಚ್ಛ ro_sprite_error; \
	} ಅನ್ಯಥಾ ಯದಿ (err == ROSPRITE_OK) { \
	} ಅನ್ಯಥಾ { \
		ಗಚ್ಛ ro_sprite_error; \
	} \
} ಯಾವತ್(0)




ಸ್ಥಿರ nserror nssprite_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		ಸಂರಚನಾ llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, ಪರಿಮಾಣ(nssprite_content));
	ಯದಿ (sprite == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(sprite);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) sprite;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ಸ್ಥಿರ bool nssprite_convert(ಸಂರಚನಾ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಸಂರಚನಾ rosprite_mem_context* ctx = ರಿಕ್ತ;

	ಧ್ರುವ uint8_t *data;
	size_t size;
	ವರ್ಣ *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	ಸಂರಚನಾ rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ಪ್ರತಿಜ್ಞಾ(sprite_area->sprite_ಎಣಿಕೆ > 0);

	ಸಂರಚನಾ rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	ಯದಿ (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	uint32_t* imagebuf = (uint32_t *)(ಶೂನ್ಯ *)guit->bitmap->get_buffer(nssprite->bitmap);
	ಯದಿ (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	unsigned ವರ್ಣ *spritebuf = (unsigned ವರ್ಣ *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ಪ್ರತ್ಯಾಗಮ true;

ro_sprite_error:
	ಯದಿ (ctx != ರಿಕ್ತ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ಸ್ಥಿರ ಶೂನ್ಯ nssprite_destroy(ಸಂರಚನಾ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಯದಿ (nssprite->sprite_area != ರಿಕ್ತ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ಯದಿ (nssprite->bitmap != ರಿಕ್ತ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ಸ್ಥಿರ bool
nssprite_redraw(ಸಂರಚನಾ content *c,
		ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಯದಿ (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ಯದಿ (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ಪ್ರತ್ಯಾಗಮ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ಸ್ಥಿರ nserror nssprite_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, ಪರಿಮಾಣ(nssprite_content));
	ಯದಿ (sprite == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay convert */
	ಯದಿ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) sprite;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nssprite_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಪ್ರತ್ಯಾಗಮ nssprite->bitmap;
}

ಸ್ಥಿರ content_type nssprite_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}


ಸ್ಥಿರ bool nssprite_content_is_opaque(ಸಂರಚನಾ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ಯದಿ (nssprite->bitmap != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.data_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_internal = nssprite_get_internal,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
