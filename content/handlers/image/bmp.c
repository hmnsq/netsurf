/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * implementation of content handler for BMP images.
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <libnsbmp.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/bmp.h"

/** bmp context. */
రకనిర్వచనం నిర్మాణం nsbmp_content {
	నిర్మాణం content పునాది;

	bmp_image *bmp;	/** BMP image data */

	నిర్మాణం bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsbmp_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param bmp_state A flag word indicating the initial state
 * \return An opaque struct bitmap, or NULL on memory exhaustion
 */
స్థిర శూన్య *nsbmp_bitmap_create(పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు, గుర్తులేని పూర్ణాంకం bmp_state)
{
	గుర్తులేని పూర్ణాంకం bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_జ్ఞాపకం) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	తిరిగి guit->bitmap->create(వెడల్పు, ఎత్తు, bitmap_state);
}

స్థిర nserror nsbmp_create_bmp_దత్తాంశం(nsbmp_content *bmp)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsbmp_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_బఫరు = guit->bitmap->get_బఫరు,
	};

	bmp->bmp = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం bmp_image));
	ఒకవేళ (bmp->bmp == ఖాళీ) {
		content_ప్రసారం_error(&bmp->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}

	bmp_create(bmp->bmp, &bmp_bitmap_callbacks);

	తిరిగి NSERROR_OK;
}

స్థిర nserror
nsbmp_create(స్థిరరాశి నిర్మాణం content_handler *handler,
	     lwc_string *imime_type,
	     స్థిరరాశి నిర్మాణం http_parameter *params,
	     llcache_handle *llcache,
	     స్థిరరాశి అక్షరం *fallback_charset,
	     bool quirks,
	     నిర్మాణం content **c)
{
	nsbmp_content *bmp;
	nserror error;

	bmp = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nsbmp_content));
	ఒకవేళ (bmp == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&bmp->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(bmp);
		తిరిగి error;
	}

	error = nsbmp_create_bmp_దత్తాంశం(bmp);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(bmp);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) bmp;

	తిరిగి NSERROR_OK;
}

స్థిర bool nsbmp_convert(నిర్మాణం content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bmp_ఫలితం res;
	uint32_t swidth;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
	అక్షరం *title;

	/* set the bmp data */
	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	/* analyse the BMP */
	res = bmp_analyse(bmp->bmp, కొలత, (గుర్తులేని అక్షరం *) దత్తాంశం);
	ఎంపిక (res) {
		స్థితి BMP_OK:
			ఆపు;
		స్థితి BMP_INSUFFICIENT_జ్ఞాపకం:
			content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
			తిరిగి false;
		స్థితి BMP_INSUFFICIENT_దత్తాంశం:
		స్థితి BMP_దత్తాంశం_ERROR:
			content_ప్రసారం_error(c, NSERROR_BMP_ERROR, ఖాళీ);
			తిరిగి false;
	}

	/* Store our content width and description */
	c->వెడల్పు = bmp->bmp->వెడల్పు;
	c->ఎత్తు = bmp->bmp->ఎత్తు;
	swidth = పరిమాణం(uint32_t) * bmp->bmp->వెడల్పు;
	c->కొలత += (swidth * bmp->bmp->ఎత్తు) + 16 + 44;

	/* set title text */
	title = messages_get_buff("BMPTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	/* exit as a success */
	bmp->bitmap = bmp->bmp->bitmap;

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);

	/* Done: update status bar */
	content_సమితి_స్థితి(c, "");
	తిరిగి true;
}

స్థిర bool nsbmp_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	nsbmp_content *bmp = (nsbmp_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ఒకవేళ (bmp->bmp->decoded == false) {
		bmp_ఫలితం res;
		res = bmp_decode(bmp->bmp);
		/* allow short or incomplete image data giving a partial image*/
		ఒకవేళ ((res != BMP_OK) &&
		    (res != BMP_INSUFFICIENT_దత్తాంశం) &&
		    (res != BMP_దత్తాంశం_ERROR)) {
			తిరిగి false;
		}

		bitmap_format_గమ్యం_client(bmp->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(bmp->bitmap);
	}

	ఒకవేళ (దత్తాంశం->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ఒకవేళ (దత్తాంశం->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	తిరిగి (ctx->plot->bitmap(ctx,
				  bmp->bitmap,
				  దత్తాంశం->x, దత్తాంశం->y,
				  దత్తాంశం->వెడల్పు, దత్తాంశం->ఎత్తు,
				  దత్తాంశం->background_రంగు,
				  flags) == NSERROR_OK);
}


స్థిర శూన్య nsbmp_destroy(నిర్మాణం content *c)
{
	nsbmp_content *bmp = (nsbmp_content *) c;

	bmp_finalise(bmp->bmp);
	విడుదల(bmp->bmp);
}


స్థిర nserror nsbmp_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	nsbmp_content *new_bmp;
	nserror error;

	new_bmp = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nsbmp_content));
	ఒకవేళ (new_bmp == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &new_bmp->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&new_bmp->పునాది);
		తిరిగి error;
	}

	/* We "clone" the old content by replaying creation and conversion */
	error = nsbmp_create_bmp_దత్తాంశం(new_bmp);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&new_bmp->పునాది);
		తిరిగి error;
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY || 
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (nsbmp_convert(&new_bmp->పునాది) == false) {
			content_destroy(&new_bmp->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) new_bmp;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య *nsbmp_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	తిరిగి bmp->bitmap;
}

స్థిర content_type nsbmp_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}

స్థిర bool nsbmp_content_is_opaque(నిర్మాణం content *c)
{
	nsbmp_content *bmp = (nsbmp_content *)c;

	ఒకవేళ (bmp->bitmap != ఖాళీ) {
		తిరిగి guit->bitmap->get_opaque(bmp->bitmap);
	}

	తిరిగి false;
}

స్థిర స్థిరరాశి content_handler nsbmp_content_handler = {
	.create = nsbmp_create,
	.దత్తాంశం_complete = nsbmp_convert,
	.destroy = nsbmp_destroy,
	.redraw = nsbmp_redraw,
	.clone = nsbmp_clone,
	.get_అంతర్గత = nsbmp_get_అంతర్గత,
	.type = nsbmp_content_type,
	.is_opaque = nsbmp_content_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *nsbmp_types[] = {
	"application/bmp",
	"application/preview",
	"application/x-bmp",
	"application/x-win-bitmap",
	"image/bmp",
	"image/ms-bmp",
	"image/x-bitmap",
	"image/x-bmp",
	"image/x-ms-bmp",
	"image/x-win-bitmap",
	"image/x-windows-bmp",
	"image/x-xbitmap"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsbmp, nsbmp_types, nsbmp_content_handler);
