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

#شامل <stdio.h>
#شامل <dom/dom.h>

#شامل "utils/utils.h"
#شامل "utils/nsurl.h"
#شامل "utils/errors.h"
#شامل "netsurf/types.h"
#شامل "netsurf/content.h"
#شامل "netsurf/mouse.h"
#شامل "css/utils.h"
#شامل "css/dump.h"
#شامل "desktop/scrollbar.h"

#شامل "html/private.h"
#شامل "html/box.h"
#شامل "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
شمارش box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_چپ ||	\
			   box->type == BOX_FLOAT_راست)

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
ایستا bool
box_contains_point(ثابت css_unit_ctx *unit_len_ctx,
		   ثابت ساختار box *box,
		   صحیح x,
		   صحیح y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	اگر (box->style != پوچ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ساختار rect r = {
				 .x0 = box->border[چپ_2].width,
				 .y0 = box->border[بالاترین_2].width,
				 .x1 = box->padding[چپ_2] + box->width +
				 box->border[راست_2].width +
				 box->padding[راست_2],
				 .y1 = box->padding[بالاترین_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		اگر (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} وگرنه {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		اگر (css_rect.چپ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.چپ,
						css_rect.lunit));
		}
		اگر (css_rect.بالاترین_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.بالاترین,
						css_rect.tunit));
		}
		اگر (css_rect.راست_auto == false) {
			r.x1 = box->border[چپ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.راست,
						css_rect.runit));
		}
		اگر (css_rect.bottom_auto == false) {
			r.y1 = box->border[بالاترین_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		اگر (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			بازگشت true;
		}

		/* Not inside clip area */
		بازگشت false;
	}
	اگر (x >= -box->border[چپ_2].width &&
	    x < box->padding[چپ_2] + box->width +
	    box->padding[راست_2] + box->border[راست_2].width &&
	    y >= -box->border[بالاترین_2].width &&
	    y < box->padding[بالاترین_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		بازگشت true;
	}
	اگر (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[چپ_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[چپ_2] +
	    box->list_marker->width +
	    box->list_marker->border[راست_2].width +
	    box->list_marker->padding[راست_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[بالاترین_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[بالاترین_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		بازگشت true;
	}
	اگر ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		اگر (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			بازگشت true;
		}
	}
	اگر ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		اگر (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			بازگشت true;
		}
	}
	بازگشت false;
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
ایستا درونخطی ساختار box *
box_move_xy(ساختار box *b, شمارش box_walk_dir dir, صحیح *x, صحیح *y)
{
	ساختار box *rb = پوچ;

	انتخاب (dir) {
	حالت BOX_WALK_CHILDREN:
		b = b->children;
		اگر (b == پوچ)
			بشکن;
		*x += b->x;
		*y += b->y;
		اگر (!box_is_float(b)) {
			rb = b;
			بشکن;
		}
		fallthrough;

	حالت BOX_WALK_NEXT_SIBLING:
		انجامبده {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			اگر (b == پوچ)
				بشکن;
			*x += b->x;
			*y += b->y;
		} وقتی (box_is_float(b));
		rb = b;
		بشکن;

	حالت BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		بشکن;

	حالت BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		اگر (b == پوچ)
			بشکن;
		*x += b->x;
		*y += b->y;
		rb = b;
		بشکن;

	حالت BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		اگر (b == پوچ)
			بشکن;
		*x += b->x;
		*y += b->y;
		rb = b;
		بشکن;

	حالت BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		بشکن;

	پیشفرض:
		أكد(0 && "Bad box walk type.");
	}

	بازگشت rb;
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
ایستا درونخطی ساختار box *
box_next_xy(ساختار box *b, صحیح *x, صحیح *y, bool skip_children)
{
	ساختار box *n;
	صحیح tx, ty;

	أكد(b != پوچ);

	اگر (skip_children) {
		/* Caller is not interested in any kind of children */
		برو skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	اگر (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		بازگشت n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	اگر (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		بازگشت n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	اگر (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		بازگشت n;
	}

	اگر (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		برو done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	وقتی (b) {
		وقتی (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			اگر (box_is_float(b)) {
				/* Go on to next float, if there is one */
				برو skip_children;
			}
		}
		اگر (!b->next) {
			/* No more boxes */
			بازگشت پوچ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		اگر (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			بازگشت n;

		} وگرنه اگر (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			اگر (box_is_float(b)) {
				/* Go on to next float, if there is one */
				برو skip_children;
			}

		} وگرنه {
			/* No more boxes */
			بازگشت پوچ;
		}
	}

	أكد(b != پوچ);
	بازگشت پوچ;
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
ایستا bool
box_nearer_text_box(ساختار box *box,
		    صحیح bx, صحیح by,
		    صحیح x, صحیح y,
		    صحیح dir,
		    ساختار box **nearest,
		    صحیح *tx, صحیح *ty,
		    صحیح *nr_xd, صحیح *nr_yd)
{
	صحیح w = box->padding[چپ_2] + box->width + box->padding[راست_2];
	صحیح h = box->padding[بالاترین_2] + box->height + box->padding[BOTTOM];
	صحیح y1 = by + h;
	صحیح x1 = bx + w;
	صحیح yd = INT_MAX;
	صحیح xd = INT_MAX;

	اگر (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		بازگشت true;
	}

	اگر (box->parent->list_marker != box) {
		اگر (dir < 0) {
			/* consider only those children (partly) above-left */
			اگر (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} وگرنه {
			/* consider only those children (partly) below-right */
			اگر (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		اگر (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	بازگشت false;
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
ایستا bool
box_nearest_text_box(ساختار box *box,
		     صحیح bx, صحیح by,
		     صحیح fx, صحیح fy,
		     صحیح x, صحیح y,
		     صحیح dir,
		     ساختار box **nearest,
		     صحیح *tx, صحیح *ty,
		     صحیح *nr_xd, صحیح *nr_yd)
{
	ساختار box *child = box->children;
	صحیح c_bx, c_by;
	صحیح c_fx, c_fy;
	bool in_box = false;

	اگر (*nearest == پوچ) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	اگر (box->type == BOX_INLINE_CONTAINER) {
		صحیح bw = box->padding[چپ_2] + box->width + box->padding[راست_2];
		صحیح bh = box->padding[بالاترین_2] + box->height + box->padding[BOTTOM];
		صحیح b_y1 = by + bh;
		صحیح b_x1 = bx + bw;
		اگر (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	وقتی (child) {
		اگر (child->type == BOX_FLOAT_چپ ||
		    child->type == BOX_FLOAT_راست) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} وگرنه {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		اگر (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} وگرنه {
			c_fx = fx;
			c_fy = fy;
		}
		اگر (in_box && child->text && !child->object) {
			اگر (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				بازگشت true;
		} وگرنه {
			اگر (child->list_marker) {
				اگر (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					بازگشت true;
			}
			اگر (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				بازگشت true;
		}
		child = child->next;
	}

	بازگشت false;
}


/* Exported function documented in html/box.h */
تهی box_coords(ساختار box *box, صحیح *x, صحیح *y)
{
	*x = box->x;
	*y = box->y;
	وقتی (box->parent) {
		اگر (box_is_float(box)) {
			أكد(box->float_container);
			box = box->float_container;
		} وگرنه {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
تهی box_bounds(ساختار box *box, ساختار rect *r)
{
	صحیح width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[چپ_2] + box->width + box->padding[راست_2];
	height = box->padding[بالاترین_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
ساختار box *
box_at_point(ثابت css_unit_ctx *unit_len_ctx,
	     ساختار box *box,
	     ثابت صحیح x, ثابت صحیح y,
	     صحیح *box_x, صحیح *box_y)
{
	bool skip_children;
	bool physically;

	أكد(box);

	skip_children = false;
	وقتی ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		اگر (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			اگر (physically)
				بازگشت box;

			skip_children = false;
		} وگرنه {
			skip_children = true;
		}
	}

	بازگشت پوچ;
}


/* Exported function documented in html/box.h */
ساختار box *box_find_by_id(ساختار box *box, lwc_string *id)
{
	ساختار box *a, *b;
	bool m;

	اگر (box->id != پوچ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		بازگشت box;
	}

	برای (a = box->children; a; a = a->next) {
		اگر ((b = box_find_by_id(a, id)) != پوچ) {
			بازگشت b;
		}
	}

	بازگشت پوچ;
}


/* Exported function documented in html/box.h */
bool box_visible(ساختار box *box)
{
	/* visibility: hidden */
	اگر (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		بازگشت false;
	}

	بازگشت true;
}


/* Exported function documented in html/box.h */
تهی box_dump(FILE *stream, ساختار box *box, unsigned صحیح depth, bool style)
{
	unsigned صحیح i;
	ساختار box *c, *prev;

	برای (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	اگر (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[بالاترین_2], box->margin[چپ_2],
		box->margin[BOTTOM], box->margin[راست_2]);

	انتخاب (box->type) {
	حالت BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		بشکن;

	حالت BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		بشکن;

	حالت BOX_INLINE:
		fprintf(stream, "INLINE ");
		بشکن;

	حالت BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		بشکن;

	حالت BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		بشکن;

	حالت BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		بشکن;

	حالت BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		بشکن;

	حالت BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		بشکن;

	حالت BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		بشکن;

	حالت BOX_FLOAT_چپ:
		fprintf(stream, "FLOAT_LEFT ");
		بشکن;

	حالت BOX_FLOAT_راست:
		fprintf(stream, "FLOAT_RIGHT ");
		بشکن;

	حالت BOX_BR:
		fprintf(stream, "BR ");
		بشکن;

	حالت BOX_TEXT:
		fprintf(stream, "TEXT ");
		بشکن;

	حالت BOX_FLEX:
		fprintf(stream, "FLEX ");
		بشکن;

	حالت BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		بشکن;

	پیشفرض:
		fprintf(stream, "Unknown box type ");
	}

	اگر (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(صحیح) box->length, box->text);
	اگر (box->space)
		fprintf(stream, "space ");
	اگر (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	اگر (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	اگر (box->gadget)
		fprintf(stream, "(gadget) ");
	اگر (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	اگر (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	اگر (box->target)
		fprintf(stream, " |%s|", box->target);
	اگر (box->title)
		fprintf(stream, " [%s]", box->title);
	اگر (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	اگر (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	اگر (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	اگر (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	اگر (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	اگر (box->col) {
		fprintf(stream, " (columns");
		برای (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ثابت نویسه *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ثابت نویسه *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	اگر (box->گره != پوچ) {
		dom_string *name;
		اگر (dom_گره_get_گره_name(box->گره, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	اگر (box->list_marker) {
		برای (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	برای (c = box->children; c && c->next; c = c->next)
		;
	اگر (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	برای (prev = 0, c = box->children; c; prev = c, c = c->next) {
		اگر (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		اگر (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ثابت ساختار box * ثابت box)
{
	بازگشت box->padding[بالاترین_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ثابت ساختار box * ثابت box)
{
	بازگشت box->padding[چپ_2] +
		box->width +
		box->padding[راست_2] +
		box->border[راست_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ساختار box *
box_pick_text_box(ساختار html_content *html,
		  صحیح x, صحیح y,
		  صحیح dir,
		  صحیح *dx, صحیح *dy)
{
	ساختار box *text_box = پوچ;
	ساختار box *box;
	صحیح nr_xd, nr_yd;
	صحیح bx, by;
	صحیح fx, fy;
	صحیح tx, ty;

	اگر (html == پوچ)
		بازگشت پوچ;

	box = html->layout;
	bx = box->margin[چپ_2];
	by = box->margin[بالاترین_2];
	fx = bx;
	fy = by;

	اگر (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		اگر (text_box && text_box->text && !text_box->object) {
			صحیح w = (text_box->padding[چپ_2] +
				 text_box->width +
				 text_box->padding[راست_2]);
			صحیح h = (text_box->padding[بالاترین_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			صحیح x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			اگر (x < tx) x = tx;
			اگر (y < ty) y = ty;
			اگر (y > y1) y = y1;
			اگر (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	بازگشت text_box;
}
