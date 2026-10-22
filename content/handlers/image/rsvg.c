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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#अन्तर्भाव <librsvg/rsvg-cairo.h>
#endif

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

#अन्तर्भाव "image/rsvg.h"

प्रकारव्याख्या संरचना rsvg_content {
	संरचना content base;

	RsvgHandle *rsvgh;	/**< Context handle for RSVG renderer */
	cairo_surface_t *cs;	/**< The surface built inside a nsbitmap */
	cairo_t *ct;		/**< Cairo drawing context */
	संरचना bitmap *bitmap;	/**< Created NetSurf bitmap */
} rsvg_content;

स्थिर nserror rsvg_create_svg_data(rsvg_content *c)
{
	c->rsvgh = रिक्त;
	c->cs = रिक्त;
	c->ct = रिक्त;
	c->bitmap = रिक्त;

	जर ((c->rsvgh = rsvg_handle_new()) == रिक्त) {
		NSLOG(netsurf, INFO, "rsvg_handle_new() returned NULL.");
		content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
		परत NSERROR_NOMEM;
	}

	परत NSERROR_OK;
}


स्थिर nserror rsvg_create(स्थिरांक content_handler *handler,
		lwc_string *imime_type, स्थिरांक संरचना http_parameter *params,
		llcache_handle *llcache, स्थिरांक अक्षर *fallback_charset,
		bool quirks, संरचना content **c)
{
	rsvg_content *svg;
	nserror error;

	svg = सुस्मृति(1, आकार(rsvg_content));
	जर (svg == रिक्त)
		परत NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	जर (error != NSERROR_OK) {
		मुक्त(svg);
		परत error;
	}

	error = rsvg_create_svg_data(svg);
	जर (error != NSERROR_OK) {
		मुक्त(svg);
		परत error;
	}

	*c = (संरचना content *) svg;

	परत NSERROR_OK;
}


स्थिर bool rsvg_process_data(संरचना content *c, स्थिरांक अक्षर *data,
			अचिन्हित पूर्णांक size)
{
	rsvg_content *d = (rsvg_content *) c;
	GError *err = रिक्त;

	जर (rsvg_handle_write(d->rsvgh, (स्थिरांक guchar *)data, (gsize)size,
				&err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_write returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, रिक्त);
		परत false;
	}

	परत true;
}

स्थिर bool rsvg_convert(संरचना content *c)
{
	rsvg_content *d = (rsvg_content *) c;
	RsvgDimensionData rsvgsize;
	GError *err = रिक्त;

	जर (rsvg_handle_close(d->rsvgh, &err) == FALSE) {
		NSLOG(netsurf, INFO,
		      "rsvg_handle_close returned an error: %s", err->message);
		content_broadcast_error(c, NSERROR_SVG_ERROR, रिक्त);
		परत false;
	}

	प्रतिज्ञा(err == रिक्त);

	/* we should now be able to query librsvg for the natural size of the
	 * graphic, so we can create our bitmap.
	 */

	rsvg_handle_get_dimensions(d->rsvgh, &rsvgsize);
	c->width = rsvgsize.width;
	c->height = rsvgsize.height;

	जर ((d->bitmap = guit->bitmap->create(c->width, c->height,
			BITMAP_NONE)) == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Failed to create bitmap for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		परत false;
	}

	जर ((d->cs = cairo_image_surface_create_for_data(
			(अचिन्हित अक्षर *)guit->bitmap->get_buffer(d->bitmap),
			CAIRO_FORMAT_ARGB32,
			c->width, c->height,
			guit->bitmap->get_rowstride(d->bitmap))) == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo image surface for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		परत false;
	}

	जर ((d->ct = cairo_create(d->cs)) == रिक्त) {
		NSLOG(netsurf, INFO,
		      "Failed to create Cairo drawing context for rsvg render.");
		content_broadcast_error(c, NSERROR_NOMEM, रिक्त);
		परत false;
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

	परत true;
}

स्थिर bool rsvg_redraw(संरचना content *c, संरचना content_redraw_data *data,
		स्थिरांक संरचना rect *clip, स्थिरांक संरचना redraw_context *ctx)
{
	rsvg_content *rsvgcontent = (rsvg_content *) c;
	bitmap_flags_t flags = BITMAPF_NONE;

	प्रतिज्ञा(rsvgcontent->bitmap != रिक्त);

	जर (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	जर (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	परत (ctx->plot->bitmap(ctx,
				  rsvgcontent->bitmap,
				  data->x, data->y,
				  data->width, data->height,
				  data->background_colour,
				  flags) == NSERROR_OK);
}

स्थिर शून्य rsvg_destroy(संरचना content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	जर (d->bitmap != रिक्त) guit->bitmap->destroy(d->bitmap);
	जर (d->rsvgh != रिक्त) g_object_unref(d->rsvgh);
	जर (d->ct != रिक्त) cairo_destroy(d->ct);
	जर (d->cs != रिक्त) cairo_surface_destroy(d->cs);

	परत;
}

स्थिर nserror rsvg_clone(स्थिरांक संरचना content *old, संरचना content **newc)
{
	rsvg_content *svg;
	nserror error;
	स्थिरांक uint8_t *data;
	माप_प्रकार size;

	svg = सुस्मृति(1, आकार(rsvg_content));
	जर (svg == रिक्त)
		परत NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	जर (error != NSERROR_OK) {
		content_destroy(&svg->base);
		परत error;
	}

	/* Simply replay create/process/convert */
	error = rsvg_create_svg_data(svg);
	जर (error != NSERROR_OK) {
		content_destroy(&svg->base);
		परत error;
	}

	data = content__get_source_data(&svg->base, &size);
	जर (size > 0) {
		जर (rsvg_process_data(&svg->base, (स्थिरांक अक्षर *)data, size) == false) {
			content_destroy(&svg->base);
			परत NSERROR_NOMEM;
		}
	}

	जर (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		जर (rsvg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			परत NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) svg;

	परत NSERROR_OK;
}

स्थिर शून्य *rsvg_get_internal(स्थिरांक संरचना content *c, शून्य *context)
{
	rsvg_content *d = (rsvg_content *) c;

	परत d->bitmap;
}

स्थिर content_type rsvg_content_type(शून्य)
{
	परत CONTENT_IMAGE;
}


स्थिर bool rsvg_content_is_opaque(संरचना content *c)
{
	rsvg_content *d = (rsvg_content *) c;

	जर (d->bitmap != रिक्त) {
		परत guit->bitmap->get_opaque(d->bitmap);
	}

	परत false;
}


स्थिर स्थिरांक content_handler rsvg_content_handler = {
	.create = rsvg_create,
	.process_data = rsvg_process_data,
	.data_complete = rsvg_convert,
	.destroy = rsvg_destroy,
	.redraw = rsvg_redraw,
	.clone = rsvg_clone,
	.get_internal = rsvg_get_internal,
	.type = rsvg_content_type,
	.is_opaque = rsvg_content_is_opaque,
	.no_share = false,
};

स्थिर स्थिरांक अक्षर *rsvg_types[] = {
	"image/svg",
	"image/svg+xml"
};

CONTENT_FACTORY_REGISTER_TYPES(nsrsvg, rsvg_types, rsvg_content_handler);

