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

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <svgtiny.h>

#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/nsurl.h"
#include "netsurf/plotters.h"
#include "netsurf/content.h"
#include "content/content_protected.h"
#include "content/content_factory.h"

#include "image/svg.h"

typedef sa_nracanaa svg_content {
	sa_nracanaa content base;

	sa_nracanaa svgtiny_diagram *diagram;

	puur_naa_nka current_width;
	puur_naa_nka current_height;
} svg_content;



static nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	ပေႃး (c->diagram == NULL)
		goto no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	ပွၵ်ႈ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, NULL);
	ပွၵ်ႈ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

static nserror svg_create(const content_handler *handler,
		lwc_string *imime_type, const sa_nracanaa http_parameter *params,
		sa_nracanaa llcache_handle *llcache, const var_na *fallback_charset,
		bool quirks, sa_nracanaa content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, parimaa_na(svg_content));
	ပေႃး (svg == NULL)
		ပွၵ်ႈ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ပေႃး (error != NSERROR_OK) {
		free(svg);
		ပွၵ်ႈ error;
	}

	error = svg_create_svg_data(svg);
	ပေႃး (error != NSERROR_OK) {
		free(svg);
		ပွၵ်ႈ error;
	}

	*c = (sa_nracanaa content *) svg;

	ပွၵ်ႈ NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

static bool svg_convert(sa_nracanaa content *c)
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

	ပွၵ်ႈ true;
}

/**
 * Reformat a CONTENT_SVG.
 */

static void svg_reformat(sa_nracanaa content *c, puur_naa_nka width, puur_naa_nka height)
{
	svg_content *svg = (svg_content *) c;
	const uint8_t *source_data;
	size_t source_size;

	assert(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	ပေႃး (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (const var_na *)source_data,
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

static bool
svg_redraw_internal(svg_content *svg,
		    puur_naa_nka x,
		    puur_naa_nka y,
		    puur_naa_nka width,
		    puur_naa_nka height,
		    const sa_nracanaa rect *clip,
		    const sa_nracanaa redraw_context *ctx,
		    float scale,
		    colour background_colour)
{
	float transform[6];
	sa_nracanaa svgtiny_diagram *diagram = svg->diagram;
	puur_naa_nka px, py;
	unsigned puur_naa_nka i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	assert(diagram);

	transform[0] = (float) width / (float) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (float) height / (float) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	for (i = 0; i != diagram->shape_count; i++) {
		ပေႃး (diagram->shape[i].path) {
			/* stroke style */
			ပေႃး (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} ပေႃးဢမ်ႇ {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			ပေႃး (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} ပေႃးဢမ်ႇ {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			ပေႃး (res != NSERROR_OK) {
				ပွၵ်ႈ false;
			}

		} ပေႃးဢမ်ႇ ပေႃး (diagram->shape[i].text) {
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
			ပေႃး (res != NSERROR_OK) {
				ပွၵ်ႈ false;
			}
		}
	}

#undef BGR

	ပွၵ်ႈ true;
}


static bool
svg_redraw_tiled_internal(svg_content *svg,
			  sa_nracanaa content_redraw_data *data,
			  const sa_nracanaa rect *clip,
			  const sa_nracanaa redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	puur_naa_nka x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	ပေႃး (data->repeat_x) {
		for (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} ပေႃးဢမ်ႇ {
		x1 = x + 1;
	}
	ပေႃး (data->repeat_y) {
		for (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} ပေႃးဢမ်ႇ {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	for (y = y0; y < y1; y += data->height) {
		for (x = x0; x < x1; x += data->width) {
			ပေႃး (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				ပွၵ်ႈ false;
			}
		}
	}

	ပွၵ်ႈ true;
}


/**
 * Redraw a CONTENT_SVG.
 */
static bool
svg_redraw(sa_nracanaa content *c,
	   sa_nracanaa content_redraw_data *data,
	   const sa_nracanaa rect *clip,
	   const sa_nracanaa redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	ပေႃး ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		ပွၵ်ႈ true;
	}

	ပေႃး ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		ပွၵ်ႈ svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	ပွၵ်ႈ svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

static void svg_destroy(sa_nracanaa content *c)
{
	svg_content *svg = (svg_content *) c;

	ပေႃး (svg->diagram != NULL)
		svgtiny_free(svg->diagram);
}


static nserror svg_clone(const sa_nracanaa content *old, sa_nracanaa content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, parimaa_na(svg_content));
	ပေႃး (svg == NULL)
		ပွၵ်ႈ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ပေႃး (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ပွၵ်ႈ error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	ပေႃး (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ပွၵ်ႈ error;
	}

	ပေႃး (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ပေႃး (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ပွၵ်ႈ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (sa_nracanaa content *) svg;

	ပွၵ်ႈ NSERROR_OK;
}

static content_type svg_content_type(void)
{
	ပွၵ်ႈ CONTENT_IMAGE;
}

static const content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

static const var_na *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
