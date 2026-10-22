/*
 * Copyright 2007-2008 James Bursa <bursa@users.sourceforge.net>
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
 * implementation of content for image/svg using libsvgtiny.
 */

#अन्तर्भाव <assert.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव <svgtiny.h>

#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/content_factory.h"

#अन्तर्भाव "image/svg.h"

प्रकारनिर्देश संरचना svg_content {
	संरचना content base;

	संरचना svgtiny_diagram *diagram;

	पूर्णाङ्क current_width;
	पूर्णाङ्क current_height;
} svg_content;



स्थिर nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	यदि (c->diagram == रिक्त)
		गच्छ no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	प्रत्यागम NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, रिक्त);
	प्रत्यागम NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

स्थिर nserror svg_create(ध्रुव content_handler *handler,
		lwc_string *imime_type, ध्रुव संरचना http_parameter *params,
		संरचना llcache_handle *llcache, ध्रुव वर्ण *fallback_charset,
		bool quirks, संरचना content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, परिमाण(svg_content));
	यदि (svg == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	यदि (error != NSERROR_OK) {
		मुक्त(svg);
		प्रत्यागम error;
	}

	error = svg_create_svg_data(svg);
	यदि (error != NSERROR_OK) {
		मुक्त(svg);
		प्रत्यागम error;
	}

	*c = (संरचना content *) svg;

	प्रत्यागम NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

स्थिर bool svg_convert(संरचना content *c)
{
	/*c->title = malloc(100);
	if (c->title)
		snprintf(c->title, 100, messages_get("svgTitle"),
				width, height, c->source_size);*/
	//c->size += ?;
	content_set_ready(c);
	content_set_done(c);
	/* Done: update status bar */
	content_set_status(c, "");

	प्रत्यागम true;
}

/**
 * Reformat a CONTENT_SVG.
 */

स्थिर शून्य svg_reformat(संरचना content *c, पूर्णाङ्क width, पूर्णाङ्क height)
{
	svg_content *svg = (svg_content *) c;
	ध्रुव uint8_t *source_data;
	size_t source_size;

	प्रतिज्ञा(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	यदि (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (ध्रुव वर्ण *)source_data,
			      source_size,
			      nsurl_access(content_get_url(c)),
			      width,
			      height);

		svg->current_width = width;
		svg->current_height = height;
	}

	c->width = svg->diagram->width;
	c->height = svg->diagram->height;
}


/**
 * Redraw a CONTENT_SVG.
 */

स्थिर bool
svg_redraw_internal(svg_content *svg,
		    पूर्णाङ्क x,
		    पूर्णाङ्क y,
		    पूर्णाङ्क width,
		    पूर्णाङ्क height,
		    ध्रुव संरचना rect *clip,
		    ध्रुव संरचना redraw_context *ctx,
		    भिन्न scale,
		    colour background_colour)
{
	भिन्न transform[6];
	संरचना svgtiny_diagram *diagram = svg->diagram;
	पूर्णाङ्क px, py;
	unsigned पूर्णाङ्क i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	प्रतिज्ञा(diagram);

	transform[0] = (भिन्न) width / (भिन्न) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (भिन्न) height / (भिन्न) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	प्रति (i = 0; i != diagram->shape_मोजणी; i++) {
		यदि (diagram->shape[i].path) {
			/* stroke style */
			यदि (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} अन्यथा {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			यदि (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} अन्यथा {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			यदि (res != NSERROR_OK) {
				प्रत्यागम false;
			}

		} अन्यथा यदि (diagram->shape[i].text) {
			px = transform[0] * diagram->shape[i].text_x +
				transform[2] * diagram->shape[i].text_y +
				transform[4];
			py = transform[1] * diagram->shape[i].text_x +
				transform[3] * diagram->shape[i].text_y +
				transform[5];

			fstyle.background = 0xffffff;
			fstyle.foreground = 0x000000;
			fstyle.size = (8 * PLOT_STYLE_SCALE) * scale;

			res = ctx->plot->text(ctx,
					      &fstyle,
					      px, py,
					      diagram->shape[i].text,
					      strlen(diagram->shape[i].text));
			यदि (res != NSERROR_OK) {
				प्रत्यागम false;
			}
		}
	}

#undef BGR

	प्रत्यागम true;
}


स्थिर bool
svg_redraw_tiled_internal(svg_content *svg,
			  संरचना content_redraw_data *data,
			  ध्रुव संरचना rect *clip,
			  ध्रुव संरचना redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	पूर्णाङ्क x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	यदि (data->repeat_x) {
		प्रति (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} अन्यथा {
		x1 = x + 1;
	}
	यदि (data->repeat_y) {
		प्रति (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} अन्यथा {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	प्रति (y = y0; y < y1; y += data->height) {
		प्रति (x = x0; x < x1; x += data->width) {
			यदि (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				प्रत्यागम false;
			}
		}
	}

	प्रत्यागम true;
}


/**
 * Redraw a CONTENT_SVG.
 */
स्थिर bool
svg_redraw(संरचना content *c,
	   संरचना content_redraw_data *data,
	   ध्रुव संरचना rect *clip,
	   ध्रुव संरचना redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	यदि ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		प्रत्यागम true;
	}

	यदि ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		प्रत्यागम svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	प्रत्यागम svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

स्थिर शून्य svg_destroy(संरचना content *c)
{
	svg_content *svg = (svg_content *) c;

	यदि (svg->diagram != रिक्त)
		svgtiny_free(svg->diagram);
}


स्थिर nserror svg_clone(ध्रुव संरचना content *old, संरचना content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, परिमाण(svg_content));
	यदि (svg == रिक्त)
		प्रत्यागम NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	यदि (error != NSERROR_OK) {
		content_destroy(&svg->base);
		प्रत्यागम error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	यदि (error != NSERROR_OK) {
		content_destroy(&svg->base);
		प्रत्यागम error;
	}

	यदि (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		यदि (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			प्रत्यागम NSERROR_CLONE_FAILED;
		}
	}

	*newc = (संरचना content *) svg;

	प्रत्यागम NSERROR_OK;
}

स्थिर content_type svg_content_type(शून्य)
{
	प्रत्यागम CONTENT_IMAGE;
}

स्थिर ध्रुव content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

स्थिर ध्रुव वर्ण *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
