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

#അന്തര്ഭാവ "utils/config.h"
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/browser_window.h"
#അന്തര്ഭാവ "netsurf/plotters.h"
#അന്തര്ഭാവ "netsurf/bitmap.h"
#അന്തര്ഭാവ "netsurf/layout.h"
#അന്തര്ഭാവ "content/content.h"
#അന്തര്ഭാവ "content/content_protected.h"
#അന്തര്ഭാവ "content/textsearch.h"
#അന്തര്ഭാവ "css/utils.h"
#അന്തര്ഭാവ "desktop/selection.h"
#അന്തര്ഭാവ "desktop/print.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"
#അന്തര്ഭാവ "desktop/textarea.h"
#അന്തര്ഭാവ "desktop/gui_internal.h"

#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"
#അന്തര്ഭാവ "html/box_manipulate.h"
#അന്തര്ഭാവ "html/font.h"
#അന്തര്ഭാവ "html/form_internal.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
സ്ഥിതം bool html_redraw_box_has_background(ഘടന box *box)
{
	എങ്കിൽ (box->background != ശൂന്യ)
		തിരികെ true;

	എങ്കിൽ (box->style != ശൂന്യ) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		എങ്കിൽ (nscss_color_is_transparent(colour) == false)
			തിരികെ true;
	}

	തിരികെ false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
