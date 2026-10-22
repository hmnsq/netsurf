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

#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ "utils/nsoption.h"
#ଅନ୍ତର୍ଭାଵ "utils/corestrings.h"
#ଅନ୍ତର୍ଭାଵ "utils/log.h"
#ଅନ୍ତର୍ଭାଵ "utils/messages.h"
#ଅନ୍ତର୍ଭାଵ "utils/talloc.h"
#ଅନ୍ତର୍ଭାଵ "utils/string.h"
#ଅନ୍ତର୍ଭାଵ "utils/ascii.h"
#ଅନ୍ତର୍ଭାଵ "utils/nsurl.h"
#ଅନ୍ତର୍ଭାଵ "netsurf/plot_style.h"
#ଅନ୍ତର୍ଭାଵ "css/hints.h"
#ଅନ୍ତର୍ଭାଵ "desktop/frame_types.h"
#ଅନ୍ତର୍ଭାଵ "content/content_factory.h"

#ଅନ୍ତର୍ଭାଵ "html/html.h"
#ଅନ୍ତର୍ଭାଵ "html/private.h"
#ଅନ୍ତର୍ଭାଵ "html/object.h"
#ଅନ୍ତର୍ଭାଵ "html/box.h"
#ଅନ୍ତର୍ଭାଵ "html/box_manipulate.h"
#ଅନ୍ତର୍ଭାଵ "html/box_construct.h"
#ଅନ୍ତର୍ଭାଵ "html/box_special.h"
#ଅନ୍ତର୍ଭାଵ "html/box_textarea.h"
#ଅନ୍ତର୍ଭାଵ "html/form_internal.h"


