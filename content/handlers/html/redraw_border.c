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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/css.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/private.h"


நிலையான plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
நிலையான plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
நிலையான plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
நிலையான plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
நிலையான plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
நிலையான plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


நிலையான உள்வரி nserror
plot_clipped_rectangle(மாறிலி கட்டமைப்பு redraw_context *ctx,
		       மாறிலி plot_style_t *style,
		       மாறிலி கட்டமைப்பு rect *clip,
		       கட்டமைப்பு rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	எனில் ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} இல்லையெனில் {
		res = NSERROR_OK;
	}
	திரும்பு res;
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
நிலையான nserror
html_redraw_border_plot(மாறிலி முழு side,
			மாறிலி முழு *p,
			colour c,
			எண்ணிக்கை css_border_style_e style,
			முழு thickness,
			bool rectangular,
			மாறிலி கட்டமைப்பு rect *clip,
			மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	முழு z[8]; /* Vertices of border part */
	unsigned முழு light = side;
	plot_style_t *plot_style_bdr_in;
	plot_style_t *plot_style_bdr_out;
	nserror res = NSERROR_OK;
	கட்டமைப்பு rect rect;

	எனில் (c == NS_TRANSPARENT) {
		திரும்பு res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_colour = c;
	plot_style_bdr.stroke_width = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_colour = c;
	plot_style_fillbdr_dark.fill_colour = darken_colour(c);
	plot_style_fillbdr_light.fill_colour = lighten_colour(c);
	plot_style_fillbdr_ddark.fill_colour = double_darken_colour(c);
	plot_style_fillbdr_dlight.fill_colour = double_lighten_colour(c);

	தேர்வு (style) {
	நிலை CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	நிலை CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->வரி(ctx, &plot_style_bdr, &rect);
		நிறுத்து;

	நிலை CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	இயல்பு:
		எனில் (rectangular || thickness == 1) {

			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				எனில் ((side == உச்சி_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} இல்லையெனில் {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} இல்லையெனில் {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				எனில் ((side == இடது_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} இல்லையெனில் {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} இல்லையெனில் {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		நிறுத்து;

	நிலை CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		எனில் (res == NSERROR_OK) {
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
		நிறுத்து;

	நிலை CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	நிலை CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		எனில் (light <= 1) {
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_light;
		} இல்லையெனில் {
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
		}

		/* Render border */
		எனில் ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} இல்லையெனில் {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}

			/* Second part */
			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} இல்லையெனில் {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} இல்லையெனில் எனில் (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == உச்சி_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} இல்லையெனில் {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == இடது_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} இல்லையெனில் {
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
			எனில் (res == NSERROR_OK) {
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
		நிறுத்து;

	நிலை CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	நிலை CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		தேர்வு (light) {
		நிலை 0:
			plot_style_bdr_in = &plot_style_fillbdr_light;
			plot_style_bdr_out = &plot_style_fillbdr_dlight;
			நிறுத்து;
		நிலை 1:
			plot_style_bdr_in = &plot_style_fillbdr_ddark;
			plot_style_bdr_out = &plot_style_fillbdr_dark;
			நிறுத்து;
		நிலை 2:
			plot_style_bdr_in = &plot_style_fillbdr_dark;
			plot_style_bdr_out = &plot_style_fillbdr_ddark;
			நிறுத்து;
		நிலை 3:
			plot_style_bdr_in = &plot_style_fillbdr_dlight;
			plot_style_bdr_out = &plot_style_fillbdr_light;
			நிறுத்து;
		இயல்பு:
			plot_style_bdr_in = &plot_style_fillbdr;
			plot_style_bdr_out = &plot_style_fillbdr;
			நிறுத்து;
		}

		/* Render border */
		எனில் ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} இல்லையெனில் {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_in,
						     clip,
						     &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}

			/* Second part */
			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} இல்லையெனில் {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_out,
						     clip,
						     &rect);
		} இல்லையெனில் எனில் (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			எனில் (side == உச்சி_2 || side == வலது_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == உச்சி_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_in,
							     clip,
							     &rect);
			} இல்லையெனில் {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == இடது_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_out,
							     clip,
							     &rect);
			}
		} இல்லையெனில் {
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
			எனில் (res != NSERROR_OK) {
				திரும்பு res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_out, z, 4);
		}
		நிறுத்து;
	}

	திரும்பு res;
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
html_redraw_borders(கட்டமைப்பு box *box,
		    முழு x_parent,
		    முழு y_parent,
		    முழு p_width,
		    முழு p_height,
		    மாறிலி கட்டமைப்பு rect *clip,
		    தசம scale,
		    மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	unsigned முழு sides[] = { இடது_2, வலது_2, உச்சி_2, BOTTOM };
	முழு உச்சி = box->border[உச்சி_2].width;
	முழு வலது = box->border[வலது_2].width;
	முழு bottom = box->border[BOTTOM].width;
	முழு இடது = box->border[இடது_2].width;
	முழு x, y;
	unsigned முழு i, side;
	முழு p[8]; /* Box border vertices */
	முழு z[8]; /* Border vertices */
	bool square_end_1 = false;
	bool square_end_2 = false;
	nserror res;

	x = x_parent + box->x;
	y = y_parent + box->y;

	எனில் (scale != 1.0) {
		உச்சி *= scale;
		வலது *= scale;
		bottom *= scale;
		இடது *= scale;
		x *= scale;
		y *= scale;
	}

	ப்ரதிஜ்ஞா(box->style);

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
	p[0] = x - இடது;		p[1] = y - உச்சி;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_width;		p[5] = y + p_height;		/* C */
	p[6] = x + p_width + வலது;	p[7] = y + p_height + bottom;	/* D */

	க்கு (i = 0; i != 4; i++) {
		colour col = 0;
		side = sides[i]; /* plot order */

		எனில் (box->border[side].width == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			தொடர்;
		}

		தேர்வு (side) {
		நிலை இடது_2:
			square_end_1 = (உச்சி == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			எனில் (nscss_color_is_transparent(box->border[உச்சி_2].c) == false &&
			    box->border[உச்சி_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= உச்சி;
				square_end_1 = true;
			}
			எனில் (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
			நிறுத்து;

		நிலை வலது_2:
			square_end_1 = (உச்சி == 0);
			square_end_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			எனில் (nscss_color_is_transparent(box->border[உச்சி_2].c) == false &&
			    box->border[உச்சி_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= உச்சி;
				square_end_1 = true;
			}
			எனில் (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
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
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
			நிறுத்து;

		நிலை உச்சி_2:
			எனில் (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				தொடர்;
			}

			square_end_1 = (இடது == 0);
			square_end_2 = (வலது == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			எனில் (box->border[உச்சி_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[உச்சி_2].c == box->border[இடது_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += இடது;
				square_end_1 = true;
			}
			எனில் (box->border[உச்சி_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[உச்சி_2].c == box->border[வலது_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= வலது;
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
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
			நிறுத்து;

		நிலை BOTTOM:
			எனில் (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				தொடர்;
			}

			square_end_1 = (இடது == 0);
			square_end_2 = (வலது == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			எனில் (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[இடது_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += இடது;
				square_end_1 = true;
			}
			எனில் (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[வலது_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= வலது;
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
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
			நிறுத்து;

		இயல்பு:
			ப்ரதிஜ்ஞா(side == உச்சி_2 || side == BOTTOM ||
			       side == இடது_2 || side == வலது_2);
			நிறுத்து;
		}
	}

	திரும்பு true;
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
html_redraw_inline_borders(கட்டமைப்பு box *box,
			   கட்டமைப்பு rect b,
			   மாறிலி கட்டமைப்பு rect *clip,
			   தசம scale,
			   bool first,
			   bool last,
			   மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	முழு உச்சி = box->border[உச்சி_2].width;
	முழு வலது = box->border[வலது_2].width;
	முழு bottom = box->border[BOTTOM].width;
	முழு இடது = box->border[இடது_2].width;
	colour col;
	முழு p[8]; /* Box border vertices */
	முழு z[8]; /* Border vertices */
	bool square_end_1;
	bool square_end_2;
	nserror res;

	எனில் (scale != 1.0) {
		உச்சி *= scale;
		வலது *= scale;
		bottom *= scale;
		இடது *= scale;
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
	p[2] = first ? b.x0 + இடது : b.x0;	p[3] = b.y0 + உச்சி;	/* B */
	p[4] = last ? b.x1 - வலது : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ப்ரதிஜ்ஞா(box->style);

	/* Left */
	square_end_1 = (உச்சி == 0);
	square_end_2 = (bottom == 0);
	எனில் (இடது != 0 &&
	    first &&
	    nscss_color_is_transparent(box->border[இடது_2].c) == false) {
		col = nscss_color_to_ns(box->border[இடது_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		எனில் (nscss_color_is_transparent(box->border[உச்சி_2].c) == false &&
		    box->border[உச்சி_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= உச்சி;
			square_end_1 = true;
		}

		எனில் (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(இடது_2,
					      z,
					      col,
					      box->border[இடது_2].style,
					      இடது,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}

	/* Right */
	square_end_1 = (உச்சி == 0);
	square_end_2 = (bottom == 0);
	எனில் (வலது != 0 &&
	    last &&
	    nscss_color_is_transparent(box->border[வலது_2].c) == false) {
		col = nscss_color_to_ns(box->border[வலது_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		எனில் (nscss_color_is_transparent(box->border[உச்சி_2].c) == false &&
		    box->border[உச்சி_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= உச்சி;
			square_end_1 = true;
		}

		எனில் (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(வலது_2,
					      z,
					      col,
					      box->border[வலது_2].style,
					      வலது,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}

	/* Top */
	square_end_1 = (இடது == 0);
	square_end_2 = (வலது == 0);
	எனில் (உச்சி != 0 &&
	    nscss_color_is_transparent(box->border[உச்சி_2].c) == false) {
		col = nscss_color_to_ns(box->border[உச்சி_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		எனில் (first &&
		    box->border[உச்சி_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[உச்சி_2].c == box->border[இடது_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += இடது;
			square_end_1 = true;
		}

		எனில் (last &&
		    box->border[உச்சி_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[உச்சி_2].c == box->border[வலது_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= வலது;
			square_end_2 = true;
		}

		res = html_redraw_border_plot(உச்சி_2,
					      z,
					      col,
					      box->border[உச்சி_2].style,
					      உச்சி,
					      square_end_1 && square_end_2,
					      clip,
					      ctx);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}

	/* Bottom */
	square_end_1 = (இடது == 0);
	square_end_2 = (வலது == 0);
	எனில் (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		col = nscss_color_to_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		எனில் (first &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[இடது_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += இடது;
			square_end_1 = true;
		}

		எனில் (last &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[வலது_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= வலது;
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
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}

	திரும்பு true;
}
