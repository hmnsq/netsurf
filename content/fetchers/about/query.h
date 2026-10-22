/*
 * Copyright 2020 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf.
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
 * about scheme query handlers support interface
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_FETCHERS_ABOUT_QUERY_H
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_FETCHERS_ABOUT_QUERY_H

/**
 * generate a generic query description
 *
 * \param url The site url
 * \param key message key
 * \param out_str pointer to buffer with output
 */
nserror get_query_description(ರಚನೆ nsurl *url, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ, ಅಕ್ಷರ **ಹೊರಹೋಗು_str);

#ಆದರೆ_ಅಂತ್ಯ
