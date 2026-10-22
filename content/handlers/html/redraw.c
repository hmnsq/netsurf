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

#અન્તર્ભાવ "utils/config.h"
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/browser_window.h"
#અન્તર્ભાવ "netsurf/plotters.h"
#અન્તર્ભાવ "netsurf/bitmap.h"
#અન્તર્ભાવ "netsurf/layout.h"
#અન્તર્ભાવ "content/content.h"
#અન્તર્ભાવ "content/content_protected.h"
#અન્તર્ભાવ "content/textsearch.h"
#અન્તર્ભાવ "css/utils.h"
#અન્તર્ભાવ "desktop/selection.h"
#અન્તર્ભાવ "desktop/print.h"
#અન્તર્ભાવ "desktop/scrollbar.h"
#અન્તર્ભાવ "desktop/textarea.h"
#અન્તર્ભાવ "desktop/gui_internal.h"

#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"
#અન્તર્ભાવ "html/box_manipulate.h"
#અન્તર્ભાવ "html/font.h"
#અન્તર્ભાવ "html/form_internal.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
સ્થિર bool html_redraw_box_has_background(સંરચના box *box)
{
	જો (box->background != ખાલી)
		પાછા true;

	જો (box->style != ખાલી) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		જો (nscss_color_is_transparent(colour) == false)
			પાછા true;
	}

	પાછા false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
