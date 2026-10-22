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
	بنية content پایه;

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

	svg = حافظه_صفر_بده(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&svg->پایه, handler, imime_type, params,
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

	إذا ((bitmap = guit->bitmap->create(c->عرض, c->ارتفاع,	BITMAP_NONE)) == عدم) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		أرجع عدم;
	}

	إذا ((cs = cairo_image_surface_create_for_داده(
		     (بیعلامت حرف *)guit->bitmap->get_میانگیر(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->عرض, c->ارتفاع,
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
	viewport.عرض = c->عرض;
	viewport.ارتفاع = c->ارتفاع;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, عدم);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->عرض, c->ارتفاع);

	bitmap_format_مقصد_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	أرجع bitmap;
}

ساكن فراغ rsvg__get_demensions(ثابت rsvg_content *svgc,
		صحيح *عرض, صحيح *ارتفاع)
{
#إذا LIBRSVG_MAJOR_نسخه >= 2 && LIBRSVG_MINOR_نسخه >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_اندازه_درون_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	إذا (gotsize == TRUE) {
		*عرض = rwidth;
		*ارتفاع = rheight;
	} وإلا {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     عدم,
						     &ink_rect,
						     &logical_rect,
						     عدم);
		*عرض = ink_rect.عرض;
		*ارتفاع = ink_rect.ارتفاع;
	}
#وإلا
	Rsvg_Dimension_داده rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*عرض = rsvgsize.عرض;
	*ارتفاع = rsvgsize.ارتفاع;
#پایان_اگر
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *عرض, *ارتفاع);
}

ساكن bool rsvg_convert(بنية content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ثابت uint8_t *داده; /* content data */
	size_t اندازه; /* content data size */
	GInput_جریان * istream;
	GError *gerror = عدم;

	/* check image header is valid and get width/height */

	داده = content_get_منبع_داده_2(c, &اندازه);

	istream = g_حافظه_درون_داد_جریان_new_from_داده(داده, اندازه, عدم);
	svgc->rsvgh = rsvg_handle_new_from_جریان_sync(istream,
						       عدم,
						       RSVG_HANDLE_FLAGS_NONE,
						       عدم,
						       &gerror);
	g_object_unref(istream);
	إذا (svgc->rsvgh == عدم) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		أرجع false;
	}

	rsvg__get_demensions(svgc, &c->عرض, &c->ارتفاع);

	c->اندازه = c->عرض * c->ارتفاع * 4;

	image_cache_جمع_کن(c, عدم, rsvg_cache_convert);

	content_مجموعه_ready(c);
	content_مجموعه_انجامشده(c);
	content_مجموعه_وضعیت(c, ""); /* Done: update status bar */

	أرجع true;
}


ساكن nserror rsvg_clone(ثابت بنية content *old, بنية content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = حافظه_صفر_بده(1, حجم(rsvg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &svg->پایه);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->پایه);
		أرجع error;
	}

	/* re-convert if the content is ready */
	إذا ((old->وضعیت == CONTENT_وضعیت_READY) ||
	    (old->وضعیت == CONTENT_وضعیت_انجامشده)) {
		إذا (rsvg_convert(&svg->پایه) == false) {
			content_destroy(&svg->پایه);
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
	.داده_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_داخلی = image_cache_get_داخلی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساكن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_ثبات_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

