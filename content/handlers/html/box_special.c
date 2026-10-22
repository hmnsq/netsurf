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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"
#ಅನ್ತರ್ಭಾವ "css/hints.h"
#ಅನ್ತರ್ಭಾವ "desktop/frame_types.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/box_special.h"
#ಅನ್ತರ್ಭಾವ "html/box_textarea.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"


ಸ್ಥಿರ ಧ್ರುವ content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool box_is_ಬೇರು(dom_ಗಂಟು *n)
{
	dom_ಗಂಟು *parent;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_parent_ಗಂಟು(n, &parent);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (parent != ರಿಕ್ತ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(parent, &type);

		dom_ಗಂಟು_unref(parent);

		ಯದಿ (err != DOM_NO_ERR)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (type != DOM_DOCUMENT_ಗಂಟು)
			ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ box_object_talloc_destructor(ಸಂರಚನಾ object_params *o)
{
	ಯದಿ (o->codebase != ರಿಕ್ತ)
		nsurl_unref(o->codebase);
	ಯದಿ (o->classid != ರಿಕ್ತ)
		nsurl_unref(o->classid);
	ಯದಿ (o->data != ರಿಕ್ತ)
		nsurl_unref(o->data);

	ಪ್ರತ್ಯಾಗಮ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ಸ್ಥಿರ ಸಂರಚನಾ frame_dimension *
box_parse_multi_lengths(ಧ್ರುವ dom_string *ds, unsigned ಪೂರ್ಣಾಙ್ಕ *ಎಣಿಕೆ)
{
	ವರ್ಣ *end;
	unsigned ಪೂರ್ಣಾಙ್ಕ i, n;
	ಸಂರಚನಾ frame_dimension *length;
	ಧ್ರುವ ವರ್ಣ *s;

	s = dom_string_data(ds);

	ಪ್ರತಿ (i = 0, n = 1; s[i]; i++)
		ಯದಿ (s[i] == ',')
			n++;

	length = calloc(n, ಪರಿಮಾಣ(ಸಂರಚನಾ frame_dimension));
	ಯದಿ (!length)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತಿ (i = 0; i != n; i++) {
		ಯಾವತ್ (ascii_is_space(*s)) {
			s++;
		}
		length[i].ಮೌಲ್ಯ = strtof(s, &end);
		ಯದಿ (length[i].ಮೌಲ್ಯ <= 0) {
			length[i].ಮೌಲ್ಯ = 1;
		}
		s = end;
		ವಿಕಲ್ಪ (*s) {
			ಪಕ್ಷ '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				ಭಙ್ಗ;
			ಪಕ್ಷ '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				ಭಙ್ಗ;
		}
		ಯಾವತ್ (*s && *s != ',') {
			s++;
		}
		ಯದಿ (*s == ',') {
			s++;
		}
	}

	*ಎಣಿಕೆ = n;
	ಪ್ರತ್ಯಾಗಮ length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ box_frames_talloc_destructor(ಸಂರಚನಾ content_html_frames *f)
{
	ಯದಿ (f->url != ರಿಕ್ತ) {
		nsurl_unref(f->url);
		f->url = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}


/**
 * create a frameset box tree
 */
ಸ್ಥಿರ bool
box_create_frameset(ಸಂರಚನಾ content_html_frames *f,
		    dom_ಗಂಟು *n,
		    html_content *content)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ row, col, index, i;
	unsigned ಪೂರ್ಣಾಙ್ಕ rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ಸಂರಚನಾ frame_dimension *row_height = 0, *col_width = 0;
	dom_ಗಂಟು *c, *next;
	ಸಂರಚನಾ content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		ಯದಿ (row_height == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ {
		row_height = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ frame_dimension));
		ಯದಿ (row_height == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;
		row_height->ಮೌಲ್ಯ = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		ಯದಿ (col_width == ರಿಕ್ತ) {
			ಮುಕ್ತ(row_height);
			ಪ್ರತ್ಯಾಗಮ false;
		}
	} ಅನ್ಯಥಾ {
		col_width = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ frame_dimension));
		ಯದಿ (col_width == ರಿಕ್ತ) {
			ಮುಕ್ತ(row_height);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		col_width->ಮೌಲ್ಯ = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ಯದಿ ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		css_color color;

		ಯದಿ (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ಸಂರಚನಾ content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	ಪ್ರತಿ (row = 0; row < rows; row++) {
		ಪ್ರತಿ (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = ರಿಕ್ತ;
			frame->url = ರಿಕ್ತ;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = ರಿಕ್ತ;
		}
	}
	ಮುಕ್ತ(col_width);
	ಮುಕ್ತ(row_height);

	/* create the frameset windows */
	err = dom_ಗಂಟು_get_first_child(n, &c);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತಿ (row = 0; c != ರಿಕ್ತ && row < rows; row++) {
		ಪ್ರತಿ (col = 0; c != ರಿಕ್ತ && col < cols; col++) {
			ಯಾವತ್ (c != ರಿಕ್ತ) {
				dom_ಗಂಟು_type type;
				dom_string *name;

				err = dom_ಗಂಟು_get_ಗಂಟು_type(c, &type);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				ಯದಿ (type != DOM_ELEMENT_ಗಂಟು ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ಗಂಟು_get_next_sibling(c,
							&next);
					ಯದಿ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ಗಂಟು_unref(c);
						ಪ್ರತ್ಯಾಗಮ false;
					}

					dom_string_unref(name);
					dom_ಗಂಟು_unref(c);
					c = next;
				} ಅನ್ಯಥಾ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ಭಙ್ಗ;
				}
			}

			ಯದಿ (c == ರಿಕ್ತ)
				ಭಙ್ಗ;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &s);
			ಯದಿ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				ಯದಿ (box_create_frameset(frame, c,
						content) == false) {
					dom_ಗಂಟು_unref(c);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				err = dom_ಗಂಟು_get_next_sibling(c, &next);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				dom_ಗಂಟು_unref(c);
				c = next;
				ಅನುವರ್ತ;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ರಿಕ್ತ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ಯದಿ (url != ರಿಕ್ತ) {
				/* no self-references */
				ಯದಿ (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = ರಿಕ್ತ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			ಯದಿ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				ಯದಿ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				css_color color;

				ಯದಿ (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ಗಂಟು_get_next_sibling(c, &next);
			ಯದಿ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			dom_ಗಂಟು_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ಯದಿ (c != ರಿಕ್ತ) {
		dom_ಗಂಟು_unref(c);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ box_iframes_talloc_destructor(ಸಂರಚನಾ content_html_iframe *f)
{
	ಯದಿ (f->url != ರಿಕ್ತ) {
		nsurl_unref(f->url);
		f->url = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ bool
box_get_attribute(dom_ಗಂಟು *n,
		  ಧ್ರುವ ವರ್ಣ *attribute,
		  ಶೂನ್ಯ *context,
		  ವರ್ಣ **ಮೌಲ್ಯ)
{
	ವರ್ಣ *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ಧ್ರುವ uint8_t *) attribute,
			strlen(attribute), &attr_name);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	dom_string_unref(attr_name);

	ಯದಿ (attr != ರಿಕ್ತ) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		ಯದಿ (result == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;

		*ಮೌಲ್ಯ = result;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ಸ್ಥಿರ bool
box_ಒಳಹರಿವು_text(html_content *html, ಸಂರಚನಾ box *box, ಸಂರಚನಾ dom_ಗಂಟು *ಗಂಟು)
{
	ಸಂರಚನಾ box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	ವಿಕಲ್ಪ (display) {
	ಪಕ್ಷ CSS_DISPLAY_GRID:
	ಪಕ್ಷ CSS_DISPLAY_FLEX:
	ಪಕ್ಷ CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		box->type = BOX_INLINE_BLOCK;
		ಭಙ್ಗ;
	}

	inline_container = box_create(ರಿಕ್ತ, 0, false, 0, 0, 0, 0, html->bctx);
	ಯದಿ (!inline_container)
		ಪ್ರತ್ಯಾಗಮ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ರಿಕ್ತ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ಯದಿ (!inline_box)
		ಪ್ರತ್ಯಾಗಮ false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ಪ್ರತ್ಯಾಗಮ box_textarea_create_textarea(html, box, ಗಂಟು);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool box_select_add_option(ಸಂರಚನಾ form_control *control, dom_ಗಂಟು *n)
{
	ವರ್ಣ *ಮೌಲ್ಯ = ರಿಕ್ತ;
	ವರ್ಣ *text = ರಿಕ್ತ;
	ವರ್ಣ *text_nowrap = ರಿಕ್ತ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ಗಂಟು_get_text_content(n, &content);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (content != ರಿಕ್ತ) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} ಅನ್ಯಥಾ {
		text = strdup("");
	}

	ಯದಿ (text == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_ಮೌಲ್ಯ, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ಮೌಲ್ಯ = strdup(dom_string_data(s));
		dom_string_unref(s);
	} ಅನ್ಯಥಾ {
		ಮೌಲ್ಯ = strdup(text);
	}

	ಯದಿ (ಮೌಲ್ಯ == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;

	ಯದಿ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	ಯದಿ (text_nowrap == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;

	ಯದಿ (form_add_option(control, ಮೌಲ್ಯ, text_nowrap, selected, n) == false)
		ಗಚ್ಛ no_memory;

	ಮುಕ್ತ(text);

	ಪ್ರತ್ಯಾಗಮ true;

no_memory:
	ಮುಕ್ತ(ಮೌಲ್ಯ);
	ಮುಕ್ತ(text);
	ಮುಕ್ತ(text_nowrap);
	ಪ್ರತ್ಯಾಗಮ false;
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
ಸ್ಥಿರ bool
box_a(dom_ಗಂಟು *n,
      html_content *content,
      ಸಂರಚನಾ box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		ಯದಿ (!ok)
			ಪ್ರತ್ಯಾಗಮ false;
		ಯದಿ (url) {
			ಯದಿ (box->href != ರಿಕ್ತ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ಯದಿ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ತುದಿ_2))
			box->target = "_top";
		ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		ಅನ್ಯಥಾ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			ಯದಿ (!box->target) {
				dom_string_unref(s);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
		dom_string_unref(s);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Document body special element handler [7.5.1].
 */
ಸ್ಥಿರ bool
box_body(dom_ಗಂಟು *n,
	 html_content *content,
	 ಸಂರಚನಾ box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ಯದಿ (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} ಅನ್ಯಥಾ {
		content->background_colour = nscss_color_to_ns(color);
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ಸ್ಥಿರ bool
box_br(dom_ಗಂಟು *n,
       html_content *content,
       ಸಂರಚನಾ box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * special element handler for Push button [17.5].
 */
ಸ್ಥಿರ bool
box_button(dom_ಗಂಟು *n,
	   html_content *content,
	   ಸಂರಚನಾ box *box,
	   bool *convert_children)
{
	ಸಂರಚನಾ form_control *gadget;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಯದಿ (!gadget)
		ಪ್ರತ್ಯಾಗಮ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Canvas element
 */
ಸ್ಥಿರ bool
box_canvas(dom_ಗಂಟು *n,
	     html_content *content,
	     ಸಂರಚನಾ box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ಯದಿ (!content->enable_scripting) {
		ಪ್ರತ್ಯಾಗಮ true;
	}
	*convert_children = false;

	ಯದಿ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಪ್ರತ್ಯಾಗಮ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ಸ್ಥಿರ bool
box_embed(dom_ಗಂಟು *n,
	  html_content *content,
	  ಸಂರಚನಾ box *box,
	  bool *convert_children)
{
	ಸಂರಚನಾ object_params *params;
	ಸಂರಚನಾ object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t ಅಂಕೆ_attrs;
	dom_string *src;
	dom_exception err;

	ಯದಿ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಪ್ರತ್ಯಾಗಮ true;

	params = talloc(content->bctx, ಸಂರಚನಾ object_params);
	ಯದಿ (params == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ರಿಕ್ತ;
	params->type = ರಿಕ್ತ;
	params->codetype = ರಿಕ್ತ;
	params->codebase = ರಿಕ್ತ;
	params->classid = ರಿಕ್ತ;
	params->params = ರಿಕ್ತ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ಯದಿ (err != DOM_NO_ERR || src == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;
	ಯದಿ (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	dom_string_unref(src);

	ಯದಿ (params->data == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	/* Don't include ourself */
	ಯದಿ (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		ಪ್ರತ್ಯಾಗಮ true;

	/* add attributes as parameters to linked list */
	err = dom_ಗಂಟು_get_attributes(n, &attrs);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	err = dom_namednodemap_get_length(attrs, &ಅಂಕೆ_attrs);
	ಯದಿ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತಿ (idx = 0; idx < ಅಂಕೆ_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *ಮೌಲ್ಯ;

		err = dom_namednodemap_item(attrs, idx, (ಶೂನ್ಯ *) &attr);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		err = dom_attr_get_name(attr, &name);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(attr);
			dom_namednodemap_unref(attrs);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(name);
			ಅನುವರ್ತ;
		}

		err = dom_attr_get_ಮೌಲ್ಯ(attr, &ಮೌಲ್ಯ);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		param = talloc(content->bctx, ಸಂರಚನಾ object_param);
		ಯದಿ (param == ರಿಕ್ತ) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(ಮೌಲ್ಯ);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->ಮೌಲ್ಯ = talloc_strdup(content->bctx, dom_string_data(ಮೌಲ್ಯ));
		param->type = ರಿಕ್ತ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = ರಿಕ್ತ;

		dom_string_unref(ಮೌಲ್ಯ);
		dom_string_unref(name);
		dom_ಗಂಟು_unref(attr);

		ಯದಿ (param->name == ರಿಕ್ತ || param->ಮೌಲ್ಯ == ರಿಕ್ತ ||
				param->valuetype == ರಿಕ್ತ) {
			dom_namednodemap_unref(attrs);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ಪ್ರತ್ಯಾಗಮ html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ಸ್ಥಿರ bool
box_frameset(dom_ಗಂಟು *n,
	     html_content *content,
	     ಸಂರಚನಾ box *box,
	     bool *convert_children)
{
	bool ok;

	ಯದಿ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ಯದಿ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ಪ್ರತ್ಯಾಗಮ true;
	}

	content->frameset = talloc_zero(content->bctx,
					ಸಂರಚನಾ content_html_frames);
	ಯದಿ (!content->frameset) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ಯದಿ (ok) {
		box->type = BOX_NONE;
	}

	ಯದಿ (convert_children) {
		*convert_children = false;
	}
	ಪ್ರತ್ಯಾಗಮ ok;
}


/**
 * Inline subwindow [16.5].
 */
ಸ್ಥಿರ bool
box_iframe(dom_ಗಂಟು *n,
	   html_content *content,
	   ಸಂರಚನಾ box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ಸಂರಚನಾ content_html_iframe *iframe;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ಯದಿ (err != DOM_NO_ERR || s == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;
	ಯದಿ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ಪ್ರತ್ಯಾಗಮ false;
	}
	dom_string_unref(s);
	ಯದಿ (url == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	/* don't include ourself */
	ಯದಿ (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ಸಂರಚನಾ content_html_iframe);
	ಯದಿ (iframe == ರಿಕ್ತ) {
		nsurl_unref(url);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = ರಿಕ್ತ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		css_color color;

		ಯದಿ (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	ಪ್ರತಿಜ್ಞಾ(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ಯದಿ (convert_children)
		*convert_children = false;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Embedded image [13.2].
 */
ಸ್ಥಿರ bool
box_image(dom_ಗಂಟು *n,
	  html_content *content,
	  ಸಂರಚನಾ box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ಗಣನಾ css_width_e wtype;
	ಗಣನಾ css_height_e htype;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ಯದಿ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಪ್ರತ್ಯಾಗಮ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
		ವರ್ಣ *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		ಯದಿ (alt == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;
		box->text = talloc_strdup(content->bctx, alt);
		ಮುಕ್ತ(alt);
		ಯದಿ (box->text == ರಿಕ್ತ)
			ಪ್ರತ್ಯಾಗಮ false;
		box->length = strlen(box->text);
	}

	ಯದಿ (nsoption_bool(foreground_images) == false) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* imagemap associated with this image */
	ಯದಿ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ಯದಿ (err != DOM_NO_ERR || s == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	dom_string_unref(s);

	ಯದಿ (url == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &ಮೌಲ್ಯ, &wunit);
	htype = css_computed_height(box->style, &ಮೌಲ್ಯ, &hunit);

	ಯದಿ (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ಪ್ರತ್ಯಾಗಮ ok;
}


/**
 * Form control [17.4].
 */
ಸ್ಥಿರ bool
box_ಒಳಹರಿವು(dom_ಗಂಟು *n,
	  html_content *content,
	  ಸಂರಚನಾ box *box,
	  bool *convert_children)
{
	ಸಂರಚನಾ form_control *gadget;
	dom_string *type = ರಿಕ್ತ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಯದಿ (gadget == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ಯದಿ ((err != DOM_NO_ERR) || (type == ರಿಕ್ತ)) {
		/* no type so "text" is assumed */
		ಯದಿ (box_ಒಳಹರಿವು_text(content, box, n) == false) {
			ಪ್ರತ್ಯಾಗಮ false;
		}
		*convert_children = false;
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಯದಿ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ಯದಿ (box_ಒಳಹರಿವು_text(content, box, n) == false)
			ಗಚ್ಛ no_memory;

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ಅನ್ಯಥಾ ಯದಿ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ಸಂರಚನಾ box *inline_container, *inline_box;

		ಯದಿ (box_button(n, content, box, 0) == false)
			ಗಚ್ಛ no_memory;

		inline_container = box_create(ರಿಕ್ತ, 0, false, 0, 0, 0, 0,
				content->bctx);
		ಯದಿ (inline_container == ರಿಕ್ತ)
			ಗಚ್ಛ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ರಿಕ್ತ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ಯದಿ (inline_box == ರಿಕ್ತ)
			ಗಚ್ಛ no_memory;

		inline_box->type = BOX_TEXT;

		ಯದಿ (box->gadget->ಮೌಲ್ಯ != ರಿಕ್ತ)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->ಮೌಲ್ಯ);
		ಅನ್ಯಥಾ ಯದಿ (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ಅನ್ಯಥಾ ಯದಿ (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ಅನ್ಯಥಾ
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		ಯದಿ (inline_box->text == ರಿಕ್ತ)
			ಗಚ್ಛ no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ಯದಿ (box->style && ns_computed_display(box->style,
				box_is_ಬೇರು(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ಯದಿ (err == DOM_NO_ERR && s != ರಿಕ್ತ) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				ಯದಿ (error != NSERROR_OK)
					ಗಚ್ಛ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ಯದಿ (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					ಯದಿ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						ಗಚ್ಛ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} ಅನ್ಯಥಾ {
		/* unhandled type the default is "text" */
		ಯದಿ (box_ಒಳಹರಿವು_text(content, box, n) == false)
			ಗಚ್ಛ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	ಪ್ರತ್ಯಾಗಮ true;

no_memory:
	dom_string_unref(type);

	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Noscript element
 */
ಸ್ಥಿರ bool
box_noscript(dom_ಗಂಟು *n,
	     html_content *content,
	     ಸಂರಚನಾ box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ಯದಿ (content->enable_scripting) {
		*convert_children = false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Generic embedded object [13.3].
 */
ಸ್ಥಿರ bool
box_object(dom_ಗಂಟು *n,
	   html_content *content,
	   ಸಂರಚನಾ box *box,
	   bool *convert_children)
{
	ಸಂರಚನಾ object_params *params;
	ಸಂರಚನಾ object_param *param;
	dom_string *codebase, *classid, *data;
	dom_ಗಂಟು *c;
	dom_exception err;

	ಯದಿ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ಸಂರಚನಾ object_params);
	ಯದಿ (params == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ರಿಕ್ತ;
	params->type = ರಿಕ್ತ;
	params->codetype = ರಿಕ್ತ;
	params->codebase = ರಿಕ್ತ;
	params->classid = ರಿಕ್ತ;
	params->params = ರಿಕ್ತ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ಯದಿ (err == DOM_NO_ERR && codebase != ರಿಕ್ತ) {
		ಯದಿ (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		dom_string_unref(codebase);
	}
	ಯದಿ (params->codebase == ರಿಕ್ತ)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ಯದಿ (err == DOM_NO_ERR && classid != ರಿಕ್ತ) {
		ಯದಿ (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	ಯದಿ (err == DOM_NO_ERR && data != ರಿಕ್ತ) {
		ಯದಿ (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		dom_string_unref(data);
	}

	ಯದಿ (params->classid == ರಿಕ್ತ && params->data == ರಿಕ್ತ)
		/* nothing to embed; ignore */
		ಪ್ರತ್ಯಾಗಮ true;

	/* Don't include ourself */
	ಯದಿ (params->classid != ರಿಕ್ತ && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (params->data != ರಿಕ್ತ && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		ಪ್ರತ್ಯಾಗಮ true;

	/* codetype and type are MIME types */
	ಯದಿ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (box_get_attribute(n, "type", params, &params->type) == false)
		ಪ್ರತ್ಯಾಗಮ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ಯದಿ (params->classid != ರಿಕ್ತ && params->data == ರಿಕ್ತ &&
			params->codetype != ರಿಕ್ತ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		ಯದಿ (lerror != lwc_error_ok)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		lwc_string_unref(icodetype);
	}

	ಯದಿ (params->data != ರಿಕ್ತ && params->type != ರಿಕ್ತ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		ಯದಿ (lerror != lwc_error_ok)
			ಪ್ರತ್ಯಾಗಮ false;

		ಯದಿ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ಪ್ರತ್ಯಾಗಮ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ಗಂಟು_get_first_child(n, &c);
	ಯದಿ (err != DOM_NO_ERR)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯಾವತ್ (c != ರಿಕ್ತ) {
		dom_ಗಂಟು *next;
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(c, &type);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (type == DOM_ELEMENT_ಗಂಟು) {
			dom_string *name;

			err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
			ಯದಿ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ಗಂಟು_unref(c);
				ಭಙ್ಗ;
			}
			dom_string_unref(name);

			param = talloc(params, ಸಂರಚನಾ object_param);
			ಯದಿ (param == ರಿಕ್ತ) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}
			param->name = ರಿಕ್ತ;
			param->ಮೌಲ್ಯ = ರಿಕ್ತ;
			param->type = ರಿಕ್ತ;
			param->valuetype = ರಿಕ್ತ;
			param->next = ರಿಕ್ತ;

			ಯದಿ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (box_get_attribute(c, "value", param,
					&param->ಮೌಲ್ಯ) == false) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ಗಂಟು_unref(c);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯದಿ (param->valuetype == ರಿಕ್ತ) {
				param->valuetype = talloc_strdup(param, "data");
				ಯದಿ (param->valuetype == ರಿಕ್ತ) {
					dom_ಗಂಟು_unref(c);
					ಪ್ರತ್ಯಾಗಮ false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_ಗಂಟು_get_next_sibling(c, &next);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		dom_ಗಂಟು_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ಯದಿ (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ಪ್ರತ್ಯಾಗಮ false;

	*convert_children = false;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Preformatted text [9.3.4].
 */
ಸ್ಥಿರ bool
box_pre(dom_ಗಂಟು *n,
	html_content *content,
	ಸಂರಚನಾ box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Option selector [17.6].
 */
ಸ್ಥಿರ bool
box_select(dom_ಗಂಟು *n,
	   html_content *content,
	   ಸಂರಚನಾ box *box,
	   bool *convert_children)
{
	ಸಂರಚನಾ box *inline_container;
	ಸಂರಚನಾ box *inline_box;
	ಸಂರಚನಾ form_control *gadget;
	dom_ಗಂಟು *c, *c2;
	dom_ಗಂಟು *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಯದಿ (gadget == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	gadget->html = content;
	err = dom_ಗಂಟು_get_first_child(n, &c);
	ಯದಿ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯಾವತ್ (c != ರಿಕ್ತ) {
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			form_free_control(gadget);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		ಯದಿ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ಯದಿ (box_select_add_option(gadget, c) == false) {
				dom_ಗಂಟು_unref(c);
				form_free_control(gadget);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		} ಅನ್ಯಥಾ ಯದಿ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ಗಂಟು_get_first_child(c, &c2);
			ಯದಿ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				form_free_control(gadget);
				ಪ್ರತ್ಯಾಗಮ false;
			}

			ಯಾವತ್ (c2 != ರಿಕ್ತ) {
				dom_string *c2_name;

				err = dom_ಗಂಟು_get_ಗಂಟು_name(c2, &c2_name);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c2);
					dom_ಗಂಟು_unref(c);
					form_free_control(gadget);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				ಯದಿ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ಯದಿ (box_select_add_option(gadget,
							c2) == false) {
						dom_ಗಂಟು_unref(c2);
						dom_ಗಂಟು_unref(c);
						form_free_control(gadget);
						ಪ್ರತ್ಯಾಗಮ false;
					}
				} ಅನ್ಯಥಾ {
					dom_string_unref(c2_name);
				}

				err = dom_ಗಂಟು_get_next_sibling(c2, &next2);
				ಯದಿ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c2);
					dom_ಗಂಟು_unref(c);
					form_free_control(gadget);
					ಪ್ರತ್ಯಾಗಮ false;
				}

				dom_ಗಂಟು_unref(c2);
				c2 = next2;
			}
		} ಅನ್ಯಥಾ {
			dom_string_unref(name);
		}

		err = dom_ಗಂಟು_get_next_sibling(c, &next);
		ಯದಿ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			form_free_control(gadget);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		dom_ಗಂಟು_unref(c);
		c = next;
	}

	ಯದಿ (gadget->data.select.ಅಂಕೆ_ವಸ್ತುಗಳು == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ರಿಕ್ತ, 0, false, 0, 0, 0, 0, content->bctx);
	ಯದಿ (inline_container == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ರಿಕ್ತ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ಯದಿ (inline_box == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	ಯದಿ (gadget->data.select.multiple == false &&
			gadget->data.select.ಅಂಕೆ_selected == 0) {
		gadget->data.select.current = gadget->data.select.ವಸ್ತುಗಳು;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.ಅಂಕೆ_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->ಗಂಟು, true);
	}

	ಯದಿ (gadget->data.select.ಅಂಕೆ_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ಅನ್ಯಥಾ ಯದಿ (gadget->data.select.ಅಂಕೆ_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	ಅನ್ಯಥಾ
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ಯದಿ (inline_box->text == ರಿಕ್ತ)
		ಗಚ್ಛ no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	ಪ್ರತ್ಯಾಗಮ true;

no_memory:
	ಪ್ರತ್ಯಾಗಮ false;
}


/**
 * Multi-line text field [17.7].
 */
ಸ್ಥಿರ bool box_textarea(dom_ಗಂಟು *n,
			html_content *content,
			ಸಂರಚನಾ box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಯದಿ (box->gadget == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ಯದಿ (!box_ಒಳಹರಿವು_text(content, box, n))
		ಪ್ರತ್ಯಾಗಮ false;

	*convert_children = false;
	ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ಗಂಟು *ಗಂಟು,
			 html_content *content,
			 ಸಂರಚನಾ box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(ಗಂಟು, &tag_type);
	ಯದಿ (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ವಿಕಲ್ಪ (tag_type) {
	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
		res = box_ಒಳಹರಿವು(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಪಕ್ಷ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ಗಂಟು, content, box, convert_children);
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		res = true;
	}

	ಪ್ರತ್ಯಾಗಮ res;
}
