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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ <librsvg/rsvg.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ RSVG_CAIRO_H
#ಅನ್ತರ್ಭಾವ <librsvg/rsvg-cairo.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <nsutils/endian.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "content/llcache.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"
#ಅನ್ತರ್ಭಾವ "desktop/bitmap.h"

#ಅನ್ತರ್ಭಾವ "image/rsvg.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ rsvg_content {
	ರಚನೆ content ತಳ;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	ರಚನೆ bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

ಸ್ಥಿರ nserror rsvg_create_svg_ದತ್ತಾಂಶ(rsvg_content *c)
{
	c->rsvgh = ಖಾಲಿ;
	c->cs = ಖಾಲಿ;
	c->ct = ಖಾಲಿ;
	c->bitmap = ಖಾಲಿ;

	ಆದರೆ ((c->rsvgh = rsvg_handle_new()) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_ಪ್ರಸಾರ_error(&c->ತಳ, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
	}

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ nserror rsvg_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
		lwc_string *imime_type, ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
		llcache_handle *llcache, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
		bool quirks, ರಚನೆ content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(rsvg_content));
	ಆದರೆ (svg == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__init(&svg->ತಳ, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(svg);
		ಹಿಂತಿರುಗಿ error;
	}

	error = rsvg_create_svg_ದತ್ತಾಂಶ(svg);
	ಆದರೆ (error != NSERROR_OK) {
		ಬಿಡುಗಡೆ(svg);
		ಹಿಂತಿರುಗಿ error;
	}

	*c = (ರಚನೆ content *) svg;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ bool rsvg_process_ದತ್ತಾಂಶ(ರಚನೆ content *c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ,
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = ಖಾಲಿ;

	ಆದರೆ (rsvg_handle_write(d->rsvgh, (ಸ್ಥಿರಾಂಕ guchar *)ದತ್ತಾಂಶ, (gsize)ಗಾತ್ರ_2,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->ಸಂದೇಶ_2);
		content_ಪ್ರಸಾರ_error(c, NSERROR_SVG_ERROR, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool rsvg_convert(ರಚನೆ content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_ದತ್ತಾಂಶ rsvgsize;
	GError *err = ಖಾಲಿ;

	ಆದರೆ (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->ಸಂದೇಶ_2);
		content_ಪ್ರಸಾರ_error(c, NSERROR_SVG_ERROR, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಪ್ರತಿಜ್ಞಾ(err == ಖಾಲಿ);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->ಅಗಲ = rsvgsize.ಅಗಲ;
	c->ಎತ್ತರ = rsvgsize.ಎತ್ತರ;

	ಆದರೆ ((d->bitmap = guit->bitmap->create(c->ಅಗಲ, c->ಎತ್ತರ,
			BITMAP_NONE)) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ ((d->cs = cairo_image_surface_create_for_ದತ್ತಾಂಶ(
			(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)guit->bitmap->get_ಬಫರ್(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->ಅಗಲ, c->ಎತ್ತರ,
			guit->bitmap->get_rowstride(d->bitmap))) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ ((d->ct = cairo_create(d->cs)) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_ಪ್ರಸಾರ_error(c, NSERROR_NOMEM, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ false;
	}

	rsvg_handle_render_cairo(d->rsvgh, d->ct);

	bitmap_format_ಗಮ್ಯ_client(d->bitmap, &(bitmap_fmt_t) {
		.layout = BITMAP_LAYOUT_ARGB8888,
	});
	guit->bitmap->modified(d->bitmap);
	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
	/* Done: update status bar */
	content_ಗಣ_ಸ್ಥಿತಿ(c, "");

	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ bool rsvg_redraw(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	ಪ್ರತಿಜ್ಞಾ(rsvgcontent->bitmap != ಖಾಲಿ);

	ಆದರೆ (ದತ್ತಾಂಶ->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	ಆದರೆ (ದತ್ತಾಂಶ->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ಹಿಂತಿರುಗಿ (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  ದತ್ತಾಂಶ->x, ದತ್ತಾಂಶ->y,
				  ದತ್ತಾಂಶ->ಅಗಲ, ದತ್ತಾಂಶ->ಎತ್ತರ,
				  ದತ್ತಾಂಶ->background_ಬಣ್ಣ,
				  flags) == NSERROR_OK);
}

ಸ್ಥಿರ ಶೂನ್ಯ rsvg_destroy(ರಚನೆ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಆದರೆ (d->bitmap != ಖಾಲಿ) guit->bitmap->destroy(d->bitmap);
	ಆದರೆ (d->rsvgh != ಖಾಲಿ) g_object_unref(d->rsvgh);
	ಆದರೆ (d->ct != ಖಾಲಿ) cairo_destroy(d->ct);
	ಆದರೆ (d->cs != ಖಾಲಿ) cairo_surface_destroy(d->cs);

	ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ nserror rsvg_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	rsvg_content *svg;
	nserror error;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ;
	size_t ಗಾತ್ರ_2;

	svg = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(rsvg_content));
	ಆದರೆ (svg == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &svg->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&svg->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_ದತ್ತಾಂಶ(svg);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&svg->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(&svg->ತಳ, &ಗಾತ್ರ_2);
	ಆದರೆ (ಗಾತ್ರ_2 > 0) {
		ಆದರೆ (rsvg_process_ದತ್ತಾಂಶ(&svg->ತಳ, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)ದತ್ತಾಂಶ, ಗಾತ್ರ_2) == false) {
			content_destroy(&svg->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_NOMEM;
		}
	}

	ಆದರೆ (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY ||
			old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ) {
		ಆದರೆ (rsvg_convert(&svg->ತಳ) == false) {
			content_destroy(&svg->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *) svg;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ *rsvg_get_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ರಚನೆ content *c, ಶೂನ್ಯ *context)
{
	rsvg_content *d = (rsvg_content *) c;

	ಹಿಂತಿರುಗಿ d->bitmap;
}

ಸ್ಥಿರ content_type rsvg_content_type(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ CONTENT_IMAGE;
}


ಸ್ಥಿರ bool rsvg_content_is_opaque(ರಚನೆ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಆದರೆ (d->bitmap != ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ guit->bitmap->get_opaque(d->bitmap);
	}

	ಹಿಂತಿರುಗಿ false;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_ದತ್ತಾಂಶ = rsvg_process_ದತ್ತಾಂಶ,
	.ದತ್ತಾಂಶ_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_ಆಂತರಿಕ = rsvg_get_ಆಂತರಿಕ,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

