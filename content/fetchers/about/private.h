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

संरचना fetch_about_context;
संरचना fetch_multipart_data;

/**
 * set http response code on about response
 */
bool fetch_about_set_http_code(संरचना fetch_about_context *ctx, दीर्घ code);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_header(संरचना fetch_about_context *ctx, स्थिरांक अक्षर *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(संरचना fetch_about_context *ctx, स्थिरांक uint8_t *data, माप_प्रकार data_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(संरचना fetch_about_context *ctx, स्थिरांक अक्षर *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_finished(संरचना fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(संरचना fetch_about_context *ctx);

/**
 * get the fetch url
 */
संरचना nsurl *fetch_about_get_url(संरचना fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
स्थिरांक संरचना fetch_multipart_data *fetch_about_get_multipart(संरचना fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(संरचना fetch_about_context *ctx, स्थिरांक अक्षर *target);

#endif
