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

#include <stdbool.h>
#include <stdlib.h>

#include "utils/utils.h"
#include "utils/log.h"
#include "netsurf/plotters.h"
#include "netsurf/css.h"

#include "html/box.h"
#include "html/private.h"


static plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
static plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
static plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
static plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
static plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
static plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


static anta_hstha nserror
plot_clipped_rectangle(const sa_nracanaa redraw_context *ctx,
		       const plot_style_t *style,
		       const sa_nracanaa rect *clip,
		       sa_nracanaa rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ᤀᤡ᤺ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ᤀᤡ᤺ᤆᤡ {
		res = NSERROR_OK;
	}
	ᤀᤡᤴ res;
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
static nserror
html_redraw_border_plot(const puur_naa_nka side,
			const puur_naa_nka *p,
			colour c,
			ga_nanaa css_border_style_e style,
			puur_naa_nka thickness,
			bool rectangular,
			const sa_nracanaa rect *clip,
			const sa_nracanaa redraw_context *ctx)
{
	puur_naa_nka z[8]; /* Vertices of border part */
	unsigned puur_naa_nka light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	sa_nracanaa rect rect;

	ᤀᤡ᤺ (c == NS_TRANSPARENT) {
		ᤀᤡᤴ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	switch (style) {
	pak_sa CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	pak_sa CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->line(ctx, &plot_style_bdr, &rect);
		bha_nga;

	pak_sa CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	default:
		ᤀᤡ᤺ (rectangular || thickness == 1) {

			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ᤀᤡ᤺ ((side == TOP) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ᤀᤡ᤺ᤆᤡ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ᤀᤡ᤺ ((side == LEFT) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ᤀᤡ᤺ᤆᤡ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ᤀᤡ᤺ᤆᤡ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		bha_nga;

	pak_sa CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ᤀᤡ᤺ (res == NSERROR_OK) {
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
		bha_nga;

	pak_sa CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	pak_sa CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ᤀᤡ᤺ (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} ᤀᤡ᤺ᤆᤡ {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ᤀᤡ᤺ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ res;
			}

			/* Second part */
			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == TOP) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == LEFT) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ᤀᤡ᤺ᤆᤡ {
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
			ᤀᤡ᤺ (res == NSERROR_OK) {
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
		bha_nga;

	pak_sa CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	pak_sa CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		switch (light) {
		pak_sa 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			bha_nga;
		pak_sa 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			bha_nga;
		pak_sa 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			bha_nga;
		pak_sa 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			bha_nga;
		default:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			bha_nga;
		}

		/* Render border */
		ᤀᤡ᤺ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ res;
			}

			/* Second part */
			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ᤀᤡ᤺ (side == TOP || side == RIGHT) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == TOP) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} ᤀᤡ᤺ᤆᤡ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == LEFT) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} ᤀᤡ᤺ᤆᤡ {
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
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		bha_nga;
	}

	ᤀᤡᤴ res;
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
html_redraw_borders(sa_nracanaa box *box,
		    puur_naa_nka x_parent,
		    puur_naa_nka y_parent,
		    puur_naa_nka p_width,
		    puur_naa_nka p_height,
		    const sa_nracanaa rect *clip,
		    float scale,
		    const sa_nracanaa redraw_context *ctx)
{
	unsigned puur_naa_nka sides[] = { LEFT, RIGHT, TOP, BOTTOM };
	puur_naa_nka top = box->border[TOP].width;
	puur_naa_nka right = box->border[RIGHT].width;
	puur_naa_nka bottom = box->border[BOTTOM].width;
	puur_naa_nka left = box->border[LEFT].width;
	puur_naa_nka x, y;
	unsigned puur_naa_nka i, side;
	puur_naa_nka p[8]; /* Box border vertices */
	puur_naa_nka z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	ᤀᤡ᤺ (scale != 1.0) {
		top *= scale;
		right *= scale;
		bottom *= scale;
		left *= scale;
		x *= scale;
		y *= scale;
	}

	assert(box->style);

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
	p[0] = x - left;		p[1] = y - top;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + right;	p[7] = y + p_height + bottom;	/* D */

	for (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		ᤀᤡ᤺ (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			continue;
		}

		switch (side) {
		pak_sa LEFT:
			square_end_1 = (top == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ᤀᤡ᤺ (nscss_color_is_transparent(box->border[TOP].c) == false &&
			    box->border[TOP].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= top;
				square_end_1 = true;
			}
			ᤀᤡ᤺ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ false;
			}
			bha_nga;

		pak_sa RIGHT:
			square_end_1 = (top == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ᤀᤡ᤺ (nscss_color_is_transparent(box->border[TOP].c) == false &&
			    box->border[TOP].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= top;
				square_end_1 = true;
			}
			ᤀᤡ᤺ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ false;
			}
			bha_nga;

		pak_sa TOP:
			ᤀᤡ᤺ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				continue;
			}

			square_end_1 = (left == 0);
			square_end_2 = (right == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ᤀᤡ᤺ (box->border[TOP].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[TOP].c == box->border[LEFT].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += left;
				square_end_1 = true;
			}
			ᤀᤡ᤺ (box->border[TOP].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[TOP].c == box->border[RIGHT].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= right;
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
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ false;
			}
			bha_nga;

		pak_sa BOTTOM:
			ᤀᤡ᤺ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				continue;
			}

			square_end_1 = (left == 0);
			square_end_2 = (right == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ᤀᤡ᤺ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[LEFT].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += left;
				square_end_1 = true;
			}
			ᤀᤡ᤺ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[RIGHT].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= right;
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
			ᤀᤡ᤺ (res != NSERROR_OK) {
				ᤀᤡᤴ false;
			}
			bha_nga;

		default:
			assert(side == TOP || side == BOTTOM ||
			       side == LEFT || side == RIGHT);
			bha_nga;
		}
	}

	ᤀᤡᤴ true;
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
html_redraw_inline_borders(sa_nracanaa box *box,
			   sa_nracanaa rect b,
			   const sa_nracanaa rect *clip,
			   float scale,
			   bool first,
			   bool last,
			   const sa_nracanaa redraw_context *ctx)
{
	puur_naa_nka top = box->border[TOP].width;
	puur_naa_nka right = box->border[RIGHT].width;
	puur_naa_nka bottom = box->border[BOTTOM].width;
	puur_naa_nka left = box->border[LEFT].width;
	colour col;
	puur_naa_nka p[8]; /* Box border vertices */
	puur_naa_nka z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	ᤀᤡ᤺ (scale != 1.0) {
		top *= scale;
		right *= scale;
		bottom *= scale;
		left *= scale;
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
	p[2] = first ? b.x0 + left : b.x0;	p[3] = b.y0 + top;	/* B */
	p[4] = last ? b.x1 - right : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	assert(box->style);

	/* Left */
	square_end_1 = (top == 0);
	square_end_2 = (bottom == 0);
	ᤀᤡ᤺ (left != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[LEFT].c) == false) {
		col = nscss_color_to_ns(box->border[LEFT].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ᤀᤡ᤺ (nscss_color_is_transparent(box->border[TOP].c) == false &&
		    box->border[TOP].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= top;
			square_end_1 = true;
		}

		ᤀᤡ᤺ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(LEFT,
					      z,
					      col,
					      box->border[LEFT].style,
					      left,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ᤀᤡ᤺ (res != NSERROR_OK) {
			ᤀᤡᤴ false;
		}
	}

	/* Right */
	square_end_1 = (top == 0);
	square_end_2 = (bottom == 0);
	ᤀᤡ᤺ (right != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[RIGHT].c) == false) {
		col = nscss_color_to_ns(box->border[RIGHT].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ᤀᤡ᤺ (nscss_color_is_transparent(box->border[TOP].c) == false &&
		    box->border[TOP].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= top;
			square_end_1 = true;
		}

		ᤀᤡ᤺ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(RIGHT,
					      z,
					      col,
					      box->border[RIGHT].style,
					      right,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ᤀᤡ᤺ (res != NSERROR_OK) {
			ᤀᤡᤴ false;
		}
	}

	/* Top */
	square_end_1 = (left == 0);
	square_end_2 = (right == 0);
	ᤀᤡ᤺ (top != 0 &&
	    nscss_color_is_transparent(box->border[TOP].c) == false) {
		col = nscss_color_to_ns(box->border[TOP].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ᤀᤡ᤺ (first &&
		    box->border[TOP].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[TOP].c == box->border[LEFT].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += left;
			square_end_1 = true;
		}

		ᤀᤡ᤺ (last &&
		    box->border[TOP].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[TOP].c == box->border[RIGHT].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= right;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(TOP,
					      z,
					      col,
					      box->border[TOP].style,
					      top,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		ᤀᤡ᤺ (res != NSERROR_OK) {
			ᤀᤡᤴ false;
		}
	}

	/* Bottom */
	square_end_1 = (left == 0);
	square_end_2 = (right == 0);
	ᤀᤡ᤺ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ᤀᤡ᤺ (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[LEFT].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += left;
			square_end_1 = true;
		}

		ᤀᤡ᤺ (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[RIGHT].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= right;
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
		ᤀᤡ᤺ (res != NSERROR_OK) {
			ᤀᤡᤴ false;
		}
	}

	ᤀᤡᤴ true;
}
