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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ <librosprite.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/nssprite.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nssprite_content {
	ସଂରଚନା content base;
	ସଂରଚନା bitmap *bitmap;	/**< Created NetSurf bitmap */

	ସଂରଚନା rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) କର { \
	rosprite_error err = x; \
	ଯଦି (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		ଯାଅ ro_sprite_error; \
	} ଅନ୍ୟଥା ଯଦି (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		ଯାଅ ro_sprite_error; \
	} ଅନ୍ୟଥା ଯଦି (err == ROSPRITE_OK) { \
	} ଅନ୍ୟଥା { \
		ଯାଅ ro_sprite_error; \
	} \
} ଯେତେବେଳେ(0)




ସ୍ଥିର nserror nssprite_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		ସଂରଚନା llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, ଆକାର(nssprite_content));
	ଯଦି (sprite == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(sprite);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) sprite;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ସ୍ଥିର bool nssprite_convert(ସଂରଚନା content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ସଂରଚନା rosprite_mem_context* ctx = ଖାଲି;

	ଅଚଳ uint8_t *data;
	size_t size;
	ଅକ୍ଷର *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	ସଂରଚନା rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ପ୍ରତିଜ୍ଞା(sprite_area->sprite_ଗଣନା > 0);

	ସଂରଚନା rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	ଯଦି (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}
	uint32_t* imagebuf = (uint32_t *)(ଶୂନ୍ୟ *)guit->bitmap->get_buffer(nssprite->bitmap);
	ଯଦି (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}
	unsigned ଅକ୍ଷର *spritebuf = (unsigned ଅକ୍ଷର *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ଫେରନ୍ତୁ true;

ro_sprite_error:
	ଯଦି (ctx != ଖାଲି) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, ଖାଲି);

	ଫେରନ୍ତୁ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ସ୍ଥିର ଶୂନ୍ୟ nssprite_destroy(ସଂରଚନା content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ଯଦି (nssprite->sprite_area != ଖାଲି)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ଯଦି (nssprite->bitmap != ଖାଲି)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ସ୍ଥିର bool
nssprite_redraw(ସଂରଚନା content *c,
		ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ଯଦି (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ଯଦି (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ଫେରନ୍ତୁ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ସ୍ଥିର nserror nssprite_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, ଆକାର(nssprite_content));
	ଯଦି (sprite == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay convert */
	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) sprite;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ *nssprite_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ଫେରନ୍ତୁ nssprite->bitmap;
}

ସ୍ଥିର content_type nssprite_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}


ସ୍ଥିର bool nssprite_content_is_opaque(ସଂରଚନା content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ଯଦି (nssprite->bitmap != ଖାଲି) {
		ଫେରନ୍ତୁ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର ଅଚଳ content_handler nssprite_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
