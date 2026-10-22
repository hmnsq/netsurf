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

#ifndef NETSURF_CONTENT_FETCHERS_ABOUT_PRIVATE_H
#define NETSURF_CONTENT_FETCHERS_ABOUT_PRIVATE_H

ഘടന fetch_about_context;
ഘടന fetch_multipart_data;

/**
 * set http response code on about response
 */
bool fetch_about_set_http_code(ഘടന fetch_about_context *ctx, long code);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_header(ഘടന fetch_about_context *ctx, സ്ഥിരം അക്ഷരം *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(ഘടന fetch_about_context *ctx, സ്ഥിരം uint8_t *data, size_t data_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(ഘടന fetch_about_context *ctx, സ്ഥിരം അക്ഷരം *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_finished(ഘടന fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(ഘടന fetch_about_context *ctx);

/**
 * get the fetch url
 */
ഘടന nsurl *fetch_about_get_url(ഘടന fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
സ്ഥിരം ഘടന fetch_multipart_data *fetch_about_get_multipart(ഘടന fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(ഘടന fetch_about_context *ctx, സ്ഥിരം അക്ഷരം *target);

#endif
