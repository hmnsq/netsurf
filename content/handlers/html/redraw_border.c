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


ایستا plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ایستا plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ایستا plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ایستا plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ایستا plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ایستا plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ایستا درونخطی nserror
plot_clipped_rectangle(ثابت ساختار redraw_context *ctx,
		       ثابت plot_style_t *style,
		       ثابت ساختار rect *clip,
		       ساختار rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	اگر ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} وگرنه {
		res = NSERROR_OK;
	}
	بازگشت res;
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
ایستا nserror
html_redraw_border_plot(ثابت صحیح side,
			ثابت صحیح *p,
			colour c,
			شمارش css_border_style_e style,
			صحیح thickness,
			bool rectangular,
			ثابت ساختار rect *clip,
			ثابت ساختار redraw_context *ctx)
{
	صحیح z[8]; /* Vertices of border part */
	unsigned صحیح light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	ساختار rect rect;

	اگر (c == NS_TRANSPARENT) {
		بازگشت res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

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
		بشکن;

	حالت CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	پیشفرض:
		اگر (rectangular || thickness == 1) {

			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				اگر ((side == بالاترین_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} وگرنه {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} وگرنه {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				اگر ((side == چپ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} وگرنه {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} وگرنه {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		بشکن;

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
		بشکن;

	حالت CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	حالت CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		اگر (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} وگرنه {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		اگر ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} وگرنه {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}

			/* Second part */
			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} وگرنه {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} وگرنه اگر (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == بالاترین_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} وگرنه {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == چپ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} وگرنه {
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
			اگر (res == NSERROR_OK) {
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
		بشکن;

	حالت CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	حالت CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		انتخاب (light) {
		حالت 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			بشکن;
		حالت 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			بشکن;
		حالت 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			بشکن;
		حالت 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			بشکن;
		پیشفرض:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			بشکن;
		}

		/* Render border */
		اگر ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} وگرنه {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}

			/* Second part */
			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} وگرنه {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} وگرنه اگر (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			اگر (side == بالاترین_2 || side == راست_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == بالاترین_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} وگرنه {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == چپ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} وگرنه {
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
			اگر (res != NSERROR_OK) {
				بازگشت res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		بشکن;
	}

	بازگشت res;
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
html_redraw_borders(ساختار box *box,
		    صحیح x_parent,
		    صحیح y_parent,
		    صحیح p_width,
		    صحیح p_height,
		    ثابت ساختار rect *clip,
		    اعشاری scale,
		    ثابت ساختار redraw_context *ctx)
{
	unsigned صحیح sides[] = { چپ_2, راست_2, بالاترین_2, BOTTOM };
	صحیح بالاترین = box->border[بالاترین_2].width;
	صحیح راست = box->border[راست_2].width;
	صحیح bottom = box->border[BOTTOM].width;
	صحیح چپ = box->border[چپ_2].width;
	صحیح x, y;
	unsigned صحیح i, side;
	صحیح p[8]; /* Box border vertices */
	صحیح z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	اگر (scale != 1.0) {
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
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + راست;	p[7] = y + p_height + bottom;	/* D */

	برای (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		اگر (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ادامه;
		}

		انتخاب (side) {
		حالت چپ_2:
			square_end_1 = (بالاترین == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			اگر (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
			    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= بالاترین;
				square_end_1 = true;
			}
			اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
			بشکن;

		حالت راست_2:
			square_end_1 = (بالاترین == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			اگر (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
			    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= بالاترین;
				square_end_1 = true;
			}
			اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
			بشکن;

		حالت بالاترین_2:
			اگر (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ادامه;
			}

			square_end_1 = (چپ == 0);
			square_end_2 = (راست == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			اگر (box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[بالاترین_2].c == box->border[چپ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += چپ;
				square_end_1 = true;
			}
			اگر (box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[بالاترین_2].c == box->border[راست_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= راست;
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
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
			بشکن;

		حالت BOTTOM:
			اگر (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ادامه;
			}

			square_end_1 = (چپ == 0);
			square_end_2 = (راست == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			اگر (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[چپ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += چپ;
				square_end_1 = true;
			}
			اگر (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[راست_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= راست;
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
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
			بشکن;

		پیشفرض:
			أكد(side == بالاترین_2 || side == BOTTOM ||
			       side == چپ_2 || side == راست_2);
			بشکن;
		}
	}

	بازگشت true;
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
html_redraw_inline_borders(ساختار box *box,
			   ساختار rect b,
			   ثابت ساختار rect *clip,
			   اعشاری scale,
			   bool first,
			   bool last,
			   ثابت ساختار redraw_context *ctx)
{
	صحیح بالاترین = box->border[بالاترین_2].width;
	صحیح راست = box->border[راست_2].width;
	صحیح bottom = box->border[BOTTOM].width;
	صحیح چپ = box->border[چپ_2].width;
	colour col;
	صحیح p[8]; /* Box border vertices */
	صحیح z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	اگر (scale != 1.0) {
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
	p[2] = first ? b.x0 + چپ : b.x0;	p[3] = b.y0 + بالاترین;	/* B */
	p[4] = last ? b.x1 - راست : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	أكد(box->style);

	/* Left */
	square_end_1 = (بالاترین == 0);
	square_end_2 = (bottom == 0);
	اگر (چپ != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[چپ_2].c) == false) {
		col = nscss_color_to_ns(box->border[چپ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		اگر (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
		    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= بالاترین;
			square_end_1 = true;
		}

		اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(چپ_2,
					      z,
					      col,
					      box->border[چپ_2].style,
					      چپ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}

	/* Right */
	square_end_1 = (بالاترین == 0);
	square_end_2 = (bottom == 0);
	اگر (راست != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[راست_2].c) == false) {
		col = nscss_color_to_ns(box->border[راست_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		اگر (nscss_color_is_transparent(box->border[بالاترین_2].c) == false &&
		    box->border[بالاترین_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= بالاترین;
			square_end_1 = true;
		}

		اگر (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(راست_2,
					      z,
					      col,
					      box->border[راست_2].style,
					      راست,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}

	/* Top */
	square_end_1 = (چپ == 0);
	square_end_2 = (راست == 0);
	اگر (بالاترین != 0 &&
	    nscss_color_is_transparent(box->border[بالاترین_2].c) == false) {
		col = nscss_color_to_ns(box->border[بالاترین_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		اگر (first &&
		    box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[بالاترین_2].c == box->border[چپ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += چپ;
			square_end_1 = true;
		}

		اگر (last &&
		    box->border[بالاترین_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[بالاترین_2].c == box->border[راست_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= راست;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(بالاترین_2,
					      z,
					      col,
					      box->border[بالاترین_2].style,
					      بالاترین,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}

	/* Bottom */
	square_end_1 = (چپ == 0);
	square_end_2 = (راست == 0);
	اگر (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		اگر (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[چپ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += چپ;
			square_end_1 = true;
		}

		اگر (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[راست_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= راست;
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
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}

	بازگشت true;
}
