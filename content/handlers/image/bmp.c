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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <libnsbmp.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/bmp.h"

/** bmp context. */
પ્રકારવ્યાખ્યા સંરચના nsbmp_content {
	સંરચના content base;

	bmp_image *bmp;	/** BMP image data */

	સંરચના bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
સ્થિર શૂન્ય *nsbmp_bitmap_create(પૂર્ણાંક width, પૂર્ણાંક height, unsigned પૂર્ણાંક bmp_state)
{
	unsigned પૂર્ણાંક bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	પાછા guit->bitmap->create(width, height, bitmap_state);
}

સ્થિર nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, કદ(સંરચના bmp_image));
	જો (bmp->bmp == ખાલી) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, ખાલી);
		પાછા NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	પાછા NSERROR_OK;
}

સ્થિર nserror
nsbmp_create(અચલ સંરચના content_handler *handler,
	     lwc_string *imime_type,
	     અચલ સંરચના http_parameter *params,
	     llcache_handle *llcache,
	     અચલ અક્ષર *fallback_charset,
	     bool quirks,
	     સંરચના content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, કદ(nsbmp_content));
	જો (bmp == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(bmp);
		પાછા error;
	}

	error = nsbmp_create_bmp_data(bmp);
	જો (error != NSERROR_OK) {
		મુક્ત(bmp);
		પાછા error;
	}

	*c = (સંરચના content *) bmp;

	પાછા NSERROR_OK;
}

સ્થિર bool nsbmp_convert(સંરચના content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	અચલ uint8_t *data;
	size_t size;
	અક્ષર *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned અક્ષર *) data);
	પસંદગી (res) {
		સ્થિતિ BMP_OK:
			તોડો;
		સ્થિતિ BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
			પાછા false;
		સ્થિતિ BMP_INSUFFICIENT_DATA:
		સ્થિતિ BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, ખાલી);
			પાછા false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = કદ(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	પાછા true;
}

સ્થિર bool nsbmp_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	જો (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		જો ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			પાછા false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	જો (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	જો (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	પાછા (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


સ્થિર શૂન્ય nsbmp_destroy(સંરચના content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	મુક્ત(bmp->bmp);
}


સ્થિર nserror nsbmp_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, કદ(nsbmp_content));
	જો (new_bmp == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	જો (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		પાછા error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	જો (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		પાછા error;
	}

	જો (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		જો (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) new_bmp;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય *nsbmp_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	પાછા bmp->bitmap;
}

સ્થિર content_type nsbmp_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}

સ્થિર bool nsbmp_content_is_opaque(સંરચના content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	જો (bmp->bitmap != ખાલી) {
		પાછા guit->bitmap->get_opaque(bmp->bitmap);
	}

	પાછા false;
}

સ્થિર અચલ content_handler nsbmp_content_handler = {
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

સ્થિર અચલ અક્ષર *nsbmp_types[] = {
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
