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

#ضمن <assert.h>
#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>

#ضمن <nsutils/assert.h>

#ضمن <nsgif.h>

#ضمن "utils/log.h"
#ضمن "utils/utils.h"
#ضمن "utils/messages.h"
#ضمن "utils/nsoption.h"
#ضمن "netsurf/misc.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/image.h"
#ضمن "image/gif.h"

تعريفنوع بنية gif_content {
	بنية content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

ساكن مضمن nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	بدل (gif_res) {
	حالة NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		اكسر;
	افتراضي:
		err = NSERROR_GIF_ERROR;
		اكسر;
	}

	أرجع err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ساكن فراغ *gif_bitmap_create(صحيح width, صحيح height)
{
	أرجع guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
ساكن nsgif_bitmap_fmt_t nsgif__get_bitmap_format(فراغ)
{
	ns_static_assert((صحيح)BITMAP_LAYOUT_R8G8B8A8 == (صحيح)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((صحيح)BITMAP_LAYOUT_B8G8R8A8 == (صحيح)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((صحيح)BITMAP_LAYOUT_A8R8G8B8 == (صحيح)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((صحيح)BITMAP_LAYOUT_A8B8G8R8 == (صحيح)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((صحيح)BITMAP_LAYOUT_RGBA8888 == (صحيح)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((صحيح)BITMAP_LAYOUT_BGRA8888 == (صحيح)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((صحيح)BITMAP_LAYOUT_ARGB8888 == (صحيح)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((صحيح)BITMAP_LAYOUT_ABGR8888 == (صحيح)NSGIF_BITMAP_FMT_ABGR8888);

	أرجع (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

ساكن nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	ثابت nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	𐡄𐡍 (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, عدم);
		أرجع err;
	}

	أرجع NSERROR_OK;
}

ساكن nserror gif_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, حجم(gif_content));
	𐡄𐡍 (result == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	𐡄𐡍 (error != NSERROR_OK) {
		حرر(result);
		أرجع error;
	}

	error = gif_create_gif_data(result);
	𐡄𐡍 (error != NSERROR_OK) {
		حرر(result);
		أرجع error;
	}

	*c = (بنية content *) result;

	أرجع NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ساكن فراغ gif_animate_cb(فراغ *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ساكن nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	𐡄𐡍 (gif_res != NSGIF_OK) {
		أرجع gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	𐡄𐡍 (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	𐡄𐡍 (redraw) {
		اتحاد content_msg_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_MSG_REDRAW, &data);
	}

	أرجع NSERROR_OK;
}

ساكن فراغ gif_animate_cb(فراغ *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

ساكن bool gif_convert(بنية content *c)
{
	gif_content *gif = (gif_content *) c;
	ثابت nsgif_info_t *gif_info;
	ثابت uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	حرف *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	𐡄𐡍 (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	أكد(gif_info != عدم);

	/* Abort on bad GIFs */
	𐡄𐡍 (gif_info->frame_count == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		أرجع false;
	} وإلا 𐡄𐡍 (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		أرجع false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	𐡄𐡍 (title != عدم) {
		content__set_title(c, title);
		حرر(title);
	}

	err = gif__animate(gif, false);
	𐡄𐡍 (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, عدم);
		أرجع false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	أرجع true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ساكن nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	𐡄𐡍 (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	أرجع nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

ساكن bool gif_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	𐡄𐡍 (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		أرجع false;
	}

	أرجع image_bitmap_plot(bitmap, data, clip, ctx);
}

ساكن فراغ gif_destroy(بنية content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ساكن nserror gif_clone(ثابت بنية content *old, بنية content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, حجم(gif_content));
	𐡄𐡍 (gif == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	𐡄𐡍 (error != NSERROR_OK) {
		content_destroy(&gif->base);
		أرجع error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	𐡄𐡍 (error != NSERROR_OK) {
		content_destroy(&gif->base);
		أرجع error;
	}

	𐡄𐡍 (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		𐡄𐡍 (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) gif;

	أرجع NSERROR_OK;
}

ساكن فراغ gif_add_user(بنية content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	𐡄𐡍 (gif->gif == عدم) أرجع;

	𐡄𐡍 (content_count_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		𐡄𐡍 (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ساكن فراغ gif_remove_user(بنية content *c)
{
	𐡄𐡍 (content_count_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

ساكن nsgif_bitmap_t *gif_get_bitmap(
		ثابت بنية content *c, فراغ *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	𐡄𐡍 (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		أرجع عدم;
	}

	أرجع bitmap;
}

ساكن content_type gif_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}

ساكن bool gif_content_is_opaque(بنية content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	𐡄𐡍 (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		أرجع false;
	}

	أرجع guit->bitmap->get_opaque(bitmap);
}

ساكن ثابت content_handler gif_content_handler = {
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

ساكن ثابت حرف *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
