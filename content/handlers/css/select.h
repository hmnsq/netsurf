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

#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

ಸಂರಚನಾ content;
ಸಂರಚನಾ nsurl;

/**
 * Selection context
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ಸಂರಚನಾ nsurl *base_url;
	lwc_string *universal;
	ಧ್ರುವ css_computed_style *ಬೇರು_style;
	ಧ್ರುವ css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ಧ್ರುವ uint8_t *data, size_t len,
		ಧ್ರುವ ವರ್ಣ *charset, ಧ್ರುವ ವರ್ಣ *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ಗಂಟು *n,
		ಧ್ರುವ css_media *media,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ಧ್ರುವ css_unit_ctx *unit_len_ctx,
		ಧ್ರುವ css_computed_style *parent);


css_error named_ancestor_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಧ್ರುವ css_qname *qname, ಶೂನ್ಯ **ancestor);

css_error ಗಂಟು_is_visited(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *match);

#endif
