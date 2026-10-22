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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <libnsbmp.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image.h"
#ಅನ್ತರ್ಭಾವ "image/ico.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nsico_content {
	ಸಂರಚನಾ content base;

	ಸಂರಚನಾ ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಶೂನ್ಯ *nsico_bitmap_create(ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height, unsigned ಪೂರ್ಣಾಙ್ಕ bmp_state)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ಪ್ರತ್ಯಾಗಮ guit->bitmap->create(width, height, bitmap_state);
}

ಸ್ಥಿರ nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, ಪರಿಮಾಣ(ico_collection));
	ಯದಿ (c->ico == ರಿಕ್ತ) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ nserror nsico_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, ಪರಿಮಾಣ(nsico_content));
	ಯದಿ (result == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = nsico_create_ico_data(result);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) result;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}



ಸ್ಥಿರ bool nsico_convert(ಸಂರಚನಾ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ಸಂರಚನಾ bmp_image *bmp;
	bmp_result res;
	ಧ್ರುವ uint8_t *data;
	size_t size;
	ವರ್ಣ *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned ವರ್ಣ *) data);

	ವಿಕಲ್ಪ (res) {
	ಪಕ್ಷ BMP_OK:
		ಭಙ್ಗ;
	ಪಕ್ಷ BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	ಪಕ್ಷ BMP_INSUFFICIENT_DATA:
	ಪಕ್ಷ BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	ಯದಿ (bmp == ರಿಕ್ತ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool nsico__decode(ಸಂರಚನಾ bmp_image *ico)
{
	ಯದಿ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ಯದಿ (bmp_decode(ico) != BMP_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool nsico_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ಸಂರಚನಾ bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	ಯದಿ (bmp == ರಿಕ್ತ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* ensure its decided */
	ಯದಿ (!nsico__decode(bmp)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


ಸ್ಥಿರ ಶೂನ್ಯ nsico_destroy(ಸಂರಚನಾ content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	ಮುಕ್ತ(ico->ico);
}

ಸ್ಥಿರ nserror nsico_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, ಪರಿಮಾಣ(nsico_content));
	ಯದಿ (ico == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) ico;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *nsico_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ಸಂರಚನಾ bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	ಯದಿ (bmp == ರಿಕ್ತ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (!nsico__decode(bmp)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ bmp->bitmap;
}

ಸ್ಥಿರ content_type nsico_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}

ಸ್ಥಿರ bool nsico_is_opaque(ಸಂರಚನಾ content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ಸಂರಚನಾ bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	ಯದಿ (bmp == ರಿಕ್ತ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!nsico__decode(bmp)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(bmp->bitmap);
}

ಸ್ಥಿರ ಧ್ರುವ content_handler nsico_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
