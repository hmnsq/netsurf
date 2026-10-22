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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <libnsbmp.h>

#ضمن "utils/utils.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/bmp.h"

/** bmp context. */
تعريفنوع بنية nsbmp_content {
	بنية content ادھار;

	bmp_image *bmp;	/** BMP image data */

	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ساكن فراغ *nsbmp_bitmap_create(صحيح چوڑائی, صحيح اچائی, بنانشانی صحيح bmp_state)
{
	بنانشانی صحيح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_حافظہ) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	أرجع guit->bitmap->create(چوڑائی, اچائی, bitmap_state);
}

ساكن nserror nsbmp_create_bmp_انکڑے(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_بفر = guit->bitmap->get_بفر,
	};

	bmp->bmp = صفر_حافظہ_دیو(1, حجم(بنية bmp_image));
	إذا (bmp->bmp == عدم) {
		content_نشر_error(&bmp->ادھار, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	أرجع NSERROR_OK;
}

ساكن nserror
nsbmp_create(ثابت بنية content_handler *handler,
	     lwc_string *imime_type,
	     ثابت بنية http_parameter *params,
	     llcache_handle *llcache,
	     ثابت حرف *fallback_charset,
	     bool quirks,
	     بنية content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = صفر_حافظہ_دیو(1, حجم(nsbmp_content));
	إذا (bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&bmp->ادھار, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(bmp);
		أرجع error;
	}

	error = nsbmp_create_bmp_انکڑے(bmp);
	إذا (error != NSERROR_OK) {
		حرر(bmp);
		أرجع error;
	}

	*c = (بنية content *) bmp;

	أرجع NSERROR_OK;
}

ساكن bool nsbmp_convert(بنية content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_نتیجہ res;
	uint32_t swidth;
	ثابت uint8_t *انکڑے;
	size_t ناپ;
	حرف *title;

	/* set the bmp data */
	انکڑے = content_get_سروت_انکڑے_2(c, &ناپ);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, ناپ, (بنانشانی حرف *) انکڑے);
	بدل (res) {
		حالة BMP_OK:
			اكسر;
		حالة BMP_INSUFFICIENT_حافظہ:
			content_نشر_error(c, NSERROR_NOMEM, عدم);
			أرجع false;
		حالة BMP_INSUFFICIENT_انکڑے:
		حالة BMP_انکڑے_ERROR:
			content_نشر_error(c, NSERROR_BMP_ERROR, عدم);
			أرجع false;
	}

	/* Store our content width and description */
	c->چوڑائی = bmp->bmp->چوڑائی;
	c->اچائی = bmp->bmp->اچائی;
	swidth = حجم(uint32_t) * bmp->bmp->چوڑائی;
	c->ناپ += (swidth * bmp->bmp->اچائی) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_پتی(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اچائی);
	إذا (title != عدم) {
		content_سیٹ_title(c, title);
		حرر(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_سیٹ_ready(c);
	content_سیٹ_پورا(c);

	/* Done: update status bar */
	content_سیٹ_حالت(c, "");
	أرجع true;
}

ساكن bool nsbmp_redraw(بنية content *c, بنية content_redraw_انکڑے *انکڑے,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	إذا (bmp->bmp->decoded == false) {
		bmp_نتیجہ res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		إذا ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_انکڑے) &&
		    (res != BMP_انکڑے_ERROR)) {
			أرجع false;
		}

		bitmap_format_منزل_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	إذا (انکڑے->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (انکڑے->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  انکڑے->x, انکڑے->y,
				  انکڑے->چوڑائی, انکڑے->اچائی,
				  انکڑے->background_رنگ,
				  flags) == NSERROR_OK);
}


ساكن فراغ nsbmp_destroy(بنية content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	حرر(bmp->bmp);
}


ساكن nserror nsbmp_clone(ثابت بنية content *old, بنية content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = صفر_حافظہ_دیو(1, حجم(nsbmp_content));
	إذا (new_bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->ادھار);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->ادھار);
		أرجع error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_انکڑے(new_bmp);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->ادھار);
		أرجع error;
	}

	إذا (old->حالت == CONTENT_حالت_READY || 
			old->حالت == CONTENT_حالت_پورا) {
		إذا (nsbmp_convert(&new_bmp->ادھار) == false) {
			content_destroy(&new_bmp->ادھار);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) new_bmp;

	أرجع NSERROR_OK;
}

ساكن فراغ *nsbmp_get_اندرونی(ثابت بنية content *c, فراغ *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	أرجع bmp->bitmap;
}

ساكن content_type nsbmp_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}

ساكن bool nsbmp_content_is_opaque(بنية content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	إذا (bmp->bitmap != عدم) {
		أرجع guit->bitmap->get_opaque(bmp->bitmap);
	}

	أرجع false;
}

ساكن ثابت content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.انکڑے_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_اندرونی = nsbmp_get_اندرونی,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *nsbmp_types[] = {
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
