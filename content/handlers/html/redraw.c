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
ساكن بنية box *html_redraw_پیدا_کن_bg_box(بنية box *box)
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
	إذا (box->والد == عدم) {
		/* Root box */
		إذا (html_redraw_box_has_background(box))
			أرجع box;

		/* No background on root box: consider body box, if any */
		إذا (box->children != عدم) {
			إذا (html_redraw_box_has_background(box->children))
				أرجع box->children;
		}
	} وإلا إذا (box->والد != عدم && box->والد->والد == عدم) {
		/* Body box: only render background if root has its own */
		إذا (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->والد))
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
	    size_t جابه_جایی,
	    صحيح space,
	    ثابت plot_font_style_t *fstyle,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح ارتفاع,
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
	plot_fstyle.اندازه *= scale;

	/* is this box part of a selection? */
	إذا (!excluded && ctx->interactive == true) {
		بیعلامت len = utf8_len + (space ? 1 : 0);
		بیعلامت شروع_idx;
		بیعلامت انتها_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  جابه_جایی,
					  جابه_جایی + len,
					  &شروع_idx,
					  &انتها_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (c->textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     جابه_جایی,
						     جابه_جایی + len,
						     &شروع_idx,
						     &انتها_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			بیعلامت endtxt_idx = انتها_idx;
			bool clip_تغییریافته = false;
			bool متن_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (انتها_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				أكد(انتها_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->عرض(fstyle,
						  utf8_متن, شروع_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->عرض(fstyle,
						  utf8_متن, endtxt_idx,
						  &endx);
			إذا (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			إذا (انتها_idx > utf8_len) {
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
					     y + (صحيح)(ارتفاع * 0.75 * scale),
					     utf8_متن,
					     شروع_idx) != NSERROR_OK))
				أرجع false;

			pstyle_fill_hback.fill_رنگ = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ارتفاع * scale;
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

					clip_تغییریافته = true;
				} وإلا {
					متن_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_رنگ;
			fstyle_hback.foreground = رنگ_مقصد_bw_furthest(
				pstyle_fill_hback.fill_رنگ);

			إذا (متن_visible &&
			    (ctx->plot->متن(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(ارتفاع * 0.75 * scale),
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

					clip_تغییریافته = true;

					res = ctx->plot->متن(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(ارتفاع * 0.75 * scale),
							      utf8_متن,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_تغییریافته &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->متن(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (ارتفاع * 0.75 * scale),
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

ساكن bool html_redraw_checkbox(صحيح x, صحيح y, صحيح عرض, صحيح ارتفاع,
		bool selected, ثابت بنية redraw_context *ctx)
{
	مزدوج z;
	nserror res;
	بنية rect rect;

	z = عرض * 0.15;
	إذا (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + عرض;
	rect.y1 = y + ارتفاع;
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
	rect.y1 = y + ارتفاع;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across right */
	rect.x0 = x + عرض;
	rect.x1 = x + عرض;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + ارتفاع;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		إذا (عرض < 12 || ارتفاع < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + عرض - z;
			rect.y1 = y + ارتفاع - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + عرض - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + ارتفاع - z;
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + ارتفاع - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (ارتفاع / 2);
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
ساكن bool html_redraw_radio(صحيح x, صحيح y, صحيح عرض, صحيح ارتفاع,
		bool selected, ثابت بنية redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + عرض * 0.5,
			      y + ارتفاع * 0.5,
			      عرض * 0.5 - 1);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + عرض * 0.5,
			     y + ارتفاع * 0.5,
			     عرض * 0.5 - 1,
			     45,
			     225);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + عرض * 0.5,
			     y + ارتفاع * 0.5,
			     عرض * 0.5 - 1,
			     225,
			     45);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + عرض * 0.5,
				      y + ارتفاع * 0.5,
				      عرض * 0.3 - 1);
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

ساكن bool html_redraw_file(صحيح x, صحيح y, صحيح عرض, صحيح ارتفاع,
		بنية box *box, عائم scale, رنگ background_رنگ,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	صحيح متن_عرض;
	ثابت حرف *متن;
	size_t درازی;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_رنگ;

	إذا (box->gadget->مقدار) {
		متن = box->gadget->مقدار;
	} وإلا {
		متن = messages_get("Form_Drop");
	}
	درازی = طول_رشته(متن);

	res = guit->layout->عرض(&fstyle, متن, درازی, &متن_عرض);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}
	متن_عرض *= scale;
	إذا (عرض < متن_عرض + 8) {
		x = x + عرض - متن_عرض - 4;
	} وإلا {
		x = x + 4;
	}

	res = ctx->plot->متن(ctx, &fstyle, x, y + ارتفاع * 0.75, متن, درازی);
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
	bool clip_مقصد_children = false;
	بنية box *clip_box = box;
	صحيح ox = x, oy = y;
	صحيح عرض, ارتفاع;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	بنية box *والد;
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
		إذا (!box->والد) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[چپ_2] * scale;
			y -= box->margin[بالاترین_2] * scale;
			عرض = box->margin[چپ_2] + box->padding[چپ_2] +
					box->عرض + box->padding[راست_2] +
					box->margin[راست_2];
			ارتفاع = box->margin[بالاترین_2] + box->padding[بالاترین_2] +
					box->ارتفاع + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} وإلا {
			عرض = box->padding[چپ_2] + box->عرض +
					box->padding[راست_2];
			ارتفاع = box->padding[بالاترین_2] + box->ارتفاع +
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
			x += (عرض -
				content_get_عرض(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (ارتفاع -
				content_get_ارتفاع(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} وإلا {
			y += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	إذا (box->type == BOX_جدول_ردیف) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		لكل (والد = box->والد;
			((والد) && (والد->type != BOX_جدول));
				والد = والد->والد);
		أكد(والد && (والد->style));

		css_computed_border_spacing(والد->style, &h, &hu, &v, &vu);

		clip_مقصد_children = (h > 0) || (v > 0);

		إذا (clip_مقصد_children)
			clip_box = box->children;
	}

	لكل (; clip_box; clip_box = clip_box->بعدی) {
		/* clip to child boxes if needed */
		إذا (clip_مقصد_children) {
			أكد(clip_box->type == BOX_جدول_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[چپ_2] +
					clip_box->عرض +
					clip_box->padding[راست_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[بالاترین_2] +
					clip_box->ارتفاع +
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
			*background_رنگ = nscss_color_مقصد_ns(bgcol);
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
			عرض = content_get_عرض(background->background);
			ارتفاع = content_get_ارتفاع(background->background);

			/* ensure clip area only as large as required */
			إذا (!repeat_x) {
				إذا (r.x0 < x)
					r.x0 = x;
				إذا (r.x1 > x + عرض * scale)
					r.x1 = x + عرض * scale;
			}
			إذا (!repeat_y) {
				إذا (r.y0 < y)
					r.y0 = y;
				إذا (r.y1 > y + ارتفاع * scale)
					r.y1 = y + ارتفاع * scale;
			}
			/* valid clipping rectangles only */
			إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				بنية content_redraw_داده bg_داده;

				res = ctx->plot->clip(ctx, &r);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}

				bg_داده.x = x;
				bg_داده.y = y;
				bg_داده.عرض = ceilf(عرض * scale);
				bg_داده.ارتفاع = ceilf(ارتفاع * scale);
				bg_داده.background_رنگ = *background_رنگ;
				bg_داده.scale = scale;
				bg_داده.repeat_x = repeat_x;
				bg_داده.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_داده, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		إذا (!clip_مقصد_children)
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
		bool اولی, bool آخرین, رنگ *background_رنگ,
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
					content_get_عرض(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			إذا (!repeat_x && ((hpos < 2 && !اولی) ||
					(hpos > 98 && !آخرین))){
				plot_content = false;
			}
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_ارتفاع(box->background) *
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
		*background_رنگ = nscss_color_مقصد_ns(bgcol);
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
		صحيح عرض = content_get_عرض(box->background);
		صحيح ارتفاع = content_get_ارتفاع(box->background);

		إذا (!repeat_x) {
			إذا (r.x0 < x)
				r.x0 = x;
			إذا (r.x1 > x + عرض * scale)
				r.x1 = x + عرض * scale;
		}
		إذا (!repeat_y) {
			إذا (r.y0 < y)
				r.y0 = y;
			إذا (r.y1 > y + ارتفاع * scale)
				r.y1 = y + ارتفاع * scale;
		}
		/* valid clipping rectangles only */
		إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			بنية content_redraw_داده bg_داده;

			res = ctx->plot->clip(ctx, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			bg_داده.x = x;
			bg_داده.y = y;
			bg_داده.عرض = ceilf(عرض * scale);
			bg_داده.ارتفاع = ceilf(ارتفاع * scale);
			bg_داده.background_رنگ = *background_رنگ;
			bg_داده.scale = scale;
			bg_داده.repeat_x = repeat_x;
			bg_داده.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_داده, &r, ctx);
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
				   عائم نسبت,
				   ثابت بنية redraw_context *ctx)
{
	بنية box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_رنگ = رنگ,
	};
	nserror res;
	بنية rect rect;

	لكل (c = box->بعدی;
	     c && c != box->inline_انتها;
	     c = c->بعدی) {
		إذا (c->type != BOX_متن) {
			استمر;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->ارتفاع * نسبت) * scale;
		rect.x1 = (x + c->x + c->عرض) * scale;
		rect.y1 = (y + c->y + c->ارتفاع * نسبت) * scale;
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
html_redraw_متن_decoration_بلوک(بنية box *box,
				  صحيح x, صحيح y,
				  عائم scale,
				  رنگ رنگ,
				  عائم نسبت,
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
	لكل (c = box->children; c; c = c->بعدی) {
		إذا (c->type == BOX_متن) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->ارتفاع * نسبت) * scale;
			rect.x1 = (x + c->x + c->عرض) * scale;
			rect.y1 = (y + c->y + c->ارتفاع * نسبت) * scale;
			res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا إذا ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_بلوک)) {
			إذا (!html_redraw_متن_decoration_بلوک(c,
					x + c->x, y + c->y,
					scale, رنگ, نسبت, ctx))
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
		صحيح x_والد, صحيح y_والد, عائم scale,
		رنگ background_رنگ, ثابت بنية redraw_context *ctx)
{
	ساكن ثابت تعداد css_متن_decoration_e decoration[] = {
		CSS_متن_DECORATION_UNDERLINE, CSS_متن_DECORATION_OVERLINE,
		CSS_متن_DECORATION_سطر_THROUGH };
	ساكن ثابت عائم سطر_نسبت[] = { 0.9, 0.1, 0.5 };
	رنگ fgcol;
	بیعلامت صحيح i;
	css_color ستون;

	css_computed_color(box->style, &ستون);
	fgcol = nscss_color_مقصد_ns(ستون);

	/* antialias colour for under/overline */
	إذا (html_redraw_printing == false)
		fgcol = blend_رنگ(background_رنگ, fgcol);

	إذا (box->type == BOX_INLINE) {
		إذا (!box->inline_انتها)
			أرجع true;
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_متن_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_متن_decoration_inline(box,
						x_والد, y_والد, scale,
						fgcol, سطر_نسبت[i], ctx))
					أرجع false;
	} وإلا {
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_متن_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_متن_decoration_بلوک(box,
						x_والد + box->x,
						y_والد + box->y,
						scale,
						fgcol, سطر_نسبت[i], ctx))
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
			 box->درازی,
			 box->byte_جابه_جایی,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->ارتفاع,
			 scale,
			 excluded,
			 (بنية content *)html,
			 html->sel,
			 ctx))
		أرجع false;

	أرجع true;
}

bool html_redraw_box(ثابت html_content *html, بنية box *box,
		صحيح x_والد, صحيح y_والد,
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
		صحيح x_والد, صحيح y_والد,
		ثابت بنية rect *clip, عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->بعدی) {

		إذا (c->type != BOX_FLOAT_چپ && c->type != BOX_FLOAT_راست)
			إذا (!html_redraw_box(html, c,
					x_والد + box->x -
					scrollbar_get_جابه_جایی(box->scroll_x),
					y_والد + box->y -
					scrollbar_get_جابه_جایی(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				أرجع false;
	}
	لكل (c = box->float_children; c; c = c->بعدی_float)
		إذا (!html_redraw_box(html, c,
				x_والد + box->x -
				scrollbar_get_جابه_جایی(box->scroll_x),
				y_والد + box->y -
				scrollbar_get_جابه_جایی(box->scroll_y),
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
		صحيح x_والد, صحيح y_والد,
		ثابت بنية rect *clip, ثابت عائم scale,
		رنگ current_background_color,
		ثابت بنية redraw_context *ctx)
{
	ثابت بنية plotter_جدول *plot = ctx->plot;
	صحيح x, y;
	صحيح عرض, ارتفاع;
	صحيح padding_چپ, padding_بالاترین, padding_عرض, padding_ارتفاع;
	صحيح border_چپ, border_بالاترین, border_راست, border_bottom;
	بنية rect r;
	بنية rect rect;
	صحيح x_scrolled, y_scrolled;
	بنية box *bg_box = عدم;
	css_computed_clip_rect css_rect;
	تعداد css_سرریز_e سرریز_x = CSS_سرریز_VISIBLE;
	تعداد css_سرریز_e سرریز_y = CSS_سرریز_VISIBLE;
	dom_exception exc;
	dom_html_element_type برچسب_type;


	إذا (html_redraw_printing && (box->flags & PRINTED))
		أرجع true;

	إذا (box->style != عدم) {
		سرریز_x = css_computed_سرریز_x(box->style);
		سرریز_y = css_computed_سرریز_y(box->style);
	}

	/* avoid trivial FP maths */
	إذا (scale == 1.0) {
		x = x_والد + box->x;
		y = y_والد + box->y;
		عرض = box->عرض;
		ارتفاع = box->ارتفاع;
		padding_چپ = box->padding[چپ_2];
		padding_بالاترین = box->padding[بالاترین_2];
		padding_عرض = padding_چپ + box->عرض + box->padding[راست_2];
		padding_ارتفاع = padding_بالاترین + box->ارتفاع +
				box->padding[BOTTOM];
		border_چپ = box->border[چپ_2].عرض;
		border_بالاترین = box->border[بالاترین_2].عرض;
		border_راست = box->border[راست_2].عرض;
		border_bottom = box->border[BOTTOM].عرض;
	} وإلا {
		x = (x_والد + box->x) * scale;
		y = (y_والد + box->y) * scale;
		عرض = box->عرض * scale;
		ارتفاع = box->ارتفاع * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_چپ = box->padding[چپ_2] ? box->padding[چپ_2] * scale
				: 0;
		padding_بالاترین = box->padding[بالاترین_2] ? box->padding[بالاترین_2] * scale
				: 0;
		padding_عرض = (box->padding[چپ_2] + box->عرض +
				box->padding[راست_2]) * scale;
		padding_ارتفاع = (box->padding[بالاترین_2] + box->ارتفاع +
				box->padding[BOTTOM]) * scale;
		border_چپ = box->border[چپ_2].عرض * scale;
		border_بالاترین = box->border[بالاترین_2].عرض * scale;
		border_راست = box->border[راست_2].عرض * scale;
		border_bottom = box->border[BOTTOM].عرض * scale;
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && سرریز_x != CSS_سرریز_VISIBLE &&
			box->والد != عدم) {
		/* box contents clipped to box size */
		r.x0 = x - border_چپ;
		r.x1 = x + padding_عرض + border_راست;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} وإلا {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		إذا (!box->والد) {
			/* root element */
			صحيح margin_چپ, margin_راست;
			إذا (scale == 1.0) {
				margin_چپ = box->margin[چپ_2];
				margin_راست = box->margin[راست_2];
			} وإلا {
				margin_چپ = box->margin[چپ_2] * scale;
				margin_راست = box->margin[راست_2] * scale;
			}
			r.x0 = x - border_چپ - margin_چپ < r.x0 ?
					x - border_چپ - margin_چپ : r.x0;
			r.x1 = x + padding_عرض + border_راست +
					margin_راست > r.x1 ?
					x + padding_عرض + border_راست +
					margin_راست : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && سرریز_y != CSS_سرریز_VISIBLE &&
			box->والد != عدم) {
		/* box contents clipped to box size */
		r.y0 = y - border_بالاترین;
		r.y1 = y + padding_ارتفاع + border_bottom;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} وإلا {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		إذا (!box->والد) {
			/* root element */
			صحيح margin_بالاترین, margin_bottom;
			إذا (scale == 1.0) {
				margin_بالاترین = box->margin[بالاترین_2];
				margin_bottom = box->margin[BOTTOM];
			} وإلا {
				margin_بالاترین = box->margin[بالاترین_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_بالاترین - margin_بالاترین < r.y0 ?
					y - border_بالاترین - margin_بالاترین : r.y0;
			r.y1 = y + padding_ارتفاع + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_ارتفاع + border_bottom +
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
				إذا (r.y0 < html_redraw_printing_بالاترین_cropped)
					html_redraw_printing_بالاترین_cropped = r.y0;
				أرجع true;
			}
		}
		وإلا box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	إذا (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		إذا ((ctx->plot->گروه_شروع) &&
		    (ctx->plot->گروه_شروع(ctx, "hidden box") != NSERROR_OK))
			أرجع false;
		إذا (!html_redraw_box_children(html, box, x_والد, y_والد,
				&r, scale, current_background_color, ctx))
			أرجع false;
		أرجع ((!ctx->plot->گروه_انتها) || (ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
	}

	إذا ((ctx->plot->گروه_شروع) &&
	    (ctx->plot->گروه_شروع(ctx,"vis box") != NSERROR_OK)) {
		أرجع false;
	}

	إذا (box->style != عدم &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		إذا (css_rect.چپ_auto == false)
			r.x0 = x - border_چپ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.چپ, css_rect.lunit));

		إذا (css_rect.بالاترین_auto == false)
			r.y0 = y - border_بالاترین + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.بالاترین, css_rect.tunit));

		إذا (css_rect.راست_auto == false)
			r.x1 = x - border_چپ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.راست, css_rect.runit));

		إذا (css_rect.bottom_auto == false)
			r.y1 = y - border_بالاترین + FIXTOINT(css_unit_len2device_px(
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
			أرجع ((!ctx->plot->گروه_انتها) ||
				(ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
		/* clip to it */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;

	} وإلا إذا (box->type == BOX_بلوک || box->type == BOX_INLINE_بلوک ||
			box->type == BOX_جدول_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		إذا (r.x0 < clip->x0) r.x0 = clip->x0;
		إذا (r.y0 < clip->y0) r.y0 = clip->y0;
		إذا (clip->x1 < r.x1) r.x1 = clip->x1;
		إذا (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		إذا (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			أرجع ((!ctx->plot->گروه_انتها) ||
				(ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
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

	bg_box = html_redraw_پیدا_کن_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	إذا (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_متن &&
			bg_box->type != BOX_INLINE_انتها &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != عدم &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		بنية rect p;
		p.x0 = x - border_چپ < r.x0 ? r.x0 : x - border_چپ;
		p.y0 = y - border_بالاترین < r.y0 ? r.y0 : y - border_بالاترین;
		p.x1 = x + padding_عرض + border_راست < r.x1 ?
				x + padding_عرض + border_راست : r.x1;
		p.y1 = y + padding_ارتفاع + border_bottom < r.y1 ?
				y + padding_ارتفاع + border_bottom : r.y1;
		إذا (!box->والد) {
			/* Root element, special case:
			 * background covers margins too */
			صحيح m_چپ, m_بالاترین, m_راست, m_bottom;
			إذا (scale == 1.0) {
				m_چپ = box->margin[چپ_2];
				m_بالاترین = box->margin[بالاترین_2];
				m_راست = box->margin[راست_2];
				m_bottom = box->margin[BOTTOM];
			} وإلا {
				m_چپ = box->margin[چپ_2] * scale;
				m_بالاترین = box->margin[بالاترین_2] * scale;
				m_راست = box->margin[راست_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_چپ < r.x0 ? r.x0 : p.x0 - m_چپ;
			p.y0 = p.y0 - m_بالاترین < r.y0 ? r.y0 : p.y0 - m_بالاترین;
			p.x1 = p.x1 + m_راست < r.x1 ? p.x1 + m_راست : r.x1;
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
	    box->type != BOX_INLINE_انتها &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != عدم &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_بالاترین || border_راست || border_bottom || border_چپ)) {
		إذا (!html_redraw_borders(box, x_والد, y_والد,
				padding_عرض, padding_ارتفاع, &r,
				scale, ctx))
			أرجع false;
	}

	/* backgrounds and borders for non-replaced inlines */
	إذا (box->style && box->type == BOX_INLINE && box->inline_انتها &&
			(html_redraw_box_has_background(box) ||
			border_بالاترین || border_راست ||
			border_bottom || border_چپ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		بنية box *ib;
		بنية rect b; /* border edge rectangle */
		بنية rect p; /* clipped rect */
		bool اولی = true;
		صحيح ib_x;
		صحيح ib_y = y;
		صحيح ib_p_عرض;
		صحيح ib_b_چپ, ib_b_راست;

		b.x0 = x - border_چپ;
		b.x1 = x + padding_عرض + border_راست;
		b.y0 = y - border_بالاترین;
		b.y1 = y + padding_ارتفاع + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		لكل (ib = box; ib; ib = ib->بعدی) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			إذا (ib->type == BOX_FLOAT_چپ ||
					ib->type == BOX_FLOAT_راست)
				استمر;
			إذا (scale == 1.0) {
				ib_x = x_والد + ib->x;
				ib_y = y_والد + ib->y;
				ib_p_عرض = ib->padding[چپ_2] + ib->عرض +
						ib->padding[راست_2];
				ib_b_چپ = ib->border[چپ_2].عرض;
				ib_b_راست = ib->border[راست_2].عرض;
			} وإلا {
				ib_x = (x_والد + ib->x) * scale;
				ib_y = (y_والد + ib->y) * scale;
				ib_p_عرض = (ib->padding[چپ_2] + ib->عرض +
						ib->padding[راست_2]) * scale;
				ib_b_چپ = ib->border[چپ_2].عرض * scale;
				ib_b_راست = ib->border[راست_2].عرض * scale;
			}

			إذا ((ib->flags & NEW_سطر) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				إذا (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						اولی, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					أرجع false;
				/* restore previous graphics window */
				إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					أرجع false;
				إذا (!html_redraw_inline_borders(box, b, &r,
						scale, اولی, false, ctx))
					أرجع false;
				/* reset coords */
				b.x0 = ib_x - ib_b_چپ;
				b.y0 = ib_y - border_بالاترین - padding_بالاترین;
				b.y1 = ib_y + padding_ارتفاع - padding_بالاترین +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				اولی = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_عرض + ib_b_راست;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			إذا (ib == box->inline_انتها)
				/* reached end of BOX_INLINE span */
				اكسر;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		إذا (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				اولی, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			أرجع false;
		/* restore previous graphics window */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
		إذا (!html_redraw_inline_borders(box, b, &r, scale, اولی, true,
				ctx))
			أرجع false;

	}

	/* Debug outlines */
	إذا (html_redraw_debug) {
		صحيح margin_چپ, margin_راست;
		صحيح margin_بالاترین, margin_bottom;
		إذا (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_چپ = box->margin[چپ_2];
			margin_بالاترین = box->margin[بالاترین_2];
			margin_راست = box->margin[راست_2];
			margin_bottom = box->margin[BOTTOM];
		} وإلا {
			margin_چپ = box->margin[چپ_2] * scale;
			margin_بالاترین = box->margin[بالاترین_2] * scale;
			margin_راست = box->margin[راست_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_چپ;
		rect.y0 = y + padding_بالاترین;
		rect.x1 = x + padding_چپ + عرض;
		rect.y1 = y + padding_بالاترین + ارتفاع;
		إذا (ctx->plot->rectangle(ctx, plot_style_content_یال, &rect) != NSERROR_OK)
			أرجع false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_عرض;
		rect.y1 = y + padding_ارتفاع;
		إذا (ctx->plot->rectangle(ctx, plot_style_padding_یال, &rect) != NSERROR_OK)
			أرجع false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_چپ - margin_چپ;
		rect.y0 = y - border_بالاترین - margin_بالاترین;
		rect.x1 = x + padding_عرض + border_راست + margin_راست;
		rect.y1 = y + padding_ارتفاع + border_bottom + margin_bottom;
		إذا (ctx->plot->rectangle(ctx, plot_style_margin_یال, &rect) != NSERROR_OK)
			أرجع false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	إذا (box->والد != عدم) {
		bool need_clip = false;
		إذا (box->object || box->flags & IFRAME ||
				(سرریز_x != CSS_سرریز_VISIBLE &&
				 سرریز_y != CSS_سرریز_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_عرض;
			r.y1 = y + padding_ارتفاع;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.x1 <= r.x0 || r.y1 <= r.y0) {
				أرجع (!ctx->plot->گروه_انتها ||
					(ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (سرریز_x != CSS_سرریز_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_عرض;
			r.y1 = clip->y1;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (r.x1 <= r.x0) {
				أرجع (!ctx->plot->گروه_انتها ||
					(ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (سرریز_y != CSS_سرریز_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_ارتفاع;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.y1 <= r.y0) {
				أرجع (!ctx->plot->گروه_انتها ||
					(ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		إذا (need_clip &&
		    (box->type == BOX_بلوک ||
		     box->type == BOX_INLINE_بلوک ||
		     box->type == BOX_جدول_CELL || box->object)) {
			إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				أرجع false;
		}
	}

	/* text decoration */
	إذا ((box->type != BOX_متن) &&
	    box->style &&
	    css_computed_متن_decoration(box->style) !=	CSS_متن_DECORATION_NONE) {
		إذا (!html_redraw_متن_decoration(box, x_والد, y_والد,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->گره != عدم) {
		exc = dom_html_element_get_برچسب_type(box->گره, &برچسب_type);
		إذا (exc != DOM_NO_ERR) {
			برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} وإلا {
		برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	إذا (box->object && عرض != 0 && ارتفاع != 0) {
		بنية content_redraw_داده obj_داده;

		x_scrolled = x - scrollbar_get_جابه_جایی(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_جابه_جایی(box->scroll_y) * scale;

		obj_داده.x = x_scrolled + padding_چپ;
		obj_داده.y = y_scrolled + padding_بالاترین;
		obj_داده.عرض = عرض;
		obj_داده.ارتفاع = ارتفاع;
		obj_داده.background_رنگ = current_background_color;
		obj_داده.scale = scale;
		obj_داده.repeat_x = false;
		obj_داده.repeat_y = false;

		إذا (content_get_type(box->object) == CONTENT_HTML) {
			obj_داده.x /= scale;
			obj_داده.y /= scale;
		}

		إذا (!content_redraw(box->object, &obj_داده, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ثابت حرف *obj = "\xef\xbf\xbc";
			صحيح obj_عرض;
			صحيح obj_x = x + padding_چپ;
			nserror res;

			rect.x0 = x + padding_چپ;
			rect.y0 = y + padding_بالاترین;
			rect.x1 = x + padding_چپ + عرض - 1;
			rect.y1 = y + padding_بالاترین + ارتفاع - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			res = guit->layout->عرض(plot_fstyle_broken_object,
						  obj,
						  حجم(obj) - 1,
						  &obj_عرض);
			إذا (res != NSERROR_OK) {
				obj_x += 1;
			} وإلا {
				obj_x += عرض / 2 - obj_عرض / 2;
			}

			إذا (ctx->plot->متن(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_بالاترین + (صحيح)(ارتفاع * 0.75),
					    obj, حجم(obj) - 1) != NSERROR_OK)
				أرجع false;
		}
	} وإلا إذا (برچسب_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->گره != عدم &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		بنية bitmap *bitmap = عدم;
		exc = dom_گره_get_user_داده(box->گره,
					     corestring_dom_ns_کلید_canvas_گره_داده,
					     &bitmap);
		إذا (exc != DOM_NO_ERR) {
			bitmap = عدم;
		}
		إذا (bitmap != عدم &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_چپ, y + padding_بالاترین,
				      عرض, ارتفاع, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			أرجع false;
	} وإلا إذا (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_پنجره_redraw(box->iframe,
				x + padding_چپ,
				y + padding_بالاترین, &r, ctx);

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		إذا (!html_redraw_checkbox(x + padding_چپ, y + padding_بالاترین,
				عرض, ارتفاع, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_RADIO) {
		إذا (!html_redraw_radio(x + padding_چپ, y + padding_بالاترین,
				عرض, ارتفاع, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_FILE) {
		إذا (!html_redraw_file(x + padding_چپ, y + padding_بالاترین,
				عرض, ارتفاع, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			أرجع false;

	} وإلا إذا (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->داده.متن.ta, x, y,
				current_background_color, scale, &r, ctx);

	} وإلا إذا (box->متن) {
		إذا (!html_redraw_متن_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			أرجع false;

	} وإلا {
		إذا (!html_redraw_box_children(html, box, x_والد, y_والد, &r,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->type == BOX_بلوک || box->type == BOX_INLINE_بلوک ||
			box->type == BOX_جدول_CELL || box->type == BOX_INLINE)
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;

	/* list marker */
	إذا (box->list_marker) {
		إذا (!html_redraw_box(html, box->list_marker,
				x_والد + box->x -
				scrollbar_get_جابه_جایی(box->scroll_x),
				y_والد + box->y -
				scrollbar_get_جابه_جایی(box->scroll_y),
				clip, scale, current_background_color, ctx))
			أرجع false;
	}

	/* scrollbars */
	إذا (((box->style && box->type != BOX_BR &&
	      box->type != BOX_جدول && box->type != BOX_INLINE &&
	      (box->gadget == عدم || box->gadget->type != GADGET_TEXTAREA) &&
	      (سرریز_x == CSS_سرریز_SCROLL ||
	       سرریز_x == CSS_سرریز_AUTO ||
	       سرریز_y == CSS_سرریز_SCROLL ||
	       سرریز_y == CSS_سرریز_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->والد != عدم) {
		nserror res;
		bool has_x_scroll = (سرریز_x == CSS_سرریز_SCROLL);
		bool has_y_scroll = (سرریز_y == CSS_سرریز_SCROLL);

		has_x_scroll |= (سرریز_x == CSS_سرریز_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (سرریز_y == CSS_سرریز_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((بنية content *)html,
					    box, has_x_scroll, has_y_scroll);
		إذا (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			أرجع false;
		}

		إذا (box->scroll_x != عدم)
			scrollbar_redraw(box->scroll_x,
					x_والد + box->x,
					y_والد + box->y + box->padding[بالاترین_2] +
					box->ارتفاع + box->padding[BOTTOM] -
					SCROLLBAR_عرض, clip, scale, ctx);
		إذا (box->scroll_y != عدم)
			scrollbar_redraw(box->scroll_y,
					x_والد + box->x + box->padding[چپ_2] +
					box->عرض + box->padding[راست_2] -
					SCROLLBAR_عرض,
					y_والد + box->y, clip, scale, ctx);
	}

	إذا (box->type == BOX_بلوک || box->type == BOX_INLINE_بلوک ||
	    box->type == BOX_جدول_CELL || box->type == BOX_INLINE) {
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;
	}

	أرجع ((!plot->گروه_انتها) || (ctx->plot->گروه_انتها(ctx) == NSERROR_OK));
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

bool html_redraw(بنية content *c, بنية content_redraw_داده *داده,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	بنية box *box;
	bool نتیجه = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_رنگ = داده->background_رنگ,
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
				داده->scale, clip);
	}

	إذا (!select_only) {
		/* clear to background colour */
		نتیجه = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		إذا (html->background_رنگ != NS_TRANSPARENT)
			pstyle_fill_bg.fill_رنگ = html->background_رنگ;

		نتیجه &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		نتیجه &= html_redraw_box(html, box, داده->x, داده->y, clip,
				داده->scale, pstyle_fill_bg.fill_رنگ, ctx);
	}

	إذا (select) {
		صحيح menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[چپ_2].عرض;
		menu_y += box->ارتفاع + box->border[BOTTOM].عرض +
				box->padding[BOTTOM] + box->padding[بالاترین_2];
		نتیجه &= form_redraw_select_menu(html->visible_select_menu,
				داده->x + menu_x, داده->y + menu_y,
				داده->scale, clip, ctx);
	}

	أرجع نتیجه;

}
