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

#समावेश "utils/config.h"
#समावेश <assert.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <math.h>
#समावेश <dom/dom.h>

#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/utils.h"
#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/browser_window.h"
#समावेश "netsurf/plotters.h"
#समावेश "netsurf/bitmap.h"
#समावेश "netsurf/layout.h"
#समावेश "content/content.h"
#समावेश "content/content_protected.h"
#समावेश "content/textsearch.h"
#समावेश "css/utils.h"
#समावेश "desktop/selection.h"
#समावेश "desktop/print.h"
#समावेश "desktop/scrollbar.h"
#समावेश "desktop/textarea.h"
#समावेश "desktop/gui_internal.h"

#समावेश "html/box.h"
#समावेश "html/box_inspect.h"
#समावेश "html/box_manipulate.h"
#समावेश "html/font.h"
#समावेश "html/form_internal.h"
#समावेश "html/private.h"
#समावेश "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
अटल bool html_redraw_box_has_background(काष्ठा box *box)
{
	अगर (box->background != शून्य)
		वापस true;

	अगर (box->style != शून्य) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		अगर (nscss_color_is_transparent(colour) == false)
			वापस true;
	}

	वापस false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
अटल काष्ठा box *html_redraw_find_bg_box(काष्ठा box *box)
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
	अगर (box->parent == शून्य) {
		/* Root box */
		अगर (html_redraw_box_has_background(box))
			वापस box;

		/* No background on root box: consider body box, if any */
		अगर (box->children != शून्य) {
			अगर (html_redraw_box_has_background(box->children))
				वापस box->children;
		}
	} अन्यथा अगर (box->parent != शून्य && box->parent->parent == शून्य) {
		/* Body box: only render background if root has its own */
		अगर (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			वापस box;
	} अन्यथा {
		/* Any other box */
		अगर (html_redraw_box_has_background(box))
			वापस box;
	}

	वापस शून्य;
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

अटल bool
text_redraw(स्थिर अक्षर *utf8_text,
	    माप_प्रकार utf8_len,
	    माप_प्रकार offset,
	    पूर्णांक space,
	    स्थिर plot_font_style_t *fstyle,
	    पूर्णांक x,
	    पूर्णांक y,
	    स्थिर काष्ठा rect *clip,
	    पूर्णांक height,
	    भग्न scale,
	    bool excluded,
	    काष्ठा content *c,
	    स्थिर काष्ठा selection *sel,
	    स्थिर काष्ठा redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.आकार *= scale;

	/* is this box part of a selection? */
	अगर (!excluded && ctx->interactive == true) {
		अचिन्हित len = utf8_len + (space ? 1 : 0);
		अचिन्हित start_idx;
		अचिन्हित end_idx;

		/* first try the browser window's current selection */
		अगर (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		अगर (!highlighted &&
		    (c->textsearch.context != शून्य) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		अगर (highlighted) {
			काष्ठा rect r;
			अचिन्हित endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			पूर्णांक startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			अगर (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				निश्चित(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			अगर (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			अगर (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			अगर (end_idx > utf8_len) {
					endx += space;
			}

			अगर (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			अगर ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (पूर्णांक)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				वापस false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			अगर (res != NSERROR_OK) {
				वापस false;
			}

			अगर (start_idx > 0) {
				पूर्णांक px0 = max(x + startx, clip->x0);
				पूर्णांक px1 = min(x + endx, clip->x1);

				अगर (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					अगर (res != NSERROR_OK) {
						वापस false;
					}

					clip_changed = true;
				} अन्यथा {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			अगर (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (पूर्णांक)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				वापस false;
			}

			/* draw any text succeeding highlighted portion */
			अगर (endtxt_idx < utf8_len) {
				पूर्णांक px0 = max(x + endx, clip->x0);
				अगर (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					अगर (res != NSERROR_OK) {
						वापस false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (पूर्णांक)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					अगर (res != NSERROR_OK) {
						वापस false;
					}
				}
			}

			अगर (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				वापस false;
			}
		}
	}

	अगर (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (पूर्णांक) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}
	वापस true;
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

अटल bool html_redraw_checkbox(पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height,
		bool selected, स्थिर काष्ठा redraw_context *ctx)
{
	द्विगुन z;
	nserror res;
	काष्ठा rect rect;

	z = width * 0.15;
	अगर (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->पंक्ति(ctx, plot_style_stroke_darkwbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->पंक्ति(ctx, plot_style_stroke_darkwbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->पंक्ति(ctx, plot_style_stroke_lightwbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->पंक्ति(ctx, plot_style_stroke_lightwbasec, &rect);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	अगर (selected) {
		अगर (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}
		} अन्यथा {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->पंक्ति(ctx, plot_style_stroke_wblobc, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->पंक्ति(ctx, plot_style_stroke_wblobc, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}
		}
	}
	वापस true;
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
अटल bool html_redraw_radio(पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height,
		bool selected, स्थिर काष्ठा redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	अगर (res != NSERROR_OK) {
		वापस false;
	}

	अगर (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}

	वापस true;
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

अटल bool html_redraw_file(पूर्णांक x, पूर्णांक y, पूर्णांक width, पूर्णांक height,
		काष्ठा box *box, भग्न scale, colour background_colour,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर काष्ठा redraw_context *ctx)
{
	पूर्णांक text_width;
	स्थिर अक्षर *text;
	माप_प्रकार लंबाई;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	अगर (box->gadget->मूल्य) {
		text = box->gadget->मूल्य;
	} अन्यथा {
		text = messages_get("Form_Drop");
	}
	लंबाई = म_माप(text);

	res = guit->layout->width(&fstyle, text, लंबाई, &text_width);
	अगर (res != NSERROR_OK) {
		वापस false;
	}
	text_width *= scale;
	अगर (width < text_width + 8) {
		x = x + width - text_width - 4;
	} अन्यथा {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, लंबाई);
	अगर (res != NSERROR_OK) {
		वापस false;
	}
	वापस true;
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

अटल bool html_redraw_background(पूर्णांक x, पूर्णांक y, काष्ठा box *box, भग्न scale,
		स्थिर काष्ठा rect *clip, colour *background_colour,
		काष्ठा box *background,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर काष्ठा redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	काष्ठा box *clip_box = box;
	पूर्णांक ox = x, oy = y;
	पूर्णांक width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	काष्ठा box *parent;
	काष्ठा rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	अगर (ctx->background_images == false)
		वापस true;

	plot_content = (background->background != शून्य);

	अगर (plot_content) {
		अगर (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[बायाँ_2] * scale;
			y -= box->margin[शीर्ष_2] * scale;
			width = box->margin[बायाँ_2] + box->padding[बायाँ_2] +
					box->width + box->padding[दायाँ_2] +
					box->margin[दायाँ_2];
			height = box->margin[शीर्ष_2] + box->padding[शीर्ष_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} अन्यथा {
			width = box->padding[बायाँ_2] + box->width +
					box->padding[दायाँ_2];
			height = box->padding[शीर्ष_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		चयन (css_computed_background_repeat(background->style)) {
		हाल CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_NO_REPEAT:
			अवरोध;

		शेष:
			अवरोध;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		अगर (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} अन्यथा {
			x += (पूर्णांक) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		अगर (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} अन्यथा {
			y += (पूर्णांक) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	अगर (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		क्रम (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		निश्चित(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		अगर (clip_to_children)
			clip_box = box->children;
	}

	क्रम (; clip_box; clip_box = clip_box->अग्र) {
		/* clip to child boxes if needed */
		अगर (clip_to_children) {
			निश्चित(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[बायाँ_2] +
					clip_box->width +
					clip_box->padding[दायाँ_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[शीर्ष_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			अगर (r.x0 < clip->x0) r.x0 = clip->x0;
			अगर (r.y0 < clip->y0) r.y0 = clip->y0;
			अगर (r.x1 > clip->x1) r.x1 = clip->x1;
			अगर (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			अगर ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != शून्य) &&
			     content_get_opaque(clip_box->background)))
				जारी;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		अगर (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			अगर (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				अगर (res != NSERROR_OK) {
					वापस false;
				}
			}
		}
		/* and plot the image */
		अगर (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			अगर (!repeat_x) {
				अगर (r.x0 < x)
					r.x0 = x;
				अगर (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			अगर (!repeat_y) {
				अगर (r.y0 < y)
					r.y0 = y;
				अगर (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			अगर ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				काष्ठा content_redraw_तथ्य bg_तथ्य;

				res = ctx->plot->clip(ctx, &r);
				अगर (res != NSERROR_OK) {
					वापस false;
				}

				bg_तथ्य.x = x;
				bg_तथ्य.y = y;
				bg_तथ्य.width = ceilf(width * scale);
				bg_तथ्य.height = ceilf(height * scale);
				bg_तथ्य.background_colour = *background_colour;
				bg_तथ्य.scale = scale;
				bg_तथ्य.repeat_x = repeat_x;
				bg_तथ्य.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_तथ्य, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		अगर (!clip_to_children)
			वापस true;
	}
	वापस true;
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

अटल bool html_redraw_inline_background(पूर्णांक x, पूर्णांक y, काष्ठा box *box,
		भग्न scale, स्थिर काष्ठा rect *clip, काष्ठा rect b,
		bool first, bool last, colour *background_colour,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा rect r = *clip;
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

	plot_content = (box->background != शून्य);

	अगर (html_redraw_printing && nsoption_bool(remove_backgrounds))
		वापस true;

	अगर (plot_content) {
		/* handle background-repeat */
		चयन (css_computed_background_repeat(box->style)) {
		हाल CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			अवरोध;

		हाल CSS_BACKGROUND_REPEAT_NO_REPEAT:
			अवरोध;

		शेष:
			अवरोध;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		अगर (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			अगर (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} अन्यथा {
			x += (पूर्णांक) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		अगर (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} अन्यथा {
			y += (पूर्णांक) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	अगर (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		अगर (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			अगर (res != NSERROR_OK) {
				वापस false;
			}
		}
	}
	/* and plot the image */
	अगर (plot_content) {
		पूर्णांक width = content_get_width(box->background);
		पूर्णांक height = content_get_height(box->background);

		अगर (!repeat_x) {
			अगर (r.x0 < x)
				r.x0 = x;
			अगर (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		अगर (!repeat_y) {
			अगर (r.y0 < y)
				r.y0 = y;
			अगर (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		अगर ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			काष्ठा content_redraw_तथ्य bg_तथ्य;

			res = ctx->plot->clip(ctx, &r);
			अगर (res != NSERROR_OK) {
				वापस false;
			}

			bg_तथ्य.x = x;
			bg_तथ्य.y = y;
			bg_तथ्य.width = ceilf(width * scale);
			bg_तथ्य.height = ceilf(height * scale);
			bg_तथ्य.background_colour = *background_colour;
			bg_तथ्य.scale = scale;
			bg_तथ्य.repeat_x = repeat_x;
			bg_तथ्य.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_तथ्य, &r, ctx);
		}
	}

	वापस true;
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

अटल bool
html_redraw_text_decoration_inline(काष्ठा box *box,
				   पूर्णांक x, पूर्णांक y,
				   भग्न scale,
				   colour colour,
				   भग्न ratio,
				   स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	काष्ठा rect rect;

	क्रम (c = box->अग्र;
	     c && c != box->inline_end;
	     c = c->अग्र) {
		अगर (c->type != BOX_TEXT) {
			जारी;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->पंक्ति(ctx, &plot_style_box, &rect);
		अगर (res != NSERROR_OK) {
			वापस false;
		}
	}
	वापस true;
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

अटल bool
html_redraw_text_decoration_block(काष्ठा box *box,
				  पूर्णांक x, पूर्णांक y,
				  भग्न scale,
				  colour colour,
				  भग्न ratio,
				  स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	काष्ठा rect rect;

	/* draw through text descendants */
	क्रम (c = box->children; c; c = c->अग्र) {
		अगर (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->पंक्ति(ctx, &plot_style_box, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}
		} अन्यथा अगर ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			अगर (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				वापस false;
		}
	}
	वापस true;
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

अटल bool html_redraw_text_decoration(काष्ठा box *box,
		पूर्णांक x_parent, पूर्णांक y_parent, भग्न scale,
		colour background_colour, स्थिर काष्ठा redraw_context *ctx)
{
	अटल स्थिर क्रमागत css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_पंक्ति_THROUGH };
	अटल स्थिर भग्न पंक्ति_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	अचिन्हित पूर्णांक i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	अगर (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	अगर (box->type == BOX_INLINE) {
		अगर (!box->inline_end)
			वापस true;
		क्रम (i = 0; i != NOF_ELEMENTS(decoration); i++)
			अगर (css_computed_text_decoration(box->style) &
					decoration[i])
				अगर (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, पंक्ति_ratio[i], ctx))
					वापस false;
	} अन्यथा {
		क्रम (i = 0; i != NOF_ELEMENTS(decoration); i++)
			अगर (css_computed_text_decoration(box->style) &
					decoration[i])
				अगर (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, पंक्ति_ratio[i], ctx))
					वापस false;
	}

	वापस true;
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

अटल bool html_redraw_text_box(स्थिर html_content *html, काष्ठा box *box,
		पूर्णांक x, पूर्णांक y, स्थिर काष्ठा rect *clip, भग्न scale,
		colour current_background_color,
		स्थिर काष्ठा redraw_context *ctx)
{
	bool excluded = (box->object != शून्य);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	अगर (!text_redraw(box->text,
			 box->लंबाई,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (काष्ठा content *)html,
			 html->sel,
			 ctx))
		वापस false;

	वापस true;
}

bool html_redraw_box(स्थिर html_content *html, काष्ठा box *box,
		पूर्णांक x_parent, पूर्णांक y_parent,
		स्थिर काष्ठा rect *clip, भग्न scale,
		colour current_background_color,
		स्थिर काष्ठा redraw_context *ctx);

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

अटल bool html_redraw_box_children(स्थिर html_content *html, काष्ठा box *box,
		पूर्णांक x_parent, पूर्णांक y_parent,
		स्थिर काष्ठा rect *clip, भग्न scale,
		colour current_background_color,
		स्थिर काष्ठा redraw_context *ctx)
{
	काष्ठा box *c;

	क्रम (c = box->children; c; c = c->अग्र) {

		अगर (c->type != BOX_FLOAT_बायाँ && c->type != BOX_FLOAT_दायाँ)
			अगर (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				वापस false;
	}
	क्रम (c = box->float_children; c; c = c->अग्र_float)
		अगर (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			वापस false;

	वापस true;
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

bool html_redraw_box(स्थिर html_content *html, काष्ठा box *box,
		पूर्णांक x_parent, पूर्णांक y_parent,
		स्थिर काष्ठा rect *clip, स्थिर भग्न scale,
		colour current_background_color,
		स्थिर काष्ठा redraw_context *ctx)
{
	स्थिर काष्ठा plotter_table *plot = ctx->plot;
	पूर्णांक x, y;
	पूर्णांक width, height;
	पूर्णांक padding_बायाँ, padding_शीर्ष, padding_width, padding_height;
	पूर्णांक border_बायाँ, border_शीर्ष, border_दायाँ, border_bottom;
	काष्ठा rect r;
	काष्ठा rect rect;
	पूर्णांक x_scrolled, y_scrolled;
	काष्ठा box *bg_box = शून्य;
	css_computed_clip_rect css_rect;
	क्रमागत css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	क्रमागत css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	अगर (html_redraw_printing && (box->flags & PRINTED))
		वापस true;

	अगर (box->style != शून्य) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	अगर (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_बायाँ = box->padding[बायाँ_2];
		padding_शीर्ष = box->padding[शीर्ष_2];
		padding_width = padding_बायाँ + box->width + box->padding[दायाँ_2];
		padding_height = padding_शीर्ष + box->height +
				box->padding[BOTTOM];
		border_बायाँ = box->border[बायाँ_2].width;
		border_शीर्ष = box->border[शीर्ष_2].width;
		border_दायाँ = box->border[दायाँ_2].width;
		border_bottom = box->border[BOTTOM].width;
	} अन्यथा {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_बायाँ = box->padding[बायाँ_2] ? box->padding[बायाँ_2] * scale
				: 0;
		padding_शीर्ष = box->padding[शीर्ष_2] ? box->padding[शीर्ष_2] * scale
				: 0;
		padding_width = (box->padding[बायाँ_2] + box->width +
				box->padding[दायाँ_2]) * scale;
		padding_height = (box->padding[शीर्ष_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_बायाँ = box->border[बायाँ_2].width * scale;
		border_शीर्ष = box->border[शीर्ष_2].width * scale;
		border_दायाँ = box->border[दायाँ_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	अगर (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != शून्य) {
		/* box contents clipped to box size */
		r.x0 = x - border_बायाँ;
		r.x1 = x + padding_width + border_दायाँ;
	} अन्यथा {
		/* box contents can hang out of the box; use descendant box */
		अगर (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} अन्यथा {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		अगर (!box->parent) {
			/* root element */
			पूर्णांक margin_बायाँ, margin_दायाँ;
			अगर (scale == 1.0) {
				margin_बायाँ = box->margin[बायाँ_2];
				margin_दायाँ = box->margin[दायाँ_2];
			} अन्यथा {
				margin_बायाँ = box->margin[बायाँ_2] * scale;
				margin_दायाँ = box->margin[दायाँ_2] * scale;
			}
			r.x0 = x - border_बायाँ - margin_बायाँ < r.x0 ?
					x - border_बायाँ - margin_बायाँ : r.x0;
			r.x1 = x + padding_width + border_दायाँ +
					margin_दायाँ > r.x1 ?
					x + padding_width + border_दायाँ +
					margin_दायाँ : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	अगर (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != शून्य) {
		/* box contents clipped to box size */
		r.y0 = y - border_शीर्ष;
		r.y1 = y + padding_height + border_bottom;
	} अन्यथा {
		/* box contents can hang out of the box; use descendant box */
		अगर (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} अन्यथा {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		अगर (!box->parent) {
			/* root element */
			पूर्णांक margin_शीर्ष, margin_bottom;
			अगर (scale == 1.0) {
				margin_शीर्ष = box->margin[शीर्ष_2];
				margin_bottom = box->margin[BOTTOM];
			} अन्यथा {
				margin_शीर्ष = box->margin[शीर्ष_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_शीर्ष - margin_शीर्ष < r.y0 ?
					y - border_शीर्ष - margin_शीर्ष : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	अगर (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		वापस true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	अगर (html_redraw_printing) {
		अगर (r.y1 > html_redraw_printing_border) {
			अगर (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				अगर (r.y0 < html_redraw_printing_शीर्ष_cropped)
					html_redraw_printing_शीर्ष_cropped = r.y0;
				वापस true;
			}
		}
		अन्यथा box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	अगर (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		अगर ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			वापस false;
		अगर (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			वापस false;
		वापस ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	अगर ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		वापस false;
	}

	अगर (box->style != शून्य &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		अगर (css_rect.बायाँ_auto == false)
			r.x0 = x - border_बायाँ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.बायाँ, css_rect.lunit));

		अगर (css_rect.शीर्ष_auto == false)
			r.y0 = y - border_शीर्ष + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.शीर्ष, css_rect.tunit));

		अगर (css_rect.दायाँ_auto == false)
			r.x1 = x - border_बायाँ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.दायाँ, css_rect.runit));

		अगर (css_rect.bottom_auto == false)
			r.y1 = y - border_शीर्ष + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		अगर (r.x0 < clip->x0) r.x0 = clip->x0;
		अगर (r.y0 < clip->y0) r.y0 = clip->y0;
		अगर (clip->x1 < r.x1) r.x1 = clip->x1;
		अगर (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		अगर (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			वापस ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			वापस false;

	} अन्यथा अगर (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		अगर (r.x0 < clip->x0) r.x0 = clip->x0;
		अगर (r.y0 < clip->y0) r.y0 = clip->y0;
		अगर (clip->x1 < r.x1) r.x1 = clip->x1;
		अगर (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		अगर (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			वापस ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			वापस false;
	} अन्यथा {
		/* clip box is fine, clip to it */
		r = *clip;
		अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			वापस false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	अगर (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != शून्य &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		काष्ठा rect p;
		p.x0 = x - border_बायाँ < r.x0 ? r.x0 : x - border_बायाँ;
		p.y0 = y - border_शीर्ष < r.y0 ? r.y0 : y - border_शीर्ष;
		p.x1 = x + padding_width + border_दायाँ < r.x1 ?
				x + padding_width + border_दायाँ : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		अगर (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			पूर्णांक m_बायाँ, m_शीर्ष, m_दायाँ, m_bottom;
			अगर (scale == 1.0) {
				m_बायाँ = box->margin[बायाँ_2];
				m_शीर्ष = box->margin[शीर्ष_2];
				m_दायाँ = box->margin[दायाँ_2];
				m_bottom = box->margin[BOTTOM];
			} अन्यथा {
				m_बायाँ = box->margin[बायाँ_2] * scale;
				m_शीर्ष = box->margin[शीर्ष_2] * scale;
				m_दायाँ = box->margin[दायाँ_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_बायाँ < r.x0 ? r.x0 : p.x0 - m_बायाँ;
			p.y0 = p.y0 - m_शीर्ष < r.y0 ? r.y0 : p.y0 - m_शीर्ष;
			p.x1 = p.x1 + m_दायाँ < r.x1 ? p.x1 + m_दायाँ : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		अगर ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			अगर (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				वापस false;
			/* restore previous graphics window */
			अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				वापस false;
		}
	}

	/* borders for block level content and replaced inlines */
	अगर (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != शून्य &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_शीर्ष || border_दायाँ || border_bottom || border_बायाँ)) {
		अगर (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			वापस false;
	}

	/* backgrounds and borders for non-replaced inlines */
	अगर (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_शीर्ष || border_दायाँ ||
			border_bottom || border_बायाँ)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		काष्ठा box *ib;
		काष्ठा rect b; /* border edge rectangle */
		काष्ठा rect p; /* clipped rect */
		bool first = true;
		पूर्णांक ib_x;
		पूर्णांक ib_y = y;
		पूर्णांक ib_p_width;
		पूर्णांक ib_b_बायाँ, ib_b_दायाँ;

		b.x0 = x - border_बायाँ;
		b.x1 = x + padding_width + border_दायाँ;
		b.y0 = y - border_शीर्ष;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		क्रम (ib = box; ib; ib = ib->अग्र) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			अगर (ib->type == BOX_FLOAT_बायाँ ||
					ib->type == BOX_FLOAT_दायाँ)
				जारी;
			अगर (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[बायाँ_2] + ib->width +
						ib->padding[दायाँ_2];
				ib_b_बायाँ = ib->border[बायाँ_2].width;
				ib_b_दायाँ = ib->border[दायाँ_2].width;
			} अन्यथा {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[बायाँ_2] + ib->width +
						ib->padding[दायाँ_2]) * scale;
				ib_b_बायाँ = ib->border[बायाँ_2].width * scale;
				ib_b_दायाँ = ib->border[दायाँ_2].width * scale;
			}

			अगर ((ib->flags & NEW_पंक्ति) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				अगर (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					वापस false;
				/* restore previous graphics window */
				अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					वापस false;
				अगर (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					वापस false;
				/* reset coords */
				b.x0 = ib_x - ib_b_बायाँ;
				b.y0 = ib_y - border_शीर्ष - padding_शीर्ष;
				b.y1 = ib_y + padding_height - padding_शीर्ष +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_दायाँ;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			अगर (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				अवरोध;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		अगर (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			वापस false;
		/* restore previous graphics window */
		अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			वापस false;
		अगर (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			वापस false;

	}

	/* Debug outlines */
	अगर (html_redraw_debug) {
		पूर्णांक margin_बायाँ, margin_दायाँ;
		पूर्णांक margin_शीर्ष, margin_bottom;
		अगर (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_बायाँ = box->margin[बायाँ_2];
			margin_शीर्ष = box->margin[शीर्ष_2];
			margin_दायाँ = box->margin[दायाँ_2];
			margin_bottom = box->margin[BOTTOM];
		} अन्यथा {
			margin_बायाँ = box->margin[बायाँ_2] * scale;
			margin_शीर्ष = box->margin[शीर्ष_2] * scale;
			margin_दायाँ = box->margin[दायाँ_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_बायाँ;
		rect.y0 = y + padding_शीर्ष;
		rect.x1 = x + padding_बायाँ + width;
		rect.y1 = y + padding_शीर्ष + height;
		अगर (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			वापस false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		अगर (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			वापस false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_बायाँ - margin_बायाँ;
		rect.y0 = y - border_शीर्ष - margin_शीर्ष;
		rect.x1 = x + padding_width + border_दायाँ + margin_दायाँ;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		अगर (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			वापस false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	अगर (box->parent != शून्य) {
		bool need_clip = false;
		अगर (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			अगर (r.x0 < clip->x0) r.x0 = clip->x0;
			अगर (r.y0 < clip->y0) r.y0 = clip->y0;
			अगर (clip->x1 < r.x1) r.x1 = clip->x1;
			अगर (clip->y1 < r.y1) r.y1 = clip->y1;
			अगर (r.x1 <= r.x0 || r.y1 <= r.y0) {
				वापस (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} अन्यथा अगर (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			अगर (r.x0 < clip->x0) r.x0 = clip->x0;
			अगर (clip->x1 < r.x1) r.x1 = clip->x1;
			अगर (r.x1 <= r.x0) {
				वापस (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} अन्यथा अगर (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			अगर (r.y0 < clip->y0) r.y0 = clip->y0;
			अगर (clip->y1 < r.y1) r.y1 = clip->y1;
			अगर (r.y1 <= r.y0) {
				वापस (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		अगर (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			अगर (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				वापस false;
		}
	}

	/* text decoration */
	अगर ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		अगर (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			वापस false;
	}

	अगर (box->गाँठ != शून्य) {
		exc = dom_html_element_get_tag_type(box->गाँठ, &tag_type);
		अगर (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} अन्यथा {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	अगर (box->object && width != 0 && height != 0) {
		काष्ठा content_redraw_तथ्य obj_तथ्य;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_तथ्य.x = x_scrolled + padding_बायाँ;
		obj_तथ्य.y = y_scrolled + padding_शीर्ष;
		obj_तथ्य.width = width;
		obj_तथ्य.height = height;
		obj_तथ्य.background_colour = current_background_color;
		obj_तथ्य.scale = scale;
		obj_तथ्य.repeat_x = false;
		obj_तथ्य.repeat_y = false;

		अगर (content_get_type(box->object) == CONTENT_HTML) {
			obj_तथ्य.x /= scale;
			obj_तथ्य.y /= scale;
		}

		अगर (!content_redraw(box->object, &obj_तथ्य, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			स्थिर अक्षर *obj = "\xef\xbf\xbc";
			पूर्णांक obj_width;
			पूर्णांक obj_x = x + padding_बायाँ;
			nserror res;

			rect.x0 = x + padding_बायाँ;
			rect.y0 = y + padding_शीर्ष;
			rect.x1 = x + padding_बायाँ + width - 1;
			rect.y1 = y + padding_शीर्ष + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			अगर (res != NSERROR_OK) {
				वापस false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  माप(obj) - 1,
						  &obj_width);
			अगर (res != NSERROR_OK) {
				obj_x += 1;
			} अन्यथा {
				obj_x += width / 2 - obj_width / 2;
			}

			अगर (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_शीर्ष + (पूर्णांक)(height * 0.75),
					    obj, माप(obj) - 1) != NSERROR_OK)
				वापस false;
		}
	} अन्यथा अगर (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->गाँठ != शून्य &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		काष्ठा bitmap *bitmap = शून्य;
		exc = dom_गाँठ_get_user_तथ्य(box->गाँठ,
					     corestring_dom_ns_कुंजी_canvas_गाँठ_तथ्य,
					     &bitmap);
		अगर (exc != DOM_NO_ERR) {
			bitmap = शून्य;
		}
		अगर (bitmap != शून्य &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_बायाँ, y + padding_शीर्ष,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			वापस false;
	} अन्यथा अगर (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_बायाँ,
				y + padding_शीर्ष, &r, ctx);

	} अन्यथा अगर (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		अगर (!html_redraw_checkbox(x + padding_बायाँ, y + padding_शीर्ष,
				width, height, box->gadget->selected, ctx))
			वापस false;

	} अन्यथा अगर (box->gadget && box->gadget->type == GADGET_RADIO) {
		अगर (!html_redraw_radio(x + padding_बायाँ, y + padding_शीर्ष,
				width, height, box->gadget->selected, ctx))
			वापस false;

	} अन्यथा अगर (box->gadget && box->gadget->type == GADGET_FILE) {
		अगर (!html_redraw_file(x + padding_बायाँ, y + padding_शीर्ष,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			वापस false;

	} अन्यथा अगर (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->तथ्य.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} अन्यथा अगर (box->text) {
		अगर (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			वापस false;

	} अन्यथा {
		अगर (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			वापस false;
	}

	अगर (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		अगर (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			वापस false;

	/* list marker */
	अगर (box->list_marker) {
		अगर (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			वापस false;
	}

	/* scrollbars */
	अगर (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == शून्य || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != शून्य) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((काष्ठा content *)html,
					    box, has_x_scroll, has_y_scroll);
		अगर (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			वापस false;
		}

		अगर (box->scroll_x != शून्य)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[शीर्ष_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		अगर (box->scroll_y != शून्य)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[बायाँ_2] +
					box->width + box->padding[दायाँ_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	अगर (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		अगर (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			वापस false;
	}

	वापस ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(काष्ठा content *c, काष्ठा content_redraw_तथ्य *तथ्य,
		स्थिर काष्ठा rect *clip, स्थिर काष्ठा redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	काष्ठा box *box;
	bool परिणाम = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = तथ्य->background_colour,
	};

	box = html->layout;
	निश्चित(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	अगर (ctx->interactive && html->visible_select_menu != शून्य) {
		काष्ठा form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				तथ्य->scale, clip);
	}

	अगर (!select_only) {
		/* clear to background colour */
		परिणाम = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		अगर (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		परिणाम &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		परिणाम &= html_redraw_box(html, box, तथ्य->x, तथ्य->y, clip,
				तथ्य->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	अगर (select) {
		पूर्णांक menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[बायाँ_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[शीर्ष_2];
		परिणाम &= form_redraw_select_menu(html->visible_select_menu,
				तथ्य->x + menu_x, तथ्य->y + menu_y,
				तथ्य->scale, clip, ctx);
	}

	वापस परिणाम;

}
