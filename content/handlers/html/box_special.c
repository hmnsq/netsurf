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

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsoption.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/corestrings.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/log.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/messages.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/talloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/string.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/ascii.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "utils/nsurl.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netsurf/plot_style.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "css/hints.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "desktop/frame_types.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "content/content_factory.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "html/html.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/private.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/object.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_manipulate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_construct.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_special.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/box_textarea.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "html/form_internal.h"


ਸਥਿਰ ਅਚਲ content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool box_is_ਜੜ੍ਹ(dom_ਗੰਢ *n)
{
	dom_ਗੰਢ *ਮਾਪਾ;
	dom_ਗੰਢ_type type;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਮਾਪਾ_ਗੰਢ(n, &ਮਾਪਾ);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (ਮਾਪਾ != ਨਿਰਾ) {
		err = dom_ਗੰਢ_get_ਗੰਢ_type(ਮਾਪਾ, &type);

		dom_ਗੰਢ_unref(ਮਾਪਾ);

		ਜੇ (err != DOM_NO_ERR)
			ਵਾਪਸ false;

		ਜੇ (type != DOM_DOCUMENT_ਗੰਢ)
			ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ਸਥਿਰ ਪੂਰਨ box_object_talloc_destructor(ਬਣਤਰ object_params *o)
{
	ਜੇ (o->codebase != ਨਿਰਾ)
		nsurl_unref(o->codebase);
	ਜੇ (o->classid != ਨਿਰਾ)
		nsurl_unref(o->classid);
	ਜੇ (o->ਅੰਕੜੇ != ਨਿਰਾ)
		nsurl_unref(o->ਅੰਕੜੇ);

	ਵਾਪਸ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ਸਥਿਰ ਬਣਤਰ ਚੌਖਟਾ_dimension *
box_parse_multi_lengths(ਅਚਲ dom_string *ds, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *ਗਿਣਤੀ_2)
{
	ਅੱਖਰ *ਸਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, n;
	ਬਣਤਰ ਚੌਖਟਾ_dimension *ਲੰਮਾਈ;
	ਅਚਲ ਅੱਖਰ *s;

	s = dom_string_ਅੰਕੜੇ(ds);

	ਲਈ (i = 0, n = 1; s[i]; i++)
		ਜੇ (s[i] == ',')
			n++;

	ਲੰਮਾਈ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(n, ਆਕਾਰ(ਬਣਤਰ ਚੌਖਟਾ_dimension));
	ਜੇ (!ਲੰਮਾਈ)
		ਵਾਪਸ ਨਿਰਾ;

	ਲਈ (i = 0; i != n; i++) {
		ਜਦੋਂ (ascii_is_space(*s)) {
			s++;
		}
		ਲੰਮਾਈ[i].ਮੁੱਲ = strtof(s, &ਸਿਰਾ);
		ਜੇ (ਲੰਮਾਈ[i].ਮੁੱਲ <= 0) {
			ਲੰਮਾਈ[i].ਮੁੱਲ = 1;
		}
		s = ਸਿਰਾ;
		ਚੋਣ (*s) {
			ਹਾਲਤ '%':
				ਲੰਮਾਈ[i].unit = ਚੌਖਟਾ_DIMENSION_PERCENT;
				ਰੋਕੋ;
			ਹਾਲਤ '*':
				ਲੰਮਾਈ[i].unit = ਚੌਖਟਾ_DIMENSION_RELATIVE;
				ਰੋਕੋ;
			ਮੂਲ:
				ਲੰਮਾਈ[i].unit = ਚੌਖਟਾ_DIMENSION_PIXELS;
				ਰੋਕੋ;
		}
		ਜਦੋਂ (*s && *s != ',') {
			s++;
		}
		ਜੇ (*s == ',') {
			s++;
		}
	}

	*ਗਿਣਤੀ_2 = n;
	ਵਾਪਸ ਲੰਮਾਈ;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ਸਥਿਰ ਪੂਰਨ box_ਚੌਖਟੇ_talloc_destructor(ਬਣਤਰ content_html_ਚੌਖਟੇ *f)
{
	ਜੇ (f->url != ਨਿਰਾ) {
		nsurl_unref(f->url);
		f->url = ਨਿਰਾ;
	}

	ਵਾਪਸ 0;
}


/**
 * create a frameset box tree
 */
ਸਥਿਰ bool
box_create_frameset(ਬਣਤਰ content_html_ਚੌਖਟੇ *f,
		    dom_ਗੰਢ *n,
		    html_content *content)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀ, ਖਾਨਾ, index, i;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪੰਕਤੀਆਂ = 1, ਖਾਨੇ_ਗਿਣਤੀ = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ਬਣਤਰ ਚੌਖਟਾ_dimension *ਪੰਕਤੀ_ਉਚਾਈ = 0, *ਖਾਨਾ_ਚੌੜਾਈ = 0;
	dom_ਗੰਢ *c, *ਅਗਲਾ;
	ਬਣਤਰ content_html_ਚੌਖਟੇ *ਚੌਖਟਾ;
	bool default_border = true;
	ਰੰਗ default_border_ਰੰਗ = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_ਪੰਕਤੀਆਂ, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਪੰਕਤੀ_ਉਚਾਈ = box_parse_multi_lengths(s, &ਪੰਕਤੀਆਂ);
		dom_string_unref(s);
		ਜੇ (ਪੰਕਤੀ_ਉਚਾਈ == ਨਿਰਾ)
			ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		ਪੰਕਤੀ_ਉਚਾਈ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ਚੌਖਟਾ_dimension));
		ਜੇ (ਪੰਕਤੀ_ਉਚਾਈ == ਨਿਰਾ)
			ਵਾਪਸ false;
		ਪੰਕਤੀ_ਉਚਾਈ->ਮੁੱਲ = 100;
		ਪੰਕਤੀ_ਉਚਾਈ->unit = ਚੌਖਟਾ_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_ਖਾਨੇ_ਗਿਣਤੀ, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਖਾਨਾ_ਚੌੜਾਈ = box_parse_multi_lengths(s, &ਖਾਨੇ_ਗਿਣਤੀ);
		dom_string_unref(s);
		ਜੇ (ਖਾਨਾ_ਚੌੜਾਈ == ਨਿਰਾ) {
			ਮੁਕਤ(ਪੰਕਤੀ_ਉਚਾਈ);
			ਵਾਪਸ false;
		}
	} ਨਹੀਂਤਾਂ {
		ਖਾਨਾ_ਚੌੜਾਈ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ਚੌਖਟਾ_dimension));
		ਜੇ (ਖਾਨਾ_ਚੌੜਾਈ == ਨਿਰਾ) {
			ਮੁਕਤ(ਪੰਕਤੀ_ਉਚਾਈ);
			ਵਾਪਸ false;
		}
		ਖਾਨਾ_ਚੌੜਾਈ->ਮੁੱਲ = 100;
		ਖਾਨਾ_ਚੌੜਾਈ->unit = ਚੌਖਟਾ_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਜੇ ((dom_string_ਅੰਕੜੇ(s)[0] == '0') &&
				(dom_string_ਅੰਕੜੇ(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		css_color color;

		ਜੇ (nscss_parse_ਰੰਗ(dom_string_ਅੰਕੜੇ(s), &color))
			default_border_ਰੰਗ = nscss_color_ਮੰਜ਼ਿਲ_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->ਖਾਨੇ_ਗਿਣਤੀ = ਖਾਨੇ_ਗਿਣਤੀ;
	f->ਪੰਕਤੀਆਂ = ਪੰਕਤੀਆਂ;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ਬਣਤਰ content_html_ਚੌਖਟੇ,
								(ਪੰਕਤੀਆਂ * ਖਾਨੇ_ਗਿਣਤੀ));

	talloc_ਸੈੱਟ_destructor(f->children, box_ਚੌਖਟੇ_talloc_destructor);

	ਲਈ (ਪੰਕਤੀ = 0; ਪੰਕਤੀ < ਪੰਕਤੀਆਂ; ਪੰਕਤੀ++) {
		ਲਈ (ਖਾਨਾ = 0; ਖਾਨਾ < ਖਾਨੇ_ਗਿਣਤੀ; ਖਾਨਾ++) {
			index = (ਪੰਕਤੀ * ਖਾਨੇ_ਗਿਣਤੀ) + ਖਾਨਾ;
			ਚੌਖਟਾ = &f->children[index];
			ਚੌਖਟਾ->ਖਾਨੇ_ਗਿਣਤੀ = 0;
			ਚੌਖਟਾ->ਪੰਕਤੀਆਂ = 0;
			ਚੌਖਟਾ->ਚੌੜਾਈ = ਖਾਨਾ_ਚੌੜਾਈ[ਖਾਨਾ];
			ਚੌਖਟਾ->ਉਚਾਈ = ਪੰਕਤੀ_ਉਚਾਈ[ਪੰਕਤੀ];
			ਚੌਖਟਾ->margin_ਚੌੜਾਈ = 0;
			ਚੌਖਟਾ->margin_ਉਚਾਈ = 0;
			ਚੌਖਟਾ->name = ਨਿਰਾ;
			ਚੌਖਟਾ->url = ਨਿਰਾ;
			ਚੌਖਟਾ->no_resize = false;
			ਚੌਖਟਾ->scrolling = BW_SCROLLING_AUTO;
			ਚੌਖਟਾ->border = default_border;
			ਚੌਖਟਾ->border_ਰੰਗ = default_border_ਰੰਗ;
			ਚੌਖਟਾ->children = ਨਿਰਾ;
		}
	}
	ਮੁਕਤ(ਖਾਨਾ_ਚੌੜਾਈ);
	ਮੁਕਤ(ਪੰਕਤੀ_ਉਚਾਈ);

	/* create the frameset windows */
	err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(n, &c);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਲਈ (ਪੰਕਤੀ = 0; c != ਨਿਰਾ && ਪੰਕਤੀ < ਪੰਕਤੀਆਂ; ਪੰਕਤੀ++) {
		ਲਈ (ਖਾਨਾ = 0; c != ਨਿਰਾ && ਖਾਨਾ < ਖਾਨੇ_ਗਿਣਤੀ; ਖਾਨਾ++) {
			ਜਦੋਂ (c != ਨਿਰਾ) {
				dom_ਗੰਢ_type type;
				dom_string *name;

				err = dom_ਗੰਢ_get_ਗੰਢ_type(c, &type);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(c);
					ਵਾਪਸ false;
				}

				err = dom_ਗੰਢ_get_ਗੰਢ_name(c, &name);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(c);
					ਵਾਪਸ false;
				}

				ਜੇ (type != DOM_ELEMENT_ਗੰਢ ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_ਚੌਖਟਾ) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c,
							&ਅਗਲਾ);
					ਜੇ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ਗੰਢ_unref(c);
						ਵਾਪਸ false;
					}

					dom_string_unref(name);
					dom_ਗੰਢ_unref(c);
					c = ਅਗਲਾ;
				} ਨਹੀਂਤਾਂ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ਰੋਕੋ;
				}
			}

			ਜੇ (c == ਨਿਰਾ)
				ਰੋਕੋ;

			/* get current frame */
			index = (ਪੰਕਤੀ * ਖਾਨੇ_ਗਿਣਤੀ) + ਖਾਨਾ;
			ਚੌਖਟਾ = &f->children[index];

			/* nest framesets */
			err = dom_ਗੰਢ_get_ਗੰਢ_name(c, &s);
			ਜੇ (err != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				ਚੌਖਟਾ->border = 0;
				ਜੇ (box_create_frameset(ਚੌਖਟਾ, c,
						content) == false) {
					dom_ਗੰਢ_unref(c);
					ਵਾਪਸ false;
				}

				err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c, &ਅਗਲਾ);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(c);
					ਵਾਪਸ false;
				}

				dom_ਗੰਢ_unref(c);
				c = ਅਗਲਾ;
				ਜਾਰੀ;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ਨਿਰਾ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				box_ਕੱਢੋ_link(content, s, content->ਅਧਾਰ_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ਜੇ (url != ਨਿਰਾ) {
				/* no self-references */
				ਜੇ (nsurl_compare(content->ਅਧਾਰ_url, url,
						NSURL_COMPLETE) == false)
					ਚੌਖਟਾ->url = url;
				url = ਨਿਰਾ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				ਚੌਖਟਾ->name = talloc_strdup(content->bctx,
						dom_string_ਅੰਕੜੇ(s));
				dom_string_unref(s);
			}

			ਜੇ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &ਚੌਖਟਾ->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				ਚੌਖਟਾ->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				i = atoi(dom_string_ਅੰਕੜੇ(s));
				ਚੌਖਟਾ->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				ਜੇ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					ਚੌਖਟਾ->scrolling = BW_SCROLLING_YES;
				ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					ਚੌਖਟਾ->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				ਚੌਖਟਾ->margin_ਚੌੜਾਈ = atoi(dom_string_ਅੰਕੜੇ(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				ਚੌਖਟਾ->margin_ਉਚਾਈ = atoi(dom_string_ਅੰਕੜੇ(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				css_color color;

				ਜੇ (nscss_parse_ਰੰਗ(dom_string_ਅੰਕੜੇ(s),
						&color))
					ਚੌਖਟਾ->border_ਰੰਗ =
						nscss_color_ਮੰਜ਼ਿਲ_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c, &ਅਗਲਾ);
			ਜੇ (err != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			dom_ਗੰਢ_unref(c);
			c = ਅਗਲਾ;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ਜੇ (c != ਨਿਰਾ) {
		dom_ਗੰਢ_unref(c);
	}

	ਵਾਪਸ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ਸਥਿਰ ਪੂਰਨ box_iframes_talloc_destructor(ਬਣਤਰ content_html_iframe *f)
{
	ਜੇ (f->url != ਨਿਰਾ) {
		nsurl_unref(f->url);
		f->url = ਨਿਰਾ;
	}

	ਵਾਪਸ 0;
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
ਸਥਿਰ bool
box_get_attribute(dom_ਗੰਢ *n,
		  ਅਚਲ ਅੱਖਰ *attribute,
		  ਖਾਲੀ *context,
		  ਅੱਖਰ **ਮੁੱਲ)
{
	ਅੱਖਰ *ਨਤੀਜਾ;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ਅਚਲ uint8_t *) attribute,
			ਲੜੀ_ਲੰਬਾਈ(attribute), &attr_name);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ਜੇ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ਵਾਪਸ false;
	}

	dom_string_unref(attr_name);

	ਜੇ (attr != ਨਿਰਾ) {
		ਨਤੀਜਾ = talloc_strdup(context, dom_string_ਅੰਕੜੇ(attr));

		dom_string_unref(attr);

		ਜੇ (ਨਤੀਜਾ == ਨਿਰਾ)
			ਵਾਪਸ false;

		*ਮੁੱਲ = ਨਤੀਜਾ;
	}

	ਵਾਪਸ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ਸਥਿਰ bool
box_ਨਿਵੇਸ਼_ਪਾਠ(html_content *html, ਬਣਤਰ box *box, ਬਣਤਰ dom_ਗੰਢ *ਗੰਢ)
{
	ਬਣਤਰ box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	ਚੋਣ (display) {
	ਹਾਲਤ CSS_DISPLAY_GRID:
	ਹਾਲਤ CSS_DISPLAY_FLEX:
	ਹਾਲਤ CSS_DISPLAY_ਖੰਡ:
		box->type = BOX_ਖੰਡ;
		ਰੋਕੋ;
	ਮੂਲ:
		box->type = BOX_INLINE_ਖੰਡ;
		ਰੋਕੋ;
	}

	inline_container = box_create(ਨਿਰਾ, 0, false, 0, 0, 0, 0, html->bctx);
	ਜੇ (!inline_container)
		ਵਾਪਸ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ਨਿਰਾ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ਜੇ (!inline_box)
		ਵਾਪਸ false;
	inline_box->type = BOX_ਪਾਠ;
	inline_box->ਪਾਠ = talloc_strdup(html->bctx, "");

	box_ਜੋੜੋ_ਬੱਚਾ(inline_container, inline_box);
	box_ਜੋੜੋ_ਬੱਚਾ(box, inline_container);

	ਵਾਪਸ box_textarea_create_textarea(html, box, ਗੰਢ);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ਸਥਿਰ bool box_select_ਜੋੜੋ_option(ਬਣਤਰ form_control *control, dom_ਗੰਢ *n)
{
	ਅੱਖਰ *ਮੁੱਲ = ਨਿਰਾ;
	ਅੱਖਰ *ਪਾਠ = ਨਿਰਾ;
	ਅੱਖਰ *ਪਾਠ_nowrap = ਨਿਰਾ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ਗੰਢ_get_ਪਾਠ_content(n, &content);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜੇ (content != ਨਿਰਾ) {
		ਪਾਠ = squash_whitespace(dom_string_ਅੰਕੜੇ(content));
		dom_string_unref(content);
	} ਨਹੀਂਤਾਂ {
		ਪਾਠ = strdup("");
	}

	ਜੇ (ਪਾਠ == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	err = dom_element_get_attribute(n, corestring_dom_ਮੁੱਲ, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਮੁੱਲ = strdup(dom_string_ਅੰਕੜੇ(s));
		dom_string_unref(s);
	} ਨਹੀਂਤਾਂ {
		ਮੁੱਲ = strdup(ਪਾਠ);
	}

	ਜੇ (ਮੁੱਲ == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	ਜੇ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	ਪਾਠ_nowrap = cnv_space2nbsp(ਪਾਠ);
	ਜੇ (ਪਾਠ_nowrap == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	ਜੇ (form_ਜੋੜੋ_option(control, ਮੁੱਲ, ਪਾਠ_nowrap, selected, n) == false)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	ਮੁਕਤ(ਪਾਠ);

	ਵਾਪਸ true;

no_ਸਮ੍ਰਿਤੀ:
	ਮੁਕਤ(ਮੁੱਲ);
	ਮੁਕਤ(ਪਾਠ);
	ਮੁਕਤ(ਪਾਠ_nowrap);
	ਵਾਪਸ false;
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
ਸਥਿਰ bool
box_a(dom_ਗੰਢ *n,
      html_content *content,
      ਬਣਤਰ box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ok = box_ਕੱਢੋ_link(content, s, content->ਅਧਾਰ_url, &url);
		dom_string_unref(s);
		ਜੇ (!ok)
			ਵਾਪਸ false;
		ਜੇ (url) {
			ਜੇ (box->href != ਨਿਰਾ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ਜੇ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_ਨਿਸ਼ਾਨਾ, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->ਨਿਸ਼ਾਨਾ = "_blank";
		ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ਸਿਖਰ_2))
			box->ਨਿਸ਼ਾਨਾ = "_top";
		ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ਮਾਪਾ))
			box->ਨਿਸ਼ਾਨਾ = "_parent";
		ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->ਨਿਸ਼ਾਨਾ = "_self";
		ਨਹੀਂਤਾਂ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->ਨਿਸ਼ਾਨਾ = talloc_strdup(content->bctx,
					dom_string_ਅੰਕੜੇ(s));
			ਜੇ (!box->ਨਿਸ਼ਾਨਾ) {
				dom_string_unref(s);
				ਵਾਪਸ false;
			}
		}
		dom_string_unref(s);
	}

	ਵਾਪਸ true;
}


/**
 * Document body special element handler [7.5.1].
 */
ਸਥਿਰ bool
box_body(dom_ਗੰਢ *n,
	 html_content *content,
	 ਬਣਤਰ box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ਜੇ (nscss_color_is_transparent(color)) {
		content->background_ਰੰਗ = NS_TRANSPARENT;
	} ਨਹੀਂਤਾਂ {
		content->background_ਰੰਗ = nscss_color_ਮੰਜ਼ਿਲ_ns(color);
	}

	ਵਾਪਸ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ਸਥਿਰ bool
box_br(dom_ਗੰਢ *n,
       html_content *content,
       ਬਣਤਰ box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ਵਾਪਸ true;
}


/**
 * special element handler for Push button [17.5].
 */
ਸਥਿਰ bool
box_button(dom_ਗੰਢ *n,
	   html_content *content,
	   ਬਣਤਰ box *box,
	   bool *convert_children)
{
	ਬਣਤਰ form_control *gadget;

	gadget = html_forms_get_control_for_ਗੰਢ(content->forms, n);
	ਜੇ (!gadget)
		ਵਾਪਸ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_ਖੰਡ;

	/* Just render the contents */

	ਵਾਪਸ true;
}


/**
 * Canvas element
 */
ਸਥਿਰ bool
box_canvas(dom_ਗੰਢ *n,
	     html_content *content,
	     ਬਣਤਰ box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ਜੇ (!content->enable_scripting) {
		ਵਾਪਸ true;
	}
	*convert_children = false;

	ਜੇ (box->style && ns_computed_display(box->style,
			box_is_ਜੜ੍ਹ(n)) == CSS_DISPLAY_NONE)
		ਵਾਪਸ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ਵਾਪਸ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ਸਥਿਰ bool
box_embed(dom_ਗੰਢ *n,
	  html_content *content,
	  ਬਣਤਰ box *box,
	  bool *convert_children)
{
	ਬਣਤਰ object_params *params;
	ਬਣਤਰ object_param *param;
	dom_namednodemap *attrs;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ idx;
	uint32_t ਅੰਕ_attrs;
	dom_string *src;
	dom_exception err;

	ਜੇ (box->style && ns_computed_display(box->style,
			box_is_ਜੜ੍ਹ(n)) == CSS_DISPLAY_NONE)
		ਵਾਪਸ true;

	params = talloc(content->bctx, ਬਣਤਰ object_params);
	ਜੇ (params == ਨਿਰਾ)
		ਵਾਪਸ false;

	talloc_ਸੈੱਟ_destructor(params, box_object_talloc_destructor);

	params->ਅੰਕੜੇ = ਨਿਰਾ;
	params->type = ਨਿਰਾ;
	params->codetype = ਨਿਰਾ;
	params->codebase = ਨਿਰਾ;
	params->classid = ਨਿਰਾ;
	params->params = ਨਿਰਾ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ਜੇ (err != DOM_NO_ERR || src == ਨਿਰਾ)
		ਵਾਪਸ true;
	ਜੇ (box_ਕੱਢੋ_link(content, src, content->ਅਧਾਰ_url,
			     &params->ਅੰਕੜੇ) == false) {
		dom_string_unref(src);
		ਵਾਪਸ false;
	}

	dom_string_unref(src);

	ਜੇ (params->ਅੰਕੜੇ == ਨਿਰਾ)
		ਵਾਪਸ true;

	/* Don't include ourself */
	ਜੇ (nsurl_compare(content->ਅਧਾਰ_url, params->ਅੰਕੜੇ, NSURL_COMPLETE))
		ਵਾਪਸ true;

	/* add attributes as parameters to linked list */
	err = dom_ਗੰਢ_get_ਗੁਣ(n, &attrs);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	err = dom_namednodemap_get_ਲੰਮਾਈ(attrs, &ਅੰਕ_attrs);
	ਜੇ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ਵਾਪਸ false;
	}

	ਲਈ (idx = 0; idx < ਅੰਕ_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *ਮੁੱਲ;

		err = dom_namednodemap_ਵਸਤੂ(attrs, idx, (ਖਾਲੀ *) &attr);
		ਜੇ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ਵਾਪਸ false;
		}

		err = dom_attr_get_name(attr, &name);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(attr);
			dom_namednodemap_unref(attrs);
			ਵਾਪਸ false;
		}

		ਜੇ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ਗੰਢ_unref(attr);
			dom_string_unref(name);
			ਜਾਰੀ;
		}

		err = dom_attr_get_ਮੁੱਲ(attr, &ਮੁੱਲ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ਵਾਪਸ false;
		}

		param = talloc(content->bctx, ਬਣਤਰ object_param);
		ਜੇ (param == ਨਿਰਾ) {
			dom_ਗੰਢ_unref(attr);
			dom_string_unref(ਮੁੱਲ);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ਵਾਪਸ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_ਅੰਕੜੇ(name));
		param->ਮੁੱਲ = talloc_strdup(content->bctx, dom_string_ਅੰਕੜੇ(ਮੁੱਲ));
		param->type = ਨਿਰਾ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->ਅਗਲਾ = ਨਿਰਾ;

		dom_string_unref(ਮੁੱਲ);
		dom_string_unref(name);
		dom_ਗੰਢ_unref(attr);

		ਜੇ (param->name == ਨਿਰਾ || param->ਮੁੱਲ == ਨਿਰਾ ||
				param->valuetype == ਨਿਰਾ) {
			dom_namednodemap_unref(attrs);
			ਵਾਪਸ false;
		}

		param->ਅਗਲਾ = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ਵਾਪਸ html_fetch_object(content, params->ਅੰਕੜੇ, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ਸਥਿਰ bool
box_frameset(dom_ਗੰਢ *n,
	     html_content *content,
	     ਬਣਤਰ box *box,
	     bool *convert_children)
{
	bool ok;

	ਜੇ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ਜੇ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ਵਾਪਸ true;
	}

	content->frameset = talloc_zero(content->bctx,
					ਬਣਤਰ content_html_ਚੌਖਟੇ);
	ਜੇ (!content->frameset) {
		ਵਾਪਸ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ਜੇ (ok) {
		box->type = BOX_NONE;
	}

	ਜੇ (convert_children) {
		*convert_children = false;
	}
	ਵਾਪਸ ok;
}


/**
 * Inline subwindow [16.5].
 */
ਸਥਿਰ bool
box_iframe(dom_ਗੰਢ *n,
	   html_content *content,
	   ਬਣਤਰ box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ਬਣਤਰ content_html_iframe *iframe;
	ਪੂਰਨ i;

	ਜੇ (box->style && ns_computed_display(box->style,
			box_is_ਜੜ੍ਹ(n)) == CSS_DISPLAY_NONE)
		ਵਾਪਸ true;

	ਜੇ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ਵਾਪਸ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ਜੇ (err != DOM_NO_ERR || s == ਨਿਰਾ)
		ਵਾਪਸ true;
	ਜੇ (box_ਕੱਢੋ_link(content, s, content->ਅਧਾਰ_url, &url) == false) {
		dom_string_unref(s);
		ਵਾਪਸ false;
	}
	dom_string_unref(s);
	ਜੇ (url == ਨਿਰਾ)
		ਵਾਪਸ true;

	/* don't include ourself */
	ਜੇ (nsurl_compare(content->ਅਧਾਰ_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ਵਾਪਸ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ਬਣਤਰ content_html_iframe);
	ਜੇ (iframe == ਨਿਰਾ) {
		nsurl_unref(url);
		ਵਾਪਸ false;
	}

	talloc_ਸੈੱਟ_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_ਚੌੜਾਈ = 0;
	iframe->margin_ਉਚਾਈ = 0;
	iframe->name = ਨਿਰਾ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->ਅਗਲਾ = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_ਅੰਕੜੇ(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		i = atoi(dom_string_ਅੰਕੜੇ(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		css_color color;

		ਜੇ (nscss_parse_ਰੰਗ(dom_string_ਅੰਕੜੇ(s), &color))
			iframe->border_ਰੰਗ = nscss_color_ਮੰਜ਼ਿਲ_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		iframe->margin_ਚੌੜਾਈ = atoi(dom_string_ਅੰਕੜੇ(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		iframe->margin_ਉਚਾਈ = atoi(dom_string_ਅੰਕੜੇ(s));
		dom_string_unref(s);
	}

	/* box */
	ਪ੍ਰਤਿਜ੍ਞਾ(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ਜੇ (convert_children)
		*convert_children = false;
	ਵਾਪਸ true;
}


/**
 * Embedded image [13.2].
 */
ਸਥਿਰ bool
box_image(dom_ਗੰਢ *n,
	  html_content *content,
	  ਬਣਤਰ box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ਗਿਣਤੀ css_ਚੌੜਾਈ_e wtype;
	ਗਿਣਤੀ css_ਉਚਾਈ_e htype;
	css_fixed ਮੁੱਲ = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ਜੇ (box->style && ns_computed_display(box->style,
			box_is_ਜੜ੍ਹ(n)) == CSS_DISPLAY_NONE)
		ਵਾਪਸ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
		ਅੱਖਰ *alt = squash_whitespace(dom_string_ਅੰਕੜੇ(s));
		dom_string_unref(s);
		ਜੇ (alt == ਨਿਰਾ)
			ਵਾਪਸ false;
		box->ਪਾਠ = talloc_strdup(content->bctx, alt);
		ਮੁਕਤ(alt);
		ਜੇ (box->ਪਾਠ == ਨਿਰਾ)
			ਵਾਪਸ false;
		box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(box->ਪਾਠ);
	}

	ਜੇ (nsoption_bool(foreground_images) == false) {
		ਵਾਪਸ true;
	}

	/* imagemap associated with this image */
	ਜੇ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ਵਾਪਸ false;
	ਜੇ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ਜੇ (err != DOM_NO_ERR || s == ਨਿਰਾ)
		ਵਾਪਸ true;

	ਜੇ (box_ਕੱਢੋ_link(content, s, content->ਅਧਾਰ_url, &url) == false) {
		dom_string_unref(s);
		ਵਾਪਸ false;
	}

	dom_string_unref(s);

	ਜੇ (url == ਨਿਰਾ)
		ਵਾਪਸ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_ਚੌੜਾਈ(box->style, &ਮੁੱਲ, &wunit);
	htype = css_computed_ਉਚਾਈ(box->style, &ਮੁੱਲ, &hunit);

	ਜੇ (wtype == CSS_ਚੌੜਾਈ_ਸੈੱਟ &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_ਉਚਾਈ_ਸੈੱਟ &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ਵਾਪਸ ok;
}


/**
 * Form control [17.4].
 */
ਸਥਿਰ bool
box_ਨਿਵੇਸ਼(dom_ਗੰਢ *n,
	  html_content *content,
	  ਬਣਤਰ box *box,
	  bool *convert_children)
{
	ਬਣਤਰ form_control *gadget;
	dom_string *type = ਨਿਰਾ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ਗੰਢ(content->forms, n);
	ਜੇ (gadget == ਨਿਰਾ) {
		ਵਾਪਸ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ਜੇ ((err != DOM_NO_ERR) || (type == ਨਿਰਾ)) {
		/* no type so "text" is assumed */
		ਜੇ (box_ਨਿਵੇਸ਼_ਪਾਠ(content, box, n) == false) {
			ਵਾਪਸ false;
		}
		*convert_children = false;
		ਵਾਪਸ true;
	}

	ਜੇ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ਜੇ (box_ਨਿਵੇਸ਼_ਪਾਠ(content, box, n) == false)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_ਖੰਡ;

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ਨਹੀਂਤਾਂ ਜੇ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ਬਣਤਰ box *inline_container, *inline_box;

		ਜੇ (box_button(n, content, box, 0) == false)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

		inline_container = box_create(ਨਿਰਾ, 0, false, 0, 0, 0, 0,
				content->bctx);
		ਜੇ (inline_container == ਨਿਰਾ)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ਨਿਰਾ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ਜੇ (inline_box == ਨਿਰਾ)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

		inline_box->type = BOX_ਪਾਠ;

		ਜੇ (box->gadget->ਮੁੱਲ != ਨਿਰਾ)
			inline_box->ਪਾਠ = talloc_strdup(content->bctx,
					box->gadget->ਮੁੱਲ);
		ਨਹੀਂਤਾਂ ਜੇ (box->gadget->type == GADGET_SUBMIT)
			inline_box->ਪਾਠ = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ਨਹੀਂਤਾਂ ਜੇ (box->gadget->type == GADGET_RESET)
			inline_box->ਪਾਠ = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ਨਹੀਂਤਾਂ
			inline_box->ਪਾਠ = talloc_strdup(content->bctx,
							 "Button");

		ਜੇ (inline_box->ਪਾਠ == ਨਿਰਾ)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

		inline_box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(inline_box->ਪਾਠ);

		box_ਜੋੜੋ_ਬੱਚਾ(inline_container, inline_box);

		box_ਜੋੜੋ_ਬੱਚਾ(box, inline_container);

	} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ਜੇ (box->style && ns_computed_display(box->style,
				box_is_ਜੜ੍ਹ(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ਜੇ (err == DOM_NO_ERR && s != ਨਿਰਾ) {
				error = nsurl_join(content->ਅਧਾਰ_url,
						dom_string_ਅੰਕੜੇ(s), &url);
				dom_string_unref(s);
				ਜੇ (error != NSERROR_OK)
					ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ਜੇ (nsurl_compare(url, content->ਅਧਾਰ_url,
						NSURL_COMPLETE) == false) {
					ਜੇ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
					}
				}
				nsurl_unref(url);
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* unhandled type the default is "text" */
		ਜੇ (box_ਨਿਵੇਸ਼_ਪਾਠ(content, box, n) == false)
			ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	}

	dom_string_unref(type);

	*convert_children = false;

	ਵਾਪਸ true;

no_ਸਮ੍ਰਿਤੀ:
	dom_string_unref(type);

	ਵਾਪਸ false;
}


/**
 * Noscript element
 */
ਸਥਿਰ bool
box_noscript(dom_ਗੰਢ *n,
	     html_content *content,
	     ਬਣਤਰ box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ਜੇ (content->enable_scripting) {
		*convert_children = false;
	}

	ਵਾਪਸ true;
}


/**
 * Generic embedded object [13.3].
 */
ਸਥਿਰ bool
box_object(dom_ਗੰਢ *n,
	   html_content *content,
	   ਬਣਤਰ box *box,
	   bool *convert_children)
{
	ਬਣਤਰ object_params *params;
	ਬਣਤਰ object_param *param;
	dom_string *codebase, *classid, *ਅੰਕੜੇ;
	dom_ਗੰਢ *c;
	dom_exception err;

	ਜੇ (box->style && ns_computed_display(box->style,
			box_is_ਜੜ੍ਹ(n)) == CSS_DISPLAY_NONE)
		ਵਾਪਸ true;

	ਜੇ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ਵਾਪਸ false;
	ਜੇ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ਬਣਤਰ object_params);
	ਜੇ (params == ਨਿਰਾ)
		ਵਾਪਸ false;

	talloc_ਸੈੱਟ_destructor(params, box_object_talloc_destructor);

	params->ਅੰਕੜੇ = ਨਿਰਾ;
	params->type = ਨਿਰਾ;
	params->codetype = ਨਿਰਾ;
	params->codebase = ਨਿਰਾ;
	params->classid = ਨਿਰਾ;
	params->params = ਨਿਰਾ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ਜੇ (err == DOM_NO_ERR && codebase != ਨਿਰਾ) {
		ਜੇ (box_ਕੱਢੋ_link(content, codebase,	content->ਅਧਾਰ_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ਵਾਪਸ false;
		}
		dom_string_unref(codebase);
	}
	ਜੇ (params->codebase == ਨਿਰਾ)
		params->codebase = nsurl_ref(content->ਅਧਾਰ_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ਜੇ (err == DOM_NO_ERR && classid != ਨਿਰਾ) {
		ਜੇ (box_ਕੱਢੋ_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ਵਾਪਸ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_ਅੰਕੜੇ, &ਅੰਕੜੇ);
	ਜੇ (err == DOM_NO_ERR && ਅੰਕੜੇ != ਨਿਰਾ) {
		ਜੇ (box_ਕੱਢੋ_link(content, ਅੰਕੜੇ,
				params->codebase, &params->ਅੰਕੜੇ) == false) {
			dom_string_unref(ਅੰਕੜੇ);
			ਵਾਪਸ false;
		}
		dom_string_unref(ਅੰਕੜੇ);
	}

	ਜੇ (params->classid == ਨਿਰਾ && params->ਅੰਕੜੇ == ਨਿਰਾ)
		/* nothing to embed; ignore */
		ਵਾਪਸ true;

	/* Don't include ourself */
	ਜੇ (params->classid != ਨਿਰਾ && nsurl_compare(content->ਅਧਾਰ_url,
			params->classid, NSURL_COMPLETE))
		ਵਾਪਸ true;

	ਜੇ (params->ਅੰਕੜੇ != ਨਿਰਾ && nsurl_compare(content->ਅਧਾਰ_url,
			params->ਅੰਕੜੇ, NSURL_COMPLETE))
		ਵਾਪਸ true;

	/* codetype and type are MIME types */
	ਜੇ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ਵਾਪਸ false;
	ਜੇ (box_get_attribute(n, "type", params, &params->type) == false)
		ਵਾਪਸ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ਜੇ (params->classid != ਨਿਰਾ && params->ਅੰਕੜੇ == ਨਿਰਾ &&
			params->codetype != ਨਿਰਾ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				ਲੜੀ_ਲੰਬਾਈ(params->codetype), &icodetype);
		ਜੇ (lerror != lwc_error_ok)
			ਵਾਪਸ false;

		ਜੇ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ਵਾਪਸ true;
		}

		lwc_string_unref(icodetype);
	}

	ਜੇ (params->ਅੰਕੜੇ != ਨਿਰਾ && params->type != ਨਿਰਾ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, ਲੜੀ_ਲੰਬਾਈ(params->type),
				&itype);
		ਜੇ (lerror != lwc_error_ok)
			ਵਾਪਸ false;

		ਜੇ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ਵਾਪਸ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(n, &c);
	ਜੇ (err != DOM_NO_ERR)
		ਵਾਪਸ false;

	ਜਦੋਂ (c != ਨਿਰਾ) {
		dom_ਗੰਢ *ਅਗਲਾ;
		dom_ਗੰਢ_type type;

		err = dom_ਗੰਢ_get_ਗੰਢ_type(c, &type);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(c);
			ਵਾਪਸ false;
		}

		ਜੇ (type == DOM_ELEMENT_ਗੰਢ) {
			dom_string *name;

			err = dom_ਗੰਢ_get_ਗੰਢ_name(c, &name);
			ਜੇ (err != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ਗੰਢ_unref(c);
				ਰੋਕੋ;
			}
			dom_string_unref(name);

			param = talloc(params, ਬਣਤਰ object_param);
			ਜੇ (param == ਨਿਰਾ) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}
			param->name = ਨਿਰਾ;
			param->ਮੁੱਲ = ਨਿਰਾ;
			param->type = ਨਿਰਾ;
			param->valuetype = ਨਿਰਾ;
			param->ਅਗਲਾ = ਨਿਰਾ;

			ਜੇ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (box_get_attribute(c, "value", param,
					&param->ਮੁੱਲ) == false) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ਗੰਢ_unref(c);
				ਵਾਪਸ false;
			}

			ਜੇ (param->valuetype == ਨਿਰਾ) {
				param->valuetype = talloc_strdup(param, "data");
				ਜੇ (param->valuetype == ਨਿਰਾ) {
					dom_ਗੰਢ_unref(c);
					ਵਾਪਸ false;
				}
			}

			param->ਅਗਲਾ = params->params;
			params->params = param;
		}

		err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c, &ਅਗਲਾ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(c);
			ਵਾਪਸ false;
		}

		dom_ਗੰਢ_unref(c);
		c = ਅਗਲਾ;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ਜੇ (!html_fetch_object(content,
			       params->ਅੰਕੜੇ ? params->ਅੰਕੜੇ : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ਵਾਪਸ false;

	*convert_children = false;
	ਵਾਪਸ true;
}


/**
 * Preformatted text [9.3.4].
 */
ਸਥਿਰ bool
box_pre(dom_ਗੰਢ *n,
	html_content *content,
	ਬਣਤਰ box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ਵਾਪਸ true;
}


/**
 * Option selector [17.6].
 */
ਸਥਿਰ bool
box_select(dom_ਗੰਢ *n,
	   html_content *content,
	   ਬਣਤਰ box *box,
	   bool *convert_children)
{
	ਬਣਤਰ box *inline_container;
	ਬਣਤਰ box *inline_box;
	ਬਣਤਰ form_control *gadget;
	dom_ਗੰਢ *c, *c2;
	dom_ਗੰਢ *ਅਗਲਾ, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ਗੰਢ(content->forms, n);
	ਜੇ (gadget == ਨਿਰਾ)
		ਵਾਪਸ false;

	gadget->html = content;
	err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(n, &c);
	ਜੇ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ਵਾਪਸ false;
	}

	ਜਦੋਂ (c != ਨਿਰਾ) {
		dom_string *name;

		err = dom_ਗੰਢ_get_ਗੰਢ_name(c, &name);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(c);
			form_free_control(gadget);
			ਵਾਪਸ false;
		}

		ਜੇ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ਜੇ (box_select_ਜੋੜੋ_option(gadget, c) == false) {
				dom_ਗੰਢ_unref(c);
				form_free_control(gadget);
				ਵਾਪਸ false;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ਗੰਢ_get_ਪਹਿਲਾ_ਬੱਚਾ(c, &c2);
			ਜੇ (err != DOM_NO_ERR) {
				dom_ਗੰਢ_unref(c);
				form_free_control(gadget);
				ਵਾਪਸ false;
			}

			ਜਦੋਂ (c2 != ਨਿਰਾ) {
				dom_string *c2_name;

				err = dom_ਗੰਢ_get_ਗੰਢ_name(c2, &c2_name);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(c2);
					dom_ਗੰਢ_unref(c);
					form_free_control(gadget);
					ਵਾਪਸ false;
				}

				ਜੇ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ਜੇ (box_select_ਜੋੜੋ_option(gadget,
							c2) == false) {
						dom_ਗੰਢ_unref(c2);
						dom_ਗੰਢ_unref(c);
						form_free_control(gadget);
						ਵਾਪਸ false;
					}
				} ਨਹੀਂਤਾਂ {
					dom_string_unref(c2_name);
				}

				err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c2, &next2);
				ਜੇ (err != DOM_NO_ERR) {
					dom_ਗੰਢ_unref(c2);
					dom_ਗੰਢ_unref(c);
					form_free_control(gadget);
					ਵਾਪਸ false;
				}

				dom_ਗੰਢ_unref(c2);
				c2 = next2;
			}
		} ਨਹੀਂਤਾਂ {
			dom_string_unref(name);
		}

		err = dom_ਗੰਢ_get_ਅਗਲਾ_sibling(c, &ਅਗਲਾ);
		ਜੇ (err != DOM_NO_ERR) {
			dom_ਗੰਢ_unref(c);
			form_free_control(gadget);
			ਵਾਪਸ false;
		}

		dom_ਗੰਢ_unref(c);
		c = ਅਗਲਾ;
	}

	ਜੇ (gadget->ਅੰਕੜੇ.select.ਅੰਕ_ਚੀਜ਼ਾਂ == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ਵਾਪਸ true;
	}

	box->type = BOX_INLINE_ਖੰਡ;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ਨਿਰਾ, 0, false, 0, 0, 0, 0, content->bctx);
	ਜੇ (inline_container == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ਨਿਰਾ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ਜੇ (inline_box == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;
	inline_box->type = BOX_ਪਾਠ;
	box_ਜੋੜੋ_ਬੱਚਾ(inline_container, inline_box);
	box_ਜੋੜੋ_ਬੱਚਾ(box, inline_container);

	ਜੇ (gadget->ਅੰਕੜੇ.select.ਗੁਣਜ == false &&
			gadget->ਅੰਕੜੇ.select.ਅੰਕ_selected == 0) {
		gadget->ਅੰਕੜੇ.select.current = gadget->ਅੰਕੜੇ.select.ਚੀਜ਼ਾਂ;
		gadget->ਅੰਕੜੇ.select.current->initial_selected =
			gadget->ਅੰਕੜੇ.select.current->selected = true;
		gadget->ਅੰਕੜੇ.select.ਅੰਕ_selected = 1;
		dom_html_option_element_ਸੈੱਟ_selected(
				gadget->ਅੰਕੜੇ.select.current->ਗੰਢ, true);
	}

	ਜੇ (gadget->ਅੰਕੜੇ.select.ਅੰਕ_selected == 0)
		inline_box->ਪਾਠ = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ਨਹੀਂਤਾਂ ਜੇ (gadget->ਅੰਕੜੇ.select.ਅੰਕ_selected == 1)
		inline_box->ਪਾਠ = talloc_strdup(content->bctx,
				gadget->ਅੰਕੜੇ.select.current->ਪਾਠ);
	ਨਹੀਂਤਾਂ
		inline_box->ਪਾਠ = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ਜੇ (inline_box->ਪਾਠ == ਨਿਰਾ)
		ਜਾਓ no_ਸਮ੍ਰਿਤੀ;

	inline_box->ਲੰਮਾਈ = ਲੜੀ_ਲੰਬਾਈ(inline_box->ਪਾਠ);

	*convert_children = false;
	ਵਾਪਸ true;

no_ਸਮ੍ਰਿਤੀ:
	ਵਾਪਸ false;
}


/**
 * Multi-line text field [17.7].
 */
ਸਥਿਰ bool box_textarea(dom_ਗੰਢ *n,
			html_content *content,
			ਬਣਤਰ box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ਗੰਢ(content->forms, n);
	ਜੇ (box->gadget == ਨਿਰਾ)
		ਵਾਪਸ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ਜੇ (!box_ਨਿਵੇਸ਼_ਪਾਠ(content, box, n))
		ਵਾਪਸ false;

	*convert_children = false;
	ਵਾਪਸ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ਗੰਢ *ਗੰਢ,
			 html_content *content,
			 ਬਣਤਰ box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type ਟੈਗ_type;
	bool res;

	exc = dom_html_element_get_ਟੈਗ_type(ਗੰਢ, &ਟੈਗ_type);
	ਜੇ (exc != DOM_NO_ERR) {
		ਟੈਗ_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ਚੋਣ (ਟੈਗ_type) {
	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_ਨਿਵੇਸ਼:
		res = box_ਨਿਵੇਸ਼(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਹਾਲਤ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ਗੰਢ, content, box, convert_children);
		ਰੋਕੋ;

	ਮੂਲ:
		res = true;
	}

	ਵਾਪਸ res;
}
