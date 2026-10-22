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

#شامل <assert.h>
#شامل <limits.h>
#شامل <string.h>
#شامل <stdlib.h>

#شامل <svgtiny.h>

#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/content.h"
#شامل "content/content_protected.h"
#شامل "content/content_factory.h"

#شامل "image/svg.h"

تعریفنوع ساختار svg_content {
	ساختار content base;

	ساختار svgtiny_diagram *diagram;

	صحیح current_width;
	صحیح current_height;
} svg_content;



ایستا nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	اگر (c->diagram == پوچ)
		برو no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	بازگشت NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, پوچ);
	بازگشت NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

ایستا nserror svg_create(ثابت content_handler *handler,
		lwc_string *imime_type, ثابت ساختار http_parameter *params,
		ساختار llcache_handle *llcache, ثابت نویسه *fallback_charset,
		bool quirks, ساختار content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, اندازه(svg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		بازگشت error;
	}

	error = svg_create_svg_data(svg);
	اگر (error != NSERROR_OK) {
		آزاد(svg);
		بازگشت error;
	}

	*c = (ساختار content *) svg;

	بازگشت NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

ایستا bool svg_convert(ساختار content *c)
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

	بازگشت true;
}

/**
 * Reformat a CONTENT_SVG.
 */

ایستا تهی svg_reformat(ساختار content *c, صحیح width, صحیح height)
{
	svg_content *svg = (svg_content *) c;
	ثابت uint8_t *source_data;
	size_t source_size;

	أكد(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	اگر (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (ثابت نویسه *)source_data,
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

ایستا bool
svg_redraw_internal(svg_content *svg,
		    صحیح x,
		    صحیح y,
		    صحیح width,
		    صحیح height,
		    ثابت ساختار rect *clip,
		    ثابت ساختار redraw_context *ctx,
		    اعشاری scale,
		    colour background_colour)
{
	اعشاری transform[6];
	ساختار svgtiny_diagram *diagram = svg->diagram;
	صحیح px, py;
	unsigned صحیح i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	أكد(diagram);

	transform[0] = (اعشاری) width / (اعشاری) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (اعشاری) height / (اعشاری) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	برای (i = 0; i != diagram->shape_شمار; i++) {
		اگر (diagram->shape[i].path) {
			/* stroke style */
			اگر (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} وگرنه {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			اگر (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} وگرنه {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}

		} وگرنه اگر (diagram->shape[i].text) {
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
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		}
	}

#undef BGR

	بازگشت true;
}


ایستا bool
svg_redraw_tiled_internal(svg_content *svg,
			  ساختار content_redraw_data *data,
			  ثابت ساختار rect *clip,
			  ثابت ساختار redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	صحیح x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	اگر (data->repeat_x) {
		برای (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} وگرنه {
		x1 = x + 1;
	}
	اگر (data->repeat_y) {
		برای (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} وگرنه {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	برای (y = y0; y < y1; y += data->height) {
		برای (x = x0; x < x1; x += data->width) {
			اگر (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				بازگشت false;
			}
		}
	}

	بازگشت true;
}


/**
 * Redraw a CONTENT_SVG.
 */
ایستا bool
svg_redraw(ساختار content *c,
	   ساختار content_redraw_data *data,
	   ثابت ساختار rect *clip,
	   ثابت ساختار redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	اگر ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		بازگشت true;
	}

	اگر ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		بازگشت svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	بازگشت svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

ایستا تهی svg_destroy(ساختار content *c)
{
	svg_content *svg = (svg_content *) c;

	اگر (svg->diagram != پوچ)
		svgtiny_free(svg->diagram);
}


ایستا nserror svg_clone(ثابت ساختار content *old, ساختار content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, اندازه(svg_content));
	اگر (svg == پوچ)
		بازگشت NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->base);
		بازگشت error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	اگر (error != NSERROR_OK) {
		content_destroy(&svg->base);
		بازگشت error;
	}

	اگر (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		اگر (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			بازگشت NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ساختار content *) svg;

	بازگشت NSERROR_OK;
}

ایستا content_type svg_content_type(تهی)
{
	بازگشت CONTENT_IMAGE;
}

ایستا ثابت content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

ایستا ثابت نویسه *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
