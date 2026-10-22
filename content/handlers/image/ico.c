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

#include <stdbool.h>
#include <stdlib.h>
#include <libnsbmp.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/image.h"
#include "image/ico.h"

typedef sa_nracanaa nsico_content {
	sa_nracanaa content base;

	sa_nracanaa ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
static void *nsico_bitmap_create(puur_naa_nka width, puur_naa_nka height, unsigned puur_naa_nka bmp_state)
{
	unsigned puur_naa_nka bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	ᬯᬮᬶ guit->bitmap->create(width, height, bitmap_state);
}

static nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, parimaa_na(ico_collection));
	ᬬᬾᬦ᭄ (c->ico == NULL) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ᬯᬮᬶ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	ᬯᬮᬶ NSERROR_OK;
}


static nserror nsico_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, parimaa_na(nsico_content));
	ᬬᬾᬦ᭄ (result == NULL)
		ᬯᬮᬶ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(result);
		ᬯᬮᬶ error;
	}

	error = nsico_create_ico_data(result);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		free(result);
		ᬯᬮᬶ error;
	}

	*c = (sa_nracanaa content *) result;

	ᬯᬮᬶ NSERROR_OK;
}



static bool nsico_convert(sa_nracanaa content *c)
{
	nsico_content *ico = (nsico_content *) c;
	sa_nracanaa bmp_image *bmp;
	bmp_result res;
	const uint8_t *data;
	size_t size;
	var_na *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned var_na *) data);

	switch (res) {
	pak_sa BMP_OK:
		bha_nga;
	pak_sa BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ᬯᬮᬶ false;
	pak_sa BMP_INSUFFICIENT_DATA:
	pak_sa BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, NULL);
		ᬯᬮᬶ false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ᬬᬾᬦ᭄ (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	ᬬᬾᬦ᭄ (bmp == NULL) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ᬯᬮᬶ false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ᬯᬮᬶ true;
}

static bool nsico__decode(sa_nracanaa bmp_image *ico)
{
	ᬬᬾᬦ᭄ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ᬬᬾᬦ᭄ (bmp_decode(ico) != BMP_OK) {
			ᬯᬮᬶ false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	ᬯᬮᬶ true;
}

static bool nsico_redraw(sa_nracanaa content *c, sa_nracanaa content_redraw_data *data,
		const sa_nracanaa rect *clip, const sa_nracanaa redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	sa_nracanaa bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	ᬬᬾᬦ᭄ (bmp == NULL) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ᬯᬮᬶ false;
	}

	/* ensure its decided */
	ᬬᬾᬦ᭄ (!nsico__decode(bmp)) {
		ᬯᬮᬶ false;
	}

	ᬯᬮᬶ image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


static void nsico_destroy(sa_nracanaa content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	free(ico->ico);
}

static nserror nsico_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, parimaa_na(nsico_content));
	ᬬᬾᬦ᭄ (ico == NULL)
		ᬯᬮᬶ NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ᬯᬮᬶ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	ᬬᬾᬦ᭄ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		ᬯᬮᬶ error;
	}

	ᬬᬾᬦ᭄ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ᬬᬾᬦ᭄ (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			ᬯᬮᬶ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) ico;

	ᬯᬮᬶ NSERROR_OK;
}

static void *nsico_get_internal(const sa_nracanaa content *c, void *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	sa_nracanaa bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	ᬬᬾᬦ᭄ (bmp == NULL) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ᬯᬮᬶ NULL;
	}

	ᬬᬾᬦ᭄ (!nsico__decode(bmp)) {
		ᬯᬮᬶ NULL;
	}

	ᬯᬮᬶ bmp->bitmap;
}

static content_type nsico_content_type(void)
{
	ᬯᬮᬶ CONTENT_IMAGE;
}

static bool nsico_is_opaque(sa_nracanaa content *c)
{
	nsico_content *ico = (nsico_content *) c;
	sa_nracanaa bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	ᬬᬾᬦ᭄ (bmp == NULL) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		ᬯᬮᬶ false;
	}

	ᬬᬾᬦ᭄ (!nsico__decode(bmp)) {
		ᬯᬮᬶ false;
	}

	ᬯᬮᬶ guit->bitmap->get_opaque(bmp->bitmap);
}

static const content_handler nsico_content_handler = {
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

static const var_na *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
