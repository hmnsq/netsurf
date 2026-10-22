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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/errors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/content.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/mouse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/dump.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/scrollbar.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
ਗਿਣਤੀ box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_ਮਾਪਾ,
		   BOX_WALK_ਅਗਲਾ_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_ਅਗਲਾ_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ box_is_float(box) (box->type == BOX_FLOAT_ਖੱਬਾ ||	\
			   box->type == BOX_FLOAT_ਸੱਜਾ)

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
ਸਥਿਰ bool
box_ਸ਼ਾਮਲ_ਹੈ_ਬਿੰਦੂ(ਅਚਲ css_unit_ctx *unit_len_ctx,
		   ਅਚਲ ਬਣਤਰ box *box,
		   ਪੂਰਨ x,
		   ਪੂਰਨ y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ਜੇ (box->style != ਨਿਰਾ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ਬਣਤਰ rect r = {
				 .x0 = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ,
				 .y0 = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ,
				 .x1 = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ +
				 box->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
				 box->padding[ਸੱਜਾ_2],
				 .y1 = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
				 box->border[BOTTOM].ਚੌੜਾਈ +
				 box->padding[BOTTOM]
		};
		ਜੇ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ਨਹੀਂਤਾਂ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ਜੇ (css_rect.ਖੱਬਾ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ਖੱਬਾ,
						css_rect.lunit));
		}
		ਜੇ (css_rect.ਸਿਖਰ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ਸਿਖਰ,
						css_rect.tunit));
		}
		ਜੇ (css_rect.ਸੱਜਾ_auto == false) {
			r.x1 = box->border[ਖੱਬਾ_2].ਚੌੜਾਈ +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ਸੱਜਾ,
						css_rect.runit));
		}
		ਜੇ (css_rect.bottom_auto == false) {
			r.y1 = box->border[ਸਿਖਰ_2].ਚੌੜਾਈ +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ਜੇ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ਵਾਪਸ true;
		}

		/* Not inside clip area */
		ਵਾਪਸ false;
	}
	ਜੇ (x >= -box->border[ਖੱਬਾ_2].ਚੌੜਾਈ &&
	    x < box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ +
	    box->padding[ਸੱਜਾ_2] + box->border[ਸੱਜਾ_2].ਚੌੜਾਈ &&
	    y >= -box->border[ਸਿਖਰ_2].ਚੌੜਾਈ &&
	    y < box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ +
	    box->padding[BOTTOM] + box->border[BOTTOM].ਚੌੜਾਈ) {
		*physically = true;
		ਵਾਪਸ true;
	}
	ਜੇ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ਖੱਬਾ_2].ਚੌੜਾਈ &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ਖੱਬਾ_2] +
	    box->list_marker->ਚੌੜਾਈ +
	    box->list_marker->border[ਸੱਜਾ_2].ਚੌੜਾਈ +
	    box->list_marker->padding[ਸੱਜਾ_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[ਸਿਖਰ_2].ਚੌੜਾਈ &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[ਸਿਖਰ_2] +
	    box->list_marker->ਉਚਾਈ +
	    box->list_marker->border[BOTTOM].ਚੌੜਾਈ +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ਵਾਪਸ true;
	}
	ਜੇ ((box->style && css_computed_ਉਛਾਲ_x(box->style) ==
	     CSS_ਉਛਾਲ_VISIBLE) || !box->style) {
		ਜੇ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ਵਾਪਸ true;
		}
	}
	ਜੇ ((box->style && css_computed_ਉਛਾਲ_y(box->style) ==
	     CSS_ਉਛਾਲ_VISIBLE) || !box->style) {
		ਜੇ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ਵਾਪਸ true;
		}
	}
	ਵਾਪਸ false;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ box *
