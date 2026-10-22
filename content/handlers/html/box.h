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

#నిర్వచితం_కాకపోతే NETSURF_HTML_BOX_H
#నిర్వచించు NETSURF_HTML_BOX_H

#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <libcss/libcss.h>

#అన్తర్భావ "content/handlers/css/utils.h"

నిర్మాణం content;
నిర్మాణం box;
నిర్మాణం browser_కిటికీ;
నిర్మాణం html_content;
నిర్మాణం nsurl;
నిర్మాణం dom_కణుపు;
నిర్మాణం dom_string;
నిర్మాణం rect;

#నిర్వచించు UNKNOWN_వెడల్పు గరిష్ఠ_పూర్ణాంకం
#నిర్వచించు UNKNOWN_MAX_వెడల్పు గరిష్ఠ_పూర్ణాంకం


రకనిర్వచనం శూన్య (*box_construct_complete_cb)(నిర్మాణం html_content *c, bool success);


/**
 * Type of a struct box.
 */
రకనిర్వచనం గణన {
	BOX_ఖండం,
	BOX_INLINE_CONTAINER,
	BOX_INLINE,
	BOX_పట్టిక,
	BOX_పట్టిక_అడ్డువరుస,
	BOX_పట్టిక_CELL,
	BOX_పట్టిక_అడ్డువరుస_సమూహం,
	BOX_FLOAT_ఎడమ,
	BOX_FLOAT_కుడి,
	BOX_INLINE_ఖండం,
	BOX_BR,
	BOX_పాఠ్యం,
	BOX_INLINE_చివర,
	BOX_NONE,
	BOX_FLEX,
	BOX_INLINE_FLEX,
} box_type;


/**
 * Flags for a struct box.
 */
రకనిర్వచనం గణన {
	NEW_పంక్తి    = 1 << 0,	/* first inline on a new line */
	STYLE_OWNED = 1 << 1,	/* style is owned by this box */
	PRINTED     = 1 << 2,	/* box has already been printed */
	PRE_STRIP   = 1 << 3,	/* PRE tag needing leading newline stripped */
	CLONE       = 1 << 4,	/* continuation of previous box from wrapping */
	MEASURED    = 1 << 5,	/* text box width has been measured */
	HAS_ఎత్తు  = 1 << 6,	/* box has height (perhaps due to children) */
	MAKE_ఎత్తు = 1 << 7,	/* box causes its own height */
	NEED_MIN    = 1 << 8,	/* minimum width is required for layout */
	REPLACE_DIM = 1 << 9,	/* replaced element has given dimensions */
	IFRAME      = 1 << 10,	/* box contains an iframe */
	CONVERT_CHILDREN = 1 << 11,  /* wanted children converting */
	IS_REPLACED = 1 << 12	/* box is a replaced element */
} box_flags;


/**
 * Sides of a box
 */
గణన box_side { శిఖరం_2, కుడి_2, BOTTOM, ఎడమ_2 };


/**
 * Container for box border details
 */
నిర్మాణం box_border {
	గణన css_border_style_e style;	/**< border-style */
	css_color c;			/**< border-color value */
	పూర్ణాంకం వెడల్పు;			/**< border-width (pixels) */
};


/**
 * Table column data.
 */
నిర్మాణం స్తంభం {
	/**
	 * Type of column.
	 */
	గణన {
	      స్తంభం_వెడల్పు_UNKNOWN,
	      స్తంభం_వెడల్పు_FIXED,
	      స్తంభం_వెడల్పు_AUTO,
	      స్తంభం_వెడల్పు_PERCENT,
	      స్తంభం_వెడల్పు_RELATIVE
	} type;

	/**
	 * Preferred width of column. Pixels for FIXED, percentage for
	 *  PERCENT, relative units for RELATIVE, unused for AUTO.
	 */
	పూర్ణాంకం వెడల్పు;

	/**
	 * Minimum width of content.
	 */
	పూర్ణాంకం min;

	/**
	 * Maximum width of content.
	 */
	పూర్ణాంకం max;

	/**
	 * Whether all of column's cells are css positioned.
	 */
	bool positioned;
};


/**
 * Linked list of object element parameters.
 */
నిర్మాణం object_param {
	అక్షరం *name;
	అక్షరం *విలువ;
	అక్షరం *type;
	అక్షరం *valuetype;
	నిర్మాణం object_param *తదుపరి;
};


/**
 * Parameters for object element and similar elements.
 */
నిర్మాణం object_params {
	నిర్మాణం nsurl *దత్తాంశం;
	అక్షరం *type;
	అక్షరం *codetype;
	నిర్మాణం nsurl *codebase;
	నిర్మాణం nsurl *classid;
	నిర్మాణం object_param *params;
};


/**
 * Node in box tree. All dimensions are in pixels.
 */
