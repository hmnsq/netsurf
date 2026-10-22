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


typedef sa_nracanaa rsvg_content {
	sa_nracanaa content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


static nserror
rsvg_create(const content_handler *handler,
	    lwc_string *imime_type,
	    const sa_nracanaa http_parameter *params,
	    llcache_handle *llcache,
	    const var_na *fallback_charset,
	    bool quirks,
	    sa_nracanaa content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, parimaa_na(rsvg_content));
	ຖ້າ (svg == NULL)
		ສົ່ງຄືນ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	ຖ້າ (error != NSERROR_OK) {
		free(svg);
		ສົ່ງຄືນ error;
	}

	*c = (sa_nracanaa content *)svg;

	ສົ່ງຄືນ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
static sa_nracanaa bitmap *
rsvg_cache_convert(sa_nracanaa content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	sa_nracanaa bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ຖ້າ ((bitmap = guit->bitmap->create(c->width, c->height,	BITMAP_NONE)) == NULL) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ສົ່ງຄືນ NULL;
	}

	ຖ້າ ((cs = cairo_image_surface_create_for_data(
		     (unsigned var_na *)guit->bitmap->get_buffer(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->width, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == NULL) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ສົ່ງຄືນ NULL;
	}
	ຖ້າ ((cr = cairo_create(cs)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ສົ່ງຄືນ NULL;
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

	ສົ່ງຄືນ bitmap;
}

static void rsvg__get_demensions(const rsvg_content *svgc,
		ຈຳນວນເຕັມ *width, ຈຳນວນເຕັມ *height)
{
#ຖ້າ LIBRSVG_MAJOR_VERSION >= 2 && LIBRSVG_MINOR_VERSION >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_size_in_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ຖ້າ (gotsize == TRUE) {
		*width = rwidth;
		*height = rheight;
	} ບໍ່ດັ່ງນັ້ນ {
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
#ບໍ່ດັ່ງນັ້ນ
	RsvgDimensionData rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*width = rsvgsize.width;
	*height = rsvgsize.height;
#endif
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *width, *height);
}

static bool rsvg_convert(sa_nracanaa content *c)
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
	ຖ້າ (svgc->rsvgh == NULL) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ສົ່ງຄືນ false;
	}

	rsvg__get_demensions(svgc, &c->width, &c->height);

	c->size = c->width * c->height * 4;

	image_cache_add(c, NULL, rsvg_cache_convert);

	content_set_ready(c);
	content_set_done(c);
	content_set_status(c, ""); /* Done: update status bar */

	ສົ່ງຄືນ true;
}


static nserror rsvg_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, parimaa_na(rsvg_content));
	ຖ້າ (svg == NULL)
		ສົ່ງຄືນ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ຖ້າ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ສົ່ງຄືນ error;
	}

	/* re-convert if the content is ready */
	ຖ້າ ((old->status == CONTENT_STATUS_READY) ||
	    (old->status == CONTENT_STATUS_DONE)) {
		ຖ້າ (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ສົ່ງຄືນ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *)svg;

	ສົ່ງຄືນ NSERROR_OK;
}


static void rsvg_destroy(sa_nracanaa content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ຖ້າ (d->rsvgh != NULL) {
		g_object_unref(d->rsvgh);
		d->rsvgh = NULL;
	}

	ສົ່ງຄືນ image_cache_destroy(c);
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

static const var_na *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

