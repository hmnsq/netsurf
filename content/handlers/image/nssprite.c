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

प्रकारव्याख्या संरचना nssprite_content {
	संरचना content base;
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */

	संरचना rosprite_area* sprite_area;
} nssprite_content;


#define ERRCHK(x) करा { \
	rosprite_error err = x; \
	जर (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		जा ro_sprite_error; \
	} अन्यथा जर (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		जा ro_sprite_error; \
	} अन्यथा जर (err == ROSPRITE_OK) { \
	} अन्यथा { \
		जा ro_sprite_error; \
	} \
} पर्यंत(0)




स्थिर nserror nssprite_create(स्थिरांक content_handler *handler,
		lwc_string *imime_type, स्थिरांक संरचना http_parameter *params,
		संरचना llcache_handle *llcache, स्थिरांक अक्षर *fallback_charset,
		bool quirks, संरचना content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = सुस्मृति(1, आकार(nssprite_content));
	जर (sprite == रिक्त)
		परत NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	जर (error != NSERROR_OK) {
		मुक्त(sprite);
		परत error;
	}

	*c = (संरचना content *) sprite;

	परत NSERROR_OK;
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

	स्थिरांक uint8_t *data;
	माप_प्रकार size;
	अक्षर *title;

	data = content__get_source_data(c, &size);

	ERRCHK(rosprite_create_mem_context((uint8_t *) data, size, &ctx));

	संरचना rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	प्रतिज्ञा(sprite_area->sprite_मोजणी > 0);

	संरचना rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	जर (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		परत false;
	}
	uint32_t* imagebuf = (uint32_t *)(शून्य *)guit->bitmap->get_buffer(nssprite->bitmap);
	जर (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		परत false;
	}
	अचिन्हित अक्षर *spritebuf = (अचिन्हित अक्षर *)sprite->image;

	स_नकल(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	जर (title != रिक्त) {
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

	परत true;

ro_sprite_error:
	जर (ctx != रिक्त) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, रिक्त);

	परत false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

स्थिर शून्य nssprite_destroy(संरचना content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	जर (nssprite->sprite_area != रिक्त)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	जर (nssprite->bitmap != रिक्त)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

स्थिर bool
nssprite_redraw(संरचना content *c,
		संरचना content_redraw_data *data,
		स्थिरांक संरचना rect *clip,
		स्थिरांक संरचना redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	जर (data->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	जर (data->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	परत (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


स्थिर nserror nssprite_clone(स्थिरांक संरचना content *old, संरचना content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = सुस्मृति(1, आकार(nssprite_content));
	जर (sprite == रिक्त)
		परत NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	जर (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		परत error;
	}

	/* Simply replay convert */
	जर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		जर (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			परत NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) sprite;

	परत NSERROR_OK;
}

स्थिर शून्य *nssprite_get_internal(स्थिरांक संरचना content *c, शून्य *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	परत nssprite->bitmap;
}

स्थिर content_type nssprite_content_type(शून्य)
{
	परत CONTENT_IMAGE;
}


स्थिर bool nssprite_content_is_opaque(संरचना content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	जर (nssprite->bitmap != रिक्त) {
		परत guit->bitmap->get_opaque(nssprite->bitmap);
	}

	परत false;
}

स्थिर स्थिरांक content_handler nssprite_content_handler = {
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

स्थिर स्थिरांक अक्षर *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
