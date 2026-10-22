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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن <librosprite.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/nssprite.h"

تعريفنوع بنية nssprite_content {
	بنية content base;
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */

	بنية rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) ܥܒܕ { \
	rosprite_error err = x; \
	ܐܢ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		اذهب ro_sprite_error; \
	} ܐܚܪܢܐ ܐܢ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		اذهب ro_sprite_error; \
	} ܐܚܪܢܐ ܐܢ (err == ROSPRITE_OK) { \
	} ܐܚܪܢܐ { \
		اذهب ro_sprite_error; \
	} \
} ܟܡܐ(0)




ساكن nserror nssprite_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		بنية llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, حجم(nssprite_content));
	ܐܢ (sprite == عدم)
		ܦܢܐ NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ܐܢ (error != NSERROR_OK) {
		حرر(sprite);
		ܦܢܐ error;
	}

	*c = (بنية content *) sprite;

	ܦܢܐ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ساكن bool nssprite_convert(بنية content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	بنية rosprite_mem_context* ctx = عدم;

	ثابت uint8_t *data;
	size_t size;
	حرف *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	بنية rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	أكد(sprite_area->sprite_count > 0);

	بنية rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	ܐܢ (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		ܦܢܐ false;
	}
	uint32_t* imagebuf = (uint32_t *)(فراغ *)guit->bitmap->get_buffer(nssprite->bitmap);
	ܐܢ (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		ܦܢܐ false;
	}
	unsigned حرف *spritebuf = (unsigned حرف *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ܐܢ (title != عدم) {
		content__set_title(c, title);
		حرر(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ܦܢܐ true;

ro_sprite_error:
	ܐܢ (ctx != عدم) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, عدم);

	ܦܢܐ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ساكن فراغ nssprite_destroy(بنية content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ܐܢ (nssprite->sprite_area != عدم)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ܐܢ (nssprite->bitmap != عدم)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ساكن bool
nssprite_redraw(بنية content *c,
		بنية content_redraw_data *data,
		ثابت بنية rect *clip,
		ثابت بنية redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ܐܢ (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ܐܢ (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ܦܢܐ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ساكن nserror nssprite_clone(ثابت بنية content *old, بنية content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, حجم(nssprite_content));
	ܐܢ (sprite == عدم)
		ܦܢܐ NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	ܐܢ (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ܦܢܐ error;
	}

	/* Simply replay convert */
	ܐܢ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ܐܢ (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ܦܢܐ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) sprite;

	ܦܢܐ NSERROR_OK;
}

ساكن فراغ *nssprite_get_internal(ثابت بنية content *c, فراغ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ܦܢܐ nssprite->bitmap;
}

ساكن content_type nssprite_content_type(فراغ)
{
	ܦܢܐ CONTENT_IMAGE;
}


ساكن bool nssprite_content_is_opaque(بنية content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ܐܢ (nssprite->bitmap != عدم) {
		ܦܢܐ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ܦܢܐ false;
}

ساكن ثابت content_handler nssprite_content_handler = {
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

ساكن ثابت حرف *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
