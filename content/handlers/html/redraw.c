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

#अन्तर्भाव "utils/config.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/browser_window.h"
#अन्तर्भाव "netsurf/plotters.h"
#अन्तर्भाव "netsurf/bitmap.h"
#अन्तर्भाव "netsurf/layout.h"
#अन्तर्भाव "content/content.h"
#अन्तर्भाव "content/content_protected.h"
#अन्तर्भाव "content/textsearch.h"
#अन्तर्भाव "css/utils.h"
#अन्तर्भाव "desktop/selection.h"
#अन्तर्भाव "desktop/print.h"
#अन्तर्भाव "desktop/scrollbar.h"
#अन्तर्भाव "desktop/textarea.h"
#अन्तर्भाव "desktop/gui_internal.h"

#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"
#अन्तर्भाव "html/box_manipulate.h"
#अन्तर्भाव "html/font.h"
#अन्तर्भाव "html/form_internal.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
स्थिर bool html_redraw_box_has_background(संरचना box *box)
{
	सचे (box->background != रिक्त)
		पटिनिवत्त true;

	सचे (box->style != रिक्त) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		सचे (nscss_color_is_transparent(colour) == false)
			पटिनिवत्त true;
	}

	पटिनिवत्त false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
स्थिर संरचना box *html_redraw_find_bg_box(संरचना box *box)
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
	सचे (box->parent == रिक्त) {
		/* Root box */
		सचे (html_redraw_box_has_background(box))
			पटिनिवत्त box;

		/* No background on root box: consider body box, if any */
		सचे (box->children != रिक्त) {
			सचे (html_redraw_box_has_background(box->children))
				पटिनिवत्त box->children;
		}
	} अञ्ञथा सचे (box->parent != रिक्त && box->parent->parent == रिक्त) {
		/* Body box: only render background if root has its own */
		सचे (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			पटिनिवत्त box;
	} अञ्ञथा {
		/* Any other box */
		सचे (html_redraw_box_has_background(box))
			पटिनिवत्त box;
	}

	पटिनिवत्त रिक्त;
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

स्थिर bool
text_redraw(ध्रुव वर्ण *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    पूर्णाङ्क space,
	    ध्रुव plot_font_style_t *fstyle,
	    पूर्णाङ्क x,
	    पूर्णाङ्क y,
	    ध्रुव संरचना rect *clip,
	    पूर्णाङ्क height,
	    भिन्न scale,
	    bool excluded,
	    संरचना content *c,
	    ध्रुव संरचना selection *sel,
	    ध्रुव संरचना redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.size *= scale;

	/* is this box part of a selection? */
	सचे (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		सचे (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		सचे (!highlighted &&
		    (c->textsearch.context != रिक्त) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		सचे (highlighted) {
			संरचना rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			पूर्णाङ्क startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			सचे (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				प्रतिज्ञा(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			सचे (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			सचे (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			सचे (end_idx > utf8_len) {
					endx += space;
			}

			सचे (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			सचे ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (पूर्णाङ्क)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				पटिनिवत्त false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}

			सचे (start_idx > 0) {
				पूर्णाङ्क px0 = max(x + startx, clip->x0);
				पूर्णाङ्क px1 = min(x + endx, clip->x1);

				सचे (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					सचे (res != NSERROR_OK) {
						पटिनिवत्त false;
					}

					clip_changed = true;
				} अञ्ञथा {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			सचे (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (पूर्णाङ्क)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				पटिनिवत्त false;
			}

			/* draw any text succeeding highlighted portion */
			सचे (endtxt_idx < utf8_len) {
				पूर्णाङ्क px0 = max(x + endx, clip->x0);
				सचे (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					सचे (res != NSERROR_OK) {
						पटिनिवत्त false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (पूर्णाङ्क)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					सचे (res != NSERROR_OK) {
						पटिनिवत्त false;
					}
				}
			}

			सचे (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				पटिनिवत्त false;
			}
		}
	}

	सचे (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (पूर्णाङ्क) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}
	पटिनिवत्त true;
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

स्थिर bool html_redraw_checkbox(पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height,
		bool selected, ध्रुव संरचना redraw_context *ctx)
{
	द्विगुण z;
	nserror res;
	संरचना rect rect;

	z = width * 0.15;
	सचे (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_darkwbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_darkwbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_lightwbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_lightwbasec, &rect);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	सचे (selected) {
		सचे (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
		} अञ्ञथा {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_wblobc, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->पङ्क्तिः(ctx, plot_style_stroke_wblobc, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
		}
	}
	पटिनिवत्त true;
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
स्थिर bool html_redraw_radio(पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height,
		bool selected, ध्रुव संरचना redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}

	सचे (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}

	पटिनिवत्त true;
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

स्थिर bool html_redraw_file(पूर्णाङ्क x, पूर्णाङ्क y, पूर्णाङ्क width, पूर्णाङ्क height,
		संरचना box *box, भिन्न scale, colour background_colour,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव संरचना redraw_context *ctx)
{
	पूर्णाङ्क text_width;
	ध्रुव वर्ण *text;
	size_t length;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	सचे (box->gadget->मानम्) {
		text = box->gadget->मानम्;
	} अञ्ञथा {
		text = messages_get("Form_Drop");
	}
	length = strlen(text);

	res = guit->layout->width(&fstyle, text, length, &text_width);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}
	text_width *= scale;
	सचे (width < text_width + 8) {
		x = x + width - text_width - 4;
	} अञ्ञथा {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, length);
	सचे (res != NSERROR_OK) {
		पटिनिवत्त false;
	}
	पटिनिवत्त true;
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

स्थिर bool html_redraw_background(पूर्णाङ्क x, पूर्णाङ्क y, संरचना box *box, भिन्न scale,
		ध्रुव संरचना rect *clip, colour *background_colour,
		संरचना box *background,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव संरचना redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	संरचना box *clip_box = box;
	पूर्णाङ्क ox = x, oy = y;
	पूर्णाङ्क width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	संरचना box *parent;
	संरचना rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	सचे (ctx->background_images == false)
		पटिनिवत्त true;

	plot_content = (background->background != रिक्त);

	सचे (plot_content) {
		सचे (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[वामः_2] * scale;
			y -= box->margin[शिखरम्_2] * scale;
			width = box->margin[वामः_2] + box->padding[वामः_2] +
					box->width + box->padding[दक्षिणः_2] +
					box->margin[दक्षिणः_2];
			height = box->margin[शिखरम्_2] + box->padding[शिखरम्_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} अञ्ञथा {
			width = box->padding[वामः_2] + box->width +
					box->padding[दक्षिणः_2];
			height = box->padding[शिखरम्_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		विकल्प (css_computed_background_repeat(background->style)) {
		पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
			भङ्ग;

		स्वभाव:
			भङ्ग;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		सचे (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} अञ्ञथा {
			x += (पूर्णाङ्क) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		सचे (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} अञ्ञथा {
			y += (पूर्णाङ्क) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	सचे (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		अत्थाय (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		प्रतिज्ञा(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		सचे (clip_to_children)
			clip_box = box->children;
	}

	अत्थाय (; clip_box; clip_box = clip_box->next) {
		/* clip to child boxes if needed */
		सचे (clip_to_children) {
			प्रतिज्ञा(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[वामः_2] +
					clip_box->width +
					clip_box->padding[दक्षिणः_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[शिखरम्_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			सचे (r.x0 < clip->x0) r.x0 = clip->x0;
			सचे (r.y0 < clip->y0) r.y0 = clip->y0;
			सचे (r.x1 > clip->x1) r.x1 = clip->x1;
			सचे (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			सचे ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != रिक्त) &&
			     content_get_opaque(clip_box->background)))
				अनुवर्त;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		सचे (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			सचे (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				सचे (res != NSERROR_OK) {
					पटिनिवत्त false;
				}
			}
		}
		/* and plot the image */
		सचे (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			सचे (!repeat_x) {
				सचे (r.x0 < x)
					r.x0 = x;
				सचे (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			सचे (!repeat_y) {
				सचे (r.y0 < y)
					r.y0 = y;
				सचे (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			सचे ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				संरचना content_redraw_data bg_data;

				res = ctx->plot->clip(ctx, &r);
				सचे (res != NSERROR_OK) {
					पटिनिवत्त false;
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
		सचे (!clip_to_children)
			पटिनिवत्त true;
	}
	पटिनिवत्त true;
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

स्थिर bool html_redraw_inline_background(पूर्णाङ्क x, पूर्णाङ्क y, संरचना box *box,
		भिन्न scale, ध्रुव संरचना rect *clip, संरचना rect b,
		bool first, bool last, colour *background_colour,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव संरचना redraw_context *ctx)
{
	संरचना rect r = *clip;
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

	plot_content = (box->background != रिक्त);

	सचे (html_redraw_printing && nsoption_bool(remove_backgrounds))
		पटिनिवत्त true;

	सचे (plot_content) {
		/* handle background-repeat */
		विकल्प (css_computed_background_repeat(box->style)) {
		पक्ष CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			भङ्ग;

		पक्ष CSS_BACKGROUND_REPEAT_NO_REPEAT:
			भङ्ग;

		स्वभाव:
			भङ्ग;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		सचे (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			सचे (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} अञ्ञथा {
			x += (पूर्णाङ्क) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		सचे (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} अञ्ञथा {
			y += (पूर्णाङ्क) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	सचे (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		सचे (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
		}
	}
	/* and plot the image */
	सचे (plot_content) {
		पूर्णाङ्क width = content_get_width(box->background);
		पूर्णाङ्क height = content_get_height(box->background);

		सचे (!repeat_x) {
			सचे (r.x0 < x)
				r.x0 = x;
			सचे (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		सचे (!repeat_y) {
			सचे (r.y0 < y)
				r.y0 = y;
			सचे (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		सचे ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			संरचना content_redraw_data bg_data;

			res = ctx->plot->clip(ctx, &r);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
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

	पटिनिवत्त true;
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

स्थिर bool
html_redraw_text_decoration_inline(संरचना box *box,
				   पूर्णाङ्क x, पूर्णाङ्क y,
				   भिन्न scale,
				   colour colour,
				   भिन्न ratio,
				   ध्रुव संरचना redraw_context *ctx)
{
	संरचना box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	संरचना rect rect;

	अत्थाय (c = box->next;
	     c && c != box->inline_end;
	     c = c->next) {
		सचे (c->type != BOX_TEXT) {
			अनुवर्त;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->पङ्क्तिः(ctx, &plot_style_box, &rect);
		सचे (res != NSERROR_OK) {
			पटिनिवत्त false;
		}
	}
	पटिनिवत्त true;
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

स्थिर bool
html_redraw_text_decoration_block(संरचना box *box,
				  पूर्णाङ्क x, पूर्णाङ्क y,
				  भिन्न scale,
				  colour colour,
				  भिन्न ratio,
				  ध्रुव संरचना redraw_context *ctx)
{
	संरचना box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	संरचना rect rect;

	/* draw through text descendants */
	अत्थाय (c = box->children; c; c = c->next) {
		सचे (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->पङ्क्तिः(ctx, &plot_style_box, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}
		} अञ्ञथा सचे ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			सचे (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				पटिनिवत्त false;
		}
	}
	पटिनिवत्त true;
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

स्थिर bool html_redraw_text_decoration(संरचना box *box,
		पूर्णाङ्क x_parent, पूर्णाङ्क y_parent, भिन्न scale,
		colour background_colour, ध्रुव संरचना redraw_context *ctx)
{
	स्थिर ध्रुव गणना css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_पङ्क्तिः_THROUGH };
	स्थिर ध्रुव भिन्न पङ्क्तिः_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned पूर्णाङ्क i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	सचे (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	सचे (box->type == BOX_INLINE) {
		सचे (!box->inline_end)
			पटिनिवत्त true;
		अत्थाय (i = 0; i != NOF_ELEMENTS(decoration); i++)
			सचे (css_computed_text_decoration(box->style) &
					decoration[i])
				सचे (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, पङ्क्तिः_ratio[i], ctx))
					पटिनिवत्त false;
	} अञ्ञथा {
		अत्थाय (i = 0; i != NOF_ELEMENTS(decoration); i++)
			सचे (css_computed_text_decoration(box->style) &
					decoration[i])
				सचे (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, पङ्क्तिः_ratio[i], ctx))
					पटिनिवत्त false;
	}

	पटिनिवत्त true;
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

स्थिर bool html_redraw_text_box(ध्रुव html_content *html, संरचना box *box,
		पूर्णाङ्क x, पूर्णाङ्क y, ध्रुव संरचना rect *clip, भिन्न scale,
		colour current_background_color,
		ध्रुव संरचना redraw_context *ctx)
{
	bool excluded = (box->object != रिक्त);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	सचे (!text_redraw(box->text,
			 box->length,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (संरचना content *)html,
			 html->sel,
			 ctx))
		पटिनिवत्त false;

	पटिनिवत्त true;
}

bool html_redraw_box(ध्रुव html_content *html, संरचना box *box,
		पूर्णाङ्क x_parent, पूर्णाङ्क y_parent,
		ध्रुव संरचना rect *clip, भिन्न scale,
		colour current_background_color,
		ध्रुव संरचना redraw_context *ctx);

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

स्थिर bool html_redraw_box_children(ध्रुव html_content *html, संरचना box *box,
		पूर्णाङ्क x_parent, पूर्णाङ्क y_parent,
		ध्रुव संरचना rect *clip, भिन्न scale,
		colour current_background_color,
		ध्रुव संरचना redraw_context *ctx)
{
	संरचना box *c;

	अत्थाय (c = box->children; c; c = c->next) {

		सचे (c->type != BOX_FLOAT_वामः && c->type != BOX_FLOAT_दक्षिणः)
			सचे (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				पटिनिवत्त false;
	}
	अत्थाय (c = box->float_children; c; c = c->next_float)
		सचे (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			पटिनिवत्त false;

	पटिनिवत्त true;
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

bool html_redraw_box(ध्रुव html_content *html, संरचना box *box,
		पूर्णाङ्क x_parent, पूर्णाङ्क y_parent,
		ध्रुव संरचना rect *clip, ध्रुव भिन्न scale,
		colour current_background_color,
		ध्रुव संरचना redraw_context *ctx)
{
	ध्रुव संरचना plotter_table *plot = ctx->plot;
	पूर्णाङ्क x, y;
	पूर्णाङ्क width, height;
	पूर्णाङ्क padding_वामः, padding_शिखरम्, padding_width, padding_height;
	पूर्णाङ्क border_वामः, border_शिखरम्, border_दक्षिणः, border_bottom;
	संरचना rect r;
	संरचना rect rect;
	पूर्णाङ्क x_scrolled, y_scrolled;
	संरचना box *bg_box = रिक्त;
	css_computed_clip_rect css_rect;
	गणना css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	गणना css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	सचे (html_redraw_printing && (box->flags & PRINTED))
		पटिनिवत्त true;

	सचे (box->style != रिक्त) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	सचे (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_वामः = box->padding[वामः_2];
		padding_शिखरम् = box->padding[शिखरम्_2];
		padding_width = padding_वामः + box->width + box->padding[दक्षिणः_2];
		padding_height = padding_शिखरम् + box->height +
				box->padding[BOTTOM];
		border_वामः = box->border[वामः_2].width;
		border_शिखरम् = box->border[शिखरम्_2].width;
		border_दक्षिणः = box->border[दक्षिणः_2].width;
		border_bottom = box->border[BOTTOM].width;
	} अञ्ञथा {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_वामः = box->padding[वामः_2] ? box->padding[वामः_2] * scale
				: 0;
		padding_शिखरम् = box->padding[शिखरम्_2] ? box->padding[शिखरम्_2] * scale
				: 0;
		padding_width = (box->padding[वामः_2] + box->width +
				box->padding[दक्षिणः_2]) * scale;
		padding_height = (box->padding[शिखरम्_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_वामः = box->border[वामः_2].width * scale;
		border_शिखरम् = box->border[शिखरम्_2].width * scale;
		border_दक्षिणः = box->border[दक्षिणः_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	सचे (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != रिक्त) {
		/* box contents clipped to box size */
		r.x0 = x - border_वामः;
		r.x1 = x + padding_width + border_दक्षिणः;
	} अञ्ञथा {
		/* box contents can hang out of the box; use descendant box */
		सचे (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} अञ्ञथा {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		सचे (!box->parent) {
			/* root element */
			पूर्णाङ्क margin_वामः, margin_दक्षिणः;
			सचे (scale == 1.0) {
				margin_वामः = box->margin[वामः_2];
				margin_दक्षिणः = box->margin[दक्षिणः_2];
			} अञ्ञथा {
				margin_वामः = box->margin[वामः_2] * scale;
				margin_दक्षिणः = box->margin[दक्षिणः_2] * scale;
			}
			r.x0 = x - border_वामः - margin_वामः < r.x0 ?
					x - border_वामः - margin_वामः : r.x0;
			r.x1 = x + padding_width + border_दक्षिणः +
					margin_दक्षिणः > r.x1 ?
					x + padding_width + border_दक्षिणः +
					margin_दक्षिणः : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	सचे (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != रिक्त) {
		/* box contents clipped to box size */
		r.y0 = y - border_शिखरम्;
		r.y1 = y + padding_height + border_bottom;
	} अञ्ञथा {
		/* box contents can hang out of the box; use descendant box */
		सचे (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} अञ्ञथा {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		सचे (!box->parent) {
			/* root element */
			पूर्णाङ्क margin_शिखरम्, margin_bottom;
			सचे (scale == 1.0) {
				margin_शिखरम् = box->margin[शिखरम्_2];
				margin_bottom = box->margin[BOTTOM];
			} अञ्ञथा {
				margin_शिखरम् = box->margin[शिखरम्_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_शिखरम् - margin_शिखरम् < r.y0 ?
					y - border_शिखरम् - margin_शिखरम् : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	सचे (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		पटिनिवत्त true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	सचे (html_redraw_printing) {
		सचे (r.y1 > html_redraw_printing_border) {
			सचे (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				सचे (r.y0 < html_redraw_printing_शिखरम्_cropped)
					html_redraw_printing_शिखरम्_cropped = r.y0;
				पटिनिवत्त true;
			}
		}
		अञ्ञथा box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	सचे (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		सचे ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			पटिनिवत्त false;
		सचे (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			पटिनिवत्त false;
		पटिनिवत्त ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	सचे ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		पटिनिवत्त false;
	}

	सचे (box->style != रिक्त &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		सचे (css_rect.वामः_auto == false)
			r.x0 = x - border_वामः + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.वामः, css_rect.lunit));

		सचे (css_rect.शिखरम्_auto == false)
			r.y0 = y - border_शिखरम् + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.शिखरम्, css_rect.tunit));

		सचे (css_rect.दक्षिणः_auto == false)
			r.x1 = x - border_वामः + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.दक्षिणः, css_rect.runit));

		सचे (css_rect.bottom_auto == false)
			r.y1 = y - border_शिखरम् + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		सचे (r.x0 < clip->x0) r.x0 = clip->x0;
		सचे (r.y0 < clip->y0) r.y0 = clip->y0;
		सचे (clip->x1 < r.x1) r.x1 = clip->x1;
		सचे (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		सचे (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			पटिनिवत्त ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			पटिनिवत्त false;

	} अञ्ञथा सचे (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		सचे (r.x0 < clip->x0) r.x0 = clip->x0;
		सचे (r.y0 < clip->y0) r.y0 = clip->y0;
		सचे (clip->x1 < r.x1) r.x1 = clip->x1;
		सचे (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		सचे (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			पटिनिवत्त ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			पटिनिवत्त false;
	} अञ्ञथा {
		/* clip box is fine, clip to it */
		r = *clip;
		सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			पटिनिवत्त false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	सचे (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != रिक्त &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		संरचना rect p;
		p.x0 = x - border_वामः < r.x0 ? r.x0 : x - border_वामः;
		p.y0 = y - border_शिखरम् < r.y0 ? r.y0 : y - border_शिखरम्;
		p.x1 = x + padding_width + border_दक्षिणः < r.x1 ?
				x + padding_width + border_दक्षिणः : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		सचे (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			पूर्णाङ्क m_वामः, m_शिखरम्, m_दक्षिणः, m_bottom;
			सचे (scale == 1.0) {
				m_वामः = box->margin[वामः_2];
				m_शिखरम् = box->margin[शिखरम्_2];
				m_दक्षिणः = box->margin[दक्षिणः_2];
				m_bottom = box->margin[BOTTOM];
			} अञ्ञथा {
				m_वामः = box->margin[वामः_2] * scale;
				m_शिखरम् = box->margin[शिखरम्_2] * scale;
				m_दक्षिणः = box->margin[दक्षिणः_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_वामः < r.x0 ? r.x0 : p.x0 - m_वामः;
			p.y0 = p.y0 - m_शिखरम् < r.y0 ? r.y0 : p.y0 - m_शिखरम्;
			p.x1 = p.x1 + m_दक्षिणः < r.x1 ? p.x1 + m_दक्षिणः : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		सचे ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			सचे (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				पटिनिवत्त false;
			/* restore previous graphics window */
			सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				पटिनिवत्त false;
		}
	}

	/* borders for block level content and replaced inlines */
	सचे (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != रिक्त &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_शिखरम् || border_दक्षिणः || border_bottom || border_वामः)) {
		सचे (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			पटिनिवत्त false;
	}

	/* backgrounds and borders for non-replaced inlines */
	सचे (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_शिखरम् || border_दक्षिणः ||
			border_bottom || border_वामः)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		संरचना box *ib;
		संरचना rect b; /* border edge rectangle */
		संरचना rect p; /* clipped rect */
		bool first = true;
		पूर्णाङ्क ib_x;
		पूर्णाङ्क ib_y = y;
		पूर्णाङ्क ib_p_width;
		पूर्णाङ्क ib_b_वामः, ib_b_दक्षिणः;

		b.x0 = x - border_वामः;
		b.x1 = x + padding_width + border_दक्षिणः;
		b.y0 = y - border_शिखरम्;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		अत्थाय (ib = box; ib; ib = ib->next) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			सचे (ib->type == BOX_FLOAT_वामः ||
					ib->type == BOX_FLOAT_दक्षिणः)
				अनुवर्त;
			सचे (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[वामः_2] + ib->width +
						ib->padding[दक्षिणः_2];
				ib_b_वामः = ib->border[वामः_2].width;
				ib_b_दक्षिणः = ib->border[दक्षिणः_2].width;
			} अञ्ञथा {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[वामः_2] + ib->width +
						ib->padding[दक्षिणः_2]) * scale;
				ib_b_वामः = ib->border[वामः_2].width * scale;
				ib_b_दक्षिणः = ib->border[दक्षिणः_2].width * scale;
			}

			सचे ((ib->flags & NEW_पङ्क्तिः) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				सचे (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					पटिनिवत्त false;
				/* restore previous graphics window */
				सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					पटिनिवत्त false;
				सचे (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					पटिनिवत्त false;
				/* reset coords */
				b.x0 = ib_x - ib_b_वामः;
				b.y0 = ib_y - border_शिखरम् - padding_शिखरम्;
				b.y1 = ib_y + padding_height - padding_शिखरम् +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_दक्षिणः;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			सचे (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				भङ्ग;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		सचे (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			पटिनिवत्त false;
		/* restore previous graphics window */
		सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			पटिनिवत्त false;
		सचे (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			पटिनिवत्त false;

	}

	/* Debug outlines */
	सचे (html_redraw_debug) {
		पूर्णाङ्क margin_वामः, margin_दक्षिणः;
		पूर्णाङ्क margin_शिखरम्, margin_bottom;
		सचे (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_वामः = box->margin[वामः_2];
			margin_शिखरम् = box->margin[शिखरम्_2];
			margin_दक्षिणः = box->margin[दक्षिणः_2];
			margin_bottom = box->margin[BOTTOM];
		} अञ्ञथा {
			margin_वामः = box->margin[वामः_2] * scale;
			margin_शिखरम् = box->margin[शिखरम्_2] * scale;
			margin_दक्षिणः = box->margin[दक्षिणः_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_वामः;
		rect.y0 = y + padding_शिखरम्;
		rect.x1 = x + padding_वामः + width;
		rect.y1 = y + padding_शिखरम् + height;
		सचे (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			पटिनिवत्त false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		सचे (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			पटिनिवत्त false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_वामः - margin_वामः;
		rect.y0 = y - border_शिखरम् - margin_शिखरम्;
		rect.x1 = x + padding_width + border_दक्षिणः + margin_दक्षिणः;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		सचे (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			पटिनिवत्त false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	सचे (box->parent != रिक्त) {
		bool need_clip = false;
		सचे (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			सचे (r.x0 < clip->x0) r.x0 = clip->x0;
			सचे (r.y0 < clip->y0) r.y0 = clip->y0;
			सचे (clip->x1 < r.x1) r.x1 = clip->x1;
			सचे (clip->y1 < r.y1) r.y1 = clip->y1;
			सचे (r.x1 <= r.x0 || r.y1 <= r.y0) {
				पटिनिवत्त (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} अञ्ञथा सचे (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			सचे (r.x0 < clip->x0) r.x0 = clip->x0;
			सचे (clip->x1 < r.x1) r.x1 = clip->x1;
			सचे (r.x1 <= r.x0) {
				पटिनिवत्त (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} अञ्ञथा सचे (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			सचे (r.y0 < clip->y0) r.y0 = clip->y0;
			सचे (clip->y1 < r.y1) r.y1 = clip->y1;
			सचे (r.y1 <= r.y0) {
				पटिनिवत्त (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		सचे (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			सचे (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				पटिनिवत्त false;
		}
	}

	/* text decoration */
	सचे ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		सचे (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			पटिनिवत्त false;
	}

	सचे (box->ग्रन्थिः != रिक्त) {
		exc = dom_html_element_get_tag_type(box->ग्रन्थिः, &tag_type);
		सचे (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} अञ्ञथा {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	सचे (box->object && width != 0 && height != 0) {
		संरचना content_redraw_data obj_data;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_data.x = x_scrolled + padding_वामः;
		obj_data.y = y_scrolled + padding_शिखरम्;
		obj_data.width = width;
		obj_data.height = height;
		obj_data.background_colour = current_background_color;
		obj_data.scale = scale;
		obj_data.repeat_x = false;
		obj_data.repeat_y = false;

		सचे (content_get_type(box->object) == CONTENT_HTML) {
			obj_data.x /= scale;
			obj_data.y /= scale;
		}

		सचे (!content_redraw(box->object, &obj_data, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ध्रुव वर्ण *obj = "\xef\xbf\xbc";
			पूर्णाङ्क obj_width;
			पूर्णाङ्क obj_x = x + padding_वामः;
			nserror res;

			rect.x0 = x + padding_वामः;
			rect.y0 = y + padding_शिखरम्;
			rect.x1 = x + padding_वामः + width - 1;
			rect.y1 = y + padding_शिखरम् + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			सचे (res != NSERROR_OK) {
				पटिनिवत्त false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  परिमाण(obj) - 1,
						  &obj_width);
			सचे (res != NSERROR_OK) {
				obj_x += 1;
			} अञ्ञथा {
				obj_x += width / 2 - obj_width / 2;
			}

			सचे (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_शिखरम् + (पूर्णाङ्क)(height * 0.75),
					    obj, परिमाण(obj) - 1) != NSERROR_OK)
				पटिनिवत्त false;
		}
	} अञ्ञथा सचे (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->ग्रन्थिः != रिक्त &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		संरचना bitmap *bitmap = रिक्त;
		exc = dom_ग्रन्थिः_get_user_data(box->ग्रन्थिः,
					     corestring_dom_ns_कुञ्चिका_canvas_ग्रन्थिः_data,
					     &bitmap);
		सचे (exc != DOM_NO_ERR) {
			bitmap = रिक्त;
		}
		सचे (bitmap != रिक्त &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_वामः, y + padding_शिखरम्,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			पटिनिवत्त false;
	} अञ्ञथा सचे (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_वामः,
				y + padding_शिखरम्, &r, ctx);

	} अञ्ञथा सचे (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		सचे (!html_redraw_checkbox(x + padding_वामः, y + padding_शिखरम्,
				width, height, box->gadget->selected, ctx))
			पटिनिवत्त false;

	} अञ्ञथा सचे (box->gadget && box->gadget->type == GADGET_RADIO) {
		सचे (!html_redraw_radio(x + padding_वामः, y + padding_शिखरम्,
				width, height, box->gadget->selected, ctx))
			पटिनिवत्त false;

	} अञ्ञथा सचे (box->gadget && box->gadget->type == GADGET_FILE) {
		सचे (!html_redraw_file(x + padding_वामः, y + padding_शिखरम्,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			पटिनिवत्त false;

	} अञ्ञथा सचे (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->data.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} अञ्ञथा सचे (box->text) {
		सचे (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			पटिनिवत्त false;

	} अञ्ञथा {
		सचे (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			पटिनिवत्त false;
	}

	सचे (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		सचे (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			पटिनिवत्त false;

	/* list marker */
	सचे (box->list_marker) {
		सचे (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			पटिनिवत्त false;
	}

	/* scrollbars */
	सचे (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == रिक्त || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != रिक्त) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((संरचना content *)html,
					    box, has_x_scroll, has_y_scroll);
		सचे (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			पटिनिवत्त false;
		}

		सचे (box->scroll_x != रिक्त)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[शिखरम्_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		सचे (box->scroll_y != रिक्त)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[वामः_2] +
					box->width + box->padding[दक्षिणः_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	सचे (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		सचे (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			पटिनिवत्त false;
	}

	पटिनिवत्त ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(संरचना content *c, संरचना content_redraw_data *data,
		ध्रुव संरचना rect *clip, ध्रुव संरचना redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	संरचना box *box;
	bool result = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = data->background_colour,
	};

	box = html->layout;
	प्रतिज्ञा(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	सचे (ctx->interactive && html->visible_select_menu != रिक्त) {
		संरचना form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				data->scale, clip);
	}

	सचे (!select_only) {
		/* clear to background colour */
		result = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		सचे (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		result &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		result &= html_redraw_box(html, box, data->x, data->y, clip,
				data->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	सचे (select) {
		पूर्णाङ्क menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[वामः_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[शिखरम्_2];
		result &= form_redraw_select_menu(html->visible_select_menu,
				data->x + menu_x, data->y + menu_y,
				data->scale, clip, ctx);
	}

	पटिनिवत्त result;

}