સ્થિર સંરચના box *html_redraw_find_bg_box(સંરચના box *box)
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
	જો (box->parent == ખાલી) {
		/* Root box */
		જો (html_redraw_box_has_background(box))
			પાછા box;

		/* No background on root box: consider body box, if any */
		જો (box->children != ખાલી) {
			જો (html_redraw_box_has_background(box->children))
				પાછા box->children;
		}
	} નહીંતર જો (box->parent != ખાલી && box->parent->parent == ખાલી) {
		/* Body box: only render background if root has its own */
		જો (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			પાછા box;
	} નહીંતર {
		/* Any other box */
		જો (html_redraw_box_has_background(box))
			પાછા box;
	}

	પાછા ખાલી;
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

સ્થિર bool
text_redraw(અચલ અક્ષર *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    પૂર્ણાંક space,
	    અચલ plot_font_style_t *fstyle,
	    પૂર્ણાંક x,
	    પૂર્ણાંક y,
	    અચલ સંરચના rect *clip,
	    પૂર્ણાંક height,
	    દશાંશ scale,
	    bool excluded,
	    સંરચના content *c,
	    અચલ સંરચના selection *sel,
	    અચલ સંરચના redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	જો (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		જો (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		જો (!highlighted &&
		    (c->textsearch.context != ખાલી) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		જો (highlighted) {
			સંરચના rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			પૂર્ણાંક startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			જો (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				પ્રતિજ્ઞા(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			જો (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			જો (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			જો (end_idx > utf8_len) {
					endx += space;
			}

			જો (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			જો ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (પૂર્ણાંક)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				પાછા false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			જો (res != NSERROR_OK) {
				પાછા false;
			}

			જો (start_idx > 0) {
				પૂર્ણાંક px0 = max(x + startx, clip->x0);
				પૂર્ણાંક px1 = min(x + endx, clip->x1);

				જો (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					જો (res != NSERROR_OK) {
						પાછા false;
					}

					clip_changed = true;
				} નહીંતર {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			જો (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (પૂર્ણાંક)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				પાછા false;
			}

			/* draw any text succeeding highlighted portion */
			જો (endtxt_idx < utf8_len) {
				પૂર્ણાંક px0 = max(x + endx, clip->x0);
				જો (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					જો (res != NSERROR_OK) {
						પાછા false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (પૂર્ણાંક)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					જો (res != NSERROR_OK) {
						પાછા false;
					}
				}
			}

			જો (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				પાછા false;
			}
		}
	}

	જો (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (પૂર્ણાંક) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}
	પાછા true;
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

સ્થિર bool html_redraw_checkbox(પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height,
		bool selected, અચલ સંરચના redraw_context *ctx)
{
	દ્વિગુણ z;
	nserror res;
	સંરચના rect rect;

	z = width * 0.15;
	જો (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->લીટી(ctx, plot_style_stroke_darkwbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->લીટી(ctx, plot_style_stroke_darkwbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->લીટી(ctx, plot_style_stroke_lightwbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->લીટી(ctx, plot_style_stroke_lightwbasec, &rect);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	જો (selected) {
		જો (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		} નહીંતર {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->લીટી(ctx, plot_style_stroke_wblobc, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->લીટી(ctx, plot_style_stroke_wblobc, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		}
	}
	પાછા true;
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
સ્થિર bool html_redraw_radio(પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height,
		bool selected, અચલ સંરચના redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	જો (res != NSERROR_OK) {
		પાછા false;
	}

	જો (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}

	પાછા true;
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

સ્થિર bool html_redraw_file(પૂર્ણાંક x, પૂર્ણાંક y, પૂર્ણાંક width, પૂર્ણાંક height,
		સંરચના box *box, દશાંશ scale, colour background_colour,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ સંરચના redraw_context *ctx)
{
	પૂર્ણાંક text_width;
	અચલ અક્ષર *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	જો (box->gadget->કિંમત) {
		text = box->gadget->કિંમત;
	} નહીંતર {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	જો (res != NSERROR_OK) {
		પાછા false;
	}
	text_width *= scale;
	જો (width < text_width + 8) {
		x = x + width - text_width - 4;
	} નહીંતર {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	જો (res != NSERROR_OK) {
		પાછા false;
	}
	પાછા true;
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

સ્થિર bool html_redraw_background(પૂર્ણાંક x, પૂર્ણાંક y, સંરચના box *box, દશાંશ scale,
		અચલ સંરચના rect *clip, colour *background_colour,
		સંરચના box *background,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ સંરચના redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	સંરચના box *clip_box = box;
	પૂર્ણાંક ox = x, oy = y;
	પૂર્ણાંક width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	સંરચના box *parent;
	સંરચના rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	જો (ctx->background_images == false)
		પાછા true;

	plot_content = (background->background != ખાલી);

	જો (plot_content) {
		જો (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[ડાબું_2] * scale;
			y -= box->margin[ટોચ_2] * scale;
			width = box->margin[ડાબું_2] + box->padding[ડાબું_2] +
					box->width + box->padding[જમણું_2] +
					box->margin[જમણું_2];
			height = box->margin[ટોચ_2] + box->padding[ટોચ_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} નહીંતર {
			width = box->padding[ડાબું_2] + box->width +
					box->padding[જમણું_2];
			height = box->padding[ટોચ_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		પસંદગી (css_computed_background_repeat(background->style)) {
		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			તોડો;

		મૂળભૂત:
			તોડો;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		જો (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} નહીંતર {
			x += (પૂર્ણાંક) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		જો (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} નહીંતર {
			y += (પૂર્ણાંક) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	જો (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		માટે (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		પ્રતિજ્ઞા(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		જો (clip_to_children)
			clip_box = box->children;
	}

	માટે (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		જો (clip_to_children) {
			પ્રતિજ્ઞા(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[ડાબું_2] +
					clip_box->width +
					clip_box->padding[જમણું_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[ટોચ_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			જો (r.x0 < clip->x0) r.x0 = clip->x0;
			જો (r.y0 < clip->y0) r.y0 = clip->y0;
			જો (r.x1 > clip->x1) r.x1 = clip->x1;
			જો (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			જો ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ખાલી) &&
			     content_get_opaque(clip_box->background)))
				ચાલુ;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		જો (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			જો (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				જો (res != NSERROR_OK) {
					પાછા false;
				}
			}
		}
		/* and plot the image */
		જો (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			જો (!repeat_x) {
				જો (r.x0 < x)
					r.x0 = x;
				જો (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			જો (!repeat_y) {
				જો (r.y0 < y)
					r.y0 = y;
				જો (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			જો ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				સંરચના content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				જો (res != NSERROR_OK) {
					પાછા false;
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
		જો (!clip_to_children)
			પાછા true;
	}
	પાછા true;
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

સ્થિર bool html_redraw_inline_background(પૂર્ણાંક x, પૂર્ણાંક y, સંરચના box *box,
		દશાંશ scale, અચલ સંરચના rect *clip, સંરચના rect b,
		bool first, bool last, colour *background_colour,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ સંરચના redraw_context *ctx)
{
	સંરચના rect r = *clip;
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

	plot_content = (box->background != ખાલી);

	જો (html_redraw_printing && nsoption_bool(remove_backgrounds))
		પાછા true;

	જો (plot_content) {
		/* handle background-repeat */
		પસંદગી (css_computed_background_repeat(box->style)) {
		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			તોડો;

		સ્થિતિ CSS_BACKGROUND_REPEAT_NO_REPEAT:
			તોડો;

		મૂળભૂત:
			તોડો;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		જો (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			જો (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} નહીંતર {
			x += (પૂર્ણાંક) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		જો (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} નહીંતર {
			y += (પૂર્ણાંક) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	જો (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		જો (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		}
	}
	/* and plot the image */
	જો (plot_content) {
		પૂર્ણાંક width = content_get_width(box->background);
		પૂર્ણાંક height = content_get_height(box->background);

		જો (!repeat_x) {
			જો (r.x0 < x)
				r.x0 = x;
			જો (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		જો (!repeat_y) {
			જો (r.y0 < y)
				r.y0 = y;
			જો (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		જો ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			સંરચના content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			જો (res != NSERROR_OK) {
				પાછા false;
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

	પાછા true;
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

સ્થિર bool
html_redraw_text_decoration_inline(સંરચના box *box,
				   પૂર્ણાંક x, પૂર્ણાંક y,
				   દશાંશ scale,
				   colour colour,
				   દશાંશ ratio,
				   અચલ સંરચના redraw_context *ctx)
{
	સંરચના box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	સંરચના rect rect;

	માટે (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		જો (c->type != BOX_TEXT) {
			ચાલુ;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->લીટી(ctx, &plot_style_box, &rect);
		જો (res != NSERROR_OK) {
			પાછા false;
		}
	}
	પાછા true;
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

સ્થિર bool
html_redraw_text_decoration_block(સંરચના box *box,
				  પૂર્ણાંક x, પૂર્ણાંક y,
				  દશાંશ scale,
				  colour colour,
				  દશાંશ ratio,
				  અચલ સંરચના redraw_context *ctx)
{
	સંરચના box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	સંરચના rect rect;

	/* draw through text descendants */
	માટે (c = box->children; c; c = c->next) {
		જો (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->લીટી(ctx, &plot_style_box, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}
		} નહીંતર જો ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			જો (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				પાછા false;
		}
	}
	પાછા true;
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

સ્થિર bool html_redraw_text_decoration(સંરચના box *box,
		પૂર્ણાંક x_parent, પૂર્ણાંક y_parent, દશાંશ scale,
		colour background_colour, અચલ સંરચના redraw_context *ctx)
{
	સ્થિર અચલ ગણના css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_લીટી_THROUGH };
	સ્થિર અચલ દશાંશ લીટી_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned પૂર્ણાંક i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	જો (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	જો (box->type == BOX_INLINE) {
		જો (!box->inline_end)
			પાછા true;
		માટે (i = 0; i != NOF_ELEMENTS(decoration); i++)
			જો (css_computed_text_decoration(box->style) &
					decoration[i])
				જો (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, લીટી_ratio[i], ctx))
					પાછા false;
	} નહીંતર {
		માટે (i = 0; i != NOF_ELEMENTS(decoration); i++)
			જો (css_computed_text_decoration(box->style) &
					decoration[i])
				જો (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, લીટી_ratio[i], ctx))
					પાછા false;
	}

	પાછા true;
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

સ્થિર bool html_redraw_text_box(અચલ html_content *html, સંરચના box *box,
		પૂર્ણાંક x, પૂર્ણાંક y, અચલ સંરચના rect *clip, દશાંશ scale,
		colour current_background_color,
		અચલ સંરચના redraw_context *ctx)
{
	bool excluded = (box->object != ખાલી);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	જો (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (સંરચના content *)html,
			 html->sel,
			 ctx))
		પાછા false;

	પાછા true;
}

bool html_redraw_box(અચલ html_content *html, સંરચના box *box,
		પૂર્ણાંક x_parent, પૂર્ણાંક y_parent,
		અચલ સંરચના rect *clip, દશાંશ scale,
		colour current_background_color,
		અચલ સંરચના redraw_context *ctx);

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

સ્થિર bool html_redraw_box_children(અચલ html_content *html, સંરચના box *box,
		પૂર્ણાંક x_parent, પૂર્ણાંક y_parent,
		અચલ સંરચના rect *clip, દશાંશ scale,
		colour current_background_color,
		અચલ સંરચના redraw_context *ctx)
{
	સંરચના box *c;

	માટે (c = box->children; c; c = c->next) {

		જો (c->type != BOX_FLOAT_ડાબું && c->type != BOX_FLOAT_જમણું)
			જો (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				પાછા false;
	}
	માટે (c = box->float_children; c; c = c->next_float)
		જો (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			પાછા false;

	પાછા true;
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

bool html_redraw_box(અચલ html_content *html, સંરચના box *box,
		પૂર્ણાંક x_parent, પૂર્ણાંક y_parent,
		અચલ સંરચના rect *clip, અચલ દશાંશ scale,
		colour current_background_color,
		અચલ સંરચના redraw_context *ctx)
{
	અચલ સંરચના plotter_table *plot = ctx->plot;
	પૂર્ણાંક x, y;
	પૂર્ણાંક width, height;
	પૂર્ણાંક padding_ડાબું, padding_ટોચ, padding_width, padding_height;
	પૂર્ણાંક border_ડાબું, border_ટોચ, border_જમણું, border_bottom;
	સંરચના rect r;
	સંરચના rect rect;
	પૂર્ણાંક x_scrolled, y_scrolled;
	સંરચના box *bg_box = ખાલી;
	css_computed_clip_rect css_rect;
	ગણના css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ગણના css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	જો (html_redraw_printing && (box->flags & PRINTED))
		પાછા true;

	જો (box->style != ખાલી) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	જો (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_ડાબું = box->padding[ડાબું_2];
		padding_ટોચ = box->padding[ટોચ_2];
		padding_width = padding_ડાબું + box->width + box->padding[જમણું_2];
		padding_height = padding_ટોચ + box->height +
				box->padding[BOTTOM];
		border_ડાબું = box->border[ડાબું_2].width;
		border_ટોચ = box->border[ટોચ_2].width;
		border_જમણું = box->border[જમણું_2].width;
		border_bottom = box->border[BOTTOM].width;
	} નહીંતર {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_ડાબું = box->padding[ડાબું_2] ? box->padding[ડાબું_2] * scale
				: 0;
		padding_ટોચ = box->padding[ટોચ_2] ? box->padding[ટોચ_2] * scale
				: 0;
		padding_width = (box->padding[ડાબું_2] + box->width +
				box->padding[જમણું_2]) * scale;
		padding_height = (box->padding[ટોચ_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_ડાબું = box->border[ડાબું_2].width * scale;
		border_ટોચ = box->border[ટોચ_2].width * scale;
		border_જમણું = box->border[જમણું_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	જો (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != ખાલી) {
		/* box contents clipped to box size */
		r.x0 = x - border_ડાબું;
		r.x1 = x + padding_width + border_જમણું;
	} નહીંતર {
		/* box contents can hang out of the box; use descendant box */
		જો (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} નહીંતર {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		જો (!box->parent) {
			/* root element */
			પૂર્ણાંક margin_ડાબું, margin_જમણું;
			જો (scale == 1.0) {
				margin_ડાબું = box->margin[ડાબું_2];
				margin_જમણું = box->margin[જમણું_2];
			} નહીંતર {
				margin_ડાબું = box->margin[ડાબું_2] * scale;
				margin_જમણું = box->margin[જમણું_2] * scale;
			}
			r.x0 = x - border_ડાબું - margin_ડાબું < r.x0 ?
					x - border_ડાબું - margin_ડાબું : r.x0;
			r.x1 = x + padding_width + border_જમણું +
					margin_જમણું > r.x1 ?
					x + padding_width + border_જમણું +
					margin_જમણું : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	જો (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != ખાલી) {
		/* box contents clipped to box size */
		r.y0 = y - border_ટોચ;
		r.y1 = y + padding_height + border_bottom;
	} નહીંતર {
		/* box contents can hang out of the box; use descendant box */
		જો (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} નહીંતર {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		જો (!box->parent) {
			/* root element */
			પૂર્ણાંક margin_ટોચ, margin_bottom;
			જો (scale == 1.0) {
				margin_ટોચ = box->margin[ટોચ_2];
				margin_bottom = box->margin[BOTTOM];
			} નહીંતર {
				margin_ટોચ = box->margin[ટોચ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_ટોચ - margin_ટોચ < r.y0 ?
					y - border_ટોચ - margin_ટોચ : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	જો (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		પાછા true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	જો (html_redraw_printing) {
		જો (r.y1 > html_redraw_printing_border) {
			જો (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				જો (r.y0 < html_redraw_printing_ટોચ_cropped)
					html_redraw_printing_ટોચ_cropped = r.y0;
				પાછા true;
			}
		}
		નહીંતર box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	જો (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		જો ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			પાછા false;
		જો (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			પાછા false;
		પાછા ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	જો ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		પાછા false;
	}

	જો (box->style != ખાલી &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		જો (css_rect.ડાબું_auto == false)
			r.x0 = x - border_ડાબું + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ડાબું, css_rect.lunit));

		જો (css_rect.ટોચ_auto == false)
			r.y0 = y - border_ટોચ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ટોચ, css_rect.tunit));

		જો (css_rect.જમણું_auto == false)
			r.x1 = x - border_ડાબું + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.જમણું, css_rect.runit));

		જો (css_rect.bottom_auto == false)
			r.y1 = y - border_ટોચ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		જો (r.x0 < clip->x0) r.x0 = clip->x0;
		જો (r.y0 < clip->y0) r.y0 = clip->y0;
		જો (clip->x1 < r.x1) r.x1 = clip->x1;
		જો (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		જો (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			પાછા ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			પાછા false;

	} નહીંતર જો (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		જો (r.x0 < clip->x0) r.x0 = clip->x0;
		જો (r.y0 < clip->y0) r.y0 = clip->y0;
		જો (clip->x1 < r.x1) r.x1 = clip->x1;
		જો (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		જો (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			પાછા ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			પાછા false;
	} નહીંતર {
		/* clip box is fine, clip to it */
		r = *clip;
		જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			પાછા false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	જો (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ખાલી &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		સંરચના rect p;
		p.x0 = x - border_ડાબું < r.x0 ? r.x0 : x - border_ડાબું;
		p.y0 = y - border_ટોચ < r.y0 ? r.y0 : y - border_ટોચ;
		p.x1 = x + padding_width + border_જમણું < r.x1 ?
				x + padding_width + border_જમણું : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		જો (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			પૂર્ણાંક m_ડાબું, m_ટોચ, m_જમણું, m_bottom;
			જો (scale == 1.0) {
				m_ડાબું = box->margin[ડાબું_2];
				m_ટોચ = box->margin[ટોચ_2];
				m_જમણું = box->margin[જમણું_2];
				m_bottom = box->margin[BOTTOM];
			} નહીંતર {
				m_ડાબું = box->margin[ડાબું_2] * scale;
				m_ટોચ = box->margin[ટોચ_2] * scale;
				m_જમણું = box->margin[જમણું_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_ડાબું < r.x0 ? r.x0 : p.x0 - m_ડાબું;
			p.y0 = p.y0 - m_ટોચ < r.y0 ? r.y0 : p.y0 - m_ટોચ;
			p.x1 = p.x1 + m_જમણું < r.x1 ? p.x1 + m_જમણું : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		જો ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			જો (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				પાછા false;
			/* restore previous graphics window */
			જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				પાછા false;
		}
	}

	/* borders for block level content and replaced inlines */
	જો (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ખાલી &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_ટોચ || border_જમણું || border_bottom || border_ડાબું)) {
		જો (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			પાછા false;
	}

	/* backgrounds and borders for non-replaced inlines */
	જો (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_ટોચ || border_જમણું ||
			border_bottom || border_ડાબું)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		સંરચના box *ib;
		સંરચના rect b; /* border edge rectangle */
		સંરચના rect p; /* clipped rect */
		bool first = true;
		પૂર્ણાંક ib_x;
		પૂર્ણાંક ib_y = y;
		પૂર્ણાંક ib_p_width;
		પૂર્ણાંક ib_b_ડાબું, ib_b_જમણું;

		b.x0 = x - border_ડાબું;
		b.x1 = x + padding_width + border_જમણું;
		b.y0 = y - border_ટોચ;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		માટે (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			જો (ib->type == BOX_FLOAT_ડાબું ||
					ib->type == BOX_FLOAT_જમણું)
				ચાલુ;
			જો (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[ડાબું_2] + ib->width +
						ib->padding[જમણું_2];
				ib_b_ડાબું = ib->border[ડાબું_2].width;
				ib_b_જમણું = ib->border[જમણું_2].width;
			} નહીંતર {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[ડાબું_2] + ib->width +
						ib->padding[જમણું_2]) * scale;
				ib_b_ડાબું = ib->border[ડાબું_2].width * scale;
				ib_b_જમણું = ib->border[જમણું_2].width * scale;
			}

			જો ((ib->flags & NEW_લીટી) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				જો (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					પાછા false;
				/* restore previous graphics window */
				જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					પાછા false;
				જો (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					પાછા false;
				/* reset coords */
				b.x0 = ib_x - ib_b_ડાબું;
				b.y0 = ib_y - border_ટોચ - padding_ટોચ;
				b.y1 = ib_y + padding_height - padding_ટોચ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_જમણું;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			જો (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				તોડો;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		જો (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			પાછા false;
		/* restore previous graphics window */
		જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			પાછા false;
		જો (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			પાછા false;

	}

	/* Debug outlines */
	જો (html_redraw_debug) {
		પૂર્ણાંક margin_ડાબું, margin_જમણું;
		પૂર્ણાંક margin_ટોચ, margin_bottom;
		જો (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_ડાબું = box->margin[ડાબું_2];
			margin_ટોચ = box->margin[ટોચ_2];
			margin_જમણું = box->margin[જમણું_2];
			margin_bottom = box->margin[BOTTOM];
		} નહીંતર {
			margin_ડાબું = box->margin[ડાબું_2] * scale;
			margin_ટોચ = box->margin[ટોચ_2] * scale;
			margin_જમણું = box->margin[જમણું_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_ડાબું;
		rect.y0 = y + padding_ટોચ;
		rect.x1 = x + padding_ડાબું + width;
		rect.y1 = y + padding_ટોચ + height;
		જો (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			પાછા false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		જો (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			પાછા false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_ડાબું - margin_ડાબું;
		rect.y0 = y - border_ટોચ - margin_ટોચ;
		rect.x1 = x + padding_width + border_જમણું + margin_જમણું;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		જો (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			પાછા false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	જો (box->parent != ખાલી) {
		bool need_clip = false;
		જો (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			જો (r.x0 < clip->x0) r.x0 = clip->x0;
			જો (r.y0 < clip->y0) r.y0 = clip->y0;
			જો (clip->x1 < r.x1) r.x1 = clip->x1;
			જો (clip->y1 < r.y1) r.y1 = clip->y1;
			જો (r.x1 <= r.x0 || r.y1 <= r.y0) {
				પાછા (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} નહીંતર જો (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			જો (r.x0 < clip->x0) r.x0 = clip->x0;
			જો (clip->x1 < r.x1) r.x1 = clip->x1;
			જો (r.x1 <= r.x0) {
				પાછા (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} નહીંતર જો (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			જો (r.y0 < clip->y0) r.y0 = clip->y0;
			જો (clip->y1 < r.y1) r.y1 = clip->y1;
			જો (r.y1 <= r.y0) {
				પાછા (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		જો (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			જો (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				પાછા false;
		}
	}

	/* text decoration */
	જો ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		જો (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			પાછા false;
	}

	જો (box->ગાંઠ != ખાલી) {
		exc = dom_html_element_get_tag_type(box->ગાંઠ, &tag_type);
		જો (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} નહીંતર {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	જો (box->object && width != 0 && height != 0) {
		સંરચના content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_ડાબું;
		obj_data.y = y_scrolled + padding_ટોચ;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		જો (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		જો (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			અચલ અક્ષર *obj = "\xef\xbf\xbc";
			પૂર્ણાંક obj_width;
			પૂર્ણાંક obj_x = x + padding_ડાબું;
			nserror res;

			rect.x0 = x + padding_ડાબું;
			rect.y0 = y + padding_ટોચ;
			rect.x1 = x + padding_ડાબું + width - 1;
			rect.y1 = y + padding_ટોચ + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			જો (res != NSERROR_OK) {
				પાછા false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  કદ(obj) - 1,
						  &obj_width);
			જો (res != NSERROR_OK) {
				obj_x += 1;
			} નહીંતર {
				obj_x += width / 2 - obj_width / 2;
			}

			જો (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_ટોચ + (પૂર્ણાંક)(height * 0.75),
					    obj, કદ(obj) - 1) != NSERROR_OK)
				પાછા false;
		}
	} નહીંતર જો (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ગાંઠ != ખાલી &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		સંરચના bitmap *bitmap = ખાલી;
		exc = dom_ગાંઠ_get_user_data(box->ગાંઠ,
					     corestring_dom_ns_ચાવી_canvas_ગાંઠ_data,
					     &bitmap);
		જો (exc != DOM_NO_ERR) {
			bitmap = ખાલી;
		}
		જો (bitmap != ખાલી &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_ડાબું, y + padding_ટોચ,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			પાછા false;
	} નહીંતર જો (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_ડાબું,
				y + padding_ટોચ, &r, ctx);

	} નહીંતર જો (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		જો (!html_redraw_checkbox(x + padding_ડાબું, y + padding_ટોચ,
				width, height, box->gadget->selected, ctx))
			પાછા false;

	} નહીંતર જો (box->gadget && box->gadget->type == GADGET_RADIO) {
		જો (!html_redraw_radio(x + padding_ડાબું, y + padding_ટોચ,
				width, height, box->gadget->selected, ctx))
			પાછા false;

	} નહીંતર જો (box->gadget && box->gadget->type == GADGET_FILE) {
		જો (!html_redraw_file(x + padding_ડાબું, y + padding_ટોચ,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			પાછા false;

	} નહીંતર જો (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} નહીંતર જો (box->text) {
		જો (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			પાછા false;

	} નહીંતર {
		જો (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			પાછા false;
	}

	જો (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		જો (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			પાછા false;

	/* list marker */
	જો (box->list_marker) {
		જો (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			પાછા false;
	}

	/* scrollbars */
	જો (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == ખાલી || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != ખાલી) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((સંરચના content *)html,
					    box, has_x_scroll, has_y_scroll);
		જો (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			પાછા false;
		}

		જો (box->scroll_x != ખાલી)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[ટોચ_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		જો (box->scroll_y != ખાલી)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[ડાબું_2] +
					box->width + box->padding[જમણું_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	જો (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		જો (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			પાછા false;
	}

	પાછા ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(સંરચના content *c, સંરચના content_redraw_data *data,
		અચલ સંરચના rect *clip, અચલ સંરચના redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	સંરચના box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	પ્રતિજ્ઞા(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	જો (ctx->interactive && html->visible_select_menu != ખાલી) {
		સંરચના form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	જો (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		જો (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	જો (select) {
		પૂર્ણાંક menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[ડાબું_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[ટોચ_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	પાછા result;

}
