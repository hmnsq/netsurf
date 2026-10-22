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
تعریفنوع ساختار nsbmp_content {
	ساختار content base;

	bmp_image *bmp;	/** BMP image data */

	ساختار bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ایستا تهی *nsbmp_bitmap_create(صحیح width, صحیح height, unsigned صحیح bmp_state)
{
	unsigned صحیح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	بازگشت guit->bitmap->create(width, height, bitmap_state);
}

ایستا nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, اندازه(ساختار bmp_image));
	اگر (bmp->bmp == پوچ) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, پوچ);
		بازگشت NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	بازگشت NSERROR_OK;
}

ایستا nserror
nsbmp_create(ثابت ساختار content_handler *handler,
	     lwc_string *imime_type,
	     ثابت ساختار http_parameter *params,
	     llcache_handle *llcache,
	     ثابت نویسه *fallback_charset,
	     bool quirks,
	     ساختار content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, اندازه(nsbmp_content));
	اگر (bmp == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(bmp);
		بازگشت error;
	}

	error = nsbmp_create_bmp_data(bmp);
	اگر (error != NSERROR_OK) {
		آزاد(bmp);
		بازگشت error;
	}

	*c = (ساختار content *) bmp;

	بازگشت NSERROR_OK;
}

ایستا bool nsbmp_convert(ساختار content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	ثابت uint8_t *data;
	size_t size;
	نویسه *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned نویسه *) data);
	انتخاب (res) {
		حالت BMP_OK:
			بشکن;
		حالت BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, پوچ);
			بازگشت false;
		حالت BMP_INSUFFICIENT_DATA:
		حالت BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, پوچ);
			بازگشت false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = اندازه(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	اگر (title != پوچ) {
		content__set_title(c, title);
		آزاد(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	بازگشت true;
}

ایستا bool nsbmp_redraw(ساختار content *c, ساختار content_redraw_data *data,
		ثابت ساختار rect *clip, ثابت ساختار redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	اگر (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		اگر ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			بازگشت false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	اگر (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	بازگشت (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ایستا تهی nsbmp_destroy(ساختار content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	آزاد(bmp->bmp);
}


ایستا nserror nsbmp_clone(ثابت ساختار content *old, ساختار content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, اندازه(nsbmp_content));
	اگر (new_bmp == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		بازگشت error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	اگر (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		بازگشت error;
	}

	اگر (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		اگر (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *) new_bmp;

	بازگشت NSERROR_OK;
}

ایستا تهی *nsbmp_get_internal(ثابت ساختار content *c, تهی *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	بازگشت bmp->bitmap;
}

ایستا content_type nsbmp_content_type(تهی)
{
	بازگشت CONTENT_IMAGE;
}

ایستا bool nsbmp_content_is_opaque(ساختار content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	اگر (bmp->bitmap != پوچ) {
		بازگشت guit->bitmap->get_opaque(bmp->bitmap);
	}

	بازگشت false;
}

ایستا ثابت content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.data_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_internal = nsbmp_get_internal,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

ایستا ثابت نویسه *nsbmp_types[] = {
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
