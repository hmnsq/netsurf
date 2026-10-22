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
ಎಣಿಕೆ box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_ಪೋಷಕ,
		   BOX_WALK_ಮುಂದಿನ_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_ಮುಂದಿನ_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#ವ್ಯಾಖ್ಯಾನಿಸು box_is_float(box) (box->type == BOX_FLOAT_ಎಡ ||	\
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
box_ಒಳಗೊಂಡಿದೆ_ಬಿಂದು(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		   ಸ್ಥಿರಾಂಕ ರಚನೆ box *box,
		   ಪೂರ್ಣಾಂಕ x,
		   ಪೂರ್ಣಾಂಕ y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ಆದರೆ (box->style != ಖಾಲಿ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ರಚನೆ rect r = {
				 .x0 = box->border[ಎಡ_2].ಅಗಲ,
				 .y0 = box->border[ತುದಿ_2].ಅಗಲ,
				 .x1 = box->padding[ಎಡ_2] + box->ಅಗಲ +
				 box->border[ಬಲ_2].ಅಗಲ +
				 box->padding[ಬಲ_2],
				 .y1 = box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
				 box->border[BOTTOM].ಅಗಲ +
				 box->padding[BOTTOM]
		};
		ಆದರೆ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ಆದರೆ (css_rect.ಎಡ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ಎಡ,
						css_rect.lunit));
		}
		ಆದರೆ (css_rect.ತುದಿ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ತುದಿ,
						css_rect.tunit));
		}
		ಆದರೆ (css_rect.ಬಲ_auto == false) {
			r.x1 = box->border[ಎಡ_2].ಅಗಲ +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ಬಲ,
						css_rect.runit));
		}
		ಆದರೆ (css_rect.bottom_auto == false) {
			r.y1 = box->border[ತುದಿ_2].ಅಗಲ +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ಆದರೆ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ಹಿಂತಿರುಗಿ true;
		}

		/* Not inside clip area */
		ಹಿಂತಿರುಗಿ false;
	}
	ಆದರೆ (x >= -box->border[ಎಡ_2].ಅಗಲ &&
	    x < box->padding[ಎಡ_2] + box->ಅಗಲ +
	    box->padding[ಬಲ_2] + box->border[ಬಲ_2].ಅಗಲ &&
	    y >= -box->border[ತುದಿ_2].ಅಗಲ &&
	    y < box->padding[ತುದಿ_2] + box->ಎತ್ತರ +
	    box->padding[BOTTOM] + box->border[BOTTOM].ಅಗಲ) {
		*physically = true;
		ಹಿಂತಿರುಗಿ true;
	}
	ಆದರೆ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ಎಡ_2].ಅಗಲ &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ಎಡ_2] +
	    box->list_marker->ಅಗಲ +
	    box->list_marker->border[ಬಲ_2].ಅಗಲ +
	    box->list_marker->padding[ಬಲ_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[ತುದಿ_2].ಅಗಲ &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[ತುದಿ_2] +
	    box->list_marker->ಎತ್ತರ +
	    box->list_marker->border[BOTTOM].ಅಗಲ +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ಹಿಂತಿರುಗಿ true;
	}
	ಆದರೆ ((box->style && css_computed_ಅತಿಪ್ರವಾಹ_x(box->style) ==
	     CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) || !box->style) {
		ಆದರೆ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ಹಿಂತಿರುಗಿ true;
		}
	}
	ಆದರೆ ((box->style && css_computed_ಅತಿಪ್ರವಾಹ_y(box->style) ==
	     CSS_ಅತಿಪ್ರವಾಹ_VISIBLE) || !box->style) {
		ಆದರೆ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ಹಿಂತಿರುಗಿ true;
		}
	}
	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ box *
