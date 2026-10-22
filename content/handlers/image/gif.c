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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव <nsutils/assert.h>

#अन्तर्भाव <nsgif.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "netsurf/misc.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image.h"
#अन्तर्भाव "image/gif.h"

प्रकारनिर्देश संरचना gif_content {
	संरचना content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

स्थिर अन्तःस्थ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	विकल्प (gif_res) {
	पक्ष NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		रोक;
	स्वभाव:
		err = NSERROR_GIF_ERROR;
		रोक;
	}

	फर्काऊ err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
स्थिर शून्य *gif_bitmap_create(पूर्णाङ्क width, पूर्णाङ्क height)
{
	फर्काऊ guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
स्थिर nsgif_bitmap_fmt_t nsgif__get_bitmap_format(शून्य)
{
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_R8G8B8A8 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_B8G8R8A8 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_A8R8G8B8 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_A8B8G8R8 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_RGBA8888 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_BGRA8888 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_ARGB8888 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((पूर्णाङ्क)BITMAP_LAYOUT_ABGR8888 == (पूर्णाङ्क)NSGIF_BITMAP_FMT_ABGR8888);

	फर्काऊ (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

स्थिर nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	अचर nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	यदि (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, रिक्त);
		फर्काऊ err;
	}

	फर्काऊ NSERROR_OK;
}

स्थिर nserror gif_create(अचर content_handler *handler,
		lwc_string *imime_type, अचर संरचना http_parameter *params,
		llcache_handle *llcache, अचर वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	gif_content *result;
	nserror error;

	result = सुस्मृति(1, परिमाण(gif_content));
	यदि (result == रिक्त)
		फर्काऊ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(result);
		फर्काऊ error;
	}

	error = gif_create_gif_data(result);
	यदि (error != NSERROR_OK) {
		मुक्त(result);
		फर्काऊ error;
	}

	*c = (संरचना content *) result;

	फर्काऊ NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
स्थिर शून्य gif_animate_cb(शून्य *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
स्थिर nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	यदि (gif_res != NSGIF_OK) {
		फर्काऊ gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	यदि (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	यदि (redraw) {
		सङ्घ content_सन्देश_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_सन्देश_REDRAW, &data);
	}

	फर्काऊ NSERROR_OK;
}

स्थिर शून्य gif_animate_cb(शून्य *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

स्थिर bool gif_convert(संरचना content *c)
{
	gif_content *gif = (gif_content *) c;
	अचर nsgif_info_t *gif_info;
	अचर uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	माप_प्रकार size;
	वर्ण *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	यदि (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	प्रतिज्ञा(gif_info != रिक्त);

	/* Abort on bad GIFs */
	यदि (gif_info->frame_गन्ती == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		फर्काऊ false;
	} अन्यथा यदि (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		फर्काऊ false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	यदि (title != रिक्त) {
		content__set_title(c, title);
		मुक्त(title);
	}

	err = gif__animate(gif, false);
	यदि (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, रिक्त);
		फर्काऊ false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	फर्काऊ true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
स्थिर nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	यदि (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	फर्काऊ nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

स्थिर bool gif_redraw(संरचना content *c, संरचना content_redraw_data *data,
		अचर संरचना rect *clip, अचर संरचना redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	यदि (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		फर्काऊ false;
	}

	फर्काऊ image_bitmap_plot(bitmap, data, clip, ctx);
}

स्थिर शून्य gif_destroy(संरचना content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

स्थिर nserror gif_clone(अचर संरचना content *old, संरचना content **newc)
{
	gif_content *gif;
	nserror error;

	gif = सुस्मृति(1, परिमाण(gif_content));
	यदि (gif == रिक्त)
		फर्काऊ NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&gif->base);
		फर्काऊ error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	यदि (error != NSERROR_OK) {
		content_destroy(&gif->base);
		फर्काऊ error;
	}

	यदि (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		यदि (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			फर्काऊ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) gif;

	फर्काऊ NSERROR_OK;
}

स्थिर शून्य gif_add_user(संरचना content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	यदि (gif->gif == रिक्त) फर्काऊ;

	यदि (content_गन्ती_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		यदि (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

स्थिर शून्य gif_remove_user(संरचना content *c)
{
	यदि (content_गन्ती_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

स्थिर nsgif_bitmap_t *gif_get_bitmap(
		अचर संरचना content *c, शून्य *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	यदि (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		फर्काऊ रिक्त;
	}

	फर्काऊ bitmap;
}

स्थिर content_type gif_content_type(शून्य)
{
	फर्काऊ CONTENT_IMAGE;
}

स्थिर bool gif_content_is_opaque(संरचना content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	यदि (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		फर्काऊ false;
	}

	फर्काऊ guit->bitmap->get_opaque(bitmap);
}

स्थिर अचर content_handler gif_content_handler = {
	.create = gif_create,
	.data_complete = gif_convert,
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

स्थिर अचर वर्ण *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
