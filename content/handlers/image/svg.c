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

#ضمن <assert.h>
#ضمن <limits.h>
#ضمن <string.h>
#ضمن <stdlib.h>

#ضمن <svgtiny.h>

#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/content_factory.h"

#ضمن "image/svg.h"

تعريفنوع بنية svg_content {
	بنية content base;

	بنية svgtiny_diagram *diagram;

	صحيح current_width;
	صحيح current_height;
} svg_content;



ساكن nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	إذا (c->diagram == عدم)
		اذهب no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	أرجع NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, عدم);
	أرجع NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

ساكن nserror svg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت بنية http_parameter *params,
		بنية llcache_handle *llcache, ثابت حرف *fallback_charset,
		bool quirks, بنية content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, حجم(svg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	error = svg_create_svg_data(svg);
	إذا (error != NSERROR_OK) {
		حرر(svg);
		أرجع error;
	}

	*c = (بنية content *) svg;

	أرجع NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

ساكن bool svg_convert(بنية content *c)
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

	أرجع true;
}

/**
 * Reformat a CONTENT_SVG.
 */

ساكن فراغ svg_reformat(بنية content *c, صحيح width, صحيح height)
{
	svg_content *svg = (svg_content *) c;
	ثابت uint8_t *source_data;
	size_t source_size;

	أكد(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	إذا (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (ثابت حرف *)source_data,
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

ساكن bool
svg_redraw_internal(svg_content *svg,
		    صحيح x,
		    صحيح y,
		    صحيح width,
		    صحيح height,
		    ثابت بنية rect *clip,
		    ثابت بنية redraw_context *ctx,
		    عائم scale,
		    colour background_colour)
{
	عائم transform[6];
	بنية svgtiny_diagram *diagram = svg->diagram;
	صحيح px, py;
	unsigned صحيح i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	أكد(diagram);

	transform[0] = (عائم) width / (عائم) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (عائم) height / (عائم) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	لكل (i = 0; i != diagram->shape_عدد; i++) {
		إذا (diagram->shape[i].path) {
			/* stroke style */
			إذا (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} وإلا {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			إذا (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} وإلا {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

		} وإلا إذا (diagram->shape[i].text) {
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
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}
	}

#undef BGR

	أرجع true;
}


ساكن bool
svg_redraw_tiled_internal(svg_content *svg,
			  بنية content_redraw_data *data,
			  ثابت بنية rect *clip,
			  ثابت بنية redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	صحيح x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	إذا (data->repeat_x) {
		لكل (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} وإلا {
		x1 = x + 1;
	}
	إذا (data->repeat_y) {
		لكل (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} وإلا {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	لكل (y = y0; y < y1; y += data->height) {
		لكل (x = x0; x < x1; x += data->width) {
			إذا (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				أرجع false;
			}
		}
	}

	أرجع true;
}


/**
 * Redraw a CONTENT_SVG.
 */
ساكن bool
svg_redraw(بنية content *c,
	   بنية content_redraw_data *data,
	   ثابت بنية rect *clip,
	   ثابت بنية redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	إذا ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		أرجع true;
	}

	إذا ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		أرجع svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	أرجع svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

ساكن فراغ svg_destroy(بنية content *c)
{
	svg_content *svg = (svg_content *) c;

	إذا (svg->diagram != عدم)
		svgtiny_free(svg->diagram);
}


ساكن nserror svg_clone(ثابت بنية content *old, بنية content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, حجم(svg_content));
	إذا (svg == عدم)
		أرجع NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->base);
		أرجع error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	إذا (error != NSERROR_OK) {
		content_destroy(&svg->base);
		أرجع error;
	}

	إذا (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		إذا (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			أرجع NSERROR_CLONE_FAILED;
		}
	}

	*newc = (بنية content *) svg;

	أرجع NSERROR_OK;
}

ساكن content_type svg_content_type(فراغ)
{
	أرجع CONTENT_IMAGE;
}

ساكن ثابت content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

ساكن ثابت حرف *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
