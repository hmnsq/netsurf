/*
 * Copyright 2005 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2005 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2006 Richard Wilson <info@tinct.net>
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
 * Implementation of special element handling conversion.
 */

#include <string.h>
#include <stdbool.h>
#include <dom/dom.h>

#include "utils/nsoption.h"
#include "utils/corestrings.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/talloc.h"
#include "utils/string.h"
#include "utils/ascii.h"
#include "utils/nsurl.h"
#include "netsurf/plot_style.h"
#include "css/hints.h"
#include "desktop/frame_types.h"
#include "content/content_factory.h"

#include "html/html.h"
#include "html/private.h"
#include "html/object.h"
#include "html/box.h"
#include "html/box_manipulate.h"
#include "html/box_construct.h"
#include "html/box_special.h"
#include "html/box_textarea.h"
#include "html/form_internal.h"


static const content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
static anta_hstha bool box_is_root(dom_node *n)
{
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (parent != NULL) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
			ꦮꦭꦶ false;

		ꦪꦼꦤ꧀ (type != DOM_DOCUMENT_NODE)
			ꦮꦭꦶ false;
	}

	ꦮꦭꦶ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static puur_naa_nka box_object_talloc_destructor(sa_nracanaa object_params *o)
{
	ꦪꦼꦤ꧀ (o->codebase != NULL)
		nsurl_unref(o->codebase);
	ꦪꦼꦤ꧀ (o->classid != NULL)
		nsurl_unref(o->classid);
	ꦪꦼꦤ꧀ (o->data != NULL)
		nsurl_unref(o->data);

	ꦮꦭꦶ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
static sa_nracanaa frame_dimension *
box_parse_multi_lengths(const dom_string *ds, unsigned puur_naa_nka *count)
{
	var_na *end;
	unsigned puur_naa_nka i, n;
	sa_nracanaa frame_dimension *length;
	const var_na *s;

	s = dom_string_data(ds);

	ꦏꦁꦒꦺꦴ (i = 0, n = 1; s[i]; i++)
		ꦪꦼꦤ꧀ (s[i] == ',')
			n++;

	length = calloc(n, parimaa_na(sa_nracanaa frame_dimension));
	ꦪꦼꦤ꧀ (!length)
		ꦮꦭꦶ NULL;

	ꦏꦁꦒꦺꦴ (i = 0; i != n; i++) {
		ꦱꦼꦢꦼꦭ (ascii_is_space(*s)) {
			s++;
		}
		length[i].value = strtof(s, &end);
		ꦪꦼꦤ꧀ (length[i].value <= 0) {
			length[i].value = 1;
		}
		s = end;
		switch (*s) {
			pak_sa '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				bha_nga;
			pak_sa '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				bha_nga;
			default:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				bha_nga;
		}
		ꦱꦼꦢꦼꦭ (*s && *s != ',') {
			s++;
		}
		ꦪꦼꦤ꧀ (*s == ',') {
			s++;
		}
	}

	*count = n;
	ꦮꦭꦶ length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static puur_naa_nka box_frames_talloc_destructor(sa_nracanaa content_html_frames *f)
{
	ꦪꦼꦤ꧀ (f->url != NULL) {
		nsurl_unref(f->url);
		f->url = NULL;
	}

	ꦮꦭꦶ 0;
}


/**
 * create a frameset box tree
 */
static bool
box_create_frameset(sa_nracanaa content_html_frames *f,
		    dom_node *n,
		    html_content *content)
{
	unsigned puur_naa_nka row, col, index, i;
	unsigned puur_naa_nka rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	sa_nracanaa frame_dimension *row_height = 0, *col_width = 0;
	dom_node *c, *next;
	sa_nracanaa content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		ꦪꦼꦤ꧀ (row_height == NULL)
			ꦮꦭꦶ false;
	} ꦭꦶꦪ {
		row_height = calloc(1, parimaa_na(sa_nracanaa frame_dimension));
		ꦪꦼꦤ꧀ (row_height == NULL)
			ꦮꦭꦶ false;
		row_height->value = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		ꦪꦼꦤ꧀ (col_width == NULL) {
			free(row_height);
			ꦮꦭꦶ false;
		}
	} ꦭꦶꦪ {
		col_width = calloc(1, parimaa_na(sa_nracanaa frame_dimension));
		ꦪꦼꦤ꧀ (col_width == NULL) {
			free(row_height);
			ꦮꦭꦶ false;
		}
		col_width->value = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		ꦪꦼꦤ꧀ ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		css_color color;

		ꦪꦼꦤ꧀ (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, sa_nracanaa content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	ꦏꦁꦒꦺꦴ (row = 0; row < rows; row++) {
		ꦏꦁꦒꦺꦴ (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = NULL;
			frame->url = NULL;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = NULL;
		}
	}
	free(col_width);
	free(row_height);

	/* create the frameset windows */
	err = dom_node_get_first_child(n, &c);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	ꦏꦁꦒꦺꦴ (row = 0; c != NULL && row < rows; row++) {
		ꦏꦁꦒꦺꦴ (col = 0; c != NULL && col < cols; col++) {
			ꦱꦼꦢꦼꦭ (c != NULL) {
				dom_node_type type;
				dom_string *name;

				err = dom_node_get_node_type(c, &type);
				ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ꦮꦭꦶ false;
				}

				err = dom_node_get_node_name(c, &name);
				ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ꦮꦭꦶ false;
				}

				ꦪꦼꦤ꧀ (type != DOM_ELEMENT_NODE ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_node_get_next_sibling(c,
							&next);
					ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_node_unref(c);
						ꦮꦭꦶ false;
					}

					dom_string_unref(name);
					dom_node_unref(c);
					c = next;
				} ꦭꦶꦪ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					bha_nga;
				}
			}

			ꦪꦼꦤ꧀ (c == NULL)
				bha_nga;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_node_get_node_name(c, &s);
			ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				ꦪꦼꦤ꧀ (box_create_frameset(frame, c,
						content) == false) {
					dom_node_unref(c);
					ꦮꦭꦶ false;
				}

				err = dom_node_get_next_sibling(c, &next);
				ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ꦮꦭꦶ false;
				}

				dom_node_unref(c);
				c = next;
				continue;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = NULL;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ꦪꦼꦤ꧀ (url != NULL) {
				/* no self-references */
				ꦪꦼꦤ꧀ (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = NULL;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			ꦪꦼꦤ꧀ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				css_color color;

				ꦪꦼꦤ꧀ (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_node_get_next_sibling(c, &next);
			ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			dom_node_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ꦪꦼꦤ꧀ (c != NULL) {
		dom_node_unref(c);
	}

	ꦮꦭꦶ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static puur_naa_nka box_iframes_talloc_destructor(sa_nracanaa content_html_iframe *f)
{
	ꦪꦼꦤ꧀ (f->url != NULL) {
		nsurl_unref(f->url);
		f->url = NULL;
	}

	ꦮꦭꦶ 0;
}


/**
 * Get the value of a dom node element's attribute.
 *
 * \param  n	      dom element node
 * \param  attribute  name of attribute
 * \param  context    talloc context for result buffer
 * \param  value      updated to value, if the attribute is present
 * \return  true on success, false if attribute present but memory exhausted
 *
 * \note returning true does not imply that the attribute was found. If the
 * attribute was not found, *value will be unchanged.
 */
static bool
box_get_attribute(dom_node *n,
		  const var_na *attribute,
		  void *context,
		  var_na **value)
{
	var_na *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((const uint8_t *) attribute,
			strlen(attribute), &attr_name);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ꦮꦭꦶ false;
	}

	dom_string_unref(attr_name);

	ꦪꦼꦤ꧀ (attr != NULL) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		ꦪꦼꦤ꧀ (result == NULL)
			ꦮꦭꦶ false;

		*value = result;
	}

	ꦮꦭꦶ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
static bool
box_input_text(html_content *html, sa_nracanaa box *box, sa_nracanaa dom_node *node)
{
	sa_nracanaa box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	switch (display) {
	pak_sa CSS_DISPLAY_GRID:
	pak_sa CSS_DISPLAY_FLEX:
	pak_sa CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		bha_nga;
	default:
		box->type = BOX_INLINE_BLOCK;
		bha_nga;
	}

	inline_container = box_create(NULL, 0, false, 0, 0, 0, 0, html->bctx);
	ꦪꦼꦤ꧀ (!inline_container)
		ꦮꦭꦶ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(NULL, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ꦪꦼꦤ꧀ (!inline_box)
		ꦮꦭꦶ false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ꦮꦭꦶ box_textarea_create_textarea(html, box, node);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
static bool box_select_add_option(sa_nracanaa form_control *control, dom_node *n)
{
	var_na *value = NULL;
	var_na *text = NULL;
	var_na *text_nowrap = NULL;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_node_get_text_content(n, &content);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (content != NULL) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} ꦭꦶꦪ {
		text = strdup("");
	}

	ꦪꦼꦤ꧀ (text == NULL)
		goto no_memory;

	err = dom_element_get_attribute(n, corestring_dom_value, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		value = strdup(dom_string_data(s));
		dom_string_unref(s);
	} ꦭꦶꦪ {
		value = strdup(text);
	}

	ꦪꦼꦤ꧀ (value == NULL)
		goto no_memory;

	ꦪꦼꦤ꧀ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	ꦪꦼꦤ꧀ (text_nowrap == NULL)
		goto no_memory;

	ꦪꦼꦤ꧀ (form_add_option(control, value, text_nowrap, selected, n) == false)
		goto no_memory;

	free(text);

	ꦮꦭꦶ true;

no_memory:
	free(value);
	free(text);
	free(text_nowrap);
	ꦮꦭꦶ false;
}


/**
 * \name  Special case element handlers
 *
 * These functions are called by box_construct_element() when an element is
 * being converted, according to the entries in element_table.
 *
 * The parameters are the xmlNode, the content for the document, and a partly
 * filled in box structure for the element.
 *
 * Return true on success, false on memory exhaustion. Set *convert_children
 * to false if children of this element in the XML tree should be skipped (for
 * example, if they have been processed in some special way already).
 *
 * Elements ordered as in the HTML 4.01 specification. Section numbers in
 * brackets [] refer to the spec.
 *
 * \{
 */

/**
 * special element handler for Anchor [12.2].
 */
static bool
box_a(dom_node *n,
      html_content *content,
      sa_nracanaa box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		ꦪꦼꦤ꧀ (!ok)
			ꦮꦭꦶ false;
		ꦪꦼꦤ꧀ (url) {
			ꦪꦼꦤ꧀ (box->href != NULL)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ꦪꦼꦤ꧀ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__top))
			box->target = "_top";
		ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		ꦭꦶꦪ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			ꦪꦼꦤ꧀ (!box->target) {
				dom_string_unref(s);
				ꦮꦭꦶ false;
			}
		}
		dom_string_unref(s);
	}

	ꦮꦭꦶ true;
}


/**
 * Document body special element handler [7.5.1].
 */
static bool
box_body(dom_node *n,
	 html_content *content,
	 sa_nracanaa box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ꦪꦼꦤ꧀ (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} ꦭꦶꦪ {
		content->background_colour = nscss_color_to_ns(color);
	}

	ꦮꦭꦶ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
static bool
box_br(dom_node *n,
       html_content *content,
       sa_nracanaa box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ꦮꦭꦶ true;
}


/**
 * special element handler for Push button [17.5].
 */
static bool
box_button(dom_node *n,
	   html_content *content,
	   sa_nracanaa box *box,
	   bool *convert_children)
{
	sa_nracanaa form_control *gadget;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ꦪꦼꦤ꧀ (!gadget)
		ꦮꦭꦶ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	ꦮꦭꦶ true;
}


/**
 * Canvas element
 */
static bool
box_canvas(dom_node *n,
	     html_content *content,
	     sa_nracanaa box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ꦪꦼꦤ꧀ (!content->enable_scripting) {
		ꦮꦭꦶ true;
	}
	*convert_children = false;

	ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ꦮꦭꦶ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ꦮꦭꦶ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
static bool
box_embed(dom_node *n,
	  html_content *content,
	  sa_nracanaa box *box,
	  bool *convert_children)
{
	sa_nracanaa object_params *params;
	sa_nracanaa object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t num_attrs;
	dom_string *src;
	dom_exception err;

	ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ꦮꦭꦶ true;

	params = talloc(content->bctx, sa_nracanaa object_params);
	ꦪꦼꦤ꧀ (params == NULL)
		ꦮꦭꦶ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = NULL;
	params->type = NULL;
	params->codetype = NULL;
	params->codebase = NULL;
	params->classid = NULL;
	params->params = NULL;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR || src == NULL)
		ꦮꦭꦶ true;
	ꦪꦼꦤ꧀ (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		ꦮꦭꦶ false;
	}

	dom_string_unref(src);

	ꦪꦼꦤ꧀ (params->data == NULL)
		ꦮꦭꦶ true;

	/* Don't include ourself */
	ꦪꦼꦤ꧀ (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		ꦮꦭꦶ true;

	/* add attributes as parameters to linked list */
	err = dom_node_get_attributes(n, &attrs);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	err = dom_namednodemap_get_length(attrs, &num_attrs);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ꦮꦭꦶ false;
	}

	ꦏꦁꦒꦺꦴ (idx = 0; idx < num_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *value;

		err = dom_namednodemap_item(attrs, idx, (void *) &attr);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ꦮꦭꦶ false;
		}

		err = dom_attr_get_name(attr, &name);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_namednodemap_unref(attrs);
			ꦮꦭꦶ false;
		}

		ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_node_unref(attr);
			dom_string_unref(name);
			continue;
		}

		err = dom_attr_get_value(attr, &value);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ꦮꦭꦶ false;
		}

		param = talloc(content->bctx, sa_nracanaa object_param);
		ꦪꦼꦤ꧀ (param == NULL) {
			dom_node_unref(attr);
			dom_string_unref(value);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ꦮꦭꦶ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->value = talloc_strdup(content->bctx, dom_string_data(value));
		param->type = NULL;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = NULL;

		dom_string_unref(value);
		dom_string_unref(name);
		dom_node_unref(attr);

		ꦪꦼꦤ꧀ (param->name == NULL || param->value == NULL ||
				param->valuetype == NULL) {
			dom_namednodemap_unref(attrs);
			ꦮꦭꦶ false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ꦮꦭꦶ html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
static bool
box_frameset(dom_node *n,
	     html_content *content,
	     sa_nracanaa box *box,
	     bool *convert_children)
{
	bool ok;

	ꦪꦼꦤ꧀ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ꦪꦼꦤ꧀ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ꦮꦭꦶ true;
	}

	content->frameset = talloc_zero(content->bctx,
					sa_nracanaa content_html_frames);
	ꦪꦼꦤ꧀ (!content->frameset) {
		ꦮꦭꦶ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ꦪꦼꦤ꧀ (ok) {
		box->type = BOX_NONE;
	}

	ꦪꦼꦤ꧀ (convert_children) {
		*convert_children = false;
	}
	ꦮꦭꦶ ok;
}


/**
 * Inline subwindow [16.5].
 */
static bool
box_iframe(dom_node *n,
	   html_content *content,
	   sa_nracanaa box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	sa_nracanaa content_html_iframe *iframe;
	puur_naa_nka i;

	ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ꦮꦭꦶ true;

	ꦪꦼꦤ꧀ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ꦮꦭꦶ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR || s == NULL)
		ꦮꦭꦶ true;
	ꦪꦼꦤ꧀ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ꦮꦭꦶ false;
	}
	dom_string_unref(s);
	ꦪꦼꦤ꧀ (url == NULL)
		ꦮꦭꦶ true;

	/* don't include ourself */
	ꦪꦼꦤ꧀ (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ꦮꦭꦶ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, sa_nracanaa content_html_iframe);
	ꦪꦼꦤ꧀ (iframe == NULL) {
		nsurl_unref(url);
		ꦮꦭꦶ false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = NULL;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		css_color color;

		ꦪꦼꦤ꧀ (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	assert(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ꦪꦼꦤ꧀ (convert_children)
		*convert_children = false;
	ꦮꦭꦶ true;
}


/**
 * Embedded image [13.2].
 */
static bool
box_image(dom_node *n,
	  html_content *content,
	  sa_nracanaa box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ga_nanaa css_width_e wtype;
	ga_nanaa css_height_e htype;
	css_fixed value = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ꦮꦭꦶ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
		var_na *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		ꦪꦼꦤ꧀ (alt == NULL)
			ꦮꦭꦶ false;
		box->text = talloc_strdup(content->bctx, alt);
		free(alt);
		ꦪꦼꦤ꧀ (box->text == NULL)
			ꦮꦭꦶ false;
		box->length = strlen(box->text);
	}

	ꦪꦼꦤ꧀ (nsoption_bool(foreground_images) == false) {
		ꦮꦭꦶ true;
	}

	/* imagemap associated with this image */
	ꦪꦼꦤ꧀ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ꦮꦭꦶ false;
	ꦪꦼꦤ꧀ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR || s == NULL)
		ꦮꦭꦶ true;

	ꦪꦼꦤ꧀ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ꦮꦭꦶ false;
	}

	dom_string_unref(s);

	ꦪꦼꦤ꧀ (url == NULL)
		ꦮꦭꦶ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &value, &wunit);
	htype = css_computed_height(box->style, &value, &hunit);

	ꦪꦼꦤ꧀ (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ꦮꦭꦶ ok;
}


