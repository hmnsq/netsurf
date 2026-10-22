/*
 * Copyright 2004-2008 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004-2007 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004-2007 Richard Wilson <info@tinct.net>
 * Copyright 2005-2006 Adrian Lees <adrianl@users.sourceforge.net>
 * Copyright 2006 Rob Kendrick <rjek@netsurf-browser.org>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net>
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
 * Redrawing CONTENT_HTML implementation.
 */

#ضمن "utils/config.h"
#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <math.h>
#ضمن <dom/dom.h>

#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/layout.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/textsearch.h"
#ضمن "css/utils.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/print.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"
#ضمن "html/private.h"
#ضمن "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ساكن bool html_redraw_box_has_background(بنية box *box)
{
	إذا (box->background != عدم)
		أرجع true;

	إذا (box->style != عدم) {
		css_color رنگ;

		css_computed_background_color(box->style, &رنگ);

		إذا (nscss_color_is_transparent(رنگ) == false)
			أرجع true;
	}

	أرجع false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ساكن بنية box *html_redraw_لبھو_bg_box(بنية box *box)
{
	/* Thanks to backwards compatibility, CSS defines the following:
	 *
	 * + If the box is for the root element and it has a background,
	 *   use that (and then process the body box with no special case)
	 * + If the box is for the root element and it has no background,
	 *   then use the background (if any) from the body element as if
	 *   it were specified on the root. Then, when the box for the body
	 *   element is processed, ignore the background.
	 * + For any other box, just use its own styling.
	 */
	إذا (box->ماپا == عدم) {
		/* Root box */
		إذا (html_redraw_box_has_background(box))
			أرجع box;

		/* No background on root box: consider body box, if any */
		إذا (box->children != عدم) {
			إذا (html_redraw_box_has_background(box->children))
				أرجع box->children;
		}
	} وإلا إذا (box->ماپا != عدم && box->ماپا->ماپا == عدم) {
		/* Body box: only render background if root has its own */
		إذا (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->ماپا))
			أرجع box;
	} وإلا {
		/* Any other box */
		إذا (html_redraw_box_has_background(box))
			أرجع box;
	}

	أرجع عدم;
}

/**
 * Redraw a short text string, complete with highlighting
 * (for selection/search)
 *
 * \param utf8_text pointer to UTF-8 text string
 * \param utf8_len  length of string, in bytes
 * \param offset    byte offset within textual representation
 * \param space     width of space that follows string (0 = no space)
 * \param fstyle    text style to use (pass text size unscaled)
 * \param x         x ordinate at which to plot text
 * \param y         y ordinate at which to plot text
 * \param clip      pointer to current clip rectangle
 * \param height    height of text string
 * \param scale     current display scale (1.0 = 100%)
 * \param excluded  exclude this text string from the selection
 * \param c         Content being redrawn.
 * \param sel       Selection context
 * \param search    Search context
 * \param ctx	    current redraw context
 * \return true iff successful and redraw should proceed
 */

