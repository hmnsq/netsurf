/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Sean Fox <dyntryx@gmail.com>
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
 * implementation of content handler for BMP images.
 */

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <libnsbmp.h>

#समावेश "utils/utils.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/bmp.h"

/** bmp context. */
प्रकार काष्ठा nsbmp_content {
	काष्ठा content base;

	bmp_image *bmp;	/** BMP image data */

	काष्ठा bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
अटल व्योम *nsbmp_bitmap_create(पूर्णांक width, पूर्णांक height, अचिन्हित पूर्णांक bmp_state)
{
	अचिन्हित पूर्णांक bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_MEMORY) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	वापस guit->bitmap->create(width, height, bitmap_state);
}

अटल nserror nsbmp_create_bmp_तथ्य(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_बफर = guit->bitmap->get_बफर,
	};

	bmp->bmp = सुस्मृति(1, माप(काष्ठा bmp_image));
	अगर (bmp->bmp == शून्य) {
		content_broadcast_error(&bmp->base, NSERROR_NOMEM, शून्य);
		वापस NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	वापस NSERROR_OK;
}

अटल nserror
nsbmp_create(स्थिर काष्ठा content_handler *handler,
	     lwc_string *imime_type,
	     स्थिर काष्ठा http_parameter *params,
	     llcache_handle *llcache,
	     स्थिर अक्षर *fallback_charset,
	     bool quirks,
	     काष्ठा content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = सुस्मृति(1, माप(nsbmp_content));
	अगर (bmp == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&bmp->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(bmp);
		वापस error;
	}

	error = nsbmp_create_bmp_तथ्य(bmp);
	अगर (error != NSERROR_OK) {
		मुक्त(bmp);
		वापस error;
	}

	*c = (काष्ठा content *) bmp;

	वापस NSERROR_OK;
}

अटल bool nsbmp_convert(काष्ठा content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_परिणाम res;
	uint32_t swidth;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;
	अक्षर *title;

	/* set the bmp data */
	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, आकार, (अचिन्हित अक्षर *) तथ्य);
	चयन (res) {
		हाल BMP_OK:
			अवरोध;
		हाल BMP_INSUFFICIENT_MEMORY:
			content_broadcast_error(c, NSERROR_NOMEM, शून्य);
			वापस false;
		हाल BMP_INSUFFICIENT_तथ्य:
		हाल BMP_तथ्य_ERROR:
			content_broadcast_error(c, NSERROR_BMP_ERROR, शून्य);
			वापस false;
	}

	/* Store our content width and description */
	c->width = bmp->bmp->width;
	c->height = bmp->bmp->height;
	swidth = माप(uint32_t) * bmp->bmp->width;
	c->आकार += (swidth * bmp->bmp->height) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	अगर (title != शून्य) {
		content__set_title(c, title);
		मुक्त(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	वापस true;
}

अटल bool nsbmp_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	अगर (bmp->bmp->decoded == false) {
		bmp_परिणाम res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		अगर ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_तथ्य) &&
		    (res != BMP_तथ्य_ERROR)) {
			वापस false;
		}

		bitmap_format_to_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	अगर (तथ्य->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	अगर (तथ्य->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	वापस (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  तथ्य->x, तथ्य->y,
				  तथ्य->width, तथ्य->height,
				  तथ्य->background_colour,
				  flags) == NSERROR_OK);
}


अटल व्योम nsbmp_destroy(काष्ठा content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	मुक्त(bmp->bmp);
}


अटल nserror nsbmp_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = सुस्मृति(1, माप(nsbmp_content));
	अगर (new_bmp == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		वापस error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_तथ्य(new_bmp);
	अगर (error != NSERROR_OK) {
		content_destroy(&new_bmp->base);
		वापस error;
	}

	अगर (old->status == CONTENT_STATUS_READY || 
			old->status == CONTENT_STATUS_DONE) {
		अगर (nsbmp_convert(&new_bmp->base) == false) {
			content_destroy(&new_bmp->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) new_bmp;

	वापस NSERROR_OK;
}

अटल व्योम *nsbmp_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	वापस bmp->bitmap;
}

अटल content_type nsbmp_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}

अटल bool nsbmp_content_is_opaque(काष्ठा content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	अगर (bmp->bitmap != शून्य) {
		वापस guit->bitmap->get_opaque(bmp->bitmap);
	}

	वापस false;
}

अटल स्थिर content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.तथ्य_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_internal = nsbmp_get_internal,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *nsbmp_types[] = {
	"application/bmp",
	"application/preview",
	"application/x-bmp",
	"application/x-win-bitmap",
	"image/bmp",
	"image/ms-bmp",
	"image/x-bitmap",
	"image/x-bmp",
	"image/x-ms-bmp",
	"image/x-win-bitmap",
	"image/x-windows-bmp",
	"image/x-xbitmap"
};

CONTENT_FACTORY_REGISTER_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