/**
 * Form control [17.4].
 */
static bool
box_input(dom_node *n,
	  html_content *content,
	  sa_nracanaa box *box,
	  bool *convert_children)
{
	sa_nracanaa form_control *gadget;
	dom_string *type = NULL;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ꦪꦼꦤ꧀ (gadget == NULL) {
		ꦮꦭꦶ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ꦪꦼꦤ꧀ ((err != DOM_NO_ERR) || (type == NULL)) {
		/* no type so "text" is assumed */
		ꦪꦼꦤ꧀ (box_input_text(content, box, n) == false) {
			ꦮꦭꦶ false;
		}
		*convert_children = false;
		ꦮꦭꦶ true;
	}

	ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ꦪꦼꦤ꧀ (box_input_text(content, box, n) == false)
			goto no_memory;

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ꦭꦶꦪ ꦪꦼꦤ꧀ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		sa_nracanaa box *inline_container, *inline_box;

		ꦪꦼꦤ꧀ (box_button(n, content, box, 0) == false)
			goto no_memory;

		inline_container = box_create(NULL, 0, false, 0, 0, 0, 0,
				content->bctx);
		ꦪꦼꦤ꧀ (inline_container == NULL)
			goto no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(NULL, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ꦪꦼꦤ꧀ (inline_box == NULL)
			goto no_memory;

		inline_box->type = BOX_TEXT;

		ꦪꦼꦤ꧀ (box->gadget->value != NULL)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->value);
		ꦭꦶꦪ ꦪꦼꦤ꧀ (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ꦭꦶꦪ ꦪꦼꦤ꧀ (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ꦭꦶꦪ
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		ꦪꦼꦤ꧀ (inline_box->text == NULL)
			goto no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
				box_is_root(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ꦪꦼꦤ꧀ (err == DOM_NO_ERR && s != NULL) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				ꦪꦼꦤ꧀ (error != NSERROR_OK)
					goto no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ꦪꦼꦤ꧀ (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					ꦪꦼꦤ꧀ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						goto no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} ꦭꦶꦪ {
		/* unhandled type the default is "text" */
		ꦪꦼꦤ꧀ (box_input_text(content, box, n) == false)
			goto no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	ꦮꦭꦶ true;

no_memory:
	dom_string_unref(type);

	ꦮꦭꦶ false;
}


/**
 * Noscript element
 */
static bool
box_noscript(dom_node *n,
	     html_content *content,
	     sa_nracanaa box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ꦪꦼꦤ꧀ (content->enable_scripting) {
		*convert_children = false;
	}

	ꦮꦭꦶ true;
}


/**
 * Generic embedded object [13.3].
 */
static bool
box_object(dom_node *n,
	   html_content *content,
	   sa_nracanaa box *box,
	   bool *convert_children)
{
	sa_nracanaa object_params *params;
	sa_nracanaa object_param *param;
	dom_string *codebase, *classid, *data;
	dom_node *c;
	dom_exception err;

	ꦪꦼꦤ꧀ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ꦮꦭꦶ true;

	ꦪꦼꦤ꧀ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ꦮꦭꦶ false;
	ꦪꦼꦤ꧀ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, sa_nracanaa object_params);
	ꦪꦼꦤ꧀ (params == NULL)
		ꦮꦭꦶ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = NULL;
	params->type = NULL;
	params->codetype = NULL;
	params->codebase = NULL;
	params->classid = NULL;
	params->params = NULL;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && codebase != NULL) {
		ꦪꦼꦤ꧀ (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ꦮꦭꦶ false;
		}
		dom_string_unref(codebase);
	}
	ꦪꦼꦤ꧀ (params->codebase == NULL)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && classid != NULL) {
		ꦪꦼꦤ꧀ (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ꦮꦭꦶ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	ꦪꦼꦤ꧀ (err == DOM_NO_ERR && data != NULL) {
		ꦪꦼꦤ꧀ (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			ꦮꦭꦶ false;
		}
		dom_string_unref(data);
	}

	ꦪꦼꦤ꧀ (params->classid == NULL && params->data == NULL)
		/* nothing to embed; ignore */
		ꦮꦭꦶ true;

	/* Don't include ourself */
	ꦪꦼꦤ꧀ (params->classid != NULL && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		ꦮꦭꦶ true;

	ꦪꦼꦤ꧀ (params->data != NULL && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		ꦮꦭꦶ true;

	/* codetype and type are MIME types */
	ꦪꦼꦤ꧀ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ꦮꦭꦶ false;
	ꦪꦼꦤ꧀ (box_get_attribute(n, "type", params, &params->type) == false)
		ꦮꦭꦶ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ꦪꦼꦤ꧀ (params->classid != NULL && params->data == NULL &&
			params->codetype != NULL) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		ꦪꦼꦤ꧀ (lerror != lwc_error_ok)
			ꦮꦭꦶ false;

		ꦪꦼꦤ꧀ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ꦮꦭꦶ true;
		}

		lwc_string_unref(icodetype);
	}

	ꦪꦼꦤ꧀ (params->data != NULL && params->type != NULL) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		ꦪꦼꦤ꧀ (lerror != lwc_error_ok)
			ꦮꦭꦶ false;

		ꦪꦼꦤ꧀ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ꦮꦭꦶ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_node_get_first_child(n, &c);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR)
		ꦮꦭꦶ false;

	ꦱꦼꦢꦼꦭ (c != NULL) {
		dom_node *next;
		dom_node_type type;

		err = dom_node_get_node_type(c, &type);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			ꦮꦭꦶ false;
		}

		ꦪꦼꦤ꧀ (type == DOM_ELEMENT_NODE) {
			dom_string *name;

			err = dom_node_get_node_name(c, &name);
			ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_node_unref(c);
				bha_nga;
			}
			dom_string_unref(name);

			param = talloc(params, sa_nracanaa object_param);
			ꦪꦼꦤ꧀ (param == NULL) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}
			param->name = NULL;
			param->value = NULL;
			param->type = NULL;
			param->valuetype = NULL;
			param->next = NULL;

			ꦪꦼꦤ꧀ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (box_get_attribute(c, "value", param,
					&param->value) == false) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_node_unref(c);
				ꦮꦭꦶ false;
			}

			ꦪꦼꦤ꧀ (param->valuetype == NULL) {
				param->valuetype = talloc_strdup(param, "data");
				ꦪꦼꦤ꧀ (param->valuetype == NULL) {
					dom_node_unref(c);
					ꦮꦭꦶ false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_node_get_next_sibling(c, &next);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			ꦮꦭꦶ false;
		}

		dom_node_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ꦪꦼꦤ꧀ (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ꦮꦭꦶ false;

	*convert_children = false;
	ꦮꦭꦶ true;
}


/**
 * Preformatted text [9.3.4].
 */
static bool
box_pre(dom_node *n,
	html_content *content,
	sa_nracanaa box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ꦮꦭꦶ true;
}


/**
 * Option selector [17.6].
 */
static bool
box_select(dom_node *n,
	   html_content *content,
	   sa_nracanaa box *box,
	   bool *convert_children)
{
	sa_nracanaa box *inline_container;
	sa_nracanaa box *inline_box;
	sa_nracanaa form_control *gadget;
	dom_node *c, *c2;
	dom_node *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ꦪꦼꦤ꧀ (gadget == NULL)
		ꦮꦭꦶ false;

	gadget->html = content;
	err = dom_node_get_first_child(n, &c);
	ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ꦮꦭꦶ false;
	}

	ꦱꦼꦢꦼꦭ (c != NULL) {
		dom_string *name;

		err = dom_node_get_node_name(c, &name);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			ꦮꦭꦶ false;
		}

		ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ꦪꦼꦤ꧀ (box_select_add_option(gadget, c) == false) {
				dom_node_unref(c);
				form_free_control(gadget);
				ꦮꦭꦶ false;
			}
		} ꦭꦶꦪ ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_node_get_first_child(c, &c2);
			ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				form_free_control(gadget);
				ꦮꦭꦶ false;
			}

			ꦱꦼꦢꦼꦭ (c2 != NULL) {
				dom_string *c2_name;

				err = dom_node_get_node_name(c2, &c2_name);
				ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					ꦮꦭꦶ false;
				}

				ꦪꦼꦤ꧀ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ꦪꦼꦤ꧀ (box_select_add_option(gadget,
							c2) == false) {
						dom_node_unref(c2);
						dom_node_unref(c);
						form_free_control(gadget);
						ꦮꦭꦶ false;
					}
				} ꦭꦶꦪ {
					dom_string_unref(c2_name);
				}

				err = dom_node_get_next_sibling(c2, &next2);
				ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					ꦮꦭꦶ false;
				}

				dom_node_unref(c2);
				c2 = next2;
			}
		} ꦭꦶꦪ {
			dom_string_unref(name);
		}

		err = dom_node_get_next_sibling(c, &next);
		ꦪꦼꦤ꧀ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			ꦮꦭꦶ false;
		}

		dom_node_unref(c);
		c = next;
	}

	ꦪꦼꦤ꧀ (gadget->data.select.num_items == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ꦮꦭꦶ true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(NULL, 0, false, 0, 0, 0, 0, content->bctx);
	ꦪꦼꦤ꧀ (inline_container == NULL)
		goto no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(NULL, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ꦪꦼꦤ꧀ (inline_box == NULL)
		goto no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ꦪꦼꦤ꧀ (gadget->data.select.multiple == false &&
			gadget->data.select.num_selected == 0) {
		gadget->data.select.current = gadget->data.select.items;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.num_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->node, true);
	}

	ꦪꦼꦤ꧀ (gadget->data.select.num_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ꦭꦶꦪ ꦪꦼꦤ꧀ (gadget->data.select.num_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	ꦭꦶꦪ
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ꦪꦼꦤ꧀ (inline_box->text == NULL)
		goto no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	ꦮꦭꦶ true;

no_memory:
	ꦮꦭꦶ false;
}


/**
 * Multi-line text field [17.7].
 */
static bool box_textarea(dom_node *n,
			html_content *content,
			sa_nracanaa box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_node(content->forms, n);
	ꦪꦼꦤ꧀ (box->gadget == NULL)
		ꦮꦭꦶ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ꦪꦼꦤ꧀ (!box_input_text(content, box, n))
		ꦮꦭꦶ false;

	*convert_children = false;
	ꦮꦭꦶ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_node *node,
			 html_content *content,
			 sa_nracanaa box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(node, &tag_type);
	ꦪꦼꦤ꧀ (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	switch (tag_type) {
	pak_sa DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_INPUT:
		res = box_input(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(node, content, box, convert_children);
		bha_nga;

	pak_sa DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(node, content, box, convert_children);
		bha_nga;

	default:
		res = true;
	}

	ꦮꦭꦶ res;
}
