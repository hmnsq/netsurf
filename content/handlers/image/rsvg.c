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
#اگر_تعریف_نشده RSVG_CAIRO_H
#ضمن <librsvg/rsvg-cairo.h>
#پایان_اگر

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
	بنية content پایه;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ساكن nserror rsvg_create_svg_داده(rsvg_content *c)
{
	c->rsvgh = عدم;
	c->cs = عدم;
	c->ct = عدم;
	c->bitmap = عدم;

	إذا ((c->rsvgh = rsvg_handle_new()) == عدم) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_پخش_error(&c->پایه, NSERROR_NOMEM, عدم);
		أرجع NSERROR_NOMEM;
	}

	أرجع NSERROR_OK;
}


ساكن nserror rsvg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = حافظه_صفر_بده(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&svg->پایه, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	error = rsvg_create_svg_داده(svg);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	*c = (بنية content *) svg;

	أرجع NSERROR_OK;
}


ساكن bool rsvg_process_داده(بنية content *c, ثابت حرف *داده,
			بیعلامت صحيح اندازه)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = عدم;

	إذا (rsvg_handle_write(d->rsvgh, (ثابت guchar *)داده, (gsize)اندازه,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->پیام_2);
		content_پخش_error(c, NSERROR_SVG_ERROR, عدم);
		أرجع false;
	}

	أرجع true;
}

ساكن bool rsvg_convert(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_داده rsvgsize;
	GError *err = عدم;

	إذا (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->پیام_2);
		content_پخش_error(c, NSERROR_SVG_ERROR, عدم);
		أرجع false;
	}

	أكد(err == عدم);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->عرض = rsvgsize.عرض;
	c->ارتفاع = rsvgsize.ارتفاع;

	إذا ((d->bitmap = guit->bitmap->create(c->عرض, c->ارتفاع,
			BITMAP_NONE)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_پخش_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	إذا ((d->cs = cairo_image_surface_create_for_داده(
			(بیعلامت حرف *)guit->bitmap->get_میانگیر(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->عرض, c->ارتفاع,
			guit->bitmap->get_rowstride(d->bitmap))) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_پخش_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	إذا ((d->ct = cairo_create(d->cs)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_پخش_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_مقصد_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);
	/* Done: update status bar */
	content_مجموعه_وضعیت(c, "");

	أرجع true;
}

ساكن bool rsvg_redraw(بنية content *c, بنية content_redraw_داده *داده,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	أكد(rsvgcontent->bitmap != عدم);

	إذا (داده->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (داده->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  داده->x, داده->y,
				  داده->عرض, داده->ارتفاع,
				  داده->background_رنگ,
				  flags) == NSERROR_OK);
}

ساكن فراغ rsvg_destroy(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	إذا (d->bitmap != عدم) guit->bitmap->destroy(d->bitmap);
	إذا (d->rsvgh != عدم) g_object_unref(d->rsvgh);
	إذا (d->ct != عدم) cairo_destroy(d->ct);
	إذا (d->cs != عدم) cairo_surface_destroy(d->cs);

	أرجع;
}

ساكن nserror rsvg_clone(ثابت بنية content *old, بنية content **newc)
{
	rsvg_content *svg;
	nserror error;
	ثابت uint8_t *داده;
	size_t اندازه;

	svg = حافظه_صفر_بده(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &svg->پایه);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->پایه);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_داده(svg);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->پایه);
		أرجع error;
	}

	داده = content_get_منبع_داده_2(&svg->پایه, &اندازه);
	إذا (اندازه > 0) {
		إذا (rsvg_process_داده(&svg->پایه, (ثابت حرف *)داده, اندازه) == false) {
			content_destroy(&svg->پایه);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->وضعیت == CONTENT_وضعیت_READY ||
			old->وضعیت == CONTENT_وضعیت_انجامشده) {
		إذا (rsvg_convert(&svg->پایه) == false) {
			content_destroy(&svg->پایه);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) svg;

	أرجع NSERROR_OK;
}

ساكن فراغ *rsvg_get_داخلی(ثابت بنية content *c, فراغ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	أرجع d->bitmap;
}

ساكن content_type rsvg_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}


ساكن bool rsvg_content_is_opaque(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	إذا (d->bitmap != عدم) {
		أرجع guit->bitmap->get_opaque(d->bitmap);
	}

	أرجع false;
}


ساكن ثابت content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_داده = rsvg_process_داده,
	.داده_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_داخلی = rsvg_get_داخلی,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ثبات_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

