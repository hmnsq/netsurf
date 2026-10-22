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

#شامل <assert.h>
#شامل <string.h>
#شامل <stdbool.h>
#شامل <stdlib.h>

#شامل <nsutils/assert.h>

#شامل <nsgif.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/messages.h"
#شامل "utils/nsoption.h"
#شامل "netsurf/misc.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image.h"
#شامل "image/gif.h"

قسمتعریف ساخت gif_content {
	ساخت content بنیاد;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_چوکھٹا;   /**< current frame to display [0...(max-1)] */
} gif_content;

ساکن اندرونی nserror gif_nsgif_error_مقصد_ns(nsgif_error gif_res)
{
	nserror err;

	انتخاب (gif_res) {
	حالت NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		توڑو;
	طےشدہ:
		err = NSERROR_GIF_ERROR;
		توڑو;
	}

	واپس err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ساکن خالی *gif_bitmap_create(صحیح چوڑائی, صحیح اونچائی)
{
	واپس guit->bitmap->create(چوڑائی, اونچائی, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
ساکن nsgif_bitmap_fmt_t nsgif__get_bitmap_format(خالی)
{
	ns_static_assert((صحیح)BITMAP_LAYOUT_R8G8B8A8 == (صحیح)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((صحیح)BITMAP_LAYOUT_B8G8R8A8 == (صحیح)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((صحیح)BITMAP_LAYOUT_A8R8G8B8 == (صحیح)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((صحیح)BITMAP_LAYOUT_A8B8G8R8 == (صحیح)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((صحیح)BITMAP_LAYOUT_RGBA8888 == (صحیح)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((صحیح)BITMAP_LAYOUT_BGRA8888 == (صحیح)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((صحیح)BITMAP_LAYOUT_ARGB8888 == (صحیح)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((صحیح)BITMAP_LAYOUT_ABGR8888 == (صحیح)NSGIF_BITMAP_FMT_ABGR8888);

	واپس (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

ساکن nserror gif_create_gif_ڈیٹا(gif_content *c)
{
	nsgif_error gif_res;
	ثابت nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_بفر = guit->bitmap->get_بفر,
		.سیٹ_opaque = guit->bitmap->سیٹ_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	اگر (gif_res != NSGIF_OK) {
		nserror err = gif_nsgif_error_مقصد_ns(gif_res);
		content_نشر_error(&c->بنیاد, err, تہی);
		واپس err;
	}

	واپس NSERROR_OK;
}

ساکن nserror gif_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, ساخت content **c)
{
	gif_content *نتیجہ;
	nserror error;

	نتیجہ = صفر_حافظہ_دو(1, حجم(gif_content));
	اگر (نتیجہ == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&نتیجہ->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(نتیجہ);
		واپس error;
	}

	error = gif_create_gif_ڈیٹا(نتیجہ);
	اگر (error != NSERROR_OK) {
		آزاد(نتیجہ);
		واپس error;
	}

	*c = (ساخت content *) نتیجہ;

	واپس NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ساکن خالی gif_animate_cb(خالی *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ساکن nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_چوکھٹا_prepare(gif->gif, &rect, &delay, &f);
	اگر (gif_res != NSGIF_OK) {
		واپس gif_nsgif_error_مقصد_ns(gif_res);
	}

	gif->current_چوکھٹا = f;

	/* Continue animating if we should */
	اگر (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->جدول_وقت(delay * 10, gif_animate_cb, gif);
	}

	اگر (redraw) {
		اتحاد content_پیغام_ڈیٹا ڈیٹا;

		/* area within gif to redraw */
		ڈیٹا.redraw.x = rect.x0;
		ڈیٹا.redraw.y = rect.y0;
		ڈیٹا.redraw.چوڑائی  = rect.x1 - rect.x0;
		ڈیٹا.redraw.اونچائی = rect.y1 - rect.y0;

		content_نشر(&gif->بنیاد, CONTENT_پیغام_REDRAW, &ڈیٹا);
	}

	واپس NSERROR_OK;
}

ساکن خالی gif_animate_cb(خالی *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

ساکن bool gif_convert(ساخت content *c)
{
	gif_content *gif = (gif_content *) c;
	ثابت nsgif_info_t *gif_info;
	ثابت uint8_t *ڈیٹا;
	nsgif_error gif_err;
	nserror err;
	size_t سائز;
	حرف *title;

	/* Get the animation */
	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &سائز);

	/* Initialise the GIF */
	gif_err = nsgif_ڈیٹا_جائزہ(gif->gif, سائز, ڈیٹا);
	اگر (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_ڈیٹا_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	أكد(gif_info != تہی);

	/* Abort on bad GIFs */
	اگر (gif_info->چوکھٹا_گنتی == 0) {
		err = gif_nsgif_error_مقصد_ns(gif_err);
		content_نشر_error(c, err, "GIF with no frames.");
		واپس false;
	} ورنہ اگر (gif_info->چوڑائی == 0 || gif_info->اونچائی == 0) {
		err = gif_nsgif_error_مقصد_ns(gif_err);
		content_نشر_error(c, err, "Zero size image.");
		واپس false;
	}

	/* Store our content width, height and calculate size */
	c->چوڑائی = gif_info->چوڑائی;
	c->اونچائی = gif_info->اونچائی;
	c->سائز += (gif_info->چوڑائی * gif_info->اونچائی * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_برگ(llcache_handle_get_url(c->llcache)),
			c->چوڑائی, c->اونچائی);
	اگر (title != تہی) {
		content_سیٹ_title(c, title);
		آزاد(title);
	}

	err = gif__animate(gif, false);
	اگر (err != NSERROR_OK) {
		content_نشر_error(c, NSERROR_GIF_ERROR, تہی);
		واپس false;
	}

	/* Exit as a success */
	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);

	/* Done: update status bar */
	content_سیٹ_حالت(c, "");
	واپس true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ساکن nsgif_error gif_get_چوکھٹا(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_چوکھٹا = gif->current_چوکھٹا;
	اگر (!nsoption_bool(animate_images)) {
		current_چوکھٹا = 0;
	}

	واپس nsgif_چوکھٹا_decode(gif->gif, current_چوکھٹا, bitmap);
}

ساکن bool gif_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	اگر (gif_get_چوکھٹا(gif, &bitmap) != NSGIF_OK) {
		واپس false;
	}

	واپس image_bitmap_plot(bitmap, ڈیٹا, clip, ctx);
}

ساکن خالی gif_destroy(ساخت content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->جدول_وقت(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ساکن nserror gif_clone(ثابت ساخت content *old, ساخت content **newc)
{
	gif_content *gif;
	nserror error;

	gif = صفر_حافظہ_دو(1, حجم(gif_content));
	اگر (gif == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &gif->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&gif->بنیاد);
		واپس error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_ڈیٹا(gif);
	اگر (error != NSERROR_OK) {
		content_destroy(&gif->بنیاد);
		واپس error;
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY ||
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (gif_convert(&gif->بنیاد) == false) {
			content_destroy(&gif->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) gif;

	واپس NSERROR_OK;
}

ساکن خالی gif_جوڑو_user(ساخت content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	اگر (gif->gif == تہی) واپس;

	اگر (content_گنتی_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		اگر (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ساکن خالی gif_remove_user(ساخت content *c)
{
	اگر (content_گنتی_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->جدول_وقت(-1, gif_animate_cb, c);
	}
}

ساکن nsgif_bitmap_t *gif_get_bitmap(
		ثابت ساخت content *c, خالی *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	اگر (gif_get_چوکھٹا(gif, &bitmap) != NSGIF_OK) {
		واپس تہی;
	}

	واپس bitmap;
}

ساکن content_type gif_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}

ساکن bool gif_content_is_opaque(ساخت content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	اگر (gif_get_چوکھٹا(gif, &bitmap) != NSGIF_OK) {
		واپس false;
	}

	واپس guit->bitmap->get_opaque(bitmap);
}

ساکن ثابت content_handler gif_content_handler = {
	.create = gif_create,
	.ڈیٹا_complete = gif_convert,
	.destroy = gif_destroy,
	.redraw = gif_redraw,
	.clone = gif_clone,
	.جوڑو_user = gif_جوڑو_user,
	.remove_user = gif_remove_user,
	.get_اندرونی = gif_get_bitmap,
	.type = gif_content_type,
	.is_opaque = gif_content_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsgif, gif_types, gif_content_handler);
