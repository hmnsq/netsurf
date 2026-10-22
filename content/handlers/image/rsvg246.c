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

#شامل <stdlib.h>
#شامل <stdbool.h>
#شامل <assert.h>
#شامل <string.h>
#شامل <sys/types.h>

#شامل <librsvg/rsvg.h>

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

#شامل "image/image_cache.h"

#شامل "image/rsvg.h"


قسمتعریف ساخت rsvg_content {
	ساخت content بنیاد;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


ساکن nserror
rsvg_create(ثابت content_handler *handler,
	    lwc_string *imime_type,
	    ثابت ساخت http_parameter *params,
	    llcache_handle *llcache,
	    ثابت حرف *fallback_charset,
	    bool quirks,
	    ساخت content **c)
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

	*c = (ساخت content *)svg;

	واپس NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
ساکن ساخت bitmap *
rsvg_cache_convert(ساخت content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ساخت bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	اگر ((bitmap = guit->bitmap->create(c->چوڑائی, c->اونچائی,	BITMAP_NONE)) == تہی) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		واپس تہی;
	}

	اگر ((cs = cairo_image_surface_create_for_ڈیٹا(
		     (غیرعلامتی حرف *)guit->bitmap->get_بفر(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->چوڑائی, c->اونچائی,
		     guit->bitmap->get_rowstride(bitmap))) == تہی) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		واپس تہی;
	}
	اگر ((cr = cairo_create(cs)) == تہی) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		واپس تہی;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.چوڑائی = c->چوڑائی;
	viewport.اونچائی = c->اونچائی;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, تہی);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->چوڑائی, c->اونچائی);

	bitmap_format_مقصد_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	واپس bitmap;
}

ساکن خالی rsvg__get_demensions(ثابت rsvg_content *svgc,
		صحیح *چوڑائی, صحیح *اونچائی)
{
#اگر LIBRSVG_MAJOR_نسخہ >= 2 && LIBRSVG_MINOR_نسخہ >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_سائز_اندر_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	اگر (gotsize == TRUE) {
		*چوڑائی = rwidth;
		*اونچائی = rheight;
	} ورنہ {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     تہی,
						     &ink_rect,
						     &logical_rect,
						     تہی);
		*چوڑائی = ink_rect.چوڑائی;
		*اونچائی = ink_rect.اونچائی;
	}
#ورنہ
	Rsvg_Dimension_ڈیٹا rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*چوڑائی = rsvgsize.چوڑائی;
	*اونچائی = rsvgsize.اونچائی;
#اگر_ختم
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *چوڑائی, *اونچائی);
}

ساکن bool rsvg_convert(ساخت content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ثابت uint8_t *ڈیٹا; /* content data */
	size_t سائز; /* content data size */
	GInput_دھارا * istream;
	GError *gerror = تہی;

	/* check image header is valid and get width/height */

	ڈیٹا = content_get_ماخذ_ڈیٹا_2(c, &سائز);

	istream = g_حافظہ_داخلہ_دھارا_new_from_ڈیٹا(ڈیٹا, سائز, تہی);
	svgc->rsvgh = rsvg_handle_new_from_دھارا_sync(istream,
						       تہی,
						       RSVG_HANDLE_FLAGS_NONE,
						       تہی,
						       &gerror);
	g_object_unref(istream);
	اگر (svgc->rsvgh == تہی) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		واپس false;
	}

	rsvg__get_demensions(svgc, &c->چوڑائی, &c->اونچائی);

	c->سائز = c->چوڑائی * c->اونچائی * 4;

	image_cache_جوڑو(c, تہی, rsvg_cache_convert);

	content_سیٹ_ready(c);
	content_سیٹ_مکمل(c);
	content_سیٹ_حالت(c, ""); /* Done: update status bar */

	واپس true;
}


ساکن nserror rsvg_clone(ثابت ساخت content *old, ساخت content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = صفر_حافظہ_دو(1, حجم(rsvg_content));
	اگر (svg == تہی)
		واپس NSERROR_NOMEM;

	error = content__clone(old, &svg->بنیاد);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->بنیاد);
		واپس error;
	}

	/* re-convert if the content is ready */
	اگر ((old->حالت_2 == CONTENT_حالت_READY) ||
	    (old->حالت_2 == CONTENT_حالت_مکمل)) {
		اگر (rsvg_convert(&svg->بنیاد) == false) {
			content_destroy(&svg->بنیاد);
			واپس NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساخت content *)svg;

	واپس NSERROR_OK;
}


ساکن خالی rsvg_destroy(ساخت content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	اگر (d->rsvgh != تہی) {
		g_object_unref(d->rsvgh);
		d->rsvgh = تہی;
	}

	واپس image_cache_destroy(c);
}

ساکن ثابت content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.ڈیٹا_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_اندرونی = image_cache_get_اندرونی,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

ساکن ثابت حرف *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_رجسٹر_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

