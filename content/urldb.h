/*
 * Copyright 2006 John M Bell <jmb202@ecs.soton.ac.uk>
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
 * Unified URL information database internal interface.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_URLDB_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_URLDB_H

#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>

#ಅನ್ತರ್ಭಾವ "netsurf/url_db.h"
#ಅನ್ತರ್ಭಾವ "netsurf/cookie_db.h"

/**
 * Destroy urldb
 */
ಶೂನ್ಯ urldb_destroy(ಶೂನ್ಯ);


/**
 * Set the cross-session persistence of the entry for an URL
 *
 * \param url Absolute URL to persist
 * \param persist True to persist, false otherwise
 * \return NSERROR_OK on success or NSERROR_NOT_FOUND if url not in database
 */
nserror urldb_ಗಣ_url_persistence(ರಚನೆ nsurl *url, bool persist);


/**
 * Insert an URL into the database
 *
 * \param url Absolute URL to insert
 * \return true on success, false otherwise
 */
bool urldb_ಕೂಡಿಸು_url(ರಚನೆ nsurl *url);


/**
 * Set an URL's title string, replacing any existing one
 *
 * \param url The URL to look for
 * \param title The title string to use (copied)
 * \return NSERROR_OK on success otherwise appropriate error code
 */
nserror urldb_ಗಣ_url_title(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title);


/**
 * Set an URL's content type
 *
 * \param url The URL to look for
 * \param type The type to set
 * \return NSERROR_OK on success or NSERROR_NOT_FOUND if url not in database
 */
nserror urldb_ಗಣ_url_content_type(ರಚನೆ nsurl *url, content_type type);


/**
 * Set authentication data for an URL
 *
 * \param url The URL to consider
 * \param realm The authentication realm
 * \param auth The authentication details (in form username:password)
 */
ಶೂನ್ಯ urldb_ಗಣ_auth_details(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *auth);


/**
 * Look up authentication details in database
 *
 * \param url Absolute URL to search for
 * \param realm When non-NULL, it is realm which can be used to determine
 *        the protection space when that's not been done before for given URL.
 * \return Pointer to authentication details, or NULL if not found
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *urldb_get_auth_details(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *realm);


/**
 * Update an URL's visit data
 *
 * \param url The URL to update
 * \return NSERROR_OK on success or NSERROR_NOT_FOUND if url not in database
 */
nserror urldb_update_url_ಭೇಟಿ_ದತ್ತಾಂಶ(ರಚನೆ nsurl *url);


/**
 * Reset an URL's visit statistics
 *
 * \param url The URL to reset
 */
ಶೂನ್ಯ urldb_reset_url_ಭೇಟಿ_ದತ್ತಾಂಶ(ರಚನೆ nsurl *url);


/**
 * Extract an URL from the db
 *
 * \param url URL to extract
 * \return Pointer to database's copy of URL or NULL if not found
 */
ರಚನೆ nsurl *urldb_get_url(ರಚನೆ nsurl *url);


/**
 * Retrieve certificate verification permissions from database
 *
 * \param url Absolute URL to search for
 * \return true to permit connections to hosts with invalid certificates,
 * false otherwise.
 */
bool urldb_get_cert_permissions(ರಚನೆ nsurl *url);


/**
 * Parse Set-Cookie header and insert cookie(s) into database
 *
 * \param header Header to parse, with Set-Cookie: stripped
 * \param url URL being fetched
 * \param referrer Referring resource, or 0 for verifiable transaction
 * \return true on success, false otherwise
 */
bool urldb_ಗಣ_cookie(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆ, ರಚನೆ nsurl *url, ರಚನೆ nsurl *referrer);


/**
 * Retrieve cookies for an URL
 *
 * \param url URL being fetched
 * \param include_http_only Whether to include HTTP(S) only cookies.
 * \return Cookies string for libcurl (on heap), or NULL on error/no cookies
 */
ಅಕ್ಷರ *urldb_get_cookie(ರಚನೆ nsurl *url, bool include_http_only);


/**
 * Set HSTS policy for an URL
 *
 * \param url URL being fetched
 * \param header Strict-Transport-Security header value
 * \return true on success, false otherwise
 */
bool urldb_ಗಣ_hsts_policy(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶೀರ್ಷಿಕೆ);


/**
 * Determine if HSTS policy is enabled for an URL
 *
 * \param url URL being fetched
 * \return true if HSTS policy is enabled, false otherwise
 */
bool urldb_get_hsts_enabled(ರಚನೆ nsurl *url);

#ಆದರೆ_ಅಂತ್ಯ
