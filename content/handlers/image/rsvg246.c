/*
 * Copyright 2022 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of content handler for image/svg using librsvg 2.46 API.
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

#ಅನ್ತರ್ಭಾವ "image/image_cache.h"

#ಅನ್ತರ್ಭಾವ "image/rsvg.h"


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ rsvg_content {
	ರಚನೆ content ತಳ;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ಸ್ಥಿರ nserror
rsvg_create(ಸ್ಥಿರಾಂಕ content_handler *handler,
	    lwc_string *imime_type,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ http_parameter *params,
	    llcache_handle *llcache,
	    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fallback_charset,
	    bool quirks,
	    ರಚನೆ content **c)
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

	*c = (ರಚನೆ content *)svg;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ಸ್ಥಿರ ರಚನೆ bitmap *
rsvg_cache_convert(ರಚನೆ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ರಚನೆ bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ಆದರೆ ((bitmap = guit->bitmap->create(c->ಅಗಲ, c->ಎತ್ತರ,	BITMAP_NONE)) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ ((cs = cairo_image_surface_create_for_ದತ್ತಾಂಶ(
		     (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)guit->bitmap->get_ಬಫರ್(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->ಅಗಲ, c->ಎತ್ತರ,
		     guit->bitmap->get_rowstride(bitmap))) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ ((cr = cairo_create(cs)) == ಖಾಲಿ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.ಅಗಲ = c->ಅಗಲ;
	viewport.ಎತ್ತರ = c->ಎತ್ತರ;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ಖಾಲಿ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->ಅಗಲ, c->ಎತ್ತರ);

	bitmap_format_ಗಮ್ಯ_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	ಹಿಂತಿರುಗಿ bitmap;
}

ಸ್ಥಿರ ಶೂನ್ಯ rsvg__get_demensions(ಸ್ಥಿರಾಂಕ rsvg_content *svgc,
		ಪೂರ್ಣಾಂಕ *ಅಗಲ, ಪೂರ್ಣಾಂಕ *ಎತ್ತರ)
{
#ಆದರೆ LIBRSVG_MAJOR_ಆವೃತ್ತಿ >= 2 && LIBRSVG_MINOR_ಆವೃತ್ತಿ >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_ಗಾತ್ರ_ಒಳ_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ಆದರೆ (gotsize == TRUE) {
		*ಅಗಲ = rwidth;
		*ಎತ್ತರ = rheight;
	} ಇಲ್ಲದಿದ್ದರೆ {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ಖಾಲಿ,
						     &ink_rect,
						     &logical_rect,
						     ಖಾಲಿ);
		*ಅಗಲ = ink_rect.ಅಗಲ;
		*ಎತ್ತರ = ink_rect.ಎತ್ತರ;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	Rsvg_Dimension_ದತ್ತಾಂಶ rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*ಅಗಲ = rsvgsize.ಅಗಲ;
	*ಎತ್ತರ = rsvgsize.ಎತ್ತರ;
#ಆದರೆ_ಅಂತ್ಯ
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *ಅಗಲ, *ಎತ್ತರ);
}

ಸ್ಥಿರ bool rsvg_convert(ರಚನೆ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ; /* content data */
	size_t ಗಾತ್ರ_2; /* content data size */
	GInput_ಹರಿವು * istream;
	GError *gerror = ಖಾಲಿ;

	/* check image header is valid and get width/height */

	ದತ್ತಾಂಶ = content_get_ಮೂಲ_ದತ್ತಾಂಶ_2(c, &ಗಾತ್ರ_2);

	istream = g_ಸ್ಮರಣೆ_ಒಳಹರಿವು_ಹರಿವು_new_from_ದತ್ತಾಂಶ(ದತ್ತಾಂಶ, ಗಾತ್ರ_2, ಖಾಲಿ);
	svgc->rsvgh = rsvg_handle_new_from_ಹರಿವು_sync(istream,
						       ಖಾಲಿ,
						       RSVG_HANDLE_FLAGS_NONE,
						       ಖಾಲಿ,
						       &gerror);
	g_object_unref(istream);
	ಆದರೆ (svgc->rsvgh == ಖಾಲಿ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ಹಿಂತಿರುಗಿ false;
	}

	rsvg__get_demensions(svgc, &c->ಅಗಲ, &c->ಎತ್ತರ);

	c->ಗಾತ್ರ_2 = c->ಅಗಲ * c->ಎತ್ತರ * 4;

	image_cache_ಕೂಡಿಸು(c, ಖಾಲಿ, rsvg_cache_convert);

	content_ಗಣ_ready(c);
	content_ಗಣ_ಮುಗಿದ(c);
	content_ಗಣ_ಸ್ಥಿತಿ(c, ""); /* Done: update status bar */

	ಹಿಂತಿರುಗಿ true;
}


ಸ್ಥಿರ nserror rsvg_clone(ಸ್ಥಿರಾಂಕ ರಚನೆ content *old, ರಚನೆ content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(rsvg_content));
	ಆದರೆ (svg == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ NSERROR_NOMEM;

	error = content__clone(old, &svg->ತಳ);
	ಆದರೆ (error != NSERROR_OK) {
		content_destroy(&svg->ತಳ);
		ಹಿಂತಿರುಗಿ error;
	}

	/* re-convert if the content is ready */
	ಆದರೆ ((old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_READY) ||
	    (old->ಸ್ಥಿತಿ_2 == CONTENT_ಸ್ಥಿತಿ_ಮುಗಿದ)) {
		ಆದರೆ (rsvg_convert(&svg->ತಳ) == false) {
			content_destroy(&svg->ತಳ);
			ಹಿಂತಿರುಗಿ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ರಚನೆ content *)svg;

	ಹಿಂತಿರುಗಿ NSERROR_OK;
}


ಸ್ಥಿರ ಶೂನ್ಯ rsvg_destroy(ರಚನೆ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ಆದರೆ (d->rsvgh != ಖಾಲಿ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ image_cache_destroy(c);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.ದತ್ತಾಂಶ_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_ಆಂತರಿಕ = image_cache_get_ಆಂತರಿಕ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ರಿಜಿಸ್ಟರ್_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

