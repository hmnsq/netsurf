/*
 * Copyright 2017 Vincent Sanders <vince@netsurf-browser.org>
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
 *
 * Redrawing CONTENT_HTML borders implementation.
 */

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/css.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"


ସ୍ଥିର plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ସ୍ଥିର plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ସ୍ଥିର plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ସ୍ଥିର plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ସ୍ଥିର plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ସ୍ଥିର plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ସ୍ଥିର ଧାଡ଼ିଭିତରେ nserror
plot_clipped_rectangle(ଅଚଳ ସଂରଚନା redraw_context *ctx,
		       ଅଚଳ plot_style_t *style,
		       ଅଚଳ ସଂରଚନା rect *clip,
		       ସଂରଚନା rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ଯଦି ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ଅନ୍ୟଥା {
		res = NSERROR_OK;
	}
	ଫେରନ୍ତୁ res;
}


/**
 * Draw one border.
 *
 * \param side index of border side (TOP, RIGHT, BOTTOM, LEFT)
 * \param p array of precomputed border vertices
 * \param c colour for border
 * \param style border line style
 * \param thickness border thickness
 * \param rectangular whether border is rectangular
 * \param clip cliping area for redrawing border.
 * \param ctx current redraw context
 * \return NSERROR_OK if successful otherwise appropriate error code
 */