ساكن bool
متن_redraw(ثابت حرف *utf8_متن,
	    size_t utf8_len,
	    size_t وتھ,
	    صحيح space,
	    ثابت plot_font_style_t *fstyle,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح اچائی,
	    عائم scale,
	    bool excluded,
	    بنية content *c,
	    ثابت بنية selection *sel,
	    ثابت بنية redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.ناپ *= scale;

	/* is this box part of a selection? */
	إذا (!excluded && ctx->interactive == true) {
		بنانشانی len = utf8_len + (space ? 1 : 0);
		بنانشانی شروع_idx;
		بنانشانی سرا_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  وتھ,
					  وتھ + len,
					  &شروع_idx,
					  &سرا_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (c->textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     وتھ,
						     وتھ + len,
						     &شروع_idx,
						     &سرا_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بنانشانی endtxt_idx = سرا_idx;
			bool clip_بدلیا = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (سرا_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				أكد(سرا_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->چوڑائی(fstyle,
						  utf8_متن, شروع_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->چوڑائی(fstyle,
						  utf8_متن, endtxt_idx,
						  &endx);
			إذا (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			إذا (سرا_idx > utf8_len) {
					endx += space;
			}

			إذا (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			إذا ((شروع_idx > 0) &&
			    (ctx->plot->متن(ctx,
					     &plot_fstyle,
					     x,
					     y + (صحيح)(اچائی * 0.75 * scale),
					     utf8_متن,
					     شروع_idx) != NSERROR_OK))
				أرجع false;

			pstyle_fill_hback.fill_رنگ = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + اچائی * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			إذا (شروع_idx > 0) {
				صحيح px0 = max(x + startx, clip->x0);
				صحيح px1 = min(x + endx, clip->x1);

				إذا (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_بدلیا = true;
				} وإلا {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_منزل_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			إذا (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(اچائی * 0.75 * scale),
					     utf8_متن,
					     endtxt_idx) != NSERROR_OK)) {
				أرجع false;
			}

			/* draw any text succeeding highlighted portion */
			إذا (endtxt_idx < utf8_len) {
				صحيح px0 = max(x + endx, clip->x0);
				إذا (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_بدلیا = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(اچائی * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_بدلیا &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (اچائی * 0.75 * scale),
				      utf8_متن,
				      utf8_len);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
}


/**
 * Plot a checkbox.
 *
 * \param  x	     left coordinate
 * \param  y	     top coordinate
 * \param  width     dimensions of checkbox
 * \param  height    dimensions of checkbox
 * \param  selected  the checkbox is selected
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_checkbox(صحيح x, صحيح y, صحيح چوڑائی, صحيح اچائی,
		bool selected, ثابت بنية redraw_context *ctx)
{
	مزدوج z;
	nserror res;
	بنية rect rect;

	z = چوڑائی * 0.15;
	إذا (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + چوڑائی;
	rect.y1 = y + اچائی;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + اچائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across right */
	rect.x0 = x + چوڑائی;
	rect.x1 = x + چوڑائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + اچائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		إذا (چوڑائی < 12 || اچائی < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + چوڑائی - z;
			rect.y1 = y + اچائی - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + چوڑائی - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + اچائی - z;
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + اچائی - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (اچائی / 2);
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}
	}
	أرجع true;
}


/**
 * Plot a radio icon.
 *
 * \param  x	     left coordinate
 * \param  y	     top coordinate
 * \param  width     dimensions of radio icon
 * \param  height    dimensions of radio icon
 * \param  selected  the radio icon is selected
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 */
ساكن bool html_redraw_radio(صحيح x, صحيح y, صحيح چوڑائی, صحيح اچائی,
		bool selected, ثابت بنية redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + چوڑائی * 0.5,
			      y + اچائی * 0.5,
			      چوڑائی * 0.5 - 1);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + چوڑائی * 0.5,
			     y + اچائی * 0.5,
			     چوڑائی * 0.5 - 1,
			     45,
			     225);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + چوڑائی * 0.5,
			     y + اچائی * 0.5,
			     چوڑائی * 0.5 - 1,
			     225,
			     45);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + چوڑائی * 0.5,
				      y + اچائی * 0.5,
				      چوڑائی * 0.3 - 1);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	أرجع true;
}


/**
 * Plot a file upload input.
 *
 * \param  x	     left coordinate
 * \param  y	     top coordinate
 * \param  width     dimensions of input
 * \param  height    dimensions of input
 * \param  box	     box of input
 * \param  scale     scale for redraw
 * \param  background_colour  current background colour
 * \param  unit_len_ctx   Length conversion context
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_file(صحيح x, صحيح y, صحيح چوڑائی, صحيح اچائی,
		بنية box *box, عائم scale, رنگ background_رنگ,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	صحيح متن_چوڑائی;
	ثابت حرف *متن;
	size_t لمائی;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_رنگ;

	إذا (box->gadget->قدر) {
		متن = box->gadget->قدر;
	} وإلا {
		متن = messages_get("Form_Drop");
	}
	لمائی = لڑی_لمبائی(متن);

	res = guit->layout->چوڑائی(&fstyle, متن, لمائی, &متن_چوڑائی);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}
	متن_چوڑائی *= scale;
	إذا (چوڑائی < متن_چوڑائی + 8) {
		x = x + چوڑائی - متن_چوڑائی - 4;
	} وإلا {
		x = x + 4;
	}

	res = ctx->plot->متن(ctx, &fstyle, x, y + اچائی * 0.75, متن, لمائی);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}
	أرجع true;
}


/**
 * Plot background images.
 *
 * The reason for the presence of \a background is the backwards compatibility
 * mess that is backgrounds on &lt;body&gt;. The background will be drawn relative
 * to \a box, using the background information contained within \a background.
 *
 * \param  x	  coordinate of box
 * \param  y	  coordinate of box
 * \param  box	  box to draw background image of
 * \param  scale  scale for redraw
 * \param  clip   current clip rectangle
 * \param  background_colour  current background colour
 * \param  background  box containing background details (usually \a box)
 * \param  unit_len_ctx  Length conversion context
 * \param  ctx      current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_background(صحيح x, صحيح y, بنية box *box, عائم scale,
		ثابت بنية rect *clip, رنگ *background_رنگ,
		بنية box *background,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_رنگ = true;
	bool plot_content;
	bool clip_منزل_children = false;
	بنية box *clip_box = box;
	صحيح ox = x, oy = y;
	صحيح چوڑائی, اچائی;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	بنية box *ماپا;
	بنية rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = *background_رنگ,
	};
	nserror res;

	إذا (ctx->background_images == false)
		أرجع true;

	plot_content = (background->background != عدم);

	إذا (plot_content) {
		إذا (!box->ماپا) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[بایاں_2] * scale;
			y -= box->margin[چوٹی_2] * scale;
			چوڑائی = box->margin[بایاں_2] + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2] +
					box->margin[دایاں_2];
			اچائی = box->margin[چوٹی_2] + box->padding[چوٹی_2] +
					box->اچائی + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} وإلا {
			چوڑائی = box->padding[بایاں_2] + box->چوڑائی +
					box->padding[دایاں_2];
			اچائی = box->padding[چوٹی_2] + box->اچائی +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		بدل (css_computed_background_repeat(background->style)) {
		حالة CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_رنگ = !content_get_opaque(background->background);
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
			اكسر;

		افتراضي:
			اكسر;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		إذا (hunit == CSS_UNIT_PCT) {
			x += (چوڑائی -
				content_get_چوڑائی(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (اچائی -
				content_get_اچائی(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} وإلا {
			y += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	إذا (box->type == BOX_جدول_صف) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		لكل (ماپا = box->ماپا;
			((ماپا) && (ماپا->type != BOX_جدول));
				ماپا = ماپا->ماپا);
		أكد(ماپا && (ماپا->style));

		css_computed_border_spacing(ماپا->style, &h, &hu, &v, &vu);

		clip_منزل_children = (h > 0) || (v > 0);

		إذا (clip_منزل_children)
			clip_box = box->children;
	}

	لكل (; clip_box; clip_box = clip_box->اگلا) {
		/* clip to child boxes if needed */
		إذا (clip_منزل_children) {
			أكد(clip_box->type == BOX_جدول_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[بایاں_2] +
					clip_box->چوڑائی +
					clip_box->padding[دایاں_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[چوٹی_2] +
					clip_box->اچائی +
					clip_box->padding[BOTTOM]) * scale;

			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (r.x1 > clip->x1) r.x1 = clip->x1;
			إذا (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			إذا ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != عدم) &&
			     content_get_opaque(clip_box->background)))
				استمر;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		إذا (nscss_color_is_transparent(bgcol) == false) {
			*background_رنگ = nscss_color_منزل_ns(bgcol);
			pstyle_fill_bg.fill_رنگ = *background_رنگ;
			إذا (plot_رنگ) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}
			}
		}
		/* and plot the image */
		إذا (plot_content) {
			چوڑائی = content_get_چوڑائی(background->background);
			اچائی = content_get_اچائی(background->background);

			/* ensure clip area only as large as required */
			إذا (!repeat_x) {
				إذا (r.x0 < x)
					r.x0 = x;
				إذا (r.x1 > x + چوڑائی * scale)
					r.x1 = x + چوڑائی * scale;
			}
			إذا (!repeat_y) {
				إذا (r.y0 < y)
					r.y0 = y;
				إذا (r.y1 > y + اچائی * scale)
					r.y1 = y + اچائی * scale;
			}
			/* valid clipping rectangles only */
			إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				بنية content_redraw_انکڑے bg_انکڑے;

				res = ctx->plot->clip(ctx, &r);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}

				bg_انکڑے.x = x;
				bg_انکڑے.y = y;
				bg_انکڑے.چوڑائی = ceilf(چوڑائی * scale);
				bg_انکڑے.اچائی = ceilf(اچائی * scale);
				bg_انکڑے.background_رنگ = *background_رنگ;
				bg_انکڑے.scale = scale;
				bg_انکڑے.repeat_x = repeat_x;
				bg_انکڑے.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_انکڑے, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		إذا (!clip_منزل_children)
			أرجع true;
	}
	أرجع true;
}


