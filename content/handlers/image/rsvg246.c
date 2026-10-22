/*
 * Copyright 2022 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handler for image/svg using librsvg 2.46 API.
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

#అన్తర్భావ "image/image_cache.h"

#అన్తర్భావ "image/rsvg.h"


రకనిర్వచనం నిర్మాణం rsvg_content {
	నిర్మాణం content పునాది;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


స్థిర nserror
rsvg_create(స్థిరరాశి content_handler *handler,
	    lwc_string *imime_type,
	    స్థిరరాశి నిర్మాణం http_parameter *params,
	    llcache_handle *llcache,
	    స్థిరరాశి అక్షరం *fallback_charset,
	    bool quirks,
	    నిర్మాణం content **c)
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

	*c = (నిర్మాణం content *)svg;

	తిరిగి NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
స్థిర నిర్మాణం bitmap *
rsvg_cache_convert(నిర్మాణం content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	నిర్మాణం bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ఒకవేళ ((bitmap = guit->bitmap->create(c->వెడల్పు, c->ఎత్తు,	BITMAP_NONE)) == ఖాళీ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		తిరిగి ఖాళీ;
	}

	ఒకవేళ ((cs = cairo_image_surface_create_for_దత్తాంశం(
		     (గుర్తులేని అక్షరం *)guit->bitmap->get_బఫరు(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->వెడల్పు, c->ఎత్తు,
		     guit->bitmap->get_rowstride(bitmap))) == ఖాళీ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		తిరిగి ఖాళీ;
	}
	ఒకవేళ ((cr = cairo_create(cs)) == ఖాళీ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		తిరిగి ఖాళీ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.వెడల్పు = c->వెడల్పు;
	viewport.ఎత్తు = c->ఎత్తు;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ఖాళీ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->వెడల్పు, c->ఎత్తు);

	bitmap_format_గమ్యం_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	తిరిగి bitmap;
}

స్థిర శూన్య rsvg__get_demensions(స్థిరరాశి rsvg_content *svgc,
		పూర్ణాంకం *వెడల్పు, పూర్ణాంకం *ఎత్తు)
{
#ఒకవేళ LIBRSVG_MAJOR_సంస్కరణ >= 2 && LIBRSVG_MINOR_సంస్కరణ >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_కొలత_లోపల_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ఒకవేళ (gotsize == TRUE) {
		*వెడల్పు = rwidth;
		*ఎత్తు = rheight;
	} లేకపోతే {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ఖాళీ,
						     &ink_rect,
						     &logical_rect,
						     ఖాళీ);
		*వెడల్పు = ink_rect.వెడల్పు;
		*ఎత్తు = ink_rect.ఎత్తు;
	}
#లేకపోతే
	Rsvg_Dimension_దత్తాంశం rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*వెడల్పు = rsvgsize.వెడల్పు;
	*ఎత్తు = rsvgsize.ఎత్తు;
#అయితే_ముగింపు
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *వెడల్పు, *ఎత్తు);
}

స్థిర bool rsvg_convert(నిర్మాణం content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	స్థిరరాశి uint8_t *దత్తాంశం; /* content data */
	size_t కొలత; /* content data size */
	GInput_ప్రవాహం * istream;
	GError *gerror = ఖాళీ;

	/* check image header is valid and get width/height */

	దత్తాంశం = content_get_ఆధారం_దత్తాంశం_2(c, &కొలత);

	istream = g_జ్ఞాపకం_ఆదానం_ప్రవాహం_new_from_దత్తాంశం(దత్తాంశం, కొలత, ఖాళీ);
	svgc->rsvgh = rsvg_handle_new_from_ప్రవాహం_sync(istream,
						       ఖాళీ,
						       RSVG_HANDLE_FLAGS_NONE,
						       ఖాళీ,
						       &gerror);
	g_object_unref(istream);
	ఒకవేళ (svgc->rsvgh == ఖాళీ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		తిరిగి false;
	}

	rsvg__get_demensions(svgc, &c->వెడల్పు, &c->ఎత్తు);

	c->కొలత = c->వెడల్పు * c->ఎత్తు * 4;

	image_cache_కూడు(c, ఖాళీ, rsvg_cache_convert);

	content_సమితి_ready(c);
	content_సమితి_పూర్తి(c);
	content_సమితి_స్థితి(c, ""); /* Done: update status bar */

	తిరిగి true;
}


స్థిర nserror rsvg_clone(స్థిరరాశి నిర్మాణం content *old, నిర్మాణం content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(rsvg_content));
	ఒకవేళ (svg == ఖాళీ)
		తిరిగి NSERROR_NOMEM;

	error = content__clone(old, &svg->పునాది);
	ఒకవేళ (error != NSERROR_OK) {
		content_destroy(&svg->పునాది);
		తిరిగి error;
	}

	/* re-convert if the content is ready */
	ఒకవేళ ((old->స్థితి_2 == CONTENT_స్థితి_READY) ||
	    (old->స్థితి_2 == CONTENT_స్థితి_పూర్తి)) {
		ఒకవేళ (rsvg_convert(&svg->పునాది) == false) {
			content_destroy(&svg->పునాది);
			తిరిగి NSERROR_CLONE_FAILED;
		}
	}

	*newc = (నిర్మాణం content *)svg;

	తిరిగి NSERROR_OK;
}


స్థిర శూన్య rsvg_destroy(నిర్మాణం content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ఒకవేళ (d->rsvgh != ఖాళీ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ఖాళీ;
	}

	తిరిగి image_cache_destroy(c);
}

స్థిర స్థిరరాశి content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.దత్తాంశం_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_అంతర్గత = image_cache_get_అంతర్గత,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

స్థిర స్థిరరాశి అక్షరం *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_రిజిస్టరు_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

