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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ <nsutils/assert.h>

#அந்தர்பாவ <nsgif.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "netsurf/misc.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image.h"
#அந்தர்பாவ "image/gif.h"

வகைவரை கட்டமைப்பு gif_content {
	கட்டமைப்பு content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

நிலையான உள்வரி nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	தேர்வு (gif_res) {
	நிலை NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		நிறுத்து;
	இயல்பு:
		err = NSERROR_GIF_ERROR;
		நிறுத்து;
	}

	திரும்பு err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
நிலையான வெறுமை *gif_bitmap_create(முழு width, முழு height)
{
	திரும்பு guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
நிலையான nsgif_bitmap_fmt_t nsgif__get_bitmap_format(வெறுமை)
{
	ns_static_assert((முழு)BITMAP_LAYOUT_R8G8B8A8 == (முழு)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((முழு)BITMAP_LAYOUT_B8G8R8A8 == (முழு)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((முழு)BITMAP_LAYOUT_A8R8G8B8 == (முழு)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((முழு)BITMAP_LAYOUT_A8B8G8R8 == (முழு)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((முழு)BITMAP_LAYOUT_RGBA8888 == (முழு)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((முழு)BITMAP_LAYOUT_BGRA8888 == (முழு)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((முழு)BITMAP_LAYOUT_ARGB8888 == (முழு)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((முழு)BITMAP_LAYOUT_ABGR8888 == (முழு)NSGIF_BITMAP_FMT_ABGR8888);

	திரும்பு (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

நிலையான nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	மாறிலி nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	எனில் (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, வெற்று);
		திரும்பு err;
	}

	திரும்பு NSERROR_OK;
}

நிலையான nserror gif_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, அளவு(gif_content));
	எனில் (result == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(result);
		திரும்பு error;
	}

	error = gif_create_gif_data(result);
	எனில் (error != NSERROR_OK) {
		விடுவி(result);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) result;

	திரும்பு NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
நிலையான வெறுமை gif_animate_cb(வெறுமை *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
நிலையான nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	எனில் (gif_res != NSGIF_OK) {
		திரும்பு gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	எனில் (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	எனில் (redraw) {
		ஒன்றிப்பு content_செய்தி_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_செய்தி_REDRAW, &data);
	}

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை gif_animate_cb(வெறுமை *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

நிலையான bool gif_convert(கட்டமைப்பு content *c)
{
	gif_content *gif = (gif_content *) c;
	மாறிலி nsgif_info_t *gif_info;
	மாறிலி uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	எழுத்து *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	எனில் (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	ப்ரதிஜ்ஞா(gif_info != வெற்று);

	/* Abort on bad GIFs */
	எனில் (gif_info->frame_மொத்தம் == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		திரும்பு false;
	} இல்லையெனில் எனில் (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		திரும்பு false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	எனில் (title != வெற்று) {
		content__set_title(c, title);
		விடுவி(title);
	}

	err = gif__animate(gif, false);
	எனில் (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, வெற்று);
		திரும்பு false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	திரும்பு true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
நிலையான nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	எனில் (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	திரும்பு nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

நிலையான bool gif_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	எனில் (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		திரும்பு false;
	}

	திரும்பு image_bitmap_plot(bitmap, data, clip, ctx);
}

நிலையான வெறுமை gif_destroy(கட்டமைப்பு content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

நிலையான nserror gif_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, அளவு(gif_content));
	எனில் (gif == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&gif->base);
		திரும்பு error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	எனில் (error != NSERROR_OK) {
		content_destroy(&gif->base);
		திரும்பு error;
	}

	எனில் (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		எனில் (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) gif;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை gif_add_user(கட்டமைப்பு content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	எனில் (gif->gif == வெற்று) திரும்பு;

	எனில் (content_மொத்தம்_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		எனில் (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

நிலையான வெறுமை gif_remove_user(கட்டமைப்பு content *c)
{
	எனில் (content_மொத்தம்_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

நிலையான nsgif_bitmap_t *gif_get_bitmap(
		மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	எனில் (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		திரும்பு வெற்று;
	}

	திரும்பு bitmap;
}

நிலையான content_type gif_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}

நிலையான bool gif_content_is_opaque(கட்டமைப்பு content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	எனில் (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		திரும்பு false;
	}

	திரும்பு guit->bitmap->get_opaque(bitmap);
}

நிலையான மாறிலி content_handler gif_content_handler = {
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

நிலையான மாறிலி எழுத்து *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
