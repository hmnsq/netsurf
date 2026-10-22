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

#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ "utils/nsoption.h"
#ಅನ್ತರ್ಭಾವ "utils/corestrings.h"
#ಅನ್ತರ್ಭಾವ "utils/log.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/talloc.h"
#ಅನ್ತರ್ಭಾವ "utils/string.h"
#ಅನ್ತರ್ಭಾವ "utils/ascii.h"
#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"
#ಅನ್ತರ್ಭಾವ "css/hints.h"
#ಅನ್ತರ್ಭಾವ "desktop/frame_types.h"
#ಅನ್ತರ್ಭಾವ "content/content_factory.h"

#ಅನ್ತರ್ಭಾವ "html/html.h"
#ಅನ್ತರ್ಭಾವ "html/private.h"
#ಅನ್ತರ್ಭಾವ "html/object.h"
#ಅನ್ತರ್ಭಾವ "html/box.h"
#ಅನ್ತರ್ಭಾವ "html/box_manipulate.h"
#ಅನ್ತರ್ಭಾವ "html/box_construct.h"
#ಅನ್ತರ್ಭಾವ "html/box_special.h"
#ಅನ್ತರ್ಭಾವ "html/box_textarea.h"
#ಅನ್ತರ್ಭಾವ "html/form_internal.h"


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ content_type image_types = CONTENT_IMAGE;


/**
 * determine if a box is the root node
 *
 * \param n node to check
 * \return true if node is root else false.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool box_is_ಬೇರು(dom_ಗಂಟು *n)
{
	dom_ಗಂಟು *ಪೋಷಕ;
	dom_ಗಂಟು_type type;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಪೋಷಕ_ಗಂಟು(n, &ಪೋಷಕ);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ಪೋಷಕ != ಖಾಲಿ) {
		err = dom_ಗಂಟು_get_ಗಂಟು_type(ಪೋಷಕ, &type);

		dom_ಗಂಟು_unref(ಪೋಷಕ);

		ಆದರೆ (err != DOM_NO_ERR)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (type != DOM_DOCUMENT_ಗಂಟು)
			ಹಿಂತಿರುಗಿ false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Destructor for object_params, for &lt;object&gt; elements
 *
 * \param o  The object params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ box_object_talloc_destructor(ರಚನೆ object_params *o)
{
	ಆದರೆ (o->codebase != ಖಾಲಿ)
		nsurl_unref(o->codebase);
	ಆದರೆ (o->classid != ಖಾಲಿ)
		nsurl_unref(o->classid);
	ಆದರೆ (o->ದತ್ತಾಂಶ != ಖಾಲಿ)
		nsurl_unref(o->ದತ್ತಾಂಶ);

	ಹಿಂತಿರುಗಿ 0;
}


/**
 * Parse a multi-length-list, as defined by HTML 4.01.
 *
 * \param ds dom string to parse
 * \param count updated to number of entries
 * \return array of struct box_multi_length, or 0 on memory exhaustion
 */
