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

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل <librosprite.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/nssprite.h"

تعریفنوع ساختار nssprite_content {
	ساختار content base;
	ساختار bitmap *bitmap;	/**< Created NetSurf bitmap */

	ساختار rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) انجامبده { \
	rosprite_error err = x; \
	اگر (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		برو ro_sprite_error; \
	} وگرنه اگر (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		برو ro_sprite_error; \
	} وگرنه اگر (err == ROSPRITE_OK) { \
	} وگرنه { \
		برو ro_sprite_error; \
	} \
} وقتی(0)




ایستا nserror nssprite_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		ساختار llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, اندازه(nssprite_content));
	اگر (sprite == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(sprite);
		بازگشت error;
	}

	*c = (ساختار content *) sprite;

	بازگشت NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ایستا bool nssprite_convert(ساختار content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ساختار rosprite_mem_context* ctx = پوچ;

	ثابت uint8_t *data;
	size_t size;
	نویسه *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	ساختار rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	أكد(sprite_area->sprite_شمار > 0);

	ساختار rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	اگر (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, پوچ);
		بازگشت false;
	}
	uint32_t* imagebuf = (uint32_t *)(تهی *)guit->bitmap->get_buffer(nssprite->bitmap);
	اگر (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, پوچ);
		بازگشت false;
	}
	unsigned نویسه *spritebuf = (unsigned نویسه *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	اگر (title != پوچ) {
		content__set_title(c, title);
		آزاد(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	بازگشت true;

ro_sprite_error:
	اگر (ctx != پوچ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, پوچ);

	بازگشت false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ایستا تهی nssprite_destroy(ساختار content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	اگر (nssprite->sprite_area != پوچ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	اگر (nssprite->bitmap != پوچ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ایستا bool
nssprite_redraw(ساختار content *c,
		ساختار content_redraw_data *data,
		ثابت ساختار rect *clip,
		ثابت ساختار redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	اگر (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	اگر (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	بازگشت (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ایستا nserror nssprite_clone(ثابت ساختار content *old, ساختار content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, اندازه(nssprite_content));
	اگر (sprite == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		بازگشت error;
	}

	/* Simply replay convert */
	اگر (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		اگر (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *) sprite;

	بازگشت NSERROR_OK;
}

ایستا تهی *nssprite_get_internal(ثابت ساختار content *c, تهی *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	بازگشت nssprite->bitmap;
}

ایستا content_type nssprite_content_type(تهی)
{
	بازگشت CONTENT_IMAGE;
}


ایستا bool nssprite_content_is_opaque(ساختار content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	اگر (nssprite->bitmap != پوچ) {
		بازگشت guit->bitmap->get_opaque(nssprite->bitmap);
	}

	بازگشت false;
}

ایستا ثابت content_handler nssprite_content_handler = {
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

ایستا ثابت نویسه *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
