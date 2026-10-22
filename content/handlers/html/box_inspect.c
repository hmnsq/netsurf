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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/utils.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "utils/errors.h"
#અન્તર્ભાવ "netsurf/types.h"
#અન્તર્ભાવ "netsurf/content.h"
#અન્તર્ભાવ "netsurf/mouse.h"
#અન્તર્ભાવ "css/utils.h"
#અન્તર્ભાવ "css/dump.h"
#અન્તર્ભાવ "desktop/scrollbar.h"

#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_inspect.h"

/**
 * Direction to move in a box-tree walk
 */
ગણના box_walk_dir {
		   BOX_WALK_CHILDREN,
		   BOX_WALK_PARENT,
		   BOX_WALK_NEXT_SIBLING,
		   BOX_WALK_FLOAT_CHILDREN,
		   BOX_WALK_NEXT_FLOAT_SIBLING,
		   BOX_WALK_FLOAT_CONTAINER
};

#define box_is_float(box) (box->type == BOX_FLOAT_ડાબું ||	\
			   box->type == BOX_FLOAT_જમણું)

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
સ્થિર bool
box_contains_point(અચલ css_unit_ctx *unit_len_ctx,
		   અચલ સંરચના box *box,
		   પૂર્ણાંક x,
		   પૂર્ણાંક y,
		   bool *physically)
{
	css_computed_clip_rect css_rect;

	જો (box->style != ખાલી &&
	    css_computed_position(box->style) == CSS_POSITION_ABSOLUTE &&
	    css_computed_clip(box->style, &css_rect) == CSS_CLIP_RECT) {
		/* We have an absolutly positioned box with a clip rect */
		સંરચના rect r = {
				 .x0 = box->border[ડાબું_2].width,
				 .y0 = box->border[ટોચ_2].width,
				 .x1 = box->padding[ડાબું_2] + box->width +
				 box->border[જમણું_2].width +
				 box->padding[જમણું_2],
				 .y1 = box->padding[ટોચ_2] + box->height +
				 box->border[BOTTOM].width +
				 box->padding[BOTTOM]
		};
		જો (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			*physically = true;
		} નહીંતર {
			*physically = false;
		}

		/* Adjust rect to css clip region */
		જો (css_rect.ડાબું_auto == false) {
			r.x0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ડાબું,
						css_rect.lunit));
		}
		જો (css_rect.ટોચ_auto == false) {
			r.y0 += FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.ટોચ,
						css_rect.tunit));
		}
		જો (css_rect.જમણું_auto == false) {
			r.x1 = box->border[ડાબું_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.જમણું,
						css_rect.runit));
		}
		જો (css_rect.bottom_auto == false) {
			r.y1 = box->border[ટોચ_2].width +
				FIXTOINT(css_unit_len2device_px(
						box->style,
						unit_len_ctx,
						css_rect.bottom,
						css_rect.bunit));
		}

		/* Test if point is in clipped box */
		જો (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1) {
			/* inside clip area */
			પાછા true;
		}

		/* Not inside clip area */
		પાછા false;
	}
	જો (x >= -box->border[ડાબું_2].width &&
	    x < box->padding[ડાબું_2] + box->width +
	    box->padding[જમણું_2] + box->border[જમણું_2].width &&
	    y >= -box->border[ટોચ_2].width &&
	    y < box->padding[ટોચ_2] + box->height +
	    box->padding[BOTTOM] + box->border[BOTTOM].width) {
		*physically = true;
		પાછા true;
	}
	જો (box->list_marker && box->list_marker->x - box->x <= x +
	    box->list_marker->border[ડાબું_2].width &&
	    x < box->list_marker->x - box->x +
	    box->list_marker->padding[ડાબું_2] +
	    box->list_marker->width +
	    box->list_marker->border[જમણું_2].width +
	    box->list_marker->padding[જમણું_2] &&
	    box->list_marker->y - box->y <= y +
	    box->list_marker->border[ટોચ_2].width &&
	    y < box->list_marker->y - box->y +
	    box->list_marker->padding[ટોચ_2] +
	    box->list_marker->height +
	    box->list_marker->border[BOTTOM].width +
	    box->list_marker->padding[BOTTOM]) {
		*physically = true;
		પાછા true;
	}
	જો ((box->style && css_computed_overflow_x(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		જો (box->descendant_x0 <= x &&
		    x < box->descendant_x1) {
			*physically = false;
			પાછા true;
		}
	}
	જો ((box->style && css_computed_overflow_y(box->style) ==
	     CSS_OVERFLOW_VISIBLE) || !box->style) {
		જો (box->descendant_y0 <= y &&
		    y < box->descendant_y1) {
			*physically = false;
			પાછા true;
		}
	}
	પાછા false;
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
સ્થિર અંતઃરેખ સંરચના box *
box_move_xy(સંરચના box *b, ગણના box_walk_dir dir, પૂર્ણાંક *x, પૂર્ણાંક *y)
{
	સંરચના box *rb = ખાલી;

	પસંદગી (dir) {
	સ્થિતિ BOX_WALK_CHILDREN:
		b = b->children;
		જો (b == ખાલી)
			તોડો;
		*x += b->x;
		*y += b->y;
		જો (!box_is_float(b)) {
			rb = b;
			તોડો;
		}
		fallthrough;

	સ્થિતિ BOX_WALK_NEXT_SIBLING:
		કરો {
			*x -= b->x;
			*y -= b->y;
			b = b->next;
			જો (b == ખાલી)
				તોડો;
			*x += b->x;
			*y += b->y;
		} જ્યારે (box_is_float(b));
		rb = b;
		તોડો;

	સ્થિતિ BOX_WALK_PARENT:
		*x -= b->x;
		*y -= b->y;
		rb = b->parent;
		તોડો;

	સ્થિતિ BOX_WALK_FLOAT_CHILDREN:
		b = b->float_children;
		જો (b == ખાલી)
			તોડો;
		*x += b->x;
		*y += b->y;
		rb = b;
		તોડો;

	સ્થિતિ BOX_WALK_NEXT_FLOAT_SIBLING:
		*x -= b->x;
		*y -= b->y;
		b = b->next_float;
		જો (b == ખાલી)
			તોડો;
		*x += b->x;
		*y += b->y;
		rb = b;
		તોડો;

	સ્થિતિ BOX_WALK_FLOAT_CONTAINER:
		*x -= b->x;
		*y -= b->y;
		rb = b->float_container;
		તોડો;

	મૂળભૂત:
		પ્રતિજ્ઞા(0 && "Bad box walk type.");
	}

	પાછા rb;
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
સ્થિર અંતઃરેખ સંરચના box *
box_next_xy(સંરચના box *b, પૂર્ણાંક *x, પૂર્ણાંક *y, bool skip_children)
{
	સંરચના box *n;
	પૂર્ણાંક tx, ty;

	પ્રતિજ્ઞા(b != ખાલી);

	જો (skip_children) {
		/* Caller is not interested in any kind of children */
		જાઓ skip_children;
	}

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_FLOAT_CHILDREN, &tx, &ty);
	જો (n) {
		/* Next node is float child */
		*x = tx;
		*y = ty;
		પાછા n;
	}
 done_float_children:

	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_CHILDREN, &tx, &ty);
	જો (n) {
		/* Next node is child */
		*x = tx;
		*y = ty;
		પાછા n;
	}

 skip_children:
	tx = *x; ty = *y;
	n = box_move_xy(b, BOX_WALK_NEXT_FLOAT_SIBLING, &tx, &ty);
	જો (n) {
		/* Go to next float sibling */
		*x = tx;
		*y = ty;
		પાછા n;
	}

	જો (box_is_float(b)) {
		/* Done floats, but the float container may have children,
		 * or siblings, or ansestors with siblings.  Change to
		 * float container and move past handling its float children.
		 */
		b = box_move_xy(b, BOX_WALK_FLOAT_CONTAINER, x, y);
		જાઓ done_float_children;
	}

	/* Go to next sibling, or nearest ancestor with next sibling. */
	જ્યારે (b) {
		જ્યારે (!b->next && b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			જો (box_is_float(b)) {
				/* Go on to next float, if there is one */
				જાઓ skip_children;
			}
		}
		જો (!b->next) {
			/* No more boxes */
			પાછા ખાલી;
		}

		tx = *x; ty = *y;
		n = box_move_xy(b, BOX_WALK_NEXT_SIBLING, &tx, &ty);
		જો (n) {
			/* Go to non-float (ancestor) sibling */
			*x = tx;
			*y = ty;
			પાછા n;

		} નહીંતર જો (b->parent) {
			b = box_move_xy(b, BOX_WALK_PARENT, x, y);
			જો (box_is_float(b)) {
				/* Go on to next float, if there is one */
				જાઓ skip_children;
			}

		} નહીંતર {
			/* No more boxes */
			પાછા ખાલી;
		}
	}

	પ્રતિજ્ઞા(b != ખાલી);
	પાછા ખાલી;
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
સ્થિર bool
box_nearer_text_box(સંરચના box *box,
		    પૂર્ણાંક bx, પૂર્ણાંક by,
		    પૂર્ણાંક x, પૂર્ણાંક y,
		    પૂર્ણાંક dir,
		    સંરચના box **nearest,
		    પૂર્ણાંક *tx, પૂર્ણાંક *ty,
		    પૂર્ણાંક *nr_xd, પૂર્ણાંક *nr_yd)
{
	પૂર્ણાંક w = box->padding[ડાબું_2] + box->width + box->padding[જમણું_2];
	પૂર્ણાંક h = box->padding[ટોચ_2] + box->height + box->padding[BOTTOM];
	પૂર્ણાંક y1 = by + h;
	પૂર્ણાંક x1 = bx + w;
	પૂર્ણાંક yd = INT_MAX;
	પૂર્ણાંક xd = INT_MAX;

	જો (x >= bx && x1 > x && y >= by && y1 > y) {
		*nearest = box;
		*tx = bx;
		*ty = by;
		પાછા true;
	}

	જો (box->parent->list_marker != box) {
		જો (dir < 0) {
			/* consider only those children (partly) above-left */
			જો (by <= y && bx < x) {
				yd = y <= y1 ? 0 : y - y1;
				xd = x <= x1 ? 0 : x - x1;
			}
		} નહીંતર {
			/* consider only those children (partly) below-right */
			જો (y1 > y && x1 > x) {
				yd = y > by ? 0 : by - y;
				xd = x > bx ? 0 : bx - x;
			}
		}

		/* give y displacement precedence over x */
		જો (yd < *nr_yd || (yd == *nr_yd && xd <= *nr_xd)) {
			*nr_yd = yd;
			*nr_xd = xd;
			*nearest = box;
			*tx = bx;
			*ty = by;
		}
	}
	પાછા false;
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
સ્થિર bool
box_nearest_text_box(સંરચના box *box,
		     પૂર્ણાંક bx, પૂર્ણાંક by,
		     પૂર્ણાંક fx, પૂર્ણાંક fy,
		     પૂર્ણાંક x, પૂર્ણાંક y,
		     પૂર્ણાંક dir,
		     સંરચના box **nearest,
		     પૂર્ણાંક *tx, પૂર્ણાંક *ty,
		     પૂર્ણાંક *nr_xd, પૂર્ણાંક *nr_yd)
{
	સંરચના box *child = box->children;
	પૂર્ણાંક c_bx, c_by;
	પૂર્ણાંક c_fx, c_fy;
	bool in_box = false;

	જો (*nearest == ખાલી) {
		*nr_xd = INT_MAX / 2; /* displacement of 'nearest so far' */
		*nr_yd = INT_MAX / 2;
	}
	જો (box->type == BOX_INLINE_CONTAINER) {
		પૂર્ણાંક bw = box->padding[ડાબું_2] + box->width + box->padding[જમણું_2];
		પૂર્ણાંક bh = box->padding[ટોચ_2] + box->height + box->padding[BOTTOM];
		પૂર્ણાંક b_y1 = by + bh;
		પૂર્ણાંક b_x1 = bx + bw;
		જો (x >= bx && b_x1 > x && y >= by && b_y1 > y) {
			in_box = true;
		}
	}

	જ્યારે (child) {
		જો (child->type == BOX_FLOAT_ડાબું ||
		    child->type == BOX_FLOAT_જમણું) {
			c_bx = fx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = fy + child->y -
				scrollbar_get_offset(child->scroll_y);
		} નહીંતર {
			c_bx = bx + child->x -
				scrollbar_get_offset(child->scroll_x);
			c_by = by + child->y -
				scrollbar_get_offset(child->scroll_y);
		}
		જો (child->float_children) {
			c_fx = c_bx;
			c_fy = c_by;
		} નહીંતર {
			c_fx = fx;
			c_fy = fy;
		}
		જો (in_box && child->text && !child->object) {
			જો (box_nearer_text_box(child,
						c_bx, c_by, x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
				પાછા true;
		} નહીંતર {
			જો (child->list_marker) {
				જો (box_nearer_text_box(
						child->list_marker,
						c_bx + child->list_marker->x,
						c_by + child->list_marker->y,
						x, y, dir, nearest,
						tx, ty, nr_xd, nr_yd))
					પાછા true;
			}
			જો (box_nearest_text_box(child, c_bx, c_by,
						 c_fx, c_fy,
						 x, y, dir, nearest, tx, ty,
						 nr_xd, nr_yd))
				પાછા true;
		}
		child = child->next;
	}

	પાછા false;
}


/* Exported function documented in html/box.h */
શૂન્ય box_coords(સંરચના box *box, પૂર્ણાંક *x, પૂર્ણાંક *y)
{
	*x = box->x;
	*y = box->y;
	જ્યારે (box->parent) {
		જો (box_is_float(box)) {
			પ્રતિજ્ઞા(box->float_container);
			box = box->float_container;
		} નહીંતર {
			box = box->parent;
		}
		*x += box->x - scrollbar_get_offset(box->scroll_x);
		*y += box->y - scrollbar_get_offset(box->scroll_y);
	}
}


/* Exported function documented in html/box.h */
શૂન્ય box_bounds(સંરચના box *box, સંરચના rect *r)
{
	પૂર્ણાંક width, height;

	box_coords(box, &r->x0, &r->y0);

	width = box->padding[ડાબું_2] + box->width + box->padding[જમણું_2];
	height = box->padding[ટોચ_2] + box->height + box->padding[BOTTOM];

	r->x1 = r->x0 + width;
	r->y1 = r->y0 + height;
}


/* Exported function documented in html/box.h */
સંરચના box *
box_at_point(અચલ css_unit_ctx *unit_len_ctx,
	     સંરચના box *box,
	     અચલ પૂર્ણાંક x, અચલ પૂર્ણાંક y,
	     પૂર્ણાંક *box_x, પૂર્ણાંક *box_y)
{
	bool skip_children;
	bool physically;

	પ્રતિજ્ઞા(box);

	skip_children = false;
	જ્યારે ((box = box_next_xy(box, box_x, box_y, skip_children))) {
		જો (box_contains_point(unit_len_ctx, box, x - *box_x, y - *box_y,
				       &physically)) {
			*box_x -= scrollbar_get_offset(box->scroll_x);
			*box_y -= scrollbar_get_offset(box->scroll_y);

			જો (physically)
				પાછા box;

			skip_children = false;
		} નહીંતર {
			skip_children = true;
		}
	}

	પાછા ખાલી;
}


/* Exported function documented in html/box.h */
સંરચના box *box_find_by_id(સંરચના box *box, lwc_string *id)
{
	સંરચના box *a, *b;
	bool m;

	જો (box->id != ખાલી &&
	    lwc_string_isequal(id, box->id, &m) == lwc_error_ok &&
	    m == true) {
		પાછા box;
	}

	માટે (a = box->children; a; a = a->next) {
		જો ((b = box_find_by_id(a, id)) != ખાલી) {
			પાછા b;
		}
	}

	પાછા ખાલી;
}


/* Exported function documented in html/box.h */
bool box_visible(સંરચના box *box)
{
	/* visibility: hidden */
	જો (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		પાછા false;
	}

	પાછા true;
}


/* Exported function documented in html/box.h */
શૂન્ય box_dump(FILE *stream, સંરચના box *box, unsigned પૂર્ણાંક depth, bool style)
{
	unsigned પૂર્ણાંક i;
	સંરચના box *c, *prev;

	માટે (i = 0; i != depth; i++) {
		fprintf(stream, "  ");
	}

	fprintf(stream, "%p ", box);
	fprintf(stream, "x%i y%i w%i h%i ",
		box->x, box->y, box->width, box->height);
	જો (box->max_width != UNKNOWN_MAX_WIDTH) {
		fprintf(stream, "min%i max%i ", box->min_width, box->max_width);
	}
	fprintf(stream, "desc(%i %i %i %i) ",
		box->descendant_x0, box->descendant_y0,
		box->descendant_x1, box->descendant_y1);

	fprintf(stream, "m(%i %i %i %i) ",
		box->margin[ટોચ_2], box->margin[ડાબું_2],
		box->margin[BOTTOM], box->margin[જમણું_2]);

	પસંદગી (box->type) {
	સ્થિતિ BOX_BLOCK:
		fprintf(stream, "BLOCK ");
		તોડો;

	સ્થિતિ BOX_INLINE_CONTAINER:
		fprintf(stream, "INLINE_CONTAINER ");
		તોડો;

	સ્થિતિ BOX_INLINE:
		fprintf(stream, "INLINE ");
		તોડો;

	સ્થિતિ BOX_INLINE_END:
		fprintf(stream, "INLINE_END ");
		તોડો;

	સ્થિતિ BOX_INLINE_BLOCK:
		fprintf(stream, "INLINE_BLOCK ");
		તોડો;

	સ્થિતિ BOX_TABLE:
		fprintf(stream, "TABLE [columns %i] ", box->columns);
		તોડો;

	સ્થિતિ BOX_TABLE_ROW:
		fprintf(stream, "TABLE_ROW ");
		તોડો;

	સ્થિતિ BOX_TABLE_CELL:
		fprintf(stream, "TABLE_CELL [columns %i, start %i, rows %i] ",
			box->columns,
			box->start_column,
			box->rows);
		તોડો;

	સ્થિતિ BOX_TABLE_ROW_GROUP:
		fprintf(stream, "TABLE_ROW_GROUP ");
		તોડો;

	સ્થિતિ BOX_FLOAT_ડાબું:
		fprintf(stream, "FLOAT_LEFT ");
		તોડો;

	સ્થિતિ BOX_FLOAT_જમણું:
		fprintf(stream, "FLOAT_RIGHT ");
		તોડો;

	સ્થિતિ BOX_BR:
		fprintf(stream, "BR ");
		તોડો;

	સ્થિતિ BOX_TEXT:
		fprintf(stream, "TEXT ");
		તોડો;

	સ્થિતિ BOX_FLEX:
		fprintf(stream, "FLEX ");
		તોડો;

	સ્થિતિ BOX_INLINE_FLEX:
		fprintf(stream, "INLINE_FLEX ");
		તોડો;

	મૂળભૂત:
		fprintf(stream, "Unknown box type ");
	}

	જો (box->text)
		fprintf(stream, "%li '%.*s' ", (unsigned long) box->byte_offset,
			(પૂર્ણાંક) box->length, box->text);
	જો (box->space)
		fprintf(stream, "space ");
	જો (box->object) {
		fprintf(stream, "(object '%s') ",
			nsurl_access(hlcache_handle_get_url(box->object)));
	}
	જો (box->iframe) {
		fprintf(stream, "(iframe) ");
	}
	જો (box->gadget)
		fprintf(stream, "(gadget) ");
	જો (style && box->style)
		nscss_dump_computed_style(stream, box->style);
	જો (box->href)
		fprintf(stream, " -> '%s'", nsurl_access(box->href));
	જો (box->target)
		fprintf(stream, " |%s|", box->target);
	જો (box->title)
		fprintf(stream, " [%s]", box->title);
	જો (box->id)
		fprintf(stream, " ID:%s", lwc_string_data(box->id));
	જો (box->type == BOX_INLINE || box->type == BOX_INLINE_END)
		fprintf(stream, " inline_end %p", box->inline_end);
	જો (box->float_children)
		fprintf(stream, " float_children %p", box->float_children);
	જો (box->next_float)
		fprintf(stream, " next_float %p", box->next_float);
	જો (box->float_container)
		fprintf(stream, " float_container %p", box->float_container);
	જો (box->col) {
		fprintf(stream, " (columns");
		માટે (i = 0; i != box->columns; i++) {
			fprintf(stream, " (%s %s %i %i %i)",
				((અચલ અક્ષર *[]) {
					"UNKNOWN",
					"FIXED",
					"AUTO",
					"PERCENT",
					"RELATIVE"
						})
				[box->col[i].type],
				((અચલ અક્ષર *[]) {
					"normal",
					"positioned"})
				[box->col[i].positioned],
				box->col[i].width,
				box->col[i].min, box->col[i].max);
		}
		fprintf(stream, ")");
	}
	જો (box->ગાંઠ != ખાલી) {
		dom_string *name;
		જો (dom_ગાંઠ_get_ગાંઠ_name(box->ગાંઠ, &name) == DOM_NO_ERR) {
			fprintf(stream, " <%s>", dom_string_data(name));
			dom_string_unref(name);
		}
	}
	fprintf(stream, "\n");

	જો (box->list_marker) {
		માટે (i = 0; i != depth; i++)
			fprintf(stream, "  ");
		fprintf(stream, "list_marker:\n");
		box_dump(stream, box->list_marker, depth + 1, style);
	}

	માટે (c = box->children; c && c->next; c = c->next)
		;
	જો (box->last != c)
		fprintf(stream, "warning: box->last %p (should be %p) "
			"(box %p)\n", box->last, c, box);
	માટે (prev = 0, c = box->children; c; prev = c, c = c->next) {
		જો (c->parent != box)
			fprintf(stream, "warning: box->parent %p (should be "
				"%p) (box on next line)\n",
				c->parent, box);
		જો (c->prev != prev)
			fprintf(stream, "warning: box->prev %p (should be "
				"%p) (box on next line)\n",
				c->prev, prev);
		box_dump(stream, c, depth + 1, style);
	}
}


/* exported interface documented in html/box.h */
bool box_vscrollbar_present(અચલ સંરચના box * અચલ box)
{
	પાછા box->padding[ટોચ_2] +
		box->height +
		box->padding[BOTTOM] +
		box->border[BOTTOM].width < box->descendant_y1;
}


/* exported interface documented in html/box.h */
bool box_hscrollbar_present(અચલ સંરચના box * અચલ box)
{
	પાછા box->padding[ડાબું_2] +
		box->width +
		box->padding[જમણું_2] +
		box->border[જમણું_2].width < box->descendant_x1;
}


/* Exported function documented in html/box.h */
સંરચના box *
box_pick_text_box(સંરચના html_content *html,
		  પૂર્ણાંક x, પૂર્ણાંક y,
		  પૂર્ણાંક dir,
		  પૂર્ણાંક *dx, પૂર્ણાંક *dy)
{
	સંરચના box *text_box = ખાલી;
	સંરચના box *box;
	પૂર્ણાંક nr_xd, nr_yd;
	પૂર્ણાંક bx, by;
	પૂર્ણાંક fx, fy;
	પૂર્ણાંક tx, ty;

	જો (html == ખાલી)
		પાછા ખાલી;

	box = html->layout;
	bx = box->margin[ડાબું_2];
	by = box->margin[ટોચ_2];
	fx = bx;
	fy = by;

	જો (!box_nearest_text_box(box, bx, by, fx, fy, x, y,
				  dir, &text_box, &tx, &ty, &nr_xd, &nr_yd)) {
		જો (text_box && text_box->text && !text_box->object) {
			પૂર્ણાંક w = (text_box->padding[ડાબું_2] +
				 text_box->width +
				 text_box->padding[જમણું_2]);
			પૂર્ણાંક h = (text_box->padding[ટોચ_2] +
				 text_box->height +
				 text_box->padding[BOTTOM]);
			પૂર્ણાંક x1, y1;

			y1 = ty + h;
			x1 = tx + w;

			/* ensure point lies within the text box */
			જો (x < tx) x = tx;
			જો (y < ty) y = ty;
			જો (y > y1) y = y1;
			જો (x > x1) x = x1;
		}
	}

	/* return coordinates relative to box */
	*dx = x - tx;
	*dy = y - ty;

	પાછા text_box;
}