box_move_xy(ರಚನೆ box *b, ಎಣಿಕೆ box_walk_dir dir, ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y)
{
	ರಚನೆ box *rb = ಖಾಲಿ;

	ಆಯ್ಕೆ (dir) {
	ಸ್ಥಿತಿ BOX_WALK_CHILDREN:
		b = b->children;
		ಆದರೆ (b == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;
		*x += b->x;
		*y += b->y;
		ಆದರೆ (!box_is_float(b)) {
			rb = b;
			ನಿಲ್ಲಿಸು;
		}
		fallthrough;

	ಸ್ಥಿತಿ BOX_WALK_ಮುಂದಿನ_SIBLING:
		ಮಾಡು {
			*x -= b->x;
			*y -= b->y;
			b = b->ಮುಂದಿನ;
			ಆದರೆ (b == ಖಾಲಿ)
				ನಿಲ್ಲಿಸು;
			*x += b->x;
			*y += b->y;
		} ಆಗ (box_is_float(b));
		rb = b;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_WALK_ಪೋಷಕ:
		*x -= b->x;
		*y -= b->y;
		rb = b->ಪೋಷಕ;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ಆದರೆ (b == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;
		*x += b->x;
		*y += b->y;
		rb = b;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_WALK_ಮುಂದಿನ_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->ಮುಂದಿನ_float;
		ಆದರೆ (b == ಖಾಲಿ)
			ನಿಲ್ಲಿಸು;
		*x += b->x;
		*y += b->y;
		rb = b;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		ಪ್ರತಿಜ್ಞಾ(0 && "Bad box walk type.");
	}

	ಹಿಂತಿರುಗಿ rb;
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ box *
box_ಮುಂದಿನ_xy(ರಚನೆ box *b, ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y, bool skip_children)
{
	ರಚನೆ box *n;
	ಪೂರ್ಣಾಂಕ tx, ty;

	ಪ್ರತಿಜ್ಞಾ(b != ಖಾಲಿ);

	ಆದರೆ (skip_children) {
		/* Caller is not interested in any kind of children */
		ಹೋಗು skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ಆದರೆ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ಹಿಂತಿರುಗಿ n;
	}
 ಮುಗಿದ_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ಆದರೆ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ಹಿಂತಿರುಗಿ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_ಮುಂದಿನ_FLOAT_SIBLING, &tx, &ty);
	ಆದರೆ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ಹಿಂತಿರುಗಿ n;
	}

	ಆದರೆ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		ಹೋಗು ಮುಗಿದ_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ಆಗ (b) {
		ಆಗ (!b->ಮುಂದಿನ && b->ಪೋಷಕ) {
			b = box_move_xy(b, BOX_WALK_ಪೋಷಕ, x, y);
			ಆದರೆ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ಹೋಗು skip_children;
			}
		}
		ಆದರೆ (!b->ಮುಂದಿನ) {
			/* No more boxes */
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_ಮುಂದಿನ_SIBLING, &tx, &ty);
		ಆದರೆ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ಹಿಂತಿರುಗಿ n;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (b->ಪೋಷಕ) {
			b = box_move_xy(b, BOX_WALK_ಪೋಷಕ, x, y);
			ಆದರೆ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ಹೋಗು skip_children;
			}

		} ಇಲ್ಲದಿದ್ದರೆ {
			/* No more boxes */
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}

	ಪ್ರತಿಜ್ಞಾ(b != ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
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
box_nearer_ಪಠ್ಯ_box(ರಚನೆ box *box,
		    ಪೂರ್ಣಾಂಕ bx, ಪೂರ್ಣಾಂಕ by,
		    ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		    ಪೂರ್ಣಾಂಕ dir,
		    ರಚನೆ box **ಹತ್ತಿರದ,
		    ಪೂರ್ಣಾಂಕ *tx, ಪೂರ್ಣಾಂಕ *ty,
		    ಪೂರ್ಣಾಂಕ *nr_xd, ಪೂರ್ಣಾಂಕ *nr_yd)
{
	ಪೂರ್ಣಾಂಕ w = box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2];
	ಪೂರ್ಣಾಂಕ h = box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM];
	ಪೂರ್ಣಾಂಕ y1 = by + h;
	ಪೂರ್ಣಾಂಕ x1 = bx + w;
	ಪೂರ್ಣಾಂಕ yd = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
	ಪೂರ್ಣಾಂಕ xd = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;

	ಆದರೆ (x >= bx && x1 > x && y >= by && y1 > y) {
		*ಹತ್ತಿರದ = box;
		*tx = bx;
		*ty = by;
		ಹಿಂತಿರುಗಿ true;
	}

	ಆದರೆ (box->ಪೋಷಕ->list_marker != box) {
		ಆದರೆ (dir < 0) {
			/* consider only those children (partly) above-left */
			ಆದರೆ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* consider only those children (partly) below-right */
			ಆದರೆ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ಆದರೆ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*ಹತ್ತಿರದ = box;
			*tx = bx;
			*ty = by;
		}
	}
	ಹಿಂತಿರುಗಿ false;
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
box_ಹತ್ತಿರದ_ಪಠ್ಯ_box(ರಚನೆ box *box,
		     ಪೂರ್ಣಾಂಕ bx, ಪೂರ್ಣಾಂಕ by,
		     ಪೂರ್ಣಾಂಕ fx, ಪೂರ್ಣಾಂಕ fy,
		     ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		     ಪೂರ್ಣಾಂಕ dir,
		     ರಚನೆ box **ಹತ್ತಿರದ,
		     ಪೂರ್ಣಾಂಕ *tx, ಪೂರ್ಣಾಂಕ *ty,
		     ಪೂರ್ಣಾಂಕ *nr_xd, ಪೂರ್ಣಾಂಕ *nr_yd)
{
	ರಚನೆ box *ಮಗು = box->children;
	ಪೂರ್ಣಾಂಕ c_bx, c_by;
	ಪೂರ್ಣಾಂಕ c_fx, c_fy;
	bool ಒಳ_box = false;

	ಆದರೆ (*ಹತ್ತಿರದ == ಖಾಲಿ) {
		*nr_xd = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 2; /* displacement of 'nearest so far' */
		*nr_yd = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ / 2;
	}
	ಆದರೆ (box->type == BOX_INLINE_CONTAINER) {
		ಪೂರ್ಣಾಂಕ bw = box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2];
		ಪೂರ್ಣಾಂಕ bh = box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM];
		ಪೂರ್ಣಾಂಕ b_y1 = by + bh;
		ಪೂರ್ಣಾಂಕ b_x1 = bx + bw;
		ಆದರೆ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			ಒಳ_box = true;
		}
	}

	ಆಗ (ಮಗು) {
		ಆದರೆ (ಮಗು->type == BOX_FLOAT_ಎಡ ||
		    ಮಗು->type == BOX_FLOAT_ಬಲ) {
			c_bx = fx + ಮಗು->x -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(ಮಗು->scroll_x);
			c_by = fy + ಮಗು->y -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(ಮಗು->scroll_y);
		} ಇಲ್ಲದಿದ್ದರೆ {
			c_bx = bx + ಮಗು->x -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(ಮಗು->scroll_x);
			c_by = by + ಮಗು->y -
				scrollbar_get_ಸ್ಥಾನಾಂತರ(ಮಗು->scroll_y);
		}
		ಆದರೆ (ಮಗು->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ಇಲ್ಲದಿದ್ದರೆ {
			c_fx = fx;
			c_fy = fy;
		}
		ಆದರೆ (ಒಳ_box && ಮಗು->ಪಠ್ಯ && !ಮಗು->object) {
			ಆದರೆ (box_nearer_ಪಠ್ಯ_box(ಮಗು,
						c_bx, c_by, x, y, dir, ಹತ್ತಿರದ,
						tx, ty, nr_xd, nr_yd))
				ಹಿಂತಿರುಗಿ true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (ಮಗು->list_marker) {
				ಆದರೆ (box_nearer_ಪಠ್ಯ_box(
						ಮಗು->list_marker,
						c_bx + ಮಗು->list_marker->x,
						c_by + ಮಗು->list_marker->y,
						x, y, dir, ಹತ್ತಿರದ,
						tx, ty, nr_xd, nr_yd))
					ಹಿಂತಿರುಗಿ true;
			}
			ಆದರೆ (box_ಹತ್ತಿರದ_ಪಠ್ಯ_box(ಮಗು, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, ಹತ್ತಿರದ, tx, ty,
						 nr_xd, nr_yd))
				ಹಿಂತಿರುಗಿ true;
		}
		ಮಗು = ಮಗು->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ false;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_coords(ರಚನೆ box *box, ಪೂರ್ಣಾಂಕ *x, ಪೂರ್ಣಾಂಕ *y)
{
	*x = box->x;
	*y = box->y;
	ಆಗ (box->ಪೋಷಕ) {
		ಆದರೆ (box_is_float(box)) {
			ಪ್ರತಿಜ್ಞಾ(box->float_container);
			box = box->float_container;
		} ಇಲ್ಲದಿದ್ದರೆ {
			box = box->ಪೋಷಕ;
		}
		*x += box->x - scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x);
		*y += box->y - scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_bounds(ರಚನೆ box *box, ರಚನೆ rect *r)
{
	ಪೂರ್ಣಾಂಕ ಅಗಲ, ಎತ್ತರ;

	box_coords(box, &r->x0, &r->y0);

	ಅಗಲ = box->padding[ಎಡ_2] + box->ಅಗಲ + box->padding[ಬಲ_2];
	ಎತ್ತರ = box->padding[ತುದಿ_2] + box->ಎತ್ತರ + box->padding[BOTTOM];

	r->x1 = r->x0 + ಅಗಲ;
	r->y1 = r->y0 + ಎತ್ತರ;
}


/* Exported function documented in html/box.h */
ರಚನೆ box *
box_at_ಬಿಂದು(ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
	     ರಚನೆ box *box,
	     ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ x, ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ y,
	     ಪೂರ್ಣಾಂಕ *box_x, ಪೂರ್ಣಾಂಕ *box_y)
{
	bool skip_children;
	bool physically;

	ಪ್ರತಿಜ್ಞಾ(box);

	skip_children = false;
	ಆಗ ((box = box_ಮುಂದಿನ_xy(box, box_x, box_y, skip_children))) {
		ಆದರೆ (box_ಒಳಗೊಂಡಿದೆ_ಬಿಂದು(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_x);
			*box_y -= scrollbar_get_ಸ್ಥಾನಾಂತರ(box->scroll_y);

			ಆದರೆ (physically)
				ಹಿಂತಿರುಗಿ box;

			skip_children = false;
		} ಇಲ್ಲದಿದ್ದರೆ {
			skip_children = true;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/* Exported function documented in html/box.h */
ರಚನೆ box *box_ಕಂಡುಹಿಡಿ_by_id(ರಚನೆ box *box, lwc_string *id)
{
	ರಚನೆ box *a, *b;
	bool m;

	ಆದರೆ (box->id != ಖಾಲಿ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ಹಿಂತಿರುಗಿ box;
	}

	ಗಾಗಿ (a = box->children; a; a = a->ಮುಂದಿನ) {
		ಆದರೆ ((b = box_ಕಂಡುಹಿಡಿ_by_id(a, id)) != ಖಾಲಿ) {
			ಹಿಂತಿರುಗಿ b;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/* Exported function documented in html/box.h */
bool box_visible(ರಚನೆ box *box)
{
	/* visibility: hidden */
	ಆದರೆ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/* Exported function documented in html/box.h */
ಶೂನ್ಯ box_dump(FILE *ಹರಿವು, ರಚನೆ box *box, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆಳ, bool style)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ box *c, *prev;

	ಗಾಗಿ (i = 0; i != ಆಳ; i++) {
		fprintf(ಹರಿವು, "  ");
	}

	fprintf(ಹರಿವು, "%p ", box);
	fprintf(ಹರಿವು, "x%i y%i w%i h%i ",
		box->x, box->y, box->ಅಗಲ, box->ಎತ್ತರ);
	ಆದರೆ (box->max_ಅಗಲ != UNKNOWN_MAX_ಅಗಲ) {
		fprintf(ಹರಿವು, "min%i max%i ", box->min_ಅಗಲ, box->max_ಅಗಲ);
	}
	fprintf(ಹರಿವು, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(ಹರಿವು, "m(%i %i %i %i) ",
		box->margin[ತುದಿ_2], box->margin[ಎಡ_2],
		box->margin[BOTTOM], box->margin[ಬಲ_2]);

	ಆಯ್ಕೆ (box->type) {
	ಸ್ಥಿತಿ BOX_ಖಂಡ:
		fprintf(ಹರಿವು, "BLOCK ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_INLINE_CONTAINER:
		fprintf(ಹರಿವು, "INLINE_CONTAINER ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_INLINE:
		fprintf(ಹರಿವು, "INLINE ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_INLINE_ಕೊನೆ:
		fprintf(ಹರಿವು, "INLINE_END ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_INLINE_ಖಂಡ:
		fprintf(ಹರಿವು, "INLINE_BLOCK ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ:
		fprintf(ಹರಿವು, "TABLE [columns %i] ", box->columns);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು:
		fprintf(ಹರಿವು, "TABLE_ROW ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_CELL:
		fprintf(ಹರಿವು, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->ಆರಂಭ_ಸ್ತಂಭ,
			box->ಅಡ್ಡಸಾಲುಗಳು);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು:
		fprintf(ಹರಿವು, "TABLE_ROW_GROUP ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_FLOAT_ಎಡ:
		fprintf(ಹರಿವು, "FLOAT_LEFT ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_FLOAT_ಬಲ:
		fprintf(ಹರಿವು, "FLOAT_RIGHT ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_BR:
		fprintf(ಹರಿವು, "BR ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_ಪಠ್ಯ:
		fprintf(ಹರಿವು, "TEXT ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_FLEX:
		fprintf(ಹರಿವು, "FLEX ");
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ BOX_INLINE_FLEX:
		fprintf(ಹರಿವು, "INLINE_FLEX ");
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		fprintf(ಹರಿವು, "Unknown box type ");
	}

	ಆದರೆ (box->ಪಠ್ಯ)
		fprintf(ಹರಿವು, "%li '%.*s' ", (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) box->byte_ಸ್ಥಾನಾಂತರ,
			(ಪೂರ್ಣಾಂಕ) box->ಉದ್ದಳತೆ, box->ಪಠ್ಯ);
	ಆದರೆ (box->space)
		fprintf(ಹರಿವು, "space ");
	ಆದರೆ (box->object) {
		fprintf(ಹರಿವು, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ಆದರೆ (box->iframe) {
		fprintf(ಹರಿವು, "(iframe) ");
	}
	ಆದರೆ (box->gadget)
		fprintf(ಹರಿವು, "(gadget) ");
	ಆದರೆ (style && box->style)
		nscss_dump_computed_style(ಹರಿವು, box->style);
	ಆದರೆ (box->href)
		fprintf(ಹರಿವು, " -> '%s'", nsurl_access(box->href));
	ಆದರೆ (box->ಗುರಿ)
		fprintf(ಹರಿವು, " |%s|", box->ಗುರಿ);
	ಆದರೆ (box->title)
		fprintf(ಹರಿವು, " [%s]", box->title);
	ಆದರೆ (box->id)
		fprintf(ಹರಿವು, " ID:%s", lwc_string_ದತ್ತಾಂಶ(box->id));
	ಆದರೆ (box->type == BOX_INLINE || box->type == BOX_INLINE_ಕೊನೆ)
		fprintf(ಹರಿವು, " inline_end %p", box->inline_ಕೊನೆ);
	ಆದರೆ (box->float_children)
		fprintf(ಹರಿವು, " float_children %p", box->float_children);
	ಆದರೆ (box->ಮುಂದಿನ_float)
		fprintf(ಹರಿವು, " next_float %p", box->ಮುಂದಿನ_float);
	ಆದರೆ (box->float_container)
		fprintf(ಹರಿವು, " float_container %p", box->float_container);
	ಆದರೆ (box->ಲಂಬಸಾಲು) {
		fprintf(ಹರಿವು, " (columns");
		ಗಾಗಿ (i = 0; i != box->columns; i++) {
			fprintf(ಹರಿವು, " (%s %s %i %i %i)",
				((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->ಲಂಬಸಾಲು[i].type],
				((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *[]) {
					"normal",
					"positioned"})
				[box->ಲಂಬಸಾಲು[i].positioned],
				box->ಲಂಬಸಾಲು[i].ಅಗಲ,
				box->ಲಂಬಸಾಲು[i].min, box->ಲಂಬಸಾಲು[i].max);
		}
		fprintf(ಹರಿವು, ")");
	}
	ಆದರೆ (box->ಗಂಟು != ಖಾಲಿ) {
		dom_string *name;
		ಆದರೆ (dom_ಗಂಟು_get_ಗಂಟು_name(box->ಗಂಟು, &name) == DOM_NO_ERR) {
			fprintf(ಹರಿವು, " <%s>", dom_string_ದತ್ತಾಂಶ(name));
			dom_string_unref(name);
		}
	}
	fprintf(ಹರಿವು, "\n");

	ಆದರೆ (box->list_marker) {
		ಗಾಗಿ (i = 0; i != ಆಳ; i++)
			fprintf(ಹರಿವು, "  ");
		fprintf(ಹರಿವು, "list_marker:\n");
		box_dump(ಹರಿವು, box->list_marker, ಆಳ + 1, style);
	}

	ಗಾಗಿ (c = box->children; c && c->ಮುಂದಿನ; c = c->ಮುಂದಿನ)
		;
	ಆದರೆ (box->ಕೊನೆಯ != c)
		fprintf(ಹರಿವು, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->ಕೊನೆಯ, c, box);
	ಗಾಗಿ (prev = 0, c = box->children; c; prev = c, c = c->ಮುಂದಿನ) {
		ಆದರೆ (c->ಪೋಷಕ != box)
			fprintf(ಹರಿವು, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->ಪೋಷಕ, box);
		ಆದರೆ (c->prev != prev)
			fprintf(ಹರಿವು, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(ಹರಿವು, c, ಆಳ + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ಸ್ಥಿರಾಂಕ ರಚನೆ box * ಸ್ಥಿರಾಂಕ box)
{
	ಹಿಂತಿರುಗಿ box->padding[ತುದಿ_2] +
		box->ಎತ್ತರ +
		box->padding[BOTTOM] +
		box->border[BOTTOM].ಅಗಲ < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ಸ್ಥಿರಾಂಕ ರಚನೆ box * ಸ್ಥಿರಾಂಕ box)
{
	ಹಿಂತಿರುಗಿ box->padding[ಎಡ_2] +
		box->ಅಗಲ +
		box->padding[ಬಲ_2] +
		box->border[ಬಲ_2].ಅಗಲ < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ರಚನೆ box *
box_pick_ಪಠ್ಯ_box(ರಚನೆ html_content *html,
		  ಪೂರ್ಣಾಂಕ x, ಪೂರ್ಣಾಂಕ y,
		  ಪೂರ್ಣಾಂಕ dir,
		  ಪೂರ್ಣಾಂಕ *dx, ಪೂರ್ಣಾಂಕ *dy)
{
	ರಚನೆ box *ಪಠ್ಯ_box = ಖಾಲಿ;
	ರಚನೆ box *box;
	ಪೂರ್ಣಾಂಕ nr_xd, nr_yd;
	ಪೂರ್ಣಾಂಕ bx, by;
	ಪೂರ್ಣಾಂಕ fx, fy;
	ಪೂರ್ಣಾಂಕ tx, ty;

	ಆದರೆ (html == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	box = html->layout;
	bx = box->margin[ಎಡ_2];
	by = box->margin[ತುದಿ_2];
	fx = bx;
	fy = by;

	ಆದರೆ (!box_ಹತ್ತಿರದ_ಪಠ್ಯ_box(box, bx, by, fx, fy, x, y,
				  dir, &ಪಠ್ಯ_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ಆದರೆ (ಪಠ್ಯ_box && ಪಠ್ಯ_box->ಪಠ್ಯ && !ಪಠ್ಯ_box->object) {
			ಪೂರ್ಣಾಂಕ w = (ಪಠ್ಯ_box->padding[ಎಡ_2] +
				 ಪಠ್ಯ_box->ಅಗಲ +
				 ಪಠ್ಯ_box->padding[ಬಲ_2]);
			ಪೂರ್ಣಾಂಕ h = (ಪಠ್ಯ_box->padding[ತುದಿ_2] +
				 ಪಠ್ಯ_box->ಎತ್ತರ +
				 ಪಠ್ಯ_box->padding[BOTTOM]);
			ಪೂರ್ಣಾಂಕ x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ಆದರೆ (x < tx) x = tx;
			ಆದರೆ (y < ty) y = ty;
			ಆದರೆ (y > y1) y = y1;
			ಆದರೆ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ಹಿಂತಿರುಗಿ ಪಠ್ಯ_box;
}
