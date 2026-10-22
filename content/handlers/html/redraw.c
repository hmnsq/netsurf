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

#அந்தர்பாவ "utils/config.h"
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/browser_window.h"
#அந்தர்பாவ "netsurf/plotters.h"
#அந்தர்பாவ "netsurf/bitmap.h"
#அந்தர்பாவ "netsurf/layout.h"
#அந்தர்பாவ "content/content.h"
#அந்தர்பாவ "content/content_protected.h"
#அந்தர்பாவ "content/textsearch.h"
#அந்தர்பாவ "css/utils.h"
#அந்தர்பாவ "desktop/selection.h"
#அந்தர்பாவ "desktop/print.h"
#அந்தர்பாவ "desktop/scrollbar.h"
#அந்தர்பாவ "desktop/textarea.h"
#அந்தர்பாவ "desktop/gui_internal.h"

#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"
#அந்தர்பாவ "html/box_manipulate.h"
#அந்தர்பாவ "html/font.h"
#அந்தர்பாவ "html/form_internal.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
நிலையான bool html_redraw_box_has_background(கட்டமைப்பு box *box)
{
	எனில் (box->background != வெற்று)
		திரும்பு true;

	எனில் (box->style != வெற்று) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		எனில் (nscss_color_is_transparent(colour) == false)
			திரும்பு true;
	}

	திரும்பு false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
