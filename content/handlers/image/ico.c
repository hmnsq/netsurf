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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <libnsbmp.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image.h"
#अन्तर्भाव "image/ico.h"

प्रकारनिर्देश संरचना nsico_content {
	संरचना content base;

	संरचना ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
स्थिर शून्य *nsico_bitmap_create(पूर्णाङ्क width, पूर्णाङ्क height, unsigned पूर्णाङ्क bmp_state)
{
	unsigned पूर्णाङ्क bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	पटिनिवत्त guit->bitmap->create(width, height, bitmap_state);
}

स्थिर nserror nsico_create_ico_data(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_buffer = guit->bitmap->get_buffer,
	};

	c->ico = calloc(1, परिमाण(ico_collection));
	सचे (c->ico == रिक्त) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		पटिनिवत्त NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	पटिनिवत्त NSERROR_OK;
}


स्थिर nserror nsico_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	nsico_content *result;
	nserror error;

	result = calloc(1, परिमाण(nsico_content));
	सचे (result == रिक्त)
		पटिनिवत्त NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	सचे (error != NSERROR_OK) {
		मुक्त(result);
		पटिनिवत्त error;
	}

	error = nsico_create_ico_data(result);
	सचे (error != NSERROR_OK) {
		मुक्त(result);
		पटिनिवत्त error;
	}

	*c = (संरचना content *) result;

	पटिनिवत्त NSERROR_OK;
}



स्थिर bool nsico_convert(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;
	संरचना bmp_image *bmp;
	bmp_result res;
	ध्रुव uint8_t *data;
	size_t size;
	वर्ण *title;

	/* set the ico data */
	data = content__get_source_data(c, &size);

	/* analyse the ico */
	res = ico_analyse(ico->ico, size, (unsigned वर्ण *) data);

	विकल्प (res) {
	पक्ष BMP_OK:
		भङ्ग;
	पक्ष BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		पटिनिवत्त false;
	पक्ष BMP_INSUFFICIENT_DATA:
	पक्ष BMP_DATA_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, रिक्त);
		पटिनिवत्त false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->size += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	सचे (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	सचे (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		पटिनिवत्त false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	पटिनिवत्त true;
}

स्थिर bool nsico__decode(संरचना bmp_image *ico)
{
	सचे (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		सचे (bmp_decode(ico) != BMP_OK) {
			पटिनिवत्त false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	पटिनिवत्त true;
}

स्थिर bool nsico_redraw(संरचना content *c, संरचना content_redraw_data *data,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	संरचना bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, data->width, data->height);
	सचे (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		पटिनिवत्त false;
	}

	/* ensure its decided */
	सचे (!nsico__decode(bmp)) {
		पटिनिवत्त false;
	}

	पटिनिवत्त image_bitmap_plot(bmp->bitmap, data, clip, ctx);
}


स्थिर शून्य nsico_destroy(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	मुक्त(ico->ico);
}

स्थिर nserror nsico_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = calloc(1, परिमाण(nsico_content));
	सचे (ico == रिक्त)
		पटिनिवत्त NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	सचे (error != NSERROR_OK) {
		content_destroy(&ico->base);
		पटिनिवत्त error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_data(ico);
	सचे (error != NSERROR_OK) {
		content_destroy(&ico->base);
		पटिनिवत्त error;
	}

	सचे (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		सचे (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			पटिनिवत्त NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) ico;

	पटिनिवत्त NSERROR_OK;
}

स्थिर शून्य *nsico_get_internal(ध्रुव संरचना content *c, शून्य *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	संरचना bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	सचे (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		पटिनिवत्त रिक्त;
	}

	सचे (!nsico__decode(bmp)) {
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त bmp->bitmap;
}

स्थिर content_type nsico_content_type(शून्य)
{
	पटिनिवत्त CONTENT_IMAGE;
}

स्थिर bool nsico_is_opaque(संरचना content *c)
{
	nsico_content *ico = (nsico_content *) c;
	संरचना bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	सचे (bmp == रिक्त) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		पटिनिवत्त false;
	}

	सचे (!nsico__decode(bmp)) {
		पटिनिवत्त false;
	}

	पटिनिवत्त guit->bitmap->get_opaque(bmp->bitmap);
}

स्थिर ध्रुव content_handler nsico_content_handler = {
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

स्थिर ध्रुव वर्ण *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
