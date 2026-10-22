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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ <librsvg/rsvg.h>

#ಅನ್ತರ್ಭಾವ <nsutils/endian.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"

#ಅನ್ತರ್ಭಾವ "image/rsvg.h"


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ rsvg_content {
	ಸಂರಚನಾ content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ಸ್ಥಿರ nserror
rsvg_create(ಧ್ರುವ content_handler *handler,
	    lwc_string *imime_type,
	    ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
	    llcache_handle *llcache,
	    ಧ್ರುವ ವರ್ಣ *fallback_charset,
	    bool quirks,
	    ಸಂರಚನಾ content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ಪರಿಮಾಣ(rsvg_content));
	ಯದಿ (svg == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(svg);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *)svg;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ಸ್ಥಿರ ಸಂರಚನಾ bitmap *
rsvg_cache_convert(ಸಂರಚನಾ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ಸಂರಚನಾ bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ಯದಿ ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ ((cs = cairo_image_surface_create_for_data(
		     (unsigned ವರ್ಣ *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಯದಿ ((cr = cairo_create(cs)) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ರಿಕ್ತ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	ಪ್ರತ್ಯಾಗಮ bitmap;
}

ಸ್ಥಿರ ಶೂನ್ಯ rsvg__get_demensions(ಧ್ರುವ rsvg_content *svgc,
		ಪೂರ್ಣಾಙ್ಕ *width, ಪೂರ್ಣಾಙ್ಕ *height)
{
#ಯದಿ LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ಯದಿ (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} ಅನ್ಯಥಾ {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ರಿಕ್ತ,
						     &ink_rect,
						     &logical_rect,
						     ರಿಕ್ತ);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#ಅನ್ಯಥಾ
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

ಸ್ಥಿರ bool rsvg_convert(ಸಂರಚನಾ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ಧ್ರುವ uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = ರಿಕ್ತ;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_ಒಳಹರಿವು_stream_new_from_data(data, size, ರಿಕ್ತ);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       ರಿಕ್ತ,
						       RSVG_HANDLE_FLAGS_NONE,
						       ರಿಕ್ತ,
						       &gerror);
	g_object_unref(istream);
	ಯದಿ (svgc->rsvgh == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ಪ್ರತ್ಯಾಗಮ false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, ರಿಕ್ತ, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ಪ್ರತ್ಯಾಗಮ true;
}


ಸ್ಥಿರ nserror rsvg_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ಪರಿಮಾಣ(rsvg_content));
	ಯದಿ (svg == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* re-convert if the content is ready */
	ಯದಿ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ಯದಿ (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *)svg;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ ಶೂನ್ಯ rsvg_destroy(ಸಂರಚನಾ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಯದಿ (d->rsvgh != ರಿಕ್ತ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ image_cache_destroy(c);
}

ಸ್ಥಿರ ಧ್ರುವ content_handler rsvg_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

