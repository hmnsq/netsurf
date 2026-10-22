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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ <nsutils/assert.h>

#ಅನ್ತರ್ಭಾವ <nsgif.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "netsurf/misc.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image.h"
#ಅನ್ತರ್ಭಾವ "image/gif.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ gif_content {
	ಸಂರಚನಾ content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	ವಿಕಲ್ಪ (gif_res) {
	ಪಕ್ಷ NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		err = NSERROR_GIF_ERROR;
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ಸ್ಥಿರ ಶೂನ್ಯ *gif_bitmap_create(ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height)
{
	ಪ್ರತ್ಯಾಗಮ guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
ಸ್ಥಿರ nsgif_bitmap_fmt_t nsgif__get_bitmap_format(ಶೂನ್ಯ)
{
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_R8G8B8A8 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_B8G8R8A8 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_A8R8G8B8 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_A8B8G8R8 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_RGBA8888 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_BGRA8888 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_ARGB8888 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((ಪೂರ್ಣಾಙ್ಕ)BITMAP_LAYOUT_ABGR8888 == (ಪೂರ್ಣಾಙ್ಕ)NSGIF_BITMAP_FMT_ABGR8888);

	ಪ್ರತ್ಯಾಗಮ (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

ಸ್ಥಿರ nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	ಧ್ರುವ nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	ಯದಿ (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ err;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ nserror gif_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, ಪರಿಮಾಣ(gif_content));
	ಯದಿ (result == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = gif_create_gif_data(result);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(result);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) result;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ಸ್ಥಿರ ಶೂನ್ಯ gif_animate_cb(ಶೂನ್ಯ *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ಸ್ಥಿರ nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	ಯದಿ (gif_res != NSGIF_OK) {
		ಪ್ರತ್ಯಾಗಮ gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	ಯದಿ (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	ಯದಿ (redraw) {
		ಸಙ್ಘ content_ಸಂದೇಶ_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_ಸಂದೇಶ_REDRAW, &data);
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ gif_animate_cb(ಶೂನ್ಯ *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

ಸ್ಥಿರ bool gif_convert(ಸಂರಚನಾ content *c)
{
	gif_content *gif = (gif_content *) c;
	ಧ್ರುವ nsgif_info_t *gif_info;
	ಧ್ರುವ uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	ವರ್ಣ *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	ಯದಿ (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	ಪ್ರತಿಜ್ಞಾ(gif_info != ರಿಕ್ತ);

	/* Abort on bad GIFs */
	ಯದಿ (gif_info->frame_ಎಣಿಕೆ == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ ಯದಿ (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ಯದಿ (title != ರಿಕ್ತ) {
		content__set_title(c, title);
		ಮುಕ್ತ(title);
	}

	err = gif__animate(gif, false);
	ಯದಿ (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ಪ್ರತ್ಯಾಗಮ true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ಸ್ಥಿರ nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	ಯದಿ (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	ಪ್ರತ್ಯಾಗಮ nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

ಸ್ಥಿರ bool gif_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ಯದಿ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ image_bitmap_plot(bitmap, data, clip, ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ gif_destroy(ಸಂರಚನಾ content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ಸ್ಥಿರ nserror gif_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, ಪರಿಮಾಣ(gif_content));
	ಯದಿ (gif == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) gif;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ gif_add_user(ಸಂರಚನಾ content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	ಯದಿ (gif->gif == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (content_ಎಣಿಕೆ_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		ಯದಿ (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ gif_remove_user(ಸಂರಚನಾ content *c)
{
	ಯದಿ (content_ಎಣಿಕೆ_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

ಸ್ಥಿರ nsgif_bitmap_t *gif_get_bitmap(
		ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ಯದಿ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ bitmap;
}

ಸ್ಥಿರ content_type gif_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}

ಸ್ಥಿರ bool gif_content_is_opaque(ಸಂರಚನಾ content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ಯದಿ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(bitmap);
}

ಸ್ಥಿರ ಧ್ರುವ content_handler gif_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
