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
	بنية content بنياد;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_چوکٺو;   /**< current frame to display [0...(max-1)] */
} gif_content;

ساكن مضمن nserror gif_nsgif_error_منزل_ns(nsgif_error gif_res)
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
ساكن فراغ *gif_bitmap_create(صحيح ويڪر, صحيح اوچائي)
{
	أرجع guit->bitmap->create(ويڪر, اوچائي, BITMAP_NONE);
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

ساكن nserror gif_create_gif_معلومات(gif_content *c)
{
	nsgif_error gif_res;
	ثابت nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_بفر = guit->bitmap->get_بفر,
		.سيٽ_opaque = guit->bitmap->سيٽ_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	إذا (gif_res != NSGIF_OK) {
		nserror err = gif_nsgif_error_منزل_ns(gif_res);
		content_نشر_error(&c->بنياد, err, عدم);
		أرجع err;
	}

	أرجع NSERROR_OK;
}

ساكن nserror gif_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	gif_content *نتيجو;
	nserror error;

	نتيجو = صفر_يادگيري_ڏيو(1, حجم(gif_content));
	إذا (نتيجو == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&نتيجو->بنياد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(نتيجو);
		أرجع error;
	}

	error = gif_create_gif_معلومات(نتيجو);
	إذا (error != NSERROR_OK) {
		حرر(نتيجو);
		أرجع error;
	}

	*c = (بنية content *) نتيجو;

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

	gif_res = nsgif_چوکٺو_prepare(gif->gif, &rect, &delay, &f);
	إذا (gif_res != NSGIF_OK) {
		أرجع gif_nsgif_error_منزل_ns(gif_res);
	}

	gif->current_چوکٺو = f;

	/* Continue animating if we should */
	إذا (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->وقت_جدول(delay * 10, gif_animate_cb, gif);
	}

	إذا (redraw) {
		اتحاد content_نياپو_معلومات معلومات;

		/* area within gif to redraw */
		معلومات.redraw.x = rect.x0;
		معلومات.redraw.y = rect.y0;
		معلومات.redraw.ويڪر  = rect.x1 - rect.x0;
		معلومات.redraw.اوچائي = rect.y1 - rect.y0;

		content_نشر(&gif->بنياد, CONTENT_نياپو_REDRAW, &معلومات);
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
	ثابت uint8_t *معلومات;
	nsgif_error gif_err;
	nserror err;
	size_t سائيز;
	حرف *title;

	/* Get the animation */
	معلومات = content_get_ذريعو_معلومات_2(c, &سائيز);

	/* Initialise the GIF */
	gif_err = nsgif_معلومات_جاچ(gif->gif, سائيز, معلومات);
	إذا (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_معلومات_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	أكد(gif_info != عدم);

	/* Abort on bad GIFs */
	إذا (gif_info->چوکٺو_ڳڻپ == 0) {
		err = gif_nsgif_error_منزل_ns(gif_err);
		content_نشر_error(c, err, "GIF with no frames.");
		أرجع false;
	} وإلا إذا (gif_info->ويڪر == 0 || gif_info->اوچائي == 0) {
		err = gif_nsgif_error_منزل_ns(gif_err);
		content_نشر_error(c, err, "Zero size image.");
		أرجع false;
	}

	/* Store our content width, height and calculate size */
	c->ويڪر = gif_info->ويڪر;
	c->اوچائي = gif_info->اوچائي;
	c->سائيز += (gif_info->ويڪر * gif_info->اوچائي * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_پن(llcache_handle_get_url(c->llcache)),
			c->ويڪر, c->اوچائي);
	إذا (title != عدم) {
		content_سيٽ_title(c, title);
		حرر(title);
	}

	err = gif__animate(gif, false);
	إذا (err != NSERROR_OK) {
		content_نشر_error(c, NSERROR_GIF_ERROR, عدم);
		أرجع false;
	}

	/* Exit as a success */
	content_سيٽ_ready(c);
	content_سيٽ_پورو(c);

	/* Done: update status bar */
	content_سيٽ_حالت(c, "");
	أرجع true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ساكن nsgif_error gif_get_چوکٺو(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_چوکٺو = gif->current_چوکٺو;
	إذا (!nsoption_bool(animate_images)) {
		current_چوکٺو = 0;
	}

	أرجع nsgif_چوکٺو_decode(gif->gif, current_چوکٺو, bitmap);
}

ساكن bool gif_redraw(بنية content *c, بنية content_redraw_معلومات *معلومات,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	إذا (gif_get_چوکٺو(gif, &bitmap) != NSGIF_OK) {
		أرجع false;
	}

	أرجع image_bitmap_plot(bitmap, معلومات, clip, ctx);
}

ساكن فراغ gif_destroy(بنية content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->وقت_جدول(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ساكن nserror gif_clone(ثابت بنية content *old, بنية content **newc)
{
	gif_content *gif;
	nserror error;

	gif = صفر_يادگيري_ڏيو(1, حجم(gif_content));
	إذا (gif == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &gif->بنياد);
	إذا (error != NSERROR_OK) {
		content_destroy(&gif->بنياد);
		أرجع error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_معلومات(gif);
	إذا (error != NSERROR_OK) {
		content_destroy(&gif->بنياد);
		أرجع error;
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
			old->حالت == CONTENT_حالت_پورو) {
		إذا (gif_convert(&gif->بنياد) == false) {
			content_destroy(&gif->بنياد);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) gif;

	أرجع NSERROR_OK;
}

ساكن فراغ gif_جوڙيو_user(بنية content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	إذا (gif->gif == عدم) أرجع;

	إذا (content_ڳڻپ_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		إذا (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ساكن فراغ gif_remove_user(بنية content *c)
{
	إذا (content_ڳڻپ_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->وقت_جدول(-1, gif_animate_cb, c);
	}
}

ساكن nsgif_bitmap_t *gif_get_bitmap(
		ثابت بنية content *c, فراغ *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	إذا (gif_get_چوکٺو(gif, &bitmap) != NSGIF_OK) {
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

	إذا (gif_get_چوکٺو(gif, &bitmap) != NSGIF_OK) {
		أرجع false;
	}

	أرجع guit->bitmap->get_opaque(bitmap);
}

ساكن ثابت content_handler gif_content_handler = {
	.create = gif_create,
	.معلومات_complete = gif_convert,
	.destroy = gif_destroy,
	.redraw = gif_redraw,
	.clone = gif_clone,
	.جوڙيو_user = gif_جوڙيو_user,
	.remove_user = gif_remove_user,
	.get_اندروني = gif_get_bitmap,
	.type = gif_content_type,
	.is_opaque = gif_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_رجسٽر_TYPES(nsgif, gif_types, gif_content_handler);
