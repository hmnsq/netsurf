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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <libnsbmp.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image.h"
#অন্তর্ভাব "image/ico.h"

প্রকারনির্দেশ সংরচনা nsico_content {
	সংরচনা content base;

	সংরচনা ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
স্থির শূন্য *nsico_bitmap_create(পূর্ণাঙ্ক width, পূর্ণাঙ্ক height, unsigned পূর্ণাঙ্ক bmp_state)
{
	unsigned পূর্ণাঙ্ক bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	প্রত্যাগম guit->bitmap->create(width, height, bitmap_state);
}

স্থির nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, পরিমাণ(ico_collection));
	যদি (c->ico == রিক্ত) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	প্রত্যাগম NSERROR_OK;
}


স্থির nserror nsico_create(ধ্রুব content_handler *handler,
		lwc_string *imime_type, ধ্রুব সংরচনা http_parameter *params,
		llcache_handle *llcache, ধ্রুব বর্ণ *fallback_charset,
		bool quirks, সংরচনা content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, পরিমাণ(nsico_content));
	যদি (result == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(result);
		প্রত্যাগম error;
	}

	error = nsico_create_ico_data(result);
	যদি (error != NSERROR_OK) {
		মুক্ত(result);
		প্রত্যাগম error;
	}

	*c = (সংরচনা content *) result;

	প্রত্যাগম NSERROR_OK;
}



স্থির bool nsico_convert(সংরচনা content *c)
{
	nsico_content *ico = (nsico_content *) c;
	সংরচনা bmp_image *bmp;
	bmp_result res;
	ধ্রুব uint8_t *data;
	size_t size;
	বর্ণ *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned বর্ণ *) data);

	বিকল্প (res) {
	পক্ষ BMP_OK:
		ভঙ্গ;
	পক্ষ BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম false;
	পক্ষ BMP_INSUFFICIENT_DATA:
	পক্ষ BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, রিক্ত);
		প্রত্যাগম false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != রিক্ত) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	যদি (bmp == রিক্ত) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		প্রত্যাগম false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	প্রত্যাগম true;
}

স্থির bool nsico__decode(সংরচনা bmp_image *ico)
{
	যদি (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		যদি (bmp_decode(ico) != BMP_OK) {
			প্রত্যাগম false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	প্রত্যাগম true;
}

স্থির bool nsico_redraw(সংরচনা content *c, সংরচনা content_redraw_data *data,
		ধ্রুব সংরচনা rect *clip, ধ্রুব সংরচনা redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	সংরচনা bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	যদি (bmp == রিক্ত) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		প্রত্যাগম false;
	}

	/* ensure its decided */
	যদি (!nsico__decode(bmp)) {
		প্রত্যাগম false;
	}

	প্রত্যাগম image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


স্থির শূন্য nsico_destroy(সংরচনা content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	মুক্ত(ico->ico);
}

স্থির nserror nsico_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, পরিমাণ(nsico_content));
	যদি (ico == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&ico->base);
		প্রত্যাগম error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	যদি (error != NSERROR_OK) {
		content_destroy(&ico->base);
		প্রত্যাগম error;
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			প্রত্যাগম NSERROR_CLONE_FAILED;
		}
	}

	*newc = (সংরচনা content *) ico;

	প্রত্যাগম NSERROR_OK;
}

স্থির শূন্য *nsico_get_internal(ধ্রুব সংরচনা content *c, শূন্য *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	সংরচনা bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	যদি (bmp == রিক্ত) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		প্রত্যাগম রিক্ত;
	}

	যদি (!nsico__decode(bmp)) {
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম bmp->bitmap;
}

স্থির content_type nsico_content_type(শূন্য)
{
	প্রত্যাগম CONTENT_IMAGE;
}

স্থির bool nsico_is_opaque(সংরচনা content *c)
{
	nsico_content *ico = (nsico_content *) c;
	সংরচনা bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	যদি (bmp == রিক্ত) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		প্রত্যাগম false;
	}

	যদি (!nsico__decode(bmp)) {
		প্রত্যাগম false;
	}

	প্রত্যাগম guit->bitmap->get_opaque(bmp->bitmap);
}

স্থির ধ্রুব content_handler nsico_content_handler = {
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

স্থির ধ্রুব বর্ণ *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
