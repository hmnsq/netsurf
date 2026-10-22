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

#اگر_تعریف_نشده NETSURF_CONTENT_DIRLIST_H_
#تعریف_کن NETSURF_CONTENT_DIRLIST_H_

#ضمن <stdbool.h>

#تعریف_کن DIRLIST_NO_NAME_ستونی 1
#تعریف_کن DIRLIST_NO_TYPE_ستونی 1 << 1
#تعریف_کن DIRLIST_NO_اندازه_ستونی 1 << 2
#تعریف_کن DIRLIST_NO_DATE_ستونی 1 << 3
#تعریف_کن DIRLIST_NO_زمان_ستونی 1 << 4

بنية nsurl;

bool dirlist_generate_بالاترین(حرف *میانگیر, صحيح میانگیر_درازی);
bool dirlist_generate_hide_columns(صحيح flags, حرف *میانگیر, صحيح میانگیر_درازی);
bool dirlist_generate_title(ثابت حرف *title, حرف *میانگیر, صحيح میانگیر_درازی);
bool dirlist_generate_والد_link(ثابت حرف *والد, حرف *میانگیر,
		صحيح میانگیر_درازی);
bool dirlist_generate_headings(حرف *میانگیر, صحيح میانگیر_درازی);
bool dirlist_generate_ردیف(bool even, bool directory, بنية nsurl *url,
		حرف *name, ثابت حرف *mimetype, دراز دراز اندازه, حرف *date,
		حرف *time, حرف *میانگیر, صحيح میانگیر_درازی);
bool dirlist_generate_bottom(حرف *میانگیر, صحيح میانگیر_درازی);

#پایان_اگر
