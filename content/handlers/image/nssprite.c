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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ <librosprite.h>

#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/plotters.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "content/llcache.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"
#অন্তর্ভাৱ "desktop/bitmap.h"

#অন্তর্ভাৱ "image/nssprite.h"

প্রকারনির্দেশ সংরচনা nssprite_content {
	সংরচনা content base;
	সংরচনা bitmap *bitmap;	/**< Created NetSurf bitmap */

	সংরচনা rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) কৰা { \
	rosprite_error err = x; \
	যদি (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		গচ্ছ ro_sprite_error; \
	} অন্যথা যদি (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		গচ্ছ ro_sprite_error; \
	} অন্যথা যদি (err == ROSPRITE_OK) { \
	} অন্যথা { \
		গচ্ছ ro_sprite_error; \
	} \
} যেতিয়ালৈ(0)




স্থির nserror nssprite_create(ধ্রুৱ content_handler *handler,
		lwc_string *imime_type, ধ্রুৱ সংরচনা http_parameter *params,
		সংরচনা llcache_handle *llcache, ধ্রুৱ ৱর্ণ *fallback_charset,
		bool quirks, সংরচনা content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, পরিমাণ(nssprite_content));
	যদি (sprite == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(sprite);
		ঘূৰাই error;
	}

	*c = (সংরচনা content *) sprite;

	ঘূৰাই NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

স্থির bool nssprite_convert(সংরচনা content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	সংরচনা rosprite_mem_context* ctx = রিক্ত;

	ধ্রুৱ uint8_t *data;
	size_t size;
	ৱর্ণ *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	সংরচনা rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	প্রতিজ্ঞা(sprite_area->sprite_গুনতি > 0);

	সংরচনা rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	যদি (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই false;
	}
	uint32_t* imagebuf = (uint32_t *)(শূন্য *)guit->bitmap->get_buffer(nssprite->bitmap);
	যদি (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		ঘূৰাই false;
	}
	unsigned ৱর্ণ *spritebuf = (unsigned ৱর্ণ *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != রিক্ত) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ঘূৰাই true;

ro_sprite_error:
	যদি (ctx != রিক্ত) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, রিক্ত);

	ঘূৰাই false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

স্থির শূন্য nssprite_destroy(সংরচনা content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	যদি (nssprite->sprite_area != রিক্ত)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	যদি (nssprite->bitmap != রিক্ত)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

স্থির bool
nssprite_redraw(সংরচনা content *c,
		সংরচনা content_redraw_data *data,
		ধ্রুৱ সংরচনা rect *clip,
		ধ্রুৱ সংরচনা redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	যদি (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	যদি (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ঘূৰাই (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


স্থির nserror nssprite_clone(ধ্রুৱ সংরচনা content *old, সংরচনা content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, পরিমাণ(nssprite_content));
	যদি (sprite == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ঘূৰাই error;
	}

	/* Simply replay convert */
	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ঘূৰাই NSERROR_CLONE_FAILED;
		}
	}

	*newc = (সংরচনা content *) sprite;

	ঘূৰাই NSERROR_OK;
}

স্থির শূন্য *nssprite_get_internal(ধ্রুৱ সংরচনা content *c, শূন্য *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ঘূৰাই nssprite->bitmap;
}

স্থির content_type nssprite_content_type(শূন্য)
{
	ঘূৰাই CONTENT_IMAGE;
}


স্থির bool nssprite_content_is_opaque(সংরচনা content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	যদি (nssprite->bitmap != রিক্ত) {
		ঘূৰাই guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ঘূৰাই false;
}

স্থির ধ্রুৱ content_handler nssprite_content_handler = {
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

স্থির ধ্রুৱ ৱর্ণ *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
