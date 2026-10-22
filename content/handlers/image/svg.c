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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ <svgtiny.h>

#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/content_factory.h"

#અન્તર્ભાવ "image/svg.h"

પ્રકારવ્યાખ્યા સંરચના svg_content {
	સંરચના content base;

	સંરચના svgtiny_diagram *diagram;

	પૂર્ણાંક current_width;
	પૂર્ણાંક current_height;
} svg_content;



સ્થિર nserror svg_create_svg_data(svg_content *c)
{
	c->diagram = svgtiny_create();
	જો (c->diagram == ખાલી)
		જાઓ no_memory;

	c->current_width = INT_MAX;
	c->current_height = INT_MAX;

	પાછા NSERROR_OK;

no_memory:
	content_broadcast_error(&c->base, NSERROR_NOMEM, ખાલી);
	પાછા NSERROR_NOMEM;
}


/**
 * Create a CONTENT_SVG.
 */

સ્થિર nserror svg_create(અચલ content_handler *handler,
		lwc_string *imime_type, અચલ સંરચના http_parameter *params,
		સંરચના llcache_handle *llcache, અચલ અક્ષર *fallback_charset,
		bool quirks, સંરચના content **c)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, કદ(svg_content));
	જો (svg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__init(&svg->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	જો (error != NSERROR_OK) {
		મુક્ત(svg);
		પાછા error;
	}

	error = svg_create_svg_data(svg);
	જો (error != NSERROR_OK) {
		મુક્ત(svg);
		પાછા error;
	}

	*c = (સંરચના content *) svg;

	પાછા NSERROR_OK;
}



/**
 * Convert a CONTENT_SVG for display.
 */

સ્થિર bool svg_convert(સંરચના content *c)
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

	પાછા true;
}

/**
 * Reformat a CONTENT_SVG.
 */

