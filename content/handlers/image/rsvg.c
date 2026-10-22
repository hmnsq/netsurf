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

#समावेश <stdlib.h>
#समावेश <stdbool.h>
#समावेश <assert.h>
#समावेश <string.h>
#समावेश <sys/types.h>

#समावेश <librsvg/rsvg.h>
#अगर_अघोषित RSVG_CAIRO_H
#समावेश <librsvg/rsvg-cairo.h>
#पूर्ण_अगर

#समावेश <nsutils/endian.h>

#समावेश "utils/log.h"
#समावेश "utils/utils.h"
#समावेश "utils/messages.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/content.h"
#समावेश "content/llcache.h"
#समावेश "content/content_protected.h"
#समावेश "content/content_factory.h"
#समावेश "desktop/gui_internal.h"
#समावेश "desktop/bitmap.h"

#समावेश "image/rsvg.h"

प्रकार काष्ठा rsvg_content {
	काष्ठा content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	काष्ठा bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

अटल nserror rsvg_create_svg_तथ्य(rsvg_content *c)
{
	c->rsvgh = शून्य;
	c->cs = शून्य;
	c->ct = शून्य;
	c->bitmap = शून्य;

	अगर ((c->rsvgh = rsvg_handle_new()) == शून्य) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, शून्य);
		वापस NSERROR_NOMEM;
	}

	वापस NSERROR_OK;
}


अटल nserror rsvg_create(स्थिर content_handler *handler,
		lwc_string *imime_type, स्थिर काष्ठा http_parameter *params,
		llcache_handle *llcache, स्थिर अक्षर *fallback_charset,
		bool quirks, काष्ठा content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, माप(rsvg_content));
	अगर (svg == शून्य)
		वापस NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	अगर (error != NSERROR_OK) {
		मुक्त(svg);
		वापस error;
	}

	error = rsvg_create_svg_तथ्य(svg);
	अगर (error != NSERROR_OK) {
		मुक्त(svg);
		वापस error;
	}

	*c = (काष्ठा content *) svg;

	वापस NSERROR_OK;
}


अटल bool rsvg_process_तथ्य(काष्ठा content *c, स्थिर अक्षर *तथ्य,
			अचिन्हित पूर्णांक आकार)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = शून्य;

	अगर (rsvg_handle_write(d->rsvgh, (स्थिर guchar *)तथ्य, (gsize)आकार,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, शून्य);
		वापस false;
	}

	वापस true;
}

अटल bool rsvg_convert(काष्ठा content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	Rsvg_Dimension_तथ्य rsvgsize;
	GError *err = शून्य;

	अगर (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, शून्य);
		वापस false;
	}

	निश्चित(err == शून्य);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	अगर ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == शून्य) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
	}

	अगर ((d->cs = cairo_image_surface_create_for_तथ्य(
			(अचिन्हित अक्षर *)guit->bitmap->get_बफर(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == शून्य) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
	}

	अगर ((d->ct = cairo_create(d->cs)) == शून्य) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, शून्य);
		वापस false;
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

	वापस true;
}

अटल bool rsvg_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	निश्चित(rsvgcontent->bitmap != शून्य);

	अगर (तथ्य->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	अगर (तथ्य->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	वापस (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  तथ्य->x, तथ्य->y,
				  तथ्य->width, तथ्य->height,
				  तथ्य->background_colour,
				  flags) == NSERROR_OK);
}

अटल व्योम rsvg_destroy(काष्ठा content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	अगर (d->bitmap != शून्य) guit->bitmap->destroy(d->bitmap);
	अगर (d->rsvgh != शून्य) g_object_unref(d->rsvgh);
	अगर (d->ct != शून्य) cairo_destroy(d->ct);
	अगर (d->cs != शून्य) cairo_surface_destroy(d->cs);

	वापस;
}

अटल nserror rsvg_clone(स्थिर काष्ठा content *old, काष्ठा content **newc)
{
	rsvg_content *svg;
	nserror error;
	स्थिर uint8_t *तथ्य;
	माप_प्रकार आकार;

	svg = सुस्मृति(1, माप(rsvg_content));
	अगर (svg == शून्य)
		वापस NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	अगर (error != NSERROR_OK) {
		content_destroy(&svg->base);
		वापस error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_तथ्य(svg);
	अगर (error != NSERROR_OK) {
		content_destroy(&svg->base);
		वापस error;
	}

	तथ्य = content_get_source_तथ्य_2(&svg->base, &आकार);
	अगर (आकार > 0) {
		अगर (rsvg_process_तथ्य(&svg->base, (स्थिर अक्षर *)तथ्य, आकार) == false) {
			content_destroy(&svg->base);
			वापस NSERROR_NOMEM;
		}
	}

	अगर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		अगर (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			वापस NSERROR_CLONE_FAILED;
		}
	}

	*newc = (काष्ठा content *) svg;

	वापस NSERROR_OK;
}

अटल व्योम *rsvg_get_internal(स्थिर काष्ठा content *c, व्योम *context)
{
	rsvg_content *d = (rsvg_content *) c;

	वापस d->bitmap;
}

अटल content_type rsvg_content_type(व्योम)
{
	वापस CONTENT_IMAGE;
}


अटल bool rsvg_content_is_opaque(काष्ठा content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	अगर (d->bitmap != शून्य) {
		वापस guit->bitmap->get_opaque(d->bitmap);
	}

	वापस false;
}


अटल स्थिर content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_तथ्य = rsvg_process_तथ्य,
	.तथ्य_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_internal = rsvg_get_internal,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

अटल स्थिर अक्षर *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

