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

#ضمن <stdlib.h>
#ضمن <stdbool.h>
#ضمن <assert.h>
#ضمن <string.h>
#ضمن <sys/types.h>

#ضمن <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#ضمن <librsvg/rsvg-cairo.h>
#endif

#ضمن <nsutils/endian.h>

#ضمن "utils/log.h"
#ضمن "utils/utils.h"
#ضمن "utils/messages.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/content.h"
#ضمن "content/llcache.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"
#ضمن "desktop/gui_internal.h"
#ضمن "desktop/bitmap.h"

#ضمن "image/rsvg.h"

تعريفنوع بنية rsvg_content {
	بنية content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ساكن nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = عدم;
	c->cs = عدم;
	c->ct = عدم;
	c->bitmap = عدم;

	إذا ((c->rsvgh = rsvg_handle_new()) == عدم) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
		إرجاع NSERROR_NOMEM;
	}

	إرجاع NSERROR_OK;
}


ساكن nserror rsvg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, حجم(rsvg_content));
	إذا (svg == عدم)
		إرجاع NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		إرجاع error;
	}

	error = rsvg_create_svg_data(svg);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		إرجاع error;
	}

	*c = (بنية content *) svg;

	إرجاع NSERROR_OK;
}


ساكن bool rsvg_process_data(بنية content *c, ثابت حرف *data,
			unsigned صحيح size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = عدم;

	إذا (rsvg_handle_write(d->rsvgh, (ثابت guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, عدم);
		إرجاع false;
	}

	إرجاع true;
}

ساكن bool rsvg_convert(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = عدم;

	إذا (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, عدم);
		إرجاع false;
	}

	أكد(err == عدم);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	إذا ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		إرجاع false;
	}

	إذا ((d->cs = cairo_image_surface_create_for_data(
			(unsigned حرف *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		إرجاع false;
	}

	إذا ((d->ct = cairo_create(d->cs)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, عدم);
		إرجاع false;
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

	إرجاع true;
}

ساكن bool rsvg_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	أكد(rsvgcontent->bitmap != عدم);

	إذا (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	إرجاع (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

ساكن فراغ rsvg_destroy(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	إذا (d->bitmap != عدم) guit->bitmap->destroy(d->bitmap);
	إذا (d->rsvgh != عدم) g_object_unref(d->rsvgh);
	إذا (d->ct != عدم) cairo_destroy(d->ct);
	إذا (d->cs != عدم) cairo_surface_destroy(d->cs);

	إرجاع;
}

ساكن nserror rsvg_clone(ثابت بنية content *old, بنية content **newc)
{
	rsvg_content *svg;
	nserror error;
	ثابت uint8_t *data;
	size_t size;

	svg = calloc(1, حجم(rsvg_content));
	إذا (svg == عدم)
		إرجاع NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->base);
		إرجاع error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->base);
		إرجاع error;
	}

	data = content__get_source_data(&svg->base, &size);
	إذا (size > 0) {
		إذا (rsvg_process_data(&svg->base, (ثابت حرف *)data, size) == false) {
			content_destroy(&svg->base);
			إرجاع NSERROR_NOMEM;
		}
	}

	إذا (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		إذا (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			إرجاع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) svg;

	إرجاع NSERROR_OK;
}

ساكن فراغ *rsvg_get_internal(ثابت بنية content *c, فراغ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	إرجاع d->bitmap;
}

ساكن content_type rsvg_content_type(فراغ)
{
	إرجاع CONTENT_IMAGE;
}


ساكن bool rsvg_content_is_opaque(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	إذا (d->bitmap != عدم) {
		إرجاع guit->bitmap->get_opaque(d->bitmap);
	}

	إرجاع false;
}


ساكن ثابت content_handler rsvg_content_handler = {
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

ساكن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

