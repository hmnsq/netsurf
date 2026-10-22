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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <libnsbmp.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/bmp.h"

/** bmp context. */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nsbmp_content {
	ସଂରଚନା content base;

	bmp_image *bmp;	/** BMP image data */

	ସଂରଚନା bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
ସ୍ଥିର ଶୂନ୍ୟ *nsbmp_bitmap_create(ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_state)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ଫେରନ୍ତୁ guit->bitmap->create(width, height, bitmap_state);
}

ସ୍ଥିର nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, ଆକାର(ସଂରଚନା bmp_image));
	ଯଦି (bmp->bmp == ଖାଲି) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror
nsbmp_create(ଅଚଳ ସଂରଚନା content_handler *handler,
	     lwc_string *imime_type,
	     ଅଚଳ ସଂରଚନା http_parameter *params,
	     llcache_handle *llcache,
	     ଅଚଳ ଅକ୍ଷର *fallback_charset,
	     bool quirks,
	     ସଂରଚନା content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, ଆକାର(nsbmp_content));
	ଯଦି (bmp == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(bmp);
		ଫେରନ୍ତୁ error;
	}

	error = nsbmp_create_bmp_data(bmp);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(bmp);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) bmp;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର bool nsbmp_convert(ସଂରଚନା content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	ଅଚଳ uint8_t *data;
	size_t size;
	ଅକ୍ଷର *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned ଅକ୍ଷର *) data);
	ବାଛ (res) {
		ସ୍ଥିତି BMP_OK:
			ଭାଙ୍ଗ;
		ସ୍ଥିତି BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
			ଫେରନ୍ତୁ false;
		ସ୍ଥିତି BMP_INSUFFICIENT_DATA:
		ସ୍ଥିତି BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, ଖାଲି);
			ଫେରନ୍ତୁ false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = ଆକାର(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool nsbmp_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ଯଦି (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ଯଦି ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			ଫେରନ୍ତୁ false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ଯଦି (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ଯଦି (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ଫେରନ୍ତୁ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


ସ୍ଥିର ଶୂନ୍ୟ nsbmp_destroy(ସଂରଚନା content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	ମୁକ୍ତ(bmp->bmp);
}


ସ୍ଥିର nserror nsbmp_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, ଆକାର(nsbmp_content));
	ଯଦି (new_bmp == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ଫେରନ୍ତୁ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ଫେରନ୍ତୁ error;
	}

	ଯଦି (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) new_bmp;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ *nsbmp_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ଫେରନ୍ତୁ bmp->bitmap;
}

ସ୍ଥିର content_type nsbmp_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}

ସ୍ଥିର bool nsbmp_content_is_opaque(ସଂରଚନା content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ଯଦି (bmp->bitmap != ଖାଲି) {
		ଫେରନ୍ତୁ guit->bitmap->get_opaque(bmp->bitmap);
	}

	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର ଅଚଳ content_handler nsbmp_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nsbmp_types[] = {
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
