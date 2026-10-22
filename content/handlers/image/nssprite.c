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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ <librosprite.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/nssprite.h"

തരനിർവചനം ഘടന nssprite_content {
	ഘടന content base;
	ഘടന bitmap *bitmap;	/**< Created NetSurf bitmap */

	ഘടന rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) ചെയ്യുക { \
	rosprite_error err = x; \
	എങ്കിൽ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		പോകുക ro_sprite_error; \
	} അല്ലെങ്കിൽ എങ്കിൽ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		പോകുക ro_sprite_error; \
	} അല്ലെങ്കിൽ എങ്കിൽ (err == ROSPRITE_OK) { \
	} അല്ലെങ്കിൽ { \
		പോകുക ro_sprite_error; \
	} \
} ആകുമ്പോൾ(0)




സ്ഥിതം nserror nssprite_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		ഘടന llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, വലുപ്പം(nssprite_content));
	എങ്കിൽ (sprite == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(sprite);
		തിരികെ error;
	}

	*c = (ഘടന content *) sprite;

	തിരികെ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

സ്ഥിതം bool nssprite_convert(ഘടന content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ഘടന rosprite_mem_context* ctx = ശൂന്യ;

	സ്ഥിരം uint8_t *data;
	size_t size;
	അക്ഷരം *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	ഘടന rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	പ്രതിജ്ഞാ(sprite_area->sprite_എണ്ണം > 0);

	ഘടന rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	എങ്കിൽ (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
	}
	uint32_t* imagebuf = (uint32_t *)(ശൂന്യം *)guit->bitmap->get_buffer(nssprite->bitmap);
	എങ്കിൽ (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
	}
	unsigned അക്ഷരം *spritebuf = (unsigned അക്ഷരം *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	തിരികെ true;

ro_sprite_error:
	എങ്കിൽ (ctx != ശൂന്യ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, ശൂന്യ);

	തിരികെ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

സ്ഥിതം ശൂന്യം nssprite_destroy(ഘടന content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	എങ്കിൽ (nssprite->sprite_area != ശൂന്യ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	എങ്കിൽ (nssprite->bitmap != ശൂന്യ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

സ്ഥിതം bool
nssprite_redraw(ഘടന content *c,
		ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	എങ്കിൽ (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	എങ്കിൽ (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	തിരികെ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


സ്ഥിതം nserror nssprite_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, വലുപ്പം(nssprite_content));
	എങ്കിൽ (sprite == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		തിരികെ error;
	}

	/* Simply replay convert */
	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) sprite;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം *nssprite_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	തിരികെ nssprite->bitmap;
}

സ്ഥിതം content_type nssprite_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}


സ്ഥിതം bool nssprite_content_is_opaque(ഘടന content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	എങ്കിൽ (nssprite->bitmap != ശൂന്യ) {
		തിരികെ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	തിരികെ false;
}

സ്ഥിതം സ്ഥിരം content_handler nssprite_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
