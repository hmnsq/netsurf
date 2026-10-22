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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <libnsbmp.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image.h"
#അന്തര്ഭാവ "image/ico.h"

തരനിർവചനം ഘടന nsico_content {
	ഘടന content base;

	ഘടന ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
സ്ഥിതം ശൂന്യം *nsico_bitmap_create(പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height, unsigned പൂർണ്ണസംഖ്യ bmp_state)
{
	unsigned പൂർണ്ണസംഖ്യ bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	തിരികെ guit->bitmap->create(width, height, bitmap_state);
}

സ്ഥിതം nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, വലുപ്പം(ico_collection));
	എങ്കിൽ (c->ico == ശൂന്യ) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	തിരികെ NSERROR_OK;
}


സ്ഥിതം nserror nsico_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, വലുപ്പം(nsico_content));
	എങ്കിൽ (result == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	error = nsico_create_ico_data(result);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	*c = (ഘടന content *) result;

	തിരികെ NSERROR_OK;
}



സ്ഥിതം bool nsico_convert(ഘടന content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ഘടന bmp_image *bmp;
	bmp_result res;
	സ്ഥിരം uint8_t *data;
	size_t size;
	അക്ഷരം *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned അക്ഷരം *) data);

	തിരഞ്ഞെടുപ്പ് (res) {
	അവസ്ഥ BMP_OK:
		നിർത്തുക;
	അവസ്ഥ BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
	അവസ്ഥ BMP_INSUFFICIENT_DATA:
	അവസ്ഥ BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, ശൂന്യ);
		തിരികെ false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	എങ്കിൽ (bmp == ശൂന്യ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		തിരികെ false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	തിരികെ true;
}

സ്ഥിതം bool nsico__decode(ഘടന bmp_image *ico)
{
	എങ്കിൽ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		എങ്കിൽ (bmp_decode(ico) != BMP_OK) {
			തിരികെ false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	തിരികെ true;
}

സ്ഥിതം bool nsico_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	ഘടന bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	എങ്കിൽ (bmp == ശൂന്യ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		തിരികെ false;
	}

	/* ensure its decided */
	എങ്കിൽ (!nsico__decode(bmp)) {
		തിരികെ false;
	}

	തിരികെ image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


സ്ഥിതം ശൂന്യം nsico_destroy(ഘടന content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	സ്വതന്ത്രം(ico->ico);
}

സ്ഥിതം nserror nsico_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, വലുപ്പം(nsico_content));
	എങ്കിൽ (ico == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		തിരികെ error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&ico->base);
		തിരികെ error;
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) ico;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം *nsico_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	ഘടന bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	എങ്കിൽ (bmp == ശൂന്യ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (!nsico__decode(bmp)) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ bmp->bitmap;
}

സ്ഥിതം content_type nsico_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}

സ്ഥിതം bool nsico_is_opaque(ഘടന content *c)
{
	nsico_content *ico = (nsico_content *) c;
	ഘടന bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	എങ്കിൽ (bmp == ശൂന്യ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		തിരികെ false;
	}

	എങ്കിൽ (!nsico__decode(bmp)) {
		തിരികെ false;
	}

	തിരികെ guit->bitmap->get_opaque(bmp->bitmap);
}

സ്ഥിതം സ്ഥിരം content_handler nsico_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
