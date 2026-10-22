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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/assert.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <nsgif.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "image/gif.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ gif_content {
	ਬਣਤਰ content ਅਧਾਰ;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_ਚੌਖਟਾ;   /**< current frame to display [0...(max-1)] */
} gif_content;

ਸਥਿਰ ਸਤਰਅੰਦਰ nserror gif_nsgif_error_ਮੰਜ਼ਿਲ_ns(nsgif_error gif_res)
{
	nserror err;

	ਚੋਣ (gif_res) {
	ਹਾਲਤ NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		ਰੋਕੋ;
	ਮੂਲ:
		err = NSERROR_GIF_ERROR;
		ਰੋਕੋ;
	}

	ਵਾਪਸ err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
ਸਥਿਰ ਖਾਲੀ *gif_bitmap_create(ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ)
{
	ਵਾਪਸ guit->bitmap->create(ਚੌੜਾਈ, ਉਚਾਈ, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
ਸਥਿਰ nsgif_bitmap_fmt_t nsgif__get_bitmap_format(ਖਾਲੀ)
{
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_R8G8B8A8 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_B8G8R8A8 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_A8R8G8B8 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_A8B8G8R8 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_RGBA8888 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_BGRA8888 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_ARGB8888 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((ਪੂਰਨ)BITMAP_LAYOUT_ABGR8888 == (ਪੂਰਨ)NSGIF_BITMAP_FMT_ABGR8888);

	ਵਾਪਸ (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

ਸਥਿਰ nserror gif_create_gif_ਅੰਕੜੇ(gif_content *c)
{
	nsgif_error gif_res;
	ਅਚਲ nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_ਬਫਰ = guit->bitmap->get_ਬਫਰ,
		.ਸੈੱਟ_opaque = guit->bitmap->ਸੈੱਟ_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	ਜੇ (gif_res != NSGIF_OK) {
		nserror err = gif_nsgif_error_ਮੰਜ਼ਿਲ_ns(gif_res);
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, err, ਨਿਰਾ);
		ਵਾਪਸ err;
	}

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ nserror gif_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	gif_content *ਨਤੀਜਾ;
	nserror error;

	ਨਤੀਜਾ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(gif_content));
	ਜੇ (ਨਤੀਜਾ == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&ਨਤੀਜਾ->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	error = gif_create_gif_ਅੰਕੜੇ(ਨਤੀਜਾ);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) ਨਤੀਜਾ;

	ਵਾਪਸ NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ਸਥਿਰ ਖਾਲੀ gif_animate_cb(ਖਾਲੀ *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
ਸਥਿਰ nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_ਚੌਖਟਾ_prepare(gif->gif, &rect, &delay, &f);
	ਜੇ (gif_res != NSGIF_OK) {
		ਵਾਪਸ gif_nsgif_error_ਮੰਜ਼ਿਲ_ns(gif_res);
	}

	gif->current_ਚੌਖਟਾ = f;

	/* Continue animating if we should */
	ਜੇ (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(delay * 10, gif_animate_cb, gif);
	}

	ਜੇ (redraw) {
		ਸੰਘ content_ਸੁਨੇਹਾ_ਅੰਕੜੇ ਅੰਕੜੇ;

		/* area within gif to redraw */
		ਅੰਕੜੇ.redraw.x = rect.x0;
		ਅੰਕੜੇ.redraw.y = rect.y0;
		ਅੰਕੜੇ.redraw.ਚੌੜਾਈ  = rect.x1 - rect.x0;
		ਅੰਕੜੇ.redraw.ਉਚਾਈ = rect.y1 - rect.y0;

		content_ਪ੍ਰਸਾਰਣ(&gif->ਅਧਾਰ, CONTENT_ਸੁਨੇਹਾ_REDRAW, &ਅੰਕੜੇ);
	}

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ gif_animate_cb(ਖਾਲੀ *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

ਸਥਿਰ bool gif_convert(ਬਣਤਰ content *c)
{
	gif_content *gif = (gif_content *) c;
	ਅਚਲ nsgif_info_t *gif_info;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	nsgif_error gif_err;
	nserror err;
	size_t ਮਾਪ;
	ਅੱਖਰ *title;

	/* Get the animation */
	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	/* Initialise the GIF */
	gif_err = nsgif_ਅੰਕੜੇ_ਜਾਂਚ(gif->gif, ਮਾਪ, ਅੰਕੜੇ);
	ਜੇ (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_ਅੰਕੜੇ_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	ਪ੍ਰਤਿਜ੍ਞਾ(gif_info != ਨਿਰਾ);

	/* Abort on bad GIFs */
	ਜੇ (gif_info->ਚੌਖਟਾ_ਗਿਣਤੀ == 0) {
		err = gif_nsgif_error_ਮੰਜ਼ਿਲ_ns(gif_err);
		content_ਪ੍ਰਸਾਰਣ_error(c, err, "GIF with no frames.");
		ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ ਜੇ (gif_info->ਚੌੜਾਈ == 0 || gif_info->ਉਚਾਈ == 0) {
		err = gif_nsgif_error_ਮੰਜ਼ਿਲ_ns(gif_err);
		content_ਪ੍ਰਸਾਰਣ_error(c, err, "Zero size image.");
		ਵਾਪਸ false;
	}

	/* Store our content width, height and calculate size */
	c->ਚੌੜਾਈ = gif_info->ਚੌੜਾਈ;
	c->ਉਚਾਈ = gif_info->ਉਚਾਈ;
	c->ਮਾਪ += (gif_info->ਚੌੜਾਈ * gif_info->ਉਚਾਈ * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_ਪੱਤੀ(llcache_handle_get_url(c->llcache)),
			c->ਚੌੜਾਈ, c->ਉਚਾਈ);
	ਜੇ (title != ਨਿਰਾ) {
		content_ਸੈੱਟ_title(c, title);
		ਮੁਕਤ(title);
	}

	err = gif__animate(gif, false);
	ਜੇ (err != NSERROR_OK) {
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_GIF_ERROR, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	/* Exit as a success */
	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);

	/* Done: update status bar */
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, "");
	ਵਾਪਸ true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
ਸਥਿਰ nsgif_error gif_get_ਚੌਖਟਾ(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_ਚੌਖਟਾ = gif->current_ਚੌਖਟਾ;
	ਜੇ (!nsoption_bool(animate_images)) {
		current_ਚੌਖਟਾ = 0;
	}

	ਵਾਪਸ nsgif_ਚੌਖਟਾ_decode(gif->gif, current_ਚੌਖਟਾ, bitmap);
}

ਸਥਿਰ bool gif_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ਜੇ (gif_get_ਚੌਖਟਾ(gif, &bitmap) != NSGIF_OK) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ image_bitmap_plot(bitmap, ਅੰਕੜੇ, clip, ctx);
}

ਸਥਿਰ ਖਾਲੀ gif_destroy(ਬਣਤਰ content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

ਸਥਿਰ nserror gif_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	gif_content *gif;
	nserror error;

	gif = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(gif_content));
	ਜੇ (gif == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &gif->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&gif->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_ਅੰਕੜੇ(gif);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&gif->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (gif_convert(&gif->ਅਧਾਰ) == false) {
			content_destroy(&gif->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) gif;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ gif_ਜੋੜੋ_user(ਬਣਤਰ content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	ਜੇ (gif->gif == ਨਿਰਾ) ਵਾਪਸ;

	ਜੇ (content_ਗਿਣਤੀ_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		ਜੇ (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

ਸਥਿਰ ਖਾਲੀ gif_remove_user(ਬਣਤਰ content *c)
{
	ਜੇ (content_ਗਿਣਤੀ_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->ਸਮਾਂ_ਸਾਰਣੀ(-1, gif_animate_cb, c);
	}
}

ਸਥਿਰ nsgif_bitmap_t *gif_get_bitmap(
		ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ਜੇ (gif_get_ਚੌਖਟਾ(gif, &bitmap) != NSGIF_OK) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ bitmap;
}

ਸਥਿਰ content_type gif_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}

ਸਥਿਰ bool gif_content_is_opaque(ਬਣਤਰ content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ਜੇ (gif_get_ਚੌਖਟਾ(gif, &bitmap) != NSGIF_OK) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ guit->bitmap->get_opaque(bitmap);
}

ਸਥਿਰ ਅਚਲ content_handler gif_content_handler = {
	.create = gif_create,
	.ਅੰਕੜੇ_complete = gif_convert,
	.destroy = gif_destroy,
	.redraw = gif_redraw,
	.clone = gif_clone,
	.ਜੋੜੋ_user = gif_ਜੋੜੋ_user,
	.remove_user = gif_remove_user,
	.get_ਅੰਦਰੂਨੀ = gif_get_bitmap,
	.type = gif_content_type,
	.is_opaque = gif_content_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsgif, gif_types, gif_content_handler);
