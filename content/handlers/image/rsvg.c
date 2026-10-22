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

#شامل <stdlib.h>
#شامل <stdbool.h>
#شامل <assert.h>
#شامل <string.h>
#شامل <sys/types.h>

#شامل <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#شامل <librsvg/rsvg-cairo.h>
#endif

#شامل <nsutils/endian.h>

#شامل "utils/log.h"
#شامل "utils/utils.h"
#شامل "utils/messages.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/content.h"
#شامل "content/llcache.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"
#شامل "desktop/gui_internal.h"
#شامل "desktop/bitmap.h"

#شامل "image/rsvg.h"

تعریفنوع ساختار rsvg_content {
	ساختار content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ساختار bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ایستا nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = پوچ;
	c->cs = پوچ;
	c->ct = پوچ;
	c->bitmap = پوچ;

	اگر ((c->rsvgh = rsvg_handle_new()) == پوچ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
		بازگشت NSERROR_NOMEM;
	}

	بازگشت NSERROR_OK;
}


ایستا nserror rsvg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, اندازه(rsvg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		بازگشت error;
	}

	error = rsvg_create_svg_data(svg);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		بازگشت error;
	}

	*c = (ساختار content *) svg;

	بازگشت NSERROR_OK;
}


ایستا bool rsvg_process_data(ساختار content *c, ثابت نویسه *data,
			unsigned صحیح size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = پوچ;

	اگر (rsvg_handle_write(d->rsvgh, (ثابت guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, پوچ);
		بازگشت false;
	}

	بازگشت true;
}

ایستا bool rsvg_convert(ساختار content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = پوچ;

	اگر (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, پوچ);
		بازگشت false;
	}

	أكد(err == پوچ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	اگر ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == پوچ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, پوچ);
		بازگشت false;
	}

	اگر ((d->cs = cairo_image_surface_create_for_data(
			(unsigned نویسه *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == پوچ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, پوچ);
		بازگشت false;
	}

	اگر ((d->ct = cairo_create(d->cs)) == پوچ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, پوچ);
		بازگشت false;
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

	بازگشت true;
}

ایستا bool rsvg_redraw(ساختار content *c, ساختار content_redraw_data *data,
		ثابت ساختار rect *clip, ثابت ساختار redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	أكد(rsvgcontent->bitmap != پوچ);

	اگر (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	بازگشت (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

ایستا تهی rsvg_destroy(ساختار content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->bitmap != پوچ) guit->bitmap->destroy(d->bitmap);
	اگر (d->rsvgh != پوچ) g_object_unref(d->rsvgh);
	اگر (d->ct != پوچ) cairo_destroy(d->ct);
	اگر (d->cs != پوچ) cairo_surface_destroy(d->cs);

	بازگشت;
}

ایستا nserror rsvg_clone(ثابت ساختار content *old, ساختار content **newc)
{
	rsvg_content *svg;
	nserror error;
	ثابت uint8_t *data;
	size_t size;

	svg = calloc(1, اندازه(rsvg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->base);
		بازگشت error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->base);
		بازگشت error;
	}

	data = content__get_source_data(&svg->base, &size);
	اگر (size > 0) {
		اگر (rsvg_process_data(&svg->base, (ثابت نویسه *)data, size) == false) {
			content_destroy(&svg->base);
			بازگشت NSERROR_NOMEM;
		}
	}

	اگر (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		اگر (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *) svg;

	بازگشت NSERROR_OK;
}

ایستا تهی *rsvg_get_internal(ثابت ساختار content *c, تهی *context)
{
	rsvg_content *d = (rsvg_content *) c;

	بازگشت d->bitmap;
}

ایستا content_type rsvg_content_type(تهی)
{
	بازگشت CONTENT_IMAGE;
}


ایستا bool rsvg_content_is_opaque(ساختار content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->bitmap != پوچ) {
		بازگشت guit->bitmap->get_opaque(d->bitmap);
	}

	بازگشت false;
}


ایستا ثابت content_handler rsvg_content_handler = {
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

ایستا ثابت نویسه *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

