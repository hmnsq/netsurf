/*
 * Copyright 2019 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handling for image/webp
 *
 * This implementation uses the google webp library.
 * Image cache handling is performed by the generic NetSurf handler.
 */

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <setjmp.h>

#అన్తర్భావ <webp/decode.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "content/llcache.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/content_factory.h"
#అన్తర్భావ "desktop/gui_internal.h"
#అన్తర్భావ "desktop/bitmap.h"

#అన్తర్భావ "image/image_cache.h"

#అన్తర్భావ "webp.h"

/**
 * Content create entry point.
 *
 * create a content object for the webp
 */
స్థిర nserror
webp_create(స్థిరరాశి content_handler *handler,
	      lwc_string *imime_type,
	      స్థిరరాశి నిర్మాణం http_parameter *params,
	      llcache_handle *llcache,
	      స్థిరరాశి అక్షరం *fallback_charset,
	      bool quirks,
	      నిర్మాణం content **c)
{
	నిర్మాణం content *webp_c; /* webp content object */
	nserror res;

	webp_c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content));
	ఒకవేళ (webp_c == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	res = content__init(webp_c,
			    handler,
			    imime_type,
			    params,
			    llcache,
			    fallback_charset,
			    quirks);
	ఒకవేళ (res != NSERROR_OK) {
		విడుదల(webp_c);
		తిరిగి res;
	}

	*c = webp_c;

	తిరిగి NSERROR_OK;
}

/**
 * create a bitmap from webp content.
 */
స్థిర నిర్మాణం bitmap *
webp_cache_convert(నిర్మాణం content *c)
{
	స్థిరరాశి uint8_t *ఆధారం_దత్తాంశం; /* webp source data */
	size_t ఆధారం_కొలత; /* length of webp source data */
	VP8_స్థితి_సంకేతం webpres;
	WebPBitstreamFeatures webpfeatures;
	గుర్తులేని పూర్ణాంకం bmap_flags;
	uint8_t *pixels = ఖాళీ;
	uint8_t *decoded;
	size_t rowstride;
	నిర్మాణం bitmap *bitmap = ఖాళీ;
	bitmap_fmt_t webp_fmt = {
		.layout = bitmap_fmt.layout,
	};

	ఆధారం_దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &ఆధారం_కొలత);

	webpres = WebPGetFeatures(ఆధారం_దత్తాంశం, ఆధారం_కొలత, &webpfeatures);

	ఒకవేళ (webpres != VP8_స్థితి_OK) {
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (webpfeatures.has_alpha == 0) {
		bmap_flags = BITMAP_OPAQUE;
		/* Image has no alpha. Premultiplied alpha makes no difference.
		 * Optimisation: Avoid unnecessary conversion by copying format.
		 */
		webp_fmt.pma = bitmap_fmt.pma;
	} లేకపోతే {
		bmap_flags = BITMAP_NONE;
	}

	/* create bitmap */
	bitmap = guit->bitmap->create(webpfeatures.వెడల్పు,
				      webpfeatures.ఎత్తు,
				      bmap_flags);
	ఒకవేళ (bitmap == ఖాళీ) {
		/* empty bitmap could not be created */
		తిరిగి ఖాళీ;
	}

	pixels = guit->bitmap->get_బఫరు(bitmap);
	ఒకవేళ (pixels == ఖాళీ) {
		/* bitmap with no buffer available */
		guit->bitmap->destroy(bitmap);
		తిరిగి ఖాళీ;
	}

	rowstride = guit->bitmap->get_rowstride(bitmap);

	ఎంపిక (webp_fmt.layout) {
	అప్రమేయ:
		/* WebP has no ABGR function, fall back to default. */
		webp_fmt.layout = BITMAP_LAYOUT_R8G8B8A8;
		fallthrough;
	స్థితి BITMAP_LAYOUT_R8G8B8A8:
		decoded = WebPDecodeRGBAInto(ఆధారం_దత్తాంశం, ఆధారం_కొలత, pixels,
				rowstride * webpfeatures.ఎత్తు, rowstride);
		ఆపు;

	స్థితి BITMAP_LAYOUT_B8G8R8A8:
		decoded = WebPDecodeBGRAInto(ఆధారం_దత్తాంశం, ఆధారం_కొలత, pixels,
				rowstride * webpfeatures.ఎత్తు, rowstride);
		ఆపు;

	స్థితి BITMAP_LAYOUT_A8R8G8B8:
		decoded = WebPDecodeARGBInto(ఆధారం_దత్తాంశం, ఆధారం_కొలత, pixels,
				rowstride * webpfeatures.ఎత్తు, rowstride);
		ఆపు;
	}
	ఒకవేళ (decoded == ఖాళీ) {
		/* decode failed */
		guit->bitmap->destroy(bitmap);
		తిరిగి ఖాళీ;
	}

	bitmap_format_గమ్యం_client(bitmap, &webp_fmt);
	guit->bitmap->modified(bitmap);

	తిరిగి bitmap;
}

/**
 * Convert the webp source data content.
 *
 * This ensures there is valid webp source data in the content object
 *  and then adds it to the image cache ready to be converted on
 *  demand.
 *
 * \param c The webp content object
 * \return true on successful processing of teh webp content else false
 */
స్థిర bool webp_convert(నిర్మాణం content *c)
{
	పూర్ణాంకం res;
	స్థిరరాశి uint8_t* దత్తాంశం;
	size_t దత్తాంశం_కొలత;
	పూర్ణాంకం వెడల్పు;
	పూర్ణాంకం ఎత్తు;

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &దత్తాంశం_కొలత);

	res = WebPGetInfo(దత్తాంశం, దత్తాంశం_కొలత, &వెడల్పు, &ఎత్తు);
	ఒకవేళ (res == 0) {
		NSLOG(netsurf, INFO, "WebPGetInfo failed:%p", c);
		తిరిగి false;
	}

	c->వెడల్పు = వెడల్పు;
	c->ఎత్తు = ఎత్తు;
	c->కొలత = c->వెడల్పు * c->ఎత్తు * 4;

	image_cache_కూడు(c, ఖాళీ, webp_cache_convert);

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);

	తిరిగి true;
}

/**
 * Clone content.
 */
స్థిర nserror webp_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **new_c)
{
	నిర్మాణం content *webp_c; /* cloned webp content */
	nserror res;

	webp_c = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం content));
	ఒకవేళ (webp_c == ఖాళీ) {
		తిరిగి NSERROR_NOMEM;
	}

	res = content__clone(old, webp_c);
	ఒకవేళ (res != NSERROR_OK) {
		content_destroy(webp_c);
		తిరిగి res;
	}

	/* re-convert if the content is ready */
	ఒకవేళ ((old->స్థితి_2 == CONTENT_స్థితి_READY) ||
	    (old->స్థితి_2 == CONTENT_స్థితి_పూర్తి)) {
		ఒకవేళ (webp_convert(webp_c) == false) {
			content_destroy(webp_c);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*new_c = webp_c;

	తిరిగి NSERROR_OK;
}

స్థిర స్థిరరాశి content_handler webp_content_handler = {
	.create = webp_create,
	.దత్తాంశం_complete = webp_convert,
	.destroy = image_cache_destroy,
	.redraw = image_cache_redraw,
	.clone = webp_clone,
	.get_అంతర్గత = image_cache_get_అంతర్గత,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *webp_types[] = {
	"image/webp"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nswebp, webp_types, webp_content_handler);