நிலையான கட்டமைப்பு box *html_redraw_find_bg_box(கட்டமைப்பு box *box)
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
	எனில் (box->parent == வெற்று) {
		/* Root box */
		எனில் (html_redraw_box_has_background(box))
			திரும்பு box;

		/* No background on root box: consider body box, if any */
		எனில் (box->children != வெற்று) {
			எனில் (html_redraw_box_has_background(box->children))
				திரும்பு box->children;
		}
	} இல்லையெனில் எனில் (box->parent != வெற்று && box->parent->parent == வெற்று) {
		/* Body box: only render background if root has its own */
		எனில் (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			திரும்பு box;
	} இல்லையெனில் {
		/* Any other box */
		எனில் (html_redraw_box_has_background(box))
			திரும்பு box;
	}

	திரும்பு வெற்று;
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

நிலையான bool
text_redraw(மாறிலி எழுத்து *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    முழு space,
	    மாறிலி plot_font_style_t *fstyle,
	    முழு x,
	    முழு y,
	    மாறிலி கட்டமைப்பு rect *clip,
	    முழு height,
	    தசம scale,
	    bool excluded,
	    கட்டமைப்பு content *c,
	    மாறிலி கட்டமைப்பு selection *sel,
	    மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	எனில் (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		எனில் (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		எனில் (!highlighted &&
		    (c->textsearch.context != வெற்று) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		எனில் (highlighted) {
			கட்டமைப்பு rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			முழு startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			எனில் (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				ப்ரதிஜ்ஞா(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			எனில் (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			எனில் (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			எனில் (end_idx > utf8_len) {
					endx += space;
			}

			எனில் (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			எனில் ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (முழு)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				திரும்பு false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}

			எனில் (start_idx > 0) {
				முழு px0 = max(x + startx, clip->x0);
				முழு px1 = min(x + endx, clip->x1);

				எனில் (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}

					clip_changed = true;
				} இல்லையெனில் {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			எனில் (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (முழு)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				திரும்பு false;
			}

			/* draw any text succeeding highlighted portion */
			எனில் (endtxt_idx < utf8_len) {
				முழு px0 = max(x + endx, clip->x0);
				எனில் (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (முழு)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					எனில் (res != NSERROR_OK) {
						திரும்பு false;
					}
				}
			}

			எனில் (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				திரும்பு false;
			}
		}
	}

	எனில் (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (முழு) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}
	திரும்பு true;
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

நிலையான bool html_redraw_checkbox(முழு x, முழு y, முழு width, முழு height,
		bool selected, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	இரட்டை z;
	nserror res;
	கட்டமைப்பு rect rect;

	z = width * 0.15;
	எனில் (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->வரி(ctx, plot_style_stroke_darkwbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->வரி(ctx, plot_style_stroke_darkwbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->வரி(ctx, plot_style_stroke_lightwbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->வரி(ctx, plot_style_stroke_lightwbasec, &rect);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	எனில் (selected) {
		எனில் (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
		} இல்லையெனில் {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->வரி(ctx, plot_style_stroke_wblobc, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->வரி(ctx, plot_style_stroke_wblobc, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
		}
	}
	திரும்பு true;
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
நிலையான bool html_redraw_radio(முழு x, முழு y, முழு width, முழு height,
		bool selected, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}

	எனில் (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}

	திரும்பு true;
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

நிலையான bool html_redraw_file(முழு x, முழு y, முழு width, முழு height,
		கட்டமைப்பு box *box, தசம scale, colour background_colour,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	முழு text_width;
	மாறிலி எழுத்து *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	எனில் (box->gadget->மதிப்பு) {
		text = box->gadget->மதிப்பு;
	} இல்லையெனில் {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}
	text_width *= scale;
	எனில் (width < text_width + 8) {
		x = x + width - text_width - 4;
	} இல்லையெனில் {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	எனில் (res != NSERROR_OK) {
		திரும்பு false;
	}
	திரும்பு true;
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

நிலையான bool html_redraw_background(முழு x, முழு y, கட்டமைப்பு box *box, தசம scale,
		மாறிலி கட்டமைப்பு rect *clip, colour *background_colour,
		கட்டமைப்பு box *background,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	கட்டமைப்பு box *clip_box = box;
	முழு ox = x, oy = y;
	முழு width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	கட்டமைப்பு box *parent;
	கட்டமைப்பு rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	எனில் (ctx->background_images == false)
		திரும்பு true;

	plot_content = (background->background != வெற்று);

	எனில் (plot_content) {
		எனில் (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[இடது_2] * scale;
			y -= box->margin[உச்சி_2] * scale;
			width = box->margin[இடது_2] + box->padding[இடது_2] +
					box->width + box->padding[வலது_2] +
					box->margin[வலது_2];
			height = box->margin[உச்சி_2] + box->padding[உச்சி_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} இல்லையெனில் {
			width = box->padding[இடது_2] + box->width +
					box->padding[வலது_2];
			height = box->padding[உச்சி_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		தேர்வு (css_computed_background_repeat(background->style)) {
		நிலை CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_NO_REPEAT:
			நிறுத்து;

		இயல்பு:
			நிறுத்து;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		எனில் (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} இல்லையெனில் {
			x += (முழு) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		எனில் (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} இல்லையெனில் {
			y += (முழு) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	எனில் (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		க்கு (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		ப்ரதிஜ்ஞா(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		எனில் (clip_to_children)
			clip_box = box->children;
	}

	க்கு (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		எனில் (clip_to_children) {
			ப்ரதிஜ்ஞா(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[இடது_2] +
					clip_box->width +
					clip_box->padding[வலது_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[உச்சி_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			எனில் (r.x0 < clip->x0) r.x0 = clip->x0;
			எனில் (r.y0 < clip->y0) r.y0 = clip->y0;
			எனில் (r.x1 > clip->x1) r.x1 = clip->x1;
			எனில் (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			எனில் ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != வெற்று) &&
			     content_get_opaque(clip_box->background)))
				தொடர்;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		எனில் (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			எனில் (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				எனில் (res != NSERROR_OK) {
					திரும்பு false;
				}
			}
		}
		/* and plot the image */
		எனில் (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			எனில் (!repeat_x) {
				எனில் (r.x0 < x)
					r.x0 = x;
				எனில் (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			எனில் (!repeat_y) {
				எனில் (r.y0 < y)
					r.y0 = y;
				எனில் (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			எனில் ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				கட்டமைப்பு content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				எனில் (res != NSERROR_OK) {
					திரும்பு false;
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
		எனில் (!clip_to_children)
			திரும்பு true;
	}
	திரும்பு true;
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

நிலையான bool html_redraw_inline_background(முழு x, முழு y, கட்டமைப்பு box *box,
		தசம scale, மாறிலி கட்டமைப்பு rect *clip, கட்டமைப்பு rect b,
		bool first, bool last, colour *background_colour,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு rect r = *clip;
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

	plot_content = (box->background != வெற்று);

	எனில் (html_redraw_printing && nsoption_bool(remove_backgrounds))
		திரும்பு true;

	எனில் (plot_content) {
		/* handle background-repeat */
		தேர்வு (css_computed_background_repeat(box->style)) {
		நிலை CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			நிறுத்து;

		நிலை CSS_BACKGROUND_REPEAT_NO_REPEAT:
			நிறுத்து;

		இயல்பு:
			நிறுத்து;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		எனில் (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			எனில் (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} இல்லையெனில் {
			x += (முழு) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		எனில் (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} இல்லையெனில் {
			y += (முழு) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	எனில் (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		எனில் (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
		}
	}
	/* and plot the image */
	எனில் (plot_content) {
		முழு width = content_get_width(box->background);
		முழு height = content_get_height(box->background);

		எனில் (!repeat_x) {
			எனில் (r.x0 < x)
				r.x0 = x;
			எனில் (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		எனில் (!repeat_y) {
			எனில் (r.y0 < y)
				r.y0 = y;
			எனில் (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		எனில் ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			கட்டமைப்பு content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
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

	திரும்பு true;
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

நிலையான bool
html_redraw_text_decoration_inline(கட்டமைப்பு box *box,
				   முழு x, முழு y,
				   தசம scale,
				   colour colour,
				   தசம ratio,
				   மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	கட்டமைப்பு rect rect;

	க்கு (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		எனில் (c->type != BOX_TEXT) {
			தொடர்;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->வரி(ctx, &plot_style_box, &rect);
		எனில் (res != NSERROR_OK) {
			திரும்பு false;
		}
	}
	திரும்பு true;
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

நிலையான bool
html_redraw_text_decoration_block(கட்டமைப்பு box *box,
				  முழு x, முழு y,
				  தசம scale,
				  colour colour,
				  தசம ratio,
				  மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	கட்டமைப்பு rect rect;

	/* draw through text descendants */
	க்கு (c = box->children; c; c = c->next) {
		எனில் (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->வரி(ctx, &plot_style_box, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}
		} இல்லையெனில் எனில் ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			எனில் (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				திரும்பு false;
		}
	}
	திரும்பு true;
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

நிலையான bool html_redraw_text_decoration(கட்டமைப்பு box *box,
		முழு x_parent, முழு y_parent, தசம scale,
		colour background_colour, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	நிலையான மாறிலி எண்ணிக்கை css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_வரி_THROUGH };
	நிலையான மாறிலி தசம வரி_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned முழு i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	எனில் (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	எனில் (box->type == BOX_INLINE) {
		எனில் (!box->inline_end)
			திரும்பு true;
		க்கு (i = 0; i != NOF_ELEMENTS(decoration); i++)
			எனில் (css_computed_text_decoration(box->style) &
					decoration[i])
				எனில் (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, வரி_ratio[i], ctx))
					திரும்பு false;
	} இல்லையெனில் {
		க்கு (i = 0; i != NOF_ELEMENTS(decoration); i++)
			எனில் (css_computed_text_decoration(box->style) &
					decoration[i])
				எனில் (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, வரி_ratio[i], ctx))
					திரும்பு false;
	}

	திரும்பு true;
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

நிலையான bool html_redraw_text_box(மாறிலி html_content *html, கட்டமைப்பு box *box,
		முழு x, முழு y, மாறிலி கட்டமைப்பு rect *clip, தசம scale,
		colour current_background_color,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	bool excluded = (box->object != வெற்று);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	எனில் (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (கட்டமைப்பு content *)html,
			 html->sel,
			 ctx))
		திரும்பு false;

	திரும்பு true;
}

bool html_redraw_box(மாறிலி html_content *html, கட்டமைப்பு box *box,
		முழு x_parent, முழு y_parent,
		மாறிலி கட்டமைப்பு rect *clip, தசம scale,
		colour current_background_color,
		மாறிலி கட்டமைப்பு redraw_context *ctx);

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

நிலையான bool html_redraw_box_children(மாறிலி html_content *html, கட்டமைப்பு box *box,
		முழு x_parent, முழு y_parent,
		மாறிலி கட்டமைப்பு rect *clip, தசம scale,
		colour current_background_color,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	கட்டமைப்பு box *c;

	க்கு (c = box->children; c; c = c->next) {

		எனில் (c->type != BOX_FLOAT_இடது && c->type != BOX_FLOAT_வலது)
			எனில் (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				திரும்பு false;
	}
	க்கு (c = box->float_children; c; c = c->next_float)
		எனில் (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			திரும்பு false;

	திரும்பு true;
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

bool html_redraw_box(மாறிலி html_content *html, கட்டமைப்பு box *box,
		முழு x_parent, முழு y_parent,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி தசம scale,
		colour current_background_color,
		மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	மாறிலி கட்டமைப்பு plotter_table *plot = ctx->plot;
	முழு x, y;
	முழு width, height;
	முழு padding_இடது, padding_உச்சி, padding_width, padding_height;
	முழு border_இடது, border_உச்சி, border_வலது, border_bottom;
	கட்டமைப்பு rect r;
	கட்டமைப்பு rect rect;
	முழு x_scrolled, y_scrolled;
	கட்டமைப்பு box *bg_box = வெற்று;
	css_computed_clip_rect css_rect;
	எண்ணிக்கை css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	எண்ணிக்கை css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	எனில் (html_redraw_printing && (box->flags & PRINTED))
		திரும்பு true;

	எனில் (box->style != வெற்று) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	எனில் (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_இடது = box->padding[இடது_2];
		padding_உச்சி = box->padding[உச்சி_2];
		padding_width = padding_இடது + box->width + box->padding[வலது_2];
		padding_height = padding_உச்சி + box->height +
				box->padding[BOTTOM];
		border_இடது = box->border[இடது_2].width;
		border_உச்சி = box->border[உச்சி_2].width;
		border_வலது = box->border[வலது_2].width;
		border_bottom = box->border[BOTTOM].width;
	} இல்லையெனில் {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_இடது = box->padding[இடது_2] ? box->padding[இடது_2] * scale
				: 0;
		padding_உச்சி = box->padding[உச்சி_2] ? box->padding[உச்சி_2] * scale
				: 0;
		padding_width = (box->padding[இடது_2] + box->width +
				box->padding[வலது_2]) * scale;
		padding_height = (box->padding[உச்சி_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_இடது = box->border[இடது_2].width * scale;
		border_உச்சி = box->border[உச்சி_2].width * scale;
		border_வலது = box->border[வலது_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	எனில் (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != வெற்று) {
		/* box contents clipped to box size */
		r.x0 = x - border_இடது;
		r.x1 = x + padding_width + border_வலது;
	} இல்லையெனில் {
		/* box contents can hang out of the box; use descendant box */
		எனில் (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} இல்லையெனில் {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		எனில் (!box->parent) {
			/* root element */
			முழு margin_இடது, margin_வலது;
			எனில் (scale == 1.0) {
				margin_இடது = box->margin[இடது_2];
				margin_வலது = box->margin[வலது_2];
			} இல்லையெனில் {
				margin_இடது = box->margin[இடது_2] * scale;
				margin_வலது = box->margin[வலது_2] * scale;
			}
			r.x0 = x - border_இடது - margin_இடது < r.x0 ?
					x - border_இடது - margin_இடது : r.x0;
			r.x1 = x + padding_width + border_வலது +
					margin_வலது > r.x1 ?
					x + padding_width + border_வலது +
					margin_வலது : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	எனில் (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != வெற்று) {
		/* box contents clipped to box size */
		r.y0 = y - border_உச்சி;
		r.y1 = y + padding_height + border_bottom;
	} இல்லையெனில் {
		/* box contents can hang out of the box; use descendant box */
		எனில் (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} இல்லையெனில் {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		எனில் (!box->parent) {
			/* root element */
			முழு margin_உச்சி, margin_bottom;
			எனில் (scale == 1.0) {
				margin_உச்சி = box->margin[உச்சி_2];
				margin_bottom = box->margin[BOTTOM];
			} இல்லையெனில் {
				margin_உச்சி = box->margin[உச்சி_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_உச்சி - margin_உச்சி < r.y0 ?
					y - border_உச்சி - margin_உச்சி : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	எனில் (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		திரும்பு true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	எனில் (html_redraw_printing) {
		எனில் (r.y1 > html_redraw_printing_border) {
			எனில் (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				எனில் (r.y0 < html_redraw_printing_உச்சி_cropped)
					html_redraw_printing_உச்சி_cropped = r.y0;
				திரும்பு true;
			}
		}
		இல்லையெனில் box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	எனில் (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		எனில் ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			திரும்பு false;
		எனில் (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			திரும்பு false;
		திரும்பு ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	எனில் ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		திரும்பு false;
	}

	எனில் (box->style != வெற்று &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		எனில் (css_rect.இடது_auto == false)
			r.x0 = x - border_இடது + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.இடது, css_rect.lunit));

		எனில் (css_rect.உச்சி_auto == false)
			r.y0 = y - border_உச்சி + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.உச்சி, css_rect.tunit));

		எனில் (css_rect.வலது_auto == false)
			r.x1 = x - border_இடது + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.வலது, css_rect.runit));

		எனில் (css_rect.bottom_auto == false)
			r.y1 = y - border_உச்சி + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		எனில் (r.x0 < clip->x0) r.x0 = clip->x0;
		எனில் (r.y0 < clip->y0) r.y0 = clip->y0;
		எனில் (clip->x1 < r.x1) r.x1 = clip->x1;
		எனில் (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		எனில் (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			திரும்பு ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			திரும்பு false;

	} இல்லையெனில் எனில் (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		எனில் (r.x0 < clip->x0) r.x0 = clip->x0;
		எனில் (r.y0 < clip->y0) r.y0 = clip->y0;
		எனில் (clip->x1 < r.x1) r.x1 = clip->x1;
		எனில் (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		எனில் (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			திரும்பு ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			திரும்பு false;
	} இல்லையெனில் {
		/* clip box is fine, clip to it */
		r = *clip;
		எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			திரும்பு false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	எனில் (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != வெற்று &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		கட்டமைப்பு rect p;
		p.x0 = x - border_இடது < r.x0 ? r.x0 : x - border_இடது;
		p.y0 = y - border_உச்சி < r.y0 ? r.y0 : y - border_உச்சி;
		p.x1 = x + padding_width + border_வலது < r.x1 ?
				x + padding_width + border_வலது : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		எனில் (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			முழு m_இடது, m_உச்சி, m_வலது, m_bottom;
			எனில் (scale == 1.0) {
				m_இடது = box->margin[இடது_2];
				m_உச்சி = box->margin[உச்சி_2];
				m_வலது = box->margin[வலது_2];
				m_bottom = box->margin[BOTTOM];
			} இல்லையெனில் {
				m_இடது = box->margin[இடது_2] * scale;
				m_உச்சி = box->margin[உச்சி_2] * scale;
				m_வலது = box->margin[வலது_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_இடது < r.x0 ? r.x0 : p.x0 - m_இடது;
			p.y0 = p.y0 - m_உச்சி < r.y0 ? r.y0 : p.y0 - m_உச்சி;
			p.x1 = p.x1 + m_வலது < r.x1 ? p.x1 + m_வலது : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		எனில் ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			எனில் (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				திரும்பு false;
			/* restore previous graphics window */
			எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				திரும்பு false;
		}
	}

	/* borders for block level content and replaced inlines */
	எனில் (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != வெற்று &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_உச்சி || border_வலது || border_bottom || border_இடது)) {
		எனில் (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			திரும்பு false;
	}

	/* backgrounds and borders for non-replaced inlines */
	எனில் (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_உச்சி || border_வலது ||
			border_bottom || border_இடது)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		கட்டமைப்பு box *ib;
		கட்டமைப்பு rect b; /* border edge rectangle */
		கட்டமைப்பு rect p; /* clipped rect */
		bool first = true;
		முழு ib_x;
		முழு ib_y = y;
		முழு ib_p_width;
		முழு ib_b_இடது, ib_b_வலது;

		b.x0 = x - border_இடது;
		b.x1 = x + padding_width + border_வலது;
		b.y0 = y - border_உச்சி;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		க்கு (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			எனில் (ib->type == BOX_FLOAT_இடது ||
					ib->type == BOX_FLOAT_வலது)
				தொடர்;
			எனில் (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[இடது_2] + ib->width +
						ib->padding[வலது_2];
				ib_b_இடது = ib->border[இடது_2].width;
				ib_b_வலது = ib->border[வலது_2].width;
			} இல்லையெனில் {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[இடது_2] + ib->width +
						ib->padding[வலது_2]) * scale;
				ib_b_இடது = ib->border[இடது_2].width * scale;
				ib_b_வலது = ib->border[வலது_2].width * scale;
			}

			எனில் ((ib->flags & NEW_வரி) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				எனில் (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					திரும்பு false;
				/* restore previous graphics window */
				எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					திரும்பு false;
				எனில் (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					திரும்பு false;
				/* reset coords */
				b.x0 = ib_x - ib_b_இடது;
				b.y0 = ib_y - border_உச்சி - padding_உச்சி;
				b.y1 = ib_y + padding_height - padding_உச்சி +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_வலது;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			எனில் (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				நிறுத்து;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		எனில் (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			திரும்பு false;
		/* restore previous graphics window */
		எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			திரும்பு false;
		எனில் (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			திரும்பு false;

	}

	/* Debug outlines */
	எனில் (html_redraw_debug) {
		முழு margin_இடது, margin_வலது;
		முழு margin_உச்சி, margin_bottom;
		எனில் (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_இடது = box->margin[இடது_2];
			margin_உச்சி = box->margin[உச்சி_2];
			margin_வலது = box->margin[வலது_2];
			margin_bottom = box->margin[BOTTOM];
		} இல்லையெனில் {
			margin_இடது = box->margin[இடது_2] * scale;
			margin_உச்சி = box->margin[உச்சி_2] * scale;
			margin_வலது = box->margin[வலது_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_இடது;
		rect.y0 = y + padding_உச்சி;
		rect.x1 = x + padding_இடது + width;
		rect.y1 = y + padding_உச்சி + height;
		எனில் (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			திரும்பு false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		எனில் (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			திரும்பு false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_இடது - margin_இடது;
		rect.y0 = y - border_உச்சி - margin_உச்சி;
		rect.x1 = x + padding_width + border_வலது + margin_வலது;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		எனில் (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			திரும்பு false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	எனில் (box->parent != வெற்று) {
		bool need_clip = false;
		எனில் (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			எனில் (r.x0 < clip->x0) r.x0 = clip->x0;
			எனில் (r.y0 < clip->y0) r.y0 = clip->y0;
			எனில் (clip->x1 < r.x1) r.x1 = clip->x1;
			எனில் (clip->y1 < r.y1) r.y1 = clip->y1;
			எனில் (r.x1 <= r.x0 || r.y1 <= r.y0) {
				திரும்பு (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} இல்லையெனில் எனில் (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			எனில் (r.x0 < clip->x0) r.x0 = clip->x0;
			எனில் (clip->x1 < r.x1) r.x1 = clip->x1;
			எனில் (r.x1 <= r.x0) {
				திரும்பு (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} இல்லையெனில் எனில் (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			எனில் (r.y0 < clip->y0) r.y0 = clip->y0;
			எனில் (clip->y1 < r.y1) r.y1 = clip->y1;
			எனில் (r.y1 <= r.y0) {
				திரும்பு (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		எனில் (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			எனில் (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				திரும்பு false;
		}
	}

	/* text decoration */
	எனில் ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		எனில் (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			திரும்பு false;
	}

	எனில் (box->முனை != வெற்று) {
		exc = dom_html_element_get_tag_type(box->முனை, &tag_type);
		எனில் (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} இல்லையெனில் {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	எனில் (box->object && width != 0 && height != 0) {
		கட்டமைப்பு content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_இடது;
		obj_data.y = y_scrolled + padding_உச்சி;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		எனில் (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		எனில் (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			மாறிலி எழுத்து *obj = "\xef\xbf\xbc";
			முழு obj_width;
			முழு obj_x = x + padding_இடது;
			nserror res;

			rect.x0 = x + padding_இடது;
			rect.y0 = y + padding_உச்சி;
			rect.x1 = x + padding_இடது + width - 1;
			rect.y1 = y + padding_உச்சி + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			எனில் (res != NSERROR_OK) {
				திரும்பு false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  அளவு(obj) - 1,
						  &obj_width);
			எனில் (res != NSERROR_OK) {
				obj_x += 1;
			} இல்லையெனில் {
				obj_x += width / 2 - obj_width / 2;
			}

			எனில் (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_உச்சி + (முழு)(height * 0.75),
					    obj, அளவு(obj) - 1) != NSERROR_OK)
				திரும்பு false;
		}
	} இல்லையெனில் எனில் (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->முனை != வெற்று &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		கட்டமைப்பு bitmap *bitmap = வெற்று;
		exc = dom_முனை_get_user_data(box->முனை,
					     corestring_dom_ns_சாவி_canvas_முனை_data,
					     &bitmap);
		எனில் (exc != DOM_NO_ERR) {
			bitmap = வெற்று;
		}
		எனில் (bitmap != வெற்று &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_இடது, y + padding_உச்சி,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			திரும்பு false;
	} இல்லையெனில் எனில் (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_இடது,
				y + padding_உச்சி, &r, ctx);

	} இல்லையெனில் எனில் (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		எனில் (!html_redraw_checkbox(x + padding_இடது, y + padding_உச்சி,
				width, height, box->gadget->selected, ctx))
			திரும்பு false;

	} இல்லையெனில் எனில் (box->gadget && box->gadget->type == GADGET_RADIO) {
		எனில் (!html_redraw_radio(x + padding_இடது, y + padding_உச்சி,
				width, height, box->gadget->selected, ctx))
			திரும்பு false;

	} இல்லையெனில் எனில் (box->gadget && box->gadget->type == GADGET_FILE) {
		எனில் (!html_redraw_file(x + padding_இடது, y + padding_உச்சி,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			திரும்பு false;

	} இல்லையெனில் எனில் (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} இல்லையெனில் எனில் (box->text) {
		எனில் (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			திரும்பு false;

	} இல்லையெனில் {
		எனில் (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			திரும்பு false;
	}

	எனில் (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		எனில் (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			திரும்பு false;

	/* list marker */
	எனில் (box->list_marker) {
		எனில் (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			திரும்பு false;
	}

	/* scrollbars */
	எனில் (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == வெற்று || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != வெற்று) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((கட்டமைப்பு content *)html,
					    box, has_x_scroll, has_y_scroll);
		எனில் (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			திரும்பு false;
		}

		எனில் (box->scroll_x != வெற்று)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[உச்சி_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		எனில் (box->scroll_y != வெற்று)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[இடது_2] +
					box->width + box->padding[வலது_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	எனில் (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		எனில் (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			திரும்பு false;
	}

	திரும்பு ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(கட்டமைப்பு content *c, கட்டமைப்பு content_redraw_data *data,
		மாறிலி கட்டமைப்பு rect *clip, மாறிலி கட்டமைப்பு redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	கட்டமைப்பு box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	ப்ரதிஜ்ஞா(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	எனில் (ctx->interactive && html->visible_select_menu != வெற்று) {
		கட்டமைப்பு form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	எனில் (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		எனில் (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	எனில் (select) {
		முழு menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[இடது_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[உச்சி_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	திரும்பு result;

}
