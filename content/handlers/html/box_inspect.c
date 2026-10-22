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

#include <stdio.h>
#include <dom/dom.h>

#include "utils/utils.h"
#include "utils/nsurl.h"
#include "utils/errors.h"
#include "netsurf/types.h"
#include "netsurf/content.h"
#include "netsurf/mouse.h"
#include "css/utils.h"
#include "css/dump.h"
#include "desktop/scrollbar.h"

#include "html/private.h"
#include "html/box.h"
#include "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
ga_nanaa box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_LEFT ||	\
			   box->type == BOX_FLOAT_RIGHT)

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
static bool
box_contains_point(const css_unit_ctx *unit_len_ctx,
		   const sa_nracanaa box *box,
		   puur_naa_nka x,
		   puur_naa_nka y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ᯀᯮᯝ (box->style != NULL &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		sa_nracanaa rect r = {
				 .x0 = box->border[LEFT].width,
				 .y0 = box->border[TOP].width,
				 .x1 = box->padding[LEFT] + box->width +
				 box->border[RIGHT].width +
				 box->padding[RIGHT],
				 .y1 = box->padding[TOP] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		ᯀᯮᯝ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ᯀᯮᯝᯂᯪ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ᯀᯮᯝ (css_rect.left_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.left,
						css_rect.lunit));
		}
		ᯀᯮᯝ (css_rect.top_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.top,
						css_rect.tunit));
		}
		ᯀᯮᯝ (css_rect.right_auto == false) {
			r.x1 = box->border[LEFT].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.right,
						css_rect.runit));
		}
		ᯀᯮᯝ (css_rect.bottom_auto == false) {
			r.y1 = box->border[TOP].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ᯀᯮᯝ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ᯀᯮᯞ true;
		}

		/* Not inside clip area */
		ᯀᯮᯞ false;
	}
	ᯀᯮᯝ (x >= -box->border[LEFT].width &&
	    x < box->padding[LEFT] + box->width +
	    box->padding[RIGHT] + box->border[RIGHT].width &&
	    y >= -box->border[TOP].width &&
	    y < box->padding[TOP] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		ᯀᯮᯞ true;
	}
	ᯀᯮᯝ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[LEFT].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[LEFT] +
	    box->list_marker->width +
	    box->list_marker->border[RIGHT].width +
	    box->list_marker->padding[RIGHT] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[TOP].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[TOP] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ᯀᯮᯞ true;
	}
	ᯀᯮᯝ ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ᯀᯮᯝ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ᯀᯮᯞ true;
		}
	}
	ᯀᯮᯝ ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		ᯀᯮᯝ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ᯀᯮᯞ true;
		}
	}
	ᯀᯮᯞ false;
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
static anta_hstha sa_nracanaa box *
box_move_xy(sa_nracanaa box *b, ga_nanaa box_walk_dir dir, puur_naa_nka *x, puur_naa_nka *y)
{
	sa_nracanaa box *rb = NULL;

	switch (dir) {
	pak_sa BOX_WALK_CHILDREN:
		b = b->children;
		ᯀᯮᯝ (b == NULL)
			bha_nga;
		*x += b->x;
		*y += b->y;
		ᯀᯮᯝ (!box_is_float(b)) {
			rb = b;
			bha_nga;
		}
		fallthrough;

	pak_sa BOX_WALK_NEXT_SIBLING:
		do {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			ᯀᯮᯝ (b == NULL)
				bha_nga;
			*x += b->x;
			*y += b->y;
		} ᯀᯬᯝ (box_is_float(b));
		rb = b;
		bha_nga;

	pak_sa BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		bha_nga;

	pak_sa BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ᯀᯮᯝ (b == NULL)
			bha_nga;
		*x += b->x;
		*y += b->y;
		rb = b;
		bha_nga;

	pak_sa BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		ᯀᯮᯝ (b == NULL)
			bha_nga;
		*x += b->x;
		*y += b->y;
		rb = b;
		bha_nga;

	pak_sa BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		bha_nga;

	default:
		assert(0 && "Bad box walk type.");
	}

	ᯀᯮᯞ rb;
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
static anta_hstha sa_nracanaa box *
box_next_xy(sa_nracanaa box *b, puur_naa_nka *x, puur_naa_nka *y, bool skip_children)
{
	sa_nracanaa box *n;
	puur_naa_nka tx, ty;

	assert(b != NULL);

	ᯀᯮᯝ (skip_children) {
		/* Caller is not interested in any kind of children */
		goto skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ᯀᯮᯝ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ᯀᯮᯞ n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ᯀᯮᯝ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ᯀᯮᯞ n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	ᯀᯮᯝ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ᯀᯮᯞ n;
	}

	ᯀᯮᯝ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		goto done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	ᯀᯬᯝ (b) {
		ᯀᯬᯝ (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ᯀᯮᯝ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				goto skip_children;
			}
		}
		ᯀᯮᯝ (!b->next) {
			/* No more boxes */
			ᯀᯮᯞ NULL;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		ᯀᯮᯝ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ᯀᯮᯞ n;

		} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			ᯀᯮᯝ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				goto skip_children;
			}

		} ᯀᯮᯝᯂᯪ {
			/* No more boxes */
			ᯀᯮᯞ NULL;
		}
	}

	assert(b != NULL);
	ᯀᯮᯞ NULL;
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
static bool
box_nearer_text_box(sa_nracanaa box *box,
		    puur_naa_nka bx, puur_naa_nka by,
		    puur_naa_nka x, puur_naa_nka y,
		    puur_naa_nka dir,
		    sa_nracanaa box **nearest,
		    puur_naa_nka *tx, puur_naa_nka *ty,
		    puur_naa_nka *nr_xd, puur_naa_nka *nr_yd)
{
	puur_naa_nka w = box->padding[LEFT] + box->width + box->padding[RIGHT];
	puur_naa_nka h = box->padding[TOP] + box->height + box->padding[BOTTOM];
	puur_naa_nka y1 = by + h;
	puur_naa_nka x1 = bx + w;
	puur_naa_nka yd = INT_MAX;
	puur_naa_nka xd = INT_MAX;

	ᯀᯮᯝ (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		ᯀᯮᯞ true;
	}

	ᯀᯮᯝ (box->parent->list_marker != box) {
		ᯀᯮᯝ (dir < 0) {
			/* consider only those children (partly) above-left */
			ᯀᯮᯝ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ᯀᯮᯝᯂᯪ {
			/* consider only those children (partly) below-right */
			ᯀᯮᯝ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ᯀᯮᯝ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	ᯀᯮᯞ false;
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
static bool
box_nearest_text_box(sa_nracanaa box *box,
		     puur_naa_nka bx, puur_naa_nka by,
		     puur_naa_nka fx, puur_naa_nka fy,
		     puur_naa_nka x, puur_naa_nka y,
		     puur_naa_nka dir,
		     sa_nracanaa box **nearest,
		     puur_naa_nka *tx, puur_naa_nka *ty,
		     puur_naa_nka *nr_xd, puur_naa_nka *nr_yd)
{
	sa_nracanaa box *child = box->children;
	puur_naa_nka c_bx, c_by;
	puur_naa_nka c_fx, c_fy;
	bool in_box = false;

	ᯀᯮᯝ (*nearest == NULL) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	ᯀᯮᯝ (box->type == BOX_INLINE_CONTAINER) {
		puur_naa_nka bw = box->padding[LEFT] + box->width + box->padding[RIGHT];
		puur_naa_nka bh = box->padding[TOP] + box->height + box->padding[BOTTOM];
		puur_naa_nka b_y1 = by + bh;
		puur_naa_nka b_x1 = bx + bw;
		ᯀᯮᯝ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	ᯀᯬᯝ (child) {
		ᯀᯮᯝ (child->type == BOX_FLOAT_LEFT ||
		    child->type == BOX_FLOAT_RIGHT) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} ᯀᯮᯝᯂᯪ {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		ᯀᯮᯝ (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ᯀᯮᯝᯂᯪ {
			c_fx = fx;
			c_fy = fy;
		}
		ᯀᯮᯝ (in_box && child->text && !child->object) {
			ᯀᯮᯝ (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				ᯀᯮᯞ true;
		} ᯀᯮᯝᯂᯪ {
			ᯀᯮᯝ (child->list_marker) {
				ᯀᯮᯝ (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					ᯀᯮᯞ true;
			}
			ᯀᯮᯝ (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				ᯀᯮᯞ true;
		}
		child = child->next;
	}

	ᯀᯮᯞ false;
}


/* Exported function documented in html/box.h */
void box_coords(sa_nracanaa box *box, puur_naa_nka *x, puur_naa_nka *y)
{
	*x = box->x;
	*y = box->y;
	ᯀᯬᯝ (box->parent) {
		ᯀᯮᯝ (box_is_float(box)) {
			assert(box->float_container);
			box = box->float_container;
		} ᯀᯮᯝᯂᯪ {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
void box_bounds(sa_nracanaa box *box, sa_nracanaa rect *r)
{
	puur_naa_nka width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[LEFT] + box->width + box->padding[RIGHT];
	height = box->padding[TOP] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
sa_nracanaa box *
box_at_point(const css_unit_ctx *unit_len_ctx,
	     sa_nracanaa box *box,
	     const puur_naa_nka x, const puur_naa_nka y,
	     puur_naa_nka *box_x, puur_naa_nka *box_y)
{
	bool skip_children;
	bool physically;

	assert(box);

	skip_children = false;
	ᯀᯬᯝ ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		ᯀᯮᯝ (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			ᯀᯮᯝ (physically)
				ᯀᯮᯞ box;

			skip_children = false;
		} ᯀᯮᯝᯂᯪ {
			skip_children = true;
		}
	}

	ᯀᯮᯞ NULL;
}


/* Exported function documented in html/box.h */
sa_nracanaa box *box_find_by_id(sa_nracanaa box *box, lwc_string *id)
{
	sa_nracanaa box *a, *b;
	bool m;

	ᯀᯮᯝ (box->id != NULL &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ᯀᯮᯞ box;
	}

	for (a = box->children; a; a = a->next) {
		ᯀᯮᯝ ((b = box_find_by_id(a, id)) != NULL) {
			ᯀᯮᯞ b;
		}
	}

	ᯀᯮᯞ NULL;
}


/* Exported function documented in html/box.h */
bool box_visible(sa_nracanaa box *box)
{
	/* visibility: hidden */
	ᯀᯮᯝ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ᯀᯮᯞ false;
	}

	ᯀᯮᯞ true;
}


/* Exported function documented in html/box.h */
void box_dump(FILE *stream, sa_nracanaa box *box, unsigned puur_naa_nka depth, bool style)
{
	unsigned puur_naa_nka i;
	sa_nracanaa box *c, *prev;

	for (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	ᯀᯮᯝ (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[TOP], box->margin[LEFT],
		box->margin[BOTTOM], box->margin[RIGHT]);

	switch (box->type) {
	pak_sa BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		bha_nga;

	pak_sa BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		bha_nga;

	pak_sa BOX_INLINE:
		fprintf(stream, "INLINE ");
		bha_nga;

	pak_sa BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		bha_nga;

	pak_sa BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		bha_nga;

	pak_sa BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		bha_nga;

	pak_sa BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		bha_nga;

	pak_sa BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		bha_nga;

	pak_sa BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		bha_nga;

	pak_sa BOX_FLOAT_LEFT:
		fprintf(stream, "FLOAT_LEFT ");
		bha_nga;

	pak_sa BOX_FLOAT_RIGHT:
		fprintf(stream, "FLOAT_RIGHT ");
		bha_nga;

	pak_sa BOX_BR:
		fprintf(stream, "BR ");
		bha_nga;

	pak_sa BOX_TEXT:
		fprintf(stream, "TEXT ");
		bha_nga;

	pak_sa BOX_FLEX:
		fprintf(stream, "FLEX ");
		bha_nga;

	pak_sa BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		bha_nga;

	default:
		fprintf(stream, "Unknown box type ");
	}

	ᯀᯮᯝ (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(puur_naa_nka) box->length, box->text);
	ᯀᯮᯝ (box->space)
		fprintf(stream, "space ");
	ᯀᯮᯝ (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ᯀᯮᯝ (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	ᯀᯮᯝ (box->gadget)
		fprintf(stream, "(gadget) ");
	ᯀᯮᯝ (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	ᯀᯮᯝ (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	ᯀᯮᯝ (box->target)
		fprintf(stream, " |%s|", box->target);
	ᯀᯮᯝ (box->title)
		fprintf(stream, " [%s]", box->title);
	ᯀᯮᯝ (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	ᯀᯮᯝ (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	ᯀᯮᯝ (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	ᯀᯮᯝ (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	ᯀᯮᯝ (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	ᯀᯮᯝ (box->col) {
		fprintf(stream, " (columns");
		for (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((const var_na *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((const var_na *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	ᯀᯮᯝ (box->node != NULL) {
		dom_string *name;
		ᯀᯮᯝ (dom_node_get_node_name(box->node, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	ᯀᯮᯝ (box->list_marker) {
		for (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	for (c = box->children; c && c->next; c = c->next)
		;
	ᯀᯮᯝ (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	for (prev = 0, c = box->children; c; prev = c, c = c->next) {
		ᯀᯮᯝ (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		ᯀᯮᯝ (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(const sa_nracanaa box * const box)
{
	ᯀᯮᯞ box->padding[TOP] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(const sa_nracanaa box * const box)
{
	ᯀᯮᯞ box->padding[LEFT] +
		box->width +
		box->padding[RIGHT] +
		box->border[RIGHT].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
sa_nracanaa box *
box_pick_text_box(sa_nracanaa html_content *html,
		  puur_naa_nka x, puur_naa_nka y,
		  puur_naa_nka dir,
		  puur_naa_nka *dx, puur_naa_nka *dy)
{
	sa_nracanaa box *text_box = NULL;
	sa_nracanaa box *box;
	puur_naa_nka nr_xd, nr_yd;
	puur_naa_nka bx, by;
	puur_naa_nka fx, fy;
	puur_naa_nka tx, ty;

	ᯀᯮᯝ (html == NULL)
		ᯀᯮᯞ NULL;

	box = html->layout;
	bx = box->margin[LEFT];
	by = box->margin[TOP];
	fx = bx;
	fy = by;

	ᯀᯮᯝ (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ᯀᯮᯝ (text_box && text_box->text && !text_box->object) {
			puur_naa_nka w = (text_box->padding[LEFT] +
				 text_box->width +
				 text_box->padding[RIGHT]);
			puur_naa_nka h = (text_box->padding[TOP] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			puur_naa_nka x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ᯀᯮᯝ (x < tx) x = tx;
			ᯀᯮᯝ (y < ty) y = ty;
			ᯀᯮᯝ (y > y1) y = y1;
			ᯀᯮᯝ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ᯀᯮᯞ text_box;
}
