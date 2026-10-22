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

#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <dom/dom.h>

#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/talloc.h"
#ضمن "utils/string.h"
#ضمن "utils/ascii.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/plot_style.h"
#ضمن "css/hints.h"
#ضمن "desktop/frame_types.h"
#ضمن "content/content_factory.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/object.h"
#ضمن "html/box.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/box_construct.h"
#ضمن "html/box_special.h"
#ضمن "html/box_textarea.h"
#ضمن "html/form_internal.h"


ساكن ثابت content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ساكن مضمن bool box_is_root(dom_node *n)
{
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	err = dom_node_get_parent_node(n, &parent);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (parent != عدم) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		ܐܢ (err != DOM_NO_ERR)
			ܦܢܐ false;

		ܐܢ (type != DOM_DOCUMENT_NODE)
			ܦܢܐ false;
	}

	ܦܢܐ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_object_talloc_destructor(بنية object_params *o)
{
	ܐܢ (o->codebase != عدم)
		nsurl_unref(o->codebase);
	ܐܢ (o->classid != عدم)
		nsurl_unref(o->classid);
	ܐܢ (o->data != عدم)
		nsurl_unref(o->data);

	ܦܢܐ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ساكن بنية frame_dimension *
box_parse_multi_lengths(ثابت dom_string *ds, unsigned صحيح *count)
{
	حرف *end;
	unsigned صحيح i, n;
	بنية frame_dimension *length;
	ثابت حرف *s;

	s = dom_string_data(ds);

	لكل (i = 0, n = 1; s[i]; i++)
		ܐܢ (s[i] == ',')
			n++;

	length = calloc(n, حجم(بنية frame_dimension));
	ܐܢ (!length)
		ܦܢܐ عدم;

	لكل (i = 0; i != n; i++) {
		ܟܡܐ (ascii_is_space(*s)) {
			s++;
		}
		length[i].value = strtof(s, &end);
		ܐܢ (length[i].value <= 0) {
			length[i].value = 1;
		}
		s = end;
		بدل (*s) {
			حالة '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				اكسر;
			حالة '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				اكسر;
			افتراضي:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				اكسر;
		}
		ܟܡܐ (*s && *s != ',') {
			s++;
		}
		ܐܢ (*s == ',') {
			s++;
		}
	}

	*count = n;
	ܦܢܐ length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_frames_talloc_destructor(بنية content_html_frames *f)
{
	ܐܢ (f->url != عدم) {
		nsurl_unref(f->url);
		f->url = عدم;
	}

	ܦܢܐ 0;
}


/**
 * create a frameset box tree
 */
ساكن bool
box_create_frameset(بنية content_html_frames *f,
		    dom_node *n,
		    html_content *content)
{
	unsigned صحيح row, col, index, i;
	unsigned صحيح rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	بنية frame_dimension *row_height = 0, *col_width = 0;
	dom_node *c, *next;
	بنية content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		ܐܢ (row_height == عدم)
			ܦܢܐ false;
	} ܐܚܪܢܐ {
		row_height = calloc(1, حجم(بنية frame_dimension));
		ܐܢ (row_height == عدم)
			ܦܢܐ false;
		row_height->value = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		ܐܢ (col_width == عدم) {
			حرر(row_height);
			ܦܢܐ false;
		}
	} ܐܚܪܢܐ {
		col_width = calloc(1, حجم(بنية frame_dimension));
		ܐܢ (col_width == عدم) {
			حرر(row_height);
			ܦܢܐ false;
		}
		col_width->value = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		ܐܢ ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		css_color color;

		ܐܢ (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, بنية content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	لكل (row = 0; row < rows; row++) {
		لكل (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = عدم;
			frame->url = عدم;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = عدم;
		}
	}
	حرر(col_width);
	حرر(row_height);

	/* create the frameset windows */
	err = dom_node_get_first_child(n, &c);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	لكل (row = 0; c != عدم && row < rows; row++) {
		لكل (col = 0; c != عدم && col < cols; col++) {
			ܟܡܐ (c != عدم) {
				dom_node_type type;
				dom_string *name;

				err = dom_node_get_node_type(c, &type);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ܦܢܐ false;
				}

				err = dom_node_get_node_name(c, &name);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ܦܢܐ false;
				}

				ܐܢ (type != DOM_ELEMENT_NODE ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_node_get_next_sibling(c,
							&next);
					ܐܢ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_node_unref(c);
						ܦܢܐ false;
					}

					dom_string_unref(name);
					dom_node_unref(c);
					c = next;
				} ܐܚܪܢܐ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					اكسر;
				}
			}

			ܐܢ (c == عدم)
				اكسر;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_node_get_node_name(c, &s);
			ܐܢ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				ܐܢ (box_create_frameset(frame, c,
						content) == false) {
					dom_node_unref(c);
					ܦܢܐ false;
				}

				err = dom_node_get_next_sibling(c, &next);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(c);
					ܦܢܐ false;
				}

				dom_node_unref(c);
				c = next;
				استمر;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = عدم;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ܐܢ (url != عدم) {
				/* no self-references */
				ܐܢ (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = عدم;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			ܐܢ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				ܐܢ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				css_color color;

				ܐܢ (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_node_get_next_sibling(c, &next);
			ܐܢ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			dom_node_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ܐܢ (c != عدم) {
		dom_node_unref(c);
	}

	ܦܢܐ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_iframes_talloc_destructor(بنية content_html_iframe *f)
{
	ܐܢ (f->url != عدم) {
		nsurl_unref(f->url);
		f->url = عدم;
	}

	ܦܢܐ 0;
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
ساكن bool
box_get_attribute(dom_node *n,
		  ثابت حرف *attribute,
		  فراغ *context,
		  حرف **value)
{
	حرف *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ثابت uint8_t *) attribute,
			strlen(attribute), &attr_name);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ܐܢ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ܦܢܐ false;
	}

	dom_string_unref(attr_name);

	ܐܢ (attr != عدم) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		ܐܢ (result == عدم)
			ܦܢܐ false;

		*value = result;
	}

	ܦܢܐ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ساكن bool
box_input_text(html_content *html, بنية box *box, بنية dom_node *node)
{
	بنية box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	بدل (display) {
	حالة CSS_DISPLAY_GRID:
	حالة CSS_DISPLAY_FLEX:
	حالة CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		اكسر;
	افتراضي:
		box->type = BOX_INLINE_BLOCK;
		اكسر;
	}

	inline_container = box_create(عدم, 0, false, 0, 0, 0, 0, html->bctx);
	ܐܢ (!inline_container)
		ܦܢܐ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(عدم, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ܐܢ (!inline_box)
		ܦܢܐ false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ܦܢܐ box_textarea_create_textarea(html, box, node);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ساكن bool box_select_add_option(بنية form_control *control, dom_node *n)
{
	حرف *value = عدم;
	حرف *text = عدم;
	حرف *text_nowrap = عدم;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_node_get_text_content(n, &content);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܐܢ (content != عدم) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} ܐܚܪܢܐ {
		text = strdup("");
	}

	ܐܢ (text == عدم)
		اذهب no_memory;

	err = dom_element_get_attribute(n, corestring_dom_value, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		value = strdup(dom_string_data(s));
		dom_string_unref(s);
	} ܐܚܪܢܐ {
		value = strdup(text);
	}

	ܐܢ (value == عدم)
		اذهب no_memory;

	ܐܢ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	ܐܢ (text_nowrap == عدم)
		اذهب no_memory;

	ܐܢ (form_add_option(control, value, text_nowrap, selected, n) == false)
		اذهب no_memory;

	حرر(text);

	ܦܢܐ true;

no_memory:
	حرر(value);
	حرر(text);
	حرر(text_nowrap);
	ܦܢܐ false;
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
ساكن bool
box_a(dom_node *n,
      html_content *content,
      بنية box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		ܐܢ (!ok)
			ܦܢܐ false;
		ܐܢ (url) {
			ܐܢ (box->href != عدم)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ܐܢ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__top))
			box->target = "_top";
		ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		ܐܚܪܢܐ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			ܐܢ (!box->target) {
				dom_string_unref(s);
				ܦܢܐ false;
			}
		}
		dom_string_unref(s);
	}

	ܦܢܐ true;
}


/**
 * Document body special element handler [7.5.1].
 */
ساكن bool
box_body(dom_node *n,
	 html_content *content,
	 بنية box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ܐܢ (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} ܐܚܪܢܐ {
		content->background_colour = nscss_color_to_ns(color);
	}

	ܦܢܐ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ساكن bool
box_br(dom_node *n,
       html_content *content,
       بنية box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ܦܢܐ true;
}


/**
 * special element handler for Push button [17.5].
 */
ساكن bool
box_button(dom_node *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية form_control *gadget;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ܐܢ (!gadget)
		ܦܢܐ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	ܦܢܐ true;
}


/**
 * Canvas element
 */
ساكن bool
box_canvas(dom_node *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ܐܢ (!content->enable_scripting) {
		ܦܢܐ true;
	}
	*convert_children = false;

	ܐܢ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ܦܢܐ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ܦܢܐ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ساكن bool
box_embed(dom_node *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	بنية object_params *params;
	بنية object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t num_attrs;
	dom_string *src;
	dom_exception err;

	ܐܢ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ܦܢܐ true;

	params = talloc(content->bctx, بنية object_params);
	ܐܢ (params == عدم)
		ܦܢܐ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = عدم;
	params->type = عدم;
	params->codetype = عدم;
	params->codebase = عدم;
	params->classid = عدم;
	params->params = عدم;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ܐܢ (err != DOM_NO_ERR || src == عدم)
		ܦܢܐ true;
	ܐܢ (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		ܦܢܐ false;
	}

	dom_string_unref(src);

	ܐܢ (params->data == عدم)
		ܦܢܐ true;

	/* Don't include ourself */
	ܐܢ (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		ܦܢܐ true;

	/* add attributes as parameters to linked list */
	err = dom_node_get_attributes(n, &attrs);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	err = dom_namednodemap_get_length(attrs, &num_attrs);
	ܐܢ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ܦܢܐ false;
	}

	لكل (idx = 0; idx < num_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *value;

		err = dom_namednodemap_item(attrs, idx, (فراغ *) &attr);
		ܐܢ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ܦܢܐ false;
		}

		err = dom_attr_get_name(attr, &name);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_namednodemap_unref(attrs);
			ܦܢܐ false;
		}

		ܐܢ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_node_unref(attr);
			dom_string_unref(name);
			استمر;
		}

		err = dom_attr_get_value(attr, &value);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ܦܢܐ false;
		}

		param = talloc(content->bctx, بنية object_param);
		ܐܢ (param == عدم) {
			dom_node_unref(attr);
			dom_string_unref(value);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ܦܢܐ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->value = talloc_strdup(content->bctx, dom_string_data(value));
		param->type = عدم;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = عدم;

		dom_string_unref(value);
		dom_string_unref(name);
		dom_node_unref(attr);

		ܐܢ (param->name == عدم || param->value == عدم ||
				param->valuetype == عدم) {
			dom_namednodemap_unref(attrs);
			ܦܢܐ false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ܦܢܐ html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ساكن bool
box_frameset(dom_node *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	bool ok;

	ܐܢ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ܐܢ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ܦܢܐ true;
	}

	content->frameset = talloc_zero(content->bctx,
					بنية content_html_frames);
	ܐܢ (!content->frameset) {
		ܦܢܐ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ܐܢ (ok) {
		box->type = BOX_NONE;
	}

	ܐܢ (convert_children) {
		*convert_children = false;
	}
	ܦܢܐ ok;
}


/**
 * Inline subwindow [16.5].
 */
ساكن bool
box_iframe(dom_node *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	بنية content_html_iframe *iframe;
	صحيح i;

	ܐܢ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ܦܢܐ true;

	ܐܢ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ܦܢܐ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ܐܢ (err != DOM_NO_ERR || s == عدم)
		ܦܢܐ true;
	ܐܢ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ܦܢܐ false;
	}
	dom_string_unref(s);
	ܐܢ (url == عدم)
		ܦܢܐ true;

	/* don't include ourself */
	ܐܢ (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ܦܢܐ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, بنية content_html_iframe);
	ܐܢ (iframe == عدم) {
		nsurl_unref(url);
		ܦܢܐ false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = عدم;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		css_color color;

		ܐܢ (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	أكد(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ܐܢ (convert_children)
		*convert_children = false;
	ܦܢܐ true;
}


/**
 * Embedded image [13.2].
 */
ساكن bool
box_image(dom_node *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	تعداد css_width_e wtype;
	تعداد css_height_e htype;
	css_fixed value = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ܐܢ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ܦܢܐ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ܐܢ (err == DOM_NO_ERR && s != عدم) {
		حرف *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		ܐܢ (alt == عدم)
			ܦܢܐ false;
		box->text = talloc_strdup(content->bctx, alt);
		حرر(alt);
		ܐܢ (box->text == عدم)
			ܦܢܐ false;
		box->length = strlen(box->text);
	}

	ܐܢ (nsoption_bool(foreground_images) == false) {
		ܦܢܐ true;
	}

	/* imagemap associated with this image */
	ܐܢ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ܦܢܐ false;
	ܐܢ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ܐܢ (err != DOM_NO_ERR || s == عدم)
		ܦܢܐ true;

	ܐܢ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ܦܢܐ false;
	}

	dom_string_unref(s);

	ܐܢ (url == عدم)
		ܦܢܐ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &value, &wunit);
	htype = css_computed_height(box->style, &value, &hunit);

	ܐܢ (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ܦܢܐ ok;
}


/**
 * Form control [17.4].
 */
ساكن bool
box_input(dom_node *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	بنية form_control *gadget;
	dom_string *type = عدم;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ܐܢ (gadget == عدم) {
		ܦܢܐ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ܐܢ ((err != DOM_NO_ERR) || (type == عدم)) {
		/* no type so "text" is assumed */
		ܐܢ (box_input_text(content, box, n) == false) {
			ܦܢܐ false;
		}
		*convert_children = false;
		ܦܢܐ true;
	}

	ܐܢ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ܐܢ (box_input_text(content, box, n) == false)
			اذهب no_memory;

	} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ܐܚܪܢܐ ܐܢ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		بنية box *inline_container, *inline_box;

		ܐܢ (box_button(n, content, box, 0) == false)
			اذهب no_memory;

		inline_container = box_create(عدم, 0, false, 0, 0, 0, 0,
				content->bctx);
		ܐܢ (inline_container == عدم)
			اذهب no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(عدم, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ܐܢ (inline_box == عدم)
			اذهب no_memory;

		inline_box->type = BOX_TEXT;

		ܐܢ (box->gadget->value != عدم)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->value);
		ܐܚܪܢܐ ܐܢ (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ܐܚܪܢܐ ܐܢ (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ܐܚܪܢܐ
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		ܐܢ (inline_box->text == عدم)
			اذهب no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ܐܢ (box->style && ns_computed_display(box->style,
				box_is_root(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ܐܢ (err == DOM_NO_ERR && s != عدم) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				ܐܢ (error != NSERROR_OK)
					اذهب no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ܐܢ (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					ܐܢ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						اذهب no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} ܐܚܪܢܐ {
		/* unhandled type the default is "text" */
		ܐܢ (box_input_text(content, box, n) == false)
			اذهب no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	ܦܢܐ true;

no_memory:
	dom_string_unref(type);

	ܦܢܐ false;
}


/**
 * Noscript element
 */
ساكن bool
box_noscript(dom_node *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ܐܢ (content->enable_scripting) {
		*convert_children = false;
	}

	ܦܢܐ true;
}


/**
 * Generic embedded object [13.3].
 */
ساكن bool
box_object(dom_node *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية object_params *params;
	بنية object_param *param;
	dom_string *codebase, *classid, *data;
	dom_node *c;
	dom_exception err;

	ܐܢ (box->style && ns_computed_display(box->style,
			box_is_root(n)) == CSS_DISPLAY_NONE)
		ܦܢܐ true;

	ܐܢ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ܦܢܐ false;
	ܐܢ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, بنية object_params);
	ܐܢ (params == عدم)
		ܦܢܐ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = عدم;
	params->type = عدم;
	params->codetype = عدم;
	params->codebase = عدم;
	params->classid = عدم;
	params->params = عدم;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ܐܢ (err == DOM_NO_ERR && codebase != عدم) {
		ܐܢ (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ܦܢܐ false;
		}
		dom_string_unref(codebase);
	}
	ܐܢ (params->codebase == عدم)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ܐܢ (err == DOM_NO_ERR && classid != عدم) {
		ܐܢ (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ܦܢܐ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	ܐܢ (err == DOM_NO_ERR && data != عدم) {
		ܐܢ (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			ܦܢܐ false;
		}
		dom_string_unref(data);
	}

	ܐܢ (params->classid == عدم && params->data == عدم)
		/* nothing to embed; ignore */
		ܦܢܐ true;

	/* Don't include ourself */
	ܐܢ (params->classid != عدم && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		ܦܢܐ true;

	ܐܢ (params->data != عدم && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		ܦܢܐ true;

	/* codetype and type are MIME types */
	ܐܢ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ܦܢܐ false;
	ܐܢ (box_get_attribute(n, "type", params, &params->type) == false)
		ܦܢܐ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ܐܢ (params->classid != عدم && params->data == عدم &&
			params->codetype != عدم) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		ܐܢ (lerror != lwc_error_ok)
			ܦܢܐ false;

		ܐܢ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ܦܢܐ true;
		}

		lwc_string_unref(icodetype);
	}

	ܐܢ (params->data != عدم && params->type != عدم) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		ܐܢ (lerror != lwc_error_ok)
			ܦܢܐ false;

		ܐܢ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ܦܢܐ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_node_get_first_child(n, &c);
	ܐܢ (err != DOM_NO_ERR)
		ܦܢܐ false;

	ܟܡܐ (c != عدم) {
		dom_node *next;
		dom_node_type type;

		err = dom_node_get_node_type(c, &type);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			ܦܢܐ false;
		}

		ܐܢ (type == DOM_ELEMENT_NODE) {
			dom_string *name;

			err = dom_node_get_node_name(c, &name);
			ܐܢ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_node_unref(c);
				اكسر;
			}
			dom_string_unref(name);

			param = talloc(params, بنية object_param);
			ܐܢ (param == عدم) {
				dom_node_unref(c);
				ܦܢܐ false;
			}
			param->name = عدم;
			param->value = عدم;
			param->type = عدم;
			param->valuetype = عدم;
			param->next = عدم;

			ܐܢ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (box_get_attribute(c, "value", param,
					&param->value) == false) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_node_unref(c);
				ܦܢܐ false;
			}

			ܐܢ (param->valuetype == عدم) {
				param->valuetype = talloc_strdup(param, "data");
				ܐܢ (param->valuetype == عدم) {
					dom_node_unref(c);
					ܦܢܐ false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_node_get_next_sibling(c, &next);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			ܦܢܐ false;
		}

		dom_node_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ܐܢ (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ܦܢܐ false;

	*convert_children = false;
	ܦܢܐ true;
}


/**
 * Preformatted text [9.3.4].
 */
ساكن bool
box_pre(dom_node *n,
	html_content *content,
	بنية box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ܦܢܐ true;
}


/**
 * Option selector [17.6].
 */
ساكن bool
box_select(dom_node *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية box *inline_container;
	بنية box *inline_box;
	بنية form_control *gadget;
	dom_node *c, *c2;
	dom_node *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_node(content->forms, n);
	ܐܢ (gadget == عدم)
		ܦܢܐ false;

	gadget->html = content;
	err = dom_node_get_first_child(n, &c);
	ܐܢ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ܦܢܐ false;
	}

	ܟܡܐ (c != عدم) {
		dom_string *name;

		err = dom_node_get_node_name(c, &name);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			ܦܢܐ false;
		}

		ܐܢ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ܐܢ (box_select_add_option(gadget, c) == false) {
				dom_node_unref(c);
				form_free_control(gadget);
				ܦܢܐ false;
			}
		} ܐܚܪܢܐ ܐܢ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_node_get_first_child(c, &c2);
			ܐܢ (err != DOM_NO_ERR) {
				dom_node_unref(c);
				form_free_control(gadget);
				ܦܢܐ false;
			}

			ܟܡܐ (c2 != عدم) {
				dom_string *c2_name;

				err = dom_node_get_node_name(c2, &c2_name);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					ܦܢܐ false;
				}

				ܐܢ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ܐܢ (box_select_add_option(gadget,
							c2) == false) {
						dom_node_unref(c2);
						dom_node_unref(c);
						form_free_control(gadget);
						ܦܢܐ false;
					}
				} ܐܚܪܢܐ {
					dom_string_unref(c2_name);
				}

				err = dom_node_get_next_sibling(c2, &next2);
				ܐܢ (err != DOM_NO_ERR) {
					dom_node_unref(c2);
					dom_node_unref(c);
					form_free_control(gadget);
					ܦܢܐ false;
				}

				dom_node_unref(c2);
				c2 = next2;
			}
		} ܐܚܪܢܐ {
			dom_string_unref(name);
		}

		err = dom_node_get_next_sibling(c, &next);
		ܐܢ (err != DOM_NO_ERR) {
			dom_node_unref(c);
			form_free_control(gadget);
			ܦܢܐ false;
		}

		dom_node_unref(c);
		c = next;
	}

	ܐܢ (gadget->data.select.num_items == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ܦܢܐ true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(عدم, 0, false, 0, 0, 0, 0, content->bctx);
	ܐܢ (inline_container == عدم)
		اذهب no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(عدم, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ܐܢ (inline_box == عدم)
		اذهب no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ܐܢ (gadget->data.select.multiple == false &&
			gadget->data.select.num_selected == 0) {
		gadget->data.select.current = gadget->data.select.items;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.num_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->node, true);
	}

	ܐܢ (gadget->data.select.num_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ܐܚܪܢܐ ܐܢ (gadget->data.select.num_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	ܐܚܪܢܐ
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ܐܢ (inline_box->text == عدم)
		اذهب no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	ܦܢܐ true;

no_memory:
	ܦܢܐ false;
}


/**
 * Multi-line text field [17.7].
 */
ساكن bool box_textarea(dom_node *n,
			html_content *content,
			بنية box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_node(content->forms, n);
	ܐܢ (box->gadget == عدم)
		ܦܢܐ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ܐܢ (!box_input_text(content, box, n))
		ܦܢܐ false;

	*convert_children = false;
	ܦܢܐ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_node *node,
			 html_content *content,
			 بنية box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(node, &tag_type);
	ܐܢ (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	بدل (tag_type) {
	حالة DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_INPUT:
		res = box_input(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(node, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(node, content, box, convert_children);
		اكسر;

	افتراضي:
		res = true;
	}

	ܦܢܐ res;
}
