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

#شامل <stdbool.h>

#define DIRLIST_NO_NAME_COLUMN 1
#define DIRLIST_NO_TYPE_COLUMN 1 << 1
#define DIRLIST_NO_SIZE_COLUMN 1 << 2
#define DIRLIST_NO_DATE_COLUMN 1 << 3
#define DIRLIST_NO_TIME_COLUMN 1 << 4

ساختار nsurl;

bool dirlist_generate_بالاترین(نویسه *buffer, صحیح buffer_length);
bool dirlist_generate_hide_columns(صحیح flags, نویسه *buffer, صحیح buffer_length);
bool dirlist_generate_title(ثابت نویسه *title, نویسه *buffer, صحیح buffer_length);
bool dirlist_generate_parent_link(ثابت نویسه *parent, نویسه *buffer,
		صحیح buffer_length);
bool dirlist_generate_headings(نویسه *buffer, صحیح buffer_length);
bool dirlist_generate_row(bool even, bool directory, ساختار nsurl *url,
		نویسه *name, ثابت نویسه *mimetype, long long size, نویسه *date,
		نویسه *time, نویسه *buffer, صحیح buffer_length);
bool dirlist_generate_bottom(نویسه *buffer, صحیح buffer_length);

#endif
