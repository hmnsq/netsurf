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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ <librosprite.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/nssprite.h"

రకనిర్వచనం నిర్మాణం nssprite_content {
	నిర్మాణం content పునాది;
	నిర్మాణం bitmap *bitmap;	/**< Created NetSurf bitmap */

	నిర్మాణం rosprite_area* sprite_area;
} nssprite_content;


#నిర్వచించు ERRCHK(x) చేయి { \
	rosprite_error err = x; \
	ఒకవేళ (err == ROSPRITE_EOF) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_EOF when loading sprite file"); \
		వెళ్ళు ro_sprite_error; \
	} లేకపోతే ఒకవేళ (err == ROSPRITE_BADMODE) { \
		NSLOG(netsurf, INFO, "Got ROSPRITE_BADMODE when loading sprite file"); \
		వెళ్ళు ro_sprite_error; \
	} లేకపోతే ఒకవేళ (err == ROSPRITE_OK) { \
	} లేకపోతే { \
		వెళ్ళు ro_sprite_error; \
	} \
} అయితే(0)




స్థిర nserror nssprite_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		నిర్మాణం llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	nssprite_content *sprite;
	nserror error;

	sprite = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nssprite_content));
	ఒకవేళ (sprite == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&sprite->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(sprite);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) sprite;

	తిరిగి NSERROR_OK;
}

/**
 * Convert a CONTENT_SPRITE for display.
 *
 * No conversion is necessary. We merely read the sprite dimensions.
 */

స్థిర bool nssprite_convert(నిర్మాణం content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	నిర్మాణం rosprite_mem_context* ctx = ఖాళీ;

	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
	అక్షరం *title;

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	ERRCHK(rosprite_create_mem_context((uint8_t *) దత్తాంశం, కొలత, &ctx));

	నిర్మాణం rosprite_area* sprite_area;
	ERRCHK(rosprite_ఎక్కించు(rosprite_mem_reader, ctx, &sprite_area));
	rosprite_destroy_mem_context(ctx);
	nssprite->sprite_area = sprite_area;

	ప్రతిజ్ఞా(sprite_area->sprite_లెక్క > 0);

	నిర్మాణం rosprite* sprite = sprite_area->sprites[0];

	nssprite->bitmap = guit->bitmap->create(sprite->వెడల్పు, sprite->ఎత్తు, BITMAP_NONE);
	ఒకవేళ (!nssprite->bitmap) {
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	}
	uint32_t* imagebuf = (uint32_t *)(శూన్య *)guit->bitmap->get_బఫరు(nssprite->bitmap);
	ఒకవేళ (!imagebuf) {
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	}
	గుర్తులేని అక్షరం *spritebuf = (గుర్తులేని అక్షరం *)sprite->image;

	జ్ఞాపకం_నకలు(imagebuf, spritebuf, sprite->వెడల్పు * sprite->ఎత్తు * 4);

	c->వెడల్పు = sprite->వెడల్పు;
	c->ఎత్తు = sprite->ఎత్తు;

	/* set title text */
	title = messages_get_buff("SpriteTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	bitmap_format_గమ్యం_client(nssprite->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_A8B8G8R8,
	});
	guit->bitmap->modified(nssprite->bitmap);

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
	content_సమితి_స్థితి(c, ""); /* Done: update status bar */

	తిరిగి true;

ro_sprite_error:
	ఒకవేళ (ctx != ఖాళీ) {
		rosprite_destroy_mem_context(ctx);
	}
	content_ప్రసారం_error(c, NSERROR_SPRITE_ERROR, ఖాళీ);

	తిరిగి false;
}


/**
 * Destroy a CONTENT_SPRITE and free all resources it owns.
 */

స్థిర శూన్య nssprite_destroy(నిర్మాణం content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ఒకవేళ (nssprite->sprite_area != ఖాళీ)
		rosprite_destroy_sprite_area(nssprite->sprite_area);
	ఒకవేళ (nssprite->bitmap != ఖాళీ)
		guit->bitmap->destroy(nssprite->bitmap);
}


/**
 * Redraw a CONTENT_SPRITE.
 */

స్థిర bool
nssprite_redraw(నిర్మాణం content *c,
		నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	nssprite_content *nssprite = (nssprite_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ఒకవేళ (దత్తాంశం->repeat_x) {
		flags |= BITMAPF_REPEAT_X;
	}
	ఒకవేళ (దత్తాంశం->repeat_y) {
		flags |= BITMAPF_REPEAT_Y;
	}

	తిరిగి (ctx->plot->bitmap(ctx,
				  nssprite->bitmap,
				  దత్తాంశం->x, దత్తాంశం->y,
				  దత్తాంశం->వెడల్పు, దత్తాంశం->ఎత్తు,
				  దత్తాంశం->background_రంగు,
				  flags) == NSERROR_OK);
}


స్థిర nserror nssprite_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	nssprite_content *sprite;
	nserror error;

	sprite = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nssprite_content));
	ఒకవేళ (sprite == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &sprite->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&sprite->పునాది);
		తిరిగి error;
	}

	/* Simply replay convert */
	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (nssprite_convert(&sprite->పునాది) == false) {
			content_destroy(&sprite->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) sprite;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య *nssprite_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	తిరిగి nssprite->bitmap;
}

స్థిర content_type nssprite_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}


స్థిర bool nssprite_content_is_opaque(నిర్మాణం content *c)
{
	nssprite_content *nssprite = (nssprite_content *) c;

	ఒకవేళ (nssprite->bitmap != ఖాళీ) {
		తిరిగి guit->bitmap->get_opaque(nssprite->bitmap);
	}

	తిరిగి false;
}

స్థిర స్థిరరాశి content_handler nssprite_content_handler = {
	.create = nssprite_create,
	.దత్తాంశం_complete = nssprite_convert,
	.destroy = nssprite_destroy,
	.redraw = nssprite_redraw,
	.clone = nssprite_clone,
	.get_అంతర్గత = nssprite_get_అంతర్గత,
	.type = nssprite_content_type,
	.is_opaque = nssprite_content_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *nssprite_types[] = {
	"image/x-riscos-sprite"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nssprite, nssprite_types, nssprite_content_handler);
