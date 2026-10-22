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

#define box_is_float(box) (box->type == BOX_FLOAT_वामः ||	\
			   box->type == BOX_FLOAT_दक्षिणः)

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
box_contains_point(ध्रुव css_unit_ctx *unit_len_ctx,
		   ध्रुव संरचना box *box,
		   पूर्णाङ्क x,
		   पूर्णाङ्क y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	जइ (box->style != रिक्त &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		संरचना rect r = {
				 .x0 = box->border[वामः_2].width,
				 .y0 = box->border[शिखरम्_2].width,
				 .x1 = box->padding[वामः_2] + box->width +
				 box->border[दक्षिणः_2].width +
				 box->padding[दक्षिणः_2],
				 .y1 = box->padding[शिखरम्_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		जइ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} अण्णहा {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		जइ (css_rect.वामः_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.वामः,
						css_rect.lunit));
		}
		जइ (css_rect.शिखरम्_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.शिखरम्,
						css_rect.tunit));
		}
		जइ (css_rect.दक्षिणः_auto == false) {
			r.x1 = box->border[वामः_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.दक्षिणः,
						css_rect.runit));
		}
		जइ (css_rect.bottom_auto == false) {
			r.y1 = box->border[शिखरम्_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		जइ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			प्रत्यागम true;
		}

		/* Not inside clip area */
		प्रत्यागम false;
	}
	जइ (x >= -box->border[वामः_2].width &&
	    x < box->padding[वामः_2] + box->width +
	    box->padding[दक्षिणः_2] + box->border[दक्षिणः_2].width &&
	    y >= -box->border[शिखरम्_2].width &&
	    y < box->padding[शिखरम्_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		प्रत्यागम true;
	}
	जइ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[वामः_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[वामः_2] +
	    box->list_marker->width +
	    box->list_marker->border[दक्षिणः_2].width +
	    box->list_marker->padding[दक्षिणः_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[शिखरम्_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[शिखरम्_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		प्रत्यागम true;
	}
	जइ ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		जइ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			प्रत्यागम true;
		}
	}
	जइ ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		जइ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			प्रत्यागम true;
		}
	}
	प्रत्यागम false;
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
स्थिर अन्तःस्थ संरचना box *
box_move_xy(संरचना box *b, गणना box_walk_dir dir, पूर्णाङ्क *x, पूर्णाङ्क *y)
{
	संरचना box *rb = रिक्त;

	विकल्प (dir) {
	पक्ष BOX_WALK_CHILDREN:
		b = b->children;
		जइ (b == रिक्त)
			भङ्ग;
		*x += b->x;
		*y += b->y;
		जइ (!box_is_float(b)) {
			rb = b;
			भङ्ग;
		}
		fallthrough;

	पक्ष BOX_WALK_NEXT_SIBLING:
		करेहि {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			जइ (b == रिक्त)
				भङ्ग;
			*x += b->x;
			*y += b->y;
		} जाव (box_is_float(b));
		rb = b;
		भङ्ग;

	पक्ष BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		भङ्ग;

	पक्ष BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		जइ (b == रिक्त)
			भङ्ग;
		*x += b->x;
		*y += b->y;
		rb = b;
		भङ्ग;

	पक्ष BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		जइ (b == रिक्त)
			भङ्ग;
		*x += b->x;
		*y += b->y;
		rb = b;
		भङ्ग;

	पक्ष BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		भङ्ग;

	स्वभाव:
		प्रतिज्ञा(0 && "Bad box walk type.");
	}

	प्रत्यागम rb;
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
स्थिर अन्तःस्थ संरचना box *
box_next_xy(संरचना box *b, पूर्णाङ्क *x, पूर्णाङ्क *y, bool skip_children)
{
	संरचना box *n;
	पूर्णाङ्क tx, ty;

	प्रतिज्ञा(b != रिक्त);

	जइ (skip_children) {
		/* Caller is not interested in any kind of children */
		गच्छ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	जइ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		प्रत्यागम n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	जइ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		प्रत्यागम n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	जइ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		प्रत्यागम n;
	}

	जइ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		गच्छ done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	जाव (b) {
		जाव (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			जइ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				गच्छ skip_children;
			}
		}
		जइ (!b->next) {
			/* No more boxes */
			प्रत्यागम रिक्त;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		जइ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			प्रत्यागम n;

		} अण्णहा जइ (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			जइ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				गच्छ skip_children;
			}

		} अण्णहा {
			/* No more boxes */
			प्रत्यागम रिक्त;
		}
	}

	प्रतिज्ञा(b != रिक्त);
	प्रत्यागम रिक्त;
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
		    पूर्णाङ्क bx, पूर्णाङ्क by,
		    पूर्णाङ्क x, पूर्णाङ्क y,
		    पूर्णाङ्क dir,
		    संरचना box **nearest,
		    पूर्णाङ्क *tx, पूर्णाङ्क *ty,
		    पूर्णाङ्क *nr_xd, पूर्णाङ्क *nr_yd)
{
	पूर्णाङ्क w = box->padding[वामः_2] + box->width + box->padding[दक्षिणः_2];
	पूर्णाङ्क h = box->padding[शिखरम्_2] + box->height + box->padding[BOTTOM];
	पूर्णाङ्क y1 = by + h;
	पूर्णाङ्क x1 = bx + w;
	पूर्णाङ्क yd = INT_MAX;
	पूर्णाङ्क xd = INT_MAX;

	जइ (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		प्रत्यागम true;
	}

	जइ (box->parent->list_marker != box) {
		जइ (dir < 0) {
			/* consider only those children (partly) above-left */
			जइ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} अण्णहा {
			/* consider only those children (partly) below-right */
			जइ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		जइ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	प्रत्यागम false;
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
		     पूर्णाङ्क bx, पूर्णाङ्क by,
		     पूर्णाङ्क fx, पूर्णाङ्क fy,
		     पूर्णाङ्क x, पूर्णाङ्क y,
		     पूर्णाङ्क dir,
		     संरचना box **nearest,
		     पूर्णाङ्क *tx, पूर्णाङ्क *ty,
		     पूर्णाङ्क *nr_xd, पूर्णाङ्क *nr_yd)
{
	संरचना box *child = box->children;
	पूर्णाङ्क c_bx, c_by;
	पूर्णाङ्क c_fx, c_fy;
	bool in_box = false;

	जइ (*nearest == रिक्त) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	जइ (box->type == BOX_INLINE_CONTAINER) {
		पूर्णाङ्क bw = box->padding[वामः_2] + box->width + box->padding[दक्षिणः_2];
		पूर्णाङ्क bh = box->padding[शिखरम्_2] + box->height + box->padding[BOTTOM];
		पूर्णाङ्क b_y1 = by + bh;
		पूर्णाङ्क b_x1 = bx + bw;
		जइ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	जाव (child) {
		जइ (child->type == BOX_FLOAT_वामः ||
		    child->type == BOX_FLOAT_दक्षिणः) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} अण्णहा {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		जइ (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} अण्णहा {
			c_fx = fx;
			c_fy = fy;
		}
		जइ (in_box && child->text && !child->object) {
			जइ (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				प्रत्यागम true;
		} अण्णहा {
			जइ (child->list_marker) {
				जइ (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					प्रत्यागम true;
			}
			जइ (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				प्रत्यागम true;
		}
		child = child->next;
	}

	प्रत्यागम false;
}


/* Exported function documented in html/box.h */
शून्य box_coords(संरचना box *box, पूर्णाङ्क *x, पूर्णाङ्क *y)
{
	*x = box->x;
	*y = box->y;
	जाव (box->parent) {
		जइ (box_is_float(box)) {
			प्रतिज्ञा(box->float_container);
			box = box->float_container;
		} अण्णहा {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
शून्य box_bounds(संरचना box *box, संरचना rect *r)
{
	पूर्णाङ्क width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[वामः_2] + box->width + box->padding[दक्षिणः_2];
	height = box->padding[शिखरम्_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
संरचना box *
box_at_point(ध्रुव css_unit_ctx *unit_len_ctx,
	     संरचना box *box,
	     ध्रुव पूर्णाङ्क x, ध्रुव पूर्णाङ्क y,
	     पूर्णाङ्क *box_x, पूर्णाङ्क *box_y)
{
	bool skip_children;
	bool physically;

	प्रतिज्ञा(box);

	skip_children = false;
	जाव ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		जइ (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			जइ (physically)
				प्रत्यागम box;

			skip_children = false;
		} अण्णहा {
			skip_children = true;
		}
	}

	प्रत्यागम रिक्त;
}


/* Exported function documented in html/box.h */
संरचना box *box_find_by_id(संरचना box *box, lwc_string *id)
{
	संरचना box *a, *b;
	bool m;

	जइ (box->id != रिक्त &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		प्रत्यागम box;
	}

	प्रति (a = box->children; a; a = a->next) {
		जइ ((b = box_find_by_id(a, id)) != रिक्त) {
			प्रत्यागम b;
		}
	}

	प्रत्यागम रिक्त;
}


/* Exported function documented in html/box.h */
bool box_visible(संरचना box *box)
{
	/* visibility: hidden */
	जइ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		प्रत्यागम false;
	}

	प्रत्यागम true;
}


/* Exported function documented in html/box.h */
शून्य box_dump(FILE *stream, संरचना box *box, unsigned पूर्णाङ्क depth, bool style)
{
	unsigned पूर्णाङ्क i;
	संरचना box *c, *prev;

	प्रति (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	जइ (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[शिखरम्_2], box->margin[वामः_2],
		box->margin[BOTTOM], box->margin[दक्षिणः_2]);

	विकल्प (box->type) {
	पक्ष BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		भङ्ग;

	पक्ष BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		भङ्ग;

	पक्ष BOX_INLINE:
		fprintf(stream, "INLINE ");
		भङ्ग;

	पक्ष BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		भङ्ग;

	पक्ष BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		भङ्ग;

	पक्ष BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		भङ्ग;

	पक्ष BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		भङ्ग;

	पक्ष BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		भङ्ग;

	पक्ष BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		भङ्ग;

	पक्ष BOX_FLOAT_वामः:
		fprintf(stream, "FLOAT_LEFT ");
		भङ्ग;

	पक्ष BOX_FLOAT_दक्षिणः:
		fprintf(stream, "FLOAT_RIGHT ");
		भङ्ग;

	पक्ष BOX_BR:
		fprintf(stream, "BR ");
		भङ्ग;

	पक्ष BOX_TEXT:
		fprintf(stream, "TEXT ");
		भङ्ग;

	पक्ष BOX_FLEX:
		fprintf(stream, "FLEX ");
		भङ्ग;

	पक्ष BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		भङ्ग;

	स्वभाव:
		fprintf(stream, "Unknown box type ");
	}

	जइ (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(पूर्णाङ्क) box->length, box->text);
	जइ (box->space)
		fprintf(stream, "space ");
	जइ (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	जइ (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	जइ (box->gadget)
		fprintf(stream, "(gadget) ");
	जइ (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	जइ (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	जइ (box->target)
		fprintf(stream, " |%s|", box->target);
	जइ (box->title)
		fprintf(stream, " [%s]", box->title);
	जइ (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	जइ (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	जइ (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	जइ (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	जइ (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	जइ (box->col) {
		fprintf(stream, " (columns");
		प्रति (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((ध्रुव वर्ण *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((ध्रुव वर्ण *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	जइ (box->ग्रन्थिः != रिक्त) {
		dom_string *name;
		जइ (dom_ग्रन्थिः_get_ग्रन्थिः_name(box->ग्रन्थिः, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	जइ (box->list_marker) {
		प्रति (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	प्रति (c = box->children; c && c->next; c = c->next)
		;
	जइ (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	प्रति (prev = 0, c = box->children; c; prev = c, c = c->next) {
		जइ (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		जइ (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ध्रुव संरचना box * ध्रुव box)
{
	प्रत्यागम box->padding[शिखरम्_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ध्रुव संरचना box * ध्रुव box)
{
	प्रत्यागम box->padding[वामः_2] +
		box->width +
		box->padding[दक्षिणः_2] +
		box->border[दक्षिणः_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
संरचना box *
box_pick_text_box(संरचना html_content *html,
		  पूर्णाङ्क x, पूर्णाङ्क y,
		  पूर्णाङ्क dir,
		  पूर्णाङ्क *dx, पूर्णाङ्क *dy)
{
	संरचना box *text_box = रिक्त;
	संरचना box *box;
	पूर्णाङ्क nr_xd, nr_yd;
	पूर्णाङ्क bx, by;
	पूर्णाङ्क fx, fy;
	पूर्णाङ्क tx, ty;

	जइ (html == रिक्त)
		प्रत्यागम रिक्त;

	box = html->layout;
	bx = box->margin[वामः_2];
	by = box->margin[शिखरम्_2];
	fx = bx;
	fy = by;

	जइ (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		जइ (text_box && text_box->text && !text_box->object) {
			पूर्णाङ्क w = (text_box->padding[वामः_2] +
				 text_box->width +
				 text_box->padding[दक्षिणः_2]);
			पूर्णाङ्क h = (text_box->padding[शिखरम्_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			पूर्णाङ्क x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			जइ (x < tx) x = tx;
			जइ (y < ty) y = ty;
			जइ (y > y1) y = y1;
			जइ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	प्रत्यागम text_box;
}
