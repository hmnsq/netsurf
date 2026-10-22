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
شمار box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_والد,
		   BOX_WALK_اگلا_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_اگلا_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#تعریف_کرو box_is_float(box) (box->type == BOX_FLOAT_بایاں ||	\
			   box->type == BOX_FLOAT_دایاں)

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
ساکن bool
box_شامل_ہے_نقطہ(ثابت css_unit_ctx *unit_len_ctx,
		   ثابت ساخت box *box,
		   صحیح x,
		   صحیح y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	اگر (box->style != تہی &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		ساخت rect r = {
				 .x0 = box->border[بایاں_2].چوڑائی,
				 .y0 = box->border[چوٹی_2].چوڑائی,
				 .x1 = box->padding[بایاں_2] + box->چوڑائی +
				 box->border[دایاں_2].چوڑائی +
				 box->padding[دایاں_2],
				 .y1 = box->padding[چوٹی_2] + box->اونچائی +
				 box->border[BOTTOM].چوڑائی +
				 box->padding[BOTTOM]
		};
		اگر (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} ورنہ {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		اگر (css_rect.بایاں_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.بایاں,
						css_rect.lunit));
		}
		اگر (css_rect.چوٹی_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.چوٹی,
						css_rect.tunit));
		}
		اگر (css_rect.دایاں_auto == false) {
			r.x1 = box->border[بایاں_2].چوڑائی +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.دایاں,
						css_rect.runit));
		}
		اگر (css_rect.bottom_auto == false) {
			r.y1 = box->border[چوٹی_2].چوڑائی +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		اگر (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			واپس true;
		}

		/* Not inside clip area */
		واپس false;
	}
	اگر (x >= -box->border[بایاں_2].چوڑائی &&
	    x < box->padding[بایاں_2] + box->چوڑائی +
	    box->padding[دایاں_2] + box->border[دایاں_2].چوڑائی &&
	    y >= -box->border[چوٹی_2].چوڑائی &&
	    y < box->padding[چوٹی_2] + box->اونچائی +
	    box->padding[BOTTOM] + box->border[BOTTOM].چوڑائی) {
		*physically = true;
		واپس true;
	}
	اگر (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[بایاں_2].چوڑائی &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[بایاں_2] +
	    box->list_marker->چوڑائی +
	    box->list_marker->border[دایاں_2].چوڑائی +
	    box->list_marker->padding[دایاں_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[چوٹی_2].چوڑائی &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[چوٹی_2] +
	    box->list_marker->اونچائی +
	    box->list_marker->border[BOTTOM].چوڑائی +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		واپس true;
	}
	اگر ((box->style && css_computed_حد_سے_تجاوز_x(box->style) ==
	     CSS_حد_سے_تجاوز_VISIBLE) || !box->style) {
		اگر (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			واپس true;
		}
	}
	اگر ((box->style && css_computed_حد_سے_تجاوز_y(box->style) ==
	     CSS_حد_سے_تجاوز_VISIBLE) || !box->style) {
		اگر (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			واپس true;
		}
	}
	واپس false;
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
ساکن اندرونی ساخت box *
box_move_xy(ساخت box *b, شمار box_walk_dir dir, صحیح *x, صحیح *y)
{
	ساخت box *rb = تہی;

	انتخاب (dir) {
	حالت BOX_WALK_CHILDREN:
		b = b->children;
		اگر (b == تہی)
			توڑو;
		*x += b->x;
		*y += b->y;
		اگر (!box_is_float(b)) {
			rb = b;
			توڑو;
		}
		fallthrough;

	حالت BOX_WALK_اگلا_SIBLING:
		کرو {
			*x -= b->x;
			*y -= b->y;
			b = b->اگلا;
			اگر (b == تہی)
				توڑو;
			*x += b->x;
			*y += b->y;
		} جب (box_is_float(b));
		rb = b;
		توڑو;

	حالت BOX_WALK_والد:
		*x -= b->x;
		*y -= b->y;
		rb = b->والد;
		توڑو;

	حالت BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		اگر (b == تہی)
			توڑو;
		*x += b->x;
		*y += b->y;
		rb = b;
		توڑو;

	حالت BOX_WALK_اگلا_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->اگلا_float;
		اگر (b == تہی)
			توڑو;
		*x += b->x;
		*y += b->y;
		rb = b;
		توڑو;

	حالت BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		توڑو;

	طےشدہ:
		أكد(0 && "Bad box walk type.");
	}

	واپس rb;
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
ساکن اندرونی ساخت box *
box_اگلا_xy(ساخت box *b, صحیح *x, صحیح *y, bool skip_children)
{
	ساخت box *n;
	صحیح tx, ty;

	أكد(b != تہی);

	اگر (skip_children) {
		/* Caller is not interested in any kind of children */
		جاؤ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	اگر (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		واپس n;
	}
 مکمل_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	اگر (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		واپس n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_اگلا_FLOAT_SIBLING, &tx, &ty);
	اگر (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		واپس n;
	}

	اگر (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		جاؤ مکمل_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	جب (b) {
		جب (!b->اگلا && b->والد) {
			b = box_move_xy(b, BOX_WALK_والد, x, y);
			اگر (box_is_float(b)) {
				/* Go on to next float, if there is one */
				جاؤ skip_children;
			}
		}
		اگر (!b->اگلا) {
			/* No more boxes */
			واپس تہی;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_اگلا_SIBLING, &tx, &ty);
		اگر (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			واپس n;

		} ورنہ اگر (b->والد) {
			b = box_move_xy(b, BOX_WALK_والد, x, y);
			اگر (box_is_float(b)) {
				/* Go on to next float, if there is one */
				جاؤ skip_children;
			}

		} ورنہ {
			/* No more boxes */
			واپس تہی;
		}
	}

	أكد(b != تہی);
	واپس تہی;
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
ساکن bool
box_nearer_متن_box(ساخت box *box,
		    صحیح bx, صحیح by,
		    صحیح x, صحیح y,
		    صحیح dir,
		    ساخت box **قریب_ترین,
		    صحیح *tx, صحیح *ty,
		    صحیح *nr_xd, صحیح *nr_yd)
{
	صحیح w = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2];
	صحیح h = box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM];
	صحیح y1 = by + h;
	صحیح x1 = bx + w;
	صحیح yd = زیادہ_سے_زیادہ_عدد;
	صحیح xd = زیادہ_سے_زیادہ_عدد;

	اگر (x >= bx && x1 > x && y >= by && y1 > y) {
		*قریب_ترین = box;
		*tx = bx;
		*ty = by;
		واپس true;
	}

	اگر (box->والد->list_marker != box) {
		اگر (dir < 0) {
			/* consider only those children (partly) above-left */
			اگر (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} ورنہ {
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
			*قریب_ترین = box;
			*tx = bx;
			*ty = by;
		}
	}
	واپس false;
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
ساکن bool
box_قریب_ترین_متن_box(ساخت box *box,
		     صحیح bx, صحیح by,
		     صحیح fx, صحیح fy,
		     صحیح x, صحیح y,
		     صحیح dir,
		     ساخت box **قریب_ترین,
		     صحیح *tx, صحیح *ty,
		     صحیح *nr_xd, صحیح *nr_yd)
{
	ساخت box *بچہ = box->children;
	صحیح c_bx, c_by;
	صحیح c_fx, c_fy;
	bool اندر_box = false;

	اگر (*قریب_ترین == تہی) {
		*nr_xd = زیادہ_سے_زیادہ_عدد / 2; /* displacement of 'nearest so far' */
		*nr_yd = زیادہ_سے_زیادہ_عدد / 2;
	}
	اگر (box->type == BOX_INLINE_CONTAINER) {
		صحیح bw = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2];
		صحیح bh = box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM];
		صحیح b_y1 = by + bh;
		صحیح b_x1 = bx + bw;
		اگر (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			اندر_box = true;
		}
	}

	جب (بچہ) {
		اگر (بچہ->type == BOX_FLOAT_بایاں ||
		    بچہ->type == BOX_FLOAT_دایاں) {
			c_bx = fx + بچہ->x -
				scrollbar_get_انحراف(بچہ->scroll_x);
			c_by = fy + بچہ->y -
				scrollbar_get_انحراف(بچہ->scroll_y);
		} ورنہ {
			c_bx = bx + بچہ->x -
				scrollbar_get_انحراف(بچہ->scroll_x);
			c_by = by + بچہ->y -
				scrollbar_get_انحراف(بچہ->scroll_y);
		}
		اگر (بچہ->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} ورنہ {
			c_fx = fx;
			c_fy = fy;
		}
		اگر (اندر_box && بچہ->متن && !بچہ->object) {
			اگر (box_nearer_متن_box(بچہ,
						c_bx, c_by, x, y, dir, قریب_ترین,
						tx, ty, nr_xd, nr_yd))
				واپس true;
		} ورنہ {
			اگر (بچہ->list_marker) {
				اگر (box_nearer_متن_box(
						بچہ->list_marker,
						c_bx + بچہ->list_marker->x,
						c_by + بچہ->list_marker->y,
						x, y, dir, قریب_ترین,
						tx, ty, nr_xd, nr_yd))
					واپس true;
			}
			اگر (box_قریب_ترین_متن_box(بچہ, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, قریب_ترین, tx, ty,
						 nr_xd, nr_yd))
				واپس true;
		}
		بچہ = بچہ->اگلا;
	}

	واپس false;
}


/* Exported function documented in html/box.h */
خالی box_coords(ساخت box *box, صحیح *x, صحیح *y)
{
	*x = box->x;
	*y = box->y;
	جب (box->والد) {
		اگر (box_is_float(box)) {
			أكد(box->float_container);
			box = box->float_container;
		} ورنہ {
			box = box->والد;
		}
		*x += box->x - scrollbar_get_انحراف(box->scroll_x);
		*y += box->y - scrollbar_get_انحراف(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
خالی box_bounds(ساخت box *box, ساخت rect *r)
{
	صحیح چوڑائی, اونچائی;

	box_coords(box, &r->x0, &r->y0);

	چوڑائی = box->padding[بایاں_2] + box->چوڑائی + box->padding[دایاں_2];
	اونچائی = box->padding[چوٹی_2] + box->اونچائی + box->padding[BOTTOM];

	r->x1 = r->x0 + چوڑائی;
	r->y1 = r->y0 + اونچائی;
}


/* Exported function documented in html/box.h */
ساخت box *
box_at_نقطہ(ثابت css_unit_ctx *unit_len_ctx,
	     ساخت box *box,
	     ثابت صحیح x, ثابت صحیح y,
	     صحیح *box_x, صحیح *box_y)
{
	bool skip_children;
	bool physically;

	أكد(box);

	skip_children = false;
	جب ((box = box_اگلا_xy(box, box_x, box_y, skip_children))) {
		اگر (box_شامل_ہے_نقطہ(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_انحراف(box->scroll_x);
			*box_y -= scrollbar_get_انحراف(box->scroll_y);

			اگر (physically)
				واپس box;

			skip_children = false;
		} ورنہ {
			skip_children = true;
		}
	}

	واپس تہی;
}


/* Exported function documented in html/box.h */
ساخت box *box_ڈھونڈو_by_id(ساخت box *box, lwc_string *id)
{
	ساخت box *a, *b;
	bool m;

	اگر (box->id != تہی &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		واپس box;
	}

	برائے (a = box->children; a; a = a->اگلا) {
		اگر ((b = box_ڈھونڈو_by_id(a, id)) != تہی) {
			واپس b;
		}
	}

	واپس تہی;
}


/* Exported function documented in html/box.h */
bool box_visible(ساخت box *box)
{
	/* visibility: hidden */
	اگر (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		واپس false;
	}

	واپس true;
}


/* Exported function documented in html/box.h */
خالی box_dump(FILE *دھارا, ساخت box *box, غیرعلامتی صحیح گہرائی, bool style)
{
	غیرعلامتی صحیح i;
	ساخت box *c, *پچھلا;

	برائے (i = 0; i != گہرائی; i++) {
		fprintf(دھارا, "  ");
	}

	fprintf(دھارا, "%p ", box);
	fprintf(دھارا, "x%i y%i w%i h%i ",
		box->x, box->y, box->چوڑائی, box->اونچائی);
	اگر (box->max_چوڑائی != UNKNOWN_MAX_چوڑائی) {
		fprintf(دھارا, "min%i max%i ", box->min_چوڑائی, box->max_چوڑائی);
	}
	fprintf(دھارا, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(دھارا, "m(%i %i %i %i) ",
		box->margin[چوٹی_2], box->margin[بایاں_2],
		box->margin[BOTTOM], box->margin[دایاں_2]);

	انتخاب (box->type) {
	حالت BOX_بلاک:
		fprintf(دھارا, "BLOCK ");
		توڑو;

	حالت BOX_INLINE_CONTAINER:
		fprintf(دھارا, "INLINE_CONTAINER ");
		توڑو;

	حالت BOX_INLINE:
		fprintf(دھارا, "INLINE ");
		توڑو;

	حالت BOX_INLINE_انتہا:
		fprintf(دھارا, "INLINE_END ");
		توڑو;

	حالت BOX_INLINE_بلاک:
		fprintf(دھارا, "INLINE_BLOCK ");
		توڑو;

	حالت BOX_جدول:
		fprintf(دھارا, "TABLE [columns %i] ", box->columns);
		توڑو;

	حالت BOX_جدول_صف:
		fprintf(دھارا, "TABLE_ROW ");
		توڑو;

	حالت BOX_جدول_CELL:
		fprintf(دھارا, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->آغاز_عمود,
			box->صفیں);
		توڑو;

	حالت BOX_جدول_صف_گروہ:
		fprintf(دھارا, "TABLE_ROW_GROUP ");
		توڑو;

	حالت BOX_FLOAT_بایاں:
		fprintf(دھارا, "FLOAT_LEFT ");
		توڑو;

	حالت BOX_FLOAT_دایاں:
		fprintf(دھارا, "FLOAT_RIGHT ");
		توڑو;

	حالت BOX_BR:
		fprintf(دھارا, "BR ");
		توڑو;

	حالت BOX_متن:
		fprintf(دھارا, "TEXT ");
		توڑو;

	حالت BOX_FLEX:
		fprintf(دھارا, "FLEX ");
		توڑو;

	حالت BOX_INLINE_FLEX:
		fprintf(دھارا, "INLINE_FLEX ");
		توڑو;

	طےشدہ:
		fprintf(دھارا, "Unknown box type ");
	}

	اگر (box->متن)
		fprintf(دھارا, "%li '%.*s' ", (غیرعلامتی طویل) box->byte_انحراف,
			(صحیح) box->لمبائی, box->متن);
	اگر (box->space)
		fprintf(دھارا, "space ");
	اگر (box->object) {
		fprintf(دھارا, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	اگر (box->iframe) {
		fprintf(دھارا, "(iframe) ");
	}
	اگر (box->gadget)
		fprintf(دھارا, "(gadget) ");
	اگر (style && box->style)
		nscss_dump_computed_style(دھارا, box->style);
	اگر (box->href)
		fprintf(دھارا, " -> '%s'", nsurl_access(box->href));
	اگر (box->ہدف)
		fprintf(دھارا, " |%s|", box->ہدف);
	اگر (box->title)
		fprintf(دھارا, " [%s]", box->title);
	اگر (box->id)
		fprintf(دھارا, " ID:%s", lwc_string_ڈیٹا(box->id));
	اگر (box->type == BOX_INLINE || box->type == BOX_INLINE_انتہا)
		fprintf(دھارا, " inline_end %p", box->inline_انتہا);
	اگر (box->float_children)
		fprintf(دھارا, " float_children %p", box->float_children);
	اگر (box->اگلا_float)
		fprintf(دھارا, " next_float %p", box->اگلا_float);
	اگر (box->float_container)
		fprintf(دھارا, " float_container %p", box->float_container);
	اگر (box->ستون) {
		fprintf(دھارا, " (columns");
		برائے (i = 0; i != box->columns; i++) {
			fprintf(دھارا, " (%s %s %i %i %i)",
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
				box->ستون[i].چوڑائی,
				box->ستون[i].min, box->ستون[i].max);
		}
		fprintf(دھارا, ")");
	}
	اگر (box->گرہ != تہی) {
		dom_string *name;
		اگر (dom_گرہ_get_گرہ_name(box->گرہ, &name) == DOM_NO_ERR) {
			fprintf(دھارا, " <%s>", dom_string_ڈیٹا(name));
			dom_string_unref(name);
		}
	}
	fprintf(دھارا, "\n");

	اگر (box->list_marker) {
		برائے (i = 0; i != گہرائی; i++)
			fprintf(دھارا, "  ");
		fprintf(دھارا, "list_marker:\n");
		box_dump(دھارا, box->list_marker, گہرائی + 1, style);
	}

	برائے (c = box->children; c && c->اگلا; c = c->اگلا)
		;
	اگر (box->آخری != c)
		fprintf(دھارا, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->آخری, c, box);
	برائے (پچھلا = 0, c = box->children; c; پچھلا = c, c = c->اگلا) {
		اگر (c->والد != box)
			fprintf(دھارا, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->والد, box);
		اگر (c->پچھلا != پچھلا)
			fprintf(دھارا, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->پچھلا, پچھلا);
		box_dump(دھارا, c, گہرائی + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ثابت ساخت box * ثابت box)
{
	واپس box->padding[چوٹی_2] +
		box->اونچائی +
		box->padding[BOTTOM] +
		box->border[BOTTOM].چوڑائی < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ثابت ساخت box * ثابت box)
{
	واپس box->padding[بایاں_2] +
		box->چوڑائی +
		box->padding[دایاں_2] +
		box->border[دایاں_2].چوڑائی < box->descendant_x1;
}


/* Exported function documented in html/box.h */
ساخت box *
box_pick_متن_box(ساخت html_content *html,
		  صحیح x, صحیح y,
		  صحیح dir,
		  صحیح *dx, صحیح *dy)
{
	ساخت box *متن_box = تہی;
	ساخت box *box;
	صحیح nr_xd, nr_yd;
	صحیح bx, by;
	صحیح fx, fy;
	صحیح tx, ty;

	اگر (html == تہی)
		واپس تہی;

	box = html->layout;
	bx = box->margin[بایاں_2];
	by = box->margin[چوٹی_2];
	fx = bx;
	fy = by;

	اگر (!box_قریب_ترین_متن_box(box, bx, by, fx, fy, x, y,
				  dir, &متن_box, &tx, &ty, &nr_xd, &nr_yd)) {
		اگر (متن_box && متن_box->متن && !متن_box->object) {
			صحیح w = (متن_box->padding[بایاں_2] +
				 متن_box->چوڑائی +
				 متن_box->padding[دایاں_2]);
			صحیح h = (متن_box->padding[چوٹی_2] +
				 متن_box->اونچائی +
				 متن_box->padding[BOTTOM]);
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

	واپس متن_box;
}
