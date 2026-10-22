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

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/nsurl.h"
#include "utils/messages.h"
#include "utils/nscolour.h"

#include "netsurf/inttypes.h"
#include "netsurf/types.h"
#include "netsurf/plot_style.h"

#include "dirlist.h"
#include "desktop/system_colour.h"

static ຈຳນວນເຕັມ dirlist_filesize_calculate(unsigned long *bytesize);
static ຈຳນວນເຕັມ dirlist_filesize_value(unsigned long bytesize);
static var_na* dirlist_filesize_unit(unsigned long bytesize);


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

bool dirlist_generate_top(var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	ຈຳນວນເຕັມ error = snprintf(buffer, buffer_length,
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" title=\"Standard\" "
			"type=\"text/css\" href=\"resource:internal.css\">\n"
		"<style>\n");
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;

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

bool dirlist_generate_hide_columns(ຈຳນວນເຕັມ flags, var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	ຈຳນວນເຕັມ error = snprintf(buffer, buffer_length,
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
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
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

bool dirlist_generate_title(const var_na *title, var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	const var_na *stylesheet;
	nserror err;
	ຈຳນວນເຕັມ error;

	ຖ້າ (title == NULL)
		title = "";

	err = nscolour_get_stylesheet(&stylesheet);
	ຖ້າ (err != NSERROR_OK) {
		ສົ່ງຄືນ false;
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
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
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

bool dirlist_generate_parent_link(const var_na *parent, var_na *buffer,
		ຈຳນວນເຕັມ buffer_length)
{
	ຈຳນວນເຕັມ error = snprintf(buffer, buffer_length,
			"<p><a href=\"%s\">%s</a></p>",
			parent, messages_get("FileParent"));
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
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

bool dirlist_generate_headings(var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	ຈຳນວນເຕັມ error = snprintf(buffer, buffer_length,
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
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
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

bool dirlist_generate_row(bool even, bool directory, nsurl *url, var_na *name,
		const var_na *mimetype, long long size, var_na *date, var_na *time,
		var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	const var_na *unit;
	var_na size_string[100];
	ຈຳນວນເຕັມ error;

	ຖ້າ (size < 0) {
		unit = "";
		strncpy(size_string, "", parimaa_na size_string);
	} ບໍ່ດັ່ງນັ້ນ {
		unit = messages_get(dirlist_filesize_unit((unsigned long)size));
		snprintf(size_string, parimaa_na size_string, "%d",
				dirlist_filesize_value((unsigned long)size));
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
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
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

bool dirlist_generate_bottom(var_na *buffer, ຈຳນວນເຕັມ buffer_length)
{
	ຈຳນວນເຕັມ error = snprintf(buffer, buffer_length,
			"</div>\n"
			"</body>\n"
			"</html>\n");
	ຖ້າ (error < 0 || error >= buffer_length)
		/* Error or buffer too small */
		ສົ່ງຄືນ false;
	ບໍ່ດັ່ງນັ້ນ
		/* OK */
		ສົ່ງຄືນ true;
}


/**
 * Obtain display value and units for filesize after conversion to B/kB/MB/GB,
 * as appropriate.  
 *
 * \param  bytesize  file size in bytes, updated to filesize in output units
 * \return  number of times bytesize has been divided by 1024
 */

ຈຳນວນເຕັມ dirlist_filesize_calculate(unsigned long *bytesize)
{
	ຈຳນວນເຕັມ i = 0;
	ໃນຂະນະທີ່ (*bytesize > 1024 * 4) {
		*bytesize /= 1024;
		i++;
		ຖ້າ (i == 3)
			bha_nga;
	}
	ສົ່ງຄືນ i;
}


/**
 * Obtain display value for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Value to display for file size, in units given by filesize_unit()
 */

ຈຳນວນເຕັມ dirlist_filesize_value(unsigned long bytesize)
{
	dirlist_filesize_calculate(&bytesize);
	ສົ່ງຄືນ (ຈຳນວນເຕັມ)bytesize;
}


/**
 * Obtain display units for filesize after conversion to B/kB/MB/GB,
 * as appropriate
 *
 * \param  bytesize  file size in bytes
 * \return  Units to display for file size, for value given by filesize_value()
 */

var_na* dirlist_filesize_unit(unsigned long bytesize)
{
	const var_na* units[] = { "Bytes", "kBytes", "MBytes", "GBytes" };
	ສົ່ງຄືນ (var_na*)units[dirlist_filesize_calculate(&bytesize)];
}
