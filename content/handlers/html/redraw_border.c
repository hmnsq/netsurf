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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/css.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/private.h"


स्थिर plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
स्थिर plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
स्थिर plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
स्थिर plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
स्थिर plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
स्थिर plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


स्थिर अन्तःस्थ nserror
plot_clipped_rectangle(ध्रुव संरचना redraw_context *ctx,
		       ध्रुव plot_style_t *style,
		       ध्रुव संरचना rect *clip,
		       संरचना rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	सचे ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} अञ्ञथा {
		res = NSERROR_OK;
	}
	पटिनिवत्त res;
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
स्थिर nserror
html_redraw_border_plot(ध्रुव पूर्णाङ्क side,
			ध्रुव पूर्णाङ्क *p,
			colour c,
			गणना css_border_style_e style,
			पूर्णाङ्क thickness,
			bool rectangular,
			ध्रुव संरचना rect *clip,
			ध्रुव संरचना redraw_context *ctx)
{
	पूर्णाङ्क z[8]; /* Vertices of border part */
	unsigned पूर्णाङ्क light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	संरचना rect rect;

	सचे (c == NS_TRANSPARENT) {
		पटिनिवत्त res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	विकल्प (style) {
	पक्ष CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	पक्ष CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->पङ्क्तिः(ctx, &plot_style_bdr, &rect);
		भङ्ग;

	पक्ष CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	स्वभाव:
		सचे (rectangular || thickness == 1) {

			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				सचे ((side == शिखरम्_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} अञ्ञथा {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} अञ्ञथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				सचे ((side == वामः_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} अञ्ञथा {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} अञ्ञथा {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		भङ्ग;

	पक्ष CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		सचे (res == NSERROR_OK) {
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
		भङ्ग;

	पक्ष CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	पक्ष CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		सचे (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} अञ्ञथा {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		सचे ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} अञ्ञथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त res;
			}

			/* Second part */
			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} अञ्ञथा {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} अञ्ञथा सचे (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == शिखरम्_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} अञ्ञथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == वामः_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} अञ्ञथा {
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
			सचे (res == NSERROR_OK) {
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
		भङ्ग;

	पक्ष CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	पक्ष CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		विकल्प (light) {
		पक्ष 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			भङ्ग;
		पक्ष 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			भङ्ग;
		पक्ष 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			भङ्ग;
		पक्ष 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			भङ्ग;
		स्वभाव:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			भङ्ग;
		}

		/* Render border */
		सचे ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} अञ्ञथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त res;
			}

			/* Second part */
			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} अञ्ञथा {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} अञ्ञथा सचे (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			सचे (side == शिखरम्_2 || side == दक्षिणः_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == शिखरम्_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} अञ्ञथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == वामः_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} अञ्ञथा {
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		भङ्ग;
	}

	पटिनिवत्त res;
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
html_redraw_borders(संरचना box *box,
		    पूर्णाङ्क x_parent,
		    पूर्णाङ्क y_parent,
		    पूर्णाङ्क p_width,
		    पूर्णाङ्क p_height,
		    ध्रुव संरचना rect *clip,
		    भिन्न scale,
		    ध्रुव संरचना redraw_context *ctx)
{
	unsigned पूर्णाङ्क sides[] = { वामः_2, दक्षिणः_2, शिखरम्_2, BOTTOM };
	पूर्णाङ्क शिखरम् = box->border[शिखरम्_2].width;
	पूर्णाङ्क दक्षिणः = box->border[दक्षिणः_2].width;
	पूर्णाङ्क bottom = box->border[BOTTOM].width;
	पूर्णाङ्क वामः = box->border[वामः_2].width;
	पूर्णाङ्क x, y;
	unsigned पूर्णाङ्क i, side;
	पूर्णाङ्क p[8]; /* Box border vertices */
	पूर्णाङ्क z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	सचे (scale != 1.0) {
		शिखरम् *= scale;
		दक्षिणः *= scale;
		bottom *= scale;
		वामः *= scale;
		x *= scale;
		y *= scale;
	}

	प्रतिज्ञा(box->style);

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
	p[0] = x - वामः;		p[1] = y - शिखरम्;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + दक्षिणः;	p[7] = y + p_height + bottom;	/* D */

	अत्थाय (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		सचे (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			अनुवर्त;
		}

		विकल्प (side) {
		पक्ष वामः_2:
			square_end_1 = (शिखरम् == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			सचे (nscss_color_is_transparent(box->border[शिखरम्_2].c) == false &&
			    box->border[शिखरम्_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= शिखरम्;
				square_end_1 = true;
			}
			सचे (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
			भङ्ग;

		पक्ष दक्षिणः_2:
			square_end_1 = (शिखरम् == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			सचे (nscss_color_is_transparent(box->border[शिखरम्_2].c) == false &&
			    box->border[शिखरम्_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= शिखरम्;
				square_end_1 = true;
			}
			सचे (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
			भङ्ग;

		पक्ष शिखरम्_2:
			सचे (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				अनुवर्त;
			}

			square_end_1 = (वामः == 0);
			square_end_2 = (दक्षिणः == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			सचे (box->border[शिखरम्_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[शिखरम्_2].c == box->border[वामः_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += वामः;
				square_end_1 = true;
			}
			सचे (box->border[शिखरम्_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[शिखरम्_2].c == box->border[दक्षिणः_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= दक्षिणः;
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
			भङ्ग;

		पक्ष BOTTOM:
			सचे (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				अनुवर्त;
			}

			square_end_1 = (वामः == 0);
			square_end_2 = (दक्षिणः == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			सचे (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[वामः_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += वामः;
				square_end_1 = true;
			}
			सचे (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[दक्षिणः_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= दक्षिणः;
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
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
			भङ्ग;

		स्वभाव:
			प्रतिज्ञा(side == शिखरम्_2 || side == BOTTOM ||
			       side == वामः_2 || side == दक्षिणः_2);
			भङ्ग;
		}
	}

	पटिनिवत्त true;
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
html_redraw_inline_borders(संरचना box *box,
			   संरचना rect b,
			   ध्रुव संरचना rect *clip,
			   भिन्न scale,
			   bool first,
			   bool last,
			   ध्रुव संरचना redraw_context *ctx)
{
	पूर्णाङ्क शिखरम् = box->border[शिखरम्_2].width;
	पूर्णाङ्क दक्षिणः = box->border[दक्षिणः_2].width;
	पूर्णाङ्क bottom = box->border[BOTTOM].width;
	पूर्णाङ्क वामः = box->border[वामः_2].width;
	colour col;
	पूर्णाङ्क p[8]; /* Box border vertices */
	पूर्णाङ्क z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	सचे (scale != 1.0) {
		शिखरम् *= scale;
		दक्षिणः *= scale;
		bottom *= scale;
		वामः *= scale;
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
	p[2] = first ? b.x0 + वामः : b.x0;	p[3] = b.y0 + शिखरम्;	/* B */
	p[4] = last ? b.x1 - दक्षिणः : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	प्रतिज्ञा(box->style);

	/* Left */
	square_end_1 = (शिखरम् == 0);
	square_end_2 = (bottom == 0);
	सचे (वामः != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[वामः_2].c) == false) {
		col = nscss_color_to_ns(box->border[वामः_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		सचे (nscss_color_is_transparent(box->border[शिखरम्_2].c) == false &&
		    box->border[शिखरम्_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= शिखरम्;
			square_end_1 = true;
		}

		सचे (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(वामः_2,
					      z,
					      col,
					      box->border[वामः_2].style,
					      वामः,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}

	/* Right */
	square_end_1 = (शिखरम् == 0);
	square_end_2 = (bottom == 0);
	सचे (दक्षिणः != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[दक्षिणः_2].c) == false) {
		col = nscss_color_to_ns(box->border[दक्षिणः_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		सचे (nscss_color_is_transparent(box->border[शिखरम्_2].c) == false &&
		    box->border[शिखरम्_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= शिखरम्;
			square_end_1 = true;
		}

		सचे (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(दक्षिणः_2,
					      z,
					      col,
					      box->border[दक्षिणः_2].style,
					      दक्षिणः,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}

	/* Top */
	square_end_1 = (वामः == 0);
	square_end_2 = (दक्षिणः == 0);
	सचे (शिखरम् != 0 &&
	    nscss_color_is_transparent(box->border[शिखरम्_2].c) == false) {
		col = nscss_color_to_ns(box->border[शिखरम्_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		सचे (first &&
		    box->border[शिखरम्_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[शिखरम्_2].c == box->border[वामः_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += वामः;
			square_end_1 = true;
		}

		सचे (last &&
		    box->border[शिखरम्_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[शिखरम्_2].c == box->border[दक्षिणः_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= दक्षिणः;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(शिखरम्_2,
					      z,
					      col,
					      box->border[शिखरम्_2].style,
					      शिखरम्,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}

	/* Bottom */
	square_end_1 = (वामः == 0);
	square_end_2 = (दक्षिणः == 0);
	सचे (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		सचे (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[वामः_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += वामः;
			square_end_1 = true;
		}

		सचे (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[दक्षिणः_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= दक्षिणः;
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
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}

	पटिनिवत्त true;
}