నిర్మాణం box {
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
	నిర్మాణం dom_కణుపు *కణుపు;

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
	నిర్మాణం box *తదుపరి;

	/**
	 * Previous sibling box, or NULL.
	 */
	నిర్మాణం box *prev;

	/**
	 * First child box, or NULL.
	 */
	నిర్మాణం box *children;

	/**
	 * Last child box, or NULL.
	 */
	నిర్మాణం box *చివరి;

	/**
	 * Parent box, or NULL.
	 */
	నిర్మాణం box *మాతృ;

	/**
	 * INLINE_END box corresponding to this INLINE box, or INLINE
	 * box corresponding to this INLINE_END box.
	 */
	నిర్మాణం box *inline_చివర;


	/**
	 * First float child box, or NULL. Float boxes are in the tree
	 * twice, in this list for the block box which defines the
	 * area for floats, and also in the standard tree given by
	 * children, next, prev, etc.
	 */
	నిర్మాణం box *float_children;

	/**
	 * Next sibling float box.
	 */
	నిర్మాణం box *తదుపరి_float;

	/**
	 * If box is a float, points to box's containing block
	 */
	నిర్మాణం box *float_container;

	/**
	 * Level below which subsequent floats must be cleared.  This
	 * is used only for boxes with float_children
	 */
	పూర్ణాంకం clear_level;

	/**
	 * Level below which floats have been placed.
	 */
	పూర్ణాంకం cached_ఉంచు_క్రింద_level;


	/**
	 * Coordinate of left padding edge relative to parent box, or
	 * relative to ancestor that contains this box in
	 * float_children for FLOAT_.
	 */
	పూర్ణాంకం x;
	/**
	 * Coordinate of top padding edge, relative as for x.
	 */
	పూర్ణాంకం y;

	/**
	 * Width of content box (excluding padding etc.).
	 */
	పూర్ణాంకం వెడల్పు;
	/**
	 * Height of content box (excluding padding etc.).
	 */
	పూర్ణాంకం ఎత్తు;

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
	పూర్ణాంకం descendant_x0;  /**< left edge of descendants */
	పూర్ణాంకం descendant_y0;  /**< top edge of descendants */
	పూర్ణాంకం descendant_x1;  /**< right edge of descendants */
	పూర్ణాంకం descendant_y1;  /**< bottom edge of descendants */

	/**
	 * Margin: TOP, RIGHT, BOTTOM, LEFT.
	 */
	పూర్ణాంకం margin[4];

	/**
	 * Padding: TOP, RIGHT, BOTTOM, LEFT.
	 */
	పూర్ణాంకం padding[4];

	/**
	 * Border: TOP, RIGHT, BOTTOM, LEFT.
	 */
	నిర్మాణం box_border border[4];

	/**
	 * Horizontal scroll.
	 */
	నిర్మాణం scrollbar *scroll_x;

	/**
	 * Vertical scroll.
	 */
	నిర్మాణం scrollbar *scroll_y;

	/**
	 * Width of box taking all line breaks (including margins
	 * etc). Must be non-negative.
	 */
	పూర్ణాంకం min_వెడల్పు;

	/**
	 * Width that would be taken with no line breaks. Must be
	 * non-negative.
	 */
	పూర్ణాంకం max_వెడల్పు;


	/**
	 * Text, or NULL if none. Unterminated.
	 */
	అక్షరం *పాఠ్యం;

	/**
	 * Length of text.
	 */
	size_t నిడివి;

	/**
	 * Width of space after current text (depends on font and size).
	 */
	పూర్ణాంకం space;

	/**
	 * Byte offset within a textual representation of this content.
	 */
	size_t byte_స్థానభ్రంశం;


	/**
	 * Link, or NULL.
	 */
	నిర్మాణం nsurl *href;

	/**
	 * Link target, or NULL.
	 */
	స్థిరరాశి అక్షరం *లక్ష్యం;

	/**
	 * Title, or NULL.
	 */
	స్థిరరాశి అక్షరం *title;


	/**
	 * Number of columns for TABLE / TABLE_CELL.
	 */
	గుర్తులేని పూర్ణాంకం columns;

	/**
	 * Number of rows for TABLE only.
	 */
	గుర్తులేని పూర్ణాంకం అడ్డువరుసలు;

	/**
	 * Start column for TABLE_CELL only.
	 */
	గుర్తులేని పూర్ణాంకం ప్రారంభం_స్తంభం;

	/**
	 * Array of table column data for TABLE only.
	 */
	నిర్మాణం స్తంభం *నిలువు;

	/**
	 * List item value.
	 */
	పూర్ణాంకం list_విలువ;

	/**
	 * List marker box if this is a list-item, or NULL.
	 */
	నిర్మాణం box *list_marker;


	/**
	 * Form control data, or NULL if not a form control.
	 */
	నిర్మాణం form_control* gadget;


	/**
	 * (Image)map to use with this object, or NULL if none
	 */
	అక్షరం *usemap;


	/**
	 * Background image for this box, or NULL if none
	 */
	నిర్మాణం hlcache_handle *background;


	/**
	 * Object in this box (usually an image), or NULL if none.
	 */
	నిర్మాణం hlcache_handle* object;

	/**
	 * Parameters for the object, or NULL.
	 */
	నిర్మాణం object_params *object_params;


	/**
	 * Iframe's browser_window, or NULL if none
	 */
	నిర్మాణం browser_కిటికీ *iframe;

};


#అయితే_ముగింపు
