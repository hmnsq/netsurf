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
	بنية content پایه;

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
ساكن فراغ *nsbmp_bitmap_create(صحيح عرض, صحيح ارتفاع, بیعلامت صحيح bmp_state)
{
	بیعلامت صحيح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_حافظه) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	أرجع guit->bitmap->create(عرض, ارتفاع, bitmap_state);
}

ساكن nserror nsbmp_create_bmp_داده(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_میانگیر = guit->bitmap->get_میانگیر,
	};

	bmp->bmp = حافظه_صفر_بده(1, حجم(بنية bmp_image));
	إذا (bmp->bmp == عدم) {
		content_پخش_error(&bmp->پایه, NSERROR_NOMEM, عدم);
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

	bmp = حافظه_صفر_بده(1, حجم(nsbmp_content));
	إذا (bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&bmp->پایه, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(bmp);
		أرجع error;
	}

	error = nsbmp_create_bmp_داده(bmp);
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
	bmp_نتیجه res;
	uint32_t swidth;
	ثابت uint8_t *داده;
	size_t اندازه;
	حرف *title;

	/* set the bmp data */
	داده = content_get_منبع_داده_2(c, &اندازه);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, اندازه, (بیعلامت حرف *) داده);
	بدل (res) {
		حالة BMP_OK:
			اكسر;
		حالة BMP_INSUFFICIENT_حافظه:
			content_پخش_error(c, NSERROR_NOMEM, عدم);
			أرجع false;
		حالة BMP_INSUFFICIENT_داده:
		حالة BMP_داده_ERROR:
			content_پخش_error(c, NSERROR_BMP_ERROR, عدم);
			أرجع false;
	}

	/* Store our content width and description */
	c->عرض = bmp->bmp->عرض;
	c->ارتفاع = bmp->bmp->ارتفاع;
	swidth = حجم(uint32_t) * bmp->bmp->عرض;
	c->اندازه += (swidth * bmp->bmp->ارتفاع) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->عرض, c->ارتفاع);
	إذا (title != عدم) {
		content_مجموعه_title(c, title);
		حرر(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);

	/* Done: update status bar */
	content_مجموعه_وضعیت(c, "");
	أرجع true;
}

ساكن bool nsbmp_redraw(بنية content *c, بنية content_redraw_داده *داده,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	إذا (bmp->bmp->decoded == false) {
		bmp_نتیجه res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		إذا ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_داده) &&
		    (res != BMP_داده_ERROR)) {
			أرجع false;
		}

		bitmap_format_مقصد_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	إذا (داده->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (داده->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  داده->x, داده->y,
				  داده->عرض, داده->ارتفاع,
				  داده->background_رنگ,
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

	new_bmp = حافظه_صفر_بده(1, حجم(nsbmp_content));
	إذا (new_bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->پایه);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->پایه);
		أرجع error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_داده(new_bmp);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->پایه);
		أرجع error;
	}

	إذا (old->وضعیت == CONTENT_وضعیت_READY || 
			old->وضعیت == CONTENT_وضعیت_انجامشده) {
		إذا (nsbmp_convert(&new_bmp->پایه) == false) {
			content_destroy(&new_bmp->پایه);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) new_bmp;

	أرجع NSERROR_OK;
}

ساكن فراغ *nsbmp_get_داخلی(ثابت بنية content *c, فراغ *context)
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
	.داده_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_داخلی = nsbmp_get_داخلی,
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

CONTENT_FACTORY_ثبات_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
