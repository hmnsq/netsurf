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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/utils.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "utils/errors.h"
#অন্তর্ভাব "netsurf/types.h"
#অন্তর্ভাব "netsurf/content.h"
#অন্তর্ভাব "netsurf/mouse.h"
#অন্তর্ভাব "css/utils.h"
#অন্তর্ভাব "css/dump.h"
#অন্তর্ভাব "desktop/scrollbar.h"

#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
গণনা box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_পরবর্তী_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_পরবর্তী_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_বাম ||	\
			   box->type == BOX_FLOAT_ডান)

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
স্থির bool
box_contains_point(ধ্রুবক css_unit_ctx *unit_len_ctx,
		   ধ্রুবক কাঠামো box *box,
		   পূর্ণসংখ্যা x,
		   পূর্ণসংখ্যা y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	যদি (box->style != ফাঁকা &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		কাঠামো rect r = {
				 .x0 = box->border[বাম_2].width,
				 .y0 = box->border[শীর্ষ_2].width,
				 .x1 = box->padding[বাম_2] + box->width +
				 box->border[ডান_2].width +
				 box->padding[ডান_2],
				 .y1 = box->padding[শীর্ষ_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		যদি (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} অন্যথা {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		যদি (css_rect.বাম_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.বাম,
						css_rect.lunit));
		}
		যদি (css_rect.শীর্ষ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.শীর্ষ,
						css_rect.tunit));
		}
		যদি (css_rect.ডান_auto == false) {
			r.x1 = box->border[বাম_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ডান,
						css_rect.runit));
		}
		যদি (css_rect.bottom_auto == false) {
			r.y1 = box->border[শীর্ষ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		যদি (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			ফেরত true;
		}

		/* Not inside clip area */
		ফেরত false;
	}
	যদি (x >= -box->border[বাম_2].width &&
	    x < box->padding[বাম_2] + box->width +
	    box->padding[ডান_2] + box->border[ডান_2].width &&
	    y >= -box->border[শীর্ষ_2].width &&
	    y < box->padding[শীর্ষ_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		ফেরত true;
	}
	যদি (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[বাম_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[বাম_2] +
	    box->list_marker->width +
	    box->list_marker->border[ডান_2].width +
	    box->list_marker->padding[ডান_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[শীর্ষ_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[শীর্ষ_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		ফেরত true;
	}
	যদি ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		যদি (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			ফেরত true;
		}
	}
	যদি ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		যদি (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			ফেরত true;
		}
	}
	ফেরত false;
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
স্থির অন্তঃরেখ কাঠামো box *
box_move_xy(কাঠামো box *b, গণনা box_walk_dir dir, পূর্ণসংখ্যা *x, পূর্ণসংখ্যা *y)
{
	কাঠামো box *rb = ফাঁকা;

	নির্বাচন (dir) {
	ক্ষেত্র BOX_WALK_CHILDREN:
		b = b->children;
		যদি (b == ফাঁকা)
			বিরতি;
		*x += b->x;
		*y += b->y;
		যদি (!box_is_float(b)) {
			rb = b;
			বিরতি;
		}
		fallthrough;

	ক্ষেত্র BOX_WALK_পরবর্তী_SIBLING:
		করো {
			*x -= b->x;
			*y -= b->y;
			b = b->পরবর্তী;
			যদি (b == ফাঁকা)
				বিরতি;
			*x += b->x;
			*y += b->y;
		} যখন (box_is_float(b));
		rb = b;
		বিরতি;

	ক্ষেত্র BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		বিরতি;

	ক্ষেত্র BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		যদি (b == ফাঁকা)
			বিরতি;
		*x += b->x;
		*y += b->y;
		rb = b;
		বিরতি;

	ক্ষেত্র BOX_WALK_পরবর্তী_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->পরবর্তী_float;
		যদি (b == ফাঁকা)
			বিরতি;
		*x += b->x;
		*y += b->y;
		rb = b;
		বিরতি;

	ক্ষেত্র BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		বিরতি;

	পূর্বনির্ধারিত:
		প্রতিজ্ঞা(0 && "Bad box walk type.");
	}

	ফেরত rb;
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
স্থির অন্তঃরেখ কাঠামো box *
box_পরবর্তী_xy(কাঠামো box *b, পূর্ণসংখ্যা *x, পূর্ণসংখ্যা *y, bool skip_children)
{
	কাঠামো box *n;
	পূর্ণসংখ্যা tx, ty;

	প্রতিজ্ঞা(b != ফাঁকা);

	যদি (skip_children) {
		/* Caller is not interested in any kind of children */
		যাও skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	যদি (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		ফেরত n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	যদি (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		ফেরত n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_পরবর্তী_FLOAT_SIBLING, &tx, &ty);
	যদি (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		ফেরত n;
	}

	যদি (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		যাও done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	যখন (b) {
		যখন (!b->পরবর্তী && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			যদি (box_is_float(b)) {
				/* Go on to next float, if there is one */
				যাও skip_children;
			}
		}
		যদি (!b->পরবর্তী) {
			/* No more boxes */
			ফেরত ফাঁকা;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_পরবর্তী_SIBLING, &tx, &ty);
		যদি (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			ফেরত n;

		} অন্যথা যদি (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			যদি (box_is_float(b)) {
				/* Go on to next float, if there is one */
				যাও skip_children;
			}

		} অন্যথা {
			/* No more boxes */
			ফেরত ফাঁকা;
		}
	}

	প্রতিজ্ঞা(b != ফাঁকা);
	ফেরত ফাঁকা;
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
স্থির bool
box_nearer_text_box(কাঠামো box *box,
		    পূর্ণসংখ্যা bx, পূর্ণসংখ্যা by,
		    পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		    পূর্ণসংখ্যা dir,
		    কাঠামো box **nearest,
		    পূর্ণসংখ্যা *tx, পূর্ণসংখ্যা *ty,
		    পূর্ণসংখ্যা *nr_xd, পূর্ণসংখ্যা *nr_yd)
{
	পূর্ণসংখ্যা w = box->padding[বাম_2] + box->width + box->padding[ডান_2];
	পূর্ণসংখ্যা h = box->padding[শীর্ষ_2] + box->height + box->padding[BOTTOM];
	পূর্ণসংখ্যা y1 = by + h;
	পূর্ণসংখ্যা x1 = bx + w;
	পূর্ণসংখ্যা yd = INT_MAX;
	পূর্ণসংখ্যা xd = INT_MAX;

	যদি (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		ফেরত true;
	}

	যদি (box->parent->list_marker != box) {
		যদি (dir < 0) {
			/* consider only those children (partly) above-left */
			যদি (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} অন্যথা {
			/* consider only those children (partly) below-right */
			যদি (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		যদি (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	ফেরত false;
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
স্থির bool
box_nearest_text_box(কাঠামো box *box,
		     পূর্ণসংখ্যা bx, পূর্ণসংখ্যা by,
		     পূর্ণসংখ্যা fx, পূর্ণসংখ্যা fy,
		     পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		     পূর্ণসংখ্যা dir,
		     কাঠামো box **nearest,
		     পূর্ণসংখ্যা *tx, পূর্ণসংখ্যা *ty,
		     পূর্ণসংখ্যা *nr_xd, পূর্ণসংখ্যা *nr_yd)
{
	কাঠামো box *child = box->children;
	পূর্ণসংখ্যা c_bx, c_by;
	পূর্ণসংখ্যা c_fx, c_fy;
	bool in_box = false;

	যদি (*nearest == ফাঁকা) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	যদি (box->type == BOX_INLINE_CONTAINER) {
		পূর্ণসংখ্যা bw = box->padding[বাম_2] + box->width + box->padding[ডান_2];
		পূর্ণসংখ্যা bh = box->padding[শীর্ষ_2] + box->height + box->padding[BOTTOM];
		পূর্ণসংখ্যা b_y1 = by + bh;
		পূর্ণসংখ্যা b_x1 = bx + bw;
		যদি (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	যখন (child) {
		যদি (child->type == BOX_FLOAT_বাম ||
		    child->type == BOX_FLOAT_ডান) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} অন্যথা {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		যদি (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} অন্যথা {
			c_fx = fx;
			c_fy = fy;
		}
		যদি (in_box && child->text && !child->object) {
			যদি (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				ফেরত true;
		} অন্যথা {
			যদি (child->list_marker) {
				যদি (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					ফেরত true;
			}
			যদি (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				ফেরত true;
		}
		child = child->পরবর্তী;
	}

	ফেরত false;
}


/* Exported function documented in html/box.h */
শূন্য box_coords(কাঠামো box *box, পূর্ণসংখ্যা *x, পূর্ণসংখ্যা *y)
{
	*x = box->x;
	*y = box->y;
	যখন (box->parent) {
		যদি (box_is_float(box)) {
			প্রতিজ্ঞা(box->float_container);
			box = box->float_container;
		} অন্যথা {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
শূন্য box_bounds(কাঠামো box *box, কাঠামো rect *r)
{
	পূর্ণসংখ্যা width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[বাম_2] + box->width + box->padding[ডান_2];
	height = box->padding[শীর্ষ_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
কাঠামো box *
box_at_point(ধ্রুবক css_unit_ctx *unit_len_ctx,
	     কাঠামো box *box,
	     ধ্রুবক পূর্ণসংখ্যা x, ধ্রুবক পূর্ণসংখ্যা y,
	     পূর্ণসংখ্যা *box_x, পূর্ণসংখ্যা *box_y)
{
	bool skip_children;
	bool physically;

	প্রতিজ্ঞা(box);

	skip_children = false;
	যখন ((box = box_পরবর্তী_xy(box, box_x, box_y, skip_children))) {
		যদি (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			যদি (physically)
				ফেরত box;

			skip_children = false;
		} অন্যথা {
			skip_children = true;
		}
	}

	ফেরত ফাঁকা;
}


/* Exported function documented in html/box.h */
কাঠামো box *box_find_by_id(কাঠামো box *box, lwc_string *id)
{
	কাঠামো box *a, *b;
	bool m;

	যদি (box->id != ফাঁকা &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		ফেরত box;
	}

	জন্য (a = box->children; a; a = a->পরবর্তী) {
		যদি ((b = box_find_by_id(a, id)) != ফাঁকা) {
			ফেরত b;
		}
	}

	ফেরত ফাঁকা;
}


/* Exported function documented in html/box.h */
bool box_visible(কাঠামো box *box)
{
	/* visibility: hidden */
	যদি (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		ফেরত false;
	}

	ফেরত true;
}


/* Exported function documented in html/box.h */
শূন্য box_dump(FILE *stream, কাঠামো box *box, unsigned পূর্ণসংখ্যা depth, bool style)
{
	unsigned পূর্ণসংখ্যা i;
	কাঠামো box *c, *পূর্ব;

	জন্য (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	যদি (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[শীর্ষ_2], box->margin[বাম_2],
		box->margin[BOTTOM], box->margin[ডান_2]);

	নির্বাচন (box->type) {
	ক্ষেত্র BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		বিরতি;

	ক্ষেত্র BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		বিরতি;

	ক্ষেত্র BOX_INLINE:
		fprintf(stream, "INLINE ");
		বিরতি;

	ক্ষেত্র BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		বিরতি;

	ক্ষেত্র BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		বিরতি;

	ক্ষেত্র BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		বিরতি;

	ক্ষেত্র BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		বিরতি;

	ক্ষেত্র BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		বিরতি;

	ক্ষেত্র BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		বিরতি;

	ক্ষেত্র BOX_FLOAT_বাম:
		fprintf(stream, "FLOAT_LEFT ");
		বিরতি;

	ক্ষেত্র BOX_FLOAT_ডান:
		fprintf(stream, "FLOAT_RIGHT ");
		বিরতি;

	ক্ষেত্র BOX_BR:
		fprintf(stream, "BR ");
		বিরতি;

	ক্ষেত্র BOX_TEXT:
		fprintf(stream, "TEXT ");
		বিরতি;

	ক্ষেত্র BOX_FLEX:
		fprintf(stream, "FLEX ");
		বিরতি;

	ক্ষেত্র BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		বিরতি;

	পূর্বনির্ধারিত:
		fprintf(stream, "Unknown box type ");
	}

	যদি (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(পূর্ণসংখ্যা) box->দৈর্ঘ্য, box->text);
	যদি (box->space)
		fprintf(stream, "space ");
	যদি (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	যদি (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	যদি (box->gadget)
		fprintf(stream, "(gadget) ");
	যদি (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	যদি (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	যদি (box->target)
		fprintf(stream, " |%s|", box->target);
	যদি (box->title)
		fprintf(stream, " [%s]", box->title);
	যদি (box->id)
		fprintf(stream, " ID:%s", lwc_string_তথ্য(box->id));
	যদি (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	যদি (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	যদি (box->পরবর্তী_float)
		fprintf(stream, " next_float %p", box->পরবর্তী_float);
	যদি (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	যদি (box->col) {
		fprintf(stream, " (columns");
		জন্য (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ধ্রুবক অক্ষর *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ধ্রুবক অক্ষর *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	যদি (box->গ্রন্থি != ফাঁকা) {
		dom_string *name;
		যদি (dom_গ্রন্থি_get_গ্রন্থি_name(box->গ্রন্থি, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_তথ্য(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	যদি (box->list_marker) {
		জন্য (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	জন্য (c = box->children; c && c->পরবর্তী; c = c->পরবর্তী)
		;
	যদি (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	জন্য (পূর্ব = 0, c = box->children; c; পূর্ব = c, c = c->পরবর্তী) {
		যদি (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		যদি (c->পূর্ব != পূর্ব)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->পূর্ব, পূর্ব);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ধ্রুবক কাঠামো box * ধ্রুবক box)
{
	ফেরত box->padding[শীর্ষ_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ধ্রুবক কাঠামো box * ধ্রুবক box)
{
	ফেরত box->padding[বাম_2] +
		box->width +
		box->padding[ডান_2] +
		box->border[ডান_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
কাঠামো box *
box_pick_text_box(কাঠামো html_content *html,
		  পূর্ণসংখ্যা x, পূর্ণসংখ্যা y,
		  পূর্ণসংখ্যা dir,
		  পূর্ণসংখ্যা *dx, পূর্ণসংখ্যা *dy)
{
	কাঠামো box *text_box = ফাঁকা;
	কাঠামো box *box;
	পূর্ণসংখ্যা nr_xd, nr_yd;
	পূর্ণসংখ্যা bx, by;
	পূর্ণসংখ্যা fx, fy;
	পূর্ণসংখ্যা tx, ty;

	যদি (html == ফাঁকা)
		ফেরত ফাঁকা;

	box = html->layout;
	bx = box->margin[বাম_2];
	by = box->margin[শীর্ষ_2];
	fx = bx;
	fy = by;

	যদি (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		যদি (text_box && text_box->text && !text_box->object) {
			পূর্ণসংখ্যা w = (text_box->padding[বাম_2] +
				 text_box->width +
				 text_box->padding[ডান_2]);
			পূর্ণসংখ্যা h = (text_box->padding[শীর্ষ_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			পূর্ণসংখ্যা x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			যদি (x < tx) x = tx;
			যদি (y < ty) y = ty;
			যদি (y > y1) y = y1;
			যদি (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	ফেরত text_box;
}
