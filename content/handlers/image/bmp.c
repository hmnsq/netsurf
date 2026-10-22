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

#include <stdbool.h>
#include <stdlib.h>
#include <libnsbmp.h>

#include "utils/utils.h"
#include "utils/messages.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/bmp.h"

/** bmp context. */
typedef sa_nracanaa nsbmp_content {
	sa_nracanaa content base;

	bmp_image *bmp;	/** BMP image data */

	sa_nracanaa bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
static void *nsbmp_bitmap_create(puur_naa_nka width, puur_naa_nka height, unsigned puur_naa_nka bmp_state)
{
	unsigned puur_naa_nka bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ᬯᬮᬶ guit->bitmap->create(width, height, bitmap_state);
}

static nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, parimaa_na(sa_nracanaa bmp_image));
	ᬬᬾᬦ᭄ (bmp->bmp == NULL) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, NULL);
		ᬯᬮᬶ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	ᬯᬮᬶ NSERROR_OK;
}

static nserror
nsbmp_create(const sa_nracanaa content_handler *handler,
	     lwc_string *imime_type,
	     const sa_nracanaa http_parameter *params,
	     llcache_handle *llcache,
	     const var_na *fallback_charset,
	     bool quirks,
	     sa_nracanaa content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, parimaa_na(nsbmp_content));
	ᬬᬾᬦ᭄ (bmp == NULL)
		ᬯᬮᬶ NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(bmp);
		ᬯᬮᬶ error;
	}

	error = nsbmp_create_bmp_data(bmp);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(bmp);
		ᬯᬮᬶ error;
	}

	*c = (sa_nracanaa content *) bmp;

	ᬯᬮᬶ NSERROR_OK;
}

static bool nsbmp_convert(sa_nracanaa content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	const uint8_t *data;
	size_t size;
	var_na *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned var_na *) data);
	switch (res) {
		pak_sa BMP_OK:
			bha_nga;
		pak_sa BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, NULL);
			ᬯᬮᬶ false;
		pak_sa BMP_INSUFFICIENT_DATA:
		pak_sa BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, NULL);
			ᬯᬮᬶ false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = parimaa_na(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ᬬᬾᬦ᭄ (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ᬯᬮᬶ true;
}

static bool nsbmp_redraw(sa_nracanaa content *c, sa_nracanaa content_redraw_data *data,
		const sa_nracanaa rect *clip, const sa_nracanaa redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ᬬᬾᬦ᭄ (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ᬬᬾᬦ᭄ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			ᬯᬮᬶ false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ᬬᬾᬦ᭄ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ᬬᬾᬦ᭄ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ᬯᬮᬶ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


static void nsbmp_destroy(sa_nracanaa content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	free(bmp->bmp);
}


static nserror nsbmp_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, parimaa_na(nsbmp_content));
	ᬬᬾᬦ᭄ (new_bmp == NULL)
		ᬯᬮᬶ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ᬯᬮᬶ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		ᬯᬮᬶ error;
	}

	ᬬᬾᬦ᭄ (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		ᬬᬾᬦ᭄ (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			ᬯᬮᬶ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) new_bmp;

	ᬯᬮᬶ NSERROR_OK;
}

static void *nsbmp_get_internal(const sa_nracanaa content *c, void *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ᬯᬮᬶ bmp->bitmap;
}

static content_type nsbmp_content_type(void)
{
	ᬯᬮᬶ CONTENT_IMAGE;
}

static bool nsbmp_content_is_opaque(sa_nracanaa content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ᬬᬾᬦ᭄ (bmp->bitmap != NULL) {
		ᬯᬮᬶ guit->bitmap->get_opaque(bmp->bitmap);
	}

	ᬯᬮᬶ false;
}

static const content_handler nsbmp_content_handler = {
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

static const var_na *nsbmp_types[] = {
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
