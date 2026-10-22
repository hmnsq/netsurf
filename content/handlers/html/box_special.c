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
static inline bool box_is_root(dom_node *n)
{
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	если (err != DOM_NO_ERR)
		вернуть false;

	если (parent != NULL) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		если (err != DOM_NO_ERR)
			вернуть false;

		если (type != DOM_DOCUMENT_NODE)
			вернуть false;
	}

	вернуть true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static целое box_object_talloc_destructor(struct object_params *o)
{
	если (o->codebase != NULL)
		nsurl_unref(o->codebase);
	если (o->classid != NULL)
		nsurl_unref(o->classid);
	если (o->data != NULL)
		nsurl_unref(o->data);

	вернуть 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
static struct frame_dimension *
box_parse_multi_lengths(const dom_string *ds, unsigned целое *count)
{
	char *end;
	unsigned целое i, n;
	struct frame_dimension *length;
	const char *s;

	s = dom_string_data(ds);

	для (i = 0, n = 1; s[i]; i++)
		если (s[i] == ',')
			n++;

	length = calloc(n, sizeof(struct frame_dimension));
	если (!length)
		вернуть NULL;

	для (i = 0; i != n; i++) {
		пока (ascii_is_space(*s)) {
			s++;
		}
		length[i].value = strtof(s, &end);
		если (length[i].value <= 0) {
			length[i].value = 1;
		}
		s = end;
		switch (*s) {
			case '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				break;
			case '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				break;
			default:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				break;
		}
		пока (*s && *s != ',') {
			s++;
		}
		если (*s == ',') {
			s++;
		}
	}

	*count = n;
	вернуть length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static целое box_frames_talloc_destructor(struct content_html_frames *f)
{
	если (f->url != NULL) {
		nsurl_unref(f->url);
		f->url = NULL;
	}

	вернуть 0;
}


/**
 * create a frameset box tree
 */
static bool
box_create_frameset(struct content_html_frames *f,
		    dom_node *n,
		    html_content *content)
{
	unsigned целое row, col, index, i;
	unsigned целое rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	struct frame_dimension *row_height = 0, *col_width = 0;
	dom_node *c, *next;
	struct content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		если (row_height == NULL)
			вернуть false;
	} иначе {
		row_height = calloc(1, sizeof(struct frame_dimension));
		если (row_height == NULL)
			вернуть false;
		row_height->value = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		если (col_width == NULL) {
			free(row_height);
			вернуть false;
		}
	} иначе {
		col_width = calloc(1, sizeof(struct frame_dimension));
		если (col_width == NULL) {
			free(row_height);
			вернуть false;
		}
		col_width->value = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		если ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		css_color color;

		если (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, struct content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	для (row = 0; row < rows; row++) {
		для (col = 0; col < cols; col++) {
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
	если (err != DOM_NO_ERR)
		вернуть false;

	для (row = 0; c != NULL && row < rows; row++) {
		для (col = 0; c != NULL && col < cols; col++) {
			пока (c != NULL) {
				dom_node_type type;
				dom_string *name;

				err = dom_node_get_node_type(c, &type);
				если (err != DOM_NO_ERR) {
					dom_node_unref(c);
					вернуть false;
				}

				err = dom_node_get_node_name(c, &name);
				если (err != DOM_NO_ERR) {
					dom_node_unref(c);
					вернуть false;
				}

				если (type != DOM_ELEMENT_NODE ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_node_get_next_sibling(c,
							&next);
					если (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_node_unref(c);
						вернуть false;
					}

					dom_string_unref(name);
					dom_node_unref(c);
					c = next;
				} иначе {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					break;
				}
			}

			если (c == NULL)
				break;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_node_get_node_name(c, &s);
			если (err != DOM_NO_ERR) {
				dom_node_unref(c);
				вернуть false;
			}

			если (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				если (box_create_frameset(frame, c,
						content) == false) {
					dom_node_unref(c);
					вернуть false;
				}

				err = dom_node_get_next_sibling(c, &next);
				если (err != DOM_NO_ERR) {
					dom_node_unref(c);
					вернуть false;
				}

				dom_node_unref(c);
				c = next;
				continue;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = NULL;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			если (err == DOM_NO_ERR && s != NULL) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			если (url != NULL) {
				/* no self-references */
				если (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = NULL;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			если (err == DOM_NO_ERR && s != NULL) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			если (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			если (err == DOM_NO_ERR && s != NULL) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			если (err == DOM_NO_ERR && s != NULL) {
				если (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				иначе если (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			если (err == DOM_NO_ERR && s != NULL) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			если (err == DOM_NO_ERR && s != NULL) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			если (err == DOM_NO_ERR && s != NULL) {
				css_color color;

				если (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_node_get_next_sibling(c, &next);
			если (err != DOM_NO_ERR) {
				dom_node_unref(c);
				вернуть false;
			}

			dom_node_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	если (c != NULL) {
		dom_node_unref(c);
	}

	вернуть true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
static целое box_iframes_talloc_destructor(struct content_html_iframe *f)
{
	если (f->url != NULL) {
		nsurl_unref(f->url);
		f->url = NULL;
	}

	вернуть 0;
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
		  const char *attribute,
		  void *context,
		  char **value)
{
	char *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((const uint8_t *) attribute,
			strlen(attribute), &attr_name);
	если (err != DOM_NO_ERR)
		вернуть false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	если (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		вернуть false;
	}

	dom_string_unref(attr_name);

	если (attr != NULL) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		если (result == NULL)
			вернуть false;

		*value = result;
	}

	вернуть true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
static bool
box_input_text(html_content *html, struct box *box, struct dom_node *node)
{
	struct box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	switch (display) {
	case CSS_DISPLAY_GRID:
	case CSS_DISPLAY_FLEX:
	case CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		break;
	default:
		box->type = BOX_INLINE_BLOCK;
		break;
	}

	inline_container = box_create(NULL, 0, false, 0, 0, 0, 0, html->bctx);
	если (!inline_container)
		вернуть false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(NULL, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	если (!inline_box)
		вернуть false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	вернуть box_textarea_create_textarea(html, box, node);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
static bool box_select_add_option(struct form_control *control, dom_node *n)
{
	char *value = NULL;
	char *text = NULL;
	char *text_nowrap = NULL;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_node_get_text_content(n, &content);
	если (err != DOM_NO_ERR)
		вернуть false;

	если (content != NULL) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} иначе {
		text = strdup("");
	}

	если (text == NULL)
		goto no_memory;

	err = dom_element_get_attribute(n, corestring_dom_value, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		value = strdup(dom_string_data(s));
		dom_string_unref(s);
	} иначе {
		value = strdup(text);
	}

	если (value == NULL)
		goto no_memory;

	если (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	если (text_nowrap == NULL)
		goto no_memory;

	если (form_add_option(control, value, text_nowrap, selected, n) == false)
		goto no_memory;

	free(text);

	вернуть true;

no_memory:
	free(value);
	free(text);
	free(text_nowrap);
	вернуть false;
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
      struct box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		если (!ok)
			вернуть false;
		если (url) {
			если (box->href != NULL)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		если (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		иначе если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__top))
			box->target = "_top";
		иначе если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		иначе если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		иначе {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			если (!box->target) {
				dom_string_unref(s);
				вернуть false;
			}
		}
		dom_string_unref(s);
	}

	вернуть true;
}


/**
 * Document body special element handler [7.5.1].
 */
static bool
box_body(dom_node *n,
	 html_content *content,
	 struct box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	если (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} иначе {
		content->background_colour = nscss_color_to_ns(color);
	}

	вернуть true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
static bool
box_br(dom_node *n,
       html_content *content,
       struct box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	вернуть true;
}


/**
 * special element handler for Push button [17.5].
 */
static bool
box_button(dom_node *n,
	   html_content *content,
	   struct box *box,
	   bool *convert_children)
{
	struct form_control *gadget;

	gadget = html_forms_get_control_for_node(content->forms, n);
	если (!gadget)
		вернуть false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	вернуть true;
}


/**
 * Canvas element
 */
static bool
box_canvas(dom_node *n,
	     html_content *content,
	     struct box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	если (!content->enable_scripting) {
		вернуть true;
	}
	*convert_children = false;

	если (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		вернуть true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	вернуть true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
static bool
box_embed(dom_node *n,
	  html_content *content,
	  struct box *box,
	  bool *convert_children)
{
	struct object_params *params;
	struct object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t num_attrs;
	dom_string *src;
	dom_exception err;

	если (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		вернуть true;

	params = talloc(content->bctx, struct object_params);
	если (params == NULL)
		вернуть false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = NULL;
	params->type = NULL;
	params->codetype = NULL;
	params->codebase = NULL;
	params->classid = NULL;
	params->params = NULL;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	если (err != DOM_NO_ERR || src == NULL)
		вернуть true;
	если (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		вернуть false;
	}

	dom_string_unref(src);

	если (params->data == NULL)
		вернуть true;

	/* Don't include ourself */
	если (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		вернуть true;

	/* add attributes as parameters to linked list */
	err = dom_node_get_attributes(n, &attrs);
	если (err != DOM_NO_ERR)
		вернуть false;

	err = dom_namednodemap_get_length(attrs, &num_attrs);
	если (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		вернуть false;
	}

	для (idx = 0; idx < num_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *value;

		err = dom_namednodemap_item(attrs, idx, (void *) &attr);
		если (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			вернуть false;
		}

		err = dom_attr_get_name(attr, &name);
		если (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_namednodemap_unref(attrs);
			вернуть false;
		}

		если (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_node_unref(attr);
			dom_string_unref(name);
			continue;
		}

		err = dom_attr_get_value(attr, &value);
		если (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			вернуть false;
		}

		param = talloc(content->bctx, struct object_param);
		если (param == NULL) {
			dom_node_unref(attr);
			dom_string_unref(value);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			вернуть false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->value = talloc_strdup(content->bctx, dom_string_data(value));
		param->type = NULL;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = NULL;

		dom_string_unref(value);
		dom_string_unref(name);
		dom_node_unref(attr);

		если (param->name == NULL || param->value == NULL ||
				param->valuetype == NULL) {
			dom_namednodemap_unref(attrs);
			вернуть false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	вернуть html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
static bool
box_frameset(dom_node *n,
	     html_content *content,
	     struct box *box,
	     bool *convert_children)
{
	bool ok;

	если (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		если (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		вернуть true;
	}

	content->frameset = talloc_zero(content->bctx,
					struct content_html_frames);
	если (!content->frameset) {
		вернуть false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	если (ok) {
		box->type = BOX_NONE;
	}

	если (convert_children) {
		*convert_children = false;
	}
	вернуть ok;
}


/**
 * Inline subwindow [16.5].
 */
static bool
box_iframe(dom_node *n,
	   html_content *content,
	   struct box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	struct content_html_iframe *iframe;
	целое i;

	если (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		вернуть true;

	если (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		вернуть true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	если (err != DOM_NO_ERR || s == NULL)
		вернуть true;
	если (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		вернуть false;
	}
	dom_string_unref(s);
	если (url == NULL)
		вернуть true;

	/* don't include ourself */
	если (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		вернуть true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, struct content_html_iframe);
	если (iframe == NULL) {
		nsurl_unref(url);
		вернуть false;
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
	если (err == DOM_NO_ERR && s != NULL) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		css_color color;

		если (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		иначе если (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	assert(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	если (convert_children)
		*convert_children = false;
	вернуть true;
}


/**
 * Embedded image [13.2].
 */
static bool
box_image(dom_node *n,
	  html_content *content,
	  struct box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	enum css_width_e wtype;
	enum css_height_e htype;
	css_fixed value = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	если (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		вернуть true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	если (err == DOM_NO_ERR && s != NULL) {
		char *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		если (alt == NULL)
			вернуть false;
		box->text = talloc_strdup(content->bctx, alt);
		free(alt);
		если (box->text == NULL)
			вернуть false;
		box->length = strlen(box->text);
	}

	если (nsoption_bool(foreground_images) == false) {
		вернуть true;
	}

	/* imagemap associated with this image */
	если (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		вернуть false;
	если (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	если (err != DOM_NO_ERR || s == NULL)
		вернуть true;

	если (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		вернуть false;
	}

	dom_string_unref(s);

	если (url == NULL)
		вернуть true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &value, &wunit);
	htype = css_computed_height(box->style, &value, &hunit);

	если (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	вернуть ok;
}


/**
 * Form control [17.4].
 */
static bool
box_input(dom_node *n,
	  html_content *content,
	  struct box *box,
	  bool *convert_children)
{
	struct form_control *gadget;
	dom_string *type = NULL;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_node(content->forms, n);
	если (gadget == NULL) {
		вернуть false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	если ((err != DOM_NO_ERR) || (type == NULL)) {
		/* no type so "text" is assumed */
		если (box_input_text(content, box, n) == false) {
			вернуть false;
		}
		*convert_children = false;
		вернуть true;
	}

	если (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		если (box_input_text(content, box, n) == false)
			goto no_memory;

	} иначе если (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} иначе если (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} иначе если ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} иначе если (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		struct box *inline_container, *inline_box;

		если (box_button(n, content, box, 0) == false)
			goto no_memory;

		inline_container = box_create(NULL, 0, false, 0, 0, 0, 0,
				content->bctx);
		если (inline_container == NULL)
			goto no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(NULL, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		если (inline_box == NULL)
			goto no_memory;

		inline_box->type = BOX_TEXT;

		если (box->gadget->value != NULL)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->value);
		иначе если (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		иначе если (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		иначе
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		если (inline_box->text == NULL)
			goto no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} иначе если (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		если (box->style && ns_computed_display(box->style,
				box_is_root(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			если (err == DOM_NO_ERR && s != NULL) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				если (error != NSERROR_OK)
					goto no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				если (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					если (!html_fetch_object(content,
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
	} иначе {
		/* unhandled type the default is "text" */
		если (box_input_text(content, box, n) == false)
			goto no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	вернуть true;

no_memory:
	dom_string_unref(type);

	вернуть false;
}


/**
 * Noscript element
 */
static bool
box_noscript(dom_node *n,
	     html_content *content,
	     struct box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	если (content->enable_scripting) {
		*convert_children = false;
	}

	вернуть true;
}


/**
 * Generic embedded object [13.3].
 */
static bool
box_object(dom_node *n,
	   html_content *content,
	   struct box *box,
	   bool *convert_children)
{
	struct object_params *params;
	struct object_param *param;
	dom_string *codebase, *classid, *data;
	dom_node *c;
	dom_exception err;

	если (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		вернуть true;

	если (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		вернуть false;
	если (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, struct object_params);
	если (params == NULL)
		вернуть false;

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
	если (err == DOM_NO_ERR && codebase != NULL) {
		если (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			вернуть false;
		}
		dom_string_unref(codebase);
	}
	если (params->codebase == NULL)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	если (err == DOM_NO_ERR && classid != NULL) {
		если (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			вернуть false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	если (err == DOM_NO_ERR && data != NULL) {
		если (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			вернуть false;
		}
		dom_string_unref(data);
	}

	если (params->classid == NULL && params->data == NULL)
		/* nothing to embed; ignore */
		вернуть true;

	/* Don't include ourself */
	если (params->classid != NULL && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		вернуть true;

	если (params->data != NULL && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		вернуть true;

	/* codetype and type are MIME types */
	если (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		вернуть false;
	если (box_get_attribute(n, "type", params, &params->type) == false)
		вернуть false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	если (params->classid != NULL && params->data == NULL &&
			params->codetype != NULL) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		если (lerror != lwc_error_ok)
			вернуть false;

		если (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			вернуть true;
		}

		lwc_string_unref(icodetype);
	}

	если (params->data != NULL && params->type != NULL) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		если (lerror != lwc_error_ok)
			вернуть false;

		если (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			вернуть true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_node_get_first_child(n, &c);
	если (err != DOM_NO_ERR)
		вернуть false;

	пока (c != NULL) {
		dom_node *next;
		dom_node_type type;

		err = dom_node_get_node_type(c, &type);
		если (err != DOM_NO_ERR) {
			dom_node_unref(c);
			вернуть false;
		}

		если (type == DOM_ELEMENT_NODE) {
			dom_string *name;

			err = dom_node_get_node_name(c, &name);
			если (err != DOM_NO_ERR) {
				dom_node_unref(c);
				вернуть false;
			}

			если (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_node_unref(c);
				break;
			}
			dom_string_unref(name);

			param = talloc(params, struct object_param);
			если (param == NULL) {
				dom_node_unref(c);
				вернуть false;
			}
			param->name = NULL;
			param->value = NULL;
			param->type = NULL;
			param->valuetype = NULL;
			param->next = NULL;

			если (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_node_unref(c);
				вернуть false;
			}

			если (box_get_attribute(c, "value", param,
					&param->value) == false) {
				dom_node_unref(c);
				вернуть false;
			}

			если (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_node_unref(c);
				вернуть false;
			}

			если (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_node_unref(c);
				вернуть false;
			}

			если (param->valuetype == NULL) {
				param->valuetype = talloc_strdup(param, "data");
				если (param->valuetype == NULL) {
					dom_node_unref(c);
					вернуть false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_node_get_next_sibling(c, &next);
		если (err != DOM_NO_ERR) {
			dom_node_unref(c);
			вернуть false;
		}

		dom_node_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	если (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		вернуть false;

	*convert_children = false;
	вернуть true;
}


/**
 * Preformatted text [9.3.4].
 */
static bool
box_pre(dom_node *n,
	html_content *content,
	struct box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	вернуть true;
}


/**
 * Option selector [17.6].
 */
static bool
box_select(dom_node *n,
	   html_content *content,
	   struct box *box,
	   bool *convert_children)
{
	struct box *inline_container;
	struct box *inline_box;
	struct form_control *gadget;
	dom_node *c, *c2;
	dom_node *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_node(content->forms, n);
	если (gadget == NULL)
		вернуть false;

	gadget->html = content;
	err = dom_node_get_first_child(n, &c);
	если (err != DOM_NO_ERR) {
		form_free_control(gadget);
		вернуть false;
	}

	пока (c != NULL) {
		dom_string *name;

		err = dom_node_get_node_name(c, &name);
		если (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			вернуть false;
		}

		если (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			если (box_select_add_option(gadget, c) == false) {
				dom_node_unref(c);
				form_free_control(gadget);
				вернуть false;
			}
		} иначе если (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_node_get_first_child(c, &c2);
			если (err != DOM_NO_ERR) {
				dom_node_unref(c);
				form_free_control(gadget);
				вернуть false;
			}

			пока (c2 != NULL) {
				dom_string *c2_name;

				err = dom_node_get_node_name(c2, &c2_name);
				если (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					вернуть false;
				}

				если (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					если (box_select_add_option(gadget,
							c2) == false) {
						dom_node_unref(c2);
						dom_node_unref(c);
						form_free_control(gadget);
						вернуть false;
					}
				} иначе {
					dom_string_unref(c2_name);
				}

				err = dom_node_get_next_sibling(c2, &next2);
				если (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					вернуть false;
				}

				dom_node_unref(c2);
				c2 = next2;
			}
		} иначе {
			dom_string_unref(name);
		}

		err = dom_node_get_next_sibling(c, &next);
		если (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			вернуть false;
		}

		dom_node_unref(c);
		c = next;
	}

	если (gadget->data.select.num_items == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		вернуть true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(NULL, 0, false, 0, 0, 0, 0, content->bctx);
	если (inline_container == NULL)
		goto no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(NULL, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	если (inline_box == NULL)
		goto no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	если (gadget->data.select.multiple == false &&
			gadget->data.select.num_selected == 0) {
		gadget->data.select.current = gadget->data.select.items;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.num_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->node, true);
	}

	если (gadget->data.select.num_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	иначе если (gadget->data.select.num_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	иначе
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	если (inline_box->text == NULL)
		goto no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	вернуть true;

no_memory:
	вернуть false;
}


/**
 * Multi-line text field [17.7].
 */
static bool box_textarea(dom_node *n,
			html_content *content,
			struct box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_node(content->forms, n);
	если (box->gadget == NULL)
		вернуть false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	если (!box_input_text(content, box, n))
		вернуть false;

	*convert_children = false;
	вернуть true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_node *node,
			 html_content *content,
			 struct box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(node, &tag_type);
	если (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	switch (tag_type) {
	case DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_INPUT:
		res = box_input(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(node, content, box, convert_children);
		break;

	case DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(node, content, box, convert_children);
		break;

	default:
		res = true;
	}

	вернуть res;
}
