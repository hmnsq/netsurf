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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <libnsbmp.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/bmp.h"

/** bmp context. */
प्रकारनिर्देश संरचना nsbmp_content {
	संरचना content base;

	bmp_image *bmp;	/** BMP image data */

	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
स्थिर शून्य *nsbmp_bitmap_create(पूर्णाङ्क width, पूर्णाङ्क height, unsigned पूर्णाङ्क bmp_state)
{
	unsigned पूर्णाङ्क bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	प्रत्यागम guit->bitmap->create(width, height, bitmap_state);
}

स्थिर nserror nsbmp_create_bmp_data(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	bmp->bmp = calloc(1, परिमाण(संरचना bmp_image));
	यदि (bmp->bmp == रिक्त) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, रिक्त);
		प्रत्यागम NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	प्रत्यागम NSERROR_OK;
}

स्थिर nserror
nsbmp_create(ध्रुव संरचना content_handler *handler,
	     lwc_string *imime_type,
	     ध्रुव संरचना http_parameter *params,
	     llcache_handle *llcache,
	     ध्रुव वर्ण *fallback_charset,
	     bool quirks,
	     संरचना content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = calloc(1, परिमाण(nsbmp_content));
	यदि (bmp == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(bmp);
		प्रत्यागम error;
	}

	error = nsbmp_create_bmp_data(bmp);
	यदि (error != NSERROR_OK) {
		मुक्त(bmp);
		प्रत्यागम error;
	}

	*c = (संरचना content *) bmp;

	प्रत्यागम NSERROR_OK;
}

स्थिर bool nsbmp_convert(संरचना content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_result res;
	uint32_t swidth;
	ध्रुव uint8_t *data;
	size_t size;
	वर्ण *title;

	/* set the bmp data */
	data = content__get_source_data(c, &size);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, size, (unsigned वर्ण *) data);
	विकल्प (res) {
		पक्ष BMP_OK:
			भङ्ग;
		पक्ष BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
			प्रत्यागम false;
		पक्ष BMP_INSUFFICIENT_DATA:
		पक्ष BMP_DATA_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, रिक्त);
			प्रत्यागम false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = परिमाण(uint32_t) * bmp->bmp->width;
	c->size += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	यदि (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	प्रत्यागम true;
}

स्थिर bool nsbmp_redraw(संरचना content *c, संरचना content_redraw_data *data,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	यदि (bmp->bmp->decoded == false) {
		bmp_result res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		यदि ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_DATA) &&
		    (res != BMP_DATA_ERROR)) {
			प्रत्यागम false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	यदि (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	यदि (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	प्रत्यागम (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}


स्थिर शून्य nsbmp_destroy(संरचना content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	मुक्त(bmp->bmp);
}


स्थिर nserror nsbmp_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = calloc(1, परिमाण(nsbmp_content));
	यदि (new_bmp == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		प्रत्यागम error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_data(new_bmp);
	यदि (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		प्रत्यागम error;
	}

	यदि (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		यदि (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) new_bmp;

	प्रत्यागम NSERROR_OK;
}

स्थिर शून्य *nsbmp_get_internal(ध्रुव संरचना content *c, शून्य *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	प्रत्यागम bmp->bitmap;
}

स्थिर content_type nsbmp_content_type(शून्य)
{
	प्रत्यागम CONTENT_IMAGE;
}

स्थिर bool nsbmp_content_is_opaque(संरचना content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	यदि (bmp->bitmap != रिक्त) {
		प्रत्यागम guit->bitmap->get_opaque(bmp->bitmap);
	}

	प्रत्यागम false;
}

स्थिर ध्रुव content_handler nsbmp_content_handler = {
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

स्थिर ध्रुव वर्ण *nsbmp_types[] = {
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
