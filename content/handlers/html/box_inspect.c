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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/utils.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "utils/errors.h"
#அந்தர்பாவ "netsurf/types.h"
#அந்தர்பாவ "netsurf/content.h"
#அந்தர்பாவ "netsurf/mouse.h"
#அந்தர்பாவ "css/utils.h"
#அந்தர்பாவ "css/dump.h"
#அந்தர்பாவ "desktop/scrollbar.h"

#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
எண்ணிக்கை box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_இடது ||	\
			   box->type == BOX_FLOAT_வலது)

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
நிலையான bool
box_contains_point(மாறிலி css_unit_ctx *unit_len_ctx,
		   மாறிலி கட்டமைப்பு box *box,
		   முழு x,
		   முழு y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	எனில் (box->style != வெற்று &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		கட்டமைப்பு rect r = {
				 .x0 = box->border[இடது_2].width,
				 .y0 = box->border[உச்சி_2].width,
				 .x1 = box->padding[இடது_2] + box->width +
				 box->border[வலது_2].width +
				 box->padding[வலது_2],
				 .y1 = box->padding[உச்சி_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		எனில் (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} இல்லையெனில் {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		எனில் (css_rect.இடது_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.இடது,
						css_rect.lunit));
		}
		எனில் (css_rect.உச்சி_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.உச்சி,
						css_rect.tunit));
		}
		எனில் (css_rect.வலது_auto == false) {
			r.x1 = box->border[இடது_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.வலது,
						css_rect.runit));
		}
		எனில் (css_rect.bottom_auto == false) {
			r.y1 = box->border[உச்சி_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		எனில் (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			திரும்பு true;
		}

		/* Not inside clip area */
		திரும்பு false;
	}
	எனில் (x >= -box->border[இடது_2].width &&
	    x < box->padding[இடது_2] + box->width +
	    box->padding[வலது_2] + box->border[வலது_2].width &&
	    y >= -box->border[உச்சி_2].width &&
	    y < box->padding[உச்சி_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		திரும்பு true;
	}
	எனில் (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[இடது_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[இடது_2] +
	    box->list_marker->width +
	    box->list_marker->border[வலது_2].width +
	    box->list_marker->padding[வலது_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[உச்சி_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[உச்சி_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		திரும்பு true;
	}
	எனில் ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		எனில் (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			திரும்பு true;
		}
	}
	எனில் ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		எனில் (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			திரும்பு true;
		}
	}
	திரும்பு false;
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
நிலையான உள்வரி கட்டமைப்பு box *
box_move_xy(கட்டமைப்பு box *b, எண்ணிக்கை box_walk_dir dir, முழு *x, முழு *y)
{
	கட்டமைப்பு box *rb = வெற்று;

	தேர்வு (dir) {
	நிலை BOX_WALK_CHILDREN:
		b = b->children;
		எனில் (b == வெற்று)
			நிறுத்து;
		*x += b->x;
		*y += b->y;
		எனில் (!box_is_float(b)) {
			rb = b;
			நிறுத்து;
		}
		fallthrough;

	நிலை BOX_WALK_NEXT_SIBLING:
		செய் {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			எனில் (b == வெற்று)
				நிறுத்து;
			*x += b->x;
			*y += b->y;
		} போது (box_is_float(b));
		rb = b;
		நிறுத்து;

	நிலை BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		நிறுத்து;

	நிலை BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		எனில் (b == வெற்று)
			நிறுத்து;
		*x += b->x;
		*y += b->y;
		rb = b;
		நிறுத்து;

	நிலை BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		எனில் (b == வெற்று)
			நிறுத்து;
		*x += b->x;
		*y += b->y;
		rb = b;
		நிறுத்து;

	நிலை BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		நிறுத்து;

	இயல்பு:
		ப்ரதிஜ்ஞா(0 && "Bad box walk type.");
	}

	திரும்பு rb;
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
நிலையான உள்வரி கட்டமைப்பு box *
box_next_xy(கட்டமைப்பு box *b, முழு *x, முழு *y, bool skip_children)
{
	கட்டமைப்பு box *n;
	முழு tx, ty;

	ப்ரதிஜ்ஞா(b != வெற்று);

	எனில் (skip_children) {
		/* Caller is not interested in any kind of children */
		செல் skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	எனில் (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		திரும்பு n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	எனில் (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		திரும்பு n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	எனில் (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		திரும்பு n;
	}

	எனில் (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		செல் done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	போது (b) {
		போது (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			எனில் (box_is_float(b)) {
				/* Go on to next float, if there is one */
				செல் skip_children;
			}
		}
		எனில் (!b->next) {
			/* No more boxes */
			திரும்பு வெற்று;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		எனில் (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			திரும்பு n;

		} இல்லையெனில் எனில் (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			எனில் (box_is_float(b)) {
				/* Go on to next float, if there is one */
				செல் skip_children;
			}

		} இல்லையெனில் {
			/* No more boxes */
			திரும்பு வெற்று;
		}
	}

	ப்ரதிஜ்ஞா(b != வெற்று);
	திரும்பு வெற்று;
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
நிலையான bool
box_nearer_text_box(கட்டமைப்பு box *box,
		    முழு bx, முழு by,
		    முழு x, முழு y,
		    முழு dir,
		    கட்டமைப்பு box **nearest,
		    முழு *tx, முழு *ty,
		    முழு *nr_xd, முழு *nr_yd)
{
	முழு w = box->padding[இடது_2] + box->width + box->padding[வலது_2];
	முழு h = box->padding[உச்சி_2] + box->height + box->padding[BOTTOM];
	முழு y1 = by + h;
	முழு x1 = bx + w;
	முழு yd = INT_MAX;
	முழு xd = INT_MAX;

	எனில் (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		திரும்பு true;
	}

	எனில் (box->parent->list_marker != box) {
		எனில் (dir < 0) {
			/* consider only those children (partly) above-left */
			எனில் (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} இல்லையெனில் {
			/* consider only those children (partly) below-right */
			எனில் (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		எனில் (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	திரும்பு false;
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
நிலையான bool
box_nearest_text_box(கட்டமைப்பு box *box,
		     முழு bx, முழு by,
		     முழு fx, முழு fy,
		     முழு x, முழு y,
		     முழு dir,
		     கட்டமைப்பு box **nearest,
		     முழு *tx, முழு *ty,
		     முழு *nr_xd, முழு *nr_yd)
{
	கட்டமைப்பு box *child = box->children;
	முழு c_bx, c_by;
	முழு c_fx, c_fy;
	bool in_box = false;

	எனில் (*nearest == வெற்று) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	எனில் (box->type == BOX_INLINE_CONTAINER) {
		முழு bw = box->padding[இடது_2] + box->width + box->padding[வலது_2];
		முழு bh = box->padding[உச்சி_2] + box->height + box->padding[BOTTOM];
		முழு b_y1 = by + bh;
		முழு b_x1 = bx + bw;
		எனில் (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	போது (child) {
		எனில் (child->type == BOX_FLOAT_இடது ||
		    child->type == BOX_FLOAT_வலது) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} இல்லையெனில் {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		எனில் (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} இல்லையெனில் {
			c_fx = fx;
			c_fy = fy;
		}
		எனில் (in_box && child->text && !child->object) {
			எனில் (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				திரும்பு true;
		} இல்லையெனில் {
			எனில் (child->list_marker) {
				எனில் (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					திரும்பு true;
			}
			எனில் (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				திரும்பு true;
		}
		child = child->next;
	}

	திரும்பு false;
}


/* Exported function documented in html/box.h */
வெறுமை box_coords(கட்டமைப்பு box *box, முழு *x, முழு *y)
{
	*x = box->x;
	*y = box->y;
	போது (box->parent) {
		எனில் (box_is_float(box)) {
			ப்ரதிஜ்ஞா(box->float_container);
			box = box->float_container;
		} இல்லையெனில் {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
வெறுமை box_bounds(கட்டமைப்பு box *box, கட்டமைப்பு rect *r)
{
	முழு width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[இடது_2] + box->width + box->padding[வலது_2];
	height = box->padding[உச்சி_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
கட்டமைப்பு box *
box_at_point(மாறிலி css_unit_ctx *unit_len_ctx,
	     கட்டமைப்பு box *box,
	     மாறிலி முழு x, மாறிலி முழு y,
	     முழு *box_x, முழு *box_y)
{
	bool skip_children;
	bool physically;

	ப்ரதிஜ்ஞா(box);

	skip_children = false;
	போது ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		எனில் (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			எனில் (physically)
				திரும்பு box;

			skip_children = false;
		} இல்லையெனில் {
			skip_children = true;
		}
	}

	திரும்பு வெற்று;
}


/* Exported function documented in html/box.h */
கட்டமைப்பு box *box_find_by_id(கட்டமைப்பு box *box, lwc_string *id)
{
	கட்டமைப்பு box *a, *b;
	bool m;

	எனில் (box->id != வெற்று &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		திரும்பு box;
	}

	க்கு (a = box->children; a; a = a->next) {
		எனில் ((b = box_find_by_id(a, id)) != வெற்று) {
			திரும்பு b;
		}
	}

	திரும்பு வெற்று;
}


/* Exported function documented in html/box.h */
bool box_visible(கட்டமைப்பு box *box)
{
	/* visibility: hidden */
	எனில் (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		திரும்பு false;
	}

	திரும்பு true;
}


/* Exported function documented in html/box.h */
வெறுமை box_dump(FILE *stream, கட்டமைப்பு box *box, unsigned முழு depth, bool style)
{
	unsigned முழு i;
	கட்டமைப்பு box *c, *prev;

	க்கு (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	எனில் (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[உச்சி_2], box->margin[இடது_2],
		box->margin[BOTTOM], box->margin[வலது_2]);

	தேர்வு (box->type) {
	நிலை BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		நிறுத்து;

	நிலை BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		நிறுத்து;

	நிலை BOX_INLINE:
		fprintf(stream, "INLINE ");
		நிறுத்து;

	நிலை BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		நிறுத்து;

	நிலை BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		நிறுத்து;

	நிலை BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		நிறுத்து;

	நிலை BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		நிறுத்து;

	நிலை BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		நிறுத்து;

	நிலை BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		நிறுத்து;

	நிலை BOX_FLOAT_இடது:
		fprintf(stream, "FLOAT_LEFT ");
		நிறுத்து;

	நிலை BOX_FLOAT_வலது:
		fprintf(stream, "FLOAT_RIGHT ");
		நிறுத்து;

	நிலை BOX_BR:
		fprintf(stream, "BR ");
		நிறுத்து;

	நிலை BOX_TEXT:
		fprintf(stream, "TEXT ");
		நிறுத்து;

	நிலை BOX_FLEX:
		fprintf(stream, "FLEX ");
		நிறுத்து;

	நிலை BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		நிறுத்து;

	இயல்பு:
		fprintf(stream, "Unknown box type ");
	}

	எனில் (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(முழு) box->length, box->text);
	எனில் (box->space)
		fprintf(stream, "space ");
	எனில் (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	எனில் (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	எனில் (box->gadget)
		fprintf(stream, "(gadget) ");
	எனில் (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	எனில் (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	எனில் (box->target)
		fprintf(stream, " |%s|", box->target);
	எனில் (box->title)
		fprintf(stream, " [%s]", box->title);
	எனில் (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	எனில் (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	எனில் (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	எனில் (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	எனில் (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	எனில் (box->col) {
		fprintf(stream, " (columns");
		க்கு (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((மாறிலி எழுத்து *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((மாறிலி எழுத்து *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	எனில் (box->முனை != வெற்று) {
		dom_string *name;
		எனில் (dom_முனை_get_முனை_name(box->முனை, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	எனில் (box->list_marker) {
		க்கு (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	க்கு (c = box->children; c && c->next; c = c->next)
		;
	எனில் (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	க்கு (prev = 0, c = box->children; c; prev = c, c = c->next) {
		எனில் (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		எனில் (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(மாறிலி கட்டமைப்பு box * மாறிலி box)
{
	திரும்பு box->padding[உச்சி_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(மாறிலி கட்டமைப்பு box * மாறிலி box)
{
	திரும்பு box->padding[இடது_2] +
		box->width +
		box->padding[வலது_2] +
		box->border[வலது_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
கட்டமைப்பு box *
box_pick_text_box(கட்டமைப்பு html_content *html,
		  முழு x, முழு y,
		  முழு dir,
		  முழு *dx, முழு *dy)
{
	கட்டமைப்பு box *text_box = வெற்று;
	கட்டமைப்பு box *box;
	முழு nr_xd, nr_yd;
	முழு bx, by;
	முழு fx, fy;
	முழு tx, ty;

	எனில் (html == வெற்று)
		திரும்பு வெற்று;

	box = html->layout;
	bx = box->margin[இடது_2];
	by = box->margin[உச்சி_2];
	fx = bx;
	fy = by;

	எனில் (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		எனில் (text_box && text_box->text && !text_box->object) {
			முழு w = (text_box->padding[இடது_2] +
				 text_box->width +
				 text_box->padding[வலது_2]);
			முழு h = (text_box->padding[உச்சி_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			முழு x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			எனில் (x < tx) x = tx;
			எனில் (y < ty) y = ty;
			எனில் (y > y1) y = y1;
			எனில் (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	திரும்பு text_box;
}
