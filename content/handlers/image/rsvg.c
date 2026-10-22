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

typedef struct rsvg_content {
	struct content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	struct bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

static nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = NULL;
	c->cs = NULL;
	c->ct = NULL;
	c->bitmap = NULL;

	ако ((c->rsvgh = rsvg_handle_new()) == NULL) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
		върни NSERROR_NOMEM;
	}

	върни NSERROR_OK;
}


static nserror rsvg_create(const content_handler *handler,
		lwc_string *imime_type, const struct http_parameter *params,
		llcache_handle *llcache, const char *fallback_charset,
		bool quirks, struct content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, sizeof(rsvg_content));
	ако (svg == NULL)
		върни NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ако (error != NSERROR_OK) {
		free(svg);
		върни error;
	}

	error = rsvg_create_svg_data(svg);
	ако (error != NSERROR_OK) {
		free(svg);
		върни error;
	}

	*c = (struct content *) svg;

	върни NSERROR_OK;
}


static bool rsvg_process_data(struct content *c, const char *data,
			unsigned цяло size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = NULL;

	ако (rsvg_handle_write(d->rsvgh, (const guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, NULL);
		върни false;
	}

	върни true;
}

static bool rsvg_convert(struct content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = NULL;

	ако (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, NULL);
		върни false;
	}

	assert(err == NULL);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	ако ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		върни false;
	}

	ако ((d->cs = cairo_image_surface_create_for_data(
			(unsigned char *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		върни false;
	}

	ако ((d->ct = cairo_create(d->cs)) == NULL) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, NULL);
		върни false;
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

	върни true;
}

static bool rsvg_redraw(struct content *c, struct content_redraw_data *data,
		const struct rect *clip, const struct redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	assert(rsvgcontent->bitmap != NULL);

	ако (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ако (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	върни (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

static void rsvg_destroy(struct content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ако (d->bitmap != NULL) guit->bitmap->destroy(d->bitmap);
	ако (d->rsvgh != NULL) g_object_unref(d->rsvgh);
	ако (d->ct != NULL) cairo_destroy(d->ct);
	ако (d->cs != NULL) cairo_surface_destroy(d->cs);

	върни;
}

static nserror rsvg_clone(const struct content *old, struct content **newc)
{
	rsvg_content *svg;
	nserror error;
	const uint8_t *data;
	size_t size;

	svg = calloc(1, sizeof(rsvg_content));
	ако (svg == NULL)
		върни NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ако (error != NSERROR_OK) {
		content_destroy(&svg->base);
		върни error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	ако (error != NSERROR_OK) {
		content_destroy(&svg->base);
		върни error;
	}

	data = content__get_source_data(&svg->base, &size);
	ако (size > 0) {
		ако (rsvg_process_data(&svg->base, (const char *)data, size) == false) {
			content_destroy(&svg->base);
			върни NSERROR_NOMEM;
		}
	}

	ако (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ако (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			върни NSERROR_CLONE_FAILED;
		}
	}

	*newc = (struct content *) svg;

	върни NSERROR_OK;
}

static void *rsvg_get_internal(const struct content *c, void *context)
{
	rsvg_content *d = (rsvg_content *) c;

	върни d->bitmap;
}

static content_type rsvg_content_type(void)
{
	върни CONTENT_IMAGE;
}


static bool rsvg_content_is_opaque(struct content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ако (d->bitmap != NULL) {
		върни guit->bitmap->get_opaque(d->bitmap);
	}

	върни false;
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

static const char *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

