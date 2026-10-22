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

#अन्तर्भाव <stdbool.h>

#define DIRLIST_NO_NAME_COLUMN 1
#define DIRLIST_NO_TYPE_COLUMN 1 << 1
#define DIRLIST_NO_SIZE_COLUMN 1 << 2
#define DIRLIST_NO_DATE_COLUMN 1 << 3
#define DIRLIST_NO_TIME_COLUMN 1 << 4

संरचना nsurl;

bool dirlist_generate_शीर्ष(वर्ण *buffer, पूर्णाङ्क buffer_length);
bool dirlist_generate_hide_columns(पूर्णाङ्क flags, वर्ण *buffer, पूर्णाङ्क buffer_length);
bool dirlist_generate_title(ध्रुव वर्ण *title, वर्ण *buffer, पूर्णाङ्क buffer_length);
bool dirlist_generate_parent_link(ध्रुव वर्ण *parent, वर्ण *buffer,
		पूर्णाङ्क buffer_length);
bool dirlist_generate_headings(वर्ण *buffer, पूर्णाङ्क buffer_length);
bool dirlist_generate_row(bool even, bool directory, संरचना nsurl *url,
		वर्ण *name, ध्रुव वर्ण *mimetype, दीर्घ दीर्घ size, वर्ण *date,
		वर्ण *समय, वर्ण *buffer, पूर्णाङ्क buffer_length);
bool dirlist_generate_bottom(वर्ण *buffer, पूर्णाङ्क buffer_length);

#endif
