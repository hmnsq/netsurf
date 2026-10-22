/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#ifndef NETSURF_CSS_SELECT_H_
#define NETSURF_CSS_SELECT_H_

#अन्तर्भाव <stdint.h>

#अन्तर्भाव <dom/dom.h>

#अन्तर्भाव <libcss/libcss.h>

संरचना content;
संरचना nsurl;

/**
 * Selection context
 */
प्रकारनिर्देश संरचना nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	संरचना nsurl *base_url;
	lwc_string *universal;
	ध्रुव css_computed_style *मूळ_style;
	ध्रुव css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ध्रुव uint8_t *data, size_t len,
		ध्रुव वर्ण *charset, ध्रुव वर्ण *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_गाठ *n,
		ध्रुव css_media *media,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ध्रुव css_unit_ctx *unit_len_ctx,
		ध्रुव css_computed_style *parent);


css_error named_ancestor_गाठ(शून्य *pw, शून्य *गाठ,
		ध्रुव css_qname *qname, शून्य **ancestor);

css_error गाठ_is_visited(शून्य *pw, शून्य *गाठ, bool *match);

#endif
