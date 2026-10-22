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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/css.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/private.h"


સ્થિર plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
સ્થિર plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
સ્થિર plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
સ્થિર plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
સ્થિર plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
સ્થિર plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


સ્થિર અંતઃરેખ nserror
plot_clipped_rectangle(અચલ સંરચના redraw_context *ctx,
		       અચલ plot_style_t *style,
		       અચલ સંરચના rect *clip,
		       સંરચના rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	જો ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} નહીંતર {
		res = NSERROR_OK;
	}
	પાછા res;
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
સ્થિર nserror
html_redraw_border_plot(અચલ પૂર્ણાંક side,
			અચલ પૂર્ણાંક *p,
			colour c,
			ગણના css_border_style_e style,
			પૂર્ણાંક thickness,
			bool rectangular,
			અચલ સંરચના rect *clip,
			અચલ સંરચના redraw_context *ctx)
{
	પૂર્ણાંક z[8]; /* Vertices of border part */
	unsigned પૂર્ણાંક light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	સંરચના rect rect;

	જો (c == NS_TRANSPARENT) {
		પાછા res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	પસંદગી (style) {
	સ્થિતિ CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->લીટી(ctx, &plot_style_bdr, &rect);
		તોડો;

	સ્થિતિ CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	મૂળભૂત:
		જો (rectangular || thickness == 1) {

			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				જો ((side == ટોચ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} નહીંતર {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} નહીંતર {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				જો ((side == ડાબું_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} નહીંતર {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} નહીંતર {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		તોડો;

	સ્થિતિ CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		જો (res == NSERROR_OK) {
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
		તોડો;

	સ્થિતિ CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		જો (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} નહીંતર {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		જો ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} નહીંતર {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			જો (res != NSERROR_OK) {
				પાછા res;
			}

			/* Second part */
			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} નહીંતર {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} નહીંતર જો (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ટોચ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} નહીંતર {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ડાબું_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} નહીંતર {
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
			જો (res == NSERROR_OK) {
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
		તોડો;

	સ્થિતિ CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	સ્થિતિ CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		પસંદગી (light) {
		સ્થિતિ 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			તોડો;
		સ્થિતિ 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			તોડો;
		સ્થિતિ 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			તોડો;
		સ્થિતિ 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			તોડો;
		મૂળભૂત:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			તોડો;
		}

		/* Render border */
		જો ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} નહીંતર {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			જો (res != NSERROR_OK) {
				પાછા res;
			}

			/* Second part */
			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} નહીંતર {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} નહીંતર જો (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			જો (side == ટોચ_2 || side == જમણું_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ટોચ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} નહીંતર {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ડાબું_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} નહીંતર {
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
			જો (res != NSERROR_OK) {
				પાછા res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		તોડો;
	}

	પાછા res;
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
html_redraw_borders(સંરચના box *box,
		    પૂર્ણાંક x_parent,
		    પૂર્ણાંક y_parent,
		    પૂર્ણાંક p_width,
		    પૂર્ણાંક p_height,
		    અચલ સંરચના rect *clip,
		    દશાંશ scale,
		    અચલ સંરચના redraw_context *ctx)
{
	unsigned પૂર્ણાંક sides[] = { ડાબું_2, જમણું_2, ટોચ_2, BOTTOM };
	પૂર્ણાંક ટોચ = box->border[ટોચ_2].width;
	પૂર્ણાંક જમણું = box->border[જમણું_2].width;
	પૂર્ણાંક bottom = box->border[BOTTOM].width;
	પૂર્ણાંક ડાબું = box->border[ડાબું_2].width;
	પૂર્ણાંક x, y;
	unsigned પૂર્ણાંક i, side;
	પૂર્ણાંક p[8]; /* Box border vertices */
	પૂર્ણાંક z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	જો (scale != 1.0) {
		ટોચ *= scale;
		જમણું *= scale;
		bottom *= scale;
		ડાબું *= scale;
		x *= scale;
		y *= scale;
	}

	પ્રતિજ્ઞા(box->style);

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
	p[0] = x - ડાબું;		p[1] = y - ટોચ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + જમણું;	p[7] = y + p_height + bottom;	/* D */

	માટે (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		જો (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ચાલુ;
		}

		પસંદગી (side) {
		સ્થિતિ ડાબું_2:
			square_end_1 = (ટોચ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			જો (nscss_color_is_transparent(box->border[ટોચ_2].c) == false &&
			    box->border[ટોચ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= ટોચ;
				square_end_1 = true;
			}
			જો (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			જો (res != NSERROR_OK) {
				પાછા false;
			}
			તોડો;

		સ્થિતિ જમણું_2:
			square_end_1 = (ટોચ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			જો (nscss_color_is_transparent(box->border[ટોચ_2].c) == false &&
			    box->border[ટોચ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= ટોચ;
				square_end_1 = true;
			}
			જો (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			જો (res != NSERROR_OK) {
				પાછા false;
			}
			તોડો;

		સ્થિતિ ટોચ_2:
			જો (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ચાલુ;
			}

			square_end_1 = (ડાબું == 0);
			square_end_2 = (જમણું == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			જો (box->border[ટોચ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ટોચ_2].c == box->border[ડાબું_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ડાબું;
				square_end_1 = true;
			}
			જો (box->border[ટોચ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ટોચ_2].c == box->border[જમણું_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= જમણું;
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
			જો (res != NSERROR_OK) {
				પાછા false;
			}
			તોડો;

		સ્થિતિ BOTTOM:
			જો (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ચાલુ;
			}

			square_end_1 = (ડાબું == 0);
			square_end_2 = (જમણું == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			જો (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ડાબું_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ડાબું;
				square_end_1 = true;
			}
			જો (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[જમણું_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= જમણું;
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
			જો (res != NSERROR_OK) {
				પાછા false;
			}
			તોડો;

		મૂળભૂત:
			પ્રતિજ્ઞા(side == ટોચ_2 || side == BOTTOM ||
			       side == ડાબું_2 || side == જમણું_2);
			તોડો;
		}
	}

	પાછા true;
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
html_redraw_inline_borders(સંરચના box *box,
			   સંરચના rect b,
			   અચલ સંરચના rect *clip,
			   દશાંશ scale,
			   bool first,
			   bool last,
			   અચલ સંરચના redraw_context *ctx)
{
	પૂર્ણાંક ટોચ = box->border[ટોચ_2].width;
	પૂર્ણાંક જમણું = box->border[જમણું_2].width;
	પૂર્ણાંક bottom = box->border[BOTTOM].width;
	પૂર્ણાંક ડાબું = box->border[ડાબું_2].width;
	colour col;
	પૂર્ણાંક p[8]; /* Box border vertices */
	પૂર્ણાંક z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	જો (scale != 1.0) {
		ટોચ *= scale;
		જમણું *= scale;
		bottom *= scale;
		ડાબું *= scale;
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
	p[2] = first ? b.x0 + ડાબું : b.x0;	p[3] = b.y0 + ટોચ;	/* B */
	p[4] = last ? b.x1 - જમણું : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	પ્રતિજ્ઞા(box->style);

	/* Left */
	square_end_1 = (ટોચ == 0);
	square_end_2 = (bottom == 0);
	જો (ડાબું != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[ડાબું_2].c) == false) {
		col = nscss_color_to_ns(box->border[ડાબું_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		જો (nscss_color_is_transparent(box->border[ટોચ_2].c) == false &&
		    box->border[ટોચ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= ટોચ;
			square_end_1 = true;
		}

		જો (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ડાબું_2,
					      z,
					      col,
					      box->border[ડાબું_2].style,
					      ડાબું,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}

	/* Right */
	square_end_1 = (ટોચ == 0);
	square_end_2 = (bottom == 0);
	જો (જમણું != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[જમણું_2].c) == false) {
		col = nscss_color_to_ns(box->border[જમણું_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		જો (nscss_color_is_transparent(box->border[ટોચ_2].c) == false &&
		    box->border[ટોચ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= ટોચ;
			square_end_1 = true;
		}

		જો (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(જમણું_2,
					      z,
					      col,
					      box->border[જમણું_2].style,
					      જમણું,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}

	/* Top */
	square_end_1 = (ડાબું == 0);
	square_end_2 = (જમણું == 0);
	જો (ટોચ != 0 &&
	    nscss_color_is_transparent(box->border[ટોચ_2].c) == false) {
		col = nscss_color_to_ns(box->border[ટોચ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		જો (first &&
		    box->border[ટોચ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ટોચ_2].c == box->border[ડાબું_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ડાબું;
			square_end_1 = true;
		}

		જો (last &&
		    box->border[ટોચ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ટોચ_2].c == box->border[જમણું_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= જમણું;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ટોચ_2,
					      z,
					      col,
					      box->border[ટોચ_2].style,
					      ટોચ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}

	/* Bottom */
	square_end_1 = (ડાબું == 0);
	square_end_2 = (જમણું == 0);
	જો (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		જો (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ડાબું_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ડાબું;
			square_end_1 = true;
		}

		જો (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[જમણું_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= જમણું;
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
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}

	પાછા true;
}
