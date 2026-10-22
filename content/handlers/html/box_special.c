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

#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ "utils/nsoption.h"
#അന്തര്ഭാവ "utils/corestrings.h"
#അന്തര്ഭാവ "utils/log.h"
#അന്തര്ഭാവ "utils/messages.h"
#അന്തര്ഭാവ "utils/talloc.h"
#അന്തര്ഭാവ "utils/string.h"
#അന്തര്ഭാവ "utils/ascii.h"
#അന്തര്ഭാവ "utils/nsurl.h"
#അന്തര്ഭാവ "netsurf/plot_style.h"
#അന്തര്ഭാവ "css/hints.h"
#അന്തര്ഭാവ "desktop/frame_types.h"
#അന്തര്ഭാവ "content/content_factory.h"

#അന്തര്ഭാവ "html/html.h"
#അന്തര്ഭാവ "html/private.h"
#അന്തര്ഭാവ "html/object.h"
#അന്തര്ഭാവ "html/box.h"
#അന്തര്ഭാവ "html/box_manipulate.h"
#അന്തര്ഭാവ "html/box_construct.h"
#അന്തര്ഭാവ "html/box_special.h"
#അന്തര്ഭാവ "html/box_textarea.h"
#അന്തര്ഭാവ "html/form_internal.h"


