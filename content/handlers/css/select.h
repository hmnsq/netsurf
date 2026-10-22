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

#شامل <stdint.h>

#شامل <dom/dom.h>

#شامل <libcss/libcss.h>

ساختار content;
ساختار nsurl;

/**
 * Selection context
 */
تعریفنوع ساختار nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ساختار nsurl *base_url;
	lwc_string *universal;
	ثابت css_computed_style *ریشه_style;
	ثابت css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ثابت uint8_t *data, size_t len,
		ثابت نویسه *charset, ثابت نویسه *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_گره *n,
		ثابت css_media *media,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *parent);


css_error named_ancestor_گره(تهی *pw, تهی *گره,
		ثابت css_qname *qname, تهی **ancestor);

css_error گره_is_visited(تهی *pw, تهی *گره, bool *match);

#endif
