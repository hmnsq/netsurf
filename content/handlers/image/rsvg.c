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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#അന്തര്ഭാവ <librsvg/rsvg-cairo.h>
#endif

#അന്തര്ഭാവ <nsutils/endian.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "content/llcache.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/content_factory.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"
#അന്തര്ഭാവ "desktop/bitmap.h"

#അന്തര്ഭാവ "image/rsvg.h"

തരനിർവചനം ഘടന rsvg_content {
	ഘടന content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ഘടന bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

സ്ഥിതം nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = ശൂന്യ;
	c->cs = ശൂന്യ;
	c->ct = ശൂന്യ;
	c->bitmap = ശൂന്യ;

	എങ്കിൽ ((c->rsvgh = rsvg_handle_new()) == ശൂന്യ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ NSERROR_NOMEM;
	}

	തിരികെ NSERROR_OK;
}


സ്ഥിതം nserror rsvg_create(സ്ഥിരം content_handler *handler,
		lwc_string *imime_type, സ്ഥിരം ഘടന http_parameter *params,
		llcache_handle *llcache, സ്ഥിരം അക്ഷരം *fallback_charset,
		bool quirks, ഘടന content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, വലുപ്പം(rsvg_content));
	എങ്കിൽ (svg == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(svg);
		തിരികെ error;
	}

	error = rsvg_create_svg_data(svg);
	എങ്കിൽ (error != NSERROR_OK) {
		സ്വതന്ത്രം(svg);
		തിരികെ error;
	}

	*c = (ഘടന content *) svg;

	തിരികെ NSERROR_OK;
}


സ്ഥിതം bool rsvg_process_data(ഘടന content *c, സ്ഥിരം അക്ഷരം *data,
			unsigned പൂർണ്ണസംഖ്യ size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ശൂന്യ;

	എങ്കിൽ (rsvg_handle_write(d->rsvgh, (സ്ഥിരം guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ശൂന്യ);
		തിരികെ false;
	}

	തിരികെ true;
}

സ്ഥിതം bool rsvg_convert(ഘടന content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = ശൂന്യ;

	എങ്കിൽ (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ശൂന്യ);
		തിരികെ false;
	}

	പ്രതിജ്ഞാ(err == ശൂന്യ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	എങ്കിൽ ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
	}

	എങ്കിൽ ((d->cs = cairo_image_surface_create_for_data(
			(unsigned അക്ഷരം *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
	}

	എങ്കിൽ ((d->ct = cairo_create(d->cs)) == ശൂന്യ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ശൂന്യ);
		തിരികെ false;
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

	തിരികെ true;
}

സ്ഥിതം bool rsvg_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	പ്രതിജ്ഞാ(rsvgcontent->bitmap != ശൂന്യ);

	എങ്കിൽ (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	എങ്കിൽ (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	തിരികെ (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

സ്ഥിതം ശൂന്യം rsvg_destroy(ഘടന content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	എങ്കിൽ (d->bitmap != ശൂന്യ) guit->bitmap->destroy(d->bitmap);
	എങ്കിൽ (d->rsvgh != ശൂന്യ) g_object_unref(d->rsvgh);
	എങ്കിൽ (d->ct != ശൂന്യ) cairo_destroy(d->ct);
	എങ്കിൽ (d->cs != ശൂന്യ) cairo_surface_destroy(d->cs);

	തിരികെ;
}

സ്ഥിതം nserror rsvg_clone(സ്ഥിരം ഘടന content *old, ഘടന content **newc)
{
	rsvg_content *svg;
	nserror error;
	സ്ഥിരം uint8_t *data;
	size_t size;

	svg = calloc(1, വലുപ്പം(rsvg_content));
	എങ്കിൽ (svg == ശൂന്യ)
		തിരികെ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		തിരികെ error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	എങ്കിൽ (error != NSERROR_OK) {
		content_destroy(&svg->base);
		തിരികെ error;
	}

	data = content__get_source_data(&svg->base, &size);
	എങ്കിൽ (size > 0) {
		എങ്കിൽ (rsvg_process_data(&svg->base, (സ്ഥിരം അക്ഷരം *)data, size) == false) {
			content_destroy(&svg->base);
			തിരികെ NSERROR_NOMEM;
		}
	}

	എങ്കിൽ (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		എങ്കിൽ (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			തിരികെ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ഘടന content *) svg;

	തിരികെ NSERROR_OK;
}

സ്ഥിതം ശൂന്യം *rsvg_get_internal(സ്ഥിരം ഘടന content *c, ശൂന്യം *context)
{
	rsvg_content *d = (rsvg_content *) c;

	തിരികെ d->bitmap;
}

സ്ഥിതം content_type rsvg_content_type(ശൂന്യം)
{
	തിരികെ CONTENT_IMAGE;
}


സ്ഥിതം bool rsvg_content_is_opaque(ഘടന content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	എങ്കിൽ (d->bitmap != ശൂന്യ) {
		തിരികെ guit->bitmap->get_opaque(d->bitmap);
	}

	തിരികെ false;
}


സ്ഥിതം സ്ഥിരം content_handler rsvg_content_handler = {
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

സ്ഥിതം സ്ഥിരം അക്ഷരം *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

