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

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#அந்தர்பாவ <librsvg/rsvg-cairo.h>
#endif

#அந்தர்பாவ <nsutils/endian.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "content/llcache.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/content_factory.h"
#அந்தர்பாவ "desktop/gui_internal.h"
#அந்தர்பாவ "desktop/bitmap.h"

#அந்தர்பாவ "image/rsvg.h"

வகைவரை கட்டமைப்பு rsvg_content {
	கட்டமைப்பு content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	கட்டமைப்பு bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

நிலையான nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = வெற்று;
	c->cs = வெற்று;
	c->ct = வெற்று;
	c->bitmap = வெற்று;

	எனில் ((c->rsvgh = rsvg_handle_new()) == வெற்று) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, வெற்று);
		திரும்பு NSERROR_NOMEM;
	}

	திரும்பு NSERROR_OK;
}


நிலையான nserror rsvg_create(மாறிலி content_handler *handler,
		lwc_string *imime_type, மாறிலி கட்டமைப்பு http_parameter *params,
		llcache_handle *llcache, மாறிலி எழுத்து *fallback_charset,
		bool quirks, கட்டமைப்பு content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, அளவு(rsvg_content));
	எனில் (svg == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	எனில் (error != NSERROR_OK) {
		விடுவி(svg);
		திரும்பு error;
	}

	error = rsvg_create_svg_data(svg);
	எனில் (error != NSERROR_OK) {
		விடுவி(svg);
		திரும்பு error;
	}

	*c = (கட்டமைப்பு content *) svg;

	திரும்பு NSERROR_OK;
}


நிலையான bool rsvg_process_data(கட்டமைப்பு content *c, மாறிலி எழுத்து *data,
			unsigned முழு size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = வெற்று;

	எனில் (rsvg_handle_write(d->rsvgh, (மாறிலி guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, வெற்று);
		திரும்பு false;
	}

	திரும்பு true;
}

நிலையான bool rsvg_convert(கட்டமைப்பு content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = வெற்று;

	எனில் (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, வெற்று);
		திரும்பு false;
	}

	ப்ரதிஜ்ஞா(err == வெற்று);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	எனில் ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
	}

	எனில் ((d->cs = cairo_image_surface_create_for_data(
			(unsigned எழுத்து *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
	}

	எனில் ((d->ct = cairo_create(d->cs)) == வெற்று) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, வெற்று);
		திரும்பு false;
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

	திரும்பு true;
}

நிலையான bool rsvg_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ப்ரதிஜ்ஞா(rsvgcontent->bitmap != வெற்று);

	எனில் (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	எனில் (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	திரும்பு (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

நிலையான வெறுமை rsvg_destroy(கட்டமைப்பு content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	எனில் (d->bitmap != வெற்று) guit->bitmap->destroy(d->bitmap);
	எனில் (d->rsvgh != வெற்று) g_object_unref(d->rsvgh);
	எனில் (d->ct != வெற்று) cairo_destroy(d->ct);
	எனில் (d->cs != வெற்று) cairo_surface_destroy(d->cs);

	திரும்பு;
}

நிலையான nserror rsvg_clone(மாறிலி கட்டமைப்பு content *old, கட்டமைப்பு content **newc)
{
	rsvg_content *svg;
	nserror error;
	மாறிலி uint8_t *data;
	size_t size;

	svg = calloc(1, அளவு(rsvg_content));
	எனில் (svg == வெற்று)
		திரும்பு NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	எனில் (error != NSERROR_OK) {
		content_destroy(&svg->base);
		திரும்பு error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	எனில் (error != NSERROR_OK) {
		content_destroy(&svg->base);
		திரும்பு error;
	}

	data = content__get_source_data(&svg->base, &size);
	எனில் (size > 0) {
		எனில் (rsvg_process_data(&svg->base, (மாறிலி எழுத்து *)data, size) == false) {
			content_destroy(&svg->base);
			திரும்பு NSERROR_NOMEM;
		}
	}

	எனில் (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		எனில் (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			திரும்பு NSERROR_CLONE_FAILED;
		}
	}

	*newc = (கட்டமைப்பு content *) svg;

	திரும்பு NSERROR_OK;
}

நிலையான வெறுமை *rsvg_get_internal(மாறிலி கட்டமைப்பு content *c, வெறுமை *context)
{
	rsvg_content *d = (rsvg_content *) c;

	திரும்பு d->bitmap;
}

நிலையான content_type rsvg_content_type(வெறுமை)
{
	திரும்பு CONTENT_IMAGE;
}


நிலையான bool rsvg_content_is_opaque(கட்டமைப்பு content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	எனில் (d->bitmap != வெற்று) {
		திரும்பு guit->bitmap->get_opaque(d->bitmap);
	}

	திரும்பு false;
}


நிலையான மாறிலி content_handler rsvg_content_handler = {
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

நிலையான மாறிலி எழுத்து *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

