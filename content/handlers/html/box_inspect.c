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
		   BOX_WALK_پلار,
		   BOX_WALK_بل_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_بل_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#تعریف_کړه box_is_float(box) (box->type == BOX_FLOAT_کیڼ ||	\
			   box->type == BOX_FLOAT_ښی)

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
box_لري_ټکی(ثابت css_unit_ctx *unit_len_ctx,
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
				 .x0 = box->border[کیڼ_2].پلنوالی,
				 .y0 = box->border[سر_2].پلنوالی,
				 .x1 = box->padding[کیڼ_2] + box->پلنوالی +
				 box->border[ښی_2].پلنوالی +
				 box->padding[ښی_2],
				 .y1 = box->padding[سر_2] + box->لوړوالی +
				 box->border[BOTTOM].پلنوالی +
				 box->padding[BOTTOM]
		};
		إذا (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} وإلا {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		إذا (css_rect.کیڼ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.کیڼ,
						css_rect.lunit));
		}
		إذا (css_rect.سر_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.سر,
						css_rect.tunit));
		}
		إذا (css_rect.ښی_auto == false) {
			r.x1 = box->border[کیڼ_2].پلنوالی +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ښی,
						css_rect.runit));
		}
		إذا (css_rect.bottom_auto == false) {
			r.y1 = box->border[سر_2].پلنوالی +
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
	إذا (x >= -box->border[کیڼ_2].پلنوالی &&
	    x < box->padding[کیڼ_2] + box->پلنوالی +
	    box->padding[ښی_2] + box->border[ښی_2].پلنوالی &&
	    y >= -box->border[سر_2].پلنوالی &&
	    y < box->padding[سر_2] + box->لوړوالی +
	    box->padding[BOTTOM] + box->border[BOTTOM].پلنوالی) {
		*physically = true;
		أرجع true;
	}
	إذا (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[کیڼ_2].پلنوالی &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[کیڼ_2] +
	    box->list_marker->پلنوالی +
	    box->list_marker->border[ښی_2].پلنوالی +
	    box->list_marker->padding[ښی_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[سر_2].پلنوالی &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[سر_2] +
	    box->list_marker->لوړوالی +
	    box->list_marker->border[BOTTOM].پلنوالی +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		أرجع true;
	}
	إذا ((box->style && css_computed_ډکېدنه_x(box->style) ==
	     CSS_ډکېدنه_VISIBLE) || !box->style) {
		إذا (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			أرجع true;
		}
	}
	إذا ((box->style && css_computed_ډکېدنه_y(box->style) ==
	     CSS_ډکېدنه_VISIBLE) || !box->style) {
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

	حالة BOX_WALK_بل_SIBLING:
		افعل {
			*x -= b->x;
			*y -= b->y;
			b = b->بل;
			إذا (b == عدم)
				اكسر;
			*x += b->x;
			*y += b->y;
		} طالما (box_is_float(b));
		rb = b;
		اكسر;

	حالة BOX_WALK_پلار:
		*x -= b->x;
		*y -= b->y;
		rb = b->پلار;
		اكسر;

	حالة BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		إذا (b == عدم)
			اكسر;
		*x += b->x;
		*y += b->y;
		rb = b;
		اكسر;

	حالة BOX_WALK_بل_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->بل_float;
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
box_بل_xy(بنية box *b, صحيح *x, صحيح *y, bool skip_children)
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
 بشپړ_float_children:

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
	n = box_move_xy(b, BOX_WALK_بل_FLOAT_SIBLING, &tx, &ty);
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
		اذهب بشپړ_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	طالما (b) {
		طالما (!b->بل && b->پلار) {
			b = box_move_xy(b, BOX_WALK_پلار, x, y);
			إذا (box_is_float(b)) {
				/* Go on to next float, if there is one */
				اذهب skip_children;
			}
		}
		إذا (!b->بل) {
			/* No more boxes */
			أرجع عدم;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_بل_SIBLING, &tx, &ty);
		إذا (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			أرجع n;

		} وإلا إذا (b->پلار) {
			b = box_move_xy(b, BOX_WALK_پلار, x, y);
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
box_nearer_متن_box(بنية box *box,
		    صحيح bx, صحيح by,
		    صحيح x, صحيح y,
		    صحيح dir,
		    بنية box **تر_ټولو_نږدې,
		    صحيح *tx, صحيح *ty,
		    صحيح *nr_xd, صحيح *nr_yd)
{
	صحيح w = box->padding[کیڼ_2] + box->پلنوالی + box->padding[ښی_2];
	صحيح h = box->padding[سر_2] + box->لوړوالی + box->padding[BOTTOM];
	صحيح y1 = by + h;
	صحيح x1 = bx + w;
	صحيح yd = لوی_عدد;
	صحيح xd = لوی_عدد;

	إذا (x >= bx && x1 > x && y >= by && y1 > y) {
		*تر_ټولو_نږدې = box;
		*tx = bx;
		*ty = by;
		أرجع true;
	}

	إذا (box->پلار->list_marker != box) {
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
			*تر_ټولو_نږدې = box;
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
box_تر_ټولو_نږدې_متن_box(بنية box *box,
		     صحيح bx, صحيح by,
		     صحيح fx, صحيح fy,
		     صحيح x, صحيح y,
		     صحيح dir,
		     بنية box **تر_ټولو_نږدې,
		     صحيح *tx, صحيح *ty,
		     صحيح *nr_xd, صحيح *nr_yd)
{
	بنية box *ماشوم = box->children;
	صحيح c_bx, c_by;
	صحيح c_fx, c_fy;
	bool دننه_box = false;

	إذا (*تر_ټولو_نږدې == عدم) {
		*nr_xd = لوی_عدد / 2; /* displacement of 'nearest so far' */
		*nr_yd = لوی_عدد / 2;
	}
	إذا (box->type == BOX_INLINE_CONTAINER) {
		صحيح bw = box->padding[کیڼ_2] + box->پلنوالی + box->padding[ښی_2];
		صحيح bh = box->padding[سر_2] + box->لوړوالی + box->padding[BOTTOM];
		صحيح b_y1 = by + bh;
		صحيح b_x1 = bx + bw;
		إذا (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			دننه_box = true;
		}
	}

	طالما (ماشوم) {
		إذا (ماشوم->type == BOX_FLOAT_کیڼ ||
		    ماشوم->type == BOX_FLOAT_ښی) {
			c_bx = fx + ماشوم->x -
				scrollbar_get_دننه_واټن(ماشوم->scroll_x);
			c_by = fy + ماشوم->y -
				scrollbar_get_دننه_واټن(ماشوم->scroll_y);
		} وإلا {
			c_bx = bx + ماشوم->x -
				scrollbar_get_دننه_واټن(ماشوم->scroll_x);
			c_by = by + ماشوم->y -
				scrollbar_get_دننه_واټن(ماشوم->scroll_y);
		}
		إذا (ماشوم->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} وإلا {
			c_fx = fx;
			c_fy = fy;
		}
		إذا (دننه_box && ماشوم->متن && !ماشوم->object) {
			إذا (box_nearer_متن_box(ماشوم,
						c_bx, c_by, x, y, dir, تر_ټولو_نږدې,
						tx, ty, nr_xd, nr_yd))
				أرجع true;
		} وإلا {
			إذا (ماشوم->list_marker) {
				إذا (box_nearer_متن_box(
						ماشوم->list_marker,
						c_bx + ماشوم->list_marker->x,
						c_by + ماشوم->list_marker->y,
						x, y, dir, تر_ټولو_نږدې,
						tx, ty, nr_xd, nr_yd))
					أرجع true;
			}
			إذا (box_تر_ټولو_نږدې_متن_box(ماشوم, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, تر_ټولو_نږدې, tx, ty,
						 nr_xd, nr_yd))
				أرجع true;
		}
		ماشوم = ماشوم->بل;
	}

	أرجع false;
}


/* Exported function documented in html/box.h */
فراغ box_coords(بنية box *box, صحيح *x, صحيح *y)
{
	*x = box->x;
	*y = box->y;
	طالما (box->پلار) {
		إذا (box_is_float(box)) {
			أكد(box->float_container);
			box = box->float_container;
		} وإلا {
			box = box->پلار;
		}
		*x += box->x - scrollbar_get_دننه_واټن(box->scroll_x);
		*y += box->y - scrollbar_get_دننه_واټن(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
فراغ box_bounds(بنية box *box, بنية rect *r)
{
	صحيح پلنوالی, لوړوالی;

	box_coords(box, &r->x0, &r->y0);

	پلنوالی = box->padding[کیڼ_2] + box->پلنوالی + box->padding[ښی_2];
	لوړوالی = box->padding[سر_2] + box->لوړوالی + box->padding[BOTTOM];

	r->x1 = r->x0 + پلنوالی;
	r->y1 = r->y0 + لوړوالی;
}


/* Exported function documented in html/box.h */
بنية box *
box_at_ټکی(ثابت css_unit_ctx *unit_len_ctx,
	     بنية box *box,
	     ثابت صحيح x, ثابت صحيح y,
	     صحيح *box_x, صحيح *box_y)
{
	bool skip_children;
	bool physically;

	أكد(box);

	skip_children = false;
	طالما ((box = box_بل_xy(box, box_x, box_y, skip_children))) {
		إذا (box_لري_ټکی(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_دننه_واټن(box->scroll_x);
			*box_y -= scrollbar_get_دننه_واټن(box->scroll_y);

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
بنية box *box_پیدا_کړه_by_id(بنية box *box, lwc_string *id)
{
	بنية box *a, *b;
	bool m;

	إذا (box->id != عدم &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		أرجع box;
	}

	لكل (a = box->children; a; a = a->بل) {
		إذا ((b = box_پیدا_کړه_by_id(a, id)) != عدم) {
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
فراغ box_dump(FILE *بهیر, بنية box *box, بینښه صحيح ژوروالی, bool style)
{
	بینښه صحيح i;
	بنية box *c, *prev;

	لكل (i = 0; i != ژوروالی; i++) {
		fprintf(بهیر, "  ");
	}

	fprintf(بهیر, "%p ", box);
	fprintf(بهیر, "x%i y%i w%i h%i ",
		box->x, box->y, box->پلنوالی, box->لوړوالی);
	إذا (box->max_پلنوالی != UNKNOWN_MAX_پلنوالی) {
		fprintf(بهیر, "min%i max%i ", box->min_پلنوالی, box->max_پلنوالی);
	}
	fprintf(بهیر, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(بهیر, "m(%i %i %i %i) ",
		box->margin[سر_2], box->margin[کیڼ_2],
		box->margin[BOTTOM], box->margin[ښی_2]);

	بدل (box->type) {
	حالة BOX_بلاک:
		fprintf(بهیر, "BLOCK ");
		اكسر;

	حالة BOX_INLINE_CONTAINER:
		fprintf(بهیر, "INLINE_CONTAINER ");
		اكسر;

	حالة BOX_INLINE:
		fprintf(بهیر, "INLINE ");
		اكسر;

	حالة BOX_INLINE_پای_ټکی:
		fprintf(بهیر, "INLINE_END ");
		اكسر;

	حالة BOX_INLINE_بلاک:
		fprintf(بهیر, "INLINE_BLOCK ");
		اكسر;

	حالة BOX_جدول:
		fprintf(بهیر, "TABLE [columns %i] ", box->columns);
		اكسر;

	حالة BOX_جدول_لیکه:
		fprintf(بهیر, "TABLE_ROW ");
		اكسر;

	حالة BOX_جدول_CELL:
		fprintf(بهیر, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->پیل_ستنه,
			box->لیکې);
		اكسر;

	حالة BOX_جدول_لیکه_ډله:
		fprintf(بهیر, "TABLE_ROW_GROUP ");
		اكسر;

	حالة BOX_FLOAT_کیڼ:
		fprintf(بهیر, "FLOAT_LEFT ");
		اكسر;

	حالة BOX_FLOAT_ښی:
		fprintf(بهیر, "FLOAT_RIGHT ");
		اكسر;

	حالة BOX_BR:
		fprintf(بهیر, "BR ");
		اكسر;

	حالة BOX_متن:
		fprintf(بهیر, "TEXT ");
		اكسر;

	حالة BOX_FLEX:
		fprintf(بهیر, "FLEX ");
		اكسر;

	حالة BOX_INLINE_FLEX:
		fprintf(بهیر, "INLINE_FLEX ");
		اكسر;

	افتراضي:
		fprintf(بهیر, "Unknown box type ");
	}

	إذا (box->متن)
		fprintf(بهیر, "%li '%.*s' ", (بینښه اوږد) box->byte_دننه_واټن,
			(صحيح) box->اوږدتیا, box->متن);
	إذا (box->space)
		fprintf(بهیر, "space ");
	إذا (box->object) {
		fprintf(بهیر, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	إذا (box->iframe) {
		fprintf(بهیر, "(iframe) ");
	}
	إذا (box->gadget)
		fprintf(بهیر, "(gadget) ");
	إذا (style && box->style)
		nscss_dump_computed_style(بهیر, box->style);
	إذا (box->href)
		fprintf(بهیر, " -> '%s'", nsurl_access(box->href));
	إذا (box->موخه)
		fprintf(بهیر, " |%s|", box->موخه);
	إذا (box->title)
		fprintf(بهیر, " [%s]", box->title);
	إذا (box->id)
		fprintf(بهیر, " ID:%s", lwc_string_معلومات(box->id));
	إذا (box->type == BOX_INLINE || box->type == BOX_INLINE_پای_ټکی)
		fprintf(بهیر, " inline_end %p", box->inline_پای_ټکی);
	إذا (box->float_children)
		fprintf(بهیر, " float_children %p", box->float_children);
	إذا (box->بل_float)
		fprintf(بهیر, " next_float %p", box->بل_float);
	إذا (box->float_container)
		fprintf(بهیر, " float_container %p", box->float_container);
	إذا (box->ستون) {
		fprintf(بهیر, " (columns");
		لكل (i = 0; i != box->columns; i++) {
			fprintf(بهیر, " (%s %s %i %i %i)",
				((ثابت حرف *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->ستون[i].type],
				((ثابت حرف *[]) {
					"normal",
					"positioned"})
				[box->ستون[i].positioned],
				box->ستون[i].پلنوالی,
				box->ستون[i].min, box->ستون[i].max);
		}
		fprintf(بهیر, ")");
	}
	إذا (box->غوټه != عدم) {
		dom_string *name;
		إذا (dom_غوټه_get_غوټه_name(box->غوټه, &name) == DOM_NO_ERR) {
			fprintf(بهیر, " <%s>", dom_string_معلومات(name));
			dom_string_unref(name);
		}
	}
	fprintf(بهیر, "\n");

	إذا (box->list_marker) {
		لكل (i = 0; i != ژوروالی; i++)
			fprintf(بهیر, "  ");
		fprintf(بهیر, "list_marker:\n");
		box_dump(بهیر, box->list_marker, ژوروالی + 1, style);
	}

	لكل (c = box->children; c && c->بل; c = c->بل)
		;
	إذا (box->وروستی != c)
		fprintf(بهیر, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->وروستی, c, box);
	لكل (prev = 0, c = box->children; c; prev = c, c = c->بل) {
		إذا (c->پلار != box)
			fprintf(بهیر, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->پلار, box);
		إذا (c->prev != prev)
			fprintf(بهیر, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(بهیر, c, ژوروالی + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[سر_2] +
		box->لوړوالی +
		box->padding[BOTTOM] +
		box->border[BOTTOM].پلنوالی < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[کیڼ_2] +
		box->پلنوالی +
		box->padding[ښی_2] +
		box->border[ښی_2].پلنوالی < box->descendant_x1;
}


/* Exported function documented in html/box.h */
بنية box *
box_pick_متن_box(بنية html_content *html,
		  صحيح x, صحيح y,
		  صحيح dir,
		  صحيح *dx, صحيح *dy)
{
	بنية box *متن_box = عدم;
	بنية box *box;
	صحيح nr_xd, nr_yd;
	صحيح bx, by;
	صحيح fx, fy;
	صحيح tx, ty;

	إذا (html == عدم)
		أرجع عدم;

	box = html->layout;
	bx = box->margin[کیڼ_2];
	by = box->margin[سر_2];
	fx = bx;
	fy = by;

	إذا (!box_تر_ټولو_نږدې_متن_box(box, bx, by, fx, fy, x, y,
				  dir, &متن_box, &tx, &ty, &nr_xd, &nr_yd)) {
		إذا (متن_box && متن_box->متن && !متن_box->object) {
			صحيح w = (متن_box->padding[کیڼ_2] +
				 متن_box->پلنوالی +
				 متن_box->padding[ښی_2]);
			صحيح h = (متن_box->padding[سر_2] +
				 متن_box->لوړوالی +
				 متن_box->padding[BOTTOM]);
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

	أرجع متن_box;
}
