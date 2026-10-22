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

#جے_بنا_تعریف NETSURF_CONTENT_FETCHERS_ABOUT_نجی_H
#تعریف_کرو NETSURF_CONTENT_FETCHERS_ABOUT_نجی_H

بنية fetch_about_context;
بنية fetch_multipart_انکڑے;

/**
 * set http response code on about response
 */
bool fetch_about_سیٹ_http_کوڈ(بنية fetch_about_context *ctx, لما کوڈ);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_سرخی(بنية fetch_about_context *ctx, ثابت حرف *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(بنية fetch_about_context *ctx, ثابت uint8_t *انکڑے, size_t انکڑے_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(بنية fetch_about_context *ctx, ثابت حرف *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_مکمل(بنية fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(بنية fetch_about_context *ctx);

/**
 * get the fetch url
 */
بنية nsurl *fetch_about_get_url(بنية fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
ثابت بنية fetch_multipart_انکڑے *fetch_about_get_multipart(بنية fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(بنية fetch_about_context *ctx, ثابت حرف *نشانہ);

#جے_انت
