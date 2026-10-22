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

#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <dom/dom.h>

#समावेश "utils/nsoption.h"
#समावेश "utils/corestrings.h"
#समावेश "utils/log.h"
#समावेश "utils/messages.h"
#समावेश "utils/talloc.h"
#समावेश "utils/string.h"
#समावेश "utils/ascii.h"
#समावेश "utils/nsurl.h"
#समावेश "netsurf/plot_style.h"
#समावेश "css/hints.h"
#समावेश "desktop/frame_types.h"
#समावेश "content/content_factory.h"

#समावेश "html/html.h"
#समावेश "html/private.h"
#समावेश "html/object.h"
#समावेश "html/box.h"
#समावेश "html/box_manipulate.h"
#समावेश "html/box_construct.h"
#समावेश "html/box_special.h"
#समावेश "html/box_textarea.h"
#समावेश "html/form_internal.h"


अटल स्थिर content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
अटल अंतरभूत bool box_is_मूल(dom_गाँठ *n)
{
	dom_गाँठ *parent;
	dom_गाँठ_type type;
	dom_exception err;

	err = dom_गाँठ_get_parent_गाँठ(n, &parent);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (parent != शून्य) {
		err = dom_गाँठ_get_गाँठ_type(parent, &type);

		dom_गाँठ_unref(parent);

		अगर (err != DOM_NO_ERR)
			वापस false;

		अगर (type != DOM_DOCUMENT_गाँठ)
			वापस false;
	}

	वापस true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
अटल पूर्णांक box_object_talloc_destructor(काष्ठा object_params *o)
{
	अगर (o->codebase != शून्य)
		nsurl_unref(o->codebase);
	अगर (o->classid != शून्य)
		nsurl_unref(o->classid);
	अगर (o->तथ्य != शून्य)
		nsurl_unref(o->तथ्य);

	वापस 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
अटल काष्ठा frame_dimension *
box_parse_multi_lengths(स्थिर dom_string *ds, अचिन्हित पूर्णांक *गिनती)
{
	अक्षर *end;
	अचिन्हित पूर्णांक i, n;
	काष्ठा frame_dimension *लंबाई;
	स्थिर अक्षर *s;

	s = dom_string_तथ्य(ds);

	क्रम (i = 0, n = 1; s[i]; i++)
		अगर (s[i] == ',')
			n++;

	लंबाई = सुस्मृति(n, माप(काष्ठा frame_dimension));
	अगर (!लंबाई)
		वापस शून्य;

	क्रम (i = 0; i != n; i++) {
		जबतक (ascii_is_space(*s)) {
			s++;
		}
		लंबाई[i].मूल्य = strtof(s, &end);
		अगर (लंबाई[i].मूल्य <= 0) {
			लंबाई[i].मूल्य = 1;
		}
		s = end;
		चयन (*s) {
			हाल '%':
				लंबाई[i].unit = FRAME_DIMENSION_PERCENT;
				अवरोध;
			हाल '*':
				लंबाई[i].unit = FRAME_DIMENSION_RELATIVE;
				अवरोध;
			शेष:
				लंबाई[i].unit = FRAME_DIMENSION_PIXELS;
				अवरोध;
		}
		जबतक (*s && *s != ',') {
			s++;
		}
		अगर (*s == ',') {
			s++;
		}
	}

	*गिनती = n;
	वापस लंबाई;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
अटल पूर्णांक box_frames_talloc_destructor(काष्ठा content_html_frames *f)
{
	अगर (f->url != शून्य) {
		nsurl_unref(f->url);
		f->url = शून्य;
	}

	वापस 0;
}


/**
 * create a frameset box tree
 */
अटल bool
box_create_frameset(काष्ठा content_html_frames *f,
		    dom_गाँठ *n,
		    html_content *content)
{
	अचिन्हित पूर्णांक row, col, index, i;
	अचिन्हित पूर्णांक rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	काष्ठा frame_dimension *row_height = 0, *col_width = 0;
	dom_गाँठ *c, *अग्र;
	काष्ठा content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		अगर (row_height == शून्य)
			वापस false;
	} अन्यथा {
		row_height = सुस्मृति(1, माप(काष्ठा frame_dimension));
		अगर (row_height == शून्य)
			वापस false;
		row_height->मूल्य = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		अगर (col_width == शून्य) {
			मुक्त(row_height);
			वापस false;
		}
	} अन्यथा {
		col_width = सुस्मृति(1, माप(काष्ठा frame_dimension));
		अगर (col_width == शून्य) {
			मुक्त(row_height);
			वापस false;
		}
		col_width->मूल्य = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		अगर ((dom_string_तथ्य(s)[0] == '0') &&
				(dom_string_तथ्य(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		css_color color;

		अगर (nscss_parse_colour(dom_string_तथ्य(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, काष्ठा content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	क्रम (row = 0; row < rows; row++) {
		क्रम (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = शून्य;
			frame->url = शून्य;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = शून्य;
		}
	}
	मुक्त(col_width);
	मुक्त(row_height);

	/* create the frameset windows */
	err = dom_गाँठ_get_first_child(n, &c);
	अगर (err != DOM_NO_ERR)
		वापस false;

	क्रम (row = 0; c != शून्य && row < rows; row++) {
		क्रम (col = 0; c != शून्य && col < cols; col++) {
			जबतक (c != शून्य) {
				dom_गाँठ_type type;
				dom_string *name;

				err = dom_गाँठ_get_गाँठ_type(c, &type);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(c);
					वापस false;
				}

				err = dom_गाँठ_get_गाँठ_name(c, &name);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(c);
					वापस false;
				}

				अगर (type != DOM_ELEMENT_गाँठ ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_गाँठ_get_अग्र_sibling(c,
							&अग्र);
					अगर (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_गाँठ_unref(c);
						वापस false;
					}

					dom_string_unref(name);
					dom_गाँठ_unref(c);
					c = अग्र;
				} अन्यथा {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					अवरोध;
				}
			}

			अगर (c == शून्य)
				अवरोध;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_गाँठ_get_गाँठ_name(c, &s);
			अगर (err != DOM_NO_ERR) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				अगर (box_create_frameset(frame, c,
						content) == false) {
					dom_गाँठ_unref(c);
					वापस false;
				}

				err = dom_गाँठ_get_अग्र_sibling(c, &अग्र);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(c);
					वापस false;
				}

				dom_गाँठ_unref(c);
				c = अग्र;
				जारी;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = शून्य;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			अगर (url != शून्य) {
				/* no self-references */
				अगर (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = शून्य;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_तथ्य(s));
				dom_string_unref(s);
			}

			अगर (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				i = म_से_प(dom_string_तथ्य(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				अगर (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				अन्यथा अगर (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				frame->margin_width = म_से_प(dom_string_तथ्य(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				frame->margin_height = म_से_प(dom_string_तथ्य(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				css_color color;

				अगर (nscss_parse_colour(dom_string_तथ्य(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_गाँठ_get_अग्र_sibling(c, &अग्र);
			अगर (err != DOM_NO_ERR) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			dom_गाँठ_unref(c);
			c = अग्र;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	अगर (c != शून्य) {
		dom_गाँठ_unref(c);
	}

	वापस true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
अटल पूर्णांक box_iframes_talloc_destructor(काष्ठा content_html_iframe *f)
{
	अगर (f->url != शून्य) {
		nsurl_unref(f->url);
		f->url = शून्य;
	}

	वापस 0;
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
अटल bool
box_get_attribute(dom_गाँठ *n,
		  स्थिर अक्षर *attribute,
		  व्योम *context,
		  अक्षर **मूल्य)
{
	अक्षर *परिणाम;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((स्थिर uint8_t *) attribute,
			म_माप(attribute), &attr_name);
	अगर (err != DOM_NO_ERR)
		वापस false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	अगर (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		वापस false;
	}

	dom_string_unref(attr_name);

	अगर (attr != शून्य) {
		परिणाम = talloc_strdup(context, dom_string_तथ्य(attr));

		dom_string_unref(attr);

		अगर (परिणाम == शून्य)
			वापस false;

		*मूल्य = परिणाम;
	}

	वापस true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
अटल bool
box_निवेश_text(html_content *html, काष्ठा box *box, काष्ठा dom_गाँठ *गाँठ)
{
	काष्ठा box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	चयन (display) {
	हाल CSS_DISPLAY_GRID:
	हाल CSS_DISPLAY_FLEX:
	हाल CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		अवरोध;
	शेष:
		box->type = BOX_INLINE_BLOCK;
		अवरोध;
	}

	inline_container = box_create(शून्य, 0, false, 0, 0, 0, 0, html->bctx);
	अगर (!inline_container)
		वापस false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(शून्य, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	अगर (!inline_box)
		वापस false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	वापस box_textarea_create_textarea(html, box, गाँठ);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
अटल bool box_select_add_option(काष्ठा form_control *control, dom_गाँठ *n)
{
	अक्षर *मूल्य = शून्य;
	अक्षर *text = शून्य;
	अक्षर *text_nowrap = शून्य;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_गाँठ_get_text_content(n, &content);
	अगर (err != DOM_NO_ERR)
		वापस false;

	अगर (content != शून्य) {
		text = squash_whitespace(dom_string_तथ्य(content));
		dom_string_unref(content);
	} अन्यथा {
		text = strdup("");
	}

	अगर (text == शून्य)
		जाओ no_memory;

	err = dom_element_get_attribute(n, corestring_dom_मूल्य, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		मूल्य = strdup(dom_string_तथ्य(s));
		dom_string_unref(s);
	} अन्यथा {
		मूल्य = strdup(text);
	}

	अगर (मूल्य == शून्य)
		जाओ no_memory;

	अगर (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	अगर (text_nowrap == शून्य)
		जाओ no_memory;

	अगर (form_add_option(control, मूल्य, text_nowrap, selected, n) == false)
		जाओ no_memory;

	मुक्त(text);

	वापस true;

no_memory:
	मुक्त(मूल्य);
	मुक्त(text);
	मुक्त(text_nowrap);
	वापस false;
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
अटल bool
box_a(dom_गाँठ *n,
      html_content *content,
      काष्ठा box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		अगर (!ok)
			वापस false;
		अगर (url) {
			अगर (box->href != शून्य)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		अगर (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		अन्यथा अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_शीर्ष_2))
			box->target = "_top";
		अन्यथा अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		अन्यथा अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		अन्यथा {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_तथ्य(s));
			अगर (!box->target) {
				dom_string_unref(s);
				वापस false;
			}
		}
		dom_string_unref(s);
	}

	वापस true;
}


/**
 * Document body special element handler [7.5.1].
 */
अटल bool
box_body(dom_गाँठ *n,
	 html_content *content,
	 काष्ठा box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	अगर (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} अन्यथा {
		content->background_colour = nscss_color_to_ns(color);
	}

	वापस true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
अटल bool
box_br(dom_गाँठ *n,
       html_content *content,
       काष्ठा box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	वापस true;
}


/**
 * special element handler for Push button [17.5].
 */
अटल bool
box_button(dom_गाँठ *n,
	   html_content *content,
	   काष्ठा box *box,
	   bool *convert_children)
{
	काष्ठा form_control *gadget;

	gadget = html_forms_get_control_for_गाँठ(content->forms, n);
	अगर (!gadget)
		वापस false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	वापस true;
}


/**
 * Canvas element
 */
अटल bool
box_canvas(dom_गाँठ *n,
	     html_content *content,
	     काष्ठा box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	अगर (!content->enable_scripting) {
		वापस true;
	}
	*convert_children = false;

	अगर (box->style && ns_computed_display(box->style,
			box_is_मूल(n)) == CSS_DISPLAY_NONE)
		वापस true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	वापस true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
अटल bool
box_embed(dom_गाँठ *n,
	  html_content *content,
	  काष्ठा box *box,
	  bool *convert_children)
{
	काष्ठा object_params *params;
	काष्ठा object_param *param;
	dom_namednodemap *attrs;
	अचिन्हित दीर्घ idx;
	uint32_t अंक_attrs;
	dom_string *src;
	dom_exception err;

	अगर (box->style && ns_computed_display(box->style,
			box_is_मूल(n)) == CSS_DISPLAY_NONE)
		वापस true;

	params = talloc(content->bctx, काष्ठा object_params);
	अगर (params == शून्य)
		वापस false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->तथ्य = शून्य;
	params->type = शून्य;
	params->codetype = शून्य;
	params->codebase = शून्य;
	params->classid = शून्य;
	params->params = शून्य;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	अगर (err != DOM_NO_ERR || src == शून्य)
		वापस true;
	अगर (box_extract_link(content, src, content->base_url,
			     &params->तथ्य) == false) {
		dom_string_unref(src);
		वापस false;
	}

	dom_string_unref(src);

	अगर (params->तथ्य == शून्य)
		वापस true;

	/* Don't include ourself */
	अगर (nsurl_compare(content->base_url, params->तथ्य, NSURL_COMPLETE))
		वापस true;

	/* add attributes as parameters to linked list */
	err = dom_गाँठ_get_attributes(n, &attrs);
	अगर (err != DOM_NO_ERR)
		वापस false;

	err = dom_namednodemap_get_लंबाई(attrs, &अंक_attrs);
	अगर (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		वापस false;
	}

	क्रम (idx = 0; idx < अंक_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *मूल्य;

		err = dom_namednodemap_item(attrs, idx, (व्योम *) &attr);
		अगर (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			वापस false;
		}

		err = dom_attr_get_name(attr, &name);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(attr);
			dom_namednodemap_unref(attrs);
			वापस false;
		}

		अगर (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_गाँठ_unref(attr);
			dom_string_unref(name);
			जारी;
		}

		err = dom_attr_get_मूल्य(attr, &मूल्य);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			वापस false;
		}

		param = talloc(content->bctx, काष्ठा object_param);
		अगर (param == शून्य) {
			dom_गाँठ_unref(attr);
			dom_string_unref(मूल्य);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			वापस false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_तथ्य(name));
		param->मूल्य = talloc_strdup(content->bctx, dom_string_तथ्य(मूल्य));
		param->type = शून्य;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->अग्र = शून्य;

		dom_string_unref(मूल्य);
		dom_string_unref(name);
		dom_गाँठ_unref(attr);

		अगर (param->name == शून्य || param->मूल्य == शून्य ||
				param->valuetype == शून्य) {
			dom_namednodemap_unref(attrs);
			वापस false;
		}

		param->अग्र = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	वापस html_fetch_object(content, params->तथ्य, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
अटल bool
box_frameset(dom_गाँठ *n,
	     html_content *content,
	     काष्ठा box *box,
	     bool *convert_children)
{
	bool ok;

	अगर (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		अगर (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		वापस true;
	}

	content->frameset = talloc_zero(content->bctx,
					काष्ठा content_html_frames);
	अगर (!content->frameset) {
		वापस false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	अगर (ok) {
		box->type = BOX_NONE;
	}

	अगर (convert_children) {
		*convert_children = false;
	}
	वापस ok;
}


/**
 * Inline subwindow [16.5].
 */
अटल bool
box_iframe(dom_गाँठ *n,
	   html_content *content,
	   काष्ठा box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	काष्ठा content_html_iframe *iframe;
	पूर्णांक i;

	अगर (box->style && ns_computed_display(box->style,
			box_is_मूल(n)) == CSS_DISPLAY_NONE)
		वापस true;

	अगर (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		वापस true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	अगर (err != DOM_NO_ERR || s == शून्य)
		वापस true;
	अगर (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		वापस false;
	}
	dom_string_unref(s);
	अगर (url == शून्य)
		वापस true;

	/* don't include ourself */
	अगर (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		वापस true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, काष्ठा content_html_iframe);
	अगर (iframe == शून्य) {
		nsurl_unref(url);
		वापस false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = शून्य;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->अग्र = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		iframe->name = talloc_strdup(content->bctx, dom_string_तथ्य(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		i = म_से_प(dom_string_तथ्य(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		css_color color;

		अगर (nscss_parse_colour(dom_string_तथ्य(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		अन्यथा अगर (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		iframe->margin_width = म_से_प(dom_string_तथ्य(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		iframe->margin_height = म_से_प(dom_string_तथ्य(s));
		dom_string_unref(s);
	}

	/* box */
	निश्चित(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	अगर (convert_children)
		*convert_children = false;
	वापस true;
}


/**
 * Embedded image [13.2].
 */
अटल bool
box_image(dom_गाँठ *n,
	  html_content *content,
	  काष्ठा box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	क्रमागत css_width_e wtype;
	क्रमागत css_height_e htype;
	css_fixed मूल्य = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	अगर (box->style && ns_computed_display(box->style,
			box_is_मूल(n)) == CSS_DISPLAY_NONE)
		वापस true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	अगर (err == DOM_NO_ERR && s != शून्य) {
		अक्षर *alt = squash_whitespace(dom_string_तथ्य(s));
		dom_string_unref(s);
		अगर (alt == शून्य)
			वापस false;
		box->text = talloc_strdup(content->bctx, alt);
		मुक्त(alt);
		अगर (box->text == शून्य)
			वापस false;
		box->लंबाई = म_माप(box->text);
	}

	अगर (nsoption_bool(foreground_images) == false) {
		वापस true;
	}

	/* imagemap associated with this image */
	अगर (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		वापस false;
	अगर (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	अगर (err != DOM_NO_ERR || s == शून्य)
		वापस true;

	अगर (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		वापस false;
	}

	dom_string_unref(s);

	अगर (url == शून्य)
		वापस true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &मूल्य, &wunit);
	htype = css_computed_height(box->style, &मूल्य, &hunit);

	अगर (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	वापस ok;
}


/**
 * Form control [17.4].
 */
अटल bool
box_निवेश(dom_गाँठ *n,
	  html_content *content,
	  काष्ठा box *box,
	  bool *convert_children)
{
	काष्ठा form_control *gadget;
	dom_string *type = शून्य;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_गाँठ(content->forms, n);
	अगर (gadget == शून्य) {
		वापस false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	अगर ((err != DOM_NO_ERR) || (type == शून्य)) {
		/* no type so "text" is assumed */
		अगर (box_निवेश_text(content, box, n) == false) {
			वापस false;
		}
		*convert_children = false;
		वापस true;
	}

	अगर (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		अगर (box_निवेश_text(content, box, n) == false)
			जाओ no_memory;

	} अन्यथा अगर (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} अन्यथा अगर (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} अन्यथा अगर ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} अन्यथा अगर (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		काष्ठा box *inline_container, *inline_box;

		अगर (box_button(n, content, box, 0) == false)
			जाओ no_memory;

		inline_container = box_create(शून्य, 0, false, 0, 0, 0, 0,
				content->bctx);
		अगर (inline_container == शून्य)
			जाओ no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(शून्य, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		अगर (inline_box == शून्य)
			जाओ no_memory;

		inline_box->type = BOX_TEXT;

		अगर (box->gadget->मूल्य != शून्य)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->मूल्य);
		अन्यथा अगर (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		अन्यथा अगर (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		अन्यथा
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		अगर (inline_box->text == शून्य)
			जाओ no_memory;

		inline_box->लंबाई = म_माप(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} अन्यथा अगर (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		अगर (box->style && ns_computed_display(box->style,
				box_is_मूल(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			अगर (err == DOM_NO_ERR && s != शून्य) {
				error = nsurl_join(content->base_url,
						dom_string_तथ्य(s), &url);
				dom_string_unref(s);
				अगर (error != NSERROR_OK)
					जाओ no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				अगर (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					अगर (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						जाओ no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} अन्यथा {
		/* unhandled type the default is "text" */
		अगर (box_निवेश_text(content, box, n) == false)
			जाओ no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	वापस true;

no_memory:
	dom_string_unref(type);

	वापस false;
}


/**
 * Noscript element
 */
अटल bool
box_noscript(dom_गाँठ *n,
	     html_content *content,
	     काष्ठा box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	अगर (content->enable_scripting) {
		*convert_children = false;
	}

	वापस true;
}


/**
 * Generic embedded object [13.3].
 */
अटल bool
box_object(dom_गाँठ *n,
	   html_content *content,
	   काष्ठा box *box,
	   bool *convert_children)
{
	काष्ठा object_params *params;
	काष्ठा object_param *param;
	dom_string *codebase, *classid, *तथ्य;
	dom_गाँठ *c;
	dom_exception err;

	अगर (box->style && ns_computed_display(box->style,
			box_is_मूल(n)) == CSS_DISPLAY_NONE)
		वापस true;

	अगर (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		वापस false;
	अगर (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, काष्ठा object_params);
	अगर (params == शून्य)
		वापस false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->तथ्य = शून्य;
	params->type = शून्य;
	params->codetype = शून्य;
	params->codebase = शून्य;
	params->classid = शून्य;
	params->params = शून्य;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	अगर (err == DOM_NO_ERR && codebase != शून्य) {
		अगर (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			वापस false;
		}
		dom_string_unref(codebase);
	}
	अगर (params->codebase == शून्य)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	अगर (err == DOM_NO_ERR && classid != शून्य) {
		अगर (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			वापस false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_तथ्य, &तथ्य);
	अगर (err == DOM_NO_ERR && तथ्य != शून्य) {
		अगर (box_extract_link(content, तथ्य,
				params->codebase, &params->तथ्य) == false) {
			dom_string_unref(तथ्य);
			वापस false;
		}
		dom_string_unref(तथ्य);
	}

	अगर (params->classid == शून्य && params->तथ्य == शून्य)
		/* nothing to embed; ignore */
		वापस true;

	/* Don't include ourself */
	अगर (params->classid != शून्य && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		वापस true;

	अगर (params->तथ्य != शून्य && nsurl_compare(content->base_url,
			params->तथ्य, NSURL_COMPLETE))
		वापस true;

	/* codetype and type are MIME types */
	अगर (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		वापस false;
	अगर (box_get_attribute(n, "type", params, &params->type) == false)
		वापस false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	अगर (params->classid != शून्य && params->तथ्य == शून्य &&
			params->codetype != शून्य) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				म_माप(params->codetype), &icodetype);
		अगर (lerror != lwc_error_ok)
			वापस false;

		अगर (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			वापस true;
		}

		lwc_string_unref(icodetype);
	}

	अगर (params->तथ्य != शून्य && params->type != शून्य) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, म_माप(params->type),
				&itype);
		अगर (lerror != lwc_error_ok)
			वापस false;

		अगर (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			वापस true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_गाँठ_get_first_child(n, &c);
	अगर (err != DOM_NO_ERR)
		वापस false;

	जबतक (c != शून्य) {
		dom_गाँठ *अग्र;
		dom_गाँठ_type type;

		err = dom_गाँठ_get_गाँठ_type(c, &type);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(c);
			वापस false;
		}

		अगर (type == DOM_ELEMENT_गाँठ) {
			dom_string *name;

			err = dom_गाँठ_get_गाँठ_name(c, &name);
			अगर (err != DOM_NO_ERR) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_गाँठ_unref(c);
				अवरोध;
			}
			dom_string_unref(name);

			param = talloc(params, काष्ठा object_param);
			अगर (param == शून्य) {
				dom_गाँठ_unref(c);
				वापस false;
			}
			param->name = शून्य;
			param->मूल्य = शून्य;
			param->type = शून्य;
			param->valuetype = शून्य;
			param->अग्र = शून्य;

			अगर (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (box_get_attribute(c, "value", param,
					&param->मूल्य) == false) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_गाँठ_unref(c);
				वापस false;
			}

			अगर (param->valuetype == शून्य) {
				param->valuetype = talloc_strdup(param, "data");
				अगर (param->valuetype == शून्य) {
					dom_गाँठ_unref(c);
					वापस false;
				}
			}

			param->अग्र = params->params;
			params->params = param;
		}

		err = dom_गाँठ_get_अग्र_sibling(c, &अग्र);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(c);
			वापस false;
		}

		dom_गाँठ_unref(c);
		c = अग्र;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	अगर (!html_fetch_object(content,
			       params->तथ्य ? params->तथ्य : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		वापस false;

	*convert_children = false;
	वापस true;
}


/**
 * Preformatted text [9.3.4].
 */
अटल bool
box_pre(dom_गाँठ *n,
	html_content *content,
	काष्ठा box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	वापस true;
}


/**
 * Option selector [17.6].
 */
अटल bool
box_select(dom_गाँठ *n,
	   html_content *content,
	   काष्ठा box *box,
	   bool *convert_children)
{
	काष्ठा box *inline_container;
	काष्ठा box *inline_box;
	काष्ठा form_control *gadget;
	dom_गाँठ *c, *c2;
	dom_गाँठ *अग्र, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_गाँठ(content->forms, n);
	अगर (gadget == शून्य)
		वापस false;

	gadget->html = content;
	err = dom_गाँठ_get_first_child(n, &c);
	अगर (err != DOM_NO_ERR) {
		form_free_control(gadget);
		वापस false;
	}

	जबतक (c != शून्य) {
		dom_string *name;

		err = dom_गाँठ_get_गाँठ_name(c, &name);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(c);
			form_free_control(gadget);
			वापस false;
		}

		अगर (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			अगर (box_select_add_option(gadget, c) == false) {
				dom_गाँठ_unref(c);
				form_free_control(gadget);
				वापस false;
			}
		} अन्यथा अगर (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_गाँठ_get_first_child(c, &c2);
			अगर (err != DOM_NO_ERR) {
				dom_गाँठ_unref(c);
				form_free_control(gadget);
				वापस false;
			}

			जबतक (c2 != शून्य) {
				dom_string *c2_name;

				err = dom_गाँठ_get_गाँठ_name(c2, &c2_name);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(c2);
					dom_गाँठ_unref(c);
					form_free_control(gadget);
					वापस false;
				}

				अगर (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					अगर (box_select_add_option(gadget,
							c2) == false) {
						dom_गाँठ_unref(c2);
						dom_गाँठ_unref(c);
						form_free_control(gadget);
						वापस false;
					}
				} अन्यथा {
					dom_string_unref(c2_name);
				}

				err = dom_गाँठ_get_अग्र_sibling(c2, &next2);
				अगर (err != DOM_NO_ERR) {
					dom_गाँठ_unref(c2);
					dom_गाँठ_unref(c);
					form_free_control(gadget);
					वापस false;
				}

				dom_गाँठ_unref(c2);
				c2 = next2;
			}
		} अन्यथा {
			dom_string_unref(name);
		}

		err = dom_गाँठ_get_अग्र_sibling(c, &अग्र);
		अगर (err != DOM_NO_ERR) {
			dom_गाँठ_unref(c);
			form_free_control(gadget);
			वापस false;
		}

		dom_गाँठ_unref(c);
		c = अग्र;
	}

	अगर (gadget->तथ्य.select.अंक_वस्तुएँ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		वापस true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(शून्य, 0, false, 0, 0, 0, 0, content->bctx);
	अगर (inline_container == शून्य)
		जाओ no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(शून्य, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	अगर (inline_box == शून्य)
		जाओ no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	अगर (gadget->तथ्य.select.multiple == false &&
			gadget->तथ्य.select.अंक_selected == 0) {
		gadget->तथ्य.select.current = gadget->तथ्य.select.वस्तुएँ;
		gadget->तथ्य.select.current->initial_selected =
			gadget->तथ्य.select.current->selected = true;
		gadget->तथ्य.select.अंक_selected = 1;
		dom_html_option_element_set_selected(
				gadget->तथ्य.select.current->गाँठ, true);
	}

	अगर (gadget->तथ्य.select.अंक_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	अन्यथा अगर (gadget->तथ्य.select.अंक_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->तथ्य.select.current->text);
	अन्यथा
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	अगर (inline_box->text == शून्य)
		जाओ no_memory;

	inline_box->लंबाई = म_माप(inline_box->text);

	*convert_children = false;
	वापस true;

no_memory:
	वापस false;
}


/**
 * Multi-line text field [17.7].
 */
अटल bool box_textarea(dom_गाँठ *n,
			html_content *content,
			काष्ठा box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_गाँठ(content->forms, n);
	अगर (box->gadget == शून्य)
		वापस false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	अगर (!box_निवेश_text(content, box, n))
		वापस false;

	*convert_children = false;
	वापस true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_गाँठ *गाँठ,
			 html_content *content,
			 काष्ठा box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(गाँठ, &tag_type);
	अगर (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	चयन (tag_type) {
	हाल DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_निवेश:
		res = box_निवेश(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(गाँठ, content, box, convert_children);
		अवरोध;

	हाल DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(गाँठ, content, box, convert_children);
		अवरोध;

	शेष:
		res = true;
	}

	वापस res;
}
