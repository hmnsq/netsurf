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

#اگر_غیر_تعریف_شدہ NETSURF_CONTENT_FETCHERS_ABOUT_نجی_H
#تعریف_کرو NETSURF_CONTENT_FETCHERS_ABOUT_نجی_H

ساخت fetch_about_context;
ساخت fetch_multipart_ڈیٹا;

/**
 * set http response code on about response
 */
bool fetch_about_سیٹ_http_رمز(ساخت fetch_about_context *ctx, طویل رمز);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_سرخی(ساخت fetch_about_context *ctx, ثابت حرف *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(ساخت fetch_about_context *ctx, ثابت uint8_t *ڈیٹا, size_t ڈیٹا_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(ساخت fetch_about_context *ctx, ثابت حرف *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_ختم_شدہ(ساخت fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(ساخت fetch_about_context *ctx);

/**
 * get the fetch url
 */
ساخت nsurl *fetch_about_get_url(ساخت fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
ثابت ساخت fetch_multipart_ڈیٹا *fetch_about_get_multipart(ساخت fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(ساخت fetch_about_context *ctx, ثابت حرف *ہدف);

#اگر_ختم
