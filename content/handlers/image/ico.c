/*
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * implementation for image/ico content handler
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <libnsbmp.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/image.h"
#అన్తర్భావ "image/ico.h"

రకనిర్వచనం నిర్మాణం nsico_content {
	నిర్మాణం content పునాది;

	నిర్మాణం ico_collection *ico; /** ICO collection data */
} nsico_content;

/**
 * Callback for libnsbmp; forwards the call to bitmap_create()
 *
 * \param  width   width of image in pixels
 * \param  height  width of image in pixels
 * \param  bmp_state A flag word indicating the initial state
 * \return an opaque struct bitmap, or NULL on memory exhaustion
 */
స్థిర శూన్య *nsico_bitmap_create(పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు, గుర్తులేని పూర్ణాంకం bmp_state)
{
	గుర్తులేని పూర్ణాంకం bitmap_state = BITMAP_NONE;

	/* set bitmap state based on bmp state */
	bitmap_state |= (bmp_state & BMP_OPAQUE) ? BITMAP_OPAQUE : 0;
	bitmap_state |= (bmp_state & BMP_CLEAR_జ్ఞాపకం) ?
			BITMAP_CLEAR : 0;

	/* return the created bitmap */
	తిరిగి guit->bitmap->create(వెడల్పు, ఎత్తు, bitmap_state);
}

స్థిర nserror nsico_create_ico_దత్తాంశం(nsico_content *c)
{
	bmp_bitmap_callback_vt bmp_bitmap_callbacks = {
		.bitmap_create = nsico_bitmap_create,
		.bitmap_destroy = guit->bitmap->destroy,
		.bitmap_get_బఫరు = guit->bitmap->get_బఫరు,
	};

	c->ico = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(ico_collection));
	ఒకవేళ (c->ico == ఖాళీ) {
		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}
	ico_collection_create(c->ico, &bmp_bitmap_callbacks);
	తిరిగి NSERROR_OK;
}


స్థిర nserror nsico_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	nsico_content *ఫలితం;
	nserror error;

	ఫలితం = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nsico_content));
	ఒకవేళ (ఫలితం == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&ఫలితం->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	error = nsico_create_ico_దత్తాంశం(ఫలితం);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(ఫలితం);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) ఫలితం;

	తిరిగి NSERROR_OK;
}



స్థిర bool nsico_convert(నిర్మాణం content *c)
{
	nsico_content *ico = (nsico_content *) c;
	నిర్మాణం bmp_image *bmp;
	bmp_ఫలితం res;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;
	అక్షరం *title;

	/* set the ico data */
	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	/* analyse the ico */
	res = ico_analyse(ico->ico, కొలత, (గుర్తులేని అక్షరం *) దత్తాంశం);

	ఎంపిక (res) {
	స్థితి BMP_OK:
		ఆపు;
	స్థితి BMP_INSUFFICIENT_జ్ఞాపకం:
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	స్థితి BMP_INSUFFICIENT_దత్తాంశం:
	స్థితి BMP_దత్తాంశం_ERROR:
		content_ప్రసారం_error(c, NSERROR_ICO_ERROR, ఖాళీ);
		తిరిగి false;
	}

	/* Store our content width, height and calculate size */
	c->వెడల్పు = ico->ico->వెడల్పు;
	c->ఎత్తు = ico->ico->ఎత్తు;
	c->కొలత += (ico->ico->వెడల్పు * ico->ico->ఎత్తు * 4) + 16 + 44;

	/* set title text */
	title = messages_get_buff("ICOTitle",
			nsurl_access_ఆకు(llcache_handle_get_url(c->llcache)),
			c->వెడల్పు, c->ఎత్తు);
	ఒకవేళ (title != ఖాళీ) {
		content_సమితి_title(c, title);
		విడుదల(title);
	}

	/* select largest icon to ensure one can be selected */
	bmp = ico_కనుగొను(ico->ico, 255, 255);
	ఒకవేళ (bmp == ఖాళీ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		తిరిగి false;
	}

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);

	/* Done: update status bar */
	content_సమితి_స్థితి(c, "");
	తిరిగి true;
}

