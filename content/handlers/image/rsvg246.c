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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ <librsvg/rsvg.h>

#અન્તર્ભાવ <nsutils/endian.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/llcache.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"
#અન્તર્ભાવ "desktop/gui_internal.h"
#અન્તર્ભાવ "desktop/bitmap.h"

#અન્તર્ભાવ "image/image_cache.h"

#અન્તર્ભાવ "image/rsvg.h"


પ્રકારવ્યાખ્યા સંરચના rsvg_content {
	સંરચના content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


સ્થિર nserror
rsvg_create(અચલ content_handler *handler,
	    lwc_string *imime_type,
	    અચલ સંરચના http_parameter *params,
	    llcache_handle *llcache,
	    અચલ અક્ષર *fallback_charset,
	    bool quirks,
	    સંરચના content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, કદ(rsvg_content));
	જો (svg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(svg);
		પાછા error;
	}

	*c = (સંરચના content *)svg;

	પાછા NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
સ્થિર સંરચના bitmap *
rsvg_cache_convert(સંરચના content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	સંરચના bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	જો ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == ખાલી) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		પાછા ખાલી;
	}

	જો ((cs = cairo_image_surface_create_for_data(
		     (unsigned અક્ષર *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == ખાલી) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		પાછા ખાલી;
	}
	જો ((cr = cairo_create(cs)) == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		પાછા ખાલી;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ખાલી);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	પાછા bitmap;
}

સ્થિર શૂન્ય rsvg__get_demensions(અચલ rsvg_content *svgc,
		પૂર્ણાંક *width, પૂર્ણાંક *height)
{
#જો LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	જો (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} નહીંતર {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ખાલી,
						     &ink_rect,
						     &logical_rect,
						     ખાલી);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#નહીંતર
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

સ્થિર bool rsvg_convert(સંરચના content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	અચલ uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = ખાલી;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_આગત_stream_new_from_data(data, size, ખાલી);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       ખાલી,
						       RSVG_HANDLE_FLAGS_NONE,
						       ખાલી,
						       &gerror);
	g_object_unref(istream);
	જો (svgc->rsvgh == ખાલી) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		પાછા false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, ખાલી, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	પાછા true;
}


સ્થિર nserror rsvg_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, કદ(rsvg_content));
	જો (svg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	જો (error != NSERROR_OK) {
		content_destroy(&svg->base);
		પાછા error;
	}

	/* re-convert if the content is ready */
	જો ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		જો (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *)svg;

	પાછા NSERROR_OK;
}


સ્થિર શૂન્ય rsvg_destroy(સંરચના content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	જો (d->rsvgh != ખાલી) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ખાલી;
	}

	પાછા image_cache_destroy(c);
}

સ્થિર અચલ content_handler rsvg_content_handler = {
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

સ્થિર અચલ અક્ષર *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

