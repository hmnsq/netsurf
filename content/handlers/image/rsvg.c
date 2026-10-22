/*
 * Copyright 2007 Rob Kendrick <rjek@netsurf-browser.org>
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
 * implementation of content handler for image/svg using librsvg.
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
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif

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

#include "image/rsvg.h"

typedef sa_nracanaa rsvg_content {
	sa_nracanaa content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	sa_nracanaa bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

static nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = NULL;
	c->cs = NULL;
	c->ct = NULL;
	c->bitmap = NULL;

	གལ་སྲིད ((c->rsvgh = rsvg_handle_new()) == NULL) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		ལོག NSERROR_NOMEM;
	}

	ལོག NSERROR_OK;
}


static nserror rsvg_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, parimaa_na(rsvg_content));
	གལ་སྲིད (svg == NULL)
		ལོག NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	གལ་སྲིད (error != NSERROR_OK) {
		free(svg);
		ལོག error;
	}

	error = rsvg_create_svg_data(svg);
	གལ་སྲིད (error != NSERROR_OK) {
		free(svg);
		ལོག error;
	}

	*c = (sa_nracanaa content *) svg;

	ལོག NSERROR_OK;
}


static bool rsvg_process_data(sa_nracanaa content *c, const var_na *data,
			unsigned ཧྲིལ size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = NULL;

	གལ་སྲིད (rsvg_handle_write(d->rsvgh, (const guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, NULL);
		ལོག false;
	}

	ལོག true;
}

static bool rsvg_convert(sa_nracanaa content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = NULL;

	གལ་སྲིད (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, NULL);
		ལོག false;
	}

	assert(err == NULL);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	གལ་སྲིད ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ལོག false;
	}

	གལ་སྲིད ((d->cs = cairo_image_surface_create_for_data(
			(unsigned var_na *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ལོག false;
	}

	གལ་སྲིད ((d->ct = cairo_create(d->cs)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		ལོག false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_to_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_set_ready(c);
	content_set_done(c);
	/* Done: update status bar */
	content_set_status(c, "");

	ལོག true;
}

static bool rsvg_redraw(sa_nracanaa content *c, sa_nracanaa content_redraw_data *data,
		const sa_nracanaa rect *clip, const sa_nracanaa redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	assert(rsvgcontent->bitmap != NULL);

	གལ་སྲིད (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	གལ་སྲིད (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ལོག (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

static void rsvg_destroy(sa_nracanaa content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	གལ་སྲིད (d->bitmap != NULL) guit->bitmap->destroy(d->bitmap);
	གལ་སྲིད (d->rsvgh != NULL) g_object_unref(d->rsvgh);
	གལ་སྲིད (d->ct != NULL) cairo_destroy(d->ct);
	གལ་སྲིད (d->cs != NULL) cairo_surface_destroy(d->cs);

	ལོག;
}

static nserror rsvg_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	rsvg_content *svg;
	nserror error;
	const uint8_t *data;
	size_t size;

	svg = calloc(1, parimaa_na(rsvg_content));
	གལ་སྲིད (svg == NULL)
		ལོག NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	གལ་སྲིད (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ལོག error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	གལ་སྲིད (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ལོག error;
	}

	data = content__get_source_data(&svg->base, &size);
	གལ་སྲིད (size > 0) {
		གལ་སྲིད (rsvg_process_data(&svg->base, (const var_na *)data, size) == false) {
			content_destroy(&svg->base);
			ལོག NSERROR_NOMEM;
		}
	}

	གལ་སྲིད (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		གལ་སྲིད (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ལོག NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) svg;

	ལོག NSERROR_OK;
}

static void *rsvg_get_internal(const sa_nracanaa content *c, void *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ལོག d->bitmap;
}

static content_type rsvg_content_type(void)
{
	ལོག CONTENT_IMAGE;
}


static bool rsvg_content_is_opaque(sa_nracanaa content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	གལ་སྲིད (d->bitmap != NULL) {
		ལོག guit->bitmap->get_opaque(d->bitmap);
	}

	ལོག false;
}


static const content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_data = rsvg_process_data,
	.data_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_internal = rsvg_get_internal,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

static const var_na *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

