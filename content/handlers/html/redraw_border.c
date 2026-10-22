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

#ضمن <stdbool.h>
#ضمن <stdlib.h>

#ضمن "utils/utils.h"
#ضمن "utils/log.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/css.h"

#ضمن "html/box.h"
#ضمن "html/private.h"


ساكن plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ساكن plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساكن plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساكن plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساكن plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ساكن plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ساكن مضمن nserror
plot_clipped_rectangle(ثابت بنية redraw_context *ctx,
		       ثابت plot_style_t *style,
		       ثابت بنية rect *clip,
		       بنية rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	إذا ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} وإلا {
		res = NSERROR_OK;
	}
	أرجع res;
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
ساكن nserror
html_redraw_border_plot(ثابت صحيح side,
			ثابت صحيح *p,
			رنگ c,
			تعداد css_border_style_e style,
			صحيح thickness,
			bool rectangular,
			ثابت بنية rect *clip,
			ثابت بنية redraw_context *ctx)
{
	صحيح z[8]; /* Vertices of border part */
	بیعلامت صحيح light = side;
	plot_style_t *plot_style_bdr_درون;
	plot_style_t *plot_style_bdr_بیرون;
	nserror res = NSERROR_OK;
	بنية rect rect;

	إذا (c == NS_TRANSPARENT) {
		أرجع res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_رنگ = c;
	plot_style_bdr.stroke_عرض = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_رنگ = c;
	plot_style_fillbdr_dark.fill_رنگ = darken_رنگ(c);
	plot_style_fillbdr_light.fill_رنگ = lighten_رنگ(c);
	plot_style_fillbdr_ddark.fill_رنگ = double_darken_رنگ(c);
	plot_style_fillbdr_dlight.fill_رنگ = double_lighten_رنگ(c);

	بدل (style) {
	حالة CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	حالة CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->سطر(ctx, &plot_style_bdr, &rect);
		اكسر;

	حالة CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	افتراضي:
		إذا (rectangular || thickness == 1) {

			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				إذا ((side == بالاترین_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} وإلا {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} وإلا {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				إذا ((side == چپ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} وإلا {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} وإلا {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		اكسر;

	حالة CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		إذا (res == NSERROR_OK) {
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
		اكسر;

	حالة CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	حالة CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		إذا (light <= 1) {
			plot_style_bdr_درون = &plot_style_fillbdr_dark;
			plot_style_bdr_بیرون = &plot_style_fillbdr_light;
		} وإلا {
			plot_style_bdr_درون = &plot_style_fillbdr_light;
			plot_style_bdr_بیرون = &plot_style_fillbdr_dark;
		}

		/* Render border */
		إذا ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} وإلا {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_درون,
						     clip,
						     &rect);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			/* Second part */
			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} وإلا {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_بیرون,
						     clip,
						     &rect);
		} وإلا إذا (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == بالاترین_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_درون,
							     clip,
							     &rect);
			} وإلا {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == چپ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_بیرون,
							     clip,
							     &rect);
			}
		} وإلا {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_درون, z, 4);
			إذا (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_بیرون,
							 z,
							 4);
			}
		}
		اكسر;

	حالة CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	حالة CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		بدل (light) {
		حالة 0:
			plot_style_bdr_درون = &plot_style_fillbdr_light;
			plot_style_bdr_بیرون = &plot_style_fillbdr_dlight;
			اكسر;
		حالة 1:
			plot_style_bdr_درون = &plot_style_fillbdr_ddark;
			plot_style_bdr_بیرون = &plot_style_fillbdr_dark;
			اكسر;
		حالة 2:
			plot_style_bdr_درون = &plot_style_fillbdr_dark;
			plot_style_bdr_بیرون = &plot_style_fillbdr_ddark;
			اكسر;
		حالة 3:
			plot_style_bdr_درون = &plot_style_fillbdr_dlight;
			plot_style_bdr_بیرون = &plot_style_fillbdr_light;
			اكسر;
		افتراضي:
			plot_style_bdr_درون = &plot_style_fillbdr;
			plot_style_bdr_بیرون = &plot_style_fillbdr;
			اكسر;
		}

		/* Render border */
		إذا ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} وإلا {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_درون,
						     clip,
						     &rect);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}

			/* Second part */
			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} وإلا {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_بیرون,
						     clip,
						     &rect);
		} وإلا إذا (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			إذا (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == بالاترین_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_درون,
							     clip,
							     &rect);
			} وإلا {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == چپ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_بیرون,
							     clip,
							     &rect);
			}
		} وإلا {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_درون, z, 4);
			إذا (res != NSERROR_OK) {
				أرجع res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_بیرون, z, 4);
		}
		اكسر;
	}

	أرجع res;
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
html_redraw_borders(بنية box *box,
		    صحيح x_والد,
		    صحيح y_والد,
		    صحيح p_عرض,
		    صحيح p_ارتفاع,
		    ثابت بنية rect *clip,
		    عائم scale,
		    ثابت بنية redraw_context *ctx)
{
	بیعلامت صحيح sides[] = { چپ_2, راست_2, بالاترین_2, BOTTOM };
	صحيح بالاترین = box->border[بالاترین_2].عرض;
	صحيح راست = box->border[راست_2].عرض;
	صحيح bottom = box->border[BOTTOM].عرض;
	صحيح چپ = box->border[چپ_2].عرض;
	صحيح x, y;
	بیعلامت صحيح i, side;
	صحيح p[8]; /* Box border vertices */
	صحيح z[8]; /* Border vertices */
	bool مربع_انتها_1 = false;
	bool مربع_انتها_2 = false;
	nserror res;

	x = x_والد + box->x;
	y = y_والد + box->y;

	إذا (scale != 1.0) {
		بالاترین *= scale;
		راست *= scale;
		bottom *= scale;
		چپ *= scale;
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
	p[0] = x - چپ;		p[1] = y - بالاترین;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_عرض;		p[5] = y + p_ارتفاع;		/* C */
	p[6] = x + p_عرض + راست;	p[7] = y + p_ارتفاع + bottom;	/* D */

	لكل (i = 0; i != 4; i++) {
		رنگ ستون = 0;
		side = sides[i]; /* plot order */

		إذا (box->border[side].عرض == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			استمر;
		}

		بدل (side) {
		حالة چپ_2:
			مربع_انتها_1 = (بالاترین == 0);
			مربع_انتها_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			إذا (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
			    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= بالاترین;
				مربع_انتها_1 = true;
			}
			إذا (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				مربع_انتها_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].عرض * scale,
						      مربع_انتها_1 && مربع_انتها_2,
						      clip,
						      ctx);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
			اكسر;

		حالة راست_2:
			مربع_انتها_1 = (بالاترین == 0);
			مربع_انتها_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			إذا (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
			    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= بالاترین;
				مربع_انتها_1 = true;
			}
			إذا (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				مربع_انتها_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].عرض * scale,
						      مربع_انتها_1 && مربع_انتها_2,
						      clip,
						      ctx);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
			اكسر;

		حالة بالاترین_2:
			إذا (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				استمر;
			}

			مربع_انتها_1 = (چپ == 0);
			مربع_انتها_2 = (راست == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			إذا (box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[بالاترین_2].c == box->border[چپ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += چپ;
				مربع_انتها_1 = true;
			}
			إذا (box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[بالاترین_2].c == box->border[راست_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= راست;
				مربع_انتها_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].عرض * scale,
						      مربع_انتها_1 && مربع_انتها_2,
						      clip,
						      ctx);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
			اكسر;

		حالة BOTTOM:
			إذا (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				استمر;
			}

			مربع_انتها_1 = (چپ == 0);
			مربع_انتها_2 = (راست == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			إذا (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[چپ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += چپ;
				مربع_انتها_1 = true;
			}
			إذا (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[راست_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= راست;
				مربع_انتها_2 = true;
			}

			ستون = nscss_color_مقصد_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ستون,
						      box->border[side].style,
						      box->border[side].عرض * scale,
						      مربع_انتها_1 && مربع_انتها_2,
						      clip,
						      ctx);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
			اكسر;

		افتراضي:
			أكد(side == بالاترین_2 || side == BOTTOM ||
			       side == چپ_2 || side == راست_2);
			اكسر;
		}
	}

	أرجع true;
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
html_redraw_inline_borders(بنية box *box,
			   بنية rect b,
			   ثابت بنية rect *clip,
			   عائم scale,
			   bool اولی,
			   bool آخرین,
			   ثابت بنية redraw_context *ctx)
{
	صحيح بالاترین = box->border[بالاترین_2].عرض;
	صحيح راست = box->border[راست_2].عرض;
	صحيح bottom = box->border[BOTTOM].عرض;
	صحيح چپ = box->border[چپ_2].عرض;
	رنگ ستون;
	صحيح p[8]; /* Box border vertices */
	صحيح z[8]; /* Border vertices */
	bool مربع_انتها_1;
	bool مربع_انتها_2;
	nserror res;

	إذا (scale != 1.0) {
		بالاترین *= scale;
		راست *= scale;
		bottom *= scale;
		چپ *= scale;
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
	p[2] = اولی ? b.x0 + چپ : b.x0;	p[3] = b.y0 + بالاترین;	/* B */
	p[4] = آخرین ? b.x1 - راست : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	أكد(box->style);

	/* Left */
	مربع_انتها_1 = (بالاترین == 0);
	مربع_انتها_2 = (bottom == 0);
	إذا (چپ != 0 &&
	    اولی &&
	    nscss_color_is_transparent(box->border[چپ_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[چپ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		إذا (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
		    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= بالاترین;
			مربع_انتها_1 = true;
		}

		إذا (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			مربع_انتها_2 = true;
		}

		res = html_redraw_border_plot(چپ_2,
					      z,
					      ستون,
					      box->border[چپ_2].style,
					      چپ,
					      مربع_انتها_1 && مربع_انتها_2,
					      clip,
					      ctx);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	/* Right */
	مربع_انتها_1 = (بالاترین == 0);
	مربع_انتها_2 = (bottom == 0);
	إذا (راست != 0 &&
	    آخرین &&
	    nscss_color_is_transparent(box->border[راست_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[راست_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		إذا (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
		    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= بالاترین;
			مربع_انتها_1 = true;
		}

		إذا (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			مربع_انتها_2 = true;
		}

		res = html_redraw_border_plot(راست_2,
					      z,
					      ستون,
					      box->border[راست_2].style,
					      راست,
					      مربع_انتها_1 && مربع_انتها_2,
					      clip,
					      ctx);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	/* Top */
	مربع_انتها_1 = (چپ == 0);
	مربع_انتها_2 = (راست == 0);
	إذا (بالاترین != 0 &&
	    nscss_color_is_transparent(box->border[بالاترین_2].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[بالاترین_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		إذا (اولی &&
		    box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[بالاترین_2].c == box->border[چپ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += چپ;
			مربع_انتها_1 = true;
		}

		إذا (آخرین &&
		    box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[بالاترین_2].c == box->border[راست_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= راست;
			مربع_انتها_2 = true;
		}

		res = html_redraw_border_plot(بالاترین_2,
					      z,
					      ستون,
					      box->border[بالاترین_2].style,
					      بالاترین,
					      مربع_انتها_1 && مربع_انتها_2,
					      clip,
					      ctx);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	/* Bottom */
	مربع_انتها_1 = (چپ == 0);
	مربع_انتها_2 = (راست == 0);
	إذا (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		ستون = nscss_color_مقصد_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		إذا (اولی &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[چپ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += چپ;
			مربع_انتها_1 = true;
		}

		إذا (آخرین &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[راست_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= راست;
			مربع_انتها_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      ستون,
					      box->border[BOTTOM].style,
					      bottom,
					      مربع_انتها_1 && مربع_انتها_2,
					      clip,
					      ctx);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	أرجع true;
}
