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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/css.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"


ਸਥਿਰ plot_style_t plot_style_bdr = {
	.stroke_type = PLOT_OP_TYPE_DASH,
};
ਸਥਿਰ plot_style_t plot_style_fillbdr = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ਸਥਿਰ plot_style_t plot_style_fillbdr_dark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ਸਥਿਰ plot_style_t plot_style_fillbdr_light = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ਸਥਿਰ plot_style_t plot_style_fillbdr_ddark = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};
ਸਥਿਰ plot_style_t plot_style_fillbdr_dlight = {
	.fill_type = PLOT_OP_TYPE_SOLID,
};


ਸਥਿਰ ਸਤਰਅੰਦਰ nserror
plot_clipped_rectangle(ਅਚਲ ਬਣਤਰ redraw_context *ctx,
		       ਅਚਲ plot_style_t *style,
		       ਅਚਲ ਬਣਤਰ rect *clip,
		       ਬਣਤਰ rect *rect)
{
	nserror res;

	rect->x0 = (clip->x0 > rect->x0) ? clip->x0 : rect->x0;
	rect->y0 = (clip->y0 > rect->y0) ? clip->y0 : rect->y0;
	rect->x1 = (clip->x1 < rect->x1) ? clip->x1 : rect->x1;
	rect->y1 = (clip->y1 < rect->y1) ? clip->y1 : rect->y1;
	ਜੇ ((rect->x0 < rect->x1) && (rect->y0 < rect->y1)) {
		/* valid clip rectangles only */
		res = ctx->plot->rectangle(ctx, style, rect);
	} ਨਹੀਂਤਾਂ {
		res = NSERROR_OK;
	}
	ਵਾਪਸ res;
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
ਸਥਿਰ nserror
html_redraw_border_plot(ਅਚਲ ਪੂਰਨ side,
			ਅਚਲ ਪੂਰਨ *p,
			ਰੰਗ c,
			ਗਿਣਤੀ css_border_style_e style,
			ਪੂਰਨ thickness,
			bool rectangular,
			ਅਚਲ ਬਣਤਰ rect *clip,
			ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਪੂਰਨ z[8]; /* Vertices of border part */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ light = side;
	plot_style_t *plot_style_bdr_ਵਿੱਚ;
	plot_style_t *plot_style_bdr_ਬਾਹਰਲਾ;
	nserror res = NSERROR_OK;
	ਬਣਤਰ rect rect;

	ਜੇ (c == NS_TRANSPARENT) {
		ਵਾਪਸ res;
	}

	plot_style_bdr.stroke_type = PLOT_OP_TYPE_DASH;
	plot_style_bdr.stroke_ਰੰਗ = c;
	plot_style_bdr.stroke_ਚੌੜਾਈ = (thickness << PLOT_STYLE_RADIX);
	plot_style_fillbdr.fill_ਰੰਗ = c;
	plot_style_fillbdr_dark.fill_ਰੰਗ = darken_ਰੰਗ(c);
	plot_style_fillbdr_light.fill_ਰੰਗ = lighten_ਰੰਗ(c);
	plot_style_fillbdr_ddark.fill_ਰੰਗ = double_darken_ਰੰਗ(c);
	plot_style_fillbdr_dlight.fill_ਰੰਗ = double_lighten_ਰੰਗ(c);

	ਚੋਣ (style) {
	ਹਾਲਤ CSS_BORDER_STYLE_DOTTED:
		plot_style_bdr.stroke_type = PLOT_OP_TYPE_DOT;
		fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_DASHED:
		rect.x0 = (p[0] + p[2]) / 2;
		rect.y0 = (p[1] + p[3]) / 2;
		rect.x1 = (p[4] + p[6]) / 2;
		rect.y1 = (p[5] + p[7]) / 2;
		res = ctx->plot->ਸਤਰ(ctx, &plot_style_bdr, &rect);
		ਰੋਕੋ;

	ਹਾਲਤ CSS_BORDER_STYLE_SOLID:
		/* solid is the default */
	ਮੂਲ:
		ਜੇ (rectangular || thickness == 1) {

			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				ਜੇ ((side == ਸਿਖਰ_2) &&
				    (p[4] - p[6] != 0)) {
					rect.x1 = p[4];
				} ਨਹੀਂਤਾਂ {
					rect.x1 = p[6];
				}
				rect.y1 = p[7];
			} ਨਹੀਂਤਾਂ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				ਜੇ ((side == ਖੱਬਾ_2) &&
				    (p[1] - p[3] != 0)) {
					rect.y1 = p[1];
				} ਨਹੀਂਤਾਂ {
					rect.y1 = p[3];
				}
			}
			res = plot_clipped_rectangle(ctx,
						     &plot_style_fillbdr,
						     clip,
						     &rect);
		} ਨਹੀਂਤਾਂ {
			res = ctx->plot->polygon(ctx, &plot_style_fillbdr, p, 4);
		}
		ਰੋਕੋ;

	ਹਾਲਤ CSS_BORDER_STYLE_DOUBLE:
		z[0] = p[0];
		z[1] = p[1];
		z[2] = (p[0] * 2 + p[2]) / 3;
		z[3] = (p[1] * 2 + p[3]) / 3;
		z[4] = (p[6] * 2 + p[4]) / 3;
		z[5] = (p[7] * 2 + p[5]) / 3;
		z[6] = p[6];
		z[7] = p[7];
		res = ctx->plot->polygon(ctx, &plot_style_fillbdr, z, 4);
		ਜੇ (res == NSERROR_OK) {
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
		ਰੋਕੋ;

	ਹਾਲਤ CSS_BORDER_STYLE_GROOVE:
		light = 3 - light;
		fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_RIDGE:
		/* choose correct colours for each part of the border line */
		ਜੇ (light <= 1) {
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_dark;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_light;
		} ਨਹੀਂਤਾਂ {
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_light;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_dark;
		}

		/* Render border */
		ਜੇ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ਨਹੀਂਤਾਂ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ਵਿੱਚ,
						     clip,
						     &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}

			/* Second part */
			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ਨਹੀਂਤਾਂ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ਬਾਹਰਲਾ,
						     clip,
						     &rect);
		} ਨਹੀਂਤਾਂ ਜੇ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ਸਿਖਰ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;

				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ਵਿੱਚ,
							     clip,
							     &rect);
			} ਨਹੀਂਤਾਂ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ਖੱਬਾ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ਬਾਹਰਲਾ,
							     clip,
							     &rect);
			}
		} ਨਹੀਂਤਾਂ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_ਵਿੱਚ, z, 4);
			ਜੇ (res == NSERROR_OK) {
				z[0] = p[2];
				z[1] = p[3];
				z[6] = p[4];
				z[7] = p[5];
				res = ctx->plot->polygon(ctx,
							 plot_style_bdr_ਬਾਹਰਲਾ,
							 z,
							 4);
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ CSS_BORDER_STYLE_INSET:
		light = (light + 2) % 4;
		fallthrough;
	ਹਾਲਤ CSS_BORDER_STYLE_OUTSET:
		/* choose correct colours for each part of the border line */
		ਚੋਣ (light) {
		ਹਾਲਤ 0:
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_light;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_dlight;
			ਰੋਕੋ;
		ਹਾਲਤ 1:
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_ddark;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_dark;
			ਰੋਕੋ;
		ਹਾਲਤ 2:
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_dark;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_ddark;
			ਰੋਕੋ;
		ਹਾਲਤ 3:
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr_dlight;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr_light;
			ਰੋਕੋ;
		ਮੂਲ:
			plot_style_bdr_ਵਿੱਚ = &plot_style_fillbdr;
			plot_style_bdr_ਬਾਹਰਲਾ = &plot_style_fillbdr;
			ਰੋਕੋ;
		}

		/* Render border */
		ਜੇ ((rectangular || thickness == 2) && thickness != 1) {
			/* Border made up from two parts and can be
			 * plotted with rectangles
			 */

			/* First part */
			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = (p[0] + p[2]) / 2;
				rect.y0 = (p[1] + p[3]) / 2;
				rect.x1 = p[6];
				rect.y1 = p[7];
			} ਨਹੀਂਤਾਂ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = (p[0] + p[2]) / 2;
				rect.y1 = (p[1] + p[3]) / 2;
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ਵਿੱਚ,
						     clip,
						     &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}

			/* Second part */
			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = (p[6] + p[4]) / 2;
				rect.y1 = (p[7] + p[5]) / 2;
			} ਨਹੀਂਤਾਂ {
				rect.x0 = (p[6] + p[4]) / 2;
				rect.y0 = (p[7] + p[5]) / 2;
				rect.x1 = p[2];
				rect.y1 = p[3];
			}
			res = plot_clipped_rectangle(ctx,
						     plot_style_bdr_ਬਾਹਰਲਾ,
						     clip,
						     &rect);
		} ਨਹੀਂਤਾਂ ਜੇ (thickness == 1) {
			/* Border made up from one part which can be
			 * plotted as a rectangle
			 */

			ਜੇ (side == ਸਿਖਰ_2 || side == ਸੱਜਾ_2) {
				rect.x0 = p[2];
				rect.y0 = p[3];
				rect.x1 = p[6];
				rect.y1 = p[7];
				rect.x1 = ((side == ਸਿਖਰ_2) && (p[4] - p[6] != 0)) ?
					rect.x1 + p[4] - p[6] : rect.x1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ਵਿੱਚ,
							     clip,
							     &rect);
			} ਨਹੀਂਤਾਂ {
				rect.x0 = p[6];
				rect.y0 = p[7];
				rect.x1 = p[2];
				rect.y1 = p[3];
				rect.y1 = ((side == ਖੱਬਾ_2) && (p[1] - p[3] != 0)) ?
					rect.y1 + p[1] - p[3] : rect.y1;
				res = plot_clipped_rectangle(ctx,
							     plot_style_bdr_ਬਾਹਰਲਾ,
							     clip,
							     &rect);
			}
		} ਨਹੀਂਤਾਂ {
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
			res = ctx->plot->polygon(ctx, plot_style_bdr_ਵਿੱਚ, z, 4);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ res;
			}
			z[0] = p[2];
			z[1] = p[3];
			z[6] = p[4];
			z[7] = p[5];
			res = ctx->plot->polygon(ctx, plot_style_bdr_ਬਾਹਰਲਾ, z, 4);
		}
		ਰੋਕੋ;
	}

	ਵਾਪਸ res;
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
html_redraw_borders(ਬਣਤਰ box *box,
		    ਪੂਰਨ x_ਮਾਪਾ,
		    ਪੂਰਨ y_ਮਾਪਾ,
		    ਪੂਰਨ p_ਚੌੜਾਈ,
		    ਪੂਰਨ p_ਉਚਾਈ,
		    ਅਚਲ ਬਣਤਰ rect *clip,
		    ਦਸ਼ਮਲਵ scale,
		    ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sides[] = { ਖੱਬਾ_2, ਸੱਜਾ_2, ਸਿਖਰ_2, BOTTOM };
	ਪੂਰਨ ਸਿਖਰ = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
	ਪੂਰਨ ਸੱਜਾ = box->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
	ਪੂਰਨ bottom = box->border[BOTTOM].ਚੌੜਾਈ;
	ਪੂਰਨ ਖੱਬਾ = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	ਪੂਰਨ x, y;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, side;
	ਪੂਰਨ p[8]; /* Box border vertices */
	ਪੂਰਨ z[8]; /* Border vertices */
	bool ਵਰਗਫਲ_ਸਿਰਾ_1 = false;
	bool ਵਰਗਫਲ_ਸਿਰਾ_2 = false;
	nserror res;

	x = x_ਮਾਪਾ + box->x;
	y = y_ਮਾਪਾ + box->y;

	ਜੇ (scale != 1.0) {
		ਸਿਖਰ *= scale;
		ਸੱਜਾ *= scale;
		bottom *= scale;
		ਖੱਬਾ *= scale;
		x *= scale;
		y *= scale;
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(box->style);

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
	p[0] = x - ਖੱਬਾ;		p[1] = y - ਸਿਖਰ;			/* A */
	p[2] = x;			p[3] = y;			/* B */
	p[4] = x + p_ਚੌੜਾਈ;		p[5] = y + p_ਉਚਾਈ;		/* C */
	p[6] = x + p_ਚੌੜਾਈ + ਸੱਜਾ;	p[7] = y + p_ਉਚਾਈ + bottom;	/* D */

	ਲਈ (i = 0; i != 4; i++) {
		ਰੰਗ ਖਾਨਾ = 0;
		side = sides[i]; /* plot order */

		ਜੇ (box->border[side].ਚੌੜਾਈ == 0 ||
		    nscss_color_is_transparent(box->border[side].c)) {
			ਜਾਰੀ;
		}

		ਚੋਣ (side) {
		ਹਾਲਤ ਖੱਬਾ_2:
			ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਸਿਖਰ == 0);
			ਵਰਗਫਲ_ਸਿਰਾ_2 = (bottom == 0);

			z[0] = p[0];	z[1] = p[7];
			z[2] = p[2];	z[3] = p[5];
			z[4] = p[2];	z[5] = p[3];
			z[6] = p[0];	z[7] = p[1];

			ਜੇ (nscss_color_is_transparent(box->border[ਸਿਖਰ_2].c) == false &&
			    box->border[ਸਿਖਰ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[5] -= ਸਿਖਰ;
				ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
			}
			ਜੇ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[3] += bottom;
				ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
			}

			ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ਖਾਨਾ,
						      box->border[side].style,
						      box->border[side].ਚੌੜਾਈ * scale,
						      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
						      clip,
						      ctx);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;

		ਹਾਲਤ ਸੱਜਾ_2:
			ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਸਿਖਰ == 0);
			ਵਰਗਫਲ_ਸਿਰਾ_2 = (bottom == 0);

			z[0] = p[6];	z[1] = p[1];
			z[2] = p[4];	z[3] = p[3];
			z[4] = p[4];	z[5] = p[5];
			z[6] = p[6];	z[7] = p[7];

			ਜੇ (nscss_color_is_transparent(box->border[ਸਿਖਰ_2].c) == false &&
			    box->border[ਸਿਖਰ_2].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang top corner fully,
				 * if top border is opaque
				 */
				z[3] -= ਸਿਖਰ;
				ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
			}
			ਜੇ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
			    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
				/* make border overhang bottom corner fully,
				 * if bottom border is opaque
				 */
				z[5] += bottom;
				ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
			}

			ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ਖਾਨਾ,
						      box->border[side].style,
						      box->border[side].ਚੌੜਾਈ * scale,
						      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
						      clip,
						      ctx);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;

		ਹਾਲਤ ਸਿਖਰ_2:
			ਜੇ (clip->y0 > p[3]) {
				/* clip rectangle is below border; nothing to
				 * plot
				 */
				ਜਾਰੀ;
			}

			ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਖੱਬਾ == 0);
			ਵਰਗਫਲ_ਸਿਰਾ_2 = (ਸੱਜਾ == 0);

			z[0] = p[2];	z[1] = p[3];
			z[2] = p[0];	z[3] = p[1];
			z[4] = p[6];	z[5] = p[1];
			z[6] = p[4];	z[7] = p[3];

			ਜੇ (box->border[ਸਿਖਰ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ਸਿਖਰ_2].c == box->border[ਖੱਬਾ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[2] += ਖੱਬਾ;
				ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
			}
			ਜੇ (box->border[ਸਿਖਰ_2].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[ਸਿਖਰ_2].c == box->border[ਸੱਜਾ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[4] -= ਸੱਜਾ;
				ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
			}

			ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ਖਾਨਾ,
						      box->border[side].style,
						      box->border[side].ਚੌੜਾਈ * scale,
						      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
						      clip,
						      ctx);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;

		ਹਾਲਤ BOTTOM:
			ਜੇ (clip->y1 < p[5]) {
				/* clip rectangle is above border; nothing to
				 * plot
				 */
				ਜਾਰੀ;
			}

			ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਖੱਬਾ == 0);
			ਵਰਗਫਲ_ਸਿਰਾ_2 = (ਸੱਜਾ == 0);

			z[0] = p[4];	z[1] = p[5];
			z[2] = p[6];	z[3] = p[7];
			z[4] = p[0];	z[5] = p[7];
			z[6] = p[2];	z[7] = p[5];

			ਜੇ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ਖੱਬਾ_2].c) {
				/* don't bother overlapping left corner if
				 * it's the same colour anyway
				 */
				z[4] += ਖੱਬਾ;
				ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
			}
			ਜੇ (box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
			    box->border[BOTTOM].c == box->border[ਸੱਜਾ_2].c) {
				/* don't bother overlapping right corner if
				 * it's the same colour anyway
				 */
				z[2] -= ਸੱਜਾ;
				ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
			}

			ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[side].c);

			res = html_redraw_border_plot(side,
						      z,
						      ਖਾਨਾ,
						      box->border[side].style,
						      box->border[side].ਚੌੜਾਈ * scale,
						      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
						      clip,
						      ctx);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
			ਰੋਕੋ;

		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(side == ਸਿਖਰ_2 || side == BOTTOM ||
			       side == ਖੱਬਾ_2 || side == ਸੱਜਾ_2);
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ true;
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
html_redraw_inline_borders(ਬਣਤਰ box *box,
			   ਬਣਤਰ rect b,
			   ਅਚਲ ਬਣਤਰ rect *clip,
			   ਦਸ਼ਮਲਵ scale,
			   bool ਪਹਿਲਾ,
			   bool ਆਖਰੀ,
			   ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਪੂਰਨ ਸਿਖਰ = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
	ਪੂਰਨ ਸੱਜਾ = box->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
	ਪੂਰਨ bottom = box->border[BOTTOM].ਚੌੜਾਈ;
	ਪੂਰਨ ਖੱਬਾ = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
	ਰੰਗ ਖਾਨਾ;
	ਪੂਰਨ p[8]; /* Box border vertices */
	ਪੂਰਨ z[8]; /* Border vertices */
	bool ਵਰਗਫਲ_ਸਿਰਾ_1;
	bool ਵਰਗਫਲ_ਸਿਰਾ_2;
	nserror res;

	ਜੇ (scale != 1.0) {
		ਸਿਖਰ *= scale;
		ਸੱਜਾ *= scale;
		bottom *= scale;
		ਖੱਬਾ *= scale;
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
	p[2] = ਪਹਿਲਾ ? b.x0 + ਖੱਬਾ : b.x0;	p[3] = b.y0 + ਸਿਖਰ;	/* B */
	p[4] = ਆਖਰੀ ? b.x1 - ਸੱਜਾ : b.x1;	p[5] = b.y1 - bottom;	/* C */
	p[6] = b.x1;				p[7] = b.y1;		/* D */

	ਪ੍ਰਤਿਜ੍ਞਾ(box->style);

	/* Left */
	ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਸਿਖਰ == 0);
	ਵਰਗਫਲ_ਸਿਰਾ_2 = (bottom == 0);
	ਜੇ (ਖੱਬਾ != 0 &&
	    ਪਹਿਲਾ &&
	    nscss_color_is_transparent(box->border[ਖੱਬਾ_2].c) == false) {
		ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[ਖੱਬਾ_2].c);

		z[0] = p[0];	z[1] = p[7];
		z[2] = p[2];	z[3] = p[5];
		z[4] = p[2];	z[5] = p[3];
		z[6] = p[0];	z[7] = p[1];

		ਜੇ (nscss_color_is_transparent(box->border[ਸਿਖਰ_2].c) == false &&
		    box->border[ਸਿਖਰ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[5] -= ਸਿਖਰ;
			ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
		}

		ਜੇ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[3] += bottom;
			ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
		}

		res = html_redraw_border_plot(ਖੱਬਾ_2,
					      z,
					      ਖਾਨਾ,
					      box->border[ਖੱਬਾ_2].style,
					      ਖੱਬਾ,
					      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
					      clip,
					      ctx);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}

	/* Right */
	ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਸਿਖਰ == 0);
	ਵਰਗਫਲ_ਸਿਰਾ_2 = (bottom == 0);
	ਜੇ (ਸੱਜਾ != 0 &&
	    ਆਖਰੀ &&
	    nscss_color_is_transparent(box->border[ਸੱਜਾ_2].c) == false) {
		ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[ਸੱਜਾ_2].c);

		z[0] = p[6];	z[1] = p[1];
		z[2] = p[4];	z[3] = p[3];
		z[4] = p[4];	z[5] = p[5];
		z[6] = p[6];	z[7] = p[7];

		ਜੇ (nscss_color_is_transparent(box->border[ਸਿਖਰ_2].c) == false &&
		    box->border[ਸਿਖਰ_2].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang top corner fully,
			 * if top border is opaque
			 */
			z[3] -= ਸਿਖਰ;
			ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
		}

		ਜੇ (nscss_color_is_transparent(box->border[BOTTOM].c) == false &&
		    box->border[BOTTOM].style != CSS_BORDER_STYLE_DOUBLE) {
			/* make border overhang bottom corner fully,
			 * if bottom border is opaque
			 */
			z[5] += bottom;
			ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
		}

		res = html_redraw_border_plot(ਸੱਜਾ_2,
					      z,
					      ਖਾਨਾ,
					      box->border[ਸੱਜਾ_2].style,
					      ਸੱਜਾ,
					      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
					      clip,
					      ctx);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}

	/* Top */
	ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਖੱਬਾ == 0);
	ਵਰਗਫਲ_ਸਿਰਾ_2 = (ਸੱਜਾ == 0);
	ਜੇ (ਸਿਖਰ != 0 &&
	    nscss_color_is_transparent(box->border[ਸਿਖਰ_2].c) == false) {
		ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[ਸਿਖਰ_2].c);

		z[0] = p[2];	z[1] = p[3];
		z[2] = p[0];	z[3] = p[1];
		z[4] = p[6];	z[5] = p[1];
		z[6] = p[4];	z[7] = p[3];

		ਜੇ (ਪਹਿਲਾ &&
		    box->border[ਸਿਖਰ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ਸਿਖਰ_2].c == box->border[ਖੱਬਾ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[2] += ਖੱਬਾ;
			ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
		}

		ਜੇ (ਆਖਰੀ &&
		    box->border[ਸਿਖਰ_2].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[ਸਿਖਰ_2].c == box->border[ਸੱਜਾ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[4] -= ਸੱਜਾ;
			ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
		}

		res = html_redraw_border_plot(ਸਿਖਰ_2,
					      z,
					      ਖਾਨਾ,
					      box->border[ਸਿਖਰ_2].style,
					      ਸਿਖਰ,
					      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
					      clip,
					      ctx);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}

	/* Bottom */
	ਵਰਗਫਲ_ਸਿਰਾ_1 = (ਖੱਬਾ == 0);
	ਵਰਗਫਲ_ਸਿਰਾ_2 = (ਸੱਜਾ == 0);
	ਜੇ (bottom != 0 &&
	    nscss_color_is_transparent(box->border[BOTTOM].c) == false) {
		ਖਾਨਾ = nscss_color_ਮੰਜ਼ਿਲ_ns(box->border[BOTTOM].c);

		z[0] = p[4];	z[1] = p[5];
		z[2] = p[6];	z[3] = p[7];
		z[4] = p[0];	z[5] = p[7];
		z[6] = p[2];	z[7] = p[5];

		ਜੇ (ਪਹਿਲਾ &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ਖੱਬਾ_2].c) {
			/* don't bother overlapping left corner if
			 * it's the same colour anyway
			 */
			z[4] += ਖੱਬਾ;
			ਵਰਗਫਲ_ਸਿਰਾ_1 = true;
		}

		ਜੇ (ਆਖਰੀ &&
		    box->border[BOTTOM].style == CSS_BORDER_STYLE_SOLID &&
		    box->border[BOTTOM].c == box->border[ਸੱਜਾ_2].c) {
			/* don't bother overlapping right corner if
			 * it's the same colour anyway
			 */
			z[2] -= ਸੱਜਾ;
			ਵਰਗਫਲ_ਸਿਰਾ_2 = true;
		}

		res = html_redraw_border_plot(BOTTOM,
					      z,
					      ਖਾਨਾ,
					      box->border[BOTTOM].style,
					      bottom,
					      ਵਰਗਫਲ_ਸਿਰਾ_1 && ਵਰਗਫਲ_ਸਿਰਾ_2,
					      clip,
					      ctx);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
}
