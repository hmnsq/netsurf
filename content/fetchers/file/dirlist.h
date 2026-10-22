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

#جيڪڏهن_وصف_نه_ٿيل NETSURF_CONTENT_DIRLIST_H_
#وصف_ڪريو NETSURF_CONTENT_DIRLIST_H_

#ضمن <stdbool.h>

#وصف_ڪريو DIRLIST_NO_NAME_ڪالم 1
#وصف_ڪريو DIRLIST_NO_TYPE_ڪالم 1 << 1
#وصف_ڪريو DIRLIST_NO_سائيز_ڪالم 1 << 2
#وصف_ڪريو DIRLIST_NO_DATE_ڪالم 1 << 3
#وصف_ڪريو DIRLIST_NO_وقت_ڪالم 1 << 4

بنية nsurl;

bool dirlist_generate_چوٽي(حرف *بفر, صحيح بفر_ڊيگهائي);
bool dirlist_generate_hide_columns(صحيح flags, حرف *بفر, صحيح بفر_ڊيگهائي);
bool dirlist_generate_title(ثابت حرف *title, حرف *بفر, صحيح بفر_ڊيگهائي);
bool dirlist_generate_والد_link(ثابت حرف *والد, حرف *بفر,
		صحيح بفر_ڊيگهائي);
bool dirlist_generate_headings(حرف *بفر, صحيح بفر_ڊيگهائي);
bool dirlist_generate_صف(bool even, bool directory, بنية nsurl *url,
		حرف *name, ثابت حرف *mimetype, ڊگهو ڊگهو سائيز, حرف *date,
		حرف *time, حرف *بفر, صحيح بفر_ڊيگهائي);
bool dirlist_generate_bottom(حرف *بفر, صحيح بفر_ڊيگهائي);

#جيڪڏهن_پورو
