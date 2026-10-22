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

#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <libcss/libcss.h>

#অন্তর্ভাব "content/handlers/css/utils.h"

কাঠামো content;
কাঠামো box;
কাঠামো browser_window;
কাঠামো html_content;
কাঠামো nsurl;
কাঠামো dom_গ্রন্থি;
কাঠামো dom_string;
কাঠামো rect;

#define UNKNOWN_WIDTH INT_MAX
#define UNKNOWN_MAX_WIDTH INT_MAX


প্রকারলেখ শূন্য (*box_construct_complete_cb)(কাঠামো html_content *c, bool success);


/**
 * Type of a struct box.
 */
প্রকারলেখ গণনা {
	BOX_BLOCK,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_TABLE,
	BOX_TABLE_ROW,
	BOX_TABLE_CELL,
	BOX_TABLE_ROW_GROUP,
	BOX_FLOAT_বাম,
	BOX_FLOAT_ডান,
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
প্রকারলেখ গণনা {
	NEW_সারি    = 1 << 0,	/* first inline on a new line */
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
গণনা box_side { শীর্ষ_2, ডান_2, BOTTOM, বাম_2 };


/**
 * Container for box border details
 */
কাঠামো box_border {
	গণনা css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	পূর্ণসংখ্যা width;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
কাঠামো column {
	/**
	 * Type of column.
	 */
	গণনা {
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
	পূর্ণসংখ্যা width;

	/**
	 * Minimum width of content.
	 */
	পূর্ণসংখ্যা min;

	/**
	 * Maximum width of content.
	 */
	পূর্ণসংখ্যা max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
কাঠামো object_param {
	অক্ষর *name;
	অক্ষর *মান;
	অক্ষর *type;
	অক্ষর *valuetype;
	কাঠামো object_param *পরবর্তী;
};


/**
 * Parameters for object element and similar elements.
 */
কাঠামো object_params {
	কাঠামো nsurl *তথ্য;
	অক্ষর *type;
	অক্ষর *codetype;
	কাঠামো nsurl *codebase;
	কাঠামো nsurl *classid;
	কাঠামো object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
কাঠামো box {
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
	কাঠামো dom_গ্রন্থি *গ্রন্থি;

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
	কাঠামো box *পরবর্তী;

	/**
	 * Previous sibling box, or NULL.
	 */
	কাঠামো box *পূর্ব;

	/**
	 * First child box, or NULL.
	 */
	কাঠামো box *children;

	/**
	 * Last child box, or NULL.
	 */
	কাঠামো box *last;

	/**
	 * Parent box, or NULL.
	 */
	কাঠামো box *parent;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	কাঠামো box *inline_end;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	কাঠামো box *float_children;

	/**
	 * Next sibling float box.
	 */
	কাঠামো box *পরবর্তী_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	কাঠামো box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	পূর্ণসংখ্যা clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	পূর্ণসংখ্যা cached_place_below_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	পূর্ণসংখ্যা x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	পূর্ণসংখ্যা y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	পূর্ণসংখ্যা width;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	পূর্ণসংখ্যা height;

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
	পূর্ণসংখ্যা descendant_x0;  /**< left edge of descendants */
	পূর্ণসংখ্যা descendant_y0;  /**< top edge of descendants */
	পূর্ণসংখ্যা descendant_x1;  /**< right edge of descendants */
	পূর্ণসংখ্যা descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	পূর্ণসংখ্যা margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	পূর্ণসংখ্যা padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	কাঠামো box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	কাঠামো scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	কাঠামো scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	পূর্ণসংখ্যা min_width;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	পূর্ণসংখ্যা max_width;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	অক্ষর *text;

	/**
	 * Length of text.
	 */
	size_t দৈর্ঘ্য;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	পূর্ণসংখ্যা space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_offset;


	/**
	 * Link, or NULL.
	 */
	কাঠামো nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ধ্রুবক অক্ষর *target;

	/**
	 * Title, or NULL.
	 */
	ধ্রুবক অক্ষর *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	unsigned পূর্ণসংখ্যা columns;

	/**
	 * Number of rows for TABLE only.
	 */
	unsigned পূর্ণসংখ্যা rows;

	/**
	 * Start column for TABLE_CELL only.
	 */
	unsigned পূর্ণসংখ্যা start_column;

	/**
	 * Array of table column data for TABLE only.
	 */
	কাঠামো column *col;

	/**
	 * List item value.
	 */
	পূর্ণসংখ্যা list_মান;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	কাঠামো box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	কাঠামো form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	অক্ষর *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	কাঠামো hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	কাঠামো hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	কাঠামো object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	কাঠামো browser_window *iframe;

};


#endif
