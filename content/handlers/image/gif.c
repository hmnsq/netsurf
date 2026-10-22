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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ <nsutils/assert.h>

#ଅନ୍ତର୍ଭାଵ <nsgif.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/misc.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image.h"
#ଅନ୍ତର୍ଭାଵ "image/gif.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା gif_content {
	ସଂରଚନା content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	ବାଛ (gif_res) {
	ସ୍ଥିତି NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		err = NSERROR_GIF_ERROR;
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ସ୍ଥିର ଶୂନ୍ୟ *gif_bitmap_create(ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	ଫେରନ୍ତୁ guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
ସ୍ଥିର nsgif_bitmap_fmt_t nsgif__get_bitmap_format(ଶୂନ୍ୟ)
{
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_R8G8B8A8 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_B8G8R8A8 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_A8R8G8B8 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_A8B8G8R8 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_RGBA8888 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_BGRA8888 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_ARGB8888 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)BITMAP_LAYOUT_ABGR8888 == (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)NSGIF_BITMAP_FMT_ABGR8888);

	ଫେରନ୍ତୁ (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

ସ୍ଥିର nserror gif_create_gif_data(gif_content *c)
{
	nsgif_error gif_res;
	ଅଚଳ nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_buffer = guit->bitmap->get_buffer,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	ଯଦି (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, ଖାଲି);
		ଫେରନ୍ତୁ err;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର nserror gif_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	gif_content *result;
	nserror error;

	result = calloc(1, ଆକାର(gif_content));
	ଯଦି (result == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&result->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	error = gif_create_gif_data(result);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(result);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) result;

	ଫେରନ୍ତୁ NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ସ୍ଥିର ଶୂନ୍ୟ gif_animate_cb(ଶୂନ୍ୟ *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ସ୍ଥିର nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	ଯଦି (gif_res != NSGIF_OK) {
		ଫେରନ୍ତୁ gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	ଯଦି (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	ଯଦି (redraw) {
		ସଂଘ content_ସନ୍ଦେଶ_data data;

		/* area within gif to redraw */
		data.redraw.x = rect.x0;
		data.redraw.y = rect.y0;
		data.redraw.width  = rect.x1 - rect.x0;
		data.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_ସନ୍ଦେଶ_REDRAW, &data);
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ gif_animate_cb(ଶୂନ୍ୟ *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

ସ୍ଥିର bool gif_convert(ସଂରଚନା content *c)
{
	gif_content *gif = (gif_content *) c;
	ଅଚଳ nsgif_info_t *gif_info;
	ଅଚଳ uint8_t *data;
	nsgif_error gif_err;
	nserror err;
	size_t size;
	ଅକ୍ଷର *title;

	/* Get the animation */
	data = content__get_source_data(c, &size);

	/* Initialise the GIF */
	gif_err = nsgif_data_scan(gif->gif, size, data);
	ଯଦି (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_data_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	ପ୍ରତିଜ୍ଞା(gif_info != ଖାଲି);

	/* Abort on bad GIFs */
	ଯଦି (gif_info->frame_ଗଣନା == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା ଯଦି (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		ଫେରନ୍ତୁ false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->size += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	ଯଦି (title != ଖାଲି) {
		content__set_title(c, title);
		ମୁକ୍ତ(title);
	}

	err = gif__animate(gif, false);
	ଯଦି (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ଫେରନ୍ତୁ true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ସ୍ଥିର nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	ଯଦି (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	ଫେରନ୍ତୁ nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

ସ୍ଥିର bool gif_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ଯଦି (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ image_bitmap_plot(bitmap, data, clip, ctx);
}

ସ୍ଥିର ଶୂନ୍ୟ gif_destroy(ସଂରଚନା content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ସ୍ଥିର nserror gif_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, ଆକାର(gif_content));
	ଯଦି (gif == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_data(gif);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ଫେରନ୍ତୁ error;
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) gif;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ gif_add_user(ସଂରଚନା content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	ଯଦି (gif->gif == ଖାଲି) ଫେରନ୍ତୁ;

	ଯଦି (content_ଗଣନା_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		ଯଦି (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ସ୍ଥିର ଶୂନ୍ୟ gif_remove_user(ସଂରଚନା content *c)
{
	ଯଦି (content_ଗଣନା_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

ସ୍ଥିର nsgif_bitmap_t *gif_get_bitmap(
		ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ଯଦି (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ bitmap;
}

ସ୍ଥିର content_type gif_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}

ସ୍ଥିର bool gif_content_is_opaque(ସଂରଚନା content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ଯଦି (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ guit->bitmap->get_opaque(bitmap);
}

ସ୍ଥିର ଅଚଳ content_handler gif_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
