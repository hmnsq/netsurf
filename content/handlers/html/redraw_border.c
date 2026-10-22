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

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/css.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/private.h"


స్థిర plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
స్థిర plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
స్థిర plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
స్థిర plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
స్థిర plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
స్థిర plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


స్థిర అంతఃవరుస nserror
plot_clipped_rectangle(స్థిరరాశి నిర్మాణం redraw_context *ctx,
		       స్థిరరాశి plot_style_t *style,
		       స్థిరరాశి నిర్మాణం rect *clip,
		       నిర్మాణం rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ఒకవేళ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} లేకపోతే {
		res = NSERROR_OK;
	}
	తిరిగి res;
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
స్థిర nserror
html_redraw_border_plot(స్థిరరాశి పూర్ణాంకం side,
			స్థిరరాశి పూర్ణాంకం *p,
			రంగు c,
			గణన css_border_style_e style,
			పూర్ణాంకం thickness,
			bool rectangular,
			స్థిరరాశి నిర్మాణం rect *clip,
			స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	పూర్ణాంకం z[8]; /* Vertices of border part */
	గుర్తులేని పూర్ణాంకం light = side;
	plot_style_t *plot_style_bdr_లోపల;
	plot_style_t *plot_style_bdr_బహిర్గతం;
	nserror res = NSERROR_OK;
	నిర్మాణం rect rect;

	ఒకవేళ (c == NS_TRANSPARENT) {
		తిరిగి res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_రంగు = c;
	plot_style_bdr.stroke_వెడల్పు = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_రంగు = c;
	plot_style_fillbdr_dark.fill_రంగు = darken_రంగు(c);
	plot_style_fillbdr_light.fill_రంగు = lighten_రంగు(c);
	plot_style_fillbdr_ddark.fill_రంగు = double_darken_రంగు(c);
	plot_style_fillbdr_dlight.fill_రంగు = double_lighten_రంగు(c);

	ఎంపిక (style) {
	స్థితి CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	స్థితి CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->పంక్తి(ctx, &plot_style_bdr, &rect);
		ఆపు;

	స్థితి CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	అప్రమేయ:
		ఒకవేళ (rectangular || thickness == 1) {

			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ఒకవేళ ((side == శిఖరం_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} లేకపోతే {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} లేకపోతే {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ఒకవేళ ((side == ఎడమ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} లేకపోతే {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} లేకపోతే {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		ఆపు;

	స్థితి CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ఒకవేళ (res == NSERROR_OK) {
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
		ఆపు;

	స్థితి CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	స్థితి CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ఒకవేళ (light <= 1) {
			plot_style_bdr_లోపల = &plot_style_fillbdr_dark;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_light;
		} లేకపోతే {
			plot_style_bdr_లోపల = &plot_style_fillbdr_light;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ఒకవేళ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} లేకపోతే {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_లోపల,
						     clip,
						     &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}

			/* Second part */
			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} లేకపోతే {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_బహిర్గతం,
						     clip,
						     &rect);
		} లేకపోతే ఒకవేళ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == శిఖరం_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_లోపల,
							     clip,
							     &rect);
			} లేకపోతే {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ఎడమ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_బహిర్గతం,
							     clip,
							     &rect);
			}
		} లేకపోతే {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_లోపల, z, 4);
			ఒకవేళ (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_బహిర్గతం,
							 z,
							 4);
			}
		}
		ఆపు;

	స్థితి CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	స్థితి CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		ఎంపిక (light) {
		స్థితి 0:
			plot_style_bdr_లోపల = &plot_style_fillbdr_light;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_dlight;
			ఆపు;
		స్థితి 1:
			plot_style_bdr_లోపల = &plot_style_fillbdr_ddark;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_dark;
			ఆపు;
		స్థితి 2:
			plot_style_bdr_లోపల = &plot_style_fillbdr_dark;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_ddark;
			ఆపు;
		స్థితి 3:
			plot_style_bdr_లోపల = &plot_style_fillbdr_dlight;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr_light;
			ఆపు;
		అప్రమేయ:
			plot_style_bdr_లోపల = &plot_style_fillbdr;
			plot_style_bdr_బహిర్గతం = &plot_style_fillbdr;
			ఆపు;
		}

		/* Render border */
		ఒకవేళ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} లేకపోతే {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_లోపల,
						     clip,
						     &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}

			/* Second part */
			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} లేకపోతే {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_బహిర్గతం,
						     clip,
						     &rect);
		} లేకపోతే ఒకవేళ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ఒకవేళ (side == శిఖరం_2 || side == కుడి_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == శిఖరం_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_లోపల,
							     clip,
							     &rect);
			} లేకపోతే {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ఎడమ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_బహిర్గతం,
							     clip,
							     &rect);
			}
		} లేకపోతే {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_లోపల, z, 4);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_బహిర్గతం, z, 4);
		}
		ఆపు;
	}

	తిరిగి res;
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
html_redraw_borders(నిర్మాణం box *box,
		    పూర్ణాంకం x_మాతృ,
		    పూర్ణాంకం y_మాతృ,
		    పూర్ణాంకం p_వెడల్పు,
		    పూర్ణాంకం p_ఎత్తు,
		    స్థిరరాశి నిర్మాణం rect *clip,
		    దశాంశం scale,
		    స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	గుర్తులేని పూర్ణాంకం sides[] = { ఎడమ_2, కుడి_2, శిఖరం_2, BOTTOM };
	పూర్ణాంకం శిఖరం = box->border[శిఖరం_2].వెడల్పు;
	పూర్ణాంకం కుడి = box->border[కుడి_2].వెడల్పు;
	పూర్ణాంకం bottom = box->border[BOTTOM].వెడల్పు;
	పూర్ణాంకం ఎడమ = box->border[ఎడమ_2].వెడల్పు;
	పూర్ణాంకం x, y;
	గుర్తులేని పూర్ణాంకం i, side;
	పూర్ణాంకం p[8]; /* Box border vertices */
	పూర్ణాంకం z[8]; /* Border vertices */
	bool వర్గం_చివర_1 = false;
	bool వర్గం_చివర_2 = false;
	nserror res;

	x = x_మాతృ + box->x;
	y = y_మాతృ + box->y;

	ఒకవేళ (scale != 1.0) {
		శిఖరం *= scale;
		కుడి *= scale;
		bottom *= scale;
		ఎడమ *= scale;
		x *= scale;
		y *= scale;
	}

	ప్రతిజ్ఞా(box->style);

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
	p[0] = x - ఎడమ;		p[1] = y - శిఖరం;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_వెడల్పు;		p[5] = y + p_ఎత్తు;		/* C */
	p[6] = x + p_వెడల్పు + కుడి;	p[7] = y + p_ఎత్తు + bottom;	/* D */

	కొరకు (i = 0; i != 4; i++) {
		రంగు నిలువు = 0;
		side = sides[i]; /* plot order */

		ఒకవేళ (box->border[side].వెడల్పు == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			కొనసాగు;
		}

		ఎంపిక (side) {
		స్థితి ఎడమ_2:
			వర్గం_చివర_1 = (శిఖరం == 0);
			వర్గం_చివర_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ఒకవేళ (nscss_color_is_transparent(box->border[శిఖరం_2].c) == false &&
			    box->border[శిఖరం_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= శిఖరం;
				వర్గం_చివర_1 = true;
			}
			ఒకవేళ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				వర్గం_చివర_2 = true;
			}

			నిలువు = nscss_color_గమ్యం_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      నిలువు,
						      box->border[side].style,
						      box->border[side].వెడల్పు * scale,
						      వర్గం_చివర_1 && వర్గం_చివర_2,
						      clip,
						      ctx);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
			ఆపు;

		స్థితి కుడి_2:
			వర్గం_చివర_1 = (శిఖరం == 0);
			వర్గం_చివర_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ఒకవేళ (nscss_color_is_transparent(box->border[శిఖరం_2].c) == false &&
			    box->border[శిఖరం_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= శిఖరం;
				వర్గం_చివర_1 = true;
			}
			ఒకవేళ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				వర్గం_చివర_2 = true;
			}

			నిలువు = nscss_color_గమ్యం_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      నిలువు,
						      box->border[side].style,
						      box->border[side].వెడల్పు * scale,
						      వర్గం_చివర_1 && వర్గం_చివర_2,
						      clip,
						      ctx);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
			ఆపు;

		స్థితి శిఖరం_2:
			ఒకవేళ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				కొనసాగు;
			}

			వర్గం_చివర_1 = (ఎడమ == 0);
			వర్గం_చివర_2 = (కుడి == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ఒకవేళ (box->border[శిఖరం_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[శిఖరం_2].c == box->border[ఎడమ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ఎడమ;
				వర్గం_చివర_1 = true;
			}
			ఒకవేళ (box->border[శిఖరం_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[శిఖరం_2].c == box->border[కుడి_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= కుడి;
				వర్గం_చివర_2 = true;
			}

			నిలువు = nscss_color_గమ్యం_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      నిలువు,
						      box->border[side].style,
						      box->border[side].వెడల్పు * scale,
						      వర్గం_చివర_1 && వర్గం_చివర_2,
						      clip,
						      ctx);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
			ఆపు;

		స్థితి BOTTOM:
			ఒకవేళ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				కొనసాగు;
			}

			వర్గం_చివర_1 = (ఎడమ == 0);
			వర్గం_చివర_2 = (కుడి == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ఒకవేళ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ఎడమ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ఎడమ;
				వర్గం_చివర_1 = true;
			}
			ఒకవేళ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[కుడి_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= కుడి;
				వర్గం_చివర_2 = true;
			}

			నిలువు = nscss_color_గమ్యం_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      నిలువు,
						      box->border[side].style,
						      box->border[side].వెడల్పు * scale,
						      వర్గం_చివర_1 && వర్గం_చివర_2,
						      clip,
						      ctx);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
			ఆపు;

		అప్రమేయ:
			ప్రతిజ్ఞా(side == శిఖరం_2 || side == BOTTOM ||
			       side == ఎడమ_2 || side == కుడి_2);
			ఆపు;
		}
	}

	తిరిగి true;
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
html_redraw_inline_borders(నిర్మాణం box *box,
			   నిర్మాణం rect b,
			   స్థిరరాశి నిర్మాణం rect *clip,
			   దశాంశం scale,
			   bool మొదటి,
			   bool చివరి,
			   స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	పూర్ణాంకం శిఖరం = box->border[శిఖరం_2].వెడల్పు;
	పూర్ణాంకం కుడి = box->border[కుడి_2].వెడల్పు;
	పూర్ణాంకం bottom = box->border[BOTTOM].వెడల్పు;
	పూర్ణాంకం ఎడమ = box->border[ఎడమ_2].వెడల్పు;
	రంగు నిలువు;
	పూర్ణాంకం p[8]; /* Box border vertices */
	పూర్ణాంకం z[8]; /* Border vertices */
	bool వర్గం_చివర_1;
	bool వర్గం_చివర_2;
	nserror res;

	ఒకవేళ (scale != 1.0) {
		శిఖరం *= scale;
		కుడి *= scale;
		bottom *= scale;
		ఎడమ *= scale;
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
	p[2] = మొదటి ? b.x0 + ఎడమ : b.x0;	p[3] = b.y0 + శిఖరం;	/* B */
	p[4] = చివరి ? b.x1 - కుడి : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ప్రతిజ్ఞా(box->style);

	/* Left */
	వర్గం_చివర_1 = (శిఖరం == 0);
	వర్గం_చివర_2 = (bottom == 0);
	ఒకవేళ (ఎడమ != 0 &&
	    మొదటి &&
	    nscss_color_is_transparent(box->border[ఎడమ_2].c) == false) {
		నిలువు = nscss_color_గమ్యం_ns(box->border[ఎడమ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ఒకవేళ (nscss_color_is_transparent(box->border[శిఖరం_2].c) == false &&
		    box->border[శిఖరం_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= శిఖరం;
			వర్గం_చివర_1 = true;
		}

		ఒకవేళ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			వర్గం_చివర_2 = true;
		}

		res = html_redraw_border_plot(ఎడమ_2,
					      z,
					      నిలువు,
					      box->border[ఎడమ_2].style,
					      ఎడమ,
					      వర్గం_చివర_1 && వర్గం_చివర_2,
					      clip,
					      ctx);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}

	/* Right */
	వర్గం_చివర_1 = (శిఖరం == 0);
	వర్గం_చివర_2 = (bottom == 0);
	ఒకవేళ (కుడి != 0 &&
	    చివరి &&
	    nscss_color_is_transparent(box->border[కుడి_2].c) == false) {
		నిలువు = nscss_color_గమ్యం_ns(box->border[కుడి_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ఒకవేళ (nscss_color_is_transparent(box->border[శిఖరం_2].c) == false &&
		    box->border[శిఖరం_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= శిఖరం;
			వర్గం_చివర_1 = true;
		}

		ఒకవేళ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			వర్గం_చివర_2 = true;
		}

		res = html_redraw_border_plot(కుడి_2,
					      z,
					      నిలువు,
					      box->border[కుడి_2].style,
					      కుడి,
					      వర్గం_చివర_1 && వర్గం_చివర_2,
					      clip,
					      ctx);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}

	/* Top */
	వర్గం_చివర_1 = (ఎడమ == 0);
	వర్గం_చివర_2 = (కుడి == 0);
	ఒకవేళ (శిఖరం != 0 &&
	    nscss_color_is_transparent(box->border[శిఖరం_2].c) == false) {
		నిలువు = nscss_color_గమ్యం_ns(box->border[శిఖరం_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ఒకవేళ (మొదటి &&
		    box->border[శిఖరం_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[శిఖరం_2].c == box->border[ఎడమ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ఎడమ;
			వర్గం_చివర_1 = true;
		}

		ఒకవేళ (చివరి &&
		    box->border[శిఖరం_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[శిఖరం_2].c == box->border[కుడి_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= కుడి;
			వర్గం_చివర_2 = true;
		}

		res = html_redraw_border_plot(శిఖరం_2,
					      z,
					      నిలువు,
					      box->border[శిఖరం_2].style,
					      శిఖరం,
					      వర్గం_చివర_1 && వర్గం_చివర_2,
					      clip,
					      ctx);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}

	/* Bottom */
	వర్గం_చివర_1 = (ఎడమ == 0);
	వర్గం_చివర_2 = (కుడి == 0);
	ఒకవేళ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		నిలువు = nscss_color_గమ్యం_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ఒకవేళ (మొదటి &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ఎడమ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ఎడమ;
			వర్గం_చివర_1 = true;
		}

		ఒకవేళ (చివరి &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[కుడి_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= కుడి;
			వర్గం_చివర_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      నిలువు,
					      box->border[BOTTOM].style,
					      bottom,
					      వర్గం_చివర_1 && వర్గం_చివర_2,
					      clip,
					      ctx);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}

	తిరిగి true;
}
