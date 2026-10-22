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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_HTML_BOX_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_HTML_BOX_H

#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libcss/libcss.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "content/handlers/css/utils.h"

ਬਣਤਰ content;
ਬਣਤਰ box;
ਬਣਤਰ browser_ਖਿੜਕੀ;
ਬਣਤਰ html_content;
ਬਣਤਰ nsurl;
ਬਣਤਰ dom_ਗੰਢ;
ਬਣਤਰ dom_string;
ਬਣਤਰ rect;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNKNOWN_ਚੌੜਾਈ ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNKNOWN_MAX_ਚੌੜਾਈ ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*box_construct_complete_cb)(ਬਣਤਰ html_content *c, bool success);


/**
 * Type of a struct box.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	BOX_ਖੰਡ,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_ਸਾਰਣੀ,
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ,
	BOX_ਸਾਰਣੀ_CELL,
	BOX_ਸਾਰਣੀ_ਪੰਕਤੀ_ਸਮੂਹ,
	BOX_FLOAT_ਖੱਬਾ,
	BOX_FLOAT_ਸੱਜਾ,
	BOX_INLINE_ਖੰਡ,
	BOX_BR,
	BOX_ਪਾਠ,
	BOX_INLINE_ਸਿਰਾ,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	NEW_ਸਤਰ    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_ਉਚਾਈ  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_ਉਚਾਈ = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
ਗਿਣਤੀ box_side { ਸਿਖਰ_2, ਸੱਜਾ_2, BOTTOM, ਖੱਬਾ_2 };


/**
 * Container for box border details
 */
ਬਣਤਰ box_border {
	ਗਿਣਤੀ css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	ਪੂਰਨ ਚੌੜਾਈ;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
ਬਣਤਰ ਕਾਲਮ {
	/**
	 * Type of column.
	 */
	ਗਿਣਤੀ {
	      ਕਾਲਮ_ਚੌੜਾਈ_UNKNOWN,
	      ਕਾਲਮ_ਚੌੜਾਈ_FIXED,
	      ਕਾਲਮ_ਚੌੜਾਈ_AUTO,
	      ਕਾਲਮ_ਚੌੜਾਈ_PERCENT,
	      ਕਾਲਮ_ਚੌੜਾਈ_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	ਪੂਰਨ ਚੌੜਾਈ;

	/**
	 * Minimum width of content.
	 */
	ਪੂਰਨ min;

	/**
	 * Maximum width of content.
	 */
	ਪੂਰਨ max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
ਬਣਤਰ object_param {
	ਅੱਖਰ *name;
	ਅੱਖਰ *ਮੁੱਲ;
	ਅੱਖਰ *type;
	ਅੱਖਰ *valuetype;
	ਬਣਤਰ object_param *ਅਗਲਾ;
};


/**
 * Parameters for object element and similar elements.
 */
ਬਣਤਰ object_params {
	ਬਣਤਰ nsurl *ਅੰਕੜੇ;
	ਅੱਖਰ *type;
	ਅੱਖਰ *codetype;
	ਬਣਤਰ nsurl *codebase;
	ਬਣਤਰ nsurl *classid;
	ਬਣਤਰ object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
ਬਣਤਰ box {
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
	ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ;

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
	ਬਣਤਰ box *ਅਗਲਾ;

	/**
	 * Previous sibling box, or NULL.
	 */
	ਬਣਤਰ box *prev;

	/**
	 * First child box, or NULL.
	 */
	ਬਣਤਰ box *children;

	/**
	 * Last child box, or NULL.
	 */
	ਬਣਤਰ box *ਆਖਰੀ;

	/**
	 * Parent box, or NULL.
	 */
	ਬਣਤਰ box *ਮਾਪਾ;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	ਬਣਤਰ box *inline_ਸਿਰਾ;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	ਬਣਤਰ box *float_children;

	/**
	 * Next sibling float box.
	 */
	ਬਣਤਰ box *ਅਗਲਾ_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	ਬਣਤਰ box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	ਪੂਰਨ clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	ਪੂਰਨ cached_ਰੱਖੋ_ਹੇਠਾਂ_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	ਪੂਰਨ x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	ਪੂਰਨ y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	ਪੂਰਨ ਚੌੜਾਈ;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	ਪੂਰਨ ਉਚਾਈ;

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
	ਪੂਰਨ descendant_x0;  /**< left edge of descendants */
	ਪੂਰਨ descendant_y0;  /**< top edge of descendants */
	ਪੂਰਨ descendant_x1;  /**< right edge of descendants */
	ਪੂਰਨ descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ਪੂਰਨ margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ਪੂਰਨ padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	ਬਣਤਰ box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	ਬਣਤਰ scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	ਬਣਤਰ scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	ਪੂਰਨ min_ਚੌੜਾਈ;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	ਪੂਰਨ max_ਚੌੜਾਈ;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	ਅੱਖਰ *ਪਾਠ;

	/**
	 * Length of text.
	 */
	size_t ਲੰਮਾਈ;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	ਪੂਰਨ space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_ਵਿੱਥ;


	/**
	 * Link, or NULL.
	 */
	ਬਣਤਰ nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ;

	/**
	 * Title, or NULL.
	 */
	ਅਚਲ ਅੱਖਰ *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ columns;

	/**
	 * Number of rows for TABLE only.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ;

	/**
	 * Start column for TABLE_CELL only.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ੁਰੂ_ਕਾਲਮ;

	/**
	 * Array of table column data for TABLE only.
	 */
	ਬਣਤਰ ਕਾਲਮ *ਖਾਨਾ;

	/**
	 * List item value.
	 */
	ਪੂਰਨ list_ਮੁੱਲ;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	ਬਣਤਰ box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	ਬਣਤਰ form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	ਅੱਖਰ *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	ਬਣਤਰ hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	ਬਣਤਰ hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	ਬਣਤਰ object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	ਬਣਤਰ browser_ਖਿੜਕੀ *iframe;

};


#ਜੇ_ਅੰਤ
