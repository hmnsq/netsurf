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
#اگر_غیر_تعریف_شدہ RSVG_CAIRO_H
#شامل <librsvg/rsvg-cairo.h>
#اگر_ختم

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

قسمتعریف ساخت rsvg_content {
	ساخت content بنیاد;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ساخت bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ساکن nserror rsvg_create_svg_ڈیٹا(rsvg_content *c)
{
	c->rsvgh = تہی;
	c->cs = تہی;
	c->ct = تہی;
	c->bitmap = تہی;

	اگر ((c->rsvgh = rsvg_handle_new()) == تہی) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_نشر_error(&c->بنیاد, NSERROR_NOMEM, تہی);
		واپس NSERROR_NOMEM;
	}

	واپس NSERROR_OK;
}


ساکن nserror rsvg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساخت http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, ساخت content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = صفر_حافظہ_دو(1, حجم(rsvg_content));
	اگر (svg == تہی)
		واپس NSERROR_NOMEM;

	error = content__init(&svg->بنیاد, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		واپس error;
	}

	error = rsvg_create_svg_ڈیٹا(svg);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		واپس error;
	}

	*c = (ساخت content *) svg;

	واپس NSERROR_OK;
}


ساکن bool rsvg_process_ڈیٹا(ساخت content *c, ثابت حرف *ڈیٹا,
			غیرعلامتی صحیح سائز)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = تہی;

	اگر (rsvg_handle_write(d->rsvgh, (ثابت guchar *)ڈیٹا, (gsize)سائز,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->پیغام_2);
		content_نشر_error(c, NSERROR_SVG_ERROR, تہی);
		واپس false;
	}

	واپس true;
}

ساکن bool rsvg_convert(ساخت content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_ڈیٹا rsvgsize;
	GError *err = تہی;

	اگر (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->پیغام_2);
		content_نشر_error(c, NSERROR_SVG_ERROR, تہی);
		واپس false;
	}

	أكد(err == تہی);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->چوڑائی = rsvgsize.چوڑائی;
	c->اونچائی = rsvgsize.اونچائی;

	اگر ((d->bitmap = guit->bitmap->create(c->چوڑائی, c->اونچائی,
			BITMAP_NONE)) == تہی) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	}

	اگر ((d->cs = cairo_image_surface_create_for_ڈیٹا(
			(غیرعلامتی حرف *)guit->bitmap->get_بفر(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->چوڑائی, c->اونچائی,
			guit->bitmap->get_rowstride(d->bitmap))) == تہی) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	}

	اگر ((d->ct = cairo_create(d->cs)) == تہی) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, تہی);
		واپس false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_مقصد_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
	/* Done: update status bar */
	content_سیٹ_حالت(c, "");

	واپس true;
}

ساکن bool rsvg_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	أكد(rsvgcontent->bitmap != تہی);

	اگر (ڈیٹا->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	اگر (ڈیٹا->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	واپس (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  ڈیٹا->x, ڈیٹا->y,
				  ڈیٹا->چوڑائی, ڈیٹا->اونچائی,
				  ڈیٹا->background_رنگ,
				  flags) == NSERROR_OK);
}

ساکن خالی rsvg_destroy(ساخت content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->bitmap != تہی) guit->bitmap->destroy(d->bitmap);
	اگر (d->rsvgh != تہی) g_object_unref(d->rsvgh);
	اگر (d->ct != تہی) cairo_destroy(d->ct);
	اگر (d->cs != تہی) cairo_surface_destroy(d->cs);

	واپس;
}

ساکن nserror rsvg_clone(ثابت ساخت content *old, ساخت content **newc)
{
	rsvg_content *svg;
	nserror error;
	ثابت uint8_t *ڈیٹا;
	size_t سائز;

	svg = صفر_حافظہ_دو(1, حجم(rsvg_content));
	اگر (svg == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &svg->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->بنیاد);
		واپس error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_ڈیٹا(svg);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->بنیاد);
		واپس error;
	}

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(&svg->بنیاد, &سائز);
	اگر (سائز > 0) {
		اگر (rsvg_process_ڈیٹا(&svg->بنیاد, (ثابت حرف *)ڈیٹا, سائز) == false) {
			content_destroy(&svg->بنیاد);
			واپس NSERROR_NOMEM;
		}
	}

	اگر (old->حالت_2 == CONTENT_حالت_READY ||
			old->حالت_2 == CONTENT_حالت_مکمل) {
		اگر (rsvg_convert(&svg->بنیاد) == false) {
			content_destroy(&svg->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *) svg;

	واپس NSERROR_OK;
}

ساکن خالی *rsvg_get_اندرونی(ثابت ساخت content *c, خالی *context)
{
	rsvg_content *d = (rsvg_content *) c;

	واپس d->bitmap;
}

ساکن content_type rsvg_content_type(خالی)
{
	واپس CONTENT_IMAGE;
}


ساکن bool rsvg_content_is_opaque(ساخت content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->bitmap != تہی) {
		واپس guit->bitmap->get_opaque(d->bitmap);
	}

	واپس false;
}


ساکن ثابت content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_ڈیٹا = rsvg_process_ڈیٹا,
	.ڈیٹا_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_اندرونی = rsvg_get_اندرونی,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

