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

#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ "utils/nsoption.h"
#అన్తర్భావ "utils/corestrings.h"
#అన్తర్భావ "utils/log.h"
#అన్తర్భావ "utils/messages.h"
#అన్తర్భావ "utils/talloc.h"
#అన్తర్భావ "utils/string.h"
#అన్తర్భావ "utils/ascii.h"
#అన్తర్భావ "utils/nsurl.h"
#అన్తర్భావ "netsurf/plot_style.h"
#అన్తర్భావ "css/hints.h"
#అన్తర్భావ "desktop/frame_types.h"
#అన్తర్భావ "content/content_factory.h"

#అన్తర్భావ "html/html.h"
#అన్తర్భావ "html/private.h"
#అన్తర్భావ "html/object.h"
#అన్తర్భావ "html/box.h"
#అన్తర్భావ "html/box_manipulate.h"
#అన్తర్భావ "html/box_construct.h"
#అన్తర్భావ "html/box_special.h"
#అన్తర్భావ "html/box_textarea.h"
#అన్తర్భావ "html/form_internal.h"


స్థిర స్థిరరాశి content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
స్థిర అంతఃవరుస bool box_is_మూలం(dom_కణుపు *n)
{
	dom_కణుపు *మాతృ;
	dom_కణుపు_type type;
	dom_exception err;

	err = dom_కణుపు_get_మాతృ_కణుపు(n, &మాతృ);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (మాతృ != ఖాళీ) {
		err = dom_కణుపు_get_కణుపు_type(మాతృ, &type);

		dom_కణుపు_unref(మాతృ);

		ఒకవేళ (err != DOM_NO_ERR)
			తిరిగి false;

		ఒకవేళ (type != DOM_DOCUMENT_కణుపు)
			తిరిగి false;
	}

	తిరిగి true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
స్థిర పూర్ణాంకం box_object_talloc_destructor(నిర్మాణం object_params *o)
{
	ఒకవేళ (o->codebase != ఖాళీ)
		nsurl_unref(o->codebase);
	ఒకవేళ (o->classid != ఖాళీ)
		nsurl_unref(o->classid);
	ఒకవేళ (o->దత్తాంశం != ఖాళీ)
		nsurl_unref(o->దత్తాంశం);

	తిరిగి 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
స్థిర నిర్మాణం చట్రం_dimension *
box_parse_multi_lengths(స్థిరరాశి dom_string *ds, గుర్తులేని పూర్ణాంకం *లెక్క)
{
	అక్షరం *చివర;
	గుర్తులేని పూర్ణాంకం i, n;
	నిర్మాణం చట్రం_dimension *నిడివి;
	స్థిరరాశి అక్షరం *s;

	s = dom_string_దత్తాంశం(ds);

	కొరకు (i = 0, n = 1; s[i]; i++)
		ఒకవేళ (s[i] == ',')
			n++;

	నిడివి = సున్నా_జ్ఞాపకం_ఇవ్వు(n, పరిమాణం(నిర్మాణం చట్రం_dimension));
	ఒకవేళ (!నిడివి)
		తిరిగి ఖాళీ;

	కొరకు (i = 0; i != n; i++) {
		అయితే (ascii_is_space(*s)) {
			s++;
		}
		నిడివి[i].విలువ = strtof(s, &చివర);
		ఒకవేళ (నిడివి[i].విలువ <= 0) {
			నిడివి[i].విలువ = 1;
		}
		s = చివర;
		ఎంపిక (*s) {
			స్థితి '%':
				నిడివి[i].unit = చట్రం_DIMENSION_PERCENT;
				ఆపు;
			స్థితి '*':
				నిడివి[i].unit = చట్రం_DIMENSION_RELATIVE;
				ఆపు;
			అప్రమేయ:
				నిడివి[i].unit = చట్రం_DIMENSION_PIXELS;
				ఆపు;
		}
		అయితే (*s && *s != ',') {
			s++;
		}
		ఒకవేళ (*s == ',') {
			s++;
		}
	}

	*లెక్క = n;
	తిరిగి నిడివి;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
స్థిర పూర్ణాంకం box_చట్రాలు_talloc_destructor(నిర్మాణం content_html_చట్రాలు *f)
{
	ఒకవేళ (f->url != ఖాళీ) {
		nsurl_unref(f->url);
		f->url = ఖాళీ;
	}

	తిరిగి 0;
}


/**
 * create a frameset box tree
 */
స్థిర bool
box_create_frameset(నిర్మాణం content_html_చట్రాలు *f,
		    dom_కణుపు *n,
		    html_content *content)
{
	గుర్తులేని పూర్ణాంకం అడ్డువరుస, నిలువు, index, i;
	గుర్తులేని పూర్ణాంకం అడ్డువరుసలు = 1, నిలువుల_సంఖ్య = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	నిర్మాణం చట్రం_dimension *అడ్డువరుస_ఎత్తు = 0, *నిలువు_వెడల్పు = 0;
	dom_కణుపు *c, *తదుపరి;
	నిర్మాణం content_html_చట్రాలు *చట్రం;
	bool default_border = true;
	రంగు default_border_రంగు = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_అడ్డువరుసలు, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		అడ్డువరుస_ఎత్తు = box_parse_multi_lengths(s, &అడ్డువరుసలు);
		dom_string_unref(s);
		ఒకవేళ (అడ్డువరుస_ఎత్తు == ఖాళీ)
			తిరిగి false;
	} లేకపోతే {
		అడ్డువరుస_ఎత్తు = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం చట్రం_dimension));
		ఒకవేళ (అడ్డువరుస_ఎత్తు == ఖాళీ)
			తిరిగి false;
		అడ్డువరుస_ఎత్తు->విలువ = 100;
		అడ్డువరుస_ఎత్తు->unit = చట్రం_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_నిలువుల_సంఖ్య, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		నిలువు_వెడల్పు = box_parse_multi_lengths(s, &నిలువుల_సంఖ్య);
		dom_string_unref(s);
		ఒకవేళ (నిలువు_వెడల్పు == ఖాళీ) {
			విడుదల(అడ్డువరుస_ఎత్తు);
			తిరిగి false;
		}
	} లేకపోతే {
		నిలువు_వెడల్పు = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం చట్రం_dimension));
		ఒకవేళ (నిలువు_వెడల్పు == ఖాళీ) {
			విడుదల(అడ్డువరుస_ఎత్తు);
			తిరిగి false;
		}
		నిలువు_వెడల్పు->విలువ = 100;
		నిలువు_వెడల్పు->unit = చట్రం_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		ఒకవేళ ((dom_string_దత్తాంశం(s)[0] == '0') &&
				(dom_string_దత్తాంశం(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		css_color color;

		ఒకవేళ (nscss_parse_రంగు(dom_string_దత్తాంశం(s), &color))
			default_border_రంగు = nscss_color_గమ్యం_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->నిలువుల_సంఖ్య = నిలువుల_సంఖ్య;
	f->అడ్డువరుసలు = అడ్డువరుసలు;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, నిర్మాణం content_html_చట్రాలు,
								(అడ్డువరుసలు * నిలువుల_సంఖ్య));

	talloc_సమితి_destructor(f->children, box_చట్రాలు_talloc_destructor);

	కొరకు (అడ్డువరుస = 0; అడ్డువరుస < అడ్డువరుసలు; అడ్డువరుస++) {
		కొరకు (నిలువు = 0; నిలువు < నిలువుల_సంఖ్య; నిలువు++) {
			index = (అడ్డువరుస * నిలువుల_సంఖ్య) + నిలువు;
			చట్రం = &f->children[index];
			చట్రం->నిలువుల_సంఖ్య = 0;
			చట్రం->అడ్డువరుసలు = 0;
			చట్రం->వెడల్పు = నిలువు_వెడల్పు[నిలువు];
			చట్రం->ఎత్తు = అడ్డువరుస_ఎత్తు[అడ్డువరుస];
			చట్రం->margin_వెడల్పు = 0;
			చట్రం->margin_ఎత్తు = 0;
			చట్రం->name = ఖాళీ;
			చట్రం->url = ఖాళీ;
			చట్రం->no_resize = false;
			చట్రం->scrolling = BW_SCROLLING_AUTO;
			చట్రం->border = default_border;
			చట్రం->border_రంగు = default_border_రంగు;
			చట్రం->children = ఖాళీ;
		}
	}
	విడుదల(నిలువు_వెడల్పు);
	విడుదల(అడ్డువరుస_ఎత్తు);

	/* create the frameset windows */
	err = dom_కణుపు_get_మొదటి_బిడ్డ(n, &c);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	కొరకు (అడ్డువరుస = 0; c != ఖాళీ && అడ్డువరుస < అడ్డువరుసలు; అడ్డువరుస++) {
		కొరకు (నిలువు = 0; c != ఖాళీ && నిలువు < నిలువుల_సంఖ్య; నిలువు++) {
			అయితే (c != ఖాళీ) {
				dom_కణుపు_type type;
				dom_string *name;

				err = dom_కణుపు_get_కణుపు_type(c, &type);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(c);
					తిరిగి false;
				}

				err = dom_కణుపు_get_కణుపు_name(c, &name);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(c);
					తిరిగి false;
				}

				ఒకవేళ (type != DOM_ELEMENT_కణుపు ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_చట్రం) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_కణుపు_get_తదుపరి_sibling(c,
							&తదుపరి);
					ఒకవేళ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_కణుపు_unref(c);
						తిరిగి false;
					}

					dom_string_unref(name);
					dom_కణుపు_unref(c);
					c = తదుపరి;
				} లేకపోతే {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ఆపు;
				}
			}

			ఒకవేళ (c == ఖాళీ)
				ఆపు;

			/* get current frame */
			index = (అడ్డువరుస * నిలువుల_సంఖ్య) + నిలువు;
			చట్రం = &f->children[index];

			/* nest framesets */
			err = dom_కణుపు_get_కణుపు_name(c, &s);
			ఒకవేళ (err != DOM_NO_ERR) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				చట్రం->border = 0;
				ఒకవేళ (box_create_frameset(చట్రం, c,
						content) == false) {
					dom_కణుపు_unref(c);
					తిరిగి false;
				}

				err = dom_కణుపు_get_తదుపరి_sibling(c, &తదుపరి);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(c);
					తిరిగి false;
				}

				dom_కణుపు_unref(c);
				c = తదుపరి;
				కొనసాగు;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ఖాళీ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				box_వెలికితీయి_link(content, s, content->పునాది_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ఒకవేళ (url != ఖాళీ) {
				/* no self-references */
				ఒకవేళ (nsurl_compare(content->పునాది_url, url,
						NSURL_COMPLETE) == false)
					చట్రం->url = url;
				url = ఖాళీ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				చట్రం->name = talloc_strdup(content->bctx,
						dom_string_దత్తాంశం(s));
				dom_string_unref(s);
			}

			ఒకవేళ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &చట్రం->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				చట్రం->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				i = atoi(dom_string_దత్తాంశం(s));
				చట్రం->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				ఒకవేళ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					చట్రం->scrolling = BW_SCROLLING_YES;
				లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					చట్రం->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				చట్రం->margin_వెడల్పు = atoi(dom_string_దత్తాంశం(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				చట్రం->margin_ఎత్తు = atoi(dom_string_దత్తాంశం(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				css_color color;

				ఒకవేళ (nscss_parse_రంగు(dom_string_దత్తాంశం(s),
						&color))
					చట్రం->border_రంగు =
						nscss_color_గమ్యం_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_కణుపు_get_తదుపరి_sibling(c, &తదుపరి);
			ఒకవేళ (err != DOM_NO_ERR) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			dom_కణుపు_unref(c);
			c = తదుపరి;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ఒకవేళ (c != ఖాళీ) {
		dom_కణుపు_unref(c);
	}

	తిరిగి true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
స్థిర పూర్ణాంకం box_iframes_talloc_destructor(నిర్మాణం content_html_iframe *f)
{
	ఒకవేళ (f->url != ఖాళీ) {
		nsurl_unref(f->url);
		f->url = ఖాళీ;
	}

	తిరిగి 0;
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
స్థిర bool
box_get_attribute(dom_కణుపు *n,
		  స్థిరరాశి అక్షరం *attribute,
		  శూన్య *context,
		  అక్షరం **విలువ)
{
	అక్షరం *ఫలితం;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((స్థిరరాశి uint8_t *) attribute,
			అక్షరశ్రేణి_పొడవు(attribute), &attr_name);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		తిరిగి false;
	}

	dom_string_unref(attr_name);

	ఒకవేళ (attr != ఖాళీ) {
		ఫలితం = talloc_strdup(context, dom_string_దత్తాంశం(attr));

		dom_string_unref(attr);

		ఒకవేళ (ఫలితం == ఖాళీ)
			తిరిగి false;

		*విలువ = ఫలితం;
	}

	తిరిగి true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
స్థిర bool
box_ఆదానం_పాఠ్యం(html_content *html, నిర్మాణం box *box, నిర్మాణం dom_కణుపు *కణుపు)
{
	నిర్మాణం box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	ఎంపిక (display) {
	స్థితి CSS_DISPLAY_GRID:
	స్థితి CSS_DISPLAY_FLEX:
	స్థితి CSS_DISPLAY_ఖండం:
		box->type = BOX_ఖండం;
		ఆపు;
	అప్రమేయ:
		box->type = BOX_INLINE_ఖండం;
		ఆపు;
	}

	inline_container = box_create(ఖాళీ, 0, false, 0, 0, 0, 0, html->bctx);
	ఒకవేళ (!inline_container)
		తిరిగి false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ఖాళీ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ఒకవేళ (!inline_box)
		తిరిగి false;
	inline_box->type = BOX_పాఠ్యం;
	inline_box->పాఠ్యం = talloc_strdup(html->bctx, "");

	box_కూడు_బిడ్డ(inline_container, inline_box);
	box_కూడు_బిడ్డ(box, inline_container);

	తిరిగి box_textarea_create_textarea(html, box, కణుపు);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
స్థిర bool box_select_కూడు_option(నిర్మాణం form_control *control, dom_కణుపు *n)
{
	అక్షరం *విలువ = ఖాళీ;
	అక్షరం *పాఠ్యం = ఖాళీ;
	అక్షరం *పాఠ్యం_nowrap = ఖాళీ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_కణుపు_get_పాఠ్యం_content(n, &content);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	ఒకవేళ (content != ఖాళీ) {
		పాఠ్యం = squash_whitespace(dom_string_దత్తాంశం(content));
		dom_string_unref(content);
	} లేకపోతే {
		పాఠ్యం = strdup("");
	}

	ఒకవేళ (పాఠ్యం == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;

	err = dom_element_get_attribute(n, corestring_dom_విలువ, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		విలువ = strdup(dom_string_దత్తాంశం(s));
		dom_string_unref(s);
	} లేకపోతే {
		విలువ = strdup(పాఠ్యం);
	}

	ఒకవేళ (విలువ == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;

	ఒకవేళ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	పాఠ్యం_nowrap = cnv_space2nbsp(పాఠ్యం);
	ఒకవేళ (పాఠ్యం_nowrap == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;

	ఒకవేళ (form_కూడు_option(control, విలువ, పాఠ్యం_nowrap, selected, n) == false)
		వెళ్ళు no_జ్ఞాపకం;

	విడుదల(పాఠ్యం);

	తిరిగి true;

no_జ్ఞాపకం:
	విడుదల(విలువ);
	విడుదల(పాఠ్యం);
	విడుదల(పాఠ్యం_nowrap);
	తిరిగి false;
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
స్థిర bool
box_a(dom_కణుపు *n,
      html_content *content,
      నిర్మాణం box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		ok = box_వెలికితీయి_link(content, s, content->పునాది_url, &url);
		dom_string_unref(s);
		ఒకవేళ (!ok)
			తిరిగి false;
		ఒకవేళ (url) {
			ఒకవేళ (box->href != ఖాళీ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ఒకవేళ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_లక్ష్యం, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->లక్ష్యం = "_blank";
		లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_శిఖరం_2))
			box->లక్ష్యం = "_top";
		లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_మాతృ))
			box->లక్ష్యం = "_parent";
		లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->లక్ష్యం = "_self";
		లేకపోతే {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->లక్ష్యం = talloc_strdup(content->bctx,
					dom_string_దత్తాంశం(s));
			ఒకవేళ (!box->లక్ష్యం) {
				dom_string_unref(s);
				తిరిగి false;
			}
		}
		dom_string_unref(s);
	}

	తిరిగి true;
}


/**
 * Document body special element handler [7.5.1].
 */
స్థిర bool
box_body(dom_కణుపు *n,
	 html_content *content,
	 నిర్మాణం box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ఒకవేళ (nscss_color_is_transparent(color)) {
		content->background_రంగు = NS_TRANSPARENT;
	} లేకపోతే {
		content->background_రంగు = nscss_color_గమ్యం_ns(color);
	}

	తిరిగి true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
స్థిర bool
box_br(dom_కణుపు *n,
       html_content *content,
       నిర్మాణం box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	తిరిగి true;
}


/**
 * special element handler for Push button [17.5].
 */
స్థిర bool
box_button(dom_కణుపు *n,
	   html_content *content,
	   నిర్మాణం box *box,
	   bool *convert_children)
{
	నిర్మాణం form_control *gadget;

	gadget = html_forms_get_control_for_కణుపు(content->forms, n);
	ఒకవేళ (!gadget)
		తిరిగి false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_ఖండం;

	/* Just render the contents */

	తిరిగి true;
}


/**
 * Canvas element
 */
స్థిర bool
box_canvas(dom_కణుపు *n,
	     html_content *content,
	     నిర్మాణం box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ఒకవేళ (!content->enable_scripting) {
		తిరిగి true;
	}
	*convert_children = false;

	ఒకవేళ (box->style && ns_computed_display(box->style,
			box_is_మూలం(n)) == CSS_DISPLAY_NONE)
		తిరిగి true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	తిరిగి true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
స్థిర bool
box_embed(dom_కణుపు *n,
	  html_content *content,
	  నిర్మాణం box *box,
	  bool *convert_children)
{
	నిర్మాణం object_params *params;
	నిర్మాణం object_param *param;
	dom_namednodemap *attrs;
	గుర్తులేని దీర్ఘ idx;
	uint32_t అంకె_attrs;
	dom_string *src;
	dom_exception err;

	ఒకవేళ (box->style && ns_computed_display(box->style,
			box_is_మూలం(n)) == CSS_DISPLAY_NONE)
		తిరిగి true;

	params = talloc(content->bctx, నిర్మాణం object_params);
	ఒకవేళ (params == ఖాళీ)
		తిరిగి false;

	talloc_సమితి_destructor(params, box_object_talloc_destructor);

	params->దత్తాంశం = ఖాళీ;
	params->type = ఖాళీ;
	params->codetype = ఖాళీ;
	params->codebase = ఖాళీ;
	params->classid = ఖాళీ;
	params->params = ఖాళీ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ఒకవేళ (err != DOM_NO_ERR || src == ఖాళీ)
		తిరిగి true;
	ఒకవేళ (box_వెలికితీయి_link(content, src, content->పునాది_url,
			     &params->దత్తాంశం) == false) {
		dom_string_unref(src);
		తిరిగి false;
	}

	dom_string_unref(src);

	ఒకవేళ (params->దత్తాంశం == ఖాళీ)
		తిరిగి true;

	/* Don't include ourself */
	ఒకవేళ (nsurl_compare(content->పునాది_url, params->దత్తాంశం, NSURL_COMPLETE))
		తిరిగి true;

	/* add attributes as parameters to linked list */
	err = dom_కణుపు_get_లక్షణాలు(n, &attrs);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	err = dom_namednodemap_get_నిడివి(attrs, &అంకె_attrs);
	ఒకవేళ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		తిరిగి false;
	}

	కొరకు (idx = 0; idx < అంకె_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *విలువ;

		err = dom_namednodemap_అంశం(attrs, idx, (శూన్య *) &attr);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			తిరిగి false;
		}

		err = dom_attr_get_name(attr, &name);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(attr);
			dom_namednodemap_unref(attrs);
			తిరిగి false;
		}

		ఒకవేళ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_కణుపు_unref(attr);
			dom_string_unref(name);
			కొనసాగు;
		}

		err = dom_attr_get_విలువ(attr, &విలువ);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			తిరిగి false;
		}

		param = talloc(content->bctx, నిర్మాణం object_param);
		ఒకవేళ (param == ఖాళీ) {
			dom_కణుపు_unref(attr);
			dom_string_unref(విలువ);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			తిరిగి false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_దత్తాంశం(name));
		param->విలువ = talloc_strdup(content->bctx, dom_string_దత్తాంశం(విలువ));
		param->type = ఖాళీ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->తదుపరి = ఖాళీ;

		dom_string_unref(విలువ);
		dom_string_unref(name);
		dom_కణుపు_unref(attr);

		ఒకవేళ (param->name == ఖాళీ || param->విలువ == ఖాళీ ||
				param->valuetype == ఖాళీ) {
			dom_namednodemap_unref(attrs);
			తిరిగి false;
		}

		param->తదుపరి = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	తిరిగి html_fetch_object(content, params->దత్తాంశం, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
స్థిర bool
box_frameset(dom_కణుపు *n,
	     html_content *content,
	     నిర్మాణం box *box,
	     bool *convert_children)
{
	bool ok;

	ఒకవేళ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ఒకవేళ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		తిరిగి true;
	}

	content->frameset = talloc_zero(content->bctx,
					నిర్మాణం content_html_చట్రాలు);
	ఒకవేళ (!content->frameset) {
		తిరిగి false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ఒకవేళ (ok) {
		box->type = BOX_NONE;
	}

	ఒకవేళ (convert_children) {
		*convert_children = false;
	}
	తిరిగి ok;
}


/**
 * Inline subwindow [16.5].
 */
స్థిర bool
box_iframe(dom_కణుపు *n,
	   html_content *content,
	   నిర్మాణం box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	నిర్మాణం content_html_iframe *iframe;
	పూర్ణాంకం i;

	ఒకవేళ (box->style && ns_computed_display(box->style,
			box_is_మూలం(n)) == CSS_DISPLAY_NONE)
		తిరిగి true;

	ఒకవేళ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		తిరిగి true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ఒకవేళ (err != DOM_NO_ERR || s == ఖాళీ)
		తిరిగి true;
	ఒకవేళ (box_వెలికితీయి_link(content, s, content->పునాది_url, &url) == false) {
		dom_string_unref(s);
		తిరిగి false;
	}
	dom_string_unref(s);
	ఒకవేళ (url == ఖాళీ)
		తిరిగి true;

	/* don't include ourself */
	ఒకవేళ (nsurl_compare(content->పునాది_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		తిరిగి true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, నిర్మాణం content_html_iframe);
	ఒకవేళ (iframe == ఖాళీ) {
		nsurl_unref(url);
		తిరిగి false;
	}

	talloc_సమితి_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_వెడల్పు = 0;
	iframe->margin_ఎత్తు = 0;
	iframe->name = ఖాళీ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->తదుపరి = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_దత్తాంశం(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		i = atoi(dom_string_దత్తాంశం(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		css_color color;

		ఒకవేళ (nscss_parse_రంగు(dom_string_దత్తాంశం(s), &color))
			iframe->border_రంగు = nscss_color_గమ్యం_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		iframe->margin_వెడల్పు = atoi(dom_string_దత్తాంశం(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		iframe->margin_ఎత్తు = atoi(dom_string_దత్తాంశం(s));
		dom_string_unref(s);
	}

	/* box */
	ప్రతిజ్ఞా(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ఒకవేళ (convert_children)
		*convert_children = false;
	తిరిగి true;
}


/**
 * Embedded image [13.2].
 */
స్థిర bool
box_image(dom_కణుపు *n,
	  html_content *content,
	  నిర్మాణం box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	గణన css_వెడల్పు_e wtype;
	గణన css_ఎత్తు_e htype;
	css_fixed విలువ = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ఒకవేళ (box->style && ns_computed_display(box->style,
			box_is_మూలం(n)) == CSS_DISPLAY_NONE)
		తిరిగి true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
		అక్షరం *alt = squash_whitespace(dom_string_దత్తాంశం(s));
		dom_string_unref(s);
		ఒకవేళ (alt == ఖాళీ)
			తిరిగి false;
		box->పాఠ్యం = talloc_strdup(content->bctx, alt);
		విడుదల(alt);
		ఒకవేళ (box->పాఠ్యం == ఖాళీ)
			తిరిగి false;
		box->నిడివి = అక్షరశ్రేణి_పొడవు(box->పాఠ్యం);
	}

	ఒకవేళ (nsoption_bool(foreground_images) == false) {
		తిరిగి true;
	}

	/* imagemap associated with this image */
	ఒకవేళ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		తిరిగి false;
	ఒకవేళ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ఒకవేళ (err != DOM_NO_ERR || s == ఖాళీ)
		తిరిగి true;

	ఒకవేళ (box_వెలికితీయి_link(content, s, content->పునాది_url, &url) == false) {
		dom_string_unref(s);
		తిరిగి false;
	}

	dom_string_unref(s);

	ఒకవేళ (url == ఖాళీ)
		తిరిగి true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_వెడల్పు(box->style, &విలువ, &wunit);
	htype = css_computed_ఎత్తు(box->style, &విలువ, &hunit);

	ఒకవేళ (wtype == CSS_వెడల్పు_సమితి &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_ఎత్తు_సమితి &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	తిరిగి ok;
}


/**
 * Form control [17.4].
 */
స్థిర bool
box_ఆదానం(dom_కణుపు *n,
	  html_content *content,
	  నిర్మాణం box *box,
	  bool *convert_children)
{
	నిర్మాణం form_control *gadget;
	dom_string *type = ఖాళీ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_కణుపు(content->forms, n);
	ఒకవేళ (gadget == ఖాళీ) {
		తిరిగి false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ఒకవేళ ((err != DOM_NO_ERR) || (type == ఖాళీ)) {
		/* no type so "text" is assumed */
		ఒకవేళ (box_ఆదానం_పాఠ్యం(content, box, n) == false) {
			తిరిగి false;
		}
		*convert_children = false;
		తిరిగి true;
	}

	ఒకవేళ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ఒకవేళ (box_ఆదానం_పాఠ్యం(content, box, n) == false)
			వెళ్ళు no_జ్ఞాపకం;

	} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_ఖండం;

	} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} లేకపోతే ఒకవేళ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		నిర్మాణం box *inline_container, *inline_box;

		ఒకవేళ (box_button(n, content, box, 0) == false)
			వెళ్ళు no_జ్ఞాపకం;

		inline_container = box_create(ఖాళీ, 0, false, 0, 0, 0, 0,
				content->bctx);
		ఒకవేళ (inline_container == ఖాళీ)
			వెళ్ళు no_జ్ఞాపకం;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ఖాళీ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ఒకవేళ (inline_box == ఖాళీ)
			వెళ్ళు no_జ్ఞాపకం;

		inline_box->type = BOX_పాఠ్యం;

		ఒకవేళ (box->gadget->విలువ != ఖాళీ)
			inline_box->పాఠ్యం = talloc_strdup(content->bctx,
					box->gadget->విలువ);
		లేకపోతే ఒకవేళ (box->gadget->type == GADGET_SUBMIT)
			inline_box->పాఠ్యం = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		లేకపోతే ఒకవేళ (box->gadget->type == GADGET_RESET)
			inline_box->పాఠ్యం = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		లేకపోతే
			inline_box->పాఠ్యం = talloc_strdup(content->bctx,
							 "Button");

		ఒకవేళ (inline_box->పాఠ్యం == ఖాళీ)
			వెళ్ళు no_జ్ఞాపకం;

		inline_box->నిడివి = అక్షరశ్రేణి_పొడవు(inline_box->పాఠ్యం);

		box_కూడు_బిడ్డ(inline_container, inline_box);

		box_కూడు_బిడ్డ(box, inline_container);

	} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ఒకవేళ (box->style && ns_computed_display(box->style,
				box_is_మూలం(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ఒకవేళ (err == DOM_NO_ERR && s != ఖాళీ) {
				error = nsurl_join(content->పునాది_url,
						dom_string_దత్తాంశం(s), &url);
				dom_string_unref(s);
				ఒకవేళ (error != NSERROR_OK)
					వెళ్ళు no_జ్ఞాపకం;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ఒకవేళ (nsurl_compare(url, content->పునాది_url,
						NSURL_COMPLETE) == false) {
					ఒకవేళ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						వెళ్ళు no_జ్ఞాపకం;
					}
				}
				nsurl_unref(url);
			}
		}
	} లేకపోతే {
		/* unhandled type the default is "text" */
		ఒకవేళ (box_ఆదానం_పాఠ్యం(content, box, n) == false)
			వెళ్ళు no_జ్ఞాపకం;
	}

	dom_string_unref(type);

	*convert_children = false;

	తిరిగి true;

no_జ్ఞాపకం:
	dom_string_unref(type);

	తిరిగి false;
}


/**
 * Noscript element
 */
స్థిర bool
box_noscript(dom_కణుపు *n,
	     html_content *content,
	     నిర్మాణం box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ఒకవేళ (content->enable_scripting) {
		*convert_children = false;
	}

	తిరిగి true;
}


/**
 * Generic embedded object [13.3].
 */
స్థిర bool
box_object(dom_కణుపు *n,
	   html_content *content,
	   నిర్మాణం box *box,
	   bool *convert_children)
{
	నిర్మాణం object_params *params;
	నిర్మాణం object_param *param;
	dom_string *codebase, *classid, *దత్తాంశం;
	dom_కణుపు *c;
	dom_exception err;

	ఒకవేళ (box->style && ns_computed_display(box->style,
			box_is_మూలం(n)) == CSS_DISPLAY_NONE)
		తిరిగి true;

	ఒకవేళ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		తిరిగి false;
	ఒకవేళ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, నిర్మాణం object_params);
	ఒకవేళ (params == ఖాళీ)
		తిరిగి false;

	talloc_సమితి_destructor(params, box_object_talloc_destructor);

	params->దత్తాంశం = ఖాళీ;
	params->type = ఖాళీ;
	params->codetype = ఖాళీ;
	params->codebase = ఖాళీ;
	params->classid = ఖాళీ;
	params->params = ఖాళీ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ఒకవేళ (err == DOM_NO_ERR && codebase != ఖాళీ) {
		ఒకవేళ (box_వెలికితీయి_link(content, codebase,	content->పునాది_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			తిరిగి false;
		}
		dom_string_unref(codebase);
	}
	ఒకవేళ (params->codebase == ఖాళీ)
		params->codebase = nsurl_ref(content->పునాది_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ఒకవేళ (err == DOM_NO_ERR && classid != ఖాళీ) {
		ఒకవేళ (box_వెలికితీయి_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			తిరిగి false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_దత్తాంశం, &దత్తాంశం);
	ఒకవేళ (err == DOM_NO_ERR && దత్తాంశం != ఖాళీ) {
		ఒకవేళ (box_వెలికితీయి_link(content, దత్తాంశం,
				params->codebase, &params->దత్తాంశం) == false) {
			dom_string_unref(దత్తాంశం);
			తిరిగి false;
		}
		dom_string_unref(దత్తాంశం);
	}

	ఒకవేళ (params->classid == ఖాళీ && params->దత్తాంశం == ఖాళీ)
		/* nothing to embed; ignore */
		తిరిగి true;

	/* Don't include ourself */
	ఒకవేళ (params->classid != ఖాళీ && nsurl_compare(content->పునాది_url,
			params->classid, NSURL_COMPLETE))
		తిరిగి true;

	ఒకవేళ (params->దత్తాంశం != ఖాళీ && nsurl_compare(content->పునాది_url,
			params->దత్తాంశం, NSURL_COMPLETE))
		తిరిగి true;

	/* codetype and type are MIME types */
	ఒకవేళ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		తిరిగి false;
	ఒకవేళ (box_get_attribute(n, "type", params, &params->type) == false)
		తిరిగి false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ఒకవేళ (params->classid != ఖాళీ && params->దత్తాంశం == ఖాళీ &&
			params->codetype != ఖాళీ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				అక్షరశ్రేణి_పొడవు(params->codetype), &icodetype);
		ఒకవేళ (lerror != lwc_error_ok)
			తిరిగి false;

		ఒకవేళ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			తిరిగి true;
		}

		lwc_string_unref(icodetype);
	}

	ఒకవేళ (params->దత్తాంశం != ఖాళీ && params->type != ఖాళీ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, అక్షరశ్రేణి_పొడవు(params->type),
				&itype);
		ఒకవేళ (lerror != lwc_error_ok)
			తిరిగి false;

		ఒకవేళ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			తిరిగి true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_కణుపు_get_మొదటి_బిడ్డ(n, &c);
	ఒకవేళ (err != DOM_NO_ERR)
		తిరిగి false;

	అయితే (c != ఖాళీ) {
		dom_కణుపు *తదుపరి;
		dom_కణుపు_type type;

		err = dom_కణుపు_get_కణుపు_type(c, &type);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(c);
			తిరిగి false;
		}

		ఒకవేళ (type == DOM_ELEMENT_కణుపు) {
			dom_string *name;

			err = dom_కణుపు_get_కణుపు_name(c, &name);
			ఒకవేళ (err != DOM_NO_ERR) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_కణుపు_unref(c);
				ఆపు;
			}
			dom_string_unref(name);

			param = talloc(params, నిర్మాణం object_param);
			ఒకవేళ (param == ఖాళీ) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}
			param->name = ఖాళీ;
			param->విలువ = ఖాళీ;
			param->type = ఖాళీ;
			param->valuetype = ఖాళీ;
			param->తదుపరి = ఖాళీ;

			ఒకవేళ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (box_get_attribute(c, "value", param,
					&param->విలువ) == false) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_కణుపు_unref(c);
				తిరిగి false;
			}

			ఒకవేళ (param->valuetype == ఖాళీ) {
				param->valuetype = talloc_strdup(param, "data");
				ఒకవేళ (param->valuetype == ఖాళీ) {
					dom_కణుపు_unref(c);
					తిరిగి false;
				}
			}

			param->తదుపరి = params->params;
			params->params = param;
		}

		err = dom_కణుపు_get_తదుపరి_sibling(c, &తదుపరి);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(c);
			తిరిగి false;
		}

		dom_కణుపు_unref(c);
		c = తదుపరి;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ఒకవేళ (!html_fetch_object(content,
			       params->దత్తాంశం ? params->దత్తాంశం : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		తిరిగి false;

	*convert_children = false;
	తిరిగి true;
}


/**
 * Preformatted text [9.3.4].
 */
స్థిర bool
box_pre(dom_కణుపు *n,
	html_content *content,
	నిర్మాణం box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	తిరిగి true;
}


/**
 * Option selector [17.6].
 */
స్థిర bool
box_select(dom_కణుపు *n,
	   html_content *content,
	   నిర్మాణం box *box,
	   bool *convert_children)
{
	నిర్మాణం box *inline_container;
	నిర్మాణం box *inline_box;
	నిర్మాణం form_control *gadget;
	dom_కణుపు *c, *c2;
	dom_కణుపు *తదుపరి, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_కణుపు(content->forms, n);
	ఒకవేళ (gadget == ఖాళీ)
		తిరిగి false;

	gadget->html = content;
	err = dom_కణుపు_get_మొదటి_బిడ్డ(n, &c);
	ఒకవేళ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		తిరిగి false;
	}

	అయితే (c != ఖాళీ) {
		dom_string *name;

		err = dom_కణుపు_get_కణుపు_name(c, &name);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(c);
			form_free_control(gadget);
			తిరిగి false;
		}

		ఒకవేళ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ఒకవేళ (box_select_కూడు_option(gadget, c) == false) {
				dom_కణుపు_unref(c);
				form_free_control(gadget);
				తిరిగి false;
			}
		} లేకపోతే ఒకవేళ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_కణుపు_get_మొదటి_బిడ్డ(c, &c2);
			ఒకవేళ (err != DOM_NO_ERR) {
				dom_కణుపు_unref(c);
				form_free_control(gadget);
				తిరిగి false;
			}

			అయితే (c2 != ఖాళీ) {
				dom_string *c2_name;

				err = dom_కణుపు_get_కణుపు_name(c2, &c2_name);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(c2);
					dom_కణుపు_unref(c);
					form_free_control(gadget);
					తిరిగి false;
				}

				ఒకవేళ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ఒకవేళ (box_select_కూడు_option(gadget,
							c2) == false) {
						dom_కణుపు_unref(c2);
						dom_కణుపు_unref(c);
						form_free_control(gadget);
						తిరిగి false;
					}
				} లేకపోతే {
					dom_string_unref(c2_name);
				}

				err = dom_కణుపు_get_తదుపరి_sibling(c2, &next2);
				ఒకవేళ (err != DOM_NO_ERR) {
					dom_కణుపు_unref(c2);
					dom_కణుపు_unref(c);
					form_free_control(gadget);
					తిరిగి false;
				}

				dom_కణుపు_unref(c2);
				c2 = next2;
			}
		} లేకపోతే {
			dom_string_unref(name);
		}

		err = dom_కణుపు_get_తదుపరి_sibling(c, &తదుపరి);
		ఒకవేళ (err != DOM_NO_ERR) {
			dom_కణుపు_unref(c);
			form_free_control(gadget);
			తిరిగి false;
		}

		dom_కణుపు_unref(c);
		c = తదుపరి;
	}

	ఒకవేళ (gadget->దత్తాంశం.select.అంకె_వస్తువులు == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		తిరిగి true;
	}

	box->type = BOX_INLINE_ఖండం;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ఖాళీ, 0, false, 0, 0, 0, 0, content->bctx);
	ఒకవేళ (inline_container == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ఖాళీ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ఒకవేళ (inline_box == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;
	inline_box->type = BOX_పాఠ్యం;
	box_కూడు_బిడ్డ(inline_container, inline_box);
	box_కూడు_బిడ్డ(box, inline_container);

	ఒకవేళ (gadget->దత్తాంశం.select.గుణిజం == false &&
			gadget->దత్తాంశం.select.అంకె_selected == 0) {
		gadget->దత్తాంశం.select.current = gadget->దత్తాంశం.select.వస్తువులు;
		gadget->దత్తాంశం.select.current->initial_selected =
			gadget->దత్తాంశం.select.current->selected = true;
		gadget->దత్తాంశం.select.అంకె_selected = 1;
		dom_html_option_element_సమితి_selected(
				gadget->దత్తాంశం.select.current->కణుపు, true);
	}

	ఒకవేళ (gadget->దత్తాంశం.select.అంకె_selected == 0)
		inline_box->పాఠ్యం = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	లేకపోతే ఒకవేళ (gadget->దత్తాంశం.select.అంకె_selected == 1)
		inline_box->పాఠ్యం = talloc_strdup(content->bctx,
				gadget->దత్తాంశం.select.current->పాఠ్యం);
	లేకపోతే
		inline_box->పాఠ్యం = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ఒకవేళ (inline_box->పాఠ్యం == ఖాళీ)
		వెళ్ళు no_జ్ఞాపకం;

	inline_box->నిడివి = అక్షరశ్రేణి_పొడవు(inline_box->పాఠ్యం);

	*convert_children = false;
	తిరిగి true;

no_జ్ఞాపకం:
	తిరిగి false;
}


/**
 * Multi-line text field [17.7].
 */
స్థిర bool box_textarea(dom_కణుపు *n,
			html_content *content,
			నిర్మాణం box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_కణుపు(content->forms, n);
	ఒకవేళ (box->gadget == ఖాళీ)
		తిరిగి false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ఒకవేళ (!box_ఆదానం_పాఠ్యం(content, box, n))
		తిరిగి false;

	*convert_children = false;
	తిరిగి true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_కణుపు *కణుపు,
			 html_content *content,
			 నిర్మాణం box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type ట్యాగ్_type;
	bool res;

	exc = dom_html_element_get_ట్యాగ్_type(కణుపు, &ట్యాగ్_type);
	ఒకవేళ (exc != DOM_NO_ERR) {
		ట్యాగ్_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ఎంపిక (ట్యాగ్_type) {
	స్థితి DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_ఆదానం:
		res = box_ఆదానం(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(కణుపు, content, box, convert_children);
		ఆపు;

	స్థితి DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(కణుపు, content, box, convert_children);
		ఆపు;

	అప్రమేయ:
		res = true;
	}

	తిరిగి res;
}
