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

#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ "utils/nsoption.h"
#அந்தர்பாவ "utils/corestrings.h"
#அந்தர்பாவ "utils/log.h"
#அந்தர்பாவ "utils/messages.h"
#அந்தர்பாவ "utils/talloc.h"
#அந்தர்பாவ "utils/string.h"
#அந்தர்பாவ "utils/ascii.h"
#அந்தர்பாவ "utils/nsurl.h"
#அந்தர்பாவ "netsurf/plot_style.h"
#அந்தர்பாவ "css/hints.h"
#அந்தர்பாவ "desktop/frame_types.h"
#அந்தர்பாவ "content/content_factory.h"

#அந்தர்பாவ "html/html.h"
#அந்தர்பாவ "html/private.h"
#அந்தர்பாவ "html/object.h"
#அந்தர்பாவ "html/box.h"
#அந்தர்பாவ "html/box_manipulate.h"
#அந்தர்பாவ "html/box_construct.h"
#அந்தர்பாவ "html/box_special.h"
#அந்தர்பாவ "html/box_textarea.h"
#அந்தர்பாவ "html/form_internal.h"


நிலையான மாறிலி content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
நிலையான உள்வரி bool box_is_வேர்(dom_முனை *n)
{
	dom_முனை *parent;
	dom_முனை_type type;
	dom_exception err;

	err = dom_முனை_get_parent_முனை(n, &parent);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (parent != வெற்று) {
		err = dom_முனை_get_முனை_type(parent, &type);

		dom_முனை_unref(parent);

		எனில் (err != DOM_NO_ERR)
			திரும்பு false;

		எனில் (type != DOM_DOCUMENT_முனை)
			திரும்பு false;
	}

	திரும்பு true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
நிலையான முழு box_object_talloc_destructor(கட்டமைப்பு object_params *o)
{
	எனில் (o->codebase != வெற்று)
		nsurl_unref(o->codebase);
	எனில் (o->classid != வெற்று)
		nsurl_unref(o->classid);
	எனில் (o->data != வெற்று)
		nsurl_unref(o->data);

	திரும்பு 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
நிலையான கட்டமைப்பு frame_dimension *
box_parse_multi_lengths(மாறிலி dom_string *ds, unsigned முழு *மொத்தம்)
{
	எழுத்து *end;
	unsigned முழு i, n;
	கட்டமைப்பு frame_dimension *length;
	மாறிலி எழுத்து *s;

	s = dom_string_data(ds);

	க்கு (i = 0, n = 1; s[i]; i++)
		எனில் (s[i] == ',')
			n++;

	length = calloc(n, அளவு(கட்டமைப்பு frame_dimension));
	எனில் (!length)
		திரும்பு வெற்று;

	க்கு (i = 0; i != n; i++) {
		போது (ascii_is_space(*s)) {
			s++;
		}
		length[i].மதிப்பு = strtof(s, &end);
		எனில் (length[i].மதிப்பு <= 0) {
			length[i].மதிப்பு = 1;
		}
		s = end;
		தேர்வு (*s) {
			நிலை '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				நிறுத்து;
			நிலை '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				நிறுத்து;
			இயல்பு:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				நிறுத்து;
		}
		போது (*s && *s != ',') {
			s++;
		}
		எனில் (*s == ',') {
			s++;
		}
	}

	*மொத்தம் = n;
	திரும்பு length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
நிலையான முழு box_frames_talloc_destructor(கட்டமைப்பு content_html_frames *f)
{
	எனில் (f->url != வெற்று) {
		nsurl_unref(f->url);
		f->url = வெற்று;
	}

	திரும்பு 0;
}


/**
 * create a frameset box tree
 */
நிலையான bool
box_create_frameset(கட்டமைப்பு content_html_frames *f,
		    dom_முனை *n,
		    html_content *content)
{
	unsigned முழு row, col, index, i;
	unsigned முழு rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	கட்டமைப்பு frame_dimension *row_height = 0, *col_width = 0;
	dom_முனை *c, *next;
	கட்டமைப்பு content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		எனில் (row_height == வெற்று)
			திரும்பு false;
	} இல்லையெனில் {
		row_height = calloc(1, அளவு(கட்டமைப்பு frame_dimension));
		எனில் (row_height == வெற்று)
			திரும்பு false;
		row_height->மதிப்பு = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		எனில் (col_width == வெற்று) {
			விடுவி(row_height);
			திரும்பு false;
		}
	} இல்லையெனில் {
		col_width = calloc(1, அளவு(கட்டமைப்பு frame_dimension));
		எனில் (col_width == வெற்று) {
			விடுவி(row_height);
			திரும்பு false;
		}
		col_width->மதிப்பு = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		எனில் ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		css_color color;

		எனில் (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, கட்டமைப்பு content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	க்கு (row = 0; row < rows; row++) {
		க்கு (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = வெற்று;
			frame->url = வெற்று;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = வெற்று;
		}
	}
	விடுவி(col_width);
	விடுவி(row_height);

	/* create the frameset windows */
	err = dom_முனை_get_first_child(n, &c);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	க்கு (row = 0; c != வெற்று && row < rows; row++) {
		க்கு (col = 0; c != வெற்று && col < cols; col++) {
			போது (c != வெற்று) {
				dom_முனை_type type;
				dom_string *name;

				err = dom_முனை_get_முனை_type(c, &type);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(c);
					திரும்பு false;
				}

				err = dom_முனை_get_முனை_name(c, &name);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(c);
					திரும்பு false;
				}

				எனில் (type != DOM_ELEMENT_முனை ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_முனை_get_next_sibling(c,
							&next);
					எனில் (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_முனை_unref(c);
						திரும்பு false;
					}

					dom_string_unref(name);
					dom_முனை_unref(c);
					c = next;
				} இல்லையெனில் {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					நிறுத்து;
				}
			}

			எனில் (c == வெற்று)
				நிறுத்து;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_முனை_get_முனை_name(c, &s);
			எனில் (err != DOM_NO_ERR) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				எனில் (box_create_frameset(frame, c,
						content) == false) {
					dom_முனை_unref(c);
					திரும்பு false;
				}

				err = dom_முனை_get_next_sibling(c, &next);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(c);
					திரும்பு false;
				}

				dom_முனை_unref(c);
				c = next;
				தொடர்;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = வெற்று;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			எனில் (url != வெற்று) {
				/* no self-references */
				எனில் (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = வெற்று;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			எனில் (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				எனில் (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				css_color color;

				எனில் (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_முனை_get_next_sibling(c, &next);
			எனில் (err != DOM_NO_ERR) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			dom_முனை_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	எனில் (c != வெற்று) {
		dom_முனை_unref(c);
	}

	திரும்பு true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
நிலையான முழு box_iframes_talloc_destructor(கட்டமைப்பு content_html_iframe *f)
{
	எனில் (f->url != வெற்று) {
		nsurl_unref(f->url);
		f->url = வெற்று;
	}

	திரும்பு 0;
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
நிலையான bool
box_get_attribute(dom_முனை *n,
		  மாறிலி எழுத்து *attribute,
		  வெறுமை *context,
		  எழுத்து **மதிப்பு)
{
	எழுத்து *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((மாறிலி uint8_t *) attribute,
			strlen(attribute), &attr_name);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	எனில் (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		திரும்பு false;
	}

	dom_string_unref(attr_name);

	எனில் (attr != வெற்று) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		எனில் (result == வெற்று)
			திரும்பு false;

		*மதிப்பு = result;
	}

	திரும்பு true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
நிலையான bool
box_உள்ளீடு_text(html_content *html, கட்டமைப்பு box *box, கட்டமைப்பு dom_முனை *முனை)
{
	கட்டமைப்பு box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	தேர்வு (display) {
	நிலை CSS_DISPLAY_GRID:
	நிலை CSS_DISPLAY_FLEX:
	நிலை CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		நிறுத்து;
	இயல்பு:
		box->type = BOX_INLINE_BLOCK;
		நிறுத்து;
	}

	inline_container = box_create(வெற்று, 0, false, 0, 0, 0, 0, html->bctx);
	எனில் (!inline_container)
		திரும்பு false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(வெற்று, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	எனில் (!inline_box)
		திரும்பு false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	திரும்பு box_textarea_create_textarea(html, box, முனை);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
நிலையான bool box_select_add_option(கட்டமைப்பு form_control *control, dom_முனை *n)
{
	எழுத்து *மதிப்பு = வெற்று;
	எழுத்து *text = வெற்று;
	எழுத்து *text_nowrap = வெற்று;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_முனை_get_text_content(n, &content);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	எனில் (content != வெற்று) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} இல்லையெனில் {
		text = strdup("");
	}

	எனில் (text == வெற்று)
		செல் no_memory;

	err = dom_element_get_attribute(n, corestring_dom_மதிப்பு, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		மதிப்பு = strdup(dom_string_data(s));
		dom_string_unref(s);
	} இல்லையெனில் {
		மதிப்பு = strdup(text);
	}

	எனில் (மதிப்பு == வெற்று)
		செல் no_memory;

	எனில் (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	எனில் (text_nowrap == வெற்று)
		செல் no_memory;

	எனில் (form_add_option(control, மதிப்பு, text_nowrap, selected, n) == false)
		செல் no_memory;

	விடுவி(text);

	திரும்பு true;

no_memory:
	விடுவி(மதிப்பு);
	விடுவி(text);
	விடுவி(text_nowrap);
	திரும்பு false;
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
நிலையான bool
box_a(dom_முனை *n,
      html_content *content,
      கட்டமைப்பு box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		எனில் (!ok)
			திரும்பு false;
		எனில் (url) {
			எனில் (box->href != வெற்று)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		எனில் (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_உச்சி_2))
			box->target = "_top";
		இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		இல்லையெனில் {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			எனில் (!box->target) {
				dom_string_unref(s);
				திரும்பு false;
			}
		}
		dom_string_unref(s);
	}

	திரும்பு true;
}


/**
 * Document body special element handler [7.5.1].
 */
நிலையான bool
box_body(dom_முனை *n,
	 html_content *content,
	 கட்டமைப்பு box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	எனில் (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} இல்லையெனில் {
		content->background_colour = nscss_color_to_ns(color);
	}

	திரும்பு true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
நிலையான bool
box_br(dom_முனை *n,
       html_content *content,
       கட்டமைப்பு box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	திரும்பு true;
}


/**
 * special element handler for Push button [17.5].
 */
நிலையான bool
box_button(dom_முனை *n,
	   html_content *content,
	   கட்டமைப்பு box *box,
	   bool *convert_children)
{
	கட்டமைப்பு form_control *gadget;

	gadget = html_forms_get_control_for_முனை(content->forms, n);
	எனில் (!gadget)
		திரும்பு false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	திரும்பு true;
}


/**
 * Canvas element
 */
நிலையான bool
box_canvas(dom_முனை *n,
	     html_content *content,
	     கட்டமைப்பு box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	எனில் (!content->enable_scripting) {
		திரும்பு true;
	}
	*convert_children = false;

	எனில் (box->style && ns_computed_display(box->style,
			box_is_வேர்(n)) == CSS_DISPLAY_NONE)
		திரும்பு true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	திரும்பு true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
நிலையான bool
box_embed(dom_முனை *n,
	  html_content *content,
	  கட்டமைப்பு box *box,
	  bool *convert_children)
{
	கட்டமைப்பு object_params *params;
	கட்டமைப்பு object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t இலக்கம்_attrs;
	dom_string *src;
	dom_exception err;

	எனில் (box->style && ns_computed_display(box->style,
			box_is_வேர்(n)) == CSS_DISPLAY_NONE)
		திரும்பு true;

	params = talloc(content->bctx, கட்டமைப்பு object_params);
	எனில் (params == வெற்று)
		திரும்பு false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = வெற்று;
	params->type = வெற்று;
	params->codetype = வெற்று;
	params->codebase = வெற்று;
	params->classid = வெற்று;
	params->params = வெற்று;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	எனில் (err != DOM_NO_ERR || src == வெற்று)
		திரும்பு true;
	எனில் (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		திரும்பு false;
	}

	dom_string_unref(src);

	எனில் (params->data == வெற்று)
		திரும்பு true;

	/* Don't include ourself */
	எனில் (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		திரும்பு true;

	/* add attributes as parameters to linked list */
	err = dom_முனை_get_attributes(n, &attrs);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	err = dom_namednodemap_get_length(attrs, &இலக்கம்_attrs);
	எனில் (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		திரும்பு false;
	}

	க்கு (idx = 0; idx < இலக்கம்_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *மதிப்பு;

		err = dom_namednodemap_item(attrs, idx, (வெறுமை *) &attr);
		எனில் (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			திரும்பு false;
		}

		err = dom_attr_get_name(attr, &name);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(attr);
			dom_namednodemap_unref(attrs);
			திரும்பு false;
		}

		எனில் (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_முனை_unref(attr);
			dom_string_unref(name);
			தொடர்;
		}

		err = dom_attr_get_மதிப்பு(attr, &மதிப்பு);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			திரும்பு false;
		}

		param = talloc(content->bctx, கட்டமைப்பு object_param);
		எனில் (param == வெற்று) {
			dom_முனை_unref(attr);
			dom_string_unref(மதிப்பு);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			திரும்பு false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->மதிப்பு = talloc_strdup(content->bctx, dom_string_data(மதிப்பு));
		param->type = வெற்று;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = வெற்று;

		dom_string_unref(மதிப்பு);
		dom_string_unref(name);
		dom_முனை_unref(attr);

		எனில் (param->name == வெற்று || param->மதிப்பு == வெற்று ||
				param->valuetype == வெற்று) {
			dom_namednodemap_unref(attrs);
			திரும்பு false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	திரும்பு html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
நிலையான bool
box_frameset(dom_முனை *n,
	     html_content *content,
	     கட்டமைப்பு box *box,
	     bool *convert_children)
{
	bool ok;

	எனில் (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		எனில் (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		திரும்பு true;
	}

	content->frameset = talloc_zero(content->bctx,
					கட்டமைப்பு content_html_frames);
	எனில் (!content->frameset) {
		திரும்பு false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	எனில் (ok) {
		box->type = BOX_NONE;
	}

	எனில் (convert_children) {
		*convert_children = false;
	}
	திரும்பு ok;
}


/**
 * Inline subwindow [16.5].
 */
நிலையான bool
box_iframe(dom_முனை *n,
	   html_content *content,
	   கட்டமைப்பு box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	கட்டமைப்பு content_html_iframe *iframe;
	முழு i;

	எனில் (box->style && ns_computed_display(box->style,
			box_is_வேர்(n)) == CSS_DISPLAY_NONE)
		திரும்பு true;

	எனில் (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		திரும்பு true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	எனில் (err != DOM_NO_ERR || s == வெற்று)
		திரும்பு true;
	எனில் (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		திரும்பு false;
	}
	dom_string_unref(s);
	எனில் (url == வெற்று)
		திரும்பு true;

	/* don't include ourself */
	எனில் (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		திரும்பு true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, கட்டமைப்பு content_html_iframe);
	எனில் (iframe == வெற்று) {
		nsurl_unref(url);
		திரும்பு false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = வெற்று;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		css_color color;

		எனில் (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	ப்ரதிஜ்ஞா(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	எனில் (convert_children)
		*convert_children = false;
	திரும்பு true;
}


/**
 * Embedded image [13.2].
 */
நிலையான bool
box_image(dom_முனை *n,
	  html_content *content,
	  கட்டமைப்பு box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	எண்ணிக்கை css_width_e wtype;
	எண்ணிக்கை css_height_e htype;
	css_fixed மதிப்பு = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	எனில் (box->style && ns_computed_display(box->style,
			box_is_வேர்(n)) == CSS_DISPLAY_NONE)
		திரும்பு true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	எனில் (err == DOM_NO_ERR && s != வெற்று) {
		எழுத்து *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		எனில் (alt == வெற்று)
			திரும்பு false;
		box->text = talloc_strdup(content->bctx, alt);
		விடுவி(alt);
		எனில் (box->text == வெற்று)
			திரும்பு false;
		box->length = strlen(box->text);
	}

	எனில் (nsoption_bool(foreground_images) == false) {
		திரும்பு true;
	}

	/* imagemap associated with this image */
	எனில் (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		திரும்பு false;
	எனில் (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	எனில் (err != DOM_NO_ERR || s == வெற்று)
		திரும்பு true;

	எனில் (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		திரும்பு false;
	}

	dom_string_unref(s);

	எனில் (url == வெற்று)
		திரும்பு true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &மதிப்பு, &wunit);
	htype = css_computed_height(box->style, &மதிப்பு, &hunit);

	எனில் (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	திரும்பு ok;
}


/**
 * Form control [17.4].
 */
நிலையான bool
box_உள்ளீடு(dom_முனை *n,
	  html_content *content,
	  கட்டமைப்பு box *box,
	  bool *convert_children)
{
	கட்டமைப்பு form_control *gadget;
	dom_string *type = வெற்று;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_முனை(content->forms, n);
	எனில் (gadget == வெற்று) {
		திரும்பு false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	எனில் ((err != DOM_NO_ERR) || (type == வெற்று)) {
		/* no type so "text" is assumed */
		எனில் (box_உள்ளீடு_text(content, box, n) == false) {
			திரும்பு false;
		}
		*convert_children = false;
		திரும்பு true;
	}

	எனில் (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		எனில் (box_உள்ளீடு_text(content, box, n) == false)
			செல் no_memory;

	} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} இல்லையெனில் எனில் ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		கட்டமைப்பு box *inline_container, *inline_box;

		எனில் (box_button(n, content, box, 0) == false)
			செல் no_memory;

		inline_container = box_create(வெற்று, 0, false, 0, 0, 0, 0,
				content->bctx);
		எனில் (inline_container == வெற்று)
			செல் no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(வெற்று, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		எனில் (inline_box == வெற்று)
			செல் no_memory;

		inline_box->type = BOX_TEXT;

		எனில் (box->gadget->மதிப்பு != வெற்று)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->மதிப்பு);
		இல்லையெனில் எனில் (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		இல்லையெனில் எனில் (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		இல்லையெனில்
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		எனில் (inline_box->text == வெற்று)
			செல் no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		எனில் (box->style && ns_computed_display(box->style,
				box_is_வேர்(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			எனில் (err == DOM_NO_ERR && s != வெற்று) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				எனில் (error != NSERROR_OK)
					செல் no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				எனில் (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					எனில் (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						செல் no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} இல்லையெனில் {
		/* unhandled type the default is "text" */
		எனில் (box_உள்ளீடு_text(content, box, n) == false)
			செல் no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	திரும்பு true;

no_memory:
	dom_string_unref(type);

	திரும்பு false;
}


/**
 * Noscript element
 */
நிலையான bool
box_noscript(dom_முனை *n,
	     html_content *content,
	     கட்டமைப்பு box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	எனில் (content->enable_scripting) {
		*convert_children = false;
	}

	திரும்பு true;
}


/**
 * Generic embedded object [13.3].
 */
நிலையான bool
box_object(dom_முனை *n,
	   html_content *content,
	   கட்டமைப்பு box *box,
	   bool *convert_children)
{
	கட்டமைப்பு object_params *params;
	கட்டமைப்பு object_param *param;
	dom_string *codebase, *classid, *data;
	dom_முனை *c;
	dom_exception err;

	எனில் (box->style && ns_computed_display(box->style,
			box_is_வேர்(n)) == CSS_DISPLAY_NONE)
		திரும்பு true;

	எனில் (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		திரும்பு false;
	எனில் (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, கட்டமைப்பு object_params);
	எனில் (params == வெற்று)
		திரும்பு false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = வெற்று;
	params->type = வெற்று;
	params->codetype = வெற்று;
	params->codebase = வெற்று;
	params->classid = வெற்று;
	params->params = வெற்று;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	எனில் (err == DOM_NO_ERR && codebase != வெற்று) {
		எனில் (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			திரும்பு false;
		}
		dom_string_unref(codebase);
	}
	எனில் (params->codebase == வெற்று)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	எனில் (err == DOM_NO_ERR && classid != வெற்று) {
		எனில் (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			திரும்பு false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	எனில் (err == DOM_NO_ERR && data != வெற்று) {
		எனில் (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			திரும்பு false;
		}
		dom_string_unref(data);
	}

	எனில் (params->classid == வெற்று && params->data == வெற்று)
		/* nothing to embed; ignore */
		திரும்பு true;

	/* Don't include ourself */
	எனில் (params->classid != வெற்று && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		திரும்பு true;

	எனில் (params->data != வெற்று && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		திரும்பு true;

	/* codetype and type are MIME types */
	எனில் (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		திரும்பு false;
	எனில் (box_get_attribute(n, "type", params, &params->type) == false)
		திரும்பு false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	எனில் (params->classid != வெற்று && params->data == வெற்று &&
			params->codetype != வெற்று) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		எனில் (lerror != lwc_error_ok)
			திரும்பு false;

		எனில் (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			திரும்பு true;
		}

		lwc_string_unref(icodetype);
	}

	எனில் (params->data != வெற்று && params->type != வெற்று) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		எனில் (lerror != lwc_error_ok)
			திரும்பு false;

		எனில் (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			திரும்பு true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_முனை_get_first_child(n, &c);
	எனில் (err != DOM_NO_ERR)
		திரும்பு false;

	போது (c != வெற்று) {
		dom_முனை *next;
		dom_முனை_type type;

		err = dom_முனை_get_முனை_type(c, &type);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(c);
			திரும்பு false;
		}

		எனில் (type == DOM_ELEMENT_முனை) {
			dom_string *name;

			err = dom_முனை_get_முனை_name(c, &name);
			எனில் (err != DOM_NO_ERR) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_முனை_unref(c);
				நிறுத்து;
			}
			dom_string_unref(name);

			param = talloc(params, கட்டமைப்பு object_param);
			எனில் (param == வெற்று) {
				dom_முனை_unref(c);
				திரும்பு false;
			}
			param->name = வெற்று;
			param->மதிப்பு = வெற்று;
			param->type = வெற்று;
			param->valuetype = வெற்று;
			param->next = வெற்று;

			எனில் (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (box_get_attribute(c, "value", param,
					&param->மதிப்பு) == false) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_முனை_unref(c);
				திரும்பு false;
			}

			எனில் (param->valuetype == வெற்று) {
				param->valuetype = talloc_strdup(param, "data");
				எனில் (param->valuetype == வெற்று) {
					dom_முனை_unref(c);
					திரும்பு false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_முனை_get_next_sibling(c, &next);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(c);
			திரும்பு false;
		}

		dom_முனை_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	எனில் (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		திரும்பு false;

	*convert_children = false;
	திரும்பு true;
}


/**
 * Preformatted text [9.3.4].
 */
நிலையான bool
box_pre(dom_முனை *n,
	html_content *content,
	கட்டமைப்பு box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	திரும்பு true;
}


/**
 * Option selector [17.6].
 */
நிலையான bool
box_select(dom_முனை *n,
	   html_content *content,
	   கட்டமைப்பு box *box,
	   bool *convert_children)
{
	கட்டமைப்பு box *inline_container;
	கட்டமைப்பு box *inline_box;
	கட்டமைப்பு form_control *gadget;
	dom_முனை *c, *c2;
	dom_முனை *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_முனை(content->forms, n);
	எனில் (gadget == வெற்று)
		திரும்பு false;

	gadget->html = content;
	err = dom_முனை_get_first_child(n, &c);
	எனில் (err != DOM_NO_ERR) {
		form_free_control(gadget);
		திரும்பு false;
	}

	போது (c != வெற்று) {
		dom_string *name;

		err = dom_முனை_get_முனை_name(c, &name);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(c);
			form_free_control(gadget);
			திரும்பு false;
		}

		எனில் (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			எனில் (box_select_add_option(gadget, c) == false) {
				dom_முனை_unref(c);
				form_free_control(gadget);
				திரும்பு false;
			}
		} இல்லையெனில் எனில் (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_முனை_get_first_child(c, &c2);
			எனில் (err != DOM_NO_ERR) {
				dom_முனை_unref(c);
				form_free_control(gadget);
				திரும்பு false;
			}

			போது (c2 != வெற்று) {
				dom_string *c2_name;

				err = dom_முனை_get_முனை_name(c2, &c2_name);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(c2);
					dom_முனை_unref(c);
					form_free_control(gadget);
					திரும்பு false;
				}

				எனில் (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					எனில் (box_select_add_option(gadget,
							c2) == false) {
						dom_முனை_unref(c2);
						dom_முனை_unref(c);
						form_free_control(gadget);
						திரும்பு false;
					}
				} இல்லையெனில் {
					dom_string_unref(c2_name);
				}

				err = dom_முனை_get_next_sibling(c2, &next2);
				எனில் (err != DOM_NO_ERR) {
					dom_முனை_unref(c2);
					dom_முனை_unref(c);
					form_free_control(gadget);
					திரும்பு false;
				}

				dom_முனை_unref(c2);
				c2 = next2;
			}
		} இல்லையெனில் {
			dom_string_unref(name);
		}

		err = dom_முனை_get_next_sibling(c, &next);
		எனில் (err != DOM_NO_ERR) {
			dom_முனை_unref(c);
			form_free_control(gadget);
			திரும்பு false;
		}

		dom_முனை_unref(c);
		c = next;
	}

	எனில் (gadget->data.select.இலக்கம்_பொருட்கள் == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		திரும்பு true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(வெற்று, 0, false, 0, 0, 0, 0, content->bctx);
	எனில் (inline_container == வெற்று)
		செல் no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(வெற்று, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	எனில் (inline_box == வெற்று)
		செல் no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	எனில் (gadget->data.select.multiple == false &&
			gadget->data.select.இலக்கம்_selected == 0) {
		gadget->data.select.current = gadget->data.select.பொருட்கள்;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.இலக்கம்_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->முனை, true);
	}

	எனில் (gadget->data.select.இலக்கம்_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	இல்லையெனில் எனில் (gadget->data.select.இலக்கம்_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	இல்லையெனில்
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	எனில் (inline_box->text == வெற்று)
		செல் no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	திரும்பு true;

no_memory:
	திரும்பு false;
}


/**
 * Multi-line text field [17.7].
 */
நிலையான bool box_textarea(dom_முனை *n,
			html_content *content,
			கட்டமைப்பு box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_முனை(content->forms, n);
	எனில் (box->gadget == வெற்று)
		திரும்பு false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	எனில் (!box_உள்ளீடு_text(content, box, n))
		திரும்பு false;

	*convert_children = false;
	திரும்பு true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_முனை *முனை,
			 html_content *content,
			 கட்டமைப்பு box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(முனை, &tag_type);
	எனில் (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	தேர்வு (tag_type) {
	நிலை DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_உள்ளீடு:
		res = box_உள்ளீடு(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(முனை, content, box, convert_children);
		நிறுத்து;

	நிலை DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(முனை, content, box, convert_children);
		நிறுத்து;

	இயல்பு:
		res = true;
	}

	திரும்பு res;
}
