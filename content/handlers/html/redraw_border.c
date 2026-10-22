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

#समावेश <stdbool.h>
#समावेश <stdlib.h>

#समावेश "utils/utils.h"
#समावेश "utils/log.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/css.h"

#समावेश "html/box.h"
#समावेश "html/private.h"


अटल plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
अटल plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
अटल plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
अटल plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
अटल plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
अटल plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


अटल अंतरभूत nserror
plot_clipped_rectangle(स्थिर काष्ठा redraw_context *ctx,
		       स्थिर plot_style_t *style,
		       स्थिर काष्ठा rect *clip,
		       काष्ठा rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	अगर ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} अन्यथा {
		res = NSERROR_OK;
	}
	वापस res;
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
अटल nserror
html_redraw_border_plot(स्थिर पूर्णांक side,
			स्थिर पूर्णांक *p,
			colour c,
			क्रमागत css_border_style_e style,
			पूर्णांक thickness,
			bool rectangular,
			स्थिर काष्ठा rect *clip,
			स्थिर काष्ठा redraw_context *ctx)
{
	पूर्णांक z[8]; /* Vertices of border part */
	अचिन्हित पूर्णांक light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	काष्ठा rect rect;

	अगर (c == NS_TRANSPARENT) {
		वापस res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	चयन (style) {
	हाल CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	हाल CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->पंक्ति(ctx, &plot_style_bdr, &rect);
		अवरोध;

	हाल CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	शेष:
		अगर (rectangular || thickness == 1) {

			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				अगर ((side == शीर्ष_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} अन्यथा {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} अन्यथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				अगर ((side == बायाँ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} अन्यथा {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} अन्यथा {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		अवरोध;

	हाल CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		अगर (res == NSERROR_OK) {
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
		अवरोध;

	हाल CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	हाल CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		अगर (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} अन्यथा {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		अगर ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} अन्यथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			अगर (res != NSERROR_OK) {
				वापस res;
			}

			/* Second part */
			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} अन्यथा {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} अन्यथा अगर (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == शीर्ष_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} अन्यथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == बायाँ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} अन्यथा {
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
			अगर (res == NSERROR_OK) {
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
		अवरोध;

	हाल CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	हाल CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		चयन (light) {
		हाल 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			अवरोध;
		हाल 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			अवरोध;
		हाल 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			अवरोध;
		हाल 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			अवरोध;
		शेष:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			अवरोध;
		}

		/* Render border */
		अगर ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} अन्यथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			अगर (res != NSERROR_OK) {
				वापस res;
			}

			/* Second part */
			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} अन्यथा {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} अन्यथा अगर (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			अगर (side == शीर्ष_2 || side == दायाँ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == शीर्ष_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} अन्यथा {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == बायाँ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} अन्यथा {
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
			अगर (res != NSERROR_OK) {
				वापस res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		अवरोध;
	}

	वापस res;
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
html_redraw_borders(काष्ठा box *box,
		    पूर्णांक x_parent,
		    पूर्णांक y_parent,
		    पूर्णांक p_width,
		    पूर्णांक p_height,
		    स्थिर काष्ठा rect *clip,
		    भग्न scale,
		    स्थिर काष्ठा redraw_context *ctx)
{
	अचिन्हित पूर्णांक sides[] = { बायाँ_2, दायाँ_2, शीर्ष_2, BOTTOM };
	पूर्णांक शीर्ष = box->border[शीर्ष_2].width;
	पूर्णांक दायाँ = box->border[दायाँ_2].width;
	पूर्णांक bottom = box->border[BOTTOM].width;
	पूर्णांक बायाँ = box->border[बायाँ_2].width;
	पूर्णांक x, y;
	अचिन्हित पूर्णांक i, side;
	पूर्णांक p[8]; /* Box border vertices */
	पूर्णांक z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	अगर (scale != 1.0) {
		शीर्ष *= scale;
		दायाँ *= scale;
		bottom *= scale;
		बायाँ *= scale;
		x *= scale;
		y *= scale;
	}

	निश्चित(box->style);

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
	p[0] = x - बायाँ;		p[1] = y - शीर्ष;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + दायाँ;	p[7] = y + p_height + bottom;	/* D */

	क्रम (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		अगर (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			जारी;
		}

		चयन (side) {
		हाल बायाँ_2:
			square_end_1 = (शीर्ष == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			अगर (nscss_color_is_transparent(box->border[शीर्ष_2].c) == false &&
			    box->border[शीर्ष_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= शीर्ष;
				square_end_1 = true;
			}
			अगर (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			अगर (res != NSERROR_OK) {
				वापस false;
			}
			अवरोध;

		हाल दायाँ_2:
			square_end_1 = (शीर्ष == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			अगर (nscss_color_is_transparent(box->border[शीर्ष_2].c) == false &&
			    box->border[शीर्ष_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= शीर्ष;
				square_end_1 = true;
			}
			अगर (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			अगर (res != NSERROR_OK) {
				वापस false;
			}
			अवरोध;

		हाल शीर्ष_2:
			अगर (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				जारी;
			}

			square_end_1 = (बायाँ == 0);
			square_end_2 = (दायाँ == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			अगर (box->border[शीर्ष_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[शीर्ष_2].c == box->border[बायाँ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += बायाँ;
				square_end_1 = true;
			}
			अगर (box->border[शीर्ष_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[शीर्ष_2].c == box->border[दायाँ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= दायाँ;
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
			अगर (res != NSERROR_OK) {
				वापस false;
			}
			अवरोध;

		हाल BOTTOM:
			अगर (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				जारी;
			}

			square_end_1 = (बायाँ == 0);
			square_end_2 = (दायाँ == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			अगर (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[बायाँ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += बायाँ;
				square_end_1 = true;
			}
			अगर (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[दायाँ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= दायाँ;
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
			अगर (res != NSERROR_OK) {
				वापस false;
			}
			अवरोध;

		शेष:
			निश्चित(side == शीर्ष_2 || side == BOTTOM ||
			       side == बायाँ_2 || side == दायाँ_2);
			अवरोध;
		}
	}

	वापस true;
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
html_redraw_inline_borders(काष्ठा box *box,
			   काष्ठा rect b,
			   स्थिर काष्ठा rect *clip,
			   भग्न scale,
			   bool first,
			   bool last,
			   स्थिर काष्ठा redraw_context *ctx)
{
	पूर्णांक शीर्ष = box->border[शीर्ष_2].width;
	पूर्णांक दायाँ = box->border[दायाँ_2].width;
	पूर्णांक bottom = box->border[BOTTOM].width;
	पूर्णांक बायाँ = box->border[बायाँ_2].width;
	colour col;
	पूर्णांक p[8]; /* Box border vertices */
	पूर्णांक z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	अगर (scale != 1.0) {
		शीर्ष *= scale;
		दायाँ *= scale;
		bottom *= scale;
		बायाँ *= scale;
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
	p[2] = first ? b.x0 + बायाँ : b.x0;	p[3] = b.y0 + शीर्ष;	/* B */
	p[4] = last ? b.x1 - दायाँ : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	निश्चित(box->style);

	/* Left */
	square_end_1 = (शीर्ष == 0);
	square_end_2 = (bottom == 0);
	अगर (बायाँ != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[बायाँ_2].c) == false) {
		col = nscss_color_to_ns(box->border[बायाँ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		अगर (nscss_color_is_transparent(box->border[शीर्ष_2].c) == false &&
		    box->border[शीर्ष_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= शीर्ष;
			square_end_1 = true;
		}

		अगर (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(बायाँ_2,
					      z,
					      col,
					      box->border[बायाँ_2].style,
					      बायाँ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}

	/* Right */
	square_end_1 = (शीर्ष == 0);
	square_end_2 = (bottom == 0);
	अगर (दायाँ != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[दायाँ_2].c) == false) {
		col = nscss_color_to_ns(box->border[दायाँ_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		अगर (nscss_color_is_transparent(box->border[शीर्ष_2].c) == false &&
		    box->border[शीर्ष_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= शीर्ष;
			square_end_1 = true;
		}

		अगर (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(दायाँ_2,
					      z,
					      col,
					      box->border[दायाँ_2].style,
					      दायाँ,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}

	/* Top */
	square_end_1 = (बायाँ == 0);
	square_end_2 = (दायाँ == 0);
	अगर (शीर्ष != 0 &&
	    nscss_color_is_transparent(box->border[शीर्ष_2].c) == false) {
		col = nscss_color_to_ns(box->border[शीर्ष_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		अगर (first &&
		    box->border[शीर्ष_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[शीर्ष_2].c == box->border[बायाँ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += बायाँ;
			square_end_1 = true;
		}

		अगर (last &&
		    box->border[शीर्ष_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[शीर्ष_2].c == box->border[दायाँ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= दायाँ;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(शीर्ष_2,
					      z,
					      col,
					      box->border[शीर्ष_2].style,
					      शीर्ष,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}

	/* Bottom */
	square_end_1 = (बायाँ == 0);
	square_end_2 = (दायाँ == 0);
	अगर (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		अगर (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[बायाँ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += बायाँ;
			square_end_1 = true;
		}

		अगर (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[दायाँ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= दायाँ;
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
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}

	वापस true;
}