സ്ഥിതം സ്ഥിരം content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
സ്ഥിതം വരിയിൽ bool box_is_വേര്(dom_കണ്ണി *n)
{
	dom_കണ്ണി *parent;
	dom_കണ്ണി_type type;
	dom_exception err;

	err = dom_കണ്ണി_get_parent_കണ്ണി(n, &parent);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (parent != ശൂന്യ) {
		err = dom_കണ്ണി_get_കണ്ണി_type(parent, &type);

		dom_കണ്ണി_unref(parent);

		എങ്കിൽ (err != DOM_NO_ERR)
			തിരികെ false;

		എങ്കിൽ (type != DOM_DOCUMENT_കണ്ണി)
			തിരികെ false;
	}

	തിരികെ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ box_object_talloc_destructor(ഘടന object_params *o)
{
	എങ്കിൽ (o->codebase != ശൂന്യ)
		nsurl_unref(o->codebase);
	എങ്കിൽ (o->classid != ശൂന്യ)
		nsurl_unref(o->classid);
	എങ്കിൽ (o->data != ശൂന്യ)
		nsurl_unref(o->data);

	തിരികെ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
സ്ഥിതം ഘടന frame_dimension *
box_parse_multi_lengths(സ്ഥിരം dom_string *ds, unsigned പൂർണ്ണസംഖ്യ *എണ്ണം)
{
	അക്ഷരം *end;
	unsigned പൂർണ്ണസംഖ്യ i, n;
	ഘടന frame_dimension *length;
	സ്ഥിരം അക്ഷരം *s;

	s = dom_string_data(ds);

	വേണ്ടി (i = 0, n = 1; s[i]; i++)
		എങ്കിൽ (s[i] == ',')
			n++;

	length = calloc(n, വലുപ്പം(ഘടന frame_dimension));
	എങ്കിൽ (!length)
		തിരികെ ശൂന്യ;

	വേണ്ടി (i = 0; i != n; i++) {
		ആകുമ്പോൾ (ascii_is_space(*s)) {
			s++;
		}
		length[i].മൂല്യം = strtof(s, &end);
		എങ്കിൽ (length[i].മൂല്യം <= 0) {
			length[i].മൂല്യം = 1;
		}
		s = end;
		തിരഞ്ഞെടുപ്പ് (*s) {
			അവസ്ഥ '%':
				length[i].unit = FRAME_DIMENSION_PERCENT;
				നിർത്തുക;
			അവസ്ഥ '*':
				length[i].unit = FRAME_DIMENSION_RELATIVE;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				length[i].unit = FRAME_DIMENSION_PIXELS;
				നിർത്തുക;
		}
		ആകുമ്പോൾ (*s && *s != ',') {
			s++;
		}
		എങ്കിൽ (*s == ',') {
			s++;
		}
	}

	*എണ്ണം = n;
	തിരികെ length;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ box_frames_talloc_destructor(ഘടന content_html_frames *f)
{
	എങ്കിൽ (f->url != ശൂന്യ) {
		nsurl_unref(f->url);
		f->url = ശൂന്യ;
	}

	തിരികെ 0;
}


/**
 * create a frameset box tree
 */
സ്ഥിതം bool
box_create_frameset(ഘടന content_html_frames *f,
		    dom_കണ്ണി *n,
		    html_content *content)
{
	unsigned പൂർണ്ണസംഖ്യ row, col, index, i;
	unsigned പൂർണ്ണസംഖ്യ rows = 1, cols = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ഘടന frame_dimension *row_height = 0, *col_width = 0;
	dom_കണ്ണി *c, *next;
	ഘടന content_html_frames *frame;
	bool default_border = true;
	colour default_border_colour = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_rows, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		row_height = box_parse_multi_lengths(s, &rows);
		dom_string_unref(s);
		എങ്കിൽ (row_height == ശൂന്യ)
			തിരികെ false;
	} അല്ലെങ്കിൽ {
		row_height = calloc(1, വലുപ്പം(ഘടന frame_dimension));
		എങ്കിൽ (row_height == ശൂന്യ)
			തിരികെ false;
		row_height->മൂല്യം = 100;
		row_height->unit = FRAME_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_cols, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		col_width = box_parse_multi_lengths(s, &cols);
		dom_string_unref(s);
		എങ്കിൽ (col_width == ശൂന്യ) {
			സ്വതന്ത്രം(row_height);
			തിരികെ false;
		}
	} അല്ലെങ്കിൽ {
		col_width = calloc(1, വലുപ്പം(ഘടന frame_dimension));
		എങ്കിൽ (col_width == ശൂന്യ) {
			സ്വതന്ത്രം(row_height);
			തിരികെ false;
		}
		col_width->മൂല്യം = 100;
		col_width->unit = FRAME_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		എങ്കിൽ ((dom_string_data(s)[0] == '0') &&
				(dom_string_data(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		css_color color;

		എങ്കിൽ (nscss_parse_colour(dom_string_data(s), &color))
			default_border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->cols = cols;
	f->rows = rows;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ഘടന content_html_frames,
								(rows * cols));

	talloc_set_destructor(f->children, box_frames_talloc_destructor);

	വേണ്ടി (row = 0; row < rows; row++) {
		വേണ്ടി (col = 0; col < cols; col++) {
			index = (row * cols) + col;
			frame = &f->children[index];
			frame->cols = 0;
			frame->rows = 0;
			frame->width = col_width[col];
			frame->height = row_height[row];
			frame->margin_width = 0;
			frame->margin_height = 0;
			frame->name = ശൂന്യ;
			frame->url = ശൂന്യ;
			frame->no_resize = false;
			frame->scrolling = BW_SCROLLING_AUTO;
			frame->border = default_border;
			frame->border_colour = default_border_colour;
			frame->children = ശൂന്യ;
		}
	}
	സ്വതന്ത്രം(col_width);
	സ്വതന്ത്രം(row_height);

	/* create the frameset windows */
	err = dom_കണ്ണി_get_first_child(n, &c);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	വേണ്ടി (row = 0; c != ശൂന്യ && row < rows; row++) {
		വേണ്ടി (col = 0; c != ശൂന്യ && col < cols; col++) {
			ആകുമ്പോൾ (c != ശൂന്യ) {
				dom_കണ്ണി_type type;
				dom_string *name;

				err = dom_കണ്ണി_get_കണ്ണി_type(c, &type);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(c);
					തിരികെ false;
				}

				err = dom_കണ്ണി_get_കണ്ണി_name(c, &name);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(c);
					തിരികെ false;
				}

				എങ്കിൽ (type != DOM_ELEMENT_കണ്ണി ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frame) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_കണ്ണി_get_next_sibling(c,
							&next);
					എങ്കിൽ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_കണ്ണി_unref(c);
						തിരികെ false;
					}

					dom_string_unref(name);
					dom_കണ്ണി_unref(c);
					c = next;
				} അല്ലെങ്കിൽ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					നിർത്തുക;
				}
			}

			എങ്കിൽ (c == ശൂന്യ)
				നിർത്തുക;

			/* get current frame */
			index = (row * cols) + col;
			frame = &f->children[index];

			/* nest framesets */
			err = dom_കണ്ണി_get_കണ്ണി_name(c, &s);
			എങ്കിൽ (err != DOM_NO_ERR) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				frame->border = 0;
				എങ്കിൽ (box_create_frameset(frame, c,
						content) == false) {
					dom_കണ്ണി_unref(c);
					തിരികെ false;
				}

				err = dom_കണ്ണി_get_next_sibling(c, &next);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(c);
					തിരികെ false;
				}

				dom_കണ്ണി_unref(c);
				c = next;
				തുടരുക;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ശൂന്യ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				box_extract_link(content, s, content->base_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			എങ്കിൽ (url != ശൂന്യ) {
				/* no self-references */
				എങ്കിൽ (nsurl_compare(content->base_url, url,
						NSURL_COMPLETE) == false)
					frame->url = url;
				url = ശൂന്യ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				frame->name = talloc_strdup(content->bctx,
						dom_string_data(s));
				dom_string_unref(s);
			}

			എങ്കിൽ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &frame->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				frame->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				i = atoi(dom_string_data(s));
				frame->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					frame->scrolling = BW_SCROLLING_YES;
				അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					frame->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				frame->margin_width = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				frame->margin_height = atoi(dom_string_data(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				css_color color;

				എങ്കിൽ (nscss_parse_colour(dom_string_data(s),
						&color))
					frame->border_colour =
						nscss_color_to_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_കണ്ണി_get_next_sibling(c, &next);
			എങ്കിൽ (err != DOM_NO_ERR) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			dom_കണ്ണി_unref(c);
			c = next;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	എങ്കിൽ (c != ശൂന്യ) {
		dom_കണ്ണി_unref(c);
	}

	തിരികെ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ box_iframes_talloc_destructor(ഘടന content_html_iframe *f)
{
	എങ്കിൽ (f->url != ശൂന്യ) {
		nsurl_unref(f->url);
		f->url = ശൂന്യ;
	}

	തിരികെ 0;
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
സ്ഥിതം bool
box_get_attribute(dom_കണ്ണി *n,
		  സ്ഥിരം അക്ഷരം *attribute,
		  ശൂന്യം *context,
		  അക്ഷരം **മൂല്യം)
{
	അക്ഷരം *result;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((സ്ഥിരം uint8_t *) attribute,
			strlen(attribute), &attr_name);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		തിരികെ false;
	}

	dom_string_unref(attr_name);

	എങ്കിൽ (attr != ശൂന്യ) {
		result = talloc_strdup(context, dom_string_data(attr));

		dom_string_unref(attr);

		എങ്കിൽ (result == ശൂന്യ)
			തിരികെ false;

		*മൂല്യം = result;
	}

	തിരികെ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
സ്ഥിതം bool
box_നിവേശം_text(html_content *html, ഘടന box *box, ഘടന dom_കണ്ണി *കണ്ണി)
{
	ഘടന box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	തിരഞ്ഞെടുപ്പ് (display) {
	അവസ്ഥ CSS_DISPLAY_GRID:
	അവസ്ഥ CSS_DISPLAY_FLEX:
	അവസ്ഥ CSS_DISPLAY_BLOCK:
		box->type = BOX_BLOCK;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		box->type = BOX_INLINE_BLOCK;
		നിർത്തുക;
	}

	inline_container = box_create(ശൂന്യ, 0, false, 0, 0, 0, 0, html->bctx);
	എങ്കിൽ (!inline_container)
		തിരികെ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ശൂന്യ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	എങ്കിൽ (!inline_box)
		തിരികെ false;
	inline_box->type = BOX_TEXT;
	inline_box->text = talloc_strdup(html->bctx, "");

	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	തിരികെ box_textarea_create_textarea(html, box, കണ്ണി);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
സ്ഥിതം bool box_select_add_option(ഘടന form_control *control, dom_കണ്ണി *n)
{
	അക്ഷരം *മൂല്യം = ശൂന്യ;
	അക്ഷരം *text = ശൂന്യ;
	അക്ഷരം *text_nowrap = ശൂന്യ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_കണ്ണി_get_text_content(n, &content);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	എങ്കിൽ (content != ശൂന്യ) {
		text = squash_whitespace(dom_string_data(content));
		dom_string_unref(content);
	} അല്ലെങ്കിൽ {
		text = strdup("");
	}

	എങ്കിൽ (text == ശൂന്യ)
		പോകുക no_memory;

	err = dom_element_get_attribute(n, corestring_dom_മൂല്യം, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		മൂല്യം = strdup(dom_string_data(s));
		dom_string_unref(s);
	} അല്ലെങ്കിൽ {
		മൂല്യം = strdup(text);
	}

	എങ്കിൽ (മൂല്യം == ശൂന്യ)
		പോകുക no_memory;

	എങ്കിൽ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	text_nowrap = cnv_space2nbsp(text);
	എങ്കിൽ (text_nowrap == ശൂന്യ)
		പോകുക no_memory;

	എങ്കിൽ (form_add_option(control, മൂല്യം, text_nowrap, selected, n) == false)
		പോകുക no_memory;

	സ്വതന്ത്രം(text);

	തിരികെ true;

no_memory:
	സ്വതന്ത്രം(മൂല്യം);
	സ്വതന്ത്രം(text);
	സ്വതന്ത്രം(text_nowrap);
	തിരികെ false;
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
സ്ഥിതം bool
box_a(dom_കണ്ണി *n,
      html_content *content,
      ഘടന box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		ok = box_extract_link(content, s, content->base_url, &url);
		dom_string_unref(s);
		എങ്കിൽ (!ok)
			തിരികെ false;
		എങ്കിൽ (url) {
			എങ്കിൽ (box->href != ശൂന്യ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		എങ്കിൽ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_target, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->target = "_blank";
		അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_മുകൾ_2))
			box->target = "_top";
		അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__parent))
			box->target = "_parent";
		അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->target = "_self";
		അല്ലെങ്കിൽ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->target = talloc_strdup(content->bctx,
					dom_string_data(s));
			എങ്കിൽ (!box->target) {
				dom_string_unref(s);
				തിരികെ false;
			}
		}
		dom_string_unref(s);
	}

	തിരികെ true;
}


/**
 * Document body special element handler [7.5.1].
 */
സ്ഥിതം bool
box_body(dom_കണ്ണി *n,
	 html_content *content,
	 ഘടന box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	എങ്കിൽ (nscss_color_is_transparent(color)) {
		content->background_colour = NS_TRANSPARENT;
	} അല്ലെങ്കിൽ {
		content->background_colour = nscss_color_to_ns(color);
	}

	തിരികെ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
സ്ഥിതം bool
box_br(dom_കണ്ണി *n,
       html_content *content,
       ഘടന box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	തിരികെ true;
}


/**
 * special element handler for Push button [17.5].
 */
സ്ഥിതം bool
box_button(dom_കണ്ണി *n,
	   html_content *content,
	   ഘടന box *box,
	   bool *convert_children)
{
	ഘടന form_control *gadget;

	gadget = html_forms_get_control_for_കണ്ണി(content->forms, n);
	എങ്കിൽ (!gadget)
		തിരികെ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_BLOCK;

	/* Just render the contents */

	തിരികെ true;
}


/**
 * Canvas element
 */
സ്ഥിതം bool
box_canvas(dom_കണ്ണി *n,
	     html_content *content,
	     ഘടന box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	എങ്കിൽ (!content->enable_scripting) {
		തിരികെ true;
	}
	*convert_children = false;

	എങ്കിൽ (box->style && ns_computed_display(box->style,
			box_is_വേര്(n)) == CSS_DISPLAY_NONE)
		തിരികെ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	തിരികെ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
സ്ഥിതം bool
box_embed(dom_കണ്ണി *n,
	  html_content *content,
	  ഘടന box *box,
	  bool *convert_children)
{
	ഘടന object_params *params;
	ഘടന object_param *param;
	dom_namednodemap *attrs;
	unsigned long idx;
	uint32_t അക്കം_attrs;
	dom_string *src;
	dom_exception err;

	എങ്കിൽ (box->style && ns_computed_display(box->style,
			box_is_വേര്(n)) == CSS_DISPLAY_NONE)
		തിരികെ true;

	params = talloc(content->bctx, ഘടന object_params);
	എങ്കിൽ (params == ശൂന്യ)
		തിരികെ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ശൂന്യ;
	params->type = ശൂന്യ;
	params->codetype = ശൂന്യ;
	params->codebase = ശൂന്യ;
	params->classid = ശൂന്യ;
	params->params = ശൂന്യ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	എങ്കിൽ (err != DOM_NO_ERR || src == ശൂന്യ)
		തിരികെ true;
	എങ്കിൽ (box_extract_link(content, src, content->base_url,
			     &params->data) == false) {
		dom_string_unref(src);
		തിരികെ false;
	}

	dom_string_unref(src);

	എങ്കിൽ (params->data == ശൂന്യ)
		തിരികെ true;

	/* Don't include ourself */
	എങ്കിൽ (nsurl_compare(content->base_url, params->data, NSURL_COMPLETE))
		തിരികെ true;

	/* add attributes as parameters to linked list */
	err = dom_കണ്ണി_get_attributes(n, &attrs);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	err = dom_namednodemap_get_length(attrs, &അക്കം_attrs);
	എങ്കിൽ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		തിരികെ false;
	}

	വേണ്ടി (idx = 0; idx < അക്കം_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *മൂല്യം;

		err = dom_namednodemap_item(attrs, idx, (ശൂന്യം *) &attr);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			തിരികെ false;
		}

		err = dom_attr_get_name(attr, &name);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(attr);
			dom_namednodemap_unref(attrs);
			തിരികെ false;
		}

		എങ്കിൽ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_കണ്ണി_unref(attr);
			dom_string_unref(name);
			തുടരുക;
		}

		err = dom_attr_get_മൂല്യം(attr, &മൂല്യം);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			തിരികെ false;
		}

		param = talloc(content->bctx, ഘടന object_param);
		എങ്കിൽ (param == ശൂന്യ) {
			dom_കണ്ണി_unref(attr);
			dom_string_unref(മൂല്യം);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			തിരികെ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_data(name));
		param->മൂല്യം = talloc_strdup(content->bctx, dom_string_data(മൂല്യം));
		param->type = ശൂന്യ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->next = ശൂന്യ;

		dom_string_unref(മൂല്യം);
		dom_string_unref(name);
		dom_കണ്ണി_unref(attr);

		എങ്കിൽ (param->name == ശൂന്യ || param->മൂല്യം == ശൂന്യ ||
				param->valuetype == ശൂന്യ) {
			dom_namednodemap_unref(attrs);
			തിരികെ false;
		}

		param->next = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	തിരികെ html_fetch_object(content, params->data, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
സ്ഥിതം bool
box_frameset(dom_കണ്ണി *n,
	     html_content *content,
	     ഘടന box *box,
	     bool *convert_children)
{
	bool ok;

	എങ്കിൽ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		എങ്കിൽ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		തിരികെ true;
	}

	content->frameset = talloc_zero(content->bctx,
					ഘടന content_html_frames);
	എങ്കിൽ (!content->frameset) {
		തിരികെ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	എങ്കിൽ (ok) {
		box->type = BOX_NONE;
	}

	എങ്കിൽ (convert_children) {
		*convert_children = false;
	}
	തിരികെ ok;
}


/**
 * Inline subwindow [16.5].
 */
സ്ഥിതം bool
box_iframe(dom_കണ്ണി *n,
	   html_content *content,
	   ഘടന box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ഘടന content_html_iframe *iframe;
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (box->style && ns_computed_display(box->style,
			box_is_വേര്(n)) == CSS_DISPLAY_NONE)
		തിരികെ true;

	എങ്കിൽ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		തിരികെ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	എങ്കിൽ (err != DOM_NO_ERR || s == ശൂന്യ)
		തിരികെ true;
	എങ്കിൽ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		തിരികെ false;
	}
	dom_string_unref(s);
	എങ്കിൽ (url == ശൂന്യ)
		തിരികെ true;

	/* don't include ourself */
	എങ്കിൽ (nsurl_compare(content->base_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		തിരികെ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ഘടന content_html_iframe);
	എങ്കിൽ (iframe == ശൂന്യ) {
		nsurl_unref(url);
		തിരികെ false;
	}

	talloc_set_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_width = 0;
	iframe->margin_height = 0;
	iframe->name = ശൂന്യ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->next = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		i = atoi(dom_string_data(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		css_color color;

		എങ്കിൽ (nscss_parse_colour(dom_string_data(s), &color))
			iframe->border_colour = nscss_color_to_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		iframe->margin_width = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		iframe->margin_height = atoi(dom_string_data(s));
		dom_string_unref(s);
	}

	/* box */
	പ്രതിജ്ഞാ(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	എങ്കിൽ (convert_children)
		*convert_children = false;
	തിരികെ true;
}


/**
 * Embedded image [13.2].
 */
സ്ഥിതം bool
box_image(dom_കണ്ണി *n,
	  html_content *content,
	  ഘടന box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	എണ്ണൽ css_width_e wtype;
	എണ്ണൽ css_height_e htype;
	css_fixed മൂല്യം = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	എങ്കിൽ (box->style && ns_computed_display(box->style,
			box_is_വേര്(n)) == CSS_DISPLAY_NONE)
		തിരികെ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
		അക്ഷരം *alt = squash_whitespace(dom_string_data(s));
		dom_string_unref(s);
		എങ്കിൽ (alt == ശൂന്യ)
			തിരികെ false;
		box->text = talloc_strdup(content->bctx, alt);
		സ്വതന്ത്രം(alt);
		എങ്കിൽ (box->text == ശൂന്യ)
			തിരികെ false;
		box->length = strlen(box->text);
	}

	എങ്കിൽ (nsoption_bool(foreground_images) == false) {
		തിരികെ true;
	}

	/* imagemap associated with this image */
	എങ്കിൽ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		തിരികെ false;
	എങ്കിൽ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	എങ്കിൽ (err != DOM_NO_ERR || s == ശൂന്യ)
		തിരികെ true;

	എങ്കിൽ (box_extract_link(content, s, content->base_url, &url) == false) {
		dom_string_unref(s);
		തിരികെ false;
	}

	dom_string_unref(s);

	എങ്കിൽ (url == ശൂന്യ)
		തിരികെ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_width(box->style, &മൂല്യം, &wunit);
	htype = css_computed_height(box->style, &മൂല്യം, &hunit);

	എങ്കിൽ (wtype == CSS_WIDTH_SET &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_HEIGHT_SET &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	തിരികെ ok;
}


/**
 * Form control [17.4].
 */
സ്ഥിതം bool
box_നിവേശം(dom_കണ്ണി *n,
	  html_content *content,
	  ഘടന box *box,
	  bool *convert_children)
{
	ഘടന form_control *gadget;
	dom_string *type = ശൂന്യ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_കണ്ണി(content->forms, n);
	എങ്കിൽ (gadget == ശൂന്യ) {
		തിരികെ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	എങ്കിൽ ((err != DOM_NO_ERR) || (type == ശൂന്യ)) {
		/* no type so "text" is assumed */
		എങ്കിൽ (box_നിവേശം_text(content, box, n) == false) {
			തിരികെ false;
		}
		*convert_children = false;
		തിരികെ true;
	}

	എങ്കിൽ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		എങ്കിൽ (box_നിവേശം_text(content, box, n) == false)
			പോകുക no_memory;

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_BLOCK;

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} അല്ലെങ്കിൽ എങ്കിൽ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ഘടന box *inline_container, *inline_box;

		എങ്കിൽ (box_button(n, content, box, 0) == false)
			പോകുക no_memory;

		inline_container = box_create(ശൂന്യ, 0, false, 0, 0, 0, 0,
				content->bctx);
		എങ്കിൽ (inline_container == ശൂന്യ)
			പോകുക no_memory;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ശൂന്യ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		എങ്കിൽ (inline_box == ശൂന്യ)
			പോകുക no_memory;

		inline_box->type = BOX_TEXT;

		എങ്കിൽ (box->gadget->മൂല്യം != ശൂന്യ)
			inline_box->text = talloc_strdup(content->bctx,
					box->gadget->മൂല്യം);
		അല്ലെങ്കിൽ എങ്കിൽ (box->gadget->type == GADGET_SUBMIT)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		അല്ലെങ്കിൽ എങ്കിൽ (box->gadget->type == GADGET_RESET)
			inline_box->text = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		അല്ലെങ്കിൽ
			inline_box->text = talloc_strdup(content->bctx,
							 "Button");

		എങ്കിൽ (inline_box->text == ശൂന്യ)
			പോകുക no_memory;

		inline_box->length = strlen(inline_box->text);

		box_add_child(inline_container, inline_box);

		box_add_child(box, inline_container);

	} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		എങ്കിൽ (box->style && ns_computed_display(box->style,
				box_is_വേര്(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			എങ്കിൽ (err == DOM_NO_ERR && s != ശൂന്യ) {
				error = nsurl_join(content->base_url,
						dom_string_data(s), &url);
				dom_string_unref(s);
				എങ്കിൽ (error != NSERROR_OK)
					പോകുക no_memory;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				എങ്കിൽ (nsurl_compare(url, content->base_url,
						NSURL_COMPLETE) == false) {
					എങ്കിൽ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						പോകുക no_memory;
					}
				}
				nsurl_unref(url);
			}
		}
	} അല്ലെങ്കിൽ {
		/* unhandled type the default is "text" */
		എങ്കിൽ (box_നിവേശം_text(content, box, n) == false)
			പോകുക no_memory;
	}

	dom_string_unref(type);

	*convert_children = false;

	തിരികെ true;

no_memory:
	dom_string_unref(type);

	തിരികെ false;
}


/**
 * Noscript element
 */
സ്ഥിതം bool
box_noscript(dom_കണ്ണി *n,
	     html_content *content,
	     ഘടന box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	എങ്കിൽ (content->enable_scripting) {
		*convert_children = false;
	}

	തിരികെ true;
}


/**
 * Generic embedded object [13.3].
 */
സ്ഥിതം bool
box_object(dom_കണ്ണി *n,
	   html_content *content,
	   ഘടന box *box,
	   bool *convert_children)
{
	ഘടന object_params *params;
	ഘടന object_param *param;
	dom_string *codebase, *classid, *data;
	dom_കണ്ണി *c;
	dom_exception err;

	എങ്കിൽ (box->style && ns_computed_display(box->style,
			box_is_വേര്(n)) == CSS_DISPLAY_NONE)
		തിരികെ true;

	എങ്കിൽ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		തിരികെ false;
	എങ്കിൽ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ഘടന object_params);
	എങ്കിൽ (params == ശൂന്യ)
		തിരികെ false;

	talloc_set_destructor(params, box_object_talloc_destructor);

	params->data = ശൂന്യ;
	params->type = ശൂന്യ;
	params->codetype = ശൂന്യ;
	params->codebase = ശൂന്യ;
	params->classid = ശൂന്യ;
	params->params = ശൂന്യ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	എങ്കിൽ (err == DOM_NO_ERR && codebase != ശൂന്യ) {
		എങ്കിൽ (box_extract_link(content, codebase,	content->base_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			തിരികെ false;
		}
		dom_string_unref(codebase);
	}
	എങ്കിൽ (params->codebase == ശൂന്യ)
		params->codebase = nsurl_ref(content->base_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	എങ്കിൽ (err == DOM_NO_ERR && classid != ശൂന്യ) {
		എങ്കിൽ (box_extract_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			തിരികെ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_data, &data);
	എങ്കിൽ (err == DOM_NO_ERR && data != ശൂന്യ) {
		എങ്കിൽ (box_extract_link(content, data,
				params->codebase, &params->data) == false) {
			dom_string_unref(data);
			തിരികെ false;
		}
		dom_string_unref(data);
	}

	എങ്കിൽ (params->classid == ശൂന്യ && params->data == ശൂന്യ)
		/* nothing to embed; ignore */
		തിരികെ true;

	/* Don't include ourself */
	എങ്കിൽ (params->classid != ശൂന്യ && nsurl_compare(content->base_url,
			params->classid, NSURL_COMPLETE))
		തിരികെ true;

	എങ്കിൽ (params->data != ശൂന്യ && nsurl_compare(content->base_url,
			params->data, NSURL_COMPLETE))
		തിരികെ true;

	/* codetype and type are MIME types */
	എങ്കിൽ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		തിരികെ false;
	എങ്കിൽ (box_get_attribute(n, "type", params, &params->type) == false)
		തിരികെ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	എങ്കിൽ (params->classid != ശൂന്യ && params->data == ശൂന്യ &&
			params->codetype != ശൂന്യ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				strlen(params->codetype), &icodetype);
		എങ്കിൽ (lerror != lwc_error_ok)
			തിരികെ false;

		എങ്കിൽ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			തിരികെ true;
		}

		lwc_string_unref(icodetype);
	}

	എങ്കിൽ (params->data != ശൂന്യ && params->type != ശൂന്യ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, strlen(params->type),
				&itype);
		എങ്കിൽ (lerror != lwc_error_ok)
			തിരികെ false;

		എങ്കിൽ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			തിരികെ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_കണ്ണി_get_first_child(n, &c);
	എങ്കിൽ (err != DOM_NO_ERR)
		തിരികെ false;

	ആകുമ്പോൾ (c != ശൂന്യ) {
		dom_കണ്ണി *next;
		dom_കണ്ണി_type type;

		err = dom_കണ്ണി_get_കണ്ണി_type(c, &type);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(c);
			തിരികെ false;
		}

		എങ്കിൽ (type == DOM_ELEMENT_കണ്ണി) {
			dom_string *name;

			err = dom_കണ്ണി_get_കണ്ണി_name(c, &name);
			എങ്കിൽ (err != DOM_NO_ERR) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_കണ്ണി_unref(c);
				നിർത്തുക;
			}
			dom_string_unref(name);

			param = talloc(params, ഘടന object_param);
			എങ്കിൽ (param == ശൂന്യ) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}
			param->name = ശൂന്യ;
			param->മൂല്യം = ശൂന്യ;
			param->type = ശൂന്യ;
			param->valuetype = ശൂന്യ;
			param->next = ശൂന്യ;

			എങ്കിൽ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (box_get_attribute(c, "value", param,
					&param->മൂല്യം) == false) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_കണ്ണി_unref(c);
				തിരികെ false;
			}

			എങ്കിൽ (param->valuetype == ശൂന്യ) {
				param->valuetype = talloc_strdup(param, "data");
				എങ്കിൽ (param->valuetype == ശൂന്യ) {
					dom_കണ്ണി_unref(c);
					തിരികെ false;
				}
			}

			param->next = params->params;
			params->params = param;
		}

		err = dom_കണ്ണി_get_next_sibling(c, &next);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(c);
			തിരികെ false;
		}

		dom_കണ്ണി_unref(c);
		c = next;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	എങ്കിൽ (!html_fetch_object(content,
			       params->data ? params->data : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		തിരികെ false;

	*convert_children = false;
	തിരികെ true;
}


/**
 * Preformatted text [9.3.4].
 */
സ്ഥിതം bool
box_pre(dom_കണ്ണി *n,
	html_content *content,
	ഘടന box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	തിരികെ true;
}


/**
 * Option selector [17.6].
 */
സ്ഥിതം bool
box_select(dom_കണ്ണി *n,
	   html_content *content,
	   ഘടന box *box,
	   bool *convert_children)
{
	ഘടന box *inline_container;
	ഘടന box *inline_box;
	ഘടന form_control *gadget;
	dom_കണ്ണി *c, *c2;
	dom_കണ്ണി *next, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_കണ്ണി(content->forms, n);
	എങ്കിൽ (gadget == ശൂന്യ)
		തിരികെ false;

	gadget->html = content;
	err = dom_കണ്ണി_get_first_child(n, &c);
	എങ്കിൽ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		തിരികെ false;
	}

	ആകുമ്പോൾ (c != ശൂന്യ) {
		dom_string *name;

		err = dom_കണ്ണി_get_കണ്ണി_name(c, &name);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(c);
			form_free_control(gadget);
			തിരികെ false;
		}

		എങ്കിൽ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			എങ്കിൽ (box_select_add_option(gadget, c) == false) {
				dom_കണ്ണി_unref(c);
				form_free_control(gadget);
				തിരികെ false;
			}
		} അല്ലെങ്കിൽ എങ്കിൽ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_കണ്ണി_get_first_child(c, &c2);
			എങ്കിൽ (err != DOM_NO_ERR) {
				dom_കണ്ണി_unref(c);
				form_free_control(gadget);
				തിരികെ false;
			}

			ആകുമ്പോൾ (c2 != ശൂന്യ) {
				dom_string *c2_name;

				err = dom_കണ്ണി_get_കണ്ണി_name(c2, &c2_name);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(c2);
					dom_കണ്ണി_unref(c);
					form_free_control(gadget);
					തിരികെ false;
				}

				എങ്കിൽ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					എങ്കിൽ (box_select_add_option(gadget,
							c2) == false) {
						dom_കണ്ണി_unref(c2);
						dom_കണ്ണി_unref(c);
						form_free_control(gadget);
						തിരികെ false;
					}
				} അല്ലെങ്കിൽ {
					dom_string_unref(c2_name);
				}

				err = dom_കണ്ണി_get_next_sibling(c2, &next2);
				എങ്കിൽ (err != DOM_NO_ERR) {
					dom_കണ്ണി_unref(c2);
					dom_കണ്ണി_unref(c);
					form_free_control(gadget);
					തിരികെ false;
				}

				dom_കണ്ണി_unref(c2);
				c2 = next2;
			}
		} അല്ലെങ്കിൽ {
			dom_string_unref(name);
		}

		err = dom_കണ്ണി_get_next_sibling(c, &next);
		എങ്കിൽ (err != DOM_NO_ERR) {
			dom_കണ്ണി_unref(c);
			form_free_control(gadget);
			തിരികെ false;
		}

		dom_കണ്ണി_unref(c);
		c = next;
	}

	എങ്കിൽ (gadget->data.select.അക്കം_ഇനങ്ങൾ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		തിരികെ true;
	}

	box->type = BOX_INLINE_BLOCK;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ശൂന്യ, 0, false, 0, 0, 0, 0, content->bctx);
	എങ്കിൽ (inline_container == ശൂന്യ)
		പോകുക no_memory;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ശൂന്യ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	എങ്കിൽ (inline_box == ശൂന്യ)
		പോകുക no_memory;
	inline_box->type = BOX_TEXT;
	box_add_child(inline_container, inline_box);
	box_add_child(box, inline_container);

	എങ്കിൽ (gadget->data.select.multiple == false &&
			gadget->data.select.അക്കം_selected == 0) {
		gadget->data.select.current = gadget->data.select.ഇനങ്ങൾ;
		gadget->data.select.current->initial_selected =
			gadget->data.select.current->selected = true;
		gadget->data.select.അക്കം_selected = 1;
		dom_html_option_element_set_selected(
				gadget->data.select.current->കണ്ണി, true);
	}

	എങ്കിൽ (gadget->data.select.അക്കം_selected == 0)
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	അല്ലെങ്കിൽ എങ്കിൽ (gadget->data.select.അക്കം_selected == 1)
		inline_box->text = talloc_strdup(content->bctx,
				gadget->data.select.current->text);
	അല്ലെങ്കിൽ
		inline_box->text = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	എങ്കിൽ (inline_box->text == ശൂന്യ)
		പോകുക no_memory;

	inline_box->length = strlen(inline_box->text);

	*convert_children = false;
	തിരികെ true;

no_memory:
	തിരികെ false;
}


/**
 * Multi-line text field [17.7].
 */
സ്ഥിതം bool box_textarea(dom_കണ്ണി *n,
			html_content *content,
			ഘടന box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_കണ്ണി(content->forms, n);
	എങ്കിൽ (box->gadget == ശൂന്യ)
		തിരികെ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	എങ്കിൽ (!box_നിവേശം_text(content, box, n))
		തിരികെ false;

	*convert_children = false;
	തിരികെ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_കണ്ണി *കണ്ണി,
			 html_content *content,
			 ഘടന box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type tag_type;
	bool res;

	exc = dom_html_element_get_tag_type(കണ്ണി, &tag_type);
	എങ്കിൽ (exc != DOM_NO_ERR) {
		tag_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	തിരഞ്ഞെടുപ്പ് (tag_type) {
	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_നിവേശം:
		res = box_നിവേശം(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	അവസ്ഥ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(കണ്ണി, content, box, convert_children);
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		res = true;
	}

	തിരികെ res;
}
