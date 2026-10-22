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
	بنية content بنياد;
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */

	بنية rosprite_area* sprite_area;
} nssprite_content;


#وصف_ڪريو ERRCHK(x) افعل { \
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

	sprite = صفر_يادگيري_ڏيو(1, حجم(nssprite_content));
	إذا (sprite == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&sprite->بنياد, handler, imime_type, params,
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

	ثابت uint8_t *معلومات;
	size_t سائيز;
	حرف *title;

	معلومات = content_get_ذريعو_معلومات_2(c, &سائيز);

	ERRCHK(rosprite_create_mem_context((uint8_t *) معلومات, سائيز, &ctx));

	بنية rosprite_area* sprite_area;
	ERRCHK(rosprite_ڀريو(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	أكد(sprite_area->sprite_ڳڻپ > 0);

	بنية rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->ويڪر, sprite->اوچائي, BITMAP_NONE);
	إذا (!nssprite->bitmap) {
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}
	uint32_t* imagebuf = (uint32_t *)(فراغ *)guit->bitmap->get_بفر(nssprite->bitmap);
	إذا (!imagebuf) {
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}
	بنانشان حرف *spritebuf = (بنانشان حرف *)sprite->image;

	يادگيري_نقل(imagebuf, spritebuf, sprite->ويڪر * sprite->اوچائي * 4);

	c->ويڪر = sprite->ويڪر;
	c->اوچائي = sprite->اوچائي;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_پن(llcache_handle_get_url(c->llcache)),
			c->ويڪر, c->اوچائي);
	إذا (title != عدم) {
		content_سيٽ_title(c, title);
		حرر(title);
	}

	bitmap_format_منزل_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_سيٽ_ready(c);
	content_سيٽ_پورو(c);
	content_سيٽ_حالت(c, ""); /* Done: update status bar */

	أرجع true;

ro_sprite_error:
	إذا (ctx != عدم) {
		rosprite_destroy_mem_context(ctx);
	}
	content_نشر_error(c, NSERROR_SPRITE_ERROR, عدم);

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
		بنية content_redraw_معلومات *معلومات,
		ثابت بنية rect *clip,
		ثابت بنية redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	إذا (معلومات->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	إذا (معلومات->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	أرجع (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  معلومات->x, معلومات->y,
				  معلومات->ويڪر, معلومات->اوچائي,
				  معلومات->background_رنگ,
				  flags) == NSERROR_OK);
}


ساكن nserror nssprite_clone(ثابت بنية content *old, بنية content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = صفر_يادگيري_ڏيو(1, حجم(nssprite_content));
	إذا (sprite == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &sprite->بنياد);
	إذا (error != NSERROR_OK) {
		content_destroy(&sprite->بنياد);
		أرجع error;
	}

	/* Simply replay convert */
	إذا (old->حالت == CONTENT_حالت_READY ||
			old->حالت == CONTENT_حالت_پورو) {
		إذا (nssprite_convert(&sprite->بنياد) == false) {
			content_destroy(&sprite->بنياد);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) sprite;

	أرجع NSERROR_OK;
}

ساكن فراغ *nssprite_get_اندروني(ثابت بنية content *c, فراغ *context)
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
	.معلومات_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_اندروني = nssprite_get_اندروني,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_رجسٽر_TYPES(nssprite, nssprite_types, nssprite_content_handler);
