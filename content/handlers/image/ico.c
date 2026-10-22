/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * implementation for image/ico content handler
 */

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <libnsbmp.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image.h"
#அந்தர்பாவ "image/ico.h"

வகைவரை கட்டமைப்பு nsico_content {
	கட்டமைப்பு content base;

	கட்டமைப்பு ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
நிலையான வெறுமை *nsico_bitmap_create(முழு width, முழு height, unsigned முழு bmp_state)
{
	unsigned முழு bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	திரும்பு guit->bitmap->create(width, height, bitmap_state);
}

நிலையான nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, அளவு(ico_collection));
	எனில் (c->ico == வெற்று) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	திரும்பு NSERROR_OK;
}


நிலையான nserror nsico_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, அளவு(nsico_content));
	எனில் (result == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(result);
		திரும்பு error;
	}

	error = nsico_create_ico_data(result);
	எனில் (error != NSERROR_OK) {
		விடுவி(result);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) result;

	திரும்பு NSERROR_OK;
}



நிலையான bool nsico_convert(கட்டமைப்பு content *c)
{
	nsico_content *ico = (nsico_content *) c;
	கட்டமைப்பு bmp_image *bmp;
	bmp_result res;
	மாறிலி uint8_t *data;
	size_t size;
	எழுத்து *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned எழுத்து *) data);

	தேர்வு (res) {
	நிலை BMP_OK:
		நிறுத்து;
	நிலை BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
	நிலை BMP_INSUFFICIENT_DATA:
	நிலை BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, வெற்று);
		திரும்பு false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	எனில் (bmp == வெற்று) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		திரும்பு false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	திரும்பு true;
}

நிலையான bool nsico__decode(கட்டமைப்பு bmp_image *ico)
{
	எனில் (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		எனில் (bmp_decode(ico) != BMP_OK) {
			திரும்பு false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	திரும்பு true;
}

நிலையான bool nsico_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	கட்டமைப்பு bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	எனில் (bmp == வெற்று) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		திரும்பு false;
	}

	/* ensure its decided */
	எனில் (!nsico__decode(bmp)) {
		திரும்பு false;
	}

	திரும்பு image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


நிலையான வெறுமை nsico_destroy(கட்டமைப்பு content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	விடுவி(ico->ico);
}

நிலையான nserror nsico_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, அளவு(nsico_content));
	எனில் (ico == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&ico->base);
		திரும்பு error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	எனில் (error != NSERROR_OK) {
		content_destroy(&ico->base);
		திரும்பு error;
	}

	எனில் (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		எனில் (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) ico;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை *nsico_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	கட்டமைப்பு bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	எனில் (bmp == வெற்று) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		திரும்பு வெற்று;
	}

	எனில் (!nsico__decode(bmp)) {
		திரும்பு வெற்று;
	}

	திரும்பு bmp->bitmap;
}

நிலையான content_type nsico_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}

நிலையான bool nsico_is_opaque(கட்டமைப்பு content *c)
{
	nsico_content *ico = (nsico_content *) c;
	கட்டமைப்பு bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	எனில் (bmp == வெற்று) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		திரும்பு false;
	}

	எனில் (!nsico__decode(bmp)) {
		திரும்பு false;
	}

	திரும்பு guit->bitmap->get_opaque(bmp->bitmap);
}

நிலையான மாறிலி content_handler nsico_content_handler = {
	.create = nsico_create,
	.data_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_internal = nsico_get_internal,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

நிலையான மாறிலி எழுத்து *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