സ്ഥിതം ഘടന box *html_redraw_find_bg_box(ഘടന box *box)
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
	എങ്കിൽ (box->parent == ശൂന്യ) {
		/* Root box */
		എങ്കിൽ (html_redraw_box_has_background(box))
			തിരികെ box;

		/* No background on root box: consider body box, if any */
		എങ്കിൽ (box->children != ശൂന്യ) {
			എങ്കിൽ (html_redraw_box_has_background(box->children))
				തിരികെ box->children;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (box->parent != ശൂന്യ && box->parent->parent == ശൂന്യ) {
		/* Body box: only render background if root has its own */
		എങ്കിൽ (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			തിരികെ box;
	} അല്ലെങ്കിൽ {
		/* Any other box */
		എങ്കിൽ (html_redraw_box_has_background(box))
			തിരികെ box;
	}

	തിരികെ ശൂന്യ;
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

സ്ഥിതം bool
text_redraw(സ്ഥിരം അക്ഷരം *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    പൂർണ്ണസംഖ്യ space,
	    സ്ഥിരം plot_font_style_t *fstyle,
	    പൂർണ്ണസംഖ്യ x,
	    പൂർണ്ണസംഖ്യ y,
	    സ്ഥിരം ഘടന rect *clip,
	    പൂർണ്ണസംഖ്യ height,
	    ദശാംശം scale,
	    bool excluded,
	    ഘടന content *c,
	    സ്ഥിരം ഘടന selection *sel,
	    സ്ഥിരം ഘടന redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	എങ്കിൽ (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		എങ്കിൽ (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		എങ്കിൽ (!highlighted &&
		    (c->textsearch.context != ശൂന്യ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		എങ്കിൽ (highlighted) {
			ഘടന rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			പൂർണ്ണസംഖ്യ startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			എങ്കിൽ (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				പ്രതിജ്ഞാ(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			എങ്കിൽ (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			എങ്കിൽ (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			എങ്കിൽ (end_idx > utf8_len) {
					endx += space;
			}

			എങ്കിൽ (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			എങ്കിൽ ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				തിരികെ false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}

			എങ്കിൽ (start_idx > 0) {
				പൂർണ്ണസംഖ്യ px0 = max(x + startx, clip->x0);
				പൂർണ്ണസംഖ്യ px1 = min(x + endx, clip->x1);

				എങ്കിൽ (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}

					clip_changed = true;
				} അല്ലെങ്കിൽ {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			എങ്കിൽ (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				തിരികെ false;
			}

			/* draw any text succeeding highlighted portion */
			എങ്കിൽ (endtxt_idx < utf8_len) {
				പൂർണ്ണസംഖ്യ px0 = max(x + endx, clip->x0);
				എങ്കിൽ (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (പൂർണ്ണസംഖ്യ)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					എങ്കിൽ (res != NSERROR_OK) {
						തിരികെ false;
					}
				}
			}

			എങ്കിൽ (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				തിരികെ false;
			}
		}
	}

	എങ്കിൽ (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (പൂർണ്ണസംഖ്യ) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}
	തിരികെ true;
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

സ്ഥിതം bool html_redraw_checkbox(പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height,
		bool selected, സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഇരട്ട z;
	nserror res;
	ഘടന rect rect;

	z = width * 0.15;
	എങ്കിൽ (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->വരി(ctx, plot_style_stroke_darkwbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->വരി(ctx, plot_style_stroke_darkwbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->വരി(ctx, plot_style_stroke_lightwbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->വരി(ctx, plot_style_stroke_lightwbasec, &rect);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	എങ്കിൽ (selected) {
		എങ്കിൽ (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
		} അല്ലെങ്കിൽ {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->വരി(ctx, plot_style_stroke_wblobc, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->വരി(ctx, plot_style_stroke_wblobc, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
		}
	}
	തിരികെ true;
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
സ്ഥിതം bool html_redraw_radio(പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height,
		bool selected, സ്ഥിരം ഘടന redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}

	എങ്കിൽ (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}

	തിരികെ true;
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

സ്ഥിതം bool html_redraw_file(പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, പൂർണ്ണസംഖ്യ width, പൂർണ്ണസംഖ്യ height,
		ഘടന box *box, ദശാംശം scale, colour background_colour,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	പൂർണ്ണസംഖ്യ text_width;
	സ്ഥിരം അക്ഷരം *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	എങ്കിൽ (box->gadget->മൂല്യം) {
		text = box->gadget->മൂല്യം;
	} അല്ലെങ്കിൽ {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}
	text_width *= scale;
	എങ്കിൽ (width < text_width + 8) {
		x = x + width - text_width - 4;
	} അല്ലെങ്കിൽ {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	എങ്കിൽ (res != NSERROR_OK) {
		തിരികെ false;
	}
	തിരികെ true;
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

സ്ഥിതം bool html_redraw_background(പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, ഘടന box *box, ദശാംശം scale,
		സ്ഥിരം ഘടന rect *clip, colour *background_colour,
		ഘടന box *background,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	ഘടന box *clip_box = box;
	പൂർണ്ണസംഖ്യ ox = x, oy = y;
	പൂർണ്ണസംഖ്യ width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ഘടന box *parent;
	ഘടന rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	എങ്കിൽ (ctx->background_images == false)
		തിരികെ true;

	plot_content = (background->background != ശൂന്യ);

	എങ്കിൽ (plot_content) {
		എങ്കിൽ (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ഇടത്_2] * scale;
			y -= box->margin[മുകൾ_2] * scale;
			width = box->margin[ഇടത്_2] + box->padding[ഇടത്_2] +
					box->width + box->padding[വലത്_2] +
					box->margin[വലത്_2];
			height = box->margin[മുകൾ_2] + box->padding[മുകൾ_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} അല്ലെങ്കിൽ {
			width = box->padding[ഇടത്_2] + box->width +
					box->padding[വലത്_2];
			height = box->padding[മുകൾ_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		തിരഞ്ഞെടുപ്പ് (css_computed_background_repeat(background->style)) {
		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		എങ്കിൽ (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} അല്ലെങ്കിൽ {
			x += (പൂർണ്ണസംഖ്യ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		എങ്കിൽ (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} അല്ലെങ്കിൽ {
			y += (പൂർണ്ണസംഖ്യ) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	എങ്കിൽ (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		വേണ്ടി (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		പ്രതിജ്ഞാ(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		എങ്കിൽ (clip_to_children)
			clip_box = box->children;
	}

	വേണ്ടി (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		എങ്കിൽ (clip_to_children) {
			പ്രതിജ്ഞാ(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ഇടത്_2] +
					clip_box->width +
					clip_box->padding[വലത്_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[മുകൾ_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			എങ്കിൽ (r.x0 < clip->x0) r.x0 = clip->x0;
			എങ്കിൽ (r.y0 < clip->y0) r.y0 = clip->y0;
			എങ്കിൽ (r.x1 > clip->x1) r.x1 = clip->x1;
			എങ്കിൽ (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			എങ്കിൽ ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ശൂന്യ) &&
			     content_get_opaque(clip_box->background)))
				തുടരുക;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		എങ്കിൽ (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			എങ്കിൽ (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				എങ്കിൽ (res != NSERROR_OK) {
					തിരികെ false;
				}
			}
		}
		/* and plot the image */
		എങ്കിൽ (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			എങ്കിൽ (!repeat_x) {
				എങ്കിൽ (r.x0 < x)
					r.x0 = x;
				എങ്കിൽ (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			എങ്കിൽ (!repeat_y) {
				എങ്കിൽ (r.y0 < y)
					r.y0 = y;
				എങ്കിൽ (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			എങ്കിൽ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ഘടന content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				എങ്കിൽ (res != NSERROR_OK) {
					തിരികെ false;
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
		എങ്കിൽ (!clip_to_children)
			തിരികെ true;
	}
	തിരികെ true;
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

സ്ഥിതം bool html_redraw_inline_background(പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, ഘടന box *box,
		ദശാംശം scale, സ്ഥിരം ഘടന rect *clip, ഘടന rect b,
		bool first, bool last, colour *background_colour,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന rect r = *clip;
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

	plot_content = (box->background != ശൂന്യ);

	എങ്കിൽ (html_redraw_printing && nsoption_bool(remove_backgrounds))
		തിരികെ true;

	എങ്കിൽ (plot_content) {
		/* handle background-repeat */
		തിരഞ്ഞെടുപ്പ് (css_computed_background_repeat(box->style)) {
		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			നിർത്തുക;

		അവസ്ഥ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
			നിർത്തുക;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		എങ്കിൽ (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			എങ്കിൽ (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} അല്ലെങ്കിൽ {
			x += (പൂർണ്ണസംഖ്യ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		എങ്കിൽ (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} അല്ലെങ്കിൽ {
			y += (പൂർണ്ണസംഖ്യ) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	എങ്കിൽ (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		എങ്കിൽ (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
		}
	}
	/* and plot the image */
	എങ്കിൽ (plot_content) {
		പൂർണ്ണസംഖ്യ width = content_get_width(box->background);
		പൂർണ്ണസംഖ്യ height = content_get_height(box->background);

		എങ്കിൽ (!repeat_x) {
			എങ്കിൽ (r.x0 < x)
				r.x0 = x;
			എങ്കിൽ (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		എങ്കിൽ (!repeat_y) {
			എങ്കിൽ (r.y0 < y)
				r.y0 = y;
			എങ്കിൽ (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		എങ്കിൽ ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ഘടന content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
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

	തിരികെ true;
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

സ്ഥിതം bool
html_redraw_text_decoration_inline(ഘടന box *box,
				   പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
				   ദശാംശം scale,
				   colour colour,
				   ദശാംശം ratio,
				   സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ഘടന rect rect;

	വേണ്ടി (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		എങ്കിൽ (c->type != BOX_TEXT) {
			തുടരുക;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->വരി(ctx, &plot_style_box, &rect);
		എങ്കിൽ (res != NSERROR_OK) {
			തിരികെ false;
		}
	}
	തിരികെ true;
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

സ്ഥിതം bool
html_redraw_text_decoration_block(ഘടന box *box,
				  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
				  ദശാംശം scale,
				  colour colour,
				  ദശാംശം ratio,
				  സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ഘടന rect rect;

	/* draw through text descendants */
	വേണ്ടി (c = box->children; c; c = c->next) {
		എങ്കിൽ (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->വരി(ctx, &plot_style_box, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			എങ്കിൽ (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				തിരികെ false;
		}
	}
	തിരികെ true;
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

സ്ഥിതം bool html_redraw_text_decoration(ഘടന box *box,
		പൂർണ്ണസംഖ്യ x_parent, പൂർണ്ണസംഖ്യ y_parent, ദശാംശം scale,
		colour background_colour, സ്ഥിരം ഘടന redraw_context *ctx)
{
	സ്ഥിതം സ്ഥിരം എണ്ണൽ css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_വരി_THROUGH };
	സ്ഥിതം സ്ഥിരം ദശാംശം വരി_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned പൂർണ്ണസംഖ്യ i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	എങ്കിൽ (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	എങ്കിൽ (box->type == BOX_INLINE) {
		എങ്കിൽ (!box->inline_end)
			തിരികെ true;
		വേണ്ടി (i = 0; i != NOF_ELEMENTS(decoration); i++)
			എങ്കിൽ (css_computed_text_decoration(box->style) &
					decoration[i])
				എങ്കിൽ (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, വരി_ratio[i], ctx))
					തിരികെ false;
	} അല്ലെങ്കിൽ {
		വേണ്ടി (i = 0; i != NOF_ELEMENTS(decoration); i++)
			എങ്കിൽ (css_computed_text_decoration(box->style) &
					decoration[i])
				എങ്കിൽ (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, വരി_ratio[i], ctx))
					തിരികെ false;
	}

	തിരികെ true;
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

സ്ഥിതം bool html_redraw_text_box(സ്ഥിരം html_content *html, ഘടന box *box,
		പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y, സ്ഥിരം ഘടന rect *clip, ദശാംശം scale,
		colour current_background_color,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	bool excluded = (box->object != ശൂന്യ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	എങ്കിൽ (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (ഘടന content *)html,
			 html->sel,
			 ctx))
		തിരികെ false;

	തിരികെ true;
}

bool html_redraw_box(സ്ഥിരം html_content *html, ഘടന box *box,
		പൂർണ്ണസംഖ്യ x_parent, പൂർണ്ണസംഖ്യ y_parent,
		സ്ഥിരം ഘടന rect *clip, ദശാംശം scale,
		colour current_background_color,
		സ്ഥിരം ഘടന redraw_context *ctx);

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

സ്ഥിതം bool html_redraw_box_children(സ്ഥിരം html_content *html, ഘടന box *box,
		പൂർണ്ണസംഖ്യ x_parent, പൂർണ്ണസംഖ്യ y_parent,
		സ്ഥിരം ഘടന rect *clip, ദശാംശം scale,
		colour current_background_color,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	ഘടന box *c;

	വേണ്ടി (c = box->children; c; c = c->next) {

		എങ്കിൽ (c->type != BOX_FLOAT_ഇടത് && c->type != BOX_FLOAT_വലത്)
			എങ്കിൽ (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				തിരികെ false;
	}
	വേണ്ടി (c = box->float_children; c; c = c->next_float)
		എങ്കിൽ (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			തിരികെ false;

	തിരികെ true;
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

bool html_redraw_box(സ്ഥിരം html_content *html, ഘടന box *box,
		പൂർണ്ണസംഖ്യ x_parent, പൂർണ്ണസംഖ്യ y_parent,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ദശാംശം scale,
		colour current_background_color,
		സ്ഥിരം ഘടന redraw_context *ctx)
{
	സ്ഥിരം ഘടന plotter_table *plot = ctx->plot;
	പൂർണ്ണസംഖ്യ x, y;
	പൂർണ്ണസംഖ്യ width, height;
	പൂർണ്ണസംഖ്യ padding_ഇടത്, padding_മുകൾ, padding_width, padding_height;
	പൂർണ്ണസംഖ്യ border_ഇടത്, border_മുകൾ, border_വലത്, border_bottom;
	ഘടന rect r;
	ഘടന rect rect;
	പൂർണ്ണസംഖ്യ x_scrolled, y_scrolled;
	ഘടന box *bg_box = ശൂന്യ;
	css_computed_clip_rect css_rect;
	എണ്ണൽ css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	എണ്ണൽ css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	എങ്കിൽ (html_redraw_printing && (box->flags & PRINTED))
		തിരികെ true;

	എങ്കിൽ (box->style != ശൂന്യ) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	എങ്കിൽ (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_ഇടത് = box->padding[ഇടത്_2];
		padding_മുകൾ = box->padding[മുകൾ_2];
		padding_width = padding_ഇടത് + box->width + box->padding[വലത്_2];
		padding_height = padding_മുകൾ + box->height +
				box->padding[BOTTOM];
		border_ഇടത് = box->border[ഇടത്_2].width;
		border_മുകൾ = box->border[മുകൾ_2].width;
		border_വലത് = box->border[വലത്_2].width;
		border_bottom = box->border[BOTTOM].width;
	} അല്ലെങ്കിൽ {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ഇടത് = box->padding[ഇടത്_2] ? box->padding[ഇടത്_2] * scale
				: 0;
		padding_മുകൾ = box->padding[മുകൾ_2] ? box->padding[മുകൾ_2] * scale
				: 0;
		padding_width = (box->padding[ഇടത്_2] + box->width +
				box->padding[വലത്_2]) * scale;
		padding_height = (box->padding[മുകൾ_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_ഇടത് = box->border[ഇടത്_2].width * scale;
		border_മുകൾ = box->border[മുകൾ_2].width * scale;
		border_വലത് = box->border[വലത്_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	എങ്കിൽ (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != ശൂന്യ) {
		/* box contents clipped to box size */
		r.x0 = x - border_ഇടത്;
		r.x1 = x + padding_width + border_വലത്;
	} അല്ലെങ്കിൽ {
		/* box contents can hang out of the box; use descendant box */
		എങ്കിൽ (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} അല്ലെങ്കിൽ {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		എങ്കിൽ (!box->parent) {
			/* root element */
			പൂർണ്ണസംഖ്യ margin_ഇടത്, margin_വലത്;
			എങ്കിൽ (scale == 1.0) {
				margin_ഇടത് = box->margin[ഇടത്_2];
				margin_വലത് = box->margin[വലത്_2];
			} അല്ലെങ്കിൽ {
				margin_ഇടത് = box->margin[ഇടത്_2] * scale;
				margin_വലത് = box->margin[വലത്_2] * scale;
			}
			r.x0 = x - border_ഇടത് - margin_ഇടത് < r.x0 ?
					x - border_ഇടത് - margin_ഇടത് : r.x0;
			r.x1 = x + padding_width + border_വലത് +
					margin_വലത് > r.x1 ?
					x + padding_width + border_വലത് +
					margin_വലത് : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	എങ്കിൽ (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != ശൂന്യ) {
		/* box contents clipped to box size */
		r.y0 = y - border_മുകൾ;
		r.y1 = y + padding_height + border_bottom;
	} അല്ലെങ്കിൽ {
		/* box contents can hang out of the box; use descendant box */
		എങ്കിൽ (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} അല്ലെങ്കിൽ {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		എങ്കിൽ (!box->parent) {
			/* root element */
			പൂർണ്ണസംഖ്യ margin_മുകൾ, margin_bottom;
			എങ്കിൽ (scale == 1.0) {
				margin_മുകൾ = box->margin[മുകൾ_2];
				margin_bottom = box->margin[BOTTOM];
			} അല്ലെങ്കിൽ {
				margin_മുകൾ = box->margin[മുകൾ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_മുകൾ - margin_മുകൾ < r.y0 ?
					y - border_മുകൾ - margin_മുകൾ : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	എങ്കിൽ (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		തിരികെ true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	എങ്കിൽ (html_redraw_printing) {
		എങ്കിൽ (r.y1 > html_redraw_printing_border) {
			എങ്കിൽ (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				എങ്കിൽ (r.y0 < html_redraw_printing_മുകൾ_cropped)
					html_redraw_printing_മുകൾ_cropped = r.y0;
				തിരികെ true;
			}
		}
		അല്ലെങ്കിൽ box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	എങ്കിൽ (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		എങ്കിൽ ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			തിരികെ false;
		എങ്കിൽ (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			തിരികെ false;
		തിരികെ ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	എങ്കിൽ ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		തിരികെ false;
	}

	എങ്കിൽ (box->style != ശൂന്യ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		എങ്കിൽ (css_rect.ഇടത്_auto == false)
			r.x0 = x - border_ഇടത് + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ഇടത്, css_rect.lunit));

		എങ്കിൽ (css_rect.മുകൾ_auto == false)
			r.y0 = y - border_മുകൾ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.മുകൾ, css_rect.tunit));

		എങ്കിൽ (css_rect.വലത്_auto == false)
			r.x1 = x - border_ഇടത് + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.വലത്, css_rect.runit));

		എങ്കിൽ (css_rect.bottom_auto == false)
			r.y1 = y - border_മുകൾ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		എങ്കിൽ (r.x0 < clip->x0) r.x0 = clip->x0;
		എങ്കിൽ (r.y0 < clip->y0) r.y0 = clip->y0;
		എങ്കിൽ (clip->x1 < r.x1) r.x1 = clip->x1;
		എങ്കിൽ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		എങ്കിൽ (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			തിരികെ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			തിരികെ false;

	} അല്ലെങ്കിൽ എങ്കിൽ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		എങ്കിൽ (r.x0 < clip->x0) r.x0 = clip->x0;
		എങ്കിൽ (r.y0 < clip->y0) r.y0 = clip->y0;
		എങ്കിൽ (clip->x1 < r.x1) r.x1 = clip->x1;
		എങ്കിൽ (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		എങ്കിൽ (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			തിരികെ ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			തിരികെ false;
	} അല്ലെങ്കിൽ {
		/* clip box is fine, clip to it */
		r = *clip;
		എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			തിരികെ false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	എങ്കിൽ (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ശൂന്യ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ഘടന rect p;
		p.x0 = x - border_ഇടത് < r.x0 ? r.x0 : x - border_ഇടത്;
		p.y0 = y - border_മുകൾ < r.y0 ? r.y0 : y - border_മുകൾ;
		p.x1 = x + padding_width + border_വലത് < r.x1 ?
				x + padding_width + border_വലത് : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		എങ്കിൽ (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			പൂർണ്ണസംഖ്യ m_ഇടത്, m_മുകൾ, m_വലത്, m_bottom;
			എങ്കിൽ (scale == 1.0) {
				m_ഇടത് = box->margin[ഇടത്_2];
				m_മുകൾ = box->margin[മുകൾ_2];
				m_വലത് = box->margin[വലത്_2];
				m_bottom = box->margin[BOTTOM];
			} അല്ലെങ്കിൽ {
				m_ഇടത് = box->margin[ഇടത്_2] * scale;
				m_മുകൾ = box->margin[മുകൾ_2] * scale;
				m_വലത് = box->margin[വലത്_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ഇടത് < r.x0 ? r.x0 : p.x0 - m_ഇടത്;
			p.y0 = p.y0 - m_മുകൾ < r.y0 ? r.y0 : p.y0 - m_മുകൾ;
			p.x1 = p.x1 + m_വലത് < r.x1 ? p.x1 + m_വലത് : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		എങ്കിൽ ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			എങ്കിൽ (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				തിരികെ false;
			/* restore previous graphics window */
			എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				തിരികെ false;
		}
	}

	/* borders for block level content and replaced inlines */
	എങ്കിൽ (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ശൂന്യ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_മുകൾ || border_വലത് || border_bottom || border_ഇടത്)) {
		എങ്കിൽ (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			തിരികെ false;
	}

	/* backgrounds and borders for non-replaced inlines */
	എങ്കിൽ (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_മുകൾ || border_വലത് ||
			border_bottom || border_ഇടത്)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ഘടന box *ib;
		ഘടന rect b; /* border edge rectangle */
		ഘടന rect p; /* clipped rect */
		bool first = true;
		പൂർണ്ണസംഖ്യ ib_x;
		പൂർണ്ണസംഖ്യ ib_y = y;
		പൂർണ്ണസംഖ്യ ib_p_width;
		പൂർണ്ണസംഖ്യ ib_b_ഇടത്, ib_b_വലത്;

		b.x0 = x - border_ഇടത്;
		b.x1 = x + padding_width + border_വലത്;
		b.y0 = y - border_മുകൾ;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		വേണ്ടി (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			എങ്കിൽ (ib->type == BOX_FLOAT_ഇടത് ||
					ib->type == BOX_FLOAT_വലത്)
				തുടരുക;
			എങ്കിൽ (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[ഇടത്_2] + ib->width +
						ib->padding[വലത്_2];
				ib_b_ഇടത് = ib->border[ഇടത്_2].width;
				ib_b_വലത് = ib->border[വലത്_2].width;
			} അല്ലെങ്കിൽ {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[ഇടത്_2] + ib->width +
						ib->padding[വലത്_2]) * scale;
				ib_b_ഇടത് = ib->border[ഇടത്_2].width * scale;
				ib_b_വലത് = ib->border[വലത്_2].width * scale;
			}

			എങ്കിൽ ((ib->flags & NEW_വരി) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				എങ്കിൽ (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					തിരികെ false;
				/* restore previous graphics window */
				എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					തിരികെ false;
				എങ്കിൽ (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					തിരികെ false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ഇടത്;
				b.y0 = ib_y - border_മുകൾ - padding_മുകൾ;
				b.y1 = ib_y + padding_height - padding_മുകൾ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_വലത്;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			എങ്കിൽ (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				നിർത്തുക;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		എങ്കിൽ (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			തിരികെ false;
		/* restore previous graphics window */
		എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			തിരികെ false;
		എങ്കിൽ (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			തിരികെ false;

	}

	/* Debug outlines */
	എങ്കിൽ (html_redraw_debug) {
		പൂർണ്ണസംഖ്യ margin_ഇടത്, margin_വലത്;
		പൂർണ്ണസംഖ്യ margin_മുകൾ, margin_bottom;
		എങ്കിൽ (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ഇടത് = box->margin[ഇടത്_2];
			margin_മുകൾ = box->margin[മുകൾ_2];
			margin_വലത് = box->margin[വലത്_2];
			margin_bottom = box->margin[BOTTOM];
		} അല്ലെങ്കിൽ {
			margin_ഇടത് = box->margin[ഇടത്_2] * scale;
			margin_മുകൾ = box->margin[മുകൾ_2] * scale;
			margin_വലത് = box->margin[വലത്_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ഇടത്;
		rect.y0 = y + padding_മുകൾ;
		rect.x1 = x + padding_ഇടത് + width;
		rect.y1 = y + padding_മുകൾ + height;
		എങ്കിൽ (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			തിരികെ false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		എങ്കിൽ (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			തിരികെ false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ഇടത് - margin_ഇടത്;
		rect.y0 = y - border_മുകൾ - margin_മുകൾ;
		rect.x1 = x + padding_width + border_വലത് + margin_വലത്;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		എങ്കിൽ (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			തിരികെ false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	എങ്കിൽ (box->parent != ശൂന്യ) {
		bool need_clip = false;
		എങ്കിൽ (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			എങ്കിൽ (r.x0 < clip->x0) r.x0 = clip->x0;
			എങ്കിൽ (r.y0 < clip->y0) r.y0 = clip->y0;
			എങ്കിൽ (clip->x1 < r.x1) r.x1 = clip->x1;
			എങ്കിൽ (clip->y1 < r.y1) r.y1 = clip->y1;
			എങ്കിൽ (r.x1 <= r.x0 || r.y1 <= r.y0) {
				തിരികെ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} അല്ലെങ്കിൽ എങ്കിൽ (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			എങ്കിൽ (r.x0 < clip->x0) r.x0 = clip->x0;
			എങ്കിൽ (clip->x1 < r.x1) r.x1 = clip->x1;
			എങ്കിൽ (r.x1 <= r.x0) {
				തിരികെ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} അല്ലെങ്കിൽ എങ്കിൽ (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			എങ്കിൽ (r.y0 < clip->y0) r.y0 = clip->y0;
			എങ്കിൽ (clip->y1 < r.y1) r.y1 = clip->y1;
			എങ്കിൽ (r.y1 <= r.y0) {
				തിരികെ (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		എങ്കിൽ (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			എങ്കിൽ (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				തിരികെ false;
		}
	}

	/* text decoration */
	എങ്കിൽ ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		എങ്കിൽ (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			തിരികെ false;
	}

	എങ്കിൽ (box->കണ്ണി != ശൂന്യ) {
		exc = dom_html_element_get_tag_type(box->കണ്ണി, &tag_type);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} അല്ലെങ്കിൽ {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	എങ്കിൽ (box->object && width != 0 && height != 0) {
		ഘടന content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_ഇടത്;
		obj_data.y = y_scrolled + padding_മുകൾ;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		എങ്കിൽ (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		എങ്കിൽ (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			സ്ഥിരം അക്ഷരം *obj = "\xef\xbf\xbc";
			പൂർണ്ണസംഖ്യ obj_width;
			പൂർണ്ണസംഖ്യ obj_x = x + padding_ഇടത്;
			nserror res;

			rect.x0 = x + padding_ഇടത്;
			rect.y0 = y + padding_മുകൾ;
			rect.x1 = x + padding_ഇടത് + width - 1;
			rect.y1 = y + padding_മുകൾ + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			എങ്കിൽ (res != NSERROR_OK) {
				തിരികെ false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  വലുപ്പം(obj) - 1,
						  &obj_width);
			എങ്കിൽ (res != NSERROR_OK) {
				obj_x += 1;
			} അല്ലെങ്കിൽ {
				obj_x += width / 2 - obj_width / 2;
			}

			എങ്കിൽ (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_മുകൾ + (പൂർണ്ണസംഖ്യ)(height * 0.75),
					    obj, വലുപ്പം(obj) - 1) != NSERROR_OK)
				തിരികെ false;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->കണ്ണി != ശൂന്യ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ഘടന bitmap *bitmap = ശൂന്യ;
		exc = dom_കണ്ണി_get_user_data(box->കണ്ണി,
					     corestring_dom_ns_താക്കോൽ_canvas_കണ്ണി_data,
					     &bitmap);
		എങ്കിൽ (exc != DOM_NO_ERR) {
			bitmap = ശൂന്യ;
		}
		എങ്കിൽ (bitmap != ശൂന്യ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ഇടത്, y + padding_മുകൾ,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			തിരികെ false;
	} അല്ലെങ്കിൽ എങ്കിൽ (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_ഇടത്,
				y + padding_മുകൾ, &r, ctx);

	} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		എങ്കിൽ (!html_redraw_checkbox(x + padding_ഇടത്, y + padding_മുകൾ,
				width, height, box->gadget->selected, ctx))
			തിരികെ false;

	} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget && box->gadget->type == GADGET_RADIO) {
		എങ്കിൽ (!html_redraw_radio(x + padding_ഇടത്, y + padding_മുകൾ,
				width, height, box->gadget->selected, ctx))
			തിരികെ false;

	} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget && box->gadget->type == GADGET_FILE) {
		എങ്കിൽ (!html_redraw_file(x + padding_ഇടത്, y + padding_മുകൾ,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			തിരികെ false;

	} അല്ലെങ്കിൽ എങ്കിൽ (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} അല്ലെങ്കിൽ എങ്കിൽ (box->text) {
		എങ്കിൽ (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			തിരികെ false;

	} അല്ലെങ്കിൽ {
		എങ്കിൽ (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			തിരികെ false;
	}

	എങ്കിൽ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		എങ്കിൽ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			തിരികെ false;

	/* list marker */
	എങ്കിൽ (box->list_marker) {
		എങ്കിൽ (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			തിരികെ false;
	}

	/* scrollbars */
	എങ്കിൽ (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == ശൂന്യ || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != ശൂന്യ) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ഘടന content *)html,
					    box, has_x_scroll, has_y_scroll);
		എങ്കിൽ (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			തിരികെ false;
		}

		എങ്കിൽ (box->scroll_x != ശൂന്യ)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[മുകൾ_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		എങ്കിൽ (box->scroll_y != ശൂന്യ)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[ഇടത്_2] +
					box->width + box->padding[വലത്_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	എങ്കിൽ (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		എങ്കിൽ (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			തിരികെ false;
	}

	തിരികെ ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(ഘടന content *c, ഘടന content_redraw_data *data,
		സ്ഥിരം ഘടന rect *clip, സ്ഥിരം ഘടന redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ഘടന box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	പ്രതിജ്ഞാ(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	എങ്കിൽ (ctx->interactive && html->visible_select_menu != ശൂന്യ) {
		ഘടന form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	എങ്കിൽ (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		എങ്കിൽ (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	എങ്കിൽ (select) {
		പൂർണ്ണസംഖ്യ menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ഇടത്_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[മുകൾ_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	തിരികെ result;

}