ಸ್ಥಿರ ರಚನೆ ಚೌಕಟ್ಟು_dimension *
box_parse_multi_lengths(ಸ್ಥಿರಾಂಕ dom_string *ds, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *ಎಣಿಕೆ_2)
{
	ಅಕ್ಷರ *ಕೊನೆ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, n;
	ರಚನೆ ಚೌಕಟ್ಟು_dimension *ಉದ್ದಳತೆ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;

	s = dom_string_ದತ್ತಾಂಶ(ds);

	ಗಾಗಿ (i = 0, n = 1; s[i]; i++)
		ಆದರೆ (s[i] == ',')
			n++;

	ಉದ್ದಳತೆ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(n, ಗಾತ್ರ(ರಚನೆ ಚೌಕಟ್ಟು_dimension));
	ಆದರೆ (!ಉದ್ದಳತೆ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಗಾಗಿ (i = 0; i != n; i++) {
		ಆಗ (ascii_is_space(*s)) {
			s++;
		}
		ಉದ್ದಳತೆ[i].ಮೌಲ್ಯ = strtof(s, &ಕೊನೆ);
		ಆದರೆ (ಉದ್ದಳತೆ[i].ಮೌಲ್ಯ <= 0) {
			ಉದ್ದಳತೆ[i].ಮೌಲ್ಯ = 1;
		}
		s = ಕೊನೆ;
		ಆಯ್ಕೆ (*s) {
			ಸ್ಥಿತಿ '%':
				ಉದ್ದಳತೆ[i].unit = ಚೌಕಟ್ಟು_DIMENSION_PERCENT;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '*':
				ಉದ್ದಳತೆ[i].unit = ಚೌಕಟ್ಟು_DIMENSION_RELATIVE;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ಉದ್ದಳತೆ[i].unit = ಚೌಕಟ್ಟು_DIMENSION_PIXELS;
				ನಿಲ್ಲಿಸು;
		}
		ಆಗ (*s && *s != ',') {
			s++;
		}
		ಆದರೆ (*s == ',') {
			s++;
		}
	}

	*ಎಣಿಕೆ_2 = n;
	ಹಿಂತಿರುಗಿ ಉದ್ದಳತೆ;
}


/**
 * Destructor for content_html_frames, for frame elements
 *
 * \param f  The frame params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ box_ಚೌಕಟ್ಟುಗಳು_talloc_destructor(ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *f)
{
	ಆದರೆ (f->url != ಖಾಲಿ) {
		nsurl_unref(f->url);
		f->url = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ 0;
}


/**
 * create a frameset box tree
 */
ಸ್ಥಿರ bool
box_create_frameset(ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *f,
		    dom_ಗಂಟು *n,
		    html_content *content)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲು, ಲಂಬಸಾಲು, index, i;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಡ್ಡಸಾಲುಗಳು = 1, ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 1;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ರಚನೆ ಚೌಕಟ್ಟು_dimension *ಅಡ್ಡಸಾಲು_ಎತ್ತರ = 0, *ಲಂಬಸಾಲು_ಅಗಲ = 0;
	dom_ಗಂಟು *c, *ಮುಂದಿನ;
	ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು *ಚೌಕಟ್ಟು;
	bool default_border = true;
	ಬಣ್ಣ default_border_ಬಣ್ಣ = 0x000000;

	/* parse rows and columns */
	err = dom_element_get_attribute(n, corestring_dom_ಅಡ್ಡಸಾಲುಗಳು, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಅಡ್ಡಸಾಲು_ಎತ್ತರ = box_parse_multi_lengths(s, &ಅಡ್ಡಸಾಲುಗಳು);
		dom_string_unref(s);
		ಆದರೆ (ಅಡ್ಡಸಾಲು_ಎತ್ತರ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಅಡ್ಡಸಾಲು_ಎತ್ತರ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ಚೌಕಟ್ಟು_dimension));
		ಆದರೆ (ಅಡ್ಡಸಾಲು_ಎತ್ತರ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;
		ಅಡ್ಡಸಾಲು_ಎತ್ತರ->ಮೌಲ್ಯ = 100;
		ಅಡ್ಡಸಾಲು_ಎತ್ತರ->unit = ಚೌಕಟ್ಟು_DIMENSION_PERCENT;
	}

	err = dom_element_get_attribute(n, corestring_dom_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಲಂಬಸಾಲು_ಅಗಲ = box_parse_multi_lengths(s, &ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ);
		dom_string_unref(s);
		ಆದರೆ (ಲಂಬಸಾಲು_ಅಗಲ == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಎತ್ತರ);
			ಹಿಂತಿರುಗಿ false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಲಂಬಸಾಲು_ಅಗಲ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ಚೌಕಟ್ಟು_dimension));
		ಆದರೆ (ಲಂಬಸಾಲು_ಅಗಲ == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಎತ್ತರ);
			ಹಿಂತಿರುಗಿ false;
		}
		ಲಂಬಸಾಲು_ಅಗಲ->ಮೌಲ್ಯ = 100;
		ಲಂಬಸಾಲು_ಅಗಲ->unit = ಚೌಕಟ್ಟು_DIMENSION_PERCENT;
	}

	/* common extension: border="0|1" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_border, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಆದರೆ ((dom_string_ದತ್ತಾಂಶ(s)[0] == '0') &&
				(dom_string_ದತ್ತಾಂಶ(s)[1] == '\0'))
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: frameborder="yes|no" to control all children */
	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no) == 0)
			default_border = false;
		dom_string_unref(s);
	}

	/* common extension: bordercolor="#RRGGBB|<named colour>" to control
	 *all children */
	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		css_color color;

		ಆದರೆ (nscss_parse_ಬಣ್ಣ(dom_string_ದತ್ತಾಂಶ(s), &color))
			default_border_ಬಣ್ಣ = nscss_color_ಗಮ್ಯ_ns(color);

		dom_string_unref(s);
	}

	/* update frameset and create default children */
	f->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
	f->ಅಡ್ಡಸಾಲುಗಳು = ಅಡ್ಡಸಾಲುಗಳು;
	f->scrolling = BW_SCROLLING_NO;
	f->children = talloc_array(content->bctx, ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು,
								(ಅಡ್ಡಸಾಲುಗಳು * ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ));

	talloc_ಗಣ_destructor(f->children, box_ಚೌಕಟ್ಟುಗಳು_talloc_destructor);

	ಗಾಗಿ (ಅಡ್ಡಸಾಲು = 0; ಅಡ್ಡಸಾಲು < ಅಡ್ಡಸಾಲುಗಳು; ಅಡ್ಡಸಾಲು++) {
		ಗಾಗಿ (ಲಂಬಸಾಲು = 0; ಲಂಬಸಾಲು < ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ; ಲಂಬಸಾಲು++) {
			index = (ಅಡ್ಡಸಾಲು * ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) + ಲಂಬಸಾಲು;
			ಚೌಕಟ್ಟು = &f->children[index];
			ಚೌಕಟ್ಟು->ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = 0;
			ಚೌಕಟ್ಟು->ಅಡ್ಡಸಾಲುಗಳು = 0;
			ಚೌಕಟ್ಟು->ಅಗಲ = ಲಂಬಸಾಲು_ಅಗಲ[ಲಂಬಸಾಲು];
			ಚೌಕಟ್ಟು->ಎತ್ತರ = ಅಡ್ಡಸಾಲು_ಎತ್ತರ[ಅಡ್ಡಸಾಲು];
			ಚೌಕಟ್ಟು->margin_ಅಗಲ = 0;
			ಚೌಕಟ್ಟು->margin_ಎತ್ತರ = 0;
			ಚೌಕಟ್ಟು->name = ಖಾಲಿ;
			ಚೌಕಟ್ಟು->url = ಖಾಲಿ;
			ಚೌಕಟ್ಟು->no_resize = false;
			ಚೌಕಟ್ಟು->scrolling = BW_SCROLLING_AUTO;
			ಚೌಕಟ್ಟು->border = default_border;
			ಚೌಕಟ್ಟು->border_ಬಣ್ಣ = default_border_ಬಣ್ಣ;
			ಚೌಕಟ್ಟು->children = ಖಾಲಿ;
		}
	}
	ಬಿಡುಗಡೆ(ಲಂಬಸಾಲು_ಅಗಲ);
	ಬಿಡುಗಡೆ(ಅಡ್ಡಸಾಲು_ಎತ್ತರ);

	/* create the frameset windows */
	err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(n, &c);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಗಾಗಿ (ಅಡ್ಡಸಾಲು = 0; c != ಖಾಲಿ && ಅಡ್ಡಸಾಲು < ಅಡ್ಡಸಾಲುಗಳು; ಅಡ್ಡಸಾಲು++) {
		ಗಾಗಿ (ಲಂಬಸಾಲು = 0; c != ಖಾಲಿ && ಲಂಬಸಾಲು < ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ; ಲಂಬಸಾಲು++) {
			ಆಗ (c != ಖಾಲಿ) {
				dom_ಗಂಟು_type type;
				dom_string *name;

				err = dom_ಗಂಟು_get_ಗಂಟು_type(c, &type);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಹಿಂತಿರುಗಿ false;
				}

				err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಹಿಂತಿರುಗಿ false;
				}

				ಆದರೆ (type != DOM_ELEMENT_ಗಂಟು ||
					(!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_ಚೌಕಟ್ಟು) &&
					!dom_string_caseless_lwc_isequal(
							name,
							corestring_lwc_frameset
							))) {
					err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c,
							&ಮುಂದಿನ);
					ಆದರೆ (err != DOM_NO_ERR) {
						dom_string_unref(name);
						dom_ಗಂಟು_unref(c);
						ಹಿಂತಿರುಗಿ false;
					}

					dom_string_unref(name);
					dom_ಗಂಟು_unref(c);
					c = ಮುಂದಿನ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* Got a FRAME or FRAMESET element */
					dom_string_unref(name);
					ನಿಲ್ಲಿಸು;
				}
			}

			ಆದರೆ (c == ಖಾಲಿ)
				ನಿಲ್ಲಿಸು;

			/* get current frame */
			index = (ಅಡ್ಡಸಾಲು * ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) + ಲಂಬಸಾಲು;
			ಚೌಕಟ್ಟು = &f->children[index];

			/* nest framesets */
			err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &s);
			ಆದರೆ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (dom_string_caseless_lwc_isequal(s,
					corestring_lwc_frameset)) {
				dom_string_unref(s);
				ಚೌಕಟ್ಟು->border = 0;
				ಆದರೆ (box_create_frameset(ಚೌಕಟ್ಟು, c,
						content) == false) {
					dom_ಗಂಟು_unref(c);
					ಹಿಂತಿರುಗಿ false;
				}

				err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c, &ಮುಂದಿನ);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c);
					ಹಿಂತಿರುಗಿ false;
				}

				dom_ಗಂಟು_unref(c);
				c = ಮುಂದಿನ;
				ಮುಂದುವರಿ;
			}

			dom_string_unref(s);

			/* get frame URL (not required) */
			url = ಖಾಲಿ;
			err = dom_element_get_attribute(c, corestring_dom_src, &s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				box_ಹೊರತೆಗೆ_link(content, s, content->ತಳ_url,
						 &url);
				dom_string_unref(s);
			}

			/* copy url */
			ಆದರೆ (url != ಖಾಲಿ) {
				/* no self-references */
				ಆದರೆ (nsurl_compare(content->ತಳ_url, url,
						NSURL_COMPLETE) == false)
					ಚೌಕಟ್ಟು->url = url;
				url = ಖಾಲಿ;
			}

			/* fill in specified values */
			err = dom_element_get_attribute(c, corestring_dom_name, &s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				ಚೌಕಟ್ಟು->name = talloc_strdup(content->bctx,
						dom_string_ದತ್ತಾಂಶ(s));
				dom_string_unref(s);
			}

			ಆದರೆ (dom_element_has_attribute(c, corestring_dom_noresize,
						      &ಚೌಕಟ್ಟು->no_resize) != DOM_NO_ERR) {
				/* If we can't read the attribute for some reason,
				 * assume we didn't have it.
				 */
				ಚೌಕಟ್ಟು->no_resize = false;
			}

			err = dom_element_get_attribute(c, corestring_dom_frameborder,
					&s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				i = atoi(dom_string_ದತ್ತಾಂಶ(s));
				ಚೌಕಟ್ಟು->border = (i != 0);
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_scrolling, &s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				ಆದರೆ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_yes))
					ಚೌಕಟ್ಟು->scrolling = BW_SCROLLING_YES;
				ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(s,
						corestring_lwc_no))
					ಚೌಕಟ್ಟು->scrolling = BW_SCROLLING_NO;
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginwidth,
					&s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				ಚೌಕಟ್ಟು->margin_ಅಗಲ = atoi(dom_string_ದತ್ತಾಂಶ(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_marginheight,
					&s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				ಚೌಕಟ್ಟು->margin_ಎತ್ತರ = atoi(dom_string_ದತ್ತಾಂಶ(s));
				dom_string_unref(s);
			}

			err = dom_element_get_attribute(c, corestring_dom_bordercolor,
					&s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				css_color color;

				ಆದರೆ (nscss_parse_ಬಣ್ಣ(dom_string_ದತ್ತಾಂಶ(s),
						&color))
					ಚೌಕಟ್ಟು->border_ಬಣ್ಣ =
						nscss_color_ಗಮ್ಯ_ns(color);

				dom_string_unref(s);
			}

			/* advance */
			err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c, &ಮುಂದಿನ);
			ಆದರೆ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			dom_ಗಂಟು_unref(c);
			c = ಮುಂದಿನ;
		}
	}

	/* If the last child wasn't a frame, we still need to unref it */
	ಆದರೆ (c != ಖಾಲಿ) {
		dom_ಗಂಟು_unref(c);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Destructor for content_html_iframe, for &lt;iframe&gt; elements
 *
 * \param f The iframe params being destroyed.
 * \return 0 to allow talloc to continue destroying the tree.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ box_iframes_talloc_destructor(ರಚನೆ content_html_iframe *f)
{
	ಆದರೆ (f->url != ಖಾಲಿ) {
		nsurl_unref(f->url);
		f->url = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ bool
box_get_attribute(dom_ಗಂಟು *n,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attribute,
		  ಶೂನ್ಯ *context,
		  ಅಕ್ಷರ **ಮೌಲ್ಯ)
{
	ಅಕ್ಷರ *ಫಲಿತಾಂಶ;
	dom_string *attr, *attr_name;
	dom_exception err;

	err = dom_string_create_interned((ಸ್ಥಿರಾಂಕ uint8_t *) attribute,
			ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(attribute), &attr_name);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	err = dom_element_get_attribute(n, attr_name, &attr);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_string_unref(attr_name);
		ಹಿಂತಿರುಗಿ false;
	}

	dom_string_unref(attr_name);

	ಆದರೆ (attr != ಖಾಲಿ) {
		ಫಲಿತಾಂಶ = talloc_strdup(context, dom_string_ದತ್ತಾಂಶ(attr));

		dom_string_unref(attr);

		ಆದರೆ (ಫಲಿತಾಂಶ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;

		*ಮೌಲ್ಯ = ಫಲಿತಾಂಶ;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Helper function for adding textarea widget to box.
 *
 * This is a load of hacks to ensure boxes replaced with textareas
 * can be handled by the layout code.
 */
ಸ್ಥಿರ bool
box_ಒಳಹರಿವು_ಪಠ್ಯ(html_content *html, ರಚನೆ box *box, ರಚನೆ dom_ಗಂಟು *ಗಂಟು)
{
	ರಚನೆ box *inline_container, *inline_box;
	uint8_t display = css_computed_display_static(box->style);

	ಆಯ್ಕೆ (display) {
	ಸ್ಥಿತಿ CSS_DISPLAY_GRID:
	ಸ್ಥಿತಿ CSS_DISPLAY_FLEX:
	ಸ್ಥಿತಿ CSS_DISPLAY_ಖಂಡ:
		box->type = BOX_ಖಂಡ;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		box->type = BOX_INLINE_ಖಂಡ;
		ನಿಲ್ಲಿಸು;
	}

	inline_container = box_create(ಖಾಲಿ, 0, false, 0, 0, 0, 0, html->bctx);
	ಆದರೆ (!inline_container)
		ಹಿಂತಿರುಗಿ false;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ಖಾಲಿ, box->style, false, 0, 0, box->title, 0,
			html->bctx);
	ಆದರೆ (!inline_box)
		ಹಿಂತಿರುಗಿ false;
	inline_box->type = BOX_ಪಠ್ಯ;
	inline_box->ಪಠ್ಯ = talloc_strdup(html->bctx, "");

	box_ಕೂಡಿಸು_ಮಗು(inline_container, inline_box);
	box_ಕೂಡಿಸು_ಮಗು(box, inline_container);

	ಹಿಂತಿರುಗಿ box_textarea_create_textarea(html, box, ಗಂಟು);
}


/**
 * Add an option to a form select control (helper function for box_select()).
 *
 * \param  control  select containing the &lt;option&gt;
 * \param  n	    xml element node for &lt;option&gt;
 * \return  true on success, false on memory exhaustion
 */
ಸ್ಥಿರ bool box_select_ಕೂಡಿಸು_option(ರಚನೆ form_control *control, dom_ಗಂಟು *n)
{
	ಅಕ್ಷರ *ಮೌಲ್ಯ = ಖಾಲಿ;
	ಅಕ್ಷರ *ಪಠ್ಯ = ಖಾಲಿ;
	ಅಕ್ಷರ *ಪಠ್ಯ_nowrap = ಖಾಲಿ;
	bool selected;
	dom_string *content, *s;
	dom_exception err;

	err = dom_ಗಂಟು_get_ಪಠ್ಯ_content(n, &content);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (content != ಖಾಲಿ) {
		ಪಠ್ಯ = squash_whitespace(dom_string_ದತ್ತಾಂಶ(content));
		dom_string_unref(content);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪಠ್ಯ = strdup("");
	}

	ಆದರೆ (ಪಠ್ಯ == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	err = dom_element_get_attribute(n, corestring_dom_ಮೌಲ್ಯ, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಮೌಲ್ಯ = strdup(dom_string_ದತ್ತಾಂಶ(s));
		dom_string_unref(s);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಮೌಲ್ಯ = strdup(ಪಠ್ಯ);
	}

	ಆದರೆ (ಮೌಲ್ಯ == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	ಆದರೆ (dom_element_has_attribute(n, corestring_dom_selected, &selected) != DOM_NO_ERR) {
		/* Assume not selected if we can't read the attribute presence */
		selected = false;
	}

	/* replace spaces/TABs with hard spaces to prevent line wrapping */
	ಪಠ್ಯ_nowrap = cnv_space2nbsp(ಪಠ್ಯ);
	ಆದರೆ (ಪಠ್ಯ_nowrap == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	ಆದರೆ (form_ಕೂಡಿಸು_option(control, ಮೌಲ್ಯ, ಪಠ್ಯ_nowrap, selected, n) == false)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	ಬಿಡುಗಡೆ(ಪಠ್ಯ);

	ಹಿಂತಿರುಗಿ true;

no_ಸ್ಮರಣೆ:
	ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(ಪಠ್ಯ);
	ಬಿಡುಗಡೆ(ಪಠ್ಯ_nowrap);
	ಹಿಂತಿರುಗಿ false;
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
ಸ್ಥಿರ bool
box_a(dom_ಗಂಟು *n,
      html_content *content,
      ರಚನೆ box *box,
      bool *convert_children)
{
	bool ok;
	nsurl *url;
	dom_string *s;
	dom_exception err;

	err = dom_element_get_attribute(n, corestring_dom_href, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ok = box_ಹೊರತೆಗೆ_link(content, s, content->ತಳ_url, &url);
		dom_string_unref(s);
		ಆದರೆ (!ok)
			ಹಿಂತಿರುಗಿ false;
		ಆದರೆ (url) {
			ಆದರೆ (box->href != ಖಾಲಿ)
				nsurl_unref(box->href);
			box->href = url;
		}
	}

	/* name and id share the same namespace */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		lwc_string *lwc_name;

		err = dom_string_intern(s, &lwc_name);

		dom_string_unref(s);

		ಆದರೆ (err == DOM_NO_ERR) {
			/* name replaces existing id
			 * TODO: really? */
			lwc_string_unref(box->id);
			box->id = lwc_name;
		}
	}

	/* target frame [16.3] */
	err = dom_element_get_attribute(n, corestring_dom_ಗುರಿ, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__blank))
			box->ಗುರಿ = "_blank";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ತುದಿ_2))
			box->ಗುರಿ = "_top";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_ಪೋಷಕ))
			box->ಗುರಿ = "_parent";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc__self))
			/* the default may have been overridden by a
			 * <base target=...>, so this is different to 0 */
			box->ಗುರಿ = "_self";
		ಇಲ್ಲದಿದ್ದರೆ {
			/* 6.16 says that frame names must begin with [a-zA-Z]
			 * This doesn't match reality, so just take anything */
			box->ಗುರಿ = talloc_strdup(content->bctx,
					dom_string_ದತ್ತಾಂಶ(s));
			ಆದರೆ (!box->ಗುರಿ) {
				dom_string_unref(s);
				ಹಿಂತಿರುಗಿ false;
			}
		}
		dom_string_unref(s);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Document body special element handler [7.5.1].
 */
ಸ್ಥಿರ bool
box_body(dom_ಗಂಟು *n,
	 html_content *content,
	 ರಚನೆ box *box,
	 bool *convert_children)
{
	css_color color;

	css_computed_background_color(box->style, &color);
	ಆದರೆ (nscss_color_is_transparent(color)) {
		content->background_ಬಣ್ಣ = NS_TRANSPARENT;
	} ಇಲ್ಲದಿದ್ದರೆ {
		content->background_ಬಣ್ಣ = nscss_color_ಗಮ್ಯ_ns(color);
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * special element handler for forced line break [9.3.2].
 */
ಸ್ಥಿರ bool
box_br(dom_ಗಂಟು *n,
       html_content *content,
       ರಚನೆ box *box,
       bool *convert_children)
{
	box->type = BOX_BR;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * special element handler for Push button [17.5].
 */
ಸ್ಥಿರ bool
box_button(dom_ಗಂಟು *n,
	   html_content *content,
	   ರಚನೆ box *box,
	   bool *convert_children)
{
	ರಚನೆ form_control *gadget;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಆದರೆ (!gadget)
		ಹಿಂತಿರುಗಿ false;

	gadget->html = content;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	box->type = BOX_INLINE_ಖಂಡ;

	/* Just render the contents */

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Canvas element
 */
ಸ್ಥಿರ bool
box_canvas(dom_ಗಂಟು *n,
	     html_content *content,
	     ರಚನೆ box *box,
	     bool *convert_children)
{
	/* If scripting is not enabled display the contents of canvas */
	ಆದರೆ (!content->enable_scripting) {
		ಹಿಂತಿರುಗಿ true;
	}
	*convert_children = false;

	ಆದರೆ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಹಿಂತಿರುಗಿ true;

	/* This is replaced content */
	box->flags |= IS_REPLACED | REPLACE_DIM;

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Embedded object (not in any HTML specification:
 * see http://wp.netscape.com/assist/net_sites/new_html3_prop.html )
 */
ಸ್ಥಿರ bool
box_embed(dom_ಗಂಟು *n,
	  html_content *content,
	  ರಚನೆ box *box,
	  bool *convert_children)
{
	ರಚನೆ object_params *params;
	ರಚನೆ object_param *param;
	dom_namednodemap *attrs;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ idx;
	uint32_t ಅಂಕೆ_attrs;
	dom_string *src;
	dom_exception err;

	ಆದರೆ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಹಿಂತಿರುಗಿ true;

	params = talloc(content->bctx, ರಚನೆ object_params);
	ಆದರೆ (params == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	talloc_ಗಣ_destructor(params, box_object_talloc_destructor);

	params->ದತ್ತಾಂಶ = ಖಾಲಿ;
	params->type = ಖಾಲಿ;
	params->codetype = ಖಾಲಿ;
	params->codebase = ಖಾಲಿ;
	params->classid = ಖಾಲಿ;
	params->params = ಖಾಲಿ;

	/* src is a URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &src);
	ಆದರೆ (err != DOM_NO_ERR || src == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;
	ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, src, content->ತಳ_url,
			     &params->ದತ್ತಾಂಶ) == false) {
		dom_string_unref(src);
		ಹಿಂತಿರುಗಿ false;
	}

	dom_string_unref(src);

	ಆದರೆ (params->ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	/* Don't include ourself */
	ಆದರೆ (nsurl_compare(content->ತಳ_url, params->ದತ್ತಾಂಶ, NSURL_COMPLETE))
		ಹಿಂತಿರುಗಿ true;

	/* add attributes as parameters to linked list */
	err = dom_ಗಂಟು_get_ಗುಣಲಕ್ಷಣಗಳು(n, &attrs);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	err = dom_namednodemap_get_ಉದ್ದಳತೆ(attrs, &ಅಂಕೆ_attrs);
	ಆದರೆ (err != DOM_NO_ERR) {
		dom_namednodemap_unref(attrs);
		ಹಿಂತಿರುಗಿ false;
	}

	ಗಾಗಿ (idx = 0; idx < ಅಂಕೆ_attrs; idx++) {
		dom_attr *attr;
		dom_string *name, *ಮೌಲ್ಯ;

		err = dom_namednodemap_ಅಂಶ(attrs, idx, (ಶೂನ್ಯ *) &attr);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_namednodemap_unref(attrs);
			ಹಿಂತಿರುಗಿ false;
		}

		err = dom_attr_get_name(attr, &name);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(attr);
			dom_namednodemap_unref(attrs);
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (dom_string_caseless_lwc_isequal(name, corestring_lwc_src)) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(name);
			ಮುಂದುವರಿ;
		}

		err = dom_attr_get_ಮೌಲ್ಯ(attr, &ಮೌಲ್ಯ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ಹಿಂತಿರುಗಿ false;
		}

		param = talloc(content->bctx, ರಚನೆ object_param);
		ಆದರೆ (param == ಖಾಲಿ) {
			dom_ಗಂಟು_unref(attr);
			dom_string_unref(ಮೌಲ್ಯ);
			dom_string_unref(name);
			dom_namednodemap_unref(attrs);
			ಹಿಂತಿರುಗಿ false;
		}

		param->name = talloc_strdup(content->bctx, dom_string_ದತ್ತಾಂಶ(name));
		param->ಮೌಲ್ಯ = talloc_strdup(content->bctx, dom_string_ದತ್ತಾಂಶ(ಮೌಲ್ಯ));
		param->type = ಖಾಲಿ;
		param->valuetype = talloc_strdup(content->bctx, "data");
		param->ಮುಂದಿನ = ಖಾಲಿ;

		dom_string_unref(ಮೌಲ್ಯ);
		dom_string_unref(name);
		dom_ಗಂಟು_unref(attr);

		ಆದರೆ (param->name == ಖಾಲಿ || param->ಮೌಲ್ಯ == ಖಾಲಿ ||
				param->valuetype == ಖಾಲಿ) {
			dom_namednodemap_unref(attrs);
			ಹಿಂತಿರುಗಿ false;
		}

		param->ಮುಂದಿನ = params->params;
		params->params = param;
	}

	dom_namednodemap_unref(attrs);

	box->object_params = params;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ಹಿಂತಿರುಗಿ html_fetch_object(content, params->ದತ್ತಾಂಶ, box, CONTENT_ANY, false);
}


/**
 * Window subdivision [16.2.1].
 */
ಸ್ಥಿರ bool
box_frameset(dom_ಗಂಟು *n,
	     html_content *content,
	     ರಚನೆ box *box,
	     bool *convert_children)
{
	bool ok;

	ಆದರೆ (content->frameset) {
		NSLOG(netsurf, INFO, "Error: multiple framesets in document.");
		/* Don't convert children */
		ಆದರೆ (convert_children)
			*convert_children = false;
		/* And ignore this spurious frameset */
		box->type = BOX_NONE;
		ಹಿಂತಿರುಗಿ true;
	}

	content->frameset = talloc_zero(content->bctx,
					ರಚನೆ content_html_ಚೌಕಟ್ಟುಗಳು);
	ಆದರೆ (!content->frameset) {
		ಹಿಂತಿರುಗಿ false;
	}

	ok = box_create_frameset(content->frameset, n, content);
	ಆದರೆ (ok) {
		box->type = BOX_NONE;
	}

	ಆದರೆ (convert_children) {
		*convert_children = false;
	}
	ಹಿಂತಿರುಗಿ ok;
}


/**
 * Inline subwindow [16.5].
 */
ಸ್ಥಿರ bool
box_iframe(dom_ಗಂಟು *n,
	   html_content *content,
	   ರಚನೆ box *box,
	   bool *convert_children)
{
	nsurl *url;
	dom_string *s;
	dom_exception err;
	ರಚನೆ content_html_iframe *iframe;
	ಪೂರ್ಣಾಂಕ i;

	ಆದರೆ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (box->style &&
	    css_computed_visibility(box->style) == CSS_VISIBILITY_HIDDEN) {
		/* Don't create iframe discriptors for invisible iframes
		 * TODO: handle hidden iframes at browser_window generation
		 * time instead? */
		ಹಿಂತಿರುಗಿ true;
	}

	/* get frame URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ಆದರೆ (err != DOM_NO_ERR || s == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;
	ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, s, content->ತಳ_url, &url) == false) {
		dom_string_unref(s);
		ಹಿಂತಿರುಗಿ false;
	}
	dom_string_unref(s);
	ಆದರೆ (url == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	/* don't include ourself */
	ಆದರೆ (nsurl_compare(content->ತಳ_url, url, NSURL_COMPLETE)) {
		nsurl_unref(url);
		ಹಿಂತಿರುಗಿ true;
	}

	/* create a new iframe */
	iframe = talloc(content->bctx, ರಚನೆ content_html_iframe);
	ಆದರೆ (iframe == ಖಾಲಿ) {
		nsurl_unref(url);
		ಹಿಂತಿರುಗಿ false;
	}

	talloc_ಗಣ_destructor(iframe, box_iframes_talloc_destructor);

	iframe->box = box;
	iframe->margin_ಅಗಲ = 0;
	iframe->margin_ಎತ್ತರ = 0;
	iframe->name = ಖಾಲಿ;
	iframe->url = url;
	iframe->scrolling = BW_SCROLLING_AUTO;
	iframe->border = true;

	/* Add this iframe to the linked list of iframes */
	iframe->ಮುಂದಿನ = content->iframe;
	content->iframe = iframe;

	/* fill in specified values */
	err = dom_element_get_attribute(n, corestring_dom_name, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		iframe->name = talloc_strdup(content->bctx, dom_string_ದತ್ತಾಂಶ(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_frameborder, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		i = atoi(dom_string_ದತ್ತಾಂಶ(s));
		iframe->border = (i != 0);
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_bordercolor, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		css_color color;

		ಆದರೆ (nscss_parse_ಬಣ್ಣ(dom_string_ದತ್ತಾಂಶ(s), &color))
			iframe->border_ಬಣ್ಣ = nscss_color_ಗಮ್ಯ_ns(color);

		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_scrolling, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_yes))
			iframe->scrolling = BW_SCROLLING_YES;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(s,
				corestring_lwc_no))
			iframe->scrolling = BW_SCROLLING_NO;
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginwidth, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		iframe->margin_ಅಗಲ = atoi(dom_string_ದತ್ತಾಂಶ(s));
		dom_string_unref(s);
	}

	err = dom_element_get_attribute(n, corestring_dom_marginheight, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		iframe->margin_ಎತ್ತರ = atoi(dom_string_ದತ್ತಾಂಶ(s));
		dom_string_unref(s);
	}

	/* box */
	ಪ್ರತಿಜ್ಞಾ(box->style);
	box->flags |= IFRAME;
	box->flags |= IS_REPLACED;

	/* Showing iframe, so don't show alternate content */
	ಆದರೆ (convert_children)
		*convert_children = false;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Embedded image [13.2].
 */
ಸ್ಥಿರ bool
box_image(dom_ಗಂಟು *n,
	  html_content *content,
	  ರಚನೆ box *box,
	  bool *convert_children)
{
	bool ok;
	dom_string *s;
	dom_exception err;
	nsurl *url;
	ಎಣಿಕೆ css_ಅಗಲ_e wtype;
	ಎಣಿಕೆ css_ಎತ್ತರ_e htype;
	css_fixed ಮೌಲ್ಯ = 0;
	css_unit wunit = CSS_UNIT_PX;
	css_unit hunit = CSS_UNIT_PX;

	ಆದರೆ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಹಿಂತಿರುಗಿ true;

	/* handle alt text */
	err = dom_element_get_attribute(n, corestring_dom_alt, &s);
	ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
		ಅಕ್ಷರ *alt = squash_whitespace(dom_string_ದತ್ತಾಂಶ(s));
		dom_string_unref(s);
		ಆದರೆ (alt == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;
		box->ಪಠ್ಯ = talloc_strdup(content->bctx, alt);
		ಬಿಡುಗಡೆ(alt);
		ಆದರೆ (box->ಪಠ್ಯ == ಖಾಲಿ)
			ಹಿಂತಿರುಗಿ false;
		box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(box->ಪಠ್ಯ);
	}

	ಆದರೆ (nsoption_bool(foreground_images) == false) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* imagemap associated with this image */
	ಆದರೆ (!box_get_attribute(n, "usemap", content->bctx, &box->usemap))
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	/* get image URL */
	err = dom_element_get_attribute(n, corestring_dom_src, &s);
	ಆದರೆ (err != DOM_NO_ERR || s == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, s, content->ತಳ_url, &url) == false) {
		dom_string_unref(s);
		ಹಿಂತಿರುಗಿ false;
	}

	dom_string_unref(s);

	ಆದರೆ (url == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ true;

	/* start fetch */
	box->flags |= IS_REPLACED;
	ok = html_fetch_object(content, url, box, image_types, false);
	nsurl_unref(url);

	wtype = css_computed_ಅಗಲ(box->style, &ಮೌಲ್ಯ, &wunit);
	htype = css_computed_ಎತ್ತರ(box->style, &ಮೌಲ್ಯ, &hunit);

	ಆದರೆ (wtype == CSS_ಅಗಲ_ಗಣ &&
	    wunit != CSS_UNIT_PCT &&
	    htype == CSS_ಎತ್ತರ_ಗಣ &&
	    hunit != CSS_UNIT_PCT) {
		/* We know the dimensions the image will be shown at
		 * before it's fetched. */
		box->flags |= REPLACE_DIM;
	}

	ಹಿಂತಿರುಗಿ ok;
}


/**
 * Form control [17.4].
 */
ಸ್ಥಿರ bool
box_ಒಳಹರಿವು(dom_ಗಂಟು *n,
	  html_content *content,
	  ರಚನೆ box *box,
	  bool *convert_children)
{
	ರಚನೆ form_control *gadget;
	dom_string *type = ಖಾಲಿ;
	dom_exception err;
	nsurl *url;
	nserror error;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಆದರೆ (gadget == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ false;
	}

	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;
	gadget->html = content;

	/* get entry type */
	err = dom_element_get_attribute(n, corestring_dom_type, &type);
	ಆದರೆ ((err != DOM_NO_ERR) || (type == ಖಾಲಿ)) {
		/* no type so "text" is assumed */
		ಆದರೆ (box_ಒಳಹರಿವು_ಪಠ್ಯ(content, box, n) == false) {
			ಹಿಂತಿರುಗಿ false;
		}
		*convert_children = false;
		ಹಿಂತಿರುಗಿ true;
	}

	ಆದರೆ (dom_string_caseless_lwc_isequal(type, corestring_lwc_password)) {
		ಆದರೆ (box_ಒಳಹರಿವು_ಪಠ್ಯ(content, box, n) == false)
			ಹೋಗು no_ಸ್ಮರಣೆ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(type, corestring_lwc_file)) {
		box->type = BOX_INLINE_ಖಂಡ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(type,
			corestring_lwc_hidden)) {
		/* no box for hidden inputs */
		box->type = BOX_NONE;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((dom_string_caseless_lwc_isequal(type,
				corestring_lwc_checkbox) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_radio))) {

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(type,
				corestring_lwc_submit) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_reset) ||
			dom_string_caseless_lwc_isequal(type,
				corestring_lwc_button)) {
		ರಚನೆ box *inline_container, *inline_box;

		ಆದರೆ (box_button(n, content, box, 0) == false)
			ಹೋಗು no_ಸ್ಮರಣೆ;

		inline_container = box_create(ಖಾಲಿ, 0, false, 0, 0, 0, 0,
				content->bctx);
		ಆದರೆ (inline_container == ಖಾಲಿ)
			ಹೋಗು no_ಸ್ಮರಣೆ;

		inline_container->type = BOX_INLINE_CONTAINER;

		inline_box = box_create(ಖಾಲಿ, box->style, false, 0, 0,
				box->title, 0, content->bctx);
		ಆದರೆ (inline_box == ಖಾಲಿ)
			ಹೋಗು no_ಸ್ಮರಣೆ;

		inline_box->type = BOX_ಪಠ್ಯ;

		ಆದರೆ (box->gadget->ಮೌಲ್ಯ != ಖಾಲಿ)
			inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
					box->gadget->ಮೌಲ್ಯ);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget->type == GADGET_SUBMIT)
			inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
					messages_get("Form_Submit"));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (box->gadget->type == GADGET_RESET)
			inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
					messages_get("Form_Reset"));
		ಇಲ್ಲದಿದ್ದರೆ
			inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
							 "Button");

		ಆದರೆ (inline_box->ಪಠ್ಯ == ಖಾಲಿ)
			ಹೋಗು no_ಸ್ಮರಣೆ;

		inline_box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(inline_box->ಪಠ್ಯ);

		box_ಕೂಡಿಸು_ಮಗು(inline_container, inline_box);

		box_ಕೂಡಿಸು_ಮಗು(box, inline_container);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(type,
						   corestring_lwc_image)) {
		gadget->type = GADGET_IMAGE;

		ಆದರೆ (box->style && ns_computed_display(box->style,
				box_is_ಬೇರು(n)) != CSS_DISPLAY_NONE &&
				nsoption_bool(foreground_images) == true) {
			dom_string *s;

			err = dom_element_get_attribute(n, corestring_dom_src, &s);
			ಆದರೆ (err == DOM_NO_ERR && s != ಖಾಲಿ) {
				error = nsurl_join(content->ತಳ_url,
						dom_string_ದತ್ತಾಂಶ(s), &url);
				dom_string_unref(s);
				ಆದರೆ (error != NSERROR_OK)
					ಹೋಗು no_ಸ್ಮರಣೆ;

				/* if url is equivalent to the parent's url,
				 * we've got infinite inclusion. stop it here
				 */
				ಆದರೆ (nsurl_compare(url, content->ತಳ_url,
						NSURL_COMPLETE) == false) {
					ಆದರೆ (!html_fetch_object(content,
							       url,
							       box,
							       image_types,
							       false)) {
						nsurl_unref(url);
						ಹೋಗು no_ಸ್ಮರಣೆ;
					}
				}
				nsurl_unref(url);
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* unhandled type the default is "text" */
		ಆದರೆ (box_ಒಳಹರಿವು_ಪಠ್ಯ(content, box, n) == false)
			ಹೋಗು no_ಸ್ಮರಣೆ;
	}

	dom_string_unref(type);

	*convert_children = false;

	ಹಿಂತಿರುಗಿ true;

no_ಸ್ಮರಣೆ:
	dom_string_unref(type);

	ಹಿಂತಿರುಗಿ false;
}


/**
 * Noscript element
 */
ಸ್ಥಿರ bool
box_noscript(dom_ಗಂಟು *n,
	     html_content *content,
	     ರಚನೆ box *box,
	     bool *convert_children)
{
	/* If scripting is enabled, do not display the contents of noscript */
	ಆದರೆ (content->enable_scripting) {
		*convert_children = false;
	}

	ಹಿಂತಿರುಗಿ true;
}


/**
 * Generic embedded object [13.3].
 */
ಸ್ಥಿರ bool
box_object(dom_ಗಂಟು *n,
	   html_content *content,
	   ರಚನೆ box *box,
	   bool *convert_children)
{
	ರಚನೆ object_params *params;
	ರಚನೆ object_param *param;
	dom_string *codebase, *classid, *ದತ್ತಾಂಶ;
	dom_ಗಂಟು *c;
	dom_exception err;

	ಆದರೆ (box->style && ns_computed_display(box->style,
			box_is_ಬೇರು(n)) == CSS_DISPLAY_NONE)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (box_get_attribute(n, "usemap", content->bctx, &box->usemap) ==
			false)
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (box->usemap && box->usemap[0] == '#')
		box->usemap++;

	params = talloc(content->bctx, ರಚನೆ object_params);
	ಆದರೆ (params == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	talloc_ಗಣ_destructor(params, box_object_talloc_destructor);

	params->ದತ್ತಾಂಶ = ಖಾಲಿ;
	params->type = ಖಾಲಿ;
	params->codetype = ಖಾಲಿ;
	params->codebase = ಖಾಲಿ;
	params->classid = ಖಾಲಿ;
	params->params = ಖಾಲಿ;

	/* codebase, classid, and data are URLs
	 * (codebase is the base for the other two) */
	err = dom_element_get_attribute(n, corestring_dom_codebase, &codebase);
	ಆದರೆ (err == DOM_NO_ERR && codebase != ಖಾಲಿ) {
		ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, codebase,	content->ತಳ_url,
				&params->codebase) == false) {
			dom_string_unref(codebase);
			ಹಿಂತಿರುಗಿ false;
		}
		dom_string_unref(codebase);
	}
	ಆದರೆ (params->codebase == ಖಾಲಿ)
		params->codebase = nsurl_ref(content->ತಳ_url);

	err = dom_element_get_attribute(n, corestring_dom_classid, &classid);
	ಆದರೆ (err == DOM_NO_ERR && classid != ಖಾಲಿ) {
		ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, classid,
				params->codebase, &params->classid) == false) {
			dom_string_unref(classid);
			ಹಿಂತಿರುಗಿ false;
		}
		dom_string_unref(classid);
	}

	err = dom_element_get_attribute(n, corestring_dom_ದತ್ತಾಂಶ, &ದತ್ತಾಂಶ);
	ಆದರೆ (err == DOM_NO_ERR && ದತ್ತಾಂಶ != ಖಾಲಿ) {
		ಆದರೆ (box_ಹೊರತೆಗೆ_link(content, ದತ್ತಾಂಶ,
				params->codebase, &params->ದತ್ತಾಂಶ) == false) {
			dom_string_unref(ದತ್ತಾಂಶ);
			ಹಿಂತಿರುಗಿ false;
		}
		dom_string_unref(ದತ್ತಾಂಶ);
	}

	ಆದರೆ (params->classid == ಖಾಲಿ && params->ದತ್ತಾಂಶ == ಖಾಲಿ)
		/* nothing to embed; ignore */
		ಹಿಂತಿರುಗಿ true;

	/* Don't include ourself */
	ಆದರೆ (params->classid != ಖಾಲಿ && nsurl_compare(content->ತಳ_url,
			params->classid, NSURL_COMPLETE))
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (params->ದತ್ತಾಂಶ != ಖಾಲಿ && nsurl_compare(content->ತಳ_url,
			params->ದತ್ತಾಂಶ, NSURL_COMPLETE))
		ಹಿಂತಿರುಗಿ true;

	/* codetype and type are MIME types */
	ಆದರೆ (box_get_attribute(n, "codetype", params,
			&params->codetype) == false)
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (box_get_attribute(n, "type", params, &params->type) == false)
		ಹಿಂತಿರುಗಿ false;

	/* classid && !data => classid is used (consult codetype)
	 * (classid || !classid) && data => data is used (consult type)
	 * !classid && !data => invalid; ignored */

	ಆದರೆ (params->classid != ಖಾಲಿ && params->ದತ್ತಾಂಶ == ಖಾಲಿ &&
			params->codetype != ಖಾಲಿ) {
		lwc_string *icodetype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->codetype,
				ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(params->codetype), &icodetype);
		ಆದರೆ (lerror != lwc_error_ok)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (content_factory_type_from_mime_type(icodetype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(icodetype);
			ಹಿಂತಿರುಗಿ true;
		}

		lwc_string_unref(icodetype);
	}

	ಆದರೆ (params->ದತ್ತಾಂಶ != ಖಾಲಿ && params->type != ಖಾಲಿ) {
		lwc_string *itype;
		lwc_error lerror;

		lerror = lwc_intern_string(params->type, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(params->type),
				&itype);
		ಆದರೆ (lerror != lwc_error_ok)
			ಹಿಂತಿರುಗಿ false;

		ಆದರೆ (content_factory_type_from_mime_type(itype) ==
				CONTENT_NONE) {
			/* can't handle this MIME type */
			lwc_string_unref(itype);
			ಹಿಂತಿರುಗಿ true;
		}

		lwc_string_unref(itype);
	}

	/* add parameters to linked list */
	err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(n, &c);
	ಆದರೆ (err != DOM_NO_ERR)
		ಹಿಂತಿರುಗಿ false;

	ಆಗ (c != ಖಾಲಿ) {
		dom_ಗಂಟು *ಮುಂದಿನ;
		dom_ಗಂಟು_type type;

		err = dom_ಗಂಟು_get_ಗಂಟು_type(c, &type);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (type == DOM_ELEMENT_ಗಂಟು) {
			dom_string *name;

			err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
			ಆದರೆ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (!dom_string_caseless_lwc_isequal(name,
					corestring_lwc_param)) {
				/* The first non-param child is the start of
				 * the alt html. Therefore, we should break
				 * out of this loop. */
				dom_string_unref(name);
				dom_ಗಂಟು_unref(c);
				ನಿಲ್ಲಿಸು;
			}
			dom_string_unref(name);

			param = talloc(params, ರಚನೆ object_param);
			ಆದರೆ (param == ಖಾಲಿ) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}
			param->name = ಖಾಲಿ;
			param->ಮೌಲ್ಯ = ಖಾಲಿ;
			param->type = ಖಾಲಿ;
			param->valuetype = ಖಾಲಿ;
			param->ಮುಂದಿನ = ಖಾಲಿ;

			ಆದರೆ (box_get_attribute(c, "name", param,
					&param->name) == false) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (box_get_attribute(c, "value", param,
					&param->ಮೌಲ್ಯ) == false) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (box_get_attribute(c, "type", param,
					&param->type) == false) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (box_get_attribute(c, "valuetype", param,
					&param->valuetype) == false) {
				dom_ಗಂಟು_unref(c);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆದರೆ (param->valuetype == ಖಾಲಿ) {
				param->valuetype = talloc_strdup(param, "data");
				ಆದರೆ (param->valuetype == ಖಾಲಿ) {
					dom_ಗಂಟು_unref(c);
					ಹಿಂತಿರುಗಿ false;
				}
			}

			param->ಮುಂದಿನ = params->params;
			params->params = param;
		}

		err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c, &ಮುಂದಿನ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			ಹಿಂತಿರುಗಿ false;
		}

		dom_ಗಂಟು_unref(c);
		c = ಮುಂದಿನ;
	}

	box->object_params = params;

	/* start fetch (MIME type is ok or not specified) */
	box->flags |= IS_REPLACED;
	ಆದರೆ (!html_fetch_object(content,
			       params->ದತ್ತಾಂಶ ? params->ದತ್ತಾಂಶ : params->classid,
			       box,
			       CONTENT_ANY,
			       false))
		ಹಿಂತಿರುಗಿ false;

	*convert_children = false;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Preformatted text [9.3.4].
 */
