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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <libnsbmp.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image.h"
#ଅନ୍ତର୍ଭାଵ "image/ico.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nsico_content {
	ସଂରଚନା content base;

	ସଂରଚନା ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ସ୍ଥିର ଶୂନ୍ୟ *nsico_bitmap_create(ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bmp_state)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ଫେରନ୍ତୁ guit->bitmap->create(width, height, bitmap_state);
}

ସ୍ଥିର nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, ଆକାର(ico_collection));
	ଯଦି (c->ico == ଖାଲି) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର nserror nsico_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, ଆକାର(nsico_content));
	ଯଦି (result == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	error = nsico_create_ico_data(result);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) result;

	ଫେରନ୍ତୁ NSERROR_OK;
}



ସ୍ଥିର bool nsico_convert(ସଂରଚନା content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ସଂରଚନା bmp_image *bmp;
	bmp_result res;
	ଅଚଳ uint8_t *data;
	size_t size;
	ଅକ୍ଷର *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned ଅକ୍ଷର *) data);

	ବାଛ (res) {
	ସ୍ଥିତି BMP_OK:
		ଭାଙ୍ଗ;
	ସ୍ଥିତି BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	ସ୍ଥିତି BMP_INSUFFICIENT_DATA:
	ସ୍ଥିତି BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	ଯଦି (bmp == ଖାଲି) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ଫେରନ୍ତୁ false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool nsico__decode(ସଂରଚନା bmp_image *ico)
{
	ଯଦି (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ଯଦି (bmp_decode(ico) != BMP_OK) {
			ଫେରନ୍ତୁ false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool nsico_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ସଂରଚନା bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	ଯଦି (bmp == ଖାଲି) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ଫେରନ୍ତୁ false;
	}

	/* ensure its decided */
	ଯଦି (!nsico__decode(bmp)) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


ସ୍ଥିର ଶୂନ୍ୟ nsico_destroy(ସଂରଚନା content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	ମୁକ୍ତ(ico->ico);
}

ସ୍ଥିର nserror nsico_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, ଆକାର(nsico_content));
	ଯଦି (ico == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ଫେରନ୍ତୁ error;
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) ico;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ *nsico_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ସଂରଚନା bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	ଯଦି (bmp == ଖାଲି) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (!nsico__decode(bmp)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ bmp->bitmap;
}

ସ୍ଥିର content_type nsico_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}

ସ୍ଥିର bool nsico_is_opaque(ସଂରଚନା content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ସଂରଚନା bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	ଯଦି (bmp == ଖାଲି) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!nsico__decode(bmp)) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ guit->bitmap->get_opaque(bmp->bitmap);
}

ସ୍ଥିର ଅଚଳ content_handler nsico_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
