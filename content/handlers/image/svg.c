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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ <svgtiny.h>

#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"

#ଅନ୍ତର୍ଭାଵ "image/svg.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା svg_content {
	ସଂରଚନା content base;

	ସଂରଚନା svgtiny_diagram *diagram;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା current_width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା current_height;
} svg_content;



ସ୍ଥିର nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	ଯଦି (c->diagram == ଖାଲି)
		ଯାଅ no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	ଫେରନ୍ତୁ NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ଖାଲି);
	ଫେରନ୍ତୁ NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

ସ୍ଥିର nserror svg_create(ଅଚଳ content_handler *handler,
		lwc_string *imime_type, ଅଚଳ ସଂରଚନା http_parameter *params,
		ସଂରଚନା llcache_handle *llcache, ଅଚଳ ଅକ୍ଷର *fallback_charset,
		bool quirks, ସଂରଚନା content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, ଆକାର(svg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(svg);
		ଫେରନ୍ତୁ error;
	}

	error = svg_create_svg_data(svg);
	ଯଦି (error != NSERROR_OK) {
		ମୁକ୍ତ(svg);
		ଫେରନ୍ତୁ error;
	}

	*c = (ସଂରଚନା content *) svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

ସ୍ଥିର bool svg_convert(ସଂରଚନା content *c)
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

	ଫେରନ୍ତୁ true;
}

/**
 * Reformat a CONTENT_SVG.
 */

ସ୍ଥିର ଶୂନ୍ୟ svg_reformat(ସଂରଚନା content *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height)
{
	svg_content *svg = (svg_content *) c;
	ଅଚଳ uint8_t *source_data;
	size_t source_size;

	ପ୍ରତିଜ୍ଞା(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	ଯଦି (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (ଅଚଳ ଅକ୍ଷର *)source_data,
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

ସ୍ଥିର bool
svg_redraw_internal(svg_content *svg,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା width,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
		    ଅଚଳ ସଂରଚନା rect *clip,
		    ଅଚଳ ସଂରଚନା redraw_context *ctx,
		    ଦଶମିକ scale,
		    colour background_colour)
{
	ଦଶମିକ transform[6];
	ସଂରଚନା svgtiny_diagram *diagram = svg->diagram;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା px, py;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	ପ୍ରତିଜ୍ଞା(diagram);

	transform[0] = (ଦଶମିକ) width / (ଦଶମିକ) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (ଦଶମିକ) height / (ଦଶମିକ) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	ପାଇଁ (i = 0; i != diagram->shape_ଗଣନା; i++) {
		ଯଦି (diagram->shape[i].path) {
			/* stroke style */
			ଯଦି (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} ଅନ୍ୟଥା {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			ଯଦି (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} ଅନ୍ୟଥା {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}

		} ଅନ୍ୟଥା ଯଦି (diagram->shape[i].text) {
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
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		}
	}

#undef BGR

	ଫେରନ୍ତୁ true;
}


ସ୍ଥିର bool
svg_redraw_tiled_internal(svg_content *svg,
			  ସଂରଚନା content_redraw_data *data,
			  ଅଚଳ ସଂରଚନା rect *clip,
			  ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	ଯଦି (data->repeat_x) {
		ପାଇଁ (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} ଅନ୍ୟଥା {
		x1 = x + 1;
	}
	ଯଦି (data->repeat_y) {
		ପାଇଁ (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} ଅନ୍ୟଥା {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	ପାଇଁ (y = y0; y < y1; y += data->height) {
		ପାଇଁ (x = x0; x < x1; x += data->width) {
			ଯଦି (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				ଫେରନ୍ତୁ false;
			}
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Redraw a CONTENT_SVG.
 */
ସ୍ଥିର bool
svg_redraw(ସଂରଚନା content *c,
	   ସଂରଚନା content_redraw_data *data,
	   ଅଚଳ ସଂରଚନା rect *clip,
	   ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	ଯଦି ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		ଫେରନ୍ତୁ true;
	}

	ଯଦି ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		ଫେରନ୍ତୁ svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	ଫେରନ୍ତୁ svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

ସ୍ଥିର ଶୂନ୍ୟ svg_destroy(ସଂରଚନା content *c)
{
	svg_content *svg = (svg_content *) c;

	ଯଦି (svg->diagram != ଖାଲି)
		svgtiny_free(svg->diagram);
}


ସ୍ଥିର nserror svg_clone(ଅଚଳ ସଂରଚନା content *old, ସଂରଚନା content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, ଆକାର(svg_content));
	ଯଦି (svg == ଖାଲି)
		ଫେରନ୍ତୁ NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ଫେରନ୍ତୁ error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	ଯଦି (error != NSERROR_OK) {
		content_destroy(&svg->base);
		ଫେରନ୍ତୁ error;
	}

	ଯଦି (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		ଯଦି (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			ଫେରନ୍ତୁ NSERROR_CLONE_FAILED;
		}
	}

	*newc = (ସଂରଚନା content *) svg;

	ଫେରନ୍ତୁ NSERROR_OK;
}

ସ୍ଥିର content_type svg_content_type(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ CONTENT_IMAGE;
}

ସ୍ଥିର ଅଚଳ content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
