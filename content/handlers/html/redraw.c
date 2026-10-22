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

#ضمن "utils/config.h"
#ضمن <assert.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <math.h>
#ضمن <dom/dom.h>

#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/utils.h"
#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/browser_window.h"
#ضمن "netsurf/plotters.h"
#ضمن "netsurf/bitmap.h"
#ضمن "netsurf/layout.h"
#ضمن "content/content.h"
#ضمن "content/content_protected.h"
#ضمن "content/textsearch.h"
#ضمن "css/utils.h"
#ضمن "desktop/selection.h"
#ضمن "desktop/print.h"
#ضمن "desktop/scrollbar.h"
#ضمن "desktop/textarea.h"
#ضمن "desktop/gui_internal.h"

#ضمن "html/box.h"
#ضمن "html/box_inspect.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/font.h"
#ضمن "html/form_internal.h"
#ضمن "html/private.h"
#ضمن "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
ساكن bool html_redraw_box_has_background(بنية box *box)
{
	إذا (box->background != عدم)
		أرجع true;

	إذا (box->style != عدم) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		إذا (nscss_color_is_transparent(colour) == false)
			أرجع true;
	}

	أرجع false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
ساكن بنية box *html_redraw_find_bg_box(بنية box *box)
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
	إذا (box->parent == عدم) {
		/* Root box */
		إذا (html_redraw_box_has_background(box))
			أرجع box;

		/* No background on root box: consider body box, if any */
		إذا (box->children != عدم) {
			إذا (html_redraw_box_has_background(box->children))
				أرجع box->children;
		}
	} وإلا إذا (box->parent != عدم && box->parent->parent == عدم) {
		/* Body box: only render background if root has its own */
		إذا (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			أرجع box;
	} وإلا {
		/* Any other box */
		إذا (html_redraw_box_has_background(box))
			أرجع box;
	}

	أرجع عدم;
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

ساكن bool
text_redraw(ثابت حرف *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    صحيح space,
	    ثابت plot_font_style_t *fstyle,
	    صحيح x,
	    صحيح y,
	    ثابت بنية rect *clip,
	    صحيح height,
	    عائم scale,
	    bool excluded,
	    بنية content *c,
	    ثابت بنية selection *sel,
	    ثابت بنية redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	إذا (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		إذا (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		إذا (!highlighted &&
		    (c->textsearch.context != عدم) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		إذا (highlighted) {
			بنية rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			صحيح startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			إذا (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				أكد(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			إذا (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			إذا (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			إذا (end_idx > utf8_len) {
					endx += space;
			}

			إذا (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			إذا ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (صحيح)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				أرجع false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			إذا (start_idx > 0) {
				صحيح px0 = max(x + startx, clip->x0);
				صحيح px1 = min(x + endx, clip->x1);

				إذا (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_changed = true;
				} وإلا {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			إذا (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (صحيح)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				أرجع false;
			}

			/* draw any text succeeding highlighted portion */
			إذا (endtxt_idx < utf8_len) {
				صحيح px0 = max(x + endx, clip->x0);
				إذا (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (صحيح)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					إذا (res != NSERROR_OK) {
						أرجع false;
					}
				}
			}

			إذا (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				أرجع false;
			}
		}
	}

	إذا (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (صحيح) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
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

ساكن bool html_redraw_checkbox(صحيح x, صحيح y, صحيح width, صحيح height,
		bool selected, ثابت بنية redraw_context *ctx)
{
	مزدوج z;
	nserror res;
	بنية rect rect;

	z = width * 0.15;
	إذا (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->قۇر(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->قۇر(ctx, plot_style_stroke_darkwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->قۇر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->قۇر(ctx, plot_style_stroke_lightwbasec, &rect);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		إذا (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->قۇر(ctx, plot_style_stroke_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->قۇر(ctx, plot_style_stroke_wblobc, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}
	}
	أرجع true;
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
ساكن bool html_redraw_radio(صحيح x, صحيح y, صحيح width, صحيح height,
		bool selected, ثابت بنية redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}

	إذا (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}

	أرجع true;
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

ساكن bool html_redraw_file(صحيح x, صحيح y, صحيح width, صحيح height,
		بنية box *box, عائم scale, colour background_colour,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	صحيح text_width;
	ثابت حرف *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	إذا (box->gadget->قىممەت) {
		text = box->gadget->قىممەت;
	} وإلا {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}
	text_width *= scale;
	إذا (width < text_width + 8) {
		x = x + width - text_width - 4;
	} وإلا {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	إذا (res != NSERROR_OK) {
		أرجع false;
	}
	أرجع true;
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

ساكن bool html_redraw_background(صحيح x, صحيح y, بنية box *box, عائم scale,
		ثابت بنية rect *clip, colour *background_colour,
		بنية box *background,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	بنية box *clip_box = box;
	صحيح ox = x, oy = y;
	صحيح width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	بنية box *parent;
	بنية rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	إذا (ctx->background_images == false)
		أرجع true;

	plot_content = (background->background != عدم);

	إذا (plot_content) {
		إذا (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[سول_2] * scale;
			y -= box->margin[چوققا_2] * scale;
			width = box->margin[سول_2] + box->padding[سول_2] +
					box->width + box->padding[ئوڭ_2] +
					box->margin[ئوڭ_2];
			height = box->margin[چوققا_2] + box->padding[چوققا_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} وإلا {
			width = box->padding[سول_2] + box->width +
					box->padding[ئوڭ_2];
			height = box->padding[چوققا_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		بدل (css_computed_background_repeat(background->style)) {
		حالة CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
			اكسر;

		افتراضي:
			اكسر;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		إذا (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} وإلا {
			y += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	إذا (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		لكل (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		أكد(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		إذا (clip_to_children)
			clip_box = box->children;
	}

	لكل (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		إذا (clip_to_children) {
			أكد(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[سول_2] +
					clip_box->width +
					clip_box->padding[ئوڭ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[چوققا_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (r.x1 > clip->x1) r.x1 = clip->x1;
			إذا (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			إذا ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != عدم) &&
			     content_get_opaque(clip_box->background)))
				استمر;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		إذا (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			إذا (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				إذا (res != NSERROR_OK) {
					أرجع false;
				}
			}
		}
		/* and plot the image */
		إذا (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			إذا (!repeat_x) {
				إذا (r.x0 < x)
					r.x0 = x;
				إذا (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			إذا (!repeat_y) {
				إذا (r.y0 < y)
					r.y0 = y;
				إذا (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				بنية content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				إذا (res != NSERROR_OK) {
					أرجع false;
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
		إذا (!clip_to_children)
			أرجع true;
	}
	أرجع true;
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

ساكن bool html_redraw_inline_background(صحيح x, صحيح y, بنية box *box,
		عائم scale, ثابت بنية rect *clip, بنية rect b,
		bool first, bool last, colour *background_colour,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت بنية redraw_context *ctx)
{
	بنية rect r = *clip;
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

	plot_content = (box->background != عدم);

	إذا (html_redraw_printing && nsoption_bool(remove_backgrounds))
		أرجع true;

	إذا (plot_content) {
		/* handle background-repeat */
		بدل (css_computed_background_repeat(box->style)) {
		حالة CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			اكسر;

		حالة CSS_BACKGROUND_REPEAT_NO_REPEAT:
			اكسر;

		افتراضي:
			اكسر;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		إذا (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			إذا (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} وإلا {
			x += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		إذا (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} وإلا {
			y += (صحيح) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	إذا (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		إذا (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		}
	}
	/* and plot the image */
	إذا (plot_content) {
		صحيح width = content_get_width(box->background);
		صحيح height = content_get_height(box->background);

		إذا (!repeat_x) {
			إذا (r.x0 < x)
				r.x0 = x;
			إذا (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		إذا (!repeat_y) {
			إذا (r.y0 < y)
				r.y0 = y;
			إذا (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		إذا ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			بنية content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			إذا (res != NSERROR_OK) {
				أرجع false;
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

	أرجع true;
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

ساكن bool
html_redraw_text_decoration_inline(بنية box *box,
				   صحيح x, صحيح y,
				   عائم scale,
				   colour colour,
				   عائم ratio,
				   ثابت بنية redraw_context *ctx)
{
	بنية box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	بنية rect rect;

	لكل (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		إذا (c->type != BOX_TEXT) {
			استمر;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->قۇر(ctx, &plot_style_box, &rect);
		إذا (res != NSERROR_OK) {
			أرجع false;
		}
	}
	أرجع true;
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

ساكن bool
html_redraw_text_decoration_block(بنية box *box,
				  صحيح x, صحيح y,
				  عائم scale,
				  colour colour,
				  عائم ratio,
				  ثابت بنية redraw_context *ctx)
{
	بنية box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	بنية rect rect;

	/* draw through text descendants */
	لكل (c = box->children; c; c = c->next) {
		إذا (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->قۇر(ctx, &plot_style_box, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}
		} وإلا إذا ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			إذا (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				أرجع false;
		}
	}
	أرجع true;
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

ساكن bool html_redraw_text_decoration(بنية box *box,
		صحيح x_parent, صحيح y_parent, عائم scale,
		colour background_colour, ثابت بنية redraw_context *ctx)
{
	ساكن ثابت تعداد css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_قۇر_THROUGH };
	ساكن ثابت عائم قۇر_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned صحيح i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	إذا (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	إذا (box->type == BOX_INLINE) {
		إذا (!box->inline_end)
			أرجع true;
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_text_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, قۇر_ratio[i], ctx))
					أرجع false;
	} وإلا {
		لكل (i = 0; i != NOF_ELEMENTS(decoration); i++)
			إذا (css_computed_text_decoration(box->style) &
					decoration[i])
				إذا (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, قۇر_ratio[i], ctx))
					أرجع false;
	}

	أرجع true;
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

ساكن bool html_redraw_text_box(ثابت html_content *html, بنية box *box,
		صحيح x, صحيح y, ثابت بنية rect *clip, عائم scale,
		colour current_background_color,
		ثابت بنية redraw_context *ctx)
{
	bool excluded = (box->object != عدم);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	إذا (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (بنية content *)html,
			 html->sel,
			 ctx))
		أرجع false;

	أرجع true;
}

bool html_redraw_box(ثابت html_content *html, بنية box *box,
		صحيح x_parent, صحيح y_parent,
		ثابت بنية rect *clip, عائم scale,
		colour current_background_color,
		ثابت بنية redraw_context *ctx);

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

ساكن bool html_redraw_box_children(ثابت html_content *html, بنية box *box,
		صحيح x_parent, صحيح y_parent,
		ثابت بنية rect *clip, عائم scale,
		colour current_background_color,
		ثابت بنية redraw_context *ctx)
{
	بنية box *c;

	لكل (c = box->children; c; c = c->next) {

		إذا (c->type != BOX_FLOAT_سول && c->type != BOX_FLOAT_ئوڭ)
			إذا (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				أرجع false;
	}
	لكل (c = box->float_children; c; c = c->next_float)
		إذا (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			أرجع false;

	أرجع true;
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

bool html_redraw_box(ثابت html_content *html, بنية box *box,
		صحيح x_parent, صحيح y_parent,
		ثابت بنية rect *clip, ثابت عائم scale,
		colour current_background_color,
		ثابت بنية redraw_context *ctx)
{
	ثابت بنية plotter_table *plot = ctx->plot;
	صحيح x, y;
	صحيح width, height;
	صحيح padding_سول, padding_چوققا, padding_width, padding_height;
	صحيح border_سول, border_چوققا, border_ئوڭ, border_bottom;
	بنية rect r;
	بنية rect rect;
	صحيح x_scrolled, y_scrolled;
	بنية box *bg_box = عدم;
	css_computed_clip_rect css_rect;
	تعداد css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	تعداد css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	إذا (html_redraw_printing && (box->flags & PRINTED))
		أرجع true;

	إذا (box->style != عدم) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	إذا (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_سول = box->padding[سول_2];
		padding_چوققا = box->padding[چوققا_2];
		padding_width = padding_سول + box->width + box->padding[ئوڭ_2];
		padding_height = padding_چوققا + box->height +
				box->padding[BOTTOM];
		border_سول = box->border[سول_2].width;
		border_چوققا = box->border[چوققا_2].width;
		border_ئوڭ = box->border[ئوڭ_2].width;
		border_bottom = box->border[BOTTOM].width;
	} وإلا {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_سول = box->padding[سول_2] ? box->padding[سول_2] * scale
				: 0;
		padding_چوققا = box->padding[چوققا_2] ? box->padding[چوققا_2] * scale
				: 0;
		padding_width = (box->padding[سول_2] + box->width +
				box->padding[ئوڭ_2]) * scale;
		padding_height = (box->padding[چوققا_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_سول = box->border[سول_2].width * scale;
		border_چوققا = box->border[چوققا_2].width * scale;
		border_ئوڭ = box->border[ئوڭ_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != عدم) {
		/* box contents clipped to box size */
		r.x0 = x - border_سول;
		r.x1 = x + padding_width + border_ئوڭ;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} وإلا {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		إذا (!box->parent) {
			/* root element */
			صحيح margin_سول, margin_ئوڭ;
			إذا (scale == 1.0) {
				margin_سول = box->margin[سول_2];
				margin_ئوڭ = box->margin[ئوڭ_2];
			} وإلا {
				margin_سول = box->margin[سول_2] * scale;
				margin_ئوڭ = box->margin[ئوڭ_2] * scale;
			}
			r.x0 = x - border_سول - margin_سول < r.x0 ?
					x - border_سول - margin_سول : r.x0;
			r.x1 = x + padding_width + border_ئوڭ +
					margin_ئوڭ > r.x1 ?
					x + padding_width + border_ئوڭ +
					margin_ئوڭ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	إذا (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != عدم) {
		/* box contents clipped to box size */
		r.y0 = y - border_چوققا;
		r.y1 = y + padding_height + border_bottom;
	} وإلا {
		/* box contents can hang out of the box; use descendant box */
		إذا (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} وإلا {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		إذا (!box->parent) {
			/* root element */
			صحيح margin_چوققا, margin_bottom;
			إذا (scale == 1.0) {
				margin_چوققا = box->margin[چوققا_2];
				margin_bottom = box->margin[BOTTOM];
			} وإلا {
				margin_چوققا = box->margin[چوققا_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_چوققا - margin_چوققا < r.y0 ?
					y - border_چوققا - margin_چوققا : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	إذا (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		أرجع true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	إذا (html_redraw_printing) {
		إذا (r.y1 > html_redraw_printing_border) {
			إذا (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				إذا (r.y0 < html_redraw_printing_چوققا_cropped)
					html_redraw_printing_چوققا_cropped = r.y0;
				أرجع true;
			}
		}
		وإلا box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	إذا (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		إذا ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			أرجع false;
		إذا (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			أرجع false;
		أرجع ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	إذا ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		أرجع false;
	}

	إذا (box->style != عدم &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		إذا (css_rect.سول_auto == false)
			r.x0 = x - border_سول + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.سول, css_rect.lunit));

		إذا (css_rect.چوققا_auto == false)
			r.y0 = y - border_چوققا + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.چوققا, css_rect.tunit));

		إذا (css_rect.ئوڭ_auto == false)
			r.x1 = x - border_سول + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ئوڭ, css_rect.runit));

		إذا (css_rect.bottom_auto == false)
			r.y1 = y - border_چوققا + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		إذا (r.x0 < clip->x0) r.x0 = clip->x0;
		إذا (r.y0 < clip->y0) r.y0 = clip->y0;
		إذا (clip->x1 < r.x1) r.x1 = clip->x1;
		إذا (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		إذا (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			أرجع ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;

	} وإلا إذا (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		إذا (r.x0 < clip->x0) r.x0 = clip->x0;
		إذا (r.y0 < clip->y0) r.y0 = clip->y0;
		إذا (clip->x1 < r.x1) r.x1 = clip->x1;
		إذا (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		إذا (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			أرجع ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
	} وإلا {
		/* clip box is fine, clip to it */
		r = *clip;
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	إذا (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != عدم &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		بنية rect p;
		p.x0 = x - border_سول < r.x0 ? r.x0 : x - border_سول;
		p.y0 = y - border_چوققا < r.y0 ? r.y0 : y - border_چوققا;
		p.x1 = x + padding_width + border_ئوڭ < r.x1 ?
				x + padding_width + border_ئوڭ : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		إذا (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			صحيح m_سول, m_چوققا, m_ئوڭ, m_bottom;
			إذا (scale == 1.0) {
				m_سول = box->margin[سول_2];
				m_چوققا = box->margin[چوققا_2];
				m_ئوڭ = box->margin[ئوڭ_2];
				m_bottom = box->margin[BOTTOM];
			} وإلا {
				m_سول = box->margin[سول_2] * scale;
				m_چوققا = box->margin[چوققا_2] * scale;
				m_ئوڭ = box->margin[ئوڭ_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_سول < r.x0 ? r.x0 : p.x0 - m_سول;
			p.y0 = p.y0 - m_چوققا < r.y0 ? r.y0 : p.y0 - m_چوققا;
			p.x1 = p.x1 + m_ئوڭ < r.x1 ? p.x1 + m_ئوڭ : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		إذا ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			إذا (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				أرجع false;
			/* restore previous graphics window */
			إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				أرجع false;
		}
	}

	/* borders for block level content and replaced inlines */
	إذا (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != عدم &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_چوققا || border_ئوڭ || border_bottom || border_سول)) {
		إذا (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			أرجع false;
	}

	/* backgrounds and borders for non-replaced inlines */
	إذا (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_چوققا || border_ئوڭ ||
			border_bottom || border_سول)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		بنية box *ib;
		بنية rect b; /* border edge rectangle */
		بنية rect p; /* clipped rect */
		bool first = true;
		صحيح ib_x;
		صحيح ib_y = y;
		صحيح ib_p_width;
		صحيح ib_b_سول, ib_b_ئوڭ;

		b.x0 = x - border_سول;
		b.x1 = x + padding_width + border_ئوڭ;
		b.y0 = y - border_چوققا;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		لكل (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			إذا (ib->type == BOX_FLOAT_سول ||
					ib->type == BOX_FLOAT_ئوڭ)
				استمر;
			إذا (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[سول_2] + ib->width +
						ib->padding[ئوڭ_2];
				ib_b_سول = ib->border[سول_2].width;
				ib_b_ئوڭ = ib->border[ئوڭ_2].width;
			} وإلا {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[سول_2] + ib->width +
						ib->padding[ئوڭ_2]) * scale;
				ib_b_سول = ib->border[سول_2].width * scale;
				ib_b_ئوڭ = ib->border[ئوڭ_2].width * scale;
			}

			إذا ((ib->flags & NEW_قۇر) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				إذا (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					أرجع false;
				/* restore previous graphics window */
				إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					أرجع false;
				إذا (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					أرجع false;
				/* reset coords */
				b.x0 = ib_x - ib_b_سول;
				b.y0 = ib_y - border_چوققا - padding_چوققا;
				b.y1 = ib_y + padding_height - padding_چوققا +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_ئوڭ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			إذا (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				اكسر;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		إذا (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			أرجع false;
		/* restore previous graphics window */
		إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			أرجع false;
		إذا (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			أرجع false;

	}

	/* Debug outlines */
	إذا (html_redraw_debug) {
		صحيح margin_سول, margin_ئوڭ;
		صحيح margin_چوققا, margin_bottom;
		إذا (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_سول = box->margin[سول_2];
			margin_چوققا = box->margin[چوققا_2];
			margin_ئوڭ = box->margin[ئوڭ_2];
			margin_bottom = box->margin[BOTTOM];
		} وإلا {
			margin_سول = box->margin[سول_2] * scale;
			margin_چوققا = box->margin[چوققا_2] * scale;
			margin_ئوڭ = box->margin[ئوڭ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_سول;
		rect.y0 = y + padding_چوققا;
		rect.x1 = x + padding_سول + width;
		rect.y1 = y + padding_چوققا + height;
		إذا (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			أرجع false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		إذا (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			أرجع false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_سول - margin_سول;
		rect.y0 = y - border_چوققا - margin_چوققا;
		rect.x1 = x + padding_width + border_ئوڭ + margin_ئوڭ;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		إذا (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			أرجع false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	إذا (box->parent != عدم) {
		bool need_clip = false;
		إذا (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.x1 <= r.x0 || r.y1 <= r.y0) {
				أرجع (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			إذا (r.x0 < clip->x0) r.x0 = clip->x0;
			إذا (clip->x1 < r.x1) r.x1 = clip->x1;
			إذا (r.x1 <= r.x0) {
				أرجع (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} وإلا إذا (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			إذا (r.y0 < clip->y0) r.y0 = clip->y0;
			إذا (clip->y1 < r.y1) r.y1 = clip->y1;
			إذا (r.y1 <= r.y0) {
				أرجع (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		إذا (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			إذا (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				أرجع false;
		}
	}

	/* text decoration */
	إذا ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		إذا (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->تۈگۈن != عدم) {
		exc = dom_html_element_get_tag_type(box->تۈگۈن, &tag_type);
		إذا (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} وإلا {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	إذا (box->object && width != 0 && height != 0) {
		بنية content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_سول;
		obj_data.y = y_scrolled + padding_چوققا;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		إذا (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		إذا (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ثابت حرف *obj = "\xef\xbf\xbc";
			صحيح obj_width;
			صحيح obj_x = x + padding_سول;
			nserror res;

			rect.x0 = x + padding_سول;
			rect.y0 = y + padding_چوققا;
			rect.x1 = x + padding_سول + width - 1;
			rect.y1 = y + padding_چوققا + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			إذا (res != NSERROR_OK) {
				أرجع false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  حجم(obj) - 1,
						  &obj_width);
			إذا (res != NSERROR_OK) {
				obj_x += 1;
			} وإلا {
				obj_x += width / 2 - obj_width / 2;
			}

			إذا (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_چوققا + (صحيح)(height * 0.75),
					    obj, حجم(obj) - 1) != NSERROR_OK)
				أرجع false;
		}
	} وإلا إذا (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->تۈگۈن != عدم &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		بنية bitmap *bitmap = عدم;
		exc = dom_تۈگۈن_get_user_data(box->تۈگۈن,
					     corestring_dom_ns_ئاچقۇچ_canvas_تۈگۈن_data,
					     &bitmap);
		إذا (exc != DOM_NO_ERR) {
			bitmap = عدم;
		}
		إذا (bitmap != عدم &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_سول, y + padding_چوققا,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			أرجع false;
	} وإلا إذا (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_سول,
				y + padding_چوققا, &r, ctx);

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		إذا (!html_redraw_checkbox(x + padding_سول, y + padding_چوققا,
				width, height, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_RADIO) {
		إذا (!html_redraw_radio(x + padding_سول, y + padding_چوققا,
				width, height, box->gadget->selected, ctx))
			أرجع false;

	} وإلا إذا (box->gadget && box->gadget->type == GADGET_FILE) {
		إذا (!html_redraw_file(x + padding_سول, y + padding_چوققا,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			أرجع false;

	} وإلا إذا (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} وإلا إذا (box->text) {
		إذا (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			أرجع false;

	} وإلا {
		إذا (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			أرجع false;
	}

	إذا (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;

	/* list marker */
	إذا (box->list_marker) {
		إذا (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			أرجع false;
	}

	/* scrollbars */
	إذا (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == عدم || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != عدم) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((بنية content *)html,
					    box, has_x_scroll, has_y_scroll);
		إذا (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			أرجع false;
		}

		إذا (box->scroll_x != عدم)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[چوققا_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		إذا (box->scroll_y != عدم)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[سول_2] +
					box->width + box->padding[ئوڭ_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	إذا (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		إذا (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			أرجع false;
	}

	أرجع ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(بنية content *c, بنية content_redraw_data *data,
		ثابت بنية rect *clip, ثابت بنية redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	بنية box *box;
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
	إذا (ctx->interactive && html->visible_select_menu != عدم) {
		بنية form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	إذا (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		إذا (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	إذا (select) {
		صحيح menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[سول_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[چوققا_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	أرجع result;

}
