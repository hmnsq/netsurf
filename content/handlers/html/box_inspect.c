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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/utils.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "utils/errors.h"
#അന്തര്ഭാവ "netsurf/types.h"
#അന്തര്ഭാവ "netsurf/content.h"
#അന്തര്ഭാവ "netsurf/mouse.h"
#അന്തര്ഭാവ "css/utils.h"
#അന്തര്ഭാവ "css/dump.h"
#അന്തര്ഭാവ "desktop/scrollbar.h"

#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
എണ്ണൽ box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_ഇടത് ||	\
			   box->type == BOX_FLOAT_വലത്)

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
സ്ഥിതം bool
box_contains_point(സ്ഥിരം css_unit_ctx *unit_len_ctx,
		   സ്ഥിരം ഘടന box *box,
		   പൂർണ്ണസംഖ്യ x,
		   പൂർണ്ണസംഖ്യ y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	എങ്കിൽ (box->style != ശൂന്യ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ഘടന rect r = {
				 .x0 = box->border[ഇടത്_2].width,
				 .y0 = box->border[മുകൾ_2].width,
				 .x1 = box->padding[ഇടത്_2] + box->width +
				 box->border[വലത്_2].width +
				 box->padding[വലത്_2],
				 .y1 = box->padding[മുകൾ_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		എങ്കിൽ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} അല്ലെങ്കിൽ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		എങ്കിൽ (css_rect.ഇടത്_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ഇടത്,
						css_rect.lunit));
		}
		എങ്കിൽ (css_rect.മുകൾ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.മുകൾ,
						css_rect.tunit));
		}
		എങ്കിൽ (css_rect.വലത്_auto == false) {
			r.x1 = box->border[ഇടത്_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.വലത്,
						css_rect.runit));
		}
		എങ്കിൽ (css_rect.bottom_auto == false) {
			r.y1 = box->border[മുകൾ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		എങ്കിൽ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			തിരികെ true;
		}

		/* Not inside clip area */
		തിരികെ false;
	}
	എങ്കിൽ (x >= -box->border[ഇടത്_2].width &&
	    x < box->padding[ഇടത്_2] + box->width +
	    box->padding[വലത്_2] + box->border[വലത്_2].width &&
	    y >= -box->border[മുകൾ_2].width &&
	    y < box->padding[മുകൾ_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		തിരികെ true;
	}
	എങ്കിൽ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ഇടത്_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ഇടത്_2] +
	    box->list_marker->width +
	    box->list_marker->border[വലത്_2].width +
	    box->list_marker->padding[വലത്_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[മുകൾ_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[മുകൾ_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		തിരികെ true;
	}
	എങ്കിൽ ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		എങ്കിൽ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			തിരികെ true;
		}
	}
	എങ്കിൽ ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		എങ്കിൽ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			തിരികെ true;
		}
	}
	തിരികെ false;
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
സ്ഥിതം വരിയിൽ ഘടന box *
box_move_xy(ഘടന box *b, എണ്ണൽ box_walk_dir dir, പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y)
{
	ഘടന box *rb = ശൂന്യ;

	തിരഞ്ഞെടുപ്പ് (dir) {
	അവസ്ഥ BOX_WALK_CHILDREN:
		b = b->children;
		എങ്കിൽ (b == ശൂന്യ)
			നിർത്തുക;
		*x += b->x;
		*y += b->y;
		എങ്കിൽ (!box_is_float(b)) {
			rb = b;
			നിർത്തുക;
		}
		fallthrough;

	അവസ്ഥ BOX_WALK_NEXT_SIBLING:
		ചെയ്യുക {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			എങ്കിൽ (b == ശൂന്യ)
				നിർത്തുക;
			*x += b->x;
			*y += b->y;
		} ആകുമ്പോൾ (box_is_float(b));
		rb = b;
		നിർത്തുക;

	അവസ്ഥ BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		നിർത്തുക;

	അവസ്ഥ BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		എങ്കിൽ (b == ശൂന്യ)
			നിർത്തുക;
		*x += b->x;
		*y += b->y;
		rb = b;
		നിർത്തുക;

	അവസ്ഥ BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		എങ്കിൽ (b == ശൂന്യ)
			നിർത്തുക;
		*x += b->x;
		*y += b->y;
		rb = b;
		നിർത്തുക;

	അവസ്ഥ BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		പ്രതിജ്ഞാ(0 && "Bad box walk type.");
	}

	തിരികെ rb;
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
സ്ഥിതം വരിയിൽ ഘടന box *
box_next_xy(ഘടന box *b, പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y, bool skip_children)
{
	ഘടന box *n;
	പൂർണ്ണസംഖ്യ tx, ty;

	പ്രതിജ്ഞാ(b != ശൂന്യ);

	എങ്കിൽ (skip_children) {
		/* Caller is not interested in any kind of children */
		പോകുക skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	എങ്കിൽ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		തിരികെ n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	എങ്കിൽ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		തിരികെ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	എങ്കിൽ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		തിരികെ n;
	}

	എങ്കിൽ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		പോകുക done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ആകുമ്പോൾ (b) {
		ആകുമ്പോൾ (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			എങ്കിൽ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				പോകുക skip_children;
			}
		}
		എങ്കിൽ (!b->next) {
			/* No more boxes */
			തിരികെ ശൂന്യ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		എങ്കിൽ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			തിരികെ n;

		} അല്ലെങ്കിൽ എങ്കിൽ (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			എങ്കിൽ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				പോകുക skip_children;
			}

		} അല്ലെങ്കിൽ {
			/* No more boxes */
			തിരികെ ശൂന്യ;
		}
	}

	പ്രതിജ്ഞാ(b != ശൂന്യ);
	തിരികെ ശൂന്യ;
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
സ്ഥിതം bool
box_nearer_text_box(ഘടന box *box,
		    പൂർണ്ണസംഖ്യ bx, പൂർണ്ണസംഖ്യ by,
		    പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		    പൂർണ്ണസംഖ്യ dir,
		    ഘടന box **nearest,
		    പൂർണ്ണസംഖ്യ *tx, പൂർണ്ണസംഖ്യ *ty,
		    പൂർണ്ണസംഖ്യ *nr_xd, പൂർണ്ണസംഖ്യ *nr_yd)
{
	പൂർണ്ണസംഖ്യ w = box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2];
	പൂർണ്ണസംഖ്യ h = box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM];
	പൂർണ്ണസംഖ്യ y1 = by + h;
	പൂർണ്ണസംഖ്യ x1 = bx + w;
	പൂർണ്ണസംഖ്യ yd = INT_MAX;
	പൂർണ്ണസംഖ്യ xd = INT_MAX;

	എങ്കിൽ (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		തിരികെ true;
	}

	എങ്കിൽ (box->parent->list_marker != box) {
		എങ്കിൽ (dir < 0) {
			/* consider only those children (partly) above-left */
			എങ്കിൽ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} അല്ലെങ്കിൽ {
			/* consider only those children (partly) below-right */
			എങ്കിൽ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		എങ്കിൽ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	തിരികെ false;
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
സ്ഥിതം bool
box_nearest_text_box(ഘടന box *box,
		     പൂർണ്ണസംഖ്യ bx, പൂർണ്ണസംഖ്യ by,
		     പൂർണ്ണസംഖ്യ fx, പൂർണ്ണസംഖ്യ fy,
		     പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		     പൂർണ്ണസംഖ്യ dir,
		     ഘടന box **nearest,
		     പൂർണ്ണസംഖ്യ *tx, പൂർണ്ണസംഖ്യ *ty,
		     പൂർണ്ണസംഖ്യ *nr_xd, പൂർണ്ണസംഖ്യ *nr_yd)
{
	ഘടന box *child = box->children;
	പൂർണ്ണസംഖ്യ c_bx, c_by;
	പൂർണ്ണസംഖ്യ c_fx, c_fy;
	bool in_box = false;

	എങ്കിൽ (*nearest == ശൂന്യ) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	എങ്കിൽ (box->type == BOX_INLINE_CONTAINER) {
		പൂർണ്ണസംഖ്യ bw = box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2];
		പൂർണ്ണസംഖ്യ bh = box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM];
		പൂർണ്ണസംഖ്യ b_y1 = by + bh;
		പൂർണ്ണസംഖ്യ b_x1 = bx + bw;
		എങ്കിൽ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	ആകുമ്പോൾ (child) {
		എങ്കിൽ (child->type == BOX_FLOAT_ഇടത് ||
		    child->type == BOX_FLOAT_വലത്) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} അല്ലെങ്കിൽ {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		എങ്കിൽ (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} അല്ലെങ്കിൽ {
			c_fx = fx;
			c_fy = fy;
		}
		എങ്കിൽ (in_box && child->text && !child->object) {
			എങ്കിൽ (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				തിരികെ true;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (child->list_marker) {
				എങ്കിൽ (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					തിരികെ true;
			}
			എങ്കിൽ (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				തിരികെ true;
		}
		child = child->next;
	}

	തിരികെ false;
}


/* Exported function documented in html/box.h */
ശൂന്യം box_coords(ഘടന box *box, പൂർണ്ണസംഖ്യ *x, പൂർണ്ണസംഖ്യ *y)
{
	*x = box->x;
	*y = box->y;
	ആകുമ്പോൾ (box->parent) {
		എങ്കിൽ (box_is_float(box)) {
			പ്രതിജ്ഞാ(box->float_container);
			box = box->float_container;
		} അല്ലെങ്കിൽ {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
ശൂന്യം box_bounds(ഘടന box *box, ഘടന rect *r)
{
	പൂർണ്ണസംഖ്യ width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[ഇടത്_2] + box->width + box->padding[വലത്_2];
	height = box->padding[മുകൾ_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
ഘടന box *
box_at_point(സ്ഥിരം css_unit_ctx *unit_len_ctx,
	     ഘടന box *box,
	     സ്ഥിരം പൂർണ്ണസംഖ്യ x, സ്ഥിരം പൂർണ്ണസംഖ്യ y,
	     പൂർണ്ണസംഖ്യ *box_x, പൂർണ്ണസംഖ്യ *box_y)
{
	bool skip_children;
	bool physically;

	പ്രതിജ്ഞാ(box);

	skip_children = false;
	ആകുമ്പോൾ ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		എങ്കിൽ (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			എങ്കിൽ (physically)
				തിരികെ box;

			skip_children = false;
		} അല്ലെങ്കിൽ {
			skip_children = true;
		}
	}

	തിരികെ ശൂന്യ;
}


/* Exported function documented in html/box.h */
ഘടന box *box_find_by_id(ഘടന box *box, lwc_string *id)
{
	ഘടന box *a, *b;
	bool m;

	എങ്കിൽ (box->id != ശൂന്യ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		തിരികെ box;
	}

	വേണ്ടി (a = box->children; a; a = a->next) {
		എങ്കിൽ ((b = box_find_by_id(a, id)) != ശൂന്യ) {
			തിരികെ b;
		}
	}

	തിരികെ ശൂന്യ;
}


/* Exported function documented in html/box.h */
bool box_visible(ഘടന box *box)
{
	/* visibility: hidden */
	എങ്കിൽ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		തിരികെ false;
	}

	തിരികെ true;
}


/* Exported function documented in html/box.h */
ശൂന്യം box_dump(FILE *stream, ഘടന box *box, unsigned പൂർണ്ണസംഖ്യ depth, bool style)
{
	unsigned പൂർണ്ണസംഖ്യ i;
	ഘടന box *c, *prev;

	വേണ്ടി (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	എങ്കിൽ (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[മുകൾ_2], box->margin[ഇടത്_2],
		box->margin[BOTTOM], box->margin[വലത്_2]);

	തിരഞ്ഞെടുപ്പ് (box->type) {
	അവസ്ഥ BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		നിർത്തുക;

	അവസ്ഥ BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		നിർത്തുക;

	അവസ്ഥ BOX_INLINE:
		fprintf(stream, "INLINE ");
		നിർത്തുക;

	അവസ്ഥ BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		നിർത്തുക;

	അവസ്ഥ BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		നിർത്തുക;

	അവസ്ഥ BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		നിർത്തുക;

	അവസ്ഥ BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		നിർത്തുക;

	അവസ്ഥ BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		നിർത്തുക;

	അവസ്ഥ BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		നിർത്തുക;

	അവസ്ഥ BOX_FLOAT_ഇടത്:
		fprintf(stream, "FLOAT_LEFT ");
		നിർത്തുക;

	അവസ്ഥ BOX_FLOAT_വലത്:
		fprintf(stream, "FLOAT_RIGHT ");
		നിർത്തുക;

	അവസ്ഥ BOX_BR:
		fprintf(stream, "BR ");
		നിർത്തുക;

	അവസ്ഥ BOX_TEXT:
		fprintf(stream, "TEXT ");
		നിർത്തുക;

	അവസ്ഥ BOX_FLEX:
		fprintf(stream, "FLEX ");
		നിർത്തുക;

	അവസ്ഥ BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		fprintf(stream, "Unknown box type ");
	}

	എങ്കിൽ (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(പൂർണ്ണസംഖ്യ) box->length, box->text);
	എങ്കിൽ (box->space)
		fprintf(stream, "space ");
	എങ്കിൽ (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	എങ്കിൽ (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	എങ്കിൽ (box->gadget)
		fprintf(stream, "(gadget) ");
	എങ്കിൽ (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	എങ്കിൽ (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	എങ്കിൽ (box->target)
		fprintf(stream, " |%s|", box->target);
	എങ്കിൽ (box->title)
		fprintf(stream, " [%s]", box->title);
	എങ്കിൽ (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	എങ്കിൽ (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	എങ്കിൽ (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	എങ്കിൽ (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	എങ്കിൽ (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	എങ്കിൽ (box->col) {
		fprintf(stream, " (columns");
		വേണ്ടി (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((സ്ഥിരം അക്ഷരം *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((സ്ഥിരം അക്ഷരം *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	എങ്കിൽ (box->കണ്ണി != ശൂന്യ) {
		dom_string *name;
		എങ്കിൽ (dom_കണ്ണി_get_കണ്ണി_name(box->കണ്ണി, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	എങ്കിൽ (box->list_marker) {
		വേണ്ടി (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	വേണ്ടി (c = box->children; c && c->next; c = c->next)
		;
	എങ്കിൽ (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	വേണ്ടി (prev = 0, c = box->children; c; prev = c, c = c->next) {
		എങ്കിൽ (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		എങ്കിൽ (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(സ്ഥിരം ഘടന box * സ്ഥിരം box)
{
	തിരികെ box->padding[മുകൾ_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(സ്ഥിരം ഘടന box * സ്ഥിരം box)
{
	തിരികെ box->padding[ഇടത്_2] +
		box->width +
		box->padding[വലത്_2] +
		box->border[വലത്_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ഘടന box *
box_pick_text_box(ഘടന html_content *html,
		  പൂർണ്ണസംഖ്യ x, പൂർണ്ണസംഖ്യ y,
		  പൂർണ്ണസംഖ്യ dir,
		  പൂർണ്ണസംഖ്യ *dx, പൂർണ്ണസംഖ്യ *dy)
{
	ഘടന box *text_box = ശൂന്യ;
	ഘടന box *box;
	പൂർണ്ണസംഖ്യ nr_xd, nr_yd;
	പൂർണ്ണസംഖ്യ bx, by;
	പൂർണ്ണസംഖ്യ fx, fy;
	പൂർണ്ണസംഖ്യ tx, ty;

	എങ്കിൽ (html == ശൂന്യ)
		തിരികെ ശൂന്യ;

	box = html->layout;
	bx = box->margin[ഇടത്_2];
	by = box->margin[മുകൾ_2];
	fx = bx;
	fy = by;

	എങ്കിൽ (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		എങ്കിൽ (text_box && text_box->text && !text_box->object) {
			പൂർണ്ണസംഖ്യ w = (text_box->padding[ഇടത്_2] +
				 text_box->width +
				 text_box->padding[വലത്_2]);
			പൂർണ്ണസംഖ്യ h = (text_box->padding[മുകൾ_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			പൂർണ്ണസംഖ്യ x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			എങ്കിൽ (x < tx) x = tx;
			എങ്കിൽ (y < ty) y = ty;
			എങ്കിൽ (y > y1) y = y1;
			എങ്കിൽ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	തിരികെ text_box;
}
