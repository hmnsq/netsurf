/*
 * Copyright 2011 John-Mark Bell <jmb@netsurf-browser.org>
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
 * MIME type sniffer interface
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_MIMESNIFF_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_MIMESNIFF_H_

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ <libwapcaplet/libwapcaplet.h>
#ಅನ್ತರ್ಭಾವ "utils/errors.h"

ರಚನೆ llcache_handle;

/**
 * Compute the effective MIME type for an object
 *
 * The implementation uses the sniffing algorithm described in
 * http://mimesniff.spec.whatwg.org/
 *
 * \param content_type_header Source content type header
 * \param data            First data chunk, or NULL
 * \param len             Length of \a data, in bytes
 * \param sniff_allowed   Whether MIME type sniffing is allowed
 * \param image_only      Sniff image types only
 * \param effective_type  Location to receive computed type
 * \return NSERROR_OK on success,
 *         NSERROR_NEED_DATA iff \a data is NULL and data is needed
 *         NSERROR_NOT_FOUND if sniffing is prohibited and no 
 *                           Content-Type header was found
 */
nserror mimesniff_compute_effective_type(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *content_type_ಶೀರ್ಷಿಕೆ,
		ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len, bool sniff_allowed,
		bool image_only, lwc_string **effective_type);

#ಆದರೆ_ಅಂತ್ಯ