స్థిర bool nsico__decode(నిర్మాణం bmp_image *ico)
{
	ఒకవేళ (ico->decoded == false) {
		NSLOG(netsurf, DEBUG, "Decoding ICO %p", ico);
		ఒకవేళ (bmp_decode(ico) != BMP_OK) {
			తిరిగి false;
		}

		bitmap_format_గమ్యం_client(ico->bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_R8G8B8A8,
		});
		guit->bitmap->modified(ico->bitmap);

	}

	తిరిగి true;
}

స్థిర bool nsico_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	nsico_content *ico = (nsico_content *)c;
	నిర్మాణం bmp_image *bmp;

	/* select most appropriate sized icon for size */
	bmp = ico_కనుగొను(ico->ico, దత్తాంశం->వెడల్పు, దత్తాంశం->ఎత్తు);
	ఒకవేళ (bmp == ఖాళీ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		తిరిగి false;
	}

	/* ensure its decided */
	ఒకవేళ (!nsico__decode(bmp)) {
		తిరిగి false;
	}

	తిరిగి image_bitmap_plot(bmp->bitmap, దత్తాంశం, clip, ctx);
}


స్థిర శూన్య nsico_destroy(నిర్మాణం content *c)
{
	nsico_content *ico = (nsico_content *) c;

	ico_finalise(ico->ico);
	విడుదల(ico->ico);
}

స్థిర nserror nsico_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	nsico_content *ico;
	nserror error;

	ico = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(nsico_content));
	ఒకవేళ (ico == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &ico->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&ico->పునాది);
		తిరిగి error;
	}

	/* Simply replay creation and conversion */
	error = nsico_create_ico_దత్తాంశం(ico);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&ico->పునాది);
		తిరిగి error;
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (nsico_convert(&ico->పునాది) == false) {
			content_destroy(&ico->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) ico;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య *nsico_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	nsico_content *ico = (nsico_content *) c;
	/* TODO: Pick best size for purpose.
	 *       Currently assumes it's for a URL bar. */
	నిర్మాణం bmp_image *bmp;

	bmp = ico_కనుగొను(ico->ico, 16, 16);
	ఒకవేళ (bmp == ఖాళీ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (!nsico__decode(bmp)) {
		తిరిగి ఖాళీ;
	}

	తిరిగి bmp->bitmap;
}

స్థిర content_type nsico_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}

స్థిర bool nsico_is_opaque(నిర్మాణం content *c)
{
	nsico_content *ico = (nsico_content *) c;
	నిర్మాణం bmp_image *bmp;

	/**
	 * \todo Pick best size for purpose. Currently assumes
	 *         it's for a URL bar.
	 */
	bmp = ico_కనుగొను(ico->ico, 16, 16);
	ఒకవేళ (bmp == ఖాళీ) {
		/* return error */
		NSLOG(netsurf, INFO, "Failed to select icon");
		తిరిగి false;
	}

	ఒకవేళ (!nsico__decode(bmp)) {
		తిరిగి false;
	}

	తిరిగి guit->bitmap->get_opaque(bmp->bitmap);
}

స్థిర స్థిరరాశి content_handler nsico_content_handler = {
	.create = nsico_create,
	.దత్తాంశం_complete = nsico_convert,
	.destroy = nsico_destroy,
	.redraw = nsico_redraw,
	.clone = nsico_clone,
	.get_అంతర్గత = nsico_get_అంతర్గత,
	.type = nsico_content_type,
	.is_opaque = nsico_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *nsico_types[] = {
	"application/ico",
	"application/x-ico",
	"image/ico",
	"image/vnd.microsoft.icon",
	"image/x-icon"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsico, nsico_types, nsico_content_handler);
