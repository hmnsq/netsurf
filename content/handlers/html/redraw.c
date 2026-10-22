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

#شامل "utils/config.h"
#شامل <assert.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <math.h>
#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/layout.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/textsearch.h"
#شامل "css/utils.h"
#شامل "desktop/selection.h"
#شامل "desktop/print.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"
#شامل "desktop/gui_internal.h"

#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/box_manipulate.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"
#شامل "html/private.h"
#شامل "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ساکن bool html_redraw_box_has_background(ساخت box *box)
{
	اگر (box->background != تہی)
		واپس true;

	اگر (box->style != تہی) {
		css_color رنگ;

		css_computed_background_color(box->style, &رنگ);

		اگر (nscss_color_is_transparent(رنگ) == false)
			واپس true;
	}

	واپس false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ساکن ساخت box *html_redraw_ڈھونڈو_bg_box(ساخت box *box)
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
	اگر (box->والد == تہی) {
		/* Root box */
		اگر (html_redraw_box_has_background(box))
			واپس box;

		/* No background on root box: consider body box, if any */
		اگر (box->children != تہی) {
			اگر (html_redraw_box_has_background(box->children))
				واپس box->children;
		}
	} ورنہ اگر (box->والد != تہی && box->والد->والد == تہی) {
		/* Body box: only render background if root has its own */
		اگر (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->والد))
			واپس box;
	} ورنہ {
		/* Any other box */
		اگر (html_redraw_box_has_background(box))
			واپس box;
	}

	واپس تہی;
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

