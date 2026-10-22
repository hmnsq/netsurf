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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ <nsutils/assert.h>

#అన్తర్భావ <nsgif.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "netsurf/misc.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/image.h"
#అన్తర్భావ "image/gif.h"

రకనిర్వచనం నిర్మాణం gif_content {
	నిర్మాణం content పునాది;

	nsgif_t *gif; /**< GIF animation data */
	uint32_t current_చట్రం;   /**< current frame to display [0...(max-1)] */
} gif_content;

స్థిర అంతఃవరుస nserror gif_nsgif_error_గమ్యం_ns(nsgif_error gif_res)
{
	nserror err;

	ఎంపిక (gif_res) {
	స్థితి NSGIF_ERR_OOM:
		err = NSERROR_NOMEM;
		ఆపు;
	అప్రమేయ:
		err = NSERROR_GIF_ERROR;
		ఆపు;
	}

	తిరిగి err;
}

/**
 * Callback for libnsgif; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
స్థిర శూన్య *gif_bitmap_create(పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు)
{
	తిరిగి guit->bitmap->create(వెడల్పు, ఎత్తు, BITMAP_NONE);
}

/**
 * Convert client bitmap format to a LibNSGIF format specifier.
 */
స్థిర nsgif_bitmap_fmt_t nsgif__get_bitmap_format(శూన్య)
{
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_R8G8B8A8 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_R8G8B8A8);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_B8G8R8A8 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_B8G8R8A8);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_A8R8G8B8 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_A8R8G8B8);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_A8B8G8R8 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_A8B8G8R8);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_RGBA8888 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_RGBA8888);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_BGRA8888 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_BGRA8888);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_ARGB8888 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_ARGB8888);
	ns_static_assert((పూర్ణాంకం)BITMAP_LAYOUT_ABGR8888 == (పూర్ణాంకం)NSGIF_BITMAP_FMT_ABGR8888);

	తిరిగి (nsgif_bitmap_fmt_t)bitmap_fmt.layout;
}

స్థిర nserror gif_create_gif_దత్తాంశం(gif_content *c)
{
	nsgif_error gif_res;
	స్థిరరాశి nsgif_bitmap_cb_vt gif_bitmap_callbacks = {
		.create = gif_bitmap_create,
		.destroy = guit->bitmap->destroy,
		.get_బఫరు = guit->bitmap->get_బఫరు,
		.సమితి_opaque = guit->bitmap->సమితి_opaque,
		.test_opaque = bitmap_test_opaque,
		.modified = guit->bitmap->modified,
	};

	gif_res = nsgif_create(&gif_bitmap_callbacks,
			nsgif__get_bitmap_format(), &c->gif);
	ఒకవేళ (gif_res != NSGIF_OK) {
		nserror err = gif_nsgif_error_గమ్యం_ns(gif_res);
		content_ప్రసారం_error(&c->పునాది, err, ఖాళీ);
		తిరిగి err;
	}

	తిరిగి NSERROR_OK;
}

స్థిర nserror gif_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	gif_content *ఫలితం;
	nserror error;

	ఫలితం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(gif_content));
	ఒకవేళ (ఫలితం == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&ఫలితం->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	error = gif_create_gif_దత్తాంశం(ఫలితం);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) ఫలితం;

	తిరిగి NSERROR_OK;
}

/**
 * Scheduler callback. Performs any necessary animation.
 *
 * \param p  The content to animate
*/
స్థిర శూన్య gif_animate_cb(శూన్య *p);

