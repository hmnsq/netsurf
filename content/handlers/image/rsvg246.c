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

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ <librsvg/rsvg.h>

#அந்தர்பாவ <nsutils/endian.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/image_cache.h"

#அந்தர்பாவ "image/rsvg.h"


வகைவரை கட்டமைப்பு rsvg_content {
	கட்டமைப்பு content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


நிலையான nserror
rsvg_create(மாறிலி content_handler *handler,
	    lwc_string *imime_type,
	    மாறிலி கட்டமைப்பு http_parameter *params,
	    llcache_handle *llcache,
	    மாறிலி எழுத்து *fallback_charset,
	    bool quirks,
	    கட்டமைப்பு content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, அளவு(rsvg_content));
	எனில் (svg == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(svg);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *)svg;

	திரும்பு NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
நிலையான கட்டமைப்பு bitmap *
rsvg_cache_convert(கட்டமைப்பு content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	கட்டமைப்பு bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	எனில் ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == வெற்று) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		திரும்பு வெற்று;
	}

	எனில் ((cs = cairo_image_surface_create_for_data(
		     (unsigned எழுத்து *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == வெற்று) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		திரும்பு வெற்று;
	}
	எனில் ((cr = cairo_create(cs)) == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		திரும்பு வெற்று;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, வெற்று);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	திரும்பு bitmap;
}

நிலையான வெறுமை rsvg__get_demensions(மாறிலி rsvg_content *svgc,
		முழு *width, முழு *height)
{
#எனில் LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	எனில் (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} இல்லையெனில் {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     வெற்று,
						     &ink_rect,
						     &logical_rect,
						     வெற்று);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#இல்லையெனில்
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

நிலையான bool rsvg_convert(கட்டமைப்பு content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	மாறிலி uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = வெற்று;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_உள்ளீடு_stream_new_from_data(data, size, வெற்று);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       வெற்று,
						       RSVG_HANDLE_FLAGS_NONE,
						       வெற்று,
						       &gerror);
	g_object_unref(istream);
	எனில் (svgc->rsvgh == வெற்று) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		திரும்பு false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, வெற்று, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	திரும்பு true;
}


நிலையான nserror rsvg_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, அளவு(rsvg_content));
	எனில் (svg == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&svg->base);
		திரும்பு error;
	}

	/* re-convert if the content is ready */
	எனில் ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		எனில் (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *)svg;

	திரும்பு NSERROR_OK;
}


நிலையான வெறுமை rsvg_destroy(கட்டமைப்பு content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	எனில் (d->rsvgh != வெற்று) {
		g_object_unref(d->rsvgh);
		d->rsvgh = வெற்று;
	}

	திரும்பு image_cache_destroy(c);
}

நிலையான மாறிலி content_handler rsvg_content_handler = {
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

நிலையான மாறிலி எழுத்து *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

