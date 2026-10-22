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

ساکن صحیح dirlist_filesize_calculate(غیرعلامتی طویل *bytesize);
ساکن صحیح dirlist_filesize_قدر(غیرعلامتی طویل bytesize);
ساکن حرف* dirlist_filesize_unit(غیرعلامتی طویل bytesize);


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

bool dirlist_generate_چوٹی(حرف *بفر, صحیح بفر_لمبائی)
{
	صحیح error = snprintf(بفر, بفر_لمبائی,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;

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

bool dirlist_generate_hide_columns(صحیح flags, حرف *بفر, صحیح بفر_لمبائی)
{
	صحیح error = snprintf(بفر, بفر_لمبائی,
			"%s\n%s\n%s\n%s\n%s\n",
			(flags & DIRLIST_NO_NAME_عمود) ?
					"span.name { display: none; }\n" : "",
			(flags & DIRLIST_NO_TYPE_عمود) ?
					"span.type { display: none; }\n" : "",
			(flags & DIRLIST_NO_سائز_عمود) ?
					"span.size { display: none; }\n" : "",
			(flags & DIRLIST_NO_DATE_عمود) ?
					"span.date { display: none; }\n" : "",
			(flags & DIRLIST_NO_وقت_عمود) ?
					"span.time { display: none; }\n" : "");
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
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

bool dirlist_generate_title(ثابت حرف *title, حرف *بفر, صحیح بفر_لمبائی)
{
	ثابت حرف *stylesheet;
	nserror err;
	صحیح error;

	اگر (title == تہی)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	اگر (err != NSERROR_OK) {
		واپس false;
	}

	error = snprintf(بفر, بفر_لمبائی,
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
			رنگ_rb_تبادلہ(nscolours[NSCOLOUR_WIN_ODD_BG]),
			stylesheet, title);
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
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

bool dirlist_generate_والد_link(ثابت حرف *والد, حرف *بفر,
		صحیح بفر_لمبائی)
{
	صحیح error = snprintf(بفر, بفر_لمبائی,
			"<p><a href=\"%s\">%s</a></p>",
			والد, messages_get("FileParent"));
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
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

bool dirlist_generate_headings(حرف *بفر, صحیح بفر_لمبائی)
{
	صحیح error = snprintf(بفر, بفر_لمبائی,
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
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
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

bool dirlist_generate_صف(bool even, bool directory, nsurl *url, حرف *name,
		ثابت حرف *mimetype, طویل طویل سائز, حرف *date, حرف *time,
		حرف *بفر, صحیح بفر_لمبائی)
{
	ثابت حرف *unit;
	حرف سائز_string[100];
	صحیح error;

	اگر (سائز < 0) {
		unit = "";
		strncpy(سائز_string, "", حجم سائز_string);
	} ورنہ {
		unit = messages_get(dirlist_filesize_unit((غیرعلامتی طویل)سائز));
		snprintf(سائز_string, حجم سائز_string, "%d",
				dirlist_filesize_قدر((غیرعلامتی طویل)سائز));
	}

	error = snprintf(بفر, بفر_لمبائی,
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
			name, mimetype, سائز_string, unit, date, time);
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
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

bool dirlist_generate_bottom(حرف *بفر, صحیح بفر_لمبائی)
{
	صحیح error = snprintf(بفر, بفر_لمبائی,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	اگر (error < 0 || error >= بفر_لمبائی)
		/* Error or buffer too small */
		واپس false;
	ورنہ
		/* OK */
		واپس true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

صحیح dirlist_filesize_calculate(غیرعلامتی طویل *bytesize)
{
	صحیح i = 0;
	جب (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		اگر (i == 3)
			توڑو;
	}
	واپس i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

صحیح dirlist_filesize_قدر(غیرعلامتی طویل bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	واپس (صحیح)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

حرف* dirlist_filesize_unit(غیرعلامتی طویل bytesize)
{
	ثابت حرف* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	واپس (حرف*)units[dirlist_filesize_calculate(&bytesize)];
}
