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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <librosprite.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/nssprite.h"

typedef sa_nracanaa nssprite_content {
	sa_nracanaa content base;
	sa_nracanaa bitmap *bitmap;	/**< Created NetSurf bitmap */

	sa_nracanaa rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) do { \
	rosprite_error err = x; \
	ᮚᮦᮔ᮪ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		goto ro_sprite_error; \
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		goto ro_sprite_error; \
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (err == ROSPRITE_OK) { \
	} ᮜᮤᮚ { \
		goto ro_sprite_error; \
	} \
} ᮞᮨᮓᮨᮜ(0)




static nserror nssprite_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		sa_nracanaa llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, parimaa_na(nssprite_content));
	ᮚᮦᮔ᮪ (sprite == NULL)
		ᮝᮜᮤ NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ᮚᮦᮔ᮪ (error != NSERROR_OK) {
		free(sprite);
		ᮝᮜᮤ error;
	}

	*c = (sa_nracanaa content *) sprite;

	ᮝᮜᮤ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

static bool nssprite_convert(sa_nracanaa content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	sa_nracanaa rosprite_mem_context* ctx = NULL;

	const uint8_t *data;
	size_t size;
	var_na *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	sa_nracanaa rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	assert(sprite_area->sprite_count > 0);

	sa_nracanaa rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	ᮚᮦᮔ᮪ (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ᮝᮜᮤ false;
	}
	uint32_t* imagebuf = (uint32_t *)(void *)guit->bitmap->get_buffer(nssprite->bitmap);
	ᮚᮦᮔ᮪ (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ᮝᮜᮤ false;
	}
	unsigned var_na *spritebuf = (unsigned var_na *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ᮚᮦᮔ᮪ (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ᮝᮜᮤ true;

ro_sprite_error:
	ᮚᮦᮔ᮪ (ctx != NULL) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, NULL);

	ᮝᮜᮤ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

static void nssprite_destroy(sa_nracanaa content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ᮚᮦᮔ᮪ (nssprite->sprite_area != NULL)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ᮚᮦᮔ᮪ (nssprite->bitmap != NULL)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

static bool
nssprite_redraw(sa_nracanaa content *c,
		sa_nracanaa content_redraw_data *data,
		const sa_nracanaa rect *clip,
		const sa_nracanaa redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ᮚᮦᮔ᮪ (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ᮚᮦᮔ᮪ (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ᮝᮜᮤ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


static nserror nssprite_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, parimaa_na(nssprite_content));
	ᮚᮦᮔ᮪ (sprite == NULL)
		ᮝᮜᮤ NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	ᮚᮦᮔ᮪ (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ᮝᮜᮤ error;
	}

	/* Simply replay convert */
	ᮚᮦᮔ᮪ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ᮚᮦᮔ᮪ (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ᮝᮜᮤ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) sprite;

	ᮝᮜᮤ NSERROR_OK;
}

static void *nssprite_get_internal(const sa_nracanaa content *c, void *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ᮝᮜᮤ nssprite->bitmap;
}

static content_type nssprite_content_type(void)
{
	ᮝᮜᮤ CONTENT_IMAGE;
}


static bool nssprite_content_is_opaque(sa_nracanaa content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ᮚᮦᮔ᮪ (nssprite->bitmap != NULL) {
		ᮝᮜᮤ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ᮝᮜᮤ false;
}

static const content_handler nssprite_content_handler = {
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

static const var_na *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
