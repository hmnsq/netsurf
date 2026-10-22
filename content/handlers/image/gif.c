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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ <nsutils/assert.h>

#અન્તર્ભાવ <nsgif.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "netsurf/misc.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/image.h"
#અન્તર્ભાવ "image/gif.h"

પ્રકારવ્યાખ્યા સંરચના gif_content {
	સંરચના content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

સ્થિર અંતઃરેખ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	પસંદગી (gif_res) {
	સ્થિતિ NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		તોડો;
	મૂળભૂત:
		err = NSERROR_GIF_ERROR;
		તોડો;
	}

	પાછા err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
સ્થિર શૂન્ય *gif_bitmap_create(પૂર્ણાંક width, પૂર્ણાંક height)
{
	પાછા guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
સ્થિર nsgif_bitmap_fmt_t nsgif__get_bitmap_format(શૂન્ય)
{
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_R8G8B8A8 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_B8G8R8A8 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_A8R8G8B8 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_A8B8G8R8 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_RGBA8888 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_BGRA8888 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_ARGB8888 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((પૂર્ણાંક)BITMAP_LAYOUT_ABGR8888 == (પૂર્ણાંક)NSGIF_BITMAP_FMT_ABGR8888);

	પાછા (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

સ્થિર nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	અચલ nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	જો (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, ખાલી);
		પાછા err;
	}

	પાછા NSERROR_OK;
}

સ્થિર nserror gif_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, કદ(gif_content));
	જો (result == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(result);
		પાછા error;
	}

	error = gif_create_gif_data(result);
	જો (error != NSERROR_OK) {
		મુક્ત(result);
		પાછા error;
	}

	*c = (સંરચના content *) result;

	પાછા NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
સ્થિર શૂન્ય gif_animate_cb(શૂન્ય *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
સ્થિર nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	જો (gif_res != NSGIF_OK) {
		પાછા gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	જો (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	જો (redraw) {
		સંઘ content_સંદેશ_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_સંદેશ_REDRAW, &data);
	}

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય gif_animate_cb(શૂન્ય *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

સ્થિર bool gif_convert(સંરચના content *c)
{
	gif_content *gif = (gif_content *) c;
	અચલ nsgif_info_t *gif_info;
	અચલ uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	અક્ષર *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	જો (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	પ્રતિજ્ઞા(gif_info != ખાલી);

	/* Abort on bad GIFs */
	જો (gif_info->frame_ગણતરી == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		પાછા false;
	} નહીંતર જો (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		પાછા false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	જો (title != ખાલી) {
		content__set_title(c, title);
		મુક્ત(title);
	}

	err = gif__animate(gif, false);
	જો (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, ખાલી);
		પાછા false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	પાછા true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
સ્થિર nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	જો (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	પાછા nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

સ્થિર bool gif_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	જો (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		પાછા false;
	}

	પાછા image_bitmap_plot(bitmap, data, clip, ctx);
}

સ્થિર શૂન્ય gif_destroy(સંરચના content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

સ્થિર nserror gif_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, કદ(gif_content));
	જો (gif == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	જો (error != NSERROR_OK) {
		content_destroy(&gif->base);
		પાછા error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	જો (error != NSERROR_OK) {
		content_destroy(&gif->base);
		પાછા error;
	}

	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) gif;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય gif_add_user(સંરચના content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	જો (gif->gif == ખાલી) પાછા;

	જો (content_ગણતરી_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		જો (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

સ્થિર શૂન્ય gif_remove_user(સંરચના content *c)
{
	જો (content_ગણતરી_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

સ્થિર nsgif_bitmap_t *gif_get_bitmap(
		અચલ સંરચના content *c, શૂન્ય *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	જો (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		પાછા ખાલી;
	}

	પાછા bitmap;
}

સ્થિર content_type gif_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}

સ્થિર bool gif_content_is_opaque(સંરચના content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	જો (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		પાછા false;
	}

	પાછા guit->bitmap->get_opaque(bitmap);
}

સ્થિર અચલ content_handler gif_content_handler = {
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

સ્થિર અચલ અક્ષર *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
