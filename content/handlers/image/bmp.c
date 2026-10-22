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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libnsbmp.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/bmp.h"

/** bmp context. */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nsbmp_content {
	ಸಂರಚನಾ content base;

	bmp_image *bmp;	/** BMP image data */

	ಸಂರಚನಾ bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಶೂನ್ಯ *nsbmp_bitmap_create(ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height, unsigned ಪೂರ್ಣಾಙ್ಕ bmp_state)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ಪ್ರತ್ಯಾಗಮ guit->bitmap->create(width, height, bitmap_state);
}

ಸ್ಥಿರ nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ bmp_image));
	ಯದಿ (bmp->bmp == ರಿಕ್ತ) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ nserror
nsbmp_create(ಧ್ರುವ ಸಂರಚನಾ content_handler *handler,
	     lwc_string *imime_type,
	     ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
	     llcache_handle *llcache,
	     ಧ್ರುವ ವರ್ಣ *fallback_charset,
	     bool quirks,
	     ಸಂರಚನಾ content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, ಪರಿಮಾಣ(nsbmp_content));
	ಯದಿ (bmp == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(bmp);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = nsbmp_create_bmp_data(bmp);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(bmp);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) bmp;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ bool nsbmp_convert(ಸಂರಚನಾ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	ಧ್ರುವ uint8_t *data;
	size_t size;
	ವರ್ಣ *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned ವರ್ಣ *) data);
	ವಿಕಲ್ಪ (res) {
		ಪಕ್ಷ BMP_OK:
			ಭಙ್ಗ;
		ಪಕ್ಷ BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
			ಪ್ರತ್ಯಾಗಮ false;
		ಪಕ್ಷ BMP_INSUFFICIENT_DATA:
		ಪಕ್ಷ BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, ರಿಕ್ತ);
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = ಪರಿಮಾಣ(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool nsbmp_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಯದಿ (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ಯದಿ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ಯದಿ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ಯದಿ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ಪ್ರತ್ಯಾಗಮ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ಸ್ಥಿರ ಶೂನ್ಯ nsbmp_destroy(ಸಂರಚನಾ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	ಮುಕ್ತ(bmp->bmp);
}


ಸ್ಥಿರ nserror nsbmp_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, ಪರಿಮಾಣ(nsbmp_content));
	ಯದಿ (new_bmp == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) new_bmp;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nsbmp_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ಪ್ರತ್ಯಾಗಮ bmp->bitmap;
}

ಸ್ಥಿರ content_type nsbmp_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}

ಸ್ಥಿರ bool nsbmp_content_is_opaque(ಸಂರಚನಾ content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ಯದಿ (bmp->bitmap != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(bmp->bitmap);
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nsbmp_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nsbmp_types[] = {
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