box_move_xy(ਬਣਤਰ box *b, ਗਿਣਤੀ box_walk_dir dir, ਪੂਰਨ *x, ਪੂਰਨ *y)
{
	ਬਣਤਰ box *rb = ਨਿਰਾ;

	ਚੋਣ (dir) {
	ਹਾਲਤ BOX_WALK_CHILDREN:
		b = b->children;
		ਜੇ (b == ਨਿਰਾ)
			ਰੋਕੋ;
		*x += b->x;
		*y += b->y;
		ਜੇ (!box_is_float(b)) {
			rb = b;
			ਰੋਕੋ;
		}
		fallthrough;

	ਹਾਲਤ BOX_WALK_ਅਗਲਾ_SIBLING:
		ਕਰੋ {
			*x -= b->x;
			*y -= b->y;
			b = b->ਅਗਲਾ;
			ਜੇ (b == ਨਿਰਾ)
				ਰੋਕੋ;
			*x += b->x;
			*y += b->y;
		} ਜਦੋਂ (box_is_float(b));
		rb = b;
		ਰੋਕੋ;

	ਹਾਲਤ BOX_WALK_ਮਾਪਾ:
		*x -= b->x;
		*y -= b->y;
		rb = b->ਮਾਪਾ;
		ਰੋਕੋ;

	ਹਾਲਤ BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ਜੇ (b == ਨਿਰਾ)
			ਰੋਕੋ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ਰੋਕੋ;

	ਹਾਲਤ BOX_WALK_ਅਗਲਾ_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->ਅਗਲਾ_float;
		ਜੇ (b == ਨਿਰਾ)
			ਰੋਕੋ;
		*x += b->x;
		*y += b->y;
		rb = b;
		ਰੋਕੋ;

	ਹਾਲਤ BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		ਰੋਕੋ;

	ਮੂਲ:
		ਪ੍ਰਤਿਜ੍ਞਾ(0 && "Bad box walk type.");
	}

	ਵਾਪਸ rb;
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ box *
box_ਅਗਲਾ_xy(ਬਣਤਰ box *b, ਪੂਰਨ *x, ਪੂਰਨ *y, bool skip_children)
{
	ਬਣਤਰ box *n;
	ਪੂਰਨ tx, ty;

	ਪ੍ਰਤਿਜ੍ਞਾ(b != ਨਿਰਾ);

	ਜੇ (skip_children) {
		/* Caller is not interested in any kind of children */
		ਜਾਓ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ਜੇ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ਵਾਪਸ n;
	}
 ਪੂਰਾ_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ਜੇ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ਵਾਪਸ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_ਅਗਲਾ_FLOAT_SIBLING, &tx, &ty);
	ਜੇ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ਵਾਪਸ n;
	}

	ਜੇ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		ਜਾਓ ਪੂਰਾ_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ਜਦੋਂ (b) {
		ਜਦੋਂ (!b->ਅਗਲਾ && b->ਮਾਪਾ) {
			b = box_move_xy(b, BOX_WALK_ਮਾਪਾ, x, y);
			ਜੇ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ਜਾਓ skip_children;
			}
		}
		ਜੇ (!b->ਅਗਲਾ) {
			/* No more boxes */
			ਵਾਪਸ ਨਿਰਾ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_ਅਗਲਾ_SIBLING, &tx, &ty);
		ਜੇ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ਵਾਪਸ n;

		} ਨਹੀਂਤਾਂ ਜੇ (b->ਮਾਪਾ) {
			b = box_move_xy(b, BOX_WALK_ਮਾਪਾ, x, y);
			ਜੇ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				ਜਾਓ skip_children;
			}

		} ਨਹੀਂਤਾਂ {
			/* No more boxes */
			ਵਾਪਸ ਨਿਰਾ;
		}
	}

	ਪ੍ਰਤਿਜ੍ਞਾ(b != ਨਿਰਾ);
	ਵਾਪਸ ਨਿਰਾ;
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
ਸਥਿਰ bool
box_nearer_ਪਾਠ_box(ਬਣਤਰ box *box,
		    ਪੂਰਨ bx, ਪੂਰਨ by,
		    ਪੂਰਨ x, ਪੂਰਨ y,
		    ਪੂਰਨ dir,
		    ਬਣਤਰ box **ਸਭ_ਤੋਂ_ਨੇੜੇ,
		    ਪੂਰਨ *tx, ਪੂਰਨ *ty,
		    ਪੂਰਨ *nr_xd, ਪੂਰਨ *nr_yd)
{
	ਪੂਰਨ w = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
	ਪੂਰਨ h = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM];
	ਪੂਰਨ y1 = by + h;
	ਪੂਰਨ x1 = bx + w;
	ਪੂਰਨ yd = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
	ਪੂਰਨ xd = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;

	ਜੇ (x >= bx && x1 > x && y >= by && y1 > y) {
		*ਸਭ_ਤੋਂ_ਨੇੜੇ = box;
		*tx = bx;
		*ty = by;
		ਵਾਪਸ true;
	}

	ਜੇ (box->ਮਾਪਾ->list_marker != box) {
		ਜੇ (dir < 0) {
			/* consider only those children (partly) above-left */
			ਜੇ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ਨਹੀਂਤਾਂ {
			/* consider only those children (partly) below-right */
			ਜੇ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ਜੇ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*ਸਭ_ਤੋਂ_ਨੇੜੇ = box;
			*tx = bx;
			*ty = by;
		}
	}
	ਵਾਪਸ false;
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
ਸਥਿਰ bool
box_ਸਭ_ਤੋਂ_ਨੇੜੇ_ਪਾਠ_box(ਬਣਤਰ box *box,
		     ਪੂਰਨ bx, ਪੂਰਨ by,
		     ਪੂਰਨ fx, ਪੂਰਨ fy,
		     ਪੂਰਨ x, ਪੂਰਨ y,
		     ਪੂਰਨ dir,
		     ਬਣਤਰ box **ਸਭ_ਤੋਂ_ਨੇੜੇ,
		     ਪੂਰਨ *tx, ਪੂਰਨ *ty,
		     ਪੂਰਨ *nr_xd, ਪੂਰਨ *nr_yd)
{
	ਬਣਤਰ box *ਬੱਚਾ = box->children;
	ਪੂਰਨ c_bx, c_by;
	ਪੂਰਨ c_fx, c_fy;
	bool ਵਿੱਚ_box = false;

	ਜੇ (*ਸਭ_ਤੋਂ_ਨੇੜੇ == ਨਿਰਾ) {
		*nr_xd = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 2; /* displacement of 'nearest so far' */
		*nr_yd = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ / 2;
	}
	ਜੇ (box->type == BOX_INLINE_CONTAINER) {
		ਪੂਰਨ bw = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
		ਪੂਰਨ bh = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM];
		ਪੂਰਨ b_y1 = by + bh;
		ਪੂਰਨ b_x1 = bx + bw;
		ਜੇ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			ਵਿੱਚ_box = true;
		}
	}

	ਜਦੋਂ (ਬੱਚਾ) {
		ਜੇ (ਬੱਚਾ->type == BOX_FLOAT_ਖੱਬਾ ||
		    ਬੱਚਾ->type == BOX_FLOAT_ਸੱਜਾ) {
			c_bx = fx + ਬੱਚਾ->x -
				scrollbar_get_ਵਿੱਥ(ਬੱਚਾ->scroll_x);
			c_by = fy + ਬੱਚਾ->y -
				scrollbar_get_ਵਿੱਥ(ਬੱਚਾ->scroll_y);
		} ਨਹੀਂਤਾਂ {
			c_bx = bx + ਬੱਚਾ->x -
				scrollbar_get_ਵਿੱਥ(ਬੱਚਾ->scroll_x);
			c_by = by + ਬੱਚਾ->y -
				scrollbar_get_ਵਿੱਥ(ਬੱਚਾ->scroll_y);
		}
		ਜੇ (ਬੱਚਾ->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ਨਹੀਂਤਾਂ {
			c_fx = fx;
			c_fy = fy;
		}
		ਜੇ (ਵਿੱਚ_box && ਬੱਚਾ->ਪਾਠ && !ਬੱਚਾ->object) {
			ਜੇ (box_nearer_ਪਾਠ_box(ਬੱਚਾ,
						c_bx, c_by, x, y, dir, ਸਭ_ਤੋਂ_ਨੇੜੇ,
						tx, ty, nr_xd, nr_yd))
				ਵਾਪਸ true;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (ਬੱਚਾ->list_marker) {
				ਜੇ (box_nearer_ਪਾਠ_box(
						ਬੱਚਾ->list_marker,
						c_bx + ਬੱਚਾ->list_marker->x,
						c_by + ਬੱਚਾ->list_marker->y,
						x, y, dir, ਸਭ_ਤੋਂ_ਨੇੜੇ,
						tx, ty, nr_xd, nr_yd))
					ਵਾਪਸ true;
			}
			ਜੇ (box_ਸਭ_ਤੋਂ_ਨੇੜੇ_ਪਾਠ_box(ਬੱਚਾ, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, ਸਭ_ਤੋਂ_ਨੇੜੇ, tx, ty,
						 nr_xd, nr_yd))
				ਵਾਪਸ true;
		}
		ਬੱਚਾ = ਬੱਚਾ->ਅਗਲਾ;
	}

	ਵਾਪਸ false;
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_coords(ਬਣਤਰ box *box, ਪੂਰਨ *x, ਪੂਰਨ *y)
{
	*x = box->x;
	*y = box->y;
	ਜਦੋਂ (box->ਮਾਪਾ) {
		ਜੇ (box_is_float(box)) {
			ਪ੍ਰਤਿਜ੍ਞਾ(box->float_container);
			box = box->float_container;
		} ਨਹੀਂਤਾਂ {
			box = box->ਮਾਪਾ;
		}
		*x += box->x - scrollbar_get_ਵਿੱਥ(box->scroll_x);
		*y += box->y - scrollbar_get_ਵਿੱਥ(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_bounds(ਬਣਤਰ box *box, ਬਣਤਰ rect *r)
{
	ਪੂਰਨ ਚੌੜਾਈ, ਉਚਾਈ;

	box_coords(box, &r->x0, &r->y0);

	ਚੌੜਾਈ = box->padding[ਖੱਬਾ_2] + box->ਚੌੜਾਈ + box->padding[ਸੱਜਾ_2];
	ਉਚਾਈ = box->padding[ਸਿਖਰ_2] + box->ਉਚਾਈ + box->padding[BOTTOM];

	r->x1 = r->x0 + ਚੌੜਾਈ;
	r->y1 = r->y0 + ਉਚਾਈ;
}


/* Exported function documented in html/box.h */
ਬਣਤਰ box *
box_at_ਬਿੰਦੂ(ਅਚਲ css_unit_ctx *unit_len_ctx,
	     ਬਣਤਰ box *box,
	     ਅਚਲ ਪੂਰਨ x, ਅਚਲ ਪੂਰਨ y,
	     ਪੂਰਨ *box_x, ਪੂਰਨ *box_y)
{
	bool skip_children;
	bool physically;

	ਪ੍ਰਤਿਜ੍ਞਾ(box);

	skip_children = false;
	ਜਦੋਂ ((box = box_ਅਗਲਾ_xy(box, box_x, box_y, skip_children))) {
		ਜੇ (box_ਸ਼ਾਮਲ_ਹੈ_ਬਿੰਦੂ(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_ਵਿੱਥ(box->scroll_x);
			*box_y -= scrollbar_get_ਵਿੱਥ(box->scroll_y);

			ਜੇ (physically)
				ਵਾਪਸ box;

			skip_children = false;
		} ਨਹੀਂਤਾਂ {
			skip_children = true;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/* Exported function documented in html/box.h */
ਬਣਤਰ box *box_ਲੱਭੋ_by_id(ਬਣਤਰ box *box, lwc_string *id)
{
	ਬਣਤਰ box *a, *b;
	bool m;

	ਜੇ (box->id != ਨਿਰਾ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ਵਾਪਸ box;
	}

	ਲਈ (a = box->children; a; a = a->ਅਗਲਾ) {
		ਜੇ ((b = box_ਲੱਭੋ_by_id(a, id)) != ਨਿਰਾ) {
			ਵਾਪਸ b;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}


/* Exported function documented in html/box.h */
bool box_visible(ਬਣਤਰ box *box)
{
	/* visibility: hidden */
	ਜੇ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}


/* Exported function documented in html/box.h */
ਖਾਲੀ box_dump(FILE *ਧਾਰਾ, ਬਣਤਰ box *box, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਡੂੰਘਾਈ, bool style)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਬਣਤਰ box *c, *prev;

	ਲਈ (i = 0; i != ਡੂੰਘਾਈ; i++) {
		fprintf(ਧਾਰਾ, "  ");
	}

	fprintf(ਧਾਰਾ, "%p ", box);
	fprintf(ਧਾਰਾ, "x%i y%i w%i h%i ",
		box->x, box->y, box->ਚੌੜਾਈ, box->ਉਚਾਈ);
	ਜੇ (box->max_ਚੌੜਾਈ != UNKNOWN_MAX_ਚੌੜਾਈ) {
		fprintf(ਧਾਰਾ, "min%i max%i ", box->min_ਚੌੜਾਈ, box->max_ਚੌੜਾਈ);
	}
	fprintf(ਧਾਰਾ, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(ਧਾਰਾ, "m(%i %i %i %i) ",
		box->margin[ਸਿਖਰ_2], box->margin[ਖੱਬਾ_2],
		box->margin[BOTTOM], box->margin[ਸੱਜਾ_2]);

	ਚੋਣ (box->type) {
	ਹਾਲਤ BOX_ਖੰਡ:
		fprintf(ਧਾਰਾ, "BLOCK ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_INLINE_CONTAINER:
		fprintf(ਧਾਰਾ, "INLINE_CONTAINER ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_INLINE:
		fprintf(ਧਾਰਾ, "INLINE ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_INLINE_ਸਿਰਾ:
		fprintf(ਧਾਰਾ, "INLINE_END ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_INLINE_ਖੰਡ:
		fprintf(ਧਾਰਾ, "INLINE_BLOCK ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_ਸਾਰਣੀ:
		fprintf(ਧਾਰਾ, "TABLE [columns %i] ", box->columns);
		ਰੋਕੋ;

	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ:
		fprintf(ਧਾਰਾ, "TABLE_ROW ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_ਸਾਰਣੀ_CELL:
		fprintf(ਧਾਰਾ, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->ਸ਼ੁਰੂ_ਕਾਲਮ,
			box->ਪੰਕਤੀਆਂ);
		ਰੋਕੋ;

	ਹਾਲਤ BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ:
		fprintf(ਧਾਰਾ, "TABLE_ROW_GROUP ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_FLOAT_ਖੱਬਾ:
		fprintf(ਧਾਰਾ, "FLOAT_LEFT ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_FLOAT_ਸੱਜਾ:
		fprintf(ਧਾਰਾ, "FLOAT_RIGHT ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_BR:
		fprintf(ਧਾਰਾ, "BR ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_ਪਾਠ:
		fprintf(ਧਾਰਾ, "TEXT ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_FLEX:
		fprintf(ਧਾਰਾ, "FLEX ");
		ਰੋਕੋ;

	ਹਾਲਤ BOX_INLINE_FLEX:
		fprintf(ਧਾਰਾ, "INLINE_FLEX ");
		ਰੋਕੋ;

	ਮੂਲ:
		fprintf(ਧਾਰਾ, "Unknown box type ");
	}

	ਜੇ (box->ਪਾਠ)
		fprintf(ਧਾਰਾ, "%li '%.*s' ", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) box->byte_ਵਿੱਥ,
			(ਪੂਰਨ) box->ਲੰਮਾਈ, box->ਪਾਠ);
	ਜੇ (box->space)
		fprintf(ਧਾਰਾ, "space ");
	ਜੇ (box->object) {
		fprintf(ਧਾਰਾ, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ਜੇ (box->iframe) {
		fprintf(ਧਾਰਾ, "(iframe) ");
	}
	ਜੇ (box->gadget)
		fprintf(ਧਾਰਾ, "(gadget) ");
	ਜੇ (style && box->style)
		nscss_dump_computed_style(ਧਾਰਾ, box->style);
	ਜੇ (box->href)
		fprintf(ਧਾਰਾ, " -> '%s'", nsurl_access(box->href));
	ਜੇ (box->ਨਿਸ਼ਾਨਾ)
		fprintf(ਧਾਰਾ, " |%s|", box->ਨਿਸ਼ਾਨਾ);
	ਜੇ (box->title)
		fprintf(ਧਾਰਾ, " [%s]", box->title);
	ਜੇ (box->id)
		fprintf(ਧਾਰਾ, " ID:%s", lwc_string_ਅੰਕੜੇ(box->id));
	ਜੇ (box->type == BOX_INLINE || box->type == BOX_INLINE_ਸਿਰਾ)
		fprintf(ਧਾਰਾ, " inline_end %p", box->inline_ਸਿਰਾ);
	ਜੇ (box->float_children)
		fprintf(ਧਾਰਾ, " float_children %p", box->float_children);
	ਜੇ (box->ਅਗਲਾ_float)
		fprintf(ਧਾਰਾ, " next_float %p", box->ਅਗਲਾ_float);
	ਜੇ (box->float_container)
		fprintf(ਧਾਰਾ, " float_container %p", box->float_container);
	ਜੇ (box->ਖਾਨਾ) {
		fprintf(ਧਾਰਾ, " (columns");
		ਲਈ (i = 0; i != box->columns; i++) {
			fprintf(ਧਾਰਾ, " (%s %s %i %i %i)",
				((ਅਚਲ ਅੱਖਰ *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->ਖਾਨਾ[i].type],
				((ਅਚਲ ਅੱਖਰ *[]) {
					"normal",
					"positioned"})
				[box->ਖਾਨਾ[i].positioned],
				box->ਖਾਨਾ[i].ਚੌੜਾਈ,
				box->ਖਾਨਾ[i].min, box->ਖਾਨਾ[i].max);
		}
		fprintf(ਧਾਰਾ, ")");
	}
	ਜੇ (box->ਗੰਢ != ਨਿਰਾ) {
		dom_string *name;
		ਜੇ (dom_ਗੰਢ_get_ਗੰਢ_name(box->ਗੰਢ, &name) == DOM_NO_ERR) {
			fprintf(ਧਾਰਾ, " <%s>", dom_string_ਅੰਕੜੇ(name));
			dom_string_unref(name);
		}
	}
	fprintf(ਧਾਰਾ, "\n");

	ਜੇ (box->list_marker) {
		ਲਈ (i = 0; i != ਡੂੰਘਾਈ; i++)
			fprintf(ਧਾਰਾ, "  ");
		fprintf(ਧਾਰਾ, "list_marker:\n");
		box_dump(ਧਾਰਾ, box->list_marker, ਡੂੰਘਾਈ + 1, style);
	}

	ਲਈ (c = box->children; c && c->ਅਗਲਾ; c = c->ਅਗਲਾ)
		;
	ਜੇ (box->ਆਖਰੀ != c)
		fprintf(ਧਾਰਾ, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->ਆਖਰੀ, c, box);
	ਲਈ (prev = 0, c = box->children; c; prev = c, c = c->ਅਗਲਾ) {
		ਜੇ (c->ਮਾਪਾ != box)
			fprintf(ਧਾਰਾ, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->ਮਾਪਾ, box);
		ਜੇ (c->prev != prev)
			fprintf(ਧਾਰਾ, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(ਧਾਰਾ, c, ਡੂੰਘਾਈ + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ਅਚਲ ਬਣਤਰ box * ਅਚਲ box)
{
	ਵਾਪਸ box->padding[ਸਿਖਰ_2] +
		box->ਉਚਾਈ +
		box->padding[BOTTOM] +
		box->border[BOTTOM].ਚੌੜਾਈ < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ਅਚਲ ਬਣਤਰ box * ਅਚਲ box)
{
	ਵਾਪਸ box->padding[ਖੱਬਾ_2] +
		box->ਚੌੜਾਈ +
		box->padding[ਸੱਜਾ_2] +
		box->border[ਸੱਜਾ_2].ਚੌੜਾਈ < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ਬਣਤਰ box *
box_pick_ਪਾਠ_box(ਬਣਤਰ html_content *html,
		  ਪੂਰਨ x, ਪੂਰਨ y,
		  ਪੂਰਨ dir,
		  ਪੂਰਨ *dx, ਪੂਰਨ *dy)
{
	ਬਣਤਰ box *ਪਾਠ_box = ਨਿਰਾ;
	ਬਣਤਰ box *box;
	ਪੂਰਨ nr_xd, nr_yd;
	ਪੂਰਨ bx, by;
	ਪੂਰਨ fx, fy;
	ਪੂਰਨ tx, ty;

	ਜੇ (html == ਨਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	box = html->layout;
	bx = box->margin[ਖੱਬਾ_2];
	by = box->margin[ਸਿਖਰ_2];
	fx = bx;
	fy = by;

	ਜੇ (!box_ਸਭ_ਤੋਂ_ਨੇੜੇ_ਪਾਠ_box(box, bx, by, fx, fy, x, y,
				  dir, &ਪਾਠ_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ਜੇ (ਪਾਠ_box && ਪਾਠ_box->ਪਾਠ && !ਪਾਠ_box->object) {
			ਪੂਰਨ w = (ਪਾਠ_box->padding[ਖੱਬਾ_2] +
				 ਪਾਠ_box->ਚੌੜਾਈ +
				 ਪਾਠ_box->padding[ਸੱਜਾ_2]);
			ਪੂਰਨ h = (ਪਾਠ_box->padding[ਸਿਖਰ_2] +
				 ਪਾਠ_box->ਉਚਾਈ +
				 ਪਾਠ_box->padding[BOTTOM]);
			ਪੂਰਨ x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ਜੇ (x < tx) x = tx;
			ਜੇ (y < ty) y = ty;
			ਜੇ (y > y1) y = y1;
			ਜੇ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ਵਾਪਸ ਪਾਠ_box;
}