/**
 * Performs any necessary animation.
 *
 * \param p  The content to animate
*/
స్థిర nserror gif__animate(gif_content *gif, bool redraw)
{
	nsgif_error gif_res;
	nsgif_rect_t rect;
	uint32_t delay;
	uint32_t f;

	gif_res = nsgif_చట్రం_prepare(gif->gif, &rect, &delay, &f);
	ఒకవేళ (gif_res != NSGIF_OK) {
		తిరిగి gif_nsgif_error_గమ్యం_ns(gif_res);
	}

	gif->current_చట్రం = f;

	/* Continue animating if we should */
	ఒకవేళ (nsoption_bool(animate_images) && delay != NSGIF_INFINITE) {
		guit->misc->కాలపట్టిక(delay * 10, gif_animate_cb, gif);
	}

	ఒకవేళ (redraw) {
		సంఘం content_సందేశం_దత్తాంశం దత్తాంశం;

		/* area within gif to redraw */
		దత్తాంశం.redraw.x = rect.x0;
		దత్తాంశం.redraw.y = rect.y0;
		దత్తాంశం.redraw.వెడల్పు  = rect.x1 - rect.x0;
		దత్తాంశం.redraw.ఎత్తు = rect.y1 - rect.y0;

		content_ప్రసారం(&gif->పునాది, CONTENT_సందేశం_REDRAW, &దత్తాంశం);
	}

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య gif_animate_cb(శూన్య *p)
{
	gif_content *gif = p;

	gif__animate(gif, true);
}

స్థిర bool gif_convert(నిర్మాణం content *c)
{
	gif_content *gif = (gif_content *) c;
	స్థిరరాశి nsgif_info_t *gif_info;
	స్థిరరాశి uint8_t *దత్తాంశం;
	nsgif_error gif_err;
	nserror err;
	size_t కొలత;
	అక్షరం *title;

	/* Get the animation */
	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	/* Initialise the GIF */
	gif_err = nsgif_దత్తాంశం_పరిశీలన(gif->gif, కొలత, దత్తాంశం);
	ఒకవేళ (gif_err != NSGIF_OK) {
		NSLOG(netsurf, INFO, "nsgif scan: %s", nsgif_strerror(gif_err));
		/* Not fatal unless we have no frames. */
	}

	nsgif_దత్తాంశం_complete(gif->gif);

	gif_info = nsgif_get_info(gif->gif);
	ప్రతిజ్ఞా(gif_info != ఖాళీ);

	/* Abort on bad GIFs */
	ఒకవేళ (gif_info->చట్రం_లెక్క == 0) {
		err = gif_nsgif_error_గమ్యం_ns(gif_err);
		content_ప్రసారం_error(c, err, "GIF with no frames.");
		తిరిగి false;
	} లేకపోతే ఒకవేళ (gif_info->వెడల్పు == 0 || gif_info->ఎత్తు == 0) {
		err = gif_nsgif_error_గమ్యం_ns(gif_err);
		content_ప్రసారం_error(c, err, "Zero size image.");
		తిరిగి false;
	}

	/* Store our content width, height and calculate size */
	c->వెడల్పు = gif_info->వెడల్పు;
	c->ఎత్తు = gif_info->ఎత్తు;
	c->కొలత += (gif_info->వెడల్పు * gif_info->ఎత్తు * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("GIFTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	err = gif__animate(gif, false);
	ఒకవేళ (err != NSERROR_OK) {
		content_ప్రసారం_error(c, NSERROR_GIF_ERROR, ఖాళీ);
		తిరిగి false;
	}

	/* Exit as a success */
	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);

	/* Done: update status bar */
	content_సమితి_స్థితి(c, "");
	తిరిగి true;
}

/**
 * Updates the GIF bitmap to display the current frame
 *
 * \param gif The gif context to update.
 * \return NSGIF_OK on success else apropriate error code.
 */
స్థిర nsgif_error gif_get_చట్రం(gif_content *gif,
		nsgif_bitmap_t **bitmap)
{
	uint32_t current_చట్రం = gif->current_చట్రం;
	ఒకవేళ (!nsoption_bool(animate_images)) {
		current_చట్రం = 0;
	}

	తిరిగి nsgif_చట్రం_decode(gif->gif, current_చట్రం, bitmap);
}

స్థిర bool gif_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ఒకవేళ (gif_get_చట్రం(gif, &bitmap) != NSGIF_OK) {
		తిరిగి false;
	}

	తిరిగి image_bitmap_plot(bitmap, దత్తాంశం, clip, ctx);
}

స్థిర శూన్య gif_destroy(నిర్మాణం content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Free all the associated memory buffers */
	guit->misc->కాలపట్టిక(-1, gif_animate_cb, c);
	nsgif_destroy(gif->gif);
}

స్థిర nserror gif_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	gif_content *gif;
	nserror error;

	gif = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(gif_content));
	ఒకవేళ (gif == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &gif->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&gif->పునాది);
		తిరిగి error;
	}

	/* Simply replay creation and conversion of content */
	error = gif_create_gif_దత్తాంశం(gif);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&gif->పునాది);
		తిరిగి error;
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (gif_convert(&gif->పునాది) == false) {
			content_destroy(&gif->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) gif;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య gif_కూడు_user(నిర్మాణం content *c)
{
	gif_content *gif = (gif_content *) c;

	/* Ensure this content has already been converted.
	 * If it hasn't, the animation will start at the conversion phase instead. */
	ఒకవేళ (gif->gif == ఖాళీ) తిరిగి;

	ఒకవేళ (content_లెక్క_users(c) == 1) {
		/* First user, and content already converted, so start the animation. */
		ఒకవేళ (nsgif_reset(gif->gif) == NSGIF_OK) {
			gif__animate(gif, true);
		}
	}
}

స్థిర శూన్య gif_remove_user(నిర్మాణం content *c)
{
	ఒకవేళ (content_లెక్క_users(c) == 1) {
		/* Last user is about to be removed from this content, so stop the animation. */
		guit->misc->కాలపట్టిక(-1, gif_animate_cb, c);
	}
}

స్థిర nsgif_bitmap_t *gif_get_bitmap(
		స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ఒకవేళ (gif_get_చట్రం(gif, &bitmap) != NSGIF_OK) {
		తిరిగి ఖాళీ;
	}

	తిరిగి bitmap;
}

స్థిర content_type gif_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}

స్థిర bool gif_content_is_opaque(నిర్మాణం content *c)
{
	gif_content *gif = (gif_content *) c;
	nsgif_bitmap_t *bitmap;

	ఒకవేళ (gif_get_చట్రం(gif, &bitmap) != NSGIF_OK) {
		తిరిగి false;
	}

	తిరిగి guit->bitmap->get_opaque(bitmap);
}

స్థిర స్థిరరాశి content_handler gif_content_handler = {
	.create = gif_create,
	.దత్తాంశం_complete = gif_convert,
	.destroy = gif_destroy,
	.redraw = gif_redraw,
	.clone = gif_clone,
	.కూడు_user = gif_కూడు_user,
	.remove_user = gif_remove_user,
	.get_అంతర్గత = gif_get_bitmap,
	.type = gif_content_type,
	.is_opaque = gif_content_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *gif_types[] = {
	"image/gif"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsgif, gif_types, gif_content_handler);
