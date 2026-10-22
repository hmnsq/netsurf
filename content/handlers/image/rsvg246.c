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

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/types.h>

#অন্তর্ভাৱ <librsvg/rsvg.h>

#অন্তর্ভাৱ <nsutils/endian.h>

#অন্তর্ভাৱ "utils/log.h"
#অন্তর্ভাৱ "utils/utils.h"
#অন্তর্ভাৱ "utils/messages.h"
#অন্তর্ভাৱ "netsurf/plotters.h"
#অন্তর্ভাৱ "netsurf/bitmap.h"
#অন্তর্ভাৱ "netsurf/content.h"
#অন্তর্ভাৱ "content/llcache.h"
#অন্তর্ভাৱ "content/content_protected.h"
#অন্তর্ভাৱ "content/content_factory.h"
#অন্তর্ভাৱ "desktop/gui_internal.h"
#অন্তর্ভাৱ "desktop/bitmap.h"

#অন্তর্ভাৱ "image/image_cache.h"

#অন্তর্ভাৱ "image/rsvg.h"


প্রকারনির্দেশ সংরচনা rsvg_content {
	সংরচনা content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


স্থির nserror
rsvg_create(ধ্রুৱ content_handler *handler,
	    lwc_string *imime_type,
	    ধ্রুৱ সংরচনা http_parameter *params,
	    llcache_handle *llcache,
	    ধ্রুৱ ৱর্ণ *fallback_charset,
	    bool quirks,
	    সংরচনা content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, পরিমাণ(rsvg_content));
	যদি (svg == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(svg);
		ঘূৰাই error;
	}

	*c = (সংরচনা content *)svg;

	ঘূৰাই NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
স্থির সংরচনা bitmap *
rsvg_cache_convert(সংরচনা content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	সংরচনা bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	যদি ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == রিক্ত) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ঘূৰাই রিক্ত;
	}

	যদি ((cs = cairo_image_surface_create_for_data(
		     (unsigned ৱর্ণ *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == রিক্ত) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ঘূৰাই রিক্ত;
	}
	যদি ((cr = cairo_create(cs)) == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ঘূৰাই রিক্ত;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, রিক্ত);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	ঘূৰাই bitmap;
}

স্থির শূন্য rsvg__get_demensions(ধ্রুৱ rsvg_content *svgc,
		পূর্ণাঙ্ক *width, পূর্ণাঙ্ক *height)
{
#যদি LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	যদি (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} অন্যথা {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     রিক্ত,
						     &ink_rect,
						     &logical_rect,
						     রিক্ত);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#অন্যথা
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

স্থির bool rsvg_convert(সংরচনা content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ধ্রুৱ uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = রিক্ত;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_প্ৰবেশ_stream_new_from_data(data, size, রিক্ত);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       রিক্ত,
						       RSVG_HANDLE_FLAGS_NONE,
						       রিক্ত,
						       &gerror);
	g_object_unref(istream);
	যদি (svgc->rsvgh == রিক্ত) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ঘূৰাই false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, রিক্ত, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ঘূৰাই true;
}


স্থির nserror rsvg_clone(ধ্রুৱ সংরচনা content *old, সংরচনা content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, পরিমাণ(rsvg_content));
	যদি (svg == রিক্ত)
		ঘূৰাই NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ঘূৰাই error;
	}

	/* re-convert if the content is ready */
	যদি ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		যদি (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ঘূৰাই NSERROR_CLONE_FAILED;
		}
	}

	*newc = (সংরচনা content *)svg;

	ঘূৰাই NSERROR_OK;
}


স্থির শূন্য rsvg_destroy(সংরচনা content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	যদি (d->rsvgh != রিক্ত) {
		g_object_unref(d->rsvgh);
		d->rsvgh = রিক্ত;
	}

	ঘূৰাই image_cache_destroy(c);
}

স্থির ধ্রুৱ content_handler rsvg_content_handler = {
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

স্থির ধ্রুৱ ৱর্ণ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

