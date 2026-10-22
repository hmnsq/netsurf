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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>

#include <librsvg/rsvg.h>

#include <nsutils/endian.h>

#include "utils/log.h"
#include "utils/utils.h"
#include "utils/messages.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "content/llcache.h"
#include "content/content_protected.h"
#include "content/content_factory.h"
#include "desktop/gui_internal.h"
#include "desktop/bitmap.h"

#include "image/image_cache.h"

#include "image/rsvg.h"


typedef struct rsvg_content {
	struct content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


static nserror
rsvg_create(const content_handler *handler,
	    lwc_string *imime_type,
	    const struct http_parameter *params,
	    llcache_handle *llcache,
	    const char *fallback_charset,
	    bool quirks,
	    struct content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, sizeof(rsvg_content));
	如果 (svg == NULL)
		返回 NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	如果 (error != NSERROR_OK) {
		free(svg);
		返回 error;
	}

	*c = (struct content *)svg;

	返回 NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
static struct bitmap *
rsvg_cache_convert(struct content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	struct bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	如果 ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == NULL) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		返回 NULL;
	}

	如果 ((cs = cairo_image_surface_create_for_data(
		     (unsigned char *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == NULL) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		返回 NULL;
	}
	如果 ((cr = cairo_create(cs)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		返回 NULL;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = c->width;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, NULL);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->width, c->height);

	bitmap_format_to_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	返回 bitmap;
}

static void rsvg__get_demensions(const rsvg_content *svgc,
		整数 *width, 整数 *height)
{
#如果 LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	如果 (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} 否则 {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     NULL,
						     &ink_rect,
						     &logical_rect,
						     NULL);
		*width = ink_rect.width;
		*height = ink_rect.height;
	}
#否则
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

static bool rsvg_convert(struct content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	const uint8_t *data; /* content data */
	size_t size; /* content data size */
	GInputStream * istream;
	GError *gerror = NULL;

	/* check image header is valid and get width/height */

	data = content__get_source_data(c, &size);

	istream = g_memory_input_stream_new_from_data(data, size, NULL);
	svgc->rsvgh = rsvg_handle_new_from_stream_sync(istream,
						       NULL,
						       RSVG_HANDLE_FLAGS_NONE,
						       NULL,
						       &gerror);
	g_object_unref(istream);
	如果 (svgc->rsvgh == NULL) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		返回 false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, NULL, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	返回 true;
}


static nserror rsvg_clone(const struct content *old, struct content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, sizeof(rsvg_content));
	如果 (svg == NULL)
		返回 NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	如果 (error != NSERROR_OK) {
		content_destroy(&svg->base);
		返回 error;
	}

	/* re-convert if the content is ready */
	如果 ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		如果 (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			返回 NSERROR_CLONE_FAILED;
		}
	}

	*newc = (struct content *)svg;

	返回 NSERROR_OK;
}


static void rsvg_destroy(struct content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	如果 (d->rsvgh != NULL) {
		g_object_unref(d->rsvgh);
		d->rsvgh = NULL;
	}

	返回 image_cache_destroy(c);
}

static const content_handler rsvg_content_handler = {
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

static const char *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

