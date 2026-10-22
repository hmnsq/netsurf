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
#جے_بنا_تعریف RSVG_CAIRO_H
#ضمن <librsvg/rsvg-cairo.h>
#جے_انت

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
	بنية content ادھار;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	بنية bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ساكن nserror rsvg_create_svg_انکڑے(rsvg_content *c)
{
	c->rsvgh = عدم;
	c->cs = عدم;
	c->ct = عدم;
	c->bitmap = عدم;

	إذا ((c->rsvgh = rsvg_handle_new()) == عدم) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_نشر_error(&c->ادھار, NSERROR_NOMEM, عدم);
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

	svg = صفر_حافظہ_دیو(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&svg->ادھار, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	error = rsvg_create_svg_انکڑے(svg);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	*c = (بنية content *) svg;

	أرجع NSERROR_OK;
}


ساكن bool rsvg_process_انکڑے(بنية content *c, ثابت حرف *انکڑے,
			بنانشانی صحيح ناپ)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = عدم;

	إذا (rsvg_handle_write(d->rsvgh, (ثابت guchar *)انکڑے, (gsize)ناپ,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->سنیہا);
		content_نشر_error(c, NSERROR_SVG_ERROR, عدم);
		أرجع false;
	}

	أرجع true;
}

ساكن bool rsvg_convert(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_انکڑے rsvgsize;
	GError *err = عدم;

	إذا (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->سنیہا);
		content_نشر_error(c, NSERROR_SVG_ERROR, عدم);
		أرجع false;
	}

	أكد(err == عدم);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->چوڑائی = rsvgsize.چوڑائی;
	c->اچائی = rsvgsize.اچائی;

	إذا ((d->bitmap = guit->bitmap->create(c->چوڑائی, c->اچائی,
			BITMAP_NONE)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	إذا ((d->cs = cairo_image_surface_create_for_انکڑے(
			(بنانشانی حرف *)guit->bitmap->get_بفر(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->چوڑائی, c->اچائی,
			guit->bitmap->get_rowstride(d->bitmap))) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	إذا ((d->ct = cairo_create(d->cs)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_نشر_error(c, NSERROR_NOMEM, عدم);
		أرجع false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_منزل_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_سیٹ_ready(c);
	content_سیٹ_پورا(c);
	/* Done: update status bar */
	content_سیٹ_حالت(c, "");

	أرجع true;
}

ساكن bool rsvg_redraw(بنية content *c, بنية content_redraw_انکڑے *انکڑے,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	أكد(rsvgcontent->bitmap != عدم);

	إذا (انکڑے->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	إذا (انکڑے->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	أرجع (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  انکڑے->x, انکڑے->y,
				  انکڑے->چوڑائی, انکڑے->اچائی,
				  انکڑے->background_رنگ,
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
	ثابت uint8_t *انکڑے;
	size_t ناپ;

	svg = صفر_حافظہ_دیو(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &svg->ادھار);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->ادھار);
		أرجع error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_انکڑے(svg);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->ادھار);
		أرجع error;
	}

	انکڑے = content_get_سروت_انکڑے_2(&svg->ادھار, &ناپ);
	إذا (ناپ > 0) {
		إذا (rsvg_process_انکڑے(&svg->ادھار, (ثابت حرف *)انکڑے, ناپ) == false) {
			content_destroy(&svg->ادھار);
			أرجع NSERROR_NOMEM;
		}
	}

	إذا (old->حالت == CONTENT_حالت_READY ||
			old->حالت == CONTENT_حالت_پورا) {
		إذا (rsvg_convert(&svg->ادھار) == false) {
			content_destroy(&svg->ادھار);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) svg;

	أرجع NSERROR_OK;
}

ساكن فراغ *rsvg_get_اندرونی(ثابت بنية content *c, فراغ *context)
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
	.process_انکڑے = rsvg_process_انکڑے,
	.انکڑے_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_اندرونی = rsvg_get_اندرونی,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

