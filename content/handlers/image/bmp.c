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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <libnsbmp.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/bmp.h"

/** bmp context. */
തരനിർവചനം ഘടന nsbmp_content {
	ഘടന content base;

	bmp_image *bmp;	/** BMP image data */

	ഘടന bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
സ്ഥിതം ശൂന്യം *nsbmp_bitmap_create(പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height, unsigned പൂർണ്ണസംഖ്യ bmp_state)
{
	unsigned പൂർണ്ണസംഖ്യ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	തിരികെ guit->bitmap->create(width, height, bitmap_state);
}

സ്ഥിതം nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, വലുപ്പം(ഘടന bmp_image));
	എങ്കിൽ (bmp->bmp == ശൂന്യ) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror
nsbmp_create(സ്ഥിരം ഘടന content_handler *handler,
	     lwc_string *imime_type,
	     സ്ഥിരം ഘടന http_parameter *params,
	     llcache_handle *llcache,
	     സ്ഥിരം അക്ഷരം *fallback_charset,
	     bool quirks,
	     ഘടന content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, വലുപ്പം(nsbmp_content));
	എങ്കിൽ (bmp == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(bmp);
		തിരികെ error;
	}

	error = nsbmp_create_bmp_data(bmp);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(bmp);
		തിരികെ error;
	}

	*c = (ഘടന content *) bmp;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം bool nsbmp_convert(ഘടന content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	സ്ഥിരം uint8_t *data;
	size_t size;
	അക്ഷരം *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned അക്ഷരം *) data);
	തിരഞ്ഞെടുപ്പ് (res) {
		അവസ്ഥ BMP_OK:
			നിർത്തുക;
		അവസ്ഥ BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
			തിരികെ false;
		അവസ്ഥ BMP_INSUFFICIENT_DATA:
		അവസ്ഥ BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, ശൂന്യ);
			തിരികെ false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = വലുപ്പം(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	തിരികെ true;
}

സ്ഥിതം bool nsbmp_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	എങ്കിൽ (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		എങ്കിൽ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			തിരികെ false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	എങ്കിൽ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	എങ്കിൽ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	തിരികെ (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


സ്ഥിതം ശൂന്യം nsbmp_destroy(ഘടന content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	സ്വതന്ത്രം(bmp->bmp);
}


സ്ഥിതം nserror nsbmp_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, വലുപ്പം(nsbmp_content));
	എങ്കിൽ (new_bmp == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		തിരികെ error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		തിരികെ error;
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) new_bmp;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം *nsbmp_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	തിരികെ bmp->bitmap;
}

സ്ഥിതം content_type nsbmp_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}

സ്ഥിതം bool nsbmp_content_is_opaque(ഘടന content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	എങ്കിൽ (bmp->bitmap != ശൂന്യ) {
		തിരികെ guit->bitmap->get_opaque(bmp->bitmap);
	}

	തിരികെ false;
}

സ്ഥിതം സ്ഥിരം content_handler nsbmp_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nsbmp_types[] = {
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
