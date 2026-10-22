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

প্রকারলেখ কাঠামো rsvg_content {
	কাঠামো content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	কাঠামো bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

স্থির nserror rsvg_create_svg_তথ্য(rsvg_content *c)
{
	c->rsvgh = ফাঁকা;
	c->cs = ফাঁকা;
	c->ct = ফাঁকা;
	c->bitmap = ফাঁকা;

	যদি ((c->rsvgh = rsvg_handle_new()) == ফাঁকা) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, ফাঁকা);
		ফেরত NSERROR_NOMEM;
	}

	ফেরত NSERROR_OK;
}


স্থির nserror rsvg_create(ধ্রুবক content_handler *handler,
		lwc_string *imime_type, ধ্রুবক কাঠামো http_parameter *params,
		llcache_handle *llcache, ধ্রুবক অক্ষর *fallback_charset,
		bool quirks, কাঠামো content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = calloc(1, আকার(rsvg_content));
	যদি (svg == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	যদি (error != NSERROR_OK) {
		মুক্ত(svg);
		ফেরত error;
	}

	error = rsvg_create_svg_তথ্য(svg);
	যদি (error != NSERROR_OK) {
		মুক্ত(svg);
		ফেরত error;
	}

	*c = (কাঠামো content *) svg;

	ফেরত NSERROR_OK;
}


স্থির bool rsvg_process_তথ্য(কাঠামো content *c, ধ্রুবক অক্ষর *তথ্য,
			unsigned পূর্ণসংখ্যা আকার_2)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ফাঁকা;

	যদি (rsvg_handle_write(d->rsvgh, (ধ্রুবক guchar *)তথ্য, (gsize)আকার_2,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ফাঁকা);
		ফেরত false;
	}

	ফেরত true;
}

স্থির bool rsvg_convert(কাঠামো content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_তথ্য rsvgsize;
	GError *err = ফাঁকা;

	যদি (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, ফাঁকা);
		ফেরত false;
	}

	প্রতিজ্ঞা(err == ফাঁকা);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	যদি ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
	}

	যদি ((d->cs = cairo_image_surface_create_for_তথ্য(
			(unsigned অক্ষর *)guit->bitmap->get_বাফার(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
	}

	যদি ((d->ct = cairo_create(d->cs)) == ফাঁকা) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, ফাঁকা);
		ফেরত false;
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

	ফেরত true;
}

স্থির bool rsvg_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	প্রতিজ্ঞা(rsvgcontent->bitmap != ফাঁকা);

	যদি (তথ্য->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	যদি (তথ্য->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ফেরত (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  তথ্য->x, তথ্য->y,
				  তথ্য->width, তথ্য->height,
				  তথ্য->background_colour,
				  flags) == NSERROR_OK);
}

স্থির শূন্য rsvg_destroy(কাঠামো content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	যদি (d->bitmap != ফাঁকা) guit->bitmap->destroy(d->bitmap);
	যদি (d->rsvgh != ফাঁকা) g_object_unref(d->rsvgh);
	যদি (d->ct != ফাঁকা) cairo_destroy(d->ct);
	যদি (d->cs != ফাঁকা) cairo_surface_destroy(d->cs);

	ফেরত;
}

স্থির nserror rsvg_clone(ধ্রুবক কাঠামো content *old, কাঠামো content **newc)
{
	rsvg_content *svg;
	nserror error;
	ধ্রুবক uint8_t *তথ্য;
	size_t আকার_2;

	svg = calloc(1, আকার(rsvg_content));
	যদি (svg == ফাঁকা)
		ফেরত NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	যদি (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ফেরত error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_তথ্য(svg);
	যদি (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ফেরত error;
	}

	তথ্য = content_get_source_তথ্য_2(&svg->base, &আকার_2);
	যদি (আকার_2 > 0) {
		যদি (rsvg_process_তথ্য(&svg->base, (ধ্রুবক অক্ষর *)তথ্য, আকার_2) == false) {
			content_destroy(&svg->base);
			ফেরত NSERROR_NOMEM;
		}
	}

	যদি (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		যদি (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ফেরত NSERROR_CLONE_FAILED;
		}
	}

	*newc = (কাঠামো content *) svg;

	ফেরত NSERROR_OK;
}

স্থির শূন্য *rsvg_get_internal(ধ্রুবক কাঠামো content *c, শূন্য *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ফেরত d->bitmap;
}

স্থির content_type rsvg_content_type(শূন্য)
{
	ফেরত CONTENT_IMAGE;
}


স্থির bool rsvg_content_is_opaque(কাঠামো content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	যদি (d->bitmap != ফাঁকা) {
		ফেরত guit->bitmap->get_opaque(d->bitmap);
	}

	ফেরত false;
}


স্থির ধ্রুবক content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_তথ্য = rsvg_process_তথ্য,
	.তথ্য_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_internal = rsvg_get_internal,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

স্থির ধ্রুবক অক্ষর *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

