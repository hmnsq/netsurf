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

#شامل "utils/config.h"
#شامل <assert.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <math.h>
#شامل <dom/dom.h>

#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/utils.h"
#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "netsurf/content.h"
#شامل "netsurf/browser_window.h"
#شامل "netsurf/plotters.h"
#شامل "netsurf/bitmap.h"
#شامل "netsurf/layout.h"
#شامل "content/content.h"
#شامل "content/content_protected.h"
#شامل "content/textsearch.h"
#شامل "css/utils.h"
#شامل "desktop/selection.h"
#شامل "desktop/print.h"
#شامل "desktop/scrollbar.h"
#شامل "desktop/textarea.h"
#شامل "desktop/gui_internal.h"

#شامل "html/box.h"
#شامل "html/box_inspect.h"
#شامل "html/box_manipulate.h"
#شامل "html/font.h"
#شامل "html/form_internal.h"
#شامل "html/private.h"
#شامل "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ایستا bool html_redraw_box_has_background(ساختار box *box)
{
	اگر (box->background != پوچ)
		بازگشت true;

	اگر (box->style != پوچ) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		اگر (nscss_color_is_transparent(colour) == false)
			بازگشت true;
	}

	بازگشت false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ایستا ساختار box *html_redraw_find_bg_box(ساختار box *box)
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
	اگر (box->parent == پوچ) {
		/* Root box */
		اگر (html_redraw_box_has_background(box))
			بازگشت box;

		/* No background on root box: consider body box, if any */
		اگر (box->children != پوچ) {
			اگر (html_redraw_box_has_background(box->children))
				بازگشت box->children;
		}
	} وگرنه اگر (box->parent != پوچ && box->parent->parent == پوچ) {
		/* Body box: only render background if root has its own */
		اگر (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			بازگشت box;
	} وگرنه {
		/* Any other box */
		اگر (html_redraw_box_has_background(box))
			بازگشت box;
	}

	بازگشت پوچ;
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

ایستا bool
text_redraw(ثابت نویسه *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    صحیح space,
	    ثابت plot_font_style_t *fstyle,
	    صحیح x,
	    صحیح y,
	    ثابت ساختار rect *clip,
	    صحیح height,
	    اعشاری scale,
	    bool excluded,
	    ساختار content *c,
	    ثابت ساختار selection *sel,
	    ثابت ساختار redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	اگر (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		اگر (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		اگر (!highlighted &&
		    (c->textsearch.context != پوچ) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		اگر (highlighted) {
			ساختار rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			صحیح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			اگر (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				أكد(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			اگر (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			اگر (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			اگر (end_idx > utf8_len) {
					endx += space;
			}

			اگر (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			اگر ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (صحیح)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				بازگشت false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}

			اگر (start_idx > 0) {
				صحیح px0 = max(x + startx, clip->x0);
				صحیح px1 = min(x + endx, clip->x1);

				اگر (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}

					clip_changed = true;
				} وگرنه {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			اگر (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحیح)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				بازگشت false;
			}

			/* draw any text succeeding highlighted portion */
			اگر (endtxt_idx < utf8_len) {
				صحیح px0 = max(x + endx, clip->x0);
				اگر (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحیح)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					اگر (res != NSERROR_OK) {
						بازگشت false;
					}
				}
			}

			اگر (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				بازگشت false;
			}
		}
	}

	اگر (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحیح) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}
	بازگشت true;
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

ایستا bool html_redraw_checkbox(صحیح x, صحیح y, صحیح width, صحیح height,
		bool selected, ثابت ساختار redraw_context *ctx)
{
	دوگانه z;
	nserror res;
	ساختار rect rect;

	z = width * 0.15;
	اگر (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->سطر(ctx, plot_style_stroke_darkwbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->سطر(ctx, plot_style_stroke_lightwbasec, &rect);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	اگر (selected) {
		اگر (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		} وگرنه {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->سطر(ctx, plot_style_stroke_wblobc, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		}
	}
	بازگشت true;
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
ایستا bool html_redraw_radio(صحیح x, صحیح y, صحیح width, صحیح height,
		bool selected, ثابت ساختار redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}

	اگر (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}

	بازگشت true;
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

ایستا bool html_redraw_file(صحیح x, صحیح y, صحیح width, صحیح height,
		ساختار box *box, اعشاری scale, colour background_colour,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساختار redraw_context *ctx)
{
	صحیح text_width;
	ثابت نویسه *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	اگر (box->gadget->مقدار) {
		text = box->gadget->مقدار;
	} وگرنه {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}
	text_width *= scale;
	اگر (width < text_width + 8) {
		x = x + width - text_width - 4;
	} وگرنه {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	اگر (res != NSERROR_OK) {
		بازگشت false;
	}
	بازگشت true;
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

ایستا bool html_redraw_background(صحیح x, صحیح y, ساختار box *box, اعشاری scale,
		ثابت ساختار rect *clip, colour *background_colour,
		ساختار box *background,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساختار redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	ساختار box *clip_box = box;
	صحیح ox = x, oy = y;
	صحیح width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	ساختار box *parent;
	ساختار rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	اگر (ctx->background_images == false)
		بازگشت true;

	plot_content = (background->background != پوچ);

	اگر (plot_content) {
		اگر (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[چپ_2] * scale;
			y -= box->margin[بالاترین_2] * scale;
			width = box->margin[چپ_2] + box->padding[چپ_2] +
					box->width + box->padding[راست_2] +
					box->margin[راست_2];
			height = box->margin[بالاترین_2] + box->padding[بالاترین_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} وگرنه {
			width = box->padding[چپ_2] + box->width +
					box->padding[راست_2];
			height = box->padding[بالاترین_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		انتخاب (css_computed_background_repeat(background->style)) {
		حالت CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
			بشکن;

		پیشفرض:
			بشکن;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		اگر (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} وگرنه {
			x += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		اگر (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} وگرنه {
			y += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	اگر (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		برای (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		أكد(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		اگر (clip_to_children)
			clip_box = box->children;
	}

	برای (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		اگر (clip_to_children) {
			أكد(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[چپ_2] +
					clip_box->width +
					clip_box->padding[راست_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[بالاترین_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (r.x1 > clip->x1) r.x1 = clip->x1;
			اگر (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			اگر ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != پوچ) &&
			     content_get_opaque(clip_box->background)))
				ادامه;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		اگر (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			اگر (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				اگر (res != NSERROR_OK) {
					بازگشت false;
				}
			}
		}
		/* and plot the image */
		اگر (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			اگر (!repeat_x) {
				اگر (r.x0 < x)
					r.x0 = x;
				اگر (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			اگر (!repeat_y) {
				اگر (r.y0 < y)
					r.y0 = y;
				اگر (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			اگر ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				ساختار content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				اگر (res != NSERROR_OK) {
					بازگشت false;
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
		اگر (!clip_to_children)
			بازگشت true;
	}
	بازگشت true;
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

ایستا bool html_redraw_inline_background(صحیح x, صحیح y, ساختار box *box,
		اعشاری scale, ثابت ساختار rect *clip, ساختار rect b,
		bool first, bool last, colour *background_colour,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت ساختار redraw_context *ctx)
{
	ساختار rect r = *clip;
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

	plot_content = (box->background != پوچ);

	اگر (html_redraw_printing && nsoption_bool(remove_backgrounds))
		بازگشت true;

	اگر (plot_content) {
		/* handle background-repeat */
		انتخاب (css_computed_background_repeat(box->style)) {
		حالت CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			بشکن;

		حالت CSS_BACKGROUND_REPEAT_NO_REPEAT:
			بشکن;

		پیشفرض:
			بشکن;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		اگر (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			اگر (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} وگرنه {
			x += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		اگر (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} وگرنه {
			y += (صحیح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	اگر (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		اگر (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		}
	}
	/* and plot the image */
	اگر (plot_content) {
		صحیح width = content_get_width(box->background);
		صحیح height = content_get_height(box->background);

		اگر (!repeat_x) {
			اگر (r.x0 < x)
				r.x0 = x;
			اگر (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		اگر (!repeat_y) {
			اگر (r.y0 < y)
				r.y0 = y;
			اگر (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		اگر ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			ساختار content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			اگر (res != NSERROR_OK) {
				بازگشت false;
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

	بازگشت true;
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

ایستا bool
html_redraw_text_decoration_inline(ساختار box *box,
				   صحیح x, صحیح y,
				   اعشاری scale,
				   colour colour,
				   اعشاری ratio,
				   ثابت ساختار redraw_context *ctx)
{
	ساختار box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ساختار rect rect;

	برای (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		اگر (c->type != BOX_TEXT) {
			ادامه;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
		اگر (res != NSERROR_OK) {
			بازگشت false;
		}
	}
	بازگشت true;
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

ایستا bool
html_redraw_text_decoration_block(ساختار box *box,
				  صحیح x, صحیح y,
				  اعشاری scale,
				  colour colour,
				  اعشاری ratio,
				  ثابت ساختار redraw_context *ctx)
{
	ساختار box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	ساختار rect rect;

	/* draw through text descendants */
	برای (c = box->children; c; c = c->next) {
		اگر (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->سطر(ctx, &plot_style_box, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}
		} وگرنه اگر ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			اگر (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				بازگشت false;
		}
	}
	بازگشت true;
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

ایستا bool html_redraw_text_decoration(ساختار box *box,
		صحیح x_parent, صحیح y_parent, اعشاری scale,
		colour background_colour, ثابت ساختار redraw_context *ctx)
{
	ایستا ثابت شمارش css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_سطر_THROUGH };
	ایستا ثابت اعشاری سطر_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned صحیح i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	اگر (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	اگر (box->type == BOX_INLINE) {
		اگر (!box->inline_end)
			بازگشت true;
		برای (i = 0; i != NOF_ELEMENTS(decoration); i++)
			اگر (css_computed_text_decoration(box->style) &
					decoration[i])
				اگر (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, سطر_ratio[i], ctx))
					بازگشت false;
	} وگرنه {
		برای (i = 0; i != NOF_ELEMENTS(decoration); i++)
			اگر (css_computed_text_decoration(box->style) &
					decoration[i])
				اگر (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, سطر_ratio[i], ctx))
					بازگشت false;
	}

	بازگشت true;
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

ایستا bool html_redraw_text_box(ثابت html_content *html, ساختار box *box,
		صحیح x, صحیح y, ثابت ساختار rect *clip, اعشاری scale,
		colour current_background_color,
		ثابت ساختار redraw_context *ctx)
{
	bool excluded = (box->object != پوچ);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	اگر (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (ساختار content *)html,
			 html->sel,
			 ctx))
		بازگشت false;

	بازگشت true;
}

bool html_redraw_box(ثابت html_content *html, ساختار box *box,
		صحیح x_parent, صحیح y_parent,
		ثابت ساختار rect *clip, اعشاری scale,
		colour current_background_color,
		ثابت ساختار redraw_context *ctx);

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

ایستا bool html_redraw_box_children(ثابت html_content *html, ساختار box *box,
		صحیح x_parent, صحیح y_parent,
		ثابت ساختار rect *clip, اعشاری scale,
		colour current_background_color,
		ثابت ساختار redraw_context *ctx)
{
	ساختار box *c;

	برای (c = box->children; c; c = c->next) {

		اگر (c->type != BOX_FLOAT_چپ && c->type != BOX_FLOAT_راست)
			اگر (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				بازگشت false;
	}
	برای (c = box->float_children; c; c = c->next_float)
		اگر (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			بازگشت false;

	بازگشت true;
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

bool html_redraw_box(ثابت html_content *html, ساختار box *box,
		صحیح x_parent, صحیح y_parent,
		ثابت ساختار rect *clip, ثابت اعشاری scale,
		colour current_background_color,
		ثابت ساختار redraw_context *ctx)
{
	ثابت ساختار plotter_table *plot = ctx->plot;
	صحیح x, y;
	صحیح width, height;
	صحیح padding_چپ, padding_بالاترین, padding_width, padding_height;
	صحیح border_چپ, border_بالاترین, border_راست, border_bottom;
	ساختار rect r;
	ساختار rect rect;
	صحیح x_scrolled, y_scrolled;
	ساختار box *bg_box = پوچ;
	css_computed_clip_rect css_rect;
	شمارش css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	شمارش css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	اگر (html_redraw_printing && (box->flags & PRINTED))
		بازگشت true;

	اگر (box->style != پوچ) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	اگر (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_چپ = box->padding[چپ_2];
		padding_بالاترین = box->padding[بالاترین_2];
		padding_width = padding_چپ + box->width + box->padding[راست_2];
		padding_height = padding_بالاترین + box->height +
				box->padding[BOTTOM];
		border_چپ = box->border[چپ_2].width;
		border_بالاترین = box->border[بالاترین_2].width;
		border_راست = box->border[راست_2].width;
		border_bottom = box->border[BOTTOM].width;
	} وگرنه {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_چپ = box->padding[چپ_2] ? box->padding[چپ_2] * scale
				: 0;
		padding_بالاترین = box->padding[بالاترین_2] ? box->padding[بالاترین_2] * scale
				: 0;
		padding_width = (box->padding[چپ_2] + box->width +
				box->padding[راست_2]) * scale;
		padding_height = (box->padding[بالاترین_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_چپ = box->border[چپ_2].width * scale;
		border_بالاترین = box->border[بالاترین_2].width * scale;
		border_راست = box->border[راست_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	اگر (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != پوچ) {
		/* box contents clipped to box size */
		r.x0 = x - border_چپ;
		r.x1 = x + padding_width + border_راست;
	} وگرنه {
		/* box contents can hang out of the box; use descendant box */
		اگر (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} وگرنه {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		اگر (!box->parent) {
			/* root element */
			صحیح margin_چپ, margin_راست;
			اگر (scale == 1.0) {
				margin_چپ = box->margin[چپ_2];
				margin_راست = box->margin[راست_2];
			} وگرنه {
				margin_چپ = box->margin[چپ_2] * scale;
				margin_راست = box->margin[راست_2] * scale;
			}
			r.x0 = x - border_چپ - margin_چپ < r.x0 ?
					x - border_چپ - margin_چپ : r.x0;
			r.x1 = x + padding_width + border_راست +
					margin_راست > r.x1 ?
					x + padding_width + border_راست +
					margin_راست : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	اگر (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != پوچ) {
		/* box contents clipped to box size */
		r.y0 = y - border_بالاترین;
		r.y1 = y + padding_height + border_bottom;
	} وگرنه {
		/* box contents can hang out of the box; use descendant box */
		اگر (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} وگرنه {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		اگر (!box->parent) {
			/* root element */
			صحیح margin_بالاترین, margin_bottom;
			اگر (scale == 1.0) {
				margin_بالاترین = box->margin[بالاترین_2];
				margin_bottom = box->margin[BOTTOM];
			} وگرنه {
				margin_بالاترین = box->margin[بالاترین_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_بالاترین - margin_بالاترین < r.y0 ?
					y - border_بالاترین - margin_بالاترین : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	اگر (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		بازگشت true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	اگر (html_redraw_printing) {
		اگر (r.y1 > html_redraw_printing_border) {
			اگر (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				اگر (r.y0 < html_redraw_printing_بالاترین_cropped)
					html_redraw_printing_بالاترین_cropped = r.y0;
				بازگشت true;
			}
		}
		وگرنه box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	اگر (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		اگر ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			بازگشت false;
		اگر (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			بازگشت false;
		بازگشت ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	اگر ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		بازگشت false;
	}

	اگر (box->style != پوچ &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		اگر (css_rect.چپ_auto == false)
			r.x0 = x - border_چپ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.چپ, css_rect.lunit));

		اگر (css_rect.بالاترین_auto == false)
			r.y0 = y - border_بالاترین + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.بالاترین, css_rect.tunit));

		اگر (css_rect.راست_auto == false)
			r.x1 = x - border_چپ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.راست, css_rect.runit));

		اگر (css_rect.bottom_auto == false)
			r.y1 = y - border_بالاترین + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		اگر (r.x0 < clip->x0) r.x0 = clip->x0;
		اگر (r.y0 < clip->y0) r.y0 = clip->y0;
		اگر (clip->x1 < r.x1) r.x1 = clip->x1;
		اگر (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		اگر (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			بازگشت ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			بازگشت false;

	} وگرنه اگر (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		اگر (r.x0 < clip->x0) r.x0 = clip->x0;
		اگر (r.y0 < clip->y0) r.y0 = clip->y0;
		اگر (clip->x1 < r.x1) r.x1 = clip->x1;
		اگر (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		اگر (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			بازگشت ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			بازگشت false;
	} وگرنه {
		/* clip box is fine, clip to it */
		r = *clip;
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			بازگشت false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	اگر (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != پوچ &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		ساختار rect p;
		p.x0 = x - border_چپ < r.x0 ? r.x0 : x - border_چپ;
		p.y0 = y - border_بالاترین < r.y0 ? r.y0 : y - border_بالاترین;
		p.x1 = x + padding_width + border_راست < r.x1 ?
				x + padding_width + border_راست : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		اگر (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			صحیح m_چپ, m_بالاترین, m_راست, m_bottom;
			اگر (scale == 1.0) {
				m_چپ = box->margin[چپ_2];
				m_بالاترین = box->margin[بالاترین_2];
				m_راست = box->margin[راست_2];
				m_bottom = box->margin[BOTTOM];
			} وگرنه {
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
		اگر ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			اگر (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				بازگشت false;
			/* restore previous graphics window */
			اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				بازگشت false;
		}
	}

	/* borders for block level content and replaced inlines */
	اگر (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != پوچ &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_بالاترین || border_راست || border_bottom || border_چپ)) {
		اگر (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			بازگشت false;
	}

	/* backgrounds and borders for non-replaced inlines */
	اگر (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_بالاترین || border_راست ||
			border_bottom || border_چپ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		ساختار box *ib;
		ساختار rect b; /* border edge rectangle */
		ساختار rect p; /* clipped rect */
		bool first = true;
		صحیح ib_x;
		صحیح ib_y = y;
		صحیح ib_p_width;
		صحیح ib_b_چپ, ib_b_راست;

		b.x0 = x - border_چپ;
		b.x1 = x + padding_width + border_راست;
		b.y0 = y - border_بالاترین;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		برای (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			اگر (ib->type == BOX_FLOAT_چپ ||
					ib->type == BOX_FLOAT_راست)
				ادامه;
			اگر (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[چپ_2] + ib->width +
						ib->padding[راست_2];
				ib_b_چپ = ib->border[چپ_2].width;
				ib_b_راست = ib->border[راست_2].width;
			} وگرنه {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[چپ_2] + ib->width +
						ib->padding[راست_2]) * scale;
				ib_b_چپ = ib->border[چپ_2].width * scale;
				ib_b_راست = ib->border[راست_2].width * scale;
			}

			اگر ((ib->flags & NEW_سطر) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				اگر (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					بازگشت false;
				/* restore previous graphics window */
				اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					بازگشت false;
				اگر (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					بازگشت false;
				/* reset coords */
				b.x0 = ib_x - ib_b_چپ;
				b.y0 = ib_y - border_بالاترین - padding_بالاترین;
				b.y1 = ib_y + padding_height - padding_بالاترین +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_راست;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			اگر (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				بشکن;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		اگر (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			بازگشت false;
		/* restore previous graphics window */
		اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			بازگشت false;
		اگر (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			بازگشت false;

	}

	/* Debug outlines */
	اگر (html_redraw_debug) {
		صحیح margin_چپ, margin_راست;
		صحیح margin_بالاترین, margin_bottom;
		اگر (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_چپ = box->margin[چپ_2];
			margin_بالاترین = box->margin[بالاترین_2];
			margin_راست = box->margin[راست_2];
			margin_bottom = box->margin[BOTTOM];
		} وگرنه {
			margin_چپ = box->margin[چپ_2] * scale;
			margin_بالاترین = box->margin[بالاترین_2] * scale;
			margin_راست = box->margin[راست_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_چپ;
		rect.y0 = y + padding_بالاترین;
		rect.x1 = x + padding_چپ + width;
		rect.y1 = y + padding_بالاترین + height;
		اگر (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			بازگشت false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		اگر (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			بازگشت false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_چپ - margin_چپ;
		rect.y0 = y - border_بالاترین - margin_بالاترین;
		rect.x1 = x + padding_width + border_راست + margin_راست;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		اگر (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			بازگشت false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	اگر (box->parent != پوچ) {
		bool need_clip = false;
		اگر (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (clip->x1 < r.x1) r.x1 = clip->x1;
			اگر (clip->y1 < r.y1) r.y1 = clip->y1;
			اگر (r.x1 <= r.x0 || r.y1 <= r.y0) {
				بازگشت (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وگرنه اگر (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			اگر (r.x0 < clip->x0) r.x0 = clip->x0;
			اگر (clip->x1 < r.x1) r.x1 = clip->x1;
			اگر (r.x1 <= r.x0) {
				بازگشت (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وگرنه اگر (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			اگر (r.y0 < clip->y0) r.y0 = clip->y0;
			اگر (clip->y1 < r.y1) r.y1 = clip->y1;
			اگر (r.y1 <= r.y0) {
				بازگشت (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		اگر (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			اگر (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				بازگشت false;
		}
	}

	/* text decoration */
	اگر ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		اگر (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			بازگشت false;
	}

	اگر (box->گره != پوچ) {
		exc = dom_html_element_get_tag_type(box->گره, &tag_type);
		اگر (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} وگرنه {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	اگر (box->object && width != 0 && height != 0) {
		ساختار content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_چپ;
		obj_data.y = y_scrolled + padding_بالاترین;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		اگر (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		اگر (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ثابت نویسه *obj = "\xef\xbf\xbc";
			صحیح obj_width;
			صحیح obj_x = x + padding_چپ;
			nserror res;

			rect.x0 = x + padding_چپ;
			rect.y0 = y + padding_بالاترین;
			rect.x1 = x + padding_چپ + width - 1;
			rect.y1 = y + padding_بالاترین + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			اگر (res != NSERROR_OK) {
				بازگشت false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  اندازه(obj) - 1,
						  &obj_width);
			اگر (res != NSERROR_OK) {
				obj_x += 1;
			} وگرنه {
				obj_x += width / 2 - obj_width / 2;
			}

			اگر (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_بالاترین + (صحیح)(height * 0.75),
					    obj, اندازه(obj) - 1) != NSERROR_OK)
				بازگشت false;
		}
	} وگرنه اگر (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->گره != پوچ &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		ساختار bitmap *bitmap = پوچ;
		exc = dom_گره_get_user_data(box->گره,
					     corestring_dom_ns_کلید_canvas_گره_data,
					     &bitmap);
		اگر (exc != DOM_NO_ERR) {
			bitmap = پوچ;
		}
		اگر (bitmap != پوچ &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_چپ, y + padding_بالاترین,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			بازگشت false;
	} وگرنه اگر (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_چپ,
				y + padding_بالاترین, &r, ctx);

	} وگرنه اگر (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		اگر (!html_redraw_checkbox(x + padding_چپ, y + padding_بالاترین,
				width, height, box->gadget->selected, ctx))
			بازگشت false;

	} وگرنه اگر (box->gadget && box->gadget->type == GADGET_RADIO) {
		اگر (!html_redraw_radio(x + padding_چپ, y + padding_بالاترین,
				width, height, box->gadget->selected, ctx))
			بازگشت false;

	} وگرنه اگر (box->gadget && box->gadget->type == GADGET_FILE) {
		اگر (!html_redraw_file(x + padding_چپ, y + padding_بالاترین,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			بازگشت false;

	} وگرنه اگر (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} وگرنه اگر (box->text) {
		اگر (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			بازگشت false;

	} وگرنه {
		اگر (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			بازگشت false;
	}

	اگر (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		اگر (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			بازگشت false;

	/* list marker */
	اگر (box->list_marker) {
		اگر (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			بازگشت false;
	}

	/* scrollbars */
	اگر (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == پوچ || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != پوچ) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((ساختار content *)html,
					    box, has_x_scroll, has_y_scroll);
		اگر (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			بازگشت false;
		}

		اگر (box->scroll_x != پوچ)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[بالاترین_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		اگر (box->scroll_y != پوچ)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[چپ_2] +
					box->width + box->padding[راست_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	اگر (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		اگر (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			بازگشت false;
	}

	بازگشت ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(ساختار content *c, ساختار content_redraw_data *data,
		ثابت ساختار rect *clip, ثابت ساختار redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	ساختار box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	أكد(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	اگر (ctx->interactive && html->visible_select_menu != پوچ) {
		ساختار form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	اگر (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		اگر (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	اگر (select) {
		صحیح menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[چپ_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[بالاترین_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	بازگشت result;

}
