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

#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव "utils/nsoption.h"
#अन्तर्भाव "utils/corestrings.h"
#अन्तर्भाव "utils/log.h"
#अन्तर्भाव "utils/messages.h"
#अन्तर्भाव "utils/talloc.h"
#अन्तर्भाव "utils/string.h"
#अन्तर्भाव "utils/ascii.h"
#अन्तर्भाव "utils/nsurl.h"
#अन्तर्भाव "netsurf/plot_style.h"
#अन्तर्भाव "css/hints.h"
#अन्तर्भाव "desktop/frame_types.h"
#अन्तर्भाव "content/content_factory.h"

#अन्तर्भाव "html/html.h"
#अन्तर्भाव "html/private.h"
#अन्तर्भाव "html/object.h"
#अन्तर्भाव "html/box.h"
#अन्तर्भाव "html/box_manipulate.h"
#अन्तर्भाव "html/box_construct.h"
#अन्तर्भाव "html/box_special.h"
#अन्तर्भाव "html/box_textarea.h"
#अन्तर्भाव "html/form_internal.h"


स्थिर ध्रुव content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
स्थिर अन्तःस्थ bool box_is_मूलम्(dom_ग्रन्थिः *n)
{
	dom_ग्रन्थिः *parent;
	dom_ग्रन्थिः_type type;
	dom_exception err;

	err = dom_ग्रन्थिः_get_parent_ग्रन्थिः(n, &parent);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	सचे (parent != रिक्त) {
		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(parent, &type);

		dom_ग्रन्थिः_unref(parent);

		सचे (err != DOM_NO_ERR)
			पटिनिवत्त false;

		सचे (type != DOM_DOCUMENT_ग्रन्थिः)
			पटिनिवत्त false;
	}

	पटिनिवत्त true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_object_talloc_destructor(संरचना object_params *o)
{
	सचे (o->codebase != रिक्त)
		nsurl_unref(o->codebase);
	सचे (o->classid != रिक्त)
		nsurl_unref(o->classid);
	सचे (o->data != रिक्त)
		nsurl_unref(o->data);

	पटिनिवत्त 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
स्थिर संरचना frame_dimension *
box_parse_multi_lengths(ध्रुव dom_string *ds, unsigned पूर्णाङ्क *गणनम्)
{
	वर्ण *end;
	unsigned पूर्णाङ्क i, n;
	संरचना frame_dimension *length;
	ध्रुव वर्ण *s;

	s = dom_string_data(ds);

	अत्थाय (i = 0, n = 1; s[i]; i++)
		सचे (s[i] == ',')
			n++;

	length = calloc(n, परिमाण(संरचना frame_dimension));
	सचे (!length)
		पटिनिवत्त रिक्त;

	अत्थाय (i = 0; i != n; i++) {
		याव (ascii_is_space(*s)) {
			s++;
		}
		length[i].मानम् = strtof(s, &end);
		सचे (length[i].मानम् <= 0) {
			length[i].मानम् = 1;
		}
		s = end;
		विकल्प (*s) {
			पक्ष '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				भङ्ग;
			पक्ष '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				भङ्ग;
			स्वभाव:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				भङ्ग;
		}
		याव (*s && *s != ',') {
			s++;
		}
		सचे (*s == ',') {
			s++;
		}
	}

	*गणनम् = n;
	पटिनिवत्त length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_frames_talloc_destructor(संरचना content_html_frames *f)
{
	सचे (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	पटिनिवत्त 0;
}


/**
 * create a frameset box tree
 */
स्थिर bool
box_create_frameset(संरचना content_html_frames *f,
		    dom_ग्रन्थिः *n,
		    html_content *content)
{
	unsigned पूर्णाङ्क row, col, index, i;
	unsigned पूर्णाङ्क rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	संरचना frame_dimension *row_height = 0, *col_width = 0;
	dom_ग्रन्थिः *c, *next;
	संरचना content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		सचे (row_height == रिक्त)
			पटिनिवत्त false;
	} अञ्ञथा {
		row_height = calloc(1, परिमाण(संरचना frame_dimension));
		सचे (row_height == रिक्त)
			पटिनिवत्त false;
		row_height->मानम् = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		सचे (col_width == रिक्त) {
			मुक्त(row_height);
			पटिनिवत्त false;
		}
	} अञ्ञथा {
		col_width = calloc(1, परिमाण(संरचना frame_dimension));
		सचे (col_width == रिक्त) {
			मुक्त(row_height);
			पटिनिवत्त false;
		}
		col_width->मानम् = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		सचे ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		सचे (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, संरचना content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	अत्थाय (row = 0; row < rows; row++) {
		अत्थाय (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = रिक्त;
			frame->url = रिक्त;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = रिक्त;
		}
	}
	मुक्त(col_width);
	मुक्त(row_height);

	/* create the frameset windows */
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	अत्थाय (row = 0; c != रिक्त && row < rows; row++) {
		अत्थाय (col = 0; c != रिक्त && col < cols; col++) {
			याव (c != रिक्त) {
				dom_ग्रन्थिः_type type;
				dom_string *name;

				err = dom_ग्रन्थिः_get_ग्रन्थिः_type(c, &type);
				सचे (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					पटिनिवत्त false;
				}

				err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
				सचे (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					पटिनिवत्त false;
				}

				सचे (type != DOM_ELEMENT_ग्रन्थिः ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ग्रन्थिः_get_next_sibling(c,
							&next);
					सचे (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ग्रन्थिः_unref(c);
						पटिनिवत्त false;
					}

					dom_string_unref(name);
					dom_ग्रन्थिः_unref(c);
					c = next;
				} अञ्ञथा {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					भङ्ग;
				}
			}

			सचे (c == रिक्त)
				भङ्ग;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &s);
			सचे (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				सचे (box_create_frameset(frame, c,
						content) == false) {
					dom_ग्रन्थिः_unref(c);
					पटिनिवत्त false;
				}

				err = dom_ग्रन्थिः_get_next_sibling(c, &next);
				सचे (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					पटिनिवत्त false;
				}

				dom_ग्रन्थिः_unref(c);
				c = next;
				अनुवर्त;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = रिक्त;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			सचे (url != रिक्त) {
				/* no self-references */
				सचे (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = रिक्त;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			सचे (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				सचे (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				अञ्ञथा सचे (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				css_color color;

				सचे (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ग्रन्थिः_get_next_sibling(c, &next);
			सचे (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			dom_ग्रन्थिः_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	सचे (c != रिक्त) {
		dom_ग्रन्थिः_unref(c);
	}

	पटिनिवत्त true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_iframes_talloc_destructor(संरचना content_html_iframe *f)
{
	सचे (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	पटिनिवत्त 0;
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
स्थिर bool
box_get_attribute(dom_ग्रन्थिः *n,
		  ध्रुव वर्ण *attribute,
		  शून्य *context,
		  वर्ण **मानम्)
{
	वर्ण *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ध्रुव uint8_t *) attribute,
			strlen(attribute), &attr_name);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	सचे (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		पटिनिवत्त false;
	}

	dom_string_unref(attr_name);

	सचे (attr != रिक्त) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		सचे (result == रिक्त)
			पटिनिवत्त false;

		*मानम् = result;
	}

	पटिनिवत्त true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
स्थिर bool
box_निवेशः_text(html_content *html, संरचना box *box, संरचना dom_ग्रन्थिः *ग्रन्थिः)
{
	संरचना box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	विकल्प (display) {
	पक्ष CSS_DISPLAY_GRID:
	पक्ष CSS_DISPLAY_FLEX:
	पक्ष CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		भङ्ग;
	स्वभाव:
		box->type = BOX_INLINE_BLOCK;
		भङ्ग;
	}

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, html->bctx);
	सचे (!inline_container)
		पटिनिवत्त false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	सचे (!inline_box)
		पटिनिवत्त false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	पटिनिवत्त box_textarea_create_textarea(html, box, ग्रन्थिः);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool box_select_add_option(संरचना form_control *control, dom_ग्रन्थिः *n)
{
	वर्ण *मानम् = रिक्त;
	वर्ण *text = रिक्त;
	वर्ण *text_nowrap = रिक्त;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ग्रन्थिः_get_text_content(n, &content);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	सचे (content != रिक्त) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} अञ्ञथा {
		text = strdup("");
	}

	सचे (text == रिक्त)
		गच्छ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_मानम्, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		मानम् = strdup(dom_string_data(s));
		dom_string_unref(s);
	} अञ्ञथा {
		मानम् = strdup(text);
	}

	सचे (मानम् == रिक्त)
		गच्छ no_memory;

	सचे (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	सचे (text_nowrap == रिक्त)
		गच्छ no_memory;

	सचे (form_add_option(control, मानम्, text_nowrap, selected, n) == false)
		गच्छ no_memory;

	मुक्त(text);

	पटिनिवत्त true;

no_memory:
	मुक्त(मानम्);
	मुक्त(text);
	मुक्त(text_nowrap);
	पटिनिवत्त false;
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
स्थिर bool
box_a(dom_ग्रन्थिः *n,
      html_content *content,
      संरचना box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		सचे (!ok)
			पटिनिवत्त false;
		सचे (url) {
			सचे (box->href != रिक्त)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		सचे (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		अञ्ञथा सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_शिखरम्_2))
			box->target = "_top";
		अञ्ञथा सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		अञ्ञथा सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		अञ्ञथा {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			सचे (!box->target) {
				dom_string_unref(s);
				पटिनिवत्त false;
			}
		}
		dom_string_unref(s);
	}

	पटिनिवत्त true;
}


/**
 * Document body special element handler [7.5.1].
 */
स्थिर bool
box_body(dom_ग्रन्थिः *n,
	 html_content *content,
	 संरचना box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	सचे (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} अञ्ञथा {
		content->background_colour = nscss_color_to_ns(color);
	}

	पटिनिवत्त true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
स्थिर bool
box_br(dom_ग्रन्थिः *n,
       html_content *content,
       संरचना box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	पटिनिवत्त true;
}


/**
 * special element handler for Push button [17.5].
 */
स्थिर bool
box_button(dom_ग्रन्थिः *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना form_control *gadget;

	gadget = html_forms_get_control_for_ग्रन्थिः(content->forms, n);
	सचे (!gadget)
		पटिनिवत्त false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	पटिनिवत्त true;
}


/**
 * Canvas element
 */
स्थिर bool
box_canvas(dom_ग्रन्थिः *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	सचे (!content->enable_scripting) {
		पटिनिवत्त true;
	}
	*convert_children = false;

	सचे (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		पटिनिवत्त true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	पटिनिवत्त true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
स्थिर bool
box_embed(dom_ग्रन्थिः *n,
	  html_content *content,
	  संरचना box *box,
	  bool *convert_children)
{
	संरचना object_params *params;
	संरचना object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t अङ्कः_attrs;
	dom_string *src;
	dom_exception err;

	सचे (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		पटिनिवत्त true;

	params = talloc(content->bctx, संरचना object_params);
	सचे (params == रिक्त)
		पटिनिवत्त false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = रिक्त;
	params->type = रिक्त;
	params->codetype = रिक्त;
	params->codebase = रिक्त;
	params->classid = रिक्त;
	params->params = रिक्त;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	सचे (err != DOM_NO_ERR || src == रिक्त)
		पटिनिवत्त true;
	सचे (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		पटिनिवत्त false;
	}

	dom_string_unref(src);

	सचे (params->data == रिक्त)
		पटिनिवत्त true;

	/* Don't include ourself */
	सचे (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		पटिनिवत्त true;

	/* add attributes as parameters to linked list */
	err = dom_ग्रन्थिः_get_attributes(n, &attrs);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	err = dom_namednodemap_get_length(attrs, &अङ्कः_attrs);
	सचे (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		पटिनिवत्त false;
	}

	अत्थाय (idx = 0; idx < अङ्कः_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *मानम्;

		err = dom_namednodemap_item(attrs, idx, (शून्य *) &attr);
		सचे (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			पटिनिवत्त false;
		}

		err = dom_attr_get_name(attr, &name);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(attr);
			dom_namednodemap_unref(attrs);
			पटिनिवत्त false;
		}

		सचे (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(name);
			अनुवर्त;
		}

		err = dom_attr_get_मानम्(attr, &मानम्);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			पटिनिवत्त false;
		}

		param = talloc(content->bctx, संरचना object_param);
		सचे (param == रिक्त) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(मानम्);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			पटिनिवत्त false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->मानम् = talloc_strdup(content->bctx, dom_string_data(मानम्));
		param->type = रिक्त;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = रिक्त;

		dom_string_unref(मानम्);
		dom_string_unref(name);
		dom_ग्रन्थिः_unref(attr);

		सचे (param->name == रिक्त || param->मानम् == रिक्त ||
				param->valuetype == रिक्त) {
			dom_namednodemap_unref(attrs);
			पटिनिवत्त false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	पटिनिवत्त html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
स्थिर bool
box_frameset(dom_ग्रन्थिः *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	bool ok;

	सचे (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		सचे (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		पटिनिवत्त true;
	}

	content->frameset = talloc_zero(content->bctx,
					संरचना content_html_frames);
	सचे (!content->frameset) {
		पटिनिवत्त false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	सचे (ok) {
		box->type = BOX_NONE;
	}

	सचे (convert_children) {
		*convert_children = false;
	}
	पटिनिवत्त ok;
}


/**
 * Inline subwindow [16.5].
 */
स्थिर bool
box_iframe(dom_ग्रन्थिः *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	संरचना content_html_iframe *iframe;
	पूर्णाङ्क i;

	सचे (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		पटिनिवत्त true;

	सचे (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		पटिनिवत्त true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	सचे (err != DOM_NO_ERR || s == रिक्त)
		पटिनिवत्त true;
	सचे (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		पटिनिवत्त false;
	}
	dom_string_unref(s);
	सचे (url == रिक्त)
		पटिनिवत्त true;

	/* don't include ourself */
	सचे (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		पटिनिवत्त true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, संरचना content_html_iframe);
	सचे (iframe == रिक्त) {
		nsurl_unref(url);
		पटिनिवत्त false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = रिक्त;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		सचे (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		अञ्ञथा सचे (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	प्रतिज्ञा(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	सचे (convert_children)
		*convert_children = false;
	पटिनिवत्त true;
}


/**
 * Embedded image [13.2].
 */
स्थिर bool
box_image(dom_ग्रन्थिः *n,
	  html_content *content,
	  संरचना box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	गणना css_width_e wtype;
	गणना css_height_e htype;
	css_fixed मानम् = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	सचे (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		पटिनिवत्त true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	सचे (err == DOM_NO_ERR && s != रिक्त) {
		वर्ण *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		सचे (alt == रिक्त)
			पटिनिवत्त false;
		box->text = talloc_strdup(content->bctx, alt);
		मुक्त(alt);
		सचे (box->text == रिक्त)
			पटिनिवत्त false;
		box->length = strlen(box->text);
	}

	सचे (nsoption_bool(foreground_images) == false) {
		पटिनिवत्त true;
	}

	/* imagemap associated with this image */
	सचे (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		पटिनिवत्त false;
	सचे (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	सचे (err != DOM_NO_ERR || s == रिक्त)
		पटिनिवत्त true;

	सचे (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		पटिनिवत्त false;
	}

	dom_string_unref(s);

	सचे (url == रिक्त)
		पटिनिवत्त true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &मानम्, &wunit);
	htype = css_computed_height(box->style, &मानम्, &hunit);

	सचे (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	पटिनिवत्त ok;
}


/**
 * Form control [17.4].
 */
स्थिर bool
box_निवेशः(dom_ग्रन्थिः *n,
	  html_content *content,
	  संरचना box *box,
	  bool *convert_children)
{
	संरचना form_control *gadget;
	dom_string *type = रिक्त;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ग्रन्थिः(content->forms, n);
	सचे (gadget == रिक्त) {
		पटिनिवत्त false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	सचे ((err != DOM_NO_ERR) || (type == रिक्त)) {
		/* no type so "text" is assumed */
		सचे (box_निवेशः_text(content, box, n) == false) {
			पटिनिवत्त false;
		}
		*convert_children = false;
		पटिनिवत्त true;
	}

	सचे (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		सचे (box_निवेशः_text(content, box, n) == false)
			गच्छ no_memory;

	} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} अञ्ञथा सचे ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		संरचना box *inline_container, *inline_box;

		सचे (box_button(n, content, box, 0) == false)
			गच्छ no_memory;

		inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0,
				content->bctx);
		सचे (inline_container == रिक्त)
			गच्छ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(रिक्त, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		सचे (inline_box == रिक्त)
			गच्छ no_memory;

		inline_box->type = BOX_TEXT;

		सचे (box->gadget->मानम् != रिक्त)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->मानम्);
		अञ्ञथा सचे (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		अञ्ञथा सचे (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		अञ्ञथा
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		सचे (inline_box->text == रिक्त)
			गच्छ no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		सचे (box->style && ns_computed_display(box->style,
				box_is_मूलम्(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			सचे (err == DOM_NO_ERR && s != रिक्त) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				सचे (error != NSERROR_OK)
					गच्छ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				सचे (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					सचे (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						गच्छ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} अञ्ञथा {
		/* unhandled type the default is "text" */
		सचे (box_निवेशः_text(content, box, n) == false)
			गच्छ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	पटिनिवत्त true;

no_memory:
	dom_string_unref(type);

	पटिनिवत्त false;
}


/**
 * Noscript element
 */
स्थिर bool
box_noscript(dom_ग्रन्थिः *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	सचे (content->enable_scripting) {
		*convert_children = false;
	}

	पटिनिवत्त true;
}


/**
 * Generic embedded object [13.3].
 */
स्थिर bool
box_object(dom_ग्रन्थिः *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना object_params *params;
	संरचना object_param *param;
	dom_string *codebase, *classid, *data;
	dom_ग्रन्थिः *c;
	dom_exception err;

	सचे (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		पटिनिवत्त true;

	सचे (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		पटिनिवत्त false;
	सचे (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, संरचना object_params);
	सचे (params == रिक्त)
		पटिनिवत्त false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = रिक्त;
	params->type = रिक्त;
	params->codetype = रिक्त;
	params->codebase = रिक्त;
	params->classid = रिक्त;
	params->params = रिक्त;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	सचे (err == DOM_NO_ERR && codebase != रिक्त) {
		सचे (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			पटिनिवत्त false;
		}
		dom_string_unref(codebase);
	}
	सचे (params->codebase == रिक्त)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	सचे (err == DOM_NO_ERR && classid != रिक्त) {
		सचे (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			पटिनिवत्त false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	सचे (err == DOM_NO_ERR && data != रिक्त) {
		सचे (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			पटिनिवत्त false;
		}
		dom_string_unref(data);
	}

	सचे (params->classid == रिक्त && params->data == रिक्त)
		/* nothing to embed; ignore */
		पटिनिवत्त true;

	/* Don't include ourself */
	सचे (params->classid != रिक्त && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		पटिनिवत्त true;

	सचे (params->data != रिक्त && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		पटिनिवत्त true;

	/* codetype and type are MIME types */
	सचे (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		पटिनिवत्त false;
	सचे (box_get_attribute(n, "type", params, &params->type) == false)
		पटिनिवत्त false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	सचे (params->classid != रिक्त && params->data == रिक्त &&
			params->codetype != रिक्त) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		सचे (lerror != lwc_error_ok)
			पटिनिवत्त false;

		सचे (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			पटिनिवत्त true;
		}

		lwc_string_unref(icodetype);
	}

	सचे (params->data != रिक्त && params->type != रिक्त) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		सचे (lerror != lwc_error_ok)
			पटिनिवत्त false;

		सचे (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			पटिनिवत्त true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	सचे (err != DOM_NO_ERR)
		पटिनिवत्त false;

	याव (c != रिक्त) {
		dom_ग्रन्थिः *next;
		dom_ग्रन्थिः_type type;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(c, &type);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			पटिनिवत्त false;
		}

		सचे (type == DOM_ELEMENT_ग्रन्थिः) {
			dom_string *name;

			err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
			सचे (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ग्रन्थिः_unref(c);
				भङ्ग;
			}
			dom_string_unref(name);

			param = talloc(params, संरचना object_param);
			सचे (param == रिक्त) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}
			param->name = रिक्त;
			param->मानम् = रिक्त;
			param->type = रिक्त;
			param->valuetype = रिक्त;
			param->next = रिक्त;

			सचे (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (box_get_attribute(c, "value", param,
					&param->मानम्) == false) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ग्रन्थिः_unref(c);
				पटिनिवत्त false;
			}

			सचे (param->valuetype == रिक्त) {
				param->valuetype = talloc_strdup(param, "data");
				सचे (param->valuetype == रिक्त) {
					dom_ग्रन्थिः_unref(c);
					पटिनिवत्त false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_ग्रन्थिः_get_next_sibling(c, &next);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			पटिनिवत्त false;
		}

		dom_ग्रन्थिः_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	सचे (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		पटिनिवत्त false;

	*convert_children = false;
	पटिनिवत्त true;
}


/**
 * Preformatted text [9.3.4].
 */
स्थिर bool
box_pre(dom_ग्रन्थिः *n,
	html_content *content,
	संरचना box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	पटिनिवत्त true;
}


/**
 * Option selector [17.6].
 */
स्थिर bool
box_select(dom_ग्रन्थिः *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना box *inline_container;
	संरचना box *inline_box;
	संरचना form_control *gadget;
	dom_ग्रन्थिः *c, *c2;
	dom_ग्रन्थिः *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ग्रन्थिः(content->forms, n);
	सचे (gadget == रिक्त)
		पटिनिवत्त false;

	gadget->html = content;
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	सचे (err != DOM_NO_ERR) {
		form_free_control(gadget);
		पटिनिवत्त false;
	}

	याव (c != रिक्त) {
		dom_string *name;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			form_free_control(gadget);
			पटिनिवत्त false;
		}

		सचे (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			सचे (box_select_add_option(gadget, c) == false) {
				dom_ग्रन्थिः_unref(c);
				form_free_control(gadget);
				पटिनिवत्त false;
			}
		} अञ्ञथा सचे (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ग्रन्थिः_get_first_child(c, &c2);
			सचे (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				form_free_control(gadget);
				पटिनिवत्त false;
			}

			याव (c2 != रिक्त) {
				dom_string *c2_name;

				err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c2, &c2_name);
				सचे (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c2);
					dom_ग्रन्थिः_unref(c);
					form_free_control(gadget);
					पटिनिवत्त false;
				}

				सचे (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					सचे (box_select_add_option(gadget,
							c2) == false) {
						dom_ग्रन्थिः_unref(c2);
						dom_ग्रन्थिः_unref(c);
						form_free_control(gadget);
						पटिनिवत्त false;
					}
				} अञ्ञथा {
					dom_string_unref(c2_name);
				}

				err = dom_ग्रन्थिः_get_next_sibling(c2, &next2);
				सचे (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c2);
					dom_ग्रन्थिः_unref(c);
					form_free_control(gadget);
					पटिनिवत्त false;
				}

				dom_ग्रन्थिः_unref(c2);
				c2 = next2;
			}
		} अञ्ञथा {
			dom_string_unref(name);
		}

		err = dom_ग्रन्थिः_get_next_sibling(c, &next);
		सचे (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			form_free_control(gadget);
			पटिनिवत्त false;
		}

		dom_ग्रन्थिः_unref(c);
		c = next;
	}

	सचे (gadget->data.select.अङ्कः_वस्तूनि == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		पटिनिवत्त true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, content->bctx);
	सचे (inline_container == रिक्त)
		गच्छ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	सचे (inline_box == रिक्त)
		गच्छ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	सचे (gadget->data.select.multiple == false &&
			gadget->data.select.अङ्कः_selected == 0) {
		gadget->data.select.current = gadget->data.select.वस्तूनि;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.अङ्कः_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->ग्रन्थिः, true);
	}

	सचे (gadget->data.select.अङ्कः_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	अञ्ञथा सचे (gadget->data.select.अङ्कः_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	अञ्ञथा
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	सचे (inline_box->text == रिक्त)
		गच्छ no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	पटिनिवत्त true;

no_memory:
	पटिनिवत्त false;
}


/**
 * Multi-line text field [17.7].
 */
स्थिर bool box_textarea(dom_ग्रन्थिः *n,
			html_content *content,
			संरचना box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ग्रन्थिः(content->forms, n);
	सचे (box->gadget == रिक्त)
		पटिनिवत्त false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	सचे (!box_निवेशः_text(content, box, n))
		पटिनिवत्त false;

	*convert_children = false;
	पटिनिवत्त true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ग्रन्थिः *ग्रन्थिः,
			 html_content *content,
			 संरचना box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(ग्रन्थिः, &tag_type);
	सचे (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	विकल्प (tag_type) {
	पक्ष DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_निवेशः:
		res = box_निवेशः(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	पक्ष DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ग्रन्थिः, content, box, convert_children);
		भङ्ग;

	स्वभाव:
		res = true;
	}

	पटिनिवत्त res;
}
