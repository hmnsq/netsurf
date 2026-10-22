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

#अगर_अघोषित NETSURF_HTML_BOX_H
#घोषणा NETSURF_HTML_BOX_H

#समावेश <limits.h>
#समावेश <stdbool.h>
#समावेश <libcss/libcss.h>

#समावेश "content/handlers/css/utils.h"

काष्ठा content;
काष्ठा box;
काष्ठा browser_window;
काष्ठा html_content;
काष्ठा nsurl;
काष्ठा dom_गाँठ;
काष्ठा dom_string;
काष्ठा rect;

#घोषणा UNKNOWN_WIDTH पूर्णांक_उच्च
#घोषणा UNKNOWN_MAX_WIDTH पूर्णांक_उच्च


प्रकार व्योम (*box_construct_complete_cb)(काष्ठा html_content *c, bool success);


/**
 * Type of a struct box.
 */
प्रकार क्रमागत {
	BOX_BLOCK,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_TABLE,
	BOX_TABLE_ROW,
	BOX_TABLE_CELL,
	BOX_TABLE_ROW_GROUP,
	BOX_FLOAT_बायाँ,
	BOX_FLOAT_दायाँ,
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
प्रकार क्रमागत {
	NEW_पंक्ति    = 1 << 0,	/* first inline on a new line */
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
क्रमागत box_side { शीर्ष_2, दायाँ_2, BOTTOM, बायाँ_2 };


/**
 * Container for box border details
 */
काष्ठा box_border {
	क्रमागत css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	पूर्णांक width;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
काष्ठा column {
	/**
	 * Type of column.
	 */
	क्रमागत {
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
	पूर्णांक width;

	/**
	 * Minimum width of content.
	 */
	पूर्णांक min;

	/**
	 * Maximum width of content.
	 */
	पूर्णांक max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
काष्ठा object_param {
	अक्षर *name;
	अक्षर *मूल्य;
	अक्षर *type;
	अक्षर *valuetype;
	काष्ठा object_param *अग्र;
};


/**
 * Parameters for object element and similar elements.
 */
काष्ठा object_params {
	काष्ठा nsurl *तथ्य;
	अक्षर *type;
	अक्षर *codetype;
	काष्ठा nsurl *codebase;
	काष्ठा nsurl *classid;
	काष्ठा object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
काष्ठा box {
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
	काष्ठा dom_गाँठ *गाँठ;

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
	काष्ठा box *अग्र;

	/**
	 * Previous sibling box, or NULL.
	 */
	काष्ठा box *पूर्व;

	/**
	 * First child box, or NULL.
	 */
	काष्ठा box *children;

	/**
	 * Last child box, or NULL.
	 */
	काष्ठा box *last;

	/**
	 * Parent box, or NULL.
	 */
	काष्ठा box *parent;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	काष्ठा box *inline_end;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	काष्ठा box *float_children;

	/**
	 * Next sibling float box.
	 */
	काष्ठा box *अग्र_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	काष्ठा box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	पूर्णांक clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	पूर्णांक cached_place_below_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	पूर्णांक x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	पूर्णांक y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	पूर्णांक width;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	पूर्णांक height;

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
	पूर्णांक descendant_x0;  /**< left edge of descendants */
	पूर्णांक descendant_y0;  /**< top edge of descendants */
	पूर्णांक descendant_x1;  /**< right edge of descendants */
	पूर्णांक descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	पूर्णांक margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	पूर्णांक padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	काष्ठा box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	काष्ठा scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	काष्ठा scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	पूर्णांक min_width;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	पूर्णांक max_width;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	अक्षर *text;

	/**
	 * Length of text.
	 */
	माप_प्रकार लंबाई;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	पूर्णांक space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	माप_प्रकार byte_offset;


	/**
	 * Link, or NULL.
	 */
	काष्ठा nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	स्थिर अक्षर *target;

	/**
	 * Title, or NULL.
	 */
	स्थिर अक्षर *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	अचिन्हित पूर्णांक columns;

	/**
	 * Number of rows for TABLE only.
	 */
	अचिन्हित पूर्णांक rows;

	/**
	 * Start column for TABLE_CELL only.
	 */
	अचिन्हित पूर्णांक start_column;

	/**
	 * Array of table column data for TABLE only.
	 */
	काष्ठा column *col;

	/**
	 * List item value.
	 */
	पूर्णांक list_मूल्य;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	काष्ठा box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	काष्ठा form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	अक्षर *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	काष्ठा hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	काष्ठा hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	काष्ठा object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	काष्ठा browser_window *iframe;

};


#पूर्ण_अगर
