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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/utils.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/errors.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/content.h"
#ಅನ್ತರ್ಭಾವ "netsurf/mouse.h"
#ಅನ್ತರ್ಭಾವ "css/utils.h"
#ಅನ್ತರ್ಭಾವ "css/dump.h"
#ಅನ್ತರ್ಭಾವ "desktop/scrollbar.h"

#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
ಗಣನಾ box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_ಎಡ ||	\
			   box->type == BOX_FLOAT_ಬಲ)

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
ಸ್ಥಿರ bool
box_contains_point(ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		   ಧ್ರುವ ಸಂರಚನಾ box *box,
		   ಪೂರ್ಣಾಙ್ಕ x,
		   ಪೂರ್ಣಾಙ್ಕ y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ಯದಿ (box->style != ರಿಕ್ತ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ಸಂರಚನಾ rect r = {
				 .x0 = box->border[ಎಡ_2].width,
				 .y0 = box->border[ತುದಿ_2].width,
				 .x1 = box->padding[ಎಡ_2] + box->width +
				 box->border[ಬಲ_2].width +
				 box->padding[ಬಲ_2],
				 .y1 = box->padding[ತುದಿ_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		ಯದಿ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ಅನ್ಯಥಾ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ಯದಿ (css_rect.ಎಡ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ಎಡ,
						css_rect.lunit));
		}
		ಯದಿ (css_rect.ತುದಿ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ತುದಿ,
						css_rect.tunit));
		}
		ಯದಿ (css_rect.ಬಲ_auto == false) {
			r.x1 = box->border[ಎಡ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ಬಲ,
						css_rect.runit));
		}
		ಯದಿ (css_rect.bottom_auto == false) {
			r.y1 = box->border[ತುದಿ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ಯದಿ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ಪ್ರತ್ಯಾಗಮ true;
		}

		/* Not inside clip area */
		ಪ್ರತ್ಯಾಗಮ false;
	}
	ಯದಿ (x >= -box->border[ಎಡ_2].width &&
	    x < box->padding[ಎಡ_2] + box->width +
	    box->padding[ಬಲ_2] + box->border[ಬಲ_2].width &&
	    y >= -box->border[ತುದಿ_2].width &&
	    y < box->padding[ತುದಿ_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		ಪ್ರತ್ಯಾಗಮ true;
	}
	ಯದಿ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ಎಡ_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ಎಡ_2] +
	    box->list_marker->width +
	    box->list_marker->border[ಬಲ_2].width +
	    box->list_marker->padding[ಬಲ_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[ತುದಿ_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[ತುದಿ_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ಪ್ರತ್ಯಾಗಮ true;
	}
	ಯದಿ ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ಯದಿ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}
	ಯದಿ ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ಯದಿ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ಪ್ರತ್ಯಾಗಮ true;
		}
	}
	ಪ್ರತ್ಯಾಗಮ false;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ box *
