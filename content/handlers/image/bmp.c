/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Sean Fox <dyntryx@gmail.com>
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
 * implementation of content handler for BMP images.
 */

#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <libnsbmp.h>

#شامل "utils/utils.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/bmp.h"

/** bmp context. */
قسمتعریف ساخت nsbmp_content {
	ساخت content بنیاد;

	bmp_image *bmp;	/** BMP image data */

	ساخت bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ساکن خالی *nsbmp_bitmap_create(صحیح چوڑائی, صحیح اونچائی, غیرعلامتی صحیح bmp_state)
{
	غیرعلامتی صحیح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_حافظہ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	واپس guit->bitmap->create(چوڑائی, اونچائی, bitmap_state);
}

ساکن nserror nsbmp_create_bmp_ڈیٹا(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_بفر = guit->bitmap->get_بفر,
	};

	bmp->bmp = صفر_حافظہ_دو(1, حجم(ساخت bmp_image));
	اگر (bmp->bmp == تہی) {
		content_نشر_error(&bmp->بنیاد, NSERROR_NOMEM, تہی);
		واپس NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	واپس NSERROR_OK;
}

ساکن nserror
nsbmp_create(ثابت ساخت content_handler *handler,
	     lwc_string *imime_type,
	     ثابت ساخت http_parameter *params,
	     llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     ساخت content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = صفر_حافظہ_دو(1, حجم(nsbmp_content));
	اگر (bmp == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&bmp->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(bmp);
		واپس error;
	}

	error = nsbmp_create_bmp_ڈیٹا(bmp);
	اگر (error != NSERROR_OK) {
		آزاد(bmp);
		واپس error;
	}

	*c = (ساخت content *) bmp;

	واپس NSERROR_OK;
}

ساکن bool nsbmp_convert(ساخت content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_نتیجہ res;
	uint32_t swidth;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;
	حرف *title;

	/* set the bmp data */
	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &سائز);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, سائز, (غیرعلامتی حرف *) ڈیٹا);
	انتخاب (res) {
		حالت BMP_OK:
			توڑو;
		حالت BMP_INSUFFICIENT_حافظہ:
			content_نشر_error(c, NSERROR_NOMEM, تہی);
			واپس false;
		حالت BMP_INSUFFICIENT_ڈیٹا:
		حالت BMP_ڈیٹا_ERROR:
			content_نشر_error(c, NSERROR_BMP_ERROR, تہی);
			واپس false;
	}

	/* Store our content width and description */
	c->چوڑائی = bmp->bmp->چوڑائی;
	c->اونچائی = bmp->bmp->اونچائی;
	swidth = حجم(uint32_t) * bmp->bmp->چوڑائی;
	c->سائز += (swidth * bmp->bmp->اونچائی) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اونچائی);
	اگر (title != تہی) {
		content_سیٹ_title(c, title);
		آزاد(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);

	/* Done: update status bar */
	content_سیٹ_حالت(c, "");
	واپس true;
}

ساکن bool nsbmp_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	اگر (bmp->bmp->decoded == false) {
		bmp_نتیجہ res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		اگر ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_ڈیٹا) &&
		    (res != BMP_ڈیٹا_ERROR)) {
			واپس false;
		}

		bitmap_format_مقصد_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	اگر (ڈیٹا->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (ڈیٹا->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	واپس (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  ڈیٹا->x, ڈیٹا->y,
				  ڈیٹا->چوڑائی, ڈیٹا->اونچائی,
				  ڈیٹا->background_رنگ,
				  flags) == NSERROR_OK);
}


ساکن خالی nsbmp_destroy(ساخت content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	آزاد(bmp->bmp);
}


ساکن nserror nsbmp_clone(ثابت ساخت content *old, ساخت content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = صفر_حافظہ_دو(1, حجم(nsbmp_content));
	اگر (new_bmp == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_bmp->بنیاد);
		واپس error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_ڈیٹا(new_bmp);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_bmp->بنیاد);
		واپس error;
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY || 
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (nsbmp_convert(&new_bmp->بنیاد) == false) {
			content_destroy(&new_bmp->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) new_bmp;

	واپس NSERROR_OK;
}

ساکن خالی *nsbmp_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	واپس bmp->bitmap;
}

ساکن content_type nsbmp_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}

ساکن bool nsbmp_content_is_opaque(ساخت content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	اگر (bmp->bitmap != تہی) {
		واپس guit->bitmap->get_opaque(bmp->bitmap);
	}

	واپس false;
}

ساکن ثابت content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.ڈیٹا_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_اندرونی = nsbmp_get_اندرونی,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *nsbmp_types[] = {
	"application/bmp",
	"application/preview",
	"application/x-bmp",
	"application/x-win-bitmap",
	"image/bmp",
	"image/ms-bmp",
	"image/x-bitmap",
	"image/x-bmp",
	"image/x-ms-bmp",
	"image/x-win-bitmap",
	"image/x-windows-bmp",
	"image/x-xbitmap"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
