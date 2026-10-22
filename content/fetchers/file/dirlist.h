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

#جے_بنا_تعریف NETSURF_CONTENT_DIRLIST_H_
#تعریف_کرو NETSURF_CONTENT_DIRLIST_H_

#ضمن <stdbool.h>

#تعریف_کرو DIRLIST_NO_NAME_کالم 1
#تعریف_کرو DIRLIST_NO_TYPE_کالم 1 << 1
#تعریف_کرو DIRLIST_NO_ناپ_کالم 1 << 2
#تعریف_کرو DIRLIST_NO_DATE_کالم 1 << 3
#تعریف_کرو DIRLIST_NO_ویلا_کالم 1 << 4

بنية nsurl;

bool dirlist_generate_چوٹی(حرف *بفر, صحيح بفر_لمائی);
bool dirlist_generate_hide_columns(صحيح flags, حرف *بفر, صحيح بفر_لمائی);
bool dirlist_generate_title(ثابت حرف *title, حرف *بفر, صحيح بفر_لمائی);
bool dirlist_generate_ماپا_link(ثابت حرف *ماپا, حرف *بفر,
		صحيح بفر_لمائی);
bool dirlist_generate_headings(حرف *بفر, صحيح بفر_لمائی);
bool dirlist_generate_صف(bool even, bool directory, بنية nsurl *url,
		حرف *name, ثابت حرف *mimetype, لما لما ناپ, حرف *date,
		حرف *time, حرف *بفر, صحيح بفر_لمائی);
bool dirlist_generate_bottom(حرف *بفر, صحيح بفر_لمائی);

#جے_انت
