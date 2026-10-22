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


स्थिर स्थिरांक content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
स्थिर ओळीत bool box_is_मूळ(dom_गाठ *n)
{
	dom_गाठ *parent;
	dom_गाठ_type type;
	dom_exception err;

	err = dom_गाठ_get_parent_गाठ(n, &parent);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (parent != रिक्त) {
		err = dom_गाठ_get_गाठ_type(parent, &type);

		dom_गाठ_unref(parent);

		जर (err != DOM_NO_ERR)
			परत false;

		जर (type != DOM_DOCUMENT_गाठ)
			परत false;
	}

	परत true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णांक box_object_talloc_destructor(संरचना object_params *o)
{
	जर (o->codebase != रिक्त)
		nsurl_unref(o->codebase);
	जर (o->classid != रिक्त)
		nsurl_unref(o->classid);
	जर (o->data != रिक्त)
		nsurl_unref(o->data);

	परत 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
स्थिर संरचना frame_dimension *
box_parse_multi_lengths(स्थिरांक dom_string *ds, अचिन्हित पूर्णांक *मोजणी)
{
	अक्षर *end;
	अचिन्हित पूर्णांक i, n;
	संरचना frame_dimension *length;
	स्थिरांक अक्षर *s;

	s = dom_string_data(ds);

	साठी (i = 0, n = 1; s[i]; i++)
		जर (s[i] == ',')
			n++;

	length = सुस्मृति(n, आकार(संरचना frame_dimension));
	जर (!length)
		परत रिक्त;

	साठी (i = 0; i != n; i++) {
		पर्यंत (ascii_is_space(*s)) {
			s++;
		}
		length[i].मूल्य = strtof(s, &end);
		जर (length[i].मूल्य <= 0) {
			length[i].मूल्य = 1;
		}
		s = end;
		निवड (*s) {
			स्थिती '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				थांबा;
			स्थिती '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				थांबा;
			पूर्वनिर्धारित:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				थांबा;
		}
		पर्यंत (*s && *s != ',') {
			s++;
		}
		जर (*s == ',') {
			s++;
		}
	}

	*मोजणी = n;
	परत length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णांक box_frames_talloc_destructor(संरचना content_html_frames *f)
{
	जर (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	परत 0;
}


/**
 * create a frameset box tree
 */
स्थिर bool
box_create_frameset(संरचना content_html_frames *f,
		    dom_गाठ *n,
		    html_content *content)
{
	अचिन्हित पूर्णांक row, col, index, i;
	अचिन्हित पूर्णांक rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	संरचना frame_dimension *row_height = 0, *col_width = 0;
	dom_गाठ *c, *next;
	संरचना content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		जर (row_height == रिक्त)
			परत false;
	} अन्यथा {
		row_height = सुस्मृति(1, आकार(संरचना frame_dimension));
		जर (row_height == रिक्त)
			परत false;
		row_height->मूल्य = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		जर (col_width == रिक्त) {
			मुक्त(row_height);
			परत false;
		}
	} अन्यथा {
		col_width = सुस्मृति(1, आकार(संरचना frame_dimension));
		जर (col_width == रिक्त) {
			मुक्त(row_height);
			परत false;
		}
		col_width->मूल्य = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		जर ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		जर (nscss_parse_colour(dom_string_data(s), &color))
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

	साठी (row = 0; row < rows; row++) {
		साठी (col = 0; col < cols; col++) {
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
	err = dom_गाठ_get_first_child(n, &c);
	जर (err != DOM_NO_ERR)
		परत false;

	साठी (row = 0; c != रिक्त && row < rows; row++) {
		साठी (col = 0; c != रिक्त && col < cols; col++) {
			पर्यंत (c != रिक्त) {
				dom_गाठ_type type;
				dom_string *name;

				err = dom_गाठ_get_गाठ_type(c, &type);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(c);
					परत false;
				}

				err = dom_गाठ_get_गाठ_name(c, &name);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(c);
					परत false;
				}

				जर (type != DOM_ELEMENT_गाठ ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_गाठ_get_next_sibling(c,
							&next);
					जर (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_गाठ_unref(c);
						परत false;
					}

					dom_string_unref(name);
					dom_गाठ_unref(c);
					c = next;
				} अन्यथा {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					थांबा;
				}
			}

			जर (c == रिक्त)
				थांबा;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_गाठ_get_गाठ_name(c, &s);
			जर (err != DOM_NO_ERR) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				जर (box_create_frameset(frame, c,
						content) == false) {
					dom_गाठ_unref(c);
					परत false;
				}

				err = dom_गाठ_get_next_sibling(c, &next);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(c);
					परत false;
				}

				dom_गाठ_unref(c);
				c = next;
				सुरूठेवा;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = रिक्त;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			जर (url != रिक्त) {
				/* no self-references */
				जर (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = रिक्त;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			जर (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				i = म_से_प(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				जर (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				अन्यथा जर (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_width = म_से_प(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_height = म_से_प(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				css_color color;

				जर (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_गाठ_get_next_sibling(c, &next);
			जर (err != DOM_NO_ERR) {
				dom_गाठ_unref(c);
				परत false;
			}

			dom_गाठ_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	जर (c != रिक्त) {
		dom_गाठ_unref(c);
	}

	परत true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णांक box_iframes_talloc_destructor(संरचना content_html_iframe *f)
{
	जर (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	परत 0;
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
box_get_attribute(dom_गाठ *n,
		  स्थिरांक अक्षर *attribute,
		  शून्य *context,
		  अक्षर **मूल्य)
{
	अक्षर *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((स्थिरांक uint8_t *) attribute,
			म_माप(attribute), &attr_name);
	जर (err != DOM_NO_ERR)
		परत false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	जर (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		परत false;
	}

	dom_string_unref(attr_name);

	जर (attr != रिक्त) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		जर (result == रिक्त)
			परत false;

		*मूल्य = result;
	}

	परत true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
स्थिर bool
box_आदान_text(html_content *html, संरचना box *box, संरचना dom_गाठ *गाठ)
{
	संरचना box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	निवड (display) {
	स्थिती CSS_DISPLAY_GRID:
	स्थिती CSS_DISPLAY_FLEX:
	स्थिती CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		थांबा;
	पूर्वनिर्धारित:
		box->type = BOX_INLINE_BLOCK;
		थांबा;
	}

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, html->bctx);
	जर (!inline_container)
		परत false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	जर (!inline_box)
		परत false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	परत box_textarea_create_textarea(html, box, गाठ);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool box_select_add_option(संरचना form_control *control, dom_गाठ *n)
{
	अक्षर *मूल्य = रिक्त;
	अक्षर *text = रिक्त;
	अक्षर *text_nowrap = रिक्त;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_गाठ_get_text_content(n, &content);
	जर (err != DOM_NO_ERR)
		परत false;

	जर (content != रिक्त) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} अन्यथा {
		text = strdup("");
	}

	जर (text == रिक्त)
		जा no_memory;

	err = dom_element_get_attribute(n, corestring_dom_मूल्य, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		मूल्य = strdup(dom_string_data(s));
		dom_string_unref(s);
	} अन्यथा {
		मूल्य = strdup(text);
	}

	जर (मूल्य == रिक्त)
		जा no_memory;

	जर (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	जर (text_nowrap == रिक्त)
		जा no_memory;

	जर (form_add_option(control, मूल्य, text_nowrap, selected, n) == false)
		जा no_memory;

	मुक्त(text);

	परत true;

no_memory:
	मुक्त(मूल्य);
	मुक्त(text);
	मुक्त(text_nowrap);
	परत false;
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
box_a(dom_गाठ *n,
      html_content *content,
      संरचना box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		जर (!ok)
			परत false;
		जर (url) {
			जर (box->href != रिक्त)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		जर (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		अन्यथा जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_शीर्ष_2))
			box->target = "_top";
		अन्यथा जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		अन्यथा जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		अन्यथा {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			जर (!box->target) {
				dom_string_unref(s);
				परत false;
			}
		}
		dom_string_unref(s);
	}

	परत true;
}


/**
 * Document body special element handler [7.5.1].
 */
स्थिर bool
box_body(dom_गाठ *n,
	 html_content *content,
	 संरचना box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	जर (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} अन्यथा {
		content->background_colour = nscss_color_to_ns(color);
	}

	परत true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
स्थिर bool
box_br(dom_गाठ *n,
       html_content *content,
       संरचना box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	परत true;
}


/**
 * special element handler for Push button [17.5].
 */
स्थिर bool
box_button(dom_गाठ *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना form_control *gadget;

	gadget = html_forms_get_control_for_गाठ(content->forms, n);
	जर (!gadget)
		परत false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	परत true;
}


/**
 * Canvas element
 */
स्थिर bool
box_canvas(dom_गाठ *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	जर (!content->enable_scripting) {
		परत true;
	}
	*convert_children = false;

	जर (box->style && ns_computed_display(box->style,
			box_is_मूळ(n)) == CSS_DISPLAY_NONE)
		परत true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	परत true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
स्थिर bool
box_embed(dom_गाठ *n,
	  html_content *content,
	  संरचना box *box,
	  bool *convert_children)
{
	संरचना object_params *params;
	संरचना object_param *param;
	dom_namednodemap *attrs;
	अचिन्हित दीर्घ idx;
	uint32_t अंक_attrs;
	dom_string *src;
	dom_exception err;

	जर (box->style && ns_computed_display(box->style,
			box_is_मूळ(n)) == CSS_DISPLAY_NONE)
		परत true;

	params = talloc(content->bctx, संरचना object_params);
	जर (params == रिक्त)
		परत false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = रिक्त;
	params->type = रिक्त;
	params->codetype = रिक्त;
	params->codebase = रिक्त;
	params->classid = रिक्त;
	params->params = रिक्त;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	जर (err != DOM_NO_ERR || src == रिक्त)
		परत true;
	जर (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		परत false;
	}

	dom_string_unref(src);

	जर (params->data == रिक्त)
		परत true;

	/* Don't include ourself */
	जर (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		परत true;

	/* add attributes as parameters to linked list */
	err = dom_गाठ_get_attributes(n, &attrs);
	जर (err != DOM_NO_ERR)
		परत false;

	err = dom_namednodemap_get_length(attrs, &अंक_attrs);
	जर (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		परत false;
	}

	साठी (idx = 0; idx < अंक_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *मूल्य;

		err = dom_namednodemap_item(attrs, idx, (शून्य *) &attr);
		जर (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			परत false;
		}

		err = dom_attr_get_name(attr, &name);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(attr);
			dom_namednodemap_unref(attrs);
			परत false;
		}

		जर (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_गाठ_unref(attr);
			dom_string_unref(name);
			सुरूठेवा;
		}

		err = dom_attr_get_मूल्य(attr, &मूल्य);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			परत false;
		}

		param = talloc(content->bctx, संरचना object_param);
		जर (param == रिक्त) {
			dom_गाठ_unref(attr);
			dom_string_unref(मूल्य);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			परत false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->मूल्य = talloc_strdup(content->bctx, dom_string_data(मूल्य));
		param->type = रिक्त;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = रिक्त;

		dom_string_unref(मूल्य);
		dom_string_unref(name);
		dom_गाठ_unref(attr);

		जर (param->name == रिक्त || param->मूल्य == रिक्त ||
				param->valuetype == रिक्त) {
			dom_namednodemap_unref(attrs);
			परत false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	परत html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
स्थिर bool
box_frameset(dom_गाठ *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	bool ok;

	जर (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		जर (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		परत true;
	}

	content->frameset = talloc_zero(content->bctx,
					संरचना content_html_frames);
	जर (!content->frameset) {
		परत false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	जर (ok) {
		box->type = BOX_NONE;
	}

	जर (convert_children) {
		*convert_children = false;
	}
	परत ok;
}


/**
 * Inline subwindow [16.5].
 */
स्थिर bool
box_iframe(dom_गाठ *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	संरचना content_html_iframe *iframe;
	पूर्णांक i;

	जर (box->style && ns_computed_display(box->style,
			box_is_मूळ(n)) == CSS_DISPLAY_NONE)
		परत true;

	जर (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		परत true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	जर (err != DOM_NO_ERR || s == रिक्त)
		परत true;
	जर (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		परत false;
	}
	dom_string_unref(s);
	जर (url == रिक्त)
		परत true;

	/* don't include ourself */
	जर (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		परत true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, संरचना content_html_iframe);
	जर (iframe == रिक्त) {
		nsurl_unref(url);
		परत false;
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
	जर (err == DOM_NO_ERR && s != रिक्त) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		i = म_से_प(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		जर (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		अन्यथा जर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_width = म_से_प(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_height = म_से_प(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	प्रतिज्ञा(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	जर (convert_children)
		*convert_children = false;
	परत true;
}


/**
 * Embedded image [13.2].
 */
स्थिर bool
box_image(dom_गाठ *n,
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
	css_fixed मूल्य = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	जर (box->style && ns_computed_display(box->style,
			box_is_मूळ(n)) == CSS_DISPLAY_NONE)
		परत true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	जर (err == DOM_NO_ERR && s != रिक्त) {
		अक्षर *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		जर (alt == रिक्त)
			परत false;
		box->text = talloc_strdup(content->bctx, alt);
		मुक्त(alt);
		जर (box->text == रिक्त)
			परत false;
		box->length = म_माप(box->text);
	}

	जर (nsoption_bool(foreground_images) == false) {
		परत true;
	}

	/* imagemap associated with this image */
	जर (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		परत false;
	जर (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	जर (err != DOM_NO_ERR || s == रिक्त)
		परत true;

	जर (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		परत false;
	}

	dom_string_unref(s);

	जर (url == रिक्त)
		परत true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &मूल्य, &wunit);
	htype = css_computed_height(box->style, &मूल्य, &hunit);

	जर (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	परत ok;
}


/**
 * Form control [17.4].
 */
स्थिर bool
box_आदान(dom_गाठ *n,
	  html_content *content,
	  संरचना box *box,
	  bool *convert_children)
{
	संरचना form_control *gadget;
	dom_string *type = रिक्त;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_गाठ(content->forms, n);
	जर (gadget == रिक्त) {
		परत false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	जर ((err != DOM_NO_ERR) || (type == रिक्त)) {
		/* no type so "text" is assumed */
		जर (box_आदान_text(content, box, n) == false) {
			परत false;
		}
		*convert_children = false;
		परत true;
	}

	जर (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		जर (box_आदान_text(content, box, n) == false)
			जा no_memory;

	} अन्यथा जर (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} अन्यथा जर (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} अन्यथा जर ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} अन्यथा जर (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		संरचना box *inline_container, *inline_box;

		जर (box_button(n, content, box, 0) == false)
			जा no_memory;

		inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0,
				content->bctx);
		जर (inline_container == रिक्त)
			जा no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(रिक्त, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		जर (inline_box == रिक्त)
			जा no_memory;

		inline_box->type = BOX_TEXT;

		जर (box->gadget->मूल्य != रिक्त)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->मूल्य);
		अन्यथा जर (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		अन्यथा जर (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		अन्यथा
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		जर (inline_box->text == रिक्त)
			जा no_memory;

		inline_box->length = म_माप(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} अन्यथा जर (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		जर (box->style && ns_computed_display(box->style,
				box_is_मूळ(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			जर (err == DOM_NO_ERR && s != रिक्त) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				जर (error != NSERROR_OK)
					जा no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				जर (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					जर (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						जा no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} अन्यथा {
		/* unhandled type the default is "text" */
		जर (box_आदान_text(content, box, n) == false)
			जा no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	परत true;

no_memory:
	dom_string_unref(type);

	परत false;
}


/**
 * Noscript element
 */
स्थिर bool
box_noscript(dom_गाठ *n,
	     html_content *content,
	     संरचना box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	जर (content->enable_scripting) {
		*convert_children = false;
	}

	परत true;
}


/**
 * Generic embedded object [13.3].
 */
स्थिर bool
box_object(dom_गाठ *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना object_params *params;
	संरचना object_param *param;
	dom_string *codebase, *classid, *data;
	dom_गाठ *c;
	dom_exception err;

	जर (box->style && ns_computed_display(box->style,
			box_is_मूळ(n)) == CSS_DISPLAY_NONE)
		परत true;

	जर (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		परत false;
	जर (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, संरचना object_params);
	जर (params == रिक्त)
		परत false;

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
	जर (err == DOM_NO_ERR && codebase != रिक्त) {
		जर (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			परत false;
		}
		dom_string_unref(codebase);
	}
	जर (params->codebase == रिक्त)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	जर (err == DOM_NO_ERR && classid != रिक्त) {
		जर (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			परत false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	जर (err == DOM_NO_ERR && data != रिक्त) {
		जर (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			परत false;
		}
		dom_string_unref(data);
	}

	जर (params->classid == रिक्त && params->data == रिक्त)
		/* nothing to embed; ignore */
		परत true;

	/* Don't include ourself */
	जर (params->classid != रिक्त && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		परत true;

	जर (params->data != रिक्त && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		परत true;

	/* codetype and type are MIME types */
	जर (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		परत false;
	जर (box_get_attribute(n, "type", params, &params->type) == false)
		परत false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	जर (params->classid != रिक्त && params->data == रिक्त &&
			params->codetype != रिक्त) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				म_माप(params->codetype), &icodetype);
		जर (lerror != lwc_error_ok)
			परत false;

		जर (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			परत true;
		}

		lwc_string_unref(icodetype);
	}

	जर (params->data != रिक्त && params->type != रिक्त) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, म_माप(params->type),
				&itype);
		जर (lerror != lwc_error_ok)
			परत false;

		जर (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			परत true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_गाठ_get_first_child(n, &c);
	जर (err != DOM_NO_ERR)
		परत false;

	पर्यंत (c != रिक्त) {
		dom_गाठ *next;
		dom_गाठ_type type;

		err = dom_गाठ_get_गाठ_type(c, &type);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(c);
			परत false;
		}

		जर (type == DOM_ELEMENT_गाठ) {
			dom_string *name;

			err = dom_गाठ_get_गाठ_name(c, &name);
			जर (err != DOM_NO_ERR) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_गाठ_unref(c);
				थांबा;
			}
			dom_string_unref(name);

			param = talloc(params, संरचना object_param);
			जर (param == रिक्त) {
				dom_गाठ_unref(c);
				परत false;
			}
			param->name = रिक्त;
			param->मूल्य = रिक्त;
			param->type = रिक्त;
			param->valuetype = रिक्त;
			param->next = रिक्त;

			जर (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (box_get_attribute(c, "value", param,
					&param->मूल्य) == false) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_गाठ_unref(c);
				परत false;
			}

			जर (param->valuetype == रिक्त) {
				param->valuetype = talloc_strdup(param, "data");
				जर (param->valuetype == रिक्त) {
					dom_गाठ_unref(c);
					परत false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_गाठ_get_next_sibling(c, &next);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(c);
			परत false;
		}

		dom_गाठ_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	जर (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		परत false;

	*convert_children = false;
	परत true;
}


/**
 * Preformatted text [9.3.4].
 */
स्थिर bool
box_pre(dom_गाठ *n,
	html_content *content,
	संरचना box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	परत true;
}


/**
 * Option selector [17.6].
 */
स्थिर bool
box_select(dom_गाठ *n,
	   html_content *content,
	   संरचना box *box,
	   bool *convert_children)
{
	संरचना box *inline_container;
	संरचना box *inline_box;
	संरचना form_control *gadget;
	dom_गाठ *c, *c2;
	dom_गाठ *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_गाठ(content->forms, n);
	जर (gadget == रिक्त)
		परत false;

	gadget->html = content;
	err = dom_गाठ_get_first_child(n, &c);
	जर (err != DOM_NO_ERR) {
		form_free_control(gadget);
		परत false;
	}

	पर्यंत (c != रिक्त) {
		dom_string *name;

		err = dom_गाठ_get_गाठ_name(c, &name);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(c);
			form_free_control(gadget);
			परत false;
		}

		जर (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			जर (box_select_add_option(gadget, c) == false) {
				dom_गाठ_unref(c);
				form_free_control(gadget);
				परत false;
			}
		} अन्यथा जर (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_गाठ_get_first_child(c, &c2);
			जर (err != DOM_NO_ERR) {
				dom_गाठ_unref(c);
				form_free_control(gadget);
				परत false;
			}

			पर्यंत (c2 != रिक्त) {
				dom_string *c2_name;

				err = dom_गाठ_get_गाठ_name(c2, &c2_name);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(c2);
					dom_गाठ_unref(c);
					form_free_control(gadget);
					परत false;
				}

				जर (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					जर (box_select_add_option(gadget,
							c2) == false) {
						dom_गाठ_unref(c2);
						dom_गाठ_unref(c);
						form_free_control(gadget);
						परत false;
					}
				} अन्यथा {
					dom_string_unref(c2_name);
				}

				err = dom_गाठ_get_next_sibling(c2, &next2);
				जर (err != DOM_NO_ERR) {
					dom_गाठ_unref(c2);
					dom_गाठ_unref(c);
					form_free_control(gadget);
					परत false;
				}

				dom_गाठ_unref(c2);
				c2 = next2;
			}
		} अन्यथा {
			dom_string_unref(name);
		}

		err = dom_गाठ_get_next_sibling(c, &next);
		जर (err != DOM_NO_ERR) {
			dom_गाठ_unref(c);
			form_free_control(gadget);
			परत false;
		}

		dom_गाठ_unref(c);
		c = next;
	}

	जर (gadget->data.select.अंक_वस्तुसंच == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		परत true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, content->bctx);
	जर (inline_container == रिक्त)
		जा no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	जर (inline_box == रिक्त)
		जा no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	जर (gadget->data.select.multiple == false &&
			gadget->data.select.अंक_selected == 0) {
		gadget->data.select.current = gadget->data.select.वस्तुसंच;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.अंक_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->गाठ, true);
	}

	जर (gadget->data.select.अंक_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	अन्यथा जर (gadget->data.select.अंक_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	अन्यथा
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	जर (inline_box->text == रिक्त)
		जा no_memory;

	inline_box->length = म_माप(inline_box->text);

	*convert_children = false;
	परत true;

no_memory:
	परत false;
}


/**
 * Multi-line text field [17.7].
 */
स्थिर bool box_textarea(dom_गाठ *n,
			html_content *content,
			संरचना box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_गाठ(content->forms, n);
	जर (box->gadget == रिक्त)
		परत false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	जर (!box_आदान_text(content, box, n))
		परत false;

	*convert_children = false;
	परत true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_गाठ *गाठ,
			 html_content *content,
			 संरचना box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(गाठ, &tag_type);
	जर (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	निवड (tag_type) {
	स्थिती DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_आदान:
		res = box_आदान(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(गाठ, content, box, convert_children);
		थांबा;

	स्थिती DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(गाठ, content, box, convert_children);
		थांबा;

	पूर्वनिर्धारित:
		res = true;
	}

	परत res;
}
