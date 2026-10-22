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

#اگر_غیر_تعریف_شدہ NETSURF_HTML_BOX_H
#تعریف_کرو NETSURF_HTML_BOX_H

#شامل <limits.h>
#شامل <stdbool.h>
#شامل <libcss/libcss.h>

#شامل "content/handlers/css/utils.h"

ساخت content;
ساخت box;
ساخت browser_کھڑکی;
ساخت html_content;
ساخت nsurl;
ساخت dom_گرہ;
ساخت dom_string;
ساخت rect;

#تعریف_کرو UNKNOWN_چوڑائی زیادہ_سے_زیادہ_عدد
#تعریف_کرو UNKNOWN_MAX_چوڑائی زیادہ_سے_زیادہ_عدد


قسمتعریف خالی (*box_construct_complete_cb)(ساخت html_content *c, bool success);


/**
 * Type of a struct box.
 */
قسمتعریف شمار {
	BOX_بلاک,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_جدول,
	BOX_جدول_صف,
	BOX_جدول_CELL,
	BOX_جدول_صف_گروہ,
	BOX_FLOAT_بایاں,
	BOX_FLOAT_دایاں,
	BOX_INLINE_بلاک,
	BOX_BR,
	BOX_متن,
	BOX_INLINE_انتہا,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
قسمتعریف شمار {
	NEW_سطر    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_اونچائی  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_اونچائی = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
شمار box_side { چوٹی_2, دایاں_2, BOTTOM, بایاں_2 };


/**
 * Container for box border details
 */
ساخت box_border {
	شمار css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	صحیح چوڑائی;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
ساخت عمود {
	/**
	 * Type of column.
	 */
	شمار {
	      عمود_چوڑائی_UNKNOWN,
	      عمود_چوڑائی_FIXED,
	      عمود_چوڑائی_AUTO,
	      عمود_چوڑائی_PERCENT,
	      عمود_چوڑائی_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	صحیح چوڑائی;

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
ساخت object_param {
	حرف *name;
	حرف *قدر;
	حرف *type;
	حرف *valuetype;
	ساخت object_param *اگلا;
};


/**
 * Parameters for object element and similar elements.
 */
ساخت object_params {
	ساخت nsurl *ڈیٹا;
	حرف *type;
	حرف *codetype;
	ساخت nsurl *codebase;
	ساخت nsurl *classid;
	ساخت object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
ساخت box {
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
	ساخت dom_گرہ *گرہ;

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
	ساخت box *اگلا;

	/**
	 * Previous sibling box, or NULL.
	 */
	ساخت box *پچھلا;

	/**
	 * First child box, or NULL.
	 */
	ساخت box *children;

	/**
	 * Last child box, or NULL.
	 */
	ساخت box *آخری;

	/**
	 * Parent box, or NULL.
	 */
	ساخت box *والد;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	ساخت box *inline_انتہا;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	ساخت box *float_children;

	/**
	 * Next sibling float box.
	 */
	ساخت box *اگلا_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	ساخت box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	صحیح clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	صحیح cached_رکھو_نیچے_level;


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
	صحیح چوڑائی;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	صحیح اونچائی;

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
	ساخت box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	ساخت scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	ساخت scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	صحیح min_چوڑائی;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	صحیح max_چوڑائی;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	حرف *متن;

	/**
	 * Length of text.
	 */
	size_t لمبائی;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	صحیح space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_انحراف;


	/**
	 * Link, or NULL.
	 */
	ساخت nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ثابت حرف *ہدف;

	/**
	 * Title, or NULL.
	 */
	ثابت حرف *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	غیرعلامتی صحیح columns;

	/**
	 * Number of rows for TABLE only.
	 */
	غیرعلامتی صحیح صفیں;

	/**
	 * Start column for TABLE_CELL only.
	 */
	غیرعلامتی صحیح آغاز_عمود;

	/**
	 * Array of table column data for TABLE only.
	 */
	ساخت عمود *ستون;

	/**
	 * List item value.
	 */
	صحیح list_قدر;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	ساخت box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	ساخت form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	حرف *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	ساخت hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	ساخت hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	ساخت object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	ساخت browser_کھڑکی *iframe;

};


#اگر_ختم