ಸ್ಥಿರ bool
box_pre(dom_ಗಂಟು *n,
	html_content *content,
	ರಚನೆ box *box,
	bool *convert_children)
{
	box->flags |= PRE_STRIP;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * Option selector [17.6].
 */
ಸ್ಥಿರ bool
box_select(dom_ಗಂಟು *n,
	   html_content *content,
	   ರಚನೆ box *box,
	   bool *convert_children)
{
	ರಚನೆ box *inline_container;
	ರಚನೆ box *inline_box;
	ರಚನೆ form_control *gadget;
	dom_ಗಂಟು *c, *c2;
	dom_ಗಂಟು *ಮುಂದಿನ, *next2;
	dom_exception err;

	gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಆದರೆ (gadget == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	gadget->html = content;
	err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(n, &c);
	ಆದರೆ (err != DOM_NO_ERR) {
		form_free_control(gadget);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆಗ (c != ಖಾಲಿ) {
		dom_string *name;

		err = dom_ಗಂಟು_get_ಗಂಟು_name(c, &name);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			form_free_control(gadget);
			ಹಿಂತಿರುಗಿ false;
		}

		ಆದರೆ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_option)) {
			dom_string_unref(name);

			ಆದರೆ (box_select_ಕೂಡಿಸು_option(gadget, c) == false) {
				dom_ಗಂಟು_unref(c);
				form_free_control(gadget);
				ಹಿಂತಿರುಗಿ false;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (dom_string_caseless_lwc_isequal(name,
				corestring_lwc_optgroup)) {
			dom_string_unref(name);

			err = dom_ಗಂಟು_get_ಮೊದಲ_ಮಗು(c, &c2);
			ಆದರೆ (err != DOM_NO_ERR) {
				dom_ಗಂಟು_unref(c);
				form_free_control(gadget);
				ಹಿಂತಿರುಗಿ false;
			}

			ಆಗ (c2 != ಖಾಲಿ) {
				dom_string *c2_name;

				err = dom_ಗಂಟು_get_ಗಂಟು_name(c2, &c2_name);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c2);
					dom_ಗಂಟು_unref(c);
					form_free_control(gadget);
					ಹಿಂತಿರುಗಿ false;
				}

				ಆದರೆ (dom_string_caseless_lwc_isequal(c2_name,
						corestring_lwc_option)) {
					dom_string_unref(c2_name);

					ಆದರೆ (box_select_ಕೂಡಿಸು_option(gadget,
							c2) == false) {
						dom_ಗಂಟು_unref(c2);
						dom_ಗಂಟು_unref(c);
						form_free_control(gadget);
						ಹಿಂತಿರುಗಿ false;
					}
				} ಇಲ್ಲದಿದ್ದರೆ {
					dom_string_unref(c2_name);
				}

				err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c2, &next2);
				ಆದರೆ (err != DOM_NO_ERR) {
					dom_ಗಂಟು_unref(c2);
					dom_ಗಂಟು_unref(c);
					form_free_control(gadget);
					ಹಿಂತಿರುಗಿ false;
				}

				dom_ಗಂಟು_unref(c2);
				c2 = next2;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			dom_string_unref(name);
		}

		err = dom_ಗಂಟು_get_ಮುಂದಿನ_sibling(c, &ಮುಂದಿನ);
		ಆದರೆ (err != DOM_NO_ERR) {
			dom_ಗಂಟು_unref(c);
			form_free_control(gadget);
			ಹಿಂತಿರುಗಿ false;
		}

		dom_ಗಂಟು_unref(c);
		c = ಮುಂದಿನ;
	}

	ಆದರೆ (gadget->ದತ್ತಾಂಶ.select.ಅಂಕೆ_ವಸ್ತುಗಳು == 0) {
		/* no options: ignore entire select */
		form_free_control(gadget);
		ಹಿಂತಿರುಗಿ true;
	}

	box->type = BOX_INLINE_ಖಂಡ;
	box->gadget = gadget;
	box->flags |= IS_REPLACED;
	gadget->box = box;

	inline_container = box_create(ಖಾಲಿ, 0, false, 0, 0, 0, 0, content->bctx);
	ಆದರೆ (inline_container == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;
	inline_container->type = BOX_INLINE_CONTAINER;
	inline_box = box_create(ಖಾಲಿ, box->style, false, 0, 0, box->title, 0,
			content->bctx);
	ಆದರೆ (inline_box == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;
	inline_box->type = BOX_ಪಠ್ಯ;
	box_ಕೂಡಿಸು_ಮಗು(inline_container, inline_box);
	box_ಕೂಡಿಸು_ಮಗು(box, inline_container);

	ಆದರೆ (gadget->ದತ್ತಾಂಶ.select.ಅಪವರ್ತ್ಯ == false &&
			gadget->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 0) {
		gadget->ದತ್ತಾಂಶ.select.current = gadget->ದತ್ತಾಂಶ.select.ವಸ್ತುಗಳು;
		gadget->ದತ್ತಾಂಶ.select.current->initial_selected =
			gadget->ದತ್ತಾಂಶ.select.current->selected = true;
		gadget->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected = 1;
		dom_html_option_element_ಗಣ_selected(
				gadget->ದತ್ತಾಂಶ.select.current->ಗಂಟು, true);
	}

	ಆದರೆ (gadget->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 0)
		inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
				messages_get("Form_None"));
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (gadget->ದತ್ತಾಂಶ.select.ಅಂಕೆ_selected == 1)
		inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
				gadget->ದತ್ತಾಂಶ.select.current->ಪಠ್ಯ);
	ಇಲ್ಲದಿದ್ದರೆ
		inline_box->ಪಠ್ಯ = talloc_strdup(content->bctx,
				messages_get("Form_Many"));
	ಆದರೆ (inline_box->ಪಠ್ಯ == ಖಾಲಿ)
		ಹೋಗು no_ಸ್ಮರಣೆ;

	inline_box->ಉದ್ದಳತೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(inline_box->ಪಠ್ಯ);

	*convert_children = false;
	ಹಿಂತಿರುಗಿ true;

no_ಸ್ಮರಣೆ:
	ಹಿಂತಿರುಗಿ false;
}


