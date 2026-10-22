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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <librsvg/rsvg.h>

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

#ਅਨ੍ਤਰ੍ਭਾਵ "image/image_cache.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "image/rsvg.h"


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rsvg_content {
	ਬਣਤਰ content ਅਧਾਰ;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ਸਥਿਰ nserror
rsvg_create(ਅਚਲ content_handler *handler,
	    lwc_string *imime_type,
	    ਅਚਲ ਬਣਤਰ http_parameter *params,
	    llcache_handle *llcache,
	    ਅਚਲ ਅੱਖਰ *fallback_charset,
	    bool quirks,
	    ਬਣਤਰ content **c)
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

	*c = (ਬਣਤਰ content *)svg;

	ਵਾਪਸ NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ਸਥਿਰ ਬਣਤਰ bitmap *
rsvg_cache_convert(ਬਣਤਰ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ਬਣਤਰ bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	ਜੇ ((bitmap = guit->bitmap->create(c->ਚੌੜਾਈ, c->ਉਚਾਈ,	BITMAP_NONE)) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ ((cs = cairo_image_surface_create_for_ਅੰਕੜੇ(
		     (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)guit->bitmap->get_ਬਫਰ(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->ਚੌੜਾਈ, c->ਉਚਾਈ,
		     guit->bitmap->get_rowstride(bitmap))) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ ((cr = cairo_create(cs)) == ਨਿਰਾ) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		ਵਾਪਸ ਨਿਰਾ;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.ਚੌੜਾਈ = c->ਚੌੜਾਈ;
	viewport.ਉਚਾਈ = c->ਉਚਾਈ;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, ਨਿਰਾ);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->ਚੌੜਾਈ, c->ਉਚਾਈ);

	bitmap_format_ਮੰਜ਼ਿਲ_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	ਵਾਪਸ bitmap;
}

ਸਥਿਰ ਖਾਲੀ rsvg__get_demensions(ਅਚਲ rsvg_content *svgc,
		ਪੂਰਨ *ਚੌੜਾਈ, ਪੂਰਨ *ਉਚਾਈ)
{
#ਜੇ LIBRSVG_MAJOR_ਸੰਸਕਰਣ >= 2 && LIBRSVG_MINOR_ਸੰਸਕਰਣ >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_ਮਾਪ_ਵਿੱਚ_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	ਜੇ (gotsize == TRUE) {
		*ਚੌੜਾਈ = rwidth;
		*ਉਚਾਈ = rheight;
	} ਨਹੀਂਤਾਂ {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     ਨਿਰਾ,
						     &ink_rect,
						     &logical_rect,
						     ਨਿਰਾ);
		*ਚੌੜਾਈ = ink_rect.ਚੌੜਾਈ;
		*ਉਚਾਈ = ink_rect.ਉਚਾਈ;
	}
#ਨਹੀਂਤਾਂ
	Rsvg_Dimension_ਅੰਕੜੇ rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*ਚੌੜਾਈ = rsvgsize.ਚੌੜਾਈ;
	*ਉਚਾਈ = rsvgsize.ਉਚਾਈ;
#ਜੇ_ਅੰਤ
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *ਚੌੜਾਈ, *ਉਚਾਈ);
}

ਸਥਿਰ bool rsvg_convert(ਬਣਤਰ content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ਅਚਲ uint8_t *ਅੰਕੜੇ; /* content data */
	size_t ਮਾਪ; /* content data size */
	GInput_ਧਾਰਾ * istream;
	GError *gerror = ਨਿਰਾ;

	/* check image header is valid and get width/height */

	ਅੰਕੜੇ = content_get_ਸਰੋਤ_ਅੰਕੜੇ_2(c, &ਮਾਪ);

	istream = g_ਸਮ੍ਰਿਤੀ_ਨਿਵੇਸ਼_ਧਾਰਾ_new_from_ਅੰਕੜੇ(ਅੰਕੜੇ, ਮਾਪ, ਨਿਰਾ);
	svgc->rsvgh = rsvg_handle_new_from_ਧਾਰਾ_sync(istream,
						       ਨਿਰਾ,
						       RSVG_HANDLE_FLAGS_NONE,
						       ਨਿਰਾ,
						       &gerror);
	g_object_unref(istream);
	ਜੇ (svgc->rsvgh == ਨਿਰਾ) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		ਵਾਪਸ false;
	}

	rsvg__get_demensions(svgc, &c->ਚੌੜਾਈ, &c->ਉਚਾਈ);

	c->ਮਾਪ = c->ਚੌੜਾਈ * c->ਉਚਾਈ * 4;

	image_cache_ਜੋੜੋ(c, ਨਿਰਾ, rsvg_cache_convert);

	content_ਸੈੱਟ_ready(c);
	content_ਸੈੱਟ_ਪੂਰਾ(c);
	content_ਸੈੱਟ_ਸਥਿਤੀ(c, ""); /* Done: update status bar */

	ਵਾਪਸ true;
}


ਸਥਿਰ nserror rsvg_clone(ਅਚਲ ਬਣਤਰ content *old, ਬਣਤਰ content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(rsvg_content));
	ਜੇ (svg == ਨਿਰਾ)
		ਵਾਪਸ NSERROR_NOMEM;

	error = content__clone(old, &svg->ਅਧਾਰ);
	ਜੇ (error != NSERROR_OK) {
		content_destroy(&svg->ਅਧਾਰ);
		ਵਾਪਸ error;
	}

	/* re-convert if the content is ready */
	ਜੇ ((old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_READY) ||
	    (old->ਸਥਿਤੀ == CONTENT_ਸਥਿਤੀ_ਪੂਰਾ)) {
		ਜੇ (rsvg_convert(&svg->ਅਧਾਰ) == false) {
			content_destroy(&svg->ਅਧਾਰ);
			ਵਾਪਸ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ਬਣਤਰ content *)svg;

	ਵਾਪਸ NSERROR_OK;
}


ਸਥਿਰ ਖਾਲੀ rsvg_destroy(ਬਣਤਰ content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	ਜੇ (d->rsvgh != ਨਿਰਾ) {
		g_object_unref(d->rsvgh);
		d->rsvgh = ਨਿਰਾ;
	}

	ਵਾਪਸ image_cache_destroy(c);
}

ਸਥਿਰ ਅਚਲ content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.ਅੰਕੜੇ_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_ਅੰਦਰੂਨੀ = image_cache_get_ਅੰਦਰੂਨੀ,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ਰਜਿਸਟਰ_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

