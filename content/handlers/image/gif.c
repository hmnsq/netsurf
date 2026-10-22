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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব <nsutils/assert.h>

#অন্তর্ভাব <nsgif.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "netsurf/misc.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/image.h"
#অন্তর্ভাব "image/gif.h"

প্রকারলেখ কাঠামো gif_content {
	কাঠামো content base;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_frame;   /**< current frame to display [0...(max-1)] */
} gif_content;

স্থির অন্তঃরেখ nserror gif__nsgif_error_to_ns(nsgif_error gif_res)
{
	nserror err;

	নির্বাচন (gif_res) {
	ক্ষেত্র NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		বিরতি;
	পূর্বনির্ধারিত:
		err = NSERROR_GIF_ERROR;
		বিরতি;
	}

	ফেরত err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
স্থির শূন্য *gif_bitmap_create(পূর্ণসংখ্যা width, পূর্ণসংখ্যা height)
{
	ফেরত guit->bitmap->create(width, height, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
স্থির nsgif_bitmap_fmt_t nsgif__get_bitmap_format(শূন্য)
{
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_R8G8B8A8 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_B8G8R8A8 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_A8R8G8B8 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_A8B8G8R8 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_RGBA8888 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_BGRA8888 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_ARGB8888 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((পূর্ণসংখ্যা)BITMAP_LAYOUT_ABGR8888 == (পূর্ণসংখ্যা)NSGIF_BITMAP_FMT_ABGR8888);

	ফেরত (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

স্থির nserror gif_create_gif_তথ্য(gif_content *c)
{
	nsgif_error gif_res;
	ধ্রুবক nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_বাফার = guit->bitmap->get_বাফার,
		.set_opaque = guit->bitmap->set_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	যদি (gif_res != NSGIF_OK) {
		nserror err = gif__nsgif_error_to_ns(gif_res);
		content_broadcast_error(&c->base, err, ফাঁকা);
		ফেরত err;
	}

	ফেরত NSERROR_OK;
}

স্থির nserror gif_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks, কাঠামো content **c)
{
	gif_content *ফলাফল;
	nserror error;

	ফলাফল = calloc(1, আকার(gif_content));
	যদি (ফলাফল == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&ফলাফল->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(ফলাফল);
		ফেরত error;
	}

	error = gif_create_gif_তথ্য(ফলাফল);
	যদি (error != NSERROR_OK) {
		মুক্ত(ফলাফল);
		ফেরত error;
	}

	*c = (কাঠামো content *) ফলাফল;

	ফেরত NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
স্থির শূন্য gif_animate_cb(শূন্য *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
স্থির nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_frame_prepare(gif->gif, &rect, &delay, &f);
	যদি (gif_res != NSGIF_OK) {
		ফেরত gif__nsgif_error_to_ns(gif_res);
	}

	gif->current_frame = f;

	/* Continue animating if we should */
	যদি (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->schedule(delay * 10, gif_animate_cb, gif);
	}

	যদি (redraw) {
		সংঘ content_বার্তা_তথ্য তথ্য;

		/* area within gif to redraw */
		তথ্য.redraw.x = rect.x0;
		তথ্য.redraw.y = rect.y0;
		তথ্য.redraw.width  = rect.x1 - rect.x0;
		তথ্য.redraw.height = rect.y1 - rect.y0;

		content_broadcast(&gif->base, CONTENT_বার্তা_REDRAW, &তথ্য);
	}

	ফেরত NSERROR_OK;
}

স্থির শূন্য gif_animate_cb(শূন্য *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

স্থির bool gif_convert(কাঠামো content *c)
{
	gif_content *gif = (gif_content *) c;
	ধ্রুবক nsgif_info_t *gif_info;
	ধ্রুবক uint8_t *তথ্য;
	nsgif_error gif_err;
	nserror err;
	size_t আকার_2;
	অক্ষর *title;

	/* Get the animation */
	তথ্য = content_get_source_তথ্য_2(c, &আকার_2);

	/* Initialise the GIF */
	gif_err = nsgif_তথ্য_scan(gif->gif, আকার_2, তথ্য);
	যদি (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_তথ্য_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	প্রতিজ্ঞা(gif_info != ফাঁকা);

	/* Abort on bad GIFs */
	যদি (gif_info->frame_গুনতি == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "GIF with no frames.");
		ফেরত false;
	} অন্যথা যদি (gif_info->width == 0 || gif_info->height == 0) {
		err = gif__nsgif_error_to_ns(gif_err);
		content_broadcast_error(c, err, "Zero size image.");
		ফেরত false;
	}

	/* Store our content width, height and calculate size */
	c->width = gif_info->width;
	c->height = gif_info->height;
	c->আকার_2 += (gif_info->width * gif_info->height * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	যদি (title != ফাঁকা) {
		content__set_title(c, title);
		মুক্ত(title);
	}

	err = gif__animate(gif, false);
	যদি (err != NSERROR_OK) {
		content_broadcast_error(c, NSERROR_GIF_ERROR, ফাঁকা);
		ফেরত false;
	}

	/* Exit as a success */
	content_set_ready(c);
	content_set_done(c);

	/* Done: update status bar */
	content_set_status(c, "");
	ফেরত true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
স্থির nsgif_error gif_get_frame(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_frame = gif->current_frame;
	যদি (!nsoption_bool(animate_images)) {
		current_frame = 0;
	}

	ফেরত nsgif_frame_decode(gif->gif, current_frame, bitmap);
}

স্থির bool gif_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	যদি (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ফেরত false;
	}

	ফেরত image_bitmap_plot(bitmap, তথ্য, clip, ctx);
}

স্থির শূন্য gif_destroy(কাঠামো content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->schedule(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

স্থির nserror gif_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	gif_content *gif;
	nserror error;

	gif = calloc(1, আকার(gif_content));
	যদি (gif == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &gif->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ফেরত error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_তথ্য(gif);
	যদি (error != NSERROR_OK) {
		content_destroy(&gif->base);
		ফেরত error;
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (gif_convert(&gif->base) == false) {
			content_destroy(&gif->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*newc = (কাঠামো content *) gif;

	ফেরত NSERROR_OK;
}

স্থির শূন্য gif_add_user(কাঠামো content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	যদি (gif->gif == ফাঁকা) ফেরত;

	যদি (content_গুনতি_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		যদি (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

স্থির শূন্য gif_remove_user(কাঠামো content *c)
{
	যদি (content_গুনতি_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->schedule(-1, gif_animate_cb, c);
	}
}

স্থির nsgif_bitmap_t *gif_get_bitmap(
		ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	যদি (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ফেরত ফাঁকা;
	}

	ফেরত bitmap;
}

স্থির content_type gif_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}

স্থির bool gif_content_is_opaque(কাঠামো content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	যদি (gif_get_frame(gif, &bitmap) != NSGIF_OK) {
		ফেরত false;
	}

	ফেরত guit->bitmap->get_opaque(bitmap);
}

স্থির ধ্রুবক content_handler gif_content_handler = {
	.create = gif_create,
	.তথ্য_complete = gif_convert,
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

স্থির ধ্রুবক অক্ষর *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_REGISTER_TYPES(nsgif, gif_types, gif_content_handler);