ସ୍ଥିର ଅଚଳ content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool box_is_ମୂଳ(dom_ଗଣ୍ଠି *n)
{
	dom_ଗଣ୍ଠି *parent;
	dom_ଗଣ୍ଠି_type type;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_parent_ଗଣ୍ଠି(n, &parent);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (parent != ଖାଲି) {
		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(parent, &type);

		dom_ଗଣ୍ଠି_unref(parent);

		ଯଦି (err != DOM_NO_ERR)
			ଫେରନ୍ତୁ false;

		ଯଦି (type != DOM_DOCUMENT_ଗଣ୍ଠି)
			ଫେରନ୍ତୁ false;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_object_talloc_destructor(ସଂରଚନା object_params *o)
{
	ଯଦି (o->codebase != ଖାଲି)
		nsurl_unref(o->codebase);
	ଯଦି (o->classid != ଖାଲି)
		nsurl_unref(o->classid);
	ଯଦି (o->data != ଖାଲି)
		nsurl_unref(o->data);

	ଫେରନ୍ତୁ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ସ୍ଥିର ସଂରଚନା frame_dimension *
box_parse_multi_lengths(ଅଚଳ dom_string *ds, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଗଣନା_2)
{
	ଅକ୍ଷର *end;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
	ସଂରଚନା frame_dimension *length;
	ଅଚଳ ଅକ୍ଷର *s;

	s = dom_string_data(ds);

	ପାଇଁ (i = 0, n = 1; s[i]; i++)
		ଯଦି (s[i] == ',')
			n++;

	length = calloc(n, ଆକାର(ସଂରଚନା frame_dimension));
	ଯଦି (!length)
		ଫେରନ୍ତୁ ଖାଲି;

	ପାଇଁ (i = 0; i != n; i++) {
		ଯେତେବେଳେ (ascii_is_space(*s)) {
			s++;
		}
		length[i].ମୂଲ୍ୟ = strtof(s, &end);
		ଯଦି (length[i].ମୂଲ୍ୟ <= 0) {
			length[i].ମୂଲ୍ୟ = 1;
		}
		s = end;
		ବାଛ (*s) {
			ସ୍ଥିତି '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				ଭାଙ୍ଗ;
		}
		ଯେତେବେଳେ (*s && *s != ',') {
			s++;
		}
		ଯଦି (*s == ',') {
			s++;
		}
	}

	*ଗଣନା_2 = n;
	ଫେରନ୍ତୁ length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_frames_talloc_destructor(ସଂରଚନା content_html_frames *f)
{
	ଯଦି (f->url != ଖାଲି) {
		nsurl_unref(f->url);
		f->url = ଖାଲି;
	}

	ଫେରନ୍ତୁ 0;
}


/**
 * create a frameset box tree
 */
ସ୍ଥିର bool
box_create_frameset(ସଂରଚନା content_html_frames *f,
		    dom_ଗଣ୍ଠି *n,
		    html_content *content)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା row, col, index, i;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ସଂରଚନା frame_dimension *row_height = 0, *col_width = 0;
	dom_ଗଣ୍ଠି *c, *next;
	ସଂରଚନା content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		ଯଦି (row_height == ଖାଲି)
			ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା {
		row_height = calloc(1, ଆକାର(ସଂରଚନା frame_dimension));
		ଯଦି (row_height == ଖାଲି)
			ଫେରନ୍ତୁ false;
		row_height->ମୂଲ୍ୟ = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		ଯଦି (col_width == ଖାଲି) {
			ମୁକ୍ତ(row_height);
			ଫେରନ୍ତୁ false;
		}
	} ଅନ୍ୟଥା {
		col_width = calloc(1, ଆକାର(ସଂରଚନା frame_dimension));
		ଯଦି (col_width == ଖାଲି) {
			ମୁକ୍ତ(row_height);
			ଫେରନ୍ତୁ false;
		}
		col_width->ମୂଲ୍ୟ = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ଯଦି ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		css_color color;

		ଯଦି (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ସଂରଚନା content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	ପାଇଁ (row = 0; row < rows; row++) {
		ପାଇଁ (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = ଖାଲି;
			frame->url = ଖାଲି;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = ଖାଲି;
		}
	}
	ମୁକ୍ତ(col_width);
	ମୁକ୍ତ(row_height);

	/* create the frameset windows */
	err = dom_ଗଣ୍ଠି_get_first_child(n, &c);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ପାଇଁ (row = 0; c != ଖାଲି && row < rows; row++) {
		ପାଇଁ (col = 0; c != ଖାଲି && col < cols; col++) {
			ଯେତେବେଳେ (c != ଖାଲି) {
				dom_ଗଣ୍ଠି_type type;
				dom_string *name;

				err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(c, &type);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(c);
					ଫେରନ୍ତୁ false;
				}

				err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(c, &name);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(c);
					ଫେରନ୍ତୁ false;
				}

				ଯଦି (type != DOM_ELEMENT_ଗଣ୍ଠି ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ଗଣ୍ଠି_get_next_sibling(c,
							&next);
					ଯଦି (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ଗଣ୍ଠି_unref(c);
						ଫେରନ୍ତୁ false;
					}

					dom_string_unref(name);
					dom_ଗଣ୍ଠି_unref(c);
					c = next;
				} ଅନ୍ୟଥା {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ଭାଙ୍ଗ;
				}
			}

			ଯଦି (c == ଖାଲି)
				ଭାଙ୍ଗ;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(c, &s);
			ଯଦି (err != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				ଯଦି (box_create_frameset(frame, c,
						content) == false) {
					dom_ଗଣ୍ଠି_unref(c);
					ଫେରନ୍ତୁ false;
				}

				err = dom_ଗଣ୍ଠି_get_next_sibling(c, &next);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(c);
					ଫେରନ୍ତୁ false;
				}

				dom_ଗଣ୍ଠି_unref(c);
				c = next;
				ଜାରି;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ଖାଲି;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ଯଦି (url != ଖାଲି) {
				/* no self-references */
				ଯଦି (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = ଖାଲି;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			ଯଦି (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				ଯଦି (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				css_color color;

				ଯଦି (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ଗଣ୍ଠି_get_next_sibling(c, &next);
			ଯଦି (err != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			dom_ଗଣ୍ଠି_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ଯଦି (c != ଖାଲି) {
		dom_ଗଣ୍ଠି_unref(c);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା box_iframes_talloc_destructor(ସଂରଚନା content_html_iframe *f)
{
	ଯଦି (f->url != ଖାଲି) {
		nsurl_unref(f->url);
		f->url = ଖାଲି;
	}

	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର bool
box_get_attribute(dom_ଗଣ୍ଠି *n,
		  ଅଚଳ ଅକ୍ଷର *attribute,
		  ଶୂନ୍ୟ *context,
		  ଅକ୍ଷର **ମୂଲ୍ୟ)
{
	ଅକ୍ଷର *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ଅଚଳ uint8_t *) attribute,
			strlen(attribute), &attr_name);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ଯଦି (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ଫେରନ୍ତୁ false;
	}

	dom_string_unref(attr_name);

	ଯଦି (attr != ଖାଲି) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		ଯଦି (result == ଖାଲି)
			ଫେରନ୍ତୁ false;

		*ମୂଲ୍ୟ = result;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ସ୍ଥିର bool
box_ନିବେଶ_text(html_content *html, ସଂରଚନା box *box, ସଂରଚନା dom_ଗଣ୍ଠି *ଗଣ୍ଠି)
{
	ସଂରଚନା box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	ବାଛ (display) {
	ସ୍ଥିତି CSS_DISPLAY_GRID:
	ସ୍ଥିତି CSS_DISPLAY_FLEX:
	ସ୍ଥିତି CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		box->type = BOX_INLINE_BLOCK;
		ଭାଙ୍ଗ;
	}

	inline_container = box_create(ଖାଲି, 0, false, 0, 0, 0, 0, html->bctx);
	ଯଦି (!inline_container)
		ଫେରନ୍ତୁ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ଖାଲି, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ଯଦି (!inline_box)
		ଫେରନ୍ତୁ false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ଫେରନ୍ତୁ box_textarea_create_textarea(html, box, ଗଣ୍ଠି);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ସ୍ଥିର bool box_select_add_option(ସଂରଚନା form_control *control, dom_ଗଣ୍ଠି *n)
{
	ଅକ୍ଷର *ମୂଲ୍ୟ = ଖାଲି;
	ଅକ୍ଷର *text = ଖାଲି;
	ଅକ୍ଷର *text_nowrap = ଖାଲି;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ଗଣ୍ଠି_get_text_content(n, &content);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯଦି (content != ଖାଲି) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} ଅନ୍ୟଥା {
		text = strdup("");
	}

	ଯଦି (text == ଖାଲି)
		ଯାଅ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_ମୂଲ୍ୟ, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ମୂଲ୍ୟ = strdup(dom_string_data(s));
		dom_string_unref(s);
	} ଅନ୍ୟଥା {
		ମୂଲ୍ୟ = strdup(text);
	}

	ଯଦି (ମୂଲ୍ୟ == ଖାଲି)
		ଯାଅ no_memory;

	ଯଦି (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	ଯଦି (text_nowrap == ଖାଲି)
		ଯାଅ no_memory;

	ଯଦି (form_add_option(control, ମୂଲ୍ୟ, text_nowrap, selected, n) == false)
		ଯାଅ no_memory;

	ମୁକ୍ତ(text);

	ଫେରନ୍ତୁ true;

no_memory:
	ମୁକ୍ତ(ମୂଲ୍ୟ);
	ମୁକ୍ତ(text);
	ମୁକ୍ତ(text_nowrap);
	ଫେରନ୍ତୁ false;
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
ସ୍ଥିର bool
box_a(dom_ଗଣ୍ଠି *n,
      html_content *content,
      ସଂରଚନା box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		ଯଦି (!ok)
			ଫେରନ୍ତୁ false;
		ଯଦି (url) {
			ଯଦି (box->href != ଖାଲି)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ଯଦି (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ଶୀର୍ଷ_2))
			box->target = "_top";
		ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		ଅନ୍ୟଥା {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			ଯଦି (!box->target) {
				dom_string_unref(s);
				ଫେରନ୍ତୁ false;
			}
		}
		dom_string_unref(s);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Document body special element handler [7.5.1].
 */
ସ୍ଥିର bool
box_body(dom_ଗଣ୍ଠି *n,
	 html_content *content,
	 ସଂରଚନା box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ଯଦି (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} ଅନ୍ୟଥା {
		content->background_colour = nscss_color_to_ns(color);
	}

	ଫେରନ୍ତୁ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ସ୍ଥିର bool
box_br(dom_ଗଣ୍ଠି *n,
       html_content *content,
       ସଂରଚନା box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ଫେରନ୍ତୁ true;
}


/**
 * special element handler for Push button [17.5].
 */
ସ୍ଥିର bool
box_button(dom_ଗଣ୍ଠି *n,
	   html_content *content,
	   ସଂରଚନା box *box,
	   bool *convert_children)
{
	ସଂରଚନା form_control *gadget;

	gadget = html_forms_get_control_for_ଗଣ୍ଠି(content->forms, n);
	ଯଦି (!gadget)
		ଫେରନ୍ତୁ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	ଫେରନ୍ତୁ true;
}


/**
 * Canvas element
 */
ସ୍ଥିର bool
box_canvas(dom_ଗଣ୍ଠି *n,
	     html_content *content,
	     ସଂରଚନା box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ଯଦି (!content->enable_scripting) {
		ଫେରନ୍ତୁ true;
	}
	*convert_children = false;

	ଯଦି (box->style && ns_computed_display(box->style,
			box_is_ମୂଳ(n)) == CSS_DISPLAY_NONE)
		ଫେରନ୍ତୁ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ଫେରନ୍ତୁ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ସ୍ଥିର bool
box_embed(dom_ଗଣ୍ଠି *n,
	  html_content *content,
	  ସଂରଚନା box *box,
	  bool *convert_children)
{
	ସଂରଚନା object_params *params;
	ସଂରଚନା object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t ଅଙ୍କ_attrs;
	dom_string *src;
	dom_exception err;

	ଯଦି (box->style && ns_computed_display(box->style,
			box_is_ମୂଳ(n)) == CSS_DISPLAY_NONE)
		ଫେରନ୍ତୁ true;

	params = talloc(content->bctx, ସଂରଚନା object_params);
	ଯଦି (params == ଖାଲି)
		ଫେରନ୍ତୁ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ଖାଲି;
	params->type = ଖାଲି;
	params->codetype = ଖାଲି;
	params->codebase = ଖାଲି;
	params->classid = ଖାଲି;
	params->params = ଖାଲି;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ଯଦି (err != DOM_NO_ERR || src == ଖାଲି)
		ଫେରନ୍ତୁ true;
	ଯଦି (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		ଫେରନ୍ତୁ false;
	}

	dom_string_unref(src);

	ଯଦି (params->data == ଖାଲି)
		ଫେରନ୍ତୁ true;

	/* Don't include ourself */
	ଯଦି (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		ଫେରନ୍ତୁ true;

	/* add attributes as parameters to linked list */
	err = dom_ଗଣ୍ଠି_get_attributes(n, &attrs);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	err = dom_namednodemap_get_length(attrs, &ଅଙ୍କ_attrs);
	ଯଦି (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ଫେରନ୍ତୁ false;
	}

	ପାଇଁ (idx = 0; idx < ଅଙ୍କ_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *ମୂଲ୍ୟ;

		err = dom_namednodemap_item(attrs, idx, (ଶୂନ୍ୟ *) &attr);
		ଯଦି (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ଫେରନ୍ତୁ false;
		}

		err = dom_attr_get_name(attr, &name);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(attr);
			dom_namednodemap_unref(attrs);
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ଗଣ୍ଠି_unref(attr);
			dom_string_unref(name);
			ଜାରି;
		}

		err = dom_attr_get_ମୂଲ୍ୟ(attr, &ମୂଲ୍ୟ);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ଫେରନ୍ତୁ false;
		}

		param = talloc(content->bctx, ସଂରଚନା object_param);
		ଯଦି (param == ଖାଲି) {
			dom_ଗଣ୍ଠି_unref(attr);
			dom_string_unref(ମୂଲ୍ୟ);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ଫେରନ୍ତୁ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->ମୂଲ୍ୟ = talloc_strdup(content->bctx, dom_string_data(ମୂଲ୍ୟ));
		param->type = ଖାଲି;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = ଖାଲି;

		dom_string_unref(ମୂଲ୍ୟ);
		dom_string_unref(name);
		dom_ଗଣ୍ଠି_unref(attr);

		ଯଦି (param->name == ଖାଲି || param->ମୂଲ୍ୟ == ଖାଲି ||
				param->valuetype == ଖାଲି) {
			dom_namednodemap_unref(attrs);
			ଫେରନ୍ତୁ false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ଫେରନ୍ତୁ html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ସ୍ଥିର bool
box_frameset(dom_ଗଣ୍ଠି *n,
	     html_content *content,
	     ସଂରଚନା box *box,
	     bool *convert_children)
{
	bool ok;

	ଯଦି (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ଯଦି (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ଫେରନ୍ତୁ true;
	}

	content->frameset = talloc_zero(content->bctx,
					ସଂରଚନା content_html_frames);
	ଯଦି (!content->frameset) {
		ଫେରନ୍ତୁ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ଯଦି (ok) {
		box->type = BOX_NONE;
	}

	ଯଦି (convert_children) {
		*convert_children = false;
	}
	ଫେରନ୍ତୁ ok;
}


/**
 * Inline subwindow [16.5].
 */
ସ୍ଥିର bool
box_iframe(dom_ଗଣ୍ଠି *n,
	   html_content *content,
	   ସଂରଚନା box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ସଂରଚନା content_html_iframe *iframe;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (box->style && ns_computed_display(box->style,
			box_is_ମୂଳ(n)) == CSS_DISPLAY_NONE)
		ଫେରନ୍ତୁ true;

	ଯଦି (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ଫେରନ୍ତୁ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ଯଦି (err != DOM_NO_ERR || s == ଖାଲି)
		ଫେରନ୍ତୁ true;
	ଯଦି (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ଫେରନ୍ତୁ false;
	}
	dom_string_unref(s);
	ଯଦି (url == ଖାଲି)
		ଫେରନ୍ତୁ true;

	/* don't include ourself */
	ଯଦି (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ଫେରନ୍ତୁ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ସଂରଚନା content_html_iframe);
	ଯଦି (iframe == ଖାଲି) {
		nsurl_unref(url);
		ଫେରନ୍ତୁ false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = ଖାଲି;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		css_color color;

		ଯଦି (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	ପ୍ରତିଜ୍ଞା(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ଯଦି (convert_children)
		*convert_children = false;
	ଫେରନ୍ତୁ true;
}


/**
 * Embedded image [13.2].
 */
ସ୍ଥିର bool
box_image(dom_ଗଣ୍ଠି *n,
	  html_content *content,
	  ସଂରଚନା box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ଗଣନା css_width_e wtype;
	ଗଣନା css_height_e htype;
	css_fixed ମୂଲ୍ୟ = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ଯଦି (box->style && ns_computed_display(box->style,
			box_is_ମୂଳ(n)) == CSS_DISPLAY_NONE)
		ଫେରନ୍ତୁ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
		ଅକ୍ଷର *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		ଯଦି (alt == ଖାଲି)
			ଫେରନ୍ତୁ false;
		box->text = talloc_strdup(content->bctx, alt);
		ମୁକ୍ତ(alt);
		ଯଦି (box->text == ଖାଲି)
			ଫେରନ୍ତୁ false;
		box->length = strlen(box->text);
	}

	ଯଦି (nsoption_bool(foreground_images) == false) {
		ଫେରନ୍ତୁ true;
	}

	/* imagemap associated with this image */
	ଯଦି (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ଫେରନ୍ତୁ false;
	ଯଦି (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ଯଦି (err != DOM_NO_ERR || s == ଖାଲି)
		ଫେରନ୍ତୁ true;

	ଯଦି (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ଫେରନ୍ତୁ false;
	}

	dom_string_unref(s);

	ଯଦି (url == ଖାଲି)
		ଫେରନ୍ତୁ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &ମୂଲ୍ୟ, &wunit);
	htype = css_computed_height(box->style, &ମୂଲ୍ୟ, &hunit);

	ଯଦି (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ଫେରନ୍ତୁ ok;
}


/**
 * Form control [17.4].
 */
ସ୍ଥିର bool
box_ନିବେଶ(dom_ଗଣ୍ଠି *n,
	  html_content *content,
	  ସଂରଚନା box *box,
	  bool *convert_children)
{
	ସଂରଚନା form_control *gadget;
	dom_string *type = ଖାଲି;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ଗଣ୍ଠି(content->forms, n);
	ଯଦି (gadget == ଖାଲି) {
		ଫେରନ୍ତୁ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ଯଦି ((err != DOM_NO_ERR) || (type == ଖାଲି)) {
		/* no type so "text" is assumed */
		ଯଦି (box_ନିବେଶ_text(content, box, n) == false) {
			ଫେରନ୍ତୁ false;
		}
		*convert_children = false;
		ଫେରନ୍ତୁ true;
	}

	ଯଦି (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ଯଦି (box_ନିବେଶ_text(content, box, n) == false)
			ଯାଅ no_memory;

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ଅନ୍ୟଥା ଯଦି ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ସଂରଚନା box *inline_container, *inline_box;

		ଯଦି (box_button(n, content, box, 0) == false)
			ଯାଅ no_memory;

		inline_container = box_create(ଖାଲି, 0, false, 0, 0, 0, 0,
				content->bctx);
		ଯଦି (inline_container == ଖାଲି)
			ଯାଅ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ଖାଲି, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ଯଦି (inline_box == ଖାଲି)
			ଯାଅ no_memory;

		inline_box->type = BOX_TEXT;

		ଯଦି (box->gadget->ମୂଲ୍ୟ != ଖାଲି)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->ମୂଲ୍ୟ);
		ଅନ୍ୟଥା ଯଦି (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ଅନ୍ୟଥା ଯଦି (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ଅନ୍ୟଥା
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		ଯଦି (inline_box->text == ଖାଲି)
			ଯାଅ no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ଯଦି (box->style && ns_computed_display(box->style,
				box_is_ମୂଳ(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ଯଦି (err == DOM_NO_ERR && s != ଖାଲି) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				ଯଦି (error != NSERROR_OK)
					ଯାଅ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ଯଦି (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					ଯଦି (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						ଯାଅ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} ଅନ୍ୟଥା {
		/* unhandled type the default is "text" */
		ଯଦି (box_ନିବେଶ_text(content, box, n) == false)
			ଯାଅ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	ଫେରନ୍ତୁ true;

no_memory:
	dom_string_unref(type);

	ଫେରନ୍ତୁ false;
}


/**
 * Noscript element
 */
ସ୍ଥିର bool
box_noscript(dom_ଗଣ୍ଠି *n,
	     html_content *content,
	     ସଂରଚନା box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ଯଦି (content->enable_scripting) {
		*convert_children = false;
	}

	ଫେରନ୍ତୁ true;
}


/**
 * Generic embedded object [13.3].
 */
ସ୍ଥିର bool
box_object(dom_ଗଣ୍ଠି *n,
	   html_content *content,
	   ସଂରଚନା box *box,
	   bool *convert_children)
{
	ସଂରଚନା object_params *params;
	ସଂରଚନା object_param *param;
	dom_string *codebase, *classid, *data;
	dom_ଗଣ୍ଠି *c;
	dom_exception err;

	ଯଦି (box->style && ns_computed_display(box->style,
			box_is_ମୂଳ(n)) == CSS_DISPLAY_NONE)
		ଫେରନ୍ତୁ true;

	ଯଦି (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ଫେରନ୍ତୁ false;
	ଯଦି (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ସଂରଚନା object_params);
	ଯଦି (params == ଖାଲି)
		ଫେରନ୍ତୁ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ଖାଲି;
	params->type = ଖାଲି;
	params->codetype = ଖାଲି;
	params->codebase = ଖାଲି;
	params->classid = ଖାଲି;
	params->params = ଖାଲି;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ଯଦି (err == DOM_NO_ERR && codebase != ଖାଲି) {
		ଯଦି (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ଫେରନ୍ତୁ false;
		}
		dom_string_unref(codebase);
	}
	ଯଦି (params->codebase == ଖାଲି)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ଯଦି (err == DOM_NO_ERR && classid != ଖାଲି) {
		ଯଦି (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ଫେରନ୍ତୁ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	ଯଦି (err == DOM_NO_ERR && data != ଖାଲି) {
		ଯଦି (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			ଫେରନ୍ତୁ false;
		}
		dom_string_unref(data);
	}

	ଯଦି (params->classid == ଖାଲି && params->data == ଖାଲି)
		/* nothing to embed; ignore */
		ଫେରନ୍ତୁ true;

	/* Don't include ourself */
	ଯଦି (params->classid != ଖାଲି && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		ଫେରନ୍ତୁ true;

	ଯଦି (params->data != ଖାଲି && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		ଫେରନ୍ତୁ true;

	/* codetype and type are MIME types */
	ଯଦି (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ଫେରନ୍ତୁ false;
	ଯଦି (box_get_attribute(n, "type", params, &params->type) == false)
		ଫେରନ୍ତୁ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ଯଦି (params->classid != ଖାଲି && params->data == ଖାଲି &&
			params->codetype != ଖାଲି) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		ଯଦି (lerror != lwc_error_ok)
			ଫେରନ୍ତୁ false;

		ଯଦି (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ଫେରନ୍ତୁ true;
		}

		lwc_string_unref(icodetype);
	}

	ଯଦି (params->data != ଖାଲି && params->type != ଖାଲି) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		ଯଦି (lerror != lwc_error_ok)
			ଫେରନ୍ତୁ false;

		ଯଦି (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ଫେରନ୍ତୁ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ଗଣ୍ଠି_get_first_child(n, &c);
	ଯଦି (err != DOM_NO_ERR)
		ଫେରନ୍ତୁ false;

	ଯେତେବେଳେ (c != ଖାଲି) {
		dom_ଗଣ୍ଠି *next;
		dom_ଗଣ୍ଠି_type type;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_type(c, &type);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(c);
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (type == DOM_ELEMENT_ଗଣ୍ଠି) {
			dom_string *name;

			err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(c, &name);
			ଯଦି (err != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ଗଣ୍ଠି_unref(c);
				ଭାଙ୍ଗ;
			}
			dom_string_unref(name);

			param = talloc(params, ସଂରଚନା object_param);
			ଯଦି (param == ଖାଲି) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}
			param->name = ଖାଲି;
			param->ମୂଲ୍ୟ = ଖାଲି;
			param->type = ଖାଲି;
			param->valuetype = ଖାଲି;
			param->next = ଖାଲି;

			ଯଦି (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (box_get_attribute(c, "value", param,
					&param->ମୂଲ୍ୟ) == false) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ଗଣ୍ଠି_unref(c);
				ଫେରନ୍ତୁ false;
			}

			ଯଦି (param->valuetype == ଖାଲି) {
				param->valuetype = talloc_strdup(param, "data");
				ଯଦି (param->valuetype == ଖାଲି) {
					dom_ଗଣ୍ଠି_unref(c);
					ଫେରନ୍ତୁ false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_ଗଣ୍ଠି_get_next_sibling(c, &next);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(c);
			ଫେରନ୍ତୁ false;
		}

		dom_ଗଣ୍ଠି_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ଯଦି (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ଫେରନ୍ତୁ false;

	*convert_children = false;
	ଫେରନ୍ତୁ true;
}


/**
 * Preformatted text [9.3.4].
 */
ସ୍ଥିର bool
box_pre(dom_ଗଣ୍ଠି *n,
	html_content *content,
	ସଂରଚନା box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ଫେରନ୍ତୁ true;
}


/**
 * Option selector [17.6].
 */
ସ୍ଥିର bool
box_select(dom_ଗଣ୍ଠି *n,
	   html_content *content,
	   ସଂରଚନା box *box,
	   bool *convert_children)
{
	ସଂରଚନା box *inline_container;
	ସଂରଚନା box *inline_box;
	ସଂରଚନା form_control *gadget;
	dom_ଗଣ୍ଠି *c, *c2;
	dom_ଗଣ୍ଠି *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ଗଣ୍ଠି(content->forms, n);
	ଯଦି (gadget == ଖାଲି)
		ଫେରନ୍ତୁ false;

	gadget->html = content;
	err = dom_ଗଣ୍ଠି_get_first_child(n, &c);
	ଯଦି (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ଫେରନ୍ତୁ false;
	}

	ଯେତେବେଳେ (c != ଖାଲି) {
		dom_string *name;

		err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(c, &name);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(c);
			form_free_control(gadget);
			ଫେରନ୍ତୁ false;
		}

		ଯଦି (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ଯଦି (box_select_add_option(gadget, c) == false) {
				dom_ଗଣ୍ଠି_unref(c);
				form_free_control(gadget);
				ଫେରନ୍ତୁ false;
			}
		} ଅନ୍ୟଥା ଯଦି (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ଗଣ୍ଠି_get_first_child(c, &c2);
			ଯଦି (err != DOM_NO_ERR) {
				dom_ଗଣ୍ଠି_unref(c);
				form_free_control(gadget);
				ଫେରନ୍ତୁ false;
			}

			ଯେତେବେଳେ (c2 != ଖାଲି) {
				dom_string *c2_name;

				err = dom_ଗଣ୍ଠି_get_ଗଣ୍ଠି_name(c2, &c2_name);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(c2);
					dom_ଗଣ୍ଠି_unref(c);
					form_free_control(gadget);
					ଫେରନ୍ତୁ false;
				}

				ଯଦି (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ଯଦି (box_select_add_option(gadget,
							c2) == false) {
						dom_ଗଣ୍ଠି_unref(c2);
						dom_ଗଣ୍ଠି_unref(c);
						form_free_control(gadget);
						ଫେରନ୍ତୁ false;
					}
				} ଅନ୍ୟଥା {
					dom_string_unref(c2_name);
				}

				err = dom_ଗଣ୍ଠି_get_next_sibling(c2, &next2);
				ଯଦି (err != DOM_NO_ERR) {
					dom_ଗଣ୍ଠି_unref(c2);
					dom_ଗଣ୍ଠି_unref(c);
					form_free_control(gadget);
					ଫେରନ୍ତୁ false;
				}

				dom_ଗଣ୍ଠି_unref(c2);
				c2 = next2;
			}
		} ଅନ୍ୟଥା {
			dom_string_unref(name);
		}

		err = dom_ଗଣ୍ଠି_get_next_sibling(c, &next);
		ଯଦି (err != DOM_NO_ERR) {
			dom_ଗଣ୍ଠି_unref(c);
			form_free_control(gadget);
			ଫେରନ୍ତୁ false;
		}

		dom_ଗଣ୍ଠି_unref(c);
		c = next;
	}

	ଯଦି (gadget->data.select.ଅଙ୍କ_ବସ୍ତୁଗୁଡ଼ିକ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ଫେରନ୍ତୁ true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ଖାଲି, 0, false, 0, 0, 0, 0, content->bctx);
	ଯଦି (inline_container == ଖାଲି)
		ଯାଅ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ଖାଲି, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ଯଦି (inline_box == ଖାଲି)
		ଯାଅ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ଯଦି (gadget->data.select.multiple == false &&
			gadget->data.select.ଅଙ୍କ_selected == 0) {
		gadget->data.select.current = gadget->data.select.ବସ୍ତୁଗୁଡ଼ିକ;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.ଅଙ୍କ_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->ଗଣ୍ଠି, true);
	}

	ଯଦି (gadget->data.select.ଅଙ୍କ_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ଅନ୍ୟଥା ଯଦି (gadget->data.select.ଅଙ୍କ_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	ଅନ୍ୟଥା
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ଯଦି (inline_box->text == ଖାଲି)
		ଯାଅ no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	ଫେରନ୍ତୁ true;

no_memory:
	ଫେରନ୍ତୁ false;
}


/**
 * Multi-line text field [17.7].
 */
ସ୍ଥିର bool box_textarea(dom_ଗଣ୍ଠି *n,
			html_content *content,
			ସଂରଚନା box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ଗଣ୍ଠି(content->forms, n);
	ଯଦି (box->gadget == ଖାଲି)
		ଫେରନ୍ତୁ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ଯଦି (!box_ନିବେଶ_text(content, box, n))
		ଫେରନ୍ତୁ false;

	*convert_children = false;
	ଫେରନ୍ତୁ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ଗଣ୍ଠି *ଗଣ୍ଠି,
			 html_content *content,
			 ସଂରଚନା box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(ଗଣ୍ଠି, &tag_type);
	ଯଦି (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ବାଛ (tag_type) {
	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_ନିବେଶ:
		res = box_ନିବେଶ(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ଗଣ୍ଠି, content, box, convert_children);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		res = true;
	}

	ଫେରନ୍ତୁ res;
}
