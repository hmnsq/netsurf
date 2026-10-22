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

#অন্তর্ভাব "utils/config.h"
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/browser_window.h"
#অন্তর্ভাব "netsurf/plotters.h"
#অন্তর্ভাব "netsurf/bitmap.h"
#অন্তর্ভাব "netsurf/layout.h"
#অন্তর্ভাব "content/content.h"
#অন্তর্ভাব "content/content_protected.h"
#অন্তর্ভাব "content/textsearch.h"
#অন্তর্ভাব "css/utils.h"
#অন্তর্ভাব "desktop/selection.h"
#অন্তর্ভাব "desktop/print.h"
#অন্তর্ভাব "desktop/scrollbar.h"
#অন্তর্ভাব "desktop/textarea.h"
#অন্তর্ভাব "desktop/gui_internal.h"

#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"
#অন্তর্ভাব "html/box_manipulate.h"
#অন্তর্ভাব "html/font.h"
#অন্তর্ভাব "html/form_internal.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/layout.h"


bool html_redraw_debug = false;

/**
 * Determine if a box has a background that needs drawing
 *
 * \param box  Box to consider
 * \return True if box has a background, false otherwise.
 */
স্থির bool html_redraw_box_has_background(কাঠামো box *box)
{
	যদি (box->background != ফাঁকা)
		ফেরত true;

	যদি (box->style != ফাঁকা) {
		css_color colour;

		css_computed_background_color(box->style, &colour);

		যদি (nscss_color_is_transparent(colour) == false)
			ফেরত true;
	}

	ফেরত false;
}

/**
 * Find the background box for a box
 *
 * \param box  Box to find background box for
 * \return Pointer to background box, or NULL if there is none
 */
স্থির কাঠামো box *html_redraw_find_bg_box(কাঠামো box *box)
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
	যদি (box->parent == ফাঁকা) {
		/* Root box */
		যদি (html_redraw_box_has_background(box))
			ফেরত box;

		/* No background on root box: consider body box, if any */
		যদি (box->children != ফাঁকা) {
			যদি (html_redraw_box_has_background(box->children))
				ফেরত box->children;
		}
	} অন্যথা যদি (box->parent != ফাঁকা && box->parent->parent == ফাঁকা) {
		/* Body box: only render background if root has its own */
		যদি (html_redraw_box_has_background(box) &&
				html_redraw_box_has_background(box->parent))
			ফেরত box;
	} অন্যথা {
		/* Any other box */
		যদি (html_redraw_box_has_background(box))
			ফেরত box;
	}

	ফেরত ফাঁকা;
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

স্থির bool
text_redraw(ধ্রুবক অক্ষর *utf8_text,
	    size_t utf8_len,
	    size_t offset,
	    পূর্ণসংখ্যা space,
	    ধ্রুবক plot_font_style_t *fstyle,
	    পূর্ণসংখ্যা x,
	    পূর্ণসংখ্যা y,
	    ধ্রুবক কাঠামো rect *clip,
	    পূর্ণসংখ্যা height,
	    দশমিক scale,
	    bool excluded,
	    কাঠামো content *c,
	    ধ্রুবক কাঠামো selection *sel,
	    ধ্রুবক কাঠামো redraw_context *ctx)
{
	bool highlighted = false;
	plot_font_style_t plot_fstyle = *fstyle;
	nserror res;

	/* Need scaled text size to pass to plotters */
	plot_fstyle.আকার_2 *= scale;

	/* is this box part of a selection? */
	যদি (!excluded && ctx->interactive == true) {
		unsigned len = utf8_len + (space ? 1 : 0);
		unsigned start_idx;
		unsigned end_idx;

		/* first try the browser window's current selection */
		যদি (selection_highlighted(sel,
					  offset,
					  offset + len,
					  &start_idx,
					  &end_idx)) {
			highlighted = true;
		}

		/* what about the current search operation, if any? */
		যদি (!highlighted &&
		    (c->textsearch.context != ফাঁকা) &&
		    content_textsearch_ishighlighted(c->textsearch.context,
						     offset,
						     offset + len,
						     &start_idx,
						     &end_idx)) {
			highlighted = true;
		}

		/* \todo make search terms visible within selected text */
		যদি (highlighted) {
			কাঠামো rect r;
			unsigned endtxt_idx = end_idx;
			bool clip_changed = false;
			bool text_visible = true;
			পূর্ণসংখ্যা startx, endx;
			plot_style_t pstyle_fill_hback = *plot_style_fill_white;
			plot_font_style_t fstyle_hback = plot_fstyle;

			যদি (end_idx > utf8_len) {
				/* adjust for trailing space, not present in
				 * utf8_text */
				প্রতিজ্ঞা(end_idx == utf8_len + 1);
				endtxt_idx = utf8_len;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, start_idx,
						  &startx);
			যদি (res != NSERROR_OK) {
				startx = 0;
			}

			res = guit->layout->width(fstyle,
						  utf8_text, endtxt_idx,
						  &endx);
			যদি (res != NSERROR_OK) {
				endx = 0;
			}

			/* is there a trailing space that should be highlighted
			 * as well? */
			যদি (end_idx > utf8_len) {
					endx += space;
			}

			যদি (scale != 1.0) {
				startx *= scale;
				endx *= scale;
			}

			/* draw any text preceding highlighted portion */
			যদি ((start_idx > 0) &&
			    (ctx->plot->text(ctx,
					     &plot_fstyle,
					     x,
					     y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
					     utf8_text,
					     start_idx) != NSERROR_OK))
				ফেরত false;

			pstyle_fill_hback.fill_colour = fstyle->foreground;

			/* highlighted portion */
			r.x0 = x + startx;
			r.y0 = y;
			r.x1 = x + endx;
			r.y1 = y + height * scale;
			res = ctx->plot->rectangle(ctx, &pstyle_fill_hback, &r);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}

			যদি (start_idx > 0) {
				পূর্ণসংখ্যা px0 = max(x + startx, clip->x0);
				পূর্ণসংখ্যা px1 = min(x + endx, clip->x1);

				যদি (px0 < px1) {
					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = px1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}

					clip_changed = true;
				} অন্যথা {
					text_visible = false;
				}
			}

			fstyle_hback.background =
				pstyle_fill_hback.fill_colour;
			fstyle_hback.foreground = colour_to_bw_furthest(
				pstyle_fill_hback.fill_colour);

			যদি (text_visible &&
			    (ctx->plot->text(ctx,
					     &fstyle_hback,
					     x,
					     y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
					     utf8_text,
					     endtxt_idx) != NSERROR_OK)) {
				ফেরত false;
			}

			/* draw any text succeeding highlighted portion */
			যদি (endtxt_idx < utf8_len) {
				পূর্ণসংখ্যা px0 = max(x + endx, clip->x0);
				যদি (px0 < clip->x1) {

					r.x0 = px0;
					r.y0 = clip->y0;
					r.x1 = clip->x1;
					r.y1 = clip->y1;
					res = ctx->plot->clip(ctx, &r);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}

					clip_changed = true;

					res = ctx->plot->text(ctx,
							      &plot_fstyle,
							      x,
							      y + (পূর্ণসংখ্যা)(height * 0.75 * scale),
							      utf8_text,
							      utf8_len);
					যদি (res != NSERROR_OK) {
						ফেরত false;
					}
				}
			}

			যদি (clip_changed &&
			    (ctx->plot->clip(ctx, clip) != NSERROR_OK)) {
				ফেরত false;
			}
		}
	}

	যদি (!highlighted) {
		res = ctx->plot->text(ctx,
				      &plot_fstyle,
				      x,
				      y + (পূর্ণসংখ্যা) (height * 0.75 * scale),
				      utf8_text,
				      utf8_len);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}
	ফেরত true;
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

