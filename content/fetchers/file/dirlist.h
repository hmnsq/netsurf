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

#که_نه_تعریف_شوی NETSURF_CONTENT_DIRLIST_H_
#تعریف_کړه NETSURF_CONTENT_DIRLIST_H_

#ضمن <stdbool.h>

#تعریف_کړه DIRLIST_NO_NAME_ستنه 1
#تعریف_کړه DIRLIST_NO_TYPE_ستنه 1 << 1
#تعریف_کړه DIRLIST_NO_اندازه_ستنه 1 << 2
#تعریف_کړه DIRLIST_NO_DATE_ستنه 1 << 3
#تعریف_کړه DIRLIST_NO_وخت_ستنه 1 << 4

بنية nsurl;

bool dirlist_generate_سر(حرف *بفر, صحيح بفر_اوږدتیا);
bool dirlist_generate_hide_columns(صحيح flags, حرف *بفر, صحيح بفر_اوږدتیا);
bool dirlist_generate_title(ثابت حرف *title, حرف *بفر, صحيح بفر_اوږدتیا);
bool dirlist_generate_پلار_link(ثابت حرف *پلار, حرف *بفر,
		صحيح بفر_اوږدتیا);
bool dirlist_generate_headings(حرف *بفر, صحيح بفر_اوږدتیا);
bool dirlist_generate_لیکه(bool even, bool directory, بنية nsurl *url,
		حرف *name, ثابت حرف *mimetype, اوږد اوږد اندازه, حرف *date,
		حرف *time, حرف *بفر, صحيح بفر_اوږدتیا);
bool dirlist_generate_bottom(حرف *بفر, صحيح بفر_اوږدتیا);

#د_که_پای
