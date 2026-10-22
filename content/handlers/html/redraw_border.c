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

#شامل <stdbool.h>
#شامل <stdlib.h>

#شامل "utils/utils.h"
#شامل "utils/log.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/css.h"

#شامل "html/box.h"
#شامل "html/private.h"


ساکن plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ساکن plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساکن plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساکن plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساکن plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساکن plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ساکن اندرونی nserror
plot_clipped_rectangle(ثابت ساخت redraw_context *ctx,
		       ثابت plot_style_t *style,
		       ثابت ساخت rect *clip,
		       ساخت rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	اگر ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ورنہ {
		res = NSERROR_OK;
	}
	واپس res;
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
ساکن nserror
html_redraw_border_plot(ثابت صحیح side,
			ثابت صحیح *p,
			رنگ c,
			شمار css_border_style_e style,
			صحیح thickness,
			bool rectangular,
			ثابت ساخت rect *clip,
			ثابت ساخت redraw_context *ctx)
{
	صحیح z[8]; /* Vertices of border part */
	غیرعلامتی صحیح light = side;
	plot_style_t *plot_style_bdr_اندر;
	plot_style_t *plot_style_bdr_برآمد;
	nserror res = NSERROR_OK;
	ساخت rect rect;

	اگر (c == NS_TRANSPARENT) {
		واپس res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_رنگ = c;
	plot_style_bdr.stroke_چوڑائی = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_رنگ = c;
	plot_style_fillbdr_dark.fill_رنگ = darken_رنگ(c);
	plot_style_fillbdr_light.fill_رنگ = lighten_رنگ(c);
	plot_style_fillbdr_ddark.fill_رنگ = double_darken_رنگ(c);
	plot_style_fillbdr_dlight.fill_رنگ = double_lighten_رنگ(c);

	انتخاب (style) {
	حالت CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	حالت CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->سطر(ctx, &plot_style_bdr, &rect);
		توڑو;

	حالت CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	طےشدہ:
		اگر (rectangular || thickness == 1) {

			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				اگر ((side == چوٹی_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ورنہ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ورنہ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				اگر ((side == بایاں_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ورنہ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ورنہ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		توڑو;

	حالت CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		اگر (res == NSERROR_OK) {
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
		توڑو;

	حالت CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		اگر (light <= 1) {
			plot_style_bdr_اندر = &plot_style_fillbdr_dark;
			plot_style_bdr_برآمد = &plot_style_fillbdr_light;
		} ورنہ {
			plot_style_bdr_اندر = &plot_style_fillbdr_light;
			plot_style_bdr_برآمد = &plot_style_fillbdr_dark;
		}

		/* Render border */
		اگر ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ورنہ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_اندر,
						     clip,
						     &rect);
			اگر (res != NSERROR_OK) {
				واپس res;
			}

			/* Second part */
			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ورنہ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_برآمد,
						     clip,
						     &rect);
		} ورنہ اگر (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == چوٹی_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_اندر,
							     clip,
							     &rect);
			} ورنہ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == بایاں_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_برآمد,
							     clip,
							     &rect);
			}
		} ورنہ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_اندر, z, 4);
			اگر (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_برآمد,
							 z,
							 4);
			}
		}
		توڑو;

	حالت CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		انتخاب (light) {
		حالت 0:
			plot_style_bdr_اندر = &plot_style_fillbdr_light;
			plot_style_bdr_برآمد = &plot_style_fillbdr_dlight;
			توڑو;
		حالت 1:
			plot_style_bdr_اندر = &plot_style_fillbdr_ddark;
			plot_style_bdr_برآمد = &plot_style_fillbdr_dark;
			توڑو;
		حالت 2:
			plot_style_bdr_اندر = &plot_style_fillbdr_dark;
			plot_style_bdr_برآمد = &plot_style_fillbdr_ddark;
			توڑو;
		حالت 3:
			plot_style_bdr_اندر = &plot_style_fillbdr_dlight;
			plot_style_bdr_برآمد = &plot_style_fillbdr_light;
			توڑو;
		طےشدہ:
			plot_style_bdr_اندر = &plot_style_fillbdr;
			plot_style_bdr_برآمد = &plot_style_fillbdr;
			توڑو;
		}

		/* Render border */
		اگر ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ورنہ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_اندر,
						     clip,
						     &rect);
			اگر (res != NSERROR_OK) {
				واپس res;
			}

			/* Second part */
			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ورنہ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_برآمد,
						     clip,
						     &rect);
		} ورنہ اگر (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			اگر (side == چوٹی_2 || side == دایاں_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == چوٹی_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_اندر,
							     clip,
							     &rect);
			} ورنہ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == بایاں_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_برآمد,
							     clip,
							     &rect);
			}
		} ورنہ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_اندر, z, 4);
			اگر (res != NSERROR_OK) {
				واپس res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_برآمد, z, 4);
		}
		توڑو;
	}

	واپس res;
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
html_redraw_borders(ساخت box *box,
		    صحیح x_والد,
		    صحیح y_والد,
		    صحیح p_چوڑائی,
		    صحیح p_اونچائی,
		    ثابت ساخت rect *clip,
		    اعشاری scale,
		    ثابت ساخت redraw_context *ctx)
{
	غیرعلامتی صحیح sides[] = { بایاں_2, دایاں_2, چوٹی_2, BOTTOM };
	صحیح چوٹی = box->border[چوٹی_2].چوڑائی;
	صحیح دایاں = box->border[دایاں_2].چوڑائی;
	صحیح bottom = box->border[BOTTOM].چوڑائی;
	صحیح بایاں = box->border[بایاں_2].چوڑائی;
	صحیح x, y;
	غیرعلامتی صحیح i, side;
	صحیح p[8]; /* Box border vertices */
	صحیح z[8]; /* Border vertices */
	bool مربع_انتہا_1 = false;
	bool مربع_انتہا_2 = false;
	nserror res;

	x = x_والد + box->x;
	y = y_والد + box->y;

	اگر (scale != 1.0) {
		چوٹی *= scale;
		دایاں *= scale;
		bottom *= scale;
		بایاں *= scale;
		x *= scale;
		y *= scale;
	}

	أكد(box->style);

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
	p[0] = x - بایاں;		p[1] = y - چوٹی;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_چوڑائی;		p[5] = y + p_اونچائی;		/* C */
	p[6] = x + p_چوڑائی + دایاں;	p[7] = y + p_اونچائی + bottom;	/* D */

	برائے (i = 0; i != 4; i++) {
		رنگ ستون = 0;
		side = sides[i]; /* plot order */

		اگر (box->border[side].چوڑائی == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			جاری;
		}

		انتخاب (side) {
		حالت بایاں_2:
			مربع_انتہا_1 = (چوٹی == 0);
			مربع_انتہا_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			اگر (nscss_color_is_transparent(box->border[چوٹی_2].c) == false &&
			    box->border[چوٹی_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= چوٹی;
				مربع_انتہا_1 = true;
			}
			اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				مربع_انتہا_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].چوڑائی * scale,
						      مربع_انتہا_1 && مربع_انتہا_2,
						      clip,
						      ctx);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
			توڑو;

		حالت دایاں_2:
			مربع_انتہا_1 = (چوٹی == 0);
			مربع_انتہا_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			اگر (nscss_color_is_transparent(box->border[چوٹی_2].c) == false &&
			    box->border[چوٹی_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= چوٹی;
				مربع_انتہا_1 = true;
			}
			اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				مربع_انتہا_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].چوڑائی * scale,
						      مربع_انتہا_1 && مربع_انتہا_2,
						      clip,
						      ctx);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
			توڑو;

		حالت چوٹی_2:
			اگر (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				جاری;
			}

			مربع_انتہا_1 = (بایاں == 0);
			مربع_انتہا_2 = (دایاں == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			اگر (box->border[چوٹی_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[چوٹی_2].c == box->border[بایاں_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += بایاں;
				مربع_انتہا_1 = true;
			}
			اگر (box->border[چوٹی_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[چوٹی_2].c == box->border[دایاں_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= دایاں;
				مربع_انتہا_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].چوڑائی * scale,
						      مربع_انتہا_1 && مربع_انتہا_2,
						      clip,
						      ctx);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
			توڑو;

		حالت BOTTOM:
			اگر (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				جاری;
			}

			مربع_انتہا_1 = (بایاں == 0);
			مربع_انتہا_2 = (دایاں == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			اگر (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[بایاں_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += بایاں;
				مربع_انتہا_1 = true;
			}
			اگر (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[دایاں_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= دایاں;
				مربع_انتہا_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].چوڑائی * scale,
						      مربع_انتہا_1 && مربع_انتہا_2,
						      clip,
						      ctx);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
			توڑو;

		طےشدہ:
			أكد(side == چوٹی_2 || side == BOTTOM ||
			       side == بایاں_2 || side == دایاں_2);
			توڑو;
		}
	}

	واپس true;
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
html_redraw_inline_borders(ساخت box *box,
			   ساخت rect b,
			   ثابت ساخت rect *clip,
			   اعشاری scale,
			   bool پہلا,
			   bool آخری,
			   ثابت ساخت redraw_context *ctx)
{
	صحیح چوٹی = box->border[چوٹی_2].چوڑائی;
	صحیح دایاں = box->border[دایاں_2].چوڑائی;
	صحیح bottom = box->border[BOTTOM].چوڑائی;
	صحیح بایاں = box->border[بایاں_2].چوڑائی;
	رنگ ستون;
	صحیح p[8]; /* Box border vertices */
	صحیح z[8]; /* Border vertices */
	bool مربع_انتہا_1;
	bool مربع_انتہا_2;
	nserror res;

	اگر (scale != 1.0) {
		چوٹی *= scale;
		دایاں *= scale;
		bottom *= scale;
		بایاں *= scale;
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
	p[2] = پہلا ? b.x0 + بایاں : b.x0;	p[3] = b.y0 + چوٹی;	/* B */
	p[4] = آخری ? b.x1 - دایاں : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	أكد(box->style);

	/* Left */
	مربع_انتہا_1 = (چوٹی == 0);
	مربع_انتہا_2 = (bottom == 0);
	اگر (بایاں != 0 &&
	    پہلا &&
	    nscss_color_is_transparent(box->border[بایاں_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[بایاں_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		اگر (nscss_color_is_transparent(box->border[چوٹی_2].c) == false &&
		    box->border[چوٹی_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= چوٹی;
			مربع_انتہا_1 = true;
		}

		اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			مربع_انتہا_2 = true;
		}

		res = html_redraw_border_plot(بایاں_2,
					      z,
					      ستون,
					      box->border[بایاں_2].style,
					      بایاں,
					      مربع_انتہا_1 && مربع_انتہا_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}

	/* Right */
	مربع_انتہا_1 = (چوٹی == 0);
	مربع_انتہا_2 = (bottom == 0);
	اگر (دایاں != 0 &&
	    آخری &&
	    nscss_color_is_transparent(box->border[دایاں_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[دایاں_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		اگر (nscss_color_is_transparent(box->border[چوٹی_2].c) == false &&
		    box->border[چوٹی_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= چوٹی;
			مربع_انتہا_1 = true;
		}

		اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			مربع_انتہا_2 = true;
		}

		res = html_redraw_border_plot(دایاں_2,
					      z,
					      ستون,
					      box->border[دایاں_2].style,
					      دایاں,
					      مربع_انتہا_1 && مربع_انتہا_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}

	/* Top */
	مربع_انتہا_1 = (بایاں == 0);
	مربع_انتہا_2 = (دایاں == 0);
	اگر (چوٹی != 0 &&
	    nscss_color_is_transparent(box->border[چوٹی_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[چوٹی_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		اگر (پہلا &&
		    box->border[چوٹی_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[چوٹی_2].c == box->border[بایاں_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += بایاں;
			مربع_انتہا_1 = true;
		}

		اگر (آخری &&
		    box->border[چوٹی_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[چوٹی_2].c == box->border[دایاں_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= دایاں;
			مربع_انتہا_2 = true;
		}

		res = html_redraw_border_plot(چوٹی_2,
					      z,
					      ستون,
					      box->border[چوٹی_2].style,
					      چوٹی,
					      مربع_انتہا_1 && مربع_انتہا_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}

	/* Bottom */
	مربع_انتہا_1 = (بایاں == 0);
	مربع_انتہا_2 = (دایاں == 0);
	اگر (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		اگر (پہلا &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[بایاں_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += بایاں;
			مربع_انتہا_1 = true;
		}

		اگر (آخری &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[دایاں_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= دایاں;
			مربع_انتہا_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      ستون,
					      box->border[BOTTOM].style,
					      bottom,
					      مربع_انتہا_1 && مربع_انتہا_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}

	واپس true;
}