/**
 * Multi-line text field [17.7].
 */
ಸ್ಥಿರ bool box_textarea(dom_ಗಂಟು *n,
			html_content *content,
			ರಚನೆ box *box,
			bool *convert_children)
{
	/* Get the form_control for the DOM node */
	box->gadget = html_forms_get_control_for_ಗಂಟು(content->forms, n);
	ಆದರೆ (box->gadget == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ false;

	box->flags |= IS_REPLACED;
	box->gadget->html = content;
	box->gadget->box = box;

	ಆದರೆ (!box_ಒಳಹರಿವು_ಪಠ್ಯ(content, box, n))
		ಹಿಂತಿರುಗಿ false;

	*convert_children = false;
	ಹಿಂತಿರುಗಿ true;
}


/**
 * \}
 */


/* exported interface documented in html/box_special.h */
bool
convert_special_elements(dom_ಗಂಟು *ಗಂಟು,
			 html_content *content,
			 ರಚನೆ box *box,
			 bool *convert_children)
{
	dom_exception exc;
	dom_html_element_type ಟ್ಯಾಗ್_type;
	bool res;

	exc = dom_html_element_get_ಟ್ಯಾಗ್_type(ಗಂಟು, &ಟ್ಯಾಗ್_type);
	ಆದರೆ (exc != DOM_NO_ERR) {
		ಟ್ಯಾಗ್_type = DOM_HTML_ELEMENT_TYPE__UNKNOWN;
	}

	ಆಯ್ಕೆ (ಟ್ಯಾಗ್_type) {
	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_A:
		res =  box_a(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BODY:
		res = box_body(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BR:
		res = box_br(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_BUTTON:
		res = box_button(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_CANVAS:
		res = box_canvas(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_EMBED:
		res = box_embed(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_FRAMESET:
		res = box_frameset(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IFRAME:
		res = box_iframe(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_IMG:
		res = box_image(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_ಒಳಹರಿವು:
		res = box_ಒಳಹರಿವು(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_NOSCRIPT:
		res = box_noscript(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_OBJECT:
		res = box_object(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_PRE:
		res = box_pre(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_SELECT:
		res = box_select(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ DOM_HTML_ELEMENT_TYPE_TEXTAREA:
		res = box_textarea(ಗಂಟು, content, box, convert_children);
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		res = true;
	}

	ಹಿಂತಿರುಗಿ res;
}
