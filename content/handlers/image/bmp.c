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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <libnsbmp.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/bmp.h"

/** bmp context. */
வகைவரை கட்டமைப்பு nsbmp_content {
	கட்டமைப்பு content base;

	bmp_image *bmp;	/** BMP image data */

	கட்டமைப்பு bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
நிலையான வெறுமை *nsbmp_bitmap_create(முழு width, முழு height, unsigned முழு bmp_state)
{
	unsigned முழு bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	திரும்பு guit->bitmap->create(width, height, bitmap_state);
}

நிலையான nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, அளவு(கட்டமைப்பு bmp_image));
	எனில் (bmp->bmp == வெற்று) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	திரும்பு NSERROR_OK;
}

நிலையான nserror
nsbmp_create(மாறிலி கட்டமைப்பு content_handler *handler,
	     lwc_string *imime_type,
	     மாறிலி கட்டமைப்பு http_parameter *params,
	     llcache_handle *llcache,
	     மாறிலி எழுத்து *fallback_charset,
	     bool quirks,
	     கட்டமைப்பு content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, அளவு(nsbmp_content));
	எனில் (bmp == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(bmp);
		திரும்பு error;
	}

	error = nsbmp_create_bmp_data(bmp);
	எனில் (error != NSERROR_OK) {
		விடுவி(bmp);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) bmp;

	திரும்பு NSERROR_OK;
}

நிலையான bool nsbmp_convert(கட்டமைப்பு content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	மாறிலி uint8_t *data;
	size_t size;
	எழுத்து *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned எழுத்து *) data);
	தேர்வு (res) {
		நிலை BMP_OK:
			நிறுத்து;
		நிலை BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
			திரும்பு false;
		நிலை BMP_INSUFFICIENT_DATA:
		நிலை BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, வெற்று);
			திரும்பு false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = அளவு(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	திரும்பு true;
}

நிலையான bool nsbmp_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	எனில் (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		எனில் ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			திரும்பு false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	எனில் (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	எனில் (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	திரும்பு (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


நிலையான வெறுமை nsbmp_destroy(கட்டமைப்பு content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	விடுவி(bmp->bmp);
}


நிலையான nserror nsbmp_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, அளவு(nsbmp_content));
	எனில் (new_bmp == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		திரும்பு error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	எனில் (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		திரும்பு error;
	}

	எனில் (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		எனில் (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) new_bmp;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை *nsbmp_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	திரும்பு bmp->bitmap;
}

நிலையான content_type nsbmp_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}

நிலையான bool nsbmp_content_is_opaque(கட்டமைப்பு content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	எனில் (bmp->bitmap != வெற்று) {
		திரும்பு guit->bitmap->get_opaque(bmp->bitmap);
	}

	திரும்பு false;
}

நிலையான மாறிலி content_handler nsbmp_content_handler = {
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

நிலையான மாறிலி எழுத்து *nsbmp_types[] = {
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
