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

#समावेश <stdlib.h>
#समावेश <stdbool.h>
#समावेश <assert.h>
#समावेश <string.h>
#समावेश <sys/types.h>

#समावेश <librsvg/rsvg.h>

#समावेश <nsutils/endian.h>

#समावेश "utils/log.h"
#समावेश "utils/utils.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/image_cache.h"

#समावेश "image/rsvg.h"


प्रकार काष्ठा rsvg_content {
	काष्ठा content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


अटल nserror
rsvg_create(स्थिर content_handler *handler,
	    lwc_string *imime_type,
	    स्थिर काष्ठा http_parameter *params,
	    llcache_handle *llcache,
	    स्थिर अक्षर *fallback_charset,
	    bool quirks,
	    काष्ठा content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, माप(rsvg_content));
	अगर (svg == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(svg);
		वापस error;
	}

	*c = (काष्ठा content *)svg;

	वापस NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
अटल काष्ठा bitmap *
rsvg_cache_convert(काष्ठा content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	काष्ठा bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	अगर ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == शून्य) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		वापस शून्य;
	}

	अगर ((cs = cairo_image_surface_create_for_तथ्य(
		     (अचिन्हित अक्षर *)guit->bitmap->get_बफर(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == शून्य) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		वापस शून्य;
	}
	अगर ((cr = cairo_create(cs)) == शून्य) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		वापस शून्य;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, शून्य);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	वापस bitmap;
}

अटल व्योम rsvg__get_demensions(स्थिर rsvg_content *svgc,
		पूर्णांक *width, पूर्णांक *height)
{
#अगर LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_आकार_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	अगर (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} अन्यथा {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     शून्य,
						     &ink_rect,
						     &logical_rect,
						     शून्य);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#अन्यथा
	Rsvg_Dimension_तथ्य rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#पूर्ण_अगर
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

अटल bool rsvg_convert(काष्ठा content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	स्थिर uint8_t *तथ्य; /* content data */
	माप_प्रकार आकार; /* content data size */
	GInputStream * istream;
	GError *gerror = शून्य;

	/* check image header is valid and get width/height */

	तथ्य = content_get_source_तथ्य_2(c, &आकार);

	istream = g_memory_निवेश_stream_new_from_तथ्य(तथ्य, आकार, शून्य);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       शून्य,
						       RSVG_HANDLE_FLAGS_NONE,
						       शून्य,
						       &gerror);
	g_object_unref(istream);
	अगर (svgc->rsvgh == शून्य) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		वापस false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->आकार = c->width * c->height * 4;

	image_cache_add(c, शून्य, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	वापस true;
}


अटल nserror rsvg_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, माप(rsvg_content));
	अगर (svg == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&svg->base);
		वापस error;
	}

	/* re-convert if the content is ready */
	अगर ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		अगर (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *)svg;

	वापस NSERROR_OK;
}


अटल व्योम rsvg_destroy(काष्ठा content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	अगर (d->rsvgh != शून्य) {
		g_object_unref(d->rsvgh);
		d->rsvgh = शून्य;
	}

	वापस image_cache_destroy(c);
}

अटल स्थिर content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.तथ्य_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_internal = image_cache_get_internal,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

