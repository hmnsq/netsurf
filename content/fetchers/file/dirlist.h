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

#अगर_अघोषित NETSURF_CONTENT_DIRLIST_H_
#घोषणा NETSURF_CONTENT_DIRLIST_H_

#समावेश <stdbool.h>

#घोषणा DIRLIST_NO_NAME_COLUMN 1
#घोषणा DIRLIST_NO_TYPE_COLUMN 1 << 1
#घोषणा DIRLIST_NO_आकार_COLUMN 1 << 2
#घोषणा DIRLIST_NO_DATE_COLUMN 1 << 3
#घोषणा DIRLIST_NO_TIME_COLUMN 1 << 4

काष्ठा nsurl;

bool dirlist_generate_शीर्ष(अक्षर *बफर, पूर्णांक बफर_लंबाई);
bool dirlist_generate_hide_columns(पूर्णांक flags, अक्षर *बफर, पूर्णांक बफर_लंबाई);
bool dirlist_generate_title(स्थिर अक्षर *title, अक्षर *बफर, पूर्णांक बफर_लंबाई);
bool dirlist_generate_parent_link(स्थिर अक्षर *parent, अक्षर *बफर,
		पूर्णांक बफर_लंबाई);
bool dirlist_generate_headings(अक्षर *बफर, पूर्णांक बफर_लंबाई);
bool dirlist_generate_row(bool even, bool directory, काष्ठा nsurl *url,
		अक्षर *name, स्थिर अक्षर *mimetype, दीर्घ दीर्घ आकार, अक्षर *date,
		अक्षर *समय, अक्षर *बफर, पूर्णांक बफर_लंबाई);
bool dirlist_generate_bottom(अक्षर *बफर, पूर्णांक बफर_लंबाई);

#पूर्ण_अगर
