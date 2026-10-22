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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_FETCHERS_ABOUT_ಖಾಸಗಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_FETCHERS_ABOUT_ಖಾಸಗಿ_H

ರಚನೆ fetch_about_context;
ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ;

/**
 * set http response code on about response
 */
bool fetch_about_ಗಣ_http_ಸಂಕೇತ(ರಚನೆ fetch_about_context *ctx, ದೀರ್ಘ ಸಂಕೇತ_2);

/**
 * Send a header on the about response
 *
 * \param ctx The about fetch context
 * \param fmt The format specifier of the header
 * \return true if the fetch has been aborted else false
 */
bool fetch_about_send_ಶೀರ್ಷಿಕೆ(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);

/**
 * send data on the about response
 */
nserror fetch_about_senddata(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t ದತ್ತಾಂಶ_len);

/**
 * send formatted data on the about response
 */
nserror fetch_about_ssenddataf(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);

/**
 * complete the about fetch response
 */
bool fetch_about_send_ಪೂರ್ಣಗೊಂಡ(ರಚನೆ fetch_about_context *ctx);

/**
 * Generate a 500 server error respnse
 *
 * \param ctx The fetcher context.
 * \return true if handled false if aborted.
 */
bool fetch_about_srverror(ರಚನೆ fetch_about_context *ctx);

/**
 * get the fetch url
 */
ರಚನೆ nsurl *fetch_about_get_url(ರಚನೆ fetch_about_context *ctx);

/**
 * get multipart fetch data
 */
ಸ್ಥಿರಾಂಕ ರಚನೆ fetch_multipart_ದತ್ತಾಂಶ *fetch_about_get_multipart(ರಚನೆ fetch_about_context *ctx);


/**
 * cause a 302 redirect to target
 */
bool fetch_about_redirect(ರಚನೆ fetch_about_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ);

#ಆದರೆ_ಅಂತ್ಯ
