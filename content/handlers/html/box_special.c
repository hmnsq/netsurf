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

#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ "utils/nsoption.h"
#અન્તર્ભાવ "utils/corestrings.h"
#અન્તર્ભાવ "utils/log.h"
#અન્તર્ભાવ "utils/messages.h"
#અન્તર્ભાવ "utils/talloc.h"
#અન્તર્ભાવ "utils/string.h"
#અન્તર્ભાવ "utils/ascii.h"
#અન્તર્ભાવ "utils/nsurl.h"
#અન્તર્ભાવ "netsurf/plot_style.h"
#અન્તર્ભાવ "css/hints.h"
#અન્તર્ભાવ "desktop/frame_types.h"
#અન્તર્ભાવ "content/content_factory.h"

#અન્તર્ભાવ "html/html.h"
#અન્તર્ભાવ "html/private.h"
#અન્તર્ભાવ "html/object.h"
#અન્તર્ભાવ "html/box.h"
#અન્તર્ભાવ "html/box_manipulate.h"
#અન્તર્ભાવ "html/box_construct.h"
#અન્તર્ભાવ "html/box_special.h"
#અન્તર્ભાવ "html/box_textarea.h"
#અન્તર્ભાવ "html/form_internal.h"


સ્થિર અચલ content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
સ્થિર અંતઃરેખ bool box_is_મૂળ(dom_ગાંઠ *n)
{
	dom_ગાંઠ *parent;
	dom_ગાંઠ_type type;
	dom_exception err;

	err = dom_ગાંઠ_get_parent_ગાંઠ(n, &parent);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (parent != ખાલી) {
		err = dom_ગાંઠ_get_ગાંઠ_type(parent, &type);

		dom_ગાંઠ_unref(parent);

		જો (err != DOM_NO_ERR)
			પાછા false;

		જો (type != DOM_DOCUMENT_ગાંઠ)
			પાછા false;
	}

	પાછા true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
સ્થિર પૂર્ણાંક box_object_talloc_destructor(સંરચના object_params *o)
{
	જો (o->codebase != ખાલી)
		nsurl_unref(o->codebase);
	જો (o->classid != ખાલી)
		nsurl_unref(o->classid);
	જો (o->data != ખાલી)
		nsurl_unref(o->data);

	પાછા 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
સ્થિર સંરચના frame_dimension *
box_parse_multi_lengths(અચલ dom_string *ds, unsigned પૂર્ણાંક *ગણતરી)
{
	અક્ષર *end;
	unsigned પૂર્ણાંક i, n;
	સંરચના frame_dimension *length;
	અચલ અક્ષર *s;

	s = dom_string_data(ds);

	માટે (i = 0, n = 1; s[i]; i++)
		જો (s[i] == ',')
			n++;

	length = calloc(n, કદ(સંરચના frame_dimension));
	જો (!length)
		પાછા ખાલી;

	માટે (i = 0; i != n; i++) {
		જ્યારે (ascii_is_space(*s)) {
			s++;
		}
		length[i].કિંમત = strtof(s, &end);
		જો (length[i].કિંમત <= 0) {
			length[i].કિંમત = 1;
		}
		s = end;
		પસંદગી (*s) {
			સ્થિતિ '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				તોડો;
			સ્થિતિ '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				તોડો;
			મૂળભૂત:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				તોડો;
		}
		જ્યારે (*s && *s != ',') {
			s++;
		}
		જો (*s == ',') {
			s++;
		}
	}

	*ગણતરી = n;
	પાછા length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
સ્થિર પૂર્ણાંક box_frames_talloc_destructor(સંરચના content_html_frames *f)
{
	જો (f->url != ખાલી) {
		nsurl_unref(f->url);
		f->url = ખાલી;
	}

	પાછા 0;
}


/**
 * create a frameset box tree
 */
સ્થિર bool
box_create_frameset(સંરચના content_html_frames *f,
		    dom_ગાંઠ *n,
		    html_content *content)
{
	unsigned પૂર્ણાંક row, col, index, i;
	unsigned પૂર્ણાંક rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	સંરચના frame_dimension *row_height = 0, *col_width = 0;
	dom_ગાંઠ *c, *next;
	સંરચના content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		જો (row_height == ખાલી)
			પાછા false;
	} નહીંતર {
		row_height = calloc(1, કદ(સંરચના frame_dimension));
		જો (row_height == ખાલી)
			પાછા false;
		row_height->કિંમત = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		જો (col_width == ખાલી) {
			મુક્ત(row_height);
			પાછા false;
		}
	} નહીંતર {
		col_width = calloc(1, કદ(સંરચના frame_dimension));
		જો (col_width == ખાલી) {
			મુક્ત(row_height);
			પાછા false;
		}
		col_width->કિંમત = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		જો ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		css_color color;

		જો (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, સંરચના content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	માટે (row = 0; row < rows; row++) {
		માટે (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = ખાલી;
			frame->url = ખાલી;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = ખાલી;
		}
	}
	મુક્ત(col_width);
	મુક્ત(row_height);

	/* create the frameset windows */
	err = dom_ગાંઠ_get_first_child(n, &c);
	જો (err != DOM_NO_ERR)
		પાછા false;

	માટે (row = 0; c != ખાલી && row < rows; row++) {
		માટે (col = 0; c != ખાલી && col < cols; col++) {
			જ્યારે (c != ખાલી) {
				dom_ગાંઠ_type type;
				dom_string *name;

				err = dom_ગાંઠ_get_ગાંઠ_type(c, &type);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(c);
					પાછા false;
				}

				err = dom_ગાંઠ_get_ગાંઠ_name(c, &name);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(c);
					પાછા false;
				}

				જો (type != DOM_ELEMENT_ગાંઠ ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ગાંઠ_get_next_sibling(c,
							&next);
					જો (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ગાંઠ_unref(c);
						પાછા false;
					}

					dom_string_unref(name);
					dom_ગાંઠ_unref(c);
					c = next;
				} નહીંતર {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					તોડો;
				}
			}

			જો (c == ખાલી)
				તોડો;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_ગાંઠ_get_ગાંઠ_name(c, &s);
			જો (err != DOM_NO_ERR) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				જો (box_create_frameset(frame, c,
						content) == false) {
					dom_ગાંઠ_unref(c);
					પાછા false;
				}

				err = dom_ગાંઠ_get_next_sibling(c, &next);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(c);
					પાછા false;
				}

				dom_ગાંઠ_unref(c);
				c = next;
				ચાલુ;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ખાલી;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			જો (url != ખાલી) {
				/* no self-references */
				જો (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = ખાલી;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			જો (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				જો (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				નહીંતર જો (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				css_color color;

				જો (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ગાંઠ_get_next_sibling(c, &next);
			જો (err != DOM_NO_ERR) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			dom_ગાંઠ_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	જો (c != ખાલી) {
		dom_ગાંઠ_unref(c);
	}

	પાછા true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
સ્થિર પૂર્ણાંક box_iframes_talloc_destructor(સંરચના content_html_iframe *f)
{
	જો (f->url != ખાલી) {
		nsurl_unref(f->url);
		f->url = ખાલી;
	}

	પાછા 0;
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
સ્થિર bool
box_get_attribute(dom_ગાંઠ *n,
		  અચલ અક્ષર *attribute,
		  શૂન્ય *context,
		  અક્ષર **કિંમત)
{
	અક્ષર *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((અચલ uint8_t *) attribute,
			strlen(attribute), &attr_name);
	જો (err != DOM_NO_ERR)
		પાછા false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	જો (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		પાછા false;
	}

	dom_string_unref(attr_name);

	જો (attr != ખાલી) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		જો (result == ખાલી)
			પાછા false;

		*કિંમત = result;
	}

	પાછા true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
સ્થિર bool
box_આગત_text(html_content *html, સંરચના box *box, સંરચના dom_ગાંઠ *ગાંઠ)
{
	સંરચના box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	પસંદગી (display) {
	સ્થિતિ CSS_DISPLAY_GRID:
	સ્થિતિ CSS_DISPLAY_FLEX:
	સ્થિતિ CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		તોડો;
	મૂળભૂત:
		box->type = BOX_INLINE_BLOCK;
		તોડો;
	}

	inline_container = box_create(ખાલી, 0, false, 0, 0, 0, 0, html->bctx);
	જો (!inline_container)
		પાછા false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ખાલી, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	જો (!inline_box)
		પાછા false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	પાછા box_textarea_create_textarea(html, box, ગાંઠ);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
સ્થિર bool box_select_add_option(સંરચના form_control *control, dom_ગાંઠ *n)
{
	અક્ષર *કિંમત = ખાલી;
	અક્ષર *text = ખાલી;
	અક્ષર *text_nowrap = ખાલી;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ગાંઠ_get_text_content(n, &content);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જો (content != ખાલી) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} નહીંતર {
		text = strdup("");
	}

	જો (text == ખાલી)
		જાઓ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_કિંમત, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		કિંમત = strdup(dom_string_data(s));
		dom_string_unref(s);
	} નહીંતર {
		કિંમત = strdup(text);
	}

	જો (કિંમત == ખાલી)
		જાઓ no_memory;

	જો (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	જો (text_nowrap == ખાલી)
		જાઓ no_memory;

	જો (form_add_option(control, કિંમત, text_nowrap, selected, n) == false)
		જાઓ no_memory;

	મુક્ત(text);

	પાછા true;

no_memory:
	મુક્ત(કિંમત);
	મુક્ત(text);
	મુક્ત(text_nowrap);
	પાછા false;
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
સ્થિર bool
box_a(dom_ગાંઠ *n,
      html_content *content,
      સંરચના box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		જો (!ok)
			પાછા false;
		જો (url) {
			જો (box->href != ખાલી)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		જો (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		નહીંતર જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ટોચ_2))
			box->target = "_top";
		નહીંતર જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		નહીંતર જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		નહીંતર {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			જો (!box->target) {
				dom_string_unref(s);
				પાછા false;
			}
		}
		dom_string_unref(s);
	}

	પાછા true;
}


/**
 * Document body special element handler [7.5.1].
 */
સ્થિર bool
box_body(dom_ગાંઠ *n,
	 html_content *content,
	 સંરચના box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	જો (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} નહીંતર {
		content->background_colour = nscss_color_to_ns(color);
	}

	પાછા true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
સ્થિર bool
box_br(dom_ગાંઠ *n,
       html_content *content,
       સંરચના box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	પાછા true;
}


/**
 * special element handler for Push button [17.5].
 */
સ્થિર bool
box_button(dom_ગાંઠ *n,
	   html_content *content,
	   સંરચના box *box,
	   bool *convert_children)
{
	સંરચના form_control *gadget;

	gadget = html_forms_get_control_for_ગાંઠ(content->forms, n);
	જો (!gadget)
		પાછા false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	પાછા true;
}


/**
 * Canvas element
 */
સ્થિર bool
box_canvas(dom_ગાંઠ *n,
	     html_content *content,
	     સંરચના box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	જો (!content->enable_scripting) {
		પાછા true;
	}
	*convert_children = false;

	જો (box->style && ns_computed_display(box->style,
			box_is_મૂળ(n)) == CSS_DISPLAY_NONE)
		પાછા true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	પાછા true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
સ્થિર bool
box_embed(dom_ગાંઠ *n,
	  html_content *content,
	  સંરચના box *box,
	  bool *convert_children)
{
	સંરચના object_params *params;
	સંરચના object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t અંક_attrs;
	dom_string *src;
	dom_exception err;

	જો (box->style && ns_computed_display(box->style,
			box_is_મૂળ(n)) == CSS_DISPLAY_NONE)
		પાછા true;

	params = talloc(content->bctx, સંરચના object_params);
	જો (params == ખાલી)
		પાછા false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ખાલી;
	params->type = ખાલી;
	params->codetype = ખાલી;
	params->codebase = ખાલી;
	params->classid = ખાલી;
	params->params = ખાલી;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	જો (err != DOM_NO_ERR || src == ખાલી)
		પાછા true;
	જો (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		પાછા false;
	}

	dom_string_unref(src);

	જો (params->data == ખાલી)
		પાછા true;

	/* Don't include ourself */
	જો (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		પાછા true;

	/* add attributes as parameters to linked list */
	err = dom_ગાંઠ_get_attributes(n, &attrs);
	જો (err != DOM_NO_ERR)
		પાછા false;

	err = dom_namednodemap_get_length(attrs, &અંક_attrs);
	જો (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		પાછા false;
	}

	માટે (idx = 0; idx < અંક_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *કિંમત;

		err = dom_namednodemap_item(attrs, idx, (શૂન્ય *) &attr);
		જો (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			પાછા false;
		}

		err = dom_attr_get_name(attr, &name);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(attr);
			dom_namednodemap_unref(attrs);
			પાછા false;
		}

		જો (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ગાંઠ_unref(attr);
			dom_string_unref(name);
			ચાલુ;
		}

		err = dom_attr_get_કિંમત(attr, &કિંમત);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			પાછા false;
		}

		param = talloc(content->bctx, સંરચના object_param);
		જો (param == ખાલી) {
			dom_ગાંઠ_unref(attr);
			dom_string_unref(કિંમત);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			પાછા false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->કિંમત = talloc_strdup(content->bctx, dom_string_data(કિંમત));
		param->type = ખાલી;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = ખાલી;

		dom_string_unref(કિંમત);
		dom_string_unref(name);
		dom_ગાંઠ_unref(attr);

		જો (param->name == ખાલી || param->કિંમત == ખાલી ||
				param->valuetype == ખાલી) {
			dom_namednodemap_unref(attrs);
			પાછા false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	પાછા html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
સ્થિર bool
box_frameset(dom_ગાંઠ *n,
	     html_content *content,
	     સંરચના box *box,
	     bool *convert_children)
{
	bool ok;

	જો (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		જો (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		પાછા true;
	}

	content->frameset = talloc_zero(content->bctx,
					સંરચના content_html_frames);
	જો (!content->frameset) {
		પાછા false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	જો (ok) {
		box->type = BOX_NONE;
	}

	જો (convert_children) {
		*convert_children = false;
	}
	પાછા ok;
}


/**
 * Inline subwindow [16.5].
 */
સ્થિર bool
box_iframe(dom_ગાંઠ *n,
	   html_content *content,
	   સંરચના box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	સંરચના content_html_iframe *iframe;
	પૂર્ણાંક i;

	જો (box->style && ns_computed_display(box->style,
			box_is_મૂળ(n)) == CSS_DISPLAY_NONE)
		પાછા true;

	જો (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		પાછા true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	જો (err != DOM_NO_ERR || s == ખાલી)
		પાછા true;
	જો (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		પાછા false;
	}
	dom_string_unref(s);
	જો (url == ખાલી)
		પાછા true;

	/* don't include ourself */
	જો (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		પાછા true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, સંરચના content_html_iframe);
	જો (iframe == ખાલી) {
		nsurl_unref(url);
		પાછા false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = ખાલી;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		css_color color;

		જો (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		નહીંતર જો (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	પ્રતિજ્ઞા(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	જો (convert_children)
		*convert_children = false;
	પાછા true;
}


/**
 * Embedded image [13.2].
 */
સ્થિર bool
box_image(dom_ગાંઠ *n,
	  html_content *content,
	  સંરચના box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ગણના css_width_e wtype;
	ગણના css_height_e htype;
	css_fixed કિંમત = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	જો (box->style && ns_computed_display(box->style,
			box_is_મૂળ(n)) == CSS_DISPLAY_NONE)
		પાછા true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	જો (err == DOM_NO_ERR && s != ખાલી) {
		અક્ષર *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		જો (alt == ખાલી)
			પાછા false;
		box->text = talloc_strdup(content->bctx, alt);
		મુક્ત(alt);
		જો (box->text == ખાલી)
			પાછા false;
		box->length = strlen(box->text);
	}

	જો (nsoption_bool(foreground_images) == false) {
		પાછા true;
	}

	/* imagemap associated with this image */
	જો (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		પાછા false;
	જો (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	જો (err != DOM_NO_ERR || s == ખાલી)
		પાછા true;

	જો (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		પાછા false;
	}

	dom_string_unref(s);

	જો (url == ખાલી)
		પાછા true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &કિંમત, &wunit);
	htype = css_computed_height(box->style, &કિંમત, &hunit);

	જો (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	પાછા ok;
}


/**
 * Form control [17.4].
 */
સ્થિર bool
box_આગત(dom_ગાંઠ *n,
	  html_content *content,
	  સંરચના box *box,
	  bool *convert_children)
{
	સંરચના form_control *gadget;
	dom_string *type = ખાલી;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ગાંઠ(content->forms, n);
	જો (gadget == ખાલી) {
		પાછા false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	જો ((err != DOM_NO_ERR) || (type == ખાલી)) {
		/* no type so "text" is assumed */
		જો (box_આગત_text(content, box, n) == false) {
			પાછા false;
		}
		*convert_children = false;
		પાછા true;
	}

	જો (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		જો (box_આગત_text(content, box, n) == false)
			જાઓ no_memory;

	} નહીંતર જો (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} નહીંતર જો (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} નહીંતર જો ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} નહીંતર જો (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		સંરચના box *inline_container, *inline_box;

		જો (box_button(n, content, box, 0) == false)
			જાઓ no_memory;

		inline_container = box_create(ખાલી, 0, false, 0, 0, 0, 0,
				content->bctx);
		જો (inline_container == ખાલી)
			જાઓ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ખાલી, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		જો (inline_box == ખાલી)
			જાઓ no_memory;

		inline_box->type = BOX_TEXT;

		જો (box->gadget->કિંમત != ખાલી)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->કિંમત);
		નહીંતર જો (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		નહીંતર જો (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		નહીંતર
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		જો (inline_box->text == ખાલી)
			જાઓ no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} નહીંતર જો (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		જો (box->style && ns_computed_display(box->style,
				box_is_મૂળ(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			જો (err == DOM_NO_ERR && s != ખાલી) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				જો (error != NSERROR_OK)
					જાઓ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				જો (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					જો (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						જાઓ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} નહીંતર {
		/* unhandled type the default is "text" */
		જો (box_આગત_text(content, box, n) == false)
			જાઓ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	પાછા true;

no_memory:
	dom_string_unref(type);

	પાછા false;
}


/**
 * Noscript element
 */
સ્થિર bool
box_noscript(dom_ગાંઠ *n,
	     html_content *content,
	     સંરચના box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	જો (content->enable_scripting) {
		*convert_children = false;
	}

	પાછા true;
}


/**
 * Generic embedded object [13.3].
 */
સ્થિર bool
box_object(dom_ગાંઠ *n,
	   html_content *content,
	   સંરચના box *box,
	   bool *convert_children)
{
	સંરચના object_params *params;
	સંરચના object_param *param;
	dom_string *codebase, *classid, *data;
	dom_ગાંઠ *c;
	dom_exception err;

	જો (box->style && ns_computed_display(box->style,
			box_is_મૂળ(n)) == CSS_DISPLAY_NONE)
		પાછા true;

	જો (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		પાછા false;
	જો (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, સંરચના object_params);
	જો (params == ખાલી)
		પાછા false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ખાલી;
	params->type = ખાલી;
	params->codetype = ખાલી;
	params->codebase = ખાલી;
	params->classid = ખાલી;
	params->params = ખાલી;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	જો (err == DOM_NO_ERR && codebase != ખાલી) {
		જો (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			પાછા false;
		}
		dom_string_unref(codebase);
	}
	જો (params->codebase == ખાલી)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	જો (err == DOM_NO_ERR && classid != ખાલી) {
		જો (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			પાછા false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	જો (err == DOM_NO_ERR && data != ખાલી) {
		જો (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			પાછા false;
		}
		dom_string_unref(data);
	}

	જો (params->classid == ખાલી && params->data == ખાલી)
		/* nothing to embed; ignore */
		પાછા true;

	/* Don't include ourself */
	જો (params->classid != ખાલી && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		પાછા true;

	જો (params->data != ખાલી && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		પાછા true;

	/* codetype and type are MIME types */
	જો (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		પાછા false;
	જો (box_get_attribute(n, "type", params, &params->type) == false)
		પાછા false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	જો (params->classid != ખાલી && params->data == ખાલી &&
			params->codetype != ખાલી) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		જો (lerror != lwc_error_ok)
			પાછા false;

		જો (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			પાછા true;
		}

		lwc_string_unref(icodetype);
	}

	જો (params->data != ખાલી && params->type != ખાલી) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		જો (lerror != lwc_error_ok)
			પાછા false;

		જો (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			પાછા true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ગાંઠ_get_first_child(n, &c);
	જો (err != DOM_NO_ERR)
		પાછા false;

	જ્યારે (c != ખાલી) {
		dom_ગાંઠ *next;
		dom_ગાંઠ_type type;

		err = dom_ગાંઠ_get_ગાંઠ_type(c, &type);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(c);
			પાછા false;
		}

		જો (type == DOM_ELEMENT_ગાંઠ) {
			dom_string *name;

			err = dom_ગાંઠ_get_ગાંઠ_name(c, &name);
			જો (err != DOM_NO_ERR) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ગાંઠ_unref(c);
				તોડો;
			}
			dom_string_unref(name);

			param = talloc(params, સંરચના object_param);
			જો (param == ખાલી) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}
			param->name = ખાલી;
			param->કિંમત = ખાલી;
			param->type = ખાલી;
			param->valuetype = ખાલી;
			param->next = ખાલી;

			જો (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (box_get_attribute(c, "value", param,
					&param->કિંમત) == false) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ગાંઠ_unref(c);
				પાછા false;
			}

			જો (param->valuetype == ખાલી) {
				param->valuetype = talloc_strdup(param, "data");
				જો (param->valuetype == ખાલી) {
					dom_ગાંઠ_unref(c);
					પાછા false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_ગાંઠ_get_next_sibling(c, &next);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(c);
			પાછા false;
		}

		dom_ગાંઠ_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	જો (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		પાછા false;

	*convert_children = false;
	પાછા true;
}


/**
 * Preformatted text [9.3.4].
 */
સ્થિર bool
box_pre(dom_ગાંઠ *n,
	html_content *content,
	સંરચના box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	પાછા true;
}


/**
 * Option selector [17.6].
 */
સ્થિર bool
box_select(dom_ગાંઠ *n,
	   html_content *content,
	   સંરચના box *box,
	   bool *convert_children)
{
	સંરચના box *inline_container;
	સંરચના box *inline_box;
	સંરચના form_control *gadget;
	dom_ગાંઠ *c, *c2;
	dom_ગાંઠ *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ગાંઠ(content->forms, n);
	જો (gadget == ખાલી)
		પાછા false;

	gadget->html = content;
	err = dom_ગાંઠ_get_first_child(n, &c);
	જો (err != DOM_NO_ERR) {
		form_free_control(gadget);
		પાછા false;
	}

	જ્યારે (c != ખાલી) {
		dom_string *name;

		err = dom_ગાંઠ_get_ગાંઠ_name(c, &name);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(c);
			form_free_control(gadget);
			પાછા false;
		}

		જો (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			જો (box_select_add_option(gadget, c) == false) {
				dom_ગાંઠ_unref(c);
				form_free_control(gadget);
				પાછા false;
			}
		} નહીંતર જો (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ગાંઠ_get_first_child(c, &c2);
			જો (err != DOM_NO_ERR) {
				dom_ગાંઠ_unref(c);
				form_free_control(gadget);
				પાછા false;
			}

			જ્યારે (c2 != ખાલી) {
				dom_string *c2_name;

				err = dom_ગાંઠ_get_ગાંઠ_name(c2, &c2_name);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(c2);
					dom_ગાંઠ_unref(c);
					form_free_control(gadget);
					પાછા false;
				}

				જો (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					જો (box_select_add_option(gadget,
							c2) == false) {
						dom_ગાંઠ_unref(c2);
						dom_ગાંઠ_unref(c);
						form_free_control(gadget);
						પાછા false;
					}
				} નહીંતર {
					dom_string_unref(c2_name);
				}

				err = dom_ગાંઠ_get_next_sibling(c2, &next2);
				જો (err != DOM_NO_ERR) {
					dom_ગાંઠ_unref(c2);
					dom_ગાંઠ_unref(c);
					form_free_control(gadget);
					પાછા false;
				}

				dom_ગાંઠ_unref(c2);
				c2 = next2;
			}
		} નહીંતર {
			dom_string_unref(name);
		}

		err = dom_ગાંઠ_get_next_sibling(c, &next);
		જો (err != DOM_NO_ERR) {
			dom_ગાંઠ_unref(c);
			form_free_control(gadget);
			પાછા false;
		}

		dom_ગાંઠ_unref(c);
		c = next;
	}

	જો (gadget->data.select.અંક_વસ્તુઓ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		પાછા true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ખાલી, 0, false, 0, 0, 0, 0, content->bctx);
	જો (inline_container == ખાલી)
		જાઓ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ખાલી, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	જો (inline_box == ખાલી)
		જાઓ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	જો (gadget->data.select.multiple == false &&
			gadget->data.select.અંક_selected == 0) {
		gadget->data.select.current = gadget->data.select.વસ્તુઓ;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.અંક_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->ગાંઠ, true);
	}

	જો (gadget->data.select.અંક_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	નહીંતર જો (gadget->data.select.અંક_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	નહીંતર
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	જો (inline_box->text == ખાલી)
		જાઓ no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	પાછા true;

no_memory:
	પાછા false;
}


/**
 * Multi-line text field [17.7].
 */
સ્થિર bool box_textarea(dom_ગાંઠ *n,
			html_content *content,
			સંરચના box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ગાંઠ(content->forms, n);
	જો (box->gadget == ખાલી)
		પાછા false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	જો (!box_આગત_text(content, box, n))
		પાછા false;

	*convert_children = false;
	પાછા true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ગાંઠ *ગાંઠ,
			 html_content *content,
			 સંરચના box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(ગાંઠ, &tag_type);
	જો (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	પસંદગી (tag_type) {
	સ્થિતિ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_આગત:
		res = box_આગત(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ગાંઠ, content, box, convert_children);
		તોડો;

	સ્થિતિ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ગાંઠ, content, box, convert_children);
		તોડો;

	મૂળભૂત:
		res = true;
	}

	પાછા res;
}
