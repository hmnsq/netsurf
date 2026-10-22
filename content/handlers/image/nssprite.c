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
	بنية content پایه;
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */

	بنية rosprite_area* sprite_area;
} nssprite_content;


#تعریف_کن ERRCHK(x) افعل { \
	rosprite_error err = x; \
	إذا (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		اذهب ro_sprite_error; \
	} وإلا إذا (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		اذهب ro_sprite_error; \
	} وإلا إذا (err == ROSPRITE_OK) { \
	} وإلا { \
		اذهب ro_sprite_error; \
	} \
} طالما(0)




ساكن nserror nssprite_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		بنية llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = حافظه_صفر_بده(1, حجم(nssprite_content));
	إذا (sprite == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&sprite->پایه, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(sprite);
		أرجع error;
	}

	*c = (بنية content *) sprite;

	أرجع NSERROR_OK;
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

	ثابت uint8_t *داده;
	size_t اندازه;
	حرف *title;

	داده = content_get_منبع_داده_2(c, &اندازه);

	ERRCHK(rosprite_create_mem_context((uint8_t *) داده, اندازه, &ctx));

	بنية rosprite_area* sprite_area;
	ERRCHK(rosprite_بارگیری(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	أكد(sprite_area->sprite_شمار > 0);

	بنية rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->عرض, sprite->ارتفاع, BITMAP_NONE);
	إذا (!nssprite->bitmap) {
		content_پخش_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}
	uint32_t* imagebuf = (uint32_t *)(فراغ *)guit->bitmap->get_میانگیر(nssprite->bitmap);
	إذا (!imagebuf) {
		content_پخش_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}
	بیعلامت حرف *spritebuf = (بیعلامت حرف *)sprite->image;

	کپی_حافظه(imagebuf, spritebuf, sprite->عرض * sprite->ارتفاع * 4);

	c->عرض = sprite->عرض;
	c->ارتفاع = sprite->ارتفاع;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->عرض, c->ارتفاع);
	إذا (title != عدم) {
		content_مجموعه_title(c, title);
		حرر(title);
	}

	bitmap_format_مقصد_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);
	content_مجموعه_وضعیت(c, ""); /* Done: update status bar */

	أرجع true;

ro_sprite_error:
	إذا (ctx != عدم) {
		rosprite_destroy_mem_context(ctx);
	}
	content_پخش_error(c, NSERROR_SPRITE_ERROR, عدم);

	أرجع false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ساكن فراغ nssprite_destroy(بنية content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	إذا (nssprite->sprite_area != عدم)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	إذا (nssprite->bitmap != عدم)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ساكن bool
nssprite_redraw(بنية content *c,
		بنية content_redraw_داده *داده,
		ثابت بنية rect *clip,
		ثابت بنية redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	إذا (داده->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	إذا (داده->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	أرجع (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  داده->x, داده->y,
				  داده->عرض, داده->ارتفاع,
				  داده->background_رنگ,
				  flags) == NSERROR_OK);
}


ساكن nserror nssprite_clone(ثابت بنية content *old, بنية content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = حافظه_صفر_بده(1, حجم(nssprite_content));
	إذا (sprite == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &sprite->پایه);
	إذا (error != NSERROR_OK) {
		content_destroy(&sprite->پایه);
		أرجع error;
	}

	/* Simply replay convert */
	إذا (old->وضعیت == CONTENT_وضعیت_READY ||
			old->وضعیت == CONTENT_وضعیت_انجامشده) {
		إذا (nssprite_convert(&sprite->پایه) == false) {
			content_destroy(&sprite->پایه);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) sprite;

	أرجع NSERROR_OK;
}

ساكن فراغ *nssprite_get_داخلی(ثابت بنية content *c, فراغ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	أرجع nssprite->bitmap;
}

ساكن content_type nssprite_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}


ساكن bool nssprite_content_is_opaque(بنية content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	إذا (nssprite->bitmap != عدم) {
		أرجع guit->bitmap->get_opaque(nssprite->bitmap);
	}

	أرجع false;
}

ساكن ثابت content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.داده_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_داخلی = nssprite_get_داخلی,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_ثبات_TYPES(nssprite, nssprite_types, nssprite_content_handler);
