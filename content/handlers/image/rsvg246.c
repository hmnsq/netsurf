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

#ضمن <stdlib.h>
#ضمن <stdbool.h>
#ضمن <assert.h>
#ضمن <string.h>
#ضمن <sys/types.h>

#ضمن <librsvg/rsvg.h>

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

#ضمن "image/image_cache.h"

#ضمن "image/rsvg.h"


تعريفنوع بنية rsvg_content {
	بنية content بنياد;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ساكن nserror
rsvg_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت بنية http_parameter *params,
	    llcache_handle *llcache,
	    ثابت حرف *fallback_charset,
	    bool quirks,
	    بنية content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = صفر_يادگيري_ڏيو(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&svg->بنياد, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	*c = (بنية content *)svg;

	أرجع NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ساكن بنية bitmap *
rsvg_cache_convert(بنية content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	بنية bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	إذا ((bitmap = guit->bitmap->create(c->ويڪر, c->اوچائي,	BITMAP_NONE)) == عدم) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		أرجع عدم;
	}

	إذا ((cs = cairo_image_surface_create_for_معلومات(
		     (بنانشان حرف *)guit->bitmap->get_بفر(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->ويڪر, c->اوچائي,
		     guit->bitmap->get_rowstride(bitmap))) == عدم) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		أرجع عدم;
	}
	إذا ((cr = cairo_create(cs)) == عدم) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		أرجع عدم;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.ويڪر = c->ويڪر;
	viewport.اوچائي = c->اوچائي;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, عدم);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->ويڪر, c->اوچائي);

	bitmap_format_منزل_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	أرجع bitmap;
}

ساكن فراغ rsvg__get_demensions(ثابت rsvg_content *svgc,
		صحيح *ويڪر, صحيح *اوچائي)
{
#إذا LIBRSVG_MAJOR_نسخو >= 2 && LIBRSVG_MINOR_نسخو >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_سائيز_اندريون_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	إذا (gotsize == TRUE) {
		*ويڪر = rwidth;
		*اوچائي = rheight;
	} وإلا {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     عدم,
						     &ink_rect,
						     &logical_rect,
						     عدم);
		*ويڪر = ink_rect.ويڪر;
		*اوچائي = ink_rect.اوچائي;
	}
#وإلا
	Rsvg_Dimension_معلومات rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*ويڪر = rsvgsize.ويڪر;
	*اوچائي = rsvgsize.اوچائي;
#جيڪڏهن_پورو
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *ويڪر, *اوچائي);
}

ساكن bool rsvg_convert(بنية content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ثابت uint8_t *معلومات; /* content data */
	size_t سائيز; /* content data size */
	GInput_وهڪرو * istream;
	GError *gerror = عدم;

	/* check image header is valid and get width/height */

	معلومات = content_get_ذريعو_معلومات_2(c, &سائيز);

	istream = g_يادگيري_داخلا_وهڪرو_new_from_معلومات(معلومات, سائيز, عدم);
	svgc->rsvgh = rsvg_handle_new_from_وهڪرو_sync(istream,
						       عدم,
						       RSVG_HANDLE_FLAGS_NONE,
						       عدم,
						       &gerror);
	g_object_unref(istream);
	إذا (svgc->rsvgh == عدم) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		أرجع false;
	}

	rsvg__get_demensions(svgc, &c->ويڪر, &c->اوچائي);

	c->سائيز = c->ويڪر * c->اوچائي * 4;

	image_cache_جوڙيو(c, عدم, rsvg_cache_convert);

	content_سيٽ_ready(c);
	content_سيٽ_پورو(c);
	content_سيٽ_حالت(c, ""); /* Done: update status bar */

	أرجع true;
}


ساكن nserror rsvg_clone(ثابت بنية content *old, بنية content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = صفر_يادگيري_ڏيو(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &svg->بنياد);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->بنياد);
		أرجع error;
	}

	/* re-convert if the content is ready */
	إذا ((old->حالت == CONTENT_حالت_READY) ||
	    (old->حالت == CONTENT_حالت_پورو)) {
		إذا (rsvg_convert(&svg->بنياد) == false) {
			content_destroy(&svg->بنياد);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *)svg;

	أرجع NSERROR_OK;
}


ساكن فراغ rsvg_destroy(بنية content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	إذا (d->rsvgh != عدم) {
		g_object_unref(d->rsvgh);
		d->rsvgh = عدم;
	}

	أرجع image_cache_destroy(c);
}

ساكن ثابت content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.معلومات_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_اندروني = image_cache_get_اندروني,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_رجسٽر_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

