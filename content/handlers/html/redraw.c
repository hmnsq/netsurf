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
ಸ್ಥಿರ bool html_redraw_box_has_background(ಸಂರಚನಾ box *box)
{
	ಯದಿ (box->background != ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (box->style != ರಿಕ್ತ) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		ಯದಿ (nscss_color_is_transparent(colour) == false)
			ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ಸ್ಥಿರ ಸಂರಚನಾ box *html_redraw_find_bg_box(ಸಂರಚನಾ box *box)
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
	ಯದಿ (box->parent == ರಿಕ್ತ) {
		/* Root box */
		ಯದಿ (html_redraw_box_has_background(box))
			ಪ್ರತ್ಯಾಗಮ box;

		/* No background on root box: consider body box, if any */
		ಯದಿ (box->children != ರಿಕ್ತ) {
			ಯದಿ (html_redraw_box_has_background(box->children))
				ಪ್ರತ್ಯಾಗಮ box->children;
		}
	} ಅನ್ಯಥಾ ಯದಿ (box->parent != ರಿಕ್ತ && box->parent->parent == ರಿಕ್ತ) {
		/* Body box: only render background if root has its own */
		ಯದಿ (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			ಪ್ರತ್ಯಾಗಮ box;
	} ಅನ್ಯಥಾ {
		/* Any other box */
		ಯದಿ (html_redraw_box_has_background(box))
			ಪ್ರತ್ಯಾಗಮ box;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
text_redraw(ಧ್ರುವ ವರ್ಣ *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    ಪೂರ್ಣಾಙ್ಕ space,
	    ಧ್ರುವ plot_font_style_t *fstyle,
	    ಪೂರ್ಣಾಙ್ಕ x,
	    ಪೂರ್ಣಾಙ್ಕ y,
	    ಧ್ರುವ ಸಂರಚನಾ rect *clip,
	    ಪೂರ್ಣಾಙ್ಕ height,
	    ಭಿನ್ನ scale,
	    bool excluded,
	    ಸಂರಚನಾ content *c,
	    ಧ್ರುವ ಸಂರಚನಾ selection *sel,
	    ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ಯದಿ (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ಯದಿ (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ಯದಿ (!highlighted &&
		    (c->textsearch.context != ರಿಕ್ತ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ಯದಿ (highlighted) {
			ಸಂರಚನಾ rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			ಪೂರ್ಣಾಙ್ಕ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ಯದಿ (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ಪ್ರತಿಜ್ಞಾ(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			ಯದಿ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			ಯದಿ (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ಯದಿ (end_idx > utf8_len) {
					endx += space;
			}

			ಯದಿ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ಯದಿ ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				ಪ್ರತ್ಯಾಗಮ false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (start_idx > 0) {
				ಪೂರ್ಣಾಙ್ಕ px0 = max(x + startx, clip->x0);
				ಪೂರ್ಣಾಙ್ಕ px1 = min(x + endx, clip->x1);

				ಯದಿ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}

					clip_changed = true;
				} ಅನ್ಯಥಾ {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ಯದಿ (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			/* draw any text succeeding highlighted portion */
			ಯದಿ (endtxt_idx < utf8_len) {
				ಪೂರ್ಣಾಙ್ಕ px0 = max(x + endx, clip->x0);
				ಯದಿ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ಯದಿ (res != NSERROR_OK) {
						ಪ್ರತ್ಯಾಗಮ false;
					}
				}
			}

			ಯದಿ (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}

	ಯದಿ (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (ಪೂರ್ಣಾಙ್ಕ) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}
	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_checkbox(ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height,
		bool selected, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ದ್ವಿಗುಣ z;
	nserror res;
	ಸಂರಚನಾ rect rect;

	z = width * 0.15;
	ಯದಿ (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_darkwbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_darkwbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_lightwbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_lightwbasec, &rect);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (selected) {
		ಯದಿ (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		} ಅನ್ಯಥಾ {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_wblobc, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->ಸಾಲು(ctx, plot_style_stroke_wblobc, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}
	ಪ್ರತ್ಯಾಗಮ true;
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
ಸ್ಥಿರ bool html_redraw_radio(ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height,
		bool selected, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_file(ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಪೂರ್ಣಾಙ್ಕ width, ಪೂರ್ಣಾಙ್ಕ height,
		ಸಂರಚನಾ box *box, ಭಿನ್ನ scale, colour background_colour,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಪೂರ್ಣಾಙ್ಕ text_width;
	ಧ್ರುವ ವರ್ಣ *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	ಯದಿ (box->gadget->ಮೌಲ್ಯ) {
		text = box->gadget->ಮೌಲ್ಯ;
	} ಅನ್ಯಥಾ {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}
	text_width *= scale;
	ಯದಿ (width < text_width + 8) {
		x = x + width - text_width - 4;
	} ಅನ್ಯಥಾ {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	ಯದಿ (res != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_background(ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಸಂರಚನಾ box *box, ಭಿನ್ನ scale,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, colour *background_colour,
		ಸಂರಚನಾ box *background,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	ಸಂರಚನಾ box *clip_box = box;
	ಪೂರ್ಣಾಙ್ಕ ox = x, oy = y;
	ಪೂರ್ಣಾಙ್ಕ width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ಸಂರಚನಾ box *parent;
	ಸಂರಚನಾ rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	ಯದಿ (ctx->background_images == false)
		ಪ್ರತ್ಯಾಗಮ true;

	plot_content = (background->background != ರಿಕ್ತ);

	ಯದಿ (plot_content) {
		ಯದಿ (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ಎಡ_2] * scale;
			y -= box->margin[ತುದಿ_2] * scale;
			width = box->margin[ಎಡ_2] + box->padding[ಎಡ_2] +
					box->width + box->padding[ಬಲ_2] +
					box->margin[ಬಲ_2];
			height = box->margin[ತುದಿ_2] + box->padding[ತುದಿ_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} ಅನ್ಯಥಾ {
			width = box->padding[ಎಡ_2] + box->width +
					box->padding[ಬಲ_2];
			height = box->padding[ತುದಿ_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		ವಿಕಲ್ಪ (css_computed_background_repeat(background->style)) {
		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ಯದಿ (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} ಅನ್ಯಥಾ {
			x += (ಪೂರ್ಣಾಙ್ಕ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ಯದಿ (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} ಅನ್ಯಥಾ {
			y += (ಪೂರ್ಣಾಙ್ಕ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ಯದಿ (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		ಪ್ರತಿ (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		ಪ್ರತಿಜ್ಞಾ(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		ಯದಿ (clip_to_children)
			clip_box = box->children;
	}

	ಪ್ರತಿ (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		ಯದಿ (clip_to_children) {
			ಪ್ರತಿಜ್ಞಾ(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ಎಡ_2] +
					clip_box->width +
					clip_box->padding[ಬಲ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[ತುದಿ_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			ಯದಿ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಯದಿ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಯದಿ (r.x1 > clip->x1) r.x1 = clip->x1;
			ಯದಿ (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ಯದಿ ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ರಿಕ್ತ) &&
			     content_get_opaque(clip_box->background)))
				ಅನುವರ್ತ;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ಯದಿ (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			ಯದಿ (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ಯದಿ (res != NSERROR_OK) {
					ಪ್ರತ್ಯಾಗಮ false;
				}
			}
		}
		/* and plot the image */
		ಯದಿ (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			ಯದಿ (!repeat_x) {
				ಯದಿ (r.x0 < x)
					r.x0 = x;
				ಯದಿ (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			ಯದಿ (!repeat_y) {
				ಯದಿ (r.y0 < y)
					r.y0 = y;
				ಯದಿ (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			ಯದಿ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ಸಂರಚನಾ content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				ಯದಿ (res != NSERROR_OK) {
					ಪ್ರತ್ಯಾಗಮ false;
				}

				bg_data.x = x;
				bg_data.y = y;
				bg_data.width = ceilf(width * scale);
				bg_data.height = ceilf(height * scale);
				bg_data.background_colour = *background_colour;
				bg_data.scale = scale;
				bg_data.repeat_x = repeat_x;
				bg_data.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_data, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		ಯದಿ (!clip_to_children)
			ಪ್ರತ್ಯಾಗಮ true;
	}
	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_inline_background(ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಸಂರಚನಾ box *box,
		ಭಿನ್ನ scale, ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಸಂರಚನಾ rect b,
		bool first, bool last, colour *background_colour,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ rect r = *clip;
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	plot_content = (box->background != ರಿಕ್ತ);

	ಯದಿ (html_redraw_printing && nsoption_bool(remove_backgrounds))
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (plot_content) {
		/* handle background-repeat */
		ವಿಕಲ್ಪ (css_computed_background_repeat(box->style)) {
		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ಭಙ್ಗ;

		ಸ್ವಭಾವ:
			ಭಙ್ಗ;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ಯದಿ (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ಯದಿ (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} ಅನ್ಯಥಾ {
			x += (ಪೂರ್ಣಾಙ್ಕ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ಯದಿ (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} ಅನ್ಯಥಾ {
			y += (ಪೂರ್ಣಾಙ್ಕ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ಯದಿ (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		ಯದಿ (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}
	/* and plot the image */
	ಯದಿ (plot_content) {
		ಪೂರ್ಣಾಙ್ಕ width = content_get_width(box->background);
		ಪೂರ್ಣಾಙ್ಕ height = content_get_height(box->background);

		ಯದಿ (!repeat_x) {
			ಯದಿ (r.x0 < x)
				r.x0 = x;
			ಯದಿ (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		ಯದಿ (!repeat_y) {
			ಯದಿ (r.y0 < y)
				r.y0 = y;
			ಯದಿ (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		ಯದಿ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ಸಂರಚನಾ content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			bg_data.x = x;
			bg_data.y = y;
			bg_data.width = ceilf(width * scale);
			bg_data.height = ceilf(height * scale);
			bg_data.background_colour = *background_colour;
			bg_data.scale = scale;
			bg_data.repeat_x = repeat_x;
			bg_data.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_data, &r, ctx);
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
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
html_redraw_text_decoration_inline(ಸಂರಚನಾ box *box,
				   ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
				   ಭಿನ್ನ scale,
				   colour colour,
				   ಭಿನ್ನ ratio,
				   ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ಸಂರಚನಾ rect rect;

	ಪ್ರತಿ (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		ಯದಿ (c->type != BOX_TEXT) {
			ಅನುವರ್ತ;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->ಸಾಲು(ctx, &plot_style_box, &rect);
		ಯದಿ (res != NSERROR_OK) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}
	ಪ್ರತ್ಯಾಗಮ true;
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
html_redraw_text_decoration_block(ಸಂರಚನಾ box *box,
				  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
				  ಭಿನ್ನ scale,
				  colour colour,
				  ಭಿನ್ನ ratio,
				  ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ಸಂರಚನಾ rect rect;

	/* draw through text descendants */
	ಪ್ರತಿ (c = box->children; c; c = c->next) {
		ಯದಿ (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->ಸಾಲು(ctx, &plot_style_box, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}
		} ಅನ್ಯಥಾ ಯದಿ ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			ಯದಿ (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				ಪ್ರತ್ಯಾಗಮ false;
		}
	}
	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_text_decoration(ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ x_parent, ಪೂರ್ಣಾಙ್ಕ y_parent, ಭಿನ್ನ scale,
		colour background_colour, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸ್ಥಿರ ಧ್ರುವ ಗಣನಾ css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_ಸಾಲು_THROUGH };
	ಸ್ಥಿರ ಧ್ರುವ ಭಿನ್ನ ಸಾಲು_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	ಯದಿ (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	ಯದಿ (box->type == BOX_INLINE) {
		ಯದಿ (!box->inline_end)
			ಪ್ರತ್ಯಾಗಮ true;
		ಪ್ರತಿ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ಯದಿ (css_computed_text_decoration(box->style) &
					decoration[i])
				ಯದಿ (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, ಸಾಲು_ratio[i], ctx))
					ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ {
		ಪ್ರತಿ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ಯದಿ (css_computed_text_decoration(box->style) &
					decoration[i])
				ಯದಿ (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, ಸಾಲು_ratio[i], ctx))
					ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
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

ಸ್ಥಿರ bool html_redraw_text_box(ಧ್ರುವ html_content *html, ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y, ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಭಿನ್ನ scale,
		colour current_background_color,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	bool excluded = (box->object != ರಿಕ್ತ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ಯದಿ (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (ಸಂರಚನಾ content *)html,
			 html->sel,
			 ctx))
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ true;
}

bool html_redraw_box(ಧ್ರುವ html_content *html, ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ x_parent, ಪೂರ್ಣಾಙ್ಕ y_parent,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಭಿನ್ನ scale,
		colour current_background_color,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx);

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

ಸ್ಥಿರ bool html_redraw_box_children(ಧ್ರುವ html_content *html, ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ x_parent, ಪೂರ್ಣಾಙ್ಕ y_parent,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಭಿನ್ನ scale,
		colour current_background_color,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಸಂರಚನಾ box *c;

	ಪ್ರತಿ (c = box->children; c; c = c->next) {

		ಯದಿ (c->type != BOX_FLOAT_ಎಡ && c->type != BOX_FLOAT_ಬಲ)
			ಯದಿ (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				ಪ್ರತ್ಯಾಗಮ false;
	}
	ಪ್ರತಿ (c = box->float_children; c; c = c->next_float)
		ಯದಿ (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ true;
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

bool html_redraw_box(ಧ್ರುವ html_content *html, ಸಂರಚನಾ box *box,
		ಪೂರ್ಣಾಙ್ಕ x_parent, ಪೂರ್ಣಾಙ್ಕ y_parent,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಭಿನ್ನ scale,
		colour current_background_color,
		ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	ಧ್ರುವ ಸಂರಚನಾ plotter_table *plot = ctx->plot;
	ಪೂರ್ಣಾಙ್ಕ x, y;
	ಪೂರ್ಣಾಙ್ಕ width, height;
	ಪೂರ್ಣಾಙ್ಕ padding_ಎಡ, padding_ತುದಿ, padding_width, padding_height;
	ಪೂರ್ಣಾಙ್ಕ border_ಎಡ, border_ತುದಿ, border_ಬಲ, border_bottom;
	ಸಂರಚನಾ rect r;
	ಸಂರಚನಾ rect rect;
	ಪೂರ್ಣಾಙ್ಕ x_scrolled, y_scrolled;
	ಸಂರಚನಾ box *bg_box = ರಿಕ್ತ;
	css_computed_clip_rect css_rect;
	ಗಣನಾ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ಗಣನಾ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	ಯದಿ (html_redraw_printing && (box->flags & PRINTED))
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (box->style != ರಿಕ್ತ) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	ಯದಿ (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_ಎಡ = box->padding[ಎಡ_2];
		padding_ತುದಿ = box->padding[ತುದಿ_2];
		padding_width = padding_ಎಡ + box->width + box->padding[ಬಲ_2];
		padding_height = padding_ತುದಿ + box->height +
				box->padding[BOTTOM];
		border_ಎಡ = box->border[ಎಡ_2].width;
		border_ತುದಿ = box->border[ತುದಿ_2].width;
		border_ಬಲ = box->border[ಬಲ_2].width;
		border_bottom = box->border[BOTTOM].width;
	} ಅನ್ಯಥಾ {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ಎಡ = box->padding[ಎಡ_2] ? box->padding[ಎಡ_2] * scale
				: 0;
		padding_ತುದಿ = box->padding[ತುದಿ_2] ? box->padding[ತುದಿ_2] * scale
				: 0;
		padding_width = (box->padding[ಎಡ_2] + box->width +
				box->padding[ಬಲ_2]) * scale;
		padding_height = (box->padding[ತುದಿ_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_ಎಡ = box->border[ಎಡ_2].width * scale;
		border_ತುದಿ = box->border[ತುದಿ_2].width * scale;
		border_ಬಲ = box->border[ಬಲ_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ಯದಿ (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != ರಿಕ್ತ) {
		/* box contents clipped to box size */
		r.x0 = x - border_ಎಡ;
		r.x1 = x + padding_width + border_ಬಲ;
	} ಅನ್ಯಥಾ {
		/* box contents can hang out of the box; use descendant box */
		ಯದಿ (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} ಅನ್ಯಥಾ {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ಯದಿ (!box->parent) {
			/* root element */
			ಪೂರ್ಣಾಙ್ಕ margin_ಎಡ, margin_ಬಲ;
			ಯದಿ (scale == 1.0) {
				margin_ಎಡ = box->margin[ಎಡ_2];
				margin_ಬಲ = box->margin[ಬಲ_2];
			} ಅನ್ಯಥಾ {
				margin_ಎಡ = box->margin[ಎಡ_2] * scale;
				margin_ಬಲ = box->margin[ಬಲ_2] * scale;
			}
			r.x0 = x - border_ಎಡ - margin_ಎಡ < r.x0 ?
					x - border_ಎಡ - margin_ಎಡ : r.x0;
			r.x1 = x + padding_width + border_ಬಲ +
					margin_ಬಲ > r.x1 ?
					x + padding_width + border_ಬಲ +
					margin_ಬಲ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ಯದಿ (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != ರಿಕ್ತ) {
		/* box contents clipped to box size */
		r.y0 = y - border_ತುದಿ;
		r.y1 = y + padding_height + border_bottom;
	} ಅನ್ಯಥಾ {
		/* box contents can hang out of the box; use descendant box */
		ಯದಿ (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} ಅನ್ಯಥಾ {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ಯದಿ (!box->parent) {
			/* root element */
			ಪೂರ್ಣಾಙ್ಕ margin_ತುದಿ, margin_bottom;
			ಯದಿ (scale == 1.0) {
				margin_ತುದಿ = box->margin[ತುದಿ_2];
				margin_bottom = box->margin[BOTTOM];
			} ಅನ್ಯಥಾ {
				margin_ತುದಿ = box->margin[ತುದಿ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_ತುದಿ - margin_ತುದಿ < r.y0 ?
					y - border_ತುದಿ - margin_ತುದಿ : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ಯದಿ (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		ಪ್ರತ್ಯಾಗಮ true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ಯದಿ (html_redraw_printing) {
		ಯದಿ (r.y1 > html_redraw_printing_border) {
			ಯದಿ (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ಯದಿ (r.y0 < html_redraw_printing_ತುದಿ_cropped)
					html_redraw_printing_ತುದಿ_cropped = r.y0;
				ಪ್ರತ್ಯಾಗಮ true;
			}
		}
		ಅನ್ಯಥಾ box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ಯದಿ (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ಯದಿ ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			ಪ್ರತ್ಯಾಗಮ false;
		ಯದಿ (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
		ಪ್ರತ್ಯಾಗಮ ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	ಯದಿ ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (box->style != ರಿಕ್ತ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ಯದಿ (css_rect.ಎಡ_auto == false)
			r.x0 = x - border_ಎಡ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ಎಡ, css_rect.lunit));

		ಯದಿ (css_rect.ತುದಿ_auto == false)
			r.y0 = y - border_ತುದಿ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ತುದಿ, css_rect.tunit));

		ಯದಿ (css_rect.ಬಲ_auto == false)
			r.x1 = x - border_ಎಡ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ಬಲ, css_rect.runit));

		ಯದಿ (css_rect.bottom_auto == false)
			r.y1 = y - border_ತುದಿ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ಯದಿ (r.x0 < clip->x0) r.x0 = clip->x0;
		ಯದಿ (r.y0 < clip->y0) r.y0 = clip->y0;
		ಯದಿ (clip->x1 < r.x1) r.x1 = clip->x1;
		ಯದಿ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ಯದಿ (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			ಪ್ರತ್ಯಾಗಮ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ ಯದಿ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ಯದಿ (r.x0 < clip->x0) r.x0 = clip->x0;
		ಯದಿ (r.y0 < clip->y0) r.y0 = clip->y0;
		ಯದಿ (clip->x1 < r.x1) r.x1 = clip->x1;
		ಯದಿ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ಯದಿ (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			ಪ್ರತ್ಯಾಗಮ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ {
		/* clip box is fine, clip to it */
		r = *clip;
		ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ಯದಿ (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ರಿಕ್ತ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ಸಂರಚನಾ rect p;
		p.x0 = x - border_ಎಡ < r.x0 ? r.x0 : x - border_ಎಡ;
		p.y0 = y - border_ತುದಿ < r.y0 ? r.y0 : y - border_ತುದಿ;
		p.x1 = x + padding_width + border_ಬಲ < r.x1 ?
				x + padding_width + border_ಬಲ : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		ಯದಿ (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			ಪೂರ್ಣಾಙ್ಕ m_ಎಡ, m_ತುದಿ, m_ಬಲ, m_bottom;
			ಯದಿ (scale == 1.0) {
				m_ಎಡ = box->margin[ಎಡ_2];
				m_ತುದಿ = box->margin[ತುದಿ_2];
				m_ಬಲ = box->margin[ಬಲ_2];
				m_bottom = box->margin[BOTTOM];
			} ಅನ್ಯಥಾ {
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
		ಯದಿ ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ಯದಿ (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				ಪ್ರತ್ಯಾಗಮ false;
			/* restore previous graphics window */
			ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* borders for block level content and replaced inlines */
	ಯದಿ (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ರಿಕ್ತ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_ತುದಿ || border_ಬಲ || border_bottom || border_ಎಡ)) {
		ಯದಿ (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ಯದಿ (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_ತುದಿ || border_ಬಲ ||
			border_bottom || border_ಎಡ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ಸಂರಚನಾ box *ib;
		ಸಂರಚನಾ rect b; /* border edge rectangle */
		ಸಂರಚನಾ rect p; /* clipped rect */
		bool first = true;
		ಪೂರ್ಣಾಙ್ಕ ib_x;
		ಪೂರ್ಣಾಙ್ಕ ib_y = y;
		ಪೂರ್ಣಾಙ್ಕ ib_p_width;
		ಪೂರ್ಣಾಙ್ಕ ib_b_ಎಡ, ib_b_ಬಲ;

		b.x0 = x - border_ಎಡ;
		b.x1 = x + padding_width + border_ಬಲ;
		b.y0 = y - border_ತುದಿ;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		ಪ್ರತಿ (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ಯದಿ (ib->type == BOX_FLOAT_ಎಡ ||
					ib->type == BOX_FLOAT_ಬಲ)
				ಅನುವರ್ತ;
			ಯದಿ (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[ಎಡ_2] + ib->width +
						ib->padding[ಬಲ_2];
				ib_b_ಎಡ = ib->border[ಎಡ_2].width;
				ib_b_ಬಲ = ib->border[ಬಲ_2].width;
			} ಅನ್ಯಥಾ {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[ಎಡ_2] + ib->width +
						ib->padding[ಬಲ_2]) * scale;
				ib_b_ಎಡ = ib->border[ಎಡ_2].width * scale;
				ib_b_ಬಲ = ib->border[ಬಲ_2].width * scale;
			}

			ಯದಿ ((ib->flags & NEW_ಸಾಲು) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ಯದಿ (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					ಪ್ರತ್ಯಾಗಮ false;
				/* restore previous graphics window */
				ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					ಪ್ರತ್ಯಾಗಮ false;
				ಯದಿ (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					ಪ್ರತ್ಯಾಗಮ false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ಎಡ;
				b.y0 = ib_y - border_ತುದಿ - padding_ತುದಿ;
				b.y1 = ib_y + padding_height - padding_ತುದಿ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_ಬಲ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ಯದಿ (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				ಭಙ್ಗ;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ಯದಿ (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
		/* restore previous graphics window */
		ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
		ಯದಿ (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	}

	/* Debug outlines */
	ಯದಿ (html_redraw_debug) {
		ಪೂರ್ಣಾಙ್ಕ margin_ಎಡ, margin_ಬಲ;
		ಪೂರ್ಣಾಙ್ಕ margin_ತುದಿ, margin_bottom;
		ಯದಿ (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ಎಡ = box->margin[ಎಡ_2];
			margin_ತುದಿ = box->margin[ತುದಿ_2];
			margin_ಬಲ = box->margin[ಬಲ_2];
			margin_bottom = box->margin[BOTTOM];
		} ಅನ್ಯಥಾ {
			margin_ಎಡ = box->margin[ಎಡ_2] * scale;
			margin_ತುದಿ = box->margin[ತುದಿ_2] * scale;
			margin_ಬಲ = box->margin[ಬಲ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ಎಡ;
		rect.y0 = y + padding_ತುದಿ;
		rect.x1 = x + padding_ಎಡ + width;
		rect.y1 = y + padding_ತುದಿ + height;
		ಯದಿ (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		ಯದಿ (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ಎಡ - margin_ಎಡ;
		rect.y0 = y - border_ತುದಿ - margin_ತುದಿ;
		rect.x1 = x + padding_width + border_ಬಲ + margin_ಬಲ;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		ಯದಿ (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ಯದಿ (box->parent != ರಿಕ್ತ) {
		bool need_clip = false;
		ಯದಿ (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			ಯದಿ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಯದಿ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಯದಿ (clip->x1 < r.x1) r.x1 = clip->x1;
			ಯದಿ (clip->y1 < r.y1) r.y1 = clip->y1;
			ಯದಿ (r.x1 <= r.x0 || r.y1 <= r.y0) {
				ಪ್ರತ್ಯಾಗಮ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ಅನ್ಯಥಾ ಯದಿ (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			ಯದಿ (r.x0 < clip->x0) r.x0 = clip->x0;
			ಯದಿ (clip->x1 < r.x1) r.x1 = clip->x1;
			ಯದಿ (r.x1 <= r.x0) {
				ಪ್ರತ್ಯಾಗಮ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ಅನ್ಯಥಾ ಯದಿ (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			ಯದಿ (r.y0 < clip->y0) r.y0 = clip->y0;
			ಯದಿ (clip->y1 < r.y1) r.y1 = clip->y1;
			ಯದಿ (r.y1 <= r.y0) {
				ಪ್ರತ್ಯಾಗಮ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ಯದಿ (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			ಯದಿ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	/* text decoration */
	ಯದಿ ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		ಯದಿ (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (box->ಗಂಟು != ರಿಕ್ತ) {
		exc = dom_html_element_get_tag_type(box->ಗಂಟು, &tag_type);
		ಯದಿ (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} ಅನ್ಯಥಾ {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ಯದಿ (box->object && width != 0 && height != 0) {
		ಸಂರಚನಾ content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_ಎಡ;
		obj_data.y = y_scrolled + padding_ತುದಿ;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		ಯದಿ (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		ಯದಿ (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ಧ್ರುವ ವರ್ಣ *obj = "\xef\xbf\xbc";
			ಪೂರ್ಣಾಙ್ಕ obj_width;
			ಪೂರ್ಣಾಙ್ಕ obj_x = x + padding_ಎಡ;
			nserror res;

			rect.x0 = x + padding_ಎಡ;
			rect.y0 = y + padding_ತುದಿ;
			rect.x1 = x + padding_ಎಡ + width - 1;
			rect.y1 = y + padding_ತುದಿ + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ಯದಿ (res != NSERROR_OK) {
				ಪ್ರತ್ಯಾಗಮ false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  ಪರಿಮಾಣ(obj) - 1,
						  &obj_width);
			ಯದಿ (res != NSERROR_OK) {
				obj_x += 1;
			} ಅನ್ಯಥಾ {
				obj_x += width / 2 - obj_width / 2;
			}

			ಯದಿ (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_ತುದಿ + (ಪೂರ್ಣಾಙ್ಕ)(height * 0.75),
					    obj, ಪರಿಮಾಣ(obj) - 1) != NSERROR_OK)
				ಪ್ರತ್ಯಾಗಮ false;
		}
	} ಅನ್ಯಥಾ ಯದಿ (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ಗಂಟು != ರಿಕ್ತ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ಸಂರಚನಾ bitmap *bitmap = ರಿಕ್ತ;
		exc = dom_ಗಂಟು_get_user_data(box->ಗಂಟು,
					     corestring_dom_ns_ಕೀಲಿ_canvas_ಗಂಟು_data,
					     &bitmap);
		ಯದಿ (exc != DOM_NO_ERR) {
			bitmap = ರಿಕ್ತ;
		}
		ಯದಿ (bitmap != ರಿಕ್ತ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ಎಡ, y + padding_ತುದಿ,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ ಯದಿ (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_ಎಡ,
				y + padding_ತುದಿ, &r, ctx);

	} ಅನ್ಯಥಾ ಯದಿ (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ಯದಿ (!html_redraw_checkbox(x + padding_ಎಡ, y + padding_ತುದಿ,
				width, height, box->gadget->selected, ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ ಯದಿ (box->gadget && box->gadget->type == GADGET_RADIO) {
		ಯದಿ (!html_redraw_radio(x + padding_ಎಡ, y + padding_ತುದಿ,
				width, height, box->gadget->selected, ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ ಯದಿ (box->gadget && box->gadget->type == GADGET_FILE) {
		ಯದಿ (!html_redraw_file(x + padding_ಎಡ, y + padding_ತುದಿ,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ ಯದಿ (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} ಅನ್ಯಥಾ ಯದಿ (box->text) {
		ಯದಿ (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			ಪ್ರತ್ಯಾಗಮ false;

	} ಅನ್ಯಥಾ {
		ಯದಿ (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		ಯದಿ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;

	/* list marker */
	ಯದಿ (box->list_marker) {
		ಯದಿ (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			ಪ್ರತ್ಯಾಗಮ false;
	}

	/* scrollbars */
	ಯದಿ (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == ರಿಕ್ತ || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != ರಿಕ್ತ) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ಸಂರಚನಾ content *)html,
					    box, has_x_scroll, has_y_scroll);
		ಯದಿ (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (box->scroll_x != ರಿಕ್ತ)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[ತುದಿ_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		ಯದಿ (box->scroll_y != ರಿಕ್ತ)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[ಎಡ_2] +
					box->width + box->padding[ಬಲ_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	ಯದಿ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		ಯದಿ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(ಸಂರಚನಾ content *c, ಸಂರಚನಾ content_redraw_data *data,
		ಧ್ರುವ ಸಂರಚನಾ rect *clip, ಧ್ರುವ ಸಂರಚನಾ redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ಸಂರಚನಾ box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	ಪ್ರತಿಜ್ಞಾ(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ಯದಿ (ctx->interactive && html->visible_select_menu != ರಿಕ್ತ) {
		ಸಂರಚನಾ form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	ಯದಿ (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ಯದಿ (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	ಯದಿ (select) {
		ಪೂರ್ಣಾಙ್ಕ menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ಎಡ_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[ತುದಿ_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	ಪ್ರತ್ಯಾಗಮ result;

}
