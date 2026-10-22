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

#అన్తర్భావ "utils/config.h"
#అన్తర్భావ <assert.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/browser_window.h"
#అన్తర్భావ "netsurf/plotters.h"
#అన్తర్భావ "netsurf/bitmap.h"
#అన్తర్భావ "netsurf/layout.h"
#అన్తర్భావ "content/content.h"
#అన్తర్భావ "content/content_protected.h"
#అన్తర్భావ "content/textsearch.h"
#అన్తర్భావ "css/utils.h"
#అన్తర్భావ "desktop/selection.h"
#అన్తర్భావ "desktop/print.h"
#అన్తర్భావ "desktop/scrollbar.h"
#అన్తర్భావ "desktop/textarea.h"
#అన్తర్భావ "desktop/gui_internal.h"

#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"
#అన్తర్భావ "html/box_manipulate.h"
#అన్తర్భావ "html/font.h"
#అన్తర్భావ "html/form_internal.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
స్థిర bool html_redraw_box_has_background(నిర్మాణం box *box)
{
	ఒకవేళ (box->background != ఖాళీ)
		తిరిగి true;

	ఒకవేళ (box->style != ఖాళీ) {
		css_color రంగు;

		css_computed_background_color(box->style, &రంగు);

		ఒకవేళ (nscss_color_is_transparent(రంగు) == false)
			తిరిగి true;
	}

	తిరిగి false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
స్థిర నిర్మాణం box *html_redraw_కనుగొను_bg_box(నిర్మాణం box *box)
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
	ఒకవేళ (box->మాతృ == ఖాళీ) {
		/* Root box */
		ఒకవేళ (html_redraw_box_has_background(box))
			తిరిగి box;

		/* No background on root box: consider body box, if any */
		ఒకవేళ (box->children != ఖాళీ) {
			ఒకవేళ (html_redraw_box_has_background(box->children))
				తిరిగి box->children;
		}
	} లేకపోతే ఒకవేళ (box->మాతృ != ఖాళీ && box->మాతృ->మాతృ == ఖాళీ) {
		/* Body box: only render background if root has its own */
		ఒకవేళ (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->మాతృ))
			తిరిగి box;
	} లేకపోతే {
		/* Any other box */
		ఒకవేళ (html_redraw_box_has_background(box))
			తిరిగి box;
	}

	తిరిగి ఖాళీ;
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

