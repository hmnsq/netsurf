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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#ಅನ್ತರ್ಭಾವ <librsvg/rsvg-cairo.h>
#endif

#ಅನ್ತರ್ಭಾವ <nsutils/endian.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/rsvg.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ rsvg_content {
	ಸಂರಚನಾ content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ಸಂರಚನಾ bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ಸ್ಥಿರ nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = ರಿಕ್ತ;
	c->cs = ರಿಕ್ತ;
	c->ct = ರಿಕ್ತ;
	c->bitmap = ರಿಕ್ತ;

	ಯದಿ ((c->rsvgh = rsvg_handle_new()) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
	}

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ nserror rsvg_create(ಧ್ರುವ content_handler *handler,
		lwc_string *imime_type, ಧ್ರುವ ಸಂರಚನಾ http_parameter *params,
		llcache_handle *llcache, ಧ್ರುವ ವರ್ಣ *fallback_charset,
		bool quirks, ಸಂರಚನಾ content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ಪರಿಮಾಣ(rsvg_content));
	ಯದಿ (svg == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(svg);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	error = rsvg_create_svg_data(svg);
	ಯದಿ (error != NSERROR_OK) {
		ಮುಕ್ತ(svg);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	*c = (ಸಂರಚನಾ content *) svg;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}


ಸ್ಥಿರ bool rsvg_process_data(ಸಂರಚನಾ content *c, ಧ್ರುವ ವರ್ಣ *data,
			unsigned ಪೂರ್ಣಾಙ್ಕ size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ರಿಕ್ತ;

	ಯದಿ (rsvg_handle_write(d->rsvgh, (ಧ್ರುವ guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool rsvg_convert(ಸಂರಚನಾ content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = ರಿಕ್ತ;

	ಯದಿ (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತಿಜ್ಞಾ(err == ರಿಕ್ತ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	ಯದಿ ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ ((d->cs = cairo_image_surface_create_for_data(
			(unsigned ವರ್ಣ *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ ((d->ct = cairo_create(d->cs)) == ರಿಕ್ತ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ false;
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

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ bool rsvg_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಪ್ರತಿಜ್ಞಾ(rsvgcontent->bitmap != ರಿಕ್ತ);

	ಯದಿ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ಯದಿ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ಪ್ರತ್ಯಾಗಮ (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

ಸ್ಥಿರ ಶೂನ್ಯ rsvg_destroy(ಸಂರಚನಾ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಯದಿ (d->bitmap != ರಿಕ್ತ) guit->bitmap->destroy(d->bitmap);
	ಯದಿ (d->rsvgh != ರಿಕ್ತ) g_object_unref(d->rsvgh);
	ಯದಿ (d->ct != ರಿಕ್ತ) cairo_destroy(d->ct);
	ಯದಿ (d->cs != ರಿಕ್ತ) cairo_surface_destroy(d->cs);

	ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ nserror rsvg_clone(ಧ್ರುವ ಸಂರಚನಾ content *old, ಸಂರಚನಾ content **newc)
{
	rsvg_content *svg;
	nserror error;
	ಧ್ರುವ uint8_t *data;
	size_t size;

	svg = calloc(1, ಪರಿಮಾಣ(rsvg_content));
	ಯದಿ (svg == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	ಯದಿ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ಪ್ರತ್ಯಾಗಮ error;
	}

	data = content__get_source_data(&svg->base, &size);
	ಯದಿ (size > 0) {
		ಯದಿ (rsvg_process_data(&svg->base, (ಧ್ರುವ ವರ್ಣ *)data, size) == false) {
			content_destroy(&svg->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_NOMEM;
		}
	}

	ಯದಿ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ಯದಿ (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ಪ್ರತ್ಯಾಗಮ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ಸಂರಚನಾ content *) svg;

	ಪ್ರತ್ಯಾಗಮ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *rsvg_get_internal(ಧ್ರುವ ಸಂರಚನಾ content *c, ಶೂನ್ಯ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ಪ್ರತ್ಯಾಗಮ d->bitmap;
}

ಸ್ಥಿರ content_type rsvg_content_type(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ CONTENT_IMAGE;
}


ಸ್ಥಿರ bool rsvg_content_is_opaque(ಸಂರಚನಾ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಯದಿ (d->bitmap != ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ guit->bitmap->get_opaque(d->bitmap);
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


ಸ್ಥಿರ ಧ್ರುವ content_handler rsvg_content_handler = {
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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