ساکن bool
متن_redraw(ثابت حرف *utf8_متن,
	    size_t utf8_len,
	    size_t انحراف,
	    صحیح space,
	    ثابت plot_font_style_t *fstyle,
	    صحیح x,
	    صحیح y,
	    ثابت ساخت rect *clip,
	    صحیح اونچائی,
	    اعشاری scale,
	    bool excluded,
	    ساخت content *c,
	    ثابت ساخت selection *sel,
	    ثابت ساخت redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.سائز *= scale;

	/* is this box part of a selection? */
	اگر (!excluded && ctx->interactive == true) {
		غیرعلامتی len = utf8_len + (space ? 1 : 0);
		غیرعلامتی آغاز_idx;
		غیرعلامتی انتہا_idx;

		/* first try the browser window's current selection */
		اگر (selection_highlighted(sel,
					  انحراف,
					  انحراف + len,
					  &آغاز_idx,
					  &انتہا_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		اگر (!highlighted &&
		    (c->textsearch.context != تہی) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     انحراف,
						     انحراف + len,
						     &آغاز_idx,
						     &انتہا_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		اگر (highlighted) {
			ساخت rect r;
			غیرعلامتی endtxt_idx = انتہا_idx;
			bool clip_بدلا = false;
			bool متن_visible = true;
			صحیح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			اگر (انتہا_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				أكد(انتہا_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->چوڑائی(fstyle,
						  utf8_متن, آغاز_idx,
						  &startx);
			اگر (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->چوڑائی(fstyle,
						  utf8_متن, endtxt_idx,
						  &endx);
			اگر (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			اگر (انتہا_idx > utf8_len) {
					endx += space;
			}

			اگر (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			اگر ((آغاز_idx > 0) &&
			    (ctx->plot->متن(ctx,
					     &plot_fstyle,
					     x,
					     y + (صحیح)(اونچائی * 0.75 * scale),
					     utf8_متن,
					     آغاز_idx) != NSERROR_OK))
				واپس false;

			pstyle_fill_hback.fill_رنگ = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + اونچائی * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			اگر (res != NSERROR_OK) {
				واپس false;
			}

			اگر (آغاز_idx > 0) {
				صحیح px0 = max(x + startx, clip->x0);
				صحیح px1 = min(x + endx, clip->x1);

				اگر (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						واپس false;
					}

					clip_بدلا = true;
				} ورنہ {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_مقصد_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			اگر (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحیح)(اونچائی * 0.75 * scale),
					     utf8_متن,
					     endtxt_idx) != NSERROR_OK)) {
				واپس false;
			}

			/* draw any text succeeding highlighted portion */
			اگر (endtxt_idx < utf8_len) {
				صحیح px0 = max(x + endx, clip->x0);
				اگر (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						واپس false;
					}

					clip_بدلا = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحیح)(اونچائی * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					اگر (res != NSERROR_OK) {
						واپس false;
					}
				}
			}

			اگر (clip_بدلا &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				واپس false;
			}
		}
	}

	اگر (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحیح) (اونچائی * 0.75 * scale),
				      utf8_متن,
				      utf8_len);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}
	واپس true;
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

ساکن bool html_redraw_checkbox(صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی,
		bool selected, ثابت ساخت redraw_context *ctx)
{
	دوہرا z;
	nserror res;
	ساخت rect rect;

	z = چوڑائی * 0.15;
	اگر (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + چوڑائی;
	rect.y1 = y + اونچائی;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + اونچائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* light line across right */
	rect.x0 = x + چوڑائی;
	rect.x1 = x + چوڑائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + اونچائی;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	اگر (selected) {
		اگر (چوڑائی < 12 || اونچائی < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + چوڑائی - z;
			rect.y1 = y + اونچائی - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
		} ورنہ {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + چوڑائی - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + اونچائی - z;
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + اونچائی - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (اونچائی / 2);
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
		}
	}
	واپس true;
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
ساکن bool html_redraw_radio(صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی,
		bool selected, ثابت ساخت redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + چوڑائی * 0.5,
			      y + اونچائی * 0.5,
			      چوڑائی * 0.5 - 1);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + چوڑائی * 0.5,
			     y + اونچائی * 0.5,
			     چوڑائی * 0.5 - 1,
			     45,
			     225);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + چوڑائی * 0.5,
			     y + اونچائی * 0.5,
			     چوڑائی * 0.5 - 1,
			     225,
			     45);
	اگر (res != NSERROR_OK) {
		واپس false;
	}

	اگر (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + چوڑائی * 0.5,
				      y + اونچائی * 0.5,
				      چوڑائی * 0.3 - 1);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}

	واپس true;
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

ساکن bool html_redraw_file(صحیح x, صحیح y, صحیح چوڑائی, صحیح اونچائی,
		ساخت box *box, اعشاری scale, رنگ background_رنگ,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساخت redraw_context *ctx)
{
	صحیح متن_چوڑائی;
	ثابت حرف *متن;
	size_t لمبائی;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_رنگ;

	اگر (box->gadget->قدر) {
		متن = box->gadget->قدر;
	} ورنہ {
		متن = messages_get("Form_Drop");
	}
	لمبائی = لڑی_لمبائی(متن);

	res = guit->layout->چوڑائی(&fstyle, متن, لمبائی, &متن_چوڑائی);
	اگر (res != NSERROR_OK) {
		واپس false;
	}
	متن_چوڑائی *= scale;
	اگر (چوڑائی < متن_چوڑائی + 8) {
		x = x + چوڑائی - متن_چوڑائی - 4;
	} ورنہ {
		x = x + 4;
	}

	res = ctx->plot->متن(ctx, &fstyle, x, y + اونچائی * 0.75, متن, لمبائی);
	اگر (res != NSERROR_OK) {
		واپس false;
	}
	واپس true;
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

ساکن bool html_redraw_background(صحیح x, صحیح y, ساخت box *box, اعشاری scale,
		ثابت ساخت rect *clip, رنگ *background_رنگ,
		ساخت box *background,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساخت redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_رنگ = true;
	bool plot_content;
	bool clip_مقصد_children = false;
	ساخت box *clip_box = box;
	صحیح ox = x, oy = y;
	صحیح چوڑائی, اونچائی;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ساخت box *والد;
	ساخت rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = *background_رنگ,
	};
	nserror res;

	اگر (ctx->background_images == false)
		واپس true;

	plot_content = (background->background != تہی);

	اگر (plot_content) {
		اگر (!box->والد) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[بایاں_2] * scale;
			y -= box->margin[چوٹی_2] * scale;
			چوڑائی = box->margin[بایاں_2] + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2] +
					box->margin[دایاں_2];
			اونچائی = box->margin[چوٹی_2] + box->padding[چوٹی_2] +
					box->اونچائی + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} ورنہ {
			چوڑائی = box->padding[بایاں_2] + box->چوڑائی +
					box->padding[دایاں_2];
			اونچائی = box->padding[چوٹی_2] + box->اونچائی +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		انتخاب (css_computed_background_repeat(background->style)) {
		حالت CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_رنگ = !content_get_opaque(background->background);
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
			توڑو;

		طےشدہ:
			توڑو;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		اگر (hunit == CSS_UNIT_PCT) {
			x += (چوڑائی -
				content_get_چوڑائی(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} ورنہ {
			x += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		اگر (vunit == CSS_UNIT_PCT) {
			y += (اونچائی -
				content_get_اونچائی(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} ورنہ {
			y += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	اگر (box->type == BOX_جدول_صف) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		برائے (والد = box->والد;
			((والد) && (والد->type != BOX_جدول));
				والد = والد->والد);
		أكد(والد && (والد->style));

		css_computed_border_spacing(والد->style, &h, &hu, &v, &vu);

		clip_مقصد_children = (h > 0) || (v > 0);

		اگر (clip_مقصد_children)
			clip_box = box->children;
	}

	برائے (; clip_box; clip_box = clip_box->اگلا) {
		/* clip to child boxes if needed */
		اگر (clip_مقصد_children) {
			أكد(clip_box->type == BOX_جدول_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[بایاں_2] +
					clip_box->چوڑائی +
					clip_box->padding[دایاں_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[چوٹی_2] +
					clip_box->اونچائی +
					clip_box->padding[BOTTOM]) * scale;

			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (r.x1 > clip->x1) r.x1 = clip->x1;
			اگر (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			اگر ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != تہی) &&
			     content_get_opaque(clip_box->background)))
				جاری;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		اگر (nscss_color_is_transparent(bgcol) == false) {
			*background_رنگ = nscss_color_مقصد_ns(bgcol);
			pstyle_fill_bg.fill_رنگ = *background_رنگ;
			اگر (plot_رنگ) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				اگر (res != NSERROR_OK) {
					واپس false;
				}
			}
		}
		/* and plot the image */
		اگر (plot_content) {
			چوڑائی = content_get_چوڑائی(background->background);
			اونچائی = content_get_اونچائی(background->background);

			/* ensure clip area only as large as required */
			اگر (!repeat_x) {
				اگر (r.x0 < x)
					r.x0 = x;
				اگر (r.x1 > x + چوڑائی * scale)
					r.x1 = x + چوڑائی * scale;
			}
			اگر (!repeat_y) {
				اگر (r.y0 < y)
					r.y0 = y;
				اگر (r.y1 > y + اونچائی * scale)
					r.y1 = y + اونچائی * scale;
			}
			/* valid clipping rectangles only */
			اگر ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ساخت content_redraw_ڈیٹا bg_ڈیٹا;

				res = ctx->plot->clip(ctx, &r);
				اگر (res != NSERROR_OK) {
					واپس false;
				}

				bg_ڈیٹا.x = x;
				bg_ڈیٹا.y = y;
				bg_ڈیٹا.چوڑائی = ceilf(چوڑائی * scale);
				bg_ڈیٹا.اونچائی = ceilf(اونچائی * scale);
				bg_ڈیٹا.background_رنگ = *background_رنگ;
				bg_ڈیٹا.scale = scale;
				bg_ڈیٹا.repeat_x = repeat_x;
				bg_ڈیٹا.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_ڈیٹا, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		اگر (!clip_مقصد_children)
			واپس true;
	}
	واپس true;
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

ساکن bool html_redraw_inline_background(صحیح x, صحیح y, ساخت box *box,
		اعشاری scale, ثابت ساخت rect *clip, ساخت rect b,
		bool پہلا, bool آخری, رنگ *background_رنگ,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساخت redraw_context *ctx)
{
	ساخت rect r = *clip;
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

	plot_content = (box->background != تہی);

	اگر (html_redraw_printing && nsoption_bool(remove_backgrounds))
		واپس true;

	اگر (plot_content) {
		/* handle background-repeat */
		انتخاب (css_computed_background_repeat(box->style)) {
		حالت CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_رنگ = !content_get_opaque(box->background);
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			توڑو;

		حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
			توڑو;

		طےشدہ:
			توڑو;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		اگر (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_چوڑائی(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			اگر (!repeat_x && ((hpos < 2 && !پہلا) ||
					(hpos > 98 && !آخری))){
				plot_content = false;
			}
		} ورنہ {
			x += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		اگر (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_اونچائی(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} ورنہ {
			y += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	اگر (nscss_color_is_transparent(bgcol) == false) {
		*background_رنگ = nscss_color_مقصد_ns(bgcol);
		pstyle_fill_bg.fill_رنگ = *background_رنگ;

		اگر (plot_رنگ) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
		}
	}
	/* and plot the image */
	اگر (plot_content) {
		صحیح چوڑائی = content_get_چوڑائی(box->background);
		صحیح اونچائی = content_get_اونچائی(box->background);

		اگر (!repeat_x) {
			اگر (r.x0 < x)
				r.x0 = x;
			اگر (r.x1 > x + چوڑائی * scale)
				r.x1 = x + چوڑائی * scale;
		}
		اگر (!repeat_y) {
			اگر (r.y0 < y)
				r.y0 = y;
			اگر (r.y1 > y + اونچائی * scale)
				r.y1 = y + اونچائی * scale;
		}
		/* valid clipping rectangles only */
		اگر ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ساخت content_redraw_ڈیٹا bg_ڈیٹا;

			res = ctx->plot->clip(ctx, &r);
			اگر (res != NSERROR_OK) {
				واپس false;
			}

			bg_ڈیٹا.x = x;
			bg_ڈیٹا.y = y;
			bg_ڈیٹا.چوڑائی = ceilf(چوڑائی * scale);
			bg_ڈیٹا.اونچائی = ceilf(اونچائی * scale);
			bg_ڈیٹا.background_رنگ = *background_رنگ;
			bg_ڈیٹا.scale = scale;
			bg_ڈیٹا.repeat_x = repeat_x;
			bg_ڈیٹا.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_ڈیٹا, &r, ctx);
		}
	}

	واپس true;
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

ساکن bool
html_redraw_متن_decoration_inline(ساخت box *box,
				   صحیح x, صحیح y,
				   اعشاری scale,
				   رنگ رنگ,
				   اعشاری تناسب,
				   ثابت ساخت redraw_context *ctx)
{
	ساخت box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_رنگ = رنگ,
	};
	nserror res;
	ساخت rect rect;

	برائے (c = box->اگلا;
	     c && c != box->inline_انتہا;
	     c = c->اگلا) {
		اگر (c->type != BOX_متن) {
			جاری;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->اونچائی * تناسب) * scale;
		rect.x1 = (x + c->x + c->چوڑائی) * scale;
		rect.y1 = (y + c->y + c->اونچائی * تناسب) * scale;
		res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
		اگر (res != NSERROR_OK) {
			واپس false;
		}
	}
	واپس true;
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

ساکن bool
html_redraw_متن_decoration_بلاک(ساخت box *box,
				  صحیح x, صحیح y,
				  اعشاری scale,
				  رنگ رنگ,
				  اعشاری تناسب,
				  ثابت ساخت redraw_context *ctx)
{
	ساخت box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_رنگ = رنگ,
	};
	nserror res;
	ساخت rect rect;

	/* draw through text descendants */
	برائے (c = box->children; c; c = c->اگلا) {
		اگر (c->type == BOX_متن) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->اونچائی * تناسب) * scale;
			rect.x1 = (x + c->x + c->چوڑائی) * scale;
			rect.y1 = (y + c->y + c->اونچائی * تناسب) * scale;
			res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}
		} ورنہ اگر ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_بلاک)) {
			اگر (!html_redraw_متن_decoration_بلاک(c,
					x + c->x, y + c->y,
					scale, رنگ, تناسب, ctx))
				واپس false;
		}
	}
	واپس true;
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

ساکن bool html_redraw_متن_decoration(ساخت box *box,
		صحیح x_والد, صحیح y_والد, اعشاری scale,
		رنگ background_رنگ, ثابت ساخت redraw_context *ctx)
{
	ساکن ثابت شمار css_متن_decoration_e decoration[] = {
		CSS_متن_DECORATION_UNDERLINE, CSS_متن_DECORATION_OVERLINE,
		CSS_متن_DECORATION_سطر_THROUGH };
	ساکن ثابت اعشاری سطر_تناسب[] = { 0.9, 0.1, 0.5 };
	رنگ fgcol;
	غیرعلامتی صحیح i;
	css_color ستون;

	css_computed_color(box->style, &ستون);
	fgcol = nscss_color_مقصد_ns(ستون);

	/* antialias colour for under/overline */
	اگر (html_redraw_printing == false)
		fgcol = blend_رنگ(background_رنگ, fgcol);

	اگر (box->type == BOX_INLINE) {
		اگر (!box->inline_انتہا)
			واپس true;
		برائے (i = 0; i != NOF_ELEMENTS(decoration); i++)
			اگر (css_computed_متن_decoration(box->style) &
					decoration[i])
				اگر (!html_redraw_متن_decoration_inline(box,
						x_والد, y_والد, scale,
						fgcol, سطر_تناسب[i], ctx))
					واپس false;
	} ورنہ {
		برائے (i = 0; i != NOF_ELEMENTS(decoration); i++)
			اگر (css_computed_متن_decoration(box->style) &
					decoration[i])
				اگر (!html_redraw_متن_decoration_بلاک(box,
						x_والد + box->x,
						y_والد + box->y,
						scale,
						fgcol, سطر_تناسب[i], ctx))
					واپس false;
	}

	واپس true;
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

ساکن bool html_redraw_متن_box(ثابت html_content *html, ساخت box *box,
		صحیح x, صحیح y, ثابت ساخت rect *clip, اعشاری scale,
		رنگ current_background_color,
		ثابت ساخت redraw_context *ctx)
{
	bool excluded = (box->object != تہی);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	اگر (!متن_redraw(box->متن,
			 box->لمبائی,
			 box->byte_انحراف,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->اونچائی,
			 scale,
			 excluded,
			 (ساخت content *)html,
			 html->sel,
			 ctx))
		واپس false;

	واپس true;
}

bool html_redraw_box(ثابت html_content *html, ساخت box *box,
		صحیح x_والد, صحیح y_والد,
		ثابت ساخت rect *clip, اعشاری scale,
		رنگ current_background_color,
		ثابت ساخت redraw_context *ctx);

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

ساکن bool html_redraw_box_children(ثابت html_content *html, ساخت box *box,
		صحیح x_والد, صحیح y_والد,
		ثابت ساخت rect *clip, اعشاری scale,
		رنگ current_background_color,
		ثابت ساخت redraw_context *ctx)
{
	ساخت box *c;

	برائے (c = box->children; c; c = c->اگلا) {

		اگر (c->type != BOX_FLOAT_بایاں && c->type != BOX_FLOAT_دایاں)
			اگر (!html_redraw_box(html, c,
					x_والد + box->x -
					scrollbar_get_انحراف(box->scroll_x),
					y_والد + box->y -
					scrollbar_get_انحراف(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				واپس false;
	}
	برائے (c = box->float_children; c; c = c->اگلا_float)
		اگر (!html_redraw_box(html, c,
				x_والد + box->x -
				scrollbar_get_انحراف(box->scroll_x),
				y_والد + box->y -
				scrollbar_get_انحراف(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			واپس false;

	واپس true;
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

bool html_redraw_box(ثابت html_content *html, ساخت box *box,
		صحیح x_والد, صحیح y_والد,
		ثابت ساخت rect *clip, ثابت اعشاری scale,
		رنگ current_background_color,
		ثابت ساخت redraw_context *ctx)
{
	ثابت ساخت plotter_جدول *plot = ctx->plot;
	صحیح x, y;
	صحیح چوڑائی, اونچائی;
	صحیح padding_بایاں, padding_چوٹی, padding_چوڑائی, padding_اونچائی;
	صحیح border_بایاں, border_چوٹی, border_دایاں, border_bottom;
	ساخت rect r;
	ساخت rect rect;
	صحیح x_scrolled, y_scrolled;
	ساخت box *bg_box = تہی;
	css_computed_clip_rect css_rect;
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_x = CSS_حد_سے_تجاوز_VISIBLE;
	شمار css_حد_سے_تجاوز_e حد_سے_تجاوز_y = CSS_حد_سے_تجاوز_VISIBLE;
	dom_exception exc;
	dom_html_element_type لیبل_type;


	اگر (html_redraw_printing && (box->flags & PRINTED))
		واپس true;

	اگر (box->style != تہی) {
		حد_سے_تجاوز_x = css_computed_حد_سے_تجاوز_x(box->style);
		حد_سے_تجاوز_y = css_computed_حد_سے_تجاوز_y(box->style);
	}

	/* avoid trivial FP maths */
	اگر (scale == 1.0) {
		x = x_والد + box->x;
		y = y_والد + box->y;
		چوڑائی = box->چوڑائی;
		اونچائی = box->اونچائی;
		padding_بایاں = box->padding[بایاں_2];
		padding_چوٹی = box->padding[چوٹی_2];
		padding_چوڑائی = padding_بایاں + box->چوڑائی + box->padding[دایاں_2];
		padding_اونچائی = padding_چوٹی + box->اونچائی +
				box->padding[BOTTOM];
		border_بایاں = box->border[بایاں_2].چوڑائی;
		border_چوٹی = box->border[چوٹی_2].چوڑائی;
		border_دایاں = box->border[دایاں_2].چوڑائی;
		border_bottom = box->border[BOTTOM].چوڑائی;
	} ورنہ {
		x = (x_والد + box->x) * scale;
		y = (y_والد + box->y) * scale;
		چوڑائی = box->چوڑائی * scale;
		اونچائی = box->اونچائی * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_بایاں = box->padding[بایاں_2] ? box->padding[بایاں_2] * scale
				: 0;
		padding_چوٹی = box->padding[چوٹی_2] ? box->padding[چوٹی_2] * scale
				: 0;
		padding_چوڑائی = (box->padding[بایاں_2] + box->چوڑائی +
				box->padding[دایاں_2]) * scale;
		padding_اونچائی = (box->padding[چوٹی_2] + box->اونچائی +
				box->padding[BOTTOM]) * scale;
		border_بایاں = box->border[بایاں_2].چوڑائی * scale;
		border_چوٹی = box->border[چوٹی_2].چوڑائی * scale;
		border_دایاں = box->border[دایاں_2].چوڑائی * scale;
		border_bottom = box->border[BOTTOM].چوڑائی * scale;
	}

	/* calculate rectangle covering this box and descendants */
	اگر (box->style && حد_سے_تجاوز_x != CSS_حد_سے_تجاوز_VISIBLE &&
			box->والد != تہی) {
		/* box contents clipped to box size */
		r.x0 = x - border_بایاں;
		r.x1 = x + padding_چوڑائی + border_دایاں;
	} ورنہ {
		/* box contents can hang out of the box; use descendant box */
		اگر (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} ورنہ {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		اگر (!box->والد) {
			/* root element */
			صحیح margin_بایاں, margin_دایاں;
			اگر (scale == 1.0) {
				margin_بایاں = box->margin[بایاں_2];
				margin_دایاں = box->margin[دایاں_2];
			} ورنہ {
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
	اگر (box->style && حد_سے_تجاوز_y != CSS_حد_سے_تجاوز_VISIBLE &&
			box->والد != تہی) {
		/* box contents clipped to box size */
		r.y0 = y - border_چوٹی;
		r.y1 = y + padding_اونچائی + border_bottom;
	} ورنہ {
		/* box contents can hang out of the box; use descendant box */
		اگر (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} ورنہ {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		اگر (!box->والد) {
			/* root element */
			صحیح margin_چوٹی, margin_bottom;
			اگر (scale == 1.0) {
				margin_چوٹی = box->margin[چوٹی_2];
				margin_bottom = box->margin[BOTTOM];
			} ورنہ {
				margin_چوٹی = box->margin[چوٹی_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_چوٹی - margin_چوٹی < r.y0 ?
					y - border_چوٹی - margin_چوٹی : r.y0;
			r.y1 = y + padding_اونچائی + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_اونچائی + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	اگر (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		واپس true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	اگر (html_redraw_printing) {
		اگر (r.y1 > html_redraw_printing_border) {
			اگر (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_متن ||
					box->type == BOX_جدول_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				اگر (r.y0 < html_redraw_printing_چوٹی_cropped)
					html_redraw_printing_چوٹی_cropped = r.y0;
				واپس true;
			}
		}
		ورنہ box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	اگر (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		اگر ((ctx->plot->گروہ_آغاز) &&
		    (ctx->plot->گروہ_آغاز(ctx, "hidden box") != NSERROR_OK))
			واپس false;
		اگر (!html_redraw_box_children(html, box, x_والد, y_والد,
				&r, scale, current_background_color, ctx))
			واپس false;
		واپس ((!ctx->plot->گروہ_انتہا) || (ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
	}

	اگر ((ctx->plot->گروہ_آغاز) &&
	    (ctx->plot->گروہ_آغاز(ctx,"vis box") != NSERROR_OK)) {
		واپس false;
	}

	اگر (box->style != تہی &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		اگر (css_rect.بایاں_auto == false)
			r.x0 = x - border_بایاں + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.بایاں, css_rect.lunit));

		اگر (css_rect.چوٹی_auto == false)
			r.y0 = y - border_چوٹی + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.چوٹی, css_rect.tunit));

		اگر (css_rect.دایاں_auto == false)
			r.x1 = x - border_بایاں + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.دایاں, css_rect.runit));

		اگر (css_rect.bottom_auto == false)
			r.y1 = y - border_چوٹی + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		اگر (r.x0 < clip->x0) r.x0 = clip->x0;
		اگر (r.y0 < clip->y0) r.y0 = clip->y0;
		اگر (clip->x1 < r.x1) r.x1 = clip->x1;
		اگر (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		اگر (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			واپس ((!ctx->plot->گروہ_انتہا) ||
				(ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
		/* clip to it */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			واپس false;

	} ورنہ اگر (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->type == BOX_جدول_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		اگر (r.x0 < clip->x0) r.x0 = clip->x0;
		اگر (r.y0 < clip->y0) r.y0 = clip->y0;
		اگر (clip->x1 < r.x1) r.x1 = clip->x1;
		اگر (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		اگر (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			واپس ((!ctx->plot->گروہ_انتہا) ||
				(ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
		/* clip to it */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			واپس false;
	} ورنہ {
		/* clip box is fine, clip to it */
		r = *clip;
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			واپس false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_ڈھونڈو_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	اگر (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_متن &&
			bg_box->type != BOX_INLINE_انتہا &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != تہی &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ساخت rect p;
		p.x0 = x - border_بایاں < r.x0 ? r.x0 : x - border_بایاں;
		p.y0 = y - border_چوٹی < r.y0 ? r.y0 : y - border_چوٹی;
		p.x1 = x + padding_چوڑائی + border_دایاں < r.x1 ?
				x + padding_چوڑائی + border_دایاں : r.x1;
		p.y1 = y + padding_اونچائی + border_bottom < r.y1 ?
				y + padding_اونچائی + border_bottom : r.y1;
		اگر (!box->والد) {
			/* Root element, special case:
			 * background covers margins too */
			صحیح m_بایاں, m_چوٹی, m_دایاں, m_bottom;
			اگر (scale == 1.0) {
				m_بایاں = box->margin[بایاں_2];
				m_چوٹی = box->margin[چوٹی_2];
				m_دایاں = box->margin[دایاں_2];
				m_bottom = box->margin[BOTTOM];
			} ورنہ {
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
		اگر ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			اگر (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				واپس false;
			/* restore previous graphics window */
			اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				واپس false;
		}
	}

	/* borders for block level content and replaced inlines */
	اگر (box->style &&
	    box->type != BOX_متن &&
	    box->type != BOX_INLINE_انتہا &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != تہی &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_چوٹی || border_دایاں || border_bottom || border_بایاں)) {
		اگر (!html_redraw_borders(box, x_والد, y_والد,
				padding_چوڑائی, padding_اونچائی, &r,
				scale, ctx))
			واپس false;
	}

	/* backgrounds and borders for non-replaced inlines */
	اگر (box->style && box->type == BOX_INLINE && box->inline_انتہا &&
			(html_redraw_box_has_background(box) ||
			border_چوٹی || border_دایاں ||
			border_bottom || border_بایاں)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ساخت box *ib;
		ساخت rect b; /* border edge rectangle */
		ساخت rect p; /* clipped rect */
		bool پہلا = true;
		صحیح ib_x;
		صحیح ib_y = y;
		صحیح ib_p_چوڑائی;
		صحیح ib_b_بایاں, ib_b_دایاں;

		b.x0 = x - border_بایاں;
		b.x1 = x + padding_چوڑائی + border_دایاں;
		b.y0 = y - border_چوٹی;
		b.y1 = y + padding_اونچائی + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		برائے (ib = box; ib; ib = ib->اگلا) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			اگر (ib->type == BOX_FLOAT_بایاں ||
					ib->type == BOX_FLOAT_دایاں)
				جاری;
			اگر (scale == 1.0) {
				ib_x = x_والد + ib->x;
				ib_y = y_والد + ib->y;
				ib_p_چوڑائی = ib->padding[بایاں_2] + ib->چوڑائی +
						ib->padding[دایاں_2];
				ib_b_بایاں = ib->border[بایاں_2].چوڑائی;
				ib_b_دایاں = ib->border[دایاں_2].چوڑائی;
			} ورنہ {
				ib_x = (x_والد + ib->x) * scale;
				ib_y = (y_والد + ib->y) * scale;
				ib_p_چوڑائی = (ib->padding[بایاں_2] + ib->چوڑائی +
						ib->padding[دایاں_2]) * scale;
				ib_b_بایاں = ib->border[بایاں_2].چوڑائی * scale;
				ib_b_دایاں = ib->border[دایاں_2].چوڑائی * scale;
			}

			اگر ((ib->flags & NEW_سطر) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				اگر (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						پہلا, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					واپس false;
				/* restore previous graphics window */
				اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					واپس false;
				اگر (!html_redraw_inline_borders(box, b, &r,
						scale, پہلا, false, ctx))
					واپس false;
				/* reset coords */
				b.x0 = ib_x - ib_b_بایاں;
				b.y0 = ib_y - border_چوٹی - padding_چوٹی;
				b.y1 = ib_y + padding_اونچائی - padding_چوٹی +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				پہلا = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_چوڑائی + ib_b_دایاں;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			اگر (ib == box->inline_انتہا)
				/* reached end of BOX_INLINE span */
				توڑو;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		اگر (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				پہلا, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			واپس false;
		/* restore previous graphics window */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			واپس false;
		اگر (!html_redraw_inline_borders(box, b, &r, scale, پہلا, true,
				ctx))
			واپس false;

	}

	/* Debug outlines */
	اگر (html_redraw_debug) {
		صحیح margin_بایاں, margin_دایاں;
		صحیح margin_چوٹی, margin_bottom;
		اگر (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_بایاں = box->margin[بایاں_2];
			margin_چوٹی = box->margin[چوٹی_2];
			margin_دایاں = box->margin[دایاں_2];
			margin_bottom = box->margin[BOTTOM];
		} ورنہ {
			margin_بایاں = box->margin[بایاں_2] * scale;
			margin_چوٹی = box->margin[چوٹی_2] * scale;
			margin_دایاں = box->margin[دایاں_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_بایاں;
		rect.y0 = y + padding_چوٹی;
		rect.x1 = x + padding_بایاں + چوڑائی;
		rect.y1 = y + padding_چوٹی + اونچائی;
		اگر (ctx->plot->rectangle(ctx, plot_style_content_کنارہ, &rect) != NSERROR_OK)
			واپس false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_چوڑائی;
		rect.y1 = y + padding_اونچائی;
		اگر (ctx->plot->rectangle(ctx, plot_style_padding_کنارہ, &rect) != NSERROR_OK)
			واپس false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_بایاں - margin_بایاں;
		rect.y0 = y - border_چوٹی - margin_چوٹی;
		rect.x1 = x + padding_چوڑائی + border_دایاں + margin_دایاں;
		rect.y1 = y + padding_اونچائی + border_bottom + margin_bottom;
		اگر (ctx->plot->rectangle(ctx, plot_style_margin_کنارہ, &rect) != NSERROR_OK)
			واپس false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	اگر (box->والد != تہی) {
		bool need_clip = false;
		اگر (box->object || box->flags & IFRAME ||
				(حد_سے_تجاوز_x != CSS_حد_سے_تجاوز_VISIBLE &&
				 حد_سے_تجاوز_y != CSS_حد_سے_تجاوز_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_چوڑائی;
			r.y1 = y + padding_اونچائی;
			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (clip->x1 < r.x1) r.x1 = clip->x1;
			اگر (clip->y1 < r.y1) r.y1 = clip->y1;
			اگر (r.x1 <= r.x0 || r.y1 <= r.y0) {
				واپس (!ctx->plot->گروہ_انتہا ||
					(ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ورنہ اگر (حد_سے_تجاوز_x != CSS_حد_سے_تجاوز_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_چوڑائی;
			r.y1 = clip->y1;
			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (clip->x1 < r.x1) r.x1 = clip->x1;
			اگر (r.x1 <= r.x0) {
				واپس (!ctx->plot->گروہ_انتہا ||
					(ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ورنہ اگر (حد_سے_تجاوز_y != CSS_حد_سے_تجاوز_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_اونچائی;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (clip->y1 < r.y1) r.y1 = clip->y1;
			اگر (r.y1 <= r.y0) {
				واپس (!ctx->plot->گروہ_انتہا ||
					(ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		اگر (need_clip &&
		    (box->type == BOX_بلاک ||
		     box->type == BOX_INLINE_بلاک ||
		     box->type == BOX_جدول_CELL || box->object)) {
			اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				واپس false;
		}
	}

	/* text decoration */
	اگر ((box->type != BOX_متن) &&
	    box->style &&
	    css_computed_متن_decoration(box->style) !=	CSS_متن_DECORATION_NONE) {
		اگر (!html_redraw_متن_decoration(box, x_والد, y_والد,
				scale, current_background_color, ctx))
			واپس false;
	}

	اگر (box->گرہ != تہی) {
		exc = dom_html_element_get_لیبل_type(box->گرہ, &لیبل_type);
		اگر (exc != DOM_NO_ERR) {
			لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} ورنہ {
		لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	اگر (box->object && چوڑائی != 0 && اونچائی != 0) {
		ساخت content_redraw_ڈیٹا obj_ڈیٹا;

		x_scrolled = x - scrollbar_get_انحراف(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_انحراف(box->scroll_y) * scale;

		obj_ڈیٹا.x = x_scrolled + padding_بایاں;
		obj_ڈیٹا.y = y_scrolled + padding_چوٹی;
		obj_ڈیٹا.چوڑائی = چوڑائی;
		obj_ڈیٹا.اونچائی = اونچائی;
		obj_ڈیٹا.background_رنگ = current_background_color;
		obj_ڈیٹا.scale = scale;
		obj_ڈیٹا.repeat_x = false;
		obj_ڈیٹا.repeat_y = false;

		اگر (content_get_type(box->object) == CONTENT_HTML) {
			obj_ڈیٹا.x /= scale;
			obj_ڈیٹا.y /= scale;
		}

		اگر (!content_redraw(box->object, &obj_ڈیٹا, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ثابت حرف *obj = "\xef\xbf\xbc";
			صحیح obj_چوڑائی;
			صحیح obj_x = x + padding_بایاں;
			nserror res;

			rect.x0 = x + padding_بایاں;
			rect.y0 = y + padding_چوٹی;
			rect.x1 = x + padding_بایاں + چوڑائی - 1;
			rect.y1 = y + padding_چوٹی + اونچائی - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			اگر (res != NSERROR_OK) {
				واپس false;
			}

			res = guit->layout->چوڑائی(plot_fstyle_broken_object,
						  obj,
						  حجم(obj) - 1,
						  &obj_چوڑائی);
			اگر (res != NSERROR_OK) {
				obj_x += 1;
			} ورنہ {
				obj_x += چوڑائی / 2 - obj_چوڑائی / 2;
			}

			اگر (ctx->plot->متن(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_چوٹی + (صحیح)(اونچائی * 0.75),
					    obj, حجم(obj) - 1) != NSERROR_OK)
				واپس false;
		}
	} ورنہ اگر (لیبل_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->گرہ != تہی &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ساخت bitmap *bitmap = تہی;
		exc = dom_گرہ_get_user_ڈیٹا(box->گرہ,
					     corestring_dom_ns_کلید_canvas_گرہ_ڈیٹا,
					     &bitmap);
		اگر (exc != DOM_NO_ERR) {
			bitmap = تہی;
		}
		اگر (bitmap != تہی &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_بایاں, y + padding_چوٹی,
				      چوڑائی, اونچائی, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			واپس false;
	} ورنہ اگر (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_کھڑکی_redraw(box->iframe,
				x + padding_بایاں,
				y + padding_چوٹی, &r, ctx);

	} ورنہ اگر (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		اگر (!html_redraw_checkbox(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اونچائی, box->gadget->selected, ctx))
			واپس false;

	} ورنہ اگر (box->gadget && box->gadget->type == GADGET_RADIO) {
		اگر (!html_redraw_radio(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اونچائی, box->gadget->selected, ctx))
			واپس false;

	} ورنہ اگر (box->gadget && box->gadget->type == GADGET_FILE) {
		اگر (!html_redraw_file(x + padding_بایاں, y + padding_چوٹی,
				چوڑائی, اونچائی, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			واپس false;

	} ورنہ اگر (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->ڈیٹا.متن.ta, x, y,
				current_background_color, scale, &r, ctx);

	} ورنہ اگر (box->متن) {
		اگر (!html_redraw_متن_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			واپس false;

	} ورنہ {
		اگر (!html_redraw_box_children(html, box, x_والد, y_والد, &r,
				scale, current_background_color, ctx))
			واپس false;
	}

	اگر (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
			box->type == BOX_جدول_CELL || box->type == BOX_INLINE)
		اگر (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			واپس false;

	/* list marker */
	اگر (box->list_marker) {
		اگر (!html_redraw_box(html, box->list_marker,
				x_والد + box->x -
				scrollbar_get_انحراف(box->scroll_x),
				y_والد + box->y -
				scrollbar_get_انحراف(box->scroll_y),
				clip, scale, current_background_color, ctx))
			واپس false;
	}

	/* scrollbars */
	اگر (((box->style && box->type != BOX_BR &&
	      box->type != BOX_جدول && box->type != BOX_INLINE &&
	      (box->gadget == تہی || box->gadget->type != GADGET_TEXTAREA) &&
	      (حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL ||
	       حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_AUTO ||
	       حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL ||
	       حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->والد != تہی) {
		nserror res;
		bool has_x_scroll = (حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_SCROLL);
		bool has_y_scroll = (حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_SCROLL);

		has_x_scroll |= (حد_سے_تجاوز_x == CSS_حد_سے_تجاوز_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (حد_سے_تجاوز_y == CSS_حد_سے_تجاوز_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ساخت content *)html,
					    box, has_x_scroll, has_y_scroll);
		اگر (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			واپس false;
		}

		اگر (box->scroll_x != تہی)
			scrollbar_redraw(box->scroll_x,
					x_والد + box->x,
					y_والد + box->y + box->padding[چوٹی_2] +
					box->اونچائی + box->padding[BOTTOM] -
					SCROLLBAR_چوڑائی, clip, scale, ctx);
		اگر (box->scroll_y != تہی)
			scrollbar_redraw(box->scroll_y,
					x_والد + box->x + box->padding[بایاں_2] +
					box->چوڑائی + box->padding[دایاں_2] -
					SCROLLBAR_چوڑائی,
					y_والد + box->y, clip, scale, ctx);
	}

	اگر (box->type == BOX_بلاک || box->type == BOX_INLINE_بلاک ||
	    box->type == BOX_جدول_CELL || box->type == BOX_INLINE) {
		اگر (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			واپس false;
	}

	واپس ((!plot->گروہ_انتہا) || (ctx->plot->گروہ_انتہا(ctx) == NSERROR_OK));
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

bool html_redraw(ساخت content *c, ساخت content_redraw_ڈیٹا *ڈیٹا,
		ثابت ساخت rect *clip, ثابت ساخت redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ساخت box *box;
	bool نتیجہ = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = ڈیٹا->background_رنگ,
	};

	box = html->layout;
	أكد(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	اگر (ctx->interactive && html->visible_select_menu != تہی) {
		ساخت form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				ڈیٹا->scale, clip);
	}

	اگر (!select_only) {
		/* clear to background colour */
		نتیجہ = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		اگر (html->background_رنگ != NS_TRANSPARENT)
			pstyle_fill_bg.fill_رنگ = html->background_رنگ;

		نتیجہ &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		نتیجہ &= html_redraw_box(html, box, ڈیٹا->x, ڈیٹا->y, clip,
				ڈیٹا->scale, pstyle_fill_bg.fill_رنگ, ctx);
	}

	اگر (select) {
		صحیح menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[بایاں_2].چوڑائی;
		menu_y += box->اونچائی + box->border[BOTTOM].چوڑائی +
				box->padding[BOTTOM] + box->padding[چوٹی_2];
		نتیجہ &= form_redraw_select_menu(html->visible_select_menu,
				ڈیٹا->x + menu_x, ڈیٹا->y + menu_y,
				ڈیٹا->scale, clip, ctx);
	}

	واپس نتیجہ;

}