ସ୍ଥିର nserror
html_redraw_border_plot(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା side,
			ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *p,
			colour c,
			ଗଣନା css_border_style_e style,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା thickness,
			bool rectangular,
			ଅଚଳ ସଂରଚନା rect *clip,
			ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା z[8]; /* Vertices of border part */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	ସଂରଚନା rect rect;

	ଯଦି (c == NS_TRANSPARENT) {
		ଫେରନ୍ତୁ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	ବାଛ (style) {
	ସ୍ଥିତି CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->ଧାଡ଼ି(ctx, &plot_style_bdr, &rect);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଯଦି (rectangular || thickness == 1) {

			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ଯଦି ((side == ଶୀର୍ଷ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ଅନ୍ୟଥା {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ଅନ୍ୟଥା {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ଯଦି ((side == ବାମ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ଅନ୍ୟଥା {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ଅନ୍ୟଥା {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ଯଦି (res == NSERROR_OK) {
			z[0] = p[2];
			z[1] = p[3];
			z[2] = (p[2] * 2 + p[0]) / 3;
			z[3] = (p[3] * 2 + p[1]) / 3;
			z[4] = (p[4] * 2 + p[6]) / 3;
			z[5] = (p[5] * 2 + p[7]) / 3;
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ଯଦି (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} ଅନ୍ୟଥା {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ଯଦି ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ଅନ୍ୟଥା {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}

			/* Second part */
			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ଅନ୍ୟଥା {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ଅନ୍ୟଥା ଯଦି (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ଶୀର୍ଷ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ଅନ୍ୟଥା {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ବାମ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ଅନ୍ୟଥା {
			/* Border made up from two parts and can't be
			 * plotted with rectangles
			 */
			z[0] = p[0];
			z[1] = p[1];
			z[2] = (p[0] + p[2]) / 2;
			z[3] = (p[1] + p[3]) / 2;
			z[4] = (p[6] + p[4]) / 2;
			z[5] = (p[7] + p[5]) / 2;
			z[6] = p[6];
			z[7] = p[7];
			res = ctx->plot->polygon(ctx, plot_style_bdr_in, z, 4);
			ଯଦି (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_out,
							 z,
							 4);
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	ସ୍ଥିତି CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		ବାଛ (light) {
		ସ୍ଥିତି 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			ଭାଙ୍ଗ;
		}

		/* Render border */
		ଯଦି ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ଅନ୍ୟଥା {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}

			/* Second part */
			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ଅନ୍ୟଥା {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ଅନ୍ୟଥା ଯଦି (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ଯଦି (side == ଶୀର୍ଷ_2 || side == ଡାହାଣ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ଶୀର୍ଷ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ଅନ୍ୟଥା {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ବାମ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ଅନ୍ୟଥା {
			/* Border made up from two parts and can't be
			 * plotted with rectangles
			 */

			z[0] = p[0];
			z[1] = p[1];
			z[2] = (p[0] + p[2]) / 2;
			z[3] = (p[1] + p[3]) / 2;
			z[4] = (p[6] + p[4]) / 2;
			z[5] = (p[7] + p[5]) / 2;
			z[6] = p[6];
			z[7] = p[7];
			res = ctx->plot->polygon(ctx, plot_style_bdr_in, z, 4);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ res;
}


/**
 * Draw borders for a box.
 *
 * \param box box to draw
 * \param x_parent coordinate of left padding edge of parent of box
 * \param y_parent coordinate of top padding edge of parent of box
 * \param p_width width of padding box
 * \param p_height height of padding box
 * \param clip cliping area for redrawing border.
 * \param scale scale for redraw
 * \param ctx current redraw context
 * \return true if successful, false otherwise
 */
bool
html_redraw_borders(ସଂରଚନା box *box,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_width,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା p_height,
		    ଅଚଳ ସଂରଚନା rect *clip,
		    ଦଶମିକ scale,
		    ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା sides[] = { ବାମ_2, ଡାହାଣ_2, ଶୀର୍ଷ_2, BOTTOM };
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ = box->border[ଶୀର୍ଷ_2].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଡାହାଣ = box->border[ଡାହାଣ_2].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bottom = box->border[BOTTOM].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ବାମ = box->border[ବାମ_2].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, side;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା p[8]; /* Box border vertices */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	ଯଦି (scale != 1.0) {
		ଶୀର୍ଷ *= scale;
		ଡାହାଣ *= scale;
		bottom *= scale;
		ବାମ *= scale;
		x *= scale;
		y *= scale;
	}

	ପ୍ରତିଜ୍ଞା(box->style);

	/* Calculate border vertices
	 *
	 *    A----------------------+
	 *    | \                  / |
	 *    |   B--------------+   |
	 *    |   |              |   |
	 *    |   +--------------C   |
	 *    | /                  \ |
	 *    +----------------------D
	 */
	p[0] = x - ବାମ;		p[1] = y - ଶୀର୍ଷ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + ଡାହାଣ;	p[7] = y + p_height + bottom;	/* D */

	ପାଇଁ (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		ଯଦି (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ଜାରି;
		}

		ବାଛ (side) {
		ସ୍ଥିତି ବାମ_2:
			square_end_1 = (ଶୀର୍ଷ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ଯଦି (nscss_color_is_transparent(box->border[ଶୀର୍ଷ_2].c) == false &&
			    box->border[ଶୀର୍ଷ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= ଶୀର୍ଷ;
				square_end_1 = true;
			}
			ଯଦି (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				square_end_2 = true;
			}

			col = nscss_color_to_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      col,
						      box->border[side].style,
						      box->border[side].width * scale,
						      square_end_1 && square_end_2,
						      clip,
						      ctx);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି ଡାହାଣ_2:
			square_end_1 = (ଶୀର୍ଷ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ଯଦି (nscss_color_is_transparent(box->border[ଶୀର୍ଷ_2].c) == false &&
			    box->border[ଶୀର୍ଷ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= ଶୀର୍ଷ;
				square_end_1 = true;
			}
			ଯଦି (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				square_end_2 = true;
			}

			col = nscss_color_to_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      col,
						      box->border[side].style,
						      box->border[side].width * scale,
						      square_end_1 && square_end_2,
						      clip,
						      ctx);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି ଶୀର୍ଷ_2:
			ଯଦି (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ଜାରି;
			}

			square_end_1 = (ବାମ == 0);
			square_end_2 = (ଡାହାଣ == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ଯଦି (box->border[ଶୀର୍ଷ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ଶୀର୍ଷ_2].c == box->border[ବାମ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ବାମ;
				square_end_1 = true;
			}
			ଯଦି (box->border[ଶୀର୍ଷ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ଶୀର୍ଷ_2].c == box->border[ଡାହାଣ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= ଡାହାଣ;
				square_end_2 = true;
			}

			col = nscss_color_to_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      col,
						      box->border[side].style,
						      box->border[side].width * scale,
						      square_end_1 && square_end_2,
						      clip,
						      ctx);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି BOTTOM:
			ଯଦି (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ଜାରି;
			}

			square_end_1 = (ବାମ == 0);
			square_end_2 = (ଡାହାଣ == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ଯଦି (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ବାମ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ବାମ;
				square_end_1 = true;
			}
			ଯଦି (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ଡାହାଣ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= ଡାହାଣ;
				square_end_2 = true;
			}

			col = nscss_color_to_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      col,
						      box->border[side].style,
						      box->border[side].width * scale,
						      square_end_1 && square_end_2,
						      clip,
						      ctx);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(side == ଶୀର୍ଷ_2 || side == BOTTOM ||
			       side == ବାମ_2 || side == ଡାହାଣ_2);
			ଭାଙ୍ଗ;
		}
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Draw an inline's borders.
 *
 * \param box BOX_INLINE which created the border
 * \param b coordinates of border edge rectangle
 * \param clip cliping area for redrawing border.
 * \param scale scale for redraw
 * \param first true if this is the first rectangle associated with the inline
 * \param last true if this is the last rectangle associated with the inline
 * \param ctx current redraw context
 * \return true if successful, false otherwise
 */
bool
html_redraw_inline_borders(ସଂରଚନା box *box,
			   ସଂରଚନା rect b,
			   ଅଚଳ ସଂରଚନା rect *clip,
			   ଦଶମିକ scale,
			   bool first,
			   bool last,
			   ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ = box->border[ଶୀର୍ଷ_2].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଡାହାଣ = box->border[ଡାହାଣ_2].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bottom = box->border[BOTTOM].width;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ବାମ = box->border[ବାମ_2].width;
	colour col;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା p[8]; /* Box border vertices */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	ଯଦି (scale != 1.0) {
		ଶୀର୍ଷ *= scale;
		ଡାହାଣ *= scale;
		bottom *= scale;
		ବାମ *= scale;
	}

	/* Calculate border vertices
	 *
	 *    A----------------------+
	 *    | \                  / |
	 *    |   B--------------+   |
	 *    |   |              |   |
	 *    |   +--------------C   |
	 *    | /                  \ |
	 *    +----------------------D
	 */
	p[0] = b.x0;				p[1] = b.y0;		/* A */
	p[2] = first ? b.x0 + ବାମ : b.x0;	p[3] = b.y0 + ଶୀର୍ଷ;	/* B */
	p[4] = last ? b.x1 - ଡାହାଣ : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ପ୍ରତିଜ୍ଞା(box->style);

	/* Left */
	square_end_1 = (ଶୀର୍ଷ == 0);
	square_end_2 = (bottom == 0);
	ଯଦି (ବାମ != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[ବାମ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ବାମ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ଯଦି (nscss_color_is_transparent(box->border[ଶୀର୍ଷ_2].c) == false &&
		    box->border[ଶୀର୍ଷ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= ଶୀର୍ଷ;
			square_end_1 = true;
		}

		ଯଦି (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ବାମ_2,
					      z,
					      col,
					      box->border[ବାମ_2].style,
					      ବାମ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}

	/* Right */
	square_end_1 = (ଶୀର୍ଷ == 0);
	square_end_2 = (bottom == 0);
	ଯଦି (ଡାହାଣ != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[ଡାହାଣ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ଡାହାଣ_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ଯଦି (nscss_color_is_transparent(box->border[ଶୀର୍ଷ_2].c) == false &&
		    box->border[ଶୀର୍ଷ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= ଶୀର୍ଷ;
			square_end_1 = true;
		}

		ଯଦି (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ଡାହାଣ_2,
					      z,
					      col,
					      box->border[ଡାହାଣ_2].style,
					      ଡାହାଣ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}

	/* Top */
	square_end_1 = (ବାମ == 0);
	square_end_2 = (ଡାହାଣ == 0);
	ଯଦି (ଶୀର୍ଷ != 0 &&
	    nscss_color_is_transparent(box->border[ଶୀର୍ଷ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ଶୀର୍ଷ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ଯଦି (first &&
		    box->border[ଶୀର୍ଷ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ଶୀର୍ଷ_2].c == box->border[ବାମ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ବାମ;
			square_end_1 = true;
		}

		ଯଦି (last &&
		    box->border[ଶୀର୍ଷ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ଶୀର୍ଷ_2].c == box->border[ଡାହାଣ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= ଡାହାଣ;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ଶୀର୍ଷ_2,
					      z,
					      col,
					      box->border[ଶୀର୍ଷ_2].style,
					      ଶୀର୍ଷ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}

	/* Bottom */
	square_end_1 = (ବାମ == 0);
	square_end_2 = (ଡାହାଣ == 0);
	ଯଦି (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ଯଦି (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ବାମ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ବାମ;
			square_end_1 = true;
		}

		ଯଦି (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ଡାହାଣ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= ଡାହାଣ;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      col,
					      box->border[BOTTOM].style,
					      bottom,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
}