/**
 * Plot an inline's background and/or background image.
 *
 * \param  x	  coordinate of box
 * \param  y	  coordinate of box
 * \param  box	  BOX_INLINE which created the background
 * \param  scale  scale for redraw
 * \param  clip	  coordinates of clip rectangle
 * \param  b	  coordinates of border edge rectangle
 * \param  first  true if this is the first rectangle associated with the inline
 * \param  last   true if this is the last rectangle associated with the inline
 * \param  background_colour  updated to current background colour if plotted
 * \param  unit_len_ctx  Length conversion context
 * \param  ctx      current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_inline_background(صحيح x, صحيح y, بنية box *box,
		عائم scale, ثابت بنية rect *clip, بنية rect b,
		bool پہلا, bool آخری, رنگ *background_رنگ,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	بنية rect r = *clip;
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_رنگ = true;
	bool plot_content;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = *background_رنگ,
	};
	nserror res;

	plot_content = (box->background != عدم);

	إذا (html_redraw_printing && nsoption_bool(remove_backgrounds))
		أرجع true;

	إذا (plot_content) {
		/* handle background-repeat */
		بدل (css_computed_background_repeat(box->style)) {
		حالة CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_رنگ = !content_get_opaque(box->background);
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
			اكسر;

		افتراضي:
			اكسر;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		إذا (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_چوڑائی(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			إذا (!repeat_x && ((hpos < 2 && !پہلا) ||
					(hpos > 98 && !آخری))){
				plot_content = false;
			}
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_اچائی(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} وإلا {
			y += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	إذا (nscss_color_is_transparent(bgcol) == false) {
		*background_رنگ = nscss_color_منزل_ns(bgcol);
		pstyle_fill_bg.fill_رنگ = *background_رنگ;

		إذا (plot_رنگ) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}
	}
	/* and plot the image */
	إذا (plot_content) {
		صحيح چوڑائی = content_get_چوڑائی(box->background);
		صحيح اچائی = content_get_اچائی(box->background);

		إذا (!repeat_x) {
			إذا (r.x0 < x)
				r.x0 = x;
			إذا (r.x1 > x + چوڑائی * scale)
				r.x1 = x + چوڑائی * scale;
		}
		إذا (!repeat_y) {
			إذا (r.y0 < y)
				r.y0 = y;
			إذا (r.y1 > y + اچائی * scale)
				r.y1 = y + اچائی * scale;
		}
		/* valid clipping rectangles only */
		إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			بنية content_redraw_انکڑے bg_انکڑے;

			res = ctx->plot->clip(ctx, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			bg_انکڑے.x = x;
			bg_انکڑے.y = y;
			bg_انکڑے.چوڑائی = ceilf(چوڑائی * scale);
			bg_انکڑے.اچائی = ceilf(اچائی * scale);
			bg_انکڑے.background_رنگ = *background_رنگ;
			bg_انکڑے.scale = scale;
			bg_انکڑے.repeat_x = repeat_x;
			bg_انکڑے.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_انکڑے, &r, ctx);
		}
	}

	أرجع true;
}


/**
 * Plot text decoration for an inline box.
 *
 * \param  box     box to plot decorations for, of type BOX_INLINE
 * \param  x       x coordinate of parent of box
 * \param  y       y coordinate of parent of box
 * \param  scale   scale for redraw
 * \param  colour  colour for decorations
 * \param  ratio   position of line as a ratio of line height
 * \param  ctx	   current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool
html_redraw_متن_decoration_inline(بنية box *box,
				   صحيح x, صحيح y,
				   عائم scale,
				   رنگ رنگ,
				   عائم تناسب,
				   ثابت بنية redraw_context *ctx)
{
	بنية box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_رنگ = رنگ,
	};
	nserror res;
	بنية rect rect;

	لكل (c = box->اگلا;
	     c && c != box->inline_سرا;
	     c = c->اگلا) {
		إذا (c->type != BOX_متن) {
			استمر;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->اچائی * تناسب) * scale;
		rect.x1 = (x + c->x + c->چوڑائی) * scale;
		rect.y1 = (y + c->y + c->اچائی * تناسب) * scale;
		res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
}


/**
 * Plot text decoration for an non-inline box.
 *
 * \param  box     box to plot decorations for, of type other than BOX_INLINE
 * \param  x       x coordinate of box
 * \param  y       y coordinate of box
 * \param  scale   scale for redraw
 * \param  colour  colour for decorations
 * \param  ratio   position of line as a ratio of line height
 * \param  ctx	   current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool
html_redraw_متن_decoration_بلاک(بنية box *box,
				  صحيح x, صحيح y,
				  عائم scale,
				  رنگ رنگ,
				  عائم تناسب,
				  ثابت بنية redraw_context *ctx)
{
	بنية box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_رنگ = رنگ,
	};
	nserror res;
	بنية rect rect;

	/* draw through text descendants */
	لكل (c = box->children; c; c = c->اگلا) {
		إذا (c->type == BOX_متن) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->اچائی * تناسب) * scale;
			rect.x1 = (x + c->x + c->چوڑائی) * scale;
			rect.y1 = (y + c->y + c->اچائی * تناسب) * scale;
			res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا إذا ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_بلاک)) {
			إذا (!html_redraw_متن_decoration_بلاک(c,
					x + c->x, y + c->y,
					scale, رنگ, تناسب, ctx))
				أرجع false;
		}
	}
	أرجع true;
}


