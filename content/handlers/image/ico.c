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

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <libnsbmp.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/image.h"
#समावेश "image/ico.h"

प्रकार काष्ठा nsico_content {
	काष्ठा content base;

	काष्ठा ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
अटल व्योम *nsico_bitmap_create(पूर्णांक width, पूर्णांक height, अचिन्हित पूर्णांक bmp_state)
{
	अचिन्हित पूर्णांक bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	वापस guit->bitmap->create(width, height, bitmap_state);
}

अटल nserror nsico_create_ico_तथ्य(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_बफर = guit->bitmap->get_बफर,
	};

	c->ico = सुस्मृति(1, माप(ico_collection));
	अगर (c->ico == शून्य) {
		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	वापस NSERROR_OK;
}


अटल nserror nsico_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	nsico_content *परिणाम;
	nserror error;

	परिणाम = सुस्मृति(1, माप(nsico_content));
	अगर (परिणाम == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&परिणाम->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(परिणाम);
		वापस error;
	}

	error = nsico_create_ico_तथ्य(परिणाम);
	अगर (error != NSERROR_OK) {
		मुक्त(परिणाम);
		वापस error;
	}

	*c = (काष्ठा content *) परिणाम;

	वापस NSERROR_OK;
}



अटल bool nsico_convert(काष्ठा content *c)
{
	nsico_content *ico = (nsico_content *) c;
	काष्ठा bmp_image *bmp;
	bmp_परिणाम res;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;
	अक्षर *title;

	/* set the ico data */
	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	/* analyse the ico */
	res = ico_analyse(ico->ico, आकार, (अचिन्हित अक्षर *) तथ्य);

	चयन (res) {
	हाल BMP_OK:
		अवरोध;
	हाल BMP_INSUFFICIENT_MEMORY:
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
	हाल BMP_INSUFFICIENT_तथ्य:
	हाल BMP_तथ्य_ERROR:
		content_broadcast_error(c, NSERROR_ICO_ERROR, शून्य);
		वापस false;
	}

	/* Store our content width, height and calculate size */
	c->width = ico->ico->width;
	c->height = ico->ico->height;
	c->आकार += (ico->ico->width * ico->ico->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	अगर (title != शून्य) {
		content__set_title(c, title);
		मुक्त(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_find(ico->ico, 255, 255);
	अगर (bmp == शून्य) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		वापस false;
	}

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	वापस true;
}

अटल bool nsico__decode(काष्ठा bmp_image *ico)
{
	अगर (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		अगर (bmp_decode(ico) != BMP_OK) {
			वापस false;
		}

		bitmap_format_to_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	वापस true;
}

अटल bool nsico_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	काष्ठा bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_find(ico->ico, तथ्य->width, तथ्य->height);
	अगर (bmp == शून्य) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		वापस false;
	}

	/* ensure its decided */
	अगर (!nsico__decode(bmp)) {
		वापस false;
	}

	वापस image_bitmap_plot(bmp->bitmap, तथ्य, clip, ctx);
}


अटल व्योम nsico_destroy(काष्ठा content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	मुक्त(ico->ico);
}

अटल nserror nsico_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = सुस्मृति(1, माप(nsico_content));
	अगर (ico == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &ico->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&ico->base);
		वापस error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_तथ्य(ico);
	अगर (error != NSERROR_OK) {
		content_destroy(&ico->base);
		वापस error;
	}

	अगर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		अगर (nsico_convert(&ico->base) == false) {
			content_destroy(&ico->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) ico;

	वापस NSERROR_OK;
}

अटल व्योम *nsico_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	काष्ठा bmp_image *bmp;

	bmp = ico_find(ico->ico, 16, 16);
	अगर (bmp == शून्य) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		वापस शून्य;
	}

	अगर (!nsico__decode(bmp)) {
		वापस शून्य;
	}

	वापस bmp->bitmap;
}

अटल content_type nsico_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}

अटल bool nsico_is_opaque(काष्ठा content *c)
{
	nsico_content *ico = (nsico_content *) c;
	काष्ठा bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_find(ico->ico, 16, 16);
	अगर (bmp == शून्य) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		वापस false;
	}

	अगर (!nsico__decode(bmp)) {
		वापस false;
	}

	वापस guit->bitmap->get_opaque(bmp->bitmap);
}

अटल स्थिर content_handler nsico_content_handler = {
	.create = nsico_create,
	.तथ्य_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_internal = nsico_get_internal,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_REGISTER_TYPES(nsico, nsico_types, nsico_content_handler);
