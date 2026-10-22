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

#ضمن <stdio.h>
#ضمن <dom/dom.h>

#ضمن "utils/utils.h"
#ضمن "utils/nsurl.h"
#ضمن "utils/errors.h"
#ضمن "netsurf/types.h"
#ضمن "netsurf/content.h"
#ضمن "netsurf/mouse.h"
#ضمن "css/utils.h"
#ضمن "css/dump.h"
#ضمن "desktop/scrollbar.h"

#ضمن "html/private.h"
#ضمن "html/box.h"
#ضمن "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
تعداد box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_يسار ||	\
			   box->type == BOX_FLOAT_يمين)

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
ساكن bool
box_contains_point(ثابت css_unit_ctx *unit_len_ctx,
		   ثابت بنية box *box,
		   صحيح x,
		   صحيح y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	إذا (box->style != عدم &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		بنية rect r = {
				 .x0 = box->border[يسار_2].width,
				 .y0 = box->border[قمة_2].width,
				 .x1 = box->padding[يسار_2] + box->width +
				 box->border[يمين_2].width +
				 box->padding[يمين_2],
				 .y1 = box->padding[قمة_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		إذا (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} وإلا {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		إذا (css_rect.يسار_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.يسار,
						css_rect.lunit));
		}
		إذا (css_rect.قمة_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.قمة,
						css_rect.tunit));
		}
		إذا (css_rect.يمين_auto == false) {
			r.x1 = box->border[يسار_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.يمين,
						css_rect.runit));
		}
		إذا (css_rect.bottom_auto == false) {
			r.y1 = box->border[قمة_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		إذا (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			أرجع true;
		}

		/* Not inside clip area */
		أرجع false;
	}
	إذا (x >= -box->border[يسار_2].width &&
	    x < box->padding[يسار_2] + box->width +
	    box->padding[يمين_2] + box->border[يمين_2].width &&
	    y >= -box->border[قمة_2].width &&
	    y < box->padding[قمة_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		أرجع true;
	}
	إذا (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[يسار_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[يسار_2] +
	    box->list_marker->width +
	    box->list_marker->border[يمين_2].width +
	    box->list_marker->padding[يمين_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[قمة_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[قمة_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		أرجع true;
	}
	إذا ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		إذا (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			أرجع true;
		}
	}
	إذا ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		إذا (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			أرجع true;
		}
	}
	أرجع false;
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
ساكن مضمن بنية box *
box_move_xy(بنية box *b, تعداد box_walk_dir dir, صحيح *x, صحيح *y)
{
	بنية box *rb = عدم;

	بدل (dir) {
	حالة BOX_WALK_CHILDREN:
		b = b->children;
		إذا (b == عدم)
			اكسر;
		*x += b->x;
		*y += b->y;
		إذا (!box_is_float(b)) {
			rb = b;
			اكسر;
		}
		fallthrough;

	حالة BOX_WALK_NEXT_SIBLING:
		افعل {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			إذا (b == عدم)
				اكسر;
			*x += b->x;
			*y += b->y;
		} طالما (box_is_float(b));
		rb = b;
		اكسر;

	حالة BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		اكسر;

	حالة BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		إذا (b == عدم)
			اكسر;
		*x += b->x;
		*y += b->y;
		rb = b;
		اكسر;

	حالة BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		إذا (b == عدم)
			اكسر;
		*x += b->x;
		*y += b->y;
		rb = b;
		اكسر;

	حالة BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		اكسر;

	افتراضي:
		أكد(0 && "Bad box walk type.");
	}

	أرجع rb;
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
ساكن مضمن بنية box *
box_next_xy(بنية box *b, صحيح *x, صحيح *y, bool skip_children)
{
	بنية box *n;
	صحيح tx, ty;

	أكد(b != عدم);

	إذا (skip_children) {
		/* Caller is not interested in any kind of children */
		اذهب skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	إذا (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		أرجع n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	إذا (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		أرجع n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	إذا (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		أرجع n;
	}

	إذا (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		اذهب done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	طالما (b) {
		طالما (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			إذا (box_is_float(b)) {
				/* Go on to next float, if there is one */
				اذهب skip_children;
			}
		}
		إذا (!b->next) {
			/* No more boxes */
			أرجع عدم;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		إذا (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			أرجع n;

		} وإلا إذا (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			إذا (box_is_float(b)) {
				/* Go on to next float, if there is one */
				اذهب skip_children;
			}

		} وإلا {
			/* No more boxes */
			أرجع عدم;
		}
	}

	أكد(b != عدم);
	أرجع عدم;
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
ساكن bool
box_nearer_text_box(بنية box *box,
		    صحيح bx, صحيح by,
		    صحيح x, صحيح y,
		    صحيح dir,
		    بنية box **nearest,
		    صحيح *tx, صحيح *ty,
		    صحيح *nr_xd, صحيح *nr_yd)
{
	صحيح w = box->padding[يسار_2] + box->width + box->padding[يمين_2];
	صحيح h = box->padding[قمة_2] + box->height + box->padding[BOTTOM];
	صحيح y1 = by + h;
	صحيح x1 = bx + w;
	صحيح yd = INT_MAX;
	صحيح xd = INT_MAX;

	إذا (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		أرجع true;
	}

	إذا (box->parent->list_marker != box) {
		إذا (dir < 0) {
			/* consider only those children (partly) above-left */
			إذا (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} وإلا {
			/* consider only those children (partly) below-right */
			إذا (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		إذا (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	أرجع false;
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
ساكن bool
box_nearest_text_box(بنية box *box,
		     صحيح bx, صحيح by,
		     صحيح fx, صحيح fy,
		     صحيح x, صحيح y,
		     صحيح dir,
		     بنية box **nearest,
		     صحيح *tx, صحيح *ty,
		     صحيح *nr_xd, صحيح *nr_yd)
{
	بنية box *child = box->children;
	صحيح c_bx, c_by;
	صحيح c_fx, c_fy;
	bool in_box = false;

	إذا (*nearest == عدم) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	إذا (box->type == BOX_INLINE_CONTAINER) {
		صحيح bw = box->padding[يسار_2] + box->width + box->padding[يمين_2];
		صحيح bh = box->padding[قمة_2] + box->height + box->padding[BOTTOM];
		صحيح b_y1 = by + bh;
		صحيح b_x1 = bx + bw;
		إذا (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	طالما (child) {
		إذا (child->type == BOX_FLOAT_يسار ||
		    child->type == BOX_FLOAT_يمين) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} وإلا {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		إذا (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} وإلا {
			c_fx = fx;
			c_fy = fy;
		}
		إذا (in_box && child->text && !child->object) {
			إذا (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				أرجع true;
		} وإلا {
			إذا (child->list_marker) {
				إذا (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					أرجع true;
			}
			إذا (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				أرجع true;
		}
		child = child->next;
	}

	أرجع false;
}


/* Exported function documented in html/box.h */
فراغ box_coords(بنية box *box, صحيح *x, صحيح *y)
{
	*x = box->x;
	*y = box->y;
	طالما (box->parent) {
		إذا (box_is_float(box)) {
			أكد(box->float_container);
			box = box->float_container;
		} وإلا {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
فراغ box_bounds(بنية box *box, بنية rect *r)
{
	صحيح width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[يسار_2] + box->width + box->padding[يمين_2];
	height = box->padding[قمة_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
بنية box *
box_at_point(ثابت css_unit_ctx *unit_len_ctx,
	     بنية box *box,
	     ثابت صحيح x, ثابت صحيح y,
	     صحيح *box_x, صحيح *box_y)
{
	bool skip_children;
	bool physically;

	أكد(box);

	skip_children = false;
	طالما ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		إذا (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			إذا (physically)
				أرجع box;

			skip_children = false;
		} وإلا {
			skip_children = true;
		}
	}

	أرجع عدم;
}


/* Exported function documented in html/box.h */
بنية box *box_find_by_id(بنية box *box, lwc_string *id)
{
	بنية box *a, *b;
	bool m;

	إذا (box->id != عدم &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		أرجع box;
	}

	لكل (a = box->children; a; a = a->next) {
		إذا ((b = box_find_by_id(a, id)) != عدم) {
			أرجع b;
		}
	}

	أرجع عدم;
}


/* Exported function documented in html/box.h */
bool box_visible(بنية box *box)
{
	/* visibility: hidden */
	إذا (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		أرجع false;
	}

	أرجع true;
}


/* Exported function documented in html/box.h */
فراغ box_dump(FILE *stream, بنية box *box, unsigned صحيح depth, bool style)
{
	unsigned صحيح i;
	بنية box *c, *prev;

	لكل (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	إذا (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[قمة_2], box->margin[يسار_2],
		box->margin[BOTTOM], box->margin[يمين_2]);

	بدل (box->type) {
	حالة BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		اكسر;

	حالة BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		اكسر;

	حالة BOX_INLINE:
		fprintf(stream, "INLINE ");
		اكسر;

	حالة BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		اكسر;

	حالة BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		اكسر;

	حالة BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		اكسر;

	حالة BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		اكسر;

	حالة BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		اكسر;

	حالة BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		اكسر;

	حالة BOX_FLOAT_يسار:
		fprintf(stream, "FLOAT_LEFT ");
		اكسر;

	حالة BOX_FLOAT_يمين:
		fprintf(stream, "FLOAT_RIGHT ");
		اكسر;

	حالة BOX_BR:
		fprintf(stream, "BR ");
		اكسر;

	حالة BOX_TEXT:
		fprintf(stream, "TEXT ");
		اكسر;

	حالة BOX_FLEX:
		fprintf(stream, "FLEX ");
		اكسر;

	حالة BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		اكسر;

	افتراضي:
		fprintf(stream, "Unknown box type ");
	}

	إذا (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(صحيح) box->length, box->text);
	إذا (box->space)
		fprintf(stream, "space ");
	إذا (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	إذا (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	إذا (box->gadget)
		fprintf(stream, "(gadget) ");
	إذا (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	إذا (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	إذا (box->target)
		fprintf(stream, " |%s|", box->target);
	إذا (box->title)
		fprintf(stream, " [%s]", box->title);
	إذا (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	إذا (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	إذا (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	إذا (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	إذا (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	إذا (box->col) {
		fprintf(stream, " (columns");
		لكل (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ثابت حرف *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ثابت حرف *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	إذا (box->عقدة != عدم) {
		dom_string *name;
		إذا (dom_عقدة_get_عقدة_name(box->عقدة, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	إذا (box->list_marker) {
		لكل (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	لكل (c = box->children; c && c->next; c = c->next)
		;
	إذا (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	لكل (prev = 0, c = box->children; c; prev = c, c = c->next) {
		إذا (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		إذا (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[قمة_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[يسار_2] +
		box->width +
		box->padding[يمين_2] +
		box->border[يمين_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
بنية box *
box_pick_text_box(بنية html_content *html,
		  صحيح x, صحيح y,
		  صحيح dir,
		  صحيح *dx, صحيح *dy)
{
	بنية box *text_box = عدم;
	بنية box *box;
	صحيح nr_xd, nr_yd;
	صحيح bx, by;
	صحيح fx, fy;
	صحيح tx, ty;

	إذا (html == عدم)
		أرجع عدم;

	box = html->layout;
	bx = box->margin[يسار_2];
	by = box->margin[قمة_2];
	fx = bx;
	fy = by;

	إذا (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		إذا (text_box && text_box->text && !text_box->object) {
			صحيح w = (text_box->padding[يسار_2] +
				 text_box->width +
				 text_box->padding[يمين_2]);
			صحيح h = (text_box->padding[قمة_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			صحيح x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			إذا (x < tx) x = tx;
			إذا (y < ty) y = ty;
			إذا (y > y1) y = y1;
			إذا (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	أرجع text_box;
}
