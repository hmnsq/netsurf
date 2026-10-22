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

#شامل <string.h>
#شامل <stdbool.h>
#شامل <dom/dom.h>

#شامل "utils/nsoption.h"
#شامل "utils/corestrings.h"
#شامل "utils/log.h"
#شامل "utils/messages.h"
#شامل "utils/talloc.h"
#شامل "utils/string.h"
#شامل "utils/ascii.h"
#شامل "utils/nsurl.h"
#شامل "netsurf/plot_style.h"
#شامل "css/hints.h"
#شامل "desktop/frame_types.h"
#شامل "content/content_factory.h"

#شامل "html/html.h"
#شامل "html/private.h"
#شامل "html/object.h"
#شامل "html/box.h"
#شامل "html/box_manipulate.h"
#شامل "html/box_construct.h"
#شامل "html/box_special.h"
#شامل "html/box_textarea.h"
#شامل "html/form_internal.h"


ایستا ثابت content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ایستا درونخطی bool box_is_ریشه(dom_گره *n)
{
	dom_گره *parent;
	dom_گره_type type;
	dom_exception err;

	err = dom_گره_get_parent_گره(n, &parent);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (parent != پوچ) {
		err = dom_گره_get_گره_type(parent, &type);

		dom_گره_unref(parent);

		اگر (err != DOM_NO_ERR)
			بازگشت false;

		اگر (type != DOM_DOCUMENT_گره)
			بازگشت false;
	}

	بازگشت true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ایستا صحیح box_object_talloc_destructor(ساختار object_params *o)
{
	اگر (o->codebase != پوچ)
		nsurl_unref(o->codebase);
	اگر (o->classid != پوچ)
		nsurl_unref(o->classid);
	اگر (o->data != پوچ)
		nsurl_unref(o->data);

	بازگشت 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ایستا ساختار frame_dimension *
box_parse_multi_lengths(ثابت dom_string *ds, unsigned صحیح *شمار)
{
	نویسه *end;
	unsigned صحیح i, n;
	ساختار frame_dimension *length;
	ثابت نویسه *s;

	s = dom_string_data(ds);

	برای (i = 0, n = 1; s[i]; i++)
		اگر (s[i] == ',')
			n++;

	length = calloc(n, اندازه(ساختار frame_dimension));
	اگر (!length)
		بازگشت پوچ;

	برای (i = 0; i != n; i++) {
		وقتی (ascii_is_space(*s)) {
			s++;
		}
		length[i].مقدار = strtof(s, &end);
		اگر (length[i].مقدار <= 0) {
			length[i].مقدار = 1;
		}
		s = end;
		انتخاب (*s) {
			حالت '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				بشکن;
			حالت '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				بشکن;
			پیشفرض:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				بشکن;
		}
		وقتی (*s && *s != ',') {
			s++;
		}
		اگر (*s == ',') {
			s++;
		}
	}

	*شمار = n;
	بازگشت length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ایستا صحیح box_frames_talloc_destructor(ساختار content_html_frames *f)
{
	اگر (f->url != پوچ) {
		nsurl_unref(f->url);
		f->url = پوچ;
	}

	بازگشت 0;
}


/**
 * create a frameset box tree
 */
ایستا bool
box_create_frameset(ساختار content_html_frames *f,
		    dom_گره *n,
		    html_content *content)
{
	unsigned صحیح row, col, index, i;
	unsigned صحیح rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ساختار frame_dimension *row_height = 0, *col_width = 0;
	dom_گره *c, *next;
	ساختار content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		اگر (row_height == پوچ)
			بازگشت false;
	} وگرنه {
		row_height = calloc(1, اندازه(ساختار frame_dimension));
		اگر (row_height == پوچ)
			بازگشت false;
		row_height->مقدار = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		اگر (col_width == پوچ) {
			آزاد(row_height);
			بازگشت false;
		}
	} وگرنه {
		col_width = calloc(1, اندازه(ساختار frame_dimension));
		اگر (col_width == پوچ) {
			آزاد(row_height);
			بازگشت false;
		}
		col_width->مقدار = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		اگر ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		css_color color;

		اگر (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ساختار content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	برای (row = 0; row < rows; row++) {
		برای (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = پوچ;
			frame->url = پوچ;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = پوچ;
		}
	}
	آزاد(col_width);
	آزاد(row_height);

	/* create the frameset windows */
	err = dom_گره_get_first_child(n, &c);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	برای (row = 0; c != پوچ && row < rows; row++) {
		برای (col = 0; c != پوچ && col < cols; col++) {
			وقتی (c != پوچ) {
				dom_گره_type type;
				dom_string *name;

				err = dom_گره_get_گره_type(c, &type);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					بازگشت false;
				}

				err = dom_گره_get_گره_name(c, &name);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					بازگشت false;
				}

				اگر (type != DOM_ELEMENT_گره ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_گره_get_next_sibling(c,
							&next);
					اگر (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_گره_unref(c);
						بازگشت false;
					}

					dom_string_unref(name);
					dom_گره_unref(c);
					c = next;
				} وگرنه {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					بشکن;
				}
			}

			اگر (c == پوچ)
				بشکن;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_گره_get_گره_name(c, &s);
			اگر (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				اگر (box_create_frameset(frame, c,
						content) == false) {
					dom_گره_unref(c);
					بازگشت false;
				}

				err = dom_گره_get_next_sibling(c, &next);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					بازگشت false;
				}

				dom_گره_unref(c);
				c = next;
				ادامه;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = پوچ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			اگر (url != پوچ) {
				/* no self-references */
				اگر (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = پوچ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			اگر (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				اگر (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				وگرنه اگر (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				css_color color;

				اگر (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_گره_get_next_sibling(c, &next);
			اگر (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				بازگشت false;
			}

			dom_گره_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	اگر (c != پوچ) {
		dom_گره_unref(c);
	}

	بازگشت true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ایستا صحیح box_iframes_talloc_destructor(ساختار content_html_iframe *f)
{
	اگر (f->url != پوچ) {
		nsurl_unref(f->url);
		f->url = پوچ;
	}

	بازگشت 0;
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
ایستا bool
box_get_attribute(dom_گره *n,
		  ثابت نویسه *attribute,
		  تهی *context,
		  نویسه **مقدار)
{
	نویسه *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ثابت uint8_t *) attribute,
			strlen(attribute), &attr_name);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		بازگشت false;
	}

	dom_string_unref(attr_name);

	اگر (attr != پوچ) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		اگر (result == پوچ)
			بازگشت false;

		*مقدار = result;
	}

	بازگشت true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ایستا bool
box_درون_داد_text(html_content *html, ساختار box *box, ساختار dom_گره *گره)
{
	ساختار box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	انتخاب (display) {
	حالت CSS_DISPLAY_GRID:
	حالت CSS_DISPLAY_FLEX:
	حالت CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		بشکن;
	پیشفرض:
		box->type = BOX_INLINE_BLOCK;
		بشکن;
	}

	inline_container = box_create(پوچ, 0, false, 0, 0, 0, 0, html->bctx);
	اگر (!inline_container)
		بازگشت false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(پوچ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	اگر (!inline_box)
		بازگشت false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	بازگشت box_textarea_create_textarea(html, box, گره);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ایستا bool box_select_add_option(ساختار form_control *control, dom_گره *n)
{
	نویسه *مقدار = پوچ;
	نویسه *text = پوچ;
	نویسه *text_nowrap = پوچ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_گره_get_text_content(n, &content);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	اگر (content != پوچ) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} وگرنه {
		text = strdup("");
	}

	اگر (text == پوچ)
		برو no_memory;

	err = dom_element_get_attribute(n, corestring_dom_مقدار, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		مقدار = strdup(dom_string_data(s));
		dom_string_unref(s);
	} وگرنه {
		مقدار = strdup(text);
	}

	اگر (مقدار == پوچ)
		برو no_memory;

	اگر (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	اگر (text_nowrap == پوچ)
		برو no_memory;

	اگر (form_add_option(control, مقدار, text_nowrap, selected, n) == false)
		برو no_memory;

	آزاد(text);

	بازگشت true;

no_memory:
	آزاد(مقدار);
	آزاد(text);
	آزاد(text_nowrap);
	بازگشت false;
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
ایستا bool
box_a(dom_گره *n,
      html_content *content,
      ساختار box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		اگر (!ok)
			بازگشت false;
		اگر (url) {
			اگر (box->href != پوچ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		اگر (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		وگرنه اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_بالاترین_2))
			box->target = "_top";
		وگرنه اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		وگرنه اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		وگرنه {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			اگر (!box->target) {
				dom_string_unref(s);
				بازگشت false;
			}
		}
		dom_string_unref(s);
	}

	بازگشت true;
}


/**
 * Document body special element handler [7.5.1].
 */
ایستا bool
box_body(dom_گره *n,
	 html_content *content,
	 ساختار box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	اگر (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} وگرنه {
		content->background_colour = nscss_color_to_ns(color);
	}

	بازگشت true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ایستا bool
box_br(dom_گره *n,
       html_content *content,
       ساختار box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	بازگشت true;
}


/**
 * special element handler for Push button [17.5].
 */
ایستا bool
box_button(dom_گره *n,
	   html_content *content,
	   ساختار box *box,
	   bool *convert_children)
{
	ساختار form_control *gadget;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	اگر (!gadget)
		بازگشت false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	بازگشت true;
}


/**
 * Canvas element
 */
ایستا bool
box_canvas(dom_گره *n,
	     html_content *content,
	     ساختار box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	اگر (!content->enable_scripting) {
		بازگشت true;
	}
	*convert_children = false;

	اگر (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		بازگشت true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	بازگشت true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ایستا bool
box_embed(dom_گره *n,
	  html_content *content,
	  ساختار box *box,
	  bool *convert_children)
{
	ساختار object_params *params;
	ساختار object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t رقم_attrs;
	dom_string *src;
	dom_exception err;

	اگر (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		بازگشت true;

	params = talloc(content->bctx, ساختار object_params);
	اگر (params == پوچ)
		بازگشت false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = پوچ;
	params->type = پوچ;
	params->codetype = پوچ;
	params->codebase = پوچ;
	params->classid = پوچ;
	params->params = پوچ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	اگر (err != DOM_NO_ERR || src == پوچ)
		بازگشت true;
	اگر (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		بازگشت false;
	}

	dom_string_unref(src);

	اگر (params->data == پوچ)
		بازگشت true;

	/* Don't include ourself */
	اگر (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		بازگشت true;

	/* add attributes as parameters to linked list */
	err = dom_گره_get_attributes(n, &attrs);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	err = dom_namednodemap_get_length(attrs, &رقم_attrs);
	اگر (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		بازگشت false;
	}

	برای (idx = 0; idx < رقم_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *مقدار;

		err = dom_namednodemap_item(attrs, idx, (تهی *) &attr);
		اگر (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			بازگشت false;
		}

		err = dom_attr_get_name(attr, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(attr);
			dom_namednodemap_unref(attrs);
			بازگشت false;
		}

		اگر (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_گره_unref(attr);
			dom_string_unref(name);
			ادامه;
		}

		err = dom_attr_get_مقدار(attr, &مقدار);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			بازگشت false;
		}

		param = talloc(content->bctx, ساختار object_param);
		اگر (param == پوچ) {
			dom_گره_unref(attr);
			dom_string_unref(مقدار);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			بازگشت false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->مقدار = talloc_strdup(content->bctx, dom_string_data(مقدار));
		param->type = پوچ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = پوچ;

		dom_string_unref(مقدار);
		dom_string_unref(name);
		dom_گره_unref(attr);

		اگر (param->name == پوچ || param->مقدار == پوچ ||
				param->valuetype == پوچ) {
			dom_namednodemap_unref(attrs);
			بازگشت false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	بازگشت html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ایستا bool
box_frameset(dom_گره *n,
	     html_content *content,
	     ساختار box *box,
	     bool *convert_children)
{
	bool ok;

	اگر (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		اگر (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		بازگشت true;
	}

	content->frameset = talloc_zero(content->bctx,
					ساختار content_html_frames);
	اگر (!content->frameset) {
		بازگشت false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	اگر (ok) {
		box->type = BOX_NONE;
	}

	اگر (convert_children) {
		*convert_children = false;
	}
	بازگشت ok;
}


/**
 * Inline subwindow [16.5].
 */
ایستا bool
box_iframe(dom_گره *n,
	   html_content *content,
	   ساختار box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ساختار content_html_iframe *iframe;
	صحیح i;

	اگر (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		بازگشت true;

	اگر (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		بازگشت true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	اگر (err != DOM_NO_ERR || s == پوچ)
		بازگشت true;
	اگر (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		بازگشت false;
	}
	dom_string_unref(s);
	اگر (url == پوچ)
		بازگشت true;

	/* don't include ourself */
	اگر (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		بازگشت true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ساختار content_html_iframe);
	اگر (iframe == پوچ) {
		nsurl_unref(url);
		بازگشت false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = پوچ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		css_color color;

		اگر (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		وگرنه اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	أكد(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	اگر (convert_children)
		*convert_children = false;
	بازگشت true;
}


/**
 * Embedded image [13.2].
 */
ایستا bool
box_image(dom_گره *n,
	  html_content *content,
	  ساختار box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	شمارش css_width_e wtype;
	شمارش css_height_e htype;
	css_fixed مقدار = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	اگر (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		بازگشت true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	اگر (err == DOM_NO_ERR && s != پوچ) {
		نویسه *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		اگر (alt == پوچ)
			بازگشت false;
		box->text = talloc_strdup(content->bctx, alt);
		آزاد(alt);
		اگر (box->text == پوچ)
			بازگشت false;
		box->length = strlen(box->text);
	}

	اگر (nsoption_bool(foreground_images) == false) {
		بازگشت true;
	}

	/* imagemap associated with this image */
	اگر (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		بازگشت false;
	اگر (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	اگر (err != DOM_NO_ERR || s == پوچ)
		بازگشت true;

	اگر (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		بازگشت false;
	}

	dom_string_unref(s);

	اگر (url == پوچ)
		بازگشت true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &مقدار, &wunit);
	htype = css_computed_height(box->style, &مقدار, &hunit);

	اگر (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	بازگشت ok;
}


/**
 * Form control [17.4].
 */
ایستا bool
box_درون_داد(dom_گره *n,
	  html_content *content,
	  ساختار box *box,
	  bool *convert_children)
{
	ساختار form_control *gadget;
	dom_string *type = پوچ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	اگر (gadget == پوچ) {
		بازگشت false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	اگر ((err != DOM_NO_ERR) || (type == پوچ)) {
		/* no type so "text" is assumed */
		اگر (box_درون_داد_text(content, box, n) == false) {
			بازگشت false;
		}
		*convert_children = false;
		بازگشت true;
	}

	اگر (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		اگر (box_درون_داد_text(content, box, n) == false)
			برو no_memory;

	} وگرنه اگر (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} وگرنه اگر (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} وگرنه اگر ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} وگرنه اگر (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ساختار box *inline_container, *inline_box;

		اگر (box_button(n, content, box, 0) == false)
			برو no_memory;

		inline_container = box_create(پوچ, 0, false, 0, 0, 0, 0,
				content->bctx);
		اگر (inline_container == پوچ)
			برو no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(پوچ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		اگر (inline_box == پوچ)
			برو no_memory;

		inline_box->type = BOX_TEXT;

		اگر (box->gadget->مقدار != پوچ)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->مقدار);
		وگرنه اگر (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		وگرنه اگر (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		وگرنه
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		اگر (inline_box->text == پوچ)
			برو no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} وگرنه اگر (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		اگر (box->style && ns_computed_display(box->style,
				box_is_ریشه(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			اگر (err == DOM_NO_ERR && s != پوچ) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				اگر (error != NSERROR_OK)
					برو no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				اگر (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					اگر (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						برو no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} وگرنه {
		/* unhandled type the default is "text" */
		اگر (box_درون_داد_text(content, box, n) == false)
			برو no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	بازگشت true;

no_memory:
	dom_string_unref(type);

	بازگشت false;
}


/**
 * Noscript element
 */
ایستا bool
box_noscript(dom_گره *n,
	     html_content *content,
	     ساختار box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	اگر (content->enable_scripting) {
		*convert_children = false;
	}

	بازگشت true;
}


/**
 * Generic embedded object [13.3].
 */
ایستا bool
box_object(dom_گره *n,
	   html_content *content,
	   ساختار box *box,
	   bool *convert_children)
{
	ساختار object_params *params;
	ساختار object_param *param;
	dom_string *codebase, *classid, *data;
	dom_گره *c;
	dom_exception err;

	اگر (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		بازگشت true;

	اگر (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		بازگشت false;
	اگر (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ساختار object_params);
	اگر (params == پوچ)
		بازگشت false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = پوچ;
	params->type = پوچ;
	params->codetype = پوچ;
	params->codebase = پوچ;
	params->classid = پوچ;
	params->params = پوچ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	اگر (err == DOM_NO_ERR && codebase != پوچ) {
		اگر (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			بازگشت false;
		}
		dom_string_unref(codebase);
	}
	اگر (params->codebase == پوچ)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	اگر (err == DOM_NO_ERR && classid != پوچ) {
		اگر (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			بازگشت false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	اگر (err == DOM_NO_ERR && data != پوچ) {
		اگر (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			بازگشت false;
		}
		dom_string_unref(data);
	}

	اگر (params->classid == پوچ && params->data == پوچ)
		/* nothing to embed; ignore */
		بازگشت true;

	/* Don't include ourself */
	اگر (params->classid != پوچ && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		بازگشت true;

	اگر (params->data != پوچ && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		بازگشت true;

	/* codetype and type are MIME types */
	اگر (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		بازگشت false;
	اگر (box_get_attribute(n, "type", params, &params->type) == false)
		بازگشت false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	اگر (params->classid != پوچ && params->data == پوچ &&
			params->codetype != پوچ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		اگر (lerror != lwc_error_ok)
			بازگشت false;

		اگر (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			بازگشت true;
		}

		lwc_string_unref(icodetype);
	}

	اگر (params->data != پوچ && params->type != پوچ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		اگر (lerror != lwc_error_ok)
			بازگشت false;

		اگر (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			بازگشت true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_گره_get_first_child(n, &c);
	اگر (err != DOM_NO_ERR)
		بازگشت false;

	وقتی (c != پوچ) {
		dom_گره *next;
		dom_گره_type type;

		err = dom_گره_get_گره_type(c, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			بازگشت false;
		}

		اگر (type == DOM_ELEMENT_گره) {
			dom_string *name;

			err = dom_گره_get_گره_name(c, &name);
			اگر (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_گره_unref(c);
				بشکن;
			}
			dom_string_unref(name);

			param = talloc(params, ساختار object_param);
			اگر (param == پوچ) {
				dom_گره_unref(c);
				بازگشت false;
			}
			param->name = پوچ;
			param->مقدار = پوچ;
			param->type = پوچ;
			param->valuetype = پوچ;
			param->next = پوچ;

			اگر (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (box_get_attribute(c, "value", param,
					&param->مقدار) == false) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_گره_unref(c);
				بازگشت false;
			}

			اگر (param->valuetype == پوچ) {
				param->valuetype = talloc_strdup(param, "data");
				اگر (param->valuetype == پوچ) {
					dom_گره_unref(c);
					بازگشت false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_گره_get_next_sibling(c, &next);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			بازگشت false;
		}

		dom_گره_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	اگر (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		بازگشت false;

	*convert_children = false;
	بازگشت true;
}


/**
 * Preformatted text [9.3.4].
 */
ایستا bool
box_pre(dom_گره *n,
	html_content *content,
	ساختار box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	بازگشت true;
}


/**
 * Option selector [17.6].
 */
ایستا bool
box_select(dom_گره *n,
	   html_content *content,
	   ساختار box *box,
	   bool *convert_children)
{
	ساختار box *inline_container;
	ساختار box *inline_box;
	ساختار form_control *gadget;
	dom_گره *c, *c2;
	dom_گره *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	اگر (gadget == پوچ)
		بازگشت false;

	gadget->html = content;
	err = dom_گره_get_first_child(n, &c);
	اگر (err != DOM_NO_ERR) {
		form_free_control(gadget);
		بازگشت false;
	}

	وقتی (c != پوچ) {
		dom_string *name;

		err = dom_گره_get_گره_name(c, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			form_free_control(gadget);
			بازگشت false;
		}

		اگر (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			اگر (box_select_add_option(gadget, c) == false) {
				dom_گره_unref(c);
				form_free_control(gadget);
				بازگشت false;
			}
		} وگرنه اگر (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_گره_get_first_child(c, &c2);
			اگر (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				form_free_control(gadget);
				بازگشت false;
			}

			وقتی (c2 != پوچ) {
				dom_string *c2_name;

				err = dom_گره_get_گره_name(c2, &c2_name);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(c2);
					dom_گره_unref(c);
					form_free_control(gadget);
					بازگشت false;
				}

				اگر (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					اگر (box_select_add_option(gadget,
							c2) == false) {
						dom_گره_unref(c2);
						dom_گره_unref(c);
						form_free_control(gadget);
						بازگشت false;
					}
				} وگرنه {
					dom_string_unref(c2_name);
				}

				err = dom_گره_get_next_sibling(c2, &next2);
				اگر (err != DOM_NO_ERR) {
					dom_گره_unref(c2);
					dom_گره_unref(c);
					form_free_control(gadget);
					بازگشت false;
				}

				dom_گره_unref(c2);
				c2 = next2;
			}
		} وگرنه {
			dom_string_unref(name);
		}

		err = dom_گره_get_next_sibling(c, &next);
		اگر (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			form_free_control(gadget);
			بازگشت false;
		}

		dom_گره_unref(c);
		c = next;
	}

	اگر (gadget->data.select.رقم_اقلام == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		بازگشت true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(پوچ, 0, false, 0, 0, 0, 0, content->bctx);
	اگر (inline_container == پوچ)
		برو no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(پوچ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	اگر (inline_box == پوچ)
		برو no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	اگر (gadget->data.select.multiple == false &&
			gadget->data.select.رقم_selected == 0) {
		gadget->data.select.current = gadget->data.select.اقلام;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.رقم_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->گره, true);
	}

	اگر (gadget->data.select.رقم_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	وگرنه اگر (gadget->data.select.رقم_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	وگرنه
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	اگر (inline_box->text == پوچ)
		برو no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	بازگشت true;

no_memory:
	بازگشت false;
}


/**
 * Multi-line text field [17.7].
 */
ایستا bool box_textarea(dom_گره *n,
			html_content *content,
			ساختار box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_گره(content->forms, n);
	اگر (box->gadget == پوچ)
		بازگشت false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	اگر (!box_درون_داد_text(content, box, n))
		بازگشت false;

	*convert_children = false;
	بازگشت true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_گره *گره,
			 html_content *content,
			 ساختار box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(گره, &tag_type);
	اگر (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	انتخاب (tag_type) {
	حالت DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_درون_داد:
		res = box_درون_داد(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(گره, content, box, convert_children);
		بشکن;

	حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(گره, content, box, convert_children);
		بشکن;

	پیشفرض:
		res = true;
	}

	بازگشت res;
}
