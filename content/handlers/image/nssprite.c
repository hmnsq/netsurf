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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব <librosprite.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/nssprite.h"

প্রকারলেখ কাঠামো nssprite_content {
	কাঠামো content base;
	কাঠামো bitmap *bitmap;	/**< Created NetSurf bitmap */

	কাঠামো rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) করো { \
	rosprite_error err = x; \
	যদি (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		যাও ro_sprite_error; \
	} অন্যথা যদি (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		যাও ro_sprite_error; \
	} অন্যথা যদি (err == ROSPRITE_OK) { \
	} অন্যথা { \
		যাও ro_sprite_error; \
	} \
} যখন(0)




স্থির nserror nssprite_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		কাঠামো llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks, কাঠামো content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, আকার(nssprite_content));
	যদি (sprite == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(sprite);
		ফেরত error;
	}

	*c = (কাঠামো content *) sprite;

	ফেরত NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

স্থির bool nssprite_convert(কাঠামো content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	কাঠামো rosprite_mem_context* ctx = ফাঁকা;

	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;
	অক্ষর *title;

	তথ্য = content_get_source_তথ্য_2(c, &আকার_2);

	ERRCHK(rosprite_create_mem_context((uint8_t *) তথ্য, আকার_2, &ctx));

	কাঠামো rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	প্রতিজ্ঞা(sprite_area->sprite_গুনতি > 0);

	কাঠামো rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	যদি (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
	}
	uint32_t* imagebuf = (uint32_t *)(শূন্য *)guit->bitmap->get_বাফার(nssprite->bitmap);
	যদি (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
	}
	unsigned অক্ষর *spritebuf = (unsigned অক্ষর *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != ফাঁকা) {
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

	ফেরত true;

ro_sprite_error:
	যদি (ctx != ফাঁকা) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, ফাঁকা);

	ফেরত false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

স্থির শূন্য nssprite_destroy(কাঠামো content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	যদি (nssprite->sprite_area != ফাঁকা)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	যদি (nssprite->bitmap != ফাঁকা)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

স্থির bool
nssprite_redraw(কাঠামো content *c,
		কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	যদি (তথ্য->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	যদি (তথ্য->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ফেরত (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  তথ্য->x, তথ্য->y,
				  তথ্য->width, তথ্য->height,
				  তথ্য->background_colour,
				  flags) == NSERROR_OK);
}


স্থির nserror nssprite_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, আকার(nssprite_content));
	যদি (sprite == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		ফেরত error;
	}

	/* Simply replay convert */
	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*newc = (কাঠামো content *) sprite;

	ফেরত NSERROR_OK;
}

স্থির শূন্য *nssprite_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ফেরত nssprite->bitmap;
}

স্থির content_type nssprite_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}


স্থির bool nssprite_content_is_opaque(কাঠামো content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	যদি (nssprite->bitmap != ফাঁকা) {
		ফেরত guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ফেরত false;
}

স্থির ধ্রুবক content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.তথ্য_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_internal = nssprite_get_internal,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
