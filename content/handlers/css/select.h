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

#અન્તર્ભાવ <stdint.h>

#અન્તર્ભાવ <dom/dom.h>

#અન્તર્ભાવ <libcss/libcss.h>

સંરચના content;
સંરચના nsurl;

/**
 * Selection context
 */
પ્રકારવ્યાખ્યા સંરચના nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	સંરચના nsurl *base_url;
	lwc_string *universal;
	અચલ css_computed_style *મૂળ_style;
	અચલ css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(અચલ uint8_t *data, size_t len,
		અચલ અક્ષર *charset, અચલ અક્ષર *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ગાંઠ *n,
		અચલ css_media *media,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		અચલ css_unit_ctx *unit_len_ctx,
		અચલ css_computed_style *parent);


css_error named_ancestor_ગાંઠ(શૂન્ય *pw, શૂન્ય *ગાંઠ,
		અચલ css_qname *qname, શૂન્ય **ancestor);

css_error ગાંઠ_is_visited(શૂન્ય *pw, શૂન્ય *ગાંઠ, bool *match);

#endif
