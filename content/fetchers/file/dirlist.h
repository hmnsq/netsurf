/*
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
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
 * interface to generate HTML content for displaying directory listings.
 *
 * These functions should in general be called via the content interface.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NETSURF_CONTENT_DIRLIST_H_
#ವ್ಯಾಖ್ಯಾನಿಸು NETSURF_CONTENT_DIRLIST_H_

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ವ್ಯಾಖ್ಯಾನಿಸು DIRLIST_NO_NAME_ಸ್ತಂಭ 1
#ವ್ಯಾಖ್ಯಾನಿಸು DIRLIST_NO_TYPE_ಸ್ತಂಭ 1 << 1
#ವ್ಯಾಖ್ಯಾನಿಸು DIRLIST_NO_ಗಾತ್ರ_ಸ್ತಂಭ 1 << 2
#ವ್ಯಾಖ್ಯಾನಿಸು DIRLIST_NO_DATE_ಸ್ತಂಭ 1 << 3
#ವ್ಯಾಖ್ಯಾನಿಸು DIRLIST_NO_ಸಮಯ_ಸ್ತಂಭ 1 << 4

ರಚನೆ nsurl;

bool dirlist_generate_ತುದಿ(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_hide_columns(ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_title(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *title, ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_ಪೋಷಕ_link(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೋಷಕ, ಅಕ್ಷರ *ಬಫರ್,
		ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_headings(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_ಅಡ್ಡಸಾಲು(bool even, bool directory, ರಚನೆ nsurl *url,
		ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mimetype, ದೀರ್ಘ ದೀರ್ಘ ಗಾತ್ರ_2, ಅಕ್ಷರ *date,
		ಅಕ್ಷರ *time, ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);
bool dirlist_generate_bottom(ಅಕ್ಷರ *ಬಫರ್, ಪೂರ್ಣಾಂಕ ಬಫರ್_ಉದ್ದಳತೆ);

#ಆದರೆ_ಅಂತ್ಯ