સ્થિર શૂન્ય svg_reformat(સંરચના content *c, પૂર્ણાંક width, પૂર્ણાંક height)
{
	svg_content *svg = (svg_content *) c;
	અચલ uint8_t *source_data;
	size_t source_size;

	પ્રતિજ્ઞા(svg->diagram);

	/* Avoid reformats to same width/height as we already reformatted to */
	જો (width != svg->current_width || height != svg->current_height) {
		source_data = content__get_source_data(c, &source_size);

		svgtiny_parse(svg->diagram,
			      (અચલ અક્ષર *)source_data,
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

સ્થિર bool
svg_redraw_internal(svg_content *svg,
		    પૂર્ણાંક x,
		    પૂર્ણાંક y,
		    પૂર્ણાંક width,
		    પૂર્ણાંક height,
		    અચલ સંરચના rect *clip,
		    અચલ સંરચના redraw_context *ctx,
		    દશાંશ scale,
		    colour background_colour)
{
	દશાંશ transform[6];
	સંરચના svgtiny_diagram *diagram = svg->diagram;
	પૂર્ણાંક px, py;
	unsigned પૂર્ણાંક i;
	plot_font_style_t fstyle = *plot_style_font;
	plot_style_t pstyle;
	nserror res;

	પ્રતિજ્ઞા(diagram);

	transform[0] = (દશાંશ) width / (દશાંશ) svg->base.width;
	transform[1] = 0;
	transform[2] = 0;
	transform[3] = (દશાંશ) height / (દશાંશ) svg->base.height;
	transform[4] = x;
	transform[5] = y;

#define BGR(c) (((svgtiny_RED((c))) |					\
		 (svgtiny_GREEN((c)) << 8) |				\
		 (svgtiny_BLUE((c)) << 16)))

	માટે (i = 0; i != diagram->shape_ગણતરી; i++) {
		જો (diagram->shape[i].path) {
			/* stroke style */
			જો (diagram->shape[i].stroke == svgtiny_TRANSPARENT) {
				pstyle.stroke_type = PLOT_OP_TYPE_NONE;
				pstyle.stroke_colour = NS_TRANSPARENT;
			} નહીંતર {
				pstyle.stroke_type = PLOT_OP_TYPE_SOLID;
				pstyle.stroke_colour = BGR(diagram->shape[i].stroke);
			}
			pstyle.stroke_width = plot_style_int_to_fixed(
					diagram->shape[i].stroke_width);

			/* fill style */
			જો (diagram->shape[i].fill == svgtiny_TRANSPARENT) {
				pstyle.fill_type = PLOT_OP_TYPE_NONE;
				pstyle.fill_colour = NS_TRANSPARENT;
			} નહીંતર {
				pstyle.fill_type = PLOT_OP_TYPE_SOLID;
				pstyle.fill_colour = BGR(diagram->shape[i].fill);
			}

			/* draw the path */
			res = ctx->plot->path(ctx,
					&pstyle,
					diagram->shape[i].path,
					diagram->shape[i].path_length,
					transform);
			જો (res != NSERROR_OK) {
				પાછા false;
			}

		} નહીંતર જો (diagram->shape[i].text) {
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
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		}
	}

#undef BGR

	પાછા true;
}


સ્થિર bool
svg_redraw_tiled_internal(svg_content *svg,
			  સંરચના content_redraw_data *data,
			  અચલ સંરચના rect *clip,
			  અચલ સંરચના redraw_context *ctx)
{
	/* Tiled redraw required.  SVG repeats to extents of clip
	 * rectangle, in x, y or both directions */
	પૂર્ણાંક x, y, x0, y0, x1, y1;

	x = x0 = data->x;
	y = y0 = data->y;

	/* Find the redraw boundaries to loop within */
	જો (data->repeat_x) {
		માટે (; x0 > clip->x0; x0 -= data->width);
		x1 = clip->x1;
	} નહીંતર {
		x1 = x + 1;
	}
	જો (data->repeat_y) {
		માટે (; y0 > clip->y0; y0 -= data->height);
		y1 = clip->y1;
	} નહીંતર {
		y1 = y + 1;
	}

	/* Repeatedly plot the SVG across the area */
	માટે (y = y0; y < y1; y += data->height) {
		માટે (x = x0; x < x1; x += data->width) {
			જો (!svg_redraw_internal(svg, x, y,
						 data->width, data->height,
						 clip, ctx, data->scale,
						 data->background_colour)) {
				પાછા false;
			}
		}
	}

	પાછા true;
}


/**
 * Redraw a CONTENT_SVG.
 */
સ્થિર bool
svg_redraw(સંરચના content *c,
	   સંરચના content_redraw_data *data,
	   અચલ સંરચના rect *clip,
	   અચલ સંરચના redraw_context *ctx)
{
	svg_content *svg = (svg_content *)c;

	જો ((data->width <= 0) && (data->height <= 0)) {
		/* No point trying to plot SVG if it does not occupy a valid
		 * area */
		પાછા true;
	}

	જો ((data->repeat_x == false) && (data->repeat_y == false)) {
		/* Simple case: SVG is not tiled */
		પાછા svg_redraw_internal(svg, data->x, data->y,
				data->width, data->height,
				clip, ctx, data->scale,
				data->background_colour);
	}

	પાછા svg_redraw_tiled_internal(svg, data, clip, ctx);
}


/**
 * Destroy a CONTENT_SVG and free all resources it owns.
 */

સ્થિર શૂન્ય svg_destroy(સંરચના content *c)
{
	svg_content *svg = (svg_content *) c;

	જો (svg->diagram != ખાલી)
		svgtiny_free(svg->diagram);
}


સ્થિર nserror svg_clone(અચલ સંરચના content *old, સંરચના content **newc)
{
	svg_content *svg;
	nserror error;

	svg = calloc(1, કદ(svg_content));
	જો (svg == ખાલી)
		પાછા NSERROR_NOMEM;

	error = content__clone(old, &svg->base);
	જો (error != NSERROR_OK) {
		content_destroy(&svg->base);
		પાછા error;
	}

	/* Simply replay create/convert */
	error = svg_create_svg_data(svg);
	જો (error != NSERROR_OK) {
		content_destroy(&svg->base);
		પાછા error;
	}

	જો (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		જો (svg_convert(&svg->base) == false) {
			content_destroy(&svg->base);
			પાછા NSERROR_CLONE_FAILED;
		}
	}

	*newc = (સંરચના content *) svg;

	પાછા NSERROR_OK;
}

સ્થિર content_type svg_content_type(શૂન્ય)
{
	પાછા CONTENT_IMAGE;
}

સ્થિર અચલ content_handler svg_content_handler = {
	.create = svg_create,
	.data_complete = svg_convert,
	.reformat = svg_reformat,
	.destroy = svg_destroy,
	.redraw = svg_redraw,
	.clone = svg_clone,
	.type = svg_content_type,
	.no_share = true
};

સ્થિર અચલ અક્ષર *svg_types[] = {
	"image/svg",
	"image/svg+xml"
};


CONTENT_FACTORY_REGISTER_TYPES(svg, svg_types, svg_content_handler);
