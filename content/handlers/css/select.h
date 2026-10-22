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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CSS_SELECT_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CSS_SELECT_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <dom/dom.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <libcss/libcss.h>

ਬਣਤਰ content;
ਬਣਤਰ nsurl;

/**
 * Selection context
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ਬਣਤਰ nsurl *ਅਧਾਰ_url;
	lwc_string *universal;
	ਅਚਲ css_computed_style *ਜੜ੍ਹ_style;
	ਅਚਲ css_computed_style *ਮਾਪਾ_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len,
		ਅਚਲ ਅੱਖਰ *charset, ਅਚਲ ਅੱਖਰ *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ਗੰਢ *n,
		ਅਚਲ css_media *media,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ਅਚਲ css_unit_ctx *unit_len_ctx,
		ਅਚਲ css_computed_style *ਮਾਪਾ);


css_error named_ancestor_ਗੰਢ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ,
		ਅਚਲ css_qname *qname, ਖਾਲੀ **ancestor);

css_error ਗੰਢ_is_ਦੇਖਿਆ(ਖਾਲੀ *pw, ਖਾਲੀ *ਗੰਢ, bool *ਮੇਲ);

#ਜੇ_ਅੰਤ
