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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_HTML_BOX_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_HTML_BOX_H

#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

#ಅನ್ತರ್ಭಾವ "content/handlers/css/utils.h"

ರಚನೆ content;
ರಚನೆ box;
ರಚನೆ browser_ಕಿಟಕಿ;
ರಚನೆ html_content;
ರಚನೆ nsurl;
ರಚನೆ dom_ಗಂಟು;
ರಚನೆ dom_string;
ರಚನೆ rect;

#ವ್ಯಾಖ್ಯಾನಿಸು UNKNOWN_ಅಗಲ ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು UNKNOWN_MAX_ಅಗಲ ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*box_construct_complete_cb)(ರಚನೆ html_content *c, bool success);


/**
 * Type of a struct box.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	BOX_ಖಂಡ,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_ಕೋಷ್ಟಕ,
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು,
	BOX_ಕೋಷ್ಟಕ_CELL,
	BOX_ಕೋಷ್ಟಕ_ಅಡ್ಡಸಾಲು_ಗುಂಪು,
	BOX_FLOAT_ಎಡ,
	BOX_FLOAT_ಬಲ,
	BOX_INLINE_ಖಂಡ,
	BOX_BR,
	BOX_ಪಠ್ಯ,
	BOX_INLINE_ಕೊನೆ,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	NEW_ಸಾಲು    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_ಎತ್ತರ  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_ಎತ್ತರ = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
ಎಣಿಕೆ box_side { ತುದಿ_2, ಬಲ_2, BOTTOM, ಎಡ_2 };


/**
 * Container for box border details
 */
ರಚನೆ box_border {
	ಎಣಿಕೆ css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	ಪೂರ್ಣಾಂಕ ಅಗಲ;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
ರಚನೆ ಸ್ತಂಭ {
	/**
	 * Type of column.
	 */
	ಎಣಿಕೆ {
	      ಸ್ತಂಭ_ಅಗಲ_UNKNOWN,
	      ಸ್ತಂಭ_ಅಗಲ_FIXED,
	      ಸ್ತಂಭ_ಅಗಲ_AUTO,
	      ಸ್ತಂಭ_ಅಗಲ_PERCENT,
	      ಸ್ತಂಭ_ಅಗಲ_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	ಪೂರ್ಣಾಂಕ ಅಗಲ;

	/**
	 * Minimum width of content.
	 */
	ಪೂರ್ಣಾಂಕ min;

	/**
	 * Maximum width of content.
	 */
	ಪೂರ್ಣಾಂಕ max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
ರಚನೆ object_param {
	ಅಕ್ಷರ *name;
	ಅಕ್ಷರ *ಮೌಲ್ಯ;
	ಅಕ್ಷರ *type;
	ಅಕ್ಷರ *valuetype;
	ರಚನೆ object_param *ಮುಂದಿನ;
};


/**
 * Parameters for object element and similar elements.
 */
ರಚನೆ object_params {
	ರಚನೆ nsurl *ದತ್ತಾಂಶ;
	ಅಕ್ಷರ *type;
	ಅಕ್ಷರ *codetype;
	ರಚನೆ nsurl *codebase;
	ರಚನೆ nsurl *classid;
	ರಚನೆ object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
ರಚನೆ box {
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
	ರಚನೆ dom_ಗಂಟು *ಗಂಟು;

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
	ರಚನೆ box *ಮುಂದಿನ;

	/**
	 * Previous sibling box, or NULL.
	 */
	ರಚನೆ box *prev;

	/**
	 * First child box, or NULL.
	 */
	ರಚನೆ box *children;

	/**
	 * Last child box, or NULL.
	 */
	ರಚನೆ box *ಕೊನೆಯ;

	/**
	 * Parent box, or NULL.
	 */
	ರಚನೆ box *ಪೋಷಕ;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	ರಚನೆ box *inline_ಕೊನೆ;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	ರಚನೆ box *float_children;

	/**
	 * Next sibling float box.
	 */
	ರಚನೆ box *ಮುಂದಿನ_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	ರಚನೆ box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	ಪೂರ್ಣಾಂಕ clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	ಪೂರ್ಣಾಂಕ cached_ಇರಿಸು_ಕೆಳಗೆ_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	ಪೂರ್ಣಾಂಕ x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	ಪೂರ್ಣಾಂಕ y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	ಪೂರ್ಣಾಂಕ ಅಗಲ;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	ಪೂರ್ಣಾಂಕ ಎತ್ತರ;

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
	ಪೂರ್ಣಾಂಕ descendant_x0;  /**< left edge of descendants */
	ಪೂರ್ಣಾಂಕ descendant_y0;  /**< top edge of descendants */
	ಪೂರ್ಣಾಂಕ descendant_x1;  /**< right edge of descendants */
	ಪೂರ್ಣಾಂಕ descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ಪೂರ್ಣಾಂಕ margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ಪೂರ್ಣಾಂಕ padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ರಚನೆ box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	ರಚನೆ scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	ರಚನೆ scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	ಪೂರ್ಣಾಂಕ min_ಅಗಲ;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	ಪೂರ್ಣಾಂಕ max_ಅಗಲ;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	ಅಕ್ಷರ *ಪಠ್ಯ;

	/**
	 * Length of text.
	 */
	size_t ಉದ್ದಳತೆ;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	ಪೂರ್ಣಾಂಕ space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_ಸ್ಥಾನಾಂತರ;


	/**
	 * Link, or NULL.
	 */
	ರಚನೆ nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ;

	/**
	 * Title, or NULL.
	 */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ columns;

	/**
	 * Number of rows for TABLE only.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು;

	/**
	 * Start column for TABLE_CELL only.
	 */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆರಂಭ_ಸ್ತಂಭ;

	/**
	 * Array of table column data for TABLE only.
	 */
	ರಚನೆ ಸ್ತಂಭ *ಲಂಬಸಾಲು;

	/**
	 * List item value.
	 */
	ಪೂರ್ಣಾಂಕ list_ಮೌಲ್ಯ;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	ರಚನೆ box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	ರಚನೆ form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	ಅಕ್ಷರ *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	ರಚನೆ hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	ರಚನೆ hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	ರಚನೆ object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	ರಚನೆ browser_ಕಿಟಕಿ *iframe;

};


#ಆದರೆ_ಅಂತ್ಯ
