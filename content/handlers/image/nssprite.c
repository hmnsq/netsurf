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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव <librosprite.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/nssprite.h"

प्रकारनिर्देश संरचना nssprite_content {
	संरचना content base;
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */

	संरचना rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) कुरु { \
	rosprite_error err = x; \
	यदि (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		गच्छ ro_sprite_error; \
	} अन्यथा यदि (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		गच्छ ro_sprite_error; \
	} अन्यथा यदि (err == ROSPRITE_OK) { \
	} अन्यथा { \
		गच्छ ro_sprite_error; \
	} \
} यावत्(0)




स्थिर nserror nssprite_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		संरचना llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, परिमाण(nssprite_content));
	यदि (sprite == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(sprite);
		प्रत्यागम error;
	}

	*c = (संरचना content *) sprite;

	प्रत्यागम NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

स्थिर bool nssprite_convert(संरचना content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	संरचना rosprite_mem_context* ctx = रिक्त;

	ध्रुव uint8_t *data;
	size_t size;
	वर्ण *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	संरचना rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	प्रतिज्ञा(sprite_area->sprite_गिनती > 0);

	संरचना rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	यदि (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;
	}
	uint32_t* imagebuf = (uint32_t *)(शून्य *)guit->bitmap->get_buffer(nssprite->bitmap);
	यदि (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;
	}
	unsigned वर्ण *spritebuf = (unsigned वर्ण *)sprite->image;

	memcpy(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	यदि (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	bitmap_format_to_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	प्रत्यागम true;

ro_sprite_error:
	यदि (ctx != रिक्त) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, रिक्त);

	प्रत्यागम false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

स्थिर शून्य nssprite_destroy(संरचना content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	यदि (nssprite->sprite_area != रिक्त)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	यदि (nssprite->bitmap != रिक्त)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

स्थिर bool
nssprite_redraw(संरचना content *c,
		संरचना content_redraw_data *data,
		ध्रुव संरचना rect *clip,
		ध्रुव संरचना redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	यदि (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	यदि (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	प्रत्यागम (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


स्थिर nserror nssprite_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = calloc(1, परिमाण(nssprite_content));
	यदि (sprite == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		प्रत्यागम error;
	}

	/* Simply replay convert */
	यदि (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		यदि (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) sprite;

	प्रत्यागम NSERROR_OK;
}

स्थिर शून्य *nssprite_get_internal(ध्रुव संरचना content *c, शून्य *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	प्रत्यागम nssprite->bitmap;
}

स्थिर content_type nssprite_content_type(शून्य)
{
	प्रत्यागम CONTENT_IMAGE;
}


स्थिर bool nssprite_content_is_opaque(संरचना content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	यदि (nssprite->bitmap != रिक्त) {
		प्रत्यागम guit->bitmap->get_opaque(nssprite->bitmap);
	}

	प्रत्यागम false;
}

स्थिर ध्रुव content_handler nssprite_content_handler = {
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

स्थिर ध्रुव वर्ण *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
