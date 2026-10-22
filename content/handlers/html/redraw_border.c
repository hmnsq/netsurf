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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/css.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"


ಸ್ಥಿರ plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ಸ್ಥಿರ plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ಸ್ಥಿರ plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ಸ್ಥಿರ plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ಸ್ಥಿರ plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ಸ್ಥಿರ plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ nserror
plot_clipped_rectangle(ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx,
		       ಧ್ರುವ plot_style_t *style,
		       ಧ್ರುವ ಸಂರಚನಾ rect *clip,
		       ಸಂರಚನಾ rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ಯದಿ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ಅನ್ಯಥಾ {
		res = NSERROR_OK;
	}
	ಪ್ರತ್ಯಾಗಮ res;
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
ಸ್ಥಿರ nserror
html_redraw_border_plot(ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ side,
			ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *p,
			colour c,
			ಗಣನಾ css_border_style_e style,
			ಪೂರ್ಣಾಙ್ಕ thickness,
			bool rectangular,
			ಧ್ರುವ ಸಂರಚನಾ rect *clip,
			ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಪೂರ್ಣಾಙ್ಕ z[8]; /* Vertices of border part */
	unsigned ಪೂರ್ಣಾಙ್ಕ light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	ಸಂರಚನಾ rect rect;

	ಯದಿ (c == NS_TRANSPARENT) {
		ಪ್ರತ್ಯಾಗಮ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	ವಿಕಲ್ಪ (style) {
	ಪಕ್ಷ CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	ಪಕ್ಷ CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->ಸಾಲು(ctx, &plot_style_bdr, &rect);
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	ಸ್ವಭಾವ:
		ಯದಿ (rectangular || thickness == 1) {

			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ಯದಿ ((side == ತುದಿ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ಅನ್ಯಥಾ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ಅನ್ಯಥಾ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ಯದಿ ((side == ಎಡ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ಅನ್ಯಥಾ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ಅನ್ಯಥಾ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ಯದಿ (res == NSERROR_OK) {
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
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	ಪಕ್ಷ CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ಯದಿ (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} ಅನ್ಯಥಾ {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ಯದಿ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ಅನ್ಯಥಾ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}

			/* Second part */
			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ಅನ್ಯಥಾ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ಅನ್ಯಥಾ ಯದಿ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ತುದಿ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ಅನ್ಯಥಾ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ಎಡ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ಅನ್ಯಥಾ {
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
			ಯದಿ (res == NSERROR_OK) {
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
		ಭಙ್ಗ;

	ಪಕ್ಷ CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	ಪಕ್ಷ CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		ವಿಕಲ್ಪ (light) {
		ಪಕ್ಷ 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			ಭಙ್ಗ;
		ಪಕ್ಷ 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			ಭಙ್ಗ;
		ಪಕ್ಷ 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			ಭಙ್ಗ;
		ಪಕ್ಷ 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			ಭಙ್ಗ;
		}

		/* Render border */
		ಯದಿ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ಅನ್ಯಥಾ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}

			/* Second part */
			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ಅನ್ಯಥಾ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ಅನ್ಯಥಾ ಯದಿ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ಯದಿ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ತುದಿ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ಅನ್ಯಥಾ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ಎಡ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ಅನ್ಯಥಾ {
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ res;
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
html_redraw_borders(ಸಂರಚನಾ box *box,
		    ಪೂರ್ಣಾಙ್ಕ x_parent,
		    ಪೂರ್ಣಾಙ್ಕ y_parent,
		    ಪೂರ್ಣಾಙ್ಕ p_width,
		    ಪೂರ್ಣಾಙ್ಕ p_height,
		    ಧ್ರುವ ಸಂರಚನಾ rect *clip,
		    ಭಿನ್ನ scale,
		    ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ sides[] = { ಎಡ_2, ಬಲ_2, ತುದಿ_2, BOTTOM };
	ಪೂರ್ಣಾಙ್ಕ ತುದಿ = box->border[ತುದಿ_2].width;
	ಪೂರ್ಣಾಙ್ಕ ಬಲ = box->border[ಬಲ_2].width;
	ಪೂರ್ಣಾಙ್ಕ bottom = box->border[BOTTOM].width;
	ಪೂರ್ಣಾಙ್ಕ ಎಡ = box->border[ಎಡ_2].width;
	ಪೂರ್ಣಾಙ್ಕ x, y;
	unsigned ಪೂರ್ಣಾಙ್ಕ i, side;
	ಪೂರ್ಣಾಙ್ಕ p[8]; /* Box border vertices */
	ಪೂರ್ಣಾಙ್ಕ z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	ಯದಿ (scale != 1.0) {
		ತುದಿ *= scale;
		ಬಲ *= scale;
		bottom *= scale;
		ಎಡ *= scale;
		x *= scale;
		y *= scale;
	}

	ಪ್ರತಿಜ್ಞಾ(box->style);

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
	p[0] = x - ಎಡ;		p[1] = y - ತುದಿ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + ಬಲ;	p[7] = y + p_height + bottom;	/* D */

	ಪ್ರತಿ (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		ಯದಿ (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ಅನುವರ್ತ;
		}

		ವಿಕಲ್ಪ (side) {
		ಪಕ್ಷ ಎಡ_2:
			square_end_1 = (ತುದಿ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ಯದಿ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
			    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= ತುದಿ;
				square_end_1 = true;
			}
			ಯದಿ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ ಬಲ_2:
			square_end_1 = (ತುದಿ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ಯದಿ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
			    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= ತುದಿ;
				square_end_1 = true;
			}
			ಯದಿ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ ತುದಿ_2:
			ಯದಿ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ಅನುವರ್ತ;
			}

			square_end_1 = (ಎಡ == 0);
			square_end_2 = (ಬಲ == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ಯದಿ (box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ತುದಿ_2].c == box->border[ಎಡ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ಎಡ;
				square_end_1 = true;
			}
			ಯದಿ (box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ತುದಿ_2].c == box->border[ಬಲ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= ಬಲ;
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ BOTTOM:
			ಯದಿ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ಅನುವರ್ತ;
			}

			square_end_1 = (ಎಡ == 0);
			square_end_2 = (ಬಲ == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ಯದಿ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ಎಡ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ಎಡ;
				square_end_1 = true;
			}
			ಯದಿ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ಬಲ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= ಬಲ;
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
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(side == ತುದಿ_2 || side == BOTTOM ||
			       side == ಎಡ_2 || side == ಬಲ_2);
			ಭಙ್ಗ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
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
html_redraw_inline_borders(ಸಂರಚನಾ box *box,
			   ಸಂರಚನಾ rect b,
			   ಧ್ರುವ ಸಂರಚನಾ rect *clip,
			   ಭಿನ್ನ scale,
			   bool first,
			   bool last,
			   ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಪೂರ್ಣಾಙ್ಕ ತುದಿ = box->border[ತುದಿ_2].width;
	ಪೂರ್ಣಾಙ್ಕ ಬಲ = box->border[ಬಲ_2].width;
	ಪೂರ್ಣಾಙ್ಕ bottom = box->border[BOTTOM].width;
	ಪೂರ್ಣಾಙ್ಕ ಎಡ = box->border[ಎಡ_2].width;
	colour col;
	ಪೂರ್ಣಾಙ್ಕ p[8]; /* Box border vertices */
	ಪೂರ್ಣಾಙ್ಕ z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	ಯದಿ (scale != 1.0) {
		ತುದಿ *= scale;
		ಬಲ *= scale;
		bottom *= scale;
		ಎಡ *= scale;
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
	p[2] = first ? b.x0 + ಎಡ : b.x0;	p[3] = b.y0 + ತುದಿ;	/* B */
	p[4] = last ? b.x1 - ಬಲ : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ಪ್ರತಿಜ್ಞಾ(box->style);

	/* Left */
	square_end_1 = (ತುದಿ == 0);
	square_end_2 = (bottom == 0);
	ಯದಿ (ಎಡ != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[ಎಡ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ಎಡ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ಯದಿ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
		    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= ತುದಿ;
			square_end_1 = true;
		}

		ಯದಿ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ಎಡ_2,
					      z,
					      col,
					      box->border[ಎಡ_2].style,
					      ಎಡ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* Right */
	square_end_1 = (ತುದಿ == 0);
	square_end_2 = (bottom == 0);
	ಯದಿ (ಬಲ != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[ಬಲ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ಬಲ_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ಯದಿ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
		    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= ತುದಿ;
			square_end_1 = true;
		}

		ಯದಿ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ಬಲ_2,
					      z,
					      col,
					      box->border[ಬಲ_2].style,
					      ಬಲ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* Top */
	square_end_1 = (ಎಡ == 0);
	square_end_2 = (ಬಲ == 0);
	ಯದಿ (ತುದಿ != 0 &&
	    nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ತುದಿ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ಯದಿ (first &&
		    box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ತುದಿ_2].c == box->border[ಎಡ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ಎಡ;
			square_end_1 = true;
		}

		ಯದಿ (last &&
		    box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ತುದಿ_2].c == box->border[ಬಲ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= ಬಲ;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ತುದಿ_2,
					      z,
					      col,
					      box->border[ತುದಿ_2].style,
					      ತುದಿ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* Bottom */
	square_end_1 = (ಎಡ == 0);
	square_end_2 = (ಬಲ == 0);
	ಯದಿ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ಯದಿ (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ಎಡ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ಎಡ;
			square_end_1 = true;
		}

		ಯದಿ (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ಬಲ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= ಬಲ;
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
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}
