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
	بنية content بنسټ;

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
ساكن فراغ *nsbmp_bitmap_create(صحيح پلنوالی, صحيح لوړوالی, بینښه صحيح bmp_state)
{
	بینښه صحيح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_حافظه) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	أرجع guit->bitmap->create(پلنوالی, لوړوالی, bitmap_state);
}

ساكن nserror nsbmp_create_bmp_معلومات(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_بفر = guit->bitmap->get_بفر,
	};

	bmp->bmp = حافظه_صفر_ورکړه(1, حجم(بنية bmp_image));
	إذا (bmp->bmp == عدم) {
		content_خپرونه_error(&bmp->بنسټ, NSERROR_NOMEM, عدم);
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

	bmp = حافظه_صفر_ورکړه(1, حجم(nsbmp_content));
	إذا (bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&bmp->بنسټ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(bmp);
		أرجع error;
	}

	error = nsbmp_create_bmp_معلومات(bmp);
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
	bmp_پایله res;
	uint32_t swidth;
	ثابت uint8_t *معلومات;
	size_t اندازه;
	حرف *title;

	/* set the bmp data */
	معلومات = content_get_سرچینه_معلومات_2(c, &اندازه);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, اندازه, (بینښه حرف *) معلومات);
	بدل (res) {
		حالة BMP_OK:
			اكسر;
		حالة BMP_INSUFFICIENT_حافظه:
			content_خپرونه_error(c, NSERROR_NOMEM, عدم);
			أرجع false;
		حالة BMP_INSUFFICIENT_معلومات:
		حالة BMP_معلومات_ERROR:
			content_خپرونه_error(c, NSERROR_BMP_ERROR, عدم);
			أرجع false;
	}

	/* Store our content width and description */
	c->پلنوالی = bmp->bmp->پلنوالی;
	c->لوړوالی = bmp->bmp->لوړوالی;
	swidth = حجم(uint32_t) * bmp->bmp->پلنوالی;
	c->اندازه += (swidth * bmp->bmp->لوړوالی) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_ونې_پاڼه(llcache_handle_get_url(c->llcache)),
			c->پلنوالی, c->لوړوالی);
	إذا (title != عدم) {
		content_سېټ_title(c, title);
		حرر(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_سېټ_ready(c);
	content_سېټ_بشپړ(c);

	/* Done: update status bar */
	content_سېټ_حالت(c, "");
	أرجع true;
}

ساكن bool nsbmp_redraw(بنية content *c, بنية content_redraw_معلومات *معلومات,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	إذا (bmp->bmp->decoded == false) {
		bmp_پایله res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		إذا ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_معلومات) &&
		    (res != BMP_معلومات_ERROR)) {
			أرجع false;
		}

		bitmap_format_منزل_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	إذا (معلومات->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (معلومات->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  معلومات->x, معلومات->y,
				  معلومات->پلنوالی, معلومات->لوړوالی,
				  معلومات->background_رنګ,
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

	new_bmp = حافظه_صفر_ورکړه(1, حجم(nsbmp_content));
	إذا (new_bmp == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->بنسټ);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->بنسټ);
		أرجع error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_معلومات(new_bmp);
	إذا (error != NSERROR_OK) {
		content_destroy(&new_bmp->بنسټ);
		أرجع error;
	}

	إذا (old->حالت == CONTENT_حالت_READY || 
			old->حالت == CONTENT_حالت_بشپړ) {
		إذا (nsbmp_convert(&new_bmp->بنسټ) == false) {
			content_destroy(&new_bmp->بنسټ);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) new_bmp;

	أرجع NSERROR_OK;
}

ساكن فراغ *nsbmp_get_دننی(ثابت بنية content *c, فراغ *context)
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
	.معلومات_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_دننی = nsbmp_get_دننی,
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

CONTENT_FACTORY_ثبتوونکی_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
