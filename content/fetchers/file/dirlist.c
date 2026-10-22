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

/** \file
 * Generate HTML content for displaying directory listings (implementation).
 */

#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>

#समावेश "utils/nsurl.h"
#समावेश "utils/messages.h"
#समावेश "utils/nscolour.h"

#समावेश "netsurf/inttypes.h"
#समावेश "netsurf/types.h"
#समावेश "netsurf/plot_style.h"

#समावेश "dirlist.h"
#समावेश "desktop/system_colour.h"

अटल पूर्णांक dirlist_filesize_calculate(अचिन्हित दीर्घ *bytesize);
अटल पूर्णांक dirlist_filesize_मूल्य(अचिन्हित दीर्घ bytesize);
अटल अक्षर* dirlist_filesize_unit(अचिन्हित दीर्घ bytesize);


/**
 * Generates the top part of an HTML directory listing page
 *
 * \return  Top of directory listing HTML
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_शीर्ष(अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	पूर्णांक error = snprintf(बफर, बफर_लंबाई,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;

}


/**
 * Generates the part of an HTML directory listing page that can suppress
 * particular columns
 *
 * \param  flags	  flags for which cols to suppress. 0 to suppress none
 * \param  buffer	  buffer to fill with generated HTML
 * \param  buffer_length  maximum size of buffer
 * \return  true iff buffer filled without error
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_hide_columns(पूर्णांक flags, अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	पूर्णांक error = snprintf(बफर, बफर_लंबाई,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_COLUMN) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_COLUMN) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_आकार_COLUMN) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_COLUMN) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_TIME_COLUMN) ?
					"span.time { display: none; }\n" : "");
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Generates the part of an HTML directory listing page that contains the title
 *
 * \param  title	  title to use
 * \param  buffer	  buffer to fill with generated HTML
 * \param  buffer_length  maximum size of buffer
 * \return  true iff buffer filled without error
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_title(स्थिर अक्षर *title, अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	स्थिर अक्षर *stylesheet;
	nserror err;
	पूर्णांक error;

	अगर (title == शून्य)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	अगर (err != NSERROR_OK) {
		वापस false;
	}

	error = snprintf(बफर, बफर_लंबाई,
			"</style>\n"
			"<title>%s</title>\n"
			"<style>\n"
			"html {\n"
			"\tbackground-color: #%06"PRIx32";\n"
			"}\n"
			"%s"
			"</style>\n"
			"</head>\n"
			"<body id=\"dirlist\" class=\"ns-even-bg ns-even-fg ns-border\">\n"
			"<h1 class=\"ns-border\">%s</h1>\n",
			title,
			colour_rb_swap(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Generates the part of an HTML directory listing page that links to the parent
 * directory
 *
 * \param  parent	  url of parent directory
 * \param  buffer	  buffer to fill with generated HTML
 * \param  buffer_length  maximum size of buffer
 * \return  true iff buffer filled without error
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_parent_link(स्थिर अक्षर *parent, अक्षर *बफर,
		पूर्णांक बफर_लंबाई)
{
	पूर्णांक error = snprintf(बफर, बफर_लंबाई,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Generates the part of an HTML directory listing page that displays the column
 * headings
 *
 * \param  buffer	  buffer to fill with generated HTML
 * \param  buffer_length  maximum size of buffer
 * \return  true iff buffer filled without error
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_headings(अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	पूर्णांक error = snprintf(बफर, बफर_लंबाई,
			"<div>\n"
			"<strong>\n"
			"\t<span class=\"name\">%s</span>\n"
			"\t<span class=\"type\">%s</span>\n"
			"\t<span class=\"size\">%s</span>"
			"<span class=\"size\"></span>\n"
			"\t<span class=\"date\">%s</span>\n"
			"\t<span class=\"time\">%s</span>\n"
			"</strong>\n",
			messages_get("FileName"), messages_get("FileType"),
			messages_get("FileSize"), messages_get("FileDate"),
			messages_get("FileTime"));
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Generates the part of an HTML directory listing page that displays a row
 * in the directory contents table
 *
 * \param  even		  evenness of row number, for alternate row colouring
 * \param  directory	  whether this row is for a directory (or a file)
 * \param  url		  url for row entry
 * \param  name		  name of row entry
 * \param  mimetype	  MIME type of row entry
 * \param  size		  size of row entry.  If negative, size is left blank
 * \param  date		  date row entry was last modified
 * \param  time		  time row entry was last modified
 * \param  buffer	  buffer to fill with generated HTML
 * \param  buffer_length  maximum size of buffer
 * \return  true iff buffer filled without error
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_row(bool even, bool directory, nsurl *url, अक्षर *name,
		स्थिर अक्षर *mimetype, दीर्घ दीर्घ आकार, अक्षर *date, अक्षर *समय,
		अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	स्थिर अक्षर *unit;
	अक्षर आकार_string[100];
	पूर्णांक error;

	अगर (आकार < 0) {
		unit = "";
		म_नकलन(आकार_string, "", माप आकार_string);
	} अन्यथा {
		unit = messages_get(dirlist_filesize_unit((अचिन्हित दीर्घ)आकार));
		snprintf(आकार_string, माप आकार_string, "%d",
				dirlist_filesize_मूल्य((अचिन्हित दीर्घ)आकार));
	}

	error = snprintf(बफर, बफर_लंबाई,
			"<a href=\"%s\" class=\"%s %s\">\n"
			"\t<span class=\"name ns-border\">%s</span>\n"
			"\t<span class=\"type ns-border\">%s</span>\n"
			"\t<span class=\"size ns-border\">%s</span>"
			"<span class=\"size ns-border\">%s</span>\n"
			"\t<span class=\"date ns-border\">%s</span>\n"
			"\t<span class=\"time ns-border\">%s</span>\n"
			"</a>\n", nsurl_access(url),
			even ? "even ns-even-bg" : "odd  ns-odd-bg",
			directory ? "dir" : "file",
			name, mimetype, आकार_string, unit, date, समय);
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Generates the bottom part of an HTML directory listing page
 *
 * \return  Bottom of directory listing HTML
 *
 * This is part of a series of functions.  To generate a complete page,
 * call the following functions in order:
 *
 *     dirlist_generate_top()
 *     dirlist_generate_hide_columns()  -- optional
 *     dirlist_generate_title()
 *     dirlist_generate_parent_link()   -- optional
 *     dirlist_generate_headings()
 *     dirlist_generate_row()           -- call 'n' times for 'n' rows
 *     dirlist_generate_bottom()
 */

bool dirlist_generate_bottom(अक्षर *बफर, पूर्णांक बफर_लंबाई)
{
	पूर्णांक error = snprintf(बफर, बफर_लंबाई,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	अगर (error < 0 || error >= बफर_लंबाई)
		/* Error or buffer too small */
		वापस false;
	अन्यथा
		/* OK */
		वापस true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

पूर्णांक dirlist_filesize_calculate(अचिन्हित दीर्घ *bytesize)
{
	पूर्णांक i = 0;
	जबतक (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		अगर (i == 3)
			अवरोध;
	}
	वापस i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

पूर्णांक dirlist_filesize_मूल्य(अचिन्हित दीर्घ bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	वापस (पूर्णांक)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

अक्षर* dirlist_filesize_unit(अचिन्हित दीर्घ bytesize)
{
	स्थिर अक्षर* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	वापस (अक्षर*)units[dirlist_filesize_calculate(&bytesize)];
}