box_move_xy(ಸಂರಚನಾ box *b, ಗಣನಾ box_walk_dir dir, ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y)
{
	ಸಂರಚನಾ box *rb = ರಿಕ್ತ;

	ವಿಕಲ್ಪ (dir) {
	ಪಕ್ಷ BOX_WALK_CHILDREN:
		b = b->children;
		ಯದಿ (b == ರಿಕ್ತ)
			ಭಙ್ಗ;
		*x += b->x;
		*y += b->y;
		ಯದಿ (!box_is_float(b)) {
			rb = b;
			ಭಙ್ಗ;
		}
		fallthrough;

	ಪಕ್ಷ BOX_WALK_NEXT_SIBLING:
		ಕುರು {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			ಯದಿ (b == ರಿಕ್ತ)
				ಭಙ್ಗ;
			*x += b->x;
			*y += b->y;
		} ಯಾವತ್ (box_is_float(b));
		rb = b;
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ಯದಿ (b == ರಿಕ್ತ)
			ಭಙ್ಗ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		ಯದಿ (b == ರಿಕ್ತ)
			ಭಙ್ಗ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		ಪ್ರತಿಜ್ಞಾ(0 && "Bad box walk type.");
	}

	ಪ್ರತ್ಯಾಗಮ rb;
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ box *
box_next_xy(ಸಂರಚನಾ box *b, ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y, bool skip_children)
{
	ಸಂರಚನಾ box *n;
	ಪೂರ್ಣಾಙ್ಕ tx, ty;

	ಪ್ರತಿಜ್ಞಾ(b != ರಿಕ್ತ);

	ಯದಿ (skip_children) {
		/* Caller is not interested in any kind of children */
		ಗಚ್ಛ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ಯದಿ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ಪ್ರತ್ಯಾಗಮ n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ಯದಿ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ಪ್ರತ್ಯಾಗಮ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	ಯದಿ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ಪ್ರತ್ಯಾಗಮ n;
	}

	ಯದಿ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		ಗಚ್ಛ done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ಯಾವತ್ (b) {
		ಯಾವತ್ (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ಯದಿ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ಗಚ್ಛ skip_children;
			}
		}
		ಯದಿ (!b->next) {
			/* No more boxes */
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		ಯದಿ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ಪ್ರತ್ಯಾಗಮ n;

		} ಅನ್ಯಥಾ ಯದಿ (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ಯದಿ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ಗಚ್ಛ skip_children;
			}

		} ಅನ್ಯಥಾ {
			/* No more boxes */
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	}

	ಪ್ರತಿಜ್ಞಾ(b != ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
ಸ್ಥಿರ bool
box_nearer_text_box(ಸಂರಚನಾ box *box,
		    ಪೂರ್ಣಾಙ್ಕ bx, ಪೂರ್ಣಾಙ್ಕ by,
		    ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		    ಪೂರ್ಣಾಙ್ಕ dir,
		    ಸಂರಚನಾ box **nearest,
		    ಪೂರ್ಣಾಙ್ಕ *tx, ಪೂರ್ಣಾಙ್ಕ *ty,
		    ಪೂರ್ಣಾಙ್ಕ *nr_xd, ಪೂರ್ಣಾಙ್ಕ *nr_yd)
{
	ಪೂರ್ಣಾಙ್ಕ w = box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2];
	ಪೂರ್ಣಾಙ್ಕ h = box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM];
	ಪೂರ್ಣಾಙ್ಕ y1 = by + h;
	ಪೂರ್ಣಾಙ್ಕ x1 = bx + w;
	ಪೂರ್ಣಾಙ್ಕ yd = INT_MAX;
	ಪೂರ್ಣಾಙ್ಕ xd = INT_MAX;

	ಯದಿ (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಯದಿ (box->parent->list_marker != box) {
		ಯದಿ (dir < 0) {
			/* consider only those children (partly) above-left */
			ಯದಿ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ಅನ್ಯಥಾ {
			/* consider only those children (partly) below-right */
			ಯದಿ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ಯದಿ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	ಪ್ರತ್ಯಾಗಮ false;
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
ಸ್ಥಿರ bool
box_nearest_text_box(ಸಂರಚನಾ box *box,
		     ಪೂರ್ಣಾಙ್ಕ bx, ಪೂರ್ಣಾಙ್ಕ by,
		     ಪೂರ್ಣಾಙ್ಕ fx, ಪೂರ್ಣಾಙ್ಕ fy,
		     ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		     ಪೂರ್ಣಾಙ್ಕ dir,
		     ಸಂರಚನಾ box **nearest,
		     ಪೂರ್ಣಾಙ್ಕ *tx, ಪೂರ್ಣಾಙ್ಕ *ty,
		     ಪೂರ್ಣಾಙ್ಕ *nr_xd, ಪೂರ್ಣಾಙ್ಕ *nr_yd)
{
	ಸಂರಚನಾ box *child = box->children;
	ಪೂರ್ಣಾಙ್ಕ c_bx, c_by;
	ಪೂರ್ಣಾಙ್ಕ c_fx, c_fy;
	bool in_box = false;

	ಯದಿ (*nearest == ರಿಕ್ತ) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	ಯದಿ (box->type == BOX_INLINE_CONTAINER) {
		ಪೂರ್ಣಾಙ್ಕ bw = box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2];
		ಪೂರ್ಣಾಙ್ಕ bh = box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM];
		ಪೂರ್ಣಾಙ್ಕ b_y1 = by + bh;
		ಪೂರ್ಣಾಙ್ಕ b_x1 = bx + bw;
		ಯದಿ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	ಯಾವತ್ (child) {
		ಯದಿ (child->type == BOX_FLOAT_ಎಡ ||
		    child->type == BOX_FLOAT_ಬಲ) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} ಅನ್ಯಥಾ {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		ಯದಿ (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ಅನ್ಯಥಾ {
			c_fx = fx;
			c_fy = fy;
		}
		ಯದಿ (in_box && child->text && !child->object) {
			ಯದಿ (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				ಪ್ರತ್ಯಾಗಮ true;
		} ಅನ್ಯಥಾ {
			ಯದಿ (child->list_marker) {
				ಯದಿ (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					ಪ್ರತ್ಯಾಗಮ true;
			}
			ಯದಿ (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				ಪ್ರತ್ಯಾಗಮ true;
		}
		child = child->next;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_coords(ಸಂರಚನಾ box *box, ಪೂರ್ಣಾಙ್ಕ *x, ಪೂರ್ಣಾಙ್ಕ *y)
{
	*x = box->x;
	*y = box->y;
	ಯಾವತ್ (box->parent) {
		ಯದಿ (box_is_float(box)) {
			ಪ್ರತಿಜ್ಞಾ(box->float_container);
			box = box->float_container;
		} ಅನ್ಯಥಾ {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_bounds(ಸಂರಚನಾ box *box, ಸಂರಚನಾ rect *r)
{
	ಪೂರ್ಣಾಙ್ಕ width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[ಎಡ_2] + box->width + box->padding[ಬಲ_2];
	height = box->padding[ತುದಿ_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
ಸಂರಚನಾ box *
box_at_point(ಧ್ರುವ css_unit_ctx *unit_len_ctx,
	     ಸಂರಚನಾ box *box,
	     ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ x, ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ y,
	     ಪೂರ್ಣಾಙ್ಕ *box_x, ಪೂರ್ಣಾಙ್ಕ *box_y)
{
	bool skip_children;
	bool physically;

	ಪ್ರತಿಜ್ಞಾ(box);

	skip_children = false;
	ಯಾವತ್ ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		ಯದಿ (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			ಯದಿ (physically)
				ಪ್ರತ್ಯಾಗಮ box;

			skip_children = false;
		} ಅನ್ಯಥಾ {
			skip_children = true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* Exported function documented in html/box.h */
ಸಂರಚನಾ box *box_find_by_id(ಸಂರಚನಾ box *box, lwc_string *id)
{
	ಸಂರಚನಾ box *a, *b;
	bool m;

	ಯದಿ (box->id != ರಿಕ್ತ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ಪ್ರತ್ಯಾಗಮ box;
	}

	ಪ್ರತಿ (a = box->children; a; a = a->next) {
		ಯದಿ ((b = box_find_by_id(a, id)) != ರಿಕ್ತ) {
			ಪ್ರತ್ಯಾಗಮ b;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/* Exported function documented in html/box.h */
bool box_visible(ಸಂರಚನಾ box *box)
{
	/* visibility: hidden */
	ಯದಿ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_dump(FILE *stream, ಸಂರಚನಾ box *box, unsigned ಪೂರ್ಣಾಙ್ಕ depth, bool style)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ box *c, *prev;

	ಪ್ರತಿ (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	ಯದಿ (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[ತುದಿ_2], box->margin[ಎಡ_2],
		box->margin[BOTTOM], box->margin[ಬಲ_2]);

	ವಿಕಲ್ಪ (box->type) {
	ಪಕ್ಷ BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_INLINE:
		fprintf(stream, "INLINE ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_FLOAT_ಎಡ:
		fprintf(stream, "FLOAT_LEFT ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_FLOAT_ಬಲ:
		fprintf(stream, "FLOAT_RIGHT ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_BR:
		fprintf(stream, "BR ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_TEXT:
		fprintf(stream, "TEXT ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_FLEX:
		fprintf(stream, "FLEX ");
		ಭಙ್ಗ;

	ಪಕ್ಷ BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		fprintf(stream, "Unknown box type ");
	}

	ಯದಿ (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(ಪೂರ್ಣಾಙ್ಕ) box->length, box->text);
	ಯದಿ (box->space)
		fprintf(stream, "space ");
	ಯದಿ (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ಯದಿ (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	ಯದಿ (box->gadget)
		fprintf(stream, "(gadget) ");
	ಯದಿ (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	ಯದಿ (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	ಯದಿ (box->target)
		fprintf(stream, " |%s|", box->target);
	ಯದಿ (box->title)
		fprintf(stream, " [%s]", box->title);
	ಯದಿ (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	ಯದಿ (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	ಯದಿ (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	ಯದಿ (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	ಯದಿ (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	ಯದಿ (box->col) {
		fprintf(stream, " (columns");
		ಪ್ರತಿ (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ಧ್ರುವ ವರ್ಣ *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ಧ್ರುವ ವರ್ಣ *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	ಯದಿ (box->ಗಂಟು != ರಿಕ್ತ) {
		dom_string *name;
		ಯದಿ (dom_ಗಂಟು_get_ಗಂಟು_name(box->ಗಂಟು, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	ಯದಿ (box->list_marker) {
		ಪ್ರತಿ (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	ಪ್ರತಿ (c = box->children; c && c->next; c = c->next)
		;
	ಯದಿ (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	ಪ್ರತಿ (prev = 0, c = box->children; c; prev = c, c = c->next) {
		ಯದಿ (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		ಯದಿ (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ಧ್ರುವ ಸಂರಚನಾ box * ಧ್ರುವ box)
{
	ಪ್ರತ್ಯಾಗಮ box->padding[ತುದಿ_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ಧ್ರುವ ಸಂರಚನಾ box * ಧ್ರುವ box)
{
	ಪ್ರತ್ಯಾಗಮ box->padding[ಎಡ_2] +
		box->width +
		box->padding[ಬಲ_2] +
		box->border[ಬಲ_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ಸಂರಚನಾ box *
box_pick_text_box(ಸಂರಚನಾ html_content *html,
		  ಪೂರ್ಣಾಙ್ಕ x, ಪೂರ್ಣಾಙ್ಕ y,
		  ಪೂರ್ಣಾಙ್ಕ dir,
		  ಪೂರ್ಣಾಙ್ಕ *dx, ಪೂರ್ಣಾಙ್ಕ *dy)
{
	ಸಂರಚನಾ box *text_box = ರಿಕ್ತ;
	ಸಂರಚನಾ box *box;
	ಪೂರ್ಣಾಙ್ಕ nr_xd, nr_yd;
	ಪೂರ್ಣಾಙ್ಕ bx, by;
	ಪೂರ್ಣಾಙ್ಕ fx, fy;
	ಪೂರ್ಣಾಙ್ಕ tx, ty;

	ಯದಿ (html == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	box = html->layout;
	bx = box->margin[ಎಡ_2];
	by = box->margin[ತುದಿ_2];
	fx = bx;
	fy = by;

	ಯದಿ (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ಯದಿ (text_box && text_box->text && !text_box->object) {
			ಪೂರ್ಣಾಙ್ಕ w = (text_box->padding[ಎಡ_2] +
				 text_box->width +
				 text_box->padding[ಬಲ_2]);
			ಪೂರ್ಣಾಙ್ಕ h = (text_box->padding[ತುದಿ_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			ಪೂರ್ಣಾಙ್ಕ x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ಯದಿ (x < tx) x = tx;
			ಯದಿ (y < ty) y = ty;
			ಯದಿ (y > y1) y = y1;
			ಯದಿ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ಪ್ರತ್ಯಾಗಮ text_box;
}
