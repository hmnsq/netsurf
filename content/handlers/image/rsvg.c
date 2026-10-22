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

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#ଅନ୍ତର୍ଭାଵ <librsvg/rsvg-cairo.h>
#endif

#ଅନ୍ତର୍ଭାଵ <nsutils/endian.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/llcache.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"
#ଅନ୍ତର୍ଭାଵ "desktop/bitmap.h"

#ଅନ୍ତର୍ଭାଵ "image/rsvg.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା rsvg_content {
	ସଂରଚନା content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ସଂରଚନା bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ସ୍ଥିର nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = ଖାଲି;
	c->cs = ଖାଲି;
	c->ct = ଖାଲି;
	c->bitmap = ଖାଲି;

	ଯଦି ((c->rsvgh = rsvg_handle_new()) == ଖାଲି) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ NSERROR_NOMEM;
	}

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର nserror rsvg_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, ଆକାର(rsvg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(svg);
		ଫେରନ୍ତୁ error;
	}

	error = rsvg_create_svg_data(svg);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(svg);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}


ସ୍ଥିର bool rsvg_process_data(ସଂରଚନା content *c, ଅଚଳ ଅକ୍ଷର *data,
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ଖାଲି;

	ଯଦି (rsvg_handle_write(d->rsvgh, (ଅଚଳ guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool rsvg_convert(ସଂରଚନା content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = ଖାଲି;

	ଯଦି (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ପ୍ରତିଜ୍ଞା(err == ଖାଲି);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	ଯଦି ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଯଦି ((d->cs = cairo_image_surface_create_for_data(
			(unsigned ଅକ୍ଷର *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
	}

	ଯଦି ((d->ct = cairo_create(d->cs)) == ଖାଲି) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ଖାଲି);
		ଫେରନ୍ତୁ false;
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

	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର bool rsvg_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ପ୍ରତିଜ୍ଞା(rsvgcontent->bitmap != ଖାଲି);

	ଯଦି (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ଯଦି (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ଫେରନ୍ତୁ (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

ସ୍ଥିର ଶୂନ୍ୟ rsvg_destroy(ସଂରଚନା content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ଯଦି (d->bitmap != ଖାଲି) guit->bitmap->destroy(d->bitmap);
	ଯଦି (d->rsvgh != ଖାଲି) g_object_unref(d->rsvgh);
	ଯଦି (d->ct != ଖାଲି) cairo_destroy(d->ct);
	ଯଦି (d->cs != ଖାଲି) cairo_surface_destroy(d->cs);

	ଫେରନ୍ତୁ;
}

ସ୍ଥିର nserror rsvg_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	rsvg_content *svg;
	nserror error;
	ଅଚଳ uint8_t *data;
	size_t size;

	svg = calloc(1, ଆକାର(rsvg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ଫେରନ୍ତୁ error;
	}

	data = content__get_source_data(&svg->base, &size);
	ଯଦି (size > 0) {
		ଯଦି (rsvg_process_data(&svg->base, (ଅଚଳ ଅକ୍ଷର *)data, size) == false) {
			content_destroy(&svg->base);
			ଫେରନ୍ତୁ NSERROR_NOMEM;
		}
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ *rsvg_get_internal(ଅଚଳ ସଂରଚନା content *c, ଶୂନ୍ୟ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ଫେରନ୍ତୁ d->bitmap;
}

ସ୍ଥିର content_type rsvg_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}


ସ୍ଥିର bool rsvg_content_is_opaque(ସଂରଚନା content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ଯଦି (d->bitmap != ଖାଲି) {
		ଫେରନ୍ତୁ guit->bitmap->get_opaque(d->bitmap);
	}

	ଫେରନ୍ତୁ false;
}


ସ୍ଥିର ଅଚଳ content_handler rsvg_content_handler = {
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

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

