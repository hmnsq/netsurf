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

#ಅನ್ತರ್ಭಾವ "utils/config.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/browser_window.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plotters.h"
#ಅನ್ತರ್ಭಾವ "netsurf/bitmap.h"
#ಅನ್ತರ್ಭಾವ "netsurf/layout.h"
#ಅನ್ತರ್ಭಾವ "content/content.h"
#ಅನ್ತರ್ಭಾವ "content/content_protected.h"
#ಅನ್ತರ್ಭಾವ "content/textsearch.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "desktop/selection.h"
#ಅನ್ತರ್ಭಾವ "desktop/print.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"
#ಅನ್ತರ್ಭಾವ "desktop/textarea.h"
#ಅನ್ತರ್ಭಾವ "desktop/gui_internal.h"

#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"
#ಅನ್ತರ್ಭಾವ "html/font.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ಸ್ಥಿರ bool html_redraw_box_has_background(ರಚನೆ box *box)
{
	ಆದರೆ (box->background != ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (box->style != ಖಾಲಿ) {
		css_color ಬಣ್ಣ;

		css_computed_background_color(box->style, &ಬಣ್ಣ);

		ಆದರೆ (nscss_color_is_transparent(ಬಣ್ಣ) == false)
			ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ಸ್ಥಿರ ರಚನೆ box *html_redraw_ಕಂಡುಹಿಡಿ_bg_box(ರಚನೆ box *box)
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
	ಆದರೆ (box->ಪೋಷಕ == ಖಾಲಿ) {
		/* Root box */
		ಆದರೆ (html_redraw_box_has_background(box))
			ಹಿಂತಿರುಗಿ box;

		/* No background on root box: consider body box, if any */
		ಆದರೆ (box->children != ಖಾಲಿ) {
			ಆದರೆ (html_redraw_box_has_background(box->children))
				ಹಿಂತಿರುಗಿ box->children;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪೋಷಕ != ಖಾಲಿ && box->ಪೋಷಕ->ಪೋಷಕ == ಖಾಲಿ) {
		/* Body box: only render background if root has its own */
		ಆದರೆ (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->ಪೋಷಕ))
			ಹಿಂತಿರುಗಿ box;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Any other box */
		ಆದರೆ (html_redraw_box_has_background(box))
			ಹಿಂತಿರುಗಿ box;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
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

ಸ್ಥಿರ bool
ಪಠ್ಯ_redraw(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *utf8_ಪಠ್ಯ,
	    size_t utf8_len,
	    size_t ಸ್ಥಾನಾಂತರ,
	    ಪೂರ್ಣಾಂಕ space,
	    ಸ್ಥಿರಾಂಕ plot_font_style_t *fstyle,
	    ಪೂರ್ಣಾಂಕ x,
	    ಪೂರ್ಣಾಂಕ y,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip,
	    ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
	    ದಶಮಾಂಶ scale,
	    bool excluded,
	    ರಚನೆ content *c,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ selection *sel,
	    ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.ಗಾತ್ರ_2 *= scale;

	/* is this box part of a selection? */
	ಆದರೆ (!excluded && ctx->interactive == true) {
		ಚಿಹ್ನೆರಹಿತ len = utf8_len + (space ? 1 : 0);
		ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_idx;
		ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_idx;

		/* first try the browser window's current selection */
		ಆದರೆ (selection_highlighted(sel,
					  ಸ್ಥಾನಾಂತರ,
					  ಸ್ಥಾನಾಂತರ + len,
					  &ಆರಂಭ_idx,
					  &ಕೊನೆ_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ಆದರೆ (!highlighted &&
		    (c->textsearch.context != ಖಾಲಿ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     ಸ್ಥಾನಾಂತರ,
						     ಸ್ಥಾನಾಂತರ + len,
						     &ಆರಂಭ_idx,
						     &ಕೊನೆ_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ಆದರೆ (highlighted) {
			ರಚನೆ rect r;
			ಚಿಹ್ನೆರಹಿತ endtxt_idx = ಕೊನೆ_idx;
			bool clip_ಬದಲಾಯಿತು = false;
			bool ಪಠ್ಯ_visible = true;
			ಪೂರ್ಣಾಂಕ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ಆದರೆ (ಕೊನೆ_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ಪ್ರತಿಜ್ಞಾ(ಕೊನೆ_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->ಅಗಲ(fstyle,
						  utf8_ಪಠ್ಯ, ಆರಂಭ_idx,
						  &startx);
			ಆದರೆ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->ಅಗಲ(fstyle,
						  utf8_ಪಠ್ಯ, endtxt_idx,
						  &endx);
			ಆದರೆ (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ಆದರೆ (ಕೊನೆ_idx > utf8_len) {
					endx += space;
			}

			ಆದರೆ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ಆದರೆ ((ಆರಂಭ_idx > 0) &&
			    (ctx->plot->ಪಠ್ಯ(ctx,
					     &plot_fstyle,
					     x,
					     y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
					     utf8_ಪಠ್ಯ,
					     ಆರಂಭ_idx) != NSERROR_OK))
				ಹಿಂತಿರುಗಿ false;

			pstyle_fill_hback.fill_ಬಣ್ಣ = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ಎತ್ತರ * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (ಆರಂಭ_idx > 0) {
				ಪೂರ್ಣಾಂಕ px0 = max(x + startx, clip->x0);
				ಪೂರ್ಣಾಂಕ px1 = min(x + endx, clip->x1);

				ಆದರೆ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}

					clip_ಬದಲಾಯಿತು = true;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಪಠ್ಯ_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_ಬಣ್ಣ;
			fstyle_hback.foreground = ಬಣ್ಣ_ಗಮ್ಯ_bw_furthest(
				pstyle_fill_hback.fill_ಬಣ್ಣ);

			ಆದರೆ (ಪಠ್ಯ_visible &&
			    (ctx->plot->ಪಠ್ಯ(ctx,
					     &fstyle_hback,
					     x,
					     y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
					     utf8_ಪಠ್ಯ,
					     endtxt_idx) != NSERROR_OK)) {
				ಹಿಂತಿರುಗಿ false;
			}

			/* draw any text succeeding highlighted portion */
			ಆದರೆ (endtxt_idx < utf8_len) {
				ಪೂರ್ಣಾಂಕ px0 = max(x + endx, clip->x0);
				ಆದರೆ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}

					clip_ಬದಲಾಯಿತು = true;

					res = ctx->plot->ಪಠ್ಯ(ctx,
							      &plot_fstyle,
							      x,
							      y + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75 * scale),
							      utf8_ಪಠ್ಯ,
							      utf8_len);
					ಆದರೆ (res != NSERROR_OK) {
						ಹಿಂತಿರುಗಿ false;
					}
				}
			}

			ಆದರೆ (clip_ಬದಲಾಯಿತು &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}

	ಆದರೆ (!highlighted) {
		res = ctx->plot->ಪಠ್ಯ(ctx,
				      &plot_fstyle,
				      x,
				      y + (ಪೂರ್ಣಾಂಕ) (ಎತ್ತರ * 0.75 * scale),
				      utf8_ಪಠ್ಯ,
				      utf8_len);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}
	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_checkbox(ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
		bool selected, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ದ್ವಿಗುಣ z;
	nserror res;
	ರಚನೆ rect rect;

	z = ಅಗಲ * 0.15;
	ಆದರೆ (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + ಅಗಲ;
	rect.y1 = y + ಎತ್ತರ;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_darkwbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + ಎತ್ತರ;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_darkwbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* light line across right */
	rect.x0 = x + ಅಗಲ;
	rect.x1 = x + ಅಗಲ;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_lightwbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + ಎತ್ತರ;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_lightwbasec, &rect);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (selected) {
		ಆದರೆ (ಅಗಲ < 12 || ಎತ್ತರ < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + ಅಗಲ - z;
			rect.y1 = y + ಎತ್ತರ - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + ಅಗಲ - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + ಎತ್ತರ - z;
			res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_wblobc, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + ಎತ್ತರ - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (ಎತ್ತರ / 2);
			res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_wblobc, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}
	ಹಿಂತಿರುಗಿ true;
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
ಸ್ಥಿರ bool html_redraw_radio(ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
		bool selected, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + ಅಗಲ * 0.5,
			      y + ಎತ್ತರ * 0.5,
			      ಅಗಲ * 0.5 - 1);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + ಅಗಲ * 0.5,
			     y + ಎತ್ತರ * 0.5,
			     ಅಗಲ * 0.5 - 1,
			     45,
			     225);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + ಅಗಲ * 0.5,
			     y + ಎತ್ತರ * 0.5,
			     ಅಗಲ * 0.5 - 1,
			     225,
			     45);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + ಅಗಲ * 0.5,
				      y + ಎತ್ತರ * 0.5,
				      ಅಗಲ * 0.3 - 1);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_file(ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಪೂರ್ಣಾಂಕ ಅಗಲ, ಪೂರ್ಣಾಂಕ ಎತ್ತರ,
		ರಚನೆ box *box, ದಶಮಾಂಶ scale, ಬಣ್ಣ background_ಬಣ್ಣ,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಪೂರ್ಣಾಂಕ ಪಠ್ಯ_ಅಗಲ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪಠ್ಯ;
	size_t ಉದ್ದಳತೆ;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_ಬಣ್ಣ;

	ಆದರೆ (box->gadget->ಮೌಲ್ಯ) {
		ಪಠ್ಯ = box->gadget->ಮೌಲ್ಯ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪಠ್ಯ = messages_get("Form_Drop");
	}
	ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪಠ್ಯ);

	res = guit->layout->ಅಗಲ(&fstyle, ಪಠ್ಯ, ಉದ್ದಳತೆ, &ಪಠ್ಯ_ಅಗಲ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}
	ಪಠ್ಯ_ಅಗಲ *= scale;
	ಆದರೆ (ಅಗಲ < ಪಠ್ಯ_ಅಗಲ + 8) {
		x = x + ಅಗಲ - ಪಠ್ಯ_ಅಗಲ - 4;
	} ಇಲ್ಲದಿದ್ದರೆ {
		x = x + 4;
	}

	res = ctx->plot->ಪಠ್ಯ(ctx, &fstyle, x, y + ಎತ್ತರ * 0.75, ಪಠ್ಯ, ಉದ್ದಳತೆ);
	ಆದರೆ (res != NSERROR_OK) {
		ಹಿಂತಿರುಗಿ false;
	}
	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_background(ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ರಚನೆ box *box, ದಶಮಾಂಶ scale,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಬಣ್ಣ *background_ಬಣ್ಣ,
		ರಚನೆ box *background,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_ಬಣ್ಣ = true;
	bool plot_content;
	bool clip_ಗಮ್ಯ_children = false;
	ರಚನೆ box *clip_box = box;
	ಪೂರ್ಣಾಂಕ ox = x, oy = y;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ರಚನೆ box *ಪೋಷಕ;
	ರಚನೆ rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ಬಣ್ಣ = *background_ಬಣ್ಣ,
	};
	nserror res;

	ಆದರೆ (ctx->background_images == false)
		ಹಿಂತಿರುಗಿ true;

	plot_content = (background->background != ಖಾಲಿ);

	ಆದರೆ (plot_content) {
		ಆದರೆ (!box->ಪೋಷಕ) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ಎಡ_2] * scale;
			y -= box->margin[ತುದಿ_2] * scale;
			ಅಗಲ = box->margin[ಎಡ_2] + box->padding[ಎಡ_2] +
					box->ಅಗಲ + box->padding[ಬಲ_2] +
					box->margin[ಬಲ_2];
			ಎತ್ತರ = box->margin[ತುದಿ_2] + box->padding[ತುದಿ_2] +
					box->ಎತ್ತರ + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಗಲ = box->padding[ಎಡ_2] + box->ಅಗಲ +
					box->padding[ಬಲ_2];
			ಎತ್ತರ = box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		ಆಯ್ಕೆ (css_computed_background_repeat(background->style)) {
		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_ಬಣ್ಣ = !content_get_opaque(background->background);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ಆದರೆ (hunit == CSS_UNIT_PCT) {
			x += (ಅಗಲ -
				content_get_ಅಗಲ(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} ಇಲ್ಲದಿದ್ದರೆ {
			x += (ಪೂರ್ಣಾಂಕ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ಆದರೆ (vunit == CSS_UNIT_PCT) {
			y += (ಎತ್ತರ -
				content_get_ಎತ್ತರ(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} ಇಲ್ಲದಿದ್ದರೆ {
			y += (ಪೂರ್ಣಾಂಕ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ಆದರೆ (box->type == BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		ಗಾಗಿ (ಪೋಷಕ = box->ಪೋಷಕ;
			((ಪೋಷಕ) && (ಪೋಷಕ->type != BOX_ಕೋಷ್ಟಕ));
				ಪೋಷಕ = ಪೋಷಕ->ಪೋಷಕ);
		ಪ್ರತಿಜ್ಞಾ(ಪೋಷಕ && (ಪೋಷಕ->style));

		css_computed_border_spacing(ಪೋಷಕ->style, &h, &hu, &v, &vu);

		clip_ಗಮ್ಯ_children = (h > 0) || (v > 0);

		ಆದರೆ (clip_ಗಮ್ಯ_children)
			clip_box = box->children;
	}

	ಗಾಗಿ (; clip_box; clip_box = clip_box->ಮುಂದಿನ) {
		/* clip to child boxes if needed */
		ಆದರೆ (clip_ಗಮ್ಯ_children) {
			ಪ್ರತಿಜ್ಞಾ(clip_box->type == BOX_ಕೋಷ್ಟಕ_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ಎಡ_2] +
					clip_box->ಅಗಲ +
					clip_box->padding[ಬಲ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[ತುದಿ_2] +
					clip_box->ಎತ್ತರ +
					clip_box->padding[BOTTOM]) * scale;

			ಆದರೆ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಆದರೆ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಆದರೆ (r.x1 > clip->x1) r.x1 = clip->x1;
			ಆದರೆ (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ಆದರೆ ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ಖಾಲಿ) &&
			     content_get_opaque(clip_box->background)))
				ಮುಂದುವರಿ;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ಆದರೆ (nscss_color_is_transparent(bgcol) == false) {
			*background_ಬಣ್ಣ = nscss_color_ಗಮ್ಯ_ns(bgcol);
			pstyle_fill_bg.fill_ಬಣ್ಣ = *background_ಬಣ್ಣ;
			ಆದರೆ (plot_ಬಣ್ಣ) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ಆದರೆ (res != NSERROR_OK) {
					ಹಿಂತಿರುಗಿ false;
				}
			}
		}
		/* and plot the image */
		ಆದರೆ (plot_content) {
			ಅಗಲ = content_get_ಅಗಲ(background->background);
			ಎತ್ತರ = content_get_ಎತ್ತರ(background->background);

			/* ensure clip area only as large as required */
			ಆದರೆ (!repeat_x) {
				ಆದರೆ (r.x0 < x)
					r.x0 = x;
				ಆದರೆ (r.x1 > x + ಅಗಲ * scale)
					r.x1 = x + ಅಗಲ * scale;
			}
			ಆದರೆ (!repeat_y) {
				ಆದರೆ (r.y0 < y)
					r.y0 = y;
				ಆದರೆ (r.y1 > y + ಎತ್ತರ * scale)
					r.y1 = y + ಎತ್ತರ * scale;
			}
			/* valid clipping rectangles only */
			ಆದರೆ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ರಚನೆ content_redraw_ದತ್ತಾಂಶ bg_ದತ್ತಾಂಶ;

				res = ctx->plot->clip(ctx, &r);
				ಆದರೆ (res != NSERROR_OK) {
					ಹಿಂತಿರುಗಿ false;
				}

				bg_ದತ್ತಾಂಶ.x = x;
				bg_ದತ್ತಾಂಶ.y = y;
				bg_ದತ್ತಾಂಶ.ಅಗಲ = ceilf(ಅಗಲ * scale);
				bg_ದತ್ತಾಂಶ.ಎತ್ತರ = ceilf(ಎತ್ತರ * scale);
				bg_ದತ್ತಾಂಶ.background_ಬಣ್ಣ = *background_ಬಣ್ಣ;
				bg_ದತ್ತಾಂಶ.scale = scale;
				bg_ದತ್ತಾಂಶ.repeat_x = repeat_x;
				bg_ದತ್ತಾಂಶ.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_ದತ್ತಾಂಶ, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		ಆದರೆ (!clip_ಗಮ್ಯ_children)
			ಹಿಂತಿರುಗಿ true;
	}
	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_inline_background(ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ರಚನೆ box *box,
		ದಶಮಾಂಶ scale, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ರಚನೆ rect b,
		bool ಮೊದಲ, bool ಕೊನೆಯ, ಬಣ್ಣ *background_ಬಣ್ಣ,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ rect r = *clip;
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_ಬಣ್ಣ = true;
	bool plot_content;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ಬಣ್ಣ = *background_ಬಣ್ಣ,
	};
	nserror res;

	plot_content = (box->background != ಖಾಲಿ);

	ಆದರೆ (html_redraw_printing && nsoption_bool(remove_backgrounds))
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (plot_content) {
		/* handle background-repeat */
		ಆಯ್ಕೆ (css_computed_background_repeat(box->style)) {
		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_ಬಣ್ಣ = !content_get_opaque(box->background);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
			ನಿಲ್ಲಿಸು;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ಆದರೆ (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_ಅಗಲ(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ಆದರೆ (!repeat_x && ((hpos < 2 && !ಮೊದಲ) ||
					(hpos > 98 && !ಕೊನೆಯ))){
				plot_content = false;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			x += (ಪೂರ್ಣಾಂಕ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ಆದರೆ (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_ಎತ್ತರ(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} ಇಲ್ಲದಿದ್ದರೆ {
			y += (ಪೂರ್ಣಾಂಕ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ಆದರೆ (nscss_color_is_transparent(bgcol) == false) {
		*background_ಬಣ್ಣ = nscss_color_ಗಮ್ಯ_ns(bgcol);
		pstyle_fill_bg.fill_ಬಣ್ಣ = *background_ಬಣ್ಣ;

		ಆದರೆ (plot_ಬಣ್ಣ) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}
	/* and plot the image */
	ಆದರೆ (plot_content) {
		ಪೂರ್ಣಾಂಕ ಅಗಲ = content_get_ಅಗಲ(box->background);
		ಪೂರ್ಣಾಂಕ ಎತ್ತರ = content_get_ಎತ್ತರ(box->background);

		ಆದರೆ (!repeat_x) {
			ಆದರೆ (r.x0 < x)
				r.x0 = x;
			ಆದರೆ (r.x1 > x + ಅಗಲ * scale)
				r.x1 = x + ಅಗಲ * scale;
		}
		ಆದರೆ (!repeat_y) {
			ಆದರೆ (r.y0 < y)
				r.y0 = y;
			ಆದರೆ (r.y1 > y + ಎತ್ತರ * scale)
				r.y1 = y + ಎತ್ತರ * scale;
		}
		/* valid clipping rectangles only */
		ಆದರೆ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ರಚನೆ content_redraw_ದತ್ತಾಂಶ bg_ದತ್ತಾಂಶ;

			res = ctx->plot->clip(ctx, &r);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}

			bg_ದತ್ತಾಂಶ.x = x;
			bg_ದತ್ತಾಂಶ.y = y;
			bg_ದತ್ತಾಂಶ.ಅಗಲ = ceilf(ಅಗಲ * scale);
			bg_ದತ್ತಾಂಶ.ಎತ್ತರ = ceilf(ಎತ್ತರ * scale);
			bg_ದತ್ತಾಂಶ.background_ಬಣ್ಣ = *background_ಬಣ್ಣ;
			bg_ದತ್ತಾಂಶ.scale = scale;
			bg_ದತ್ತಾಂಶ.repeat_x = repeat_x;
			bg_ದತ್ತಾಂಶ.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_ದತ್ತಾಂಶ, &r, ctx);
		}
	}

	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool
html_redraw_ಪಠ್ಯ_decoration_inline(ರಚನೆ box *box,
				   ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
				   ದಶಮಾಂಶ scale,
				   ಬಣ್ಣ ಬಣ್ಣ,
				   ದಶಮಾಂಶ ಅನುಪಾತ,
				   ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_ಬಣ್ಣ = ಬಣ್ಣ,
	};
	nserror res;
	ರಚನೆ rect rect;

	ಗಾಗಿ (c = box->ಮುಂದಿನ;
	     c && c != box->inline_ಕೊನೆ;
	     c = c->ಮುಂದಿನ) {
		ಆದರೆ (c->type != BOX_ಪಠ್ಯ) {
			ಮುಂದುವರಿ;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->ಎತ್ತರ * ಅನುಪಾತ) * scale;
		rect.x1 = (x + c->x + c->ಅಗಲ) * scale;
		rect.y1 = (y + c->y + c->ಎತ್ತರ * ಅನುಪಾತ) * scale;
		res = ctx->plot->ಸಾಲು(ctx, &plot_style_box, &rect);
		ಆದರೆ (res != NSERROR_OK) {
			ಹಿಂತಿರುಗಿ false;
		}
	}
	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool
html_redraw_ಪಠ್ಯ_decoration_ಖಂಡ(ರಚನೆ box *box,
				  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
				  ದಶಮಾಂಶ scale,
				  ಬಣ್ಣ ಬಣ್ಣ,
				  ದಶಮಾಂಶ ಅನುಪಾತ,
				  ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_ಬಣ್ಣ = ಬಣ್ಣ,
	};
	nserror res;
	ರಚನೆ rect rect;

	/* draw through text descendants */
	ಗಾಗಿ (c = box->children; c; c = c->ಮುಂದಿನ) {
		ಆದರೆ (c->type == BOX_ಪಠ್ಯ) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->ಎತ್ತರ * ಅನುಪಾತ) * scale;
			rect.x1 = (x + c->x + c->ಅಗಲ) * scale;
			rect.y1 = (y + c->y + c->ಎತ್ತರ * ಅನುಪಾತ) * scale;
			res = ctx->plot->ಸಾಲು(ctx, &plot_style_box, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_ಖಂಡ)) {
			ಆದರೆ (!html_redraw_ಪಠ್ಯ_decoration_ಖಂಡ(c,
					x + c->x, y + c->y,
					scale, ಬಣ್ಣ, ಅನುಪಾತ, ctx))
				ಹಿಂತಿರುಗಿ false;
		}
	}
	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_ಪಠ್ಯ_decoration(ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ, ದಶಮಾಂಶ scale,
		ಬಣ್ಣ background_ಬಣ್ಣ, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಎಣಿಕೆ css_ಪಠ್ಯ_decoration_e decoration[] = {
		CSS_ಪಠ್ಯ_DECORATION_UNDERLINE, CSS_ಪಠ್ಯ_DECORATION_OVERLINE,
		CSS_ಪಠ್ಯ_DECORATION_ಸಾಲು_THROUGH };
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ದಶಮಾಂಶ ಸಾಲು_ಅನುಪಾತ[] = { 0.9, 0.1, 0.5 };
	ಬಣ್ಣ fgcol;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	css_color ಲಂಬಸಾಲು;

	css_computed_color(box->style, &ಲಂಬಸಾಲು);
	fgcol = nscss_color_ಗಮ್ಯ_ns(ಲಂಬಸಾಲು);

	/* antialias colour for under/overline */
	ಆದರೆ (html_redraw_printing == false)
		fgcol = blend_ಬಣ್ಣ(background_ಬಣ್ಣ, fgcol);

	ಆದರೆ (box->type == BOX_INLINE) {
		ಆದರೆ (!box->inline_ಕೊನೆ)
			ಹಿಂತಿರುಗಿ true;
		ಗಾಗಿ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ಆದರೆ (css_computed_ಪಠ್ಯ_decoration(box->style) &
					decoration[i])
				ಆದರೆ (!html_redraw_ಪಠ್ಯ_decoration_inline(box,
						x_ಪೋಷಕ, y_ಪೋಷಕ, scale,
						fgcol, ಸಾಲು_ಅನುಪಾತ[i], ctx))
					ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಗಾಗಿ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ಆದರೆ (css_computed_ಪಠ್ಯ_decoration(box->style) &
					decoration[i])
				ಆದರೆ (!html_redraw_ಪಠ್ಯ_decoration_ಖಂಡ(box,
						x_ಪೋಷಕ + box->x,
						y_ಪೋಷಕ + box->y,
						scale,
						fgcol, ಸಾಲು_ಅನುಪಾತ[i], ctx))
					ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
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

ಸ್ಥಿರ bool html_redraw_ಪಠ್ಯ_box(ಸ್ಥಿರಾಂಕ html_content *html, ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y, ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ದಶಮಾಂಶ scale,
		ಬಣ್ಣ current_background_color,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	bool excluded = (box->object != ಖಾಲಿ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ಆದರೆ (!ಪಠ್ಯ_redraw(box->ಪಠ್ಯ,
			 box->ಉದ್ದಳತೆ,
			 box->byte_ಸ್ಥಾನಾಂತರ,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->ಎತ್ತರ,
			 scale,
			 excluded,
			 (ರಚನೆ content *)html,
			 html->sel,
			 ctx))
		ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ true;
}

bool html_redraw_box(ಸ್ಥಿರಾಂಕ html_content *html, ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ದಶಮಾಂಶ scale,
		ಬಣ್ಣ current_background_color,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx);

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

ಸ್ಥಿರ bool html_redraw_box_children(ಸ್ಥಿರಾಂಕ html_content *html, ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ದಶಮಾಂಶ scale,
		ಬಣ್ಣ current_background_color,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ರಚನೆ box *c;

	ಗಾಗಿ (c = box->children; c; c = c->ಮುಂದಿನ) {

		ಆದರೆ (c->type != BOX_FLOAT_ಎಡ && c->type != BOX_FLOAT_ಬಲ)
			ಆದರೆ (!html_redraw_box(html, c,
					x_ಪೋಷಕ + box->x -
					scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x),
					y_ಪೋಷಕ + box->y -
					scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				ಹಿಂತಿರುಗಿ false;
	}
	ಗಾಗಿ (c = box->float_children; c; c = c->ಮುಂದಿನ_float)
		ಆದರೆ (!html_redraw_box(html, c,
				x_ಪೋಷಕ + box->x -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x),
				y_ಪೋಷಕ + box->y -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ true;
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

bool html_redraw_box(ಸ್ಥಿರಾಂಕ html_content *html, ರಚನೆ box *box,
		ಪೂರ್ಣಾಂಕ x_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ y_ಪೋಷಕ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ದಶಮಾಂಶ scale,
		ಬಣ್ಣ current_background_color,
		ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ plotter_ಕೋಷ್ಟಕ *plot = ctx->plot;
	ಪೂರ್ಣಾಂಕ x, y;
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ padding_ಎಡ, padding_ತುದಿ, padding_ಅಗಲ, padding_ಎತ್ತರ;
	ಪೂರ್ಣಾಂಕ border_ಎಡ, border_ತುದಿ, border_ಬಲ, border_bottom;
	ರಚನೆ rect r;
	ರಚನೆ rect rect;
	ಪೂರ್ಣಾಂಕ x_scrolled, y_scrolled;
	ರಚನೆ box *bg_box = ಖಾಲಿ;
	css_computed_clip_rect css_rect;
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_x = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;
	ಎಣಿಕೆ css_ಅತಿಪ್ರವಾಹ_e ಅತಿಪ್ರವಾಹ_y = CSS_ಅತಿಪ್ರವಾಹ_VISIBLE;
	dom_exception exc;
	dom_html_element_type ಟ್ಯಾಗ್_type;


	ಆದರೆ (html_redraw_printing && (box->flags & PRINTED))
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (box->style != ಖಾಲಿ) {
		ಅತಿಪ್ರವಾಹ_x = css_computed_ಅತಿಪ್ರವಾಹ_x(box->style);
		ಅತಿಪ್ರವಾಹ_y = css_computed_ಅತಿಪ್ರವಾಹ_y(box->style);
	}

	/* avoid trivial FP maths */
	ಆದರೆ (scale == 1.0) {
		x = x_ಪೋಷಕ + box->x;
		y = y_ಪೋಷಕ + box->y;
		ಅಗಲ = box->ಅಗಲ;
		ಎತ್ತರ = box->ಎತ್ತರ;
		padding_ಎಡ = box->padding[ಎಡ_2];
		padding_ತುದಿ = box->padding[ತುದಿ_2];
		padding_ಅಗಲ = padding_ಎಡ + box->ಅಗಲ + box->padding[ಬಲ_2];
		padding_ಎತ್ತರ = padding_ತುದಿ + box->ಎತ್ತರ +
				box->padding[BOTTOM];
		border_ಎಡ = box->border[ಎಡ_2].ಅಗಲ;
		border_ತುದಿ = box->border[ತುದಿ_2].ಅಗಲ;
		border_ಬಲ = box->border[ಬಲ_2].ಅಗಲ;
		border_bottom = box->border[BOTTOM].ಅಗಲ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		x = (x_ಪೋಷಕ + box->x) * scale;
		y = (y_ಪೋಷಕ + box->y) * scale;
		ಅಗಲ = box->ಅಗಲ * scale;
		ಎತ್ತರ = box->ಎತ್ತರ * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ಎಡ = box->padding[ಎಡ_2] ? box->padding[ಎಡ_2] * scale
				: 0;
		padding_ತುದಿ = box->padding[ತುದಿ_2] ? box->padding[ತುದಿ_2] * scale
				: 0;
		padding_ಅಗಲ = (box->padding[ಎಡ_2] + box->ಅಗಲ +
				box->padding[ಬಲ_2]) * scale;
		padding_ಎತ್ತರ = (box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
				box->padding[BOTTOM]) * scale;
		border_ಎಡ = box->border[ಎಡ_2].ಅಗಲ * scale;
		border_ತುದಿ = box->border[ತುದಿ_2].ಅಗಲ * scale;
		border_ಬಲ = box->border[ಬಲ_2].ಅಗಲ * scale;
		border_bottom = box->border[BOTTOM].ಅಗಲ * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ಆದರೆ (box->style && ಅತಿಪ್ರವಾಹ_x != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
			box->ಪೋಷಕ != ಖಾಲಿ) {
		/* box contents clipped to box size */
		r.x0 = x - border_ಎಡ;
		r.x1 = x + padding_ಅಗಲ + border_ಬಲ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* box contents can hang out of the box; use descendant box */
		ಆದರೆ (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ಆದರೆ (!box->ಪೋಷಕ) {
			/* root element */
			ಪೂರ್ಣಾಂಕ margin_ಎಡ, margin_ಬಲ;
			ಆದರೆ (scale == 1.0) {
				margin_ಎಡ = box->margin[ಎಡ_2];
				margin_ಬಲ = box->margin[ಬಲ_2];
			} ಇಲ್ಲದಿದ್ದರೆ {
				margin_ಎಡ = box->margin[ಎಡ_2] * scale;
				margin_ಬಲ = box->margin[ಬಲ_2] * scale;
			}
			r.x0 = x - border_ಎಡ - margin_ಎಡ < r.x0 ?
					x - border_ಎಡ - margin_ಎಡ : r.x0;
			r.x1 = x + padding_ಅಗಲ + border_ಬಲ +
					margin_ಬಲ > r.x1 ?
					x + padding_ಅಗಲ + border_ಬಲ +
					margin_ಬಲ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ಆದರೆ (box->style && ಅತಿಪ್ರವಾಹ_y != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
			box->ಪೋಷಕ != ಖಾಲಿ) {
		/* box contents clipped to box size */
		r.y0 = y - border_ತುದಿ;
		r.y1 = y + padding_ಎತ್ತರ + border_bottom;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* box contents can hang out of the box; use descendant box */
		ಆದರೆ (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ಆದರೆ (!box->ಪೋಷಕ) {
			/* root element */
			ಪೂರ್ಣಾಂಕ margin_ತುದಿ, margin_bottom;
			ಆದರೆ (scale == 1.0) {
				margin_ತುದಿ = box->margin[ತುದಿ_2];
				margin_bottom = box->margin[BOTTOM];
			} ಇಲ್ಲದಿದ್ದರೆ {
				margin_ತುದಿ = box->margin[ತುದಿ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_ತುದಿ - margin_ತುದಿ < r.y0 ?
					y - border_ತುದಿ - margin_ತುದಿ : r.y0;
			r.y1 = y + padding_ಎತ್ತರ + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_ಎತ್ತರ + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ಆದರೆ (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		ಹಿಂತಿರುಗಿ true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ಆದರೆ (html_redraw_printing) {
		ಆದರೆ (r.y1 > html_redraw_printing_border) {
			ಆದರೆ (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_ಪಠ್ಯ ||
					box->type == BOX_ಕೋಷ್ಟಕ_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ಆದರೆ (r.y0 < html_redraw_printing_ತುದಿ_cropped)
					html_redraw_printing_ತುದಿ_cropped = r.y0;
				ಹಿಂತಿರುಗಿ true;
			}
		}
		ಇಲ್ಲದಿದ್ದರೆ box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ಆದರೆ (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ಆದರೆ ((ctx->plot->ಗುಂಪು_ಆರಂಭ) &&
		    (ctx->plot->ಗುಂಪು_ಆರಂಭ(ctx, "hidden box") != NSERROR_OK))
			ಹಿಂತಿರುಗಿ false;
		ಆದರೆ (!html_redraw_box_children(html, box, x_ಪೋಷಕ, y_ಪೋಷಕ,
				&r, scale, current_background_color, ctx))
			ಹಿಂತಿರುಗಿ false;
		ಹಿಂತಿರುಗಿ ((!ctx->plot->ಗುಂಪು_ಕೊನೆ) || (ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
	}

	ಆದರೆ ((ctx->plot->ಗುಂಪು_ಆರಂಭ) &&
	    (ctx->plot->ಗುಂಪು_ಆರಂಭ(ctx,"vis box") != NSERROR_OK)) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (box->style != ಖಾಲಿ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ಆದರೆ (css_rect.ಎಡ_auto == false)
			r.x0 = x - border_ಎಡ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ಎಡ, css_rect.lunit));

		ಆದರೆ (css_rect.ತುದಿ_auto == false)
			r.y0 = y - border_ತುದಿ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ತುದಿ, css_rect.tunit));

		ಆದರೆ (css_rect.ಬಲ_auto == false)
			r.x1 = x - border_ಎಡ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ಬಲ, css_rect.runit));

		ಆದರೆ (css_rect.bottom_auto == false)
			r.y1 = y - border_ತುದಿ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ಆದರೆ (r.x0 < clip->x0) r.x0 = clip->x0;
		ಆದರೆ (r.y0 < clip->y0) r.y0 = clip->y0;
		ಆದರೆ (clip->x1 < r.x1) r.x1 = clip->x1;
		ಆದರೆ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ಆದರೆ (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			ಹಿಂತಿರುಗಿ ((!ctx->plot->ಗುಂಪು_ಕೊನೆ) ||
				(ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
		/* clip to it */
		ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->type == BOX_ಖಂಡ || box->type == BOX_INLINE_ಖಂಡ ||
			box->type == BOX_ಕೋಷ್ಟಕ_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ಆದರೆ (r.x0 < clip->x0) r.x0 = clip->x0;
		ಆದರೆ (r.y0 < clip->y0) r.y0 = clip->y0;
		ಆದರೆ (clip->x1 < r.x1) r.x1 = clip->x1;
		ಆದರೆ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ಆದರೆ (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			ಹಿಂತಿರುಗಿ ((!ctx->plot->ಗುಂಪು_ಕೊನೆ) ||
				(ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
		/* clip to it */
		ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* clip box is fine, clip to it */
		r = *clip;
		ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_ಕಂಡುಹಿಡಿ_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ಆದರೆ (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_ಪಠ್ಯ &&
			bg_box->type != BOX_INLINE_ಕೊನೆ &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ಖಾಲಿ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ರಚನೆ rect p;
		p.x0 = x - border_ಎಡ < r.x0 ? r.x0 : x - border_ಎಡ;
		p.y0 = y - border_ತುದಿ < r.y0 ? r.y0 : y - border_ತುದಿ;
		p.x1 = x + padding_ಅಗಲ + border_ಬಲ < r.x1 ?
				x + padding_ಅಗಲ + border_ಬಲ : r.x1;
		p.y1 = y + padding_ಎತ್ತರ + border_bottom < r.y1 ?
				y + padding_ಎತ್ತರ + border_bottom : r.y1;
		ಆದರೆ (!box->ಪೋಷಕ) {
			/* Root element, special case:
			 * background covers margins too */
			ಪೂರ್ಣಾಂಕ m_ಎಡ, m_ತುದಿ, m_ಬಲ, m_bottom;
			ಆದರೆ (scale == 1.0) {
				m_ಎಡ = box->margin[ಎಡ_2];
				m_ತುದಿ = box->margin[ತುದಿ_2];
				m_ಬಲ = box->margin[ಬಲ_2];
				m_bottom = box->margin[BOTTOM];
			} ಇಲ್ಲದಿದ್ದರೆ {
				m_ಎಡ = box->margin[ಎಡ_2] * scale;
				m_ತುದಿ = box->margin[ತುದಿ_2] * scale;
				m_ಬಲ = box->margin[ಬಲ_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ಎಡ < r.x0 ? r.x0 : p.x0 - m_ಎಡ;
			p.y0 = p.y0 - m_ತುದಿ < r.y0 ? r.y0 : p.y0 - m_ತುದಿ;
			p.x1 = p.x1 + m_ಬಲ < r.x1 ? p.x1 + m_ಬಲ : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		ಆದರೆ ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ಆದರೆ (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				ಹಿಂತಿರುಗಿ false;
			/* restore previous graphics window */
			ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ಹಿಂತಿರುಗಿ false;
		}
	}

	/* borders for block level content and replaced inlines */
	ಆದರೆ (box->style &&
	    box->type != BOX_ಪಠ್ಯ &&
	    box->type != BOX_INLINE_ಕೊನೆ &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ಖಾಲಿ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_ತುದಿ || border_ಬಲ || border_bottom || border_ಎಡ)) {
		ಆದರೆ (!html_redraw_borders(box, x_ಪೋಷಕ, y_ಪೋಷಕ,
				padding_ಅಗಲ, padding_ಎತ್ತರ, &r,
				scale, ctx))
			ಹಿಂತಿರುಗಿ false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ಆದರೆ (box->style && box->type == BOX_INLINE && box->inline_ಕೊನೆ &&
			(html_redraw_box_has_background(box) ||
			border_ತುದಿ || border_ಬಲ ||
			border_bottom || border_ಎಡ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ರಚನೆ box *ib;
		ರಚನೆ rect b; /* border edge rectangle */
		ರಚನೆ rect p; /* clipped rect */
		bool ಮೊದಲ = true;
		ಪೂರ್ಣಾಂಕ ib_x;
		ಪೂರ್ಣಾಂಕ ib_y = y;
		ಪೂರ್ಣಾಂಕ ib_p_ಅಗಲ;
		ಪೂರ್ಣಾಂಕ ib_b_ಎಡ, ib_b_ಬಲ;

		b.x0 = x - border_ಎಡ;
		b.x1 = x + padding_ಅಗಲ + border_ಬಲ;
		b.y0 = y - border_ತುದಿ;
		b.y1 = y + padding_ಎತ್ತರ + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		ಗಾಗಿ (ib = box; ib; ib = ib->ಮುಂದಿನ) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ಆದರೆ (ib->type == BOX_FLOAT_ಎಡ ||
					ib->type == BOX_FLOAT_ಬಲ)
				ಮುಂದುವರಿ;
			ಆದರೆ (scale == 1.0) {
				ib_x = x_ಪೋಷಕ + ib->x;
				ib_y = y_ಪೋಷಕ + ib->y;
				ib_p_ಅಗಲ = ib->padding[ಎಡ_2] + ib->ಅಗಲ +
						ib->padding[ಬಲ_2];
				ib_b_ಎಡ = ib->border[ಎಡ_2].ಅಗಲ;
				ib_b_ಬಲ = ib->border[ಬಲ_2].ಅಗಲ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ib_x = (x_ಪೋಷಕ + ib->x) * scale;
				ib_y = (y_ಪೋಷಕ + ib->y) * scale;
				ib_p_ಅಗಲ = (ib->padding[ಎಡ_2] + ib->ಅಗಲ +
						ib->padding[ಬಲ_2]) * scale;
				ib_b_ಎಡ = ib->border[ಎಡ_2].ಅಗಲ * scale;
				ib_b_ಬಲ = ib->border[ಬಲ_2].ಅಗಲ * scale;
			}

			ಆದರೆ ((ib->flags & NEW_ಸಾಲು) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ಆದರೆ (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						ಮೊದಲ, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					ಹಿಂತಿರುಗಿ false;
				/* restore previous graphics window */
				ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					ಹಿಂತಿರುಗಿ false;
				ಆದರೆ (!html_redraw_inline_borders(box, b, &r,
						scale, ಮೊದಲ, false, ctx))
					ಹಿಂತಿರುಗಿ false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ಎಡ;
				b.y0 = ib_y - border_ತುದಿ - padding_ತುದಿ;
				b.y1 = ib_y + padding_ಎತ್ತರ - padding_ತುದಿ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				ಮೊದಲ = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_ಅಗಲ + ib_b_ಬಲ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ಆದರೆ (ib == box->inline_ಕೊನೆ)
				/* reached end of BOX_INLINE span */
				ನಿಲ್ಲಿಸು;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ಆದರೆ (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				ಮೊದಲ, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			ಹಿಂತಿರುಗಿ false;
		/* restore previous graphics window */
		ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
		ಆದರೆ (!html_redraw_inline_borders(box, b, &r, scale, ಮೊದಲ, true,
				ctx))
			ಹಿಂತಿರುಗಿ false;

	}

	/* Debug outlines */
	ಆದರೆ (html_redraw_debug) {
		ಪೂರ್ಣಾಂಕ margin_ಎಡ, margin_ಬಲ;
		ಪೂರ್ಣಾಂಕ margin_ತುದಿ, margin_bottom;
		ಆದರೆ (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ಎಡ = box->margin[ಎಡ_2];
			margin_ತುದಿ = box->margin[ತುದಿ_2];
			margin_ಬಲ = box->margin[ಬಲ_2];
			margin_bottom = box->margin[BOTTOM];
		} ಇಲ್ಲದಿದ್ದರೆ {
			margin_ಎಡ = box->margin[ಎಡ_2] * scale;
			margin_ತುದಿ = box->margin[ತುದಿ_2] * scale;
			margin_ಬಲ = box->margin[ಬಲ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ಎಡ;
		rect.y0 = y + padding_ತುದಿ;
		rect.x1 = x + padding_ಎಡ + ಅಗಲ;
		rect.y1 = y + padding_ತುದಿ + ಎತ್ತರ;
		ಆದರೆ (ctx->plot->rectangle(ctx, plot_style_content_ಅಂಚು, &rect) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_ಅಗಲ;
		rect.y1 = y + padding_ಎತ್ತರ;
		ಆದರೆ (ctx->plot->rectangle(ctx, plot_style_padding_ಅಂಚು, &rect) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ಎಡ - margin_ಎಡ;
		rect.y0 = y - border_ತುದಿ - margin_ತುದಿ;
		rect.x1 = x + padding_ಅಗಲ + border_ಬಲ + margin_ಬಲ;
		rect.y1 = y + padding_ಎತ್ತರ + border_bottom + margin_bottom;
		ಆದರೆ (ctx->plot->rectangle(ctx, plot_style_margin_ಅಂಚು, &rect) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ಆದರೆ (box->ಪೋಷಕ != ಖಾಲಿ) {
		bool need_clip = false;
		ಆದರೆ (box->object || box->flags & IFRAME ||
				(ಅತಿಪ್ರವಾಹ_x != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE &&
				 ಅತಿಪ್ರವಾಹ_y != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_ಅಗಲ;
			r.y1 = y + padding_ಎತ್ತರ;
			ಆದರೆ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಆದರೆ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಆದರೆ (clip->x1 < r.x1) r.x1 = clip->x1;
			ಆದರೆ (clip->y1 < r.y1) r.y1 = clip->y1;
			ಆದರೆ (r.x1 <= r.x0 || r.y1 <= r.y0) {
				ಹಿಂತಿರುಗಿ (!ctx->plot->ಗುಂಪು_ಕೊನೆ ||
					(ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅತಿಪ್ರವಾಹ_x != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_ಅಗಲ;
			r.y1 = clip->y1;
			ಆದರೆ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಆದರೆ (clip->x1 < r.x1) r.x1 = clip->x1;
			ಆದರೆ (r.x1 <= r.x0) {
				ಹಿಂತಿರುಗಿ (!ctx->plot->ಗುಂಪು_ಕೊನೆ ||
					(ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅತಿಪ್ರವಾಹ_y != CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_ಎತ್ತರ;
			ಆದರೆ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಆದರೆ (clip->y1 < r.y1) r.y1 = clip->y1;
			ಆದರೆ (r.y1 <= r.y0) {
				ಹಿಂತಿರುಗಿ (!ctx->plot->ಗುಂಪು_ಕೊನೆ ||
					(ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ಆದರೆ (need_clip &&
		    (box->type == BOX_ಖಂಡ ||
		     box->type == BOX_INLINE_ಖಂಡ ||
		     box->type == BOX_ಕೋಷ್ಟಕ_CELL || box->object)) {
			ಆದರೆ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ಹಿಂತಿರುಗಿ false;
		}
	}

	/* text decoration */
	ಆದರೆ ((box->type != BOX_ಪಠ್ಯ) &&
	    box->style &&
	    css_computed_ಪಠ್ಯ_decoration(box->style) !=	CSS_ಪಠ್ಯ_DECORATION_NONE) {
		ಆದರೆ (!html_redraw_ಪಠ್ಯ_decoration(box, x_ಪೋಷಕ, y_ಪೋಷಕ,
				scale, current_background_color, ctx))
			ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (box->ಗಂಟು != ಖಾಲಿ) {
		exc = dom_html_element_get_ಟ್ಯಾಗ್_type(box->ಗಂಟು, &ಟ್ಯಾಗ್_type);
		ಆದರೆ (exc != DOM_NO_ERR) {
			ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ಆದರೆ (box->object && ಅಗಲ != 0 && ಎತ್ತರ != 0) {
		ರಚನೆ content_redraw_ದತ್ತಾಂಶ obj_ದತ್ತಾಂಶ;

		x_scrolled = x - scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y) * scale;

		obj_ದತ್ತಾಂಶ.x = x_scrolled + padding_ಎಡ;
		obj_ದತ್ತಾಂಶ.y = y_scrolled + padding_ತುದಿ;
		obj_ದತ್ತಾಂಶ.ಅಗಲ = ಅಗಲ;
		obj_ದತ್ತಾಂಶ.ಎತ್ತರ = ಎತ್ತರ;
		obj_ದತ್ತಾಂಶ.background_ಬಣ್ಣ = current_background_color;
		obj_ದತ್ತಾಂಶ.scale = scale;
		obj_ದತ್ತಾಂಶ.repeat_x = false;
		obj_ದತ್ತಾಂಶ.repeat_y = false;

		ಆದರೆ (content_get_type(box->object) == CONTENT_HTML) {
			obj_ದತ್ತಾಂಶ.x /= scale;
			obj_ದತ್ತಾಂಶ.y /= scale;
		}

		ಆದರೆ (!content_redraw(box->object, &obj_ದತ್ತಾಂಶ, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *obj = "\xef\xbf\xbc";
			ಪೂರ್ಣಾಂಕ obj_ಅಗಲ;
			ಪೂರ್ಣಾಂಕ obj_x = x + padding_ಎಡ;
			nserror res;

			rect.x0 = x + padding_ಎಡ;
			rect.y0 = y + padding_ತುದಿ;
			rect.x1 = x + padding_ಎಡ + ಅಗಲ - 1;
			rect.y1 = y + padding_ತುದಿ + ಎತ್ತರ - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ಆದರೆ (res != NSERROR_OK) {
				ಹಿಂತಿರುಗಿ false;
			}

			res = guit->layout->ಅಗಲ(plot_fstyle_broken_object,
						  obj,
						  ಗಾತ್ರ(obj) - 1,
						  &obj_ಅಗಲ);
			ಆದರೆ (res != NSERROR_OK) {
				obj_x += 1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				obj_x += ಅಗಲ / 2 - obj_ಅಗಲ / 2;
			}

			ಆದರೆ (ctx->plot->ಪಠ್ಯ(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_ತುದಿ + (ಪೂರ್ಣಾಂಕ)(ಎತ್ತರ * 0.75),
					    obj, ಗಾತ್ರ(obj) - 1) != NSERROR_OK)
				ಹಿಂತಿರುಗಿ false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಟ್ಯಾಗ್_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ಗಂಟು != ಖಾಲಿ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ರಚನೆ bitmap *bitmap = ಖಾಲಿ;
		exc = dom_ಗಂಟು_get_user_ದತ್ತಾಂಶ(box->ಗಂಟು,
					     corestring_dom_ns_ಕೀಲಿ_canvas_ಗಂಟು_ದತ್ತಾಂಶ,
					     &bitmap);
		ಆದರೆ (exc != DOM_NO_ERR) {
			bitmap = ಖಾಲಿ;
		}
		ಆದರೆ (bitmap != ಖಾಲಿ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ಎಡ, y + padding_ತುದಿ,
				      ಅಗಲ, ಎತ್ತರ, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_ಕಿಟಕಿ_redraw(box->iframe,
				x + padding_ಎಡ,
				y + padding_ತುದಿ, &r, ctx);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ಆದರೆ (!html_redraw_checkbox(x + padding_ಎಡ, y + padding_ತುದಿ,
				ಅಗಲ, ಎತ್ತರ, box->gadget->selected, ctx))
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget && box->gadget->type == GADGET_RADIO) {
		ಆದರೆ (!html_redraw_radio(x + padding_ಎಡ, y + padding_ತುದಿ,
				ಅಗಲ, ಎತ್ತರ, box->gadget->selected, ctx))
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget && box->gadget->type == GADGET_FILE) {
		ಆದರೆ (!html_redraw_file(x + padding_ಎಡ, y + padding_ತುದಿ,
				ಅಗಲ, ಎತ್ತರ, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->ದತ್ತಾಂಶ.ಪಠ್ಯ.ta, x, y,
				current_background_color, scale, &r, ctx);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->ಪಠ್ಯ) {
		ಆದರೆ (!html_redraw_ಪಠ್ಯ_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			ಹಿಂತಿರುಗಿ false;

	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (!html_redraw_box_children(html, box, x_ಪೋಷಕ, y_ಪೋಷಕ, &r,
				scale, current_background_color, ctx))
			ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (box->type == BOX_ಖಂಡ || box->type == BOX_INLINE_ಖಂಡ ||
			box->type == BOX_ಕೋಷ್ಟಕ_CELL || box->type == BOX_INLINE)
		ಆದರೆ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;

	/* list marker */
	ಆದರೆ (box->list_marker) {
		ಆದರೆ (!html_redraw_box(html, box->list_marker,
				x_ಪೋಷಕ + box->x -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x),
				y_ಪೋಷಕ + box->y -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y),
				clip, scale, current_background_color, ctx))
			ಹಿಂತಿರುಗಿ false;
	}

	/* scrollbars */
	ಆದರೆ (((box->style && box->type != BOX_BR &&
	      box->type != BOX_ಕೋಷ್ಟಕ && box->type != BOX_INLINE &&
	      (box->gadget == ಖಾಲಿ || box->gadget->type != GADGET_TEXTAREA) &&
	      (ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
	       ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_AUTO ||
	       ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL ||
	       ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->ಪೋಷಕ != ಖಾಲಿ) {
		nserror res;
		bool has_x_scroll = (ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_SCROLL);
		bool has_y_scroll = (ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_SCROLL);

		has_x_scroll |= (ಅತಿಪ್ರವಾಹ_x == CSS_ಅತಿಪ್ರವಾಹ_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (ಅತಿಪ್ರವಾಹ_y == CSS_ಅತಿಪ್ರವಾಹ_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ರಚನೆ content *)html,
					    box, has_x_scroll, has_y_scroll);
		ಆದರೆ (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (box->scroll_x != ಖಾಲಿ)
			scrollbar_redraw(box->scroll_x,
					x_ಪೋಷಕ + box->x,
					y_ಪೋಷಕ + box->y + box->padding[ತುದಿ_2] +
					box->ಎತ್ತರ + box->padding[BOTTOM] -
					SCROLLBAR_ಅಗಲ, clip, scale, ctx);
		ಆದರೆ (box->scroll_y != ಖಾಲಿ)
			scrollbar_redraw(box->scroll_y,
					x_ಪೋಷಕ + box->x + box->padding[ಎಡ_2] +
					box->ಅಗಲ + box->padding[ಬಲ_2] -
					SCROLLBAR_ಅಗಲ,
					y_ಪೋಷಕ + box->y, clip, scale, ctx);
	}

	ಆದರೆ (box->type == BOX_ಖಂಡ || box->type == BOX_INLINE_ಖಂಡ ||
	    box->type == BOX_ಕೋಷ್ಟಕ_CELL || box->type == BOX_INLINE) {
		ಆದರೆ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ ((!plot->ಗುಂಪು_ಕೊನೆ) || (ctx->plot->ಗುಂಪು_ಕೊನೆ(ctx) == NSERROR_OK));
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

bool html_redraw(ರಚನೆ content *c, ರಚನೆ content_redraw_ದತ್ತಾಂಶ *ದತ್ತಾಂಶ,
		ಸ್ಥಿರಾಂಕ ರಚನೆ rect *clip, ಸ್ಥಿರಾಂಕ ರಚನೆ redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ರಚನೆ box *box;
	bool ಫಲಿತಾಂಶ = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_ಬಣ್ಣ = ದತ್ತಾಂಶ->background_ಬಣ್ಣ,
	};

	box = html->layout;
	ಪ್ರತಿಜ್ಞಾ(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ಆದರೆ (ctx->interactive && html->visible_select_menu != ಖಾಲಿ) {
		ರಚನೆ form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				ದತ್ತಾಂಶ->scale, clip);
	}

	ಆದರೆ (!select_only) {
		/* clear to background colour */
		ಫಲಿತಾಂಶ = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ಆದರೆ (html->background_ಬಣ್ಣ != NS_TRANSPARENT)
			pstyle_fill_bg.fill_ಬಣ್ಣ = html->background_ಬಣ್ಣ;

		ಫಲಿತಾಂಶ &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		ಫಲಿತಾಂಶ &= html_redraw_box(html, box, ದತ್ತಾಂಶ->x, ದತ್ತಾಂಶ->y, clip,
				ದತ್ತಾಂಶ->scale, pstyle_fill_bg.fill_ಬಣ್ಣ, ctx);
	}

	ಆದರೆ (select) {
		ಪೂರ್ಣಾಂಕ menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ಎಡ_2].ಅಗಲ;
		menu_y += box->ಎತ್ತರ + box->border[BOTTOM].ಅಗಲ +
				box->padding[BOTTOM] + box->padding[ತುದಿ_2];
		ಫಲಿತಾಂಶ &= form_redraw_select_menu(html->visible_select_menu,
				ದತ್ತಾಂಶ->x + menu_x, ದತ್ತಾಂಶ->y + menu_y,
				ದತ್ತಾಂಶ->scale, clip, ctx);
	}

	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

}
