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


ಸ್ಥಿರ ಸಾಲೊಳಗೆ nserror
plot_clipped_rectangle(ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx,
		       ಸ್ಥಿರಾಂಕ plot_style_t *style,
		       ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
		       ರಚನೆ rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ಆದರೆ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ಇಲ್ಲದಿದ್ದರೆ {
		res = NSERROR_OK;
	}
	ಹಿಂತಿರುಗಿ res;
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
html_redraw_border_plot(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ side,
			ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *p,
			ಬಣ್ಣ c,
			ಎಣಿಕೆ css_border_style_e style,
			ಪೂರ್ಣಾಂಕ thickness,
			bool rectangular,
			ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
			ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಪೂರ್ಣಾಂಕ z[8]; /* Vertices of border part */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ light = side;
	plot_style_t *plot_style_bdr_ಒಳ;
	plot_style_t *plot_style_bdr_ಹೊರಹೋಗು;
	nserror res = NSERROR_OK;
	ರಚನೆ rect rect;

	ಆದರೆ (c == NS_TRANSPARENT) {
		ಹಿಂತಿರುಗಿ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_ಬಣ್ಣ = c;
	plot_style_bdr.stroke_ಅಗಲ = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_ಬಣ್ಣ = c;
	plot_style_fillbdr_dark.fill_ಬಣ್ಣ = darken_ಬಣ್ಣ(c);
	plot_style_fillbdr_light.fill_ಬಣ್ಣ = lighten_ಬಣ್ಣ(c);
	plot_style_fillbdr_ddark.fill_ಬಣ್ಣ = double_darken_ಬಣ್ಣ(c);
	plot_style_fillbdr_dlight.fill_ಬಣ್ಣ = double_lighten_ಬಣ್ಣ(c);

	ಆಯ್ಕೆ (style) {
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->ಸಾಲು(ctx, &plot_style_bdr, &rect);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಆದರೆ (rectangular || thickness == 1) {

			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ಆದರೆ ((side == ತುದಿ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ಇಲ್ಲದಿದ್ದರೆ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ಆದರೆ ((side == ಎಡ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ಇಲ್ಲದಿದ್ದರೆ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ಇಲ್ಲದಿದ್ದರೆ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ಆದರೆ (res == NSERROR_OK) {
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
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ಆದರೆ (light <= 1) {
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_dark;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_light;
		} ಇಲ್ಲದಿದ್ದರೆ {
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_light;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ಆದರೆ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ಒಳ,
						     clip,
						     &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}

			/* Second part */
			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ಹೊರಹೋಗು,
						     clip,
						     &rect);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ತುದಿ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ಒಳ,
							     clip,
							     &rect);
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ಎಡ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ಹೊರಹೋಗು,
							     clip,
							     &rect);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_ಒಳ, z, 4);
			ಆದರೆ (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_ಹೊರಹೋಗು,
							 z,
							 4);
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	ಸ್ಥಿತಿ CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		ಆಯ್ಕೆ (light) {
		ಸ್ಥಿತಿ 0:
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_light;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_dlight;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 1:
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_ddark;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_dark;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 2:
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_dark;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_ddark;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 3:
			plot_style_bdr_ಒಳ = &plot_style_fillbdr_dlight;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr_light;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			plot_style_bdr_ಒಳ = &plot_style_fillbdr;
			plot_style_bdr_ಹೊರಹೋಗು = &plot_style_fillbdr;
			ನಿಲ್ಲಿಸು;
		}

		/* Render border */
		ಆದರೆ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ಒಳ,
						     clip,
						     &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}

			/* Second part */
			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ಹೊರಹೋಗು,
						     clip,
						     &rect);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ಆದರೆ (side == ತುದಿ_2 || side == ಬಲ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ತುದಿ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ಒಳ,
							     clip,
							     &rect);
			} ಇಲ್ಲದಿದ್ದರೆ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ಎಡ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ಹೊರಹೋಗು,
							     clip,
							     &rect);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_ಒಳ, z, 4);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_ಹೊರಹೋಗು, z, 4);
		}
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ res;
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
html_redraw_borders(ರಚನೆ box *box,
		    ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ,
		    ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ,
		    ಪೂರ್ಣಾಂಕ p_ಅಗಲ,
		    ಪೂರ್ಣಾಂಕ p_ಎತ್ತರ,
		    ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
		    ದಶಮಾಂಶ scale,
		    ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sides[] = { ಎಡ_2, ಬಲ_2, ತುದಿ_2, BOTTOM };
	ಪೂರ್ಣಾಂಕ ತುದಿ = box->border[ತುದಿ_2].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಬಲ = box->border[ಬಲ_2].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ bottom = box->border[BOTTOM].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಎಡ = box->border[ಎಡ_2].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ x, y;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, side;
	ಪೂರ್ಣಾಂಕ p[8]; /* Box border vertices */
	ಪೂರ್ಣಾಂಕ z[8]; /* Border vertices */
	bool ವರ್ಗಫಲ_ಕೊನೆ_1 = false;
	bool ವರ್ಗಫಲ_ಕೊನೆ_2 = false;
	nserror res;

	x = x_ಪೋಷಕ + box->x;
	y = y_ಪೋಷಕ + box->y;

	ಆದರೆ (scale != 1.0) {
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
	p[4] = x + p_ಅಗಲ;		p[5] = y + p_ಎತ್ತರ;		/* C */
	p[6] = x + p_ಅಗಲ + ಬಲ;	p[7] = y + p_ಎತ್ತರ + bottom;	/* D */

	ಗಾಗಿ (i = 0; i != 4; i++) {
		ಬಣ್ಣ ಲಂಬಸಾಲು = 0;
		side = sides[i]; /* plot order */

		ಆದರೆ (box->border[side].ಅಗಲ == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ಮುಂದುವರಿ;
		}

		ಆಯ್ಕೆ (side) {
		ಸ್ಥಿತಿ ಎಡ_2:
			ವರ್ಗಫಲ_ಕೊನೆ_1 = (ತುದಿ == 0);
			ವರ್ಗಫಲ_ಕೊನೆ_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ಆದರೆ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
			    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= ತುದಿ;
				ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
			}
			ಆದರೆ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
			}

			ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ಲಂಬಸಾಲು,
						      box->border[side].style,
						      box->border[side].ಅಗಲ * scale,
						      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
						      clip,
						      ctx);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ ಬಲ_2:
			ವರ್ಗಫಲ_ಕೊನೆ_1 = (ತುದಿ == 0);
			ವರ್ಗಫಲ_ಕೊನೆ_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ಆದರೆ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
			    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= ತುದಿ;
				ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
			}
			ಆದರೆ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
			}

			ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ಲಂಬಸಾಲು,
						      box->border[side].style,
						      box->border[side].ಅಗಲ * scale,
						      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
						      clip,
						      ctx);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ ತುದಿ_2:
			ಆದರೆ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ಮುಂದುವರಿ;
			}

			ವರ್ಗಫಲ_ಕೊನೆ_1 = (ಎಡ == 0);
			ವರ್ಗಫಲ_ಕೊನೆ_2 = (ಬಲ == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ಆದರೆ (box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ತುದಿ_2].c == box->border[ಎಡ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ಎಡ;
				ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
			}
			ಆದರೆ (box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ತುದಿ_2].c == box->border[ಬಲ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= ಬಲ;
				ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
			}

			ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ಲಂಬಸಾಲು,
						      box->border[side].style,
						      box->border[side].ಅಗಲ * scale,
						      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
						      clip,
						      ctx);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ BOTTOM:
			ಆದರೆ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ಮುಂದುವರಿ;
			}

			ವರ್ಗಫಲ_ಕೊನೆ_1 = (ಎಡ == 0);
			ವರ್ಗಫಲ_ಕೊನೆ_2 = (ಬಲ == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ಆದರೆ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ಎಡ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ಎಡ;
				ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
			}
			ಆದರೆ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ಬಲ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= ಬಲ;
				ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
			}

			ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ಲಂಬಸಾಲು,
						      box->border[side].style,
						      box->border[side].ಅಗಲ * scale,
						      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
						      clip,
						      ctx);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(side == ತುದಿ_2 || side == BOTTOM ||
			       side == ಎಡ_2 || side == ಬಲ_2);
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ true;
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
html_redraw_inline_borders(ರಚನೆ box *box,
			   ರಚನೆ rect b,
			   ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
			   ದಶಮಾಂಶ scale,
			   bool ಮೊದಲ,
			   bool ಕೊನೆಯ,
			   ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಪೂರ್ಣಾಂಕ ತುದಿ = box->border[ತುದಿ_2].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಬಲ = box->border[ಬಲ_2].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ bottom = box->border[BOTTOM].ಅಗಲ;
	ಪೂರ್ಣಾಂಕ ಎಡ = box->border[ಎಡ_2].ಅಗಲ;
	ಬಣ್ಣ ಲಂಬಸಾಲು;
	ಪೂರ್ಣಾಂಕ p[8]; /* Box border vertices */
	ಪೂರ್ಣಾಂಕ z[8]; /* Border vertices */
	bool ವರ್ಗಫಲ_ಕೊನೆ_1;
	bool ವರ್ಗಫಲ_ಕೊನೆ_2;
	nserror res;

	ಆದರೆ (scale != 1.0) {
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
	p[2] = ಮೊದಲ ? b.x0 + ಎಡ : b.x0;	p[3] = b.y0 + ತುದಿ;	/* B */
	p[4] = ಕೊನೆಯ ? b.x1 - ಬಲ : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ಪ್ರತಿಜ್ಞಾ(box->style);

	/* Left */
	ವರ್ಗಫಲ_ಕೊನೆ_1 = (ತುದಿ == 0);
	ವರ್ಗಫಲ_ಕೊನೆ_2 = (bottom == 0);
	ಆದರೆ (ಎಡ != 0 &&
	    ಮೊದಲ &&
	    nscss_color_is_transparent(box->border[ಎಡ_2].c) == false) {
		ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[ಎಡ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ಆದರೆ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
		    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= ತುದಿ;
			ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
		}

		ಆದರೆ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
		}

		res = html_redraw_border_plot(ಎಡ_2,
					      z,
					      ಲಂಬಸಾಲು,
					      box->border[ಎಡ_2].style,
					      ಎಡ,
					      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
					      clip,
					      ctx);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	/* Right */
	ವರ್ಗಫಲ_ಕೊನೆ_1 = (ತುದಿ == 0);
	ವರ್ಗಫಲ_ಕೊನೆ_2 = (bottom == 0);
	ಆದರೆ (ಬಲ != 0 &&
	    ಕೊನೆಯ &&
	    nscss_color_is_transparent(box->border[ಬಲ_2].c) == false) {
		ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[ಬಲ_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ಆದರೆ (nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false &&
		    box->border[ತುದಿ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= ತುದಿ;
			ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
		}

		ಆದರೆ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
		}

		res = html_redraw_border_plot(ಬಲ_2,
					      z,
					      ಲಂಬಸಾಲು,
					      box->border[ಬಲ_2].style,
					      ಬಲ,
					      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
					      clip,
					      ctx);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	/* Top */
	ವರ್ಗಫಲ_ಕೊನೆ_1 = (ಎಡ == 0);
	ವರ್ಗಫಲ_ಕೊನೆ_2 = (ಬಲ == 0);
	ಆದರೆ (ತುದಿ != 0 &&
	    nscss_color_is_transparent(box->border[ತುದಿ_2].c) == false) {
		ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[ತುದಿ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ಆದರೆ (ಮೊದಲ &&
		    box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ತುದಿ_2].c == box->border[ಎಡ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ಎಡ;
			ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
		}

		ಆದರೆ (ಕೊನೆಯ &&
		    box->border[ತುದಿ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ತುದಿ_2].c == box->border[ಬಲ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= ಬಲ;
			ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
		}

		res = html_redraw_border_plot(ತುದಿ_2,
					      z,
					      ಲಂಬಸಾಲು,
					      box->border[ತುದಿ_2].style,
					      ತುದಿ,
					      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
					      clip,
					      ctx);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	/* Bottom */
	ವರ್ಗಫಲ_ಕೊನೆ_1 = (ಎಡ == 0);
	ವರ್ಗಫಲ_ಕೊನೆ_2 = (ಬಲ == 0);
	ಆದರೆ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		ಲಂಬಸಾಲು = nscss_color_ಗಮ್ಯ_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ಆದರೆ (ಮೊದಲ &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ಎಡ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ಎಡ;
			ವರ್ಗಫಲ_ಕೊನೆ_1 = true;
		}

		ಆದರೆ (ಕೊನೆಯ &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ಬಲ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= ಬಲ;
			ವರ್ಗಫಲ_ಕೊನೆ_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      ಲಂಬಸಾಲು,
					      box->border[BOTTOM].style,
					      bottom,
					      ವರ್ಗಫಲ_ಕೊನೆ_1 && ವರ್ಗಫಲ_ಕೊನೆ_2,
					      clip,
					      ctx);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}
