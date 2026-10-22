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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/css.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/private.h"


স্থির plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
স্থির plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
স্থির plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
স্থির plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
স্থির plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
স্থির plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


স্থির অন্তঃরেখ nserror
plot_clipped_rectangle(ধ্রুবক কাঠামো redraw_context *ctx,
		       ধ্রুবক plot_style_t *style,
		       ধ্রুবক কাঠামো rect *clip,
		       কাঠামো rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	যদি ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} অন্যথা {
		res = NSERROR_OK;
	}
	ফেরত res;
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
স্থির nserror
html_redraw_border_plot(ধ্রুবক পূর্ণসংখ্যা side,
			ধ্রুবক পূর্ণসংখ্যা *p,
			colour c,
			গণনা css_border_style_e style,
			পূর্ণসংখ্যা thickness,
			bool rectangular,
			ধ্রুবক কাঠামো rect *clip,
			ধ্রুবক কাঠামো redraw_context *ctx)
{
	পূর্ণসংখ্যা z[8]; /* Vertices of border part */
	unsigned পূর্ণসংখ্যা light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	কাঠামো rect rect;

	যদি (c == NS_TRANSPARENT) {
		ফেরত res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	নির্বাচন (style) {
	ক্ষেত্র CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->সারি(ctx, &plot_style_bdr, &rect);
		বিরতি;

	ক্ষেত্র CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	পূর্বনির্ধারিত:
		যদি (rectangular || thickness == 1) {

			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				যদি ((side == শীর্ষ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} অন্যথা {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} অন্যথা {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				যদি ((side == বাম_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} অন্যথা {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} অন্যথা {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		বিরতি;

	ক্ষেত্র CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		যদি (res == NSERROR_OK) {
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
		বিরতি;

	ক্ষেত্র CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		যদি (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} অন্যথা {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		যদি ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} অন্যথা {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			যদি (res != NSERROR_OK) {
				ফেরত res;
			}

			/* Second part */
			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} অন্যথা {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} অন্যথা যদি (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == শীর্ষ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} অন্যথা {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == বাম_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} অন্যথা {
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
			যদি (res == NSERROR_OK) {
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
		বিরতি;

	ক্ষেত্র CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	ক্ষেত্র CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		নির্বাচন (light) {
		ক্ষেত্র 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			বিরতি;
		ক্ষেত্র 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			বিরতি;
		ক্ষেত্র 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			বিরতি;
		ক্ষেত্র 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			বিরতি;
		পূর্বনির্ধারিত:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			বিরতি;
		}

		/* Render border */
		যদি ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} অন্যথা {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			যদি (res != NSERROR_OK) {
				ফেরত res;
			}

			/* Second part */
			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} অন্যথা {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} অন্যথা যদি (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			যদি (side == শীর্ষ_2 || side == ডান_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == শীর্ষ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} অন্যথা {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == বাম_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} অন্যথা {
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
			যদি (res != NSERROR_OK) {
				ফেরত res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		বিরতি;
	}

	ফেরত res;
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
html_redraw_borders(কাঠামো box *box,
		    পূর্ণসংখ্যা x_parent,
		    পূর্ণসংখ্যা y_parent,
		    পূর্ণসংখ্যা p_width,
		    পূর্ণসংখ্যা p_height,
		    ধ্রুবক কাঠামো rect *clip,
		    দশমিক scale,
		    ধ্রুবক কাঠামো redraw_context *ctx)
{
	unsigned পূর্ণসংখ্যা sides[] = { বাম_2, ডান_2, শীর্ষ_2, BOTTOM };
	পূর্ণসংখ্যা শীর্ষ = box->border[শীর্ষ_2].width;
	পূর্ণসংখ্যা ডান = box->border[ডান_2].width;
	পূর্ণসংখ্যা bottom = box->border[BOTTOM].width;
	পূর্ণসংখ্যা বাম = box->border[বাম_2].width;
	পূর্ণসংখ্যা x, y;
	unsigned পূর্ণসংখ্যা i, side;
	পূর্ণসংখ্যা p[8]; /* Box border vertices */
	পূর্ণসংখ্যা z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	যদি (scale != 1.0) {
		শীর্ষ *= scale;
		ডান *= scale;
		bottom *= scale;
		বাম *= scale;
		x *= scale;
		y *= scale;
	}

	প্রতিজ্ঞা(box->style);

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
	p[0] = x - বাম;		p[1] = y - শীর্ষ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + ডান;	p[7] = y + p_height + bottom;	/* D */

	জন্য (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		যদি (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			চালু;
		}

		নির্বাচন (side) {
		ক্ষেত্র বাম_2:
			square_end_1 = (শীর্ষ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			যদি (nscss_color_is_transparent(box->border[শীর্ষ_2].c) == false &&
			    box->border[শীর্ষ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= শীর্ষ;
				square_end_1 = true;
			}
			যদি (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
			বিরতি;

		ক্ষেত্র ডান_2:
			square_end_1 = (শীর্ষ == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			যদি (nscss_color_is_transparent(box->border[শীর্ষ_2].c) == false &&
			    box->border[শীর্ষ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= শীর্ষ;
				square_end_1 = true;
			}
			যদি (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
			বিরতি;

		ক্ষেত্র শীর্ষ_2:
			যদি (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				চালু;
			}

			square_end_1 = (বাম == 0);
			square_end_2 = (ডান == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			যদি (box->border[শীর্ষ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[শীর্ষ_2].c == box->border[বাম_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += বাম;
				square_end_1 = true;
			}
			যদি (box->border[শীর্ষ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[শীর্ষ_2].c == box->border[ডান_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= ডান;
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
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
			বিরতি;

		ক্ষেত্র BOTTOM:
			যদি (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				চালু;
			}

			square_end_1 = (বাম == 0);
			square_end_2 = (ডান == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			যদি (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[বাম_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += বাম;
				square_end_1 = true;
			}
			যদি (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ডান_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= ডান;
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
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
			বিরতি;

		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(side == শীর্ষ_2 || side == BOTTOM ||
			       side == বাম_2 || side == ডান_2);
			বিরতি;
		}
	}

	ফেরত true;
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
html_redraw_inline_borders(কাঠামো box *box,
			   কাঠামো rect b,
			   ধ্রুবক কাঠামো rect *clip,
			   দশমিক scale,
			   bool first,
			   bool last,
			   ধ্রুবক কাঠামো redraw_context *ctx)
{
	পূর্ণসংখ্যা শীর্ষ = box->border[শীর্ষ_2].width;
	পূর্ণসংখ্যা ডান = box->border[ডান_2].width;
	পূর্ণসংখ্যা bottom = box->border[BOTTOM].width;
	পূর্ণসংখ্যা বাম = box->border[বাম_2].width;
	colour col;
	পূর্ণসংখ্যা p[8]; /* Box border vertices */
	পূর্ণসংখ্যা z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	যদি (scale != 1.0) {
		শীর্ষ *= scale;
		ডান *= scale;
		bottom *= scale;
		বাম *= scale;
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
	p[2] = first ? b.x0 + বাম : b.x0;	p[3] = b.y0 + শীর্ষ;	/* B */
	p[4] = last ? b.x1 - ডান : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	প্রতিজ্ঞা(box->style);

	/* Left */
	square_end_1 = (শীর্ষ == 0);
	square_end_2 = (bottom == 0);
	যদি (বাম != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[বাম_2].c) == false) {
		col = nscss_color_to_ns(box->border[বাম_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		যদি (nscss_color_is_transparent(box->border[শীর্ষ_2].c) == false &&
		    box->border[শীর্ষ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= শীর্ষ;
			square_end_1 = true;
		}

		যদি (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(বাম_2,
					      z,
					      col,
					      box->border[বাম_2].style,
					      বাম,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}

	/* Right */
	square_end_1 = (শীর্ষ == 0);
	square_end_2 = (bottom == 0);
	যদি (ডান != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[ডান_2].c) == false) {
		col = nscss_color_to_ns(box->border[ডান_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		যদি (nscss_color_is_transparent(box->border[শীর্ষ_2].c) == false &&
		    box->border[শীর্ষ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= শীর্ষ;
			square_end_1 = true;
		}

		যদি (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(ডান_2,
					      z,
					      col,
					      box->border[ডান_2].style,
					      ডান,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}

	/* Top */
	square_end_1 = (বাম == 0);
	square_end_2 = (ডান == 0);
	যদি (শীর্ষ != 0 &&
	    nscss_color_is_transparent(box->border[শীর্ষ_2].c) == false) {
		col = nscss_color_to_ns(box->border[শীর্ষ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		যদি (first &&
		    box->border[শীর্ষ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[শীর্ষ_2].c == box->border[বাম_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += বাম;
			square_end_1 = true;
		}

		যদি (last &&
		    box->border[শীর্ষ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[শীর্ষ_2].c == box->border[ডান_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= ডান;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(শীর্ষ_2,
					      z,
					      col,
					      box->border[শীর্ষ_2].style,
					      শীর্ষ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}

	/* Bottom */
	square_end_1 = (বাম == 0);
	square_end_2 = (ডান == 0);
	যদি (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		যদি (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[বাম_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += বাম;
			square_end_1 = true;
		}

		যদি (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ডান_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= ডান;
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
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}

	ফেরত true;
}
