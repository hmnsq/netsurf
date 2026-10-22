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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_FETCHERS_ABOUT_ਨਿੱਜੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_FETCHERS_ABOUT_ਨਿੱਜੀ_H

ਬਣਤਰ fetch_about_context;
ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ;

/**
 * set http response code on about response
 */
bool fetch_about_ਸੈੱਟ_http_ਕੋਡ(ਬਣਤਰ fetch_about_context *ctx, ਲੰਮਾ ਕੋਡ);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_ਸਿਰਲੇਖ(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t ਅੰਕੜੇ_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_ਮੁਕੰਮਲ(ਬਣਤਰ fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(ਬਣਤਰ fetch_about_context *ctx);

/**
 * get the fetch url
 */
ਬਣਤਰ nsurl *fetch_about_get_url(ਬਣਤਰ fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
ਅਚਲ ਬਣਤਰ fetch_multipart_ਅੰਕੜੇ *fetch_about_get_multipart(ਬਣਤਰ fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(ਬਣਤਰ fetch_about_context *ctx, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ);

#ਜੇ_ਅੰਤ
