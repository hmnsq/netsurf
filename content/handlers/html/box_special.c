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


ساکن ثابت content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ساکن اندرونی bool box_is_جڑ(dom_گرہ *n)
{
	dom_گرہ *والد;
	dom_گرہ_type type;
	dom_exception err;

	err = dom_گرہ_get_والد_گرہ(n, &والد);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (والد != تہی) {
		err = dom_گرہ_get_گرہ_type(والد, &type);

		dom_گرہ_unref(والد);

		اگر (err != DOM_NO_ERR)
			واپس false;

		اگر (type != DOM_DOCUMENT_گرہ)
			واپس false;
	}

	واپس true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساکن صحیح box_object_talloc_destructor(ساخت object_params *o)
{
	اگر (o->codebase != تہی)
		nsurl_unref(o->codebase);
	اگر (o->classid != تہی)
		nsurl_unref(o->classid);
	اگر (o->ڈیٹا != تہی)
		nsurl_unref(o->ڈیٹا);

	واپس 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ساکن ساخت چوکھٹا_dimension *
box_parse_multi_lengths(ثابت dom_string *ds, غیرعلامتی صحیح *گنتی)
{
	حرف *انتہا;
	غیرعلامتی صحیح i, n;
	ساخت چوکھٹا_dimension *لمبائی;
	ثابت حرف *s;

	s = dom_string_ڈیٹا(ds);

	برائے (i = 0, n = 1; s[i]; i++)
		اگر (s[i] == ',')
			n++;

	لمبائی = صفر_حافظہ_دو(n, حجم(ساخت چوکھٹا_dimension));
	اگر (!لمبائی)
		واپس تہی;

	برائے (i = 0; i != n; i++) {
		جب (ascii_is_space(*s)) {
			s++;
		}
		لمبائی[i].قدر = strtof(s, &انتہا);
		اگر (لمبائی[i].قدر <= 0) {
			لمبائی[i].قدر = 1;
		}
		s = انتہا;
		انتخاب (*s) {
			حالت '%':
				لمبائی[i].unit = چوکھٹا_DIMENSION_PERCENT;
				توڑو;
			حالت '*':
				لمبائی[i].unit = چوکھٹا_DIMENSION_RELATIVE;
				توڑو;
			طےشدہ:
				لمبائی[i].unit = چوکھٹا_DIMENSION_PIXELS;
				توڑو;
		}
		جب (*s && *s != ',') {
			s++;
		}
		اگر (*s == ',') {
			s++;
		}
	}

	*گنتی = n;
	واپس لمبائی;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساکن صحیح box_چوکھٹے_talloc_destructor(ساخت content_html_چوکھٹے *f)
{
	اگر (f->url != تہی) {
		nsurl_unref(f->url);
		f->url = تہی;
	}

	واپس 0;
}


/**
 * create a frameset box tree
 */
ساکن bool
box_create_frameset(ساخت content_html_چوکھٹے *f,
		    dom_گرہ *n,
		    html_content *content)
{
	غیرعلامتی صحیح صف, ستون, index, i;
	غیرعلامتی صحیح صفیں = 1, ستون_تعداد = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ساخت چوکھٹا_dimension *صف_اونچائی = 0, *ستون_چوڑائی = 0;
	dom_گرہ *c, *اگلا;
	ساخت content_html_چوکھٹے *چوکھٹا;
	bool default_border = true;
	رنگ default_border_رنگ = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_صفیں, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		صف_اونچائی = box_parse_multi_lengths(s, &صفیں);
		dom_string_unref(s);
		اگر (صف_اونچائی == تہی)
			واپس false;
	} ورنہ {
		صف_اونچائی = صفر_حافظہ_دو(1, حجم(ساخت چوکھٹا_dimension));
		اگر (صف_اونچائی == تہی)
			واپس false;
		صف_اونچائی->قدر = 100;
		صف_اونچائی->unit = چوکھٹا_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_ستون_تعداد, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		ستون_چوڑائی = box_parse_multi_lengths(s, &ستون_تعداد);
		dom_string_unref(s);
		اگر (ستون_چوڑائی == تہی) {
			آزاد(صف_اونچائی);
			واپس false;
		}
	} ورنہ {
		ستون_چوڑائی = صفر_حافظہ_دو(1, حجم(ساخت چوکھٹا_dimension));
		اگر (ستون_چوڑائی == تہی) {
			آزاد(صف_اونچائی);
			واپس false;
		}
		ستون_چوڑائی->قدر = 100;
		ستون_چوڑائی->unit = چوکھٹا_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		اگر ((dom_string_ڈیٹا(s)[0] == '0') &&
				(dom_string_ڈیٹا(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		css_color color;

		اگر (nscss_parse_رنگ(dom_string_ڈیٹا(s), &color))
			default_border_رنگ = nscss_color_مقصد_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->ستون_تعداد = ستون_تعداد;
	f->صفیں = صفیں;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ساخت content_html_چوکھٹے,
								(صفیں * ستون_تعداد));

	talloc_سیٹ_destructor(f->children, box_چوکھٹے_talloc_destructor);

	برائے (صف = 0; صف < صفیں; صف++) {
		برائے (ستون = 0; ستون < ستون_تعداد; ستون++) {
			index = (صف * ستون_تعداد) + ستون;
			چوکھٹا = &f->children[index];
			چوکھٹا->ستون_تعداد = 0;
			چوکھٹا->صفیں = 0;
			چوکھٹا->چوڑائی = ستون_چوڑائی[ستون];
			چوکھٹا->اونچائی = صف_اونچائی[صف];
			چوکھٹا->margin_چوڑائی = 0;
			چوکھٹا->margin_اونچائی = 0;
			چوکھٹا->name = تہی;
			چوکھٹا->url = تہی;
			چوکھٹا->no_resize = false;
			چوکھٹا->scrolling = BW_SCROLLING_AUTO;
			چوکھٹا->border = default_border;
			چوکھٹا->border_رنگ = default_border_رنگ;
			چوکھٹا->children = تہی;
		}
	}
	آزاد(ستون_چوڑائی);
	آزاد(صف_اونچائی);

	/* create the frameset windows */
	err = dom_گرہ_get_پہلا_بچہ(n, &c);
	اگر (err != DOM_NO_ERR)
		واپس false;

	برائے (صف = 0; c != تہی && صف < صفیں; صف++) {
		برائے (ستون = 0; c != تہی && ستون < ستون_تعداد; ستون++) {
			جب (c != تہی) {
				dom_گرہ_type type;
				dom_string *name;

				err = dom_گرہ_get_گرہ_type(c, &type);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(c);
					واپس false;
				}

				err = dom_گرہ_get_گرہ_name(c, &name);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(c);
					واپس false;
				}

				اگر (type != DOM_ELEMENT_گرہ ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_چوکھٹا) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_گرہ_get_اگلا_sibling(c,
							&اگلا);
					اگر (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_گرہ_unref(c);
						واپس false;
					}

					dom_string_unref(name);
					dom_گرہ_unref(c);
					c = اگلا;
				} ورنہ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					توڑو;
				}
			}

			اگر (c == تہی)
				توڑو;

			/* get current frame */
			index = (صف * ستون_تعداد) + ستون;
			چوکھٹا = &f->children[index];

			/* nest framesets */
			err = dom_گرہ_get_گرہ_name(c, &s);
			اگر (err != DOM_NO_ERR) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				چوکھٹا->border = 0;
				اگر (box_create_frameset(چوکھٹا, c,
						content) == false) {
					dom_گرہ_unref(c);
					واپس false;
				}

				err = dom_گرہ_get_اگلا_sibling(c, &اگلا);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(c);
					واپس false;
				}

				dom_گرہ_unref(c);
				c = اگلا;
				جاری;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = تہی;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				box_نکالو_link(content, s, content->بنیاد_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			اگر (url != تہی) {
				/* no self-references */
				اگر (nsurl_compare(content->بنیاد_url, url,
						NSURL_COMPLETE) == false)
					چوکھٹا->url = url;
				url = تہی;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				چوکھٹا->name = talloc_strdup(content->bctx,
						dom_string_ڈیٹا(s));
				dom_string_unref(s);
			}

			اگر (dom_element_has_attribute(c, corestring_dom_noresize,
						      &چوکھٹا->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				چوکھٹا->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				i = atoi(dom_string_ڈیٹا(s));
				چوکھٹا->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				اگر (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					چوکھٹا->scrolling = BW_SCROLLING_YES;
				ورنہ اگر (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					چوکھٹا->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				چوکھٹا->margin_چوڑائی = atoi(dom_string_ڈیٹا(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				چوکھٹا->margin_اونچائی = atoi(dom_string_ڈیٹا(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				css_color color;

				اگر (nscss_parse_رنگ(dom_string_ڈیٹا(s),
						&color))
					چوکھٹا->border_رنگ =
						nscss_color_مقصد_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_گرہ_get_اگلا_sibling(c, &اگلا);
			اگر (err != DOM_NO_ERR) {
				dom_گرہ_unref(c);
				واپس false;
			}

			dom_گرہ_unref(c);
			c = اگلا;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	اگر (c != تہی) {
		dom_گرہ_unref(c);
	}

	واپس true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساکن صحیح box_iframes_talloc_destructor(ساخت content_html_iframe *f)
{
	اگر (f->url != تہی) {
		nsurl_unref(f->url);
		f->url = تہی;
	}

	واپس 0;
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
ساکن bool
box_get_attribute(dom_گرہ *n,
		  ثابت حرف *attribute,
		  خالی *context,
		  حرف **قدر)
{
	حرف *نتیجہ;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ثابت uint8_t *) attribute,
			لڑی_لمبائی(attribute), &attr_name);
	اگر (err != DOM_NO_ERR)
		واپس false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	اگر (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		واپس false;
	}

	dom_string_unref(attr_name);

	اگر (attr != تہی) {
		نتیجہ = talloc_strdup(context, dom_string_ڈیٹا(attr));

		dom_string_unref(attr);

		اگر (نتیجہ == تہی)
			واپس false;

		*قدر = نتیجہ;
	}

	واپس true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ساکن bool
box_داخلہ_متن(html_content *html, ساخت box *box, ساخت dom_گرہ *گرہ)
{
	ساخت box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	انتخاب (display) {
	حالت CSS_DISPLAY_GRID:
	حالت CSS_DISPLAY_FLEX:
	حالت CSS_DISPLAY_بلاک:
		box->type = BOX_بلاک;
		توڑو;
	طےشدہ:
		box->type = BOX_INLINE_بلاک;
		توڑو;
	}

	inline_container = box_create(تہی, 0, false, 0, 0, 0, 0, html->bctx);
	اگر (!inline_container)
		واپس false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(تہی, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	اگر (!inline_box)
		واپس false;
	inline_box->type = BOX_متن;
	inline_box->متن = talloc_strdup(html->bctx, "");

	box_جوڑو_بچہ(inline_container, inline_box);
	box_جوڑو_بچہ(box, inline_container);

	واپس box_textarea_create_textarea(html, box, گرہ);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ساکن bool box_select_جوڑو_option(ساخت form_control *control, dom_گرہ *n)
{
	حرف *قدر = تہی;
	حرف *متن = تہی;
	حرف *متن_nowrap = تہی;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_گرہ_get_متن_content(n, &content);
	اگر (err != DOM_NO_ERR)
		واپس false;

	اگر (content != تہی) {
		متن = squash_whitespace(dom_string_ڈیٹا(content));
		dom_string_unref(content);
	} ورنہ {
		متن = strdup("");
	}

	اگر (متن == تہی)
		جاؤ no_حافظہ;

	err = dom_element_get_attribute(n, corestring_dom_قدر, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		قدر = strdup(dom_string_ڈیٹا(s));
		dom_string_unref(s);
	} ورنہ {
		قدر = strdup(متن);
	}

	اگر (قدر == تہی)
		جاؤ no_حافظہ;

	اگر (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	متن_nowrap = cnv_space2nbsp(متن);
	اگر (متن_nowrap == تہی)
		جاؤ no_حافظہ;

	اگر (form_جوڑو_option(control, قدر, متن_nowrap, selected, n) == false)
		جاؤ no_حافظہ;

	آزاد(متن);

	واپس true;

no_حافظہ:
	آزاد(قدر);
	آزاد(متن);
	آزاد(متن_nowrap);
	واپس false;
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
ساکن bool
box_a(dom_گرہ *n,
      html_content *content,
      ساخت box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		ok = box_نکالو_link(content, s, content->بنیاد_url, &url);
		dom_string_unref(s);
		اگر (!ok)
			واپس false;
		اگر (url) {
			اگر (box->href != تہی)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
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
	err = dom_element_get_attribute(n, corestring_dom_ہدف, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->ہدف = "_blank";
		ورنہ اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_چوٹی_2))
			box->ہدف = "_top";
		ورنہ اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_والد))
			box->ہدف = "_parent";
		ورنہ اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->ہدف = "_self";
		ورنہ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->ہدف = talloc_strdup(content->bctx,
					dom_string_ڈیٹا(s));
			اگر (!box->ہدف) {
				dom_string_unref(s);
				واپس false;
			}
		}
		dom_string_unref(s);
	}

	واپس true;
}


/**
 * Document body special element handler [7.5.1].
 */
ساکن bool
box_body(dom_گرہ *n,
	 html_content *content,
	 ساخت box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	اگر (nscss_color_is_transparent(color)) {
		content->background_رنگ = NS_TRANSPARENT;
	} ورنہ {
		content->background_رنگ = nscss_color_مقصد_ns(color);
	}

	واپس true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ساکن bool
box_br(dom_گرہ *n,
       html_content *content,
       ساخت box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	واپس true;
}


/**
 * special element handler for Push button [17.5].
 */
ساکن bool
box_button(dom_گرہ *n,
	   html_content *content,
	   ساخت box *box,
	   bool *convert_children)
{
	ساخت form_control *gadget;

	gadget = html_forms_get_control_for_گرہ(content->forms, n);
	اگر (!gadget)
		واپس false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_بلاک;

	/* Just render the contents */

	واپس true;
}


/**
 * Canvas element
 */
ساکن bool
box_canvas(dom_گرہ *n,
	     html_content *content,
	     ساخت box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	اگر (!content->enable_scripting) {
		واپس true;
	}
	*convert_children = false;

	اگر (box->style && ns_computed_display(box->style,
			box_is_جڑ(n)) == CSS_DISPLAY_NONE)
		واپس true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	واپس true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ساکن bool
box_embed(dom_گرہ *n,
	  html_content *content,
	  ساخت box *box,
	  bool *convert_children)
{
	ساخت object_params *params;
	ساخت object_param *param;
	dom_namednodemap *attrs;
	غیرعلامتی طویل idx;
	uint32_t عدد_attrs;
	dom_string *src;
	dom_exception err;

	اگر (box->style && ns_computed_display(box->style,
			box_is_جڑ(n)) == CSS_DISPLAY_NONE)
		واپس true;

	params = talloc(content->bctx, ساخت object_params);
	اگر (params == تہی)
		واپس false;

	talloc_سیٹ_destructor(params, box_object_talloc_destructor);

	params->ڈیٹا = تہی;
	params->type = تہی;
	params->codetype = تہی;
	params->codebase = تہی;
	params->classid = تہی;
	params->params = تہی;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	اگر (err != DOM_NO_ERR || src == تہی)
		واپس true;
	اگر (box_نکالو_link(content, src, content->بنیاد_url,
			     &params->ڈیٹا) == false) {
		dom_string_unref(src);
		واپس false;
	}

	dom_string_unref(src);

	اگر (params->ڈیٹا == تہی)
		واپس true;

	/* Don't include ourself */
	اگر (nsurl_compare(content->بنیاد_url, params->ڈیٹا, NSURL_COMPLETE))
		واپس true;

	/* add attributes as parameters to linked list */
	err = dom_گرہ_get_اوصاف(n, &attrs);
	اگر (err != DOM_NO_ERR)
		واپس false;

	err = dom_namednodemap_get_لمبائی(attrs, &عدد_attrs);
	اگر (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		واپس false;
	}

	برائے (idx = 0; idx < عدد_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *قدر;

		err = dom_namednodemap_شے(attrs, idx, (خالی *) &attr);
		اگر (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			واپس false;
		}

		err = dom_attr_get_name(attr, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(attr);
			dom_namednodemap_unref(attrs);
			واپس false;
		}

		اگر (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_گرہ_unref(attr);
			dom_string_unref(name);
			جاری;
		}

		err = dom_attr_get_قدر(attr, &قدر);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			واپس false;
		}

		param = talloc(content->bctx, ساخت object_param);
		اگر (param == تہی) {
			dom_گرہ_unref(attr);
			dom_string_unref(قدر);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			واپس false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_ڈیٹا(name));
		param->قدر = talloc_strdup(content->bctx, dom_string_ڈیٹا(قدر));
		param->type = تہی;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->اگلا = تہی;

		dom_string_unref(قدر);
		dom_string_unref(name);
		dom_گرہ_unref(attr);

		اگر (param->name == تہی || param->قدر == تہی ||
				param->valuetype == تہی) {
			dom_namednodemap_unref(attrs);
			واپس false;
		}

		param->اگلا = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	واپس html_fetch_object(content, params->ڈیٹا, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ساکن bool
box_frameset(dom_گرہ *n,
	     html_content *content,
	     ساخت box *box,
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
		واپس true;
	}

	content->frameset = talloc_zero(content->bctx,
					ساخت content_html_چوکھٹے);
	اگر (!content->frameset) {
		واپس false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	اگر (ok) {
		box->type = BOX_NONE;
	}

	اگر (convert_children) {
		*convert_children = false;
	}
	واپس ok;
}


/**
 * Inline subwindow [16.5].
 */
ساکن bool
box_iframe(dom_گرہ *n,
	   html_content *content,
	   ساخت box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ساخت content_html_iframe *iframe;
	صحیح i;

	اگر (box->style && ns_computed_display(box->style,
			box_is_جڑ(n)) == CSS_DISPLAY_NONE)
		واپس true;

	اگر (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		واپس true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	اگر (err != DOM_NO_ERR || s == تہی)
		واپس true;
	اگر (box_نکالو_link(content, s, content->بنیاد_url, &url) == false) {
		dom_string_unref(s);
		واپس false;
	}
	dom_string_unref(s);
	اگر (url == تہی)
		واپس true;

	/* don't include ourself */
	اگر (nsurl_compare(content->بنیاد_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		واپس true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ساخت content_html_iframe);
	اگر (iframe == تہی) {
		nsurl_unref(url);
		واپس false;
	}

	talloc_سیٹ_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_چوڑائی = 0;
	iframe->margin_اونچائی = 0;
	iframe->name = تہی;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->اگلا = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		iframe->name = talloc_strdup(content->bctx, dom_string_ڈیٹا(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		i = atoi(dom_string_ڈیٹا(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		css_color color;

		اگر (nscss_parse_رنگ(dom_string_ڈیٹا(s), &color))
			iframe->border_رنگ = nscss_color_مقصد_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ورنہ اگر (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		iframe->margin_چوڑائی = atoi(dom_string_ڈیٹا(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		iframe->margin_اونچائی = atoi(dom_string_ڈیٹا(s));
		dom_string_unref(s);
	}

	/* box */
	أكد(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	اگر (convert_children)
		*convert_children = false;
	واپس true;
}


/**
 * Embedded image [13.2].
 */
ساکن bool
box_image(dom_گرہ *n,
	  html_content *content,
	  ساخت box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	شمار css_چوڑائی_e wtype;
	شمار css_اونچائی_e htype;
	css_fixed قدر = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	اگر (box->style && ns_computed_display(box->style,
			box_is_جڑ(n)) == CSS_DISPLAY_NONE)
		واپس true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	اگر (err == DOM_NO_ERR && s != تہی) {
		حرف *alt = squash_whitespace(dom_string_ڈیٹا(s));
		dom_string_unref(s);
		اگر (alt == تہی)
			واپس false;
		box->متن = talloc_strdup(content->bctx, alt);
		آزاد(alt);
		اگر (box->متن == تہی)
			واپس false;
		box->لمبائی = لڑی_لمبائی(box->متن);
	}

	اگر (nsoption_bool(foreground_images) == false) {
		واپس true;
	}

	/* imagemap associated with this image */
	اگر (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		واپس false;
	اگر (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	اگر (err != DOM_NO_ERR || s == تہی)
		واپس true;

	اگر (box_نکالو_link(content, s, content->بنیاد_url, &url) == false) {
		dom_string_unref(s);
		واپس false;
	}

	dom_string_unref(s);

	اگر (url == تہی)
		واپس true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_چوڑائی(box->style, &قدر, &wunit);
	htype = css_computed_اونچائی(box->style, &قدر, &hunit);

	اگر (wtype == CSS_چوڑائی_سیٹ &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_اونچائی_سیٹ &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	واپس ok;
}


/**
 * Form control [17.4].
 */
ساکن bool
box_داخلہ(dom_گرہ *n,
	  html_content *content,
	  ساخت box *box,
	  bool *convert_children)
{
	ساخت form_control *gadget;
	dom_string *type = تہی;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_گرہ(content->forms, n);
	اگر (gadget == تہی) {
		واپس false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	اگر ((err != DOM_NO_ERR) || (type == تہی)) {
		/* no type so "text" is assumed */
		اگر (box_داخلہ_متن(content, box, n) == false) {
			واپس false;
		}
		*convert_children = false;
		واپس true;
	}

	اگر (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		اگر (box_داخلہ_متن(content, box, n) == false)
			جاؤ no_حافظہ;

	} ورنہ اگر (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_بلاک;

	} ورنہ اگر (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ورنہ اگر ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ورنہ اگر (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ساخت box *inline_container, *inline_box;

		اگر (box_button(n, content, box, 0) == false)
			جاؤ no_حافظہ;

		inline_container = box_create(تہی, 0, false, 0, 0, 0, 0,
				content->bctx);
		اگر (inline_container == تہی)
			جاؤ no_حافظہ;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(تہی, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		اگر (inline_box == تہی)
			جاؤ no_حافظہ;

		inline_box->type = BOX_متن;

		اگر (box->gadget->قدر != تہی)
			inline_box->متن = talloc_strdup(content->bctx,
					box->gadget->قدر);
		ورنہ اگر (box->gadget->type == GADGET_SUBMIT)
			inline_box->متن = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ورنہ اگر (box->gadget->type == GADGET_RESET)
			inline_box->متن = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ورنہ
			inline_box->متن = talloc_strdup(content->bctx,
							 "Button");

		اگر (inline_box->متن == تہی)
			جاؤ no_حافظہ;

		inline_box->لمبائی = لڑی_لمبائی(inline_box->متن);

		box_جوڑو_بچہ(inline_container, inline_box);

		box_جوڑو_بچہ(box, inline_container);

	} ورنہ اگر (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		اگر (box->style && ns_computed_display(box->style,
				box_is_جڑ(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			اگر (err == DOM_NO_ERR && s != تہی) {
				error = nsurl_join(content->بنیاد_url,
						dom_string_ڈیٹا(s), &url);
				dom_string_unref(s);
				اگر (error != NSERROR_OK)
					جاؤ no_حافظہ;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				اگر (nsurl_compare(url, content->بنیاد_url,
						NSURL_COMPLETE) == false) {
					اگر (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						جاؤ no_حافظہ;
					}
				}
				nsurl_unref(url);
			}
		}
	} ورنہ {
		/* unhandled type the default is "text" */
		اگر (box_داخلہ_متن(content, box, n) == false)
			جاؤ no_حافظہ;
	}

	dom_string_unref(type);

	*convert_children = false;

	واپس true;

no_حافظہ:
	dom_string_unref(type);

	واپس false;
}


/**
 * Noscript element
 */
ساکن bool
box_noscript(dom_گرہ *n,
	     html_content *content,
	     ساخت box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	اگر (content->enable_scripting) {
		*convert_children = false;
	}

	واپس true;
}


/**
 * Generic embedded object [13.3].
 */
ساکن bool
box_object(dom_گرہ *n,
	   html_content *content,
	   ساخت box *box,
	   bool *convert_children)
{
	ساخت object_params *params;
	ساخت object_param *param;
	dom_string *codebase, *classid, *ڈیٹا;
	dom_گرہ *c;
	dom_exception err;

	اگر (box->style && ns_computed_display(box->style,
			box_is_جڑ(n)) == CSS_DISPLAY_NONE)
		واپس true;

	اگر (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		واپس false;
	اگر (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ساخت object_params);
	اگر (params == تہی)
		واپس false;

	talloc_سیٹ_destructor(params, box_object_talloc_destructor);

	params->ڈیٹا = تہی;
	params->type = تہی;
	params->codetype = تہی;
	params->codebase = تہی;
	params->classid = تہی;
	params->params = تہی;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	اگر (err == DOM_NO_ERR && codebase != تہی) {
		اگر (box_نکالو_link(content, codebase,	content->بنیاد_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			واپس false;
		}
		dom_string_unref(codebase);
	}
	اگر (params->codebase == تہی)
		params->codebase = nsurl_ref(content->بنیاد_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	اگر (err == DOM_NO_ERR && classid != تہی) {
		اگر (box_نکالو_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			واپس false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_ڈیٹا, &ڈیٹا);
	اگر (err == DOM_NO_ERR && ڈیٹا != تہی) {
		اگر (box_نکالو_link(content, ڈیٹا,
				params->codebase, &params->ڈیٹا) == false) {
			dom_string_unref(ڈیٹا);
			واپس false;
		}
		dom_string_unref(ڈیٹا);
	}

	اگر (params->classid == تہی && params->ڈیٹا == تہی)
		/* nothing to embed; ignore */
		واپس true;

	/* Don't include ourself */
	اگر (params->classid != تہی && nsurl_compare(content->بنیاد_url,
			params->classid, NSURL_COMPLETE))
		واپس true;

	اگر (params->ڈیٹا != تہی && nsurl_compare(content->بنیاد_url,
			params->ڈیٹا, NSURL_COMPLETE))
		واپس true;

	/* codetype and type are MIME types */
	اگر (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		واپس false;
	اگر (box_get_attribute(n, "type", params, &params->type) == false)
		واپس false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	اگر (params->classid != تہی && params->ڈیٹا == تہی &&
			params->codetype != تہی) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				لڑی_لمبائی(params->codetype), &icodetype);
		اگر (lerror != lwc_error_ok)
			واپس false;

		اگر (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			واپس true;
		}

		lwc_string_unref(icodetype);
	}

	اگر (params->ڈیٹا != تہی && params->type != تہی) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, لڑی_لمبائی(params->type),
				&itype);
		اگر (lerror != lwc_error_ok)
			واپس false;

		اگر (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			واپس true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_گرہ_get_پہلا_بچہ(n, &c);
	اگر (err != DOM_NO_ERR)
		واپس false;

	جب (c != تہی) {
		dom_گرہ *اگلا;
		dom_گرہ_type type;

		err = dom_گرہ_get_گرہ_type(c, &type);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(c);
			واپس false;
		}

		اگر (type == DOM_ELEMENT_گرہ) {
			dom_string *name;

			err = dom_گرہ_get_گرہ_name(c, &name);
			اگر (err != DOM_NO_ERR) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_گرہ_unref(c);
				توڑو;
			}
			dom_string_unref(name);

			param = talloc(params, ساخت object_param);
			اگر (param == تہی) {
				dom_گرہ_unref(c);
				واپس false;
			}
			param->name = تہی;
			param->قدر = تہی;
			param->type = تہی;
			param->valuetype = تہی;
			param->اگلا = تہی;

			اگر (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (box_get_attribute(c, "value", param,
					&param->قدر) == false) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_گرہ_unref(c);
				واپس false;
			}

			اگر (param->valuetype == تہی) {
				param->valuetype = talloc_strdup(param, "data");
				اگر (param->valuetype == تہی) {
					dom_گرہ_unref(c);
					واپس false;
				}
			}

			param->اگلا = params->params;
			params->params = param;
		}

		err = dom_گرہ_get_اگلا_sibling(c, &اگلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(c);
			واپس false;
		}

		dom_گرہ_unref(c);
		c = اگلا;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	اگر (!html_fetch_object(content,
			       params->ڈیٹا ? params->ڈیٹا : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		واپس false;

	*convert_children = false;
	واپس true;
}


/**
 * Preformatted text [9.3.4].
 */
ساکن bool
box_pre(dom_گرہ *n,
	html_content *content,
	ساخت box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	واپس true;
}


/**
 * Option selector [17.6].
 */
ساکن bool
box_select(dom_گرہ *n,
	   html_content *content,
	   ساخت box *box,
	   bool *convert_children)
{
	ساخت box *inline_container;
	ساخت box *inline_box;
	ساخت form_control *gadget;
	dom_گرہ *c, *c2;
	dom_گرہ *اگلا, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_گرہ(content->forms, n);
	اگر (gadget == تہی)
		واپس false;

	gadget->html = content;
	err = dom_گرہ_get_پہلا_بچہ(n, &c);
	اگر (err != DOM_NO_ERR) {
		form_free_control(gadget);
		واپس false;
	}

	جب (c != تہی) {
		dom_string *name;

		err = dom_گرہ_get_گرہ_name(c, &name);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(c);
			form_free_control(gadget);
			واپس false;
		}

		اگر (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			اگر (box_select_جوڑو_option(gadget, c) == false) {
				dom_گرہ_unref(c);
				form_free_control(gadget);
				واپس false;
			}
		} ورنہ اگر (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_گرہ_get_پہلا_بچہ(c, &c2);
			اگر (err != DOM_NO_ERR) {
				dom_گرہ_unref(c);
				form_free_control(gadget);
				واپس false;
			}

			جب (c2 != تہی) {
				dom_string *c2_name;

				err = dom_گرہ_get_گرہ_name(c2, &c2_name);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(c2);
					dom_گرہ_unref(c);
					form_free_control(gadget);
					واپس false;
				}

				اگر (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					اگر (box_select_جوڑو_option(gadget,
							c2) == false) {
						dom_گرہ_unref(c2);
						dom_گرہ_unref(c);
						form_free_control(gadget);
						واپس false;
					}
				} ورنہ {
					dom_string_unref(c2_name);
				}

				err = dom_گرہ_get_اگلا_sibling(c2, &next2);
				اگر (err != DOM_NO_ERR) {
					dom_گرہ_unref(c2);
					dom_گرہ_unref(c);
					form_free_control(gadget);
					واپس false;
				}

				dom_گرہ_unref(c2);
				c2 = next2;
			}
		} ورنہ {
			dom_string_unref(name);
		}

		err = dom_گرہ_get_اگلا_sibling(c, &اگلا);
		اگر (err != DOM_NO_ERR) {
			dom_گرہ_unref(c);
			form_free_control(gadget);
			واپس false;
		}

		dom_گرہ_unref(c);
		c = اگلا;
	}

	اگر (gadget->ڈیٹا.select.عدد_اشیاء == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		واپس true;
	}

	box->type = BOX_INLINE_بلاک;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(تہی, 0, false, 0, 0, 0, 0, content->bctx);
	اگر (inline_container == تہی)
		جاؤ no_حافظہ;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(تہی, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	اگر (inline_box == تہی)
		جاؤ no_حافظہ;
	inline_box->type = BOX_متن;
	box_جوڑو_بچہ(inline_container, inline_box);
	box_جوڑو_بچہ(box, inline_container);

	اگر (gadget->ڈیٹا.select.مضرب == false &&
			gadget->ڈیٹا.select.عدد_selected == 0) {
		gadget->ڈیٹا.select.current = gadget->ڈیٹا.select.اشیاء;
		gadget->ڈیٹا.select.current->initial_selected =
			gadget->ڈیٹا.select.current->selected = true;
		gadget->ڈیٹا.select.عدد_selected = 1;
		dom_html_option_element_سیٹ_selected(
				gadget->ڈیٹا.select.current->گرہ, true);
	}

	اگر (gadget->ڈیٹا.select.عدد_selected == 0)
		inline_box->متن = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ورنہ اگر (gadget->ڈیٹا.select.عدد_selected == 1)
		inline_box->متن = talloc_strdup(content->bctx,
				gadget->ڈیٹا.select.current->متن);
	ورنہ
		inline_box->متن = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	اگر (inline_box->متن == تہی)
		جاؤ no_حافظہ;

	inline_box->لمبائی = لڑی_لمبائی(inline_box->متن);

	*convert_children = false;
	واپس true;

no_حافظہ:
	واپس false;
}


/**
 * Multi-line text field [17.7].
 */
ساکن bool box_textarea(dom_گرہ *n,
			html_content *content,
			ساخت box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_گرہ(content->forms, n);
	اگر (box->gadget == تہی)
		واپس false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	اگر (!box_داخلہ_متن(content, box, n))
		واپس false;

	*convert_children = false;
	واپس true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_گرہ *گرہ,
			 html_content *content,
			 ساخت box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type لیبل_type;
	bool res;

	exc = dom_html_element_get_لیبل_type(گرہ, &لیبل_type);
	اگر (exc != DOM_NO_ERR) {
		لیبل_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	انتخاب (لیبل_type) {
	حالت DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_داخلہ:
		res = box_داخلہ(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(گرہ, content, box, convert_children);
		توڑو;

	حالت DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(گرہ, content, box, convert_children);
		توڑو;

	طےشدہ:
		res = true;
	}

	واپس res;
}
