/*
 * Copyright 2007 Rob Kendrick <rjek@netsurf-browser.org>
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
 * implementation of content handler for image/svg using librsvg.
 *
 * SVG files are rendered to a NetSurf bitmap by creating a Cairo rendering
 * surface (content_rsvg_data.cs) over the bitmap's data, creating a Cairo
 * drawing context using that surface, and then passing that drawing context
 * to librsvg which then uses Cairo calls to plot the graphic to the bitmap.
 * We store this in content->bitmap, and then use the usual bitmap plotter
 * function to render it for redraw requests.
 */

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ <librsvg/rsvg.h>
#నిర్వచితం_కాకపోతే RSVG_CAIRO_H
#అన్తర్భావ <librsvg/rsvg-cairo.h>
#అయితే_ముగింపు

#అన్తర్భావ <nsutils/endian.h>

#అన్తర్భావ "utils/log.h"
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

#అన్తర్భావ "image/rsvg.h"

రకనిర్వచనం నిర్మాణం rsvg_content {
	నిర్మాణం content పునాది;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	నిర్మాణం bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

స్థిర nserror rsvg_create_svg_దత్తాంశం(rsvg_content *c)
{
	c->rsvgh = ఖాళీ;
	c->cs = ఖాళీ;
	c->ct = ఖాళీ;
	c->bitmap = ఖాళీ;

	ఒకవేళ ((c->rsvgh = rsvg_handle_new()) == ఖాళీ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_ప్రసారం_error(&c->పునాది, NSERROR_NOMEM, ఖాళీ);
		తిరిగి NSERROR_NOMEM;
	}

	తిరిగి NSERROR_OK;
}


స్థిర nserror rsvg_create(స్థిరరాశి content_handler *handler,
		lwc_string *imime_type, స్థిరరాశి నిర్మాణం http_parameter *params,
		llcache_handle *llcache, స్థిరరాశి అక్షరం *fallback_charset,
		bool quirks, నిర్మాణం content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(rsvg_content));
	ఒకవేళ (svg == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__init(&svg->పునాది, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(svg);
		తిరిగి error;
	}

	error = rsvg_create_svg_దత్తాంశం(svg);
	ఒకవేళ (error != NSERROR_OK) {
		విడుదల(svg);
		తిరిగి error;
	}

	*c = (నిర్మాణం content *) svg;

	తిరిగి NSERROR_OK;
}


స్థిర bool rsvg_process_దత్తాంశం(నిర్మాణం content *c, స్థిరరాశి అక్షరం *దత్తాంశం,
			గుర్తులేని పూర్ణాంకం కొలత)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ఖాళీ;

	ఒకవేళ (rsvg_handle_write(d->rsvgh, (స్థిరరాశి guchar *)దత్తాంశం, (gsize)కొలత,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->సందేశం_2);
		content_ప్రసారం_error(c, NSERROR_SVG_ERROR, ఖాళీ);
		తిరిగి false;
	}

	తిరిగి true;
}

స్థిర bool rsvg_convert(నిర్మాణం content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_దత్తాంశం rsvgsize;
	GError *err = ఖాళీ;

	ఒకవేళ (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->సందేశం_2);
		content_ప్రసారం_error(c, NSERROR_SVG_ERROR, ఖాళీ);
		తిరిగి false;
	}

	ప్రతిజ్ఞా(err == ఖాళీ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->వెడల్పు = rsvgsize.వెడల్పు;
	c->ఎత్తు = rsvgsize.ఎత్తు;

	ఒకవేళ ((d->bitmap = guit->bitmap->create(c->వెడల్పు, c->ఎత్తు,
			BITMAP_NONE)) == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	}

	ఒకవేళ ((d->cs = cairo_image_surface_create_for_దత్తాంశం(
			(గుర్తులేని అక్షరం *)guit->bitmap->get_బఫరు(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->వెడల్పు, c->ఎత్తు,
			guit->bitmap->get_rowstride(d->bitmap))) == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	}

	ఒకవేళ ((d->ct = cairo_create(d->cs)) == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_ప్రసారం_error(c, NSERROR_NOMEM, ఖాళీ);
		తిరిగి false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_గమ్యం_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
	/* Done: update status bar */
	content_సమితి_స్థితి(c, "");

	తిరిగి true;
}

స్థిర bool rsvg_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ప్రతిజ్ఞా(rsvgcontent->bitmap != ఖాళీ);

	ఒకవేళ (దత్తాంశం->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ఒకవేళ (దత్తాంశం->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	తిరిగి (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  దత్తాంశం->x, దత్తాంశం->y,
				  దత్తాంశం->వెడల్పు, దత్తాంశం->ఎత్తు,
				  దత్తాంశం->background_రంగు,
				  flags) == NSERROR_OK);
}

స్థిర శూన్య rsvg_destroy(నిర్మాణం content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ఒకవేళ (d->bitmap != ఖాళీ) guit->bitmap->destroy(d->bitmap);
	ఒకవేళ (d->rsvgh != ఖాళీ) g_object_unref(d->rsvgh);
	ఒకవేళ (d->ct != ఖాళీ) cairo_destroy(d->ct);
	ఒకవేళ (d->cs != ఖాళీ) cairo_surface_destroy(d->cs);

	తిరిగి;
}

స్థిర nserror rsvg_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	rsvg_content *svg;
	nserror error;
	స్థిరరాశి uint8_t *దత్తాంశం;
	size_t కొలత;

	svg = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(rsvg_content));
	ఒకవేళ (svg == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &svg->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&svg->పునాది);
		తిరిగి error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_దత్తాంశం(svg);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&svg->పునాది);
		తిరిగి error;
	}

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(&svg->పునాది, &కొలత);
	ఒకవేళ (కొలత > 0) {
		ఒకవేళ (rsvg_process_దత్తాంశం(&svg->పునాది, (స్థిరరాశి అక్షరం *)దత్తాంశం, కొలత) == false) {
			content_destroy(&svg->పునాది);
			తిరిగి NSERROR_NOMEM;
		}
	}

	ఒకవేళ (old->స్థితి_2 == CONTENT_స్థితి_READY ||
			old->స్థితి_2 == CONTENT_స్థితి_పూర్తి) {
		ఒకవేళ (rsvg_convert(&svg->పునాది) == false) {
			content_destroy(&svg->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *) svg;

	తిరిగి NSERROR_OK;
}

స్థిర శూన్య *rsvg_get_అంతర్గత(స్థిరరాశి నిర్మాణం content *c, శూన్య *context)
{
	rsvg_content *d = (rsvg_content *) c;

	తిరిగి d->bitmap;
}

స్థిర content_type rsvg_content_type(శూన్య)
{
	తిరిగి CONTENT_IMAGE;
}


స్థిర bool rsvg_content_is_opaque(నిర్మాణం content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ఒకవేళ (d->bitmap != ఖాళీ) {
		తిరిగి guit->bitmap->get_opaque(d->bitmap);
	}

	తిరిగి false;
}


స్థిర స్థిరరాశి content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_దత్తాంశం = rsvg_process_దత్తాంశం,
	.దత్తాంశం_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_అంతర్గత = rsvg_get_అంతర్గత,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

