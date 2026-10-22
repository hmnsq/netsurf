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

#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ଅନ୍ତର୍ଭାଵ <dom/dom.h>

#ଅନ୍ତର୍ଭାଵ <libcss/libcss.h>

ସଂରଚନା content;
ସଂରଚନା nsurl;

/**
 * Selection context
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ସଂରଚନା nsurl *base_url;
	lwc_string *universal;
	ଅଚଳ css_computed_style *ମୂଳ_style;
	ଅଚଳ css_computed_style *parent_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ଅଚଳ uint8_t *data, size_t len,
		ଅଚଳ ଅକ୍ଷର *charset, ଅଚଳ ଅକ୍ଷର *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ଗଣ୍ଠି *n,
		ଅଚଳ css_media *media,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ଅଚଳ css_unit_ctx *unit_len_ctx,
		ଅଚଳ css_computed_style *parent);


css_error named_ancestor_ଗଣ୍ଠି(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି,
		ଅଚଳ css_qname *qname, ଶୂନ୍ୟ **ancestor);

css_error ଗଣ୍ଠି_is_visited(ଶୂନ୍ୟ *pw, ଶୂନ୍ୟ *ଗଣ୍ଠି, bool *match);

#endif
