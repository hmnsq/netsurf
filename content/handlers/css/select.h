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

#నిర్వచితం_కాకపోతే NETSURF_CSS_SELECT_H_
#నిర్వచించు NETSURF_CSS_SELECT_H_

#అన్తర్భావ <stdint.h>

#అన్తర్భావ <dom/dom.h>

#అన్తర్భావ <libcss/libcss.h>

నిర్మాణం content;
నిర్మాణం nsurl;

/**
 * Selection context
 */
రకనిర్వచనం నిర్మాణం nscss_select_ctx
{
	css_select_ctx *ctx;
	bool quirks;
	నిర్మాణం nsurl *పునాది_url;
	lwc_string *universal;
	స్థిరరాశి css_computed_style *మూలం_style;
	స్థిరరాశి css_computed_style *మాతృ_style;
} nscss_select_ctx;

css_stylesheet *nscss_create_inline_style(స్థిరరాశి uint8_t *దత్తాంశం, size_t len,
		స్థిరరాశి అక్షరం *charset, స్థిరరాశి అక్షరం *url, bool allow_quirks);

css_select_results *nscss_get_style(nscss_select_ctx *ctx, dom_కణుపు *n,
		స్థిరరాశి css_media *media,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_stylesheet *inline_style);

css_computed_style *nscss_get_blank_style(nscss_select_ctx *ctx,
		స్థిరరాశి css_unit_ctx *unit_len_ctx,
		స్థిరరాశి css_computed_style *మాతృ);


css_error named_ancestor_కణుపు(శూన్య *pw, శూన్య *కణుపు,
		స్థిరరాశి css_qname *qname, శూన్య **ancestor);

css_error కణుపు_is_సందర్శించిన(శూన్య *pw, శూన్య *కణుపు, bool *సరిపోలిక);

#అయితే_ముగింపు