স্থির bool html_redraw_checkbox(পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height,
		bool selected, ধ্রুবক কাঠামো redraw_context *ctx)
{
	দ্বিগুণ z;
	nserror res;
	কাঠামো rect rect;

	z = width * 0.15;
	যদি (z == 0) {
		z = 1;
	}

	rect.x0 = x;
	rect.y0 = y ;
	rect.x1 = x + width;
	rect.y1 = y + height;
	res = ctx->plot->rectangle(ctx, plot_style_fill_wbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* dark line across top */
	rect.y1 = y;
	res = ctx->plot->সারি(ctx, plot_style_stroke_darkwbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* dark line across left */
	rect.x1 = x;
	rect.y1 = y + height;
	res = ctx->plot->সারি(ctx, plot_style_stroke_darkwbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* light line across right */
	rect.x0 = x + width;
	rect.x1 = x + width;
	res = ctx->plot->সারি(ctx, plot_style_stroke_lightwbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* light line across bottom */
	rect.x0 = x;
	rect.y0 = y + height;
	res = ctx->plot->সারি(ctx, plot_style_stroke_lightwbasec, &rect);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	যদি (selected) {
		যদি (width < 12 || height < 12) {
			/* render a solid box instead of a tick */
			rect.x0 = x + z + z;
			rect.y0 = y + z + z;
			rect.x1 = x + width - z;
			rect.y1 = y + height - z;
			res = ctx->plot->rectangle(ctx, plot_style_fill_wblobc, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
		} অন্যথা {
			/* render a tick, as it'll fit comfortably */
			rect.x0 = x + width - z;
			rect.y0 = y + z;
			rect.x1 = x + (z * 3);
			rect.y1 = y + height - z;
			res = ctx->plot->সারি(ctx, plot_style_stroke_wblobc, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}

			rect.x0 = x + (z * 3);
			rect.y0 = y + height - z;
			rect.x1 = x + z + z;
			rect.y1 = y + (height / 2);
			res = ctx->plot->সারি(ctx, plot_style_stroke_wblobc, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
		}
	}
	ফেরত true;
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
স্থির bool html_redraw_radio(পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height,
		bool selected, ধ্রুবক কাঠামো redraw_context *ctx)
{
	nserror res;

	/* plot background of radio button */
	res = ctx->plot->disc(ctx,
			      plot_style_fill_wbasec,
			      x + width * 0.5,
			      y + height * 0.5,
			      width * 0.5 - 1);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* plot dark arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_darkwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     45,
			     225);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	/* plot light arc */
	res = ctx->plot->arc(ctx,
			     plot_style_fill_lightwbasec,
			     x + width * 0.5,
			     y + height * 0.5,
			     width * 0.5 - 1,
			     225,
			     45);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}

	যদি (selected) {
		/* plot selection blob */
		res = ctx->plot->disc(ctx,
				      plot_style_fill_wblobc,
				      x + width * 0.5,
				      y + height * 0.5,
				      width * 0.3 - 1);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}

	ফেরত true;
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

স্থির bool html_redraw_file(পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, পূর্ণসংখ্যা width, পূর্ণসংখ্যা height,
		কাঠামো box *box, দশমিক scale, colour background_colour,
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	পূর্ণসংখ্যা text_width;
	ধ্রুবক অক্ষর *text;
	size_t দৈর্ঘ্য;
	plot_font_style_t fstyle;
	nserror res;

	font_plot_style_from_css(unit_len_ctx, box->style, &fstyle);
	fstyle.background = background_colour;

	যদি (box->gadget->মান) {
		text = box->gadget->মান;
	} অন্যথা {
		text = messages_get("Form_Drop");
	}
	দৈর্ঘ্য = strlen(text);

	res = guit->layout->width(&fstyle, text, দৈর্ঘ্য, &text_width);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}
	text_width *= scale;
	যদি (width < text_width + 8) {
		x = x + width - text_width - 4;
	} অন্যথা {
		x = x + 4;
	}

	res = ctx->plot->text(ctx, &fstyle, x, y + height * 0.75, text, দৈর্ঘ্য);
	যদি (res != NSERROR_OK) {
		ফেরত false;
	}
	ফেরত true;
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

স্থির bool html_redraw_background(পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, কাঠামো box *box, দশমিক scale,
		ধ্রুবক কাঠামো rect *clip, colour *background_colour,
		কাঠামো box *background,
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	bool repeat_x = false;
	bool repeat_y = false;
	bool plot_colour = true;
	bool plot_content;
	bool clip_to_children = false;
	কাঠামো box *clip_box = box;
	পূর্ণসংখ্যা ox = x, oy = y;
	পূর্ণসংখ্যা width, height;
	css_fixed hpos = 0, vpos = 0;
	css_unit hunit = CSS_UNIT_PX, vunit = CSS_UNIT_PX;
	কাঠামো box *parent;
	কাঠামো rect r = *clip;
	css_color bgcol;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = *background_colour,
	};
	nserror res;

	যদি (ctx->background_images == false)
		ফেরত true;

	plot_content = (background->background != ফাঁকা);

	যদি (plot_content) {
		যদি (!box->parent) {
			/* Root element, special case:
			 * background origin calc. is based on margin box */
			x -= box->margin[বাম_2] * scale;
			y -= box->margin[শীর্ষ_2] * scale;
			width = box->margin[বাম_2] + box->padding[বাম_2] +
					box->width + box->padding[ডান_2] +
					box->margin[ডান_2];
			height = box->margin[শীর্ষ_2] + box->padding[শীর্ষ_2] +
					box->height + box->padding[BOTTOM] +
					box->margin[BOTTOM];
		} অন্যথা {
			width = box->padding[বাম_2] + box->width +
					box->padding[ডান_2];
			height = box->padding[শীর্ষ_2] + box->height +
					box->padding[BOTTOM];
		}
		/* handle background-repeat */
		নির্বাচন (css_computed_background_repeat(background->style)) {
		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque */
			plot_colour = !content_get_opaque(background->background);
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_NO_REPEAT:
			বিরতি;

		পূর্বনির্ধারিত:
			বিরতি;
		}

		/* handle background-position */
		css_computed_background_position(background->style,
				&hpos, &hunit, &vpos, &vunit);
		যদি (hunit == CSS_UNIT_PCT) {
			x += (width -
				content_get_width(background->background)) *
				scale * FIXTOFLT(hpos) / 100.;
		} অন্যথা {
			x += (পূর্ণসংখ্যা) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		যদি (vunit == CSS_UNIT_PCT) {
			y += (height -
				content_get_height(background->background)) *
				scale * FIXTOFLT(vpos) / 100.;
		} অন্যথা {
			y += (পূর্ণসংখ্যা) (FIXTOFLT(css_unit_len2device_px(
					background->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* special case for table rows as their background needs
	 * to be clipped to all the cells */
	যদি (box->type == BOX_TABLE_ROW) {
		css_fixed h = 0, v = 0;
		css_unit hu = CSS_UNIT_PX, vu = CSS_UNIT_PX;

		জন্য (parent = box->parent;
			((parent) && (parent->type != BOX_TABLE));
				parent = parent->parent);
		প্রতিজ্ঞা(parent && (parent->style));

		css_computed_border_spacing(parent->style, &h, &hu, &v, &vu);

		clip_to_children = (h > 0) || (v > 0);

		যদি (clip_to_children)
			clip_box = box->children;
	}

	জন্য (; clip_box; clip_box = clip_box->পরবর্তী) {
		/* clip to child boxes if needed */
		যদি (clip_to_children) {
			প্রতিজ্ঞা(clip_box->type == BOX_TABLE_CELL);

			/* update clip.* to the child cell */
			r.x0 = ox + (clip_box->x * scale);
			r.y0 = oy + (clip_box->y * scale);
			r.x1 = r.x0 + (clip_box->padding[বাম_2] +
					clip_box->width +
					clip_box->padding[ডান_2]) * scale;
			r.y1 = r.y0 + (clip_box->padding[শীর্ষ_2] +
					clip_box->height +
					clip_box->padding[BOTTOM]) * scale;

			যদি (r.x0 < clip->x0) r.x0 = clip->x0;
			যদি (r.y0 < clip->y0) r.y0 = clip->y0;
			যদি (r.x1 > clip->x1) r.x1 = clip->x1;
			যদি (r.y1 > clip->y1) r.y1 = clip->y1;

			css_computed_background_color(clip_box->style, &bgcol);

			/* <td> attributes override <tr> */
			/* if the background content is opaque there
			 * is no need to plot underneath it.
			 */
			যদি ((r.x0 >= r.x1) ||
			    (r.y0 >= r.y1) ||
			    (nscss_color_is_transparent(bgcol) == false) ||
			    ((clip_box->background != ফাঁকা) &&
			     content_get_opaque(clip_box->background)))
				চালু;
		}

		/* plot the background colour */
		css_computed_background_color(background->style, &bgcol);

		যদি (nscss_color_is_transparent(bgcol) == false) {
			*background_colour = nscss_color_to_ns(bgcol);
			pstyle_fill_bg.fill_colour = *background_colour;
			যদি (plot_colour) {
				res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
				যদি (res != NSERROR_OK) {
					ফেরত false;
				}
			}
		}
		/* and plot the image */
		যদি (plot_content) {
			width = content_get_width(background->background);
			height = content_get_height(background->background);

			/* ensure clip area only as large as required */
			যদি (!repeat_x) {
				যদি (r.x0 < x)
					r.x0 = x;
				যদি (r.x1 > x + width * scale)
					r.x1 = x + width * scale;
			}
			যদি (!repeat_y) {
				যদি (r.y0 < y)
					r.y0 = y;
				যদি (r.y1 > y + height * scale)
					r.y1 = y + height * scale;
			}
			/* valid clipping rectangles only */
			যদি ((r.x0 < r.x1) && (r.y0 < r.y1)) {
				কাঠামো content_redraw_তথ্য bg_তথ্য;

				res = ctx->plot->clip(ctx, &r);
				যদি (res != NSERROR_OK) {
					ফেরত false;
				}

				bg_তথ্য.x = x;
				bg_তথ্য.y = y;
				bg_তথ্য.width = ceilf(width * scale);
				bg_তথ্য.height = ceilf(height * scale);
				bg_তথ্য.background_colour = *background_colour;
				bg_তথ্য.scale = scale;
				bg_তথ্য.repeat_x = repeat_x;
				bg_তথ্য.repeat_y = repeat_y;

				/* We just continue if redraw fails */
				content_redraw(background->background,
						&bg_তথ্য, &r, ctx);
			}
		}

		/* only <tr> rows being clipped to child boxes loop */
		যদি (!clip_to_children)
			ফেরত true;
	}
	ফেরত true;
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

স্থির bool html_redraw_inline_background(পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, কাঠামো box *box,
		দশমিক scale, ধ্রুবক কাঠামো rect *clip, কাঠামো rect b,
		bool first, bool last, colour *background_colour,
		ধ্রুবক css_unit_ctx *unit_len_ctx,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো rect r = *clip;
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

	plot_content = (box->background != ফাঁকা);

	যদি (html_redraw_printing && nsoption_bool(remove_backgrounds))
		ফেরত true;

	যদি (plot_content) {
		/* handle background-repeat */
		নির্বাচন (css_computed_background_repeat(box->style)) {
		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT:
			repeat_x = repeat_y = true;
			/* optimisation: only plot the colour if
			 * bitmap is not opaque
			 */
			plot_colour = !content_get_opaque(box->background);
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_X:
			repeat_x = true;
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_REPEAT_Y:
			repeat_y = true;
			বিরতি;

		ক্ষেত্র CSS_BACKGROUND_REPEAT_NO_REPEAT:
			বিরতি;

		পূর্বনির্ধারিত:
			বিরতি;
		}

		/* handle background-position */
		css_computed_background_position(box->style,
				&hpos, &hunit, &vpos, &vunit);
		যদি (hunit == CSS_UNIT_PCT) {
			x += (b.x1 - b.x0 -
					content_get_width(box->background) *
					scale) * FIXTOFLT(hpos) / 100.;

			যদি (!repeat_x && ((hpos < 2 && !first) ||
					(hpos > 98 && !last))){
				plot_content = false;
			}
		} অন্যথা {
			x += (পূর্ণসংখ্যা) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					hpos, hunit)) * scale);
		}

		যদি (vunit == CSS_UNIT_PCT) {
			y += (b.y1 - b.y0 -
					content_get_height(box->background) *
					scale) * FIXTOFLT(vpos) / 100.;
		} অন্যথা {
			y += (পূর্ণসংখ্যা) (FIXTOFLT(css_unit_len2device_px(
					box->style, unit_len_ctx,
					vpos, vunit)) * scale);
		}
	}

	/* plot the background colour */
	css_computed_background_color(box->style, &bgcol);

	যদি (nscss_color_is_transparent(bgcol) == false) {
		*background_colour = nscss_color_to_ns(bgcol);
		pstyle_fill_bg.fill_colour = *background_colour;

		যদি (plot_colour) {
			res = ctx->plot->rectangle(ctx, &pstyle_fill_bg, &r);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
		}
	}
	/* and plot the image */
	যদি (plot_content) {
		পূর্ণসংখ্যা width = content_get_width(box->background);
		পূর্ণসংখ্যা height = content_get_height(box->background);

		যদি (!repeat_x) {
			যদি (r.x0 < x)
				r.x0 = x;
			যদি (r.x1 > x + width * scale)
				r.x1 = x + width * scale;
		}
		যদি (!repeat_y) {
			যদি (r.y0 < y)
				r.y0 = y;
			যদি (r.y1 > y + height * scale)
				r.y1 = y + height * scale;
		}
		/* valid clipping rectangles only */
		যদি ((r.x0 < r.x1) && (r.y0 < r.y1)) {
			কাঠামো content_redraw_তথ্য bg_তথ্য;

			res = ctx->plot->clip(ctx, &r);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}

			bg_তথ্য.x = x;
			bg_তথ্য.y = y;
			bg_তথ্য.width = ceilf(width * scale);
			bg_তথ্য.height = ceilf(height * scale);
			bg_তথ্য.background_colour = *background_colour;
			bg_তথ্য.scale = scale;
			bg_তথ্য.repeat_x = repeat_x;
			bg_তথ্য.repeat_y = repeat_y;

			/* We just continue if redraw fails */
			content_redraw(box->background, &bg_তথ্য, &r, ctx);
		}
	}

	ফেরত true;
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

স্থির bool
html_redraw_text_decoration_inline(কাঠামো box *box,
				   পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
				   দশমিক scale,
				   colour colour,
				   দশমিক ratio,
				   ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	কাঠামো rect rect;

	জন্য (c = box->পরবর্তী;
	     c && c != box->inline_end;
	     c = c->পরবর্তী) {
		যদি (c->type != BOX_TEXT) {
			চালু;
		}
		rect.x0 = (x + c->x) * scale;
		rect.y0 = (y + c->y + c->height * ratio) * scale;
		rect.x1 = (x + c->x + c->width) * scale;
		rect.y1 = (y + c->y + c->height * ratio) * scale;
		res = ctx->plot->সারি(ctx, &plot_style_box, &rect);
		যদি (res != NSERROR_OK) {
			ফেরত false;
		}
	}
	ফেরত true;
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

স্থির bool
html_redraw_text_decoration_block(কাঠামো box *box,
				  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
				  দশমিক scale,
				  colour colour,
				  দশমিক ratio,
				  ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো box *c;
	plot_style_t plot_style_box = {
		.stroke_type = PLOT_OP_TYPE_SOLID,
		.stroke_colour = colour,
	};
	nserror res;
	কাঠামো rect rect;

	/* draw through text descendants */
	জন্য (c = box->children; c; c = c->পরবর্তী) {
		যদি (c->type == BOX_TEXT) {
			rect.x0 = (x + c->x) * scale;
			rect.y0 = (y + c->y + c->height * ratio) * scale;
			rect.x1 = (x + c->x + c->width) * scale;
			rect.y1 = (y + c->y + c->height * ratio) * scale;
			res = ctx->plot->সারি(ctx, &plot_style_box, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}
		} অন্যথা যদি ((c->type == BOX_INLINE_CONTAINER) || (c->type == BOX_BLOCK)) {
			যদি (!html_redraw_text_decoration_block(c,
					x + c->x, y + c->y,
					scale, colour, ratio, ctx))
				ফেরত false;
		}
	}
	ফেরত true;
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

স্থির bool html_redraw_text_decoration(কাঠামো box *box,
		পূর্ণসংখ্যা x_parent, পূর্ণসংখ্যা y_parent, দশমিক scale,
		colour background_colour, ধ্রুবক কাঠামো redraw_context *ctx)
{
	স্থির ধ্রুবক গণনা css_text_decoration_e decoration[] = {
		CSS_TEXT_DECORATION_UNDERLINE, CSS_TEXT_DECORATION_OVERLINE,
		CSS_TEXT_DECORATION_সারি_THROUGH };
	স্থির ধ্রুবক দশমিক সারি_ratio[] = { 0.9, 0.1, 0.5 };
	colour fgcol;
	unsigned পূর্ণসংখ্যা i;
	css_color col;

	css_computed_color(box->style, &col);
	fgcol = nscss_color_to_ns(col);

	/* antialias colour for under/overline */
	যদি (html_redraw_printing == false)
		fgcol = blend_colour(background_colour, fgcol);

	যদি (box->type == BOX_INLINE) {
		যদি (!box->inline_end)
			ফেরত true;
		জন্য (i = 0; i != NOF_ELEMENTS(decoration); i++)
			যদি (css_computed_text_decoration(box->style) &
					decoration[i])
				যদি (!html_redraw_text_decoration_inline(box,
						x_parent, y_parent, scale,
						fgcol, সারি_ratio[i], ctx))
					ফেরত false;
	} অন্যথা {
		জন্য (i = 0; i != NOF_ELEMENTS(decoration); i++)
			যদি (css_computed_text_decoration(box->style) &
					decoration[i])
				যদি (!html_redraw_text_decoration_block(box,
						x_parent + box->x,
						y_parent + box->y,
						scale,
						fgcol, সারি_ratio[i], ctx))
					ফেরত false;
	}

	ফেরত true;
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

স্থির bool html_redraw_text_box(ধ্রুবক html_content *html, কাঠামো box *box,
		পূর্ণসংখ্যা x, পূর্ণসংখ্যা y, ধ্রুবক কাঠামো rect *clip, দশমিক scale,
		colour current_background_color,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	bool excluded = (box->object != ফাঁকা);
	plot_font_style_t fstyle;

	font_plot_style_from_css(&html->unit_len_ctx, box->style, &fstyle);
	fstyle.background = current_background_color;

	যদি (!text_redraw(box->text,
			 box->দৈর্ঘ্য,
			 box->byte_offset,
			 box->space,
			 &fstyle,
			 x, y,
			 clip,
			 box->height,
			 scale,
			 excluded,
			 (কাঠামো content *)html,
			 html->sel,
			 ctx))
		ফেরত false;

	ফেরত true;
}

bool html_redraw_box(ধ্রুবক html_content *html, কাঠামো box *box,
		পূর্ণসংখ্যা x_parent, পূর্ণসংখ্যা y_parent,
		ধ্রুবক কাঠামো rect *clip, দশমিক scale,
		colour current_background_color,
		ধ্রুবক কাঠামো redraw_context *ctx);

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

স্থির bool html_redraw_box_children(ধ্রুবক html_content *html, কাঠামো box *box,
		পূর্ণসংখ্যা x_parent, পূর্ণসংখ্যা y_parent,
		ধ্রুবক কাঠামো rect *clip, দশমিক scale,
		colour current_background_color,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	কাঠামো box *c;

	জন্য (c = box->children; c; c = c->পরবর্তী) {

		যদি (c->type != BOX_FLOAT_বাম && c->type != BOX_FLOAT_ডান)
			যদি (!html_redraw_box(html, c,
					x_parent + box->x -
					scrollbar_get_offset(box->scroll_x),
					y_parent + box->y -
					scrollbar_get_offset(box->scroll_y),
					clip, scale, current_background_color,
					ctx))
				ফেরত false;
	}
	জন্য (c = box->float_children; c; c = c->পরবর্তী_float)
		যদি (!html_redraw_box(html, c,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color,
				ctx))
			ফেরত false;

	ফেরত true;
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

bool html_redraw_box(ধ্রুবক html_content *html, কাঠামো box *box,
		পূর্ণসংখ্যা x_parent, পূর্ণসংখ্যা y_parent,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক দশমিক scale,
		colour current_background_color,
		ধ্রুবক কাঠামো redraw_context *ctx)
{
	ধ্রুবক কাঠামো plotter_table *plot = ctx->plot;
	পূর্ণসংখ্যা x, y;
	পূর্ণসংখ্যা width, height;
	পূর্ণসংখ্যা padding_বাম, padding_শীর্ষ, padding_width, padding_height;
	পূর্ণসংখ্যা border_বাম, border_শীর্ষ, border_ডান, border_bottom;
	কাঠামো rect r;
	কাঠামো rect rect;
	পূর্ণসংখ্যা x_scrolled, y_scrolled;
	কাঠামো box *bg_box = ফাঁকা;
	css_computed_clip_rect css_rect;
	গণনা css_overflow_e overflow_x = CSS_OVERFLOW_VISIBLE;
	গণনা css_overflow_e overflow_y = CSS_OVERFLOW_VISIBLE;
	dom_exception exc;
	dom_html_element_type tag_type;


	যদি (html_redraw_printing && (box->flags & PRINTED))
		ফেরত true;

	যদি (box->style != ফাঁকা) {
		overflow_x = css_computed_overflow_x(box->style);
		overflow_y = css_computed_overflow_y(box->style);
	}

	/* avoid trivial FP maths */
	যদি (scale == 1.0) {
		x = x_parent + box->x;
		y = y_parent + box->y;
		width = box->width;
		height = box->height;
		padding_বাম = box->padding[বাম_2];
		padding_শীর্ষ = box->padding[শীর্ষ_2];
		padding_width = padding_বাম + box->width + box->padding[ডান_2];
		padding_height = padding_শীর্ষ + box->height +
				box->padding[BOTTOM];
		border_বাম = box->border[বাম_2].width;
		border_শীর্ষ = box->border[শীর্ষ_2].width;
		border_ডান = box->border[ডান_2].width;
		border_bottom = box->border[BOTTOM].width;
	} অন্যথা {
		x = (x_parent + box->x) * scale;
		y = (y_parent + box->y) * scale;
		width = box->width * scale;
		height = box->height * scale;
		/* left and top padding values are normally zero,
		 * so avoid trivial FP maths */
		padding_বাম = box->padding[বাম_2] ? box->padding[বাম_2] * scale
				: 0;
		padding_শীর্ষ = box->padding[শীর্ষ_2] ? box->padding[শীর্ষ_2] * scale
				: 0;
		padding_width = (box->padding[বাম_2] + box->width +
				box->padding[ডান_2]) * scale;
		padding_height = (box->padding[শীর্ষ_2] + box->height +
				box->padding[BOTTOM]) * scale;
		border_বাম = box->border[বাম_2].width * scale;
		border_শীর্ষ = box->border[শীর্ষ_2].width * scale;
		border_ডান = box->border[ডান_2].width * scale;
		border_bottom = box->border[BOTTOM].width * scale;
	}

	/* calculate rectangle covering this box and descendants */
	যদি (box->style && overflow_x != CSS_OVERFLOW_VISIBLE &&
			box->parent != ফাঁকা) {
		/* box contents clipped to box size */
		r.x0 = x - border_বাম;
		r.x1 = x + padding_width + border_ডান;
	} অন্যথা {
		/* box contents can hang out of the box; use descendant box */
		যদি (scale == 1.0) {
			r.x0 = x + box->descendant_x0;
			r.x1 = x + box->descendant_x1 + 1;
		} অন্যথা {
			r.x0 = x + box->descendant_x0 * scale;
			r.x1 = x + box->descendant_x1 * scale + 1;
		}
		যদি (!box->parent) {
			/* root element */
			পূর্ণসংখ্যা margin_বাম, margin_ডান;
			যদি (scale == 1.0) {
				margin_বাম = box->margin[বাম_2];
				margin_ডান = box->margin[ডান_2];
			} অন্যথা {
				margin_বাম = box->margin[বাম_2] * scale;
				margin_ডান = box->margin[ডান_2] * scale;
			}
			r.x0 = x - border_বাম - margin_বাম < r.x0 ?
					x - border_বাম - margin_বাম : r.x0;
			r.x1 = x + padding_width + border_ডান +
					margin_ডান > r.x1 ?
					x + padding_width + border_ডান +
					margin_ডান : r.x1;
		}
	}

	/* calculate rectangle covering this box and descendants */
	যদি (box->style && overflow_y != CSS_OVERFLOW_VISIBLE &&
			box->parent != ফাঁকা) {
		/* box contents clipped to box size */
		r.y0 = y - border_শীর্ষ;
		r.y1 = y + padding_height + border_bottom;
	} অন্যথা {
		/* box contents can hang out of the box; use descendant box */
		যদি (scale == 1.0) {
			r.y0 = y + box->descendant_y0;
			r.y1 = y + box->descendant_y1 + 1;
		} অন্যথা {
			r.y0 = y + box->descendant_y0 * scale;
			r.y1 = y + box->descendant_y1 * scale + 1;
		}
		যদি (!box->parent) {
			/* root element */
			পূর্ণসংখ্যা margin_শীর্ষ, margin_bottom;
			যদি (scale == 1.0) {
				margin_শীর্ষ = box->margin[শীর্ষ_2];
				margin_bottom = box->margin[BOTTOM];
			} অন্যথা {
				margin_শীর্ষ = box->margin[শীর্ষ_2] * scale;
				margin_bottom = box->margin[BOTTOM] * scale;
			}
			r.y0 = y - border_শীর্ষ - margin_শীর্ষ < r.y0 ?
					y - border_শীর্ষ - margin_শীর্ষ : r.y0;
			r.y1 = y + padding_height + border_bottom +
					margin_bottom > r.y1 ?
					y + padding_height + border_bottom +
					margin_bottom : r.y1;
		}
	}

	/* return if the rectangle is completely outside the clip rectangle */
	যদি (clip->y1 < r.y0 || r.y1 < clip->y0 ||
			clip->x1 < r.x0 || r.x1 < clip->x0)
		ফেরত true;

	/*if the rectangle is under the page bottom but it can fit in a page,
	don't print it now*/
	যদি (html_redraw_printing) {
		যদি (r.y1 > html_redraw_printing_border) {
			যদি (r.y1 - r.y0 <= html_redraw_printing_border &&
					(box->type == BOX_TEXT ||
					box->type == BOX_TABLE_CELL
					|| box->object || box->gadget)) {
				/*remember the highest of all points from the
				not printed elements*/
				যদি (r.y0 < html_redraw_printing_শীর্ষ_cropped)
					html_redraw_printing_শীর্ষ_cropped = r.y0;
				ফেরত true;
			}
		}
		অন্যথা box->flags |= PRINTED; /*it won't be printed anymore*/
	}

	/* if visibility is hidden render children only */
	যদি (box->style && css_computed_visibility(box->style) ==
			CSS_VISIBILITY_HIDDEN) {
		যদি ((ctx->plot->group_start) &&
		    (ctx->plot->group_start(ctx, "hidden box") != NSERROR_OK))
			ফেরত false;
		যদি (!html_redraw_box_children(html, box, x_parent, y_parent,
				&r, scale, current_background_color, ctx))
			ফেরত false;
		ফেরত ((!ctx->plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
	}

	যদি ((ctx->plot->group_start) &&
	    (ctx->plot->group_start(ctx,"vis box") != NSERROR_OK)) {
		ফেরত false;
	}

	যদি (box->style != ফাঁকা &&
			css_computed_position(box->style) ==
					CSS_POSITION_ABSOLUTE &&
			css_computed_clip(box->style, &css_rect) ==
					CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		যদি (css_rect.বাম_auto == false)
			r.x0 = x - border_বাম + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.বাম, css_rect.lunit));

		যদি (css_rect.শীর্ষ_auto == false)
			r.y0 = y - border_শীর্ষ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.শীর্ষ, css_rect.tunit));

		যদি (css_rect.ডান_auto == false)
			r.x1 = x - border_বাম + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.ডান, css_rect.runit));

		যদি (css_rect.bottom_auto == false)
			r.y1 = y - border_শীর্ষ + FIXTOINT(css_unit_len2device_px(
					box->style, &html->unit_len_ctx,
					css_rect.bottom, css_rect.bunit));

		/* find intersection of clip rectangle and box */
		যদি (r.x0 < clip->x0) r.x0 = clip->x0;
		যদি (r.y0 < clip->y0) r.y0 = clip->y0;
		যদি (clip->x1 < r.x1) r.x1 = clip->x1;
		যদি (clip->y1 < r.y1) r.y1 = clip->y1;
		/* Nothing to do for invalid rectangles */
		যদি (r.x0 >= r.x1 || r.y0 >= r.y1)
			/* not an error */
			ফেরত ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ফেরত false;

	} অন্যথা যদি (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->object) {
		/* find intersection of clip rectangle and box */
		যদি (r.x0 < clip->x0) r.x0 = clip->x0;
		যদি (r.y0 < clip->y0) r.y0 = clip->y0;
		যদি (clip->x1 < r.x1) r.x1 = clip->x1;
		যদি (clip->y1 < r.y1) r.y1 = clip->y1;
		/* no point trying to draw 0-width/height boxes */
		যদি (r.x0 == r.x1 || r.y0 == r.y1)
			/* not an error */
			ফেরত ((!ctx->plot->group_end) ||
				(ctx->plot->group_end(ctx) == NSERROR_OK));
		/* clip to it */
		যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ফেরত false;
	} অন্যথা {
		/* clip box is fine, clip to it */
		r = *clip;
		যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ফেরত false;
	}

	/* background colour and image for block level content and replaced
	 * inlines */

	bg_box = html_redraw_find_bg_box(box);

	/* bg_box == NULL implies that this box should not have
	* its background rendered. Otherwise filter out linebreaks,
	* optimize away non-differing inlines, only plot background
	* for BOX_TEXT it's in an inline */
	যদি (bg_box && bg_box->type != BOX_BR &&
			bg_box->type != BOX_TEXT &&
			bg_box->type != BOX_INLINE_END &&
			(bg_box->type != BOX_INLINE || bg_box->object ||
			bg_box->flags & IFRAME || box->flags & REPLACE_DIM ||
			(bg_box->gadget != ফাঁকা &&
			(bg_box->gadget->type == GADGET_TEXTAREA ||
			bg_box->gadget->type == GADGET_TEXTBOX ||
			bg_box->gadget->type == GADGET_PASSWORD)))) {
		/* find intersection of clip box and border edge */
		কাঠামো rect p;
		p.x0 = x - border_বাম < r.x0 ? r.x0 : x - border_বাম;
		p.y0 = y - border_শীর্ষ < r.y0 ? r.y0 : y - border_শীর্ষ;
		p.x1 = x + padding_width + border_ডান < r.x1 ?
				x + padding_width + border_ডান : r.x1;
		p.y1 = y + padding_height + border_bottom < r.y1 ?
				y + padding_height + border_bottom : r.y1;
		যদি (!box->parent) {
			/* Root element, special case:
			 * background covers margins too */
			পূর্ণসংখ্যা m_বাম, m_শীর্ষ, m_ডান, m_bottom;
			যদি (scale == 1.0) {
				m_বাম = box->margin[বাম_2];
				m_শীর্ষ = box->margin[শীর্ষ_2];
				m_ডান = box->margin[ডান_2];
				m_bottom = box->margin[BOTTOM];
			} অন্যথা {
				m_বাম = box->margin[বাম_2] * scale;
				m_শীর্ষ = box->margin[শীর্ষ_2] * scale;
				m_ডান = box->margin[ডান_2] * scale;
				m_bottom = box->margin[BOTTOM] * scale;
			}
			p.x0 = p.x0 - m_বাম < r.x0 ? r.x0 : p.x0 - m_বাম;
			p.y0 = p.y0 - m_শীর্ষ < r.y0 ? r.y0 : p.y0 - m_শীর্ষ;
			p.x1 = p.x1 + m_ডান < r.x1 ? p.x1 + m_ডান : r.x1;
			p.y1 = p.y1 + m_bottom < r.y1 ? p.y1 + m_bottom : r.y1;
		}
		/* valid clipping rectangles only */
		যদি ((p.x0 < p.x1) && (p.y0 < p.y1)) {
			/* plot background */
			যদি (!html_redraw_background(x, y, box, scale, &p,
					&current_background_color, bg_box,
					&html->unit_len_ctx, ctx))
				ফেরত false;
			/* restore previous graphics window */
			যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ফেরত false;
		}
	}

	/* borders for block level content and replaced inlines */
	যদি (box->style &&
	    box->type != BOX_TEXT &&
	    box->type != BOX_INLINE_END &&
	    (box->type != BOX_INLINE || box->object ||
	     box->flags & IFRAME || box->flags & REPLACE_DIM ||
	     (box->gadget != ফাঁকা &&
	      (box->gadget->type == GADGET_TEXTAREA ||
	       box->gadget->type == GADGET_TEXTBOX ||
	       box->gadget->type == GADGET_PASSWORD))) &&
	    (border_শীর্ষ || border_ডান || border_bottom || border_বাম)) {
		যদি (!html_redraw_borders(box, x_parent, y_parent,
				padding_width, padding_height, &r,
				scale, ctx))
			ফেরত false;
	}

	/* backgrounds and borders for non-replaced inlines */
	যদি (box->style && box->type == BOX_INLINE && box->inline_end &&
			(html_redraw_box_has_background(box) ||
			border_শীর্ষ || border_ডান ||
			border_bottom || border_বাম)) {
		/* inline backgrounds and borders span other boxes and may
		 * wrap onto separate lines */
		কাঠামো box *ib;
		কাঠামো rect b; /* border edge rectangle */
		কাঠামো rect p; /* clipped rect */
		bool first = true;
		পূর্ণসংখ্যা ib_x;
		পূর্ণসংখ্যা ib_y = y;
		পূর্ণসংখ্যা ib_p_width;
		পূর্ণসংখ্যা ib_b_বাম, ib_b_ডান;

		b.x0 = x - border_বাম;
		b.x1 = x + padding_width + border_ডান;
		b.y0 = y - border_শীর্ষ;
		b.y1 = y + padding_height + border_bottom;

		p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
		p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;
		p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
		p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;
		জন্য (ib = box; ib; ib = ib->পরবর্তী) {
			/* to get extents of rectangle(s) associated with
			 * inline, cycle though all boxes in inline, skipping
			 * over floats */
			যদি (ib->type == BOX_FLOAT_বাম ||
					ib->type == BOX_FLOAT_ডান)
				চালু;
			যদি (scale == 1.0) {
				ib_x = x_parent + ib->x;
				ib_y = y_parent + ib->y;
				ib_p_width = ib->padding[বাম_2] + ib->width +
						ib->padding[ডান_2];
				ib_b_বাম = ib->border[বাম_2].width;
				ib_b_ডান = ib->border[ডান_2].width;
			} অন্যথা {
				ib_x = (x_parent + ib->x) * scale;
				ib_y = (y_parent + ib->y) * scale;
				ib_p_width = (ib->padding[বাম_2] + ib->width +
						ib->padding[ডান_2]) * scale;
				ib_b_বাম = ib->border[বাম_2].width * scale;
				ib_b_ডান = ib->border[ডান_2].width * scale;
			}

			যদি ((ib->flags & NEW_সারি) && ib != box) {
				/* inline element has wrapped, plot background
				 * and borders */
				যদি (!html_redraw_inline_background(
						x, y, box, scale, &p, b,
						first, false,
						&current_background_color,
						&html->unit_len_ctx, ctx))
					ফেরত false;
				/* restore previous graphics window */
				যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
					ফেরত false;
				যদি (!html_redraw_inline_borders(box, b, &r,
						scale, first, false, ctx))
					ফেরত false;
				/* reset coords */
				b.x0 = ib_x - ib_b_বাম;
				b.y0 = ib_y - border_শীর্ষ - padding_শীর্ষ;
				b.y1 = ib_y + padding_height - padding_শীর্ষ +
						border_bottom;

				p.x0 = b.x0 < r.x0 ? r.x0 : b.x0;
				p.y0 = b.y0 < r.y0 ? r.y0 : b.y0;
				p.y1 = b.y1 < r.y1 ? b.y1 : r.y1;

				first = false;
			}

			/* increase width for current box */
			b.x1 = ib_x + ib_p_width + ib_b_ডান;
			p.x1 = b.x1 < r.x1 ? b.x1 : r.x1;

			যদি (ib == box->inline_end)
				/* reached end of BOX_INLINE span */
				বিরতি;
		}
		/* plot background and borders for last rectangle of
		 * the inline */
		যদি (!html_redraw_inline_background(x, ib_y, box, scale, &p, b,
				first, true, &current_background_color,
				&html->unit_len_ctx, ctx))
			ফেরত false;
		/* restore previous graphics window */
		যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
			ফেরত false;
		যদি (!html_redraw_inline_borders(box, b, &r, scale, first, true,
				ctx))
			ফেরত false;

	}

	/* Debug outlines */
	যদি (html_redraw_debug) {
		পূর্ণসংখ্যা margin_বাম, margin_ডান;
		পূর্ণসংখ্যা margin_শীর্ষ, margin_bottom;
		যদি (scale == 1.0) {
			/* avoid trivial fp maths */
			margin_বাম = box->margin[বাম_2];
			margin_শীর্ষ = box->margin[শীর্ষ_2];
			margin_ডান = box->margin[ডান_2];
			margin_bottom = box->margin[BOTTOM];
		} অন্যথা {
			margin_বাম = box->margin[বাম_2] * scale;
			margin_শীর্ষ = box->margin[শীর্ষ_2] * scale;
			margin_ডান = box->margin[ডান_2] * scale;
			margin_bottom = box->margin[BOTTOM] * scale;
		}
		/* Content edge -- blue */
		rect.x0 = x + padding_বাম;
		rect.y0 = y + padding_শীর্ষ;
		rect.x1 = x + padding_বাম + width;
		rect.y1 = y + padding_শীর্ষ + height;
		যদি (ctx->plot->rectangle(ctx, plot_style_content_edge, &rect) != NSERROR_OK)
			ফেরত false;

		/* Padding edge -- red */
		rect.x0 = x;
		rect.y0 = y;
		rect.x1 = x + padding_width;
		rect.y1 = y + padding_height;
		যদি (ctx->plot->rectangle(ctx, plot_style_padding_edge, &rect) != NSERROR_OK)
			ফেরত false;

		/* Margin edge -- yellow */
		rect.x0 = x - border_বাম - margin_বাম;
		rect.y0 = y - border_শীর্ষ - margin_শীর্ষ;
		rect.x1 = x + padding_width + border_ডান + margin_ডান;
		rect.y1 = y + padding_height + border_bottom + margin_bottom;
		যদি (ctx->plot->rectangle(ctx, plot_style_margin_edge, &rect) != NSERROR_OK)
			ফেরত false;
	}

	/* clip to the padding edge for objects, or boxes with overflow hidden
	 * or scroll, unless it's the root element */
	যদি (box->parent != ফাঁকা) {
		bool need_clip = false;
		যদি (box->object || box->flags & IFRAME ||
				(overflow_x != CSS_OVERFLOW_VISIBLE &&
				 overflow_y != CSS_OVERFLOW_VISIBLE)) {
			r.x0 = x;
			r.y0 = y;
			r.x1 = x + padding_width;
			r.y1 = y + padding_height;
			যদি (r.x0 < clip->x0) r.x0 = clip->x0;
			যদি (r.y0 < clip->y0) r.y0 = clip->y0;
			যদি (clip->x1 < r.x1) r.x1 = clip->x1;
			যদি (clip->y1 < r.y1) r.y1 = clip->y1;
			যদি (r.x1 <= r.x0 || r.y1 <= r.y0) {
				ফেরত (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} অন্যথা যদি (overflow_x != CSS_OVERFLOW_VISIBLE) {
			r.x0 = x;
			r.y0 = clip->y0;
			r.x1 = x + padding_width;
			r.y1 = clip->y1;
			যদি (r.x0 < clip->x0) r.x0 = clip->x0;
			যদি (clip->x1 < r.x1) r.x1 = clip->x1;
			যদি (r.x1 <= r.x0) {
				ফেরত (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;

		} অন্যথা যদি (overflow_y != CSS_OVERFLOW_VISIBLE) {
			r.x0 = clip->x0;
			r.y0 = y;
			r.x1 = clip->x1;
			r.y1 = y + padding_height;
			যদি (r.y0 < clip->y0) r.y0 = clip->y0;
			যদি (clip->y1 < r.y1) r.y1 = clip->y1;
			যদি (r.y1 <= r.y0) {
				ফেরত (!ctx->plot->group_end ||
					(ctx->plot->group_end(ctx) == NSERROR_OK));
			}
			need_clip = true;
		}

		যদি (need_clip &&
		    (box->type == BOX_BLOCK ||
		     box->type == BOX_INLINE_BLOCK ||
		     box->type == BOX_TABLE_CELL || box->object)) {
			যদি (ctx->plot->clip(ctx, &r) != NSERROR_OK)
				ফেরত false;
		}
	}

	/* text decoration */
	যদি ((box->type != BOX_TEXT) &&
	    box->style &&
	    css_computed_text_decoration(box->style) !=	CSS_TEXT_DECORATION_NONE) {
		যদি (!html_redraw_text_decoration(box, x_parent, y_parent,
				scale, current_background_color, ctx))
			ফেরত false;
	}

	যদি (box->গ্রন্থি != ফাঁকা) {
		exc = dom_html_element_get_tag_type(box->গ্রন্থি, &tag_type);
		যদি (exc != DOM_NO_ERR) {
			tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
		}
	} অন্যথা {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	যদি (box->object && width != 0 && height != 0) {
		কাঠামো content_redraw_তথ্য obj_তথ্য;

		x_scrolled = x - scrollbar_get_offset(box->scroll_x) * scale;
		y_scrolled = y - scrollbar_get_offset(box->scroll_y) * scale;

		obj_তথ্য.x = x_scrolled + padding_বাম;
		obj_তথ্য.y = y_scrolled + padding_শীর্ষ;
		obj_তথ্য.width = width;
		obj_তথ্য.height = height;
		obj_তথ্য.background_colour = current_background_color;
		obj_তথ্য.scale = scale;
		obj_তথ্য.repeat_x = false;
		obj_তথ্য.repeat_y = false;

		যদি (content_get_type(box->object) == CONTENT_HTML) {
			obj_তথ্য.x /= scale;
			obj_তথ্য.y /= scale;
		}

		যদি (!content_redraw(box->object, &obj_তথ্য, &r, ctx)) {
			/* Show image fail */
			/* Unicode (U+FFFC) 'OBJECT REPLACEMENT CHARACTER' */
			ধ্রুবক অক্ষর *obj = "\xef\xbf\xbc";
			পূর্ণসংখ্যা obj_width;
			পূর্ণসংখ্যা obj_x = x + padding_বাম;
			nserror res;

			rect.x0 = x + padding_বাম;
			rect.y0 = y + padding_শীর্ষ;
			rect.x1 = x + padding_বাম + width - 1;
			rect.y1 = y + padding_শীর্ষ + height - 1;
			res = ctx->plot->rectangle(ctx, plot_style_broken_object, &rect);
			যদি (res != NSERROR_OK) {
				ফেরত false;
			}

			res = guit->layout->width(plot_fstyle_broken_object,
						  obj,
						  আকার(obj) - 1,
						  &obj_width);
			যদি (res != NSERROR_OK) {
				obj_x += 1;
			} অন্যথা {
				obj_x += width / 2 - obj_width / 2;
			}

			যদি (ctx->plot->text(ctx,
					    plot_fstyle_broken_object,
					    obj_x, y + padding_শীর্ষ + (পূর্ণসংখ্যা)(height * 0.75),
					    obj, আকার(obj) - 1) != NSERROR_OK)
				ফেরত false;
		}
	} অন্যথা যদি (tag_type == DOM_HTML_ELEMENT_TYPE_CANVAS &&
		   box->গ্রন্থি != ফাঁকা &&
		   box->flags & REPLACE_DIM) {
		/* Canvas to draw */
		কাঠামো bitmap *bitmap = ফাঁকা;
		exc = dom_গ্রন্থি_get_user_তথ্য(box->গ্রন্থি,
					     corestring_dom_ns_চাবি_canvas_গ্রন্থি_তথ্য,
					     &bitmap);
		যদি (exc != DOM_NO_ERR) {
			bitmap = ফাঁকা;
		}
		যদি (bitmap != ফাঁকা &&
		    ctx->plot->bitmap(ctx, bitmap, x + padding_বাম, y + padding_শীর্ষ,
				      width, height, current_background_color,
				      BITMAPF_NONE) != NSERROR_OK)
			ফেরত false;
	} অন্যথা যদি (box->iframe) {
		/* Offset is passed to browser window redraw unscaled */
		browser_window_redraw(box->iframe,
				x + padding_বাম,
				y + padding_শীর্ষ, &r, ctx);

	} অন্যথা যদি (box->gadget && box->gadget->type == GADGET_CHECKBOX) {
		যদি (!html_redraw_checkbox(x + padding_বাম, y + padding_শীর্ষ,
				width, height, box->gadget->selected, ctx))
			ফেরত false;

	} অন্যথা যদি (box->gadget && box->gadget->type == GADGET_RADIO) {
		যদি (!html_redraw_radio(x + padding_বাম, y + padding_শীর্ষ,
				width, height, box->gadget->selected, ctx))
			ফেরত false;

	} অন্যথা যদি (box->gadget && box->gadget->type == GADGET_FILE) {
		যদি (!html_redraw_file(x + padding_বাম, y + padding_শীর্ষ,
				width, height, box, scale,
				current_background_color, &html->unit_len_ctx, ctx))
			ফেরত false;

	} অন্যথা যদি (box->gadget &&
			(box->gadget->type == GADGET_TEXTAREA ||
			box->gadget->type == GADGET_PASSWORD ||
			box->gadget->type == GADGET_TEXTBOX)) {
		textarea_redraw(box->gadget->তথ্য.text.ta, x, y,
				current_background_color, scale, &r, ctx);

	} অন্যথা যদি (box->text) {
		যদি (!html_redraw_text_box(html, box, x, y, &r, scale,
				current_background_color, ctx))
			ফেরত false;

	} অন্যথা {
		যদি (!html_redraw_box_children(html, box, x_parent, y_parent, &r,
				scale, current_background_color, ctx))
			ফেরত false;
	}

	যদি (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
			box->type == BOX_TABLE_CELL || box->type == BOX_INLINE)
		যদি (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ফেরত false;

	/* list marker */
	যদি (box->list_marker) {
		যদি (!html_redraw_box(html, box->list_marker,
				x_parent + box->x -
				scrollbar_get_offset(box->scroll_x),
				y_parent + box->y -
				scrollbar_get_offset(box->scroll_y),
				clip, scale, current_background_color, ctx))
			ফেরত false;
	}

	/* scrollbars */
	যদি (((box->style && box->type != BOX_BR &&
	      box->type != BOX_TABLE && box->type != BOX_INLINE &&
	      (box->gadget == ফাঁকা || box->gadget->type != GADGET_TEXTAREA) &&
	      (overflow_x == CSS_OVERFLOW_SCROLL ||
	       overflow_x == CSS_OVERFLOW_AUTO ||
	       overflow_y == CSS_OVERFLOW_SCROLL ||
	       overflow_y == CSS_OVERFLOW_AUTO)) ||
	     (box->object && content_get_type(box->object) ==
	      CONTENT_HTML)) && box->parent != ফাঁকা) {
		nserror res;
		bool has_x_scroll = (overflow_x == CSS_OVERFLOW_SCROLL);
		bool has_y_scroll = (overflow_y == CSS_OVERFLOW_SCROLL);

		has_x_scroll |= (overflow_x == CSS_OVERFLOW_AUTO) &&
				box_hscrollbar_present(box);
		has_y_scroll |= (overflow_y == CSS_OVERFLOW_AUTO) &&
				box_vscrollbar_present(box);

		res = box_handle_scrollbars((কাঠামো content *)html,
					    box, has_x_scroll, has_y_scroll);
		যদি (res != NSERROR_OK) {
			NSLOG(netsurf, INFO, "%s", messages_get_errorcode(res));
			ফেরত false;
		}

		যদি (box->scroll_x != ফাঁকা)
			scrollbar_redraw(box->scroll_x,
					x_parent + box->x,
					y_parent + box->y + box->padding[শীর্ষ_2] +
					box->height + box->padding[BOTTOM] -
					SCROLLBAR_WIDTH, clip, scale, ctx);
		যদি (box->scroll_y != ফাঁকা)
			scrollbar_redraw(box->scroll_y,
					x_parent + box->x + box->padding[বাম_2] +
					box->width + box->padding[ডান_2] -
					SCROLLBAR_WIDTH,
					y_parent + box->y, clip, scale, ctx);
	}

	যদি (box->type == BOX_BLOCK || box->type == BOX_INLINE_BLOCK ||
	    box->type == BOX_TABLE_CELL || box->type == BOX_INLINE) {
		যদি (ctx->plot->clip(ctx, clip) != NSERROR_OK)
			ফেরত false;
	}

	ফেরত ((!plot->group_end) || (ctx->plot->group_end(ctx) == NSERROR_OK));
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

bool html_redraw(কাঠামো content *c, কাঠামো content_redraw_তথ্য *তথ্য,
		ধ্রুবক কাঠামো rect *clip, ধ্রুবক কাঠামো redraw_context *ctx)
{
	html_content *html = (html_content *) c;
	কাঠামো box *box;
	bool ফলাফল = true;
	bool select, select_only;
	plot_style_t pstyle_fill_bg = {
		.fill_type = PLOT_OP_TYPE_SOLID,
		.fill_colour = তথ্য->background_colour,
	};

	box = html->layout;
	প্রতিজ্ঞা(box);

	/* The select menu needs special treating because, when opened, it
	 * reaches beyond its layout box.
	 */
	select = false;
	select_only = false;
	যদি (ctx->interactive && html->visible_select_menu != ফাঁকা) {
		কাঠামো form_control *control = html->visible_select_menu;
		select = true;
		/* check if the redraw rectangle is completely inside of the
		   select menu */
		select_only = form_clip_inside_select_menu(control,
				তথ্য->scale, clip);
	}

	যদি (!select_only) {
		/* clear to background colour */
		ফলাফল = (ctx->plot->clip(ctx, clip) == NSERROR_OK);

		যদি (html->background_colour != NS_TRANSPARENT)
			pstyle_fill_bg.fill_colour = html->background_colour;

		ফলাফল &= (ctx->plot->rectangle(ctx, &pstyle_fill_bg, clip) == NSERROR_OK);

		ফলাফল &= html_redraw_box(html, box, তথ্য->x, তথ্য->y, clip,
				তথ্য->scale, pstyle_fill_bg.fill_colour, ctx);
	}

	যদি (select) {
		পূর্ণসংখ্যা menu_x, menu_y;
		box = html->visible_select_menu->box;
		box_coords(box, &menu_x, &menu_y);

		menu_x -= box->border[বাম_2].width;
		menu_y += box->height + box->border[BOTTOM].width +
				box->padding[BOTTOM] + box->padding[শীর্ষ_2];
		ফলাফল &= form_redraw_select_menu(html->visible_select_menu,
				তথ্য->x + menu_x, তথ্য->y + menu_y,
				তথ্য->scale, clip, ctx);
	}

	ফেরত ফলাফল;

}
