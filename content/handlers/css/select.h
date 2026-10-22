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

#அந்தர்பாவ <stdint.h>

#அந்தர்பாவ <dom/dom.h>

#அந்தர்பாவ <libcss/libcss.h>

கட்டமைப்பு content;
கட்டமைப்பு nsurl;

/**
 * Selection context
 */
வகைவரை கட்டமைப்பு nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	கட்டமைப்பு nsurl *base_url;
	lwc_string *universal;
	மாறிலி css_computed_style *வேர்_style;
	மாறிலி css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(மாறிலி uint8_t *data, size_t len,
		மாறிலி எழுத்து *charset, மாறிலி எழுத்து *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_முனை *n,
		மாறிலி css_media *media,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		மாறிலி css_unit_ctx *unit_len_ctx,
		மாறிலி css_computed_style *parent);


css_error named_ancestor_முனை(வெறுமை *pw, வெறுமை *முனை,
		மாறிலி css_qname *qname, வெறுமை **ancestor);

css_error முனை_is_visited(வெறுமை *pw, வெறுமை *முனை, bool *match);

#endif
