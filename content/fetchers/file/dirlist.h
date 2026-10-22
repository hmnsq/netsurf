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

#ifndef NETSURF_CONTENT_DIRLIST_H_
#define NETSURF_CONTENT_DIRLIST_H_

#অন্তর্ভাব <stdbool.h>

#define DIRLIST_NO_NAME_COLUMN 1
#define DIRLIST_NO_TYPE_COLUMN 1 << 1
#define DIRLIST_NO_আকার_COLUMN 1 << 2
#define DIRLIST_NO_DATE_COLUMN 1 << 3
#define DIRLIST_NO_TIME_COLUMN 1 << 4

কাঠামো nsurl;

bool dirlist_generate_শীর্ষ(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_hide_columns(পূর্ণসংখ্যা flags, অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_title(ধ্রুবক অক্ষর *title, অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_parent_link(ধ্রুবক অক্ষর *parent, অক্ষর *বাফার,
		পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_headings(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_row(bool even, bool directory, কাঠামো nsurl *url,
		অক্ষর *name, ধ্রুবক অক্ষর *mimetype, long long আকার_2, অক্ষর *date,
		অক্ষর *time, অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);
bool dirlist_generate_bottom(অক্ষর *বাফার, পূর্ণসংখ্যা বাফার_দৈর্ঘ্য);

#endif
