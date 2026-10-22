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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <librsvg/rsvg.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RSVG_CAIRO_H
#ਅਨ੍ਤਰ੍ਭਾਵ <librsvg/rsvg-cairo.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <nsutils/endian.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/llcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/bitmap.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/rsvg.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rsvg_content {
	ਬਣਤਰ content ਅਧਾਰ;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ਬਣਤਰ bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ਸਥਿਰ nserror rsvg_create_svg_ਅੰਕੜੇ(rsvg_content *c)
{
	c->rsvgh = ਨਿਰਾ;
	c->cs = ਨਿਰਾ;
	c->ct = ਨਿਰਾ;
	c->bitmap = ਨਿਰਾ;

	ਜੇ ((c->rsvgh = rsvg_handle_new()) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_ਪ੍ਰਸਾਰਣ_error(&c->ਅਧਾਰ, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ NSERROR_NOMEM;
	}

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ nserror rsvg_create(ਅਚਲ content_handler *handler,
		lwc_string *imime_type, ਅਚਲ ਬਣਤਰ http_parameter *params,
		llcache_handle *llcache, ਅਚਲ ਅੱਖਰ *fallback_charset,
		bool quirks, ਬਣਤਰ content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(rsvg_content));
	ਜੇ (svg == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__init(&svg->ਅਧਾਰ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(svg);
		ਵਾਪਸ error;
	}

	error = rsvg_create_svg_ਅੰਕੜੇ(svg);
	ਜੇ (error != NSERROR_OK) {
		ਮੁਕਤ(svg);
		ਵਾਪਸ error;
	}

	*c = (ਬਣਤਰ content *) svg;

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ bool rsvg_process_ਅੰਕੜੇ(ਬਣਤਰ content *c, ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ,
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਮਾਪ)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ਨਿਰਾ;

	ਜੇ (rsvg_handle_write(d->rsvgh, (ਅਚਲ guchar *)ਅੰਕੜੇ, (gsize)ਮਾਪ,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->ਸੁਨੇਹਾ_2);
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_SVG_ERROR, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}

ਸਥਿਰ bool rsvg_convert(ਬਣਤਰ content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_ਅੰਕੜੇ rsvgsize;
	GError *err = ਨਿਰਾ;

	ਜੇ (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->ਸੁਨੇਹਾ_2);
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_SVG_ERROR, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(err == ਨਿਰਾ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->ਚੌੜਾਈ = rsvgsize.ਚੌੜਾਈ;
	c->ਉਚਾਈ = rsvgsize.ਉਚਾਈ;

	ਜੇ ((d->bitmap = guit->bitmap->create(c->ਚੌੜਾਈ, c->ਉਚਾਈ,
			BITMAP_NONE)) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਜੇ ((d->cs = cairo_image_surface_create_for_ਅੰਕੜੇ(
			(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)guit->bitmap->get_ਬਫਰ(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->ਚੌੜਾਈ, c->ਉਚਾਈ,
			guit->bitmap->get_rowstride(d->bitmap))) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	ਜੇ ((d->ct = cairo_create(d->cs)) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_ਪ੍ਰਸਾਰਣ_error(c, NSERROR_NOMEM, ਨਿਰਾ);
		ਵਾਪਸ false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_ਮੰਜ਼ਿਲ_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
	/* Done: update status bar */
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, "");

	ਵਾਪਸ true;
}

ਸਥਿਰ bool rsvg_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ਪ੍ਰਤਿਜ੍ਞਾ(rsvgcontent->bitmap != ਨਿਰਾ);

	ਜੇ (ਅੰਕੜੇ->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ਜੇ (ਅੰਕੜੇ->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ਵਾਪਸ (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  ਅੰਕੜੇ->x, ਅੰਕੜੇ->y,
				  ਅੰਕੜੇ->ਚੌੜਾਈ, ਅੰਕੜੇ->ਉਚਾਈ,
				  ਅੰਕੜੇ->background_ਰੰਗ,
				  flags) == NSERROR_OK);
}

ਸਥਿਰ ਖਾਲੀ rsvg_destroy(ਬਣਤਰ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ਜੇ (d->bitmap != ਨਿਰਾ) guit->bitmap->destroy(d->bitmap);
	ਜੇ (d->rsvgh != ਨਿਰਾ) g_object_unref(d->rsvgh);
	ਜੇ (d->ct != ਨਿਰਾ) cairo_destroy(d->ct);
	ਜੇ (d->cs != ਨਿਰਾ) cairo_surface_destroy(d->cs);

	ਵਾਪਸ;
}

ਸਥਿਰ nserror rsvg_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	rsvg_content *svg;
	nserror error;
	ਅਚਲ uint8_t *ਅੰਕੜੇ;
	size_t ਮਾਪ;

	svg = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(rsvg_content));
	ਜੇ (svg == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &svg->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&svg->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_ਅੰਕੜੇ(svg);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&svg->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(&svg->ਅਧਾਰ, &ਮਾਪ);
	ਜੇ (ਮਾਪ > 0) {
		ਜੇ (rsvg_process_ਅੰਕੜੇ(&svg->ਅਧਾਰ, (ਅਚਲ ਅੱਖਰ *)ਅੰਕੜੇ, ਮਾਪ) == false) {
			content_destroy(&svg->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_NOMEM;
		}
	}

	ਜੇ (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY ||
			old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ) {
		ਜੇ (rsvg_convert(&svg->ਅਧਾਰ) == false) {
			content_destroy(&svg->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *) svg;

	ਵਾਪਸ NSERROR_OK;
}

ਸਥਿਰ ਖਾਲੀ *rsvg_get_ਅੰਦਰੂਨੀ(ਅਚਲ ਬਣਤਰ content *c, ਖਾਲੀ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ਵਾਪਸ d->bitmap;
}

ਸਥਿਰ content_type rsvg_content_type(ਖਾਲੀ)
{
	ਵਾਪਸ CONTENT_IMAGE;
}


ਸਥਿਰ bool rsvg_content_is_opaque(ਬਣਤਰ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ਜੇ (d->bitmap != ਨਿਰਾ) {
		ਵਾਪਸ guit->bitmap->get_opaque(d->bitmap);
	}

	ਵਾਪਸ false;
}


ਸਥਿਰ ਅਚਲ content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_ਅੰਕੜੇ = rsvg_process_ਅੰਕੜੇ,
	.ਅੰਕੜੇ_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_ਅੰਦਰੂਨੀ = rsvg_get_ਅੰਦਰੂਨੀ,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

