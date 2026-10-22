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

#include "utils/config.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dom/dom.h>

#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/nsoption.h"
#include "utils/corestrings.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
#include "netsurf/layout.h"
#include "content/content.h"
#include "content/content_protected.h"
#include "content/textsearch.h"
#include "css/utils.h"
#include "desktop/selection.h"
#include "desktop/print.h"
#include "desktop/scrollbar.h"
#include "desktop/textarea.h"
#include "desktop/gui_internal.h"

#include "html/box.h"
#include "html/box_inspect.h"
#include "html/box_manipulate.h"
#include "html/font.h"
#include "html/form_internal.h"
#include "html/private.h"
#include "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
static bool html_redraw_box_has_background(sa_nracanaa box *box)
{
	ถ้า (box->background != NULL)
		คืน true;

	ถ้า (box->style != NULL) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		ถ้า (nscss_color_is_transparent(colour) == false)
			คืน true;
	}

	คืน false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
static sa_nracanaa box *html_redraw_find_bg_box(sa_nracanaa box *box)
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
	ถ้า (box->parent == NULL) {
		/* Root box */
		ถ้า (html_redraw_box_has_background(box))
			คืน box;

		/* No background on root box: consider body box, if any */
		ถ้า (box->children != NULL) {
			ถ้า (html_redraw_box_has_background(box->children))
				คืน box->children;
		}
	} มิฉะนั้น ถ้า (box->parent != NULL && box->parent->parent == NULL) {
		/* Body box: only render background if root has its own */
		ถ้า (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			คืน box;
	} มิฉะนั้น {
		/* Any other box */
		ถ้า (html_redraw_box_has_background(box))
			คืน box;
	}

	คืน NULL;
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

static bool
text_redraw(const var_na *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    จำนวนเต็ม space,
	    const plot_font_style_t *fstyle,
	    จำนวนเต็ม x,
	    จำนวนเต็ม y,
	    const sa_nracanaa rect *clip,
	    จำนวนเต็ม height,
	    float scale,
	    bool excluded,
	    sa_nracanaa content *c,
	    const sa_nracanaa selection *sel,
	    const sa_nracanaa redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	ถ้า (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		ถ้า (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		ถ้า (!highlighted &&
		    (c->textsearch.context != NULL) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		ถ้า (highlighted) {
			sa_nracanaa rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			จำนวนเต็ม startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			ถ้า (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				assert(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			ถ้า (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			ถ้า (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			ถ้า (end_idx > utf8_len) {
					endx += space;
			}

			ถ้า (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			ถ้า ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (จำนวนเต็ม)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				คืน false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}

			ถ้า (start_idx > 0) {
				จำนวนเต็ม px0 = max(x + startx, clip->x0);
				จำนวนเต็ม px1 = min(x + endx, clip->x1);

				ถ้า (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ถ้า (res != NSERROR_OK) {
						คืน false;
					}

					clip_changed = true;
				} มิฉะนั้น {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			ถ้า (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (จำนวนเต็ม)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				คืน false;
			}

			/* draw any text succeeding highlighted portion */
			ถ้า (endtxt_idx < utf8_len) {
				จำนวนเต็ม px0 = max(x + endx, clip->x0);
				ถ้า (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					ถ้า (res != NSERROR_OK) {
						คืน false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (จำนวนเต็ม)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					ถ้า (res != NSERROR_OK) {
						คืน false;
					}
				}
			}

			ถ้า (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				คืน false;
			}
		}
	}

	ถ้า (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (จำนวนเต็ม) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		ถ้า (res != NSERROR_OK) {
			คืน false;
		}
	}
	คืน true;
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

static bool html_redraw_checkbox(จำนวนเต็ม x, จำนวนเต็ม y, จำนวนเต็ม width, จำนวนเต็ม height,
		bool selected, const sa_nracanaa redraw_context *ctx)
{
	dviguu_na z;
	nserror res;
	sa_nracanaa rect rect;

	z = width * 0.15;
	ถ้า (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->line(ctx, plot_style_stroke_darkwbasec, &rect);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->line(ctx, plot_style_stroke_darkwbasec, &rect);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->line(ctx, plot_style_stroke_lightwbasec, &rect);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->line(ctx, plot_style_stroke_lightwbasec, &rect);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	ถ้า (selected) {
		ถ้า (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}
		} มิฉะนั้น {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->line(ctx, plot_style_stroke_wblobc, &rect);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->line(ctx, plot_style_stroke_wblobc, &rect);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}
		}
	}
	คืน true;
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
static bool html_redraw_radio(จำนวนเต็ม x, จำนวนเต็ม y, จำนวนเต็ม width, จำนวนเต็ม height,
		bool selected, const sa_nracanaa redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}

	ถ้า (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		ถ้า (res != NSERROR_OK) {
			คืน false;
		}
	}

	คืน true;
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

static bool html_redraw_file(จำนวนเต็ม x, จำนวนเต็ม y, จำนวนเต็ม width, จำนวนเต็ม height,
		sa_nracanaa box *box, float scale, colour background_colour,
		const css_unit_ctx *unit_len_ctx,
		const sa_nracanaa redraw_context *ctx)
{
	จำนวนเต็ม text_width;
	const var_na *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	ถ้า (box->gadget->value) {
		text = box->gadget->value;
	} มิฉะนั้น {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}
	text_width *= scale;
	ถ้า (width < text_width + 8) {
		x = x + width - text_width - 4;
	} มิฉะนั้น {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	ถ้า (res != NSERROR_OK) {
		คืน false;
	}
	คืน true;
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

static bool html_redraw_background(จำนวนเต็ม x, จำนวนเต็ม y, sa_nracanaa box *box, float scale,
		const sa_nracanaa rect *clip, colour *background_colour,
		sa_nracanaa box *background,
		const css_unit_ctx *unit_len_ctx,
		const sa_nracanaa redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	sa_nracanaa box *clip_box = box;
	จำนวนเต็ม ox = x, oy = y;
	จำนวนเต็ม width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	sa_nracanaa box *parent;
	sa_nracanaa rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	ถ้า (ctx->background_images == false)
		คืน true;

	plot_content = (background->background != NULL);

	ถ้า (plot_content) {
		ถ้า (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[LEFT] * scale;
			y -= box->margin[TOP] * scale;
			width = box->margin[LEFT] + box->padding[LEFT] +
					box->width + box->padding[RIGHT] +
					box->margin[RIGHT];
			height = box->margin[TOP] + box->padding[TOP] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} มิฉะนั้น {
			width = box->padding[LEFT] + box->width +
					box->padding[RIGHT];
			height = box->padding[TOP] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		switch (css_computed_background_repeat(background->style)) {
		pak_sa CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_NO_REPEAT:
			bha_nga;

		default:
			bha_nga;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		ถ้า (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} มิฉะนั้น {
			x += (จำนวนเต็ม) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ถ้า (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} มิฉะนั้น {
			y += (จำนวนเต็ม) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	ถ้า (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		สำหรับ (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		assert(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		ถ้า (clip_to_children)
			clip_box = box->children;
	}

	สำหรับ (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		ถ้า (clip_to_children) {
			assert(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[LEFT] +
					clip_box->width +
					clip_box->padding[RIGHT]) * scale;
			r.y1 = r.y0 + (clip_box->padding[TOP] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			ถ้า (r.x0 < clip->x0) r.x0 = clip->x0;
			ถ้า (r.y0 < clip->y0) r.y0 = clip->y0;
			ถ้า (r.x1 > clip->x1) r.x1 = clip->x1;
			ถ้า (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			ถ้า ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != NULL) &&
			     content_get_opaque(clip_box->background)))
				continue;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		ถ้า (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			ถ้า (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				ถ้า (res != NSERROR_OK) {
					คืน false;
				}
			}
		}
		/* and plot the image */
		ถ้า (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			ถ้า (!repeat_x) {
				ถ้า (r.x0 < x)
					r.x0 = x;
				ถ้า (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			ถ้า (!repeat_y) {
				ถ้า (r.y0 < y)
					r.y0 = y;
				ถ้า (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			ถ้า ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				sa_nracanaa content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				ถ้า (res != NSERROR_OK) {
					คืน false;
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
		ถ้า (!clip_to_children)
			คืน true;
	}
	คืน true;
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

static bool html_redraw_inline_background(จำนวนเต็ม x, จำนวนเต็ม y, sa_nracanaa box *box,
		float scale, const sa_nracanaa rect *clip, sa_nracanaa rect b,
		bool first, bool last, colour *background_colour,
		const css_unit_ctx *unit_len_ctx,
		const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa rect r = *clip;
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

	plot_content = (box->background != NULL);

	ถ้า (html_redraw_printing && nsoption_bool(remove_backgrounds))
		คืน true;

	ถ้า (plot_content) {
		/* handle background-repeat */
		switch (css_computed_background_repeat(box->style)) {
		pak_sa CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			bha_nga;

		pak_sa CSS_BACKGROUND_REPEAT_NO_REPEAT:
			bha_nga;

		default:
			bha_nga;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		ถ้า (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			ถ้า (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} มิฉะนั้น {
			x += (จำนวนเต็ม) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		ถ้า (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} มิฉะนั้น {
			y += (จำนวนเต็ม) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	ถ้า (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		ถ้า (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}
		}
	}
	/* and plot the image */
	ถ้า (plot_content) {
		จำนวนเต็ม width = content_get_width(box->background);
		จำนวนเต็ม height = content_get_height(box->background);

		ถ้า (!repeat_x) {
			ถ้า (r.x0 < x)
				r.x0 = x;
			ถ้า (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		ถ้า (!repeat_y) {
			ถ้า (r.y0 < y)
				r.y0 = y;
			ถ้า (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		ถ้า ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			sa_nracanaa content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			ถ้า (res != NSERROR_OK) {
				คืน false;
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

	คืน true;
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

static bool
html_redraw_text_decoration_inline(sa_nracanaa box *box,
				   จำนวนเต็ม x, จำนวนเต็ม y,
				   float scale,
				   colour colour,
				   float ratio,
				   const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	sa_nracanaa rect rect;

	สำหรับ (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		ถ้า (c->type != BOX_TEXT) {
			continue;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->line(ctx, &plot_style_box, &rect);
		ถ้า (res != NSERROR_OK) {
			คืน false;
		}
	}
	คืน true;
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

static bool
html_redraw_text_decoration_block(sa_nracanaa box *box,
				  จำนวนเต็ม x, จำนวนเต็ม y,
				  float scale,
				  colour colour,
				  float ratio,
				  const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	sa_nracanaa rect rect;

	/* draw through text descendants */
	สำหรับ (c = box->children; c; c = c->next) {
		ถ้า (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->line(ctx, &plot_style_box, &rect);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}
		} มิฉะนั้น ถ้า ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			ถ้า (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				คืน false;
		}
	}
	คืน true;
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

static bool html_redraw_text_decoration(sa_nracanaa box *box,
		จำนวนเต็ม x_parent, จำนวนเต็ม y_parent, float scale,
		colour background_colour, const sa_nracanaa redraw_context *ctx)
{
	static const ga_nanaa css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_LINE_THROUGH };
	static const float line_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned จำนวนเต็ม i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	ถ้า (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	ถ้า (box->type == BOX_INLINE) {
		ถ้า (!box->inline_end)
			คืน true;
		สำหรับ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ถ้า (css_computed_text_decoration(box->style) &
					decoration[i])
				ถ้า (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, line_ratio[i], ctx))
					คืน false;
	} มิฉะนั้น {
		สำหรับ (i = 0; i != NOF_ELEMENTS(decoration); i++)
			ถ้า (css_computed_text_decoration(box->style) &
					decoration[i])
				ถ้า (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, line_ratio[i], ctx))
					คืน false;
	}

	คืน true;
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

static bool html_redraw_text_box(const html_content *html, sa_nracanaa box *box,
		จำนวนเต็ม x, จำนวนเต็ม y, const sa_nracanaa rect *clip, float scale,
		colour current_background_color,
		const sa_nracanaa redraw_context *ctx)
{
	bool excluded = (box->object != NULL);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	ถ้า (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (sa_nracanaa content *)html,
			 html->sel,
			 ctx))
		คืน false;

	คืน true;
}

bool html_redraw_box(const html_content *html, sa_nracanaa box *box,
		จำนวนเต็ม x_parent, จำนวนเต็ม y_parent,
		const sa_nracanaa rect *clip, float scale,
		colour current_background_color,
		const sa_nracanaa redraw_context *ctx);

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

static bool html_redraw_box_children(const html_content *html, sa_nracanaa box *box,
		จำนวนเต็ม x_parent, จำนวนเต็ม y_parent,
		const sa_nracanaa rect *clip, float scale,
		colour current_background_color,
		const sa_nracanaa redraw_context *ctx)
{
	sa_nracanaa box *c;

	สำหรับ (c = box->children; c; c = c->next) {

		ถ้า (c->type != BOX_FLOAT_LEFT && c->type != BOX_FLOAT_RIGHT)
			ถ้า (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				คืน false;
	}
	สำหรับ (c = box->float_children; c; c = c->next_float)
		ถ้า (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			คืน false;

	คืน true;
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

bool html_redraw_box(const html_content *html, sa_nracanaa box *box,
		จำนวนเต็ม x_parent, จำนวนเต็ม y_parent,
		const sa_nracanaa rect *clip, const float scale,
		colour current_background_color,
		const sa_nracanaa redraw_context *ctx)
{
	const sa_nracanaa plotter_table *plot = ctx->plot;
	จำนวนเต็ม x, y;
	จำนวนเต็ม width, height;
	จำนวนเต็ม padding_left, padding_top, padding_width, padding_height;
	จำนวนเต็ม border_left, border_top, border_right, border_bottom;
	sa_nracanaa rect r;
	sa_nracanaa rect rect;
	จำนวนเต็ม x_scrolled, y_scrolled;
	sa_nracanaa box *bg_box = NULL;
	css_computed_clip_rect css_rect;
	ga_nanaa css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	ga_nanaa css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	ถ้า (html_redraw_printing && (box->flags & PRINTED))
		คืน true;

	ถ้า (box->style != NULL) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	ถ้า (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_left = box->padding[LEFT];
		padding_top = box->padding[TOP];
		padding_width = padding_left + box->width + box->padding[RIGHT];
		padding_height = padding_top + box->height +
				box->padding[BOTTOM];
		border_left = box->border[LEFT].width;
		border_top = box->border[TOP].width;
		border_right = box->border[RIGHT].width;
		border_bottom = box->border[BOTTOM].width;
	} มิฉะนั้น {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_left = box->padding[LEFT] ? box->padding[LEFT] * scale
				: 0;
		padding_top = box->padding[TOP] ? box->padding[TOP] * scale
				: 0;
		padding_width = (box->padding[LEFT] + box->width +
				box->padding[RIGHT]) * scale;
		padding_height = (box->padding[TOP] + box->height +
				box->padding[BOTTOM]) * scale;
		border_left = box->border[LEFT].width * scale;
		border_top = box->border[TOP].width * scale;
		border_right = box->border[RIGHT].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	ถ้า (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != NULL) {
		/* box contents clipped to box size */
		r.x0 = x - border_left;
		r.x1 = x + padding_width + border_right;
	} มิฉะนั้น {
		/* box contents can hang out of the box; use descendant box */
		ถ้า (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} มิฉะนั้น {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		ถ้า (!box->parent) {
			/* root element */
			จำนวนเต็ม margin_left, margin_right;
			ถ้า (scale == 1.0) {
				margin_left = box->margin[LEFT];
				margin_right = box->margin[RIGHT];
			} มิฉะนั้น {
				margin_left = box->margin[LEFT] * scale;
				margin_right = box->margin[RIGHT] * scale;
			}
			r.x0 = x - border_left - margin_left < r.x0 ?
					x - border_left - margin_left : r.x0;
			r.x1 = x + padding_width + border_right +
					margin_right > r.x1 ?
					x + padding_width + border_right +
					margin_right : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	ถ้า (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != NULL) {
		/* box contents clipped to box size */
		r.y0 = y - border_top;
		r.y1 = y + padding_height + border_bottom;
	} มิฉะนั้น {
		/* box contents can hang out of the box; use descendant box */
		ถ้า (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} มิฉะนั้น {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		ถ้า (!box->parent) {
			/* root element */
			จำนวนเต็ม margin_top, margin_bottom;
			ถ้า (scale == 1.0) {
				margin_top = box->margin[TOP];
				margin_bottom = box->margin[BOTTOM];
			} มิฉะนั้น {
				margin_top = box->margin[TOP] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_top - margin_top < r.y0 ?
					y - border_top - margin_top : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	ถ้า (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		คืน true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	ถ้า (html_redraw_printing) {
		ถ้า (r.y1 > html_redraw_printing_border) {
			ถ้า (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				ถ้า (r.y0 < html_redraw_printing_top_cropped)
					html_redraw_printing_top_cropped = r.y0;
				คืน true;
			}
		}
		มิฉะนั้น box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	ถ้า (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		ถ้า ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			คืน false;
		ถ้า (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			คืน false;
		คืน ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	ถ้า ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		คืน false;
	}

	ถ้า (box->style != NULL &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ถ้า (css_rect.left_auto == false)
			r.x0 = x - border_left + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.left, css_rect.lunit));

		ถ้า (css_rect.top_auto == false)
			r.y0 = y - border_top + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.top, css_rect.tunit));

		ถ้า (css_rect.right_auto == false)
			r.x1 = x - border_left + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.right, css_rect.runit));

		ถ้า (css_rect.bottom_auto == false)
			r.y1 = y - border_top + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		ถ้า (r.x0 < clip->x0) r.x0 = clip->x0;
		ถ้า (r.y0 < clip->y0) r.y0 = clip->y0;
		ถ้า (clip->x1 < r.x1) r.x1 = clip->x1;
		ถ้า (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		ถ้า (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			คืน ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			คืน false;

	} มิฉะนั้น ถ้า (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		ถ้า (r.x0 < clip->x0) r.x0 = clip->x0;
		ถ้า (r.y0 < clip->y0) r.y0 = clip->y0;
		ถ้า (clip->x1 < r.x1) r.x1 = clip->x1;
		ถ้า (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		ถ้า (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			คืน ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			คืน false;
	} มิฉะนั้น {
		/* clip box is fine, clip to it */
		r = *clip;
		ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			คืน false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	ถ้า (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != NULL &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		sa_nracanaa rect p;
		p.x0 = x - border_left < r.x0 ? r.x0 : x - border_left;
		p.y0 = y - border_top < r.y0 ? r.y0 : y - border_top;
		p.x1 = x + padding_width + border_right < r.x1 ?
				x + padding_width + border_right : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		ถ้า (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			จำนวนเต็ม m_left, m_top, m_right, m_bottom;
			ถ้า (scale == 1.0) {
				m_left = box->margin[LEFT];
				m_top = box->margin[TOP];
				m_right = box->margin[RIGHT];
				m_bottom = box->margin[BOTTOM];
			} มิฉะนั้น {
				m_left = box->margin[LEFT] * scale;
				m_top = box->margin[TOP] * scale;
				m_right = box->margin[RIGHT] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_left < r.x0 ? r.x0 : p.x0 - m_left;
			p.y0 = p.y0 - m_top < r.y0 ? r.y0 : p.y0 - m_top;
			p.x1 = p.x1 + m_right < r.x1 ? p.x1 + m_right : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		ถ้า ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			ถ้า (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				คืน false;
			/* restore previous graphics window */
			ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				คืน false;
		}
	}

	/* borders for block level content and replaced inlines */
	ถ้า (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != NULL &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_top || border_right || border_bottom || border_left)) {
		ถ้า (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			คืน false;
	}

	/* backgrounds and borders for non-replaced inlines */
	ถ้า (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_top || border_right ||
			border_bottom || border_left)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		sa_nracanaa box *ib;
		sa_nracanaa rect b; /* border edge rectangle */
		sa_nracanaa rect p; /* clipped rect */
		bool first = true;
		จำนวนเต็ม ib_x;
		จำนวนเต็ม ib_y = y;
		จำนวนเต็ม ib_p_width;
		จำนวนเต็ม ib_b_left, ib_b_right;

		b.x0 = x - border_left;
		b.x1 = x + padding_width + border_right;
		b.y0 = y - border_top;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		สำหรับ (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			ถ้า (ib->type == BOX_FLOAT_LEFT ||
					ib->type == BOX_FLOAT_RIGHT)
				continue;
			ถ้า (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[LEFT] + ib->width +
						ib->padding[RIGHT];
				ib_b_left = ib->border[LEFT].width;
				ib_b_right = ib->border[RIGHT].width;
			} มิฉะนั้น {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[LEFT] + ib->width +
						ib->padding[RIGHT]) * scale;
				ib_b_left = ib->border[LEFT].width * scale;
				ib_b_right = ib->border[RIGHT].width * scale;
			}

			ถ้า ((ib->flags & NEW_LINE) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				ถ้า (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					คืน false;
				/* restore previous graphics window */
				ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					คืน false;
				ถ้า (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					คืน false;
				/* reset coords */
				b.x0 = ib_x - ib_b_left;
				b.y0 = ib_y - border_top - padding_top;
				b.y1 = ib_y + padding_height - padding_top +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_right;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			ถ้า (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				bha_nga;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		ถ้า (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			คืน false;
		/* restore previous graphics window */
		ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			คืน false;
		ถ้า (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			คืน false;

	}

	/* Debug outlines */
	ถ้า (html_redraw_debug) {
		จำนวนเต็ม margin_left, margin_right;
		จำนวนเต็ม margin_top, margin_bottom;
		ถ้า (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_left = box->margin[LEFT];
			margin_top = box->margin[TOP];
			margin_right = box->margin[RIGHT];
			margin_bottom = box->margin[BOTTOM];
		} มิฉะนั้น {
			margin_left = box->margin[LEFT] * scale;
			margin_top = box->margin[TOP] * scale;
			margin_right = box->margin[RIGHT] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_left;
		rect.y0 = y + padding_top;
		rect.x1 = x + padding_left + width;
		rect.y1 = y + padding_top + height;
		ถ้า (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			คืน false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		ถ้า (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			คืน false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_left - margin_left;
		rect.y0 = y - border_top - margin_top;
		rect.x1 = x + padding_width + border_right + margin_right;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		ถ้า (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			คืน false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	ถ้า (box->parent != NULL) {
		bool need_clip = false;
		ถ้า (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			ถ้า (r.x0 < clip->x0) r.x0 = clip->x0;
			ถ้า (r.y0 < clip->y0) r.y0 = clip->y0;
			ถ้า (clip->x1 < r.x1) r.x1 = clip->x1;
			ถ้า (clip->y1 < r.y1) r.y1 = clip->y1;
			ถ้า (r.x1 <= r.x0 || r.y1 <= r.y0) {
				คืน (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} มิฉะนั้น ถ้า (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			ถ้า (r.x0 < clip->x0) r.x0 = clip->x0;
			ถ้า (clip->x1 < r.x1) r.x1 = clip->x1;
			ถ้า (r.x1 <= r.x0) {
				คืน (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} มิฉะนั้น ถ้า (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			ถ้า (r.y0 < clip->y0) r.y0 = clip->y0;
			ถ้า (clip->y1 < r.y1) r.y1 = clip->y1;
			ถ้า (r.y1 <= r.y0) {
				คืน (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		ถ้า (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			ถ้า (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				คืน false;
		}
	}

	/* text decoration */
	ถ้า ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		ถ้า (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			คืน false;
	}

	ถ้า (box->node != NULL) {
		exc = dom_html_element_get_tag_type(box->node, &tag_type);
		ถ้า (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} มิฉะนั้น {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ถ้า (box->object && width != 0 && height != 0) {
		sa_nracanaa content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_left;
		obj_data.y = y_scrolled + padding_top;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		ถ้า (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		ถ้า (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			const var_na *obj = "\xef\xbf\xbc";
			จำนวนเต็ม obj_width;
			จำนวนเต็ม obj_x = x + padding_left;
			nserror res;

			rect.x0 = x + padding_left;
			rect.y0 = y + padding_top;
			rect.x1 = x + padding_left + width - 1;
			rect.y1 = y + padding_top + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			ถ้า (res != NSERROR_OK) {
				คืน false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  parimaa_na(obj) - 1,
						  &obj_width);
			ถ้า (res != NSERROR_OK) {
				obj_x += 1;
			} มิฉะนั้น {
				obj_x += width / 2 - obj_width / 2;
			}

			ถ้า (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_top + (จำนวนเต็ม)(height * 0.75),
					    obj, parimaa_na(obj) - 1) != NSERROR_OK)
				คืน false;
		}
	} มิฉะนั้น ถ้า (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->node != NULL &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		sa_nracanaa bitmap *bitmap = NULL;
		exc = dom_node_get_user_data(box->node,
					     corestring_dom___ns_key_canvas_node_data,
					     &bitmap);
		ถ้า (exc != DOM_NO_ERR) {
			bitmap = NULL;
		}
		ถ้า (bitmap != NULL &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_left, y + padding_top,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			คืน false;
	} มิฉะนั้น ถ้า (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_left,
				y + padding_top, &r, ctx);

	} มิฉะนั้น ถ้า (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		ถ้า (!html_redraw_checkbox(x + padding_left, y + padding_top,
				width, height, box->gadget->selected, ctx))
			คืน false;

	} มิฉะนั้น ถ้า (box->gadget && box->gadget->type == GADGET_RADIO) {
		ถ้า (!html_redraw_radio(x + padding_left, y + padding_top,
				width, height, box->gadget->selected, ctx))
			คืน false;

	} มิฉะนั้น ถ้า (box->gadget && box->gadget->type == GADGET_FILE) {
		ถ้า (!html_redraw_file(x + padding_left, y + padding_top,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			คืน false;

	} มิฉะนั้น ถ้า (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} มิฉะนั้น ถ้า (box->text) {
		ถ้า (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			คืน false;

	} มิฉะนั้น {
		ถ้า (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			คืน false;
	}

	ถ้า (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		ถ้า (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			คืน false;

	/* list marker */
	ถ้า (box->list_marker) {
		ถ้า (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			คืน false;
	}

	/* scrollbars */
	ถ้า (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == NULL || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != NULL) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((sa_nracanaa content *)html,
					    box, has_x_scroll, has_y_scroll);
		ถ้า (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			คืน false;
		}

		ถ้า (box->scroll_x != NULL)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[TOP] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		ถ้า (box->scroll_y != NULL)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[LEFT] +
					box->width + box->padding[RIGHT] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	ถ้า (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		ถ้า (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			คืน false;
	}

	คืน ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(sa_nracanaa content *c, sa_nracanaa content_redraw_data *data,
		const sa_nracanaa rect *clip, const sa_nracanaa redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	sa_nracanaa box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	assert(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	ถ้า (ctx->interactive && html->visible_select_menu != NULL) {
		sa_nracanaa form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	ถ้า (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		ถ้า (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	ถ้า (select) {
		จำนวนเต็ม menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[LEFT].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[TOP];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	คืน result;

}
