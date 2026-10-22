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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/utils.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "utils/errors.h"
#అన్తర్భావ "netsurf/types.h"
#అన్తర్భావ "netsurf/content.h"
#అన్తర్భావ "netsurf/mouse.h"
#అన్తర్భావ "css/utils.h"
#అన్తర్భావ "css/dump.h"
#అన్తర్భావ "desktop/scrollbar.h"

#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
గణన box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_మాతృ,
		   BOX_WALK_తదుపరి_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_తదుపరి_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#నిర్వచించు box_is_float(box) (box->type == BOX_FLOAT_ఎడమ ||	\
			   box->type == BOX_FLOAT_కుడి)

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
స్థిర bool
box_కలిగియుంది_బిందువు(స్థిరరాశి css_unit_ctx *unit_len_ctx,
		   స్థిరరాశి నిర్మాణం box *box,
		   పూర్ణాంకం x,
		   పూర్ణాంకం y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	ఒకవేళ (box->style != ఖాళీ &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		నిర్మాణం rect r = {
				 .x0 = box->border[ఎడమ_2].వెడల్పు,
				 .y0 = box->border[శిఖరం_2].వెడల్పు,
				 .x1 = box->padding[ఎడమ_2] + box->వెడల్పు +
				 box->border[కుడి_2].వెడల్పు +
				 box->padding[కుడి_2],
				 .y1 = box->padding[శిఖరం_2] + box->ఎత్తు +
				 box->border[BOTTOM].వెడల్పు +
				 box->padding[BOTTOM]
		};
		ఒకవేళ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} లేకపోతే {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		ఒకవేళ (css_rect.ఎడమ_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ఎడమ,
						css_rect.lunit));
		}
		ఒకవేళ (css_rect.శిఖరం_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.శిఖరం,
						css_rect.tunit));
		}
		ఒకవేళ (css_rect.కుడి_auto == false) {
			r.x1 = box->border[ఎడమ_2].వెడల్పు +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.కుడి,
						css_rect.runit));
		}
		ఒకవేళ (css_rect.bottom_auto == false) {
			r.y1 = box->border[శిఖరం_2].వెడల్పు +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		ఒకవేళ (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			తిరిగి true;
		}

		/* Not inside clip area */
		తిరిగి false;
	}
	ఒకవేళ (x >= -box->border[ఎడమ_2].వెడల్పు &&
	    x < box->padding[ఎడమ_2] + box->వెడల్పు +
	    box->padding[కుడి_2] + box->border[కుడి_2].వెడల్పు &&
	    y >= -box->border[శిఖరం_2].వెడల్పు &&
	    y < box->padding[శిఖరం_2] + box->ఎత్తు +
	    box->padding[BOTTOM] + box->border[BOTTOM].వెడల్పు) {
		*physically = true;
		తిరిగి true;
	}
	ఒకవేళ (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ఎడమ_2].వెడల్పు &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ఎడమ_2] +
	    box->list_marker->వెడల్పు +
	    box->list_marker->border[కుడి_2].వెడల్పు +
	    box->list_marker->padding[కుడి_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[శిఖరం_2].వెడల్పు &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[శిఖరం_2] +
	    box->list_marker->ఎత్తు +
	    box->list_marker->border[BOTTOM].వెడల్పు +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		తిరిగి true;
	}
	ఒకవేళ ((box->style && css_computed_అధికప్రవాహం_x(box->style) ==
	     CSS_అధికప్రవాహం_VISIBLE) || !box->style) {
		ఒకవేళ (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			తిరిగి true;
		}
	}
	ఒకవేళ ((box->style && css_computed_అధికప్రవాహం_y(box->style) ==
	     CSS_అధికప్రవాహం_VISIBLE) || !box->style) {
		ఒకవేళ (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			తిరిగి true;
		}
	}
	తిరిగి false;
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
స్థిర అంతఃవరుస నిర్మాణం box *
box_move_xy(నిర్మాణం box *b, గణన box_walk_dir dir, పూర్ణాంకం *x, పూర్ణాంకం *y)
{
	నిర్మాణం box *rb = ఖాళీ;

	ఎంపిక (dir) {
	స్థితి BOX_WALK_CHILDREN:
		b = b->children;
		ఒకవేళ (b == ఖాళీ)
			ఆపు;
		*x += b->x;
		*y += b->y;
		ఒకవేళ (!box_is_float(b)) {
			rb = b;
			ఆపు;
		}
		fallthrough;

	స్థితి BOX_WALK_తదుపరి_SIBLING:
		చేయి {
			*x -= b->x;
			*y -= b->y;
			b = b->తదుపరి;
			ఒకవేళ (b == ఖాళీ)
				ఆపు;
			*x += b->x;
			*y += b->y;
		} అయితే (box_is_float(b));
		rb = b;
		ఆపు;

	స్థితి BOX_WALK_మాతృ:
		*x -= b->x;
		*y -= b->y;
		rb = b->మాతృ;
		ఆపు;

	స్థితి BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		ఒకవేళ (b == ఖాళీ)
			ఆపు;
		*x += b->x;
		*y += b->y;
		rb = b;
		ఆపు;

	స్థితి BOX_WALK_తదుపరి_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->తదుపరి_float;
		ఒకవేళ (b == ఖాళీ)
			ఆపు;
		*x += b->x;
		*y += b->y;
		rb = b;
		ఆపు;

	స్థితి BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		ఆపు;

	అప్రమేయ:
		ప్రతిజ్ఞా(0 && "Bad box walk type.");
	}

	తిరిగి rb;
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
స్థిర అంతఃవరుస నిర్మాణం box *
box_తదుపరి_xy(నిర్మాణం box *b, పూర్ణాంకం *x, పూర్ణాంకం *y, bool skip_children)
{
	నిర్మాణం box *n;
	పూర్ణాంకం tx, ty;

	ప్రతిజ్ఞా(b != ఖాళీ);

	ఒకవేళ (skip_children) {
		/* Caller is not interested in any kind of children */
		వెళ్ళు skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	ఒకవేళ (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		తిరిగి n;
	}
 పూర్తి_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	ఒకవేళ (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		తిరిగి n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_తదుపరి_FLOAT_SIBLING, &tx, &ty);
	ఒకవేళ (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		తిరిగి n;
	}

	ఒకవేళ (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		వెళ్ళు పూర్తి_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	అయితే (b) {
		అయితే (!b->తదుపరి && b->మాతృ) {
			b = box_move_xy(b, BOX_WALK_మాతృ, x, y);
			ఒకవేళ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				వెళ్ళు skip_children;
			}
		}
		ఒకవేళ (!b->తదుపరి) {
			/* No more boxes */
			తిరిగి ఖాళీ;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_తదుపరి_SIBLING, &tx, &ty);
		ఒకవేళ (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			తిరిగి n;

		} లేకపోతే ఒకవేళ (b->మాతృ) {
			b = box_move_xy(b, BOX_WALK_మాతృ, x, y);
			ఒకవేళ (box_is_float(b)) {
				/* Go on to next float, if there is one */
				వెళ్ళు skip_children;
			}

		} లేకపోతే {
			/* No more boxes */
			తిరిగి ఖాళీ;
		}
	}

	ప్రతిజ్ఞా(b != ఖాళీ);
	తిరిగి ఖాళీ;
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
స్థిర bool
box_nearer_పాఠ్యం_box(నిర్మాణం box *box,
		    పూర్ణాంకం bx, పూర్ణాంకం by,
		    పూర్ణాంకం x, పూర్ణాంకం y,
		    పూర్ణాంకం dir,
		    నిర్మాణం box **సమీప,
		    పూర్ణాంకం *tx, పూర్ణాంకం *ty,
		    పూర్ణాంకం *nr_xd, పూర్ణాంకం *nr_yd)
{
	పూర్ణాంకం w = box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2];
	పూర్ణాంకం h = box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM];
	పూర్ణాంకం y1 = by + h;
	పూర్ణాంకం x1 = bx + w;
	పూర్ణాంకం yd = గరిష్ఠ_పూర్ణాంకం;
	పూర్ణాంకం xd = గరిష్ఠ_పూర్ణాంకం;

	ఒకవేళ (x >= bx && x1 > x && y >= by && y1 > y) {
		*సమీప = box;
		*tx = bx;
		*ty = by;
		తిరిగి true;
	}

	ఒకవేళ (box->మాతృ->list_marker != box) {
		ఒకవేళ (dir < 0) {
			/* consider only those children (partly) above-left */
			ఒకవేళ (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} లేకపోతే {
			/* consider only those children (partly) below-right */
			ఒకవేళ (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		ఒకవేళ (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*సమీప = box;
			*tx = bx;
			*ty = by;
		}
	}
	తిరిగి false;
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
స్థిర bool
box_సమీప_పాఠ్యం_box(నిర్మాణం box *box,
		     పూర్ణాంకం bx, పూర్ణాంకం by,
		     పూర్ణాంకం fx, పూర్ణాంకం fy,
		     పూర్ణాంకం x, పూర్ణాంకం y,
		     పూర్ణాంకం dir,
		     నిర్మాణం box **సమీప,
		     పూర్ణాంకం *tx, పూర్ణాంకం *ty,
		     పూర్ణాంకం *nr_xd, పూర్ణాంకం *nr_yd)
{
	నిర్మాణం box *బిడ్డ = box->children;
	పూర్ణాంకం c_bx, c_by;
	పూర్ణాంకం c_fx, c_fy;
	bool లోపల_box = false;

	ఒకవేళ (*సమీప == ఖాళీ) {
		*nr_xd = గరిష్ఠ_పూర్ణాంకం / 2; /* displacement of 'nearest so far' */
		*nr_yd = గరిష్ఠ_పూర్ణాంకం / 2;
	}
	ఒకవేళ (box->type == BOX_INLINE_CONTAINER) {
		పూర్ణాంకం bw = box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2];
		పూర్ణాంకం bh = box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM];
		పూర్ణాంకం b_y1 = by + bh;
		పూర్ణాంకం b_x1 = bx + bw;
		ఒకవేళ (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			లోపల_box = true;
		}
	}

	అయితే (బిడ్డ) {
		ఒకవేళ (బిడ్డ->type == BOX_FLOAT_ఎడమ ||
		    బిడ్డ->type == BOX_FLOAT_కుడి) {
			c_bx = fx + బిడ్డ->x -
				scrollbar_get_స్థానభ్రంశం(బిడ్డ->scroll_x);
			c_by = fy + బిడ్డ->y -
				scrollbar_get_స్థానభ్రంశం(బిడ్డ->scroll_y);
		} లేకపోతే {
			c_bx = bx + బిడ్డ->x -
				scrollbar_get_స్థానభ్రంశం(బిడ్డ->scroll_x);
			c_by = by + బిడ్డ->y -
				scrollbar_get_స్థానభ్రంశం(బిడ్డ->scroll_y);
		}
		ఒకవేళ (బిడ్డ->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} లేకపోతే {
			c_fx = fx;
			c_fy = fy;
		}
		ఒకవేళ (లోపల_box && బిడ్డ->పాఠ్యం && !బిడ్డ->object) {
			ఒకవేళ (box_nearer_పాఠ్యం_box(బిడ్డ,
						c_bx, c_by, x, y, dir, సమీప,
						tx, ty, nr_xd, nr_yd))
				తిరిగి true;
		} లేకపోతే {
			ఒకవేళ (బిడ్డ->list_marker) {
				ఒకవేళ (box_nearer_పాఠ్యం_box(
						బిడ్డ->list_marker,
						c_bx + బిడ్డ->list_marker->x,
						c_by + బిడ్డ->list_marker->y,
						x, y, dir, సమీప,
						tx, ty, nr_xd, nr_yd))
					తిరిగి true;
			}
			ఒకవేళ (box_సమీప_పాఠ్యం_box(బిడ్డ, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, సమీప, tx, ty,
						 nr_xd, nr_yd))
				తిరిగి true;
		}
		బిడ్డ = బిడ్డ->తదుపరి;
	}

	తిరిగి false;
}


/* Exported function documented in html/box.h */
శూన్య box_coords(నిర్మాణం box *box, పూర్ణాంకం *x, పూర్ణాంకం *y)
{
	*x = box->x;
	*y = box->y;
	అయితే (box->మాతృ) {
		ఒకవేళ (box_is_float(box)) {
			ప్రతిజ్ఞా(box->float_container);
			box = box->float_container;
		} లేకపోతే {
			box = box->మాతృ;
		}
		*x += box->x - scrollbar_get_స్థానభ్రంశం(box->scroll_x);
		*y += box->y - scrollbar_get_స్థానభ్రంశం(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
శూన్య box_bounds(నిర్మాణం box *box, నిర్మాణం rect *r)
{
	పూర్ణాంకం వెడల్పు, ఎత్తు;

	box_coords(box, &r->x0, &r->y0);

	వెడల్పు = box->padding[ఎడమ_2] + box->వెడల్పు + box->padding[కుడి_2];
	ఎత్తు = box->padding[శిఖరం_2] + box->ఎత్తు + box->padding[BOTTOM];

	r->x1 = r->x0 + వెడల్పు;
	r->y1 = r->y0 + ఎత్తు;
}


/* Exported function documented in html/box.h */
నిర్మాణం box *
box_at_బిందువు(స్థిరరాశి css_unit_ctx *unit_len_ctx,
	     నిర్మాణం box *box,
	     స్థిరరాశి పూర్ణాంకం x, స్థిరరాశి పూర్ణాంకం y,
	     పూర్ణాంకం *box_x, పూర్ణాంకం *box_y)
{
	bool skip_children;
	bool physically;

	ప్రతిజ్ఞా(box);

	skip_children = false;
	అయితే ((box = box_తదుపరి_xy(box, box_x, box_y, skip_children))) {
		ఒకవేళ (box_కలిగియుంది_బిందువు(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_స్థానభ్రంశం(box->scroll_x);
			*box_y -= scrollbar_get_స్థానభ్రంశం(box->scroll_y);

			ఒకవేళ (physically)
				తిరిగి box;

			skip_children = false;
		} లేకపోతే {
			skip_children = true;
		}
	}

	తిరిగి ఖాళీ;
}


/* Exported function documented in html/box.h */
నిర్మాణం box *box_కనుగొను_by_id(నిర్మాణం box *box, lwc_string *id)
{
	నిర్మాణం box *a, *b;
	bool m;

	ఒకవేళ (box->id != ఖాళీ &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		తిరిగి box;
	}

	కొరకు (a = box->children; a; a = a->తదుపరి) {
		ఒకవేళ ((b = box_కనుగొను_by_id(a, id)) != ఖాళీ) {
			తిరిగి b;
		}
	}

	తిరిగి ఖాళీ;
}


/* Exported function documented in html/box.h */
bool box_visible(నిర్మాణం box *box)
{
	/* visibility: hidden */
	ఒకవేళ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		తిరిగి false;
	}

	తిరిగి true;
}


/* Exported function documented in html/box.h */
శూన్య box_dump(FILE *ప్రవాహం, నిర్మాణం box *box, గుర్తులేని పూర్ణాంకం లోతు, bool style)
{
	గుర్తులేని పూర్ణాంకం i;
	నిర్మాణం box *c, *prev;

	కొరకు (i = 0; i != లోతు; i++) {
		fprintf(ప్రవాహం, "  ");
	}

	fprintf(ప్రవాహం, "%p ", box);
	fprintf(ప్రవాహం, "x%i y%i w%i h%i ",
		box->x, box->y, box->వెడల్పు, box->ఎత్తు);
	ఒకవేళ (box->max_వెడల్పు != UNKNOWN_MAX_వెడల్పు) {
		fprintf(ప్రవాహం, "min%i max%i ", box->min_వెడల్పు, box->max_వెడల్పు);
	}
	fprintf(ప్రవాహం, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(ప్రవాహం, "m(%i %i %i %i) ",
		box->margin[శిఖరం_2], box->margin[ఎడమ_2],
		box->margin[BOTTOM], box->margin[కుడి_2]);

	ఎంపిక (box->type) {
	స్థితి BOX_ఖండం:
		fprintf(ప్రవాహం, "BLOCK ");
		ఆపు;

	స్థితి BOX_INLINE_CONTAINER:
		fprintf(ప్రవాహం, "INLINE_CONTAINER ");
		ఆపు;

	స్థితి BOX_INLINE:
		fprintf(ప్రవాహం, "INLINE ");
		ఆపు;

	స్థితి BOX_INLINE_చివర:
		fprintf(ప్రవాహం, "INLINE_END ");
		ఆపు;

	స్థితి BOX_INLINE_ఖండం:
		fprintf(ప్రవాహం, "INLINE_BLOCK ");
		ఆపు;

	స్థితి BOX_పట్టిక:
		fprintf(ప్రవాహం, "TABLE [columns %i] ", box->columns);
		ఆపు;

	స్థితి BOX_పట్టిక_అడ్డువరుస:
		fprintf(ప్రవాహం, "TABLE_ROW ");
		ఆపు;

	స్థితి BOX_పట్టిక_CELL:
		fprintf(ప్రవాహం, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->ప్రారంభం_స్తంభం,
			box->అడ్డువరుసలు);
		ఆపు;

	స్థితి BOX_పట్టిక_అడ్డువరుస_సమూహం:
		fprintf(ప్రవాహం, "TABLE_ROW_GROUP ");
		ఆపు;

	స్థితి BOX_FLOAT_ఎడమ:
		fprintf(ప్రవాహం, "FLOAT_LEFT ");
		ఆపు;

	స్థితి BOX_FLOAT_కుడి:
		fprintf(ప్రవాహం, "FLOAT_RIGHT ");
		ఆపు;

	స్థితి BOX_BR:
		fprintf(ప్రవాహం, "BR ");
		ఆపు;

	స్థితి BOX_పాఠ్యం:
		fprintf(ప్రవాహం, "TEXT ");
		ఆపు;

	స్థితి BOX_FLEX:
		fprintf(ప్రవాహం, "FLEX ");
		ఆపు;

	స్థితి BOX_INLINE_FLEX:
		fprintf(ప్రవాహం, "INLINE_FLEX ");
		ఆపు;

	అప్రమేయ:
		fprintf(ప్రవాహం, "Unknown box type ");
	}

	ఒకవేళ (box->పాఠ్యం)
		fprintf(ప్రవాహం, "%li '%.*s' ", (గుర్తులేని దీర్ఘ) box->byte_స్థానభ్రంశం,
			(పూర్ణాంకం) box->నిడివి, box->పాఠ్యం);
	ఒకవేళ (box->space)
		fprintf(ప్రవాహం, "space ");
	ఒకవేళ (box->object) {
		fprintf(ప్రవాహం, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	ఒకవేళ (box->iframe) {
		fprintf(ప్రవాహం, "(iframe) ");
	}
	ఒకవేళ (box->gadget)
		fprintf(ప్రవాహం, "(gadget) ");
	ఒకవేళ (style && box->style)
		nscss_dump_computed_style(ప్రవాహం, box->style);
	ఒకవేళ (box->href)
		fprintf(ప్రవాహం, " -> '%s'", nsurl_access(box->href));
	ఒకవేళ (box->లక్ష్యం)
		fprintf(ప్రవాహం, " |%s|", box->లక్ష్యం);
	ఒకవేళ (box->title)
		fprintf(ప్రవాహం, " [%s]", box->title);
	ఒకవేళ (box->id)
		fprintf(ప్రవాహం, " ID:%s", lwc_string_దత్తాంశం(box->id));
	ఒకవేళ (box->type == BOX_INLINE || box->type == BOX_INLINE_చివర)
		fprintf(ప్రవాహం, " inline_end %p", box->inline_చివర);
	ఒకవేళ (box->float_children)
		fprintf(ప్రవాహం, " float_children %p", box->float_children);
	ఒకవేళ (box->తదుపరి_float)
		fprintf(ప్రవాహం, " next_float %p", box->తదుపరి_float);
	ఒకవేళ (box->float_container)
		fprintf(ప్రవాహం, " float_container %p", box->float_container);
	ఒకవేళ (box->నిలువు) {
		fprintf(ప్రవాహం, " (columns");
		కొరకు (i = 0; i != box->columns; i++) {
			fprintf(ప్రవాహం, " (%s %s %i %i %i)",
				((స్థిరరాశి అక్షరం *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->నిలువు[i].type],
				((స్థిరరాశి అక్షరం *[]) {
					"normal",
					"positioned"})
				[box->నిలువు[i].positioned],
				box->నిలువు[i].వెడల్పు,
				box->నిలువు[i].min, box->నిలువు[i].max);
		}
		fprintf(ప్రవాహం, ")");
	}
	ఒకవేళ (box->కణుపు != ఖాళీ) {
		dom_string *name;
		ఒకవేళ (dom_కణుపు_get_కణుపు_name(box->కణుపు, &name) == DOM_NO_ERR) {
			fprintf(ప్రవాహం, " <%s>", dom_string_దత్తాంశం(name));
			dom_string_unref(name);
		}
	}
	fprintf(ప్రవాహం, "\n");

	ఒకవేళ (box->list_marker) {
		కొరకు (i = 0; i != లోతు; i++)
			fprintf(ప్రవాహం, "  ");
		fprintf(ప్రవాహం, "list_marker:\n");
		box_dump(ప్రవాహం, box->list_marker, లోతు + 1, style);
	}

	కొరకు (c = box->children; c && c->తదుపరి; c = c->తదుపరి)
		;
	ఒకవేళ (box->చివరి != c)
		fprintf(ప్రవాహం, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->చివరి, c, box);
	కొరకు (prev = 0, c = box->children; c; prev = c, c = c->తదుపరి) {
		ఒకవేళ (c->మాతృ != box)
			fprintf(ప్రవాహం, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->మాతృ, box);
		ఒకవేళ (c->prev != prev)
			fprintf(ప్రవాహం, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(ప్రవాహం, c, లోతు + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(స్థిరరాశి నిర్మాణం box * స్థిరరాశి box)
{
	తిరిగి box->padding[శిఖరం_2] +
		box->ఎత్తు +
		box->padding[BOTTOM] +
		box->border[BOTTOM].వెడల్పు < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(స్థిరరాశి నిర్మాణం box * స్థిరరాశి box)
{
	తిరిగి box->padding[ఎడమ_2] +
		box->వెడల్పు +
		box->padding[కుడి_2] +
		box->border[కుడి_2].వెడల్పు < box->descendant_x1;
}


/* Exported function documented in html/box.h */
నిర్మాణం box *
box_pick_పాఠ్యం_box(నిర్మాణం html_content *html,
		  పూర్ణాంకం x, పూర్ణాంకం y,
		  పూర్ణాంకం dir,
		  పూర్ణాంకం *dx, పూర్ణాంకం *dy)
{
	నిర్మాణం box *పాఠ్యం_box = ఖాళీ;
	నిర్మాణం box *box;
	పూర్ణాంకం nr_xd, nr_yd;
	పూర్ణాంకం bx, by;
	పూర్ణాంకం fx, fy;
	పూర్ణాంకం tx, ty;

	ఒకవేళ (html == ఖాళీ)
		తిరిగి ఖాళీ;

	box = html->layout;
	bx = box->margin[ఎడమ_2];
	by = box->margin[శిఖరం_2];
	fx = bx;
	fy = by;

	ఒకవేళ (!box_సమీప_పాఠ్యం_box(box, bx, by, fx, fy, x, y,
				  dir, &పాఠ్యం_box, &tx, &ty, &nr_xd, &nr_yd)) {
		ఒకవేళ (పాఠ్యం_box && పాఠ్యం_box->పాఠ్యం && !పాఠ్యం_box->object) {
			పూర్ణాంకం w = (పాఠ్యం_box->padding[ఎడమ_2] +
				 పాఠ్యం_box->వెడల్పు +
				 పాఠ్యం_box->padding[కుడి_2]);
			పూర్ణాంకం h = (పాఠ్యం_box->padding[శిఖరం_2] +
				 పాఠ్యం_box->ఎత్తు +
				 పాఠ్యం_box->padding[BOTTOM]);
			పూర్ణాంకం x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			ఒకవేళ (x < tx) x = tx;
			ఒకవేళ (y < ty) y = ty;
			ఒకవేళ (y > y1) y = y1;
			ఒకవేళ (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	తిరిగి పాఠ్యం_box;
}
