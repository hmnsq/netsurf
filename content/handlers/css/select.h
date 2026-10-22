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

#अगर_अघोषित NETSURF_CSS_SELECT_H_
#घोषणा NETSURF_CSS_SELECT_H_

#समावेश <stdint.h>

#समावेश <dom/dom.h>

#समावेश <libcss/libcss.h>

काष्ठा content;
काष्ठा nsurl;

/**
 * Selection context
 */
प्रकार काष्ठा nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	काष्ठा nsurl *base_url;
	lwc_string *universal;
	स्थिर css_computed_style *मूल_style;
	स्थिर css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(स्थिर uint8_t *तथ्य, माप_प्रकार len,
		स्थिर अक्षर *charset, स्थिर अक्षर *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_गाँठ *n,
		स्थिर css_media *media,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		स्थिर css_unit_ctx *unit_len_ctx,
		स्थिर css_computed_style *parent);


css_error named_ancestor_गाँठ(व्योम *pw, व्योम *गाँठ,
		स्थिर css_qname *qname, व्योम **ancestor);

css_error गाँठ_is_visited(व्योम *pw, व्योम *गाँठ, bool *match);

#पूर्ण_अगर
