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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ <librosprite.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/nssprite.h"

வகைவரை கட்டமைப்பு nssprite_content {
	கட்டமைப்பு content base;
	கட்டமைப்பு bitmap *bitmap;	/**< Created NetSurf bitmap */

	கட்டமைப்பு rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) செய் { \
	rosprite_error err = x; \
	எனில் (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		செல் ro_sprite_error; \
	} இல்லையெனில் எனில் (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		செல் ro_sprite_error; \
	} இல்லையெனில் எனில் (err == ROSPRITE_OK) { \
	} இல்லையெனில் { \
		செல் ro_sprite_error; \
	} \
} போது(0)




நிலையான nserror nssprite_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		கட்டமைப்பு llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, அளவு(nssprite_content));
	எனில் (sprite == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(sprite);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) sprite;

	திரும்பு NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

நிலையான bool nssprite_convert(கட்டமைப்பு content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	கட்டமைப்பு rosprite_mem_context* ctx = வெற்று;

	மாறிலி uint8_t *data;
	size_t size;
	எழுத்து *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	கட்டமைப்பு rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ப்ரதிஜ்ஞா(sprite_area->sprite_மொத்தம் > 0);

	கட்டமைப்பு rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	எனில் (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
	}
	uint32_t* imagebuf = (uint32_t *)(வெறுமை *)guit->bitmap->get_buffer(nssprite->bitmap);
	எனில் (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
	}
	unsigned எழுத்து *spritebuf = (unsigned எழுத்து *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	திரும்பு true;

ro_sprite_error:
	எனில் (ctx != வெற்று) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, வெற்று);

	திரும்பு false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

நிலையான வெறுமை nssprite_destroy(கட்டமைப்பு content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	எனில் (nssprite->sprite_area != வெற்று)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	எனில் (nssprite->bitmap != வெற்று)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

நிலையான bool
nssprite_redraw(கட்டமைப்பு content *c,
		கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	எனில் (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	எனில் (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	திரும்பு (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


நிலையான nserror nssprite_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, அளவு(nssprite_content));
	எனில் (sprite == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		திரும்பு error;
	}

	/* Simply replay convert */
	எனில் (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		எனில் (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) sprite;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை *nssprite_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	திரும்பு nssprite->bitmap;
}

நிலையான content_type nssprite_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}


நிலையான bool nssprite_content_is_opaque(கட்டமைப்பு content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	எனில் (nssprite->bitmap != வெற்று) {
		திரும்பு guit->bitmap->get_opaque(nssprite->bitmap);
	}

	திரும்பு false;
}

நிலையான மாறிலி content_handler nssprite_content_handler = {
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

நிலையான மாறிலி எழுத்து *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
