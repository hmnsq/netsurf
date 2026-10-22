/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
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
 * Box interface.
 *
 */

#ifndef NETSURF_HTML_BOX_H
#define NETSURF_HTML_BOX_H

#شامل <limits.h>
#شامل <stdbool.h>
#شامل <libcss/libcss.h>

#شامل "content/handlers/css/utils.h"

ساختار content;
ساختار box;
ساختار browser_window;
ساختار html_content;
ساختار nsurl;
ساختار dom_گره;
ساختار dom_string;
ساختار rect;

#define UNKNOWN_WIDTH INT_MAX
#define UNKNOWN_MAX_WIDTH INT_MAX


تعریفنوع تهی (*box_construct_complete_cb)(ساختار html_content *c, bool success);


/**
 * Type of a struct box.
 */
تعریفنوع شمارش {
	BOX_BLOCK,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_TABLE,
	BOX_TABLE_ROW,
	BOX_TABLE_CELL,
	BOX_TABLE_ROW_GROUP,
	BOX_FLOAT_چپ,
	BOX_FLOAT_راست,
	BOX_INLINE_BLOCK,
	BOX_BR,
	BOX_TEXT,
	BOX_INLINE_END,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
تعریفنوع شمارش {
	NEW_سطر    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_HEIGHT  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_HEIGHT = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
شمارش box_side { بالاترین_2, راست_2, BOTTOM, چپ_2 };


/**
 * Container for box border details
 */
ساختار box_border {
	شمارش css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	صحیح width;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
ساختار column {
	/**
	 * Type of column.
	 */
	شمارش {
	      COLUMN_WIDTH_UNKNOWN,
	      COLUMN_WIDTH_FIXED,
	      COLUMN_WIDTH_AUTO,
	      COLUMN_WIDTH_PERCENT,
	      COLUMN_WIDTH_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	صحیح width;

	/**
	 * Minimum width of content.
	 */
	صحیح min;

	/**
	 * Maximum width of content.
	 */
	صحیح max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
ساختار object_param {
	نویسه *name;
	نویسه *مقدار;
	نویسه *type;
	نویسه *valuetype;
	ساختار object_param *next;
};


/**
 * Parameters for object element and similar elements.
 */
ساختار object_params {
	ساختار nsurl *data;
	نویسه *type;
	نویسه *codetype;
	ساختار nsurl *codebase;
	ساختار nsurl *classid;
	ساختار object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
ساختار box {
	/**
	 * Type of box.
	 */
	box_type type;

	/**
	 * Box flags
	 */
	box_flags flags;

	/**
	 * DOM node that generated this box or NULL
	 */
	ساختار dom_گره *گره;

	/**
	 * Computed styles for elements and their pseudo elements.
	 *  NULL on non-element boxes.
	 */
	css_select_results *styles;

	/**
	 * Style for this box. 0 for INLINE_CONTAINER and
	 *  FLOAT_*. Pointer into a box's 'styles' select results,
	 *  except for implied boxes, where it is a pointer to an
	 *  owned computed style.
	 */
	css_computed_style *style;

	/**
	 *  value of id attribute (or name for anchors)
	 */
	lwc_string *id;


	/**
	 * Next sibling box, or NULL.
	 */
	ساختار box *next;

	/**
	 * Previous sibling box, or NULL.
	 */
	ساختار box *prev;

	/**
	 * First child box, or NULL.
	 */
	ساختار box *children;

	/**
	 * Last child box, or NULL.
	 */
	ساختار box *last;

	/**
	 * Parent box, or NULL.
	 */
	ساختار box *parent;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	ساختار box *inline_end;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	ساختار box *float_children;

	/**
	 * Next sibling float box.
	 */
	ساختار box *next_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	ساختار box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	صحیح clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	صحیح cached_place_below_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	صحیح x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	صحیح y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	صحیح width;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	صحیح height;

	/* These four variables determine the maximum extent of a box's
	 * descendants. They are relative to the x,y coordinates of the box.
	 *
	 * Their use depends on the overflow CSS property:
	 *
	 * Overflow:	Usage:
	 * visible	The content of the box is displayed within these
	 *		dimensions.
	 * hidden	These are ignored. Content is plotted within the box
	 *		dimensions.
	 * scroll	These are used to determine the extent of the
	 *		scrollable area.
	 * auto		As "scroll".
	 */
	صحیح descendant_x0;  /**< left edge of descendants */
	صحیح descendant_y0;  /**< top edge of descendants */
	صحیح descendant_x1;  /**< right edge of descendants */
	صحیح descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	صحیح margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	صحیح padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ساختار box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	ساختار scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	ساختار scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	صحیح min_width;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	صحیح max_width;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	نویسه *text;

	/**
	 * Length of text.
	 */
	size_t length;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	صحیح space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_offset;


	/**
	 * Link, or NULL.
	 */
	ساختار nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ثابت نویسه *target;

	/**
	 * Title, or NULL.
	 */
	ثابت نویسه *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	unsigned صحیح columns;

	/**
	 * Number of rows for TABLE only.
	 */
	unsigned صحیح rows;

	/**
	 * Start column for TABLE_CELL only.
	 */
	unsigned صحیح start_column;

	/**
	 * Array of table column data for TABLE only.
	 */
	ساختار column *col;

	/**
	 * List item value.
	 */
	صحیح list_مقدار;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	ساختار box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	ساختار form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	نویسه *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	ساختار hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	ساختار hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	ساختار object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	ساختار browser_window *iframe;

};


#endif