/**
 * Plot text decoration for a box.
 *
 * \param  box       box to plot decorations for
 * \param  x_parent  x coordinate of parent of box
 * \param  y_parent  y coordinate of parent of box
 * \param  scale     scale for redraw
 * \param  background_colour  current background colour
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_متن_decoration(بنية box *box,
		صحيح x_ماپا, صحيح y_ماپا, عائم scale,
		رنگ background_رنگ, ثابت بنية redraw_context *ctx)
{
	ساكن ثابت تعداد css_متن_decoration_e decoration[] = {
		CSS_متن_DECORATION_UNDERLINE, CSS_متن_DECORATION_OVERLINE,
		CSS_متن_DECORATION_سطر_THROUGH };
	ساكن ثابت عائم سطر_تناسب[] = { 0.9, 0.1, 0.5 };
	رنگ fgcol;
	بنانشانی صحيح i;
	css_color خانہ;

	css_computed_color(box->style, &خانہ);
	fgcol = nscss_color_منزل_ns(خانہ);

	/* antialias colour for under/overline */
	إذا (html_redraw_printing == false)
		fgcol = blend_رنگ(background_رنگ, fgcol);

	إذا (box->type == BOX_INLINE) {
		إذا (!box->inline_سرا)
			أرجع true;
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_متن_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_متن_decoration_inline(box,
						x_ماپا, y_ماپا, scale,
						fgcol, سطر_تناسب[i], ctx))
					أرجع false;
	} وإلا {
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_متن_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_متن_decoration_بلاک(box,
						x_ماپا + box->x,
						y_ماپا + box->y,
						scale,
						fgcol, سطر_تناسب[i], ctx))
					أرجع false;
	}

	أرجع true;
}


/**
 * Redraw the text content of a box, possibly partially highlighted
 * because the text has been selected, or matches a search operation.
 *
 * \param html The html content to redraw text within.
 * \param  box      box with text content
 * \param  x        x co-ord of box
 * \param  y        y co-ord of box
 * \param  clip     current clip rectangle
 * \param  scale    current scale setting (1.0 = 100%)
 * \param  current_background_color
 * \param  ctx	    current redraw context
 * \return true iff successful and redraw should proceed
 */

ساكن bool html_redraw_متن_box(ثابت html_content *html, بنية box *box,
		صحيح x, صحيح y, ثابت بنية rect *clip, عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx)
{
	bool excluded = (box->object != عدم);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	إذا (!متن_redraw(box->متن,
			 box->لمائی,
			 box->byte_وتھ,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->اچائی,
			 scale,
			 excluded,
			 (بنية content *)html,
			 html->sel,
			 ctx))
		أرجع false;

	أرجع true;
}

bool html_redraw_box(ثابت html_content *html, بنية box *box,
		صحيح x_ماپا, صحيح y_ماپا,
		ثابت بنية rect *clip, عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx);

/**
 * Draw the various children of a box.
 *
 * \param  html	     html content
 * \param  box	     box to draw children of
 * \param  x_parent  coordinate of parent box
 * \param  y_parent  coordinate of parent box
 * \param  clip      clip rectangle
 * \param  scale     scale for redraw
 * \param  current_background_color  background colour under this box
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 */

