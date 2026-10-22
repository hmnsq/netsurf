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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CSS_SELECT_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CSS_SELECT_H_

#ಅನ್ತರ್ಭಾವ <stdint.h>

#ಅನ್ತರ್ಭಾವ <dom/dom.h>

#ಅನ್ತರ್ಭಾವ <libcss/libcss.h>

ರಚನೆ content;
ರಚನೆ nsurl;

/**
 * Selection context
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	ರಚನೆ nsurl *ತಳ_url;
	lwc_string *universal;
	ಸ್ಥಿರಾಂಕ css_computed_style *ಬೇರು_style;
	ಸ್ಥಿರಾಂಕ css_computed_style *ಪೋಷಕ_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *charset, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_ಗಂಟು *n,
		ಸ್ಥಿರಾಂಕ css_media *media,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		ಸ್ಥಿರಾಂಕ css_unit_ctx *unit_len_ctx,
		ಸ್ಥಿರಾಂಕ css_computed_style *ಪೋಷಕ);


css_error named_ancestor_ಗಂಟು(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು,
		ಸ್ಥಿರಾಂಕ css_qname *qname, ಶೂನ್ಯ **ancestor);

css_error ಗಂಟು_is_ಭೇಟಿಯಾದ(ಶೂನ್ಯ *pw, ಶೂನ್ಯ *ಗಂಟು, bool *ಹೊಂದಿಕೆ);

#ಆದರೆ_ಅಂತ್ಯ
