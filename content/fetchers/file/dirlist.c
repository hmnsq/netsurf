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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "utils/nsurl.h"
#ಅನ್ತರ್ಭಾವ "utils/messages.h"
#ಅನ್ತರ್ಭಾವ "utils/nscolour.h"

#ಅನ್ತರ್ಭಾವ "netsurf/inttypes.h"
#ಅನ್ತರ್ಭಾವ "netsurf/types.h"
#ಅನ್ತರ್ಭಾವ "netsurf/plot_style.h"

#ಅನ್ತರ್ಭಾವ "dirlist.h"
#ಅನ್ತರ್ಭಾವ "desktop/system_colour.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dirlist_filesize_calculate(unsigned long *bytesize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dirlist_filesize_ಮೌಲ್ಯ(unsigned long bytesize);
ಸ್ಥಿರ ವರ್ಣ* dirlist_filesize_unit(unsigned long bytesize);


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

bool dirlist_generate_ತುದಿ(ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಪೂರ್ಣಾಙ್ಕ error = snprintf(buffer, buffer_length,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;

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

bool dirlist_generate_hide_columns(ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಪೂರ್ಣಾಙ್ಕ error = snprintf(buffer, buffer_length,
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
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
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

bool dirlist_generate_title(ಧ್ರುವ ವರ್ಣ *title, ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಧ್ರುವ ವರ್ಣ *stylesheet;
	nserror err;
	ಪೂರ್ಣಾಙ್ಕ error;

	ಯದಿ (title == ರಿಕ್ತ)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ಯದಿ (err != NSERROR_OK) {
		ಪ್ರತ್ಯಾಗಮ false;
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
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
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

bool dirlist_generate_parent_link(ಧ್ರುವ ವರ್ಣ *parent, ವರ್ಣ *buffer,
		ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಪೂರ್ಣಾಙ್ಕ error = snprintf(buffer, buffer_length,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
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

bool dirlist_generate_headings(ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಪೂರ್ಣಾಙ್ಕ error = snprintf(buffer, buffer_length,
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
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
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

bool dirlist_generate_row(bool even, bool directory, nsurl *url, ವರ್ಣ *name,
		ಧ್ರುವ ವರ್ಣ *mimetype, long long size, ವರ್ಣ *date, ವರ್ಣ *time,
		ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಧ್ರುವ ವರ್ಣ *unit;
	ವರ್ಣ size_string[100];
	ಪೂರ್ಣಾಙ್ಕ error;

	ಯದಿ (size < 0) {
		unit = "";
		strncpy(size_string, "", ಪರಿಮಾಣ size_string);
	} ಅನ್ಯಥಾ {
		unit = messages_get(dirlist_filesize_unit((unsigned long)size));
		snprintf(size_string, ಪರಿಮಾಣ size_string, "%d",
				dirlist_filesize_ಮೌಲ್ಯ((unsigned long)size));
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
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
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

bool dirlist_generate_bottom(ವರ್ಣ *buffer, ಪೂರ್ಣಾಙ್ಕ buffer_length)
{
	ಪೂರ್ಣಾಙ್ಕ error = snprintf(buffer, buffer_length,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ಯದಿ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ಪ್ರತ್ಯಾಗಮ false;
	ಅನ್ಯಥಾ
		/* OK */
		ಪ್ರತ್ಯಾಗಮ true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

ಪೂರ್ಣಾಙ್ಕ dirlist_filesize_calculate(unsigned long *bytesize)
{
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	ಯಾವತ್ (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ಯದಿ (i == 3)
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

ಪೂರ್ಣಾಙ್ಕ dirlist_filesize_ಮೌಲ್ಯ(unsigned long bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

ವರ್ಣ* dirlist_filesize_unit(unsigned long bytesize)
{
	ಧ್ರುವ ವರ್ಣ* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)units[dirlist_filesize_calculate(&bytesize)];
}
