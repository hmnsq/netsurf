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

#شامل <stdlib.h>
#شامل <stdbool.h>
#شامل <assert.h>
#شامل <string.h>
#شامل <sys/types.h>

#شامل <librsvg/rsvg.h>

#شامل <nsutils/endian.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/image_cache.h"

#شامل "image/rsvg.h"


تعریفنوع ساختار rsvg_content {
	ساختار content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ایستا nserror
rsvg_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت ساختار http_parameter *params,
	    llcache_handle *llcache,
	    ثابت نویسه *fallback_charset,
	    bool quirks,
	    ساختار content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, اندازه(rsvg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		بازگشت error;
	}

	*c = (ساختار content *)svg;

	بازگشت NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ایستا ساختار bitmap *
rsvg_cache_convert(ساختار content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ساختار bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	اگر ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == پوچ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		بازگشت پوچ;
	}

	اگر ((cs = cairo_image_surface_create_for_data(
		     (unsigned نویسه *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == پوچ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		بازگشت پوچ;
	}
	اگر ((cr = cairo_create(cs)) == پوچ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		بازگشت پوچ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, پوچ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	بازگشت bitmap;
}

ایستا تهی rsvg__get_demensions(ثابت rsvg_content *svgc,
		صحیح *width, صحیح *height)
{
#اگر LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	اگر (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} وگرنه {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     پوچ,
						     &ink_rect,
						     &logical_rect,
						     پوچ);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#وگرنه
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

ایستا bool rsvg_convert(ساختار content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ثابت uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = پوچ;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_درون_داد_stream_new_from_data(data, size, پوچ);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       پوچ,
						       RSVG_HANDLE_FLAGS_NONE,
						       پوچ,
						       &gerror);
	g_object_unref(istream);
	اگر (svgc->rsvgh == پوچ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		بازگشت false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, پوچ, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	بازگشت true;
}


ایستا nserror rsvg_clone(ثابت ساختار content *old, ساختار content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, اندازه(rsvg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->base);
		بازگشت error;
	}

	/* re-convert if the content is ready */
	اگر ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		اگر (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *)svg;

	بازگشت NSERROR_OK;
}


ایستا تهی rsvg_destroy(ساختار content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->rsvgh != پوچ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = پوچ;
	}

	بازگشت image_cache_destroy(c);
}

ایستا ثابت content_handler rsvg_content_handler = {
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

ایستا ثابت نویسه *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

