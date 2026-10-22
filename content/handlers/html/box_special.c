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
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (parent != रिक्त) {
		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(parent, &type);

		dom_ग्रन्थिः_unref(parent);

		यदि (err != DOM_NO_ERR)
			प्रत्यागम false;

		यदि (type != DOM_DOCUMENT_ग्रन्थिः)
			प्रत्यागम false;
	}

	प्रत्यागम true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_object_talloc_destructor(संरचना object_params *o)
{
	यदि (o->codebase != रिक्त)
		nsurl_unref(o->codebase);
	यदि (o->classid != रिक्त)
		nsurl_unref(o->classid);
	यदि (o->दत्त != रिक्त)
		nsurl_unref(o->दत्त);

	प्रत्यागम 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
स्थिर संरचना frame_dimension *
box_parse_multi_lengths(ध्रुव dom_string *ds, अचिह्नित पूर्णाङ्क *गणनम्)
{
	वर्ण *अन्तः;
	अचिह्नित पूर्णाङ्क i, n;
	संरचना frame_dimension *दीर्घता;
	ध्रुव वर्ण *s;

	s = dom_string_दत्त(ds);

	प्रति (i = 0, n = 1; s[i]; i++)
		यदि (s[i] == ',')
			n++;

	दीर्घता = शून्यस्मृतिं_देहि(n, परिमाण(संरचना frame_dimension));
	यदि (!दीर्घता)
		प्रत्यागम रिक्त;

	प्रति (i = 0; i != n; i++) {
		यावत् (ascii_is_space(*s)) {
			s++;
		}
		दीर्घता[i].मानम् = strtof(s, &अन्तः);
		यदि (दीर्घता[i].मानम् <= 0) {
			दीर्घता[i].मानम् = 1;
		}
		s = अन्तः;
		विकल्प (*s) {
			पक्ष '%':
				दीर्घता[i].unit = FRAME_DIMENSION_PERCENT;
				भङ्ग;
			पक्ष '*':
				दीर्घता[i].unit = FRAME_DIMENSION_RELATIVE;
				भङ्ग;
			स्वभाव:
				दीर्घता[i].unit = FRAME_DIMENSION_PIXELS;
				भङ्ग;
		}
		यावत् (*s && *s != ',') {
			s++;
		}
		यदि (*s == ',') {
			s++;
		}
	}

	*गणनम् = n;
	प्रत्यागम दीर्घता;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_चौकटाः_talloc_destructor(संरचना content_html_चौकटाः *f)
{
	यदि (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	प्रत्यागम 0;
}


/**
 * create a frameset box tree
 */
स्थिर bool
box_create_frameset(संरचना content_html_चौकटाः *f,
		    dom_ग्रन्थिः *n,
		    html_content *content)
{
	अचिह्नित पूर्णाङ्क row, col, index, i;
	अचिह्नित पूर्णाङ्क पङ्क्तिकाः = 1, स्तम्भसङ्ख्या = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	संरचना frame_dimension *row_height = 0, *col_विस्तारः = 0;
	dom_ग्रन्थिः *c, *अग्र;
	संरचना content_html_चौकटाः *frame;
	bool default_border = true;
	रङ्गः default_border_रङ्गः = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_पङ्क्तिकाः, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		row_height = box_parse_multi_lengths(s, &पङ्क्तिकाः);
		dom_string_unref(s);
		यदि (row_height == रिक्त)
			प्रत्यागम false;
	} अन्यथा {
		row_height = शून्यस्मृतिं_देहि(1, परिमाण(संरचना frame_dimension));
		यदि (row_height == रिक्त)
			प्रत्यागम false;
		row_height->मानम् = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_स्तम्भसङ्ख्या, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		col_विस्तारः = box_parse_multi_lengths(s, &स्तम्भसङ्ख्या);
		dom_string_unref(s);
		यदि (col_विस्तारः == रिक्त) {
			मुक्त(row_height);
			प्रत्यागम false;
		}
	} अन्यथा {
		col_विस्तारः = शून्यस्मृतिं_देहि(1, परिमाण(संरचना frame_dimension));
		यदि (col_विस्तारः == रिक्त) {
			मुक्त(row_height);
			प्रत्यागम false;
		}
		col_विस्तारः->मानम् = 100;
		col_विस्तारः->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		यदि ((dom_string_दत्त(s)[0] == '0') &&
				(dom_string_दत्त(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		यदि (nscss_parse_रङ्गः(dom_string_दत्त(s), &color))
			default_border_रङ्गः = nscss_color_गन्तव्यम्_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->स्तम्भसङ्ख्या = स्तम्भसङ्ख्या;
	f->पङ्क्तिकाः = पङ्क्तिकाः;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, संरचना content_html_चौकटाः,
								(पङ्क्तिकाः * स्तम्भसङ्ख्या));

	talloc_समुच्चयः_destructor(f->children, box_चौकटाः_talloc_destructor);

	प्रति (row = 0; row < पङ्क्तिकाः; row++) {
		प्रति (col = 0; col < स्तम्भसङ्ख्या; col++) {
			index = (row * स्तम्भसङ्ख्या) + col;
			frame = &f->children[index];
			frame->स्तम्भसङ्ख्या = 0;
			frame->पङ्क्तिकाः = 0;
			frame->विस्तारः = col_विस्तारः[col];
			frame->height = row_height[row];
			frame->margin_विस्तारः = 0;
			frame->margin_height = 0;
			frame->name = रिक्त;
			frame->url = रिक्त;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_रङ्गः = default_border_रङ्गः;
			frame->children = रिक्त;
		}
	}
	मुक्त(col_विस्तारः);
	मुक्त(row_height);

	/* create the frameset windows */
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	प्रति (row = 0; c != रिक्त && row < पङ्क्तिकाः; row++) {
		प्रति (col = 0; c != रिक्त && col < स्तम्भसङ्ख्या; col++) {
			यावत् (c != रिक्त) {
				dom_ग्रन्थिः_type type;
				dom_string *name;

				err = dom_ग्रन्थिः_get_ग्रन्थिः_type(c, &type);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					प्रत्यागम false;
				}

				err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					प्रत्यागम false;
				}

				यदि (type != DOM_ELEMENT_ग्रन्थिः ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ग्रन्थिः_get_अग्र_sibling(c,
							&अग्र);
					यदि (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ग्रन्थिः_unref(c);
						प्रत्यागम false;
					}

					dom_string_unref(name);
					dom_ग्रन्थिः_unref(c);
					c = अग्र;
				} अन्यथा {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					भङ्ग;
				}
			}

			यदि (c == रिक्त)
				भङ्ग;

			/* get current frame */
			index = (row * स्तम्भसङ्ख्या) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &s);
			यदि (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				यदि (box_create_frameset(frame, c,
						content) == false) {
					dom_ग्रन्थिः_unref(c);
					प्रत्यागम false;
				}

				err = dom_ग्रन्थिः_get_अग्र_sibling(c, &अग्र);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c);
					प्रत्यागम false;
				}

				dom_ग्रन्थिः_unref(c);
				c = अग्र;
				अनुवर्त;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = रिक्त;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				box_extract_link(content, s, content->आधारः_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			यदि (url != रिक्त) {
				/* no self-references */
				यदि (nsurl_compare(content->आधारः_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = रिक्त;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_दत्त(s));
				dom_string_unref(s);
			}

			यदि (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				i = atoi(dom_string_दत्त(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				यदि (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				अन्यथा यदि (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_विस्तारः = atoi(dom_string_दत्त(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				frame->margin_height = atoi(dom_string_दत्त(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				css_color color;

				यदि (nscss_parse_रङ्गः(dom_string_दत्त(s),
						&color))
					frame->border_रङ्गः =
						nscss_color_गन्तव्यम्_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ग्रन्थिः_get_अग्र_sibling(c, &अग्र);
			यदि (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			dom_ग्रन्थिः_unref(c);
			c = अग्र;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	यदि (c != रिक्त) {
		dom_ग्रन्थिः_unref(c);
	}

	प्रत्यागम true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
स्थिर पूर्णाङ्क box_iframes_talloc_destructor(संरचना content_html_iframe *f)
{
	यदि (f->url != रिक्त) {
		nsurl_unref(f->url);
		f->url = रिक्त;
	}

	प्रत्यागम 0;
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
	वर्ण *परिणाम;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ध्रुव uint8_t *) attribute,
			वाक्यदैर्घ्यम्(attribute), &attr_name);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	यदि (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		प्रत्यागम false;
	}

	dom_string_unref(attr_name);

	यदि (attr != रिक्त) {
		परिणाम = talloc_strdup(context, dom_string_दत्त(attr));

		dom_string_unref(attr);

		यदि (परिणाम == रिक्त)
			प्रत्यागम false;

		*मानम् = परिणाम;
	}

	प्रत्यागम true;
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
	पक्ष CSS_DISPLAY_खण्डम्:
		box->type = BOX_खण्डम्;
		भङ्ग;
	स्वभाव:
		box->type = BOX_INLINE_खण्डम्;
		भङ्ग;
	}

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, html->bctx);
	यदि (!inline_container)
		प्रत्यागम false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	यदि (!inline_box)
		प्रत्यागम false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_सङ्कलय_child(inline_container, inline_box);
	box_सङ्कलय_child(box, inline_container);

	प्रत्यागम box_textarea_create_textarea(html, box, ग्रन्थिः);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
स्थिर bool box_select_सङ्कलय_option(संरचना form_control *control, dom_ग्रन्थिः *n)
{
	वर्ण *मानम् = रिक्त;
	वर्ण *text = रिक्त;
	वर्ण *text_nowrap = रिक्त;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ग्रन्थिः_get_text_content(n, &content);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यदि (content != रिक्त) {
		text = squash_whitespace(dom_string_दत्त(content));
		dom_string_unref(content);
	} अन्यथा {
		text = strdup("");
	}

	यदि (text == रिक्त)
		गच्छ no_स्मृतिः;

	err = dom_element_get_attribute(n, corestring_dom_मानम्, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		मानम् = strdup(dom_string_दत्त(s));
		dom_string_unref(s);
	} अन्यथा {
		मानम् = strdup(text);
	}

	यदि (मानम् == रिक्त)
		गच्छ no_स्मृतिः;

	यदि (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	यदि (text_nowrap == रिक्त)
		गच्छ no_स्मृतिः;

	यदि (form_सङ्कलय_option(control, मानम्, text_nowrap, selected, n) == false)
		गच्छ no_स्मृतिः;

	मुक्त(text);

	प्रत्यागम true;

no_स्मृतिः:
	मुक्त(मानम्);
	मुक्त(text);
	मुक्त(text_nowrap);
	प्रत्यागम false;
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
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		ok = box_extract_link(content, s, content->आधारः_url, &url);
		dom_string_unref(s);
		यदि (!ok)
			प्रत्यागम false;
		यदि (url) {
			यदि (box->href != रिक्त)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		यदि (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		अन्यथा यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_शिखरम्_2))
			box->target = "_top";
		अन्यथा यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		अन्यथा यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		अन्यथा {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_दत्त(s));
			यदि (!box->target) {
				dom_string_unref(s);
				प्रत्यागम false;
			}
		}
		dom_string_unref(s);
	}

	प्रत्यागम true;
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
	यदि (nscss_color_is_transparent(color)) {
		content->background_रङ्गः = NS_TRANSPARENT;
	} अन्यथा {
		content->background_रङ्गः = nscss_color_गन्तव्यम्_ns(color);
	}

	प्रत्यागम true;
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
	प्रत्यागम true;
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
	यदि (!gadget)
		प्रत्यागम false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_खण्डम्;

	/* Just render the contents */

	प्रत्यागम true;
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
	यदि (!content->enable_scripting) {
		प्रत्यागम true;
	}
	*convert_children = false;

	यदि (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		प्रत्यागम true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	प्रत्यागम true;
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
	अचिह्नित दीर्घ idx;
	uint32_t अङ्कः_attrs;
	dom_string *src;
	dom_exception err;

	यदि (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		प्रत्यागम true;

	params = talloc(content->bctx, संरचना object_params);
	यदि (params == रिक्त)
		प्रत्यागम false;

	talloc_समुच्चयः_destructor(params, box_object_talloc_destructor);

	params->दत्त = रिक्त;
	params->type = रिक्त;
	params->codetype = रिक्त;
	params->codebase = रिक्त;
	params->classid = रिक्त;
	params->params = रिक्त;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	यदि (err != DOM_NO_ERR || src == रिक्त)
		प्रत्यागम true;
	यदि (box_extract_link(content, src, content->आधारः_url,
			     &params->दत्त) == false) {
		dom_string_unref(src);
		प्रत्यागम false;
	}

	dom_string_unref(src);

	यदि (params->दत्त == रिक्त)
		प्रत्यागम true;

	/* Don't include ourself */
	यदि (nsurl_compare(content->आधारः_url, params->दत्त, NSURL_COMPLETE))
		प्रत्यागम true;

	/* add attributes as parameters to linked list */
	err = dom_ग्रन्थिः_get_गुणाः(n, &attrs);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	err = dom_namednodemap_get_दीर्घता(attrs, &अङ्कः_attrs);
	यदि (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		प्रत्यागम false;
	}

	प्रति (idx = 0; idx < अङ्कः_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *मानम्;

		err = dom_namednodemap_item(attrs, idx, (शून्य *) &attr);
		यदि (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			प्रत्यागम false;
		}

		err = dom_attr_get_name(attr, &name);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(attr);
			dom_namednodemap_unref(attrs);
			प्रत्यागम false;
		}

		यदि (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(name);
			अनुवर्त;
		}

		err = dom_attr_get_मानम्(attr, &मानम्);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			प्रत्यागम false;
		}

		param = talloc(content->bctx, संरचना object_param);
		यदि (param == रिक्त) {
			dom_ग्रन्थिः_unref(attr);
			dom_string_unref(मानम्);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			प्रत्यागम false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_दत्त(name));
		param->मानम् = talloc_strdup(content->bctx, dom_string_दत्त(मानम्));
		param->type = रिक्त;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->अग्र = रिक्त;

		dom_string_unref(मानम्);
		dom_string_unref(name);
		dom_ग्रन्थिः_unref(attr);

		यदि (param->name == रिक्त || param->मानम् == रिक्त ||
				param->valuetype == रिक्त) {
			dom_namednodemap_unref(attrs);
			प्रत्यागम false;
		}

		param->अग्र = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	प्रत्यागम html_fetch_object(content, params->दत्त, box, CONTENT_ANY, false);
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

	यदि (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		यदि (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		प्रत्यागम true;
	}

	content->frameset = talloc_zero(content->bctx,
					संरचना content_html_चौकटाः);
	यदि (!content->frameset) {
		प्रत्यागम false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	यदि (ok) {
		box->type = BOX_NONE;
	}

	यदि (convert_children) {
		*convert_children = false;
	}
	प्रत्यागम ok;
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

	यदि (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		प्रत्यागम true;

	यदि (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		प्रत्यागम true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	यदि (err != DOM_NO_ERR || s == रिक्त)
		प्रत्यागम true;
	यदि (box_extract_link(content, s, content->आधारः_url, &url) == false) {
		dom_string_unref(s);
		प्रत्यागम false;
	}
	dom_string_unref(s);
	यदि (url == रिक्त)
		प्रत्यागम true;

	/* don't include ourself */
	यदि (nsurl_compare(content->आधारः_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		प्रत्यागम true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, संरचना content_html_iframe);
	यदि (iframe == रिक्त) {
		nsurl_unref(url);
		प्रत्यागम false;
	}

	talloc_समुच्चयः_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_विस्तारः = 0;
	iframe->margin_height = 0;
	iframe->name = रिक्त;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->अग्र = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		iframe->name = talloc_strdup(content->bctx, dom_string_दत्त(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		i = atoi(dom_string_दत्त(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		css_color color;

		यदि (nscss_parse_रङ्गः(dom_string_दत्त(s), &color))
			iframe->border_रङ्गः = nscss_color_गन्तव्यम्_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		अन्यथा यदि (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_विस्तारः = atoi(dom_string_दत्त(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		iframe->margin_height = atoi(dom_string_दत्त(s));
		dom_string_unref(s);
	}

	/* box */
	प्रतिज्ञा(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	यदि (convert_children)
		*convert_children = false;
	प्रत्यागम true;
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
	गणना css_विस्तारः_e wtype;
	गणना css_height_e htype;
	css_fixed मानम् = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	यदि (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		प्रत्यागम true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	यदि (err == DOM_NO_ERR && s != रिक्त) {
		वर्ण *alt = squash_whitespace(dom_string_दत्त(s));
		dom_string_unref(s);
		यदि (alt == रिक्त)
			प्रत्यागम false;
		box->text = talloc_strdup(content->bctx, alt);
		मुक्त(alt);
		यदि (box->text == रिक्त)
			प्रत्यागम false;
		box->दीर्घता = वाक्यदैर्घ्यम्(box->text);
	}

	यदि (nsoption_bool(foreground_images) == false) {
		प्रत्यागम true;
	}

	/* imagemap associated with this image */
	यदि (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		प्रत्यागम false;
	यदि (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	यदि (err != DOM_NO_ERR || s == रिक्त)
		प्रत्यागम true;

	यदि (box_extract_link(content, s, content->आधारः_url, &url) == false) {
		dom_string_unref(s);
		प्रत्यागम false;
	}

	dom_string_unref(s);

	यदि (url == रिक्त)
		प्रत्यागम true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_विस्तारः(box->style, &मानम्, &wunit);
	htype = css_computed_height(box->style, &मानम्, &hunit);

	यदि (wtype == CSS_विस्तारः_समुच्चयः &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_समुच्चयः &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	प्रत्यागम ok;
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
	यदि (gadget == रिक्त) {
		प्रत्यागम false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	यदि ((err != DOM_NO_ERR) || (type == रिक्त)) {
		/* no type so "text" is assumed */
		यदि (box_निवेशः_text(content, box, n) == false) {
			प्रत्यागम false;
		}
		*convert_children = false;
		प्रत्यागम true;
	}

	यदि (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		यदि (box_निवेशः_text(content, box, n) == false)
			गच्छ no_स्मृतिः;

	} अन्यथा यदि (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_खण्डम्;

	} अन्यथा यदि (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} अन्यथा यदि ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} अन्यथा यदि (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		संरचना box *inline_container, *inline_box;

		यदि (box_button(n, content, box, 0) == false)
			गच्छ no_स्मृतिः;

		inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0,
				content->bctx);
		यदि (inline_container == रिक्त)
			गच्छ no_स्मृतिः;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(रिक्त, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		यदि (inline_box == रिक्त)
			गच्छ no_स्मृतिः;

		inline_box->type = BOX_TEXT;

		यदि (box->gadget->मानम् != रिक्त)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->मानम्);
		अन्यथा यदि (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		अन्यथा यदि (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		अन्यथा
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		यदि (inline_box->text == रिक्त)
			गच्छ no_स्मृतिः;

		inline_box->दीर्घता = वाक्यदैर्घ्यम्(inline_box->text);

		box_सङ्कलय_child(inline_container, inline_box);

		box_सङ्कलय_child(box, inline_container);

	} अन्यथा यदि (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		यदि (box->style && ns_computed_display(box->style,
				box_is_मूलम्(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			यदि (err == DOM_NO_ERR && s != रिक्त) {
				error = nsurl_join(content->आधारः_url,
						dom_string_दत्त(s), &url);
				dom_string_unref(s);
				यदि (error != NSERROR_OK)
					गच्छ no_स्मृतिः;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				यदि (nsurl_compare(url, content->आधारः_url,
						NSURL_COMPLETE) == false) {
					यदि (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						गच्छ no_स्मृतिः;
					}
				}
				nsurl_unref(url);
			}
		}
	} अन्यथा {
		/* unhandled type the default is "text" */
		यदि (box_निवेशः_text(content, box, n) == false)
			गच्छ no_स्मृतिः;
	}

	dom_string_unref(type);

	*convert_children = false;

	प्रत्यागम true;

no_स्मृतिः:
	dom_string_unref(type);

	प्रत्यागम false;
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
	यदि (content->enable_scripting) {
		*convert_children = false;
	}

	प्रत्यागम true;
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
	dom_string *codebase, *classid, *दत्त;
	dom_ग्रन्थिः *c;
	dom_exception err;

	यदि (box->style && ns_computed_display(box->style,
			box_is_मूलम्(n)) == CSS_DISPLAY_NONE)
		प्रत्यागम true;

	यदि (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		प्रत्यागम false;
	यदि (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, संरचना object_params);
	यदि (params == रिक्त)
		प्रत्यागम false;

	talloc_समुच्चयः_destructor(params, box_object_talloc_destructor);

	params->दत्त = रिक्त;
	params->type = रिक्त;
	params->codetype = रिक्त;
	params->codebase = रिक्त;
	params->classid = रिक्त;
	params->params = रिक्त;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	यदि (err == DOM_NO_ERR && codebase != रिक्त) {
		यदि (box_extract_link(content, codebase,	content->आधारः_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			प्रत्यागम false;
		}
		dom_string_unref(codebase);
	}
	यदि (params->codebase == रिक्त)
		params->codebase = nsurl_ref(content->आधारः_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	यदि (err == DOM_NO_ERR && classid != रिक्त) {
		यदि (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			प्रत्यागम false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_दत्त, &दत्त);
	यदि (err == DOM_NO_ERR && दत्त != रिक्त) {
		यदि (box_extract_link(content, दत्त,
				params->codebase, &params->दत्त) == false) {
			dom_string_unref(दत्त);
			प्रत्यागम false;
		}
		dom_string_unref(दत्त);
	}

	यदि (params->classid == रिक्त && params->दत्त == रिक्त)
		/* nothing to embed; ignore */
		प्रत्यागम true;

	/* Don't include ourself */
	यदि (params->classid != रिक्त && nsurl_compare(content->आधारः_url,
			params->classid, NSURL_COMPLETE))
		प्रत्यागम true;

	यदि (params->दत्त != रिक्त && nsurl_compare(content->आधारः_url,
			params->दत्त, NSURL_COMPLETE))
		प्रत्यागम true;

	/* codetype and type are MIME types */
	यदि (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		प्रत्यागम false;
	यदि (box_get_attribute(n, "type", params, &params->type) == false)
		प्रत्यागम false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	यदि (params->classid != रिक्त && params->दत्त == रिक्त &&
			params->codetype != रिक्त) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				वाक्यदैर्घ्यम्(params->codetype), &icodetype);
		यदि (lerror != lwc_error_ok)
			प्रत्यागम false;

		यदि (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			प्रत्यागम true;
		}

		lwc_string_unref(icodetype);
	}

	यदि (params->दत्त != रिक्त && params->type != रिक्त) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, वाक्यदैर्घ्यम्(params->type),
				&itype);
		यदि (lerror != lwc_error_ok)
			प्रत्यागम false;

		यदि (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			प्रत्यागम true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	यदि (err != DOM_NO_ERR)
		प्रत्यागम false;

	यावत् (c != रिक्त) {
		dom_ग्रन्थिः *अग्र;
		dom_ग्रन्थिः_type type;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_type(c, &type);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			प्रत्यागम false;
		}

		यदि (type == DOM_ELEMENT_ग्रन्थिः) {
			dom_string *name;

			err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
			यदि (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (!dom_string_caseless_lwc_isequal(name,
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
			यदि (param == रिक्त) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}
			param->name = रिक्त;
			param->मानम् = रिक्त;
			param->type = रिक्त;
			param->valuetype = रिक्त;
			param->अग्र = रिक्त;

			यदि (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (box_get_attribute(c, "value", param,
					&param->मानम्) == false) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ग्रन्थिः_unref(c);
				प्रत्यागम false;
			}

			यदि (param->valuetype == रिक्त) {
				param->valuetype = talloc_strdup(param, "data");
				यदि (param->valuetype == रिक्त) {
					dom_ग्रन्थिः_unref(c);
					प्रत्यागम false;
				}
			}

			param->अग्र = params->params;
			params->params = param;
		}

		err = dom_ग्रन्थिः_get_अग्र_sibling(c, &अग्र);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			प्रत्यागम false;
		}

		dom_ग्रन्थिः_unref(c);
		c = अग्र;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	यदि (!html_fetch_object(content,
			       params->दत्त ? params->दत्त : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		प्रत्यागम false;

	*convert_children = false;
	प्रत्यागम true;
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
	प्रत्यागम true;
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
	dom_ग्रन्थिः *अग्र, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ग्रन्थिः(content->forms, n);
	यदि (gadget == रिक्त)
		प्रत्यागम false;

	gadget->html = content;
	err = dom_ग्रन्थिः_get_first_child(n, &c);
	यदि (err != DOM_NO_ERR) {
		form_free_control(gadget);
		प्रत्यागम false;
	}

	यावत् (c != रिक्त) {
		dom_string *name;

		err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c, &name);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			form_free_control(gadget);
			प्रत्यागम false;
		}

		यदि (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			यदि (box_select_सङ्कलय_option(gadget, c) == false) {
				dom_ग्रन्थिः_unref(c);
				form_free_control(gadget);
				प्रत्यागम false;
			}
		} अन्यथा यदि (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ग्रन्थिः_get_first_child(c, &c2);
			यदि (err != DOM_NO_ERR) {
				dom_ग्रन्थिः_unref(c);
				form_free_control(gadget);
				प्रत्यागम false;
			}

			यावत् (c2 != रिक्त) {
				dom_string *c2_name;

				err = dom_ग्रन्थिः_get_ग्रन्थिः_name(c2, &c2_name);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c2);
					dom_ग्रन्थिः_unref(c);
					form_free_control(gadget);
					प्रत्यागम false;
				}

				यदि (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					यदि (box_select_सङ्कलय_option(gadget,
							c2) == false) {
						dom_ग्रन्थिः_unref(c2);
						dom_ग्रन्थिः_unref(c);
						form_free_control(gadget);
						प्रत्यागम false;
					}
				} अन्यथा {
					dom_string_unref(c2_name);
				}

				err = dom_ग्रन्थिः_get_अग्र_sibling(c2, &next2);
				यदि (err != DOM_NO_ERR) {
					dom_ग्रन्थिः_unref(c2);
					dom_ग्रन्थिः_unref(c);
					form_free_control(gadget);
					प्रत्यागम false;
				}

				dom_ग्रन्थिः_unref(c2);
				c2 = next2;
			}
		} अन्यथा {
			dom_string_unref(name);
		}

		err = dom_ग्रन्थिः_get_अग्र_sibling(c, &अग्र);
		यदि (err != DOM_NO_ERR) {
			dom_ग्रन्थिः_unref(c);
			form_free_control(gadget);
			प्रत्यागम false;
		}

		dom_ग्रन्थिः_unref(c);
		c = अग्र;
	}

	यदि (gadget->दत्त.select.अङ्कः_वस्तूनि == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		प्रत्यागम true;
	}

	box->type = BOX_INLINE_खण्डम्;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(रिक्त, 0, false, 0, 0, 0, 0, content->bctx);
	यदि (inline_container == रिक्त)
		गच्छ no_स्मृतिः;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(रिक्त, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	यदि (inline_box == रिक्त)
		गच्छ no_स्मृतिः;
	inline_box->type = BOX_TEXT;
	box_सङ्कलय_child(inline_container, inline_box);
	box_सङ्कलय_child(box, inline_container);

	यदि (gadget->दत्त.select.गुणजः == false &&
			gadget->दत्त.select.अङ्कः_selected == 0) {
		gadget->दत्त.select.current = gadget->दत्त.select.वस्तूनि;
		gadget->दत्त.select.current->initial_selected =
			gadget->दत्त.select.current->selected = true;
		gadget->दत्त.select.अङ्कः_selected = 1;
		dom_html_option_element_समुच्चयः_selected(
				gadget->दत्त.select.current->ग्रन्थिः, true);
	}

	यदि (gadget->दत्त.select.अङ्कः_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	अन्यथा यदि (gadget->दत्त.select.अङ्कः_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->दत्त.select.current->text);
	अन्यथा
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	यदि (inline_box->text == रिक्त)
		गच्छ no_स्मृतिः;

	inline_box->दीर्घता = वाक्यदैर्घ्यम्(inline_box->text);

	*convert_children = false;
	प्रत्यागम true;

no_स्मृतिः:
	प्रत्यागम false;
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
	यदि (box->gadget == रिक्त)
		प्रत्यागम false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	यदि (!box_निवेशः_text(content, box, n))
		प्रत्यागम false;

	*convert_children = false;
	प्रत्यागम true;
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
	dom_html_element_type चिह्नकम्_type;
	bool res;

	exc = dom_html_element_get_चिह्नकम्_type(ग्रन्थिः, &चिह्नकम्_type);
	यदि (exc != DOM_NO_ERR) {
		चिह्नकम्_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	विकल्प (चिह्नकम्_type) {
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

	प्रत्यागम res;
}
