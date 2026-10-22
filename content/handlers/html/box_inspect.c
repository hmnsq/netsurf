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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/utils.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "utils/errors.h"
#अन्तर्भाव "netsurf/types.h"
#अन्तर्भाव "netsurf/content.h"
#अन्तर्भाव "netsurf/mouse.h"
#अन्तर्भाव "css/utils.h"
#अन्तर्भाव "css/dump.h"
#अन्तर्भाव "desktop/scrollbar.h"

#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
गणना box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_डावा ||	\
			   box->type == BOX_FLOAT_उजवा)

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
स्थिर bool
box_contains_point(स्थिरांक css_unit_ctx *unit_len_ctx,
		   स्थिरांक संरचना box *box,
		   पूर्णांक x,
		   पूर्णांक y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	जर (box->style != रिक्त &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		संरचना rect r = {
				 .x0 = box->border[डावा_2].width,
				 .y0 = box->border[शीर्ष_2].width,
				 .x1 = box->padding[डावा_2] + box->width +
				 box->border[उजवा_2].width +
				 box->padding[उजवा_2],
				 .y1 = box->padding[शीर्ष_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		जर (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} अन्यथा {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		जर (css_rect.डावा_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.डावा,
						css_rect.lunit));
		}
		जर (css_rect.शीर्ष_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.शीर्ष,
						css_rect.tunit));
		}
		जर (css_rect.उजवा_auto == false) {
			r.x1 = box->border[डावा_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.उजवा,
						css_rect.runit));
		}
		जर (css_rect.bottom_auto == false) {
			r.y1 = box->border[शीर्ष_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		जर (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			परत true;
		}

		/* Not inside clip area */
		परत false;
	}
	जर (x >= -box->border[डावा_2].width &&
	    x < box->padding[डावा_2] + box->width +
	    box->padding[उजवा_2] + box->border[उजवा_2].width &&
	    y >= -box->border[शीर्ष_2].width &&
	    y < box->padding[शीर्ष_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		परत true;
	}
	जर (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[डावा_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[डावा_2] +
	    box->list_marker->width +
	    box->list_marker->border[उजवा_2].width +
	    box->list_marker->padding[उजवा_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[शीर्ष_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[शीर्ष_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		परत true;
	}
	जर ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		जर (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			परत true;
		}
	}
	जर ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		जर (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			परत true;
		}
	}
	परत false;
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
स्थिर ओळीत संरचना box *
box_move_xy(संरचना box *b, गणना box_walk_dir dir, पूर्णांक *x, पूर्णांक *y)
{
	संरचना box *rb = रिक्त;

	निवड (dir) {
	स्थिती BOX_WALK_CHILDREN:
		b = b->children;
		जर (b == रिक्त)
			थांबा;
		*x += b->x;
		*y += b->y;
		जर (!box_is_float(b)) {
			rb = b;
			थांबा;
		}
		fallthrough;

	स्थिती BOX_WALK_NEXT_SIBLING:
		करा {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			जर (b == रिक्त)
				थांबा;
			*x += b->x;
			*y += b->y;
		} पर्यंत (box_is_float(b));
		rb = b;
		थांबा;

	स्थिती BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		थांबा;

	स्थिती BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		जर (b == रिक्त)
			थांबा;
		*x += b->x;
		*y += b->y;
		rb = b;
		थांबा;

	स्थिती BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		जर (b == रिक्त)
			थांबा;
		*x += b->x;
		*y += b->y;
		rb = b;
		थांबा;

	स्थिती BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		थांबा;

	पूर्वनिर्धारित:
		प्रतिज्ञा(0 && "Bad box walk type.");
	}

	परत rb;
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
स्थिर ओळीत संरचना box *
box_next_xy(संरचना box *b, पूर्णांक *x, पूर्णांक *y, bool skip_children)
{
	संरचना box *n;
	पूर्णांक tx, ty;

	प्रतिज्ञा(b != रिक्त);

	जर (skip_children) {
		/* Caller is not interested in any kind of children */
		जा skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	जर (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		परत n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	जर (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		परत n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	जर (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		परत n;
	}

	जर (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		जा done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	पर्यंत (b) {
		पर्यंत (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			जर (box_is_float(b)) {
				/* Go on to next float, if there is one */
				जा skip_children;
			}
		}
		जर (!b->next) {
			/* No more boxes */
			परत रिक्त;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		जर (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			परत n;

		} अन्यथा जर (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			जर (box_is_float(b)) {
				/* Go on to next float, if there is one */
				जा skip_children;
			}

		} अन्यथा {
			/* No more boxes */
			परत रिक्त;
		}
	}

	प्रतिज्ञा(b != रिक्त);
	परत रिक्त;
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
स्थिर bool
box_nearer_text_box(संरचना box *box,
		    पूर्णांक bx, पूर्णांक by,
		    पूर्णांक x, पूर्णांक y,
		    पूर्णांक dir,
		    संरचना box **nearest,
		    पूर्णांक *tx, पूर्णांक *ty,
		    पूर्णांक *nr_xd, पूर्णांक *nr_yd)
{
	पूर्णांक w = box->padding[डावा_2] + box->width + box->padding[उजवा_2];
	पूर्णांक h = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];
	पूर्णांक y1 = by + h;
	पूर्णांक x1 = bx + w;
	पूर्णांक yd = पूर्णांक_उच्च;
	पूर्णांक xd = पूर्णांक_उच्च;

	जर (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		परत true;
	}

	जर (box->parent->list_marker != box) {
		जर (dir < 0) {
			/* consider only those children (partly) above-left */
			जर (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} अन्यथा {
			/* consider only those children (partly) below-right */
			जर (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		जर (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	परत false;
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
स्थिर bool
box_nearest_text_box(संरचना box *box,
		     पूर्णांक bx, पूर्णांक by,
		     पूर्णांक fx, पूर्णांक fy,
		     पूर्णांक x, पूर्णांक y,
		     पूर्णांक dir,
		     संरचना box **nearest,
		     पूर्णांक *tx, पूर्णांक *ty,
		     पूर्णांक *nr_xd, पूर्णांक *nr_yd)
{
	संरचना box *child = box->children;
	पूर्णांक c_bx, c_by;
	पूर्णांक c_fx, c_fy;
	bool in_box = false;

	जर (*nearest == रिक्त) {
		*nr_xd = पूर्णांक_उच्च / 2; /* displacement of 'nearest so far' */
		*nr_yd = पूर्णांक_उच्च / 2;
	}
	जर (box->type == BOX_INLINE_CONTAINER) {
		पूर्णांक bw = box->padding[डावा_2] + box->width + box->padding[उजवा_2];
		पूर्णांक bh = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];
		पूर्णांक b_y1 = by + bh;
		पूर्णांक b_x1 = bx + bw;
		जर (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	पर्यंत (child) {
		जर (child->type == BOX_FLOAT_डावा ||
		    child->type == BOX_FLOAT_उजवा) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} अन्यथा {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		जर (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} अन्यथा {
			c_fx = fx;
			c_fy = fy;
		}
		जर (in_box && child->text && !child->object) {
			जर (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				परत true;
		} अन्यथा {
			जर (child->list_marker) {
				जर (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					परत true;
			}
			जर (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				परत true;
		}
		child = child->next;
	}

	परत false;
}


/* Exported function documented in html/box.h */
शून्य box_coords(संरचना box *box, पूर्णांक *x, पूर्णांक *y)
{
	*x = box->x;
	*y = box->y;
	पर्यंत (box->parent) {
		जर (box_is_float(box)) {
			प्रतिज्ञा(box->float_container);
			box = box->float_container;
		} अन्यथा {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
शून्य box_bounds(संरचना box *box, संरचना rect *r)
{
	पूर्णांक width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[डावा_2] + box->width + box->padding[उजवा_2];
	height = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
संरचना box *
box_at_point(स्थिरांक css_unit_ctx *unit_len_ctx,
	     संरचना box *box,
	     स्थिरांक पूर्णांक x, स्थिरांक पूर्णांक y,
	     पूर्णांक *box_x, पूर्णांक *box_y)
{
	bool skip_children;
	bool physically;

	प्रतिज्ञा(box);

	skip_children = false;
	पर्यंत ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		जर (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			जर (physically)
				परत box;

			skip_children = false;
		} अन्यथा {
			skip_children = true;
		}
	}

	परत रिक्त;
}


/* Exported function documented in html/box.h */
संरचना box *box_find_by_id(संरचना box *box, lwc_string *id)
{
	संरचना box *a, *b;
	bool m;

	जर (box->id != रिक्त &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		परत box;
	}

	साठी (a = box->children; a; a = a->next) {
		जर ((b = box_find_by_id(a, id)) != रिक्त) {
			परत b;
		}
	}

	परत रिक्त;
}


/* Exported function documented in html/box.h */
bool box_visible(संरचना box *box)
{
	/* visibility: hidden */
	जर (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		परत false;
	}

	परत true;
}


/* Exported function documented in html/box.h */
शून्य box_dump(खाता *stream, संरचना box *box, अचिन्हित पूर्णांक depth, bool style)
{
	अचिन्हित पूर्णांक i;
	संरचना box *c, *prev;

	साठी (i = 0; i != depth; i++) {
		ख_लिखो(stream, "  ");
	}

	ख_लिखो(stream, "%p ", box);
	ख_लिखो(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	जर (box->max_width != UNKNOWN_MAX_WIDTH) {
		ख_लिखो(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	ख_लिखो(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	ख_लिखो(stream, "m(%i %i %i %i) ",
		box->margin[शीर्ष_2], box->margin[डावा_2],
		box->margin[BOTTOM], box->margin[उजवा_2]);

	निवड (box->type) {
	स्थिती BOX_BLOCK:
		ख_लिखो(stream, "BLOCK ");
		थांबा;

	स्थिती BOX_INLINE_CONTAINER:
		ख_लिखो(stream, "INLINE_CONTAINER ");
		थांबा;

	स्थिती BOX_INLINE:
		ख_लिखो(stream, "INLINE ");
		थांबा;

	स्थिती BOX_INLINE_END:
		ख_लिखो(stream, "INLINE_END ");
		थांबा;

	स्थिती BOX_INLINE_BLOCK:
		ख_लिखो(stream, "INLINE_BLOCK ");
		थांबा;

	स्थिती BOX_TABLE:
		ख_लिखो(stream, "TABLE [columns %i] ", box->columns);
		थांबा;

	स्थिती BOX_TABLE_ROW:
		ख_लिखो(stream, "TABLE_ROW ");
		थांबा;

	स्थिती BOX_TABLE_CELL:
		ख_लिखो(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		थांबा;

	स्थिती BOX_TABLE_ROW_GROUP:
		ख_लिखो(stream, "TABLE_ROW_GROUP ");
		थांबा;

	स्थिती BOX_FLOAT_डावा:
		ख_लिखो(stream, "FLOAT_LEFT ");
		थांबा;

	स्थिती BOX_FLOAT_उजवा:
		ख_लिखो(stream, "FLOAT_RIGHT ");
		थांबा;

	स्थिती BOX_BR:
		ख_लिखो(stream, "BR ");
		थांबा;

	स्थिती BOX_TEXT:
		ख_लिखो(stream, "TEXT ");
		थांबा;

	स्थिती BOX_FLEX:
		ख_लिखो(stream, "FLEX ");
		थांबा;

	स्थिती BOX_INLINE_FLEX:
		ख_लिखो(stream, "INLINE_FLEX ");
		थांबा;

	पूर्वनिर्धारित:
		ख_लिखो(stream, "Unknown box type ");
	}

	जर (box->text)
		ख_लिखो(stream, "%li '%.*s' ", (अचिन्हित दीर्घ) box->byte_offset,
			(पूर्णांक) box->length, box->text);
	जर (box->space)
		ख_लिखो(stream, "space ");
	जर (box->object) {
		ख_लिखो(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	जर (box->iframe) {
		ख_लिखो(stream, "(iframe) ");
	}
	जर (box->gadget)
		ख_लिखो(stream, "(gadget) ");
	जर (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	जर (box->href)
		ख_लिखो(stream, " -> '%s'", nsurl_access(box->href));
	जर (box->target)
		ख_लिखो(stream, " |%s|", box->target);
	जर (box->title)
		ख_लिखो(stream, " [%s]", box->title);
	जर (box->id)
		ख_लिखो(stream, " ID:%s", lwc_string_data(box->id));
	जर (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		ख_लिखो(stream, " inline_end %p", box->inline_end);
	जर (box->float_children)
		ख_लिखो(stream, " float_children %p", box->float_children);
	जर (box->next_float)
		ख_लिखो(stream, " next_float %p", box->next_float);
	जर (box->float_container)
		ख_लिखो(stream, " float_container %p", box->float_container);
	जर (box->col) {
		ख_लिखो(stream, " (columns");
		साठी (i = 0; i != box->columns; i++) {
			ख_लिखो(stream, " (%s %s %i %i %i)",
				((स्थिरांक अक्षर *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((स्थिरांक अक्षर *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		ख_लिखो(stream, ")");
	}
	जर (box->गाठ != रिक्त) {
		dom_string *name;
		जर (dom_गाठ_get_गाठ_name(box->गाठ, &name) == DOM_NO_ERR) {
			ख_लिखो(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	ख_लिखो(stream, "\n");

	जर (box->list_marker) {
		साठी (i = 0; i != depth; i++)
			ख_लिखो(stream, "  ");
		ख_लिखो(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	साठी (c = box->children; c && c->next; c = c->next)
		;
	जर (box->last != c)
		ख_लिखो(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	साठी (prev = 0, c = box->children; c; prev = c, c = c->next) {
		जर (c->parent != box)
			ख_लिखो(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		जर (c->prev != prev)
			ख_लिखो(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(स्थिरांक संरचना box * स्थिरांक box)
{
	परत box->padding[शीर्ष_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(स्थिरांक संरचना box * स्थिरांक box)
{
	परत box->padding[डावा_2] +
		box->width +
		box->padding[उजवा_2] +
		box->border[उजवा_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
संरचना box *
box_pick_text_box(संरचना html_content *html,
		  पूर्णांक x, पूर्णांक y,
		  पूर्णांक dir,
		  पूर्णांक *dx, पूर्णांक *dy)
{
	संरचना box *text_box = रिक्त;
	संरचना box *box;
	पूर्णांक nr_xd, nr_yd;
	पूर्णांक bx, by;
	पूर्णांक fx, fy;
	पूर्णांक tx, ty;

	जर (html == रिक्त)
		परत रिक्त;

	box = html->layout;
	bx = box->margin[डावा_2];
	by = box->margin[शीर्ष_2];
	fx = bx;
	fy = by;

	जर (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		जर (text_box && text_box->text && !text_box->object) {
			पूर्णांक w = (text_box->padding[डावा_2] +
				 text_box->width +
				 text_box->padding[उजवा_2]);
			पूर्णांक h = (text_box->padding[शीर्ष_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			पूर्णांक x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			जर (x < tx) x = tx;
			जर (y < ty) y = ty;
			जर (y > y1) y = y1;
			जर (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	परत text_box;
}
