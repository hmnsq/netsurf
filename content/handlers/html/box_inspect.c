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
		   BOX_WALK_والد,
		   BOX_WALK_ايندڙ_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_ايندڙ_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#وصف_ڪريو box_is_float(box) (box->type == BOX_FLOAT_کاٻو ||	\
			   box->type == BOX_FLOAT_ساڄو)

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
box_شامل_آهي_نقطو(ثابت css_unit_ctx *unit_len_ctx,
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
				 .x0 = box->border[کاٻو_2].ويڪر,
				 .y0 = box->border[چوٽي_2].ويڪر,
				 .x1 = box->padding[کاٻو_2] + box->ويڪر +
				 box->border[ساڄو_2].ويڪر +
				 box->padding[ساڄو_2],
				 .y1 = box->padding[چوٽي_2] + box->اوچائي +
				 box->border[BOTTOM].ويڪر +
				 box->padding[BOTTOM]
		};
		إذا (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} وإلا {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		إذا (css_rect.کاٻو_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.کاٻو,
						css_rect.lunit));
		}
		إذا (css_rect.چوٽي_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.چوٽي,
						css_rect.tunit));
		}
		إذا (css_rect.ساڄو_auto == false) {
			r.x1 = box->border[کاٻو_2].ويڪر +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ساڄو,
						css_rect.runit));
		}
		إذا (css_rect.bottom_auto == false) {
			r.y1 = box->border[چوٽي_2].ويڪر +
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
	إذا (x >= -box->border[کاٻو_2].ويڪر &&
	    x < box->padding[کاٻو_2] + box->ويڪر +
	    box->padding[ساڄو_2] + box->border[ساڄو_2].ويڪر &&
	    y >= -box->border[چوٽي_2].ويڪر &&
	    y < box->padding[چوٽي_2] + box->اوچائي +
	    box->padding[BOTTOM] + box->border[BOTTOM].ويڪر) {
		*physically = true;
		أرجع true;
	}
	إذا (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[کاٻو_2].ويڪر &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[کاٻو_2] +
	    box->list_marker->ويڪر +
	    box->list_marker->border[ساڄو_2].ويڪر +
	    box->list_marker->padding[ساڄو_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[چوٽي_2].ويڪر &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[چوٽي_2] +
	    box->list_marker->اوچائي +
	    box->list_marker->border[BOTTOM].ويڪر +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		أرجع true;
	}
	إذا ((box->style && css_computed_حد_کان_مٿي_x(box->style) ==
	     CSS_حد_کان_مٿي_VISIBLE) || !box->style) {
		إذا (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			أرجع true;
		}
	}
	إذا ((box->style && css_computed_حد_کان_مٿي_y(box->style) ==
	     CSS_حد_کان_مٿي_VISIBLE) || !box->style) {
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

	حالة BOX_WALK_ايندڙ_SIBLING:
		افعل {
			*x -= b->x;
			*y -= b->y;
			b = b->ايندڙ;
			إذا (b == عدم)
				اكسر;
			*x += b->x;
			*y += b->y;
		} طالما (box_is_float(b));
		rb = b;
		اكسر;

	حالة BOX_WALK_والد:
		*x -= b->x;
		*y -= b->y;
		rb = b->والد;
		اكسر;

	حالة BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		إذا (b == عدم)
			اكسر;
		*x += b->x;
		*y += b->y;
		rb = b;
		اكسر;

	حالة BOX_WALK_ايندڙ_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->ايندڙ_float;
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
box_ايندڙ_xy(بنية box *b, صحيح *x, صحيح *y, bool skip_children)
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
 پورو_float_children:

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
	n = box_move_xy(b, BOX_WALK_ايندڙ_FLOAT_SIBLING, &tx, &ty);
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
		اذهب پورو_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	طالما (b) {
		طالما (!b->ايندڙ && b->والد) {
			b = box_move_xy(b, BOX_WALK_والد, x, y);
			إذا (box_is_float(b)) {
				/* Go on to next float, if there is one */
				اذهب skip_children;
			}
		}
		إذا (!b->ايندڙ) {
			/* No more boxes */
			أرجع عدم;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_ايندڙ_SIBLING, &tx, &ty);
		إذا (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			أرجع n;

		} وإلا إذا (b->والد) {
			b = box_move_xy(b, BOX_WALK_والد, x, y);
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
		    بنية box **سڀ_کان_ويجهو,
		    صحيح *tx, صحيح *ty,
		    صحيح *nr_xd, صحيح *nr_yd)
{
	صحيح w = box->padding[کاٻو_2] + box->ويڪر + box->padding[ساڄو_2];
	صحيح h = box->padding[چوٽي_2] + box->اوچائي + box->padding[BOTTOM];
	صحيح y1 = by + h;
	صحيح x1 = bx + w;
	صحيح yd = وڌ_کان_وڌ_عدد;
	صحيح xd = وڌ_کان_وڌ_عدد;

	إذا (x >= bx && x1 > x && y >= by && y1 > y) {
		*سڀ_کان_ويجهو = box;
		*tx = bx;
		*ty = by;
		أرجع true;
	}

	إذا (box->والد->list_marker != box) {
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
			*سڀ_کان_ويجهو = box;
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
box_سڀ_کان_ويجهو_متن_box(بنية box *box,
		     صحيح bx, صحيح by,
		     صحيح fx, صحيح fy,
		     صحيح x, صحيح y,
		     صحيح dir,
		     بنية box **سڀ_کان_ويجهو,
		     صحيح *tx, صحيح *ty,
		     صحيح *nr_xd, صحيح *nr_yd)
{
	بنية box *ٻار = box->children;
	صحيح c_bx, c_by;
	صحيح c_fx, c_fy;
	bool اندريون_box = false;

	إذا (*سڀ_کان_ويجهو == عدم) {
		*nr_xd = وڌ_کان_وڌ_عدد / 2; /* displacement of 'nearest so far' */
		*nr_yd = وڌ_کان_وڌ_عدد / 2;
	}
	إذا (box->type == BOX_INLINE_CONTAINER) {
		صحيح bw = box->padding[کاٻو_2] + box->ويڪر + box->padding[ساڄو_2];
		صحيح bh = box->padding[چوٽي_2] + box->اوچائي + box->padding[BOTTOM];
		صحيح b_y1 = by + bh;
		صحيح b_x1 = bx + bw;
		إذا (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			اندريون_box = true;
		}
	}

	طالما (ٻار) {
		إذا (ٻار->type == BOX_FLOAT_کاٻو ||
		    ٻار->type == BOX_FLOAT_ساڄو) {
			c_bx = fx + ٻار->x -
				scrollbar_get_وٿي(ٻار->scroll_x);
			c_by = fy + ٻار->y -
				scrollbar_get_وٿي(ٻار->scroll_y);
		} وإلا {
			c_bx = bx + ٻار->x -
				scrollbar_get_وٿي(ٻار->scroll_x);
			c_by = by + ٻار->y -
				scrollbar_get_وٿي(ٻار->scroll_y);
		}
		إذا (ٻار->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} وإلا {
			c_fx = fx;
			c_fy = fy;
		}
		إذا (اندريون_box && ٻار->متن && !ٻار->object) {
			إذا (box_nearer_متن_box(ٻار,
						c_bx, c_by, x, y, dir, سڀ_کان_ويجهو,
						tx, ty, nr_xd, nr_yd))
				أرجع true;
		} وإلا {
			إذا (ٻار->list_marker) {
				إذا (box_nearer_متن_box(
						ٻار->list_marker,
						c_bx + ٻار->list_marker->x,
						c_by + ٻار->list_marker->y,
						x, y, dir, سڀ_کان_ويجهو,
						tx, ty, nr_xd, nr_yd))
					أرجع true;
			}
			إذا (box_سڀ_کان_ويجهو_متن_box(ٻار, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, سڀ_کان_ويجهو, tx, ty,
						 nr_xd, nr_yd))
				أرجع true;
		}
		ٻار = ٻار->ايندڙ;
	}

	أرجع false;
}


/* Exported function documented in html/box.h */
فراغ box_coords(بنية box *box, صحيح *x, صحيح *y)
{
	*x = box->x;
	*y = box->y;
	طالما (box->والد) {
		إذا (box_is_float(box)) {
			أكد(box->float_container);
			box = box->float_container;
		} وإلا {
			box = box->والد;
		}
		*x += box->x - scrollbar_get_وٿي(box->scroll_x);
		*y += box->y - scrollbar_get_وٿي(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
فراغ box_bounds(بنية box *box, بنية rect *r)
{
	صحيح ويڪر, اوچائي;

	box_coords(box, &r->x0, &r->y0);

	ويڪر = box->padding[کاٻو_2] + box->ويڪر + box->padding[ساڄو_2];
	اوچائي = box->padding[چوٽي_2] + box->اوچائي + box->padding[BOTTOM];

	r->x1 = r->x0 + ويڪر;
	r->y1 = r->y0 + اوچائي;
}


/* Exported function documented in html/box.h */
بنية box *
box_at_نقطو(ثابت css_unit_ctx *unit_len_ctx,
	     بنية box *box,
	     ثابت صحيح x, ثابت صحيح y,
	     صحيح *box_x, صحيح *box_y)
{
	bool skip_children;
	bool physically;

	أكد(box);

	skip_children = false;
	طالما ((box = box_ايندڙ_xy(box, box_x, box_y, skip_children))) {
		إذا (box_شامل_آهي_نقطو(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_وٿي(box->scroll_x);
			*box_y -= scrollbar_get_وٿي(box->scroll_y);

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
بنية box *box_ڳوليو_by_id(بنية box *box, lwc_string *id)
{
	بنية box *a, *b;
	bool m;

	إذا (box->id != عدم &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		أرجع box;
	}

	لكل (a = box->children; a; a = a->ايندڙ) {
		إذا ((b = box_ڳوليو_by_id(a, id)) != عدم) {
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
فراغ box_dump(FILE *وهڪرو, بنية box *box, بنانشان صحيح اونهائي, bool style)
{
	بنانشان صحيح i;
	بنية box *c, *prev;

	لكل (i = 0; i != اونهائي; i++) {
		fprintf(وهڪرو, "  ");
	}

	fprintf(وهڪرو, "%p ", box);
	fprintf(وهڪرو, "x%i y%i w%i h%i ",
		box->x, box->y, box->ويڪر, box->اوچائي);
	إذا (box->max_ويڪر != UNKNOWN_MAX_ويڪر) {
		fprintf(وهڪرو, "min%i max%i ", box->min_ويڪر, box->max_ويڪر);
	}
	fprintf(وهڪرو, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(وهڪرو, "m(%i %i %i %i) ",
		box->margin[چوٽي_2], box->margin[کاٻو_2],
		box->margin[BOTTOM], box->margin[ساڄو_2]);

	بدل (box->type) {
	حالة BOX_بلاڪ:
		fprintf(وهڪرو, "BLOCK ");
		اكسر;

	حالة BOX_INLINE_CONTAINER:
		fprintf(وهڪرو, "INLINE_CONTAINER ");
		اكسر;

	حالة BOX_INLINE:
		fprintf(وهڪرو, "INLINE ");
		اكسر;

	حالة BOX_INLINE_پڇاڙي:
		fprintf(وهڪرو, "INLINE_END ");
		اكسر;

	حالة BOX_INLINE_بلاڪ:
		fprintf(وهڪرو, "INLINE_BLOCK ");
		اكسر;

	حالة BOX_جدول:
		fprintf(وهڪرو, "TABLE [columns %i] ", box->columns);
		اكسر;

	حالة BOX_جدول_صف:
		fprintf(وهڪرو, "TABLE_ROW ");
		اكسر;

	حالة BOX_جدول_CELL:
		fprintf(وهڪرو, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->شروعات_ڪالم,
			box->صفون);
		اكسر;

	حالة BOX_جدول_صف_گروهه:
		fprintf(وهڪرو, "TABLE_ROW_GROUP ");
		اكسر;

	حالة BOX_FLOAT_کاٻو:
		fprintf(وهڪرو, "FLOAT_LEFT ");
		اكسر;

	حالة BOX_FLOAT_ساڄو:
		fprintf(وهڪرو, "FLOAT_RIGHT ");
		اكسر;

	حالة BOX_BR:
		fprintf(وهڪرو, "BR ");
		اكسر;

	حالة BOX_متن:
		fprintf(وهڪرو, "TEXT ");
		اكسر;

	حالة BOX_FLEX:
		fprintf(وهڪرو, "FLEX ");
		اكسر;

	حالة BOX_INLINE_FLEX:
		fprintf(وهڪرو, "INLINE_FLEX ");
		اكسر;

	افتراضي:
		fprintf(وهڪرو, "Unknown box type ");
	}

	إذا (box->متن)
		fprintf(وهڪرو, "%li '%.*s' ", (بنانشان ڊگهو) box->byte_وٿي,
			(صحيح) box->ڊيگهائي, box->متن);
	إذا (box->space)
		fprintf(وهڪرو, "space ");
	إذا (box->object) {
		fprintf(وهڪرو, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	إذا (box->iframe) {
		fprintf(وهڪرو, "(iframe) ");
	}
	إذا (box->gadget)
		fprintf(وهڪرو, "(gadget) ");
	إذا (style && box->style)
		nscss_dump_computed_style(وهڪرو, box->style);
	إذا (box->href)
		fprintf(وهڪرو, " -> '%s'", nsurl_access(box->href));
	إذا (box->هدف)
		fprintf(وهڪرو, " |%s|", box->هدف);
	إذا (box->title)
		fprintf(وهڪرو, " [%s]", box->title);
	إذا (box->id)
		fprintf(وهڪرو, " ID:%s", lwc_string_معلومات(box->id));
	إذا (box->type == BOX_INLINE || box->type == BOX_INLINE_پڇاڙي)
		fprintf(وهڪرو, " inline_end %p", box->inline_پڇاڙي);
	إذا (box->float_children)
		fprintf(وهڪرو, " float_children %p", box->float_children);
	إذا (box->ايندڙ_float)
		fprintf(وهڪرو, " next_float %p", box->ايندڙ_float);
	إذا (box->float_container)
		fprintf(وهڪرو, " float_container %p", box->float_container);
	إذا (box->کانو) {
		fprintf(وهڪرو, " (columns");
		لكل (i = 0; i != box->columns; i++) {
			fprintf(وهڪرو, " (%s %s %i %i %i)",
				((ثابت حرف *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->کانو[i].type],
				((ثابت حرف *[]) {
					"normal",
					"positioned"})
				[box->کانو[i].positioned],
				box->کانو[i].ويڪر,
				box->کانو[i].min, box->کانو[i].max);
		}
		fprintf(وهڪرو, ")");
	}
	إذا (box->ڳنڍ != عدم) {
		dom_string *name;
		إذا (dom_ڳنڍ_get_ڳنڍ_name(box->ڳنڍ, &name) == DOM_NO_ERR) {
			fprintf(وهڪرو, " <%s>", dom_string_معلومات(name));
			dom_string_unref(name);
		}
	}
	fprintf(وهڪرو, "\n");

	إذا (box->list_marker) {
		لكل (i = 0; i != اونهائي; i++)
			fprintf(وهڪرو, "  ");
		fprintf(وهڪرو, "list_marker:\n");
		box_dump(وهڪرو, box->list_marker, اونهائي + 1, style);
	}

	لكل (c = box->children; c && c->ايندڙ; c = c->ايندڙ)
		;
	إذا (box->آخري != c)
		fprintf(وهڪرو, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->آخري, c, box);
	لكل (prev = 0, c = box->children; c; prev = c, c = c->ايندڙ) {
		إذا (c->والد != box)
			fprintf(وهڪرو, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->والد, box);
		إذا (c->prev != prev)
			fprintf(وهڪرو, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(وهڪرو, c, اونهائي + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[چوٽي_2] +
		box->اوچائي +
		box->padding[BOTTOM] +
		box->border[BOTTOM].ويڪر < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(ثابت بنية box * ثابت box)
{
	أرجع box->padding[کاٻو_2] +
		box->ويڪر +
		box->padding[ساڄو_2] +
		box->border[ساڄو_2].ويڪر < box->descendant_x1;
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
	bx = box->margin[کاٻو_2];
	by = box->margin[چوٽي_2];
	fx = bx;
	fy = by;

	إذا (!box_سڀ_کان_ويجهو_متن_box(box, bx, by, fx, fy, x, y,
				  dir, &متن_box, &tx, &ty, &nr_xd, &nr_yd)) {
		إذا (متن_box && متن_box->متن && !متن_box->object) {
			صحيح w = (متن_box->padding[کاٻو_2] +
				 متن_box->ويڪر +
				 متن_box->padding[ساڄو_2]);
			صحيح h = (متن_box->padding[چوٽي_2] +
				 متن_box->اوچائي +
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