ساكن bool html_redraw_box_children(ثابت html_content *html, بنية box *box,
		صحيح x_ماپا, صحيح y_ماپا,
		ثابت بنية rect *clip, عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->اگلا) {

		إذا (c->type != BOX_FLOAT_بایاں && c->type != BOX_FLOAT_دایاں)
			إذا (!html_redraw_box(html, c,
					x_ماپا + box->x -
					scrollbar_get_وتھ(box->scroll_x),
					y_ماپا + box->y -
					scrollbar_get_وتھ(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				أرجع false;
	}
	لكل (c = box->float_children; c; c = c->اگلا_float)
		إذا (!html_redraw_box(html, c,
				x_ماپا + box->x -
				scrollbar_get_وتھ(box->scroll_x),
				y_ماپا + box->y -
				scrollbar_get_وتھ(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			أرجع false;

	أرجع true;
}

/**
 * Recursively draw a box.
 *
 * \param  html	     html content
 * \param  box	     box to draw
 * \param  x_parent  coordinate of parent box
 * \param  y_parent  coordinate of parent box
 * \param  clip      clip rectangle
 * \param  scale     scale for redraw
 * \param  current_background_color  background colour under this box
 * \param  ctx	     current redraw context
 * \return true if successful, false otherwise
 *
 * x, y, clip_[xy][01] are in target coordinates.
 */

bool html_redraw_box(ثابت html_content *html, بنية box *box,
		صحيح x_ماپا, صحيح y_ماپا,
		ثابت بنية rect *clip, ثابت عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx)
{
	ثابت بنية plotter_جدول *plot = ctx->plot;
	صحيح x, y;
	صحيح چوڑائی, اچائی;
	صحيح padding_بایاں, padding_چوٹی, padding_چوڑائی, padding_اچائی;
	صحيح border_بایاں, border_چوٹی, border_دایاں, border_bottom;
	بنية rect r;
	بنية rect rect;
	صحيح x_scrolled, y_scrolled;
	بنية box *bg_box = عدم;
	css_computed_clip_rect css_rect;
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_x = CSS_حد_توں_ودھ_VISIBLE;
	تعداد css_حد_توں_ودھ_e حد_توں_ودھ_y = CSS_حد_توں_ودھ_VISIBLE;
	dom_exception exc;
	dom_html_element_type ٹیگ_type;


	إذا (html_redraw_printing && (box->flags & PRINTED))
		أرجع true;

	إذا (box->style != عدم) {
		حد_توں_ودھ_x = css_computed_حد_توں_ودھ_x(box->style);
		حد_توں_ودھ_y = css_computed_حد_توں_ودھ_y(box->style);
	}

	/* avoid trivial FP maths */
	إذا (scale == 1.0) {
		x = x_ماپا + box->x;
		y = y_ماپا + box->y;
		چوڑائی = box->چوڑائی;
		اچائی = box->اچائی;
		padding_بایاں = box->padding[بایاں_2];
		padding_چوٹی = box->padding[چوٹی_2];
		padding_چوڑائی = padding_بایاں + box->چوڑائی + box->padding[دایاں_2];
		padding_اچائی = padding_چوٹی + box->اچائی +
				box->padding[BOTTOM];
		border_بایاں = box->border[بایاں_2].چوڑائی;
		border_چوٹی = box->border[چوٹی_2].چوڑائی;
		border_دایاں = box->border[دایاں_2].چوڑائی;
		border_bottom = box->border[BOTTOM].چوڑائی;
	} وإلا {
		x = (x_ماپا + box->x) * scale;
		y = (y_ماپا + box->y) * scale;
		چوڑائی = box->چوڑائی * scale;
		اچائی = box->اچائی * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_بایاں = box->padding[بایاں_2] ? box->padding[بایاں_2] * scale
				: 0;
		padding_چوٹی = box->padding[چوٹی_2] ? box->padding[چوٹی_2] * scale
				: 0;
		padding_چوڑائی = (box->padding[بایاں_2] + box->چوڑائی +
				box->padding[دایاں_2]) * scale;
		padding_اچائی = (box->padding[چوٹی_2] + box->اچائی +
				box->padding[BOTTOM]) * scale;
		border_بایاں = box->border[بایاں_2].چوڑائی * scale;
		border_چوٹی = box->border[چوٹی_2].چوڑائی * scale;
		border_دایاں = box->border[دایاں_2].چوڑائی * scale;
		border_bottom = box->border[BOTTOM].چوڑائی * scale;
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && حد_توں_ودھ_x != CSS_حد_توں_ودھ_VISIBLE &&
			box->ماپا != عدم) {
		/* box contents clipped to box size */
		r.x0 = x - border_بایاں;
		r.x1 = x + padding_چوڑائی + border_دایاں;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} وإلا {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		إذا (!box->ماپا) {
			/* root element */
			صحيح margin_بایاں, margin_دایاں;
			إذا (scale == 1.0) {
				margin_بایاں = box->margin[بایاں_2];
				margin_دایاں = box->margin[دایاں_2];
			} وإلا {
				margin_بایاں = box->margin[بایاں_2] * scale;
				margin_دایاں = box->margin[دایاں_2] * scale;
			}
			r.x0 = x - border_بایاں - margin_بایاں < r.x0 ?
					x - border_بایاں - margin_بایاں : r.x0;
			r.x1 = x + padding_چوڑائی + border_دایاں +
					margin_دایاں > r.x1 ?
					x + padding_چوڑائی + border_دایاں +
					margin_دایاں : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && حد_توں_ودھ_y != CSS_حد_توں_ودھ_VISIBLE &&
			box->ماپا != عدم) {
		/* box contents clipped to box size */
		r.y0 = y - border_چوٹی;
		r.y1 = y + padding_اچائی + border_bottom;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} وإلا {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		إذا (!box->ماپا) {
			/* root element */
			صحيح margin_چوٹی, margin_bottom;
			إذا (scale == 1.0) {
				margin_چوٹی = box->margin[چوٹی_2];
				margin_bottom = box->margin[BOTTOM];
			} وإلا {
				margin_چوٹی = box->margin[چوٹی_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_چوٹی - margin_چوٹی < r.y0 ?
					y - border_چوٹی - margin_چوٹی : r.y0;
			r.y1 = y + padding_اچائی + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_اچائی + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	إذا (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		أرجع true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	إذا (html_redraw_printing) {
		إذا (r.y1 > html_redraw_printing_border) {
			إذا (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_متن ||
					box->type == BOX_جدول_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				إذا (r.y0 < html_redraw_printing_چوٹی_cropped)
					html_redraw_printing_چوٹی_cropped = r.y0;
				أرجع true;
			}
		}
		وإلا box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	إذا (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		إذا ((ctx->plot->گروہ_شروع) &&
		    (ctx->plot->گروہ_شروع(ctx, "hidden box") != NSERROR_OK))
			أرجع false;
		إذا (!html_redraw_box_children(html, box, x_ماپا, y_ماپا,
				&r, scale, current_background_color, ctx))
			أرجع false;
		أرجع ((!ctx->plot->گروہ_سرا) || (ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
	}

	إذا ((ctx->plot->گروہ_شروع) &&
	    (ctx->plot->گروہ_شروع(ctx,"vis box") != NSERROR_OK)) {
		أرجع false;
	}

	إذا (box->style != عدم &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		إذا (css_rect.بایاں_auto == false)
			r.x0 = x - border_بایاں + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.بایاں, css_rect.lunit));

		إذا (css_rect.چوٹی_auto == false)
			r.y0 = y - border_چوٹی + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.چوٹی, css_rect.tunit));

		إذا (css_rect.دایاں_auto == false)
			r.x1 = x - border_بایاں + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.دایاں, css_rect.runit));

		إذا (css_rect.bottom_auto == false)
			r.y1 = y - border_چوٹی + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		إذا (r.x0 < clip->x0) r.x0 = clip->x0;
		إذا (r.y0 < clip->y0) r.y0 = clip->y0;
		إذا (clip->x1 < r.x1) r.x1 = clip->x1;
		إذا (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		إذا (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			أرجع ((!ctx->plot->گروہ_سرا) ||
				(ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
		/* clip to it */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;

	} وإلا إذا (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->type == BOX_جدول_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		إذا (r.x0 < clip->x0) r.x0 = clip->x0;
		إذا (r.y0 < clip->y0) r.y0 = clip->y0;
		إذا (clip->x1 < r.x1) r.x1 = clip->x1;
		إذا (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		إذا (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			أرجع ((!ctx->plot->گروہ_سرا) ||
				(ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
		/* clip to it */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
	} وإلا {
		/* clip box is fine, clip to it */
		r = *clip;
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_لبھو_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	إذا (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_متن &&
			bg_box->type != BOX_INLINE_سرا &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != عدم &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		بنية rect p;
		p.x0 = x - border_بایاں < r.x0 ? r.x0 : x - border_بایاں;
		p.y0 = y - border_چوٹی < r.y0 ? r.y0 : y - border_چوٹی;
		p.x1 = x + padding_چوڑائی + border_دایاں < r.x1 ?
				x + padding_چوڑائی + border_دایاں : r.x1;
		p.y1 = y + padding_اچائی + border_bottom < r.y1 ?
				y + padding_اچائی + border_bottom : r.y1;
		إذا (!box->ماپا) {
			/* Root element, special case:
			 * background covers margins too */
			صحيح m_بایاں, m_چوٹی, m_دایاں, m_bottom;
			إذا (scale == 1.0) {
				m_بایاں = box->margin[بایاں_2];
				m_چوٹی = box->margin[چوٹی_2];
				m_دایاں = box->margin[دایاں_2];
				m_bottom = box->margin[BOTTOM];
			} وإلا {
				m_بایاں = box->margin[بایاں_2] * scale;
				m_چوٹی = box->margin[چوٹی_2] * scale;
				m_دایاں = box->margin[دایاں_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_بایاں < r.x0 ? r.x0 : p.x0 - m_بایاں;
			p.y0 = p.y0 - m_چوٹی < r.y0 ? r.y0 : p.y0 - m_چوٹی;
			p.x1 = p.x1 + m_دایاں < r.x1 ? p.x1 + m_دایاں : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		إذا ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			إذا (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				أرجع false;
			/* restore previous graphics window */
			إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				أرجع false;
		}
	}

	/* borders for block level content and replaced inlines */
	إذا (box->style &&
	    box->type != BOX_متن &&
	    box->type != BOX_INLINE_سرا &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != عدم &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_چوٹی || border_دایاں || border_bottom || border_بایاں)) {
		إذا (!html_redraw_borders(box, x_ماپا, y_ماپا,
				padding_چوڑائی, padding_اچائی, &r,
				scale, ctx))
			أرجع false;
	}

	/* backgrounds and borders for non-replaced inlines */
	إذا (box->style && box->type == BOX_INLINE && box->inline_سرا &&
			(html_redraw_box_has_background(box) ||
			border_چوٹی || border_دایاں ||
			border_bottom || border_بایاں)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		بنية box *ib;
		بنية rect b; /* border edge rectangle */
		بنية rect p; /* clipped rect */
		bool پہلا = true;
		صحيح ib_x;
		صحيح ib_y = y;
		صحيح ib_p_چوڑائی;
		صحيح ib_b_بایاں, ib_b_دایاں;

		b.x0 = x - border_بایاں;
		b.x1 = x + padding_چوڑائی + border_دایاں;
		b.y0 = y - border_چوٹی;
		b.y1 = y + padding_اچائی + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		لكل (ib = box; ib; ib = ib->اگلا) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			إذا (ib->type == BOX_FLOAT_بایاں ||
					ib->type == BOX_FLOAT_دایاں)
				استمر;
			إذا (scale == 1.0) {
				ib_x = x_ماپا + ib->x;
				ib_y = y_ماپا + ib->y;
				ib_p_چوڑائی = ib->padding[بایاں_2] + ib->چوڑائی +
						ib->padding[دایاں_2];
				ib_b_بایاں = ib->border[بایاں_2].چوڑائی;
				ib_b_دایاں = ib->border[دایاں_2].چوڑائی;
			} وإلا {
				ib_x = (x_ماپا + ib->x) * scale;
				ib_y = (y_ماپا + ib->y) * scale;
				ib_p_چوڑائی = (ib->padding[بایاں_2] + ib->چوڑائی +
						ib->padding[دایاں_2]) * scale;
				ib_b_بایاں = ib->border[بایاں_2].چوڑائی * scale;
				ib_b_دایاں = ib->border[دایاں_2].چوڑائی * scale;
			}

			إذا ((ib->flags & NEW_سطر) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				إذا (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						پہلا, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					أرجع false;
				/* restore previous graphics window */
				إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					أرجع false;
				إذا (!html_redraw_inline_borders(box, b, &r,
						scale, پہلا, false, ctx))
					أرجع false;
				/* reset coords */
				b.x0 = ib_x - ib_b_بایاں;
				b.y0 = ib_y - border_چوٹی - padding_چوٹی;
				b.y1 = ib_y + padding_اچائی - padding_چوٹی +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				پہلا = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_چوڑائی + ib_b_دایاں;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			إذا (ib == box->inline_سرا)
				/* reached end of BOX_INLINE span */
				اكسر;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		إذا (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				پہلا, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			أرجع false;
		/* restore previous graphics window */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
		إذا (!html_redraw_inline_borders(box, b, &r, scale, پہلا, true,
				ctx))
			أرجع false;

	}

	/* Debug outlines */
	إذا (html_redraw_debug) {
		صحيح margin_بایاں, margin_دایاں;
		صحيح margin_چوٹی, margin_bottom;
		إذا (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_بایاں = box->margin[بایاں_2];
			margin_چوٹی = box->margin[چوٹی_2];
			margin_دایاں = box->margin[دایاں_2];
			margin_bottom = box->margin[BOTTOM];
		} وإلا {
			margin_بایاں = box->margin[بایاں_2] * scale;
			margin_چوٹی = box->margin[چوٹی_2] * scale;
			margin_دایاں = box->margin[دایاں_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_بایاں;
		rect.y0 = y + padding_چوٹی;
		rect.x1 = x + padding_بایاں + چوڑائی;
		rect.y1 = y + padding_چوٹی + اچائی;
		إذا (ctx->plot->rectangle(ctx, plot_style_content_کنارا, &rect) != NSERROR_OK)
			أرجع false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_چوڑائی;
		rect.y1 = y + padding_اچائی;
		إذا (ctx->plot->rectangle(ctx, plot_style_padding_کنارا, &rect) != NSERROR_OK)
			أرجع false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_بایاں - margin_بایاں;
		rect.y0 = y - border_چوٹی - margin_چوٹی;
		rect.x1 = x + padding_چوڑائی + border_دایاں + margin_دایاں;
		rect.y1 = y + padding_اچائی + border_bottom + margin_bottom;
		إذا (ctx->plot->rectangle(ctx, plot_style_margin_کنارا, &rect) != NSERROR_OK)
			أرجع false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	إذا (box->ماپا != عدم) {
		bool need_clip = false;
		إذا (box->object || box->flags & IFRAME ||
				(حد_توں_ودھ_x != CSS_حد_توں_ودھ_VISIBLE &&
				 حد_توں_ودھ_y != CSS_حد_توں_ودھ_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_چوڑائی;
			r.y1 = y + padding_اچائی;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.x1 <= r.x0 || r.y1 <= r.y0) {
				أرجع (!ctx->plot->گروہ_سرا ||
					(ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (حد_توں_ودھ_x != CSS_حد_توں_ودھ_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_چوڑائی;
			r.y1 = clip->y1;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (r.x1 <= r.x0) {
				أرجع (!ctx->plot->گروہ_سرا ||
					(ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (حد_توں_ودھ_y != CSS_حد_توں_ودھ_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_اچائی;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.y1 <= r.y0) {
				أرجع (!ctx->plot->گروہ_سرا ||
					(ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		إذا (need_clip &&
		    (box->type == BOX_بلاک ||
		     box->type == BOX_INLINE_بلاک ||
		     box->type == BOX_جدول_CELL || box->object)) {
			إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				أرجع false;
		}
	}

	/* text decoration */
	إذا ((box->type != BOX_متن) &&
	    box->style &&
	    css_computed_متن_decoration(box->style) !=	CSS_متن_DECORATION_NONE) {
		إذا (!html_redraw_متن_decoration(box, x_ماپا, y_ماپا,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->گرہ != عدم) {
		exc = dom_html_element_get_ٹیگ_type(box->گرہ, &ٹیگ_type);
		إذا (exc != DOM_NO_ERR) {
			ٹیگ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} وإلا {
		ٹیگ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	إذا (box->object && چوڑائی != 0 && اچائی != 0) {
		بنية content_redraw_انکڑے obj_انکڑے;

		x_scrolled = x - scrollbar_get_وتھ(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_وتھ(box->scroll_y) * scale;

		obj_انکڑے.x = x_scrolled + padding_بایاں;
		obj_انکڑے.y = y_scrolled + padding_چوٹی;
		obj_انکڑے.چوڑائی = چوڑائی;
		obj_انکڑے.اچائی = اچائی;
		obj_انکڑے.background_رنگ = current_background_color;
		obj_انکڑے.scale = scale;
		obj_انکڑے.repeat_x = false;
		obj_انکڑے.repeat_y = false;

		إذا (content_get_type(box->object) == CONTENT_HTML) {
			obj_انکڑے.x /= scale;
			obj_انکڑے.y /= scale;
		}

		إذا (!content_redraw(box->object, &obj_انکڑے, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ثابت حرف *obj = "\xef\xbf\xbc";
			صحيح obj_چوڑائی;
			صحيح obj_x = x + padding_بایاں;
			nserror res;

			rect.x0 = x + padding_بایاں;
			rect.y0 = y + padding_چوٹی;
			rect.x1 = x + padding_بایاں + چوڑائی - 1;
			rect.y1 = y + padding_چوٹی + اچائی - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			res = guit->layout->چوڑائی(plot_fstyle_broken_object,
						  obj,
						  حجم(obj) - 1,
						  &obj_چوڑائی);
			إذا (res != NSERROR_OK) {
				obj_x += 1;
			} وإلا {
				obj_x += چوڑائی / 2 - obj_چوڑائی / 2;
			}

			إذا (ctx->plot->متن(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_چوٹی + (صحيح)(اچائی * 0.75),
					    obj, حجم(obj) - 1) != NSERROR_OK)
				أرجع false;
		}
	} وإلا إذا (ٹیگ_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->گرہ != عدم &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		بنية bitmap *bitmap = عدم;
		exc = dom_گرہ_get_user_انکڑے(box->گرہ,
					     corestring_dom_ns_کلید_canvas_گرہ_انکڑے,
					     &bitmap);
		إذا (exc != DOM_NO_ERR) {
			bitmap = عدم;
		}
		إذا (bitmap != عدم &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_بایاں, y + padding_چوٹی,
				      چوڑائی, اچائی, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			أرجع false;
	} وإلا إذا (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_کھڑکی_redraw(box->iframe,
				x + padding_بایاں,
				y + padding_چوٹی, &r, ctx);

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		إذا (!html_redraw_checkbox(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اچائی, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_RADIO) {
		إذا (!html_redraw_radio(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اچائی, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_FILE) {
		إذا (!html_redraw_file(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اچائی, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			أرجع false;

	} وإلا إذا (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->انکڑے.متن.ta, x, y,
				current_background_color, scale, &r, ctx);

	} وإلا إذا (box->متن) {
		إذا (!html_redraw_متن_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			أرجع false;

	} وإلا {
		إذا (!html_redraw_box_children(html, box, x_ماپا, y_ماپا, &r,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->type == BOX_جدول_CELL || box->type == BOX_INLINE)
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;

	/* list marker */
	إذا (box->list_marker) {
		إذا (!html_redraw_box(html, box->list_marker,
				x_ماپا + box->x -
				scrollbar_get_وتھ(box->scroll_x),
				y_ماپا + box->y -
				scrollbar_get_وتھ(box->scroll_y),
				clip, scale, current_background_color, ctx))
			أرجع false;
	}

	/* scrollbars */
	إذا (((box->style && box->type != BOX_BR &&
	      box->type != BOX_جدول && box->type != BOX_INLINE &&
	      (box->gadget == عدم || box->gadget->type != GADGET_TEXTAREA) &&
	      (حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL ||
	       حد_توں_ودھ_x == CSS_حد_توں_ودھ_AUTO ||
	       حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL ||
	       حد_توں_ودھ_y == CSS_حد_توں_ودھ_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->ماپا != عدم) {
		nserror res;
		bool has_x_scroll = (حد_توں_ودھ_x == CSS_حد_توں_ودھ_SCROLL);
		bool has_y_scroll = (حد_توں_ودھ_y == CSS_حد_توں_ودھ_SCROLL);

		has_x_scroll |= (حد_توں_ودھ_x == CSS_حد_توں_ودھ_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (حد_توں_ودھ_y == CSS_حد_توں_ودھ_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((بنية content *)html,
					    box, has_x_scroll, has_y_scroll);
		إذا (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			أرجع false;
		}

		إذا (box->scroll_x != عدم)
			scrollbar_redraw(box->scroll_x,
					x_ماپا + box->x,
					y_ماپا + box->y + box->padding[چوٹی_2] +
					box->اچائی + box->padding[BOTTOM] -
					SCROLLBAR_چوڑائی, clip, scale, ctx);
		إذا (box->scroll_y != عدم)
			scrollbar_redraw(box->scroll_y,
					x_ماپا + box->x + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2] -
					SCROLLBAR_چوڑائی,
					y_ماپا + box->y, clip, scale, ctx);
	}

	إذا (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
	    box->type == BOX_جدول_CELL || box->type == BOX_INLINE) {
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;
	}

	أرجع ((!plot->گروہ_سرا) || (ctx->plot->گروہ_سرا(ctx) == NSERROR_OK));
}

/**
 * Draw a CONTENT_HTML using the current set of plotters (plot).
 *
 * \param  c	 content of type CONTENT_HTML
 * \param  data	 redraw data for this content redraw
 * \param  clip	 current clip region
 * \param  ctx	 current redraw context
 * \return true if successful, false otherwise
 *
 * x, y, clip_[xy][01] are in target coordinates.
 */

bool html_redraw(بنية content *c, بنية content_redraw_انکڑے *انکڑے,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	بنية box *box;
	bool نتیجہ = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = انکڑے->background_رنگ,
	};

	box = html->layout;
	أكد(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	إذا (ctx->interactive && html->visible_select_menu != عدم) {
		بنية form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				انکڑے->scale, clip);
	}

	إذا (!select_only) {
		/* clear to background colour */
		نتیجہ = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		إذا (html->background_رنگ != NS_TRANSPARENT)
			pstyle_fill_bg.fill_رنگ = html->background_رنگ;

		نتیجہ &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		نتیجہ &= html_redraw_box(html, box, انکڑے->x, انکڑے->y, clip,
				انکڑے->scale, pstyle_fill_bg.fill_رنگ, ctx);
	}

	إذا (select) {
		صحيح menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[بایاں_2].چوڑائی;
		menu_y += box->اچائی + box->border[BOTTOM].چوڑائی +
				box->padding[BOTTOM] + box->padding[چوٹی_2];
		نتیجہ &= form_redraw_select_menu(html->visible_select_menu,
				انکڑے->x + menu_x, انکڑے->y + menu_y,
				انکڑے->scale, clip, ctx);
	}

	أرجع نتیجہ;

}
