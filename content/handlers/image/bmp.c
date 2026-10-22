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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <libnsbmp.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/bmp.h"

/** bmp context. */
প্রকারলেখ কাঠামো nsbmp_content {
	কাঠামো content base;

	bmp_image *bmp;	/** BMP image data */

	কাঠামো bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
স্থির শূন্য *nsbmp_bitmap_create(পূর্ণসংখ্যা width, পূর্ণসংখ্যা height, unsigned পূর্ণসংখ্যা bmp_state)
{
	unsigned পূর্ণসংখ্যা bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ফেরত guit->bitmap->create(width, height, bitmap_state);
}

স্থির nserror nsbmp_create_bmp_তথ্য(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_বাফার = guit->bitmap->get_বাফার,
	};

	bmp->bmp = calloc(1, আকার(কাঠামো bmp_image));
	যদি (bmp->bmp == ফাঁকা) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ফেরত NSERROR_OK;
}

স্থির nserror
nsbmp_create(ধ্রুবক কাঠামো content_handler *handler,
	     lwc_string *imime_type,
	     ধ্রুবক কাঠামো http_parameter *params,
	     llcache_handle *llcache,
	     ধ্রুবক অক্ষর *fallback_charset,
	     bool quirks,
	     কাঠামো content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, আকার(nsbmp_content));
	যদি (bmp == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(bmp);
		ফেরত error;
	}

	error = nsbmp_create_bmp_তথ্য(bmp);
	যদি (error != NSERROR_OK) {
		মুক্ত(bmp);
		ফেরত error;
	}

	*c = (কাঠামো content *) bmp;

	ফেরত NSERROR_OK;
}

স্থির bool nsbmp_convert(কাঠামো content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_ফলাফল res;
	uint32_t swidth;
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;
	অক্ষর *title;

	/* set the bmp data */
	তথ্য = content_get_source_তথ্য_2(c, &আকার_2);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, আকার_2, (unsigned অক্ষর *) তথ্য);
	নির্বাচন (res) {
		ক্ষেত্র BMP_OK:
			বিরতি;
		ক্ষেত্র BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
			ফেরত false;
		ক্ষেত্র BMP_INSUFFICIENT_তথ্য:
		ক্ষেত্র BMP_তথ্য_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, ফাঁকা);
			ফেরত false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = আকার(uint32_t) * bmp->bmp->width;
	c->আকার_2 += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != ফাঁকা) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ফেরত true;
}

স্থির bool nsbmp_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	যদি (bmp->bmp->decoded == false) {
		bmp_ফলাফল res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		যদি ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_তথ্য) &&
		    (res != BMP_তথ্য_ERROR)) {
			ফেরত false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	যদি (তথ্য->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	যদি (তথ্য->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ফেরত (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  তথ্য->x, তথ্য->y,
				  তথ্য->width, তথ্য->height,
				  তথ্য->background_colour,
				  flags) == NSERROR_OK);
}


স্থির শূন্য nsbmp_destroy(কাঠামো content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	মুক্ত(bmp->bmp);
}


স্থির nserror nsbmp_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, আকার(nsbmp_content));
	যদি (new_bmp == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ফেরত error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_তথ্য(new_bmp);
	যদি (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ফেরত error;
	}

	যদি (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		যদি (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*newc = (কাঠামো content *) new_bmp;

	ফেরত NSERROR_OK;
}

স্থির শূন্য *nsbmp_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ফেরত bmp->bitmap;
}

স্থির content_type nsbmp_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}

স্থির bool nsbmp_content_is_opaque(কাঠামো content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	যদি (bmp->bitmap != ফাঁকা) {
		ফেরত guit->bitmap->get_opaque(bmp->bitmap);
	}

	ফেরত false;
}

স্থির ধ্রুবক content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.তথ্য_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_internal = nsbmp_get_internal,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *nsbmp_types[] = {
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

CONTENT_FACTORY_REGISTER_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
