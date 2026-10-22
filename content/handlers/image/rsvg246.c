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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ <librsvg/rsvg.h>

#അന്തര്ഭാവ <nsutils/endian.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/image_cache.h"

#അന്തര്ഭാവ "image/rsvg.h"


തരനിർവചനം ഘടന rsvg_content {
	ഘടന content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


സ്ഥിതം nserror
rsvg_create(സ്ഥിരം content_handler *handler,
	    lwc_string *imime_type,
	    സ്ഥിരം ഘടന http_parameter *params,
	    llcache_handle *llcache,
	    സ്ഥിരം അക്ഷരം *fallback_charset,
	    bool quirks,
	    ഘടന content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, വലുപ്പം(rsvg_content));
	എങ്കിൽ (svg == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(svg);
		തിരികെ error;
	}

	*c = (ഘടന content *)svg;

	തിരികെ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
സ്ഥിതം ഘടന bitmap *
rsvg_cache_convert(ഘടന content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ഘടന bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	എങ്കിൽ ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ ((cs = cairo_image_surface_create_for_data(
		     (unsigned അക്ഷരം *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		തിരികെ ശൂന്യ;
	}
	എങ്കിൽ ((cr = cairo_create(cs)) == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		തിരികെ ശൂന്യ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ശൂന്യ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	തിരികെ bitmap;
}

സ്ഥിതം ശൂന്യം rsvg__get_demensions(സ്ഥിരം rsvg_content *svgc,
		പൂർണ്ണസംഖ്യ *width, പൂർണ്ണസംഖ്യ *height)
{
#എങ്കിൽ LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	എങ്കിൽ (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} അല്ലെങ്കിൽ {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ശൂന്യ,
						     &ink_rect,
						     &logical_rect,
						     ശൂന്യ);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#അല്ലെങ്കിൽ
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

സ്ഥിതം bool rsvg_convert(ഘടന content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	സ്ഥിരം uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = ശൂന്യ;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_നിവേശം_stream_new_from_data(data, size, ശൂന്യ);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       ശൂന്യ,
						       RSVG_HANDLE_FLAGS_NONE,
						       ശൂന്യ,
						       &gerror);
	g_object_unref(istream);
	എങ്കിൽ (svgc->rsvgh == ശൂന്യ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		തിരികെ false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, ശൂന്യ, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	തിരികെ true;
}


സ്ഥിതം nserror rsvg_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, വലുപ്പം(rsvg_content));
	എങ്കിൽ (svg == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		തിരികെ error;
	}

	/* re-convert if the content is ready */
	എങ്കിൽ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		എങ്കിൽ (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *)svg;

	തിരികെ NSERROR_OK;
}


സ്ഥിതം ശൂന്യം rsvg_destroy(ഘടന content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	എങ്കിൽ (d->rsvgh != ശൂന്യ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ശൂന്യ;
	}

	തിരികെ image_cache_destroy(c);
}

സ്ഥിതം സ്ഥിരം content_handler rsvg_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

