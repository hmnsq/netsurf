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

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/browser_window.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plotters.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/bitmap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/layout.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_protected.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/textsearch.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/selection.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/print.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/gui_internal.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_manipulate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/font.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ਸਥਿਰ bool html_redraw_box_has_background(ਬਣਤਰ box *box)
{
	ਜੇ (box->background != ਨਿਰਾ)
		ਵਾਪਸ true;

	ਜੇ (box->style != ਨਿਰਾ) {
		css_color ਰੰਗ;

		css_computed_background_color(box->style, &ਰੰਗ);

		ਜੇ (nscss_color_is_transparent(ਰੰਗ) == false)
			ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ਸਥਿਰ ਬਣਤਰ box *html_redraw_ਲੱਭੋ_bg_box(ਬਣਤਰ box *box)
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
	ਜੇ (box->ਮਾਪਾ == ਨਿਰਾ) {
		/* Root box */
		ਜੇ (html_redraw_box_has_background(box))
			ਵਾਪਸ box;

		/* No background on root box: consider body box, if any */
		ਜੇ (box->children != ਨਿਰਾ) {
			ਜੇ (html_redraw_box_has_background(box->children))
				ਵਾਪਸ box->children;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (box->ਮਾਪਾ != ਨਿਰਾ && box->ਮਾਪਾ->ਮਾਪਾ == ਨਿਰਾ) {
		/* Body box: only render background if root has its own */
		ਜੇ (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->ਮਾਪਾ))
			ਵਾਪਸ box;
	} ਨਹੀਂਤਾਂ {
		/* Any other box */
		ਜੇ (html_redraw_box_has_background(box))
			ਵਾਪਸ box;
	}

	ਵਾਪਸ ਨਿਰਾ;
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

ਸਥਿਰ bool
ਪਾਠ_redraw(ਅਚਲ ਅੱਖਰ *utf8_ਪਾਠ,
	    size_t utf8_len,
	    size_t ਵਿੱਥ,
	    ਪੂਰਨ space,
	    ਅਚਲ plot_font_style_t *fstyle,
	    ਪੂਰਨ x,
	    ਪੂਰਨ y,
	    ਅਚਲ ਬਣਤਰ rect *clip,
	    ਪੂਰਨ ਉਚਾਈ,
	    ਦਸ਼ਮਲਵ scale,
	    bool excluded,
	    ਬਣਤਰ content *c,
	    ਅਚਲ ਬਣਤਰ selection *sel,
	    ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.ਮਾਪ *= scale;

	/* is this box part of a selection? */
	ਜੇ (!excluded && ctx->interactive == true) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ len = utf8_len + (space ? 1 : 0);
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_idx;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_idx;

		/* first try the browser window's current selection */
		ਜੇ (selection_highlighted(sel,
					  ਵਿੱਥ,
					  ਵਿੱਥ + len,
					  &ਸ਼ੁਰੂ_idx,
					  &ਸਿਰਾ_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ਜੇ (!highlighted &&
		    (c->textsearch.context != ਨਿਰਾ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     ਵਿੱਥ,
						     ਵਿੱਥ + len,
						     &ਸ਼ੁਰੂ_idx,
						     &ਸਿਰਾ_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ਜੇ (highlighted) {
			ਬਣਤਰ rect r;
			ਬਿਨਾਨਿਸ਼ਾਨੀ endtxt_idx = ਸਿਰਾ_idx;
			bool clip_ਬਦਲਿਆ = false;
			bool ਪਾਠ_visible = true;
			ਪੂਰਨ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ਜੇ (ਸਿਰਾ_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰਾ_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->ਚੌੜਾਈ(fstyle,
						  utf8_ਪਾਠ, ਸ਼ੁਰੂ_idx,
						  &startx);
			ਜੇ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->ਚੌੜਾਈ(fstyle,
						  utf8_ਪਾਠ, endtxt_idx,
						  &endx);
			ਜੇ (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ਜੇ (ਸਿਰਾ_idx > utf8_len) {
					endx += space;
			}

			ਜੇ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ਜੇ ((ਸ਼ੁਰੂ_idx > 0) &&
			    (ctx->plot->ਪਾਠ(ctx,
					     &plot_fstyle,
					     x,
					     y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
					     utf8_ਪਾਠ,
					     ਸ਼ੁਰੂ_idx) != NSERROR_OK))
				ਵਾਪਸ false;

			pstyle_fill_hback.fill_ਰੰਗ = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ਉਚਾਈ * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}

			ਜੇ (ਸ਼ੁਰੂ_idx > 0) {
				ਪੂਰਨ px0 = max(x + startx, clip->x0);
				ਪੂਰਨ px1 = min(x + endx, clip->x1);

				ਜੇ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}

					clip_ਬਦਲਿਆ = true;
				} ਨਹੀਂਤਾਂ {
					ਪਾਠ_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_ਰੰਗ;
			fstyle_hback.foreground = ਰੰਗ_ਮੰਜ਼ਿਲ_bw_furthest(
				pstyle_fill_hback.fill_ਰੰਗ);

			ਜੇ (ਪਾਠ_visible &&
			    (ctx->plot->ਪਾਠ(ctx,
					     &fstyle_hback,
					     x,
					     y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
					     utf8_ਪਾਠ,
					     endtxt_idx) != NSERROR_OK)) {
				ਵਾਪਸ false;
			}

			/* draw any text succeeding highlighted portion */
			ਜੇ (endtxt_idx < utf8_len) {
				ਪੂਰਨ px0 = max(x + endx, clip->x0);
				ਜੇ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}

					clip_ਬਦਲਿਆ = true;

					res = ctx->plot->ਪਾਠ(ctx,
							      &plot_fstyle,
							      x,
							      y + (ਪੂਰਨ)(ਉਚਾਈ * 0.75 * scale),
							      utf8_ਪਾਠ,
							      utf8_len);
					ਜੇ (res != NSERROR_OK) {
						ਵਾਪਸ false;
					}
				}
			}

			ਜੇ (clip_ਬਦਲਿਆ &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ਵਾਪਸ false;
			}
		}
	}

	ਜੇ (!highlighted) {
		res = ctx->plot->ਪਾਠ(ctx,
				      &plot_fstyle,
				      x,
				      y + (ਪੂਰਨ) (ਉਚਾਈ * 0.75 * scale),
				      utf8_ਪਾਠ,
				      utf8_len);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}
	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_checkbox(ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ,
		bool selected, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਦੁੱਗਣਾ z;
	nserror res;
	ਬਣਤਰ rect rect;

	z = ਚੌੜਾਈ * 0.15;
	ਜੇ (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + ਚੌੜਾਈ;
	rect.y1 = y + ਉਚਾਈ;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_darkwbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + ਉਚਾਈ;
	res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_darkwbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* light line across right */
	rect.x0 = x + ਚੌੜਾਈ;
	rect.x1 = x + ਚੌੜਾਈ;
	res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_lightwbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + ਉਚਾਈ;
	res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_lightwbasec, &rect);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	ਜੇ (selected) {
		ਜੇ (ਚੌੜਾਈ < 12 || ਉਚਾਈ < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + ਚੌੜਾਈ - z;
			rect.y1 = y + ਉਚਾਈ - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
		} ਨਹੀਂਤਾਂ {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + ਚੌੜਾਈ - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + ਉਚਾਈ - z;
			res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_wblobc, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + ਉਚਾਈ - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (ਉਚਾਈ / 2);
			res = ctx->plot->ਸਤਰ(ctx, plot_style_stroke_wblobc, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
		}
	}
	ਵਾਪਸ true;
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
ਸਥਿਰ bool html_redraw_radio(ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ,
		bool selected, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + ਚੌੜਾਈ * 0.5,
			      y + ਉਚਾਈ * 0.5,
			      ਚੌੜਾਈ * 0.5 - 1);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + ਚੌੜਾਈ * 0.5,
			     y + ਉਚਾਈ * 0.5,
			     ਚੌੜਾਈ * 0.5 - 1,
			     45,
			     225);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + ਚੌੜਾਈ * 0.5,
			     y + ਉਚਾਈ * 0.5,
			     ਚੌੜਾਈ * 0.5 - 1,
			     225,
			     45);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}

	ਜੇ (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + ਚੌੜਾਈ * 0.5,
				      y + ਉਚਾਈ * 0.5,
				      ਚੌੜਾਈ * 0.3 - 1);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}

	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_file(ਪੂਰਨ x, ਪੂਰਨ y, ਪੂਰਨ ਚੌੜਾਈ, ਪੂਰਨ ਉਚਾਈ,
		ਬਣਤਰ box *box, ਦਸ਼ਮਲਵ scale, ਰੰਗ background_ਰੰਗ,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਪੂਰਨ ਪਾਠ_ਚੌੜਾਈ;
	ਅਚਲ ਅੱਖਰ *ਪਾਠ;
	size_t ਲੰਮਾਈ;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_ਰੰਗ;

	ਜੇ (box->gadget->ਮੁੱਲ) {
		ਪਾਠ = box->gadget->ਮੁੱਲ;
	} ਨਹੀਂਤਾਂ {
		ਪਾਠ = messages_get("Form_Drop");
	}
	ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(ਪਾਠ);

	res = guit->layout->ਚੌੜਾਈ(&fstyle, ਪਾਠ, ਲੰਮਾਈ, &ਪਾਠ_ਚੌੜਾਈ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}
	ਪਾਠ_ਚੌੜਾਈ *= scale;
	ਜੇ (ਚੌੜਾਈ < ਪਾਠ_ਚੌੜਾਈ + 8) {
		x = x + ਚੌੜਾਈ - ਪਾਠ_ਚੌੜਾਈ - 4;
	} ਨਹੀਂਤਾਂ {
		x = x + 4;
	}

	res = ctx->plot->ਪਾਠ(ctx, &fstyle, x, y + ਉਚਾਈ * 0.75, ਪਾਠ, ਲੰਮਾਈ);
	ਜੇ (res != NSERROR_OK) {
		ਵਾਪਸ false;
	}
	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_background(ਪੂਰਨ x, ਪੂਰਨ y, ਬਣਤਰ box *box, ਦਸ਼ਮਲਵ scale,
		ਅਚਲ ਬਣਤਰ rect *clip, ਰੰਗ *background_ਰੰਗ,
		ਬਣਤਰ box *background,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_ਰੰਗ = true;
	bool plot_content;
	bool clip_ਮੰਜ਼ਿਲ_children = false;
	ਬਣਤਰ box *clip_box = box;
	ਪੂਰਨ ox = x, oy = y;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ਬਣਤਰ box *ਮਾਪਾ;
	ਬਣਤਰ rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ਰੰਗ = *background_ਰੰਗ,
	};
	nserror res;

	ਜੇ (ctx->background_images == false)
		ਵਾਪਸ true;

	plot_content = (background->background != ਨਿਰਾ);

	ਜੇ (plot_content) {
		ਜੇ (!box->ਮਾਪਾ) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ਖੱਬਾ_2] * scale;
			y -= box->margin[ਸਿਖਰ_2] * scale;
			ਚੌੜਾਈ = box->margin[ਖੱਬਾ_2] + box->padding[ਖੱਬਾ_2] +
					box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] +
					box->margin[ਸੱਜਾ_2];
			ਉਚਾਈ = box->margin[ਸਿਖਰ_2] + box->padding[ਸਿਖਰ_2] +
					box->ਉਚਾਈ + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} ਨਹੀਂਤਾਂ {
			ਚੌੜਾਈ = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ +
					box->padding[ਸੱਜਾ_2];
			ਉਚਾਈ = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		ਚੋਣ (css_computed_background_repeat(background->style)) {
		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_ਰੰਗ = !content_get_opaque(background->background);
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ਰੋਕੋ;

		ਮੂਲ:
			ਰੋਕੋ;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ਜੇ (hunit == CSS_UNIT_PCT) {
			x += (ਚੌੜਾਈ -
				content_get_ਚੌੜਾਈ(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} ਨਹੀਂਤਾਂ {
			x += (ਪੂਰਨ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ਜੇ (vunit == CSS_UNIT_PCT) {
			y += (ਉਚਾਈ -
				content_get_ਉਚਾਈ(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} ਨਹੀਂਤਾਂ {
			y += (ਪੂਰਨ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ਜੇ (box->type == BOX_ਸਾਰਣੀ_ਪੰਕਤੀ) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		ਲਈ (ਮਾਪਾ = box->ਮਾਪਾ;
			((ਮਾਪਾ) && (ਮਾਪਾ->type != BOX_ਸਾਰਣੀ));
				ਮਾਪਾ = ਮਾਪਾ->ਮਾਪਾ);
		ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪਾ && (ਮਾਪਾ->style));

		css_computed_border_spacing(ਮਾਪਾ->style, &h, &hu, &v, &vu);

		clip_ਮੰਜ਼ਿਲ_children = (h > 0) || (v > 0);

		ਜੇ (clip_ਮੰਜ਼ਿਲ_children)
			clip_box = box->children;
	}

	ਲਈ (; clip_box; clip_box = clip_box->ਅਗਲਾ) {
		/* clip to child boxes if needed */
		ਜੇ (clip_ਮੰਜ਼ਿਲ_children) {
			ਪ੍ਰਤਿਜ੍ਞਾ(clip_box->type == BOX_ਸਾਰਣੀ_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ਖੱਬਾ_2] +
					clip_box->ਚੌੜਾਈ +
					clip_box->padding[ਸੱਜਾ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[ਸਿਖਰ_2] +
					clip_box->ਉਚਾਈ +
					clip_box->padding[BOTTOM]) * scale;

			ਜੇ (r.x0 < clip->x0) r.x0 = clip->x0;
			ਜੇ (r.y0 < clip->y0) r.y0 = clip->y0;
			ਜੇ (r.x1 > clip->x1) r.x1 = clip->x1;
			ਜੇ (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ਜੇ ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ਨਿਰਾ) &&
			     content_get_opaque(clip_box->background)))
				ਜਾਰੀ;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ਜੇ (nscss_color_is_transparent(bgcol) == false) {
			*background_ਰੰਗ = nscss_color_ਮੰਜ਼ਿਲ_ns(bgcol);
			pstyle_fill_bg.fill_ਰੰਗ = *background_ਰੰਗ;
			ਜੇ (plot_ਰੰਗ) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ਜੇ (res != NSERROR_OK) {
					ਵਾਪਸ false;
				}
			}
		}
		/* and plot the image */
		ਜੇ (plot_content) {
			ਚੌੜਾਈ = content_get_ਚੌੜਾਈ(background->background);
			ਉਚਾਈ = content_get_ਉਚਾਈ(background->background);

			/* ensure clip area only as large as required */
			ਜੇ (!repeat_x) {
				ਜੇ (r.x0 < x)
					r.x0 = x;
				ਜੇ (r.x1 > x + ਚੌੜਾਈ * scale)
					r.x1 = x + ਚੌੜਾਈ * scale;
			}
			ਜੇ (!repeat_y) {
				ਜੇ (r.y0 < y)
					r.y0 = y;
				ਜੇ (r.y1 > y + ਉਚਾਈ * scale)
					r.y1 = y + ਉਚਾਈ * scale;
			}
			/* valid clipping rectangles only */
			ਜੇ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ਬਣਤਰ content_redraw_ਅੰਕੜੇ bg_ਅੰਕੜੇ;

				res = ctx->plot->clip(ctx, &r);
				ਜੇ (res != NSERROR_OK) {
					ਵਾਪਸ false;
				}

				bg_ਅੰਕੜੇ.x = x;
				bg_ਅੰਕੜੇ.y = y;
				bg_ਅੰਕੜੇ.ਚੌੜਾਈ = ceilf(ਚੌੜਾਈ * scale);
				bg_ਅੰਕੜੇ.ਉਚਾਈ = ceilf(ਉਚਾਈ * scale);
				bg_ਅੰਕੜੇ.background_ਰੰਗ = *background_ਰੰਗ;
				bg_ਅੰਕੜੇ.scale = scale;
				bg_ਅੰਕੜੇ.repeat_x = repeat_x;
				bg_ਅੰਕੜੇ.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_ਅੰਕੜੇ, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		ਜੇ (!clip_ਮੰਜ਼ਿਲ_children)
			ਵਾਪਸ true;
	}
	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_inline_background(ਪੂਰਨ x, ਪੂਰਨ y, ਬਣਤਰ box *box,
		ਦਸ਼ਮਲਵ scale, ਅਚਲ ਬਣਤਰ rect *clip, ਬਣਤਰ rect b,
		bool ਪਹਿਲਾ, bool ਆਖਰੀ, ਰੰਗ *background_ਰੰਗ,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ rect r = *clip;
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_ਰੰਗ = true;
	bool plot_content;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ਰੰਗ = *background_ਰੰਗ,
	};
	nserror res;

	plot_content = (box->background != ਨਿਰਾ);

	ਜੇ (html_redraw_printing && nsoption_bool(remove_backgrounds))
		ਵਾਪਸ true;

	ਜੇ (plot_content) {
		/* handle background-repeat */
		ਚੋਣ (css_computed_background_repeat(box->style)) {
		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_ਰੰਗ = !content_get_opaque(box->background);
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ਰੋਕੋ;

		ਹਾਲਤ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ਰੋਕੋ;

		ਮੂਲ:
			ਰੋਕੋ;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ਜੇ (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_ਚੌੜਾਈ(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ਜੇ (!repeat_x && ((hpos < 2 && !ਪਹਿਲਾ) ||
					(hpos > 98 && !ਆਖਰੀ))){
				plot_content = false;
			}
		} ਨਹੀਂਤਾਂ {
			x += (ਪੂਰਨ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ਜੇ (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_ਉਚਾਈ(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} ਨਹੀਂਤਾਂ {
			y += (ਪੂਰਨ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ਜੇ (nscss_color_is_transparent(bgcol) == false) {
		*background_ਰੰਗ = nscss_color_ਮੰਜ਼ਿਲ_ns(bgcol);
		pstyle_fill_bg.fill_ਰੰਗ = *background_ਰੰਗ;

		ਜੇ (plot_ਰੰਗ) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
		}
	}
	/* and plot the image */
	ਜੇ (plot_content) {
		ਪੂਰਨ ਚੌੜਾਈ = content_get_ਚੌੜਾਈ(box->background);
		ਪੂਰਨ ਉਚਾਈ = content_get_ਉਚਾਈ(box->background);

		ਜੇ (!repeat_x) {
			ਜੇ (r.x0 < x)
				r.x0 = x;
			ਜੇ (r.x1 > x + ਚੌੜਾਈ * scale)
				r.x1 = x + ਚੌੜਾਈ * scale;
		}
		ਜੇ (!repeat_y) {
			ਜੇ (r.y0 < y)
				r.y0 = y;
			ਜੇ (r.y1 > y + ਉਚਾਈ * scale)
				r.y1 = y + ਉਚਾਈ * scale;
		}
		/* valid clipping rectangles only */
		ਜੇ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ਬਣਤਰ content_redraw_ਅੰਕੜੇ bg_ਅੰਕੜੇ;

			res = ctx->plot->clip(ctx, &r);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}

			bg_ਅੰਕੜੇ.x = x;
			bg_ਅੰਕੜੇ.y = y;
			bg_ਅੰਕੜੇ.ਚੌੜਾਈ = ceilf(ਚੌੜਾਈ * scale);
			bg_ਅੰਕੜੇ.ਉਚਾਈ = ceilf(ਉਚਾਈ * scale);
			bg_ਅੰਕੜੇ.background_ਰੰਗ = *background_ਰੰਗ;
			bg_ਅੰਕੜੇ.scale = scale;
			bg_ਅੰਕੜੇ.repeat_x = repeat_x;
			bg_ਅੰਕੜੇ.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_ਅੰਕੜੇ, &r, ctx);
		}
	}

	ਵਾਪਸ true;
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

ਸਥਿਰ bool
html_redraw_ਪਾਠ_decoration_inline(ਬਣਤਰ box *box,
				   ਪੂਰਨ x, ਪੂਰਨ y,
				   ਦਸ਼ਮਲਵ scale,
				   ਰੰਗ ਰੰਗ,
				   ਦਸ਼ਮਲਵ ਅਨੁਪਾਤ,
				   ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_ਰੰਗ = ਰੰਗ,
	};
	nserror res;
	ਬਣਤਰ rect rect;

	ਲਈ (c = box->ਅਗਲਾ;
	     c && c != box->inline_ਸਿਰਾ;
	     c = c->ਅਗਲਾ) {
		ਜੇ (c->type != BOX_ਪਾਠ) {
			ਜਾਰੀ;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->ਉਚਾਈ * ਅਨੁਪਾਤ) * scale;
		rect.x1 = (x + c->x + c->ਚੌੜਾਈ) * scale;
		rect.y1 = (y + c->y + c->ਉਚਾਈ * ਅਨੁਪਾਤ) * scale;
		res = ctx->plot->ਸਤਰ(ctx, &plot_style_box, &rect);
		ਜੇ (res != NSERROR_OK) {
			ਵਾਪਸ false;
		}
	}
	ਵਾਪਸ true;
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

ਸਥਿਰ bool
html_redraw_ਪਾਠ_decoration_ਖੰਡ(ਬਣਤਰ box *box,
				  ਪੂਰਨ x, ਪੂਰਨ y,
				  ਦਸ਼ਮਲਵ scale,
				  ਰੰਗ ਰੰਗ,
				  ਦਸ਼ਮਲਵ ਅਨੁਪਾਤ,
				  ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_ਰੰਗ = ਰੰਗ,
	};
	nserror res;
	ਬਣਤਰ rect rect;

	/* draw through text descendants */
	ਲਈ (c = box->children; c; c = c->ਅਗਲਾ) {
		ਜੇ (c->type == BOX_ਪਾਠ) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->ਉਚਾਈ * ਅਨੁਪਾਤ) * scale;
			rect.x1 = (x + c->x + c->ਚੌੜਾਈ) * scale;
			rect.y1 = (y + c->y + c->ਉਚਾਈ * ਅਨੁਪਾਤ) * scale;
			res = ctx->plot->ਸਤਰ(ctx, &plot_style_box, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}
		} ਨਹੀਂਤਾਂ ਜੇ ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_ਖੰਡ)) {
			ਜੇ (!html_redraw_ਪਾਠ_decoration_ਖੰਡ(c,
					x + c->x, y + c->y,
					scale, ਰੰਗ, ਅਨੁਪਾਤ, ctx))
				ਵਾਪਸ false;
		}
	}
	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_ਪਾਠ_decoration(ਬਣਤਰ box *box,
		ਪੂਰਨ x_ਮਾਪਾ, ਪੂਰਨ y_ਮਾਪਾ, ਦਸ਼ਮਲਵ scale,
		ਰੰਗ background_ਰੰਗ, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਸਥਿਰ ਅਚਲ ਗਿਣਤੀ css_ਪਾਠ_decoration_e decoration[] = {
		CSS_ਪਾਠ_DECORATION_UNDERLINE, CSS_ਪਾਠ_DECORATION_OVERLINE,
		CSS_ਪਾਠ_DECORATION_ਸਤਰ_THROUGH };
	ਸਥਿਰ ਅਚਲ ਦਸ਼ਮਲਵ ਸਤਰ_ਅਨੁਪਾਤ[] = { 0.9, 0.1, 0.5 };
	ਰੰਗ fgcol;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	css_color ਖਾਨਾ;

	css_computed_color(box->style, &ਖਾਨਾ);
	fgcol = nscss_color_ਮੰਜ਼ਿਲ_ns(ਖਾਨਾ);

	/* antialias colour for under/overline */
	ਜੇ (html_redraw_printing == false)
		fgcol = blend_ਰੰਗ(background_ਰੰਗ, fgcol);

	ਜੇ (box->type == BOX_INLINE) {
		ਜੇ (!box->inline_ਸਿਰਾ)
			ਵਾਪਸ true;
		ਲਈ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ਜੇ (css_computed_ਪਾਠ_decoration(box->style) &
					decoration[i])
				ਜੇ (!html_redraw_ਪਾਠ_decoration_inline(box,
						x_ਮਾਪਾ, y_ਮਾਪਾ, scale,
						fgcol, ਸਤਰ_ਅਨੁਪਾਤ[i], ctx))
					ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		ਲਈ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ਜੇ (css_computed_ਪਾਠ_decoration(box->style) &
					decoration[i])
				ਜੇ (!html_redraw_ਪਾਠ_decoration_ਖੰਡ(box,
						x_ਮਾਪਾ + box->x,
						y_ਮਾਪਾ + box->y,
						scale,
						fgcol, ਸਤਰ_ਅਨੁਪਾਤ[i], ctx))
					ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
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

ਸਥਿਰ bool html_redraw_ਪਾਠ_box(ਅਚਲ html_content *html, ਬਣਤਰ box *box,
		ਪੂਰਨ x, ਪੂਰਨ y, ਅਚਲ ਬਣਤਰ rect *clip, ਦਸ਼ਮਲਵ scale,
		ਰੰਗ current_background_color,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	bool excluded = (box->object != ਨਿਰਾ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ਜੇ (!ਪਾਠ_redraw(box->ਪਾਠ,
			 box->ਲੰਮਾਈ,
			 box->byte_ਵਿੱਥ,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->ਉਚਾਈ,
			 scale,
			 excluded,
			 (ਬਣਤਰ content *)html,
			 html->sel,
			 ctx))
		ਵਾਪਸ false;

	ਵਾਪਸ true;
}

bool html_redraw_box(ਅਚਲ html_content *html, ਬਣਤਰ box *box,
		ਪੂਰਨ x_ਮਾਪਾ, ਪੂਰਨ y_ਮਾਪਾ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਦਸ਼ਮਲਵ scale,
		ਰੰਗ current_background_color,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx);

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

ਸਥਿਰ bool html_redraw_box_children(ਅਚਲ html_content *html, ਬਣਤਰ box *box,
		ਪੂਰਨ x_ਮਾਪਾ, ਪੂਰਨ y_ਮਾਪਾ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਦਸ਼ਮਲਵ scale,
		ਰੰਗ current_background_color,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਬਣਤਰ box *c;

	ਲਈ (c = box->children; c; c = c->ਅਗਲਾ) {

		ਜੇ (c->type != BOX_FLOAT_ਖੱਬਾ && c->type != BOX_FLOAT_ਸੱਜਾ)
			ਜੇ (!html_redraw_box(html, c,
					x_ਮਾਪਾ + box->x -
					scrollbar_get_ਵਿੱਥ(box->scroll_x),
					y_ਮਾਪਾ + box->y -
					scrollbar_get_ਵਿੱਥ(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				ਵਾਪਸ false;
	}
	ਲਈ (c = box->float_children; c; c = c->ਅਗਲਾ_float)
		ਜੇ (!html_redraw_box(html, c,
				x_ਮਾਪਾ + box->x -
				scrollbar_get_ਵਿੱਥ(box->scroll_x),
				y_ਮਾਪਾ + box->y -
				scrollbar_get_ਵਿੱਥ(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			ਵਾਪਸ false;

	ਵਾਪਸ true;
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

bool html_redraw_box(ਅਚਲ html_content *html, ਬਣਤਰ box *box,
		ਪੂਰਨ x_ਮਾਪਾ, ਪੂਰਨ y_ਮਾਪਾ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਦਸ਼ਮਲਵ scale,
		ਰੰਗ current_background_color,
		ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	ਅਚਲ ਬਣਤਰ plotter_ਸਾਰਣੀ *plot = ctx->plot;
	ਪੂਰਨ x, y;
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;
	ਪੂਰਨ padding_ਖੱਬਾ, padding_ਸਿਖਰ, padding_ਚੌੜਾਈ, padding_ਉਚਾਈ;
	ਪੂਰਨ border_ਖੱਬਾ, border_ਸਿਖਰ, border_ਸੱਜਾ, border_bottom;
	ਬਣਤਰ rect r;
	ਬਣਤਰ rect rect;
	ਪੂਰਨ x_scrolled, y_scrolled;
	ਬਣਤਰ box *bg_box = ਨਿਰਾ;
	css_computed_clip_rect css_rect;
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_x = CSS_ਉਛਾਲ_VISIBLE;
	ਗਿਣਤੀ css_ਉਛਾਲ_e ਉਛਾਲ_y = CSS_ਉਛਾਲ_VISIBLE;
	dom_exception exc;
	dom_html_element_type ਟੈਗ_type;


	ਜੇ (html_redraw_printing && (box->flags & PRINTED))
		ਵਾਪਸ true;

	ਜੇ (box->style != ਨਿਰਾ) {
		ਉਛਾਲ_x = css_computed_ਉਛਾਲ_x(box->style);
		ਉਛਾਲ_y = css_computed_ਉਛਾਲ_y(box->style);
	}

	/* avoid trivial FP maths */
	ਜੇ (scale == 1.0) {
		x = x_ਮਾਪਾ + box->x;
		y = y_ਮਾਪਾ + box->y;
		ਚੌੜਾਈ = box->ਚੌੜਾਈ;
		ਉਚਾਈ = box->ਉਚਾਈ;
		padding_ਖੱਬਾ = box->padding[ਖੱਬਾ_2];
		padding_ਸਿਖਰ = box->padding[ਸਿਖਰ_2];
		padding_ਚੌੜਾਈ = padding_ਖੱਬਾ + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
		padding_ਉਚਾਈ = padding_ਸਿਖਰ + box->ਉਚਾਈ +
				box->padding[BOTTOM];
		border_ਖੱਬਾ = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
		border_ਸਿਖਰ = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ;
		border_ਸੱਜਾ = box->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
		border_bottom = box->border[BOTTOM].ਚੌੜਾਈ;
	} ਨਹੀਂਤਾਂ {
		x = (x_ਮਾਪਾ + box->x) * scale;
		y = (y_ਮਾਪਾ + box->y) * scale;
		ਚੌੜਾਈ = box->ਚੌੜਾਈ * scale;
		ਉਚਾਈ = box->ਉਚਾਈ * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ਖੱਬਾ = box->padding[ਖੱਬਾ_2] ? box->padding[ਖੱਬਾ_2] * scale
				: 0;
		padding_ਸਿਖਰ = box->padding[ਸਿਖਰ_2] ? box->padding[ਸਿਖਰ_2] * scale
				: 0;
		padding_ਚੌੜਾਈ = (box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ +
				box->padding[ਸੱਜਾ_2]) * scale;
		padding_ਉਚਾਈ = (box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
				box->padding[BOTTOM]) * scale;
		border_ਖੱਬਾ = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ * scale;
		border_ਸਿਖਰ = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ * scale;
		border_ਸੱਜਾ = box->border[ਸੱਜਾ_2].ਚੌੜਾਈ * scale;
		border_bottom = box->border[BOTTOM].ਚੌੜਾਈ * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ਜੇ (box->style && ਉਛਾਲ_x != CSS_ਉਛਾਲ_VISIBLE &&
			box->ਮਾਪਾ != ਨਿਰਾ) {
		/* box contents clipped to box size */
		r.x0 = x - border_ਖੱਬਾ;
		r.x1 = x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ;
	} ਨਹੀਂਤਾਂ {
		/* box contents can hang out of the box; use descendant box */
		ਜੇ (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} ਨਹੀਂਤਾਂ {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ਜੇ (!box->ਮਾਪਾ) {
			/* root element */
			ਪੂਰਨ margin_ਖੱਬਾ, margin_ਸੱਜਾ;
			ਜੇ (scale == 1.0) {
				margin_ਖੱਬਾ = box->margin[ਖੱਬਾ_2];
				margin_ਸੱਜਾ = box->margin[ਸੱਜਾ_2];
			} ਨਹੀਂਤਾਂ {
				margin_ਖੱਬਾ = box->margin[ਖੱਬਾ_2] * scale;
				margin_ਸੱਜਾ = box->margin[ਸੱਜਾ_2] * scale;
			}
			r.x0 = x - border_ਖੱਬਾ - margin_ਖੱਬਾ < r.x0 ?
					x - border_ਖੱਬਾ - margin_ਖੱਬਾ : r.x0;
			r.x1 = x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ +
					margin_ਸੱਜਾ > r.x1 ?
					x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ +
					margin_ਸੱਜਾ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ਜੇ (box->style && ਉਛਾਲ_y != CSS_ਉਛਾਲ_VISIBLE &&
			box->ਮਾਪਾ != ਨਿਰਾ) {
		/* box contents clipped to box size */
		r.y0 = y - border_ਸਿਖਰ;
		r.y1 = y + padding_ਉਚਾਈ + border_bottom;
	} ਨਹੀਂਤਾਂ {
		/* box contents can hang out of the box; use descendant box */
		ਜੇ (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} ਨਹੀਂਤਾਂ {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ਜੇ (!box->ਮਾਪਾ) {
			/* root element */
			ਪੂਰਨ margin_ਸਿਖਰ, margin_bottom;
			ਜੇ (scale == 1.0) {
				margin_ਸਿਖਰ = box->margin[ਸਿਖਰ_2];
				margin_bottom = box->margin[BOTTOM];
			} ਨਹੀਂਤਾਂ {
				margin_ਸਿਖਰ = box->margin[ਸਿਖਰ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_ਸਿਖਰ - margin_ਸਿਖਰ < r.y0 ?
					y - border_ਸਿਖਰ - margin_ਸਿਖਰ : r.y0;
			r.y1 = y + padding_ਉਚਾਈ + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_ਉਚਾਈ + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ਜੇ (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		ਵਾਪਸ true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ਜੇ (html_redraw_printing) {
		ਜੇ (r.y1 > html_redraw_printing_border) {
			ਜੇ (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_ਪਾਠ ||
					box->type == BOX_ਸਾਰਣੀ_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ਜੇ (r.y0 < html_redraw_printing_ਸਿਖਰ_cropped)
					html_redraw_printing_ਸਿਖਰ_cropped = r.y0;
				ਵਾਪਸ true;
			}
		}
		ਨਹੀਂਤਾਂ box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ਜੇ (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ਜੇ ((ctx->plot->ਸਮੂਹ_ਸ਼ੁਰੂ) &&
		    (ctx->plot->ਸਮੂਹ_ਸ਼ੁਰੂ(ctx, "hidden box") != NSERROR_OK))
			ਵਾਪਸ false;
		ਜੇ (!html_redraw_box_children(html, box, x_ਮਾਪਾ, y_ਮਾਪਾ,
				&r, scale, current_background_color, ctx))
			ਵਾਪਸ false;
		ਵਾਪਸ ((!ctx->plot->ਸਮੂਹ_ਸਿਰਾ) || (ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
	}

	ਜੇ ((ctx->plot->ਸਮੂਹ_ਸ਼ੁਰੂ) &&
	    (ctx->plot->ਸਮੂਹ_ਸ਼ੁਰੂ(ctx,"vis box") != NSERROR_OK)) {
		ਵਾਪਸ false;
	}

	ਜੇ (box->style != ਨਿਰਾ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ਜੇ (css_rect.ਖੱਬਾ_auto == false)
			r.x0 = x - border_ਖੱਬਾ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ਖੱਬਾ, css_rect.lunit));

		ਜੇ (css_rect.ਸਿਖਰ_auto == false)
			r.y0 = y - border_ਸਿਖਰ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ਸਿਖਰ, css_rect.tunit));

		ਜੇ (css_rect.ਸੱਜਾ_auto == false)
			r.x1 = x - border_ਖੱਬਾ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ਸੱਜਾ, css_rect.runit));

		ਜੇ (css_rect.bottom_auto == false)
			r.y1 = y - border_ਸਿਖਰ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ਜੇ (r.x0 < clip->x0) r.x0 = clip->x0;
		ਜੇ (r.y0 < clip->y0) r.y0 = clip->y0;
		ਜੇ (clip->x1 < r.x1) r.x1 = clip->x1;
		ਜੇ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ਜੇ (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			ਵਾਪਸ ((!ctx->plot->ਸਮੂਹ_ਸਿਰਾ) ||
				(ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
		/* clip to it */
		ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ ਜੇ (box->type == BOX_ਖੰਡ || box->type == BOX_INLINE_ਖੰਡ ||
			box->type == BOX_ਸਾਰਣੀ_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ਜੇ (r.x0 < clip->x0) r.x0 = clip->x0;
		ਜੇ (r.y0 < clip->y0) r.y0 = clip->y0;
		ਜੇ (clip->x1 < r.x1) r.x1 = clip->x1;
		ਜੇ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ਜੇ (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			ਵਾਪਸ ((!ctx->plot->ਸਮੂਹ_ਸਿਰਾ) ||
				(ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
		/* clip to it */
		ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		/* clip box is fine, clip to it */
		r = *clip;
		ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ਵਾਪਸ false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_ਲੱਭੋ_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ਜੇ (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_ਪਾਠ &&
			bg_box->type != BOX_INLINE_ਸਿਰਾ &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ਨਿਰਾ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ਬਣਤਰ rect p;
		p.x0 = x - border_ਖੱਬਾ < r.x0 ? r.x0 : x - border_ਖੱਬਾ;
		p.y0 = y - border_ਸਿਖਰ < r.y0 ? r.y0 : y - border_ਸਿਖਰ;
		p.x1 = x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ < r.x1 ?
				x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ : r.x1;
		p.y1 = y + padding_ਉਚਾਈ + border_bottom < r.y1 ?
				y + padding_ਉਚਾਈ + border_bottom : r.y1;
		ਜੇ (!box->ਮਾਪਾ) {
			/* Root element, special case:
			 * background covers margins too */
			ਪੂਰਨ m_ਖੱਬਾ, m_ਸਿਖਰ, m_ਸੱਜਾ, m_bottom;
			ਜੇ (scale == 1.0) {
				m_ਖੱਬਾ = box->margin[ਖੱਬਾ_2];
				m_ਸਿਖਰ = box->margin[ਸਿਖਰ_2];
				m_ਸੱਜਾ = box->margin[ਸੱਜਾ_2];
				m_bottom = box->margin[BOTTOM];
			} ਨਹੀਂਤਾਂ {
				m_ਖੱਬਾ = box->margin[ਖੱਬਾ_2] * scale;
				m_ਸਿਖਰ = box->margin[ਸਿਖਰ_2] * scale;
				m_ਸੱਜਾ = box->margin[ਸੱਜਾ_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ਖੱਬਾ < r.x0 ? r.x0 : p.x0 - m_ਖੱਬਾ;
			p.y0 = p.y0 - m_ਸਿਖਰ < r.y0 ? r.y0 : p.y0 - m_ਸਿਖਰ;
			p.x1 = p.x1 + m_ਸੱਜਾ < r.x1 ? p.x1 + m_ਸੱਜਾ : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		ਜੇ ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ਜੇ (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				ਵਾਪਸ false;
			/* restore previous graphics window */
			ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ਵਾਪਸ false;
		}
	}

	/* borders for block level content and replaced inlines */
	ਜੇ (box->style &&
	    box->type != BOX_ਪਾਠ &&
	    box->type != BOX_INLINE_ਸਿਰਾ &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ਨਿਰਾ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_ਸਿਖਰ || border_ਸੱਜਾ || border_bottom || border_ਖੱਬਾ)) {
		ਜੇ (!html_redraw_borders(box, x_ਮਾਪਾ, y_ਮਾਪਾ,
				padding_ਚੌੜਾਈ, padding_ਉਚਾਈ, &r,
				scale, ctx))
			ਵਾਪਸ false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ਜੇ (box->style && box->type == BOX_INLINE && box->inline_ਸਿਰਾ &&
			(html_redraw_box_has_background(box) ||
			border_ਸਿਖਰ || border_ਸੱਜਾ ||
			border_bottom || border_ਖੱਬਾ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ਬਣਤਰ box *ib;
		ਬਣਤਰ rect b; /* border edge rectangle */
		ਬਣਤਰ rect p; /* clipped rect */
		bool ਪਹਿਲਾ = true;
		ਪੂਰਨ ib_x;
		ਪੂਰਨ ib_y = y;
		ਪੂਰਨ ib_p_ਚੌੜਾਈ;
		ਪੂਰਨ ib_b_ਖੱਬਾ, ib_b_ਸੱਜਾ;

		b.x0 = x - border_ਖੱਬਾ;
		b.x1 = x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ;
		b.y0 = y - border_ਸਿਖਰ;
		b.y1 = y + padding_ਉਚਾਈ + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		ਲਈ (ib = box; ib; ib = ib->ਅਗਲਾ) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ਜੇ (ib->type == BOX_FLOAT_ਖੱਬਾ ||
					ib->type == BOX_FLOAT_ਸੱਜਾ)
				ਜਾਰੀ;
			ਜੇ (scale == 1.0) {
				ib_x = x_ਮਾਪਾ + ib->x;
				ib_y = y_ਮਾਪਾ + ib->y;
				ib_p_ਚੌੜਾਈ = ib->padding[ਖੱਬਾ_2] + ib->ਚੌੜਾਈ +
						ib->padding[ਸੱਜਾ_2];
				ib_b_ਖੱਬਾ = ib->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
				ib_b_ਸੱਜਾ = ib->border[ਸੱਜਾ_2].ਚੌੜਾਈ;
			} ਨਹੀਂਤਾਂ {
				ib_x = (x_ਮਾਪਾ + ib->x) * scale;
				ib_y = (y_ਮਾਪਾ + ib->y) * scale;
				ib_p_ਚੌੜਾਈ = (ib->padding[ਖੱਬਾ_2] + ib->ਚੌੜਾਈ +
						ib->padding[ਸੱਜਾ_2]) * scale;
				ib_b_ਖੱਬਾ = ib->border[ਖੱਬਾ_2].ਚੌੜਾਈ * scale;
				ib_b_ਸੱਜਾ = ib->border[ਸੱਜਾ_2].ਚੌੜਾਈ * scale;
			}

			ਜੇ ((ib->flags & NEW_ਸਤਰ) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ਜੇ (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						ਪਹਿਲਾ, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					ਵਾਪਸ false;
				/* restore previous graphics window */
				ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					ਵਾਪਸ false;
				ਜੇ (!html_redraw_inline_borders(box, b, &r,
						scale, ਪਹਿਲਾ, false, ctx))
					ਵਾਪਸ false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ਖੱਬਾ;
				b.y0 = ib_y - border_ਸਿਖਰ - padding_ਸਿਖਰ;
				b.y1 = ib_y + padding_ਉਚਾਈ - padding_ਸਿਖਰ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				ਪਹਿਲਾ = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_ਚੌੜਾਈ + ib_b_ਸੱਜਾ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ਜੇ (ib == box->inline_ਸਿਰਾ)
				/* reached end of BOX_INLINE span */
				ਰੋਕੋ;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ਜੇ (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				ਪਹਿਲਾ, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			ਵਾਪਸ false;
		/* restore previous graphics window */
		ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ਵਾਪਸ false;
		ਜੇ (!html_redraw_inline_borders(box, b, &r, scale, ਪਹਿਲਾ, true,
				ctx))
			ਵਾਪਸ false;

	}

	/* Debug outlines */
	ਜੇ (html_redraw_debug) {
		ਪੂਰਨ margin_ਖੱਬਾ, margin_ਸੱਜਾ;
		ਪੂਰਨ margin_ਸਿਖਰ, margin_bottom;
		ਜੇ (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ਖੱਬਾ = box->margin[ਖੱਬਾ_2];
			margin_ਸਿਖਰ = box->margin[ਸਿਖਰ_2];
			margin_ਸੱਜਾ = box->margin[ਸੱਜਾ_2];
			margin_bottom = box->margin[BOTTOM];
		} ਨਹੀਂਤਾਂ {
			margin_ਖੱਬਾ = box->margin[ਖੱਬਾ_2] * scale;
			margin_ਸਿਖਰ = box->margin[ਸਿਖਰ_2] * scale;
			margin_ਸੱਜਾ = box->margin[ਸੱਜਾ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ਖੱਬਾ;
		rect.y0 = y + padding_ਸਿਖਰ;
		rect.x1 = x + padding_ਖੱਬਾ + ਚੌੜਾਈ;
		rect.y1 = y + padding_ਸਿਖਰ + ਉਚਾਈ;
		ਜੇ (ctx->plot->rectangle(ctx, plot_style_content_ਕਿਨਾਰਾ, &rect) != NSERROR_OK)
			ਵਾਪਸ false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_ਚੌੜਾਈ;
		rect.y1 = y + padding_ਉਚਾਈ;
		ਜੇ (ctx->plot->rectangle(ctx, plot_style_padding_ਕਿਨਾਰਾ, &rect) != NSERROR_OK)
			ਵਾਪਸ false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ਖੱਬਾ - margin_ਖੱਬਾ;
		rect.y0 = y - border_ਸਿਖਰ - margin_ਸਿਖਰ;
		rect.x1 = x + padding_ਚੌੜਾਈ + border_ਸੱਜਾ + margin_ਸੱਜਾ;
		rect.y1 = y + padding_ਉਚਾਈ + border_bottom + margin_bottom;
		ਜੇ (ctx->plot->rectangle(ctx, plot_style_margin_ਕਿਨਾਰਾ, &rect) != NSERROR_OK)
			ਵਾਪਸ false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ਜੇ (box->ਮਾਪਾ != ਨਿਰਾ) {
		bool need_clip = false;
		ਜੇ (box->object || box->flags & IFRAME ||
				(ਉਛਾਲ_x != CSS_ਉਛਾਲ_VISIBLE &&
				 ਉਛਾਲ_y != CSS_ਉਛਾਲ_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_ਚੌੜਾਈ;
			r.y1 = y + padding_ਉਚਾਈ;
			ਜੇ (r.x0 < clip->x0) r.x0 = clip->x0;
			ਜੇ (r.y0 < clip->y0) r.y0 = clip->y0;
			ਜੇ (clip->x1 < r.x1) r.x1 = clip->x1;
			ਜੇ (clip->y1 < r.y1) r.y1 = clip->y1;
			ਜੇ (r.x1 <= r.x0 || r.y1 <= r.y0) {
				ਵਾਪਸ (!ctx->plot->ਸਮੂਹ_ਸਿਰਾ ||
					(ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ਨਹੀਂਤਾਂ ਜੇ (ਉਛਾਲ_x != CSS_ਉਛਾਲ_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_ਚੌੜਾਈ;
			r.y1 = clip->y1;
			ਜੇ (r.x0 < clip->x0) r.x0 = clip->x0;
			ਜੇ (clip->x1 < r.x1) r.x1 = clip->x1;
			ਜੇ (r.x1 <= r.x0) {
				ਵਾਪਸ (!ctx->plot->ਸਮੂਹ_ਸਿਰਾ ||
					(ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ਨਹੀਂਤਾਂ ਜੇ (ਉਛਾਲ_y != CSS_ਉਛਾਲ_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_ਉਚਾਈ;
			ਜੇ (r.y0 < clip->y0) r.y0 = clip->y0;
			ਜੇ (clip->y1 < r.y1) r.y1 = clip->y1;
			ਜੇ (r.y1 <= r.y0) {
				ਵਾਪਸ (!ctx->plot->ਸਮੂਹ_ਸਿਰਾ ||
					(ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ਜੇ (need_clip &&
		    (box->type == BOX_ਖੰਡ ||
		     box->type == BOX_INLINE_ਖੰਡ ||
		     box->type == BOX_ਸਾਰਣੀ_CELL || box->object)) {
			ਜੇ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ਵਾਪਸ false;
		}
	}

	/* text decoration */
	ਜੇ ((box->type != BOX_ਪਾਠ) &&
	    box->style &&
	    css_computed_ਪਾਠ_decoration(box->style) !=	CSS_ਪਾਠ_DECORATION_NONE) {
		ਜੇ (!html_redraw_ਪਾਠ_decoration(box, x_ਮਾਪਾ, y_ਮਾਪਾ,
				scale, current_background_color, ctx))
			ਵਾਪਸ false;
	}

	ਜੇ (box->ਗੰਢ != ਨਿਰਾ) {
		exc = dom_html_element_get_ਟੈਗ_type(box->ਗੰਢ, &ਟੈਗ_type);
		ਜੇ (exc != DOM_NO_ERR) {
			ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} ਨਹੀਂਤਾਂ {
		ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ਜੇ (box->object && ਚੌੜਾਈ != 0 && ਉਚਾਈ != 0) {
		ਬਣਤਰ content_redraw_ਅੰਕੜੇ obj_ਅੰਕੜੇ;

		x_scrolled = x - scrollbar_get_ਵਿੱਥ(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_ਵਿੱਥ(box->scroll_y) * scale;

		obj_ਅੰਕੜੇ.x = x_scrolled + padding_ਖੱਬਾ;
		obj_ਅੰਕੜੇ.y = y_scrolled + padding_ਸਿਖਰ;
		obj_ਅੰਕੜੇ.ਚੌੜਾਈ = ਚੌੜਾਈ;
		obj_ਅੰਕੜੇ.ਉਚਾਈ = ਉਚਾਈ;
		obj_ਅੰਕੜੇ.background_ਰੰਗ = current_background_color;
		obj_ਅੰਕੜੇ.scale = scale;
		obj_ਅੰਕੜੇ.repeat_x = false;
		obj_ਅੰਕੜੇ.repeat_y = false;

		ਜੇ (content_get_type(box->object) == CONTENT_HTML) {
			obj_ਅੰਕੜੇ.x /= scale;
			obj_ਅੰਕੜੇ.y /= scale;
		}

		ਜੇ (!content_redraw(box->object, &obj_ਅੰਕੜੇ, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ਅਚਲ ਅੱਖਰ *obj = "\xef\xbf\xbc";
			ਪੂਰਨ obj_ਚੌੜਾਈ;
			ਪੂਰਨ obj_x = x + padding_ਖੱਬਾ;
			nserror res;

			rect.x0 = x + padding_ਖੱਬਾ;
			rect.y0 = y + padding_ਸਿਖਰ;
			rect.x1 = x + padding_ਖੱਬਾ + ਚੌੜਾਈ - 1;
			rect.y1 = y + padding_ਸਿਖਰ + ਉਚਾਈ - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ਜੇ (res != NSERROR_OK) {
				ਵਾਪਸ false;
			}

			res = guit->layout->ਚੌੜਾਈ(plot_fstyle_broken_object,
						  obj,
						  ਆਕਾਰ(obj) - 1,
						  &obj_ਚੌੜਾਈ);
			ਜੇ (res != NSERROR_OK) {
				obj_x += 1;
			} ਨਹੀਂਤਾਂ {
				obj_x += ਚੌੜਾਈ / 2 - obj_ਚੌੜਾਈ / 2;
			}

			ਜੇ (ctx->plot->ਪਾਠ(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_ਸਿਖਰ + (ਪੂਰਨ)(ਉਚਾਈ * 0.75),
					    obj, ਆਕਾਰ(obj) - 1) != NSERROR_OK)
				ਵਾਪਸ false;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (ਟੈਗ_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ਗੰਢ != ਨਿਰਾ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ਬਣਤਰ bitmap *bitmap = ਨਿਰਾ;
		exc = dom_ਗੰਢ_get_user_ਅੰਕੜੇ(box->ਗੰਢ,
					     corestring_dom_ns_ਕੁੰਜੀ_canvas_ਗੰਢ_ਅੰਕੜੇ,
					     &bitmap);
		ਜੇ (exc != DOM_NO_ERR) {
			bitmap = ਨਿਰਾ;
		}
		ਜੇ (bitmap != ਨਿਰਾ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ਖੱਬਾ, y + padding_ਸਿਖਰ,
				      ਚੌੜਾਈ, ਉਚਾਈ, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ ਜੇ (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_ਖਿੜਕੀ_redraw(box->iframe,
				x + padding_ਖੱਬਾ,
				y + padding_ਸਿਖਰ, &r, ctx);

	} ਨਹੀਂਤਾਂ ਜੇ (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ਜੇ (!html_redraw_checkbox(x + padding_ਖੱਬਾ, y + padding_ਸਿਖਰ,
				ਚੌੜਾਈ, ਉਚਾਈ, box->gadget->selected, ctx))
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ ਜੇ (box->gadget && box->gadget->type == GADGET_RADIO) {
		ਜੇ (!html_redraw_radio(x + padding_ਖੱਬਾ, y + padding_ਸਿਖਰ,
				ਚੌੜਾਈ, ਉਚਾਈ, box->gadget->selected, ctx))
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ ਜੇ (box->gadget && box->gadget->type == GADGET_FILE) {
		ਜੇ (!html_redraw_file(x + padding_ਖੱਬਾ, y + padding_ਸਿਖਰ,
				ਚੌੜਾਈ, ਉਚਾਈ, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ ਜੇ (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->ਅੰਕੜੇ.ਪਾਠ.ta, x, y,
				current_background_color, scale, &r, ctx);

	} ਨਹੀਂਤਾਂ ਜੇ (box->ਪਾਠ) {
		ਜੇ (!html_redraw_ਪਾਠ_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			ਵਾਪਸ false;

	} ਨਹੀਂਤਾਂ {
		ਜੇ (!html_redraw_box_children(html, box, x_ਮਾਪਾ, y_ਮਾਪਾ, &r,
				scale, current_background_color, ctx))
			ਵਾਪਸ false;
	}

	ਜੇ (box->type == BOX_ਖੰਡ || box->type == BOX_INLINE_ਖੰਡ ||
			box->type == BOX_ਸਾਰਣੀ_CELL || box->type == BOX_INLINE)
		ਜੇ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ਵਾਪਸ false;

	/* list marker */
	ਜੇ (box->list_marker) {
		ਜੇ (!html_redraw_box(html, box->list_marker,
				x_ਮਾਪਾ + box->x -
				scrollbar_get_ਵਿੱਥ(box->scroll_x),
				y_ਮਾਪਾ + box->y -
				scrollbar_get_ਵਿੱਥ(box->scroll_y),
				clip, scale, current_background_color, ctx))
			ਵਾਪਸ false;
	}

	/* scrollbars */
	ਜੇ (((box->style && box->type != BOX_BR &&
	      box->type != BOX_ਸਾਰਣੀ && box->type != BOX_INLINE &&
	      (box->gadget == ਨਿਰਾ || box->gadget->type != GADGET_TEXTAREA) &&
	      (ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL ||
	       ਉਛਾਲ_x == CSS_ਉਛਾਲ_AUTO ||
	       ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL ||
	       ਉਛਾਲ_y == CSS_ਉਛਾਲ_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->ਮਾਪਾ != ਨਿਰਾ) {
		nserror res;
		bool has_x_scroll = (ਉਛਾਲ_x == CSS_ਉਛਾਲ_SCROLL);
		bool has_y_scroll = (ਉਛਾਲ_y == CSS_ਉਛਾਲ_SCROLL);

		has_x_scroll |= (ਉਛਾਲ_x == CSS_ਉਛਾਲ_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (ਉਛਾਲ_y == CSS_ਉਛਾਲ_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ਬਣਤਰ content *)html,
					    box, has_x_scroll, has_y_scroll);
		ਜੇ (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			ਵਾਪਸ false;
		}

		ਜੇ (box->scroll_x != ਨਿਰਾ)
			scrollbar_redraw(box->scroll_x,
					x_ਮਾਪਾ + box->x,
					y_ਮਾਪਾ + box->y + box->padding[ਸਿਖਰ_2] +
					box->ਉਚਾਈ + box->padding[BOTTOM] -
					SCROLLBAR_ਚੌੜਾਈ, clip, scale, ctx);
		ਜੇ (box->scroll_y != ਨਿਰਾ)
			scrollbar_redraw(box->scroll_y,
					x_ਮਾਪਾ + box->x + box->padding[ਖੱਬਾ_2] +
					box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2] -
					SCROLLBAR_ਚੌੜਾਈ,
					y_ਮਾਪਾ + box->y, clip, scale, ctx);
	}

	ਜੇ (box->type == BOX_ਖੰਡ || box->type == BOX_INLINE_ਖੰਡ ||
	    box->type == BOX_ਸਾਰਣੀ_CELL || box->type == BOX_INLINE) {
		ਜੇ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ਵਾਪਸ false;
	}

	ਵਾਪਸ ((!plot->ਸਮੂਹ_ਸਿਰਾ) || (ctx->plot->ਸਮੂਹ_ਸਿਰਾ(ctx) == NSERROR_OK));
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

bool html_redraw(ਬਣਤਰ content *c, ਬਣਤਰ content_redraw_ਅੰਕੜੇ *ਅੰਕੜੇ,
		ਅਚਲ ਬਣਤਰ rect *clip, ਅਚਲ ਬਣਤਰ redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ਬਣਤਰ box *box;
	bool ਨਤੀਜਾ = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ਰੰਗ = ਅੰਕੜੇ->background_ਰੰਗ,
	};

	box = html->layout;
	ਪ੍ਰਤਿਜ੍ਞਾ(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ਜੇ (ctx->interactive && html->visible_select_menu != ਨਿਰਾ) {
		ਬਣਤਰ form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				ਅੰਕੜੇ->scale, clip);
	}

	ਜੇ (!select_only) {
		/* clear to background colour */
		ਨਤੀਜਾ = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ਜੇ (html->background_ਰੰਗ != NS_TRANSPARENT)
			pstyle_fill_bg.fill_ਰੰਗ = html->background_ਰੰਗ;

		ਨਤੀਜਾ &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		ਨਤੀਜਾ &= html_redraw_box(html, box, ਅੰਕੜੇ->x, ਅੰਕੜੇ->y, clip,
				ਅੰਕੜੇ->scale, pstyle_fill_bg.fill_ਰੰਗ, ctx);
	}

	ਜੇ (select) {
		ਪੂਰਨ menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ਖੱਬਾ_2].ਚੌੜਾਈ;
		menu_y += box->ਉਚਾਈ + box->border[BOTTOM].ਚੌੜਾਈ +
				box->padding[BOTTOM] + box->padding[ਸਿਖਰ_2];
		ਨਤੀਜਾ &= form_redraw_select_menu(html->visible_select_menu,
				ਅੰਕੜੇ->x + menu_x, ਅੰਕੜੇ->y + menu_y,
				ਅੰਕੜੇ->scale, clip, ctx);
	}

	ਵਾਪਸ ਨਤੀਜਾ;

}
