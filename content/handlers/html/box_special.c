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

#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <dom/dom.h>

#অন্তর্ভাব "utils/nsoption.h"
#অন্তর্ভাব "utils/corestrings.h"
#অন্তর্ভাব "utils/log.h"
#অন্তর্ভাব "utils/messages.h"
#অন্তর্ভাব "utils/talloc.h"
#অন্তর্ভাব "utils/string.h"
#অন্তর্ভাব "utils/ascii.h"
#অন্তর্ভাব "utils/nsurl.h"
#অন্তর্ভাব "netsurf/plot_style.h"
#অন্তর্ভাব "css/hints.h"
#অন্তর্ভাব "desktop/frame_types.h"
#অন্তর্ভাব "content/content_factory.h"

#অন্তর্ভাব "html/html.h"
#অন্তর্ভাব "html/private.h"
#অন্তর্ভাব "html/object.h"
#অন্তর্ভাব "html/box.h"
#অন্তর্ভাব "html/box_manipulate.h"
#অন্তর্ভাব "html/box_construct.h"
#অন্তর্ভাব "html/box_special.h"
#অন্তর্ভাব "html/box_textarea.h"
#অন্তর্ভাব "html/form_internal.h"


স্থির ধ্রুব content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
স্থির অন্তঃস্থ bool box_is_মূল(dom_গ্রন্থি *n)
{
	dom_গ্রন্থি *parent;
	dom_গ্রন্থি_type type;
	dom_exception err;

	err = dom_গ্রন্থি_get_parent_গ্রন্থি(n, &parent);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (parent != রিক্ত) {
		err = dom_গ্রন্থি_get_গ্রন্থি_type(parent, &type);

		dom_গ্রন্থি_unref(parent);

		যদি (err != DOM_NO_ERR)
			প্রত্যাগম false;

		যদি (type != DOM_DOCUMENT_গ্রন্থি)
			প্রত্যাগম false;
	}

	প্রত্যাগম true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
স্থির পূর্ণাঙ্ক box_object_talloc_destructor(সংরচনা object_params *o)
{
	যদি (o->codebase != রিক্ত)
		nsurl_unref(o->codebase);
	যদি (o->classid != রিক্ত)
		nsurl_unref(o->classid);
	যদি (o->data != রিক্ত)
		nsurl_unref(o->data);

	প্রত্যাগম 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
স্থির সংরচনা frame_dimension *
box_parse_multi_lengths(ধ্রুব dom_string *ds, unsigned পূর্ণাঙ্ক *গুনতি)
{
	বর্ণ *end;
	unsigned পূর্ণাঙ্ক i, n;
	সংরচনা frame_dimension *length;
	ধ্রুব বর্ণ *s;

	s = dom_string_data(ds);

	প্রতি (i = 0, n = 1; s[i]; i++)
		যদি (s[i] == ',')
			n++;

	length = calloc(n, পরিমাণ(সংরচনা frame_dimension));
	যদি (!length)
		প্রত্যাগম রিক্ত;

	প্রতি (i = 0; i != n; i++) {
		যাবত্ (ascii_is_space(*s)) {
			s++;
		}
		length[i].মান = strtof(s, &end);
		যদি (length[i].মান <= 0) {
			length[i].মান = 1;
		}
		s = end;
		বিকল্প (*s) {
			পক্ষ '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				ভঙ্গ;
			পক্ষ '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				ভঙ্গ;
			স্বভাব:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				ভঙ্গ;
		}
		যাবত্ (*s && *s != ',') {
			s++;
		}
		যদি (*s == ',') {
			s++;
		}
	}

	*গুনতি = n;
	প্রত্যাগম length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
স্থির পূর্ণাঙ্ক box_frames_talloc_destructor(সংরচনা content_html_frames *f)
{
	যদি (f->url != রিক্ত) {
		nsurl_unref(f->url);
		f->url = রিক্ত;
	}

	প্রত্যাগম 0;
}


/**
 * create a frameset box tree
 */
স্থির bool
box_create_frameset(সংরচনা content_html_frames *f,
		    dom_গ্রন্থি *n,
		    html_content *content)
{
	unsigned পূর্ণাঙ্ক row, col, index, i;
	unsigned পূর্ণাঙ্ক rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	সংরচনা frame_dimension *row_height = 0, *col_width = 0;
	dom_গ্রন্থি *c, *next;
	সংরচনা content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		যদি (row_height == রিক্ত)
			প্রত্যাগম false;
	} অন্যথা {
		row_height = calloc(1, পরিমাণ(সংরচনা frame_dimension));
		যদি (row_height == রিক্ত)
			প্রত্যাগম false;
		row_height->মান = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		যদি (col_width == রিক্ত) {
			মুক্ত(row_height);
			প্রত্যাগম false;
		}
	} অন্যথা {
		col_width = calloc(1, পরিমাণ(সংরচনা frame_dimension));
		যদি (col_width == রিক্ত) {
			মুক্ত(row_height);
			প্রত্যাগম false;
		}
		col_width->মান = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		যদি ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		css_color color;

		যদি (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, সংরচনা content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	প্রতি (row = 0; row < rows; row++) {
		প্রতি (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = রিক্ত;
			frame->url = রিক্ত;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = রিক্ত;
		}
	}
	মুক্ত(col_width);
	মুক্ত(row_height);

	/* create the frameset windows */
	err = dom_গ্রন্থি_get_first_child(n, &c);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	প্রতি (row = 0; c != রিক্ত && row < rows; row++) {
		প্রতি (col = 0; c != রিক্ত && col < cols; col++) {
			যাবত্ (c != রিক্ত) {
				dom_গ্রন্থি_type type;
				dom_string *name;

				err = dom_গ্রন্থি_get_গ্রন্থি_type(c, &type);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(c);
					প্রত্যাগম false;
				}

				err = dom_গ্রন্থি_get_গ্রন্থি_name(c, &name);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(c);
					প্রত্যাগম false;
				}

				যদি (type != DOM_ELEMENT_গ্রন্থি ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_গ্রন্থি_get_next_sibling(c,
							&next);
					যদি (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_গ্রন্থি_unref(c);
						প্রত্যাগম false;
					}

					dom_string_unref(name);
					dom_গ্রন্থি_unref(c);
					c = next;
				} অন্যথা {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ভঙ্গ;
				}
			}

			যদি (c == রিক্ত)
				ভঙ্গ;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_গ্রন্থি_get_গ্রন্থি_name(c, &s);
			যদি (err != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				যদি (box_create_frameset(frame, c,
						content) == false) {
					dom_গ্রন্থি_unref(c);
					প্রত্যাগম false;
				}

				err = dom_গ্রন্থি_get_next_sibling(c, &next);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(c);
					প্রত্যাগম false;
				}

				dom_গ্রন্থি_unref(c);
				c = next;
				অনুবর্ত;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = রিক্ত;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			যদি (url != রিক্ত) {
				/* no self-references */
				যদি (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = রিক্ত;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			যদি (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				যদি (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				অন্যথা যদি (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				css_color color;

				যদি (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_গ্রন্থি_get_next_sibling(c, &next);
			যদি (err != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			dom_গ্রন্থি_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	যদি (c != রিক্ত) {
		dom_গ্রন্থি_unref(c);
	}

	প্রত্যাগম true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
স্থির পূর্ণাঙ্ক box_iframes_talloc_destructor(সংরচনা content_html_iframe *f)
{
	যদি (f->url != রিক্ত) {
		nsurl_unref(f->url);
		f->url = রিক্ত;
	}

	প্রত্যাগম 0;
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
স্থির bool
box_get_attribute(dom_গ্রন্থি *n,
		  ধ্রুব বর্ণ *attribute,
		  শূন্য *context,
		  বর্ণ **মান)
{
	বর্ণ *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ধ্রুব uint8_t *) attribute,
			strlen(attribute), &attr_name);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	যদি (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		প্রত্যাগম false;
	}

	dom_string_unref(attr_name);

	যদি (attr != রিক্ত) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		যদি (result == রিক্ত)
			প্রত্যাগম false;

		*মান = result;
	}

	প্রত্যাগম true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
স্থির bool
box_প্রবেশ_text(html_content *html, সংরচনা box *box, সংরচনা dom_গ্রন্থি *গ্রন্থি)
{
	সংরচনা box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	বিকল্প (display) {
	পক্ষ CSS_DISPLAY_GRID:
	পক্ষ CSS_DISPLAY_FLEX:
	পক্ষ CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		ভঙ্গ;
	স্বভাব:
		box->type = BOX_INLINE_BLOCK;
		ভঙ্গ;
	}

	inline_container = box_create(রিক্ত, 0, false, 0, 0, 0, 0, html->bctx);
	যদি (!inline_container)
		প্রত্যাগম false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(রিক্ত, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	যদি (!inline_box)
		প্রত্যাগম false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	প্রত্যাগম box_textarea_create_textarea(html, box, গ্রন্থি);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
স্থির bool box_select_add_option(সংরচনা form_control *control, dom_গ্রন্থি *n)
{
	বর্ণ *মান = রিক্ত;
	বর্ণ *text = রিক্ত;
	বর্ণ *text_nowrap = রিক্ত;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_গ্রন্থি_get_text_content(n, &content);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যদি (content != রিক্ত) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} অন্যথা {
		text = strdup("");
	}

	যদি (text == রিক্ত)
		গচ্ছ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_মান, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		মান = strdup(dom_string_data(s));
		dom_string_unref(s);
	} অন্যথা {
		মান = strdup(text);
	}

	যদি (মান == রিক্ত)
		গচ্ছ no_memory;

	যদি (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	যদি (text_nowrap == রিক্ত)
		গচ্ছ no_memory;

	যদি (form_add_option(control, মান, text_nowrap, selected, n) == false)
		গচ্ছ no_memory;

	মুক্ত(text);

	প্রত্যাগম true;

no_memory:
	মুক্ত(মান);
	মুক্ত(text);
	মুক্ত(text_nowrap);
	প্রত্যাগম false;
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
স্থির bool
box_a(dom_গ্রন্থি *n,
      html_content *content,
      সংরচনা box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		যদি (!ok)
			প্রত্যাগম false;
		যদি (url) {
			যদি (box->href != রিক্ত)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		যদি (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		অন্যথা যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_শীর্ষ_2))
			box->target = "_top";
		অন্যথা যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		অন্যথা যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		অন্যথা {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			যদি (!box->target) {
				dom_string_unref(s);
				প্রত্যাগম false;
			}
		}
		dom_string_unref(s);
	}

	প্রত্যাগম true;
}


/**
 * Document body special element handler [7.5.1].
 */
স্থির bool
box_body(dom_গ্রন্থি *n,
	 html_content *content,
	 সংরচনা box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	যদি (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} অন্যথা {
		content->background_colour = nscss_color_to_ns(color);
	}

	প্রত্যাগম true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
স্থির bool
box_br(dom_গ্রন্থি *n,
       html_content *content,
       সংরচনা box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	প্রত্যাগম true;
}


/**
 * special element handler for Push button [17.5].
 */
স্থির bool
box_button(dom_গ্রন্থি *n,
	   html_content *content,
	   সংরচনা box *box,
	   bool *convert_children)
{
	সংরচনা form_control *gadget;

	gadget = html_forms_get_control_for_গ্রন্থি(content->forms, n);
	যদি (!gadget)
		প্রত্যাগম false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	প্রত্যাগম true;
}


/**
 * Canvas element
 */
স্থির bool
box_canvas(dom_গ্রন্থি *n,
	     html_content *content,
	     সংরচনা box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	যদি (!content->enable_scripting) {
		প্রত্যাগম true;
	}
	*convert_children = false;

	যদি (box->style && ns_computed_display(box->style,
			box_is_মূল(n)) == CSS_DISPLAY_NONE)
		প্রত্যাগম true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	প্রত্যাগম true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
স্থির bool
box_embed(dom_গ্রন্থি *n,
	  html_content *content,
	  সংরচনা box *box,
	  bool *convert_children)
{
	সংরচনা object_params *params;
	সংরচনা object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t অঙ্ক_attrs;
	dom_string *src;
	dom_exception err;

	যদি (box->style && ns_computed_display(box->style,
			box_is_মূল(n)) == CSS_DISPLAY_NONE)
		প্রত্যাগম true;

	params = talloc(content->bctx, সংরচনা object_params);
	যদি (params == রিক্ত)
		প্রত্যাগম false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = রিক্ত;
	params->type = রিক্ত;
	params->codetype = রিক্ত;
	params->codebase = রিক্ত;
	params->classid = রিক্ত;
	params->params = রিক্ত;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	যদি (err != DOM_NO_ERR || src == রিক্ত)
		প্রত্যাগম true;
	যদি (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		প্রত্যাগম false;
	}

	dom_string_unref(src);

	যদি (params->data == রিক্ত)
		প্রত্যাগম true;

	/* Don't include ourself */
	যদি (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		প্রত্যাগম true;

	/* add attributes as parameters to linked list */
	err = dom_গ্রন্থি_get_attributes(n, &attrs);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	err = dom_namednodemap_get_length(attrs, &অঙ্ক_attrs);
	যদি (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		প্রত্যাগম false;
	}

	প্রতি (idx = 0; idx < অঙ্ক_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *মান;

		err = dom_namednodemap_item(attrs, idx, (শূন্য *) &attr);
		যদি (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			প্রত্যাগম false;
		}

		err = dom_attr_get_name(attr, &name);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(attr);
			dom_namednodemap_unref(attrs);
			প্রত্যাগম false;
		}

		যদি (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_গ্রন্থি_unref(attr);
			dom_string_unref(name);
			অনুবর্ত;
		}

		err = dom_attr_get_মান(attr, &মান);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			প্রত্যাগম false;
		}

		param = talloc(content->bctx, সংরচনা object_param);
		যদি (param == রিক্ত) {
			dom_গ্রন্থি_unref(attr);
			dom_string_unref(মান);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			প্রত্যাগম false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->মান = talloc_strdup(content->bctx, dom_string_data(মান));
		param->type = রিক্ত;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = রিক্ত;

		dom_string_unref(মান);
		dom_string_unref(name);
		dom_গ্রন্থি_unref(attr);

		যদি (param->name == রিক্ত || param->মান == রিক্ত ||
				param->valuetype == রিক্ত) {
			dom_namednodemap_unref(attrs);
			প্রত্যাগম false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	প্রত্যাগম html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
স্থির bool
box_frameset(dom_গ্রন্থি *n,
	     html_content *content,
	     সংরচনা box *box,
	     bool *convert_children)
{
	bool ok;

	যদি (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		যদি (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		প্রত্যাগম true;
	}

	content->frameset = talloc_zero(content->bctx,
					সংরচনা content_html_frames);
	যদি (!content->frameset) {
		প্রত্যাগম false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	যদি (ok) {
		box->type = BOX_NONE;
	}

	যদি (convert_children) {
		*convert_children = false;
	}
	প্রত্যাগম ok;
}


/**
 * Inline subwindow [16.5].
 */
স্থির bool
box_iframe(dom_গ্রন্থি *n,
	   html_content *content,
	   সংরচনা box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	সংরচনা content_html_iframe *iframe;
	পূর্ণাঙ্ক i;

	যদি (box->style && ns_computed_display(box->style,
			box_is_মূল(n)) == CSS_DISPLAY_NONE)
		প্রত্যাগম true;

	যদি (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		প্রত্যাগম true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	যদি (err != DOM_NO_ERR || s == রিক্ত)
		প্রত্যাগম true;
	যদি (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		প্রত্যাগম false;
	}
	dom_string_unref(s);
	যদি (url == রিক্ত)
		প্রত্যাগম true;

	/* don't include ourself */
	যদি (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		প্রত্যাগম true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, সংরচনা content_html_iframe);
	যদি (iframe == রিক্ত) {
		nsurl_unref(url);
		প্রত্যাগম false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = রিক্ত;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		css_color color;

		যদি (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		অন্যথা যদি (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	প্রতিজ্ঞা(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	যদি (convert_children)
		*convert_children = false;
	প্রত্যাগম true;
}


/**
 * Embedded image [13.2].
 */
স্থির bool
box_image(dom_গ্রন্থি *n,
	  html_content *content,
	  সংরচনা box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	গণনা css_width_e wtype;
	গণনা css_height_e htype;
	css_fixed মান = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	যদি (box->style && ns_computed_display(box->style,
			box_is_মূল(n)) == CSS_DISPLAY_NONE)
		প্রত্যাগম true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	যদি (err == DOM_NO_ERR && s != রিক্ত) {
		বর্ণ *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		যদি (alt == রিক্ত)
			প্রত্যাগম false;
		box->text = talloc_strdup(content->bctx, alt);
		মুক্ত(alt);
		যদি (box->text == রিক্ত)
			প্রত্যাগম false;
		box->length = strlen(box->text);
	}

	যদি (nsoption_bool(foreground_images) == false) {
		প্রত্যাগম true;
	}

	/* imagemap associated with this image */
	যদি (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		প্রত্যাগম false;
	যদি (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	যদি (err != DOM_NO_ERR || s == রিক্ত)
		প্রত্যাগম true;

	যদি (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		প্রত্যাগম false;
	}

	dom_string_unref(s);

	যদি (url == রিক্ত)
		প্রত্যাগম true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &মান, &wunit);
	htype = css_computed_height(box->style, &মান, &hunit);

	যদি (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	প্রত্যাগম ok;
}


/**
 * Form control [17.4].
 */
স্থির bool
box_প্রবেশ(dom_গ্রন্থি *n,
	  html_content *content,
	  সংরচনা box *box,
	  bool *convert_children)
{
	সংরচনা form_control *gadget;
	dom_string *type = রিক্ত;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_গ্রন্থি(content->forms, n);
	যদি (gadget == রিক্ত) {
		প্রত্যাগম false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	যদি ((err != DOM_NO_ERR) || (type == রিক্ত)) {
		/* no type so "text" is assumed */
		যদি (box_প্রবেশ_text(content, box, n) == false) {
			প্রত্যাগম false;
		}
		*convert_children = false;
		প্রত্যাগম true;
	}

	যদি (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		যদি (box_প্রবেশ_text(content, box, n) == false)
			গচ্ছ no_memory;

	} অন্যথা যদি (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} অন্যথা যদি (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} অন্যথা যদি ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} অন্যথা যদি (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		সংরচনা box *inline_container, *inline_box;

		যদি (box_button(n, content, box, 0) == false)
			গচ্ছ no_memory;

		inline_container = box_create(রিক্ত, 0, false, 0, 0, 0, 0,
				content->bctx);
		যদি (inline_container == রিক্ত)
			গচ্ছ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(রিক্ত, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		যদি (inline_box == রিক্ত)
			গচ্ছ no_memory;

		inline_box->type = BOX_TEXT;

		যদি (box->gadget->মান != রিক্ত)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->মান);
		অন্যথা যদি (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		অন্যথা যদি (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		অন্যথা
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		যদি (inline_box->text == রিক্ত)
			গচ্ছ no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} অন্যথা যদি (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		যদি (box->style && ns_computed_display(box->style,
				box_is_মূল(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			যদি (err == DOM_NO_ERR && s != রিক্ত) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				যদি (error != NSERROR_OK)
					গচ্ছ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				যদি (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					যদি (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						গচ্ছ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} অন্যথা {
		/* unhandled type the default is "text" */
		যদি (box_প্রবেশ_text(content, box, n) == false)
			গচ্ছ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	প্রত্যাগম true;

no_memory:
	dom_string_unref(type);

	প্রত্যাগম false;
}


/**
 * Noscript element
 */
স্থির bool
box_noscript(dom_গ্রন্থি *n,
	     html_content *content,
	     সংরচনা box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	যদি (content->enable_scripting) {
		*convert_children = false;
	}

	প্রত্যাগম true;
}


/**
 * Generic embedded object [13.3].
 */
স্থির bool
box_object(dom_গ্রন্থি *n,
	   html_content *content,
	   সংরচনা box *box,
	   bool *convert_children)
{
	সংরচনা object_params *params;
	সংরচনা object_param *param;
	dom_string *codebase, *classid, *data;
	dom_গ্রন্থি *c;
	dom_exception err;

	যদি (box->style && ns_computed_display(box->style,
			box_is_মূল(n)) == CSS_DISPLAY_NONE)
		প্রত্যাগম true;

	যদি (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		প্রত্যাগম false;
	যদি (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, সংরচনা object_params);
	যদি (params == রিক্ত)
		প্রত্যাগম false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = রিক্ত;
	params->type = রিক্ত;
	params->codetype = রিক্ত;
	params->codebase = রিক্ত;
	params->classid = রিক্ত;
	params->params = রিক্ত;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	যদি (err == DOM_NO_ERR && codebase != রিক্ত) {
		যদি (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			প্রত্যাগম false;
		}
		dom_string_unref(codebase);
	}
	যদি (params->codebase == রিক্ত)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	যদি (err == DOM_NO_ERR && classid != রিক্ত) {
		যদি (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			প্রত্যাগম false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	যদি (err == DOM_NO_ERR && data != রিক্ত) {
		যদি (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			প্রত্যাগম false;
		}
		dom_string_unref(data);
	}

	যদি (params->classid == রিক্ত && params->data == রিক্ত)
		/* nothing to embed; ignore */
		প্রত্যাগম true;

	/* Don't include ourself */
	যদি (params->classid != রিক্ত && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		প্রত্যাগম true;

	যদি (params->data != রিক্ত && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		প্রত্যাগম true;

	/* codetype and type are MIME types */
	যদি (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		প্রত্যাগম false;
	যদি (box_get_attribute(n, "type", params, &params->type) == false)
		প্রত্যাগম false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	যদি (params->classid != রিক্ত && params->data == রিক্ত &&
			params->codetype != রিক্ত) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		যদি (lerror != lwc_error_ok)
			প্রত্যাগম false;

		যদি (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			প্রত্যাগম true;
		}

		lwc_string_unref(icodetype);
	}

	যদি (params->data != রিক্ত && params->type != রিক্ত) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		যদি (lerror != lwc_error_ok)
			প্রত্যাগম false;

		যদি (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			প্রত্যাগম true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_গ্রন্থি_get_first_child(n, &c);
	যদি (err != DOM_NO_ERR)
		প্রত্যাগম false;

	যাবত্ (c != রিক্ত) {
		dom_গ্রন্থি *next;
		dom_গ্রন্থি_type type;

		err = dom_গ্রন্থি_get_গ্রন্থি_type(c, &type);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(c);
			প্রত্যাগম false;
		}

		যদি (type == DOM_ELEMENT_গ্রন্থি) {
			dom_string *name;

			err = dom_গ্রন্থি_get_গ্রন্থি_name(c, &name);
			যদি (err != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_গ্রন্থি_unref(c);
				ভঙ্গ;
			}
			dom_string_unref(name);

			param = talloc(params, সংরচনা object_param);
			যদি (param == রিক্ত) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}
			param->name = রিক্ত;
			param->মান = রিক্ত;
			param->type = রিক্ত;
			param->valuetype = রিক্ত;
			param->next = রিক্ত;

			যদি (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (box_get_attribute(c, "value", param,
					&param->মান) == false) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_গ্রন্থি_unref(c);
				প্রত্যাগম false;
			}

			যদি (param->valuetype == রিক্ত) {
				param->valuetype = talloc_strdup(param, "data");
				যদি (param->valuetype == রিক্ত) {
					dom_গ্রন্থি_unref(c);
					প্রত্যাগম false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_গ্রন্থি_get_next_sibling(c, &next);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(c);
			প্রত্যাগম false;
		}

		dom_গ্রন্থি_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	যদি (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		প্রত্যাগম false;

	*convert_children = false;
	প্রত্যাগম true;
}


/**
 * Preformatted text [9.3.4].
 */
স্থির bool
box_pre(dom_গ্রন্থি *n,
	html_content *content,
	সংরচনা box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	প্রত্যাগম true;
}


/**
 * Option selector [17.6].
 */
স্থির bool
box_select(dom_গ্রন্থি *n,
	   html_content *content,
	   সংরচনা box *box,
	   bool *convert_children)
{
	সংরচনা box *inline_container;
	সংরচনা box *inline_box;
	সংরচনা form_control *gadget;
	dom_গ্রন্থি *c, *c2;
	dom_গ্রন্থি *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_গ্রন্থি(content->forms, n);
	যদি (gadget == রিক্ত)
		প্রত্যাগম false;

	gadget->html = content;
	err = dom_গ্রন্থি_get_first_child(n, &c);
	যদি (err != DOM_NO_ERR) {
		form_free_control(gadget);
		প্রত্যাগম false;
	}

	যাবত্ (c != রিক্ত) {
		dom_string *name;

		err = dom_গ্রন্থি_get_গ্রন্থি_name(c, &name);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(c);
			form_free_control(gadget);
			প্রত্যাগম false;
		}

		যদি (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			যদি (box_select_add_option(gadget, c) == false) {
				dom_গ্রন্থি_unref(c);
				form_free_control(gadget);
				প্রত্যাগম false;
			}
		} অন্যথা যদি (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_গ্রন্থি_get_first_child(c, &c2);
			যদি (err != DOM_NO_ERR) {
				dom_গ্রন্থি_unref(c);
				form_free_control(gadget);
				প্রত্যাগম false;
			}

			যাবত্ (c2 != রিক্ত) {
				dom_string *c2_name;

				err = dom_গ্রন্থি_get_গ্রন্থি_name(c2, &c2_name);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(c2);
					dom_গ্রন্থি_unref(c);
					form_free_control(gadget);
					প্রত্যাগম false;
				}

				যদি (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					যদি (box_select_add_option(gadget,
							c2) == false) {
						dom_গ্রন্থি_unref(c2);
						dom_গ্রন্থি_unref(c);
						form_free_control(gadget);
						প্রত্যাগম false;
					}
				} অন্যথা {
					dom_string_unref(c2_name);
				}

				err = dom_গ্রন্থি_get_next_sibling(c2, &next2);
				যদি (err != DOM_NO_ERR) {
					dom_গ্রন্থি_unref(c2);
					dom_গ্রন্থি_unref(c);
					form_free_control(gadget);
					প্রত্যাগম false;
				}

				dom_গ্রন্থি_unref(c2);
				c2 = next2;
			}
		} অন্যথা {
			dom_string_unref(name);
		}

		err = dom_গ্রন্থি_get_next_sibling(c, &next);
		যদি (err != DOM_NO_ERR) {
			dom_গ্রন্থি_unref(c);
			form_free_control(gadget);
			প্রত্যাগম false;
		}

		dom_গ্রন্থি_unref(c);
		c = next;
	}

	যদি (gadget->data.select.অঙ্ক_বস্তুসমূহ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		প্রত্যাগম true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(রিক্ত, 0, false, 0, 0, 0, 0, content->bctx);
	যদি (inline_container == রিক্ত)
		গচ্ছ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(রিক্ত, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	যদি (inline_box == রিক্ত)
		গচ্ছ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	যদি (gadget->data.select.multiple == false &&
			gadget->data.select.অঙ্ক_selected == 0) {
		gadget->data.select.current = gadget->data.select.বস্তুসমূহ;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.অঙ্ক_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->গ্রন্থি, true);
	}

	যদি (gadget->data.select.অঙ্ক_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	অন্যথা যদি (gadget->data.select.অঙ্ক_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	অন্যথা
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	যদি (inline_box->text == রিক্ত)
		গচ্ছ no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	প্রত্যাগম true;

no_memory:
	প্রত্যাগম false;
}


/**
 * Multi-line text field [17.7].
 */
স্থির bool box_textarea(dom_গ্রন্থি *n,
			html_content *content,
			সংরচনা box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_গ্রন্থি(content->forms, n);
	যদি (box->gadget == রিক্ত)
		প্রত্যাগম false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	যদি (!box_প্রবেশ_text(content, box, n))
		প্রত্যাগম false;

	*convert_children = false;
	প্রত্যাগম true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_গ্রন্থি *গ্রন্থি,
			 html_content *content,
			 সংরচনা box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(গ্রন্থি, &tag_type);
	যদি (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	বিকল্প (tag_type) {
	পক্ষ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_প্রবেশ:
		res = box_প্রবেশ(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	পক্ষ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(গ্রন্থি, content, box, convert_children);
		ভঙ্গ;

	স্বভাব:
		res = true;
	}

	প্রত্যাগম res;
}
