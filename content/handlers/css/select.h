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

#അന്തര്ഭാവ <stdint.h>

#അന്തര്ഭാവ <dom/dom.h>

#അന്തര്ഭാവ <libcss/libcss.h>

ഘടന content;
ഘടന nsurl;

/**
 * Selection context
 */
തരനിർവചനം ഘടന nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ഘടന nsurl *base_url;
	lwc_string *universal;
	സ്ഥിരം css_computed_style *വേര്_style;
	സ്ഥിരം css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(സ്ഥിരം uint8_t *data, size_t len,
		സ്ഥിരം അക്ഷരം *charset, സ്ഥിരം അക്ഷരം *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_കണ്ണി *n,
		സ്ഥിരം css_media *media,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		സ്ഥിരം css_unit_ctx *unit_len_ctx,
		സ്ഥിരം css_computed_style *parent);


css_error named_ancestor_കണ്ണി(ശൂന്യം *pw, ശൂന്യം *കണ്ണി,
		സ്ഥിരം css_qname *qname, ശൂന്യം **ancestor);

css_error കണ്ണി_is_visited(ശൂന്യം *pw, ശൂന്യം *കണ്ണി, bool *match);

#endif
