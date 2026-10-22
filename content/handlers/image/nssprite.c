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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <librosprite.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/nssprite.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nssprite_content {
	ਬਣਤਰ content ਅਧਾਰ;
	ਬਣਤਰ bitmap *bitmap;	/**< Created NetSurf bitmap */

	ਬਣਤਰ rosprite_area* sprite_area;
} nssprite_content;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERRCHK(x) ਕਰੋ { \
	rosprite_error err = x; \
	ਜੇ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		ਜਾਓ ro_sprite_error; \
	} ਨਹੀਂਤਾਂ ਜੇ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		ਜਾਓ ro_sprite_error; \
	} ਨਹੀਂਤਾਂ ਜੇ (err == ROSPRITE_OK) { \
	} ਨਹੀਂਤਾਂ { \
		ਜਾਓ ro_sprite_error; \
	} \
} ਜਦੋਂ(0)




ਸਥਿਰ nserror nssprite_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		ਬਣਤਰ llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nssprite_content));
	ਜੇ (sprite == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&sprite->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(sprite);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) sprite;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

ਸਥਿਰ bool nssprite_convert(ਬਣਤਰ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ਬਣਤਰ rosprite_mem_context* ctx = ਨਿਰਾ;

	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	ERRCHK(rosprite_create_mem_context((uint8_t *) ਅੰਕੜੇ, ਮਾਪ, &ctx));

	ਬਣਤਰ rosprite_area* sprite_area;
	ERRCHK(rosprite_ਲੱਦੋ(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ਪ੍ਰਤਿਜ੍ਞਾ(sprite_area->sprite_ਗਿਣਤੀ > 0);

	ਬਣਤਰ rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->ਚੌੜਾਈ, sprite->ਉਚਾਈ, BITMAP_NONE);
	ਜੇ (!nssprite->bitmap) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}
	uint32_t* imagebuf = (uint32_t *)(ਖਾਲੀ *)guit->bitmap->get_ਬਫਰ(nssprite->bitmap);
	ਜੇ (!imagebuf) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *spritebuf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)sprite->image;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(imagebuf, spritebuf, sprite->ਚੌੜਾਈ * sprite->ਉਚਾਈ * 4);

	c->ਚੌੜਾਈ = sprite->ਚੌੜਾਈ;
	c->ਉਚਾਈ = sprite->ਉਚਾਈ;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	bitmap_format_ਮੰਜ਼ਿਲ_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, ""); /* Done: update status bar */

	ਵਾਪਸ true;

ro_sprite_error:
	ਜੇ (ctx != ਨਿਰਾ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_SPRITE_ERROR, ਨਿਰਾ);

	ਵਾਪਸ false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

ਸਥਿਰ ਖਾਲੀ nssprite_destroy(ਬਣਤਰ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ਜੇ (nssprite->sprite_area != ਨਿਰਾ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ਜੇ (nssprite->bitmap != ਨਿਰਾ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

ਸਥਿਰ bool
nssprite_redraw(ਬਣਤਰ content *c,
		ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ਜੇ (ਅੰਕੜੇ->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ਜੇ (ਅੰਕੜੇ->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	ਵਾਪਸ (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  ਅੰਕੜੇ->x, ਅੰਕੜੇ->y,
				  ਅੰਕੜੇ->ਚੌੜਾਈ, ਅੰਕੜੇ->ਉਚਾਈ,
				  ਅੰਕੜੇ->background_ਰੰਗ,
				  flags) == NSERROR_OK);
}


ਸਥਿਰ nserror nssprite_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(nssprite_content));
	ਜੇ (sprite == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &sprite->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&sprite->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay convert */
	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (nssprite_convert(&sprite->ਅਧਾਰ) == false) {
			content_destroy(&sprite->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) sprite;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ *nssprite_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ਵਾਪਸ nssprite->bitmap;
}

ਸਥਿਰ content_type nssprite_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}


ਸਥਿਰ bool nssprite_content_is_opaque(ਬਣਤਰ content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ਜੇ (nssprite->bitmap != ਨਿਰਾ) {
		ਵਾਪਸ guit->bitmap->get_opaque(nssprite->bitmap);
	}

	ਵਾਪਸ false;
}

ਸਥਿਰ ਅਚਲ content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.ਅੰਕੜੇ_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_ਅੰਦਰੂਨੀ = nssprite_get_ਅੰਦਰੂਨੀ,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nssprite, nssprite_types, nssprite_content_handler);
