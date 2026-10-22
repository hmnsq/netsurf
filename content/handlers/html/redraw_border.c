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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/css.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/private.h"


സ്ഥിതം plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
സ്ഥിതം plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
സ്ഥിതം plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
സ്ഥിതം plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
സ്ഥിതം plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
സ്ഥിതം plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


സ്ഥിതം വരിയിൽ nserror
plot_clipped_rectangle(സ്ഥിരം ഘടന redraw_context *ctx,
		       സ്ഥിരം plot_style_t *style,
		       സ്ഥിരം ഘടന rect *clip,
		       ഘടന rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	എങ്കിൽ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} അല്ലെങ്കിൽ {
		res = NSERROR_OK;
	}
	തിരികെ res;
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
സ്ഥിതം nserror
html_redraw_border_plot(സ്ഥിരം പൂർണ്ണസംഖ്യ side,
			സ്ഥിരം പൂർണ്ണസംഖ്യ *p,
			colour c,
			എണ്ണൽ css_border_style_e style,
			പൂർണ്ണസംഖ്യ thickness,
			bool rectangular,
			സ്ഥിരം ഘടന rect *clip,
			സ്ഥിരം ഘടന redraw_context *ctx)
{
	പൂർണ്ണസംഖ്യ z[8]; /* Vertices of border part */
	unsigned പൂർണ്ണസംഖ്യ light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	ഘടന rect rect;

	എങ്കിൽ (c == NS_TRANSPARENT) {
		തിരികെ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	തിരഞ്ഞെടുപ്പ് (style) {
	അവസ്ഥ CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->വരി(ctx, &plot_style_bdr, &rect);
		നിർത്തുക;

	അവസ്ഥ CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	സ്ഥിരസ്ഥിതി:
		എങ്കിൽ (rectangular || thickness == 1) {

			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				എങ്കിൽ ((side == മുകൾ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} അല്ലെങ്കിൽ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} അല്ലെങ്കിൽ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				എങ്കിൽ ((side == ഇടത്_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} അല്ലെങ്കിൽ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} അല്ലെങ്കിൽ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		നിർത്തുക;

	അവസ്ഥ CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		എങ്കിൽ (res == NSERROR_OK) {
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
		നിർത്തുക;

	അവസ്ഥ CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		എങ്കിൽ (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} അല്ലെങ്കിൽ {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		എങ്കിൽ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} അല്ലെങ്കിൽ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}

			/* Second part */
			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} അല്ലെങ്കിൽ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} അല്ലെങ്കിൽ എങ്കിൽ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == മുകൾ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} അല്ലെങ്കിൽ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ഇടത്_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} അല്ലെങ്കിൽ {
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
			എങ്കിൽ (res == NSERROR_OK) {
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
		നിർത്തുക;

	അവസ്ഥ CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	അവസ്ഥ CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		തിരഞ്ഞെടുപ്പ് (light) {
		അവസ്ഥ 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			നിർത്തുക;
		അവസ്ഥ 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			നിർത്തുക;
		അവസ്ഥ 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			നിർത്തുക;
		അവസ്ഥ 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			നിർത്തുക;
		}

		/* Render border */
		എങ്കിൽ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} അല്ലെങ്കിൽ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}

			/* Second part */
			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} അല്ലെങ്കിൽ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} അല്ലെങ്കിൽ എങ്കിൽ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			എങ്കിൽ (side == മുകൾ_2 || side == വലത്_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == മുകൾ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} അല്ലെങ്കിൽ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ഇടത്_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} അല്ലെങ്കിൽ {
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		നിർത്തുക;
	}

	തിരികെ res;
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
html_redraw_borders(ഘടന box *box,
		    പൂർണ്ണസംഖ്യ x_parent,
		    പൂർണ്ണസംഖ്യ y_parent,
		    പൂർണ്ണസംഖ്യ p_width,
		    പൂർണ്ണസംഖ്യ p_height,
		    സ്ഥിരം ഘടന rect *clip,
		    ദശാംശം scale,
		    സ്ഥിരം ഘടന redraw_context *ctx)
{
	unsigned പൂർണ്ണസംഖ്യ sides[] = { ഇടത്_2, വലത്_2, മുകൾ_2, BOTTOM };
	പൂർണ്ണസംഖ്യ മുകൾ = box->border[മുകൾ_2].width;
	പൂർണ്ണസംഖ്യ വലത് = box->border[വലത്_2].width;
	പൂർണ്ണസംഖ്യ bottom = box->border[BOTTOM].width;
	പൂർണ്ണസംഖ്യ ഇടത് = box->border[ഇടത്_2].width;
	പൂർണ്ണസംഖ്യ x, y;
	unsigned പൂർണ്ണസംഖ്യ i, side;
	പൂർണ്ണസംഖ്യ p[8]; /* Box border vertices */
	പൂർണ്ണസംഖ്യ z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	എങ്കിൽ (scale != 1.0) {
		മുകൾ *= scale;
		വലത് *= scale;
		bottom *= scale;
		ഇടത് *= scale;
		x *= scale;
		y *= scale;
	}

	പ്രതിജ്ഞാ(box->style);

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
	p[0] = x - ഇടത്;		p[1] = y - മുകൾ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + വലത്;	p[7] = y + p_height + bottom;	/* D */

	വേണ്ടി (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		എങ്കിൽ (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			തുടരുക;
		}

		തിരഞ്ഞെടുപ്പ് (side) {
		അവസ്ഥ ഇടത്_2:
			square_end_1 = (മുകൾ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			എങ്കിൽ (nscss_color_is_transparent(box->border[മുകൾ_2].c) == false &&
			    box->border[മുകൾ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= മുകൾ;
				square_end_1 = true;
			}
			എങ്കിൽ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
			നിർത്തുക;

		അവസ്ഥ വലത്_2:
			square_end_1 = (മുകൾ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			എങ്കിൽ (nscss_color_is_transparent(box->border[മുകൾ_2].c) == false &&
			    box->border[മുകൾ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= മുകൾ;
				square_end_1 = true;
			}
			എങ്കിൽ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
			നിർത്തുക;

		അവസ്ഥ മുകൾ_2:
			എങ്കിൽ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				തുടരുക;
			}

			square_end_1 = (ഇടത് == 0);
			square_end_2 = (വലത് == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			എങ്കിൽ (box->border[മുകൾ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[മുകൾ_2].c == box->border[ഇടത്_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ഇടത്;
				square_end_1 = true;
			}
			എങ്കിൽ (box->border[മുകൾ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[മുകൾ_2].c == box->border[വലത്_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= വലത്;
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
			നിർത്തുക;

		അവസ്ഥ BOTTOM:
			എങ്കിൽ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				തുടരുക;
			}

			square_end_1 = (ഇടത് == 0);
			square_end_2 = (വലത് == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			എങ്കിൽ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ഇടത്_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ഇടത്;
				square_end_1 = true;
			}
			എങ്കിൽ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[വലത്_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= വലത്;
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
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(side == മുകൾ_2 || side == BOTTOM ||
			       side == ഇടത്_2 || side == വലത്_2);
			നിർത്തുക;
		}
	}

	തിരികെ true;
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
html_redraw_inline_borders(ഘടന box *box,
			   ഘടന rect b,
			   സ്ഥിരം ഘടന rect *clip,
			   ദശാംശം scale,
			   bool first,
			   bool last,
			   സ്ഥിരം ഘടന redraw_context *ctx)
{
	പൂർണ്ണസംഖ്യ മുകൾ = box->border[മുകൾ_2].width;
	പൂർണ്ണസംഖ്യ വലത് = box->border[വലത്_2].width;
	പൂർണ്ണസംഖ്യ bottom = box->border[BOTTOM].width;
	പൂർണ്ണസംഖ്യ ഇടത് = box->border[ഇടത്_2].width;
	colour col;
	പൂർണ്ണസംഖ്യ p[8]; /* Box border vertices */
	പൂർണ്ണസംഖ്യ z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	എങ്കിൽ (scale != 1.0) {
		മുകൾ *= scale;
		വലത് *= scale;
		bottom *= scale;
		ഇടത് *= scale;
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
	p[2] = first ? b.x0 + ഇടത് : b.x0;	p[3] = b.y0 + മുകൾ;	/* B */
	p[4] = last ? b.x1 - വലത് : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	പ്രതിജ്ഞാ(box->style);

	/* Left */
	square_end_1 = (മുകൾ == 0);
	square_end_2 = (bottom == 0);
	എങ്കിൽ (ഇടത് != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[ഇടത്_2].c) == false) {
		col = nscss_color_to_ns(box->border[ഇടത്_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		എങ്കിൽ (nscss_color_is_transparent(box->border[മുകൾ_2].c) == false &&
		    box->border[മുകൾ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= മുകൾ;
			square_end_1 = true;
		}

		എങ്കിൽ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ഇടത്_2,
					      z,
					      col,
					      box->border[ഇടത്_2].style,
					      ഇടത്,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}

	/* Right */
	square_end_1 = (മുകൾ == 0);
	square_end_2 = (bottom == 0);
	എങ്കിൽ (വലത് != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[വലത്_2].c) == false) {
		col = nscss_color_to_ns(box->border[വലത്_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		എങ്കിൽ (nscss_color_is_transparent(box->border[മുകൾ_2].c) == false &&
		    box->border[മുകൾ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= മുകൾ;
			square_end_1 = true;
		}

		എങ്കിൽ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(വലത്_2,
					      z,
					      col,
					      box->border[വലത്_2].style,
					      വലത്,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}

	/* Top */
	square_end_1 = (ഇടത് == 0);
	square_end_2 = (വലത് == 0);
	എങ്കിൽ (മുകൾ != 0 &&
	    nscss_color_is_transparent(box->border[മുകൾ_2].c) == false) {
		col = nscss_color_to_ns(box->border[മുകൾ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		എങ്കിൽ (first &&
		    box->border[മുകൾ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[മുകൾ_2].c == box->border[ഇടത്_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ഇടത്;
			square_end_1 = true;
		}

		എങ്കിൽ (last &&
		    box->border[മുകൾ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[മുകൾ_2].c == box->border[വലത്_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= വലത്;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(മുകൾ_2,
					      z,
					      col,
					      box->border[മുകൾ_2].style,
					      മുകൾ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}

	/* Bottom */
	square_end_1 = (ഇടത് == 0);
	square_end_2 = (വലത് == 0);
	എങ്കിൽ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		എങ്കിൽ (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ഇടത്_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ഇടത്;
			square_end_1 = true;
		}

		എങ്കിൽ (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[വലത്_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= വലത്;
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
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}

	തിരികെ true;
}
