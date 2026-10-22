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
static bool html_redraw_box_has_background(struct box *box)
{
	nếu (box->background != NULL)
		trả_về true;

	nếu (box->style != NULL) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		nếu (nscss_color_is_transparent(colour) == false)
			trả_về true;
	}

	trả_về false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
static struct box *html_redraw_find_bg_box(struct box *box)
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
	nếu (box->parent == NULL) {
		/* Root box */
		nếu (html_redraw_box_has_background(box))
			trả_về box;

		/* No background on root box: consider body box, if any */
		nếu (box->children != NULL) {
			nếu (html_redraw_box_has_background(box->children))
				trả_về box->children;
		}
	} không_thì nếu (box->parent != NULL && box->parent->parent == NULL) {
		/* Body box: only render background if root has its own */
		nếu (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			trả_về box;
	} không_thì {
		/* Any other box */
		nếu (html_redraw_box_has_background(box))
			trả_về box;
	}

	trả_về NULL;
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
text_redraw(const char *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    số_nguyên space,
	    const plot_font_style_t *fstyle,
	    số_nguyên x,
	    số_nguyên y,
	    const struct rect *clip,
	    số_nguyên height,
	    float scale,
	    bool excluded,
	    struct content *c,
	    const struct selection *sel,
	    const struct redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	nếu (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		nếu (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		nếu (!highlighted &&
		    (c->textsearch.context != NULL) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		nếu (highlighted) {
			struct rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			số_nguyên startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			nếu (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				assert(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			nếu (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			nếu (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			nếu (end_idx > utf8_len) {
					endx += space;
			}

			nếu (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			nếu ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (số_nguyên)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				trả_về false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}

			nếu (start_idx > 0) {
				số_nguyên px0 = max(x + startx, clip->x0);
				số_nguyên px1 = min(x + endx, clip->x1);

				nếu (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					nếu (res != NSERROR_OK) {
						trả_về false;
					}

					clip_changed = true;
				} không_thì {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			nếu (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (số_nguyên)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				trả_về false;
			}

			/* draw any text succeeding highlighted portion */
			nếu (endtxt_idx < utf8_len) {
				số_nguyên px0 = max(x + endx, clip->x0);
				nếu (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					nếu (res != NSERROR_OK) {
						trả_về false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (số_nguyên)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					nếu (res != NSERROR_OK) {
						trả_về false;
					}
				}
			}

			nếu (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				trả_về false;
			}
		}
	}

	nếu (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (số_nguyên) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		nếu (res != NSERROR_OK) {
			trả_về false;
		}
	}
	trả_về true;
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

static bool html_redraw_checkbox(số_nguyên x, số_nguyên y, số_nguyên width, số_nguyên height,
		bool selected, const struct redraw_context *ctx)
{
	double z;
	nserror res;
	struct rect rect;

	z = width * 0.15;
	nếu (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->line(ctx, plot_style_stroke_darkwbasec, &rect);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->line(ctx, plot_style_stroke_darkwbasec, &rect);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->line(ctx, plot_style_stroke_lightwbasec, &rect);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->line(ctx, plot_style_stroke_lightwbasec, &rect);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	nếu (selected) {
		nếu (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}
		} không_thì {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->line(ctx, plot_style_stroke_wblobc, &rect);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->line(ctx, plot_style_stroke_wblobc, &rect);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}
		}
	}
	trả_về true;
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
static bool html_redraw_radio(số_nguyên x, số_nguyên y, số_nguyên width, số_nguyên height,
		bool selected, const struct redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}

	nếu (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		nếu (res != NSERROR_OK) {
			trả_về false;
		}
	}

	trả_về true;
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

static bool html_redraw_file(số_nguyên x, số_nguyên y, số_nguyên width, số_nguyên height,
		struct box *box, float scale, colour background_colour,
		const css_unit_ctx *unit_len_ctx,
		const struct redraw_context *ctx)
{
	số_nguyên text_width;
	const char *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	nếu (box->gadget->value) {
		text = box->gadget->value;
	} không_thì {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}
	text_width *= scale;
	nếu (width < text_width + 8) {
		x = x + width - text_width - 4;
	} không_thì {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	nếu (res != NSERROR_OK) {
		trả_về false;
	}
	trả_về true;
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

static bool html_redraw_background(số_nguyên x, số_nguyên y, struct box *box, float scale,
		const struct rect *clip, colour *background_colour,
		struct box *background,
		const css_unit_ctx *unit_len_ctx,
		const struct redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	struct box *clip_box = box;
	số_nguyên ox = x, oy = y;
	số_nguyên width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	struct box *parent;
	struct rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	nếu (ctx->background_images == false)
		trả_về true;

	plot_content = (background->background != NULL);

	nếu (plot_content) {
		nếu (!box->parent) {
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
		} không_thì {
			width = box->padding[LEFT] + box->width +
					box->padding[RIGHT];
			height = box->padding[TOP] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		switch (css_computed_background_repeat(background->style)) {
		case CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			break;

		case CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			break;

		case CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			break;

		case CSS_BACKGROUND_REPEAT_NO_REPEAT:
			break;

		default:
			break;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		nếu (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} không_thì {
			x += (số_nguyên) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		nếu (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} không_thì {
			y += (số_nguyên) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	nếu (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		cho (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		assert(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		nếu (clip_to_children)
			clip_box = box->children;
	}

	cho (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		nếu (clip_to_children) {
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

			nếu (r.x0 < clip->x0) r.x0 = clip->x0;
			nếu (r.y0 < clip->y0) r.y0 = clip->y0;
			nếu (r.x1 > clip->x1) r.x1 = clip->x1;
			nếu (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			nếu ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != NULL) &&
			     content_get_opaque(clip_box->background)))
				continue;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		nếu (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			nếu (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				nếu (res != NSERROR_OK) {
					trả_về false;
				}
			}
		}
		/* and plot the image */
		nếu (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			nếu (!repeat_x) {
				nếu (r.x0 < x)
					r.x0 = x;
				nếu (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			nếu (!repeat_y) {
				nếu (r.y0 < y)
					r.y0 = y;
				nếu (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			nếu ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				struct content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				nếu (res != NSERROR_OK) {
					trả_về false;
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
		nếu (!clip_to_children)
			trả_về true;
	}
	trả_về true;
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

static bool html_redraw_inline_background(số_nguyên x, số_nguyên y, struct box *box,
		float scale, const struct rect *clip, struct rect b,
		bool first, bool last, colour *background_colour,
		const css_unit_ctx *unit_len_ctx,
		const struct redraw_context *ctx)
{
	struct rect r = *clip;
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

	nếu (html_redraw_printing && nsoption_bool(remove_backgrounds))
		trả_về true;

	nếu (plot_content) {
		/* handle background-repeat */
		switch (css_computed_background_repeat(box->style)) {
		case CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			break;

		case CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			break;

		case CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			break;

		case CSS_BACKGROUND_REPEAT_NO_REPEAT:
			break;

		default:
			break;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		nếu (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			nếu (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} không_thì {
			x += (số_nguyên) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		nếu (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} không_thì {
			y += (số_nguyên) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	nếu (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		nếu (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}
		}
	}
	/* and plot the image */
	nếu (plot_content) {
		số_nguyên width = content_get_width(box->background);
		số_nguyên height = content_get_height(box->background);

		nếu (!repeat_x) {
			nếu (r.x0 < x)
				r.x0 = x;
			nếu (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		nếu (!repeat_y) {
			nếu (r.y0 < y)
				r.y0 = y;
			nếu (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		nếu ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			struct content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			nếu (res != NSERROR_OK) {
				trả_về false;
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

	trả_về true;
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
html_redraw_text_decoration_inline(struct box *box,
				   số_nguyên x, số_nguyên y,
				   float scale,
				   colour colour,
				   float ratio,
				   const struct redraw_context *ctx)
{
	struct box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	struct rect rect;

	cho (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		nếu (c->type != BOX_TEXT) {
			continue;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->line(ctx, &plot_style_box, &rect);
		nếu (res != NSERROR_OK) {
			trả_về false;
		}
	}
	trả_về true;
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
html_redraw_text_decoration_block(struct box *box,
				  số_nguyên x, số_nguyên y,
				  float scale,
				  colour colour,
				  float ratio,
				  const struct redraw_context *ctx)
{
	struct box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	struct rect rect;

	/* draw through text descendants */
	cho (c = box->children; c; c = c->next) {
		nếu (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->line(ctx, &plot_style_box, &rect);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}
		} không_thì nếu ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			nếu (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				trả_về false;
		}
	}
	trả_về true;
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

static bool html_redraw_text_decoration(struct box *box,
		số_nguyên x_parent, số_nguyên y_parent, float scale,
		colour background_colour, const struct redraw_context *ctx)
{
	static const enum css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_LINE_THROUGH };
	static const float line_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned số_nguyên i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	nếu (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	nếu (box->type == BOX_INLINE) {
		nếu (!box->inline_end)
			trả_về true;
		cho (i = 0; i != NOF_ELEMENTS(decoration); i++)
			nếu (css_computed_text_decoration(box->style) &
					decoration[i])
				nếu (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, line_ratio[i], ctx))
					trả_về false;
	} không_thì {
		cho (i = 0; i != NOF_ELEMENTS(decoration); i++)
			nếu (css_computed_text_decoration(box->style) &
					decoration[i])
				nếu (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, line_ratio[i], ctx))
					trả_về false;
	}

	trả_về true;
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

static bool html_redraw_text_box(const html_content *html, struct box *box,
		số_nguyên x, số_nguyên y, const struct rect *clip, float scale,
		colour current_background_color,
		const struct redraw_context *ctx)
{
	bool excluded = (box->object != NULL);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	nếu (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (struct content *)html,
			 html->sel,
			 ctx))
		trả_về false;

	trả_về true;
}

bool html_redraw_box(const html_content *html, struct box *box,
		số_nguyên x_parent, số_nguyên y_parent,
		const struct rect *clip, float scale,
		colour current_background_color,
		const struct redraw_context *ctx);

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

static bool html_redraw_box_children(const html_content *html, struct box *box,
		số_nguyên x_parent, số_nguyên y_parent,
		const struct rect *clip, float scale,
		colour current_background_color,
		const struct redraw_context *ctx)
{
	struct box *c;

	cho (c = box->children; c; c = c->next) {

		nếu (c->type != BOX_FLOAT_LEFT && c->type != BOX_FLOAT_RIGHT)
			nếu (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				trả_về false;
	}
	cho (c = box->float_children; c; c = c->next_float)
		nếu (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			trả_về false;

	trả_về true;
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

bool html_redraw_box(const html_content *html, struct box *box,
		số_nguyên x_parent, số_nguyên y_parent,
		const struct rect *clip, const float scale,
		colour current_background_color,
		const struct redraw_context *ctx)
{
	const struct plotter_table *plot = ctx->plot;
	số_nguyên x, y;
	số_nguyên width, height;
	số_nguyên padding_left, padding_top, padding_width, padding_height;
	số_nguyên border_left, border_top, border_right, border_bottom;
	struct rect r;
	struct rect rect;
	số_nguyên x_scrolled, y_scrolled;
	struct box *bg_box = NULL;
	css_computed_clip_rect css_rect;
	enum css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	enum css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	nếu (html_redraw_printing && (box->flags & PRINTED))
		trả_về true;

	nếu (box->style != NULL) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	nếu (scale == 1.0) {
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
	} không_thì {
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
	nếu (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != NULL) {
		/* box contents clipped to box size */
		r.x0 = x - border_left;
		r.x1 = x + padding_width + border_right;
	} không_thì {
		/* box contents can hang out of the box; use descendant box */
		nếu (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} không_thì {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		nếu (!box->parent) {
			/* root element */
			số_nguyên margin_left, margin_right;
			nếu (scale == 1.0) {
				margin_left = box->margin[LEFT];
				margin_right = box->margin[RIGHT];
			} không_thì {
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
	nếu (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != NULL) {
		/* box contents clipped to box size */
		r.y0 = y - border_top;
		r.y1 = y + padding_height + border_bottom;
	} không_thì {
		/* box contents can hang out of the box; use descendant box */
		nếu (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} không_thì {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		nếu (!box->parent) {
			/* root element */
			số_nguyên margin_top, margin_bottom;
			nếu (scale == 1.0) {
				margin_top = box->margin[TOP];
				margin_bottom = box->margin[BOTTOM];
			} không_thì {
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
	nếu (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		trả_về true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	nếu (html_redraw_printing) {
		nếu (r.y1 > html_redraw_printing_border) {
			nếu (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				nếu (r.y0 < html_redraw_printing_top_cropped)
					html_redraw_printing_top_cropped = r.y0;
				trả_về true;
			}
		}
		không_thì box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	nếu (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		nếu ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			trả_về false;
		nếu (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			trả_về false;
		trả_về ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	nếu ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		trả_về false;
	}

	nếu (box->style != NULL &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		nếu (css_rect.left_auto == false)
			r.x0 = x - border_left + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.left, css_rect.lunit));

		nếu (css_rect.top_auto == false)
			r.y0 = y - border_top + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.top, css_rect.tunit));

		nếu (css_rect.right_auto == false)
			r.x1 = x - border_left + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.right, css_rect.runit));

		nếu (css_rect.bottom_auto == false)
			r.y1 = y - border_top + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		nếu (r.x0 < clip->x0) r.x0 = clip->x0;
		nếu (r.y0 < clip->y0) r.y0 = clip->y0;
		nếu (clip->x1 < r.x1) r.x1 = clip->x1;
		nếu (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		nếu (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			trả_về ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			trả_về false;

	} không_thì nếu (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		nếu (r.x0 < clip->x0) r.x0 = clip->x0;
		nếu (r.y0 < clip->y0) r.y0 = clip->y0;
		nếu (clip->x1 < r.x1) r.x1 = clip->x1;
		nếu (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		nếu (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			trả_về ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			trả_về false;
	} không_thì {
		/* clip box is fine, clip to it */
		r = *clip;
		nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			trả_về false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	nếu (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != NULL &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		struct rect p;
		p.x0 = x - border_left < r.x0 ? r.x0 : x - border_left;
		p.y0 = y - border_top < r.y0 ? r.y0 : y - border_top;
		p.x1 = x + padding_width + border_right < r.x1 ?
				x + padding_width + border_right : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		nếu (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			số_nguyên m_left, m_top, m_right, m_bottom;
			nếu (scale == 1.0) {
				m_left = box->margin[LEFT];
				m_top = box->margin[TOP];
				m_right = box->margin[RIGHT];
				m_bottom = box->margin[BOTTOM];
			} không_thì {
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
		nếu ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			nếu (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				trả_về false;
			/* restore previous graphics window */
			nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				trả_về false;
		}
	}

	/* borders for block level content and replaced inlines */
	nếu (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != NULL &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_top || border_right || border_bottom || border_left)) {
		nếu (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			trả_về false;
	}

	/* backgrounds and borders for non-replaced inlines */
	nếu (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_top || border_right ||
			border_bottom || border_left)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		struct box *ib;
		struct rect b; /* border edge rectangle */
		struct rect p; /* clipped rect */
		bool first = true;
		số_nguyên ib_x;
		số_nguyên ib_y = y;
		số_nguyên ib_p_width;
		số_nguyên ib_b_left, ib_b_right;

		b.x0 = x - border_left;
		b.x1 = x + padding_width + border_right;
		b.y0 = y - border_top;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		cho (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			nếu (ib->type == BOX_FLOAT_LEFT ||
					ib->type == BOX_FLOAT_RIGHT)
				continue;
			nếu (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[LEFT] + ib->width +
						ib->padding[RIGHT];
				ib_b_left = ib->border[LEFT].width;
				ib_b_right = ib->border[RIGHT].width;
			} không_thì {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[LEFT] + ib->width +
						ib->padding[RIGHT]) * scale;
				ib_b_left = ib->border[LEFT].width * scale;
				ib_b_right = ib->border[RIGHT].width * scale;
			}

			nếu ((ib->flags & NEW_LINE) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				nếu (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					trả_về false;
				/* restore previous graphics window */
				nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					trả_về false;
				nếu (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					trả_về false;
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

			nếu (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				break;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		nếu (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			trả_về false;
		/* restore previous graphics window */
		nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			trả_về false;
		nếu (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			trả_về false;

	}

	/* Debug outlines */
	nếu (html_redraw_debug) {
		số_nguyên margin_left, margin_right;
		số_nguyên margin_top, margin_bottom;
		nếu (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_left = box->margin[LEFT];
			margin_top = box->margin[TOP];
			margin_right = box->margin[RIGHT];
			margin_bottom = box->margin[BOTTOM];
		} không_thì {
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
		nếu (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			trả_về false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		nếu (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			trả_về false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_left - margin_left;
		rect.y0 = y - border_top - margin_top;
		rect.x1 = x + padding_width + border_right + margin_right;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		nếu (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			trả_về false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	nếu (box->parent != NULL) {
		bool need_clip = false;
		nếu (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			nếu (r.x0 < clip->x0) r.x0 = clip->x0;
			nếu (r.y0 < clip->y0) r.y0 = clip->y0;
			nếu (clip->x1 < r.x1) r.x1 = clip->x1;
			nếu (clip->y1 < r.y1) r.y1 = clip->y1;
			nếu (r.x1 <= r.x0 || r.y1 <= r.y0) {
				trả_về (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} không_thì nếu (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			nếu (r.x0 < clip->x0) r.x0 = clip->x0;
			nếu (clip->x1 < r.x1) r.x1 = clip->x1;
			nếu (r.x1 <= r.x0) {
				trả_về (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} không_thì nếu (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			nếu (r.y0 < clip->y0) r.y0 = clip->y0;
			nếu (clip->y1 < r.y1) r.y1 = clip->y1;
			nếu (r.y1 <= r.y0) {
				trả_về (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		nếu (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			nếu (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				trả_về false;
		}
	}

	/* text decoration */
	nếu ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		nếu (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			trả_về false;
	}

	nếu (box->node != NULL) {
		exc = dom_html_element_get_tag_type(box->node, &tag_type);
		nếu (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} không_thì {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	nếu (box->object && width != 0 && height != 0) {
		struct content_redraw_data obj_data;

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

		nếu (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		nếu (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			const char *obj = "\xef\xbf\xbc";
			số_nguyên obj_width;
			số_nguyên obj_x = x + padding_left;
			nserror res;

			rect.x0 = x + padding_left;
			rect.y0 = y + padding_top;
			rect.x1 = x + padding_left + width - 1;
			rect.y1 = y + padding_top + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			nếu (res != NSERROR_OK) {
				trả_về false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  sizeof(obj) - 1,
						  &obj_width);
			nếu (res != NSERROR_OK) {
				obj_x += 1;
			} không_thì {
				obj_x += width / 2 - obj_width / 2;
			}

			nếu (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_top + (số_nguyên)(height * 0.75),
					    obj, sizeof(obj) - 1) != NSERROR_OK)
				trả_về false;
		}
	} không_thì nếu (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->node != NULL &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		struct bitmap *bitmap = NULL;
		exc = dom_node_get_user_data(box->node,
					     corestring_dom___ns_key_canvas_node_data,
					     &bitmap);
		nếu (exc != DOM_NO_ERR) {
			bitmap = NULL;
		}
		nếu (bitmap != NULL &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_left, y + padding_top,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			trả_về false;
	} không_thì nếu (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_left,
				y + padding_top, &r, ctx);

	} không_thì nếu (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		nếu (!html_redraw_checkbox(x + padding_left, y + padding_top,
				width, height, box->gadget->selected, ctx))
			trả_về false;

	} không_thì nếu (box->gadget && box->gadget->type == GADGET_RADIO) {
		nếu (!html_redraw_radio(x + padding_left, y + padding_top,
				width, height, box->gadget->selected, ctx))
			trả_về false;

	} không_thì nếu (box->gadget && box->gadget->type == GADGET_FILE) {
		nếu (!html_redraw_file(x + padding_left, y + padding_top,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			trả_về false;

	} không_thì nếu (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} không_thì nếu (box->text) {
		nếu (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			trả_về false;

	} không_thì {
		nếu (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			trả_về false;
	}

	nếu (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		nếu (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			trả_về false;

	/* list marker */
	nếu (box->list_marker) {
		nếu (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			trả_về false;
	}

	/* scrollbars */
	nếu (((box->style && box->type != BOX_BR &&
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

		res = box_handle_scrollbars((struct content *)html,
					    box, has_x_scroll, has_y_scroll);
		nếu (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			trả_về false;
		}

		nếu (box->scroll_x != NULL)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[TOP] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		nếu (box->scroll_y != NULL)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[LEFT] +
					box->width + box->padding[RIGHT] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	nếu (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		nếu (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			trả_về false;
	}

	trả_về ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(struct content *c, struct content_redraw_data *data,
		const struct rect *clip, const struct redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	struct box *box;
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
	nếu (ctx->interactive && html->visible_select_menu != NULL) {
		struct form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	nếu (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		nếu (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	nếu (select) {
		số_nguyên menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[LEFT].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[TOP];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	trả_về result;

}
