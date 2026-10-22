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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ <librsvg/rsvg.h>

#ଅନ୍ତର୍ଭାଵ <nsutils/endian.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/image_cache.h"

#ଅନ୍ତର୍ଭାଵ "image/rsvg.h"


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା rsvg_content {
	ସଂରଚନା content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ସ୍ଥିର nserror
rsvg_create(ଅଚଳ content_handler *handler,
	    lwc_string *imime_type,
	    ଅଚଳ ସଂରଚନା http_parameter *params,
	    llcache_handle *llcache,
	    ଅଚଳ ଅକ୍ଷର *fallback_charset,
	    bool quirks,
	    ସଂରଚନା content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ଆକାର(rsvg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(svg);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *)svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ସ୍ଥିର ସଂରଚନା bitmap *
rsvg_cache_convert(ସଂରଚନା content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ସଂରଚନା bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ଯଦି ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == ଖାଲି) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି ((cs = cairo_image_surface_create_for_data(
		     (unsigned ଅକ୍ଷର *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == ଖାଲି) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ଯଦି ((cr = cairo_create(cs)) == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ଖାଲି);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	ଫେରନ୍ତୁ bitmap;
}

ସ୍ଥିର ଶୂନ୍ୟ rsvg__get_demensions(ଅଚଳ rsvg_content *svgc,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *height)
{
#ଯଦି LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ଯଦି (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} ଅନ୍ୟଥା {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ଖାଲି,
						     &ink_rect,
						     &logical_rect,
						     ଖାଲି);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#ଅନ୍ୟଥା
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

ସ୍ଥିର bool rsvg_convert(ସଂରଚନା content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ଅଚଳ uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = ଖାଲି;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_ନିବେଶ_stream_new_from_data(data, size, ଖାଲି);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       ଖାଲି,
						       RSVG_HANDLE_FLAGS_NONE,
						       ଖାଲି,
						       &gerror);
	g_object_unref(istream);
	ଯଦି (svgc->rsvgh == ଖାଲି) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ଫେରନ୍ତୁ false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, ଖାଲି, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର nserror rsvg_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ଆକାର(rsvg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ଫେରନ୍ତୁ error;
	}

	/* re-convert if the content is ready */
	ଯଦି ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ଯଦି (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *)svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର ଶୂନ୍ୟ rsvg_destroy(ସଂରଚନା content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ଯଦି (d->rsvgh != ଖାଲି) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ଖାଲି;
	}

	ଫେରନ୍ତୁ image_cache_destroy(c);
}

ସ୍ଥିର ଅଚଳ content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.data_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