స్థిర bool
పాఠ్యం_redraw(స్థిరరాశి అక్షరం *utf8_పాఠ్యం,
	    size_t utf8_len,
	    size_t స్థానభ్రంశం,
	    పూర్ణాంకం space,
	    స్థిరరాశి plot_font_style_t *fstyle,
	    పూర్ణాంకం x,
	    పూర్ణాంకం y,
	    స్థిరరాశి నిర్మాణం rect *clip,
	    పూర్ణాంకం ఎత్తు,
	    దశాంశం scale,
	    bool excluded,
	    నిర్మాణం content *c,
	    స్థిరరాశి నిర్మాణం selection *sel,
	    స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.కొలత *= scale;

	/* is this box part of a selection? */
	ఒకవేళ (!excluded && ctx->interactive == true) {
		గుర్తులేని len = utf8_len + (space ? 1 : 0);
		గుర్తులేని ప్రారంభం_idx;
		గుర్తులేని చివర_idx;

		/* first try the browser window's current selection */
		ఒకవేళ (selection_highlighted(sel,
					  స్థానభ్రంశం,
					  స్థానభ్రంశం + len,
					  &ప్రారంభం_idx,
					  &చివర_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ఒకవేళ (!highlighted &&
		    (c->textsearch.context != ఖాళీ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     స్థానభ్రంశం,
						     స్థానభ్రంశం + len,
						     &ప్రారంభం_idx,
						     &చివర_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ఒకవేళ (highlighted) {
			నిర్మాణం rect r;
			గుర్తులేని endtxt_idx = చివర_idx;
			bool clip_మారింది = false;
			bool పాఠ్యం_visible = true;
			పూర్ణాంకం startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ఒకవేళ (చివర_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ప్రతిజ్ఞా(చివర_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->వెడల్పు(fstyle,
						  utf8_పాఠ్యం, ప్రారంభం_idx,
						  &startx);
			ఒకవేళ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->వెడల్పు(fstyle,
						  utf8_పాఠ్యం, endtxt_idx,
						  &endx);
			ఒకవేళ (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ఒకవేళ (చివర_idx > utf8_len) {
					endx += space;
			}

			ఒకవేళ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ఒకవేళ ((ప్రారంభం_idx > 0) &&
			    (ctx->plot->పాఠ్యం(ctx,
					     &plot_fstyle,
					     x,
					     y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
					     utf8_పాఠ్యం,
					     ప్రారంభం_idx) != NSERROR_OK))
				తిరిగి false;

			pstyle_fill_hback.fill_రంగు = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + ఎత్తు * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}

			ఒకవేళ (ప్రారంభం_idx > 0) {
				పూర్ణాంకం px0 = max(x + startx, clip->x0);
				పూర్ణాంకం px1 = min(x + endx, clip->x1);

				ఒకవేళ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}

					clip_మారింది = true;
				} లేకపోతే {
					పాఠ్యం_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_రంగు;
			fstyle_hback.foreground = రంగు_గమ్యం_bw_furthest(
				pstyle_fill_hback.fill_రంగు);

			ఒకవేళ (పాఠ్యం_visible &&
			    (ctx->plot->పాఠ్యం(ctx,
					     &fstyle_hback,
					     x,
					     y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
					     utf8_పాఠ్యం,
					     endtxt_idx) != NSERROR_OK)) {
				తిరిగి false;
			}

			/* draw any text succeeding highlighted portion */
			ఒకవేళ (endtxt_idx < utf8_len) {
				పూర్ణాంకం px0 = max(x + endx, clip->x0);
				ఒకవేళ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}

					clip_మారింది = true;

					res = ctx->plot->పాఠ్యం(ctx,
							      &plot_fstyle,
							      x,
							      y + (పూర్ణాంకం)(ఎత్తు * 0.75 * scale),
							      utf8_పాఠ్యం,
							      utf8_len);
					ఒకవేళ (res != NSERROR_OK) {
						తిరిగి false;
					}
				}
			}

			ఒకవేళ (clip_మారింది &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				తిరిగి false;
			}
		}
	}

	ఒకవేళ (!highlighted) {
		res = ctx->plot->పాఠ్యం(ctx,
				      &plot_fstyle,
				      x,
				      y + (పూర్ణాంకం) (ఎత్తు * 0.75 * scale),
				      utf8_పాఠ్యం,
				      utf8_len);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}
	తిరిగి true;
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

స్థిర bool html_redraw_checkbox(పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు,
		bool selected, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	ద్విగుణ z;
	nserror res;
	నిర్మాణం rect rect;

	z = వెడల్పు * 0.15;
	ఒకవేళ (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + వెడల్పు;
	rect.y1 = y + ఎత్తు;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->పంక్తి(ctx, plot_style_stroke_darkwbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + ఎత్తు;
	res = ctx->plot->పంక్తి(ctx, plot_style_stroke_darkwbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* light line across right */
	rect.x0 = x + వెడల్పు;
	rect.x1 = x + వెడల్పు;
	res = ctx->plot->పంక్తి(ctx, plot_style_stroke_lightwbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + ఎత్తు;
	res = ctx->plot->పంక్తి(ctx, plot_style_stroke_lightwbasec, &rect);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	ఒకవేళ (selected) {
		ఒకవేళ (వెడల్పు < 12 || ఎత్తు < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + వెడల్పు - z;
			rect.y1 = y + ఎత్తు - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
		} లేకపోతే {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + వెడల్పు - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + ఎత్తు - z;
			res = ctx->plot->పంక్తి(ctx, plot_style_stroke_wblobc, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + ఎత్తు - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (ఎత్తు / 2);
			res = ctx->plot->పంక్తి(ctx, plot_style_stroke_wblobc, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
		}
	}
	తిరిగి true;
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
స్థిర bool html_redraw_radio(పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు,
		bool selected, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + వెడల్పు * 0.5,
			      y + ఎత్తు * 0.5,
			      వెడల్పు * 0.5 - 1);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + వెడల్పు * 0.5,
			     y + ఎత్తు * 0.5,
			     వెడల్పు * 0.5 - 1,
			     45,
			     225);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + వెడల్పు * 0.5,
			     y + ఎత్తు * 0.5,
			     వెడల్పు * 0.5 - 1,
			     225,
			     45);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}

	ఒకవేళ (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + వెడల్పు * 0.5,
				      y + ఎత్తు * 0.5,
				      వెడల్పు * 0.3 - 1);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}

	తిరిగి true;
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

స్థిర bool html_redraw_file(పూర్ణాంకం x, పూర్ణాంకం y, పూర్ణాంకం వెడల్పు, పూర్ణాంకం ఎత్తు,
		నిర్మాణం box *box, దశాంశం scale, రంగు background_రంగు,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	పూర్ణాంకం పాఠ్యం_వెడల్పు;
	స్థిరరాశి అక్షరం *పాఠ్యం;
	size_t నిడివి;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_రంగు;

	ఒకవేళ (box->gadget->విలువ) {
		పాఠ్యం = box->gadget->విలువ;
	} లేకపోతే {
		పాఠ్యం = messages_get("Form_Drop");
	}
	నిడివి = అక్షరశ్రేణి_పొడవు(పాఠ్యం);

	res = guit->layout->వెడల్పు(&fstyle, పాఠ్యం, నిడివి, &పాఠ్యం_వెడల్పు);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}
	పాఠ్యం_వెడల్పు *= scale;
	ఒకవేళ (వెడల్పు < పాఠ్యం_వెడల్పు + 8) {
		x = x + వెడల్పు - పాఠ్యం_వెడల్పు - 4;
	} లేకపోతే {
		x = x + 4;
	}

	res = ctx->plot->పాఠ్యం(ctx, &fstyle, x, y + ఎత్తు * 0.75, పాఠ్యం, నిడివి);
	ఒకవేళ (res != NSERROR_OK) {
		తిరిగి false;
	}
	తిరిగి true;
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

స్థిర bool html_redraw_background(పూర్ణాంకం x, పూర్ణాంకం y, నిర్మాణం box *box, దశాంశం scale,
		స్థిరరాశి నిర్మాణం rect *clip, రంగు *background_రంగు,
		నిర్మాణం box *background,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_రంగు = true;
	bool plot_content;
	bool clip_గమ్యం_children = false;
	నిర్మాణం box *clip_box = box;
	పూర్ణాంకం ox = x, oy = y;
	పూర్ణాంకం వెడల్పు, ఎత్తు;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	నిర్మాణం box *మాతృ;
	నిర్మాణం rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_రంగు = *background_రంగు,
	};
	nserror res;

	ఒకవేళ (ctx->background_images == false)
		తిరిగి true;

	plot_content = (background->background != ఖాళీ);

	ఒకవేళ (plot_content) {
		ఒకవేళ (!box->మాతృ) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ఎడమ_2] * scale;
			y -= box->margin[శిఖరం_2] * scale;
			వెడల్పు = box->margin[ఎడమ_2] + box->padding[ఎడమ_2] +
					box->వెడల్పు + box->padding[కుడి_2] +
					box->margin[కుడి_2];
			ఎత్తు = box->margin[శిఖరం_2] + box->padding[శిఖరం_2] +
					box->ఎత్తు + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} లేకపోతే {
			వెడల్పు = box->padding[ఎడమ_2] + box->వెడల్పు +
					box->padding[కుడి_2];
			ఎత్తు = box->padding[శిఖరం_2] + box->ఎత్తు +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		ఎంపిక (css_computed_background_repeat(background->style)) {
		స్థితి CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_రంగు = !content_get_opaque(background->background);
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ఆపు;

		అప్రమేయ:
			ఆపు;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ఒకవేళ (hunit == CSS_UNIT_PCT) {
			x += (వెడల్పు -
				content_get_వెడల్పు(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} లేకపోతే {
			x += (పూర్ణాంకం) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ఒకవేళ (vunit == CSS_UNIT_PCT) {
			y += (ఎత్తు -
				content_get_ఎత్తు(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} లేకపోతే {
			y += (పూర్ణాంకం) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ఒకవేళ (box->type == BOX_పట్టిక_అడ్డువరుస) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		కొరకు (మాతృ = box->మాతృ;
			((మాతృ) && (మాతృ->type != BOX_పట్టిక));
				మాతృ = మాతృ->మాతృ);
		ప్రతిజ్ఞా(మాతృ && (మాతృ->style));

		css_computed_border_spacing(మాతృ->style, &h, &hu, &v, &vu);

		clip_గమ్యం_children = (h > 0) || (v > 0);

		ఒకవేళ (clip_గమ్యం_children)
			clip_box = box->children;
	}

	కొరకు (; clip_box; clip_box = clip_box->తదుపరి) {
		/* clip to child boxes if needed */
		ఒకవేళ (clip_గమ్యం_children) {
			ప్రతిజ్ఞా(clip_box->type == BOX_పట్టిక_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ఎడమ_2] +
					clip_box->వెడల్పు +
					clip_box->padding[కుడి_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[శిఖరం_2] +
					clip_box->ఎత్తు +
					clip_box->padding[BOTTOM]) * scale;

			ఒకవేళ (r.x0 < clip->x0) r.x0 = clip->x0;
			ఒకవేళ (r.y0 < clip->y0) r.y0 = clip->y0;
			ఒకవేళ (r.x1 > clip->x1) r.x1 = clip->x1;
			ఒకవేళ (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ఒకవేళ ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ఖాళీ) &&
			     content_get_opaque(clip_box->background)))
				కొనసాగు;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ఒకవేళ (nscss_color_is_transparent(bgcol) == false) {
			*background_రంగు = nscss_color_గమ్యం_ns(bgcol);
			pstyle_fill_bg.fill_రంగు = *background_రంగు;
			ఒకవేళ (plot_రంగు) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ఒకవేళ (res != NSERROR_OK) {
					తిరిగి false;
				}
			}
		}
		/* and plot the image */
		ఒకవేళ (plot_content) {
			వెడల్పు = content_get_వెడల్పు(background->background);
			ఎత్తు = content_get_ఎత్తు(background->background);

			/* ensure clip area only as large as required */
			ఒకవేళ (!repeat_x) {
				ఒకవేళ (r.x0 < x)
					r.x0 = x;
				ఒకవేళ (r.x1 > x + వెడల్పు * scale)
					r.x1 = x + వెడల్పు * scale;
			}
			ఒకవేళ (!repeat_y) {
				ఒకవేళ (r.y0 < y)
					r.y0 = y;
				ఒకవేళ (r.y1 > y + ఎత్తు * scale)
					r.y1 = y + ఎత్తు * scale;
			}
			/* valid clipping rectangles only */
			ఒకవేళ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				నిర్మాణం content_redraw_దత్తాంశం bg_దత్తాంశం;

				res = ctx->plot->clip(ctx, &r);
				ఒకవేళ (res != NSERROR_OK) {
					తిరిగి false;
				}

				bg_దత్తాంశం.x = x;
				bg_దత్తాంశం.y = y;
				bg_దత్తాంశం.వెడల్పు = ceilf(వెడల్పు * scale);
				bg_దత్తాంశం.ఎత్తు = ceilf(ఎత్తు * scale);
				bg_దత్తాంశం.background_రంగు = *background_రంగు;
				bg_దత్తాంశం.scale = scale;
				bg_దత్తాంశం.repeat_x = repeat_x;
				bg_దత్తాంశం.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_దత్తాంశం, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		ఒకవేళ (!clip_గమ్యం_children)
			తిరిగి true;
	}
	తిరిగి true;
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

స్థిర bool html_redraw_inline_background(పూర్ణాంకం x, పూర్ణాంకం y, నిర్మాణం box *box,
		దశాంశం scale, స్థిరరాశి నిర్మాణం rect *clip, నిర్మాణం rect b,
		bool మొదటి, bool చివరి, రంగు *background_రంగు,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం rect r = *clip;
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_రంగు = true;
	bool plot_content;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_రంగు = *background_రంగు,
	};
	nserror res;

	plot_content = (box->background != ఖాళీ);

	ఒకవేళ (html_redraw_printing && nsoption_bool(remove_backgrounds))
		తిరిగి true;

	ఒకవేళ (plot_content) {
		/* handle background-repeat */
		ఎంపిక (css_computed_background_repeat(box->style)) {
		స్థితి CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_రంగు = !content_get_opaque(box->background);
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			ఆపు;

		స్థితి CSS_BACKGROUND_REPEAT_NO_REPEAT:
			ఆపు;

		అప్రమేయ:
			ఆపు;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ఒకవేళ (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_వెడల్పు(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ఒకవేళ (!repeat_x && ((hpos < 2 && !మొదటి) ||
					(hpos > 98 && !చివరి))){
				plot_content = false;
			}
		} లేకపోతే {
			x += (పూర్ణాంకం) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ఒకవేళ (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_ఎత్తు(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} లేకపోతే {
			y += (పూర్ణాంకం) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ఒకవేళ (nscss_color_is_transparent(bgcol) == false) {
		*background_రంగు = nscss_color_గమ్యం_ns(bgcol);
		pstyle_fill_bg.fill_రంగు = *background_రంగు;

		ఒకవేళ (plot_రంగు) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
		}
	}
	/* and plot the image */
	ఒకవేళ (plot_content) {
		పూర్ణాంకం వెడల్పు = content_get_వెడల్పు(box->background);
		పూర్ణాంకం ఎత్తు = content_get_ఎత్తు(box->background);

		ఒకవేళ (!repeat_x) {
			ఒకవేళ (r.x0 < x)
				r.x0 = x;
			ఒకవేళ (r.x1 > x + వెడల్పు * scale)
				r.x1 = x + వెడల్పు * scale;
		}
		ఒకవేళ (!repeat_y) {
			ఒకవేళ (r.y0 < y)
				r.y0 = y;
			ఒకవేళ (r.y1 > y + ఎత్తు * scale)
				r.y1 = y + ఎత్తు * scale;
		}
		/* valid clipping rectangles only */
		ఒకవేళ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			నిర్మాణం content_redraw_దత్తాంశం bg_దత్తాంశం;

			res = ctx->plot->clip(ctx, &r);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}

			bg_దత్తాంశం.x = x;
			bg_దత్తాంశం.y = y;
			bg_దత్తాంశం.వెడల్పు = ceilf(వెడల్పు * scale);
			bg_దత్తాంశం.ఎత్తు = ceilf(ఎత్తు * scale);
			bg_దత్తాంశం.background_రంగు = *background_రంగు;
			bg_దత్తాంశం.scale = scale;
			bg_దత్తాంశం.repeat_x = repeat_x;
			bg_దత్తాంశం.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_దత్తాంశం, &r, ctx);
		}
	}

	తిరిగి true;
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

స్థిర bool
html_redraw_పాఠ్యం_decoration_inline(నిర్మాణం box *box,
				   పూర్ణాంకం x, పూర్ణాంకం y,
				   దశాంశం scale,
				   రంగు రంగు,
				   దశాంశం నిష్పత్తి,
				   స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_రంగు = రంగు,
	};
	nserror res;
	నిర్మాణం rect rect;

	కొరకు (c = box->తదుపరి;
	     c && c != box->inline_చివర;
	     c = c->తదుపరి) {
		ఒకవేళ (c->type != BOX_పాఠ్యం) {
			కొనసాగు;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->ఎత్తు * నిష్పత్తి) * scale;
		rect.x1 = (x + c->x + c->వెడల్పు) * scale;
		rect.y1 = (y + c->y + c->ఎత్తు * నిష్పత్తి) * scale;
		res = ctx->plot->పంక్తి(ctx, &plot_style_box, &rect);
		ఒకవేళ (res != NSERROR_OK) {
			తిరిగి false;
		}
	}
	తిరిగి true;
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

స్థిర bool
html_redraw_పాఠ్యం_decoration_ఖండం(నిర్మాణం box *box,
				  పూర్ణాంకం x, పూర్ణాంకం y,
				  దశాంశం scale,
				  రంగు రంగు,
				  దశాంశం నిష్పత్తి,
				  స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_రంగు = రంగు,
	};
	nserror res;
	నిర్మాణం rect rect;

	/* draw through text descendants */
	కొరకు (c = box->children; c; c = c->తదుపరి) {
		ఒకవేళ (c->type == BOX_పాఠ్యం) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->ఎత్తు * నిష్పత్తి) * scale;
			rect.x1 = (x + c->x + c->వెడల్పు) * scale;
			rect.y1 = (y + c->y + c->ఎత్తు * నిష్పత్తి) * scale;
			res = ctx->plot->పంక్తి(ctx, &plot_style_box, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}
		} లేకపోతే ఒకవేళ ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_ఖండం)) {
			ఒకవేళ (!html_redraw_పాఠ్యం_decoration_ఖండం(c,
					x + c->x, y + c->y,
					scale, రంగు, నిష్పత్తి, ctx))
				తిరిగి false;
		}
	}
	తిరిగి true;
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

స్థిర bool html_redraw_పాఠ్యం_decoration(నిర్మాణం box *box,
		పూర్ణాంకం x_మాతృ, పూర్ణాంకం y_మాతృ, దశాంశం scale,
		రంగు background_రంగు, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	స్థిర స్థిరరాశి గణన css_పాఠ్యం_decoration_e decoration[] = {
		CSS_పాఠ్యం_DECORATION_UNDERLINE, CSS_పాఠ్యం_DECORATION_OVERLINE,
		CSS_పాఠ్యం_DECORATION_పంక్తి_THROUGH };
	స్థిర స్థిరరాశి దశాంశం పంక్తి_నిష్పత్తి[] = { 0.9, 0.1, 0.5 };
	రంగు fgcol;
	గుర్తులేని పూర్ణాంకం i;
	css_color నిలువు;

	css_computed_color(box->style, &నిలువు);
	fgcol = nscss_color_గమ్యం_ns(నిలువు);

	/* antialias colour for under/overline */
	ఒకవేళ (html_redraw_printing == false)
		fgcol = blend_రంగు(background_రంగు, fgcol);

	ఒకవేళ (box->type == BOX_INLINE) {
		ఒకవేళ (!box->inline_చివర)
			తిరిగి true;
		కొరకు (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ఒకవేళ (css_computed_పాఠ్యం_decoration(box->style) &
					decoration[i])
				ఒకవేళ (!html_redraw_పాఠ్యం_decoration_inline(box,
						x_మాతృ, y_మాతృ, scale,
						fgcol, పంక్తి_నిష్పత్తి[i], ctx))
					తిరిగి false;
	} లేకపోతే {
		కొరకు (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ఒకవేళ (css_computed_పాఠ్యం_decoration(box->style) &
					decoration[i])
				ఒకవేళ (!html_redraw_పాఠ్యం_decoration_ఖండం(box,
						x_మాతృ + box->x,
						y_మాతృ + box->y,
						scale,
						fgcol, పంక్తి_నిష్పత్తి[i], ctx))
					తిరిగి false;
	}

	తిరిగి true;
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

స్థిర bool html_redraw_పాఠ్యం_box(స్థిరరాశి html_content *html, నిర్మాణం box *box,
		పూర్ణాంకం x, పూర్ణాంకం y, స్థిరరాశి నిర్మాణం rect *clip, దశాంశం scale,
		రంగు current_background_color,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	bool excluded = (box->object != ఖాళీ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ఒకవేళ (!పాఠ్యం_redraw(box->పాఠ్యం,
			 box->నిడివి,
			 box->byte_స్థానభ్రంశం,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->ఎత్తు,
			 scale,
			 excluded,
			 (నిర్మాణం content *)html,
			 html->sel,
			 ctx))
		తిరిగి false;

	తిరిగి true;
}

bool html_redraw_box(స్థిరరాశి html_content *html, నిర్మాణం box *box,
		పూర్ణాంకం x_మాతృ, పూర్ణాంకం y_మాతృ,
		స్థిరరాశి నిర్మాణం rect *clip, దశాంశం scale,
		రంగు current_background_color,
		స్థిరరాశి నిర్మాణం redraw_context *ctx);

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

స్థిర bool html_redraw_box_children(స్థిరరాశి html_content *html, నిర్మాణం box *box,
		పూర్ణాంకం x_మాతృ, పూర్ణాంకం y_మాతృ,
		స్థిరరాశి నిర్మాణం rect *clip, దశాంశం scale,
		రంగు current_background_color,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	నిర్మాణం box *c;

	కొరకు (c = box->children; c; c = c->తదుపరి) {

		ఒకవేళ (c->type != BOX_FLOAT_ఎడమ && c->type != BOX_FLOAT_కుడి)
			ఒకవేళ (!html_redraw_box(html, c,
					x_మాతృ + box->x -
					scrollbar_get_స్థానభ్రంశం(box->scroll_x),
					y_మాతృ + box->y -
					scrollbar_get_స్థానభ్రంశం(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				తిరిగి false;
	}
	కొరకు (c = box->float_children; c; c = c->తదుపరి_float)
		ఒకవేళ (!html_redraw_box(html, c,
				x_మాతృ + box->x -
				scrollbar_get_స్థానభ్రంశం(box->scroll_x),
				y_మాతృ + box->y -
				scrollbar_get_స్థానభ్రంశం(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			తిరిగి false;

	తిరిగి true;
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

bool html_redraw_box(స్థిరరాశి html_content *html, నిర్మాణం box *box,
		పూర్ణాంకం x_మాతృ, పూర్ణాంకం y_మాతృ,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి దశాంశం scale,
		రంగు current_background_color,
		స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	స్థిరరాశి నిర్మాణం plotter_పట్టిక *plot = ctx->plot;
	పూర్ణాంకం x, y;
	పూర్ణాంకం వెడల్పు, ఎత్తు;
	పూర్ణాంకం padding_ఎడమ, padding_శిఖరం, padding_వెడల్పు, padding_ఎత్తు;
	పూర్ణాంకం border_ఎడమ, border_శిఖరం, border_కుడి, border_bottom;
	నిర్మాణం rect r;
	నిర్మాణం rect rect;
	పూర్ణాంకం x_scrolled, y_scrolled;
	నిర్మాణం box *bg_box = ఖాళీ;
	css_computed_clip_rect css_rect;
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_x = CSS_అధికప్రవాహం_VISIBLE;
	గణన css_అధికప్రవాహం_e అధికప్రవాహం_y = CSS_అధికప్రవాహం_VISIBLE;
	dom_exception exc;
	dom_html_element_type ట్యాగ్_type;


	ఒకవేళ (html_redraw_printing && (box->flags & PRINTED))
		తిరిగి true;

	ఒకవేళ (box->style != ఖాళీ) {
		అధికప్రవాహం_x = css_computed_అధికప్రవాహం_x(box->style);
		అధికప్రవాహం_y = css_computed_అధికప్రవాహం_y(box->style);
	}

	/* avoid trivial FP maths */
	ఒకవేళ (scale == 1.0) {
		x = x_మాతృ + box->x;
		y = y_మాతృ + box->y;
		వెడల్పు = box->వెడల్పు;
		ఎత్తు = box->ఎత్తు;
		padding_ఎడమ = box->padding[ఎడమ_2];
		padding_శిఖరం = box->padding[శిఖరం_2];
		padding_వెడల్పు = padding_ఎడమ + box->వెడల్పు + box->padding[కుడి_2];
		padding_ఎత్తు = padding_శిఖరం + box->ఎత్తు +
				box->padding[BOTTOM];
		border_ఎడమ = box->border[ఎడమ_2].వెడల్పు;
		border_శిఖరం = box->border[శిఖరం_2].వెడల్పు;
		border_కుడి = box->border[కుడి_2].వెడల్పు;
		border_bottom = box->border[BOTTOM].వెడల్పు;
	} లేకపోతే {
		x = (x_మాతృ + box->x) * scale;
		y = (y_మాతృ + box->y) * scale;
		వెడల్పు = box->వెడల్పు * scale;
		ఎత్తు = box->ఎత్తు * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ఎడమ = box->padding[ఎడమ_2] ? box->padding[ఎడమ_2] * scale
				: 0;
		padding_శిఖరం = box->padding[శిఖరం_2] ? box->padding[శిఖరం_2] * scale
				: 0;
		padding_వెడల్పు = (box->padding[ఎడమ_2] + box->వెడల్పు +
				box->padding[కుడి_2]) * scale;
		padding_ఎత్తు = (box->padding[శిఖరం_2] + box->ఎత్తు +
				box->padding[BOTTOM]) * scale;
		border_ఎడమ = box->border[ఎడమ_2].వెడల్పు * scale;
		border_శిఖరం = box->border[శిఖరం_2].వెడల్పు * scale;
		border_కుడి = box->border[కుడి_2].వెడల్పు * scale;
		border_bottom = box->border[BOTTOM].వెడల్పు * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ఒకవేళ (box->style && అధికప్రవాహం_x != CSS_అధికప్రవాహం_VISIBLE &&
			box->మాతృ != ఖాళీ) {
		/* box contents clipped to box size */
		r.x0 = x - border_ఎడమ;
		r.x1 = x + padding_వెడల్పు + border_కుడి;
	} లేకపోతే {
		/* box contents can hang out of the box; use descendant box */
		ఒకవేళ (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} లేకపోతే {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ఒకవేళ (!box->మాతృ) {
			/* root element */
			పూర్ణాంకం margin_ఎడమ, margin_కుడి;
			ఒకవేళ (scale == 1.0) {
				margin_ఎడమ = box->margin[ఎడమ_2];
				margin_కుడి = box->margin[కుడి_2];
			} లేకపోతే {
				margin_ఎడమ = box->margin[ఎడమ_2] * scale;
				margin_కుడి = box->margin[కుడి_2] * scale;
			}
			r.x0 = x - border_ఎడమ - margin_ఎడమ < r.x0 ?
					x - border_ఎడమ - margin_ఎడమ : r.x0;
			r.x1 = x + padding_వెడల్పు + border_కుడి +
					margin_కుడి > r.x1 ?
					x + padding_వెడల్పు + border_కుడి +
					margin_కుడి : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ఒకవేళ (box->style && అధికప్రవాహం_y != CSS_అధికప్రవాహం_VISIBLE &&
			box->మాతృ != ఖాళీ) {
		/* box contents clipped to box size */
		r.y0 = y - border_శిఖరం;
		r.y1 = y + padding_ఎత్తు + border_bottom;
	} లేకపోతే {
		/* box contents can hang out of the box; use descendant box */
		ఒకవేళ (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} లేకపోతే {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ఒకవేళ (!box->మాతృ) {
			/* root element */
			పూర్ణాంకం margin_శిఖరం, margin_bottom;
			ఒకవేళ (scale == 1.0) {
				margin_శిఖరం = box->margin[శిఖరం_2];
				margin_bottom = box->margin[BOTTOM];
			} లేకపోతే {
				margin_శిఖరం = box->margin[శిఖరం_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_శిఖరం - margin_శిఖరం < r.y0 ?
					y - border_శిఖరం - margin_శిఖరం : r.y0;
			r.y1 = y + padding_ఎత్తు + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_ఎత్తు + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ఒకవేళ (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		తిరిగి true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ఒకవేళ (html_redraw_printing) {
		ఒకవేళ (r.y1 > html_redraw_printing_border) {
			ఒకవేళ (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_పాఠ్యం ||
					box->type == BOX_పట్టిక_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ఒకవేళ (r.y0 < html_redraw_printing_శిఖరం_cropped)
					html_redraw_printing_శిఖరం_cropped = r.y0;
				తిరిగి true;
			}
		}
		లేకపోతే box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ఒకవేళ (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ఒకవేళ ((ctx->plot->సమూహం_ప్రారంభం) &&
		    (ctx->plot->సమూహం_ప్రారంభం(ctx, "hidden box") != NSERROR_OK))
			తిరిగి false;
		ఒకవేళ (!html_redraw_box_children(html, box, x_మాతృ, y_మాతృ,
				&r, scale, current_background_color, ctx))
			తిరిగి false;
		తిరిగి ((!ctx->plot->సమూహం_చివర) || (ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
	}

	ఒకవేళ ((ctx->plot->సమూహం_ప్రారంభం) &&
	    (ctx->plot->సమూహం_ప్రారంభం(ctx,"vis box") != NSERROR_OK)) {
		తిరిగి false;
	}

	ఒకవేళ (box->style != ఖాళీ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ఒకవేళ (css_rect.ఎడమ_auto == false)
			r.x0 = x - border_ఎడమ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ఎడమ, css_rect.lunit));

		ఒకవేళ (css_rect.శిఖరం_auto == false)
			r.y0 = y - border_శిఖరం + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.శిఖరం, css_rect.tunit));

		ఒకవేళ (css_rect.కుడి_auto == false)
			r.x1 = x - border_ఎడమ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.కుడి, css_rect.runit));

		ఒకవేళ (css_rect.bottom_auto == false)
			r.y1 = y - border_శిఖరం + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ఒకవేళ (r.x0 < clip->x0) r.x0 = clip->x0;
		ఒకవేళ (r.y0 < clip->y0) r.y0 = clip->y0;
		ఒకవేళ (clip->x1 < r.x1) r.x1 = clip->x1;
		ఒకవేళ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ఒకవేళ (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			తిరిగి ((!ctx->plot->సమూహం_చివర) ||
				(ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
		/* clip to it */
		ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			తిరిగి false;

	} లేకపోతే ఒకవేళ (box->type == BOX_ఖండం || box->type == BOX_INLINE_ఖండం ||
			box->type == BOX_పట్టిక_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ఒకవేళ (r.x0 < clip->x0) r.x0 = clip->x0;
		ఒకవేళ (r.y0 < clip->y0) r.y0 = clip->y0;
		ఒకవేళ (clip->x1 < r.x1) r.x1 = clip->x1;
		ఒకవేళ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ఒకవేళ (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			తిరిగి ((!ctx->plot->సమూహం_చివర) ||
				(ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
		/* clip to it */
		ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			తిరిగి false;
	} లేకపోతే {
		/* clip box is fine, clip to it */
		r = *clip;
		ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			తిరిగి false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_కనుగొను_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ఒకవేళ (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_పాఠ్యం &&
			bg_box->type != BOX_INLINE_చివర &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ఖాళీ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		నిర్మాణం rect p;
		p.x0 = x - border_ఎడమ < r.x0 ? r.x0 : x - border_ఎడమ;
		p.y0 = y - border_శిఖరం < r.y0 ? r.y0 : y - border_శిఖరం;
		p.x1 = x + padding_వెడల్పు + border_కుడి < r.x1 ?
				x + padding_వెడల్పు + border_కుడి : r.x1;
		p.y1 = y + padding_ఎత్తు + border_bottom < r.y1 ?
				y + padding_ఎత్తు + border_bottom : r.y1;
		ఒకవేళ (!box->మాతృ) {
			/* Root element, special case:
			 * background covers margins too */
			పూర్ణాంకం m_ఎడమ, m_శిఖరం, m_కుడి, m_bottom;
			ఒకవేళ (scale == 1.0) {
				m_ఎడమ = box->margin[ఎడమ_2];
				m_శిఖరం = box->margin[శిఖరం_2];
				m_కుడి = box->margin[కుడి_2];
				m_bottom = box->margin[BOTTOM];
			} లేకపోతే {
				m_ఎడమ = box->margin[ఎడమ_2] * scale;
				m_శిఖరం = box->margin[శిఖరం_2] * scale;
				m_కుడి = box->margin[కుడి_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ఎడమ < r.x0 ? r.x0 : p.x0 - m_ఎడమ;
			p.y0 = p.y0 - m_శిఖరం < r.y0 ? r.y0 : p.y0 - m_శిఖరం;
			p.x1 = p.x1 + m_కుడి < r.x1 ? p.x1 + m_కుడి : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		ఒకవేళ ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ఒకవేళ (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				తిరిగి false;
			/* restore previous graphics window */
			ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				తిరిగి false;
		}
	}

	/* borders for block level content and replaced inlines */
	ఒకవేళ (box->style &&
	    box->type != BOX_పాఠ్యం &&
	    box->type != BOX_INLINE_చివర &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ఖాళీ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_శిఖరం || border_కుడి || border_bottom || border_ఎడమ)) {
		ఒకవేళ (!html_redraw_borders(box, x_మాతృ, y_మాతృ,
				padding_వెడల్పు, padding_ఎత్తు, &r,
				scale, ctx))
			తిరిగి false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ఒకవేళ (box->style && box->type == BOX_INLINE && box->inline_చివర &&
			(html_redraw_box_has_background(box) ||
			border_శిఖరం || border_కుడి ||
			border_bottom || border_ఎడమ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		నిర్మాణం box *ib;
		నిర్మాణం rect b; /* border edge rectangle */
		నిర్మాణం rect p; /* clipped rect */
		bool మొదటి = true;
		పూర్ణాంకం ib_x;
		పూర్ణాంకం ib_y = y;
		పూర్ణాంకం ib_p_వెడల్పు;
		పూర్ణాంకం ib_b_ఎడమ, ib_b_కుడి;

		b.x0 = x - border_ఎడమ;
		b.x1 = x + padding_వెడల్పు + border_కుడి;
		b.y0 = y - border_శిఖరం;
		b.y1 = y + padding_ఎత్తు + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		కొరకు (ib = box; ib; ib = ib->తదుపరి) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ఒకవేళ (ib->type == BOX_FLOAT_ఎడమ ||
					ib->type == BOX_FLOAT_కుడి)
				కొనసాగు;
			ఒకవేళ (scale == 1.0) {
				ib_x = x_మాతృ + ib->x;
				ib_y = y_మాతృ + ib->y;
				ib_p_వెడల్పు = ib->padding[ఎడమ_2] + ib->వెడల్పు +
						ib->padding[కుడి_2];
				ib_b_ఎడమ = ib->border[ఎడమ_2].వెడల్పు;
				ib_b_కుడి = ib->border[కుడి_2].వెడల్పు;
			} లేకపోతే {
				ib_x = (x_మాతృ + ib->x) * scale;
				ib_y = (y_మాతృ + ib->y) * scale;
				ib_p_వెడల్పు = (ib->padding[ఎడమ_2] + ib->వెడల్పు +
						ib->padding[కుడి_2]) * scale;
				ib_b_ఎడమ = ib->border[ఎడమ_2].వెడల్పు * scale;
				ib_b_కుడి = ib->border[కుడి_2].వెడల్పు * scale;
			}

			ఒకవేళ ((ib->flags & NEW_పంక్తి) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ఒకవేళ (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						మొదటి, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					తిరిగి false;
				/* restore previous graphics window */
				ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					తిరిగి false;
				ఒకవేళ (!html_redraw_inline_borders(box, b, &r,
						scale, మొదటి, false, ctx))
					తిరిగి false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ఎడమ;
				b.y0 = ib_y - border_శిఖరం - padding_శిఖరం;
				b.y1 = ib_y + padding_ఎత్తు - padding_శిఖరం +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				మొదటి = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_వెడల్పు + ib_b_కుడి;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ఒకవేళ (ib == box->inline_చివర)
				/* reached end of BOX_INLINE span */
				ఆపు;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ఒకవేళ (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				మొదటి, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			తిరిగి false;
		/* restore previous graphics window */
		ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			తిరిగి false;
		ఒకవేళ (!html_redraw_inline_borders(box, b, &r, scale, మొదటి, true,
				ctx))
			తిరిగి false;

	}

	/* Debug outlines */
	ఒకవేళ (html_redraw_debug) {
		పూర్ణాంకం margin_ఎడమ, margin_కుడి;
		పూర్ణాంకం margin_శిఖరం, margin_bottom;
		ఒకవేళ (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ఎడమ = box->margin[ఎడమ_2];
			margin_శిఖరం = box->margin[శిఖరం_2];
			margin_కుడి = box->margin[కుడి_2];
			margin_bottom = box->margin[BOTTOM];
		} లేకపోతే {
			margin_ఎడమ = box->margin[ఎడమ_2] * scale;
			margin_శిఖరం = box->margin[శిఖరం_2] * scale;
			margin_కుడి = box->margin[కుడి_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ఎడమ;
		rect.y0 = y + padding_శిఖరం;
		rect.x1 = x + padding_ఎడమ + వెడల్పు;
		rect.y1 = y + padding_శిఖరం + ఎత్తు;
		ఒకవేళ (ctx->plot->rectangle(ctx, plot_style_content_అంచు, &rect) != NSERROR_OK)
			తిరిగి false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_వెడల్పు;
		rect.y1 = y + padding_ఎత్తు;
		ఒకవేళ (ctx->plot->rectangle(ctx, plot_style_padding_అంచు, &rect) != NSERROR_OK)
			తిరిగి false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ఎడమ - margin_ఎడమ;
		rect.y0 = y - border_శిఖరం - margin_శిఖరం;
		rect.x1 = x + padding_వెడల్పు + border_కుడి + margin_కుడి;
		rect.y1 = y + padding_ఎత్తు + border_bottom + margin_bottom;
		ఒకవేళ (ctx->plot->rectangle(ctx, plot_style_margin_అంచు, &rect) != NSERROR_OK)
			తిరిగి false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ఒకవేళ (box->మాతృ != ఖాళీ) {
		bool need_clip = false;
		ఒకవేళ (box->object || box->flags & IFRAME ||
				(అధికప్రవాహం_x != CSS_అధికప్రవాహం_VISIBLE &&
				 అధికప్రవాహం_y != CSS_అధికప్రవాహం_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_వెడల్పు;
			r.y1 = y + padding_ఎత్తు;
			ఒకవేళ (r.x0 < clip->x0) r.x0 = clip->x0;
			ఒకవేళ (r.y0 < clip->y0) r.y0 = clip->y0;
			ఒకవేళ (clip->x1 < r.x1) r.x1 = clip->x1;
			ఒకవేళ (clip->y1 < r.y1) r.y1 = clip->y1;
			ఒకవేళ (r.x1 <= r.x0 || r.y1 <= r.y0) {
				తిరిగి (!ctx->plot->సమూహం_చివర ||
					(ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} లేకపోతే ఒకవేళ (అధికప్రవాహం_x != CSS_అధికప్రవాహం_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_వెడల్పు;
			r.y1 = clip->y1;
			ఒకవేళ (r.x0 < clip->x0) r.x0 = clip->x0;
			ఒకవేళ (clip->x1 < r.x1) r.x1 = clip->x1;
			ఒకవేళ (r.x1 <= r.x0) {
				తిరిగి (!ctx->plot->సమూహం_చివర ||
					(ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} లేకపోతే ఒకవేళ (అధికప్రవాహం_y != CSS_అధికప్రవాహం_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_ఎత్తు;
			ఒకవేళ (r.y0 < clip->y0) r.y0 = clip->y0;
			ఒకవేళ (clip->y1 < r.y1) r.y1 = clip->y1;
			ఒకవేళ (r.y1 <= r.y0) {
				తిరిగి (!ctx->plot->సమూహం_చివర ||
					(ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ఒకవేళ (need_clip &&
		    (box->type == BOX_ఖండం ||
		     box->type == BOX_INLINE_ఖండం ||
		     box->type == BOX_పట్టిక_CELL || box->object)) {
			ఒకవేళ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				తిరిగి false;
		}
	}

	/* text decoration */
	ఒకవేళ ((box->type != BOX_పాఠ్యం) &&
	    box->style &&
	    css_computed_పాఠ్యం_decoration(box->style) !=	CSS_పాఠ్యం_DECORATION_NONE) {
		ఒకవేళ (!html_redraw_పాఠ్యం_decoration(box, x_మాతృ, y_మాతృ,
				scale, current_background_color, ctx))
			తిరిగి false;
	}

	ఒకవేళ (box->కణుపు != ఖాళీ) {
		exc = dom_html_element_get_ట్యాగ్_type(box->కణుపు, &ట్యాగ్_type);
		ఒకవేళ (exc != DOM_NO_ERR) {
			ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} లేకపోతే {
		ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ఒకవేళ (box->object && వెడల్పు != 0 && ఎత్తు != 0) {
		నిర్మాణం content_redraw_దత్తాంశం obj_దత్తాంశం;

		x_scrolled = x - scrollbar_get_స్థానభ్రంశం(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_స్థానభ్రంశం(box->scroll_y) * scale;

		obj_దత్తాంశం.x = x_scrolled + padding_ఎడమ;
		obj_దత్తాంశం.y = y_scrolled + padding_శిఖరం;
		obj_దత్తాంశం.వెడల్పు = వెడల్పు;
		obj_దత్తాంశం.ఎత్తు = ఎత్తు;
		obj_దత్తాంశం.background_రంగు = current_background_color;
		obj_దత్తాంశం.scale = scale;
		obj_దత్తాంశం.repeat_x = false;
		obj_దత్తాంశం.repeat_y = false;

		ఒకవేళ (content_get_type(box->object) == CONTENT_HTML) {
			obj_దత్తాంశం.x /= scale;
			obj_దత్తాంశం.y /= scale;
		}

		ఒకవేళ (!content_redraw(box->object, &obj_దత్తాంశం, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			స్థిరరాశి అక్షరం *obj = "\xef\xbf\xbc";
			పూర్ణాంకం obj_వెడల్పు;
			పూర్ణాంకం obj_x = x + padding_ఎడమ;
			nserror res;

			rect.x0 = x + padding_ఎడమ;
			rect.y0 = y + padding_శిఖరం;
			rect.x1 = x + padding_ఎడమ + వెడల్పు - 1;
			rect.y1 = y + padding_శిఖరం + ఎత్తు - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ఒకవేళ (res != NSERROR_OK) {
				తిరిగి false;
			}

			res = guit->layout->వెడల్పు(plot_fstyle_broken_object,
						  obj,
						  పరిమాణం(obj) - 1,
						  &obj_వెడల్పు);
			ఒకవేళ (res != NSERROR_OK) {
				obj_x += 1;
			} లేకపోతే {
				obj_x += వెడల్పు / 2 - obj_వెడల్పు / 2;
			}

			ఒకవేళ (ctx->plot->పాఠ్యం(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_శిఖరం + (పూర్ణాంకం)(ఎత్తు * 0.75),
					    obj, పరిమాణం(obj) - 1) != NSERROR_OK)
				తిరిగి false;
		}
	} లేకపోతే ఒకవేళ (ట్యాగ్_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->కణుపు != ఖాళీ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		నిర్మాణం bitmap *bitmap = ఖాళీ;
		exc = dom_కణుపు_get_user_దత్తాంశం(box->కణుపు,
					     corestring_dom_ns_కీలకం_canvas_కణుపు_దత్తాంశం,
					     &bitmap);
		ఒకవేళ (exc != DOM_NO_ERR) {
			bitmap = ఖాళీ;
		}
		ఒకవేళ (bitmap != ఖాళీ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ఎడమ, y + padding_శిఖరం,
				      వెడల్పు, ఎత్తు, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			తిరిగి false;
	} లేకపోతే ఒకవేళ (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_కిటికీ_redraw(box->iframe,
				x + padding_ఎడమ,
				y + padding_శిఖరం, &r, ctx);

	} లేకపోతే ఒకవేళ (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ఒకవేళ (!html_redraw_checkbox(x + padding_ఎడమ, y + padding_శిఖరం,
				వెడల్పు, ఎత్తు, box->gadget->selected, ctx))
			తిరిగి false;

	} లేకపోతే ఒకవేళ (box->gadget && box->gadget->type == GADGET_RADIO) {
		ఒకవేళ (!html_redraw_radio(x + padding_ఎడమ, y + padding_శిఖరం,
				వెడల్పు, ఎత్తు, box->gadget->selected, ctx))
			తిరిగి false;

	} లేకపోతే ఒకవేళ (box->gadget && box->gadget->type == GADGET_FILE) {
		ఒకవేళ (!html_redraw_file(x + padding_ఎడమ, y + padding_శిఖరం,
				వెడల్పు, ఎత్తు, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			తిరిగి false;

	} లేకపోతే ఒకవేళ (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->దత్తాంశం.పాఠ్యం.ta, x, y,
				current_background_color, scale, &r, ctx);

	} లేకపోతే ఒకవేళ (box->పాఠ్యం) {
		ఒకవేళ (!html_redraw_పాఠ్యం_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			తిరిగి false;

	} లేకపోతే {
		ఒకవేళ (!html_redraw_box_children(html, box, x_మాతృ, y_మాతృ, &r,
				scale, current_background_color, ctx))
			తిరిగి false;
	}

	ఒకవేళ (box->type == BOX_ఖండం || box->type == BOX_INLINE_ఖండం ||
			box->type == BOX_పట్టిక_CELL || box->type == BOX_INLINE)
		ఒకవేళ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			తిరిగి false;

	/* list marker */
	ఒకవేళ (box->list_marker) {
		ఒకవేళ (!html_redraw_box(html, box->list_marker,
				x_మాతృ + box->x -
				scrollbar_get_స్థానభ్రంశం(box->scroll_x),
				y_మాతృ + box->y -
				scrollbar_get_స్థానభ్రంశం(box->scroll_y),
				clip, scale, current_background_color, ctx))
			తిరిగి false;
	}

	/* scrollbars */
	ఒకవేళ (((box->style && box->type != BOX_BR &&
	      box->type != BOX_పట్టిక && box->type != BOX_INLINE &&
	      (box->gadget == ఖాళీ || box->gadget->type != GADGET_TEXTAREA) &&
	      (అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL ||
	       అధికప్రవాహం_x == CSS_అధికప్రవాహం_AUTO ||
	       అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL ||
	       అధికప్రవాహం_y == CSS_అధికప్రవాహం_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->మాతృ != ఖాళీ) {
		nserror res;
		bool has_x_scroll = (అధికప్రవాహం_x == CSS_అధికప్రవాహం_SCROLL);
		bool has_y_scroll = (అధికప్రవాహం_y == CSS_అధికప్రవాహం_SCROLL);

		has_x_scroll |= (అధికప్రవాహం_x == CSS_అధికప్రవాహం_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (అధికప్రవాహం_y == CSS_అధికప్రవాహం_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((నిర్మాణం content *)html,
					    box, has_x_scroll, has_y_scroll);
		ఒకవేళ (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			తిరిగి false;
		}

		ఒకవేళ (box->scroll_x != ఖాళీ)
			scrollbar_redraw(box->scroll_x,
					x_మాతృ + box->x,
					y_మాతృ + box->y + box->padding[శిఖరం_2] +
					box->ఎత్తు + box->padding[BOTTOM] -
					SCROLLBAR_వెడల్పు, clip, scale, ctx);
		ఒకవేళ (box->scroll_y != ఖాళీ)
			scrollbar_redraw(box->scroll_y,
					x_మాతృ + box->x + box->padding[ఎడమ_2] +
					box->వెడల్పు + box->padding[కుడి_2] -
					SCROLLBAR_వెడల్పు,
					y_మాతృ + box->y, clip, scale, ctx);
	}

	ఒకవేళ (box->type == BOX_ఖండం || box->type == BOX_INLINE_ఖండం ||
	    box->type == BOX_పట్టిక_CELL || box->type == BOX_INLINE) {
		ఒకవేళ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			తిరిగి false;
	}

	తిరిగి ((!plot->సమూహం_చివర) || (ctx->plot->సమూహం_చివర(ctx) == NSERROR_OK));
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

bool html_redraw(నిర్మాణం content *c, నిర్మాణం content_redraw_దత్తాంశం *దత్తాంశం,
		స్థిరరాశి నిర్మాణం rect *clip, స్థిరరాశి నిర్మాణం redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	నిర్మాణం box *box;
	bool ఫలితం = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_రంగు = దత్తాంశం->background_రంగు,
	};

	box = html->layout;
	ప్రతిజ్ఞా(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ఒకవేళ (ctx->interactive && html->visible_select_menu != ఖాళీ) {
		నిర్మాణం form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				దత్తాంశం->scale, clip);
	}

	ఒకవేళ (!select_only) {
		/* clear to background colour */
		ఫలితం = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ఒకవేళ (html->background_రంగు != NS_TRANSPARENT)
			pstyle_fill_bg.fill_రంగు = html->background_రంగు;

		ఫలితం &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		ఫలితం &= html_redraw_box(html, box, దత్తాంశం->x, దత్తాంశం->y, clip,
				దత్తాంశం->scale, pstyle_fill_bg.fill_రంగు, ctx);
	}

	ఒకవేళ (select) {
		పూర్ణాంకం menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ఎడమ_2].వెడల్పు;
		menu_y += box->ఎత్తు + box->border[BOTTOM].వెడల్పు +
				box->padding[BOTTOM] + box->padding[శిఖరం_2];
		ఫలితం &= form_redraw_select_menu(html->visible_select_menu,
				దత్తాంశం->x + menu_x, దత్తాంశం->y + menu_y,
				దత్తాంశం->scale, clip, ctx);
	}

	తిరిగి ఫలితం;

}
