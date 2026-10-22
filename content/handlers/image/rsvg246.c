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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव <librsvg/rsvg.h>

#अन्तर्भाव <nsutils/endian.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/llcache.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"
#अन्तर्भाव "desktop/gui_internal.h"
#अन्तर्भाव "desktop/bitmap.h"

#अन्तर्भाव "image/image_cache.h"

#अन्तर्भाव "image/rsvg.h"


प्रकारनिर्देश संरचना rsvg_content {
	संरचना content आधारः;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
} rsvg_content;


स्थिर nserror
rsvg_create(ध्रुव content_handler *handler,
	    lwc_string *imime_type,
	    ध्रुव संरचना http_parameter *params,
	    llcache_handle *llcache,
	    ध्रुव वर्ण *fallback_charset,
	    bool quirks,
	    संरचना content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = शून्यस्मृतिं_देहि(1, परिमाण(rsvg_content));
	यदि (svg == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&svg->आधारः, handler, imime_type, params,
			      llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(svg);
		प्रत्यागम error;
	}

	*c = (संरचना content *)svg;

	प्रत्यागम NSERROR_OK;
}


/**
 * create a bitmap from jpeg content for the image cache.
 */
स्थिर संरचना bitmap *
rsvg_cache_convert(संरचना content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	संरचना bitmap *bitmap;
	cairo_surface_t *cs;
	cairo_t *cr;
	RsvgRectangle viewport;
	gboolean renderres;

	यदि ((bitmap = guit->bitmap->create(c->विस्तारः, c->height,	BITMAP_NONE)) == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create bitmap for rsvg render.");
		प्रत्यागम रिक्त;
	}

	यदि ((cs = cairo_image_surface_create_for_दत्त(
		     (अचिह्नित वर्ण *)guit->bitmap->get_बफर(bitmap),
		     CAIRO_FORMAT_ARGB32,
		     c->विस्तारः, c->height,
		     guit->bitmap->get_rowstride(bitmap))) == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create Cairo image surface for rsvg render.");
		guit->bitmap->destroy(bitmap);
		प्रत्यागम रिक्त;
	}
	यदि ((cr = cairo_create(cs)) == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		cairo_surface_destroy(cs);
		guit->bitmap->destroy(bitmap);
		प्रत्यागम रिक्त;
	}

	viewport.x = 0;
	viewport.y = 0;
	viewport.विस्तारः = c->विस्तारः;
	viewport.height = c->height;
	renderres = rsvg_handle_render_document(svgc->rsvgh, cr, &viewport, रिक्त);
	NSLOG(netsurf, DEBUG, "rsvg render:%d, width:%d, height %d", renderres, c->विस्तारः, c->height);

	bitmap_format_गन्तव्यम्_client(bitmap, &(bitmap_fmt_t) {
			.layout = BITMAP_LAYOUT_ARGB8888,
		});
	guit->bitmap->modified(bitmap);

	cairo_destroy(cr);
	cairo_surface_destroy(cs);

	प्रत्यागम bitmap;
}

स्थिर शून्य rsvg__get_demensions(ध्रुव rsvg_content *svgc,
		पूर्णाङ्क *विस्तारः, पूर्णाङ्क *height)
{
#यदि LIBRSVG_MAJOR_संस्करणम् >= 2 && LIBRSVG_MINOR_संस्करणम् >= 52
	gdouble rwidth;
	gdouble rheight;
	gboolean gotsize;

	gotsize = rsvg_handle_get_intrinsic_आकार_अन्तर्_pixels(svgc->rsvgh,
							   &rwidth,
							   &rheight);
	यदि (gotsize == TRUE) {
		*विस्तारः = rwidth;
		*height = rheight;
	} अन्यथा {
		RsvgRectangle ink_rect;
		RsvgRectangle logical_rect;
		rsvg_handle_get_geometry_for_element(svgc->rsvgh,
						     रिक्त,
						     &ink_rect,
						     &logical_rect,
						     रिक्त);
		*विस्तारः = ink_rect.विस्तारः;
		*height = ink_rect.height;
	}
#अन्यथा
	Rsvg_Dimension_दत्त rsvgsize;

	rsvg_handle_get_dimensions(svgc->rsvgh, &rsvgsize);
	*विस्तारः = rsvgsize.विस्तारः;
	*height = rsvgsize.height;
#यदि_अन्तः
	NSLOG(netsurf, DEBUG, "rsvg width:%d height:%d.", *विस्तारः, *height);
}

स्थिर bool rsvg_convert(संरचना content *c)
{
	rsvg_content *svgc = (rsvg_content *)c;
	ध्रुव uint8_t *दत्त; /* content data */
	size_t आकार; /* content data size */
	GInput_प्रवाहः * istream;
	GError *gerror = रिक्त;

	/* check image header is valid and get width/height */

	दत्त = content_get_source_दत्त_2(c, &आकार);

	istream = g_स्मृतिः_निवेशः_प्रवाहः_new_from_दत्त(दत्त, आकार, रिक्त);
	svgc->rsvgh = rsvg_handle_new_from_प्रवाहः_sync(istream,
						       रिक्त,
						       RSVG_HANDLE_FLAGS_NONE,
						       रिक्त,
						       &gerror);
	g_object_unref(istream);
	यदि (svgc->rsvgh == रिक्त) {
		NSLOG(netsurf, INFO, "Failed to create rsvg handle for content.");
		प्रत्यागम false;
	}

	rsvg__get_demensions(svgc, &c->विस्तारः, &c->height);

	c->आकार = c->विस्तारः * c->height * 4;

	image_cache_सङ्कलय(c, रिक्त, rsvg_cache_convert);

	content_समुच्चयः_ready(c);
	content_समुच्चयः_समाप्तम्(c);
	content_समुच्चयः_स्थितिः(c, ""); /* Done: update status bar */

	प्रत्यागम true;
}


स्थिर nserror rsvg_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	rsvg_content *svg;
	nserror error;

	svg = शून्यस्मृतिं_देहि(1, परिमाण(rsvg_content));
	यदि (svg == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &svg->आधारः);
	यदि (error != NSERROR_OK) {
		content_destroy(&svg->आधारः);
		प्रत्यागम error;
	}

	/* re-convert if the content is ready */
	यदि ((old->स्थितिः == CONTENT_स्थितिः_READY) ||
	    (old->स्थितिः == CONTENT_स्थितिः_समाप्तम्)) {
		यदि (rsvg_convert(&svg->आधारः) == false) {
			content_destroy(&svg->आधारः);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *)svg;

	प्रत्यागम NSERROR_OK;
}


स्थिर शून्य rsvg_destroy(संरचना content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	यदि (d->rsvgh != रिक्त) {
		g_object_unref(d->rsvgh);
		d->rsvgh = रिक्त;
	}

	प्रत्यागम image_cache_destroy(c);
}

स्थिर ध्रुव content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.दत्त_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = image_cache_redraw,
	.clone = rsvg_clone,
	.get_आन्तरिकम् = image_cache_get_आन्तरिकम्,
	.type = image_cache_content_type,
	.is_opaque = image_cache_is_opaque,
	.no_share = false,
};

स्थिर ध्रुव वर्ण *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_पञ्जिकाकोशः_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

