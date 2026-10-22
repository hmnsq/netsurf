/*
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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
 * implementation of box tree inspection.
 */

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/utils.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "utils/errors.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/types.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/content.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/mouse.h"
#ଅନ୍ତର୍ଭାଵ "css/utils.h"
#ଅନ୍ତର୍ଭାଵ "css/dump.h"
#ଅନ୍ତର୍ଭାଵ "desktop/scrollbar.h"

#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
ଗଣନା box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_ବାମ ||	\
			   box->type == BOX_FLOAT_ଡାହାଣ)

/**
 * Determine if a point lies within a box.
 *
 * \param[in]  unit_len_ctx     CSS length conversion context to use.
 * \param[in]  box         Box to consider
 * \param[in]  x           Coordinate relative to box
 * \param[in]  y           Coordinate relative to box
 * \param[out] physically  If function returning true, physically is set true
 *                         iff point is within the box's physical dimensions and
 *                         false if the point is not within the box's physical
 *                         dimensions but is in the area defined by the box's
 *                         descendants.  If function returns false, physically
 *                         is undefined.
 * \return  true if the point is within the box or a descendant box
 *
 * This is a helper function for box_at_point().
 */
ସ୍ଥିର bool
box_contains_point(ଅଚଳ css_unit_ctx *unit_len_ctx,
		   ଅଚଳ ସଂରଚନା box *box,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା x,
		   ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ଯଦି (box->style != ଖାଲି &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ସଂରଚନା rect r = {
				 .x0 = box->border[ବାମ_2].width,
				 .y0 = box->border[ଶୀର୍ଷ_2].width,
				 .x1 = box->padding[ବାମ_2] + box->width +
				 box->border[ଡାହାଣ_2].width +
				 box->padding[ଡାହାଣ_2],
				 .y1 = box->padding[ଶୀର୍ଷ_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		ଯଦି (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ଅନ୍ୟଥା {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ଯଦି (css_rect.ବାମ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ବାମ,
						css_rect.lunit));
		}
		ଯଦି (css_rect.ଶୀର୍ଷ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ଶୀର୍ଷ,
						css_rect.tunit));
		}
		ଯଦି (css_rect.ଡାହାଣ_auto == false) {
			r.x1 = box->border[ବାମ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ଡାହାଣ,
						css_rect.runit));
		}
		ଯଦି (css_rect.bottom_auto == false) {
			r.y1 = box->border[ଶୀର୍ଷ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ଯଦି (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ଫେରନ୍ତୁ true;
		}

		/* Not inside clip area */
		ଫେରନ୍ତୁ false;
	}
	ଯଦି (x >= -box->border[ବାମ_2].width &&
	    x < box->padding[ବାମ_2] + box->width +
	    box->padding[ଡାହାଣ_2] + box->border[ଡାହାଣ_2].width &&
	    y >= -box->border[ଶୀର୍ଷ_2].width &&
	    y < box->padding[ଶୀର୍ଷ_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		ଫେରନ୍ତୁ true;
	}
	ଯଦି (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ବାମ_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ବାମ_2] +
	    box->list_marker->width +
	    box->list_marker->border[ଡାହାଣ_2].width +
	    box->list_marker->padding[ଡାହାଣ_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[ଶୀର୍ଷ_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[ଶୀର୍ଷ_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ଫେରନ୍ତୁ true;
	}
	ଯଦି ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ଯଦି (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ଫେରନ୍ତୁ true;
		}
	}
	ଯଦି ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ଯଦି (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ଫେରନ୍ତୁ true;
		}
	}
	ଫେରନ୍ତୁ false;
}


/**
 * Move from box to next box in given direction, adjusting for box coord change
 *
 * \param b box to move from from
 * \param dir direction to move in
 * \param x box's global x-coord, updated to position of next box
 * \param y box's global y-coord, updated to position of next box
 *
 * If no box can be found in given direction, NULL is returned.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା box *
box_move_xy(ସଂରଚନା box *b, ଗଣନା box_walk_dir dir, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y)
{
	ସଂରଚନା box *rb = ଖାଲି;

	ବାଛ (dir) {
	ସ୍ଥିତି BOX_WALK_CHILDREN:
		b = b->children;
		ଯଦି (b == ଖାଲି)
			ଭାଙ୍ଗ;
		*x += b->x;
		*y += b->y;
		ଯଦି (!box_is_float(b)) {
			rb = b;
			ଭାଙ୍ଗ;
		}
		fallthrough;

	ସ୍ଥିତି BOX_WALK_NEXT_SIBLING:
		କର {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			ଯଦି (b == ଖାଲି)
				ଭାଙ୍ଗ;
			*x += b->x;
			*y += b->y;
		} ଯେତେବେଳେ (box_is_float(b));
		rb = b;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ଯଦି (b == ଖାଲି)
			ଭାଙ୍ଗ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		ଯଦି (b == ଖାଲି)
			ଭାଙ୍ଗ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ପ୍ରତିଜ୍ଞା(0 && "Bad box walk type.");
	}

	ଫେରନ୍ତୁ rb;
}


/**
 * Itterator for walking to next box in interaction order
 *
 * \param b	box to find next box from
 * \param x	box's global x-coord, updated to position of next box
 * \param y	box's global y-coord, updated to position of next box
 * \param skip_children	whether to skip box's children
 *
 * This walks to a boxes float children before its children.  When walking
 * children, floating boxes are skipped.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା box *
box_next_xy(ସଂରଚନା box *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y, bool skip_children)
{
	ସଂରଚନା box *n;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tx, ty;

	ପ୍ରତିଜ୍ଞା(b != ଖାଲି);

	ଯଦି (skip_children) {
		/* Caller is not interested in any kind of children */
		ଯାଅ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ଯଦି (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ଫେରନ୍ତୁ n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ଯଦି (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ଫେରନ୍ତୁ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	ଯଦି (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ଫେରନ୍ତୁ n;
	}

	ଯଦି (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		ଯାଅ done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ଯେତେବେଳେ (b) {
		ଯେତେବେଳେ (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ଯଦି (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ଯାଅ skip_children;
			}
		}
		ଯଦି (!b->next) {
			/* No more boxes */
			ଫେରନ୍ତୁ ଖାଲି;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		ଯଦି (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ଫେରନ୍ତୁ n;

		} ଅନ୍ୟଥା ଯଦି (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ଯଦି (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ଯାଅ skip_children;
			}

		} ଅନ୍ୟଥା {
			/* No more boxes */
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}

	ପ୍ରତିଜ୍ଞା(b != ଖାଲି);
	ଫେରନ୍ତୁ ଖାଲି;
}


/**
 * Check whether box is nearer mouse coordinates than current nearest box
 *
 * \param  box      box to test
 * \param  bx	    position of box, in global document coordinates
 * \param  by	    position of box, in global document coordinates
 * \param  x	    mouse point, in global document coordinates
 * \param  y	    mouse point, in global document coordinates
 * \param  dir      direction in which to search (-1 = above-left,
 *						  +1 = below-right)
 * \param  nearest  nearest text box found, or NULL if none
 *		    updated if box is nearer than existing nearest
 * \param  tx	    position of text_box, in global document coordinates
 *		    updated if box is nearer than existing nearest
 * \param  ty	    position of text_box, in global document coordinates
 *		    updated if box is nearer than existing nearest
 * \param  nr_xd    distance to nearest text box found
 *		    updated if box is nearer than existing nearest
 * \param  nr_yd    distance to nearest text box found
 *		    updated if box is nearer than existing nearest
 * \return true if mouse point is inside box
 */
ସ୍ଥିର bool
box_nearer_text_box(ସଂରଚନା box *box,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା by,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir,
		    ସଂରଚନା box **nearest,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ty,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nr_xd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nr_yd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା w = box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା y1 = by + h;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x1 = bx + w;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା yd = INT_MAX;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା xd = INT_MAX;

	ଯଦି (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		ଫେରନ୍ତୁ true;
	}

	ଯଦି (box->parent->list_marker != box) {
		ଯଦି (dir < 0) {
			/* consider only those children (partly) above-left */
			ଯଦି (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ଅନ୍ୟଥା {
			/* consider only those children (partly) below-right */
			ଯଦି (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ଯଦି (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	ଫେରନ୍ତୁ false;
}


/**
 * Pick the text box child of 'box' that is closest to and above-left
 * (dir -ve) or below-right (dir +ve) of the point 'x,y'
 *
 * \param  box      parent box
 * \param  bx	    position of box, in global document coordinates
 * \param  by	    position of box, in global document coordinates
 * \param  fx	    position of float parent, in global document coordinates
 * \param  fy	    position of float parent, in global document coordinates
 * \param  x	    mouse point, in global document coordinates
 * \param  y	    mouse point, in global document coordinates
 * \param  dir      direction in which to search (-1 = above-left,
 *						  +1 = below-right)
 * \param  nearest  nearest text box found, or NULL if none
 *		    updated if a descendant of box is nearer than old nearest
 * \param  tx	    position of nearest, in global document coordinates
 *		    updated if a descendant of box is nearer than old nearest
 * \param  ty	    position of nearest, in global document coordinates
 *		    updated if a descendant of box is nearer than old nearest
 * \param  nr_xd    distance to nearest text box found
 *		    updated if a descendant of box is nearer than old nearest
 * \param  nr_yd    distance to nearest text box found
 *		    updated if a descendant of box is nearer than old nearest
 * \return true if mouse point is inside text_box
 */
ସ୍ଥିର bool
box_nearest_text_box(ସଂରଚନା box *box,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା bx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା by,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା fx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fy,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir,
		     ସଂରଚନା box **nearest,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ty,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nr_xd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nr_yd)
{
	ସଂରଚନା box *child = box->children;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c_bx, c_by;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c_fx, c_fy;
	bool in_box = false;

	ଯଦି (*nearest == ଖାଲି) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	ଯଦି (box->type == BOX_INLINE_CONTAINER) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା bw = box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା bh = box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_y1 = by + bh;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_x1 = bx + bw;
		ଯଦି (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	ଯେତେବେଳେ (child) {
		ଯଦି (child->type == BOX_FLOAT_ବାମ ||
		    child->type == BOX_FLOAT_ଡାହାଣ) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} ଅନ୍ୟଥା {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		ଯଦି (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ଅନ୍ୟଥା {
			c_fx = fx;
			c_fy = fy;
		}
		ଯଦି (in_box && child->text && !child->object) {
			ଯଦି (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				ଫେରନ୍ତୁ true;
		} ଅନ୍ୟଥା {
			ଯଦି (child->list_marker) {
				ଯଦି (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					ଫେରନ୍ତୁ true;
			}
			ଯଦି (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				ଫେରନ୍ତୁ true;
		}
		child = child->next;
	}

	ଫେରନ୍ତୁ false;
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_coords(ସଂରଚନା box *box, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *y)
{
	*x = box->x;
	*y = box->y;
	ଯେତେବେଳେ (box->parent) {
		ଯଦି (box_is_float(box)) {
			ପ୍ରତିଜ୍ଞା(box->float_container);
			box = box->float_container;
		} ଅନ୍ୟଥା {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_bounds(ସଂରଚନା box *box, ସଂରଚନା rect *r)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[ବାମ_2] + box->width + box->padding[ଡାହାଣ_2];
	height = box->padding[ଶୀର୍ଷ_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
ସଂରଚନା box *
box_at_point(ଅଚଳ css_unit_ctx *unit_len_ctx,
	     ସଂରଚନା box *box,
	     ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
	     ପୂର୍ଣ୍ଣସଂଖ୍ୟା *box_x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *box_y)
{
	bool skip_children;
	bool physically;

	ପ୍ରତିଜ୍ଞା(box);

	skip_children = false;
	ଯେତେବେଳେ ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		ଯଦି (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			ଯଦି (physically)
				ଫେରନ୍ତୁ box;

			skip_children = false;
		} ଅନ୍ୟଥା {
			skip_children = true;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}


/* Exported function documented in html/box.h */
ସଂରଚନା box *box_find_by_id(ସଂରଚନା box *box, lwc_string *id)
{
	ସଂରଚନା box *a, *b;
	bool m;

	ଯଦି (box->id != ଖାଲି &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ଫେରନ୍ତୁ box;
	}

	ପାଇଁ (a = box->children; a; a = a->next) {
		ଯଦି ((b = box_find_by_id(a, id)) != ଖାଲି) {
			ଫେରନ୍ତୁ b;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}


/* Exported function documented in html/box.h */
bool box_visible(ସଂରଚନା box *box)
{
	/* visibility: hidden */
	ଯଦି (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}


/* Exported function documented in html/box.h */
ଶୂନ୍ୟ box_dump(FILE *stream, ସଂରଚନା box *box, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା depth, bool style)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା box *c, *prev;

	ପାଇଁ (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	ଯଦି (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[ଶୀର୍ଷ_2], box->margin[ବାମ_2],
		box->margin[BOTTOM], box->margin[ଡାହାଣ_2]);

	ବାଛ (box->type) {
	ସ୍ଥିତି BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_INLINE:
		fprintf(stream, "INLINE ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_FLOAT_ବାମ:
		fprintf(stream, "FLOAT_LEFT ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_FLOAT_ଡାହାଣ:
		fprintf(stream, "FLOAT_RIGHT ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_BR:
		fprintf(stream, "BR ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_TEXT:
		fprintf(stream, "TEXT ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_FLEX:
		fprintf(stream, "FLEX ");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		fprintf(stream, "Unknown box type ");
	}

	ଯଦି (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) box->length, box->text);
	ଯଦି (box->space)
		fprintf(stream, "space ");
	ଯଦି (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ଯଦି (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	ଯଦି (box->gadget)
		fprintf(stream, "(gadget) ");
	ଯଦି (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	ଯଦି (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	ଯଦି (box->target)
		fprintf(stream, " |%s|", box->target);
	ଯଦି (box->title)
		fprintf(stream, " [%s]", box->title);
	ଯଦି (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	ଯଦି (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	ଯଦି (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	ଯଦି (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	ଯଦି (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	ଯଦି (box->col) {
		fprintf(stream, " (columns");
		ପାଇଁ (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ଅଚଳ ଅକ୍ଷର *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ଅଚଳ ଅକ୍ଷର *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	ଯଦି (box->ଗଣ୍ଠି != ଖାଲି) {
		dom_string *name;
		ଯଦି (dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(box->ଗଣ୍ଠି, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	ଯଦି (box->list_marker) {
		ପାଇଁ (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	ପାଇଁ (c = box->children; c && c->next; c = c->next)
		;
	ଯଦି (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	ପାଇଁ (prev = 0, c = box->children; c; prev = c, c = c->next) {
		ଯଦି (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		ଯଦି (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ଅଚଳ ସଂରଚନା box * ଅଚଳ box)
{
	ଫେରନ୍ତୁ box->padding[ଶୀର୍ଷ_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ଅଚଳ ସଂରଚନା box * ଅଚଳ box)
{
	ଫେରନ୍ତୁ box->padding[ବାମ_2] +
		box->width +
		box->padding[ଡାହାଣ_2] +
		box->border[ଡାହାଣ_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ସଂରଚନା box *
box_pick_text_box(ସଂରଚନା html_content *html,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dy)
{
	ସଂରଚନା box *text_box = ଖାଲି;
	ସଂରଚନା box *box;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nr_xd, nr_yd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା bx, by;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fx, fy;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tx, ty;

	ଯଦି (html == ଖାଲି)
		ଫେରନ୍ତୁ ଖାଲି;

	box = html->layout;
	bx = box->margin[ବାମ_2];
	by = box->margin[ଶୀର୍ଷ_2];
	fx = bx;
	fy = by;

	ଯଦି (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ଯଦି (text_box && text_box->text && !text_box->object) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା w = (text_box->padding[ବାମ_2] +
				 text_box->width +
				 text_box->padding[ଡାହାଣ_2]);
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = (text_box->padding[ଶୀର୍ଷ_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ଯଦି (x < tx) x = tx;
			ଯଦି (y < ty) y = ty;
			ଯଦି (y > y1) y = y1;
			ଯଦି (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ଫେରନ୍ତୁ text_box;
}
