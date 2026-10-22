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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#અન્તર્ભાવ <librsvg/rsvg-cairo.h>
#endif

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

#અન્તર્ભાવ "image/rsvg.h"

પ્રકારવ્યાખ્યા સંરચના rsvg_content {
	સંરચના content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	સંરચના bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

સ્થિર nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = ખાલી;
	c->cs = ખાલી;
	c->ct = ખાલી;
	c->bitmap = ખાલી;

	જો ((c->rsvgh = rsvg_handle_new()) == ખાલી) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
		પાછા NSERROR_NOMEM;
	}

	પાછા NSERROR_OK;
}


સ્થિર nserror rsvg_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
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

	error = rsvg_create_svg_data(svg);
	જો (error != NSERROR_OK) {
		મુક્ત(svg);
		પાછા error;
	}

	*c = (સંરચના content *) svg;

	પાછા NSERROR_OK;
}


સ્થિર bool rsvg_process_data(સંરચના content *c, અચલ અક્ષર *data,
			unsigned પૂર્ણાંક size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ખાલી;

	જો (rsvg_handle_write(d->rsvgh, (અચલ guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ખાલી);
		પાછા false;
	}

	પાછા true;
}

સ્થિર bool rsvg_convert(સંરચના content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = ખાલી;

	જો (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ખાલી);
		પાછા false;
	}

	પ્રતિજ્ઞા(err == ખાલી);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	જો ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
	}

	જો ((d->cs = cairo_image_surface_create_for_data(
			(unsigned અક્ષર *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
	}

	જો ((d->ct = cairo_create(d->cs)) == ખાલી) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ખાલી);
		પાછા false;
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

	પાછા true;
}

સ્થિર bool rsvg_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	પ્રતિજ્ઞા(rsvgcontent->bitmap != ખાલી);

	જો (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	જો (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	પાછા (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

સ્થિર શૂન્ય rsvg_destroy(સંરચના content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	જો (d->bitmap != ખાલી) guit->bitmap->destroy(d->bitmap);
	જો (d->rsvgh != ખાલી) g_object_unref(d->rsvgh);
	જો (d->ct != ખાલી) cairo_destroy(d->ct);
	જો (d->cs != ખાલી) cairo_surface_destroy(d->cs);

	પાછા;
}

સ્થિર nserror rsvg_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	rsvg_content *svg;
	nserror error;
	અચલ uint8_t *data;
	size_t size;

	svg = calloc(1, કદ(rsvg_content));
	જો (svg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	જો (error != NSERROR_OK) {
		content_destroy(&svg->base);
		પાછા error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	જો (error != NSERROR_OK) {
		content_destroy(&svg->base);
		પાછા error;
	}

	data = content__get_source_data(&svg->base, &size);
	જો (size > 0) {
		જો (rsvg_process_data(&svg->base, (અચલ અક્ષર *)data, size) == false) {
			content_destroy(&svg->base);
			પાછા NSERROR_NOMEM;
		}
	}

	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) svg;

	પાછા NSERROR_OK;
}

સ્થિર શૂન્ય *rsvg_get_internal(અચલ સંરચના content *c, શૂન્ય *context)
{
	rsvg_content *d = (rsvg_content *) c;

	પાછા d->bitmap;
}

સ્થિર content_type rsvg_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}


સ્થિર bool rsvg_content_is_opaque(સંરચના content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	જો (d->bitmap != ખાલી) {
		પાછા guit->bitmap->get_opaque(d->bitmap);
	}

	પાછા false;
}


સ્થિર અચલ content_handler rsvg_content_handler = {
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

સ્થિર અચલ અક્ષર *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

