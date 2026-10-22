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

#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <libnsbmp.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image.h"
#ضمن "image/ico.h"

تعريفنوع بنية nsico_content {
	بنية content base;

	بنية ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ساكن فراغ *nsico_bitmap_create(صحيح width, صحيح height, unsigned صحيح bmp_state)
{
	unsigned صحيح bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	أرجع guit->bitmap->create(width, height, bitmap_state);
}

ساكن nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, حجم(ico_collection));
	𐤀𐤌 (c->ico == عدم) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	أرجع NSERROR_OK;
}


ساكن nserror nsico_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, حجم(nsico_content));
	𐤀𐤌 (result == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	𐤀𐤌 (error != NSERROR_OK) {
		حرر(result);
		أرجع error;
	}

	error = nsico_create_ico_data(result);
	𐤀𐤌 (error != NSERROR_OK) {
		حرر(result);
		أرجع error;
	}

	*c = (بنية content *) result;

	أرجع NSERROR_OK;
}



ساكن bool nsico_convert(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;
	بنية bmp_image *bmp;
	bmp_result res;
	ثابت uint8_t *data;
	size_t size;
	حرف *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned حرف *) data);

	بدل (res) {
	حالة BMP_OK:
		اكسر;
	حالة BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	حالة BMP_INSUFFICIENT_DATA:
	حالة BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, عدم);
		أرجع false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	𐤀𐤌 (title != عدم) {
		content__set_title(c, title);
		حرر(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	𐤀𐤌 (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	أرجع true;
}

ساكن bool nsico__decode(بنية bmp_image *ico)
{
	𐤀𐤌 (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		𐤀𐤌 (bmp_decode(ico) != BMP_OK) {
			أرجع false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	أرجع true;
}

ساكن bool nsico_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	بنية bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	𐤀𐤌 (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	/* ensure its decided */
	𐤀𐤌 (!nsico__decode(bmp)) {
		أرجع false;
	}

	أرجع image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


ساكن فراغ nsico_destroy(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	حرر(ico->ico);
}

ساكن nserror nsico_clone(ثابت بنية content *old, بنية content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, حجم(nsico_content));
	𐤀𐤌 (ico == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&ico->base);
		أرجع error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	𐤀𐤌 (error != NSERROR_OK) {
		content_destroy(&ico->base);
		أرجع error;
	}

	𐤀𐤌 (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		𐤀𐤌 (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) ico;

	أرجع NSERROR_OK;
}

ساكن فراغ *nsico_get_internal(ثابت بنية content *c, فراغ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	بنية bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	𐤀𐤌 (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع عدم;
	}

	𐤀𐤌 (!nsico__decode(bmp)) {
		أرجع عدم;
	}

	أرجع bmp->bitmap;
}

ساكن content_type nsico_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}

ساكن bool nsico_is_opaque(بنية content *c)
{
	nsico_content *ico = (nsico_content *) c;
	بنية bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	𐤀𐤌 (bmp == عدم) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		أرجع false;
	}

	𐤀𐤌 (!nsico__decode(bmp)) {
		أرجع false;
	}

	أرجع guit->bitmap->get_opaque(bmp->bitmap);
}

ساكن ثابت content_handler nsico_content_handler = {
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

ساكن ثابت حرف *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
