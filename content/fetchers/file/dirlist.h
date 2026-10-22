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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NETSURF_CONTENT_DIRLIST_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NETSURF_CONTENT_DIRLIST_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIRLIST_NO_NAME_ਕਾਲਮ 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIRLIST_NO_TYPE_ਕਾਲਮ 1 << 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIRLIST_NO_ਮਾਪ_ਕਾਲਮ 1 << 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIRLIST_NO_DATE_ਕਾਲਮ 1 << 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIRLIST_NO_ਸਮਾਂ_ਕਾਲਮ 1 << 4

ਬਣਤਰ nsurl;

bool dirlist_generate_ਸਿਖਰ(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_hide_columns(ਪੂਰਨ flags, ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_title(ਅਚਲ ਅੱਖਰ *title, ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_ਮਾਪਾ_link(ਅਚਲ ਅੱਖਰ *ਮਾਪਾ, ਅੱਖਰ *ਬਫਰ,
		ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_headings(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_ਪੰਕਤੀ(bool even, bool directory, ਬਣਤਰ nsurl *url,
		ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *mimetype, ਲੰਮਾ ਲੰਮਾ ਮਾਪ, ਅੱਖਰ *date,
		ਅੱਖਰ *time, ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);
bool dirlist_generate_bottom(ਅੱਖਰ *ਬਫਰ, ਪੂਰਨ ਬਫਰ_ਲੰਮਾਈ);

#ਜੇ_ਅੰਤ
