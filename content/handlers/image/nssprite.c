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

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश <librosprite.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/nssprite.h"

प्रकार काष्ठा nssprite_content {
	काष्ठा content base;
	काष्ठा bitmap *bitmap;	/**< Created NetSurf bitmap */

	काष्ठा rosprite_area* sprite_area;
} nssprite_content;


#घोषणा ERRCHK(x) करो { \
	rosprite_error err = x; \
	अगर (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		जाओ ro_sprite_error; \
	} अन्यथा अगर (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		जाओ ro_sprite_error; \
	} अन्यथा अगर (err == ROSPRITE_OK) { \
	} अन्यथा { \
		जाओ ro_sprite_error; \
	} \
} जबतक(0)




अटल nserror nssprite_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		काष्ठा llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = सुस्मृति(1, माप(nssprite_content));
	अगर (sprite == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&sprite->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(sprite);
		वापस error;
	}

	*c = (काष्ठा content *) sprite;

	वापस NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

अटल bool nssprite_convert(काष्ठा content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	काष्ठा rosprite_mem_context* ctx = शून्य;

	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;
	अक्षर *title;

	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	ERRCHK(rosprite_create_mem_context((uint8_t *) तथ्य, आकार, &ctx));

	काष्ठा rosprite_area* sprite_area;
	ERRCHK(rosprite_load(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	निश्चित(sprite_area->sprite_गिनती > 0);

	काष्ठा rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->width, sprite->height, BITMAP_NONE);
	अगर (!nssprite->bitmap) {
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
	}
	uint32_t* imagebuf = (uint32_t *)(व्योम *)guit->bitmap->get_बफर(nssprite->bitmap);
	अगर (!imagebuf) {
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
	}
	अचिन्हित अक्षर *spritebuf = (अचिन्हित अक्षर *)sprite->image;

	स_नकल(imagebuf, spritebuf, sprite->width * sprite->height * 4);

	c->width = sprite->width;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	अगर (title != शून्य) {
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

	वापस true;

ro_sprite_error:
	अगर (ctx != शून्य) {
		rosprite_destroy_mem_context(ctx);
	}
	content_broadcast_error(c, NSERROR_SPRITE_ERROR, शून्य);

	वापस false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

अटल व्योम nssprite_destroy(काष्ठा content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	अगर (nssprite->sprite_area != शून्य)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	अगर (nssprite->bitmap != शून्य)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

अटल bool
nssprite_redraw(काष्ठा content *c,
		काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip,
		स्थिर काष्ठा redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	अगर (तथ्य->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	अगर (तथ्य->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	वापस (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  तथ्य->x, तथ्य->y,
				  तथ्य->width, तथ्य->height,
				  तथ्य->background_colour,
				  flags) == NSERROR_OK);
}


अटल nserror nssprite_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = सुस्मृति(1, माप(nssprite_content));
	अगर (sprite == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &sprite->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&sprite->base);
		वापस error;
	}

	/* Simply replay convert */
	अगर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		अगर (nssprite_convert(&sprite->base) == false) {
			content_destroy(&sprite->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) sprite;

	वापस NSERROR_OK;
}

अटल व्योम *nssprite_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	वापस nssprite->bitmap;
}

अटल content_type nssprite_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}


अटल bool nssprite_content_is_opaque(काष्ठा content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	अगर (nssprite->bitmap != शून्य) {
		वापस guit->bitmap->get_opaque(nssprite->bitmap);
	}

	वापस false;
}

अटल स्थिर content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.तथ्य_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_internal = nssprite_get_internal,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_REGISTER_TYPES(nssprite, nssprite_types, nssprite_content_handler);
