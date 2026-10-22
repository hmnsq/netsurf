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

#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <dom/dom.h>

#ضمن "utils/nsoption.h"
#ضمن "utils/corestrings.h"
#ضمن "utils/log.h"
#ضمن "utils/messages.h"
#ضمن "utils/talloc.h"
#ضمن "utils/string.h"
#ضمن "utils/ascii.h"
#ضمن "utils/nsurl.h"
#ضمن "netsurf/plot_style.h"
#ضمن "css/hints.h"
#ضمن "desktop/frame_types.h"
#ضمن "content/content_factory.h"

#ضمن "html/html.h"
#ضمن "html/private.h"
#ضمن "html/object.h"
#ضمن "html/box.h"
#ضمن "html/box_manipulate.h"
#ضمن "html/box_construct.h"
#ضمن "html/box_special.h"
#ضمن "html/box_textarea.h"
#ضمن "html/form_internal.h"


ساكن ثابت content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ساكن مضمن bool box_is_ریشه(dom_گره *n)
{
	dom_گره *والد;
	dom_گره_type type;
	dom_exception err;

	err = dom_گره_get_والد_گره(n, &والد);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (والد != عدم) {
		err = dom_گره_get_گره_type(والد, &type);

		dom_گره_unref(والد);

		إذا (err != DOM_NO_ERR)
			أرجع false;

		إذا (type != DOM_DOCUMENT_گره)
			أرجع false;
	}

	أرجع true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_object_talloc_destructor(بنية object_params *o)
{
	إذا (o->codebase != عدم)
		nsurl_unref(o->codebase);
	إذا (o->classid != عدم)
		nsurl_unref(o->classid);
	إذا (o->داده != عدم)
		nsurl_unref(o->داده);

	أرجع 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ساكن بنية قاب_dimension *
box_parse_multi_lengths(ثابت dom_string *ds, بیعلامت صحيح *شمار)
{
	حرف *انتها;
	بیعلامت صحيح i, n;
	بنية قاب_dimension *درازی;
	ثابت حرف *s;

	s = dom_string_داده(ds);

	لكل (i = 0, n = 1; s[i]; i++)
		إذا (s[i] == ',')
			n++;

	درازی = حافظه_صفر_بده(n, حجم(بنية قاب_dimension));
	إذا (!درازی)
		أرجع عدم;

	لكل (i = 0; i != n; i++) {
		طالما (ascii_is_space(*s)) {
			s++;
		}
		درازی[i].مقدار = strtof(s, &انتها);
		إذا (درازی[i].مقدار <= 0) {
			درازی[i].مقدار = 1;
		}
		s = انتها;
		بدل (*s) {
			حالة '%':
				درازی[i].unit = قاب_DIMENSION_PERCENT;
				اكسر;
			حالة '*':
				درازی[i].unit = قاب_DIMENSION_RELATIVE;
				اكسر;
			افتراضي:
				درازی[i].unit = قاب_DIMENSION_PIXELS;
				اكسر;
		}
		طالما (*s && *s != ',') {
			s++;
		}
		إذا (*s == ',') {
			s++;
		}
	}

	*شمار = n;
	أرجع درازی;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_قابها_talloc_destructor(بنية content_html_قابها *f)
{
	إذا (f->url != عدم) {
		nsurl_unref(f->url);
		f->url = عدم;
	}

	أرجع 0;
}


/**
 * create a frameset box tree
 */
ساكن bool
box_create_frameset(بنية content_html_قابها *f,
		    dom_گره *n,
		    html_content *content)
{
	بیعلامت صحيح ردیف, ستون, index, i;
	بیعلامت صحيح ردیفها = 1, تعداد_ستون = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	بنية قاب_dimension *ردیف_ارتفاع = 0, *ستون_عرض = 0;
	dom_گره *c, *بعدی;
	بنية content_html_قابها *قاب;
	bool default_border = true;
	رنگ default_border_رنگ = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_ردیفها, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		ردیف_ارتفاع = box_parse_multi_lengths(s, &ردیفها);
		dom_string_unref(s);
		إذا (ردیف_ارتفاع == عدم)
			أرجع false;
	} وإلا {
		ردیف_ارتفاع = حافظه_صفر_بده(1, حجم(بنية قاب_dimension));
		إذا (ردیف_ارتفاع == عدم)
			أرجع false;
		ردیف_ارتفاع->مقدار = 100;
		ردیف_ارتفاع->unit = قاب_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_تعداد_ستون, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		ستون_عرض = box_parse_multi_lengths(s, &تعداد_ستون);
		dom_string_unref(s);
		إذا (ستون_عرض == عدم) {
			حرر(ردیف_ارتفاع);
			أرجع false;
		}
	} وإلا {
		ستون_عرض = حافظه_صفر_بده(1, حجم(بنية قاب_dimension));
		إذا (ستون_عرض == عدم) {
			حرر(ردیف_ارتفاع);
			أرجع false;
		}
		ستون_عرض->مقدار = 100;
		ستون_عرض->unit = قاب_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		إذا ((dom_string_داده(s)[0] == '0') &&
				(dom_string_داده(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		css_color color;

		إذا (nscss_parse_رنگ(dom_string_داده(s), &color))
			default_border_رنگ = nscss_color_مقصد_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->تعداد_ستون = تعداد_ستون;
	f->ردیفها = ردیفها;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, بنية content_html_قابها,
								(ردیفها * تعداد_ستون));

	talloc_مجموعه_destructor(f->children, box_قابها_talloc_destructor);

	لكل (ردیف = 0; ردیف < ردیفها; ردیف++) {
		لكل (ستون = 0; ستون < تعداد_ستون; ستون++) {
			index = (ردیف * تعداد_ستون) + ستون;
			قاب = &f->children[index];
			قاب->تعداد_ستون = 0;
			قاب->ردیفها = 0;
			قاب->عرض = ستون_عرض[ستون];
			قاب->ارتفاع = ردیف_ارتفاع[ردیف];
			قاب->margin_عرض = 0;
			قاب->margin_ارتفاع = 0;
			قاب->name = عدم;
			قاب->url = عدم;
			قاب->no_resize = false;
			قاب->scrolling = BW_SCROLLING_AUTO;
			قاب->border = default_border;
			قاب->border_رنگ = default_border_رنگ;
			قاب->children = عدم;
		}
	}
	حرر(ستون_عرض);
	حرر(ردیف_ارتفاع);

	/* create the frameset windows */
	err = dom_گره_get_اولی_فرزند(n, &c);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	لكل (ردیف = 0; c != عدم && ردیف < ردیفها; ردیف++) {
		لكل (ستون = 0; c != عدم && ستون < تعداد_ستون; ستون++) {
			طالما (c != عدم) {
				dom_گره_type type;
				dom_string *name;

				err = dom_گره_get_گره_type(c, &type);
				إذا (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					أرجع false;
				}

				err = dom_گره_get_گره_name(c, &name);
				إذا (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					أرجع false;
				}

				إذا (type != DOM_ELEMENT_گره ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_قاب) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_گره_get_بعدی_sibling(c,
							&بعدی);
					إذا (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_گره_unref(c);
						أرجع false;
					}

					dom_string_unref(name);
					dom_گره_unref(c);
					c = بعدی;
				} وإلا {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					اكسر;
				}
			}

			إذا (c == عدم)
				اكسر;

			/* get current frame */
			index = (ردیف * تعداد_ستون) + ستون;
			قاب = &f->children[index];

			/* nest framesets */
			err = dom_گره_get_گره_name(c, &s);
			إذا (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				قاب->border = 0;
				إذا (box_create_frameset(قاب, c,
						content) == false) {
					dom_گره_unref(c);
					أرجع false;
				}

				err = dom_گره_get_بعدی_sibling(c, &بعدی);
				إذا (err != DOM_NO_ERR) {
					dom_گره_unref(c);
					أرجع false;
				}

				dom_گره_unref(c);
				c = بعدی;
				استمر;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = عدم;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				box_استخراج_link(content, s, content->پایه_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			إذا (url != عدم) {
				/* no self-references */
				إذا (nsurl_compare(content->پایه_url, url,
						NSURL_COMPLETE) == false)
					قاب->url = url;
				url = عدم;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				قاب->name = talloc_strdup(content->bctx,
						dom_string_داده(s));
				dom_string_unref(s);
			}

			إذا (dom_element_has_attribute(c, corestring_dom_noresize,
						      &قاب->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				قاب->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				i = atoi(dom_string_داده(s));
				قاب->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				إذا (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					قاب->scrolling = BW_SCROLLING_YES;
				وإلا إذا (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					قاب->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				قاب->margin_عرض = atoi(dom_string_داده(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				قاب->margin_ارتفاع = atoi(dom_string_داده(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				css_color color;

				إذا (nscss_parse_رنگ(dom_string_داده(s),
						&color))
					قاب->border_رنگ =
						nscss_color_مقصد_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_گره_get_بعدی_sibling(c, &بعدی);
			إذا (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				أرجع false;
			}

			dom_گره_unref(c);
			c = بعدی;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	إذا (c != عدم) {
		dom_گره_unref(c);
	}

	أرجع true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ساكن صحيح box_iframes_talloc_destructor(بنية content_html_iframe *f)
{
	إذا (f->url != عدم) {
		nsurl_unref(f->url);
		f->url = عدم;
	}

	أرجع 0;
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
ساكن bool
box_get_attribute(dom_گره *n,
		  ثابت حرف *attribute,
		  فراغ *context,
		  حرف **مقدار)
{
	حرف *نتیجه;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ثابت uint8_t *) attribute,
			طول_رشته(attribute), &attr_name);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	إذا (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		أرجع false;
	}

	dom_string_unref(attr_name);

	إذا (attr != عدم) {
		نتیجه = talloc_strdup(context, dom_string_داده(attr));

		dom_string_unref(attr);

		إذا (نتیجه == عدم)
			أرجع false;

		*مقدار = نتیجه;
	}

	أرجع true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ساكن bool
box_درون_داد_متن(html_content *html, بنية box *box, بنية dom_گره *گره)
{
	بنية box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	بدل (display) {
	حالة CSS_DISPLAY_GRID:
	حالة CSS_DISPLAY_FLEX:
	حالة CSS_DISPLAY_بلوک:
		box->type = BOX_بلوک;
		اكسر;
	افتراضي:
		box->type = BOX_INLINE_بلوک;
		اكسر;
	}

	inline_container = box_create(عدم, 0, false, 0, 0, 0, 0, html->bctx);
	إذا (!inline_container)
		أرجع false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(عدم, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	إذا (!inline_box)
		أرجع false;
	inline_box->type = BOX_متن;
	inline_box->متن = talloc_strdup(html->bctx, "");

	box_جمع_کن_فرزند(inline_container, inline_box);
	box_جمع_کن_فرزند(box, inline_container);

	أرجع box_textarea_create_textarea(html, box, گره);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ساكن bool box_select_جمع_کن_option(بنية form_control *control, dom_گره *n)
{
	حرف *مقدار = عدم;
	حرف *متن = عدم;
	حرف *متن_nowrap = عدم;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_گره_get_متن_content(n, &content);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	إذا (content != عدم) {
		متن = squash_whitespace(dom_string_داده(content));
		dom_string_unref(content);
	} وإلا {
		متن = strdup("");
	}

	إذا (متن == عدم)
		اذهب no_حافظه;

	err = dom_element_get_attribute(n, corestring_dom_مقدار, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		مقدار = strdup(dom_string_داده(s));
		dom_string_unref(s);
	} وإلا {
		مقدار = strdup(متن);
	}

	إذا (مقدار == عدم)
		اذهب no_حافظه;

	إذا (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	متن_nowrap = cnv_space2nbsp(متن);
	إذا (متن_nowrap == عدم)
		اذهب no_حافظه;

	إذا (form_جمع_کن_option(control, مقدار, متن_nowrap, selected, n) == false)
		اذهب no_حافظه;

	حرر(متن);

	أرجع true;

no_حافظه:
	حرر(مقدار);
	حرر(متن);
	حرر(متن_nowrap);
	أرجع false;
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
ساكن bool
box_a(dom_گره *n,
      html_content *content,
      بنية box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		ok = box_استخراج_link(content, s, content->پایه_url, &url);
		dom_string_unref(s);
		إذا (!ok)
			أرجع false;
		إذا (url) {
			إذا (box->href != عدم)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		إذا (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_هدف, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->هدف = "_blank";
		وإلا إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_بالاترین_2))
			box->هدف = "_top";
		وإلا إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_والد))
			box->هدف = "_parent";
		وإلا إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->هدف = "_self";
		وإلا {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->هدف = talloc_strdup(content->bctx,
					dom_string_داده(s));
			إذا (!box->هدف) {
				dom_string_unref(s);
				أرجع false;
			}
		}
		dom_string_unref(s);
	}

	أرجع true;
}


/**
 * Document body special element handler [7.5.1].
 */
ساكن bool
box_body(dom_گره *n,
	 html_content *content,
	 بنية box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	إذا (nscss_color_is_transparent(color)) {
		content->background_رنگ = NS_TRANSPARENT;
	} وإلا {
		content->background_رنگ = nscss_color_مقصد_ns(color);
	}

	أرجع true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ساكن bool
box_br(dom_گره *n,
       html_content *content,
       بنية box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	أرجع true;
}


/**
 * special element handler for Push button [17.5].
 */
ساكن bool
box_button(dom_گره *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية form_control *gadget;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	إذا (!gadget)
		أرجع false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_بلوک;

	/* Just render the contents */

	أرجع true;
}


/**
 * Canvas element
 */
ساكن bool
box_canvas(dom_گره *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	إذا (!content->enable_scripting) {
		أرجع true;
	}
	*convert_children = false;

	إذا (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		أرجع true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	أرجع true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ساكن bool
box_embed(dom_گره *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	بنية object_params *params;
	بنية object_param *param;
	dom_namednodemap *attrs;
	بیعلامت دراز idx;
	uint32_t رقم_attrs;
	dom_string *src;
	dom_exception err;

	إذا (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		أرجع true;

	params = talloc(content->bctx, بنية object_params);
	إذا (params == عدم)
		أرجع false;

	talloc_مجموعه_destructor(params, box_object_talloc_destructor);

	params->داده = عدم;
	params->type = عدم;
	params->codetype = عدم;
	params->codebase = عدم;
	params->classid = عدم;
	params->params = عدم;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	إذا (err != DOM_NO_ERR || src == عدم)
		أرجع true;
	إذا (box_استخراج_link(content, src, content->پایه_url,
			     &params->داده) == false) {
		dom_string_unref(src);
		أرجع false;
	}

	dom_string_unref(src);

	إذا (params->داده == عدم)
		أرجع true;

	/* Don't include ourself */
	إذا (nsurl_compare(content->پایه_url, params->داده, NSURL_COMPLETE))
		أرجع true;

	/* add attributes as parameters to linked list */
	err = dom_گره_get_صفات(n, &attrs);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	err = dom_namednodemap_get_درازی(attrs, &رقم_attrs);
	إذا (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		أرجع false;
	}

	لكل (idx = 0; idx < رقم_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *مقدار;

		err = dom_namednodemap_قلم(attrs, idx, (فراغ *) &attr);
		إذا (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			أرجع false;
		}

		err = dom_attr_get_name(attr, &name);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(attr);
			dom_namednodemap_unref(attrs);
			أرجع false;
		}

		إذا (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_گره_unref(attr);
			dom_string_unref(name);
			استمر;
		}

		err = dom_attr_get_مقدار(attr, &مقدار);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			أرجع false;
		}

		param = talloc(content->bctx, بنية object_param);
		إذا (param == عدم) {
			dom_گره_unref(attr);
			dom_string_unref(مقدار);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			أرجع false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_داده(name));
		param->مقدار = talloc_strdup(content->bctx, dom_string_داده(مقدار));
		param->type = عدم;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->بعدی = عدم;

		dom_string_unref(مقدار);
		dom_string_unref(name);
		dom_گره_unref(attr);

		إذا (param->name == عدم || param->مقدار == عدم ||
				param->valuetype == عدم) {
			dom_namednodemap_unref(attrs);
			أرجع false;
		}

		param->بعدی = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	أرجع html_fetch_object(content, params->داده, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ساكن bool
box_frameset(dom_گره *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	bool ok;

	إذا (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		إذا (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		أرجع true;
	}

	content->frameset = talloc_zero(content->bctx,
					بنية content_html_قابها);
	إذا (!content->frameset) {
		أرجع false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	إذا (ok) {
		box->type = BOX_NONE;
	}

	إذا (convert_children) {
		*convert_children = false;
	}
	أرجع ok;
}


/**
 * Inline subwindow [16.5].
 */
ساكن bool
box_iframe(dom_گره *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	بنية content_html_iframe *iframe;
	صحيح i;

	إذا (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		أرجع true;

	إذا (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		أرجع true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	إذا (err != DOM_NO_ERR || s == عدم)
		أرجع true;
	إذا (box_استخراج_link(content, s, content->پایه_url, &url) == false) {
		dom_string_unref(s);
		أرجع false;
	}
	dom_string_unref(s);
	إذا (url == عدم)
		أرجع true;

	/* don't include ourself */
	إذا (nsurl_compare(content->پایه_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		أرجع true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, بنية content_html_iframe);
	إذا (iframe == عدم) {
		nsurl_unref(url);
		أرجع false;
	}

	talloc_مجموعه_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_عرض = 0;
	iframe->margin_ارتفاع = 0;
	iframe->name = عدم;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->بعدی = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		iframe->name = talloc_strdup(content->bctx, dom_string_داده(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		i = atoi(dom_string_داده(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		css_color color;

		إذا (nscss_parse_رنگ(dom_string_داده(s), &color))
			iframe->border_رنگ = nscss_color_مقصد_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		وإلا إذا (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		iframe->margin_عرض = atoi(dom_string_داده(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		iframe->margin_ارتفاع = atoi(dom_string_داده(s));
		dom_string_unref(s);
	}

	/* box */
	أكد(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	إذا (convert_children)
		*convert_children = false;
	أرجع true;
}


/**
 * Embedded image [13.2].
 */
ساكن bool
box_image(dom_گره *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	تعداد css_عرض_e wtype;
	تعداد css_ارتفاع_e htype;
	css_fixed مقدار = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	إذا (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		أرجع true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	إذا (err == DOM_NO_ERR && s != عدم) {
		حرف *alt = squash_whitespace(dom_string_داده(s));
		dom_string_unref(s);
		إذا (alt == عدم)
			أرجع false;
		box->متن = talloc_strdup(content->bctx, alt);
		حرر(alt);
		إذا (box->متن == عدم)
			أرجع false;
		box->درازی = طول_رشته(box->متن);
	}

	إذا (nsoption_bool(foreground_images) == false) {
		أرجع true;
	}

	/* imagemap associated with this image */
	إذا (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		أرجع false;
	إذا (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	إذا (err != DOM_NO_ERR || s == عدم)
		أرجع true;

	إذا (box_استخراج_link(content, s, content->پایه_url, &url) == false) {
		dom_string_unref(s);
		أرجع false;
	}

	dom_string_unref(s);

	إذا (url == عدم)
		أرجع true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_عرض(box->style, &مقدار, &wunit);
	htype = css_computed_ارتفاع(box->style, &مقدار, &hunit);

	إذا (wtype == CSS_عرض_مجموعه &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_ارتفاع_مجموعه &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	أرجع ok;
}


/**
 * Form control [17.4].
 */
ساكن bool
box_درون_داد(dom_گره *n,
	  html_content *content,
	  بنية box *box,
	  bool *convert_children)
{
	بنية form_control *gadget;
	dom_string *type = عدم;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	إذا (gadget == عدم) {
		أرجع false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	إذا ((err != DOM_NO_ERR) || (type == عدم)) {
		/* no type so "text" is assumed */
		إذا (box_درون_داد_متن(content, box, n) == false) {
			أرجع false;
		}
		*convert_children = false;
		أرجع true;
	}

	إذا (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		إذا (box_درون_داد_متن(content, box, n) == false)
			اذهب no_حافظه;

	} وإلا إذا (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_بلوک;

	} وإلا إذا (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} وإلا إذا ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} وإلا إذا (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		بنية box *inline_container, *inline_box;

		إذا (box_button(n, content, box, 0) == false)
			اذهب no_حافظه;

		inline_container = box_create(عدم, 0, false, 0, 0, 0, 0,
				content->bctx);
		إذا (inline_container == عدم)
			اذهب no_حافظه;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(عدم, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		إذا (inline_box == عدم)
			اذهب no_حافظه;

		inline_box->type = BOX_متن;

		إذا (box->gadget->مقدار != عدم)
			inline_box->متن = talloc_strdup(content->bctx,
					box->gadget->مقدار);
		وإلا إذا (box->gadget->type == GADGET_SUBMIT)
			inline_box->متن = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		وإلا إذا (box->gadget->type == GADGET_RESET)
			inline_box->متن = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		وإلا
			inline_box->متن = talloc_strdup(content->bctx,
							 "Button");

		إذا (inline_box->متن == عدم)
			اذهب no_حافظه;

		inline_box->درازی = طول_رشته(inline_box->متن);

		box_جمع_کن_فرزند(inline_container, inline_box);

		box_جمع_کن_فرزند(box, inline_container);

	} وإلا إذا (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		إذا (box->style && ns_computed_display(box->style,
				box_is_ریشه(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			إذا (err == DOM_NO_ERR && s != عدم) {
				error = nsurl_join(content->پایه_url,
						dom_string_داده(s), &url);
				dom_string_unref(s);
				إذا (error != NSERROR_OK)
					اذهب no_حافظه;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				إذا (nsurl_compare(url, content->پایه_url,
						NSURL_COMPLETE) == false) {
					إذا (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						اذهب no_حافظه;
					}
				}
				nsurl_unref(url);
			}
		}
	} وإلا {
		/* unhandled type the default is "text" */
		إذا (box_درون_داد_متن(content, box, n) == false)
			اذهب no_حافظه;
	}

	dom_string_unref(type);

	*convert_children = false;

	أرجع true;

no_حافظه:
	dom_string_unref(type);

	أرجع false;
}


/**
 * Noscript element
 */
ساكن bool
box_noscript(dom_گره *n,
	     html_content *content,
	     بنية box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	إذا (content->enable_scripting) {
		*convert_children = false;
	}

	أرجع true;
}


/**
 * Generic embedded object [13.3].
 */
ساكن bool
box_object(dom_گره *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية object_params *params;
	بنية object_param *param;
	dom_string *codebase, *classid, *داده;
	dom_گره *c;
	dom_exception err;

	إذا (box->style && ns_computed_display(box->style,
			box_is_ریشه(n)) == CSS_DISPLAY_NONE)
		أرجع true;

	إذا (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		أرجع false;
	إذا (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, بنية object_params);
	إذا (params == عدم)
		أرجع false;

	talloc_مجموعه_destructor(params, box_object_talloc_destructor);

	params->داده = عدم;
	params->type = عدم;
	params->codetype = عدم;
	params->codebase = عدم;
	params->classid = عدم;
	params->params = عدم;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	إذا (err == DOM_NO_ERR && codebase != عدم) {
		إذا (box_استخراج_link(content, codebase,	content->پایه_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			أرجع false;
		}
		dom_string_unref(codebase);
	}
	إذا (params->codebase == عدم)
		params->codebase = nsurl_ref(content->پایه_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	إذا (err == DOM_NO_ERR && classid != عدم) {
		إذا (box_استخراج_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			أرجع false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_داده, &داده);
	إذا (err == DOM_NO_ERR && داده != عدم) {
		إذا (box_استخراج_link(content, داده,
				params->codebase, &params->داده) == false) {
			dom_string_unref(داده);
			أرجع false;
		}
		dom_string_unref(داده);
	}

	إذا (params->classid == عدم && params->داده == عدم)
		/* nothing to embed; ignore */
		أرجع true;

	/* Don't include ourself */
	إذا (params->classid != عدم && nsurl_compare(content->پایه_url,
			params->classid, NSURL_COMPLETE))
		أرجع true;

	إذا (params->داده != عدم && nsurl_compare(content->پایه_url,
			params->داده, NSURL_COMPLETE))
		أرجع true;

	/* codetype and type are MIME types */
	إذا (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		أرجع false;
	إذا (box_get_attribute(n, "type", params, &params->type) == false)
		أرجع false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	إذا (params->classid != عدم && params->داده == عدم &&
			params->codetype != عدم) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				طول_رشته(params->codetype), &icodetype);
		إذا (lerror != lwc_error_ok)
			أرجع false;

		إذا (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			أرجع true;
		}

		lwc_string_unref(icodetype);
	}

	إذا (params->داده != عدم && params->type != عدم) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, طول_رشته(params->type),
				&itype);
		إذا (lerror != lwc_error_ok)
			أرجع false;

		إذا (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			أرجع true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_گره_get_اولی_فرزند(n, &c);
	إذا (err != DOM_NO_ERR)
		أرجع false;

	طالما (c != عدم) {
		dom_گره *بعدی;
		dom_گره_type type;

		err = dom_گره_get_گره_type(c, &type);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			أرجع false;
		}

		إذا (type == DOM_ELEMENT_گره) {
			dom_string *name;

			err = dom_گره_get_گره_name(c, &name);
			إذا (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_گره_unref(c);
				اكسر;
			}
			dom_string_unref(name);

			param = talloc(params, بنية object_param);
			إذا (param == عدم) {
				dom_گره_unref(c);
				أرجع false;
			}
			param->name = عدم;
			param->مقدار = عدم;
			param->type = عدم;
			param->valuetype = عدم;
			param->بعدی = عدم;

			إذا (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (box_get_attribute(c, "value", param,
					&param->مقدار) == false) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_گره_unref(c);
				أرجع false;
			}

			إذا (param->valuetype == عدم) {
				param->valuetype = talloc_strdup(param, "data");
				إذا (param->valuetype == عدم) {
					dom_گره_unref(c);
					أرجع false;
				}
			}

			param->بعدی = params->params;
			params->params = param;
		}

		err = dom_گره_get_بعدی_sibling(c, &بعدی);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			أرجع false;
		}

		dom_گره_unref(c);
		c = بعدی;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	إذا (!html_fetch_object(content,
			       params->داده ? params->داده : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		أرجع false;

	*convert_children = false;
	أرجع true;
}


/**
 * Preformatted text [9.3.4].
 */
ساكن bool
box_pre(dom_گره *n,
	html_content *content,
	بنية box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	أرجع true;
}


/**
 * Option selector [17.6].
 */
ساكن bool
box_select(dom_گره *n,
	   html_content *content,
	   بنية box *box,
	   bool *convert_children)
{
	بنية box *inline_container;
	بنية box *inline_box;
	بنية form_control *gadget;
	dom_گره *c, *c2;
	dom_گره *بعدی, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_گره(content->forms, n);
	إذا (gadget == عدم)
		أرجع false;

	gadget->html = content;
	err = dom_گره_get_اولی_فرزند(n, &c);
	إذا (err != DOM_NO_ERR) {
		form_free_control(gadget);
		أرجع false;
	}

	طالما (c != عدم) {
		dom_string *name;

		err = dom_گره_get_گره_name(c, &name);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			form_free_control(gadget);
			أرجع false;
		}

		إذا (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			إذا (box_select_جمع_کن_option(gadget, c) == false) {
				dom_گره_unref(c);
				form_free_control(gadget);
				أرجع false;
			}
		} وإلا إذا (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_گره_get_اولی_فرزند(c, &c2);
			إذا (err != DOM_NO_ERR) {
				dom_گره_unref(c);
				form_free_control(gadget);
				أرجع false;
			}

			طالما (c2 != عدم) {
				dom_string *c2_name;

				err = dom_گره_get_گره_name(c2, &c2_name);
				إذا (err != DOM_NO_ERR) {
					dom_گره_unref(c2);
					dom_گره_unref(c);
					form_free_control(gadget);
					أرجع false;
				}

				إذا (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					إذا (box_select_جمع_کن_option(gadget,
							c2) == false) {
						dom_گره_unref(c2);
						dom_گره_unref(c);
						form_free_control(gadget);
						أرجع false;
					}
				} وإلا {
					dom_string_unref(c2_name);
				}

				err = dom_گره_get_بعدی_sibling(c2, &next2);
				إذا (err != DOM_NO_ERR) {
					dom_گره_unref(c2);
					dom_گره_unref(c);
					form_free_control(gadget);
					أرجع false;
				}

				dom_گره_unref(c2);
				c2 = next2;
			}
		} وإلا {
			dom_string_unref(name);
		}

		err = dom_گره_get_بعدی_sibling(c, &بعدی);
		إذا (err != DOM_NO_ERR) {
			dom_گره_unref(c);
			form_free_control(gadget);
			أرجع false;
		}

		dom_گره_unref(c);
		c = بعدی;
	}

	إذا (gadget->داده.select.رقم_اقلام == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		أرجع true;
	}

	box->type = BOX_INLINE_بلوک;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(عدم, 0, false, 0, 0, 0, 0, content->bctx);
	إذا (inline_container == عدم)
		اذهب no_حافظه;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(عدم, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	إذا (inline_box == عدم)
		اذهب no_حافظه;
	inline_box->type = BOX_متن;
	box_جمع_کن_فرزند(inline_container, inline_box);
	box_جمع_کن_فرزند(box, inline_container);

	إذا (gadget->داده.select.مضرب == false &&
			gadget->داده.select.رقم_selected == 0) {
		gadget->داده.select.current = gadget->داده.select.اقلام;
		gadget->داده.select.current->initial_selected =
			gadget->داده.select.current->selected = true;
		gadget->داده.select.رقم_selected = 1;
		dom_html_option_element_مجموعه_selected(
				gadget->داده.select.current->گره, true);
	}

	إذا (gadget->داده.select.رقم_selected == 0)
		inline_box->متن = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	وإلا إذا (gadget->داده.select.رقم_selected == 1)
		inline_box->متن = talloc_strdup(content->bctx,
				gadget->داده.select.current->متن);
	وإلا
		inline_box->متن = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	إذا (inline_box->متن == عدم)
		اذهب no_حافظه;

	inline_box->درازی = طول_رشته(inline_box->متن);

	*convert_children = false;
	أرجع true;

no_حافظه:
	أرجع false;
}


/**
 * Multi-line text field [17.7].
 */
ساكن bool box_textarea(dom_گره *n,
			html_content *content,
			بنية box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_گره(content->forms, n);
	إذا (box->gadget == عدم)
		أرجع false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	إذا (!box_درون_داد_متن(content, box, n))
		أرجع false;

	*convert_children = false;
	أرجع true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_گره *گره,
			 html_content *content,
			 بنية box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type برچسب_type;
	bool res;

	exc = dom_html_element_get_برچسب_type(گره, &برچسب_type);
	إذا (exc != DOM_NO_ERR) {
		برچسب_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	بدل (برچسب_type) {
	حالة DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_درون_داد:
		res = box_درون_داد(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(گره, content, box, convert_children);
		اكسر;

	حالة DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(گره, content, box, convert_children);
		اكسر;

	افتراضي:
		res = true;
	}

	أرجع res;
}
