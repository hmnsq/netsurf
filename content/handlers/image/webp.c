/*
 * Copyright 2019 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handling for image/webp
 *
 * This implementation uses the google webp library.
 * Image cache handling is performed by the generic NetSurf handler.
 */

#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <setjmp.h>

#समावेश <webp/decode.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/bitmap.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/image_cache.h"

#समावेश "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
अटल nserror
webp_create(स्थिर content_handler *handler,
	      lwc_string *imime_type,
	      स्थिर काष्ठा http_parameter *params,
	      llcache_handle *llcache,
	      स्थिर अक्षर *fallback_charset,
	      bool quirks,
	      काष्ठा content **c)
{
	काष्ठा content *webp_c; /* webp content object */
	nserror res;

	webp_c = सुस्मृति(1, माप(काष्ठा content));
	अगर (webp_c == शून्य) {
		वापस NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	अगर (res != NSERROR_OK) {
		मुक्त(webp_c);
		वापस res;
	}

	*c = webp_c;

	वापस NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
अटल काष्ठा bitmap *
webp_cache_convert(काष्ठा content *c)
{
	स्थिर uint8_t *source_तथ्य; /* webp source data */
	माप_प्रकार source_आकार; /* length of webp source data */
	VP8StatusCode webpres;
	WebPBitstreamFeatures webpfeatures;
	अचिन्हित पूर्णांक bmap_flags;
	uint8_t *pixels = शून्य;
	uint8_t *decoded;
	माप_प्रकार rowstride;
	काष्ठा bitmap *bitmap = शून्य;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	source_तथ्य = content_get_source_तथ्य_2(c, &source_आकार);

	webpres = WebPGetFeatures(source_तथ्य, source_आकार, &webpfeatures);

	अगर (webpres != VP8_STATUS_OK) {
		वापस शून्य;
	}

	अगर (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} अन्यथा {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.width,
				      webpfeatures.height,
				      bmap_flags);
	अगर (bitmap == शून्य) {
		/* empty bitmap could not be created */
		वापस शून्य;
	}

	pixels = guit->bitmap->get_बफर(bitmap);
	अगर (pixels == शून्य) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		वापस शून्य;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	चयन (webp_fmt.layout) {
	शेष:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	हाल BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(source_तथ्य, source_आकार, pixels,
				rowstride * webpfeatures.height, rowstride);
		अवरोध;

	हाल BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(source_तथ्य, source_आकार, pixels,
				rowstride * webpfeatures.height, rowstride);
		अवरोध;

	हाल BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(source_तथ्य, source_आकार, pixels,
				rowstride * webpfeatures.height, rowstride);
		अवरोध;
	}
	अगर (decoded == शून्य) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		वापस शून्य;
	}

	bitmap_format_to_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	वापस bitmap;
}

/**
 * Convert the webp source data content.
 *
 * This ensures there is valid webp source data in the content object
 *  and then adds it to the image cache ready to be converted on
 *  demand.
 *
 * \param c The webp content object
 * \return true on successful processing of teh webp content else false
 */
अटल bool webp_convert(काष्ठा content *c)
{
	पूर्णांक res;
	स्थिर uint8_t* तथ्य;
	माप_प्रकार तथ्य_आकार;
	पूर्णांक width;
	पूर्णांक height;

	तथ्य = content_get_source_तथ्य_2(c, &तथ्य_आकार);

	res = WebPGetInfo(तथ्य, तथ्य_आकार, &width, &height);
	अगर (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		वापस false;
	}

	c->width = width;
	c->height = height;
	c->आकार = c->width * c->height * 4;

	image_cache_add(c, शून्य, webp_cache_convert);

	content_set_ready(c);
	content_set_done(c);

	वापस true;
}

/**
 * Clone content.
 */
अटल nserror webp_clone(स्थिर काष्ठा content *old, काष्ठा content **new_c)
{
	काष्ठा content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = सुस्मृति(1, माप(काष्ठा content));
	अगर (webp_c == शून्य) {
		वापस NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	अगर (res != NSERROR_OK) {
		content_destroy(webp_c);
		वापस res;
	}

	/* re-convert if the content is ready */
	अगर ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		अगर (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	वापस NSERROR_OK;
}

अटल स्थिर content_handler webp_content_handler = {
	.create = webp_create,
	.तथ्य_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_REGISTER_TYPES(nswebp, webp_types, webp_content_handler);
