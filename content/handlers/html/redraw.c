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

#ଅନ୍ତର୍ଭାଵ "utils/config.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/browser_window.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plotters.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/bitmap.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/layout.h"
#ଅନ୍ତର୍ଭାଵ "content/content.h"
#ଅନ୍ତର୍ଭାଵ "content/content_protected.h"
#ଅନ୍ତର୍ଭାଵ "content/textsearch.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"
#ଅନ୍ତର୍ଭାଵ "desktop/selection.h"
#ଅନ୍ତର୍ଭାଵ "desktop/print.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"
#ଅନ୍ତର୍ଭାଵ "desktop/textarea.h"
#ଅନ୍ତର୍ଭାଵ "desktop/gui_internal.h"

#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"
#ଅନ୍ତର୍ଭାଵ "html/box_manipulate.h"
#ଅନ୍ତର୍ଭାଵ "html/font.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ସ୍ଥିର bool html_redraw_box_has_background(ସଂରଚନା box *box)
{
	ଯଦି (box->background != ଖାଲି)
		ଫେରନ୍ତୁ true;

	ଯଦି (box->style != ଖାଲି) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		ଯଦି (nscss_color_is_transparent(colour) == false)
			ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ସ୍ଥିର ସଂରଚନା box *html_redraw_find_bg_box(ସଂରଚନା box *box)
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
	ଯଦି (box->parent == ଖାଲି) {
		/* Root box */
		ଯଦି (html_redraw_box_has_background(box))
			ଫେରନ୍ତୁ box;

		/* No background on root box: consider body box, if any */
		ଯଦି (box->children != ଖାଲି) {
			ଯଦି (html_redraw_box_has_background(box->children))
				ଫେରନ୍ତୁ box->children;
		}
	} ଅନ୍ୟଥା ଯଦି (box->parent != ଖାଲି && box->parent->parent == ଖାଲି) {
		/* Body box: only render background if root has its own */
		ଯଦି (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			ଫେରନ୍ତୁ box;
	} ଅନ୍ୟଥା {
		/* Any other box */
		ଯଦି (html_redraw_box_has_background(box))
			ଫେରନ୍ତୁ box;
	}

	ଫେରନ୍ତୁ ଖାଲି;
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

ସ୍ଥିର bool
text_redraw(ଅଚଳ ଅକ୍ଷର *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା space,
	    ଅଚଳ plot_font_style_t *fstyle,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
	    ଅଚଳ ସଂରଚନା rect *clip,
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
	    ଦଶମିକ scale,
	    bool excluded,
	    ସଂରଚନା content *c,
	    ଅଚଳ ସଂରଚନା selection *sel,
	    ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ଯଦି (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ଯଦି (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ଯଦି (!highlighted &&
		    (c->textsearch.context != ଖାଲି) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ଯଦି (highlighted) {
			ସଂରଚନା rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ଯଦି (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ପ୍ରତିଜ୍ଞା(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			ଯଦି (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			ଯଦି (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ଯଦି (end_idx > utf8_len) {
					endx += space;
			}

			ଯଦି (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ଯଦି ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				ଫେରନ୍ତୁ false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (start_idx > 0) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px0 = max(x + startx, clip->x0);
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px1 = min(x + endx, clip->x1);

				ଯଦି (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}

					clip_changed = true;
				} ଅନ୍ୟଥା {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ଯଦି (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ଫେରନ୍ତୁ false;
			}

			/* draw any text succeeding highlighted portion */
			ଯଦି (endtxt_idx < utf8_len) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା px0 = max(x + endx, clip->x0);
				ଯଦି (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ଯଦି (res != NSERROR_OK) {
						ଫେରନ୍ତୁ false;
					}
				}
			}

			ଯଦି (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ଫେରନ୍ତୁ false;
			}
		}
	}

	ଯଦି (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}
	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_checkbox(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
		bool selected, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ଦ୍ୱିଗୁଣ z;
	nserror res;
	ସଂରଚନା rect rect;

	z = width * 0.15;
	ଯଦି (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_darkwbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_darkwbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_lightwbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_lightwbasec, &rect);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (selected) {
		ଯଦି (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		} ଅନ୍ୟଥା {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_wblobc, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->ଧାଡ଼ି(ctx, plot_style_stroke_wblobc, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		}
	}
	ଫେରନ୍ତୁ true;
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
ସ୍ଥିର bool html_redraw_radio(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
		bool selected, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}

	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_file(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, ପୂର୍ଣ୍ଣସଂଖ୍ୟା height,
		ସଂରଚନା box *box, ଦଶମିକ scale, colour background_colour,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା text_width;
	ଅଚଳ ଅକ୍ଷର *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	ଯଦି (box->gadget->ମୂଲ୍ୟ) {
		text = box->gadget->ମୂଲ୍ୟ;
	} ଅନ୍ୟଥା {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}
	text_width *= scale;
	ଯଦି (width < text_width + 8) {
		x = x + width - text_width - 4;
	} ଅନ୍ୟଥା {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	ଯଦି (res != NSERROR_OK) {
		ଫେରନ୍ତୁ false;
	}
	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_background(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ସଂରଚନା box *box, ଦଶମିକ scale,
		ଅଚଳ ସଂରଚନା rect *clip, colour *background_colour,
		ସଂରଚନା box *background,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	ସଂରଚନା box *clip_box = box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ox = x, oy = y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ସଂରଚନା box *parent;
	ସଂରଚନା rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	ଯଦି (ctx->background_images == false)
		ଫେରନ୍ତୁ true;

	plot_content = (background->background != ଖାଲି);

	ଯଦି (plot_content) {
		ଯଦି (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ବାମ_2] * scale;
			y -= box->margin[ଶୀର୍ଷ_2] * scale;
			width = box->margin[ବାମ_2] + box->padding[ବାମ_2] +
					box->width + box->padding[ଡାହାଣ_2] +
					box->margin[ଡାହାଣ_2];
			height = box->margin[ଶୀର୍ଷ_2] + box->padding[ଶୀର୍ଷ_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} ଅନ୍ୟଥା {
			width = box->padding[ବାମ_2] + box->width +
					box->padding[ଡାହାଣ_2];
			height = box->padding[ଶୀର୍ଷ_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		ବାଛ (css_computed_background_repeat(background->style)) {
		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ଯଦି (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} ଅନ୍ୟଥା {
			x += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ଯଦି (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} ଅନ୍ୟଥା {
			y += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ଯଦି (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		ପାଇଁ (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		ପ୍ରତିଜ୍ଞା(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		ଯଦି (clip_to_children)
			clip_box = box->children;
	}

	ପାଇଁ (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		ଯଦି (clip_to_children) {
			ପ୍ରତିଜ୍ଞା(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ବାମ_2] +
					clip_box->width +
					clip_box->padding[ଡାହାଣ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[ଶୀର୍ଷ_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			ଯଦି (r.x0 < clip->x0) r.x0 = clip->x0;
			ଯଦି (r.y0 < clip->y0) r.y0 = clip->y0;
			ଯଦି (r.x1 > clip->x1) r.x1 = clip->x1;
			ଯଦି (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ଯଦି ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ଖାଲି) &&
			     content_get_opaque(clip_box->background)))
				ଜାରି;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ଯଦି (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			ଯଦି (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ଯଦି (res != NSERROR_OK) {
					ଫେରନ୍ତୁ false;
				}
			}
		}
		/* and plot the image */
		ଯଦି (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			ଯଦି (!repeat_x) {
				ଯଦି (r.x0 < x)
					r.x0 = x;
				ଯଦି (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			ଯଦି (!repeat_y) {
				ଯଦି (r.y0 < y)
					r.y0 = y;
				ଯଦି (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			ଯଦି ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ସଂରଚନା content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				ଯଦି (res != NSERROR_OK) {
					ଫେରନ୍ତୁ false;
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
		ଯଦି (!clip_to_children)
			ଫେରନ୍ତୁ true;
	}
	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_inline_background(ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ସଂରଚନା box *box,
		ଦଶମିକ scale, ଅଚଳ ସଂରଚନା rect *clip, ସଂରଚନା rect b,
		bool first, bool last, colour *background_colour,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା rect r = *clip;
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

	plot_content = (box->background != ଖାଲି);

	ଯଦି (html_redraw_printing && nsoption_bool(remove_backgrounds))
		ଫେରନ୍ତୁ true;

	ଯଦି (plot_content) {
		/* handle background-repeat */
		ବାଛ (css_computed_background_repeat(box->style)) {
		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଭାଙ୍ଗ;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ଯଦି (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ଯଦି (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} ଅନ୍ୟଥା {
			x += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ଯଦି (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} ଅନ୍ୟଥା {
			y += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ଯଦି (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		ଯଦି (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		}
	}
	/* and plot the image */
	ଯଦି (plot_content) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = content_get_width(box->background);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା height = content_get_height(box->background);

		ଯଦି (!repeat_x) {
			ଯଦି (r.x0 < x)
				r.x0 = x;
			ଯଦି (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		ଯଦି (!repeat_y) {
			ଯଦି (r.y0 < y)
				r.y0 = y;
			ଯଦି (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		ଯଦି ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ସଂରଚନା content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
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

	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool
html_redraw_text_decoration_inline(ସଂରଚନା box *box,
				   ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
				   ଦଶମିକ scale,
				   colour colour,
				   ଦଶମିକ ratio,
				   ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ସଂରଚନା rect rect;

	ପାଇଁ (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		ଯଦି (c->type != BOX_TEXT) {
			ଜାରି;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->ଧାଡ଼ି(ctx, &plot_style_box, &rect);
		ଯଦି (res != NSERROR_OK) {
			ଫେରନ୍ତୁ false;
		}
	}
	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool
html_redraw_text_decoration_block(ସଂରଚନା box *box,
				  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
				  ଦଶମିକ scale,
				  colour colour,
				  ଦଶମିକ ratio,
				  ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ସଂରଚନା rect rect;

	/* draw through text descendants */
	ପାଇଁ (c = box->children; c; c = c->next) {
		ଯଦି (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->ଧାଡ଼ି(ctx, &plot_style_box, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}
		} ଅନ୍ୟଥା ଯଦି ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			ଯଦି (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				ଫେରନ୍ତୁ false;
		}
	}
	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_text_decoration(ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent, ଦଶମିକ scale,
		colour background_colour, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସ୍ଥିର ଅଚଳ ଗଣନା css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_ଧାଡ଼ି_THROUGH };
	ସ୍ଥିର ଅଚଳ ଦଶମିକ ଧାଡ଼ି_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	ଯଦି (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	ଯଦି (box->type == BOX_INLINE) {
		ଯଦି (!box->inline_end)
			ଫେରନ୍ତୁ true;
		ପାଇଁ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ଯଦି (css_computed_text_decoration(box->style) &
					decoration[i])
				ଯଦି (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, ଧାଡ଼ି_ratio[i], ctx))
					ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା {
		ପାଇଁ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ଯଦି (css_computed_text_decoration(box->style) &
					decoration[i])
				ଯଦି (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, ଧାଡ଼ି_ratio[i], ctx))
					ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
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

ସ୍ଥିର bool html_redraw_text_box(ଅଚଳ html_content *html, ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y, ଅଚଳ ସଂରଚନା rect *clip, ଦଶମିକ scale,
		colour current_background_color,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	bool excluded = (box->object != ଖାଲି);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ଯଦି (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (ସଂରଚନା content *)html,
			 html->sel,
			 ctx))
		ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ true;
}

bool html_redraw_box(ଅଚଳ html_content *html, ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent,
		ଅଚଳ ସଂରଚନା rect *clip, ଦଶମିକ scale,
		colour current_background_color,
		ଅଚଳ ସଂରଚନା redraw_context *ctx);

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

ସ୍ଥିର bool html_redraw_box_children(ଅଚଳ html_content *html, ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent,
		ଅଚଳ ସଂରଚନା rect *clip, ଦଶମିକ scale,
		colour current_background_color,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ସଂରଚନା box *c;

	ପାଇଁ (c = box->children; c; c = c->next) {

		ଯଦି (c->type != BOX_FLOAT_ବାମ && c->type != BOX_FLOAT_ଡାହାଣ)
			ଯଦି (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				ଫେରନ୍ତୁ false;
	}
	ପାଇଁ (c = box->float_children; c; c = c->next_float)
		ଯଦି (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ true;
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

bool html_redraw_box(ଅଚଳ html_content *html, ସଂରଚନା box *box,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_parent, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y_parent,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ଦଶମିକ scale,
		colour current_background_color,
		ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	ଅଚଳ ସଂରଚନା plotter_table *plot = ctx->plot;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, y;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା padding_ବାମ, padding_ଶୀର୍ଷ, padding_width, padding_height;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା border_ବାମ, border_ଶୀର୍ଷ, border_ଡାହାଣ, border_bottom;
	ସଂରଚନା rect r;
	ସଂରଚନା rect rect;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_scrolled, y_scrolled;
	ସଂରଚନା box *bg_box = ଖାଲି;
	css_computed_clip_rect css_rect;
	ଗଣନା css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ଗଣନା css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	ଯଦି (html_redraw_printing && (box->flags & PRINTED))
		ଫେରନ୍ତୁ true;

	ଯଦି (box->style != ଖାଲି) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	ଯଦି (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_ବାମ = box->padding[ବାମ_2];
		padding_ଶୀର୍ଷ = box->padding[ଶୀର୍ଷ_2];
		padding_width = padding_ବାମ + box->width + box->padding[ଡାହାଣ_2];
		padding_height = padding_ଶୀର୍ଷ + box->height +
				box->padding[BOTTOM];
		border_ବାମ = box->border[ବାମ_2].width;
		border_ଶୀର୍ଷ = box->border[ଶୀର୍ଷ_2].width;
		border_ଡାହାଣ = box->border[ଡାହାଣ_2].width;
		border_bottom = box->border[BOTTOM].width;
	} ଅନ୍ୟଥା {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ବାମ = box->padding[ବାମ_2] ? box->padding[ବାମ_2] * scale
				: 0;
		padding_ଶୀର୍ଷ = box->padding[ଶୀର୍ଷ_2] ? box->padding[ଶୀର୍ଷ_2] * scale
				: 0;
		padding_width = (box->padding[ବାମ_2] + box->width +
				box->padding[ଡାହାଣ_2]) * scale;
		padding_height = (box->padding[ଶୀର୍ଷ_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_ବାମ = box->border[ବାମ_2].width * scale;
		border_ଶୀର୍ଷ = box->border[ଶୀର୍ଷ_2].width * scale;
		border_ଡାହାଣ = box->border[ଡାହାଣ_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ଯଦି (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != ଖାଲି) {
		/* box contents clipped to box size */
		r.x0 = x - border_ବାମ;
		r.x1 = x + padding_width + border_ଡାହାଣ;
	} ଅନ୍ୟଥା {
		/* box contents can hang out of the box; use descendant box */
		ଯଦି (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} ଅନ୍ୟଥା {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ଯଦି (!box->parent) {
			/* root element */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ବାମ, margin_ଡାହାଣ;
			ଯଦି (scale == 1.0) {
				margin_ବାମ = box->margin[ବାମ_2];
				margin_ଡାହାଣ = box->margin[ଡାହାଣ_2];
			} ଅନ୍ୟଥା {
				margin_ବାମ = box->margin[ବାମ_2] * scale;
				margin_ଡାହାଣ = box->margin[ଡାହାଣ_2] * scale;
			}
			r.x0 = x - border_ବାମ - margin_ବାମ < r.x0 ?
					x - border_ବାମ - margin_ବାମ : r.x0;
			r.x1 = x + padding_width + border_ଡାହାଣ +
					margin_ଡାହାଣ > r.x1 ?
					x + padding_width + border_ଡାହାଣ +
					margin_ଡାହାଣ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ଯଦି (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != ଖାଲି) {
		/* box contents clipped to box size */
		r.y0 = y - border_ଶୀର୍ଷ;
		r.y1 = y + padding_height + border_bottom;
	} ଅନ୍ୟଥା {
		/* box contents can hang out of the box; use descendant box */
		ଯଦି (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} ଅନ୍ୟଥା {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ଯଦି (!box->parent) {
			/* root element */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ଶୀର୍ଷ, margin_bottom;
			ଯଦି (scale == 1.0) {
				margin_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2];
				margin_bottom = box->margin[BOTTOM];
			} ଅନ୍ୟଥା {
				margin_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_ଶୀର୍ଷ - margin_ଶୀର୍ଷ < r.y0 ?
					y - border_ଶୀର୍ଷ - margin_ଶୀର୍ଷ : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ଯଦି (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		ଫେରନ୍ତୁ true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ଯଦି (html_redraw_printing) {
		ଯଦି (r.y1 > html_redraw_printing_border) {
			ଯଦି (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ଯଦି (r.y0 < html_redraw_printing_ଶୀର୍ଷ_cropped)
					html_redraw_printing_ଶୀର୍ଷ_cropped = r.y0;
				ଫେରନ୍ତୁ true;
			}
		}
		ଅନ୍ୟଥା box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ଯଦି (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ଯଦି ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			ଫେରନ୍ତୁ false;
		ଯଦି (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			ଫେରନ୍ତୁ false;
		ଫେରନ୍ତୁ ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	ଯଦି ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (box->style != ଖାଲି &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ଯଦି (css_rect.ବାମ_auto == false)
			r.x0 = x - border_ବାମ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ବାମ, css_rect.lunit));

		ଯଦି (css_rect.ଶୀର୍ଷ_auto == false)
			r.y0 = y - border_ଶୀର୍ଷ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ଶୀର୍ଷ, css_rect.tunit));

		ଯଦି (css_rect.ଡାହାଣ_auto == false)
			r.x1 = x - border_ବାମ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ଡାହାଣ, css_rect.runit));

		ଯଦି (css_rect.bottom_auto == false)
			r.y1 = y - border_ଶୀର୍ଷ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ଯଦି (r.x0 < clip->x0) r.x0 = clip->x0;
		ଯଦି (r.y0 < clip->y0) r.y0 = clip->y0;
		ଯଦି (clip->x1 < r.x1) r.x1 = clip->x1;
		ଯଦି (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ଯଦି (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			ଫେରନ୍ତୁ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା ଯଦି (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ଯଦି (r.x0 < clip->x0) r.x0 = clip->x0;
		ଯଦି (r.y0 < clip->y0) r.y0 = clip->y0;
		ଯଦି (clip->x1 < r.x1) r.x1 = clip->x1;
		ଯଦି (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ଯଦି (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			ଫେରନ୍ତୁ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା {
		/* clip box is fine, clip to it */
		r = *clip;
		ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ଯଦି (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ଖାଲି &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ସଂରଚନା rect p;
		p.x0 = x - border_ବାମ < r.x0 ? r.x0 : x - border_ବାମ;
		p.y0 = y - border_ଶୀର୍ଷ < r.y0 ? r.y0 : y - border_ଶୀର୍ଷ;
		p.x1 = x + padding_width + border_ଡାହାଣ < r.x1 ?
				x + padding_width + border_ଡାହାଣ : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		ଯଦି (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା m_ବାମ, m_ଶୀର୍ଷ, m_ଡାହାଣ, m_bottom;
			ଯଦି (scale == 1.0) {
				m_ବାମ = box->margin[ବାମ_2];
				m_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2];
				m_ଡାହାଣ = box->margin[ଡାହାଣ_2];
				m_bottom = box->margin[BOTTOM];
			} ଅନ୍ୟଥା {
				m_ବାମ = box->margin[ବାମ_2] * scale;
				m_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2] * scale;
				m_ଡାହାଣ = box->margin[ଡାହାଣ_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ବାମ < r.x0 ? r.x0 : p.x0 - m_ବାମ;
			p.y0 = p.y0 - m_ଶୀର୍ଷ < r.y0 ? r.y0 : p.y0 - m_ଶୀର୍ଷ;
			p.x1 = p.x1 + m_ଡାହାଣ < r.x1 ? p.x1 + m_ଡାହାଣ : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		ଯଦି ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ଯଦି (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				ଫେରନ୍ତୁ false;
			/* restore previous graphics window */
			ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ଫେରନ୍ତୁ false;
		}
	}

	/* borders for block level content and replaced inlines */
	ଯଦି (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ଖାଲି &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_ଶୀର୍ଷ || border_ଡାହାଣ || border_bottom || border_ବାମ)) {
		ଯଦି (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			ଫେରନ୍ତୁ false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ଯଦି (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_ଶୀର୍ଷ || border_ଡାହାଣ ||
			border_bottom || border_ବାମ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ସଂରଚନା box *ib;
		ସଂରଚନା rect b; /* border edge rectangle */
		ସଂରଚନା rect p; /* clipped rect */
		bool first = true;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ib_x;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ib_y = y;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ib_p_width;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ib_b_ବାମ, ib_b_ଡାହାଣ;

		b.x0 = x - border_ବାମ;
		b.x1 = x + padding_width + border_ଡାହାଣ;
		b.y0 = y - border_ଶୀର୍ଷ;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		ପାଇଁ (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ଯଦି (ib->type == BOX_FLOAT_ବାମ ||
					ib->type == BOX_FLOAT_ଡାହାଣ)
				ଜାରି;
			ଯଦି (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[ବାମ_2] + ib->width +
						ib->padding[ଡାହାଣ_2];
				ib_b_ବାମ = ib->border[ବାମ_2].width;
				ib_b_ଡାହାଣ = ib->border[ଡାହାଣ_2].width;
			} ଅନ୍ୟଥା {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[ବାମ_2] + ib->width +
						ib->padding[ଡାହାଣ_2]) * scale;
				ib_b_ବାମ = ib->border[ବାମ_2].width * scale;
				ib_b_ଡାହାଣ = ib->border[ଡାହାଣ_2].width * scale;
			}

			ଯଦି ((ib->flags & NEW_ଧାଡ଼ି) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ଯଦି (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					ଫେରନ୍ତୁ false;
				/* restore previous graphics window */
				ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					ଫେରନ୍ତୁ false;
				ଯଦି (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					ଫେରନ୍ତୁ false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ବାମ;
				b.y0 = ib_y - border_ଶୀର୍ଷ - padding_ଶୀର୍ଷ;
				b.y1 = ib_y + padding_height - padding_ଶୀର୍ଷ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_ଡାହାଣ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ଯଦି (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				ଭାଙ୍ଗ;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ଯଦି (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			ଫେରନ୍ତୁ false;
		/* restore previous graphics window */
		ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
		ଯଦି (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			ଫେରନ୍ତୁ false;

	}

	/* Debug outlines */
	ଯଦି (html_redraw_debug) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ବାମ, margin_ଡାହାଣ;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା margin_ଶୀର୍ଷ, margin_bottom;
		ଯଦି (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ବାମ = box->margin[ବାମ_2];
			margin_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2];
			margin_ଡାହାଣ = box->margin[ଡାହାଣ_2];
			margin_bottom = box->margin[BOTTOM];
		} ଅନ୍ୟଥା {
			margin_ବାମ = box->margin[ବାମ_2] * scale;
			margin_ଶୀର୍ଷ = box->margin[ଶୀର୍ଷ_2] * scale;
			margin_ଡାହାଣ = box->margin[ଡାହାଣ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ବାମ;
		rect.y0 = y + padding_ଶୀର୍ଷ;
		rect.x1 = x + padding_ବାମ + width;
		rect.y1 = y + padding_ଶୀର୍ଷ + height;
		ଯଦି (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			ଫେରନ୍ତୁ false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		ଯଦି (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			ଫେରନ୍ତୁ false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ବାମ - margin_ବାମ;
		rect.y0 = y - border_ଶୀର୍ଷ - margin_ଶୀର୍ଷ;
		rect.x1 = x + padding_width + border_ଡାହାଣ + margin_ଡାହାଣ;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		ଯଦି (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ଯଦି (box->parent != ଖାଲି) {
		bool need_clip = false;
		ଯଦି (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			ଯଦି (r.x0 < clip->x0) r.x0 = clip->x0;
			ଯଦି (r.y0 < clip->y0) r.y0 = clip->y0;
			ଯଦି (clip->x1 < r.x1) r.x1 = clip->x1;
			ଯଦି (clip->y1 < r.y1) r.y1 = clip->y1;
			ଯଦି (r.x1 <= r.x0 || r.y1 <= r.y0) {
				ଫେରନ୍ତୁ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ଅନ୍ୟଥା ଯଦି (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			ଯଦି (r.x0 < clip->x0) r.x0 = clip->x0;
			ଯଦି (clip->x1 < r.x1) r.x1 = clip->x1;
			ଯଦି (r.x1 <= r.x0) {
				ଫେରନ୍ତୁ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} ଅନ୍ୟଥା ଯଦି (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			ଯଦି (r.y0 < clip->y0) r.y0 = clip->y0;
			ଯଦି (clip->y1 < r.y1) r.y1 = clip->y1;
			ଯଦି (r.y1 <= r.y0) {
				ଫେରନ୍ତୁ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ଯଦି (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			ଯଦି (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ଫେରନ୍ତୁ false;
		}
	}

	/* text decoration */
	ଯଦି ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		ଯଦି (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			ଫେରନ୍ତୁ false;
	}

	ଯଦି (box->ଗଣ୍ଠି != ଖାଲି) {
		exc = dom_html_element_get_tag_type(box->ଗଣ୍ଠି, &tag_type);
		ଯଦି (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} ଅନ୍ୟଥା {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ଯଦି (box->object && width != 0 && height != 0) {
		ସଂରଚନା content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_ବାମ;
		obj_data.y = y_scrolled + padding_ଶୀର୍ଷ;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		ଯଦି (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		ଯଦି (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ଅଚଳ ଅକ୍ଷର *obj = "\xef\xbf\xbc";
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj_width;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj_x = x + padding_ବାମ;
			nserror res;

			rect.x0 = x + padding_ବାମ;
			rect.y0 = y + padding_ଶୀର୍ଷ;
			rect.x1 = x + padding_ବାମ + width - 1;
			rect.y1 = y + padding_ଶୀର୍ଷ + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ଯଦି (res != NSERROR_OK) {
				ଫେରନ୍ତୁ false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  ଆକାର(obj) - 1,
						  &obj_width);
			ଯଦି (res != NSERROR_OK) {
				obj_x += 1;
			} ଅନ୍ୟଥା {
				obj_x += width / 2 - obj_width / 2;
			}

			ଯଦି (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_ଶୀର୍ଷ + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(height * 0.75),
					    obj, ଆକାର(obj) - 1) != NSERROR_OK)
				ଫେରନ୍ତୁ false;
		}
	} ଅନ୍ୟଥା ଯଦି (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ଗଣ୍ଠି != ଖାଲି &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ସଂରଚନା bitmap *bitmap = ଖାଲି;
		exc = dom_ଗଣ୍ଠି_get_user_data(box->ଗଣ୍ଠି,
					     corestring_dom_ns_ଚାବି_canvas_ଗଣ୍ଠି_data,
					     &bitmap);
		ଯଦି (exc != DOM_NO_ERR) {
			bitmap = ଖାଲି;
		}
		ଯଦି (bitmap != ଖାଲି &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ବାମ, y + padding_ଶୀର୍ଷ,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା ଯଦି (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_ବାମ,
				y + padding_ଶୀର୍ଷ, &r, ctx);

	} ଅନ୍ୟଥା ଯଦି (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ଯଦି (!html_redraw_checkbox(x + padding_ବାମ, y + padding_ଶୀର୍ଷ,
				width, height, box->gadget->selected, ctx))
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା ଯଦି (box->gadget && box->gadget->type == GADGET_RADIO) {
		ଯଦି (!html_redraw_radio(x + padding_ବାମ, y + padding_ଶୀର୍ଷ,
				width, height, box->gadget->selected, ctx))
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା ଯଦି (box->gadget && box->gadget->type == GADGET_FILE) {
		ଯଦି (!html_redraw_file(x + padding_ବାମ, y + padding_ଶୀର୍ଷ,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା ଯଦି (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} ଅନ୍ୟଥା ଯଦି (box->text) {
		ଯଦି (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			ଫେରନ୍ତୁ false;

	} ଅନ୍ୟଥା {
		ଯଦି (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			ଫେରନ୍ତୁ false;
	}

	ଯଦି (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		ଯଦି (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ଫେରନ୍ତୁ false;

	/* list marker */
	ଯଦି (box->list_marker) {
		ଯଦି (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			ଫେରନ୍ତୁ false;
	}

	/* scrollbars */
	ଯଦି (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == ଖାଲି || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != ଖାଲି) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ସଂରଚନା content *)html,
					    box, has_x_scroll, has_y_scroll);
		ଯଦି (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (box->scroll_x != ଖାଲି)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[ଶୀର୍ଷ_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		ଯଦି (box->scroll_y != ଖାଲି)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[ବାମ_2] +
					box->width + box->padding[ଡାହାଣ_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	ଯଦି (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		ଯଦି (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(ସଂରଚନା content *c, ସଂରଚନା content_redraw_data *data,
		ଅଚଳ ସଂରଚନା rect *clip, ଅଚଳ ସଂରଚନା redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ସଂରଚନା box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	ପ୍ରତିଜ୍ଞା(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ଯଦି (ctx->interactive && html->visible_select_menu != ଖାଲି) {
		ସଂରଚନା form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	ଯଦି (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ଯଦି (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	ଯଦି (select) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ବାମ_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[ଶୀର୍ଷ_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	ଫେରନ୍ତୁ result;

}
