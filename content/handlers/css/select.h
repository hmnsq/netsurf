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

#که_نه_تعریف_شوی NETSURF_CSS_SELECT_H_
#تعریف_کړه NETSURF_CSS_SELECT_H_

#ضمن <stdint.h>

#ضمن <dom/dom.h>

#ضمن <libcss/libcss.h>

بنية content;
بنية nsurl;

/**
 * Selection context
 */
تعريفنوع بنية nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	بنية nsurl *بنسټ_url;
	lwc_string *universal;
	ثابت css_computed_style *ریښه_style;
	ثابت css_computed_style *پلار_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ثابت uint8_t *معلومات, size_t len,
		ثابت حرف *charset, ثابت حرف *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_غوټه *n,
		ثابت css_media *media,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ثابت css_unit_ctx *unit_len_ctx,
		ثابت css_computed_style *پلار);


css_error named_ancestor_غوټه(فراغ *pw, فراغ *غوټه,
		ثابت css_qname *qname, فراغ **ancestor);

css_error غوټه_is_لیدل_شوی(فراغ *pw, فراغ *غوټه, bool *سمون);

#د_که_پای
