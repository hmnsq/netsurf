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

#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <libcss/libcss.h>

#अन्तर्भाव "content/handlers/css/utils.h"

संरचना content;
संरचना box;
संरचना browser_window;
संरचना html_content;
संरचना nsurl;
संरचना dom_ग्रन्थिः;
संरचना dom_string;
संरचना rect;

#define UNKNOWN_WIDTH INT_MAX
#define UNKNOWN_MAX_WIDTH INT_MAX


प्रकारनिर्देश शून्य (*box_construct_complete_cb)(संरचना html_content *c, bool success);


/**
 * Type of a struct box.
 */
प्रकारनिर्देश गणना {
	BOX_BLOCK,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_TABLE,
	BOX_TABLE_ROW,
	BOX_TABLE_CELL,
	BOX_TABLE_ROW_GROUP,
	BOX_FLOAT_वामः,
	BOX_FLOAT_दक्षिणः,
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
प्रकारनिर्देश गणना {
	NEW_पङ्क्तिः    = 1 << 0,	/* first inline on a new line */
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
गणना box_side { शिखरम्_2, दक्षिणः_2, BOTTOM, वामः_2 };


/**
 * Container for box border details
 */
संरचना box_border {
	गणना css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	पूर्णाङ्क width;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
संरचना column {
	/**
	 * Type of column.
	 */
	गणना {
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
	पूर्णाङ्क width;

	/**
	 * Minimum width of content.
	 */
	पूर्णाङ्क min;

	/**
	 * Maximum width of content.
	 */
	पूर्णाङ्क max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
संरचना object_param {
	वर्ण *name;
	वर्ण *मानम्;
	वर्ण *type;
	वर्ण *valuetype;
	संरचना object_param *next;
};


/**
 * Parameters for object element and similar elements.
 */
संरचना object_params {
	संरचना nsurl *data;
	वर्ण *type;
	वर्ण *codetype;
	संरचना nsurl *codebase;
	संरचना nsurl *classid;
	संरचना object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
संरचना box {
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
	संरचना dom_ग्रन्थिः *ग्रन्थिः;

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
	संरचना box *next;

	/**
	 * Previous sibling box, or NULL.
	 */
	संरचना box *prev;

	/**
	 * First child box, or NULL.
	 */
	संरचना box *children;

	/**
	 * Last child box, or NULL.
	 */
	संरचना box *last;

	/**
	 * Parent box, or NULL.
	 */
	संरचना box *parent;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	संरचना box *inline_end;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	संरचना box *float_children;

	/**
	 * Next sibling float box.
	 */
	संरचना box *next_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	संरचना box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	पूर्णाङ्क clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	पूर्णाङ्क cached_place_below_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	पूर्णाङ्क x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	पूर्णाङ्क y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	पूर्णाङ्क width;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	पूर्णाङ्क height;

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
	पूर्णाङ्क descendant_x0;  /**< left edge of descendants */
	पूर्णाङ्क descendant_y0;  /**< top edge of descendants */
	पूर्णाङ्क descendant_x1;  /**< right edge of descendants */
	पूर्णाङ्क descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	पूर्णाङ्क margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	पूर्णाङ्क padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	संरचना box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	संरचना scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	संरचना scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	पूर्णाङ्क min_width;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	पूर्णाङ्क max_width;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	वर्ण *text;

	/**
	 * Length of text.
	 */
	size_t length;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	पूर्णाङ्क space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_offset;


	/**
	 * Link, or NULL.
	 */
	संरचना nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ध्रुव वर्ण *target;

	/**
	 * Title, or NULL.
	 */
	ध्रुव वर्ण *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	unsigned पूर्णाङ्क columns;

	/**
	 * Number of rows for TABLE only.
	 */
	unsigned पूर्णाङ्क rows;

	/**
	 * Start column for TABLE_CELL only.
	 */
	unsigned पूर्णाङ्क start_column;

	/**
	 * Array of table column data for TABLE only.
	 */
	संरचना column *col;

	/**
	 * List item value.
	 */
	पूर्णाङ्क list_मानम्;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	संरचना box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	संरचना form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	वर्ण *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	संरचना hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	संरचना hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	संरचना object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	संरचना browser_window *iframe;

};


#endif
