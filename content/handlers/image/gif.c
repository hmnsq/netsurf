/*
 * Copyright 2003 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004 Richard Wilson <not_ginger_matt@users.sourceforge.net>
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
 *
 * Content for image/gif implementation
 *
 * All GIFs are dynamically decompressed using the routines that gifread.c
 * provides. Whilst this allows support for progressive decoding, it is
 * not implemented here as NetSurf currently does not provide such support.
 *
 * [rjw] - Sun 4th April 2004
 */

#समावेश <assert.h>
#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>

#समावेश <nsutils/assert.h>

#समावेश <nsgif.h>

#समावेश "utils/log.h"
#समावेश "utils/utils.h"
#समावेश "utils/messages.h"
#समावेश "utils/nsoption.h"
#समावेश "netsurf/misc.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/image.h"
#समावेश "image/gif.h"

प्रकार काष्ठा gif_content {
	काष्ठा content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

अटल अंतरभूत nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	चयन (gif_res) {
	हाल NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		अवरोध;
	शेष:
		err = NSERROR_GIF_ERROR;
		अवरोध;
	}

	वापस err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
अटल व्योम *gif_bitmap_create(पूर्णांक width, पूर्णांक height)
{
	वापस guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
अटल nsgif_bitmap_fmt_t nsgif__get_bitmap_format(व्योम)
{
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_R8G8B8A8 == (पूर्णांक)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_B8G8R8A8 == (पूर्णांक)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_A8R8G8B8 == (पूर्णांक)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_A8B8G8R8 == (पूर्णांक)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_RGBA8888 == (पूर्णांक)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_BGRA8888 == (पूर्णांक)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_ARGB8888 == (पूर्णांक)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((पूर्णांक)BITMAP_LAYOUT_ABGR8888 == (पूर्णांक)NSGIF_BITMAP_FMT_ABGR8888);

	वापस (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

अटल nserror gif_create_gif_तथ्य(gif_content *c)
{
	nsgif_error gif_res;
	स्थिर nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_बफर = guit->bitmap->get_बफर,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	अगर (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, शून्य);
		वापस err;
	}

	वापस NSERROR_OK;
}

अटल nserror gif_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	gif_content *परिणाम;
	nserror error;

	परिणाम = सुस्मृति(1, माप(gif_content));
	अगर (परिणाम == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&परिणाम->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(परिणाम);
		वापस error;
	}

	error = gif_create_gif_तथ्य(परिणाम);
	अगर (error != NSERROR_OK) {
		मुक्त(परिणाम);
		वापस error;
	}

	*c = (काष्ठा content *) परिणाम;

	वापस NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
अटल व्योम gif_animate_cb(व्योम *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
अटल nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	अगर (gif_res != NSGIF_OK) {
		वापस gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	अगर (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	अगर (redraw) {
		जोड़ content_संदेश_तथ्य तथ्य;

		/* area within gif to redraw */
		तथ्य.redraw.x = rect.x0;
		तथ्य.redraw.y = rect.y0;
		तथ्य.redraw.width  = rect.x1 - rect.x0;
		तथ्य.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_संदेश_REDRAW, &तथ्य);
	}

	वापस NSERROR_OK;
}

अटल व्योम gif_animate_cb(व्योम *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

अटल bool gif_convert(काष्ठा content *c)
{
	gif_content *gif = (gif_content *) c;
	स्थिर nsgif_info_t *gif_info;
	स्थिर uint8_t *तथ्य;
	nsgif_error gif_err;
	nserror err;
	माप_प्रकार आकार;
	अक्षर *title;

	/* Get the animation */
	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	/* Initialise the GIF */
	gif_err = nsgif_तथ्य_scan(gif->gif, आकार, तथ्य);
	अगर (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_तथ्य_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	निश्चित(gif_info != शून्य);

	/* Abort on bad GIFs */
	अगर (gif_info->frame_गिनती == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		वापस false;
	} अन्यथा अगर (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		वापस false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->आकार += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	अगर (title != शून्य) {
		content__set_title(c, title);
		मुक्त(title);
	}

	err = gif__animate(gif, false);
	अगर (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, शून्य);
		वापस false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	वापस true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
अटल nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	अगर (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	वापस nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

अटल bool gif_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	अगर (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		वापस false;
	}

	वापस image_bitmap_plot(bitmap, तथ्य, clip, ctx);
}

अटल व्योम gif_destroy(काष्ठा content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

अटल nserror gif_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	gif_content *gif;
	nserror error;

	gif = सुस्मृति(1, माप(gif_content));
	अगर (gif == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&gif->base);
		वापस error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_तथ्य(gif);
	अगर (error != NSERROR_OK) {
		content_destroy(&gif->base);
		वापस error;
	}

	अगर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		अगर (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) gif;

	वापस NSERROR_OK;
}

अटल व्योम gif_add_user(काष्ठा content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	अगर (gif->gif == शून्य) वापस;

	अगर (content_गिनती_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		अगर (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

अटल व्योम gif_remove_user(काष्ठा content *c)
{
	अगर (content_गिनती_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

अटल nsgif_bitmap_t *gif_get_bitmap(
		स्थिर काष्ठा content *c, व्योम *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	अगर (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		वापस शून्य;
	}

	वापस bitmap;
}

अटल content_type gif_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}

अटल bool gif_content_is_opaque(काष्ठा content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	अगर (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		वापस false;
	}

	वापस guit->bitmap->get_opaque(bitmap);
}

अटल स्थिर content_handler gif_content_handler = {
	.create = gif_create,
	.तथ्य_complete = gif_convert,
	.destroy = gif_destroy,
	.redraw = gif_redraw,
	.clone = gif_clone,
	.add_user = gif_add_user,
	.remove_user = gif_remove_user,
	.get_internal = gif_get_bitmap,
	.type = gif_content_type,
	.is_opaque = gif_content_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
