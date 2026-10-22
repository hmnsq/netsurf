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

#شامل <stdbool.h>
#شامل <string.h>
#شامل <stdio.h>
#شامل <stdlib.h>

#شامل "utils/nsurl.h"
#شامل "utils/messages.h"
#شامل "utils/nscolour.h"

#شامل "netsurf/inttypes.h"
#شامل "netsurf/types.h"
#شامل "netsurf/plot_style.h"

#شامل "dirlist.h"
#شامل "desktop/system_colour.h"

ایستا صحیح dirlist_filesize_calculate(unsigned long *bytesize);
ایستا صحیح dirlist_filesize_مقدار(unsigned long bytesize);
ایستا نویسه* dirlist_filesize_unit(unsigned long bytesize);


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

bool dirlist_generate_بالاترین(نویسه *buffer, صحیح buffer_length)
{
	صحیح error = snprintf(buffer, buffer_length,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;

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

bool dirlist_generate_hide_columns(صحیح flags, نویسه *buffer, صحیح buffer_length)
{
	صحیح error = snprintf(buffer, buffer_length,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_COLUMN) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_COLUMN) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_SIZE_COLUMN) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_COLUMN) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_TIME_COLUMN) ?
					"span.time { display: none; }\n" : "");
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
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

bool dirlist_generate_title(ثابت نویسه *title, نویسه *buffer, صحیح buffer_length)
{
	ثابت نویسه *stylesheet;
	nserror err;
	صحیح error;

	اگر (title == پوچ)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	اگر (err != NSERROR_OK) {
		بازگشت false;
	}

	error = snprintf(buffer, buffer_length,
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
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
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

bool dirlist_generate_parent_link(ثابت نویسه *parent, نویسه *buffer,
		صحیح buffer_length)
{
	صحیح error = snprintf(buffer, buffer_length,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
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

bool dirlist_generate_headings(نویسه *buffer, صحیح buffer_length)
{
	صحیح error = snprintf(buffer, buffer_length,
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
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
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

bool dirlist_generate_row(bool even, bool directory, nsurl *url, نویسه *name,
		ثابت نویسه *mimetype, long long size, نویسه *date, نویسه *time,
		نویسه *buffer, صحیح buffer_length)
{
	ثابت نویسه *unit;
	نویسه size_string[100];
	صحیح error;

	اگر (size < 0) {
		unit = "";
		strncpy(size_string, "", اندازه size_string);
	} وگرنه {
		unit = messages_get(dirlist_filesize_unit((unsigned long)size));
		snprintf(size_string, اندازه size_string, "%d",
				dirlist_filesize_مقدار((unsigned long)size));
	}

	error = snprintf(buffer, buffer_length,
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
			name, mimetype, size_string, unit, date, time);
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
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

bool dirlist_generate_bottom(نویسه *buffer, صحیح buffer_length)
{
	صحیح error = snprintf(buffer, buffer_length,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	اگر (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		بازگشت false;
	وگرنه
		/* OK */
		بازگشت true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

صحیح dirlist_filesize_calculate(unsigned long *bytesize)
{
	صحیح i = 0;
	وقتی (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		اگر (i == 3)
			بشکن;
	}
	بازگشت i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

صحیح dirlist_filesize_مقدار(unsigned long bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	بازگشت (صحیح)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

نویسه* dirlist_filesize_unit(unsigned long bytesize)
{
	ثابت نویسه* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	بازگشت (نویسه*)units[dirlist_filesize_calculate(&bytesize)];
}
