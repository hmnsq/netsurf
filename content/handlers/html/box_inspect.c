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

#समावेश <stdio.h>
#समावेश <dom/dom.h>

#समावेश "utils/utils.h"
#समावेश "utils/nsurl.h"
#समावेश "utils/errors.h"
#समावेश "netsurf/types.h"
#समावेश "netsurf/content.h"
#समावेश "netsurf/mouse.h"
#समावेश "css/utils.h"
#समावेश "css/dump.h"
#समावेश "desktop/scrollbar.h"

#समावेश "html/private.h"
#समावेश "html/box.h"
#समावेश "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
क्रमागत box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_अग्र_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_अग्र_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#घोषणा box_is_float(box) (box->type == BOX_FLOAT_बायाँ ||	\
			   box->type == BOX_FLOAT_दायाँ)

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
अटल bool
box_contains_point(स्थिर css_unit_ctx *unit_len_ctx,
		   स्थिर काष्ठा box *box,
		   पूर्णांक x,
		   पूर्णांक y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	अगर (box->style != शून्य &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		काष्ठा rect r = {
				 .x0 = box->border[बायाँ_2].width,
				 .y0 = box->border[शीर्ष_2].width,
				 .x1 = box->padding[बायाँ_2] + box->width +
				 box->border[दायाँ_2].width +
				 box->padding[दायाँ_2],
				 .y1 = box->padding[शीर्ष_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		अगर (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} अन्यथा {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		अगर (css_rect.बायाँ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.बायाँ,
						css_rect.lunit));
		}
		अगर (css_rect.शीर्ष_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.शीर्ष,
						css_rect.tunit));
		}
		अगर (css_rect.दायाँ_auto == false) {
			r.x1 = box->border[बायाँ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.दायाँ,
						css_rect.runit));
		}
		अगर (css_rect.bottom_auto == false) {
			r.y1 = box->border[शीर्ष_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		अगर (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			वापस true;
		}

		/* Not inside clip area */
		वापस false;
	}
	अगर (x >= -box->border[बायाँ_2].width &&
	    x < box->padding[बायाँ_2] + box->width +
	    box->padding[दायाँ_2] + box->border[दायाँ_2].width &&
	    y >= -box->border[शीर्ष_2].width &&
	    y < box->padding[शीर्ष_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		वापस true;
	}
	अगर (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[बायाँ_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[बायाँ_2] +
	    box->list_marker->width +
	    box->list_marker->border[दायाँ_2].width +
	    box->list_marker->padding[दायाँ_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[शीर्ष_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[शीर्ष_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		वापस true;
	}
	अगर ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		अगर (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			वापस true;
		}
	}
	अगर ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		अगर (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			वापस true;
		}
	}
	वापस false;
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
अटल अंतरभूत काष्ठा box *
box_move_xy(काष्ठा box *b, क्रमागत box_walk_dir dir, पूर्णांक *x, पूर्णांक *y)
{
	काष्ठा box *rb = शून्य;

	चयन (dir) {
	हाल BOX_WALK_CHILDREN:
		b = b->children;
		अगर (b == शून्य)
			अवरोध;
		*x += b->x;
		*y += b->y;
		अगर (!box_is_float(b)) {
			rb = b;
			अवरोध;
		}
		fallthrough;

	हाल BOX_WALK_अग्र_SIBLING:
		करो {
			*x -= b->x;
			*y -= b->y;
			b = b->अग्र;
			अगर (b == शून्य)
				अवरोध;
			*x += b->x;
			*y += b->y;
		} जबतक (box_is_float(b));
		rb = b;
		अवरोध;

	हाल BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		अवरोध;

	हाल BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		अगर (b == शून्य)
			अवरोध;
		*x += b->x;
		*y += b->y;
		rb = b;
		अवरोध;

	हाल BOX_WALK_अग्र_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->अग्र_float;
		अगर (b == शून्य)
			अवरोध;
		*x += b->x;
		*y += b->y;
		rb = b;
		अवरोध;

	हाल BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		अवरोध;

	शेष:
		निश्चित(0 && "Bad box walk type.");
	}

	वापस rb;
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
अटल अंतरभूत काष्ठा box *
box_अग्र_xy(काष्ठा box *b, पूर्णांक *x, पूर्णांक *y, bool skip_children)
{
	काष्ठा box *n;
	पूर्णांक tx, ty;

	निश्चित(b != शून्य);

	अगर (skip_children) {
		/* Caller is not interested in any kind of children */
		जाओ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	अगर (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		वापस n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	अगर (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		वापस n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_अग्र_FLOAT_SIBLING, &tx, &ty);
	अगर (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		वापस n;
	}

	अगर (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		जाओ done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	जबतक (b) {
		जबतक (!b->अग्र && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			अगर (box_is_float(b)) {
				/* Go on to next float, if there is one */
				जाओ skip_children;
			}
		}
		अगर (!b->अग्र) {
			/* No more boxes */
			वापस शून्य;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_अग्र_SIBLING, &tx, &ty);
		अगर (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			वापस n;

		} अन्यथा अगर (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			अगर (box_is_float(b)) {
				/* Go on to next float, if there is one */
				जाओ skip_children;
			}

		} अन्यथा {
			/* No more boxes */
			वापस शून्य;
		}
	}

	निश्चित(b != शून्य);
	वापस शून्य;
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
अटल bool
box_nearer_text_box(काष्ठा box *box,
		    पूर्णांक bx, पूर्णांक by,
		    पूर्णांक x, पूर्णांक y,
		    पूर्णांक dir,
		    काष्ठा box **nearest,
		    पूर्णांक *tx, पूर्णांक *ty,
		    पूर्णांक *nr_xd, पूर्णांक *nr_yd)
{
	पूर्णांक w = box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2];
	पूर्णांक h = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];
	पूर्णांक y1 = by + h;
	पूर्णांक x1 = bx + w;
	पूर्णांक yd = पूर्णांक_उच्च;
	पूर्णांक xd = पूर्णांक_उच्च;

	अगर (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		वापस true;
	}

	अगर (box->parent->list_marker != box) {
		अगर (dir < 0) {
			/* consider only those children (partly) above-left */
			अगर (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} अन्यथा {
			/* consider only those children (partly) below-right */
			अगर (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		अगर (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	वापस false;
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
अटल bool
box_nearest_text_box(काष्ठा box *box,
		     पूर्णांक bx, पूर्णांक by,
		     पूर्णांक fx, पूर्णांक fy,
		     पूर्णांक x, पूर्णांक y,
		     पूर्णांक dir,
		     काष्ठा box **nearest,
		     पूर्णांक *tx, पूर्णांक *ty,
		     पूर्णांक *nr_xd, पूर्णांक *nr_yd)
{
	काष्ठा box *child = box->children;
	पूर्णांक c_bx, c_by;
	पूर्णांक c_fx, c_fy;
	bool in_box = false;

	अगर (*nearest == शून्य) {
		*nr_xd = पूर्णांक_उच्च / 2; /* displacement of 'nearest so far' */
		*nr_yd = पूर्णांक_उच्च / 2;
	}
	अगर (box->type == BOX_INLINE_CONTAINER) {
		पूर्णांक bw = box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2];
		पूर्णांक bh = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];
		पूर्णांक b_y1 = by + bh;
		पूर्णांक b_x1 = bx + bw;
		अगर (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	जबतक (child) {
		अगर (child->type == BOX_FLOAT_बायाँ ||
		    child->type == BOX_FLOAT_दायाँ) {
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
		अगर (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} अन्यथा {
			c_fx = fx;
			c_fy = fy;
		}
		अगर (in_box && child->text && !child->object) {
			अगर (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				वापस true;
		} अन्यथा {
			अगर (child->list_marker) {
				अगर (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					वापस true;
			}
			अगर (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				वापस true;
		}
		child = child->अग्र;
	}

	वापस false;
}


/* Exported function documented in html/box.h */
व्योम box_coords(काष्ठा box *box, पूर्णांक *x, पूर्णांक *y)
{
	*x = box->x;
	*y = box->y;
	जबतक (box->parent) {
		अगर (box_is_float(box)) {
			निश्चित(box->float_container);
			box = box->float_container;
		} अन्यथा {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
व्योम box_bounds(काष्ठा box *box, काष्ठा rect *r)
{
	पूर्णांक width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[बायाँ_2] + box->width + box->padding[दायाँ_2];
	height = box->padding[शीर्ष_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
काष्ठा box *
box_at_point(स्थिर css_unit_ctx *unit_len_ctx,
	     काष्ठा box *box,
	     स्थिर पूर्णांक x, स्थिर पूर्णांक y,
	     पूर्णांक *box_x, पूर्णांक *box_y)
{
	bool skip_children;
	bool physically;

	निश्चित(box);

	skip_children = false;
	जबतक ((box = box_अग्र_xy(box, box_x, box_y, skip_children))) {
		अगर (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			अगर (physically)
				वापस box;

			skip_children = false;
		} अन्यथा {
			skip_children = true;
		}
	}

	वापस शून्य;
}


/* Exported function documented in html/box.h */
काष्ठा box *box_find_by_id(काष्ठा box *box, lwc_string *id)
{
	काष्ठा box *a, *b;
	bool m;

	अगर (box->id != शून्य &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		वापस box;
	}

	क्रम (a = box->children; a; a = a->अग्र) {
		अगर ((b = box_find_by_id(a, id)) != शून्य) {
			वापस b;
		}
	}

	वापस शून्य;
}


/* Exported function documented in html/box.h */
bool box_visible(काष्ठा box *box)
{
	/* visibility: hidden */
	अगर (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		वापस false;
	}

	वापस true;
}


/* Exported function documented in html/box.h */
व्योम box_dump(खाता *stream, काष्ठा box *box, अचिन्हित पूर्णांक depth, bool style)
{
	अचिन्हित पूर्णांक i;
	काष्ठा box *c, *पूर्व;

	क्रम (i = 0; i != depth; i++) {
		ख_लिखो(stream, "  ");
	}

	ख_लिखो(stream, "%p ", box);
	ख_लिखो(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	अगर (box->max_width != UNKNOWN_MAX_WIDTH) {
		ख_लिखो(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	ख_लिखो(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	ख_लिखो(stream, "m(%i %i %i %i) ",
		box->margin[शीर्ष_2], box->margin[बायाँ_2],
		box->margin[BOTTOM], box->margin[दायाँ_2]);

	चयन (box->type) {
	हाल BOX_BLOCK:
		ख_लिखो(stream, "BLOCK ");
		अवरोध;

	हाल BOX_INLINE_CONTAINER:
		ख_लिखो(stream, "INLINE_CONTAINER ");
		अवरोध;

	हाल BOX_INLINE:
		ख_लिखो(stream, "INLINE ");
		अवरोध;

	हाल BOX_INLINE_END:
		ख_लिखो(stream, "INLINE_END ");
		अवरोध;

	हाल BOX_INLINE_BLOCK:
		ख_लिखो(stream, "INLINE_BLOCK ");
		अवरोध;

	हाल BOX_TABLE:
		ख_लिखो(stream, "TABLE [columns %i] ", box->columns);
		अवरोध;

	हाल BOX_TABLE_ROW:
		ख_लिखो(stream, "TABLE_ROW ");
		अवरोध;

	हाल BOX_TABLE_CELL:
		ख_लिखो(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		अवरोध;

	हाल BOX_TABLE_ROW_GROUP:
		ख_लिखो(stream, "TABLE_ROW_GROUP ");
		अवरोध;

	हाल BOX_FLOAT_बायाँ:
		ख_लिखो(stream, "FLOAT_LEFT ");
		अवरोध;

	हाल BOX_FLOAT_दायाँ:
		ख_लिखो(stream, "FLOAT_RIGHT ");
		अवरोध;

	हाल BOX_BR:
		ख_लिखो(stream, "BR ");
		अवरोध;

	हाल BOX_TEXT:
		ख_लिखो(stream, "TEXT ");
		अवरोध;

	हाल BOX_FLEX:
		ख_लिखो(stream, "FLEX ");
		अवरोध;

	हाल BOX_INLINE_FLEX:
		ख_लिखो(stream, "INLINE_FLEX ");
		अवरोध;

	शेष:
		ख_लिखो(stream, "Unknown box type ");
	}

	अगर (box->text)
		ख_लिखो(stream, "%li '%.*s' ", (अचिन्हित दीर्घ) box->byte_offset,
			(पूर्णांक) box->लंबाई, box->text);
	अगर (box->space)
		ख_लिखो(stream, "space ");
	अगर (box->object) {
		ख_लिखो(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	अगर (box->iframe) {
		ख_लिखो(stream, "(iframe) ");
	}
	अगर (box->gadget)
		ख_लिखो(stream, "(gadget) ");
	अगर (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	अगर (box->href)
		ख_लिखो(stream, " -> '%s'", nsurl_access(box->href));
	अगर (box->target)
		ख_लिखो(stream, " |%s|", box->target);
	अगर (box->title)
		ख_लिखो(stream, " [%s]", box->title);
	अगर (box->id)
		ख_लिखो(stream, " ID:%s", lwc_string_तथ्य(box->id));
	अगर (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		ख_लिखो(stream, " inline_end %p", box->inline_end);
	अगर (box->float_children)
		ख_लिखो(stream, " float_children %p", box->float_children);
	अगर (box->अग्र_float)
		ख_लिखो(stream, " next_float %p", box->अग्र_float);
	अगर (box->float_container)
		ख_लिखो(stream, " float_container %p", box->float_container);
	अगर (box->col) {
		ख_लिखो(stream, " (columns");
		क्रम (i = 0; i != box->columns; i++) {
			ख_लिखो(stream, " (%s %s %i %i %i)",
				((स्थिर अक्षर *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((स्थिर अक्षर *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		ख_लिखो(stream, ")");
	}
	अगर (box->गाँठ != शून्य) {
		dom_string *name;
		अगर (dom_गाँठ_get_गाँठ_name(box->गाँठ, &name) == DOM_NO_ERR) {
			ख_लिखो(stream, " <%s>", dom_string_तथ्य(name));
			dom_string_unref(name);
		}
	}
	ख_लिखो(stream, "\n");

	अगर (box->list_marker) {
		क्रम (i = 0; i != depth; i++)
			ख_लिखो(stream, "  ");
		ख_लिखो(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	क्रम (c = box->children; c && c->अग्र; c = c->अग्र)
		;
	अगर (box->last != c)
		ख_लिखो(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	क्रम (पूर्व = 0, c = box->children; c; पूर्व = c, c = c->अग्र) {
		अगर (c->parent != box)
			ख_लिखो(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		अगर (c->पूर्व != पूर्व)
			ख_लिखो(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->पूर्व, पूर्व);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(स्थिर काष्ठा box * स्थिर box)
{
	वापस box->padding[शीर्ष_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(स्थिर काष्ठा box * स्थिर box)
{
	वापस box->padding[बायाँ_2] +
		box->width +
		box->padding[दायाँ_2] +
		box->border[दायाँ_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
काष्ठा box *
box_pick_text_box(काष्ठा html_content *html,
		  पूर्णांक x, पूर्णांक y,
		  पूर्णांक dir,
		  पूर्णांक *dx, पूर्णांक *dy)
{
	काष्ठा box *text_box = शून्य;
	काष्ठा box *box;
	पूर्णांक nr_xd, nr_yd;
	पूर्णांक bx, by;
	पूर्णांक fx, fy;
	पूर्णांक tx, ty;

	अगर (html == शून्य)
		वापस शून्य;

	box = html->layout;
	bx = box->margin[बायाँ_2];
	by = box->margin[शीर्ष_2];
	fx = bx;
	fy = by;

	अगर (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		अगर (text_box && text_box->text && !text_box->object) {
			पूर्णांक w = (text_box->padding[बायाँ_2] +
				 text_box->width +
				 text_box->padding[दायाँ_2]);
			पूर्णांक h = (text_box->padding[शीर्ष_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			पूर्णांक x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			अगर (x < tx) x = tx;
			अगर (y < ty) y = ty;
			अगर (y > y1) y = y1;
			अगर (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	वापस text_box;
}
