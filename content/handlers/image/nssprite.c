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

قسمتعریف ساخت nssprite_content {
	ساخت content بنیاد;
	ساخت bitmap *bitmap;	/**< Created NetSurf bitmap */

	ساخت rosprite_area* sprite_area;
} nssprite_content;


#تعریف_کرو ERRCHK(x) کرو { \
	rosprite_error err = x; \
	اگر (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		جاؤ ro_sprite_error; \
	} ورنہ اگر (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		جاؤ ro_sprite_error; \
	} ورنہ اگر (err == ROSPRITE_OK) { \
	} ورنہ { \
		جاؤ ro_sprite_error; \
	} \
} جب(0)




ساکن nserror nssprite_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		ساخت llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, ساخت content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = صفر_حافظہ_دو(1, حجم(nssprite_content));
	اگر (sprite == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&sprite->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(sprite);
		واپس error;
	}

	*c = (ساخت content *) sprite;

	واپس NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ساکن bool nssprite_convert(ساخت content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ساخت rosprite_mem_context* ctx = تہی;

	ثابت uint8_t *ڈیٹا;
	size_t سائز;
	حرف *title;

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &سائز);

	ERRCHK(rosprite_create_mem_context((uint8_t *) ڈیٹا, سائز, &ctx));

	ساخت rosprite_area* sprite_area;
	ERRCHK(rosprite_بھرو(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	أكد(sprite_area->sprite_گنتی > 0);

	ساخت rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->چوڑائی, sprite->اونچائی, BITMAP_NONE);
	اگر (!nssprite->bitmap) {
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	}
	uint32_t* imagebuf = (uint32_t *)(خالی *)guit->bitmap->get_بفر(nssprite->bitmap);
	اگر (!imagebuf) {
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	}
	غیرعلامتی حرف *spritebuf = (غیرعلامتی حرف *)sprite->image;

	حافظہ_نقل(imagebuf, spritebuf, sprite->چوڑائی * sprite->اونچائی * 4);

	c->چوڑائی = sprite->چوڑائی;
	c->اونچائی = sprite->اونچائی;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اونچائی);
	اگر (title != تہی) {
		content_سیٹ_title(c, title);
		آزاد(title);
	}

	bitmap_format_مقصد_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
	content_سیٹ_حالت(c, ""); /* Done: update status bar */

	واپس true;

ro_sprite_error:
	اگر (ctx != تہی) {
		rosprite_destroy_mem_context(ctx);
	}
	content_نشر_error(c, NSERROR_SPRITE_ERROR, تہی);

	واپس false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ساکن خالی nssprite_destroy(ساخت content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	اگر (nssprite->sprite_area != تہی)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	اگر (nssprite->bitmap != تہی)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ساکن bool
nssprite_redraw(ساخت content *c,
		ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip,
		ثابت ساخت redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	اگر (ڈیٹا->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	اگر (ڈیٹا->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	واپس (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  ڈیٹا->x, ڈیٹا->y,
				  ڈیٹا->چوڑائی, ڈیٹا->اونچائی,
				  ڈیٹا->background_رنگ,
				  flags) == NSERROR_OK);
}


ساکن nserror nssprite_clone(ثابت ساخت content *old, ساخت content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = صفر_حافظہ_دو(1, حجم(nssprite_content));
	اگر (sprite == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &sprite->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&sprite->بنیاد);
		واپس error;
	}

	/* Simply replay convert */
	اگر (old->حالت_2 == CONTENT_حالت_READY ||
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (nssprite_convert(&sprite->بنیاد) == false) {
			content_destroy(&sprite->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) sprite;

	واپس NSERROR_OK;
}

ساکن خالی *nssprite_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	واپس nssprite->bitmap;
}

ساکن content_type nssprite_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}


ساکن bool nssprite_content_is_opaque(ساخت content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	اگر (nssprite->bitmap != تہی) {
		واپس guit->bitmap->get_opaque(nssprite->bitmap);
	}

	واپس false;
}

ساکن ثابت content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.ڈیٹا_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_اندرونی = nssprite_get_اندرونی,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_رجسٹر_TYPES(nssprite, nssprite_types, nssprite_content_handler);
