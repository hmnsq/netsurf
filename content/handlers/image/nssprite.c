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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ <librosprite.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/nssprite.h"

પ્રકારવ્યાખ્યા સંરચના nssprite_content {
	સંરચના content base;
	સંરચના bitmap *bitmap;	/**< Created NetSurf bitmap */

	સંરચના rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) કરો { \
	rosprite_error err = x; \
	જો (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		જાઓ ro_sprite_error; \
	} નહીંતર જો (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		જાઓ ro_sprite_error; \
	} નહીંતર જો (err == ROSPRITE_OK) { \
	} નહીંતર { \
		જાઓ ro_sprite_error; \
	} \
} જ્યારે(0)




સ્થિર nserror nssprite_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		સંરચના llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, કદ(nssprite_content));
	જો (sprite == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(sprite);
		પાછા error;
	}

	*c = (સંરચના content *) sprite;

	પાછા NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

સ્થિર bool nssprite_convert(સંરચના content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	સંરચના rosprite_mem_context* ctx = ખાલી;

	અચલ uint8_t *data;
	size_t size;
	અક્ષર *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	સંરચના rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	પ્રતિજ્ઞા(sprite_area->sprite_ગણતરી > 0);

	સંરચના rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	જો (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
	}
	uint32_t* imagebuf = (uint32_t *)(શૂન્ય *)guit->bitmap->get_buffer(nssprite->bitmap);
	જો (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
	}
	unsigned અક્ષર *spritebuf = (unsigned અક્ષર *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	પાછા true;

ro_sprite_error:
	જો (ctx != ખાલી) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, ખાલી);

	પાછા false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

સ્થિર શૂન્ય nssprite_destroy(સંરચના content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	જો (nssprite->sprite_area != ખાલી)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	જો (nssprite->bitmap != ખાલી)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

સ્થિર bool
nssprite_redraw(સંરચના content *c,
		સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip,
		અચલ સંરચના redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	જો (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	જો (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	પાછા (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


સ્થિર nserror nssprite_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, કદ(nssprite_content));
	જો (sprite == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	જો (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		પાછા error;
	}

	/* Simply replay convert */
	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) sprite;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય *nssprite_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	પાછા nssprite->bitmap;
}

સ્થિર content_type nssprite_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}


સ્થિર bool nssprite_content_is_opaque(સંરચના content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	જો (nssprite->bitmap != ખાલી) {
		પાછા guit->bitmap->get_opaque(nssprite->bitmap);
	}

	પાછા false;
}

સ્થિર અચલ content_handler nssprite_content_handler = {
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

સ્થિર અચલ અક્ષર *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
