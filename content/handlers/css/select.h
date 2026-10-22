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

#অন্তর্ভাৱ <stdint.h>

#অন্তর্ভাৱ <dom/dom.h>

#অন্তর্ভাৱ <libcss/libcss.h>

সংরচনা content;
সংরচনা nsurl;

/**
 * Selection context
 */
প্রকারনির্দেশ সংরচনা nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	সংরচনা nsurl *base_url;
	lwc_string *universal;
	ধ্রুৱ css_computed_style *মূল_style;
	ধ্রুৱ css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ধ্রুৱ uint8_t *data, size_t len,
		ধ্রুৱ ৱর্ণ *charset, ধ্রুৱ ৱর্ণ *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_গ্ৰন্থি *n,
		ধ্রুৱ css_media *media,
		ধ্রুৱ css_unit_ctx *unit_len_ctx,
		ধ্রুৱ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ধ্রুৱ css_unit_ctx *unit_len_ctx,
		ধ্রুৱ css_computed_style *parent);


css_error named_ancestor_গ্ৰন্থি(শূন্য *pw, শূন্য *গ্ৰন্থি,
		ধ্রুৱ css_qname *qname, শূন্য **ancestor);

css_error গ্ৰন্থি_is_visited(শূন্য *pw, শূন্য *গ্ৰন্থি, bool *match);

#endif
