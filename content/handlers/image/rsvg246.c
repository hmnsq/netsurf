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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव <librsvg/rsvg.h>

#अन्तर्भाव <nsutils/endian.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"

#अन्तर्भाव "image/rsvg.h"


प्रकारनिर्देश संरचना rsvg_content {
	संरचना content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


स्थिर nserror
rsvg_create(अचर content_handler *handler,
	    lwc_string *imime_type,
	    अचर संरचना http_parameter *params,
	    llcache_handle *llcache,
	    अचर वर्ण *fallback_charset,
	    bool quirks,
	    संरचना content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, परिमाण(rsvg_content));
	यदि (svg == रिक्त)
		फर्काऊ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(svg);
		फर्काऊ error;
	}

	*c = (संरचना content *)svg;

	फर्काऊ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
स्थिर संरचना bitmap *
rsvg_cache_convert(संरचना content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	संरचना bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	यदि ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		फर्काऊ रिक्त;
	}

	यदि ((cs = cairo_image_surface_create_for_data(
		     (अचिन्हित वर्ण *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		फर्काऊ रिक्त;
	}
	यदि ((cr = cairo_create(cs)) == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		फर्काऊ रिक्त;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, रिक्त);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	फर्काऊ bitmap;
}

स्थिर शून्य rsvg__get_demensions(अचर rsvg_content *svgc,
		पूर्णाङ्क *width, पूर्णाङ्क *height)
{
#यदि LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	यदि (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} अन्यथा {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     रिक्त,
						     &ink_rect,
						     &logical_rect,
						     रिक्त);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#अन्यथा
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

स्थिर bool rsvg_convert(संरचना content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	अचर uint8_t *data; /* content data */
	माप_प्रकार size; /* content data size */
	GInputStream * istream;
	GError *gerror = रिक्त;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_आगत_stream_new_from_data(data, size, रिक्त);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       रिक्त,
						       RSVG_HANDLE_FLAGS_NONE,
						       रिक्त,
						       &gerror);
	g_object_unref(istream);
	यदि (svgc->rsvgh == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		फर्काऊ false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, रिक्त, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	फर्काऊ true;
}


स्थिर nserror rsvg_clone(अचर संरचना content *old, संरचना content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, परिमाण(rsvg_content));
	यदि (svg == रिक्त)
		फर्काऊ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&svg->base);
		फर्काऊ error;
	}

	/* re-convert if the content is ready */
	यदि ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		यदि (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			फर्काऊ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *)svg;

	फर्काऊ NSERROR_OK;
}


स्थिर शून्य rsvg_destroy(संरचना content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	यदि (d->rsvgh != रिक्त) {
		g_object_unref(d->rsvgh);
		d->rsvgh = रिक्त;
	}

	फर्काऊ image_cache_destroy(c);
}

स्थिर अचर content_handler rsvg_content_handler = {
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

स्थिर अचर वर्ण *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

