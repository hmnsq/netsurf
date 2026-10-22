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

#جے_بنا_تعریف NETSURF_HTML_BOX_H
#تعریف_کرو NETSURF_HTML_BOX_H

#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <libcss/libcss.h>

#ضمن "content/handlers/css/utils.h"

بنية content;
بنية box;
بنية browser_کھڑکی;
بنية html_content;
بنية nsurl;
بنية dom_گرہ;
بنية dom_string;
بنية rect;

#تعریف_کرو UNKNOWN_چوڑائی ودھ_توں_ودھ_عدد
#تعریف_کرو UNKNOWN_MAX_چوڑائی ودھ_توں_ودھ_عدد


تعريفنوع فراغ (*box_construct_complete_cb)(بنية html_content *c, bool success);


/**
 * Type of a struct box.
 */
تعريفنوع تعداد {
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
	BOX_INLINE_سرا,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
تعريفنوع تعداد {
	NEW_سطر    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_اچائی  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_اچائی = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
تعداد box_side { چوٹی_2, دایاں_2, BOTTOM, بایاں_2 };


/**
 * Container for box border details
 */
بنية box_border {
	تعداد css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	صحيح چوڑائی;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
بنية کالم {
	/**
	 * Type of column.
	 */
	تعداد {
	      کالم_چوڑائی_UNKNOWN,
	      کالم_چوڑائی_FIXED,
	      کالم_چوڑائی_AUTO,
	      کالم_چوڑائی_PERCENT,
	      کالم_چوڑائی_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	صحيح چوڑائی;

	/**
	 * Minimum width of content.
	 */
	صحيح min;

	/**
	 * Maximum width of content.
	 */
	صحيح max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
بنية object_param {
	حرف *name;
	حرف *قدر;
	حرف *type;
	حرف *valuetype;
	بنية object_param *اگلا;
};


/**
 * Parameters for object element and similar elements.
 */
بنية object_params {
	بنية nsurl *انکڑے;
	حرف *type;
	حرف *codetype;
	بنية nsurl *codebase;
	بنية nsurl *classid;
	بنية object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
بنية box {
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
	بنية dom_گرہ *گرہ;

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
	بنية box *اگلا;

	/**
	 * Previous sibling box, or NULL.
	 */
	بنية box *prev;

	/**
	 * First child box, or NULL.
	 */
	بنية box *children;

	/**
	 * Last child box, or NULL.
	 */
	بنية box *آخری;

	/**
	 * Parent box, or NULL.
	 */
	بنية box *ماپا;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	بنية box *inline_سرا;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	بنية box *float_children;

	/**
	 * Next sibling float box.
	 */
	بنية box *اگلا_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	بنية box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	صحيح clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	صحيح cached_رکھو_ہیٹھاں_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	صحيح x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	صحيح y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	صحيح چوڑائی;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	صحيح اچائی;

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
	صحيح descendant_x0;  /**< left edge of descendants */
	صحيح descendant_y0;  /**< top edge of descendants */
	صحيح descendant_x1;  /**< right edge of descendants */
	صحيح descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	صحيح margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	صحيح padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	بنية box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	بنية scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	بنية scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	صحيح min_چوڑائی;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	صحيح max_چوڑائی;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	حرف *متن;

	/**
	 * Length of text.
	 */
	size_t لمائی;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	صحيح space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_وتھ;


	/**
	 * Link, or NULL.
	 */
	بنية nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	ثابت حرف *نشانہ;

	/**
	 * Title, or NULL.
	 */
	ثابت حرف *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	بنانشانی صحيح columns;

	/**
	 * Number of rows for TABLE only.
	 */
	بنانشانی صحيح صفاں;

	/**
	 * Start column for TABLE_CELL only.
	 */
	بنانشانی صحيح شروع_کالم;

	/**
	 * Array of table column data for TABLE only.
	 */
	بنية کالم *خانہ;

	/**
	 * List item value.
	 */
	صحيح list_قدر;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	بنية box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	بنية form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	حرف *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	بنية hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	بنية hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	بنية object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	بنية browser_کھڑکی *iframe;

};


#جے_انت
