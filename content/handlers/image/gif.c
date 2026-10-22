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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ <nsutils/assert.h>

#അന്തര്ഭാവ <nsgif.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "netsurf/misc.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image.h"
#അന്തര്ഭാവ "image/gif.h"

തരനിർവചനം ഘടന gif_content {
	ഘടന content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

സ്ഥിതം വരിയിൽ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	തിരഞ്ഞെടുപ്പ് (gif_res) {
	അവസ്ഥ NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		err = NSERROR_GIF_ERROR;
		നിർത്തുക;
	}

	തിരികെ err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
സ്ഥിതം ശൂന്യം *gif_bitmap_create(പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height)
{
	തിരികെ guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
സ്ഥിതം nsgif_bitmap_fmt_t nsgif__get_bitmap_format(ശൂന്യം)
{
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_R8G8B8A8 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_B8G8R8A8 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_A8R8G8B8 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_A8B8G8R8 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_RGBA8888 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_BGRA8888 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_ARGB8888 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((പൂർണ്ണസംഖ്യ)BITMAP_LAYOUT_ABGR8888 == (പൂർണ്ണസംഖ്യ)NSGIF_BITMAP_FMT_ABGR8888);

	തിരികെ (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

സ്ഥിതം nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	സ്ഥിരം nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	എങ്കിൽ (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, ശൂന്യ);
		തിരികെ err;
	}

	തിരികെ NSERROR_OK;
}

സ്ഥിതം nserror gif_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, വലുപ്പം(gif_content));
	എങ്കിൽ (result == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	error = gif_create_gif_data(result);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(result);
		തിരികെ error;
	}

	*c = (ഘടന content *) result;

	തിരികെ NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
സ്ഥിതം ശൂന്യം gif_animate_cb(ശൂന്യം *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
സ്ഥിതം nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	എങ്കിൽ (gif_res != NSGIF_OK) {
		തിരികെ gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	എങ്കിൽ (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	എങ്കിൽ (redraw) {
		സംഘം content_സന്ദേശം_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_സന്ദേശം_REDRAW, &data);
	}

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം gif_animate_cb(ശൂന്യം *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

സ്ഥിതം bool gif_convert(ഘടന content *c)
{
	gif_content *gif = (gif_content *) c;
	സ്ഥിരം nsgif_info_t *gif_info;
	സ്ഥിരം uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	അക്ഷരം *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	എങ്കിൽ (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	പ്രതിജ്ഞാ(gif_info != ശൂന്യ);

	/* Abort on bad GIFs */
	എങ്കിൽ (gif_info->frame_എണ്ണം == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		തിരികെ false;
	} അല്ലെങ്കിൽ എങ്കിൽ (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		തിരികെ false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	എങ്കിൽ (title != ശൂന്യ) {
		content__set_title(c, title);
		സ്വതന്ത്രം(title);
	}

	err = gif__animate(gif, false);
	എങ്കിൽ (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, ശൂന്യ);
		തിരികെ false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	തിരികെ true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
സ്ഥിതം nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	എങ്കിൽ (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	തിരികെ nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

സ്ഥിതം bool gif_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	എങ്കിൽ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		തിരികെ false;
	}

	തിരികെ image_bitmap_plot(bitmap, data, clip, ctx);
}

സ്ഥിതം ശൂന്യം gif_destroy(ഘടന content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

സ്ഥിതം nserror gif_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, വലുപ്പം(gif_content));
	എങ്കിൽ (gif == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&gif->base);
		തിരികെ error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&gif->base);
		തിരികെ error;
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) gif;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം gif_add_user(ഘടന content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	എങ്കിൽ (gif->gif == ശൂന്യ) തിരികെ;

	എങ്കിൽ (content_എണ്ണം_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		എങ്കിൽ (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

സ്ഥിതം ശൂന്യം gif_remove_user(ഘടന content *c)
{
	എങ്കിൽ (content_എണ്ണം_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

സ്ഥിതം nsgif_bitmap_t *gif_get_bitmap(
		സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	എങ്കിൽ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ bitmap;
}

സ്ഥിതം content_type gif_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}

സ്ഥിതം bool gif_content_is_opaque(ഘടന content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	എങ്കിൽ (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		തിരികെ false;
	}

	തിരികെ guit->bitmap->get_opaque(bitmap);
}

സ്ഥിതം സ്ഥിരം content_handler gif_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
