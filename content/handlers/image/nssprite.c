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
	संरचना content आधारः;
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */

	संरचना rosprite_area* sprite_area;
} nssprite_content;


#परिभाषय ERRCHK(x) कुरु { \
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

	sprite = शून्यस्मृतिं_देहि(1, परिमाण(nssprite_content));
	यदि (sprite == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&sprite->आधारः, handler, imime_type, params,
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

	ध्रुव uint8_t *दत्त;
	size_t आकार;
	वर्ण *title;

	दत्त = content_get_source_दत्त_2(c, &आकार);

	ERRCHK(rosprite_create_mem_context((uint8_t *) दत्त, आकार, &ctx));

	संरचना rosprite_area* sprite_area;
	ERRCHK(rosprite_आरोपय(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	प्रतिज्ञा(sprite_area->sprite_गणनम् > 0);

	संरचना rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->विस्तारः, sprite->height, BITMAP_NONE);
	यदि (!nssprite->bitmap) {
		content_प्रसारणम्_error(c, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;
	}
	uint32_t* imagebuf = (uint32_t *)(शून्य *)guit->bitmap->get_बफर(nssprite->bitmap);
	यदि (!imagebuf) {
		content_प्रसारणम्_error(c, NSERROR_NOMEM, रिक्त);
		प्रत्यागम false;
	}
	अचिह्नित वर्ण *spritebuf = (अचिह्नित वर्ण *)sprite->image;

	स्मृतिं_प्रतिलिख(imagebuf, spritebuf, sprite->विस्तारः * sprite->height * 4);

	c->विस्तारः = sprite->विस्तारः;
	c->height = sprite->height;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_पर्णम्(llcache_handle_get_url(c->llcache)),
			c->विस्तारः, c->height);
	यदि (title != रिक्त) {
		content_समुच्चयः_title(c, title);
		मुक्त(title);
	}

	bitmap_format_गन्तव्यम्_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);
	content_समुच्चयः_स्थितिः(c, ""); /* Done: update status bar */

	प्रत्यागम true;

ro_sprite_error:
	यदि (ctx != रिक्त) {
		rosprite_destroy_mem_context(ctx);
	}
	content_प्रसारणम्_error(c, NSERROR_SPRITE_ERROR, रिक्त);

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
		संरचना content_redraw_दत्त *दत्त,
		ध्रुव संरचना rect *clip,
		ध्रुव संरचना redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	यदि (दत्त->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	यदि (दत्त->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	प्रत्यागम (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  दत्त->x, दत्त->y,
				  दत्त->विस्तारः, दत्त->height,
				  दत्त->background_रङ्गः,
				  flags) == NSERROR_OK);
}


स्थिर nserror nssprite_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = शून्यस्मृतिं_देहि(1, परिमाण(nssprite_content));
	यदि (sprite == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &sprite->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&sprite->आधारः);
		प्रत्यागम error;
	}

	/* Simply replay convert */
	यदि (old->स्थितिः == CONTENT_स्थितिः_READY ||
			old->स्थितिः == CONTENT_स्थितिः_समाप्तम्) {
		यदि (nssprite_convert(&sprite->आधारः) == false) {
			content_destroy(&sprite->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) sprite;

	प्रत्यागम NSERROR_OK;
}

स्थिर शून्य *nssprite_get_आन्तरिकम्(ध्रुव संरचना content *c, शून्य *context)
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
	.दत्त_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_आन्तरिकम् = nssprite_get_आन्तरिकम्,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nssprite, nssprite_types, nssprite_content_handler);
