/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
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

/**
 * \file
 * Private interfaces for the about scheme fetcher.
 */

#నిర్వచితం_కాకపోతే NETSURF_CONTENT_FETCHERS_ABOUT_వ్యక్తిగత_H
#నిర్వచించు NETSURF_CONTENT_FETCHERS_ABOUT_వ్యక్తిగత_H

నిర్మాణం fetch_about_context;
నిర్మాణం fetch_multipart_దత్తాంశం;

/**
 * set http response code on about response
 */
bool fetch_about_సమితి_http_సంకేతం(నిర్మాణం fetch_about_context *ctx, దీర్ఘ సంకేతం_2);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_శీర్షిక(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి uint8_t *దత్తాంశం, size_t దత్తాంశం_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_ముగిసిన(నిర్మాణం fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(నిర్మాణం fetch_about_context *ctx);

/**
 * get the fetch url
 */
నిర్మాణం nsurl *fetch_about_get_url(నిర్మాణం fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
స్థిరరాశి నిర్మాణం fetch_multipart_దత్తాంశం *fetch_about_get_multipart(నిర్మాణం fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(నిర్మాణం fetch_about_context *ctx, స్థిరరాశి అక్షరం *లక్ష్యం);

#అయితే_ముగింపు
