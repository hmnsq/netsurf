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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <libnsbmp.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/image.h"
#અન્તર્ભાવ "image/ico.h"

પ્રકારવ્યાખ્યા સંરચના nsico_content {
	સંરચના content base;

	સંરચના ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
સ્થિર શૂન્ય *nsico_bitmap_create(પૂર્ણાંક width, પૂર્ણાંક height, unsigned પૂર્ણાંક bmp_state)
{
	unsigned પૂર્ણાંક bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	પાછા guit->bitmap->create(width, height, bitmap_state);
}

સ્થિર nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, કદ(ico_collection));
	જો (c->ico == ખાલી) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	પાછા NSERROR_OK;
}


સ્થિર nserror nsico_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, કદ(nsico_content));
	જો (result == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(result);
		પાછા error;
	}

	error = nsico_create_ico_data(result);
	જો (error != NSERROR_OK) {
		મુક્ત(result);
		પાછા error;
	}

	*c = (સંરચના content *) result;

	પાછા NSERROR_OK;
}



સ્થિર bool nsico_convert(સંરચના content *c)
{
	nsico_content *ico = (nsico_content *) c;
	સંરચના bmp_image *bmp;
	bmp_result res;
	અચલ uint8_t *data;
	size_t size;
	અક્ષર *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned અક્ષર *) data);

	પસંદગી (res) {
	સ્થિતિ BMP_OK:
		તોડો;
	સ્થિતિ BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
	સ્થિતિ BMP_INSUFFICIENT_DATA:
	સ્થિતિ BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, ખાલી);
		પાછા false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	જો (bmp == ખાલી) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		પાછા false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	પાછા true;
}

સ્થિર bool nsico__decode(સંરચના bmp_image *ico)
{
	જો (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		જો (bmp_decode(ico) != BMP_OK) {
			પાછા false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	પાછા true;
}

સ્થિર bool nsico_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	સંરચના bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	જો (bmp == ખાલી) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		પાછા false;
	}

	/* ensure its decided */
	જો (!nsico__decode(bmp)) {
		પાછા false;
	}

	પાછા image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


સ્થિર શૂન્ય nsico_destroy(સંરચના content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	મુક્ત(ico->ico);
}

સ્થિર nserror nsico_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, કદ(nsico_content));
	જો (ico == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	જો (error != NSERROR_OK) {
		content_destroy(&ico->base);
		પાછા error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	જો (error != NSERROR_OK) {
		content_destroy(&ico->base);
		પાછા error;
	}

	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) ico;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય *nsico_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	સંરચના bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	જો (bmp == ખાલી) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		પાછા ખાલી;
	}

	જો (!nsico__decode(bmp)) {
		પાછા ખાલી;
	}

	પાછા bmp->bitmap;
}

સ્થિર content_type nsico_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}

સ્થિર bool nsico_is_opaque(સંરચના content *c)
{
	nsico_content *ico = (nsico_content *) c;
	સંરચના bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	જો (bmp == ખાલી) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		પાછા false;
	}

	જો (!nsico__decode(bmp)) {
		પાછા false;
	}

	પાછા guit->bitmap->get_opaque(bmp->bitmap);
}

સ્થિર અચલ content_handler nsico_content_handler = {
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

સ્થિર અચલ અક્ષર *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
