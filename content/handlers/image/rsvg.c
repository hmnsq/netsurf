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

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#অন্তর্ভাব <librsvg/rsvg-cairo.h>
#endif

#অন্তর্ভাব <nsutils/endian.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "content/llcache.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/content_factory.h"
#অন্তর্ভাব "desktop/gui_internal.h"
#অন্তর্ভাব "desktop/bitmap.h"

#অন্তর্ভাব "image/rsvg.h"

প্রকারনির্দেশ সংরচনা rsvg_content {
	সংরচনা content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	সংরচনা bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

স্থির nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = রিক্ত;
	c->cs = রিক্ত;
	c->ct = রিক্ত;
	c->bitmap = রিক্ত;

	যদি ((c->rsvgh = rsvg_handle_new()) == রিক্ত) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম NSERROR_NOMEM;
	}

	প্রত্যাগম NSERROR_OK;
}


স্থির nserror rsvg_create(ধ্রুব content_handler *handler,
		lwc_string *imime_type, ধ্রুব সংরচনা http_parameter *params,
		llcache_handle *llcache, ধ্রুব বর্ণ *fallback_charset,
		bool quirks, সংরচনা content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, পরিমাণ(rsvg_content));
	যদি (svg == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(svg);
		প্রত্যাগম error;
	}

	error = rsvg_create_svg_data(svg);
	যদি (error != NSERROR_OK) {
		মুক্ত(svg);
		প্রত্যাগম error;
	}

	*c = (সংরচনা content *) svg;

	প্রত্যাগম NSERROR_OK;
}


স্থির bool rsvg_process_data(সংরচনা content *c, ধ্রুব বর্ণ *data,
			unsigned পূর্ণাঙ্ক size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = রিক্ত;

	যদি (rsvg_handle_write(d->rsvgh, (ধ্রুব guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, রিক্ত);
		প্রত্যাগম false;
	}

	প্রত্যাগম true;
}

স্থির bool rsvg_convert(সংরচনা content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = রিক্ত;

	যদি (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, রিক্ত);
		প্রত্যাগম false;
	}

	প্রতিজ্ঞা(err == রিক্ত);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	যদি ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম false;
	}

	যদি ((d->cs = cairo_image_surface_create_for_data(
			(unsigned বর্ণ *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম false;
	}

	যদি ((d->ct = cairo_create(d->cs)) == রিক্ত) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, রিক্ত);
		প্রত্যাগম false;
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

	প্রত্যাগম true;
}

স্থির bool rsvg_redraw(সংরচনা content *c, সংরচনা content_redraw_data *data,
		ধ্রুব সংরচনা rect *clip, ধ্রুব সংরচনা redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	প্রতিজ্ঞা(rsvgcontent->bitmap != রিক্ত);

	যদি (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	যদি (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	প্রত্যাগম (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

স্থির শূন্য rsvg_destroy(সংরচনা content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	যদি (d->bitmap != রিক্ত) guit->bitmap->destroy(d->bitmap);
	যদি (d->rsvgh != রিক্ত) g_object_unref(d->rsvgh);
	যদি (d->ct != রিক্ত) cairo_destroy(d->ct);
	যদি (d->cs != রিক্ত) cairo_surface_destroy(d->cs);

	প্রত্যাগম;
}

স্থির nserror rsvg_clone(ধ্রুব সংরচনা content *old, সংরচনা content **newc)
{
	rsvg_content *svg;
	nserror error;
	ধ্রুব uint8_t *data;
	size_t size;

	svg = calloc(1, পরিমাণ(rsvg_content));
	যদি (svg == রিক্ত)
		প্রত্যাগম NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&svg->base);
		প্রত্যাগম error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	যদি (error != NSERROR_OK) {
		content_destroy(&svg->base);
		প্রত্যাগম error;
	}

	data = content__get_source_data(&svg->base, &size);
	যদি (size > 0) {
		যদি (rsvg_process_data(&svg->base, (ধ্রুব বর্ণ *)data, size) == false) {
			content_destroy(&svg->base);
			প্রত্যাগম NSERROR_NOMEM;
		}
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			প্রত্যাগম NSERROR_CLONE_FAILED;
		}
	}

	*newc = (সংরচনা content *) svg;

	প্রত্যাগম NSERROR_OK;
}

স্থির শূন্য *rsvg_get_internal(ধ্রুব সংরচনা content *c, শূন্য *context)
{
	rsvg_content *d = (rsvg_content *) c;

	প্রত্যাগম d->bitmap;
}

স্থির content_type rsvg_content_type(শূন্য)
{
	প্রত্যাগম CONTENT_IMAGE;
}


স্থির bool rsvg_content_is_opaque(সংরচনা content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	যদি (d->bitmap != রিক্ত) {
		প্রত্যাগম guit->bitmap->get_opaque(d->bitmap);
	}

	প্রত্যাগম false;
}


স্থির ধ্রুব content_handler rsvg_content_handler = {
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

স্থির ধ্রুব বর্ণ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

